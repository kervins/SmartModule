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
			unsigned : 8;
		} statusBits;
		uint8_t status;
	} ;

	struct
	{
		uint8_t length;
		uint8_t head;
		uint8_t tail;
		Task tasks[SHELL_MAX_TASK_COUNT];
	} taskQueue;
	Task* currentTask;
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

#endif