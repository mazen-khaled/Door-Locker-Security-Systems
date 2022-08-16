/*
 ===============================================================================
 Name        : Micro1.c
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

#include "keypad.h"
#include "timer0.h"
#include "uart.h"
#include "lcd.h"
/*******************************************************************************
 *                                Definitions                                  *
 *******************************************************************************/
#define  OPEN_DOOR_SYMBOL        '+'
#define  CHANGE_PASS_SYMBOL      '-'
#define  PASSWORD_LONG           5
#define  READY_TO_COMMUNICATE    0x80
#define  ENTER                   13
#define  PASSWORD_UNMATCH        0
#define  PASSWORD_MATCH          1
#define  Time_To_Open_The_Door   15
#define  Time_To_Hold_The_Door   3
#define  Time_To_Close_The_Door  15
#define  Warning_Time            60
#define  DANGER                  0x09
/*******************************************************************************
 *                             Global Variables                                *
 *******************************************************************************/
uint8 Keypad_pressed_Key=0;
uint8 Entered_Password[PASSWORD_LONG];
uint8 UART_Received_Byte=0;
uint16 g_ticks = 0;
/*******************************************************************************
 *                      	Functions Prototypes                                *
 *******************************************************************************/
void Timer0_callBack(void);
void Password_Setting(void);
void Get_Password (uint8* ,uint8);
void Send_Password (uint8*);
void Main_Options_Display (void);
void Opening_the_Door();
void PASS_matched_Message(void);
void Wrong_Password_Message(void);
void Warning_Message(void);
void Enter_Pass_Message(void);
void RePassword_display(void);
void change_Pass_Message(void);
/******************************************************************************
 *                         		Main Function                              *
 ******************************************************************************/
int main(void){
	SREG|=(1<<7); /*Enable I-bit*/

	/*Initialization UART*/
	UART_ConfigType UART_Configuration ={9600,_8_BITS,DISABLED,_1_BIT};
	UART_init(&UART_Configuration);

	/*Initialization The timer*/
	TIMER_ConfigType Timer_Configuration={F_CPU_1024,Compare_Mode,0,2900};
	Timer0_init(&Timer_Configuration);
	Timer0_SetCallBack(Timer0_callBack);

	/*Initialization The LCD*/
	LCD_init();

	/*Setting the password for the first time*/
	Password_Setting();

	while(1){
		/*display for opening the door or change password*/
		Main_Options_Display();

		Keypad_pressed_Key = 0 ;

		/*check for '+' is pressed or '-' */
		while((Keypad_pressed_Key != OPEN_DOOR_SYMBOL)&&(Keypad_pressed_Key != CHANGE_PASS_SYMBOL)){
			Keypad_pressed_Key = KEYPAD_getPressedKey();
		}

		/*check if '+' is selected for open the door*/
		if (Keypad_pressed_Key == OPEN_DOOR_SYMBOL){
			Enter_Pass_Message();

			/*Enter the password*/
			Get_Password(Entered_Password,PASSWORD_LONG);

			/*send the password to MC2 for checking it*/
			Send_Password(Entered_Password);

			/*send the option to MC2*/
			UART_sendByte(OPEN_DOOR_SYMBOL);

			/*Receive the state of the entered password from MC2*/
			UART_Received_Byte = UART_recieveByte();

			/*check the states of the entered password*/
			if (UART_Received_Byte == PASSWORD_MATCH){
				Opening_the_Door();
			}
			else if(UART_Received_Byte  == PASSWORD_UNMATCH){
				Wrong_Password_Message();
			}
			else if(UART_Received_Byte  == DANGER){
				Warning_Message();
			}
		}

		/*check if '-' is selected for change the password*/
		else if(Keypad_pressed_Key == CHANGE_PASS_SYMBOL){
			Enter_Pass_Message();

			/*Enter the password*/
			Get_Password(Entered_Password,PASSWORD_LONG);

			/*send the password to MC2 for checking it*/
			Send_Password(Entered_Password);

			/*send the option to MC2*/
			UART_sendByte(CHANGE_PASS_SYMBOL);

			/*Receive the state of the entered password from MC2*/
			UART_Received_Byte = UART_recieveByte();

			/*check the states of the entered password*/
			if (UART_Received_Byte == PASSWORD_MATCH){
				LCD_clearScreen();
				change_Pass_Message();
				Password_Setting();
			}
			else if (UART_Received_Byte == PASSWORD_UNMATCH){
				Wrong_Password_Message();
			}
			else if (UART_Received_Byte  == DANGER){
				Warning_Message();
			}
		}
	}
	return 0;
}

/*******************************************************************************
 *                         	Functions Definitions                      		   *
 *******************************************************************************/

/*timer counter*/
void Timer0_callBack(void){
	g_ticks++;
}
/********************************************************************************/

/*password Setting*/
void Password_Setting(void){

	/*variable to save password in*/
	uint8 Password[PASSWORD_LONG];

	/*variable to save password For confirming*/
	uint8 Re_Entered_Password[PASSWORD_LONG];

	/*State of the two entered password*/
	uint8 pass_state = PASSWORD_UNMATCH;

	/*loop until Password is matched */
	while(pass_state == PASSWORD_UNMATCH){

		/*Entering the first password*/
		Enter_Pass_Message();
		Get_Password (Password,PASSWORD_LONG);
		Send_Password(Password);
		_delay_ms(200);

		/*Confirming the first password*/
		RePassword_display();
		Get_Password(Re_Entered_Password,PASSWORD_LONG);
		Send_Password(Re_Entered_Password);

		/*Waiting MC2 to check if the two password are the same*/
		while(UART_recieveByte()!=READY_TO_COMMUNICATE);

		/*check for Matched passwords receive 1 else receive 0 from MC2*/
		/*save the check in pass_state*/
		pass_state = UART_recieveByte();

		/*check the state of the entered password*/
		if (pass_state==PASSWORD_MATCH){
			PASS_matched_Message();
		}
		else if (pass_state==PASSWORD_UNMATCH){
			Wrong_Password_Message();
		}
		LCD_clearScreen();
	}
}
/*******************************************************************************/

/*save the password entered from keypad*/
void Get_Password (uint8* pass,uint8 size){
	uint8 key, i;

	/*loop for Receiving password from keypad and save it in the array*/
	for(i=0;i<size;i++){
		_delay_ms(200);
		key = KEYPAD_getPressedKey();
		LCD_displayCharacter('*');
		pass[i] = key;
		_delay_ms(200);
	}

	/*for conform the password*/
	while(KEYPAD_getPressedKey() != ENTER);
}
/*******************************************************************************/

/*Sending the password to MC2 using UART*/
void Send_Password (uint8* pass){
	UART_sendByte(READY_TO_COMMUNICATE);

	while(UART_recieveByte() != READY_TO_COMMUNICATE);

	for (uint8 i=0;i<PASSWORD_LONG;i++)
	{
		UART_sendByte(pass[i]);
		_delay_ms(50);
	}
}
/*******************************************************************************/

/*Opening the door then holding it then closing it after a certain time*/
void Opening_the_Door(){

	/*Opening time*/
	LCD_clearScreen();
	LCD_displayString("opening the Door");
	g_ticks = 0;
	while (g_ticks<=Time_To_Open_The_Door);

	/*Holding time*/
	LCD_clearScreen();
	LCD_displayString("Door is opened");
	g_ticks = 0;
	while (g_ticks<=Time_To_Hold_The_Door);

	/*Closing time*/
	LCD_clearScreen();
	LCD_displayString("closing the Door");
	g_ticks = 0;
	while (g_ticks<=Time_To_Close_The_Door);

	LCD_clearScreen();
}
/*******************************************************************************/
/*								LCD Display	Screen							   */
/*******************************************************************************/
void Main_Options_Display (void){
	LCD_clearScreen();
	LCD_displayStringRowColumn(0,0," + : Open door");
	LCD_displayStringRowColumn(1,0," - : Change Pass");
}
/*******************************************************************************/
void PASS_matched_Message(void){
	LCD_clearScreen();
	LCD_displayStringRowColumn(0,3,"Password");
	LCD_displayStringRowColumn(1,5,"Saved");
	_delay_ms(1500);
}
/*******************************************************************************/
void Wrong_Password_Message(void){
	LCD_clearScreen();
	LCD_displayStringRowColumn(0,3,"Wrong pass");
	LCD_displayStringRowColumn(1,3,"Try again");
	_delay_ms(1500);
}
/*******************************************************************************/
void Warning_Message(void){
	g_ticks=0;
	/*Flashing Warning massage*/
	while(g_ticks < Warning_Time ){
		LCD_clearScreen();
		LCD_displayStringRowColumn(0,0,"    Warning     ");
		LCD_displayStringRowColumn(1,0,"  !!!!!!!!!!!!  ");
		_delay_ms(300);
		LCD_clearScreen();
		_delay_ms(200);
	}
}
/*******************************************************************************/
void Enter_Pass_Message(void){
	LCD_clearScreen();
	LCD_displayStringRowColumn(0,0,"Please enter ");
	LCD_displayStringRowColumn(1,0,"Pass: ");
}
/*******************************************************************************/
void RePassword_display(void){
	LCD_clearScreen();
	LCD_displayStringRowColumn(0,0,"re-enter ");
	LCD_displayStringRowColumn(1,0,"Pass: ");
}
/*******************************************************************************/
void change_Pass_Message(void){
	LCD_clearScreen();
	LCD_displayStringRowColumn(0,0,"change pass ");
	LCD_displayStringRowColumn(1,0,"Pass: ");
}
/*******************************************************************************/
