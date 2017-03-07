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
#include "ringbuffer.h"
#include "utility.h"
#include "main.h"

// GLOBAL VARIABLES------------------------------------------------------------

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
						LineTermination txLineTermination, LineTermination rxLineTermination,
						const CommDataRegisters* registers)
{
	CommPort comm;
	comm.status = 0x3;
	comm.txLineTermination = txLineTermination;
	comm.rxLineTermination = rxLineTermination;
	comm.delimCount = 0;
	comm.RxAction = NULL;
	comm.LineAction = NULL;
	comm.rxByteCount = 0;
	comm.txBuffer = RingBufferCreate(txBufferSize, txData);
	comm.rxBuffer = RingBufferCreate(rxBufferSize, rxData);
	comm.lineBuffer = RingBufferCreate(lineBufferSize, lineData);
	comm.registers = registers;
	return comm;
}

// DATA TRANSPORT FUNCTIONS----------------------------------------------------

void CommGetData(CommPort* comm)
{
	if(comm->rxBuffer.length == 0 || comm->rxByteCount == 0)
		return;

	uint8_t data = RingBufferDequeue(&comm->rxBuffer);
}

void CommGetLine(CommPort* comm)
{
	if(comm->rxBuffer.length == 0 || comm->statusBits.hasLine)
		return;

	char ch = RingBufferDequeue(&comm->rxBuffer);
	switch(ch)
	{
		case ASCII_BS:
		case ASCII_DEL:
		{
			if(comm->lineBuffer.length)
				RingBufferRemoveLast(&comm->lineBuffer, 1);
			break;
		}
		case ASCII_LF:
		case ASCII_CR:
		{
			comm->delimCount++;
			if(comm->rxLineTermination == ch || comm->delimCount == 2)
			{
				RingBufferEnqueue(&comm->lineBuffer, ASCII_NUL);
				comm->delimCount = 0;
				comm->statusBits.hasLine = true;
			}
			break;
		}
		default:
		{
			RingBufferEnqueue(&comm->lineBuffer, ch);
			break;
		}
	}
}

void CommPutLine(CommPort* source, CommPort* destination)
{
	char ch = RingBufferDequeue(&source->lineBuffer);
	if(ch == ASCII_NUL || source->lineBuffer.length == 0)
	{
		CommPutLineTermination(destination);
		source->statusBits.hasLine = false;
	}
	else
		CommPutChar(destination, ch);
}

void CommPutString(CommPort* comm, const char* str)
{
	uint24_t length = 0;
	while(comm->txBuffer.length < comm->txBuffer.bufferSize && str[length] != ASCII_NUL)
	{
		CommPutChar(comm, str[length]);
		length++;
	}
	if(length)
	{
		CommPutLineTermination(comm);
	}
}

void CommPutLineTermination(CommPort* comm)
{
	switch(comm->txLineTermination)
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
	}
}

void CommPutChar(CommPort* comm, char data)
{
	while(comm->txBuffer.length == comm->txBuffer.bufferSize)
		continue;
	RingBufferEnqueue(&comm->txBuffer, data);
	bit_set(*comm->registers->pPie, comm->registers->txieBit);
}