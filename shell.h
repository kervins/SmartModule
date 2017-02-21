/* Project:	SmartModule
 * File:	shell.h
 * Author:	Jonathan Ruisi
 * Created:	February 20, 2017, 3:39 PM
 */

#ifndef SHELL_H
#define SHELL_H

#include <stdint.h>

// DEFINITIONS-----------------------------------------------------------------
#define LINE_TERMINATOR 0x0A	// Line feed
#define LINE_LENGTH_MAX	80
#define ASCII_LF 0x0A
#define ASCII_CR 0x0D

// TYPE DEFINITIONS------------------------------------------------------------

typedef struct
{
	uint16_t lineLength;
	char lineData[LINE_LENGTH_MAX];
	uint8_t rxTarget;
	uint8_t txTarget;
	unsigned isEcho : 1;
	unsigned hasLine : 1;
	unsigned getLine : 1;
} Shell;

// GLOBAL VARIABLES------------------------------------------------------------
extern Shell _shell;

// FUNCTION PROTOTYPES---------------------------------------------------------
void ShellUpdateInput(void);
void ShellExecuteCommand(void);
void ShellPrintCommandLine(void);
void ShellPrintVersion(void);
#endif