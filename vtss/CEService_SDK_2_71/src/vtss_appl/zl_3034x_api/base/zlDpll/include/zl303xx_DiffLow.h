

/*******************************************************************************
*
*  $Id: zl303xx_DiffLow.h 6058 2011-06-10 14:37:16Z AW $
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     Definitions for low-level DIFF attribute access
*
*
*******************************************************************************/

#ifndef ZL303XX_API_DIFF_LOW_H_
#define ZL303XX_API_DIFF_LOW_H_

#ifdef __cplusplus
extern "C" {
#endif

/*****************   INCLUDE FILES   ******************************************/

#include "zl303xx_ApiLow.h"
#include "zl303xx_ApiLowDataTypes.h"

/*****************   DEFINES   ************************************************/

/***************/

/* Control Register and Bitfield Attribute Definitions */
#define ZL303XX_DIFF_CTRL_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x60, ZL303XX_MEM_SIZE_2_BYTE)

/* Diff Enable Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_DIFF_ENABLE_MASK                     (Uint32T)(0x01)
#define ZL303XX_DIFF_ENABLE_SHIFT(diffId)            (diffId)

/* Diff Run Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_DIFF_RUN_MASK                        (Uint32T)(0x01)
#define ZL303XX_DIFF_RUN_SHIFT(diffId)               ((diffId) + 2)

/* Diff Adjust Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_DIFF_OFFSET_MASK                     (Uint32T)(0x03)
#define ZL303XX_DIFF_OFFSET_SHIFT(diffId)            (((diffId) * 2) + 4)

/* Diff Freq Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_DIFF_FREQ_MASK                       (Uint32T)(0x07)
#define ZL303XX_DIFF_FREQ_SHIFT(diffId)              (((diffId) * 4) + 8)


/*****************   DATA TYPES   *********************************************/

/*****************   DATA STRUCTURES   ****************************************/

/*****************   EXTERNAL FUNCTION DECLARATIONS   *************************/

/* Function Declarations for the zl303xx_DiffConfigS type */
zlStatusE zl303xx_DiffConfigStructInit(zl303xx_ParamsS *zl303xx_Params,
                                       zl303xx_DiffConfigS *par);
zlStatusE zl303xx_DiffConfigCheck(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_DiffConfigS *par);
zlStatusE zl303xx_DiffConfigGet(zl303xx_ParamsS *zl303xx_Params,
                                zl303xx_DiffConfigS *par);
zlStatusE zl303xx_DiffConfigSet(zl303xx_ParamsS *zl303xx_Params,
                                zl303xx_DiffConfigS *par);

/* Diff Enable Access */
zlStatusE zl303xx_DiffEnableGet(zl303xx_ParamsS *zl303xx_Params,
                                zl303xx_DiffIdE diffId,
                                zl303xx_OutEnableE *val);
zlStatusE zl303xx_DiffEnableSet(zl303xx_ParamsS *zl303xx_Params,
                                zl303xx_DiffIdE diffId,
                                zl303xx_OutEnableE val);

/* Diff Run Access */
zlStatusE zl303xx_DiffRunGet(zl303xx_ParamsS *zl303xx_Params,
                             zl303xx_DiffIdE diffId,
                             zl303xx_DiffRunE *val);
zlStatusE zl303xx_DiffRunSet(zl303xx_ParamsS *zl303xx_Params,
                             zl303xx_DiffIdE diffId,
                             zl303xx_DiffRunE val);

/* Diff Adjust Access */
zlStatusE zl303xx_DiffOffsetGet(zl303xx_ParamsS *zl303xx_Params,
                                zl303xx_DiffIdE diffId,
                                zl303xx_DiffOffsetE *val);
zlStatusE zl303xx_DiffOffsetSet(zl303xx_ParamsS *zl303xx_Params,
                                zl303xx_DiffIdE diffId,
                                zl303xx_DiffOffsetE val);

/* Diff Freq Access */
zlStatusE zl303xx_DiffFreqGet(zl303xx_ParamsS *zl303xx_Params,
                              zl303xx_DiffIdE diffId,
                              zl303xx_DiffFreqE *val);
zlStatusE zl303xx_DiffFreqSet(zl303xx_ParamsS *zl303xx_Params,
                              zl303xx_DiffIdE diffId,
                              zl303xx_DiffFreqE val);

#ifdef __cplusplus
}
#endif

#endif /* MULTIPLE INCLUDE BARRIER */

