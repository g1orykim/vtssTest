

/*******************************************************************************
*
*  $Id: zl303xx_RdWr.c 8176 2012-05-03 17:43:59Z PC $
*
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     This file contains low level read/write functions for the device
*
*******************************************************************************/

/*****************   INCLUDE FILES   ******************************************/

#include "zl303xx_Global.h"
#include "zl303xx.h"
#include "zl303xx_Spi.h"
#include "zl303xx_AddressMap.h"
#include "zl303xx_RdWr.h"

#include "zl303xx_Porting.h"
#include "zl303xx_PortingFunctions.h"

#ifdef OS_LINUX
#include <fcntl.h>
#include "zl303xx_LinuxOs.h"
#endif

/*****************   DEFINES     **********************************************/

/*****************   STATIC DATA STRUCTURES   *********************************/

typedef enum
{
   ZL303XX_ACCESS_READ =  (Uint8T)0,
   ZL303XX_ACCESS_WRITE = (Uint8T)1
} zl303xx_RwAccessE;

/*****************   STATIC FUNCTION DECLARATIONS   ***************************/

/* Low-level SPI Read & Write functions for direct register access */
#ifndef OS_LINUX    /* This is done in the driver */
static zlStatusE zl303xx_SyncPageRegister(zl303xx_ParamsS *zl303xx_Params,
                                          zl303xx_ChipSelectCtrlS *chipSelect,
                                          Uint32T regAddr);
#endif

/*****************   STATIC GLOBAL VARIABLES   ********************************/

Sint32T Zl303xx_ReadWriteMutexValid = 0;
OS_MUTEX_ID Zl303xx_ReadWriteMutexId = OS_MUTEX_INVALID;

/*****************   EXPORTED GLOBAL VARIABLES   ******************************/

/*****************   EXPORTED FUNCTION DEFINTIONS   ***************************/

/**

  Function Name:
   zl303xx_ReadWriteInit

  Details:
   Used to setup mutex etc. for read / write accesses.

  Parameters:
        None

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_ReadWriteInit(void)
{
   zlStatusE status = ZL303XX_OK;

   ZL303XX_TRACE(ZL303XX_MOD_ID_RDWR, 1,
         "zl303xx_ReadWriteInit:", 0,0,0,0,0,0);

   if (status == ZL303XX_OK)
   {
      if (Zl303xx_ReadWriteMutexValid == 0)
      {
         /* first time in, so create the mutex */
         Zl303xx_ReadWriteMutexId = OS_MUTEX_CREATE();

         if (Zl303xx_ReadWriteMutexId == OS_MUTEX_INVALID)
         {
            status = ZL303XX_RTOS_SEM_CREATE_FAIL;
            ZL303XX_ERROR_TRAP("Unable to create mutex");
         }
      }

      if (status == ZL303XX_OK)
      {
         /* Increment the reference count */
         Zl303xx_ReadWriteMutexValid++;
      }
   }

   return status;
}  /* END zl303xx_ReadWriteInit */

/**

  Function Name:
   zl303xx_ReadWriteClose

  Details:
   Destroy read/write mutex etc. on application close down

  Parameters:
        None

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_ReadWriteClose(void)
{
   zlStatusE status = ZL303XX_OK;

   ZL303XX_TRACE(ZL303XX_MOD_ID_RDWR, 1,
         "zl303xx_ReadWriteClose:", 0,0,0,0,0,0);

   if (status == ZL303XX_OK)
   {
      if (Zl303xx_ReadWriteMutexValid > 0)
      {
         Zl303xx_ReadWriteMutexValid--;
      }

      /* If the last decrement was the last one, clean up the Mutex */
      if (Zl303xx_ReadWriteMutexValid == ZL303XX_FALSE)   /* == 0 */
      {

         /* Ensure no-one takes the mutex while we are deleting it */
         Uint32T intLockKey = OS_INTERRUPT_LOCK();

         OS_STATUS osStatus = OS_MUTEX_TAKE(Zl303xx_ReadWriteMutexId);

         if (osStatus != OS_OK)
         {
            status = ZL303XX_RTOS_SEM_DELETE_FAIL;
            ZL303XX_ERROR_TRAP("Unable to take semaphore");
         }

         /* Delete the Mutex */

         osStatus = OS_MUTEX_DELETE(Zl303xx_ReadWriteMutexId);
         if (osStatus != OS_OK)
         {
            status = ZL303XX_RTOS_SEM_DELETE_FAIL;
            ZL303XX_ERROR_TRAP("Unable to delete semaphore");
         }

         Zl303xx_ReadWriteMutexId = OS_MUTEX_INVALID;

         OS_INTERRUPT_UNLOCK(intLockKey);
      }
   }

   return status;
}  /* END zl303xx_ReadWriteClose */

/**

  Function Name:
   zl303xx_ReadWriteLock

  Details:
   Used to provide exclusive access for read/write functions, using a binary
   semaphore.

  Parameters:
        None

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_ReadWriteLock(void)
{
   zlStatusE status = ZL303XX_OK;

   /* Since Zl303xx_ReadWriteMutexValid is incremented each time
      zl303xx_ReadWriteInit is called, check that it is NOT 0 */
   if (Zl303xx_ReadWriteMutexValid == 0)
   {
      status = ZL303XX_RTOS_SEM_INVALID;
      ZL303XX_ERROR_TRAP("Rd/Wr semaphore not created");
   }
   else
   {
      OS_STATUS osStatus = OS_OK;

      /* Ensure no-one takes the mutex while we are locking it */
      Uint32T intLockKey = OS_INTERRUPT_LOCK();
      osStatus = OS_MUTEX_TAKE(Zl303xx_ReadWriteMutexId);
      OS_INTERRUPT_UNLOCK(intLockKey);

      if (osStatus != OS_OK)
      {
         status = ZL303XX_RTOS_SEM_TAKE_FAIL;
         ZL303XX_ERROR_TRAP("Rd/Wr semaphore TAKE failure");
      }
   }

   return status;
}  /* END zl303xx_ReadWriteLock */

/**

  Function Name:
    zl303xx_ReadWriteUnlock

  Details:
   Used to indicate that read/write function access has completed, and allow
   another task access.

  Parameters:
        None

  Return Value:
   zlStatusE

  Notes:
   The semaphore must already exist as we have previously taken it

*******************************************************************************/
zlStatusE zl303xx_ReadWriteUnlock(void)
{
   zlStatusE status = ZL303XX_OK;
   OS_STATUS osStatus;

   /* Ensure no-one takes the mutex while we are unlocking it */
   Uint32T intLockKey = OS_INTERRUPT_LOCK();
   /* Give up semaphore */
   osStatus = OS_MUTEX_GIVE(Zl303xx_ReadWriteMutexId);
   OS_INTERRUPT_UNLOCK(intLockKey);

   ZL303XX_TRACE(ZL303XX_MOD_ID_RDWR, 1,
         "zl303xx_ReadWriteUnlock:", 0,0,0,0,0,0);

   if (osStatus != OS_OK)
   {
       status = ZL303XX_RTOS_SEM_GIVE_FAIL;
       ZL303XX_ERROR_TRAP("Rd/Wr semaphore GIVE failure");
   }

   return status;
}  /* END zl303xx_ReadWriteUnlock */

/**

  Function Name:
   zl303xx_Read

  Details:
   Reads a value from a register in the device

  Parameters:
   [in]    zl303xx_Params   Pointer to the device instance parameter structure
   [in]    par            Pointer to a Read/Write control structure (if specified)
   [in]    regAddr        The 32-bit virtual register address

   [out]    data           The value read, expanded to a 32-bit object where necessary
                  (returned in host byte order).

  Return Value:
   zlStatusE      An error could occur under a fault condition so this should be
                  checked by the caller.

*******************************************************************************/

zlStatusE zl303xx_Read(zl303xx_ParamsS *zl303xx_Params, zl303xx_ReadWriteS* par,
                       Uint32T regAddr, Uint32T *data)
{
   /* Since we do not reference zl303xx_Params or par locally, only check the
      return data pointer. */
   zlStatusE status = ZL303XX_CHECK_POINTER(data);

   if (status == ZL303XX_OK)
   {
      Uint16T regSize = ZL303XX_MEM_SIZE_EXTRACT(regAddr);

      /* Make sure the virtual address is for a register of 4-bytes or less */
      if (regSize > sizeof(Uint32T))
      {
         status = ZL303XX_PARAMETER_INVALID;
      }
      else
      {
         /* Set to 0 to simplify the LSB to Host byte order conversion */
         *data = 0x00;

         /* The Size was fine so read the required bytes */
         status = zl303xx_ReadBuf(zl303xx_Params, par, regAddr,
                                  (Uint8T *)data, regSize);

         /* Regardless if the read was OK, set the return value. If read failed,
            then the status will alert the calling function & the data should
            be ignored. */
         /* Since the Read Buffer is in LSB order, convert it to Host order */
         *data = ZL303XX_LE2H32(*data);

         ZL303XX_TRACE(ZL303XX_MOD_ID_RDWR, 1,
               "zl303xx_Read: status %lu: register %#lx, data %#lx",
               status, regAddr, *data, 0,0,0);
      }
   }

   return status;
}  /* END zl303xx_Read */

/**

  Function Name:
   zl303xx_Write

  Details:
   Writes a value to a register in the device

  Parameters:
   [in]    zl303xx_Params   Pointer to the device instance parameter structure
   [in]    par            Pointer to a Read/Write control structure (if specified)
   [in]    regAddr        The 32-bit virtual register address
   [in]    data           The value to write (up to 32-bits) (in host byte order)

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_Write(zl303xx_ParamsS *zl303xx_Params, zl303xx_ReadWriteS* par,
                        Uint32T regAddr, Uint32T data)
{
   /* Do not check the zl303xx_Params or par pointers since we do not reference
      them locally. */
   zlStatusE status = ZL303XX_OK;

   if (status == ZL303XX_OK)
   {
      Uint16T regSize = ZL303XX_MEM_SIZE_EXTRACT(regAddr);

      /* Make sure the virtual address is for a register of 4-bytes or less so
         that the lower level functions do not trample on any memory. */
      if (regSize > sizeof(Uint32T))
      {
         status = ZL303XX_PARAMETER_INVALID;
      }
      else
      {
         /* Since zl303xx_WriteBuf needs LSB order, convert the Host data */
         data = ZL303XX_H2LE32(data);

         /* Write the buffer */
         status = zl303xx_WriteBuf(zl303xx_Params, par, regAddr,
                                   (Uint8T *)&data, regSize);
      }
   }

   return status;
}  /* END zl303xx_Write */

/**

  Function Name:
   zl303xx_ReadModWrite

  Details:
   Performs a Read / Modify / Write sequence to a single hardware register

  Parameters:
   [in]    zl303xx_Params   Pointer to the device instance parameter structure
   [in]    par            Pointer to a Read/Write control structure (if specified)
   [in]    regAddr        The 32-bit virtual register address
   [in]    data           Value to be written after applying mask
   [in]    mask           Indicates which bits in the register are to be changed.
                  - A '1' in any position will cause that bit to be set
                    according to the supplied data value.
                  - Other bits will be left unchanged.

   [out]    prevRegValue   If this is a non-NULL pointer then its value will be set to
                  the value that was read before the modification

  Return Value:
   zlStatusE

  Notes:
   This function has not been optimised for efficiency. It uses the raw read and
   write functions which will be a relatively slow approach. However, this is
   unlikely to be a problem in real use.
   It could be optimised later by avoiding multiple locks on the mutex and by
   caching all register values, so saving the device read each time.

*******************************************************************************/

zlStatusE zl303xx_ReadModWrite(zl303xx_ParamsS *zl303xx_Params, zl303xx_ReadWriteS* par,
                               Uint32T regAddr,
                               Uint32T writeData, Uint32T mask,
                               Uint32T *prevRegValue)
{
   zlStatusE status = ZL303XX_OK;
   Uint32T readData = 0;
   zl303xx_BooleanE gotMutex = ZL303XX_FALSE;
   zl303xx_ReadWriteS localRdWr;
   zl303xx_ReadWriteS *rdWr = NULL;

   ZL303XX_TRACE(ZL303XX_MOD_ID_RDWR, 2,
         "zl303xx_ReadModWrite: reg %#lx, writeData %#lx, mask %#lx",
         regAddr, writeData, mask, 0,0,0);

   if (status == ZL303XX_OK)
   {
      if ((zl303xx_Params == NULL) && (par == NULL))
      {
         status = ZL303XX_INVALID_POINTER;
      }
      else
      {
         if (par == NULL)
         {
            rdWr = &localRdWr;
         }
         else
         {
            rdWr = par;
         }
      }
   }

   if (status == ZL303XX_OK)
   {
      zl303xx_BooleanE useMutex = ZL303XX_TRUE;

      if (par != NULL)
      {
         if (par->osExclusionEnable != ZL303XX_TRUE)
         {
            useMutex = ZL303XX_FALSE;
         }
      }

   #if defined OS_LINUX && defined BYPASS_RDWR_MUTEX
      if (parentTaskId == pthread_self())  /* It's an INTERRUPT! - ignore the Mutex */
      {
          useMutex = ZL303XX_FALSE;
      }
   #endif

      if (useMutex == ZL303XX_TRUE)
      {
         status = zl303xx_ReadWriteLock();

         if (status == ZL303XX_OK)
         {
            gotMutex = ZL303XX_TRUE;

            /* temporarily clear the OS flag, to prevent the read and write funcs
               for attempting multiple takes */
            rdWr->osExclusionEnable = ZL303XX_FALSE;
         }
      }
   }

   if (status == ZL303XX_OK)
   {
      /* Read... */
      status = zl303xx_Read(zl303xx_Params, rdWr, regAddr, &readData);
   }

   if (status == ZL303XX_OK)
   {
      if (prevRegValue != NULL)
      {  /* Return the current value if appropriate */
         *prevRegValue = readData;

         ZL303XX_TRACE(ZL303XX_MOD_ID_RDWR, 1,
               "zl303xx_ReadModWrite: reg %#lx, writeData %#lx, mask %#lx, prevValue %#lx",
               regAddr, writeData, mask, readData, 0,0);
      }

      /* ...Modify... */
      readData &= ~mask;
      readData |= (writeData & mask);

      /* ...Write */
      status = zl303xx_Write(zl303xx_Params, rdWr, regAddr, readData);
   }

   if (gotMutex == ZL303XX_TRUE)
   {
      /* return the mutex */
      (void)zl303xx_ReadWriteUnlock();

      /* set the OS flag back */
      rdWr->osExclusionEnable = ZL303XX_TRUE;
   }

   return status;
}  /* END zl303xx_ReadModWrite */

/**

  Function Name:
   zl303xx_Read64

  Details:
   Reads a value from a 64-bit register in the device

  Parameters:
   [in]    zl303xx_Params   Pointer to the device instance parameter structure
   [in]    par            Pointer to a Read/Write control structure (if specified)
   [in]    regAddr        The 32-bit virtual register address

   [out]    data           The value read, in a pseudo 64-bit structure (returned in host
                  byte order).

  Return Value:
   zlStatusE      An error could occur under a fault condition so this should be
                  checked by the caller.

*******************************************************************************/

zlStatusE zl303xx_Read64(zl303xx_ParamsS *zl303xx_Params, zl303xx_ReadWriteS* par,
                         Uint32T regAddr, Uint64S *data)
{
   /* Since we do not reference zl303xx_Params or par locally, only check the
      return data pointer. */
   zlStatusE status = ZL303XX_CHECK_POINTER(data);

   if (status == ZL303XX_OK)
   {
      Uint16T regSize = ZL303XX_MEM_SIZE_EXTRACT(regAddr);

      /* Make sure the virtual address is really for an 8-byte register before
         we read so that we do not mess up the return data after the reformat */
      if (regSize != ZL303XX_MEM_SIZE_8_BYTE)
      {
         status = ZL303XX_PARAMETER_INVALID;
      }
      else
      {
         /* Create a local buffer so we can reformat the data after the read. */
         Uint32T tempBuf[2];

         /* The Size was fine so read the required 8 bytes */
         status = zl303xx_ReadBuf(zl303xx_Params, par, regAddr, (Uint8T*)tempBuf, regSize);

         /* Regardless if the read was OK, set the return value. If read failed,
            then the status will alert the calling function & the data should
            be ignored. */
         /* Since the Read Buffer is in LSB order, convert it to Host order */
         data->lo = ZL303XX_LE2H32(tempBuf[0]);
         data->hi = ZL303XX_LE2H32(tempBuf[1]);

         ZL303XX_TRACE(ZL303XX_MOD_ID_RDWR, 1,
               "zl303xx_Read64: reg %#lx, dataHi %#lx, dataLo %#lx",
               regAddr, data->hi, data->lo, 0,0,0);
      }
   }

   return status;
}  /* END zl303xx_Read64 */

/**

  Function Name:
   zl303xx_Write64

  Details:
   Writes a value to a 64-bit register in the device

  Parameters:
   [in]    zl303xx_Params   Pointer to the device instance parameter structure
   [in]    par            Pointer to a Read/Write control structure (if specified)
   [in]    regAddr        The 64-bit register address
   [in]    data           The value to write in a pseudo 64-bit structure (in host byte
                  order)

  Return Value:
   zlStatusE

*******************************************************************************/

zlStatusE zl303xx_Write64(zl303xx_ParamsS *zl303xx_Params, zl303xx_ReadWriteS* par,
                          Uint32T regAddr, Uint64S data)
{
   /* Do not check the zl303xx_Params or par pointers since we do not reference
      them locally. */
   zlStatusE status = ZL303XX_OK;

   if (status == ZL303XX_OK)
   {
      Uint16T regSize = ZL303XX_MEM_SIZE_EXTRACT(regAddr);

      /* Make sure the virtual address is really for an 8-byte register before
         we write so that we do not write the wrong data */
      if (regSize != ZL303XX_MEM_SIZE_8_BYTE)
      {
         status = ZL303XX_PARAMETER_INVALID;
      }
      else
      {
         /* Create a local buffer */
         Uint32T tempBuf[2];

         /* Since zl303xx_WriteBuf needs LSB order, convert the Host data */
         tempBuf[0] = ZL303XX_H2LE32(data.lo);
         tempBuf[1] = ZL303XX_H2LE32(data.hi);

         /* Write the buffer */
         status = zl303xx_WriteBuf(zl303xx_Params, par, regAddr, (Uint8T*)tempBuf, regSize);
      }
   }

   return status;
}  /* END zl303xx_Write64 */

/**

  Function Name:
   zl303xx_ReadBuf

  Details:
   Function through which all reads (zl303xx_Read & zl303xx_Read64) eventually
   come before calling the lower-level device interface.

  Parameters:
   [in]    zl303xx_Params   Pointer to the device instance parameter structure
   [in]    par            Pointer to a Read/Write control structure (if specified)
   [in]    regAddr        The 32-bit virtual register address
   [in]    regLen         The length (in bytes) of the device register being read.

   [out]    regBuffer      Pointer to the data buffer containing the read data.

  Return Value:
   zlStatusE

  Notes:
   This routine returns the raw device register values beginning at the regAddr
   specified and reading up to regLen bytes of data. Therefore:
      regBuffer[0] contains the LSB of the register address specified
      regBuffer[regLen - 1] contains the MSB
   It is the responsibility of the calling function to convert the buffer values
   to host byte-order and do any necessary casting.

*******************************************************************************/

zlStatusE zl303xx_ReadBuf(zl303xx_ParamsS *zl303xx_Params, zl303xx_ReadWriteS* par,
                          Uint32T regAddr,
                          Uint8T *regBuffer, Uint16T regLen)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_BooleanE gotMutex = ZL303XX_FALSE;
   zl303xx_ChipSelectCtrlS *chipSelectPtr = NULL;

   ZL303XX_TRACE(ZL303XX_MOD_ID_RDWR, 2,
         "zl303xx_Read: register %#lx",
         regAddr, 0,0,0,0,0);

#ifndef OS_LINUX
   /* Verify that at least one of zl303xx_Params or par is specified in order to
      execute the chip select function. We prefer zl303xx_Params.  */
   if (status == ZL303XX_OK)
   {
      if (zl303xx_Params != NULL)
      {
         chipSelectPtr = &(zl303xx_Params->spiParams.chipSelect);
      }
      else if (par != NULL)
      {
         chipSelectPtr = &(par->chipSelect);
      }
      else
      {
         status = ZL303XX_INVALID_POINTER;
         ZL303XX_ERROR_TRAP("Invalid pointer(s)");
      }
   }
#endif  /* !OS_LINUX */



   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, regBuffer);
   }

   /* It is possible that the OS has already locked the Read/Write interface
      (sometimes done if a Read-Modify-Change operation is required) in order
      to control the device access. If not, lock it ourselves. */
   if (status == ZL303XX_OK)
   {
      zl303xx_BooleanE useMutex = ZL303XX_TRUE;

      if (par != NULL)
      {
         if (par->osExclusionEnable != ZL303XX_TRUE)
         {
            useMutex = ZL303XX_FALSE;
         }
      }
   #if defined OS_LINUX && defined BYPASS_RDWR_MUTEX
      if (parentTaskId == pthread_self())  /* It's an INTERRUPT - ignore the Mutex */
      {
          useMutex = ZL303XX_FALSE;
      }
   #endif

      if (useMutex == ZL303XX_TRUE)
      {
         status = zl303xx_ReadWriteLock();

         if (status == ZL303XX_OK)
         {
            gotMutex = ZL303XX_TRUE;
         }
      }
   }

   /* Determine which of the low level Read Access functions is needed to
      service the current request. */
   if (status == ZL303XX_OK)
   {
      if ((regAddr & ZL303XX_MEM_DEVICE_MASK) == 0)
      {
         /* Read the data to out local buffer */
         switch (ZL303XX_MEM_OVRLY_EXTRACT(regAddr))
         {
            case ZL303XX_MEM_OVRLY_NONE :
            {
               /* Direct access register: read data directly */
               status = zl303xx_ReadLow(zl303xx_Params, chipSelectPtr,
                                        regAddr, regLen, regBuffer);
               break;
            }

            default :
            {
               status = ZL303XX_PARAMETER_INVALID;
               ZL303XX_ERROR_TRAP("Invalid overlay control");
               break;
            }
         }
      }
   }

   if (gotMutex == ZL303XX_TRUE)
   {
      /* return the mutex */
      (void)zl303xx_ReadWriteUnlock();
   }

   return status;
}  /* END zl303xx_ReadBuf */

/**

  Function Name:
   zl303xx_WriteBuf

  Details:
   Function through which all writes (zl303xx_Write & zl303xx_Write64) eventually
   come before calling the lower-level device interface.

  Parameters:
   [in]    zl303xx_Params   Pointer to the device instance parameter structure
   [in]    par            Pointer to a Read/Write control structure (if specified)
   [in]    regAddr        The 32-bit virtual register address
   [in]    regBuffer      Pointer to the data buffer to write (in LSB order)
   [in]    regLen         The length (in bytes) of the device register being written.

  Return Value:
   zlStatusE

  Notes:
   This routine expects the data in LSB order with:
      regBuffer[0] containing the LSB of the start register address specified
      regBuffer[regLen - 1] contains the MSB
   It is the responsibility of the calling function to convert the data from
   host byte-order to LSB order before passing the data buffer to this routine.

*******************************************************************************/

zlStatusE zl303xx_WriteBuf(zl303xx_ParamsS *zl303xx_Params, zl303xx_ReadWriteS* par,
                           Uint32T regAddr,
                           Uint8T *regBuffer, Uint16T regLen)
{
   zlStatusE status = ZL303XX_OK;
   zl303xx_BooleanE gotMutex = ZL303XX_FALSE;
   zl303xx_ChipSelectCtrlS *chipSelectPtr = NULL;
   Uint32T regBufAddr = 0;
   memcpy(&regBufAddr, regBuffer, sizeof(Uint32T));

   ZL303XX_TRACE(ZL303XX_MOD_ID_RDWR, 2,
         "zl303xx_WriteBuf: register %#lx, data0 %#lx",
         regAddr, regBufAddr, 0,0,0,0);

#ifndef OS_LINUX
   /* Verify that at least one of zl303xx_Params or par is specified in order to
      execute the chip select function. We prefer zl303xx_Params.  */
   if (status == ZL303XX_OK)
   {
      if (zl303xx_Params != NULL)
      {
         chipSelectPtr = &(zl303xx_Params->spiParams.chipSelect);
      }
      else if (par != NULL)
      {
         chipSelectPtr = &(par->chipSelect);
      }
      else
      {
         status = ZL303XX_INVALID_POINTER;
         ZL303XX_ERROR_TRAP("Invalid pointer(s)");
      }
   }
#endif  /* !OS_LINUX */


   if (status == ZL303XX_OK)
   {
      status = ZL303XX_CHECK_POINTERS(zl303xx_Params, regBuffer);
   }

   /* It is possible that the OS has already locked the Read/Write interface
      (sometimes done if a Read-Modify-Change operation is required) in order
      to control the device access. If not, lock it ourselves. */
   if (status == ZL303XX_OK)
   {
      zl303xx_BooleanE useMutex = ZL303XX_TRUE;

      if (par != NULL)
      {
         if (par->osExclusionEnable != ZL303XX_TRUE)
         {
            useMutex = ZL303XX_FALSE;
         }
      }
   #if defined OS_LINUX && defined BYPASS_RDWR_MUTEX
      if (parentTaskId == pthread_self())  /* It's an INTERRUPT - ignore the Mutex */
      {
          useMutex = ZL303XX_FALSE;
      }
   #endif

      if (useMutex == ZL303XX_TRUE)
      {
         status = zl303xx_ReadWriteLock();

         if (status == ZL303XX_OK)
         {
            gotMutex = ZL303XX_TRUE;
         }
      }
   }
   /* Determine which of the low level Write Access functions is needed to
      service the current request. */
   if (status == ZL303XX_OK)
   {
      if ((regAddr & ZL303XX_MEM_DEVICE_MASK) == 0)
      {
         /* Read the data to out local buffer */
         switch (ZL303XX_MEM_OVRLY_EXTRACT(regAddr))
         {
            case ZL303XX_MEM_OVRLY_NONE :
            {
               /* Direct access register: write data directly */
               status = zl303xx_WriteLow(zl303xx_Params, chipSelectPtr,
                                          regAddr, regLen, regBuffer);
               break;
            }

            default :
            {
               status = ZL303XX_PARAMETER_INVALID;
               ZL303XX_ERROR_TRAP("Invalid overlay control");
               break;
            }
         }
      }
   }

   if (gotMutex == ZL303XX_TRUE)
   {
      /* return the mutex */
      (void)zl303xx_ReadWriteUnlock();
   }

   return status;
}  /* END zl303xx_WriteBuf */

/*****************   INTERNAL FUNCTION DEFINITIONS   **************************/

/* zl303xx_WriteLow */
/** 

   Low level function to write a value to a direct register in the device.
   Any mutex operations and parameter checking will already have been performed.

  Parameters:
   [in]    zl303xx_Params   Pointer to the device instance parameter structure
   [in]    chipSelect     Pointer to any necessary SPI chip-select functionality
   [in]    regAddr        The 32-bit virtual register address
   [in]    regLen         The length (in bytes) of the device register being written
   [in]    dataBuf        The buffer containing the value to write (in LSB byte order)

  Return Value:
   zlStatusE

  Notes:
   The functionality of the device SPI burst mode is such that the first data
   value of the buffer is written to the address provided and the address is
   incremented (within the device) for the next byte (this continues as long as
   the SPI clock signal is provided). This effectively means that the LSB of the
   data needs to be in the dataBuf[0] element, so the interface to the device
   operates in little-endian (LSB) byte order.

   It is the responsibility of the calling routine to convert the data values
   from host byte order to LSB for compatibility to the device interface.

*******************************************************************************/
zlStatusE zl303xx_WriteLow(zl303xx_ParamsS *zl303xx_Params,
                           zl303xx_ChipSelectCtrlS *chipSelect,
                           Uint32T regAddr, Uint16T regLen,
                           Uint8T *dataBuf)
{
   zlStatusE status = ZL303XX_OK;

   /* Since we should not alter the input data values, create a local data
      buffer to match the input. */
   Uint8T tempBuf[ZL303XX_MEM_SIZE_EXTRACT(ZL303XX_MEM_SIZE_MASK)];
   memcpy(tempBuf, dataBuf, regLen);

#ifndef OS_LINUX
   /* Check if the device PAGE register needs to be changed */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_SyncPageRegister(zl303xx_Params, chipSelect, regAddr);
   }

   /* Write the buffer to the device */
   if (status == ZL303XX_OK)
   {
      /* Assert any chip select */
      if (chipSelect->csFuncPtr != NULL)
      {
         (void)(chipSelect->csFuncPtr)(chipSelect->csFuncPar, ZL303XX_TRUE);
      }

      /* Write the specified range of registers */
#if !defined(ZL_I2C_RDWR)
      status = cpuSpiWrite(zl303xx_Params,
#else
      status = cpuI2CWrite(zl303xx_Params,
#endif
                                ZL303XX_MEM_DIRECT_ADDR_EXTRACT(regAddr),
                                tempBuf, regLen);

      /* De-assert any chip select */
      if (chipSelect->csFuncPtr != NULL)
      {
         (void)(chipSelect->csFuncPtr)(chipSelect->csFuncPar, ZL303XX_FALSE);
      }

   }
#endif
#ifdef OS_LINUX

      (void) chipSelect; /* warning removal for unused variable in Linux only */
      /* Write the specified range of registers - Linux driver will deal with address page translation if required */
#if !defined(ZL_I2C_RDWR)
        status = cpuSpiWrite(zl303xx_Params,
#else
        status = cpuI2CWrite(zl303xx_Params,
#endif
                                regAddr,
                                tempBuf, regLen);

#endif

   return status;
}  /* END zl303xx_WriteLow */

/* zl303xx_ReadLow */
/** 

   Low level function to read a value from a direct register in the device.
   Any mutex operations and parameter checking will already have been performed.

  Parameters:
   [in]    zl303xx_Params   Pointer to the device instance parameter structure
   [in]    chipSelect     Pointer to any necessary SPI chip-select functionality
   [in]    regAddr        The 32-bit virtual register address
   [in]    regLen         The length (in bytes) of the device register being read.

   [out]    dataBuf        The buffer containing the values read (in LSB byte order)

  Return Value:
   zlStatusE

  Notes:
   The functionality of the device SPI burst mode is such that the first data
   value of a register is read from the address provided and the address is then
   incremented (within the device) for the next byte (this continues as long as
   the SPI clock signal is provided). This effectively means that the LSB of the
   data is in the dataBuf[0] element, so the interface to the device operates in
   little-endian (LSB) byte order.

   It is the responsibility of the calling routine to handle the LSB data and
   type cast it to match the variable(s) to which the buffer will be assigned.

*******************************************************************************/
zlStatusE zl303xx_ReadLow(zl303xx_ParamsS *zl303xx_Params,
                          zl303xx_ChipSelectCtrlS *chipSelect,
                          Uint32T regAddr, Uint16T regLen,
                          Uint8T *dataBuf)
{
   zlStatusE status = ZL303XX_OK;

#ifndef OS_LINUX
   /* Check if the device PAGE register needs to be changed */
   if (status == ZL303XX_OK)
   {
      status = zl303xx_SyncPageRegister(zl303xx_Params, chipSelect, regAddr);
   }

   /* Read the data from the device */
   if (status == ZL303XX_OK)
   {
      /* Assert any chip select */
      if (chipSelect->csFuncPtr != NULL)
      {
         (void)(chipSelect->csFuncPtr)(chipSelect->csFuncPar, ZL303XX_TRUE);
      }

      /* Read the specified range of registers */
#if !defined(ZL_I2C_RDWR)
      status = cpuSpiRead(zl303xx_Params,
#else
       status = cpuI2CRead(zl303xx_Params,
#endif
                               ZL303XX_MEM_DIRECT_ADDR_EXTRACT(regAddr),
                               dataBuf, regLen);

      /* De-assert any chip select */
      if (chipSelect->csFuncPtr != NULL)
      {
         (void)(chipSelect->csFuncPtr)(chipSelect->csFuncPar, ZL303XX_FALSE);
      }
   }
#endif
#ifdef OS_LINUX

      (void) chipSelect; /* warning removal for unused variable in Linux only */
   /* Read the specified range of registers  - Linux driver will deal with address page translation if required */
#if !defined(ZL_I2C_RDWR)
      status = cpuSpiRead(zl303xx_Params,
#else
       status = cpuI2CRead(zl303xx_Params,
#endif
                            regAddr,
                            dataBuf, regLen);
#endif

   return status;
}  /* END zl303xx_ReadLow */

/*****************   STATIC FUNCTION DEFINITIONS   ****************************/

#ifndef OS_LINUX    /* This is done in the driver */
/**

  Function Name:
   zl303xx_SyncRegisterPage

  Details:
   Low level function to write the device page control register with the page
   required for the given register address value (if not already the same).

  Parameters:
   [in]    zl303xx_Params   Pointer to the device instance parameter structure
   [in]    chipSelect     Pointer to any necessary SPI chip-select functionality
   [in]    regAddr        The 32-bit virtual register address

  Return Value:
   zlStatusE

*******************************************************************************/

static zlStatusE zl303xx_SyncPageRegister(zl303xx_ParamsS *zl303xx_Params,
                                          zl303xx_ChipSelectCtrlS *chipSelect,
                                          Uint32T regAddr)
{
   zlStatusE status = ZL303XX_OK;

   /* Check the input params pointer */
   status = ZL303XX_CHECK_POINTER(zl303xx_Params);

   /* Check if the address is page dependant and/or if it needs to change */
   if (status == ZL303XX_OK)
   {
      if (((regAddr & ZL303XX_MEM_DEVICE_MASK) == 0) &&
          (ZL303XX_MEM_CHECK_PAGE_REG(regAddr) == ZL303XX_TRUE))
      {
         /* Uint8T ensures the page value is in the buffer[0] position for
            the zl303xx_WriteLow call */
         Uint8T newPage = ZL303XX_MEM_PAGE_EXTRACT(regAddr);

         /* accessing the ZL device with a page offset so update the page
            register if necessary */
         if (zl303xx_Params->spiParams.currentPage != newPage)
         {
            ZL303XX_TRACE(ZL303XX_MOD_ID_RDWR, 1,
                  "zl303xx_SyncRegisterPage: reg %0lX, set page %0lX",
                  regAddr, newPage, 0,0,0,0);

            status = zl303xx_WriteLow(zl303xx_Params, chipSelect,
                                       ZL303XX_ADDR_PAGE_REG, sizeof(Uint8T),
                                       &newPage);  /* must be 1-byte */

            /* Save the change for future reference */
            if (status == ZL303XX_OK)
            {
               zl303xx_Params->spiParams.currentPage = newPage;
            }
         }
      }
   }

   return status;
}  /* END zl303xx_SyncRegisterPage */
#endif

/*****************   END   ****************************************************/
