/**@file		button.h
 * @brief		Header file defining a system for handling button presses using interrupts.
 *				It is capable of executing events for the following button states: Pressed, Held, Released
 * @author		Jonathan Ruisi
 * @version		1.0
 * @date		December 20, 2016
 * @copyright	GNU Public License
 */

#ifndef BUTTON_H
#define BUTTON_H

#include <stdbool.h>
#include "utility.h"

// DEFINITIONS-----------------------------------------------------------------
// Button timing (ms)
#define DEBOUNCE_DELAY	5		/**< Defines the button debounce time (in milliseconds) */
#define HOLD_DELAY		500		/**< Defines the amount of time (in milliseconds) a button must be pressed before it is considered held */

// TYPE DEFINITIONS------------------------------------------------------------

/**@enum ButtonStates
 * Defines the three button states: PRESS, HOLD, RELEASE
 */
typedef enum
{
	BTN_PRESS, BTN_HOLD, BTN_RELEASE
} ButtonStates;

/**@struct ButtonInfo
 * Contains all of the variables necessary to manage one button.
 * One <b>ButtonInfo</b> should be defined for each button in the system.
 */
typedef struct ButtonInfo
{
	unsigned long int timestamp;		/**< A timestamp of the last button event */
	ButtonStates currentState;			/**< The current state of the button */
	unsigned isDebouncing : 1;			/**< A flag indicating that the button is currently in a debounce delay */
	unsigned isUnhandled : 1;			/**< A flag indicating that the current state of the button is unhandled (its corresponding event has not yet executed) */
	unsigned currentLogicLevel : 1;		/**< The current logic level of the port to which the button is connected */
	unsigned previousLogicLevel : 1;	/**< The previous logic level of the port to which the button is connected */
	Action pressAction;					/**< An <b>Action</b> which executes when the button is pressed */
	Action holdAction;					/**< An <b>Action</b> which executes when the button is held */
	Action releaseAction;				/**< An <b>Action</b> which executes when the button is released */
} ButtonInfo;

// FUNCTION PROTOTYPES---------------------------------------------------------
void ButtonInfoInitialize(volatile ButtonInfo* button,
						  Action pressAction, Action holdAction, Action releaseAction,
						  bool activeLogicLevel);
void CheckButtonState(volatile ButtonInfo* buttonInfo, bool currentLogicLevel);
void UpdateButton(volatile ButtonInfo* buttonInfo);
void ButtonPress(void);
void ButtonHold(void);
void ButtonRelease(void);

#endif