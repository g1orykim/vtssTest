

/*******************************************************************************
*
*  $Id: zl303xx_DiffLow.c 6058 2011-06-10 14:37:16Z AW $
*
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     Functions for low-level DIFF attribute access
*
*******************************************************************************/

/*****************   INCLUDE FILES   ******************************************/

#include "zl303xx_ApiLow.h"
#include "zl303xx_DiffLow.h"
#include "zl303xx_SdhLow.h"

/*****************   DEFINES   ************************************************/

/*****************   DATA TYPES   *********************************************/

/*****************   DATA STRUCTURES   ****************************************/

/*****************   EXPORTED GLOBAL VARIABLE DECLARATIONS   ******************/

/*

  Function Name:
   zl303xx_DiffConfigStructInit

  Details:
   Initializes the members of the zl303xx_DiffConfigS data structure

  Parameters:
   [in]   zl303xx_Params Pointer to the device instance parameter structure
   [in]   par            Pointer to the zl303xx_DiffConfigS structure to Initialize

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DiffConfigStructInit(zl303xx_ParamsS *zl303xx_Params,
                                       zl303xx_DiffConfigS *par)
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
      par->run = (zl303xx_DiffRunE)ZL303XX_INVALID;
      par->offset = (zl303xx_DiffOffsetE)ZL303XX_INVALID;
      par->freq = (zl303xx_DiffFreqE)ZL303XX_INVALID;
      par->Id = (zl303xx_DiffIdE)ZL303XX_INVALID;
   }

   return status;
} /* END zl303xx_DiffConfigStructInit */

/*

  Function Name:
   zl303xx_DiffConfigCheck

  Details:
   Checks the members of the zl303xx_DiffConfigS data structure

  Parameters:
   [in]   zl303xx_Params Pointer to the device instance parameter structure
   [in]   par            Pointer to the zl303xx_DiffConfigS structure to Check

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DiffConfigCheck(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_DiffConfigS *par)
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
               ZL303XX_CHECK_DIFF_RUN(par->run) |
               ZL303XX_CHECK_DIFF_OFFSET(par->offset) |
               ZL303XX_CHECK_DIFF_FREQ(par->freq) |
               ZL303XX_CHECK_DIFF_ID(par->Id);
   }

   return status;
} /* END zl303xx_DiffConfigCheck */

/*

  Function Name:
   zl303xx_DiffConfigGet

  Details:
   Gets the members of the zl303xx_DiffConfigS data structure

  Parameters:
   [in]   zl303xx_Params Pointer to the device instance parameter structure
   [in]   par            Pointer to the zl303xx_DiffConfigS structure to Get

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DiffConfigGet(zl303xx_ParamsS *zl303xx_Params,
                                zl303xx_DiffConfigS *par)
{
   zlStatusE status = ZL303XX_OK;
   Uint32T regValue = 0, ethDiv = 0, freqConv = 0;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, par);
   }

   /* Check for a valid DiffId */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_DIFF_ID(par->Id);
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
                            ZL303XX_DIFF_CTRL_REG,
                            &regValue);
   }

   /* Extract */
   if (status == ZL303XX_OK)
   {
      par->enable = (zl303xx_OutEnableE)ZL303XX_EXTRACT(regValue,
                                        ZL303XX_DIFF_ENABLE_MASK,
                                        ZL303XX_DIFF_ENABLE_SHIFT(par->Id));

      par->run = (zl303xx_DiffRunE)ZL303XX_EXTRACT(regValue,
                                    ZL303XX_DIFF_RUN_MASK,
                                    ZL303XX_DIFF_RUN_SHIFT(par->Id));

      par->offset = (zl303xx_DiffOffsetE)ZL303XX_EXTRACT(regValue,
                                          ZL303XX_DIFF_OFFSET_MASK,
                                          ZL303XX_DIFF_OFFSET_SHIFT(par->Id));

      par->freq = (zl303xx_DiffFreqE)ZL303XX_EXTRACT(regValue,
                                      ZL303XX_DIFF_FREQ_MASK,
                                      ZL303XX_DIFF_FREQ_SHIFT(par->Id));

      /* Need eth_en and f_sel_n bits (stored in freqConv and ethDiv respectively) to know
         the actual frequency being output. Re-encode these bits into the enum value. */
      par->freq |= (ethDiv << 4) | (freqConv << 5);
   }

   return status;
} /* END zl303xx_DiffConfigGet */

/*

  Function Name:
   zl303xx_DiffConfigSet

  Details:
   Sets the members of the zl303xx_DiffConfigS data structure

  Parameters:
   [in]   zl303xx_Params Pointer to the device instance parameter structure
   [in]   par            Pointer to the zl303xx_DiffConfigS structure to Set

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DiffConfigSet(zl303xx_ParamsS *zl303xx_Params,
                                zl303xx_DiffConfigS *par)
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
      status = zl303xx_DiffConfigCheck(zl303xx_Params, par);
   }

   /* Package */
   if (status == ZL303XX_OK)
   {
      regValue = 0;
      mask = 0;

      /* enable */
      ZL303XX_INSERT(regValue, par->enable,
                             ZL303XX_DIFF_ENABLE_MASK,
                             ZL303XX_DIFF_ENABLE_SHIFT(par->Id));

      /* run */
      ZL303XX_INSERT(regValue, par->run,
                             ZL303XX_DIFF_RUN_MASK,
                             ZL303XX_DIFF_RUN_SHIFT(par->Id));

      /* offset */
      ZL303XX_INSERT(regValue, par->offset,
                             ZL303XX_DIFF_OFFSET_MASK,
                             ZL303XX_DIFF_OFFSET_SHIFT(par->Id));

      /* freq */
      ZL303XX_INSERT(regValue, par->freq,
                             ZL303XX_DIFF_FREQ_MASK,
                             ZL303XX_DIFF_FREQ_SHIFT(par->Id));

      mask |= (ZL303XX_DIFF_ENABLE_MASK << ZL303XX_DIFF_ENABLE_SHIFT(par->Id)) |
              (ZL303XX_DIFF_RUN_MASK << ZL303XX_DIFF_RUN_SHIFT(par->Id)) |
              (ZL303XX_DIFF_OFFSET_MASK << ZL303XX_DIFF_OFFSET_SHIFT(par->Id)) |
              (ZL303XX_DIFF_FREQ_MASK << ZL303XX_DIFF_FREQ_SHIFT(par->Id));

      /* Write the Data for this Register */
      status = zl303xx_ReadModWrite(zl303xx_Params, NULL,
                                    ZL303XX_DIFF_CTRL_REG,
                                    regValue, mask, NULL);
   }

   /* Package */
   if (status == ZL303XX_OK)
   {
      regValue = 0;
      mask = 0;

      /* ethDiv (f_sel_n) is encoded in bit4 of the freq enum value */
      ZL303XX_INSERT(regValue, ZL303XX_EXTRACT(par->freq, 1, 4),
                             ZL303XX_SDH_CLK_ETH_DIV_MASK,
                             ZL303XX_SDH_CLK_ETH_DIV_SHIFT(par->Id));

      /* freqConv (eth_en) is encoded in bit5 of the freq enum value */
      ZL303XX_INSERT(regValue, ZL303XX_EXTRACT(par->freq, 1, 5),
                             ZL303XX_SDH_FREQ_CONV_MASK,
                             ZL303XX_SDH_FREQ_CONV_SHIFT);

      mask |= (ZL303XX_SDH_CLK_ETH_DIV_MASK << ZL303XX_SDH_CLK_ETH_DIV_SHIFT(par->Id)) |
              (ZL303XX_SDH_FREQ_CONV_MASK << ZL303XX_SDH_FREQ_CONV_SHIFT);

      /* Write the Data for this Register */
      status = zl303xx_ReadModWrite(zl303xx_Params, NULL,
                                    ZL303XX_SDH_OUTPUT_CTRL_REG,
                                    regValue, mask, NULL);
   }

   return status;
} /* END zl303xx_DiffConfigSet */

/*

  Function Name:
   zl303xx_DiffEnableGet

  Details:
   Reads the Diff Enable attribute

  Parameters:
   [in]
   [in]   zl303xx_Params Pointer to the device instance parameter structure
   [in]   diffId         Associated DIFF Id of the attribute
   [in]   val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DiffEnableGet(zl303xx_ParamsS *zl303xx_Params,
                                zl303xx_DiffIdE diffId,
                                zl303xx_OutEnableE *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
   }

   /* Check the diffId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_DIFF_ID(diffId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_DIFF_CTRL_REG,
                              ZL303XX_DIFF_ENABLE_MASK,
                              ZL303XX_DIFF_ENABLE_SHIFT(diffId));
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_OutEnableE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_DiffEnableGet */

/*

  Function Name:
   zl303xx_DiffEnableSet

  Details:
   Writes the Diff Enable attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    diffId         Associated DIFF Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DiffEnableSet(zl303xx_ParamsS *zl303xx_Params,
                                zl303xx_DiffIdE diffId,
                                zl303xx_OutEnableE val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check params pointer */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
   }

   /* Check the diffId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_DIFF_ID(diffId);
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
                              ZL303XX_DIFF_CTRL_REG,
                              ZL303XX_DIFF_ENABLE_MASK,
                              ZL303XX_DIFF_ENABLE_SHIFT(diffId));
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_DiffEnableSet */


/*

  Function Name:
   zl303xx_DiffRunGet

  Details:
   Reads the Diff Run attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    diffId         Associated DIFF Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DiffRunGet(zl303xx_ParamsS *zl303xx_Params,
                             zl303xx_DiffIdE diffId,
                             zl303xx_DiffRunE *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
   }

   /* Check the diffId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_DIFF_ID(diffId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_DIFF_CTRL_REG,
                              ZL303XX_DIFF_RUN_MASK,
                              ZL303XX_DIFF_RUN_SHIFT(diffId));
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_DiffRunE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_DiffRunGet */

/*

  Function Name:
   zl303xx_DiffRunSet

  Details:
   Writes the Diff Run attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    diffId         Associated DIFF Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DiffRunSet(zl303xx_ParamsS *zl303xx_Params,
                             zl303xx_DiffIdE diffId,
                             zl303xx_DiffRunE val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check params pointer */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
   }

   /* Check the diffId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_DIFF_ID(diffId);
   }

   /* Check that the write value is within range */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_DIFF_RUN(val);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_DIFF_CTRL_REG,
                              ZL303XX_DIFF_RUN_MASK,
                              ZL303XX_DIFF_RUN_SHIFT(diffId));
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_DiffRunSet */


/*

  Function Name:
   zl303xx_DiffOffsetGet

  Details:
   Reads the Diff Adjust attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    diffId         Associated DIFF Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DiffOffsetGet(zl303xx_ParamsS *zl303xx_Params,
                                zl303xx_DiffIdE diffId,
                                zl303xx_DiffOffsetE *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
   }

   /* Check the diffId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_DIFF_ID(diffId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_DIFF_CTRL_REG,
                              ZL303XX_DIFF_OFFSET_MASK,
                              ZL303XX_DIFF_OFFSET_SHIFT(diffId));
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_DiffOffsetE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_DiffOffsetGet */

/*

  Function Name:
   zl303xx_DiffOffsetSet

  Details:
   Writes the Diff Adjust attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    diffId         Associated DIFF Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DiffOffsetSet(zl303xx_ParamsS *zl303xx_Params,
                                zl303xx_DiffIdE diffId,
                                zl303xx_DiffOffsetE val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check params pointer */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
   }

   /* Check the diffId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_DIFF_ID(diffId);
   }

   /* Check that the write value is within range */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_DIFF_OFFSET(val);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_DIFF_CTRL_REG,
                              ZL303XX_DIFF_OFFSET_MASK,
                              ZL303XX_DIFF_OFFSET_SHIFT(diffId));
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_DiffOffsetSet */


/*

  Function Name:
   zl303xx_DiffFreqGet

  Details:
   Reads the Diff Freq attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    diffId         Associated DIFF Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DiffFreqGet(zl303xx_ParamsS *zl303xx_Params,
                              zl303xx_DiffIdE diffId,
                              zl303xx_DiffFreqE *val)
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

   /* Check the diffId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_DIFF_ID(diffId);
   }

   /* Frequency output is dependent on f_sel_n and eth_en bits */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_SdhClkEthDivGet(zl303xx_Params,
                                     (zl303xx_ClockIdE)diffId,
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
                              ZL303XX_DIFF_CTRL_REG,
                              ZL303XX_DIFF_FREQ_MASK,
                              ZL303XX_DIFF_FREQ_SHIFT(diffId));
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_DiffFreqE)(attrPar.value);

      /* Need eth_en and f_sel_n bits (stored in freqConv and ethDiv respectively) to know
         the actual frequency being output. Re-encode these bits into the enum value. */
      *val |= (ethDiv << 4) | (freqConv << 5);
   }

   return status;
}  /* END zl303xx_DiffFreqGet */

/*

  Function Name:
   zl303xx_DiffFreqSet

  Details:
   Writes the Diff Freq attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    diffId         Associated DIFF Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DiffFreqSet(zl303xx_ParamsS *zl303xx_Params,
                              zl303xx_DiffIdE diffId,
                              zl303xx_DiffFreqE val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check params pointer */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
   }

   /* Check the diffId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_DIFF_ID(diffId);
   }

   /* Check that the write value is within range */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_DIFF_FREQ(val);
   }

   /* f_sel_n and eth_en bits are encoded in the frequency enum value */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_SdhClkEthDivSet(zl303xx_Params,
                                     (zl303xx_ClockIdE)diffId,
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
                              ZL303XX_DIFF_CTRL_REG,
                              ZL303XX_DIFF_FREQ_MASK,
                              ZL303XX_DIFF_FREQ_SHIFT(diffId));
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_DiffFreqSet */


