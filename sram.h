/* Project:	SmartModule
 * File:	sram.h
 * Author:	Jonathan Ruisi
 * Created:	February 9, 2017, 4:39 AM
 */

#ifndef SRAM_H
#define SRAM_H

#include "buffer.h"

// DEFINITIONS-----------------------------------------------------------------
// Commands
#define SRAM_COMMAND_READ	0x03	// Read data from memory starting at selected address
#define SRAM_COMMAND_WRITE	0x02	// Write (program) data to memory starting at selected address
#define SRAM_COMMAND_EQIO	0x38	// Enable QUAD I/O access
#define SRAM_COMMAND_EDIO	0x3B	// Enable DUAL I/O access
#define SRAM_COMMAND_RSTQIO	0xFF	// Reset from QUAD and DUAL to SPI I/O access
#define SRAM_COMMAND_RDMR	0x05	// Read mode register
#define SRAM_COMMAND_WRMR	0x01	// Write mode register
// Mode flags
#define SRAM_MODE_WORD		0b00	// Word mode (Write one byte at a time)
#define SRAM_MODE_PAGE		0b10	// Page mode (Sequential writes up to 32B)
#define SRAM_MODE_BURST		0b01	// Burst mode (Unlimited sequential writes)
// Size limits
#define SRAM_CAPACITY		0x20000	// 131072 Bytes
#define SRAM_BUFFER_SIZE	256		// Bytes for each rx and tx buffer
#define DMA_MAX_TRANSFER	0x400	// 1024 bytes maximum DMA transfer
// SRAM operations
#define SRAM_OP_COMMAND		0x1
#define	SRAM_OP_FILL		0x2
#define SRAM_OP_READ		0x3
#define	SRAM_OP_WRITE		0x4

// TYPE DEFINITIONS------------------------------------------------------------

typedef union
{

	struct
	{
		unsigned holdDisabled : 1;
		unsigned : 5;
		unsigned mode : 2;
	} ;
	unsigned char value;
} SramMode;

typedef struct Sram
{

	struct
	{
		unsigned char command;

		union
		{
			SramMode mode;

			struct
			{
				unsigned char upper;
				unsigned char high;
				unsigned char low;
			} addressBytes;
			unsigned short long int address;
		} ;
		char fillValue;
	} initialization;

	union
	{

		struct
		{
			unsigned busy : 1;
			unsigned currentOperation : 3;
			unsigned : 4;
		} statusBits;
		unsigned char status;
	} ;

	BufferU8* targetBuffer;
	unsigned short long int dataLength;
	unsigned short long int bytesRemaining;
	unsigned short long int readAddress;
	unsigned short long int writeAddress;
	unsigned long int startTime;
} Sram;

typedef struct MemoryAddressRingBuffer
{
	unsigned int blockSize;
	unsigned int maxBlockCount;
	unsigned short long int baseAddress;
	unsigned short long int length;
	unsigned short long int head;
	unsigned short long int tail;
} MemoryAddressRingBuffer;

// GLOBAL VARIABLES------------------------------------------------------------
extern volatile Sram _sram;

// FUNCTION PROTOTYPES---------------------------------------------------------
// Initialization
void SramStatusInitialize(void);
// SRAM User Callable Functions
void SramSetMode(SramMode mode);
void SramReadBytes(unsigned short long int address, unsigned short long int length, BufferU8* destination);
void SramWriteBytes(unsigned short long int address, BufferU8* source);
void SramFill(unsigned short long int address, unsigned short long int length, unsigned char value);
// SRAM Callback Functions
void _SramOperationStart(void);
void _SramReadBytes(void);
void _SramWriteBytes(void);
void _SramFill(void);
#endif