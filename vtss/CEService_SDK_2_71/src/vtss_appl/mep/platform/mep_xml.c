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
#include "mep.h"

#include "conf_xml_api.h"
#include "conf_xml_trace_def.h"
#include "port_api.h" /* Reqd for VTSS_FRONT_PORT_COUNT */

/* Tag IDs */
enum {
    /* Module tags */
    CX_TAG_MEP,

    /* Group tags */
    CX_TAG_PORT_TABLE,

    /* Parameter tags */
    CX_TAG_ENTRY,

    /* Last entry */
    CX_TAG_NONE
};

/* Tag table */
static cx_tag_entry_t mep_cx_tag_table[CX_TAG_NONE + 1] =
{
    [CX_TAG_MEP] = {
        .name  = "MEP",
        .descr = "MEP",
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

static const cx_kw_t cx_kw_mep_period[] = {
    { "300S",  CCM_PERIOD_300S },
    { "100S",  CCM_PERIOD_100S },
    { "10S",  CCM_PERIOD_10S },
    { "1S",  CCM_PERIOD_1S },
    { "6M",  CCM_PERIOD_6M },
    { "1M",  CCM_PERIOD_1M },
    { "6H",  CCM_PERIOD_6H },
    { NULL,       0 }
};


static BOOL cx_mep_match(const cx_table_context_t *context, ulong port_a, ulong port_b)
{
    vtss_rc   rc;
    mep_mgmt_conf_blk_t     config;

    if ((rc = mep_mgmt_config_get(&config)) != VTSS_OK)
    {
        T_EG(VTSS_TRACE_GRP_MEP,";Error getting MEP configuration - %s\n", error_txt(rc));
        return false;
    }


    // Skip port that do not have MEP
    if (port_a >= MEP_ENABLED_MAX || port_b >= MEP_ENABLED_MAX) {
        return false;
    }


    return (config.enabled[port_a] == config.enabled[port_b] &&
            config.period[port_a]  == config.period[port_b] &&
            config.level[port_a]   == config.level[port_b] &&
            !memcmp(config.meg[port_a],config.meg[port_b],MEP_MEG_STR_LEN) &&
            config.period[port_a]  == config.period[port_b] &&
            config.tx_mep[port_a]  == config.tx_mep[port_b] &&
            config.rx_mep[port_a]  == config.rx_mep[port_b]);

}

static vtss_rc cx_mep_print(cx_get_state_t *s, const cx_table_context_t *context, ulong port_no, char *ports)
{
    vtss_rc   rc;
    mep_mgmt_conf_blk_t     config;

    if ((rc = mep_mgmt_config_get(&config)) != VTSS_OK)
    {
        T_EG(VTSS_TRACE_GRP_MEP,";Error getting MEP configuration - %s\n", error_txt(rc));
        return false;
    }
    char buf[80];


    // Skip ports that do not have MEP.
    if (port_no > MEP_ENABLED_MAX) {return VTSS_OK; }

    if (ports == NULL) {

        // Help text
        /* Syntax */
        cx_add_stx_start(s);
        cx_add_attr_txt(s, "port", cx_list_txt(buf, 1, MEP_ENABLED_MAX));
        cx_add_stx_bool(s, "enabled");

        sprintf(buf, "%d ASCII characters",MEP_MEG_STR_LEN);
        cx_add_attr_txt(s,"meg_id",buf);
        cx_add_stx_ulong(s, "tx_mep", 0, MEP_TX_MAX);
        cx_add_stx_ulong(s, "exp_mep", 0, MEP_RX_MAX);
        cx_add_stx_ulong(s, "level", 0, MEP_LEVEL_MAX);
        cx_add_attr_txt(s, "period", "300s|100s|10s|1s|6m|1m|6h");
        return cx_add_stx_end(s);
    }

    cx_add_port_start(s, CX_TAG_ENTRY, ports);
    cx_add_attr_bool(s, "enabled",config.enabled[port_no]);


    strncpy(buf, config.meg[port_no],MEP_MEG_STR_LEN);
    buf[MEP_MEG_STR_LEN] = '\0'; // Add NULL terminator.
    cx_add_attr_txt(s, "meg_id", buf);

    cx_add_attr_ulong(s,"tx_mep", config.tx_mep[port_no]);
    cx_add_attr_ulong(s,"exp_mep", config.rx_mep[port_no]);
    cx_add_attr_ulong(s,"level", config.level[port_no]);
    cx_add_attr_kw(s, "period", cx_kw_mep_period,config.period[port_no]);
    return cx_add_port_end(s, CX_TAG_ENTRY);
}

static vtss_rc mep_cx_parse_func(cx_set_state_t *s)
{
    switch (s->cmd) {
    case CX_PARSE_CMD_PARM:
    {
        char           buf[256];
        vtss_port_no_t port_idx;
        BOOL           port_list[VTSS_PORT_ARRAY_SIZE+1];
        int            i;
        ulong          val;
        BOOL           global;

        vtss_rc        mep_rc;
        mep_mgmt_conf_blk_t config;

        global = (s->isid == VTSS_ISID_GLOBAL);

        if ((mep_rc = mep_mgmt_config_get(&config)) != VTSS_OK)
        {
            T_EG(VTSS_TRACE_GRP_MEP,";Error getting MEP configuration - %s\n", error_txt(mep_rc));
            sprintf(s->msg, "Error gettting MEP configuration");
            s->ignored = 1;
        }

        switch (s->id) {
        case CX_TAG_ENTRY:
            if (s->group == CX_TAG_PORT_TABLE &&
                cx_parse_ports(s, port_list, 1) == VTSS_OK) {

                T_RG(VTSS_TRACE_GRP_MEP,"MEP Entering CX_TAG_PORT_TABLE");
                BOOL                 enabled = 0;
                BOOL                 enabled_update = 0;
                uint                  tx_mep = 0;
                BOOL                 tx_mep_update = 0;
                char                 meg_id[MEP_MEG_STR_LEN];
                BOOL                 meg_id_update = 0;

                uint                  rx_mep = 0;
                BOOL                 rx_mep_update = 0;

                int                  level = 0;
                BOOL                 level_update = 0;


                uint                 period = CCM_PERIOD_300S;
                BOOL                 period_update = 0;

                T_DG(VTSS_TRACE_GRP_MEP,"MEP Update mode");
                s->p = s->next;
                for (; s->rc == VTSS_OK && cx_parse_attr(s) == VTSS_OK; s->p = s->next) {
                    if (cx_parse_kw(s, "enabled", cx_kw_mirror, &val, 1) == VTSS_OK) {
                        enabled_update = 1;
                        enabled= val;
                    }


                    if (cx_parse_ulong(s, "tx_mep", &val,0,MEP_TX_MAX) == VTSS_OK) {
                        tx_mep_update = 1;
                        tx_mep = val;
                    }


                    if (cx_parse_txt(s, "meg_id", buf, MEP_MEG_STR_LEN +1) == VTSS_OK) {
                        T_DG(VTSS_TRACE_GRP_MEP,"MEP ID = %s, buf_len = %d",buf, strlen(buf));
                        if (strlen(buf) != 6) {
                            sprintf(s->msg, "mep_id : string to short, string must be %d charaters long",MEP_MEG_STR_LEN);
                            s->rc = CONF_XML_ERROR_FILE_PARM;
                        } else {
                            meg_id_update = 1;
                            memcpy(meg_id,buf,MEP_MEG_STR_LEN);
                        }
                    }

                    if (cx_parse_kw(s, "period",cx_kw_mep_period, &val, 1) == VTSS_OK) {
                        period_update = 1;
                        period  = val;
                    }



                    if (cx_parse_ulong(s, "exp_mep", &val,0, MEP_RX_MAX) == VTSS_OK) {
                        rx_mep_update = 1;
                        rx_mep = val;
                    }

                    if (cx_parse_ulong(s, "level", &val,0, MEP_LEVEL_MAX) == VTSS_OK) {
                        level_update = 1;
                        level = val;
                    }
                }

                if (s->apply) {
                    for (port_idx = VTSS_PORT_NO_START; port_idx < VTSS_FRONT_PORT_COUNT; port_idx++) {
                        if (port_idx > MEP_ENABLED_MAX) {
                            continue;
                        }
                        if (port_list[iport2uport(port_idx)]) {

                            T_DG_PORT(VTSS_TRACE_GRP_MEP,port_idx,"MEP setting enable/disable");
                            if (enabled_update) {
                                config.enabled[port_idx] = enabled;
                            }


                            if (tx_mep_update) {
                                config.tx_mep[port_idx] = tx_mep;
                            }


                            if (meg_id_update) {
                                memcpy(config.meg[port_idx],meg_id,MEP_MEG_STR_LEN);
                            }


                            if (rx_mep_update) {
                                config.rx_mep[port_idx] = rx_mep;
                            }

                            if (level_update) {
                                config.level[port_idx] = level;
                            }

                            if (period_update) {
                                config.period[port_idx] = period;
                            }


                            // Update the configuration
                            if ((mep_rc = mep_mgmt_instance_create_set(port_idx, config.enabled[port_idx],
                                                                       config.level[port_idx],
                                                                       config.meg[port_idx],
                                                                       config.tx_mep[port_idx],
                                                                       config.rx_mep[port_idx],
                                                                       config.period[port_idx])) != VTSS_OK) {

                                sprintf(s->msg, "Invalid MEP settings for port %lu", iport2uport(port_idx));
                                s->rc = CONF_XML_ERROR_FILE_PARM;
                                T_EG(VTSS_TRACE_GRP_MEP,"Invalid MEP settings for port %lu - %s\n",port_idx, error_txt(mep_rc));
                            }

                        }
                    }
                }
            } else {
                T_RG(VTSS_TRACE_GRP_MEP,"Port table ignored");
                s->ignored = 1;
            }
            break;

        case CX_TAG_PORT_TABLE:
            T_RG(VTSS_TRACE_GRP_MEP,"Entering CX_TAG_PORT_TABLE");
            if (global)
                s->ignored = 1;
            break;
        default:
            break;
        }

        if (global && s->apply) {
            T_NG(VTSS_TRACE_GRP_MEP,"MEP  apply, isid = %d",s->isid);
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

static vtss_rc mep_cx_gen_func(cx_get_state_t *s)
{
    switch (s->cmd) {
    case CX_GEN_CMD_GLOBAL:
        break;
    case CX_GEN_CMD_SWITCH:
        T_DG(VTSS_TRACE_GRP_EPS,"switch - MEP");
        cx_add_tag_line(s, CX_TAG_MEP, 0);
        CX_RC(cx_add_port_table(s, s->isid, CX_TAG_PORT_TABLE,
                                cx_mep_match, cx_mep_print));
        CX_RC(cx_add_tag_line(s, CX_TAG_MEP, 1));
        break;
    default:
        T_E("Unknown command");
        return VTSS_RC_ERROR;
        break;
    } /* End of Switch */

    return VTSS_OK;
}

/* Register the info in to the cx_module_table */
CX_MODULE_TAB_ENTRY(
    VTSS_MODULE_ID_MEP,
    mep_cx_tag_table,
    0,
    0,
    NULL,                /* init function       */
    mep_cx_gen_func,     /* Generation fucntion */
    mep_cx_parse_func    /* parse fucntion      */
);

