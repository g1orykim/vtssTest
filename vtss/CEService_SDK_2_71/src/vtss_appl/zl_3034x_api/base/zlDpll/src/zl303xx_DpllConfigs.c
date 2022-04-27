

/*******************************************************************************
*
*  $Id: zl303xx_DpllConfigs.c 8171 2012-05-03 13:30:16Z PC $
*
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     High level PLL configurations.
*
*******************************************************************************/

/*****************   INCLUDE FILES   ******************************************/

#include "zl303xx_Global.h"  /* This should always be the first include file */
#include "zl303xx.h"
#include "zl303xx_DpllConfigs.h"
#include "zl303xx_ApiLowDataTypes.h"
#include "zl303xx_DpllLow.h"
#include "zl303xx_RefLow.h"
#include "zl303xx_SynthLow.h"

/*****************   DEFINES     **********************************************/

/*****************   DATATYPES   **********************************************/

/*****************   STATIC FUNCTION DECLARATIONS   ***************************/

/*****************   STATIC GLOBAL VARIABLES   ********************************/

/*****************   EXPORTED GLOBAL VARIABLES   ******************************/

/*****************   EXPORTED FUNCTION DEFINITIONS   **************************/

/**

  Function Name:
   zl303xx_PllInitStructInit

  Details:
   Initialises the parameter structure for the zl303xx_PllInit function

  Parameters:
   [in]  zl303xx_Params   Pointer to the device instance parameter structure
   [in]  par            Pointer to parameter structure for the function
                              See zl303xx_PllInit() for parameter descriptions

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_PllInitStructInit(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_PllInitS *par)
{
   zlStatusE status = ZL303XX_OK;

   ZL303XX_TRACE(ZL303XX_MOD_ID_PLL, 2,
         "zl303xx_PllInitStructInit", 0,0,0,0,0,0);

   /* Check the function input Parameter Pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params) |
               ZL303XX_CHECK_POINTER(par);
   }

   /* TopClientMode must be specified by the user so set INVALID for now */
   if (status == ZL303XX_OK)
   {
      par->TopClientMode = (zl303xx_BooleanE)ZL303XX_INVALID;
   }

   return status;
}  /* END zl303xx_PllInitStructInit */

/**

  Function Name:
   zl303xx_PllInit

  Details:
   Initialises the PLL block

  Parameters:
   [in]    zl303xx_Params   Pointer to the device instance parameter structure
   [in]    pllInit          Parameter structure

Structure inputs:

       TopClientMode
      ZL303XX_TRUE:  PLL is configured as a slave from a received timing stream
      ZL303XX_FALSE: PLL is a timing master (slaved to a local, physical
                        reference clock)

  Return Value:
   zlStatusE

  Notes:
   Not normally called directly by user application as device initialisation
   is performed by zl303xx_InitDevice() function

*******************************************************************************/

zlStatusE zl303xx_PllInit(zl303xx_ParamsS *zl303xx_Params,
                          zl303xx_PllInitS *par)
{
   zlStatusE status = ZL303XX_OK;

   ZL303XX_TRACE(ZL303XX_MOD_ID_PLL, 2,
         "zl303xx_PllInit", 0, 0, 0, 0, 0, 0);

   /* Check the function input Parameter Pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params) |
               ZL303XX_CHECK_POINTER(par);
   }

   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_BOOLEAN(par->TopClientMode);
   }

   /* Check current initialisation state of device */
   if (status == ZL303XX_OK)
   {
         zl303xx_Params->initState = ZL303XX_INIT_STATE_PLL_INIT_IN_PROGRESS;
   }

   /* Fill in device info for p0/p1 */
   if (status == ZL303XX_OK)
   {
      Uint32T synthId, clkId, fpId;
      zl303xx_SynthConfigS synthConfig;
      zl303xx_SynthClkConfigS synthClkConfig;
      zl303xx_SynthFpConfigS synthFpConfig;

      for (synthId = ZL303XX_SYNTH_ID_MIN; synthId <= ZL303XX_SYNTH_ID_MAX; synthId++)
      {
         synthConfig.Id = (zl303xx_SynthIdE)synthId;

         /* Get the current register values associated with the synth */
         if (status == ZL303XX_OK)
         {
            status = zl303xx_SynthConfigGet(zl303xx_Params, &synthConfig);
         }

         /* Save values to device parameters structure */
         if (status == ZL303XX_OK)
         {
             memcpy(&zl303xx_Params->pllParams.synth[synthId].config, &synthConfig, sizeof(zl303xx_SynthConfigS));
         }

         /* Each synth has clock outputs associated with it */
         for (clkId = ZL303XX_CLOCK_ID_MIN; clkId <= ZL303XX_CLOCK_ID_MAX; clkId++)
         {
            synthClkConfig.Id = (zl303xx_ClockIdE)clkId;
            synthClkConfig.synthId = (zl303xx_SynthIdE)synthId;

            /* Get the current register values associated with the clock */
            if (status == ZL303XX_OK)
            {
               status = zl303xx_SynthClkConfigGet(zl303xx_Params,
                                                  &synthClkConfig);
            }

            /* Save values to device parameters structure */
            if (status == ZL303XX_OK)
            {
                memcpy(&zl303xx_Params->pllParams.synth[synthId].clkConfig[clkId], &synthClkConfig, sizeof(zl303xx_SynthClkConfigS));
            }
         }

         /* Only p0 has frame pulse outputs */
         for (fpId = ZL303XX_FP_ID_MIN;
              (synthId == ZL303XX_SYNTH_ID_P0) && (fpId <= ZL303XX_FP_ID_MAX);
              fpId++)
         {
            synthFpConfig.Id = (zl303xx_FpIdE)fpId;
            synthFpConfig.synthId = (zl303xx_SynthIdE)synthId;

            /* Get the current register values for the frame pulse output */
            if (status == ZL303XX_OK)
            {
               status = zl303xx_SynthFpConfigGet(zl303xx_Params, &synthFpConfig);
            }

            /* Save values to device parameters structure */
            if (status == ZL303XX_OK)
            {
                memcpy(&zl303xx_Params->pllParams.synth[synthId].fpConfig[fpId], &synthFpConfig, sizeof(zl303xx_SynthFpConfigS));
            }
         }
      }
   }

#if !defined _USE_DEV_AS_TS_DEV
   /* Enable 1Hz input detection on Client and Server. This only allows the DPLL
      to detect a 1Hz input, it does not actually allow the DPLL to use/lock the
      input unless the input sync is also enabled. */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_Ref1HzEnableSet(zl303xx_Params, ZL303XX_TRUE);
   }

   /* Allow the DPLL to Lock to physical input Syncs */
   if (status == ZL303XX_OK)
   {
      status |= zl303xx_RefSyncEnableSet(zl303xx_Params, ZL303XX_SYNC_ID_0, ZL303XX_TRUE);
      status |= zl303xx_RefSyncEnableSet(zl303xx_Params, ZL303XX_SYNC_ID_1, ZL303XX_TRUE);
      status |= zl303xx_RefSyncEnableSet(zl303xx_Params, ZL303XX_SYNC_ID_2, ZL303XX_TRUE);
      status |= zl303xx_RefSyncEnableSet(zl303xx_Params, ZL303XX_SYNC_ID_8, ZL303XX_TRUE);
   }
#else
   #warning _USE_DEV_AS_TS_DEV
#endif

   /* Calculate a dcoCountPerPpm member in the zl303xx_Params structure.
      This value is used to convert the frequency offset (typically provided by
      the APR calculations), which is given in micro-Hz per Mega-Hz, into an
      equivalent DCO offset to be written to the device register. Since this
      conversion is constant, it helps to reduce math operations later.

      Each bit of the dcoOffset register represents +/- 72.76uHz of Offset.

                          (80MHz System Clock Freq)
      where: 72.76uHz =  ----------------------------
                                 (2 exp 40)
      So that;

                   freqOffsetUppm * 65.536     freqOffsetUppm * 65.536 * (2^40)
      dcoOffset = ------------------------- = ----------------------------------
                        72.76 uHz                        80 MHz

      And the conversion factor becomes;

                           65.536 * (2^40)     (dcoFreqHz/1000000) * (2^40)
         dcoCountPerPpm = ----------------- = ------------------------------
                              80 MHz                   sysFreqHz

      Note: - 2^40 (= 1.0995 trillion) is done since the dcoOffset register
               corresponds to bit 40 of the DCO count in Hardware.
            - The DCO and SYSTEM frequencies will be multiples of 8KHz so that
               the equation above may be further simplified for 32-bit math.

      For, a standard system (keep variables to the left & consts to the right):
         dcoMult = dcoFreqHz/8000 = 8192  = 2^13
         sysMult = sysFreqHz/8000 = 10000 = 2^5 * 5^5
                                  1000000 = 2^6 * 5^6

                                (2^13)dcoMult * (2^40)
         dcoCountPerPpm = -----------------------------------
                           (2^5 * 5^5)sysMult * (2^6 * 5^6)

                                (2^13)dcoMult * (2^34)
         dcoCountPerPpm = ------------------------------
                           (2^5 * 5^5)sysMult * (5^6)

                        = 900720 (rounded up)
   */

   if (status == ZL303XX_OK)
   {
      /* A little rough in terms of coding style, but OK for now */
      /* Get this from zlDpll */
      const Uint32T freqMult = 8000;
      const Uint32T _5exp6 = 15625;

      Uint8T round;

      /* Since the DCO multiple is at most 2-bytes, split the (2^34) factor
         so we can pass it to the function call. */
      zl303xx_Params->pllParams.dcoCountPerPpm = Mult_Mult_Div_U32(
                  ((zl303xx_Params->dcoClockFreqHz / freqMult) << 10),
                  (1 << 24),
                  ((zl303xx_Params->sysClockFreqHz / freqMult) * _5exp6),
                  &round, NULL);

      /* Round up the conversion if necessary */
      zl303xx_Params->pllParams.dcoCountPerPpm += round;
   }

   /* Read the initial dco offset value */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_DcoGetFreq(zl303xx_Params,
                                  &(zl303xx_Params->pllParams.dcoFreq));
      if (status != ZL303XX_OK)
      {
         ZL303XX_TRACE_ALWAYS("Failed to obtain DCO freq, status=%d, ignoring", status,0,0,0,0,0);
         status = ZL303XX_OK;
      }
   }

   /* Update the initialisation state */
   if (status == ZL303XX_OK)
   {
      zl303xx_Params->initState = ZL303XX_INIT_STATE_PLL_INIT_DONE;

   }

   return status;
}  /* END zl303xx_PllInit */

/*
  Function Name:
   zl303xx_PllConfigSet

  Details:
   Function to configure one of a set of predetermined PLL configs

  Parameters:
   [in]    zl303xx_Params   Pointer to the device instance parameter structure
   [in]    config         PLL configuration Enum

  Return Value:
   zlStatusE

 *****************************************************************************/

zlStatusE zl303xx_PllConfigSet(zl303xx_ParamsS *zl303xx_Params,
                             zl303xx_DpllConfigE pllConfig)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_RefIdE refId;
   zl303xx_RefOorE refOor = ZL303XX_OOR_9_12PPM;
   zl303xx_DpllBwE dpllBw = ZL303XX_DPLL_BW_P1Hz;
   zl303xx_DpllPslE dpllPsl = ZL303XX_DPLL_PSL_P885US;

   /* Check the val pointer */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
   }

   /* Switch validates the pllConfig value is within range
      and sets the component values */
   if (status == ZL303XX_OK)
   {
      switch (pllConfig)
      {
         case ZL303XX_CONFIG_GR1244_S3:
         {
            dpllBw = ZL303XX_DPLL_BW_1P7Hz;
            dpllPsl = ZL303XX_DPLL_PSL_61US;
            refOor = ZL303XX_OOR_9_12PPM;
            break;
         }

         case ZL303XX_CONFIG_GR253_S3:
         {
            dpllBw = ZL303XX_DPLL_BW_P1Hz;
            dpllPsl = ZL303XX_DPLL_PSL_61US;
            refOor = ZL303XX_OOR_9_12PPM;
            break;
         }

         case ZL303XX_CONFIG_SMC:
         {
            dpllBw = ZL303XX_DPLL_BW_P1Hz;
            dpllPsl = ZL303XX_DPLL_PSL_61US;
            refOor = ZL303XX_OOR_40_52PPM;
            break;
         }

         case ZL303XX_CONFIG_G813_OPT1:
         {
            dpllBw = ZL303XX_DPLL_BW_1P7Hz;
            dpllPsl = ZL303XX_DPLL_PSL_7P5US;
            refOor = ZL303XX_OOR_9_12PPM;
            break;
         }

         case ZL303XX_CONFIG_G813_OPT2:
         {
            dpllBw = ZL303XX_DPLL_BW_P1Hz;
            dpllPsl = ZL303XX_DPLL_PSL_P885US;
            refOor = ZL303XX_OOR_40_52PPM;
            break;
         }

         case ZL303XX_CONFIG_SLAVE:
         {
            dpllBw = ZL303XX_DPLL_BW_890Hz;
            dpllPsl = ZL303XX_DPLL_PSL_UNLIMITED;
            break;
         }

         default:
         {
            status = ZL303XX_PARAMETER_INVALID;
            break;
         }
      } /* End of Switch */
   }


   /* Set the Dpll1 Bandwidth */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_DpllBandwidthSet(zl303xx_Params, ZL303XX_DPLL_ID_1, dpllBw);
   }

   /* Set the Dpll1 Phase Slope Limit */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_DpllPhaseLimitSet(zl303xx_Params, ZL303XX_DPLL_ID_1, dpllPsl);
   }

   /* Set the Reference Out-of-Range Limits */
   /* The (status == ZL303XX_OK) checking is done within the loop */
   if (pllConfig != ZL303XX_CONFIG_SLAVE)
   {
      for (refId = ZL303XX_REF_ID_MIN; refId <= ZL303XX_REF_ID_MAX; refId++)
      {
         if (status == ZL303XX_OK)
         {
            status = zl303xx_RefOorLimitSet(zl303xx_Params, refId, refOor);
         }
      }
   }

   return status;
}

/*
  Function Name:
   zl303xx_DpllConfigDefaults

  Details:
   Function to configure the predetermined DPLL configs

  Parameters:
   [in]    zl303xx_Params   Pointer to the device instance parameter structure
   [in]    pDpllConfig         Pointer to the PLL configuration struct

  Return Value:
   zlStatusE

 *****************************************************************************/

zlStatusE zl303xx_DpllConfigDefaults(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_DpllConfigS *pDpllConfig,
                                   zl303xx_BooleanE TopClientMode)
{
   zlStatusE status = ZL303XX_OK;

    /* A number of configuration parameters are set based on the TopClientMode
       of the device. */

    /* Set up DPLL1 */
    if (status == ZL303XX_OK)
    {
       /* Gather the hardware default configuration for Dpll 1 */
       if (status == ZL303XX_OK)
       {
          pDpllConfig->Id = ZL303XX_DPLL_ID_1;
          status = zl303xx_DpllConfigGet(zl303xx_Params, pDpllConfig);
       }

       /* Set Dpll 1 depending on if this is a server or client device */
       if (status == ZL303XX_OK)
       {
          if (TopClientMode == ZL303XX_FALSE)
          {
             pDpllConfig->selectedRef = 0;        /* A Guess! */
        /*#warning Using DPLL1 Ref0 for Master! */
             pDpllConfig->mode = ZL303XX_DPLL_MODE_AUTO;
             pDpllConfig->hitlessSw = ZL303XX_DPLL_REALIGN;
             pDpllConfig->enable = ZL303XX_TRUE;
             pDpllConfig->bandwidth = ZL303XX_DPLL_BW_890Hz;
             pDpllConfig->phaseSlope = ZL303XX_DPLL_PSL_UNLIMITED;
             pDpllConfig->dcoOffsetEnable = ZL303XX_FALSE;
             pDpllConfig->dcoFilterEnable = ZL303XX_TRUE;
          }
          else
          {
             pDpllConfig->mode = ZL303XX_DPLL_MODE_TOP;
             pDpllConfig->enable = ZL303XX_TRUE;
             pDpllConfig->dcoOffsetEnable = ZL303XX_FALSE;
             pDpllConfig->dcoFilterEnable = ZL303XX_TRUE;

             pDpllConfig->selectedRef = 0;              /* Unused in Slave */
             pDpllConfig->hitlessSw = ZL303XX_DPLL_HITLESS;
             pDpllConfig->enable = ZL303XX_TRUE;
             pDpllConfig->bandwidth = ZL303XX_DPLL_BW_890Hz;
             pDpllConfig->phaseSlope = ZL303XX_DPLL_PSL_UNLIMITED;
         }

          status = zl303xx_DpllConfigSet(zl303xx_Params, pDpllConfig);

          /* Save the configured parameters for Dpll 1 */
          if (status == ZL303XX_OK)
          {
              memcpy(&zl303xx_Params->pllParams.config[ZL303XX_DPLL_ID_1], pDpllConfig, sizeof(zl303xx_DpllConfigS));
          }

       }
    }

    /* Set up DPLL2 */
    if (status == ZL303XX_OK)
    {
       /* Set up (Over-write) the default Dpll Config data structure */
       status = zl303xx_DpllConfigStructInit(zl303xx_Params, pDpllConfig);
    }

    if (status == ZL303XX_OK)
    {
       /* Gather the hardware default configuration for Dpll 2 */
       if (status == ZL303XX_OK)
       {
          pDpllConfig->Id = ZL303XX_DPLL_ID_2;
          status = zl303xx_DpllConfigGet(zl303xx_Params, pDpllConfig);
       }

       /* Disable Dpll 2 by default (if not already) */
       if (status == ZL303XX_OK)
       {
          if (pDpllConfig->enable)
          {
             pDpllConfig->enable = ZL303XX_FALSE;
             pDpllConfig->mode = ZL303XX_DPLL_MODE_FREE;
             pDpllConfig->selectedRef = 8;
             pDpllConfig->hldFilterBw = ZL303XX_INVALID;
             status = zl303xx_DpllConfigSet(zl303xx_Params, pDpllConfig);
          }
       }

       /* Save the configured parameters for Dpll 2 */
       if (status == ZL303XX_OK)
       {
          memcpy(&zl303xx_Params->pllParams.config[ZL303XX_DPLL_ID_2], pDpllConfig, sizeof(zl303xx_DpllConfigS));
       }
    }

    return status;
}


/*****************   STATIC FUNCTION DEFINITIONS   ****************************/

/*****************   END   ****************************************************/
