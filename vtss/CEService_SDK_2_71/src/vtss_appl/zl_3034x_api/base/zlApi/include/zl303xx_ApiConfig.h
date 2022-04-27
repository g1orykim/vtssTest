

/*******************************************************************************
*
*  $Id: zl303xx_ApiConfig.h 8341 2012-06-07 13:07:42Z PC $
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     Functions for setting and getting the user configurable parameters.
*
*******************************************************************************/

#ifndef ZL303XX_APICONFIG_H_
#define ZL303XX_APICONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

/*****************   INCLUDE FILES   ******************************************/
#include "zl303xx_Global.h"
#include "zl303xx_Error.h"
#include "zl303xx.h"

/*****************   DATA TYPES   *********************************************/
typedef enum
{
   ZL303XX_MODE_UNKNOWN = 0,
   ZL303XX_MODE_REF_TOP,
   ZL303XX_MODE_REF_EXTERNAL
} zl303xx_DeviceModeE;


/* System interrupt config. */
zlStatusE zl303xx_SetLog2SysInterruptPeriod(Uint32T log2Period);
Uint32T zl303xx_GetLog2SysInterruptPeriod(void);

zlStatusE zl303xx_SetDeviceMode(zl303xx_DeviceModeE deviceMode);
zl303xx_DeviceModeE zl303xx_GetDeviceMode(void);

#ifdef OS_LINUX
zlStatusE zl303xx_SetCliPriority(Uint8T taskPriority);
Uint8T zl303xx_GetCliPriority(void);
#endif  /* OS_LINUX */

#ifdef __cplusplus
}
#endif

#endif /*ZL303XX_APICONFIG_H_*/
