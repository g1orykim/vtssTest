

/*******************************************************************************
*
*  $Id: zl303xx_DpllLow.c 8253 2012-05-23 17:59:02Z PC $
*
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     Functions for low-level DPLL attribute access
*
*******************************************************************************/

/*****************   INCLUDE FILES   ******************************************/

#include "zl303xx_ApiLow.h"
#include "zl303xx_DpllLow.h"
#include "zl303xx_IsrLow.h"
#include "zl303xx_RefLow.h"

/*****************   DEFINES     **********************************************/

/*****************   STATIC FUNCTION DECLARATIONS   ***************************/
static Sint32T zl303xx_DpllHwModeToBoolean(void *hwParams, zl303xx_DpllModeE modeCheck,
                                           Sint32T *matched);

/*****************   STATIC GLOBAL VARIABLES   ********************************/

/*****************   EXPORTED GLOBAL VARIABLES   ******************************/

/*****************   EXPORTED FUNCTION DEFINITIONS   **************************/

/*

  Function Name:
   zl303xx_DpllConfigStructInit

  Details:
   Initialises the parameter structure for the zl303xx_PllConfig function

  Parameters:
   [in]  zl303xx_Params Pointer to the device instance parameter structure
   [in]    par      Pointer to zl303xx_DpllConfigS parameter structure
            See main function for parameter descriptions

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllConfigStructInit(zl303xx_ParamsS *zl303xx_Params, zl303xx_DpllConfigS *par)
{
   zlStatusE status = ZL303XX_OK;

   ZL303XX_TRACE(ZL303XX_MOD_ID_PLL, 2,
         "zl303xx_DpllConfigStructInit", 0, 0, 0, 0, 0, 0);

   /* Warning removal. */
   if (zl303xx_Params) {;}

   /* Check the function input Parameter Pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(par);
   }

   /* Set Invalid values for now. When using this structure, it might be useful
      to query the DPLL configuration first, modify the required members, and
      then send the new configuration back to the DPLL */
   if (status == ZL303XX_OK)
   {
      /* The dpllId must be specified whenever this structure is used */
      par->Id = (zl303xx_DpllIdE)ZL303XX_INVALID;

      /* Only DPLL2 can be disabled. The PllConfig function will screen this */
      par->enable = (zl303xx_BooleanE)ZL303XX_INVALID;

      par->mode = (zl303xx_DpllModeE)ZL303XX_INVALID;
      par->hitlessSw = (zl303xx_BooleanE)ZL303XX_INVALID;
      par->bandwidth = (zl303xx_DpllBwE)ZL303XX_INVALID;
      par->phaseSlope = (zl303xx_DpllPslE)ZL303XX_INVALID;
      par->revertEn = (zl303xx_BooleanE)ZL303XX_INVALID;
      par->dcoOffsetEnable = (zl303xx_BooleanE)ZL303XX_INVALID;
      par->dcoFilterEnable = (zl303xx_BooleanE)ZL303XX_INVALID;
      par->hldUpdateTime = (zl303xx_DpllHldUpdateE)ZL303XX_INVALID;
      par->hldFilterBw = (zl303xx_DpllHldFilterE)ZL303XX_INVALID;
      par->selectedRef = (zl303xx_RefIdE)ZL303XX_INVALID;
      par->waitToRestore = (Uint32T)ZL303XX_INVALID;
      par->pullInRange = (zl303xx_DpllPullInE)ZL303XX_INVALID;
   }

   return status;
}  /* END zl303xx_DpllConfigStructInit */

/*

  Function Name:
   zl303xx_DpllConfigCheck

  Details:
   Function to check for INVALID data in the zl303xx_DpllConfig structure.
   Refer to the zl303xx_DpllConfigS data type for the member descriptions.

  Parameters:
   [in]  zl303xx_Params Pointer to the device instance parameter structure
   [in]    par      Pointer to zl303xx_DpllConfigS parameter structure to check

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllConfigCheck(zl303xx_ParamsS *zl303xx_Params, zl303xx_DpllConfigS *par)
{
   zlStatusE status = ZL303XX_OK;

   ZL303XX_TRACE(ZL303XX_MOD_ID_PLL, 2,
         "zl303xx_DpllConfigCheck", 0, 0, 0, 0, 0, 0);

   /* Warning removal. */
   if (zl303xx_Params) {;}

   /* Check the function input Parameter Pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(par);
   }

   /* Check each member of the zl303xx_DpllConfigS structure */

   /* Since some members are only applicable to one DPLL or the other, verify
      the DPLL Id first */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_DPLL_ID(par->Id);
   }

   /* Check the common members */
   if (status == ZL303XX_OK)
   {
      /* We check mode here for the full range (including ToP but if this is
         Dpll 2 data, we screen for ToP mode in the Dpll2 specific checks */
      status = ZL303XX_CHECK_DPLL_MODE(par->mode) |
               ZL303XX_CHECK_BOOLEAN(par->hitlessSw) |
               ZL303XX_CHECK_BOOLEAN(par->revertEn) |
               ZL303XX_CHECK_BOOLEAN(par->dcoOffsetEnable) |
               ZL303XX_CHECK_BOOLEAN(par->dcoFilterEnable) |
               ZL303XX_CHECK_REF_ID(par->selectedRef) |
               ZL303XX_CHECK_DPLL_WAIT_TO_RESTORE(par->waitToRestore) |
               ZL303XX_CHECK_DPLL_PSL(par->Id,par->phaseSlope);
   }

   /* Check Dpll specific members */
   if (status == ZL303XX_OK)
   {
      if (par->Id == ZL303XX_DPLL_ID_1)
      {
         status = ZL303XX_CHECK_DPLL_BW(par->bandwidth) |
                  ZL303XX_CHECK_DPLL_HLD_UPDATE(par->hldUpdateTime) |
                  ZL303XX_CHECK_DPLL_HLD_FILTER(par->hldFilterBw) |
                  ZL303XX_CHECK_DPLL_PULL_IN(par->pullInRange);

         /* Dpll 1 can only be Enabled */
         if (status == ZL303XX_OK)
         {
            if (par->enable == ZL303XX_FALSE)
            {
               status = ZL303XX_PARAMETER_INVALID;
            }
         }
      }
      else /* DPLL 2 */
      {
         status = ZL303XX_CHECK_BOOLEAN(par->enable);

         if (status == ZL303XX_OK)
         {
            if ((par->mode == ZL303XX_DPLL_MODE_TOP) ||
                (par->bandwidth != ZL303XX_DPLL_BW_14Hz) ||
                (par->hldUpdateTime != ZL303XX_DPLL_HLD_UPD_26mS) ||
                (par->hldFilterBw != (zl303xx_DpllHldUpdateE)ZL303XX_INVALID) ||
                (par->pullInRange != ZL303XX_DPLL_PULLIN_130PPM))
            {
               status = ZL303XX_PARAMETER_INVALID;
            }
         }
      }
   }

   return status;
}  /* END zl303xx_DpllConfigCheck */

/*

  Function Name:
   zl303xx_DpllConfigGet

  Details:
   Gets the members of the zl303xx_DpllConfigS data structure

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    par            Pointer to the zl303xx_DpllConfigS structure to Get

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllConfigGet(zl303xx_ParamsS *zl303xx_Params,
                                zl303xx_DpllConfigS *par)
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
                            ZL303XX_DPLL_CONTROL_REG(par->Id),
                            &regValue);
   }

   /* Extract */
   if (status == ZL303XX_OK)
   {
      par->hitlessSw = (zl303xx_BooleanE)ZL303XX_EXTRACT(regValue,
                                         ZL303XX_DPLL_HITLESS_MASK,
                                         ZL303XX_DPLL_HITLESS_SHIFT);

      if (par->Id == ZL303XX_DPLL_ID_1)
      {
          par->bandwidth = (zl303xx_DpllBwE)ZL303XX_EXTRACT(regValue,
                                             ZL303XX_DPLL_BANDWIDTH_MASK,
                                             ZL303XX_DPLL_BANDWIDTH_SHIFT);

          if ((par->bandwidth & ZL303XX_DPLL_NON_S3E_BANDWIDTH_MASK) != ZL303XX_DPLL_BW_S3E_0P3mHZ)
          {
              par->bandwidth &= ZL303XX_DPLL_NON_S3E_BANDWIDTH_MASK;
          }
      }
      else
      {
          par->bandwidth = ZL303XX_DPLL_BW_14Hz;
      }

      par->phaseSlope = (zl303xx_DpllPslE)ZL303XX_EXTRACT(regValue,
                              ZL303XX_DPLL_PHASE_LIMIT_MASK(par->Id),
                                           ZL303XX_DPLL_PHASE_LIMIT_SHIFT);

      if (par->Id == ZL303XX_DPLL_ID_2)
      {
         par->phaseSlope += ZL303XX_DPLL_PSL_61US;
      }

      par->revertEn = (zl303xx_BooleanE)ZL303XX_EXTRACT(regValue,
                                         ZL303XX_DPLL_REVERTIVE_EN_MASK,
                                         ZL303XX_DPLL_REVERTIVE_EN_SHIFT);

      par->dcoOffsetEnable = (zl303xx_BooleanE)ZL303XX_EXTRACT(regValue,
                                                ZL303XX_DPLL_DCO_OFFSET_EN_MASK,
                                                ZL303XX_DPLL_DCO_OFFSET_EN_SHIFT);

      par->dcoFilterEnable = (zl303xx_BooleanE)ZL303XX_EXTRACT(regValue,
                                                ZL303XX_DPLL_DCO_FILTER_EN_MASK,
                                                ZL303XX_DPLL_DCO_FILTER_EN_SHIFT);

      if (par->Id == ZL303XX_DPLL_ID_1)
      {
          par->hldUpdateTime = (zl303xx_DpllHldUpdateE)ZL303XX_EXTRACT(regValue,
                                                        ZL303XX_DPLL_HOLD_UPDATE_TIME_MASK,
                                                        ZL303XX_DPLL_HOLD_UPDATE_TIME_SHIFT);


          par->hldFilterBw = (zl303xx_DpllHldFilterE)ZL303XX_EXTRACT(regValue,
                                                      ZL303XX_DPLL_HOLD_FILTER_BW_MASK,
                                                      ZL303XX_DPLL_HOLD_FILTER_BW_SHIFT);
      }
      else
      {
          par->hldUpdateTime = ZL303XX_DPLL_HLD_UPD_26mS;
          par->hldFilterBw = ZL303XX_DPLL_HLD_FILT_BYPASS;
      }

      if (par->Id == ZL303XX_DPLL_ID_2)
      {
          par->enable = (zl303xx_BooleanE)ZL303XX_EXTRACT(regValue,
                                           ZL303XX_DPLL_ENABLE_MASK,
                                           ZL303XX_DPLL_ENABLE_SHIFT);
      }
      else
      {
          par->enable = ZL303XX_TRUE;
      }
   }

   /* Read */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_Read(zl303xx_Params, NULL,
                            ZL303XX_DPLL_MODE_REG(par->Id),
                            &regValue);
   }

   /* Extract */
   if (status == ZL303XX_OK)
   {
      par->mode = (zl303xx_DpllModeE)ZL303XX_EXTRACT(regValue,
                                     ZL303XX_DPLL_MODE_SELECT_MASK,
                                     ZL303XX_DPLL_MODE_SELECT_SHIFT);
   }

   /* Read */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_Read(zl303xx_Params, NULL,
                            ZL303XX_DPLL_REF_SELECT_REG(par->Id),
                            &regValue);
   }

   /* Extract */
   if (status == ZL303XX_OK)
   {
      par->selectedRef = (zl303xx_RefIdE)ZL303XX_EXTRACT(regValue,
                                         ZL303XX_DPLL_REF_SELECT_MASK,
                                         ZL303XX_DPLL_REF_SELECT_SHIFT);
   }

   /* Read */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_Read(zl303xx_Params, NULL,
                            ZL303XX_DPLL_WAIT_TO_RES_REG(par->Id),
                            &regValue);
   }

   /* Extract */
   if (status == ZL303XX_OK)
   {
      par->waitToRestore = (Uint32T)ZL303XX_EXTRACT(regValue,
                                    ZL303XX_DPLL_WAIT_TO_RES_MASK,
                                    ZL303XX_DPLL_WAIT_TO_RES_SHIFT);
   }

   /* Read */
   if (par->Id == ZL303XX_DPLL_ID_1)
   {
       if (status == ZL303XX_OK)
       {
          status = zl303xx_Read(zl303xx_Params, NULL,
                                ZL303XX_DPLL1_PULLIN_RANGE_REG,
                                &regValue);
       }

       /* Extract */
       if (status == ZL303XX_OK)
       {
          par->pullInRange = (zl303xx_DpllPullInE)ZL303XX_EXTRACT(regValue,
                                                  ZL303XX_DPLL_PULLIN_RANGE_MASK,
                                                  ZL303XX_DPLL_PULLIN_RANGE_SHIFT);
       }
   }
   else
   {
       par->pullInRange = ZL303XX_DPLL_PULLIN_130PPM;
   }

   /* Update the device parameters structure */
   if (status == ZL303XX_OK)
   {
      zl303xx_Params->pllParams.config[par->Id] = *par;
   }

   return status;
} /* END zl303xx_DpllConfigGet */

/*

  Function Name:
   zl303xx_DpllConfigSet

  Details:
   Sets the members of the zl303xx_DpllConfigS data structure

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    par            Pointer to the zl303xx_DpllConfigS structure to Set

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllConfigSet(zl303xx_ParamsS *zl303xx_Params,
                                zl303xx_DpllConfigS *par)
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
      status = zl303xx_DpllConfigCheck(zl303xx_Params, par);
   }

   /* Package */
   if (status == ZL303XX_OK)
   {
      regValue = 0;
      mask = 0;

      /* hitlessSw */
      ZL303XX_INSERT(regValue, par->hitlessSw,
                             ZL303XX_DPLL_HITLESS_MASK,
                             ZL303XX_DPLL_HITLESS_SHIFT);

      /* bandwidth */
      ZL303XX_INSERT(regValue, par->bandwidth,
                             ZL303XX_DPLL_BANDWIDTH_MASK,
                             ZL303XX_DPLL_BANDWIDTH_SHIFT);

      /* phaseSlope */
      ZL303XX_INSERT(regValue, par->phaseSlope,
                             ZL303XX_DPLL_PHASE_LIMIT_MASK(par->Id),
                             ZL303XX_DPLL_PHASE_LIMIT_SHIFT);

      /* revertEn */
      ZL303XX_INSERT(regValue, par->revertEn,
                             ZL303XX_DPLL_REVERTIVE_EN_MASK,
                             ZL303XX_DPLL_REVERTIVE_EN_SHIFT);

      /* dcoOffsetEnable */
      ZL303XX_INSERT(regValue, par->dcoOffsetEnable,
                             ZL303XX_DPLL_DCO_OFFSET_EN_MASK,
                             ZL303XX_DPLL_DCO_OFFSET_EN_SHIFT);

      /* dcoFilterEnable */
      ZL303XX_INSERT(regValue, par->dcoFilterEnable,
                             ZL303XX_DPLL_DCO_FILTER_EN_MASK,
                             ZL303XX_DPLL_DCO_FILTER_EN_SHIFT);

      /* hldUpdateTime */
      ZL303XX_INSERT(regValue, par->hldUpdateTime,
                             ZL303XX_DPLL_HOLD_UPDATE_TIME_MASK,
                             ZL303XX_DPLL_HOLD_UPDATE_TIME_SHIFT);

      /* hldFilterBw */
      ZL303XX_INSERT(regValue, par->hldFilterBw,
                             ZL303XX_DPLL_HOLD_FILTER_BW_MASK,
                             ZL303XX_DPLL_HOLD_FILTER_BW_SHIFT);

      mask |= (ZL303XX_DPLL_HITLESS_MASK << ZL303XX_DPLL_HITLESS_SHIFT) |
              (ZL303XX_DPLL_BANDWIDTH_MASK << ZL303XX_DPLL_BANDWIDTH_SHIFT) |
              (ZL303XX_DPLL_PHASE_LIMIT_MASK(par->Id) << ZL303XX_DPLL_PHASE_LIMIT_SHIFT) |
              (ZL303XX_DPLL_REVERTIVE_EN_MASK << ZL303XX_DPLL_REVERTIVE_EN_SHIFT) |
              (ZL303XX_DPLL_DCO_OFFSET_EN_MASK << ZL303XX_DPLL_DCO_OFFSET_EN_SHIFT) |
              (ZL303XX_DPLL_DCO_FILTER_EN_MASK << ZL303XX_DPLL_DCO_FILTER_EN_SHIFT) |
              (ZL303XX_DPLL_HOLD_UPDATE_TIME_MASK << ZL303XX_DPLL_HOLD_UPDATE_TIME_SHIFT) |
              (ZL303XX_DPLL_HOLD_FILTER_BW_MASK << ZL303XX_DPLL_HOLD_FILTER_BW_SHIFT);

      /* enable */
      if (par->Id == ZL303XX_DPLL_ID_2)
      {
          ZL303XX_INSERT(regValue, par->enable,
                                 ZL303XX_DPLL_ENABLE_MASK,
                                 ZL303XX_DPLL_ENABLE_SHIFT);

          mask |= (ZL303XX_DPLL_ENABLE_MASK << ZL303XX_DPLL_ENABLE_SHIFT);
      }

      /* Write the Data for this Register */
      status = zl303xx_ReadModWrite(zl303xx_Params, NULL, ZL303XX_DPLL_CONTROL_REG(par->Id), regValue, mask, NULL);
   }

   /* Package */
   if (status == ZL303XX_OK)
   {
      regValue = 0;
      mask = 0;

      /* mode */
      ZL303XX_INSERT(regValue, par->mode,
                             ZL303XX_DPLL_MODE_SELECT_MASK,
                             ZL303XX_DPLL_MODE_SELECT_SHIFT);

      mask |= (ZL303XX_DPLL_MODE_SELECT_MASK << ZL303XX_DPLL_MODE_SELECT_SHIFT);

      /* Write the Data for this Register */
      status = zl303xx_ReadModWrite(zl303xx_Params, NULL,  ZL303XX_DPLL_MODE_REG(par->Id), regValue, mask, NULL);
   }

   /* Package */
   if (status == ZL303XX_OK)
   {
      regValue = 0;
      mask = 0;

      /* selectedRef */
      ZL303XX_INSERT(regValue, par->selectedRef,
                             ZL303XX_DPLL_REF_SELECT_MASK,
                             ZL303XX_DPLL_REF_SELECT_SHIFT);

      mask |= (ZL303XX_DPLL_REF_SELECT_MASK << ZL303XX_DPLL_REF_SELECT_SHIFT);

      /* Write the Data for this Register */
      status = zl303xx_ReadModWrite(zl303xx_Params, NULL, ZL303XX_DPLL_REF_SELECT_REG(par->Id), regValue, mask, NULL);

      if (status == ZL303XX_OK)
      {
         zl303xx_Params->pllParams.config[par->Id].selectedRef = par->selectedRef;
      }
   }

   /* Package */
   if (status == ZL303XX_OK)
   {
      regValue = 0;
      mask = 0;

      /* waitToRestore */
      ZL303XX_INSERT(regValue, par->waitToRestore,
                             ZL303XX_DPLL_WAIT_TO_RES_MASK,
                             ZL303XX_DPLL_WAIT_TO_RES_SHIFT);

      mask |= (ZL303XX_DPLL_WAIT_TO_RES_MASK << ZL303XX_DPLL_WAIT_TO_RES_SHIFT);

      /* Write the Data for this Register */
      status = zl303xx_ReadModWrite(zl303xx_Params, NULL, ZL303XX_DPLL_WAIT_TO_RES_REG(par->Id), regValue, mask, NULL);
   }

   /* Package */
   if (par->Id == ZL303XX_DPLL_ID_1)
   {
       if (status == ZL303XX_OK)
       {
          regValue = 0;
          mask = 0;

          /* pullInRange */
          ZL303XX_INSERT(regValue, par->pullInRange,
                                 ZL303XX_DPLL_PULLIN_RANGE_MASK,
                                 ZL303XX_DPLL_PULLIN_RANGE_SHIFT);

          mask |= (ZL303XX_DPLL_PULLIN_RANGE_MASK << ZL303XX_DPLL_PULLIN_RANGE_SHIFT);

          /* Write the Data for this Register */
          status = zl303xx_ReadModWrite(zl303xx_Params, NULL, ZL303XX_DPLL1_PULLIN_RANGE_REG, regValue, mask, NULL);
       }
   }

   /* Update the device parameters structure */
   if (status == ZL303XX_OK)
   {
      zl303xx_Params->pllParams.config[par->Id] = *par;
   }

   return status;
} /* END zl303xx_DpllConfigSet */

/*

  Function Name:
   zl303xx_DpllStatusStructInit

  Details:
   Initializes the members of the zl303xx_DpllStatusS data structure

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    par            Pointer to the zl303xx_DpllStatusS structure to Initialize

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllStatusStructInit(zl303xx_ParamsS *zl303xx_Params,
                                       zl303xx_DpllStatusS *par)
{
   zlStatusE status = ZL303XX_OK;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, par);
   }

   if (status == ZL303XX_OK)
   {
      par->holdover = (zl303xx_DpllHoldStateE)ZL303XX_INVALID;
      par->locked = (zl303xx_DpllLockStateE)ZL303XX_INVALID;
      par->refFailed = (zl303xx_BooleanE)ZL303XX_INVALID;
      par->Id = (zl303xx_DpllIdE)ZL303XX_INVALID;
   }

   return status;
} /* END zl303xx_DpllStatusStructInit */

/*

  Function Name:
   zl303xx_DpllStatusCheck

  Details:
   Checks the members of the zl303xx_DpllStatusS data structure

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    par            Pointer to the zl303xx_DpllStatusS structure to Check

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllStatusCheck(zl303xx_ParamsS *zl303xx_Params,
                                  zl303xx_DpllStatusS *par)
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
      status = ZL303XX_CHECK_DPLL_HOLD_STATE(par->holdover) |
               ZL303XX_CHECK_DPLL_LOCK_STATE(par->locked) |
               ZL303XX_CHECK_BOOLEAN(par->refFailed) |
               ZL303XX_CHECK_DPLL_ID(par->Id);
   }

   return status;
} /* END zl303xx_DpllStatusCheck */

/*

  Function Name:
   zl303xx_DpllStatusGet

  Details:
   Gets the members of the zl303xx_DpllStatusS data structure

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    par            Pointer to the zl303xx_DpllStatusS structure to Get

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllStatusGet(zl303xx_ParamsS *zl303xx_Params,
                                zl303xx_DpllStatusS *par)
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
                            ZL303XX_DPLL_LOCK_STATUS_REG(par->Id),
                            &regValue);
   }

   /* Extract */
   if (status == ZL303XX_OK)
   {
      par->holdover = (zl303xx_DpllHoldStateE)ZL303XX_EXTRACT(regValue,
                                              ZL303XX_DPLL_HOLDOVER_STATUS_MASK,
                                              ZL303XX_DPLL_HOLDOVER_STATUS_SHIFT);

      par->locked = (zl303xx_DpllLockStateE)ZL303XX_EXTRACT(regValue,
                                             ZL303XX_DPLL_LOCK_STATUS_MASK,
                                             ZL303XX_DPLL_LOCK_STATUS_SHIFT);

      par->refFailed = (zl303xx_BooleanE)ZL303XX_EXTRACT(regValue,
                                          ZL303XX_DPLL_CUR_REF_FAIL_STATUS_MASK,
                                          ZL303XX_DPLL_CUR_REF_FAIL_STATUS_SHIFT);
   }

   return status;
} /* END zl303xx_DpllStatusGet */

/*

  Function Name:
   zl303xx_DpllMaskConfigStructInit

  Details:
   Initializes the members of the zl303xx_DpllMaskConfigS data structure

  Parameters:
   [in]   zl303xx_Params Pointer to the device instance parameter structure
   [in]   par            Pointer to the zl303xx_DpllMaskConfigS structure to Initialize

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllMaskConfigStructInit(zl303xx_ParamsS *zl303xx_Params,
                                           zl303xx_DpllMaskConfigS *par)
{
   zlStatusE status = ZL303XX_OK;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, par);
   }

   if (status == ZL303XX_OK)
   {
      par->scmRefSwEn = (zl303xx_BooleanE)ZL303XX_INVALID;
      par->cfmRefSwEn = (zl303xx_BooleanE)ZL303XX_INVALID;
      par->gstRefSwEn = (zl303xx_BooleanE)ZL303XX_INVALID;
      par->pfmRefSwEn = (zl303xx_BooleanE)ZL303XX_INVALID;
      par->scmHoldoverEn = (zl303xx_BooleanE)ZL303XX_INVALID;
      par->cfmHoldoverEn = (zl303xx_BooleanE)ZL303XX_INVALID;
      par->gstHoldoverEn = (zl303xx_BooleanE)ZL303XX_INVALID;
      par->pfmHoldoverEn = (zl303xx_BooleanE)ZL303XX_INVALID;
      par->Id = (zl303xx_DpllIdE)ZL303XX_INVALID;
   }

   return status;
} /* END zl303xx_DpllMaskConfigStructInit */

/*

  Function Name:
   zl303xx_DpllMaskConfigCheck

  Details:
   Checks the members of the zl303xx_DpllMaskConfigS data structure

  Parameters:
   [in]   zl303xx_Params Pointer to the device instance parameter structure
   [in]   par            Pointer to the zl303xx_DpllMaskConfigS structure to Check

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllMaskConfigCheck(zl303xx_ParamsS *zl303xx_Params,
                                      zl303xx_DpllMaskConfigS *par)
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
      status = ZL303XX_CHECK_BOOLEAN(par->scmRefSwEn) |
               ZL303XX_CHECK_BOOLEAN(par->cfmRefSwEn) |
               ZL303XX_CHECK_BOOLEAN(par->gstRefSwEn) |
               ZL303XX_CHECK_BOOLEAN(par->pfmRefSwEn) |
               ZL303XX_CHECK_BOOLEAN(par->scmHoldoverEn) |
               ZL303XX_CHECK_BOOLEAN(par->cfmHoldoverEn) |
               ZL303XX_CHECK_BOOLEAN(par->gstHoldoverEn) |
               ZL303XX_CHECK_BOOLEAN(par->pfmHoldoverEn) |
               ZL303XX_CHECK_DPLL_ID(par->Id);
   }

   return status;
} /* END zl303xx_DpllMaskConfigCheck */

/*

  Function Name:
   zl303xx_DpllMaskConfigGet

  Details:
   Gets the members of the zl303xx_DpllMaskConfigS data structure

  Parameters:
   [in]   zl303xx_Params Pointer to the device instance parameter structure
   [in]   par            Pointer to the zl303xx_DpllMaskConfigS structure to Get

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllMaskConfigGet(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_DpllMaskConfigS *par)
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
                            ZL303XX_DPLL_REF_FAIL_MASK_REG(par->Id),
                            &regValue);
   }

   /* Extract */
   if (status == ZL303XX_OK)
   {
      par->scmRefSwEn = (zl303xx_BooleanE)ZL303XX_EXTRACT(regValue,
                                          ZL303XX_DPLL_SCM_REF_SW_MASK_EN_MASK,
                                          ZL303XX_DPLL_SCM_REF_SW_MASK_EN_SHIFT);

      par->cfmRefSwEn = (zl303xx_BooleanE)ZL303XX_EXTRACT(regValue,
                                           ZL303XX_DPLL_CFM_REF_SW_MASK_EN_MASK,
                                           ZL303XX_DPLL_CFM_REF_SW_MASK_EN_SHIFT);

      par->gstRefSwEn = (zl303xx_BooleanE)ZL303XX_EXTRACT(regValue,
                                           ZL303XX_DPLL_GST_REF_SW_MASK_EN_MASK,
                                           ZL303XX_DPLL_GST_REF_SW_MASK_EN_SHIFT);

      par->pfmRefSwEn = (zl303xx_BooleanE)ZL303XX_EXTRACT(regValue,
                                           ZL303XX_DPLL_PFM_REF_SW_MASK_EN_MASK,
                                           ZL303XX_DPLL_PFM_REF_SW_MASK_EN_SHIFT);

      par->scmHoldoverEn = (zl303xx_BooleanE)ZL303XX_EXTRACT(regValue,
                                              ZL303XX_DPLL_SCM_HOLD_MASK_EN_MASK,
                                              ZL303XX_DPLL_SCM_HOLD_MASK_EN_SHIFT);

      par->cfmHoldoverEn = (zl303xx_BooleanE)ZL303XX_EXTRACT(regValue,
                                              ZL303XX_DPLL_CFM_HOLD_MASK_EN_MASK,
                                              ZL303XX_DPLL_CFM_HOLD_MASK_EN_SHIFT);

      par->gstHoldoverEn = (zl303xx_BooleanE)ZL303XX_EXTRACT(regValue,
                                              ZL303XX_DPLL_GST_HOLD_MASK_EN_MASK,
                                              ZL303XX_DPLL_GST_HOLD_MASK_EN_SHIFT);

      par->pfmHoldoverEn = (zl303xx_BooleanE)ZL303XX_EXTRACT(regValue,
                                              ZL303XX_DPLL_PFM_HOLD_MASK_EN_MASK,
                                              ZL303XX_DPLL_PFM_HOLD_MASK_EN_SHIFT);
   }

   return status;
} /* END zl303xx_DpllMaskConfigGet */

/*

  Function Name:
   zl303xx_DpllMaskConfigSet

  Details:
   Sets the members of the zl303xx_DpllMaskConfigS data structure

  Parameters:
   [in]   zl303xx_Params Pointer to the device instance parameter structure
   [in]   par            Pointer to the zl303xx_DpllMaskConfigS structure to Set

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllMaskConfigSet(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_DpllMaskConfigS *par)
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
      status = zl303xx_DpllMaskConfigCheck(zl303xx_Params, par);
   }

   /* Package */
   if (status == ZL303XX_OK)
   {
      regValue = 0;
      mask = 0;

      /* scmRefSwEn */
      ZL303XX_INSERT(regValue, par->scmRefSwEn,
                             ZL303XX_DPLL_SCM_REF_SW_MASK_EN_MASK,
                             ZL303XX_DPLL_SCM_REF_SW_MASK_EN_SHIFT);

      /* cfmRefSwEn */
      ZL303XX_INSERT(regValue, par->cfmRefSwEn,
                             ZL303XX_DPLL_CFM_REF_SW_MASK_EN_MASK,
                             ZL303XX_DPLL_CFM_REF_SW_MASK_EN_SHIFT);

      /* gstRefSwEn */
      ZL303XX_INSERT(regValue, par->gstRefSwEn,
                             ZL303XX_DPLL_GST_REF_SW_MASK_EN_MASK,
                             ZL303XX_DPLL_GST_REF_SW_MASK_EN_SHIFT);

      /* pfmRefSwEn */
      ZL303XX_INSERT(regValue, par->pfmRefSwEn,
                             ZL303XX_DPLL_PFM_REF_SW_MASK_EN_MASK,
                             ZL303XX_DPLL_PFM_REF_SW_MASK_EN_SHIFT);

      /* scmHoldoverEn */
      ZL303XX_INSERT(regValue, par->scmHoldoverEn,
                             ZL303XX_DPLL_SCM_HOLD_MASK_EN_MASK,
                             ZL303XX_DPLL_SCM_HOLD_MASK_EN_SHIFT);

      /* cfmHoldoverEn */
      ZL303XX_INSERT(regValue, par->cfmHoldoverEn,
                             ZL303XX_DPLL_CFM_HOLD_MASK_EN_MASK,
                             ZL303XX_DPLL_CFM_HOLD_MASK_EN_SHIFT);

      /* gstHoldoverEn */
      ZL303XX_INSERT(regValue, par->gstHoldoverEn,
                             ZL303XX_DPLL_GST_HOLD_MASK_EN_MASK,
                             ZL303XX_DPLL_GST_HOLD_MASK_EN_SHIFT);

      /* pfmHoldoverEn */
      ZL303XX_INSERT(regValue, par->pfmHoldoverEn,
                             ZL303XX_DPLL_PFM_HOLD_MASK_EN_MASK,
                             ZL303XX_DPLL_PFM_HOLD_MASK_EN_SHIFT);

      mask |= (ZL303XX_DPLL_SCM_REF_SW_MASK_EN_MASK << ZL303XX_DPLL_SCM_REF_SW_MASK_EN_SHIFT) |
              (ZL303XX_DPLL_CFM_REF_SW_MASK_EN_MASK << ZL303XX_DPLL_CFM_REF_SW_MASK_EN_SHIFT) |
              (ZL303XX_DPLL_GST_REF_SW_MASK_EN_MASK << ZL303XX_DPLL_GST_REF_SW_MASK_EN_SHIFT) |
              (ZL303XX_DPLL_PFM_REF_SW_MASK_EN_MASK << ZL303XX_DPLL_PFM_REF_SW_MASK_EN_SHIFT) |
              (ZL303XX_DPLL_SCM_HOLD_MASK_EN_MASK << ZL303XX_DPLL_SCM_HOLD_MASK_EN_SHIFT) |
              (ZL303XX_DPLL_CFM_HOLD_MASK_EN_MASK << ZL303XX_DPLL_CFM_HOLD_MASK_EN_SHIFT) |
              (ZL303XX_DPLL_GST_HOLD_MASK_EN_MASK << ZL303XX_DPLL_GST_HOLD_MASK_EN_SHIFT) |
              (ZL303XX_DPLL_PFM_HOLD_MASK_EN_MASK << ZL303XX_DPLL_PFM_HOLD_MASK_EN_SHIFT);

      /* Write the Data for this Register */
      status = zl303xx_ReadModWrite(zl303xx_Params, NULL,
                                    ZL303XX_DPLL_REF_FAIL_MASK_REG(par->Id),
                                    regValue, mask, NULL);
   }

   return status;
} /* END zl303xx_DpllMaskConfigSet */

/*

  Function Name:
   zl303xx_DpllRefConfigStructInit

  Details:
   Initializes the members of the zl303xx_DpllRefConfigS data structure

  Parameters:
   [in]   zl303xx_Params Pointer to the device instance parameter structure
   [in]   par            Pointer to the zl303xx_DpllRefConfigS structure to Initialize

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllRefConfigStructInit(zl303xx_ParamsS *zl303xx_Params,
                                          zl303xx_DpllRefConfigS *par)
{
   zlStatusE status = ZL303XX_OK;

   /* Check the parameter pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, par);
   }

   if (status == ZL303XX_OK)
   {
      par->revertEn = (zl303xx_BooleanE)ZL303XX_INVALID;
      par->priority = (Uint32T)ZL303XX_INVALID;
      par->Id = (zl303xx_DpllIdE)ZL303XX_INVALID;
      par->refId = (zl303xx_RefIdE)ZL303XX_INVALID;
   }

   return status;
} /* END zl303xx_DpllRefConfigStructInit */

/*

  Function Name:
   zl303xx_DpllRefConfigCheck

  Details:
   Checks the members of the zl303xx_DpllRefConfigS data structure

  Parameters:
   [in]   zl303xx_Params Pointer to the device instance parameter structure
   [in]   par            Pointer to the zl303xx_DpllRefConfigS structure to Check

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllRefConfigCheck(zl303xx_ParamsS *zl303xx_Params,
                                     zl303xx_DpllRefConfigS *par)
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
      status = ZL303XX_CHECK_BOOLEAN(par->revertEn) |
               ZL303XX_CHECK_REF_PRIORITY(par->priority) |
               ZL303XX_CHECK_DPLL_ID(par->Id) |
               ZL303XX_CHECK_REF_ID(par->refId);
   }

   return status;
} /* END zl303xx_DpllRefConfigCheck */

/*

  Function Name:
   zl303xx_DpllRefConfigGet

  Details:
   Gets the members of the zl303xx_DpllRefConfigS data structure

  Parameters:
   [in]   zl303xx_Params Pointer to the device instance parameter structure
   [in]   par            Pointer to the zl303xx_DpllRefConfigS structure to Get

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllRefConfigGet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_DpllRefConfigS *par)
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
                            ZL303XX_DPLL_REF_REVERT_REG(par->Id,par->refId),
                            &regValue);
   }

   /* Extract */
   if (status == ZL303XX_OK)
   {
      par->revertEn = (zl303xx_BooleanE)ZL303XX_EXTRACT(regValue,
                                        ZL303XX_DPLL_REF_REV_SWITCH_EN_MASK,
                                        ZL303XX_DPLL_REF_REV_SWITCH_EN_SHIFT(par->refId));
   }

   /* Read */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_Read(zl303xx_Params, NULL,
                            ZL303XX_DPLL_REF_PRIORITY_REG(par->Id,par->refId),
                            &regValue);
   }

   /* Extract */
   if (status == ZL303XX_OK)
   {
      par->priority = (Uint32T)ZL303XX_EXTRACT(regValue,
                               ZL303XX_DPLL_REF_PRIORITY_MASK,
                               ZL303XX_DPLL_REF_PRIORITY_SHIFT(par->refId));
   }

   return status;
} /* END zl303xx_DpllRefConfigGet */

/*

  Function Name:
   zl303xx_DpllRefConfigSet

  Details:
   Sets the members of the zl303xx_DpllRefConfigS data structure

  Parameters:
   [in]   zl303xx_Params Pointer to the device instance parameter structure
   [in]   par            Pointer to the zl303xx_DpllRefConfigS structure to Set

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllRefConfigSet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_DpllRefConfigS *par)
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
      status = zl303xx_DpllRefConfigCheck(zl303xx_Params, par);
   }

   /* Package */
   if (status == ZL303XX_OK)
   {
      regValue = 0;
      mask = 0;

      /* revertEn */
      ZL303XX_INSERT(regValue, par->revertEn,
                             ZL303XX_DPLL_REF_REV_SWITCH_EN_MASK,
                             ZL303XX_DPLL_REF_REV_SWITCH_EN_SHIFT(par->refId));

      mask |= (ZL303XX_DPLL_REF_REV_SWITCH_EN_MASK << ZL303XX_DPLL_REF_REV_SWITCH_EN_SHIFT(par->refId));

      /* Write the Data for this Register */
      status = zl303xx_ReadModWrite(zl303xx_Params, NULL,
                                    ZL303XX_DPLL_REF_REVERT_REG(par->Id,par->refId),
                                    regValue, mask, NULL);
   }

   /* Package */
   if (status == ZL303XX_OK)
   {
      regValue = 0;
      mask = 0;

      /* priority */
      ZL303XX_INSERT(regValue, par->priority,
                             ZL303XX_DPLL_REF_PRIORITY_MASK,
                             ZL303XX_DPLL_REF_PRIORITY_SHIFT(par->refId));

      mask |= (ZL303XX_DPLL_REF_PRIORITY_MASK << ZL303XX_DPLL_REF_PRIORITY_SHIFT(par->refId));

      /* Write the Data for this Register */
      status = zl303xx_ReadModWrite(zl303xx_Params, NULL,
                                    ZL303XX_DPLL_REF_PRIORITY_REG(par->Id,par->refId),
                                    regValue, mask, NULL);
   }

   return status;
} /* END zl303xx_DpllRefConfigSet */


/*

  Function Name:
   zl303xx_DpllHitlessGet

  Details:
   Reads the Hitless attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllHitlessGet(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_DpllIdE dpllId,
                                 zl303xx_DpllHitlessE *val)
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
                              ZL303XX_DPLL_CONTROL_REG(dpllId),
                              ZL303XX_DPLL_HITLESS_MASK,
                              ZL303XX_DPLL_HITLESS_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_DpllHitlessE)(attrPar.value);
   }

   /* Update the device parameters structure */
   if (status == ZL303XX_OK)
   {
         zl303xx_Params->pllParams.config[dpllId].hitlessSw = *val;
   }

   return status;
}  /* END zl303xx_DpllHitlessGet */

/*

  Function Name:
   zl303xx_DpllHitlessSet

  Details:
   Writes the Hitless attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllHitlessSet(zl303xx_ParamsS *zl303xx_Params,
                                 zl303xx_DpllIdE dpllId,
                                 zl303xx_DpllHitlessE val)
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
      status = ZL303XX_CHECK_DPLL_HITLESS(val);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_DPLL_CONTROL_REG(dpllId),
                              ZL303XX_DPLL_HITLESS_MASK,
                              ZL303XX_DPLL_HITLESS_SHIFT);
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
      zl303xx_Params->pllParams.config[dpllId].hitlessSw = val;
   }

   return status;
}  /* END zl303xx_DpllHitlessSet */


/*

  Function Name:
   zl303xx_DpllBandwidthGet

  Details:
   Reads the Bandwidth attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllBandwidthGet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_DpllIdE dpllId,
                                   zl303xx_DpllBwE *val)
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
      if (dpllId != ZL303XX_DPLL_ID_1)
      {
         status = ZL303XX_PARAMETER_INVALID;
      }
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_DPLL_CONTROL_REG(dpllId),
                              ZL303XX_DPLL_BANDWIDTH_MASK,
                              ZL303XX_DPLL_BANDWIDTH_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_DpllBwE)(attrPar.value);

      if ((*val & ZL303XX_DPLL_NON_S3E_BANDWIDTH_MASK) != ZL303XX_DPLL_BW_S3E_0P3mHZ)
      {
          *val &= ZL303XX_DPLL_NON_S3E_BANDWIDTH_MASK;
      }
   }

   /* Update the device parameters structure */
   if (status == ZL303XX_OK)
   {
      zl303xx_Params->pllParams.config[dpllId].bandwidth = *val;
   }

   return status;
}  /* END zl303xx_DpllBandwidthGet */

/*

  Function Name:
   zl303xx_DpllBandwidthSet

  Details:
   Writes the Bandwidth attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllBandwidthSet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_DpllIdE dpllId,
                                   zl303xx_DpllBwE val)
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
      if (dpllId != ZL303XX_DPLL_ID_1)
      {
         status = ZL303XX_PARAMETER_INVALID;
      }
   }

   /* Check that the write value is within range */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_DPLL_BW(val);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_DPLL_CONTROL_REG(dpllId),
                              ZL303XX_DPLL_BANDWIDTH_MASK,
                              ZL303XX_DPLL_BANDWIDTH_SHIFT);
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
      zl303xx_Params->pllParams.config[dpllId].bandwidth = val;
   }

   return status;
}  /* END zl303xx_DpllBandwidthSet */


/*

  Function Name:
   zl303xx_DpllPhaseLimitGet

  Details:
   Reads the Phase Slope attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllPhaseLimitGet(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_DpllIdE dpllId,
                                    zl303xx_DpllPslE *val)
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
                              ZL303XX_DPLL_CONTROL_REG(dpllId),
                              ZL303XX_DPLL_PHASE_LIMIT_MASK(dpllId),
                              ZL303XX_DPLL_PHASE_LIMIT_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_DpllPslE)(attrPar.value);

      /* DPLL 2 only uses 1 bit to store PSL (61 us/s or unlimited) */
      if (dpllId == ZL303XX_DPLL_ID_2)
      {
         *val += ZL303XX_DPLL_PSL_61US;
      }
   }

   /* Update the device parameters structure */
   if (status == ZL303XX_OK)
   {
      zl303xx_Params->pllParams.config[dpllId].phaseSlope = *val;
   }

   return status;
}  /* END zl303xx_DpllPhaseLimitGet */

/*

  Function Name:
   zl303xx_DpllPhaseLimitSet

  Details:
   Writes the Phase Slope attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllPhaseLimitSet(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_DpllIdE dpllId,
                                    zl303xx_DpllPslE val)
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
      status = ZL303XX_CHECK_DPLL_PSL(dpllId,val);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_DPLL_CONTROL_REG(dpllId),
                              ZL303XX_DPLL_PHASE_LIMIT_MASK(dpllId),
                              ZL303XX_DPLL_PHASE_LIMIT_SHIFT);
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
      zl303xx_Params->pllParams.config[dpllId].phaseSlope = val;
   }

   return status;
}  /* END zl303xx_DpllPhaseLimitSet */


/*

  Function Name:
   zl303xx_DpllRevertiveEnGet

  Details:
   Reads the Revertive attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllRevertiveEnGet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_DPLL_CONTROL_REG(dpllId),
                              ZL303XX_DPLL_REVERTIVE_EN_MASK,
                              ZL303XX_DPLL_REVERTIVE_EN_SHIFT);
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
      zl303xx_Params->pllParams.config[dpllId].revertEn = *val;
   }

   return status;
}  /* END zl303xx_DpllRevertiveEnGet */

/*

  Function Name:
   zl303xx_DpllRevertiveEnSet

  Details:
   Writes the Revertive attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllRevertiveEnSet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_DPLL_CONTROL_REG(dpllId),
                              ZL303XX_DPLL_REVERTIVE_EN_MASK,
                              ZL303XX_DPLL_REVERTIVE_EN_SHIFT);
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
      zl303xx_Params->pllParams.config[dpllId].revertEn = val;
   }

   return status;
}  /* END zl303xx_DpllRevertiveEnSet */


/*

  Function Name:
   zl303xx_DpllHoldUpdateTimeGet

  Details:
   Reads the Holdover Upd Time attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllHoldUpdateTimeGet(zl303xx_ParamsS *zl303xx_Params,
                                        zl303xx_DpllIdE dpllId,
                                        zl303xx_DpllHldUpdateE *val)
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
      if (dpllId != ZL303XX_DPLL_ID_1)
      {
         status = ZL303XX_PARAMETER_INVALID;
      }
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_DPLL_CONTROL_REG(dpllId),
                              ZL303XX_DPLL_HOLD_UPDATE_TIME_MASK,
                              ZL303XX_DPLL_HOLD_UPDATE_TIME_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_DpllHldUpdateE)(attrPar.value);
   }

   /* Update the device parameters structure */
   if (status == ZL303XX_OK)
   {
      zl303xx_Params->pllParams.config[dpllId].hldUpdateTime = *val;
   }

   return status;
}  /* END zl303xx_DpllHoldUpdateTimeGet */

/*

  Function Name:
   zl303xx_DpllHoldUpdateTimeSet

  Details:
   Writes the Holdover Upd Time attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllHoldUpdateTimeSet(zl303xx_ParamsS *zl303xx_Params,
                                        zl303xx_DpllIdE dpllId,
                                        zl303xx_DpllHldUpdateE val)
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
      if (dpllId != ZL303XX_DPLL_ID_1)
      {
         status = ZL303XX_PARAMETER_INVALID;
      }
   }

   /* Check that the write value is within range */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_DPLL_HLD_UPDATE(val);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_DPLL_CONTROL_REG(dpllId),
                              ZL303XX_DPLL_HOLD_UPDATE_TIME_MASK,
                              ZL303XX_DPLL_HOLD_UPDATE_TIME_SHIFT);
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
      zl303xx_Params->pllParams.config[dpllId].hldUpdateTime = val;
   }

   return status;
}  /* END zl303xx_DpllHoldUpdateTimeSet */


/*

  Function Name:
   zl303xx_DpllHoldFilterBwGet

  Details:
   Reads the Holdover Filter BW attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllHoldFilterBwGet(zl303xx_ParamsS *zl303xx_Params,
                                      zl303xx_DpllIdE dpllId,
                                      zl303xx_DpllHldFilterE *val)
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
      if (dpllId != ZL303XX_DPLL_ID_1)
      {
         status = ZL303XX_PARAMETER_INVALID;
      }
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_DPLL_CONTROL_REG(dpllId),
                              ZL303XX_DPLL_HOLD_FILTER_BW_MASK,
                              ZL303XX_DPLL_HOLD_FILTER_BW_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_DpllHldFilterE)(attrPar.value);
   }

   /* Update the device parameters structure */
   if (status == ZL303XX_OK)
   {
      zl303xx_Params->pllParams.config[dpllId].hldFilterBw = *val;
   }

   return status;
}  /* END zl303xx_DpllHoldFilterBwGet */

/*

  Function Name:
   zl303xx_DpllHoldFilterBwSet

  Details:
   Writes the Holdover Filter BW attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllHoldFilterBwSet(zl303xx_ParamsS *zl303xx_Params,
                                      zl303xx_DpllIdE dpllId,
                                      zl303xx_DpllHldFilterE val)
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
      if (dpllId != ZL303XX_DPLL_ID_1)
      {
         status = ZL303XX_PARAMETER_INVALID;
      }
   }

   /* Check that the write value is within range */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_DPLL_HLD_FILTER(val);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_DPLL_CONTROL_REG(dpllId),
                              ZL303XX_DPLL_HOLD_FILTER_BW_MASK,
                              ZL303XX_DPLL_HOLD_FILTER_BW_SHIFT);
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
      zl303xx_Params->pllParams.config[dpllId].hldFilterBw = val;
   }

   return status;
}  /* END zl303xx_DpllHoldFilterBwSet */


/*

  Function Name:
   zl303xx_DpllModeSelectGet

  Details:
   Reads the Mode Select attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllModeSelectGet(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_DpllIdE dpllId,
                                    zl303xx_DpllModeE *val)
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
                              ZL303XX_DPLL_MODE_REG(dpllId),
                              ZL303XX_DPLL_MODE_SELECT_MASK,
                              ZL303XX_DPLL_MODE_SELECT_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_DpllModeE)(attrPar.value);
   }

   /* Update the device parameters structure */
   if (status == ZL303XX_OK)
   {
      zl303xx_Params->pllParams.config[dpllId].mode = *val;
   }

   return status;
}  /* END zl303xx_DpllModeSelectGet */

/*

  Function Name:
   zl303xx_DpllModeSelectSet

  Details:
   Writes the Mode Select attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllModeSelectSet(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_DpllIdE dpllId,
                                    zl303xx_DpllModeE val)
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
      status = ZL303XX_CHECK_DPLL_MODE(val);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_DPLL_MODE_REG(dpllId),
                              ZL303XX_DPLL_MODE_SELECT_MASK,
                              ZL303XX_DPLL_MODE_SELECT_SHIFT);
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
      zl303xx_Params->pllParams.config[dpllId].mode = val;
   }

   return status;
}  /* END zl303xx_DpllModeSelectSet */


/*

  Function Name:
   zl303xx_DpllRefSelectGet

  Details:
   Reads the Ref Select attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllRefSelectGet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_DpllIdE dpllId,
                                   zl303xx_RefIdE *val)
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
                              ZL303XX_DPLL_REF_SELECT_REG(dpllId),
                              ZL303XX_DPLL_REF_SELECT_MASK,
                              ZL303XX_DPLL_REF_SELECT_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_RefIdE)(attrPar.value);
   }

   /* Update the device parameters structure */
   if (status == ZL303XX_OK)
   {
      zl303xx_Params->pllParams.config[dpllId].selectedRef = *val;
   }

   return status;
}  /* END zl303xx_DpllRefSelectGet */

/*

  Function Name:
   zl303xx_DpllRefSelectSet

  Details:
   Writes the Ref Select attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllRefSelectSet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_DpllIdE dpllId,
                                   zl303xx_RefIdE val)
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
      status = ZL303XX_CHECK_REF_ID(val);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_DPLL_REF_SELECT_REG(dpllId),
                              ZL303XX_DPLL_REF_SELECT_MASK,
                              ZL303XX_DPLL_REF_SELECT_SHIFT);
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
      zl303xx_Params->pllParams.config[dpllId].selectedRef = val;
   }

   return status;
}  /* END zl303xx_DpllRefSelectSet */


/*

  Function Name:
   zl303xx_DpllScmRefSwMaskEnGet

  Details:
   Reads the Ref Switch SCM Mask attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllScmRefSwMaskEnGet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_DPLL_REF_FAIL_MASK_REG(dpllId),
                              ZL303XX_DPLL_SCM_REF_SW_MASK_EN_MASK,
                              ZL303XX_DPLL_SCM_REF_SW_MASK_EN_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_BooleanE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_DpllScmRefSwMaskEnGet */

/*

  Function Name:
   zl303xx_DpllScmRefSwMaskEnSet

  Details:
   Writes the Ref Switch SCM Mask attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllScmRefSwMaskEnSet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_DPLL_REF_FAIL_MASK_REG(dpllId),
                              ZL303XX_DPLL_SCM_REF_SW_MASK_EN_MASK,
                              ZL303XX_DPLL_SCM_REF_SW_MASK_EN_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_DpllScmRefSwMaskEnSet */


/*

  Function Name:
   zl303xx_DpllCfmRefSwMaskEnGet

  Details:
   Reads the Ref Switch CFM Mask attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllCfmRefSwMaskEnGet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_DPLL_REF_FAIL_MASK_REG(dpllId),
                              ZL303XX_DPLL_CFM_REF_SW_MASK_EN_MASK,
                              ZL303XX_DPLL_CFM_REF_SW_MASK_EN_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_BooleanE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_DpllCfmRefSwMaskEnGet */

/*

  Function Name:
   zl303xx_DpllCfmRefSwMaskEnSet

  Details:
   Writes the Ref Switch CFM Mask attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllCfmRefSwMaskEnSet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_DPLL_REF_FAIL_MASK_REG(dpllId),
                              ZL303XX_DPLL_CFM_REF_SW_MASK_EN_MASK,
                              ZL303XX_DPLL_CFM_REF_SW_MASK_EN_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_DpllCfmRefSwMaskEnSet */


/*

  Function Name:
   zl303xx_DpllGstRefSwMaskEnGet

  Details:
   Reads the Ref Switch GST Mask attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllGstRefSwMaskEnGet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_DPLL_REF_FAIL_MASK_REG(dpllId),
                              ZL303XX_DPLL_GST_REF_SW_MASK_EN_MASK,
                              ZL303XX_DPLL_GST_REF_SW_MASK_EN_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_BooleanE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_DpllGstRefSwMaskEnGet */

/*

  Function Name:
   zl303xx_DpllGstRefSwMaskEnSet

  Details:
   Writes the Ref Switch GST Mask attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllGstRefSwMaskEnSet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_DPLL_REF_FAIL_MASK_REG(dpllId),
                              ZL303XX_DPLL_GST_REF_SW_MASK_EN_MASK,
                              ZL303XX_DPLL_GST_REF_SW_MASK_EN_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_DpllGstRefSwMaskEnSet */


/*

  Function Name:
   zl303xx_DpllPfmRefSwMaskEnGet

  Details:
   Reads the Ref Switch PFM Mask attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllPfmRefSwMaskEnGet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_DPLL_REF_FAIL_MASK_REG(dpllId),
                              ZL303XX_DPLL_PFM_REF_SW_MASK_EN_MASK,
                              ZL303XX_DPLL_PFM_REF_SW_MASK_EN_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_BooleanE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_DpllPfmRefSwMaskEnGet */

/*

  Function Name:
   zl303xx_DpllPfmRefSwMaskEnSet

  Details:
   Writes the Ref Switch PFM Mask attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllPfmRefSwMaskEnSet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_DPLL_REF_FAIL_MASK_REG(dpllId),
                              ZL303XX_DPLL_PFM_REF_SW_MASK_EN_MASK,
                              ZL303XX_DPLL_PFM_REF_SW_MASK_EN_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_DpllPfmRefSwMaskEnSet */


/*

  Function Name:
   zl303xx_DpllScmHoldMaskEnGet

  Details:
   Reads the Holdover SCM Mask attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllScmHoldMaskEnGet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_DPLL_REF_FAIL_MASK_REG(dpllId),
                              ZL303XX_DPLL_SCM_HOLD_MASK_EN_MASK,
                              ZL303XX_DPLL_SCM_HOLD_MASK_EN_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_BooleanE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_DpllScmHoldMaskEnGet */

/*

  Function Name:
   zl303xx_DpllScmHoldMaskEnSet

  Details:
   Writes the Holdover SCM Mask attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllScmHoldMaskEnSet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_DPLL_REF_FAIL_MASK_REG(dpllId),
                              ZL303XX_DPLL_SCM_HOLD_MASK_EN_MASK,
                              ZL303XX_DPLL_SCM_HOLD_MASK_EN_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_DpllScmHoldMaskEnSet */


/*

  Function Name:
   zl303xx_DpllCfmHoldMaskEnGet

  Details:
   Reads the Holdover CFM Mask attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllCfmHoldMaskEnGet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_DPLL_REF_FAIL_MASK_REG(dpllId),
                              ZL303XX_DPLL_CFM_HOLD_MASK_EN_MASK,
                              ZL303XX_DPLL_CFM_HOLD_MASK_EN_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_BooleanE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_DpllCfmHoldMaskEnGet */

/*

  Function Name:
   zl303xx_DpllCfmHoldMaskEnSet

  Details:
   Writes the Holdover CFM Mask attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllCfmHoldMaskEnSet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_DPLL_REF_FAIL_MASK_REG(dpllId),
                              ZL303XX_DPLL_CFM_HOLD_MASK_EN_MASK,
                              ZL303XX_DPLL_CFM_HOLD_MASK_EN_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_DpllCfmHoldMaskEnSet */


/*

  Function Name:
   zl303xx_DpllGstHoldMaskEnGet

  Details:
   Reads the Holdover GST Mask attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllGstHoldMaskEnGet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_DPLL_REF_FAIL_MASK_REG(dpllId),
                              ZL303XX_DPLL_GST_HOLD_MASK_EN_MASK,
                              ZL303XX_DPLL_GST_HOLD_MASK_EN_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_BooleanE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_DpllGstHoldMaskEnGet */

/*

  Function Name:
   zl303xx_DpllGstHoldMaskEnSet

  Details:
   Writes the Holdover GST Mask attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllGstHoldMaskEnSet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_DPLL_REF_FAIL_MASK_REG(dpllId),
                              ZL303XX_DPLL_GST_HOLD_MASK_EN_MASK,
                              ZL303XX_DPLL_GST_HOLD_MASK_EN_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_DpllGstHoldMaskEnSet */


/*

  Function Name:
   zl303xx_DpllPfmHoldMaskEnGet

  Details:
   Reads the Holdover PFM Mask attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllPfmHoldMaskEnGet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_DPLL_REF_FAIL_MASK_REG(dpllId),
                              ZL303XX_DPLL_PFM_HOLD_MASK_EN_MASK,
                              ZL303XX_DPLL_PFM_HOLD_MASK_EN_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_BooleanE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_DpllPfmHoldMaskEnGet */

/*

  Function Name:
   zl303xx_DpllPfmHoldMaskEnSet

  Details:
   Writes the Holdover PFM Mask attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllPfmHoldMaskEnSet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_DPLL_REF_FAIL_MASK_REG(dpllId),
                              ZL303XX_DPLL_PFM_HOLD_MASK_EN_MASK,
                              ZL303XX_DPLL_PFM_HOLD_MASK_EN_SHIFT);
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_DpllPfmHoldMaskEnSet */


/*

  Function Name:
   zl303xx_DpllWaitToResGet

  Details:
   Reads the Wait to Restore (min) attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllWaitToResGet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_DpllIdE dpllId, Uint32T *val)
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
                              ZL303XX_DPLL_WAIT_TO_RES_REG(dpllId),
                              ZL303XX_DPLL_WAIT_TO_RES_MASK,
                              ZL303XX_DPLL_WAIT_TO_RES_SHIFT);
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
      zl303xx_Params->pllParams.config[dpllId].waitToRestore = *val;
   }

   return status;
}  /* END zl303xx_DpllWaitToResGet */

/*

  Function Name:
   zl303xx_DpllWaitToResSet

  Details:
   Writes the Wait to Restore (min) attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllWaitToResSet(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_DpllIdE dpllId, Uint32T val)
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
      status = ZL303XX_CHECK_WAIT_TO_RES(val);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_DPLL_WAIT_TO_RES_REG(dpllId),
                              ZL303XX_DPLL_WAIT_TO_RES_MASK,
                              ZL303XX_DPLL_WAIT_TO_RES_SHIFT);
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
      zl303xx_Params->pllParams.config[dpllId].waitToRestore = val;
   }

   return status;
}  /* END zl303xx_DpllWaitToResSet */


/*

  Function Name:
   zl303xx_DpllRefRevSwitchEnGet

  Details:
   Reads the Ref Rev En attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    refId          Associated REF Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllRefRevSwitchEnGet(zl303xx_ParamsS *zl303xx_Params,
                                        zl303xx_DpllIdE dpllId,
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

   /* Check the dpllId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_DPLL_ID(dpllId);
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
                              ZL303XX_DPLL_REF_REVERT_REG(dpllId,refId),
                              ZL303XX_DPLL_REF_REV_SWITCH_EN_MASK,
                              ZL303XX_DPLL_REF_REV_SWITCH_EN_SHIFT(refId));
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_BooleanE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_DpllRefRevSwitchEnGet */

/*

  Function Name:
   zl303xx_DpllRefRevSwitchEnSet

  Details:
   Writes the Ref Rev En attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    refId          Associated REF Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllRefRevSwitchEnSet(zl303xx_ParamsS *zl303xx_Params,
                                        zl303xx_DpllIdE dpllId,
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

   /* Check the dpllId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_DPLL_ID(dpllId);
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
                              ZL303XX_DPLL_REF_REVERT_REG(dpllId,refId),
                              ZL303XX_DPLL_REF_REV_SWITCH_EN_MASK,
                              ZL303XX_DPLL_REF_REV_SWITCH_EN_SHIFT(refId));
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_DpllRefRevSwitchEnSet */


/*

  Function Name:
   zl303xx_DpllRefPriorityGet

  Details:
   Reads the Ref Priority attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    refId          Associated REF Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllRefPriorityGet(zl303xx_ParamsS *zl303xx_Params,
                                     zl303xx_DpllIdE dpllId,
                                     zl303xx_RefIdE refId,
                                     Uint32T *val)
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

   /* Check the refId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_REF_ID(refId);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_DPLL_REF_PRIORITY_REG(dpllId,refId),
                              ZL303XX_DPLL_REF_PRIORITY_MASK,
                              ZL303XX_DPLL_REF_PRIORITY_SHIFT(refId));
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (Uint32T)(attrPar.value);
   }

   return status;
}  /* END zl303xx_DpllRefPriorityGet */

/*

  Function Name:
   zl303xx_DpllRefPrioritySet

  Details:
   Writes the Ref Priority attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    refId          Associated REF Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllRefPrioritySet(zl303xx_ParamsS *zl303xx_Params,
                                     zl303xx_DpllIdE dpllId,
                                     zl303xx_RefIdE refId,
                                     Uint32T val)
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

   /* Check the refId parameter */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_REF_ID(refId);
   }

   /* Check that the write value is within range */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_REF_PRIORITY(val);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_DPLL_REF_PRIORITY_REG(dpllId,refId),
                              ZL303XX_DPLL_REF_PRIORITY_MASK,
                              ZL303XX_DPLL_REF_PRIORITY_SHIFT(refId));
   }

   /* Write the Attribute value */
   if (status == ZL303XX_OK)
   {
      attrPar.value = (Uint32T)(val);
      status = zl303xx_AttrWrite(zl303xx_Params, &attrPar);
   }

   return status;
}  /* END zl303xx_DpllRefPrioritySet */


/*

  Function Name:
   zl303xx_DpllHoldoverStatusGet

  Details:
   Reads the Holdover attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllHoldoverStatusGet(zl303xx_ParamsS *zl303xx_Params,
                                        zl303xx_DpllIdE dpllId,
                                        zl303xx_DpllHoldStateE *val)
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
                              ZL303XX_DPLL_LOCK_STATUS_REG(dpllId),
                              ZL303XX_DPLL_HOLDOVER_STATUS_MASK,
                              ZL303XX_DPLL_HOLDOVER_STATUS_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_DpllHoldStateE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_DpllHoldoverStatusGet */


/*

  Function Name:
   zl303xx_DpllLockStatusGet

  Details:
   Reads the Lock attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllLockStatusGet(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_DpllIdE dpllId,
                                    zl303xx_DpllLockStateE *val)
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
                              ZL303XX_DPLL_LOCK_STATUS_REG(dpllId),
                              ZL303XX_DPLL_LOCK_STATUS_MASK,
                              ZL303XX_DPLL_LOCK_STATUS_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_DpllLockStateE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_DpllLockStatusGet */


/*

  Function Name:
   zl303xx_DpllCurRefFailStatusGet

  Details:
   Reads the Cur Ref Fail attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllCurRefFailStatusGet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_DPLL_LOCK_STATUS_REG(dpllId),
                              ZL303XX_DPLL_CUR_REF_FAIL_STATUS_MASK,
                              ZL303XX_DPLL_CUR_REF_FAIL_STATUS_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_BooleanE)(attrPar.value);
   }

   return status;
}  /* END zl303xx_DpllCurRefFailStatusGet */


/*

  Function Name:
   zl303xx_DpllPullinRangeGet

  Details:
   Reads the Pull-in Range attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllPullinRangeGet(zl303xx_ParamsS *zl303xx_Params,
                                     zl303xx_DpllIdE dpllId,
                                     zl303xx_DpllPullInE *val)
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
      if (dpllId != ZL303XX_DPLL_ID_1)
      {
         status = ZL303XX_PARAMETER_INVALID;
      }
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_DPLL1_PULLIN_RANGE_REG,
                              ZL303XX_DPLL_PULLIN_RANGE_MASK,
                              ZL303XX_DPLL_PULLIN_RANGE_SHIFT);
   }

   /* Read the Attribute & set the return value (regardless of status) */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRead(zl303xx_Params, &attrPar);
      *val = (zl303xx_DpllPullInE)(attrPar.value);
   }

   /* Update the device parameters structure */
   if (status == ZL303XX_OK)
   {
      zl303xx_Params->pllParams.config[dpllId].pullInRange = *val;
   }

   return status;
}  /* END zl303xx_DpllPullinRangeGet */

/*

  Function Name:
   zl303xx_DpllPullinRangeSet

  Details:
   Writes the Pull-in Range attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllPullinRangeSet(zl303xx_ParamsS *zl303xx_Params,
                                     zl303xx_DpllIdE dpllId,
                                     zl303xx_DpllPullInE val)
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
      if (dpllId != ZL303XX_DPLL_ID_1)
      {
         status = ZL303XX_PARAMETER_INVALID;
      }
   }

   /* Check that the write value is within range */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_DPLL_PULL_IN(val);
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_DPLL1_PULLIN_RANGE_REG,
                              ZL303XX_DPLL_PULLIN_RANGE_MASK,
                              ZL303XX_DPLL_PULLIN_RANGE_SHIFT);
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
      zl303xx_Params->pllParams.config[dpllId].pullInRange = val;
   }

   return status;
}  /* END zl303xx_DpllPullinRangeSet */


/*

  Function Name:
   zl303xx_DpllEnableGet

  Details:
   Reads the Enable attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllEnableGet(zl303xx_ParamsS *zl303xx_Params,
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
      if (dpllId != ZL303XX_DPLL_ID_2)
      {
         status = ZL303XX_PARAMETER_INVALID;
      }
   }

   /* Initialize the attribute structure with the necessary values */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructFill(zl303xx_Params, &attrPar,
                              ZL303XX_DPLL_CONTROL_REG(dpllId),
                              ZL303XX_DPLL_ENABLE_MASK,
                              ZL303XX_DPLL_ENABLE_SHIFT);
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
      zl303xx_Params->pllParams.config[dpllId].enable = *val;
   }

   return status;
}  /* END zl303xx_DpllEnableGet */

/*

  Function Name:
   zl303xx_DpllEnableSet

  Details:
   Writes the Enable attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllEnableSet(zl303xx_ParamsS *zl303xx_Params,
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
      if (dpllId != ZL303XX_DPLL_ID_2)
      {
         status = ZL303XX_PARAMETER_INVALID;
      }
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
                              ZL303XX_DPLL_CONTROL_REG(dpllId),
                              ZL303XX_DPLL_ENABLE_MASK,
                              ZL303XX_DPLL_ENABLE_SHIFT);
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
      zl303xx_Params->pllParams.config[dpllId].enable = val;
   }

   return status;
}  /* END zl303xx_DpllEnableSet */

/*

  Function Name:
   zl303xx_DpllDcoOffsetEnGet

  Details:
   Reads the DCO Offset Enable attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllDcoOffsetEnGet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_DPLL_CONTROL_REG(dpllId),
                              ZL303XX_DPLL_DCO_OFFSET_EN_MASK,
                              ZL303XX_DPLL_DCO_OFFSET_EN_SHIFT);
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
      zl303xx_Params->pllParams.config[dpllId].dcoOffsetEnable = *val;
   }

   return status;
}  /* END zl303xx_DpllDcoOffsetEnGet */

/*

  Function Name:
   zl303xx_DpllDcoOffsetEnSet

  Details:
   Writes the DCO Offset Enable attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllDcoOffsetEnSet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_DPLL_CONTROL_REG(dpllId),
                              ZL303XX_DPLL_DCO_OFFSET_EN_MASK,
                              ZL303XX_DPLL_DCO_OFFSET_EN_SHIFT);
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
      zl303xx_Params->pllParams.config[dpllId].dcoOffsetEnable = val;
   }

   return status;
}  /* END zl303xx_DpllDcoOffsetEnSet */

/*

  Function Name:
   zl303xx_DpllDcoFilterEnGet

  Details:
   Reads the DCO Filter Enable attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllDcoFilterEnGet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_DPLL_CONTROL_REG(dpllId),
                              ZL303XX_DPLL_DCO_FILTER_EN_MASK,
                              ZL303XX_DPLL_DCO_FILTER_EN_SHIFT);
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
      zl303xx_Params->pllParams.config[dpllId].dcoFilterEnable = *val;
   }

   return status;
}  /* END zl303xx_DpllDcoFilterEnGet */

/*

  Function Name:
   zl303xx_DpllDcoFilterEnSet

  Details:
   Writes the DCO Filter Enable attribute

  Parameters:
   [in]    zl303xx_Params Pointer to the device instance parameter structure
   [in]    dpllId         Associated DPLL Id of the attribute
   [in]    val            The value of the device attribute parameter

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_DpllDcoFilterEnSet(zl303xx_ParamsS *zl303xx_Params,
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
                              ZL303XX_DPLL_CONTROL_REG(dpllId),
                              ZL303XX_DPLL_DCO_FILTER_EN_MASK,
                              ZL303XX_DPLL_DCO_FILTER_EN_SHIFT);
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
      zl303xx_Params->pllParams.config[dpllId].dcoFilterEnable = val;
   }

   return status;
}  /* END zl303xx_DpllDcoFilterEnSet */

/*****************   APR PORTING FUNCTIONS   **********************************/

/* zl303xx_DpllGetHwLockStatus */
/**
   Callback for APR's getHwLockStatus() function binding.

  Parameters:
   [in]   hwParams     Pointer to the device instance parameter structure.
   [out]  lockStatus   DPLL lock status mapped to zl303xx_AprLockStatusE.

  Return Value:  ZL303XX_OK  Success.

*******************************************************************************/
Sint32T zl303xx_DpllGetHwLockStatus(void *hwParams, Sint32T *lockStatus)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_DpllStatusS dpllStatus = {0,0,ZL303XX_FALSE,0};

   /* Check input parameters. */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(lockStatus);
   }

   /* Read the DPLL lock status register. */
   if (status == ZL303XX_OK)
   {
      *lockStatus = (Sint32T)ZL303XX_LOCK_STATUS_UNKNOWN;

      dpllStatus.Id = ZL303XX_DPLL_ID_1;
      status = zl303xx_DpllStatusGet(hwParams, &dpllStatus);
   }

   /* Use the 3 bits to map to an APR state. */
   if (status == ZL303XX_OK)
   {
      if (dpllStatus.locked == ZL303XX_TRUE)
      {
         *lockStatus = (Sint32T)ZL303XX_LOCK_STATUS_LOCKED;
      }
      else if (dpllStatus.holdover == ZL303XX_TRUE)
      {
         *lockStatus = (Sint32T)ZL303XX_LOCK_STATUS_HOLDOVER;
      }
      else if (dpllStatus.refFailed == ZL303XX_TRUE)
      {
         *lockStatus = (Sint32T)ZL303XX_LOCK_STATUS_REF_FAILED;
      }
      else
      {
         *lockStatus = (Sint32T)ZL303XX_LOCK_STATUS_ACQUIRING;
      }
   }

   return (Sint32T)status;
}

/* zl303xx_DpllTakeHwDcoControl */
/**
   Callback for APR's takeHwDcoControl() function binding.

  Parameters:
   [in]   hwParams   Pointer to the device instance parameter structure.

  Return Value:  ZL303XX_OK  Success.

*******************************************************************************/
Sint32T zl303xx_DpllTakeHwDcoControl(void *hwParams)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_ParamsS *zl303xx_Params = hwParams;

   /* Check input parameters. */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
   }

   if (status == ZL303XX_OK)
   {
      /* If the ZL303xx is already in ToP mode, no further action is needed. */
      if (zl303xx_Params->pllParams.config[0].mode == ZL303XX_DPLL_MODE_TOP)
      {
         return status;
      }

      /* Set the DCO to update at every system interval. */
      status = zl303xx_TsEngSetDcoUpdateMode(zl303xx_Params, ZL303XX_UPDATE_SYS_INTRVL);
   }

   if (status == ZL303XX_OK)
   {
      /* Setting the DCO to update every sample interval (below) also latches
       * the time stamp registers, so disable the sampling mechanism that does
       * NOT do a DCO update. */
      status = zl303xx_TsEngSetTsSampleMode(zl303xx_Params, ZL303XX_UPDATE_NONE);
   }

   /* Set the DPLL to ToP mode. */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_DpllModeSelectSet(zl303xx_Params, ZL303XX_DPLL_ID_1,
                                         ZL303XX_DPLL_MODE_TOP);
   }

   return (Sint32T)status;
}

/* zl303xx_DpllReturnHwDcoControl */
/**
   Callback for APR's returnHwDcoControl() function binding.

  Parameters:
   [in]   hwParams   Pointer to the device instance parameter structure.

  Return Value:  ZL303XX_OK  Success.

*******************************************************************************/
Sint32T zl303xx_DpllReturnHwDcoControl(void *hwParams)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_ParamsS *zl303xx_Params = hwParams;

   /* Check input parameters. */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
   }

   if (status == ZL303XX_OK)
   {
      /* If the ZL303xx not in ToP mode, no further action is needed. */
      if (zl303xx_Params->pllParams.config[0].mode != ZL303XX_DPLL_MODE_TOP)
      {
         return status;
      }

      /* Set the ZL303xx to take a sample of the time stamp registers at every
       * system interval. */
      status = zl303xx_TsEngSetTsSampleMode(zl303xx_Params, ZL303XX_UPDATE_SYS_INTRVL);
   }

   /* Do not write the DCO offset register into the loop filter. */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_TsEngSetDcoUpdateMode(zl303xx_Params, ZL303XX_UPDATE_NONE);
   }

   /* Set the DPLL to AUTO mode. */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_DpllModeSelectSet(zl303xx_Params, ZL303XX_DPLL_ID_1,
                                         ZL303XX_DPLL_MODE_AUTO);
   }

   return (Sint32T)status;
}

/* zl303xx_DpllGetHwManualHoldoverStatus */
/**
   Callback function for zl303xx_AprAddDeviceS::getHwManualHoldoverStatus().

  Parameters:
   [in]   hwParams  Pointer to a device parameter structure.
   [out]  status    ZL303XX_TRUE if DPLL_1 is in manual holdover. ZL303XX_FALSE
                         otherwise.

  Return Value:  ZL303XX_OK  Success.

*******************************************************************************/
Sint32T zl303xx_DpllGetHwManualHoldoverStatus(void *hwParams, Sint32T *status)
{
   return zl303xx_DpllHwModeToBoolean(hwParams, ZL303XX_DPLL_MODE_HOLD, status);
}

/* zl303xx_DpllGetHwManualFreerunStatus */
/**
   Callback function for zl303xx_AprAddDeviceS::getHwManualFreerunStatus().

  Parameters:
   [in]   hwParams  Pointer to a device parameter structure.
   [out]  status    ZL303XX_TRUE if DPLL_1 is in manual freerun. ZL303XX_FALSE
                         otherwise.

  Return Value:  ZL303XX_OK  Success.

*******************************************************************************/
Sint32T zl303xx_DpllGetHwManualFreerunStatus(void *hwParams, Sint32T *status)
{
   return zl303xx_DpllHwModeToBoolean(hwParams, ZL303XX_DPLL_MODE_FREE, status);
}

/* zl303xx_DpllGetHwSyncInputEnStatus */
/**
   Callback function for zl303xx_AprAddDeviceS::getHwSyncInputEnStatus().

  Parameters:
   [in]   hwParams  Pointer to a device parameter structure.
   [out]  en        1 if the sync input of the current ref input for DPLL_1
                         is enabled and a valid frequency is detected.
                         0 otherwise.

  Return Value:  ZL303XX_OK  Success.

*******************************************************************************/
Sint32T zl303xx_DpllGetHwSyncInputEnStatus(void *hwParams, Sint32T *en)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_RefIdE refId;
   zl303xx_SyncIdE syncId;
   zl303xx_BooleanE syncFailed;

   if (status == ZL303XX_OK)
   {
      *en = 0;
      status = zl303xx_DpllRefSelectGet(hwParams, ZL303XX_DPLL_ID_1, &refId);
   }

   if (status == ZL303XX_OK)
   {
      /* Return early if the current reference has no associated sync input. */
      syncId = (zl303xx_SyncIdE)refId;
      if (ZL303XX_CHECK_SYNC_ID(syncId) != ZL303XX_OK)
      {
         return status;
      }

      status = zl303xx_RefSyncFailedGet(hwParams, syncId, &syncFailed);
   }

   if (status == ZL303XX_OK)
   {
      *en = !syncFailed;
   }

   return (Sint32T)status;
}

/* zl303xx_DpllGetHwOutOfRangeStatus */
/**
   Callback function for zl303xx_AprAddDeviceS::getHwOutOfRangeStatus().

  Parameters:
   [in]   hwParams  Pointer to a device parameter structure.
   [out]  oor       1 if the PFM for the active reference on DPLL_1 has
                         failed. 0 otherwise.

  Return Value:  ZL303XX_OK  Success.

*******************************************************************************/
Sint32T zl303xx_DpllGetHwOutOfRangeStatus(void *hwParams, Sint32T *oor)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_RefIdE refId;
   zl303xx_BooleanE pfmFail;

   if (status == ZL303XX_OK)
   {
      *oor = 0;
      status = zl303xx_DpllRefSelectGet(hwParams, ZL303XX_DPLL_ID_1, &refId);
   }

   if (status == ZL303XX_OK)
   {
      status = zl303xx_IsrPfmFailStatusGet(hwParams, refId, &pfmFail);
   }

   if (status == ZL303XX_OK)
   {
      *oor = (Sint32T)pfmFail;
   }

   return (Sint32T)status;
}

/*****************   STATIC FUNCTION DEFINITIONS   ****************************/

/* zl303xx_DpllHwModeToBoolean */
/** 

   Checks if DPLL_1 is in a specific mode.

  Parameters:
   [in]   hwParams   Pointer to a device parameter structure.
   [in]   modeCheck  The DPLL_1 mode to check for.
   [out]  matched    1 if the DPLL_1 mode is the same as modeCheck. 0 otherwise.

*******************************************************************************/
static Sint32T zl303xx_DpllHwModeToBoolean(void *hwParams,
                                           zl303xx_DpllModeE modeCheck,
                                           Sint32T *matched)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_DpllModeE mode;

   if (status == ZL303XX_OK)
   {
      *matched = 0;
      status = zl303xx_DpllModeSelectGet(hwParams, ZL303XX_DPLL_ID_1, &mode);
   }

   if (status == ZL303XX_OK)
   {
      if (mode == modeCheck)
      {
         *matched = (Sint32T)ZL303XX_TRUE;
      }
   }

   return (Sint32T)status;
}
