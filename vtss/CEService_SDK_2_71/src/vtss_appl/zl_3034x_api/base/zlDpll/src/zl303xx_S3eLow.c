

/*******************************************************************************
*
*  $Id: zl303xx_S3eLow.c 6058 2011-06-10 14:37:16Z AW $
*
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     Functions for low-level S3E attribute access
*
*******************************************************************************/

/*****************   INCLUDE FILES   ******************************************/

#include "zl303xx_ApiLow.h"
#include "zl303xx_S3eLow.h"

/*****************   DEFINES   ************************************************/

/*****************   DATA TYPES   *********************************************/

/*****************   DATA STRUCTURES   ****************************************/

/*****************   EXPORTED GLOBAL VARIABLE DECLARATIONS   ******************/


/*

  Function Name:
   zl303xx_S3ePboThresholdGet

  Details:
   Reads the PBO Threshold attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_S3ePboThresholdGet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_S3E_PBO_THRESHOLD_REG,
                              ZL303XX_S3E_PBO_THRESHOLD_MASK,
                              ZL303XX_S3E_PBO_THRESHOLD_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (Uint32T)(attrPar.value);
   }

   return status;
}  /* END zl303xx_S3ePboThresholdGet */

/*

  Function Name:
   zl303xx_S3ePboThresholdSet

  Details:
   Writes the PBO Threshold attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_S3ePboThresholdSet(zl303xx_ParamsS *zl303xx_Params,
                                     Uint32T val)
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
      status = ZL303XX_CHECK_PBO_THRESHOLD(val);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_S3E_PBO_THRESHOLD_REG,
                              ZL303XX_S3E_PBO_THRESHOLD_MASK,
                              ZL303XX_S3E_PBO_THRESHOLD_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_S3ePboThresholdSet */


/*

  Function Name:
   zl303xx_S3ePboMinSlopeGet

  Details:
   Reads the PBO Min Phase Chg attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_S3ePboMinSlopeGet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_S3E_PBO_MIN_SLOPE_REG,
                              ZL303XX_S3E_PBO_MIN_SLOPE_MASK,
                              ZL303XX_S3E_PBO_MIN_SLOPE_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (Uint32T)(attrPar.value);
   }

   return status;
}  /* END zl303xx_S3ePboMinSlopeGet */

/*

  Function Name:
   zl303xx_S3ePboMinSlopeSet

  Details:
   Writes the PBO Min Phase Chg attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_S3ePboMinSlopeSet(zl303xx_ParamsS *zl303xx_Params,
                                    Uint32T val)
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
      status = ZL303XX_CHECK_PBO_MIN_SLOPE(val);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_S3E_PBO_MIN_SLOPE_REG,
                              ZL303XX_S3E_PBO_MIN_SLOPE_MASK,
                              ZL303XX_S3E_PBO_MIN_SLOPE_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_S3ePboMinSlopeSet */


/*

  Function Name:
   zl303xx_S3ePboEndIntGet

  Details:
   Reads the PBO Transient Interval attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_S3ePboEndIntGet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_S3E_PBO_TRANS_INT_REG,
                              ZL303XX_S3E_PBO_END_INT_MASK,
                              ZL303XX_S3E_PBO_END_INT_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (Uint32T)(attrPar.value);
   }

   return status;
}  /* END zl303xx_S3ePboEndIntGet */

/*

  Function Name:
   zl303xx_S3ePboEndIntSet

  Details:
   Writes the PBO Transient Interval attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_S3ePboEndIntSet(zl303xx_ParamsS *zl303xx_Params, Uint32T val)
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
      status = ZL303XX_CHECK_PBO_END_INT(val);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_S3E_PBO_TRANS_INT_REG,
                              ZL303XX_S3E_PBO_END_INT_MASK,
                              ZL303XX_S3E_PBO_END_INT_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_S3ePboEndIntSet */


/*

  Function Name:
   zl303xx_S3ePboToutGet

  Details:
   Reads the PBO Timeout attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_S3ePboToutGet(zl303xx_ParamsS *zl303xx_Params, Uint32T *val)
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
                              ZL303XX_S3E_PBO_TIMEOUT_REG,
                              ZL303XX_S3E_PBO_TOUT_MASK,
                              ZL303XX_S3E_PBO_TOUT_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (Uint32T)(attrPar.value);
   }

   return status;
}  /* END zl303xx_S3ePboToutGet */

/*

  Function Name:
   zl303xx_S3ePboToutSet

  Details:
   Writes the PBO Timeout attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_S3ePboToutSet(zl303xx_ParamsS *zl303xx_Params, Uint32T val)
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
      status = ZL303XX_CHECK_PBO_TOUT(val);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_S3E_PBO_TIMEOUT_REG,
                              ZL303XX_S3E_PBO_TOUT_MASK,
                              ZL303XX_S3E_PBO_TOUT_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_S3ePboToutSet */


/*

  Function Name:
   zl303xx_S3ePboMagnGet

  Details:
   Reads the PBO Magnitude attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_S3ePboMagnGet(zl303xx_ParamsS *zl303xx_Params, Uint32T *val)
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
                              ZL303XX_S3E_PBO_MAGNITUDE_REG,
                              ZL303XX_S3E_PBO_MAGN_MASK,
                              ZL303XX_S3E_PBO_MAGN_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (Uint32T)(attrPar.value);
   }

   return status;
}  /* END zl303xx_S3ePboMagnGet */

/*

  Function Name:
   zl303xx_S3ePboMagnSet

  Details:
   Writes the PBO Magnitude attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_S3ePboMagnSet(zl303xx_ParamsS *zl303xx_Params, Uint32T val)
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
      status = ZL303XX_CHECK_PBO_MAGN(val);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_S3E_PBO_MAGNITUDE_REG,
                              ZL303XX_S3E_PBO_MAGN_MASK,
                              ZL303XX_S3E_PBO_MAGN_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_S3ePboMagnSet */


/*

  Function Name:
   zl303xx_S3ePboEnGet

  Details:
   Reads the PBO Enable attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_S3ePboEnGet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_S3E_CONTROL_REG,
                              ZL303XX_S3E_PBO_EN_MASK,
                              ZL303XX_S3E_PBO_EN_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_BooleanE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_S3ePboEnGet */

/*

  Function Name:
   zl303xx_S3ePboEnSet

  Details:
   Writes the PBO Enable attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_S3ePboEnSet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_S3E_CONTROL_REG,
                              ZL303XX_S3E_PBO_EN_MASK,
                              ZL303XX_S3E_PBO_EN_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_S3ePboEnSet */


/*

  Function Name:
   zl303xx_S3eFlockEnGet

  Details:
   Reads the Fast Lock Enable attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_S3eFlockEnGet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_S3E_CONTROL_REG,
                              ZL303XX_S3E_FLOCK_EN_MASK,
                              ZL303XX_S3E_FLOCK_EN_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_BooleanE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_S3eFlockEnGet */

/*

  Function Name:
   zl303xx_S3eFlockEnSet

  Details:
   Writes the Fast Lock Enable attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_S3eFlockEnSet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_S3E_CONTROL_REG,
                              ZL303XX_S3E_FLOCK_EN_MASK,
                              ZL303XX_S3E_FLOCK_EN_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_S3eFlockEnSet */


/*

  Function Name:
   zl303xx_S3eForceFlockGet

  Details:
   Reads the Force Fast Lock attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_S3eForceFlockGet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_S3E_CONTROL_REG,
                              ZL303XX_S3E_FORCE_FLOCK_MASK,
                              ZL303XX_S3E_FORCE_FLOCK_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_BooleanE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_S3eForceFlockGet */

/*

  Function Name:
   zl303xx_S3eForceFlockSet

  Details:
   Writes the Force Fast Lock attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_S3eForceFlockSet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_S3E_CONTROL_REG,
                              ZL303XX_S3E_FORCE_FLOCK_MASK,
                              ZL303XX_S3E_FORCE_FLOCK_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_S3eForceFlockSet */


/*

  Function Name:
   zl303xx_S3ePlimEnGet

  Details:
   Reads the Ph Limit En attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_S3ePlimEnGet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_S3E_CONTROL_REG,
                              ZL303XX_S3E_PLIM_EN_MASK,
                              ZL303XX_S3E_PLIM_EN_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_BooleanE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_S3ePlimEnGet */

/*

  Function Name:
   zl303xx_S3ePlimEnSet

  Details:
   Writes the Ph Limit En attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_S3ePlimEnSet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_S3E_CONTROL_REG,
                              ZL303XX_S3E_PLIM_EN_MASK,
                              ZL303XX_S3E_PLIM_EN_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_S3ePlimEnSet */


/*

  Function Name:
   zl303xx_S3eDampingStage1Get

  Details:
   Reads the Damping Stage 1 attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_S3eDampingStage1Get(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_S3E_FAST_LOCK_CTRL0_REG,
                              ZL303XX_S3E_DAMPING_STAGE1_MASK,
                              ZL303XX_S3E_DAMPING_STAGE1_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (Uint32T)(attrPar.value);
   }

   return status;
}  /* END zl303xx_S3eDampingStage1Get */

/*

  Function Name:
   zl303xx_S3eDampingStage1Set

  Details:
   Writes the Damping Stage 1 attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_S3eDampingStage1Set(zl303xx_ParamsS *zl303xx_Params,
                                      Uint32T val)
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
      status = ZL303XX_CHECK_DAMPING_STAGE1(val);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_S3E_FAST_LOCK_CTRL0_REG,
                              ZL303XX_S3E_DAMPING_STAGE1_MASK,
                              ZL303XX_S3E_DAMPING_STAGE1_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_S3eDampingStage1Set */


/*

  Function Name:
   zl303xx_S3eDampingStage2Get

  Details:
   Reads the Damping Stage 2 attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_S3eDampingStage2Get(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_S3E_FAST_LOCK_CTRL0_REG,
                              ZL303XX_S3E_DAMPING_STAGE2_MASK,
                              ZL303XX_S3E_DAMPING_STAGE2_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (Uint32T)(attrPar.value);
   }

   return status;
}  /* END zl303xx_S3eDampingStage2Get */

/*

  Function Name:
   zl303xx_S3eDampingStage2Set

  Details:
   Writes the Damping Stage 2 attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_S3eDampingStage2Set(zl303xx_ParamsS *zl303xx_Params,
                                      Uint32T val)
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
      status = ZL303XX_CHECK_DAMPING_STAGE2(val);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_S3E_FAST_LOCK_CTRL0_REG,
                              ZL303XX_S3E_DAMPING_STAGE2_MASK,
                              ZL303XX_S3E_DAMPING_STAGE2_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_S3eDampingStage2Set */


/*

  Function Name:
   zl303xx_S3eDampingStage3Get

  Details:
   Reads the Damping Stage 3 attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_S3eDampingStage3Get(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_S3E_FAST_LOCK_CTRL0_REG,
                              ZL303XX_S3E_DAMPING_STAGE3_MASK,
                              ZL303XX_S3E_DAMPING_STAGE3_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (Uint32T)(attrPar.value);
   }

   return status;
}  /* END zl303xx_S3eDampingStage3Get */

/*

  Function Name:
   zl303xx_S3eDampingStage3Set

  Details:
   Writes the Damping Stage 3 attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_S3eDampingStage3Set(zl303xx_ParamsS *zl303xx_Params,
                                      Uint32T val)
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
      status = ZL303XX_CHECK_DAMPING_STAGE3(val);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_S3E_FAST_LOCK_CTRL0_REG,
                              ZL303XX_S3E_DAMPING_STAGE3_MASK,
                              ZL303XX_S3E_DAMPING_STAGE3_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_S3eDampingStage3Set */


/*

  Function Name:
   zl303xx_S3eStage1TimeGet

  Details:
   Reads the Stage 1 Time attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_S3eStage1TimeGet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_S3E_FAST_LOCK_CTRL1_REG,
                              ZL303XX_S3E_STAGE1_TIME_MASK,
                              ZL303XX_S3E_STAGE1_TIME_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (Uint32T)(attrPar.value);
   }

   return status;
}  /* END zl303xx_S3eStage1TimeGet */

/*

  Function Name:
   zl303xx_S3eStage1TimeSet

  Details:
   Writes the Stage 1 Time attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_S3eStage1TimeSet(zl303xx_ParamsS *zl303xx_Params,
                                   Uint32T val)
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
      status = ZL303XX_CHECK_STAGE1_TIME(val);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_S3E_FAST_LOCK_CTRL1_REG,
                              ZL303XX_S3E_STAGE1_TIME_MASK,
                              ZL303XX_S3E_STAGE1_TIME_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_S3eStage1TimeSet */


/*

  Function Name:
   zl303xx_S3eStage1BwGet

  Details:
   Reads the Stage 1 Bandwidth attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_S3eStage1BwGet(zl303xx_ParamsS *zl303xx_Params, Uint32T *val)
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
                              ZL303XX_S3E_FAST_LOCK_CTRL1_REG,
                              ZL303XX_S3E_STAGE1_BW_MASK,
                              ZL303XX_S3E_STAGE1_BW_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (Uint32T)(attrPar.value);
   }

   return status;
}  /* END zl303xx_S3eStage1BwGet */

/*

  Function Name:
   zl303xx_S3eStage1BwSet

  Details:
   Writes the Stage 1 Bandwidth attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_S3eStage1BwSet(zl303xx_ParamsS *zl303xx_Params, Uint32T val)
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
      status = ZL303XX_CHECK_STAGE1_BW(val);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_S3E_FAST_LOCK_CTRL1_REG,
                              ZL303XX_S3E_STAGE1_BW_MASK,
                              ZL303XX_S3E_STAGE1_BW_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_S3eStage1BwSet */


/*

  Function Name:
   zl303xx_S3eStage2TimeGet

  Details:
   Reads the Stage 2 Time attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_S3eStage2TimeGet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_S3E_FAST_LOCK_CTRL2_REG,
                              ZL303XX_S3E_STAGE2_TIME_MASK,
                              ZL303XX_S3E_STAGE2_TIME_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (Uint32T)(attrPar.value);
   }

   return status;
}  /* END zl303xx_S3eStage2TimeGet */

/*

  Function Name:
   zl303xx_S3eStage2TimeSet

  Details:
   Writes the Stage 2 Time attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_S3eStage2TimeSet(zl303xx_ParamsS *zl303xx_Params,
                                   Uint32T val)
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
      status = ZL303XX_CHECK_STAGE2_TIME(val);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_S3E_FAST_LOCK_CTRL2_REG,
                              ZL303XX_S3E_STAGE2_TIME_MASK,
                              ZL303XX_S3E_STAGE2_TIME_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_S3eStage2TimeSet */


/*

  Function Name:
   zl303xx_S3eStage2BwGet

  Details:
   Reads the Stage 2 Bandwidth attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_S3eStage2BwGet(zl303xx_ParamsS *zl303xx_Params, Uint32T *val)
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
                              ZL303XX_S3E_FAST_LOCK_CTRL2_REG,
                              ZL303XX_S3E_STAGE2_BW_MASK,
                              ZL303XX_S3E_STAGE2_BW_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (Uint32T)(attrPar.value);
   }

   return status;
}  /* END zl303xx_S3eStage2BwGet */

/*

  Function Name:
   zl303xx_S3eStage2BwSet

  Details:
   Writes the Stage 2 Bandwidth attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_S3eStage2BwSet(zl303xx_ParamsS *zl303xx_Params, Uint32T val)
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
      status = ZL303XX_CHECK_STAGE2_BW(val);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_S3E_FAST_LOCK_CTRL2_REG,
                              ZL303XX_S3E_STAGE2_BW_MASK,
                              ZL303XX_S3E_STAGE2_BW_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_S3eStage2BwSet */


/*

  Function Name:
   zl303xx_S3eStage3TimeGet

  Details:
   Reads the Stage 3 Time attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_S3eStage3TimeGet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_S3E_FAST_LOCK_CTRL3_REG,
                              ZL303XX_S3E_STAGE3_TIME_MASK,
                              ZL303XX_S3E_STAGE3_TIME_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (Uint32T)(attrPar.value);
   }

   return status;
}  /* END zl303xx_S3eStage3TimeGet */

/*

  Function Name:
   zl303xx_S3eStage3TimeSet

  Details:
   Writes the Stage 3 Time attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_S3eStage3TimeSet(zl303xx_ParamsS *zl303xx_Params,
                                   Uint32T val)
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
      status = ZL303XX_CHECK_STAGE3_TIME(val);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_S3E_FAST_LOCK_CTRL3_REG,
                              ZL303XX_S3E_STAGE3_TIME_MASK,
                              ZL303XX_S3E_STAGE3_TIME_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_S3eStage3TimeSet */


/*

  Function Name:
   zl303xx_S3eStage3BwGet

  Details:
   Reads the Stage 3 Bandwidth attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_S3eStage3BwGet(zl303xx_ParamsS *zl303xx_Params, Uint32T *val)
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
                              ZL303XX_S3E_FAST_LOCK_CTRL3_REG,
                              ZL303XX_S3E_STAGE3_BW_MASK,
                              ZL303XX_S3E_STAGE3_BW_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (Uint32T)(attrPar.value);
   }

   return status;
}  /* END zl303xx_S3eStage3BwGet */

/*

  Function Name:
   zl303xx_S3eStage3BwSet

  Details:
   Writes the Stage 3 Bandwidth attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_S3eStage3BwSet(zl303xx_ParamsS *zl303xx_Params, Uint32T val)
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
      status = ZL303XX_CHECK_STAGE3_BW(val);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_S3E_FAST_LOCK_CTRL3_REG,
                              ZL303XX_S3E_STAGE3_BW_MASK,
                              ZL303XX_S3E_STAGE3_BW_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_S3eStage3BwSet */


/*

  Function Name:
   zl303xx_S3ePboEventGet

  Details:
   Reads the PBO Event attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_S3ePboEventGet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_S3E_S3E_ISR_REG,
                              ZL303XX_S3E_PBO_EVENT_MASK,
                              ZL303XX_S3E_PBO_EVENT_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_BooleanE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_S3ePboEventGet */


/*

  Function Name:
   zl303xx_S3ePboToutStatusGet

  Details:
   Reads the PBO Timeout Status attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_S3ePboToutStatusGet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_S3E_S3E_ISR_REG,
                              ZL303XX_S3E_PBO_TOUT_STATUS_MASK,
                              ZL303XX_S3E_PBO_TOUT_STATUS_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_BooleanE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_S3ePboToutStatusGet */


/*

  Function Name:
   zl303xx_S3ePboSampleGet

  Details:
   Reads the PBO Sampled attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_S3ePboSampleGet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_S3E_S3E_ISR_REG,
                              ZL303XX_S3E_PBO_SAMPLE_MASK,
                              ZL303XX_S3E_PBO_SAMPLE_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_BooleanE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_S3ePboSampleGet */


/*

  Function Name:
   zl303xx_S3ePboEventMaskGet

  Details:
   Reads the PBO Event Mask attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_S3ePboEventMaskGet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_S3E_S3E_MASK_REG,
                              ZL303XX_S3E_PBO_EVENT_MASK_MASK,
                              ZL303XX_S3E_PBO_EVENT_MASK_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_BooleanE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_S3ePboEventMaskGet */

/*

  Function Name:
   zl303xx_S3ePboEventMaskSet

  Details:
   Writes the PBO Event Mask attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_S3ePboEventMaskSet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_S3E_S3E_MASK_REG,
                              ZL303XX_S3E_PBO_EVENT_MASK_MASK,
                              ZL303XX_S3E_PBO_EVENT_MASK_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_S3ePboEventMaskSet */


/*

  Function Name:
   zl303xx_S3ePboTimeoutMaskGet

  Details:
   Reads the PBO Timeout Mask attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_S3ePboTimeoutMaskGet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_S3E_S3E_MASK_REG,
                              ZL303XX_S3E_PBO_TIMEOUT_MASK_MASK,
                              ZL303XX_S3E_PBO_TIMEOUT_MASK_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_BooleanE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_S3ePboTimeoutMaskGet */

/*

  Function Name:
   zl303xx_S3ePboTimeoutMaskSet

  Details:
   Writes the PBO Timeout Mask attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_S3ePboTimeoutMaskSet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_S3E_S3E_MASK_REG,
                              ZL303XX_S3E_PBO_TIMEOUT_MASK_MASK,
                              ZL303XX_S3E_PBO_TIMEOUT_MASK_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_S3ePboTimeoutMaskSet */


/*

  Function Name:
   zl303xx_S3ePboSampleMaskGet

  Details:
   Reads the PBO Sampled Mask attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_S3ePboSampleMaskGet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_S3E_S3E_MASK_REG,
                              ZL303XX_S3E_PBO_SAMPLE_MASK_MASK,
                              ZL303XX_S3E_PBO_SAMPLE_MASK_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_BooleanE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_S3ePboSampleMaskGet */

/*

  Function Name:
   zl303xx_S3ePboSampleMaskSet

  Details:
   Writes the PBO Sampled Mask attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_S3ePboSampleMaskSet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_S3E_S3E_MASK_REG,
                              ZL303XX_S3E_PBO_SAMPLE_MASK_MASK,
                              ZL303XX_S3E_PBO_SAMPLE_MASK_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_S3ePboSampleMaskSet */
