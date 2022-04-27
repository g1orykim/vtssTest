

/*******************************************************************************
*
*  $Id: zl303xx_LogToMsgQ.h 7835 2012-03-13 16:44:32Z JK $
*
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     The definitions of the Zarlink loggin Enums
*
*******************************************************************************/

#ifndef _ZL_MSGQ_LOGGING_H_
#define _ZL_MSGQ_LOGGING_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "zl303xx_Global.h"
#include "zl303xx_Error.h"
#include "zl303xx_Os.h"
#include "zl303xx_Trace.h"
#include <fcntl.h>

#ifdef OS_VXWORKS
    #define ZL303XX_LOG_MSGQ_TASK_PRIORITY 85 /* VxWorks Lowest for ZL-related */
#else
    #define ZL303XX_LOG_MSGQ_TASK_PRIORITY 55 /* Linux Lowest for ZL-related */
#endif
#define ZL303XX_LOG_TASK_MSGQ_SIZE 64
#define ZL303XX_LOG_MSGQ_TASK_STACK_SIZE 8000

#define MAX_FMT_LEN   256
#define FMT_ARRAY_SIZE 63   /* Must be < 64 bits (BusyBufferBits) */


typedef enum
{
    UNKNOWN_APRLOG_ENUM = 0,
    LOG_FMT_STR = 1,

    A_0000 = 0002,    
    A_1000 = 1000,    
    A_1001 = 1001,
    A_1060 = 1060,
    A_1061 = 1061,
    A_1120 = 1120,
    A_1121 = 1121,
    A_1180 = 1180,
    A_1240 = 1240,
    A_1300 = 1300,
    A_1400 = 1400,
    A_1500 = 1500,
    A_1600 = 1600,
    A_1700 = 1700,
    A_1800 = 1800,
    A_1900 = 1900,
    A_2000 = 2000,
    A_3000 = 3000,
    A_3060 = 3060,
    A_3120 = 3120,
    A_3180 = 3180,
    A_3240 = 3240,
    A_3300 = 3300,
    A_3360 = 3360,
    A_3400 = 3400,
    A_3460 = 3460,
    A_3470 = 3470,
    A_3480 = 3480,
    A_3490 = 3490,
    A_3500 = 3500,
    A_3510 = 3510,
    A_3520 = 3520,
    A_3580 = 3580,
    A_3620 = 3620,
    A_4000 = 4000,
    A_5000 = 5000,

    C_0000 = 20000,
    C_0100 = 20100,
    C_1000 = 21000,
    C_1001 = 21001,
    C_2000 = 22000,
    C_3000 = 23000,
    C_4000 = 24000,
    C_5000 = 25000,

    A_90000 =90000,
    A_90010 =90010,
    A_90020 =90020,
    A_90030 =90030,
    A_90040 =90040,
    A_90050 =90050,
    A_93000 =93000,
    A_93040 =93040,
    A_93080 =93080,
    A_93120 =93120,

    LAST_LOG_ENUM

} zl303xx_LoggingE;

typedef enum
{
    APR_ENUM_TYPE,      /* Used only within APR */
    FMT_STR_TYPE        /* Formatted msg */
} zl303xxlogMsgTypeE;

typedef struct zl303xx_MsgQLogMsgS
{
    zl303xxlogMsgTypeE logMsgType;
    zl303xx_LoggingE eNumId;

    UnativeT arg0;  /* Filled if APR_ENUM_TYPE */
    UnativeT arg1;
    UnativeT arg2;
    UnativeT arg3;
    UnativeT arg4;
    UnativeT arg5;

    Uint8T *pFmtStr;    /* Pointer to Format str */
    Uint8T indx;        /* Index into fmtArray */
}zl303xx_MsgQLogMsgS;



zlStatusE zl303xx_ReEnableLogToMsgQ(void);
zlStatusE zl303xx_DisableLogToMsgQ(void);
zlStatusE zl303xx_SetupLogToMsgQ(FILE* fd);
zlStatusE zl303xx_ShutdownLogToMsgQ(FILE* fd);
Sint32T   zl303xx_LogToMsgQ(zl303xx_LoggingE eNum, const char *str, UnativeT arg0, UnativeT arg1, 
                         UnativeT arg2, UnativeT arg3, UnativeT arg4, UnativeT arg5);
#ifdef __cplusplus
}
#endif

#endif   /* MULTIPLE INCLUDE BARRIER */

