

/*******************************************************************************
*
*  $Id: zl303xx_Error.h 6058 2011-06-10 14:37:16Z AW $
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     Error codes
*
*******************************************************************************/

#ifndef _ZL303XX_ERROR_H_
#define _ZL303XX_ERROR_H_

#ifndef ZL303XX_ERROR_BASE_NUM
   #define ZL303XX_ERROR_BASE_NUM           2000
#endif

#ifndef ZL303XX_GENERIC_ERROR_BASE_NUM
   #define ZL303XX_GENERIC_ERROR_BASE_NUM   (ZL303XX_ERROR_BASE_NUM + 900)
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Error codes used by the API */
typedef enum
{

   #include "zl303xx_ErrorLabels.h"
   #include "zl303xx_ErrorLabelsGeneric.h"

   ZL303XX_ERROR_CODE_END

} zlStatusE;

#ifdef __cplusplus
}
#endif

#endif


