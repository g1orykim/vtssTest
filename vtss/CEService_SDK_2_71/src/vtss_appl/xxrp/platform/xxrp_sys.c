/*

   Vitesse Switch API software.

 Copyright (c) 2002-2013 Vitesse Semiconductor Corporation "Vitesse". All
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
#include "../base/include/vtss_xxrp_callout.h"
#include "vtss_xxrp_api.h" /* For definition of VTSS_MRP_APPL_MAX */

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_XXRP
typedef struct mrp_mem_mgmt_oper_conf_s {
    u64 alloc_ct;
    u64 free_ct;
} xxrp_mem_mgmt_oper_conf_t;

static xxrp_mem_mgmt_oper_conf_t xxrp_mem_mgmt_oper_conf;

void *xxrp_sys_malloc(u32 size, char *file, const char *function, u32 line)
{
    void *ptr = VTSS_MALLOC(size);
    if (ptr != NULL) {
        xxrp_mem_mgmt_oper_conf.alloc_ct++;
    }

    return ptr;
}

u32 xxrp_sys_free(void *ptr, char *file, const char *function, u32 line)
{
    if ( !ptr ) {
        return VTSS_XXRP_RC_INVALID_PARAMETER;
    }
    VTSS_FREE(ptr);
    xxrp_mem_mgmt_oper_conf.free_ct++;
    return VTSS_XXRP_RC_OK;
}

u64 xxrp_mgmt_memory_mgmt_get_alloc_count(void)
{
    return xxrp_mem_mgmt_oper_conf.alloc_ct;
}

u64 xxrp_mgmt_memory_mgmt_get_free_count(void)
{
    return xxrp_mem_mgmt_oper_conf.free_ct;
}
