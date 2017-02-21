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
volatile RingBuffer _txBuffer1, _txBuffer2, _rxBuffer1, _rxBuffer2;
volatile CommStatus _commStatus1, _commStatus2;

// RING BUFFER FUNCTIONS-------------------------------------------------------

RingBuffer RingBufferCreate(uint16_t bufferSize, char* dataBuffer)
{
	RingBuffer ringBuffer;
	ringBuffer.bufferSize = bufferSize;
	ringBuffer.length = 0;
	ringBuffer.head = bufferSize - 1;
	ringBuffer.tail = 0;
	ringBuffer.data = dataBuffer;
	return ringBuffer;
}

void RingBufferEnqueue(volatile RingBuffer* buffer, char data)
{
	buffer->head = (buffer->head + 1) % buffer->bufferSize;
	buffer->data[buffer->head] = data;
	if(buffer->length == buffer->bufferSize)
		buffer->tail = (buffer->tail + 1) % buffer->bufferSize;
	else
		buffer->length++;
}

char RingBufferDequeue(volatile RingBuffer* buffer)
{
	if(buffer->length == 0)
		return -1;

	char data = buffer->data[buffer->tail];
	buffer->tail = (buffer->tail + 1) % buffer->bufferSize;
	buffer->length--;
	return data;
}

// STDIO FUNCTIONS-------------------------------------------------------------

char getch1(void)
{
	return RingBufferDequeue(&_rxBuffer1);
}

char getch2(void)
{
	return RingBufferDequeue(&_rxBuffer2);
}

void putch1(char data)
{
	RingBufferEnqueue(&_txBuffer1, data);	// Add character to TX buffer
	if(_txBuffer1.length == TX_BUFFER_SIZE)	// If buffer is full, switch to polling mode and flush buffer
	{
		PIE1bits.TX1IE	= false;
		while(_txBuffer1.length > 0)
		{
			while(!PIR1bits.TX1IF)
			{
				continue;
			}
			char character = RingBufferDequeue(&_txBuffer1);
			TXREG1 = character;
		}
	}
	PIE1bits.TX1IE	= true;					// Enable TX interrupt - this will interrupt immediately
}

void putch2(char data)
{
	RingBufferEnqueue(&_txBuffer2, data);
	if(_txBuffer2.length == TX_BUFFER_SIZE)
	{
		PIE3bits.TX2IE	= false;
		while(_txBuffer2.length > 0)
		{
			while(!PIR3bits.TX2IF)
			{
				continue;
			}
			char character = RingBufferDequeue(&_txBuffer2);
			TXREG2 = character;
		}
	}
	PIE3bits.TX2IE	= true;
}

// FLOW CONTROL FUNCTIONS------------------------------------------------------

void CheckFlowControlRx1(void)
{
	if(!_commStatus1.statusBits.isRxPaused && _rxBuffer1.length >= XOFF_THRESHOLD)
	{
		PIE1bits.TX1IE	= false;
		while(!PIR1bits.TX1IF)
		{
			continue;
		}
		TXREG1 = XOFF;
		_commStatus1.statusBits.isRxPaused = true;
		PIE1bits.TX1IE	= true;
	}
	else if(_commStatus1.statusBits.isRxPaused && _rxBuffer1.length <= XON_THRESHOLD)
	{
		PIE1bits.TX1IE	= false;
		while(!PIR1bits.TX1IF)
		{
			continue;
		}
		TXREG1 = XON;
		_commStatus1.statusBits.isRxPaused = false;
		PIE1bits.TX1IE	= true;
	}
}

void CheckFlowControlRx2(void)
{
	if(!_commStatus2.statusBits.isRxPaused && _rxBuffer2.length >= XOFF_THRESHOLD)
	{
		PIE3bits.TX2IE	= false;
		while(!PIR3bits.TX2IF)
		{
			continue;
		}
		TXREG2 = XOFF;
		_commStatus2.statusBits.isRxPaused = true;
		PIE3bits.TX2IE	= true;
	}
	else if(_commStatus2.statusBits.isRxPaused && _rxBuffer2.length <= XON_THRESHOLD)
	{
		PIE3bits.TX2IE	= false;
		while(!PIR3bits.TX2IF)
		{
			continue;
		}
		TXREG2 = XON;
		_commStatus2.statusBits.isRxPaused = false;
		PIE3bits.TX2IE	= true;
	}
}