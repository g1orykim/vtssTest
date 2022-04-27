/*

 Vitesse Switch API software.

 Copyright (c) 2002-2011 Vitesse Semiconductor Corporation "Vitesse". All
 Rights Reserved.

 Unpublished rights reserved under the copyright laws of the United States of
 America, other countries and international treaties. Permission to use, copy,
 store and modify, the software and its source code is granted. Permission to
 integrate into other products, disclose, transmit and distribute the software
 in an absolute machine readable format (e.g. HEX file) is also granted.  The
 source code of the software may not be disclosed, transmitted or distributed
 without the written permission of Vitesse. The software and its source code
 may only be used in products utilizing the Vitesse switch products.

 This copyright notice must appear in any copy, modification, disclosure,
 transmission or distribution of the software. Vitesse retains all ownership,
 copyright, trade secret and proprietary rights in the software.

 THIS SOFTWARE HAS BEEN PROVIDED "AS IS," WITHOUT EXPRESS OR IMPLIED WARRANTY
 INCLUDING, WITHOUT LIMITATION, IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 FOR A PARTICULAR USE AND NON-INFRINGEMENT.
 
 $Id$
 $Revision$

*/

#include "version.h"

#define xstr(s) str(s)
#define str(s) #s

static const char compile_time[] = MAGIC_ID_DATE xstr(COMPILE_TIME);

// Vendor specific product name
static const char product_name[] = MAGIC_ID_PROD "Vitesse " VTSS_PRODUCT_NAME " Switch";

// Build identification
#ifdef BUILD_NUMBER
  static const char version_string[] = MAGIC_ID_VERS VTSS_PRODUCT_NAME " (" VTSS_STACK_TYPE ") " xstr(SW_RELEASE) " Build " xstr(BUILD_NUMBER);
#else
  static const char version_string[] = MAGIC_ID_VERS VTSS_PRODUCT_NAME " (" VTSS_STACK_TYPE ") " xstr(SW_RELEASE);
#endif

// Version control
#ifdef CODE_REVISION
  static const char code_revision[] = MAGIC_ID_REV xstr(CODE_REVISION);
#else
  static const char code_revision[] = MAGIC_ID_REV;
#endif

/* Software version text string */
const char *misc_software_version_txt(void)
{
    return version_string + MAGIC_ID_LEN;
}

/* Software codebase revision string */
const char *misc_software_code_revision_txt(void)
{
    return code_revision + MAGIC_ID_LEN;
}

/* Software date text string */
const char *misc_software_date_txt(void)
{
    return compile_time + MAGIC_ID_LEN;
}

const char *misc_product_name(void)
{
    return product_name + MAGIC_ID_LEN;
}

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
