/* Project:	SmartModule
 * File:	main.h
 * Author:	Jonathan Ruisi
 * Created:	December 16, 2016, 6:28 AM
 */

#ifndef MAIN_H
#define MAIN_H

#include "utility.h"

// DEVELOPMENT MODE DEFINITION-------------------------------------------------
//#define	DEV_MODE_DEBUG			// Compile debugging definitions and routines

// DEFINITIONS (OSCILLATOR)----------------------------------------------------
#define	FOSC	48000000L	// PLL generated system clock frequency	(48 MHz)
#define	FCY		FOSC/4		// Instruction cycle frequency			(12 MHz)
#define TCY		1L/FCY		// Instruction period					(83.33ns)

// DEFINITIONS (PERIPHERAL)----------------------------------------------------
#define ANALOG0			PORTAbits.RA0
#define ANALOG1			PORTAbits.RA1
#define ANALOG2			PORTAbits.RA2
#define ANALOG3			PORTAbits.RA3
#define PROX			PORTAbits.RP1
#define BUTTON			PORTAbits.RP2
#define LED				LATAbits.LATA6
#define WIFI_RST		LATAbits.LATA7
#define RELAY_RES		LATBbits.LATB0
#define RELAY_SET		LATBbits.LATB1
#define RAM_CS			LATBbits.LATB3
#define RAM_SCK			PORTBbits.RP8
#define RAM_MISO		PORTBbits.RP5
#define RAM_MOSI		PORTBbits.RP7
#define DISP_CS			LATCbits.LATC2
#define DISP_SCK		PORTCbits.SCK1
#define DISP_MISO		PORTCbits.SDI1
#define DISP_MOSI		PORTCbits.SDO1
#define PC_RX			PORTCbits.RP11
#define PC_TX			PORTCbits.RP12
#define WIFI_RX			PORTCbits.TX1
#define WIFI_TX			PORTCbits.RX1

// DEFINITIONS (DATA)----------------------------------------------------------
#define TX_BUFFER_SIZE			64
#define RX_BUFFER_SIZE			256
#define LINE_BUFFER_SIZE		RX_BUFFER_SIZE
#define COMM1_LINE_QUEUE_SIZE	16
#define COMM2_LINE_QUEUE_SIZE	16

// DEFINITIONS (OTHER)---------------------------------------------------------
#define FIRMWARE_VERSION	1.00
#define WIFI_MODULE			ESP8266_01
#define ROLE				2	// 0 = SmartSwitch, 1 = SensorHub, 2 = both

// CONSTANTS-------------------------------------------------------------------
static const char* _id	= "SM000001";
//static const char* _id	= "SM000002";

// GLOBAL VARIABLES------------------------------------------------------------
extern volatile unsigned long int _tick;
extern volatile struct ButtonInfo _button;
extern struct CommPort _comm1, _comm2;
extern const struct CommDataRegisters _comm1Regs, _comm2Regs;

// FUNCTION PROTOTYPES---------------------------------------------------------
// Initialization
void ConfigureOscillator(void);
void ConfigureWDT(void);
void ConfigurePorts(void);
void ConfigureTimers(void);
void ConfigureSPI(void);
void ConfigureUSART(void);
void ConfigureRTCC(void);
void ConfigureADC(void);
void ConfigureInterrupts(void);
void ConfigureOS(void);
// Button Actions
void ButtonPress(void);
void ButtonHold(void);
void ButtonRelease(void);
// Utility Functions
void SetDateTime(DateTime*);
void GetDateTime(DateTime*);
// Debug Functions
void TestFunc1(void);

//---------------------------------------------------------------------------------------------------------------------
/*______________________________________________________*/
/* SRAM ALLOCATION MAP (131072 bytes)					*/
/*														*/
/*			0123456789ABCDEF							*/
/*			0000000000000000							*/
/*			0000000000000000							*/
/*			________________							*/
/* 0x00000:	1111111111111111	<-- Comm1 Line Buffer	*/	SCUINT24 SRAM_ADDR_COMM1_LINE_QUEUE = 0x000000;
/* 0x01000: 2222222222222222	<-- Comm2 Line Buffer	*/	SCUINT24 SRAM_ADDR_COMM2_LINE_QUEUE = 0x010000;
/* 0x02000: LLLLLLLLLLLLLLLL	<-- Load History Buffer	*/	SCUINT24 SRAM_ADDR_LOAD_QUEUE		= 0x020000;
/* 0x03000: LLLLLLLLLLLLLLLL							*/
/* 0x04000: LLLLLLLLLLLLLLLL							*/
/* 0x05000: LLLLLLLLLLLLLLLL							*/
/* 0x06000: ................							*/
/* 0x07000: ................							*/
/* 0x08000: ................							*/
/* 0x09000: ................							*/
/* 0x0A000: ................							*/
/* 0x0B000: ................							*/
/* 0x0C000: ................							*/
/* 0x0D000: ................							*/
/* 0x0E000: ................							*/
/* 0x0F000: ................							*/
/* 0x10000: ................							*/
/* 0x11000: ................							*/
/* 0x12000: ................							*/
/* 0x13000: ................							*/
/* 0x14000: ................							*/
/* 0x15000: ................							*/
/* 0x16000: ................							*/
/* 0x17000: ................							*/
/* 0x18000: ................							*/
/* 0x19000: ................							*/
/* 0x1A000: ................							*/
/* 0x1B000: ................							*/
/* 0x1C000: ................							*/
/* 0x1D000: ................							*/
/* 0x1E000: ................							*/
/* 0x1F000: ................							*/
/*______________________________________________________*/

#endif