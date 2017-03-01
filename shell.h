/* Project:	SmartModule
 * File:	shell.h
 * Author:	Jonathan Ruisi
 * Created:	February 20, 2017, 3:39 PM
 */

#ifndef SHELL_H
#define SHELL_H

#include <stdint.h>
#include "common_types.h"

// DEFINITIONS-----------------------------------------------------------------
#define LINE_TERMINATOR	ASCII_LF
#define LINE_LENGTH_MAX	200
#define PARAM_COUNT_MAX	16

// DEFINITIONS (ASCII CONTROL CHARACTERS)--------------------------------------
#define ASCII_NUL	0x00	// Null Character
#define ASCII_SOH	0x01	// Start of Heading
#define ASCII_STX	0x02	// Start of Text
#define ASCII_ETX	0x03	// End of Text
#define ASCII_EOT	0x04	// End of Transmission
#define ASCII_ENQ	0x05	// Enquiry
#define ASCII_ACK	0x06	// Acknowledgement
#define ASCII_BEL	0x07	// Bell
#define ASCII_BS	0x08	// Backspace
#define ASCII_HT	0x09	// Horizontal Tab
#define ASCII_LF	0x0A	// Line Feed
#define ASCII_VT	0x0B	// Vertical Tab
#define ASCII_FF	0x0C	// Form Feed
#define ASCII_CR	0x0D	// Carriage Return
#define ASCII_SO	0x0E	// Shift Out / X-On
#define ASCII_SI	0x0F	// Shift In / X-Off
#define ASCII_DLE	0x10	// Data Line Escape
#define ASCII_DC1	0x11	// Device Control 1 (XON)
#define ASCII_DC2	0x12	// Device Control 2
#define ASCII_DC3	0x13	// Device Control 3 (XOFF)
#define ASCII_DC4	0x14	// Device Control 4
#define ASCII_NAK	0x15	// Negative Acknowledgement
#define ASCII_SYN	0x16	// Synchronous Idle
#define ASCII_ETB	0x17	// End of Transmit Block
#define ASCII_CAN	0x18	// Cancel
#define ASCII_EM	0x19	// End of Medium
#define ASCII_SUB	0x1A	// Substitute
#define ASCII_ESC	0x1B	// Escape
#define ASCII_FS	0x1C	// File Separator
#define ASCII_GS	0x1D	// Group Separator
#define ASCII_RS	0x1E	// Record Separator
#define ASCII_US	0x1F	// Unit Separator

// TYPE DEFINITIONS------------------------------------------------------------

typedef struct
{
	uint16_t lineLength;
	char lineData[LINE_LENGTH_MAX];
	bool hasLine : 1;
} LineBuffer;

typedef struct
{
	char flag;
	int index;
	uint8_t length;
} SubstringInfo;

typedef struct
{
	uint8_t rxTarget;
	uint8_t txTarget;
	bool isEcho;
	bool isEchoWifi;
	bool isBusy;
	LineBuffer lineBuffer;
	Action currentAction;
	uint8_t paramCount;
	uint8_t paramStartIndex;	// Line buffer index where parameter text begins (including 1st delimeter)
	SubstringInfo paramList[PARAM_COUNT_MAX];
} Shell;

// GLOBAL VARIABLES------------------------------------------------------------
extern Shell _shell;

// FUNCTION PROTOTYPES---------------------------------------------------------
void ShellInitialize(uint8_t txTarget, uint8_t rxTarget, bool isEcho);
void ShellGetInput(void);
void ShellParseInput(void);
void ShellPrintNewLine(void);
void ShellPrintCommandLine(void);
void ShellEndExecution(void);
void ShellPrintInvalid(void);
void ShellPrintVersion(void);
void ShellWifiCommand(void);
#endif