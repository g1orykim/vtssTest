

/*******************************************************************************
*
*  $Id: zl303xx_S3eLow.h 6058 2011-06-10 14:37:16Z AW $
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     Definitions for low-level S3E attribute access
*
*******************************************************************************/

#ifndef ZL303XX_API_S3E_LOW_H_
#define ZL303XX_API_S3E_LOW_H_

#ifdef __cplusplus
extern "C" {
#endif

/*****************   INCLUDE FILES   ******************************************/

#include "zl303xx_ApiLow.h"
#include "zl303xx_ApiLowDataTypes.h"

/*****************   DEFINES   ************************************************/

/***************/

/* Pbo Threshold Register and Bitfield Attribute Definitions */
#define ZL303XX_S3E_PBO_THRESHOLD_REG   \
                    ZL303XX_MAKE_ADDR(0x08, 0x73, ZL303XX_MEM_SIZE_1_BYTE)

/* PBO Threshold Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_S3E_PBO_THRESHOLD_MASK               (Uint32T)(0xFF)
#define ZL303XX_S3E_PBO_THRESHOLD_SHIFT              0

#define ZL303XX_S3E_PBO_THRESHOLD_MAX   ZL303XX_S3E_PBO_THRESHOLD_MASK

#define ZL303XX_CHECK_PBO_THRESHOLD(val)  \
            ((val > ZL303XX_S3E_PBO_THRESHOLD_MAX) ?  \
            ZL303XX_PARAMETER_INVALID :  \
            ZL303XX_OK)

/***************/

/* Pbo Min Phase Chg Register and Bitfield Attribute Definitions */
#define ZL303XX_S3E_PBO_MIN_SLOPE_REG   \
                    ZL303XX_MAKE_ADDR(0x08, 0x74, ZL303XX_MEM_SIZE_1_BYTE)

/* PBO Min Phase Chg Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_S3E_PBO_MIN_SLOPE_MASK               (Uint32T)(0xFF)
#define ZL303XX_S3E_PBO_MIN_SLOPE_SHIFT              0

#define ZL303XX_S3E_PBO_MIN_SLOPE_MAX   ZL303XX_S3E_PBO_MIN_SLOPE_MASK

#define ZL303XX_CHECK_PBO_MIN_SLOPE(val)  \
            ((val > ZL303XX_S3E_PBO_MIN_SLOPE_MAX) ?  \
            ZL303XX_PARAMETER_INVALID :  \
            ZL303XX_OK)

/***************/

/* Pbo Transient Interval Register and Bitfield Attribute Definitions */
#define ZL303XX_S3E_PBO_TRANS_INT_REG   \
                    ZL303XX_MAKE_ADDR(0x08, 0x75, ZL303XX_MEM_SIZE_1_BYTE)

/* PBO Transient Interval Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_S3E_PBO_END_INT_MASK                 (Uint32T)(0x0F)
#define ZL303XX_S3E_PBO_END_INT_SHIFT                0

#define ZL303XX_S3E_PBO_END_INT_MAX   ZL303XX_S3E_PBO_END_INT_MASK

#define ZL303XX_CHECK_PBO_END_INT(val)  \
            ((val > ZL303XX_S3E_PBO_END_INT_MAX) ?  \
            ZL303XX_PARAMETER_INVALID :  \
            ZL303XX_OK)

/***************/

/* Pbo Timeout Register and Bitfield Attribute Definitions */
#define ZL303XX_S3E_PBO_TIMEOUT_REG   \
                    ZL303XX_MAKE_ADDR(0x08, 0x76, ZL303XX_MEM_SIZE_1_BYTE)

/* PBO Timeout Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_S3E_PBO_TOUT_MASK                    (Uint32T)(0xFF)
#define ZL303XX_S3E_PBO_TOUT_SHIFT                   0

#define ZL303XX_S3E_PBO_TOUT_MAX   ZL303XX_S3E_PBO_TOUT_MASK

#define ZL303XX_CHECK_PBO_TOUT(val)  \
            ((val > ZL303XX_S3E_PBO_TOUT_MAX) ?  \
            ZL303XX_PARAMETER_INVALID :  \
            ZL303XX_OK)

/***************/

/* Pbo Magnitude Register and Bitfield Attribute Definitions */
#define ZL303XX_S3E_PBO_MAGNITUDE_REG   \
                    ZL303XX_MAKE_ADDR(0x08, 0x77, ZL303XX_MEM_SIZE_2_BYTE)

/* PBO Magnitude Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_S3E_PBO_MAGN_MASK                    (Uint32T)(0xFFFF)
#define ZL303XX_S3E_PBO_MAGN_SHIFT                   0

#define ZL303XX_S3E_PBO_MAGN_MAX   ZL303XX_S3E_PBO_MAGN_MASK

#define ZL303XX_CHECK_PBO_MAGN(val)  \
            ((val > ZL303XX_S3E_PBO_MAGN_MAX) ?  \
            ZL303XX_PARAMETER_INVALID :  \
            ZL303XX_OK)

/***************/

/* S3E Control Register and Bitfield Attribute Definitions */
#define ZL303XX_S3E_CONTROL_REG   \
                    ZL303XX_MAKE_ADDR(0x08, 0x79, ZL303XX_MEM_SIZE_1_BYTE)

/* PBO Enable Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_S3E_PBO_EN_MASK                      (Uint32T)(0x01)
#define ZL303XX_S3E_PBO_EN_SHIFT                     0

/* Fast Lock Enable Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_S3E_FLOCK_EN_MASK                    (Uint32T)(0x01)
#define ZL303XX_S3E_FLOCK_EN_SHIFT                   1

/* Force Fast Lock Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_S3E_FORCE_FLOCK_MASK                 (Uint32T)(0x01)
#define ZL303XX_S3E_FORCE_FLOCK_SHIFT                2

/* Ph Limit En Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_S3E_PLIM_EN_MASK                     (Uint32T)(0x01)
#define ZL303XX_S3E_PLIM_EN_SHIFT                    3

/***************/

/* Fast Lock Damping Register and Bitfield Attribute Definitions */
#define ZL303XX_S3E_FAST_LOCK_CTRL0_REG   \
                    ZL303XX_MAKE_ADDR(0x08, 0x7A, ZL303XX_MEM_SIZE_1_BYTE)

/* Damping Stage 1 Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_S3E_DAMPING_STAGE1_MASK              (Uint32T)(0x03)
#define ZL303XX_S3E_DAMPING_STAGE1_SHIFT             0

#define ZL303XX_S3E_DAMPING_STAGE1_MAX   ZL303XX_S3E_DAMPING_STAGE1_MASK

#define ZL303XX_CHECK_DAMPING_STAGE1(val)  \
            ((val > ZL303XX_S3E_DAMPING_STAGE1_MAX) ?  \
            ZL303XX_PARAMETER_INVALID :  \
            ZL303XX_OK)

/* Damping Stage 2 Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_S3E_DAMPING_STAGE2_MASK              (Uint32T)(0x03)
#define ZL303XX_S3E_DAMPING_STAGE2_SHIFT             2

#define ZL303XX_S3E_DAMPING_STAGE2_MAX   ZL303XX_S3E_DAMPING_STAGE2_MASK

#define ZL303XX_CHECK_DAMPING_STAGE2(val)  \
            ((val > ZL303XX_S3E_DAMPING_STAGE2_MAX) ?  \
            ZL303XX_PARAMETER_INVALID :  \
            ZL303XX_OK)

/* Damping Stage 3 Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_S3E_DAMPING_STAGE3_MASK              (Uint32T)(0x03)
#define ZL303XX_S3E_DAMPING_STAGE3_SHIFT             4

#define ZL303XX_S3E_DAMPING_STAGE3_MAX   ZL303XX_S3E_DAMPING_STAGE3_MASK

#define ZL303XX_CHECK_DAMPING_STAGE3(val)  \
            ((val > ZL303XX_S3E_DAMPING_STAGE3_MAX) ?  \
            ZL303XX_PARAMETER_INVALID :  \
            ZL303XX_OK)

/***************/

/* Stage 1 Control Register and Bitfield Attribute Definitions */
#define ZL303XX_S3E_FAST_LOCK_CTRL1_REG   \
                    ZL303XX_MAKE_ADDR(0x08, 0x7B, ZL303XX_MEM_SIZE_1_BYTE)

/* Stage 1 Time Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_S3E_STAGE1_TIME_MASK                 (Uint32T)(0x1F)
#define ZL303XX_S3E_STAGE1_TIME_SHIFT                0

#define ZL303XX_S3E_STAGE1_TIME_MAX   ZL303XX_S3E_STAGE1_TIME_MASK

#define ZL303XX_CHECK_STAGE1_TIME(val)  \
            ((val > ZL303XX_S3E_STAGE1_TIME_MAX) ?  \
            ZL303XX_PARAMETER_INVALID :  \
            ZL303XX_OK)

/* Stage 1 Bandwidth Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_S3E_STAGE1_BW_MASK                   (Uint32T)(0x07)
#define ZL303XX_S3E_STAGE1_BW_SHIFT                  5

#define ZL303XX_S3E_STAGE1_BW_MAX   ZL303XX_S3E_STAGE1_BW_MASK

#define ZL303XX_CHECK_STAGE1_BW(val)  \
            ((val > ZL303XX_S3E_STAGE1_BW_MAX) ?  \
            ZL303XX_PARAMETER_INVALID :  \
            ZL303XX_OK)

/***************/

/* Stage 2 Control Register and Bitfield Attribute Definitions */
#define ZL303XX_S3E_FAST_LOCK_CTRL2_REG   \
                    ZL303XX_MAKE_ADDR(0x08, 0x7C, ZL303XX_MEM_SIZE_1_BYTE)

/* Stage 2 Time Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_S3E_STAGE2_TIME_MASK                 (Uint32T)(0x1F)
#define ZL303XX_S3E_STAGE2_TIME_SHIFT                0

#define ZL303XX_S3E_STAGE2_TIME_MAX   ZL303XX_S3E_STAGE2_TIME_MASK

#define ZL303XX_CHECK_STAGE2_TIME(val)  \
            ((val > ZL303XX_S3E_STAGE2_TIME_MAX) ?  \
            ZL303XX_PARAMETER_INVALID :  \
            ZL303XX_OK)

/* Stage 2 Bandwidth Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_S3E_STAGE2_BW_MASK                   (Uint32T)(0x07)
#define ZL303XX_S3E_STAGE2_BW_SHIFT                  5

#define ZL303XX_S3E_STAGE2_BW_MAX   ZL303XX_S3E_STAGE2_BW_MASK

#define ZL303XX_CHECK_STAGE2_BW(val)  \
            ((val > ZL303XX_S3E_STAGE2_BW_MAX) ?  \
            ZL303XX_PARAMETER_INVALID :  \
            ZL303XX_OK)

/***************/

/* Stage 3 Control Register and Bitfield Attribute Definitions */
#define ZL303XX_S3E_FAST_LOCK_CTRL3_REG   \
                    ZL303XX_MAKE_ADDR(0x08, 0x7D, ZL303XX_MEM_SIZE_1_BYTE)

/* Stage 3 Time Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_S3E_STAGE3_TIME_MASK                 (Uint32T)(0x1F)
#define ZL303XX_S3E_STAGE3_TIME_SHIFT                0

#define ZL303XX_S3E_STAGE3_TIME_MAX   ZL303XX_S3E_STAGE3_TIME_MASK

#define ZL303XX_CHECK_STAGE3_TIME(val)  \
            ((val > ZL303XX_S3E_STAGE3_TIME_MAX) ?  \
            ZL303XX_PARAMETER_INVALID :  \
            ZL303XX_OK)

/* Stage 3 Bandwidth Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_S3E_STAGE3_BW_MASK                   (Uint32T)(0x07)
#define ZL303XX_S3E_STAGE3_BW_SHIFT                  5

#define ZL303XX_S3E_STAGE3_BW_MAX   ZL303XX_S3E_STAGE3_BW_MASK

#define ZL303XX_CHECK_STAGE3_BW(val)  \
            ((val > ZL303XX_S3E_STAGE3_BW_MAX) ?  \
            ZL303XX_PARAMETER_INVALID :  \
            ZL303XX_OK)

/***************/

/* S3E Status Register and Bitfield Attribute Definitions */
#define ZL303XX_S3E_S3E_ISR_REG   \
                    ZL303XX_MAKE_ADDR(0x0F, 0x6B, ZL303XX_MEM_SIZE_1_BYTE)

/* PBO Event Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_S3E_PBO_EVENT_MASK                   (Uint32T)(0x01)
#define ZL303XX_S3E_PBO_EVENT_SHIFT                  0

/* PBO Timeout Status Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_S3E_PBO_TOUT_STATUS_MASK             (Uint32T)(0x01)
#define ZL303XX_S3E_PBO_TOUT_STATUS_SHIFT            1

/* PBO Sampled Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_S3E_PBO_SAMPLE_MASK                  (Uint32T)(0x01)
#define ZL303XX_S3E_PBO_SAMPLE_SHIFT                 2

/***************/

/* S3E ISR Enable Register and Bitfield Attribute Definitions */
#define ZL303XX_S3E_S3E_MASK_REG   \
                    ZL303XX_MAKE_ADDR(0x0F, 0x6C, ZL303XX_MEM_SIZE_1_BYTE)

/* PBO Event Mask Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_S3E_PBO_EVENT_MASK_MASK              (Uint32T)(0x01)
#define ZL303XX_S3E_PBO_EVENT_MASK_SHIFT             0

/* PBO Timeout Mask Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_S3E_PBO_TIMEOUT_MASK_MASK            (Uint32T)(0x01)
#define ZL303XX_S3E_PBO_TIMEOUT_MASK_SHIFT           1

/* PBO Sampled Mask Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_S3E_PBO_SAMPLE_MASK_MASK             (Uint32T)(0x01)
#define ZL303XX_S3E_PBO_SAMPLE_MASK_SHIFT            2


/*****************   DATA TYPES   *********************************************/

/*****************   DATA STRUCTURES   ****************************************/

/*****************   EXTERNAL FUNCTION DECLARATIONS   *************************/

/* PBO Threshold Access */
zlStatusE zl303xx_S3ePboThresholdGet(zl303xx_ParamsS *zl303xx_Params,
                                     Uint32T *val);
zlStatusE zl303xx_S3ePboThresholdSet(zl303xx_ParamsS *zl303xx_Params,
                                     Uint32T val);

/* PBO Min Phase Chg Access */
zlStatusE zl303xx_S3ePboMinSlopeGet(zl303xx_ParamsS *zl303xx_Params,
                                    Uint32T *val);
zlStatusE zl303xx_S3ePboMinSlopeSet(zl303xx_ParamsS *zl303xx_Params,
                                    Uint32T val);

/* PBO Transient Interval Access */
zlStatusE zl303xx_S3ePboEndIntGet(zl303xx_ParamsS *zl303xx_Params,
                                  Uint32T *val);
zlStatusE zl303xx_S3ePboEndIntSet(zl303xx_ParamsS *zl303xx_Params, Uint32T val);

/* PBO Timeout Access */
zlStatusE zl303xx_S3ePboToutGet(zl303xx_ParamsS *zl303xx_Params, Uint32T *val);
zlStatusE zl303xx_S3ePboToutSet(zl303xx_ParamsS *zl303xx_Params, Uint32T val);

/* PBO Magnitude Access */
zlStatusE zl303xx_S3ePboMagnGet(zl303xx_ParamsS *zl303xx_Params, Uint32T *val);
zlStatusE zl303xx_S3ePboMagnSet(zl303xx_ParamsS *zl303xx_Params, Uint32T val);

/* PBO Enable Access */
zlStatusE zl303xx_S3ePboEnGet(zl303xx_ParamsS *zl303xx_Params,
                              zl303xx_BooleanE *val);
zlStatusE zl303xx_S3ePboEnSet(zl303xx_ParamsS *zl303xx_Params,
                              zl303xx_BooleanE val);

/* Fast Lock Enable Access */
zlStatusE zl303xx_S3eFlockEnGet(zl303xx_ParamsS *zl303xx_Params,
                                zl303xx_BooleanE *val);
zlStatusE zl303xx_S3eFlockEnSet(zl303xx_ParamsS *zl303xx_Params,
                                zl303xx_BooleanE val);

/* Force Fast Lock Access */
zlStatusE zl303xx_S3eForceFlockGet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_BooleanE *val);
zlStatusE zl303xx_S3eForceFlockSet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_BooleanE val);

/* Ph Limit En Access */
zlStatusE zl303xx_S3ePlimEnGet(zl303xx_ParamsS *zl303xx_Params,
                               zl303xx_BooleanE *val);
zlStatusE zl303xx_S3ePlimEnSet(zl303xx_ParamsS *zl303xx_Params,
                               zl303xx_BooleanE val);

/* Damping Stage 1 Access */
zlStatusE zl303xx_S3eDampingStage1Get(zl303xx_ParamsS *zl303xx_Params,
                                      Uint32T *val);
zlStatusE zl303xx_S3eDampingStage1Set(zl303xx_ParamsS *zl303xx_Params,
                                      Uint32T val);

/* Damping Stage 2 Access */
zlStatusE zl303xx_S3eDampingStage2Get(zl303xx_ParamsS *zl303xx_Params,
                                      Uint32T *val);
zlStatusE zl303xx_S3eDampingStage2Set(zl303xx_ParamsS *zl303xx_Params,
                                      Uint32T val);

/* Damping Stage 3 Access */
zlStatusE zl303xx_S3eDampingStage3Get(zl303xx_ParamsS *zl303xx_Params,
                                      Uint32T *val);
zlStatusE zl303xx_S3eDampingStage3Set(zl303xx_ParamsS *zl303xx_Params,
                                      Uint32T val);

/* Stage 1 Time Access */
zlStatusE zl303xx_S3eStage1TimeGet(zl303xx_ParamsS *zl303xx_Params,
                                   Uint32T *val);
zlStatusE zl303xx_S3eStage1TimeSet(zl303xx_ParamsS *zl303xx_Params,
                                   Uint32T val);

/* Stage 1 Bandwidth Access */
zlStatusE zl303xx_S3eStage1BwGet(zl303xx_ParamsS *zl303xx_Params, Uint32T *val);
zlStatusE zl303xx_S3eStage1BwSet(zl303xx_ParamsS *zl303xx_Params, Uint32T val);

/* Stage 2 Time Access */
zlStatusE zl303xx_S3eStage2TimeGet(zl303xx_ParamsS *zl303xx_Params,
                                   Uint32T *val);
zlStatusE zl303xx_S3eStage2TimeSet(zl303xx_ParamsS *zl303xx_Params,
                                   Uint32T val);

/* Stage 2 Bandwidth Access */
zlStatusE zl303xx_S3eStage2BwGet(zl303xx_ParamsS *zl303xx_Params, Uint32T *val);
zlStatusE zl303xx_S3eStage2BwSet(zl303xx_ParamsS *zl303xx_Params, Uint32T val);

/* Stage 3 Time Access */
zlStatusE zl303xx_S3eStage3TimeGet(zl303xx_ParamsS *zl303xx_Params,
                                   Uint32T *val);
zlStatusE zl303xx_S3eStage3TimeSet(zl303xx_ParamsS *zl303xx_Params,
                                   Uint32T val);

/* Stage 3 Bandwidth Access */
zlStatusE zl303xx_S3eStage3BwGet(zl303xx_ParamsS *zl303xx_Params, Uint32T *val);
zlStatusE zl303xx_S3eStage3BwSet(zl303xx_ParamsS *zl303xx_Params, Uint32T val);

/* PBO Event Access */
zlStatusE zl303xx_S3ePboEventGet(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_BooleanE *val);

/* PBO Timeout Status Access */
zlStatusE zl303xx_S3ePboToutStatusGet(zl303xx_ParamsS *zl303xx_Params,
                                      zl303xx_BooleanE *val);

/* PBO Sampled Access */
zlStatusE zl303xx_S3ePboSampleGet(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_BooleanE *val);

/* PBO Event Mask Access */
zlStatusE zl303xx_S3ePboEventMaskGet(zl303xx_ParamsS *zl303xx_Params,
                                     zl303xx_BooleanE *val);
zlStatusE zl303xx_S3ePboEventMaskSet(zl303xx_ParamsS *zl303xx_Params,
                                     zl303xx_BooleanE val);

/* PBO Timeout Mask Access */
zlStatusE zl303xx_S3ePboTimeoutMaskGet(zl303xx_ParamsS *zl303xx_Params,
                                       zl303xx_BooleanE *val);
zlStatusE zl303xx_S3ePboTimeoutMaskSet(zl303xx_ParamsS *zl303xx_Params,
                                       zl303xx_BooleanE val);

/* PBO Sampled Mask Access */
zlStatusE zl303xx_S3ePboSampleMaskGet(zl303xx_ParamsS *zl303xx_Params,
                                      zl303xx_BooleanE *val);
zlStatusE zl303xx_S3ePboSampleMaskSet(zl303xx_ParamsS *zl303xx_Params,
                                      zl303xx_BooleanE val);

#ifdef __cplusplus
}
#endif

#endif /* MULTIPLE INCLUDE BARRIER */

