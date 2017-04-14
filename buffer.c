/* Project:	SmartModule
 * File:	buffer.c
 * Author:	Jonathan Ruisi
 * Created:	April 13, 2017, 5:56 PM
 */

#include <xc.h>
#include <stdint.h>
#include <stdlib.h>
#include "buffer.h"

// BUFFER FUNCTIONS------------------------------------------------------------

void InitializeBuffer(Buffer* buffer, unsigned int capacity, unsigned char elementSize, void* data)
{
	if(buffer == NULL)
		return;

	buffer->capacity = capacity;
	buffer->elementSize = elementSize;
	buffer->length = 0;
	buffer->data = data;
}

// RINGBUFFER FUNCTIONS--------------------------------------------------------

void InitializeRingBuffer(volatile RingBuffer* buffer,
						  unsigned int capacity, unsigned char elementSize, void* data)
{
	if(buffer == NULL)
		return;

	buffer->capacity = capacity;
	buffer->elementSize = elementSize;
	buffer->length = 0;
	buffer->head = 0;
	buffer->tail = 0;
	buffer->data = data;
}

void RingBufferSimpleEnqueue(volatile RingBuffer* buffer)
{
	if(buffer == NULL)
		return;

	// Adjust the head pointer
	buffer->head = buffer->head + 1 == buffer->capacity ? 0 : buffer->head + 1;

	// If the buffer is full, adjust the tail pointer (oldest value in the buffer is overwritten)
	if(buffer->length == buffer->capacity)
		buffer->tail = buffer->tail + 1 == buffer->capacity ? 0 : buffer->tail + 1;
	else
		buffer->length++;
}

void RingBufferEnqueue(volatile RingBuffer* buffer, void* sourceValue)
{
	if(buffer == NULL)
		return;

	// Copy data then adjust the head pointer
	if(buffer->elementSize == 1)
	{
		((uint8_t*) buffer->data)[buffer->head] = (uint8_t) sourceValue;
	}
	else
	{
		uint16_t i;
		for(i = 0; i < buffer->elementSize; i++)
			((uint8_t*) buffer->data)[(buffer->head * buffer->elementSize) + i] = ((uint8_t*) sourceValue)[i];
	}
	buffer->head = buffer->head + 1 == buffer->capacity ? 0 : buffer->head + 1;

	// If the buffer is full, adjust the tail pointer (oldest value in the buffer is overwritten)
	if(buffer->length == buffer->capacity)
		buffer->tail = buffer->tail + 1 == buffer->capacity ? 0 : buffer->tail + 1;
	else
		buffer->length++;
}

void RingBufferSimpleDequeue(volatile RingBuffer* buffer)
{
	if(buffer == NULL || buffer->length == 0)
		return;

	// Adjust the tail pointer
	buffer->tail = buffer->tail + 1 == buffer->capacity ? 0 : buffer->tail + 1;
	buffer->length--;
}

void RingBufferDequeue(volatile RingBuffer* buffer, void* destinationValue)
{
	if(buffer == NULL || buffer->length == 0 || destinationValue == NULL)
		return;

	// Copy data then adjust the tail pointer
	if(buffer->elementSize == 1)
	{
		*((uint8_t*) destinationValue) = ((uint8_t*) buffer->data)[buffer->tail];
	}
	else
	{
		uint16_t i;
		for(i = 0; i < buffer->elementSize; i++)
			((uint8_t*) destinationValue)[i] = ((uint8_t*) buffer->data)[(buffer->tail * buffer->elementSize) + i];
	}
	buffer->tail = buffer->tail + 1 == buffer->capacity ? 0 : buffer->tail + 1;
	buffer->length--;
}

// PARSING FUNCTIONS-----------------------------------------------------------

bool BufferEquals(Buffer* buffer, const void* value, unsigned int valueLength)
{
	if(buffer == NULL || value == NULL || buffer->length != valueLength)
		return false;

	unsigned int i, j;
	for(i = 0; i < buffer->length; i++)
	{
		for(j = 0; j < buffer->elementSize; j++)
		{
			if(((uint8_t*) buffer->data)[(i * buffer->elementSize) + j]
			!= ((uint8_t*) value)[(i * buffer->elementSize) + j])
				return false;
		}
	}
	return true;
}

bool BufferContains(Buffer* buffer, const void* value, unsigned int valueLength)
{
	if(buffer == NULL || value == NULL || buffer->length < valueLength || valueLength == 0)
		return false;

	unsigned int i = 0, j , k, offset;
S1:{
	offset = 1;
	for(i; i < buffer->length; i++)
		{
			for(j = 0, k = 0; j < buffer->elementSize; j++, k++)
			{
				if(((uint8_t*) buffer->data)[(i * buffer->elementSize) + j] != ((uint8_t*) value)[j])
					break;
			}

			if(k == buffer->elementSize)
			{
				i++;
				break;
			}
		}
	}

	for(i; i < buffer->length; i++, offset++)
	{
		for(j = 0; j < buffer->elementSize; j++)
		{
			if(((uint8_t*) buffer->data)[(i * buffer->elementSize) + j]
			!= ((uint8_t*) value)[(offset * buffer->elementSize) + j])
			{
				i++;
				goto S1;
			}
		}

		if(offset == valueLength - 1)
			return true;
	}
	return false;
}

// OLD-------------------------------------------------------------------------

void RingBufferU8Create(volatile RingBufferU8* buffer, uint16_t bufferSize, char* data)
{
	buffer->bufferSize = bufferSize;
	buffer->length = 0;
	buffer->head = bufferSize - 1;
	buffer->tail = 0;
	buffer->data = data;
}

void RingBufferU8Enqueue(volatile RingBufferU8* buffer, char data)
{
	if(buffer->length == buffer->bufferSize)	// Overflow hack
		return;
	buffer->head = (buffer->head + 1) % buffer->bufferSize;
	buffer->data[buffer->head] = data;
	buffer->length++;
}

char RingBufferU8Dequeue(volatile RingBufferU8* buffer)
{
	// NOTE: Does not check if buffer is empty.
	// It is essential that this be done somewhere, but was excluded here for performance reasons.
	char data = buffer->data[buffer->tail];
	buffer->tail = (buffer->tail + 1) % buffer->bufferSize;
	buffer->length--;
	return data;
}

char RingBufferU8DequeuePeek(volatile RingBufferU8* buffer)
{
	return buffer->data[buffer->tail];
}

void RingBufferU8RemoveLast(volatile RingBufferU8* buffer, uint16_t count)
{
	if(buffer->length == 0)
		return;
	if(count > buffer->length)
		count = buffer->length;
	buffer->head = (buffer->head + (buffer->bufferSize - count)) % buffer->bufferSize;
	buffer->length -= count;
}