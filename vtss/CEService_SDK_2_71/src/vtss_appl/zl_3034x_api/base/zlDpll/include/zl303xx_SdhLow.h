

/*******************************************************************************
*
*  $Id: zl303xx_SdhLow.h 6058 2011-06-10 14:37:16Z AW $
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     Definitions for low-level SDH attribute access
*
*******************************************************************************/

#ifndef ZL303XX_API_SDH_LOW_H_
#define ZL303XX_API_SDH_LOW_H_

#ifdef __cplusplus
extern "C" {
#endif

/*****************   INCLUDE FILES   ******************************************/

#include "zl303xx_ApiLow.h"
#include "zl303xx_ApiLowDataTypes.h"

/*****************   DEFINES   ************************************************/

/***************/

/* Output Control Register and Bitfield Attribute Definitions */
#define ZL303XX_SDH_OUTPUT_CTRL_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x50, ZL303XX_MEM_SIZE_2_BYTE)

/* Clk Enable Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_SDH_CLK_ENABLE_MASK                  (Uint32T)(0x01)
#define ZL303XX_SDH_CLK_ENABLE_SHIFT(clockId)        (clockId)

/* Fp Enable Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_SDH_FP_ENABLE_MASK                   (Uint32T)(0x01)
#define ZL303XX_SDH_FP_ENABLE_SHIFT(fpId)            ((fpId) + 2)

/* Dpll Source Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_SDH_IN_SOURCE_MASK                   (Uint32T)(0x01)
#define ZL303XX_SDH_IN_SOURCE_SHIFT                  6

/* PLL Enable Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_SDH_ENABLE_MASK                      (Uint32T)(0x01)
#define ZL303XX_SDH_ENABLE_SHIFT                     7

/* Clk Run Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_SDH_CLK_RUN_MASK                     (Uint32T)(0x01)
#define ZL303XX_SDH_CLK_RUN_SHIFT(clockId)           ((clockId) + 8)

/* Fp Run Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_SDH_FP_RUN_MASK                      (Uint32T)(0x01)
#define ZL303XX_SDH_FP_RUN_SHIFT(fpId)               ((fpId) + 10)

/* Clk Mode Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_SDH_CLK_ETH_DIV_MASK                 (Uint32T)(0x01)
#define ZL303XX_SDH_CLK_ETH_DIV_SHIFT(clockId)       ((clockId) + 12)

/* Center Freq Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_SDH_FREQ_CONV_MASK                   (Uint32T)(0x01)
#define ZL303XX_SDH_FREQ_CONV_SHIFT                  14

/***************/

/* Clk Divide Register and Bitfield Attribute Definitions */
#define ZL303XX_SDH_CLK_DIVIDE_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x52, ZL303XX_MEM_SIZE_1_BYTE)

/* Clk Freq Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_SDH_CLK_FREQ_MASK                    (Uint32T)(0x0F)
#define ZL303XX_SDH_CLK_FREQ_SHIFT(clockId)          ((clockId) * 4)

/***************/

/* Clk Offset Register and Bitfield Attribute Definitions */
#define ZL303XX_SDH_CLK_OFFSET0_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x53, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_SDH_CLK_OFFSET1_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x54, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_SDH_CLK_OFFSET_REG(clockId)   \
                    (Uint32T)((clockId < ZL303XX_CLOCK_ID_1) ? \
                              (ZL303XX_SDH_CLK_OFFSET0_REG) : \
                              (ZL303XX_SDH_CLK_OFFSET1_REG))


/* Clk Offset Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_SDH_CLK_OFFSET_MASK                  (Uint32T)(0x03)
#define ZL303XX_SDH_CLK_OFFSET_SHIFT                 0

/***************/

/* Fine Tune Delay Register and Bitfield Attribute Definitions */
#define ZL303XX_SDH_FINE_DELAY_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x55, ZL303XX_MEM_SIZE_1_BYTE)

/* Fine Tune Delay Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_SDH_FINE_DELAY_MASK                  (Uint32T)(0xFF)
#define ZL303XX_SDH_FINE_DELAY_SHIFT                 0

/***************/

/* Fp Freq Register and Bitfield Attribute Definitions */
#define ZL303XX_SDH_FP_FREQ0_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x56, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_SDH_FP_FREQ1_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x5B, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_SDH_FP_FREQ_REG(fpId)   \
                    (Uint32T)((fpId < ZL303XX_FP_ID_1) ? \
                              (ZL303XX_SDH_FP_FREQ0_REG) : \
                              (ZL303XX_SDH_FP_FREQ1_REG))


/* Fp Freq Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_SDH_FP_FREQ_MASK                     (Uint32T)(0x07)
#define ZL303XX_SDH_FP_FREQ_SHIFT                    0

/***************/

/* Fp Type Register and Bitfield Attribute Definitions */
#define ZL303XX_SDH_FP_TYPE0_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x57, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_SDH_FP_TYPE1_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x5C, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_SDH_FP_TYPE_REG(fpId)   \
                    (Uint32T)((fpId < ZL303XX_FP_ID_1) ? \
                              (ZL303XX_SDH_FP_TYPE0_REG) : \
                              (ZL303XX_SDH_FP_TYPE1_REG))


/* Fp Style Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_SDH_FP_STYLE_MASK                    (Uint32T)(0x01)
#define ZL303XX_SDH_FP_STYLE_SHIFT                   0

/* Fp Sync Edge Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_SDH_FP_SYNC_EDGE_MASK                (Uint32T)(0x01)
#define ZL303XX_SDH_FP_SYNC_EDGE_SHIFT               1

/* Fp Type Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_SDH_FP_TYPE_MASK                     (Uint32T)(0x07)
#define ZL303XX_SDH_FP_TYPE_SHIFT                    4

/* Fp Polarity Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_SDH_FP_POLARITY_MASK                 (Uint32T)(0x01)
#define ZL303XX_SDH_FP_POLARITY_SHIFT                7

/***************/

/* Fp Fine Offset Register and Bitfield Attribute Definitions */
#define ZL303XX_SDH_FP_FINE_OFFSET0_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x58, ZL303XX_MEM_SIZE_2_BYTE)
#define ZL303XX_SDH_FP_FINE_OFFSET1_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x5D, ZL303XX_MEM_SIZE_2_BYTE)
#define ZL303XX_SDH_FP_FINE_OFFSET_REG(fpId)   \
                    (Uint32T)((fpId < ZL303XX_FP_ID_1) ? \
                              (ZL303XX_SDH_FP_FINE_OFFSET0_REG) : \
                              (ZL303XX_SDH_FP_FINE_OFFSET1_REG))


/* Fp Fine Offset Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_SDH_FP_FINE_OFFSET_MASK              (Uint32T)(0xFFFF)
#define ZL303XX_SDH_FP_FINE_OFFSET_SHIFT             0

#define ZL303XX_SDH_FP_FINE_OFFSET_MAX   ZL303XX_SDH_FP_FINE_OFFSET_MASK

#define ZL303XX_CHECK_FP_FINE_OFFSET(val)  \
            ((val > ZL303XX_SDH_FP_FINE_OFFSET_MAX) ?  \
            ZL303XX_PARAMETER_INVALID :  \
            ZL303XX_OK)

/***************/

/* Fp Coarse Offset Register and Bitfield Attribute Definitions */
#define ZL303XX_SDH_FP_COARSE_OFFSET0_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x5A, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_SDH_FP_COARSE_OFFSET1_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x5F, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_SDH_FP_COARSE_OFFSET_REG(fpId)   \
                    (Uint32T)((fpId < ZL303XX_FP_ID_1) ? \
                              (ZL303XX_SDH_FP_COARSE_OFFSET0_REG) : \
                              (ZL303XX_SDH_FP_COARSE_OFFSET1_REG))


/* Fp Coarse Offset Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_SDH_FP_COARSE_OFFSET_MASK            (Uint32T)(0x3F)
#define ZL303XX_SDH_FP_COARSE_OFFSET_SHIFT           0

#define ZL303XX_SDH_FP_COARSE_OFFSET_MAX   ZL303XX_SDH_FP_COARSE_OFFSET_MASK

#define ZL303XX_CHECK_FP_COARSE_OFFSET(val)  \
            ((val > ZL303XX_SDH_FP_COARSE_OFFSET_MAX) ?  \
            ZL303XX_PARAMETER_INVALID :  \
            ZL303XX_OK)


/*****************   DATA TYPES   *********************************************/

/*****************   DATA STRUCTURES   ****************************************/

/*****************   EXTERNAL FUNCTION DECLARATIONS   *************************/

/* Function Declarations for the zl303xx_SdhClkConfigS type */
zlStatusE zl303xx_SdhClkConfigStructInit(zl303xx_ParamsS *zl303xx_Params,
                                         zl303xx_SdhClkConfigS *par);
zlStatusE zl303xx_SdhClkConfigCheck(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_SdhClkConfigS *par);
zlStatusE zl303xx_SdhClkConfigGet(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_SdhClkConfigS *par);
zlStatusE zl303xx_SdhClkConfigSet(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_SdhClkConfigS *par);

/* Function Declarations for the zl303xx_SdhFpConfigS type */
zlStatusE zl303xx_SdhFpConfigStructInit(zl303xx_ParamsS *zl303xx_Params,
                                        zl303xx_SdhFpConfigS *par);
zlStatusE zl303xx_SdhFpConfigCheck(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_SdhFpConfigS *par);
zlStatusE zl303xx_SdhFpConfigGet(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_SdhFpConfigS *par);
zlStatusE zl303xx_SdhFpConfigSet(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_SdhFpConfigS *par);

/* Function Declarations for the zl303xx_SdhConfigS type */
zlStatusE zl303xx_SdhConfigStructInit(zl303xx_ParamsS *zl303xx_Params,
                                      zl303xx_SdhConfigS *par);
zlStatusE zl303xx_SdhConfigCheck(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_SdhConfigS *par);
zlStatusE zl303xx_SdhConfigGet(zl303xx_ParamsS *zl303xx_Params,
                               zl303xx_SdhConfigS *par);
zlStatusE zl303xx_SdhConfigSet(zl303xx_ParamsS *zl303xx_Params,
                               zl303xx_SdhConfigS *par);

/* Clk Enable Access */
zlStatusE zl303xx_SdhClkEnableGet(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_ClockIdE clockId,
                                  zl303xx_OutEnableE *val);
zlStatusE zl303xx_SdhClkEnableSet(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_ClockIdE clockId,
                                  zl303xx_OutEnableE val);

/* Fp Enable Access */
zlStatusE zl303xx_SdhFpEnableGet(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_FpIdE fpId,
                                 zl303xx_OutEnableE *val);
zlStatusE zl303xx_SdhFpEnableSet(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_FpIdE fpId,
                                 zl303xx_OutEnableE val);

/* Dpll Source Access */
zlStatusE zl303xx_SdhInSourceGet(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_DpllIdE *val);
zlStatusE zl303xx_SdhInSourceSet(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_DpllIdE val);

/* PLL Enable Access */
zlStatusE zl303xx_SdhEnableGet(zl303xx_ParamsS *zl303xx_Params,
                               zl303xx_BooleanE *val);
zlStatusE zl303xx_SdhEnableSet(zl303xx_ParamsS *zl303xx_Params,
                               zl303xx_BooleanE val);

/* Clk Run Access */
zlStatusE zl303xx_SdhClkRunGet(zl303xx_ParamsS *zl303xx_Params,
                               zl303xx_ClockIdE clockId,
                               zl303xx_OutputRunE *val);
zlStatusE zl303xx_SdhClkRunSet(zl303xx_ParamsS *zl303xx_Params,
                               zl303xx_ClockIdE clockId,
                               zl303xx_OutputRunE val);

/* Fp Run Access */
zlStatusE zl303xx_SdhFpRunGet(zl303xx_ParamsS *zl303xx_Params,
                              zl303xx_FpIdE fpId,
                              zl303xx_OutputRunE *val);
zlStatusE zl303xx_SdhFpRunSet(zl303xx_ParamsS *zl303xx_Params,
                              zl303xx_FpIdE fpId,
                              zl303xx_OutputRunE val);

/* Clk Mode Access */
zlStatusE zl303xx_SdhClkEthDivGet(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_ClockIdE clockId,
                                  zl303xx_EthDivE *val);
zlStatusE zl303xx_SdhClkEthDivSet(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_ClockIdE clockId,
                                  zl303xx_EthDivE val);

/* Center Freq Access */
zlStatusE zl303xx_SdhFreqConvGet(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_SdhCtrFreqE *val);
zlStatusE zl303xx_SdhFreqConvSet(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_SdhCtrFreqE val);

/* Clk Freq Access */
zlStatusE zl303xx_SdhClkFreqGet(zl303xx_ParamsS *zl303xx_Params,
                                zl303xx_ClockIdE clockId,
                                zl303xx_SdhClkFreqE *val);
zlStatusE zl303xx_SdhClkFreqSet(zl303xx_ParamsS *zl303xx_Params,
                                zl303xx_ClockIdE clockId,
                                zl303xx_SdhClkFreqE val);

/* Clk Offset Access */
zlStatusE zl303xx_SdhClkOffsetGet(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_ClockIdE clockId,
                                  zl303xx_ClkOffsetE *val);
zlStatusE zl303xx_SdhClkOffsetSet(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_ClockIdE clockId,
                                  zl303xx_ClkOffsetE val);

/* Fine Tune Delay Access */
zlStatusE zl303xx_SdhFineDelayGet(zl303xx_ParamsS *zl303xx_Params,
                                  Uint32T *val);
zlStatusE zl303xx_SdhFineDelaySet(zl303xx_ParamsS *zl303xx_Params, Uint32T val);

/* Fp Freq Access */
zlStatusE zl303xx_SdhFpFreqGet(zl303xx_ParamsS *zl303xx_Params,
                               zl303xx_FpIdE fpId, zl303xx_FpFreqE *val);
zlStatusE zl303xx_SdhFpFreqSet(zl303xx_ParamsS *zl303xx_Params,
                               zl303xx_FpIdE fpId, zl303xx_FpFreqE val);

/* Fp Style Access */
zlStatusE zl303xx_SdhFpStyleGet(zl303xx_ParamsS *zl303xx_Params,
                                zl303xx_FpIdE fpId,
                                zl303xx_FpStyleE *val);
zlStatusE zl303xx_SdhFpStyleSet(zl303xx_ParamsS *zl303xx_Params,
                                zl303xx_FpIdE fpId,
                                zl303xx_FpStyleE val);

/* Fp Sync Edge Access */
zlStatusE zl303xx_SdhFpSyncEdgeGet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_FpIdE fpId,
                                   zl303xx_FpSyncEdgeE *val);
zlStatusE zl303xx_SdhFpSyncEdgeSet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_FpIdE fpId,
                                   zl303xx_FpSyncEdgeE val);

/* Fp Type Access */
zlStatusE zl303xx_SdhFpTypeGet(zl303xx_ParamsS *zl303xx_Params,
                               zl303xx_FpIdE fpId,
                               zl303xx_FpTypeSdhE *val);
zlStatusE zl303xx_SdhFpTypeSet(zl303xx_ParamsS *zl303xx_Params,
                               zl303xx_FpIdE fpId,
                               zl303xx_FpTypeSdhE val);

/* Fp Polarity Access */
zlStatusE zl303xx_SdhFpPolarityGet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_FpIdE fpId,
                                   zl303xx_FpPolarityE *val);
zlStatusE zl303xx_SdhFpPolaritySet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_FpIdE fpId,
                                   zl303xx_FpPolarityE val);

/* Fp Fine Offset Access */
zlStatusE zl303xx_SdhFpFineOffsetGet(zl303xx_ParamsS *zl303xx_Params,
                                     zl303xx_FpIdE fpId, Uint32T *val);
zlStatusE zl303xx_SdhFpFineOffsetSet(zl303xx_ParamsS *zl303xx_Params,
                                     zl303xx_FpIdE fpId, Uint32T val);

/* Fp Coarse Offset Access */
zlStatusE zl303xx_SdhFpCoarseOffsetGet(zl303xx_ParamsS *zl303xx_Params,
                                       zl303xx_FpIdE fpId, Uint32T *val);
zlStatusE zl303xx_SdhFpCoarseOffsetSet(zl303xx_ParamsS *zl303xx_Params,
                                       zl303xx_FpIdE fpId, Uint32T val);

#ifdef __cplusplus
}
#endif

#endif /* MULTIPLE INCLUDE BARRIER */

