/* Project:	SmartModule
 * File:	buffer.h
 * Author:	Jonathan Ruisi
 * Created:	April 13, 2017, 5:56 PM
 */

#ifndef BUFFER_H
#define BUFFER_H

#include <stdbool.h>

// TYPE DEFINITIONS------------------------------------------------------------
typedef void (*RingBufferAction)(struct RingBuffer*, void*) ;

typedef struct Buffer
{
	unsigned char elementSize;	// Size (bytes) of each element
	unsigned int capacity;		// Buffer capacity (elements)
	unsigned int length;		// Number of elements currently in the buffer
	void* data;					// Pointer to an array of elements
} Buffer;

// Buffers that can be used as a circular queue (data is processed FIFO)

typedef struct RingBuffer
{
	unsigned char elementSize;		// Size (bytes) of each element
	unsigned int capacity;			// Buffer capacity (elements)
	unsigned int length;			// Number of elements currently in the buffer
	unsigned int head;				// Index of the head element
	unsigned int tail;				// Index of the tail element
	RingBufferAction enqueueAction;	// Executes when data is queued into the buffer
	RingBufferAction dequeueAction;	// Executes when data is dequeued out of the buffer
	void* data;						// Pointer to an array of elements
} RingBuffer;

// FUNCTION PROTOTYPES---------------------------------------------------------
// Buffer
void InitializeBuffer(Buffer* buffer, unsigned int capacity, unsigned char elementSize, void* data);
// RingBuffer
void InitializeRingBuffer(volatile RingBuffer* buffer,
						  unsigned int capacity, unsigned char elementSize, void* data);
void RingBufferSimpleEnqueue(volatile RingBuffer* buffer);
void RingBufferSimpleDequeue(volatile RingBuffer* buffer);
void RingBufferEnqueue(volatile RingBuffer* buffer, void* sourceValue);
void RingBufferDequeue(volatile RingBuffer* buffer, void* destinationValue);
// Parsing
bool BufferEquals(Buffer* buffer, const void* value, unsigned int valueLength);
bool BufferContains(Buffer* buffer, const void* value, unsigned int valueLength);

// OLD-------------------------------------------------------------------------
// TYPE DEFINITIONS------------------------------------------------------------
// Basic buffer containing unsigned char data with a maximum size of 65536 bytes

/*typedef struct BufferU8
{
	unsigned int bufferSize;
	unsigned int length;
	unsigned char* data;
} BufferU8;

typedef struct BufferU16
{
	unsigned int bufferSize;
	unsigned int length;
	unsigned int* data;
} BufferU16;*/

// Buffers that can be used as a circular queue (data is processed FIFO)

typedef struct RingBufferU8
{
	unsigned int bufferSize;
	unsigned int length;
	unsigned int head;
	unsigned int tail;
	char* data;
} RingBufferU8;

// FUNCTION PROTOTYPES---------------------------------------------------------
// Buffer
//void BufferU8Create(BufferU8* buffer, unsigned int bufferSize, char* bufferData);
//void BufferU16Create(BufferU16* buffer, unsigned int bufferSize, unsigned int* bufferData);
// RingBuffer (volatile)
void RingBufferU8Create(volatile RingBufferU8* buffer, unsigned int bufferSize, char* data);
void RingBufferU8Enqueue(volatile RingBufferU8* buffer, char data);
char RingBufferU8Dequeue(volatile RingBufferU8* buffer);
char RingBufferU8DequeuePeek(volatile RingBufferU8* buffer);
void RingBufferU8RemoveLast(volatile RingBufferU8* buffer, unsigned int count);

#endif