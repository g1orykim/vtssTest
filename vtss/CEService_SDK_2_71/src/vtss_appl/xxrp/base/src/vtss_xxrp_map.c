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

#include "vtss_xxrp_map.h"
#include "vtss_xxrp_os.h"
#include "../include/vtss_xxrp_callout.h"
#include "vtss_xxrp_madtt.h"
#include "vtss_xxrp_mvrp.h"

u32 vtss_mrp_map_create_port(vtss_map_port_t **map_ports, u32 port_no)
{
    vtss_map_port_t *map;
    map = (vtss_map_port_t *)XXRP_SYS_MALLOC(sizeof(vtss_map_port_t));
    memset(map, 0, sizeof(vtss_map_port_t));
    map->port = port_no;
    map_ports[port_no] = map;
    return VTSS_XXRP_RC_OK;
}

u32 vtss_mrp_map_destroy_port(vtss_map_port_t **map_ports, u32 port_no)
{
    u32 rc;
    rc = XXRP_SYS_FREE(map_ports[port_no]);
    map_ports[port_no] = NULL;
    return rc;
}

u32 vtss_mrp_map_connect_port(vtss_map_port_t **map_ports, u8 msti, u32 port_no)
{
    u32             tmp_port, prev_port = L2_MAX_PORTS + 1;

    T_D("Enter vtss_mrp_map_connect_port msti = %u, port = %u\n", msti, (port_no + 1));
    /* Traverse through the ring */
    for (tmp_port = (port_no + 1); tmp_port != port_no; ) {
        if (tmp_port == L2_MAX_PORTS) { /* tmp_port reached maximum, start from the first port */
            tmp_port = 0;
            continue;
        }
        if (tmp_port == port_no) {
            break;
        }
        /* Check if any port is in the ring */
        if ((map_ports[tmp_port]) && (map_ports[tmp_port]->is_connected & (1 << msti))) {
            if (prev_port == (L2_MAX_PORTS + 1)) {
                map_ports[port_no]->next_in_connected_ring[msti] = map_ports[tmp_port];
            }
            prev_port = tmp_port;
        }
        tmp_port++;
    }
    if (prev_port == (L2_MAX_PORTS + 1)) {
        map_ports[port_no]->next_in_connected_ring[msti] = map_ports[port_no];
    } else {
        map_ports[prev_port]->next_in_connected_ring[msti] = map_ports[port_no];
    }
    map_ports[port_no]->is_connected |= (1 << msti);
    return VTSS_XXRP_RC_OK;
}

u32 vtss_mrp_map_disconnect_port(vtss_map_port_t **map_ports, u8 msti, u32 port_no)
{
    vtss_map_port_t *tmp_map, *prev_map = NULL;

    if (!map_ports[port_no]) { /* This should not happen in ideal case */
        return VTSS_XXRP_RC_NOT_FOUND;
    }
    tmp_map = map_ports[port_no]->next_in_connected_ring[msti];
    if (tmp_map) {
        for (; tmp_map != map_ports[port_no]; tmp_map = tmp_map->next_in_connected_ring[msti]) {
            prev_map = tmp_map;
        }
        if (prev_map) { /* Multiple nodes in the list case */
            prev_map->next_in_connected_ring[msti] = map_ports[port_no]->next_in_connected_ring[msti];
        }
        map_ports[port_no]->next_in_connected_ring[msti] = NULL;
        map_ports[port_no]->is_connected &= ~(1U << msti);
    }

    return VTSS_XXRP_RC_OK;
}

u32 vtss_mrp_map_find_port(vtss_map_port_t **map_ports, u8 msti, u32 port_no, vtss_map_port_t **map)
{
    if (map_ports[port_no]->is_connected & (1 << msti)) {
        *map = map_ports[port_no];
        return VTSS_XXRP_RC_OK;
    }
    return VTSS_XXRP_RC_NOT_FOUND;
}

void vtss_mrp_map_print_msti_ports(vtss_map_port_t **map_ports, u8 msti)
{
    u32             tmp_port;
    vtss_map_port_t *tmp_map = NULL, *first_port = NULL;
    for (tmp_port = 0; tmp_port < L2_MAX_PORTS; tmp_port++) {
        if (map_ports[tmp_port]) {
            if (map_ports[tmp_port]->is_connected & (1 << msti)) {
                printf("port_no:%u", (tmp_port + 1));
                tmp_map = map_ports[tmp_port]->next_in_connected_ring[msti];
                first_port = map_ports[tmp_port];
                break;
            }
        }
    }
    if (tmp_port < L2_MAX_PORTS) {
        while (tmp_map) {
            if (tmp_map == first_port) {
                break;
            }
            printf("=>");
            printf("port_no:%u", (tmp_map->port + 1));
            tmp_map = tmp_map->next_in_connected_ring[msti];
        }
    } else {
        printf("No ports found in the msti = %u", msti);
    }
    printf("\n");
}

u32 vtss_mrp_map_propagate_join(vtss_mrp_appl_t application, vtss_map_port_t **map_ports, u32 port_no, u32 mad_indx)
{
    u32                         rc = VTSS_XXRP_RC_OK;

























    return rc;
}

u32 vtss_mrp_map_propagate_leave(vtss_mrp_appl_t application, vtss_map_port_t **map_ports, u32 port_no, u32 mad_indx)
{
    u32                         rc = VTSS_XXRP_RC_OK;

























    return rc;
}
