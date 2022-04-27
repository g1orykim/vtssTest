

/*******************************************************************************
*
*  $Id: zl303xx_TsEng.h 6058 2011-06-10 14:37:16Z AW $
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     Control functions for device timestamp engine
*
*******************************************************************************/

#ifndef ZL303XX_TS_ENG_H_
#define ZL303XX_TS_ENG_H_

#ifdef __cplusplus
extern "C" {
#endif

/*****************   INCLUDE FILES   ******************************************/

#include "zl303xx_Global.h"

/*****************   DEFINES   ************************************************/

#define ZL303XX_HZ_PER_KHZ   1000
#define ZL303XX_HZ_PER_MHZ   1000000
#define ZL303XX_HZ_PER_GHZ   1000000000
#define ZL303XX_HZ_PER_4GHZ  (Uint32T)(ZL303XX_HZ_PER_GHZ << 2)

#define ZL303XX_TIMESTAMP_SIZE_32  4  /* For RTP protocol */
#define ZL303XX_TIMESTAMP_SIZE_64  8  /* For PTP/NTP protocols */

/* Interrupt Status/Mask Register bit definitions
   (ZL303XX_TSENG_ISR and ZL303XX_TSENG_ISR_MASK_REG control) */
#define ZL303XX_TSENG_ISR_DCO_UPDATE_BIT       (Uint8T)0x01
#define ZL303XX_TSENG_ISR_TS_SAMPLED_BIT       (Uint8T)0x02
#define ZL303XX_TSENG_ISR_TOD_UPDATE_BIT       (Uint8T)0x04
#define ZL303XX_TSENG_ISR_Q_FULL_BIT           (Uint8T)0x08
#define ZL303XX_TSENG_ISR_Q_EMPTY_BIT          (Uint8T)0x10

#define ZL303XX_TSENG_ISR_MASK_ALL    \
               (Uint8T)(ZL303XX_TSENG_ISR_DCO_UPDATE_BIT | \
                        ZL303XX_TSENG_ISR_TS_SAMPLED_BIT | \
                        ZL303XX_TSENG_ISR_TOD_UPDATE_BIT | \
                        ZL303XX_TSENG_ISR_Q_FULL_BIT | \
                        ZL303XX_TSENG_ISR_Q_EMPTY_BIT)

/*****************   DATA TYPES   *********************************************/

/* Enum for Output Clock to 1Hz alignment */
typedef enum
{
   ZL303XX_ALIGN_NONE,
   ZL303XX_ALIGN_RISE_CLK_TO_RISE_PPS,
   ZL303XX_ALIGN_FALL_CLK_TO_RISE_PPS,
   ZL303XX_ALIGN_RISE_CLK_TO_FALL_PPS,
   ZL303XX_ALIGN_FALL_CLK_TO_FALL_PPS,
   ZL303XX_ALIGN_RISE_CLK_TO_TOD_PPS,
   ZL303XX_ALIGN_FALL_CLK_TO_TOD_PPS
} zl303xx_ClockToPpsAlignmentTypeE;

/* Timestamp Engine event types */
typedef enum
{
   /* Base entry for range checking */
   TSENG_EVT_MIN = 0,

   /* Core definitions */
   TSENG_EVT_DCO_UPDATE = TSENG_EVT_MIN,
   TSENG_EVT_SAMPLE_READY,
   TSENG_EVT_TOD_UPDATE,
   TSENG_EVT_EXIT_Q_FULL,
   TSENG_EVT_EXIT_Q_EMPTY,

   /* Command to shutdown the TsEng task (and clean-up queues, etc.) */
   TSENG_EVT_SHUTDOWN,

   /* Added entries for error/range checking */
   TSENG_EVT_NUM_EVENTS,
   TSENG_EVT_INVALID = -1
} zl303xx_TsEngEventTypeE;

/*****************   DATA STRUCTURES   ****************************************/

typedef struct
{
   zl303xx_TsEngTsProtocolE  tsProtocol;
   Uint16T                 insertFreqKHz;
   Uint16T                 intervalVal;
} zl303xx_TsEngineInitS;

/********************************/

typedef struct
{
   Uint32T dummy;    /* Placeholder */
} zl303xx_TsEngineCloseS;

/********************************/
/* A structure for passing raw sampled data from the interrupt handler to the
   processing task. All necessary sample values are collected within the handler
   routine and sent to the registered processing task. */
typedef struct
{
   /* Device timestamps */
   Uint64S dcoTs;          /* Raw DCO timestamp (Ticks:Phase) */
   Uint32T sysTs;          /* Raw SYSTEM timestamp (Ticks) */
   Uint64S insertTs;       /* Raw INSERT timestamp (Protocol dependent) */

   /* DCO offset value */
   Sint32T freqOffsetUppm; /* Current freq offset for the device in uHz/MHz */

   /* CPU Timestamps */
   Uint32T cpuTs;          /* Raw CPU clock timestamp (Ticks; if equipped) */
   Uint32T cpuTicks;       /* Raw CPU tick count (60 Ticks/sec clock) */

   /* Last exit timestamp packet index sent in this interval. */
   Uint8T  pktIndex;
} zl303xx_TsEngRawSampleS;

/* For now, only the sample event sends additional message data. If other events
   require message data, the MAX_EVENT_BUFFER_LEN may have to be changed to the
   size of the biggest message. */
#define ZL303XX_TSENG_MAX_MSG_DATA_LEN      sizeof(zl303xx_TsEngRawSampleS)

/********************************/
/* A structure for passing TsEngine event messages to the handler task */
typedef struct
{
   zl303xx_TsEngEventTypeE eventId;
   void *device;
   Uint8T bufLen;
   Uint8T buffer[ZL303XX_TSENG_MAX_MSG_DATA_LEN];
} zl303xx_TsEngEventS;

/* Define a callback function pointer for Timestamp Engine events. It is up to
 * the user function to cast the tsEngData and hwParams structures properly. */
typedef void (*zl303xx_TsEngEvtFnT)(zl303xx_TsEngEventS *tsEngEvent);

/********************************/
/* Structure for exitTs pairs (timestamp + index). */
typedef struct
{
   Uint32T index;
   Uint32T timestamp;
} zl303xx_TsEngExitTsPairS;

/*****************   EXPORTED GLOBAL VARIABLE DECLARATIONS   ******************/

/*****************   EXTERNAL FUNCTION DECLARATIONS   *************************/

/* Timestamp Engine State Control Routines */
zlStatusE zl303xx_TsEngineInitStructInit(zl303xx_ParamsS *zl303xx_Params,
                                         zl303xx_TsEngineInitS *par);

zlStatusE zl303xx_TsEngineInit(zl303xx_ParamsS *zl303xx_Params,
                               zl303xx_TsEngineInitS *par);

zlStatusE zl303xx_TsEngineCloseStructInit(zl303xx_ParamsS *zl303xx_Params,
                                          zl303xx_TsEngineCloseS *par);
zlStatusE zl303xx_TsEngineClose(zl303xx_ParamsS *zl303xx_Params,
                                zl303xx_TsEngineCloseS *par);

/* Timestamp Engine Configuration Routines */
zlStatusE zl303xx_TsEngSetTsSampleMode(zl303xx_ParamsS *zl303xx_Params,
                                       zl303xx_UpdateTypeE mode);

zlStatusE zl303xx_TsEngSetDcoUpdateMode(zl303xx_ParamsS *zl303xx_Params,
                                        zl303xx_UpdateTypeE mode);

zlStatusE zl303xx_TsEngSetIntervalValue(zl303xx_ParamsS *zl303xx_Params,
                                        Uint8T interval);

/* Exit Timestamp Routines */
zlStatusE zl303xx_TsEngReadExitTs(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_TsEngExitTsPairS *exitTsPair);

zlStatusE zl303xx_TsEngReadExitTsFillLevel(zl303xx_ParamsS *zl303xx_Params,
                                           Uint16T *fillLevel);

/* Timestamp Engine Interrupt Control Routines */
zlStatusE zl303xx_TsEngSetInterruptMask(zl303xx_ParamsS *zl303xx_Params,
                                        Uint8T mask);

zlStatusE zl303xx_TsEngEnableInterrupt(zl303xx_ParamsS *zl303xx_Params,
                                       Uint8T mask);

zlStatusE zl303xx_TsEngDisableInterrupt(zl303xx_ParamsS *zl303xx_Params,
                                        Uint8T mask);

zlStatusE zl303xx_TsEngSetQfullThreshold(zl303xx_ParamsS *zl303xx_Params,
                                         Uint8T level);

/* Time-of-Day Control Routines */
zlStatusE zl303xx_TsEngSetTimeOfDay(zl303xx_ParamsS *zl303xx_Params,
                                    Uint32T timeOfDay);

/* Local timestamp routine */
zlStatusE zl303xx_TsEngGetAccurateTod(zl303xx_ParamsS *zl303xx_Params,
                                      Uint64S *todTime,
                                      Uint64S *sysTime);

zlStatusE zl303xx_TsEngCurrentCoarseTime(zl303xx_ParamsS *zl303xx_Params,
                                         Uint64S *sec, Uint32T *nanoSec);

/* 1Hz Alignment & 1pps alignment routines */
zlStatusE zl303xx_AlignOutputClkTo1pps(zl303xx_ParamsS *zl303xx_Params,
                                       zl303xx_BooleanE alignPulse);

/* API internal function */
zl303xx_BooleanE zl303xx_TsEngSamplesReady(zl303xx_ParamsS *zl303xx_Params);

/*****  TIMESTAMP SAMPLE HISTORY LOOKUP ROUTINES  *******************/

zlStatusE zl303xx_TsEngSampleGetCurrent(zl303xx_ParamsS *zl303xx_Params,
                                        zl303xx_TsSampleS *sample);
zlStatusE zl303xx_TsEngSampleGetByIndex(zl303xx_ParamsS *zl303xx_Params,
                                        Uint32T sampleIndex,
                                        zl303xx_TsSampleS *sample);
zlStatusE zl303xx_TsEngSampleGetBySysTs(zl303xx_ParamsS *zl303xx_Params,
                                        Uint32T sysTs,
                                        zl303xx_TsSampleS *sample);

/*****  INTERRUPT HANDLER RELATED EXTERNAL FUNCTION DECLARATIONS  *******/

/* Routine to initialize the Timestamp Engine interrupts and services to a
   default state */
zlStatusE zl303xx_TsEngInterruptInit(zl303xx_ParamsS *zl303xx_Params);

/* Main Timestamp Engine Interrupt Routine */
zlStatusE zl303xx_TsEngInterruptHandler(zl303xx_ParamsS *zl303xx_Params);

/* Routines to allow other tasks to receive notification of TsEng events */
zlStatusE zl303xx_TsEngRegisterForEvent(zl303xx_TsEngEventTypeE eventId,
                                        zl303xx_TsEngEvtFnT callback);

zlStatusE zl303xx_TsEngUnRegisterEvent(zl303xx_TsEngEventTypeE eventId,
                                       zl303xx_TsEngEvtFnT callback);

/*****  CLOCK CONVERSION ROUTINES  *******************/
Uint64S zl303xx_TsConvAdjustSysDelta(Uint32T sysTicksDelta,
                                     Uint64S sysAdjRatio);

Uint64S zl303xx_TsConvDeltaSysToDco(Uint32T sysTicksDelta,
                                    Uint64S dcoToSysRatio,
                                    Uint64S sysAdjRatio);

/***** SYSTEM TIMESTAMP (32-bit) to another clock timestamp routines *****/
zlStatusE zl303xx_TsConvTsSysToDco(zl303xx_ParamsS *zl303xx_Params,
                                   Uint32T sysTs, Uint64S *dcoTs);

zlStatusE zl303xx_TsConvTsSysToNs(zl303xx_ParamsS *zl303xx_Params,
                                  Uint32T sysTs, Uint64S *nsTs);

Uint64S zl303xx_TsConvClkTicksToNs(Uint64S clkTicks, ScaledNs32T clkPeriod);

#ifdef __cplusplus
}
#endif

#endif /* MULTIPLE INCLUDE BARRIER */

