

/*******************************************************************************
*
*  $Id: zl303xx_CustLow.h 6058 2011-06-10 14:37:16Z AW $
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     Definitions for low-level CUST attribute access
*
*
*******************************************************************************/

#ifndef ZL303XX_API_CUST_LOW_H_
#define ZL303XX_API_CUST_LOW_H_

#ifdef __cplusplus
extern "C" {
#endif

/*****************   INCLUDE FILES   ******************************************/

#include "zl303xx_ApiLow.h"
#include "zl303xx_ApiLowDataTypes.h"

/*****************   DEFINES   ************************************************/

/***************/

/* Function Declarations for the zl303xx_CustConfigS type */
zlStatusE zl303xx_CustConfigStructInit(zl303xx_ParamsS *zl303xx_Params,
                                       zl303xx_CustConfigS *par);
zlStatusE zl303xx_CustConfigCheck(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_CustConfigS *par);
zlStatusE zl303xx_CustConfigGet(zl303xx_ParamsS *zl303xx_Params,
                                zl303xx_CustConfigS *par);
zlStatusE zl303xx_CustConfigSet(zl303xx_ParamsS *zl303xx_Params,
                                zl303xx_CustConfigS *par);

/* N*8kHz Register and Bitfield Attribute Definitions */
#define ZL303XX_CUST_8K_MULT0_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x67, ZL303XX_MEM_SIZE_2_BYTE)
#define ZL303XX_CUST_8K_MULT1_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x71, ZL303XX_MEM_SIZE_2_BYTE)
#define ZL303XX_CUST_8K_MULT_REG(custId)   \
                    (Uint32T)((custId < ZL303XX_CUST_ID_B) ? \
                              (ZL303XX_CUST_8K_MULT0_REG) : \
                              (ZL303XX_CUST_8K_MULT1_REG))


/* Nx8kHz Mult Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_CUST_FREQUENCY_MASK                  (Uint32T)(0x3FFF)
#define ZL303XX_CUST_FREQUENCY_SHIFT                 0

/***************/

/* SCM Low Register and Bitfield Attribute Definitions */
#define ZL303XX_CUST_SCM_LO_LIMIT0_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x69, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_CUST_SCM_LO_LIMIT1_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x73, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_CUST_SCM_LO_LIMIT_REG(custId)   \
                    (Uint32T)((custId < ZL303XX_CUST_ID_B) ? \
                              (ZL303XX_CUST_SCM_LO_LIMIT0_REG) : \
                              (ZL303XX_CUST_SCM_LO_LIMIT1_REG))


/* SCM Low Limit Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_CUST_SCM_LO_LIMIT_MASK               (Uint32T)(0xFF)
#define ZL303XX_CUST_SCM_LO_LIMIT_SHIFT              0

/***************/

/* SCM High Register and Bitfield Attribute Definitions */
#define ZL303XX_CUST_SCM_HI_LIMIT0_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x6A, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_CUST_SCM_HI_LIMIT1_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x74, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_CUST_SCM_HI_LIMIT_REG(custId)   \
                    (Uint32T)((custId < ZL303XX_CUST_ID_B) ? \
                              (ZL303XX_CUST_SCM_HI_LIMIT0_REG) : \
                              (ZL303XX_CUST_SCM_HI_LIMIT1_REG))


/* SCM High Limit Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_CUST_SCM_HI_LIMIT_MASK               (Uint32T)(0xFF)
#define ZL303XX_CUST_SCM_HI_LIMIT_SHIFT              0

/***************/

/* CFM Low Register and Bitfield Attribute Definitions */
#define ZL303XX_CUST_CFM_LO_LIMIT0_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x6B, ZL303XX_MEM_SIZE_2_BYTE)
#define ZL303XX_CUST_CFM_LO_LIMIT1_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x75, ZL303XX_MEM_SIZE_2_BYTE)
#define ZL303XX_CUST_CFM_LO_LIMIT_REG(custId)   \
                    (Uint32T)((custId < ZL303XX_CUST_ID_B) ? \
                              (ZL303XX_CUST_CFM_LO_LIMIT0_REG) : \
                              (ZL303XX_CUST_CFM_LO_LIMIT1_REG))


/* CFM Low Limit Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_CUST_CFM_LO_LIMIT_MASK               (Uint32T)(0xFFFF)
#define ZL303XX_CUST_CFM_LO_LIMIT_SHIFT              0

/***************/

/* CFM High Register and Bitfield Attribute Definitions */
#define ZL303XX_CUST_CFM_HI_LIMIT0_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x6D, ZL303XX_MEM_SIZE_2_BYTE)
#define ZL303XX_CUST_CFM_HI_LIMIT1_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x77, ZL303XX_MEM_SIZE_2_BYTE)
#define ZL303XX_CUST_CFM_HI_LIMIT_REG(custId)   \
                    (Uint32T)((custId < ZL303XX_CUST_ID_B) ? \
                              (ZL303XX_CUST_CFM_HI_LIMIT0_REG) : \
                              (ZL303XX_CUST_CFM_HI_LIMIT1_REG))


/* CFM High Limit Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_CUST_CFM_HI_LIMIT_MASK               (Uint32T)(0xFFFF)
#define ZL303XX_CUST_CFM_HI_LIMIT_SHIFT              0

/***************/

/* CFM Cycles Register and Bitfield Attribute Definitions */
#define ZL303XX_CUST_CFM_CYCLE0_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x6F, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_CUST_CFM_CYCLE1_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x79, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_CUST_CFM_CYCLE_REG(custId)   \
                    (Uint32T)((custId < ZL303XX_CUST_ID_B) ? \
                              (ZL303XX_CUST_CFM_CYCLE0_REG) : \
                              (ZL303XX_CUST_CFM_CYCLE1_REG))


/* CFM Cycle Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_CUST_CFM_CYCLES_MASK                 (Uint32T)(0xFF)
#define ZL303XX_CUST_CFM_CYCLES_SHIFT                0

/***************/

/* Divide Register and Bitfield Attribute Definitions */
#define ZL303XX_CUST_DIVIDE0_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x70, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_CUST_DIVIDE1_REG   \
                    ZL303XX_MAKE_ADDR(0x00, 0x7A, ZL303XX_MEM_SIZE_1_BYTE)
#define ZL303XX_CUST_DIVIDE_REG(custId)   \
                    (Uint32T)((custId < ZL303XX_CUST_ID_B) ? \
                              (ZL303XX_CUST_DIVIDE0_REG) : \
                              (ZL303XX_CUST_DIVIDE1_REG))


/* Divide Bitfield Masks (Upper & Lower) & Shift Definitions */
#define ZL303XX_CUST_DIVIDE_MASK                     (Uint32T)(0x01)
#define ZL303XX_CUST_DIVIDE_SHIFT                    0


/*****************   DATA TYPES   *********************************************/

/*****************   DATA STRUCTURES   ****************************************/

/*****************   EXTERNAL FUNCTION DECLARATIONS   *************************/

/* Nx8kHz Mult Access */
zlStatusE zl303xx_CustFrequencyGet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_CustIdE custId, Uint32T *val);
zlStatusE zl303xx_CustFrequencySet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_CustIdE custId, Uint32T val);

/* SCM Low Limit Access */
zlStatusE zl303xx_CustScmLoLimitGet(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_CustIdE custId,
                                    Uint32T *val);
zlStatusE zl303xx_CustScmLoLimitSet(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_CustIdE custId, Uint32T val);

/* SCM High Limit Access */
zlStatusE zl303xx_CustScmHiLimitGet(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_CustIdE custId,
                                    Uint32T *val);
zlStatusE zl303xx_CustScmHiLimitSet(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_CustIdE custId, Uint32T val);

/* CFM Low Limit Access */
zlStatusE zl303xx_CustCfmLoLimitGet(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_CustIdE custId,
                                    Uint32T *val);
zlStatusE zl303xx_CustCfmLoLimitSet(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_CustIdE custId, Uint32T val);

/* CFM High Limit Access */
zlStatusE zl303xx_CustCfmHiLimitGet(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_CustIdE custId,
                                    Uint32T *val);
zlStatusE zl303xx_CustCfmHiLimitSet(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_CustIdE custId, Uint32T val);

/* CFM Cycle Access */
zlStatusE zl303xx_CustCfmCyclesGet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_CustIdE custId, Uint32T *val);
zlStatusE zl303xx_CustCfmCyclesSet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_CustIdE custId, Uint32T val);

/* Divide Access */
zlStatusE zl303xx_CustDivideGet(zl303xx_ParamsS *zl303xx_Params,
                                zl303xx_CustIdE custId,
                                zl303xx_BooleanE *val);
zlStatusE zl303xx_CustDivideSet(zl303xx_ParamsS *zl303xx_Params,
                                zl303xx_CustIdE custId,
                                zl303xx_BooleanE val);

#ifdef __cplusplus
}
#endif

#endif /* MULTIPLE INCLUDE BARRIER */

