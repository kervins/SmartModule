/* Project:	SmartModule
 * File:	button.h
 * Author:	Jonathan Ruisi
 * Created:	December 20, 2016, 7:52 PM
 */

#ifndef BUTTON_H
#define BUTTON_H

#include <stdint.h>
#include <stdbool.h>
#include "common_types.h"

// DEFINITIONS-----------------------------------------------------------------
// Button timing (ms)
#define DEBOUNCE_DELAY	5
#define HOLD_DELAY		500

// TYPE DEFINITIONS------------------------------------------------------------

typedef enum
{
	BTN_PRESS, BTN_HOLD, BTN_RELEASE
} ButtonStates;

typedef struct
{
	uint32_t timestamp;
	ButtonStates currentState;
	unsigned isDebouncing : 1;
	unsigned isUnhandled : 1;
	unsigned currentLogicLevel : 1;
	unsigned previousLogicLevel : 1;
	Action pressAction;
	Action holdAction;
	Action releaseAction;
} ButtonInfo;

// GLOBAL VARIABLES------------------------------------------------------------
extern volatile ButtonInfo _button;

// FUNCTION PROTOTYPES---------------------------------------------------------
ButtonInfo ButtonInfoCreate(Action pressAction, Action holdAction, Action releaseAction, bool activeLogicLevel);
void UpdateButtonState(volatile ButtonInfo* buttonInfo, bool currentLogicLevel);
void CheckButton(volatile ButtonInfo *buttonInfo);
void ButtonPress(void);
void ButtonHold(void);
void ButtonRelease(void);

#endif