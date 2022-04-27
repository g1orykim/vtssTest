

/*******************************************************************************
*
*  $Id: zl303xx_Dco.h 6058 2011-06-10 14:37:16Z AW $
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     Control functions for PLL
*
*
*******************************************************************************/

#ifndef ZL303XX_DCO_H_
#define ZL303XX_DCO_H_

#ifdef __cplusplus
extern "C" {
#endif

/*****************   INCLUDE FILES   ******************************************/

#include "zl303xx_Global.h"
#include "zl303xx.h"

/*****************   DEFINES   ************************************************/

/*****************   DATA TYPES   *********************************************/

/*****************   DATA STRUCTURES   ****************************************/

/*****************   EXPORTED GLOBAL VARIABLE DECLARATIONS   ******************/

/*****************   EXTERNAL FUNCTION DECLARATIONS   *************************/

/* DCO related functions */
zlStatusE zl303xx_DcoSetFreq(void *hwParams, Sint32T freqOffsetUppm);
zlStatusE zl303xx_DcoGetFreq(void *hwParams, Sint32T *freqOffsetUppm);

#ifdef __cplusplus
}
#endif

#endif /* MULTIPLE INCLUDE BARRIER */

