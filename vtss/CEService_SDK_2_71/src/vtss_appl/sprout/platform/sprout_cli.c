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
#include "port_api.h"
#include "topo_api.h"
#include "sprout_cli.h"
#include "vtss_module_id.h"
#include "msg_api.h"
#include "cli_trace_def.h"

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_TOPO







static char SPROUT_sid_swap_str[40];
static char SPROUT_sid_mst_prio_str[40];
static char SPROUT_sid_select_str[40];


static void SPROUT_create_parser_strings(void)
{
    (void)snprintf(SPROUT_sid_swap_str,     sizeof(SPROUT_sid_swap_str),     "Switch ID (%d-%d). Default: Show SID", VTSS_USID_START, VTSS_USID_END - 1);
    (void)snprintf(SPROUT_sid_mst_prio_str, sizeof(SPROUT_sid_mst_prio_str), "Switch ID (%d-%d) or local switch",    VTSS_USID_START, VTSS_USID_END - 1);
    (void)snprintf(SPROUT_sid_select_str,   sizeof(SPROUT_sid_select_str),   "Switch ID (%d-%d) or all switches",    VTSS_USID_START, VTSS_USID_END - 1);
}

typedef struct {
    
    BOOL    detailed;
    BOOL    productinfo;
    BOOL    local;
    BOOL    stacking_specified;
} sprout_cli_req_t;


static BOOL cli_cmd_standalone(void)
{
    if (vtss_stacking_enabled()) {
        return 0;
    }

    CPRINTF("Stacking is currently disabled\n");
    return 1;
}

static void cli_cmd_stack_sid_assign(cli_req_t *req )
{
    vtss_rc rc;

    if (cli_cmd_standalone()) {
        return;
    }

    if ((rc = topo_isid_assign(topo_usid2isid(req->usid[0]), req->mac_addr)) != VTSS_OK) {
        CPRINTF("%s\n", error_txt(rc));
    }
}

static void cli_cmd_stack_config(cli_req_t *req)
{
    vtss_rc               rc;
    vtss_usid_t           usid;
    vtss_isid_t           isid;
    stack_config_t        conf;
    BOOL                  dirty, header = 1;
    char                  buf[80], buf_b[80];
    u32                   port_count, mask, count = 0;
    vtss_port_no_t        iport;
    BOOL                  iport_list_a[VTSS_PORTS], iport_list_b[VTSS_PORTS];
    BOOL                  list_a_valid, list_b_valid, list_b_first;
    port_isid_port_info_t info;
    sprout_cli_req_t      *sprout_req = req->module_req;

    if (cli_cmd_switch_none(req) || cli_cmd_slave(req)) {
        return;
    }

    if (req->disable) {
        mask = msg_existing_switches();
        for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
            if (mask & (1 << isid)) {
                count++;
            }
        }
        if (count > 1) {
            CPRINTF("Stacking can only be disabled if the stack consists of one switch\n");
            return;
        }
    }

    for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
        if ((isid = req->stack.isid[usid]) == VTSS_ISID_END) {
            continue;
        }
        if ((rc = topo_stack_config_get(isid, &conf, &dirty)) != VTSS_OK) {
            CPRINTF("Switch %u: %s\n", usid, error_txt(rc));
            continue;
        }
        if (req->set) {
            if (req->port) {
                conf.port_0 = uport2iport(req->int_values[0]);
                conf.port_1 = uport2iport(req->int_values[1]);
            }
            if (sprout_req->stacking_specified) {
                conf.stacking = req->enable;
            }
            if ((rc = topo_stack_config_set(isid, &conf)) != VTSS_OK) {
                CPRINTF("Switch %u: %s\n", usid, error_txt(rc));
            }
            continue;
        }
        if (header) {
            header = 0;
            sprintf(buf, "%sStacking  Stack Ports  Stack Capable Ports  Status",
                    vtss_stacking_enabled() ? "Switch  " : "");
            cli_table_header(buf);
        }
        if (vtss_stacking_enabled()) {
            CPRINTF("%-8u", usid);
        }
        
        for (iport = 0; iport < VTSS_PORTS; iport++) {
            iport_list_a[iport] = (iport == conf.port_0 || iport == conf.port_1 ? 1 : 0);
        }
        CPRINTF("%-10s%-13s", cli_bool_txt(conf.stacking), cli_iport_list_txt(iport_list_a, buf));

        
        port_count = port_isid_port_count(isid);
        list_a_valid = 0;
        list_b_valid = 0;
        list_b_first = 0;
        for (iport = 0; iport < VTSS_PORTS; iport++) {
            iport_list_a[iport] = 0;
            iport_list_b[iport] = 0;
            if (iport < port_count &&
                port_isid_port_info_get(isid, iport, &info) == VTSS_OK &&
                (info.cap & PORT_CAP_STACKING)) {
                if (info.chip_no == 1) {
                    
                    iport_list_b[iport] = 1;
                    list_b_valid = 1;
                    if (!list_a_valid) {
                        list_b_first = 1;
                    }
                } else {
                    
                    iport_list_a[iport] = 1;
                    list_a_valid = 1;
                }
            }
        }

        
        if (list_a_valid) {
            if (list_b_first) {
                cli_iport_list_txt(iport_list_b, buf);
                strcat(buf, " and ");
                strcat(buf, cli_iport_list_txt(iport_list_a, buf_b));
            } else {
                cli_iport_list_txt(iport_list_a, buf);
                if (list_b_valid) {
                    strcat(buf, " and ");
                    strcat(buf, cli_iport_list_txt(iport_list_b, buf_b));
                }
            }
        } else {
            cli_iport_list_txt(iport_list_b, buf);
        }
        {
            BOOL cur_ena = vtss_stacking_enabled();
            BOOL cfg_ena = conf.stacking;
            CPRINTF("%-21s%s\n", buf, ((dirty && cur_ena) || (cur_ena != cfg_ena)) ? "Reboot Required" : "Active");
        }
    }
}


static void cli_cmd_stack_sid_delete(cli_req_t *req )
{
    vtss_rc     rc;
    vtss_isid_t isid;

    if (cli_cmd_standalone()) {
        return;
    }

    isid = topo_usid2isid(req->usid[0]);
    if (msg_switch_is_master() && msg_switch_is_local(isid)) {
        CPRINTF("Master SID can not be deleted\n");
    } else if ((rc = topo_isid_delete(isid)) != VTSS_OK) {
        CPRINTF("%s\n", error_txt(rc));
    }
} 


static void cli_cmd_stack_sid_swap(cli_req_t *req )
{
    vtss_rc rc;

    if (cli_cmd_standalone()) {
        return;
    }

    if (req->usid[0] != req->usid[1] &&
        (rc = topo_usid_swap(req->usid[0], req->usid[1])) != VTSS_OK) {
        CPRINTF("%s\n", error_txt(rc));
    }
} 


static void cli_cmd_stack_select(cli_req_t *req )
{
    vtss_isid_t isid;
    char        buf[32];
    cli_io_t    *pIO = (cli_io_t *)cli_get_io_handle();

    if (cli_cmd_standalone()) {
        return;
    }

    if (req->set) {
        if (req->all) {
            pIO->cmd.usid = VTSS_USID_ALL;
        } else {
            isid = topo_usid2isid(req->usid_sel);
            if (msg_switch_exists(isid)) {
                pIO->cmd.usid = req->usid_sel;
            } else {
                CPRINTF("Switch %d is not present in stack\n", req->usid_sel);
            }
        }
    } else {
        if (req->usid_sel == VTSS_USID_ALL) {
            strcpy(buf, "All");
        } else {
            sprintf(buf, "%d", req->usid_sel);
        }
        CPRINTF("Switch ID: %s\n", buf);
    }
}

static void cli_cmd_stack_mst_reelect(cli_req_t *req )
{
    vtss_rc rc;

    if (cli_cmd_standalone()) {
        return;
    }

    if ((rc = topo_parm_set(0, TOPO_PARM_MST_TIME_IGNORE, 1)) != VTSS_OK) {
        CPRINTF("%s\n", error_txt(rc));
    } else {
        CPRINTF("Master reelection active for the next %d seconds.\n",
                VTSS_SPROUT_MST_TIME_IGNORE_PERIOD);
    }
} 

static void cli_cmd_stack_mst_prio(cli_req_t *req )
{
    vtss_isid_t isid;
    vtss_rc rc;
    sprout_cli_req_t *sprout_req;

    if (cli_cmd_standalone()) {
        return;
    }

    sprout_req = req->module_req;

    if (sprout_req->local) {
        isid = 0;
    } else if (req->usid[0] != USID_UNSPECIFIED) {
        isid = topo_usid2isid(req->usid[0]);
    } else {
        return;
    }
    if ((rc = topo_parm_set(isid, TOPO_PARM_MST_ELECT_PRIO, req->int_values[0])) != VTSS_OK) {
        CPRINTF("%s\n", error_txt(rc));
    }
} 

static char *cli_sid_txt(vtss_usid_t sid, char *buf)
{
    if (sid == 0) {
        strcpy(buf, "-");
    } else {
        sprintf(buf, "%d", sid);
    }
    return buf;
}


static void cli_stack_stat(BOOL link, char *name, uint ca, uint cb)
{
    char buf[32];

    sprintf(buf, "%s:", name);
    CPRINTF("%-21s  ", buf);
    if (link) {
        CPRINTF("%-10s  %-10s", ca ? "Up" : "Down", cb ? "Up" : "Down");
    } else {
        CPRINTF("%-10u  %-10u", ca, cb);
    }
    CPRINTF("\n");
}


static void cli_cmd_debug_topo_config(cli_req_t *req )
{

    if (strlen(req->parm) == 0 || req->int_value_cnt == 0) {
        
        CPRINTF("Local Topo Configuration:\n");
        CPRINTF("\n");
        CPRINTF("mst_elect_prio            : %d (range is 1-4)\n",            topo_parm_get(TOPO_PARM_MST_ELECT_PRIO));
        CPRINTF("mst_time_ignore           : %d\n",                           topo_parm_get(TOPO_PARM_MST_TIME_IGNORE));
        CPRINTF("sprout_update_interval_slv: %d seconds\n",                   topo_parm_get(TOPO_PARM_SPROUT_UPDATE_INTERVAL_SLV));
        CPRINTF("sprout_update_interval_mst: %d seconds\n",                   topo_parm_get(TOPO_PARM_SPROUT_UPDATE_INTERVAL_MST));
        CPRINTF("sprout_update_age_time    : %d seconds\n",                   topo_parm_get(TOPO_PARM_SPROUT_UPDATE_AGE_TIME));
        CPRINTF("sprout_update_limit       : %d updates per second\n",        topo_parm_get(TOPO_PARM_SPROUT_UPDATE_LIMIT));
#if defined(VTSS_SPROUT_V1)
        CPRINTF("fast_mac_age_time         : %d seconds\n",                   topo_parm_get(TOPO_PARM_FAST_MAC_AGE_TIME));
        CPRINTF("fast_mac_age_count        : %d (unit: fast_mac_age_time)\n", topo_parm_get(TOPO_PARM_FAST_MAC_AGE_COUNT));
#endif
        CPRINTF("\n");
        CPRINTF("upsid[0][0]: %d\n", topo_parm_get(TOPO_PARM_UID_0_0) - 1);
        CPRINTF("upsid[0][1]: %d\n", topo_parm_get(TOPO_PARM_UID_0_1) - 1);
        CPRINTF("upsid[1][0]: %d\n", topo_parm_get(TOPO_PARM_UID_1_0) - 1);
        CPRINTF("upsid[1][1]: %d\n", topo_parm_get(TOPO_PARM_UID_1_1) - 1);
    } else if (strlen(req->parm) > 0 || req->int_value_cnt > 0) {
        
        int val;
        vtss_rc rc = VTSS_OK;
        vtss_isid_t isid;

        if (req->usid_sel != 0) {
            isid = topo_usid2isid(req->usid_sel);
        } else {
            isid = VTSS_ISID_GLOBAL;
        }

        val = req->int_values[0];

        if (strcmp(req->parm, "mst_elect_prio") == 0) {
            if (val < TOPO_PARM_MST_ELECT_PRIO_MIN ||
                val > TOPO_PARM_MST_ELECT_PRIO_MAX) {
                CPRINTF("Illegal mst_elect_prio value: %d\n", val);
            } else {
                rc = topo_parm_set(isid, TOPO_PARM_MST_ELECT_PRIO, val);
            }
        } else if (strcmp(req->parm, "mst_time_ignore") == 0) {
            if (val != 0 && val != 1) {
                CPRINTF("Illegal mst_time_ignore value: %d\n", val);
            } else {
                rc = topo_parm_set(isid, TOPO_PARM_MST_TIME_IGNORE, val);
            }
        } else if (strcmp(req->parm, "sprout_update_interval_slv") == 0) {
            rc = topo_parm_set(isid, TOPO_PARM_SPROUT_UPDATE_INTERVAL_SLV, val);
        } else if (strcmp(req->parm, "sprout_update_interval_mst") == 0) {
            rc = topo_parm_set(isid, TOPO_PARM_SPROUT_UPDATE_INTERVAL_MST, val);
        } else if (strcmp(req->parm, "sprout_update_age_time") == 0) {
            rc = topo_parm_set(isid, TOPO_PARM_SPROUT_UPDATE_AGE_TIME, val);
        } else if (strcmp(req->parm, "sprout_update_limit") == 0) {
            rc = topo_parm_set(isid, TOPO_PARM_SPROUT_UPDATE_LIMIT, val);
#if defined(VTSS_SPROUT_V1)
        } else if (strcmp(req->parm, "fast_mac_age_time") == 0) {
            rc = topo_parm_set(isid, TOPO_PARM_FAST_MAC_AGE_TIME, val);
        } else if (strcmp(req->parm, "fast_mac_age_count") == 0) {
            rc = topo_parm_set(isid, TOPO_PARM_FAST_MAC_AGE_COUNT, val);
#endif
        } else {
            CPRINTF("Unknown topo parameter: %s\n", req->parm);
        }
        if (rc != VTSS_OK) {
            CPRINTF("%s\n", error_txt(rc));
        }
    }
} 

static void cli_cmd_stack_list(cli_req_t *req, BOOL debug)
{
    topo_switch_stat_t            stat;
    topo_topology_type_t          type;
    topo_switch_list_t            *tsl_p;
    topo_switch_t                 *ts_p;
    vtss_isid_t                   isid;
    int                           len, i, chip_idx;
    char                          buf[200], *p;
    const char                    *s;
    port_info_t                   info;
    BOOL                          link_up_a;
    BOOL                          link_up_b;
    vtss_sprout_stack_port_stat_t *stat_0, *stat_1;
    sprout_cli_req_t              *sprout_req;
    int                           ports_str_len = strlen("Ports");
    int                           sp_idx[2];

    sprout_req = req->module_req;

    if (cli_cmd_standalone()) {
        return;
    }

    if (topo_switch_stat_get(VTSS_ISID_LOCAL, &stat) != VTSS_OK) {
        return;
    }

    if (!(tsl_p = VTSS_MALLOC(sizeof(topo_switch_list_t)))) {
        T_W("VTSS_MALLOC() failed, size=%zu", sizeof(topo_switch_list_t));
        return;
    }

    type = stat.topology_type;
    if (sprout_req->detailed) {
        CPRINTF("Stack Topology      : %s\n",
                type == TopoBack2Back ? "Back-to-Back" :
                type == TopoClosedLoop ? "Ring" :
                (type == TopoOpenLoop && stat.switch_cnt > 1) ? "Chain" :
                (type == TopoOpenLoop && stat.switch_cnt == 1) ? "Standalone" : "?");
        CPRINTF("Stack Member Count  : %d\n", stat.switch_cnt);
        CPRINTF("Last Topology Change: %s\n", misc_time2str(stat.topology_change_time));
    }

    if (topo_switch_list_get(tsl_p) == VTSS_OK) {
        
        if (port_no_stack(0) < port_no_stack(1)) {
            sp_idx[0] = 0;
            sp_idx[1] = 1;
        } else {
            sp_idx[0] = 1;
            sp_idx[1] = 0;
        }

        
        for (i = 0; i < ARRSZ(tsl_p->ts); i++) {
            if (!tsl_p->ts[i].vld) {
                break;
            }

            ts_p = &tsl_p->ts[i];
            ports_str_len =
                MAX(ports_str_len,
                    strlen(vtss_sprout_port_mask_to_str(ts_p->chip[0].ups_port_mask[0] |
                                                        ts_p->chip[0].ups_port_mask[1])));
            ports_str_len =
                MAX(ports_str_len,
                    strlen(vtss_sprout_port_mask_to_str(ts_p->chip[1].ups_port_mask[0] |
                                                        ts_p->chip[1].ups_port_mask[1])));
        }
        if (sprout_req->detailed) {
            CPRINTF("Master Switch       : %s\n",
                    misc_mac_txt(tsl_p->mst_switch_mac_addr, buf));
            CPRINTF("Last Master Change  : %s\n\n",
                    misc_time2str(tsl_p->mst_change_time));
        }

        
        p = &buf[0];
        p += sprintf(p, " %-17s  ",
                     "Stack Member");

        p += sprintf(p, "%s  ",
                     debug ? "USID  ISID  UPSIDs" : "SID");

        if (sprout_req->productinfo) {
            sprintf(p, "Product Name & Firmware Version");
        } else {
            if (debug) {
                p += sprintf(p, "%-15s  ", "IP Address");
            }
            len = strlen(buf);
            for (i = 0; i < len; i++) {
                cli_putchar(' ');
            }

            CPRINTF(        "%*s  ", ports_str_len, "");
            p += sprintf(p, "%-*s  ", ports_str_len, "Ports");

            if (sprout_req->detailed) {
                CPRINTF("Distance          ");
                p += sprintf(p, "Port %2u  Port %2u  ", iport2uport(port_no_stack(sp_idx[0])), iport2uport(port_no_stack(sp_idx[1])));
            }

            CPRINTF("Forwarding        ");
            p += sprintf(p, "Port %2u  Port %2u  ", iport2uport(port_no_stack(sp_idx[0])), iport2uport(port_no_stack(sp_idx[1])));

            CPRINTF(        "Master               ");
            p += sprintf(p, "Prio  Time           ");

            CPRINTF("\n");
            if (sprout_req->detailed) {
                p += sprintf(p, "Reelect");
            }
        }
        cli_table_header(buf);

        for (i = 0; i < ARRSZ(tsl_p->ts); i++) {
            if (!tsl_p->ts[i].vld) {
                break;
            }

            ts_p = &tsl_p->ts[i];

            
            for (chip_idx = 0; chip_idx < ts_p->chip_cnt; chip_idx++) {
                p = &buf[0];

                if (sprout_req->productinfo && chip_idx != 0) {
                    continue;
                }

                
                if (chip_idx == 0) {
                    p += sprintf(p, "%s%s  ",
                                 ts_p->me ? "*" : " ",
                                 misc_mac_txt(ts_p->mac_addr, &buf[100]));
                } else {
                    p += sprintf(p, " ");
                    p += sprintf(p, "%*s  ", sizeof("11-22-33-44-55-66") - 1, "");
                }

                
                isid = (ts_p->usid == 0 ? 0 : topo_usid2isid(ts_p->usid));
                if (debug) {
                    if (chip_idx == 0) {
                        p += sprintf(p, "%-4s  %-4s  ",
                                     cli_sid_txt(ts_p->usid, &buf[100]),
                                     cli_sid_txt(isid, &buf[150]));
                    } else {
                        p += sprintf(p, "%-4s  %-4s  ", "", "");
                    }
                } else {
                    if (chip_idx == 0) {
                        p += sprintf(p, "%-3s  ", cli_sid_txt(ts_p->usid, &buf[150]));
                    } else {
                        p += sprintf(p, "%*s  ", 3, "");
                    }
                }

                if (!ts_p->present) {
                    sprintf(p, "Currently not present in stack\n");
                    CPRINTF(buf);
                    continue;
                }

                if (debug) {
                    if (ts_p->chip[chip_idx].ups_cnt == 2) {
                        p += sprintf(p, "%-2d, %2d  ",
                                     ts_p->chip[chip_idx].upsid[0],
                                     ts_p->chip[chip_idx].upsid[1]);
                    } else {
                        
                        p += sprintf(p, "%-2d      ", ts_p->chip[chip_idx].upsid[0]);
                    }
                }

                if (sprout_req->productinfo) {
                    CPRINTF(buf);
                    if (req->stack.master) {
                        
                        
                        char prod_name[MSG_MAX_PRODUCT_NAME_LEN] = "N/A";
                        char ver_str[MSG_MAX_VERSION_STRING_LEN] = "N/A";

                        if (VTSS_ISID_LEGAL(isid)) {
                            msg_product_name_get(isid, prod_name);
                            msg_version_string_get(isid, ver_str);
                        }
                        CPRINTF("%s, %s\n", prod_name, ver_str);
                    } else {
                        CPRINTF("Information only available on master\n");
                    }
                    continue;
                }

                if (debug) {
                    if (chip_idx == 0) {
                        p += sprintf(p, "%-15s  ", misc_htoa(ts_p->ip_addr));
                    } else {
                        p += sprintf(p, "%-15s  ", "");
                    }
                }

                
                p += sprintf(p, "%-*s  ", ports_str_len,
                             vtss_sprout_port_mask_to_str(ts_p->chip[chip_idx].ups_port_mask[0] |
                                                          ts_p->chip[chip_idx].ups_port_mask[1]));

                if (sprout_req->detailed) {
                    
                    p += sprintf(p, "%-7s  ", ts_p->chip[chip_idx].dist_str[sp_idx[0]]);
                    p += sprintf(p, "%-7s  ", ts_p->chip[chip_idx].dist_str[sp_idx[1]]);
                }

                
                p += sprintf(p, "%-7s  %-7s  ",
                             topo_stack_port_fwd_mode_to_str(ts_p->chip[chip_idx].stack_port_fwd_mode[sp_idx[0]]),
                             topo_stack_port_fwd_mode_to_str(ts_p->chip[chip_idx].stack_port_fwd_mode[sp_idx[1]]));

                
                if (chip_idx == 0) {
                    if (ts_p->mst_capable) {
                        p += sprintf(p, "%-4d", ts_p->mst_elect_prio);
                    } else {
                        p += sprintf(p, "%-4s", "-");
                    }

                    s = (ts_p->mst_time > 0 ? cli_time_txt(ts_p->mst_time) : "-");
                    p += sprintf(p, "  %-13s", s);

                    if (sprout_req->detailed) {
                        p += sprintf(p, "  %d", ts_p->mst_time_ignore);
                    }
                } else {
                    p += sprintf(p, "%*s", 4 + 2 + 13 + 2 + 1, "");
                }
                p += sprintf(p, "\n");
                CPRINTF(buf);
            }
        }
    }

    if (sprout_req->detailed) {
        CPRINTF("\n");
        sprintf(buf, "%-20s   Port %2u     Port %2u     ",
                "", iport2uport(PORT_NO_STACK_0), iport2uport(PORT_NO_STACK_1));
        cli_table_header(buf);
        stat_0 = &stat.stack_port[0];
        stat_1 = &stat.stack_port[1];
        link_up_a = (port_info_get(PORT_NO_STACK_0, &info) == VTSS_OK ? info.link : 0);
        link_up_b = (port_info_get(PORT_NO_STACK_1, &info) == VTSS_OK ? info.link : 0);
        cli_stack_stat(1, "Link State",
                       link_up_a, link_up_b);
        cli_stack_stat(1, "SPROUT State",
                       stat_0->proto_up, stat_1->proto_up);
        CPRINTF("SPROUT Update Counters:\n");
        cli_stack_stat(0, "  Rx PDUs",
                       stat_0->sprout_update_rx_cnt, stat_1->sprout_update_rx_cnt);
        cli_stack_stat(0, "  Tx Periodic PDUs",
                       stat_0->sprout_update_periodic_tx_cnt, stat_1->sprout_update_periodic_tx_cnt);
        cli_stack_stat(0, "  Tx Triggered PDUs",
                       stat_0->sprout_update_triggered_tx_cnt, stat_1->sprout_update_triggered_tx_cnt);
        cli_stack_stat(0, "  Tx Policer Drops",
                       stat_0->sprout_update_tx_policer_drop_cnt, stat_1->sprout_update_tx_policer_drop_cnt);
        cli_stack_stat(0, "  Rx Errors",
                       stat_0->sprout_update_rx_err_cnt, stat_1->sprout_update_rx_err_cnt);
        cli_stack_stat(0, "  Tx Errors",
                       stat_0->sprout_update_tx_err_cnt, stat_1->sprout_update_tx_err_cnt);
#if defined(VTSS_SPROUT_V2)
        CPRINTF("SPROUT Alert Counters:\n");
        cli_stack_stat(0, "  Rx PDUs",
                       stat_0->sprout_alert_rx_cnt, stat_1->sprout_alert_rx_cnt);
        cli_stack_stat(0, "  Tx PDUs",
                       stat_0->sprout_alert_tx_cnt, stat_1->sprout_alert_tx_cnt);
        cli_stack_stat(0, "  Tx Policer Drops",
                       stat_0->sprout_alert_tx_policer_drop_cnt, stat_1->sprout_alert_tx_policer_drop_cnt);
        cli_stack_stat(0, "  Rx Errors",
                       stat_0->sprout_alert_rx_err_cnt, stat_1->sprout_alert_rx_err_cnt);
        cli_stack_stat(0, "  Tx Errors",
                       stat_0->sprout_alert_tx_err_cnt, stat_1->sprout_alert_tx_err_cnt);
#endif
    }

    VTSS_FREE(tsl_p);
}
static void cli_cmd_debug_stack_list(cli_req_t *req )
{
    cli_cmd_stack_list(req, 1);
}
static void cli_cmd_stack_list_list(cli_req_t *req )
{
    cli_cmd_stack_list(req, 0);
}


static void cli_cmd_debug_topo_default(cli_req_t *req )
{
    topo_parm_set_default();
    CPRINTF("Topo configuration (both local and global parameters) set to default values. YOU MUST REBOOT NOW.\n");
} 

static void cli_cmd_debug_topo_sid(cli_req_t *reg )
{
    topo_dbg_isid_tbl_print(cli_printf);
} 

static void cli_cmd_debug_topo_stat(cli_req_t *req )
{
    vtss_isid_t isid;
    topo_switch_stat_t switch_stat;
    int i;

    isid = topo_usid2isid(req->usid[0]);
    (void)topo_switch_stat_get(isid, &switch_stat);

    CPRINTF("Topo switch stat for usid=%d (isid=%d)\n", req->usid[0], isid);
    CPRINTF("Switch count: %d\n", switch_stat.switch_cnt);
    for (i = 0; i < 2; i++) {
        CPRINTF("Stack port %d: proto_up: %d\n",
                i, switch_stat.stack_port[i].proto_up);
    }
} 

#if defined(VTSS_SPROUT_FW_VER_CHK)
static void cli_cmd_debug_topo_fw_ver_mode(cli_req_t *req )
{
    vtss_isid_t isid;
    int val;

    if (req->int_value_cnt == 0) {
        CPRINTF("Local Firmware Version Mode:\n");
        CPRINTF("fw_ver_mode               : %s\n",
                (topo_parm_get(TOPO_PARM_FW_VER_MODE) == VTSS_SPROUT_FW_VER_MODE_NORMAL) ?
                "normal mode" : "null mode");
    } else {
        if (req->usid_sel != 0) {
            isid = topo_usid2isid(req->usid_sel);
        } else {
            isid = VTSS_ISID_GLOBAL;
        }

        val = req->int_values[0];

        if (val == VTSS_SPROUT_FW_VER_MODE_NULL) {
            CPRINTF("Warning: Now accepting ANY firmware versions of neighbors. This is NOT recommended and may cause system crash.\n");
        }
        topo_parm_set(isid, TOPO_PARM_FW_VER_MODE, val);
    }
} 

static void cli_cmd_debug_topo_cmef(cli_req_t *req )
{
    vtss_isid_t isid;

    if (!req->set) {
#ifdef VTSS_FEATURE_VSTAX_V2
        vtss_vstax_conf_t  vstax_conf;
        vtss_rc            rc = VTSS_OK;

        if ((rc = vtss_vstax_conf_get(NULL, &vstax_conf)) != VTSS_OK) {
            CPRINTF("%s\n", error_txt(rc));
            return;
        }

        CPRINTF("CMEF %sabled on local switch\n", vstax_conf.cmef_disable ? "Dis" : "En");
#else
        CPRINTF("CMEF is not supported in this configuration\n");
#endif  
    } else {
        if (req->usid_sel != 0) {
            isid = topo_usid2isid(req->usid_sel);
        } else {
            isid = VTSS_ISID_GLOBAL;
        }

        topo_parm_set(isid, TOPO_PARM_CMEF_MODE, req->disable ? 0 : 1);
    }
} 
#endif

static void cli_cmd_debug_topo_test(cli_req_t *req )
{
    if (req->int_value_cnt == 0) {
        topo_dbg_test(cli_printf, 0, 0, 0);
    } else if (req->int_value_cnt == 1) {
        topo_dbg_test(cli_printf, req->int_values[0], 0, 0);
    } else if (req->int_value_cnt >= 2) {
        topo_dbg_test(cli_printf, req->int_values[0], req->int_values[1], 0);
    } else if (req->int_value_cnt >= 3) {
        topo_dbg_test(cli_printf, req->int_values[0], req->int_values[1], req->int_values[2]);
    }
} 


static void cli_cmd_debug_sprout(cli_req_t *req )
{
    ulong parms[CLI_INT_VALUES_MAX];
    int i;

    for (i = 0; i < CLI_INT_VALUES_MAX; i++) {
        parms[i] = req->int_values[i];
    }

    topo_dbg_sprout(cli_printf, req->int_value_cnt, parms);
} 

static int32_t cli_sprout_generic_keyword_parse(char *cmd, cli_req_t *req, char *stx)
{
    char *found = cli_parse_find(cmd, stx);
    sprout_cli_req_t *sprout_req;

    T_I("ALL %s", found);

    sprout_req = req->module_req;

    if (found != NULL) {
        if (!strncmp(found, "productinfo", 11)) {
            sprout_req->productinfo = 1;
        } else if (!strncmp(found, "detailed", 8)) {
            sprout_req->detailed = 1;
        } else if (!strncmp(found, "local", 5)) {
            sprout_req->local = 1;
        }
    }
    return (found == NULL ? 1 : 0);
}

static int32_t cli_sprout_keyword_parse(char *cmd, char *cmd2, char *stx,
                                        char *cmd_org, cli_req_t *req)
{
    return cli_sprout_generic_keyword_parse(cmd, req, stx);
}

static int32_t cli_sprout_sid_local_parse(char *cmd, char *cmd2, char *stx,
                                          char *cmd_org, cli_req_t *req)
{
    int32_t error;
    ulong value = 0;

    error = (cli_parse_ulong(cmd, &value, VTSS_USID_START, VTSS_USID_END - 1) &&
             cli_sprout_keyword_parse(cmd, cmd2, stx, cmd_org, req));

    req->usid[0] = value;

    return error;
}

static int32_t cli_sprout_sid_all_parse(char *cmd, char *cmd2, char *stx,
                                        char *cmd_org, cli_req_t *req)
{
    int32_t error;
    ulong value = 0;

    error = (cli_parse_ulong(cmd, &value, VTSS_USID_START, VTSS_USID_END - 1) &&
             cli_parm_parse_keyword (cmd, cmd2, stx, cmd_org, req));
    req->usid_sel = value;

    return error;
}

static int32_t cli_sprout_mst_elect_prio_parse(char *cmd, char *cmd2, char *stx,
                                               char *cmd_org, cli_req_t *req)
{
    int32_t error;
    ulong ul;

    error = cli_parse_ulong(cmd, &ul, TOPO_PARM_MST_ELECT_PRIO_MIN, TOPO_PARM_MST_ELECT_PRIO_MAX);
    req->int_values[0] = (int)ul;

    return error;
}

static int32_t cli_sprout_topo_parse(char *cmd, char *cmd2, char *stx,
                                     char *cmd_org, cli_req_t *req)
{
    return cli_parse_raw(cmd, req);
}

static int32_t cli_sprout_integer_parse(char *cmd, char *cmd2, char *stx,
                                        char *cmd_org, cli_req_t *req)
{
    return cli_parse_integer(cmd, req, stx);
}

static int32_t cli_sprout_stack_enable_disable_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    sprout_cli_req_t *sprout_req = req->module_req;
    int32_t error = cli_parm_parse_keyword(cmd, cmd2, stx, cmd_org, req);
    if (!error) {
        sprout_req->stacking_specified = TRUE;
    }
    return error;
}

static int32_t cli_sprout_stackports_parse(char *cmd, char *cmd2, char *stx,
                                           char *cmd_org, cli_req_t *req)
{
    int32_t error = 1;

    if (sscanf(cmd, "%d,%d", &req->int_values[0], &req->int_values[1]) == 2 ||
        sscanf(cmd, "%d-%d", &req->int_values[0], &req->int_values[1]) == 2) {
        if (req->int_values[0] >= 1 && req->int_values[0] <= VTSS_PORTS &&
            req->int_values[1] >= 1 && req->int_values[1] <= VTSS_PORTS &&
            req->int_values[0] < req->int_values[1] ) {
            req->port = TRUE, error = 0;
        }
    }

    return error;
}

static cli_parm_t sprout_cli_parm_table[] = {
    {
        "detailed|productinfo",
        "Show product information",
        CLI_PARM_FLAG_NONE,
        cli_sprout_keyword_parse,
        cli_cmd_stack_list_list
    },
    {
        "<sid>",
        SPROUT_sid_swap_str, 
        CLI_PARM_FLAG_SET,
        cli_parm_parse_sid,
        cli_cmd_stack_sid_swap
    },
    {
        "<sid>|local",
        SPROUT_sid_mst_prio_str, 
        CLI_PARM_FLAG_SET,
        cli_sprout_sid_local_parse,
        cli_cmd_stack_mst_prio
    },
    {
        "<sid>|all",
        SPROUT_sid_select_str, 
        CLI_PARM_FLAG_SET,
        cli_sprout_sid_all_parse,
        cli_cmd_stack_select
    },
    {
        "<mst_elect_prio>",
        "Master election priority: 1-4. 1 => Highest master probability",
        CLI_PARM_FLAG_SET,
        cli_sprout_mst_elect_prio_parse,
        cli_cmd_stack_mst_prio
    },
    {
        "<topo_parameter>",
        "mst_elect_prio            : Master election priority.\n"
        "                            1 => Highest probability for becoming master.\n"
        "mst_time_ignore           : Master time ignore.\n"
        "                            0 => Master time considered. 1 => Master time ignored\n"
        "sprout_update_limit       : Max number of SPROUT updates transmitted per second.\n"
        "sprout_update_interval_slv: SPROUT update interval when slave (periodic updates). Seconds.\n"
        "sprout_update_interval_mst: SPROUT update interval when master (periodic updates). Seconds.\n"
        "sprout_update_age_time    : Stack link considered ProtoDown if no SPROUT Updates\n"
        "                            received for this time. Seconds.\n"
#if defined(VTSS_SPROUT_V1)
        "fast_mac_age_time         : Age time to be used after topology change. Seconds.\n"
        "fast_mac_age_count        : Time during which fast aging shall be done.\n"
        "                            Specified as multiples of fast_mac_age_time.\n"
#endif
        ,
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_sprout_topo_parse,
        cli_cmd_debug_topo_config
    },
    {
        "<integer>",
        "Parameter for sprout debug",
        CLI_PARM_FLAG_NONE,
        cli_sprout_integer_parse,
        cli_cmd_debug_sprout
    },
#if defined(VTSS_SPROUT_FW_VER_CHK)
    {
        "<fw_ver_mode>",
        "\n1: Normal mode, i.e. require identical firmware version in stack.\n"
        "0: Null mode, i.e. accept neighbors with any firmware versions.\n",
        CLI_PARM_FLAG_NONE,
        cli_sprout_integer_parse,
        cli_cmd_debug_topo_fw_ver_mode
    },
    {
        "enable|disable",
        "enable : Enable CMEF\n"
        "disable: Disable CMEF",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        cli_cmd_debug_topo_cmef
    },
#endif
    {
        "enable|disable",
        "enable       : Enable stack ports\n"
        "disable      : Disable stack ports",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_sprout_stack_enable_disable_parse,
        cli_cmd_stack_config
    },
    {
        "<stack_ports>",
        "Two stack capable ports must be specified. Format: 'X,Y' or 'X-Y'",
        CLI_PARM_FLAG_SET,
        cli_sprout_stackports_parse,
        cli_cmd_stack_config
    },
    {
        "detailed",
        "Show detailed information",
        CLI_PARM_FLAG_NONE,
        cli_sprout_keyword_parse,
        NULL
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
    PRIO_STACK_LIST,
    PRIO_STACK_MST_ELECT_PRIO,
    PRIO_STACK_MST_REELECT,
    PRIO_STACK_SELECT,
    PRIO_STACK_SID_SWAP,
    PRIO_STACK_SID_DELETE,
    PRIO_STACK_SID_ASSIGN,
    PRIO_STACK_CONFIG,
    PRIO_DEBUG_TOPO_CONFIG = CLI_CMD_SORT_KEY_DEFAULT,
    PRIO_DEBUG_STACK_LIST = CLI_CMD_SORT_KEY_DEFAULT,
    PRIO_DEBUG_TOPO_DEFAULT = CLI_CMD_SORT_KEY_DEFAULT,
    PRIO_DEBUG_TOPO_SID = CLI_CMD_SORT_KEY_DEFAULT,
    PRIO_DEBUG_TOPO_STAT = CLI_CMD_SORT_KEY_DEFAULT,
    PRIO_DEBUG_TOPO_FW_VER_MODE = CLI_CMD_SORT_KEY_DEFAULT,
    PRIO_DEBUG_TOPO_CMEF = CLI_CMD_SORT_KEY_DEFAULT,
    PRIO_DEBUG_TOPO_TEST = CLI_CMD_SORT_KEY_DEFAULT,
    PRIO_DEBUG_SPROUT = CLI_CMD_SORT_KEY_DEFAULT,
};

cli_cmd_tab_entry (
    "Debug Stack List [detailed]",
    NULL,
    "Show list of switches in stack with additional Topo debug info",
    PRIO_DEBUG_STACK_LIST,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_stack_list,
    NULL,
    sprout_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "Debug Topo Configuration",
    "Debug Topo Configuration [<topo_parameter>] [<integer>]",
    "Set or show Topo configuration parameters.",
    PRIO_DEBUG_TOPO_CONFIG,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_topo_config,
    NULL,
    sprout_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    NULL,
    "Debug Topo Default",
    "Set all Topo configuration parameters to default",
    PRIO_DEBUG_TOPO_DEFAULT,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_topo_default,
    NULL,
    sprout_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "Debug Topo SID",
    NULL,
    "Show SID assignments for all switches known to Topo",
    PRIO_DEBUG_TOPO_SID,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_topo_sid,
    NULL,
    sprout_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "Debug Topo Stat <sid>",
    NULL,
    "Show Topo Stat for (remote) switch",
    PRIO_DEBUG_TOPO_STAT,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_topo_stat,
    NULL,
    sprout_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

#if defined(VTSS_SPROUT_FW_VER_CHK)
cli_cmd_tab_entry (
    NULL,
    "Debug Topo FwVerMode [<fw_ver_mode>]",
    "Set firmware version mode",
    PRIO_DEBUG_TOPO_FW_VER_MODE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_TOPO,
    cli_cmd_debug_topo_fw_ver_mode,
    NULL,
    sprout_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    NULL,
    "Debug Topo CMEF [enable|disable]",
    "Enable/disable CMEF or show mode on local switch",
    PRIO_DEBUG_TOPO_CMEF,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_TOPO,
    cli_cmd_debug_topo_cmef,
    NULL,
    sprout_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif

cli_cmd_tab_entry (
    NULL,
    "Debug Topo Test [<integer>] [<integer>] [<integer>]",
    "Test command",
    PRIO_DEBUG_TOPO_TEST,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_TOPO,
    cli_cmd_debug_topo_test,
    NULL,
    sprout_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    NULL,
    "Debug Sprout [<integer>] [<integer>] [<integer>]\n"
    "             [<integer>] [<integer>] [<integer>]",
    "Debug SPROUT",
    PRIO_DEBUG_SPROUT,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_sprout,
    NULL,
    sprout_cli_parm_table,
    CLI_CMD_FLAG_NONE
);


cli_cmd_tab_entry (
    "Stack List [detailed|productinfo]",
    NULL,
    "Show the list of switches in stack",
    PRIO_STACK_LIST,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_TOPO,
    cli_cmd_stack_list_list,
    NULL,
    sprout_cli_parm_table,
    CLI_CMD_FLAG_SYS_CONF
);

cli_cmd_tab_entry (
    NULL,
    "Stack Master Priority <sid>|local <mst_elect_prio>",
    "Set the master election priority",
    PRIO_STACK_MST_ELECT_PRIO,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_TOPO,
    cli_cmd_stack_mst_prio,
    NULL,
    sprout_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    NULL,
    "Stack Master Reelect",
    "Force master reelection (ignoring master time)",
    PRIO_STACK_MST_REELECT,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_TOPO,
    cli_cmd_stack_mst_reelect,
    NULL,
    sprout_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "Stack Select",
    "Stack Select [<sid>|all]",
    "Set or show the selected switch ID",
    PRIO_STACK_SELECT,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_TOPO,
    cli_cmd_stack_select,
    NULL,
    sprout_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    NULL,
    "Stack SID Swap <sid> <sid>",
    "Swap SID values used to identify two switches",
    PRIO_STACK_SID_SWAP,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_TOPO,
    cli_cmd_stack_sid_swap,
    NULL,
    sprout_cli_parm_table,
    CLI_CMD_FLAG_NONE
);


cli_cmd_tab_entry (
    NULL,
    "Stack SID Delete <sid>",
    "Delete SID assignment and associated configuration",
    PRIO_STACK_SID_DELETE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_TOPO,
    cli_cmd_stack_sid_delete,
    NULL,
    sprout_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    NULL,
    "Stack SID Assign <sid> <mac_addr>",
    "Assign SID and associated configuration to switch."
    "\nSID must be unassigned, switch must be present and switch must not already be assigned to a SID",
    PRIO_STACK_SID_ASSIGN,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_TOPO,
    cli_cmd_stack_sid_assign,
    NULL,
    sprout_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    NULL,
    "Stack Config [enable|disable] [<stack_ports>]",
    "Configure stack ports to use."
    "\nPorts must be stack capable",
    PRIO_STACK_CONFIG,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_TOPO,
    cli_cmd_stack_config,
    NULL,
    sprout_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

void sprout_cli_req_init(void)
{
    SPROUT_create_parser_strings();

    
    cli_req_size_register(sizeof(sprout_cli_req_t));
}







