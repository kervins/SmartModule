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
#include "adc_rms.h"
#include "utility.h"

// GLOBAL VARIABLES------------------------------------------------------------
Task _taskListData[SHELL_MAX_TASKS];

// MAIN LOOP (SHELL) ----------------------------------------------------------

void ShellLoop(void)
{
	// Manage tasks (this is delayed by SHELL_RESET_DELAY upon a device reset)
	if(_tick > SHELL_RESET_DELAY)
		TaskScheduler();

	if(_shell.server->buffers.external.length && !_sram.statusBits.busy)
	{
		RingBufferDequeueSRAM(&_shell.server->buffers.external, &_shell.swapBuffer);
		while(_sram.statusBits.busy)
			continue;
		CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_VALUE_COMM1A.y, COORD_VALUE_COMM1A.x);
		CommPutSequence(_shell.terminal, ANSI_ELINE, 0);
		CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_VALUE_COMM1A.y, COORD_VALUE_COMM1A.x);
		CommPutString(_shell.terminal, (char*) _shell.swapBuffer.data);
		CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_VALUE_COMM1B.y, COORD_VALUE_COMM1B.x);
		CommPutSequence(_shell.terminal, ANSI_ELINE, 0);
		CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_VALUE_COMM1C.y, COORD_VALUE_COMM1C.x);
		CommPutSequence(_shell.terminal, ANSI_ELINE, 0);
		CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_VALUE_COMM1D.y, COORD_VALUE_COMM1D.x);
		CommPutSequence(_shell.terminal, ANSI_ELINE, 0);

		if(BufferContains(&_shell.swapBuffer, "WIFI GOT IP", 11) >= 0)
		{
			_wifi.statusBits.isSsidConnected = true;
			CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_VALUE_SSID_STATUS.y, COORD_VALUE_SSID_STATUS.x);
			CommPutString(_shell.terminal, "             ");
			CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_VALUE_SSID_STATUS.y, COORD_VALUE_SSID_STATUS.x);
			CommPutString(_shell.terminal, "Connected");
		}
		else if(BufferContains(&_shell.swapBuffer, "WIFI CONNECTED", 14) >= 0)
		{
			CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_VALUE_SSID_STATUS.y, COORD_VALUE_SSID_STATUS.x);
			CommPutString(_shell.terminal, "             ");
			CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_VALUE_SSID_STATUS.y, COORD_VALUE_SSID_STATUS.x);
			CommPutString(_shell.terminal, "Connecting...");
		}
		else if(BufferContains(&_shell.swapBuffer, "WIFI DISCONNECT", 15) >= 0)
		{
			_wifi.statusBits.isSsidConnected = false;
			_wifi.statusBits.isTcpConnected = false;
			CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_VALUE_SSID_STATUS.y, COORD_VALUE_SSID_STATUS.x);
			CommPutString(_shell.terminal, "             ");
			CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_VALUE_SSID_STATUS.y, COORD_VALUE_SSID_STATUS.x);
			CommPutString(_shell.terminal, "Disconnected");
			CommPutString(_shell.server, at_cwjap_cur);
			CommPutString(_shell.server, "=\"");
			CommPutString(_shell.server, network_ssid);
			if(network_use_password)
			{
				CommPutString(_shell.server, "\",\"");
				CommPutString(_shell.server, network_pass);
			}
			CommPutChar(_shell.server, '\"');
			CommPutNewline(_shell.server);
		}
		else if(BufferContains(&_shell.swapBuffer, "ERROR", 5) >= 0)
		{
			_shell.result.lastError = SHELL_ERROR_WIFI_COMMAND;
			_shell.result.values[0] = _tick;
		}
	}
	else if(_shell.terminal->buffers.external.length && !_sram.statusBits.busy)
	{
		RingBufferDequeueSRAM(&_shell.terminal->buffers.external, &_shell.swapBuffer);
		while(_sram.statusBits.busy)
			continue;
		CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_VALUE_ERROR.y, COORD_VALUE_ERROR.x);
		CommPutSequence(_shell.terminal, ANSI_ELINE, 0);
		CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_VALUE_COMM2A.y, COORD_VALUE_COMM2A.x);
		CommPutSequence(_shell.terminal, ANSI_ELINE, 0);
		CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_VALUE_COMM2B.y, COORD_VALUE_COMM2B.x);
		CommPutSequence(_shell.terminal, ANSI_ELINE, 0);
		CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_VALUE_COMM2C.y, COORD_VALUE_COMM2C.x);
		CommPutSequence(_shell.terminal, ANSI_ELINE, 0);
		CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_VALUE_COMM2D.y, COORD_VALUE_COMM2D.x);
		CommPutSequence(_shell.terminal, ANSI_ELINE, 0);
		ShellParseCommandLine(_shell.terminal);
	}

	if(_shell.result.lastWarning)
		ShellPrintLastWarning(32, 0);

	if(_shell.result.lastError)
		ShellPrintLastError(32, 0);
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
		_shell.result.values[0] = (uint32_t) CURRENT_TASK->action;
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

LinkedListNode* ShellAddTask(B_Action action,
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
	return _shell.task.list.last;
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
	InitializeBuffer(&_shell.swapBuffer, swapBufferSize, 1, swapBufferData);
	LinkedList_16Element_Initialize(&_shell.task.list, &_taskListData, sizeof(Task));

	// Print basic layout
	ShellPrintBasicLayout();

	// Add persistent tasks
	ShellAddTask(ShellPrintDateTime, 0, 1000, 0, false, true, true, 1, _shell.terminal);
	ShellAddTask(ShellPrintTick, 0, 125, 0, false, true, true, 1, _shell.terminal);
	ShellAddTask(ShellCalculateRMSCurrent, 0, 1000, 0, false, true, true, 0);
}

void ShellParseCommandLine(CommPort* comm)
{
	if(BufferContains(&_shell.swapBuffer, "WC:", 3) == 0)
	{
		CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_VALUE_CMD.y, COORD_VALUE_CMD.x);
		CommPutSequence(comm, ANSI_ELINE, 0);
		CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_VALUE_CMD.y, COORD_VALUE_CMD.x);
		CommPutString(_shell.terminal, (uint8_t*) _shell.swapBuffer.data);
		CommPutString(_shell.server, ((uint8_t*) _shell.swapBuffer.data) + 3);
		CommPutNewline(_shell.server);
	}
	else
	{
		_shell.result.lastError = SHELL_ERROR_COMMAND_NOT_RECOGNIZED;
	}
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
	CommPutString(_shell.terminal, "SSID: ");
	CommPutString(_shell.terminal, network_ssid);
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
	CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_LABEL_UPTIME.y, COORD_LABEL_UPTIME.x);
	CommPutString(_shell.terminal, "SYS TIME (ms):");
	CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_LABEL_COMM1A.y, COORD_LABEL_COMM1A.x);
	CommPutString(_shell.terminal, "COMM1>");
	CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_LABEL_COMM1B.y, COORD_LABEL_COMM1B.x);
	CommPutChar(_shell.terminal, '>');
	CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_LABEL_COMM1C.y, COORD_LABEL_COMM1C.x);
	CommPutChar(_shell.terminal, '>');
	CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_LABEL_COMM1D.y, COORD_LABEL_COMM1D.x);
	CommPutChar(_shell.terminal, '>');
	CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_LABEL_COMM2A.y, COORD_LABEL_COMM2A.x);
	CommPutString(_shell.terminal, "COMM2>");
	CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_LABEL_COMM2B.y, COORD_LABEL_COMM2B.x);
	CommPutChar(_shell.terminal, '>');
	CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_LABEL_COMM2C.y, COORD_LABEL_COMM2C.x);
	CommPutChar(_shell.terminal, '>');
	CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_LABEL_COMM2D.y, COORD_LABEL_COMM2D.x);
	CommPutChar(_shell.terminal, '>');
	CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_LABEL_CMD.y, COORD_LABEL_CMD.x);
	CommPutString(_shell.terminal, "CMD:");
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
		case SHELL_WARNING_FIFO_BUFFER_OVERWRITE:
		{
			ltoa(&valueStr, _shell.result.values[0], 16);
			CommPutString(_shell.terminal, "The FIFO buffer (0x");
			CommPutString(_shell.terminal, &valueStr);
			CommPutString(_shell.terminal, ") is full. At least one value has been overwritten.");
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
			CommPutString(_shell.terminal, "The task (0x");
			CommPutString(_shell.terminal, &valueStr);
			CommPutString(_shell.terminal, ") has timed out after ");
			ltoa(&valueStr, _shell.result.values[1], 10);
			CommPutString(_shell.terminal, &valueStr);
			CommPutString(_shell.terminal, "ms");
			break;
		}
		case SHELL_ERROR_NULL_REFERENCE:
		{
			CommPutString(_shell.terminal, "NULL reference");
			break;
		}
		case SHELL_ERROR_WIFI_COMMAND:
		{
			ultoa(&valueStr, _shell.result.values[0], 10);
			CommPutString(_shell.terminal, "WiFi module reported an error (system time = ");
			CommPutString(_shell.terminal, &valueStr);
			CommPutString(_shell.terminal, "ms)");
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
	switch(dt.weekday)
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
	CommPutChar(port, '0' + dt.date.Month.Tens);
	CommPutChar(port, '0' + dt.date.Month.Ones);
	CommPutChar(port, '/');
	CommPutChar(port, '0' + dt.date.Day.Tens);
	CommPutChar(port, '0' + dt.date.Day.Ones);
	CommPutChar(port, '/');
	CommPutChar(port, '0' + dt.date.Year.Tens);
	CommPutChar(port, '0' + dt.date.Year.Ones);

	// Time
	CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_VALUE_TIME.y, COORD_VALUE_TIME.x);
	CommPutChar(port, '0' + dt.time.Hour.Tens);
	CommPutChar(port, '0' + dt.time.Hour.Ones);
	CommPutChar(port, ':');
	CommPutChar(port, '0' + dt.time.Minute.Tens);
	CommPutChar(port, '0' + dt.time.Minute.Ones);
	CommPutChar(port, ':');
	CommPutChar(port, '0' + dt.time.Second.Tens);
	CommPutChar(port, '0' + dt.time.Second.Ones);
	return true;
}

// COMMANDS -------------------------------------------------------------------

bool ShellCalculateRMSCurrent(void)
{
	float rms = (float) CalculateCurrentRMS();
	CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_VALUE_LOAD.y, COORD_VALUE_LOAD.x);
	CommPutString(_shell.terminal, "            ");
	CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_VALUE_LOAD.y, COORD_VALUE_LOAD.x);
	if(rms > 1800.0)
	{
		switch(_adc.pinFloatAnimation)
		{
			case 0:
			{
				CommPutString(_shell.terminal, "Floating");
				_adc.pinFloatAnimation++;
				break;
			}
			case 1:
			{
				CommPutString(_shell.terminal, "fLoating");
				_adc.pinFloatAnimation++;
				break;
			}
			case 2:
			{
				CommPutString(_shell.terminal, "flOating");
				_adc.pinFloatAnimation++;
				break;
			}
			case 3:
			{
				CommPutString(_shell.terminal, "floAting");
				_adc.pinFloatAnimation++;
				break;
			}
			case 4:
			{
				CommPutString(_shell.terminal, "floaTing");
				_adc.pinFloatAnimation++;
				break;
			}
			case 5:
			{
				CommPutString(_shell.terminal, "floatIng");
				_adc.pinFloatAnimation++;
				break;
			}
			case 6:
			{
				CommPutString(_shell.terminal, "floatiNg");
				_adc.pinFloatAnimation++;
				break;
			}
			case 7:
			{
				CommPutString(_shell.terminal, "floatinG");
				_adc.pinFloatAnimation++;
				break;
			}
			default:
			{
				CommPutString(_shell.terminal, "floating");
				_adc.pinFloatAnimation = 0;
				break;
			}
		}
	}
	else
	{
		int status;
		unsigned char* rmsStr = ftoa(rms, &status);
		CommPutString(_shell.terminal, rmsStr);
		CommPutChar(_shell.terminal, 'W');
	}
	return true;
}