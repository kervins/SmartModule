/* Project:	SmartModule
 * File:	common_types.h
 * Author:	Jonathan Ruisi
 * Created:	March 17, 2016, 2:35 AM
 */

#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#include <stdint.h>

// FUNCTION POINTERS-----------------------------------------------------------
typedef void (*Action)(void) ;				// Generic function pointer, typedef'd for readability
typedef uint8_t (*U8Func)(void) ;			// Function pointer that returns an unsigned char
typedef void (*FuncV)(void*) ;				// Function pointer with one parameter (void*)
typedef void (*FuncU8)(uint8_t);			// Function pointer with one parameter (unsigned char)
typedef void (*FuncU8U8)(uint8_t, uint8_t);	// Function pointer with two parameters (unsigned char)

// ENUMERATED TYPES------------------------------------------------------------

typedef enum
{
	JANUARY		= 1,
	FEBRUARY	= 2,
	MARCH		= 3,
	APRIL		= 4,
	MAY			= 5,
	JUNE		= 6,
	JULY		= 7,
	AUGUST		= 8,
	SEPTEMBER	= 9,
	OCTOBER		= 10,
	NOVEMBER	= 11,
	DECEMBER	= 12
} MonthsOfYear;

typedef enum
{
	MONDAY		= 1,
	TUESDAY		= 2,
	WEDNESDAY	= 3,
	THURSDAY	= 4,
	FRIDAY		= 5,
	SATURDAY	= 6,
	SUNDAY		= 7
} DaysOfWeek;

typedef enum
{
	Write	= 0,
	Read	= 1
} IoDirection;

// STRUCTURES------------------------------------------------------------------
// Condenses a complete date and time into an efficient 43b structure

typedef struct
{
	unsigned second		: 6;
	unsigned minute		: 6;
	unsigned hour		: 5;
	unsigned dayOfWeek	: 3;
	unsigned day		: 5;
	unsigned month		: 4;
	unsigned year		: 14;
} DateTime;

// Structure that allows any 8b value to be interpreted as 2-digit Binary Coded Decimal

typedef union
{

	struct
	{
		unsigned Ones : 4;
		unsigned Tens : 4;
	} ;
	uint8_t ByteValue;
} BcdTwoDigit;

// Structure that provides both the address and length of a file

typedef struct
{
	uint24_t length;
	uint24_t address;
} FileDescriptor;

#endif