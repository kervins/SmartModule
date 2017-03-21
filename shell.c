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
	if(_shell.terminal->statusBits.hasSequence)
	{
		CommResetSequence(_shell.terminal);
	}

	while(_shell.terminal->lineQueue.length)
	{
		while(_sram.isBusy)
			continue;

		uint24_t address = _shell.terminal->lineQueueBaseAddress +
				(_shell.terminal->buffers.line.bufferSize * _shell.terminal->lineQueue.tail);
		uint8_t length = RingBufferU8VolDequeue(&_shell.terminal->lineQueue);

		SramRead(address, length, &_shell.swapBuffer);
		while(_sram.isBusy)
			continue;
		CommPutBuffer(_shell.terminal, &_shell.swapBuffer);
	}
}

// COMMANDS -------------------------------------------------------------------

// SHELL MANAGEMENT------------------------------------------------------------

void ShellInitialize(CommPort* terminalComm, uint16_t swapBufferSize, char* swapBufferData)
{
	_shell.status = 0;
	_shell.terminal = terminalComm;
	BufferU8Create(&_shell.swapBuffer, swapBufferSize, swapBufferData);
	_shell.currentTask = NULL;
	_shell.taskQueue.length = 0;
	_shell.taskQueue.head = SHELL_MAX_TASK_COUNT - 1;
	_shell.taskQueue.tail = 0;
}