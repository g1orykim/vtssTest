

/*******************************************************************************
*
*  $Id: zl303xx_SpiPort.h 8302 2012-05-31 20:26:29Z PC $
*
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     Header file for common SPI definitions on several platform
*
*******************************************************************************/

#ifndef ZL303XX_SPI_PORT_H
#define ZL303XX_SPI_PORT_H

#ifdef __cplusplus
extern "C" {
#endif

#define SPI_ERROR_BASE    2100

/* Error codes used by the API */
typedef enum
{
   /* General OK and ERROR codes */
   SPI_OK = 0,
   SPI_ERROR = SPI_ERROR_BASE,

    /* Specific Error scenerios */
    SPI_MULTIPLE_INIT,      /* SPI already initialized */
    SPI_INIT_ERROR,         /* SPI error during initialization */
    SPI_MALLOC_ERROR,       /* SPI memory allocation error */
    SPI_CMD_CP_BUSY,        /* SPI init command time-out due to CP being busy */
    SPI_MUTEX_CREATE_FAIL,  /* SPI failed to create its Mutex */
    SPI_MUTEX_DELETE_FAIL,  /* SPI failed to delete its Mutex */
    SPI_SEM_CREATE_FAIL,    /* SPI failed to create its Sema4 */
    SPI_SEM_DELETE_FAIL,    /* SPI failed to delete its Sema4 */
    SPI_ISR_CONNECT_FAIL,   /* SPI failed to connect IS-Routine to HW int */
    SPI_ISR_ENABLE_FAIL,    /* SPI failed to enable its HW int */
    SPI_ISR_DISABLE_FAIL,   /* SPI failed to disable its HW int */

    SPI_SEM_TAKE_FAIL,      /* SPI failed to take a needed Mutex/Sema */
    SPI_MUTEX_TAKE_FAIL,    /* SPI failed to take a needed Mutex/Sema */
    SPI_SEM_GIVE_FAIL,      /* SPI failed to give a needed Mutex/Sema */
    SPI_MUTEX_GIVE_FAIL,    /* SPI failed to give a needed Mutex/Sema */

   SPI_ERROR_CODE_END
} spiStatusE;

/* Enum for requested SPI IO service */
typedef enum
{
   SPI_SERVICE_WRITE = 0,
   SPI_SERVICE_READ
} spiServiceE;


#ifdef __cplusplus
}
#endif

#endif /* MULTIPLE INCLUDE BARRIER */
