 /******************************************************************************
 *
 * Module: UART
 *
 * File Name: uart.h
 *
 * Description: Header file for the UART AVR driver
 *
 * Author: Mohamed Tarek
 *
 *******************************************************************************/

#ifndef UART_H_
#define UART_H_

#include "std_types.h"

/*******************************************************************************
 *                                Structures                                  *
 *******************************************************************************/

typedef enum
{
	fdsf,sdf
}Baud_Rate;

typedef enum
{
	_5_BITS,_6_BITS,_7_BITS,_8_BITS,_9_BITS
}UART_DataBits;

typedef enum
{
	DISABLED,EVEN_PARITY=2,ODD_PARITY
}UART_ParityMode;

typedef enum
{
	_1_BIT,_2_BITS
}UART_StopBits;

typedef struct {

	uint32 BaudRate;
	UART_DataBits DataBits;
	UART_ParityMode  mode;
	UART_StopBits StopBits;
}UART_ConfigType;

/*******************************************************************************
 *                      Functions Prototypes                                   *
 *******************************************************************************/

/*
 * Description :
 * Functional responsible for Initialize the UART device by:
 * 1. Setup the Frame format like number of data bits, parity bit type and number of stop bits.
 * 2. Enable the UART.
 * 3. Setup the UART baud rate.
 */
void UART_init(const UART_ConfigType*);

/*
 * Description :
 * Functional responsible for send byte to another UART device.
 */
void UART_sendByte(const uint8 data);

/*
 * Description :
 * Functional responsible for receive byte from another UART device.
 */
uint8 UART_recieveByte(void);

/*
 * Description :
 * Send the required string through UART to the other UART device.
 */
void UART_sendString(const uint8 *Str);

/*
 * Description :
 * Receive the required string until the '#' symbol through UART from the other UART device.
 */
void UART_receiveString(uint8 *Str); // Receive until #

#endif /* UART_H_ */
