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
#include "lldp_cli.h"
#include "vtss_module_id.h"
#include "lldp_api.h"
#include "lldp_os.h"
#include "lldp_remote.h"
#include "cli_trace_def.h"

static void cli_cmd_lldp_conf(cli_req_t *req, BOOL mode, BOOL opt_tlvs, BOOL interval,
                              BOOL hold, BOOL delay, BOOL reinit,   BOOL info,  BOOL stats,
                              BOOL is_cdp_aware);

typedef struct {
    ushort lldp;
    BOOL   port_description;
    BOOL   system_name;
    BOOL   system_description;
    BOOL   system_capabilities;
    BOOL   management_addr;

    /* Keywords */
    BOOL   rx;
    BOOL   tx;
} lldp_cli_req_t;

void lldp_cli_req_init(void)
{
    /* register the size required for lldp req. structure */
    cli_req_size_register(sizeof(lldp_cli_req_t));
}

static void cli_cmd_lldp_config ( cli_req_t *req )
{
    if (!req->set) {
        cli_header("LLDP Configuration", 1);
    }
    cli_cmd_lldp_conf(req, 1, 1, 1, 1, 1, 1, 0, 0, 1);
}

static void cli_cmd_lldp_mode ( cli_req_t *req )
{
    cli_cmd_lldp_conf(req, 1, 0, 0, 0, 0, 0, 0, 0, 0);
}
static void cli_cmd_lldp_opt_tlv ( cli_req_t *req )
{
    cli_cmd_lldp_conf(req, 0, 1, 0, 0, 0, 0, 0, 0, 0);
}

static void cli_cmd_lldp_lldp_interval ( cli_req_t *req )
{
    cli_cmd_lldp_conf(req, 0, 0, 1, 0, 0, 0, 0, 0, 0);
}

static void cli_cmd_lldp_lldp_hold ( cli_req_t *req )
{
    cli_cmd_lldp_conf(req, 0, 0, 0, 1, 0, 0, 0, 0, 0);
}

static void cli_cmd_lldp_lldp_delay ( cli_req_t *req )
{
    cli_cmd_lldp_conf(req, 0, 0, 0, 0, 1, 0, 0, 0, 0);
}

static void cli_cmd_lldp_lldp_reinit ( cli_req_t *req )
{
    cli_cmd_lldp_conf(req, 0, 0, 0, 0, 0, 1, 0, 0, 0);
}

static void cli_cmd_lldp_lldp_info ( cli_req_t *req )
{
    cli_cmd_lldp_conf(req, 0, 0, 0, 0, 0, 0, 1, 0, 0);
}

static void cli_cmd_lldp_lldp_stat ( cli_req_t *req )
{
    cli_cmd_lldp_conf(req, 0, 0, 0, 0, 0, 0, 0, 1, 0);
}

#ifdef VTSS_SW_OPTION_CDP
static void cli_cmd_lldp_lldp_cdp_aware ( cli_req_t *req )
{
    cli_cmd_lldp_conf(req, 0, 0, 0, 0, 0, 0, 0, 0, 1);
}
#endif /* VTSS_SW_OPTION_CDP */

static void cli_lldp_tlv(char *name, lldp_remote_entry_t *entry, lldp_tlv_t field, u8 mgmt_addr_index)
{
    lldp_8_t output_string[512];
    CPRINTF("%-20s: ", name);
    lldp_remote_tlv_to_string(entry, field, &output_string[0], mgmt_addr_index);
    CPRINTF("%s\n", output_string);
}

#ifdef VTSS_SW_OPTION_CDP_BIST
// Calling CDP Build In Self Test
static void cli_cdp_bist (cli_req_t *req)
{
    vtss_usid_t    usid;
    vtss_isid_t    isid;
    BOOL           iport_list[VTSS_PORT_ARRAY_SIZE];
    vtss_port_no_t iport;

    // Gotta convert it to a list of iports.
    for (iport = VTSS_PORT_NO_START; iport < VTSS_PORT_NO_END; iport++) {
        iport_list[iport] = req->uport_list[iport2uport(iport)];
    }

    for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
        if ((isid = req->stack.isid[usid]) == VTSS_ISID_END) {
            continue;
        }
        CPRINTF("%s \n", cdp_bist(iport_list, isid));
    }
}
#endif // VTSS_SW_OPTION_CDP_BIST

static void cli_cmd_lldp_conf(cli_req_t *req, BOOL mode, BOOL opt_tlvs, BOOL interval,
                              BOOL hold, BOOL delay, BOOL reinit,   BOOL info,  BOOL stats,
                              BOOL is_cdp_aware)
{
    vtss_usid_t         usid;
    vtss_isid_t         isid;
    BOOL                first;
    lldp_admin_state_t  *state;
    lldp_remote_entry_t *table = NULL, *entry = NULL;
    lldp_counters_rec_t stat_table[LLDP_PORTS], *stat;
    int                 i, count = 0;
    char                lldp_no_entry_found = 1;
    char                buf[255], *p;
    lldp_cli_req_t       *lldp_req;
    vtss_rc             rc = VTSS_OK ;
    BOOL                all_opt_parameter = FALSE;
    u8                  mgmt_addr_index;
    port_iter_t         pit;

    // We don't support setting configuration from slave
    if (cli_cmd_slave_do_not_set(req)) {
        return;
    }

    lldp_req = req->module_req;
    if (interval || hold || delay || reinit) {
        lldp_common_conf_t lldp_common_conf;
        lldp_mgmt_get_common_config(&lldp_common_conf);
        if (req->set) {
            if (interval) {
                lldp_common_conf.msgTxInterval = lldp_req->lldp;
            }
            if (hold) {
                lldp_common_conf.msgTxHold = lldp_req->lldp;
            }
            if (delay) {
                lldp_common_conf.txDelay = lldp_req->lldp;
            }
            if (reinit) {
                lldp_common_conf.reInitDelay = lldp_req->lldp;
            }

            if ((rc = lldp_mgmt_set_common_config(&lldp_common_conf)) != VTSS_OK) {
                CPRINTF("%s\n", error_txt(rc));
                return;
            }
        } else {
            if (interval) {
                CPRINTF("Interval    : %d\n", lldp_common_conf.msgTxInterval);
            }
            if (hold) {
                CPRINTF("Hold        : %d\n", lldp_common_conf.msgTxHold);
            }
            if (delay) {
                CPRINTF("Tx Delay    : %d\n", lldp_common_conf.txDelay);
            }
            if (reinit) {
                CPRINTF("Reinit Delay: %d\n", lldp_common_conf.reInitDelay);
            }
        }
    }

    for (usid = VTSS_USID_START; (mode || info || stats || opt_tlvs || is_cdp_aware) && usid < VTSS_USID_END; usid++) {
        if ((isid = req->stack.isid[usid]) == VTSS_ISID_END) {
            continue;
        }
        T_D("Got a correct isid : %d", isid);

        lldp_struc_0_t      conf;
        lldp_mgmt_get_config(&conf, isid);

        if (info) {
            lldp_mgmt_get_lock();

            count = lldp_remote_get_max_entries();
            table = lldp_mgmt_get_entries(isid); // Get the LLDP entries for the switch in question.
        }

        // Get or clear statistic counters.
        if (stats) {
            if (req->clear) {
                lldp_mgmt_stat_clr(isid);
                continue;
            }
            if (lldp_mgmt_stat_get(isid, &stat_table[0], NULL, NULL)) {
                T_W("Problem getting counters");
            }
        }

        first = 1;

        // Loop through all front ports
        if (port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT) == VTSS_OK) {
            while (port_iter_getnext(&pit)) {
                if (req->uport_list[pit.uport] == 0) {
                    continue;
                }

                state = &conf.admin_state[pit.iport];

                if (req->set) {
                    T_DG(VTSS_TRACE_GRP_LLDP, "Setting LLDP configuration");
                    if (mode) {
                        *state  =
                            (req->enable ? LLDP_ENABLED_RX_TX : lldp_req->tx ? LLDP_ENABLED_TX_ONLY :
                             lldp_req->rx ? LLDP_ENABLED_RX_ONLY : LLDP_DISABLED);
                    }


                    if (lldp_req->port_description)
                        lldp_os_set_optional_tlv(LLDP_TLV_BASIC_MGMT_PORT_DESCR,
                                                 req->enable, &conf, pit.iport);

                    if (lldp_req->system_name)
                        lldp_os_set_optional_tlv(LLDP_TLV_BASIC_MGMT_SYSTEM_NAME,
                                                 req->enable, &conf, pit.iport);

                    if (lldp_req->system_description)
                        lldp_os_set_optional_tlv(LLDP_TLV_BASIC_MGMT_SYSTEM_DESCR,
                                                 req->enable, &conf, pit.iport);

                    if (lldp_req->system_capabilities)
                        lldp_os_set_optional_tlv(LLDP_TLV_BASIC_MGMT_SYSTEM_CAPA,
                                                 req->enable, &conf, pit.iport);

                    if (lldp_req->management_addr)
                        lldp_os_set_optional_tlv(LLDP_TLV_BASIC_MGMT_MGMT_ADDR,
                                                 req->enable, &conf, pit.iport);
#ifdef VTSS_SW_OPTION_CDP
                    if (is_cdp_aware) {
                        T_D("Setting CDP awareness");
                        conf.cdp_aware[pit.iport] = req->enable;
                    }
#endif
                    continue;
                }

                // If no of the option parameter is select then we print them all
                all_opt_parameter = !(lldp_req->port_description | lldp_req->system_name | lldp_req->system_description | lldp_req->system_capabilities | lldp_req->management_addr);

                if (mode || opt_tlvs || is_cdp_aware) {
                    if (first) {
                        T_D("CDP awareness");
                        first = 0;
                        cli_cmd_usid_print(usid, req, 1);
                        p = &buf[0];
                        p += sprintf(p, "Port  ");
                        if (mode) {
                            p += sprintf(p, "Mode      ");
                        }


                        if (opt_tlvs) {
                            if (lldp_req->port_description || all_opt_parameter) {
                                p += sprintf(p, "Port Descr  ");
                            }

                            if (lldp_req->system_name || all_opt_parameter) {
                                p += sprintf(p, "System Name  ");
                            }

                            if (lldp_req->system_description || all_opt_parameter) {
                                p += sprintf(p, "System Descr  ");
                            }

                            if (lldp_req->system_capabilities || all_opt_parameter) {
                                p += sprintf(p, "System Capa  ");
                            }

                            if (lldp_req->management_addr || all_opt_parameter) {
                                p += sprintf(p, "Mgmt Addr  ");
                            }
                        }

#ifdef VTSS_SW_OPTION_CDP
                        if (is_cdp_aware) {
                            p += sprintf(p, "CDP awareness");
                        }
#endif

                        cli_table_header(buf);
                    }
                    CPRINTF("%-6u", pit.uport);
                    if (mode)
                        CPRINTF("%-8s  ",
                                *state == LLDP_ENABLED_RX_TX ? "Enabled " :
                                *state == LLDP_ENABLED_TX_ONLY ? "Tx" :
                                *state == LLDP_ENABLED_RX_ONLY ? "Rx" : "Disabled");

                    if (opt_tlvs) {
                        if (lldp_req->port_description || all_opt_parameter) {
                            CPRINTF("%-11s ", cli_bool_txt(lldp_mgmt_get_opt_tlv_enabled(LLDP_TLV_BASIC_MGMT_PORT_DESCR, pit.iport, isid)));
                        }

                        if (lldp_req->system_name || all_opt_parameter) {
                            CPRINTF("%-12s ", cli_bool_txt(lldp_mgmt_get_opt_tlv_enabled(LLDP_TLV_BASIC_MGMT_SYSTEM_NAME, pit.iport, isid)));
                        }

                        if (lldp_req->system_description || all_opt_parameter) {
                            CPRINTF("%-13s ", cli_bool_txt(lldp_mgmt_get_opt_tlv_enabled(LLDP_TLV_BASIC_MGMT_SYSTEM_DESCR, pit.iport, isid)));
                        }

                        if (lldp_req->system_capabilities || all_opt_parameter) {
                            CPRINTF("%-12s ", cli_bool_txt(lldp_mgmt_get_opt_tlv_enabled(LLDP_TLV_BASIC_MGMT_SYSTEM_CAPA, pit.iport, isid)));
                        }

                        if (lldp_req->management_addr || all_opt_parameter) {
                            CPRINTF("%-11s", cli_bool_txt(lldp_mgmt_get_opt_tlv_enabled(LLDP_TLV_BASIC_MGMT_MGMT_ADDR, pit.iport, isid)));
                        }
                    }

#ifdef VTSS_SW_OPTION_CDP
                    if (is_cdp_aware) {
                        CPRINTF("%-14s  ", cli_bool_txt(conf.cdp_aware[pit.iport]));
                    }
#endif

                    CPRINTF("\n");
                }


                if (info) {
                    for (i = 0, entry = table; i < count; i++) {
                        if (entry == NULL) {
                            break;
                        }
                        if ((entry->in_use == 0) || (entry->receive_port != pit.iport)) {
                            entry++;
                            continue;
                        }

                        lldp_no_entry_found = 0;
                        if (first) {
                            first = 0;
                            cli_cmd_usid_print(usid, req, 1);
                        }

                        // This function takes an iport and prints a uport.
                        if (lldp_remote_receive_port_to_string(entry->receive_port, buf, isid)) {
                            T_D("Part of LAG");
                        }
                        CPRINTF("%-20s: %s\n", "Local port", buf);
                        cli_lldp_tlv("Chassis ID", entry, LLDP_TLV_BASIC_MGMT_CHASSIS_ID, 0);
                        cli_lldp_tlv("Port ID", entry, LLDP_TLV_BASIC_MGMT_PORT_ID, 0);
                        cli_lldp_tlv("Port Description", entry, LLDP_TLV_BASIC_MGMT_PORT_DESCR, 0);
                        cli_lldp_tlv("System Name", entry, LLDP_TLV_BASIC_MGMT_SYSTEM_NAME, 0);
                        cli_lldp_tlv("System Description", entry, LLDP_TLV_BASIC_MGMT_SYSTEM_DESCR, 0);
                        cli_lldp_tlv("System Capabilities", entry, LLDP_TLV_BASIC_MGMT_SYSTEM_CAPA, 0);

                        // Printing management address if present
                        for (mgmt_addr_index = 0; mgmt_addr_index < LLDP_MGMT_ADDR_CNT; mgmt_addr_index++) {
                            if (entry->mgmt_addr[mgmt_addr_index].length > 0) {
                                cli_lldp_tlv("Management Address", entry, LLDP_TLV_BASIC_MGMT_MGMT_ADDR, mgmt_addr_index);
                            }
                        }
#ifdef VTSS_SW_OPTION_POE
                        lldp_remote_poeinfo2string(entry, &buf[0]);
                        CPRINTF("%-20s: %s\n", "Power Over Ethernet", buf);
#endif
                        CPRINTF("\n");
                        if (entry != NULL) {
                            entry++;
                        }
                    }
                }

                if (stats) {
                    if (first) {
                        lldp_mib_stats_t global_cnt;
                        time_t           last_change_ago;
                        char             last_change_str[255];

                        (void)lldp_mgmt_stat_get(VTSS_ISID_START /* anything valid (doesn't have to exist) */, NULL, &global_cnt, &last_change_ago);
                        lldp_mgmt_last_change_ago_to_str(last_change_ago, &last_change_str[0]);

                        first = 0;
                        cli_cmd_usid_print(usid, req, 1);

                        // Global counters //
                        CPRINTF("LLDP global counters\n");

                        CPRINTF("Neighbor entries was last changed at %s.\n", last_change_str);

                        CPRINTF("Total Neighbors Entries Added    %d.\n", global_cnt.table_inserts);
                        CPRINTF("Total Neighbors Entries Deleted  %d.\n", global_cnt.table_deletes);
                        CPRINTF("Total Neighbors Entries Dropped  %d.\n", global_cnt.table_drops);
                        CPRINTF("Total Neighbors Entries Aged Out %d.\n", global_cnt.table_ageouts);

                        // Local counters //
                        CPRINTF("\nLLDP local counters\n");
                        CPRINTF("       Rx       Tx       Rx       Rx       Rx TLV   Rx TLV   Rx TLV\n");
                        CPRINTF("Port   Frames   Frames   Errors   Discards Errors   Unknown  Organiz.  Aged\n");
                        CPRINTF("----   ------   ------   ------   -------- ------   -------  -------  -----\n");


#ifdef VTSS_SW_OPTION_AGGR
                        aggr_mgmt_group_member_t glag_members;
                        vtss_glag_no_t glag_no;
                        // Insert the statistic for the GLAGs. The lowest port number in the GLAG collection contains statistic (See also packet_api.h).
                        for (glag_no = AGGR_MGMT_GROUP_NO_START; glag_no < AGGR_MGMT_GROUP_NO_END; glag_no++) {
                            // Get the port members
                            T_D("Get glag members, isid = %d, glag_no = %u", isid, glag_no);
                            (void) aggr_mgmt_members_get(isid, glag_no, &glag_members, FALSE);
                            vtss_port_no_t      iport_number;
                            // Loop through all ports. Stop at first port that is part of the GLAG.
                            for (iport_number = VTSS_PORT_NO_START; iport_number < VTSS_PORT_NO_END; iport_number++) {

                                if (iport_number < LLDP_PORTS) { // Make sure that we don't index array out-of-bounds.
                                    if (glag_members.entry.member[iport_number]) {
                                        char lag_string[50] = "";
                                        if (glag_no >= AGGR_MGMT_GLAG_START) {
                                            sprintf(lag_string, "GLAG %u", glag_no - AGGR_MGMT_GLAG_START + 1);
                                        } else {
                                            sprintf(lag_string, "LLAG %u", glag_no - AGGR_MGMT_GROUP_NO_START + 1);
                                        }

                                        stat = &stat_table[iport_number - VTSS_PORT_NO_START];
                                        CPRINTF("%-6s %-8d %-8d %-8d %-8d %-8d %-8d %-8d %-8d\n",
                                                lag_string,
                                                stat->rx_total,
                                                stat->tx_total,
                                                stat->rx_error,
                                                stat->rx_discarded,
                                                stat->TLVs_discarded,
                                                stat->TLVs_unrecognized,
                                                stat->TLVs_org_discarded,
                                                stat->ageouts);

                                        break;
                                    }
                                }
                            }
                        }
#endif // VTSS_SW_OPTION_AGGR
                    }

                    // Check if the port is part of a LAG
                    // This function takes an iport and prints a uport.
                    if (lldp_remote_receive_port_to_string(pit.iport, buf, isid) == 1) {
                        CPRINTF("%-6u %-8s%s\n",
                                pit.uport,
                                "Part of aggr ",
                                buf);
                    } else {
                        stat = &stat_table[pit.iport];
                        CPRINTF("%-6u %-8d %-8d %-8d %-8d %-8d %-8d %-8d %-8d\n",
                                pit.uport,
                                stat->rx_total,
                                stat->tx_total,
                                stat->rx_error,
                                stat->rx_discarded,
                                stat->TLVs_discarded,
                                stat->TLVs_unrecognized,
                                stat->TLVs_org_discarded,
                                stat->ageouts);
                    }
                }
            }
        } /* Port loop */

        // If we were getting info, we must release the mutex.
        if (info) {
            lldp_mgmt_get_unlock();
        }

        if (req->set) {
            if (mode) {
                rc |= lldp_mgmt_set_admin_state(conf.admin_state, isid);
            }

            if (opt_tlvs) {
                rc |= lldp_mgmt_set_optional_tlvs(conf.optional_tlv, isid);
            }

#ifdef VTSS_SW_OPTION_CDP
            if (is_cdp_aware) {
                rc |= lldp_mgmt_set_cdp_aware(conf.cdp_aware, isid);
            }
#endif

            if (rc != VTSS_OK) {
                CPRINTF("%s \n", lldp_error_txt(rc));
                return;
            }
        }
    } /* USID loop */

    if (lldp_no_entry_found && info) {
        CPRINTF("No LLDP entries found \n");
    }
}

int32_t cli_lldp_keyword_parse (char *cmd, char *cmd2, char *stx, char *cmd_org,
                                cli_req_t *req)
{
    char *found = cli_parse_find(cmd, stx);
    lldp_cli_req_t *lldp_req = NULL;

    T_I("ALLL %s", found);
    if (found != NULL) {
        lldp_req = req->module_req;

        if (!strncmp(found, "port_descr", 9 )) {
            lldp_req->port_description = 1;
        } else if (!strncmp(found, "sys_name", 8 )) {
            lldp_req->system_name = 1;
        } else if (!strncmp(found, "sys_descr", 9)) {
            lldp_req->system_description = 1;
        } else if (!strncmp(found, "sys_capa", 8)) {
            lldp_req->system_capabilities = 1;
        } else if (!strncmp(found, "mgmt_addr", 9 )) {
            lldp_req->management_addr = 1;
        }
    }

    return (found == NULL ? 1 : 0);
}

static int32_t cli_lldp_interval_parse (char *cmd, char *cmd2, char *stx, char *cmd_org,
                                        cli_req_t *req)
{
    lldp_cli_req_t *lldp_req = NULL;
    ulong error = 0;
    ulong value;

    req->parm_parsed = 1;
    lldp_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 5, 32768);
    lldp_req->lldp = value;
    return (error);
}

static int32_t cli_lldp_hold_parse (char *cmd, char *cmd2, char *stx, char *cmd_org,
                                    cli_req_t *req)
{
    lldp_cli_req_t *lldp_req = NULL;
    ulong error = 0;
    ulong value;

    req->parm_parsed = 1;
    lldp_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 2, 10);
    lldp_req->lldp = value;
    return (error);
}

static int32_t cli_lldp_delay_parse (char *cmd, char *cmd2, char *stx, char *cmd_org,
                                     cli_req_t *req)
{
    lldp_cli_req_t *lldp_req = NULL;
    ulong error = 0;
    ulong value;

    req->parm_parsed = 1;
    lldp_req = req->module_req;

    error = cli_parse_ulong(cmd, &value, 1, 8192);
    lldp_req->lldp = value;
    return (error);
}

static int32_t cli_lldp_reinit_parse (char *cmd, char *cmd2, char *stx, char *cmd_org,
                                      cli_req_t *req)
{
    lldp_cli_req_t *lldp_req = NULL;
    ulong error = 0;
    ulong value;

    req->parm_parsed = 1;
    lldp_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 1, 10);
    lldp_req->lldp = value;
    return (error);
}

static int32_t cli_lldp_optional_tlv_parse ( char *cmd, char *cmd2, char *stx, char *cmd_org,
                                             cli_req_t *req)
{
    char *found = cli_parse_find(cmd, stx);
    T_I("ALLL %s", found);

    if (found != NULL) {
        if (!strncmp(found, "enable", 6)) {
            req->enable = 1;
        } else if (!strncmp(found, "disable", 7)) {
            req->disable = 1;
        }
    }

    return (found == NULL ? 1 : 0);
}

static int32_t cli_lldp_mode_parse (char *cmd, char *cmd2, char *stx, char *cmd_org,
                                    cli_req_t *req)
{
    char *found = cli_parse_find(cmd, stx);
    lldp_cli_req_t *lldp_req = NULL;

    T_I("ALLL %s", found);

    lldp_req = req->module_req;
    if (found != NULL) {
        if (!strncmp(found, "enable", 6)) {
            req->enable = 1;
        } else if (!strncmp(found, "disable", 7)) {
            req->disable = 1;
        } else if (!strncmp(found, "rx", 2)) {
            lldp_req->rx = 1;
        } else if (!strncmp(found, "tx", 2)) {
            lldp_req->tx = 1;
        }
    }

    return (found == NULL ? 1 : 0);
}

int32_t cli_lldp_cdp_aware_parse (char *cmd, char *cmd2, char *stx, char *cmd_org,
                                  cli_req_t *req)
{
    char *found = cli_parse_find(cmd, stx);
    T_I("ALLL %s", found);

    if (found != NULL) {
        if (!strncmp(found, "enable", 6)) {
            req->enable = 1;
        } else if (!strncmp(found, "disable", 7)) {
            req->disable = 1;
        }
    }

    return (found == NULL ? 1 : 0);
}

static int32_t cli_lldp_statistics_parse (char *cmd, char *cmd2, char *stx, char *cmd_org,
                                          cli_req_t *req)
{
    char *found = cli_parse_find(cmd, stx);

    T_I("ALLL %s", found);
    req->parm_parsed = 1;

    if (found != NULL) {
        if (!strncmp(found, "clear", 5)) {
            req->clear = 1;
        }
    }

    return (found == NULL ? 1 : 0);
}

static int32_t cli_lldp_parse_keyword(char *cmd, char *cmd2, char *stx, char *cmd_org,
                                      cli_req_t *req)
{
    lldp_cli_req_t *lldp_req = NULL;

    char *found = cli_parse_find(cmd, stx);
    T_I("ALL %s", found);

    req->parm_parsed = 1;

    lldp_req = req->module_req;

    if (found != NULL) {
        if (!strncmp(found, "port_descr", 9 )) {
            lldp_req->port_description = 1;
        } else if (!strncmp(found, "sys_name", 8 )) {
            lldp_req->system_name = 1;
        } else if (!strncmp(found, "sys_descr", 9)) {
            lldp_req->system_description = 1;
        } else if (!strncmp(found, "sys_capa", 8)) {
            lldp_req->system_capabilities = 1;
        } else if (!strncmp(found, "mgmt_addr", 9 )) {
            lldp_req->management_addr = 1;
        }
    }

    return (found == NULL ? 1 : 0);
}

cli_parm_t lldp_parm_table[] = {
    {
        "port_descr|sys_name|sys_descr|sys_capa|mgmt_addr",
        "port_descr     : Description of the port\n"
        "sysm_name      : System name\n"
        "sys_descr      : Description of the system\n"
        "sys_capa       : System capabilities \n"
        "mgmt_addr      : Master's IP address \n"
        "(default: Show optional TLV's configuration)",
        CLI_PARM_FLAG_NO_TXT,
        cli_lldp_parse_keyword,
        NULL,
    },
    {
        "enable|disable",
        "enable     : Enables TLV\n"
        "disable    : Disable TLV\n"
        "(default: Show optional TLV's configuration)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_lldp_optional_tlv_parse,
        cli_cmd_lldp_opt_tlv,

    },
    {
        "enable|disable|rx|tx",
        "enable : Enable LLDP reception and transmission\n"
        "disable: Disable LLDP\n"
        "rx     : Enable LLDP reception only\n"
        "tx     : Enable LLDP transmission only\n"
        "(default: Show LLDP mode)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_lldp_mode_parse,
        cli_cmd_lldp_mode,
    },
#ifdef VTSS_SW_OPTION_CDP
    {
        "enable|disable",
        "enable : Enable CDP awareness (CDP discovery information is added to the LLDP neighbor table)\n"
        "disable: Disable CDP awareness\n"
        "(default: Show CDP awareness configuration)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_lldp_cdp_aware_parse,
        cli_cmd_lldp_lldp_cdp_aware,
    },
#endif
    {
        "<interval>",
        "LLDP transmission interval (5-32768)",
        CLI_PARM_FLAG_SET,
        cli_lldp_interval_parse,
        NULL,
    },
    {
        "<hold>",
        "LLDP hold value (2-10)",
        CLI_PARM_FLAG_SET,
        cli_lldp_hold_parse,
        NULL,
    },
    {
        "<delay>",
        "LLDP transmission delay (1-8192)",
        CLI_PARM_FLAG_SET,
        cli_lldp_delay_parse,
        cli_cmd_lldp_lldp_delay,
    },
    {
        "<reinit>",
        "LLDP reinit delay (1-10)",
        CLI_PARM_FLAG_SET,
        cli_lldp_reinit_parse,
        cli_cmd_lldp_lldp_reinit,
    },
    {
        "clear",
        "Clear LLDP statistics",
        CLI_PARM_FLAG_NONE,
        cli_lldp_statistics_parse,
        NULL,
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
    PRIO_LLDP_CONF,
    PRIO_LLDP_MODE,
    PRIO_LLDP_OPT_TLV,
    PRIO_LLDP_INTERVAL,
    PRIO_LLDP_HOLD,
    PRIO_LLDP_DELAY,
    PRIO_LLDP_REINIT,
    PRIO_LLDP_STAT,
    PRIO_LLDP_INFO,
    PRIO_LLDP_CDP_AWARE,
    PRIO_LLDP_CDP_BIST
};

/* Command table entries */
cli_cmd_tab_entry (
    "LLDP Configuration [<port_list>]",
    NULL,
    "Show LLDP configuration",
    PRIO_LLDP_CONF,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_LLDP,
    cli_cmd_lldp_config,
    NULL,
    lldp_parm_table,
    CLI_CMD_FLAG_SYS_CONF
);

cli_cmd_tab_entry (
    "LLDP Mode [<port_list>]",
    "LLDP Mode [<port_list>] [enable|disable|rx|tx]",
    "Set or show LLDP mode",
    PRIO_LLDP_MODE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_LLDP,
    cli_cmd_lldp_mode,
    NULL,
    lldp_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "LLDP Optional_TLV [<port_list>]",
    "LLDP Optional_TLV [<port_list>] [port_descr|sys_name|sys_descr|sys_capa|mgmt_addr] [enable|disable]",
    "Set or show LLDP Optional TLVs",
    PRIO_LLDP_OPT_TLV,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_LLDP,
    cli_cmd_lldp_opt_tlv,
    NULL,
    lldp_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "LLDP Interval",
    "LLDP Interval [<interval>]",
    "Set or show LLDP Tx interval",
    PRIO_LLDP_INTERVAL,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_LLDP,
    cli_cmd_lldp_lldp_interval,
    NULL,
    lldp_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "LLDP Hold",
    "LLDP Hold [<hold>]",
    "Set or show LLDP Tx hold value",
    PRIO_LLDP_HOLD,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_LLDP,
    cli_cmd_lldp_lldp_hold,
    NULL,
    lldp_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "LLDP Delay",
    "LLDP Delay [<delay>]",
    "Set or show LLDP Tx delay",
    PRIO_LLDP_DELAY,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_LLDP,
    cli_cmd_lldp_lldp_delay,
    NULL,
    lldp_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "LLDP Reinit",
    "LLDP Reinit [<reinit>]",
    "Set or show LLDP reinit delay",
    PRIO_LLDP_REINIT,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_LLDP,
    cli_cmd_lldp_lldp_reinit,
    NULL,
    lldp_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "LLDP Info [<port_list>]",
    NULL,
    "Show LLDP neighbor device information",
    PRIO_LLDP_INFO,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_LLDP,
    cli_cmd_lldp_lldp_info,
    NULL,
    lldp_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "LLDP Statistics [<port_list>]",
    "LLDP Statistics [<port_list>] [clear]",
    "Show LLDP Statistics",
    PRIO_LLDP_STAT,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_LLDP,
    cli_cmd_lldp_lldp_stat,
    NULL,
    lldp_parm_table,
    CLI_CMD_FLAG_NONE
);

#ifdef VTSS_SW_OPTION_CDP
cli_cmd_tab_entry (
    "LLDP cdp_aware [<port_list>]",
    "LLDP cdp_aware [<port_list>] [enable|disable]",
    "Set or show if discovery information from received CDP ( Cisco Discovery Protocol ) frames is added to the LLDP neighbor table",
    PRIO_LLDP_CDP_AWARE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_LLDP,
    cli_cmd_lldp_lldp_cdp_aware,
    NULL,
    lldp_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif /* VTSS_SW_OPTION_CDP */

#ifdef VTSS_SW_OPTION_CDP_BIST
cli_cmd_tab_entry (
    NULL,
    "LLDP cdp_bist [<port_list>]",
    "Start a build in self test - The user must make a physical loop between the port being testing and the next port. E.g. if the port being tested is port 15, then a physical loop must be made between port 15 and 16.",
    PRIO_LLDP_CDP_BIST,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_LLDP,
    cli_cdp_bist,
    NULL,
    lldp_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif /* VTSS_SW_OPTION_CDP_BIST */
