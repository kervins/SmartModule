/*
 * File:   main.c
 * Author: Jonathan
 *
 * Created on December 16, 2016, 6:28 AM
 */

// INCLUDES--------------------------------------------------------------------
#include <xc.h>
#include <stdbool.h>
#include <stdint.h>

#include "main.h"

// GLOBAL VARIABLES------------------------------------------------------------
volatile uint32_t _tick = 0;

// PROGRAM ENTRY---------------------------------------------------------------

void main(void)
{
	// Perform initialization
	InitializeOscillator();
	InitializePorts();
	InitializeTimers();
	InitializeInterrupts();

	// Start tick timer (Timer 4)
	T4CONbits.TMR4ON = TRUE;

	// Main program loop
	while(TRUE)
	{
		;
	}
	return;
}

// INITIALIZATION--------------------------------------------------------------

void InitializeOscillator(void)
{
	OSCCONbits.SCS	= 0b00;			// Select system clock = primary clock source (INTOSC)
	OSCCONbits.IRCF	= 0b111;		// Internal oscillator frequency select = 8MHz

	REFOCONbits.ROSEL	= 0;		// Source = FOSC
	REFOCONbits.RODIV	= 0;		// Source not scaled
	REFOCONbits.ROON	= TRUE;	// Output enable
}

void InitializePorts(void)
{
	// PORTA
	LATA	= 0b10000000;	// Clear port latch
	ANCON0	= 0b11110000;	// Enable analog input AN0-AN3
	TRISA	= 0b00011111;	// Output PORTA<7:5>, Input PORTA<3:0>

	// PORTB
	INTCON2bits.RBPU = 1;	// Disable weak pull-ups
	LATB	= 0b00000000;	// Clear port latch
	ANCON1	= 0b00010111;	// Disable analog inputs AN8-AN10,AN12
	TRISB	= 0b00001001;	// Output PORTB<7:4,2:1>, Input PORTB<3,0>

	// PORTC
	LATC	= 0b00000000;	// Clear port latch
	ANCON1bits.PCFG11 = 1;	// Disable analog inputs AN11
	TRISC	= 0b10000000;	// Outputs PORTC<6:0>, Input PORTC<7>
}

void InitializeTimers(void)
{
	// Timer 4
	// (1/(FCY/prescale))*period*postscale = timer interval
	// (1/(12MHz/16))*250*3 = 1ms
	T4CONbits.T4CKPS	= 0x2;	// Clock prescale
	T4CONbits.T4OUTPS	= 0x2;	// Output postscale
	PR4					= 0xFA;	// Timer period
}

void InitializeInterrupts(void)
{
	// Configure peripheral interrupts
	// Timer 4 period match
	IPR3bits.TMR4IP	= 1;	// High priority
	PIE3bits.TMR4IE	= 1;	// Enabled

	// Enable interrupts
	RCONbits.IPEN	= 1;	// Set prioritized interrupt mode
	INTCONbits.GIEH	= 1;	// Enable high-priority interrupts
	INTCONbits.GIEL	= 1;	// Enable low-priority interrupts
}