

/*******************************************************************************
*
*  $Id: zl303xx_Int64.h 6610 2011-08-31 17:56:25Z DP $
*
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     This file contains the type definitions for 64 bit integers.
*
*******************************************************************************/

#ifdef __cplusplus
   extern "C" {
#endif

/* the compiler directive _ZL_DISABLE_64_BIT_OPERATIONS is used to disable
   native 64 bit operations. The code makes use of the device specific version
   _ZL303XX_DISABLE_64_BIT_OPERATIONS, so map from one to the other before
   getting started */
#ifdef _ZL_DISABLE_64_BIT_OPERATIONS
   #ifndef _ZL303XX_DISABLE_64_BIT_OPERATIONS

      #define _ZL303XX_DISABLE_64_BIT_OPERATIONS

   #endif
#endif

/* make some generic 64 bit type definitions if they haven't already been made */
#ifndef _ZL_INT64_DEFINED

   #define _ZL_INT64_DEFINED
      
   #include "zl303xx_Os.h"

   #ifdef _ZL303XX_DISABLE_64_BIT_OPERATIONS

      #include "gmp.h"
      #include "gmp-impl.h"

      typedef mpz_t Uint64T;
      typedef mpz_t Sint64T;

   #else

      typedef OS_UINT64 Uint64T;
      typedef OS_SINT64 Sint64T;

   #endif
#endif

/* if the device specific macros haven't been defined then do so */
#ifndef _ZL303XX_INT64_DEFINED
   #define _ZL303XX_INT64_DEFINED

   /* define types for 64 bit numbers. This is being done locally to this file,
      since it is NOT ANSI C, and its usage should be minimised. */
   #ifdef _ZL303XX_DISABLE_64_BIT_OPERATIONS

      void zl303xx_ConvertTo64(Uint64T *val, Uint64S *structVal);
      void zl303xx_ConvertTo64Signed(Sint64T *val, Uint64S *structVal);
      void zl303xx_ConvertFrom64(Uint64S *structVal, Uint64T *val);
      void zl303xx_ShiftRight32(Sint64T *val);

      #define ZL303XX_CONVERT_TO_64(val, structVal)           zl303xx_ConvertTo64(&(val), &(structVal))
      #define ZL303XX_CONVERT_TO_64_SIGNED(val, structVal)    zl303xx_ConvertTo64Signed(&(val), &(structVal))
      #define ZL303XX_CONVERT_FROM_64(structVal, val)         zl303xx_ConvertFrom64(&(structVal), &(val))

      #define ZL303XX_SHIFT_RIGHT_32(val)                     zl303xx_ShiftRight32(&(val))

   #else

      #define ZL303XX_CONVERT_TO_64(val, structVal)           ((val) = ((Uint64T)((structVal).hi) << 32) | (Uint64T)((structVal).lo))
      #define ZL303XX_CONVERT_TO_64_SIGNED(val, structVal)    ((val) = (Sint64T)(((Uint64T)((structVal).hi) << 32) | (Uint64T)((structVal).lo)))

      /* do the shift in 2 steps below, since the compiler implements the >> 32 correctly but
         produces a warning. If optimisations are on then this will default to a single 32 bit
         op which probably won't even be a shift, so just inelegant */
      #define ZL303XX_CONVERT_FROM_64(structVal, val)   (structVal).hi = (Uint32T)(((val) >> 16) >> 16); \
                                                      (structVal).lo = (Uint32T)(val);
      #define ZL303XX_SHIFT_RIGHT_32(val)               (((val) >> 16) >> 16)

   #endif
#endif

#ifdef __cplusplus
}
#endif
