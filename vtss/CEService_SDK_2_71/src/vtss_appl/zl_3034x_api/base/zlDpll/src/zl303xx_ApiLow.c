



/*******************************************************************************
*
*  $Id: zl303xx_ApiLow.c 6058 2011-06-10 14:37:16Z AW $
*
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     ZL303XX_ module for low level API read/write functionality.
*
*******************************************************************************/


/*****************   INCLUDE FILES   ******************************************/
#include "zl303xx_ApiLow.h"

/*****************   DEFINES     **********************************************/

/*****************   STATIC FUNCTION DECLARATIONS   ***************************/

/*****************   STATIC GLOBAL VARIABLES   ********************************/

/*****************   EXPORTED GLOBAL VARIABLES   ******************************/

/*****************   EXPORTED FUNCTION DEFINITIONS   **************************/


/*

  Function Name:
   zl303xx_AttrRdWrStructInit

  Details:
   Initialises the parameter structure for the zl303xx_AttrRead and
   zl303xx_AttrWrite functions

  Parameters:
   [in]    zl303xx_Params   Pointer to the device instance parameter structure
   [in]    par            Pointer to parameter structure to be initialized
                  See main function for parameter descriptions

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_AttrRdWrStructInit(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_AttrRdWrS *par)
{
   zlStatusE status = ZL303XX_OK;

   /* Check for valid input pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, par);
   }

   /* Set the default parameter values */
   if (status == ZL303XX_OK)
   {
      par->addr = ZL303XX_INVALID_ADDRESS;
      par->mask = ZL303XX_BLANK_MASK;
      par->shift = ZL303XX_SHIFT_MAX + 1;
      par->value = 0;
   }

   return status;
}

/*

  Function Name:
   zl303xx_AttrRdWrStructFill

  Details:
   Initializes the zl303xx_AttrRead/Write parameter structure with the values
   provided.

  Parameters:
   [in]    zl303xx_Params   Pointer to the device instance parameter structure
   [in]    par            Pointer to parameter structure to be initialized
   [in]    rdwrAddr       The attribute address
   [in]    rdwrMask       The attribute bit mask
   [in]    rdwrShift      The attribute low bit position

  Return Value:
   zlStatusE

  Notes:
   Decided NOT to pass the initial attribute value since a default would have
   to be passed if the intent of the structure is a Read operation. On Writes
   the value member of the structure can be set after this call.

*******************************************************************************/

zlStatusE zl303xx_AttrRdWrStructFill(zl303xx_ParamsS *zl303xx_Params,
                                   zl303xx_AttrRdWrS *par,
                                   Uint32T rdwrAddr,
                                   Uint32T rdwrMask,
                                   Uint32T rdwrShift)
{
   zlStatusE status = ZL303XX_OK;

   /* Check for valid input pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, par);
   }

   /* Set the requested parameter values & then validate them */
   if (status == ZL303XX_OK)
   {
      par->addr = rdwrAddr;
      par->mask = rdwrMask;
      par->shift = rdwrShift;
      par->value = 0;

      status = zl303xx_AttrRdWrStructCheck(zl303xx_Params, par);
   }

   return status;
}

/*

  Function Name:
   zl303xx_AttrRdWrStructCheck

  Details:
   Checks the zl303xx_AttrRdWrS parameters for validity.

  Parameters:
   [in]    zl303xx_Params   Pointer to the device instance parameter structure
   [in]    par            Pointer to parameter structure to be verified

  Return Value:
   zlStatusE

 *****************************************************************************/

zlStatusE zl303xx_AttrRdWrStructCheck(zl303xx_ParamsS *zl303xx_Params,
                                    zl303xx_AttrRdWrS *par)
{
   zlStatusE status = ZL303XX_OK;

   /* Check for valid input pointers */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, par);
   }

   /* Check the parameter values */
   if (status == ZL303XX_OK)
   {
      if ( (par->addr == ZL303XX_INVALID_ADDRESS) ||
           (par->mask == ZL303XX_BLANK_MASK) ||
           (par->shift > ZL303XX_SHIFT_MAX) )
      {
         status = ZL303XX_PARAMETER_INVALID;
      }
   }

   return status;
}

/*

  Function Name:
   zl303xx_AttrRead

  Details:
   Reads a device attribute by first reading its associated address and then
   extracting the value at the bit range provided.

  Parameters:
   [in]    zl303xx_Params   Pointer to the device instance parameter structure
   [in]    par            Pointer to the attribute parameter structure

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_AttrRead(zl303xx_ParamsS *zl303xx_Params, zl303xx_AttrRdWrS *par)
{
   zlStatusE status = ZL303XX_OK;
   Uint32T regValue = 0;

   /* Check the parameters */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, par);
   }

   /* Check the address, mask & shift values that have been specified */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructCheck(zl303xx_Params, par);
   }

   /* Read the associated register */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_Read(zl303xx_Params, NULL, par->addr, &regValue);

      /* Filter out the device attribute */
      /* Note: Since this is a read, set par->value regardless of the read
               status. If read failed, the calling routine should flag on status
               and ignor the value anyway */
      par->value = (regValue >> par->shift) & par->mask;
   }

   return status;
}


/*

  Function Name:
   zl303xx_AttrWrite

  Details:
   Writes a device attribute by inserting the value into its bit range.
   The Read-Modify-Write functionality is used so that other attributes within
   the same memory location are not affected.

  Parameters:
   [in]    zl303xx_Params   Pointer to the device instance parameter structure
   [in]    par            Pointer to the attribute parameter structure

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_AttrWrite(zl303xx_ParamsS *zl303xx_Params, zl303xx_AttrRdWrS *par)
{
   zlStatusE status = ZL303XX_OK;
   Uint32T shiftedVal, shiftedMask;

   /* Check the parameters */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, par);
   }

   /* Check the address, mask & shift values that have been specified */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_AttrRdWrStructCheck(zl303xx_Params, par);
   }

   /* Place the value within the specified bit range and write it */
   if (status == ZL303XX_OK)
   {
      shiftedVal = ((par->value & par->mask) << par->shift);
      shiftedMask = (par->mask << par->shift);

      status = zl303xx_ReadModWrite(zl303xx_Params, NULL, par->addr, shiftedVal,
                                  shiftedMask, NULL);
   }

   return status;
}

/*****************   END   ****************************************************/


