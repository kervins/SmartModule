/* Project:	SmartModule
 * File:	shell.c
 * Author:	Jonathan Ruisi
 * Created:	February 20, 2017, 3:39 PM
 */

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "shell.h"
#include "main.h"
#include "sram.h"
#include "serial_comm.h"
#include "wifi.h"
#include "utility.h"

// SHELL COMMAND PROCESSOR FUNCTIONS ------------------------------------------

void ShellCommandProcessor(void)
{
#ifdef DEV_MODE_DEBUG
	DEBUG3 = ~DEBUG3;
#endif

	if(_shell.currentTask.action)
	{
		;
	}

#ifdef DEV_MODE_DEBUG
	if(_shell.result.lastWarning)
		ShellPrintLastWarning();
	if(_shell.result.lastError)
		ShellPrintLastError();
#endif
}

void ShellParseCommandLine(void)
{
	;
}

void ShellHandleSequence(CommPort* comm)
{
	switch(comm->sequence.terminator)
	{
		case ANSI_CUU:
		{

			break;
		}
		case ANSI_CUD:
		{

			break;
		}
		case ANSI_CUF:
		{

			break;
		}
		case ANSI_CUB:
		{

			break;
		}
	}
	CommResetSequence(comm);
}

// COMMANDS -------------------------------------------------------------------

void ShellCmdTest1(void)
{
	;
}

// SHELL MANAGEMENT------------------------------------------------------------

void ShellInitialize(CommPort* serverComm, CommPort* terminalComm,
					 uint16_t swapBufferSize, char* swapBufferData)
{
	_shell.status = 0;
	_shell.result.lastWarning = 0;
	_shell.result.lastError = 0;
	_shell.currentTask.action = NULL;
	_shell.currentTask.invocationTime = 0;
	_shell.currentTask.timeoutInterval = 0;
	_shell.server = serverComm;
	_shell.terminal = terminalComm;
	LinkedList_16Element_Initialize(&_shell.taskList);
	BufferU8Create(&_shell.swapBuffer, swapBufferSize, swapBufferData);
}

void ShellDequeueLine(ExternalLineQueue* source, BufferU8* destination)
{
	if(_sram.busy)
	{
		_shell.result.lastError = SHELL_ERROR_SRAM_BUSY;
		return;
	}
	else if(source->lineQueue.length == 0)
	{
		_shell.result.lastError = SHELL_ERROR_LINE_QUEUE_EMPTY;
		return;
	}

	uint24_t address = source->baseAddress + (source->blockSize * source->lineQueue.tail);
	uint8_t length = RingBufferDequeue(&source->lineQueue);

	if(address > SRAM_CAPACITY)
	{
		_shell.result.values[0] = address;
		_shell.result.values[1] = 0;
		_shell.result.values[2] = SRAM_CAPACITY;
		_shell.result.lastError = SHELL_ERROR_ADDRESS_RANGE;
		return;
	}
	else if(length == 0)
	{
		_shell.result.values[0] = length;
		_shell.result.lastError = SHELL_ERROR_ZERO_LENGTH;
		return;
	}
	else if(length > destination->bufferSize)
	{
		_shell.result.values[0] = length;
		_shell.result.values[1] = destination->bufferSize;
		_shell.result.lastWarning = SHELL_WARNING_DATA_TRUNCATED;
		length = destination->bufferSize;
	}

	SramRead(address, length, destination);
}

void ShellPrintVersionInfo(void)
{
	int status = 0;
	char *valueStr = ftoa(FIRMWARE_VERSION, &status);
	CommPutString(_shell.terminal, "SmartModule");
	CommPutNewline(_shell.terminal);
	CommPutString(_shell.terminal, "Hardware Rev.2");
	CommPutNewline(_shell.terminal);
	CommPutString(_shell.terminal, "Firmware V");
	CommPutString(_shell.terminal, valueStr);
	CommPutNewline(_shell.terminal);
}

void ShellPrintLastWarning(void)
{
	char valueStr[16];
	CommPutString(_shell.terminal, "WARNING: ");
	switch(_shell.result.lastWarning)
	{
		case SHELL_WARNING_DATA_TRUNCATED:
		{
			CommPutString(_shell.terminal, "Data truncated (");
			ltoa(&valueStr, _shell.result.values[0], 10);
			CommPutString(_shell.terminal, &valueStr);
			CommPutString(_shell.terminal, "->");
			ltoa(&valueStr, _shell.result.values[1], 10);
			CommPutString(_shell.terminal, &valueStr);
			CommPutChar(_shell.terminal, ')');
			break;
		}
		default:
		{
			CommPutString(_shell.terminal, "UNDEFINED");
			break;
		}
	}
	_shell.result.lastWarning = 0;
}

void ShellPrintLastError(void)
{
	char valueStr[16];
	CommPutString(_shell.terminal, "ERROR: ");
	switch(_shell.result.lastError)
	{
		case SHELL_ERROR_SRAM_BUSY:
		{
			CommPutString(_shell.terminal, "SRAM busy");
			break;
		}
		case SHELL_ERROR_ZERO_LENGTH:
		{
			CommPutString(_shell.terminal, "Length = 0");
			break;
		}
		case SHELL_ERROR_ADDRESS_RANGE:
		{
			CommPutString(_shell.terminal, "Specified address (0x");
			ltoa(&valueStr, _shell.result.values[0], 16);
			CommPutString(_shell.terminal, &valueStr);
			CommPutString(_shell.terminal, ") is outside the valid range (0x");
			ltoa(&valueStr, _shell.result.values[1], 16);
			CommPutString(_shell.terminal, &valueStr);
			CommPutString(_shell.terminal, "-0x");
			ltoa(&valueStr, _shell.result.values[2], 16);
			CommPutString(_shell.terminal, &valueStr);
			CommPutChar(_shell.terminal, ')');
			break;
		}
		case SHELL_ERROR_LINE_QUEUE_EMPTY:
		{
			CommPutString(_shell.terminal, "Line queue is empty");
			break;
		}
		case SHELL_ERROR_COMMAND_NOT_RECOGNIZED:
		{
			CommPutString(_shell.terminal, "Command not recognized: ");
			CommPutString(_shell.terminal, _shell.swapBuffer.data);
			break;
		}
		case SHELL_ERROR_TASK_TIMEOUT:
		{
			ltoa(&valueStr, _shell.result.values[0], 16);
			CommPutString(_shell.terminal, "The task (@ 0x");
			CommPutString(_shell.terminal, &valueStr);
			CommPutString(_shell.terminal, ") has timed out after ");
			ltoa(&valueStr, _tick - _shell.result.values[1], 10);
			CommPutString(_shell.terminal, &valueStr);
			CommPutString(_shell.terminal, "ms");
			break;
		}
		default:
		{
			CommPutString(_shell.terminal, "UNDEFINED");
			break;
		}
	}
	_shell.result.lastError = 0;
}