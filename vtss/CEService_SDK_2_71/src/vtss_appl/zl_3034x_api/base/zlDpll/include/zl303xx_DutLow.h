

/*******************************************************************************
*
*  $Id: zl303xx_DutLow.h 8252 2012-05-23 17:23:59Z PC $
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     Definitions for low-level DUT attribute access
*
*******************************************************************************/

#ifndef ZL303XX_API_DUT_LOW_H_
#define ZL303XX_API_DUT_LOW_H_

#ifdef __cplusplus
extern "C" {
#endif

/*****************   INCLUDE FILES   ******************************************/

#include "zl303xx_ApiLow.h"
#include "zl303xx_ApiLowDataTypes.h"

/*****************   DEFINES   ************************************************/

/***************/

/* Device Id Register and Bitfield Attribute Definitions */
#define ZL303XX_DUT_DEVICE_ID_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x00, ZL303XX_MEM_SIZE_1_BYTE)

/* Device Id Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_DUT_ID_MASK                          (Uint32T)(0x1F)
#define ZL303XX_DUT_ID_SHIFT                         0

/* Device Revision Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_DUT_REVISION_MASK                    (Uint32T)(0x03)
#define ZL303XX_DUT_REVISION_SHIFT                   5

#define ZL303XX_DUT_REVISION_MAX   ZL303XX_DUT_REVISION_MASK

#define ZL303XX_CHECK_REVISION(val)  \
            ((val > ZL303XX_DUT_REVISION_MAX) ?  \
            ZL303XX_PARAMETER_INVALID :  \
            ZL303XX_OK)

/* Device Ready Status Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_DUT_READY_STATUS_MASK                (Uint32T)(0x01)
#define ZL303XX_DUT_READY_STATUS_SHIFT               7

/***************/

/* Hardware Ctrl Register and Bitfield Attribute Definitions */
#define ZL303XX_DUT_USE_HW_CTRL_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x01, ZL303XX_MEM_SIZE_1_BYTE)

/* Dpll Mode Hw Ctrl Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_DUT_DPLL_MODE_USE_HW_CTRL_MASK       (Uint32T)(0x01)
#define ZL303XX_DUT_DPLL_MODE_USE_HW_CTRL_SHIFT      1

/* Slave Enable Hw Ctrl Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_DUT_SLAVE_EN_USE_HW_CTRL_MASK        (Uint32T)(0x01)
#define ZL303XX_DUT_SLAVE_EN_USE_HW_CTRL_SHIFT       3

/***************/

/* Page Control Register and Bitfield Attribute Definitions */
#define ZL303XX_DUT_PAGE_CTRL_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x64, ZL303XX_MEM_SIZE_1_BYTE)

/* Page Value Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_DUT_PAGE_CTRL_MASK                   (Uint32T)(0x0F)
#define ZL303XX_DUT_PAGE_CTRL_SHIFT                  0


/*****************   DATA TYPES   *********************************************/

/***************/

/* DeviceId valid values */
typedef enum
{
   ZL303XX_ZL30116 = 0,
   ZL303XX_ZL30117 = 1,
   ZL303XX_ZL30119 = 3,
   ZL303XX_ZL30120 = 4,
   ZL303XX_ZL30121 = 5,
   ZL303XX_ZL30122 = 6,
   ZL303XX_ZL30123 = 7,
   ZL303XX_ZL30321 = 11,
   ZL303XX_ZL30138 = 16,
   ZL303XX_ZL30131 = 17,
   ZL303XX_ZL30132 = 18,
   ZL303XX_ZL30133 = 19,
   ZL303XX_ZL30134 = 20,
   ZL303XX_ZL30136 = 22,
   ZL303XX_ZL30130 = 24,
    ZL303XX_DEV_ID_INVALID
} zl303xx_DeviceIdE;

/* Range limit definitions & parameter checking */
#define ZL303XX_DEVICE_ID_MIN   ZL303XX_ZL30116
#define ZL303XX_DEVICE_ID_MAX   ZL303XX_ZL30130

#define ZL303XX_CHECK_DEVICE_ID(val)   \
            ((val < ZL303XX_DEVICE_ID_MIN) || (val > ZL303XX_DEVICE_ID_MAX) ?  \
            ZL303XX_PARAMETER_INVALID : \
            ZL303XX_OK)

/***************/

/* PageId valid values */
typedef enum
{
   ZL303XX_PAGE_ID_0 = 0,
   ZL303XX_PAGE_ID_8 = 8,
   ZL303XX_PAGE_ID_F = 15
} zl303xx_PageIdE;

/* Range limit definitions & parameter checking */
#define ZL303XX_PAGE_ID_MIN   ZL303XX_PAGE_ID_0
#define ZL303XX_PAGE_ID_MAX   ZL303XX_PAGE_ID_F

#define ZL303XX_CHECK_PAGE_ID(val)   \
            (((zl303xx_PageIdE)(val) > ZL303XX_PAGE_ID_MAX) ?  \
            ZL303XX_PARAMETER_INVALID : \
            ZL303XX_OK)

/*****************   DATA STRUCTURES   ****************************************/

/*****************   EXTERNAL FUNCTION DECLARATIONS   *************************/

/* Device Id Access */
zlStatusE zl303xx_DutIdGet(zl303xx_ParamsS *zl303xx_Params,
                           zl303xx_DeviceIdE *val);

/* Device Revision Access */
zlStatusE zl303xx_DutRevisionGet(zl303xx_ParamsS *zl303xx_Params, Uint32T *val);

/* Device Ready Status Access */
zlStatusE zl303xx_DutReadyStatusGet(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_BooleanE *val);

/* Dpll Mode Hw Ctrl Access */
zlStatusE zl303xx_DutDpllModeUseHwCtrlGet(zl303xx_ParamsS *zl303xx_Params,
                                          zl303xx_BooleanE *val);
zlStatusE zl303xx_DutDpllModeUseHwCtrlSet(zl303xx_ParamsS *zl303xx_Params,
                                          zl303xx_BooleanE val);

/* Slave Enable Hw Ctrl Access */
zlStatusE zl303xx_DutSlaveEnUseHwCtrlGet(zl303xx_ParamsS *zl303xx_Params,
                                         zl303xx_BooleanE *val);
zlStatusE zl303xx_DutSlaveEnUseHwCtrlSet(zl303xx_ParamsS *zl303xx_Params,
                                         zl303xx_BooleanE val);

/* Page Value Access */
zlStatusE zl303xx_DutPageCtrlGet(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_PageIdE *val);
zlStatusE zl303xx_DutPageCtrlSet(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_PageIdE val);

#ifdef __cplusplus
}
#endif

#endif /* MULTIPLE INCLUDE BARRIER */

