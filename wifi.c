/* Project:	SmartModule
 * File:	wifi.c
 * Author:	Jonathan Ruisi
 * Created:	February 14, 2017, 7:55 AM
 */

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include "wifi.h"
#include "main.h"
#include "serial_comm.h"
#include "utility.h"

// FUNCTIONS-------------------------------------------------------------------

void WifiReset(WifiResetTypes resetType)
{
	WIFI_RST = 0;
	if(resetType == WIFI_RESET_HOLD || resetType == WIFI_RESET_RESTART)
	{
		RCSTA1bits.SPEN	= false;
		SPBRGH1	= GET_BYTE(CALCULATE_BRG_16H(76800), 1);
		SPBRG1	= GET_BYTE(CALCULATE_BRG_16H(76800), 0);
		_wifi.bootStatus = WIFI_BOOT_RESET_HOLD;
		return;
	}

	RCSTA1bits.SPEN	= true;
	_comm1.modeBits.ignoreRx = true;
	_wifi.bootStatus = WIFI_BOOT_RESET_RELEASE;
	_wifi.eventTime = _tick;
	WIFI_RST = 1;
}

void WifiHandleBoot(void)
{
	if(_wifi.bootStatus == WIFI_BOOT_RESET_RELEASE && (_tick - _wifi.eventTime > 100))
	{
		_wifi.bootStatus = WIFI_BOOT_SELFCHECK;
		_wifi.eventTime = _tick;
	}
	else if(_wifi.bootStatus == WIFI_BOOT_SELFCHECK)
	{
		RCSTA1bits.SPEN	= false;
		SPBRGH1	= GET_BYTE(CALCULATE_BRG_16H(115200), 1);
		SPBRG1	= GET_BYTE(CALCULATE_BRG_16H(115200), 0);
		RCSTA1bits.SPEN	= true;
		_comm1.modeBits.ignoreRx = false;
		_wifi.bootStatus = WIFI_BOOT_INITIALIZING;
		_wifi.eventTime = _tick;
	}
	else if(_wifi.bootStatus == WIFI_BOOT_INITIALIZING && _comm1.lineQueue.length)
	{
		// TODO: Lines are being manipulated out of order (the lineActions...)
		if(BufferContains(&_comm1.buffers.line, "ready"))
			_wifi.bootStatus = WIFI_BOOT_COMPLETE;
		_comm1.lineQueue.length = 0;
	}
}