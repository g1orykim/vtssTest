

/*******************************************************************************
*
*  $Id: zl303xx_DpllConfigs.h 6058 2011-06-10 14:37:16Z AW $
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     Header file for example files
*
*******************************************************************************/

#ifndef ZL303XX_DPLL_CONFIGS_H_
#define ZL303XX_DPLL_CONFIGS_H_

#ifdef __cplusplus
extern "C" {
#endif


/*****************   INCLUDE FILES   ******************************************/
#include "zl303xx_Global.h"
#include "zl303xx_ErrTrace.h"
#include "zl303xx_Trace.h"
#include "zl303xx_ApiLowDataTypes.h"

/*****************   DEFINES   ************************************************/

/*****************   DATA TYPES   *********************************************/

/* DPLL Configuration valid values */
typedef enum
{
   ZL303XX_CONFIG_GR1244_S3,
   ZL303XX_CONFIG_GR253_S3,
   ZL303XX_CONFIG_SMC,
   ZL303XX_CONFIG_G813_OPT1,
   ZL303XX_CONFIG_G813_OPT2,
   ZL303XX_CONFIG_SLAVE
} zl303xx_DpllConfigE;

/* Range limit definitions & parameter checking */
#define ZL303XX_DPLL_CONFIG_MIN   ZL303XX_CONFIG_GR1244_S3
#define ZL303XX_DPLL_CONFIG_MAX   ZL303XX_CONFIG_SLAVE

#define ZL303XX_CHECK_DPLL_CONFIG(val)   \
            ((val < ZL303XX_DPLL_CONFIG_MIN) || (val > ZL303XX_DPLL_CONFIG_MAX) ?  \
            ZL303XX_PARAMETER_INVALID : \
            ZL303XX_OK)

/*****************   DATA STRUCTURES   ****************************************/

/* This structure is used to Init the PLL at startup */
typedef struct
{
   zl303xx_BooleanE TopClientMode; /* TRUE if timing extracted from packets */
} zl303xx_PllInitS;

/*****************   EXPORTED GLOBAL VARIABLE DECLARATIONS   ******************/

/*****************   EXTERNAL FUNCTION DECLARATIONS   *************************/

zlStatusE zl303xx_PllInitStructInit(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_PllInitS *par);

zlStatusE zl303xx_PllInit(zl303xx_ParamsS *zl303xx_Params,
                          zl303xx_PllInitS *pllInit);

zlStatusE zl303xx_PllConfigSet(zl303xx_ParamsS *zl303xx_Params,
                             zl303xx_DpllConfigE pllConfig);

zlStatusE zl303xx_DpllConfigDefaults(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_DpllConfigS *pDpllConfig,
                                   zl303xx_BooleanE TopClientMode);

#include "zl303xx_Init.h"

#ifdef __cplusplus
}
#endif

#endif /* MULTIPLE INCLUDE BARRIER */
