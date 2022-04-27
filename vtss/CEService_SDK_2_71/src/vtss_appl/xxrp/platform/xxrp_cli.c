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

#include "cli.h"
#include "xxrp_api.h"
#include "xxrp_cli.h"

/* Ignoring lint error in cli_cmd_xxrp_pkt_dump as it is a debug function */
/*lint -esym(459, cli_cmd_xxrp_pkt_dump) */
typedef struct {
    ulong timer_val;
} xxrp_cli_req_t;

/***************************************************************************************************
 * xxrp_cli_init()
 **************************************************************************************************/
void xxrp_cli_init(void)
{
    // Register the size required for this module's structure
    cli_req_size_register(sizeof(xxrp_cli_req_t));
}

/***************************************************************************************************
 * Command functions
 **************************************************************************************************/
static void cli_cmd_mrp_global_mode(cli_req_t *req, vtss_mrp_appl_t appl)
{
    vtss_rc rc;
    BOOL    enabled;

    if (cli_cmd_switch_none(req) || cli_cmd_conf_slave(req)) {
        return;
    }

    if (req->set) {
        if ((rc = xxrp_mgmt_global_enabled_set(appl, req->enable)) != VTSS_OK) {
            cli_printf("%s\n", error_txt(rc));
        }
    } else {
        if ((rc = xxrp_mgmt_global_enabled_get(appl, &enabled)) != VTSS_OK) {
            cli_printf("%s\n", error_txt(rc));
        } else {
            CPRINTF("Global Mode : %s\n", cli_bool_txt(enabled));
        }
    }
}

static void cli_cmd_mrp_port_config(cli_req_t *req, vtss_mrp_appl_t appl, BOOL mode, BOOL join, BOOL leave, BOOL leaveall, BOOL periodic)
{
    switch_iter_t         sit;
    port_iter_t           pit;
    vtss_rc               rc;
    BOOL                  enabled;
    vtss_mrp_timer_conf_t timers;
    BOOL                  periodic_tx;
    char                  buf[80], *p;

    if (cli_cmd_switch_none(req) || cli_cmd_slave(req)) {
        return;
    }

    (void)cli_switch_iter_init(&sit);
    while (cli_switch_iter_getnext(&sit, req)) {
        (void)cli_port_iter_init(&pit, sit.isid, PORT_ITER_FLAGS_NORMAL);
        while (cli_port_iter_getnext(&pit, req)) {

            if ((rc = xxrp_mgmt_enabled_get(sit.isid, pit.iport, appl, &enabled)) != VTSS_OK) {
                cli_printf("%s\n", error_txt(rc));
                continue;
            }
            if ((rc = xxrp_mgmt_timers_get(sit.isid, pit.iport, appl, &timers)) != VTSS_OK) {
                cli_printf("%s\n", error_txt(rc));
                continue;
            }
            if ((rc = xxrp_mgmt_periodic_tx_get(sit.isid, pit.iport, appl, &periodic_tx)) != VTSS_OK) {
                cli_printf("%s\n", error_txt(rc));
                continue;
            }

            if (req->set) {
                if (mode) {
                    if ((rc = xxrp_mgmt_enabled_set(sit.isid, pit.iport, VTSS_MRP_APPL_MVRP, req->enable)) != VTSS_OK) {
                        cli_printf("%s\n", error_txt(rc));
                    }
                }
                if (join || leave || leaveall) {
                    if (join) {
                        timers.join_timer = req->value;
                    }
                    if (leave) {
                        timers.leave_timer = req->value;
                    }
                    if (leaveall) {
                        timers.leave_all_timer = req->value;
                    }
                    if ((rc = xxrp_mgmt_timers_set(sit.isid, pit.iport, appl, &timers)) != VTSS_OK) {
                        cli_printf("%s\n", error_txt(rc));
                    }
                } else if (periodic) {
                    if ((rc = xxrp_mgmt_periodic_tx_set(sit.isid, pit.iport, appl, req->enable)) != VTSS_OK) {
                        cli_printf("%s\n", error_txt(rc));
                    }
                }
            } else {
                if (pit.first) {
                    cli_cmd_usid_print(sit.usid, req, 1);
                    p = &buf[0];
                    p += sprintf(p, "Port  ");
                    if (mode) {
                        p += sprintf(p, "Mode      ");
                    }
                    if (join) {
                        p += sprintf(p, "Join  ");
                    }
                    if (leave) {
                        p += sprintf(p, "Leave  ");
                    }
                    if (leaveall) {
                        p += sprintf(p, "LvAll  ");
                    }
                    if (periodic) {
                        p += sprintf(p, "Per. Tx  ");
                    }
                    cli_table_header(buf);
                }
                CPRINTF("%-6u", pit.uport);
                if (mode) {
                    CPRINTF("%-10s", cli_bool_txt(enabled));
                }
                if (join) {
                    CPRINTF("%-6u", timers.join_timer);
                }
                if (leave) {
                    CPRINTF("%-7u", timers.leave_timer);
                }
                if (leaveall) {
                    CPRINTF("%-7u", timers.leave_all_timer);
                }
                if (periodic) {
                    CPRINTF("%-10s", cli_bool_txt(periodic_tx));
                }
                CPRINTF("\n");
            }
        }
    }
}

static void cli_cmd_debug_xxrp_memory_stat(cli_req_t *req)
{
    u64        alloc_ct = 0;
    u64        free_ct  = 0;

    CPRINTF("XXRP Memory usage\n\r");
    alloc_ct = xxrp_mgmt_memory_mgmt_get_alloc_count();
    free_ct  = xxrp_mgmt_memory_mgmt_get_free_count();
    CPRINTF("Alloc Count - %-6lu    Free Count - %-6lu\n\r",
            (unsigned long)alloc_ct, (unsigned long)free_ct);
}

static void cli_cmd_xxrp_memory_stat(cli_req_t *req)
{
    cli_cmd_debug_xxrp_memory_stat(req);
}


















































































































































/***************************************************************************************************
 * Parameter functions
 **************************************************************************************************/
static int32_t cli_parm_parse_mrp_join_timer_val(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    req->parm_parsed = 1;
    return cli_parse_ulong(cmd, &req->value, VTSS_MRP_JOIN_TIMER_MIN, VTSS_MRP_JOIN_TIMER_MAX);
}

static int32_t cli_parm_parse_mrp_leave_timer_val(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    req->parm_parsed = 1;
    return cli_parse_ulong(cmd, &req->value, VTSS_MRP_LEAVE_TIMER_MIN, VTSS_MRP_LEAVE_TIMER_MAX);
}

static int32_t cli_parm_parse_mrp_leaveall_timer_val(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    req->parm_parsed = 1;
    return cli_parse_ulong(cmd, &req->value, VTSS_MRP_LEAVEALL_TIMER_MIN, VTSS_MRP_LEAVEALL_TIMER_MAX);
}
static int32_t cli_parm_parse_msti(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    req->parm_parsed = 1;
    return cli_parse_ulong(cmd, &req->value, 0, 7);
}
static int32_t cli_parm_parse_machine_index(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    req->parm_parsed = 1;
    return cli_parse_ulong(cmd, &req->value, 0, 4094);
}


/***************************************************************************************************
 * Parameter table
 **************************************************************************************************/
static cli_parm_t xxrp_cli_parm_table[] = {


















































































    {NULL, NULL, 0, 0, NULL}
};

/***************************************************************************************************
 * Command table
 **************************************************************************************************/
enum {
    PRIO_MVRP_CONF,
    PRIO_MVRP_GLOBAL_MODE,
    PRIO_MVRP_PORT_MODE,
    PRIO_MVRP_JOIN_TIMER_CONF,
    PRIO_MVRP_LEAVE_TIMER_CONF,
    PRIO_MVRP_LEAVEALL_TIMER_CONF,
    PRIO_MVRP_PERIODIC_TX_TMR_CONF,
    PRIO_MVRP_PORT_STATS,
    PRIO_DEBUG_MVRP_PRINT_RING  = CLI_CMD_SORT_KEY_DEFAULT,
    PRIO_DEBUG_MRP_MEMORY_STAT = CLI_CMD_SORT_KEY_DEFAULT,
    PRIO_DEBUG_MRP_PKT_DUMP = CLI_CMD_SORT_KEY_DEFAULT,
    PRIO_DEBUG_MRP_MAD_PRINT = CLI_CMD_SORT_KEY_DEFAULT
};

/* Command table entries */
























































































































cli_cmd_tab_entry (
    "Debug XXRP Memory Usage",
    NULL,
    "show the XXRP and its application's dynamic memory usage",
    PRIO_DEBUG_MRP_MEMORY_STAT,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_xxrp_memory_stat,
    NULL,
    xxrp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "NULL",
    "Debug MVRP Pktdump [enable|disable]",
    "Enable or Disable MVRP packet dump",
    PRIO_DEBUG_MRP_PKT_DUMP,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_xxrp_pkt_dump,
    NULL,
    xxrp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "NULL",
    "Debug MVRP mad print [<port_list>] [<machine_index>]",
    "Print MAD structure",
    PRIO_DEBUG_MRP_MAD_PRINT,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_xxrp_mad_print,
    NULL,
    xxrp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

