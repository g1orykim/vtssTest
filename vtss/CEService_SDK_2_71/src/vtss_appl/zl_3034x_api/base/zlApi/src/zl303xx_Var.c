

/*******************************************************************************
*
*  $Id: zl303xx_Var.c 8092 2012-04-17 15:27:30Z PC $
*
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     This file contains feature support for ZL303XX_ variants.
*
*******************************************************************************/

/*****************   INCLUDE FILES   ******************************************/
#include "zl303xx_Global.h"
#include "zl303xx.h"
#include "zl303xx_RdWr.h"
#include "zl303xx_Var.h"
#include "zl303xx_DutLow.h"


/*****************   DEFINES     **********************************************/

/*****************   STATIC FUNCTION DECLARATIONS   ***************************/

/*****************   STATIC GLOBAL VARIABLES   ********************************/

/*****************   EXPORTED GLOBAL VARIABLES   ******************************/

/*****************   EXPORTED FUNCTION DEFINITIONS   **************************/

/**

  Function Name:
   zl303xx_UpdateDeviceIdAndRev

  Details:
   Fills the Device ID and Revision fields in zl303xx_Params

  Parameters:
   [in]    zl303xx_Params      Pointer to the device instance parameter structure

  Return Value:
   zlStatusE

*******************************************************************************/
zlStatusE zl303xx_InitDeviceIdAndRev(zl303xx_ParamsS *zl303xx_Params)
{
   zlStatusE status;
   Uint32T regValue;

   status = ZL303XX_CHECK_POINTER(zl303xx_Params);

   /* Read register containing Device ID and Revision */
   if (status == ZL303XX_OK)
   {
       status = zl303xx_Read(zl303xx_Params, NULL, ZL303XX_DUT_DEVICE_ID_REG, &regValue);
   }

   /* Extract data, and fill appropriate fields in zl303xx_Params */
   if (status == ZL303XX_OK)
   {
      zl303xx_Params->deviceId = (Uint8T)((regValue >> ZL303XX_DUT_ID_SHIFT) & ZL303XX_DUT_ID_MASK);
      zl303xx_Params->deviceRev = (Uint8T)((regValue >> ZL303XX_DUT_REVISION_SHIFT) & ZL303XX_DUT_REVISION_MASK);
   }

   return status;
}


/**

  Function Name:
   zl303xx_GetDeviceId

  Details:
   Returns the Device ID stored in zl303xx_Params

  Parameters:
   [in]    zl303xx_Params      Pointer to the device instance parameter structure

   [out]    deviceId          Pointer to Uint8T Device ID

  Return Value:
   zlStatusE


*******************************************************************************/
zlStatusE zl303xx_GetDeviceId(zl303xx_ParamsS *zl303xx_Params, Uint8T *deviceId)
{
    zlStatusE status = ZL303XX_CHECK_POINTER(zl303xx_Params) |
                       ZL303XX_CHECK_POINTER(deviceId);

    if (status == ZL303XX_OK)
    {
        *deviceId = zl303xx_Params->deviceId;
    }

    return status;
}


/**

  Function Name:
   zl303xx_GetDeviceRevision

  Details:
   Returns the Device Revision stored in zl303xx_Params

  Parameters:
   [in]    zl303xx_Params      Pointer to the device instance parameter structure

   [out]    revision          Pointer to Uint8T Device Revision

  Return Value:
   zlStatusE

*******************************************************************************/
zlStatusE zl303xx_GetDeviceRev(zl303xx_ParamsS *zl303xx_Params, Uint8T *revision)
{
    zlStatusE status = ZL303XX_CHECK_POINTER(zl303xx_Params) |
                       ZL303XX_CHECK_POINTER(revision);

    if (status == ZL303XX_OK)
    {
        *revision = zl303xx_Params->deviceRev;
    }

    return status;
}

/**

  Function Name:
   zl303xx_PrintDeviceId

  Details:
   Prints the Device ID to the console

  Parameters:
   [in]    zl303xx_Params      Pointer to the device instance parameter structure

  Return Value:
   N/A

*******************************************************************************/
void zl303xx_PrintDeviceId(zl303xx_ParamsS *zl303xx_Params)
{
    if (ZL303XX_CHECK_POINTER(zl303xx_Params) == ZL303XX_OK)
    {
        ZL303XX_TRACE_ALWAYS("Device Id = %d", zl303xx_Params->deviceId, 0,0,0,0,0);
    }
    else
    {
        ZL303XX_TRACE_ALWAYS("Invalid pointer.",0,0,0,0,0,0);
    }
}


/**

  Function Name:
   zl303xx_PrintDeviceRev

  Details:
   Prints the Device Revision to the console

  Parameters:
   [in]    zl303xx_Params      Pointer to the device instance parameter structure

  Return Value:
   N/A

*******************************************************************************/
void zl303xx_PrintDeviceRev(zl303xx_ParamsS *zl303xx_Params)
{
    if (ZL303XX_CHECK_POINTER(zl303xx_Params) == ZL303XX_OK)
    {
        ZL303XX_TRACE_ALWAYS("Device Revision = %d", zl303xx_Params->deviceRev, 0,0,0,0,0);
    }
    else
    {
        ZL303XX_TRACE_ALWAYS("Invalid pointer.",0,0,0,0,0,0);
    }
}


/*****************   STATIC FUNCTION DEFINITIONS   ****************************/

/**
  Function Name:
   zl303xx_ReadDeviceId

  Details:
   Calls zl303xx_DutIdGet() to read Device ID from chip then converts to Uint8T

  Parameters:
   [in]    zl303xx_Params    Pointer to the device instance parameter structure

   [out]    deviceId        Pointer to Uint8T Device ID

  Return Value:
    zlStatusE

 *****************************************************************************/
zlStatusE zl303xx_ReadDeviceId(zl303xx_ParamsS *zl303xx_Params, Uint8T *deviceId)
{
    zlStatusE status;
    zl303xx_DeviceIdE id;

    /* zl303xx_Params is not referenced locally, so let called functions check */
    status = zl303xx_DutIdGet(zl303xx_Params, &id);
    if (status == ZL303XX_OK)
    {
        *deviceId = (Uint8T)id;
    }

    return status;
}


/**
  Function Name:
   zl303xx_ReadDeviceRevision

  Details:
   Calls zl303xx_DutRevisionGet() to read Rev ID from chip then converts to Uint8T

  Parameters:
   [in]    zl303xx_Params    Pointer to the device instance parameter structure

   [out]    revision        Pointer to Uint8T Device Revision

  Return Value:
    zlStatusE

 *****************************************************************************/
zlStatusE zl303xx_ReadDeviceRevision(zl303xx_ParamsS *zl303xx_Params, Uint8T *revision)
{
    zlStatusE status;
    Uint32T rev;

    /* zl303xx_Params is not referenced locally, so let called functions check */
    status = zl303xx_DutRevisionGet(zl303xx_Params, &rev);
    if (status == ZL303XX_OK)
    {
        *revision = (Uint8T)rev;
    }

    return status;
}


/*****************   END   ****************************************************/

