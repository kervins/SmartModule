#ifndef BUTTONS_H
#define BUTTONS_H

#include <stdint.h>
#include <stdbool.h>
#include "common_types.h"

// TYPE DEFINITIONS------------------------------------------------------------

typedef enum _ButtonStates
{
	BTN_PRESS, BTN_HOLD, BTN_RELEASE
} ButtonStates;

typedef struct _ButtonInfo
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

// FUNCTION PROTOTYPES---------------------------------------------------------

#endif