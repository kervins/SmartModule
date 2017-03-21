/* Project:	SmartModule
 * File:	sram.c
 * Author:	Jonathan Ruisi
 * Created:	February 9, 2017, 4:39 AM
 */

#include <xc.h>
#include <stdbool.h>
#include "sram.h"
#include "main.h"
#include "utility.h"

// SRAM USER CALLABLE FUNCTIONS------------------------------------------------

void SramSetMode(SramMode mode)
{
	if(_sram.isBusy)
		return;

	_sram.isBusy = true;
	_sram.currentOperation = SRAM_OP_COMMAND;
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

void SramRead(uint24_t address, uint24_t length, BufferU8* destination)
{
	if(_sram.isBusy)
		return;
	if(length == 0 || address >= SRAM_CAPACITY)
		return;
	if(length > destination->bufferSize)
		length = destination->bufferSize;
	if(address + length >= SRAM_CAPACITY)
		length = SRAM_CAPACITY - address;

	_sram.isBusy = true;
	_sram.currentOperation = SRAM_OP_READ;
	_sram.startTime = _tick;
	_sram.readAddress = address;
	_sram.dataLength = length;
	_sram.bytesRemaining = length;
	_sram.targetBuffer = destination;
	_sram.initialization.command = SRAM_COMMAND_READ;
	_sram.initialization.address = address;
	_SramOperationStart();
}

void SramWrite(uint24_t address, BufferU8* source)
{
	if(_sram.isBusy)
		return;
	if(source->length == 0 || address + source->length >= SRAM_CAPACITY)
		return;

	_sram.isBusy = true;
	_sram.currentOperation = SRAM_OP_WRITE;
	_sram.startTime = _tick;
	_sram.writeAddress = address;
	_sram.dataLength = source->length;
	_sram.bytesRemaining = source->length;
	_sram.targetBuffer = source;
	_sram.initialization.command = SRAM_COMMAND_WRITE;
	_sram.initialization.address = address;
	_SramOperationStart();
}

void SramFill(uint24_t address, uint24_t length, uint8_t value)
{
	if(_sram.isBusy)
		return;
	if(length == 0 || address >= SRAM_CAPACITY)
		return;
	if(address + length >= SRAM_CAPACITY)
		length = SRAM_CAPACITY - address;

	_sram.isBusy = true;
	_sram.currentOperation = SRAM_OP_FILL;
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

void _SramRead(void)
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

void _SramWrite(void)
{
	uint24_t bytesToWrite = _sram.bytesRemaining <= SRAM_BUFFER_SIZE
			? _sram.bytesRemaining
			: SRAM_BUFFER_SIZE;
	_sram.bytesRemaining -= bytesToWrite;
	_sram.writeAddress += bytesToWrite;
	//_sram.targetBuffer->length += bytesToWrite;
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
	_sram.currentOperation = 0;
	_sram.status = 0;
	_sram.dataLength = 0;
	_sram.bytesRemaining = 0;
	_sram.readAddress = 0;
	_sram.writeAddress = 0;
	_sram.targetBuffer = NULL;
	_sram.startTime = 0;
}