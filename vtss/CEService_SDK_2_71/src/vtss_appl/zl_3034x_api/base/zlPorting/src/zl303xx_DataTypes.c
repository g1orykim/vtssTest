



/*******************************************************************************
*
*  $Id: zl303xx_DataTypes.c 7159 2011-11-16 21:34:27Z SW $
*
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     This file contains functions related to the Ported Data Types
*
*******************************************************************************/

/*****************   INCLUDE FILES   ******************************************/

#include "zl303xx_DataTypes.h"

/*****************   DEFINES     **********************************************/

/* Used for some frequency calculations */
#define FREQ_MULTIPLE   (Uint32T)8000

/* define the ratio (0x100000000/1Billion) to make some of the math faster
   (Helps avoid division in several frequently used calculations).

   (2^32)/1Billion = 4.294967296
   Converting to 32-bit fractional form gives 0x04:0x4B82FA09 (U64S format).
   Shifting everything right by 3 bits gives       0x89705F41 (U32 format).
   Note that the 3 MSBs are the whole portion (4) and the 29 LSBs are the
   fractional portion (use the U3F29 convention as a reminder).     */
#define RATIO_2EXP32_1BILLION_U3F29    (Uint32T)0x89705F41

/* Macros to split up a U32 into its U16 parts. */
#define UPPER_16BITS(val)  (((val) >> 16) & 0x0000FFFF)
#define LOWER_16BITS(val)  ((val) & 0x0000FFFF)

/* A LOCAL Utility macro to add 2, 32-bit values and determine any carry. If it
   is known by the user that the addition will not overflow (or does not care
   about overflow) then the regular addition expression (+) should be used. */
/* If the result needs to be assigned to one of the input variables, use val1
   since val2 is used to determine the carry/overflow value. */
#define ADD_U32(val1,val2,result,carry)      \
   {                                         \
      result = val1 + val2;                  \
      carry = ((result < val2) ? 1 : 0);     \
   }

/* Macro to find the highest order bit in a Uint32T variable. */
/* This implementation does a binary search without the need for local variable
   & mask updating and therefore is much faster (which is the priority for the
   64-bit calculations that will use it), although it does make it look a bit
   cumbersome. */
/* It is defined locally to this module since it will mostly be used by other
   math functions here. An exported function is also provided to allow use of
   the functionality without bloating the code with an inline macro. */
/* The range is from 1 to 32 for non-zero values; 0 if val == 0 (so that error
   checking can be done). */
#define HI_BIT_U32(val)                                        \
   (Uint8T)((val & 0xFFFF0000) ?                               \
               ( (val & 0xFF000000) ?                          \
                  ( (val & 0xF0000000) ?                       \
                     ( (val & 0xC0000000) ?                    \
                        ((val & 0x80000000) ? 32 : 31) :       \
                        ((val & 0x20000000) ? 30 : 29)         \
                     ) :                                       \
                     ( (val & 0x0C000000) ?                    \
                        ((val & 0x08000000) ? 28 : 27) :       \
                        ((val & 0x02000000) ? 26 : 25)         \
                     ) ) :                                     \
                  ( (val & 0x00F00000) ?                       \
                     ( (val & 0x00C00000) ?                    \
                        ((val & 0x00800000) ? 24 : 23) :       \
                        ((val & 0x00200000) ? 22 : 21)         \
                     ) :                                       \
                     ( (val & 0x000C0000) ?                    \
                        ((val & 0x00080000) ? 20 : 19) :       \
                        ((val & 0x00020000) ? 18 : 17)         \
                     )                                         \
                  ) ) :                                        \
               ( (val & 0x0000FFFF) ?                          \
                  ( (val & 0xFF00) ?                           \
                     ( (val & 0xF000) ?                        \
                        ( (val & 0xC000) ?                     \
                           ((val & 0x8000) ? 16 : 15) :        \
                           ((val & 0x2000) ? 14 : 13)          \
                        ) :                                    \
                        ( (val & 0x0C00) ?                     \
                           ((val & 0x0800) ? 12 : 11) :        \
                           ((val & 0x0200) ? 10 : 9)           \
                        ) ) :                                  \
                     ( (val & 0x00F0) ?                        \
                        ( (val & 0x00C0) ?                     \
                           ((val & 0x0080) ? 8 : 7) :          \
                           ((val & 0x0020) ? 6 : 5)            \
                        ) :                                    \
                        ( (val & 0x000C) ?                     \
                           ((val & 0x0008) ? 4 : 3) :          \
                           ((val & 0x0002) ? 2 : 1)            \
                        ) ) ) : ( 0 ) ) )

/* Macro to find the highest order bit in a Uint64S variable. */
/* The range is from 1 to 64 for non-zero values; 0 if val == 0 (so that error
   checking can be done). */
#define HI_BIT_U64S(val)                                       \
   (Uint8T)((val.hi != 0) ?                                    \
            (ZL303XX_BITS_PER_U32 + HI_BIT_U32(val.hi)) :        \
            (HI_BIT_U32(val.lo)))


/***** COMPARE MACROS ******/

/* Macro to compare two Uint32T variables. This might not be used much for real
   32-bit values but is useful for evaluating portions of Uint64S values.     */
/* Returns 1 if val1 > val2; -1 if val1 < val2; 0 if val1 == val2             */
#define COMPARE_U32(val1,val2)                                          \
   (Sint8T)( (val1 > val2) ? (1) : ((val1 < val2) ? (-1) : (0)) )

/* Macro to compare two Uint64S variables.      */
/* Returns 1 if val1 > val2; -1 if val1 < val2; 0 if val1 == val2             */
#define COMPARE_U64S(val1,val2)                 \
   (Sint8T)((val1.hi > val2.hi) ? (1) :         \
            ( (val1.hi < val2.hi) ? (-1) : (COMPARE_U32(val1.lo, val2.lo))))


/***** SHIFT MACROS ******/

/* Macro to shift a Uint64S variables right by 1 bit.      */
#define RSHIFT_LESS32_U64S(val,shift)                                      \
   {                                                                       \
      val.lo = ((val.lo >> shift) | (val.hi << (ZL303XX_BITS_PER_U32 - shift)));   \
      val.hi = (val.hi >> shift);                                          \
   }

/* Macro to shift a Uint64S variables left by 1 bit.      */
#define LSHIFT_LESS32_U64S(val,shift)                                      \
   {                                                                       \
      val.hi = ((val.hi << shift) | (val.lo >> (ZL303XX_BITS_PER_U32 - shift)));  \
      val.lo = (val.lo << shift);                                          \
   }

/*****************   STATIC DATA TYPES   **************************************/

/* Local enum for use in switch statements to make the result of the compare
   operation more obvious */
typedef enum
{
   COMPARE_LESS_THAN       = -1,
   COMPARE_EQUAL           = 0,
   COMPARE_GREATER_THAN    = 1
} CompareTypeE;

/*****************   STATIC DATA STRUCTURES   *********************************/

/*****************   STATIC FUNCTION DECLARATIONS   ***************************/

/*****************   STATIC GLOBAL VARIABLES   ********************************/

/* define the ratio (0x100000000/1Billion) as a Uint64S (Uint32f32S). */
/* (2^32)/1Billion = 4.294967296 */
const Uint32f32S Ratio_2Exp32_1Billion = { (Uint32T)0x04, (Uint32T)0x4B82FA09 };

/* define the ratio (1Billion/0x100000000) as a Uint32T. */
/* 1Billion/(2^32) = 0.232830644 */
const Uint32T Ratio_1Billion_2Exp32 = (Uint32T)0x3B9ACA00;

/* Used for fast calculation of a clock period given the frequency. Since all
   frequencies are multiples of 8KHz, dividing the 8KHz period by a frequency
   multiple gives the period of the input frequency. As an even faster method,
   if the U64S constant is shifted right by 1-bit, then 32-bit math can be
   used instead. The final result has to be corrected by shifting left by 1-bit
   so will always have 0 as the LSB which introduces an error of
   1/65536 ns (0.0000152587890625 ns => ~15 femto Sec */
const Uint64S _8KHzPeriod_ScaledNs = { 0x01, 0xE8480000 };
const Uint32T Shifted_8KHzPeriod_ScaledNs = (Uint32T)0xF4240000;

/*****************   EXPORTED GLOBAL VARIABLES   ******************************/

/*****************   EXPORTED FUNCTION DEFINITIONS   **************************/

/**

  Function Name:
   Add_U64S

  Details:
   Function to add 2 Uint64S values.

  Parameters:
       val1     First of the 2 Uint64S values to be added
       val2     Second of the 2 Uint64S values to be added
       carry    Flag indicating if sum of the 2 Uint64S values was too large
               for the Uint64S type (carry-bit).
            If carry == NULL, this flag is not computed.

  Return Value:
   Uint64S  Sum of the 2 input Uint64S values

*******************************************************************************/

Uint64S Add_U64S(Uint64S val1, Uint64S val2, Uint8T *carry)
{
   Uint64S result;

   /* Do the basic addition */
   result.hi = val1.hi + val2.hi;
   result.lo = val1.lo + val2.lo;

   /* Determine if the lower portion overflowed */
   if (result.lo < val1.lo)
   {
      result.hi++;
   }

   /* Check the function input Parameter Pointers */
   if (carry)
   {
      /* Determine if the upper portion overflowed */
      *carry = ((result.hi < val1.hi) ? (1) : (0));
   }

   return result;
}  /* END Add_U64S */

/**

  Function Name:
   Diff_U64S

  Details:
   Function to find the difference between 2 Uint64S values.

  Parameters:
       val1        Uint64S value to subtract val2 from
       val2        Uint64S value to be subtracted from val1
       isNegative  Flag indicating if (val1 - val2) is negative
               If isNegative == NULL, this flag is not computed.

  Return Value:
   Uint64S     Differnece between the 2 input Uint64S values

*******************************************************************************/

Uint64S Diff_U64S(Uint64S val1, Uint64S val2, Uint8T *isNegative)
{
   Uint64S result;

   /* Do the basic subtraction */
   result.lo = val1.lo - val2.lo;
   result.hi = val1.hi - val2.hi;

   /* Check if we should have borrowed */
   if (result.lo > val1.lo)
   {
      result.hi--;
   }

   /* Check the function input Parameter Pointer */
   if (isNegative)
   {
      /* determine if val1 was less than val2 to begin */
      *isNegative = ((result.hi > val1.hi) ? (1) : (0));
   }

   return result;
}  /* END Diff_U64S */

/**

  Function Name:
   Compare_U64S

  Details:
   Function to compare two Uint64S values.

  Parameters:
       val1        Uint64S value 1
       val2        Uint64S value 2

  Return Value:
   Sint8T      1: if val1 > val2
               0: if val1 = val2
              -1: if val1 < val2

*******************************************************************************/

Sint8T Compare_U64S(Uint64S val1, Uint64S val2)
{
   return COMPARE_U64S(val1,val2);
}  /* END Compare_U64S */

/**

  Function Name:
   Mult_U32_U32

  Details:
   Function to multiply 2 Uint32T values to produce a Uint64S.

  Parameters:
       val1        First of the Uint32T factors
       val2        Second of the Uint32T factors

  Return Value:
   Uint64S     Result of multiplying the 2 input Uint32T values

  Notes:
   The multiplication of 2 - 32bit values cannot overflow a 64bit type.

*******************************************************************************/

Uint64S Mult_U32_U32(Uint32T val1, Uint32T val2)
{
   /* Return Value */
   Uint64S result;
   Uint32T temp32;

   /* Determine the result of multiplying the cross-words and set the temp
      variable to the result and carry value. */
   ADD_U32(LOWER_16BITS(val2) * UPPER_16BITS(val1),
           UPPER_16BITS(val2) * LOWER_16BITS(val1),
           result.lo, result.hi);

   /* Shift this term up to it proper position */
   /* Do not use the MACRO or Fn since the exact position is known. (This
      will avoid some 32-bit subtraction operations). */
   result.hi = (result.hi << ZL303XX_BITS_PER_U16) | UPPER_16BITS(result.lo);
   result.lo <<= ZL303XX_BITS_PER_U16;

   /* Add the rest of the lower portion of the result */
   temp32 = LOWER_16BITS(val2) * LOWER_16BITS(val1);
   result.lo += temp32;

   /* Get the rest of the high portion of the result */
   result.hi += UPPER_16BITS(val2) * UPPER_16BITS(val1);

   /* Increment HI on LO overflow */
   if (result.lo < temp32)
   {
      result.hi++;
   }

   return result;
}  /* END Mult_U32_U32 */

/**

  Function Name:
   Mult_U64S_U32

  Details:
   Function to multiply a Uint64S value by a Uint32T value to produce a Uint64S

  Parameters:
       val64s      The Uint64S value
       val32       The Uint32T value
       overflow    Pointer to the 32-bit overflow (if any)
               If overflow == NULL, this portion is not returned.

  Return Value:
   Uint64S     Bits 0-63 from the result of multiplying the 2 input values

  Notes:
   Since the result can potentially be a 96-bit type, any overflow (bits 64-95)
   is returned via the overflow parameter pointer (if provided, otherwise it is
   discarded).
   This gives flexibility when multiplying Uint64S types with fractional parts
   so the caller can format the actual result given its knowledge of the inputs.

*******************************************************************************/

Uint64S Mult_U64S_U32(Uint64S val64s, Uint32T val32, Uint32T *overflow)
{
   /* Return Value */
   Uint64S result = Mult_U32_U32(val64s.lo, val32);   /* Bits  0-63 */
   Uint64S temp64s = Mult_U32_U32(val64s.hi, val32);  /* Bits 32-95 */
   Uint8T carry;

   ADD_U32(result.hi,temp64s.lo,result.hi,carry);

   /* If overflow is provided, determine it */
   if (overflow != (void *)(0))
   {
      *overflow = temp64s.hi + carry;
   }

   return result;
}  /* END Mult_U64S_U32 */

/**

  Function Name:
   Div_U64S_U32

  Details:
   Function to divide a Uint64S value by a Uint32T value to produce a Uint64S
      result->hi = the 32-bit whole portion of the result
      result->lo = the 32-bit remainder (modulus)

  Parameters:
       num      64-bit numerator
       den      32-bit denominator
       mod      Pointer to the 32-bit remainder (if any)
            If mod == NULL, this portion is not returned.

  Return Value:
   Uint64S  Result of the division operation (whole portion)

*******************************************************************************/

Uint64S Div_U64S_U32(Uint64S num, Uint32T den, Uint32T *mod)
{
   Uint64S result;
   Uint64S modulus;

   /* Proceed based on the denominator */
   switch (den)
   {
      case 0 : /* Check for divide by 0 error */
      {
         result.hi = (Uint32T)(-1);
         result.lo = (Uint32T)(-1);
         modulus.lo = (Uint32T)(-1);
         break;
      }

      case 1 : /* The default case fails for dem = 1 due to the modulus
                  operation which is OK since we know the result immediately. */
      {
         result = num;
         modulus.lo = 0;
         break;
      }

      default :   /* We could check for num = 0 but it makes the overall average
                     time of the algorithm slower. */
      {
         /* Overview:

               num     num.hi:num.lo    (num.hi * 0x100000000) + num.lo
              ----- = --------------- = --------------------------------
               den          den                      den                      */

         /* Suppose numer = 0x00000001:0x00000000, the result of numer/denom
            would tell us how much each unit of numer.hi contributes to the
            whole and modulus counts.

            So, if we calculate the whole and modulus contribution factors for
            (0x01:0x00)/denom we can multiple them by the actual numer.hi to get
            the resulting whole and modulus portion resulting from numer.hi */

         /* So, num.hi * 0x100000000
             =  num.hi * (0xFFFFFFFF + 1)
             =  num.hi * 0xFFFFFFFF + num.hi    */
         Uint32T hiWholeFact = (Uint32T)(0xFFFFFFFF) / den;
         Uint32T hiModFact = (Uint32T)(0xFFFFFFFF) % den + 1;

         result.hi = 0;
         result.lo = 0;
         while (num.hi)
         {
            result = Add_U64S(result,
                              Mult_U32_U32(hiWholeFact, num.hi),
                              (void *)0);   /* carry always = 0 */

            modulus = Mult_U32_U32(hiModFact, num.hi);

            /* add the newly calculated modulus to the previous num.lo
               (the num.hi has been processed above). */
            ADD_U32(num.lo, modulus.lo, num.lo, num.hi);
            num.hi += modulus.hi;
         }

         /* We are left with only a low portion now so use 32-bit math. */
         modulus.lo = num.lo % den;    /* modulus.hi contains junk      */

         ADD_U32(result.lo, (num.lo / den), result.lo, num.hi);
         result.hi += num.hi;

         break;
      }
   }

   if (mod)
   {
      /* User has requested the final modulus */
      *mod = modulus.lo;
   }

   return result;
}  /* END Div_U64S_U32 */

/**

  Function Name:
   Div_U64S_U64S

  Details:
   Function to divide a Uint64S value by a Uint64S value to produce a Uint64S

  Parameters:
      num      64-bit numerator
      den      64-bit denominator

      mod      Pointer to the 64-bit remainder (if any)
            If mod == NULL, this portion is not returned.

  Return Value:
   Uint64S  Result of the division operation (whole portion)

*******************************************************************************/
Uint64S Div_U64S_U64S(Uint64S num, Uint64S den, Uint64S *mod)
{
   /* Compute remainder and quotient at the same time using the algorithm:

      quotient = numerator;
      remainder = 0;
      for i = 0 to (numBits - 1)
         remainder:quotient <<= 1;    where remainder:quotient is 128 bits wide
         if (remainder >= denominator)
            remainder -= denominator;
            quotient++;                                                       */

   Uint64S quotient = num, remainder = {0,0};
   Sint8T shiftDist = 0;

   /* Check for divide-by-zero error. */
   if ((den.hi == 0) && (den.lo == 0))
   {
      quotient.hi = quotient.lo = remainder.hi = remainder.lo = (Uint32T)-1;
   }

   /* Check for trivial divide-by-one case. */
   else if ((den.hi == 0) && (den.lo == 1))
   {
      /* Don't need to set anything, variables are already initialized with
       * appropriate data to return. */
   }

   else
   {
      for (; shiftDist < ZL303XX_BITS_PER_U64; shiftDist++)
      {
         /* remainder:quotient <<= 1; */
         remainder = LShift1_U64S(remainder);
         if (quotient.hi & 0x80000000)
         {
            remainder.lo++;
         }
         quotient = LShift1_U64S(quotient);

         /* if (remainder >= denominator) */
         if (Compare_U64S(remainder, den) >= 0)
         {
            /* remainder -= denominator; */
            remainder = Diff_U64S(remainder, den, (void *)(0));

            /* quotient++; */
            quotient.lo++;
         }
      }
   }

   if (mod)
   {
      /* User has requested the final modulus. */
      *mod = remainder;
   }

   return quotient;
}  /* END Div_U64S_U64S */

/**

  Function Name:
   HiBit_U32

  Details:
   Function used to fine the highest order bit in a Uint32T.

  Parameters:
       val      Input Uint32T value

  Return Value:
   Uint8T   Bit position of the highest order bit that is set.
            Range from 32 (highest bit - 0x80000000)
                    to  1 (lowest bit -  0x00000001)
            Returns 0 to indicate no bit is set (val == 0)

*******************************************************************************/

Uint8T HiBit_U32(Uint32T val)
{
   return HI_BIT_U32(val);
}  /* END HiBit_U32 */

/**

  Function Name:
   HiBit_U64S

  Details:
   Function used to fine the highest order bit in a Uint64S.

  Parameters:
       val      Input Uint64S value

  Return Value:
   Uint8T   Bit position of the highest order bit that is set.
            Range from 64 (highest bit - 0x80000000:0x00000000)
                    to  1 (lowest bit -  0x00000000:0x00000001)
            Returns 0 to indicate no bit is set (val == 0)

*******************************************************************************/

Uint8T HiBit_U64S(Uint64S val)
{
   return HI_BIT_U64S(val);
}  /* END HiBit_U32 */

/**

  Function Name:
   LShift_U64S

  Details:
   Function to shift a Uint64S value to the left by lshift number of bits.

  Parameters:
       inVal    Input Uint64S value
       lshift   The number of bits to shift the input value to the left

  Return Value:
   Uint64S  Input Uint64S value shifted by lshift bits to the left

*******************************************************************************/

Uint64S LShift_U64S(Uint64S inVal, Uint8T lshift)
{
   Uint64S result;

   if (lshift < ZL303XX_BITS_PER_U32)          /* <= 31 */
   {
      result.hi = (inVal.hi << lshift) | (inVal.lo >> (ZL303XX_BITS_PER_U32 - lshift));
      result.lo = inVal.lo << lshift;
   }
   else if (lshift < ZL303XX_BITS_PER_U64)     /* 32 - 63 */
   {
      /* All of the HI bits will have been shifted into overflow
         All of the LO bits will have been shifted into HI */
      result.hi = inVal.lo << (lshift - ZL303XX_BITS_PER_U32);
      result.lo = 0;
   }
   else  /* >= 64 */
   {
      /* All bits will be shifted left into overflow */
      result.hi = 0;
      result.lo = 0;
   }

   return result;
}  /* END LShift_U64S */

/**

  Function Name:
   RShift_U64S

  Details:
   Function to shift a Uint64S value to the right by rshift number of bits.

  Parameters:
       inVal    Input Uint64S value
       rshift   The number of bits to shift the input value to the right

  Return Value:
   Uint64S  Input Uint64S value shifted by rshift bits to the right

  Notes:
   Since this is an Unsigned right-shift, 0's are shifted in from the MS bit

*******************************************************************************/

Uint64S RShift_U64S(Uint64S inVal, Uint8T rshift)
{
   Uint64S result;

   if (rshift < ZL303XX_BITS_PER_U32)       /* <= 31 */
   {
      result.lo = (inVal.lo >> rshift) | (inVal.hi << (ZL303XX_BITS_PER_U32 - rshift));
      result.hi = inVal.hi >> rshift;
   }
   else if (rshift < ZL303XX_BITS_PER_U64)  /* 32 - 63 */
   {
      /* All of the LO bits will have been shifted into underflow
         All of the HI bits will have been shifted into LO */
      result.hi = 0;
      result.lo = inVal.hi >> (rshift - ZL303XX_BITS_PER_U32);
   }
   else  /* >= 64 */
   {
      /* All bits will be shifted right into underflow */
      result.hi = 0;
      result.lo = 0;
   }

   return result;
}  /* END RShift_U64S */

/**

  Function Name:
   LShift1_U64S

  Details:
   Function to shift a Uint64S value to the left by 1 bit.

  Parameters:
       inVal    Input Uint64S value

  Return Value:
   Uint64S  Input Uint64S value shifted to the left by 1 bit

*******************************************************************************/

Uint64S LShift1_U64S(Uint64S inVal)
{
   /* Use the defined macro */
   LSHIFT_LESS32_U64S(inVal,1);

   return inVal;
}  /* END LShift1_U64S */

/**

  Function Name:
   RShift1_U64S

  Details:
   Function to shift a Uint64S value to the right by 1 bit.

  Parameters:
       inVal    Input Uint64S value

  Return Value:
   Uint64S  Input Uint64S value shifted to the right by 1 bit

*******************************************************************************/

Uint64S RShift1_U64S(Uint64S inVal)
{
   /* Use the defined macro */
   RSHIFT_LESS32_U64S(inVal,1);

   return inVal;
}  /* END RShift1_U64S */

/**

  Function Name:
   BitNOT_U64S

  Details:
   Function to perform a bitwise NOT operation on a Uint64S.

  Parameters:
       inVal    Input Uint64S value

  Return Value:
   Uint64S  ~(inVal)

*******************************************************************************/

Uint64S BitNOT_U64S(Uint64S inVal)
{
   inVal.hi = ~(inVal.hi);
   inVal.lo = ~(inVal.lo);

   return inVal;
}  /* END BitNOT_U64S */

/**

  Function Name:
   BitAND_U64S

  Details:
   Function to perform a bitwise AND operation on 2 Uint64S values.

  Parameters:
       val1        First of the Uint64S values
       val2        Second of the Uint64S values

  Return Value:
   Uint64S  val1 & val2

*******************************************************************************/

Uint64S BitAND_U64S(Uint64S val1, Uint64S val2)
{
   Uint64S retVal;

   retVal.hi = val1.hi & val2.hi;
   retVal.lo = val1.lo & val2.lo;

   return retVal;
}  /* END BitAND_U64S */

/**

  Function Name:
   BitOR_U64S

  Details:
   Function to perform a bitwise OR operation on 2 Uint64S values.

  Parameters:
       val1        First of the Uint64S values
       val2        Second of the Uint64S values

  Return Value:
   Uint64S  val1 | val2

*******************************************************************************/

Uint64S BitOR_U64S(Uint64S val1, Uint64S val2)
{
   Uint64S retVal;

   retVal.hi = val1.hi | val2.hi;
   retVal.lo = val1.lo | val2.lo;

   return retVal;
}  /* END BitOR_U64S */

/**

  Function Name:
   Mult_Mult_Div_U32

  Details:
   Used to implement the following formula:


                 n1 * n2
       result = ---------
                   d1


   Proves useful when the final result should be less than a 32-bit value.
   Otherwise, the overflow output can be used to return the 64-bit extension.

  Parameters:
       n1       Numerator value 1 (32-bits)
       n2       Numerator value 2 (32-bits)
       d2       Denominator value (32-bits)

       round       Flag indicating if the decimal remainder as a result of the
                  division is >= 0.5 ( >= 0.5 then round = 1; otherwise = 0).
                  (If round == NULL, this value is ignored).
       overflow    For results that require more than 32-bits, this contains the
                  upper 32-bits of the result.
                  (If overflow == NULL, this value is ignored).

  Return Value:
   result    Result of the math operation

*******************************************************************************/
Uint32T Mult_Mult_Div_U32(Uint32T n1, Uint32T n2, Uint32T d1,
                          Uint8T *round, Uint32T *overflow)
{
   Uint64S temp64s;
   Uint32T remainder;

   /* Since the Div_U64S_U32 function checks for division by 0 and division
      by 1, just call the base routines. */
   temp64s = Mult_U32_U32(n1, n2);
   temp64s = Div_U64S_U32(temp64s, d1, &remainder);

   /* Update the ROUND flag (if provided). */
   if (round)
   {
      *round = ((remainder > (d1/2)) ? 1 : 0);
   }

   /* Update the OVERFLOW value (if provided). */
   if (overflow)
   {
      *overflow = temp64s.hi;
   }

   /* Return the lower 32-bit value */
   return temp64s.lo;
}  /* END Mult_Mult_Div_U32 */

/**

  Function Name:
   RatioConvert_U32

  Details:
   Used to implement the following ratio formula:


        n1     n2                n1 * d2
       ---- = ----   >>>   n2 = ---------
        d1     d2                  d1


   Proves useful when (n1 * d2) is larger than a Uint32T value.
   Often used to convert tick counts from one frequency to another.

  Parameters:
       n1    Numerator of the first ratio
       d1    Denominator of the first ratio
       d2    Denominator of the second ratio

  Return Value:
   n2    Numerator of the second ratio

*******************************************************************************/
Uint32T RatioConvert_U32(Uint32T n1, Uint32T d1, Uint32T d2)
{
   Uint64S temp64s;
   Uint8T round;

   temp64s.lo = Mult_Mult_Div_U32(n1, d2, d1, &round, &(temp64s.hi));

   /* Check if the above operation returned an error */
   if (temp64s.hi != (Uint32T)(-1))
   {
      /* Round off */
      temp64s.lo += round;
   }

   return temp64s.lo;
}  /* END RatioConvert_U32 */

/**

  Function Name:
   Ratio_U32_U32

  Details:
   Used to determine the ratio of 2 Uint32T numbers and express them as a 32-bit
   fraction. Any whole portion is also available:


        n1         n2                     n1:0
       ---- = -------------   >>>   n2 = -------
        d1     0x100000000                 d1


   Proves useful when finding the ratio of 2 frequencies to create a frequency
   conversion ratio.

  Parameters:
       n1    Numerator (the first value)
       d1    Denominator (the second value)
       whole32  any whole amount of num/denom

  Return Value:
   Uint32T  ratio of num/denom as a 32-bit fraction

 Remarks
   This function is used to determine ratios between clock frequencies or to
   calculate clock periods (ie. ratio between a frequency and 1GHz) so we do not
   expect it to be called very often (except at start up or protocol re-config).
   Thus, the U64S Division function is used with very little performance hit.

*******************************************************************************/
Uint32T Ratio_U32_U32(Uint32T n1, Uint32T d1, Uint32T *whole32)
{
   Uint64S temp64s;

   /* Since the Div_U64S_U32 function checks for division by 0 and division
      by 1, just call the base routines. */
   temp64s.hi = n1;
   temp64s.lo = d1 / 2; /* for eventual rounding */

   /* Use U64S division as little as possible but needed here */
   /* Ignor the remainder since we already have sub-nanosecond accuracy */
   temp64s = Div_U64S_U32(temp64s, d1, (void *)(0));

   /* Return the whole portion if required */
   if (whole32)
   {
      *whole32 = temp64s.hi;
   }

   return temp64s.lo;
}  /* END Ratio_U32_U32 */

/**

  Function Name:
   FractToNanos

  Details:
   Converts a Uint32T value representing a fraction of 0x100000000 to an
   equivalent Uint32T value representing a fraction of 1Billion (nanoseconds).

   Or,
           F               N
      -----------  =  -------------
      0x100000000     1,000,000,000

  Parameters:
       fract    fractional numerator as a part of 0x100000000 (given by F above)

  Return Value:
   Uint32T  fractional numerator as a part of 1Billion (given by N above)

*******************************************************************************/

Uint32T FractToNanos(Uint32T fract)
{
   Uint64S result;

   /* F * 1Billion */
   result = Mult_U32_U32(fract, ONE_BILLION);

   /* Dividing by 0x100000000 is equal to just grabbing result.hi */
   /* Round up the return value for better precision */
   return (result.hi + (result.lo >> 31));
}  /* END FractToNanos */

/**

  Function Name:
   NanosToFract

  Details:
   Convert a Uint32T value representing a fraction of 1Billion (nanoseconds)
   to an equivalent Uint32T value representing a fraction of 0x100000000.

   Or,


           F               N
      -----------  =  -------------
      0x100000000     1,000,000,000


  Parameters:
       nanos    fractional numerator as a part of 1Billion (given by N above)
       seconds  whole part of the conversion if nanos >= 1Billion
            If seconds == NULL, this portion is not returned.

  Return Value:
   Uint32T  fractional numerator as a part of 0x100000000 (given by F above)

  Notes:
   This function could have been implemented with the U64S Division function.
   However, since we expect the function to be call quite often (potentially for
   every Rx and Tx timestamp conversion) we want to make it as fast as possible.

*******************************************************************************/

Uint32T NanosToFract(Uint32T nanos, Uint32T *seconds)
{
   Uint32f32S result;

   /* We expect seconds to be 0 unless nanos >= 1Billion */
   if (seconds)
   {
      *seconds = nanos/ONE_BILLION;
   }

   /* Just get the actual nano second portion */
   nanos %= ONE_BILLION;

   /* Multiply by Ratio_2Exp32_1Billion in 2 steps:
      Fractional...  */
   /* Since Ratio_2Exp32_1Billion.lo is 32-bit fraction and nanos is Uint32T,
      result is U32F32 with HI = the whole part and lo = the fractional part. */
   result = Mult_U32_U32(nanos, Ratio_2Exp32_1Billion.lo);

   /* ... Add the rest of the Whole portion */
   result.hi += nanos * Ratio_2Exp32_1Billion.hi;

   /* Do not bother rounding the fractional portion since we already have a
      whole portion that is accurate into the sub-nanosecond range */

   /* the hi part is the whole fractional part */
   return result.hi;
}  /* END NanosToFract */

/**

  Function Name:
   ClockPeriod_ScaledNs

  Details:
   Used to determine one clock period of the specified frequency in nanoseconds
   (and fractional nanoseconds) expressed as a ScaledNs32T type.


 Essentially the same as the ratio:

        1,000,000,000                  remainder
       ---------------- =  nano(16) : -----------(16)
            freq                      0x100000000


   but can be computed easier when the input frequency is a multiple of the
   8KHz base frequency.

  Parameters:
       freqHz      Frequency of the target clock in Hz

  Return Value:
   ScaledNs32T    Period of the clock expressed as 16-bit nanoseconds: 16-bit
                  fractional nanoseconds.

  Notes:
   Assumes the input frequency is not less than 16KHz (but does not check),
   otherwise the result will overflow.

*******************************************************************************/

ScaledNs32T ClockPeriod_ScaledNs(Uint32T freqHz)
{
   Uint32T result;

   if (freqHz % FREQ_MULTIPLE)   /* Frequency is not a multiple of 8KHz */
   {
      /* If freqHz = 0, Ratio_U32_U32 will return all 1s so do not check here */
      Uint32T fracNs = Ratio_U32_U32(ONE_BILLION, freqHz, &(result));

      /* Convert this to ScaledNs */
      result <<= SCALED_NS32_FRACT_BITS;
      result |= (fracNs >> SCALED_NS32_WHOLE_BITS);
   }
   else if (freqHz != 0)         /* Frequency is a multiple of 8KHz */
   {
      result = (Shifted_8KHzPeriod_ScaledNs / (freqHz/FREQ_MULTIPLE)) << 1;
   }
   else
   {
      result = (Uint32T)0xFFFFFFFF;
   }

   return (ScaledNs32T)result;
}  /* END ClockPeriod_ScaledNs */

/**

  Function Name:
   TicksToNano_ScaledNsPeriod

  Details:
   Routine to calculate the number of nanoseconds in the tick count of a clock
   with the given period.

  Parameters:
       ticks       Number of ticks of the target clock
       period      Period if the target clock expressed as ScaledNs32T.
       fNano       Any partial nanoseconds from the tick count expressed as a 32-bit
               fraction.
               If fNano == NULL, this portion is not returned.

  Return Value:
   Uint64S     The number of whole nanoseconds in the tick count.

*******************************************************************************/

Uint64S TicksToNano_ScaledNsPeriod(Uint32T ticks, ScaledNs32T period, Uint32T *fNano)
{
   /* The result of the following multiplication is U48F16 type */
   Uint64S result64s = Mult_U32_U32(ticks, period);

   /* The fraction portion is in the lower 16 bits so shift it left to express
   it as a 32-bit fraction. */
   if (fNano)
   {
      *fNano = result64s.lo << SCALED_NS32_WHOLE_BITS;
   }

   /* The whole nanosecond portion is in the upper 48 bits so shift it right to
      express it as a U64S type. */
   RSHIFT_LESS32_U64S(result64s, SCALED_NS32_FRACT_BITS);

   return result64s;
}  /* END TicksToNano_ScaledNsPeriod */

/**

  Function Name:
   TicksToNano_ClkFreqHz

  Details:
   Routine to calculate the number of nanoseconds in the tick count of a clock
   with the given frequency.

  Parameters:
       ticks       Number of ticks of the target clock
       freqHz      Frequency if the target clock in Hz
       fNano       Any partial nanoseconds expressed as a 32-bit fraction.
               If fNano == NULL, this portion is not returned.

  Return Value:
   Uint64S     The number of nanoseconds in the tick count of the target clock.

*******************************************************************************/

Uint64S TicksToNano_ClkFreqHz(Uint32T ticks, Uint32T freqHz, Uint32T *fNano)
{
   /* First determine the clock period as a ScaledNs32T type. */
   ScaledNs32T clkPeriod = ClockPeriod_ScaledNs(freqHz);

   /* The result of the following multiplication is U64S type */
   Uint64S result64s = TicksToNano_ScaledNsPeriod(ticks, clkPeriod, fNano);

   /* All shifting, checking has been done for us. */
   return result64s;
}  /* END TicksToNano_ClkFreqHz */

/*****************   END   ****************************************************/

