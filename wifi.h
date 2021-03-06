/**@file		wifi.h
 * @brief		Header file for implementation of the ESP8266 wifi module
 * @author		Jonathan Ruisi
 * @version		1.0
 * @date		February 14, 2017
 * @warning		This code has only been tested with <b>Version 1</b> of the ESP8266
 * @copyright	GNU Public License
 */

#ifndef WIFI_H
#define WIFI_H

// DEFINITIONS-----------------------------------------------------------------
// Boot states
#define WIFI_BOOT_POWER_ON_RESET_HOLD	0	/**< Flag indicating the wifi is being held in reset until powerup has completed */
#define WIFI_BOOT_RESET_HOLD			1	/**< Flag indicating the wifi is being held in reset indefinitely */
#define WIFI_BOOT_RESET_RELEASE			2	/**< Flag indicating the wifi has been released from reset and has started its boot procedure */
#define WIFI_BOOT_SELFCHECK				3	/**< Flag indicating the wifi is currently booting and is running self-check */
#define WIFI_BOOT_INITIALIZING			4	/**< Flag indicating the wifi is currently booting and is initializing */
#define WIFI_BOOT_COMPLETE				5	/**< Flag indicating the wifi boot process is complete */
// Reset modes
#define WIFI_RESET_HOLD		0				/**< Reset of indefinite length */
#define WIFI_RESET_RELEASE	1				/**< Release the wifi from reset */
#define WIFI_RESET_RESTART	2				/**< Reset while system restarts */
// TCP status
#define WIFI_TCP_CLOSED		0				/**< TCP connection status: CLOSED */
#define WIFI_TCP_CONNECTING	1				/**< TCP connection status: CONNECTING */
#define WIFI_TCP_READY		2				/**< TCP connection status: CONNECTED */

// TYPE DEFINITIONS------------------------------------------------------------

/**@struct WifiInfo
 * Structure containing status information and event timing for the ESP8266
 */
typedef struct
{

	union
	{

		struct
		{
			unsigned boot : 3;					/**< Current boot status */
			unsigned resetMode : 2;				/**< Current reset mode */
			unsigned isSsidConnected : 1;		/**< Flag indicating whether or not the wifi is connected to a network */
			unsigned tcpConnectionStatus : 2;	/**< Flag indicating whether or not the wifi has established itself as a TCP client */
		} statusBits;
		unsigned char status;
	} ;
	unsigned long int eventTime;				/**< Timestamp of the last event */
} WifiInfo;

// CONSTANTS (NETWORK INFO)----------------------------------------------------
static const char* network_ssid			= "JASPERNET";		/**< The SSID to connect to on startup */
static const bool network_use_password	= true;				/**< Flag indicating if the SSID requires a password */
static const char* network_pass			= "";				/**< The password for the SSID */
static const char* tcp_server			= "JRUISI-LAPTOP";	/**< The TCP host */
static const unsigned int tcp_port		= 11000;			/**< The TCP port */

// CONSTANTS (ESP-8266 AT COMMANDS)--------------------------------------------
// BASIC COMMANDS
static const char* at_test				= "AT";					/**< Test AT startup */
static const char* at_echo				= "ATE";				/**< AT command echo */
static const char* at_rst				= "AT+RST";				/**< Restart module */
static const char* at_gmr				= "AT+GMR";				/**< View version info */
static const char* at_gslp				= "AT+GSLP";			/**< Enter deep-sleep mode */
static const char* at_restore			= "AT+RESTORE";			/**< Factory reset */
static const char* at_uart_cur			= "AT+UART_CUR";		/**< UART current configuration */
static const char* at_uart_def			= "AT+UART_DEF";		/**< UART default configuration, save to flash */
static const char* at_sleep				= "AT+SLEEP";			/**< Sleep mode */
static const char* at_wakeupgpio		= "AT+WAKEUPGPIO";		/**< Set a GPIO to wake ESP8266 up from light-sleep mode */
static const char* at_rfpower			= "AT+RFPOWER";			/**< Set maximum value of RF TX power */
static const char* at_rfvdd				= "AT+RFVDD";			/**< Set RF TX power according to VDD33 */
// WIFI COMMANDS
static const char* at_cwmode_cur		= "AT+CWMODE_CUR";		/**< Wifi mode */
static const char* at_cwmode_def		= "AT+CWMODE_DEF";		/**< Wifi mode (saved to flash) */
static const char* at_cwjap_cur			= "AT+CWJAP_CUR";		/**< Connect to AP */
static const char* at_cwjap_def			= "AT+CWJAP_DEP";		/**< Connect to AP */
static const char* at_cwlapopt			= "AT+CWLAPOPT";		/**< Set the configuration of command AT+CWLAP */
static const char* at_cwlap				= "AT+CWLAP";			/**< List available APs */
static const char* at_cwqap				= "AT+CWQAP";			/**< Disconnect from AP */
static const char* at_cwsap_cur			= "AT+CWSAP_CUR";		/**< Configure ESP8266 soft-AP */
static const char* at_cwsap_def			= "AT+CWSAP_DEF";		/**< Configure ESP8266 soft-AP */
static const char* at_cwlif				= "AT+CWLIF";			/**< Get station IP which is connected to ESP8266 softAP */
static const char* at_cwdhcp_cur		= "AT+CWDHCP_CUR";		/**< Enable/disable DHCP */
static const char* at_cwdhcp_def		= "AT+CWDHCP_DEF";		/**< Enable/disable DHCP */
static const char* at_cwdhcps_cur		= "AT+CWDHCPS_CUR";		/**< Set IP range of DHCP server */
static const char* at_cwdhcps_def		= "AT+CWDHCPS_DEF";		/**< Set IP range of DHCP server */
static const char* at_cwautoconn		= "AT+CWAUTOCONN";		/**< Connect to AP automatically on power-up */
static const char* at_cipstamac_cur		= "AT+CIPSTAMAC_CUR";	/**< Set MAC address of ESP8266 station */
static const char* at_cipstamac_def		= "AT+CIPSTAMAC_DEF";	/**< Set MAC address of ESP8266 station */
static const char* at_cipapmac_cur		= "AT+CIPAPMAC_CUR";	/**< Set MAC address of ESP8266 soft-AP */
static const char* at_cipapmac_def		= "AT+CIPAPMAC_DEF";	/**< Set MAC address of ESP8266 soft-AP */
static const char* at_cipsta_cur		= "AT+CIPSTA_CUR";		/**< Set IP address of ESP8266 station */
static const char* at_cipsta_def		= "AT+CIPSTA_DEF";		/**< Set IP address of ESP8266 station */
static const char* at_cipap_cur			= "AT+CIPAP_CUR";		/**< Set IP address of ESP8266 soft-AP */
static const char* at_cipap_def			= "AT+CIPAP_DEF";		/**< Set IP address of ESP8266 soft-AP */
static const char* at_cwstartsmart		= "AT+CWSTARTSMART";	/**< Start SmartConfig */
static const char* at_cwstopsmart		= "AT+CWSTOPSMART";		/**< Stop SmartConfig */
static const char* at_cwstartdiscover	= "AT+CWSTARTDISCOVER";	/**< Start the mode that the ESP8266 can be found by WeChat */
static const char* at_cwstopdiscover	= "AT+CWSTOPDISCOVER";	/**< Stop the mode that the ESP8266 can be found by WeChat */
static const char* at_wps				= "AT+WPS";				/**< Set WPS function */
static const char* at_mdns				= "AT+MDNS";			/**< Set MDNS function */
// TCP/IP COMMANDS
static const char* at_cipstatus			= "AT+CIPSTATUS";		/**< Get connection status */
static const char* at_cipdomain			= "AT+CIPDOMAIN";		/**< DNS function */
static const char* at_cipstart			= "AT+CIPSTART";		/**< Establish TCP connection, UDP transmission, or SSL connection */
static const char* at_cipsslsize		= "AT+CIPSSLSIZE";		/**< Set the size of SSL buffer */
static const char* at_cipsend			= "AT+CIPSEND";			/**< Send data */
static const char* at_cipsendex			= "AT+CIPSENDEX";		/**< Send data (if <length> or "\0" is met, data will be sent) */
static const char* at_cipsendbuf		= "AT+CIPSENDBUF";		/**< Write data into TCP send buffer */
static const char* at_cipbufreset		= "AT+CIPBUFRESET";		/**< Reset segment ID count */
static const char* at_cipbufstatus		= "AT+CIPBUFSTATUS";	/**< Check status of TCP send buffer */
static const char* at_cipcheckseq		= "AT+CIPCHECKSEQ";		/**< Check if a specific segment is sent or not */
static const char* at_cipclose			= "AT+CIPCLOSE";		/**< Close TCP/UDP/SSL connection */
static const char* at_cifsr				= "AT+CIFSR";			/**< Get local IP address */
static const char* at_cipmux			= "AT+CIPMUX";			/**< Set multiple connections mode */
static const char* at_cipserver			= "AT+CIPSERVER";		/**< Configure as server */
static const char* at_cipmode			= "AT+CIPMODE";			/**< Set transmission mode */
static const char* at_savetranslink		= "AT+SAVETRANSLINK";	/**< Save transparent transmission link to flash */
static const char* at_cipsto			= "AT+CIPSTO";			/**< Set timeout when ESP8266 runs as TCP server */
static const char* at_ciupdate			= "AT+CIUPDATE";		/**< Upgrade firmware through network (DON'T USE THIS!) */
static const char* at_ping				= "AT+PING";			/**< Ping function */
static const char* at_cipdinfo			= "AT+CIPDINFO";		/**< Show remote IP and remote port */

// GLOBAL VARIABLES------------------------------------------------------------
extern WifiInfo _wifi;

// FUNCTION PROTOTYPES---------------------------------------------------------
// ESP8266 Control
void WifiReset(void);
void WifiHandleBoot(void);
void UpdateWifi(void);

#endif