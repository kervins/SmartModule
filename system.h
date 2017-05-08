/* Project:	SmartModule
 * File:	system.h
 * Author:	Jonathan Ruisi
 * Created:	December 16, 2016, 6:28 AM
 */

#ifndef SYSTEM_H
#define SYSTEM_H

#include "utility.h"

// DEFINITIONS (OSCILLATOR)----------------------------------------------------
#define	FOSC	48000000L	/**< PLL generated system clock frequency (48 MHz) */
#define	FCY		FOSC/4		/**< Instruction cycle frequency (12 MHz) */
#define TCY		1L/FCY		/**< Instruction period	(83.33ns) */

// DEFINITIONS (PERIPHERAL)----------------------------------------------------
#define ANALOG0			PORTAbits.RA0	/**< SmartModule GPIO pin 0 */
#define ANALOG1			PORTAbits.RA1	/**< SmartModule GPIO pin 1 */
#define ANALOG2			PORTAbits.RA2	/**< SmartModule GPIO pin 2 */
#define ANALOG3			PORTAbits.RA3	/**< SmartModule GPIO pin 3 */
#define PROX			PORTAbits.RP1	/**< SmartModule input for the proximity detector */
#define BUTTON			PORTAbits.RP2	/**< SmartModule button */
#define LED				LATAbits.LATA6	/**< SmartModule LED */
#define WIFI_RST		LATAbits.LATA7	/**< ESP8266 (wifi) reset line */
#define RELAY_RES		LATBbits.LATB0	/**< SmartModule relay port: RESET signal */
#define RELAY_SET		LATBbits.LATB1	/**< SmartModule relay port: SET signal */
#define RAM_CS			LATBbits.LATB3	/**< N01S830HA (SRAM) chip select line */
#define RAM_SCK			PORTBbits.RP8	/**< N01S830HA (SRAM) SCK line */
#define RAM_MISO		PORTBbits.RP5	/**< N01S830HA (SRAM) MISO line */
#define RAM_MOSI		PORTBbits.RP7	/**< N01S830HA (SRAM) MOSI line */
#define DISP_CS			LATCbits.LATC2	/**< SmartModule expansion port chip select line */
#define DISP_SCK		PORTCbits.SCK1	/**< SmartModule expansion port SCK line */
#define DISP_MISO		PORTCbits.SDI1	/**< SmartModule expansion port MISO line */
#define DISP_MOSI		PORTCbits.SDO1	/**< SmartModule expansion port MOSI line */
#define PC_RX			PORTCbits.RP11	/**< SmartModule debug port RX line */
#define PC_TX			PORTCbits.RP12	/**< SmartModule debug port TX line */
#define WIFI_RX			PORTCbits.TX1	/**< ESP8266 (wifi) RX line */
#define WIFI_TX			PORTCbits.RX1	/**< ESP8266 (wifi) TX line */

// DEFINITIONS (DATA)----------------------------------------------------------
#define TX_BUFFER_SIZE			64				/**< Defines the size (in bytes) of all Comm TX buffers */
#define RX_BUFFER_SIZE			256				/**< Defines the size (in bytes) of all Comm RX buffers */
#define LINE_BUFFER_SIZE		RX_BUFFER_SIZE	/**< Defines the size (in bytes) of all Comm LINE buffers */
#define COMM1_LINE_QUEUE_SIZE	16				/**< Defines the number of lines that can be stored in external SRAM for Comm1 */
#define COMM2_LINE_QUEUE_SIZE	16				/**< Defines the number of lines that can be stored in external SRAM for Comm2 */

// FUNCTION PROTOTYPES---------------------------------------------------------
// Initialization Functions
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
// Utility Functions
void SetDateTime(DateTime*);
void GetDateTime(DateTime*);

#endif