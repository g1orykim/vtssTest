

/*******************************************************************************
*
*  $Id: zl303xx_Params.c 8250 2012-05-23 16:51:24Z PC $
*
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     Top level functions for creating and manipulating the zl303xx_ParamsS
*     device instance structure
*
*******************************************************************************/

/*****************   INCLUDE FILES   ******************************************/

#include "zl303xx_Global.h"
#include "zl303xx.h"
#include "zl303xx_Error.h"

/*****************   DEFINES     **********************************************/

/*****************   STATIC FUNCTION DECLARATIONS   ***************************/

/*****************   STATIC GLOBAL VARIABLES   ********************************/

/* Mutex Id for task safe device accesses */
OS_MUTEX_ID Zl303xx_DeviceMutexId = OS_MUTEX_INVALID;

/* count of the number of devices attached to the mutex */
Uint32T Zl303xx_DeviceMutexCount = 0;

/*****************   EXPORTED GLOBAL VARIABLES   ******************************/

/*****************   EXPORTED FUNCTION DEFINITIONS   **************************/

/**

  Function Name:
   zl303xx_CreateDeviceInstance

  Details:
   Allocates and initialises the memory required for one instance of the device's
   internal data structure.

  Parameters:
   [out]    zl303xx_Params   Pointer to the initialised device structure. NULL if an error
                  occurs

  Return Value:
   zlStatusE

  Notes:
   Function should be called once on initialisation. The returned structure is
   allocated on the heap. If the application is ever closed down this structure
   should be freed using zl303xx_FreeDeviceInstance()

*******************************************************************************/

zlStatusE zl303xx_CreateDeviceInstance(zl303xx_ParamsS **zl303xx_Params)
{
   zlStatusE status = ZL303XX_OK;

   ZL303XX_TRACE(ZL303XX_MOD_ID_INIT, 2,
         "zl303xx_CreateDeviceInstance",
         0,0,0,0,0,0);

   /* Check parameters */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
   }

   if (status == ZL303XX_OK)
   {
      if (*zl303xx_Params != NULL)
      {
         status = ZL303XX_ERROR;
         ZL303XX_ERROR_TRAP("zl303xx_CreateDeviceInstance: zl303xx_Params pointer not NULL");
      }
   }

   /* main function code starts */
   if (status == ZL303XX_OK)
   {
      /* Allocate a device instance structure */
      *zl303xx_Params = (zl303xx_ParamsS *)OS_CALLOC(1, sizeof(zl303xx_ParamsS));

      if (*zl303xx_Params == NULL)
      {
         status = ZL303XX_RTOS_MEMORY_FAIL;
         ZL303XX_ERROR_TRAP("zl303xx_CreateDeviceInstance: Unable to allocate device structure");
      }
   }

   if (status == ZL303XX_OK)
   {
      /* created another device structure so increment the counter */
      Zl303xx_DeviceMutexCount++;

      if (Zl303xx_DeviceMutexId == OS_MUTEX_INVALID)
      {
         /* Create device access mutex */
         Zl303xx_DeviceMutexId = OS_MUTEX_CREATE();

         if (Zl303xx_DeviceMutexId == OS_MUTEX_INVALID)
         {
            status = ZL303XX_RTOS_SEM_CREATE_FAIL;
            ZL303XX_ERROR_TRAP("zl303xx_CreateDeviceInstance: Unable to create mutex");
         }
      }
   }

   /* Force a page select on first access */
   if (status == ZL303XX_OK)
   {
      (*zl303xx_Params)->spiParams.currentPage = ~0;
   }

   if (status != ZL303XX_OK)
   {
      (void)zl303xx_FreeDeviceInstance(zl303xx_Params);
   }

   return status;
}

/**

  Function Name:
   zl303xx_FreeDeviceInstance

  Details:
   Free up the memory used for a device instance structure

  Parameters:
   [in]    zl303xx_Params   Pointer to the structure for this device instance.
                  Set to NULL on function return

  Return Value:
   ZlStatusE   ZL303XX_TRUE always

  Notes:
   Attempts to do as much clean up as possible and ignores any intermediate
   function errors

*******************************************************************************/

zlStatusE zl303xx_FreeDeviceInstance(zl303xx_ParamsS **zl303xx_Params)
{
   zlStatusE status = ZL303XX_OK;
   OS_STATUS osStatus = OS_OK;

   ZL303XX_TRACE(ZL303XX_MOD_ID_INIT, 2,
         "zl303xx_FreeDeviceInstance",
         0,0,0,0,0,0);

   /* do some parameter checking */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
   }

   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(*zl303xx_Params);
   }

   if (status == ZL303XX_OK)
   {
      if (Zl303xx_DeviceMutexCount > 0)
      {
         /* removed a device structure so decrement the counter */
         Zl303xx_DeviceMutexCount--;
      }

      if ((Zl303xx_DeviceMutexId != OS_MUTEX_INVALID) && (Zl303xx_DeviceMutexCount == 0))
      {
         osStatus = OS_MUTEX_TAKE(Zl303xx_DeviceMutexId);

         if(osStatus != OS_OK)
         {
            status = ZL303XX_RTOS_SEM_DELETE_FAIL;
            ZL303XX_ERROR_TRAP("zl303xx_FreeDeviceInstance: Unable to take mutex");
         }

         /* mutex ID is valid and only one device is attached so not going to need the
            mutex any longer */
         osStatus = OS_MUTEX_DELETE(Zl303xx_DeviceMutexId);

         if(osStatus != OS_OK)
         {
            status = ZL303XX_RTOS_MUTEX_DELETE_FAIL;
            ZL303XX_ERROR_TRAP("zl303xx_FreeDeviceInstance: Unable to delete mutex");
         }
         else
         {
            Zl303xx_DeviceMutexId = OS_MUTEX_INVALID;
         }
      }
   }

   if (status == ZL303XX_OK)
   {
      /* Reset the instance pointer */
      OS_FREE(*zl303xx_Params);
      *zl303xx_Params = NULL;
   }

   return status;
}

/**

  Function Name:
   zl303xx_LockDevParams

  Details:
   Locks the zl303xx_Params mutex

  Parameters:
   [in]    zl303xx_Params   Pointer to the structure for this device instance.

  Return Value:
   ZlStatusE

*******************************************************************************/

zlStatusE zl303xx_LockDevParams(zl303xx_ParamsS *zl303xx_Params)
{
   zlStatusE status = ZL303XX_OK;
   osStatusT osStatus;

   /* Assume that if zl303xx_Params exists then the mutex has also been created.
      If not, the mutex take function will return its own error */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
   }

   if (status == ZL303XX_OK)
   {
      if (Zl303xx_DeviceMutexId != OS_MUTEX_INVALID)
      {
         osStatus = OS_MUTEX_TAKE(Zl303xx_DeviceMutexId);

         if (osStatus != OS_OK)
         {
            status = ZL303XX_RTOS_SEM_TAKE_FAIL;
         }
      }
   }

   return status;
}

/**

  Function Name:
   zl303xx_UnlockDevParams

  Details:
   Unlocks the zl303xx_Params mutex

  Parameters:
   [in]    zl303xx_Params   Pointer to the structure for this device instance.

  Return Value:
   ZlStatusE

  Notes:
   Can only be called from the same task which locked the mutex

*******************************************************************************/

zlStatusE zl303xx_UnlockDevParams(zl303xx_ParamsS *zl303xx_Params)
{
   zlStatusE status = ZL303XX_OK;
   osStatusT osStatus;

   /* Assume that if zl303xx_Params exists then the mutex has also been created.
      If not, the mutex take function will return its own error */
   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTER(zl303xx_Params);
   }

   if (status == ZL303XX_OK)
   {
      if (Zl303xx_DeviceMutexId != OS_MUTEX_INVALID)
      {
         osStatus = OS_MUTEX_GIVE(Zl303xx_DeviceMutexId);

         if (osStatus != OS_OK)
         {
            status = ZL303XX_RTOS_SEM_GIVE_FAIL;
         }
      }
   }

   return status;
}

/*****************   END   ****************************************************/


