

/*******************************************************************************
*
*  $Id: zl303xx_ApiLow.h 6058 2011-06-10 14:37:16Z AW $
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     ZL303XX_ header for low level API read/write functionality.
*
*******************************************************************************/

#ifndef ZL303XX_API_LOW_H_
#define ZL303XX_API_LOW_H_

#ifdef __cplusplus
extern "C" {
#endif

/*****************   INCLUDE FILES   ******************************************/

#include "zl303xx_Api.h"
#include "zl303xx_AddressMap.h"

/*****************   DEFINES   ************************************************/

#define ZL303XX_INVALID_ADDRESS    0xFFFFFFFF
#define ZL303XX_BLANK_MASK         0x00000000
#define ZL303XX_SHIFT_MAX          31

#define ZL303XX_MAKE_ADDR(page,addr,size) \
                     (Uint32T)ZL303XX_MAKE_MEM_ADDR(page,addr,size,0)

#define ZL303XX_EXTRACT(value,mask,shift)  ((value >> shift) & mask)
#define ZL303XX_INSERT(regVal,bfVal,mask,shift)         \
                        (regVal &= ~(mask << shift)), \
                        (regVal |= ((bfVal & mask) << shift))

/*****************   DATA TYPES   *********************************************/

/*****************   DATA STRUCTURES   ****************************************/

typedef struct
{
   Uint32T addr;              /* Attributes Register Address                  */
   Uint32T mask;              /* Attributes bit mask                          */
   Uint32T shift;             /* Attributes low bit position in the register  */
   Uint32T value;             /* Attributes value (shifted to 0-bit position) */
} zl303xx_AttrRdWrS;

/*****************   EXPORTED GLOBAL VARIABLE DECLARATIONS   ******************/

/*****************   EXTERNAL FUNCTION DECLARATIONS   *************************/

/* Attribute Structure default initialization function */
zlStatusE zl303xx_AttrRdWrStructInit(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_AttrRdWrS *par);

/* Attribute Structure custom initialization function */
zlStatusE zl303xx_AttrRdWrStructFill(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_AttrRdWrS *par,
                                   Uint32T rdwrAddr,
                                   Uint32T rdwrMask,
                                   Uint32T rdwrShift);

/* Attribute Structure parameter validation function */
zlStatusE zl303xx_AttrRdWrStructCheck(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_AttrRdWrS *par);

/* Attribute Read/Write functions */
zlStatusE zl303xx_AttrRead(zl303xx_ParamsS *zl303xx_Params, zl303xx_AttrRdWrS *par);
zlStatusE zl303xx_AttrWrite(zl303xx_ParamsS *zl303xx_Params, zl303xx_AttrRdWrS *par);

#ifdef __cplusplus
}
#endif

#endif /* MULTIPLE INCLUDE BARRIER */

