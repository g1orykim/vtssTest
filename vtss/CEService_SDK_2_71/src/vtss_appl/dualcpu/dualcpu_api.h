/*

 Vitesse Switch API software.

 Copyright (c) 2002-2010 Vitesse Semiconductor Corporation "Vitesse". All
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


*/
/* debug msg can be enabled by cmd "debug trace module level ipc default debug" */

#ifndef _VTSS_DUALCPU_APPL_API_H_
#define _VTSS_DUALCPU_APPL_API_H_

#include "vtss_api.h" /* For vtss_rc, vtss_vid_t, etc. */

/* DUALCPU error codes (vtss_rc) */
enum {
    DUALCPU_ERROR_GEN = MODULE_ERROR_START(VTSS_MODULE_ID_DUALCPU),  /* Generic error code */
    DUALCPU_ERROR_PARM,           /* Illegal parameter */
};

/* Initialize module */
vtss_rc dualcpu_init(vtss_init_data_t *data);

#endif /* _VTSS_DUALCPU_APPL_API_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
