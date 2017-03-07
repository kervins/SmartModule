/* Project:	SmartModule
 * File:	shell.c
 * Author:	Jonathan Ruisi
 * Created:	February 20, 2017, 3:39 PM
 */

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include "shell.h"
#include "main.h"
#include "sram.h"
#include "serial_comm.h"
#include "wifi.h"
#include "common_types.h"

// GLOBAL VARIABLES------------------------------------------------------------

// INITIALIZATION FUNCTIONS----------------------------------------------------

// CONSOLE FUNCTIONS-----------------------------------------------------------

void SendLineToTerminal(CommPort* comm)
{
	CommPutLine(comm, &_comm2);
}

void SendLineToWifi(CommPort* comm)
{
	CommPutLine(comm, &_comm1);
}