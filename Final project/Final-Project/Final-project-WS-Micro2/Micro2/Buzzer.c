/********************************************************************************
 * Name        : Buzzer.c														*
 * Author      : Mazen Khaled													*
 * Description : Source file for Buzzer driver									*
 * Date        : Jul 17, 2022													*
 ********************************************************************************/

#include "gpio.h"
#include "Buzzer.h"

void Buzzer_init(void){
	/* Setup the two Buzzer pin as output pin */
	GPIO_setupPinDirection(Buzzer_PORT,Buzzer_PIN,PIN_OUTPUT);

	/* Buzzer is stopped at the beginning */
	GPIO_writePin(Buzzer_PORT,Buzzer_PIN,LOGIC_LOW);
}

void Buzzer_On(void){
	/* Buzzer is On */
	GPIO_writePin(Buzzer_PORT,Buzzer_PIN,LOGIC_HIGH);
}

void Buzzer_Off(void){
	/* Buzzer is Off */
	GPIO_writePin(Buzzer_PORT,Buzzer_PIN,LOGIC_LOW);
}
