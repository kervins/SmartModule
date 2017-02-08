/* Project:	SmartModule
 * File:	buffer.c
 * Author:	Jonathan Ruisi
 * Created:	December 20, 2016, 7:52 PM
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
	// Cannot throw exception here, but that would be appropriate...
	if(buffer->length == 0)
		return 0;

	char data = buffer->data[buffer->tail];
	buffer->tail = (buffer->tail + 1) % buffer->bufferSize;
	buffer->length--;
	return data;
}