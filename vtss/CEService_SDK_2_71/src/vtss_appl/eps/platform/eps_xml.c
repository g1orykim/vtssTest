/*

 Vitesse Switch API software.

 Copyright (c) 2002-2010 Vitesse Semiconductor Corporation "Vitesse". All
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
#include "eps.h"

#include "conf_xml_api.h"
#include "conf_xml_trace_def.h"

#include "port_api.h" /* Reqd for VTSS_FRONT_PORT_COUNT */

/* Tag IDs */
enum {
    /* Module tags */
    CX_TAG_EPS,

    /* Group tags */
    CX_TAG_PORT_TABLE,

    /* Parameter tags */
    CX_TAG_ENTRY,

    /* Last entry */
    CX_TAG_NONE
};

/* Tag table */
static cx_tag_entry_t eps_cx_tag_table[CX_TAG_NONE + 1] =
{
    [CX_TAG_EPS] = {
        .name  = "EPS",
        .descr = "EPS",
        .type = CX_TAG_TYPE_MODULE
    },
    [CX_TAG_PORT_TABLE] = {
        .name  = "port_table",
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

static const cx_kw_t cx_kw_eps_wtr_timer[] = {
    { "0s",  EPS_WTR_0S },
    { "10s",  EPS_WTR_10S },
    { "20s",  EPS_WTR_20S },
    { "5m",   EPS_WTR_5M },
    { "6m",   EPS_WTR_6M },
    { "7m",   EPS_WTR_7M },
    { "8m",   EPS_WTR_8M },
    { "9m",   EPS_WTR_9M },
    { "10m",   EPS_WTR_10M },
    { "11m",   EPS_WTR_11M },
    { "12m",   EPS_WTR_12M },
    { NULL,       0 }
};

static BOOL cx_eps_match(const cx_table_context_t *context, ulong port_a, ulong port_b)
{
    vtss_rc   rc;
    eps_mgmt_conf_blk_t     config;

    if ((rc = eps_mgmt_config_get(&config)) != VTSS_OK)
    {
        T_EG(VTSS_TRACE_GRP_EPS,"Error getting EPS configuration - %s\n", error_txt(rc));
        return false;
    }

    // Skip port that do not have EPS
    if (port_a >= EPS_ENABLED_MAX || port_b >= EPS_ENABLED_MAX) {
        return false;
    }


    return (config.prot_enabled[port_a] == config.prot_enabled[port_b] &&
            config.wtr_timer[port_a] == config.wtr_timer[port_b] &&
            config.protecting[port_a]  ==  config.protecting[port_b]);

}

static vtss_rc cx_eps_print(cx_get_state_t *s, const cx_table_context_t *context, ulong port_no, char *ports)
{

    vtss_rc   rc;
    eps_mgmt_conf_blk_t     config;
    char buf[80];

    T_DG(VTSS_TRACE_GRP_EPS,"cx_eps_print, port_no = %d",(int)port_no);
    if ((rc = eps_mgmt_config_get(&config)) != VTSS_OK)
    {
        T_EG(VTSS_TRACE_GRP_EPS,"Error gettong EPS configuration - %s\n", error_txt(rc));
        return VTSS_OK;
    }

    // Skip ports that do not have EPS.
    if (port_no > EPS_ENABLED_MAX) {return VTSS_OK; }

    if (ports == NULL) {
        /* Syntax */
        cx_add_stx_start(s);


        cx_add_attr_txt(s, "port", cx_list_txt(buf, 1, EPS_ENABLED_MAX));

        cx_add_stx_bool(s, "protection");
        cx_add_stx_ulong(s, "protection_port", 1, EPS_ENABLED_MAX);
        cx_add_stx_kw(s, "wtr_timer", cx_kw_eps_wtr_timer);
        cx_add_stx_ulong(s, "wtr", 0,EPS_WTR_MAX);
        return cx_add_stx_end(s);
    }

    cx_add_port_start(s, CX_TAG_ENTRY, ports);
    cx_add_attr_bool(s, "protection",config.prot_enabled[port_no]);
    cx_add_attr_ulong(s,"protection_port", iport2uport(config.protecting[port_no]));
    T_DG(VTSS_TRACE_GRP_EPS,"wtr timer = %u",config.wtr_timer[port_no]);
    cx_add_attr_kw(s, "wtr_timer", cx_kw_eps_wtr_timer, eps_ms2wtr(config.wtr_timer[port_no])); // Convert from ms to eps_wtr type.
    return cx_add_port_end(s, CX_TAG_ENTRY);
}

static vtss_rc eps_cx_parse_func(vtss_cx_set_state_t *s)
{
    switch (s->cmd) {
    case CX_PARSE_CMD_PARM:
    {
        vtss_port_no_t port_idx;
        BOOL           port_list[VTSS_PORT_ARRAY_SIZE+1];
        int            i;
        ulong          val;
        BOOL           global;
        vtss_rc        eps_rc;
        eps_mgmt_conf_blk_t config;

        global = (s->isid == VTSS_ISID_GLOBAL);

        T_RG(VTSS_TRACE_GRP_EPS,"Entering CX_TAG_EPS");
        if ((eps_rc = eps_mgmt_config_get(&config)) != VTSS_OK)
        {
            T_EG(VTSS_TRACE_GRP_EPS,"Error gettting EPS configuration - %s\n", error_txt(eps_rc));
            sprintf(s->msg, "Error gettting EPS configuration");
            s->ignored = 1;
        }


        switch (s->id) {
        case CX_TAG_ENTRY:
            if (s->group == CX_TAG_PORT_TABLE &&
                cx_parse_ports(s, port_list, 1) == VTSS_OK) {

                T_RG(VTSS_TRACE_GRP_EPS,"EPS Entering CX_TAG_PORT_TABLE");
                BOOL                 protection_enable = 0;
                BOOL                 protection_enable_update = 0;
                int                  protection_port = 0;
                BOOL                 protection_port_update = 0;
                int                  wtr = 0;
                BOOL                 wtr_update = 0;

                T_DG(VTSS_TRACE_GRP_EPS,"EPS Update mode");
                s->p = s->next;
                for (; s->rc == VTSS_OK && cx_parse_attr(s) == VTSS_OK; s->p = s->next) {
                    if (cx_parse_kw(s, "protection", cx_kw_mirror, &val, 1) == VTSS_OK) {
                        protection_enable_update = 1;
                        protection_enable= val;
                    }

                    if (cx_parse_ulong(s, "protection_port", &val, 1, EPS_ENABLED_MAX) == VTSS_OK) {
                        protection_port_update = 1;
                        protection_port = val;
                    }

                    if (cx_parse_kw(s, "wtr_timer",cx_kw_eps_wtr_timer, &val, 1) == VTSS_OK) {
                        wtr_update = 1;
                        wtr = eps_wtr2ms(val); // Convert to milliseconds
                    }
                }

                if (s->apply) {
                    for (port_idx = VTSS_PORT_NO_START; port_idx < VTSS_FRONT_PORT_COUNT; port_idx++) {
                        if (port_idx > EPS_ENABLED_MAX) {
                            continue;
                        }
                        if (port_list[iport2uport(port_idx)]) {
                            T_DG_PORT(VTSS_TRACE_GRP_EPS,port_idx,"EPD setting enable/disable");
                            if (protection_enable_update) {
                                config.prot_enabled[port_idx] = protection_enable;
                                }


                            if (protection_port_update) {
                                config.protecting[port_idx] = uport2iport(protection_port);
                            }

                            if (wtr_update) {
                                config.wtr_timer[port_idx] = wtr; // Convert from sec to ms.
                                T_DG_PORT(VTSS_TRACE_GRP_EPS,port_idx,"Wtr (ms)  = %u",config.wtr_timer[port_idx]);
                            }


                            // Update the configuration
                            if ((eps_rc = eps_mgmt_protection_set(port_idx,
                                                                  config.protecting[port_idx],
                                                                  config.architecture[port_idx],
                                                                  config.bidir_enabled[port_idx],
                                                                  config.prot_enabled[port_idx] )) != VTSS_OK)
                            {
                                s->ignored = 1;
                                sprintf(s->msg, "Invalid EPS settings for port %lu", iport2uport(port_idx));
                                T_EG_PORT(VTSS_TRACE_GRP_EPS,port_idx,"Invalid EPS settings - %s\n", error_txt(eps_rc));
                            }

                            if ((eps_rc = eps_mgmt_wtr_set(port_idx, config.wtr_timer[port_idx])) != VTSS_OK) {
                                s->ignored = 1;
                                sprintf(s->msg, "Invalid EPS WTR settings for port %lu", iport2uport(port_idx));
                                T_EG_PORT(VTSS_TRACE_GRP_EPS,port_idx,"Invalid EPS WTR settings - %s\n",error_txt(eps_rc));
                            }
                        }
                    }
                }
            } else {
                T_RG(VTSS_TRACE_GRP_EPS,"Port table ignored");
                s->ignored = 1;
            }
            break;

        case CX_TAG_PORT_TABLE:
            T_RG(VTSS_TRACE_GRP_EPS,"Entering CX_TAG_PORT_TABLE");
            if (global)
                s->ignored = 1;
            break;


        default:
            break;
        }

        if (global && s->apply) {
            T_NG(VTSS_TRACE_GRP_EPS,"EPS  apply, isid = %d",s->isid);
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

static vtss_rc synce_cx_gen_func(vtss_cx_set_state_t *s)
{
    switch (s->cmd) {
        case CX_GEN_CMD_GLOBAL:
            break;
        case CX_GEN_CMD_SWITCH:
            T_DG(VTSS_TRACE_GRP_EPS,"switch - EPS");
            cx_add_tag_line(s, CX_TAG_EPS, 0);
            CX_RC(cx_add_port_table(s, isid, CX_TAG_PORT_TABLE, cx_eps_match, cx_eps_print));
            CX_RC(cx_add_tag_line(s, CX_TAG_EPS, 1));
            break;
        default:
            T_E("Unknown command");
            return VTSS_RC_ERROR;
            break;
    } /* End of Switch */
    return VTSS_RC_OK;
}

/* Register the info in to the cx_module_table */
CX_MODULE_TAB_ENTRY(VTSS_MODULE_ID_EPS, eps_cx_tag_table,
                    0, 0,
                    NULL,                  /* init function       */
                    eps_cx_gen_func,       /* Generation fucntion */
                    eps_cx_parse_func);    /* parse fucntion      */
