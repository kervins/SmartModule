/* Project:	SmartModule
 * File:	sram.c
 * Author:	Jonathan Ruisi
 * Created:	February 9, 2017, 4:39 AM
 */

#include <xc.h>
#include <stdbool.h>
#include <string.h>
#include "sram.h"
#include "main.h"
#include "utility.h"

// GLOBAL VARIABLES------------------------------------------------------------
volatile SramStatus _sramStatus;
volatile SramPacket _sramPacket;

// INITIALIZATION FUNCTIONS----------------------------------------------------

SramStatus SramStatusCreate(void)
{
	SramStatus sramStatus;
	sramStatus.status = 0;
	sramStatus.dataLength = 0;
	sramStatus.readAddress = 0;
	sramStatus.writeAddress = 0;
	return sramStatus;
}

// N01S830XX Functions---------------------------------------------------------

void SramSetMode(SramMode mode)
{
	_sramStatus.statusBits.isBusy = true;
	_sramStatus.statusBits.isCommand = true;
	_sramStatus.statusBits.keepEnabled = false;
	_sramStatus.dataLength = 0;
	_sramPacket.initialization.command = SRAM_COMMAND_WRMR;
	_sramPacket.initialization.mode = mode;
	DMACON1bits.TXINC = true;
	DMACON1bits.RXINC = false;
	DMACON1bits.DUPLEX0 = 1;
	TXADDRH = GET_BYTE((unsigned int) &_sramPacket.initialization, 1);
	TXADDRL = GET_BYTE((unsigned int) &_sramPacket.initialization, 0);
	DMABCH = 0x00;
	DMABCL = 0x01;
	RAM_CS = 0;
	DMACON1bits.DMAEN = true;
}

void SramRead(uint24_t address, uint24_t length)
{
	if(address >= SRAM_CAPACITY)
		address %= SRAM_CAPACITY;
	if(length > SRAM_BUFFER_SIZE)
		length = SRAM_BUFFER_SIZE;
	if(address + length >= SRAM_CAPACITY)
		length = SRAM_CAPACITY - address;
	_sramStatus.statusBits.isReading = true;
	_sramStatus.readAddress = address;
	_sramStatus.dataLength = length;
	_sramPacket.initialization.command = SRAM_COMMAND_READ;
	_sramPacket.initialization.address = address;
	SramCommandAddress();
}

void SramReadNext(uint24_t length)
{
	if(length > SRAM_BUFFER_SIZE)
		length = SRAM_BUFFER_SIZE;

	_sramStatus.statusBits.isReading = true;
	_sramStatus.dataLength = length;
	_sramPacket.initialization.command = SRAM_COMMAND_READ;
	_sramPacket.initialization.address = _sramStatus.readAddress;
	SramCommandAddress();
}

void SramReadContinue(void)
{
	_sramStatus.statusBits.isBusy = true;
	_sramStatus.statusBits.keepEnabled = false;
	DMACON1bits.TXINC = false;
	DMACON1bits.RXINC = true;
	DMACON1bits.DUPLEX0 = 0;
	RXADDRH = GET_BYTE((unsigned int) _sramPacket.data, 1);
	RXADDRL = GET_BYTE((unsigned int) _sramPacket.data, 0);
	DMABCH = GET_BYTE(_sramStatus.dataLength - 1, 1);
	DMABCL = GET_BYTE(_sramStatus.dataLength - 1, 0);
	RAM_CS = 0;
	DMACON1bits.DMAEN = true;
}

void SramWrite(uint24_t address, const void* data, uint24_t length)
{
	if(address >= SRAM_CAPACITY)
		address %= SRAM_CAPACITY;
	if(length > SRAM_BUFFER_SIZE)
		length = SRAM_BUFFER_SIZE;
	if(address + length >= SRAM_CAPACITY)
		length = SRAM_CAPACITY - address;
	_sramStatus.statusBits.isWriting = true;
	_sramStatus.writeAddress = address;
	_sramStatus.dataLength = length;
	_sramPacket.initialization.command = SRAM_COMMAND_WRITE;
	_sramPacket.initialization.address = address;
	memcpy(_sramPacket.data, data, length);
	SramCommandAddress();
}

void SramWriteNext(const void* data, uint24_t length)
{
	if(length > SRAM_BUFFER_SIZE)
		length = SRAM_BUFFER_SIZE;

	_sramStatus.statusBits.isWriting = true;
	_sramStatus.dataLength = length;
	_sramPacket.initialization.command = SRAM_COMMAND_WRITE;
	_sramPacket.initialization.address = _sramStatus.writeAddress;
	memcpy(_sramPacket.data, data, length);
	SramCommandAddress();
}

void SramWriteContinue(void)
{
	_sramStatus.statusBits.isBusy = true;
	_sramStatus.statusBits.keepEnabled = false;
	DMACON1bits.TXINC = true;
	DMACON1bits.RXINC = false;
	DMACON1bits.DUPLEX0 = 1;
	TXADDRH = GET_BYTE((unsigned int) _sramPacket.data, 1);
	TXADDRL = GET_BYTE((unsigned int) _sramPacket.data, 0);
	DMABCH = GET_BYTE(_sramStatus.dataLength - 1, 1);
	DMABCL = GET_BYTE(_sramStatus.dataLength - 1, 0);
	RAM_CS = 0;
	DMACON1bits.DMAEN = true;
}

void SramFill(uint24_t address, uint24_t length, uint8_t value)
{
	if(address >= SRAM_CAPACITY)
		address %= SRAM_CAPACITY;
	if(address + length >= SRAM_CAPACITY)
		length = SRAM_CAPACITY - address;
	_sramStatus.statusBits.isFilling = true;
	_sramStatus.dataLength = length;
	_sramPacket.initialization.command = SRAM_COMMAND_WRITE;
	_sramPacket.initialization.address = address;
	_sramPacket.data[0] = value;
	SramCommandAddress();
}

void SramFillContinue(void)
{
	_sramStatus.statusBits.isBusy = true;
	DMACON1bits.TXINC = false;
	DMACON1bits.RXINC = false;
	DMACON1bits.DUPLEX0 = 1;
	TXADDRH = GET_BYTE((unsigned int) _sramPacket.data, 1);
	TXADDRL = GET_BYTE((unsigned int) _sramPacket.data, 0);
	if(_sramStatus.dataLength > DMA_MAX_TRANSFER)
	{
		_sramStatus.statusBits.keepEnabled = true;
		DMABCH = GET_BYTE(DMA_MAX_TRANSFER - 1, 1);
		DMABCL = GET_BYTE(DMA_MAX_TRANSFER - 1, 0);
		_sramStatus.dataLength -= DMA_MAX_TRANSFER;
	}
	else
	{
		_sramStatus.statusBits.keepEnabled = false;
		DMABCH = GET_BYTE(_sramStatus.dataLength - 1, 1);
		DMABCL = GET_BYTE(_sramStatus.dataLength - 1, 0);
		_sramStatus.dataLength = 0;
	}
	DMACON1bits.DMAEN = true;
}

void SramCommandAddress(void)
{
	_sramStatus.statusBits.isBusy = true;
	_sramStatus.statusBits.isCommand = true;
	_sramStatus.statusBits.keepEnabled = true;
	DMACON1bits.TXINC = true;
	DMACON1bits.RXINC = false;
	DMACON1bits.DUPLEX0 = 1;
	TXADDRH = GET_BYTE((unsigned int) &_sramPacket.initialization, 1);
	TXADDRL = GET_BYTE((unsigned int) &_sramPacket.initialization, 0);
	DMABCH = 0x00;
	DMABCL = 0x03;
	RAM_CS = 0;
	DMACON1bits.DMAEN = true;
}