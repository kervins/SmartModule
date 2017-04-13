/* Project:	SmartModule
 * File:	shell.c
 * Author:	Jonathan Ruisi
 * Created:	February 20, 2017, 3:39 PM
 */

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "shell.h"
#include "main.h"
#include "sram.h"
#include "serial_comm.h"
#include "wifi.h"
#include "utility.h"

// GLOBAL VARIABLES------------------------------------------------------------
Task _taskListData[SHELL_MAX_TASKS];

// MAIN LOOP (SHELL) ----------------------------------------------------------

void ShellLoop(void)
{
	// Manage tasks (this is delayed by SHELL_RESET_DELAY upon a device reset)
	if(_tick > SHELL_RESET_DELAY)
		TaskScheduler();

	// Handle incoming data from terminal and backend
	/*if(_shell.server->external.lineQueue.length && !_sram.statusBits.busy)
				{
					ShellDequeueLine(&_shell.server->external, &_shell.swapBuffer);
					while(_sram.statusBits.busy)
						continue;
					CommPutString(_shell.terminal, "WIFI: ");
					CommPutBuffer(_shell.terminal, &_shell.swapBuffer);
					CommPutNewline(_shell.terminal);
				}
				else if(_shell.terminal->external.lineQueue.length && !_sram.statusBits.busy)
				{
					ShellDequeueLine(&_shell.terminal->external, &_shell.swapBuffer);
					while(_sram.statusBits.busy)
						continue;
					ShellParseCommandLine();
				}*/

#ifdef DEV_MODE_DEBUG
	if(_shell.result.lastWarning)
		ShellPrintLastWarning(32, 0);

	if(_shell.result.lastError)
		ShellPrintLastError(32, 0);
#endif
}

// TASK MANAGEMENT-------------------------------------------------------------

void TaskScheduler(void)
{
	// If no tasks are currently running, check if more have been added to the task list
	if(_shell.task.current == NULL)
	{
		if(_shell.task.list.first == NULL)
			return;
		_shell.task.current = _shell.task.list.first;
	}

	if(CURRENT_TASK->statusBits.busy
	&& CURRENT_TASK->lastRun != 0
	&& CURRENT_TASK->timeout > 0
	&& _tick - CURRENT_TASK->lastRun > CURRENT_TASK->timeout)
	{
		_shell.result.values[0] = (unsigned long int) CURRENT_TASK->action;
		_shell.result.values[1] = _tick - CURRENT_TASK->lastRun;
		_shell.result.lastError = SHELL_ERROR_TASK_TIMEOUT;
		goto t_comp;
	}

	if(CURRENT_TASK->statusBits.modeInfinite || CURRENT_TASK->runsRemaining > 0)
	{
		// Return if the run interval has not elapsed
		if(CURRENT_TASK->statusBits.modePeriodic
		&& CURRENT_TASK->lastRun != 0
		&& (_tick - CURRENT_TASK->lastRun < CURRENT_TASK->runInterval))
			goto t_next;

		if(!CURRENT_TASK->statusBits.busy)
			CURRENT_TASK->lastRun = _tick;
		if(CURRENT_TASK->action())
		{
			CURRENT_TASK->statusBits.busy = false;
			if(!CURRENT_TASK->statusBits.modeInfinite)
				CURRENT_TASK->runsRemaining--;
		}
		else
			CURRENT_TASK->statusBits.busy = true;
	}
	else goto t_comp;

t_next:{
		// If the current task is not running exclusively, move to the next task
		if(!CURRENT_TASK->statusBits.modeExclusive)
		{
			if(_shell.task.current->next)
				_shell.task.current = _shell.task.current->next;
			else

				if(_shell.task.current != _shell.task.list.first)
				_shell.task.current = _shell.task.list.first;
		}
		return;
	}

t_comp:{
		LinkedListNode* nextNode = _shell.task.current->next;
		LinkedListRemove(&_shell.task.list, _shell.task.current);
		_shell.task.current = nextNode;
	}
}

void ShellAddTask(B_Action action,
				  unsigned int runCount, unsigned long int runInterval, unsigned long int timeout,
				  bool isExclusive, bool isInfinite, bool isPeriodic,
				  unsigned char paramCount, ...)
{

	Task task;
	task.action = action;
	task.lastRun = 0;
	task.runsRemaining = runCount;
	task.runInterval = runInterval;
	task.timeout = timeout;
	task.status = 0;
	task.statusBits.modeExclusive = isExclusive;
	task.statusBits.modeInfinite = isInfinite;
	task.statusBits.modePeriodic = isPeriodic;

	if(paramCount)
	{
		unsigned char i;
		va_list args;
		va_start(args, paramCount);
		for(i = 0; i < paramCount; i++)
		{
			task.params[i] = va_arg(args, void*);
		}
		va_end(args);
	}
	LinkedListInsert(&_shell.task.list, _shell.task.list.last, &task, false);
}

// SHELL MANAGEMENT------------------------------------------------------------

void ShellInitialize(CommPort* serverComm, CommPort* terminalComm,
					 uint16_t swapBufferSize, char* swapBufferData)
{

	_shell.status = 0;
	_shell.result.lastWarning = 0;
	_shell.result.lastError = 0;
	_shell.task.current = 0;
	_shell.server = serverComm;
	_shell.terminal = terminalComm;
	BufferU8Create(&_shell.swapBuffer, swapBufferSize, swapBufferData);
	LinkedList_16Element_Initialize(&_shell.task.list, &_taskListData, sizeof(Task));

	// Print basic layout
	ShellPrintBasicLayout();

	// Add persistent tasks
	ShellAddTask(ShellPrintDateTime, 0, 1000, 0, false, true, true, 1, _shell.terminal);
	ShellAddTask(ShellPrintTick, 0, 50, 0, false, true, true, 1, _shell.terminal);
}

void ShellParseCommandLine(void)
{
	if(BufferContains(&_shell.swapBuffer, "Reset"))
	{

		Reset();
	}
	_shell.swapBuffer.length = 0;
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

void ShellDequeueLine(ExternalRingBufferU8* source, BufferU8* destination)
{
	if(_sram.statusBits.busy)
	{
		_shell.result.lastError = SHELL_ERROR_SRAM_BUSY;
		return;
	}
	else if(source->buffer.length == 0)
	{
		_shell.result.lastError = SHELL_ERROR_LINE_QUEUE_EMPTY;
		return;
	}

	uint24_t address = source->baseAddress + (source->blockSize *  source->buffer.tail);
	uint8_t length = RingBufferDequeue(&source->buffer);

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

void ShellPrintBasicLayout(void)
{
	// Clear screen
	CommPutSequence(_shell.terminal, ANSI_EDISP, 1, 2);
	CommPutSequence(_shell.terminal, ANSI_CPOS, 0);

	// Print version info
	CommPutString(_shell.terminal, "SmartModule");
	CommPutNewline(_shell.terminal);
	CommPutString(_shell.terminal, "HW: Rev.2");
	CommPutNewline(_shell.terminal);
	CommPutString(_shell.terminal, "FW: v1.00");
	CommPutNewline(_shell.terminal);

	// Print borders
	unsigned char i;
	for(i = 0; i < 49; i++)
		CommPutChar(_shell.terminal, '_');
	CommPutSequence(_shell.terminal, ANSI_CPOS, 2, 7, 0);
	for(i = 0; i < 49; i++)
		CommPutChar(_shell.terminal, '_');
	CommPutSequence(_shell.terminal, ANSI_CPOS, 2, 1, 13);
	for(i = 0; i < 7; i++)
	{
		CommPutChar(_shell.terminal, '|');
		CommPutSequence(_shell.terminal, ANSI_CUD, 0);
		CommPutSequence(_shell.terminal, ANSI_CUB, 0);
	}
	CommPutSequence(_shell.terminal, ANSI_CPOS, 2, 1, 50);
	for(i = 0; i < 7; i++)
	{
		CommPutChar(_shell.terminal, '|');
		CommPutSequence(_shell.terminal, ANSI_CUD, 0);
		CommPutSequence(_shell.terminal, ANSI_CUB, 0);
	}

	// Print labels
	CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_LABEL_NAME.y, COORD_LABEL_NAME.x);
	CommPutString(_shell.terminal, "NAME");
	CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_LABEL_STATUS.y, COORD_LABEL_STATUS.x);
	CommPutString(_shell.terminal, "STATUS");
	CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_LABEL_SSID.y, COORD_LABEL_SSID.x);
	CommPutString(_shell.terminal, "SSID:");
	CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_LABEL_HOST.y, COORD_LABEL_HOST.x);
	CommPutString(_shell.terminal, "HOST:");
	CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_LABEL_RELAY.y, COORD_LABEL_RELAY.x);
	CommPutString(_shell.terminal, "RELAY:");
	CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_LABEL_PROX.y, COORD_LABEL_PROX.x);
	CommPutString(_shell.terminal, "PROX:");
	CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_LABEL_TEMP.y, COORD_LABEL_TEMP.x);
	CommPutString(_shell.terminal, "TEMP:");
	CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_LABEL_LOAD.y, COORD_LABEL_LOAD.x);
	CommPutString(_shell.terminal, "LOAD:");
	CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_LABEL_COMM1.y, COORD_LABEL_COMM1.x);
	CommPutString(_shell.terminal, "COMM1>");
	CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_LABEL_COMM2.y, COORD_LABEL_COMM2.x);
	CommPutString(_shell.terminal, "COMM2>");
	CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_LABEL_UPTIME.y, COORD_LABEL_UPTIME.x);
	CommPutString(_shell.terminal, "UPTIME (ms):");
}

void ShellPrintLastWarning(unsigned char row, unsigned char col)
{
	char valueStr[16];
	CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_VALUE_ERROR.y, COORD_VALUE_ERROR.x);
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

void ShellPrintLastError(unsigned char row, unsigned char col)
{
	char valueStr[16];
	CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_VALUE_ERROR.y, COORD_VALUE_ERROR.x);
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
			ltoa(&valueStr, _shell.result.values[1], 10);
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

// COMMANDS (TERMINAL OUTPUT)--------------------------------------------------

bool ShellPrintTick(void)
{
	CommPort* port = (CommPort*) CURRENT_TASK->params[0];
	CommPutSequence(port, ANSI_CPOS, 2, COORD_VALUE_UPTIME.y, COORD_VALUE_UPTIME.x);
	char tickStr[12];
	ltoa(&tickStr, _tick, 10);
	CommPutString(port, &tickStr);
	return true;
}

bool ShellPrintDateTime(void)
{
	DateTime dt;
	GetDateTime(&dt);
	CommPort* port = (CommPort*) CURRENT_TASK->params[0];

	// Weekday
	CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_VALUE_DATE.y, COORD_VALUE_DATE.x);
	switch(dt.Weekday)
	{
		case SUNDAY:
			CommPutString(port, "Sun ");
			break;
		case MONDAY:
			CommPutString(port, "Mon ");
			break;
		case TUESDAY:
			CommPutString(port, "Tue ");
			break;
		case WEDNESDAY:
			CommPutString(port, "Wed ");
			break;
		case THURSDAY:
			CommPutString(port, "Thu ");
			break;
		case FRIDAY:
			CommPutString(port, "Fri ");
			break;
		case SATURDAY:
			CommPutString(port, "Sat ");
			break;
	}

	// Date
	CommPutChar(port, '0' + dt.Month.Tens);
	CommPutChar(port, '0' + dt.Month.Ones);
	CommPutChar(port, '/');
	CommPutChar(port, '0' + dt.Day.Tens);
	CommPutChar(port, '0' + dt.Day.Ones);
	CommPutChar(port, '/');
	CommPutChar(port, '0' + dt.Year.Tens);
	CommPutChar(port, '0' + dt.Year.Ones);

	// Time
	CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_VALUE_TIME.y, COORD_VALUE_TIME.x);
	CommPutChar(port, '0' + dt.Hour.Tens);
	CommPutChar(port, '0' + dt.Hour.Ones);
	CommPutChar(port, ':');
	CommPutChar(port, '0' + dt.Minute.Tens);
	CommPutChar(port, '0' + dt.Minute.Ones);
	CommPutChar(port, ':');
	CommPutChar(port, '0' + dt.Second.Tens);
	CommPutChar(port, '0' + dt.Second.Ones);
	return true;
}

// COMMANDS -------------------------------------------------------------------

bool ShellWaitText(void)
{
	CommPort* source = (CommPort*) CURRENT_TASK->params[0];
	char* text = (char*) CURRENT_TASK->params[1];
	if(source->external.buffer.length && !_sram.statusBits.busy)
	{
		ShellDequeueLine(&source->external, &_shell.swapBuffer);
		while(_sram.statusBits.busy)
			continue;
		if(BufferEquals(&_shell.swapBuffer, text))
		{

			CommPutString(source, "MATCH!");
			CommPutNewline(source);
			return true;
		}
	}
	return false;
}