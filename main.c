/* Project:	SmartModule
 * File:	main.c
 * Author:	Jonathan Ruisi
 * Created:	December 16, 2016, 6:28 AM
 */

// INCLUDES--------------------------------------------------------------------
#include <xc.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "main.h"
#include "button.h"
#include "buffer.h"
#include "utility.h"

// GLOBAL VARIABLES------------------------------------------------------------
volatile uint32_t _tick = 0;	// Global timekeeping variable (not related to RTCC)
volatile uint8_t _usartTargetTx = 0, _usartTargetRx = 0;	// Must be set to either 1 or 2 as needed
volatile ButtonInfo _button;
volatile RingBuffer _txBuffer1, _txBuffer2, _rxBuffer1, _rxBuffer2;
LineBuffer _lineBuffer1, _lineBuffer2;

// PROGRAM ENTRY---------------------------------------------------------------

void main(void)
{
	// Initialize device
	InitializeOscillator();
	InitializePorts();
	InitializeTimers();
	//InitializeSpi();
	InitializeUSART();
	InitializeInterrupts();

	// Initialize data
	uint32_t prevTick = 0;
	_button.currentState = BTN_RELEASE;
	_button.isDebouncing = false;
	_button.isUnhandled = false;
	_button.previousLogicLevel = 1;
	_button.currentLogicLevel = 1;
	_button.timestamp = 0;
	_button.pressAction = ButtonPress;
	_button.holdAction = ButtonHold;
	_button.releaseAction = ButtonRelease;
	char txBufferData1[TX1_BUFFER_SIZE] = {0};
	char txBufferData2[TX2_BUFFER_SIZE] = {0};
	char rxBufferData1[RX1_BUFFER_SIZE] = {0};
	char rxBufferData2[RX2_BUFFER_SIZE] = {0};
	char lineBufferData1[LINE_BUFFER_SIZE];
	char lineBufferData2[LINE_BUFFER_SIZE];
	_txBuffer1 = RingBufferCreate(TX1_BUFFER_SIZE, txBufferData1);
	_txBuffer2 = RingBufferCreate(TX2_BUFFER_SIZE, txBufferData2);
	_rxBuffer1 = RingBufferCreate(RX1_BUFFER_SIZE, rxBufferData1);
	_rxBuffer2 = RingBufferCreate(RX2_BUFFER_SIZE, rxBufferData2);
	_lineBuffer1 = LineBufferCreate(LINE_BUFFER_SIZE, lineBufferData1);
	_lineBuffer2 = LineBufferCreate(LINE_BUFFER_SIZE, lineBufferData2);

	// Start tick timer (Timer 4)
	T4CONbits.TMR4ON = true;

	// Main program loop
	while(true)
	{
		// Check if button status has changed and perform appropriate action
		CheckButton(&_button);

		if(_rxBuffer1.length > 0)
		{
			_usartTargetRx = 1;
			if(_lineBuffer1.isReceiving && !_lineBuffer1.isComplete)
			{
				_lineBuffer1.data[_lineBuffer1.length] = getch();
				if(_lineBuffer1.data[_lineBuffer1.length] == 0)
				{
					_lineBuffer1.isComplete = true;
					_lineBuffer1.isReceiving = false;
				}
				_lineBuffer1.length++;
			}
			else
			{
				_usartTargetTx = 1;
				putch(getch());
			}
		}

		if(_rxBuffer2.length > 0)
		{
			_usartTargetRx = 2;
			if(_lineBuffer2.isReceiving && !_lineBuffer2.isComplete)
			{
				_lineBuffer2.data[_lineBuffer2.length] = getch();
				if(_lineBuffer2.data[_lineBuffer2.length] == 0)
				{
					_lineBuffer2.isComplete = true;
					_lineBuffer2.isReceiving = false;
				}
				_lineBuffer2.length++;
			}
			else
			{
				_usartTargetTx = 2;
				putch(getch());
			}
		}

		if(_lineBuffer1.isComplete)
		{
			_usartTargetTx = 1;
			printf("%s\n\r", _lineBuffer1.data);
			_lineBuffer1.isComplete = false;
			_lineBuffer1.length = 0;
		}

		if(_lineBuffer2.isComplete)
		{
			_usartTargetTx = 2;
			printf("%s\n\r", _lineBuffer2.data);
			_lineBuffer2.isComplete = false;
			_lineBuffer2.length = 0;
		}
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
	REFOCONbits.ROON	= false;	// Output enable
}

void InitializePorts(void)
{
	// PORTA
	LATA	= 0b00000000;	// Clear port latch
	ANCON0	= 0b11110000;	// Enable analog inputs AN0-AN3
	TRISA	= 0b00111111;	// Output PORTA<7:6>, Input PORTA<5:0>

	// PORTB
	INTCON2bits.RBPU = 1;	// Disable weak pull-ups
	LATB	= 0b00001000;	// Clear port latch
	ANCON1	= 0b00010111;	// Disable analog inputs AN8-AN10,AN12
	TRISB	= 0b00000100;	// Output PORTB<7:3,1:0>, Input PORTB<2>

	// PORTC
	LATC	= 0b00000100;	// Clear port latch
	ANCON1bits.PCFG11 = 1;	// Disable analog input AN11
	TRISC	= 0b10010010;	// Output PORTC<6:5,3:2,0>, Input PORTC<7,4,1>

	// Configure Peripheral Pin Select
	EECON2 = 0x55;			// PPS register unlock sequence
	EECON2 = 0xAA;
	PPSCONbits.IOLOCK = 0;	// Unlock PPS registers
	RPINR1	= 0x02;			// Assign External Interrupt 1		(INT1) to RP2 (PORTA<5>)
	RPINR21	= 0x05;			// Assign SPI2 Data Input			(SDI2) to RP5 (PORTB<2>)
	RPINR16	= 0x0C;			// Assign USART2 Async. Receive		(RX2) to RP12 (PORTC<1>)
	RPOR7	= 0x0A;			// Assign SPI2 Data Output			(SDO2) to RP7 (PORTB<4>)
	RPOR8	= 0x0B;			// Assign SPI2 Clock Output			(SCK2) to RP8 (PORTB<5>)
	RPOR11	= 0x06;			// Assign USART2 Async. Transmit	(TX2) to RP11 (PORTC<0>)
	PPSCONbits.IOLOCK = 1;	// Lock PPS registers
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

void InitializeSpi(void)
{
	// Configure SPI2 module for master mode operation
	SSP2STATbits.CKE	= 0;	// Transmit occurs on transition from Idle to active clock state
	SSP2STATbits.SMP	= 1;	// Input data sampled at the end of data output time
	SSP2CON1bits.CKP	= 0;	// Idle state for clock is a low level
	SSP2CON1bits.SSPM	= 0x0;	// SPI Master mode, clock = FOSC/4
	ODCON3bits.SPI2OD	= 0;	// Open-drain capability is disabled
	SSP2CON1bits.SSPEN	= 1;	// Enable SPI2
}

void InitializeUSART(void)
{
	// USART2 (Debug connector on bottom of board)
	// Calculate BRG value for desired baud rate = 57600
	// 57600 = FOSC/[4(n+1)] = 48000000/[4(n+1)]  ->  n = 207.33
	// Calculated baud rate = 48000000/[4(207+1)] = 57692.31
	// Error = [(57692.31-57600)/57600]*100 = 0.160%  ->  Tests show good stability
	SPBRGH2	= 0x00;
	SPBRG2	= 0xCF;
	TXSTA2bits.BRGH		= true;		// High baud rate
	BAUDCON2bits.BRG16	= true;		// Use 16-bit baud rate register
	TXSTA2bits.SYNC		= false;	// Asynchronous mode

	// Configure transmission
	TXSTA2bits.TX9		= false;	// 8-bit transmission
	BAUDCON2bits.TXCKP	= false;	// Idle state for transmit = HIGH
	TXSTA2bits.TXEN		= true;		// Enable transmission

	// Configure reception
	RCSTA2bits.RX9		= false;	// 8-bit reception
	BAUDCON2bits.RXDTP	= false;	// Receive data is not inverted (active-high)
	RCSTA2bits.CREN		= true;		// Enable receiver

	// Enable serial port
	RCSTA2bits.SPEN		= true;
}

void InitializeInterrupts(void)
{
	// Configure external interrupt (pushbutton = active LOW on INT1 mapped to RP2)
	INTCON2bits.INTEDG1	= 0;	// Falling edge (button)
	INTCON3bits.INT1IP	= 1;	// High priority
	INTCON3bits.INT1IE	= 1;	// Enable

	// Configure Timer 4 period match interrupt
	IPR3bits.TMR4IP	= 1;	// High priority
	PIE3bits.TMR4IE	= 1;	// Enable

	// Configure USART2 interrupts
	IPR3bits.TX2IP	= 0;	// Low priority TX interrupt
	IPR3bits.RC2IP	= 0;	// Low priority RX interrupt
	//PIE3bits.TX2IE	= 1;	// Enable TX interrupt **This is done in putch(), not here**
	PIE3bits.RC2IE	= 1;	// Enable RX interrupt

	// Configure SPI2 interrupts
	//IPR3bits.SSP2IP	= 0;	// Low priority
	//PIE3bits.SSP2IE	= 1;	// Enable

	// Enable interrupts
	RCONbits.IPEN	= 1;	// Set prioritized interrupt mode
	INTCONbits.GIEH	= 1;	// Enable high-priority interrupts
	INTCONbits.GIEL	= 1;	// Enable low-priority interrupts
}

// BUTTON ACTIONS--------------------------------------------------------------

void ButtonPress(void)
{
	_lineBuffer2.isReceiving = true;
}

void ButtonHold(void)
{
	;
}

void ButtonRelease(void)
{
	;
}

// STDIO FUNCTIONS-------------------------------------------------------------

char getch(void)
{
	char data = 0;
	if(_usartTargetRx == 1)
	{
		data = RingBufferDequeue(&_rxBuffer1);
	}
	else if (_usartTargetRx == 2)
	{
		data = RingBufferDequeue(&_rxBuffer2);
	}
	return data;
}

// Writes a character to stdout, or in this case, USART1 or USART2
// Before calling any functions that print to stdout,
// make sure to specify the target USART by setting _usartTargetTx to 1 or 2.
// All stdout functions will utilize this function, such as printf().
// A ring buffer is used so that USART transmission can be interrupt driven.
// In cases where the buffer becomes full, transmission will temporarily
// switch to polling mode in order to flush the buffer and avoid data loss.

void putch(char data)
{
	if(_usartTargetTx == 1)
	{
		RingBufferEnqueue(&_txBuffer1, data);		// Add character to TX buffer
		if(_txBuffer1.length == TX1_BUFFER_SIZE)	// If buffer is full, switch to polling mode and flush buffer
		{
			PIE1bits.TX1IE	= false;
			while(_txBuffer1.length > 0)
			{
				while(!PIR1bits.TX1IF)
				{
					continue;
				}
				char character = RingBufferDequeue(&_txBuffer1);
				TXREG1 = character;
			}
		}
		PIE1bits.TX1IE	= true;						// Enable TX interrupt - this will interrupt immediately
	}
	else if(_usartTargetTx == 2)
	{
		RingBufferEnqueue(&_txBuffer2, data);
		if(_txBuffer2.length == TX2_BUFFER_SIZE)
		{
			PIE3bits.TX2IE	= false;
			while(_txBuffer2.length > 0)
			{
				while(!PIR3bits.TX2IF)
				{
					continue;
				}
				char character = RingBufferDequeue(&_txBuffer2);
				TXREG2 = character;
			}
		}
		PIE3bits.TX2IE	= true;
	}
}