

/*******************************************************************************
*
*  $Id: zl303xx_TodMgrInternal.h 8776 2012-08-03 17:59:54Z PC $
*
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     This file contains non-exported structures and data types for the
*     Time-of-Day management task. It should only be included by the
*     zl303xx_TodMgr.c module.
*
*******************************************************************************/

#ifndef ZL303XX_TOD_MGR_INTERNAL_H_
#define ZL303XX_TOD_MGR_INTERNAL_H_

#ifdef __cplusplus
extern "C" {
#endif

/*****************   INCLUDE FILES   ******************************************/

#include "zl303xx_Global.h"
#include "zl303xx_AddressMap.h"

/*****************   DEFINES   ************************************************/

/* Estimated SPI Latency for a request to complete. */
#define _SPI_RD_WR_LATENCY    100000000

/* Mask for the TOD Alignment type (zl303xx_TodAlignmentE) defined in the
 * exported zl303xx_TodMgrTypes.h module.  */
#define ZL303XX_TOD_ALIGN_MASK     (Uint32T)0xC0

/* MACRO to verify the Alignment Type */
#define ZL303XX_CHECK_ALIGN_TYPE(align) \
               (((align) & ~ZL303XX_TOD_ALIGN_MASK) \
                     ? (ZL303XX_PARAMETER_INVALID) \
                     : (ZL303XX_OK))

/* The TOD Manager Registers. */
/* These are duplicates of other definitions but they are redefined here
 * in order to reduce the number of cross-includes. */
#define ZL303XX_TODMGR_CTRL_REG       ZL303XX_MAKE_MEM_ADDR(0x0A, 0x72, ZL303XX_MEM_SIZE_1_BYTE, ZL303XX_MEM_OVRLY_NONE)
#define ZL303XX_TODMGR_SCHED_REG      ZL303XX_MAKE_MEM_ADDR(0x0A, 0x6D, ZL303XX_MEM_SIZE_4_BYTE, ZL303XX_MEM_OVRLY_NONE)
#define ZL303XX_TODMGR_RESET_TIME_REG ZL303XX_MAKE_MEM_ADDR(0x0A, 0x76, ZL303XX_MEM_SIZE_8_BYTE, ZL303XX_MEM_OVRLY_NONE)

/* Internal 1Hz to INSERT timestamp correction value */
/* When the time-of-day is synchronized to the internal 1PPS, there is roughly
 * 37.5 nSec (3 SysClock cycles) of delay between the 1PPS edge and when the
 * hardware interrupt is generated. To correct this, the following value should
 * be added to the nanosecond portion of the TOD reset value when triggering
 * the TOD on the 1PPS signal. Scheduled resets on a SYSTEM time do not have
 * this delay so no correction is required.*/
#define ZL303XX_TODMGR_1PPS_LATCH_DELAY_NS  (Uint32T)38

/* Offset between insert time stamp and output 1Hz when using 2kHz or 8kHz
 * frame pulse alignment */
#define ZL303XX_TODMGR_8K_ALIGN_OFFSET_NS   200

/* Timeout periods for the 1PPS and TOD reset events (in mSec). */
#define ZL303XX_TODMGR_PULSE_SYNC_TIMEOUT   (Uint32T)1500 /* Should never have
                                 to wait more than 1 second for a 1Hz pulse. */
#define ZL303XX_TODMGR_SCHED_SYNC_TIMEOUT   (Uint32T)2500 /* Since we schedule
                                 1 second in the future wait 1 second longer. */

/****** Time-of-Day Auto Re-Alignment Feature. ******/
/* Definitions of the Minimum and Maximum Auto Re-Alignment Period. */
#define ZL303XX_TOD_AUTO_REALIGN_PERIOD_MIN    10    /* 10 Seconds */
#define ZL303XX_TOD_AUTO_REALIGN_PERIOD_MAX    300   /* 5  Minutes */

#define ZL303XX_TOD_AUTO_REALIGN_RETRY_MAX     80    /* Since we can get 76 samples per second max,
                                                      set this to the worse case. */

/*****************   DATA TYPES   *********************************************/

/* Time-of-Day Manager Input Message Types */
typedef enum
{
   ZL303XX_TODMGR_MSG_NONE = 0,

   ZL303XX_TODMGR_MSG_UPDATE_TIME,       /* Set a new Time of Day value. */
   ZL303XX_TODMGR_MSG_UPDATE_COMPLETE,   /* Update Complete: Used as a flag to the
                                          task so that it can flush any new UPDATE messages
                                          while the original UPDATE was in progress. */
   ZL303XX_TODMGR_MSG_SET_TIME,          /* ToD adjustment that only affects the seconds portion. */
   ZL303XX_TODMGR_MSG_TOD_SAMPLE,        /* A definite ToD point is known (sample or timestamp). */
   ZL303XX_TODMGR_MSG_SHUTDOWN,          /* Shutdown the task and delete queues, etc. */

   ZL303XX_TODMGR_MSG_NUM_TYPES
} zl303xx_TodMgrMsgTypeE;

/* Time-of-Day Manager Status (Overall status). */
/* Since it takes some time for any one update to complete, it is possible that
 * more requests will be received during the WAIT periods. All of these requests
 * will be discarded. */
typedef enum
{
   ZL303XX_TODMGR_STATE_UNINITIALISED = 0,  /* Task is not yet started. */
   ZL303XX_TODMGR_STATE_READY,              /* Ready for the next update. */
   ZL303XX_TODMGR_STATE_UPD_RX,             /* Received a new update request. */
   ZL303XX_TODMGR_STATE_1PPS_WAIT,          /* Waiting for the Internal 1PPS reset to complete. */
   ZL303XX_TODMGR_STATE_1PPS_COMP,          /* Internal 1PPS reset is complete. */
   ZL303XX_TODMGR_STATE_TOD_WAIT,           /* Waiting for the Time-of-Day reset to complete. */
   ZL303XX_TODMGR_STATE_TOD_COMP,           /* Time-of-Day reset is complete. */
   ZL303XX_TODMGR_STATE_COMP                /* Operation is finished. */
} zl303xx_TodMgrStateE;

/* Time-of-Day Update Progress Status of a device */
/* Since it takes a variety of actions and typically multiple stages (internal
 * 1PPS alignment and ToD reset) for a ToD update to complete, maintain the
 * progress of the local device. */
typedef enum
{
   ZL303XX_TOD_UPDATE_PROGRESS_IDLE = 0,       /* No Update is Active. */
   ZL303XX_TOD_UPDATE_PROGRESS_1PPS_DELAY,     /* The 1PPS realign point is determined but wait before triggering it.
                                              * (Typically because the next Edge is too close). */
   ZL303XX_TOD_UPDATE_PROGRESS_1PPS_TRIGGERED, /* The 1PPS realign trigger is triggered. */
   ZL303XX_TOD_UPDATE_PROGRESS_1PPS_COMPLETED, /* The 1PPS realign trigger has completed. */
   ZL303XX_TOD_UPDATE_PROGRESS_TOD_DELAY,      /* The ToD reset point is determined but wait before triggering it. */
   ZL303XX_TOD_UPDATE_PROGRESS_TOD_TRIGGERED,  /* The TOD reset trigger is set and awaiting an Edge. */
   ZL303XX_TOD_UPDATE_PROGRESS_TOD_COMPLETED,  /* The TOD reset is completed. */
   ZL303XX_TOD_UPDATE_PROGRESS_COMPLETED       /* The device TOD update is completed. */
} zl303xx_TodUpdateProgressE;

/* ENUM to indicate how to SYNC the TOD (insertTs). */
typedef enum
{
   ZL303XX_TODMGR_TOD_SYNC_NONE    = 0x00,  /* No SYNC action. */
   ZL303XX_TODMGR_TOD_SYNC_TO_1PPS = 0x04,  /* Align to the 1PPS. */
   ZL303XX_TODMGR_TOD_SYNC_TO_SYS  = 0x08,  /* SYNC at a specific System time. */
   ZL303XX_TODMGR_TOD_SYNC_MASK    = 0x0C   /* Mask for various TOD operations. */
} zl303xx_TodMgrTodSyncE;

/* ENUM to indicate how to SYNC the 1PPS (TsEng 1Hz). */
typedef enum
{
   ZL303XX_TODMGR_1PPS_SYNC_NONE   = 0x00,  /* No SYNC action. */
   ZL303XX_TODMGR_1PPS_SYNC_TO_EXT = 0x10,  /* Align to the input SYNC. */
   ZL303XX_TODMGR_1PPS_SYNC_TO_SYS = 0x20,  /* SYNC at a specific System time. */
   ZL303XX_TODMGR_1PPS_SYNC_MASK   = 0x30   /* Mask for various 1PPS operations. */
} zl303xx_TodMgr1ppsSyncE;

/* ENUM and defines to manage the overall Time-of-Day update strategy. */
/* Bit-0 = RESET(0) OR SYNC(1)
 * Bit-1&2 = EXTERNAL PULSE(b00), INTERNAL PULSE(b01),  STSTEM TS(b10)*/
typedef enum
{
   ZL303XX_TODMGR_TIME_RESET_ON_EXT_FP   = (Uint32T)(0x00),
   ZL303XX_TODMGR_TIME_SYNC_TO_EXT_FP    = (Uint32T)(0x01),   /* Handled via Re-align SM. */
   ZL303XX_TODMGR_TIME_RESET_ON_INT_FP   = (Uint32T)(0x02),
   ZL303XX_TODMGR_TIME_SYNC_TO_INT_FP    = (Uint32T)(0x03),
   ZL303XX_TODMGR_TIME_RESET_AT_SYS_TS   = (Uint32T)(0x04)
} zl303xx_TodMgrTimeUpdateE;

/*****************   DATA STRUCTURES (Overall ToD Manager Message Types)   ****/

/* Time-of-Day Manager Time Update structure */
typedef struct
{
   Uint32T type;
   Uint32T sysTs;
   Uint16T resetEpoch;
   Uint64S resetTime;
} zl303xx_TodMgrUpdateTimeS;

/* Time-of-Day Manager Time Update Complete structure */
typedef struct
{
   zlStatusE status;
   Uint16T resetEpoch;
   Uint64S resetTime;
   Uint32T resetSysTime;
} zl303xx_TodMgrUpdateCompleteS;

/* ToD Manager setTime() structure. */
typedef struct zl303xx_TodMgrSetTimeS
{
   Uint16T deltaEpoch;
   Uint64S deltaTime;
   zl303xx_BooleanE negative;
} zl303xx_TodMgrSetTimeS;

/* Time-of-Day Manager Time Sample structure */
typedef struct zl303xx_TodMgrTimeSampleS
{
   Uint32T sys;
   Uint64S tod;
   Uint64S dco;
} zl303xx_TodMgrTimeSampleS;

/* Length of the Message buffer member (MAX is the length of the largest one). */
/* Easier to manage with this macros. */
#define MAX_SIZE(A,B)                        ((A) > (B) ? (A) : (B))
#define ZL303XX_TODMGR_MSG_BUFFER_LEN_MAX                     \
               MAX_SIZE(MAX_SIZE(MAX_SIZE(                  \
                     sizeof(zl303xx_TodMgrUpdateTimeS),       \
                     sizeof(zl303xx_TodMgrUpdateCompleteS)),  \
                     sizeof(zl303xx_TodMgrSetTimeS)),         \
                     sizeof(zl303xx_TodMgrTimeSampleS))

/* Time-of-Day Manager task message structure */
typedef struct
{
   zl303xx_TodMgrMsgTypeE type;
   void *device;
   Uint8T bufLen;
   Uint8T buffer[ZL303XX_TODMGR_MSG_BUFFER_LEN_MAX];
} zl303xx_TodMgrMsgS;

/*****************   DATA STRUCTURES (Device ToD Manager)   *******************/

/* Structure for controlling the behaviour of the Time-of-Day Operations of a
 * device. This includes the configuration as well as any dynamic values
 * associated with a ToD action in progress. */
typedef struct zl303xx_TodMgrS
{
   /* TODO: Do we need a Sema4 to block operations at a per-device level? */
   OS_MUTEX_ID todSemId;

   /* COnfiguration of the internal 1PPS (whether it is aligned with the DCO
    * generated frame pulses or free floating). */
   zl303xx_TodAlignmentE onePpsAlignment;

   /* Configuration of the 1Hz Auto re-alignment functionality. */
   struct
   {
      zl303xx_BooleanE enabled;       /* Enabled or not */
      Uint32T        periodSec;     /* Time between updates (in seconds). */
   } autoRealign;

   /* Dynamic parameters for any Active Time-of-Day Updates. */
   struct
   {
      zl303xx_TodUpdateProgressE progress;     /* Active or not; and current progress.  */

      /* Samples related to the ToD Update progress. These are used to mark the
       * progress of the update since various triggers may require sample information
       * to verify the status. Also used for post update statistics.   */
      struct
      {
         zl303xx_TodMgrTimeSampleS pre1ppsRealign;
         zl303xx_TodMgrTimeSampleS post1ppsRealign;
         zl303xx_TodMgrTimeSampleS preTodReset;
         zl303xx_TodMgrTimeSampleS postTodReset;
      } sample;

      /* Save the value of the previous Time-of-Day Reset. */
      struct
      {
         zlStatusE status;    /* Status of the last Update. */
         Uint64S   todTime;   /* The last value of the reset ToD. */
         Uint32T   sysTime;   /* A rough estimate of the System Time at the ToD reset.
                               * If more than 53 seconds have passed, this is invalid. */
      } lastUpdate;

      /* Calculate any step in the case of a reset. */
      struct
      {
         Uint16T epoch;          /* Delta in epoch. */
         Uint32T seconds;        /* Delta in Seconds. */
         Uint32T nanoseconds;    /* Delta in nanoSeconds. */
      } step;

      /* Dynamic variables used to track the re-aligning of the internal 1pps or ToD. */
      Uint64S todAtTrigger;   /* Estimated ToD of when the re-alignment should complete. */
      Uint32T sysAtTrigger;   /* Estimated System count of the 1pps re-alignment. */
      Uint32T triggerCount;   /* Tracks the number of retries for triggering a hardware action. */
   } todUpdate;

   /* Last known reset values. These are valid only if there is a ToD update in
    * progress and which triggers are enabled. */
   Uint16T epoch;    /* TODO: Update these. */
   Uint32T sysTimeResetValue;
   Uint64S todResetValue;

   zl303xx_BooleanE insertTsOffsetEn;
   Sint32T insertTsOffsetNs;

   /* TODO: Follow the zl303xx_TodMgrInitParamsS structure to initialize:
    *       - Set the bInitialized flag.
    *       - Create any Semaphore.
    *       - Read the hardware registers to get the current configuration & store in software.
    */
} zl303xx_TodMgrS;

/*****************   EXTERNAL FUNCTION DECLARATIONS   *************************/

/* Internal routine used to Notify the TodMgr that a SYSTEM/TOD pair is known. */
zlStatusE zl303xx_TodTimeSampleNotification(
      void *zl303xx_Params,
      zl303xx_TodMgrTimeSampleS *timeSample);

#ifdef __cplusplus
}
#endif

#endif /* MULTIPLE INCLUDE BARRIER */
