

/*******************************************************************************
*
*  $Id: zl303xx_Interrupt.h 6058 2011-06-10 14:37:16Z AW $
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     System interrupt functions
*
*******************************************************************************/

#ifndef ZL303XX_INTERRUPT_H_
#define ZL303XX_INTERRUPT_H_

#ifdef __cplusplus
extern "C" {
#endif

/*****************   INCLUDE FILES   ******************************************/

#include "zl303xx_Global.h"
#include "zl303xx.h"

/*****************   DEFINES   ************************************************/

/* Bit masks for the individual interrupts in the top level interrupt registers */
/* In the ISR reg a '1' indicates the interrupt is active */
/* In the ISR mask reg a '1' indicates that that interrupt is allowed */
#define ZL303XX_TOP_ISR_REF_FAIL_0_7  (Uint8T)0x01
#define ZL303XX_TOP_ISR_DPLL1         (Uint8T)0x02
#define ZL303XX_TOP_ISR_DPLL2         (Uint8T)0x04
#define ZL303XX_TOP_ISR_REF_SYNC_8    (Uint8T)0x08
#define ZL303XX_TOP_ISR_REF_CC        (Uint8T)0x10
#define ZL303XX_TOP_ISR_REF_S3E       (Uint8T)0x20
#define ZL303XX_TOP_ISR_REF_TS_ENG    (Uint8T)0x40
#define ZL303XX_TOP_ISR_REF_BRIDGE    (Uint8T)0x80

/*****************   DATA TYPES   *********************************************/

/*****************   DATA STRUCTURES   ****************************************/

/*****************   EXPORTED GLOBAL VARIABLE DECLARATIONS   ******************/

zlStatusE zl303xx_InterruptSetTopMask(zl303xx_ParamsS *zl303xx_Params, Uint32T intSrc,
                                       Uint8T value, Uint8T mask);
void zl303xx_ProcessInterrupts(zl303xx_ParamsS *zl303xx_Params, Uint8T isrReg, Uint8T intNum);

/* Internal function for handling interrupts */
void zl303xx_InterruptHandler(zl303xx_ParamsS *zl303xx_Params, Uint8T intNum);

/*****************   EXTERNAL FUNCTION DECLARATIONS   *************************/

#ifdef __cplusplus
}
#endif

#endif /* MULTIPLE INCLUDE BARRIER */

