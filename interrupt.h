/**@file		interrupt.h
 * @brief		Header file which defines the high and low-priority interrupt service routines
 * @author		Jonathan Ruisi
 * @version		1.0
 * @date		December 19, 2016
 * @copyright	GNU Public License
 */

#ifndef INTERRUPT_H
#define INTERRUPT_H

// FUNCTION PROTOTYPES---------------------------------------------------------
void isrHighPriority(void);
void isrLowPriority(void);

#endif