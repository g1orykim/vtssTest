

/*******************************************************************************
*
*  $Id: zl303xx_DataTypes.h 7835 2012-03-13 16:44:32Z JK $
*
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     The definitions of the Zarlink standard data types
*
*******************************************************************************/

#ifndef _ZL_DATATYPES_H_
#define _ZL_DATATYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

/*****************   INCLUDE FILES   ******************************************/
#include <sys/types.h>

/*****************   DEFINES   ************************************************/

/* Common constant used for many calculations */
#define ONE_BILLION     (Uint32T)1000000000
#define ONE_MILLION     (Uint32T)1000000
#define ONE_THOUSAND    (Uint32T)1000

/* Used to convert the ScaledNanoT (defined below) */
#define SCALED_NS32_FRACT_BITS   16
#define SCALED_NS32_FRACT_MASK   (Uint32T)((1 << SCALED_NS32_FRACT_BITS) - 1)
#define SCALED_NS32_WHOLE_BITS   ((sizeof(Uint32T) * 8) - SCALED_NS32_FRACT_BITS)
#define SCALED_NS32_WHOLE_MASK   (Uint32T)(~(SCALED_NS32_FRACT_MASK))

/* Some datatype bit-size constants */
#define ZL303XX_BITS_PER_BYTE   (Uint8T)(8)
#define ZL303XX_BITS_PER_U8     (Uint8T)(ZL303XX_BITS_PER_BYTE * sizeof(Uint8T))
#define ZL303XX_BITS_PER_U4     (Uint8T)(ZL303XX_BITS_PER_U8 / 2)
#define ZL303XX_BITS_PER_U16    (Uint8T)(ZL303XX_BITS_PER_BYTE * sizeof(Uint16T))
#define ZL303XX_BITS_PER_U32    (Uint8T)(ZL303XX_BITS_PER_BYTE * sizeof(Uint32T))
#define ZL303XX_BITS_PER_U64    (Uint8T)(ZL303XX_BITS_PER_U32 * 2)

/*****************   DATA TYPES   *********************************************/
/* Also see zl303xx_Os.h ! */

/* The following settings are known to work for ILP32 and LP64 data models */
typedef unsigned char Uint8T;
typedef signed char Sint8T;

typedef unsigned short Uint16T;
typedef signed short Sint16T;

typedef unsigned int Uint32T;
typedef signed int Sint32T;

/* 
UnativeT and SnativeT should be set to the native CPU architecture size 
(e.g. 32-bit or 64-bit) and have the same dimensions as a pointer (void *)
*/
typedef unsigned long UnativeT; 
typedef signed long SnativeT;

/* Used for casts to system-specific calls */
#ifndef UCHAR_T
#define		UCHAR_T		unsigned char
#endif
#ifndef USHORT_T
#define		USHORT_T	unsigned short
#endif
#ifndef UINT_T
#define 	UINT_T		unsigned int
#endif
#ifndef SINT_T
#define 	SINT_T		signed int
#endif
#ifndef SLONG_T
#define		SLONG_T		signed long
#endif
#ifndef ULONG_T
#define		ULONG_T		unsigned long
#endif

/*****************   DATA STRUCTURES   ****************************************/

/* Structure used to support 64-bit counters without requiring 64-bit support
   in the compiler */
typedef struct
{
   Uint32T hi;
   Uint32T lo;
} Uint64S;

/* 32-bit Uint + 32-bit remainder */
typedef Uint64S Uint32r32S;

/* 32-bit Uint + 32-bit fraction of (0xFFFFFFFF + 1) */
typedef Uint64S Uint32f32S;

/* 32-bit Uint + 32-bit nanoseconds (fraction of 1,000,000,000)
   (Equivalent to 32-bit decimal with 9 decimal place precision) */
typedef Uint64S Uint32n32S;

/* Define a type in which the upper 16-bits of a 32-bit variable represents
   nanoseconds and the lower 16-bits represents fractional nanoseconds. This
   will allow us to store a clock period down to 65535 nanoSec (15.3KHz). */
typedef Uint32T ScaledNs32T;

/*****************   EXPORTED GLOBAL VARIABLE DECLARATIONS   ******************/

/*****************   EXTERNAL FUNCTION DECLARATIONS   *************************/

/* Basic Uint64S Math operations (see function headers for details) */
Uint64S Add_U64S(Uint64S val1, Uint64S val2, Uint8T *carry);
Uint64S Diff_U64S(Uint64S val1, Uint64S val2, Uint8T *isNegative);
Uint64S Mult_U32_U32(Uint32T val1, Uint32T val2);
Uint64S Mult_U64S_U32(Uint64S val64s, Uint32T val32, Uint32T *overflow);
Uint64S Div_U64S_U32(Uint64S num, Uint32T den, Uint32T *mod);
Uint64S Div_U64S_U64S(Uint64S num, Uint64S den, Uint64S *rem);

/* Compares 1 Uint64S value to another */
Sint8T Compare_U64S(Uint64S val1, Uint64S val2);

/* Routines to fine the highest order bit in a Uint32T or Uint64S. */
/* Used to optimize some of the other math routines. */
Uint8T HiBit_U32(Uint32T val);
Uint8T HiBit_U64S(Uint64S val);

/* Uint64S Shift functions */
Uint64S LShift_U64S(Uint64S inVal, Uint8T lshift);
Uint64S RShift_U64S(Uint64S inVal, Uint8T rshift);
Uint64S LShift1_U64S(Uint64S inVal);
Uint64S RShift1_U64S(Uint64S inVal);

/* Bit Logic functions */
Uint64S BitNOT_U64S(Uint64S inVal);
Uint64S BitAND_U64S(Uint64S val1, Uint64S val2);
Uint64S BitOR_U64S(Uint64S val1, Uint64S val2);

/* Routine performs the operation: (n1 * n2) / d1
   It also provides a flag indicating if the fractional remainder as a result
   of the division would round the final value up (0 or 1) */
Uint32T Mult_Mult_Div_U32(Uint32T n1, Uint32T n2, Uint32T d1,
                          Uint8T *round, Uint32T *overflow);

/* Routine to convert between any 2 Uint32T ratios. Useful when converting tick
   counts from one frequency to another. */
Uint32T RatioConvert_U32(Uint32T n1, Uint32T d1, Uint32T d2);

/* Routine to convert a ratio to a 32-bit fraction. Any whole number portion
   can also be provided. */
Uint32T Ratio_U32_U32(Uint32T n1, Uint32T d1, Uint32T *whole32);

/* Routines for converting between fractional and nanosecond formats. Useful
   when using the Uint32f32S and Uint32n32S types. */
Uint32T FractToNanos(Uint32T fract);
Uint32T NanosToFract(Uint32T nanos, Uint32T *seconds);

/* Routine for calculating a clock period expressed as a ScaledNs32T type. */
ScaledNs32T ClockPeriod_ScaledNs(Uint32T freqHz);

/* Routines to calculate the nanosecond time associated with the tick count of a
   clock with the given period or frequency. */
Uint64S TicksToNano_ScaledNsPeriod(Uint32T ticks, ScaledNs32T period, Uint32T *fNano);
Uint64S TicksToNano_ClkFreqHz(Uint32T ticks, Uint32T freqHz, Uint32T *fNano);

#ifdef __cplusplus
}
#endif

#endif   /* MULTIPLE INCLUDE BARRIER */

