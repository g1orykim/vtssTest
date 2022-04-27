

/*******************************************************************************
*
*  $Id: zl303xx_TodMgrTypes.h 7734 2012-02-27 20:40:39Z SW $
*
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     This file contains software structures and data types for the Time-of-Day
*     management task. It should be included by:
*     - zl303xx_TodMgrApi.h  (for use in the prototype definitions).
*     - zl303xx_TodMgr.c     (for type and structure usage).
*     - zl303xx.h           (so that the zl303xx_ParamsS structure can be defined
*                             with the required TodMgr members).
*
*******************************************************************************/

#ifndef ZL303XX_TOD_MGR_TYPES_H_
#define ZL303XX_TOD_MGR_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

/*****************   INCLUDE FILES   ******************************************/

#include "zl303xx_Global.h"

/*****************   DEFINES   ************************************************/

/*****************   DATA TYPES   *********************************************/

/* ENUM to indicate how the internal 1PPS (aka. Timestamp 1Hz) is aligned to
 * the internal device SYNC pulses (aka. DCO sync). */
typedef enum
{
   ZL303XX_TOD_ALIGN_TO_ALL = 0x00,   /* Aligns 1PPS with the DCO 1Hz */
   ZL303XX_TOD_ALIGN_TO_2K = 0x40,    /* Aligns 1PPS with the DCO 2KHz pulses */
   ZL303XX_TOD_ALIGN_TO_8K = 0x80,    /* Aligns 1PPS with the DCO 8KHz pulses */
   ZL303XX_TOD_ALIGN_TO_NONE = 0xC0   /* No alignment to any DCO pulses */
} zl303xx_TodAlignmentE;

/* ENUM to indicate whether the calling command wants to wait for a requested
 * Time Reset/Sync to finish before proceeding OR continue immediately. */
typedef enum
{
   ZL303XX_TODMGR_NO_WAIT = 0,     /* Boolean FALSE */
   ZL303XX_TODMGR_WAIT             /* Boolean TRUE */
} zl303xx_TodMgrWaitE;

/*****************   DATA STRUCTURES   ****************************************/

/* Time-of-Day Manager task initialization structure */
typedef struct
{
   Uint32T emptyParam;
} zl303xx_TodMgrInitParamsS;

/*****************   EXPORTED GLOBAL VARIABLE DECLARATIONS   ******************/

/*****************   EXTERNAL FUNCTION DECLARATIONS   *************************/

/* These are contained in zl303xx_TodMgrApi.h */

#ifdef __cplusplus
}
#endif

#endif /* MULTIPLE INCLUDE BARRIER */
