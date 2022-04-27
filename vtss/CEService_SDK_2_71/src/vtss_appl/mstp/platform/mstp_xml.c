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

#include "conf_xml_api.h"
#include "conf_xml_trace_def.h"
#include "mstp_xml.h"
#include "misc_api.h"
#include "vlan_api.h"

/*lint --e{534} */

/* Tag IDs */
enum {
    /* Module tags */
    CX_TAG_STP,

    /* Group tags */
    CX_TAG_PORT_TABLE,
    CX_TAG_MSTP_MSTI_TABLE,
    CX_TAG_MSTP_MSTI_PRIORITIES,
    CX_TAG_MSTP_MSTI_PORTS,

    /* Parameter tags */
    CX_TAG_VERSION,
    CX_TAG_ENTRY,
    CX_TAG_NAME,
    CX_TAG_AGE,
    CX_TAG_DELAY,
    CX_TAG_TXHOLDCOUNT,
    CX_TAG_MAXHOPS,
    CX_TAG_MSTP_BPDUFILTER,
    CX_TAG_MSTP_BPDUGUARD,
    CX_TAG_MSTP_RECOVERY,

    /* Last entry */
    CX_TAG_NONE
};

/* Tag table */
static cx_tag_entry_t mstp_cx_tag_table[CX_TAG_NONE + 1] =
{
    [CX_TAG_STP] = {
        .name  = "stp",
        .descr = "Spanning Tree Protocol",
        .type = CX_TAG_TYPE_MODULE
    },
    [CX_TAG_PORT_TABLE] = {
        .name  = "port_table",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },
    [CX_TAG_MSTP_MSTI_TABLE] = {
        .name  = "msti_mapping",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },
    [CX_TAG_MSTP_MSTI_PRIORITIES] = {
        .name  = "msti_priorities",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },
    [CX_TAG_MSTP_MSTI_PORTS] = {
        .name  = "msti_ports",
        .descr = "",
        .type = CX_TAG_TYPE_GROUP
    },
    [CX_TAG_VERSION] = {
        .name  = "version",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_ENTRY] = {
        .name  = "entry",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_NAME] = {
        .name  = "name",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_AGE] = {
        .name  = "age",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_DELAY] = {
        .name  = "delay",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_TXHOLDCOUNT] = {
        .name  = "txholdcount",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_MAXHOPS] = {
        .name  = "maxhops",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_MSTP_BPDUFILTER] = {
        .name  = "bpdufiltering",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_MSTP_BPDUGUARD] = {
        .name  = "bpduguard",
        .descr = "",
        .type = CX_TAG_TYPE_PARM
    },
    [CX_TAG_MSTP_RECOVERY] = {
        .name  = "recovery",
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


/* Keyword for STP version */
static const cx_kw_t cx_kw_stp_version[] = {
    { "mstp",       MSTP_PROTOCOL_VERSION_MSTP },
    { "rstp",       MSTP_PROTOCOL_VERSION_RSTP },
    { "stp",        MSTP_PROTOCOL_VERSION_COMPAT },
    { NULL,         0 }
};

/* Keyword for STP point2point */
static const cx_kw_t cx_kw_stp_p2p[] = {
    { "enabled",  P2P_FORCETRUE },
    { "disabled", P2P_FORCEFALSE },
    { "auto",     P2P_AUTO },
    { NULL,       0 }
};

static BOOL cx_stp_match(const cx_table_context_t *context, ulong port_a, ulong port_b)
{
    mstp_port_param_t conf_a, conf_b;
    BOOL ena_a, ena_b;

    return (mstp_get_port_config(context->isid, port_a, &ena_a, &conf_a) &&
            mstp_get_port_config(context->isid, port_b, &ena_b, &conf_b) &&
            ena_a == ena_b &&
            memcmp(&conf_a, &conf_b, sizeof(conf_a)) == 0);
}

static vtss_rc cx_stp_print(cx_get_state_t *s, const cx_table_context_t *context, ulong port_no, char *ports)
{
    mstp_port_param_t conf;
    BOOL              enable;

    if (ports == NULL) {
        /* Syntax */
        CX_RC(cx_add_stx_start(s));
        if (port_no == VTSS_PORT_NO_NONE) {
            CX_RC(cx_add_attr_txt(s, "port", "aggr"));
        } else {
            CX_RC(cx_add_stx_port(s));
        }
        CX_RC(cx_add_stx_bool(s, "mode"));
        CX_RC(cx_add_stx_bool(s, "adminedge"));
        CX_RC(cx_add_stx_bool(s, "autoedge"));
        CX_RC(cx_add_stx_kw(s, "p2p", cx_kw_stp_p2p));
        CX_RC(cx_add_stx_bool(s, "restrictedrole"));
        CX_RC(cx_add_stx_bool(s, "restrictedtcn"));
        CX_RC(cx_add_stx_bool(s, "bpduguard"));
        return cx_add_stx_end(s);
    }

    CX_RC(mstp_get_port_config(context ? context->isid : VTSS_ISID_GLOBAL, port_no, &enable, &conf) ? VTSS_OK : VTSS_UNSPECIFIED_ERROR);
    CX_RC(cx_add_port_start(s, CX_TAG_ENTRY, ports));
    CX_RC(cx_add_attr_bool(s, "mode", enable));
    CX_RC(cx_add_attr_bool(s, "adminedge", conf.adminEdgePort));
    CX_RC(cx_add_attr_bool(s, "autoedge", conf.adminAutoEdgePort));
    CX_RC(cx_add_attr_kw(s, "p2p", cx_kw_stp_p2p, conf.adminPointToPointMAC));
    CX_RC(cx_add_attr_bool(s, "restrictedrole", conf.restrictedRole));
    CX_RC(cx_add_attr_bool(s, "restrictedtcn", conf.restrictedTcn));
    CX_RC(cx_add_attr_bool(s, "bpduguard", conf.bpduGuard));
    return cx_add_port_end(s, CX_TAG_ENTRY);
}

static BOOL cx_stp_mstiport_match(const cx_table_context_t *context, ulong port_a, ulong port_b)
{
    mstp_msti_port_param_t conf_a, conf_b;
    u8 msti = *(u8*) context->custom; /* Used for MSTI index */

    return (mstp_get_msti_port_config(context->isid,
                                      msti, port_a, &conf_a) &&
            mstp_get_msti_port_config(context->isid,
                                      msti, port_b, &conf_b) &&
            memcmp(&conf_a, &conf_b, sizeof(conf_a)) == 0);
}

static vtss_rc cx_stp_mstiport_print(cx_get_state_t *s, const cx_table_context_t *context, ulong port_no, char *ports)
{
    mstp_msti_port_param_t conf;
    if (ports == NULL) {
        /* Syntax */
        CX_RC(cx_add_stx_start(s));
        if (port_no == VTSS_PORT_NO_NONE) {
            CX_RC(cx_add_attr_txt(s, "port", "aggr"));
        } else {
            CX_RC(cx_add_stx_port(s));
        }
        CX_RC(cx_add_stx_ulong(s, "msti", 0, N_MSTI_MAX-1));
        CX_RC(cx_add_stx_ulong(s, "pathcost", 0, 200000000));
        CX_RC(cx_add_stx_ulong(s, "priority", 0, 0xff));
        return cx_add_stx_end(s);
    }

    u8 msti = *(u8*) context->custom; /* Used for MSTI index */
    CX_RC(mstp_get_msti_port_config(context->isid, msti, port_no, &conf) ? VTSS_OK : VTSS_UNSPECIFIED_ERROR);
    CX_RC(cx_add_port_start(s, CX_TAG_ENTRY, ports));
    CX_RC(cx_add_attr_ulong(s, "msti", msti));
    CX_RC(cx_add_attr_ulong(s, "pathcost", conf.adminPathCost));
    CX_RC(cx_add_attr_ulong(s, "priority", conf.adminPortPriority));
    return cx_add_port_end(s, CX_TAG_ENTRY);
}

static vtss_rc mstp_cx_parse_func(cx_set_state_t *s)
{
    mstp_cx_set_state_t *state = s->mod_state;
    mstp_bridge_param_t *bconf = &state->bridge_conf;
    mstp_msti_config_t *mconf  = &state->msti_conf;
    switch (s->cmd) {
    case CX_PARSE_CMD_PARM:
    {
        char           buf[256];
        vtss_port_no_t port_idx;
        ulong          val;
        BOOL           global;

        global = (s->isid == VTSS_ISID_GLOBAL);

        switch (s->id) {
        case CX_TAG_MSTP_MSTI_TABLE:
        case CX_TAG_MSTP_MSTI_PRIORITIES:
        case CX_TAG_MSTP_MSTI_PORTS:
            break;
        case CX_TAG_AGE:
            if (global && cx_parse_val_ulong(s, &val, 6, 200) == VTSS_OK)
                bconf->bridgeMaxAge = val;
            else
                s->ignored = 1;
            break;
        case CX_TAG_DELAY:
            if (global && cx_parse_val_ulong(s, &val, 4, 30) == VTSS_OK)
                bconf->bridgeForwardDelay = val;
            else
                s->ignored = 1;
            break;
        case CX_TAG_VERSION:
            s->ignored = 1;
            if (global) {
                if(s->group == CX_TAG_MSTP_MSTI_TABLE) {
                    if(cx_parse_val_ulong(s, &val, 0, 0xffff) == VTSS_OK) {
                        mconf->revision = val;
                        s->ignored = 0;
                    }
                } else {
                    if(cx_parse_val_kw(s, cx_kw_stp_version, &val, 1) == VTSS_OK) {
                        bconf->forceVersion = val;
                        s->ignored = 0;
                    }
                }
            }
            break;
        case CX_TAG_TXHOLDCOUNT:
            if (global && cx_parse_val_ulong(s, &val, 1, 10) == VTSS_OK)
                bconf->txHoldCount = val;
            else
                s->ignored = 1;
            break;
        case CX_TAG_MAXHOPS:
            if (global && cx_parse_val_ulong(s, &val, 6, 40) == VTSS_OK)
                bconf->MaxHops = val;
            else
                s->ignored = 1;
            break;
        case CX_TAG_MSTP_BPDUFILTER:
            if (global && cx_parse_val_bool(s, &bconf->bpduFiltering, 1) != VTSS_OK)
                s->ignored = 1;
            break;
        case CX_TAG_MSTP_BPDUGUARD:
            if (global && cx_parse_val_bool(s, &bconf->bpduGuard, 1) != VTSS_OK)
                s->ignored = 1;
            break;
        case CX_TAG_MSTP_RECOVERY:
            if (global && cx_parse_val_ulong(s, &val, 0, 86400) == VTSS_OK)
                /* 0 = disable, 30 is minimum */
                bconf->errorRecoveryDelay = (val > 0 && val < 30) ? 30 : val;
            else
                s->ignored = 1;
            break;
        case CX_TAG_NAME:
            if (global && s->group == CX_TAG_MSTP_MSTI_TABLE &&
                /* NB: length +1 to allow max length = 32 (unterminated) */
                cx_parse_val_txt(s, buf, MSTP_CONFIG_NAME_MAXLEN+1) == VTSS_OK)
                strncpy(mconf->configname, buf, sizeof(mconf->configname));
            else
                s->ignored = 1;
            break;
        case CX_TAG_PORT_TABLE:
            if (global)
                s->ignored = 1;
            break;
        case CX_TAG_ENTRY:
            if (global && s->group == CX_TAG_MSTP_MSTI_TABLE) {
                if(cx_parse_ulong(s, "vid", &val, VLAN_ID_MIN, MIN(VLAN_ID_MAX, 4094)) == VTSS_OK) {
                    vtss_vid_t vid = (vtss_vid_t) val; /* Index */
                    s->p = s->next; /* Skip to "data" attributes */
                    if(cx_parse_ulong(s, "msti", &val, 1, N_MSTI_MAX-1) == VTSS_OK) {
                        mconf->map.map[vid] = (u8) val;
                    } else
                        s->ignored = 1;
                }
            } else if (global && s->group == CX_TAG_MSTP_MSTI_PRIORITIES) {
                if(cx_parse_ulong(s, "msti", &val, 0, N_MSTI_MAX-1) == VTSS_OK) {
                    u8 msti = (u8) val; /* Index */
                    s->p = s->next; /* Skip to "data" attributes */
                    if(cx_parse_ulong(s, "priority", &val, 0, 0xff) == VTSS_OK &&
                       (val & 0x0f) == 0) {
                        mstp_set_msti_priority(msti, (u8) val);
                    } else
                        s->ignored = 1;
                }
            } else if (s->group == CX_TAG_MSTP_MSTI_PORTS) {
                if((global &&
                    (s->rc = cx_parse_txt(s, "port", buf, sizeof(buf))) == VTSS_OK &&
                    strncmp(buf, "aggr", 4) == 0) ||
                   (!global &&
                    cx_parse_ports(s, state->port_list, 1) == VTSS_OK)) {
                    mstp_msti_port_param_t new;
                    BOOL pathcost = FALSE, priority = FALSE;
                    u32 msti = 0;

                    memset(&new, 0, sizeof(new));

                    s->p = s->next;
                    for (; s->rc == VTSS_OK && cx_parse_attr(s) == VTSS_OK; s->p = s->next) {
                        if (cx_parse_ulong(s, "msti", &val, 0, N_MSTI_MAX-1) == VTSS_OK)
                            msti = val;
                        else if(cx_parse_ulong(s, "pathcost", &val, 0, 200000000) == VTSS_OK) {
                            pathcost = TRUE;
                            new.adminPathCost = val;
                        } else if(cx_parse_ulong(s, "priority", &val, 0, 0xff) == VTSS_OK) {
                            priority = TRUE;
                            new.adminPortPriority = val & 0xF0;
                        } else {
                            cx_parm_unknown(s);
                        }
                    }

                    if (global) {
                        mstp_msti_port_param_t conf;
                        /* Aggregation setup */
                        if (s->apply && mstp_get_msti_port_config(0, msti, VTSS_PORT_NO_NONE, &conf)) {
                            if (pathcost)
                                conf.adminPathCost = new.adminPathCost;
                            if (priority)
                                conf.adminPortPriority = new.adminPortPriority;
                            mstp_set_msti_port_config(0, msti, VTSS_PORT_NO_NONE, &conf);
                        }
                    }
                    /* Port setup */
                    for (port_idx = VTSS_PORT_NO_START; port_idx < VTSS_PORT_NO_END; port_idx++)
                        if (state->port_list[iport2uport(port_idx)]) {
                            if (pathcost)
                                state->stp[port_idx].mstiport[msti].adminPathCost = new.adminPathCost;
                            if (priority)
                                state->stp[port_idx].mstiport[msti].adminPortPriority = new.adminPortPriority;
                        }
                }

            } else if ((global && (s->rc = cx_parse_txt(s, "port", buf, sizeof(buf))) == VTSS_OK &&
                        strncmp(buf, "aggr", 4) == 0) ||
                       (!global && s->group == CX_TAG_PORT_TABLE &&
                        cx_parse_ports(s, state->port_list, 1) == VTSS_OK)) {
                mstp_port_param_t new;
                BOOL b, new_enable_stp = FALSE, enable_stp = FALSE;
                BOOL mode = 0, adminedge = 0, autoedge = 0, p2p = 0, restrictedrole = 0, restrictedtcn = 0,
                    bpduguard = 0;

                memset(&new, 0, sizeof(new));

                s->p = s->next;
                for (; s->rc == VTSS_OK && cx_parse_attr(s) == VTSS_OK; s->p = s->next) {
                    if (cx_parse_bool(s, "mode", &new_enable_stp, 1) == VTSS_OK) {
                        mode = 1;
                    } else if (cx_parse_bool(s, "adminedge", &b, 1) == VTSS_OK) {
                        adminedge = 1;
                        new.adminEdgePort = b;
                    } else if (cx_parse_bool(s, "autoedge", &b, 1) == VTSS_OK) {
                        autoedge = 1;
                        new.adminAutoEdgePort = b;
                    } else if (cx_parse_kw(s, "p2p", cx_kw_stp_p2p, &val, 1) == VTSS_OK) {
                        p2p = 1;
                        new.adminPointToPointMAC = val;
                    } else if (cx_parse_bool(s, "restrictedrole", &b, 1) == VTSS_OK) {
                        restrictedrole = 1;
                        new.restrictedRole = b;
                    } else if (cx_parse_bool(s, "restrictedtcn", &b, 1) == VTSS_OK) {
                        restrictedtcn = 1;
                        new.restrictedTcn = b;
                    } else if (cx_parse_bool(s, "bpduguard", &b, 1) == VTSS_OK) {
                        bpduguard = 1;
                        new.bpduGuard = b;
                    }
                }
                if (global) {
                    mstp_port_param_t conf;
                    /* Aggregation setup */
                    if (s->apply && mstp_get_port_config(0, VTSS_PORT_NO_NONE, &enable_stp, &conf)) {
                        if (mode)
                            enable_stp = new_enable_stp;
                        if (adminedge)
                            conf.adminEdgePort = new.adminEdgePort;
                        if (autoedge)
                            conf.adminAutoEdgePort = new.adminAutoEdgePort;
                        if (p2p)
                            conf.adminPointToPointMAC = new.adminPointToPointMAC;
                        if (restrictedrole)
                            conf.restrictedRole = new.restrictedRole;
                        if (restrictedtcn)
                            conf.restrictedTcn = new.restrictedTcn;
                        if (bpduguard)
                            conf.bpduGuard = new.bpduGuard;
                        mstp_set_port_config(0, VTSS_PORT_NO_NONE, enable_stp, &conf);
                    }
                    break;
                }
                /* Port setup */
                for (port_idx = VTSS_PORT_NO_START; port_idx < VTSS_PORT_NO_END; port_idx++)
                    if (state->port_list[iport2uport(port_idx)]) {
                        if (mode) {
                            state->line_stp = s->line;
                            state->stp[port_idx].enable_stp = new_enable_stp;
                        }
                        if (adminedge)
                            state->stp[port_idx].conf.adminEdgePort = new.adminEdgePort;
                        if (autoedge)
                            state->stp[port_idx].conf.adminAutoEdgePort = new.adminAutoEdgePort;
                        if (p2p)
                            state->stp[port_idx].conf.adminPointToPointMAC = new.adminPointToPointMAC;
                        if (restrictedrole)
                            state->stp[port_idx].conf.restrictedRole = new.restrictedRole;
                        if (restrictedtcn)
                            state->stp[port_idx].conf.restrictedTcn = new.restrictedTcn;
                        if (bpduguard)
                            state->stp[port_idx].conf.bpduGuard = new.bpduGuard;
                    }
            } else
                s->ignored = 1;
            break;
        default:
            s->ignored = 1;
            break;
        }
        break;
    } /* CX_PARSE_CMD_PARM */
    case CX_PARSE_CMD_GLOBAL:
    {
        if (s->init) {
            (void) mstp_get_system_config(bconf);
            (void) mstp_get_msti_config(mconf, NULL);
            memset(&mconf->map, 0, sizeof(mconf->map));
        } else if (s->apply) {
            (void) mstp_set_msti_config(mconf);
            (void) mstp_set_system_config(bconf);
        }
        break;
    }
    case CX_PARSE_CMD_SWITCH:
    {
        port_iter_t pit;

        if (s->init) {
            state->line_stp.number = 0;
            
            memset(state->port_list, 0, sizeof(state->port_list));

            /* STP state */
            (void) port_iter_init(&pit, NULL, s->isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
            while (port_iter_getnext(&pit)) {
                int msti;
                state->stp[pit.iport].enable_stp = 0;
                mstp_get_port_config(s->isid, pit.iport, &state->stp[pit.iport].enable_stp, &state->stp[pit.iport].conf);
                for (msti = 0; msti < N_MSTI_MAX; msti++) {
                    mstp_get_msti_port_config(s->isid, msti, pit.iport, &state->stp[pit.iport].mstiport[msti]);
                }
            } /* end of while */
        } else if (s->apply) {
            int               msti;
            BOOL              do_update[VTSS_PORTS], enable_stp;
            mstp_port_param_t cur_conf;

            memset(do_update, 0, sizeof(do_update));

            // First step: Figure out if there's anything to update. This is needed to minimize the number
            // of calls into the VLAN module, because the code after this always starts by disabling STP and
            // then possibly re-enables it afterwards.
            (void)port_iter_init(&pit, NULL, s->isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
            while (port_iter_getnext(&pit)) {
                mstp_get_port_config(s->isid, pit.iport, &enable_stp, &cur_conf);
                if (enable_stp != state->stp[pit.iport].enable_stp || memcmp(&cur_conf, &state->stp[pit.iport].conf, sizeof(cur_conf)) != 0) {
                    do_update[pit.iport] = TRUE;
                    continue;
                }

                for (msti = 0; msti < N_MSTI_MAX; msti++) {
                    mstp_msti_port_param_t cur_mstiport;
                    mstp_get_msti_port_config(s->isid, msti, pit.iport, &cur_mstiport);
                    if (memcmp(&cur_mstiport, &state->stp[pit.iport].mstiport[msti], sizeof(cur_mstiport)) != 0) {
                        do_update[pit.iport] = TRUE;
                    }
                }
            }

            /* Disable MSTP mode */
            (void)port_iter_init(&pit, NULL, s->isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
            while (port_iter_getnext(&pit)) {

                if (do_update[pit.iport] && mstp_get_port_config(s->isid, pit.iport, &enable_stp, &cur_conf) && enable_stp) {
                    mstp_set_port_config(s->isid, pit.iport, FALSE, &cur_conf);
                }
            }

            /* Set STP mode */
            (void)port_iter_init(&pit, NULL, s->isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
            while (port_iter_getnext(&pit)) {
                if (do_update[pit.iport]) {
                    mstp_set_port_config(s->isid, pit.iport, state->stp[pit.iport].enable_stp, &state->stp[pit.iport].conf);
                    for (msti = 0; msti < N_MSTI_MAX; msti++) {
                        mstp_set_msti_port_config(s->isid, msti, pit.iport, &state->stp[pit.iport].mstiport[msti]);
                    }
                }
            }
        } /* end of else */
        break;
    } /* CX_PARSE_CMD_SWITCH */
    default:
        break;
    }

    return s->rc;
}

static vtss_rc mstp_cx_gen_func(cx_get_state_t *s)
{
    switch (s->cmd) {
        case CX_GEN_CMD_GLOBAL:
            /* Global - STP */
            T_D("global - mstp");
            CX_RC(cx_add_tag_line(s, CX_TAG_STP, 0));
            {
                mstp_bridge_param_t conf;

                if (mstp_get_system_config(&conf)) {
                    CX_RC(cx_add_val_ulong(s, CX_TAG_AGE, conf.bridgeMaxAge, 6, 40));
                    CX_RC(cx_add_val_ulong(s, CX_TAG_DELAY, conf.bridgeForwardDelay, 4, 30));
                    CX_RC(cx_add_val_kw(s, CX_TAG_VERSION, cx_kw_stp_version, 
                                        conf.forceVersion));
                    CX_RC(cx_add_val_ulong(s, CX_TAG_TXHOLDCOUNT, conf.txHoldCount, 1, 10));
                    CX_RC(cx_add_val_ulong(s, CX_TAG_MAXHOPS, conf.MaxHops, 6, 40));
                    CX_RC(cx_add_val_bool(s, CX_TAG_MSTP_BPDUFILTER, conf.bpduFiltering));
                    CX_RC(cx_add_val_bool(s, CX_TAG_MSTP_BPDUGUARD, conf.bpduGuard));
                    CX_RC(cx_add_val_ulong(s, CX_TAG_MSTP_RECOVERY, conf.errorRecoveryDelay, 
                                           0, 86400));
                }
            }

            CX_RC(cx_add_tag_line(s, CX_TAG_MSTP_MSTI_PRIORITIES, 0));
            {
                u8 priority;
                u8 msti;

                /* Entry syntax */
                CX_RC(cx_add_stx_start(s));
                CX_RC(cx_add_stx_ulong(s, "msti", 0, N_MSTI_MAX-1)); /* CIST, MST1, ... */
                CX_RC(cx_add_stx_ulong(s, "priority", 0, 0xff));
                CX_RC(cx_add_stx_end(s));

                /* The entries, CIST, MST1, ... */
                for (msti = 0; msti < N_MSTI_MAX; msti++) {
                    priority = mstp_get_msti_priority(msti);
                    CX_RC(cx_add_attr_start(s, CX_TAG_ENTRY));
                    CX_RC(cx_add_attr_ulong(s, "msti", msti));
                    CX_RC(cx_add_attr_ulong(s, "priority", priority));
                    CX_RC(cx_add_attr_end(s, CX_TAG_ENTRY));
                }
            }
            CX_RC(cx_add_tag_line(s, CX_TAG_MSTP_MSTI_PRIORITIES, 1));
            CX_RC(cx_add_tag_line(s, CX_TAG_MSTP_MSTI_TABLE, 0));
            {
                mstp_msti_config_t conf;
                if (mstp_get_msti_config(&conf, NULL)) {
                    CX_RC(cx_add_val_ulong(s, CX_TAG_VERSION, conf.revision, 0, 0xffff));
                    CX_RC(cx_add_val_txt(s, CX_TAG_NAME, conf.configname, NULL));

                    /* Entry syntax */
                    CX_RC(cx_add_stx_start(s));
                    CX_RC(cx_add_stx_ulong(s, "vid", VLAN_ID_MIN, MIN(VLAN_ID_MAX, 4094)));
                    CX_RC(cx_add_stx_ulong(s, "msti", 1, N_MSTI_MAX-1));
                    CX_RC(cx_add_stx_end(s));

                    uint i;
                    for(i = 0; i < ARRSZ(conf.map.map); i++) {
                        if(conf.map.map[i]) {
                            CX_RC(cx_add_attr_start(s, CX_TAG_ENTRY));
                            CX_RC(cx_add_attr_ulong(s, "vid", i));
                            CX_RC(cx_add_attr_ulong(s, "msti", conf.map.map[i]));
                            CX_RC(cx_add_attr_end(s, CX_TAG_ENTRY));
                        }
                    }
                }
            }
            CX_RC(cx_add_tag_line(s, CX_TAG_MSTP_MSTI_TABLE, 1));

            cx_stp_print(s, 0, VTSS_PORT_NO_NONE, NULL);
            cx_stp_print(s, 0, VTSS_PORT_NO_NONE, "aggr");

            CX_RC(cx_add_tag_line(s, CX_TAG_MSTP_MSTI_PORTS, 0));
            {
                cx_table_context_t context;
                u8 msti;
                context.isid = 0;       /* Not used */
                context.custom = &msti; /* Real index */
                cx_stp_mstiport_print(s, &context, VTSS_PORT_NO_NONE, NULL);
                for (msti = 0; msti < N_MSTI_MAX; msti++) {
                    cx_stp_mstiport_print(s, &context, VTSS_PORT_NO_NONE, "aggr");
                }
            }
            CX_RC(cx_add_tag_line(s, CX_TAG_MSTP_MSTI_PORTS, 1));

            CX_RC(cx_add_tag_line(s, CX_TAG_STP, 1));
            break;
        case CX_GEN_CMD_SWITCH:
            /* Switch - MSTP */
            T_D("switch - mstp");
            CX_RC(cx_add_tag_line(s, CX_TAG_STP, 0));
            CX_RC(cx_add_port_table(s, s->isid, CX_TAG_PORT_TABLE, cx_stp_match, cx_stp_print));
            u8 msti;
            for (msti = 0; msti < N_MSTI_MAX; msti++)
                CX_RC(cx_add_port_table_ex(s, s->isid, &msti, 
                            CX_TAG_MSTP_MSTI_PORTS,
                            cx_stp_mstiport_match, cx_stp_mstiport_print));
            CX_RC(cx_add_tag_line(s, CX_TAG_STP, 1));
            break;
        default:
            T_E("Unknown command");
            return VTSS_RC_ERROR;
    } /* End of Switch */

    return VTSS_OK;
}

/* Register the info in to the cx_module_table */
CX_MODULE_TAB_ENTRY(
    VTSS_MODULE_ID_RSTP,
    mstp_cx_tag_table,
    sizeof(mstp_cx_set_state_t),
    0,
    NULL,                    /* init function       */
    mstp_cx_gen_func,        /* Generation fucntion */
    mstp_cx_parse_func       /* parse fucntion      */
);

