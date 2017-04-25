/* Project:	SmartModule
 * File:	sram.c
 * Author:	Jonathan Ruisi
 * Created:	February 9, 2017, 4:39 AM
 */

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include "sram.h"
#include "main.h"
#include "utility.h"

// SRAM USER CALLABLE FUNCTIONS------------------------------------------------

/**
 * Writes to the mode register of the SRAM device
 * @param mode
 * <p>An <code>SramMode</code> structure containing the new mode settings to be written
 */
void SramSetMode(SramMode mode)
{
	if(_sram.statusBits.busy)
		return;

	_sram.statusBits.busy = true;
	_sram.statusBits.currentOperation = SRAM_OP_COMMAND;
	_sram.startTime = _tick;
	_sram.dataLength = 0;
	_sram.bytesRemaining = 0;
	_sram.initialization.command = SRAM_COMMAND_WRMR;
	_sram.initialization.mode = mode;
	DMACON1bits.TXINC = true;
	DMACON1bits.RXINC = false;
	DMACON1bits.DUPLEX0 = 1;
	TXADDRH = GET_BYTE((unsigned int) &_sram.initialization, 1);
	TXADDRL = GET_BYTE((unsigned int) &_sram.initialization, 0);
	DMABCH = 0x00;
	DMABCL = 0x01;
	RAM_CS = 0;
	DMACON1bits.DMAEN = true;
}

/**
 * Reads data contained in external SRAM into a buffer
 * @param address
 * <p>Address in SRAM memory from which data will be read
 * @param length
 * <p>Number of <b>elements</b> to read
 * <p>The size of an element is determined from the <code>elementSize</code> member of the destination buffer
 * @param destination
 * <p>A pointer to a <code>Buffer</code> in which to store the data
 */
void SramRead(unsigned short long int address, unsigned short long int length, Buffer* destination)
{
	if(_sram.statusBits.busy
	|| destination == NULL
	|| length == 0
	|| address >= SRAM_CAPACITY)
		return;

	if(length > destination->capacity)
		length = destination->capacity;
	if(address + (length * destination->elementSize) > SRAM_CAPACITY)
		length = (SRAM_CAPACITY - address) / destination->elementSize;

	_sram.statusBits.busy = true;
	_sram.statusBits.currentOperation = SRAM_OP_READ;
	_sram.startTime = _tick;
	_sram.readAddress = address;
	_sram.dataLength = length;
	_sram.bytesRemaining = length * destination->elementSize;
	_sram.targetBuffer = destination;
	_sram.initialization.command = SRAM_COMMAND_READ;
	_sram.initialization.address = address;
	_SramOperationStart();
}

/*void SramReadBytes(unsigned short long int address, unsigned short long int length, unsigned char* destination)
{
	if(_sram.statusBits.busy
	|| destination == NULL
	|| length == 0
	|| length + address > SRAM_CAPACITY)
		return;

	if(address + length >= SRAM_CAPACITY)
		length = SRAM_CAPACITY - address;
}*/

void SramWrite(unsigned short long int address, Buffer* source)
{
	if(_sram.statusBits.busy
	|| source == NULL
	|| source->length == 0
	|| address + (source->length * source->elementSize) > SRAM_CAPACITY)
		return;

	_sram.statusBits.busy = true;
	_sram.statusBits.currentOperation = SRAM_OP_WRITE;
	_sram.startTime = _tick;
	_sram.writeAddress = address;
	_sram.dataLength = source->length;
	_sram.bytesRemaining = source->length * source->elementSize;
	_sram.targetBuffer = source;
	_sram.initialization.command = SRAM_COMMAND_WRITE;
	_sram.initialization.address = address;
	_SramOperationStart();
}

/*void SramWriteBytes(unsigned short long int address, unsigned short long int length, unsigned char* source)
{
	if(_sram.statusBits.busy)
		return;
}*/

void SramFill(unsigned short long int address, unsigned short long int length, unsigned char value)
{
	if(_sram.statusBits.busy
	|| length == 0
	|| address >= SRAM_CAPACITY)
		return;

	if(address + length >= SRAM_CAPACITY)
		length = SRAM_CAPACITY - address;

	_sram.statusBits.busy = true;
	_sram.statusBits.currentOperation = SRAM_OP_FILL;
	_sram.startTime = _tick;
	_sram.writeAddress = address;
	_sram.dataLength = length;
	_sram.bytesRemaining = length;
	_sram.initialization.command = SRAM_COMMAND_WRITE;
	_sram.initialization.address = address;
	_sram.initialization.fillValue = value;
	_SramOperationStart();
}

// SRAM CALLBACK FUNCTIONS-----------------------------------------------------

void _SramOperationStart(void)
{
	DMACON1bits.TXINC = true;
	DMACON1bits.RXINC = false;
	DMACON1bits.DUPLEX0 = 1;
	TXADDRH = GET_BYTE((unsigned int) &_sram.initialization, 1);
	TXADDRL = GET_BYTE((unsigned int) &_sram.initialization, 0);
	DMABCH = 0x00;
	DMABCL = 0x03;
	RAM_CS = 0;
	DMACON1bits.DMAEN = true;
}

void _SramReadBytes(void)
{
	uint24_t bytesToRead = _sram.bytesRemaining <= SRAM_BUFFER_SIZE
			? _sram.bytesRemaining
			: SRAM_BUFFER_SIZE;
	_sram.bytesRemaining -= bytesToRead;
	_sram.readAddress += bytesToRead;
	DMACON1bits.TXINC = false;
	DMACON1bits.RXINC = true;
	DMACON1bits.DUPLEX0 = 0;
	RXADDRH = GET_BYTE((unsigned int) _sram.targetBuffer->data, 1);
	RXADDRL = GET_BYTE((unsigned int) _sram.targetBuffer->data, 0);
	DMABCH = GET_BYTE(bytesToRead - 1, 1);
	DMABCL = GET_BYTE(bytesToRead - 1, 0);
	RAM_CS = 0;
	DMACON1bits.DMAEN = true;
}

void _SramWriteBytes(void)
{
	uint24_t bytesToWrite = _sram.bytesRemaining <= SRAM_BUFFER_SIZE
			? _sram.bytesRemaining
			: SRAM_BUFFER_SIZE;
	_sram.bytesRemaining -= bytesToWrite;
	_sram.writeAddress += bytesToWrite;
	DMACON1bits.TXINC = true;
	DMACON1bits.RXINC = false;
	DMACON1bits.DUPLEX0 = 1;
	TXADDRH = GET_BYTE((unsigned int) _sram.targetBuffer->data, 1);
	TXADDRL = GET_BYTE((unsigned int) _sram.targetBuffer->data, 0);
	DMABCH = GET_BYTE(bytesToWrite - 1, 1);
	DMABCL = GET_BYTE(bytesToWrite - 1, 0);
	RAM_CS = 0;
	DMACON1bits.DMAEN = true;
}

void _SramFill(void)
{
	uint24_t bytesToFill = _sram.bytesRemaining <= DMA_MAX_TRANSFER
			? _sram.bytesRemaining
			: DMA_MAX_TRANSFER;
	_sram.bytesRemaining -= bytesToFill;
	_sram.writeAddress += bytesToFill;
	DMACON1bits.TXINC = false;
	DMACON1bits.RXINC = false;
	DMACON1bits.DUPLEX0 = 1;
	TXADDRH = GET_BYTE((unsigned int) &_sram.initialization.fillValue, 1);
	TXADDRL = GET_BYTE((unsigned int) &_sram.initialization.fillValue, 0);
	DMABCH = GET_BYTE(bytesToFill - 1, 1);
	DMABCL = GET_BYTE(bytesToFill - 1, 0);
	RAM_CS = 0;
	DMACON1bits.DMAEN = true;
}

// INITIALIZATION FUNCTIONS----------------------------------------------------

void SramStatusInitialize(void)
{
	_sram.initialization.command = 0;
	_sram.initialization.address = 0;
	_sram.initialization.fillValue = 0;
	_sram.statusBits.currentOperation = 0;
	_sram.status = 0;
	_sram.dataLength = 0;
	_sram.bytesRemaining = 0;
	_sram.readAddress = 0;
	_sram.writeAddress = 0;
	_sram.targetBuffer = NULL;
	_sram.startTime = 0;
}