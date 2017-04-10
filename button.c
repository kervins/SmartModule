/* Project:	SmartModule
 * File:	button.c
 * Author:	Jonathan Ruisi
 * Created:	December 20, 2016, 7:52 PM
 */

#include <xc.h>
#include "button.h"
#include "main.h"

// STATUS FUNCTIONS------------------------------------------------------------

void ButtonInfoInitialize(volatile ButtonInfo* button,
						  Action pressAction, Action holdAction, Action releaseAction,
						  bool activeLogicLevel)
{
	button->currentLogicLevel = !activeLogicLevel;
	button->previousLogicLevel = !activeLogicLevel;
	button->currentState = BTN_RELEASE;
	button->isDebouncing = false;
	button->isUnhandled = false;
	button->timestamp = 0;
	button->pressAction = pressAction;
	button->holdAction = holdAction;
	button->releaseAction = releaseAction;
}

void CheckButtonState(volatile ButtonInfo* buttonInfo, bool currentLogicLevel)
{
	if(!buttonInfo->isDebouncing)
	{
		buttonInfo->currentLogicLevel = currentLogicLevel;
		if(buttonInfo->previousLogicLevel != buttonInfo->currentLogicLevel)
		{
			if(!buttonInfo->currentLogicLevel && buttonInfo->currentState != BTN_PRESS)
			{
				INTCON2bits.INTEDG1 = 1;	// Configure INT1 interrupt to trigger on a rising edge
				buttonInfo->currentState = BTN_PRESS;
			}
			else if(buttonInfo->currentLogicLevel && buttonInfo->currentState != BTN_RELEASE)
			{
				INTCON2bits.INTEDG1 = 0;	// Configure INT1 interrupt to trigger on a falling edge
				buttonInfo->currentState = BTN_RELEASE;
			}
			buttonInfo->isUnhandled = true;
			buttonInfo->isDebouncing = true;
			buttonInfo->previousLogicLevel = buttonInfo->currentLogicLevel;
			buttonInfo->timestamp = _tick;
		}
	}
}

void UpdateButton(volatile ButtonInfo* buttonInfo)
{
	if(!buttonInfo->isUnhandled && buttonInfo->currentState == BTN_PRESS
	&& !buttonInfo->currentLogicLevel
	&& (_tick - buttonInfo->timestamp >= HOLD_DELAY))
	{
		buttonInfo->currentState = BTN_HOLD;
		buttonInfo->isUnhandled = true;
	}

	if(buttonInfo->isUnhandled)
	{
		switch(buttonInfo->currentState)
		{
			case BTN_PRESS:
				buttonInfo->pressAction();
				break;
			case BTN_HOLD:
				buttonInfo->holdAction();
				break;
			case BTN_RELEASE:
				buttonInfo->releaseAction();
				break;
		}
		buttonInfo->isUnhandled = false;
	}

	if(buttonInfo->isDebouncing && (_tick - buttonInfo->timestamp >= DEBOUNCE_DELAY))
		buttonInfo->isDebouncing = false;
}