/*

   Vitesse VCL CLI software.

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

#include "cli.h"
#include "cli_api.h"
#include "mgmt_api.h"
#include "vcl_api.h"
#include "misc_api.h"

typedef struct {
    BOOL                   all;
    BOOL                   usr_valid;
    BOOL                   is_etype;
    BOOL                   is_oui;
    BOOL                   vce_spec;
    u8                     dsap;
    u8                     ssap;
    u8                     mask_bits;
    u16                    ether_type;
    u16                    pid;
    u16                    vce_id;
    u8                     oui[OUI_SIZE];
    i8                     group_id[MAX_GROUP_NAME_LEN];
    vcl_mac_vlan_user_t    vcl_mac_user;
    cli_spec_t             sip_spec;
    ulong                  src_ip;
    ulong                  src_mask;
} vcl_cli_req_t;

void vcl_cli_req_init(void)
{
    /* register the size required for VCL req. structure */
    cli_req_size_register(sizeof(vcl_cli_req_t));
}

void vcl_ipvlan_cli_req_conf_default_set(cli_req_t *req)
{
    vcl_cli_req_t *vcl_req = req->module_req;
    vcl_req->vce_id = 0;
    vcl_req->vce_spec = FALSE;
}

static int32_t cli_vcl_parse_keyword(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    vcl_cli_req_t *vcl_req = req->module_req;
    char          *found = cli_parse_find(cmd, stx);

    req->parm_parsed = 1;

    if (found != NULL) {
        if (!strncmp(found, "combined", 8)) {
            vcl_req->vcl_mac_user = VCL_MAC_VLAN_USER_ALL;
            vcl_req->usr_valid = 1;
        } else if (!strncmp(found, "all", 3)) {
            vcl_req->all = 1;
            vcl_req->usr_valid = 1;
        } else if (!strncmp(found, "static", 6)) {
            vcl_req->vcl_mac_user = VCL_MAC_VLAN_USER_STATIC;
            vcl_req->usr_valid = 1;
#ifdef VTSS_SW_OPTION_DOT1X
        } else if (!strncmp(found, "nas", 3)) {
            vcl_req->vcl_mac_user = VCL_MAC_VLAN_USER_NAS;
            vcl_req->usr_valid = 1;
#endif
        }
    }
    return (found == NULL ? 1 : 0);
}

static void cli_cmd_debug_macvlan_conf(cli_req_t *req)
{
    BOOL                                first_entry = TRUE, next = FALSE;
    vcl_mac_vlan_mgmt_entry_t           entry;
    i8                                  buf[MGMT_PORT_BUF_SIZE];

    while (vcl_mac_vlan_mgmt_local_isid_mac_vlan_get(&entry, next, first_entry)
           == VTSS_RC_OK) {
        if (first_entry == TRUE) {
            CPRINTF("MAC Address        VID   Ports\n");
            CPRINTF("-----------------  ----  -----\n");
            first_entry = FALSE;
        }
        CPRINTF("%02x-%02x-%02x-%02x-%02x-%02x  %-4u  %s\n", entry.smac.addr[0], entry.smac.addr[1],
                entry.smac.addr[2], entry.smac.addr[3], entry.smac.addr[4], entry.smac.addr[5],
                entry.vid, cli_iport_list_txt(entry.ports, buf));
        next = TRUE;
    }
}

static void cli_cmd_vcl_macvlan_conf(cli_req_t *req)
{
    vtss_usid_t                         usid;
    vtss_isid_t                         isid;
    vcl_mac_vlan_mgmt_entry_get_cfg_t   entry;
    i8                                  buf[MGMT_PORT_BUF_SIZE];
    BOOL                                first_entry, next;

    if (cli_cmd_switch_none(req) || cli_cmd_conf_slave(req)) {
        return;
    }
    for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
        if ((isid = req->stack.isid[usid]) == VTSS_ISID_END) {
            continue;
        }
        first_entry = TRUE;
        next = FALSE;
        while (vcl_mac_vlan_mgmt_mac_vlan_get(isid, &entry, VCL_MAC_VLAN_USER_STATIC, next, first_entry)
               == VTSS_RC_OK) {
            if (first_entry == TRUE) {
                cli_cmd_usid_print(usid, req, 1);
                CPRINTF("MAC Address        VID   Ports\n");
                CPRINTF("-----------------  ----  -----\n");
                first_entry = FALSE;
            }
            CPRINTF("%02x-%02x-%02x-%02x-%02x-%02x  %-4u  %s\n", entry.smac.addr[0], entry.smac.addr[1],
                    entry.smac.addr[2], entry.smac.addr[3], entry.smac.addr[4], entry.smac.addr[5],
                    entry.vid, cli_iport_list_txt(entry.ports[isid - 1], buf));
            next = TRUE;
        }
    }
}
static void cli_cmd_vcl_macvlan_add(cli_req_t *req)
{
    vcl_mac_vlan_mgmt_entry_t   mac_vlan_entry;
    vtss_isid_t                 isid;
    vtss_usid_t                 usid;
    vtss_port_no_t              iport;
    vtss_rc                     rc;

    if (cli_cmd_switch_none(req) || cli_cmd_conf_slave(req)) {
        return;
    }
    /* If the "stack sel all" is selected, pass isid as VTSS_ISID_GLOBAL */
    if (req->usid_sel == VTSS_USID_ALL) {
        isid = VTSS_ISID_GLOBAL;
    } else {
        for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
            if ((isid = req->stack.isid[usid]) == VTSS_ISID_END) {
                continue;
            }
            break;
        }
    }
    memset(&mac_vlan_entry, 0, sizeof(mac_vlan_entry));
    /* Populate the mac_vlan_entry to pass it to VCL module */
    memcpy(mac_vlan_entry.smac.addr, req->mac_addr, sizeof(req->mac_addr));
    mac_vlan_entry.vid = req->vid;
    for (iport = VTSS_PORT_NO_START; iport < VTSS_PORT_NO_END; iport++) {
        mac_vlan_entry.ports[iport] = req->uport_list[iport2uport(iport)];
    }
    if ((rc = (vcl_mac_vlan_mgmt_mac_vlan_add(isid, &mac_vlan_entry, VCL_MAC_VLAN_USER_STATIC))) != VTSS_RC_OK) {
        if (rc == VCL_ERROR_ENTRY_WITH_DIFF_VLAN) {
            CPRINTF("cli_cmd_vcl_macvlan_add: MAC-based VLAN entry is already configured with different VLAN ID\n");
        } else {
            CPRINTF("cli_cmd_vcl_macvlan_add: Error while adding MAC-based VLAN entry\n");
        }
    }
}
static void cli_cmd_vcl_macvlan_del(cli_req_t *req)
{
    vtss_isid_t                 isid;
    vtss_usid_t                 usid;
    vtss_mac_t                  mac_addr;

    if (cli_cmd_switch_none(req) || cli_cmd_conf_slave(req)) {
        return;
    }
    /* If the "stack sel all" is selected, pass isid as VTSS_ISID_GLOBAL */
    if (req->usid_sel == VTSS_USID_ALL) {
        isid = VTSS_ISID_GLOBAL;
    } else {
        for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
            if ((isid = req->stack.isid[usid]) == VTSS_ISID_END) {
                continue;
            }
            break;
        }
    }
    memcpy(mac_addr.addr, req->mac_addr, sizeof(req->mac_addr));
    if (vcl_mac_vlan_mgmt_mac_vlan_del(isid, &mac_addr, VCL_MAC_VLAN_USER_STATIC) != VTSS_RC_OK) {
        CPRINTF("Deletion failed\n");
    }
}

static void cli_cmd_vcl_status(cli_req_t *req)
{
    vcl_cli_req_t                       *vcl_req = req->module_req;
    vtss_isid_t                         usid;
    vtss_isid_t                         isid;
    BOOL                                first_flag, first, next, entry_printed;
    vcl_mac_vlan_user_t                 usr, first_user, last_user;
    vcl_mac_vlan_mgmt_entry_get_cfg_t   entry;
    i8                                  buf[200], temp[MGMT_PORT_BUF_SIZE], *p;
    u32                                 entry_no;

    if (cli_cmd_switch_none(req) || cli_cmd_conf_slave(req)) {
        return;
    }
    //Default User is "combined".
    if (!vcl_req->usr_valid) {
        vcl_req->all = 1;
    }
    if (vcl_req->all) {
        first_user = VCL_MAC_VLAN_USER_STATIC;
        last_user = VCL_MAC_VLAN_USER_ALL;
    } else {
        first_user = vcl_req->vcl_mac_user;
        last_user = vcl_req->vcl_mac_user;
    }
    for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
        if ((isid = req->stack.isid[usid]) == VTSS_ISID_END) {
            continue;
        }
        first_flag = TRUE;
        first = TRUE;
        next = FALSE;
        entry_no = 1;
        /* First check for combined VLAN User entry to findout all the valid entries */
        while (vcl_mac_vlan_mgmt_mac_vlan_get(isid, &entry, VCL_MAC_VLAN_USER_ALL, next, first)
               == VTSS_RC_OK) {
            entry_printed = FALSE;
            /* As the entry already has MAC address, get the conf for that MAC for all the users */
            for (usr = first_user; usr <= last_user; usr++) {
                if ((vcl_mac_vlan_mgmt_mac_vlan_get(isid, &entry, usr, FALSE, FALSE) == VTSS_RC_OK)) {
                    if (first_flag == TRUE) {
                        cli_cmd_usid_print(usid, req, 1);
                        p = &buf[0];
                        p += sprintf(p, "Entry  ");
                        if (vcl_req->all) {
                            p += sprintf(p, "VCL MAC User  ");
                        }
                        p += sprintf(p, "MAC Address        ");
                        p += sprintf(p, "VID   ");
                        p += sprintf(p, "Ports");
                        cli_table_header(buf);
                        first = 0;
                        first_flag = FALSE;
                    }
                    if (entry_printed == FALSE) {
                        CPRINTF("%-5u  ", entry_no++);
                        entry_printed = TRUE;
                    } else {
                        CPRINTF("       ");
                    }
                    if (vcl_req->all) {
                        CPRINTF("%-12s  ", vcl_mac_vlan_mgmt_vcl_user_to_txt(usr));
                    }
                    CPRINTF("%02x-%02x-%02x-%02x-%02x-%02x  %-4u  %s\n", entry.smac.addr[0], entry.smac.addr[1],
                            entry.smac.addr[2], entry.smac.addr[3], entry.smac.addr[4], entry.smac.addr[5],
                            entry.vid, cli_iport_list_txt(entry.ports[isid - 1], temp));
                }
            }
            CPRINTF("\n");
            next = TRUE;
            first = FALSE;
        }
    }
}

static void cli_cmd_debug_macvlan_add(cli_req_t *req)
{
    vcl_mac_vlan_mgmt_entry_t   mac_vlan_entry;
    vtss_isid_t                 isid;
    vtss_usid_t                 usid;
    vtss_port_no_t              iport;
    vcl_cli_req_t               *vcl_req = req->module_req;

    if (cli_cmd_switch_none(req) || cli_cmd_conf_slave(req)) {
        return;
    }
    /* If the "stack sel all" is selected, pass isid as VTSS_ISID_GLOBAL */
    if (req->usid_sel == VTSS_USID_ALL) {
        isid = VTSS_ISID_GLOBAL;
    } else {
        for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
            if ((isid = req->stack.isid[usid]) == VTSS_ISID_END) {
                continue;
            }
            break;
        }
    }
    /* Populate the mac_vlan_entry to pass it to VCL module */
    memcpy(mac_vlan_entry.smac.addr, req->mac_addr, sizeof(req->mac_addr));
    mac_vlan_entry.vid = req->vid;
    for (iport = VTSS_PORT_NO_START; iport < VTSS_PORT_NO_END; iport++) {
        mac_vlan_entry.ports[iport] = req->uport_list[iport2uport(iport)];
    }
    CPRINTF("mac: %02x-%02x-%02x-%02x-%02x-%02x, vid = %d, isid = %d, user = %d\n", mac_vlan_entry.smac.addr[0], mac_vlan_entry.smac.addr[1],
            mac_vlan_entry.smac.addr[2], mac_vlan_entry.smac.addr[3], mac_vlan_entry.smac.addr[4],
            mac_vlan_entry.smac.addr[5], mac_vlan_entry.vid, isid, vcl_req->vcl_mac_user);
    if ((vcl_mac_vlan_mgmt_mac_vlan_add(isid, &mac_vlan_entry, vcl_req->vcl_mac_user)) != VTSS_RC_OK) {
        CPRINTF("cli_cmd_vcl_macvlan_add: Error while adding MAC-based VLAN entry\n");
    }
}
static void cli_cmd_debug_macvlan_del(cli_req_t *req)
{
    vtss_isid_t                 isid;
    vtss_usid_t                 usid;
    vtss_mac_t                  mac_addr;
    vcl_cli_req_t               *vcl_req = req->module_req;

    if (cli_cmd_switch_none(req) || cli_cmd_conf_slave(req)) {
        return;
    }
    /* If the "stack sel all" is selected, pass isid as VTSS_ISID_GLOBAL */
    if (req->usid_sel == VTSS_USID_ALL) {
        isid = VTSS_ISID_GLOBAL;
    } else {
        for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
            if ((isid = req->stack.isid[usid]) == VTSS_ISID_END) {
                continue;
            }
            break;
        }
    }
    memcpy(mac_addr.addr, req->mac_addr, sizeof(req->mac_addr));
    if (vcl_mac_vlan_mgmt_mac_vlan_del(isid, &mac_addr, vcl_req->vcl_mac_user) != VTSS_RC_OK) {
        CPRINTF("Deletion failed\n");
    }
}

static void cli_vcl_protovlan_protocol_to_group_add(cli_req_t                    *req,
                                                    vcl_proto_vlan_proto_entry_t *proto_grp)
{
    vcl_cli_req_t               *vcl_req = req->module_req;

    if (proto_grp->proto_encap_type == VCL_PROTO_ENCAP_ETH2) {
        proto_grp->proto.eth2_proto.eth_type = vcl_req->ether_type;
    } else if (proto_grp->proto_encap_type == VCL_PROTO_ENCAP_LLC_SNAP) {
        memcpy(proto_grp->proto.llc_snap_proto.oui, vcl_req->oui, OUI_SIZE);
        /* The SNAP header follows the 802.2 header; it has a 5-octet protocol identification field,
           consisting of a 3-octet IEEE Organizationally Unique Identifier (OUI) followed by a 2-octet
           protocol ID. If the OUI is hexadecimal 000000, the protocol ID is the Ethernet type (EtherType)
           field value for the protocol running on top of SNAP; if the OUI is an OUI for a particular
           organization, the protocol ID is a value assigned by that organization to the protocol running
           on top of SNAP. */
        if ((vcl_req->oui[0] == 0) && (vcl_req->oui[1] == 0) && (vcl_req->oui[2] == 0)) {
            if (vcl_req->pid < 0x600) {
                CPRINTF("Invalid PID. IF OUI is zero, PID is in the range of Etype(0x600-0xFFFF)\n");
                return;
            }
        }
        proto_grp->proto.llc_snap_proto.pid = vcl_req->pid;
    } else {
        proto_grp->proto.llc_other_proto.dsap = vcl_req->dsap;
        proto_grp->proto.llc_other_proto.ssap = vcl_req->ssap;
    }
    memcpy(proto_grp->group_id, vcl_req->group_id, MAX_GROUP_NAME_LEN);
    if ((vcl_proto_vlan_mgmt_proto_add(proto_grp, VCL_PROTO_VLAN_USER_STATIC)) != VTSS_RC_OK) {
        CPRINTF("Adding Protocol to Group mapping Failed\n");
    }
}
static void cli_cmd_vcl_protovlan_protocol_to_group_add_eth2(cli_req_t *req)
{
    vcl_proto_vlan_proto_entry_t   proto_grp;
    if (cli_cmd_switch_none(req) || cli_cmd_conf_slave(req)) {
        return;
    }
    proto_grp.proto_encap_type = VCL_PROTO_ENCAP_ETH2;
    cli_vcl_protovlan_protocol_to_group_add(req, &proto_grp);
}
static void cli_cmd_vcl_protovlan_protocol_to_group_add_snap(cli_req_t *req)
{
    vcl_proto_vlan_proto_entry_t   proto_grp;
    if (cli_cmd_switch_none(req) || cli_cmd_conf_slave(req)) {
        return;
    }
    proto_grp.proto_encap_type = VCL_PROTO_ENCAP_LLC_SNAP;
    cli_vcl_protovlan_protocol_to_group_add(req, &proto_grp);
}
static void cli_cmd_vcl_protovlan_protocol_to_group_add_llc(cli_req_t *req)
{
    vcl_proto_vlan_proto_entry_t   proto_grp;
    if (cli_cmd_switch_none(req) || cli_cmd_conf_slave(req)) {
        return;
    }
    proto_grp.proto_encap_type = VCL_PROTO_ENCAP_LLC_OTHER;
    cli_vcl_protovlan_protocol_to_group_add(req, &proto_grp);
}
static void cli_vcl_protovlan_protocol_to_group_delete(cli_req_t *req,
                                                       vcl_proto_vlan_proto_entry_t *proto_grp)
{
    vcl_cli_req_t               *vcl_req = req->module_req;
    if (cli_cmd_switch_none(req) || cli_cmd_conf_slave(req)) {
        return;
    }
    if (proto_grp->proto_encap_type == VCL_PROTO_ENCAP_ETH2) {
        proto_grp->proto.eth2_proto.eth_type = vcl_req->ether_type;
    } else if (proto_grp->proto_encap_type == VCL_PROTO_ENCAP_LLC_SNAP) {
        memcpy(proto_grp->proto.llc_snap_proto.oui, vcl_req->oui, OUI_SIZE);
        proto_grp->proto.llc_snap_proto.pid = vcl_req->pid;
    } else {
        proto_grp->proto.llc_other_proto.dsap = vcl_req->dsap;
        proto_grp->proto.llc_other_proto.ssap = vcl_req->ssap;
    }
    if ((vcl_proto_vlan_mgmt_proto_delete(proto_grp->proto_encap_type, &proto_grp->proto,
                                          VCL_PROTO_VLAN_USER_STATIC)) != VTSS_RC_OK) {
        CPRINTF("Deleting Protocol to Group mapping Failed\n");
    }
}
static void cli_cmd_vcl_protovlan_protocol_to_group_del_eth2(cli_req_t *req)
{
    vcl_proto_vlan_proto_entry_t   proto_grp;
    if (cli_cmd_switch_none(req) || cli_cmd_conf_slave(req)) {
        return;
    }
    proto_grp.proto_encap_type = VCL_PROTO_ENCAP_ETH2;
    cli_vcl_protovlan_protocol_to_group_delete(req, &proto_grp);
}
static void cli_cmd_vcl_protovlan_protocol_to_group_del_snap(cli_req_t *req)
{
    vcl_proto_vlan_proto_entry_t   proto_grp;
    if (cli_cmd_switch_none(req) || cli_cmd_conf_slave(req)) {
        return;
    }
    proto_grp.proto_encap_type = VCL_PROTO_ENCAP_LLC_SNAP;
    cli_vcl_protovlan_protocol_to_group_delete(req, &proto_grp);
}
static void cli_cmd_vcl_protovlan_protocol_to_group_del_llc(cli_req_t *req)
{
    vcl_proto_vlan_proto_entry_t   proto_grp;
    if (cli_cmd_switch_none(req) || cli_cmd_conf_slave(req)) {
        return;
    }
    proto_grp.proto_encap_type = VCL_PROTO_ENCAP_LLC_OTHER;
    cli_vcl_protovlan_protocol_to_group_delete(req, &proto_grp);
}
static void cli_cmd_vcl_protovlan_protocol_to_group_lookup(cli_req_t *req)
{
    vcl_proto_vlan_proto_entry_t   entry;
    BOOL                                first_entry, next;

    if (cli_cmd_switch_none(req) || cli_cmd_conf_slave(req)) {
        return;
    }
    first_entry = TRUE;
    next = FALSE;
    while (vcl_proto_vlan_mgmt_proto_get(&entry, VCL_PROTO_VLAN_USER_STATIC, next, first_entry)
           == VTSS_RC_OK) {
        if (first_entry == TRUE) {
            CPRINTF("Protocol Type  Protocol (Value)          Group ID\n");
            CPRINTF("-------------  ------------------------  --------\n");
            first_entry = FALSE;
        }
        if (entry.proto_encap_type == VCL_PROTO_ENCAP_ETH2) {
            CPRINTF("%-13s  ETYPE:0x%-4x              %s\n", vcl_proto_vlan_mgmt_proto_type_to_txt(entry.proto_encap_type),
                    entry.proto.eth2_proto.eth_type, entry.group_id);
        } else if (entry.proto_encap_type == VCL_PROTO_ENCAP_LLC_SNAP) {
            CPRINTF("%-13s  OUI-%02x:%02x:%02x; PID:0x%-4x  %s\n", vcl_proto_vlan_mgmt_proto_type_to_txt(entry.proto_encap_type),
                    entry.proto.llc_snap_proto.oui[0], entry.proto.llc_snap_proto.oui[1],
                    entry.proto.llc_snap_proto.oui[2], entry.proto.llc_snap_proto.pid, entry.group_id);
        } else {
            CPRINTF("%-13s  DSAP:0x%-2x; SSAP:0x%-2x      %s\n",
                    vcl_proto_vlan_mgmt_proto_type_to_txt(entry.proto_encap_type), entry.proto.llc_other_proto.dsap,
                    entry.proto.llc_other_proto.ssap, entry.group_id);
        }
        next = TRUE;
    }
}
static void cli_cmd_vcl_protovlan_group_to_vlan_add(cli_req_t *req)
{
    vcl_cli_req_t                       *vcl_req = req->module_req;
    vcl_proto_vlan_vlan_entry_t         grp_vlan;
    vtss_usid_t                         usid;
    vtss_isid_t                         isid;
    vtss_port_no_t                      iport;

    if (cli_cmd_switch_none(req) || cli_cmd_conf_slave(req)) {
        return;
    }
    memcpy(grp_vlan.group_id, vcl_req->group_id, MAX_GROUP_NAME_LEN);
    grp_vlan.vid = req->vid;
    /* If the "stack sel all" is selected, pass isid as VTSS_ISID_GLOBAL */
    if (req->usid_sel == VTSS_USID_ALL) {
        isid = VTSS_ISID_GLOBAL;
    } else {
        for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
            if ((isid = req->stack.isid[usid]) == VTSS_ISID_END) {
                continue;
            }
            break;
        }
    }
    for (iport = VTSS_PORT_NO_START; iport < VTSS_PORT_NO_END; iport++) {
        grp_vlan.ports[iport] = req->uport_list[iport2uport(iport)];
    }
    if ((vcl_proto_vlan_mgmt_group_entry_add(isid, &grp_vlan, VCL_PROTO_VLAN_USER_STATIC)) != VTSS_RC_OK) {
        CPRINTF("Adding Group to VLAN mapping Failed\n");
    }
}
static void cli_cmd_vcl_protovlan_group_to_vlan_del(cli_req_t *req)
{
    vcl_cli_req_t                       *vcl_req = req->module_req;
    vcl_proto_vlan_vlan_entry_t    grp_vlan;
    vtss_usid_t                         usid;
    vtss_isid_t                         isid;
    vtss_port_no_t                      iport;

    if (cli_cmd_switch_none(req) || cli_cmd_conf_slave(req)) {
        return;
    }
    memcpy(grp_vlan.group_id, vcl_req->group_id, MAX_GROUP_NAME_LEN);
    /* If the "stack sel all" is selected, pass isid as VTSS_ISID_GLOBAL */
    if (req->usid_sel == VTSS_USID_ALL) {
        isid = VTSS_ISID_GLOBAL;
    } else {
        for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
            if ((isid = req->stack.isid[usid]) == VTSS_ISID_END) {
                continue;
            }
            break;
        }
    }
    for (iport = VTSS_PORT_NO_START; iport < VTSS_PORT_NO_END; iport++) {
        grp_vlan.ports[iport] = req->uport_list[iport2uport(iport)];
    }
    if ((vcl_proto_vlan_mgmt_group_entry_delete(isid, &grp_vlan, VCL_PROTO_VLAN_USER_STATIC)) != VTSS_RC_OK) {
        CPRINTF("Deleting Group to VLAN mapping Failed\n");
    }

}
static void cli_cmd_vcl_protovlan_group_to_vlan_lookup(cli_req_t *req)
{
    vtss_usid_t                         usid;
    vtss_isid_t                         isid;
    vcl_proto_vlan_vlan_entry_t         entry;
    i8                                  buf[MGMT_PORT_BUF_SIZE];
    BOOL                                first_entry, next, first;

    if (cli_cmd_switch_none(req) || cli_cmd_conf_slave(req)) {
        return;
    }
    for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
        if ((isid = req->stack.isid[usid]) == VTSS_ISID_END) {
            continue;
        }
        first_entry = TRUE;
        next = FALSE;
        first = TRUE;
        while (vcl_proto_vlan_mgmt_group_entry_get(isid, &entry, VCL_PROTO_VLAN_USER_STATIC, next, first)
               == VTSS_RC_OK) {
            if (first_entry == TRUE) {
                cli_cmd_usid_print(usid, req, 1);
                CPRINTF("Group ID          VID   Ports\n");
                CPRINTF("----------------  ----  -----\n");
                first_entry = FALSE;
            }
            CPRINTF("%-16s  %-4u  %s\n", entry.group_id, entry.vid, cli_iport_list_txt(entry.ports, buf));
            next = TRUE;
            first = FALSE;
        }
    }
}
static void cli_cmd_vcl_protovlan_lookup(cli_req_t *req)
{

    if (cli_cmd_switch_none(req) || cli_cmd_conf_slave(req)) {
        return;
    }
    cli_cmd_vcl_protovlan_protocol_to_group_lookup(req);
    CPRINTF("\n\n");
    cli_cmd_vcl_protovlan_group_to_vlan_lookup(req);
}
static void cli_cmd_debug_protovlan_lookup(cli_req_t *req)
{
    vcl_proto_vlan_local_sid_conf_t entry;
    u32                             id = 0; //VCL_PROTO_VLAN_MAX_PROTOCOLS
    BOOL                            next = FALSE, first_entry = TRUE;
    BOOL                            ports[VTSS_PORT_ARRAY_SIZE];
    u32                             i;
    vtss_usid_t                     usid;
    vtss_isid_t                     isid;
    vcl_proto_vlan_entry_t          conf;
    i8                              buf[MGMT_PORT_BUF_SIZE];

    if (cli_cmd_switch_none(req)) {
        return;
    }
    CPRINTF("\nLOCAL TABLE\n");
    CPRINTF("-----------\n");
    while (vcl_proto_vlan_mgmt_local_entry_get(&entry, id, next) == VTSS_RC_OK) {
        next = TRUE;
        id = entry.id;
        /* Convert port bitfield to port list */
        for (i = 0; i < VTSS_PORT_ARRAY_SIZE; i++) {
            ports[i] = ((entry.ports[i / 8]) & (1 << (i % 8))) ? TRUE : FALSE;
        }
        if (first_entry) {
            CPRINTF("Protocol Type  Protocol (Value)          VID   vce_id  Ports\n");
            CPRINTF("-------------  ------------------------  ----  ------  -----\n");
            first_entry = FALSE;
        }
        if (entry.proto_encap_type == VCL_PROTO_ENCAP_ETH2) {
            CPRINTF("%-13s  ETYPE:0x%-4x              %-4d  %-6lu  %s\n", vcl_proto_vlan_mgmt_proto_type_to_txt(entry.proto_encap_type),
                    entry.proto.eth2_proto.eth_type, entry.vid, (long unsigned int)entry.id, cli_iport_list_txt(ports, buf));
        } else if (entry.proto_encap_type == VCL_PROTO_ENCAP_LLC_SNAP) {
            CPRINTF("%-13s  OUI-%02x:%02x:%02x; PID:0x%-4x  %-4d  %-6lu  %s\n",
                    vcl_proto_vlan_mgmt_proto_type_to_txt(entry.proto_encap_type),
                    entry.proto.llc_snap_proto.oui[0], entry.proto.llc_snap_proto.oui[1],
                    entry.proto.llc_snap_proto.oui[2], entry.proto.llc_snap_proto.pid, entry.vid, (long unsigned int)entry.id,
                    cli_iport_list_txt(ports, buf));
        } else {
            CPRINTF("%-13s  DSAP:0x%-2x; SSAP:0x%-2x      %-4d  %-6lu  %s\n",
                    vcl_proto_vlan_mgmt_proto_type_to_txt(entry.proto_encap_type), entry.proto.llc_other_proto.dsap,
                    entry.proto.llc_other_proto.ssap, entry.vid, (long unsigned int)entry.id, cli_iport_list_txt(ports, buf));
        }
    }
    CPRINTF("\nHW TABLE\n");
    CPRINTF("--------\n");
    for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
        if ((isid = req->stack.isid[usid]) == VTSS_ISID_END) {
            continue;
        }
        id = 0;
        next = FALSE;
        first_entry = TRUE;
        while (vcl_proto_vlan_mgmt_hw_entry_get(isid, &conf, id, VCL_PROTO_VLAN_USER_STATIC, next) == VTSS_RC_OK) {
            next = TRUE;
            id = conf.vce_id;
            /* Convert port bitfield to port list */
            for (i = 0; i < VTSS_PORT_ARRAY_SIZE; i++) {
                ports[i] = ((conf.ports[i / 8]) & (1 << (i % 8))) ? TRUE : FALSE;
            }
            if (first_entry) {
                cli_cmd_usid_print(usid, req, 1);
                CPRINTF("Protocol Type  Protocol (Value)          VID   vce_id  Ports\n");
                CPRINTF("-------------  ------------------------  ----  ------  -----\n");
                first_entry = FALSE;
            }
            if (conf.proto_encap_type == VCL_PROTO_ENCAP_ETH2) {
                CPRINTF("%-13s  ETYPE:0x%-4x              %-4d  %-6lu  %s\n", vcl_proto_vlan_mgmt_proto_type_to_txt(conf.proto_encap_type),
                        conf.proto.eth2_proto.eth_type, conf.vid, (long unsigned int)conf.vce_id, cli_iport_list_txt(ports, buf));
            } else if (conf.proto_encap_type == VCL_PROTO_ENCAP_LLC_SNAP) {
                CPRINTF("%-13s  OUI-%02x:%02x:%02x; PID:0x%-4x  %-4d  %-6lu  %s\n",
                        vcl_proto_vlan_mgmt_proto_type_to_txt(conf.proto_encap_type),
                        conf.proto.llc_snap_proto.oui[0], conf.proto.llc_snap_proto.oui[1],
                        conf.proto.llc_snap_proto.oui[2], conf.proto.llc_snap_proto.pid, conf.vid, (long unsigned int)conf.vce_id,
                        cli_iport_list_txt(ports, buf));
            } else {
                CPRINTF("%-13s  DSAP:0x%-2x; SSAP:0x%-2x      %-4d  %-6lu  %s\n",
                        vcl_proto_vlan_mgmt_proto_type_to_txt(conf.proto_encap_type), conf.proto.llc_other_proto.dsap,
                        conf.proto.llc_other_proto.ssap, conf.vid, (long unsigned int)conf.vce_id, cli_iport_list_txt(ports, buf));
            }
        }
    }
}
/* IP-Subnet-based VLAN command Handlers */
static void cli_cmd_vcl_ipvlan_lookup(cli_req_t *req)
{
    vcl_ip_vlan_mgmt_entry_t            entry;
    BOOL                                first = TRUE, next = FALSE;
    vtss_usid_t                         usid;
    vtss_isid_t                         isid;
    i8                                  buf[MGMT_PORT_BUF_SIZE];
    i8                                  ip_str[100];
    vcl_cli_req_t                       *vcl_req = req->module_req;

    if (cli_cmd_switch_none(req) || cli_cmd_conf_slave(req)) {
        return;
    }
    for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
        if ((isid = req->stack.isid[usid]) == VTSS_ISID_END) {
            continue;
        }
        if (vcl_req->vce_spec == TRUE) {
            entry.vce_id = vcl_req->vce_id;
            if (vcl_ip_vlan_mgmt_ip_vlan_get(isid, &entry, VCL_IP_VLAN_USER_STATIC, FALSE, FALSE) != VTSS_RC_OK) {
                CPRINTF("Entry with VCE ID %u not found\n", entry.vce_id);
            } else {
                CPRINTF("VCE ID  IP Address       Mask Length  VID   Ports\n");
                CPRINTF("------  ---------------  -----------  ----  -----\n");
                CPRINTF("%-6u  %-15s  %-11u  %-4u  %s\n", entry.vce_id, misc_ipv4_txt(entry.ip_addr, ip_str),
                        entry.mask_len, entry.vid, cli_iport_list_txt(entry.ports, buf));
            } /* if (vcl_ip_vlan_mgmt_ip_vlan_get(isid, &entry, VCL_IP_VLAN_USER_STATIC, FALSE, FALSE) != VTSS_RC_OK) */
        } else {
            first = TRUE;
            next = FALSE;
            while (vcl_ip_vlan_mgmt_ip_vlan_get(isid, &entry, VCL_IP_VLAN_USER_STATIC, first, next)
                   == VTSS_RC_OK) {
                if (first == TRUE) {
                    cli_cmd_usid_print(usid, req, 1);
                    CPRINTF("VCE ID  IP Address       Mask Length  VID   Ports\n");
                    CPRINTF("------  ---------------  -----------  ----  -----\n");
                    first = FALSE;
                }
                CPRINTF("%-6u  %-15s  %-11u  %-4u  %s\n", entry.vce_id, misc_ipv4_txt(entry.ip_addr, ip_str),
                        entry.mask_len, entry.vid, cli_iport_list_txt(entry.ports, buf));
                next = TRUE;
            } /* while (vcl_ip_vlan_mgmt_ip_vlan_get(isid, &entry, VCL_IP_VLAN_USER_STATIC, first, next) */
        } /* if (vcl_req->vce_spec == TRUE) */
    } /* for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) */
}
static void cli_cmd_vcl_ipvlan_add(cli_req_t *req)
{
    vcl_ip_vlan_mgmt_entry_t    ip_vlan_entry;
    vtss_isid_t                 isid;
    vtss_usid_t                 usid;
    vtss_port_no_t              iport;
    vcl_cli_req_t               *vcl_req = req->module_req;
    vtss_rc                     rc;

    if (cli_cmd_switch_none(req) || cli_cmd_conf_slave(req)) {
        return;
    }
    /* If the "stack sel all" is selected, pass isid as VTSS_ISID_GLOBAL */
    if (req->usid_sel == VTSS_USID_ALL) {
        isid = VTSS_ISID_GLOBAL;
    } else {
        for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
            if ((isid = req->stack.isid[usid]) == VTSS_ISID_END) {
                continue;
            }
            break;
        }
    }
    /* Populate the ip_vlan_entry to pass it to VCL module */
    ip_vlan_entry.vce_id = vcl_req->vce_id;
    ip_vlan_entry.ip_addr = vcl_req->src_ip;
    ip_vlan_entry.mask_len = vcl_req->mask_bits;
    ip_vlan_entry.vid = req->vid;
    for (iport = VTSS_PORT_NO_START; iport < VTSS_PORT_NO_END; iport++) {
        ip_vlan_entry.ports[iport] = req->uport_list[iport2uport(iport)];
    }
    if ((rc = vcl_ip_vlan_mgmt_ip_vlan_add(isid, &ip_vlan_entry, VCL_IP_VLAN_USER_STATIC)) != VTSS_RC_OK) {
        switch (rc) {
        case VCL_ERROR_ENTRY_WITH_DIFF_NAME:
            CPRINTF("Add failed. IP Subnet-based VLAN entry already configured with different VCE ID\n");
            break;
        case VCL_ERROR_ENTRY_WITH_DIFF_NAME_VLAN:
            CPRINTF("Add failed. IP Subnet-based VLAN entry already configured with different VCE ID and VLAN\n");
            break;
        case VCL_ERROR_ENTRY_WITH_DIFF_SUBNET:
            CPRINTF("Add failed. IP Subnet-based VLAN entry already configured with different subnet\n");
            break;
        case VCL_ERROR_ENTRY_WITH_DIFF_VLAN:
            CPRINTF("Add failed. IP Subnet-based VLAN entry already configured with different VLAN\n");
            break;
        default:
            CPRINTF("Error while adding IP Subnet-based VLAN entry\n");
            break;
        } /* switch(rc) */
    } /* if ((rc = vcl_ip_vlan_mgmt_ip_vlan_add(isid, &ip_vlan_entry, VCL_IP_VLAN_USER_STATIC)) != VTSS_RC_OK)*/
}
static void cli_cmd_vcl_ipvlan_del(cli_req_t *req)
{
    vcl_ip_vlan_mgmt_entry_t    ip_vlan_entry;
    vtss_isid_t                 isid;
    vtss_usid_t                 usid;
    vtss_rc                     rc;
    vcl_cli_req_t               *vcl_req = req->module_req;

    if (cli_cmd_switch_none(req) || cli_cmd_conf_slave(req)) {
        return;
    }
    /* If the "stack sel all" is selected, pass isid as VTSS_ISID_GLOBAL */
    if (req->usid_sel == VTSS_USID_ALL) {
        isid = VTSS_ISID_GLOBAL;
    } else {
        for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
            if ((isid = req->stack.isid[usid]) == VTSS_ISID_END) {
                continue;
            }
            break;
        }
    }
    ip_vlan_entry.vce_id = vcl_req->vce_id;
    if ((rc = vcl_ip_vlan_mgmt_ip_vlan_del(isid, &ip_vlan_entry, VCL_IP_VLAN_USER_STATIC)) != VTSS_RC_OK) {
        if (rc == VCL_ERROR_ENTRY_NOT_FOUND) {
            CPRINTF("IP Subnet-based VLAN deletion failed - matching entry not found\n");
        } else {
            CPRINTF("IP Subnet-based VLAN deletion failed\n");
        }
    }
}

static void cli_cmd_debug_vcl_port_conf(cli_req_t *req)
{
    vtss_vcl_port_conf_t conf;
    port_iter_t          pit;
    BOOL                 first = 1;

    (void)port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_UPORT, PORT_ITER_FLAGS_NORMAL);
    while (port_iter_getnext(&pit)) {
        if (req->uport_list[pit.uport] == 0 ||
            vtss_vcl_port_conf_get(NULL, pit.iport, &conf) != VTSS_OK) {
            continue;
        }
        if (req->set) {
            conf.dmac_dip = req->enable;
            (void)vtss_vcl_port_conf_set(NULL, pit.iport, &conf);
        } else {
            if (first) {
                first = 0;
                cli_table_header("Port  Address");
            }
            CPRINTF("%-6u%s\n", pit.uport, conf.dmac_dip ? "DMAC/DIP" : "SMAC/SIP");
        }
    }
}

int str2hex(const char *str_p, u16 *hex_value_p)
{
    unsigned int i = 0;
    u16 k = 0, temp = 0;

    if (strlen(str_p) > 4) {
        return 1;
    }
    for (i = 0; i < strlen(str_p); i++) {
        if (str_p[i] >= '0' && str_p[i] <= '9') {
            k = str_p[i] - '0';
        }   else if (str_p[i] >= 'A' && str_p[i] <= 'F') {
            k = str_p[i] - 'A' + 10;
        } else if (str_p[i] >= 'a' && str_p[i] <= 'f') {
            k = str_p[i] - 'a' + 10;
        }
        temp = 16 * temp + k;
    }
    *hex_value_p = temp;
    return 0;
}
static int32_t cli_vcl_parm_parse_ether_type(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    vcl_cli_req_t *vcl_req = NULL;
    int error = 0;
    unsigned int i = 0;
    BOOL is_hex = FALSE;
    req->parm_parsed = 1;
    vcl_req = req->module_req;
    char *found = cli_parse_find(cmd, stx);

    error = cli_parse_text(cmd_org, req->parm, 10);
    if (error != 0) {
        return VTSS_INVALID_PARAMETER;
    }
    if (strlen(req->parm) > 10) {
        return VTSS_INVALID_PARAMETER;
    }
    vcl_req->is_etype = TRUE;
    if (!strncmp(req->parm, "0x", 2)) {
        is_hex = TRUE;
        i = i + 2;
    }
    for (; i < strlen(req->parm); i++) {
        if ((isalpha(req->parm[i])) && (!isxdigit(req->parm[i]))) {
            vcl_req->is_etype = FALSE;
            break;
        }
    }
    if (vcl_req->is_etype == TRUE) {
        if (is_hex == TRUE) {
            error = str2hex(&req->parm[2], &(vcl_req->ether_type));
        } else {
            vcl_req->ether_type = atoi(&req->parm[0]);
        }
        if (vcl_req->ether_type < 0x600) {
            error = 1;
        }
    } else {
        error = 1;
        if (found != NULL) {
            if (((!strncmp(found, "arp", 3))) || (!strncmp(found, "ARP", 3))) {
                vcl_req->ether_type = ETHERTYPE_ARP;
                error = 0;
            } else if ((!strncmp(found, "ipx", 3)) || (!strncmp(found, "IPX", 3))) {
                vcl_req->ether_type = ETHERTYPE_IPX;
                error = 0;
            } else if ((!strncmp(found, "ip", 2)) || (!strncmp(found, "IP", 2))) {
                vcl_req->ether_type = ETHERTYPE_IP;
                error = 0;
            } else if ((!strncmp(found, "at", 2)) || (!strncmp(found, "AT", 2))) {
                vcl_req->ether_type = ETHERTYPE_AT;
                error = 0;
            }
        }
    }
    return (error);
}
static int32_t cli_vcl_parm_parse_oui(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    vcl_cli_req_t *vcl_req = NULL;
    int error = 0;
    unsigned int i = 0, j;
    char str[3];
    u16 k;
    req->parm_parsed = 1;
    vcl_req = req->module_req;
    char *found = cli_parse_find(cmd, stx);

    error = cli_parse_text(cmd_org, req->parm, 12);
    if (error != 0) {
        return VTSS_INVALID_PARAMETER;
    }
    if (strlen(req->parm) > 12) {
        return VTSS_INVALID_PARAMETER;
    }
    vcl_req->is_oui = TRUE;
    for (i = 0, j = 0; i < strlen(req->parm); i++) {
        j++;
        /* Every 3rd character should be - */
        if (j == 3) {
            if (req->parm[i] != '-') {
                CPRINTF("OUI should be in aa-bb-cc format\n");
                return 1;
            }
            j = 0;
            continue;
        }
        if (isalpha(req->parm[i]) && (!isxdigit(req->parm[i]))) {
            vcl_req->is_oui = FALSE;
            break;
        }
    }
    if (vcl_req->is_oui == TRUE) {
        strncpy(str, &req->parm[0], 2);
        str[2] = '\0';
        error = str2hex(str, &k);
        vcl_req->oui[0] = (unsigned char) k;
        strncpy(str, &req->parm[3], 2);
        str[2] = '\0';
        error = str2hex(str, &k);
        vcl_req->oui[1] = (unsigned char) k;
        strncpy(str, &req->parm[6], 2);
        str[2] = '\0';
        error = str2hex(str, &k);
        vcl_req->oui[2] = (unsigned char) k;
    } else {
        error = VTSS_INVALID_PARAMETER;
        if (found != NULL) {
            if (!strncmp(found, "rfc_1042", 8)) {
                vcl_req->oui[0] = 0x0;
                vcl_req->oui[1] = 0x0;
                vcl_req->oui[2] = 0x0;
                error = VTSS_OK;
            } else if (!strncmp(found, "snap_8021h", 10)) {
                vcl_req->oui[0] = 0x0;
                vcl_req->oui[1] = 0x0;
                vcl_req->oui[2] = 0xF8;
                error = VTSS_OK;
            }
        }
    }
    return error;
}
static int32_t cli_vcl_parm_parse_pid(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    vcl_cli_req_t *vcl_req = NULL;
    int error = 0;
    long value;
    req->parm_parsed = 1;
    vcl_req = req->module_req;
    error =  cli_parse_long(cmd, &value, 0x0, 0xFFFF);
    vcl_req->pid = value;
    return (error);
}
static int32_t cli_vcl_parm_parse_dsap(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    vcl_cli_req_t *vcl_req = NULL;
    int error = 0;
    long value;
    req->parm_parsed = 1;
    vcl_req = req->module_req;
    error =  cli_parse_long(cmd, &value, 0, 0xFF);
    vcl_req->dsap = value;
    return (error);
}
static int32_t cli_vcl_parm_parse_ssap(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    vcl_cli_req_t *vcl_req = NULL;
    int error = 0;
    long value;
    req->parm_parsed = 1;
    vcl_req = req->module_req;
    error =  cli_parse_long(cmd, &value, 0, 0xFF);
    vcl_req->ssap = value;
    return (error);
}
static int32_t cli_vcl_parm_parse_group_id(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;
    vcl_cli_req_t *vcl_req = NULL;
    req->parm_parsed = 1;
    vcl_req = req->module_req;
    error = cli_parse_text(cmd_org, vcl_req->group_id, MAX_GROUP_NAME_LEN);
    return error;
}
static int32_t cli_vcl_parm_parse_vce_id(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    vcl_cli_req_t *vcl_req = NULL;
    int error = 0;
    long value;
    req->parm_parsed = 1;
    vcl_req = req->module_req;
    error =  cli_parse_long(cmd, &value, 1, VCL_IP_VLAN_MAX_ENTRIES);
    if (!error) {
        vcl_req->vce_id = value;
        vcl_req->vce_spec = TRUE;
    }
    return (error);
}
static int32_t cli_parm_parse_src_ip_mask(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    vcl_cli_req_t *vcl_req = req->module_req;
    ulong         src_mask;
    int           error = 0, i;
    u32           maskbits;

    error = cli_parse_ipv4(cmd, &vcl_req->src_ip, &src_mask, &vcl_req->sip_spec, 0);
    maskbits = 0;
    for (i = 31; i >= 0; i--) {
        if (((src_mask >> i) & 1) == 1) {
            maskbits++;
        }
    }
    vcl_req->mask_bits = maskbits;
    if (vcl_req->sip_spec != CLI_SPEC_VAL) {
        return -1;
    }
    return error;
}

static int32_t cli_vcl_parse_addr(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char *found = cli_parse_find(cmd, stx);

    if (found != NULL && *found == 'd') {
        req->enable = 1;
    }
    return (found == NULL ? 1 : 0);
}

/* Parm table */
static cli_parm_t vcl_cli_parm_table[] = {
    {
        "ethernet2",
        "Ethernet2 Type keyword",
        CLI_PARM_FLAG_NONE,
        cli_vcl_parse_keyword,
        NULL,
    },
    {
        "etype",
        "ether type Type keyword",
        CLI_PARM_FLAG_NONE,
        cli_vcl_parse_keyword,
        NULL,
    },
    {
        "<ether_type>|arp|ip|ipx|at",
        "Ether Type (0x0600 - 0xFFFF)",
        CLI_PARM_FLAG_NONE,
        cli_vcl_parm_parse_ether_type,
        NULL,
    },
    {
        "arp",
        "ARP Type keyword",
        CLI_PARM_FLAG_NONE,
        cli_vcl_parse_keyword,
        NULL,
    },
    {
        "ip",
        "IP Type keyword",
        CLI_PARM_FLAG_NONE,
        cli_vcl_parse_keyword,
        NULL,
    },
    {
        "ipx",
        "IPX Type keyword",
        CLI_PARM_FLAG_NONE,
        cli_vcl_parse_keyword,
        NULL,
    },
    {
        "at",
        "Appletalk Type keyword",
        CLI_PARM_FLAG_NONE,
        cli_vcl_parse_keyword,
        NULL,
    },
    {
        "llc_snap",
        "LLC SNAP Type keyword",
        CLI_PARM_FLAG_NONE,
        cli_vcl_parse_keyword,
        NULL,
    },
    {
        "oui",
        "OUI Type keyword",
        CLI_PARM_FLAG_NONE,
        cli_vcl_parse_keyword,
        NULL,
    },
    {
        "<oui>|rfc_1042|snap_8021h",
        "OUI value (Hexadecimal 00-00-00 to FF-FF-FF).",
        CLI_PARM_FLAG_SET,
        cli_vcl_parm_parse_oui,
        NULL
    },
    {
        "rfc_1042",
        "RFC1042 OUI Type keyword",
        CLI_PARM_FLAG_NONE,
        cli_vcl_parse_keyword,
        NULL,
    },
    {
        "snap_8021h",
        "802.1H SNAP OUI Type keyword",
        CLI_PARM_FLAG_NONE,
        cli_vcl_parse_keyword,
        NULL,
    },
    {
        "pid",
        "PID Type keyword",
        CLI_PARM_FLAG_NONE,
        cli_vcl_parse_keyword,
        NULL,
    },
    {
        "<pid>",
        "PID value (0x0-0xFFFF). If OUI is 00-00-00, valid range of PID is from 0x0600-0xFFFF.",
        CLI_PARM_FLAG_SET,
        cli_vcl_parm_parse_pid,
        NULL
    },
    {
        "llc_other",
        "LLC other Type keyword",
        CLI_PARM_FLAG_NONE,
        cli_vcl_parse_keyword,
        NULL,
    },
    {
        "dsap",
        "DSAP Type keyword",
        CLI_PARM_FLAG_NONE,
        cli_vcl_parse_keyword,
        NULL,
    },
    {
        "<dsap>",
        "DSAP value (0x00-0xFF)",
        CLI_PARM_FLAG_SET,
        cli_vcl_parm_parse_dsap,
        NULL
    },
    {
        "ssap",
        "SSAP Type keyword",
        CLI_PARM_FLAG_NONE,
        cli_vcl_parse_keyword,
        NULL,
    },
    {
        "<ssap>",
        "SSAP value (0x00-0xFF)",
        CLI_PARM_FLAG_SET,
        cli_vcl_parm_parse_ssap,
        NULL
    },
    {
        "<group_id>",
        "Protocol group ID",
        CLI_PARM_FLAG_NONE,
        cli_vcl_parm_parse_group_id,
        NULL,
    },
    {
        "combined|static"
#ifdef VTSS_SW_OPTION_DOT1X
        "|nas"
#endif
        "|all",
        "VCL User",
        CLI_PARM_FLAG_NONE,
        cli_vcl_parse_keyword,
        NULL,
    },
    {
        "static"
#ifdef VTSS_SW_OPTION_DOT1X
        "|nas"
#endif
        ,
        "static     : Shows the VLAN entries configured by the administrator\n"
#ifdef VTSS_SW_OPTION_DOT1X
        "nas        : Shows the VLANs configured by NAS\n"
#endif
        ,
        CLI_PARM_FLAG_NONE,
        cli_vcl_parse_keyword,
        NULL,
    },
    {
        "<vce_id>",
        "Unique VCE ID (1-128) for each VCL entry",
        CLI_PARM_FLAG_NONE,
        cli_vcl_parm_parse_vce_id,
        NULL
    },
    {
        "<ip_addr_mask>",
        "Source IP address and mask (Format: a.b.c.d/n).",
        CLI_PARM_FLAG_NONE,
        cli_parm_parse_src_ip_mask,
        NULL
    },
    {
        "dmac_dip|smac_sip",
        "Destination or source address matching",
        CLI_PARM_FLAG_SET,
        cli_vcl_parse_addr,
        NULL
    },
    {NULL, NULL, 0, 0, NULL}
};

enum {
    PRIO_VCL_MAC_VLAN_CONF,
    PRIO_VCL_MAC_VLAN_ADD,
    PRIO_VCL_MAC_VLAN_DEL,
    PRIO_VCL_MAC_VLAN_STATUS,
    PRIO_DEBUG_MAC_VLAN_CONF = CLI_CMD_SORT_KEY_DEFAULT,
    PRIO_DEBUG_MAC_VLAN_ADD  = CLI_CMD_SORT_KEY_DEFAULT,
    PRIO_DEBUG_MAC_VLAN_DEL  = CLI_CMD_SORT_KEY_DEFAULT,
    PRIO_VCL_PROTO_VLAN_CONF,
    PRIO_VCL_PROTO_VLAN_PROTOCOL_TO_GROUP_ADD_ETH2,
    PRIO_VCL_PROTO_VLAN_PROTOCOL_TO_GROUP_ADD_SNAP,
    PRIO_VCL_PROTO_VLAN_PROTOCOL_TO_GROUP_ADD_LLC,
    PRIO_VCL_PROTO_VLAN_PROTOCOL_TO_GROUP_DEL_ETH2,
    PRIO_VCL_PROTO_VLAN_PROTOCOL_TO_GROUP_DEL_SNAP,
    PRIO_VCL_PROTO_VLAN_PROTOCOL_TO_GROUP_DEL_LLC,
    PRIO_VCL_PROTO_VLAN_PROTOCOL_TO_GROUP_LOOKUP,
    PRIO_VCL_PROTO_VLAN_GROUP_TO_VLAN_ADD,
    PRIO_VCL_PROTO_VLAN_GROUP_TO_VLAN_DEL,
    PRIO_VCL_PROTO_VLAN_GROUP_TO_VLAN_LOOKUP,
    PRIO_VCL_PROTO_VLAN_LOOKUP,
    PRIO_VCL_IP_VLAN_CONF,
    PRIO_VCL_IP_VLAN_ADD,
    PRIO_VCL_IP_VLAN_DEL,
    PRIO_DEBUG_PROTO_VLAN_CONF = CLI_CMD_SORT_KEY_DEFAULT,
    PRIO_DEBUG_VCL_ADDR = CLI_CMD_SORT_KEY_DEFAULT,
};

/* Command table */
cli_cmd_tab_entry(
    "VCL Macvlan Configuration",
    NULL,
    "Show VCL MAC-based VLAN configuration",
    PRIO_VCL_MAC_VLAN_CONF,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_VCL,
    cli_cmd_vcl_macvlan_conf,
    NULL/* vcl_macvlan_cli_req_conf_default_set*/,
    vcl_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry(
    NULL,
    "VCL Macvlan Add <mac_addr> <vid> [<port_list>]",
    "Add or modify VCL MAC-based VLAN entry. The maximum Macvlan entries are limited to 256",
    PRIO_VCL_MAC_VLAN_ADD,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_VCL,
    cli_cmd_vcl_macvlan_add,
    NULL/* vcl_macvlan_cli_req_conf_default_set*/,
    vcl_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry(
    NULL,
    "VCL Macvlan Del <mac_addr>",
    "Delete VCL MAC-based VLAN entry",
    PRIO_VCL_MAC_VLAN_DEL,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_VCL,
    cli_cmd_vcl_macvlan_del,
    NULL/* vcl_macvlan_cli_req_conf_default_set*/,
    vcl_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
CLI_CMD_TAB_ENTRY_DECL(cli_cmd_vcl_status) = {
    "VCL Status [combined|static"
#ifdef VTSS_SW_OPTION_DOT1X
    "|nas"
#endif
    "|all]",
    NULL,
    "Show VCL MAC-based VLAN users configuration",
    PRIO_VCL_MAC_VLAN_STATUS,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_VCL,
    cli_cmd_vcl_status,
    NULL/* vcl_macvlan_cli_req_conf_default_set*/,
    vcl_cli_parm_table,
    CLI_CMD_FLAG_NONE
};
cli_cmd_tab_entry(
    "Debug VCL Macvlan Configuration",
    NULL,
    "Show VCL MAC-based VLAN configuration on a switch",
    PRIO_DEBUG_MAC_VLAN_CONF,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_macvlan_conf,
    NULL/* vcl_macvlan_cli_req_conf_default_set*/,
    vcl_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
CLI_CMD_TAB_ENTRY_DECL(cli_cmd_debug_macvlan_add) = {
    NULL,
    "Debug VCL Macvlan Add <mac_addr> <vid> [<port_list>] [static"
#ifdef VTSS_SW_OPTION_DOT1X
    "|nas"
#endif
    "]",
    "Debug command to add or modify VCL MAC-based VLAN entry for volatile or static user",
    PRIO_DEBUG_MAC_VLAN_ADD,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_macvlan_add,
    NULL/* vcl_macvlan_cli_req_conf_default_set*/,
    vcl_cli_parm_table,
    CLI_CMD_FLAG_NONE
};
CLI_CMD_TAB_ENTRY_DECL(cli_cmd_debug_macvlan_del) = {
    NULL,
    "Debug VCL Macvlan Del <mac_addr> [static"
#ifdef VTSS_SW_OPTION_DOT1X
    "|nas"
#endif
    "]",
    "Debug command to Delete VCL MAC-based VLAN entry for volatile or static user",
    PRIO_DEBUG_MAC_VLAN_DEL,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_macvlan_del,
    NULL/* vcl_macvlan_cli_req_conf_default_set*/,
    vcl_cli_parm_table,
    CLI_CMD_FLAG_NONE
};
cli_cmd_tab_entry(
    NULL,
    "VCL ProtoVlan Protocol Add Eth2 <ether_type>|arp|ip|ipx|at <group_id>",
    "Add VCL protocol-based VLAN Ethernet-II protocol to group mapping. The maximum protocol to group mappings are limited to 128",
    PRIO_VCL_PROTO_VLAN_PROTOCOL_TO_GROUP_ADD_ETH2,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_VCL,
    cli_cmd_vcl_protovlan_protocol_to_group_add_eth2,
    NULL/* vcl_macvlan_cli_req_conf_default_set*/,
    vcl_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry(
    NULL,
    "VCL ProtoVlan Protocol Add Snap <oui>|rfc_1042|snap_8021h <pid> <group_id>",
    "Add VCL protocol-based VLAN SNAP protocol to group mapping. The maximum protocol to group mappings are limited to 128",
    PRIO_VCL_PROTO_VLAN_PROTOCOL_TO_GROUP_ADD_SNAP,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_VCL,
    cli_cmd_vcl_protovlan_protocol_to_group_add_snap,
    NULL/* vcl_macvlan_cli_req_conf_default_set*/,
    vcl_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry(
    NULL,
    "VCL ProtoVlan Protocol Add Llc <dsap> <ssap> <group_id>",
    "Add VCL protocol-based VLAN LLC protocol to group mapping. The maximum protocol to group mappings are limited to 128",
    PRIO_VCL_PROTO_VLAN_PROTOCOL_TO_GROUP_ADD_LLC,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_VCL,
    cli_cmd_vcl_protovlan_protocol_to_group_add_llc,
    NULL/* vcl_macvlan_cli_req_conf_default_set*/,
    vcl_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry(
    NULL,
    "VCL ProtoVlan Protocol Delete Eth2 <ether_type>|arp|ip|ipx|at",
    "Delete VCL protocol-based VLAN Ethernet-II protocol to group mapping",
    PRIO_VCL_PROTO_VLAN_PROTOCOL_TO_GROUP_DEL_ETH2,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_VCL,
    cli_cmd_vcl_protovlan_protocol_to_group_del_eth2,
    NULL/* vcl_macvlan_cli_req_conf_default_set*/,
    vcl_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry(
    NULL,
    "VCL ProtoVlan Protocol Delete Snap <oui>|rfc_1042|snap_8021h <pid>",
    "Delete VCL protocol-based VLAN SNAP protocol to group mapping",
    PRIO_VCL_PROTO_VLAN_PROTOCOL_TO_GROUP_DEL_SNAP,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_VCL,
    cli_cmd_vcl_protovlan_protocol_to_group_del_snap,
    NULL/* vcl_macvlan_cli_req_conf_default_set*/,
    vcl_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry(
    NULL,
    "VCL ProtoVlan Protocol Delete Llc <dsap> <ssap>",
    "Delete VCL protocol-based VLAN LLC protocol to group mapping",
    PRIO_VCL_PROTO_VLAN_PROTOCOL_TO_GROUP_DEL_LLC,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_VCL,
    cli_cmd_vcl_protovlan_protocol_to_group_del_llc,
    NULL/* vcl_macvlan_cli_req_conf_default_set*/,
    vcl_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry(
    NULL,
    "VCL ProtoVlan Vlan Add [<port_list>] <group_id> <vid>",
    "Add VCL protocol-based VLAN group to VLAN mapping. The maximum group to VLAN mappings are limited to 64",
    PRIO_VCL_PROTO_VLAN_GROUP_TO_VLAN_ADD,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_VCL,
    cli_cmd_vcl_protovlan_group_to_vlan_add,
    NULL/* vcl_macvlan_cli_req_conf_default_set*/,
    vcl_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry(
    NULL,
    "VCL ProtoVlan Vlan Delete [<port_list>] <group_id>",
    "Delete VCL protocol-based VLAN group to VLAN mapping",
    PRIO_VCL_PROTO_VLAN_GROUP_TO_VLAN_DEL,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_VCL,
    cli_cmd_vcl_protovlan_group_to_vlan_del,
    NULL/* vcl_macvlan_cli_req_conf_default_set*/,
    vcl_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry(
    "VCL ProtoVlan Conf",
    NULL,
    "Show VCL protocol-based VLAN entries",
    PRIO_VCL_PROTO_VLAN_LOOKUP,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_VCL,
    cli_cmd_vcl_protovlan_lookup,
    NULL/* vcl_macvlan_cli_req_conf_default_set*/,
    vcl_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry(
    "Debug VCL ProtoVlan Conf",
    NULL,
    "Show VCL protocol-based VLAN entries",
    PRIO_DEBUG_PROTO_VLAN_CONF,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_VCL,
    cli_cmd_debug_protovlan_lookup,
    NULL/* vcl_macvlan_cli_req_conf_default_set*/,
    vcl_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
/* IP Subnet-based VLAN Commands */
cli_cmd_tab_entry(
    "VCL IPVlan Configuration [<vce_id>]",
    NULL,
    "Show VCL IP Subnet-based VLAN configuration",
    PRIO_VCL_IP_VLAN_CONF,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_VCL,
    cli_cmd_vcl_ipvlan_lookup,
    vcl_ipvlan_cli_req_conf_default_set,
    vcl_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry(
    NULL,
    "VCL IPVlan Add [<vce_id>] <ip_addr_mask> <vid> [<port_list>]",
    "Add or modify VCL IP Subnet-based VLAN entry. The maximum IPVlan entries are limited to 128",
    PRIO_VCL_IP_VLAN_ADD,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_VCL,
    cli_cmd_vcl_ipvlan_add,
    vcl_ipvlan_cli_req_conf_default_set,
    vcl_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry(
    NULL,
    "VCL IPVlan Delete <vce_id>",
    "Delete VCL IP Subnet-based VLAN entry",
    PRIO_VCL_IP_VLAN_DEL,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_VCL,
    cli_cmd_vcl_ipvlan_del,
    NULL,
    vcl_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry(
    "Debug VCL Address [<port_list>] [dmac_dip|smac_sip]",
    NULL,
    "Set or show port address matching mode",
    PRIO_DEBUG_VCL_ADDR,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_VCL,
    cli_cmd_debug_vcl_port_conf,
    NULL,
    vcl_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
