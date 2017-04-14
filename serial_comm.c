/* Project:	SmartModule
 * File:	serial_comm.c
 * Author:	Jonathan Ruisi
 * Created:	February 14, 2017, 2:22 AM
 */

#include <xc.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "main.h"
#include "serial_comm.h"
#include "sram.h"
#include "utility.h"

// COMM PORT FUNCTIONS---------------------------------------------------------

void CommPortInitialize(CommPort* comm,
						unsigned int txBufferSize, unsigned int rxBufferSize, unsigned int lineBufferSize,
						char* txData, char* rxData, char* lineData,
						unsigned short long int lineQueueBaseAddress,
						unsigned int lineQueueSize,
						unsigned char* lineQueueData,
						NewlineFlags txNewline, NewlineFlags rxNewline,
						const CommDataRegisters* registers,
						bool enableFlowControl, bool enableEcho,
						unsigned char echoRow, unsigned char echoColumn)
{
	comm->status = enableFlowControl ? 0x3 : 0x0;	// Enable TX and RX flow control
	comm->mode = enableEcho ? 0x7 : 0x0;			// Enable character, newline, and sequence echo
	comm->modeBits.useExternalBuffer = true;
	comm->cursor.x = echoColumn;
	comm->cursor.y = echoRow;
	comm->newline.tx = txNewline;
	comm->newline.rx = rxNewline;
	comm->newline.inProgress = 0;
	comm->sequence.status = 0;
	comm->sequence.paramCount = 0;
	comm->sequence.terminator = 0;
	comm->external.baseAddress = lineQueueBaseAddress;
	comm->external.blockSize = lineBufferSize;
	comm->registers = registers;
	RingBufferU8Create(&comm->external.buffer, lineQueueSize, lineQueueData);
	RingBufferU8Create(&comm->buffers.tx, txBufferSize, txData);
	RingBufferU8Create(&comm->buffers.rx, rxBufferSize, rxData);
	InitializeBuffer(&comm->buffers.line, lineBufferSize, 1, lineData);
}

void UpdateCommPort(CommPort* comm)
{
	// Manage RX flow control (This block of code sends XON, whereas XOFF is sent in the ISR for USART RX)
	if(comm->statusBits.isRxFlowControl
	&& comm->statusBits.isRxPaused
	&& comm->buffers.rx.length <= XON_THRESHOLD)
	{
		while(!(*comm->registers->pTxSta).TRMT)
			continue;
		*comm->registers->pTxReg = ASCII_XON;
		comm->statusBits.isRxPaused = false;
	}

	// Only continue if all events have been handled
	if(comm->statusBits.hasSequence
	|| comm->external.buffer.length == comm->external.buffer.bufferSize
	|| (!comm->modeBits.useExternalBuffer && comm->statusBits.hasLine))
		return;

	// Only continue if the RX buffer is not empty
	if(comm->buffers.rx.length == 0)
		return;

	// Retrieve the next character from the RX buffer
	char ch = RingBufferU8Dequeue(&comm->buffers.rx);

	// If an escape sequence has been initiated,
	// retrieve parameters until terminating character is received
	if(comm->sequence.statusBits.csiCharCount == 1)
	{
		if(ch == '[')
			comm->sequence.statusBits.csiCharCount++;
		else
			CommResetSequence(comm);
	}
	else if(comm->sequence.statusBits.csiCharCount == 2)
	{
		// Check for terminating character (any alphabetical character)
		// Check for parameters (0-9 and : ; < = > ?)
		if((ch >= 0x41 && ch <= 0x5A) || (ch >= 0x61 && ch <= 0x7A))
		{
			comm->sequence.terminator = ch;
			comm->sequence.statusBits.sequenceId = ch;
			comm->statusBits.hasSequence = true;

			// If enabled, echo the sequence
			if(comm->modeBits.echoSequence)
			{
				CommPutChar(comm, ASCII_ESC);
				CommPutChar(comm, '[');
				uint8_t i;
				for(i = 0; i < comm->sequence.paramCount; i++)
				{
					CommPutChar(comm, comm->sequence.params[i]);
				}
				CommPutChar(comm, comm->sequence.terminator);
			}
		}
		else if(comm->sequence.paramCount < SEQ_MAX_PARAMS
				&& (ch >= 0x30 && ch <= 0x3F)
				&& !comm->sequence.terminator)
		{
			comm->sequence.params[comm->sequence.paramCount] = ch;
			comm->sequence.paramCount++;
		}
		else
		{
			CommResetSequence(comm);
		}
	}

	// If a control character was received, process it, otherwise add character to the line
	if(ch < 0x20 && !comm->modeBits.isBinaryMode && !comm->sequence.status)
	{
		switch(ch)
		{
			case ASCII_BS:
			{
				if(comm->buffers.line.length)
					comm->buffers.line.length--;
				break;
			}
			case ASCII_LF:
			{
				comm->newline.inProgress |= NEWLINE_LF;
				break;
			}
			case ASCII_CR:
			{
				comm->newline.inProgress |= NEWLINE_CR;
				break;
			}
			case ASCII_ESC:
			{
				comm->sequence.statusBits.csiCharCount++;
				break;
			}
		}
	}
	else if(!comm->sequence.status)
	{
		// Since a printable character was received, invalidate a partial newline
		comm->newline.inProgress = 0;

		// Add character to the line
		((char*) comm->buffers.line.data)[comm->buffers.line.length] = ch;
		comm->buffers.line.length++;

		// Echo received character (if enabled)
		// Backspace characters will be echoed if the line buffer is not empty
		// (printable characters will be echoed iff received outside of an escape sequence)
		if(comm->modeBits.echoRx)
		{
			while(comm->buffers.tx.length || !(*comm->registers->pTxSta).TRMT)
				continue;
			CommPutSequence(comm, ANSI_SCPOS, 0);
			CommPutSequence(comm, ANSI_CPOS, 2, comm->cursor.y, comm->cursor.x + comm->buffers.line.length - 1);
			CommPutChar(comm, ch);
			CommPutSequence(comm, ANSI_RCPOS, 0);
		}
	}

	// If the line buffer is full, flush the line to external RAM (or set hasLine flag if not using external RAM)
	if(comm->buffers.line.length == comm->buffers.line.capacity)
	{
		if(!comm->modeBits.isBinaryMode)
		{
			((char*) comm->buffers.line.data)[comm->buffers.line.capacity - 1] = ASCII_NUL;

			if(comm->modeBits.echoNewline)
			{
				while(comm->buffers.tx.length || !(*comm->registers->pTxSta).TRMT)
					continue;
				comm->cursor.y++;
				CommPutSequence(comm, ANSI_SCPOS, 0);
				CommPutSequence(comm, ANSI_CPOS, 2, comm->cursor.y, comm->cursor.x);
				CommPutNewline(comm);
				CommPutSequence(comm, ANSI_RCPOS, 0);
			}
		}

		if(comm->modeBits.useExternalBuffer)
			CommFlushLineBuffer(comm);
		else
			comm->statusBits.hasLine = true;
	}

	// If a newline has been received, flush the line to external RAM (or set hasLine flag if not using external RAM)
	if(comm->newline.inProgress == comm->newline.rx)
	{
		((char*) comm->buffers.line.data)[comm->buffers.line.length] = ASCII_NUL;
		comm->buffers.line.length++;
		comm->newline.inProgress = 0;
		if(comm->modeBits.useExternalBuffer)
			CommFlushLineBuffer(comm);
		else
			comm->statusBits.hasLine = true;

		// If enabled, echo the newline sequence
		if(comm->modeBits.echoNewline)
		{
			while(comm->buffers.tx.length || !(*comm->registers->pTxSta).TRMT)
				continue;
			comm->cursor.y++;
			CommPutSequence(comm, ANSI_SCPOS, 0);
			CommPutSequence(comm, ANSI_CPOS, 2, comm->cursor.y, comm->cursor.x);
			CommPutNewline(comm);
			CommPutSequence(comm, ANSI_RCPOS, 0);
		}
	}
}

void CommFlushLineBuffer(CommPort* comm)
{
	while(_sram.statusBits.busy)
		continue;
	RingBufferU8Enqueue(&comm->external.buffer, comm->buffers.line.length);
	SramWriteBytes(comm->external.baseAddress
				+ (comm->external.blockSize *  comm->external.buffer.head),
				&comm->buffers.line);
	comm->buffers.line.length = 0;
}

void CommResetSequence(CommPort* comm)
{
	comm->statusBits.hasSequence = false;
	comm->sequence.status = 0;
	comm->sequence.paramCount = 0;
	comm->sequence.terminator = 0;
}

void CommPutChar(CommPort* comm, char data)
{
	while(comm->buffers.tx.length == comm->buffers.tx.bufferSize)
		continue;
	RingBufferU8Enqueue(&comm->buffers.tx, data);
	bit_set(*comm->registers->pPie, comm->registers->txieBit);
}

void CommPutString(CommPort* comm, const char* str)
{
	uint16_t length;
	for(length = 0; str[length] != ASCII_NUL; length++)
		CommPutChar(comm, str[length]);
}

void CommPutNewline(CommPort* comm)
{
	if(comm->newline.tx & NEWLINE_CR)
		CommPutChar(comm, ASCII_CR);

	if(comm->newline.tx & NEWLINE_LF)
		CommPutChar(comm, ASCII_LF);
}

void CommPutSequence(CommPort* comm, unsigned char terminator, unsigned char paramCount, ...)
{
	va_list args;
	unsigned char i;
	CommPutChar(comm, ASCII_ESC);
	CommPutChar(comm, '[');

	if(paramCount)
	{
		va_start(args, paramCount);
		for(i = 0; i < paramCount; i++)
		{
			unsigned char param = va_arg(args, unsigned char);
			if(param < 10)
				CommPutChar(comm, 0x30 + param);
			else
			{
				char numStr[8];
				itoa(&numStr, param, 10);
				CommPutString(comm, &numStr);
			}

			if(i < paramCount - 1)
				CommPutChar(comm, ';');
		}
		va_end(args);
	}
	CommPutChar(comm, terminator);
}

void CommEchoSequence(CommPort* comm)
{
	unsigned char i;
	CommPutChar(comm, ASCII_ESC);
	CommPutChar(comm, '[');

	for(i = 0; i < comm->sequence.paramCount; i++)
	{
		if(comm->sequence.params[i] < 10)
			CommPutChar(comm, 0x30 + comm->sequence.params[i]);
		else
		{
			char numStr[8];
			itoa(&numStr, comm->sequence.params[i], 10);
			CommPutString(comm, &numStr);
		}

		if(i < comm->sequence.paramCount - 1)
			CommPutChar(comm, ';');
	}
	CommPutChar(comm, comm->sequence.terminator);
}