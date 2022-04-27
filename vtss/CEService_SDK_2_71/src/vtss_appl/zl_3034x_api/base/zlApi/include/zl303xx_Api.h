

/*******************************************************************************
*
*  $Id: zl303xx_Api.h 8172 2012-05-03 14:34:47Z PC $
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     Top level header file that includes all the other header files for the API
*
*******************************************************************************/

#ifndef ZL303XX_API_TOP_H
#define ZL303XX_API_TOP_H

/*****************   INCLUDE FILES                *****************************/

#include "zl303xx_Global.h"  /* This should always be the first file included */

/* Now include the porting library since most other components depend on it */
#include "zl303xx_Porting.h"

/* include other header files from this directory */
#include "zl303xx.h"
#include "zl303xx_Init.h"
#include "zl303xx_Spi.h"
#include "zl303xx_RdWr.h"

#include "zl303xx_ApiInterrupt.h"
#include "zl303xx_Interrupt.h"
#include "zl303xx_Dco.h"
#include "zl303xx_TsEng.h"
#include "zl303xx_Utils.h"


//#include "zl303xx_Apr.h"

/*****************   EXPORTED GLOBAL VARIABLE DECLARATIONS   ******************/
/* API description strings */
extern const char zl303xx_ApiBuildDate[];
extern const char zl303xx_ApiBuildTime[];
extern const char zl303xx_ApiReleaseDate[];
extern const char zl303xx_ApiReleaseTime[];
extern const char zl303xx_ApiReleaseVersion[];
extern const char zl303xx_ApiReleaseSwId[];

#endif   /* MULTIPLE INCLUDE BARRIER */
