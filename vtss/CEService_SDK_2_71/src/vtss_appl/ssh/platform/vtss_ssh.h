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

*/

#ifndef _VTSS_SSH_H_
#define _VTSS_SSH_H_

/* ================================================================= *
 *  Trace definitions
 * ================================================================= */

#include "vtss_ssh_api.h"
#include "vtss_module_id.h"

#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>

#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_SSH

#define VTSS_TRACE_GRP_DEFAULT      0
#define TRACE_GRP_CRIT              1
#define TRACE_GRP_CNT               2

#include <vtss_trace_api.h>

/* ================================================================= *
 *  SSH configuration blocks
 * ================================================================= */

/* Block versions */
#define SSH_CONF_BLK_VERSION      1

/* SSH configuration block */
typedef struct {
    u32         version;    /* Block version */
    ssh_conf_t  ssh_conf;   /* SSH configuration */
} ssh_conf_blk_t;


/* ================================================================= *
 *  SSH global structure
 * ================================================================= */

/* SSH global structure */
typedef struct {
    critd_t     crit;
    BOOL        apply_init_conf; /* A flag to specify the configuration should be applied at initial state */
    ssh_conf_t  ssh_conf;
} ssh_global_t;

#endif /* _VTSS_SSH_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
