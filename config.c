// PIC18F27J13 Configuration Bit Settings

// CONFIG1L
#pragma config WDTEN		= ON			// Watchdog Timer
#pragma config PLLDIV		= 2				// 96MHz PLL Prescaler Selection
#pragma config CFGPLLEN		= ON			// PLL Enable Configuration Bit
#pragma config STVREN		= ON			// Stack Overflow/Underflow Reset
#pragma config XINST		= OFF			// Extended Instruction Set

// CONFIG1H
#pragma config CP0			= OFF			// Code Protect

// CONFIG2L
#pragma config OSC			= INTOSCPLL		// Oscillator
#pragma config SOSCSEL		= DIG			// T1OSC/SOSC Power Selection Bits
#pragma config CLKOEC		= OFF			// EC Clock Out Enable Bit
#pragma config FCMEN		= ON			// Fail-Safe Clock Monitor
#pragma config IESO			= OFF			// Internal External Oscillator Switch Over Mode

// CONFIG2H
#pragma config WDTPS		= 32768			// Watchdog Postscaler

// CONFIG3L
#pragma config DSWDTOSC		= T1OSCREF		// DSWDT Clock Select
#pragma config RTCOSC		= T1OSCREF		// RTCC Clock Select
#pragma config DSBOREN		= ON			// Deep Sleep BOR
#pragma config DSWDTEN		= ON			// Deep Sleep Watchdog Timer
#pragma config DSWDTPS		= G2			// Deep Sleep Watchdog Postscaler

// CONFIG3H
#pragma config IOL1WAY		= ON			// IOLOCK One-Way Set Enable bit
#pragma config ADCSEL		= BIT12			// ADC 10 or 12 Bit Select
#pragma config PLLSEL		= PLL96			// PLL Selection Bit
#pragma config MSSP7B_EN	= MSK7			// MSSP address masking

// CONFIG4L
#pragma config WPFP			= PAGE_127		// Write/Erase Protect Page Start/End Location
#pragma config WPCFG		= OFF			// Write/Erase Protect Configuration Region

// CONFIG4H
#pragma config WPDIS		= OFF			// Write Protect Disable bit
#pragma config WPEND		= PAGE_WPFP		// Write/Erase Protect Region Select bit