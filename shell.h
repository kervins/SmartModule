/* Project:	SmartModule
 * File:	shell.h
 * Author:	Jonathan Ruisi
 * Created:	February 20, 2017, 3:39 PM
 */

#ifndef SHELL_H
#define SHELL_H

#include "serial_comm.h"
#include "linked_list.h"

// DEFINITIONS-----------------------------------------------------------------
#define SHELL_MAX_RESULT_VALUES	4
#define SHELL_MAX_TASK_PARAMS	4

// DEFINITIONS (WARNINGS & ERRORS)---------------------------------------------
// Warnings
#define SHELL_WARNING_DATA_TRUNCATED			1
// Errors
#define SHELL_ERROR_SRAM_BUSY					1
#define SHELL_ERROR_ZERO_LENGTH					2
#define SHELL_ERROR_ADDRESS_RANGE				3
#define SHELL_ERROR_LINE_QUEUE_EMPTY			4
#define SHELL_ERROR_COMMAND_NOT_RECOGNIZED		5
#define SHELL_ERROR_TASK_TIMEOUT				6

// TYPE DEFINITIONS------------------------------------------------------------

typedef struct Task
{
	Action action;
	void* params[4];
	unsigned long int invocationTime;
	unsigned long int timeoutInterval;
} Task;

typedef struct Shell
{

	union
	{

		struct
		{
			unsigned : 8;
		} statusBits;
		unsigned char status;
	} ;

	struct
	{
		unsigned char lastWarning;
		unsigned char lastError;
		unsigned long int values[SHELL_MAX_RESULT_VALUES];
	} result;

	CommPort* server;
	CommPort* terminal;
	LinkedList_16Element taskList;
	Task currentTask;
	BufferU8 swapBuffer;
} Shell;

// GLOBAL VARIABLES------------------------------------------------------------
extern Shell _shell;
extern const unsigned short long int shellCommandAddress;

// FUNCTION PROTOTYPES---------------------------------------------------------
// Shell Command Processor
void ShellCommandProcessor(void);
void ShellHandleSequence(CommPort* comm);
void ShellParseCommandLine(void);
// Shell Management Functions
void ShellInitialize(CommPort* serverComm, CommPort* terminalComm,
					 unsigned int swapBufferSize, char* swapBufferData);
void ShellDequeueLine(ExternalLineQueue* source, BufferU8* destination);
void ShellPrintVersionInfo(void);
void ShellPrintLastWarning(void);
void ShellPrintLastError(void);
// Commands
void ShellCmdTest1(void);

#endif