/* Project:	SmartModule
 * File:	shell.c
 * Author:	Jonathan Ruisi
 * Created:	February 20, 2017, 3:39 PM
 */

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include "shell.h"
#include "main.h"
#include "sram.h"
#include "serial_comm.h"
#include "wifi.h"
#include "common_types.h"
#include "ringbuffer.h"

// GLOBAL VARIABLES------------------------------------------------------------

// INITIALIZATION FUNCTIONS----------------------------------------------------

// LINE PARSING FUNCTIONS -----------------------------------------------------

bool LineContains(Buffer* line, const char* str)
{
	int index = 0, offset = 0;
	while(index < line->length && line->data[index] != str[0])
	{
		index++;
	}
	while(index < line->length && line->data[index] == str[offset])
	{
		index++;
		offset++;
		if(str[offset] == ASCII_NUL)
			return true;
	}
	return false;
}

// ACTIONS --------------------------------------------------------------------

void LineToTerminal(CommPort* source)
{
	CommPutLine(source, &_comm2);
	source->lineBuffer.length = 0;
	source->statusBits.hasLine = false;
}

void LineToWifi(CommPort* source)
{
	CommPutLine(source, &_comm1);
	source->lineBuffer.length = 0;
	source->statusBits.hasLine = false;
}

// COMMANDS--------------------------------------------------------------------

Shell ShellCreate(CommPort* terminalComm, uint16_t commandBufferSize, char* commandBuffer)
{
	Shell shell;
	shell.terminal = terminalComm;
	shell.commandBuffer = BufferCreate(commandBufferSize, commandBuffer);
	return shell;
}