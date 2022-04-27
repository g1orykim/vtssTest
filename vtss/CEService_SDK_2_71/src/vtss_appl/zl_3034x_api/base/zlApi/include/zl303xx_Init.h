

/*******************************************************************************
*
*  $Id: zl303xx_Init.h 8318 2012-06-01 20:47:33Z PC $
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     Device initialisation functions
*
*******************************************************************************/

#ifndef ZL303XX_INIT_H_
#define ZL303XX_INIT_H_

#ifdef __cplusplus
extern "C" {
#endif

/*****************   INCLUDE FILES   ******************************************/

#include "zl303xx_Global.h"
#include "zl303xx.h"
#include "zl303xx_TsEng.h"
#include "zl303xx_DpllConfigs.h"
#include "zl303xx_ApiConfig.h"
/*****************   DEFINES   ************************************************/

/*****************   DATA TYPES   *********************************************/

/*****************   DATA STRUCTURES   ****************************************/
typedef struct
{
   Uint32T sysClockFreqHz; /* The system clock rate */
   Uint32T dcoClockFreqHz; /* The DCO clock rate */
   zl303xx_TsEngineInitS tsEngInit;
   zl303xx_PllInitS pllInit;
} zl303xx_InitDeviceS;

typedef struct
{
   zl303xx_TsEngineCloseS tsEngClose;
} zl303xx_CloseDeviceS;

/*****************   EXPORTED GLOBAL VARIABLE DECLARATIONS   ******************/

/*****************   EXTERNAL FUNCTION DECLARATIONS   *************************/
zlStatusE zl303xx_InitApi(void);
zlStatusE zl303xx_CloseApi(void);

zlStatusE zl303xx_InitDeviceStructInit(zl303xx_ParamsS *zl303xx_Params, zl303xx_InitDeviceS *par, zl303xx_DeviceModeE deviceMode);
zlStatusE zl303xx_InitDevice(zl303xx_ParamsS *zl303xx_Params, zl303xx_InitDeviceS *par);

zlStatusE zl303xx_CloseDeviceStructInit(zl303xx_ParamsS *zl303xx_Params, zl303xx_CloseDeviceS *par);
zlStatusE zl303xx_CloseDevice(zl303xx_ParamsS *zl303xx_Params, zl303xx_CloseDeviceS *par);

#ifdef __cplusplus
}
#endif

#endif /* MULTIPLE INCLUDE BARRIER */

