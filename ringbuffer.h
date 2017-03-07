/* Project:	SmartModule
 * File:	ringbuffer.h
 * Author:	Jonathan Ruisi
 * Created:	February 14, 2017, 2:10 AM
 */

#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <stdint.h>

// TYPE DEFINITIONS------------------------------------------------------------

typedef struct RingBuffer
{
	uint16_t bufferSize;
	uint16_t length;
	uint16_t head;
	uint16_t tail;
	char* data;
} RingBuffer;

// FUNCTION PROTOTYPES---------------------------------------------------------
RingBuffer RingBufferCreate(uint16_t bufferSize, char* data);
void RingBufferReset(volatile RingBuffer* buffer);
void RingBufferEnqueue(volatile RingBuffer* buffer, char data);
char RingBufferDequeue(volatile RingBuffer* buffer);
void RingBufferRemoveLast(volatile RingBuffer* buffer, uint16_t count);
char RingBufferPeek(volatile RingBuffer* buffer, uint16_t index);

#endif