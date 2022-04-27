

/*******************************************************************************
*
*  $Id: zl303xx_DebugMisc.h 8254 2012-05-23 20:11:57Z PC $
*
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     This is the header file for the accessing routines in zl303xx_DebugMisc.c.
*
*******************************************************************************/

#ifndef _ZL303XX_DEBUGMISC_H
#define _ZL303XX_DEBUGMISC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "zl303xx_Global.h"
#include "zl303xx.h"

zlStatusE zl303xx_DebugPllStatus(zl303xx_ParamsS *zl303xx_Params);
zlStatusE zl303xx_DebugHwRefCfg(zl303xx_ParamsS *zl303xx_Params, Uint32T refId);
zlStatusE zl303xx_DebugHwRefStatus(zl303xx_ParamsS *zl303xx_Params, Uint32T refId);
zlStatusE zl303xx_DebugDpllStatus(zl303xx_ParamsS *zl303xx_Params, Uint32T dpllId);
zlStatusE zl303xx_DebugDpllConfig(zl303xx_ParamsS *zl303xx_Params, Uint32T dpllId);

void zl303xx_DebugApiBuildInfo(void);

#ifdef __cplusplus
}
#endif

#endif /* _ZL303XX_DEBUGMISC_H */
