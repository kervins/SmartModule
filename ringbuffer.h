/* Project:	SmartModule
 * File:	ringbuffer.h
 * Author:	Jonathan Ruisi
 * Created:	February 14, 2017, 2:10 AM
 */

#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <stdint.h>
#include <stdbool.h>
#include "common_types.h"

// TYPE DEFINITIONS------------------------------------------------------------
typedef struct RingBufferIterator RingBufferIterator;
typedef void (*RBIAction)(RingBufferIterator*) ;
typedef void (*RBIAction_U8)(RingBufferIterator*, uint8_t);
typedef uint8_t (*U8_RBIFunc)(RingBufferIterator*);
typedef bool (*B_RBIFunc)(RingBufferIterator*);

typedef struct RingBuffer
{
	uint16_t bufferSize;
	uint16_t length;
	uint16_t head;
	uint16_t tail;
	char* data;
} RingBuffer;

typedef struct RingBufferIterator
{
	RingBuffer* buffer;
	uint16_t index;
	RBIAction Start;
	RBIAction_U8 SetCurrent;
	U8_RBIFunc GetCurrent;
	B_RBIFunc Next;
} RingBufferIterator;

// FUNCTION PROTOTYPES---------------------------------------------------------
RingBuffer RingBufferCreate(uint16_t bufferSize, char* data);
void RingBufferEnqueue(volatile RingBuffer* buffer, char data);
char RingBufferDequeue(volatile RingBuffer* buffer);
void RingBufferRemoveLast(volatile RingBuffer* buffer, uint16_t count);
// Iterator Functions
RingBufferIterator RingBufferCreateIterator(RingBuffer* buffer);
void _RBIStart(RingBufferIterator* iterator);
void _RBISetCurrent(RingBufferIterator* iterator, uint8_t value);
uint8_t _RBIGetCurrent(RingBufferIterator* iterator);
bool _RBINext(RingBufferIterator* iterator);
#endif