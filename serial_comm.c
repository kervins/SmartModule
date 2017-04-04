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
						uint16_t txBufferSize, uint16_t rxBufferSize, uint16_t lineBufferSize,
						char* txData, char* rxData, char* lineData,
						uint24_t lineQueueBaseAddress, uint16_t lineQueueSize, uint8_t* lineQueueData,
						NewlineFlags txNewline, NewlineFlags rxNewline,
						const CommDataRegisters* registers,
						bool enableFlowControl, bool enableEcho)
{
	comm->status = enableFlowControl ? 0x3 : 0x0;	// Enable TX and RX flow control
	comm->mode = enableEcho ? 0x7 : 0x0;			// Enable character, newline, and sequence echo
	comm->modeBits.useExternalBuffer = true;
	comm->newline.tx = txNewline;
	comm->newline.rx = rxNewline;
	comm->newline.inProgress = 0;
	comm->sequence.status = 0;
	comm->sequence.paramCount = 0;
	comm->sequence.terminator = 0;
	comm->external.baseAddress = lineQueueBaseAddress;
	comm->external.blockSize = lineBufferSize;
	RingBufferCreate(&comm->external.lineQueue, lineQueueSize, lineQueueData);
	RingBufferCreate(&comm->buffers.tx, txBufferSize, txData);
	RingBufferCreate(&comm->buffers.rx, rxBufferSize, rxData);
	BufferU8Create(&comm->buffers.line, lineBufferSize, lineData);
	comm->registers = registers;
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
	|| comm->external.lineQueue.length == comm->external.lineQueue.bufferSize
	|| (!comm->modeBits.useExternalBuffer && comm->statusBits.hasLine))
		return;

	// Only continue if the RX buffer is not empty
	if(comm->buffers.rx.length == 0)
		return;

	// Retrieve the next character from the RX buffer
	char ch = RingBufferDequeue(&comm->buffers.rx);

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
				if(comm->modeBits.echoRx)
					CommPutChar(comm, ch);
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
		comm->buffers.line.data[comm->buffers.line.length] = ch;
		comm->buffers.line.length++;

		// Echo received character (if enabled)
		// Backspace characters will be echoed if the line buffer is not empty
		// (printable characters will be echoed iff received outside of an escape sequence)
		if(comm->modeBits.echoRx)
			CommPutChar(comm, ch);
	}

	// If the line buffer is full, flush the line to external RAM (or set hasLine flag if not using external RAM)
	if(comm->buffers.line.length == comm->buffers.line.bufferSize)
	{
		if(!comm->modeBits.isBinaryMode)
		{
			comm->buffers.line.data[comm->buffers.line.bufferSize - 1] = ASCII_NUL;

			if(comm->modeBits.echoNewline)
				CommPutNewline(comm);
		}

		if(comm->modeBits.useExternalBuffer)
			CommFlushLineBuffer(comm);
		else
			comm->statusBits.hasLine = true;
	}

	// If a newline has been received, flush the line to external RAM (or set hasLine flag if not using external RAM)
	if(comm->newline.inProgress == comm->newline.rx)
	{
		comm->buffers.line.data[comm->buffers.line.length] = ASCII_NUL;
		comm->buffers.line.length++;
		comm->newline.inProgress = 0;
		if(comm->modeBits.useExternalBuffer)
			CommFlushLineBuffer(comm);
		else
			comm->statusBits.hasLine = true;

		// If enabled, echo the newline sequence
		if(comm->modeBits.echoNewline)
			CommPutNewline(comm);
	}
}

void CommFlushLineBuffer(CommPort* comm)
{

	while(_sram.busy)
		continue;
	RingBufferEnqueue(&comm->external.lineQueue, comm->buffers.line.length);
	SramWrite(comm->external.baseAddress + (comm->external.blockSize * comm->external.lineQueue.head),
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
	RingBufferEnqueue(&comm->buffers.tx, data);
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

void CommPutBuffer(CommPort* comm, BufferU8* source)
{
	int i;
	for(i = 0; i < source->length && source->data[i] != ASCII_NUL; i++)
	{
		CommPutChar(comm, source->data[i]);
	}
	CommPutNewline(comm);
}

// LINKED LIST PRINT FUNCTIONS-------------------------------------------------

void CommPutLinkedListChars(LinkedList_16Element* list, CommPort* comm)
{
	if(list == 0 || comm == 0)
		return;

	uint8_t i;
	LinkedListNode* node = list->first;
	while(node)
	{
		CommPutChar(comm, (char) node->data);
		node = node->next;
	}
}

void CommPrintLinkedListInfo(LinkedList_16Element* list, CommPort* comm)
{
	if(list == 0 || comm == 0)
		return;

	uint8_t i, j;
	char numStr[17];

	CommPutString(comm, "        0       1       2       3       4       5       6       7       8       9       A       B       C       D       E       F");
	CommPutNewline(comm);
	CommPutString(comm, "______________________________________________________________________________________________________________________________________");
	CommPutNewline(comm);
	Delay1KTCYx(10);

	CommPutString(comm, "ADDR:   ");
	for(i = 0, j = 0; i < 16; i++)
	{
		itoa(&numStr, (uint16_t) & list->nodeMemory[i], 16);
		CommPutString(comm, "0x");
		CommPutString(comm, numStr);
		for(j = strlen(&numStr); j < 6; j++)
			CommPutChar(comm, ' ');
	}
	CommPutNewline(comm);
	Delay1KTCYx(10);

	CommPutString(comm, "PREV:   ");
	for(i = 0; i < 16; i++)
	{
		itoa(&numStr, (uint16_t) list->nodeMemory[i].prev, 16);
		CommPutString(comm, "0x");
		CommPutString(comm, numStr);
		for(j = strlen(&numStr); j < 6; j++)
			CommPutChar(comm, ' ');
	}
	CommPutNewline(comm);
	Delay1KTCYx(10);

	CommPutString(comm, "NEXT:   ");
	for(i = 0; i < 16; i++)
	{
		itoa(&numStr, (uint16_t) list->nodeMemory[i].next, 16);
		CommPutString(comm, "0x");
		CommPutString(comm, numStr);
		for(j = strlen(&numStr); j < 6; j++)
			CommPutChar(comm, ' ');
	}
	CommPutNewline(comm);
	Delay1KTCYx(10);

	CommPutString(comm, "ALLOC:  ");
	for(i = 0; i < 16; i++)
	{
		if((list->memoryBitmap >> i) & 0x1)
			CommPutString(comm, "XXXXXXXX");
		else
			CommPutString(comm, "________");
	}
	CommPutNewline(comm);
	Delay1KTCYx(10);

	CommPutString(comm, "DATA:   ");
	for(i = 0; i < 16; i++)
	{
		CommPutChar(comm, (char) list->nodeMemory[i].data);
		CommPutString(comm, "       ");
	}
	Delay1KTCYx(10);
	CommPutNewline(comm);
}