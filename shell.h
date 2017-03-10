/* Project:	SmartModule
 * File:	shell.h
 * Author:	Jonathan Ruisi
 * Created:	February 20, 2017, 3:39 PM
 */

#ifndef SHELL_H
#define SHELL_H

#include <stdint.h>
#include "common_types.h"
#include "serial_comm.h"

// TYPE DEFINITIONS------------------------------------------------------------

typedef struct Shell
{
	CommPort* terminal;
	Buffer commandBuffer;
} Shell;

// GLOBAL VARIABLES------------------------------------------------------------
extern Shell _shell;

// FUNCTION PROTOTYPES---------------------------------------------------------
// Initialization Functions
Shell ShellCreate(CommPort* terminalComm, uint16_t commandBufferSize, char* commandBuffer);
// Parsing Functions
bool LineContains(Buffer* line, const char* str);
// Actions
void LineToTerminal(CommPort* source);
void LineToWifi(CommPort* source);

#endif