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

#ifdef VTSS_SW_OPTION_IP2
#ifndef VTSS_SW_OPTION_NTP
#ifdef VTSS_SW_OPTION_SNTP

#ifdef VTSS_SW_OPTION_SYSUTIL
#include "sysutil_api.h"
#endif

#include "main.h"
#include "vtss_sntp_api.h"

#include "conf_xml_api.h"
#include "conf_xml_trace_def.h"

/* Tag IDs */
enum {
    /* Module tags */
    CX_TAG_SNTP,

    /* Parameter tags */
    CX_TAG_ENTRY,
    CX_TAG_MODE,

    /* Last entry */
    CX_TAG_NONE
};

/* Tag table */
static cx_tag_entry_t sntp_cx_tag_table[CX_TAG_NONE + 1] = {
    [CX_TAG_SNTP] = {
        .name  = "sntp",
        .descr = "SNTP",
        .type = CX_TAG_TYPE_MODULE
    },
    [CX_TAG_ENTRY] = {
        .name  = "entry",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_MODE] = {
        .name  = "mode",
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

static vtss_rc sntp_cx_parse_func(cx_set_state_t *s)
{
    switch (s->cmd) {
    case CX_PARSE_CMD_PARM: {
        BOOL         global;
        BOOL         mode;
        sntp_conf_t  conf;
        ulong        server_idx = 1;
        char         *p = NULL;
        //access_mgmt_entry_t entry;
#ifdef VTSS_SW_OPTION_IPV6
        vtss_ipv6_t  ipv6_addr;
#endif /* VTSS_SW_OPTION_IPV6 */

        global = (s->isid == VTSS_ISID_GLOBAL);
        if (!global) {
            s->ignored = 1;
            break;
        }

        if (s->apply && sntp_config_get(&conf) != VTSS_OK) {
            break;
        }

        switch (s->id) {
        case CX_TAG_MODE:
            if (cx_parse_val_bool(s, &mode, 1) == VTSS_OK && s->apply) {
                conf.mode = mode;
                CX_RC(sntp_config_set(&conf));
            }
            break;
        case CX_TAG_ENTRY:
            for (; s->rc == VTSS_OK && cx_parse_attr(s) == VTSS_OK; s->p = s->next) {
                p = conf.sntp_server;
                if (cx_parse_ulong(s, "server", &server_idx, 1, SNTP_MAX_SERVER_COUNT) == VTSS_OK) {
                    //do nothing
                } else if (cx_parse_val_txt(s, p, SNTP_ADDRSTRLEN) == VTSS_OK) {
                    for ( ; p != NULL && *p != '\0'; p++)
                        if (*p == ' ') { /* Spaces not allowed */
                            CX_RC(cx_parm_invalid(s));
                        }
                    if (server_idx <= SNTP_MAX_SERVER_COUNT) {
                        conf.ip_type = SNTP_IP_TYPE_IPV4;
                    }
                }
#ifdef VTSS_SW_OPTION_IPV6
                else if (cx_parse_ipv6(s, "val", &ipv6_addr) == VTSS_OK && server_idx <= SNTP_MAX_SERVER_COUNT) {
                    conf.ip_type = SNTP_IP_TYPE_IPV6;
                    conf.ipv6_addr = ipv6_addr;
                }
#endif /* VTSS_SW_OPTION_IPV6 */
            }

            if (s->apply) {
                CX_RC(sntp_config_set(&conf));
            }

            break;
        default:
            s->ignored = 1;
            break;
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

static vtss_rc sntp_cx_gen_func(cx_get_state_t *s)
{
    switch (s->cmd) {
    case CX_GEN_CMD_GLOBAL:
        /* Global - sntp */
        T_D("global - sntp");
        CX_RC(cx_add_tag_line(s, CX_TAG_SNTP, 0));
        {
            sntp_conf_t conf;
            int i;

            if (sntp_config_get(&conf) == VTSS_OK) {
                CX_RC(cx_add_val_bool(s, CX_TAG_MODE, conf.mode));

                /* Entry syntax */
                CX_RC(cx_add_stx_start(s));
                CX_RC(cx_add_stx_ulong(s, "server", 1, SNTP_MAX_SERVER_COUNT));
                CX_RC(cx_add_attr_txt(s, "val", "a.b.c.d"));
                CX_RC(cx_add_stx_end(s));

                for (i = 0; i < SNTP_MAX_SERVER_COUNT; i++) {
                    CX_RC(cx_add_attr_start(s, CX_TAG_ENTRY));
                    CX_RC(cx_add_attr_ulong(s, "server", i + 1));

                    if (conf.ip_type == SNTP_IP_TYPE_IPV4) {
                        CX_RC(cx_add_attr_txt(s, "val", conf.sntp_server));
                    }
#ifdef VTSS_SW_OPTION_IPV6
                    else {
                        CX_RC(cx_add_attr_ipv6(s, "val", conf.ipv6_addr));
                    }
#endif /* VTSS_SW_OPTION_IPV6 */
                    CX_RC(cx_add_attr_end(s, CX_TAG_ENTRY));
                }
            }
        }
        CX_RC(cx_add_tag_line(s, CX_TAG_SNTP, 1));
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
CX_MODULE_TAB_ENTRY(
    VTSS_MODULE_ID_SNTP,
    sntp_cx_tag_table,
    0,
    0,
    NULL,                   /* init function       */
    sntp_cx_gen_func,       /* Generation fucntion */
    sntp_cx_parse_func      /* parse fucntion      */
);


#endif
#endif
#endif
