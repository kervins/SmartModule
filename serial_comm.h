/* Project:	SmartModule
 * File:	serial_comm.h
 * Author:	Jonathan Ruisi
 * Created:	February 14, 2017, 2:22 AM
 */

#ifndef SERIAL_COMM_H
#define SERIAL_COMM_H

#include "utility.h"
#include "buffer.h"
#include "linked_list.h"

// MACROS (Calculates SPBRG values for USART baud rate generator)--------------
// Each definition contains a value that is to be loaded into the BRG registers (SPBRGHx:SPBRGx)
#define CALCULATE_BRG(rate)		((FOSC/rate)/64)-1	// BRG16 = 0, BRGH = 0
#define CALCULATE_BRG_H(rate)	((FOSC/rate)/16)-1	// BRG16 = 0, BRGH = 1
#define CALCULATE_BRG_16(rate)	((FOSC/rate)/16)-1	// BRG16 = 1, BRGH = 0
#define CALCULATE_BRG_16H(rate)	((FOSC/rate)/4)-1	// BRG16 = 1, BRGH = 1

// DEFINITIONS-----------------------------------------------------------------
#define XOFF_THRESHOLD	(3 * RX_BUFFER_SIZE) / 4
#define XON_THRESHOLD	RX_BUFFER_SIZE / 4
#define SEQ_MAX_PARAMS	8

// ENUMERATED TYPES------------------------------------------------------------

typedef enum
{
	NEWLINE_CR		= 1,
	NEWLINE_LF		= 2,
	NEWLINE_CRLF	= 3
} NewlineFlags;

// TYPE DEFINITIONS------------------------------------------------------------
typedef void (*CommAction)(struct CommPort*) ;

typedef struct CommDataRegisters
{
	uint8_t volatile* const pTxReg;
	TXSTAbits_t volatile* const pTxSta;
	uint8_t volatile* const pPie;
	uint8_t const txieBit;
} CommDataRegisters;

typedef struct ExternalRingBufferU8
{
	uint24_t baseAddress;
	uint16_t blockSize;
	volatile RingBufferU8 buffer;
} ExternalRingBufferU8;

typedef struct CommPort
{

	union
	{

		struct
		{
			unsigned isTxFlowControl : 1;
			unsigned isRxFlowControl : 1;
			unsigned isTxPaused : 1;
			unsigned isRxPaused : 1;
			unsigned hasLine : 1;
			unsigned hasSequence : 1;
			unsigned : 2;
		} statusBits;
		uint8_t status;
	} ;

	union
	{

		struct
		{
			unsigned echoRx : 1;
			unsigned echoNewline : 1;
			unsigned echoSequence : 1;
			unsigned useExternalBuffer : 1;
			unsigned ignoreRx : 1;
			unsigned isBinaryMode : 1;
			unsigned : 2;
		} modeBits;
		uint8_t mode;
	} ;

	struct
	{
		NewlineFlags rx;
		NewlineFlags tx;
		NewlineFlags inProgress;
	} newline;

	struct
	{

		union
		{

			struct
			{
				unsigned csiCharCount : 2;
				unsigned sequenceId : 6;
			} statusBits;
			uint8_t status;
		} ;
		uint8_t paramCount;
		uint8_t params[SEQ_MAX_PARAMS];
		uint8_t terminator;
	} sequence;

	struct
	{
		volatile RingBufferU8 tx;
		volatile RingBufferU8 rx;
		BufferU8 line;
	} buffers;

	Point cursor;
	ExternalRingBufferU8 external;
	const CommDataRegisters* registers;
} CommPort;

// FUNCTION PROTOTYPES---------------------------------------------------------
void CommPortInitialize(CommPort* comm,
						uint16_t txBufferSize, uint16_t rxBufferSize, uint16_t lineBufferSize,
						char* txData, char* rxData, char* lineData,
						uint24_t lineQueueBaseAddress, uint16_t lineQueueSize, uint8_t* lineQueueData,
						NewlineFlags txNewline, NewlineFlags rxNewline,
						const CommDataRegisters* registers,
						bool enableFlowControl, bool enableEcho,
						unsigned char echoRow, unsigned char echoColumn);
void UpdateCommPort(CommPort* comm);
void CommFlushLineBuffer(CommPort* comm);
void CommResetSequence(CommPort* comm);
void CommPutChar(CommPort* comm, char data);
void CommPutString(CommPort* comm, const char* str);
void CommPutNewline(CommPort* comm);
void CommPutBuffer(CommPort* comm, BufferU8* source);
void CommPutSequence(CommPort* comm, unsigned char terminator, unsigned char paramCount, ...);
// Linked List Print Functions
void CommPutLinkedListChars(LinkedList_16Element*, CommPort*);
void CommPrintLinkedListInfo(LinkedList_16Element*, CommPort*);

#endif