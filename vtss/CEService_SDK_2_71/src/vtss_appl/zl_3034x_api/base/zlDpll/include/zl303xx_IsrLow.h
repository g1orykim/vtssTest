

/*******************************************************************************
*
*  $Id: zl303xx_IsrLow.h 6058 2011-06-10 14:37:16Z AW $
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     Definitions for low-level ISR attribute access
*
*******************************************************************************/

#ifndef ZL303XX_API_ISR_LOW_H_
#define ZL303XX_API_ISR_LOW_H_

#ifdef __cplusplus
extern "C" {
#endif

/*****************   INCLUDE FILES   ******************************************/

#include "zl303xx_ApiLow.h"
#include "zl303xx_ApiLowDataTypes.h"

/*****************   DEFINES   ************************************************/

/***************/

/* Ref Status Register and Bitfield Attribute Definitions */
#define ZL303XX_ISR_REF_STATE0_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x02, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_ISR_REF_STATE1_REG   \
                    ZL303XX_MAKE_ADDR(0x0F, 0x65, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_ISR_REF_STATE_REG(refId)   \
                    (Uint32T)((refId < ZL303XX_REF_ID_8) ? \
                              (ZL303XX_ISR_REF_STATE0_REG) : \
                              (ZL303XX_ISR_REF_STATE1_REG))


/* Ref Fail Status Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_ISR_REF_FAIL_STATUS_MASK             (Uint32T)(0x01)
#define ZL303XX_ISR_REF_FAIL_STATUS_SHIFT(refId)     (refId % 8)

/***************/

/* DPLL Status Register and Bitfield Attribute Definitions */
#define ZL303XX_ISR_DPLL_STATE0_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x03, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_ISR_DPLL_STATE1_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x04, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_ISR_DPLL_STATE_REG(dpllId)   \
                    (Uint32T)((dpllId < ZL303XX_DPLL_ID_2) ? \
                              (ZL303XX_ISR_DPLL_STATE0_REG) : \
                              (ZL303XX_ISR_DPLL_STATE1_REG))


/* Dpll Lock Status Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_ISR_DPLL_LOCKED_STATUS_MASK          (Uint32T)(0x01)
#define ZL303XX_ISR_DPLL_LOCKED_STATUS_SHIFT         0

/* Dpll Lost Lock Status Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_ISR_DPLL_LOST_LOCK_STATUS_MASK       (Uint32T)(0x01)
#define ZL303XX_ISR_DPLL_LOST_LOCK_STATUS_SHIFT      1

/* Dpll Holdover Status Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_ISR_DPLL_HOLDOVER_STATUS_MASK        (Uint32T)(0x01)
#define ZL303XX_ISR_DPLL_HOLDOVER_STATUS_SHIFT       2

/* Dpll Ref Chnge Status Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_ISR_DPLL_REF_CHGD_STATUS_MASK        (Uint32T)(0x01)
#define ZL303XX_ISR_DPLL_REF_CHGD_STATUS_SHIFT       3

/***************/

/* Sync Fail Status Register and Bitfield Attribute Definitions */
#define ZL303XX_ISR_SYNC_FAIL_STATUS0_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x03, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_ISR_SYNC_FAIL_STATUS1_REG   \
                    ZL303XX_MAKE_ADDR(0x0F, 0x65, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_ISR_SYNC_FAIL_STATUS_REG(syncId)   \
                    (Uint32T)((syncId < ZL303XX_SYNC_ID_8) ? \
                              (ZL303XX_ISR_SYNC_FAIL_STATUS0_REG) : \
                              (ZL303XX_ISR_SYNC_FAIL_STATUS1_REG))


/* Sync Fail Status Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_ISR_SYNC_FAIL_STATUS_MASK            (Uint32T)(0x01)
#define ZL303XX_ISR_SYNC_FAIL_STATUS_SHIFT(syncId)   ((syncId % 4) + 4)

/***************/

/* Ref Monitors Register and Bitfield Attribute Definitions */
#define ZL303XX_ISR_MON_STATE0_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x05, ZL303XX_MEM_SIZE_4_BYTE)
#define ZL303XX_ISR_MON_STATE1_REG   \
                    ZL303XX_MAKE_ADDR(0x0F, 0x66, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_ISR_MON_STATE_REG(refId)   \
                    (Uint32T)((refId < ZL303XX_REF_ID_8) ? \
                              (ZL303XX_ISR_MON_STATE0_REG) : \
                              (ZL303XX_ISR_MON_STATE1_REG))


/* SCM Fail Status Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_ISR_SCM_FAIL_STATUS_MASK             (Uint32T)(0x01)
#define ZL303XX_ISR_SCM_FAIL_STATUS_SHIFT(refId)     ((refId % 8) * 4)

/* CFM Fail Status Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_ISR_CFM_FAIL_STATUS_MASK             (Uint32T)(0x01)
#define ZL303XX_ISR_CFM_FAIL_STATUS_SHIFT(refId)     (((refId % 8) * 4) + 1)

/* GST Fail Status Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_ISR_GST_FAIL_STATUS_MASK             (Uint32T)(0x01)
#define ZL303XX_ISR_GST_FAIL_STATUS_SHIFT(refId)     (((refId % 8) * 4) + 2)

/* PFM Fail Status Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_ISR_PFM_FAIL_STATUS_MASK             (Uint32T)(0x01)
#define ZL303XX_ISR_PFM_FAIL_STATUS_SHIFT(refId)     (((refId % 8) * 4) + 3)

/***************/

/* Ref ISR Enable Register and Bitfield Attribute Definitions */
#define ZL303XX_ISR_REF_MASK0_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x09, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_ISR_REF_MASK1_REG   \
                    ZL303XX_MAKE_ADDR(0x0F, 0x67, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_ISR_REF_MASK_REG(refId)   \
                    (Uint32T)((refId < ZL303XX_REF_ID_8) ? \
                              (ZL303XX_ISR_REF_MASK0_REG) : \
                              (ZL303XX_ISR_REF_MASK1_REG))


/* Ref Fail Mask Enable Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_ISR_REF_FAIL_MASK_EN_MASK            (Uint32T)(0x01)
#define ZL303XX_ISR_REF_FAIL_MASK_EN_SHIFT(refId)    (refId % 8)

/***************/

/* DPLL ISR Enable Register and Bitfield Attribute Definitions */
#define ZL303XX_ISR_DPLL_MASK0_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x0A, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_ISR_DPLL_MASK1_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x0B, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_ISR_DPLL_MASK_REG(dpllId)   \
                    (Uint32T)((dpllId < ZL303XX_DPLL_ID_2) ? \
                              (ZL303XX_ISR_DPLL_MASK0_REG) : \
                              (ZL303XX_ISR_DPLL_MASK1_REG))


/* Dpll Lock Mask En Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_ISR_DPLL_LOCKED_MASK_EN_MASK         (Uint32T)(0x01)
#define ZL303XX_ISR_DPLL_LOCKED_MASK_EN_SHIFT        0

/* Dpll Lost Lock Mask En Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_ISR_DPLL_LOST_LOCK_MASK_EN_MASK      (Uint32T)(0x01)
#define ZL303XX_ISR_DPLL_LOST_LOCK_MASK_EN_SHIFT     1

/* Dpll Holdover Mask En Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_ISR_DPLL_HOLDOVER_MASK_EN_MASK       (Uint32T)(0x01)
#define ZL303XX_ISR_DPLL_HOLDOVER_MASK_EN_SHIFT      2

/* Dpll Ref Chnge Mask En Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_ISR_DPLL_REF_CHGD_MASK_EN_MASK       (Uint32T)(0x01)
#define ZL303XX_ISR_DPLL_REF_CHGD_MASK_EN_SHIFT      3

/***************/

/* Sync Fail Mask En Register and Bitfield Attribute Definitions */
#define ZL303XX_ISR_SYNC_FAIL_MASK_EN0_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x0A, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_ISR_SYNC_FAIL_MASK_EN1_REG   \
                    ZL303XX_MAKE_ADDR(0x0F, 0x67, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_ISR_SYNC_FAIL_MASK_EN_REG(syncId)   \
                    (Uint32T)((syncId < ZL303XX_SYNC_ID_8) ? \
                              (ZL303XX_ISR_SYNC_FAIL_MASK_EN0_REG) : \
                              (ZL303XX_ISR_SYNC_FAIL_MASK_EN1_REG))


/* Sync Fail Mask En Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_ISR_SYNC_FAIL_MASK_EN_MASK           (Uint32T)(0x01)
#define ZL303XX_ISR_SYNC_FAIL_MASK_EN_SHIFT(syncId)  ((syncId % 4) + 4)

/***************/

/* Ref Mon Mask Register and Bitfield Attribute Definitions */
#define ZL303XX_ISR_MON_MASK0_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x0C, ZL303XX_MEM_SIZE_4_BYTE)
#define ZL303XX_ISR_MON_MASK1_REG   \
                    ZL303XX_MAKE_ADDR(0x0F, 0x68, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_ISR_MON_MASK_REG(refId)   \
                    (Uint32T)((refId < ZL303XX_REF_ID_8) ? \
                              (ZL303XX_ISR_MON_MASK0_REG) : \
                              (ZL303XX_ISR_MON_MASK1_REG))


/* SCM Fail Mask Enable Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_ISR_SCM_FAIL_MASK_EN_MASK            (Uint32T)(0x01)
#define ZL303XX_ISR_SCM_FAIL_MASK_EN_SHIFT(refId)    ((refId % 8) * 4)

/* CFM Fail Mask Enable Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_ISR_CFM_FAIL_MASK_EN_MASK            (Uint32T)(0x01)
#define ZL303XX_ISR_CFM_FAIL_MASK_EN_SHIFT(refId)    (((refId % 8) * 4) + 1)

/* GST Fail Mask Enable Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_ISR_GST_FAIL_MASK_EN_MASK            (Uint32T)(0x01)
#define ZL303XX_ISR_GST_FAIL_MASK_EN_SHIFT(refId)    (((refId % 8) * 4) + 2)

/* PFM Fail Mask Enable Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_ISR_PFM_FAIL_MASK_EN_MASK            (Uint32T)(0x01)
#define ZL303XX_ISR_PFM_FAIL_MASK_EN_SHIFT(refId)    (((refId % 8) * 4) + 3)

/***************/

/* Ref GST Triggers Register and Bitfield Attribute Definitions */
#define ZL303XX_ISR_GST_MASK0_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x1A, ZL303XX_MEM_SIZE_2_BYTE)
#define ZL303XX_ISR_GST_MASK1_REG   \
                    ZL303XX_MAKE_ADDR(0x08, 0x6B, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_ISR_GST_MASK_REG(refId)   \
                    (Uint32T)((refId < ZL303XX_REF_ID_8) ? \
                              (ZL303XX_ISR_GST_MASK0_REG) : \
                              (ZL303XX_ISR_GST_MASK1_REG))


/* GST-SCM Mask En Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_ISR_GST_SCM_MASK_EN_MASK             (Uint32T)(0x01)
#define ZL303XX_ISR_GST_SCM_MASK_EN_SHIFT(refId)     ((refId % 8) * 2)

/* GST-CFM Mask En Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_ISR_GST_CFM_MASK_EN_MASK             (Uint32T)(0x01)
#define ZL303XX_ISR_GST_CFM_MASK_EN_SHIFT(refId)     (((refId % 8) * 2) + 1)

/*****************   DATA TYPES   *********************************************/

/*****************   DATA STRUCTURES   ****************************************/

/*****************   EXTERNAL FUNCTION DECLARATIONS   *************************/

/* Function Declarations for the zl303xx_RefIsrStatusS type */
zlStatusE zl303xx_RefIsrStatusStructInit(zl303xx_ParamsS *zl303xx_Params,
                                         zl303xx_RefIsrStatusS *par);
zlStatusE zl303xx_RefIsrStatusCheck(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_RefIsrStatusS *par);
zlStatusE zl303xx_RefIsrStatusGet(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_RefIsrStatusS *par);

/* Function Declarations for the zl303xx_RefIsrConfigS type */
zlStatusE zl303xx_RefIsrConfigStructInit(zl303xx_ParamsS *zl303xx_Params,
                                         zl303xx_RefIsrConfigS *par);
zlStatusE zl303xx_RefIsrConfigCheck(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_RefIsrConfigS *par);
zlStatusE zl303xx_RefIsrConfigGet(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_RefIsrConfigS *par);
zlStatusE zl303xx_RefIsrConfigSet(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_RefIsrConfigS *par);

/* Function Declarations for the zl303xx_DpllIsrS type */
zlStatusE zl303xx_DpllIsrStructInit(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_DpllIsrS *par);
zlStatusE zl303xx_DpllIsrCheck(zl303xx_ParamsS *zl303xx_Params,
                               zl303xx_DpllIsrS *par);
zlStatusE zl303xx_DpllIsrGet(zl303xx_ParamsS *zl303xx_Params,
                             zl303xx_DpllIsrS *par);

/* Function Declarations for the zl303xx_DpllIsrConfigS type */
zlStatusE zl303xx_DpllIsrConfigStructInit(zl303xx_ParamsS *zl303xx_Params,
                                          zl303xx_DpllIsrConfigS *par);
zlStatusE zl303xx_DpllIsrConfigCheck(zl303xx_ParamsS *zl303xx_Params,
                                     zl303xx_DpllIsrConfigS *par);
zlStatusE zl303xx_DpllIsrConfigGet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_DpllIsrConfigS *par);
zlStatusE zl303xx_DpllIsrConfigSet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_DpllIsrConfigS *par);


/* Ref Fail Status Access */
zlStatusE zl303xx_IsrRefFailStatusGet(zl303xx_ParamsS *zl303xx_Params,
                                      zl303xx_RefIdE refId,
                                      zl303xx_BooleanE *val);

/* Dpll Lock Status Access */
zlStatusE zl303xx_IsrDpllLockedStatusGet(zl303xx_ParamsS *zl303xx_Params,
                                         zl303xx_DpllIdE dpllId,
                                         zl303xx_BooleanE *val);

/* Dpll Lost Lock Status Access */
zlStatusE zl303xx_IsrDpllLostLockStatusGet(zl303xx_ParamsS *zl303xx_Params,
                                           zl303xx_DpllIdE dpllId,
                                           zl303xx_BooleanE *val);

/* Dpll Holdover Status Access */
zlStatusE zl303xx_IsrDpllHoldoverStatusGet(zl303xx_ParamsS *zl303xx_Params,
                                           zl303xx_DpllIdE dpllId,
                                           zl303xx_BooleanE *val);

/* Dpll Ref Chnge Status Access */
zlStatusE zl303xx_IsrDpllRefChgdStatusGet(zl303xx_ParamsS *zl303xx_Params,
                                          zl303xx_DpllIdE dpllId,
                                          zl303xx_BooleanE *val);

/* Sync Fail Status Access */
zlStatusE zl303xx_IsrSyncFailStatusGet(zl303xx_ParamsS *zl303xx_Params,
                                       zl303xx_SyncIdE syncId,
                                       zl303xx_BooleanE *val);

/* SCM Fail Status Access */
zlStatusE zl303xx_IsrScmFailStatusGet(zl303xx_ParamsS *zl303xx_Params,
                                      zl303xx_RefIdE refId,
                                      zl303xx_BooleanE *val);

/* CFM Fail Status Access */
zlStatusE zl303xx_IsrCfmFailStatusGet(zl303xx_ParamsS *zl303xx_Params,
                                      zl303xx_RefIdE refId,
                                      zl303xx_BooleanE *val);

/* GST Fail Status Access */
zlStatusE zl303xx_IsrGstFailStatusGet(zl303xx_ParamsS *zl303xx_Params,
                                      zl303xx_RefIdE refId,
                                      zl303xx_BooleanE *val);

/* PFM Fail Status Access */
zlStatusE zl303xx_IsrPfmFailStatusGet(zl303xx_ParamsS *zl303xx_Params,
                                      zl303xx_RefIdE refId,
                                      zl303xx_BooleanE *val);

/* Ref Fail Mask Enable Access */
zlStatusE zl303xx_IsrRefFailMaskEnGet(zl303xx_ParamsS *zl303xx_Params,
                                      zl303xx_RefIdE refId,
                                      zl303xx_BooleanE *val);
zlStatusE zl303xx_IsrRefFailMaskEnSet(zl303xx_ParamsS *zl303xx_Params,
                                      zl303xx_RefIdE refId,
                                      zl303xx_BooleanE val);

/* Dpll Lock Mask En Access */
zlStatusE zl303xx_IsrDpllLockedMaskEnGet(zl303xx_ParamsS *zl303xx_Params,
                                         zl303xx_DpllIdE dpllId,
                                         zl303xx_BooleanE *val);
zlStatusE zl303xx_IsrDpllLockedMaskEnSet(zl303xx_ParamsS *zl303xx_Params,
                                         zl303xx_DpllIdE dpllId,
                                         zl303xx_BooleanE val);

/* Dpll Lost Lock Mask En Access */
zlStatusE zl303xx_IsrDpllLostLockMaskEnGet(zl303xx_ParamsS *zl303xx_Params,
                                           zl303xx_DpllIdE dpllId,
                                           zl303xx_BooleanE *val);
zlStatusE zl303xx_IsrDpllLostLockMaskEnSet(zl303xx_ParamsS *zl303xx_Params,
                                           zl303xx_DpllIdE dpllId,
                                           zl303xx_BooleanE val);

/* Dpll Holdover Mask En Access */
zlStatusE zl303xx_IsrDpllHoldoverMaskEnGet(zl303xx_ParamsS *zl303xx_Params,
                                           zl303xx_DpllIdE dpllId,
                                           zl303xx_BooleanE *val);
zlStatusE zl303xx_IsrDpllHoldoverMaskEnSet(zl303xx_ParamsS *zl303xx_Params,
                                           zl303xx_DpllIdE dpllId,
                                           zl303xx_BooleanE val);

/* Dpll Ref Chnge Mask En Access */
zlStatusE zl303xx_IsrDpllRefChgdMaskEnGet(zl303xx_ParamsS *zl303xx_Params,
                                          zl303xx_DpllIdE dpllId,
                                          zl303xx_BooleanE *val);
zlStatusE zl303xx_IsrDpllRefChgdMaskEnSet(zl303xx_ParamsS *zl303xx_Params,
                                          zl303xx_DpllIdE dpllId,
                                          zl303xx_BooleanE val);

/* Sync Fail Mask En Access */
zlStatusE zl303xx_IsrSyncFailMaskEnGet(zl303xx_ParamsS *zl303xx_Params,
                                       zl303xx_SyncIdE syncId,
                                       zl303xx_BooleanE *val);
zlStatusE zl303xx_IsrSyncFailMaskEnSet(zl303xx_ParamsS *zl303xx_Params,
                                       zl303xx_SyncIdE syncId,
                                       zl303xx_BooleanE val);

/* SCM Fail Mask Enable Access */
zlStatusE zl303xx_IsrScmFailMaskEnGet(zl303xx_ParamsS *zl303xx_Params,
                                      zl303xx_RefIdE refId,
                                      zl303xx_BooleanE *val);
zlStatusE zl303xx_IsrScmFailMaskEnSet(zl303xx_ParamsS *zl303xx_Params,
                                      zl303xx_RefIdE refId,
                                      zl303xx_BooleanE val);

/* CFM Fail Mask Enable Access */
zlStatusE zl303xx_IsrCfmFailMaskEnGet(zl303xx_ParamsS *zl303xx_Params,
                                      zl303xx_RefIdE refId,
                                      zl303xx_BooleanE *val);
zlStatusE zl303xx_IsrCfmFailMaskEnSet(zl303xx_ParamsS *zl303xx_Params,
                                      zl303xx_RefIdE refId,
                                      zl303xx_BooleanE val);

/* GST Fail Mask Enable Access */
zlStatusE zl303xx_IsrGstFailMaskEnGet(zl303xx_ParamsS *zl303xx_Params,
                                      zl303xx_RefIdE refId,
                                      zl303xx_BooleanE *val);
zlStatusE zl303xx_IsrGstFailMaskEnSet(zl303xx_ParamsS *zl303xx_Params,
                                      zl303xx_RefIdE refId,
                                      zl303xx_BooleanE val);

/* PFM Fail Mask Enable Access */
zlStatusE zl303xx_IsrPfmFailMaskEnGet(zl303xx_ParamsS *zl303xx_Params,
                                      zl303xx_RefIdE refId,
                                      zl303xx_BooleanE *val);
zlStatusE zl303xx_IsrPfmFailMaskEnSet(zl303xx_ParamsS *zl303xx_Params,
                                      zl303xx_RefIdE refId,
                                      zl303xx_BooleanE val);

/* GST-SCM Mask En Access */
zlStatusE zl303xx_IsrGstScmMaskEnGet(zl303xx_ParamsS *zl303xx_Params,
                                     zl303xx_RefIdE refId,
                                     zl303xx_BooleanE *val);
zlStatusE zl303xx_IsrGstScmMaskEnSet(zl303xx_ParamsS *zl303xx_Params,
                                     zl303xx_RefIdE refId,
                                     zl303xx_BooleanE val);

/* GST-CFM Mask En Access */
zlStatusE zl303xx_IsrGstCfmMaskEnGet(zl303xx_ParamsS *zl303xx_Params,
                                     zl303xx_RefIdE refId,
                                     zl303xx_BooleanE *val);
zlStatusE zl303xx_IsrGstCfmMaskEnSet(zl303xx_ParamsS *zl303xx_Params,
                                     zl303xx_RefIdE refId,
                                     zl303xx_BooleanE val);

#ifdef __cplusplus
}
#endif

#endif /* MULTIPLE INCLUDE BARRIER */

