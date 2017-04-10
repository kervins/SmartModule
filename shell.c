/* Project:	SmartModule
 * File:	shell.c
 * Author:	Jonathan Ruisi
 * Created:	February 20, 2017, 3:39 PM
 */

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "shell.h"
#include "main.h"
#include "sram.h"
#include "serial_comm.h"
#include "wifi.h"
#include "utility.h"

// GLOBAL VARIABLES------------------------------------------------------------
Task _taskListData[SHELL_MAX_TASKS];

// SHELL COMMAND PROCESSOR FUNCTIONS ------------------------------------------

void ShellCommandProcessor(void)
{
#ifdef DEV_MODE_DEBUG
	DEBUG3 = ~DEBUG3;
#endif

	// Manage tasks (this is delayed by SHELL_RESET_DELAY upon a device reset)
	if(_tick > SHELL_RESET_DELAY)
		TaskScheduler();

	// Handle incoming data from terminal and backend
	if(_shell.server->external.lineQueue.length && !_sram.statusBits.busy)
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

void TaskScheduler(void)
{
	// If no tasks are currently running, check if more have been added to the task list
	if(_shell.task.current == NULL)
	{
		if(_shell.task.list.first == NULL)
			return;
		_shell.task.current = _shell.task.list.first;
	}

	// Run current task if valid to do so, otherwise the task has completed
	if((CURRENT_TASK->mode & TASK_FLAG_INFINITE) || CURRENT_TASK->runsRemaining > 0)
	{
		// Return if the run interval has not elapsed
		if((CURRENT_TASK->mode & TASK_FLAG_PERIODIC)
		&& CURRENT_TASK->lastRun != 0
		&& (_tick - CURRENT_TASK->lastRun < CURRENT_TASK->runInterval))
		{
			return;
		}

		// Run the task
		CURRENT_TASK->lastRun = _tick;
		CURRENT_TASK->action();
		CURRENT_TASK->runsRemaining--;
	}
	else
	{
		LinkedListNode* nextNode = _shell.task.current->next;
		LinkedListRemove(&_shell.task.list, _shell.task.current);
		_shell.task.current = nextNode;
	}

	// If the current task is not running exclusively, move to the next task
	if(!(CURRENT_TASK->mode & TASK_FLAG_EXCLUSIVE))
		_shell.task.current = _shell.task.current->next;
}

// COMMANDS -------------------------------------------------------------------

void ShellPrintDateTime(void)
{
	DateTime dt;
	GetDateTime(&dt);
	CommPort* port = (CommPort*) CURRENT_TASK->params[0];

	// Weekday
	switch(dt.Weekday)
	{
		case SUNDAY:
			CommPutString(port, "Sunday");
			break;
		case MONDAY:
			CommPutString(port, "Monday");
			break;
		case TUESDAY:
			CommPutString(port, "Tuesday");
			break;
		case WEDNESDAY:
			CommPutString(port, "Wednesday");
			break;
		case THURSDAY:
			CommPutString(port, "Thursday");
			break;
		case FRIDAY:
			CommPutString(port, "Friday");
			break;
		case SATURDAY:
			CommPutString(port, "Saturday");
			break;
	}
	CommPutString(port, ", ");

	// Month
	switch(dt.Month.ByteValue)
	{
		case JANUARY:
			CommPutString(port, "January");
			break;
		case FEBRUARY:
			CommPutString(port, "February");
			break;
		case MARCH:
			CommPutString(port, "March");
			break;
		case APRIL:
			CommPutString(port, "April");
			break;
		case MAY:
			CommPutString(port, "May");
			break;
		case JUNE:
			CommPutString(port, "June");
			break;
		case JULY:
			CommPutString(port, "July");
			break;
		case AUGUST:
			CommPutString(port, "August");
			break;
		case SEPTEMBER:
			CommPutString(port, "September");
			break;
		case OCTOBER:
			CommPutString(port, "October");
			break;
		case NOVEMBER:
			CommPutString(port, "November");
			break;
		case DECEMBER:
			CommPutString(port, "December");
			break;
	}
	CommPutChar(port, ' ');

	// Day and year
	CommPutChar(port, '0' + dt.Day.Tens);
	CommPutChar(port, '0' + dt.Day.Ones);
	CommPutString(port, ", 20");
	CommPutChar(port, '0' + dt.Year.Tens);
	CommPutChar(port, '0' + dt.Year.Ones);
	CommPutChar(port, ' ');

	// Time
	CommPutChar(port, '0' + dt.Hour.Tens);
	CommPutChar(port, '0' + dt.Hour.Ones);
	CommPutChar(port, ':');
	CommPutChar(port, '0' + dt.Minute.Tens);
	CommPutChar(port, '0' + dt.Minute.Ones);
	CommPutChar(port, ':');
	CommPutChar(port, '0' + dt.Second.Tens);
	CommPutChar(port, '0' + dt.Second.Ones);
	CommPutNewline(port);
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
}

void ShellDequeueLine(ExternalLineQueue* source, BufferU8* destination)
{
	if(_sram.statusBits.busy)
	{
		_shell.result.lastError = SHELL_ERROR_SRAM_BUSY;
		return;
	}
	else if(source->lineQueue.length == 0)
	{
		_shell.result.lastError = SHELL_ERROR_LINE_QUEUE_EMPTY;
		return;
	}

	uint24_t address = source->baseAddress + (source->blockSize *  source->lineQueue.tail);
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
	char* valueStr = ftoa(FIRMWARE_VERSION, &status);
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