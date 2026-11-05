#ifndef INC_FSM_TRAFFIC_H_
#define INC_FSM_TRAFFIC_H_

void traffic_reset(void);   /* bat dau lai tu RED1_GREEN2 voi thoi gian hien tai */
void traffic_run(void);     /* goi lien tuc trong mode 1 */
int  traffic_time1(void);   /* so giay con lai cua den road 1 */
int  traffic_time2(void);   /* so giay con lai cua den road 2 */

#endif /* INC_FSM_TRAFFIC_H_ */
