/* Project:	SmartModule
 * File:	wifi.h
 * Author:	Jonathan Ruisi
 * Created:	February 14, 2017, 7:55 AM
 */

#ifndef WIFI_H
#define WIFI_H

#include <stdint.h>

// DEFINITIONS-----------------------------------------------------------------
#define WIFI_STATUS_RESET_HOLD		0
#define WIFI_STATUS_RESET_RELEASE	1
#define WIFI_STATUS_BOOT1			2
#define WIFI_STATUS_BOOT2			3
#define WIFI_STATUS_READY			4

// DEFINITIONS (Connection)----------------------------------------------------
#define WIFI_AP_SSID		"RuisiWifi"
#define WIFI_AP_PASS		"********"
#define WIFI_SERVER_PORT	11000

// GLOBAL VARIABLES------------------------------------------------------------
extern uint8_t _wifiStatus;
extern uint32_t _wifiTimestamp;

// FUNCTION PROTOTYPES---------------------------------------------------------
void WifiReset(void);

#endif