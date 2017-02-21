/* Project:	SmartModule
 * File:	shell.c
 * Author:	Jonathan Ruisi
 * Created:	February 20, 2017, 3:39 PM
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "shell.h"
#include "main.h"
#include "serial_comm.h"
#include "common_types.h"

// GLOBAL VARIABLES------------------------------------------------------------
Shell _shell;

// STDIO FUNCTIONS-------------------------------------------------------------

char getch(void)
{
	switch(_shell.rxTarget)
	{
		case 1:
			return getch1();
		case 2:
			return getch2();
		default:
			return -1;
	}
}

void putch(char data)
{
	switch(_shell.txTarget)
	{
		case 1:
			putch1(data);
		case 2:
			putch2(data);
	}
}

// CONSOLE FUNCTIONS-----------------------------------------------------------

void ShellUpdateInput(void)
{
	volatile unsigned int* length;
	switch(_shell.rxTarget)
	{
		case 1:
			length = &_rxBuffer1.length;
		case 2:
			length = &_rxBuffer2.length;
	}

	while(*length > 0)
	{
		char ch = getch();
		if(ch == ASCII_CR)
		{
			_shell.lineData[_shell.lineLength] = 0;
			_shell.hasLine = true;
			_shell.getLine = false;
			break;
		}
		_shell.lineData[_shell.lineLength] = ch;
		_shell.lineLength++;
		if(_shell.isEcho)
			putch(ch);
	}
}

void ShellExecuteCommand(void)
{
	Action action;
	if(strnicmp(_shell.lineData, "version", _shell.lineLength) == 0)
	{
		action = ShellPrintVersion;
	}
	else
	{
		action = 0;
	}

	_shell.lineLength = 0;
	_shell.hasLine = false;
	_shell.getLine = true;
	putch(ASCII_CR);
	putch(ASCII_LF);
	if(action)
		action();
	putch(ASCII_CR);
	putch(ASCII_LF);
	putch('>');
	putch(' ');
}

void ShellPrintCommandLine(void)
{
	putch('>');
	putch(' ');
}

void ShellPrintVersion(void)
{
	printf("Firmware Version %.2f", (float) FIRMWARE_VERSION);
}