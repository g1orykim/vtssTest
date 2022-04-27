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

#ifndef _VTSS_PRIVILEGE_H_
#define _VTSS_PRIVILEGE_H_

/* ================================================================= *
 *  Trace definitions
 * ================================================================= */

#include "vtss_privilege_api.h"
#include "vtss_module_id.h"

#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>

#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_PRIV_LVL

#define VTSS_TRACE_GRP_DEFAULT      0
#define TRACE_GRP_CRIT              1
#define TRACE_GRP_CNT               2

#include <vtss_trace_api.h>

/* ================================================================= *
 *  privilege configuration blocks
 * ================================================================= */

/* Block versions */
#define PRIVILEGE_CONF_BLK_VERSION      1

/* privilege configuration block */
typedef struct {
    u32                 version;        /* Block version */
    vtss_priv_conf_t    privilege_conf; /* privilege configuration */
} privilege_conf_blk_t;


/* ================================================================= *
 *  privilege global structure
 * ================================================================= */

/* privilege global structure */
typedef struct {
    critd_t             crit;
    vtss_priv_conf_t    privilege_conf;
} privilege_global_t;

#endif /* _VTSS_PRIVILEGE_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
