/* Project:	SmartModule
 * File:	main.c
 * Author:	Jonathan Ruisi
 * Created:	December 16, 2016, 6:28 AM
 */

// TODO: Compiler warning 350 and linker warning 1090 have been disabled... REENABLE when convenient
// TODO: Compiler warning level was set to -3.  It is now at 0.  Consider changing back to the more verbose -3

// INCLUDES--------------------------------------------------------------------
#include <xc.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "main.h"
#include "button.h"
#include "serial_comm.h"
#include "sram.h"
#include "wifi.h"
#include "shell.h"
#include "linked_list.h"
#include "utility.h"

// GLOBAL VARIABLES------------------------------------------------------------
volatile uint32_t _tick = 0;	// Global timekeeping variable (not related to RTCC)
volatile ButtonInfo _button;
volatile Sram _sram;
CommPort _comm1, _comm2;
const CommDataRegisters _comm1Regs = {&TXREG1, (TXSTAbits_t*) & TXSTA1, &PIE1, 4};
const CommDataRegisters _comm2Regs = {&TXREG2, (TXSTAbits_t*) & TXSTA2, &PIE3, 4};
WifiInfo _wifi;
Shell _shell;

// PROGRAM ENTRY---------------------------------------------------------------

void main(void)
{
	// Initialize device
	ConfigureOscillator();
	ConfigureWDT();
	ConfigurePorts();
	ConfigureTimers();
	ConfigureSPI();
	ConfigureUSART();
	ConfigureRTCC();
	ConfigureInterrupts();
	ConfigureOS();

	uint32_t prevTick = 0;

	// Main program loop
main_loop:
	// BUTTON------------------------------------------
	UpdateButton(&_button);

	// USART-------------------------------------------
	UpdateCommPort(&_comm1);
	UpdateCommPort(&_comm2);

	// WIFI--------------------------------------------
	UpdateWifi();

	// SHELL-------------------------------------------
	ShellLoop();
	goto main_loop;
}

// INITIALIZATION--------------------------------------------------------------

void ConfigureOscillator(void)
{
	OSCCONbits.SCS	= 0b00;			// Select system clock = primary clock source (INTOSC)
	OSCCONbits.IRCF	= 0b111;		// Internal oscillator frequency select = 8MHz

	REFOCONbits.ROSEL	= 0;		// Source = FOSC
	REFOCONbits.RODIV	= 0;		// Source not scaled
	REFOCONbits.ROON	= false;	// Output enable
}

void ConfigureWDT(void)
{
	WDTCONbits.REGSLP	= 1;	// On-chip regulator enters low-power operation when device enters Sleep mode
	WDTCONbits.VBGOE	= 0;	// Band gap reference output is disabled
	WDTCONbits.ULPEN	= 0;	// Ultra low-power wake-up module is disabled
	WDTCONbits.ULPSINK	= 0;	// Ultra low-power wake-up current sink is disabled
	WDTCONbits.SWDTEN	= 0;	// Watchdog timer is off
}

void ConfigurePorts(void)
{
	// PORTA
	LATA	= 0b00000000;	// Clear port latch
#ifdef DEV_MODE_DEBUG
	ANCON0	= 0b11111111;	// Disable analog inputs AN0-AN3
	TRISA	= 0b00110000;	// Output PORTA<7:6,3:0>, Input PORTA<5:4>
#else
	ANCON0	= 0b11110000;	// Enable analog inputs AN0-AN3
	TRISA	= 0b00111111;	// Output PORTA<7:6>, Input PORTA<5:0>
#endif

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
	RPINR1	= 0x02;			// Assign External Interrupt 1		(INT1) to RP2	(PORTA<5>)
	RPINR21	= 0x05;			// Assign SPI2 Data Input			(SDI2) to RP5	(PORTB<2>)
	RPINR22	= 0x08;			// Assign SPI2 Clock Input			(SCK2IN) to RP8	(PORTB<5>)
	RPINR16	= 0x0C;			// Assign USART2 Async. Receive		(RX2) to RP12	(PORTC<1>)
	RPOR7	= 0x0A;			// Assign SPI2 Data Output			(SDO2) to RP7	(PORTB<4>)
	RPOR8	= 0x0B;			// Assign SPI2 Clock Output			(SCK2) to RP8	(PORTB<5>)
	RPOR11	= 0x06;			// Assign USART2 Async. Transmit	(TX2) to RP11	(PORTC<0>)
	PPSCONbits.IOLOCK = 1;	// Lock PPS registers
}

void ConfigureTimers(void)
{
	// Timer 4
	// (1/(FCY/prescale))*period*postscale = timer interval
	// (1/(12MHz/16))*250*3 = 1ms
	T4CONbits.T4CKPS	= 0x2;	// Clock prescale
	T4CONbits.T4OUTPS	= 0x2;	// Output postscale
	PR4					= 0xFA;	// Timer period
}

void ConfigureSPI(void)
{
	// Configure SPI2 module for master mode operation (Mode 0)
	ODCON3bits.SPI2OD	= 0;		// Open-drain capability is disabled
	SSP2CON1bits.CKP	= 0;		// Idle state for clock is a low level
	SSP2STATbits.CKE	= 1;		// Transmit occurs on transition from active to idle clock state
	SSP2STATbits.SMP	= 0;		// Input data sampled at the middle of data output time
	SSP2CON1bits.SSPM	= 0b0000;	// SPI Master mode, clock = FOSC/4
	SSP2CON1bits.SSPEN	= 1;		// Enable SPI2

	// Configure SPI2 DMA
	DMACON1bits.SSCON1		= 0;	// SSDMA pin is not controlled by the DMA module
	DMACON1bits.SSCON0		= 0;
	DMACON1bits.DUPLEX1		= 0;	// Always operates in half-duplex mode (RX or TX is determined in SRAM functions)
	DMACON1bits.DLYINTEN	= 0;	// Disable delay interrupt
	DMACON2bits.DLYCYC		= 0x0;	// Additional inter-byte delay during transfer = 0
	DMACON2bits.INTLVL		= 0x0;	// Generate interrupt when DMA transfer is complete
}

void ConfigureUSART(void)
{
	// USART1 (ESP8266 WiFi module)
	SPBRGH1	= GET_BYTE(CALCULATE_BRG_16H(76800), 1);
	SPBRG1	= GET_BYTE(CALCULATE_BRG_16H(76800), 0);
	TXSTA1bits.BRGH		= true;		// High baud rate
	BAUDCON1bits.BRG16	= true;		// Use 16-bit baud rate register
	TXSTA1bits.SYNC		= false;	// Asynchronous mode
	// Configure transmission
	TXSTA1bits.TX9		= false;	// 8-bit transmission
	BAUDCON1bits.TXCKP	= false;	// Idle state for transmit = HIGH
	TXSTA1bits.TXEN		= true;		// Enable transmission
	// Configure reception
	RCSTA1bits.RX9		= false;	// 8-bit reception
	BAUDCON1bits.RXDTP	= false;	// Receive data is not inverted (active-high)
	RCSTA1bits.CREN		= true;		// Enable receiver
	// Enable serial port
	RCSTA1bits.SPEN		= true;

	// USART2 (Debug connector on bottom of board)
	SPBRGH2	= GET_BYTE(CALCULATE_BRG_16H(115200), 1);
	SPBRG2	= GET_BYTE(CALCULATE_BRG_16H(115200), 0);
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

void ConfigureRTCC(void)
{
	T1CONbits.T1OSCEN	= true;		// Enable Timer1 internal RC oscillator (configured as clock source for RTCC)
	RTCCFGbits.RTCSYNC	= false;	// Prevent rollover ripple on RTCVALH, RTCVALL and ALCFGRPT
	RTCCFGbits.RTCOE	= false;	// RTCC clock output disabled
	ALRMCFGbits.ALRMEN	= false;	// Alarm is disabled

	// Unlock RTCC value registers
	EECON2 = 0x55;
	EECON2 = 0xAA;
	RTCCFGbits.RTCWREN	= true;

	// Set initial date and time (will be corrected once connected to WiFi)
	RTCCFGbits.RTCPTR1 = 1;
	RTCCFGbits.RTCPTR0 = 1;
	RTCVALH = 0x16;		// Year
	RTCVALL = 0x31;		// Day
	RTCVALH = DECEMBER;	// Month
	RTCVALL = 0x23;		// Hour
	RTCVALH = SATURDAY;	// Weekday
	RTCVALL = 0x00;		// Second
	RTCVALH = 0x59;		// Minute
	RTCCFGbits.RTCEN	= true;		// Enable RTCC
	RTCCFGbits.RTCWREN	= false;	// Lock RTCC value registers
}

void ConfigureInterrupts(void)
{
	// Configure external interrupt (pushbutton = active LOW on INT1 mapped to RP2)
	INTCON2bits.INTEDG1	= 0;	// Falling edge (button)
	INTCON3bits.INT1IP	= 1;	// High priority
	INTCON3bits.INT1IE	= 1;	// Enable

	// Configure Timer 4 period match interrupt
	IPR3bits.TMR4IP	= 1;	// High priority
	PIE3bits.TMR4IE	= 1;	// Enable

	// Configure USART1 interrupts
	IPR1bits.TX1IP	= 0;	// Low priority TX interrupt
	IPR1bits.RC1IP	= 0;	// Low priority RX interrupt
	PIE1bits.TX1IE	= 0;	// Disable TX interrupt
	PIE1bits.RC1IE	= 1;	// Enable RX interrupt

	// Configure USART2 interrupts
	IPR3bits.TX2IP	= 0;	// Low priority TX interrupt
	IPR3bits.RC2IP	= 0;	// Low priority RX interrupt
	PIE3bits.TX2IE	= 0;	// Disable TX interrupt
	PIE3bits.RC2IE	= 1;	// Enable RX interrupt

	// Configure SPI2 interrupts
	IPR3bits.SSP2IP	= 0;	// Low priority
	PIE3bits.SSP2IE	= 1;	// Enable

	// Enable interrupts
	RCONbits.IPEN	= 1;	// Set prioritized interrupt mode
	INTCONbits.GIEH	= 1;	// Enable high-priority interrupts
	INTCONbits.GIEL	= 1;	// Enable low-priority interrupts
}

void ConfigureOS(void)
{
	// Allocate buffers
	char txData1[TX_BUFFER_SIZE];
	char txData2[TX_BUFFER_SIZE];
	char rxData1[RX_BUFFER_SIZE];
	char rxData2[RX_BUFFER_SIZE];
	char lineData1[LINE_BUFFER_SIZE];
	char lineData2[LINE_BUFFER_SIZE];
	uint8_t lineQueueData1[COMM1_LINE_QUEUE_SIZE];
	uint8_t lineQueueData2[COMM2_LINE_QUEUE_SIZE];
	char swapData[LINE_BUFFER_SIZE];

	// Initialize global variables
	CommPortInitialize(&_comm1,
					TX_BUFFER_SIZE, RX_BUFFER_SIZE, LINE_BUFFER_SIZE,
					&txData1, &rxData1, &lineData1,
					COMM1_LINE_QUEUE_ADDR, COMM1_LINE_QUEUE_SIZE, &lineQueueData1,
					NEWLINE_CRLF, NEWLINE_CRLF,
					&_comm1Regs,
					false, false,
					COORD_VALUE_COMM1.y, COORD_VALUE_COMM1.x);
	CommPortInitialize(&_comm2,
					TX_BUFFER_SIZE, RX_BUFFER_SIZE, LINE_BUFFER_SIZE,
					&txData2, &rxData2, &lineData2,
					COMM2_LINE_QUEUE_ADDR, COMM2_LINE_QUEUE_SIZE, &lineQueueData2,
					NEWLINE_CRLF, NEWLINE_CR,
					&_comm2Regs,
					true, false,
					COORD_VALUE_COMM2.y, COORD_VALUE_COMM2.x);
	_comm2.modeBits.echoRx = true;
	ButtonInfoInitialize(&_button, ButtonPress, ButtonHold, ButtonRelease, 0);
	SramStatusInitialize();
	ShellInitialize(&_comm1, &_comm2, LINE_BUFFER_SIZE, swapData);
	// TODO: Shell needs to print initial command line... here

	// Hold Wifi in reset
	_wifi.statusBits.resetMode = WIFI_RESET_HOLD;
	WifiReset();
	_wifi.statusBits.boot = WIFI_BOOT_POWER_ON_RESET_HOLD;

	// Set SRAM mode: HOLD function disabled, burst write
	// Blank entire SRAM array (fill with 0xFF)
	SramMode mode;
	mode.value = 0;
	mode.holdDisabled = true;
	mode.mode = SRAM_MODE_BURST;
	SramSetMode(mode);
	/*while(_sram.busy)
		continue;
	SramFill(0x000000, SRAM_CAPACITY, 0xFF);*/

	// Start tick timer (Timer 4)
	T4CONbits.TMR4ON = true;
}

// BUTTON ACTIONS--------------------------------------------------------------

void ButtonPress(void)
{
	;
}

void ButtonHold(void)
{
	;
}

void ButtonRelease(void)
{
	;
}

// UTILITY FUNCTIONS-----------------------------------------------------------

void SetDateTime(DateTime* dateTime)
{
	if(dateTime == NULL)
		return;

	RTCCFGbits.RTCWREN = true;
	RTCCFGbits.RTCPTR1 = 1;
	RTCCFGbits.RTCPTR0 = 1;
	RTCVALH = dateTime->Year.ByteValue;
	RTCVALL = dateTime->Day.ByteValue;
	RTCVALH = dateTime->Month.ByteValue;
	RTCVALL = dateTime->Hour.ByteValue;
	RTCVALH = dateTime->Weekday;
	RTCVALL = dateTime->Second.ByteValue;
	RTCVALH = dateTime->Minute.ByteValue;
	RTCCFGbits.RTCWREN = false;
}

void GetDateTime(DateTime* dateTime)
{
	if(dateTime == NULL)
		return;

	//RTCCFGbits.RTCWREN = true;
	RTCCFGbits.RTCPTR1 = 1;
	RTCCFGbits.RTCPTR0 = 1;
	dateTime->Year.ByteValue	= RTCVALH;
	dateTime->Day.ByteValue		= RTCVALL;
	dateTime->Month.ByteValue	= RTCVALH;
	dateTime->Hour.ByteValue	= RTCVALL;
	dateTime->Weekday			= RTCVALH;
	dateTime->Second.ByteValue	= RTCVALL;
	dateTime->Minute.ByteValue	= RTCVALH;
	//RTCCFGbits.RTCWREN = false;
}

// DEBUG FUNCTIONS-------------------------------------------------------------
#ifdef DEV_MODE_DEBUG

void TestFunc1(void) {
	/*LinkedList_16Element_Initialize(&_list);

	LinkedListInsert(&_list, _list.last, (void*) 'J', false);
	LinkedListInsert(&_list, _list.last, (void*) 'o', false);
	LinkedListInsert(&_list, _list.last, (void*) 'h', false);
	LinkedListInsert(&_list, _list.last, (void*) 'n', false);
	LinkedListInsert(&_list, _list.last, (void*) 'a', false);
	LinkedListInsert(&_list, _list.last, (void*) 't', false);
	LinkedListInsert(&_list, _list.last, (void*) 'h', false);
	LinkedListInsert(&_list, _list.last, (void*) 'a', false);
	LinkedListInsert(&_list, _list.last, (void*) 'n', false);
	LinkedListInsert(&_list, _list.last, (void*) 'R', false);
	LinkedListInsert(&_list, _list.last, (void*) 'u', false);
	LinkedListInsert(&_list, _list.last, (void*) 'i', false);
	LinkedListInsert(&_list, _list.last, (void*) 's', false);
	LinkedListInsert(&_list, _list.last, (void*) 'i', false);
	LinkedListInsert(&_list, _list.last, (void*) '!', false);
	LinkedListInsert(&_list, _list.last, (void*) '?', false);
	CommPrintLinkedListInfo(&_list, &_comm2);
	CommPutString(&_comm2, "        ");
	CommPutLinkedListChars(&_list, &_comm2);
	CommPutNewline(&_comm2);
	CommPutNewline(&_comm2);

	LinkedListRemove(&_list, LinkedListFindFirst(&_list, (void*) 'h'));
	LinkedListRemove(&_list, LinkedListFindFirst(&_list, (void*) '!'));
	LinkedListRemove(&_list, LinkedListFindFirst(&_list, (void*) '?'));
	CommPrintLinkedListInfo(&_list, &_comm2);
	CommPutString(&_comm2, "        ");
	CommPutLinkedListChars(&_list, &_comm2);
	CommPutNewline(&_comm2);
	CommPutNewline(&_comm2);

	LinkedListReplace(&_list, LinkedListFindLast(&_list, (void*) 'n'), (void*) '.');
	LinkedListRemove(&_list, LinkedListFindFirst(&_list, (void*) 'a'));
	LinkedListRemove(&_list, LinkedListFindFirst(&_list, (void*) 't'));
	LinkedListRemove(&_list, LinkedListFindFirst(&_list, (void*) 'h'));
	LinkedListRemove(&_list, LinkedListFindFirst(&_list, (void*) 'a'));
	CommPrintLinkedListInfo(&_list, &_comm2);
	CommPutString(&_comm2, "        ");
	CommPutLinkedListChars(&_list, &_comm2);
	CommPutNewline(&_comm2);
	CommPutNewline(&_comm2);*/ }

#endif