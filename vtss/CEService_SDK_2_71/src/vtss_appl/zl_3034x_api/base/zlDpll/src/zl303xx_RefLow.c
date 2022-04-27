

/*******************************************************************************
*
*  $Id: zl303xx_RefLow.c 6058 2011-06-10 14:37:16Z AW $
*
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     Functions for low-level REF attribute access
*
*******************************************************************************/

/*****************   INCLUDE FILES   ******************************************/

#include "zl303xx_ApiLow.h"
#include "zl303xx_RefLow.h"

/*****************   DEFINES   ************************************************/

/*****************   DATA TYPES   *********************************************/

/*****************   DATA STRUCTURES   ****************************************/

/*****************   EXPORTED GLOBAL VARIABLE DECLARATIONS   ******************/

/*

  Function Name:
   zl303xx_RefConfigStructInit

  Details:
   Initializes the members of the zl303xx_RefConfigS data structure

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    par            Pointer to the zl303xx_RefConfigS structure to Initialize

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_RefConfigStructInit(zl303xx_ParamsS *zl303xx_Params,
                                      zl303xx_RefConfigS *par)
{
   zlStatusE status = ZL303XX_OK;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, par);
   }

   if (status == ZL303XX_OK)
   {
      par->mode = (zl303xx_RefModeE)ZL303XX_INVALID;
      par->invert = (zl303xx_BooleanE)ZL303XX_INVALID;
      par->prescaler = (zl303xx_RefDivideE)ZL303XX_INVALID;
      par->oorLimit = (zl303xx_RefOorE)ZL303XX_INVALID;
      par->Id = (zl303xx_RefIdE)ZL303XX_INVALID;
   }

   return status;
} /* END zl303xx_RefConfigStructInit */

/*

  Function Name:
   zl303xx_RefConfigCheck

  Details:
   Checks the members of the zl303xx_RefConfigS data structure

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    par            Pointer to the zl303xx_RefConfigS structure to Check

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_RefConfigCheck(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_RefConfigS *par)
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
      status = ZL303XX_CHECK_REF_MODE(par->mode) |
               ZL303XX_CHECK_BOOLEAN(par->invert) |
               ZL303XX_CHECK_REF_DIVIDE(par->prescaler) |
               ZL303XX_CHECK_REF_OOR(par->oorLimit) |
               ZL303XX_CHECK_REF_ID(par->Id);
   }

   return status;
} /* END zl303xx_RefConfigCheck */

/*

  Function Name:
   zl303xx_RefConfigGet

  Details:
   Gets the members of the zl303xx_RefConfigS data structure

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    par            Pointer to the zl303xx_RefConfigS structure to Get

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_RefConfigGet(zl303xx_ParamsS *zl303xx_Params,
                               zl303xx_RefConfigS *par)
{
   zlStatusE status = ZL303XX_OK;
   Uint32T regValue = 0;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, par);
   }

   /* Check for a valid RefId */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_REF_ID(par->Id);
   }

   /* Read */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_Read(zl303xx_Params, NULL,
                            ZL303XX_REF_REF_MODE_REG(par->Id),
                            &regValue);
   }

   /* Extract */
   if (status == ZL303XX_OK)
   {
      par->mode = (zl303xx_RefModeE)ZL303XX_EXTRACT(regValue,
                                    ZL303XX_REF_FREQ_MODE_MASK,
                                    ZL303XX_REF_FREQ_MODE_SHIFT(par->Id));
   }

   /* Read */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_Read(zl303xx_Params, NULL,
                            ZL303XX_REF_REF_INVERT_REG,
                            &regValue);
   }

   /* Extract */
   if (status == ZL303XX_OK)
   {
      par->invert = (zl303xx_BooleanE)ZL303XX_EXTRACT(regValue,
                                      ZL303XX_REF_INVERT_MASK,
                                      ZL303XX_REF_INVERT_SHIFT(par->Id));
   }

   /* Only ref0 and ref1 have prescalers */
   if (par->Id <= ZL303XX_REF_ID_1)
   {
      /* Read */
      if (status == ZL303XX_OK)
      {
         status = zl303xx_Read(zl303xx_Params, NULL,
                               ZL303XX_REF_PRESCALE_CTRL_REG,
                               &regValue);
      }

      /* Extract */
      if (status == ZL303XX_OK)
      {
         par->prescaler = (zl303xx_RefDivideE)ZL303XX_EXTRACT(regValue,
                                              ZL303XX_REF_PRESCALER_MASK,
                                              ZL303XX_REF_PRESCALER_SHIFT(par->Id));
      }
   }
   else
   {
      par->prescaler = 1;
   }

   /* Read */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_Read(zl303xx_Params, NULL,
                            ZL303XX_REF_OOR_LIMIT_REG(par->Id),
                            &regValue);
   }

   /* Extract */
   if (status == ZL303XX_OK)
   {
      par->oorLimit = (zl303xx_RefOorE)ZL303XX_EXTRACT(regValue,
                                       ZL303XX_REF_OOR_LIMIT_MASK,
                                       ZL303XX_REF_OOR_LIMIT_SHIFT(par->Id));
   }

   return status;
} /* END zl303xx_RefConfigGet */

/*

  Function Name:
   zl303xx_RefConfigSet

  Details:
   Sets the members of the zl303xx_RefConfigS data structure

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    par            Pointer to the zl303xx_RefConfigS structure to Set

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_RefConfigSet(zl303xx_ParamsS *zl303xx_Params,
                               zl303xx_RefConfigS *par)
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
      status = zl303xx_RefConfigCheck(zl303xx_Params, par);
   }

   /* Package */
   if (status == ZL303XX_OK)
   {
      regValue = 0;
      mask = 0;

      /* mode */
      ZL303XX_INSERT(regValue, par->mode,
                             ZL303XX_REF_FREQ_MODE_MASK,
                             ZL303XX_REF_FREQ_MODE_SHIFT(par->Id));

      mask |= (ZL303XX_REF_FREQ_MODE_MASK << ZL303XX_REF_FREQ_MODE_SHIFT(par->Id));

      /* Write the Data for this Register */
      status = zl303xx_ReadModWrite(zl303xx_Params, NULL,
                                    ZL303XX_REF_REF_MODE_REG(par->Id),
                                    regValue, mask, NULL);
   }

   /* Package */
   if (status == ZL303XX_OK)
   {
      regValue = 0;
      mask = 0;

      /* invert */
      ZL303XX_INSERT(regValue, par->invert,
                             ZL303XX_REF_INVERT_MASK,
                             ZL303XX_REF_INVERT_SHIFT(par->Id));

      mask |= (ZL303XX_REF_INVERT_MASK << ZL303XX_REF_INVERT_SHIFT(par->Id));

      /* Write the Data for this Register */
      status = zl303xx_ReadModWrite(zl303xx_Params, NULL,
                                    ZL303XX_REF_REF_INVERT_REG,
                                    regValue, mask, NULL);
   }

   /* Only ref0 and ref1 have prescalers */
   if (par->Id <= ZL303XX_REF_ID_1)
   {
      /* Package */
      if (status == ZL303XX_OK)
      {
         regValue = 0;
         mask = 0;

         /* prescaler */
         ZL303XX_INSERT(regValue, par->prescaler,
                                ZL303XX_REF_PRESCALER_MASK,
                                ZL303XX_REF_PRESCALER_SHIFT(par->Id));

         mask |= (ZL303XX_REF_PRESCALER_MASK << ZL303XX_REF_PRESCALER_SHIFT(par->Id));

         /* Write the Data for this Register */
         status = zl303xx_ReadModWrite(zl303xx_Params, NULL,
                                    ZL303XX_REF_PRESCALE_CTRL_REG,
                                    regValue, mask, NULL);
      }
   }

   /* Package */
   if (status == ZL303XX_OK)
   {
      regValue = 0;
      mask = 0;

      /* oorLimit */
      ZL303XX_INSERT(regValue, par->oorLimit,
                             ZL303XX_REF_OOR_LIMIT_MASK,
                             ZL303XX_REF_OOR_LIMIT_SHIFT(par->Id));

      mask |= (ZL303XX_REF_OOR_LIMIT_MASK << ZL303XX_REF_OOR_LIMIT_SHIFT(par->Id));

      /* Write the Data for this Register */
      status = zl303xx_ReadModWrite(zl303xx_Params, NULL,
                                    ZL303XX_REF_OOR_LIMIT_REG(par->Id),
                                    regValue, mask, NULL);
   }

   return status;
} /* END zl303xx_RefConfigSet */

/*

  Function Name:
   zl303xx_GlobalInConfigStructInit

  Details:
   Initializes the members of the zl303xx_GlobalInConfigS data structure

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    par            Pointer to the zl303xx_GlobalInConfigS structure to Initialize

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_GlobalInConfigStructInit(zl303xx_ParamsS *zl303xx_Params,
                                           zl303xx_GlobalInConfigS *par)
{
   zlStatusE status = ZL303XX_OK;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, par);
   }

   if (status == ZL303XX_OK)
   {
      par->en1HzDetect = (zl303xx_BooleanE)ZL303XX_INVALID;
      par->ttDisqualify = (zl303xx_TtoDisQualE)ZL303XX_INVALID;
      par->ttQualify = (zl303xx_TtoQualE)ZL303XX_INVALID;
   }

   return status;
} /* END zl303xx_GlobalInConfigStructInit */

/*

  Function Name:
   zl303xx_GlobalInConfigCheck

  Details:
   Checks the members of the zl303xx_GlobalInConfigS data structure

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    par            Pointer to the zl303xx_GlobalInConfigS structure to Check

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_GlobalInConfigCheck(zl303xx_ParamsS *zl303xx_Params,
                                      zl303xx_GlobalInConfigS *par)
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
      status = ZL303XX_CHECK_BOOLEAN(par->en1HzDetect) |
               ZL303XX_CHECK_TTO_DIS_QUAL(par->ttDisqualify) |
               ZL303XX_CHECK_TTO_QUAL(par->ttQualify);
   }

   return status;
} /* END zl303xx_GlobalInConfigCheck */

/*

  Function Name:
   zl303xx_GlobalInConfigGet

  Details:
   Gets the members of the zl303xx_GlobalInConfigS data structure

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    par            Pointer to the zl303xx_GlobalInConfigS structure to Get

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_GlobalInConfigGet(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_GlobalInConfigS *par)
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
                            ZL303XX_REF_1_HZ_ENABLE_REG,
                            &regValue);
   }

   /* Extract */
   if (status == ZL303XX_OK)
   {
      par->en1HzDetect = (zl303xx_BooleanE)ZL303XX_EXTRACT(regValue,
                                           ZL303XX_REF_1_HZ_ENABLE_MASK,
                                           ZL303XX_REF_1_HZ_ENABLE_SHIFT);

      /* Update the device parameters structure */
      zl303xx_Params->pllParams.ref1HzDetectEnable = par->en1HzDetect;
   }

   /* Read */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_Read(zl303xx_Params, NULL,
                            ZL303XX_REF_GST_QUAL_TIME_REG,
                            &regValue);
   }

   /* Extract */
   if (status == ZL303XX_OK)
   {
      par->ttDisqualify = (zl303xx_TtoDisQualE)ZL303XX_EXTRACT(regValue,
                                               ZL303XX_REF_TIME2_DISQ_MASK,
                                               ZL303XX_REF_TIME2_DISQ_SHIFT);

      par->ttQualify = (zl303xx_TtoQualE)ZL303XX_EXTRACT(regValue,
                                          ZL303XX_REF_TIME2_QUALIFY_MASK,
                                          ZL303XX_REF_TIME2_QUALIFY_SHIFT);
   }

   return status;
} /* END zl303xx_GlobalInConfigGet */

/*

  Function Name:
   zl303xx_GlobalInConfigSet

  Details:
   Sets the members of the zl303xx_GlobalInConfigS data structure

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    par            Pointer to the zl303xx_GlobalInConfigS structure to Set

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_GlobalInConfigSet(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_GlobalInConfigS *par)
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
      status = zl303xx_GlobalInConfigCheck(zl303xx_Params, par);
   }

   /* Package */
   if (status == ZL303XX_OK)
   {
      regValue = 0;
      mask = 0;

      /* en1HzDetect */
      ZL303XX_INSERT(regValue, par->en1HzDetect,
                             ZL303XX_REF_1_HZ_ENABLE_MASK,
                             ZL303XX_REF_1_HZ_ENABLE_SHIFT);

      mask |= (Uint32T)(ZL303XX_REF_1_HZ_ENABLE_MASK <<
                        ZL303XX_REF_1_HZ_ENABLE_SHIFT);

      /* Write the Data for this Register */
      status = zl303xx_ReadModWrite(zl303xx_Params, NULL,
                                    ZL303XX_REF_1_HZ_ENABLE_REG,
                                    regValue, mask, NULL);
   }

   /* Update the device parameters structure */
   if (status == ZL303XX_OK)
   {
      zl303xx_Params->pllParams.ref1HzDetectEnable = par->en1HzDetect;
   }

   /* Package */
   if (status == ZL303XX_OK)
   {
      regValue = 0;
      mask = 0;

      /* ttDisqualify */
      ZL303XX_INSERT(regValue, par->ttDisqualify,
                             ZL303XX_REF_TIME2_DISQ_MASK,
                             ZL303XX_REF_TIME2_DISQ_SHIFT);

      /* ttQualify */
      ZL303XX_INSERT(regValue, par->ttQualify,
                             ZL303XX_REF_TIME2_QUALIFY_MASK,
                             ZL303XX_REF_TIME2_QUALIFY_SHIFT);

      mask |= (ZL303XX_REF_TIME2_DISQ_MASK << ZL303XX_REF_TIME2_DISQ_SHIFT) |
              (ZL303XX_REF_TIME2_QUALIFY_MASK << ZL303XX_REF_TIME2_QUALIFY_SHIFT);

      /* Write the Data for this Register */
      status = zl303xx_ReadModWrite(zl303xx_Params, NULL,
                                    ZL303XX_REF_GST_QUAL_TIME_REG,
                                    regValue, mask, NULL);
   }

   return status;
} /* END zl303xx_GlobalInConfigSet */

/*

  Function Name:
   zl303xx_SyncConfigStructInit

  Details:
   Initializes the members of the zl303xx_SyncConfigS data structure

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    par            Pointer to the zl303xx_SyncConfigS structure to Initialize

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SyncConfigStructInit(zl303xx_ParamsS *zl303xx_Params,
                                       zl303xx_SyncConfigS *par)
{
   zlStatusE status = ZL303XX_OK;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, par);
   }

   if (status == ZL303XX_OK)
   {
      par->enable = (zl303xx_BooleanE)ZL303XX_INVALID;
      par->invert = (zl303xx_BooleanE)ZL303XX_INVALID;
      par->Id = (zl303xx_SyncIdE)ZL303XX_INVALID;
   }

   return status;
} /* END zl303xx_SyncConfigStructInit */

/*

  Function Name:
   zl303xx_SyncConfigCheck

  Details:
   Checks the members of the zl303xx_SyncConfigS data structure

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    par            Pointer to the zl303xx_SyncConfigS structure to Check

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SyncConfigCheck(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_SyncConfigS *par)
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
      status = ZL303XX_CHECK_BOOLEAN(par->enable) |
               ZL303XX_CHECK_BOOLEAN(par->invert) |
               ZL303XX_CHECK_SYNC_ID(par->Id);
   }

   return status;
} /* END zl303xx_SyncConfigCheck */

/*

  Function Name:
   zl303xx_SyncConfigGet

  Details:
   Gets the members of the zl303xx_SyncConfigS data structure

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    par            Pointer to the zl303xx_SyncConfigS structure to Get

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SyncConfigGet(zl303xx_ParamsS *zl303xx_Params,
                                zl303xx_SyncConfigS *par)
{
   zlStatusE status = ZL303XX_OK;
   Uint32T regValue = 0;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, par);
   }

   /* Check for a valid SyncId */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_SYNC_ID(par->Id);
   }

   /* Read */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_Read(zl303xx_Params, NULL,
                            ZL303XX_REF_SYNC_ENABLE_REG,
                            &regValue);
   }

   /* Extract */
   if (status == ZL303XX_OK)
   {
      par->enable = (zl303xx_BooleanE)ZL303XX_EXTRACT(regValue,
                                      ZL303XX_REF_SYNC_ENABLE_MASK,
                                      ZL303XX_REF_SYNC_ENABLE_SHIFT(par->Id));

      par->invert = (zl303xx_BooleanE)ZL303XX_EXTRACT(regValue,
                                       ZL303XX_REF_SYNC_INVERT_MASK,
                                       ZL303XX_REF_SYNC_INVERT_SHIFT(par->Id));
   }

   return status;
} /* END zl303xx_SyncConfigGet */

/*

  Function Name:
   zl303xx_SyncConfigSet

  Details:
   Sets the members of the zl303xx_SyncConfigS data structure

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    par            Pointer to the zl303xx_SyncConfigS structure to Set

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SyncConfigSet(zl303xx_ParamsS *zl303xx_Params,
                                zl303xx_SyncConfigS *par)
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
      status = zl303xx_SyncConfigCheck(zl303xx_Params, par);
   }

   /* Package */
   if (status == ZL303XX_OK)
   {
      regValue = 0;
      mask = 0;

      /* enable */
      ZL303XX_INSERT(regValue, par->enable,
                             ZL303XX_REF_SYNC_ENABLE_MASK,
                             ZL303XX_REF_SYNC_ENABLE_SHIFT(par->Id));

      /* invert */
      ZL303XX_INSERT(regValue, par->invert,
                             ZL303XX_REF_SYNC_INVERT_MASK,
                             ZL303XX_REF_SYNC_INVERT_SHIFT(par->Id));

      mask |= (ZL303XX_REF_SYNC_ENABLE_MASK << ZL303XX_REF_SYNC_ENABLE_SHIFT(par->Id)) |
              (ZL303XX_REF_SYNC_INVERT_MASK << ZL303XX_REF_SYNC_INVERT_SHIFT(par->Id));

      /* Write the Data for this Register */
      status = zl303xx_ReadModWrite(zl303xx_Params, NULL,
                                    ZL303XX_REF_SYNC_ENABLE_REG,
                                    regValue, mask, NULL);
   }

   return status;
} /* END zl303xx_SyncConfigSet */

/*

  Function Name:
   zl303xx_SyncStatusStructInit

  Details:
   Initializes the members of the zl303xx_SyncStatusS data structure

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    par            Pointer to the zl303xx_SyncStatusS structure to Initialize

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SyncStatusStructInit(zl303xx_ParamsS *zl303xx_Params,
                                       zl303xx_SyncStatusS *par)
{
   zlStatusE status = ZL303XX_OK;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, par);
   }

   if (status == ZL303XX_OK)
   {
      par->detectedFreq = (zl303xx_SyncFreqE)ZL303XX_INVALID;
      par->failed = (zl303xx_BooleanE)ZL303XX_INVALID;
      par->Id = (zl303xx_SyncIdE)ZL303XX_INVALID;
   }

   return status;
} /* END zl303xx_SyncStatusStructInit */

/*

  Function Name:
   zl303xx_SyncStatusCheck

  Details:
   Checks the members of the zl303xx_SyncStatusS data structure

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    par            Pointer to the zl303xx_SyncStatusS structure to Check

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SyncStatusCheck(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_SyncStatusS *par)
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
      status = ZL303XX_CHECK_SYNC_FREQ(par->detectedFreq) |
               ZL303XX_CHECK_BOOLEAN(par->failed) |
               ZL303XX_CHECK_SYNC_ID(par->Id);
   }

   return status;
} /* END zl303xx_SyncStatusCheck */

/*

  Function Name:
   zl303xx_SyncStatusGet

  Details:
   Gets the members of the zl303xx_SyncStatusS data structure

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    par            Pointer to the zl303xx_SyncStatusS structure to Get

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_SyncStatusGet(zl303xx_ParamsS *zl303xx_Params,
                                zl303xx_SyncStatusS *par)
{
   zlStatusE status = ZL303XX_OK;
   Uint32T regValue = 0;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, par);
   }

   /* Check for a valid SyncId */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_SYNC_ID(par->Id);
   }

   /* Read */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_Read(zl303xx_Params, NULL,
                            ZL303XX_REF_SYNC_FREQ_REG,
                            &regValue);
   }

   /* Extract */
   if (status == ZL303XX_OK)
   {
      par->detectedFreq = (zl303xx_SyncFreqE)ZL303XX_EXTRACT(regValue,
                                             ZL303XX_REF_SYNC_FREQ_DETECTED_MASK,
                                             ZL303XX_REF_SYNC_FREQ_DETECTED_SHIFT(par->Id));

      par->failed = (zl303xx_BooleanE)ZL303XX_EXTRACT(regValue,
                                       ZL303XX_REF_SYNC_FAILED_MASK,
                                       ZL303XX_REF_SYNC_FAILED_SHIFT(par->Id));
   }

   return status;
} /* END zl303xx_SyncStatusGet */

/*

  Function Name:
   zl303xx_RefFreqModeGet

  Details:
   Reads the Ref Mode attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    refId          Associated REF Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_RefFreqModeGet(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_RefIdE refId,
                                 zl303xx_RefModeE *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
   }

   /* Check the refId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_REF_ID(refId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_REF_REF_MODE_REG(refId),
                              ZL303XX_REF_FREQ_MODE_MASK,
                              ZL303XX_REF_FREQ_MODE_SHIFT(refId));
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_RefModeE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_RefFreqModeGet */

/*

  Function Name:
   zl303xx_RefFreqModeSet

  Details:
   Writes the Ref Mode attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    refId          Associated REF Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_RefFreqModeSet(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_RefIdE refId,
                                 zl303xx_RefModeE val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check params pointer */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
   }

   /* Check the refId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_REF_ID(refId);
   }

   /* Check that the write value is within range */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_REF_MODE(val);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_REF_REF_MODE_REG(refId),
                              ZL303XX_REF_FREQ_MODE_MASK,
                              ZL303XX_REF_FREQ_MODE_SHIFT(refId));
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_RefFreqModeSet */


/*

  Function Name:
   zl303xx_RefInvertGet

  Details:
   Reads the Ref Invert attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    refId          Associated REF Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_RefInvertGet(zl303xx_ParamsS *zl303xx_Params,
                               zl303xx_RefIdE refId,
                               zl303xx_BooleanE *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
   }

   /* Check the refId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_REF_ID(refId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_REF_REF_INVERT_REG,
                              ZL303XX_REF_INVERT_MASK,
                              ZL303XX_REF_INVERT_SHIFT(refId));
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_BooleanE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_RefInvertGet */

/*

  Function Name:
   zl303xx_RefInvertSet

  Details:
   Writes the Ref Invert attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    refId          Associated REF Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_RefInvertSet(zl303xx_ParamsS *zl303xx_Params,
                               zl303xx_RefIdE refId,
                               zl303xx_BooleanE val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check params pointer */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
   }

   /* Check the refId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_REF_ID(refId);
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
                              ZL303XX_REF_REF_INVERT_REG,
                              ZL303XX_REF_INVERT_MASK,
                              ZL303XX_REF_INVERT_SHIFT(refId));
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_RefInvertSet */


/*

  Function Name:
   zl303xx_RefPrescalerGet

  Details:
   Reads the Ref Prescaler attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    refId          Associated REF Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_RefPrescalerGet(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_RefIdE refId,
                                  zl303xx_RefDivideE *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
   }

   /* Check the refId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_REF_ID(refId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_REF_PRESCALE_CTRL_REG,
                              ZL303XX_REF_PRESCALER_MASK,
                              ZL303XX_REF_PRESCALER_SHIFT(refId));
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_RefDivideE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_RefPrescalerGet */

/*

  Function Name:
   zl303xx_RefPrescalerSet

  Details:
   Writes the Ref Prescaler attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    refId          Associated REF Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_RefPrescalerSet(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_RefIdE refId,
                                  zl303xx_RefDivideE val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check params pointer */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
   }

   /* Check the refId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_REF_ID(refId);
   }

   /* Check that the write value is within range */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_REF_DIVIDE(val);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_REF_PRESCALE_CTRL_REG,
                              ZL303XX_REF_PRESCALER_MASK,
                              ZL303XX_REF_PRESCALER_SHIFT(refId));
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_RefPrescalerSet */


/*

  Function Name:
   zl303xx_RefFreqDetectedGet

  Details:
   Reads the Ref Freq Detected attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    refId          Associated REF Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_RefFreqDetectedGet(zl303xx_ParamsS *zl303xx_Params,
                                     zl303xx_RefIdE refId,
                                     zl303xx_RefFreqE *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
   }

   /* Check the refId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_REF_ID(refId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_REF_REF_FREQ_REG(refId),
                              ZL303XX_REF_FREQ_DETECTED_MASK,
                              ZL303XX_REF_FREQ_DETECTED_SHIFT(refId));
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_RefFreqE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_RefFreqDetectedGet */

/*

  Function Name:
   zl303xx_Ref1HzEnableGet

  Details:
   Reads the 1Hz Auto Detect attribute

  Parameters:
   [in]    zl303xx_Params   Pointer to the device instance parameter structure
   [in]    val            Pointer to the device attribute parameter read

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_Ref1HzEnableGet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_REF_1_HZ_ENABLE_REG,
                              ZL303XX_REF_1_HZ_ENABLE_MASK,
                              ZL303XX_REF_1_HZ_ENABLE_SHIFT);
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
      zl303xx_Params->pllParams.ref1HzDetectEnable = *val;
   }

   return status;
}  /* END zl303xx_Ref1HzEnableGet */

/*

  Function Name:
   zl303xx_Ref1HzEnableSet

  Details:
   Writes the 1Hz Auto Detect attribute

  Parameters:
   [in]    zl303xx_Params   Pointer to the device instance parameter structure
   [in]    val            Pointer to the device attribute parameter read

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_Ref1HzEnableSet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_REF_1_HZ_ENABLE_REG,
                              ZL303XX_REF_1_HZ_ENABLE_MASK,
                              ZL303XX_REF_1_HZ_ENABLE_SHIFT);
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
      zl303xx_Params->pllParams.ref1HzDetectEnable = val;
   }

   return status;
}  /* END zl303xx_Ref1HzEnableSet */

/*

  Function Name:
   zl303xx_RefSyncEnableGet

  Details:
   Reads the Sync Enable attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    syncId         Associated SYNC Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_RefSyncEnableGet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_SyncIdE syncId,
                                   zl303xx_BooleanE *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
   }

   /* Check the syncId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_SYNC_ID(syncId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_REF_SYNC_ENABLE_REG,
                              ZL303XX_REF_SYNC_ENABLE_MASK,
                              ZL303XX_REF_SYNC_ENABLE_SHIFT(syncId));
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_BooleanE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_RefSyncEnableGet */

/*

  Function Name:
   zl303xx_RefSyncEnableSet

  Details:
   Writes the Sync Enable attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    syncId         Associated SYNC Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_RefSyncEnableSet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_SyncIdE syncId,
                                   zl303xx_BooleanE val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check params pointer */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
   }

   /* Check the syncId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_SYNC_ID(syncId);
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
                              ZL303XX_REF_SYNC_ENABLE_REG,
                              ZL303XX_REF_SYNC_ENABLE_MASK,
                              ZL303XX_REF_SYNC_ENABLE_SHIFT(syncId));
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_RefSyncEnableSet */


/*

  Function Name:
   zl303xx_RefSyncInvertGet

  Details:
   Reads the Sync Invert attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    syncId         Associated SYNC Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_RefSyncInvertGet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_SyncIdE syncId,
                                   zl303xx_BooleanE *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
   }

   /* Check the syncId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_SYNC_ID(syncId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_REF_SYNC_ENABLE_REG,
                              ZL303XX_REF_SYNC_INVERT_MASK,
                              ZL303XX_REF_SYNC_INVERT_SHIFT(syncId));
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_BooleanE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_RefSyncInvertGet */

/*

  Function Name:
   zl303xx_RefSyncInvertSet

  Details:
   Writes the Sync Invert attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    syncId         Associated SYNC Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_RefSyncInvertSet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_SyncIdE syncId,
                                   zl303xx_BooleanE val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check params pointer */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
   }

   /* Check the syncId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_SYNC_ID(syncId);
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
                              ZL303XX_REF_SYNC_ENABLE_REG,
                              ZL303XX_REF_SYNC_INVERT_MASK,
                              ZL303XX_REF_SYNC_INVERT_SHIFT(syncId));
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_RefSyncInvertSet */


/*

  Function Name:
   zl303xx_RefSyncFreqDetectedGet

  Details:
   Reads the Sync Freq attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    syncId         Associated SYNC Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_RefSyncFreqDetectedGet(zl303xx_ParamsS *zl303xx_Params,
                                         zl303xx_SyncIdE syncId,
                                         zl303xx_SyncFreqE *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
   }

   /* Check the syncId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_SYNC_ID(syncId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_REF_SYNC_FREQ_REG,
                              ZL303XX_REF_SYNC_FREQ_DETECTED_MASK,
                              ZL303XX_REF_SYNC_FREQ_DETECTED_SHIFT(syncId));
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_SyncFreqE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_RefSyncFreqDetectedGet */


/*

  Function Name:
   zl303xx_RefSyncFailedGet

  Details:
   Reads the Sync Fail attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    syncId         Associated SYNC Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_RefSyncFailedGet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_SyncIdE syncId,
                                   zl303xx_BooleanE *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
   }

   /* Check the syncId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_SYNC_ID(syncId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_REF_SYNC_FREQ_REG,
                              ZL303XX_REF_SYNC_FAILED_MASK,
                              ZL303XX_REF_SYNC_FAILED_SHIFT(syncId));
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_BooleanE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_RefSyncFailedGet */


/*

  Function Name:
   zl303xx_RefOorLimitGet

  Details:
   Reads the Ref OOR Limit attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    refId          Associated REF Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_RefOorLimitGet(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_RefIdE refId,
                                 zl303xx_RefOorE *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
   }

   /* Check the refId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_REF_ID(refId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_REF_OOR_LIMIT_REG(refId),
                              ZL303XX_REF_OOR_LIMIT_MASK,
                              ZL303XX_REF_OOR_LIMIT_SHIFT(refId));
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_RefOorE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_RefOorLimitGet */

/*

  Function Name:
   zl303xx_RefOorLimitSet

  Details:
   Writes the Ref OOR Limit attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    refId          Associated REF Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_RefOorLimitSet(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_RefIdE refId,
                                 zl303xx_RefOorE val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check params pointer */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
   }

   /* Check the refId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_REF_ID(refId);
   }

   /* Check that the write value is within range */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_REF_OOR(val);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_REF_OOR_LIMIT_REG(refId),
                              ZL303XX_REF_OOR_LIMIT_MASK,
                              ZL303XX_REF_OOR_LIMIT_SHIFT(refId));
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_RefOorLimitSet */


/*

  Function Name:
   zl303xx_RefTime2DisqGet

  Details:
   Reads the Time to Disq attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_RefTime2DisqGet(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_TtoDisQualE *val)
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
                              ZL303XX_REF_GST_QUAL_TIME_REG,
                              ZL303XX_REF_TIME2_DISQ_MASK,
                              ZL303XX_REF_TIME2_DISQ_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_TtoDisQualE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_RefTime2DisqGet */

/*

  Function Name:
   zl303xx_RefTime2DisqSet

  Details:
   Writes the Time to Disq attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_RefTime2DisqSet(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_TtoDisQualE val)
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
      status = ZL303XX_CHECK_TTO_DIS_QUAL(val);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_REF_GST_QUAL_TIME_REG,
                              ZL303XX_REF_TIME2_DISQ_MASK,
                              ZL303XX_REF_TIME2_DISQ_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_RefTime2DisqSet */


/*

  Function Name:
   zl303xx_RefTime2QualifyGet

  Details:
   Reads the Time to Qualify attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_RefTime2QualifyGet(zl303xx_ParamsS *zl303xx_Params,
                                     zl303xx_TtoQualE *val)
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
                              ZL303XX_REF_GST_QUAL_TIME_REG,
                              ZL303XX_REF_TIME2_QUALIFY_MASK,
                              ZL303XX_REF_TIME2_QUALIFY_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_TtoQualE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_RefTime2QualifyGet */

/*

  Function Name:
   zl303xx_RefTime2QualifySet

  Details:
   Writes the Time to Qualify attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_RefTime2QualifySet(zl303xx_ParamsS *zl303xx_Params,
                                     zl303xx_TtoQualE val)
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
      status = ZL303XX_CHECK_TTO_QUAL(val);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_REF_GST_QUAL_TIME_REG,
                              ZL303XX_REF_TIME2_QUALIFY_MASK,
                              ZL303XX_REF_TIME2_QUALIFY_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_RefTime2QualifySet */


