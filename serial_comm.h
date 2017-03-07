/* Project:	SmartModule
 * File:	serial_comm.h
 * Author:	Jonathan Ruisi
 * Created:	February 14, 2017, 2:22 AM
 */

#ifndef SERIAL_COMM_H
#define SERIAL_COMM_H

#include "common_types.h"
#include "ringbuffer.h"

// MACROS (Calculates SPBRG values for USART baud rate generator)--------------
// Each definition contains a value that is to be loaded into the BRG registers (SPBRGHx:SPBRGx)
#define CALCULATE_BRG(rate)		((FOSC/rate)/64)-1	// BRG16 = 0, BRGH = 0
#define CALCULATE_BRG_H(rate)	((FOSC/rate)/16)-1	// BRG16 = 0, BRGH = 1
#define CALCULATE_BRG_16(rate)	((FOSC/rate)/16)-1	// BRG16 = 1, BRGH = 0
#define CALCULATE_BRG_16H(rate)	((FOSC/rate)/4)-1	// BRG16 = 1, BRGH = 1

// DEFINITIONS-----------------------------------------------------------------
#define TX_BUFFER_SIZE		64
#define RX_BUFFER_SIZE		256
#define LINE_BUFFER_SIZE	120
#define XOFF_THRESHOLD		(3 * RX_BUFFER_SIZE) / 4
#define XON_THRESHOLD		RX_BUFFER_SIZE / 4

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

// TYPE DEFINITIONS------------------------------------------------------------
typedef struct CommPort CommPort;		// Forward declaration for CommPort struct
typedef void (*CommAction)(CommPort*) ;

typedef enum
{
	CR_LF	= 1,
	CR_ONLY	= ASCII_CR,
	LF_ONLY	= ASCII_LF
} LineTermination;

typedef struct
{
	uint16_t bufferSize;
	uint16_t length;
	char* data;
} LineBuffer;

typedef struct
{
	uint8_t volatile* const pTxReg;
	TXSTAbits_t volatile* const pTxSta;
	uint8_t volatile* const pPie;
	uint8_t const txieBit;
} CommDataRegisters;

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
			unsigned : 3;
		} statusBits;
		uint8_t status;
	} ;
	LineTermination txLineTermination;
	LineTermination rxLineTermination;
	uint8_t delimCount;
	CommAction RxAction;
	CommAction LineAction;
	uint24_t rxByteCount;
	const CommDataRegisters * registers;
	RingBuffer volatile txBuffer;
	RingBuffer volatile rxBuffer;
	RingBuffer lineBuffer;
} CommPort;

// FUNCTION PROTOTYPES---------------------------------------------------------
CommPort CommPortCreate(uint16_t txBufferSize, uint16_t rxBufferSize, uint16_t lineBufferSize,
						char* txData, char* rxData, char* lineData,
						LineTermination txLineTermination, LineTermination rxLineTermination,
						const CommDataRegisters* registers);
void CommPortUpdate(CommPort* comm);
// Data Transport Functions
void CommGetLine(CommPort* comm);
void CommPutLine(CommPort* source, CommPort* destination);
void CommPutString(CommPort* comm, const char* str);
void CommPutLineTermination(CommPort* comm);
void CommPutChar(CommPort* comm, char data);
#endif