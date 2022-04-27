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
#include "dhcp_relay_api.h"

#include "conf_xml_api.h"
#include "conf_xml_trace_def.h"

/* Tag IDs */
enum {
    /* Module tags */
    CX_TAG_DHCP_RELAY,

    /* Parameter tags */
    CX_TAG_ADDR,
    CX_TAG_MODE,
    CX_TAG_DHCP_RELAY_INFO_MODE,
    CX_TAG_DHCP_RELAY_INFO_POLICY,

    /* Last entry */
    CX_TAG_NONE
};

/* Tag table */
static cx_tag_entry_t dhcp_relay_cx_tag_table[CX_TAG_NONE + 1] = {
    [CX_TAG_DHCP_RELAY] = {
        .name  = "dhcp_relay",
        .descr = "DHCP Relay",
        .type = CX_TAG_TYPE_MODULE
    },
    [CX_TAG_ADDR] = {
        .name  = "addr",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_MODE] = {
        .name  = "mode",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_DHCP_RELAY_INFO_MODE] = {
        .name  = "dhcp_relay_info_mode",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_DHCP_RELAY_INFO_POLICY] = {
        .name  = "dhcp_relay_info_policy",
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

/* Keyword for DHCP relay information policy */
static const cx_kw_t cx_kw_dhcp_relay_info_policy[] = {
    { "replace",    DHCP_RELAY_INFO_POLICY_REPLACE },
    { "keep",       DHCP_RELAY_INFO_POLICY_KEEP },
    { "drop",       DHCP_RELAY_INFO_POLICY_DROP },
    { NULL,       0 }
};

/*lint -sem(dhcp_relay_cx_parse_func, thread_protected)*/
/* Its safe to access global var 'relay_info_mode' */
static vtss_rc dhcp_relay_cx_parse_func(cx_set_state_t *s)
{
    static BOOL relay_info_mode = DHCP_RELAY_MGMT_ENABLED;

    switch (s->cmd) {
    case CX_PARSE_CMD_PARM: {
        ulong               val;
        BOOL                global;
        dhcp_relay_conf_t   conf;
        BOOL                mode;

        global = (s->isid == VTSS_ISID_GLOBAL);
        if (!global) {
            s->ignored = 1;
            break;
        }

        if (s->apply && dhcp_relay_mgmt_conf_get(&conf) != VTSS_OK) {
            break;
        }

        switch (s->id) {
        case CX_TAG_MODE:
            CX_RC( cx_parse_val_bool(s, &mode, 1));
            conf.relay_mode = mode;
            break;
        case CX_TAG_ADDR:
            CX_RC(cx_parse_val_ipv4(s, &conf.relay_server[0], NULL, 0));
            if (conf.relay_server[0] != 0) {
                conf.relay_server_cnt = 1;
            } else {
                conf.relay_server_cnt = 0;
            }
            break;
        case CX_TAG_DHCP_RELAY_INFO_MODE:
            CX_RC(cx_parse_val_bool(s, &relay_info_mode, 1));
            conf.relay_info_mode = relay_info_mode;
            break;
        case CX_TAG_DHCP_RELAY_INFO_POLICY:
            if (cx_parse_val_kw(s, cx_kw_dhcp_relay_info_policy, &val, 1) == VTSS_OK) {
                conf.relay_info_policy = val;
                if (relay_info_mode == DHCP_RELAY_MGMT_DISABLED && val == DHCP_RELAY_INFO_POLICY_REPLACE) {
                    CX_RC(cx_parm_error(s, "The 'Replace' policy is invalid when relay information mode is disabled."));
                }
            }
            break;
        default:
            s->ignored = 1;
            break;
        }
        if (s->apply) {
            CX_RC(dhcp_relay_mgmt_conf_set(&conf));
        }
        break;
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

static vtss_rc dhcp_relay_cx_gen_func(cx_get_state_t *s)
{
    switch (s->cmd) {
    case CX_GEN_CMD_GLOBAL:
        /* Global - dhcp_relay */
        T_D("global - dhcp_relay");
        CX_RC(cx_add_tag_line(s, CX_TAG_DHCP_RELAY, 0));
        {
            dhcp_relay_conf_t conf;

            if (dhcp_relay_mgmt_conf_get(&conf) == VTSS_OK) {
                /* We need set a vaild relay server first before enable DHCP relay mode */
                CX_RC(cx_add_val_ipv4(s, CX_TAG_ADDR, conf.relay_server[0]));
                CX_RC(cx_add_val_bool(s, CX_TAG_MODE, conf.relay_mode));
                CX_RC(cx_add_val_bool(s, CX_TAG_DHCP_RELAY_INFO_MODE, conf.relay_info_mode));
                CX_RC(cx_add_val_kw(s, CX_TAG_DHCP_RELAY_INFO_POLICY, cx_kw_dhcp_relay_info_policy, conf.relay_info_policy));
            }
        }
        CX_RC(cx_add_tag_line(s, CX_TAG_DHCP_RELAY, 1));
        break;
    case CX_GEN_CMD_SWITCH:
        break;
    default:
        T_E("Unknown command");
        return VTSS_RC_ERROR;
    } /* End of Switch */

    return VTSS_OK;
}
/* Register the info in to the cx_module_table */
CX_MODULE_TAB_ENTRY(VTSS_MODULE_ID_DHCP_RELAY, dhcp_relay_cx_tag_table,
                    0, 0,
                    NULL,                         /* init function       */
                    dhcp_relay_cx_gen_func,       /* Generation fucntion */
                    dhcp_relay_cx_parse_func);    /* parse fucntion      */
