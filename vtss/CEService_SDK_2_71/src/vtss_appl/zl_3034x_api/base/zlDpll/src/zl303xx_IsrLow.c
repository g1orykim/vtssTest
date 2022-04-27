

/*******************************************************************************
*
*  $Id: zl303xx_IsrLow.c 6058 2011-06-10 14:37:16Z AW $
*
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     Functions for low-level ISR attribute access
*
*******************************************************************************/

/*****************   INCLUDE FILES   ******************************************/

#include "zl303xx_ApiLow.h"
#include "zl303xx_IsrLow.h"

/*****************   DEFINES   ************************************************/

/*****************   DATA TYPES   *********************************************/

/*****************   DATA STRUCTURES   ****************************************/

/*****************   EXPORTED GLOBAL VARIABLE DECLARATIONS   ******************/

/*

  Function Name:
   zl303xx_RefIsrStatusStructInit

  Details:
   Initializes the members of the zl303xx_RefIsrStatusS data structure

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    par            Pointer to the zl303xx_RefIsrStatusS structure to
                           Initialize

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_RefIsrStatusStructInit(zl303xx_ParamsS *zl303xx_Params,
                                         zl303xx_RefIsrStatusS *par)
{
   zlStatusE status = ZL303XX_OK;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, par);
   }

   if (status == ZL303XX_OK)
   {
      par->refFail = (zl303xx_BooleanE)ZL303XX_INVALID;
      par->scmFail = (zl303xx_BooleanE)ZL303XX_INVALID;
      par->cfmFail = (zl303xx_BooleanE)ZL303XX_INVALID;
      par->gstFail = (zl303xx_BooleanE)ZL303XX_INVALID;
      par->pfmFail = (zl303xx_BooleanE)ZL303XX_INVALID;
      par->Id = (zl303xx_RefIdE)ZL303XX_INVALID;
   }

   return status;
} /* END zl303xx_RefIsrStatusStructInit */

/*

  Function Name:
   zl303xx_RefIsrStatusCheck

  Details:
   Checks the members of the zl303xx_RefIsrStatusS data structure

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    par            Pointer to the zl303xx_RefIsrStatusS structure to Check

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_RefIsrStatusCheck(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_RefIsrStatusS *par)
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
      status = ZL303XX_CHECK_BOOLEAN(par->refFail) |
               ZL303XX_CHECK_BOOLEAN(par->scmFail) |
               ZL303XX_CHECK_BOOLEAN(par->cfmFail) |
               ZL303XX_CHECK_BOOLEAN(par->gstFail) |
               ZL303XX_CHECK_BOOLEAN(par->pfmFail) |
               ZL303XX_CHECK_REF_ID(par->Id);
   }

   return status;
} /* END zl303xx_RefIsrStatusCheck */

/**

  Function Name:
   zl303xx_RefIsrStatusGet

  Details:
   Gets the Fail Status of all Reference Monitor components.
   Gets the members of the zl303xx_RefIsrStatusS data structure

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    par            Pointer to the zl303xx_RefIsrStatusS structure to Get

  Return Value:
   zlStatusE

  Notes:
   This function reads a register with sticky bits. It should be called
   once to read their values and a second time to knock them down.

*******************************************************************************/

zlStatusE zl303xx_RefIsrStatusGet(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_RefIsrStatusS *par)
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
                            ZL303XX_ISR_REF_STATE_REG(par->Id),
                            &regValue);
   }

   /* Extract */
   if (status == ZL303XX_OK)
   {
      par->refFail = (zl303xx_BooleanE)ZL303XX_EXTRACT(regValue,
                                       ZL303XX_ISR_REF_FAIL_STATUS_MASK,
                                       ZL303XX_ISR_REF_FAIL_STATUS_SHIFT(par->Id));
   }

   /* Read */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_Read(zl303xx_Params, NULL,
                            ZL303XX_ISR_MON_STATE_REG(par->Id),
                            &regValue);
   }

   /* Extract */
   if (status == ZL303XX_OK)
   {
      par->scmFail = (zl303xx_BooleanE)ZL303XX_EXTRACT(regValue,
                                       ZL303XX_ISR_SCM_FAIL_STATUS_MASK,
                                       ZL303XX_ISR_SCM_FAIL_STATUS_SHIFT(par->Id));

      par->cfmFail = (zl303xx_BooleanE)ZL303XX_EXTRACT(regValue,
                                        ZL303XX_ISR_CFM_FAIL_STATUS_MASK,
                                        ZL303XX_ISR_CFM_FAIL_STATUS_SHIFT(par->Id));

      par->gstFail = (zl303xx_BooleanE)ZL303XX_EXTRACT(regValue,
                                        ZL303XX_ISR_GST_FAIL_STATUS_MASK,
                                        ZL303XX_ISR_GST_FAIL_STATUS_SHIFT(par->Id));

      par->pfmFail = (zl303xx_BooleanE)ZL303XX_EXTRACT(regValue,
                                        ZL303XX_ISR_PFM_FAIL_STATUS_MASK,
                                        ZL303XX_ISR_PFM_FAIL_STATUS_SHIFT(par->Id));
   }

   return status;
} /* END zl303xx_RefIsrStatusGet */

/*

  Function Name:
   zl303xx_RefIsrConfigStructInit

  Details:
   Initializes the members of the zl303xx_RefIsrConfigS data structure

  Parameters:
   [in]   zl303xx_Params Pointer to the device instance parameter structure
   [in]   par            Pointer to the zl303xx_RefIsrConfigS structure to Initialize

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_RefIsrConfigStructInit(zl303xx_ParamsS *zl303xx_Params,
                                         zl303xx_RefIsrConfigS *par)
{
   zlStatusE status = ZL303XX_OK;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, par);
   }

   if (status == ZL303XX_OK)
   {
      par->refIsrEn = (zl303xx_BooleanE)ZL303XX_INVALID;
      par->scmIsrEn = (zl303xx_BooleanE)ZL303XX_INVALID;
      par->cfmIsrEn = (zl303xx_BooleanE)ZL303XX_INVALID;
      par->gstIsrEn = (zl303xx_BooleanE)ZL303XX_INVALID;
      par->pfmIsrEn = (zl303xx_BooleanE)ZL303XX_INVALID;
      par->gstScmIsrEn = (zl303xx_BooleanE)ZL303XX_INVALID;
      par->gstCfmIsrEn = (zl303xx_BooleanE)ZL303XX_INVALID;
      par->Id = (zl303xx_RefIdE)ZL303XX_INVALID;
   }

   return status;
} /* END zl303xx_RefIsrConfigStructInit */

/*

  Function Name:
   zl303xx_RefIsrConfigCheck

  Details:
   Checks the members of the zl303xx_RefIsrConfigS data structure

  Parameters:
   [in]   zl303xx_Params Pointer to the device instance parameter structure
   [in]   par            Pointer to the zl303xx_RefIsrConfigS structure to Check

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_RefIsrConfigCheck(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_RefIsrConfigS *par)
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
      status = ZL303XX_CHECK_BOOLEAN(par->refIsrEn) |
               ZL303XX_CHECK_BOOLEAN(par->scmIsrEn) |
               ZL303XX_CHECK_BOOLEAN(par->cfmIsrEn) |
               ZL303XX_CHECK_BOOLEAN(par->gstIsrEn) |
               ZL303XX_CHECK_BOOLEAN(par->pfmIsrEn) |
               ZL303XX_CHECK_BOOLEAN(par->gstScmIsrEn) |
               ZL303XX_CHECK_BOOLEAN(par->gstCfmIsrEn) |
               ZL303XX_CHECK_REF_ID(par->Id);
   }

   return status;
} /* END zl303xx_RefIsrConfigCheck */

/**

  Function Name:
   zl303xx_RefIsrConfigGet

  Details:
  Query all element mask settings.
  Gets the members of the zl303xx_RefIsrConfigS data structure

  Parameters:
   [in]   zl303xx_Params Pointer to the device instance parameter structure
   [in]   par            Pointer to the zl303xx_RefIsrConfigS structure to Get

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_RefIsrConfigGet(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_RefIsrConfigS *par)
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
                            ZL303XX_ISR_REF_MASK_REG(par->Id),
                            &regValue);
   }

   /* Extract */
   if (status == ZL303XX_OK)
   {
      par->refIsrEn = (zl303xx_BooleanE)ZL303XX_EXTRACT(regValue,
                                        ZL303XX_ISR_REF_FAIL_MASK_EN_MASK,
                                        ZL303XX_ISR_REF_FAIL_MASK_EN_SHIFT(par->Id));
   }

   /* Read */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_Read(zl303xx_Params, NULL,
                            ZL303XX_ISR_MON_MASK_REG(par->Id),
                            &regValue);
   }

   /* Extract */
   if (status == ZL303XX_OK)
   {
      par->scmIsrEn = (zl303xx_BooleanE)ZL303XX_EXTRACT(regValue,
                                        ZL303XX_ISR_SCM_FAIL_MASK_EN_MASK,
                                        ZL303XX_ISR_SCM_FAIL_MASK_EN_SHIFT(par->Id));

      par->cfmIsrEn = (zl303xx_BooleanE)ZL303XX_EXTRACT(regValue,
                                         ZL303XX_ISR_CFM_FAIL_MASK_EN_MASK,
                                         ZL303XX_ISR_CFM_FAIL_MASK_EN_SHIFT(par->Id));

      par->gstIsrEn = (zl303xx_BooleanE)ZL303XX_EXTRACT(regValue,
                                         ZL303XX_ISR_GST_FAIL_MASK_EN_MASK,
                                         ZL303XX_ISR_GST_FAIL_MASK_EN_SHIFT(par->Id));

      par->pfmIsrEn = (zl303xx_BooleanE)ZL303XX_EXTRACT(regValue,
                                         ZL303XX_ISR_PFM_FAIL_MASK_EN_MASK,
                                         ZL303XX_ISR_PFM_FAIL_MASK_EN_SHIFT(par->Id));
   }

   /* Read */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_Read(zl303xx_Params, NULL,
                            ZL303XX_ISR_GST_MASK_REG(par->Id),
                            &regValue);
   }

   /* Extract */
   if (status == ZL303XX_OK)
   {
      par->gstScmIsrEn = (zl303xx_BooleanE)ZL303XX_EXTRACT(regValue,
                                            ZL303XX_ISR_GST_SCM_MASK_EN_MASK,
                                            ZL303XX_ISR_GST_SCM_MASK_EN_SHIFT(par->Id));

      par->gstCfmIsrEn = (zl303xx_BooleanE)ZL303XX_EXTRACT(regValue,
                                             ZL303XX_ISR_GST_CFM_MASK_EN_MASK,
                                             ZL303XX_ISR_GST_CFM_MASK_EN_SHIFT(par->Id));
   }

   return status;
} /* END zl303xx_RefIsrConfigGet */

/**

  Function Name:
   zl303xx_RefIsrConfigSet

  Details:
   Set all element mask configurations.
   Sets the members of the zl303xx_RefIsrConfigS data structure

  Parameters:
   [in]   zl303xx_Params Pointer to the device instance parameter structure
   [in]   par            Pointer to the zl303xx_RefIsrConfigS structure to Set

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_RefIsrConfigSet(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_RefIsrConfigS *par)
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
      status = zl303xx_RefIsrConfigCheck(zl303xx_Params, par);
   }

   /* Package */
   if (status == ZL303XX_OK)
   {
      regValue = 0;
      mask = 0;

      /* refIsrEn */
      ZL303XX_INSERT(regValue, par->refIsrEn,
                             ZL303XX_ISR_REF_FAIL_MASK_EN_MASK,
                             ZL303XX_ISR_REF_FAIL_MASK_EN_SHIFT(par->Id));

      mask |= (Uint32T)(ZL303XX_ISR_REF_FAIL_MASK_EN_MASK <<
                        ZL303XX_ISR_REF_FAIL_MASK_EN_SHIFT(par->Id));

      /* Write the Data for this Register */
      status = zl303xx_ReadModWrite(zl303xx_Params, NULL,
                                    ZL303XX_ISR_REF_MASK_REG(par->Id),
                                    regValue, mask, NULL);
   }

   /* Package */
   if (status == ZL303XX_OK)
   {
      regValue = 0;
      mask = 0;

      /* scmIsrEn */
      ZL303XX_INSERT(regValue, par->scmIsrEn,
                             ZL303XX_ISR_SCM_FAIL_MASK_EN_MASK,
                             ZL303XX_ISR_SCM_FAIL_MASK_EN_SHIFT(par->Id));

      /* cfmIsrEn */
      ZL303XX_INSERT(regValue, par->cfmIsrEn,
                             ZL303XX_ISR_CFM_FAIL_MASK_EN_MASK,
                             ZL303XX_ISR_CFM_FAIL_MASK_EN_SHIFT(par->Id));

      /* gstIsrEn */
      ZL303XX_INSERT(regValue, par->gstIsrEn,
                             ZL303XX_ISR_GST_FAIL_MASK_EN_MASK,
                             ZL303XX_ISR_GST_FAIL_MASK_EN_SHIFT(par->Id));

      /* pfmIsrEn */
      ZL303XX_INSERT(regValue, par->pfmIsrEn,
                             ZL303XX_ISR_PFM_FAIL_MASK_EN_MASK,
                             ZL303XX_ISR_PFM_FAIL_MASK_EN_SHIFT(par->Id));

      mask |= (ZL303XX_ISR_SCM_FAIL_MASK_EN_MASK << ZL303XX_ISR_SCM_FAIL_MASK_EN_SHIFT(par->Id)) |
              (ZL303XX_ISR_CFM_FAIL_MASK_EN_MASK << ZL303XX_ISR_CFM_FAIL_MASK_EN_SHIFT(par->Id)) |
              (ZL303XX_ISR_GST_FAIL_MASK_EN_MASK << ZL303XX_ISR_GST_FAIL_MASK_EN_SHIFT(par->Id)) |
              (ZL303XX_ISR_PFM_FAIL_MASK_EN_MASK << ZL303XX_ISR_PFM_FAIL_MASK_EN_SHIFT(par->Id));

      /* Write the Data for this Register */
      status = zl303xx_ReadModWrite(zl303xx_Params, NULL,
                                    ZL303XX_ISR_MON_MASK_REG(par->Id),
                                    regValue, mask, NULL);
   }

   /* Package */
   if (status == ZL303XX_OK)
   {
      regValue = 0;
      mask = 0;

      /* gstScmIsrEn */
      ZL303XX_INSERT(regValue, par->gstScmIsrEn,
                             ZL303XX_ISR_GST_SCM_MASK_EN_MASK,
                             ZL303XX_ISR_GST_SCM_MASK_EN_SHIFT(par->Id));

      /* gstCfmIsrEn */
      ZL303XX_INSERT(regValue, par->gstCfmIsrEn,
                             ZL303XX_ISR_GST_CFM_MASK_EN_MASK,
                             ZL303XX_ISR_GST_CFM_MASK_EN_SHIFT(par->Id));

      mask |= (ZL303XX_ISR_GST_SCM_MASK_EN_MASK << ZL303XX_ISR_GST_SCM_MASK_EN_SHIFT(par->Id)) |
              (ZL303XX_ISR_GST_CFM_MASK_EN_MASK << ZL303XX_ISR_GST_CFM_MASK_EN_SHIFT(par->Id));

      /* Write the Data for this Register */
      status = zl303xx_ReadModWrite(zl303xx_Params, NULL,
                                    ZL303XX_ISR_GST_MASK_REG(par->Id),
                                    regValue, mask, NULL);
   }

   return status;
} /* END zl303xx_RefIsrConfigSet */

/*

  Function Name:
   zl303xx_DpllIsrStructInit

  Details:
   Initializes the members of the zl303xx_DpllIsrS data structure

  Parameters:
   [in]   zl303xx_Params Pointer to the device instance parameter structure
   [in]   par            Pointer to the zl303xx_DpllIsrS structure to Initialize

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllIsrStructInit(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_DpllIsrS *par)
{
   zlStatusE status = ZL303XX_OK;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, par);
   }

   if (status == ZL303XX_OK)
   {
      par->locked = (zl303xx_BooleanE)ZL303XX_INVALID;
      par->lostLock = (zl303xx_BooleanE)ZL303XX_INVALID;
      par->holdover = (zl303xx_BooleanE)ZL303XX_INVALID;
      par->refChange = (zl303xx_BooleanE)ZL303XX_INVALID;
      par->Id = (zl303xx_DpllIdE)ZL303XX_INVALID;
   }

   return status;
} /* END zl303xx_DpllIsrStructInit */

/*

  Function Name:
   zl303xx_DpllIsrCheck

  Details:
   Checks the members of the zl303xx_DpllIsrS data structure

  Parameters:
   [in]   zl303xx_Params Pointer to the device instance parameter structure
   [in]   par            Pointer to the zl303xx_DpllIsrS structure to Check

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllIsrCheck(zl303xx_ParamsS *zl303xx_Params,
                               zl303xx_DpllIsrS *par)
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
      status = ZL303XX_CHECK_BOOLEAN(par->locked) |
               ZL303XX_CHECK_BOOLEAN(par->lostLock) |
               ZL303XX_CHECK_BOOLEAN(par->holdover) |
               ZL303XX_CHECK_BOOLEAN(par->refChange) |
               ZL303XX_CHECK_DPLL_ID(par->Id);
   }

   return status;
} /* END zl303xx_DpllIsrCheck */

/**

  Function Name:
   zl303xx_DpllIsrGet

  Details:
   Gets the members of the zl303xx_DpllIsrS data structure indicating the current
   interrupt status of the specified DPLL.

  Parameters:
   [in]   zl303xx_Params Pointer to the device instance parameter structure
   [in]   par            Pointer to the zl303xx_DpllIsrS structure to Get

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllIsrGet(zl303xx_ParamsS *zl303xx_Params,
                             zl303xx_DpllIsrS *par)
{
   zlStatusE status = ZL303XX_OK;
   Uint32T regValue = 0;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, par);
   }

   /* Check for a valid DpllId */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_DPLL_ID(par->Id);
   }

   /* Read */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_Read(zl303xx_Params, NULL,
                            ZL303XX_ISR_DPLL_STATE_REG(par->Id),
                            &regValue);
   }

   /* Extract */
   if (status == ZL303XX_OK)
   {
      par->locked = (zl303xx_BooleanE)ZL303XX_EXTRACT(regValue,
                                      ZL303XX_ISR_DPLL_LOCKED_STATUS_MASK,
                                      ZL303XX_ISR_DPLL_LOCKED_STATUS_SHIFT);

      par->lostLock = (zl303xx_BooleanE)ZL303XX_EXTRACT(regValue,
                                         ZL303XX_ISR_DPLL_LOST_LOCK_STATUS_MASK,
                                         ZL303XX_ISR_DPLL_LOST_LOCK_STATUS_SHIFT);

      par->holdover = (zl303xx_BooleanE)ZL303XX_EXTRACT(regValue,
                                         ZL303XX_ISR_DPLL_HOLDOVER_STATUS_MASK,
                                         ZL303XX_ISR_DPLL_HOLDOVER_STATUS_SHIFT);

      par->refChange = (zl303xx_BooleanE)ZL303XX_EXTRACT(regValue,
                                          ZL303XX_ISR_DPLL_REF_CHGD_STATUS_MASK,
                                          ZL303XX_ISR_DPLL_REF_CHGD_STATUS_SHIFT);
   }

   return status;
} /* END zl303xx_DpllIsrGet */

/*

  Function Name:
   zl303xx_DpllIsrConfigStructInit

  Details:
   Initializes the members of the zl303xx_DpllIsrConfigS data structure

  Parameters:
   [in]   zl303xx_Params Pointer to the device instance parameter structure
   [in]   par            Pointer to the zl303xx_DpllIsrConfigS structure to Initialize

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllIsrConfigStructInit(zl303xx_ParamsS *zl303xx_Params,
                                          zl303xx_DpllIsrConfigS *par)
{
   zlStatusE status = ZL303XX_OK;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, par);
   }

   if (status == ZL303XX_OK)
   {
      par->lockIsrEn = (zl303xx_BooleanE)ZL303XX_INVALID;
      par->lostLockIsrEn = (zl303xx_BooleanE)ZL303XX_INVALID;
      par->holdoverIsrEn = (zl303xx_BooleanE)ZL303XX_INVALID;
      par->refChangeIsrEn = (zl303xx_BooleanE)ZL303XX_INVALID;
      par->Id = (zl303xx_DpllIdE)ZL303XX_INVALID;
   }

   return status;
} /* END zl303xx_DpllIsrConfigStructInit */

/*

  Function Name:
   zl303xx_DpllIsrConfigCheck

  Details:
   Checks the members of the zl303xx_DpllIsrConfigS data structure

  Parameters:
   [in]   zl303xx_Params Pointer to the device instance parameter structure
   [in]   par            Pointer to the zl303xx_DpllIsrConfigS structure to Check

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllIsrConfigCheck(zl303xx_ParamsS *zl303xx_Params,
                                     zl303xx_DpllIsrConfigS *par)
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
      status = ZL303XX_CHECK_BOOLEAN(par->lockIsrEn) |
               ZL303XX_CHECK_BOOLEAN(par->lostLockIsrEn) |
               ZL303XX_CHECK_BOOLEAN(par->holdoverIsrEn) |
               ZL303XX_CHECK_BOOLEAN(par->refChangeIsrEn) |
               ZL303XX_CHECK_DPLL_ID(par->Id);
   }

   return status;
} /* END zl303xx_DpllIsrConfigCheck */

/**

  Function Name:
   zl303xx_DpllIsrConfigGet

  Details:
   Gets the members of the zl303xx_DpllIsrConfigS data structure indicating the
   current interrupt configuration of the specified DPLL.

  Parameters:
   [in]   zl303xx_Params Pointer to the device instance parameter structure
   [in]   par            Pointer to the zl303xx_DpllIsrConfigS structure to Get

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllIsrConfigGet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_DpllIsrConfigS *par)
{
   zlStatusE status = ZL303XX_OK;
   Uint32T regValue = 0;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, par);
   }

   /* Check for a valid DpllId */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_DPLL_ID(par->Id);
   }

   /* Read */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_Read(zl303xx_Params, NULL,
                            ZL303XX_ISR_DPLL_MASK_REG(par->Id),
                            &regValue);
   }

   /* Extract */
   if (status == ZL303XX_OK)
   {
      par->lockIsrEn = (zl303xx_BooleanE)ZL303XX_EXTRACT(regValue,
                                         ZL303XX_ISR_DPLL_LOCKED_MASK_EN_MASK,
                                         ZL303XX_ISR_DPLL_LOCKED_MASK_EN_SHIFT);

      par->lostLockIsrEn = (zl303xx_BooleanE)ZL303XX_EXTRACT(regValue,
                                              ZL303XX_ISR_DPLL_LOST_LOCK_MASK_EN_MASK,
                                              ZL303XX_ISR_DPLL_LOST_LOCK_MASK_EN_SHIFT);

      par->holdoverIsrEn = (zl303xx_BooleanE)ZL303XX_EXTRACT(regValue,
                                              ZL303XX_ISR_DPLL_HOLDOVER_MASK_EN_MASK,
                                              ZL303XX_ISR_DPLL_HOLDOVER_MASK_EN_SHIFT);

      par->refChangeIsrEn = (zl303xx_BooleanE)ZL303XX_EXTRACT(regValue,
                                               ZL303XX_ISR_DPLL_REF_CHGD_MASK_EN_MASK,
                                               ZL303XX_ISR_DPLL_REF_CHGD_MASK_EN_SHIFT);
   }

   return status;
} /* END zl303xx_DpllIsrConfigGet */

/**

  Function Name:
   zl303xx_DpllIsrConfigSet

  Details:
   Sets the interrupt configuration of the specified DPLL using the members of
   the zl303xx_DpllIsrConfigS data structure.

  Parameters:
   [in]   zl303xx_Params Pointer to the device instance parameter structure
   [in]   par            Pointer to the zl303xx_DpllIsrConfigS structure to Set

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllIsrConfigSet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_DpllIsrConfigS *par)
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
      status = zl303xx_DpllIsrConfigCheck(zl303xx_Params, par);
   }

   /* Package */
   if (status == ZL303XX_OK)
   {
      regValue = 0;
      mask = 0;

      /* lockIsrEn */
      ZL303XX_INSERT(regValue, par->lockIsrEn,
                             ZL303XX_ISR_DPLL_LOCKED_MASK_EN_MASK,
                             ZL303XX_ISR_DPLL_LOCKED_MASK_EN_SHIFT);

      /* lostLockIsrEn */
      ZL303XX_INSERT(regValue, par->lostLockIsrEn,
                             ZL303XX_ISR_DPLL_LOST_LOCK_MASK_EN_MASK,
                             ZL303XX_ISR_DPLL_LOST_LOCK_MASK_EN_SHIFT);

      /* holdoverIsrEn */
      ZL303XX_INSERT(regValue, par->holdoverIsrEn,
                             ZL303XX_ISR_DPLL_HOLDOVER_MASK_EN_MASK,
                             ZL303XX_ISR_DPLL_HOLDOVER_MASK_EN_SHIFT);

      /* refChangeIsrEn */
      ZL303XX_INSERT(regValue, par->refChangeIsrEn,
                             ZL303XX_ISR_DPLL_REF_CHGD_MASK_EN_MASK,
                             ZL303XX_ISR_DPLL_REF_CHGD_MASK_EN_SHIFT);

      mask |= (ZL303XX_ISR_DPLL_LOCKED_MASK_EN_MASK << ZL303XX_ISR_DPLL_LOCKED_MASK_EN_SHIFT) |
              (ZL303XX_ISR_DPLL_LOST_LOCK_MASK_EN_MASK << ZL303XX_ISR_DPLL_LOST_LOCK_MASK_EN_SHIFT) |
              (ZL303XX_ISR_DPLL_HOLDOVER_MASK_EN_MASK << ZL303XX_ISR_DPLL_HOLDOVER_MASK_EN_SHIFT) |
              (ZL303XX_ISR_DPLL_REF_CHGD_MASK_EN_MASK << ZL303XX_ISR_DPLL_REF_CHGD_MASK_EN_SHIFT);

      /* Write the Data for this Register */
      status = zl303xx_ReadModWrite(zl303xx_Params, NULL,
                                    ZL303XX_ISR_DPLL_MASK_REG(par->Id),
                                    regValue, mask, NULL);
   }

   return status;
} /* END zl303xx_DpllIsrConfigSet */

/**

  Function Name:
   zl303xx_IsrRefFailStatusGet

  Details:
   Query the REF Fail Status

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    refId          Associated REF Id
   [in]    val            The Ref Fail status of the specified reference.

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_IsrRefFailStatusGet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_ISR_REF_STATE_REG(refId),
                              ZL303XX_ISR_REF_FAIL_STATUS_MASK,
                              ZL303XX_ISR_REF_FAIL_STATUS_SHIFT(refId));
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_BooleanE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_IsrRefFailStatusGet */


/*

  Function Name:
   zl303xx_IsrDpllLockedStatusGet

  Details:
   Reads the Dpll Lock Status attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_IsrDpllLockedStatusGet(zl303xx_ParamsS *zl303xx_Params,
                                         zl303xx_DpllIdE dpllId,
                                         zl303xx_BooleanE *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
   }

   /* Check the dpllId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_DPLL_ID(dpllId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_ISR_DPLL_STATE_REG(dpllId),
                              ZL303XX_ISR_DPLL_LOCKED_STATUS_MASK,
                              ZL303XX_ISR_DPLL_LOCKED_STATUS_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_BooleanE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_IsrDpllLockedStatusGet */


/*

  Function Name:
   zl303xx_IsrDpllLostLockStatusGet

  Details:
   Reads the Dpll Lost Lock Status attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_IsrDpllLostLockStatusGet(zl303xx_ParamsS *zl303xx_Params,
                                           zl303xx_DpllIdE dpllId,
                                           zl303xx_BooleanE *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
   }

   /* Check the dpllId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_DPLL_ID(dpllId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_ISR_DPLL_STATE_REG(dpllId),
                              ZL303XX_ISR_DPLL_LOST_LOCK_STATUS_MASK,
                              ZL303XX_ISR_DPLL_LOST_LOCK_STATUS_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_BooleanE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_IsrDpllLostLockStatusGet */


/*

  Function Name:
   zl303xx_IsrDpllHoldoverStatusGet

  Details:
   Reads the Dpll Holdover Status attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_IsrDpllHoldoverStatusGet(zl303xx_ParamsS *zl303xx_Params,
                                           zl303xx_DpllIdE dpllId,
                                           zl303xx_BooleanE *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
   }

   /* Check the dpllId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_DPLL_ID(dpllId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_ISR_DPLL_STATE_REG(dpllId),
                              ZL303XX_ISR_DPLL_HOLDOVER_STATUS_MASK,
                              ZL303XX_ISR_DPLL_HOLDOVER_STATUS_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_BooleanE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_IsrDpllHoldoverStatusGet */


/*

  Function Name:
   zl303xx_IsrDpllRefChgdStatusGet

  Details:
   Reads the Dpll Ref Chnge Status attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_IsrDpllRefChgdStatusGet(zl303xx_ParamsS *zl303xx_Params,
                                          zl303xx_DpllIdE dpllId,
                                          zl303xx_BooleanE *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
   }

   /* Check the dpllId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_DPLL_ID(dpllId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_ISR_DPLL_STATE_REG(dpllId),
                              ZL303XX_ISR_DPLL_REF_CHGD_STATUS_MASK,
                              ZL303XX_ISR_DPLL_REF_CHGD_STATUS_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_BooleanE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_IsrDpllRefChgdStatusGet */


/**

  Function Name:
   zl303xx_IsrSyncFailStatusGet

  Details:
   Reads the Sync Fail Status attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    syncId         Associated SYNC Id
   [in]    val            The Sync Fail Status of the specified sync input

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_IsrSyncFailStatusGet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_ISR_SYNC_FAIL_STATUS_REG(syncId),
                              ZL303XX_ISR_SYNC_FAIL_STATUS_MASK,
                              ZL303XX_ISR_SYNC_FAIL_STATUS_SHIFT(syncId));
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_BooleanE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_IsrSyncFailStatusGet */


/**

  Function Name:
   zl303xx_IsrScmFailStatusGet

  Details:
   Query the SCM Fail Status

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    refId          Associated REF Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_IsrScmFailStatusGet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_ISR_MON_STATE_REG(refId),
                              ZL303XX_ISR_SCM_FAIL_STATUS_MASK,
                              ZL303XX_ISR_SCM_FAIL_STATUS_SHIFT(refId));
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_BooleanE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_IsrScmFailStatusGet */


/**

  Function Name:
   zl303xx_IsrCfmFailStatusGet

  Details:
   Query the CFM Fail Status

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    refId          Associated REF Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_IsrCfmFailStatusGet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_ISR_MON_STATE_REG(refId),
                              ZL303XX_ISR_CFM_FAIL_STATUS_MASK,
                              ZL303XX_ISR_CFM_FAIL_STATUS_SHIFT(refId));
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_BooleanE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_IsrCfmFailStatusGet */


/**

  Function Name:
   zl303xx_IsrGstFailStatusGet

  Details:
   Query the GST Fail Status

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    refId          Associated REF Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_IsrGstFailStatusGet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_ISR_MON_STATE_REG(refId),
                              ZL303XX_ISR_GST_FAIL_STATUS_MASK,
                              ZL303XX_ISR_GST_FAIL_STATUS_SHIFT(refId));
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_BooleanE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_IsrGstFailStatusGet */


/**

  Function Name:
   zl303xx_IsrPfmFailStatusGet

  Details:
   Query the PFM Fail Status

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    refId          Associated REF Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_IsrPfmFailStatusGet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_ISR_MON_STATE_REG(refId),
                              ZL303XX_ISR_PFM_FAIL_STATUS_MASK,
                              ZL303XX_ISR_PFM_FAIL_STATUS_SHIFT(refId));
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_BooleanE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_IsrPfmFailStatusGet */


/**

  Function Name:
   zl303xx_IsrRefFailMaskEnGet

  Details:
   Query the Ref Fail Mask Enable configuration

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    refId          Associated REF Id
   [in]    val            The current Ref Fail Mask enabled configuration

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_IsrRefFailMaskEnGet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_ISR_REF_MASK_REG(refId),
                              ZL303XX_ISR_REF_FAIL_MASK_EN_MASK,
                              ZL303XX_ISR_REF_FAIL_MASK_EN_SHIFT(refId));
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_BooleanE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_IsrRefFailMaskEnGet */

/**

  Function Name:
   zl303xx_IsrRefFailMaskEnSet

  Details:
   Writes the Ref Fail Mask Enable configuration

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    refId          Associated REF
   [in]    val            The requested Ref Fail Mask enabled configuration

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_IsrRefFailMaskEnSet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_ISR_REF_MASK_REG(refId),
                              ZL303XX_ISR_REF_FAIL_MASK_EN_MASK,
                              ZL303XX_ISR_REF_FAIL_MASK_EN_SHIFT(refId));
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_IsrRefFailMaskEnSet */


/*

  Function Name:
   zl303xx_IsrDpllLockedMaskEnGet

  Details:
   Reads the Dpll Lock Mask En attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_IsrDpllLockedMaskEnGet(zl303xx_ParamsS *zl303xx_Params,
                                         zl303xx_DpllIdE dpllId,
                                         zl303xx_BooleanE *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
   }

   /* Check the dpllId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_DPLL_ID(dpllId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_ISR_DPLL_MASK_REG(dpllId),
                              ZL303XX_ISR_DPLL_LOCKED_MASK_EN_MASK,
                              ZL303XX_ISR_DPLL_LOCKED_MASK_EN_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_BooleanE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_IsrDpllLockedMaskEnGet */

/*

  Function Name:
   zl303xx_IsrDpllLockedMaskEnSet

  Details:
   Writes the Dpll Lock Mask En attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_IsrDpllLockedMaskEnSet(zl303xx_ParamsS *zl303xx_Params,
                                         zl303xx_DpllIdE dpllId,
                                         zl303xx_BooleanE val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check params pointer */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
   }

   /* Check the dpllId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_DPLL_ID(dpllId);
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
                              ZL303XX_ISR_DPLL_MASK_REG(dpllId),
                              ZL303XX_ISR_DPLL_LOCKED_MASK_EN_MASK,
                              ZL303XX_ISR_DPLL_LOCKED_MASK_EN_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_IsrDpllLockedMaskEnSet */


/*

  Function Name:
   zl303xx_IsrDpllLostLockMaskEnGet

  Details:
   Reads the Dpll Lost Lock Mask En attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_IsrDpllLostLockMaskEnGet(zl303xx_ParamsS *zl303xx_Params,
                                           zl303xx_DpllIdE dpllId,
                                           zl303xx_BooleanE *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
   }

   /* Check the dpllId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_DPLL_ID(dpllId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_ISR_DPLL_MASK_REG(dpllId),
                              ZL303XX_ISR_DPLL_LOST_LOCK_MASK_EN_MASK,
                              ZL303XX_ISR_DPLL_LOST_LOCK_MASK_EN_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_BooleanE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_IsrDpllLostLockMaskEnGet */

/*

  Function Name:
   zl303xx_IsrDpllLostLockMaskEnSet

  Details:
   Writes the Dpll Lost Lock Mask En attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_IsrDpllLostLockMaskEnSet(zl303xx_ParamsS *zl303xx_Params,
                                           zl303xx_DpllIdE dpllId,
                                           zl303xx_BooleanE val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check params pointer */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
   }

   /* Check the dpllId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_DPLL_ID(dpllId);
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
                              ZL303XX_ISR_DPLL_MASK_REG(dpllId),
                              ZL303XX_ISR_DPLL_LOST_LOCK_MASK_EN_MASK,
                              ZL303XX_ISR_DPLL_LOST_LOCK_MASK_EN_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_IsrDpllLostLockMaskEnSet */


/*

  Function Name:
   zl303xx_IsrDpllHoldoverMaskEnGet

  Details:
   Reads the Dpll Holdover Mask En attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_IsrDpllHoldoverMaskEnGet(zl303xx_ParamsS *zl303xx_Params,
                                           zl303xx_DpllIdE dpllId,
                                           zl303xx_BooleanE *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
   }

   /* Check the dpllId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_DPLL_ID(dpllId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_ISR_DPLL_MASK_REG(dpllId),
                              ZL303XX_ISR_DPLL_HOLDOVER_MASK_EN_MASK,
                              ZL303XX_ISR_DPLL_HOLDOVER_MASK_EN_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_BooleanE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_IsrDpllHoldoverMaskEnGet */

/*

  Function Name:
   zl303xx_IsrDpllHoldoverMaskEnSet

  Details:
   Writes the Dpll Holdover Mask En attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_IsrDpllHoldoverMaskEnSet(zl303xx_ParamsS *zl303xx_Params,
                                           zl303xx_DpllIdE dpllId,
                                           zl303xx_BooleanE val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check params pointer */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
   }

   /* Check the dpllId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_DPLL_ID(dpllId);
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
                              ZL303XX_ISR_DPLL_MASK_REG(dpllId),
                              ZL303XX_ISR_DPLL_HOLDOVER_MASK_EN_MASK,
                              ZL303XX_ISR_DPLL_HOLDOVER_MASK_EN_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_IsrDpllHoldoverMaskEnSet */


/*

  Function Name:
   zl303xx_IsrDpllRefChgdMaskEnGet

  Details:
   Reads the Dpll Ref Chnge Mask En attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_IsrDpllRefChgdMaskEnGet(zl303xx_ParamsS *zl303xx_Params,
                                          zl303xx_DpllIdE dpllId,
                                          zl303xx_BooleanE *val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, val);
   }

   /* Check the dpllId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_DPLL_ID(dpllId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_ISR_DPLL_MASK_REG(dpllId),
                              ZL303XX_ISR_DPLL_REF_CHGD_MASK_EN_MASK,
                              ZL303XX_ISR_DPLL_REF_CHGD_MASK_EN_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_BooleanE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_IsrDpllRefChgdMaskEnGet */

/*

  Function Name:
   zl303xx_IsrDpllRefChgdMaskEnSet

  Details:
   Writes the Dpll Ref Chnge Mask En attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_IsrDpllRefChgdMaskEnSet(zl303xx_ParamsS *zl303xx_Params,
                                          zl303xx_DpllIdE dpllId,
                                          zl303xx_BooleanE val)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_AttrRdWrS attrPar;

   /* Check params pointer */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
   }

   /* Check the dpllId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_DPLL_ID(dpllId);
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
                              ZL303XX_ISR_DPLL_MASK_REG(dpllId),
                              ZL303XX_ISR_DPLL_REF_CHGD_MASK_EN_MASK,
                              ZL303XX_ISR_DPLL_REF_CHGD_MASK_EN_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_IsrDpllRefChgdMaskEnSet */


/**

  Function Name:
   zl303xx_IsrSyncFailMaskEnGet

  Details:
   Reads the Sync Fail Mask En attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    syncId         Associated SYNC Id
   [in]    val            The current Sync Fail Mask configuration

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_IsrSyncFailMaskEnGet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_ISR_SYNC_FAIL_MASK_EN_REG(syncId),
                              ZL303XX_ISR_SYNC_FAIL_MASK_EN_MASK,
                              ZL303XX_ISR_SYNC_FAIL_MASK_EN_SHIFT(syncId));
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_BooleanE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_IsrSyncFailMaskEnGet */

/**

  Function Name:
   zl303xx_IsrSyncFailMaskEnSet

  Details:
   Writes the Sync Fail Mask En attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    syncId         Associated SYNC Id of the attribute
   [in]    val            The requested Sync Fail Mask configuration

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_IsrSyncFailMaskEnSet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_ISR_SYNC_FAIL_MASK_EN_REG(syncId),
                              ZL303XX_ISR_SYNC_FAIL_MASK_EN_MASK,
                              ZL303XX_ISR_SYNC_FAIL_MASK_EN_SHIFT(syncId));
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_IsrSyncFailMaskEnSet */


/**

  Function Name:
   zl303xx_IsrScmFailMaskEnGet

  Details:
   Query the SCM Fail Mask Enable configuration

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    refId          Associated REF Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_IsrScmFailMaskEnGet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_ISR_MON_MASK_REG(refId),
                              ZL303XX_ISR_SCM_FAIL_MASK_EN_MASK,
                              ZL303XX_ISR_SCM_FAIL_MASK_EN_SHIFT(refId));
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_BooleanE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_IsrScmFailMaskEnGet */

/**

  Function Name:
   zl303xx_IsrScmFailMaskEnSet

  Details:
   Writes the SCM Fail Mask Enable configuration

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    refId          Associated REF Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_IsrScmFailMaskEnSet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_ISR_MON_MASK_REG(refId),
                              ZL303XX_ISR_SCM_FAIL_MASK_EN_MASK,
                              ZL303XX_ISR_SCM_FAIL_MASK_EN_SHIFT(refId));
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_IsrScmFailMaskEnSet */


/**

  Function Name:
   zl303xx_IsrCfmFailMaskEnGet

  Details:
   Query the CFM Fail Mask Enable configuration

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    refId          Associated REF Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_IsrCfmFailMaskEnGet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_ISR_MON_MASK_REG(refId),
                              ZL303XX_ISR_CFM_FAIL_MASK_EN_MASK,
                              ZL303XX_ISR_CFM_FAIL_MASK_EN_SHIFT(refId));
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_BooleanE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_IsrCfmFailMaskEnGet */

/**

  Function Name:
   zl303xx_IsrCfmFailMaskEnSet

  Details:
   Writes the CFM Fail Mask Enable configuration

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    refId          Associated REF Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_IsrCfmFailMaskEnSet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_ISR_MON_MASK_REG(refId),
                              ZL303XX_ISR_CFM_FAIL_MASK_EN_MASK,
                              ZL303XX_ISR_CFM_FAIL_MASK_EN_SHIFT(refId));
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_IsrCfmFailMaskEnSet */


/**

  Function Name:
   zl303xx_IsrGstFailMaskEnGet

  Details:
   Query the GST Fail Mask Enable configuration

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    refId          Associated REF Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_IsrGstFailMaskEnGet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_ISR_MON_MASK_REG(refId),
                              ZL303XX_ISR_GST_FAIL_MASK_EN_MASK,
                              ZL303XX_ISR_GST_FAIL_MASK_EN_SHIFT(refId));
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_BooleanE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_IsrGstFailMaskEnGet */

/**

  Function Name:
   zl303xx_IsrGstFailMaskEnSet

  Details:
   Writes the GST Fail Mask Enable configuration

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    refId          Associated REF Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_IsrGstFailMaskEnSet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_ISR_MON_MASK_REG(refId),
                              ZL303XX_ISR_GST_FAIL_MASK_EN_MASK,
                              ZL303XX_ISR_GST_FAIL_MASK_EN_SHIFT(refId));
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_IsrGstFailMaskEnSet */


/**

  Function Name:
   zl303xx_IsrPfmFailMaskEnGet

  Details:
   Query the PFM Fail Mask Enable configuration

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    refId          Associated REF Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_IsrPfmFailMaskEnGet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_ISR_MON_MASK_REG(refId),
                              ZL303XX_ISR_PFM_FAIL_MASK_EN_MASK,
                              ZL303XX_ISR_PFM_FAIL_MASK_EN_SHIFT(refId));
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_BooleanE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_IsrPfmFailMaskEnGet */

/**

  Function Name:
   zl303xx_IsrPfmFailMaskEnSet

  Details:
   Writes the PFM Fail Mask Enable configuration

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    refId          Associated REF Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_IsrPfmFailMaskEnSet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_ISR_MON_MASK_REG(refId),
                              ZL303XX_ISR_PFM_FAIL_MASK_EN_MASK,
                              ZL303XX_ISR_PFM_FAIL_MASK_EN_SHIFT(refId));
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_IsrPfmFailMaskEnSet */

/**

  Function Name:
   zl303xx_IsrGstScmMaskEnGet

  Details:
   Reads the GST-SCM Mask En attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    refId          Associated REF Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_IsrGstScmMaskEnGet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_ISR_GST_MASK_REG(refId),
                              ZL303XX_ISR_GST_SCM_MASK_EN_MASK,
                              ZL303XX_ISR_GST_SCM_MASK_EN_SHIFT(refId));
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_BooleanE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_IsrGstScmMaskEnGet */

/**

  Function Name:
   zl303xx_IsrGstScmMaskEnSet

  Details:
   Writes the GST-SCM Mask En attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    refId          Associated REF Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_IsrGstScmMaskEnSet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_ISR_GST_MASK_REG(refId),
                              ZL303XX_ISR_GST_SCM_MASK_EN_MASK,
                              ZL303XX_ISR_GST_SCM_MASK_EN_SHIFT(refId));
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_IsrGstScmMaskEnSet */


/**

  Function Name:
   zl303xx_IsrGstCfmMaskEnGet

  Details:
   Reads the GST-CFM Mask En attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    refId          Associated REF Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_IsrGstCfmMaskEnGet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_ISR_GST_MASK_REG(refId),
                              ZL303XX_ISR_GST_CFM_MASK_EN_MASK,
                              ZL303XX_ISR_GST_CFM_MASK_EN_SHIFT(refId));
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_BooleanE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_IsrGstCfmMaskEnGet */

/**

  Function Name:
   zl303xx_IsrGstCfmMaskEnSet

  Details:
   Writes the GST-CFM Mask En attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    refId          Associated REF Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_IsrGstCfmMaskEnSet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_ISR_GST_MASK_REG(refId),
                              ZL303XX_ISR_GST_CFM_MASK_EN_MASK,
                              ZL303XX_ISR_GST_CFM_MASK_EN_SHIFT(refId));
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_IsrGstCfmMaskEnSet */

