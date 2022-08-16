/********************************************************************************
 * Name        : Buzzer.h														*
 * Author      : Mazen Khaled													*
 * Description : Header file for Buzzer driver									*
 * Date        : Jul 16, 2022													*
 *********************************************************************************/

#ifndef BUZZER_H_
#define BUZZER_H_

#define Buzzer_PORT 	PORTB_ID
#define Buzzer_PIN		PIN2_ID

void Buzzer_init(void);
void Buzzer_On(void);
void Buzzer_Off(void);

#endif /* BUZZER_H_ */
