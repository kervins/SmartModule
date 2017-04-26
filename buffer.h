/* Project:	SmartModule
 * File:	buffer.h
 * Author:	Jonathan Ruisi
 * Created:	April 13, 2017, 5:56 PM
 */

#ifndef BUFFER_H
#define BUFFER_H

#include <stdbool.h>

// TYPE DEFINITIONS------------------------------------------------------------

typedef struct Buffer
{
	unsigned int elementSize;	// Size (bytes) of each element
	unsigned int capacity;		// Buffer capacity (elements)
	unsigned int length;		// Number of elements currently in the buffer
	void* data;					// Pointer to an array of elements
} Buffer;

// Buffers that can be used as a circular queue (data is processed FIFO)

typedef struct RingBuffer
{
	const void* data;				// Pointer to an array of elements
	unsigned int elementSize;		// Size (bytes) of each element
	unsigned int capacity;			// Buffer capacity (elements)
	unsigned int length;			// Number of elements currently in the buffer
	unsigned int head;				// Index of the head element
	unsigned int tail;				// Index of the tail element
} RingBuffer;

// FUNCTION PROTOTYPES---------------------------------------------------------
// Buffer
void InitializeBuffer(Buffer* buffer, unsigned int capacity, unsigned int elementSize, void* data);
// RingBuffer
void InitializeRingBuffer(volatile RingBuffer* buffer,
						  unsigned int capacity,
						  unsigned int elementSize,
						  void* data);
void InitializeRingBufferSRAM(volatile RingBuffer* buffer,
							  unsigned int capacity,
							  unsigned int elementSize,
							  const unsigned short long int* baseAddress);
void RingBufferEnqueue(volatile RingBuffer* buffer, void* source);
void RingBufferDequeue(volatile RingBuffer* buffer, void* destination);
bool RingBufferEnqueueSRAM(volatile RingBuffer* buffer, Buffer* source);
bool RingBufferDequeueSRAM(volatile RingBuffer* buffer, Buffer* destination);
// Parsing
bool BufferEquals(Buffer* buffer, const void* value, unsigned int valueLength);
int BufferContains(Buffer* buffer, const void* value, unsigned int valueLength);
int BufferFind(Buffer* buffer, const void* value, unsigned int valueLength, unsigned int n);
Buffer BufferTrimLeft(Buffer* buffer, unsigned int value);
Buffer BufferTrimRight(Buffer* buffer, unsigned int value);
#endif