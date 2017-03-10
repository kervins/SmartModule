/* Project:	SmartModule
 * File:	shell.c
 * Author:	Jonathan Ruisi
 * Created:	February 20, 2017, 3:39 PM
 */

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include "shell.h"
#include "main.h"
#include "sram.h"
#include "serial_comm.h"
#include "wifi.h"
#include "common_types.h"
#include "ringbuffer.h"

// GLOBAL VARIABLES------------------------------------------------------------

// INITIALIZATION FUNCTIONS----------------------------------------------------

// LINE FUNCTIONS (RingBuffer Based)-------------------------------------------

bool LineContains(RingBuffer* line, const char* str)
{
	bool result = false;
	int index = 0;
	while(line->length)
	{
		char ch = RingBufferDequeue(line);
		if(ch == str[0])
		{
			result = true;
			break;
		}
		if(ch == ASCII_NUL)
			break;
	}
	while(line->length && result)
	{
		if(str[index] == ASCII_NUL)
			break;
		if(RingBufferDequeue(line) != str[++index])
			result = false;
	}
	return result;
}

bool LineContainsPeek(RingBuffer* line, const char* str)
{
	bool result = true;
	uint16_t offset = 0;
	RingBufferIterator iterator = RingBufferCreateIterator(line);
	while(iterator.GetCurrent(&iterator) != str[0] && iterator.Next(&iterator))
	{
		continue;
	}
	while(str[++offset] != ASCII_NUL && result && iterator.Next(&iterator))
	{
		if(iterator.GetCurrent(&iterator) != str[offset])
			result = false;
	}
	return result;
}

// CONSOLE FUNCTIONS-----------------------------------------------------------

void SendLineToTerminal(CommPort* comm)
{
	CommPutLine(comm, &_comm2);
}

void SendLineToWifi(CommPort* comm)
{
	CommPutLine(comm, &_comm1);
}