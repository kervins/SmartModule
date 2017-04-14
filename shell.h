/* Project:	SmartModule
 * File:	shell.h
 * Author:	Jonathan Ruisi
 * Created:	February 20, 2017, 3:39 PM
 */

#ifndef SHELL_H
#define SHELL_H

#include "serial_comm.h"
#include "linked_list.h"
#include "utility.h"

// DEFINITIONS-----------------------------------------------------------------
#define SHELL_MAX_RESULT_VALUES	4
#define SHELL_MAX_TASK_PARAMS	4
#define SHELL_MAX_TASKS			16
#define SHELL_RESET_DELAY		5000

// DEFINITIONS (WARNINGS & ERRORS)---------------------------------------------
// Warnings
#define SHELL_WARNING_DATA_TRUNCATED			1
#define SHELL_WARNING_FIFO_BUFFER_OVERWRITE		2
// Errors
#define SHELL_ERROR_SRAM_BUSY					1
#define SHELL_ERROR_ZERO_LENGTH					2
#define SHELL_ERROR_ADDRESS_RANGE				3
#define SHELL_ERROR_LINE_QUEUE_EMPTY			4
#define SHELL_ERROR_COMMAND_NOT_RECOGNIZED		5
#define SHELL_ERROR_TASK_TIMEOUT				6
#define SHELL_ERROR_NULL_REFERENCE				7

// MACROS----------------------------------------------------------------------
#define CURRENT_TASK ((Task*) _shell.task.current->data)

// TYPE DEFINITIONS------------------------------------------------------------

typedef struct Task
{
	B_Action action;
	void* params[4];
	unsigned int runsRemaining;
	unsigned long int lastRun;
	unsigned long int runInterval;
	unsigned long int timeout;

	union
	{

		struct
		{
			unsigned modeExclusive : 1;
			unsigned modeInfinite : 1;
			unsigned modePeriodic : 1;
			unsigned busy : 1;
			unsigned : 4;
		} statusBits;
		unsigned char status;
	} ;
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
	Buffer swapBuffer;
} Shell;

// GLOBAL VARIABLES------------------------------------------------------------
extern Shell _shell;
const struct Point COORD_LABEL_UPTIME		= {52, 1};
const struct Point COORD_LABEL_NAME			= {20, 1};
const struct Point COORD_LABEL_STATUS		= {37, 1};
const struct Point COORD_LABEL_SSID			= {14, 2};
const struct Point COORD_LABEL_HOST			= {14, 3};
const struct Point COORD_LABEL_RELAY		= {14, 5};
const struct Point COORD_LABEL_PROX			= {15, 6};
const struct Point COORD_LABEL_TEMP			= {32, 5};
const struct Point COORD_LABEL_LOAD			= {32, 6};
const struct Point COORD_LABEL_COMM1A		= {1, 9};
const struct Point COORD_LABEL_COMM1B		= {6, 10};
const struct Point COORD_LABEL_COMM1C		= {6, 11};
const struct Point COORD_LABEL_COMM1D		= {6, 12};
const struct Point COORD_LABEL_COMM2A		= {1, 15};
const struct Point COORD_LABEL_COMM2B		= {6, 16};
const struct Point COORD_LABEL_COMM2C		= {6, 17};
const struct Point COORD_LABEL_COMM2D		= {6, 18};
const struct Point COORD_VALUE_UPTIME		= {52, 2};
const struct Point COORD_VALUE_DATE			= {0, 5};
const struct Point COORD_VALUE_TIME			= {5, 6};
const struct Point COORD_VALUE_SSID_NAME	= {20, 2};
const struct Point COORD_VALUE_SSID_STATUS	= {37, 2};
const struct Point COORD_VALUE_HOST_NAME	= {20, 3};
const struct Point COORD_VALUE_HOST_STATUS	= {37, 3};
const struct Point COORD_VALUE_RELAY		= {21, 5};
const struct Point COORD_VALUE_PROX			= {21, 6};
const struct Point COORD_VALUE_TEMP			= {32, 5};
const struct Point COORD_VALUE_LOAD			= {32, 6};
const struct Point COORD_VALUE_ERROR		= {1, 32};
const struct Point COORD_VALUE_COMM1A		= {8, 9};
const struct Point COORD_VALUE_COMM1B		= {8, 10};
const struct Point COORD_VALUE_COMM1C		= {8, 11};
const struct Point COORD_VALUE_COMM1D		= {8, 12};
const struct Point COORD_VALUE_COMM2A		= {8, 15};
const struct Point COORD_VALUE_COMM2B		= {8, 16};
const struct Point COORD_VALUE_COMM2C		= {8, 17};
const struct Point COORD_VALUE_COMM2D		= {8, 18};

// FUNCTION PROTOTYPES---------------------------------------------------------
void ShellLoop(void);
// Task Management
void TaskScheduler(void);
void ShellAddTask(B_Action action,
				  unsigned int runCount, unsigned long int runInterval, unsigned long int timeout,
				  bool isExclusive, bool isInfinite, bool isPeriodic,
				  unsigned char paramCount, ...);
// Shell Management
void ShellInitialize(CommPort* serverComm, CommPort* terminalComm,
					 unsigned int swapBufferSize, char* swapBufferData);
void ShellParseCommandLine(void);
void ShellHandleSequence(CommPort* comm);
void ShellPrintBasicLayout(void);
void ShellPrintLastWarning(unsigned char row, unsigned char col);
void ShellPrintLastError(unsigned char row, unsigned char col);
void ShellDequeueLine(ExternalRingBufferU8* source, Buffer* destination);
// Commands
bool ShellWaitText(void);
bool ShellPrintTick(void);
bool ShellPrintDateTime(void);

#endif