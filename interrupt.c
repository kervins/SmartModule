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
	LED = 1;
	if(PIR3bits.SSP2IF)
	{
		if(_sram.bytesRemaining == 0)
		{
			RAM_CS = 1;
			if(_sram.currentOperation == SRAM_OP_READ)
				_sram.targetBuffer->length = _sram.dataLength;
			_sram.isBusy = false;
		}

		if(_sram.isBusy)
		{
			switch(_sram.currentOperation)
			{
				case SRAM_OP_READ:
				{
					_SramRead();
					break;
				}
				case SRAM_OP_WRITE:
				{
					_SramWrite();
					break;
				}
				case SRAM_OP_FILL:
				{
					_SramFill();
					break;
				}
			}
		}
		PIR3bits.SSP2IF = false;
	}

	if(PIR1bits.TX1IF)
	{
		if(_comm1.buffers.tx.length && !_comm1.statusBits.isTxPaused)
			TXREG1 = RingBufferU8VolDequeue(&_comm1.buffers.tx);
		else
			PIE1bits.TX1IE = false;
	}

	if(PIR1bits.RC1IF)
	{
		char data = RCREG1;
		if(!_comm1.modeBits.ignoreRx)
		{
			if(data == ASCII_XOFF && _comm1.statusBits.isTxFlowControl)
				_comm1.statusBits.isTxPaused = true;
			else if(data == ASCII_XON && _comm1.statusBits.isTxFlowControl)
				_comm1.statusBits.isTxPaused = false;
			else
				RingBufferU8VolEnqueue(&_comm1.buffers.rx, data);

			if(_comm1.statusBits.isRxFlowControl
			&&!_comm1.statusBits.isRxPaused
			&& _comm1.buffers.rx.length >= XOFF_THRESHOLD)
			{
				while(!TXSTA1bits.TRMT)
					continue;
				TXREG1 = ASCII_XOFF;
				_comm1.statusBits.isRxPaused = true;
			}
		}
	}

	if(PIR3bits.TX2IF)
	{
		if(_comm2.buffers.tx.length && !_comm2.statusBits.isTxPaused)
			TXREG2 = RingBufferU8VolDequeue(&_comm2.buffers.tx);
		else
			PIE3bits.TX2IE = false;
	}

	if(PIR3bits.RC2IF)
	{
		char data = RCREG2;
		if(!_comm2.modeBits.ignoreRx)
		{
			if(data == ASCII_XOFF && _comm2.statusBits.isTxFlowControl)
				_comm2.statusBits.isTxPaused = true;
			else if(data == ASCII_XON && _comm2.statusBits.isTxFlowControl)
				_comm2.statusBits.isTxPaused = false;
			else
				RingBufferU8VolEnqueue(&_comm2.buffers.rx, data);

			if(_comm2.statusBits.isRxFlowControl
			&&!_comm2.statusBits.isRxPaused
			&& _comm2.buffers.rx.length >= XOFF_THRESHOLD)
			{
				while(!TXSTA2bits.TRMT)
					continue;
				TXREG2 = ASCII_XOFF;
				_comm2.statusBits.isRxPaused = true;
			}
		}
	}
	LED = 0;
	return;
}