/********************************************************************************
 * Name        : timer0.c														*
 * Author      : Mazen Khaled													*
 * Description : Source file for Full timer0 driver								*
 * Date        : Jul 16, 2022													*
 ********************************************************************************/

#include <avr/io.h>
#include <avr/interrupt.h>
#include "timer0.h"
#include "common_macros.h"

static void (*g_T0_CallBackPtr)(void) = NULL_PTR;

/********************************************************************************
 	 	 	 	 	 	 	 	 Functions
 *******************************************************************************/

ISR(TIMER0_OVF_vect){
	if(g_T0_CallBackPtr != NULL_PTR)
	{
		(*g_T0_CallBackPtr)();
	}
}
ISR(TIMER0_COMP_vect){
	if(g_T0_CallBackPtr != NULL_PTR)
	{
		(*g_T0_CallBackPtr)();
	}
}

/* Description :
 * 		1)Initialize timer0.
 * 		2)Select the Option(Compare/Overflow).
 * 		3)Start the timer.
 * 	Arguments :
 * 		---> Structure contain
 * 				{clock_frequancy,Mode(CM,NM),intial-counter,compare-value}
 */
void Timer0_init(const TIMER_ConfigType *config_Ptr){
	// Set Timer initial value to 0
	TCNT0 =  (config_Ptr -> Initial_Count);

	//Setting timer0 clock frequency
	TCCR0 = (TCCR0 & 0xF8) | ((config_Ptr -> clock_frequancy) & 0x07);

	//Overflow Mode
	if((config_Ptr -> Mode) == Overflow_Mode){
		TCCR0 |= (1<<FOC0);
		TIMSK |= (1<<TOIE0);
	}

	//For Compare Mode
	else if((config_Ptr -> Mode) == Compare_Mode){
		OCR0 = (config_Ptr -> CM_Compare_Value);
		TCCR0 = (1<<FOC0) | (1<<WGM01);
		TIMSK |= (1<<OCIE0);
	}
}

void Timer_DeInit (void){
	TCNT0  = 0;
	TCCR0  = 0;
	TIMSK &= 0xFC;
}

void Timer0_SetCallBack(void(*T0_ptr)(void)){
	g_T0_CallBackPtr = T0_ptr;
}
