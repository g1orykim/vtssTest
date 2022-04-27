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
#include "vcl_api.h"

#include "conf_xml_api.h"
#include "conf_xml_trace_def.h"
#include "misc_api.h"
#include "mgmt_api.h"
#include "vlan_api.h"

/* Tag IDs */
enum {
    /* Module tags */
    CX_TAG_VCL,

    /* Group tags */
    CX_TAG_MAC_VLAN_TABLE,
    CX_TAG_PROTO_VLAN_PROTO_TABLE,
    CX_TAG_PROTO_VLAN_VLAN_TABLE,
    CX_TAG_IP_VLAN_TABLE,

    /* Parameter tags */
    CX_TAG_ENTRY,

    /* Last entry */
    CX_TAG_NONE
};

/* Tag table */
static cx_tag_entry_t vcl_cx_tag_table[CX_TAG_NONE + 1] = {
    [CX_TAG_VCL] = {
        .name  = "vcl",
        .descr = "VLAN Control List",
        .type = CX_TAG_TYPE_MODULE
    },
    [CX_TAG_MAC_VLAN_TABLE] = {
        .name  = "mac_vlan_table",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },
    [CX_TAG_PROTO_VLAN_PROTO_TABLE] = {
        .name  = "protocol_vlan_protocol_table",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },
    [CX_TAG_PROTO_VLAN_VLAN_TABLE] = {
        .name  = "protocol_vlan_vlan_table",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },
    [CX_TAG_IP_VLAN_TABLE] = {
        .name  = "ip_vlan_table",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },
    [CX_TAG_ENTRY] = {
        .name  = "entry",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    /* Last entry */
    [CX_TAG_NONE] = {
        .name  = "",
        .descr = "",
        .type = CX_TAG_TYPE_NONE
    }
};

static vtss_rc cx_parse_group(cx_set_state_t *s, char *name, char *grp_name)
{
    uint    idx;
    BOOL    error = FALSE;
    CX_RC(cx_parse_txt(s, name, grp_name, MAX_GROUP_NAME_LEN - 1));

    for (idx = 0; idx < strlen(grp_name); idx++) {
        if ((grp_name[idx]) < 48 || (grp_name[idx] > 122)) {
            error = TRUE;
        } else {
            if ((grp_name[idx] > 57) && (grp_name[idx] < 65)) {
                error = TRUE;
            } else if ((grp_name[idx] > 90) && (grp_name[idx] < 97)) {
                error = TRUE;
            }
        }
    }
    if (error == TRUE) {
        CX_RC(cx_parm_invalid(s));
    }
    return s->rc;
}

static vtss_rc cx_parse_encap_type(cx_set_state_t *s, u8 *type)
{
    i8  buf[20];
    CX_RC(cx_parse_txt(s, "encap_type", buf, 19));
    if (!strcmp(buf, "ethernet")) {
        *type = VCL_PROTO_ENCAP_ETH2;
    } else if (!strcmp(buf, "llc")) {
        *type = VCL_PROTO_ENCAP_LLC_OTHER;
    } else if (!strcmp(buf, "snap")) {
        *type = VCL_PROTO_ENCAP_LLC_SNAP;
    } else {
        CX_RC(cx_parm_invalid(s));
    }
    return s->rc;
}

static vtss_rc cx_parse_oui(cx_set_state_t *s, u8 *oui)
{
    char buf[32];
    int  i, j, c;

    CX_RC(cx_parse_txt(s, "oui", buf, sizeof(buf)));
    if (strlen(buf) != 8) {
        CX_RC(cx_parm_invalid(s));
    }
    for (i = 0; i < 8; i++) {
        j = (i % 3);
        c = tolower(buf[i]);
        if (j == 2) {
            if (c != '-') {
                return cx_parm_invalid(s);
            }
        } else {
            if (!isxdigit(c)) {
                return cx_parm_invalid(s);
            }
            c = (isdigit(c) ? (c - '0') : (10 + c - 'a'));
            if (j == 0) {
                oui[i / 3] = c * 16;
            } else {
                oui[i / 3] += c;
            }
        }
    }
    return s->rc;
}

static vtss_rc vcl_cx_parse_func(cx_set_state_t *s)
{
    switch (s->cmd) {
    case CX_PARSE_CMD_PARM: {
        BOOL           global;

        global = (s->isid == VTSS_ISID_GLOBAL);
        if (global) {
            s->ignored = 1;
            break;
        }

        switch (s->id) {
        case CX_TAG_MAC_VLAN_TABLE:
            if (s->apply) {
                vcl_mac_vlan_mgmt_entry_get_cfg_t   conf;
                /* Flush MAC-based VLAN table - Always get first entry as first entry changes on delete */
                while ((vcl_mac_vlan_mgmt_mac_vlan_get(s->isid, &conf, VCL_MAC_VLAN_USER_STATIC, FALSE,
                                                       TRUE)) == VTSS_RC_OK) {
                    if (vcl_mac_vlan_mgmt_mac_vlan_del(s->isid, &conf.smac, VCL_MAC_VLAN_USER_STATIC)
                        != VTSS_RC_OK) {
                        T_D("vcl_mac_vlan_mgmt_mac_vlan_del failed\n");
                        break;
                    }
                }
            }
            break;
        case CX_TAG_PROTO_VLAN_PROTO_TABLE:
            if (s->apply) {
                vcl_proto_vlan_proto_entry_t   entry;
                /* Delete all the protocol to group mappings */
                while (vcl_proto_vlan_mgmt_proto_get(&entry, VCL_PROTO_VLAN_USER_STATIC, FALSE, TRUE)
                       == VTSS_RC_OK) {
                    if (vcl_proto_vlan_mgmt_proto_delete(entry.proto_encap_type, &(entry.proto),
                                                         VCL_PROTO_VLAN_USER_STATIC) != VTSS_RC_OK) {
                        T_D("vcl_proto_vlan_mgmt_proto_delete failed\n");
                        break;
                    }
                }
            }
            break;
        case CX_TAG_PROTO_VLAN_VLAN_TABLE:
            if (s->apply) {
                vcl_proto_vlan_vlan_entry_t         entry;
                /* Delete all the group to VLAN mappings */
                while (vcl_proto_vlan_mgmt_group_entry_get(s->isid, &entry, VCL_PROTO_VLAN_USER_STATIC, FALSE,
                                                           TRUE) == VTSS_RC_OK) {
                    if (vcl_proto_vlan_mgmt_group_entry_delete(s->isid, &entry,
                                                               VCL_PROTO_VLAN_USER_STATIC) != VTSS_RC_OK) {
                        T_D("vcl_proto_vlan_mgmt_group_entry_delete failed\n");
                        break;
                    }
                }
            }
            break;
        case CX_TAG_IP_VLAN_TABLE:
            if (s->apply) {
                vcl_ip_vlan_mgmt_entry_t        entry;
                /* Delete all the IP-based VLAN entries */
                while (vcl_ip_vlan_mgmt_ip_vlan_get(s->isid, &entry, VCL_IP_VLAN_USER_STATIC, TRUE, FALSE) == VTSS_RC_OK) {
                    if (vcl_ip_vlan_mgmt_ip_vlan_del(s->isid, &entry, VCL_IP_VLAN_USER_STATIC) != VTSS_RC_OK) {
                        T_D("vcl_ip_vlan_mgmt_ip_vlan_del failed\n");
                        break;
                    }
                }
            }
            break;
        case CX_TAG_ENTRY: {
            vcl_mac_vlan_mgmt_entry_t   conf;
            BOOL                        mac = 0, vid = 0, port = 0;
            BOOL                        port_list[VTSS_PORT_ARRAY_SIZE + 1];
            vtss_port_no_t              port_idx;

            if (s->group == CX_TAG_MAC_VLAN_TABLE) {
                memset(&conf, 0, sizeof(vcl_mac_vlan_mgmt_entry_t));
                memset(port_list, 0, VTSS_PORT_ARRAY_SIZE + 1);
                for (; s->rc == VTSS_OK && cx_parse_attr(s) == VTSS_OK; s->p = s->next) {
                    if (cx_parse_mac(s, "mac_addr", conf.smac.addr) == VTSS_OK) {
                        mac = 1;
                    } else if (cx_parse_vid(s, &conf.vid, 0) == VTSS_OK) {
                        vid = 1;
                    } else if (cx_parse_ports(s, port_list, 0) == VTSS_OK) {
                        port = 1;
                    } else {
                        CX_RC(cx_parm_unknown(s));
                    }
                }
                if (s->apply && mac && vid && port) {
                    for (port_idx = VTSS_PORT_NO_START; port_idx < VTSS_PORT_NO_END; port_idx++) {
                        conf.ports[port_idx] = port_list[iport2uport(port_idx)];
                    }
                    if (vcl_mac_vlan_mgmt_mac_vlan_add(s->isid, &conf, VCL_MAC_VLAN_USER_STATIC)
                        != VTSS_RC_OK) {
                        T_D("Error in adding MAC-based VLAN entry\n");
                    }
                }
            } else if (s->group == CX_TAG_PROTO_VLAN_PROTO_TABLE) {
                vcl_proto_vlan_proto_entry_t    entry;
                BOOL                            group = 0, encap = 0, proto = 0;
                ulong                           temp;
                memset(&entry, 0, sizeof(vcl_proto_vlan_proto_entry_t));
                for (; s->rc == VTSS_OK && cx_parse_attr(s) == VTSS_OK; s->p = s->next) {
                    if (cx_parse_group(s, "group_id", (char *)entry.group_id) == VTSS_OK) {
                        group = 1;
                    } else if (cx_parse_encap_type(s, (u8 *) & (entry.proto_encap_type)) == VTSS_OK) {
                        encap = 1;
                        s->p = s->next;
                        if (s->rc == VTSS_OK && cx_parse_attr(s) == VTSS_OK) {
                            if (entry.proto_encap_type == VCL_PROTO_ENCAP_ETH2) {
                                if (cx_parse_ulong(s, "etype", &temp , 0x600, 0xFFFF)
                                    == VTSS_OK) {
                                    entry.proto.eth2_proto.eth_type = (u16)temp;
                                    proto = 1;
                                }
                            } else if (entry.proto_encap_type == VCL_PROTO_ENCAP_LLC_SNAP) {
                                if (cx_parse_oui(s, entry.proto.llc_snap_proto.oui) == VTSS_OK) {
                                    s->p = s->next;
                                    if (s->rc == VTSS_OK && cx_parse_attr(s) == VTSS_OK) {
                                        if (cx_parse_ulong(s, "pid", &temp, 0x0, 0xFFFF)
                                            == VTSS_OK) {
                                            entry.proto.llc_snap_proto.pid = (u16)temp;
                                            proto = 1;
                                        }
                                    }
                                }
                            } else if (entry.proto_encap_type == VCL_PROTO_ENCAP_LLC_OTHER) {
                                if (cx_parse_ulong(s, "dsap", &temp, 0x0, 0xFF)
                                    == VTSS_OK) {
                                    entry.proto.llc_other_proto.dsap = (temp & 0xFF);
                                    s->p = s->next;
                                    if (s->rc == VTSS_OK && cx_parse_attr(s) == VTSS_OK) {
                                        if (cx_parse_ulong(s, "ssap", &temp, 0x0, 0xFF)
                                            == VTSS_OK) {
                                            entry.proto.llc_other_proto.ssap = (temp & 0xFF);
                                            proto = 1;
                                        }
                                    }
                                }
                            }
                        }
                    } else {
                        CX_RC(cx_parm_unknown(s));
                    }
                }
                if (s->apply && group && encap && proto) {
                    if (vcl_proto_vlan_mgmt_proto_add(&entry, VCL_PROTO_VLAN_USER_STATIC) != VTSS_OK) {
                        T_D("Error in adding protocol for Protocol-based VLAN entry\n");
                    }
                }
            } else if (s->group == CX_TAG_PROTO_VLAN_VLAN_TABLE) {
                vcl_proto_vlan_vlan_entry_t entry;
                BOOL                        group = 0, vlan = 0, ports = 0;
                memset(&entry, 0, sizeof(vcl_proto_vlan_vlan_entry_t));
                memset(port_list, 0, VTSS_PORT_ARRAY_SIZE + 1);
                for (; s->rc == VTSS_OK && cx_parse_attr(s) == VTSS_OK; s->p = s->next) {
                    if (cx_parse_group(s, "group_id", (char *)entry.group_id) == VTSS_OK) {
                        group = 1;
                    } else if (cx_parse_vid(s, &entry.vid, 0) == VTSS_OK) {
                        vlan = 1;
                    } else if (cx_parse_ports(s, port_list, 0) == VTSS_OK) {
                        ports = 1;
                    } else {
                        CX_RC(cx_parm_unknown(s));
                    }
                }
                if (s->apply && group && vlan && ports) {
                    for (port_idx = VTSS_PORT_NO_START; port_idx < VTSS_PORT_NO_END; port_idx++) {
                        entry.ports[port_idx] = port_list[iport2uport(port_idx)];
                    }
                    if (vcl_proto_vlan_mgmt_group_entry_add(s->isid, &entry, VCL_PROTO_VLAN_USER_STATIC) != VTSS_OK) {
                        T_D("Error in adding VLAN for Protocol-based VLAN entry\n");
                    }
                }
            } else if (s->group == CX_TAG_IP_VLAN_TABLE) {
                vcl_ip_vlan_mgmt_entry_t        entry;
                BOOL                            vce = 0, ip = 0, mask = 0, ports = 0, vlan = 0;
                ulong                           temp;
                memset(&entry, 0, sizeof(vcl_ip_vlan_mgmt_entry_t));
                memset(port_list, 0, VTSS_PORT_ARRAY_SIZE + 1);
                for (; s->rc == VTSS_OK && cx_parse_attr(s) == VTSS_OK; s->p = s->next) {
                    if (cx_parse_ulong(s, "vce_id", &temp, 1, VCL_IP_VLAN_MAX_ENTRIES) == VTSS_OK) {
                        entry.vce_id = (u32)temp;
                        vce = 1;
                    } else if (cx_parse_ipv4(s, "ip_addr", &entry.ip_addr, NULL, 0) == VTSS_OK) {
                        ip = 1;
                    } else if (cx_parse_ulong(s, "mask_len", &temp, 1, 32) == VTSS_OK) {
                        entry.mask_len = (u8)temp;
                        mask = 1;
                    } else if (cx_parse_vid(s, &entry.vid, 0) == VTSS_OK) {
                        vlan = 1;
                    } else if (cx_parse_ports(s, port_list, 0) == VTSS_OK) {
                        ports = 1;
                    } else {
                        CX_RC(cx_parm_unknown(s));
                    }
                }
                if (s->apply && vce && ip && mask && vlan && ports) {
                    for (port_idx = VTSS_PORT_NO_START; port_idx < VTSS_PORT_NO_END; port_idx++) {
                        entry.ports[port_idx] = port_list[iport2uport(port_idx)];
                    }
                    if (vcl_ip_vlan_mgmt_ip_vlan_add(s->isid, &entry, VCL_IP_VLAN_USER_STATIC) != VTSS_OK) {
                        T_D("Error in adding IP-based VLAN entry\n");
                    }
                }
            } else {
                s->ignored = 1;
            }
            break;
        }
        default:
            s->ignored = 1;
            break;
        }
        } /* CX_PARSE_CMD_PARM */
    case CX_PARSE_CMD_GLOBAL:
        break;
    case CX_PARSE_CMD_SWITCH:
        break;
    default:
        break;
    }

    return s->rc;
}

static vtss_rc vcl_cx_gen_func(cx_get_state_t *s)
{
    i8                                  buf[MGMT_PORT_BUF_SIZE], temp[50];
    u8                                  mac[6];
    vcl_mac_vlan_mgmt_entry_get_cfg_t   conf;
    vcl_proto_vlan_proto_entry_t        proto_conf;
    vcl_proto_vlan_vlan_entry_t         vlan_conf;
    vcl_ip_vlan_mgmt_entry_t            ip_conf;
    BOOL                                next = FALSE, first = TRUE;
#if VTSS_SWITCH_STACKABLE
    port_isid_info_t                    pinfo;
#endif

    switch (s->cmd) {
    case CX_GEN_CMD_GLOBAL:
        break;
    case CX_GEN_CMD_SWITCH:
        /* Switch - VCL */
        T_D("switch - vcl");
        CX_RC(cx_add_tag_line(s, CX_TAG_VCL, 0));

        /* MAC-based VLAN table */
        CX_RC(cx_add_tag_line(s, CX_TAG_MAC_VLAN_TABLE, 0));
        /* Entry syntax */
        CX_RC(cx_add_stx_start(s));
        CX_RC(cx_add_attr_txt(s, "mac_addr", "'xx-xx-xx-xx-xx-xx' or 'xx.xx.xx.xx.xx.xx' or 'xxxxxxxxxxxx' (x is a hexadecimal digit)"));
        CX_RC(cx_add_stx_ulong(s, "vid", VLAN_ID_MIN, VLAN_ID_MAX));
        CX_RC(cx_add_stx_port(s));
        CX_RC(cx_add_stx_end(s));
        while ((vcl_mac_vlan_mgmt_mac_vlan_get(s->isid, &conf, VCL_MAC_VLAN_USER_STATIC, next, first))
               == VTSS_RC_OK) {
            CX_RC(cx_add_attr_start(s, CX_TAG_ENTRY));
            memcpy(mac, conf.smac.addr, 6);
            sprintf(buf, "%02x-%02x-%02x-%02x-%02x-%02x",
                    mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
            CX_RC(cx_add_attr_txt(s, "mac_addr", buf));
            CX_RC(cx_add_attr_ulong(s, "vid", conf.vid));
#if VTSS_SWITCH_STACKABLE
            if (port_isid_info_get(s->isid, &pinfo) == VTSS_RC_OK) {
                conf.ports[s->isid - 1][pinfo.stack_port_0] = 0;
                conf.ports[s->isid - 1][pinfo.stack_port_1] = 0;
            }
#endif
            CX_RC(cx_add_attr_txt(s, "port", mgmt_iport_list2txt(conf.ports[s->isid - 1], buf)));
            CX_RC(cx_add_attr_end(s, CX_TAG_ENTRY));
            next = TRUE;
            first = FALSE;
        }
        CX_RC(cx_add_tag_line(s, CX_TAG_MAC_VLAN_TABLE, 1));
        /* Protocol-based Protocol table */
        CX_RC(cx_add_tag_line(s, CX_TAG_PROTO_VLAN_PROTO_TABLE, 0));
        CX_RC(cx_add_stx_start(s));
        CX_RC(cx_add_attr_txt(s, "encap_type", "ethernet or llc or snap"));
        CX_RC(cx_add_stx_ulong(s, "etype", 0x600, 0xFFFF));
        CX_RC(cx_add_stx_ulong(s, "dsap", 0x0, 0xFF));
        CX_RC(cx_add_stx_ulong(s, "ssap", 0x0, 0xFF));
        CX_RC(cx_add_attr_txt(s, "oui", "xx-xx-xx (hexadecimal digit characters)"));
        CX_RC(cx_add_stx_ulong(s, "pid", 0x0, 0xFFFF));
        sprintf(temp, "0-%u characters (digits & alphabets)", MAX_GROUP_NAME_LEN - 1);
        CX_RC(cx_add_attr_txt(s, "group_id", temp));
        CX_RC(cx_add_stx_end(s));
        next = FALSE;
        first = TRUE;
        while ((vcl_proto_vlan_mgmt_proto_get(&proto_conf, VCL_PROTO_VLAN_USER_STATIC, next, first))
               == VTSS_RC_OK) {
            CX_RC(cx_add_attr_start(s, CX_TAG_ENTRY));
            CX_RC(cx_add_attr_txt(s, "group_id", (char *)proto_conf.group_id));
            if (proto_conf.proto_encap_type == VCL_PROTO_ENCAP_ETH2) {
                CX_RC(cx_add_attr_txt(s, "encap_type", "ethernet"));
                CX_RC(cx_add_attr_ulong(s, "etype", proto_conf.proto.eth2_proto.eth_type));
            } else if (proto_conf.proto_encap_type == VCL_PROTO_ENCAP_LLC_SNAP) {
                CX_RC(cx_add_attr_txt(s, "encap_type", "snap"));
                sprintf(temp, "%02x-%02x-%02x", proto_conf.proto.llc_snap_proto.oui[0],
                        proto_conf.proto.llc_snap_proto.oui[1], proto_conf.proto.llc_snap_proto.oui[2]);
                CX_RC(cx_add_attr_txt(s, "oui", temp));
                CX_RC(cx_add_attr_ulong(s, "pid", proto_conf.proto.llc_snap_proto.pid));
            } else if (proto_conf.proto_encap_type == VCL_PROTO_ENCAP_LLC_OTHER) {
                CX_RC(cx_add_attr_txt(s, "encap_type", "llc"));
                CX_RC(cx_add_attr_ulong(s, "dsap", proto_conf.proto.llc_other_proto.dsap));
                CX_RC(cx_add_attr_ulong(s, "ssap", proto_conf.proto.llc_other_proto.ssap));
            }
            CX_RC(cx_add_attr_end(s, CX_TAG_ENTRY));
            next = TRUE;
            first = FALSE;
        }
        CX_RC(cx_add_tag_line(s, CX_TAG_PROTO_VLAN_PROTO_TABLE, 1));

        /* Protocol-based VLAN table */
        CX_RC(cx_add_tag_line(s, CX_TAG_PROTO_VLAN_VLAN_TABLE, 0));
        CX_RC(cx_add_stx_start(s));
        sprintf(temp, "0-%u characters (digits & alphabets)", MAX_GROUP_NAME_LEN - 1);
        CX_RC(cx_add_attr_txt(s, "group_id", temp));
        CX_RC(cx_add_stx_ulong(s, "vid", VLAN_ID_MIN, VLAN_ID_MAX));
        CX_RC(cx_add_stx_port(s));
        CX_RC(cx_add_stx_end(s));
        next = FALSE;
        first = TRUE;
        while ((vcl_proto_vlan_mgmt_group_entry_get(s->isid, &vlan_conf, VCL_PROTO_VLAN_USER_STATIC, next, first))
               == VTSS_RC_OK) {
            CX_RC(cx_add_attr_start(s, CX_TAG_ENTRY));
            CX_RC(cx_add_attr_txt(s, "group_id", (char *)vlan_conf.group_id));
            CX_RC(cx_add_attr_ulong(s, "vid", vlan_conf.vid));
            CX_RC(cx_add_attr_txt(s, "port", mgmt_iport_list2txt(vlan_conf.ports, buf)));
            CX_RC(cx_add_attr_end(s, CX_TAG_ENTRY));
            next = TRUE;
            first = FALSE;
        }
        CX_RC(cx_add_tag_line(s, CX_TAG_PROTO_VLAN_VLAN_TABLE, 1));
        /* IP-based VLAN table */
        CX_RC(cx_add_tag_line(s, CX_TAG_IP_VLAN_TABLE, 0));
        /* Entry syntax */
        CX_RC(cx_add_stx_start(s));
        CX_RC(cx_add_stx_ulong(s, "vce_id", 0, VCL_IP_VLAN_MAX_ENTRIES));
        CX_RC(cx_add_attr_txt(s, "ip_addr", "IP address a.b.c.d"));
        CX_RC(cx_add_stx_ulong(s, "mask_len", 1, 32));
        CX_RC(cx_add_stx_ulong(s, "vid", VLAN_ID_MIN, VLAN_ID_MAX));
        CX_RC(cx_add_stx_port(s));
        CX_RC(cx_add_stx_end(s));
        next = FALSE;
        first = TRUE;
        while ((vcl_ip_vlan_mgmt_ip_vlan_get(s->isid, &ip_conf, VCL_MAC_VLAN_USER_STATIC, first, next)) == VTSS_RC_OK) {
            CX_RC(cx_add_attr_start(s, CX_TAG_ENTRY));
            CX_RC(cx_add_attr_ulong(s, "vce_id", ip_conf.vce_id));
            CX_RC(cx_add_attr_ipv4(s, "ip_addr", ip_conf.ip_addr));
            CX_RC(cx_add_attr_ulong(s, "mask_len", ip_conf.mask_len));
            CX_RC(cx_add_attr_ulong(s, "vid", ip_conf.vid));
            CX_RC(cx_add_attr_txt(s, "port", mgmt_iport_list2txt(ip_conf.ports, buf)));
            CX_RC(cx_add_attr_end(s, CX_TAG_ENTRY));
            first = FALSE;
            next = TRUE;
        }
        CX_RC(cx_add_tag_line(s, CX_TAG_IP_VLAN_TABLE, 1));

        CX_RC(cx_add_tag_line(s, CX_TAG_VCL, 1));
        break;
    default:
        T_E("Unknown command");
        return VTSS_RC_ERROR;
    } /* End of Switch */
    return VTSS_OK;
}

/* Register the info in to the cx_module_table */
CX_MODULE_TAB_ENTRY(
    VTSS_MODULE_ID_VCL,
    vcl_cx_tag_table,
    0,
    0,
    NULL,                   /* init function       */
    vcl_cx_gen_func,        /* Generation fucntion */
    vcl_cx_parse_func       /* parse fucntion      */
);
