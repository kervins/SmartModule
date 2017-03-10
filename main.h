/* Project:	SmartModule
 * File:	main.h
 * Author:	Jonathan Ruisi
 * Created:	December 16, 2016, 6:28 AM
 */

#ifndef MAIN_H
#define MAIN_H

#include "serial_comm.h"


// DEFINITIONS (OSCILLATOR)----------------------------------------------------
#define	FOSC	48000000L	// PLL generated system clock frequency	(48 MHz)
#define	FCY		FOSC/4		// Instruction cycle frequency			(12 MHz)
#define TCY		1L/FCY		// Instruction period					(83.33ns)

// DEFINITIONS (PERIPHERAL)----------------------------------------------------
#define ANALOG0			PORTAbits.AN0
#define ANALOG1			PORTAbits.AN1
#define ANALOG2			PORTAbits.AN2
#define ANALOG3			PORTAbits.AN3
#define BUTTON			PORTAbits.RP2
#define LED				LATAbits.LATA6
#define WIFI_RST		LATAbits.LATA7
#define RELAY0			LATBbits.LATB0
#define RELAY1			LATBbits.LATB1
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

// DEFINITIONS (OTHER)---------------------------------------------------------
#define FIRMWARE_VERSION 1.00

// GLOBAL VARIABLES------------------------------------------------------------
extern volatile uint32_t _tick;
extern CommPort _comm1, _comm2;
extern const CommDataRegisters _comm1Regs, _comm2Regs;

// FUNCTION PROTOTYPES---------------------------------------------------------
// Initialization
void InitializeOscillator(void);
void InitializeWDT(void);
void InitializePorts(void);
void InitializeTimers(void);
void InitializeSpi(void);
void InitializeUSART(void);
void InitializeRTCC(void);
void InitializeInterrupts(void);
void InitializeSystem(void);
// Button Actions
void ButtonPress(void);
void ButtonHold(void);
void ButtonRelease(void);

#endif