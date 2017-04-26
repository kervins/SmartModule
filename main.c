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
#include "smartmodule.h"
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
	ConfigureADC();
	ConfigureInterrupts();
	ConfigureOS();

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
	ANCON0	= 0b11111110;	// Enable analog input AN0
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
	RPINR1	= 0x02;			// Assign External Interrupt 1		(INT1) to RP2	(PORTA<5>)
	RPINR2	= 0x01;			// Assign External Interrupt 2		(INT2) to RP1	(PORTA<1>)
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
	// Timer 4 (used to generate OS tick)
	// (1/(FCY/prescale))*period*postscale = timer interval
	// (1/(12MHz/16))*250*3 = 1ms
	T4CONbits.T4CKPS	= 0x2;	// Clock prescale
	T4CONbits.T4OUTPS	= 0x2;	// Output postscale
	PR4					= 0xFA;	// Timer period

	// Timer 6 (used to control ADC sample rate)
	// (1/(12MHz/16))*250*5 = 1.67ms = (1/600Hz)
	T6CONbits.T6CKPS	= 0x2;	// Clock prescale
	T6CONbits.T6OUTPS	= 0x4;	// Output postscale
	PR6					= 0xFA;	// Timer period

	// Timer 0 (governs the pulse width of the relay control signals)
	T0CONbits.T08BIT	= 0;	// Configured for 16-bit operation
	T0CONbits.T0CS		= 0;	// Use instruction clock (FCY) as timer clock source
	T0CONbits.PSA		= 0;	// Enable prescaler
	T0CONbits.T0PS		= 6;	// Prescaler = 1:128
	TMR0				= TIMER0_START_VALUE;
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
	RTCVALH = 0x17;		// Year
	RTCVALL = 0x26;		// Day
	RTCVALH = APRIL;	// Month
	RTCVALL = 0x08;		// Hour
	RTCVALH = WEDNESDAY;	// Weekday
	RTCVALL = 0x00;		// Second
	RTCVALH = 0x00;		// Minute
	RTCCFGbits.RTCEN	= true;		// Enable RTCC
	RTCCFGbits.RTCWREN	= false;	// Lock RTCC value registers
}

void ConfigureADC(void)
{
	ADCON0bits.VCFG		= 0;	// VREF- = GND ... VREF+ = 3.3V
	ADCON0bits.ADCAL	= 0;	// Normal A/D Converter operation (no calibration is performed)
	ADCON0bits.CHS		= 0;	// Channel AN0
	ADCON1bits.ADFM		= 1;	// Result format right justified
	ADCON1bits.ACQT		= 2;	// A/D Acquisition time = 4uS
	ADCON1bits.ADCS		= 6;	// A/D conversion clock = FOSC/64
	ADCON0bits.ADON		= 1;	// Enable A/D converter module
}

void ConfigureInterrupts(void)
{
	// Configure external interrupt 1 (pushbutton = active LOW on INT1 mapped to RP2)
	INTCON2bits.INTEDG1	= 0;	// Falling edge (button)
	INTCON3bits.INT1IP	= 1;	// High priority
	INTCON3bits.INT1IE	= 1;	// Enable

	// Configure external interrupt 2 (proximity sensor = active HIGH on INT2 mapped to RP1)
	INTCON2bits.INTEDG2	= 1;	// Rising edge (proximity sensor)
	INTCON3bits.INT2IP	= 0;	// Low priority
	INTCON3bits.INT2IE	= 1;	// Enable

	// Configure Timer 0 overflow interrupt
	INTCON2bits.TMR0IP	= 0;	// Low priority
	INTCONbits.TMR0IE	= 1;	// Enable

	// Configure Timer 4 period match interrupt
	IPR3bits.TMR4IP	= 1;	// High priority
	PIE3bits.TMR4IE	= 1;	// Enable

	// Configure Timer 6 period match interrupt
	IPR5bits.TMR6IP	= 1;	// High priority
	PIE5bits.TMR6IE	= 1;	// Enable

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

	// Configure ADC interrupts
	IPR1bits.ADIP	= 1;	// High priority
	PIE1bits.ADIE	= 1;	// Enable

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
	char swapData[LINE_BUFFER_SIZE];

	// Initialize global variables
	CommPortInitialize(&_comm1,
					TX_BUFFER_SIZE, RX_BUFFER_SIZE, LINE_BUFFER_SIZE,
					&txData1, &rxData1, &lineData1,
					&SRAM_ADDR_COMM1_LINE_QUEUE, COMM1_LINE_QUEUE_SIZE,
					NEWLINE_CRLF, NEWLINE_CRLF,
					&_comm1Regs,
					false, false,
					COORD_VALUE_COMM1A.y, COORD_VALUE_COMM1A.x);
	CommPortInitialize(&_comm2,
					TX_BUFFER_SIZE, RX_BUFFER_SIZE, LINE_BUFFER_SIZE,
					&txData2, &rxData2, &lineData2,
					&SRAM_ADDR_COMM2_LINE_QUEUE, COMM2_LINE_QUEUE_SIZE,
					NEWLINE_CRLF, NEWLINE_CR,
					&_comm2Regs,
					true, false,
					COORD_VALUE_COMM2A.y, COORD_VALUE_COMM2A.x);
	_comm1.modeBits.echoRx = false;
	_comm2.modeBits.echoRx = true;
	ButtonInfoInitialize(&_button, ButtonPress, ButtonHold, ButtonRelease, 0);
	SramStatusInitialize();
	ShellInitialize(&_comm1, &_comm2, LINE_BUFFER_SIZE, swapData);
	InitializeLoadMeasurement();

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

	// Start tick timer (Timer 4) and ADC timer (Timer 6)
	T4CONbits.TMR4ON = true;
	T6CONbits.TMR6ON = true;
}

// BUTTON ACTIONS--------------------------------------------------------------

void ButtonPress(void)
{
	RelayControl(_relayState ? 0 : 1);
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
	RTCVALH = dateTime->date.Year.ByteValue;
	RTCVALL = dateTime->date.Day.ByteValue;
	RTCVALH = dateTime->date.Month.ByteValue;
	RTCVALL = dateTime->time.Hour.ByteValue;
	RTCVALH = dateTime->weekday;
	RTCVALL = dateTime->time.Second.ByteValue;
	RTCVALH = dateTime->time.Minute.ByteValue;
	RTCCFGbits.RTCWREN = false;
}

void GetDateTime(DateTime* dateTime)
{
	if(dateTime == NULL)
		return;

	//RTCCFGbits.RTCWREN = true;
	RTCCFGbits.RTCPTR1 = 1;
	RTCCFGbits.RTCPTR0 = 1;
	dateTime->date.Year.ByteValue	= RTCVALH;
	dateTime->date.Day.ByteValue	= RTCVALL;
	dateTime->date.Month.ByteValue	= RTCVALH;
	dateTime->time.Hour.ByteValue	= RTCVALL;
	dateTime->weekday				= RTCVALH;
	dateTime->time.Second.ByteValue	= RTCVALL;
	dateTime->time.Minute.ByteValue	= RTCVALH;
	//RTCCFGbits.RTCWREN = false;
}

// DEBUG FUNCTIONS-------------------------------------------------------------

void TestFunc1(void) {
	/*Buffer test8, test16;
	unsigned char testData8[8] = "ABC123AB";
	unsigned int testData16[8] = {0x0123, 0x4567, 0x89AB, 0xCDEF, 0x0123, 0x7788, 0x89AB, 0xCDEF};
	unsigned int valueA = 0x89AB;
	unsigned int valueB[2] = {0x89AB, 0xCDEF};
	unsigned int valueC = 0xFAFF;
	InitializeBuffer(&test8, 8, 1, testData8);
	InitializeBuffer(&test16, 8, 2, testData16);
	test8.length = 8;
	test16.length = 8;

	int sum = 0;
	test16 = BufferTrimLeft(&test16, 3);
	sum += test16.length;
	test16 = BufferTrimRight(&test16, 2);
	sum += test16.length;

	int sum = 0;
	int result = BufferFind(&test8, "A", 1, 0);
	sum += result;
	result = BufferFind(&test8, "A", 1, 1);
	sum += result;
	result = BufferFind(&test8, "AB", 2, 0);
	sum += result;
	result = BufferFind(&test8, "AB", 2, 1);
	sum += result;
	result = BufferFind(&test8, "CC", 2, 2);
	sum += result;
	result = BufferFind(&test16, &valueA, 1, 0);
	sum += result;
	result = BufferFind(&test16, &valueA, 1, 1);
	sum += result;
	result = BufferFind(&test16, &valueB, 2, 0);
	sum += result;
	result = BufferFind(&test16, &valueB, 2, 1);
	sum += result;
	result = BufferFind(&test16, &valueC, 1, 2);
	sum += result;
	LED = sum >= 0 ? 1 : 0;*/ }