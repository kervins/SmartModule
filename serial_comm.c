/* Project:	SmartModule
 * File:	serial_comm.c
 * Author:	Jonathan Ruisi
 * Created:	February 14, 2017, 2:22 AM
 */

#include <xc.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "main.h"
#include "serial_comm.h"
#include "ringbuffer.h"
#include "utility.h"
#include "common_types.h"

// COMM PORT TASK FUNCTIONS----------------------------------------------------

void CommPortUpdate(CommPort* comm)
{
	// Manage RX flow control (This block of code sends XON, XOFF is sent in the ISR for comm RX)
	if(comm->statusBits.isRxFlowControl
	&& comm->statusBits.isRxPaused
	&& comm->rxBuffer.length <= XON_THRESHOLD)
	{
		while(!(*comm->registers->pTxSta).TRMT)
			continue;
		*comm->registers->pTxReg = ASCII_XON;
		comm->statusBits.isRxPaused = false;
	}

	// Process received data
	if(comm->rxBuffer.length && !comm->statusBits.hasLine && comm->RxAction)
		comm->RxAction(comm);
	else if(comm->statusBits.hasLine && comm->LineAction)
		comm->LineAction(comm);
}

// INITIALIZATION FUNCTIONS----------------------------------------------------

CommPort CommPortCreate(uint16_t txBufferSize, uint16_t rxBufferSize, uint16_t lineBufferSize,
						char* txData, char* rxData, char* lineData,
						LineTermination txNewline, LineTermination rxNewline,
						const CommDataRegisters* registers)
{
	CommPort comm;
	comm.status = 0x3;	// By default, enable both TX and RX flow control
	comm.txNewline = txNewline;
	comm.rxNewline = rxNewline;
	comm.delimeter = 0;
	comm.RxAction = NULL;
	comm.LineAction = NULL;
	comm.txBuffer = RingBufferCreate(txBufferSize, txData);
	comm.rxBuffer = RingBufferCreate(rxBufferSize, rxData);
	comm.lineBuffer = BufferCreate(lineBufferSize, lineData);
	comm.registers = registers;
	return comm;
}

// DATA TRANSPORT FUNCTIONS----------------------------------------------------

void CommGetLine(CommPort* comm)
{
	if(comm->statusBits.hasLine)
		return;
	else if(comm->lineBuffer.length == comm->lineBuffer.bufferSize)
	{
		comm->statusBits.hasLine = true;
		return;
	}

	if(comm->rxBuffer.length == 0)
	{
		comm->statusBits.hasLine = false;
		return;
	}

	char ch = RingBufferDequeue(&comm->rxBuffer);
	if(comm->statusBits.echoRx)
		CommPutChar(comm, ch);

	switch(ch)
	{
		case ASCII_BS:
		case ASCII_DEL:
		{
			if(comm->lineBuffer.length)
				comm->lineBuffer.length--;
			break;
		}
		case ASCII_LF:
		case ASCII_CR:
		{
			comm->delimeter |= ch;
			if(comm->delimeter == comm->rxNewline)
			{
				comm->lineBuffer.data[comm->lineBuffer.length] = ASCII_NUL;
				comm->lineBuffer.length++;
				comm->delimeter = 0;
				comm->statusBits.hasLine = true;
			}
			else if(((comm->rxNewline >> 4) & 0xF) == ch)
				comm->delimeter <<= 4;
			else
				comm->delimeter = 0;
			break;
		}
		case ASCII_ESC:
		{

			break;
		}
		default:
		{
			if(comm->delimeter)		// An incomplete newline sequence has been received
				comm->delimeter = 0;
			comm->lineBuffer.data[comm->lineBuffer.length] = ch;
			comm->lineBuffer.length++;
			break;
		}
	}
}

void CommPutLine(CommPort* source, CommPort* destination)
{
	int i;
	for(i = 0; i < source->lineBuffer.length && source->lineBuffer.data[i] != ASCII_NUL; i++)
	{
		CommPutChar(destination, source->lineBuffer.data[i]);
	}
	CommPutNewline(destination);
}

void CommPutString(CommPort* comm, const char* str)
{
	uint16_t length;
	for(length = 0; str[length] != ASCII_NUL; length++)
		CommPutChar(comm, str[length]);
	if(length)
		CommPutNewline(comm);
}

void CommPutNewline(CommPort* comm)
{
	switch(comm->txNewline)
	{
		case CR_ONLY:
		{
			CommPutChar(comm, ASCII_CR);
			break;
		}
		case LF_ONLY:
		{
			CommPutChar(comm, ASCII_LF);
			break;
		}
		case CR_LF:
		{
			CommPutChar(comm, ASCII_CR);
			CommPutChar(comm, ASCII_LF);
			break;
		}
		case LF_CR:
		{
			CommPutChar(comm, ASCII_LF);
			CommPutChar(comm, ASCII_CR);
			break;
		}
	}
}

void CommPutChar(CommPort* comm, char data)
{
	while(comm->txBuffer.length == comm->txBuffer.bufferSize)
		continue;
	RingBufferEnqueue(&comm->txBuffer, data);
	bit_set(*comm->registers->pPie, comm->registers->txieBit);
}