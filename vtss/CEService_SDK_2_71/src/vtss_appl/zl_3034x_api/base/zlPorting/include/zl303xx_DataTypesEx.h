

/*******************************************************************************
*
*  $Id: zl303xx_DataTypesEx.h 6058 2011-06-10 14:37:16Z AW $
*
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     Extensions to the basic ZL datatypes for other common types
*
*******************************************************************************/

#ifndef _ZL303XX_DATATYPES_EX_H_
#define _ZL303XX_DATATYPES_EX_H_

#ifdef __cplusplus
extern "C" {
#endif

/*****************   INCLUDE FILES   ******************************************/
#include "zl303xx_ErrTrace.h"

/*****************   DATA TYPES   *********************************************/

typedef enum
{
   ZL303XX_FALSE = 0,
   ZL303XX_TRUE = 1
} zl303xx_BooleanE;

/* Macro to check for a valid boolean variable. */
/* Use of the comma operator is intentional here so that ZL303XX_ERROR_TRAP is
   called prior to assigning the return code. */
#define ZL303XX_CHECK_BOOLEAN(b) \
   ( (((b) != ZL303XX_FALSE) && ((b) != ZL303XX_TRUE)) \
      ?  (  ZL303XX_ERROR_NOTIFY("Invalid boolean value: " #b),   \
            ZL303XX_PARAMETER_INVALID)  \
      :  (  ZL303XX_OK)  \
   )

/* Macro to convert a boolean to a 0/1 integer value. Casting a boolean as
   another type should have the same affect. */
#define ZL303XX_BOOL_TO_INT(b)  \
   (((b) == ZL303XX_FALSE) ? (0) : (1))

/* Same as ZL303XX_BOOL_TO_INT on systems where system TRUE = 1 & FALSE = 0.
   Converts the system logic result to 1 for TRUE and 0 for FALSE. */
#define ZL303XX_SYS_COMPARE_TO_INT(b)  \
   ((b) ? (1) : (0))

/* Similiar to ZL303XX_SYS_COMPARE_TO_INT except this converts the system logic
   result to ZL303XX_TRUE for TRUE and ZL303XX_FALSE for FALSE. */
#define ZL303XX_SYS_COMPARE_TO_BOOL(b)  \
   ((b) ? (ZL303XX_TRUE) : (ZL303XX_FALSE))

/* Macro to convert an integer to a boolean value
   Any non-zero value is assigned TRUE */
#define ZL303XX_INT_TO_BOOL(b)   \
   (((b) == 0) ? (ZL303XX_FALSE) : (ZL303XX_TRUE))

/* Macro to invert (NOT) a boolean value.
   TRUE returns FALSE; FALSE returns TRUE */
#define ZL303XX_BOOL_NOT(b)   \
   (((b) == ZL303XX_TRUE) ? (ZL303XX_FALSE) : (ZL303XX_TRUE))

/* Macro to convert a zl303xx_BooleanE to a (const char *) for use in logging. */
#define ZL303XX_BOOL_TO_STR(b)   \
   (((b) == ZL303XX_TRUE) ? ("TRUE") : ("FALSE"))

#ifdef __cplusplus
}
#endif

#endif   /* MULTIPLE INCLUDE BARRIER */
