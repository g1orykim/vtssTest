

/*******************************************************************************
*
*  $Id: zl303xx_SdhLow.c 6058 2011-06-10 14:37:16Z AW $
*
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     Functions for low-level SDH attribute access
*
*******************************************************************************/

/*****************   INCLUDE FILES   ******************************************/

#include "zl303xx_ApiLow.h"
#include "zl303xx_SdhLow.h"
#include "zl303xx_PllFuncs.h"

/*****************   DEFINES   ************************************************/

/*****************   DATA TYPES   *********************************************/

/*****************   DATA STRUCTURES   ****************************************/

/*****************   EXPORTED GLOBAL VARIABLE DECLARATIONS   ******************/

/*

  Function Name:
   zl303xx_SdhClkConfigStructInit

  Details:
   Initializes the members of the zl303xx_SdhClkConfigS data structure

  Parameters:
   [in]   zl303xx_Params Pointer to the device instance parameter structure
   [in]   par            Pointer to the zl303xx_SdhClkConfigS structure to Initialize

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SdhClkConfigStructInit(zl303xx_ParamsS *zl303xx_Params,
                                         zl303xx_SdhClkConfigS *par)
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
      par->freq = (zl303xx_SdhClkFreqE)ZL303XX_INVALID;
      par->offset = (zl303xx_ClkOffsetE)ZL303XX_INVALID;
      par->Id = (zl303xx_ClockIdE)ZL303XX_INVALID;
   }

   return status;
} /* END zl303xx_SdhClkConfigStructInit */

/*

  Function Name:
   zl303xx_SdhClkConfigCheck

  Details:
   Checks the members of the zl303xx_SdhClkConfigS data structure

  Parameters:
   [in]   zl303xx_Params Pointer to the device instance parameter structure
   [in]   par            Pointer to the zl303xx_SdhClkConfigS structure to Check

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SdhClkConfigCheck(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_SdhClkConfigS *par)
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
               ZL303XX_CHECK_SDH_CLK_FREQ(par->freq) |
               ZL303XX_CHECK_CLK_OFFSET(par->offset) |
               ZL303XX_CHECK_CLOCK_ID(par->Id);
   }

   return status;
} /* END zl303xx_SdhClkConfigCheck */

/*

  Function Name:
   zl303xx_SdhClkConfigGet

  Details:
   Gets the members of the zl303xx_SdhClkConfigS data structure

  Parameters:
   [in]   zl303xx_Params Pointer to the device instance parameter structure
   [in]   par            Pointer to the zl303xx_SdhClkConfigS structure to Get

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SdhClkConfigGet(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_SdhClkConfigS *par)
{
   zlStatusE status = ZL303XX_OK;
   Uint32T regValue = 0, ethDiv = 0, freqConv = 0;

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
                            ZL303XX_SDH_OUTPUT_CTRL_REG,
                            &regValue);
   }

   /* Extract */
   if (status == ZL303XX_OK)
   {
      par->enable = (zl303xx_OutEnableE)ZL303XX_EXTRACT(regValue,
                                        ZL303XX_SDH_CLK_ENABLE_MASK,
                                        ZL303XX_SDH_CLK_ENABLE_SHIFT(par->Id));

      par->run = (zl303xx_OutputRunE)ZL303XX_EXTRACT(regValue,
                                      ZL303XX_SDH_CLK_RUN_MASK,
                                      ZL303XX_SDH_CLK_RUN_SHIFT(par->Id));

      ethDiv = (Uint32T)ZL303XX_EXTRACT(regValue,
                                      ZL303XX_SDH_CLK_ETH_DIV_MASK,
                                      ZL303XX_SDH_CLK_ETH_DIV_SHIFT(par->Id));

      freqConv = (Uint32T)ZL303XX_EXTRACT(regValue,
                                        ZL303XX_SDH_FREQ_CONV_MASK,
                                        ZL303XX_SDH_FREQ_CONV_SHIFT);
   }

   /* Read */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_Read(zl303xx_Params, NULL,
                            ZL303XX_SDH_CLK_DIVIDE_REG,
                            &regValue);
   }

   /* Extract */
   if (status == ZL303XX_OK)
   {
      par->freq = (zl303xx_SdhClkFreqE)ZL303XX_EXTRACT(regValue,
                                       ZL303XX_SDH_CLK_FREQ_MASK,
                                       ZL303XX_SDH_CLK_FREQ_SHIFT(par->Id));

      /* Need eth_en and f_sel_n bits (stored in freqConv and ethDiv respectively) to know
         the actual frequency being output. Re-encode these bits into the enum value. */
      par->freq |= (ethDiv << 4) | (freqConv << 5);
   }

   /* Read */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_Read(zl303xx_Params, NULL,
                            ZL303XX_SDH_CLK_OFFSET_REG(par->Id),
                            &regValue);
   }

   /* Extract */
   if (status == ZL303XX_OK)
   {
      par->offset = (zl303xx_ClkOffsetE)ZL303XX_EXTRACT(regValue,
                                        ZL303XX_SDH_CLK_OFFSET_MASK,
                                        ZL303XX_SDH_CLK_OFFSET_SHIFT);
   }

   return status;
} /* END zl303xx_SdhClkConfigGet */

/*

  Function Name:
   zl303xx_SdhClkConfigSet

  Details:
   Sets the members of the zl303xx_SdhClkConfigS data structure

  Parameters:
   [in]   zl303xx_Params Pointer to the device instance parameter structure
   [in]   par            Pointer to the zl303xx_SdhClkConfigS structure to Set

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SdhClkConfigSet(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_SdhClkConfigS *par)
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
      status = zl303xx_SdhClkConfigCheck(zl303xx_Params, par);
   }

   /* Package */
   if (status == ZL303XX_OK)
   {
      regValue = 0;
      mask = 0;

      /* enable */
      ZL303XX_INSERT(regValue, par->enable,
                             ZL303XX_SDH_CLK_ENABLE_MASK,
                             ZL303XX_SDH_CLK_ENABLE_SHIFT(par->Id));

      /* run */
      ZL303XX_INSERT(regValue, par->run,
                             ZL303XX_SDH_CLK_RUN_MASK,
                             ZL303XX_SDH_CLK_RUN_SHIFT(par->Id));

      /* ethDiv (f_sel_n) is encoded in bit4 of the freq enum value */
      ZL303XX_INSERT(regValue, ZL303XX_EXTRACT(par->freq, 1, 4),
                             ZL303XX_SDH_CLK_ETH_DIV_MASK,
                             ZL303XX_SDH_CLK_ETH_DIV_SHIFT(par->Id));

      /* freqConv (eth_en) is encoded in bit5 of the freq enum value */
      ZL303XX_INSERT(regValue, ZL303XX_EXTRACT(par->freq, 1, 5),
                             ZL303XX_SDH_FREQ_CONV_MASK,
                             ZL303XX_SDH_FREQ_CONV_SHIFT);

      mask |= (ZL303XX_SDH_CLK_ENABLE_MASK << ZL303XX_SDH_CLK_ENABLE_SHIFT(par->Id)) |
              (ZL303XX_SDH_CLK_RUN_MASK << ZL303XX_SDH_CLK_RUN_SHIFT(par->Id)) |
              (ZL303XX_SDH_CLK_ETH_DIV_MASK << ZL303XX_SDH_CLK_ETH_DIV_SHIFT(par->Id)) |
              (ZL303XX_SDH_FREQ_CONV_MASK << ZL303XX_SDH_FREQ_CONV_SHIFT);

      /* Write the Data for this Register */
      status = zl303xx_ReadModWrite(zl303xx_Params, NULL,
                                    ZL303XX_SDH_OUTPUT_CTRL_REG,
                                    regValue, mask, NULL);
   }

   /* Package */
   if (status == ZL303XX_OK)
   {
      regValue = 0;
      mask = 0;

      /* freq */
      ZL303XX_INSERT(regValue, par->freq,
                             ZL303XX_SDH_CLK_FREQ_MASK,
                             ZL303XX_SDH_CLK_FREQ_SHIFT(par->Id));

      mask |= (ZL303XX_SDH_CLK_FREQ_MASK << ZL303XX_SDH_CLK_FREQ_SHIFT(par->Id));

      /* Write the Data for this Register */
      status = zl303xx_ReadModWrite(zl303xx_Params, NULL,
                                    ZL303XX_SDH_CLK_DIVIDE_REG,
                                    regValue, mask, NULL);
   }

   /* Package */
   if (status == ZL303XX_OK)
   {
      regValue = 0;
      mask = 0;

      /* offset */
      ZL303XX_INSERT(regValue, par->offset,
                             ZL303XX_SDH_CLK_OFFSET_MASK,
                             ZL303XX_SDH_CLK_OFFSET_SHIFT);

      mask |= (ZL303XX_SDH_CLK_OFFSET_MASK << ZL303XX_SDH_CLK_OFFSET_SHIFT);

      /* Write the Data for this Register */
      status = zl303xx_ReadModWrite(zl303xx_Params, NULL,
                                    ZL303XX_SDH_CLK_OFFSET_REG(par->Id),
                                    regValue, mask, NULL);
   }

   return status;
} /* END zl303xx_SdhClkConfigSet */

/*

  Function Name:
   zl303xx_SdhFpConfigStructInit

  Details:
   Initializes the members of the zl303xx_SdhFpConfigS data structure

  Parameters:
   [in]   zl303xx_Params Pointer to the device instance parameter structure
   [in]   par            Pointer to the zl303xx_SdhFpConfigS structure to Initialize

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SdhFpConfigStructInit(zl303xx_ParamsS *zl303xx_Params,
                                        zl303xx_SdhFpConfigS *par)
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
      par->type = (zl303xx_FpTypeSdhE)ZL303XX_INVALID;
      par->polarity = (zl303xx_FpPolarityE)ZL303XX_INVALID;
      par->offset = (Uint32T) ZL303XX_INVALID;
      par->Id = (zl303xx_FpIdE)ZL303XX_INVALID;
   }

   return status;
} /* END zl303xx_SdhFpConfigStructInit */

/*

  Function Name:
   zl303xx_SdhFpConfigCheck

  Details:
   Checks the members of the zl303xx_SdhFpConfigS data structure

  Parameters:
   [in]   zl303xx_Params Pointer to the device instance parameter structure
   [in]   par            Pointer to the zl303xx_SdhFpConfigS structure to Check

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SdhFpConfigCheck(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_SdhFpConfigS *par)
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
               ZL303XX_CHECK_FP_TYPE_SDH(par->type) |
               ZL303XX_CHECK_FP_POLARITY(par->polarity) |
               zl303xx_CheckFpOffset(par->offset) |
               ZL303XX_CHECK_FP_ID(par->Id);
   }

   return status;
} /* END zl303xx_SdhFpConfigCheck */

/*

  Function Name:
   zl303xx_SdhFpConfigGet

  Details:
   Gets the members of the zl303xx_SdhFpConfigS data structure

  Parameters:
   [in]   zl303xx_Params Pointer to the device instance parameter structure
   [in]   par            Pointer to the zl303xx_SdhFpConfigS structure to Get

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SdhFpConfigGet(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_SdhFpConfigS *par)
{
   zlStatusE status = ZL303XX_OK;
   Uint32T regValue = 0, fine, coarse;

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

   /* Read */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_Read(zl303xx_Params, NULL,
                            ZL303XX_SDH_OUTPUT_CTRL_REG,
                            &regValue);
   }

   /* Extract */
   if (status == ZL303XX_OK)
   {
      par->enable = (zl303xx_OutEnableE)ZL303XX_EXTRACT(regValue,
                                        ZL303XX_SDH_FP_ENABLE_MASK,
                                        ZL303XX_SDH_FP_ENABLE_SHIFT(par->Id));

      par->run = (zl303xx_OutputRunE)ZL303XX_EXTRACT(regValue,
                                      ZL303XX_SDH_FP_RUN_MASK,
                                      ZL303XX_SDH_FP_RUN_SHIFT(par->Id));
   }

   /* Read */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_Read(zl303xx_Params, NULL,
                            ZL303XX_SDH_FP_FREQ_REG(par->Id),
                            &regValue);
   }

   /* Extract */
   if (status == ZL303XX_OK)
   {
      par->freq = (zl303xx_FpFreqE)ZL303XX_EXTRACT(regValue,
                                   ZL303XX_SDH_FP_FREQ_MASK,
                                   ZL303XX_SDH_FP_FREQ_SHIFT);
   }

   /* Read */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_Read(zl303xx_Params, NULL,
                            ZL303XX_SDH_FP_TYPE_REG(par->Id),
                            &regValue);
   }

   /* Extract */
   if (status == ZL303XX_OK)
   {
      par->style = (zl303xx_FpStyleE)ZL303XX_EXTRACT(regValue,
                                     ZL303XX_SDH_FP_STYLE_MASK,
                                     ZL303XX_SDH_FP_STYLE_SHIFT);

      par->syncEdge = (zl303xx_FpSyncEdgeE)ZL303XX_EXTRACT(regValue,
                                            ZL303XX_SDH_FP_SYNC_EDGE_MASK,
                                            ZL303XX_SDH_FP_SYNC_EDGE_SHIFT);

      par->type = (zl303xx_FpTypeSdhE)ZL303XX_EXTRACT(regValue,
                                       ZL303XX_SDH_FP_TYPE_MASK,
                                       ZL303XX_SDH_FP_TYPE_SHIFT);

      par->polarity = (zl303xx_FpPolarityE)ZL303XX_EXTRACT(regValue,
                                            ZL303XX_SDH_FP_POLARITY_MASK,
                                            ZL303XX_SDH_FP_POLARITY_SHIFT);
   }


   /* Extract */
   if (status == ZL303XX_OK)
   {
      Uint32T sdhClkDiv3;

      /* read the SDH Clock frequency register */
      status = zl303xx_Read(zl303xx_Params, NULL,
                          ZL303XX_SDH_CLK_DIVIDE_REG,
                          &regValue);

      if (status == ZL303XX_OK)
      {
          /* extract the sdhClkDiv3 */
          sdhClkDiv3 = (0x01) & (regValue >> (3 + 4 * par->Id));

          /* read the fine offset register */
          status = zl303xx_Read(zl303xx_Params, NULL,
                              ZL303XX_SDH_FP_FINE_OFFSET_REG(par->Id),
                              &regValue);
      }

      if (status == ZL303XX_OK)
      {
          fine = (Uint32T)ZL303XX_EXTRACT(regValue,
                                        ZL303XX_SDH_FP_FINE_OFFSET_MASK,
                                        ZL303XX_SDH_FP_FINE_OFFSET_SHIFT);

          /* read the coarse offset register */
          status = zl303xx_Read(zl303xx_Params, NULL,
                              ZL303XX_SDH_FP_COARSE_OFFSET_REG(par->Id),
                              &regValue);
      }

      if (status == ZL303XX_OK)
      {
          coarse = (Uint32T)ZL303XX_EXTRACT(regValue,
                                        ZL303XX_SDH_FP_COARSE_OFFSET_MASK,
                                        ZL303XX_SDH_FP_COARSE_OFFSET_SHIFT);

          par->offset = zl303xx_SdhFpOffsetInNs(fine, coarse, sdhClkDiv3);
      }
   }

   return status;
} /* END zl303xx_SdhFpConfigGet */

/*

  Function Name:
   zl303xx_SdhFpConfigSet

  Details:
   Sets the members of the zl303xx_SdhFpConfigS data structure

  Parameters:
   [in]   zl303xx_Params Pointer to the device instance parameter structure
   [in]   par            Pointer to the zl303xx_SdhFpConfigS structure to Set

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SdhFpConfigSet(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_SdhFpConfigS *par)
{
   zlStatusE status = ZL303XX_OK;
   Uint32T regValue, mask, fine, coarse, sdhClkDiv3;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, par);
   }

   /* Check the par values to be written */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_SdhFpConfigCheck(zl303xx_Params, par);
   }

   /* Package */
   if (status == ZL303XX_OK)
   {
      regValue = 0;
      mask = 0;

      /* enable */
      ZL303XX_INSERT(regValue, par->enable,
                           ZL303XX_SDH_FP_ENABLE_MASK,
                           ZL303XX_SDH_FP_ENABLE_SHIFT(par->Id));

      /* run */
      ZL303XX_INSERT(regValue, par->run,
                           ZL303XX_SDH_FP_RUN_MASK,
                           ZL303XX_SDH_FP_RUN_SHIFT(par->Id));

      mask |= (ZL303XX_SDH_FP_ENABLE_MASK << ZL303XX_SDH_FP_ENABLE_SHIFT(par->Id)) |
              (ZL303XX_SDH_FP_RUN_MASK << ZL303XX_SDH_FP_RUN_SHIFT(par->Id));

      /* Write the Data for this Register */
      status = zl303xx_ReadModWrite(zl303xx_Params, NULL,
                                    ZL303XX_SDH_OUTPUT_CTRL_REG,
                                    regValue, mask, NULL);
   }

   /* Package */
   if (status == ZL303XX_OK)
   {
      regValue = 0;
      mask = 0;

      /* freq */
      ZL303XX_INSERT(regValue, par->freq,
                           ZL303XX_SDH_FP_FREQ_MASK,
                           ZL303XX_SDH_FP_FREQ_SHIFT);

      mask |= (ZL303XX_SDH_FP_FREQ_MASK << ZL303XX_SDH_FP_FREQ_SHIFT);

      /* Write the Data for this Register */
      status = zl303xx_ReadModWrite(zl303xx_Params, NULL,
                                    ZL303XX_SDH_FP_FREQ_REG(par->Id),
                                    regValue, mask, NULL);
   }

   /* Package */
   if (status == ZL303XX_OK)
   {
      regValue = 0;
      mask = 0;

      /* style */
      ZL303XX_INSERT(regValue, par->style,
                             ZL303XX_SDH_FP_STYLE_MASK,
                             ZL303XX_SDH_FP_STYLE_SHIFT);

      /* syncEdge */
      ZL303XX_INSERT(regValue, par->syncEdge,
                             ZL303XX_SDH_FP_SYNC_EDGE_MASK,
                             ZL303XX_SDH_FP_SYNC_EDGE_SHIFT);

      /* type */
      ZL303XX_INSERT(regValue, par->type,
                             ZL303XX_SDH_FP_TYPE_MASK,
                             ZL303XX_SDH_FP_TYPE_SHIFT);

      /* polarity */
      ZL303XX_INSERT(regValue, par->polarity,
                             ZL303XX_SDH_FP_POLARITY_MASK,
                             ZL303XX_SDH_FP_POLARITY_SHIFT);

      mask |= (ZL303XX_SDH_FP_STYLE_MASK << ZL303XX_SDH_FP_STYLE_SHIFT) |
              (ZL303XX_SDH_FP_SYNC_EDGE_MASK << ZL303XX_SDH_FP_SYNC_EDGE_SHIFT) |
              (ZL303XX_SDH_FP_TYPE_MASK << ZL303XX_SDH_FP_TYPE_SHIFT) |
              (ZL303XX_SDH_FP_POLARITY_MASK << ZL303XX_SDH_FP_POLARITY_SHIFT);

      /* Write the Data for this Register */
      status = zl303xx_ReadModWrite(zl303xx_Params, NULL,
                                    ZL303XX_SDH_FP_TYPE_REG(par->Id),
                                    regValue, mask, NULL);
   }

   /* Package */
   if (status == ZL303XX_OK)
   {
      regValue = 0;
      mask = 0;

      /* offset */
      /* read the SDH Clock frequency register */
      status = zl303xx_Read(zl303xx_Params, NULL,
                          ZL303XX_SDH_CLK_DIVIDE_REG,
                          &regValue);

      if (status == ZL303XX_OK)
      {
          /* extract the sdhClkDiv3 */
          sdhClkDiv3 = (0x01) & (regValue >> (3 + 4 * par->Id));

          status = zl303xx_ClosestSdhFpOffsetReg(par->offset, &fine, &coarse, sdhClkDiv3);

          if (status == ZL303XX_OK)
          {
              regValue = 0;

              ZL303XX_INSERT(regValue, fine,
                                     ZL303XX_SDH_FP_FINE_OFFSET_MASK,
                                     ZL303XX_SDH_FP_FINE_OFFSET_SHIFT);

              mask |= (ZL303XX_SDH_FP_FINE_OFFSET_MASK << ZL303XX_SDH_FP_FINE_OFFSET_SHIFT);

              status = zl303xx_ReadModWrite(zl303xx_Params, NULL,
                                            ZL303XX_SDH_FP_FINE_OFFSET_REG(par->Id),
                                            regValue, mask, NULL);
          }

          if (status == ZL303XX_OK)
          {
              regValue = 0;
              mask = 0;

              ZL303XX_INSERT(regValue, coarse,
                                     ZL303XX_SDH_FP_COARSE_OFFSET_MASK,
                                     ZL303XX_SDH_FP_COARSE_OFFSET_SHIFT);

              mask |= (ZL303XX_SDH_FP_COARSE_OFFSET_MASK << ZL303XX_SDH_FP_COARSE_OFFSET_SHIFT);

              status = zl303xx_ReadModWrite(zl303xx_Params, NULL,
                                            ZL303XX_SDH_FP_COARSE_OFFSET_REG(par->Id),
                                            regValue, mask, NULL);
          }
      }
   }

   return status;
} /* END zl303xx_SdhFpConfigSet */

/*

  Function Name:
   zl303xx_SdhConfigStructInit

  Details:
   Initializes the members of the zl303xx_SdhConfigS data structure

  Parameters:
   [in]   zl303xx_Params Pointer to the device instance parameter structure
   [in]   par            Pointer to the zl303xx_SdhConfigS structure to Initialize

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SdhConfigStructInit(zl303xx_ParamsS *zl303xx_Params,
                                      zl303xx_SdhConfigS *par)
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
      par->centerFreq = (zl303xx_SdhCtrFreqE)ZL303XX_INVALID;
      par->fineDelay = ZL303XX_INVALID;
   }

   return status;
} /* END zl303xx_SdhConfigStructInit */

/*

  Function Name:
   zl303xx_SdhConfigCheck

  Details:
   Checks the members of the zl303xx_SdhConfigS data structure

  Parameters:
   [in]   zl303xx_Params Pointer to the device instance parameter structure
   [in]   par            Pointer to the zl303xx_SdhConfigS structure to Check

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SdhConfigCheck(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_SdhConfigS *par)
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
               ZL303XX_CHECK_SDH_CTR_FREQ(par->centerFreq) |
               zl303xx_CheckOutputFineOffset(par->fineDelay);
   }

   return status;
} /* END zl303xx_SdhConfigCheck */

/*

  Function Name:
   zl303xx_SdhConfigGet

  Details:
   Gets the members of the zl303xx_SdhConfigS data structure

  Parameters:
   [in]   zl303xx_Params Pointer to the device instance parameter structure
   [in]   par            Pointer to the zl303xx_SdhConfigS structure to Get

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SdhConfigGet(zl303xx_ParamsS *zl303xx_Params,
                               zl303xx_SdhConfigS *par)
{
   zlStatusE status = ZL303XX_OK;
   Uint32T regValue = 0;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, par);
   }


   /* Read */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_Read(zl303xx_Params, NULL,
                            ZL303XX_SDH_OUTPUT_CTRL_REG,
                            &regValue);
   }

   /* Extract */
   if (status == ZL303XX_OK)
   {
      par->source = (zl303xx_DpllIdE)ZL303XX_EXTRACT(regValue,
                                     ZL303XX_SDH_IN_SOURCE_MASK,
                                     ZL303XX_SDH_IN_SOURCE_SHIFT);

      par->enable = (zl303xx_BooleanE)ZL303XX_EXTRACT(regValue,
                                       ZL303XX_SDH_ENABLE_MASK,
                                       ZL303XX_SDH_ENABLE_SHIFT);

      par->centerFreq = (zl303xx_SdhCtrFreqE)ZL303XX_EXTRACT(regValue,
                                              ZL303XX_SDH_FREQ_CONV_MASK,
                                              ZL303XX_SDH_FREQ_CONV_SHIFT);
   }

   /* Read */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_Read(zl303xx_Params, NULL,
                            ZL303XX_SDH_FINE_DELAY_REG,
                            &regValue);
   }

   /* Extract */
   if (status == ZL303XX_OK)
   {
      par->fineDelay = zl303xx_OutputFineOffsetInPs(regValue);
   }

   return status;
} /* END zl303xx_SdhConfigGet */

/*

  Function Name:
   zl303xx_SdhConfigSet

  Details:
   Sets the members of the zl303xx_SdhConfigS data structure

  Parameters:
   [in]   zl303xx_Params Pointer to the device instance parameter structure
   [in]   par            Pointer to the zl303xx_SdhConfigS structure to Set

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SdhConfigSet(zl303xx_ParamsS *zl303xx_Params,
                               zl303xx_SdhConfigS *par)
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
      status = zl303xx_SdhConfigCheck(zl303xx_Params, par);
   }

   /* Package */
   if (status == ZL303XX_OK)
   {
      regValue = 0;
      mask = 0;

      /* source */
      ZL303XX_INSERT(regValue, par->source,
                             ZL303XX_SDH_IN_SOURCE_MASK,
                             ZL303XX_SDH_IN_SOURCE_SHIFT);

      /* enable */
      ZL303XX_INSERT(regValue, par->enable,
                             ZL303XX_SDH_ENABLE_MASK,
                             ZL303XX_SDH_ENABLE_SHIFT);

      /* centerFreq */
      ZL303XX_INSERT(regValue, par->centerFreq,
                             ZL303XX_SDH_FREQ_CONV_MASK,
                             ZL303XX_SDH_FREQ_CONV_SHIFT);

      mask |= (ZL303XX_SDH_IN_SOURCE_MASK << ZL303XX_SDH_IN_SOURCE_SHIFT) |
              (ZL303XX_SDH_ENABLE_MASK << ZL303XX_SDH_ENABLE_SHIFT) |
              (ZL303XX_SDH_FREQ_CONV_MASK << ZL303XX_SDH_FREQ_CONV_SHIFT);

      /* Write the Data for this Register */
      status = zl303xx_ReadModWrite(zl303xx_Params, NULL,
                                    ZL303XX_SDH_OUTPUT_CTRL_REG,
                                    regValue, mask, NULL);
   }

   /* Package */
   if (status == ZL303XX_OK)
   {
      regValue = 0;
      mask = 0;

      /* fineDelay */
      status = zl303xx_ClosestOutputFineOffsetReg(par->fineDelay, &regValue);

      if (status == ZL303XX_OK)
      {
          ZL303XX_INSERT(regValue, par->fineDelay,
                                 ZL303XX_SDH_FINE_DELAY_MASK,
                                 ZL303XX_SDH_FINE_DELAY_SHIFT);

          mask |= (ZL303XX_SDH_FINE_DELAY_MASK << ZL303XX_SDH_FINE_DELAY_SHIFT);

          /* Write the Data for this Register */
          status = zl303xx_ReadModWrite(zl303xx_Params, NULL,
                                        ZL303XX_SDH_FINE_DELAY_REG,
                                        regValue, mask, NULL);
      }
   }

   return status;
} /* END zl303xx_SdhConfigSet */

/*

  Function Name:
   zl303xx_SdhClkEnableGet

  Details:
   Reads the Clk Enable attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    clockId        Associated Clock Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SdhClkEnableGet(zl303xx_ParamsS *zl303xx_Params,
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

   /* Check the clockId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_CLOCK_ID(clockId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_SDH_OUTPUT_CTRL_REG,
                              ZL303XX_SDH_CLK_ENABLE_MASK,
                              ZL303XX_SDH_CLK_ENABLE_SHIFT(clockId));
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_OutEnableE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_SdhClkEnableGet */

/*

  Function Name:
   zl303xx_SdhClkEnableSet

  Details:
   Writes the Clk Enable attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    clockId        Associated Clock Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SdhClkEnableSet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_SDH_OUTPUT_CTRL_REG,
                              ZL303XX_SDH_CLK_ENABLE_MASK,
                              ZL303XX_SDH_CLK_ENABLE_SHIFT(clockId));
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_SdhClkEnableSet */


/*

  Function Name:
   zl303xx_SdhFpEnableGet

  Details:
   Reads the Fp Enable attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    fpId           Associated Fp Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SdhFpEnableGet(zl303xx_ParamsS *zl303xx_Params,
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

   /* Check the fpId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_FP_ID(fpId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_SDH_OUTPUT_CTRL_REG,
                              ZL303XX_SDH_FP_ENABLE_MASK,
                              ZL303XX_SDH_FP_ENABLE_SHIFT(fpId));
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_OutEnableE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_SdhFpEnableGet */

/*

  Function Name:
   zl303xx_SdhFpEnableSet

  Details:
   Writes the Fp Enable attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    fpId           Associated Fp Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SdhFpEnableSet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_SDH_OUTPUT_CTRL_REG,
                              ZL303XX_SDH_FP_ENABLE_MASK,
                              ZL303XX_SDH_FP_ENABLE_SHIFT(fpId));
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_SdhFpEnableSet */


/*

  Function Name:
   zl303xx_SdhInSourceGet

  Details:
   Reads the Dpll Source attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SdhInSourceGet(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_DpllIdE *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_SDH_OUTPUT_CTRL_REG,
                              ZL303XX_SDH_IN_SOURCE_MASK,
                              ZL303XX_SDH_IN_SOURCE_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_DpllIdE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_SdhInSourceGet */

/*

  Function Name:
   zl303xx_SdhInSourceSet

  Details:
   Writes the Dpll Source attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SdhInSourceSet(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_DpllIdE val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check params pointer */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
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
                              ZL303XX_SDH_OUTPUT_CTRL_REG,
                              ZL303XX_SDH_IN_SOURCE_MASK,
                              ZL303XX_SDH_IN_SOURCE_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_SdhInSourceSet */


/*

  Function Name:
   zl303xx_SdhEnableGet

  Details:
   Reads the PLL Enable attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SdhEnableGet(zl303xx_ParamsS *zl303xx_Params,
                               zl303xx_BooleanE *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_SDH_OUTPUT_CTRL_REG,
                              ZL303XX_SDH_ENABLE_MASK,
                              ZL303XX_SDH_ENABLE_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_BooleanE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_SdhEnableGet */

/*

  Function Name:
   zl303xx_SdhEnableSet

  Details:
   Writes the PLL Enable attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SdhEnableSet(zl303xx_ParamsS *zl303xx_Params,
                               zl303xx_BooleanE val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check params pointer */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
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
                              ZL303XX_SDH_OUTPUT_CTRL_REG,
                              ZL303XX_SDH_ENABLE_MASK,
                              ZL303XX_SDH_ENABLE_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_SdhEnableSet */


/*

  Function Name:
   zl303xx_SdhClkRunGet

  Details:
   Reads the Clk Run attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    clockId        Associated Clock Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SdhClkRunGet(zl303xx_ParamsS *zl303xx_Params,
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

   /* Check the clockId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_CLOCK_ID(clockId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_SDH_OUTPUT_CTRL_REG,
                              ZL303XX_SDH_CLK_RUN_MASK,
                              ZL303XX_SDH_CLK_RUN_SHIFT(clockId));
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_OutputRunE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_SdhClkRunGet */

/*

  Function Name:
   zl303xx_SdhClkRunSet

  Details:
   Writes the Clk Run attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    clockId        Associated Clock Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SdhClkRunSet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_SDH_OUTPUT_CTRL_REG,
                              ZL303XX_SDH_CLK_RUN_MASK,
                              ZL303XX_SDH_CLK_RUN_SHIFT(clockId));
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_SdhClkRunSet */


/*

  Function Name:
   zl303xx_SdhFpRunGet

  Details:
   Reads the Fp Run attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    fpId           Associated Fp Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SdhFpRunGet(zl303xx_ParamsS *zl303xx_Params,
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

   /* Check the fpId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_FP_ID(fpId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_SDH_OUTPUT_CTRL_REG,
                              ZL303XX_SDH_FP_RUN_MASK,
                              ZL303XX_SDH_FP_RUN_SHIFT(fpId));
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_OutputRunE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_SdhFpRunGet */

/*

  Function Name:
   zl303xx_SdhFpRunSet

  Details:
   Writes the Fp Run attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    fpId           Associated Fp Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SdhFpRunSet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_SDH_OUTPUT_CTRL_REG,
                              ZL303XX_SDH_FP_RUN_MASK,
                              ZL303XX_SDH_FP_RUN_SHIFT(fpId));
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_SdhFpRunSet */


/*

  Function Name:
   zl303xx_SdhClkEthDivGet

  Details:
   Reads the Clk Mode attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    clockId        Associated Clock Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SdhClkEthDivGet(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_ClockIdE clockId,
                                  zl303xx_EthDivE *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
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
                              ZL303XX_SDH_OUTPUT_CTRL_REG,
                              ZL303XX_SDH_CLK_ETH_DIV_MASK,
                              ZL303XX_SDH_CLK_ETH_DIV_SHIFT(clockId));
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_EthDivE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_SdhClkEthDivGet */

/*

  Function Name:
   zl303xx_SdhClkEthDivSet

  Details:
   Writes the Clk Mode attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    clockId        Associated Clock Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SdhClkEthDivSet(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_ClockIdE clockId,
                                  zl303xx_EthDivE val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check params pointer */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
   }

   /* Check the clockId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_CLOCK_ID(clockId);
   }

   /* Check that the write value is within range */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_ETH_DIV(val);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_SDH_OUTPUT_CTRL_REG,
                              ZL303XX_SDH_CLK_ETH_DIV_MASK,
                              ZL303XX_SDH_CLK_ETH_DIV_SHIFT(clockId));
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_SdhClkEthDivSet */


/*

  Function Name:
   zl303xx_SdhFreqConvGet

  Details:
   Reads the Center Freq attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SdhFreqConvGet(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_SdhCtrFreqE *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_SDH_OUTPUT_CTRL_REG,
                              ZL303XX_SDH_FREQ_CONV_MASK,
                              ZL303XX_SDH_FREQ_CONV_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_SdhCtrFreqE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_SdhFreqConvGet */

/*

  Function Name:
   zl303xx_SdhFreqConvSet

  Details:
   Writes the Center Freq attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SdhFreqConvSet(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_SdhCtrFreqE val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check params pointer */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
   }

   /* Check that the write value is within range */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_SDH_CTR_FREQ(val);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_SDH_OUTPUT_CTRL_REG,
                              ZL303XX_SDH_FREQ_CONV_MASK,
                              ZL303XX_SDH_FREQ_CONV_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_SdhFreqConvSet */


/*

  Function Name:
   zl303xx_SdhClkFreqGet

  Details:
   Reads the Clk Freq attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    clockId        Associated Clock Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SdhClkFreqGet(zl303xx_ParamsS *zl303xx_Params,
                                zl303xx_ClockIdE clockId,
                                zl303xx_SdhClkFreqE *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;
   zl303xx_EthDivE ethDiv;
   zl303xx_SdhCtrFreqE freqConv;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
   }

   /* Check the clockId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_CLOCK_ID(clockId);
   }

   /* Frequency output is dependent on f_sel_n and eth_en bits */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_SdhClkEthDivGet(zl303xx_Params,
                                     clockId,
                                     &ethDiv);
   }

   if (status == ZL303XX_OK)
   {
      status = zl303xx_SdhFreqConvGet(zl303xx_Params,
                                    &freqConv);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_SDH_CLK_DIVIDE_REG,
                              ZL303XX_SDH_CLK_FREQ_MASK,
                              ZL303XX_SDH_CLK_FREQ_SHIFT(clockId));
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_SdhClkFreqE)(attrPar.value);

      /* Need eth_en and f_sel_n bits (stored in freqConv and ethDiv respectively) to know
         the actual frequency being output. Re-encode these bits into the enum value. */
      *val |= (ethDiv << 4) | (freqConv << 5);
   }

   return status;
}  /* END zl303xx_SdhClkFreqGet */

/*

  Function Name:
   zl303xx_SdhClkFreqSet

  Details:
   Writes the Clk Freq attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    clockId        Associated Clock Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SdhClkFreqSet(zl303xx_ParamsS *zl303xx_Params,
                                zl303xx_ClockIdE clockId,
                                zl303xx_SdhClkFreqE val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check params pointer */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
   }

   /* Check the clockId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_CLOCK_ID(clockId);
   }

   /* Check that the write value is within range */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_SDH_CLK_FREQ(val);
   }

   /* f_sel_n and eth_en bits are encoded in the frequency enum value */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_SdhClkEthDivSet(zl303xx_Params,
                                     clockId,
                                     ZL303XX_EXTRACT(val, 1, 4));
   }

   if (status == ZL303XX_OK)
   {
      status = zl303xx_SdhFreqConvSet(zl303xx_Params,
                                    ZL303XX_EXTRACT(val, 1, 5));
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_SDH_CLK_DIVIDE_REG,
                              ZL303XX_SDH_CLK_FREQ_MASK,
                              ZL303XX_SDH_CLK_FREQ_SHIFT(clockId));
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_SdhClkFreqSet */


/*

  Function Name:
   zl303xx_SdhClkOffsetGet

  Details:
   Reads the Clk Offset attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    clockId        Associated Clock Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SdhClkOffsetGet(zl303xx_ParamsS *zl303xx_Params,
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

   /* Check the clockId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_CLOCK_ID(clockId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_SDH_CLK_OFFSET_REG(clockId),
                              ZL303XX_SDH_CLK_OFFSET_MASK,
                              ZL303XX_SDH_CLK_OFFSET_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_ClkOffsetE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_SdhClkOffsetGet */

/*

  Function Name:
   zl303xx_SdhClkOffsetSet

  Details:
   Writes the Clk Offset attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    clockId        Associated Clock Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SdhClkOffsetSet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_SDH_CLK_OFFSET_REG(clockId),
                              ZL303XX_SDH_CLK_OFFSET_MASK,
                              ZL303XX_SDH_CLK_OFFSET_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_SdhClkOffsetSet */


/*

  Function Name:
   zl303xx_SdhFineDelayGet

  Details:
   Reads the Fine Tune Delay attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SdhFineDelayGet(zl303xx_ParamsS *zl303xx_Params,
                                  Uint32T *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_SDH_FINE_DELAY_REG,
                              ZL303XX_SDH_FINE_DELAY_MASK,
                              ZL303XX_SDH_FINE_DELAY_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (Uint32T)(attrPar.value);
   }

   return status;
}  /* END zl303xx_SdhFineDelayGet */

/*

  Function Name:
   zl303xx_SdhFineDelaySet

  Details:
   Writes the Fine Tune Delay attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SdhFineDelaySet(zl303xx_ParamsS *zl303xx_Params, Uint32T val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check params pointer */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
   }

   /* Check that the write value is within range */
   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_SDH_FINE_DELAY_REG,
                              ZL303XX_SDH_FINE_DELAY_MASK,
                              ZL303XX_SDH_FINE_DELAY_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_SdhFineDelaySet */


/*

  Function Name:
   zl303xx_SdhFpFreqGet

  Details:
   Reads the Fp Freq attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    fpId           Associated Fp Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SdhFpFreqGet(zl303xx_ParamsS *zl303xx_Params,
                               zl303xx_FpIdE fpId, zl303xx_FpFreqE *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
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
                              ZL303XX_SDH_FP_FREQ_REG(fpId),
                              ZL303XX_SDH_FP_FREQ_MASK,
                              ZL303XX_SDH_FP_FREQ_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_FpFreqE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_SdhFpFreqGet */

/*

  Function Name:
   zl303xx_SdhFpFreqSet

  Details:
   Writes the Fp Freq attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    fpId           Associated Fp Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SdhFpFreqSet(zl303xx_ParamsS *zl303xx_Params,
                               zl303xx_FpIdE fpId, zl303xx_FpFreqE val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check params pointer */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
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
                              ZL303XX_SDH_FP_FREQ_REG(fpId),
                              ZL303XX_SDH_FP_FREQ_MASK,
                              ZL303XX_SDH_FP_FREQ_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_SdhFpFreqSet */


/*

  Function Name:
   zl303xx_SdhFpStyleGet

  Details:
   Reads the Fp Style attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    fpId           Associated Fp Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SdhFpStyleGet(zl303xx_ParamsS *zl303xx_Params,
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

   /* Check the fpId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_FP_ID(fpId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_SDH_FP_TYPE_REG(fpId),
                              ZL303XX_SDH_FP_STYLE_MASK,
                              ZL303XX_SDH_FP_STYLE_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_FpStyleE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_SdhFpStyleGet */

/*

  Function Name:
   zl303xx_SdhFpStyleSet

  Details:
   Writes the Fp Style attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    fpId           Associated Fp Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SdhFpStyleSet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_SDH_FP_TYPE_REG(fpId),
                              ZL303XX_SDH_FP_STYLE_MASK,
                              ZL303XX_SDH_FP_STYLE_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_SdhFpStyleSet */


/*

  Function Name:
   zl303xx_SdhFpSyncEdgeGet

  Details:
   Reads the Fp Sync Edge attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    fpId           Associated Fp Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SdhFpSyncEdgeGet(zl303xx_ParamsS *zl303xx_Params,
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

   /* Check the fpId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_FP_ID(fpId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_SDH_FP_TYPE_REG(fpId),
                              ZL303XX_SDH_FP_SYNC_EDGE_MASK,
                              ZL303XX_SDH_FP_SYNC_EDGE_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_FpSyncEdgeE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_SdhFpSyncEdgeGet */

/*

  Function Name:
   zl303xx_SdhFpSyncEdgeSet

  Details:
   Writes the Fp Sync Edge attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    fpId           Associated Fp Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SdhFpSyncEdgeSet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_SDH_FP_TYPE_REG(fpId),
                              ZL303XX_SDH_FP_SYNC_EDGE_MASK,
                              ZL303XX_SDH_FP_SYNC_EDGE_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_SdhFpSyncEdgeSet */


/*

  Function Name:
   zl303xx_SdhFpTypeGet

  Details:
   Reads the Fp Type attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    fpId           Associated Fp Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SdhFpTypeGet(zl303xx_ParamsS *zl303xx_Params,
                               zl303xx_FpIdE fpId,
                               zl303xx_FpTypeSdhE *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
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
                              ZL303XX_SDH_FP_TYPE_REG(fpId),
                              ZL303XX_SDH_FP_TYPE_MASK,
                              ZL303XX_SDH_FP_TYPE_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_FpTypeSdhE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_SdhFpTypeGet */

/*

  Function Name:
   zl303xx_SdhFpTypeSet

  Details:
   Writes the Fp Type attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    fpId           Associated Fp Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SdhFpTypeSet(zl303xx_ParamsS *zl303xx_Params,
                               zl303xx_FpIdE fpId,
                               zl303xx_FpTypeSdhE val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check params pointer */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
   }

   /* Check the fpId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_FP_ID(fpId);
   }

   /* Check that the write value is within range */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_FP_TYPE_SDH(val);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_SDH_FP_TYPE_REG(fpId),
                              ZL303XX_SDH_FP_TYPE_MASK,
                              ZL303XX_SDH_FP_TYPE_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_SdhFpTypeSet */


/*

  Function Name:
   zl303xx_SdhFpPolarityGet

  Details:
   Reads the Fp Polarity attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    fpId           Associated Fp Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SdhFpPolarityGet(zl303xx_ParamsS *zl303xx_Params,
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

   /* Check the fpId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_FP_ID(fpId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_SDH_FP_TYPE_REG(fpId),
                              ZL303XX_SDH_FP_POLARITY_MASK,
                              ZL303XX_SDH_FP_POLARITY_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_FpPolarityE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_SdhFpPolarityGet */

/*

  Function Name:
   zl303xx_SdhFpPolaritySet

  Details:
   Writes the Fp Polarity attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    fpId           Associated Fp Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SdhFpPolaritySet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_SDH_FP_TYPE_REG(fpId),
                              ZL303XX_SDH_FP_POLARITY_MASK,
                              ZL303XX_SDH_FP_POLARITY_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_SdhFpPolaritySet */


/*

  Function Name:
   zl303xx_SdhFpFineOffsetGet

  Details:
   Reads the Fp Fine Offset attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    fpId           Associated Fp Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SdhFpFineOffsetGet(zl303xx_ParamsS *zl303xx_Params,
                                     zl303xx_FpIdE fpId, Uint32T *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
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
                              ZL303XX_SDH_FP_FINE_OFFSET_REG(fpId),
                              ZL303XX_SDH_FP_FINE_OFFSET_MASK,
                              ZL303XX_SDH_FP_FINE_OFFSET_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (Uint32T)(attrPar.value);
   }

   return status;
}  /* END zl303xx_SdhFpFineOffsetGet */

/*

  Function Name:
   zl303xx_SdhFpFineOffsetSet

  Details:
   Writes the Fp Fine Offset attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    fpId           Associated Fp Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SdhFpFineOffsetSet(zl303xx_ParamsS *zl303xx_Params,
                                     zl303xx_FpIdE fpId, Uint32T val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check params pointer */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
   }

   /* Check the fpId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_FP_ID(fpId);
   }

   /* Check that the write value is within range */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_FP_FINE_OFFSET(val);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_SDH_FP_FINE_OFFSET_REG(fpId),
                              ZL303XX_SDH_FP_FINE_OFFSET_MASK,
                              ZL303XX_SDH_FP_FINE_OFFSET_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_SdhFpFineOffsetSet */


/*

  Function Name:
   zl303xx_SdhFpCoarseOffsetGet

  Details:
   Reads the Fp Coarse Offset attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    fpId           Associated Fp Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SdhFpCoarseOffsetGet(zl303xx_ParamsS *zl303xx_Params,
                                       zl303xx_FpIdE fpId, Uint32T *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
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
                              ZL303XX_SDH_FP_COARSE_OFFSET_REG(fpId),
                              ZL303XX_SDH_FP_COARSE_OFFSET_MASK,
                              ZL303XX_SDH_FP_COARSE_OFFSET_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (Uint32T)(attrPar.value);
   }

   return status;
}  /* END zl303xx_SdhFpCoarseOffsetGet */

/*

  Function Name:
   zl303xx_SdhFpCoarseOffsetSet

  Details:
   Writes the Fp Coarse Offset attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    fpId           Associated Fp Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SdhFpCoarseOffsetSet(zl303xx_ParamsS *zl303xx_Params,
                                       zl303xx_FpIdE fpId, Uint32T val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check params pointer */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
   }

   /* Check the fpId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_FP_ID(fpId);
   }

   /* Check that the write value is within range */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_FP_COARSE_OFFSET(val);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_SDH_FP_COARSE_OFFSET_REG(fpId),
                              ZL303XX_SDH_FP_COARSE_OFFSET_MASK,
                              ZL303XX_SDH_FP_COARSE_OFFSET_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_SdhFpCoarseOffsetSet */


