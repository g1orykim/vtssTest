

/*******************************************************************************
*
*  $Id: zl303xx_Global.h 8335 2012-06-05 20:48:44Z DP $
*
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     This file is included into every other API file and should be used for defining
*     truly global properties
*
*******************************************************************************/

#ifndef _ZL303XX_GLOBAL_H_
#define _ZL303XX_GLOBAL_H_

#ifdef __cplusplus
extern "C" {
#endif

/*****************   DEFINES     **********************************************/

#define ZL303XX_MICROSEMI_NAME  "Microsemi Semiconductor"
#define ZL303XX_ZARLINK_NAME    ZL303XX_MICROSEMI_NAME

/* A random date/time string. In a real application this would be replaced by a meaningful time
   The format is "YYYY-MM-DD hh:mm:ss" */
#define DATE_TIME_STRING "2011-11-11 11:11:11"

#ifdef _ZL303XX_ZLE30320_BOARD
    /* APLL Defines */
    #define ZL30310 1
    #define ZL30132 2
    #define BCM5481 3
    #define ZL30142 4 /* The ZL30142 has the same pinout as ZL30132 */

#endif

/*****************   COMPILE CONTROL FLAGS   **********************************/
/* The following define enables a check every time the timestamps are sampled
   to ensure they are changing and will output an error message if not. This
   facility can be disabled by undefining this value */
#define ZL303XX_CHECK_TIMESTAMPS_CHANGING  1

   #define _ZL303XX_LOCAL    static

#ifdef OS_LINUX
   #define NORETURN __attribute__ ((noreturn))
#else
   #define NORETURN
#endif

/* task settings for APR */
#ifdef OS_VXWORKS
#define ZL303XX_APR_AD_TASK_PRIORITY              (Uint32T)80
#endif
#ifdef OS_LINUX
#define ZL303XX_APR_AD_TASK_PRIORITY              (Uint32T)88
#endif
#define ZL303XX_APR_AD_TASK_STACK_SIZE            (Uint32T)20000

#ifdef OS_VXWORKS
#define ZL303XX_PF_TASK_PRIORITY              (Uint32T)80
#endif
#ifdef OS_LINUX
#define ZL303XX_PF_TASK_PRIORITY              (Uint32T)88
#endif
#define ZL303XX_PF_TASK_STACK_SIZE            (Uint32T)20000

#ifdef OS_VXWORKS
#define ZL303XX_APR_Sample_TASK_PRIORITY              (Uint32T)34
#endif
#ifdef OS_LINUX
#define ZL303XX_APR_Sample_TASK_PRIORITY              (Uint32T)98
#endif
#define ZL303XX_APR_Sample_TASK_STACK_SIZE            (Uint32T)20000

/* Define system log file location */
#ifdef OS_VXWORKS
#define LOG_FILE_NAME "/tgtsvr/"
#endif
#ifdef OS_LINUX
#define LOG_FILE_NAME "/tmp/"
#endif

/*****************   INCLUDE FILES   ******************************************/

#include "zl303xx_DataTypes.h"     /* Basic ZL datatypes */
#include "zl303xx_DataTypesEx.h"   /* Extended datatypes specific to this project */

#include "zl303xx_OsHeaders.h"

#include "zl303xx_Int64.h"         /* 64-bit datatypes */

#ifdef __cplusplus
}
#endif

#endif   /* MULTIPLE INCLUDE BARRIER */
