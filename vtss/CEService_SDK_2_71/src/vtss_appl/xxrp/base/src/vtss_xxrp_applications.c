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

#include "vtss_xxrp_applications.h"
#include "../include/vtss_xxrp_callout.h"
#include "vtss_gvrp.h"

u32 mrp_appl_add_to_database(vtss_mrp_t *apps[], vtss_mrp_t *application)
{
    apps[application->appl_type] = application;
    apps[application->appl_type]->mad = XXRP_SYS_MALLOC(sizeof(vtss_mrp_mad_t *) * L2_MAX_PORTS);
    apps[application->appl_type]->map = XXRP_SYS_MALLOC(sizeof(vtss_map_port_t) * L2_MAX_PORTS);
    memset(apps[application->appl_type]->mad, 0, (sizeof(vtss_mrp_mad_t *) * L2_MAX_PORTS));
    memset(apps[application->appl_type]->map, 0, (sizeof(vtss_mrp_mad_t *) * L2_MAX_PORTS));
    return VTSS_XXRP_RC_OK;
}

u32 mrp_appl_del_from_database(vtss_mrp_t *apps[], vtss_mrp_appl_t appl_type)
{
    u32 rc = VTSS_XXRP_RC_OK, tmp;
    /* Release all MAD & MAP pointers */
    for (tmp = 0; tmp < L2_MAX_PORTS; tmp++) {
        if (apps[appl_type]->mad[tmp]) {
            if (apps[appl_type]->mad[tmp]->machines) {
                rc += XXRP_SYS_FREE(apps[appl_type]->mad[tmp]->machines);
            }
            rc += XXRP_SYS_FREE(apps[appl_type]->mad[tmp]);
        }
        if (apps[appl_type]->map[tmp]) {
            rc += XXRP_SYS_FREE(apps[appl_type]->map[tmp]);
        }
    }
    rc += XXRP_SYS_FREE(apps[appl_type]->mad);
    rc += XXRP_SYS_FREE(apps[appl_type]->map);
    rc += XXRP_SYS_FREE(apps[appl_type]);
    apps[appl_type] = NULL;
    if (rc != VTSS_XXRP_RC_OK) {
        rc = VTSS_XXRP_RC_UNKNOWN;
    }


#ifdef VTSS_SW_OPTION_GVRP
    if (VTSS_GARP_APPL_GVRP == appl_type) {
        vtss_gvrp_destruct(FALSE);
    }

#endif

    return rc;
}
