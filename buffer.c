/* Project:	SmartModule
 * File:	buffer.c
 * Author:	Jonathan Ruisi
 * Created:	April 13, 2017, 5:56 PM
 */

#include <stdint.h>
#include <stdlib.h>
#include "buffer.h"

// NEW BUFFER FUNCTIONS--------------------------------------------------------

void CreateBuffer(Buffer* buffer, unsigned int capacity, unsigned char elementSize, void* data)
{
	if(buffer == NULL)
		return;

	buffer->capacity = capacity;
	buffer->elementSize = elementSize;
	buffer->length = 0;
	buffer->data = data;
}

// NEW RINGBUFFER FUNCTIONS----------------------------------------------------

// BUFFER FUNCTIONS------------------------------------------------------------

void BufferU8Create(BufferU8* buffer, uint16_t bufferSize, char* bufferData)
{
	buffer->bufferSize = bufferSize;
	buffer->length = 0;
	buffer->data = bufferData;
}

void BufferU16Create(BufferU16* buffer, uint16_t bufferSize, uint16_t* bufferData)
{
	buffer->bufferSize = bufferSize;
	buffer->length = 0;
	buffer->data = bufferData;
}

// RINGBUFFER FUNCTIONS (VOLATILE)---------------------------------------------

void RingBufferCreate(volatile RingBufferU8* buffer, uint16_t bufferSize, char* data)
{
	buffer->bufferSize = bufferSize;
	buffer->length = 0;
	buffer->head = bufferSize - 1;
	buffer->tail = 0;
	buffer->data = data;
}

void RingBufferEnqueue(volatile RingBufferU8* buffer, char data)
{
	if(buffer->length == buffer->bufferSize)	// Overflow hack
		return;
	buffer->head = (buffer->head + 1) % buffer->bufferSize;
	buffer->data[buffer->head] = data;
	buffer->length++;
}

char RingBufferDequeue(volatile RingBufferU8* buffer)
{
	// NOTE: Does not check if buffer is empty.
	// It is essential that this be done somewhere, but was excluded here for performance reasons.
	char data = buffer->data[buffer->tail];
	buffer->tail = (buffer->tail + 1) % buffer->bufferSize;
	buffer->length--;
	return data;
}

char RingBufferDequeuePeek(volatile RingBufferU8* buffer)
{
	return buffer->data[buffer->tail];
}

void RingBufferRemoveLast(volatile RingBufferU8* buffer, uint16_t count)
{
	if(buffer->length == 0)
		return;
	if(count > buffer->length)
		count = buffer->length;
	buffer->head = (buffer->head + (buffer->bufferSize - count)) % buffer->bufferSize;
	buffer->length -= count;
}

// PARSING FUNCTIONS ----------------------------------------------------------

bool BufferEquals(BufferU8* line, const char* str)
{
	int index = 0;
	for(index = 0; index < line->length && line->data[index] == str[index]; index++);
	return index == line->length;
}

bool BufferContains(BufferU8* line, const char* str)
{
	int index = 0, offset = 0;
	while(index < line->length && line->data[index] != str[0])
	{
		index++;
	}
	while(index < line->length && line->data[index] == str[offset])
	{
		index++;
		offset++;
		if(str[offset] == 0)	// ASCII null character
			return true;
	}
	return false;
}

bool BufferStartsWith(BufferU8* line, const char* str)
{
	int index = 0;
	while(index < line->length && line->data[index] == str[index])
	{
		index++;
		if(str[index] == 0)	// ASCII null character
			return true;
	}
	return false;
}

bool BufferEndsWith(BufferU8* line, const char* str)
{
	int index = 0, offset = 0;
	while(index < line->length && line->data[index] != str[0])
	{
		index++;
	}
	while(index < line->length && line->data[index] == str[offset])
	{
		index++;
		offset++;
	}
	return index == line->length;
}