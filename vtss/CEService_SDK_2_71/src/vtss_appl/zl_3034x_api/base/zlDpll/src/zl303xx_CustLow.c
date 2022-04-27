
/*******************************************************************************
*
*  $Id: zl303xx_CustLow.c 6058 2011-06-10 14:37:16Z AW $
*
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     Functions for low-level CUST attribute access
*
******************************************************************************/

/*****************   INCLUDE FILES   ******************************************/

#include "zl303xx_ApiLow.h"
#include "zl303xx_CustLow.h"
#include "zl303xx_PllFuncs.h"

/*****************   DEFINES   ************************************************/

/*****************   DATA TYPES   *********************************************/

/*****************   DATA STRUCTURES   ****************************************/

/*****************   EXPORTED GLOBAL VARIABLE DECLARATIONS   ******************/

/*

  Function Name:
   zl303xx_CustConfigStructInit

  Details:
   Initializes the members of the zl303xx_CustConfigS data structure

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    par            Pointer to the zl303xx_CustConfigS structure to Initialize

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_CustConfigStructInit(zl303xx_ParamsS *zl303xx_Params,
                                       zl303xx_CustConfigS *par)
{
   zlStatusE status = ZL303XX_OK;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, par);
   }

   if (status == ZL303XX_OK)
   {
      par->freq = (Uint32T) ZL303XX_INVALID;
      par->scmLo = (Uint32T) ZL303XX_INVALID;
      par->scmHi = (Uint32T) ZL303XX_INVALID;
      par->cfmLo = (Uint32T) ZL303XX_INVALID;
      par->cfmHi = (Uint32T) ZL303XX_INVALID;
      par->cycles = (Uint32T) ZL303XX_INVALID;
      par->cfmDivBy4 = (zl303xx_BooleanE)ZL303XX_INVALID;
      par->Id = (zl303xx_CustIdE)ZL303XX_INVALID;
   }

   return status;
} /* END zl303xx_CustConfigStructInit */

/*

  Function Name:
   zl303xx_CustConfigCheck

  Details:
   Checks the members of the zl303xx_CustConfigS data structure

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    par            Pointer to the zl303xx_CustConfigS structure to Check

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_CustConfigCheck(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_CustConfigS *par)
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
      status = zl303xx_CheckCustFreq(par->freq) |
               zl303xx_CheckScmLimit(par->scmLo) |
               zl303xx_CheckScmLimit(par->scmHi) |
               zl303xx_CheckCfmLimit(par->cfmLo) |
               zl303xx_CheckCfmLimit(par->cfmHi) |
               zl303xx_CheckCfmCycles(par->cycles) |
               ZL303XX_CHECK_BOOLEAN(par->cfmDivBy4) |
               ZL303XX_CHECK_CUST_ID(par->Id);
   }

   return status;
} /* END zl303xx_CustConfigCheck */

/*

  Function Name:
   zl303xx_CustConfigGet

  Details:
   Gets the members of the zl303xx_CustConfigS data structure

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    par            Pointer to the zl303xx_CustConfigS structure to Get

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_CustConfigGet(zl303xx_ParamsS *zl303xx_Params,
                                zl303xx_CustConfigS *par)
{
   zlStatusE status = ZL303XX_OK;
   Uint32T regValue = 0;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, par);
   }

   /* Check for a valid CustId */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_CUST_ID(par->Id);
   }

   /* Read */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_Read(zl303xx_Params, NULL,
                            ZL303XX_CUST_8K_MULT_REG(par->Id),
                            &regValue);
   }

   /* Extract */
   if (status == ZL303XX_OK)
   {
      par->freq = zl303xx_CustFreqInKHz(regValue);
   }

   /* Read */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_Read(zl303xx_Params, NULL,
                            ZL303XX_CUST_SCM_LO_LIMIT_REG(par->Id),
                            &regValue);
   }

   /* Extract */
   if (status == ZL303XX_OK)
   {
      par->scmLo = zl303xx_ScmLimitInNs(regValue);
   }

   /* Read */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_Read(zl303xx_Params, NULL,
                            ZL303XX_CUST_SCM_HI_LIMIT_REG(par->Id),
                            &regValue);
   }

   /* Extract */
   if (status == ZL303XX_OK)
   {
      par->scmHi = zl303xx_ScmLimitInNs(regValue);
   }

   /* Read */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_Read(zl303xx_Params, NULL,
                            ZL303XX_CUST_CFM_LO_LIMIT_REG(par->Id),
                            &regValue);
   }

   /* Extract */
   if (status == ZL303XX_OK)
   {
      par->cfmLo = zl303xx_CfmLimitInNs(regValue);
   }

   /* Read */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_Read(zl303xx_Params, NULL,
                            ZL303XX_CUST_CFM_HI_LIMIT_REG(par->Id),
                            &regValue);
   }

   /* Extract */
   if (status == ZL303XX_OK)
   {
      par->cfmHi = zl303xx_CfmLimitInNs(regValue);
   }

   /* Read */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_Read(zl303xx_Params, NULL,
                            ZL303XX_CUST_CFM_CYCLE_REG(par->Id),
                            &regValue);
   }

   /* Extract */
   if (status == ZL303XX_OK)
   {
      par->cycles = zl303xx_CfmCycles(regValue);
   }

   /* Read */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_Read(zl303xx_Params, NULL,
                            ZL303XX_CUST_DIVIDE_REG(par->Id),
                            &regValue);
   }

   /* Extract */
   if (status == ZL303XX_OK)
   {
      par->cfmDivBy4 = (zl303xx_BooleanE)ZL303XX_EXTRACT(regValue,
                                         ZL303XX_CUST_DIVIDE_MASK,
                                         ZL303XX_CUST_DIVIDE_SHIFT);
   }

   return status;
} /* END zl303xx_CustConfigGet */

/*

  Function Name:
   zl303xx_CustConfigSet

  Details:
   Sets the members of the zl303xx_CustConfigS data structure

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    par            Pointer to the zl303xx_CustConfigS structure to Set

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_CustConfigSet(zl303xx_ParamsS *zl303xx_Params,
                                zl303xx_CustConfigS *par)
{
   zlStatusE status = ZL303XX_OK;
   Uint32T regValue, bfValue, mask;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, par);
   }

   /* Check the par values to be written */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_CustConfigCheck(zl303xx_Params, par);
   }

   /* Package */
   if (status == ZL303XX_OK)
   {
      regValue = 0;
      mask = 0;

      /* freq */
      status = zl303xx_ClosestCustFreqReg(par->freq, &bfValue);

      if (status == ZL303XX_OK)
      {
         /* Immediately read back the register value and return it */
         par->freq = zl303xx_CustFreqInKHz(bfValue);

         ZL303XX_INSERT(regValue, bfValue,
                                ZL303XX_CUST_FREQUENCY_MASK,
                                ZL303XX_CUST_FREQUENCY_SHIFT);

         mask |= (ZL303XX_CUST_FREQUENCY_MASK << ZL303XX_CUST_FREQUENCY_SHIFT);

         /* Write the Data for this Register */
         status = zl303xx_ReadModWrite(zl303xx_Params, NULL,
                                       ZL303XX_CUST_8K_MULT_REG(par->Id),
                                       regValue, mask, NULL);
      }
   }

   /* Package */
   if (status == ZL303XX_OK)
   {
      regValue = 0;
      mask = 0;

      /* scmLo */
      status = zl303xx_ClosestScmLimitReg(par->scmLo, &bfValue);

      if (status == ZL303XX_OK)
      {
         ZL303XX_INSERT(regValue, bfValue,
                                ZL303XX_CUST_SCM_LO_LIMIT_MASK,
                                ZL303XX_CUST_SCM_LO_LIMIT_SHIFT);

         mask |= (ZL303XX_CUST_SCM_LO_LIMIT_MASK << ZL303XX_CUST_SCM_LO_LIMIT_SHIFT);

         /* Write the Data for this Register */
         status = zl303xx_ReadModWrite(zl303xx_Params, NULL,
                                       ZL303XX_CUST_SCM_LO_LIMIT_REG(par->Id),
                                       regValue, mask, NULL);
      }
   }

   /* Package */
   if (status == ZL303XX_OK)
   {
      regValue = 0;
      mask = 0;

      /* scmHi */
      status = zl303xx_ClosestScmLimitReg(par->scmHi, &bfValue);

      if (status == ZL303XX_OK)
      {
         ZL303XX_INSERT(regValue, bfValue,
                                ZL303XX_CUST_SCM_HI_LIMIT_MASK,
                                ZL303XX_CUST_SCM_HI_LIMIT_SHIFT);

         mask |= (ZL303XX_CUST_SCM_HI_LIMIT_MASK << ZL303XX_CUST_SCM_HI_LIMIT_SHIFT);

         /* Write the Data for this Register */
         status = zl303xx_ReadModWrite(zl303xx_Params, NULL,
                                       ZL303XX_CUST_SCM_HI_LIMIT_REG(par->Id),
                                       regValue, mask, NULL);
      }
   }

   /* Package */
   if (status == ZL303XX_OK)
   {
      regValue = 0;
      mask = 0;

      /* cfmLo */
      status = zl303xx_ClosestCfmLimitReg(par->cfmLo, &bfValue);

      if (status == ZL303XX_OK)
      {
         ZL303XX_INSERT(regValue, bfValue,
                                ZL303XX_CUST_CFM_LO_LIMIT_MASK,
                                ZL303XX_CUST_CFM_LO_LIMIT_SHIFT);

         mask |= (ZL303XX_CUST_CFM_LO_LIMIT_MASK << ZL303XX_CUST_CFM_LO_LIMIT_SHIFT);

         /* Write the Data for this Register */
         status = zl303xx_ReadModWrite(zl303xx_Params, NULL,
                                       ZL303XX_CUST_CFM_LO_LIMIT_REG(par->Id),
                                       regValue, mask, NULL);
      }
   }

   /* Package */
   if (status == ZL303XX_OK)
   {
      regValue = 0;
      mask = 0;

      /* cfmHi */
      status = zl303xx_ClosestCfmLimitReg(par->cfmHi, &bfValue);

      if (status == ZL303XX_OK)
      {
         ZL303XX_INSERT(regValue, bfValue,
                                ZL303XX_CUST_CFM_HI_LIMIT_MASK,
                                ZL303XX_CUST_CFM_HI_LIMIT_SHIFT);

         mask |= (ZL303XX_CUST_CFM_HI_LIMIT_MASK << ZL303XX_CUST_CFM_HI_LIMIT_SHIFT);

         /* Write the Data for this Register */
         status = zl303xx_ReadModWrite(zl303xx_Params, NULL,
                                       ZL303XX_CUST_CFM_HI_LIMIT_REG(par->Id),
                                       regValue, mask, NULL);
      }
   }

   /* Package */
   if (status == ZL303XX_OK)
   {
      regValue = 0;
      mask = 0;

      /* cycles */
      status = zl303xx_CfmCyclesReg(par->cycles, &bfValue);

      if (status == ZL303XX_OK)
      {
         ZL303XX_INSERT(regValue, bfValue,
                                ZL303XX_CUST_CFM_CYCLES_MASK,
                                ZL303XX_CUST_CFM_CYCLES_SHIFT);

         mask |= (ZL303XX_CUST_CFM_CYCLES_MASK << ZL303XX_CUST_CFM_CYCLES_SHIFT);

         /* Write the Data for this Register */
         status = zl303xx_ReadModWrite(zl303xx_Params, NULL,
                                       ZL303XX_CUST_CFM_CYCLE_REG(par->Id),
                                       regValue, mask, NULL);
      }
   }

   /* Package */
   if (status == ZL303XX_OK)
   {
      regValue = 0;
      mask = 0;

      /* cfmDivBy4 */
      ZL303XX_INSERT(regValue, par->cfmDivBy4,
                            ZL303XX_CUST_DIVIDE_MASK,
                            ZL303XX_CUST_DIVIDE_SHIFT);

      mask |= (ZL303XX_CUST_DIVIDE_MASK << ZL303XX_CUST_DIVIDE_SHIFT);

      /* Write the Data for this Register */
      status = zl303xx_ReadModWrite(zl303xx_Params, NULL,
                                    ZL303XX_CUST_DIVIDE_REG(par->Id),
                                    regValue, mask, NULL);
   }

   return status;
} /* END zl303xx_CustConfigSet */

/*

  Function Name:
   zl303xx_CustFrequencyGet

  Details:
   Reads the Nx8kHz Mult attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    custId         Associated Cust Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_CustFrequencyGet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_CustIdE custId, Uint32T *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
   }

   /* Check the custId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_CUST_ID(custId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_CUST_8K_MULT_REG(custId),
                              ZL303XX_CUST_FREQUENCY_MASK,
                              ZL303XX_CUST_FREQUENCY_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (Uint32T)(attrPar.value);
   }

   return status;
}  /* END zl303xx_CustFrequencyGet */

/*

  Function Name:
   zl303xx_CustFrequencySet

  Details:
   Writes the Nx8kHz Mult attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    custId         Associated Cust Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_CustFrequencySet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_CustIdE custId, Uint32T val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check params pointer */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
   }

   /* Check the custId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_CUST_ID(custId);
   }

   /* Check that the write value is within range */
   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_CUST_8K_MULT_REG(custId),
                              ZL303XX_CUST_FREQUENCY_MASK,
                              ZL303XX_CUST_FREQUENCY_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_CustFrequencySet */


/*

  Function Name:
   zl303xx_CustScmLoLimitGet

  Details:
   Reads the SCM Low Limit attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    custId         Associated Cust Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_CustScmLoLimitGet(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_CustIdE custId,
                                    Uint32T *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
   }

   /* Check the custId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_CUST_ID(custId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_CUST_SCM_LO_LIMIT_REG(custId),
                              ZL303XX_CUST_SCM_LO_LIMIT_MASK,
                              ZL303XX_CUST_SCM_LO_LIMIT_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (Uint32T)(attrPar.value);
   }

   return status;
}  /* END zl303xx_CustScmLoLimitGet */

/*

  Function Name:
   zl303xx_CustScmLoLimitSet

  Details:
   Writes the SCM Low Limit attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    custId         Associated Cust Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_CustScmLoLimitSet(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_CustIdE custId, Uint32T val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check params pointer */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
   }

   /* Check the custId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_CUST_ID(custId);
   }

   /* Check that the write value is within range */
   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_CUST_SCM_LO_LIMIT_REG(custId),
                              ZL303XX_CUST_SCM_LO_LIMIT_MASK,
                              ZL303XX_CUST_SCM_LO_LIMIT_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_CustScmLoLimitSet */


/*

  Function Name:
   zl303xx_CustScmHiLimitGet

  Details:
   Reads the SCM High Limit attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    custId         Associated Cust Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_CustScmHiLimitGet(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_CustIdE custId,
                                    Uint32T *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
   }

   /* Check the custId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_CUST_ID(custId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_CUST_SCM_HI_LIMIT_REG(custId),
                              ZL303XX_CUST_SCM_HI_LIMIT_MASK,
                              ZL303XX_CUST_SCM_HI_LIMIT_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (Uint32T)(attrPar.value);
   }

   return status;
}  /* END zl303xx_CustScmHiLimitGet */

/*

  Function Name:
   zl303xx_CustScmHiLimitSet

  Details:
   Writes the SCM High Limit attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    custId         Associated Cust Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_CustScmHiLimitSet(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_CustIdE custId, Uint32T val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check params pointer */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
   }

   /* Check the custId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_CUST_ID(custId);
   }

   /* Check that the write value is within range */
   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_CUST_SCM_HI_LIMIT_REG(custId),
                              ZL303XX_CUST_SCM_HI_LIMIT_MASK,
                              ZL303XX_CUST_SCM_HI_LIMIT_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_CustScmHiLimitSet */


/*

  Function Name:
   zl303xx_CustCfmLoLimitGet

  Details:
   Reads the CFM Low Limit attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    custId         Associated Cust Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_CustCfmLoLimitGet(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_CustIdE custId,
                                    Uint32T *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
   }

   /* Check the custId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_CUST_ID(custId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_CUST_CFM_LO_LIMIT_REG(custId),
                              ZL303XX_CUST_CFM_LO_LIMIT_MASK,
                              ZL303XX_CUST_CFM_LO_LIMIT_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (Uint32T)(attrPar.value);
   }

   return status;
}  /* END zl303xx_CustCfmLoLimitGet */

/*

  Function Name:
   zl303xx_CustCfmLoLimitSet

  Details:
   Writes the CFM Low Limit attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    custId         Associated Cust Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_CustCfmLoLimitSet(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_CustIdE custId, Uint32T val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check params pointer */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
   }

   /* Check the custId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_CUST_ID(custId);
   }

   /* Check that the write value is within range */
   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_CUST_CFM_LO_LIMIT_REG(custId),
                              ZL303XX_CUST_CFM_LO_LIMIT_MASK,
                              ZL303XX_CUST_CFM_LO_LIMIT_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_CustCfmLoLimitSet */


/*

  Function Name:
   zl303xx_CustCfmHiLimitGet

  Details:
   Reads the CFM High Limit attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    custId         Associated Cust Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_CustCfmHiLimitGet(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_CustIdE custId,
                                    Uint32T *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
   }

   /* Check the custId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_CUST_ID(custId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_CUST_CFM_HI_LIMIT_REG(custId),
                              ZL303XX_CUST_CFM_HI_LIMIT_MASK,
                              ZL303XX_CUST_CFM_HI_LIMIT_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (Uint32T)(attrPar.value);
   }

   return status;
}  /* END zl303xx_CustCfmHiLimitGet */

/*

  Function Name:
   zl303xx_CustCfmHiLimitSet

  Details:
   Writes the CFM High Limit attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    custId         Associated Cust Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_CustCfmHiLimitSet(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_CustIdE custId, Uint32T val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check params pointer */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
   }

   /* Check the custId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_CUST_ID(custId);
   }

   /* Check that the write value is within range */
   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_CUST_CFM_HI_LIMIT_REG(custId),
                              ZL303XX_CUST_CFM_HI_LIMIT_MASK,
                              ZL303XX_CUST_CFM_HI_LIMIT_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_CustCfmHiLimitSet */


/*

  Function Name:
   zl303xx_CustCfmCyclesGet

  Details:
   Reads the CFM Cycle attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    custId         Associated Cust Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_CustCfmCyclesGet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_CustIdE custId, Uint32T *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
   }

   /* Check the custId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_CUST_ID(custId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_CUST_CFM_CYCLE_REG(custId),
                              ZL303XX_CUST_CFM_CYCLES_MASK,
                              ZL303XX_CUST_CFM_CYCLES_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (Uint32T)(attrPar.value);
   }

   return status;
}  /* END zl303xx_CustCfmCyclesGet */

/*

  Function Name:
   zl303xx_CustCfmCyclesSet

  Details:
   Writes the CFM Cycle attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    custId         Associated Cust Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_CustCfmCyclesSet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_CustIdE custId, Uint32T val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check params pointer */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
   }

   /* Check the custId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_CUST_ID(custId);
   }

   /* Check that the write value is within range */
   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_CUST_CFM_CYCLE_REG(custId),
                              ZL303XX_CUST_CFM_CYCLES_MASK,
                              ZL303XX_CUST_CFM_CYCLES_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_CustCfmCyclesSet */


/*

  Function Name:
   zl303xx_CustDivideGet

  Details:
   Reads the Divide attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    custId         Associated Cust Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_CustDivideGet(zl303xx_ParamsS *zl303xx_Params,
                                zl303xx_CustIdE custId,
                                zl303xx_BooleanE *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
   }

   /* Check the custId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_CUST_ID(custId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_CUST_DIVIDE_REG(custId),
                              ZL303XX_CUST_DIVIDE_MASK,
                              ZL303XX_CUST_DIVIDE_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_BooleanE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_CustDivideGet */

/*

  Function Name:
   zl303xx_CustDivideSet

  Details:
   Writes the Divide attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    custId         Associated Cust Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_CustDivideSet(zl303xx_ParamsS *zl303xx_Params,
                                zl303xx_CustIdE custId,
                                zl303xx_BooleanE val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check params pointer */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
   }

   /* Check the custId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_CUST_ID(custId);
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
                              ZL303XX_CUST_DIVIDE_REG(custId),
                              ZL303XX_CUST_DIVIDE_MASK,
                              ZL303XX_CUST_DIVIDE_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_CustDivideSet */

