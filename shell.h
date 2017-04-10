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
#define SHELL_MAX_TASKS			16
#define SHELL_RESET_DELAY		5000

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

// MACROS----------------------------------------------------------------------
#define CURRENT_TASK ((Task*) _shell.task.current->data)

// TYPE DEFINITIONS------------------------------------------------------------

typedef enum
{
	TASK_FLAG_EXCLUSIVE	= 1,
	TASK_FLAG_INFINITE	= 2,
	TASK_FLAG_PERIODIC	= 4
} TaskModeFlags;

typedef struct Task
{
	Action action;
	void* params[4];
	unsigned int runsRemaining;
	unsigned long int lastRun;
	unsigned long int runInterval;
	unsigned long int timeout;
	TaskModeFlags mode;
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

	struct
	{
		LinkedList_16Element list;
		LinkedListNode* current;
	} task;

	CommPort* server;
	CommPort* terminal;
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
void TaskScheduler(void);
// Shell Management Functions
void ShellInitialize(CommPort* serverComm, CommPort* terminalComm,
					 unsigned int swapBufferSize, char* swapBufferData);
void ShellDequeueLine(ExternalLineQueue* source, BufferU8* destination);
void ShellPrintVersionInfo(void);
void ShellPrintLastWarning(void);
void ShellPrintLastError(void);
// Commands
void ShellPrintDateTime(void);

#endif