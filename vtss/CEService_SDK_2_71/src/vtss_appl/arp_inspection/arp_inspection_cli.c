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
#include "cli_grp_help.h"
#include "arp_inspection_api.h"
#include "arp_inspection_cli.h"
#include "vtss_module_id.h"
#include "cli_trace_def.h"
#include "port_api.h"
#include "vlan_api.h"

typedef struct {
    /* Keywords */
    BOOL    del;
    BOOL    add;
    BOOL    none;
    BOOL    deny;
    BOOL    permit;
    BOOL    all;
    BOOL    all_complete_commands;
} arp_inspection_cli_req_t;

void arp_insp_cli_req_init(void)
{
    /* register the size required for arp inspection req. structure */
    cli_req_size_register(sizeof(arp_inspection_cli_req_t));
}

static char *cli_cmd_arp_inspection_log_txt(arp_inspection_log_type_t type)
{
    switch (type) {
    case ARP_INSPECTION_LOG_NONE:
        return "NONE";
    case ARP_INSPECTION_LOG_DENY:
        return "DENY";
    case ARP_INSPECTION_LOG_PERMIT:
        return "PERMIT";
    case ARP_INSPECTION_LOG_ALL:
        return "ALL";
    }

    return "NONE";
}

static char *cli_cmd_arp_inspection_vlan_log_txt(u8 flags)
{
    if ((flags & ARP_INSPECTION_VLAN_LOG_DENY) && (flags & ARP_INSPECTION_VLAN_LOG_PERMIT)) {
        return "ALL";
    } else if (flags & ARP_INSPECTION_VLAN_LOG_DENY) {
        return "DENY";
    } else if (flags & ARP_INSPECTION_VLAN_LOG_PERMIT) {
        return "PERMIT";
    } else {
        return "NONE";
    }
}

static void cli_cmd_arp_inspection_conf(cli_req_t *req,
                                        BOOL arp_inspection_mode,
                                        BOOL arp_inspection_port_mode,
                                        BOOL arp_inspection_port_vlan_mode,
                                        BOOL arp_inspection_port_log,
                                        BOOL static_entry_set,
                                        BOOL status,
                                        BOOL translate_dynamic_entries)
{
    int                             i;
    vtss_rc                         rc;
    vtss_isid_t                     isid;
    vtss_uport_no_t                 uport;
    u32                             mode;
    arp_inspection_entry_t          entry;
    arp_inspection_port_mode_conf_t port_mode_conf;
    arp_inspection_cli_req_t        *arp_ips_req;
    switch_iter_t                   sit;
    port_iter_t                     pit;
    char                            buf[80], buf1[80], buf2[80], *p;
    BOOL                            first;

    arp_ips_req = req->module_req;

    if (cli_cmd_switch_none(req) ||
        cli_cmd_conf_slave(req) ||
        arp_inspection_mgmt_conf_mode_get(&mode) != VTSS_OK) {
        return;
    }

    if (req->set) {
        if (arp_inspection_mode) {
            mode = req->enable;
            if ((rc = arp_inspection_mgmt_conf_mode_set(&mode)) != VTSS_OK) {
                CPRINTF("%s\n", error_txt(rc));
            }
        }
    } else {
        if (arp_inspection_mode) {
            CPRINTF("ARP Inspection Mode : %s\n", cli_bool_txt(mode));
        }
    }

    if (arp_inspection_port_mode || arp_inspection_port_log || arp_inspection_port_vlan_mode) {

        (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_USID);
        while (switch_iter_getnext(&sit)) {
            if ((isid = req->stack.isid[sit.usid]) == VTSS_ISID_END ||
                arp_inspection_mgmt_conf_port_mode_get(isid, &port_mode_conf) != VTSS_OK ) {
                continue;
            }
            first = TRUE;

            (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT);
            while (port_iter_getnext(&pit)) {
                uport = iport2uport(pit.iport);
                if (req->uport_list[uport] == 0) {
                    continue;
                }
                if (req->set) {
                    if (arp_inspection_port_mode) {
                        port_mode_conf.mode[pit.iport] = req->enable;
                    }
                    if (arp_inspection_port_vlan_mode) {
                        port_mode_conf.check_VLAN[pit.iport] = req->enable;
                    }
                    if (arp_inspection_port_log) {
                        if (arp_ips_req->none) {
                            port_mode_conf.log_type[pit.iport] = ARP_INSPECTION_LOG_NONE;
                        } else if (arp_ips_req->deny) {
                            port_mode_conf.log_type[pit.iport] = ARP_INSPECTION_LOG_DENY;
                        } else if (arp_ips_req->permit) {
                            port_mode_conf.log_type[pit.iport] = ARP_INSPECTION_LOG_PERMIT;
                        } else if (arp_ips_req->all) {
                            port_mode_conf.log_type[pit.iport] = ARP_INSPECTION_LOG_ALL;
                        }
                    }
                } else {
                    if (first) {
                        first = FALSE;
                        cli_cmd_usid_print(sit.usid, req, 1);
                        p = &buf[0];
                        p += sprintf(p, "Port  ");
                        if (arp_inspection_port_mode) {
                            p += sprintf(p, "Port Mode  ");
                        }
                        if (arp_inspection_port_vlan_mode) {
                            p += sprintf(p, "Check VLAN  ");
                        }
                        if (arp_inspection_port_log) {
                            p += sprintf(p, "Log Type  ");
                        }
                        cli_table_header(buf);
                    }
                    CPRINTF("%-2u    ", uport);
                    if (arp_inspection_port_mode) {
                        CPRINTF("%s    ", cli_bool_txt(port_mode_conf.mode[pit.iport]));
                    }
                    if (arp_inspection_port_vlan_mode) {
                        CPRINTF("%s    ", cli_bool_txt(port_mode_conf.check_VLAN[pit.iport]));
                    }
                    if (arp_inspection_port_log) {
                        CPRINTF("%s    ", cli_cmd_arp_inspection_log_txt(port_mode_conf.log_type[pit.iport]));
                    }
                    CPRINTF("\n");
                }
            }
            if (req->set) {
                if (arp_inspection_port_mode || arp_inspection_port_log || arp_inspection_port_vlan_mode) {
                    if ((rc = arp_inspection_mgmt_conf_port_mode_set(isid, &port_mode_conf)) != VTSS_OK) {
                        CPRINTF("%s\n", error_txt(rc));
                    }
                }
            }
        }
    }

    if (static_entry_set) {
        (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_USID);
        while (switch_iter_getnext(&sit)) {
            if ((isid = req->stack.isid[sit.usid]) == VTSS_ISID_END) {
                continue;
            }
            first = TRUE;
            (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT);
            while (port_iter_getnext(&pit)) {
                uport = iport2uport(pit.iport);
                if (req->uport_list[uport] == 0) {
                    continue;
                }
                if (req->set) {
                    for (i = 0; i < 6; i++) {
                        entry.mac[i] = req->mac_addr[i];
                    }
                    entry.vid = req->vid;
                    entry.assigned_ip = req->ipv4_addr;
                    entry.isid = isid;
                    entry.port_no = pit.iport;
                    entry.type = ARP_INSPECTION_STATIC_TYPE;
                    entry.valid = TRUE;
                    if (arp_ips_req->del) {
                        if ((rc = arp_inspection_mgmt_conf_static_entry_del(&entry)) != VTSS_OK) {
                            CPRINTF("%s\n", error_txt(rc));
                        }
                    } else {
                        if ((rc = arp_inspection_mgmt_conf_static_entry_set(&entry)) != VTSS_OK) {
                            CPRINTF("%s\n", error_txt(rc));
                        }
                    }
                }
            }
        }
    }

    if (status) {
        (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_USID);
        while (switch_iter_getnext(&sit)) {
            if ((isid = req->stack.isid[sit.usid]) == VTSS_ISID_END) {
                continue;
            }
            first = TRUE;

            (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT);
            while (port_iter_getnext(&pit)) {
                uport = iport2uport(pit.iport);
                if (req->uport_list[uport] == 0) {
                    continue;
                }
                if (first) {
                    CPRINTF("%s", "\nARP Inspection Entry Table:\n");
                    first = FALSE;
                    cli_cmd_usid_print(sit.usid, req, 1);
                    p = &buf[0];
                    p += sprintf(p, "Type     ");
                    p += sprintf(p, "Port  ");
                    p += sprintf(p, "VLAN  ");
                    p += sprintf(p, "MAC Address        ");
                    p += sprintf(p, "IP Address     ");
                    cli_table_header(buf);
                }

                // Get static entries
                if (arp_inspection_mgmt_conf_static_entry_get(&entry, FALSE) == VTSS_OK) {
                    if ((entry.isid == isid) && (entry.port_no == pit.iport)) {
                        CPRINTF("Static   %4d  %4d  %17s  %-15s\n",
                                iport2uport(entry.port_no),
                                entry.vid,
                                misc_mac_txt(entry.mac, buf1),
                                misc_ipv4_txt(entry.assigned_ip, buf2));
                    }
                    while (arp_inspection_mgmt_conf_static_entry_get(&entry, TRUE) == VTSS_OK) {
                        if ((entry.isid == isid) && (entry.port_no == pit.iport)) {
                            CPRINTF("Static   %4d  %4d  %17s  %-15s\n",
                                    iport2uport(entry.port_no),
                                    entry.vid,
                                    misc_mac_txt(entry.mac, buf1),
                                    misc_ipv4_txt(entry.assigned_ip, buf2));
                        }
                    }
                }

                // Get dynamic entries
                if (arp_inspection_mgmt_conf_dynamic_entry_get(&entry, FALSE) == VTSS_OK) {
                    if ((entry.isid == isid) && (entry.port_no == pit.iport)) {
                        CPRINTF("Dynamic  %4d  %4d  %17s  %-15s\n",
                                iport2uport(entry.port_no),
                                entry.vid,
                                misc_mac_txt(entry.mac, buf1),
                                misc_ipv4_txt(entry.assigned_ip, buf2));
                    }
                    while (arp_inspection_mgmt_conf_dynamic_entry_get(&entry, TRUE) == VTSS_OK) {
                        if ((entry.isid == isid) && (entry.port_no == pit.iport)) {
                            CPRINTF("Dynamic  %4d  %4d  %17s  %-15s\n",
                                    iport2uport(entry.port_no),
                                    entry.vid,
                                    misc_mac_txt(entry.mac, buf1),
                                    misc_ipv4_txt(entry.assigned_ip, buf2));
                        }
                    }
                }
            }
        }
    }

    /* translate dynamic entries into static entries */
    if (translate_dynamic_entries) {

        // translate all entries
        if (!req->parm_parsed) {

            if ((rc = arp_inspection_mgmt_conf_translate_dynamic_into_static()) >= VTSS_OK) {
                CPRINTF("ARP Inspection:\n\tTranslate %d dynamic entries into static entries.\n", rc);
            } else {
                CPRINTF("%s\n", error_txt(rc));
            }

        } else {

            // translate single entry
            (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_USID);
            while (switch_iter_getnext(&sit)) {
                if ((isid = req->stack.isid[sit.usid]) == VTSS_ISID_END) {
                    continue;
                }
                (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT);
                while (port_iter_getnext(&pit)) {
                    uport = iport2uport(pit.iport);
                    if (req->uport_list[uport] == 0) {
                        continue;
                    }
                    if (req->set) {
                        for (i = 0; i < 6; i++) {
                            entry.mac[i] = req->mac_addr[i];
                        }
                        entry.vid = req->vid;
                        entry.assigned_ip = req->ipv4_addr;
                        entry.isid = isid;
                        entry.port_no = pit.iport;
                        entry.type = ARP_INSPECTION_DYNAMIC_TYPE;
                        entry.valid = TRUE;

                        // Get static entries
                        if (arp_inspection_mgmt_conf_dynamic_entry_check(&entry) == VTSS_OK) {
                            if ((rc = arp_inspection_mgmt_conf_translate_dynamic_entry_into_static_entry(&entry)) < VTSS_OK) {
                                CPRINTF("%s\n", error_txt(rc));
                            } else {
                                CPRINTF("ARP Inspection:\n\tTranslate %d dynamic entries into static entries.\n", rc);
                            }
                        } else {
                            CPRINTF("ARP Inspection:\n\tDon't find the dynamic entry.\n");
                        }
                    }
                }
            }
        }
    }
}

static void cli_cmd_arp_inspection_conf_vlan(cli_req_t *req,
                                             BOOL arp_inspection_vlan,
                                             BOOL arp_inspection_vlan_log)
{
    vtss_rc                         rc;
    arp_inspection_vlan_mode_conf_t vlan_mode_conf;
    arp_inspection_cli_req_t        *arp_ips_req;
    int                             idx;
    char                            buf[80], *p;
    BOOL                            first;

    if (cli_cmd_switch_none(req) || cli_cmd_conf_slave(req)) {
        return;
    }

    arp_ips_req = req->module_req;

    if (arp_inspection_vlan || arp_inspection_vlan_log) {
        first = TRUE;
        if (req->vid == VTSS_VID_NULL) {
            for (idx = VLAN_ID_MIN; idx <= VLAN_ID_MAX; idx++) {
                // get configuration
                if (arp_inspection_mgmt_conf_vlan_mode_get(idx, &vlan_mode_conf, FALSE) != VTSS_OK) {
                    continue;
                }

                if (arp_ips_req->all_complete_commands) {
                    if (arp_inspection_vlan) {
                        if (req->enable) {
                            vlan_mode_conf.flags |= ARP_INSPECTION_VLAN_MODE;
                        } else {
                            vlan_mode_conf.flags &= ~ARP_INSPECTION_VLAN_MODE;
                        }
                    }
                    if (arp_inspection_vlan_log) {
                        if (arp_ips_req->none) {
                            vlan_mode_conf.flags &= ~ARP_INSPECTION_VLAN_LOG_DENY;
                            vlan_mode_conf.flags &= ~ARP_INSPECTION_VLAN_LOG_PERMIT;
                        } else if (arp_ips_req->deny) {
                            vlan_mode_conf.flags |= ARP_INSPECTION_VLAN_LOG_DENY;
                            vlan_mode_conf.flags &= ~ARP_INSPECTION_VLAN_LOG_PERMIT;
                        } else if (arp_ips_req->permit) {
                            vlan_mode_conf.flags &= ~ARP_INSPECTION_VLAN_LOG_DENY;
                            vlan_mode_conf.flags |= ARP_INSPECTION_VLAN_LOG_PERMIT;
                        } else if (arp_ips_req->all) {
                            vlan_mode_conf.flags |= ARP_INSPECTION_VLAN_LOG_DENY;
                            vlan_mode_conf.flags |= ARP_INSPECTION_VLAN_LOG_PERMIT;
                        }
                    }

                    if ((rc = arp_inspection_mgmt_conf_vlan_mode_set(idx, &vlan_mode_conf)) != VTSS_OK) {
                        CPRINTF("%s\n", error_txt(rc));
                    }
                } else {
                    if (vlan_mode_conf.flags & ARP_INSPECTION_VLAN_MODE) {
                        if (first) {
                            CPRINTF("%s", "\nARP Inspection VLAN Configuration:\n\n");
                            first = FALSE;
                            p = &buf[0];
                            p += sprintf(p, "VLAN    ");
                            if (arp_inspection_vlan) {
                                p += sprintf(p, "VLAN mode  ");
                            }
                            if (arp_inspection_vlan_log) {
                                p += sprintf(p, "VLAN Log Type  ");
                            }
                            cli_table_header(buf);
                        }
                        CPRINTF("%-4u    ", idx);
                        if (arp_inspection_vlan) {
                            CPRINTF("%s    ", cli_bool_txt(vlan_mode_conf.flags & ARP_INSPECTION_VLAN_MODE));
                        }
                        if (arp_inspection_vlan_log) {
                            CPRINTF("%s    ", cli_cmd_arp_inspection_vlan_log_txt(vlan_mode_conf.flags));
                        }
                        CPRINTF("\n");
                    }
                }
            }

            if (arp_ips_req->all_complete_commands) {
                // save configuration
                (void) arp_inspection_mgmt_conf_vlan_mode_save();
            }
        } else {
            if (arp_inspection_mgmt_conf_vlan_mode_get(req->vid, &vlan_mode_conf, FALSE) == VTSS_OK) {
                if (arp_ips_req->all_complete_commands) {

                    if (arp_inspection_vlan) {
                        if (req->enable) {
                            vlan_mode_conf.flags |= ARP_INSPECTION_VLAN_MODE;
                        } else {
                            vlan_mode_conf.flags &= ~ARP_INSPECTION_VLAN_MODE;
                        }
                    }

                    if (arp_inspection_vlan_log) {
                        if (arp_ips_req->none) {
                            vlan_mode_conf.flags &= ~ARP_INSPECTION_VLAN_LOG_DENY;
                            vlan_mode_conf.flags &= ~ARP_INSPECTION_VLAN_LOG_PERMIT;
                        } else if (arp_ips_req->deny) {
                            vlan_mode_conf.flags |= ARP_INSPECTION_VLAN_LOG_DENY;
                            vlan_mode_conf.flags &= ~ARP_INSPECTION_VLAN_LOG_PERMIT;
                        } else if (arp_ips_req->permit) {
                            vlan_mode_conf.flags &= ~ARP_INSPECTION_VLAN_LOG_DENY;
                            vlan_mode_conf.flags |= ARP_INSPECTION_VLAN_LOG_PERMIT;
                        } else if (arp_ips_req->all) {
                            vlan_mode_conf.flags |= ARP_INSPECTION_VLAN_LOG_DENY;
                            vlan_mode_conf.flags |= ARP_INSPECTION_VLAN_LOG_PERMIT;
                        }
                    }

                    if ((rc = arp_inspection_mgmt_conf_vlan_mode_set(req->vid, &vlan_mode_conf)) != VTSS_OK) {
                        CPRINTF("%s\n", error_txt(rc));
                    }
                    // save configuration
                    (void) arp_inspection_mgmt_conf_vlan_mode_save();
                } else {
                    if (first) {
                        first = FALSE;
                        p = &buf[0];
                        p += sprintf(p, "VLAN    ");
                        if (arp_inspection_vlan) {
                            p += sprintf(p, "VLAN mode  ");
                        }
                        if (arp_inspection_vlan_log) {
                            p += sprintf(p, "VLAN Log Type  ");
                        }
                        cli_table_header(buf);
                    }
                    CPRINTF("%-4u    ", req->vid);
                    if (arp_inspection_vlan) {
                        CPRINTF("%s    ", cli_bool_txt(vlan_mode_conf.flags & ARP_INSPECTION_VLAN_MODE));
                    }
                    if (arp_inspection_vlan_log) {
                        CPRINTF("%s    ", cli_cmd_arp_inspection_vlan_log_txt(vlan_mode_conf.flags));
                    }
                    CPRINTF("\n");
                }
            }
        }

        // fix lint error
        if (first) {
            T_D("fix lint error");
        }
    }
}

static void cli_cmd_arp_inspection_config(cli_req_t *req)
{
    arp_inspection_cli_req_t    *arp_ips_req = NULL;

    if (!req->set) {
        cli_header("ARP Inspection Configuration", 1);
    }
    cli_cmd_arp_inspection_conf(req, 1, 1, 1, 1, 1, 1, 0);

    if (!req->set) {
        arp_ips_req = req->module_req;

        req->vid = VTSS_VID_NULL;
        arp_ips_req->all_complete_commands = 0;
    }
    cli_cmd_arp_inspection_conf_vlan(req, 1, 1);
}

static void cli_cmd_arp_inspection_mode(cli_req_t *req)
{
    cli_cmd_arp_inspection_conf(req, 1, 0, 0, 0, 0, 0, 0);
}

static void cli_cmd_arp_inspection_port_mode(cli_req_t *req)
{
    cli_cmd_arp_inspection_conf(req, 0, 1, 0, 0, 0, 0, 0);
}

static void cli_cmd_arp_inspection_port_vlan_mode(cli_req_t *req)
{
    cli_cmd_arp_inspection_conf(req, 0, 0, 1, 0, 0, 0, 0);
}

static void cli_cmd_arp_inspection_port_log(cli_req_t *req)
{
    cli_cmd_arp_inspection_conf(req, 0, 0, 0, 1, 0, 0, 0);
}

static void cli_cmd_arp_inspection_entry(cli_req_t *req)
{
    cli_cmd_arp_inspection_conf(req, 0, 0, 0, 0, 1, 0, 0);
}

static void cli_cmd_arp_inspection_status(cli_req_t *req)
{
    cli_cmd_arp_inspection_conf(req, 0, 0, 0, 0, 0, 1, 0);
}

static void cli_cmd_arp_inspection_translate(cli_req_t *req)
{
    cli_cmd_arp_inspection_conf(req, 0, 0, 0, 0, 0, 0, 1);
}

static void cli_cmd_arp_inspection_vlan(cli_req_t *req)
{
    cli_cmd_arp_inspection_conf_vlan(req, 1, 0);
}

static void cli_cmd_arp_inspection_vlan_log(cli_req_t *req)
{
    cli_cmd_arp_inspection_conf_vlan(req, 0, 1);
}

static int32_t cli_generic_parse_arp_insp_entry(char *cmd, char *cmd2, char *stx, char *cmd_org,
                                                cli_req_t *req)
{
    char                        *found = cli_parse_find(cmd, stx);
    arp_inspection_cli_req_t    *arp_ips_req = NULL;

    arp_ips_req = req->module_req;
    req->parm_parsed = 1;

    if (found != NULL) {
        if (!strncmp(found, "add", 3)) {
            arp_ips_req->add = 1;
        } else if (!strncmp(found, "delete", 6)) {
            arp_ips_req->del = 1;
        } else if (!strncmp(found, "disable", 7)) {
            req->disable = 1;
            arp_ips_req->all_complete_commands = 1;
        } else if (!strncmp(found, "enable", 6)) {
            req->enable = 1;
            arp_ips_req->all_complete_commands = 1;
        } else if (!strncmp(found, "none", 4)) {
            arp_ips_req->none = 1;
            arp_ips_req->all_complete_commands = 1;
        } else if (!strncmp(found, "deny", 4)) {
            arp_ips_req->deny = 1;
            arp_ips_req->all_complete_commands = 1;
        } else if (!strncmp(found, "permit", 6)) {
            arp_ips_req->permit = 1;
            arp_ips_req->all_complete_commands = 1;
        } else if (!strncmp(found, "all", 3)) {
            arp_ips_req->all = 1;
            arp_ips_req->all_complete_commands = 1;
        }
    }

    return (found == NULL ? 1 : 0);
}

static int32_t cli_generic_parse_allowed_mac(char *cmd, char *cmd2, char *stx, char *cmd_org,
                                             cli_req_t *req)
{
    req->parm_parsed = 1;
    return (cli_parse_mac(cmd, req->mac_addr, &req->mac_addr_spec, 0));
}

static int32_t cli_generic_parse_allowed_ipv4(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    req->parm_parsed = 1;
    return (cli_parse_ipv4(cmd, &req->ipv4_addr, NULL, &req->ipv4_addr_spec, 0));
}

static int32_t cli_arp_inspection_vid_parse(char *cmd, char *cmd2, char *stx, char *cmd_org,
                                            cli_req_t *req)
{
    int     error = 0;  /* As a start there is no error */
    ulong   value = 0;

    req->parm_parsed = 1;

    error = cli_parse_ulong(cmd, &value, VLAN_ID_MIN, VLAN_ID_MAX);

    if (!error) {
        req->vid_spec = CLI_SPEC_VAL;
        req->vid = value;
    }

    return (error);
}

static void cli_arp_inspection_vlan_default_set ( cli_req_t *req )
{
    arp_inspection_cli_req_t    *arp_ips_req = NULL;

    arp_ips_req = req->module_req;

    req->vid = VTSS_VID_NULL;
    arp_ips_req->all_complete_commands = 0;
}

static cli_parm_t arp_inspection_cli_parm_table[] = {
    {
        "enable|disable",
        "enable : Enable ARP Inspection\n"
        "disable: Disable ARP Inspection",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        cli_cmd_arp_inspection_mode,
    },
    {
        "enable|disable",
        "enable  : Enable ARP Inspection port\n"
        "disable : Disable ARP Inspection port\n"
        "(default: Show ARP Inspection port mode)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        cli_cmd_arp_inspection_port_mode,
    },
    {
        "enable|disable",
        "enable  : Enable ARP Inspection port VLAN mode\n"
        "disable : Disable ARP Inspection port VLAN mode\n"
        "(default: Show ARP Inspection port VLAN mode)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        cli_cmd_arp_inspection_port_vlan_mode,
    },
    {
        "none|deny|permit|all",
        "none: Log nothing\n"
        "deny: Log denied entries\n"
        "permit: Log permitted entries\n"
        "all: Log all entries\n"
        "(default: Show ARP Inspection Log Type)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_generic_parse_arp_insp_entry,
        cli_cmd_arp_inspection_port_log,
    },
    {
        "<vid>",
        "VLAN ID (1-4095)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_arp_inspection_vid_parse,
        cli_cmd_arp_inspection_vlan,
    },
    {
        "enable|disable",
        "enable  : Enable ARP Inspection VLAN\n"
        "disable : Disable ARP Inspection VLAN\n"
        "(default: Show ARP Inspection VLAN database)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_generic_parse_arp_insp_entry,
        cli_cmd_arp_inspection_vlan,
    },
    {
        "<vid>",
        "VLAN ID (1-4095)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_arp_inspection_vid_parse,
        cli_cmd_arp_inspection_vlan_log,
    },
    {
        "none|deny|permit|all",
        "none: Log nothing\n"
        "deny: Log denied entries\n"
        "permit: Log permitted entries\n"
        "all: Log all entries\n"
        "(default: Show ARP Inspection VLAN Log Type)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_generic_parse_arp_insp_entry,
        cli_cmd_arp_inspection_vlan_log,
    },
    {
        "add|delete",
        "add    : Add new port ARP inspection static entry\n"
        "delete : Delete existing port ARP inspection static entry",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_generic_parse_arp_insp_entry,
        cli_cmd_arp_inspection_entry,
    },
    {
        "<vid>",
        "VLAN ID (1-4095)",
        CLI_PARM_FLAG_SET,
        cli_arp_inspection_vid_parse,
        cli_cmd_arp_inspection_entry,
    },
    {
        "<allowed_mac>",
        "MAC address ('xx-xx-xx-xx-xx-xx' or 'xx.xx.xx.xx.xx.xx' or 'xxxxxxxxxxxx', x is a hexadecimal digit), MAC address allowed for doing ARP request",
        CLI_PARM_FLAG_SET,
        cli_generic_parse_allowed_mac,
        cli_cmd_arp_inspection_entry,
    },
    {
        "<allowed_ip>",
        "IPv4 address (a.b.c.d), IP address allowed for doing ARP request",
        CLI_PARM_FLAG_SET,
        cli_generic_parse_allowed_ipv4,
        cli_cmd_arp_inspection_entry,
    },
    {
        "<vid>",
        "VLAN ID (1-4095)",
        CLI_PARM_FLAG_SET,
        cli_arp_inspection_vid_parse,
        cli_cmd_arp_inspection_translate,
    },
    {
        "<allowed_mac>",
        "MAC address ('xx-xx-xx-xx-xx-xx' or 'xx.xx.xx.xx.xx.xx' or 'xxxxxxxxxxxx', x is a hexadecimal digit), MAC address allowed for doing ARP request",
        CLI_PARM_FLAG_SET,
        cli_generic_parse_allowed_mac,
        cli_cmd_arp_inspection_translate,
    },
    {
        "<allowed_ip>",
        "IPv4 address (a.b.c.d), IP address allowed for doing ARP request",
        CLI_PARM_FLAG_SET,
        cli_generic_parse_allowed_ipv4,
        cli_cmd_arp_inspection_translate,
    },
    {
        NULL,
        NULL,
        0,
        0,
        NULL
    },

};

enum {
    PRIO_ARP_INSPECTION_CONF,
    PRIO_ARP_INSPECTION_MODE,
    PRIO_ARP_INSPECTION_PORT_MODE,
    PRIO_ARP_INSPECTION_PORT_VLAN_MODE,
    PRIO_ARP_INSPECTION_PORT_LOG,
    PRIO_ARP_INSPECTION_VLAN,
    PRIO_ARP_INSPECTION_VLAN_LOG,
    PRIO_ARP_INSPECTION_STATIC_ENTRY_SET,
    PRIO_ARP_INSPECTION_STATUS,
    PRIO_ARP_INSPECTION_TRANSLATE
};

/* Command table entries */
cli_cmd_tab_entry (
    VTSS_CLI_GRP_SEC_NETWORK_PATH "ARP Inspection Configuration",
    NULL,
    "Show ARP inspection configuration",
    PRIO_ARP_INSPECTION_CONF,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_SECURITY,
    cli_cmd_arp_inspection_config,
    NULL,
    arp_inspection_cli_parm_table,
    CLI_CMD_FLAG_SYS_CONF
);

cli_cmd_tab_entry (
    VTSS_CLI_GRP_SEC_NETWORK_PATH "ARP Inspection Mode",
    VTSS_CLI_GRP_SEC_NETWORK_PATH "ARP Inspection Mode [enable|disable]",
    "Set or show ARP inspection mode",
    PRIO_ARP_INSPECTION_MODE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SECURITY,
    cli_cmd_arp_inspection_mode,
    NULL,
    arp_inspection_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    VTSS_CLI_GRP_SEC_NETWORK_PATH "ARP Inspection Port Mode [<port_list>]",
    VTSS_CLI_GRP_SEC_NETWORK_PATH "ARP Inspection Port Mode [<port_list>] [enable|disable]",
    "Set or show the ARP Inspection port mode",
    PRIO_ARP_INSPECTION_PORT_MODE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SECURITY,
    cli_cmd_arp_inspection_port_mode,
    NULL,
    arp_inspection_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    VTSS_CLI_GRP_SEC_NETWORK_PATH "ARP Inspection Port VLAN Mode [<port_list>]",
    VTSS_CLI_GRP_SEC_NETWORK_PATH "ARP Inspection Port VLAN Mode [<port_list>] [enable|disable]",
    "Set or show the ARP Inspection port VLAN mode",
    PRIO_ARP_INSPECTION_PORT_VLAN_MODE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SECURITY,
    cli_cmd_arp_inspection_port_vlan_mode,
    NULL,
    arp_inspection_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    VTSS_CLI_GRP_SEC_NETWORK_PATH "ARP Inspection Port Log [<port_list>]",
    VTSS_CLI_GRP_SEC_NETWORK_PATH "ARP Inspection Port Log [<port_list>] [none|deny|permit|all]",
    "Set or show the ARP Inspection port log type",
    PRIO_ARP_INSPECTION_PORT_LOG,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SECURITY,
    cli_cmd_arp_inspection_port_log,
    NULL,
    arp_inspection_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    VTSS_CLI_GRP_SEC_NETWORK_PATH "ARP Inspection VLAN [<vid>]",
    VTSS_CLI_GRP_SEC_NETWORK_PATH "ARP Inspection VLAN [<vid>] [enable|disable]",
    "Set or show the ARP Inspection VLAN database",
    PRIO_ARP_INSPECTION_VLAN,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SECURITY,
    cli_cmd_arp_inspection_vlan,
    cli_arp_inspection_vlan_default_set,
    arp_inspection_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    VTSS_CLI_GRP_SEC_NETWORK_PATH "ARP Inspection VLAN Log [<vid>]",
    VTSS_CLI_GRP_SEC_NETWORK_PATH "ARP Inspection VLAN Log [<vid>] [none|deny|permit|all]",
    "Set or show the ARP Inspection VLAN database",
    PRIO_ARP_INSPECTION_VLAN_LOG,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SECURITY,
    cli_cmd_arp_inspection_vlan_log,
    cli_arp_inspection_vlan_default_set,
    arp_inspection_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    NULL,
    VTSS_CLI_GRP_SEC_NETWORK_PATH "ARP Inspection Entry [<port_list>] add|delete\n"
    "        <vid> <allowed_mac> <allowed_ip>",
    "Add or delete ARP inspection static entry",
    PRIO_ARP_INSPECTION_STATIC_ENTRY_SET,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SECURITY,
    cli_cmd_arp_inspection_entry,
    NULL,
    arp_inspection_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    VTSS_CLI_GRP_SEC_NETWORK_PATH "ARP Inspection Status [<port_list>]",
    NULL,
    "Show ARP inspection static and dynamic entries",
    PRIO_ARP_INSPECTION_STATUS,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_SECURITY,
    cli_cmd_arp_inspection_status,
    NULL,
    arp_inspection_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    VTSS_CLI_GRP_SEC_NETWORK_PATH "ARP Inspection Translation [(<port_list> <vid> <allowed_mac> <allowed_ip>)]",
    NULL,
    "Translate ARP inspection dynamic entries into static entries",
    PRIO_ARP_INSPECTION_TRANSLATE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_SECURITY,
    cli_cmd_arp_inspection_translate,
    NULL,
    arp_inspection_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
