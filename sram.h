/**@file		sram.h
 * @brief		Header file for SRAM control
 * @author		Jonathan Ruisi
 * @version		1.0
 * @date		February 9, 2017
 * @warning		Implemented using, and only tested with, the **N01S830HA-D** SRAM IC from *On Semiconductor*
 * @copyright	GNU Public License
 */

#ifndef SRAM_H
#define SRAM_H

#include "buffer.h"
#include "utility.h"

/**
 * ## SRAM Allocation Table (131072 bytes)
 *
 * Address Range	| Contents
 * -----------------|-----------------
 * 0x00000 - 0x00FFF| Comm1 Line Buffer
 * 0x01000 - 0x01FFF| Comm2 Line Buffer
 * 0x02000 - 0x05FFF| Usage (Load) History
 * 0x06000 - 0x06FFF| Temperature History
 * 0x07000 - 0x07FFF| Proximity Event History
 * 0x08000 - 0x09FFF| System Event History
 * 0x0A000 - 0x0AFFF| Relay State History
 * 0x0B000 - 0x0CFFF| Command History
 * 0x0D000 - 0x1FFFF| **Unallocated**
 */

// DEFINITIONS ----------------------------------------------------------------
// Commands
#define SRAM_COMMAND_READ	0x03	/**< Read data from memory starting at selected address */
#define SRAM_COMMAND_WRITE	0x02	/**< Write (program) data to memory starting at selected address */
#define SRAM_COMMAND_EQIO	0x38	/**< Enable QUAD I/O access */
#define SRAM_COMMAND_EDIO	0x3B	/**< Enable DUAL I/O access */
#define SRAM_COMMAND_RSTQIO	0xFF	/**< Reset from QUAD and DUAL to SPI I/O access */
#define SRAM_COMMAND_RDMR	0x05	/**< Read mode register */
#define SRAM_COMMAND_WRMR	0x01	/**< Write mode register */
// Mode Flags
#define SRAM_MODE_WORD		0b00	/**< Word mode (Write one byte at a time) */
#define SRAM_MODE_PAGE		0b10	/**< Page mode (Sequential writes up to 32B) */
#define SRAM_MODE_BURST		0b01	/**< Burst mode (Unlimited sequential writes) */
// Size Limits
#define SRAM_CAPACITY		0x20000	/**< 131072 Bytes */
#define SRAM_BUFFER_SIZE	256		/**< Bytes for each rx and tx buffer */
#define DMA_MAX_TRANSFER	0x400	/**< 1024 bytes maximum DMA transfer */
// SRAM Operations
#define SRAM_OP_COMMAND		0x1		/**< SRAM current operation: COMMAND */
#define	SRAM_OP_FILL		0x2		/**< SRAM current operation: FILL */
#define SRAM_OP_READ		0x3		/**< SRAM current operation: READ */
#define	SRAM_OP_WRITE		0x4		/**< SRAM current operation: WRITE */

// TYPE DEFINITIONS -----------------------------------------------------------

/**
 * A structure used to read and write to the SRAM mode register
 */
typedef union
{

	struct
	{
		unsigned holdDisabled : 1;	/**< Gets/Sets whether or not the SRAM HOLD feature is enabled */
		unsigned : 5;
		unsigned mode : 2;			/**< Gets/Sets the SRAM write mode */
	} ;
	unsigned char value;
} SramMode;

/**
 * A structure containing everything necessary to control, read, and write to the SRAM
 * @see documentation describing how the data packets are composed
 */
typedef struct Sram
{

	struct
	{
		unsigned char command;					/**< The command to be sent to the SRAM */

		union
		{
			SramMode mode;						/**< The current SRAM write mode */

			struct
			{
				unsigned char upper;			/**< The MSB of the SRAM addressable space */
				unsigned char high;				/**< The HIGH byte of the SRAM addressable space */
				unsigned char low;				/**< The LSB of the SRAM addressable space */
			} addressBytes;
			unsigned short long int address;	/**< The full 24b SRAM address */
		} ;
		char fillValue;							/**< This value will be written during a fill operation */
	} initialization;

	union
	{

		struct
		{
			unsigned busy : 1;					/**< Indicates that the SRAM is currently performing an operation */
			unsigned currentOperation : 3;		/**< The current operation being performed */
			unsigned : 4;
		} statusBits;
		unsigned char status;
	} ;

	Buffer* targetBuffer;						/**< Pointer to a location in local RAM where the DMA read/write will take place */
	unsigned short long int dataLength;			/**< Number of buffer elements to read/write */
	unsigned short long int bytesRemaining;		/**< Number of bytes remaining to be read/written */
	unsigned short long int readAddress;		/**< SRAM Address of current read operation */
	unsigned short long int writeAddress;		/**< SRAM Address of current write operation */
	unsigned long int startTime;				/**< Time stamp of the start of the operation */
} Sram;

// CONSTANTS ------------------------------------------------------------------
SCUINT24 SRAM_ADDR_COMM1_LINE_QUEUE = 0x000000;	/**< SRAM memory allocation: Comm1 Line Queue */
SCUINT24 SRAM_ADDR_COMM2_LINE_QUEUE = 0x010000;	/**< SRAM memory allocation: Comm2 Line Queue */
SCUINT24 SRAM_ADDR_LOAD_QUEUE		= 0x020000;	/**< SRAM memory allocation: Load measurement history */

// GLOBAL VARIABLES -----------------------------------------------------------
extern volatile Sram _sram;

// FUNCTION PROTOTYPES --------------------------------------------------------
// Initialization
void SramStatusInitialize(void);
// SRAM User Callable Functions
void SramSetMode(SramMode mode);
void SramRead(unsigned short long int address, unsigned short long int length, Buffer* destination);
void SramWrite(unsigned short long int address, Buffer* source);
void SramFill(unsigned short long int address, unsigned short long int length, unsigned char value);
// SRAM Callback Functions
void _SramOperationStart(void);
void _SramReadBytes(void);
void _SramWriteBytes(void);
void _SramFill(void);

#endif