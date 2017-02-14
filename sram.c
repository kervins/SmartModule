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
volatile SramFullDuplexBuffer _sramBuffer;
volatile SramStatus _sramStatus;

// N01S830XX Functions---------------------------------------------------------

void SramSetMode(SramMode mode)
{
	_sramStatus.statusBits.isBusyWrite = true;
	_sramBuffer.transmit.initialization.command = SRAM_COMMAND_WRMR;
	_sramBuffer.transmit.initialization.mode = mode;
	TXADDRH = GET_BYTE((unsigned int) &_sramBuffer.transmit, 1);
	TXADDRL = GET_BYTE((unsigned int) &_sramBuffer.transmit, 0);
	RXADDRH = GET_BYTE((unsigned int) &_sramBuffer.receive, 1);
	RXADDRL = GET_BYTE((unsigned int) &_sramBuffer.receive, 0);
	DMABCH = 0x00;
	DMABCL = 0x01;
	RAM_CS = 0;
	DMACON1bits.DMAEN = true;
}

void SramRead(uint24_t address, uint24_t length)
{
	if(address >= SRAM_CAPACITY)
		address %= SRAM_CAPACITY;
	if(address + length >= SRAM_CAPACITY)
		length = SRAM_CAPACITY - address;
	_sramStatus.readAddress = address;
	SramReadNext(length);
}

void SramReadNext(uint24_t length)
{
	_sramStatus.statusBits.isBusyRead = true;
	_sramStatus.dataLength = length;
	_sramBuffer.transmit.initialization.command = SRAM_COMMAND_READ;
	_sramBuffer.transmit.initialization.address = _sramStatus.readAddress;
	TXADDRH = GET_BYTE((unsigned int) &_sramBuffer.transmit, 1);
	TXADDRL = GET_BYTE((unsigned int) &_sramBuffer.transmit, 0);
	RXADDRH = GET_BYTE((unsigned int) &_sramBuffer.receive, 1);
	RXADDRL = GET_BYTE((unsigned int) &_sramBuffer.receive, 0);
	DMABCH = GET_BYTE(_sramStatus.dataLength + 4, 1);
	DMABCL = GET_BYTE(_sramStatus.dataLength + 4, 0);
	RAM_CS = 0;
	DMACON1bits.DMAEN = true;
}

void SramWrite(uint24_t address, const char* data, uint8_t length)
{
	if(address >= SRAM_CAPACITY)
		address %= SRAM_CAPACITY;
	if(address + length >= SRAM_CAPACITY)
		length = SRAM_CAPACITY - address;
	_sramStatus.writeAddress = address;
	SramWriteNext(data, length);
}

void SramWriteNext(const char* data, uint8_t length)
{
	_sramStatus.statusBits.isBusyWrite = true;
	_sramStatus.dataLength = length;
	_sramBuffer.transmit.initialization.command = SRAM_COMMAND_WRITE;
	_sramBuffer.transmit.initialization.address = _sramStatus.writeAddress;
	strcpy((char*) _sramBuffer.transmit.txData, data);
	TXADDRH = GET_BYTE((unsigned int) &_sramBuffer.transmit, 1);
	TXADDRL = GET_BYTE((unsigned int) &_sramBuffer.transmit, 0);
	RXADDRH = GET_BYTE((unsigned int) &_sramBuffer.receive, 1);
	RXADDRL = GET_BYTE((unsigned int) &_sramBuffer.receive, 0);
	DMABCH = GET_BYTE(_sramStatus.dataLength + 4, 1);
	DMABCL = GET_BYTE(_sramStatus.dataLength + 4, 0);
	RAM_CS = 0;
	DMACON1bits.DMAEN = true;
}

void SramFill(uint24_t address, uint24_t length, uint8_t value)
{
	if(address >= SRAM_CAPACITY)
		address %= SRAM_CAPACITY;
	if(address + length >= SRAM_CAPACITY)
		length = SRAM_CAPACITY - address;
	_sramStatus.statusBits.isBusyFill = true;
	_sramStatus.statusBits.isContinuousFill = true;
	_sramBuffer.transmit.initialization.command = SRAM_COMMAND_WRITE;
	_sramBuffer.transmit.initialization.address = address;
	_sramBuffer.transmit.txData[0] = value;
	_sramStatus.dataLength = length;
	TXADDRH = GET_BYTE((unsigned int) &_sramBuffer.transmit, 1);
	TXADDRL = GET_BYTE((unsigned int) &_sramBuffer.transmit, 0);
	RXADDRH = GET_BYTE((unsigned int) &_sramBuffer.receive, 1);
	RXADDRL = GET_BYTE((unsigned int) &_sramBuffer.receive, 0);
	DMABCH = 0x00;
	DMABCL = 0x03;
	RAM_CS = 0;
	DMACON1bits.DMAEN = true;
}

void SramFillNext(void)
{
	_sramStatus.statusBits.isBusyFill = true;
	TXADDRH = GET_BYTE((unsigned int) _sramBuffer.transmit.txData, 1);
	TXADDRL = GET_BYTE((unsigned int) _sramBuffer.transmit.txData, 0);
	DMACON1bits.DUPLEX1 = 0;
	DMACON1bits.DUPLEX0 = 1;
	DMACON1bits.TXINC = 0;
	_sramStatus.dataLength -= _sramStatus.dataLength >= DMA_MAX_TRANSFER ? DMA_MAX_TRANSFER : _sramStatus.dataLength;
	DMABCH = GET_BYTE(_sramStatus.dataLength - 1, 1);
	DMABCL = GET_BYTE(_sramStatus.dataLength - 1, 0);
	DMACON1bits.DMAEN = true;
}