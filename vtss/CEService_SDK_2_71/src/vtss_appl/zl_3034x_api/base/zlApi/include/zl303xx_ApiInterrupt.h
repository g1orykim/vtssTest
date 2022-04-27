

/*******************************************************************************
*
*  $Id: zl303xx_ApiInterrupt.h 6058 2011-06-10 14:37:16Z AW $
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     System internals for the interrupt handling
*
*******************************************************************************/

#ifndef ZL303XX_API_INTERRUPT_H_
#define ZL303XX_API_INTERRUPT_H_

#ifdef __cplusplus
extern "C" {
#endif

/*****************   INCLUDE FILES   ******************************************/

#include "zl303xx_Global.h"

/*****************   DEFINES   ************************************************/

/*****************   DATA TYPES   *********************************************/
/* Interrupt request 0 is the higher priority interrupt */
enum
{
   ZL303XX_HI_PRI_INT_NUM = 0,
   ZL303XX_LO_PRI_INT_NUM = 1
};


/*****************   DATA STRUCTURES   ****************************************/
/* Structure of information required for each device attached to an interrupt */
typedef struct
{
  /* Pointer to device specific parameters */
   zl303xx_ParamsS * zl303xx_Params;
  /* Function to mask and unmask the interrupt from the device */
   void (*maskDeviceInterruptFn)(zl303xx_ParamsS * zl303xx_Params, Uint8T cpuIntNum,
                                 Uint8T deviceIntNum, zl303xx_BooleanE bMasking);
} zl303xx_DevIntParamsS;


/*****************   EXPORTED GLOBAL VARIABLE DECLARATIONS   ******************/

/*****************   EXTERNAL FUNCTION DECLARATIONS   *************************/
zlStatusE zl303xx_ApiInterruptAttach(Uint8T hiPriCpuIrqNum, zl303xx_DevIntParamsS *hiIntParams,
                                 Uint8T loPriCpuIrqNum, zl303xx_DevIntParamsS *loIntParams);
zlStatusE zl303xx_ApiInterruptDetach(Uint8T hiPriCpuIrqNum, Uint8T loPriCpuIrqNum);

zlStatusE zl303xx_ApiInterruptSetCalloutFunc(Uint8T deviceIntNum, void (*ptr)(zl303xx_ParamsS *));

/* The interrupt handler function. This needs to be "connected" to the processor
   interrupt vector by the application */
void zl303xx_IntHandlerFn(Uint8T intNum);

zlStatusE zl303xx_InterruptIsRunning(zl303xx_BooleanE *pFlag);

/* Function for internal application use only. Should not be called by an application */
zlStatusE zl303xx_ApiInterruptInit(void);

#ifdef __cplusplus
}
#endif

#endif /* MULTIPLE INCLUDE BARRIER */

