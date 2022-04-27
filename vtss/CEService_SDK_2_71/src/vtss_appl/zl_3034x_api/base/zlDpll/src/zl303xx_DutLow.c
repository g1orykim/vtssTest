

/*******************************************************************************
*
*  $Id: zl303xx_DutLow.c 6058 2011-06-10 14:37:16Z AW $
*
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     Functions for low-level DUT attribute access
*
*******************************************************************************/

/*****************   INCLUDE FILES   ******************************************/

#include "zl303xx_ApiLow.h"
#include "zl303xx_DutLow.h"

/*****************   DEFINES   ************************************************/

/*****************   DATA TYPES   *********************************************/

/*****************   DATA STRUCTURES   ****************************************/

/*****************   EXPORTED GLOBAL VARIABLE DECLARATIONS   ******************/

/*

  Function Name:
   zl303xx_DutIdGet

  Details:
   Reads the Device Id attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DutIdGet(zl303xx_ParamsS *zl303xx_Params,
                           zl303xx_DeviceIdE *val)
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
                              ZL303XX_DUT_DEVICE_ID_REG,
                              ZL303XX_DUT_ID_MASK,
                              ZL303XX_DUT_ID_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_DeviceIdE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_DutIdGet */


/*

  Function Name:
   zl303xx_DutRevisionGet

  Details:
   Reads the Device Revision attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DutRevisionGet(zl303xx_ParamsS *zl303xx_Params, Uint32T *val)
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
                              ZL303XX_DUT_DEVICE_ID_REG,
                              ZL303XX_DUT_REVISION_MASK,
                              ZL303XX_DUT_REVISION_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (Uint32T)(attrPar.value);
   }

   return status;
}  /* END zl303xx_DutRevisionGet */


/*

  Function Name:
   zl303xx_DutReadyStatusGet

  Details:
   Reads the Device Ready Status attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DutReadyStatusGet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_DUT_DEVICE_ID_REG,
                              ZL303XX_DUT_READY_STATUS_MASK,
                              ZL303XX_DUT_READY_STATUS_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_BooleanE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_DutReadyStatusGet */


/*

  Function Name:
   zl303xx_DutDpllModeUseHwCtrlGet

  Details:
   Reads the Dpll Mode Hw Ctrl attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DutDpllModeUseHwCtrlGet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_DUT_USE_HW_CTRL_REG,
                              ZL303XX_DUT_DPLL_MODE_USE_HW_CTRL_MASK,
                              ZL303XX_DUT_DPLL_MODE_USE_HW_CTRL_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_BooleanE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_DutDpllModeUseHwCtrlGet */

/*

  Function Name:
   zl303xx_DutDpllModeUseHwCtrlSet

  Details:
   Writes the Dpll Mode Hw Ctrl attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DutDpllModeUseHwCtrlSet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_DUT_USE_HW_CTRL_REG,
                              ZL303XX_DUT_DPLL_MODE_USE_HW_CTRL_MASK,
                              ZL303XX_DUT_DPLL_MODE_USE_HW_CTRL_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_DutDpllModeUseHwCtrlSet */


/*

  Function Name:
   zl303xx_DutSlaveEnUseHwCtrlGet

  Details:
   Reads the Slave Enable Hw Ctrl attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DutSlaveEnUseHwCtrlGet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_DUT_USE_HW_CTRL_REG,
                              ZL303XX_DUT_SLAVE_EN_USE_HW_CTRL_MASK,
                              ZL303XX_DUT_SLAVE_EN_USE_HW_CTRL_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_BooleanE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_DutSlaveEnUseHwCtrlGet */

/*

  Function Name:
   zl303xx_DutSlaveEnUseHwCtrlSet

  Details:
   Writes the Slave Enable Hw Ctrl attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DutSlaveEnUseHwCtrlSet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_DUT_USE_HW_CTRL_REG,
                              ZL303XX_DUT_SLAVE_EN_USE_HW_CTRL_MASK,
                              ZL303XX_DUT_SLAVE_EN_USE_HW_CTRL_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_DutSlaveEnUseHwCtrlSet */


/*

  Function Name:
   zl303xx_DutPageCtrlGet

  Details:
   Reads the Page Value attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DutPageCtrlGet(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_PageIdE *val)
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
                              ZL303XX_DUT_PAGE_CTRL_REG,
                              ZL303XX_DUT_PAGE_CTRL_MASK,
                              ZL303XX_DUT_PAGE_CTRL_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_PageIdE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_DutPageCtrlGet */

/*

  Function Name:
   zl303xx_DutPageCtrlSet

  Details:
   Writes the Page Value attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DutPageCtrlSet(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_PageIdE val)
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
      status = ZL303XX_CHECK_PAGE_ID(val);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_DUT_PAGE_CTRL_REG,
                              ZL303XX_DUT_PAGE_CTRL_MASK,
                              ZL303XX_DUT_PAGE_CTRL_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_DutPageCtrlSet */
