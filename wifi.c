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

void UpdateWifi(void)
{
	if((_wifi.statusBits.boot == WIFI_BOOT_POWER_ON_RESET_HOLD) && (_tick - _wifi.eventTime > 2000))
	{
		_wifi.statusBits.resetMode = WIFI_RESET_RELEASE;
		WifiReset();
	}
	else if(_wifi.statusBits.boot >= WIFI_BOOT_RESET_RELEASE && _wifi.statusBits.boot < WIFI_BOOT_COMPLETE)
		WifiHandleBoot();
}

void WifiReset(void)
{
	WIFI_RST = 0;
	_wifi.statusBits.isSsidConnected = false;
	_wifi.statusBits.tcpConnectionStatus = WIFI_TCP_CLOSED;
	if(_wifi.statusBits.resetMode == WIFI_RESET_HOLD || _wifi.statusBits.resetMode == WIFI_RESET_RESTART)
	{
		RCSTA1bits.SPEN	= false;
		SPBRGH1	= GET_BYTE(CALCULATE_BRG_16H(76800), 1);
		SPBRG1	= GET_BYTE(CALCULATE_BRG_16H(76800), 0);
		_wifi.statusBits.boot = WIFI_BOOT_RESET_HOLD;
		return;
	}

	RCSTA1bits.SPEN	= true;
	_comm1.modeBits.ignoreRx = true;
	_comm1.modeBits.useExternalBuffer = false;
	_wifi.statusBits.boot = WIFI_BOOT_RESET_RELEASE;
	_wifi.eventTime = _tick;
	WIFI_RST = 1;
}

void WifiHandleBoot(void)
{
	if(_wifi.statusBits.boot == WIFI_BOOT_RESET_RELEASE && (_tick - _wifi.eventTime > 100))
	{
		_wifi.statusBits.boot = WIFI_BOOT_SELFCHECK;
		_wifi.eventTime = _tick;
	}
	else if(_wifi.statusBits.boot == WIFI_BOOT_SELFCHECK)
	{
		RCSTA1bits.SPEN	= false;
		SPBRGH1	= GET_BYTE(CALCULATE_BRG_16H(115200), 1);
		SPBRG1	= GET_BYTE(CALCULATE_BRG_16H(115200), 0);
		RCSTA1bits.SPEN	= true;
		_comm1.modeBits.ignoreRx = false;
		_wifi.statusBits.boot = WIFI_BOOT_INITIALIZING;
		_wifi.eventTime = _tick;
	}
	else if(_wifi.statusBits.boot == WIFI_BOOT_INITIALIZING && _comm1.statusBits.hasLine)
	{
		if(BufferContains(&_comm1.buffers.line, "ready", 5) >= 0)
		{
			_wifi.statusBits.boot = WIFI_BOOT_COMPLETE;
			CommPutString(&_comm1, "ATE0");
			CommPutNewline(&_comm1);
			_comm1.modeBits.useExternalBuffer = true;
		}
		_comm1.buffers.line.length = 0;
		_comm1.statusBits.hasLine = false;
	}
}