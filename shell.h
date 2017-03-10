/* Project:	SmartModule
 * File:	shell.h
 * Author:	Jonathan Ruisi
 * Created:	February 20, 2017, 3:39 PM
 */

#ifndef SHELL_H
#define SHELL_H

#include <stdint.h>
#include "common_types.h"
#include "serial_comm.h"

// DEFINITIONS-----------------------------------------------------------------

// TYPE DEFINITIONS------------------------------------------------------------

// GLOBAL VARIABLES------------------------------------------------------------

// FUNCTION PROTOTYPES---------------------------------------------------------
// String (Line) Functions
bool LineContains(RingBuffer* line, const char* str);
bool LineContainsPeek(RingBuffer* line, const char* str);
// Console Functions
void SendLineToTerminal(CommPort* comm);
void SendLineToWifi(CommPort* comm);

#endif