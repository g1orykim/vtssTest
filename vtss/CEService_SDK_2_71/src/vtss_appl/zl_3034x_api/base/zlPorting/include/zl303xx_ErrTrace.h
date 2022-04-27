

/*******************************************************************************
*
*  $Id: zl303xx_ErrTrace.h 6058 2011-06-10 14:37:16Z AW $
*
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*
*     This file contains the error handling macros used by the
*     zl303xx API. The error handling is customisable, see below for
*     full description but in general the default behaviour should be satisfactory
*
*******************************************************************************/

#ifndef _ZL303XX_ERRTRACE_H
#define _ZL303XX_ERRTRACE_H

/*****************   INCLUDE FILES                *****************************/
#include "zl303xx_Global.h"

#ifdef __cplusplus
extern "C" {
#endif


/* Error handling in the ZL303XX_ API is implemented by calling the ZL303XX_ERROR_TRAP
   macro, or in rare cases, the ZL303XX_ERROR_NOTIFY macro.

   The operation of this macro depends on some other macros but its aim is to do the following:
      1. Print a meaningful message about the error, including the location where it happened if possible.
      2. Abort the process if possible.

   This is an example of how the ZL303XX_ERROR_TRAP macro is used within the API:
      if (some error condition is detected)
      {
         ZL303XX_ERROR_TRAP("Invalid parameter 1");
      }

   In detail what this does is.
      1. Set the status variable to the specified error code.
      2. Call an error trap handler function.
            Note that since this function is called on ANY fatal API error it is a useful place to set a
            debug break point
      3. The error handler function prints a meaningful message using the ZL303XX_TRACE_ERROR macro and
         if available some of the supplied location information


 */

/* Define a couple of helper macros to enable us to convert __LINE__ to a string */
#ifndef STR_1
   #define STR_1(line)   #line
#endif

#ifndef STR_2
   #define STR_2(line)   STR_1(line)
#endif

/* Define the error trap handler function to be used if no user supplied function is specified */
#ifndef ZL303XX_FATAL_ERROR_FN
   #define ZL303XX_ERROR_TRAP_FN zl303xx_ErrorTrapFn

   /* Declare the prototype for the error trap function. Any user supplied replacement
      must have the same number/type of parameters */
   void zl303xx_ErrorTrapFn(const Uint32T bStopOnError, const char *errCodeString,
                       const char *fileName, const char * const lineNum);
#endif

/* If an error handling macro has already been defined then use that, otherwise define
   one here. This allows end user overrides */
#if !defined(ZL303XX_ERROR_TRAP)

   /* __FILE__ and __LINE__ are only required in ANSI C but may exist in other compilers.
      If they exist we will use them to give more error info */
   #if defined(__FILE__) && defined(__LINE__)
      #define ZL303XX_ERROR_TRAP(errString) \
         ZL303XX_ERROR_TRAP_FN(ZL303XX_TRUE, errString, \
                           __FILE__, STR_2(__LINE__))
   #else
       #define ZL303XX_ERROR_TRAP(errString) \
         ZL303XX_ERROR_TRAP_FN(ZL303XX_TRUE, errString, \
                           0, 0)
   #endif

#endif

/* And define the milder form of error handling. This version does not stop the current
   thread if an error occurs. This should be rarely used */
#if !defined(ZL303XX_ERROR_NOTIFY)

   /* __FILE__ and __LINE__ are only required in ANSI C but may exist in other compilers.
      If they exist we will use them to give more error info */
   #if defined(__FILE__) && defined(__LINE__)
      #define ZL303XX_ERROR_NOTIFY(errString) \
         ZL303XX_ERROR_TRAP_FN(ZL303XX_FALSE, errString, \
                           __FILE__, STR_2(__LINE__))
   #else
       #define ZL303XX_ERROR_NOTIFY(errString) \
         ZL303XX_ERROR_TRAP_FN(ZL303XX_FALSE, errString, \
                           0, 0)
   #endif

#endif


#ifdef __cplusplus
}
#endif

#endif   /* MULTIPLE INCLUDE BARRIER */

