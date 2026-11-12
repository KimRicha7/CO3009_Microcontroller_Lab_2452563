/*
 * Cooperative scheduler using a "delta list" (tasks sorted by their run time).
 *  - Each task stores Delay = ticks remaining RELATIVE TO the task before it in the list.
 *  - SCH_Update only decrements the Delay of the first task -> O(1), no array scan.
 *  - Sorting (O(n)) is done in SCH_Add_Task / SCH_Dispatch_Tasks, in the main loop.
 *  - Tasks due at the same time run in the order they were added (FIFO).
 * DELAY and PERIOD are given in ms and converted to ticks using SCH_TICK_MS.
 */
#include "main.h"
#include "scheduler.h"

#define NONE 0xFF

typedef struct {
    void     (*pTask)(void);
    uint32_t Delay;      /* ticks remaining relative to the previous task in the list */
    uint32_t Period;     /* period (ticks), 0 = run once */
    uint32_t TaskID;     /* 0 = free slot */
    uint8_t  next;       /* index of the next task in the list, NONE = end */
} sTask;

static sTask SCH_tasks[SCH_MAX_TASKS];
static volatile uint8_t  head = NONE;          /* task that runs earliest */
static volatile uint32_t missedTicks = 0;      /* ticks elapsed after the first task became due but before it was dispatched */
static volatile uint32_t tickCount = 0;        /* tick counter used by get_time() */
static uint32_t nextID = 1;

/* ---- critical section against the interrupt (SCH_Update reads/writes the list) ---- */
static uint32_t enter_critical(void)
{
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    return primask;
}

static void exit_critical(uint32_t primask)
{
    if (!primask) __enable_irq();
}

static uint32_t ms_to_ticks(uint32_t ms)
{
    return ms / SCH_TICK_MS;
}

/* Insert slot 'idx' into the list, due 'delay' ticks from now.
 * Must be called inside the critical section. */
static void list_insert(uint8_t idx, uint32_t delay)
{
    uint8_t prev = NONE, cur = head;

    /* Skip tasks due earlier or at the same time (<=) to keep FIFO order */
    while (cur != NONE && SCH_tasks[cur].Delay <= delay) {
        delay -= SCH_tasks[cur].Delay;
        prev = cur;
        cur = SCH_tasks[cur].next;
    }
    SCH_tasks[idx].Delay = delay;
    SCH_tasks[idx].next = cur;
    if (cur != NONE) SCH_tasks[cur].Delay -= delay;   /* next task is now relative to the new task */
    if (prev == NONE) head = idx; else SCH_tasks[prev].next = idx;
}

/* Remove 'idx' from the list and give its Delay to the task behind it.
 * Must be called inside the critical section. Returns 1 if found. */
static uint8_t list_remove(uint8_t idx)
{
    uint8_t prev = NONE, cur = head;
    while (cur != NONE && cur != idx) {
        prev = cur;
        cur = SCH_tasks[cur].next;
    }
    if (cur == NONE) return 0;

    uint8_t nxt = SCH_tasks[idx].next;
    if (nxt != NONE) SCH_tasks[nxt].Delay += SCH_tasks[idx].Delay;
    if (prev == NONE) head = nxt; else SCH_tasks[prev].next = nxt;
    SCH_tasks[idx].next = NONE;
    return 1;
}

void SCH_Init(void)
{
    uint32_t pm = enter_critical();
    for (uint8_t i = 0; i < SCH_MAX_TASKS; i++) {
        SCH_tasks[i].pTask  = 0;
        SCH_tasks[i].Delay  = 0;
        SCH_tasks[i].Period = 0;
        SCH_tasks[i].TaskID = SCH_NO_TASK_ID;
        SCH_tasks[i].next   = NONE;
    }
    head = NONE;
    missedTicks = 0;
    tickCount = 0;
    exit_critical(pm);
}

/* Called every SCH_TICK_MS ms from the timer interrupt. No loop -> O(1). */
void SCH_Update(void)
{
    tickCount++;
    if (head == NONE) return;
    if (SCH_tasks[head].Delay > 0) {
        SCH_tasks[head].Delay--;
    } else {
        missedTicks++;          /* first task is due but the main loop has not run it yet */
    }
}

uint32_t SCH_Add_Task(void (*pFunction)(void), uint32_t DELAY, uint32_t PERIOD)
{
    if (pFunction == 0) return SCH_NO_TASK_ID;

    uint32_t pm = enter_critical();
    uint8_t idx = 0;
    while (idx < SCH_MAX_TASKS && SCH_tasks[idx].TaskID != SCH_NO_TASK_ID) idx++;
    if (idx == SCH_MAX_TASKS) {             /* no free slot */
        exit_critical(pm);
        return SCH_NO_TASK_ID;
    }

    SCH_tasks[idx].pTask  = pFunction;
    SCH_tasks[idx].Period = ms_to_ticks(PERIOD);
    SCH_tasks[idx].TaskID = nextID++;
    if (nextID == SCH_NO_TASK_ID) nextID = 1;
    list_insert(idx, ms_to_ticks(DELAY));

    uint32_t id = SCH_tasks[idx].TaskID;
    exit_critical(pm);
    return id;
}

uint8_t SCH_Delete_Task(uint32_t taskID)
{
    if (taskID == SCH_NO_TASK_ID) return SCH_ERROR;

    uint32_t pm = enter_critical();
    for (uint8_t i = 0; i < SCH_MAX_TASKS; i++) {
        if (SCH_tasks[i].TaskID == taskID) {
            list_remove(i);                 /* may not be in the list if it is currently running */
            SCH_tasks[i].pTask  = 0;
            SCH_tasks[i].Period = 0;
            SCH_tasks[i].TaskID = SCH_NO_TASK_ID;
            if (head == NONE) missedTicks = 0;
            exit_critical(pm);
            return SCH_OK;
        }
    }
    exit_critical(pm);
    return SCH_ERROR;
}

void SCH_Dispatch_Tasks(void)
{
    while (1) {
        uint32_t pm = enter_critical();
        if (head == NONE || SCH_tasks[head].Delay > 0) {
            exit_critical(pm);
            break;                          /* no task is due yet */
        }

        /* Take the first task off the list */
        uint8_t idx = head;
        uint32_t late = missedTicks;        /* how many ticks late this task is */
        head = SCH_tasks[idx].next;
        SCH_tasks[idx].next = NONE;

        /* Pass the late ticks on to the next task so the list stays correct relative to "now" */
        if (head == NONE) {
            missedTicks = 0;
        } else {
            uint32_t d = (SCH_tasks[head].Delay < missedTicks) ? SCH_tasks[head].Delay : missedTicks;
            SCH_tasks[head].Delay -= d;
            missedTicks -= d;
        }

        void (*fn)(void) = SCH_tasks[idx].pTask;
        if (SCH_tasks[idx].Period > 0) {
            /* Periodic task: re-insert it, minus the lateness, so its timing does not drift */
            uint32_t p = SCH_tasks[idx].Period;
            list_insert(idx, (p > late) ? (p - late) : 0);
        } else {
            /* One-shot task: free its slot */
            SCH_tasks[idx].pTask  = 0;
            SCH_tasks[idx].TaskID = SCH_NO_TASK_ID;
        }
        exit_critical(pm);

        fn();                               /* run the task outside the critical section */
    }
    /* HAL_PWR_EnterSLEEPMode could be added here to save power */
}

uint32_t get_time(void)
{
    return tickCount * SCH_TICK_MS;
}
