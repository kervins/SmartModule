/**@file		main.c
 * @brief		Implementation of the RTOS, task scheduler, and SmartModule operations
 * @author		Jonathan Ruisi
 * @version		1.0
 * @date		December 16, 2016
 * @copyright	GNU Public License
 */

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include "main.h"
#include "system.h"
#include "button.h"
#include "serial_comm.h"
#include "sram.h"
#include "wifi.h"
#include "linked_list.h"
#include "utility.h"

// GLOBAL VARIABLES------------------------------------------------------------
volatile uint32_t _tick = 0;	/**< Global timekeeping variable (not related to RTCC)*/
volatile ButtonInfo _button;	/**< The main SmartModule button */
volatile Sram _sram;			/**< Main SRAM control structure */
CommPort _comm1, _comm2;		/**< USART1 and USART2 (wifi and debug terminal) control structures */
const CommDataRegisters _comm1Regs = {&TXREG1, (TXSTAbits_t*) & TXSTA1, &PIE1, 4};
const CommDataRegisters _comm2Regs = {&TXREG2, (TXSTAbits_t*) & TXSTA2, &PIE3, 4};
WifiInfo _wifi;					/**< Main WIFI control structure */
Shell _shell;					/**< Main SHELL control structure */
Task _taskListData[SHELL_MAX_TASKS];
AdcRmsInfo _adc;				/**< ADC measurement control structure */
uint16_t _adcData[ADC_WINDOW_SIZE];
unsigned char _relayState;		/**< Current state of the relay */
ProxDetectInfo _prox;			/**< Proximity detection information structure */

// PROGRAM ENTRY & MAIN LOOP---------------------------------------------------

/**
 * Program entry point and main loop
 */
void main(void)
{
	// Initialize device
	ConfigureOscillator();
	ConfigureWDT();
	ConfigurePorts();
	ConfigureTimers();
	ConfigureSPI();
	ConfigureUSART();
	ConfigureRTCC();
	ConfigureADC();
	ConfigureInterrupts();
	ConfigureOS();

	// Main program loop
main_loop:
	// BUTTON------------------------------------------
	UpdateButton(&_button);

	// USART-------------------------------------------
	UpdateCommPort(&_comm1);
	UpdateCommPort(&_comm2);

	// WIFI--------------------------------------------
	UpdateWifi();

	// SHELL-------------------------------------------
	UpdateShell();
	goto main_loop;
}

// SHELL UPDATE FUNCTION-------------------------------------------------------

/**
 * This function performs all related shell operations based on the current state of the RTOS and any received commands.
 * A call to this function must be placed in the main program loop.
 * @see main()
 */
void UpdateShell(void)
{
	// Manage tasks (this is delayed by SHELL_RESET_DELAY upon a device reset)
	if(_tick > SHELL_RESET_DELAY)
		TaskScheduler();

	if(_shell.server->buffers.external.length && !_sram.statusBits.busy)
	{
		RingBufferDequeueSRAM(&_shell.server->buffers.external, &_shell.swapBuffer);
		while(_sram.statusBits.busy)
			continue;

		if((BufferContains(&_shell.swapBuffer, "OK", 2) == 0)
		|| (BufferContains(&_shell.swapBuffer, "ERROR", 2) == 0))
		{
			CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_VALUE_COMM1B.y, COORD_VALUE_COMM1B.x);
			CommPutSequence(_shell.terminal, ANSI_ELINE, 0);
			CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_VALUE_COMM1B.y, COORD_VALUE_COMM1B.x);
			CommPutString(_shell.terminal, (char*) _shell.swapBuffer.data);
		}
		else
		{
			CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_VALUE_COMM1A.y, COORD_VALUE_COMM1A.x);
			CommPutSequence(_shell.terminal, ANSI_ELINE, 0);
			CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_VALUE_COMM1B.y, COORD_VALUE_COMM1B.x);
			CommPutSequence(_shell.terminal, ANSI_ELINE, 0);
			CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_VALUE_COMM1A.y, COORD_VALUE_COMM1A.x);
			CommPutString(_shell.terminal, (char*) _shell.swapBuffer.data);
		}
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
			_wifi.eventTime = _tick;

			// Send command to connect to TCP server
			CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_VALUE_HOST_STATUS.y, COORD_VALUE_HOST_STATUS.x);
			CommPutString(_shell.terminal, "             ");
			CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_VALUE_HOST_STATUS.y, COORD_VALUE_HOST_STATUS.x);
			CommPutString(_shell.terminal, "Connecting...");
			ShellAddTask(TaskConnectTcp, 1, 0, 0, false, false, false, 0);
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
			_wifi.statusBits.tcpConnectionStatus = WIFI_TCP_CLOSED;
			CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_VALUE_SSID_STATUS.y, COORD_VALUE_SSID_STATUS.x);
			CommPutString(_shell.terminal, "             ");
			CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_VALUE_SSID_STATUS.y, COORD_VALUE_SSID_STATUS.x);
			CommPutString(_shell.terminal, "Disconnected");
			ShellAddTask(TaskConnectNetwork, 1, 0, 0, false, false, false, 0);
		}
		else if(_wifi.statusBits.isSsidConnected &&
				_wifi.statusBits.tcpConnectionStatus == WIFI_TCP_CONNECTING &&
				BufferContains(&_shell.swapBuffer, "OK", 2) == 0)
		{
			_wifi.statusBits.tcpConnectionStatus = WIFI_TCP_READY;
			CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_VALUE_HOST_STATUS.y, COORD_VALUE_HOST_STATUS.x);
			CommPutString(_shell.terminal, "             ");
			CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_VALUE_HOST_STATUS.y, COORD_VALUE_HOST_STATUS.x);
			CommPutString(_shell.terminal, "Ready");
		}
		else if(_wifi.statusBits.isSsidConnected &&
				BufferContains(&_shell.swapBuffer, "CLOSED", 6) >= 0)
		{
			_wifi.statusBits.tcpConnectionStatus = WIFI_TCP_CLOSED;
			CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_VALUE_HOST_STATUS.y, COORD_VALUE_HOST_STATUS.x);
			CommPutString(_shell.terminal, "             ");
			CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_VALUE_HOST_STATUS.y, COORD_VALUE_HOST_STATUS.x);
			CommPutString(_shell.terminal, "Closed");
			//ShellAddTask(ShellConnectTcp, 1, 0, 0, false, false, false, 0);
			//_wifi.eventTime = _tick;
		}
		else if(BufferContains(&_shell.swapBuffer, "ERROR", 5) >= 0)
		{
			_shell.result.lastError = SHELL_ERROR_WIFI_COMMAND;
			_shell.result.values[0] = _tick;
		}
		else if(BufferContains(&_shell.swapBuffer, "#", 1) == 0)
		{
			ShellParseCommandLine(&_shell.swapBuffer);
		}
		_shell.swapBuffer.length = 0;
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
		ShellParseCommandLine(&_shell.swapBuffer);
		_shell.swapBuffer.length = 0;
	}

	if(_shell.result.lastWarning)
		ShellPrintLastWarning(32, 0);

	if(_shell.result.lastError)
		ShellPrintLastError(32, 0);
}

// SHELL MANAGEMENT FUNCTIONS--------------------------------------------------

/**
 * Initializes the shell environment
 * @param serverComm		Pointer to a <code>CommPort</code> structure which is defined for the WIFI module
 * @param terminalComm		Pointer to a <code>CommPort</code> structure which is defined for the debug terminal
 * @param swapBufferSize	The capacity (in bytes) of the shell's swap buffer
 * @param swapBufferData	Pointer to an array allocated for swap buffer data
 */
void ShellInitialize(CommPort* serverComm, CommPort* terminalComm,
					 uint16_t swapBufferSize, char* swapBufferData)
{

	_shell.result.lastWarning = 0;
	_shell.result.lastError = 0;
	_shell.task.current = 0;
	_shell.server = serverComm;
	_shell.terminal = terminalComm;
	InitializeBuffer(&_shell.swapBuffer, swapBufferSize, 1, swapBufferData);
	LinkedList_16Element_Initialize(&_shell.task.list, &_taskListData, sizeof(Task));

	// Print basic layout
	ShellPrintBasicLayout();

	// Add one-shot tasks
	ShellAddTask(TaskUpdateRelayStatus, 1, 0, 0, false, false, false, 0);

	// Add persistent tasks
	ShellAddTask(TaskPrintDateTime, 0, 1000, 0, false, true, true, 1, _shell.terminal);
	ShellAddTask(TaskPrintTick, 0, 125, 0, false, true, true, 1, _shell.terminal);
	ShellAddTask(TaskCalculateRMSCurrent, 0, 500, 0, false, true, true, 0);
	ShellAddTask(TaskUpdateProximityStatus, 0, 2000, 0, false, true, true, 0);
	ShellAddTask(TaskPrintTemp, 0, 10000, 0, false, true, true, 0);
}

/**
 * Parses the buffer for any commands it recognizes
 * @param buffer Pointer to a <code>Buffer</code> containing the data to parse
 */
void ShellParseCommandLine(Buffer* buffer)
{
	if(BufferContains(buffer, "WC:", 3) == 0)
	{
		CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_VALUE_CMD.y, COORD_VALUE_CMD.x);
		CommPutSequence(_shell.terminal, ANSI_ELINE, 0);
		CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_VALUE_CMD.y, COORD_VALUE_CMD.x);
		CommPutString(_shell.terminal, (uint8_t*) buffer->data);
		CommPutString(_shell.server, ((uint8_t*) buffer->data) + 3);
		CommPutNewline(_shell.server);
	}
	else if(BufferContains(buffer, "#", 1) == 0)
	{
		_shell.swapBuffer = BufferTrimLeft(&_shell.swapBuffer, 1);
		if(BufferContains(buffer, "tcpStart", 8) == 0)
		{
			CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_VALUE_HOST_STATUS.y, COORD_VALUE_HOST_STATUS.x);
			CommPutString(_shell.terminal, "             ");
			CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_VALUE_HOST_STATUS.y, COORD_VALUE_HOST_STATUS.x);
			CommPutString(_shell.terminal, "Connecting...");
			ShellAddTask(TaskConnectTcp, 1, 0, 0, false, false, false, 0);
		}
		else if(BufferContains(buffer, "SRLS:", 5) == 0)
		{
			CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_VALUE_CMD.y, COORD_VALUE_CMD.x);
			CommPutSequence(_shell.terminal, ANSI_ELINE, 0);
			CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_VALUE_CMD.y, COORD_VALUE_CMD.x);
			CommPutString(_shell.terminal, (uint8_t*) buffer->data);

			if(BufferContains(buffer, "0", 1) == 5)
				RelayControl(0);
			else if(BufferContains(buffer, "1", 1) == 5)
				RelayControl(1);
		}
	}
	else
		_shell.result.lastError = SHELL_ERROR_COMMAND_NOT_RECOGNIZED;
}

/**
 * Callback function for handling any ANSI control sequences received on a specified COMM port
 * @param comm Pointer to a <code>CommPort</code>
 * @warning this function currently contains no actions - these must be implemented
 */
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

/**
 * Prints the basic layout of the debug environment to the debug terminal
 */
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
	CommPutString(_shell.terminal, "HOST: ");
	CommPutString(_shell.terminal, tcp_server);
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

/**
 * Prints the last warning to the debug terminal at the location specified
 * @param row	Terminal row at which the output will be printed
 * @param col	Terminal column at which the output will be printed
 */
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

/**
 * Prints the last error to the debug terminal at the location specified
 * @param row	Terminal row at which the output will be printed
 * @param col	Terminal column at which the output will be printed
 */
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

// TASK MANAGEMENT FUNCTIONS---------------------------------------------------

/**
 * The main task scheduler function.
 * This function must be called either directly or indirectly from the main program loop
 * @see UpdateShell
 */
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

/**
 * Adds a new task to the task scheduler
 * @param action		The task function (must have no parameters and return <code>bool</code>)
 * @param runCount		Total number of times the task is to run (0 if not applicable)
 * @param runInterval	Interval (in milliseconds) at which the task is executed
 * @param timeout		Length of time before a task is considered to have timed out (fails to return <b>true</b>)
 * @param isExclusive	Whether or not the task has exclusive priority
 * @param isInfinite	Whether or not the task runs infinitely
 * @param isPeriodic	Whether or not the task runs periodically
 * @param paramCount	Number of parameters to be passed to the task function
 * @param ...			List of parameters to be passed to the task function
 * @return				A pointer to the <code>LinkedListNode</code> containing the task information
 */
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

// TASKS-----------------------------------------------------------------------

/**
 * Prints the current OS tick value to the terminal
 * @return true if successful, false if failed
 */
bool TaskPrintTick(void)
{
	CommPort* port = (CommPort*) CURRENT_TASK->params[0];
	CommPutSequence(port, ANSI_CPOS, 2, COORD_VALUE_UPTIME.y, COORD_VALUE_UPTIME.x);
	char tickStr[12];
	ltoa(&tickStr, _tick, 10);
	CommPutString(port, &tickStr);
	return true;
}

/**
 * Prints the current date and time to the terminal
 * @return true if successful, false if failed
 */
bool TaskPrintDateTime(void)
{
	DateTime dt;
	GetDateTime(&dt);
	dt.date.Year.Tens = 1;	// HARD CODED = FIGURE OUT WHY year is always '00
	dt.date.Year.Ones = 7;	// HARD CODED = FIGURE OUT WHY year is always '00
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

/**
 * Calculates the RMS current
 * @return true if successful, false if failed
 */
bool TaskCalculateRMSCurrent(void)
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
		unsigned char valueStr[6];
		CommPutString(_shell.terminal, rmsStr);
		CommPutChar(_shell.terminal, 'W');

		if(_wifi.statusBits.tcpConnectionStatus == WIFI_TCP_READY)
		{
			CommPutString(_shell.server, at_cipsend);
			CommPutChar(_shell.server, '=');
			itoa(&valueStr, strlen(rmsStr), 10);
			CommPutString(_shell.server, valueStr);
			CommPutNewline(_shell.server);
			Delay10KTCYx(120);	// Delay for 100ms (quick and dirty)
			CommPutString(_shell.server, rmsStr);
			CommPutNewline(_shell.server);
		}
	}
	return true;
}

/**
 * Updates the status of the relay
 * @return true if successful, false if failed
 */
bool TaskUpdateRelayStatus(void)
{
	CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_VALUE_RELAY.y, COORD_VALUE_RELAY.x);
	CommPutString(_shell.terminal, "      ");
	CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_VALUE_RELAY.y, COORD_VALUE_RELAY.x);

	if(_relayState)
		CommPutString(_shell.terminal, "CLOSED");
	else
		CommPutString(_shell.terminal, "OPEN");
	return true;
}

/**
 * Updates the status of the proximity sensor
 * @return true if successful, false if failed
 */
bool TaskUpdateProximityStatus(void)
{
	if(!_prox.isTripped)
		return true;

	unsigned char numStr[6];
	CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_VALUE_PROX.y, COORD_VALUE_PROX.x);
	CommPutString(_shell.terminal, "     ");
	CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_VALUE_PROX.y, COORD_VALUE_PROX.x);
	itoa(&numStr, _prox.count, 10);
	CommPutString(_shell.terminal, numStr);
	_prox.isTripped = false;
	return true;
}

/**
 * Prints the current temperature to the terminal
 * @return true if successful, false if failed
 */
bool TaskPrintTemp(void)
{
	int status;
	unsigned char* tempStr = ftoa(70.0 + (float) (rand() % 5)*(0.1), &status);
	CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_VALUE_TEMP.y, COORD_VALUE_TEMP.x);
	CommPutString(_shell.terminal, "     ");
	CommPutSequence(_shell.terminal, ANSI_CPOS, 2, COORD_VALUE_TEMP.y, COORD_VALUE_TEMP.x);
	tempStr[4] = 0;
	CommPutString(_shell.terminal, tempStr);
	CommPutString(_shell.terminal, "°F");
	return true;
}

/**
 * Initiates a connection of the specified network
 * @return true if successful, false if failed
 */
bool TaskConnectNetwork(void)
{
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
	return true;
}

/**
 * Initiates a TCP client connection to the specified host
 * @return true if successful, false if failed
 */
bool TaskConnectTcp(void)
{
	if(_tick - _wifi.eventTime < 2000)
		return false;

	unsigned char numStr[6];
	itoa(&numStr, tcp_port, 10);
	CommPutString(_shell.server, at_cipstart);
	CommPutString(_shell.server, "=\"TCP\",\"");
	CommPutString(_shell.server, tcp_server);
	CommPutString(_shell.server, "\",");
	CommPutString(_shell.server, &numStr);
	CommPutNewline(_shell.server);
	_wifi.statusBits.tcpConnectionStatus = WIFI_TCP_CONNECTING;
	return true;
}

// BUTTON ACTIONS--------------------------------------------------------------

/**
 * This function executes when the button is pressed
 */
void ButtonPress(void)
{
	RelayControl(_relayState ? 0 : 1);
}

/**
 * This function executes when the button is held
 */
void ButtonHold(void)
{
	;
}

/**
 * This function executes when the button is released
 */
void ButtonRelease(void)
{
	;
}

// LOAD MEASUREMENT FUNCTIONS--------------------------------------------------

/**
 * Initializes all variables necessary for load calculations
 */
void InitializeLoadMeasurement(void)
{
	InitializeRingBuffer(&_adc.samples, ADC_WINDOW_SIZE, 2, &_adcData);
	_adc.pinFloatAnimation = 0;
}

/**
 * Calculates the RMS current from the adc sample buffer
 * @return The RMS current
 */
double CalculateCurrentRMS(void)
{
	if(_adc.samples.length != _adc.samples.capacity)
		return 0.0;

	// Pause ADC sampling
	PIE5bits.TMR6IE	= 0;

	unsigned int i;
	double result = 0.0;
	// Calculate the sum of the squares of each sample
	for(i = 0; i < ADC_WINDOW_SIZE; i++)
	{
		unsigned int sample = 0;
		RingBufferDequeue(&_adc.samples, &sample);
		double adjSample = ((sample * 0.000805860806) - 2.474) / 0.04;
		result += adjSample * adjSample;
	}

	// Divide the sum by the window size,
	// then take the square root of this value to obtain the RMS voltage
	result /= (double) ADC_WINDOW_SIZE;
	result = sqrt(result);
	result *= 120.0;

	_adc.samples.head = 0;
	_adc.samples.tail = 0;
	_adc.samples.length = 0;

	// Resume ADC sampling
	PIE5bits.TMR6IE	= 1;
	return result;
}

// RELAY CONTROL FUNCTIONS-----------------------------------------------------

/**
 * Sets the relay to the specified state
 * @param state <b>0</b> open the relay, <b>>0</b> to close the relay
 */
void RelayControl(unsigned char state)
{
	if(state)	// Close relay
	{
		RELAY_SET = 1;
		LED = 1;
	}
	else		// Open relay
	{
		RELAY_RES = 1;
		LED = 0;
	}
	_relayState = state;
	ShellAddTask(TaskUpdateRelayStatus, 1, 0, 0, false, false, false, 0);
	T0CONbits.TMR0ON = true;
}