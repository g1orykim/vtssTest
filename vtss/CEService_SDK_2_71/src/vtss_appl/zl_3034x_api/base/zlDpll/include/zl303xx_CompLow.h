

/*******************************************************************************
*
*  $Id: zl303xx_CompLow.h 6058 2011-06-10 14:37:16Z AW $
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     Definitions for low-level COMP attribute access
*
*******************************************************************************/

#ifndef ZL303XX_API_COMP_LOW_H_
#define ZL303XX_API_COMP_LOW_H_

#ifdef __cplusplus
extern "C" {
#endif

/*****************   INCLUDE FILES   ******************************************/

#include "zl303xx_ApiLow.h"
#include "zl303xx_ApiLowDataTypes.h"

/*****************   DEFINES   ************************************************/

/***************/

/* Comp Clk Ctrl Register and Bitfield Attribute Definitions */
#define ZL303XX_COMP_CONTROL0_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x7B, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_COMP_CONTROL1_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x7C, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_COMP_CONTROL_REG(compId)   \
                    (Uint32T)((compId < ZL303XX_COMP_ID_1) ? \
                              (ZL303XX_COMP_CONTROL0_REG) : \
                              (ZL303XX_COMP_CONTROL1_REG))


/* Detect Mode Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_COMP_DET_MODE_MASK                   (Uint32T)(0x03)
#define ZL303XX_COMP_DET_MODE_SHIFT                  0

/* Fp Mode Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_COMP_FP_MODE_MASK                    (Uint32T)(0x01)
#define ZL303XX_COMP_FP_MODE_SHIFT                   2

/* Fp Select Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_COMP_FP_SELECT_MASK                  (Uint32T)(0x03)
#define ZL303XX_COMP_FP_SELECT_SHIFT                 3

/***************/

/* Comp Clk Status Register and Bitfield Attribute Definitions */
#define ZL303XX_COMP_STATUS_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x7D, ZL303XX_MEM_SIZE_1_BYTE)

/* 64kHz Status Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_COMP_64KHZ_DETECT_MASK               (Uint32T)(0x01)
#define ZL303XX_COMP_64KHZ_DETECT_SHIFT(compId)      ((compId) * 3)

/* 8kHz Status Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_COMP_FP8KHZ_DETECT_MASK              (Uint32T)(0x01)
#define ZL303XX_COMP_FP8KHZ_DETECT_SHIFT(compId)     (((compId) * 3) + 1)

/* 400Hz Status Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_COMP_FP400HZ_DETECT_MASK             (Uint32T)(0x01)
#define ZL303XX_COMP_FP400HZ_DETECT_SHIFT(compId)    (((compId) * 3) + 2)

/***************/

/* Comp Clk Status Register and Bitfield Attribute Definitions */
#define ZL303XX_COMP_ISR_REG   \
                    ZL303XX_MAKE_ADDR(0x0F, 0x69, ZL303XX_MEM_SIZE_1_BYTE)

/* BPV Error Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_COMP_BPV_ERROR_MASK                  (Uint32T)(0x01)
#define ZL303XX_COMP_BPV_ERROR_SHIFT(compId)         (compId)

/***************/

/* Comp Clk ISR En Register and Bitfield Attribute Definitions */
#define ZL303XX_COMP_MASK_REG   \
                    ZL303XX_MAKE_ADDR(0x0F, 0x6A, ZL303XX_MEM_SIZE_1_BYTE)

/* BPV Mask Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_COMP_BPV_MASK_EN_MASK                (Uint32T)(0x01)
#define ZL303XX_COMP_BPV_MASK_EN_SHIFT(compId)       (compId)


/*****************   DATA TYPES   *********************************************/

/*****************   DATA STRUCTURES   ****************************************/

/*****************   EXTERNAL FUNCTION DECLARATIONS   *************************/

/* Function Declarations for the zl303xx_CompConfigS type */
zlStatusE zl303xx_CompConfigStructInit(zl303xx_ParamsS *zl303xx_Params,
                                       zl303xx_CompConfigS *par);
zlStatusE zl303xx_CompConfigCheck(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_CompConfigS *par);
zlStatusE zl303xx_CompConfigGet(zl303xx_ParamsS *zl303xx_Params,
                                zl303xx_CompConfigS *par);
zlStatusE zl303xx_CompConfigSet(zl303xx_ParamsS *zl303xx_Params,
                                zl303xx_CompConfigS *par);

/* Function Declarations for the zl303xx_CompStatusS type */
zlStatusE zl303xx_CompStatusStructInit(zl303xx_ParamsS *zl303xx_Params,
                                       zl303xx_CompStatusS *par);
zlStatusE zl303xx_CompStatusCheck(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_CompStatusS *par);
zlStatusE zl303xx_CompStatusGet(zl303xx_ParamsS *zl303xx_Params,
                                zl303xx_CompStatusS *par);

/* Detect Mode Access */
zlStatusE zl303xx_CompDetModeGet(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_CompIdE compId,
                                 zl303xx_BooleanE *val);
zlStatusE zl303xx_CompDetModeSet(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_CompIdE compId,
                                 zl303xx_BooleanE val);

/* Fp Mode Access */
zlStatusE zl303xx_CompFpModeGet(zl303xx_ParamsS *zl303xx_Params,
                                zl303xx_CompIdE compId,
                                zl303xx_CcFpModeE *val);
zlStatusE zl303xx_CompFpModeSet(zl303xx_ParamsS *zl303xx_Params,
                                zl303xx_CompIdE compId,
                                zl303xx_CcFpModeE val);

/* Fp Select Access */
zlStatusE zl303xx_CompFpSelectGet(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_CompIdE compId,
                                  zl303xx_CcFpSelectE *val);
zlStatusE zl303xx_CompFpSelectSet(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_CompIdE compId,
                                  zl303xx_CcFpSelectE val);

/* 64kHz Status Access */
zlStatusE zl303xx_Comp64khzDetectGet(zl303xx_ParamsS *zl303xx_Params,
                                     zl303xx_CompIdE compId,
                                     zl303xx_BooleanE *val);

/* 8kHz Status Access */
zlStatusE zl303xx_CompFp8khzDetectGet(zl303xx_ParamsS *zl303xx_Params,
                                      zl303xx_CompIdE compId,
                                      zl303xx_BooleanE *val);

/* 400Hz Status Access */
zlStatusE zl303xx_CompFp400hzDetectGet(zl303xx_ParamsS *zl303xx_Params,
                                       zl303xx_CompIdE compId,
                                       zl303xx_BooleanE *val);

/* BPV Error Access */
zlStatusE zl303xx_CompBpvErrorGet(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_CompIdE compId,
                                  zl303xx_BpvStatusE *val);

/* BPV Mask Access */
zlStatusE zl303xx_CompBpvMaskEnGet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_CompIdE compId,
                                   zl303xx_BooleanE *val);
zlStatusE zl303xx_CompBpvMaskEnSet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_CompIdE compId,
                                   zl303xx_BooleanE val);

#ifdef __cplusplus
}
#endif

#endif /* MULTIPLE INCLUDE BARRIER */

