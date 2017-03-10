/* Project:	SmartModule
 * File:	sram.h
 * Author:	Jonathan Ruisi
 * Created:	February 9, 2017, 4:39 AM
 */

#ifndef SRAM_H
#define SRAM_H

#include <stdint.h>

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
// Status flags
#define SRAM_STATUS_BUSY	0x7		// Mask for all busy flags
// Other
#define SRAM_CAPACITY		0x20000	// 131072 Bytes
#define SRAM_BUFFER_SIZE	256		// Bytes for each rx and tx buffer
#define DMA_MAX_TRANSFER	0x400	// 1024 bytes maximum DMA transfer

// TYPE DEFINITIONS------------------------------------------------------------

typedef union
{

	struct
	{
		unsigned holdDisabled : 1;
		unsigned : 5;
		unsigned mode : 2;
	} ;
	uint8_t value;
} SramMode;

typedef struct
{

	union
	{

		struct
		{
			unsigned isBusy : 1;
			unsigned isCommand : 1;
			unsigned isReading : 1;
			unsigned isWriting : 1;
			unsigned isFilling : 1;
			unsigned keepEnabled : 1;
			unsigned hasUnreadData : 1;
			unsigned : 1;
		} statusBits;
		uint8_t status;
	} ;
	uint24_t dataLength;
	uint24_t readAddress;
	uint24_t writeAddress;
} SramStatus;

typedef struct
{

	struct
	{
		uint8_t command;

		union
		{
			SramMode mode;

			struct
			{
				uint8_t upper;
				uint8_t high;
				uint8_t low;
			} addressBytes;
			uint24_t address;
		} ;
	} initialization;
	uint8_t data[SRAM_BUFFER_SIZE];
} SramPacket;

// GLOBAL VARIABLES------------------------------------------------------------
extern volatile SramStatus _sramStatus;
extern volatile SramPacket _sramPacket;

// FUNCTION PROTOTYPES---------------------------------------------------------
SramStatus SramStatusCreate(void);
void SramSetMode(SramMode mode);
void SramRead(uint24_t address, uint24_t length);
void SramReadNext(uint24_t length);
void _SramRead(void);
void SramWrite(uint24_t address, const void* data, uint24_t length);
void SramWriteNext(const void* data, uint24_t length);
void _SramWrite(void);
void SramFill(uint24_t address, uint24_t length, uint8_t value);
void _SramFill(void);
void _SramCommandAddress(void);
void SramUpdate(volatile SramStatus* status);
#endif