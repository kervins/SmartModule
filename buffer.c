/**@file		buffer.c
 * @brief		Implementation for linear and circular buffers
 * @author		Jonathan Ruisi
 * @version		1.0
 * @date		April 13, 2017
 * @copyright	GNU Public License
 */

#include <xc.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include "buffer.h"
#include "sram.h"

// INITIALIZATION FUNCTIONS----------------------------------------------------

/**
 * Initializes a newly created <b>Buffer</b>
 * @param buffer		Pointer to the <b>Buffer</b> to be initialized
 * @param capacity		Buffer capacity (in elements, not bytes)
 * @param elementSize	Size of buffer elements (in bytes)
 * @param data			Pointer to buffer storage (must be at least <code>capacity * elementSize</code> bytes in size)
 */
void InitializeBuffer(Buffer* buffer, unsigned int capacity, unsigned int elementSize, void* data)
{
	if(buffer == NULL)
		return;

	buffer->capacity = capacity;
	buffer->elementSize = elementSize;
	buffer->length = 0;
	buffer->data = data;
}

/**
 * Initializes a newly created <b>RingBuffer</b>
 * @param buffer		Pointer to the <b>RingBuffer</b> to be initialized
 * @param capacity		Buffer capacity (in elements, not bytes)
 * @param elementSize	Size of buffer elements (in bytes)
 * @param data			Pointer to buffer storage (must be at least <code>capacity * elementSize</code> bytes in size)
 */
void InitializeRingBuffer(volatile RingBuffer* buffer,
						  unsigned int capacity, unsigned int elementSize, void* data)
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

/**
 * Initializes a newly created <b>RingBuffer</b>, mapping it to external memory
 * @param buffer		Pointer to the <b>RingBuffer</b> to be initialized
 * @param capacity		Buffer capacity (in elements, not bytes)
 * @param elementSize	Size of buffer elements (in bytes)
 * @param baseAddress	24-bit base address of external buffer storage
 */
void InitializeRingBufferSRAM(volatile RingBuffer* buffer,
							  unsigned int capacity,
							  unsigned int elementSize,
							  const unsigned short long int* baseAddress)
{
	if(buffer == NULL)
		return;

	buffer->capacity = capacity;
	buffer->elementSize = elementSize;
	buffer->length = 0;
	buffer->head = 0;
	buffer->tail = 0;
	buffer->data = baseAddress;
}

// RINGBUFFER FUNCTIONS--------------------------------------------------------

/**
 * Adds an item to the <b>RingBuffer</b> and updates the head and tail pointers accordingly.
 * This version of the function treats the target <b>RingBuffer</b> as volatile.
 * @param buffer	Pointer to the target <b>RingBuffer</b>
 * @param source	Pointer to an item to be added to the buffer.
 *					The item's size is expected to be equal to the
 *					<code>elementSize</code> of the target buffer.
 */
void RingBufferEnqueue(volatile RingBuffer* buffer, void* source)
{
	if(buffer == NULL)
		return;

	// Copy data using a method appropriate for the element size
	if(buffer->elementSize == 1)
	{
		((uint8_t*) buffer->data)[buffer->head] = (uint8_t) source;
	}
	else if(buffer->elementSize == 2)
	{
		((uint16_t*) buffer->data)[buffer->head] = (uint16_t) source;
	}
	else if(buffer->elementSize == 3)
	{
		((uint24_t*) buffer->data)[buffer->head] = (uint24_t) source;
	}
	else if(buffer->elementSize == 4)
	{
		((uint32_t*) buffer->data)[buffer->head] = (uint32_t) source;
	}
	else
	{
		uint16_t i;
		for(i = 0; i < buffer->elementSize; i++)
			((uint8_t*) buffer->data)[(buffer->head * buffer->elementSize) + i] = ((uint8_t*) source)[i];
	}

	// Adjust the head pointer
	// If the buffer is full, adjust the tail pointer (oldest value in the buffer is overwritten)
	buffer->head = buffer->head + 1 == buffer->capacity ? 0 : buffer->head + 1;
	if(buffer->length == buffer->capacity)
		buffer->tail = buffer->tail + 1 == buffer->capacity ? 0 : buffer->tail + 1;
	else
		buffer->length++;
}

/**
 * Removes an item from the <b>RingBuffer</b> and updates the head and tail pointers accordingly.
 * This version of the function treats the target <b>RingBuffer</b> as volatile.
 * @param buffer		Pointer to the target <b>RingBuffer</b>
 * @param destination	Pointer to a destination where the item will be stored.
 *						The destination size is expected to be equal to the
 *						<code>elementSize</code> of the target buffer.
 */
void RingBufferDequeue(volatile RingBuffer* buffer, void* destination)
{
	if(buffer == NULL || destination == NULL || buffer->length == 0)
		return;

	// Copy data using a method appropriate for the element size
	if(buffer->elementSize == 1)
	{
		*((uint8_t*) destination) = ((uint8_t*) buffer->data)[buffer->tail];
	}
	else if(buffer->elementSize == 2)
	{
		*((uint16_t*) destination) = ((uint16_t*) buffer->data)[buffer->tail];
	}
	else if(buffer->elementSize == 3)
	{
		*((uint24_t*) destination) = ((uint24_t*) buffer->data)[buffer->tail];
	}
	else if(buffer->elementSize == 4)
	{
		*((uint32_t*) destination) = ((uint32_t*) buffer->data)[buffer->tail];
	}
	else
	{
		uint16_t i;
		for(i = 0; i < buffer->elementSize; i++)
			((uint8_t*) destination)[i] = ((uint8_t*) buffer->data)[(buffer->tail * buffer->elementSize) + i];
	}

	// Adjust the tail pointer and buffer length
	buffer->tail = buffer->tail + 1 == buffer->capacity ? 0 : buffer->tail + 1;
	buffer->length--;
}

/**
 * Adds an element to the <b>RingBuffer</b> and updates the head and tail pointers accordingly.
 * The element is written to external memory at a location relative to the buffer's <code>baseAddress</code>.
 * This version of the function treats the target <b>RingBuffer</b> as volatile.
 * @param buffer	Pointer to the target <b>RingBuffer</b>
 * @param source	Pointer to a <b>Buffer</b> containing the data to be stored.
 *					The <code>elementSize</code> of <code>buffer</code> and <code>source</code> do not have to be equal.
 * @return			<b>true</b> if successful, <b>false</b> otherwise
 */
bool RingBufferEnqueueSRAM(volatile RingBuffer* buffer, Buffer* source)
{
	if(_sram.statusBits.busy
	|| buffer == NULL
	|| source == NULL
	|| source->elementSize * source->length > buffer->elementSize)
		return false;

	// Calculate the SRAM address at which the write operation will begin
	uint24_t address = *((uint24_t*) buffer->data) + (buffer->elementSize * buffer->head);
	if(address + buffer->elementSize > SRAM_CAPACITY)
		return false;

	// Perform the write operation, then adjust the head pointer and buffer length
	// If the buffer is full, adjust the tail pointer (oldest value in the buffer is overwritten)
	SramWrite(address, source);
	buffer->head = buffer->head + 1 == buffer->capacity ? 0 : buffer->head + 1;
	if(buffer->length == buffer->capacity)
		buffer->tail = buffer->tail + 1 == buffer->capacity ? 0 : buffer->tail + 1;
	else
		buffer->length++;
	return true;
}

/**
 * Removes an element from the <b>RingBuffer</b> and updates the head and tail pointers accordingly.
 * The element is read from external memory at a location relative to the buffer's <code>baseAddress</code>.
 * This version of the function treats the target <b>RingBuffer</b> as volatile.
 * @param buffer		Pointer to the target <b>RingBuffer</b>
 * @param destination	Pointer to a <b>Buffer</b> which will receive the data.
 *						The <code>elementSize</code> of <code>buffer</code> and <code>destination</code>
 *						do not have to be equal, but the <code>capacity</code> of the <code>destination</code>
 *						must be large enough to store one element's worth of data.
 * @return
 */
bool RingBufferDequeueSRAM(volatile RingBuffer* buffer, Buffer* destination)
{
	if(_sram.statusBits.busy
	|| buffer == NULL
	|| destination == NULL
	|| buffer->length == 0
	|| buffer->elementSize > (destination->elementSize * destination->capacity))
		return false;

	// Calculate the SRAM address at which the read operation will begin
	uint24_t address = *((uint24_t*) buffer->data) + (buffer->elementSize * buffer->tail);
	if(address + buffer->elementSize > SRAM_CAPACITY)
		return false;

	// Perform the read operation, then adjust the tail pointer and buffer length
	SramRead(address, buffer->elementSize, destination);
	buffer->tail = buffer->tail + 1 == buffer->capacity ? 0 : buffer->tail + 1;
	buffer->length--;
	return true;
}

// BUFFER PARSING FUNCTIONS----------------------------------------------------

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

int BufferContains(Buffer* buffer, const void* value, unsigned int valueLength)
{
	if(buffer == NULL || value == NULL || buffer->length < valueLength || valueLength == 0)
		return -1;

	unsigned int i = 0, j , k, m, offset;
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
				if(valueLength == 1)
					return i;
				m = i++;
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
			return m;
	}
	return -1;
}

/**
 * Finds the <b>n</b>th occurrence of a specified value in a buffer
 * @param buffer		Pointer to the <code>Buffer</code> to be searched
 * @param value			The value to find
 * @param valueLength	The length of the search value (measured in number of elements)
 * @param n				The zero-based occurrence of <b>value</b> to find
 *						<p>For example, to find the 2nd <code>A</code> in <b>buffer</b>,
 *						set <code>value = 'A'</code> and <code>n = 1</code>
 *						(example assumes a string is being searched)
 * @return				The index in the buffer at which the <b>n</b>th occurrence was found, otherwise -1
 */
int BufferFind(Buffer* buffer, const void* value, unsigned int valueLength, unsigned int n)
{
	if(buffer == NULL || value == NULL)
		return -1;

	Buffer sub;
	InitializeBuffer(&sub, buffer->capacity, buffer->elementSize, buffer->data);
	sub.length = buffer->length;

	int index = 0, subIndex = -1;
	unsigned int count;
	for(count = 0; count <= n; count++)
	{
		subIndex = BufferContains(&sub, value, valueLength);
		if(subIndex == -1)
			return -1;
		index += subIndex + (count * valueLength);
		sub.data = ((uint8_t*) sub.data) + ((subIndex * buffer->elementSize) + (valueLength * buffer->elementSize));
	}
	return index;
}

Buffer BufferTrimLeft(Buffer* buffer, unsigned int value)
{
	if(buffer == NULL || value >= buffer->length || value == 0)
		return *buffer;

	Buffer result;
	InitializeBuffer(&result, buffer->capacity, buffer->elementSize, buffer->data);
	result.length = buffer->length - value;
	result.data = ((uint8_t*) result.data) + (result.elementSize * value);
	return result;
}

Buffer BufferTrimRight(Buffer* buffer, unsigned int value)
{
	if(buffer == NULL || value >= buffer->length || value == 0)
		return *buffer;

	Buffer result;
	InitializeBuffer(&result, buffer->capacity, buffer->elementSize, buffer->data);
	result.length = buffer->length - value;
	return result;
}