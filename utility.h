/**@file		utility.h
 * @brief		Header file defining useful macros, types, and bit manipulation routines
 * @author		Jonathan Ruisi
 * @version		1.0
 * @date		March 17, 2016
 * @copyright	GNU Public License
 */

#ifndef UTILITY_H
#define UTILITY_H

#include <stdbool.h>

// MACROS----------------------------------------------------------------------
// Math
/**@def MIN(a,b)
 * Computes the minimum of \a a and \a b
 */
#define MIN(a,b) a<b?a:b
/**@def MAX(a,b)
 * Computes the maximum of \a a and \a b
 */
#define MAX(a,b) a>b?a:b
/**@def ABS(a)
 * Computes the absolute value of \a a
 */
#define ABS(a)   a>=0?a:-a
/**@def SIGN(a)
 * Computes the signum (gets the sign) of \a a
 */
#define SIGN(a)  a==0?0:a>0?1:-1

// Bit
/**@def bit_test(var, bit)
 * Gets the state of the current bit (\a bit) in \a var
 */
#define bit_test(var, bit)	(var & (1 << bit))
/**@def bit_set(var, bit)
 * Sets the bit (\a bit) in \a var
 */
#define bit_set(var, bit)	(var |= (1 << bit))
/**@def bit_clear(var, bit)
 * Clears the bit (\a bit) in \a var
 */
#define bit_clear(var, bit)	(var &= ~(1 << bit))
/**@def bit_flip(var, bit)
 * Toggles the bit (\a bit) in \a var
 */
#define bit_flip(var, bit)	(var ^= (1 << bit))

// Bit (will not modify argument)
/**@def set_rightmost_zero
 * Sets the rightmost zero in \a var without modifying \a var, and returns the result
 */
#define set_rightmost_zero(var)			(var | (var + 1))
/**@def clear_rightmost_one
 * Clears the rightmost one in \a var without modifying \a var, and returns the result
 */
#define clear_rightmost_one(var)	(var & (var - 1))
/**@def isolate_rightmost_zero
 * Gets the position of the rightmost zero in \a var
 */
#define isolate_rightmost_zero(var)	(~var & (var + 1))
/**@def isolate_rightmost_one
 * Gets the position of the rightmost one in \a var
 */
#define isolate_rightmost_one(var)	(var & (-var))

// Bitmask
/**@def bitmask_test(var, mask)
 * Performs a bytewise AND operation on \a var based on \a mask
 */
#define bitmask_test(var, mask)		(var & (mask))
/**@def bitmask_set(var, mask)
 * Sets all bits in \a var based on \a mask
 */
#define bitmask_set(var, mask)		(var |= (mask))
/**@def bitmask_clear(var, mask)
 * Clears all bits in \a var based on \a mask
 */
#define bitmask_clear(var, mask)	(var &= (~mask))
/**@def bitmask_flip(var, mask)
 * Toggles all bits in \a var based on \a mask
 */
#define bitmask_flip(var, mask)		(var ^= (mask))

// Byte
/**@def GET_BYTE(value,byteIndex)
 * Retrieves the nth byte (\a byteIndex) from \a value
 */
#define GET_BYTE(value,byteIndex) (unsigned char)((value>>(8*byteIndex))&0xFF)

// DEFINITIONS (ASCII CONTROL CHARACTERS)--------------------------------------
#define ASCII_NUL	0x00	/**< Null Character */
#define ASCII_SOH	0x01	/**< Start of Heading */
#define ASCII_STX	0x02	/**< Start of Text */
#define ASCII_ETX	0x03	/**< End of Text */
#define ASCII_EOT	0x04	/**< End of Transmission */
#define ASCII_ENQ	0x05	/**< Enquiry */
#define ASCII_ACK	0x06	/**< Acknowledgement */
#define ASCII_BEL	0x07	/**< Bell */
#define ASCII_BS	0x08	/**< Backspace */
#define ASCII_HT	0x09	/**< Horizontal Tab */
#define ASCII_LF	0x0A	/**< Line Feed */
#define ASCII_VT	0x0B	/**< Vertical Tab */
#define ASCII_FF	0x0C	/**< Form Feed */
#define ASCII_CR	0x0D	/**< Carriage Return */
#define ASCII_SO	0x0E	/**< Shift Out / X-On */
#define ASCII_SI	0x0F	/**< Shift In / X-Off */
#define ASCII_DLE	0x10	/**< Data Line Escape */
#define ASCII_DC1	0x11	/**< Device Control 1 (XON) */
#define ASCII_XON	ASCII_DC1
#define ASCII_DC2	0x12	/**< Device Control 2 */
#define ASCII_DC3	0x13	/**< Device Control 3 (XOFF) */
#define ASCII_XOFF	ASCII_DC3
#define ASCII_DC4	0x14	/**< Device Control 4 */
#define ASCII_NAK	0x15	/**< Negative Acknowledgement */
#define ASCII_SYN	0x16	/**< Synchronous Idle */
#define ASCII_ETB	0x17	/**< End of Transmit Block */
#define ASCII_CAN	0x18	/**< Cancel */
#define ASCII_EM	0x19	/**< End of Medium */
#define ASCII_SUB	0x1A	/**< Substitute */
#define ASCII_ESC	0x1B	/**< Escape */
#define ASCII_FS	0x1C	/**< File Separator */
#define ASCII_GS	0x1D	/**< Group Separator */
#define ASCII_RS	0x1E	/**< Record Separator */
#define ASCII_US	0x1F	/**< Unit Separator */
#define ASCII_DEL	0x7F	/**< Delete */

// DEFINITIONS (ANSI CONTROL SEQUENCES)----------------------------------------
// Sequences defined here by the terminating character of the sequence
#define ANSI_CUU	0x41	/**< Cursor up						(Opt Param:	#lines) */
#define ANSI_CUD	0x42	/**< Cursor down					(Opt Param:	#lines) */
#define ANSI_CUF	0x43	/**< Cursor forward					(Opt Param:	#chars) */
#define ANSI_CUB	0x44	/**< Cursor back					(Opt Param:	#chars) */
#define ANSI_CPOS	0x48	/**< Set cursor position			(Param:	Line;Column) [omit params for cursor = 0,0] */
#define ANSI_SCPOS	0x73	/**< Save cursor position */
#define ANSI_RCPOS	0x75	/**< Restore cursor position */
#define ANSI_EDISP	0x4A	/**< Clear screen from cursor down */
// Clear screen from cursor up		(Param:	1)
// Clear entire screen				(Param:	2)
#define ANSI_ELINE	0x4B	/**< Clear line from cursor right */
// Clear line from cursor left		(Param:	1)
// Clear entire line				(Param:	2)

// BASIC TYPEDEFS--------------------------------------------------------------
/**@def SCUINT24
 * Defines a <code>static constant unsigned short long int</code>
 */
#define SCUINT24 static const unsigned short long int

// Function Pointers
typedef void (*Action)(void) ;					/**< Basic function pointer */
typedef void (*Action_pV)(void*) ;				/**< Function pointer with a <code>void*</code> argument */
typedef void (*Action_U8)(unsigned char) ;		/**< Function pointer with an <code>unsigned char</code> argument */
typedef void (*Action_U8_U8)(unsigned char, unsigned char) ;	/**< Function pointer with two <code>unsigned char</code> arguments */
typedef unsigned char (*U8_Func)(void) ;		/**< Function pointer with zero arguments that returns an <code>unsigned char</code> */
typedef unsigned char (*U8_Func_pV)(void*) ;	/**< Function pointer with a <code>void*</code> argument that returns an <code>unsigned char</code> */
typedef unsigned char (*U8_Func_U8_U8)(unsigned char, unsigned char) ;	/**<Function pointer with two <code>unsigned char</code> arguments that returns an <code>unsigned char</code> */
typedef bool (*B_Action)(void) ;				/**< Function pointer with zero arguments that returns a <code>bool</code> */

// ENUMERATED TYPES------------------------------------------------------------

/**@enum MonthsOfYear
 * The months of the year
 */
typedef enum
{
	JANUARY		= 0x01,
	FEBRUARY	= 0x02,
	MARCH		= 0x03,
	APRIL		= 0x04,
	MAY			= 0x05,
	JUNE		= 0x06,
	JULY		= 0x07,
	AUGUST		= 0x08,
	SEPTEMBER	= 0x09,
	OCTOBER		= 0x10,
	NOVEMBER	= 0x11,
	DECEMBER	= 0x12
} MonthsOfYear;

/**@enum DaysOfWeek
 * The days of the week
 */
typedef enum
{
	SUNDAY		= 0,
	MONDAY		= 1,
	TUESDAY		= 2,
	WEDNESDAY	= 3,
	THURSDAY	= 4,
	FRIDAY		= 5,
	SATURDAY	= 6
} DaysOfWeek;

/**@enum IoDirection
 * Defines basic read and write operations
 */
typedef enum
{
	Read	= 0,
	Write	= 1
} IoDirection;

// STRUCTURES------------------------------------------------------------------

/**@struct BcdTwoDigit
 * Structure that allows any 8b value to be interpreted as two-digit Binary Coded Decimal
 */
typedef union BcdTwoDigit
{

	struct
	{
		unsigned Ones : 4;		/**< The <b>ones</b> digit */
		unsigned Tens : 4;		/**< The <b>tens</b> digit */
	} ;
	unsigned char ByteValue;	/**< The BCD value as a byte */
} BcdTwoDigit;

/**@struct Date
 * Defines a date based on year, month, and day values
 * @see BcdTwoDigit
 */
typedef struct Date
{
	BcdTwoDigit Year;
	BcdTwoDigit Month;
	BcdTwoDigit Day;
} Date;

/**@struct Time
 * Defines a time based on hour, minute, and second values
 * @see BcdTwoDigit
 */
typedef struct Time
{
	BcdTwoDigit Hour;
	BcdTwoDigit Minute;
	BcdTwoDigit Second;
} Time;

/**@struct DateTime
 * Defines a date and time
 * @see Date
 * @see DaysOfWeek
 * @see Time
 */
typedef struct DateTime
{
	Date date;
	DaysOfWeek weekday;
	Time time;
} DateTime;

/**@struct FileDescriptor
 * Structure that provides both the address and length of a file
 */
typedef struct FileDescriptor
{
	unsigned short long int length;
	unsigned short long int address;
} FileDescriptor;

/**@struct Point
 * Structure that defines a 2D coordinate
 */
typedef struct Point
{
	unsigned char x;
	unsigned char y;
} Point;

#endif