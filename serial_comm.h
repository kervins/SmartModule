/**@file		serial_comm.h
 * @brief		Header file which creates an abstraction layer for any USART module on an enhanced mid-range PIC microcontroller
 * @author		Jonathan Ruisi
 * @version		1.0
 * @date		February 14, 2017
 * @copyright	GNU Public License
 */

#ifndef SERIAL_COMM_H
#define SERIAL_COMM_H

#include "utility.h"
#include "buffer.h"
#include "linked_list.h"

// MACROS (Calculates SPBRG values for USART baud rate generator)--------------
/**@def CALCULATE_BRG(rate)
 * Calculates the BRG value to be loaded into (SPBRGHx:SPBRGx) based on the desired baud rate (\a rate).
 * BRG16 = 0, BRGH = 0
 */
#define CALCULATE_BRG(rate)		((FOSC/rate)/64)-1

/**@def CALCULATE_BRG(rate)
 * Calculates the BRG value to be loaded into (SPBRGHx:SPBRGx) based on the desired baud rate (\a rate).
 * BRG16 = 0, BRGH = 1
 */
#define CALCULATE_BRG_H(rate)	((FOSC/rate)/16)-1	// BRG16 = 0, BRGH = 1

/**@def CALCULATE_BRG(rate)
 * Calculates the BRG value to be loaded into (SPBRGHx:SPBRGx) based on the desired baud rate (\a rate).
 * BRG16 = 1, BRGH = 0
 */
#define CALCULATE_BRG_16(rate)	((FOSC/rate)/16)-1	// BRG16 = 1, BRGH = 0

/**@def CALCULATE_BRG(rate)
 * Calculates the BRG value to be loaded into (SPBRGHx:SPBRGx) based on the desired baud rate (\a rate).
 * BRG16 = 1, BRGH = 1
 */
#define CALCULATE_BRG_16H(rate)	((FOSC/rate)/4)-1	// BRG16 = 1, BRGH = 1

// DEFINITIONS-----------------------------------------------------------------
#define XOFF_THRESHOLD	(3 * RX_BUFFER_SIZE) / 4	/**< Software flow control: Determines how full the RX buffer must be before an XOFF character is transmitted, pausing transmission */
#define XON_THRESHOLD	RX_BUFFER_SIZE / 4			/**< Software flow control: Determines how full the RX buffer must be before an XON character is transmitted, resuming transmission */
#define SEQ_MAX_PARAMS	8							/**< Sets the maximum parameters that can be present in an ANSI control sequence */

// ENUMERATED TYPES------------------------------------------------------------

/**@enum NewlineFlags
 * Defines the contents of a newline
 */
typedef enum
{
	NEWLINE_CR		= 1,	/**< Carriage return only */
	NEWLINE_LF		= 2,	/**< Line feed only */
	NEWLINE_CRLF	= 3		/**< Carriage return and line feed */
} NewlineFlags;

// TYPE DEFINITIONS------------------------------------------------------------

/**@struct CommDataRegisters
 * Structure which provides hardware-specific mappings to USART registers.
 * This allows the abstraction layer to work with any enhanced mid-range PIC microcontroller.
 */
typedef struct CommDataRegisters
{
	unsigned char volatile* const pTxReg;
	TXSTAbits_t volatile* const pTxSta;
	unsigned char volatile* const pPie;
	unsigned char const txieBit;
} CommDataRegisters;

/**@struct CommPort
 * Structure which contains all necessary elements for control and communication with a USART module
 * @see NewlineFlags
 * @see CommDataRegisters
 * @see RingBuffer
 * @see Buffer
 * @see Point
 */
typedef struct CommPort
{

	union
	{

		struct
		{
			unsigned isTxFlowControl : 1;	/**< Enables/disables TX software flow control */
			unsigned isRxFlowControl : 1;	/**< Enables/disables RX software flow control */
			unsigned isTxPaused : 1;		/**< Indicates that TX has been paused by software flow control */
			unsigned isRxPaused : 1;		/**< Indicates that RX has been paused by software flow control */
			unsigned hasLine : 1;			/**< Indicates that a new line has been received */
			unsigned hasSequence : 1;		/**< Indicates that an ANSI control sequence has been received */
			unsigned : 2;
		} statusBits;
		unsigned char status;
	} ;

	union
	{

		struct
		{
			unsigned echoRx : 1;			/**< Determines whether or not received characters are echoed */
			unsigned echoNewline : 1;		/**< Determines whether or not received newlines are echoed */
			unsigned echoSequence : 1;		/**< Determines whether or not received ANSI control sequences are echoed */
			unsigned useExternalBuffer : 1;	/**< Determines whether or not to use an external SRAM buffer to store received lines */
			unsigned ignoreRx : 1;			/**< Temporarily disables RX */
			unsigned isBinaryMode : 1;		/**< If set, all received data is treated as binary (all newlines and control sequences are ignored) */
			unsigned : 2;
		} modeBits;
		unsigned char mode;
	} ;

	struct
	{
		NewlineFlags rx;					/**< Defines the newline character(s) for RX */
		NewlineFlags tx;					/**< Defines the newline character(s) for TX */
		NewlineFlags inProgress;			/**< Internal use, DO NOT MODIFY */
	} newline;

	struct
	{

		union
		{

			struct
			{
				unsigned csiCharCount : 2;		/**< Internal use, DO NOT MODIFY */
				unsigned sequenceId : 6;		/**< Internal use, DO NOT MODIFY */
			} statusBits;
			unsigned char status;				/**< Internal use, DO NOT MODIFY */
		} ;
		unsigned char paramCount;				/**< The number of parameters in the parameter list for an ANSI control sequence */
		unsigned char params[SEQ_MAX_PARAMS];	/**< A list of parameters for an ANSI control sequence */
		unsigned char terminator;				/**< Defines the terminating character of an ANSI control sequence */
	} sequence;

	struct
	{
		volatile RingBuffer tx;					/**< TX FIFO buffer */
		volatile RingBuffer rx;					/**< RX FIFO buffer */
		Buffer line;							/**< Line buffer */
		RingBuffer external;					/**< FIFO buffer mapped to external SRAM */
	} buffers;

	Point cursor;								/**< Current location of the terminal cursor */
	const CommDataRegisters* registers;			/**< Pointer to a <b>CommDataRegisters</b> structure */
} CommPort;

// FUNCTION PROTOTYPES---------------------------------------------------------
void CommPortInitialize(CommPort* comm,
						unsigned int txBufferSize, unsigned int rxBufferSize, unsigned int lineBufferSize,
						char* txData, char* rxData, char* lineData,
						const unsigned short long int* lineQueueBaseAddress, unsigned int lineQueueSize,
						NewlineFlags txNewline, NewlineFlags rxNewline,
						const CommDataRegisters* registers,
						bool enableFlowControl, bool enableEcho,
						unsigned char echoRow, unsigned char echoColumn);
void UpdateCommPort(CommPort* comm);
void CommFlushLineBuffer(CommPort* comm);
void CommResetSequence(CommPort* comm);
void CommPutChar(CommPort* comm, char data);
void CommPutString(CommPort* comm, const char* str);
void CommPutSubString(CommPort* comm, const char* str, unsigned int startIndex, unsigned int length);
void CommPutNewline(CommPort* comm);
void CommPutSequence(CommPort* comm, unsigned char terminator, unsigned char paramCount, ...);

#endif