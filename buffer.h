/* Project:	SmartModule
 * File:	buffer.h
 * Author:	Jonathan Ruisi
 * Created:	February 7, 2017, 11:52 PM
 */

#ifndef BUFFER_H
#define BUFFER_H

#include <stdint.h>
#include <stdbool.h>

// TYPE DEFINITIONS------------------------------------------------------------

typedef struct _RingBuffer
{
	uint8_t bufferSize;
	uint8_t length;
	uint8_t head;
	uint8_t tail;
	char* data;
} RingBuffer;

typedef struct _LineBuffer
{
	uint8_t length;
	bool isReceiving;
	bool isComplete;
	char* data;
} LineBuffer;

// FUNCTION PROTOTYPES---------------------------------------------------------
LineBuffer LineBufferCreate(uint8_t bufferSize, char* dataBuffer);
RingBuffer RingBufferCreate(uint8_t bufferSize, char* dataBuffer);
void RingBufferEnqueue(volatile RingBuffer* buffer, char data);
char RingBufferDequeue(volatile RingBuffer* buffer);

#endif