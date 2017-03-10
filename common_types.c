/* Project:	SmartModule
 * File:	common_types.c
 * Author:	Jonathan Ruisi
 * Created:	March 17, 2016, 2:35 AM
 */

#include "common_types.h"

Buffer BufferCreate(uint16_t bufferSize, char* bufferData)
{
	Buffer buffer;
	buffer.bufferSize = bufferSize;
	buffer.length = 0;
	buffer.data = bufferData;
	return buffer;
}