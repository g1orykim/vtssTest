

/*******************************************************************************
*
*  $Id: zl303xx_ErrorLabels.h 6098 2011-06-16 14:25:56Z DP $
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     Error labels
*
*******************************************************************************/

   /* Successful completion */
   ZL303XX_OK = 0,

   /* Generic error for zl303xx device*/
   ZL303XX_ERROR = ZL303XX_ERROR_BASE_NUM,

   ZL303XX_INVALID_MODE,

   /* general return value for enum out of range  */
   ZL303XX_PARAMETER_INVALID,

   ZL303XX_INVALID_POINTER,
   ZL303XX_NOT_RUNNING,
   ZL303XX_INTERRUPT_NOT_RUNNING,
   ZL303XX_MULTIPLE_INIT_ATTEMPT,

   ZL303XX_TABLE_FULL,
   ZL303XX_TABLE_EMPTY,
   ZL303XX_TABLE_ENTRY_DUPLICATE,
   ZL303XX_TABLE_ENTRY_NOT_FOUND,     /* 10 */

   ZL303XX_DATA_CORRUPTION,

   ZL303XX_INIT_ERROR,
   ZL303XX_HARDWARE_ERROR,   /* An internal hardware error */

   ZL303XX_IO_ERROR,
   ZL303XX_TIMEOUT,

   /* Timestamp Engine Specific Errors */
   ZL303XX_TSENG_TS_OUT_OF_RANGE,

   /* RTOS errors */
   ZL303XX_RTOS_MEMORY_FAIL,

   /* RTOS semaphore errors */
   ZL303XX_RTOS_SEM_CREATE_FAIL,
   ZL303XX_RTOS_SEM_DELETE_FAIL,
   ZL303XX_RTOS_SEM_INVALID,          /* 20 */
   ZL303XX_RTOS_SEM_TAKE_FAIL,
   ZL303XX_RTOS_SEM_GIVE_FAIL,

   /* RTOS mutex errors */
   ZL303XX_RTOS_MUTEX_CREATE_FAIL,
   ZL303XX_RTOS_MUTEX_DELETE_FAIL,
   ZL303XX_RTOS_MUTEX_INVALID,
   ZL303XX_RTOS_MUTEX_TAKE_FAIL,
   ZL303XX_RTOS_MUTEX_GIVE_FAIL,

    /* RTOS message queue errors */
   ZL303XX_RTOS_MSGQ_CREATE_FAIL,
   ZL303XX_RTOS_MSGQ_DELETE_FAIL,
   ZL303XX_RTOS_MSGQ_INVALID,         /* 30 */
   ZL303XX_RTOS_MSGQ_SEND_FAIL,
   ZL303XX_RTOS_MSGQ_RECEIVE_FAIL,

  /* RTOS task errors */
   ZL303XX_RTOS_TASK_CREATE_FAIL,
   ZL303XX_RTOS_TASK_DELETE_FAIL,

   /* Transport layer errors */
   ZL303XX_TRANSPORT_LAYER_ERROR,

   /* Protocol errors */
   ZL303XX_PROTOCOL_ENGINE_ERROR,

   ZL303XX_STATISTICS_NOT_ENABLED,
   ZL303XX_STREAM_NOT_IN_USE,

   ZL303XX_CLK_SWITCH_ERROR,

   ZL303XX_EXT_API_CALL_FAIL,         /* 40 */

   ZL303XX_INVALID_OPERATION,
   ZL303XX_UNSUPPORTED_OPERATION,




/* !!!!!!!!!!!!!!  Special cases must ALWAYS BE LAST  !!!!!!!!!!!!!!! */
   /* The next enum is a special case for a test which could fail for Linux but is valid for VxWorks */
   /* This enum's value will be adjusted based on the OS flag at compilation */
   #ifdef OS_LINUX
   ZL303XX_SPECIAL_CASE_MUTEX_DELETE_FAIL = ZL303XX_OK, /* starts counting from 0 again! */
   #else
   ZL303XX_SPECIAL_CASE_MUTEX_DELETE_FAIL = ZL303XX_RTOS_MUTEX_DELETE_FAIL,
   #endif

