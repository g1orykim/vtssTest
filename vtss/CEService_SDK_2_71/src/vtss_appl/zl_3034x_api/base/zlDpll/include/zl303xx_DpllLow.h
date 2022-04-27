

/*******************************************************************************
*
*  $Id: zl303xx_DpllLow.h 8253 2012-05-23 17:59:02Z PC $
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     Definitions for low-level DPLL attribute access
*
*******************************************************************************/

#ifndef ZL303XX_API_DPLL_LOW_H_
#define ZL303XX_API_DPLL_LOW_H_

#ifdef __cplusplus
extern "C" {
#endif

/*****************   INCLUDE FILES   ******************************************/

#include "zl303xx_ApiLow.h"
#include "zl303xx_ApiLowDataTypes.h"

/*****************   DEFINES   ************************************************/

/***************/

/* Control Register and Bitfield Attribute Definitions */
#define ZL303XX_DPLL1_CONTROL_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x1D, ZL303XX_MEM_SIZE_2_BYTE)
#define ZL303XX_DPLL2_CONTROL_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x2A, ZL303XX_MEM_SIZE_2_BYTE)
#define ZL303XX_DPLL_CONTROL_REG(dpllId)   \
                    (Uint32T)((dpllId == ZL303XX_DPLL_ID_1) ? \
                              (ZL303XX_DPLL1_CONTROL_REG) : \
                              (ZL303XX_DPLL2_CONTROL_REG))

/* Hitless Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_DPLL_HITLESS_MASK                    (Uint32T)(0x01)
#define ZL303XX_DPLL_HITLESS_SHIFT                   0

/* Bandwidth Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_DPLL_BANDWIDTH_MASK                  (Uint32T)(0x67)
#define ZL303XX_DPLL_NON_S3E_BANDWIDTH_MASK          (Uint32T)(0x07)
#define ZL303XX_DPLL_BANDWIDTH_SHIFT                 1

/* Phase Slope Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_DPLL_PHASE_LIMIT_MASK(dpllId)   \
                    (Uint32T)((dpllId == ZL303XX_DPLL_ID_1) ? \
                              (0x03) : \
                              (0x01))
#define ZL303XX_DPLL_PHASE_LIMIT_SHIFT               4

/* Revertive Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_DPLL_REVERTIVE_EN_MASK               (Uint32T)(0x01)
#define ZL303XX_DPLL_REVERTIVE_EN_SHIFT              8

/* DCO Offset Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_DPLL_DCO_OFFSET_EN_MASK              (Uint32T)(0x01)
#define ZL303XX_DPLL_DCO_OFFSET_EN_SHIFT             9

/* DCO Offset Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_DPLL_DCO_FILTER_EN_MASK              (Uint32T)(0x01)
#define ZL303XX_DPLL_DCO_FILTER_EN_SHIFT             10

/* Holdover Upd Time Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_DPLL_HOLD_UPDATE_TIME_MASK           (Uint32T)(0x03)
#define ZL303XX_DPLL_HOLD_UPDATE_TIME_SHIFT          12

/* Holdover Filter BW Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_DPLL_HOLD_FILTER_BW_MASK             (Uint32T)(0x03)
#define ZL303XX_DPLL_HOLD_FILTER_BW_SHIFT            14

/***************/

/* Mode Register and Bitfield Attribute Definitions */
#define ZL303XX_DPLL1_MODE_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x1F, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_DPLL2_MODE_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x2C, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_DPLL_MODE_REG(dpllId)   \
                    (Uint32T)((dpllId == ZL303XX_DPLL_ID_1) ? \
                              (ZL303XX_DPLL1_MODE_REG) : \
                              (ZL303XX_DPLL2_MODE_REG))

/* Mode Select Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_DPLL_MODE_SELECT_MASK                (Uint32T)(0x07)
#define ZL303XX_DPLL_MODE_SELECT_SHIFT               0

/***************/

/* Ref Select Register and Bitfield Attribute Definitions */
#define ZL303XX_DPLL1_REF_SELECT_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x20, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_DPLL2_REF_SELECT_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x2D, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_DPLL_REF_SELECT_REG(dpllId)   \
                    (Uint32T)((dpllId == ZL303XX_DPLL_ID_1) ? \
                              (ZL303XX_DPLL1_REF_SELECT_REG) : \
                              (ZL303XX_DPLL2_REF_SELECT_REG))

/* Ref Select Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_DPLL_REF_SELECT_MASK                 (Uint32T)(0x0F)
#define ZL303XX_DPLL_REF_SELECT_SHIFT                0

/***************/

/* Ref Fail Mask Register and Bitfield Attribute Definitions */
#define ZL303XX_DPLL1_REF_FAIL_MASK_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x21, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_DPLL2_REF_FAIL_MASK_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x2E, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_DPLL_REF_FAIL_MASK_REG(dpllId)   \
                    (Uint32T)((dpllId == ZL303XX_DPLL_ID_1) ? \
                              (ZL303XX_DPLL1_REF_FAIL_MASK_REG) : \
                              (ZL303XX_DPLL2_REF_FAIL_MASK_REG))

/* Ref Switch SCM Mask Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_DPLL_SCM_REF_SW_MASK_EN_MASK         (Uint32T)(0x01)
#define ZL303XX_DPLL_SCM_REF_SW_MASK_EN_SHIFT        0

/* Ref Switch CFM Mask Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_DPLL_CFM_REF_SW_MASK_EN_MASK         (Uint32T)(0x01)
#define ZL303XX_DPLL_CFM_REF_SW_MASK_EN_SHIFT        1

/* Ref Switch GST Mask Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_DPLL_GST_REF_SW_MASK_EN_MASK         (Uint32T)(0x01)
#define ZL303XX_DPLL_GST_REF_SW_MASK_EN_SHIFT        2

/* Ref Switch PFM Mask Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_DPLL_PFM_REF_SW_MASK_EN_MASK         (Uint32T)(0x01)
#define ZL303XX_DPLL_PFM_REF_SW_MASK_EN_SHIFT        3

/* Holdover SCM Mask Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_DPLL_SCM_HOLD_MASK_EN_MASK           (Uint32T)(0x01)
#define ZL303XX_DPLL_SCM_HOLD_MASK_EN_SHIFT          4

/* Holdover CFM Mask Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_DPLL_CFM_HOLD_MASK_EN_MASK           (Uint32T)(0x01)
#define ZL303XX_DPLL_CFM_HOLD_MASK_EN_SHIFT          5

/* Holdover GST Mask Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_DPLL_GST_HOLD_MASK_EN_MASK           (Uint32T)(0x01)
#define ZL303XX_DPLL_GST_HOLD_MASK_EN_SHIFT          6

/* Holdover PFM Mask Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_DPLL_PFM_HOLD_MASK_EN_MASK           (Uint32T)(0x01)
#define ZL303XX_DPLL_PFM_HOLD_MASK_EN_SHIFT          7

/***************/

/* Wait to Restore Register and Bitfield Attribute Definitions */
#define ZL303XX_DPLL1_WAIT_TO_RES_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x22, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_DPLL2_WAIT_TO_RES_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x2F, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_DPLL_WAIT_TO_RES_REG(dpllId)   \
                    (Uint32T)((dpllId == ZL303XX_DPLL_ID_1) ? \
                              (ZL303XX_DPLL1_WAIT_TO_RES_REG) : \
                              (ZL303XX_DPLL2_WAIT_TO_RES_REG))

/* Wait to Restore (min) Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_DPLL_WAIT_TO_RES_MASK                (Uint32T)(0x0F)
#define ZL303XX_DPLL_WAIT_TO_RES_SHIFT               0

#define ZL303XX_DPLL_WAIT_TO_RES_MAX   ZL303XX_DPLL_WAIT_TO_RES_MASK

#define ZL303XX_CHECK_WAIT_TO_RES(val)  \
            ((val > ZL303XX_DPLL_WAIT_TO_RES_MAX) ?  \
            ZL303XX_PARAMETER_INVALID :  \
            ZL303XX_OK)

/***************/

/* Ref Revert Ctrl Register and Bitfield Attribute Definitions */
#define ZL303XX_DPLL1_REF_REVERT0_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x23, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_DPLL1_REF_REVERT1_REG   \
                    ZL303XX_MAKE_ADDR(0x08, 0x6D, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_DPLL1_REF_REVERT_REG(refId)   \
                    (Uint32T)((refId < ZL303XX_REF_ID_8) ? \
                              (ZL303XX_DPLL1_REF_REVERT0_REG) : \
                              (ZL303XX_DPLL1_REF_REVERT1_REG))

#define ZL303XX_DPLL2_REF_REVERT0_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x30, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_DPLL2_REF_REVERT1_REG   \
                    ZL303XX_MAKE_ADDR(0x08, 0x6F, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_DPLL2_REF_REVERT_REG(refId)   \
                    (Uint32T)((refId < ZL303XX_REF_ID_8) ? \
                              (ZL303XX_DPLL2_REF_REVERT0_REG) : \
                              (ZL303XX_DPLL2_REF_REVERT1_REG))

#define ZL303XX_DPLL_REF_REVERT_REG(dpllId,refId)   \
                    (Uint32T)((dpllId == ZL303XX_DPLL_ID_1) ? \
                              (ZL303XX_DPLL1_REF_REVERT_REG(refId)) : \
                              (ZL303XX_DPLL2_REF_REVERT_REG(refId)))

/* Ref Rev En Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_DPLL_REF_REV_SWITCH_EN_MASK          (Uint32T)(0x01)
#define ZL303XX_DPLL_REF_REV_SWITCH_EN_SHIFT(refId)  (refId % 8)

/***************/

/* Ref Priority Register and Bitfield Attribute Definitions */
#define ZL303XX_DPLL1_REF_PRIORITY0_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x24, ZL303XX_MEM_SIZE_4_BYTE)
#define ZL303XX_DPLL1_REF_PRIORITY1_REG   \
                    ZL303XX_MAKE_ADDR(0x08, 0x6E, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_DPLL1_REF_PRIORITY_REG(refId)   \
                    (Uint32T)((refId < ZL303XX_REF_ID_8) ? \
                              (ZL303XX_DPLL1_REF_PRIORITY0_REG) : \
                              (ZL303XX_DPLL1_REF_PRIORITY1_REG))

#define ZL303XX_DPLL2_REF_PRIORITY0_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x31, ZL303XX_MEM_SIZE_4_BYTE)
#define ZL303XX_DPLL2_REF_PRIORITY1_REG   \
                    ZL303XX_MAKE_ADDR(0x08, 0x70, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_DPLL2_REF_PRIORITY_REG(refId)   \
                    (Uint32T)((refId < ZL303XX_REF_ID_8) ? \
                              (ZL303XX_DPLL2_REF_PRIORITY0_REG) : \
                              (ZL303XX_DPLL2_REF_PRIORITY1_REG))

#define ZL303XX_DPLL_REF_PRIORITY_REG(dpllId,refId)   \
                    (Uint32T)((dpllId == ZL303XX_DPLL_ID_1) ? \
                              (ZL303XX_DPLL1_REF_PRIORITY_REG(refId)) : \
                              (ZL303XX_DPLL2_REF_PRIORITY_REG(refId)))

/* Ref Priority Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_DPLL_REF_PRIORITY_MASK               (Uint32T)(0x0F)
#define ZL303XX_DPLL_REF_PRIORITY_SHIFT(refId)       ((refId % 8) * 4)

#define ZL303XX_DPLL_REF_PRIORITY_MAX   ZL303XX_DPLL_REF_PRIORITY_MASK

#define ZL303XX_CHECK_REF_PRIORITY(val)  \
            ((val > ZL303XX_DPLL_REF_PRIORITY_MAX) ?  \
            ZL303XX_PARAMETER_INVALID :  \
            ZL303XX_OK)

/***************/

/* Lock Status Register and Bitfield Attribute Definitions */
#define ZL303XX_DPLL1_LOCK_STATUS_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x28, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_DPLL2_LOCK_STATUS_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x35, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_DPLL_LOCK_STATUS_REG(dpllId)   \
                    (Uint32T)((dpllId == ZL303XX_DPLL_ID_1) ? \
                              (ZL303XX_DPLL1_LOCK_STATUS_REG) : \
                              (ZL303XX_DPLL2_LOCK_STATUS_REG))

/* Holdover Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_DPLL_HOLDOVER_STATUS_MASK            (Uint32T)(0x01)
#define ZL303XX_DPLL_HOLDOVER_STATUS_SHIFT           0

/* Lock Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_DPLL_LOCK_STATUS_MASK                (Uint32T)(0x01)
#define ZL303XX_DPLL_LOCK_STATUS_SHIFT               1

/* Cur Ref Fail Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_DPLL_CUR_REF_FAIL_STATUS_MASK        (Uint32T)(0x01)
#define ZL303XX_DPLL_CUR_REF_FAIL_STATUS_SHIFT       2

/***************/

/* Pull-in Range Register and Bitfield Attribute Definitions */
#define ZL303XX_DPLL1_PULLIN_RANGE_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x29, ZL303XX_MEM_SIZE_1_BYTE)

/* Pull-in Range Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_DPLL_PULLIN_RANGE_MASK               (Uint32T)(0x03)
#define ZL303XX_DPLL_PULLIN_RANGE_SHIFT              0

/* Enable Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_DPLL_ENABLE_MASK                     (Uint32T)(0x01)
#define ZL303XX_DPLL_ENABLE_SHIFT                    7


/*****************   DATA TYPES   *********************************************/
#ifndef _ZL303XX_APR_H /* define only if the zl303xx_Apr.h has not been included previously */
typedef enum
{
    ZL303XX_LOCK_STATUS_ACQUIRING = 0,
    ZL303XX_LOCK_STATUS_LOCKED,
    ZL303XX_LOCK_STATUS_PHASE_LOCKED,
    ZL303XX_LOCK_STATUS_HOLDOVER,
    ZL303XX_LOCK_STATUS_REF_FAILED,
    ZL303XX_LOCK_STATUS_UNKNOWN
} zl303xx_AprLockStatusE;
#endif


/*****************   DATA STRUCTURES   ****************************************/

/*****************   EXTERNAL FUNCTION DECLARATIONS   *************************/

/* Function Declarations for the zl303xx_DpllConfigS type */
zlStatusE zl303xx_DpllConfigStructInit(zl303xx_ParamsS *zl303xx_Params,
                                       zl303xx_DpllConfigS *par);
zlStatusE zl303xx_DpllConfigCheck(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_DpllConfigS *par);
zlStatusE zl303xx_DpllConfigGet(zl303xx_ParamsS *zl303xx_Params,
                                zl303xx_DpllConfigS *par);
zlStatusE zl303xx_DpllConfigSet(zl303xx_ParamsS *zl303xx_Params,
                                zl303xx_DpllConfigS *par);

/* Function Declarations for the zl303xx_DpllStatusS type */
zlStatusE zl303xx_DpllStatusStructInit(zl303xx_ParamsS *zl303xx_Params,
                                       zl303xx_DpllStatusS *par);
zlStatusE zl303xx_DpllStatusCheck(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_DpllStatusS *par);
zlStatusE zl303xx_DpllStatusGet(zl303xx_ParamsS *zl303xx_Params,
                                zl303xx_DpllStatusS *par);

/* Function Declarations for the zl303xx_DpllMaskConfigS type */
zlStatusE zl303xx_DpllMaskConfigStructInit(zl303xx_ParamsS *zl303xx_Params,
                                           zl303xx_DpllMaskConfigS *par);
zlStatusE zl303xx_DpllMaskConfigCheck(zl303xx_ParamsS *zl303xx_Params,
                                      zl303xx_DpllMaskConfigS *par);
zlStatusE zl303xx_DpllMaskConfigGet(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_DpllMaskConfigS *par);
zlStatusE zl303xx_DpllMaskConfigSet(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_DpllMaskConfigS *par);

/* Function Declarations for the zl303xx_DpllRefConfigS type */
zlStatusE zl303xx_DpllRefConfigStructInit(zl303xx_ParamsS *zl303xx_Params,
                                          zl303xx_DpllRefConfigS *par);
zlStatusE zl303xx_DpllRefConfigCheck(zl303xx_ParamsS *zl303xx_Params,
                                     zl303xx_DpllRefConfigS *par);
zlStatusE zl303xx_DpllRefConfigGet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_DpllRefConfigS *par);
zlStatusE zl303xx_DpllRefConfigSet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_DpllRefConfigS *par);

/* Hitless Access */
zlStatusE zl303xx_DpllHitlessGet(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_DpllIdE dpllId,
                                 zl303xx_DpllHitlessE *val);
zlStatusE zl303xx_DpllHitlessSet(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_DpllIdE dpllId,
                                 zl303xx_DpllHitlessE val);

/* Bandwidth Access */
zlStatusE zl303xx_DpllBandwidthGet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_DpllIdE dpllId,
                                   zl303xx_DpllBwE *val);
zlStatusE zl303xx_DpllBandwidthSet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_DpllIdE dpllId,
                                   zl303xx_DpllBwE val);

/* Phase Slope Access */
zlStatusE zl303xx_DpllPhaseLimitGet(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_DpllIdE dpllId,
                                    zl303xx_DpllPslE *val);
zlStatusE zl303xx_DpllPhaseLimitSet(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_DpllIdE dpllId,
                                    zl303xx_DpllPslE val);

/* Revertive Access */
zlStatusE zl303xx_DpllRevertiveEnGet(zl303xx_ParamsS *zl303xx_Params,
                                     zl303xx_DpllIdE dpllId,
                                     zl303xx_BooleanE *val);
zlStatusE zl303xx_DpllRevertiveEnSet(zl303xx_ParamsS *zl303xx_Params,
                                     zl303xx_DpllIdE dpllId,
                                     zl303xx_BooleanE val);

/* Holdover Upd Time Access */
zlStatusE zl303xx_DpllHoldUpdateTimeGet(zl303xx_ParamsS *zl303xx_Params,
                                        zl303xx_DpllIdE dpllId,
                                        zl303xx_DpllHldUpdateE *val);
zlStatusE zl303xx_DpllHoldUpdateTimeSet(zl303xx_ParamsS *zl303xx_Params,
                                        zl303xx_DpllIdE dpllId,
                                        zl303xx_DpllHldUpdateE val);

/* Holdover Filter BW Access */
zlStatusE zl303xx_DpllHoldFilterBwGet(zl303xx_ParamsS *zl303xx_Params,
                                      zl303xx_DpllIdE dpllId,
                                      zl303xx_DpllHldFilterE *val);
zlStatusE zl303xx_DpllHoldFilterBwSet(zl303xx_ParamsS *zl303xx_Params,
                                      zl303xx_DpllIdE dpllId,
                                      zl303xx_DpllHldFilterE val);

/* Mode Select Access */
zlStatusE zl303xx_DpllModeSelectGet(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_DpllIdE dpllId,
                                    zl303xx_DpllModeE *val);
zlStatusE zl303xx_DpllModeSelectSet(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_DpllIdE dpllId,
                                    zl303xx_DpllModeE val);

/* Ref Select Access */
zlStatusE zl303xx_DpllRefSelectGet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_DpllIdE dpllId,
                                   zl303xx_RefIdE *val);
zlStatusE zl303xx_DpllRefSelectSet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_DpllIdE dpllId,
                                   zl303xx_RefIdE val);

/* Ref Switch SCM Mask Access */
zlStatusE zl303xx_DpllScmRefSwMaskEnGet(zl303xx_ParamsS *zl303xx_Params,
                                        zl303xx_DpllIdE dpllId,
                                        zl303xx_BooleanE *val);
zlStatusE zl303xx_DpllScmRefSwMaskEnSet(zl303xx_ParamsS *zl303xx_Params,
                                        zl303xx_DpllIdE dpllId,
                                        zl303xx_BooleanE val);

/* Ref Switch CFM Mask Access */
zlStatusE zl303xx_DpllCfmRefSwMaskEnGet(zl303xx_ParamsS *zl303xx_Params,
                                        zl303xx_DpllIdE dpllId,
                                        zl303xx_BooleanE *val);
zlStatusE zl303xx_DpllCfmRefSwMaskEnSet(zl303xx_ParamsS *zl303xx_Params,
                                        zl303xx_DpllIdE dpllId,
                                        zl303xx_BooleanE val);

/* Ref Switch GST Mask Access */
zlStatusE zl303xx_DpllGstRefSwMaskEnGet(zl303xx_ParamsS *zl303xx_Params,
                                        zl303xx_DpllIdE dpllId,
                                        zl303xx_BooleanE *val);
zlStatusE zl303xx_DpllGstRefSwMaskEnSet(zl303xx_ParamsS *zl303xx_Params,
                                        zl303xx_DpllIdE dpllId,
                                        zl303xx_BooleanE val);

/* Ref Switch PFM Mask Access */
zlStatusE zl303xx_DpllPfmRefSwMaskEnGet(zl303xx_ParamsS *zl303xx_Params,
                                        zl303xx_DpllIdE dpllId,
                                        zl303xx_BooleanE *val);
zlStatusE zl303xx_DpllPfmRefSwMaskEnSet(zl303xx_ParamsS *zl303xx_Params,
                                        zl303xx_DpllIdE dpllId,
                                        zl303xx_BooleanE val);

/* Holdover SCM Mask Access */
zlStatusE zl303xx_DpllScmHoldMaskEnGet(zl303xx_ParamsS *zl303xx_Params,
                                       zl303xx_DpllIdE dpllId,
                                       zl303xx_BooleanE *val);
zlStatusE zl303xx_DpllScmHoldMaskEnSet(zl303xx_ParamsS *zl303xx_Params,
                                       zl303xx_DpllIdE dpllId,
                                       zl303xx_BooleanE val);

/* Holdover CFM Mask Access */
zlStatusE zl303xx_DpllCfmHoldMaskEnGet(zl303xx_ParamsS *zl303xx_Params,
                                       zl303xx_DpllIdE dpllId,
                                       zl303xx_BooleanE *val);
zlStatusE zl303xx_DpllCfmHoldMaskEnSet(zl303xx_ParamsS *zl303xx_Params,
                                       zl303xx_DpllIdE dpllId,
                                       zl303xx_BooleanE val);

/* Holdover GST Mask Access */
zlStatusE zl303xx_DpllGstHoldMaskEnGet(zl303xx_ParamsS *zl303xx_Params,
                                       zl303xx_DpllIdE dpllId,
                                       zl303xx_BooleanE *val);
zlStatusE zl303xx_DpllGstHoldMaskEnSet(zl303xx_ParamsS *zl303xx_Params,
                                       zl303xx_DpllIdE dpllId,
                                       zl303xx_BooleanE val);

/* Holdover PFM Mask Access */
zlStatusE zl303xx_DpllPfmHoldMaskEnGet(zl303xx_ParamsS *zl303xx_Params,
                                       zl303xx_DpllIdE dpllId,
                                       zl303xx_BooleanE *val);
zlStatusE zl303xx_DpllPfmHoldMaskEnSet(zl303xx_ParamsS *zl303xx_Params,
                                       zl303xx_DpllIdE dpllId,
                                       zl303xx_BooleanE val);

/* Wait to Restore (min) Access */
zlStatusE zl303xx_DpllWaitToResGet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_DpllIdE dpllId,
                                   Uint32T *val);
zlStatusE zl303xx_DpllWaitToResSet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_DpllIdE dpllId,
                                   Uint32T val);

/* Ref Rev En Access */
zlStatusE zl303xx_DpllRefRevSwitchEnGet(zl303xx_ParamsS *zl303xx_Params,
                                        zl303xx_DpllIdE dpllId,
                                        zl303xx_RefIdE refId,
                                        zl303xx_BooleanE *val);
zlStatusE zl303xx_DpllRefRevSwitchEnSet(zl303xx_ParamsS *zl303xx_Params,
                                        zl303xx_DpllIdE dpllId,
                                        zl303xx_RefIdE refId,
                                        zl303xx_BooleanE val);

/* Ref Priority Access */
zlStatusE zl303xx_DpllRefPriorityGet(zl303xx_ParamsS *zl303xx_Params,
                                     zl303xx_DpllIdE dpllId,
                                     zl303xx_RefIdE refId,
                                     Uint32T *val);
zlStatusE zl303xx_DpllRefPrioritySet(zl303xx_ParamsS *zl303xx_Params,
                                     zl303xx_DpllIdE dpllId,
                                     zl303xx_RefIdE refId,
                                     Uint32T val);

/* Holdover Access */
zlStatusE zl303xx_DpllHoldoverStatusGet(zl303xx_ParamsS *zl303xx_Params,
                                        zl303xx_DpllIdE dpllId,
                                        zl303xx_DpllHoldStateE *val);

/* Lock Access */
zlStatusE zl303xx_DpllLockStatusGet(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_DpllIdE dpllId,
                                    zl303xx_DpllLockStateE *val);

/* Cur Ref Fail Access */
zlStatusE zl303xx_DpllCurRefFailStatusGet(zl303xx_ParamsS *zl303xx_Params,
                                          zl303xx_DpllIdE dpllId,
                                          zl303xx_BooleanE *val);

/* Pull-in Range Access */
zlStatusE zl303xx_DpllPullinRangeGet(zl303xx_ParamsS *zl303xx_Params,
                                     zl303xx_DpllIdE dpllId,
                                     zl303xx_DpllPullInE *val);
zlStatusE zl303xx_DpllPullinRangeSet(zl303xx_ParamsS *zl303xx_Params,
                                     zl303xx_DpllIdE dpllId,
                                     zl303xx_DpllPullInE val);

/* Enable Access */
zlStatusE zl303xx_DpllEnableGet(zl303xx_ParamsS *zl303xx_Params,
                                zl303xx_DpllIdE dpllId,
                                zl303xx_BooleanE *val);
zlStatusE zl303xx_DpllEnableSet(zl303xx_ParamsS *zl303xx_Params,
                                zl303xx_DpllIdE dpllId,
                                zl303xx_BooleanE val);

/* DCO Offset Enable */
zlStatusE zl303xx_DpllDcoOffsetEnGet(zl303xx_ParamsS *zl303xx_Params,
                                     zl303xx_DpllIdE dpllId,
                                     zl303xx_BooleanE *val);

zlStatusE zl303xx_DpllDcoOffsetEnSet(zl303xx_ParamsS *zl303xx_Params,
                                     zl303xx_DpllIdE dpllId,
                                     zl303xx_BooleanE val);

/* DCO Filter Enable */
zlStatusE zl303xx_DpllDcoFilterEnGet(zl303xx_ParamsS *zl303xx_Params,
                                     zl303xx_DpllIdE dpllId,
                                     zl303xx_BooleanE *val);

zlStatusE zl303xx_DpllDcoFilterEnSet(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_DpllIdE dpllId,
                                  zl303xx_BooleanE val);

/* APR porting functions. */
Sint32T zl303xx_DpllGetHwLockStatus(void *hwParams, Sint32T *lockStatus);
Sint32T zl303xx_DpllTakeHwDcoControl(void *hwParams);
Sint32T zl303xx_DpllReturnHwDcoControl(void *hwParams);
Sint32T zl303xx_DpllGetHwManualHoldoverStatus(void *hwParams, Sint32T *status);
Sint32T zl303xx_DpllGetHwManualFreerunStatus(void *hwParams, Sint32T *status);
Sint32T zl303xx_DpllGetHwSyncInputEnStatus(void *hwParams, Sint32T *en);
Sint32T zl303xx_DpllGetHwOutOfRangeStatus(void *hwParams, Sint32T *oor);

#ifdef __cplusplus
}
#endif

#endif /* MULTIPLE INCLUDE BARRIER */

