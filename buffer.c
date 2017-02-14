/* Project:	SmartModule
 * File:	buffer.c
 * Author:	Jonathan Ruisi
 * Created:	February 7, 2017, 11:52 PM
 */

#include <xc.h>
#include "buffer.h"

// RING BUFFER FUNCTIONS-------------------------------------------------------

LineBuffer LineBufferCreate(uint8_t bufferSize, char* dataBuffer)
{
	LineBuffer lineBuffer;
	lineBuffer.length = 0;
	lineBuffer.isReceiving = false;
	lineBuffer.isComplete = false;
	lineBuffer.data = dataBuffer;
	return lineBuffer;
}

RingBuffer RingBufferCreate(uint8_t bufferSize, char* dataBuffer)
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