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

#include "main.h"
#include "cli.h"
#include "cli_api.h"
#include "mgmt_api.h"
#include "port_api.h"
#include "mac_api.h"
#include "conf_api.h"
#include "mac_cli.h"
#include "vtss_module_id.h"
#include "cli_trace_def.h"
#include "msg_api.h"
#include "vlan_api.h"

static void cli_mac_entry_print(vtss_mac_table_entry_t *mac_entry, BOOL first);

typedef struct {
    ulong      mac_max;
    ulong      mac_age_time;
    BOOL       auto_keyword;
    BOOL       secure;
    int        int_values[CLI_INT_VALUES_MAX];
} mac_cli_req_t;


void mac_cli_req_init(void)
{
    /* register the size required for mac req. structure */
    cli_req_size_register(sizeof(mac_cli_req_t));
}

static void cli_cmd_debug_mac_dump(cli_req_t *req )
{
    ulong                  i, max, static_count = 0, dynamic_count = 0, first = 1;
    vtss_vid_mac_t         vid_mac;
    vtss_mac_table_entry_t mac_entry;
    vtss_rc                rc;

    mac_cli_req_t *mac_req;
    mac_req = req->module_req;

    vid_mac.vid = (req->vid_spec == CLI_SPEC_VAL ? req->vid : 0);
    for (i = 0; i < 6; i++) {
        vid_mac.mac.addr[i] = req->mac_addr[i];
    }

    max = mac_req->mac_max;
    if (max == 0) {
        max = 0xFFFFFFFF;
    }
    for (i = 0; i < max; i++) {
        rc = vtss_mac_table_get_next(NULL, &vid_mac, &mac_entry);
        if (rc != VTSS_OK) {
            break;
        }

#if defined(VTSS_FEATURE_VSTAX_V2) && VTSS_SWITCH_STACKABLE
        {
            char buf[MGMT_PORT_BUF_SIZE];

            if (first) {
                cli_table_header("UPSID  UPSPN     Type     VID   MAC Address        Ports");
            }
            CPRINTF("%5d  ", mac_entry.vstax2.upsid);

            if (mac_entry.vstax2.upspn & 0x80000000UL) {
                CPRINTF("Internal");
            } else {
                CPRINTF("%8u", mac_entry.vstax2.upspn);
            }
            CPRINTF("  %s  %4d  %s  ",
                    mac_entry.locked ? "Static " : "Dynamic",
                    mac_entry.vid_mac.vid, misc_mac_txt(mac_entry.vid_mac.mac.addr, buf));

            CPRINTF("%s%s\n",
                    mac_entry.vstax2.remote_entry ? "-" : cli_iport_list_txt(mac_entry.destination, buf),
                    mac_entry.copy_to_cpu || (mac_entry.vstax2.upspn & 0x80000000UL) ? ",CPU" : "");
        }
#else
        cli_mac_entry_print(&mac_entry, first);
#endif
        first = 0;
        vid_mac = mac_entry.vid_mac;
        if (mac_entry.locked) {
            static_count++;
        } else {
            dynamic_count++;
        }
    }
    CPRINTF("\nDynamic Addresses: %d\n", dynamic_count);
    CPRINTF("Static Addresses : %d\n", static_count);
}

static void cli_cmd_debug_mac_add_vol(cli_req_t *req )
{
    vtss_usid_t           usid;
    vtss_isid_t           isid;
    vtss_port_no_t        iport;
    int                   i;
    mac_mgmt_addr_entry_t mac_entry;

    if (cli_cmd_switch_none(req) || cli_cmd_conf_slave(req)) {
        return;
    }
    memset(&mac_entry, 0, sizeof(mac_entry));
    mac_entry.volatil = 1;
    mac_entry.vid_mac.vid = req->vid;
    for (i = 0; i < 6; i++) {
        mac_entry.vid_mac.mac.addr[i] = req->mac_addr[i];
    }

    for (iport = VTSS_PORT_NO_START; iport < VTSS_PORT_NO_END; iport++) {
        mac_entry.destination[iport] = (req->uport_list[iport2uport(iport)] ? 1 : 0);
    }
    mac_entry.copy_to_cpu = req->cpu_port;

    for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
        if ((isid = req->stack.isid[usid]) != VTSS_ISID_END)
            if (mac_mgmt_table_add(isid, &mac_entry) != VTSS_OK) {
                CPRINTF("Error: MAC table add operation failed\n");
            }
    }
}

static void cli_cmd_debug_mac_del_vol(cli_req_t *req )
{
    vtss_usid_t           usid;
    vtss_isid_t           isid;
    int                   i;
    vtss_vid_mac_t        vid_mac;

    if (cli_cmd_switch_none(req) || cli_cmd_conf_slave(req)) {
        return;
    }


    vid_mac.vid = MAC_ALL_VLANS;

    for (i = 0; i < 6; i++) {
        vid_mac.mac.addr[i] = req->mac_addr[i];
    }

    for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
        if ((isid = req->stack.isid[usid]) != VTSS_ISID_END)
            if (mac_mgmt_table_del(isid, &vid_mac, 1) != VTSS_OK) {
                CPRINTF("mac table del operation failed\n");
            }
    }
}

static void cli_cmd_debug_mac_del(cli_req_t *req )
{
    vtss_usid_t           usid;
    vtss_isid_t           isid;
    int                   i, a;
    vtss_vid_mac_t        vid_mac;

    if (cli_cmd_switch_none(req) || cli_cmd_conf_slave(req)) {
        return;
    }

    vid_mac.vid = req->vid;

    for (i = 0; i < 6; i++) {
        vid_mac.mac.addr[i] = req->mac_addr[i];
    }

    for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
        if ((isid = req->stack.isid[usid]) != VTSS_ISID_END)
            for (a = 1; a <= req->int_values[0]; a++) {
                if (mac_mgmt_table_del(isid, &vid_mac, 0) != VTSS_OK) {
                    CPRINTF("mac table del operation failed\n");
                }

                for (i = 5; i >= 0; i--)
                    if (++vid_mac.mac.addr[i] != 0) {
                        break;
                    }
            }
    }
}

static void cli_cmd_debug_mac_eat(cli_req_t *req )
{
    vtss_port_no_t        iport;
    int                   i, a;
    mac_mgmt_addr_entry_t mac_entry;

    if (cli_cmd_switch_none(req) || cli_cmd_conf_slave(req)) {
        return;
    }

    memset(&mac_entry, 0, sizeof(mac_entry));
    mac_entry.volatil = 1;
    mac_entry.vid_mac.vid = req->vid;

    for (iport = VTSS_PORT_NO_START; iport < VTSS_PORT_NO_END; iport++) {
        mac_entry.destination[iport] = (req->uport_list[iport2uport(iport)] ? 1 : 0);
    }

    // Only set on the master, independent of what has been selected with "/stack sel"
    for (i = 0; i < 6; i++) {
        mac_entry.vid_mac.mac.addr[i] = req->mac_addr[i];
    }

    for (a = 1; a <= req->int_values[0]; a++) {
        if (mac_mgmt_table_add(VTSS_ISID_LOCAL, &mac_entry) != VTSS_OK) {
            CPRINTF("mac table add operation failed\n");
        }
        for (i = 5; i >= 0; i--)
            if (++mac_entry.vid_mac.mac.addr[i] != 0) {
                break;
            }
    }
}

static void cli_cmd_debug_mac_static_eat(cli_req_t *req )
{
    vtss_port_no_t        iport;
    int                   i, a;
    mac_mgmt_addr_entry_t mac_entry;
    vtss_usid_t           usid;
    vtss_isid_t           isid;

    if (cli_cmd_switch_none(req) || cli_cmd_conf_slave(req)) {
        return;
    }

    memset(&mac_entry, 0, sizeof(mac_entry));
    mac_entry.vid_mac.vid = req->vid;

    for (iport = VTSS_PORT_NO_START; iport < VTSS_PORT_NO_END; iport++) {
        mac_entry.destination[iport] = (req->uport_list[iport2uport(iport)] ? 1 : 0);
    }

    for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
        if ((isid = req->stack.isid[usid]) == VTSS_ISID_END) {
            continue;
        }

        for (i = 0; i < 6; i++) {
            mac_entry.vid_mac.mac.addr[i] = req->mac_addr[i];
        }

        for (a = 1; a <= req->int_values[0]; a++) {
            if (mac_mgmt_table_add(isid, &mac_entry) != VTSS_OK) {
                CPRINTF("mac table add operation failed\n");
            }
            for (i = 5; i >= 0; i--)
                if (++mac_entry.vid_mac.mac.addr[i] != 0) {
                    break;
                }
        }
    }
}


static void mac_mgmt_table_cli_flush ( cli_req_t *req )
{
    if (mac_mgmt_table_flush() != VTSS_OK) {
        CPRINTF("mac flush operation failed\n");
    }
}

static void cli_mac_entry_print(vtss_mac_table_entry_t *mac_entry, BOOL first)
{
    char buf[MGMT_PORT_BUF_SIZE], *p = buf;

    if (first) {
#if defined(VTSS_FEATURE_VSTAX_V2) && VTSS_SWITCH_STACKABLE
        if (vtss_stacking_enabled()) {
            p += sprintf(p, "Switch   ");
        }
#endif
        sprintf(p, "Type    VID  MAC Address        Ports");
        cli_table_header(buf);
    }
#if defined(VTSS_FEATURE_VSTAX_V2) && VTSS_SWITCH_STACKABLE
    if (vtss_stacking_enabled()) {
        if (!msg_switch_is_master()) {
            CPRINTF("%-9s", mac_entry->vstax2.remote_entry ? "Remote" : "Local");
        } else {
            CPRINTF("%-9d", mac_mgmt_upsid2usid(mac_entry->vstax2.upsid));
        }
    }
#endif
    CPRINTF("%s %-4d %s  ",
            mac_entry->locked ? "Static " : "Dynamic",
            mac_entry->vid_mac.vid, misc_mac_txt(mac_entry->vid_mac.mac.addr, buf));

#if defined(VTSS_FEATURE_VSTAX_V2) && VTSS_SWITCH_STACKABLE
    CPRINTF("%s%s\n",
            (!msg_switch_is_master() && mac_entry->vstax2.remote_entry) ? "-" : cli_iport_list_txt(mac_entry->destination, buf),
            mac_entry->copy_to_cpu ? ",CPU" : "");
#else
    CPRINTF("%s%s\n", cli_iport_list_txt(mac_entry->destination, buf), mac_entry->copy_to_cpu ? ",CPU" : "");

#endif

}

static void cli_mac_entry_print_stack(mac_mgmt_table_stack_t *mac_entry, BOOL first)
{
    char buf[MGMT_PORT_BUF_SIZE], *p = buf;
    u32 i;
    vtss_isid_t isid;

    if (first) {
        sprintf(p, "ISID   Type    VID  MAC Address        Ports");
        cli_table_header(buf);
    }

    for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
        for (i = VTSS_PORT_NO_START; i < VTSS_PORT_NO_END; i++) {
            if (mac_entry->destination[isid][i] || mac_entry->copy_to_cpu) {
                goto search_end;
            }
        }
    }
search_end:
    if (isid == VTSS_ISID_END) {
        isid = VTSS_ISID_START;
    }

    CPRINTF("%-6d %s %-4d %s  ", isid,
            mac_entry->locked ? "Static " : "Dynamic",
            mac_entry->vid_mac.vid, misc_mac_txt(mac_entry->vid_mac.mac.addr, buf));
    CPRINTF("%s%s\n",
            cli_iport_list_txt(mac_entry->destination[isid], buf),
            mac_entry->copy_to_cpu ? ",CPU" : "");


}

static void cli_cmd_mac_conf(cli_req_t *req, BOOL age, BOOL learn, BOOL mac)
{
    vtss_usid_t           usid;
    vtss_isid_t           isid;
    mac_age_conf_t        conf;
    vtss_learn_mode_t     mode;
    mac_mgmt_addr_entry_t mac_entry;
    vtss_vid_mac_t        vid_mac;
    BOOL                  first, vol, chg_allowed;
    char                  buf[MGMT_PORT_BUF_SIZE];
    mac_cli_req_t          *mac_req = NULL;
    int                   i;
    port_iter_t           pit;

    mac_req = req->module_req;

    if (age) {
        if (cli_cmd_conf_slave(req) || mac_mgmt_age_time_get(&conf) != VTSS_OK) {
            return;
        }

        if (req->set) {
            conf.mac_age_time = mac_req->mac_age_time;
            if (mac_mgmt_age_time_set(&conf) != VTSS_OK) {
                CPRINTF("mac age time set operation failed\n");
            }
        } else {
            CPRINTF("MAC Age Time: %d\n", conf.mac_age_time);
        }
    }

    if (learn && cli_cmd_switch_none(req)) {
        return;
    }

    for (usid = VTSS_USID_START; learn && usid < VTSS_USID_END; usid++) {
        if ((isid = req->stack.isid[usid]) == VTSS_ISID_END) {
            continue;
        }

        first = 1;

        (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            if (req->uport_list[pit.uport] == 0) {
                continue;
            }

            if (req->set) {
                mode.automatic = mac_req->auto_keyword;
                mode.cpu = 0;
                mode.discard = mac_req->secure;
                if (mac_mgmt_learn_mode_set(isid, pit.iport, &mode) == MAC_ERROR_LEARN_FORCE_SECURE) {
                    CPRINTF("The learn mode can not be changed on port %d while the learn mode is forced to 'secure' (probably by 802.1X module)\n", pit.uport);
                }
            } else {
                if (first) {
                    cli_cmd_usid_print(usid, req, 1);
                    cli_table_header("Port  Learning");
                    first = 0;
                }
                mac_mgmt_learn_mode_get(isid, pit.iport, &mode, &chg_allowed);
                CPRINTF("%-2d    %s%s\n",
                        pit.uport,
                        mode.automatic ? "Auto" : mode.discard ? "Secure" : "Disabled",
                        chg_allowed    ? "" : " (R/O)");
            }
        }
    }

    for (usid = VTSS_USID_START; mac && usid < VTSS_USID_END; usid++) {
        if ((isid = req->stack.isid[usid]) == VTSS_ISID_END) {
            continue;
        }

        for (vol = 0; vol <= 1; vol++) {
            first = 1;
            memset(&vid_mac, 0, sizeof(vid_mac));
            i = 0;
            while (mac_mgmt_static_get_next(isid, &vid_mac, &mac_entry, 1, vol) == VTSS_OK) {
                if (first) {
                    CPRINTF("%s", vol ? "\nVolatile static:" : "\nNon-volatile static:");
                    cli_cmd_usid_print(usid, req, 1);
                    cli_table_header("VID  MAC Address        Ports");
                    first = 0;
                }
                vid_mac = mac_entry.vid_mac;
                CPRINTF("%-4d %s  ", vid_mac.vid, misc_mac_txt(vid_mac.mac.addr, buf));
                CPRINTF("%s\n", cli_iport_list_txt(mac_entry.destination, buf));
                i++;
            }
            if (i > 0) {
                CPRINTF("Total %d addresses\n", i);
            }
        }
    }
}

static void cli_cmd_mac_config(cli_req_t *req )
{
    uchar mac[6];
    char  buf[128];

    if (!req->set) {
        cli_header("MAC Configuration", 1);
    }

    if (conf_mgmt_mac_addr_get(mac, 0) >= 0) {
        CPRINTF("MAC Address : %s\n", misc_mac_txt(mac, buf));
    }

    cli_cmd_mac_conf(req, 1, 1, 1);
}

static void cli_cmd_mac_age_time(cli_req_t *req )
{
    cli_cmd_mac_conf(req, 1, 0, 0);
}
static void cli_cmd_mac_learn_mode( cli_req_t *req )
{
    cli_cmd_mac_conf(req, 0, 1, 0);
}


static void mac_cli_cmd_mac_add (cli_req_t *req )
{
    vtss_usid_t           usid;
    vtss_isid_t           isid;
    int                   i;
    mac_mgmt_addr_entry_t mac_entry;
    mac_mgmt_addr_entry_t return_mac;
    BOOL                  found_port = 0;
    port_iter_t           pit;
    switch_iter_t         sit;

    if (cli_cmd_switch_none(req) || cli_cmd_conf_slave(req)) {
        return;
    }

    if  (req->stack.count > 1) {
        if  (req->usid_sel == VTSS_USID_ALL) {
            CPRINTF("Please select a switch for the mac add operation\n");
            return;
        }
    }

    memset(&mac_entry, 0, sizeof(mac_entry));

    mac_entry.vid_mac.vid = req->vid;
    for (i = 0; i < 6; i++) {
        mac_entry.vid_mac.mac.addr[i] = req->mac_addr[i];
    }

    (void)switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID);
    while (switch_iter_getnext(&sit)) {
        if (mac_mgmt_static_get_next(sit.isid, &mac_entry.vid_mac, &return_mac, 0, 0) == VTSS_OK) {
            if (mac_mgmt_table_del(sit.isid, &mac_entry.vid_mac, 0) != VTSS_OK) {
                CPRINTF("Address exists but could not delete it\n");
                return;
            }
        }
    }

    for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
        if ((isid = req->stack.isid[usid]) != VTSS_ISID_END) {
            (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
            while (port_iter_getnext(&pit)) {
                mac_entry.destination[pit.iport] = (req->uport_list[iport2uport(pit.iport)] ? 1 : 0);
                if (mac_entry.destination[pit.iport]) {
                    found_port = 1;
                }
            }
            if (found_port) {
                if (mac_mgmt_table_add(isid, &mac_entry) != VTSS_OK) {
                    CPRINTF("mac table add operation failed\n");
                }
                found_port = 0;
                for (i = VTSS_PORT_NO_START; i < VTSS_PORT_NO_END; i++) {
                    mac_entry.destination[i] = 0;
                }
            } else {
                CPRINTF("(isid:%d)Invalid destination port\n", isid);
            }
        }
    }
}

static void mac_cli_cmd_mac_del ( cli_req_t *req )
{
    vtss_usid_t           usid;
    vtss_isid_t           isid;
    int                   i;
    vtss_vid_mac_t        vid_mac;

    if (cli_cmd_switch_none(req) || cli_cmd_conf_slave(req)) {
        return;
    }

    if  (req->stack.count > 1) {
        if  (req->usid_sel == VTSS_USID_ALL) {
            CPRINTF("Please select a switch for the mac del operation\n");
            return;
        }
    }

    vid_mac.vid = req->vid;
    for (i = 0; i < 6; i++) {
        vid_mac.mac.addr[i] = req->mac_addr[i];
    }

    for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
        if ((isid = req->stack.isid[usid]) != VTSS_ISID_END)
            if (mac_mgmt_table_del(isid, &vid_mac, 0) != VTSS_OK) {
                CPRINTF("mac table del operation failed\n");
            }
    }
}

static void mac_cli_cmd_mac_lookup ( cli_req_t *req )
{
    vtss_usid_t            usid;
    vtss_isid_t            isid;
    int                    i;
    vtss_vid_mac_t         vid_mac;
    vtss_mac_table_entry_t mac_entry;

    if (cli_cmd_switch_none(req)) {
        return;
    }

    vid_mac.vid = req->vid;
    for (i = 0; i < 6; i++) {
        vid_mac.mac.addr[i] = req->mac_addr[i];
    }

    for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
        if ((isid = req->stack.isid[usid]) == VTSS_ISID_END) {
            continue;
        }

        cli_cmd_usid_print(usid, req, 0);

        if (mac_mgmt_table_get_next(isid, &vid_mac, &mac_entry, 0) == VTSS_OK) {
            cli_mac_entry_print(&mac_entry, 1);
        } else {
            CPRINTF("MAC address not found\n");
        }
    }

}

static void mac_cli_cmd_mac_dump ( cli_req_t *req )
{
    vtss_usid_t            usid;
    vtss_isid_t            isid;
    ulong                  i, max, first;
    vtss_vid_mac_t         vid_mac;
    vtss_mac_table_entry_t mac_entry;
    mac_cli_req_t *mac_req = NULL;


    if (cli_cmd_switch_none(req)) {
        return;
    }

    mac_req = req->module_req;

    max = mac_req->mac_max;
    if (max == 0) {
        max = 0xFFFFFFFF;
    }
    for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
        if ((isid = req->stack.isid[usid]) == VTSS_ISID_END) {
            continue;
        }

        vid_mac.vid = (req->vid_spec == CLI_SPEC_VAL ? req->vid : 1);
        for (i = 0; i < 6; i++) {
            vid_mac.mac.addr[i] = req->mac_addr[i];
        }

        first = 1;
        for (i = 0; i < max; i++) {
            if (i == 0) {
                if (mac_mgmt_table_get_next(isid, &vid_mac, &mac_entry, FALSE) != VTSS_OK) {
                    // If lookup wasn't found do a lookup of the next entry
                    if (mac_mgmt_table_get_next(isid, &vid_mac, &mac_entry, TRUE) != VTSS_OK) {
                        break ;
                    }
                }
            } else {
                if (mac_mgmt_table_get_next(isid, &vid_mac, &mac_entry, 1) != VTSS_OK) {
                    break;
                }
            }

            if (first) {
                cli_cmd_usid_print(usid, req, 0);
            }

            cli_mac_entry_print(&mac_entry, first);
            first = 0;
            vid_mac = mac_entry.vid_mac;
        }
    }
}

static void mac_cli_cmd_debug_stack_dump ( cli_req_t *req )
{
    ulong                  i, max, first;
    vtss_vid_mac_t         vid_mac;
    mac_cli_req_t *mac_req = NULL;
    mac_mgmt_table_stack_t stack_entry_next;
    mac_mgmt_addr_type_t mac_type;


    if (cli_cmd_switch_none(req)) {
        return;
    }

    mac_req = req->module_req;

    max = mac_req->mac_max;
    if (max == 0) {
        max = 0xFFFFFFFF;
    }

    memset(&vid_mac, 0, sizeof(vid_mac));
    memset(&mac_type, 0, sizeof(mac_type));
    first = 1;
    for (i = 0; i < max; i++) {
        if (mac_mgmt_stack_get_next(&vid_mac, &stack_entry_next, &mac_type, 1) != VTSS_OK) {
            return;
        }
        cli_mac_entry_print_stack(&stack_entry_next, first);
        first = 0;
        vid_mac = stack_entry_next.vid_mac;
    }
}

static void mac_cli_cmd_mac_stats ( cli_req_t *req )
{
    vtss_usid_t       usid;
    vtss_isid_t       isid;
    vtss_uport_no_t   uport;
    vtss_port_no_t    iport;
    mac_table_stats_t stats;
    BOOL              found_port = 0;

    if (cli_cmd_switch_none(req)) {
        return;
    }

    for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
        if ((isid = req->stack.isid[usid]) == VTSS_ISID_END ||
            mac_mgmt_table_stats_get(isid, &stats) != VTSS_OK) {
            continue;
        }

        cli_cmd_usid_print(usid, req, 0);

        cli_table_header("Port  Dynamic Addresses");
        for (iport = VTSS_PORT_NO_START; iport < port_isid_port_count(isid); iport++) {
            uport = iport2uport(iport);
            if (req->uport_list[uport] == 0) {
                continue;
            }

            CPRINTF("%-2d    %d\n", uport, stats.learned[iport]);
            found_port = 1;
        }
        if (found_port) {
            CPRINTF("\nTotal Dynamic Addresses: %d\n", stats.learned_total);
            CPRINTF("Total Static Addresses : %d\n", stats.static_total);
        } else {
            CPRINTF("(isid:%d)Invalid port\n", isid);
        }
        found_port = 0;
    }
}

static int cli_parse_age_time ( char *cmd, char *cmd2, char *stx, char *cmd_org,
                                cli_req_t *req)
{
    mac_cli_req_t *mac_req = NULL;
    mac_req = req->module_req;
    req->parm_parsed = 1;
    return ((cli_parse_ulong(cmd, &mac_req->mac_age_time, 0, MAC_AGE_TIME_MAX) ||
             (mac_req->mac_age_time != 0 && mac_req->mac_age_time < MAC_AGE_TIME_MIN)));


}
static int cli_parse_mac_max ( char *cmd, char *cmd2, char *stx, char *cmd_org,
                               cli_req_t *req)
{
    mac_cli_req_t *mac_req = NULL;
    mac_req = req->module_req;
    req->parm_parsed = 1;
    return ( cli_parse_ulong(cmd, &mac_req->mac_max, 1, 0xffffffff));
}

static int cli_parse_mac_addr ( char *cmd, char *cmd2, char *stx, char *cmd_org,
                                cli_req_t *req)
{
    req->parm_parsed = 1;
    return ( cli_parse_mac(cmd, req->mac_addr, &req->mac_addr_spec, 0));
}

static int cli_parse_mac_port_list ( char *cmd, char *cmd2, char *stx, char *cmd_org,
                                     cli_req_t *req)
{
    int error = 0; /* As a start there is no error */
    ulong min = 1, max = VTSS_PORTS;

    req->parm_parsed = 1;

    error = (cli_parse_all(cmd) && cli_parm_parse_list(cmd, req->uport_list, min, max, 1));
    if (error && (error = cli_parse_none(cmd)) == 0) {
        vtss_uport_no_t uport;

        for (uport = min; uport < max; uport++) {
            req->uport_list[uport] = 0;
        }

    }

    return (error);
}

int32_t
cli_mac_vlan_parse ( char *cmd, char *cmd2,
                     char *stx, char *cmd_org,
                     cli_req_t *req)
{
    ulong error = 0;
    ulong value = 0;
    req->parm_parsed = 1;

    error = cli_parse_ulong(cmd, &value, VLAN_ID_MIN, VLAN_ID_MAX);
    if (!error) {
        req->vid_spec = CLI_SPEC_VAL;
        req->vid = value;
    }

    return error;
}

int32_t cli_generic_mac_parse (char *cmd, char *cmd2, char *stx, char *cmd_org,
                               cli_req_t *req)
{
    char *found = cli_parse_find(cmd, stx);
    mac_cli_req_t    *mac_req = NULL;
    BOOL error = 1;

    mac_req = req->module_req;
    req->parm_parsed = 1;

    if (found != NULL) {
        if (!strncmp(found, "auto", 4)) {
            error = 0;
            mac_req->auto_keyword = 1;
        } else if ( !strncmp(found, "secure", 6)) {
            mac_req->secure = 1;
            error = 0;
        } else if ( !strncmp(found, "disable", 7)) {
            error = 0;
        } else {
            error = 1;
        }
    }
    return (error);
}

static cli_parm_t mac_cli_parm_table[] = {
    {
        "<port_list>",
        "Port list or 'all' or 'none'",
        CLI_PARM_FLAG_NONE,
        cli_parse_mac_port_list,
        mac_cli_cmd_mac_add
    },
    {
        "<mac_addr>",
        "First MAC address ('xx-xx-xx-xx-xx-xx' or 'xx.xx.xx.xx.xx.xx' or 'xxxxxxxxxxxx', x is a hexadecimal digit), default: MAC address zero",
        CLI_PARM_FLAG_SET,
        cli_parse_mac_addr,
        mac_cli_cmd_mac_dump
    },
    {
        "<mac_max>",
        "Maximum number of MAC addresses, default: Show all addresses",
        CLI_PARM_FLAG_SET,
        cli_parse_mac_max,
        NULL
    },
    {
        "<age_time>",
        "MAC address age time (0,10-1000000) 0=disable, default: Show age time",
        CLI_PARM_FLAG_SET,
        cli_parse_age_time,
        cli_cmd_mac_age_time
    },
    {
        "<vid>",
        "VLAN ID (1-4095), default: 1",
        CLI_PARM_FLAG_SET,
        cli_mac_vlan_parse,
        mac_cli_cmd_mac_add
    },
    {
        "<vid>",
        "VLAN ID (1-4095), default: 1",
        CLI_PARM_FLAG_SET,
        cli_mac_vlan_parse,
        mac_cli_cmd_mac_del
    },
    {
        "<vid>",
        "VLAN ID (1-4095), default: 1",
        CLI_PARM_FLAG_SET,
        cli_mac_vlan_parse,
        mac_cli_cmd_mac_lookup
    },
    {
        "<vid>",
        "First VLAN ID (1-4095), default: 1",
        CLI_PARM_FLAG_SET,
        cli_mac_vlan_parse,
        mac_cli_cmd_mac_dump
    },
    {
        "auto|disable|secure",
        "auto   : Automatic learning\n"
        "disable: Disable learning\n"
        "secure : Secure learning\n"
        "(default: Show learn mode)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_generic_mac_parse,
        NULL
    },
    {
        NULL,
        NULL,
        0,
        0,
        NULL
    }
};

enum {
    PRIO_MAC_CONF,
    PRIO_MAC_ADD,
    PRIO_MAC_DEL,
    PRIO_MAC_LOOKUP,
    PRIO_MAC_AGE_TIME,
    PRIO_MAC_LEARN_MODE,
    PRIO_MAC_DUMP,
    PRIO_MAC_STATS,
    PRIO_MAC_FLUSH,
    PRIO_DEBUG_MAC_DUMP = CLI_CMD_SORT_KEY_DEFAULT,
    PRIO_DEBUG_MAC_ADD_VOL = CLI_CMD_SORT_KEY_DEFAULT,
    PRIO_DEBUG_MAC_DEL_VOL = CLI_CMD_SORT_KEY_DEFAULT,
    PRIO_DEBUG_MAC_EAT = CLI_CMD_SORT_KEY_DEFAULT,
};


/* Command table entries */
cli_cmd_tab_entry (
    "MAC Configuration [<port_list>]",
    NULL,
    "Show MAC address table configuration",
    PRIO_MAC_CONF,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_MAC,
    cli_cmd_mac_config,
    NULL,
    mac_cli_parm_table,
    CLI_CMD_FLAG_SYS_CONF
);

cli_cmd_tab_entry (
    NULL,
    "MAC Add <mac_addr> <port_list> [<vid>]",
    "Add MAC address table entry",
    PRIO_MAC_ADD,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_MAC,
    mac_cli_cmd_mac_add,
    NULL,
    mac_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    NULL,
    "MAC Delete <mac_addr> [<vid>]",
    "Delete MAC address entry",
    PRIO_MAC_DEL,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_MAC,
    mac_cli_cmd_mac_del,
    NULL,
    mac_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "MAC Lookup <mac_addr> [<vid>]",
    NULL,
    "Lookup MAC address entry",
    PRIO_MAC_LOOKUP,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_MAC,
    mac_cli_cmd_mac_lookup,
    NULL,
    mac_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "MAC Agetime",
    "MAC Agetime [<age_time>]",
    "Set or show the MAC address age timer",
    PRIO_MAC_AGE_TIME,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_MAC,
    cli_cmd_mac_age_time,
    NULL,
    mac_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "MAC Learning [<port_list>]",
    "MAC Learning [<port_list>] [auto|disable|secure]",
    "Set or show the port learn mode",
    PRIO_MAC_LEARN_MODE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_MAC,
    cli_cmd_mac_learn_mode,
    NULL,
    mac_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "MAC Dump [<mac_max>] [<mac_addr>] [<vid>]",
    NULL,
    "Show sorted list of MAC address entries",
    PRIO_MAC_DUMP,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_MAC,
    mac_cli_cmd_mac_dump,
    NULL,
    mac_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "MAC Statistics [<port_list>]",
    NULL,
    "Show MAC address table statistics",
    PRIO_MAC_STATS,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_MAC,
    mac_cli_cmd_mac_stats,
    NULL,
    mac_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    NULL,
    "MAC Flush",
    "Flush all learned entries",
    PRIO_MAC_FLUSH,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_MAC,
    mac_mgmt_table_cli_flush,
    NULL,
    mac_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "Debug MAC Dump [<mac_max>] [<mac_addr>] [<vid>]",
    NULL,
    "Show MAC address table",
    PRIO_DEBUG_MAC_DUMP,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_mac_dump,
    NULL,
    mac_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "Debug MAC stack dump",
    NULL,
    "Show MAC address table accross stack",
    PRIO_DEBUG_MAC_DUMP,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    mac_cli_cmd_debug_stack_dump,
    NULL,
    mac_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    NULL,
    "Debug MAC Voladd <mac_addr> <port_cpu_list> [<vid>]",
    "Add MAC address table entry",
    PRIO_DEBUG_MAC_ADD_VOL,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_mac_add_vol,
    NULL,
    mac_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    NULL,
    "Debug MAC Voldel <mac_addr>",
    "Delete MAC address entry",
    PRIO_DEBUG_MAC_DEL_VOL,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_mac_del_vol,
    NULL,
    mac_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    NULL,
    "Debug MAC Voleat <mac_addr> <port_list> <integer> [<vid>]",
    "Add some volatile entries",
    PRIO_DEBUG_MAC_EAT,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_mac_eat,
    NULL,
    mac_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    NULL,
    "Debug MAC Eat <mac_addr> <port_list> <integer> [<vid>]",
    "Add some static entries",
    PRIO_DEBUG_MAC_EAT,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_mac_static_eat,
    NULL,
    mac_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    NULL,
    "Debug MAC Del <mac_addr> <integer> [<vid>]",
    "Delete MAC address entry",
    PRIO_DEBUG_MAC_DEL_VOL,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_mac_del,
    NULL,
    mac_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

