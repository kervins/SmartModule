#ifndef UTILITY_H
#define UTILITY_H

// MACROS----------------------------------------------------------------------
// Math
#define MIN(a,b) a<b?a:b
#define MAX(a,b) a>b?a:b
#define ABS(a)   a>=0?a:-a
#define SIGN(a)  a==0?0:a>0?1:-1

// Bit
#define bit_test(var, bit)	(var & (1 << bit))
#define bit_set(var, bit)	(var |= (1 << bit))
#define bit_clear(var, bit)	(var &= ~(1 << bit))
#define bit_flip(var, bit)	(var ^= (1 << bit))

// Bitmask
#define bitmask_test(var, mask)		(var & (mask))
#define bitmask_set(var, mask)		(var |= (mask))
#define bitmask_clear(var, mask)	(var &= (~mask))
#define bitmask_flip(var, mask)		(var ^= (mask))

// Byte
#define GET_BYTE(value,byteIndex) (uint8_t)(value>>(8*byteIndex))

#endif