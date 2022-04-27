

/*******************************************************************************
*
*  $Id: zl303xx_RefLow.h 6058 2011-06-10 14:37:16Z AW $
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     Definitions for low-level REF attribute access
*
*******************************************************************************/

#ifndef ZL303XX_API_REF_LOW_H_
#define ZL303XX_API_REF_LOW_H_

#ifdef __cplusplus
extern "C" {
#endif

/*****************   INCLUDE FILES   ******************************************/

#include "zl303xx_ApiLow.h"
#include "zl303xx_ApiLowDataTypes.h"

/*****************   DEFINES   ************************************************/

/***************/

/* Ref Freq Mode Register and Bitfield Attribute Definitions */
#define ZL303XX_REF_REF_MODE0_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x65, ZL303XX_MEM_SIZE_2_BYTE)
#define ZL303XX_REF_REF_MODE1_REG   \
                    ZL303XX_MAKE_ADDR(0x08, 0x6C, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_REF_REF_MODE_REG(refId)   \
                    (Uint32T)((refId < ZL303XX_REF_ID_8) ? \
                              (ZL303XX_REF_REF_MODE0_REG) : \
                              (ZL303XX_REF_REF_MODE1_REG))


/* Ref Mode Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_REF_FREQ_MODE_MASK                   (Uint32T)(0x03)
#define ZL303XX_REF_FREQ_MODE_SHIFT(refId)           ((refId % 8) * 2)

/***************/

/* Ref Invertion Register and Bitfield Attribute Definitions */
#define ZL303XX_REF_REF_INVERT_REG   \
                    ZL303XX_MAKE_ADDR(0x08, 0x72, ZL303XX_MEM_SIZE_1_BYTE)

/* Ref Invert Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_REF_INVERT_MASK                      (Uint32T)(0x01)
#define ZL303XX_REF_INVERT_SHIFT(refId)              (refId % 5)

/***************/

/* Ref Prescale Divider Register and Bitfield Attribute Definitions */
#define ZL303XX_REF_PRESCALE_CTRL_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x7E, ZL303XX_MEM_SIZE_1_BYTE)

/* Ref Prescaler Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_REF_PRESCALER_MASK                   (Uint32T)(0x0F)
#define ZL303XX_REF_PRESCALER_SHIFT(refId)           ((refId % 2) * 4)

/***************/

/* Ref Freq Register and Bitfield Attribute Definitions */
#define ZL303XX_REF_REF_FREQ0_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x10, ZL303XX_MEM_SIZE_4_BYTE)
#define ZL303XX_REF_REF_FREQ1_REG   \
                    ZL303XX_MAKE_ADDR(0x08, 0x69, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_REF_REF_FREQ_REG(refId)   \
                    (Uint32T)((refId < ZL303XX_REF_ID_8) ? \
                              (ZL303XX_REF_REF_FREQ0_REG) : \
                              (ZL303XX_REF_REF_FREQ1_REG))


/* Ref Freq Detected Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_REF_FREQ_DETECTED_MASK               (Uint32T)(0x0F)
#define ZL303XX_REF_FREQ_DETECTED_SHIFT(refId)       ((refId % 8) * 4)

/***************/

/* 1Hz Enable Register and Bitfield Attribute Definitions */
#define ZL303XX_REF_1_HZ_ENABLE_REG ZL303XX_MAKE_ADDR(0x08, 0x71, ZL303XX_MEM_SIZE_1_BYTE)

/* 1Hz Auto Detect Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_REF_1_HZ_ENABLE_MASK                 (Uint32T)(0x01)
#define ZL303XX_REF_1_HZ_ENABLE_SHIFT                0

/***************/

/* Sync Enable Register and Bitfield Attribute Definitions */
#define ZL303XX_REF_SYNC_ENABLE_REG   \
                    ZL303XX_MAKE_ADDR(0x08, 0x68, ZL303XX_MEM_SIZE_1_BYTE)

/* Sync Enable Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_REF_SYNC_ENABLE_MASK                 (Uint32T)(0x01)
#define ZL303XX_REF_SYNC_ENABLE_SHIFT(syncId)        (syncId % 5)

/* Sync Invert Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_REF_SYNC_INVERT_MASK                 (Uint32T)(0x01)
#define ZL303XX_REF_SYNC_INVERT_SHIFT(syncId)        ((syncId % 5) + 4)

/***************/

/* Sync Freq Register and Bitfield Attribute Definitions */
#define ZL303XX_REF_SYNC_FREQ_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x14, ZL303XX_MEM_SIZE_2_BYTE)

/* Sync Freq Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_REF_SYNC_FREQ_DETECTED_MASK          (Uint32T)(0x07)
#define ZL303XX_REF_SYNC_FREQ_DETECTED_SHIFT(syncId) ((syncId % 5) * 4)

/* Sync Fail Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_REF_SYNC_FAILED_MASK                 (Uint32T)(0x01)
#define ZL303XX_REF_SYNC_FAILED_SHIFT(syncId)        (((syncId % 5) * 4) + 3)

/***************/

/* Ref Range Limit Register and Bitfield Attribute Definitions */
#define ZL303XX_REF_OOR_LIMIT0_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x16, ZL303XX_MEM_SIZE_4_BYTE)
#define ZL303XX_REF_OOR_LIMIT1_REG   \
                    ZL303XX_MAKE_ADDR(0x08, 0x6A, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_REF_OOR_LIMIT_REG(refId)   \
                    (Uint32T)((refId < ZL303XX_REF_ID_8) ? \
                              (ZL303XX_REF_OOR_LIMIT0_REG) : \
                              (ZL303XX_REF_OOR_LIMIT1_REG))


/* Ref OOR Limit Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_REF_OOR_LIMIT_MASK                   (Uint32T)(0x07)
#define ZL303XX_REF_OOR_LIMIT_SHIFT(refId)           ((refId % 8) * 4)

/***************/

/* Time to (Dis)Qualify Register and Bitfield Attribute Definitions */
#define ZL303XX_REF_GST_QUAL_TIME_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x1C, ZL303XX_MEM_SIZE_1_BYTE)

/* Time to Disq Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_REF_TIME2_DISQ_MASK                  (Uint32T)(0x0F)
#define ZL303XX_REF_TIME2_DISQ_SHIFT                 0

/* Time to Qualify Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_REF_TIME2_QUALIFY_MASK               (Uint32T)(0x03)
#define ZL303XX_REF_TIME2_QUALIFY_SHIFT              4


/*****************   DATA TYPES   *********************************************/

/*****************   DATA STRUCTURES   ****************************************/

/*****************   EXTERNAL FUNCTION DECLARATIONS   *************************/

/* Function Declarations for the zl303xx_RefConfigS type */
zlStatusE zl303xx_RefConfigStructInit(zl303xx_ParamsS *zl303xx_Params,
                                      zl303xx_RefConfigS *par);
zlStatusE zl303xx_RefConfigCheck(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_RefConfigS *par);
zlStatusE zl303xx_RefConfigGet(zl303xx_ParamsS *zl303xx_Params,
                               zl303xx_RefConfigS *par);
zlStatusE zl303xx_RefConfigSet(zl303xx_ParamsS *zl303xx_Params,
                               zl303xx_RefConfigS *par);

/* Function Declarations for the zl303xx_GlobalInConfigS type */
zlStatusE zl303xx_GlobalInConfigStructInit(zl303xx_ParamsS *zl303xx_Params,
                                           zl303xx_GlobalInConfigS *par);
zlStatusE zl303xx_GlobalInConfigCheck(zl303xx_ParamsS *zl303xx_Params,
                                      zl303xx_GlobalInConfigS *par);
zlStatusE zl303xx_GlobalInConfigGet(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_GlobalInConfigS *par);
zlStatusE zl303xx_GlobalInConfigSet(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_GlobalInConfigS *par);

/* Function Declarations for the zl303xx_SyncConfigS type */
zlStatusE zl303xx_SyncConfigStructInit(zl303xx_ParamsS *zl303xx_Params,
                                       zl303xx_SyncConfigS *par);
zlStatusE zl303xx_SyncConfigCheck(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_SyncConfigS *par);
zlStatusE zl303xx_SyncConfigGet(zl303xx_ParamsS *zl303xx_Params,
                                zl303xx_SyncConfigS *par);
zlStatusE zl303xx_SyncConfigSet(zl303xx_ParamsS *zl303xx_Params,
                                zl303xx_SyncConfigS *par);

/* Function Declarations for the zl303xx_SyncStatusS type */
zlStatusE zl303xx_SyncStatusStructInit(zl303xx_ParamsS *zl303xx_Params,
                                       zl303xx_SyncStatusS *par);
zlStatusE zl303xx_SyncStatusCheck(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_SyncStatusS *par);
zlStatusE zl303xx_SyncStatusGet(zl303xx_ParamsS *zl303xx_Params,
                                zl303xx_SyncStatusS *par);

/* Ref Mode Access */
zlStatusE zl303xx_RefFreqModeGet(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_RefIdE refId,
                                 zl303xx_RefModeE *val);
zlStatusE zl303xx_RefFreqModeSet(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_RefIdE refId,
                                 zl303xx_RefModeE val);

/* Ref Invert Access */
zlStatusE zl303xx_RefInvertGet(zl303xx_ParamsS *zl303xx_Params,
                               zl303xx_RefIdE refId,
                               zl303xx_BooleanE *val);
zlStatusE zl303xx_RefInvertSet(zl303xx_ParamsS *zl303xx_Params,
                               zl303xx_RefIdE refId,
                               zl303xx_BooleanE val);

/* Ref Prescaler Access */
zlStatusE zl303xx_RefPrescalerGet(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_RefIdE refId,
                                  zl303xx_RefDivideE *val);
zlStatusE zl303xx_RefPrescalerSet(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_RefIdE refId,
                                  zl303xx_RefDivideE val);

/* Ref Freq Detected Access */
zlStatusE zl303xx_RefFreqDetectedGet(zl303xx_ParamsS *zl303xx_Params,
                                     zl303xx_RefIdE refId,
                                     zl303xx_RefFreqE *val);

/* 1Hz Auto Detect Access */
zlStatusE zl303xx_Ref1HzEnableGet(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_BooleanE *val);

zlStatusE zl303xx_Ref1HzEnableSet(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_BooleanE val);
/* Sync Enable Access */
zlStatusE zl303xx_RefSyncEnableGet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_SyncIdE syncId,
                                   zl303xx_BooleanE *val);
zlStatusE zl303xx_RefSyncEnableSet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_SyncIdE syncId,
                                   zl303xx_BooleanE val);

/* Sync Invert Access */
zlStatusE zl303xx_RefSyncInvertGet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_SyncIdE syncId,
                                   zl303xx_BooleanE *val);
zlStatusE zl303xx_RefSyncInvertSet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_SyncIdE syncId,
                                   zl303xx_BooleanE val);

/* Sync Freq Access */
zlStatusE zl303xx_RefSyncFreqDetectedGet(zl303xx_ParamsS *zl303xx_Params,
                                         zl303xx_SyncIdE syncId,
                                         zl303xx_SyncFreqE *val);

/* Sync Fail Access */
zlStatusE zl303xx_RefSyncFailedGet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_SyncIdE syncId,
                                   zl303xx_BooleanE *val);

/* Ref OOR Limit Access */
zlStatusE zl303xx_RefOorLimitGet(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_RefIdE refId,
                                 zl303xx_RefOorE *val);
zlStatusE zl303xx_RefOorLimitSet(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_RefIdE refId,
                                 zl303xx_RefOorE val);

/* Time to Disq Access */
zlStatusE zl303xx_RefTime2DisqGet(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_TtoDisQualE *val);
zlStatusE zl303xx_RefTime2DisqSet(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_TtoDisQualE val);

/* Time to Qualify Access */
zlStatusE zl303xx_RefTime2QualifyGet(zl303xx_ParamsS *zl303xx_Params,
                                     zl303xx_TtoQualE *val);
zlStatusE zl303xx_RefTime2QualifySet(zl303xx_ParamsS *zl303xx_Params,
                                     zl303xx_TtoQualE val);

#ifdef __cplusplus
}
#endif

#endif /* MULTIPLE INCLUDE BARRIER */

