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
#ifdef DEV_MODE_DEBUG
	DEBUG0 = 1;
#endif
	if(PIR3bits.TMR4IF)
	{
		_tick++;
		PIR3bits.TMR4IF = false;
	}
	else if(INTCON3bits.INT1IF)
	{
		CheckButtonState(&_button, BUTTON);
		INTCON3bits.INT1IF = false;
	}
#ifdef DEV_MODE_DEBUG
	DEBUG0 = 0;
#endif
	return;
}

void __interrupt(low_priority) isrLowPriority(void)
{
#ifdef DEV_MODE_DEBUG
	DEBUG1 = 1;
#endif
	if(PIR3bits.SSP2IF)
	{
		if(_sram.bytesRemaining == 0)
		{
			RAM_CS = 1;
			if(_sram.statusBits.currentOperation == SRAM_OP_READ)
				_sram.targetBuffer->length = _sram.dataLength;
			_sram.statusBits.busy = false;
		}

		if(_sram.statusBits.busy)
		{
			switch(_sram.statusBits.currentOperation)
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
			TXREG1 = RingBufferDequeue(&_comm1.buffers.tx);
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
				RingBufferEnqueue(&_comm1.buffers.rx, data);

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
			TXREG2 = RingBufferDequeue(&_comm2.buffers.tx);
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
				RingBufferEnqueue(&_comm2.buffers.rx, data);

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
#ifdef DEV_MODE_DEBUG
	DEBUG1 = 0;
#endif
	return;
}