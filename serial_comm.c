/* Project:	SmartModule
 * File:	serial_comm.c
 * Author:	Jonathan Ruisi
 * Created:	February 14, 2017, 2:22 AM
 */

#include <xc.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "serial_comm.h"

// GLOBAL VARIABLES------------------------------------------------------------

// COMM PORT TASK FUNCTIONS----------------------------------------------------

void CommPortUpdate(CommPort* comm)
{
	// Manage RX flow control
	if(comm->statusBits.isRxFlowControl)
	{
		if(!comm->statusBits.isRxPaused
		&& comm->rxBuffer.length >= XOFF_THRESHOLD
		&& (*comm->registers->pTxSta).TRMT)
		{
			*comm->registers->pTxReg = ASCII_XOFF;
			comm->statusBits.isRxPaused = true;
		}
		else if(comm->statusBits.isRxPaused
				&& comm->rxBuffer.length <= XON_THRESHOLD
				&& (*comm->registers->pTxSta).TRMT)
		{
			*comm->registers->pTxReg = ASCII_XON;
			comm->statusBits.isRxPaused = false;
		}
	}

	// Transmit the next character in the TX buffer
	if(!comm->statusBits.isTxPaused
	&& comm->txBuffer.length
	&& (*comm->registers->pTxSta).TRMT)
	{
		char data = RingBufferDequeue(&comm->txBuffer);
		*comm->registers->pTxReg = data;
	}

	// Process received data
	if(comm->rxBuffer.length && comm->RxAction)
		comm->RxAction(comm);
}

// INITIALIZATION FUNCTIONS----------------------------------------------------

CommPort CommPortCreate(uint16_t txBufferSize, uint16_t rxBufferSize, uint16_t lineBufferSize,
						char* txData, char* rxData, char* lineData,
						LineTermination txLineTermination, LineTermination rxLineTermination,
						const CommDataRegisters* registers)
{
	CommPort comm;
	comm.status = 0x3;
	comm.txLineTermination = txLineTermination;
	comm.rxLineTermination = rxLineTermination;
	comm.delimCount = 0;
	comm.RxAction = NULL;
	comm.txBuffer = RingBufferCreate(txBufferSize, txData);
	comm.rxBuffer = RingBufferCreate(rxBufferSize, rxData);
	comm.lineBuffer = LineBufferCreate(lineBufferSize, lineData);
	comm.registers = registers;
	return comm;
}

RingBuffer RingBufferCreate(uint16_t bufferSize, char* data)
{
	RingBuffer buffer;
	buffer.bufferSize = bufferSize;
	buffer.length = 0;
	buffer.head = bufferSize - 1;
	buffer.tail = 0;
	buffer.data = data;
	return buffer;
}

LineBuffer LineBufferCreate(uint16_t bufferSize, char* data)
{
	LineBuffer buffer;
	buffer.bufferSize = bufferSize;
	buffer.length = 0;
	buffer.data = data;
	return buffer;
}

// BUFFER MANAGEMENT FUNCTIONS-------------------------------------------------

void RingBufferEnqueue(volatile RingBuffer* buffer, char data)
{
	if(buffer->length == buffer->bufferSize)	// Overflow hack
		return;
	buffer->head = (buffer->head + 1) % buffer->bufferSize;
	buffer->data[buffer->head] = data;
	buffer->length++;
}

char RingBufferDequeue(volatile RingBuffer* buffer)
{
	char data = buffer->data[buffer->tail];
	buffer->tail = (buffer->tail + 1) % buffer->bufferSize;
	buffer->length--;
	return data;
}

/*void RingBufferEnqueue(volatile RingBuffer* buffer, char data)
{
	buffer->head = (buffer->head + 1) % buffer->bufferSize;
	buffer->data[buffer->head] = data;
	if(buffer->length == buffer->bufferSize)
		buffer->tail = (buffer->tail + 1) % buffer->bufferSize;
	else
		buffer->length++;
}*/

// DATA TRANSPORT FUNCTIONS----------------------------------------------------

int CommPutString(CommPort* comm, const char* str)
{
	int length = 0;
	while(comm->txBuffer.length < comm->txBuffer.bufferSize && str[length] != ASCII_NUL)
	{
		RingBufferEnqueue(&comm->txBuffer, str[length]);
		length++;
	}
	if(length)
	{
		switch(comm->txLineTermination)
		{
			case CR_ONLY:
			{
				RingBufferEnqueue(&comm->txBuffer, ASCII_CR);
				break;
			}
			case LF_ONLY:
			{
				RingBufferEnqueue(&comm->txBuffer, ASCII_LF);
				break;
			}
			case CR_LF:
			{
				RingBufferEnqueue(&comm->txBuffer, ASCII_CR);
				RingBufferEnqueue(&comm->txBuffer, ASCII_LF);
				break;
			}
		}
	}
	return length;
}

void CommGetString(CommPort* comm)
{
	if(comm->rxBuffer.length == 0 || comm->statusBits.hasLine)
		return;

	char ch = RingBufferDequeue(&comm->rxBuffer);
	switch(ch)
	{
		case ASCII_BS:
		{
			if(comm->lineBuffer.length)
				comm->lineBuffer.length--;
			break;
		}
		case ASCII_LF:
		{
			break;
		}
		case ASCII_CR:
		{
			comm->delimCount++;
			if(comm->rxLineTermination == CR_ONLY || comm->delimCount > 1)
			{
				comm->lineBuffer.data[comm->lineBuffer.length] = '\0';
				comm->delimCount = 0;
				comm->statusBits.hasLine = true;
			}
			break;
		}
		default:
		{
			comm->lineBuffer.data[comm->lineBuffer.length] = ch;
			comm->lineBuffer.length++;
			break;
		}
	}

	comm->statusBits.isLineBufferFull = comm->rxBuffer.length == comm->rxBuffer.bufferSize;
	if(comm->statusBits.hasLine)
		comm->RxAction = NULL;
}