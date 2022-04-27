

/*******************************************************************************
*
*  $Id: zl303xx_Params.h 6058 2011-06-10 14:37:16Z AW $
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     Prototypes for structure initialisation and locking.
*
*******************************************************************************/

#ifndef _ZL303XX_PARAMS_H_
#define _ZL303XX_PARAMS_H_

#ifdef __cplusplus
extern "C" {
#endif

zlStatusE zl303xx_CreateDeviceInstance(zl303xx_ParamsS **zl303xx_Params);
zlStatusE zl303xx_FreeDeviceInstance(zl303xx_ParamsS **zl303xx_Params);

zlStatusE zl303xx_LockDevParams(zl303xx_ParamsS *zl303xx_Params);
zlStatusE zl303xx_UnlockDevParams(zl303xx_ParamsS *zl303xx_Params);

#ifdef __cplusplus
}
#endif

#endif


