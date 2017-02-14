/* Project:	SmartModule
 * File:	serial_comm.h
 * Author:	Jonathan Ruisi
 * Created:	February 14, 2017, 2:22 AM
 */

#ifndef SERIAL_COMM_H
#define SERIAL_COMM_H

// DEFINITIONS-----------------------------------------------------------------
#define XON					0x11	// Software flow control XON character
#define XOFF				0x13	// Software flow control XOFF character
#define TX_BUFFER_SIZE		32
#define RX_BUFFER_SIZE		256
#define XOFF_THRESHOLD		(3 * RX_BUFFER_SIZE) / 4
#define XON_THRESHOLD		RX_BUFFER_SIZE / 4

// TYPE DEFINITIONS------------------------------------------------------------

typedef struct _RingBuffer
{
	uint8_t bufferSize;
	uint8_t length;
	uint8_t head;
	uint8_t tail;
	char* data;
} RingBuffer;

typedef struct _CommStatus
{

	union
	{

		struct
		{
			unsigned isRxTarget : 1;
			unsigned isTxTarget : 1;
			unsigned isRxPaused : 1;
			unsigned isTxPaused : 1;
			unsigned isRxFlowControl : 1;
			unsigned : 3;
		} statusBits;
		uint8_t status;
	} ;
	RingBuffer* rxBuffer;
	RingBuffer* txBuffer;
} CommStatus;

// GLOBAL VARIABLES------------------------------------------------------------
extern volatile RingBuffer _txBuffer1, _txBuffer2, _rxBuffer1, _rxBuffer2;
extern volatile CommStatus _commStatus1, _commStatus2;

// FUNCTION PROTOTYPES---------------------------------------------------------
// Ring Buffer Functions
RingBuffer RingBufferCreate(uint16_t bufferSize, char* dataBuffer);
void RingBufferEnqueue(volatile RingBuffer* buffer, char data);
char RingBufferDequeue(volatile RingBuffer* buffer);
// STDIO Functions
char getch1(void);
char getch2(void);
void putch1(char data);
void putch2(char data);
// Flow Control Functions
void CheckFlowControlRx1(void);
void CheckFlowControlRx2(void);

#endif