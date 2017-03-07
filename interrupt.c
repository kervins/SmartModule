/* Project:	SmartModule
 * File:	interrupt.c
 * Author:	Jonathan Ruisi
 * Created:	December 19, 2016, 5:26 AM
 */

#include <xc.h>
#include <stdbool.h>
#include <stdint.h>
#include "interrupt.h"
#include "main.h"
#include "button.h"
#include "serial_comm.h"
#include "sram.h"
#include "utility.h"

void __interrupt(high_priority) isrHighPriority(void)
{
	if(PIR3bits.TMR4IF)
	{
		_tick++;
		PIR3bits.TMR4IF = false;
	}
	else if(INTCON3bits.INT1IF)
	{
		UpdateButtonState(&_button, BUTTON);
		INTCON3bits.INT1IF = false;
	}
	return;
}

void __interrupt(low_priority) isrLowPriority(void)
{
	if(PIR3bits.SSP2IF)
	{
		RAM_CS = ~_sramStatus.statusBits.keepEnabled;

		if(_sramStatus.statusBits.isCommand)
		{
			_sramStatus.statusBits.isCommand = false;
		}
		else if(_sramStatus.statusBits.isReading)
		{
			_sramStatus.readAddress += _sramStatus.dataLength;
			if(_sramStatus.readAddress >= SRAM_CAPACITY)
				_sramStatus.readAddress = 0;
			_sramStatus.statusBits.hasUnreadData = true;
			_sramStatus.statusBits.isReading = false;
		}
		else if(_sramStatus.statusBits.isWriting)
		{
			_sramStatus.writeAddress += _sramStatus.dataLength;
			if(_sramStatus.writeAddress >= SRAM_CAPACITY)
				_sramStatus.writeAddress = 0;
		}
		else if(_sramStatus.statusBits.isFilling && _sramStatus.dataLength == 0)
		{
			_sramStatus.statusBits.isFilling = false;
		}
		_sramStatus.statusBits.isBusy = false;
		PIR3bits.SSP2IF = false;
	}

	if(PIR1bits.TX1IF)
	{
		if(_comm1.txBuffer.length && !_comm1.statusBits.isTxPaused)
			TXREG1 = RingBufferDequeue(&_comm1.txBuffer);
		else
			PIE1bits.TX1IE = false;
	}

	if(PIR1bits.RC1IF)
	{
		char data = RCREG1;
		if(data == ASCII_XOFF && _comm1.statusBits.isTxFlowControl)
			_comm1.statusBits.isTxPaused = true;
		else if(data == ASCII_XON && _comm1.statusBits.isTxFlowControl)
			_comm1.statusBits.isTxPaused = false;
		else
			RingBufferEnqueue(&_comm1.rxBuffer, data);

		if(_comm1.statusBits.isRxFlowControl
		&&!_comm1.statusBits.isRxPaused
		&& _comm1.rxBuffer.length >= XOFF_THRESHOLD)
		{
			while(!TXSTA1bits.TRMT)
				continue;
			TXREG1 = ASCII_XOFF;
			_comm1.statusBits.isRxPaused = true;
		}
	}

	if(PIR3bits.TX2IF)
	{
		if(_comm2.txBuffer.length && !_comm2.statusBits.isTxPaused)
			TXREG2 = RingBufferDequeue(&_comm2.txBuffer);
		else
			PIE3bits.TX2IE = false;
	}

	if(PIR3bits.RC2IF)
	{
		char data = RCREG2;
		if(data == ASCII_XOFF && _comm2.statusBits.isTxFlowControl)
			_comm2.statusBits.isTxPaused = true;
		else if(data == ASCII_XON && _comm2.statusBits.isTxFlowControl)
			_comm2.statusBits.isTxPaused = false;
		else
			RingBufferEnqueue(&_comm2.rxBuffer, data);

		if(_comm2.statusBits.isRxFlowControl
		&&!_comm2.statusBits.isRxPaused
		&& _comm2.rxBuffer.length >= XOFF_THRESHOLD)
		{
			while(!TXSTA2bits.TRMT)
				continue;
			TXREG2 = ASCII_XOFF;
			_comm2.statusBits.isRxPaused = true;
		}
	}
	return;
}