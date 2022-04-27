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
#include "aggr_cli.h"
#include "vtss_module_id.h"
#include "cli_trace_def.h"
#include "mgmt_api.h"
#include "msg_api.h"
#include "aggr_api.h"
typedef struct {
    aggr_mgmt_group_no_t  aggr_id;

    /* Keywords */
    BOOL                  kw_smac;
    BOOL                  kw_dmac;
    BOOL                  ip;
    uint16_t              port;
} aggr_cli_req_t;

void aggr_cli_req_init(void)
{
    /* register the size required for aggr req. structure */
    cli_req_size_register(sizeof(aggr_cli_req_t));
}

/* Aggregation mode */
static void aggr_cli_cmd_mode(cli_req_t *req )
{
    vtss_aggr_mode_t mode;
    aggr_cli_req_t    *aggr_req;

    aggr_req = req->module_req;


    if (cli_cmd_conf_slave(req) || aggr_mgmt_aggr_mode_get(&mode) != VTSS_RC_OK) {
        return;
    }

    /* If no fields are specified, all fields are used */
    if (!(req->smac || req->dmac || req->aggr_ip || req->port)) {
        req->smac = 1;
        req->dmac = 1;
        req->aggr_ip = 1;
        req->port = 1;
    }

    if (req->set) {
        if (req->smac) {
            mode.smac_enable = req->enable;
        }
        if (req->dmac) {
            mode.dmac_enable = req->enable;
        }
        if (req->aggr_ip) {
            mode.sip_dip_enable = req->enable;
        }
        if (req->port) {
            mode.sport_dport_enable = req->enable;
        }

        if (aggr_mgmt_aggr_mode_set(&mode) != VTSS_RC_OK) {
            CPRINTF("Aggregation mode set operation failed\n");
        }
    } else {
        CPRINTF("Aggregation Mode:\n\n");
        if (req->smac) {
            CPRINTF("SMAC  : %s\n", cli_bool_txt(mode.smac_enable));
        }
        if (req->dmac) {
            CPRINTF("DMAC  : %s\n", cli_bool_txt(mode.dmac_enable));
        }
        if (req->aggr_ip) {
            CPRINTF("IP    : %s\n", cli_bool_txt(mode.sip_dip_enable));
        }
        if (req->port) {
            CPRINTF("Port  : %s\n", cli_bool_txt(mode.sport_dport_enable));
        }
    }
}

static void cli_aggr_list(cli_req_t *req, vtss_usid_t usid,
                          aggr_mgmt_group_no_t min, aggr_mgmt_group_no_t max, BOOL *first)
{
    vtss_isid_t              isid;
    aggr_mgmt_group_no_t     aggr_no;
    aggr_mgmt_group_member_t group, *grp;
#ifdef VTSS_SW_OPTION_LACP
    aggr_mgmt_group_member_t lacp;
#endif /* VTSS_SW_OPTION_LACP */
    char                     buf[MGMT_PORT_BUF_SIZE], *p;
    BOOL                     next;
    vtss_rc                  rc;
    vtss_port_speed_t        spd;

    if (min == max) {
        next = 0;
        aggr_no = min;
    } else {
        next = 1;
        aggr_no = (min - 1);
    }

    isid = req->stack.isid[usid];

    while (1) {
        rc = aggr_mgmt_port_members_get(isid, aggr_no, &group, next);
        grp = &group;
#ifdef VTSS_SW_OPTION_LACP
        if (msg_switch_is_master() &&
            aggr_mgmt_lacp_members_get(isid, aggr_no, &lacp, next) == VTSS_RC_OK &&
            (rc != VTSS_RC_OK || lacp.aggr_no < grp->aggr_no)) {
            /* LACP group found and static group not found or bigger */
            grp = &lacp;
            rc = VTSS_RC_OK;
        }
#endif /* VTSS_SW_OPTION_LACP */
        if (rc != VTSS_RC_OK || ((aggr_no = grp->aggr_no) > max)) {
            break;
        }

        if (!AGGR_MGMT_GROUP_IS_AGGR(aggr_no)) {
            break;
        }

        if (*first) {
            *first = 0;
            cli_cmd_usid_print(usid, req, 1);
            p = &buf[0];
            p += sprintf(p, "Aggr ID  Name    ");
#if  defined(VTSS_SW_OPTION_LACP)
            p += sprintf(p, "Type    ");
#endif /* VTSS_SW_OPTION_LACP */
            p += sprintf(p, "Configured Ports  ");
            p += sprintf(p, "Aggregated Ports  ");
            p += sprintf(p, "Group Speed");
            cli_table_header(buf);
        }

#if VTSS_SWITCH_STACKABLE
        if (aggr_no >= AGGR_MGMT_GLAG_START) {
            sprintf(buf, "%s%u", "GLAG", AGGR_MGMT_NO_TO_ID(aggr_no));
        } else {
            sprintf(buf, "%s%u", "LLAG", AGGR_MGMT_NO_TO_ID(aggr_no));
        }
#else
        sprintf(buf, "%s%u", "LLAG", AGGR_MGMT_NO_TO_ID(aggr_no));;
#endif /*VTSS_SWITCH_STACKABLE */
        CPRINTF("%-2u       %-8s", mgmt_aggr_no2id(aggr_no), buf);
#if  defined(VTSS_SW_OPTION_LACP)
        CPRINTF("%-6s  ", grp == &group ? "Static" : "LACP");
#endif /* VTSS_SW_OPTION_LACP */
        if (grp == &group) {
            CPRINTF("%-18s", cli_iport_list_txt(grp->entry.member, buf));
        } else {
            CPRINTF("%-18s", "-");
        }
        rc = aggr_mgmt_members_get(isid, aggr_no, grp, 0);
        spd = aggr_mgmt_speed_get(isid, aggr_no);

        CPRINTF("%-18s", cli_iport_list_txt(grp->entry.member, buf));
        CPRINTF("%-18s\n", spd == VTSS_SPEED_10M ? "10M" :
                spd == VTSS_SPEED_100M ?  "100M" :
                spd == VTSS_SPEED_1G ?    "1G" :
                spd == VTSS_SPEED_2500M ? "2G5" :
                spd == VTSS_SPEED_10G ?   "10G" :
                spd == VTSS_SPEED_12G ?   "12G" : (grp == &group) ? "Undefined" : "-");
        if (!next) {
            break;
        }
    }
}


/* Aggregation lookup */
static void aggr_cli_cmd_lookup(cli_req_t *req )
{
    vtss_usid_t          usid;
    aggr_mgmt_group_no_t aggr_no;
    BOOL                 first;
    aggr_cli_req_t    *aggr_req;

    aggr_req = req->module_req;

    if (cli_cmd_switch_none(req)) {
        return;
    }

    for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
        if (req->stack.isid[usid] == VTSS_ISID_END) {
            continue;
        }

        first = 1;
        if (aggr_req->aggr_id) {
            /* Specific aggregation */
            aggr_no = mgmt_aggr_id2no(aggr_req->aggr_id);
            cli_aggr_list(req, usid, aggr_no, aggr_no, &first);
        } else {
            /* List GLAGs, then LLAGs */
#if defined(VTSS_FEATURE_VSTAX_V2)
            cli_aggr_list(req, usid, AGGR_MGMT_GROUP_NO_START, AGGR_MGMT_GROUP_NO_END - 1, &first);
#else
            cli_aggr_list(req, usid, AGGR_MGMT_GLAG_START, AGGR_MGMT_GLAG_END - 1, &first);
            cli_aggr_list(req, usid, AGGR_MGMT_GROUP_NO_START, AGGR_MGMT_GLAG_START - 1, &first);
#endif
        }
    }
}

static void aggr_cli_cmd_add(cli_req_t *req )
{
    vtss_usid_t              usid;
    vtss_isid_t              isid;
    vtss_uport_no_t          uport;
    vtss_port_no_t           iport;
    char                     buf[80];
    aggr_mgmt_group_no_t     aggr_no, aggr_add, aggr_free;
    aggr_mgmt_group_member_t group;
    aggr_mgmt_group_t        iport_list;
    int                      error;
    vtss_rc                  rc;
    aggr_cli_req_t          *aggr_req;
    u32                      port_count;
    BOOL                     found_member;

    aggr_req = req->module_req;
    memset(&group, 0, sizeof(aggr_mgmt_group_member_t));
    if (cli_cmd_switch_none(req) || cli_cmd_slave(req)) {
        return;
    }

    for (iport = VTSS_PORT_NO_START; iport < VTSS_PORT_NO_END; iport++) {
        iport_list.member[iport] = req->uport_list[iport2uport(iport)];
    }

    aggr_add = (aggr_req->aggr_id ? mgmt_aggr_id2no(aggr_req->aggr_id) : aggr_req->aggr_id);

    for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
        if ((isid = req->stack.isid[usid]) == VTSS_ISID_END) {
            continue;
        }
        port_count = port_isid_port_count(isid);
        found_member = 0;

        if (mgmt_aggr_add_check(isid, aggr_add, iport_list.member, buf) != VTSS_RC_OK) {
            CPRINTF("%s\n", buf);
            return;
        }

        /* Check that ports are not already used in other aggregations and find next free ID */
        aggr_add = (aggr_req->aggr_id ? mgmt_aggr_id2no(aggr_req->aggr_id) : aggr_req->aggr_id);
        for (error = 0, aggr_no = 0, aggr_free = AGGR_MGMT_GROUP_NO_START; !error && aggr_mgmt_port_members_get(isid, aggr_no, &group, 1) == VTSS_RC_OK; ) {
            aggr_no = group.aggr_no;
            if (aggr_no == aggr_add) {
                continue;
            }
            if (aggr_no == aggr_free) {
                aggr_free++;
            }
            for (iport = VTSS_PORT_NO_START; !error && iport < port_count; iport++) {
                uport = iport2uport(iport);
                if (req->uport_list[uport] && group.entry.member[iport]) {
                    cli_cmd_usid_print(usid, req, 0);
                    CPRINTF("Port %u is already included in aggregation %u\n",
                            uport, mgmt_aggr_no2id(aggr_no));
                    error = 1;
                }
            }
        }
        if (error) {
            continue;
        }

        /* Check if more aggregation IDs are available */
        if (aggr_add == 0) {
            aggr_add = aggr_free;
            if (aggr_add >= AGGR_MGMT_GROUP_NO_END) {
                cli_cmd_usid_print(usid, req, 0);
                CPRINTF("No more aggregations available\n");
                continue;
            }
        }

        /* Add aggregation */
        for (iport = VTSS_PORT_NO_START; iport < port_count; iport++) {
            if (req->uport_list[iport2uport(iport)]) {
                found_member = 1;
            }
            group.entry.member[iport] = req->uport_list[iport2uport(iport)];
        }

        if (!found_member) {
            CPRINTF("Port members are invalid for switch (isid:%d)\n", isid);
            continue;
        }

        if ((rc = aggr_mgmt_port_members_add(isid, aggr_add, &group.entry)) != VTSS_RC_OK) {
            cli_cmd_usid_print(usid, req, 0);
            if (rc == AGGR_ERROR_LACP_ENABLED) {
                CPRINTF("LACP is enabled on one or more ports in the aggregation\n");
            } else if (rc == AGGR_ERROR_DOT1X_ENABLED) {
                CPRINTF("802.1X is enabled on one or more ports in the aggregation\n");
            } else {
                CPRINTF("Aggregation add failed\n");
            }
        }
    }
}

/* Aggregation delete */
static void aggr_cli_cmd_aggr_del(cli_req_t *req )
{
    vtss_usid_t usid;
    vtss_isid_t isid;
    aggr_cli_req_t    *aggr_req;
    aggr_mgmt_group_member_t group;

    aggr_req = req->module_req;

    req->set = 1; /* Indicate SET operation for cli_cmd_conf_slave */
    if (cli_cmd_switch_none(req) || cli_cmd_conf_slave(req)) {
        return;
    }

    for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
        if ((isid = req->stack.isid[usid]) == VTSS_ISID_END) {
            continue;
        }

        if (aggr_mgmt_port_members_get(isid, mgmt_aggr_id2no(aggr_req->aggr_id), &group, 0) == AGGR_ERROR_ENTRY_NOT_FOUND) {
            CPRINTF("The aggregation does not exist\n");
        }
        if (aggr_mgmt_group_del(isid, mgmt_aggr_id2no(aggr_req->aggr_id)) != VTSS_RC_OK) {
            CPRINTF("Aggregation delete operation failed\n");
        }

    }
}

/* List a range of aggregations */

static void aggr_cli_cmd_conf ( cli_req_t *req )
{
    req->all = 1;
    if (!req->set) {
        cli_header("Aggregation Configuration", 1);
    }
    aggr_cli_cmd_mode(req);
    aggr_cli_cmd_lookup(req);
}

#if defined(VTSS_ARCH_LUTON28)
static void aggr_cli_cmd_debug_aggr(cli_req_t *req )
{
    int addr, p, blk, sub;
    ulong reg_addr, value, value2;

    // Source port masks
    for (addr = 0x80, p = 0; addr <= 0x9b; addr++, p++) {
        reg_addr = misc_l28_reg_addr(2, 0, addr);
        if (misc_debug_reg_read(VTSS_ISID_LOCAL, 0, reg_addr, &value) != VTSS_RC_OK) {
            CPRINTF("Debug reg read operation failed\n");
        }

        if (p > 15) {
            blk = 6;
        } else {
            blk = 1;
        }

        sub = p % 16;

        reg_addr = misc_l28_reg_addr(blk, sub, 0x25);
        if (misc_debug_reg_read(VTSS_ISID_LOCAL, 0, reg_addr, &value2) != VTSS_RC_OK) {
            CPRINTF("Debug reg read operation failed\n");
        }
        printf("Source mask for port %d:0x%x. StackCfg,PortId:%u\n", p, value, (value2 & 0x1f));
    }
    printf("\n");
    // Dest masks
    for (addr = 0x40, p = 0; addr <= 0x7f; addr++, p++) {
        reg_addr = misc_l28_reg_addr(2, 0, addr);
        if (misc_debug_reg_read(VTSS_ISID_LOCAL, 0, reg_addr, &value) != VTSS_RC_OK) {
            CPRINTF("Debug reg read operation failed\n");
        }

        if (p < 30) {
            printf("Dest mask for port %d:0x%x\n", p, value);
        } else if (p == 30) {
            printf("(30)Dest mask for GLAG 0 0x%x\n", value);
        } else if (p == 31) {
            printf("(31)Dest mask for GLAG 1 0x%x\n", value);
        } else if (p == 32) {
            printf("(32)Source mask for GLAG 0 %x\n", value);
        } else if (p == 33) {
            printf("(33)Source mask for GLAG 1 %x\n", value);
        } else if (p == 34) {
            printf("(34)Allow forwarding on stack port A for frames destined to GLAG 0: %x\n", value);
        } else if (p == 35) {
            printf("(35)Allow forwarding on stack port A for frames destined to GLAG 1: %x\n", value);
        } else if (p == 36) {
            printf("(36)Allow forwarding on stack port B for frames destined to GLAG 0: %x\n", value);
        } else if (p == 37) {
            printf("(37)Allow forwarding on stack port B for frames destined to GLAG 1: %x\n", value);
        }
    }
    printf("\n");
    // Aggr masks
    for (addr = 0x30, p = 0; addr <= 0x3f; addr++, p++) {
        reg_addr = misc_l28_reg_addr(2, 0, addr);
        if (misc_debug_reg_read(VTSS_ISID_LOCAL, 0, reg_addr, &value) != VTSS_RC_OK) {
            CPRINTF("Debug reg read operation failed\n");
        }
        printf("Aggr mask %d:0x%x\n", p, value);
    }
}
#else
static void aggr_cli_cmd_debug_aggr(cli_req_t *req )
{
    aggr_mgmt_dump(cli_printf);
}
#endif /* VTSS_ARCH_LUTON28 */

static int32_t cli_parse_aggr_id (char *cmd, char *cmd2,
                                  char *stx, char *cmd_org,
                                  cli_req_t *req)
{
    aggr_cli_req_t *aggr_req = NULL;

    aggr_req = req->module_req;
    req->parm_parsed = 1;
    return (cli_parse_ulong(cmd, &aggr_req->aggr_id, MGMT_AGGR_ID_START, MGMT_AGGR_ID_END - 1));
}

static cli_parm_t aggr_cli_parm_table[] = {
#if 0 // Not Required here
    {
        "<port_list>",
        "Port list",
        CLI_PARM_FLAG_NONE,
        PRIO_AGGR_ADD,
    },
#endif
    {
        "<aggr_id>",
#if VTSS_SWITCH_STACKABLE & defined(VTSS_ARCH_LUTON28)
        "Aggregation ID, global: 1-2, local: 3-14",
#elif VTSS_SWITCH_STACKABLE & defined(VTSS_ARCH_JAGUAR_1)
        "Aggregation ID: 1-32",
#elif (AGGR_LLAG_CNT == 5)
        "Aggregation ID: 1-5",
#elif (AGGR_LLAG_CNT == 8)
        "Aggregation ID: 1-8",
#elif (AGGR_LLAG_CNT == 12)
        "Aggregation ID: 1-12",
#elif (AGGR_LLAG_CNT == 14)
        "Aggregation ID: 1-14",
#else
        "Aggregation ID: 1 - (Number of frontports divided by 2)",
#endif /* VTSS_SWITCH_STACKABLE */
        CLI_PARM_FLAG_SET,
        cli_parse_aggr_id,
        NULL,
    },
    {
        "enable|disable",
        "enable : Enable field in traffic distribution\n"
        "disable: Disable field in traffic distribution",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        NULL,
    },
    {
        "smac|dmac|ip|port",
        "smac   : Source MAC address\n"
        "dmac   : Destination MAC address\n"
        "ip     : Source and destination IP address\n"
        "port   : Source and destination UDP/TCP port",
        CLI_PARM_FLAG_NO_TXT,
        cli_parm_parse_keyword,
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
    PRIO_AGGR_CONF,
    PRIO_AGGR_ADD,
    PRIO_AGGR_DEL,
    PRIO_AGGR_LOOKUP,
    PRIO_AGGR_MODE,
    PRIO_DEBUG_AGGR = CLI_CMD_SORT_KEY_DEFAULT,
};

/* Command table entries */
cli_cmd_tab_entry (
    "Aggr Configuration",
    NULL,
    "Show link aggregation configuration",
    PRIO_AGGR_CONF,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_AGGR,
    aggr_cli_cmd_conf,
    NULL,
    aggr_cli_parm_table,
    CLI_CMD_FLAG_SYS_CONF
);

cli_cmd_tab_entry (
    NULL,
    "Aggr Add <port_list> [<aggr_id>]",
    "Add or modify link aggregation",
    PRIO_AGGR_ADD,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_AGGR,
    aggr_cli_cmd_add,
    NULL,
    aggr_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    NULL,
    "Aggr Delete <aggr_id>",
    "Delete link aggregation",
    PRIO_AGGR_DEL,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_AGGR,
    aggr_cli_cmd_aggr_del,
    NULL,
    aggr_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "Aggr Lookup [<aggr_id>]",
    NULL,
    "Lookup link aggregation",
    PRIO_AGGR_LOOKUP,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_AGGR,
    aggr_cli_cmd_lookup,
    NULL,
    aggr_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "Aggr Mode [smac|dmac|ip|port]",
    "Aggr Mode [smac|dmac|ip|port] [enable|disable]",
    "Set or show the link aggregation traffic distribution mode",
    PRIO_AGGR_MODE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_AGGR,
    aggr_cli_cmd_mode,
    NULL,
    aggr_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

#if defined(VTSS_ARCH_LUTON28)
cli_cmd_tab_entry (
    "Debug aggr [<port_list>]",
    NULL,
    "Dump port masks",
    PRIO_DEBUG_AGGR,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    aggr_cli_cmd_debug_aggr,
    NULL,
    aggr_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#else
cli_cmd_tab_entry (
    "Debug aggr",
    NULL,
    "Dump aggr details",
    PRIO_DEBUG_AGGR,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    aggr_cli_cmd_debug_aggr,
    NULL,
    aggr_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

#endif /* VTSS_ARCH_LUTON28 */
