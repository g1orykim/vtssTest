

/*******************************************************************************
*
*  $Id: zl303xx_FbkLow.h 6058 2011-06-10 14:37:16Z AW $
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     Definitions for low-level FBK attribute access
*
*******************************************************************************/

#ifndef ZL303XX_API_FBK_LOW_H_
#define ZL303XX_API_FBK_LOW_H_

#ifdef __cplusplus
extern "C" {
#endif

/*****************   INCLUDE FILES   ******************************************/

#include "zl303xx_ApiLow.h"
#include "zl303xx_ApiLowDataTypes.h"

/*****************   DEFINES   ************************************************/

/***************/

/* Control Register and Bitfield Attribute Definitions */
#define ZL303XX_FBK_CTRL_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x62, ZL303XX_MEM_SIZE_1_BYTE)

/* Clk Enable Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_FBK_CLK_ENABLE_MASK                  (Uint32T)(0x01)
#define ZL303XX_FBK_CLK_ENABLE_SHIFT                 0

/* PLL Enable Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_FBK_PLL_ENABLE_MASK                  (Uint32T)(0x01)
#define ZL303XX_FBK_PLL_ENABLE_SHIFT                 7

/* Fbk Type Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_FBK_PIN_CONFIG_MASK                  (Uint32T)(0x01)
#define ZL303XX_FBK_PIN_CONFIG_SHIFT                 4

/***************/

/* Fine Tune Delay Register and Bitfield Attribute Definitions */
#define ZL303XX_FBK_FINE_DELAY_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x63, ZL303XX_MEM_SIZE_1_BYTE)

/* Fine Tune Delay Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_FBK_FINE_DELAY_MASK                  (Uint32T)(0xFF)
#define ZL303XX_FBK_FINE_DELAY_SHIFT                 0


/*****************   DATA TYPES   *********************************************/

/*****************   DATA STRUCTURES   ****************************************/

/*****************   EXTERNAL FUNCTION DECLARATIONS   *************************/

/* Function Declarations for the zl303xx_FeedbackConfigS type */
zlStatusE zl303xx_FeedbackConfigStructInit(zl303xx_ParamsS *zl303xx_Params,
                                           zl303xx_FeedbackConfigS *par);
zlStatusE zl303xx_FeedbackConfigCheck(zl303xx_ParamsS *zl303xx_Params,
                                      zl303xx_FeedbackConfigS *par);
zlStatusE zl303xx_FeedbackConfigGet(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_FeedbackConfigS *par);
zlStatusE zl303xx_FeedbackConfigSet(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_FeedbackConfigS *par);

/* Clk Enable Access */
zlStatusE zl303xx_FbkClkEnableGet(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_OutEnableE *val);
zlStatusE zl303xx_FbkClkEnableSet(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_OutEnableE val);

/* PLL Enable Access */
zlStatusE zl303xx_FbkPllEnableGet(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_EnableE *val);
zlStatusE zl303xx_FbkPllEnableSet(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_EnableE val);

/* Fbk Type Access */
zlStatusE zl303xx_FbkPinConfigGet(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_FbkPinStateE *val);
zlStatusE zl303xx_FbkPinConfigSet(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_FbkPinStateE val);

/* Fine Tune Delay Access */
zlStatusE zl303xx_FbkFineDelayGet(zl303xx_ParamsS *zl303xx_Params,
                                  Uint32T *val);
zlStatusE zl303xx_FbkFineDelaySet(zl303xx_ParamsS *zl303xx_Params, Uint32T val);

#ifdef __cplusplus
}
#endif

#endif /* MULTIPLE INCLUDE BARRIER */

