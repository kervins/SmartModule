/* Project:	SmartModule
 * File:	shell.c
 * Author:	Jonathan Ruisi
 * Created:	February 20, 2017, 3:39 PM
 */

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "shell.h"
#include "main.h"
#include "sram.h"
#include "serial_comm.h"
#include "wifi.h"
#include "common_types.h"

// GLOBAL VARIABLES------------------------------------------------------------
volatile uint8_t _stdoutTarget = 0, _stdinTarget = 0;

// INITIALIZATION FUNCTIONS----------------------------------------------------

// CONSOLE FUNCTIONS-----------------------------------------------------------

// STDIO FUNCTIONS-------------------------------------------------------------
