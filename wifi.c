/* Project:	SmartModule
 * File:	wifi.h
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

// GLOBAL VARIABLES------------------------------------------------------------
uint8_t _wifiStatus = WIFI_STATUS_RESET_RELEASE;
uint32_t _wifiTimestamp = 0;

// FUNCTIONS-------------------------------------------------------------------

void WifiReset(void)
{
	WIFI_RST = 0;
	RCSTA1bits.SPEN	= false;
	SPBRGH1	= GET_BYTE(CALCULATE_BRG_16H(76800), 1);
	SPBRG1	= GET_BYTE(CALCULATE_BRG_16H(76800), 0);
	RCSTA1bits.SPEN	= true;
	_wifiStatus = WIFI_STATUS_RESET_RELEASE;
	_wifiTimestamp = _tick;
}