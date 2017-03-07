/* Project:	SmartModule
 * File:	ringbuffer.c
 * Author:	Jonathan Ruisi
 * Created:	February 14, 2017, 2:10 AM
 */

#include "ringbuffer.h"

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

void RingBufferReset(volatile RingBuffer* buffer)
{
	buffer->length = 0;
	buffer->head = buffer->bufferSize - 1;
	buffer->tail = 0;
}

void RingBufferEnqueue(volatile RingBuffer* buffer, char data)
{
	if(buffer->length == buffer->bufferSize)	// Overflow hack
		return;
	buffer->head = (buffer->head + 1) % buffer->bufferSize;
	buffer->data[buffer->head] = data;
	buffer->length++;
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

char RingBufferDequeue(volatile RingBuffer* buffer)
{
	char data = buffer->data[buffer->tail];
	buffer->tail = (buffer->tail + 1) % buffer->bufferSize;
	buffer->length--;
	return data;
}

void RingBufferRemoveLast(volatile RingBuffer* buffer, uint16_t count)
{
	if(buffer->length == 0)
		return;
	if(count > buffer->length)
		count = buffer->length;
	buffer->head = (buffer->head + (buffer->bufferSize - count)) % buffer->bufferSize;
	buffer->length -= count;
}

char RingBufferPeek(volatile RingBuffer* buffer, uint16_t index)
{
	if(index >= buffer->length)
		index %= buffer->length;
	return buffer->data[buffer->tail + index];
}