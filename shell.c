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

// INITIALIZATION FUNCTIONS----------------------------------------------------

void ShellInitialize(uint8_t txTarget, uint8_t rxTarget, bool isEcho)
{
	_shell.txTarget = txTarget;
	_shell.rxTarget = rxTarget;
	_shell.isEcho = isEcho;
	_shell.isEchoWifi = false;
	_shell.isBusy = false;
	_shell.currentAction = 0;
	_shell.paramCount = 0;
	_shell.lineBuffer.hasLine = false;
	_shell.lineBuffer.lineLength = 0;
	printf("SmartModule\n\r");
	ShellPrintVersion();
	putch('\n');
	putch('\r');
	putch('\n');
	putch('\r');
	ShellPrintCommandLine();
}

// CONSOLE FUNCTIONS-----------------------------------------------------------

void ShellGetInput(void)
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
			_shell.lineBuffer.hasLine = true;
			break;
		}
		_shell.lineBuffer.lineData[_shell.lineBuffer.lineLength] = ch;
		_shell.lineBuffer.lineLength++;
		if(_shell.isEcho)
			putch(ch);
	}
}

void ShellParseInput(void)
{
	_shell.isBusy = true;
	char command[16];
	char* pToken = strtok(_shell.lineBuffer.lineData, " ");
	strcpy(command, pToken);

	_shell.paramCount = 0;
	_shell.paramStartIndex = 0;
	while(_shell.paramCount < PARAM_COUNT_MAX)
	{
		pToken = strtok(NULL, " ");
		if(pToken == NULL)
			break;

		_shell.paramStartIndex = pToken - _shell.lineBuffer.lineData;
		int tokenLength = strlen(pToken);
		if(pToken[0] == '-')
		{
			_shell.paramList[_shell.paramCount].flag = pToken[1];
			pToken = strtok(NULL, " ");
			if(pToken == NULL)
				break;
			tokenLength = strlen(pToken);
		}
		else
		{
			_shell.paramList[_shell.paramCount].flag = NULL;
		}

		_shell.paramList[_shell.paramCount].index = pToken - _shell.lineBuffer.lineData;
		_shell.paramList[_shell.paramCount].length = tokenLength;
		_shell.paramCount++;
	}

	if(strnicmp(command, "version", 7) == 0)
	{
		_shell.currentAction = ShellPrintVersion;
	}
	else if(strnicmp(command, "wificmd", 4) == 0)
	{
		_shell.currentAction = ShellWifiCommand;
	}
	else
		_shell.currentAction = ShellPrintInvalid;
}

void ShellEndExecution(void)
{
	_shell.isBusy = false;
	_shell.lineBuffer.lineLength = 0;
	_shell.lineBuffer.hasLine = false;
}

void ShellPrintNewLine(void)
{
	putch(ASCII_CR);
	putch(ASCII_LF);
}

void ShellPrintCommandLine(void)
{
	putch('>');
	putch(' ');
}

void ShellPrintInvalid(void)
{
	printf("ERROR: Invalid input");
	ShellEndExecution();
}

void ShellPrintVersion(void)
{
	printf("Firmware Version %.2f", (float) FIRMWARE_VERSION);
	ShellEndExecution();
}

void ShellWifiCommand(void)
{
	if(_shell.paramCount != 1 && _shell.paramList[0].flag != NULL)
		ShellPrintInvalid();
	else
	{
		_shell.txTarget = 1;
		_shell.rxTarget = 1;
		printf("%s", _shell.lineBuffer.lineData + _shell.paramStartIndex);
		ShellEndExecution();
	}
}