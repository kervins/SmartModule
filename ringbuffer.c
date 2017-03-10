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

// ITERATION FUNCTIONS---------------------------------------------------------

RingBufferIterator RingBufferCreateIterator(RingBuffer* buffer)
{
	RingBufferIterator iterator;
	iterator.index = 0;
	iterator.buffer = buffer;
	iterator.Start = _RBIStart;
	iterator.SetCurrent = _RBISetCurrent;
	iterator.GetCurrent = _RBIGetCurrent;
	iterator.Next = _RBINext;
	return iterator;
}

void _RBIStart(RingBufferIterator* iterator)
{
	iterator->index = 0;
}

void _RBISetCurrent(RingBufferIterator* iterator, uint8_t value)
{
	iterator->buffer->data[iterator->buffer->tail + iterator->index] = value;
}

uint8_t _RBIGetCurrent(RingBufferIterator* iterator)
{
	return iterator->buffer->data[iterator->buffer->tail + iterator->index];
}

bool _RBINext(RingBufferIterator* iterator)
{
	if(iterator->index < iterator->buffer->length - 1)  // TODO: Possible problem here
	{
		iterator->index++;
		return true;
	}
	return false;
}