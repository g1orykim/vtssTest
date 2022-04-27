

/*******************************************************************************
*
*  $Id: zl303xx_SynthLow.h 6058 2011-06-10 14:37:16Z AW $
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     Definitions for low-level SYNTH attribute access
*
*******************************************************************************/

#ifndef ZL303XX_API_SYNTH_LOW_H_
#define ZL303XX_API_SYNTH_LOW_H_

#ifdef __cplusplus
extern "C" {
#endif

/*****************   INCLUDE FILES   ******************************************/

#include "zl303xx_ApiLow.h"
#include "zl303xx_ApiLowDataTypes.h"

/*****************   DEFINES   ************************************************/

/***************/

/* Output Control Register and Bitfield Attribute Definitions */
#define ZL303XX_SYNTHP0_OUTPUT_CTRL_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x36, ZL303XX_MEM_SIZE_2_BYTE)
#define ZL303XX_SYNTHP1_OUTPUT_CTRL_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x48, ZL303XX_MEM_SIZE_2_BYTE)
#define ZL303XX_SYNTH_OUTPUT_CTRL_REG(synthId)   \
                    (Uint32T)((synthId == ZL303XX_SYNTH_ID_P0) ? \
                              (ZL303XX_SYNTHP0_OUTPUT_CTRL_REG) : \
                              (ZL303XX_SYNTHP1_OUTPUT_CTRL_REG))

/* Clk Enable Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_SYNTH_CLK_ENABLE_MASK                (Uint32T)(0x01)
#define ZL303XX_SYNTH_CLK_ENABLE_SHIFT(clockId)      (clockId)

/* Fp Enable Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_SYNTH_FP_ENABLE_MASK                 (Uint32T)(0x01)
#define ZL303XX_SYNTH_FP_ENABLE_SHIFT(fpId)          ((fpId) + 2)

/* Dpll Source Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_SYNTH_IN_SOURCE_MASK                 (Uint32T)(0x01)
#define ZL303XX_SYNTH_IN_SOURCE_SHIFT                6

/* PLL Enable Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_SYNTH_ENABLE_MASK                    (Uint32T)(0x01)
#define ZL303XX_SYNTH_ENABLE_SHIFT                   7

/* Clk Run Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_SYNTH_CLK_RUN_MASK                   (Uint32T)(0x01)
#define ZL303XX_SYNTH_CLK_RUN_SHIFT(clockId)         ((clockId) + 8)

/* Fp Run Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_SYNTH_FP_RUN_MASK                    (Uint32T)(0x01)
#define ZL303XX_SYNTH_FP_RUN_SHIFT(fpId)             ((fpId) + 10)

/***************/

/* Freq Multiple Register and Bitfield Attribute Definitions */
#define ZL303XX_SYNTHP0_CLK_MULT_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x38, ZL303XX_MEM_SIZE_2_BYTE)
#define ZL303XX_SYNTHP1_CLK_MULT_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x4A, ZL303XX_MEM_SIZE_2_BYTE)
#define ZL303XX_SYNTH_CLK_MULT_REG(synthId)   \
                    (Uint32T)((synthId == ZL303XX_SYNTH_ID_P0) ? \
                              (ZL303XX_SYNTHP0_CLK_MULT_REG) : \
                              (ZL303XX_SYNTHP1_CLK_MULT_REG))

/* Nx8kHz Mult Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_SYNTH_FREQUENCY_MULT_MASK            (Uint32T)(0x3FFF)
#define ZL303XX_SYNTH_FREQUENCY_MULT_SHIFT           0

/***************/

/* Clk Offset Register and Bitfield Attribute Definitions */
#define ZL303XX_SYNTHP0_CLK_OFFSET0_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x3A, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_SYNTHP0_CLK_OFFSET1_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x3C, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_SYNTHP0_CLK_OFFSET_REG(clockId)   \
                    (Uint32T)((clockId < ZL303XX_CLOCK_ID_1) ? \
                              (ZL303XX_SYNTHP0_CLK_OFFSET0_REG) : \
                              (ZL303XX_SYNTHP0_CLK_OFFSET1_REG))

#define ZL303XX_SYNTHP1_CLK_OFFSET0_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x4C, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_SYNTHP1_CLK_OFFSET1_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x4E, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_SYNTHP1_CLK_OFFSET_REG(clockId)   \
                    (Uint32T)((clockId < ZL303XX_CLOCK_ID_1) ? \
                              (ZL303XX_SYNTHP1_CLK_OFFSET0_REG) : \
                              (ZL303XX_SYNTHP1_CLK_OFFSET1_REG))

#define ZL303XX_SYNTH_CLK_OFFSET_REG(synthId,clockId)   \
                    (Uint32T)((synthId == ZL303XX_SYNTH_ID_P0) ? \
                              (ZL303XX_SYNTHP0_CLK_OFFSET_REG(clockId)) : \
                              (ZL303XX_SYNTHP1_CLK_OFFSET_REG(clockId)))

/* Clk Offset Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_SYNTH_CLK_OFFSET_MASK                (Uint32T)(0x03)
#define ZL303XX_SYNTH_CLK_OFFSET_SHIFT               0

/***************/

#define ZL303XX_SYNTHP0_CLK_DIV_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x3B, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_SYNTHP1_CLK_DIV_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x4D, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_SYNTH_CLK_DIV_REG(synthId)   \
                    (Uint32T)((synthId == ZL303XX_SYNTH_ID_P0) ? \
                              (ZL303XX_SYNTHP0_CLK_DIV_REG) : \
                              (ZL303XX_SYNTHP1_CLK_DIV_REG))
#define ZL303XX_SYNTH_CLK_FREQ_REG(synthId,clockId)   \
                    (Uint32T)((clockId == ZL303XX_CLOCK_ID_0) ? \
                              (ZL303XX_SYNTH_CLK_MULT_REG(synthId)) : \
                              (ZL303XX_SYNTH_CLK_DIV_REG(synthId)))

/* Clk Divider Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_SYNTH_FREQUENCY_DIV_MASK             (Uint32T)(0x3F)
#define ZL303XX_SYNTH_FREQUENCY_DIV_SHIFT            0

/***************/

/* Fine Tune Delay Register and Bitfield Attribute Definitions */
#define ZL303XX_SYNTHP0_FINE_DELAY_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x3D, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_SYNTHP1_FINE_DELAY_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x4F, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_SYNTH_FINE_DELAY_REG(synthId)   \
                    (Uint32T)((synthId == ZL303XX_SYNTH_ID_P0) ? \
                              (ZL303XX_SYNTHP0_FINE_DELAY_REG) : \
                              (ZL303XX_SYNTHP1_FINE_DELAY_REG))

/* Fine Tune Delay Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_SYNTH_FINE_DELAY_MASK                (Uint32T)(0xFF)
#define ZL303XX_SYNTH_FINE_DELAY_SHIFT               0

/***************/

/* Fp Freq Register and Bitfield Attribute Definitions */
#define ZL303XX_SYNTHP0_FP_FREQ0_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x3E, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_SYNTHP0_FP_FREQ1_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x43, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_SYNTHP0_FP_FREQ_REG(fpId)   \
                    (Uint32T)((fpId < ZL303XX_FP_ID_1) ? \
                              (ZL303XX_SYNTHP0_FP_FREQ0_REG) : \
                              (ZL303XX_SYNTHP0_FP_FREQ1_REG))


/* Fp Freq Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_SYNTH_FP_FREQ_MASK                   (Uint32T)(0x0F)
#define ZL303XX_SYNTH_FP_FREQ_SHIFT                  0

/***************/

/* Fp Type Register and Bitfield Attribute Definitions */
#define ZL303XX_SYNTHP0_FP_TYPE0_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x3F, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_SYNTHP0_FP_TYPE1_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x44, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_SYNTHP0_FP_TYPE_REG(fpId)   \
                    (Uint32T)((fpId < ZL303XX_FP_ID_1) ? \
                              (ZL303XX_SYNTHP0_FP_TYPE0_REG) : \
                              (ZL303XX_SYNTHP0_FP_TYPE1_REG))


/* Fp Style Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_SYNTH_FP_STYLE_MASK                  (Uint32T)(0x01)
#define ZL303XX_SYNTH_FP_STYLE_SHIFT                 0

/* Fp Sync Edge Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_SYNTH_FP_SYNC_EDGE_MASK              (Uint32T)(0x01)
#define ZL303XX_SYNTH_FP_SYNC_EDGE_SHIFT             1

/* Fp Type Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_SYNTH_FP_TYPE_MASK                   (Uint32T)(0x07)
#define ZL303XX_SYNTH_FP_TYPE_SHIFT                  4

/* Fp Polarity Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_SYNTH_FP_POLARITY_MASK               (Uint32T)(0x01)
#define ZL303XX_SYNTH_FP_POLARITY_SHIFT              7

/***************/

/* Fp Fine Offset Register and Bitfield Attribute Definitions */
#define ZL303XX_SYNTHP0_FP_FINE_OFFSET0_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x40, ZL303XX_MEM_SIZE_2_BYTE)
#define ZL303XX_SYNTHP0_FP_FINE_OFFSET1_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x45, ZL303XX_MEM_SIZE_2_BYTE)
#define ZL303XX_SYNTHP0_FP_FINE_OFFSET_REG(fpId)   \
                    (Uint32T)((fpId < ZL303XX_FP_ID_1) ? \
                              (ZL303XX_SYNTHP0_FP_FINE_OFFSET0_REG) : \
                              (ZL303XX_SYNTHP0_FP_FINE_OFFSET1_REG))


/* Fp Fine Offset Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_SYNTH_FP_FINE_OFFSET_MASK            (Uint32T)(0xFFFF)
#define ZL303XX_SYNTH_FP_FINE_OFFSET_SHIFT           0

/***************/

/* Fp Coarse Offset Register and Bitfield Attribute Definitions */
#define ZL303XX_SYNTHP0_FP_COARSE_OFFSET0_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x42, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_SYNTHP0_FP_COARSE_OFFSET1_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x47, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_SYNTHP0_FP_COARSE_OFFSET_REG(fpId)   \
                    (Uint32T)((fpId < ZL303XX_FP_ID_1) ? \
                              (ZL303XX_SYNTHP0_FP_COARSE_OFFSET0_REG) : \
                              (ZL303XX_SYNTHP0_FP_COARSE_OFFSET1_REG))


/* Fp Coarse Offset Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_SYNTH_FP_COARSE_OFFSET_MASK          (Uint32T)(0x3F)
#define ZL303XX_SYNTH_FP_COARSE_OFFSET_SHIFT         0


/*****************   DATA TYPES   *********************************************/

/*****************   DATA STRUCTURES   ****************************************/

/*****************   EXTERNAL FUNCTION DECLARATIONS   *************************/

/* Function Declarations for the zl303xx_SynthClkConfigS type */
zlStatusE zl303xx_SynthClkConfigStructInit(zl303xx_ParamsS *zl303xx_Params,
                                           zl303xx_SynthClkConfigS *par);
zlStatusE zl303xx_SynthClkConfigCheck(zl303xx_ParamsS *zl303xx_Params,
                                      zl303xx_SynthClkConfigS *par);
zlStatusE zl303xx_SynthClkConfigGet(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_SynthClkConfigS *par);
zlStatusE zl303xx_SynthClkConfigSet(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_SynthClkConfigS *par);

/* Function Declarations for the zl303xx_SynthFpConfigS type */
zlStatusE zl303xx_SynthFpConfigStructInit(zl303xx_ParamsS *zl303xx_Params,
                                          zl303xx_SynthFpConfigS *par);
zlStatusE zl303xx_SynthFpConfigCheck(zl303xx_ParamsS *zl303xx_Params,
                                     zl303xx_SynthFpConfigS *par);
zlStatusE zl303xx_SynthFpConfigGet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_SynthFpConfigS *par);
zlStatusE zl303xx_SynthFpConfigSet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_SynthFpConfigS *par);

/* Function Declarations for the zl303xx_SynthConfigS type */
zlStatusE zl303xx_SynthConfigStructInit(zl303xx_ParamsS *zl303xx_Params,
                                        zl303xx_SynthConfigS *par);
zlStatusE zl303xx_SynthConfigCheck(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_SynthConfigS *par);
zlStatusE zl303xx_SynthConfigGet(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_SynthConfigS *par);
zlStatusE zl303xx_SynthConfigSet(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_SynthConfigS *par);

/* Clk Enable Access */
zlStatusE zl303xx_SynthClkEnableGet(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_SynthIdE synthId,
                                    zl303xx_ClockIdE clockId,
                                    zl303xx_OutEnableE *val);
zlStatusE zl303xx_SynthClkEnableSet(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_SynthIdE synthId,
                                    zl303xx_ClockIdE clockId,
                                    zl303xx_OutEnableE val);

/* Fp Enable Access */
zlStatusE zl303xx_SynthFpEnableGet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_SynthIdE synthId,
                                   zl303xx_FpIdE fpId,
                                   zl303xx_OutEnableE *val);
zlStatusE zl303xx_SynthFpEnableSet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_SynthIdE synthId,
                                   zl303xx_FpIdE fpId,
                                   zl303xx_OutEnableE val);

/* Dpll Source Access */
zlStatusE zl303xx_SynthInSourceGet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_SynthIdE synthId,
                                   zl303xx_DpllIdE *val);
zlStatusE zl303xx_SynthInSourceSet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_SynthIdE synthId,
                                   zl303xx_DpllIdE val);

/* PLL Enable Access */
zlStatusE zl303xx_SynthEnableGet(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_SynthIdE synthId,
                                 zl303xx_BooleanE *val);
zlStatusE zl303xx_SynthEnableSet(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_SynthIdE synthId,
                                 zl303xx_BooleanE val);

/* Clk Run Access */
zlStatusE zl303xx_SynthClkRunGet(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_SynthIdE synthId,
                                 zl303xx_ClockIdE clockId,
                                 zl303xx_OutputRunE *val);
zlStatusE zl303xx_SynthClkRunSet(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_SynthIdE synthId,
                                 zl303xx_ClockIdE clockId,
                                 zl303xx_OutputRunE val);

/* Fp Run Access */
zlStatusE zl303xx_SynthFpRunGet(zl303xx_ParamsS *zl303xx_Params,
                                zl303xx_SynthIdE synthId,
                                zl303xx_FpIdE fpId,
                                zl303xx_OutputRunE *val);
zlStatusE zl303xx_SynthFpRunSet(zl303xx_ParamsS *zl303xx_Params,
                                zl303xx_SynthIdE synthId,
                                zl303xx_FpIdE fpId,
                                zl303xx_OutputRunE val);

/* Nx8kHz Mult Access */
zlStatusE zl303xx_SynthFrequencyMultGet(zl303xx_ParamsS *zl303xx_Params,
                                        zl303xx_SynthIdE synthId,
                                        Uint32T *val);
zlStatusE zl303xx_SynthFrequencyMultSet(zl303xx_ParamsS *zl303xx_Params,
                                        zl303xx_SynthIdE synthId,
                                        Uint32T val);

/* Clk Offset Access */
zlStatusE zl303xx_SynthClkOffsetGet(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_SynthIdE synthId,
                                    zl303xx_ClockIdE clockId,
                                    zl303xx_ClkOffsetE *val);
zlStatusE zl303xx_SynthClkOffsetSet(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_SynthIdE synthId,
                                    zl303xx_ClockIdE clockId,
                                    zl303xx_ClkOffsetE val);

/* Clk Divider Access */
zlStatusE zl303xx_SynthFrequencyDivGet(zl303xx_ParamsS *zl303xx_Params,
                                       zl303xx_SynthIdE synthId,
                                       Uint32T *val);
zlStatusE zl303xx_SynthFrequencyDivSet(zl303xx_ParamsS *zl303xx_Params,
                                       zl303xx_SynthIdE synthId,
                                       Uint32T val);

/* Fine Tune Delay Access */
zlStatusE zl303xx_SynthFineDelayGet(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_SynthIdE synthId,
                                    Uint32T *val);
zlStatusE zl303xx_SynthFineDelaySet(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_SynthIdE synthId,
                                    Uint32T val);

/* Fp Freq Access */
zlStatusE zl303xx_SynthFpFreqGet(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_SynthIdE synthId,
                                 zl303xx_FpIdE fpId,
                                 zl303xx_FpFreqE *val);
zlStatusE zl303xx_SynthFpFreqSet(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_SynthIdE synthId,
                                 zl303xx_FpIdE fpId,
                                 zl303xx_FpFreqE val);

/* Fp Style Access */
zlStatusE zl303xx_SynthFpStyleGet(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_SynthIdE synthId,
                                  zl303xx_FpIdE fpId,
                                  zl303xx_FpStyleE *val);
zlStatusE zl303xx_SynthFpStyleSet(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_SynthIdE synthId,
                                  zl303xx_FpIdE fpId,
                                  zl303xx_FpStyleE val);

/* Fp Sync Edge Access */
zlStatusE zl303xx_SynthFpSyncEdgeGet(zl303xx_ParamsS *zl303xx_Params,
                                     zl303xx_SynthIdE synthId,
                                     zl303xx_FpIdE fpId,
                                     zl303xx_FpSyncEdgeE *val);
zlStatusE zl303xx_SynthFpSyncEdgeSet(zl303xx_ParamsS *zl303xx_Params,
                                     zl303xx_SynthIdE synthId,
                                     zl303xx_FpIdE fpId,
                                     zl303xx_FpSyncEdgeE val);

/* Fp Type Access */
zlStatusE zl303xx_SynthFpTypeGet(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_SynthIdE synthId,
                                 zl303xx_FpIdE fpId,
                                 zl303xx_FpTypePxE *val);
zlStatusE zl303xx_SynthFpTypeSet(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_SynthIdE synthId,
                                 zl303xx_FpIdE fpId,
                                 zl303xx_FpTypePxE val);

/* Fp Polarity Access */
zlStatusE zl303xx_SynthFpPolarityGet(zl303xx_ParamsS *zl303xx_Params,
                                     zl303xx_SynthIdE synthId,
                                     zl303xx_FpIdE fpId,
                                     zl303xx_FpPolarityE *val);
zlStatusE zl303xx_SynthFpPolaritySet(zl303xx_ParamsS *zl303xx_Params,
                                     zl303xx_SynthIdE synthId,
                                     zl303xx_FpIdE fpId,
                                     zl303xx_FpPolarityE val);

/* Fp Fine Offset Access */
zlStatusE zl303xx_SynthFpFineOffsetGet(zl303xx_ParamsS *zl303xx_Params,
                                       zl303xx_SynthIdE synthId,
                                       zl303xx_FpIdE fpId,
                                       Uint32T *val);
zlStatusE zl303xx_SynthFpFineOffsetSet(zl303xx_ParamsS *zl303xx_Params,
                                       zl303xx_SynthIdE synthId,
                                       zl303xx_FpIdE fpId,
                                       Uint32T val);

/* Fp Coarse Offset Access */
zlStatusE zl303xx_SynthFpCoarseOffsetGet(zl303xx_ParamsS *zl303xx_Params,
                                         zl303xx_SynthIdE synthId,
                                         zl303xx_FpIdE fpId,
                                         Uint32T *val);
zlStatusE zl303xx_SynthFpCoarseOffsetSet(zl303xx_ParamsS *zl303xx_Params,
                                         zl303xx_SynthIdE synthId,
                                         zl303xx_FpIdE fpId,
                                         Uint32T val);

#ifdef __cplusplus
}
#endif

#endif /* MULTIPLE INCLUDE BARRIER */

