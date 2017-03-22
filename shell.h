/* Project:	SmartModule
 * File:	shell.h
 * Author:	Jonathan Ruisi
 * Created:	February 20, 2017, 3:39 PM
 */

#ifndef SHELL_H
#define SHELL_H

#include <stdint.h>
#include "serial_comm.h"

// DEFINITIONS-----------------------------------------------------------------
#define SHELL_MAX_TASK_COUNT	16
#define SHELL_MAX_RESULT_VALUES	4
// Warnings
#define SHELL_WARNING_DATA_TRUNCATED	1
// Errors
#define SHELL_ERROR_SRAM_BUSY			1
#define SHELL_ERROR_ZERO_LENGTH			2
#define SHELL_ERROR_ADDRESS_RANGE		3
#define SHELL_ERROR_LINE_QUEUE_EMPTY			4

// TYPE DEFINITIONS------------------------------------------------------------

typedef struct Task
{
	Action action;				// Pointer to a function that returns void and has no parameters
	void* param;				// Optional parameter to be passed to the task
	uint16_t timeoutAfter;		// If the task has not successfully completed after this interval (ms), kill it
	uint32_t invocationTime;	// Timestamp indicating when the task was first run
} Task;

typedef struct Shell
{

	union
	{

		struct
		{
			unsigned busy : 1;
			unsigned : 7;
		} statusBits;
		uint8_t status;
	} ;

	struct
	{
		uint8_t lastWarning;
		uint8_t lastError;
		uint32_t values[SHELL_MAX_RESULT_VALUES];
	} result;

	CommPort* terminal;
	BufferU8 swapBuffer;
} Shell;

// GLOBAL VARIABLES------------------------------------------------------------
extern Shell _shell;
extern const uint24_t shellCommandAddress;

// FUNCTION PROTOTYPES---------------------------------------------------------
// Shell Command Processor
void ShellCommandProcessor(void);
// Shell Management Functions
void ShellInitialize(CommPort* terminalComm, uint16_t swapBufferSize, char* swapBufferData);
void ShellDequeueLine(ExternalLineQueue* source, BufferU8* destination);
void ShellPrintLastWarning(void);
void ShellPrintLastError(void);

#endif