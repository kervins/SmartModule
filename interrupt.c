/*
 * File:   interrupt.c
 * Author: Jonathan
 *
 * Created on December 19, 2016, 5:26 AM
 */

#include <xc.h>
#include <stdbool.h>
#include <stdint.h>
#include "interrupt.h"
#include "main.h"

void __interrupt(high_priority) isrHighPriority(void)
{
	if(PIR3bits.TMR4IF)
	{
		++_tick;
		TMR4IF = FALSE;
	}
	else if(INTCONbits.INT0IF)
	{

		INT0IF = FALSE;
	}
	return;
}

void __interrupt(low_priority) isrLowPriority(void)
{
	return;
}