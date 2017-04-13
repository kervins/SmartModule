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
	unsigned char elementSize;
	unsigned int capacity;
	unsigned int length;
	void* data;
} Buffer;

// Buffers that can be used as a circular queue (data is processed FIFO)

typedef struct RingBuffer
{
	unsigned char elementSize;
	unsigned int capacity;
	unsigned int length;
	unsigned int head;
	unsigned int tail;
	void* data;
} RingBuffer;

// FUNCTION PROTOTYPES---------------------------------------------------------
void CreateBuffer(Buffer* buffer, unsigned int capacity, unsigned char elementSize, void* data);

// OLD-------------------------------------------------------------------------
// TYPE DEFINITIONS------------------------------------------------------------
// Basic buffer containing unsigned char data with a maximum size of 65536 bytes

typedef struct BufferU8
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
} BufferU16;

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
void BufferU8Create(BufferU8* buffer, unsigned int bufferSize, char* bufferData);
void BufferU16Create(BufferU16* buffer, unsigned int bufferSize, unsigned int* bufferData);
// RingBuffer (volatile)
void RingBufferCreate(volatile RingBufferU8* buffer, unsigned int bufferSize, char* data);
void RingBufferEnqueue(volatile RingBufferU8* buffer, char data);
char RingBufferDequeue(volatile RingBufferU8* buffer);
char RingBufferDequeuePeek(volatile RingBufferU8* buffer);
void RingBufferRemoveLast(volatile RingBufferU8* buffer, unsigned int count);
// Parsing Functions
bool BufferEquals(BufferU8* line, const char* str);
bool BufferContains(BufferU8* line, const char* str);
bool BufferStartsWith(BufferU8* line, const char* str);
bool BufferEndsWith(BufferU8* line, const char* str);

#endif