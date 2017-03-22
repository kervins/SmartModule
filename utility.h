/* Project:	SmartModule
 * File:	utility.h
 * Author:	Jonathan Ruisi
 * Created:	March 17, 2016, 2:35 AM
 */

#ifndef UTILITY_H
#define UTILITY_H

#include <stdint.h>
#include <stdbool.h>

// MACROS----------------------------------------------------------------------
// Math
#define MIN(a,b) a<b?a:b
#define MAX(a,b) a>b?a:b
#define ABS(a)   a>=0?a:-a
#define SIGN(a)  a==0?0:a>0?1:-1

// Bit
#define bit_test(var, bit)	(var & (1 << bit))
#define bit_set(var, bit)	(var |= (1 << bit))
#define bit_clear(var, bit)	(var &= ~(1 << bit))
#define bit_flip(var, bit)	(var ^= (1 << bit))

// Bitmask
#define bitmask_test(var, mask)		(var & (mask))
#define bitmask_set(var, mask)		(var |= (mask))
#define bitmask_clear(var, mask)	(var &= (~mask))
#define bitmask_flip(var, mask)		(var ^= (mask))

// Byte
#define GET_BYTE(value,byteIndex) (uint8_t)((value>>(8*byteIndex))&0xFF)

// DEFINITIONS (ASCII CONTROL CHARACTERS)--------------------------------------
#define ASCII_NUL	0x00	// Null Character
#define ASCII_SOH	0x01	// Start of Heading
#define ASCII_STX	0x02	// Start of Text
#define ASCII_ETX	0x03	// End of Text
#define ASCII_EOT	0x04	// End of Transmission
#define ASCII_ENQ	0x05	// Enquiry
#define ASCII_ACK	0x06	// Acknowledgement
#define ASCII_BEL	0x07	// Bell
#define ASCII_BS	0x08	// Backspace
#define ASCII_HT	0x09	// Horizontal Tab
#define ASCII_LF	0x0A	// Line Feed
#define ASCII_VT	0x0B	// Vertical Tab
#define ASCII_FF	0x0C	// Form Feed
#define ASCII_CR	0x0D	// Carriage Return
#define ASCII_SO	0x0E	// Shift Out / X-On
#define ASCII_SI	0x0F	// Shift In / X-Off
#define ASCII_DLE	0x10	// Data Line Escape
#define ASCII_DC1	0x11	// Device Control 1 (XON)
#define ASCII_XON	ASCII_DC1
#define ASCII_DC2	0x12	// Device Control 2
#define ASCII_DC3	0x13	// Device Control 3 (XOFF)
#define ASCII_XOFF	ASCII_DC3
#define ASCII_DC4	0x14	// Device Control 4
#define ASCII_NAK	0x15	// Negative Acknowledgement
#define ASCII_SYN	0x16	// Synchronous Idle
#define ASCII_ETB	0x17	// End of Transmit Block
#define ASCII_CAN	0x18	// Cancel
#define ASCII_EM	0x19	// End of Medium
#define ASCII_SUB	0x1A	// Substitute
#define ASCII_ESC	0x1B	// Escape
#define ASCII_FS	0x1C	// File Separator
#define ASCII_GS	0x1D	// Group Separator
#define ASCII_RS	0x1E	// Record Separator
#define ASCII_US	0x1F	// Unit Separator
#define ASCII_DEL	0x7F	// Delete

// DEFINITIONS (ANSI CONTROL SEQUENCES)----------------------------------------
// Sequences defined here by the terminating character of the sequence
#define ANSI_CUU	0x41	// Cursor up
#define ANSI_CUD	0x42	// Cursor down
#define ANSI_CUF	0x43	// Cursor forward
#define ANSI_CUB	0x44	// Cursor back

// FUNCTION POINTERS-----------------------------------------------------------
typedef void (*Action)(void) ;
typedef void (*Action_pV)(void*) ;
typedef void (*Action_U8)(uint8_t);
typedef void (*Action_U8_U8)(uint8_t, uint8_t);
typedef uint8_t (*U8_Func)(void) ;
typedef uint8_t (*U8_Func_pV)(void*) ;
typedef uint8_t (*U8_Func_U8)(void) ;
typedef uint8_t (*U8_Func_U8_U8)(uint8_t, uint8_t);

// ENUMERATED TYPES------------------------------------------------------------

typedef enum
{
	JANUARY		= 1,
	FEBRUARY	= 2,
	MARCH		= 3,
	APRIL		= 4,
	MAY			= 5,
	JUNE		= 6,
	JULY		= 7,
	AUGUST		= 8,
	SEPTEMBER	= 9,
	OCTOBER		= 10,
	NOVEMBER	= 11,
	DECEMBER	= 12
} MonthsOfYear;

typedef enum
{
	MONDAY		= 1,
	TUESDAY		= 2,
	WEDNESDAY	= 3,
	THURSDAY	= 4,
	FRIDAY		= 5,
	SATURDAY	= 6,
	SUNDAY		= 7
} DaysOfWeek;

typedef enum
{
	Read	= 0,
	Write	= 1
} IoDirection;

// STRUCTURES------------------------------------------------------------------
// Basic buffer containing unsigned char data with a maximum size of 65536 bytes

typedef struct BufferU8
{
	uint16_t bufferSize;
	uint16_t length;
	unsigned char* data;
} BufferU8;

typedef struct BufferU16
{
	uint16_t bufferSize;
	uint16_t length;
	uint16_t* data;
} BufferU16;

// Buffers that can be used as a circular queue (data is processed FIFO)

typedef struct RingBufferU8
{
	uint16_t bufferSize;
	uint16_t length;
	uint16_t head;
	uint16_t tail;
	char* data;
} RingBufferU8;

// Condenses a complete date and time into an efficient 43b structure

typedef struct DateTime
{
	unsigned second		: 6;
	unsigned minute		: 6;
	unsigned hour		: 5;
	unsigned dayOfWeek	: 3;
	unsigned day		: 5;
	unsigned month		: 4;
	unsigned year		: 14;
} DateTime;

// Structure that allows any 8b value to be interpreted as 2-digit Binary Coded Decimal

typedef union BcdTwoDigit
{

	struct
	{
		unsigned Ones : 4;
		unsigned Tens : 4;
	} ;
	uint8_t ByteValue;
} BcdTwoDigit;

// Structure that provides both the address and length of a file

typedef struct FileDescriptor
{
	uint24_t length;
	uint24_t address;
} FileDescriptor;

// FUNCTION PROTOTYPES---------------------------------------------------------
// Buffer
void BufferU8Create(BufferU8* buffer, uint16_t bufferSize, char* bufferData);
void BufferU16Create(BufferU16* buffer, uint16_t bufferSize, uint16_t* bufferData);
// RingBuffer (volatile)
void RingBufferCreate(volatile RingBufferU8* buffer, uint16_t bufferSize, char* data);
void RingBufferEnqueue(volatile RingBufferU8* buffer, char data);
char RingBufferDequeue(volatile RingBufferU8* buffer);
char RingBufferDequeuePeek(volatile RingBufferU8* buffer);
void RingBufferRemoveLast(volatile RingBufferU8* buffer, uint16_t count);
// Parsing Functions
bool BufferEquals(BufferU8* line, const char* str);
bool BufferContains(BufferU8* line, const char* str);
bool BufferStartsWith(BufferU8* line, const char* str);
bool BufferEndsWith(BufferU8* line, const char* str);

#endif