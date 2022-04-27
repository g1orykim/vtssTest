

/*******************************************************************************
*
*  $Id: zl303xx_ApiLowDataTypes.h 8171 2012-05-03 13:30:16Z PC $
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     Definitions for low-level DPLL attribute access
*
*
*******************************************************************************/

#ifndef ZL303XX_API_LOW_DATA_TYPES_H_
#define ZL303XX_API_LOW_DATA_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

/*****************   INCLUDE FILES   ******************************************/
#include "zl303xx_Global.h"

/*****************   DATA TYPES / DEFINES   ***********************************/

/***************/

/* RefId valid values */
typedef enum
{
   ZL303XX_REF_ID_0 = 0,
   ZL303XX_REF_ID_1 = 1,
   ZL303XX_REF_ID_2 = 2,
   ZL303XX_REF_ID_3 = 3,
   ZL303XX_REF_ID_4 = 4,
   ZL303XX_REF_ID_5 = 5,
   ZL303XX_REF_ID_6 = 6,
   ZL303XX_REF_ID_7 = 7,
   ZL303XX_REF_ID_8 = 8
} zl303xx_RefIdE;

/* Range limit definitions & parameter checking */
#define ZL303XX_REF_ID_MIN       ZL303XX_REF_ID_0
#define ZL303XX_REF_ID_MAX       ZL303XX_REF_ID_8
#define ZL303XX_DPLL_NUM_REFS    (ZL303XX_REF_ID_MAX + 1)

#define ZL303XX_CHECK_REF_ID(val)   \
            (((zl303xx_RefIdE)(val) > ZL303XX_REF_ID_MAX) ?  \
            ZL303XX_PARAMETER_INVALID : \
            ZL303XX_OK)

/***************/

/* SyncId valid values */
typedef enum
{
   ZL303XX_SYNC_ID_0 = 0,
   ZL303XX_SYNC_ID_1 = 1,
   ZL303XX_SYNC_ID_2 = 2,
   ZL303XX_SYNC_ID_8 = 8
} zl303xx_SyncIdE;

/* Range limit definitions & parameter checking */
#define ZL303XX_SYNC_ID_MIN   ZL303XX_SYNC_ID_0
#define ZL303XX_SYNC_ID_MAX   ZL303XX_SYNC_ID_8

#define ZL303XX_CHECK_SYNC_ID(val)   \
            ((((zl303xx_SyncIdE)(val) > ZL303XX_SYNC_ID_2) && ((zl303xx_SyncIdE)(val) != ZL303XX_SYNC_ID_8)) ?  \
            ZL303XX_PARAMETER_INVALID : \
            ZL303XX_OK)

/***************/

/* DiffRun valid values */
typedef enum
{
   ZL303XX_DIFF_RUN = 0,
   ZL303XX_DIFF_LOGIC_1 = 1
} zl303xx_DiffRunE;

/* Range limit definitions & parameter checking */
#define ZL303XX_DIFF_RUN_MIN   ZL303XX_DIFF_RUN
#define ZL303XX_DIFF_RUN_MAX   ZL303XX_DIFF_LOGIC_1

#define ZL303XX_CHECK_DIFF_RUN(val)   \
            (((zl303xx_DiffRunE)(val) > ZL303XX_DIFF_RUN_MAX) ?  \
            ZL303XX_PARAMETER_INVALID : \
            ZL303XX_OK)

/***************/

/* OutEnable valid values */
typedef enum
{
   ZL303XX_OUT_HIZ = 0,
   ZL303XX_OUT_ENABLE = 1
} zl303xx_OutEnableE;

/* Range limit definitions & parameter checking */
#define ZL303XX_OUT_ENABLE_MIN   ZL303XX_OUT_HIZ
#define ZL303XX_OUT_ENABLE_MAX   ZL303XX_OUT_ENABLE

#define ZL303XX_CHECK_OUT_ENABLE(val)   \
            (((zl303xx_OutEnableE)(val) > ZL303XX_OUT_ENABLE_MAX) ?  \
            ZL303XX_PARAMETER_INVALID : \
            ZL303XX_OK)

/***************/

/* Enable valid values */
typedef enum
{
   ZL303XX_ENABLED = 0,
   ZL303XX_DISABLED = 1
} zl303xx_EnableE;

/* Range limit definitions & parameter checking */
#define ZL303XX_ENABLE_MIN   ZL303XX_ENABLED
#define ZL303XX_ENABLE_MAX   ZL303XX_DISABLED

#define ZL303XX_CHECK_ENABLE(val)   \
            (((zl303xx_EnableE)(val) > ZL303XX_ENABLE_MAX) ?  \
            ZL303XX_PARAMETER_INVALID : \
            ZL303XX_OK)

/***************/

/* PassFail valid values */
typedef enum
{
   ZL303XX_STATUS_PASS = 0,
   ZL303XX_STATUS_FAIL = 1
} zl303xx_PassFailE;

/* Range limit definitions & parameter checking */
#define ZL303XX_PASS_FAIL_MIN   ZL303XX_STATUS_PASS
#define ZL303XX_PASS_FAIL_MAX   ZL303XX_STATUS_FAIL

#define ZL303XX_CHECK_PASS_FAIL(val)   \
            (((zl303xx_PassFailE)(val) > ZL303XX_PASS_FAIL_MAX) ?  \
            ZL303XX_PARAMETER_INVALID : \
            ZL303XX_OK)

/***************/

/* IsrState valid values */
typedef enum
{
   ZL303XX_ISR_STATE_MASK = 0,
   ZL303XX_ISR_STATE_ACTV = 1
} zl303xx_IsrStateE;

/* Range limit definitions & parameter checking */
#define ZL303XX_ISR_STATE_MIN   ZL303XX_ISR_STATE_MASK
#define ZL303XX_ISR_STATE_MAX   ZL303XX_ISR_STATE_ACTV

#define ZL303XX_CHECK_ISR_STATE(val)   \
            (((zl303xx_IsrStateE)(val) > ZL303XX_ISR_STATE_MAX) ?  \
            ZL303XX_PARAMETER_INVALID : \
            ZL303XX_OK)

/***************/

/* DpllId valid values */
typedef enum
{
    ZL303XX_DPLL_ID_1,
    ZL303XX_DPLL_ID_2
} zl303xx_DpllIdE;

/* Range limit definitions & parameter checking */
#define ZL303XX_DPLL_ID_MIN      ZL303XX_DPLL_ID_1
#define ZL303XX_DPLL_ID_MAX      ZL303XX_DPLL_ID_2

#define ZL303XX_DPLL_NUM_DPLLS   (ZL303XX_DPLL_ID_MAX + 1)

#define ZL303XX_CHECK_DPLL_ID(val)   \
            (((zl303xx_DpllIdE)(val) > ZL303XX_DPLL_ID_MAX) ?  \
            ZL303XX_PARAMETER_INVALID : \
            ZL303XX_OK)

/***************/

/* ClockId valid values */
typedef enum
{
   ZL303XX_CLOCK_ID_0 = 0,
   ZL303XX_CLOCK_ID_1 = 1
} zl303xx_ClockIdE;

/* Range limit definitions & parameter checking */
#define ZL303XX_CLOCK_ID_MIN   ZL303XX_CLOCK_ID_0
#define ZL303XX_CLOCK_ID_MAX   ZL303XX_CLOCK_ID_1

#define ZL303XX_CHECK_CLOCK_ID(val)   \
            (((zl303xx_ClockIdE)(val) > ZL303XX_CLOCK_ID_MAX) ?  \
            ZL303XX_PARAMETER_INVALID : \
            ZL303XX_OK)

/***************/

/* FpId valid values */
typedef enum
{
   ZL303XX_FP_ID_0 = 0,
   ZL303XX_FP_ID_1 = 1
} zl303xx_FpIdE;

/* Range limit definitions & parameter checking */
#define ZL303XX_FP_ID_MIN   ZL303XX_FP_ID_0
#define ZL303XX_FP_ID_MAX   ZL303XX_FP_ID_1

#define ZL303XX_CHECK_FP_ID(val)   \
            (((zl303xx_FpIdE)(val) > ZL303XX_FP_ID_MAX) ?  \
            ZL303XX_PARAMETER_INVALID : \
            ZL303XX_OK)

/***************/

/* CompId valid values */
typedef enum
{
   ZL303XX_COMP_ID_0 = 0,
   ZL303XX_COMP_ID_1 = 1
} zl303xx_CompIdE;

/* Range limit definitions & parameter checking */
#define ZL303XX_COMP_ID_MIN   ZL303XX_COMP_ID_0
#define ZL303XX_COMP_ID_MAX   ZL303XX_COMP_ID_1

#define ZL303XX_CHECK_COMP_ID(val)   \
            (((zl303xx_CompIdE)(val) > ZL303XX_COMP_ID_MAX) ?  \
            ZL303XX_PARAMETER_INVALID : \
            ZL303XX_OK)

/***************/

/* OutputRun valid values */
typedef enum
{
   ZL303XX_OUT_LOGIC0 = 0,
   ZL303XX_OUT_RUN = 1
} zl303xx_OutputRunE;

/* Range limit definitions & parameter checking */
#define ZL303XX_OUTPUT_RUN_MIN   ZL303XX_OUT_LOGIC0
#define ZL303XX_OUTPUT_RUN_MAX   ZL303XX_OUT_RUN

#define ZL303XX_CHECK_OUTPUT_RUN(val)   \
            (((zl303xx_OutputRunE)(val) > ZL303XX_OUTPUT_RUN_MAX) ?  \
            ZL303XX_PARAMETER_INVALID : \
            ZL303XX_OK)

/***************/

/* CustId valid values */
typedef enum
{
   ZL303XX_CUST_ID_A = 0,
   ZL303XX_CUST_ID_B = 1
} zl303xx_CustIdE;

/* Range limit definitions & parameter checking */
#define ZL303XX_CUST_ID_MIN   ZL303XX_CUST_ID_A
#define ZL303XX_CUST_ID_MAX   ZL303XX_CUST_ID_B

#define ZL303XX_CHECK_CUST_ID(val)   \
            (((zl303xx_CustIdE)(val) > ZL303XX_CUST_ID_MAX) ?  \
            ZL303XX_PARAMETER_INVALID : \
            ZL303XX_OK)

/***************/

/* ClkOffset valid values */
typedef enum
{
   ZL303XX_CLK_OFFSET_0 = 0,
   ZL303XX_CLK_OFFSET_90 = 1,
   ZL303XX_CLK_OFFSET_180 = 2,
   ZL303XX_CLK_OFFSET_270 = 3
} zl303xx_ClkOffsetE;

/* Range limit definitions & parameter checking */
#define ZL303XX_CLK_OFFSET_MIN   ZL303XX_CLK_OFFSET_0
#define ZL303XX_CLK_OFFSET_MAX   ZL303XX_CLK_OFFSET_270

#define ZL303XX_CHECK_CLK_OFFSET(val)   \
            (((zl303xx_ClkOffsetE)(val) > ZL303XX_CLK_OFFSET_MAX) ?  \
            ZL303XX_PARAMETER_INVALID : \
            ZL303XX_OK)

/***************/

/* FpFreq valid values */
typedef enum
{
   ZL303XX_FP_FREQ_166HZ = 0,
   ZL303XX_FP_FREQ_400HZ = 1,
   ZL303XX_FP_FREQ_1KHZ = 2,
   ZL303XX_FP_FREQ_2KHZ = 3,
   ZL303XX_FP_FREQ_4KHZ = 4,
   ZL303XX_FP_FREQ_8KHZ = 5,
   ZL303XX_FP_FREQ_32KHZ = 6,
   ZL303XX_FP_FREQ_64KHZ = 7,
   ZL303XX_FP_FREQ_1HZ = 8,
   ZL303XX_FP_FREQ_1PPS = 9
} zl303xx_FpFreqE;

/* Range limit definitions & parameter checking */
#define ZL303XX_FP_FREQ_MIN   ZL303XX_FP_FREQ_166HZ
#define ZL303XX_FP_FREQ_MAX   ZL303XX_FP_FREQ_1PPS

#define ZL303XX_CHECK_FP_FREQ(val)   \
            (((zl303xx_FpFreqE)(val) > ZL303XX_FP_FREQ_MAX) ?  \
            ZL303XX_PARAMETER_INVALID : \
            ZL303XX_OK)

/***************/

/* FpStyle valid values */
typedef enum
{
   ZL303XX_FP_STYLE_50PER = 0,
   ZL303XX_FP_STYLE_E1 = 1
} zl303xx_FpStyleE;

/* Range limit definitions & parameter checking */
#define ZL303XX_FP_STYLE_MIN   ZL303XX_FP_STYLE_50PER
#define ZL303XX_FP_STYLE_MAX   ZL303XX_FP_STYLE_E1

#define ZL303XX_CHECK_FP_STYLE(val)   \
            (((zl303xx_FpStyleE)(val) > ZL303XX_FP_STYLE_MAX) ?  \
            ZL303XX_PARAMETER_INVALID : \
            ZL303XX_OK)

/***************/

/* FpSyncEdge valid values */
typedef enum
{
   ZL303XX_FP_EDGE_RISE = 0,
   ZL303XX_FP_EDGE_FALL = 1
} zl303xx_FpSyncEdgeE;

/* Range limit definitions & parameter checking */
#define ZL303XX_FP_SYNC_EDGE_MIN   ZL303XX_FP_EDGE_RISE
#define ZL303XX_FP_SYNC_EDGE_MAX   ZL303XX_FP_EDGE_FALL

#define ZL303XX_CHECK_FP_SYNC_EDGE(val)   \
            (((zl303xx_FpSyncEdgeE)(val) > ZL303XX_FP_SYNC_EDGE_MAX) ?  \
            ZL303XX_PARAMETER_INVALID : \
            ZL303XX_OK)

/***************/

/* FpTypePx valid values */
typedef enum
{
   ZL303XX_FP_TYPE_PX_F4 = 0,
   ZL303XX_FP_TYPE_PX_F8 = 1,
   ZL303XX_FP_TYPE_PX_F16 = 2,
   ZL303XX_FP_TYPE_PX_F32 = 3,
   ZL303XX_FP_TYPE_PX_F65 = 4,
   ZL303XX_FP_TYPE_PX_50_PERCENT = 5,
   ZL303XX_FP_TYPE_PX_CYCLE = 7
} zl303xx_FpTypePxE;

/* Range limit definitions & parameter checking */
#define ZL303XX_FP_TYPE_PX_MIN   ZL303XX_FP_TYPE_PX_F4
#define ZL303XX_FP_TYPE_PX_MAX   ZL303XX_FP_TYPE_PX_CYCLE

#define ZL303XX_CHECK_FP_TYPE_PX(val)   \
            (((zl303xx_FpTypePxE)(val) > ZL303XX_FP_TYPE_PX_MAX) || ((zl303xx_FpTypePxE)(val) == 6) ?  \
            ZL303XX_PARAMETER_INVALID : \
            ZL303XX_OK)

/***************/

/* FpPolarity valid values */
typedef enum
{
   ZL303XX_FP_POLARITY_POS = 0,
   ZL303XX_FP_POLARITY_NEG = 1
} zl303xx_FpPolarityE;

/* Range limit definitions & parameter checking */
#define ZL303XX_FP_POLARITY_MIN   ZL303XX_FP_POLARITY_POS
#define ZL303XX_FP_POLARITY_MAX   ZL303XX_FP_POLARITY_NEG

#define ZL303XX_CHECK_FP_POLARITY(val)   \
            (((zl303xx_FpPolarityE)(val) > ZL303XX_FP_POLARITY_MAX) ?  \
            ZL303XX_PARAMETER_INVALID : \
            ZL303XX_OK)

/***************/

/* SynthId valid values */
typedef enum
{
   ZL303XX_SYNTH_ID_P0 = 0,
   ZL303XX_SYNTH_ID_P1 = 1
} zl303xx_SynthIdE;

/* Range limit definitions & parameter checking */
#define ZL303XX_SYNTH_ID_MIN   ZL303XX_SYNTH_ID_P0
#define ZL303XX_SYNTH_ID_MAX   ZL303XX_SYNTH_ID_P1

#define ZL303XX_CHECK_SYNTH_ID(val)   \
            (((zl303xx_SynthIdE)(val) > ZL303XX_SYNTH_ID_MAX) ?  \
            ZL303XX_PARAMETER_INVALID : \
            ZL303XX_OK)

/***************/

/* RefMode valid values */
typedef enum
{
   ZL303XX_REF_MODE_AUTO = 0,
   ZL303XX_REF_MODE_CUSTA = 1,
   ZL303XX_REF_MODE_CUSTB = 2,
   ZL303XX_REF_MODE_REV2AUTO = 3
} zl303xx_RefModeE;

/* Range limit definitions & parameter checking */
#define ZL303XX_REF_MODE_MIN   ZL303XX_REF_MODE_AUTO
#define ZL303XX_REF_MODE_MAX   ZL303XX_REF_MODE_REV2AUTO

#define ZL303XX_CHECK_REF_MODE(val)   \
            (((zl303xx_RefModeE)(val) > ZL303XX_REF_MODE_MAX) ?  \
            ZL303XX_PARAMETER_INVALID : \
            ZL303XX_OK)

/***************/

/* RefDivide valid values */
typedef enum
{
   ZL303XX_REF_DIV_1 = 0,
   ZL303XX_REF_DIV_2 = 1,
   ZL303XX_REF_DIV_3 = 2,
   ZL303XX_REF_DIV_4 = 3,
   ZL303XX_REF_DIV_5 = 4,
   ZL303XX_REF_DIV_6 = 5,
   ZL303XX_REF_DIV_7 = 6,
   ZL303XX_REF_DIV_8 = 7,
   ZL303XX_REF_DIV_1P5 = 10,
   ZL303XX_REF_DIV_2P5 = 12
} zl303xx_RefDivideE;

/* Range limit definitions & parameter checking */
#define ZL303XX_REF_DIVIDE_MIN   ZL303XX_REF_DIV_1
#define ZL303XX_REF_DIVIDE_MAX   ZL303XX_REF_DIV_2P5

#define ZL303XX_CHECK_REF_DIVIDE(val)   \
            (((zl303xx_RefDivideE)(val) > ZL303XX_REF_DIVIDE_MAX) ?  \
            ZL303XX_PARAMETER_INVALID : \
            ZL303XX_OK)

/***************/

/* RefFreq valid values */
typedef enum
{
   ZL303XX_REF_2KHz = 0,
   ZL303XX_REF_8KHz = 1,
   ZL303XX_REF_64KHz = 2,
   ZL303XX_REF_1544KHz = 3,
   ZL303XX_REF_2048KHz = 4,
   ZL303XX_REF_6480KHz = 5,
   ZL303XX_REF_8192KHz = 6,
   ZL303XX_REF_16384KHz = 7,
   ZL303XX_REF_19449KHz = 8,
   ZL303XX_REF_38880KHz = 9,
   ZL303XX_REF_77760KHz = 10,
   ZL303XX_REF_UNKNOWN = 11
} zl303xx_RefFreqE;

/* Range limit definitions & parameter checking */
#define ZL303XX_REF_FREQ_MIN   ZL303XX_REF_2KHz
#define ZL303XX_REF_FREQ_MAX   ZL303XX_REF_UNKNOWN

#define ZL303XX_CHECK_REF_FREQ(val)   \
            (((zl303xx_RefFreqE)(val) > ZL303XX_REF_FREQ_MAX) ?  \
            ZL303XX_PARAMETER_INVALID : \
            ZL303XX_OK)

/***************/

/* SyncFreq valid values */
typedef enum
{
   ZL303XX_SYNC_166Hz = 0,
   ZL303XX_SYNC_400Hz = 1,
   ZL303XX_SYNC_1000Hz = 2,
   ZL303XX_SYNC_2000Hz = 3,
   ZL303XX_SYNC_1Hz = 4,
   ZL303XX_SYNC_8000Hz = 5,
   ZL303XX_SYNC_UNKNOWN = 6,
   ZL303XX_SYNC_64KHz = 7
} zl303xx_SyncFreqE;

/* Range limit definitions & parameter checking */
#define ZL303XX_SYNC_FREQ_MIN   ZL303XX_SYNC_166Hz
#define ZL303XX_SYNC_FREQ_MAX   ZL303XX_SYNC_64KHz

#define ZL303XX_CHECK_SYNC_FREQ(val)   \
            (((zl303xx_SyncFreqE)(val) > ZL303XX_SYNC_FREQ_MAX) ?  \
            ZL303XX_PARAMETER_INVALID : \
            ZL303XX_OK)

/***************/

/* RefOor valid values */
typedef enum
{
   ZL303XX_OOR_9_12PPM = 0,
   ZL303XX_OOR_40_52PPM = 1,
   ZL303XX_OOR_100_130PPM = 2,
   ZL303XX_OOR_64_83PPM = 3,
   ZL303XX_OOR_13_18PPM = 4,
   ZL303XX_OOR_24_32PPM = 5,
   ZL303XX_OOR_36_47PPM = 6,
   ZL303XX_OOR_52_67PPM = 7
} zl303xx_RefOorE;

/* Range limit definitions & parameter checking */
#define ZL303XX_REF_OOR_MIN   ZL303XX_OOR_9_12PPM
#define ZL303XX_REF_OOR_MAX   ZL303XX_OOR_52_67PPM

#define ZL303XX_CHECK_REF_OOR(val)   \
            (((zl303xx_RefOorE)(val) > ZL303XX_REF_OOR_MAX) ?  \
            ZL303XX_PARAMETER_INVALID : \
            ZL303XX_OK)

/***************/

/* TtoDisQual valid values */
typedef enum
{
   ZL303XX_TTDQ_MIN = 0,
   ZL303XX_TTDQ_P5MS = 1,
   ZL303XX_TTDQ_1MS = 2,
   ZL303XX_TTDQ_5MS = 3,
   ZL303XX_TTDQ_10MS = 4,
   ZL303XX_TTDQ_50MS = 5,
   ZL303XX_TTDQ_100MS = 6,
   ZL303XX_TTDQ_500MS = 7,
   ZL303XX_TTDQ_1000MS = 8,
   ZL303XX_TTDQ_2000MS = 9,
   ZL303XX_TTDQ_2500MS = 10,
   ZL303XX_TTDQ_4000MS = 11,
   ZL303XX_TTDQ_8000MS = 12,
   ZL303XX_TTDQ_16000MS = 13,
   ZL303XX_TTDQ_32000MS = 14,
   ZL303XX_TTDQ_64000MS = 15
} zl303xx_TtoDisQualE;

/* Range limit definitions & parameter checking */
#define ZL303XX_TTO_DIS_QUAL_MIN   ZL303XX_TTDQ_MIN
#define ZL303XX_TTO_DIS_QUAL_MAX   ZL303XX_TTDQ_64000MS

#define ZL303XX_CHECK_TTO_DIS_QUAL(val)   \
            (((zl303xx_TtoDisQualE)(val) > ZL303XX_TTO_DIS_QUAL_MAX) ?  \
            ZL303XX_PARAMETER_INVALID : \
            ZL303XX_OK)

/***************/

/* TtoQual valid values */
typedef enum
{
   ZL303XX_TTQ_X2 = 0,
   ZL303XX_TTQ_X4 = 1,
   ZL303XX_TTQ_X16 = 2,
   ZL303XX_TTQ_X32 = 3
} zl303xx_TtoQualE;

/* Range limit definitions & parameter checking */
#define ZL303XX_TTO_QUAL_MIN   ZL303XX_TTQ_X2
#define ZL303XX_TTO_QUAL_MAX   ZL303XX_TTQ_X32

#define ZL303XX_CHECK_TTO_QUAL(val)   \
            (((zl303xx_TtoQualE)(val) > ZL303XX_TTO_QUAL_MAX) ?  \
            ZL303XX_PARAMETER_INVALID : \
            ZL303XX_OK)

/***************/

/* DpllHitless valid values */
typedef enum
{
   ZL303XX_DPLL_HITLESS = 0,
   ZL303XX_DPLL_REALIGN = 1
} zl303xx_DpllHitlessE;

/* Range limit definitions & parameter checking */
#define ZL303XX_DPLL_HITLESS_MIN   ZL303XX_DPLL_HITLESS
#define ZL303XX_DPLL_HITLESS_MAX   ZL303XX_DPLL_REALIGN

#define ZL303XX_CHECK_DPLL_HITLESS(val)   \
            (((zl303xx_DpllHitlessE)(val) > ZL303XX_DPLL_HITLESS_MAX) ?  \
            ZL303XX_PARAMETER_INVALID : \
            ZL303XX_OK)

/***************/

/* DpllBw valid values */
typedef enum
{
   ZL303XX_DPLL_BW_P1Hz = 0,
   ZL303XX_DPLL_BW_1P7Hz = 1,
   ZL303XX_DPLL_BW_3P5Hz = 2,
   ZL303XX_DPLL_BW_14Hz = 3,
   ZL303XX_DPLL_BW_28Hz = 4,
   ZL303XX_DPLL_BW_890Hz = 5,
   ZL303XX_DPLL_BW_FAST = 6,
   ZL303XX_DPLL_BW_S3E_0P3mHZ = 7,
   ZL303XX_DPLL_BW_S3E_1mHZ = 39,
   ZL303XX_DPLL_BW_S3E_3mHZ = 71,
   ZL303XX_DPLL_BW_INVALID = 103
} zl303xx_DpllBwE;

/* Range limit definitions & parameter checking */
#define ZL303XX_DPLL_BW_MIN   ZL303XX_DPLL_BW_P1Hz
#define ZL303XX_DPLL_BW_MAX   ZL303XX_DPLL_BW_S3E_3mHZ

#define ZL303XX_CHECK_DPLL_BW(val)   \
            (((zl303xx_DpllBwE)(val) > ZL303XX_DPLL_BW_MAX) || \
             (((zl303xx_DpllBwE)(val) > ZL303XX_DPLL_BW_S3E_0P3mHZ) && \
              ((zl303xx_DpllBwE)(val) != ZL303XX_DPLL_BW_S3E_1mHZ) && ((zl303xx_DpllBwE)(val) != ZL303XX_DPLL_BW_S3E_3mHZ)) ?  \
            ZL303XX_PARAMETER_INVALID : \
            ZL303XX_OK)

/***************/

/* DpllPsl valid values */
typedef enum
{
   ZL303XX_DPLL_PSL_P885US = 0,
   ZL303XX_DPLL_PSL_7P5US = 1,
   ZL303XX_DPLL_PSL_61US = 2,
   ZL303XX_DPLL_PSL_UNLIMITED = 3
} zl303xx_DpllPslE;

/* Range limit definitions & parameter checking */
#define ZL303XX_DPLL_PSL_MIN   ZL303XX_DPLL_PSL_P885US
#define ZL303XX_DPLL_PSL_MAX   ZL303XX_DPLL_PSL_UNLIMITED

#define ZL303XX_CHECK_DPLL1_PSL(val)   \
            (((zl303xx_DpllPslE)(val) > ZL303XX_DPLL_PSL_MAX) ?  \
            ZL303XX_PARAMETER_INVALID : \
            ZL303XX_OK)

#define ZL303XX_CHECK_DPLL2_PSL(val)   \
            (((zl303xx_DpllPslE)(val) < ZL303XX_DPLL_PSL_61US) || ((zl303xx_DpllPslE)(val) > ZL303XX_DPLL_PSL_MAX) ?  \
            ZL303XX_PARAMETER_INVALID : \
            ZL303XX_OK)

#define ZL303XX_CHECK_DPLL_PSL(dpllId,val)   \
                    (Uint32T)((dpllId == ZL303XX_DPLL_ID_1) ? \
                              (ZL303XX_CHECK_DPLL1_PSL(val)) : \
                              (ZL303XX_CHECK_DPLL2_PSL(val)))

/***************/

/* DpllHldUpdate valid values */
typedef enum
{
   ZL303XX_DPLL_HLD_UPD_26mS = 0,
   ZL303XX_DPLL_HLD_UPD_1S = 1,
   ZL303XX_DPLL_HLD_UPD_10S = 2,
   ZL303XX_DPLL_HLD_UPD_60S = 3
} zl303xx_DpllHldUpdateE;

/* Range limit definitions & parameter checking */
#define ZL303XX_DPLL_HLD_UPDATE_MIN   ZL303XX_DPLL_HLD_UPD_26mS
#define ZL303XX_DPLL_HLD_UPDATE_MAX   ZL303XX_DPLL_HLD_UPD_60S

#define ZL303XX_CHECK_DPLL_HLD_UPDATE(val)   \
            (((zl303xx_DpllHldUpdateE)(val) > ZL303XX_DPLL_HLD_UPDATE_MAX) ?  \
            ZL303XX_PARAMETER_INVALID : \
            ZL303XX_OK)

/***************/

/* DpllHldFilter valid values */
typedef enum
{
   ZL303XX_DPLL_HLD_FILT_BYPASS = 0,
   ZL303XX_DPLL_HLD_FILT_18mHz = 1,
   ZL303XX_DPLL_HLD_FILT_P6Hz = 2,
   ZL303XX_DPLL_HLD_FILT_10Hz = 3
} zl303xx_DpllHldFilterE;

/* Range limit definitions & parameter checking */
#define ZL303XX_DPLL_HLD_FILTER_MIN   ZL303XX_DPLL_HLD_FILT_BYPASS
#define ZL303XX_DPLL_HLD_FILTER_MAX   ZL303XX_DPLL_HLD_FILT_10Hz

#define ZL303XX_CHECK_DPLL_HLD_FILTER(val)   \
            (((zl303xx_DpllHldFilterE)(val) > ZL303XX_DPLL_HLD_FILTER_MAX) ?  \
            ZL303XX_PARAMETER_INVALID : \
            ZL303XX_OK)

/***************/

/* DpllMode valid values */
typedef enum
{
   ZL303XX_DPLL_MODE_NORM = 0,
   ZL303XX_DPLL_MODE_HOLD = 1,
   ZL303XX_DPLL_MODE_FREE = 2,
   ZL303XX_DPLL_MODE_AUTO = 3,
   ZL303XX_DPLL_MODE_TOP  = 4
} zl303xx_DpllModeE;

/* Range limit definitions & parameter checking */
#define ZL303XX_DPLL_MODE_MIN   ZL303XX_DPLL_MODE_NORM
#define ZL303XX_DPLL_MODE_MAX   ZL303XX_DPLL_MODE_TOP

#define ZL303XX_CHECK_DPLL_MODE(val)   \
            (((zl303xx_DpllModeE)(val) > ZL303XX_DPLL_MODE_MAX) ?  \
            ZL303XX_PARAMETER_INVALID : \
            ZL303XX_OK)

/***************/

/* DpllHoldState valid values */
typedef enum
{
   ZL303XX_DPLL_HOLD_FALSE = 0,
   ZL303XX_DPLL_HOLD_TRUE = 1
} zl303xx_DpllHoldStateE;

/* Range limit definitions & parameter checking */
#define ZL303XX_DPLL_HOLD_STATE_MIN   ZL303XX_DPLL_HOLD_FALSE
#define ZL303XX_DPLL_HOLD_STATE_MAX   ZL303XX_DPLL_HOLD_TRUE

#define ZL303XX_CHECK_DPLL_HOLD_STATE(val)   \
            (((zl303xx_DpllHoldStateE)(val) > ZL303XX_DPLL_HOLD_STATE_MAX) ?  \
            ZL303XX_PARAMETER_INVALID : \
            ZL303XX_OK)

/***************/

/* DpllLockState valid values */
typedef enum
{
   ZL303XX_DPLL_LOCK_FALSE = 0,
   ZL303XX_DPLL_LOCK_TRUE = 1
} zl303xx_DpllLockStateE;

/* Range limit definitions & parameter checking */
#define ZL303XX_DPLL_LOCK_STATE_MIN   ZL303XX_DPLL_LOCK_FALSE
#define ZL303XX_DPLL_LOCK_STATE_MAX   ZL303XX_DPLL_LOCK_TRUE

#define ZL303XX_CHECK_DPLL_LOCK_STATE(val)   \
            (((zl303xx_DpllLockStateE)(val) > ZL303XX_DPLL_LOCK_STATE_MAX) ?  \
            ZL303XX_PARAMETER_INVALID : \
            ZL303XX_OK)

/***************/

/* DpllPullIn valid values */
typedef enum
{
   ZL303XX_DPLL_PULLIN_12PPM = 0,
   ZL303XX_DPLL_PULLIN_52PPM = 1,
   ZL303XX_DPLL_PULLIN_130PPM = 2,
   ZL303XX_DPLL_PULLIN_83PPM = 3
} zl303xx_DpllPullInE;

/* Range limit definitions & parameter checking */
#define ZL303XX_DPLL_PULL_IN_MIN   ZL303XX_DPLL_PULLIN_12PPM
#define ZL303XX_DPLL_PULL_IN_MAX   ZL303XX_DPLL_PULLIN_83PPM

#define ZL303XX_CHECK_DPLL_PULL_IN(val)   \
            (((zl303xx_DpllPullInE)(val) > ZL303XX_DPLL_PULL_IN_MAX) ?  \
            ZL303XX_PARAMETER_INVALID : \
            ZL303XX_OK)

/***************/

#define ZL303XX_DPLL_WAIT_TO_RESTORE_MIN   0x00
#define ZL303XX_DPLL_WAIT_TO_RESTORE_MAX   0x0F
#define ZL303XX_CHECK_DPLL_WAIT_TO_RESTORE(X) \
   ((X > ZL303XX_DPLL_WAIT_TO_RESTORE_MAX) ? \
    (ZL303XX_ERROR_NOTIFY("Invalid PLL wait to restore: " #X),ZL303XX_PARAMETER_INVALID) : \
    ZL303XX_OK)

/***************/

/* EthDiv valid values */
typedef enum
{
   ZL303XX_ETH_DIV_SDH = 0,
   ZL303XX_ETH_DIV_ETH = 1
} zl303xx_EthDivE;

/* Range limit definitions & parameter checking */
#define ZL303XX_ETH_DIV_MIN   ZL303XX_ETH_DIV_SDH
#define ZL303XX_ETH_DIV_MAX   ZL303XX_ETH_DIV_ETH

#define ZL303XX_CHECK_ETH_DIV(val)   \
            (((zl303xx_EthDivE)(val) > ZL303XX_ETH_DIV_MAX) ?  \
            ZL303XX_PARAMETER_INVALID : \
            ZL303XX_OK)

/***************/

/* SdhCtrFreq valid values */
typedef enum
{
   ZL303XX_SDH_CTR_FREQ_622 = 0,
   ZL303XX_SDH_CTR_FREQ_625 = 1
} zl303xx_SdhCtrFreqE;

/* Range limit definitions & parameter checking */
#define ZL303XX_SDH_CTR_FREQ_MIN   ZL303XX_SDH_CTR_FREQ_622
#define ZL303XX_SDH_CTR_FREQ_MAX   ZL303XX_SDH_CTR_FREQ_625

#define ZL303XX_CHECK_SDH_CTR_FREQ(val)   \
            (((zl303xx_SdhCtrFreqE)(val) > ZL303XX_SDH_CTR_FREQ_MAX) ?  \
            ZL303XX_PARAMETER_INVALID : \
            ZL303XX_OK)

/***************/

/* FpTypeSdh valid values */
typedef enum
{
   ZL303XX_FP_TYPE_SDH_F19 = 0,
   ZL303XX_FP_TYPE_SDH_F38 = 1,
   ZL303XX_FP_TYPE_SDH_F77 = 2,
   ZL303XX_FP_TYPE_SDH_F155 = 3,
   ZL303XX_FP_TYPE_SDH_F6 = 4,
   ZL303XX_FP_TYPE_SDH_F51 = 5,
   ZL303XX_FP_TYPE_SDH_UNDEF = 6,
   ZL303XX_FP_TYPE_SDH_CYCLE = 7
} zl303xx_FpTypeSdhE;

/* Range limit definitions & parameter checking */
#define ZL303XX_FP_TYPE_SDH_MIN   ZL303XX_FP_TYPE_SDH_F19
#define ZL303XX_FP_TYPE_SDH_MAX   ZL303XX_FP_TYPE_SDH_CYCLE

#define ZL303XX_CHECK_FP_TYPE_SDH(val)   \
            (((zl303xx_FpTypeSdhE)(val) > ZL303XX_FP_TYPE_SDH_MAX) ?  \
            ZL303XX_PARAMETER_INVALID : \
            ZL303XX_OK)

/***************/

/* SdhClkFreq valid values */
typedef enum
{
   /* Requires eth_en = 0, f_sel_n = 0. Encode f_sel_n in bit4, eth_en in bit5 of enum value. */
   ZL303XX_SDH_CLK_FREQ_77MHZ = 0x02,
   ZL303XX_SDH_CLK_FREQ_38MHZ = 0x03,
   ZL303XX_SDH_CLK_FREQ_19MHZ = 0x04,
   ZL303XX_SDH_CLK_FREQ_9MHZ = 0x05,
   ZL303XX_SDH_CLK_FREQ_51MHZ = 0x0A,
   ZL303XX_SDH_CLK_FREQ_26MHZ = 0x0B,
   ZL303XX_SDH_CLK_FREQ_13MHZ = 0x0C,
   ZL303XX_SDH_CLK_FREQ_6MHZ = 0x0D,


   /* The following values will be masked to the correct register value */
   /* Requires eth_en = 1, f_sel_n = 1. Use above encoding method. */
   ZL303XX_SDH_CLK_FREQ_125MHZ = 0x31,
   ZL303XX_SDH_CLK_FREQ_62MHZ = 0x32,
   ZL303XX_SDH_CLK_FREQ_50MHZ = 0x35,
   ZL303XX_SDH_CLK_FREQ_25MHZ = 0x36,
   ZL303XX_SDH_CLK_FREQ_12MHZ = 0x37
} zl303xx_SdhClkFreqE;

/* Range limit definitions & parameter checking */
#define ZL303XX_SDH_CLK_FREQ_MIN   ZL303XX_SDH_CLK_FREQ_77MHZ
#define ZL303XX_SDH_CLK_FREQ_MAX   ZL303XX_SDH_CLK_FREQ_12MHZ

#define ZL303XX_CHECK_SDH_CLK_FREQ(val)   \
            (((zl303xx_SdhClkFreqE)(val) > ZL303XX_SDH_CLK_FREQ_MAX) ?  \
            ZL303XX_PARAMETER_INVALID : \
            ZL303XX_OK)

/***************/

/* DiffFreq valid values */
typedef enum
{
   /* SONET/SDH Mode */
   /* Requires eth_en = 0, f_sel_n = 0. Encode f_sel_n in bit4, eth_en in bit5 of enum value. */
   ZL303XX_DIFF_FREQ_19MHZ = 0x00,
   ZL303XX_DIFF_FREQ_38MHZ = 0x01,
   ZL303XX_DIFF_FREQ_77MHZ = 0x02,
   ZL303XX_DIFF_FREQ_155MHZ = 0x03,
   ZL303XX_DIFF_FREQ_311MHZ = 0x04,
   ZL303XX_DIFF_FREQ_622MHZ = 0x05,
   ZL303XX_DIFF_FREQ_6MHZ = 0x06,
   ZL303XX_DIFF_FREQ_51MHZ = 0x07,

   /* Ethernet Mode - Low Speed */
   /* Requires eth_en = 1, f_sel_n = 1. Use above encoding method. */
   ZL303XX_DIFF_FREQ_125MHZ = 0x31,
   ZL303XX_DIFF_FREQ_62MHZ = 0x32,
   ZL303XX_DIFF_FREQ_50MHZ = 0x35,
   ZL303XX_DIFF_FREQ_25MHZ = 0x36,
   ZL303XX_DIFF_FREQ_12MHZ = 0x37,

   /* Ethernet Mode - High Speed */
   /* Requires eth_en = 1, f_sel_n = 0. Use above encoding method. */
   ZL303XX_DIFF_FREQ_156MHZ = 0x23,
   ZL303XX_DIFF_FREQ_312MHZ = 0x24
} zl303xx_DiffFreqE;

/* Range limit definitions & parameter checking */
#define ZL303XX_DIFF_FREQ_MIN   ZL303XX_DIFF_FREQ_19MHZ
#define ZL303XX_DIFF_FREQ_MAX   ZL303XX_DIFF_FREQ_12MHZ

#define ZL303XX_CHECK_DIFF_FREQ(val)   \
            (((zl303xx_DiffFreqE)(val) > ZL303XX_DIFF_FREQ_MAX) ?  \
            ZL303XX_PARAMETER_INVALID : \
            ZL303XX_OK)

/***************/

/* DiffId valid values */
typedef enum
{
   ZL303XX_DIFF_ID_0 = 0,
   ZL303XX_DIFF_ID_1 = 1
} zl303xx_DiffIdE;

/* Range limit definitions & parameter checking */
#define ZL303XX_DIFF_ID_MIN   ZL303XX_DIFF_ID_0
#define ZL303XX_DIFF_ID_MAX   ZL303XX_DIFF_ID_1

#define ZL303XX_CHECK_DIFF_ID(val)   \
            (((zl303xx_DiffIdE)(val) > ZL303XX_DIFF_ID_MAX) ?  \
            ZL303XX_PARAMETER_INVALID : \
            ZL303XX_OK)

/***************/

/* DiffOffset valid values */
typedef enum
{
   /* For SDH & Sync. Ethernet clocks, differential output alignment to the CMOS outputs
      is in steps of 1.6ns. */
   ZL303XX_DIFF_OFFSET_NONE = 0,
   ZL303XX_DIFF_OFFSET_1P6 = 1,
   ZL303XX_DIFF_OFFSET_NEG_1P6 = 2,
   ZL303XX_DIFF_OFFSET_NEG_3P2 = 3,

   /* For Ethernet clocks, coarse phase position tuning is used. */
   ZL303XX_DIFF_OFFSET_0 = 0,
   ZL303XX_DIFF_OFFSET_90 = 1,
   ZL303XX_DIFF_OFFSET_180 = 2,
   ZL303XX_DIFF_OFFSET_270 = 3
} zl303xx_DiffOffsetE;

/* Range limit definitions & parameter checking */
#define ZL303XX_DIFF_OFFSET_MIN   ZL303XX_DIFF_OFFSET_NONE
#define ZL303XX_DIFF_OFFSET_MAX   ZL303XX_DIFF_OFFSET_NEG_3P2

#define ZL303XX_CHECK_DIFF_OFFSET(val)   \
            (((zl303xx_DiffOffsetE)(val) > ZL303XX_DIFF_OFFSET_MAX) ?  \
            ZL303XX_PARAMETER_INVALID : \
            ZL303XX_OK)

/***************/

/* FbkPinState valid values */
typedef enum
{
   ZL303XX_FBK_PIN_REF8 = 0,
   ZL303XX_FBK_PIN_EXT_FBK = 1
} zl303xx_FbkPinStateE;

/* Range limit definitions & parameter checking */
#define ZL303XX_FBK_PIN_STATE_MIN   ZL303XX_FBK_PIN_REF8
#define ZL303XX_FBK_PIN_STATE_MAX   ZL303XX_FBK_PIN_EXT_FBK

#define ZL303XX_CHECK_FBK_PIN_STATE(val)   \
            (((zl303xx_FbkPinStateE)(val) > ZL303XX_FBK_PIN_STATE_MAX) ?  \
            ZL303XX_PARAMETER_INVALID : \
            ZL303XX_OK)

/***************/

/* CcFpSelect valid values */
typedef enum
{
   ZL303XX_CC_FP_SEL_NONE0 = 0,
   ZL303XX_CC_FP_SEL_8KHZ = 1,
   ZL303XX_CC_FP_SEL_400HZ = 2,
   ZL303XX_CC_FP_SEL_NONE3 = 3
} zl303xx_CcFpSelectE;

/* Range limit definitions & parameter checking */
#define ZL303XX_CC_FP_SELECT_MIN   ZL303XX_CC_FP_SEL_NONE0
#define ZL303XX_CC_FP_SELECT_MAX   ZL303XX_CC_FP_SEL_NONE3

#define ZL303XX_CHECK_CC_FP_SELECT(val)   \
            (((zl303xx_CcFpSelectE)(val) > ZL303XX_CC_FP_SELECT_MAX) ?  \
            ZL303XX_PARAMETER_INVALID : \
            ZL303XX_OK)

/***************/

/* CcFpMode valid values */
typedef enum
{
   ZL303XX_CC_FP_MODE_G378 = 0,
   ZL303XX_CC_FP_MODE_G703 = 1
} zl303xx_CcFpModeE;

/* Range limit definitions & parameter checking */
#define ZL303XX_CC_FP_MODE_MIN   ZL303XX_CC_FP_MODE_G378
#define ZL303XX_CC_FP_MODE_MAX   ZL303XX_CC_FP_MODE_G703

#define ZL303XX_CHECK_CC_FP_MODE(val)   \
            (((zl303xx_CcFpModeE)(val) > ZL303XX_CC_FP_MODE_MAX) ?  \
            ZL303XX_PARAMETER_INVALID : \
            ZL303XX_OK)

/***************/

/* BpvStatus valid values */
typedef enum
{
   ZL303XX_BPV_PASS = 0,
   ZL303XX_BPV_ERROR = 1
} zl303xx_BpvStatusE;

/* Range limit definitions & parameter checking */
#define ZL303XX_BPV_STATUS_MIN   ZL303XX_BPV_PASS
#define ZL303XX_BPV_STATUS_MAX   ZL303XX_BPV_ERROR

#define ZL303XX_CHECK_BPV_STATUS(val)   \
            (((zl303xx_BpvStatusE)(val) > ZL303XX_BPV_STATUS_MAX) ?  \
            ZL303XX_PARAMETER_INVALID : \
            ZL303XX_OK)

/***************/

/* CcDetMode valid values */
typedef enum
{
   ZL303XX_CC_DISABLED = 0,
   ZL303XX_CC_ENABLED = 1,
   ZL303XX_CC_AUTO_DETECT = 2
} zl303xx_CcDetModeE;

/* Range limit definitions & parameter checking */
#define ZL303XX_CC_DET_MODE_MIN   ZL303XX_CC_DISABLED
#define ZL303XX_CC_DET_MODE_MAX   ZL303XX_CC_AUTO_DETECT

#define ZL303XX_CHECK_CC_DET_MODE(val)   \
            (((zl303xx_CcDetModeE)(val) > ZL303XX_CC_DET_MODE_MAX) ?  \
            ZL303XX_PARAMETER_INVALID : \
            ZL303XX_OK)

/*****************   DATA STRUCTURES   ****************************************/

/********************************/
/* Structures for ISR registers */
/********************************/
typedef struct
{
   zl303xx_BooleanE     refFail;  /* Logical OR of all unmasked monitor bits */

   zl303xx_BooleanE     scmFail;  /*  \                 */
   zl303xx_BooleanE     cfmFail;  /*   \   Sticky bits  */
   zl303xx_BooleanE     gstFail;  /*   /                */
   zl303xx_BooleanE     pfmFail;  /*  /                 */

   zl303xx_RefIdE       Id;
} zl303xx_RefIsrStatusS;

typedef struct
{
   zl303xx_RefIdE       Id;
   zl303xx_BooleanE     refIsrEn;
   zl303xx_BooleanE     scmIsrEn;
   zl303xx_BooleanE     cfmIsrEn;
   zl303xx_BooleanE     pfmIsrEn;
   zl303xx_BooleanE     gstIsrEn;

   /* Inputs to the GST */
   zl303xx_BooleanE     gstScmIsrEn;
   zl303xx_BooleanE     gstCfmIsrEn;
} zl303xx_RefIsrConfigS;

typedef struct
{
   zl303xx_BooleanE     locked;
   zl303xx_BooleanE     lostLock;
   zl303xx_BooleanE     holdover;
   zl303xx_BooleanE     refChange;
   zl303xx_DpllIdE      Id;
} zl303xx_DpllIsrS;

typedef struct
{
   zl303xx_BooleanE     lockIsrEn;
   zl303xx_BooleanE     lostLockIsrEn;
   zl303xx_BooleanE     holdoverIsrEn;
   zl303xx_BooleanE     refChangeIsrEn;
   zl303xx_DpllIdE      Id;
} zl303xx_DpllIsrConfigS;

/********************************************/
/* Structures for reference input registers */
/********************************************/

typedef struct
{
   zl303xx_RefModeE     mode;
   zl303xx_BooleanE     invert;
   zl303xx_RefDivideE   prescaler;
   zl303xx_RefOorE      oorLimit;
   zl303xx_RefIdE       Id;
} zl303xx_RefConfigS;

typedef struct
{
   zl303xx_BooleanE     en1HzDetect;
   zl303xx_TtoDisQualE  ttDisqualify;
   zl303xx_TtoQualE     ttQualify;
} zl303xx_GlobalInConfigS;

typedef struct
{
   zl303xx_BooleanE     enable;
   zl303xx_BooleanE     invert;
   zl303xx_SyncIdE      Id;
} zl303xx_SyncConfigS;

typedef struct
{
   zl303xx_SyncFreqE    detectedFreq;
   zl303xx_BooleanE     failed;
   zl303xx_SyncIdE      Id;
} zl303xx_SyncStatusS;

/*********************************/
/* Structures for DPLL registers */
/*********************************/

typedef struct
{
   zl303xx_DpllHitlessE   hitlessSw;
   zl303xx_DpllBwE        bandwidth;
   zl303xx_DpllPslE       phaseSlope;
   zl303xx_BooleanE       revertEn;
   zl303xx_DpllHldUpdateE hldUpdateTime;
   zl303xx_DpllHldFilterE hldFilterBw;
   zl303xx_DpllModeE      mode;
   zl303xx_RefIdE         selectedRef;
   Uint32T              waitToRestore;  /* waitToRestore is in minutes */
   zl303xx_DpllPullInE    pullInRange;
   zl303xx_DpllIdE        Id;
   zl303xx_BooleanE       enable;
   zl303xx_BooleanE       dcoOffsetEnable;
   zl303xx_BooleanE       dcoFilterEnable;
} zl303xx_DpllConfigS;

typedef struct
{
   zl303xx_DpllHoldStateE holdover;
   zl303xx_DpllLockStateE locked;
   zl303xx_BooleanE       refFailed;
   zl303xx_DpllIdE        Id;
} zl303xx_DpllStatusS;

typedef struct
{
   zl303xx_BooleanE     scmRefSwEn;
   zl303xx_BooleanE     cfmRefSwEn;
   zl303xx_BooleanE     gstRefSwEn;
   zl303xx_BooleanE     pfmRefSwEn;
   zl303xx_BooleanE     scmHoldoverEn;
   zl303xx_BooleanE     cfmHoldoverEn;
   zl303xx_BooleanE     gstHoldoverEn;
   zl303xx_BooleanE     pfmHoldoverEn;
   zl303xx_DpllIdE      Id;
} zl303xx_DpllMaskConfigS;

typedef struct
{
   zl303xx_BooleanE     revertEn;
   Uint32T              priority;
   zl303xx_DpllIdE      Id;
   zl303xx_RefIdE       refId;
} zl303xx_DpllRefConfigS;

/***************************************/
/* Structures for clk output registers */
/***************************************/

typedef struct
{
   zl303xx_OutEnableE   enable;
   zl303xx_OutputRunE   run;
   Uint32T            freq;    /* freq is in kHz */
   zl303xx_ClkOffsetE   offset;
   zl303xx_ClockIdE     Id;
   zl303xx_SynthIdE     synthId;
} zl303xx_SynthClkConfigS;

typedef struct
{
   zl303xx_OutEnableE   enable;
   zl303xx_OutputRunE   run;
   zl303xx_FpFreqE      freq;
   zl303xx_FpStyleE     style;
   zl303xx_FpSyncEdgeE  syncEdge;
   zl303xx_FpTypePxE    type;
   zl303xx_FpPolarityE  polarity;
   Uint32T            offset;    /* offset is in nanoseconds */
   zl303xx_FpIdE        Id;
   zl303xx_SynthIdE     synthId;
} zl303xx_SynthFpConfigS;

typedef struct
{
   zl303xx_DpllIdE      source;
   zl303xx_BooleanE     enable;
   Sint32T            fineDelay;  /* fineDelay is in picoseconds */
   zl303xx_SynthIdE     Id;
} zl303xx_SynthConfigS;

/*******************************************/
/* Structures for SONET/SDH/APLL registers */
/*******************************************/

typedef struct
{
   zl303xx_OutEnableE   enable;
   zl303xx_OutputRunE   run;
   zl303xx_SdhClkFreqE  freq;
   zl303xx_ClkOffsetE   offset;
   zl303xx_ClockIdE     Id;
} zl303xx_SdhClkConfigS;

typedef struct
{
   zl303xx_OutEnableE   enable;
   zl303xx_OutputRunE   run;
   zl303xx_FpFreqE      freq;
   zl303xx_FpStyleE     style;
   zl303xx_FpSyncEdgeE  syncEdge;
   zl303xx_FpTypeSdhE   type;
   zl303xx_FpPolarityE  polarity;
   Uint32T            offset;    /* offset is in nanoseconds */
   zl303xx_FpIdE        Id;
} zl303xx_SdhFpConfigS;

typedef struct
{
   zl303xx_DpllIdE      source;
   zl303xx_BooleanE     enable;
   zl303xx_SdhCtrFreqE  centerFreq;
   Sint32T            fineDelay;  /* fineDelay is in picoseconds */
} zl303xx_SdhConfigS;

/************************************************/
/* Structures for differential output registers */
/************************************************/

typedef struct
{
   zl303xx_OutEnableE   enable;
   zl303xx_DiffRunE     run;
   zl303xx_DiffOffsetE  offset;
   zl303xx_DiffFreqE    freq;
   zl303xx_DiffIdE      Id;
} zl303xx_DiffConfigS;

/********************************************/
/* Structures for feedback output registers */
/********************************************/

typedef struct
{
   zl303xx_OutEnableE   outEnable;
   zl303xx_FbkPinStateE pinConfig;
   Sint32T            fineDelay;
   zl303xx_BooleanE     enable;
} zl303xx_FeedbackConfigS;

/*****************************************/
/* Structures for custom input registers */
/*****************************************/

typedef struct
{
   Uint32T            freq;     /* freq is in kHz */

   Uint32T            scmLo;    /*  \                         */
   Uint32T            scmHi;    /*   \ Monitor limits are in  */
   Uint32T            cfmLo;    /*   / nanoseconds.           */
   Uint32T            cfmHi;    /*  /                         */

   Uint32T            cycles;
   zl303xx_BooleanE     cfmDivBy4;
   zl303xx_CustIdE      Id;
} zl303xx_CustConfigS;

/********************************************/
/* Structures for composite clock registers */
/********************************************/

typedef struct
{
   zl303xx_CcDetModeE   detectMode;
   zl303xx_CcFpModeE    fpMode;
   zl303xx_CcFpSelectE  fpSelect;
   zl303xx_BooleanE     ccIsrEn;
   zl303xx_CompIdE      Id;
} zl303xx_CompConfigS;

typedef struct
{
   zl303xx_BooleanE     fp64kHzDet;
   zl303xx_BooleanE     fp8kHzDet;
   zl303xx_BooleanE     fp400HzDet;
   zl303xx_BpvStatusE   bpvStatus;   /* Sticky bit */
   zl303xx_CompIdE      Id;
} zl303xx_CompStatusS;

/****************/


#ifdef __cplusplus
}
#endif

#endif /* MULTIPLE INCLUDE BARRIER */
