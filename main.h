#ifndef MAIN_H
#define MAIN_H

// DEFINITIONS (OSCILLATOR)----------------------------------------------------
#define	FOSC	48000000L	// PLL generated system clock frequency	(48 MHz)
#define	FCY		FOSC/4		// Instruction cycle frequency			(12 MHz)
#define TCY		1L/FCY		// Instruction period					(83.33ns)

// DEFINITIONS (PERIPHERAL)----------------------------------------------------
#define GPIO0_IN		PORTCbits.RC0
#define GPIO1_IN		PORTCbits.RC1
#define GPIO2_IN		PORTCbits.RC2
#define GPIO3_IN		PORTCbits.RC3
#define GPIO0_OUT		LATCbits.LATC0
#define GPIO1_OUT		LATCbits.LATC1
#define GPIO2_OUT		LATCbits.LATC2
#define GPIO3_OUT		LATCbits.LATC3
#define ANALOG0			PORTAbits.AN0
#define ANALOG1			PORTAbits.AN1
#define ANALOG2			PORTAbits.AN2
#define ANALOG3			PORTAbits.AN3
#define SPI_SDI			PORTBbits.RP6
#define SPI_SCK			PORTBbits.RP7
#define SPI_SDO			PORTBbits.RP8
#define WIFI_EN			LATAbits.LATA5
#define WIFI_RST		LATAbits.LATA6
#define WIFI_RX			PORTCbits.TX1
#define WIFI_TX			PORTCbits.RX1
#define WIFI_GPIO0_IN	PORTCbits.RC5
#define WIFI_GPIO2_IN	PORTCbits.RC4
#define WIFI_GPIO0_OUT	LATCbits.LATC5
#define WIFI_GPIO2_OUT	LATCbits.LATC4
#define RELAY0			LATBbits.LATB1
#define RELAY1			LATBbits.LATB2
#define RAM_CS			LATAbits.LATA7
#define BUTTON			PORTBbits.INT0

// GLOBAL VARIABLES------------------------------------------------------------
extern volatile uint32_t _tick;

// FUNCTION PROTOTYPES---------------------------------------------------------
void InitializeOscillator(void);
void InitializePorts(void);
void InitializeTimers(void);
void InitializeInterrupts(void);

#endif