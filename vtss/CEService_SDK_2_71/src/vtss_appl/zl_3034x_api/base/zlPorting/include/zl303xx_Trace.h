

/*******************************************************************************
*
*  $Id: zl303xx_Trace.h 8348 2012-06-07 20:18:49Z DP $
*
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     This file contains support for Tracing that is used to output log messages
*
*******************************************************************************
*
* The key operations available are:
* 1. the trace control functions:
*
*     void zl303xx_TraceInit(Uint32T logFd);
*     void zl303xx_TraceSetLevel(Uint16T modId, Uint8T level);
*     void zl303xx_TraceSetLevelAll(Uint8T level);
*
* 2. the trace invocation macros which are used for actually generating
*    a trace message.
*     ZL303XX_TRACE(Uint32T modId, Uint32T level, const char *fmtStr, arg0, arg1, arg2,
                                                               arg3, arg4, arg5)
*           A trace which is controlled by the trace level
*     ZL303XX_TRACE_ALWAYS(const char *fmtStr, arg0, arg1, arg2, arg3, arg4, arg5)
*           A trace which is always displayed but can be stripped for release
*     ZL303XX_TRACE_ERROR(const char *fmtStr, arg0, arg1, arg2, arg3, arg4, arg5)
*           A trace which is always displayed
* These macros take 6 parameters after the format str which are formatted at
* run-time. They may be set to zero if they are not required.
* To ensure the trace message is displayed correctly the following rules
* must be followed:
*     a. The fmtStr must be a constant pointer to a constant string
*     b. the optional parameters must be either integers or constant
*        pointers to constant strings.
*
* 3. a macro to allow other debug code to be conditionally executed depending
*    on the trace level
*        ZL303XX_DEBUG_FILTER(modId, minDbgLevel)
*    Example:
*        ZL303XX_DEBUG_FILTER(THIS_MODULE_ID, 2)
*        {
*           Some conditional debug code.
*           Will only be executed if trace level for this module is 2 or higher
*        }
*
*******************************************************************************/

#ifndef ZL303XX_TRACE_H_
#define ZL303XX_TRACE_H_

#ifdef __cplusplus
extern "C" {
#endif

/*****************   INCLUDE FILES   ******************************************/

#include "zl303xx_Global.h"
#include <stdio.h> /* For FILE* definition */

/*****************   DEFINES   ************************************************/

/* define a string to be used for the newline character. Can be redefined on the
   command line to allow for support for different carriage return - linefeed
   combinations etc. */
#ifndef ZL303XX_NEWLINE
      #define ZL303XX_NEWLINE   "\r\n"
#endif

/* This macro may be defined on the command line to allow the tracing mechanism
   to be extended for use by end application tracing. It allocates additional
   space in the zl303xx_TraceFilter array */
#ifndef ZL303XX_NUM_USER_TRACE_MODULES
   #define ZL303XX_NUM_USER_TRACE_MODULES 0
#endif

/* Define the total space */
#define ZL303XX_NUM_TRACE_MODULES \
               (ZL303XX_NUM_API_MODULE_IDS + ZL303XX_NUM_USER_TRACE_MODULES)

/*****************   DATA TYPES   *********************************************/
/* Identifier for function ID's used for trace statements */
/* Values must be contiguous and incremental, the numerical value is supplied
   for convenience only */
typedef enum
{
   ZL303XX_MOD_ID_RDWR          = 0,
   ZL303XX_MOD_ID_INIT          = 1,
   ZL303XX_MOD_ID_LAN           = 2,
   ZL303XX_MOD_ID_LAN_STATS     = 3,

   /* Device specific interrupt module */
   ZL303XX_MOD_ID_INTERRUPT     = 4,

   /* System interrupt handling module */
   ZL303XX_MOD_ID_SYSINT        = 5,
   ZL303XX_MOD_ID_TS_ENG        = 6,
   ZL303XX_MOD_ID_PKT           = 7,
   ZL303XX_MOD_ID_PLL           = 8,
   ZL303XX_MOD_ID_APR           = 9,
   ZL303XX_MOD_ID_TS_PKT_STRM   = 10,
   ZL303XX_MOD_ID_TRNSPRT_LYR   = 11,
   ZL303XX_MOD_ID_PTP_TSTAMP    = 12,
   ZL303XX_MOD_ID_PKT_SCHED     = 13,

   ZL303XX_MOD_ID_PTP_ENGINE    = 14,    /* Main PTP Engine. */
   ZL303XX_MOD_ID_PTP_BMC       = 15,    /* PTP Best-Master-Clock related. */
   ZL303XX_MOD_ID_PTP_UNICAST   = 16,    /* PTP Unicast Negotiation related. */
   ZL303XX_MOD_ID_PTP_UNI_DISC  = 17,    /* PTP Unicast Discovery related. */
   ZL303XX_MOD_ID_PTP_STATE_UPD = 18,    /* PTP Clock, Port or Stream State related. */

    ZL303XX_MOD_ID_TS_RECORD_MGR = 19,
    ZL303XX_MOD_ID_SOCKET_LAYER  = 20,
    ZL303XX_MOD_ID_CLK_SWITCH    = 21,
    ZL303XX_MOD_ID_DCO_MGR       = 22,
    ZL303XX_MOD_ID_TRACK_PKT_PROCESS = 23,
    ZL303XX_MOD_ID_TOD_MGR       = 24,
    ZL303XX_MOD_ID_TSIF          = 25,
    ZL303XX_MOD_ID_MSGQ          = 26,
    ZL303XX_MOD_ID_FPE           = 27,
    ZL303XX_MOD_ID_PTP_FMT       = 28,   /* PTP Foreign Master Table related. */
    ZL303XX_MOD_ID_PTSF          = 29,
    ZL303XX_MOD_ID_NOTIFY        = 30,
    ZL303XX_MOD_ID_CLI           = 31,

   ZL303XX_NUM_API_MODULE_IDS   /* Must be the last element */
} zl303xx_ModuleIdE;

/*****************   DATA STRUCTURES   ****************************************/

/* We use a structure here to allow for future expansion */
typedef struct
{
   Uint8T level;
/*   Uint8T flags;  Currently unused */
} zl303xx_TraceFilterS;

/*****************   EXTERNAL GLOBAL VARIABLE DECLARATIONS   ******************/

extern zl303xx_TraceFilterS Zl303xx_TraceFilter[ZL303XX_NUM_TRACE_MODULES];

/*****************   EXTERNAL FUNCTION DECLARATIONS   *************************/

/* Initialise tracing. Should be called at least once during application
   initialisation.
   The parameter is a file descriptor previously opened with fopen() or one of
   the predefined values: OS_STDOUT or OS_STDERR.
   Note: in some environments, if the logging File Descriptor is not correctly
   set then any output, include error messages, will disappear completely. */
void zl303xx_TraceInit(FILE* logFd);

/* Close down tracing. Does not normally need to be called since applications
   normally only close tracing when they shutdown */
void zl303xx_TraceClose(FILE* logFd);

/* Set the tracing level for a specific module. Level = 0 disables tracing */
void zl303xx_TraceSetLevel(Uint16T modId, Uint8T level);

/* Set the tracing level for all modules. Level = 0 will disable all tracing */
void zl303xx_TraceSetLevelAll(Uint8T level);

/* The trace functions which do the work of displaying the message */
void zl303xx_TraceFnFiltered(Uint32T modId, Uint32T level, const char *str,
      UnativeT arg0, UnativeT arg1, UnativeT arg2,
      UnativeT arg3, UnativeT arg4, UnativeT arg5);

void zl303xx_TraceFnNoFilter(const char * str,
      UnativeT arg0, UnativeT arg1, UnativeT arg2,
      UnativeT arg3, UnativeT arg4, UnativeT arg5);


#if defined(_DEBUG)
   /* Define a filter to allow a block of code to be executed conditionally
      depending on the trace level */
   #define ZL303XX_DEBUG_FILTER(modId, minDbgLevel)  \
         if ((Zl303xx_TraceFilter[modId].level > 0) && \
             (Zl303xx_TraceFilter[modId].level >= minDbgLevel))

   /* Define the selective tracing macro. This is gated on module ID & level */
   #define ZL303XX_TRACE(modId, level, str, arg0, arg1, arg2, arg3, arg4, arg5) \
               zl303xx_TraceFnFiltered(modId, level, str ZL303XX_NEWLINE, \
                                       (UnativeT)(arg0), (UnativeT)(arg1), (UnativeT)(arg2), \
                                       (UnativeT)(arg3), (UnativeT)(arg4), (UnativeT)(arg5))

   /* Define the selective tracing macro. This just gates on the module ID. */
   #define ZL303XX_TRACE_MOD(modId, str, arg0, arg1, arg2, arg3, arg4, arg5) \
               zl303xx_TraceFnFiltered(modId, 1, str ZL303XX_NEWLINE, \
                                       (UnativeT)(arg0), (UnativeT)(arg1), (UnativeT)(arg2), \
                                       (UnativeT)(arg3), (UnativeT)(arg4), (UnativeT)(arg5))

   /* Define a trace macro which will always display but can be stripped at
      compile-time if required.*/
   #define ZL303XX_TRACE_ALWAYS(str, arg0, arg1, arg2, arg3, arg4, arg5) \
               zl303xx_TraceFnNoFilter(str ZL303XX_NEWLINE, \
                                       (UnativeT)(arg0), (UnativeT)(arg1), (UnativeT)(arg2), \
                                       (UnativeT)(arg3), (UnativeT)(arg4), (UnativeT)(arg5))
#else
   /* Strip the tracing from the code */
   #define ZL303XX_DEBUG_FILTER(modId, minDbgLevel)  if(0)

   #define ZL303XX_TRACE(modId, level, str, arg0, arg1, arg2, arg3, arg4, arg5)
   #define ZL303XX_TRACE_MOD(modId, str, arg0, arg1, arg2, arg3, arg4, arg5)
   #define ZL303XX_TRACE_ALWAYS(str, arg0, arg1, arg2, arg3, arg4, arg5)
#endif

/* Define the error tracing macro. This is gated on neither the module ID nor
   level so will always be sent to the logging stream providing that is enabled.
   It also cannot be stripped */
#define ZL303XX_TRACE_ERROR(str, arg0, arg1, arg2, arg3, arg4, arg5) \
           zl303xx_TraceFnNoFilter(str ZL303XX_NEWLINE, \
                                   (UnativeT)(arg0), (UnativeT)(arg1), (UnativeT)(arg2), \
                                   (UnativeT)(arg3), (UnativeT)(arg4), (UnativeT)(arg5))

#ifdef __cplusplus
}
#endif

#endif /* MULTIPLE INCLUDE BARRIER */

