

/*******************************************************************************
*
*  $Id: zl303xx_SynthLow.c 6058 2011-06-10 14:37:16Z AW $
*
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     Functions for low-level SYNTH attribute access
*
*******************************************************************************/

/*****************   INCLUDE FILES   ******************************************/

#include "zl303xx_ApiLow.h"
#include "zl303xx_SynthLow.h"
#include "zl303xx_PllFuncs.h"

/*****************   DEFINES   ************************************************/

/*****************   DATA TYPES   *********************************************/

/*****************   DATA STRUCTURES   ****************************************/

/*****************   EXPORTED GLOBAL VARIABLE DECLARATIONS   ******************/

/*

  Function Name:
   zl303xx_SynthClkConfigStructInit

  Details:
   Initializes the members of the zl303xx_SynthClkConfigS data structure

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    par            Pointer to the zl303xx_SynthClkConfigS structure to Initialize

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SynthClkConfigStructInit(zl303xx_ParamsS *zl303xx_Params,
                                           zl303xx_SynthClkConfigS *par)
{
   zlStatusE status = ZL303XX_OK;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, par);
   }

   if (status == ZL303XX_OK)
   {
      par->enable = (zl303xx_OutEnableE)ZL303XX_INVALID;
      par->run = (zl303xx_OutputRunE)ZL303XX_INVALID;
      par->freq = (Uint32T) ZL303XX_INVALID;
      par->offset = (zl303xx_ClkOffsetE)ZL303XX_INVALID;
      par->Id = (zl303xx_ClockIdE)ZL303XX_INVALID;
      par->synthId = (zl303xx_SynthIdE)ZL303XX_INVALID;
   }

   return status;
} /* END zl303xx_SynthClkConfigStructInit */

/*

  Function Name:
   zl303xx_SynthClkConfigCheck

  Details:
   Checks the members of the zl303xx_SynthClkConfigS data structure

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    par            Pointer to the zl303xx_SynthClkConfigS structure to Check

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SynthClkConfigCheck(zl303xx_ParamsS *zl303xx_Params,
                                      zl303xx_SynthClkConfigS *par)
{
   zlStatusE status = ZL303XX_OK;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, par);
   }

   /* Check the value of the par structure members */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_OUT_ENABLE(par->enable) |
               ZL303XX_CHECK_OUTPUT_RUN(par->run) |
               zl303xx_CheckSynthClkFreq(par->Id, par->freq) |
               ZL303XX_CHECK_CLK_OFFSET(par->offset) |
               ZL303XX_CHECK_CLOCK_ID(par->Id) |
               ZL303XX_CHECK_SYNTH_ID(par->synthId);
   }

   return status;
} /* END zl303xx_SynthClkConfigCheck */

/*

  Function Name:
   zl303xx_SynthClkConfigGet

  Details:
   Gets the members of the zl303xx_SynthClkConfigS data structure

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    par            Pointer to the zl303xx_SynthClkConfigS structure to Get

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SynthClkConfigGet(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_SynthClkConfigS *par)
{
   zlStatusE status = ZL303XX_OK;
   Uint32T regValue = 0, clk0Freq;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, par);
   }

   /* Check for a valid ClockId */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_CLOCK_ID(par->Id);
   }

   /* Read */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_Read(zl303xx_Params, NULL,
                            ZL303XX_SYNTH_OUTPUT_CTRL_REG(par->synthId),
                            &regValue);
   }

   /* Extract */
   if (status == ZL303XX_OK)
   {
      par->enable = (zl303xx_OutEnableE)ZL303XX_EXTRACT(regValue,
                                        ZL303XX_SYNTH_CLK_ENABLE_MASK,
                                        ZL303XX_SYNTH_CLK_ENABLE_SHIFT(par->Id));

      par->run = (zl303xx_OutputRunE)ZL303XX_EXTRACT(regValue,
                                      ZL303XX_SYNTH_CLK_RUN_MASK,
                                      ZL303XX_SYNTH_CLK_RUN_SHIFT(par->Id));
   }

   /* Read */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_Read(zl303xx_Params, NULL,
                            ZL303XX_SYNTH_CLK_MULT_REG(par->synthId),
                            &regValue);
   }

   /* Extract */
   if (status == ZL303XX_OK)
   {
        clk0Freq = zl303xx_SynthClk0FreqInKHz(ZL303XX_EXTRACT(regValue,
                                            ZL303XX_SYNTH_FREQUENCY_MULT_MASK,
                                            ZL303XX_SYNTH_FREQUENCY_MULT_SHIFT));

        if (par->Id == ZL303XX_CLOCK_ID_0)
        {
            par->freq = clk0Freq;
        }
        else if (par->Id == ZL303XX_CLOCK_ID_1)
        {
            status = zl303xx_Read(zl303xx_Params, NULL,
                                ZL303XX_SYNTH_CLK_DIV_REG(par->synthId),
                                &regValue);

            par->freq = (Uint32T)ZL303XX_EXTRACT(regValue,
                                    ZL303XX_SYNTH_FREQUENCY_DIV_MASK,
                                    ZL303XX_SYNTH_FREQUENCY_DIV_SHIFT);

            par->freq = zl303xx_SynthClk1FreqInKHz(par->freq, clk0Freq);
        }
        else
        {
            status = ZL303XX_PARAMETER_INVALID;
        }
   }

   /* Read */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_Read(zl303xx_Params, NULL,
                            ZL303XX_SYNTH_CLK_OFFSET_REG(par->synthId,par->Id),
                            &regValue);
   }

   /* Extract */
   if (status == ZL303XX_OK)
   {
      par->offset = (zl303xx_ClkOffsetE)ZL303XX_EXTRACT(regValue,
                                        ZL303XX_SYNTH_CLK_OFFSET_MASK,
                                        ZL303XX_SYNTH_CLK_OFFSET_SHIFT);
   }

   /* Update the device parameters structure */
   if (status == ZL303XX_OK)
   {
      zl303xx_Params->pllParams.synth[par->synthId].clkConfig[par->Id] = *par;
   }

   return status;
} /* END zl303xx_SynthClkConfigGet */

/*

  Function Name:
   zl303xx_SynthClkConfigSet

  Details:
   Sets the members of the zl303xx_SynthClkConfigS data structure

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    par            Pointer to the zl303xx_SynthClkConfigS structure to Set

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SynthClkConfigSet(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_SynthClkConfigS *par)
{
   zlStatusE status = ZL303XX_OK;
   Uint32T regValue, mask;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, par);
   }

   /* Check the par values to be written */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_SynthClkConfigCheck(zl303xx_Params, par);
   }

   /* Package */
   if (status == ZL303XX_OK)
   {
      regValue = 0;
      mask = 0;

      /* enable */
      ZL303XX_INSERT(regValue, par->enable,
                             ZL303XX_SYNTH_CLK_ENABLE_MASK,
                             ZL303XX_SYNTH_CLK_ENABLE_SHIFT(par->Id));

      /* run */
      ZL303XX_INSERT(regValue, par->run,
                             ZL303XX_SYNTH_CLK_RUN_MASK,
                             ZL303XX_SYNTH_CLK_RUN_SHIFT(par->Id));

      mask |= (ZL303XX_SYNTH_CLK_ENABLE_MASK << ZL303XX_SYNTH_CLK_ENABLE_SHIFT(par->Id)) |
              (ZL303XX_SYNTH_CLK_RUN_MASK << ZL303XX_SYNTH_CLK_RUN_SHIFT(par->Id));

      /* Write the Data for this Register */
      status = zl303xx_ReadModWrite(zl303xx_Params, NULL, ZL303XX_SYNTH_OUTPUT_CTRL_REG(par->synthId), regValue, mask, NULL);
   }

   /* Package */
   if (status == ZL303XX_OK)
   {
      Uint32T actualFreq = 0;
      regValue = 0;
      mask = 0;

      /* freq */
      if (par->Id == ZL303XX_CLOCK_ID_0)
      {
          Uint32T clk0FreqReg;

          status = zl303xx_ClosestSynthClk0FreqReg(par->freq, &clk0FreqReg);

          if (status == ZL303XX_OK)
          {
              ZL303XX_INSERT(regValue, clk0FreqReg,
                                     ZL303XX_SYNTH_FREQUENCY_MULT_MASK,
                                     ZL303XX_SYNTH_FREQUENCY_MULT_SHIFT);

              mask |= (ZL303XX_SYNTH_FREQUENCY_MULT_MASK << ZL303XX_SYNTH_FREQUENCY_MULT_SHIFT);

              /* Return the actual output frequency */
              actualFreq = zl303xx_SynthClk0FreqInKHz(clk0FreqReg);
          }
      }
      else if (par->Id == ZL303XX_CLOCK_ID_1)
      {
          Uint32T clk0Freq, clk1DivReg;

          status = zl303xx_Read(zl303xx_Params, NULL,
                                ZL303XX_SYNTH_CLK_MULT_REG(par->synthId),
                                &regValue);

          clk0Freq = zl303xx_SynthClk0FreqInKHz(ZL303XX_EXTRACT(regValue,
                                              ZL303XX_SYNTH_FREQUENCY_MULT_MASK,
                                              ZL303XX_SYNTH_FREQUENCY_MULT_SHIFT));

          regValue = 0;
          status = zl303xx_ClosestSynthClk1DivReg(clk0Freq, par->freq, &clk1DivReg);

          if (status == ZL303XX_OK)
          {
              ZL303XX_INSERT(regValue, clk1DivReg,
                                     ZL303XX_SYNTH_FREQUENCY_DIV_MASK,
                                     ZL303XX_SYNTH_FREQUENCY_DIV_SHIFT);

              mask = (ZL303XX_SYNTH_FREQUENCY_DIV_MASK << ZL303XX_SYNTH_FREQUENCY_DIV_SHIFT);

              /* Return the actual output frequency */
              actualFreq = zl303xx_SynthClk1FreqInKHz(clk1DivReg, clk0Freq);
          }
      }
      else
      {
          status = ZL303XX_PARAMETER_INVALID;
      }

      /* Write the Data for this Register */
      if (status == ZL303XX_OK)
      {
          status = zl303xx_ReadModWrite(zl303xx_Params, NULL, ZL303XX_SYNTH_CLK_FREQ_REG(par->synthId,par->Id), regValue, mask, NULL);
      }

      if (status == ZL303XX_OK)
      {
         par->freq = actualFreq;
      }
   }

   /* Package */
   if (status == ZL303XX_OK)
   {
      regValue = 0;
      mask = 0;

      /* offset */
      ZL303XX_INSERT(regValue, par->offset,
                             ZL303XX_SYNTH_CLK_OFFSET_MASK,
                             ZL303XX_SYNTH_CLK_OFFSET_SHIFT);

      mask |= (ZL303XX_SYNTH_CLK_OFFSET_MASK << ZL303XX_SYNTH_CLK_OFFSET_SHIFT);

      /* Write the Data for this Register */
      status = zl303xx_ReadModWrite(zl303xx_Params, NULL, ZL303XX_SYNTH_CLK_OFFSET_REG(par->synthId,par->Id), regValue, mask, NULL);
   }

   /* Update the device parameters structure */
   if (status == ZL303XX_OK)
   {
      zl303xx_Params->pllParams.synth[par->synthId].clkConfig[par->Id] = *par;
   }

   return status;
} /* END zl303xx_SynthClkConfigSet */

/*

  Function Name:
   zl303xx_SynthFpConfigStructInit

  Details:
   Initializes the members of the zl303xx_SynthFpConfigS data structure

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    par            Pointer to the zl303xx_SynthFpConfigS structure to Initialize

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SynthFpConfigStructInit(zl303xx_ParamsS *zl303xx_Params,
                                          zl303xx_SynthFpConfigS *par)
{
   zlStatusE status = ZL303XX_OK;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, par);
   }

   if (status == ZL303XX_OK)
   {
      par->enable = (zl303xx_OutEnableE)ZL303XX_INVALID;
      par->run = (zl303xx_OutputRunE)ZL303XX_INVALID;
      par->freq = (zl303xx_FpFreqE)ZL303XX_INVALID;
      par->style = (zl303xx_FpStyleE)ZL303XX_INVALID;
      par->syncEdge = (zl303xx_FpSyncEdgeE)ZL303XX_INVALID;
      par->type = (zl303xx_FpTypePxE)ZL303XX_INVALID;
      par->polarity = (zl303xx_FpPolarityE)ZL303XX_INVALID;
      par->offset = (Uint32T) ZL303XX_INVALID;
      par->Id = (zl303xx_FpIdE)ZL303XX_INVALID;
      par->synthId = (zl303xx_SynthIdE)ZL303XX_INVALID;
   }

   return status;
} /* END zl303xx_SynthFpConfigStructInit */

/*

  Function Name:
   zl303xx_SynthFpConfigCheck

  Details:
   Checks the members of the zl303xx_SynthFpConfigS data structure

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    par            Pointer to the zl303xx_SynthFpConfigS structure to Check

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SynthFpConfigCheck(zl303xx_ParamsS *zl303xx_Params,
                                     zl303xx_SynthFpConfigS *par)
{
   zlStatusE status = ZL303XX_OK;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, par);
   }

   /* Check the value of the par structure members */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_OUT_ENABLE(par->enable) |
               ZL303XX_CHECK_OUTPUT_RUN(par->run) |
               ZL303XX_CHECK_FP_FREQ(par->freq) |
               ZL303XX_CHECK_FP_STYLE(par->style) |
               ZL303XX_CHECK_FP_SYNC_EDGE(par->syncEdge) |
               ZL303XX_CHECK_FP_TYPE_PX(par->type) |
               ZL303XX_CHECK_FP_POLARITY(par->polarity) |
               zl303xx_CheckFpOffset(par->offset) |
               ZL303XX_CHECK_FP_ID(par->Id) |
               ZL303XX_CHECK_SYNTH_ID(par->synthId);
   }

   return status;
} /* END zl303xx_SynthFpConfigCheck */

/*

  Function Name:
   zl303xx_SynthFpConfigGet

  Details:
   Gets the members of the zl303xx_SynthFpConfigS data structure

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    par            Pointer to the zl303xx_SynthFpConfigS structure to Get

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SynthFpConfigGet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_SynthFpConfigS *par)
{
   zlStatusE status = ZL303XX_OK;
   Uint32T regValue = 0, fine = 0, coarse = 0;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, par);
   }

   /* Check for a valid FpId */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_FP_ID(par->Id);
   }

   /* Frame pulse output only available on P0 */
   if (par->synthId != ZL303XX_SYNTH_ID_P0)
   {
       status = ZL303XX_PARAMETER_INVALID;
   }

   /* Read */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_Read(zl303xx_Params, NULL,
                            ZL303XX_SYNTH_OUTPUT_CTRL_REG(par->synthId),
                            &regValue);
   }

   /* Extract */
   if (status == ZL303XX_OK)
   {
      par->enable = (zl303xx_OutEnableE)ZL303XX_EXTRACT(regValue,
                                        ZL303XX_SYNTH_FP_ENABLE_MASK,
                                        ZL303XX_SYNTH_FP_ENABLE_SHIFT(par->Id));

      par->run = (zl303xx_OutputRunE)ZL303XX_EXTRACT(regValue,
                                      ZL303XX_SYNTH_FP_RUN_MASK,
                                      ZL303XX_SYNTH_FP_RUN_SHIFT(par->Id));
   }

   /* Read */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_Read(zl303xx_Params, NULL,
                            ZL303XX_SYNTHP0_FP_FREQ_REG(par->Id),
                            &regValue);
   }

   /* Extract */
   if (status == ZL303XX_OK)
   {
      par->freq = (zl303xx_FpFreqE)ZL303XX_EXTRACT(regValue,
                                   ZL303XX_SYNTH_FP_FREQ_MASK,
                                   ZL303XX_SYNTH_FP_FREQ_SHIFT);
   }

   /* Read */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_Read(zl303xx_Params, NULL,
                            ZL303XX_SYNTHP0_FP_TYPE_REG(par->Id),
                            &regValue);
   }

   /* Extract */
   if (status == ZL303XX_OK)
   {
      par->style = (zl303xx_FpStyleE)ZL303XX_EXTRACT(regValue,
                                     ZL303XX_SYNTH_FP_STYLE_MASK,
                                     ZL303XX_SYNTH_FP_STYLE_SHIFT);

      par->syncEdge = (zl303xx_FpSyncEdgeE)ZL303XX_EXTRACT(regValue,
                                            ZL303XX_SYNTH_FP_SYNC_EDGE_MASK,
                                            ZL303XX_SYNTH_FP_SYNC_EDGE_SHIFT);

      par->type = (zl303xx_FpTypePxE)ZL303XX_EXTRACT(regValue,
                                      ZL303XX_SYNTH_FP_TYPE_MASK,
                                      ZL303XX_SYNTH_FP_TYPE_SHIFT);

      par->polarity = (zl303xx_FpPolarityE)ZL303XX_EXTRACT(regValue,
                                            ZL303XX_SYNTH_FP_POLARITY_MASK,
                                            ZL303XX_SYNTH_FP_POLARITY_SHIFT);
   }


   /* Extract */
   if (status == ZL303XX_OK)
   {
      if (par->synthId == ZL303XX_SYNTH_ID_P0)
      {
          /* read fine offset register */
          status = zl303xx_Read(zl303xx_Params, NULL,
                                ZL303XX_SYNTHP0_FP_FINE_OFFSET_REG(par->Id),
                                &regValue);

          if (status == ZL303XX_OK)
          {
              fine = (Uint32T)ZL303XX_EXTRACT(regValue,
                                            ZL303XX_SYNTH_FP_FINE_OFFSET_MASK,
                                            ZL303XX_SYNTH_FP_FINE_OFFSET_SHIFT);
          }

          /* read coarse offset register */
          status = zl303xx_Read(zl303xx_Params, NULL,
                                ZL303XX_SYNTHP0_FP_COARSE_OFFSET_REG(par->Id),
                                &regValue);

          if (status == ZL303XX_OK)
          {
              coarse = (Uint32T)ZL303XX_EXTRACT(regValue,
                                              ZL303XX_SYNTH_FP_COARSE_OFFSET_MASK,
                                              ZL303XX_SYNTH_FP_COARSE_OFFSET_SHIFT);

              par->offset = zl303xx_SynthFpOffsetInNs(fine, coarse);
          }
      }
      else
      {
          status = ZL303XX_PARAMETER_INVALID;
      }
   }

   /* Update the device parameters structure */
   if (status == ZL303XX_OK)
   {
      zl303xx_Params->pllParams.synth[par->synthId].fpConfig[par->Id] = *par;
   }

   return status;
} /* END zl303xx_SynthFpConfigGet */

/*

  Function Name:
   zl303xx_SynthFpConfigSet

  Details:
   Sets the members of the zl303xx_SynthFpConfigS data structure

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    par            Pointer to the zl303xx_SynthFpConfigS structure to Set

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SynthFpConfigSet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_SynthFpConfigS *par)
{
   zlStatusE status = ZL303XX_OK;
   Uint32T regValue, mask;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, par);
   }

   /* Check the par values to be written */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_SynthFpConfigCheck(zl303xx_Params, par);
   }

   /* Frame pulse output only available on P0 */
   if (par->synthId != ZL303XX_SYNTH_ID_P0)
   {
      status = ZL303XX_PARAMETER_INVALID;
   }

   /* Package */
   if (status == ZL303XX_OK)
   {
      regValue = 0;
      mask = 0;

      /* enable */
      ZL303XX_INSERT(regValue, par->enable,
                             ZL303XX_SYNTH_FP_ENABLE_MASK,
                             ZL303XX_SYNTH_FP_ENABLE_SHIFT(par->Id));

      /* run */
      ZL303XX_INSERT(regValue, par->run,
                             ZL303XX_SYNTH_FP_RUN_MASK,
                             ZL303XX_SYNTH_FP_RUN_SHIFT(par->Id));

      mask |= (ZL303XX_SYNTH_FP_ENABLE_MASK << ZL303XX_SYNTH_FP_ENABLE_SHIFT(par->Id)) |
              (ZL303XX_SYNTH_FP_RUN_MASK << ZL303XX_SYNTH_FP_RUN_SHIFT(par->Id));

      /* Write the Data for this Register */
      status = zl303xx_ReadModWrite(zl303xx_Params, NULL,  ZL303XX_SYNTH_OUTPUT_CTRL_REG(par->synthId), regValue, mask, NULL);
   }

   /* Package */
   if (status == ZL303XX_OK)
   {
      regValue = 0;
      mask = 0;

      /* freq */
      ZL303XX_INSERT(regValue, par->freq,
                             ZL303XX_SYNTH_FP_FREQ_MASK,
                             ZL303XX_SYNTH_FP_FREQ_SHIFT);

      mask |= (ZL303XX_SYNTH_FP_FREQ_MASK << ZL303XX_SYNTH_FP_FREQ_SHIFT);

      /* Write the Data for this Register */
      status = zl303xx_ReadModWrite(zl303xx_Params, NULL, ZL303XX_SYNTHP0_FP_FREQ_REG(par->Id), regValue, mask, NULL);
   }

   /* Package */
   if (status == ZL303XX_OK)
   {
      regValue = 0;
      mask = 0;

      /* style */
      ZL303XX_INSERT(regValue, par->style,
                             ZL303XX_SYNTH_FP_STYLE_MASK,
                             ZL303XX_SYNTH_FP_STYLE_SHIFT);

      /* syncEdge */
      ZL303XX_INSERT(regValue, par->syncEdge,
                             ZL303XX_SYNTH_FP_SYNC_EDGE_MASK,
                             ZL303XX_SYNTH_FP_SYNC_EDGE_SHIFT);

      /* type */
      ZL303XX_INSERT(regValue, par->type,
                             ZL303XX_SYNTH_FP_TYPE_MASK,
                             ZL303XX_SYNTH_FP_TYPE_SHIFT);

      /* polarity */
      ZL303XX_INSERT(regValue, par->polarity,
                             ZL303XX_SYNTH_FP_POLARITY_MASK,
                             ZL303XX_SYNTH_FP_POLARITY_SHIFT);

      mask |= (ZL303XX_SYNTH_FP_STYLE_MASK << ZL303XX_SYNTH_FP_STYLE_SHIFT) |
              (ZL303XX_SYNTH_FP_SYNC_EDGE_MASK << ZL303XX_SYNTH_FP_SYNC_EDGE_SHIFT) |
              (ZL303XX_SYNTH_FP_TYPE_MASK << ZL303XX_SYNTH_FP_TYPE_SHIFT) |
              (ZL303XX_SYNTH_FP_POLARITY_MASK << ZL303XX_SYNTH_FP_POLARITY_SHIFT);


      /* Write the Data for this Register */
      status = zl303xx_ReadModWrite(zl303xx_Params, NULL, ZL303XX_SYNTHP0_FP_TYPE_REG(par->Id), regValue, mask, NULL);
   }

   /* Package */
   if (status == ZL303XX_OK)
   {
      Uint32T fine, coarse;

      /* offset */
      /* calculate fine and coarse offset register values */
      status = zl303xx_ClosestSynthFpOffsetReg(par->offset, &fine, &coarse);

       if (status == ZL303XX_OK)
       {
           regValue = 0;
           mask = 0;

           ZL303XX_INSERT(regValue, fine,
                                  ZL303XX_SYNTH_FP_FINE_OFFSET_MASK,
                                  ZL303XX_SYNTH_FP_FINE_OFFSET_SHIFT);

           mask |= (ZL303XX_SYNTH_FP_FINE_OFFSET_MASK << ZL303XX_SYNTH_FP_FINE_OFFSET_SHIFT);

           status = zl303xx_ReadModWrite(zl303xx_Params, NULL, ZL303XX_SYNTHP0_FP_FINE_OFFSET_REG(par->Id), regValue, mask, NULL);
       }

       if (status == ZL303XX_OK)
       {
           regValue = 0;
           mask = 0;

           ZL303XX_INSERT(regValue, coarse,
                                  ZL303XX_SYNTH_FP_COARSE_OFFSET_MASK,
                                  ZL303XX_SYNTH_FP_COARSE_OFFSET_SHIFT);

           mask |= (ZL303XX_SYNTH_FP_COARSE_OFFSET_MASK << ZL303XX_SYNTH_FP_COARSE_OFFSET_SHIFT);

           status = zl303xx_ReadModWrite(zl303xx_Params, NULL, ZL303XX_SYNTHP0_FP_COARSE_OFFSET_REG(par->Id), regValue, mask, NULL);
        }

        if (status == ZL303XX_OK)
        {
           /* Return the actual frame pulse offset */
           par->offset = zl303xx_SynthFpOffsetInNs(fine, coarse);
        }
   }

   /* Update the device parameters structure */
   if (status == ZL303XX_OK)
   {
      zl303xx_Params->pllParams.synth[par->synthId].fpConfig[par->Id] = *par;
   }

   return status;
} /* END zl303xx_SynthFpConfigSet */

/*

  Function Name:
   zl303xx_SynthConfigStructInit

  Details:
   Initializes the members of the zl303xx_SynthConfigS data structure

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    par            Pointer to the zl303xx_SynthConfigS structure to Initialize

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SynthConfigStructInit(zl303xx_ParamsS *zl303xx_Params,
                                        zl303xx_SynthConfigS *par)
{
   zlStatusE status = ZL303XX_OK;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, par);
   }

   if (status == ZL303XX_OK)
   {
      par->source = (zl303xx_DpllIdE)ZL303XX_INVALID;
      par->enable = (zl303xx_BooleanE)ZL303XX_INVALID;
      par->fineDelay = ZL303XX_INVALID;
      par->Id = (zl303xx_SynthIdE)ZL303XX_INVALID;
   }

   return status;
} /* END zl303xx_SynthConfigStructInit */

/*

  Function Name:
   zl303xx_SynthConfigCheck

  Details:
   Checks the members of the zl303xx_SynthConfigS data structure

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    par            Pointer to the zl303xx_SynthConfigS structure to Check

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SynthConfigCheck(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_SynthConfigS *par)
{
   zlStatusE status = ZL303XX_OK;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, par);
   }

   /* Check the value of the par structure members */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_DPLL_ID(par->source) |
               ZL303XX_CHECK_BOOLEAN(par->enable) |
               zl303xx_CheckOutputFineOffset(par->fineDelay) |
               ZL303XX_CHECK_SYNTH_ID(par->Id);
   }

   return status;
} /* END zl303xx_SynthConfigCheck */

/*

  Function Name:
   zl303xx_SynthConfigGet

  Details:
   Gets the members of the zl303xx_SynthConfigS data structure

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    par            Pointer to the zl303xx_SynthConfigS structure to Get

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SynthConfigGet(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_SynthConfigS *par)
{
   zlStatusE status = ZL303XX_OK;
   Uint32T regValue = 0;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, par);
   }

   /* Check for a valid SynthId */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_SYNTH_ID(par->Id);
   }

   /* Read */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_Read(zl303xx_Params, NULL,
                            ZL303XX_SYNTH_OUTPUT_CTRL_REG(par->Id),
                            &regValue);
   }

   /* Extract */
   if (status == ZL303XX_OK)
   {
      par->source = (zl303xx_DpllIdE)ZL303XX_EXTRACT(regValue,
                                     ZL303XX_SYNTH_IN_SOURCE_MASK,
                                     ZL303XX_SYNTH_IN_SOURCE_SHIFT);

      par->enable = (zl303xx_BooleanE)ZL303XX_EXTRACT(regValue,
                                       ZL303XX_SYNTH_ENABLE_MASK,
                                       ZL303XX_SYNTH_ENABLE_SHIFT);
   }

   /* Read */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_Read(zl303xx_Params, NULL,
                            ZL303XX_SYNTH_FINE_DELAY_REG(par->Id),
                            &regValue);
   }

   /* Extract */
   if (status == ZL303XX_OK)
   {
      par->fineDelay = zl303xx_OutputFineOffsetInPs(regValue);
   }

   /* Update the device parameters structure */
   if (status == ZL303XX_OK)
   {
      zl303xx_Params->pllParams.synth[par->Id].config = *par;
   }

   return status;
} /* END zl303xx_SynthConfigGet */

/*

  Function Name:
   zl303xx_SynthConfigSet

  Details:
   Sets the members of the zl303xx_SynthConfigS data structure

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    par            Pointer to the zl303xx_SynthConfigS structure to Set

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SynthConfigSet(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_SynthConfigS *par)
{
   zlStatusE status = ZL303XX_OK;
   Uint32T regValue, mask;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, par);
   }

   /* Check the par values to be written */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_SynthConfigCheck(zl303xx_Params, par);
   }

   /* Package */
   if (status == ZL303XX_OK)
   {
      regValue = 0;
      mask = 0;

      /* source */
      ZL303XX_INSERT(regValue, par->source,
                           ZL303XX_SYNTH_IN_SOURCE_MASK,
                           ZL303XX_SYNTH_IN_SOURCE_SHIFT);

      /* enable */
      ZL303XX_INSERT(regValue, par->enable,
                           ZL303XX_SYNTH_ENABLE_MASK,
                           ZL303XX_SYNTH_ENABLE_SHIFT);

      mask |= (ZL303XX_SYNTH_IN_SOURCE_MASK << ZL303XX_SYNTH_IN_SOURCE_SHIFT) |
              (ZL303XX_SYNTH_ENABLE_MASK << ZL303XX_SYNTH_ENABLE_SHIFT);

      /* Write the Data for this Register */
      status = zl303xx_ReadModWrite(zl303xx_Params, NULL, ZL303XX_SYNTH_OUTPUT_CTRL_REG(par->Id), regValue, mask, NULL);
   }

   /* Package */
   if (status == ZL303XX_OK)
   {
      Uint32T fineDelayReg;
      regValue = 0;
      mask = 0;

      /* fineDelay */
      status = zl303xx_ClosestOutputFineOffsetReg(par->fineDelay, &fineDelayReg);

      if (status == ZL303XX_OK)
      {
          ZL303XX_INSERT(regValue, fineDelayReg,
                                 ZL303XX_SYNTH_FINE_DELAY_MASK,
                                 ZL303XX_SYNTH_FINE_DELAY_SHIFT);

          mask |= (ZL303XX_SYNTH_FINE_DELAY_MASK << ZL303XX_SYNTH_FINE_DELAY_SHIFT);

          /* Write the Data for this Register */
          status = zl303xx_ReadModWrite(zl303xx_Params, NULL, ZL303XX_SYNTH_FINE_DELAY_REG(par->Id), regValue, mask, NULL);
      }

      /* Return the actual offset for the px synth */
      if (status == ZL303XX_OK)
      {
         par->fineDelay = zl303xx_OutputFineOffsetInPs(fineDelayReg);
      }
   }

   /* Update the device parameters structure */
   if (status == ZL303XX_OK)
   {
      zl303xx_Params->pllParams.synth[par->Id].config = *par;
   }

   return status;
} /* END zl303xx_SynthConfigSet */

/*

  Function Name:
   zl303xx_SynthClkEnableGet

  Details:
   Reads the Clk Enable attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    synthId        Associated SYNTH Id of the attribute
   [in]    clockId        Associated Clock Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SynthClkEnableGet(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_SynthIdE synthId,
                                    zl303xx_ClockIdE clockId,
                                    zl303xx_OutEnableE *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
   }

   /* Check the synthId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_SYNTH_ID(synthId);
   }

   /* Check the clockId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_CLOCK_ID(clockId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_SYNTH_OUTPUT_CTRL_REG(synthId),
                              ZL303XX_SYNTH_CLK_ENABLE_MASK,
                              ZL303XX_SYNTH_CLK_ENABLE_SHIFT(clockId));
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_OutEnableE)(attrPar.value);
   }

   /* Update the device parameters structure */
   if (status == ZL303XX_OK)
   {
      zl303xx_Params->pllParams.synth[synthId].clkConfig[clockId].enable = *val;
   }

   return status;
}  /* END zl303xx_SynthClkEnableGet */

/*

  Function Name:
   zl303xx_SynthClkEnableSet

  Details:
   Writes the Clk Enable attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    synthId        Associated SYNTH Id of the attribute
   [in]    clockId        Associated Clock Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SynthClkEnableSet(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_SynthIdE synthId,
                                    zl303xx_ClockIdE clockId,
                                    zl303xx_OutEnableE val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check params pointer */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
   }

   /* Check the synthId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_SYNTH_ID(synthId);
   }

   /* Check the clockId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_CLOCK_ID(clockId);
   }

   /* Check that the write value is within range */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_OUT_ENABLE(val);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_SYNTH_OUTPUT_CTRL_REG(synthId),
                              ZL303XX_SYNTH_CLK_ENABLE_MASK,
                              ZL303XX_SYNTH_CLK_ENABLE_SHIFT(clockId));
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   /* Update the device parameters structure */
   if (status == ZL303XX_OK)
   {
      zl303xx_Params->pllParams.synth[synthId].clkConfig[clockId].enable = val;
   }

   return status;
}  /* END zl303xx_SynthClkEnableSet */


/*

  Function Name:
   zl303xx_SynthFpEnableGet

  Details:
   Reads the Fp Enable attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    synthId        Associated SYNTH Id of the attribute
   [in]    fpId           Associated Fp Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SynthFpEnableGet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_SynthIdE synthId,
                                   zl303xx_FpIdE fpId,
                                   zl303xx_OutEnableE *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
   }

   /* Check the synthId parameter */
   if (status == ZL303XX_OK)
   {
      if (synthId != ZL303XX_SYNTH_ID_P0)
      {
         status = ZL303XX_PARAMETER_INVALID;
      }
   }

   /* Check the fpId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_FP_ID(fpId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_SYNTH_OUTPUT_CTRL_REG(synthId),
                              ZL303XX_SYNTH_FP_ENABLE_MASK,
                              ZL303XX_SYNTH_FP_ENABLE_SHIFT(fpId));
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_OutEnableE)(attrPar.value);
   }

   /* Update the device parameters structure */
   if (status == ZL303XX_OK)
   {
      zl303xx_Params->pllParams.synth[synthId].fpConfig[fpId].enable = *val;
   }

   return status;
}  /* END zl303xx_SynthFpEnableGet */

/*

  Function Name:
   zl303xx_SynthFpEnableSet

  Details:
   Writes the Fp Enable attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    synthId        Associated SYNTH Id of the attribute
   [in]    fpId           Associated Fp Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SynthFpEnableSet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_SynthIdE synthId,
                                   zl303xx_FpIdE fpId,
                                   zl303xx_OutEnableE val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check params pointer */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
   }

   /* Check the synthId parameter */
   if (status == ZL303XX_OK)
   {
      if (synthId != ZL303XX_SYNTH_ID_P0)
      {
         status = ZL303XX_PARAMETER_INVALID;
      }
   }

   /* Check the fpId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_FP_ID(fpId);
   }

   /* Check that the write value is within range */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_OUT_ENABLE(val);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_SYNTH_OUTPUT_CTRL_REG(synthId),
                              ZL303XX_SYNTH_FP_ENABLE_MASK,
                              ZL303XX_SYNTH_FP_ENABLE_SHIFT(fpId));
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   /* Update the device parameters structure */
   if (status == ZL303XX_OK)
   {
      zl303xx_Params->pllParams.synth[synthId].fpConfig[fpId].enable = val;
   }

   return status;
}  /* END zl303xx_SynthFpEnableSet */


/*

  Function Name:
   zl303xx_SynthInSourceGet

  Details:
   Reads the Dpll Source attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    synthId        Associated SYNTH Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SynthInSourceGet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_SynthIdE synthId,
                                   zl303xx_DpllIdE *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
   }

   /* Check the synthId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_SYNTH_ID(synthId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_SYNTH_OUTPUT_CTRL_REG(synthId),
                              ZL303XX_SYNTH_IN_SOURCE_MASK,
                              ZL303XX_SYNTH_IN_SOURCE_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_DpllIdE)(attrPar.value);
   }

   /* Update the device parameters structure */
   if (status == ZL303XX_OK)
   {
      zl303xx_Params->pllParams.synth[synthId].config.source = *val;
   }

   return status;
}  /* END zl303xx_SynthInSourceGet */

/*

  Function Name:
   zl303xx_SynthInSourceSet

  Details:
   Writes the Dpll Source attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    synthId        Associated SYNTH Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SynthInSourceSet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_SynthIdE synthId,
                                   zl303xx_DpllIdE val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check params pointer */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
   }

   /* Check the synthId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_SYNTH_ID(synthId);
   }

   /* Check that the write value is within range */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_DPLL_ID(val);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_SYNTH_OUTPUT_CTRL_REG(synthId),
                              ZL303XX_SYNTH_IN_SOURCE_MASK,
                              ZL303XX_SYNTH_IN_SOURCE_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   /* Update the device parameters structure */
   if (status == ZL303XX_OK)
   {
      zl303xx_Params->pllParams.synth[synthId].config.source = val;
   }

   return status;
}  /* END zl303xx_SynthInSourceSet */


/*

  Function Name:
   zl303xx_SynthEnableGet

  Details:
   Reads the PLL Enable attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    synthId        Associated SYNTH Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SynthEnableGet(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_SynthIdE synthId,
                                 zl303xx_BooleanE *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
   }

   /* Check the synthId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_SYNTH_ID(synthId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_SYNTH_OUTPUT_CTRL_REG(synthId),
                              ZL303XX_SYNTH_ENABLE_MASK,
                              ZL303XX_SYNTH_ENABLE_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_BooleanE)(attrPar.value);
   }

   /* Update the device parameters structure */
   if (status == ZL303XX_OK)
   {
      zl303xx_Params->pllParams.synth[synthId].config.enable = *val;
   }

   return status;
}  /* END zl303xx_SynthEnableGet */

/*

  Function Name:
   zl303xx_SynthEnableSet

  Details:
   Writes the PLL Enable attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    synthId        Associated SYNTH Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SynthEnableSet(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_SynthIdE synthId,
                                 zl303xx_BooleanE val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check params pointer */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
   }

   /* Check the synthId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_SYNTH_ID(synthId);
   }

   /* Check that the write value is within range */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_BOOLEAN(val);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_SYNTH_OUTPUT_CTRL_REG(synthId),
                              ZL303XX_SYNTH_ENABLE_MASK,
                              ZL303XX_SYNTH_ENABLE_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   /* Update the device parameters structure */
   if (status == ZL303XX_OK)
   {
      zl303xx_Params->pllParams.synth[synthId].config.enable = val;
   }

   return status;
}  /* END zl303xx_SynthEnableSet */


/*

  Function Name:
   zl303xx_SynthClkRunGet

  Details:
   Reads the Clk Run attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    synthId        Associated SYNTH Id of the attribute
   [in]    clockId        Associated Clock Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SynthClkRunGet(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_SynthIdE synthId,
                                 zl303xx_ClockIdE clockId,
                                 zl303xx_OutputRunE *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
   }

   /* Check the synthId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_SYNTH_ID(synthId);
   }

   /* Check the clockId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_CLOCK_ID(clockId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_SYNTH_OUTPUT_CTRL_REG(synthId),
                              ZL303XX_SYNTH_CLK_RUN_MASK,
                              ZL303XX_SYNTH_CLK_RUN_SHIFT(clockId));
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_OutputRunE)(attrPar.value);
   }

   /* Update the device parameters structure */
   if (status == ZL303XX_OK)
   {
      zl303xx_Params->pllParams.synth[synthId].clkConfig[clockId].run = *val;
   }

   return status;
}  /* END zl303xx_SynthClkRunGet */

/*

  Function Name:
   zl303xx_SynthClkRunSet

  Details:
   Writes the Clk Run attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    synthId        Associated SYNTH Id of the attribute
   [in]    clockId        Associated Clock Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SynthClkRunSet(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_SynthIdE synthId,
                                 zl303xx_ClockIdE clockId,
                                 zl303xx_OutputRunE val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check params pointer */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
   }

   /* Check the synthId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_SYNTH_ID(synthId);
   }

   /* Check the clockId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_CLOCK_ID(clockId);
   }

   /* Check that the write value is within range */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_OUTPUT_RUN(val);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_SYNTH_OUTPUT_CTRL_REG(synthId),
                              ZL303XX_SYNTH_CLK_RUN_MASK,
                              ZL303XX_SYNTH_CLK_RUN_SHIFT(clockId));
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   if (status == ZL303XX_OK)
   {
      zl303xx_Params->pllParams.synth[synthId].clkConfig[clockId].run = val;
   }

   return status;
}  /* END zl303xx_SynthClkRunSet */


/*

  Function Name:
   zl303xx_SynthFpRunGet

  Details:
   Reads the Fp Run attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    synthId        Associated SYNTH Id of the attribute
   [in]    fpId           Associated Fp Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SynthFpRunGet(zl303xx_ParamsS *zl303xx_Params,
                                zl303xx_SynthIdE synthId,
                                zl303xx_FpIdE fpId,
                                zl303xx_OutputRunE *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
   }

   /* Check the synthId parameter */
   if (status == ZL303XX_OK)
   {
      if (synthId != ZL303XX_SYNTH_ID_P0)
      {
         status = ZL303XX_PARAMETER_INVALID;
      }
   }

   /* Check the fpId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_FP_ID(fpId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_SYNTH_OUTPUT_CTRL_REG(synthId),
                              ZL303XX_SYNTH_FP_RUN_MASK,
                              ZL303XX_SYNTH_FP_RUN_SHIFT(fpId));
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_OutputRunE)(attrPar.value);
   }

   /* Update the device parameters structure */
   if (status == ZL303XX_OK)
   {
      zl303xx_Params->pllParams.synth[synthId].fpConfig[fpId].run = *val;
   }

   return status;
}  /* END zl303xx_SynthFpRunGet */

/*

  Function Name:
   zl303xx_SynthFpRunSet

  Details:
   Writes the Fp Run attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    synthId        Associated SYNTH Id of the attribute
   [in]    fpId           Associated Fp Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SynthFpRunSet(zl303xx_ParamsS *zl303xx_Params,
                                zl303xx_SynthIdE synthId,
                                zl303xx_FpIdE fpId,
                                zl303xx_OutputRunE val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check params pointer */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
   }

   /* Check the synthId parameter */
   if (status == ZL303XX_OK)
   {
      if (synthId != ZL303XX_SYNTH_ID_P0)
      {
         status = ZL303XX_PARAMETER_INVALID;
      }
   }

   /* Check the fpId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_FP_ID(fpId);
   }

   /* Check that the write value is within range */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_OUTPUT_RUN(val);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_SYNTH_OUTPUT_CTRL_REG(synthId),
                              ZL303XX_SYNTH_FP_RUN_MASK,
                              ZL303XX_SYNTH_FP_RUN_SHIFT(fpId));
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   /* Update the device parameters structure */
   if (status == ZL303XX_OK)
   {
      zl303xx_Params->pllParams.synth[synthId].fpConfig[fpId].run = val;
   }

   return status;
}  /* END zl303xx_SynthFpRunSet */


/*

  Function Name:
   zl303xx_SynthFrequencyMultGet

  Details:
   Reads the Nx8kHz Mult attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    synthId        Associated SYNTH Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SynthFrequencyMultGet(zl303xx_ParamsS *zl303xx_Params,
                                        zl303xx_SynthIdE synthId,
                                        Uint32T *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
   }

   /* Check the synthId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_SYNTH_ID(synthId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_SYNTH_CLK_MULT_REG(synthId),
                              ZL303XX_SYNTH_FREQUENCY_MULT_MASK,
                              ZL303XX_SYNTH_FREQUENCY_MULT_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (Uint32T)(attrPar.value);
   }

   /* Update the device parameters structure */
   if (status == ZL303XX_OK)
   {
      zl303xx_Params->pllParams.synth[synthId].clkConfig[ZL303XX_CLOCK_ID_0].freq = zl303xx_SynthClk0FreqInKHz(*val);
   }

   return status;
}  /* END zl303xx_SynthFrequencyMultGet */

/*

  Function Name:
   zl303xx_SynthFrequencyMultSet

  Details:
   Writes the Nx8kHz Mult attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    synthId        Associated SYNTH Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SynthFrequencyMultSet(zl303xx_ParamsS *zl303xx_Params,
                                        zl303xx_SynthIdE synthId,
                                        Uint32T val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check params pointer */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
   }

   /* Check the synthId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_SYNTH_ID(synthId);
   }

   /* Check that the write value is within range */
   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_SYNTH_CLK_MULT_REG(synthId),
                              ZL303XX_SYNTH_FREQUENCY_MULT_MASK,
                              ZL303XX_SYNTH_FREQUENCY_MULT_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   /* Update the device parameters structure */
   if (status == ZL303XX_OK)
   {
      zl303xx_Params->pllParams.synth[synthId].clkConfig[ZL303XX_CLOCK_ID_0].freq = zl303xx_SynthClk0FreqInKHz(val);
   }

   return status;
}  /* END zl303xx_SynthFrequencyMultSet */


/*

  Function Name:
   zl303xx_SynthClkOffsetGet

  Details:
   Reads the Clk Offset attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    synthId        Associated SYNTH Id of the attribute
   [in]    clockId        Associated Clock Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SynthClkOffsetGet(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_SynthIdE synthId,
                                    zl303xx_ClockIdE clockId,
                                    zl303xx_ClkOffsetE *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
   }

   /* Check the synthId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_SYNTH_ID(synthId);
   }

   /* Check the clockId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_CLOCK_ID(clockId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_SYNTH_CLK_OFFSET_REG(synthId,clockId),
                              ZL303XX_SYNTH_CLK_OFFSET_MASK,
                              ZL303XX_SYNTH_CLK_OFFSET_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_ClkOffsetE)(attrPar.value);
   }

   /* Update the device parameters structure */
   if (status == ZL303XX_OK)
   {
      zl303xx_Params->pllParams.synth[synthId].clkConfig[clockId].offset = *val;
   }

   return status;
}  /* END zl303xx_SynthClkOffsetGet */

/*

  Function Name:
   zl303xx_SynthClkOffsetSet

  Details:
   Writes the Clk Offset attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    synthId        Associated SYNTH Id of the attribute
   [in]    clockId        Associated Clock Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SynthClkOffsetSet(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_SynthIdE synthId,
                                    zl303xx_ClockIdE clockId,
                                    zl303xx_ClkOffsetE val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check params pointer */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
   }

   /* Check the synthId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_SYNTH_ID(synthId);
   }

   /* Check the clockId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_CLOCK_ID(clockId);
   }

   /* Check that the write value is within range */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_CLK_OFFSET(val);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_SYNTH_CLK_OFFSET_REG(synthId,clockId),
                              ZL303XX_SYNTH_CLK_OFFSET_MASK,
                              ZL303XX_SYNTH_CLK_OFFSET_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   /* Update the device parameters structure */
   if (status == ZL303XX_OK)
   {
      zl303xx_Params->pllParams.synth[synthId].clkConfig[clockId].offset = val;
   }

   return status;
}  /* END zl303xx_SynthClkOffsetSet */


/*

  Function Name:
   zl303xx_SynthFrequencyDivGet

  Details:
   Reads the Clk Divider attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    synthId        Associated SYNTH Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SynthFrequencyDivGet(zl303xx_ParamsS *zl303xx_Params,
                                       zl303xx_SynthIdE synthId,
                                       Uint32T *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
   }

   /* Check the synthId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_SYNTH_ID(synthId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_SYNTH_CLK_DIV_REG(synthId),
                              ZL303XX_SYNTH_FREQUENCY_DIV_MASK,
                              ZL303XX_SYNTH_FREQUENCY_DIV_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (Uint32T)(attrPar.value);
   }

   if (status == ZL303XX_OK)
   {
      /* Assume the px_clk0 freq stored in zl303xx_Params is correct */
      zl303xx_Params->pllParams.synth[synthId].clkConfig[ZL303XX_CLOCK_ID_1].freq =
         zl303xx_SynthClk1FreqInKHz(*val, zl303xx_Params->pllParams.synth[synthId].clkConfig[ZL303XX_CLOCK_ID_0].freq);
   }

   return status;
}  /* END zl303xx_SynthFrequencyDivGet */

/*

  Function Name:
   zl303xx_SynthFrequencyDivSet

  Details:
   Writes the Clk Divider attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    synthId        Associated SYNTH Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SynthFrequencyDivSet(zl303xx_ParamsS *zl303xx_Params,
                                       zl303xx_SynthIdE synthId,
                                       Uint32T val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check params pointer */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
   }

   /* Check the synthId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_SYNTH_ID(synthId);
   }

   /* Check that the write value is within range */
   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_SYNTH_CLK_DIV_REG(synthId),
                              ZL303XX_SYNTH_FREQUENCY_DIV_MASK,
                              ZL303XX_SYNTH_FREQUENCY_DIV_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   if (status == ZL303XX_OK)
   {
      /* Assume the px_clk0 freq stored in zl303xx_Params is correct */
      zl303xx_Params->pllParams.synth[synthId].clkConfig[ZL303XX_CLOCK_ID_1].freq =
         zl303xx_SynthClk1FreqInKHz(val, zl303xx_Params->pllParams.synth[synthId].clkConfig[ZL303XX_CLOCK_ID_0].freq);
   }

   return status;
}  /* END zl303xx_SynthFrequencyDivSet */


/*

  Function Name:
   zl303xx_SynthFineDelayGet

  Details:
   Reads the Fine Tune Delay attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    synthId        Associated SYNTH Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SynthFineDelayGet(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_SynthIdE synthId,
                                    Uint32T *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
   }

   /* Check the synthId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_SYNTH_ID(synthId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_SYNTH_FINE_DELAY_REG(synthId),
                              ZL303XX_SYNTH_FINE_DELAY_MASK,
                              ZL303XX_SYNTH_FINE_DELAY_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (Uint32T)(attrPar.value);
   }

   /* Update the device parameters structure */
   if (status == ZL303XX_OK)
   {
      zl303xx_Params->pllParams.synth[synthId].config.fineDelay = zl303xx_OutputFineOffsetInPs(*val);
   }

   return status;
}  /* END zl303xx_SynthFineDelayGet */

/*

  Function Name:
   zl303xx_SynthFineDelaySet

  Details:
   Writes the Fine Tune Delay attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    synthId        Associated SYNTH Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SynthFineDelaySet(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_SynthIdE synthId,
                                    Uint32T val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check params pointer */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
   }

   /* Check the synthId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_SYNTH_ID(synthId);
   }

   /* Check that the write value is within range */
   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_SYNTH_FINE_DELAY_REG(synthId),
                              ZL303XX_SYNTH_FINE_DELAY_MASK,
                              ZL303XX_SYNTH_FINE_DELAY_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   /* Update the device parameters structure */
   if (status == ZL303XX_OK)
   {
      zl303xx_Params->pllParams.synth[synthId].config.fineDelay = zl303xx_OutputFineOffsetInPs(val);
   }

   return status;
}  /* END zl303xx_SynthFineDelaySet */


/*

  Function Name:
   zl303xx_SynthFpFreqGet

  Details:
   Reads the Fp Freq attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    synthId        Associated SYNTH Id of the attribute
   [in]    fpId           Associated Fp Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SynthFpFreqGet(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_SynthIdE synthId,
                                 zl303xx_FpIdE fpId,
                                 zl303xx_FpFreqE *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
   }

   /* Check the synthId parameter */
   if (status == ZL303XX_OK)
   {
      if (synthId != ZL303XX_SYNTH_ID_P0)
      {
         status = ZL303XX_PARAMETER_INVALID;
      }
   }

   /* Check the fpId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_FP_ID(fpId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_SYNTHP0_FP_FREQ_REG(fpId),
                              ZL303XX_SYNTH_FP_FREQ_MASK,
                              ZL303XX_SYNTH_FP_FREQ_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_FpFreqE)(attrPar.value);
   }

   /* Update the device parameters structure */
   if (status == ZL303XX_OK)
   {
      zl303xx_Params->pllParams.synth[synthId].fpConfig[fpId].freq = *val;
   }

   return status;
}  /* END zl303xx_SynthFpFreqGet */

/*

  Function Name:
   zl303xx_SynthFpFreqSet

  Details:
   Writes the Fp Freq attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    synthId        Associated SYNTH Id of the attribute
   [in]    fpId           Associated Fp Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SynthFpFreqSet(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_SynthIdE synthId,
                                 zl303xx_FpIdE fpId,
                                 zl303xx_FpFreqE val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check params pointer */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
   }

   /* Check the synthId parameter */
   if (status == ZL303XX_OK)
   {
      if (synthId != ZL303XX_SYNTH_ID_P0)
      {
         status = ZL303XX_PARAMETER_INVALID;
      }
   }

   /* Check the fpId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_FP_ID(fpId);
   }

   /* Check that the write value is within range */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_FP_FREQ(val);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_SYNTHP0_FP_FREQ_REG(fpId),
                              ZL303XX_SYNTH_FP_FREQ_MASK,
                              ZL303XX_SYNTH_FP_FREQ_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   /* Update the device parameters structure */
   if (status == ZL303XX_OK)
   {
      zl303xx_Params->pllParams.synth[synthId].fpConfig[fpId].freq = val;
   }

   return status;
}  /* END zl303xx_SynthFpFreqSet */


/*

  Function Name:
   zl303xx_SynthFpStyleGet

  Details:
   Reads the Fp Style attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    synthId        Associated SYNTH Id of the attribute
   [in]    fpId           Associated Fp Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SynthFpStyleGet(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_SynthIdE synthId,
                                  zl303xx_FpIdE fpId,
                                  zl303xx_FpStyleE *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
   }

   /* Check the synthId parameter */
   if (status == ZL303XX_OK)
   {
      if (synthId != ZL303XX_SYNTH_ID_P0)
      {
         status = ZL303XX_PARAMETER_INVALID;
      }
   }

   /* Check the fpId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_FP_ID(fpId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_SYNTHP0_FP_TYPE_REG(fpId),
                              ZL303XX_SYNTH_FP_STYLE_MASK,
                              ZL303XX_SYNTH_FP_STYLE_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_FpStyleE)(attrPar.value);
   }

   /* Update the device parameters structure */
   if (status == ZL303XX_OK)
   {
      zl303xx_Params->pllParams.synth[synthId].fpConfig[fpId].style = *val;
   }

   return status;
}  /* END zl303xx_SynthFpStyleGet */

/*

  Function Name:
   zl303xx_SynthFpStyleSet

  Details:
   Writes the Fp Style attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    synthId        Associated SYNTH Id of the attribute
   [in]    fpId           Associated Fp Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SynthFpStyleSet(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_SynthIdE synthId,
                                  zl303xx_FpIdE fpId,
                                  zl303xx_FpStyleE val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check params pointer */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
   }

   /* Check the synthId parameter */
   if (status == ZL303XX_OK)
   {
      if (synthId != ZL303XX_SYNTH_ID_P0)
      {
         status = ZL303XX_PARAMETER_INVALID;
      }
   }

   /* Check the fpId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_FP_ID(fpId);
   }

   /* Check that the write value is within range */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_FP_STYLE(val);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_SYNTHP0_FP_TYPE_REG(fpId),
                              ZL303XX_SYNTH_FP_STYLE_MASK,
                              ZL303XX_SYNTH_FP_STYLE_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   /* Update the device parameters structure */
   if (status == ZL303XX_OK)
   {
      zl303xx_Params->pllParams.synth[synthId].fpConfig[fpId].style = val;
   }

   return status;
}  /* END zl303xx_SynthFpStyleSet */


/*

  Function Name:
   zl303xx_SynthFpSyncEdgeGet

  Details:
   Reads the Fp Sync Edge attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    synthId        Associated SYNTH Id of the attribute
   [in]    fpId           Associated Fp Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SynthFpSyncEdgeGet(zl303xx_ParamsS *zl303xx_Params,
                                     zl303xx_SynthIdE synthId,
                                     zl303xx_FpIdE fpId,
                                     zl303xx_FpSyncEdgeE *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
   }

   /* Check the synthId parameter */
   if (status == ZL303XX_OK)
   {
      if (synthId != ZL303XX_SYNTH_ID_P0)
      {
         status = ZL303XX_PARAMETER_INVALID;
      }
   }

   /* Check the fpId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_FP_ID(fpId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_SYNTHP0_FP_TYPE_REG(fpId),
                              ZL303XX_SYNTH_FP_SYNC_EDGE_MASK,
                              ZL303XX_SYNTH_FP_SYNC_EDGE_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_FpSyncEdgeE)(attrPar.value);
   }

   /* Update the device parameters structure */
   if (status == ZL303XX_OK)
   {
      zl303xx_Params->pllParams.synth[synthId].fpConfig[fpId].syncEdge = *val;
   }

   return status;
}  /* END zl303xx_SynthFpSyncEdgeGet */

/*

  Function Name:
   zl303xx_SynthFpSyncEdgeSet

  Details:
   Writes the Fp Sync Edge attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    synthId        Associated SYNTH Id of the attribute
   [in]    fpId           Associated Fp Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SynthFpSyncEdgeSet(zl303xx_ParamsS *zl303xx_Params,
                                     zl303xx_SynthIdE synthId,
                                     zl303xx_FpIdE fpId,
                                     zl303xx_FpSyncEdgeE val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check params pointer */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
   }

   /* Check the synthId parameter */
   if (status == ZL303XX_OK)
   {
      if (synthId != ZL303XX_SYNTH_ID_P0)
      {
         status = ZL303XX_PARAMETER_INVALID;
      }
   }

   /* Check the fpId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_FP_ID(fpId);
   }

   /* Check that the write value is within range */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_FP_SYNC_EDGE(val);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_SYNTHP0_FP_TYPE_REG(fpId),
                              ZL303XX_SYNTH_FP_SYNC_EDGE_MASK,
                              ZL303XX_SYNTH_FP_SYNC_EDGE_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   /* Update the device parameters structure */
   if (status == ZL303XX_OK)
   {
      zl303xx_Params->pllParams.synth[synthId].fpConfig[fpId].syncEdge = val;
   }

   return status;
}  /* END zl303xx_SynthFpSyncEdgeSet */


/*

  Function Name:
   zl303xx_SynthFpTypeGet

  Details:
   Reads the Fp Type attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    synthId        Associated SYNTH Id of the attribute
   [in]    fpId           Associated Fp Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SynthFpTypeGet(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_SynthIdE synthId,
                                 zl303xx_FpIdE fpId,
                                 zl303xx_FpTypePxE *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
   }

   /* Check the synthId parameter */
   if (status == ZL303XX_OK)
   {
      if (synthId != ZL303XX_SYNTH_ID_P0)
      {
         status = ZL303XX_PARAMETER_INVALID;
      }
   }

   /* Check the fpId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_FP_ID(fpId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_SYNTHP0_FP_TYPE_REG(fpId),
                              ZL303XX_SYNTH_FP_TYPE_MASK,
                              ZL303XX_SYNTH_FP_TYPE_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_FpTypePxE)(attrPar.value);
   }

   /* Update the device parameters structure */
   if (status == ZL303XX_OK)
   {
      zl303xx_Params->pllParams.synth[synthId].fpConfig[fpId].type = *val;
   }

   return status;
}  /* END zl303xx_SynthFpTypeGet */

/*

  Function Name:
   zl303xx_SynthFpTypeSet

  Details:
   Writes the Fp Type attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    synthId        Associated SYNTH Id of the attribute
   [in]    fpId           Associated Fp Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SynthFpTypeSet(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_SynthIdE synthId,
                                 zl303xx_FpIdE fpId,
                                 zl303xx_FpTypePxE val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check params pointer */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
   }

   /* Check the synthId parameter */
   if (status == ZL303XX_OK)
   {
      if (synthId != ZL303XX_SYNTH_ID_P0)
      {
         status = ZL303XX_PARAMETER_INVALID;
      }
   }

   /* Check the fpId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_FP_ID(fpId);
   }

   /* Check that the write value is within range */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_FP_TYPE_PX(val);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_SYNTHP0_FP_TYPE_REG(fpId),
                              ZL303XX_SYNTH_FP_TYPE_MASK,
                              ZL303XX_SYNTH_FP_TYPE_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   /* Update the device parameters structure */
   if (status == ZL303XX_OK)
   {
      zl303xx_Params->pllParams.synth[synthId].fpConfig[fpId].type = val;
   }

   return status;
}  /* END zl303xx_SynthFpTypeSet */


/*

  Function Name:
   zl303xx_SynthFpPolarityGet

  Details:
   Reads the Fp Polarity attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    synthId        Associated SYNTH Id of the attribute
   [in]    fpId           Associated Fp Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SynthFpPolarityGet(zl303xx_ParamsS *zl303xx_Params,
                                     zl303xx_SynthIdE synthId,
                                     zl303xx_FpIdE fpId,
                                     zl303xx_FpPolarityE *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
   }

   /* Check the synthId parameter */
   if (status == ZL303XX_OK)
   {
      if (synthId != ZL303XX_SYNTH_ID_P0)
      {
         status = ZL303XX_PARAMETER_INVALID;
      }
   }

   /* Check the fpId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_FP_ID(fpId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_SYNTHP0_FP_TYPE_REG(fpId),
                              ZL303XX_SYNTH_FP_POLARITY_MASK,
                              ZL303XX_SYNTH_FP_POLARITY_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_FpPolarityE)(attrPar.value);
   }

   /* Update the device parameters structure */
   if (status == ZL303XX_OK)
   {
      zl303xx_Params->pllParams.synth[synthId].fpConfig[fpId].polarity = *val;
   }

   return status;
}  /* END zl303xx_SynthFpPolarityGet */

/*

  Function Name:
   zl303xx_SynthFpPolaritySet

  Details:
   Writes the Fp Polarity attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    synthId        Associated SYNTH Id of the attribute
   [in]    fpId           Associated Fp Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SynthFpPolaritySet(zl303xx_ParamsS *zl303xx_Params,
                                     zl303xx_SynthIdE synthId,
                                     zl303xx_FpIdE fpId,
                                     zl303xx_FpPolarityE val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check params pointer */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
   }

   /* Check the synthId parameter */
   if (status == ZL303XX_OK)
   {
      if (synthId != ZL303XX_SYNTH_ID_P0)
      {
         status = ZL303XX_PARAMETER_INVALID;
      }
   }

   /* Check the fpId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_FP_ID(fpId);
   }

   /* Check that the write value is within range */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_FP_POLARITY(val);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_SYNTHP0_FP_TYPE_REG(fpId),
                              ZL303XX_SYNTH_FP_POLARITY_MASK,
                              ZL303XX_SYNTH_FP_POLARITY_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   /* Update the device parameters structure */
   if (status == ZL303XX_OK)
   {
      zl303xx_Params->pllParams.synth[synthId].fpConfig[fpId].polarity = val;
   }

   return status;
}  /* END zl303xx_SynthFpPolaritySet */


/*

  Function Name:
   zl303xx_SynthFpFineOffsetGet

  Details:
   Reads the Fp Fine Offset attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    synthId        Associated SYNTH Id of the attribute
   [in]    fpId           Associated Fp Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SynthFpFineOffsetGet(zl303xx_ParamsS *zl303xx_Params,
                                       zl303xx_SynthIdE synthId,
                                       zl303xx_FpIdE fpId,
                                       Uint32T *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
   }

   /* Check the synthId parameter */
   if (status == ZL303XX_OK)
   {
      if (synthId != ZL303XX_SYNTH_ID_P0)
      {
         status = ZL303XX_PARAMETER_INVALID;
      }
   }

   /* Check the fpId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_FP_ID(fpId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_SYNTHP0_FP_FINE_OFFSET_REG(fpId),
                              ZL303XX_SYNTH_FP_FINE_OFFSET_MASK,
                              ZL303XX_SYNTH_FP_FINE_OFFSET_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (Uint32T)(attrPar.value);
   }

   /* Update the device parameters structure */
   if (status == ZL303XX_OK)
   {
      /* The coarse offset register is stored as multiples of 125 us */
      Uint32T coarseReg = zl303xx_Params->pllParams.synth[synthId].fpConfig[fpId].offset / 125000;

      /* Use register values to calculate offset in ns */
      zl303xx_Params->pllParams.synth[synthId].fpConfig[fpId].offset = zl303xx_SynthFpOffsetInNs(*val, coarseReg);
   }

   return status;
}  /* END zl303xx_SynthFpFineOffsetGet */

/*

  Function Name:
   zl303xx_SynthFpFineOffsetSet

  Details:
   Writes the Fp Fine Offset attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    synthId        Associated SYNTH Id of the attribute
   [in]    fpId           Associated Fp Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SynthFpFineOffsetSet(zl303xx_ParamsS *zl303xx_Params,
                                       zl303xx_SynthIdE synthId,
                                       zl303xx_FpIdE fpId,
                                       Uint32T val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check params pointer */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
   }

   /* Check the synthId parameter */
   if (status == ZL303XX_OK)
   {
      if (synthId != ZL303XX_SYNTH_ID_P0)
      {
         status = ZL303XX_PARAMETER_INVALID;
      }
   }

   /* Check the fpId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_FP_ID(fpId);
   }

   /* Check that the write value is within range */
   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_SYNTHP0_FP_FINE_OFFSET_REG(fpId),
                              ZL303XX_SYNTH_FP_FINE_OFFSET_MASK,
                              ZL303XX_SYNTH_FP_FINE_OFFSET_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   /* Update the device parameters structure */
   if (status == ZL303XX_OK)
   {
      /* The coarse offset register is stored as multiples of 125 us */
      Uint32T coarseReg = zl303xx_Params->pllParams.synth[synthId].fpConfig[fpId].offset / 125000;

      /* Use register values to calculate offset in ns */
      zl303xx_Params->pllParams.synth[synthId].fpConfig[fpId].offset = zl303xx_SynthFpOffsetInNs(val, coarseReg);
   }

   return status;
}  /* END zl303xx_SynthFpFineOffsetSet */


/*

  Function Name:
   zl303xx_SynthFpCoarseOffsetGet

  Details:
   Reads the Fp Coarse Offset attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    synthId        Associated SYNTH Id of the attribute
   [in]    fpId           Associated Fp Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SynthFpCoarseOffsetGet(zl303xx_ParamsS *zl303xx_Params,
                                         zl303xx_SynthIdE synthId,
                                         zl303xx_FpIdE fpId,
                                         Uint32T *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
   }

   /* Check the synthId parameter */
   if (status == ZL303XX_OK)
   {
      if (synthId != ZL303XX_SYNTH_ID_P0)
      {
         status = ZL303XX_PARAMETER_INVALID;
      }
   }

   /* Check the fpId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_FP_ID(fpId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_SYNTHP0_FP_COARSE_OFFSET_REG(fpId),
                              ZL303XX_SYNTH_FP_COARSE_OFFSET_MASK,
                              ZL303XX_SYNTH_FP_COARSE_OFFSET_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (Uint32T)(attrPar.value);
   }

   /* Update the device parameters structure */
   if (status == ZL303XX_OK)
   {
      /* Keep only the fine offset portion */
      zl303xx_Params->pllParams.synth[synthId].fpConfig[fpId].offset %= 125000;

      /* Add the coarse offset back in */
      zl303xx_Params->pllParams.synth[synthId].fpConfig[fpId].offset += (*val) * 125000;
   }

   return status;
}  /* END zl303xx_SynthFpCoarseOffsetGet */

/*

  Function Name:
   zl303xx_SynthFpCoarseOffsetSet

  Details:
   Writes the Fp Coarse Offset attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    synthId        Associated SYNTH Id of the attribute
   [in]    fpId           Associated Fp Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SynthFpCoarseOffsetSet(zl303xx_ParamsS *zl303xx_Params,
                                         zl303xx_SynthIdE synthId,
                                         zl303xx_FpIdE fpId,
                                         Uint32T val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check params pointer */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
   }

   /* Check the synthId parameter */
   if (status == ZL303XX_OK)
   {
      if (synthId != ZL303XX_SYNTH_ID_P0)
      {
         status = ZL303XX_PARAMETER_INVALID;
      }
   }

   /* Check the fpId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_FP_ID(fpId);
   }

   /* Check that the write value is within range */
   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_SYNTHP0_FP_COARSE_OFFSET_REG(fpId),
                              ZL303XX_SYNTH_FP_COARSE_OFFSET_MASK,
                              ZL303XX_SYNTH_FP_COARSE_OFFSET_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   /* Update the device parameters structure */
   if (status == ZL303XX_OK)
   {
      /* Keep only the fine offset portion */
      zl303xx_Params->pllParams.synth[synthId].fpConfig[fpId].offset %= 125000;

      /* Add the coarse offset back in */
      zl303xx_Params->pllParams.synth[synthId].fpConfig[fpId].offset += val * 125000;
   }

   return status;
}  /* END zl303xx_SynthFpCoarseOffsetSet */
