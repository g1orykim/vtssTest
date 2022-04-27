/*

 Vitesse Switch API software.

 Copyright (c) 2002-2012 Vitesse Semiconductor Corporation "Vitesse". All
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

#ifndef _VTSS_USERS_H_
#define _VTSS_USERS_H_

/* ================================================================= *
 *  Trace definitions
 * ================================================================= */

#include "vtss_users_api.h"
#include "vtss_module_id.h"

#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>

#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_USERS

#define VTSS_TRACE_GRP_DEFAULT      0
#define TRACE_GRP_CRIT              1
#define TRACE_GRP_CNT               2

#include <vtss_trace_api.h>

/* ================================================================= *
 *  users configuration blocks
 * ================================================================= */

/* Block versions */
#define USERS_CONF_BLK_VERSION      2

/* users configuration block */
typedef struct {
    u32             version;                                /* Block version */
    u32             users_conf_num;                         /* users configuration number */
    users_conf_t    users_conf[VTSS_USERS_NUMBER_OF_USERS]; /* users configuration */
} users_conf_blk_t;


/* ================================================================= *
 *  users global structure
 * ================================================================= */

/* users global structure */
typedef struct {
    critd_t         crit;
    u32             users_conf_num;
    users_conf_t    users_conf[VTSS_USERS_NUMBER_OF_USERS];
} users_global_t;

#endif /* _VTSS_USERS_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
