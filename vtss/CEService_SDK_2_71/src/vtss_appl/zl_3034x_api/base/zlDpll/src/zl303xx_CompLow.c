

/*******************************************************************************
*
*  $Id: zl303xx_CompLow.c 6058 2011-06-10 14:37:16Z AW $
*
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     Functions for low-level COMP attribute access
*
*******************************************************************************/

/*****************   INCLUDE FILES   ******************************************/

#include "zl303xx_ApiLow.h"
#include "zl303xx_CompLow.h"

/*****************   DEFINES   ************************************************/

/*****************   DATA TYPES   *********************************************/

/*****************   DATA STRUCTURES   ****************************************/

/*****************   EXPORTED GLOBAL VARIABLE DECLARATIONS   ******************/

/*

  Function Name:
   zl303xx_CompConfigStructInit

  Details:
   Initializes the members of the zl303xx_CompConfigS data structure

  Parameters:
   [in]   zl303xx_Params Pointer to the device instance parameter structure
   [in]   par            Pointer to the zl303xx_CompConfigS structure to Initialize

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_CompConfigStructInit(zl303xx_ParamsS *zl303xx_Params,
                                       zl303xx_CompConfigS *par)
{
   zlStatusE status = ZL303XX_OK;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, par);
   }

   if (status == ZL303XX_OK)
   {
      par->detectMode = (zl303xx_CcDetModeE)ZL303XX_INVALID;
      par->fpMode = (zl303xx_CcFpModeE)ZL303XX_INVALID;
      par->fpSelect = (zl303xx_CcFpSelectE)ZL303XX_INVALID;
      par->ccIsrEn = (zl303xx_BooleanE)ZL303XX_INVALID;
      par->Id = (zl303xx_CompIdE)ZL303XX_INVALID;
   }

   return status;
} /* END zl303xx_CompConfigStructInit */

/*

  Function Name:
   zl303xx_CompConfigCheck

  Details:
   Checks the members of the zl303xx_CompConfigS data structure

  Parameters:
   [in]   zl303xx_Params Pointer to the device instance parameter structure
   [in]   par            Pointer to the zl303xx_CompConfigS structure to Check

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_CompConfigCheck(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_CompConfigS *par)
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
      status = ZL303XX_CHECK_CC_DET_MODE(par->detectMode) |
               ZL303XX_CHECK_CC_FP_MODE(par->fpMode) |
               ZL303XX_CHECK_CC_FP_SELECT(par->fpSelect) |
               ZL303XX_CHECK_BOOLEAN(par->ccIsrEn) |
               ZL303XX_CHECK_COMP_ID(par->Id);
   }

   return status;
} /* END zl303xx_CompConfigCheck */

/**

  Function Name:
   zl303xx_CompConfigGet

  Details:
   Gets the current configuration of the specified Composite Clock using the
   members of the zl303xx_CompConfigS data structure

  Parameters:
   [in]   zl303xx_Params Pointer to the device instance parameter structure
   [in]   par            Pointer to the zl303xx_CompConfigS structure to Get

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_CompConfigGet(zl303xx_ParamsS *zl303xx_Params,
                                zl303xx_CompConfigS *par)
{
   zlStatusE status = ZL303XX_OK;
   Uint32T regValue = 0;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, par);
   }

   /* Check for a valid CompId */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_COMP_ID(par->Id);
   }

   /* Read */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_Read(zl303xx_Params, NULL,
                            ZL303XX_COMP_CONTROL_REG(par->Id),
                            &regValue);
   }

   /* Extract */
   if (status == ZL303XX_OK)
   {
      par->detectMode = (zl303xx_CcDetModeE)ZL303XX_EXTRACT(regValue,
                                            ZL303XX_COMP_DET_MODE_MASK,
                                            ZL303XX_COMP_DET_MODE_SHIFT);

      par->fpMode = (zl303xx_CcFpModeE)ZL303XX_EXTRACT(regValue,
                                        ZL303XX_COMP_FP_MODE_MASK,
                                        ZL303XX_COMP_FP_MODE_SHIFT);

      par->fpSelect = (zl303xx_CcFpSelectE)ZL303XX_EXTRACT(regValue,
                                            ZL303XX_COMP_FP_SELECT_MASK,
                                            ZL303XX_COMP_FP_SELECT_SHIFT);
   }

   /* Read */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_Read(zl303xx_Params, NULL,
                            ZL303XX_COMP_MASK_REG,
                            &regValue);
   }

   /* Extract */
   if (status == ZL303XX_OK)
   {
      par->ccIsrEn = (zl303xx_BooleanE)ZL303XX_EXTRACT(regValue,
                                       ZL303XX_COMP_BPV_MASK_EN_MASK,
                                       ZL303XX_COMP_BPV_MASK_EN_SHIFT(par->Id));
   }

   return status;
} /* END zl303xx_CompConfigGet */

/**

  Function Name:
   zl303xx_CompConfigSet

  Details:
   Sets the configuration of the specified Composite Clock using the members
   of the zl303xx_CompConfigS data structure.

  Parameters:
   [in]   zl303xx_Params Pointer to the device instance parameter structure
   [in]   par            Pointer to the zl303xx_CompConfigS structure to Set

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_CompConfigSet(zl303xx_ParamsS *zl303xx_Params,
                                zl303xx_CompConfigS *par)
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
      status = zl303xx_CompConfigCheck(zl303xx_Params, par);
   }

   /* Package */
   if (status == ZL303XX_OK)
   {
      regValue = 0;
      mask = 0;

      /* detectMode */
      ZL303XX_INSERT(regValue, par->detectMode,
                             ZL303XX_COMP_DET_MODE_MASK,
                             ZL303XX_COMP_DET_MODE_SHIFT);

      /* fpMode */
      ZL303XX_INSERT(regValue, par->fpMode,
                           ZL303XX_COMP_FP_MODE_MASK,
                           ZL303XX_COMP_FP_MODE_SHIFT);

      /* fpSelect */
      ZL303XX_INSERT(regValue, par->fpSelect,
                           ZL303XX_COMP_FP_SELECT_MASK,
                           ZL303XX_COMP_FP_SELECT_SHIFT);

      mask |= (ZL303XX_COMP_DET_MODE_MASK << ZL303XX_COMP_DET_MODE_SHIFT) |
              (ZL303XX_COMP_FP_MODE_MASK << ZL303XX_COMP_FP_MODE_SHIFT) |
              (ZL303XX_COMP_FP_SELECT_MASK << ZL303XX_COMP_FP_SELECT_SHIFT);

      /* Write the Data for this Register */
      status = zl303xx_ReadModWrite(zl303xx_Params, NULL,
                                    ZL303XX_COMP_CONTROL_REG(par->Id),
                                    regValue, mask, NULL);
   }

   /* Package */
   if (status == ZL303XX_OK)
   {
      regValue = 0;
      mask = 0;

      /* ccIsrEn */
      ZL303XX_INSERT(regValue, par->ccIsrEn,
                             ZL303XX_COMP_BPV_MASK_EN_MASK,
                             ZL303XX_COMP_BPV_MASK_EN_SHIFT(par->Id));

      mask |= (ZL303XX_COMP_BPV_MASK_EN_MASK << ZL303XX_COMP_BPV_MASK_EN_SHIFT(par->Id));

      /* Write the Data for this Register */
      status = zl303xx_ReadModWrite(zl303xx_Params, NULL,
                                    ZL303XX_COMP_MASK_REG,
                                    regValue, mask, NULL);
   }

   return status;
} /* END zl303xx_CompConfigSet */

/*

  Function Name:
   zl303xx_CompStatusStructInit

  Details:
   Initializes the members of the zl303xx_CompStatusS data structure

  Parameters:
   [in]   zl303xx_Params Pointer to the device instance parameter structure
   [in]   par            Pointer to the zl303xx_CompStatusS structure to Initialize

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_CompStatusStructInit(zl303xx_ParamsS *zl303xx_Params,
                                       zl303xx_CompStatusS *par)
{
   zlStatusE status = ZL303XX_OK;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, par);
   }

   if (status == ZL303XX_OK)
   {
      par->fp64kHzDet = (zl303xx_BooleanE)ZL303XX_INVALID;
      par->fp8kHzDet = (zl303xx_BooleanE)ZL303XX_INVALID;
      par->fp400HzDet = (zl303xx_BooleanE)ZL303XX_INVALID;
      par->bpvStatus = (zl303xx_BpvStatusE)ZL303XX_INVALID;
      par->Id = (zl303xx_CompIdE)ZL303XX_INVALID;
   }

   return status;
} /* END zl303xx_CompStatusStructInit */

/*

  Function Name:
   zl303xx_CompStatusCheck

  Details:
   Checks the members of the zl303xx_CompStatusS data structure

  Parameters:
   [in]   zl303xx_Params Pointer to the device instance parameter structure
   [in]   par            Pointer to the zl303xx_CompStatusS structure to Check

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_CompStatusCheck(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_CompStatusS *par)
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
      status = ZL303XX_CHECK_BOOLEAN(par->fp64kHzDet) |
               ZL303XX_CHECK_BOOLEAN(par->fp8kHzDet) |
               ZL303XX_CHECK_BOOLEAN(par->fp400HzDet) |
               ZL303XX_CHECK_BPV_STATUS(par->bpvStatus) |
               ZL303XX_CHECK_COMP_ID(par->Id);
   }

   return status;
} /* END zl303xx_CompStatusCheck */

/*

  Function Name:
   zl303xx_CompStatusGet

  Details:
   Gets the members of the zl303xx_CompStatusS data structure

  Parameters:
   [in]   zl303xx_Params Pointer to the device instance parameter structure
   [in]   par            Pointer to the zl303xx_CompStatusS structure to Get

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_CompStatusGet(zl303xx_ParamsS *zl303xx_Params,
                                zl303xx_CompStatusS *par)
{
   zlStatusE status = ZL303XX_OK;
   Uint32T regValue = 0;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, par);
   }

   /* Check for a valid CompId */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_COMP_ID(par->Id);
   }

   /* Read */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_Read(zl303xx_Params, NULL,
                            ZL303XX_COMP_STATUS_REG,
                            &regValue);
   }

   /* Extract */
   if (status == ZL303XX_OK)
   {
      par->fp64kHzDet = (zl303xx_BooleanE)ZL303XX_EXTRACT(regValue,
                                          ZL303XX_COMP_64KHZ_DETECT_MASK,
                                          ZL303XX_COMP_64KHZ_DETECT_SHIFT(par->Id));

      par->fp8kHzDet = (zl303xx_BooleanE)ZL303XX_EXTRACT(regValue,
                                          ZL303XX_COMP_FP8KHZ_DETECT_MASK,
                                          ZL303XX_COMP_FP8KHZ_DETECT_SHIFT(par->Id));

      par->fp400HzDet = (zl303xx_BooleanE)ZL303XX_EXTRACT(regValue,
                                           ZL303XX_COMP_FP400HZ_DETECT_MASK,
                                           ZL303XX_COMP_FP400HZ_DETECT_SHIFT(par->Id));
   }

   /* Read */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_Read(zl303xx_Params, NULL,
                            ZL303XX_COMP_ISR_REG,
                            &regValue);
   }

   /* Extract */
   if (status == ZL303XX_OK)
   {
      par->bpvStatus = (zl303xx_BpvStatusE)ZL303XX_EXTRACT(regValue,
                                           ZL303XX_COMP_BPV_ERROR_MASK,
                                           ZL303XX_COMP_BPV_ERROR_SHIFT(par->Id));
   }

   return status;
} /* END zl303xx_CompStatusGet */

/*

  Function Name:
   zl303xx_CompDetModeGet

  Details:
   Reads the Detect Mode attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    compId         Associated Comp Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_CompDetModeGet(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_CompIdE compId,
                                 zl303xx_BooleanE *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
   }

   /* Check the compId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_COMP_ID(compId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_COMP_CONTROL_REG(compId),
                              ZL303XX_COMP_DET_MODE_MASK,
                              ZL303XX_COMP_DET_MODE_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_BooleanE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_CompDetModeGet */

/*

  Function Name:
   zl303xx_CompDetModeSet

  Details:
   Writes the Detect Mode attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    compId         Associated Comp Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_CompDetModeSet(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_CompIdE compId,
                                 zl303xx_BooleanE val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check params pointer */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
   }

   /* Check the compId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_COMP_ID(compId);
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
                              ZL303XX_COMP_CONTROL_REG(compId),
                              ZL303XX_COMP_DET_MODE_MASK,
                              ZL303XX_COMP_DET_MODE_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_CompDetModeSet */


/*

  Function Name:
   zl303xx_CompFpModeGet

  Details:
   Reads the Fp Mode attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    compId         Associated Comp Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_CompFpModeGet(zl303xx_ParamsS *zl303xx_Params,
                                zl303xx_CompIdE compId,
                                zl303xx_CcFpModeE *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
   }

   /* Check the compId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_COMP_ID(compId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_COMP_CONTROL_REG(compId),
                              ZL303XX_COMP_FP_MODE_MASK,
                              ZL303XX_COMP_FP_MODE_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_CcFpModeE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_CompFpModeGet */

/*

  Function Name:
   zl303xx_CompFpModeSet

  Details:
   Writes the Fp Mode attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    compId         Associated Comp Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_CompFpModeSet(zl303xx_ParamsS *zl303xx_Params,
                                zl303xx_CompIdE compId,
                                zl303xx_CcFpModeE val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check params pointer */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
   }

   /* Check the compId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_COMP_ID(compId);
   }

   /* Check that the write value is within range */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_CC_FP_MODE(val);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_COMP_CONTROL_REG(compId),
                              ZL303XX_COMP_FP_MODE_MASK,
                              ZL303XX_COMP_FP_MODE_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_CompFpModeSet */


/*

  Function Name:
   zl303xx_CompFpSelectGet

  Details:
   Reads the Fp Select attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    compId         Associated Comp Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_CompFpSelectGet(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_CompIdE compId,
                                  zl303xx_CcFpSelectE *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
   }

   /* Check the compId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_COMP_ID(compId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_COMP_CONTROL_REG(compId),
                              ZL303XX_COMP_FP_SELECT_MASK,
                              ZL303XX_COMP_FP_SELECT_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_CcFpSelectE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_CompFpSelectGet */

/*

  Function Name:
   zl303xx_CompFpSelectSet

  Details:
   Writes the Fp Select attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    compId         Associated Comp Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_CompFpSelectSet(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_CompIdE compId,
                                  zl303xx_CcFpSelectE val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check params pointer */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
   }

   /* Check the compId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_COMP_ID(compId);
   }

   /* Check that the write value is within range */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_CC_FP_SELECT(val);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_COMP_CONTROL_REG(compId),
                              ZL303XX_COMP_FP_SELECT_MASK,
                              ZL303XX_COMP_FP_SELECT_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_CompFpSelectSet */


/*

  Function Name:
   zl303xx_Comp64khzDetectGet

  Details:
   Reads the 64kHz Status attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    compId         Associated Comp Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_Comp64khzDetectGet(zl303xx_ParamsS *zl303xx_Params,
                                     zl303xx_CompIdE compId,
                                     zl303xx_BooleanE *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
   }

   /* Check the compId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_COMP_ID(compId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_COMP_STATUS_REG,
                              ZL303XX_COMP_64KHZ_DETECT_MASK,
                              ZL303XX_COMP_64KHZ_DETECT_SHIFT(compId));
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_BooleanE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_Comp64khzDetectGet */


/*

  Function Name:
   zl303xx_CompFp8khzDetectGet

  Details:
   Reads the 8kHz Status attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    compId         Associated Comp Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_CompFp8khzDetectGet(zl303xx_ParamsS *zl303xx_Params,
                                      zl303xx_CompIdE compId,
                                      zl303xx_BooleanE *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
   }

   /* Check the compId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_COMP_ID(compId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_COMP_STATUS_REG,
                              ZL303XX_COMP_FP8KHZ_DETECT_MASK,
                              ZL303XX_COMP_FP8KHZ_DETECT_SHIFT(compId));
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_BooleanE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_CompFp8khzDetectGet */


/*

  Function Name:
   zl303xx_CompFp400hzDetectGet

  Details:
   Reads the 400Hz Status attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    compId         Associated Comp Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_CompFp400hzDetectGet(zl303xx_ParamsS *zl303xx_Params,
                                       zl303xx_CompIdE compId,
                                       zl303xx_BooleanE *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
   }

   /* Check the compId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_COMP_ID(compId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_COMP_STATUS_REG,
                              ZL303XX_COMP_FP400HZ_DETECT_MASK,
                              ZL303XX_COMP_FP400HZ_DETECT_SHIFT(compId));
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_BooleanE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_CompFp400hzDetectGet */


/*

  Function Name:
   zl303xx_CompBpvErrorGet

  Details:
   Reads the BPV Error attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    compId         Associated Comp Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_CompBpvErrorGet(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_CompIdE compId,
                                  zl303xx_BpvStatusE *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
   }

   /* Check the compId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_COMP_ID(compId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_COMP_ISR_REG,
                              ZL303XX_COMP_BPV_ERROR_MASK,
                              ZL303XX_COMP_BPV_ERROR_SHIFT(compId));
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_BpvStatusE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_CompBpvErrorGet */


/*

  Function Name:
   zl303xx_CompBpvMaskEnGet

  Details:
   Reads the BPV Mask attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    compId         Associated Comp Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_CompBpvMaskEnGet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_CompIdE compId,
                                   zl303xx_BooleanE *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
   }

   /* Check the compId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_COMP_ID(compId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_COMP_MASK_REG,
                              ZL303XX_COMP_BPV_MASK_EN_MASK,
                              ZL303XX_COMP_BPV_MASK_EN_SHIFT(compId));
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_BooleanE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_CompBpvMaskEnGet */

/*

  Function Name:
   zl303xx_CompBpvMaskEnSet

  Details:
   Writes the BPV Mask attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    compId         Associated Comp Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_CompBpvMaskEnSet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_CompIdE compId,
                                   zl303xx_BooleanE val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check params pointer */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
   }

   /* Check the compId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_COMP_ID(compId);
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
                              ZL303XX_COMP_MASK_REG,
                              ZL303XX_COMP_BPV_MASK_EN_MASK,
                              ZL303XX_COMP_BPV_MASK_EN_SHIFT(compId));
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_CompBpvMaskEnSet */


