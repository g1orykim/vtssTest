

/*******************************************************************************
*
*  $Id: zl303xx_Var.h 7480 2012-01-23 23:01:26Z DP $
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     This file contains headers for ZL303xx variant support.
*
*******************************************************************************/

#ifndef ZL303XX_VAR_H_
#define ZL303XX_VAR_H_

#ifdef __cplusplus
extern "C" {
#endif


/*****************   INCLUDE FILES   ******************************************/
#include "zl303xx_Global.h"
#include "zl303xx_RdWr.h"

/*****************   DEFINES   ************************************************/

/*****************   DATA TYPES   *********************************************/

/*****************   DATA STRUCTURES   ****************************************/

/*****************   EXPORTED GLOBAL VARIABLE DECLARATIONS   ******************/

/*****************   EXTERNAL FUNCTION DECLARATIONS   *************************/
zlStatusE zl303xx_InitDeviceIdAndRev(zl303xx_ParamsS *zl303xx_Params);

zlStatusE zl303xx_GetDeviceId(zl303xx_ParamsS *zl303xx_Params, Uint8T *deviceId);
zlStatusE zl303xx_GetDeviceRev(zl303xx_ParamsS *zl303xx_Params, Uint8T *revision);

void zl303xx_PrintDeviceId(zl303xx_ParamsS *zl303xx_Params);
void zl303xx_PrintDeviceRev(zl303xx_ParamsS *zl303xx_Params);

/* These functions read values directly from the hardware registers rather
   than zl303xx_Params */
zlStatusE zl303xx_ReadDeviceId(zl303xx_ParamsS *zl303xx_Params, Uint8T *deviceId);
zlStatusE zl303xx_ReadDeviceRevision(zl303xx_ParamsS *zl303xx_Params, Uint8T *revision);

#ifdef __cplusplus
}
#endif

#endif /* MULTIPLE INCLUDE BARRIER */

