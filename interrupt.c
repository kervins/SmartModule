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
			_sramStatus.dataLength = 0;
			_sramStatus.statusBits.isWriting = false;
		}
		else if(_sramStatus.statusBits.isFilling && _sramStatus.dataLength == 0)
		{
			_sramStatus.statusBits.isFilling = false;
		}
		_sramStatus.statusBits.isBusy = false;
		PIR3bits.SSP2IF = false;
	}
	if(PIR1bits.RC1IF)
	{
		char data = *_comm1.registers->pRxReg;
		if(data == ASCII_XOFF && _comm1.statusBits.isTxFlowControl)
			_comm1.statusBits.isTxPaused = true;
		else if(data == ASCII_XON && _comm1.statusBits.isTxFlowControl)
			_comm1.statusBits.isTxPaused = false;
		else
			RingBufferEnqueue(&_comm1.rxBuffer, data);
	}
	if(PIR3bits.RC2IF)
	{
		char data = *_comm2.registers->pRxReg;
		if(data == ASCII_XOFF && _comm2.statusBits.isTxFlowControl)
			_comm2.statusBits.isTxPaused = true;
		else if(data == ASCII_XON && _comm2.statusBits.isTxFlowControl)
			_comm2.statusBits.isTxPaused = false;
		else
			RingBufferEnqueue(&_comm2.rxBuffer, data);
	}
	return;
}