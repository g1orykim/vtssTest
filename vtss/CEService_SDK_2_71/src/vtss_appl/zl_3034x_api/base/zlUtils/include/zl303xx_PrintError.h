

/*******************************************************************************
*
*  $Id: zl303xx_PrintError.h 6058 2011-06-10 14:37:16Z AW $
*
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     This is the header file for the error code handling functions.
*     It contains the function prototypes and any definitions.
*
*******************************************************************************/

#ifndef _ZL303XX_PRINT_ERROR_H
#define _ZL303XX_PRINT_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif

/*****************   EXTERNAL FUNCTION DECLARATIONS   *************************/

const char *zl303xx_GetErrString2(zlStatusE status);
void zl303xx_GetErrString(zlStatusE status, char *buf);
void zl303xx_PrintErr(zlStatusE status);

#ifdef __cplusplus
}
#endif

#endif
