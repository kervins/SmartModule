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
#include "serial_comm.h"
#include "sram.h"
#include "wifi.h"
#include "shell.h"
#include "utility.h"

// GLOBAL VARIABLES------------------------------------------------------------
volatile uint32_t _tick = 0;	// Global timekeeping variable (not related to RTCC)

// PROGRAM ENTRY---------------------------------------------------------------

void main(void)
{
	// TODO:  Make SRAM file system
	// TODO:  Make shell

	// Initialize device
	InitializeOscillator();
	InitializeWDT();
	InitializePorts();
	InitializeTimers();
	InitializeSpi();
	InitializeUSART();
	InitializeInterrupts();

	// Initialize data
	uint32_t prevTick = 0;
	_button = ButtonInfoCreate(ButtonPress, ButtonHold, ButtonRelease, 0);
	char txBufferData1[TX_BUFFER_SIZE] = {0};
	char txBufferData2[TX_BUFFER_SIZE] = {0};
	char rxBufferData1[RX_BUFFER_SIZE] = {0};
	char rxBufferData2[RX_BUFFER_SIZE] = {0};
	_txBuffer1 = RingBufferCreate(TX_BUFFER_SIZE, txBufferData1);
	_txBuffer2 = RingBufferCreate(TX_BUFFER_SIZE, txBufferData2);
	_rxBuffer1 = RingBufferCreate(RX_BUFFER_SIZE, rxBufferData1);
	_rxBuffer2 = RingBufferCreate(RX_BUFFER_SIZE, rxBufferData2);
	_commStatus1.status = 0;
	_commStatus2.status = 0;
	_commStatus1.statusBits.isRxFlowControl = false;
	_commStatus2.statusBits.isRxFlowControl = true;
	_sramStatus.dataLength = 0;
	_sramStatus.readAddress = 0;
	_sramStatus.writeAddress = 0;
	_sramStatus.status = 0;

	// Initialize SRAM mode: HOLD function disabled, burst write
	SramMode mode;
	mode.value = 0;
	mode.holdDisabled = true;
	mode.mode = SRAM_MODE_BURST;
	SramSetMode(mode);

	// Blank entire SRAM array (fill with 0x00)
	SramFill(0x000000, SRAM_CAPACITY, 0xFF);

	// Initialize shell
	InitializeShell();

	// Start tick timer (Timer 4)
	T4CONbits.TMR4ON = true;

	// Main program loop
	while(true)
	{
		// BUTTON------------------------------------------
		CheckButton(&_button);

		// USART-------------------------------------------
		if(_commStatus1.statusBits.isRxFlowControl)
			CheckFlowControlRx1();
		if(_commStatus2.statusBits.isRxFlowControl)
			CheckFlowControlRx2();

		// SRAM--------------------------------------------
		if(!_sramStatus.statusBits.isBusy)
		{
			if(_sramStatus.statusBits.isReading)
				SramReadContinue();
			if(_sramStatus.statusBits.isWriting)
				SramWriteContinue();
			if(_sramStatus.statusBits.isFilling)
				SramFillContinue();
		}

		if(!_sramStatus.statusBits.isBusy && _sramStatus.statusBits.hasUnreadData)
		{
			char number[8];
			ltoa(number, _sramStatus.readAddress - _sramStatus.dataLength, 16);
			printf("READ ADDRESS = 0x%s\n\r", number);
			int i;
			for(i = 0; i < _sramStatus.dataLength; i++)
			{
				putch2(_sramPacket.data[i]);
			}
			_sramStatus.statusBits.hasUnreadData = false;
		}

		// WIFI--------------------------------------------
		// 2 seconds after startup (or WiFi reset), release WiFi module from reset
		if(_wifiStatus == WIFI_STATUS_RESET_RELEASE && (_tick - _wifiTimestamp > 2000))
		{
			WIFI_RST = 1;
			_wifiStatus = WIFI_STATUS_BOOT1;
			_wifiTimestamp = _tick;
		}

		// After WiFi is released from reset, wait for WiFi initialization to complete,
		// then change USART1 baud rate from 76800 to 115200
		if(_wifiStatus == WIFI_STATUS_BOOT1 && (_tick - _wifiTimestamp > 100))
		{
			RCSTA1bits.SPEN	= false;
			SPBRGH1	= GET_BYTE(CALCULATE_BRG_16H(115200), 1);
			SPBRG1	= GET_BYTE(CALCULATE_BRG_16H(115200), 0);
			RCSTA1bits.SPEN	= true;
			_wifiStatus = WIFI_STATUS_BOOT2;
			_wifiTimestamp = _tick;
		}

		// SHELL-------------------------------------------
		if(_shell.hasLine)
		{
			ShellExecuteCommand();
		}
		else if(_shell.getLine)
			ShellUpdateInput();
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

void InitializeWDT(void)
{
	WDTCONbits.REGSLP	= 1;	// On-chip regulator enters low-power operation when device enters Sleep mode
	WDTCONbits.VBGOE	= 0;	// Band gap reference output is disabled
	WDTCONbits.ULPEN	= 0;	// Ultra low-power wake-up module is disabled
	WDTCONbits.ULPSINK	= 0;	// Ultra low-power wake-up current sink is disabled
	WDTCONbits.SWDTEN	= 0;	// Watchdog timer is off
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
	RPINR1	= 0x02;			// Assign External Interrupt 1		(INT1) to RP2	(PORTA<5>)
	RPINR21	= 0x05;			// Assign SPI2 Data Input			(SDI2) to RP5	(PORTB<2>)
	RPINR22	= 0x08;			// Assign SPI2 Clock Input			(SCK2IN) to RP8	(PORTB<5>)
	RPINR16	= 0x0C;			// Assign USART2 Async. Receive		(RX2) to RP12	(PORTC<1>)
	RPOR7	= 0x0A;			// Assign SPI2 Data Output			(SDO2) to RP7	(PORTB<4>)
	RPOR8	= 0x0B;			// Assign SPI2 Clock Output			(SCK2) to RP8	(PORTB<5>)
	RPOR11	= 0x06;			// Assign USART2 Async. Transmit	(TX2) to RP11	(PORTC<0>)
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

void InitializeUSART(void)
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

void InitializeRTCC(void)
{
	;
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

	// Configure USART1 interrupts
	IPR1bits.TX1IP	= 0;	// Low priority TX interrupt
	IPR1bits.RC1IP	= 0;	// Low priority RX interrupt
	//PIE1bits.TX1IE= 1;	// Enable TX interrupt **This is done in putch(), not here**
	PIE1bits.RC1IE	= 1;	// Enable RX interrupt

	// Configure USART2 interrupts
	IPR3bits.TX2IP	= 0;	// Low priority TX interrupt
	IPR3bits.RC2IP	= 0;	// Low priority RX interrupt
	//PIE3bits.TX2IE= 1;	// Enable TX interrupt **This is done in putch(), not here**
	PIE3bits.RC2IE	= 1;	// Enable RX interrupt

	// Configure SPI2 interrupts
	IPR3bits.SSP2IP	= 0;	// Low priority
	PIE3bits.SSP2IE	= 1;	// Enable

	// Enable interrupts
	RCONbits.IPEN	= 1;	// Set prioritized interrupt mode
	INTCONbits.GIEH	= 1;	// Enable high-priority interrupts
	INTCONbits.GIEL	= 1;	// Enable low-priority interrupts
}

void InitializeShell(void)
{
	_shell.rxTarget = 2;
	_shell.txTarget = 2;
	_shell.getLine = true;
	_shell.hasLine = false;
	_shell.isEcho = true;
	_shell.lineLength = 0;
	printf("SmartModule\n\r");
	ShellPrintVersion();
	putch('\n');
	putch('\r');
	putch('\n');
	putch('\r');
	ShellPrintCommandLine();
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