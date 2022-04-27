

/*******************************************************************************
*
*  $Id: zl303xx_FbkLow.c 6058 2011-06-10 14:37:16Z AW $
*
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     Functions for low-level FBK attribute access
*
*******************************************************************************/

/*****************   INCLUDE FILES   ******************************************/

#include "zl303xx_ApiLow.h"
#include "zl303xx_FbkLow.h"
#include "zl303xx_PllFuncs.h"

/*****************   DEFINES   ************************************************/

/*****************   DATA TYPES   *********************************************/

/*****************   DATA STRUCTURES   ****************************************/

/*****************   EXPORTED GLOBAL VARIABLE DECLARATIONS   ******************/

/*

  Function Name:
   zl303xx_FeedbackConfigStructInit

  Details:
   Initializes the members of the zl303xx_FeedbackConfigS data structure

  Parameters:
   [in]   zl303xx_Params Pointer to the device instance parameter structure
   [in]   par            Pointer to the zl303xx_FeedbackConfigS structure to Initialize

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_FeedbackConfigStructInit(zl303xx_ParamsS *zl303xx_Params,
                                           zl303xx_FeedbackConfigS *par)
{
   zlStatusE status = ZL303XX_OK;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, par);
   }

   if (status == ZL303XX_OK)
   {
      par->outEnable = (zl303xx_OutEnableE)ZL303XX_INVALID;
      par->pinConfig = (zl303xx_FbkPinStateE)ZL303XX_INVALID;
      par->fineDelay = ZL303XX_INVALID;
      par->enable = (zl303xx_BooleanE)ZL303XX_INVALID;
   }

   return status;
} /* END zl303xx_FeedbackConfigStructInit */

/*

  Function Name:
   zl303xx_FeedbackConfigCheck

  Details:
   Checks the members of the zl303xx_FeedbackConfigS data structure

  Parameters:
   [in]   zl303xx_Params Pointer to the device instance parameter structure
   [in]   par            Pointer to the zl303xx_FeedbackConfigS structure to Check

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_FeedbackConfigCheck(zl303xx_ParamsS *zl303xx_Params,
                                      zl303xx_FeedbackConfigS *par)
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
      status = ZL303XX_CHECK_OUT_ENABLE(par->outEnable) |
               ZL303XX_CHECK_FBK_PIN_STATE(par->pinConfig) |
               zl303xx_CheckOutputFineOffset(par->fineDelay) |
               ZL303XX_CHECK_BOOLEAN(par->enable);
   }

   return status;
} /* END zl303xx_FeedbackConfigCheck */

/*

  Function Name:
   zl303xx_FeedbackConfigGet

  Details:
   Gets the members of the zl303xx_FeedbackConfigS data structure

  Parameters:
   [in]   zl303xx_Params Pointer to the device instance parameter structure
   [in]   par            Pointer to the zl303xx_FeedbackConfigS structure to Get

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_FeedbackConfigGet(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_FeedbackConfigS *par)
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
                            ZL303XX_FBK_CTRL_REG,
                            &regValue);
   }

   /* Extract */
   if (status == ZL303XX_OK)
   {
      par->outEnable = (zl303xx_OutEnableE)ZL303XX_EXTRACT(regValue,
                                           ZL303XX_FBK_CLK_ENABLE_MASK,
                                           ZL303XX_FBK_CLK_ENABLE_SHIFT);

      par->pinConfig = (zl303xx_FbkPinStateE)ZL303XX_EXTRACT(regValue,
                                              ZL303XX_FBK_PIN_CONFIG_MASK,
                                              ZL303XX_FBK_PIN_CONFIG_SHIFT);
   }

   /* Read */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_Read(zl303xx_Params, NULL,
                            ZL303XX_FBK_FINE_DELAY_REG,
                            &regValue);
   }

   /* Extract */
   if (status == ZL303XX_OK)
   {
      par->fineDelay = zl303xx_OutputFineOffsetInPs(regValue);
   }

   return status;
} /* END zl303xx_FeedbackConfigGet */

/*

  Function Name:
   zl303xx_FeedbackConfigSet

  Details:
   Sets the members of the zl303xx_FeedbackConfigS data structure

  Parameters:
   [in]   zl303xx_Params Pointer to the device instance parameter structure
   [in]   par            Pointer to the zl303xx_FeedbackConfigS structure to Set

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_FeedbackConfigSet(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_FeedbackConfigS *par)
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
      status = zl303xx_FeedbackConfigCheck(zl303xx_Params, par);
   }

   /* Package */
   if (status == ZL303XX_OK)
   {
      regValue = 0;
      mask = 0;

      /* outEnable */
      ZL303XX_INSERT(regValue, par->outEnable,
                             ZL303XX_FBK_CLK_ENABLE_MASK,
                             ZL303XX_FBK_CLK_ENABLE_SHIFT);

      /* pinConfig */
      ZL303XX_INSERT(regValue, par->pinConfig,
                             ZL303XX_FBK_PIN_CONFIG_MASK,
                             ZL303XX_FBK_PIN_CONFIG_SHIFT);

      mask |= (ZL303XX_FBK_CLK_ENABLE_MASK << ZL303XX_FBK_CLK_ENABLE_SHIFT) |
              (ZL303XX_FBK_PIN_CONFIG_MASK << ZL303XX_FBK_PIN_CONFIG_SHIFT);

      /* Write the Data for this Register */
      status = zl303xx_ReadModWrite(zl303xx_Params, NULL,
                                    ZL303XX_FBK_CTRL_REG,
                                    regValue, mask, NULL);
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
                                 ZL303XX_FBK_FINE_DELAY_MASK,
                                 ZL303XX_FBK_FINE_DELAY_SHIFT);

          mask |= (ZL303XX_FBK_FINE_DELAY_MASK << ZL303XX_FBK_FINE_DELAY_SHIFT);

          /* Write the Data for this Register */
          status = zl303xx_ReadModWrite(zl303xx_Params, NULL,
                                        ZL303XX_FBK_FINE_DELAY_REG,
                                        regValue, mask, NULL);
      }
   }

   return status;
} /* END zl303xx_FeedbackConfigSet */

/*

  Function Name:
   zl303xx_FbkClkEnableGet

  Details:
   Reads the Clk Enable attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_FbkClkEnableGet(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_OutEnableE *val)
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
                              ZL303XX_FBK_CTRL_REG,
                              ZL303XX_FBK_CLK_ENABLE_MASK,
                              ZL303XX_FBK_CLK_ENABLE_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_OutEnableE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_FbkClkEnableGet */

/*

  Function Name:
   zl303xx_FbkClkEnableSet

  Details:
   Writes the Clk Enable attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_FbkClkEnableSet(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_OutEnableE val)
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
      status = ZL303XX_CHECK_OUT_ENABLE(val);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_FBK_CTRL_REG,
                              ZL303XX_FBK_CLK_ENABLE_MASK,
                              ZL303XX_FBK_CLK_ENABLE_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_FbkClkEnableSet */

/*

  Function Name:
   zl303xx_FbkPllEnableGet

  Details:
   Reads the PLL Enable attribute

  Parameters:
   [in]    zl303xx_Params   Pointer to the device instance parameter structure
   [in]    val            Pointer to the device attribute parameter read

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_FbkPllEnableGet(zl303xx_ParamsS *zl303xx_Params,
                                zl303xx_EnableE *val)
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
                              ZL303XX_FBK_CTRL_REG,
                              ZL303XX_FBK_PLL_ENABLE_MASK,
                              ZL303XX_FBK_PLL_ENABLE_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_EnableE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_FbkPllEnableGet */

/*

  Function Name:
   zl303xx_FbkPllEnableSet

  Details:
   Writes the PLL Enable attribute

  Parameters:
   [in]    zl303xx_Params   Pointer to the device instance parameter structure
   [in]    val            Pointer to the device attribute parameter read

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_FbkPllEnableSet(zl303xx_ParamsS *zl303xx_Params, zl303xx_EnableE val)
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
      status = ZL303XX_CHECK_ENABLE(val);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_FBK_CTRL_REG,
                              ZL303XX_FBK_PLL_ENABLE_MASK,
                              ZL303XX_FBK_PLL_ENABLE_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_FbkPllEnableSet */

/*

  Function Name:
   zl303xx_FbkPinConfigGet

  Details:
   Reads the Fbk Type attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_FbkPinConfigGet(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_FbkPinStateE *val)
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
                              ZL303XX_FBK_CTRL_REG,
                              ZL303XX_FBK_PIN_CONFIG_MASK,
                              ZL303XX_FBK_PIN_CONFIG_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_FbkPinStateE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_FbkPinConfigGet */

/*

  Function Name:
   zl303xx_FbkPinConfigSet

  Details:
   Writes the Fbk Type attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_FbkPinConfigSet(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_FbkPinStateE val)
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
      status = ZL303XX_CHECK_FBK_PIN_STATE(val);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_FBK_CTRL_REG,
                              ZL303XX_FBK_PIN_CONFIG_MASK,
                              ZL303XX_FBK_PIN_CONFIG_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_FbkPinConfigSet */


/*

  Function Name:
   zl303xx_FbkFineDelayGet

  Details:
   Reads the Fine Tune Delay attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_FbkFineDelayGet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_FBK_FINE_DELAY_REG,
                              ZL303XX_FBK_FINE_DELAY_MASK,
                              ZL303XX_FBK_FINE_DELAY_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (Uint32T)(attrPar.value);
   }

   return status;
}  /* END zl303xx_FbkFineDelayGet */

/*

  Function Name:
   zl303xx_FbkFineDelaySet

  Details:
   Writes the Fine Tune Delay attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_FbkFineDelaySet(zl303xx_ParamsS *zl303xx_Params, Uint32T val)
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
                              ZL303XX_FBK_FINE_DELAY_REG,
                              ZL303XX_FBK_FINE_DELAY_MASK,
                              ZL303XX_FBK_FINE_DELAY_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_FbkFineDelaySet */
