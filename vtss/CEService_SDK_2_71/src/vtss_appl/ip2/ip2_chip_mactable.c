/*

 Vitesse API software.

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

#include "vtss_options.h"
#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_IP2 /* TBD - GRP...? */
#if !defined(VTSS_ARCH_JAGUAR_1)

#include "main.h"
#include "ip2_api.h"
//#include "ip2_trace.h"
#include "ip2_chip_api.h"
#include "packet_api.h"
#include "msg_api.h"
#include "port_api.h"

vtss_rc vtss_ip2_chip_init(void)
{
    return VTSS_OK;
}

vtss_rc vtss_ip2_chip_start(void)
{
    return VTSS_OK;
}

vtss_rc vtss_ip2_chip_master_up(const vtss_mac_t *const mac)
{
    return VTSS_OK;
}

vtss_rc vtss_ip2_chip_master_down(void)
{
    return VTSS_OK;
}

vtss_rc vtss_ip2_chip_switch_add(vtss_isid_t isid)
{
    return VTSS_OK;
}

vtss_rc vtss_ip2_chip_switch_del(vtss_isid_t isid)
{
    return VTSS_OK;
}

vtss_rc vtss_ip2_chip_mac_subscribe(const vtss_vid_t vlan, const vtss_mac_t *const mac)
{
    vtss_mac_table_entry_t entry;
    BOOL                   is_mc = (mac->addr[0] & 0x1 ? TRUE : FALSE);

    T_D("Add MAC %d", vlan);

    memset(&entry, 0x0, sizeof(vtss_mac_table_entry_t));
    entry.vid_mac.vid = vlan;
    entry.vid_mac.mac = *mac;
    entry.locked = TRUE;
#if defined(VTSS_FEATURE_MAC_CPU_QUEUE)
    entry.cpu_queue = (mac->addr[0] == 0xff ? PACKET_XTR_QU_BC : PACKET_XTR_QU_MGMT_MAC);
#endif /* VTSS_FEATURE_MAC_CPU_QUEUE */
    entry.copy_to_cpu = TRUE;
    if (is_mc) {
        port_iter_t pit;
        (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
        while (port_iter_getnext(&pit)) {
            entry.destination[pit.iport] = TRUE;
        }
    }

    return vtss_mac_table_add(NULL, &entry);
}

vtss_rc vtss_ip2_chip_rleg_add(const vtss_vid_t vlan)
{
    return VTSS_OK;
}

vtss_rc vtss_ip2_chip_rleg_del(const vtss_vid_t vlan)
{
    return VTSS_OK;
}

vtss_rc vtss_ip2_chip_mac_unsubscribe(const vtss_vid_t vlan, const vtss_mac_t *const mac)
{
    vtss_vid_mac_t vid_mac;

    T_D("Remove MAC %d", vlan);

    vid_mac.vid = vlan;
    vid_mac.mac = *mac;
    return vtss_mac_table_del(NULL, &vid_mac);
}

vtss_rc vtss_ip2_chip_route_add(const vtss_routing_entry_t *const rt)
{
    return VTSS_OK;
}

vtss_rc vtss_ip2_chip_route_del(const vtss_routing_entry_t *const rt)
{
    return VTSS_OK;
}

vtss_rc vtss_ip2_chip_neighbour_add(const vtss_neighbour_t *const nb)
{
    return VTSS_OK;
}

vtss_rc vtss_ip2_chip_neighbour_del(const vtss_neighbour_t *const nb)
{
    return VTSS_OK;
}

vtss_rc vtss_ip2_chip_counters_vlan_get(const vtss_vid_t vlan,
                                        vtss_l3_counters_t *counters)
{
    memset(counters, 0, sizeof(vtss_l3_counters_t));
    return VTSS_RC_OK;
}

vtss_rc vtss_ip2_chip_counters_vlan_clear(const vtss_vid_t vlan)
{
    return VTSS_RC_OK;
}

const char *ip2_chip_error_txt(vtss_rc rc)
{
    return "ip2_chip: undefined";
}

vtss_rc vtss_ip2_chip_routing_enable(BOOL enable)
{
    return VTSS_RC_OK;
}

#if 0
void vtss_debug_print_l3(const vtss_debug_printf_t pr,
                         const vtss_debug_info_t *const info)
{
}
#endif

#endif /* !VTSS_ARCH_JAGUAR_1 */
