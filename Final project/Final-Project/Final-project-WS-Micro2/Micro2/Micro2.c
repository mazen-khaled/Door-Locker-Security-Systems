/*
 ===============================================================================
 Name        : Micro2.c
 Author      : Mazen Khaled
 Description : Door lock security system
 Date        : Jul 16, 2022
 ===============================================================================
 */

/*******************************************************************************
 *                                Libraries                                    *
 *******************************************************************************/
#include <util/delay.h>
#include <avr/io.h>

#include "external_eeprom.h"
#include "dc_motor.h"
#include "Buzzer.h"
#include "timer0.h"
#include "uart.h"
#include "twi.h"
/*******************************************************************************
 *                                Definitions                                  *
 *******************************************************************************/
#define  OPEN_DOOR_SYMBOL        '+'
#define  CHANGE_PASS_SYMBOL      '-'
#define  PASSWORD_LONG           5
#define  READY_TO_COMMUNICATE    0x80
#define  PASSWORD_UNMATCH        0
#define  PASSWORD_MATCH          1
#define  Opening_the_Door   	 15
#define  Holding_The_Door   	 3
#define  Closing_The_Door  	 	 15
#define  Warning_Time            60
#define  STORED_PASS_ADRESS      0x00
#define  MAX_NUMPER_OF_TRIALS    3
#define  DANGER                  0x09
/*******************************************************************************
 *                             Global Variables                                *
 *******************************************************************************/
uint16 g_ticks = 0;
uint8 UART_Received_Byte=0;
uint8 recieved_password[PASSWORD_LONG];
uint8 password_Saved_In_EEPROM[PASSWORD_LONG];
uint8 Passowrd_Trials=0;
uint8 state =0;
/*******************************************************************************
 *                      	Functions Prototypes                               *
 *******************************************************************************/
/*For setting the password */
void Password_Setting(void);
/*For Receiving the password */
void Recieve_Passowrd(uint8 *pass);
/*Checking the two passwords */
uint8 Compare_Passowrds (uint8 *pass1,uint8 *pass2);
/*Send the password to the EEPROM */
void Save_Password_In_EEPROM(uint8 *pass);
/*For counting*/
void Timer0_callBack(void);
/*opening and closing the door */
void Door_Mession(void);
/*Rang in danger case */
void Buzzer_Mession(void);
/*******************************************************************************
 *                         		Main Function                                  *
 *******************************************************************************/
int main(void){
	SREG|=(1<<7); /*Enable I-bit*/

	/*Initialization UART*/
	UART_ConfigType UART_Configuration ={9600,_8_BITS,DISABLED,_1_BIT};
	UART_init(&UART_Configuration);

	/*Initialization The timer*/
	TIMER_ConfigType Timer_Configuration={F_CPU_1024,Compare_Mode,0,2900};
	Timer0_init(&Timer_Configuration);
	Timer0_SetCallBack(Timer0_callBack);

	/*Initialization I2C*/
	TWI_ConfigType TWI_configuretion ={Prescaler_1,0x77,0x02};
	TWI_init(&TWI_configuretion);

	/*Initialization DC-motor */
	DcMotor_Init();

	/*Initialization Buzzer*/
	Buzzer_init();

	/*Setting the password for the first time*/
	Password_Setting();
	while(1){

		/*Begin the process of receiving the password*/
		/*Two options need the password first*/
		UART_Received_Byte = UART_recieveByte();
		while(UART_Received_Byte != READY_TO_COMMUNICATE);
		UART_sendByte(READY_TO_COMMUNICATE);

		/*Receiving the password and save it in recieved_password*/
		Recieve_Passowrd(recieved_password);

		/*Receiving the option '+' for open the door or '-' for change password*/
		UART_Received_Byte = UART_recieveByte();

		/*Compare between the entered password and the password in EEPROM */
		state = Compare_Passowrds(recieved_password,password_Saved_In_EEPROM);

		/* Opening the door '+' */
		if(UART_Received_Byte == OPEN_DOOR_SYMBOL){

			/*for matched password open the door*/
			if (state == PASSWORD_MATCH){
				Passowrd_Trials = 0;
				UART_sendByte(PASSWORD_MATCH);
				Door_Mession();
			}
			/*for Unmatched password try again 2 times*/
			else if (state == PASSWORD_UNMATCH){
				Passowrd_Trials++;
				/*Check for the number of trials of entering the password*/
				if(Passowrd_Trials >= MAX_NUMPER_OF_TRIALS){
					UART_sendByte(DANGER);
					Buzzer_Mession();
					Passowrd_Trials = 0;
				}
				else UART_sendByte(PASSWORD_UNMATCH);
			}
		}

		/*Change the password setting '-' */
		else if (UART_Received_Byte == CHANGE_PASS_SYMBOL){
			/*for matched password New password setting*/
			if (state == PASSWORD_MATCH){
				Passowrd_Trials = 0;
				UART_sendByte(PASSWORD_MATCH);
				Password_Setting();
			}

			/*for Unmatched password try again 2 times*/
			else if (state == PASSWORD_UNMATCH){
				Passowrd_Trials++;
				if(Passowrd_Trials >= MAX_NUMPER_OF_TRIALS){
					UART_sendByte(DANGER);
					Buzzer_Mession();
					Passowrd_Trials = 0;
				}
				else UART_sendByte(PASSWORD_UNMATCH);
			}
		}
	}
	return 0;
}

/*******************************************************************************
 *                         		Function Definitions                           *
 *******************************************************************************/
void Timer0_callBack(void){
	g_ticks++;
}

void Password_Setting(void){

	/*variable to save password in*/
	uint8 Password[PASSWORD_LONG];

	/*variable to save password For confirming*/
	uint8 Re_Entered_Password[PASSWORD_LONG];

	/*State of the two entered password*/
	uint8 pass_state = PASSWORD_UNMATCH;


	while (pass_state == PASSWORD_UNMATCH){

		/*Begin the process of receiving the first password*/
		UART_Received_Byte = UART_recieveByte();
		while(UART_Received_Byte != READY_TO_COMMUNICATE);
		UART_sendByte(READY_TO_COMMUNICATE);

		/*receiving the first password and save it in Password*/
		Recieve_Passowrd(Password);

		/*Begin the process of receiving the second password*/
		UART_Received_Byte = UART_recieveByte();
		while(UART_Received_Byte != READY_TO_COMMUNICATE);
		UART_sendByte(READY_TO_COMMUNICATE);

		/*receiving the second password and save it in Re_Entered_Password*/
		Recieve_Passowrd(Re_Entered_Password);

		_delay_ms(50);

		/*Comparing between the 2 entered passwords and return it's state*/
		pass_state = Compare_Passowrds(Password,Re_Entered_Password);

		/*if the two passwords are matched save it in EEPROM (pass_state = 1)*/
		if ( pass_state == PASSWORD_MATCH){
			Save_Password_In_EEPROM(Password);
		}

		/*Sending the state of the two passwords to the MC1*/
		UART_sendByte(READY_TO_COMMUNICATE);
		UART_sendByte(pass_state);
	}
}

void Recieve_Passowrd(uint8 *pass){
	for (uint8 i=0;i<PASSWORD_LONG;i++)
	{
		pass[i]=UART_recieveByte();
		_delay_ms(50);
	}
}

uint8 Compare_Passowrds (uint8 * pass1,uint8 * pass2){

	uint8 i,flag=0;
	for (i=0 ; i<PASSWORD_LONG ; i++){
		if (pass1[i] != pass2[i]) {
			flag=1;
			break;
		}
		else
			flag = 0;
	}
	if (flag == 1) return PASSWORD_UNMATCH;
	else return PASSWORD_MATCH;
}

void Save_Password_In_EEPROM(uint8 * pass){
	for (uint8 i=0;i<PASSWORD_LONG;i++){
		EEPROM_writeByte(STORED_PASS_ADRESS +i ,pass[i]);
		password_Saved_In_EEPROM[i] = pass[i];
		_delay_ms(50);
	}
}

void Door_Mession(void){

	/*Opening the door in 15sec*/
	g_ticks=0;
	DcMotor_Rotate(DC_MOTOR_CW);
	while (g_ticks < Opening_the_Door);

	/*Holding the door in 3sec*/
	g_ticks=0;
	DcMotor_Rotate(DC_MOTOR_STOP);
	while (g_ticks < Holding_The_Door);

	/*Closing the door in 15sec*/
	g_ticks=0;
	DcMotor_Rotate(DC_MOTOR_ACW);
	while (g_ticks < Closing_The_Door);

	/*Stop the Motor*/
	DcMotor_Rotate(DC_MOTOR_STOP);
}

void Buzzer_Mession(void){
	g_ticks=0;
	Buzzer_On();
	while (g_ticks < Warning_Time);
	Buzzer_Off();
}
