

/*******************************************************************************
*
*  $Id: zl303xx_Dco.c 8094 2012-04-17 18:12:24Z PC $
*
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     This file contains control functions for the DCO.
*
*******************************************************************************/

/*****************   INCLUDE FILES   ******************************************/

#include "zl303xx.h"
#include "zl303xx_RdWr.h"
#include "zl303xx_Dco.h"
#include "zl303xx_AddressMap.h"



/*****************   DEFINES     **********************************************/

/* The addresses of the individual registers for the DCO */
#define ZL303XX_REG_DCO_FREQ_OFFSET_WR      ZL303XX_MAKE_MEM_ADDR(0x01, 0x65, ZL303XX_MEM_SIZE_4_BYTE, ZL303XX_MEM_OVRLY_NONE)
#define ZL303XX_REG_DCO_FREQ_OFFSET_RD      ZL303XX_MAKE_MEM_ADDR(0x07, 0x75, ZL303XX_MEM_SIZE_4_BYTE, ZL303XX_MEM_OVRLY_NONE)

/* Maximum frequency offset definitions */
#define ZL303XX_DCO_MAX_OFFSET_HZ_PER_MHZ      (Sint32T)125
#define ZL303XX_DCO_MIN_OFFSET_UHZ_PER_MHZ     (Sint32T)(-ZL303XX_DCO_MAX_OFFSET_HZ_PER_MHZ * 1000000)
#define ZL303XX_DCO_MAX_OFFSET_UHZ_PER_MHZ     (Sint32T)(ZL303XX_DCO_MAX_OFFSET_HZ_PER_MHZ * 1000000)


/*****************   STATIC FUNCTION DECLARATIONS   ***************************/

/*****************   STATIC GLOBAL VARIABLES   ********************************/

/*****************   EXPORTED GLOBAL VARIABLES   ******************************/

/*******************   SPECIAL FUNCTION DEFINTIONS   ***************************/

/*****************   EXPORTED FUNCTION DEFINITIONS   **************************/


/**

  Function Name:
   zl303xx_DcoSetFreq

  Details:
   Sets the DCO offset based on the frequency offset (in uHz/MHz) provided.

  Parameters:
   [in]  zl303xx_Params      Pointer to the device instance parameter structure
   [in]  freqOffsetUppm    Frequency offset of the device in units of ppm * 1e6
                                 (i.e. micro-Hz offset per Mega-Hz of frequency).

  Return Value:
   zlStatusE

  Notes:
   None

*******************************************************************************/

zlStatusE zl303xx_DcoSetFreq(void *hwParams, Sint32T freqOffsetUppm)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_ParamsS *zl303xx_Params = (zl303xx_ParamsS *)hwParams;


   ZL303XX_TRACE(ZL303XX_MOD_ID_PLL, 2,
               "zl303xx_DcoSetFreq: %ld",
               freqOffsetUppm, 0, 0, 0, 0, 0);

   /* Check the function input Parameter Pointer */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
   }


   {
       /* Limit the frequency offset to min/max hardware range */
      if (freqOffsetUppm < ZL303XX_DCO_MIN_OFFSET_UHZ_PER_MHZ)
      {
         freqOffsetUppm = ZL303XX_DCO_MIN_OFFSET_UHZ_PER_MHZ;
      }
      else if (freqOffsetUppm > ZL303XX_DCO_MAX_OFFSET_UHZ_PER_MHZ)
      {
         freqOffsetUppm = ZL303XX_DCO_MAX_OFFSET_UHZ_PER_MHZ;
      }

      /* Convert freqOffsetUppm to an equivalent DCO offset value */
      {
          Uint32T dcoOffset;
          Uint8T round;

          /* Since our 64-bit math routines do not like Signed values, track the
             signage of the frequency Offset and convert to an unsigned. Assume
             Unsigned by default. */
          zl303xx_BooleanE isNegative = ZL303XX_FALSE;


          if (freqOffsetUppm < 0)
          {
             freqOffsetUppm = ~freqOffsetUppm + 1;
             isNegative = ZL303XX_TRUE;
          }

          /* Convert the frequency offset into the equivalent DCO offset register
             value. */
          dcoOffset = Mult_Mult_Div_U32((Uint32T)freqOffsetUppm,
                               (Uint32T)(zl303xx_Params->pllParams.dcoCountPerPpm),
                               ONE_MILLION,
                               &round, NULL);

          dcoOffset += round;

          /* Convert for the original signage of the input variable */
          if (isNegative == ZL303XX_TRUE)
          {
             dcoOffset = ~dcoOffset + 1;
          }

          /* Write the equivalent DCO offset to the device register */
          status = zl303xx_Write(zl303xx_Params, NULL,
                                 ZL303XX_REG_DCO_FREQ_OFFSET_WR,
                                 dcoOffset);
/* printf("freqOffsetUppm=%d, dcoOffset=%x, dcoCountPerPpm=%d\n", freqOffsetUppm, dcoOffset, zl303xx_Params->pllParams.dcoCountPerPpm); */
      }

      if (status == ZL303XX_OK)
      {
         zl303xx_Params->pllParams.dcoFreq = freqOffsetUppm;
      }
   }

   return status;
}  /* END zl303xx_DcoSetFreq */

/**

  Function Name:
   zl303xx_DcoGetFreq

  Details:
   Gets the current DCO offset and converts it to an equivalent frequency
   offset (in units of uHz/MHz).

   PLL frequency offset

  Parameters:
   [in]    zl303xx_Params    Pointer to the device instance parameter structure

   [out]    freqOffsetUppm    Frequency offset of the device in units of ppm * 1e6
                        (i.e. micro-Hz offset per Mega-Hz of frequency).

  Return Value:
   zlStatusE

  Notes:
   None

*******************************************************************************/
zlStatusE zl303xx_DcoGetFreq(void *hwParams, Sint32T *freqOffsetUppm)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_ParamsS *zl303xx_Params = (zl303xx_ParamsS *)hwParams;


   ZL303XX_TRACE(ZL303XX_MOD_ID_PLL, 2, "zl303xx_DcoGetFreq:", 0,0,0,0,0,0);

   /* Check the function input Parameter Pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params) |
               ZL303XX_CHECK_POINTER(freqOffsetUppm);
   }

   /* Get the current, converted dcoOffset from the device register. */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_Read(zl303xx_Params, NULL,
                          ZL303XX_REG_DCO_FREQ_OFFSET_RD,
                          (Uint32T *)freqOffsetUppm);
   }

   /* Convert the DCO offset value to an equivalent freqOffsetUppm value */
   if (status == ZL303XX_OK)
   {
      /* Since our 64-bit math routines do not like Signed values, track the
         signage of the frequency Offset and convert to an unsigned. Assume
         Unsigned by default. */
      zl303xx_BooleanE isNegative = ZL303XX_FALSE;
      Uint8T round;

      if (*freqOffsetUppm < 0)
      {
         *freqOffsetUppm = ~(*freqOffsetUppm) + 1;
         isNegative = ZL303XX_TRUE;
      }

      /* Convert the frequency offset into the equivalent DCO offset register
         value. */
      *freqOffsetUppm = Mult_Mult_Div_U32((Uint32T)*freqOffsetUppm,
                           ONE_MILLION,
                           (Uint32T)(zl303xx_Params->pllParams.dcoCountPerPpm),
                           &round, NULL);

      *freqOffsetUppm += round;

      /* Convert for the original signage of the register variable */
      if (isNegative == ZL303XX_TRUE)
      {
         *freqOffsetUppm = ~(*freqOffsetUppm) + 1;
      }
   }

   return status;
}

/*****************   STATIC FUNCTION DEFINITIONS   ****************************/

/*****************   END   ****************************************************/

