

/*******************************************************************************
*
*  $Id: zl303xx_Utils.h 8226 2012-05-15 18:48:25Z PC $
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     Various utility functions
*
*******************************************************************************/

#ifndef ZL303XX_UTILS_H_
#define ZL303XX_UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif

/*****************   INCLUDE FILES   ******************************************/
#include "zl303xx_Global.h"
#include "zl303xx.h"

/*****************   DEFINES   ************************************************/

/*****************   DATA TYPES   *********************************************/

/* counters for sanity checking */
typedef struct {
   Uint32T sic;
} zl303xxtaskSanityCountersS;

/*****************   DATA STRUCTURES   ****************************************/

/*****************   EXPORTED GLOBAL VARIABLE DECLARATIONS   ******************/

/*****************   EXTERNAL FUNCTION DECLARATIONS   *************************/

zlStatusE zl303xx_GetTaskSanityCounters(zl303xx_ParamsS *zl303xx_Params, Uint32T s,
                                        Uint32T a, zl303xxtaskSanityCountersS *tsc);

#ifdef __cplusplus
}
#endif

#endif /* MULTIPLE INCLUDE BARRIER */

