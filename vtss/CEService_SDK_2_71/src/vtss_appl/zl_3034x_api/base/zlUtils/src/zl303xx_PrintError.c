

/*******************************************************************************
*
*  $Id: zl303xx_PrintError.c 7254 2011-12-02 21:25:34Z JK $
*
*  Copyright 2012 Microsemi Semiconductor Limited.
*  All rights reserved.
*
*  Module Description:
*     This file contains functions used to display text for the error codes
*
*******************************************************************************/

/*****************   INCLUDE FILES   ******************************************/

#include "zl303xx_Api.h"
#include "zl303xx_ErrorString.h"
#include "zl303xx_PrintError.h"
#include "zl303xx_Macros.h"

/*****************   DEFINES     **********************************************/

/*****************   STATIC FUNCTION DECLARATIONS   ***************************/

/*****************   STATIC GLOBAL VARIABLES   ********************************/

/*****************   EXPORTED GLOBAL VARIABLES   ******************************/

const char zl303xx_StrErrorUnknown[] = "*** Unknown error code ***";

/*****************   EXPORTED FUNCTION DEFINITIONS   **************************/

/*******************************************************************************

  Function Name:   zl303xx_GetErrString2

  Details:  Takes an error code enum in and returns a string for that code.

  Parameters:
   [in]  status   Error code to use

  Return Value: pointer to an error string

*******************************************************************************/

const char * zl303xx_GetErrString2(zlStatusE status)
{
   Uint32T  loop;

   for (loop = 0; loop < ZL303XX_ARRAY_SIZE(zlError); loop++)
   {
      if (zlError[loop].code == status)
      {
         return zlError[loop].text;
      }
   }
   return zl303xx_StrErrorUnknown;
}

/*******************************************************************************

  Function Name:   zl303xx_GetErrString

  Details:  Takes an error code enum in and returns a string for that code.

  Parameters:
   [in]  status   Error code to use

   [out] buf      pointer to a buffer to copy the error string into

*******************************************************************************/

void zl303xx_GetErrString(zlStatusE status, char *buf)
{
   strcpy(buf, zl303xx_GetErrString2(status));    /* Unknown size so strncpy cannot be used */
}

/*******************************************************************************

  Function Name:    zl303xx_PrintErr

  Details:  Takes an error code and prints the string representation to the standard
   output

  Parameters:
   [in]  status   Error code to use

  Return Value:  zlStatusE

*******************************************************************************/

void zl303xx_PrintErr(zlStatusE status)
{
   if (status != ZL303XX_OK)
   {
      Uint8T buffer[64] = {0};

      memcpy(&buffer,zl303xx_GetErrString2 (status), sizeof(buffer));

      ZL303XX_TRACE_ALWAYS("ERROR-API = %s (%d)",
            buffer, status, 0, 0, 0, 0);
   }
}

/*****************   STATIC FUNCTION DEFINITIONS   ****************************/

/*****************   END   ****************************************************/
