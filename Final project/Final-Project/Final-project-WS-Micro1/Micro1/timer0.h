/********************************************************************************
* Name        : timer0.h														*
* Author      : Mazen Khaled													*
* Description : Header file for Full timer0 driver								*
* Date        : Jul 16, 2022													*
*********************************************************************************/

#ifndef TIMER0_H_
#define TIMER0_H_

#include "std_types.h"

/*******************************************************************************
 *                                Structures                                  *
 *******************************************************************************/

typedef enum
{
    NO_CLOCK,F_CPU_CLOCK,F_CPU_8,F_CPU_64,F_CPU_256,F_CPU_1024
}TIMER0_CLOCK;

typedef enum
{
    Overflow_Mode , Compare_Mode
}TIMER0_MODE;

typedef struct
{
	TIMER0_CLOCK clock_frequancy;
	TIMER0_MODE  Mode;
    uint8 Initial_Count;
    uint16 CM_Compare_Value;
}TIMER_ConfigType;

/*******************************************************************************
 *                      Functions Prototypes                                   *
 *******************************************************************************/

/* Description :
 * 		1)Initialize timer0.
 * 		2)Select the Option(Compare/Overflow).
 * 		3)Start the timer.
 * 	Arguments :
 * 		---> Structure contain
 * 				{clock_frequancy,Mode(CM,NM),intial-counter,compare-value}
 */
void Timer0_init(const TIMER_ConfigType *config_Ptr);
void Timer_DeInit(void);
void Timer0_SetCallBack(void(*T1_ptr)(void));

#endif /* TIMER0_H_ */
