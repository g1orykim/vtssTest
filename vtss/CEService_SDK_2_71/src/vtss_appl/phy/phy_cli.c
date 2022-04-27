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
#include "vtss_api_if_api.h"
#include "cli.h"
#include "cli_api.h"
#include "mgmt_api.h"
#include "critd_api.h"
#include "port_api.h"
#include "cli_trace_def.h"
#include "port_api.h"
#include "phy_api.h"

#if defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_LUTON28) || defined(VTSS_ARCH_SERVAL)
#define CLI_BLK_MAX  (1<<6)  /* Targets (8 bit) */
#define CLI_ADDR_MAX (1<<14) /* Addresses (14 bits) */
#endif /* VTSS_ARCH_LUTON26/LUTON28/SERVAL */

#if defined(VTSS_ARCH_JAGUAR_1)
#define CLI_BLK_MAX  (1<<8)  /* Targets (8 bit) */
#define CLI_ADDR_MAX (1<<18) /* Addresses (14/18 bits) */
#endif /* VTSS_ARCH_JAGUAR_1 */
#define CLI_PHY_MAX  128  /* PHY register addresses */

static critd_t    crit; 
static int trace_grp_crit;
#define PHY_CLI_CRIT_ENTER() critd_enter(&crit, trace_grp_crit, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define PHY_CLI_CRIT_EXIT()  critd_exit( &crit, trace_grp_crit, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)

/* Ignore Lint problem with unprotected access. */
/*lint -sem(cli_phy_addr_list_parse, thread_protected) */
/*lint -sem(phy_cli_cmd_debug_reg, thread_protected) */
/*lint -sem(phy_cli_req_default_set, thread_protected) */
/*lint -sem(phy_cli_cmd_phy_10g_mode, thread_protected) */
/*lint -sem(phy_cli_cmd_channel_activate, thread_protected) */
/*lint -sem(phy_cli_cmd_phy_conf, thread_protected) */
/*lint -sem(phy_cli_cmd_phy_id, thread_protected) */
/*lint -sem(phy_cli_cmd_state_create, thread_protected) */
/*lint -sem(phy_cli_cmd_state_restart, thread_protected) */
/*lint -sem(phy_cli_cmd_channel_failover, thread_protected) */

uchar phy_addr_bf[VTSS_BF_SIZE(CLI_ADDR_MAX)];

typedef struct {
    /* Keywords */
    BOOL    detailed;
    BOOL    clear;
    BOOL    binary;
    BOOL    decimal;
    BOOL    add;
    BOOL    full;
    BOOL    enable;
    BOOL    disable;
    u32     lan;
    u32     cool;
    u32     inst;
    BOOL    module_all; 
    vtss_debug_layer_t layer;
    vtss_debug_group_t group;
} phy_cli_req_t;

/****************************************************************************/
/*  Default phy_req initialization function                                */
/****************************************************************************/
static void phy_cli_req_default_set(cli_req_t *req)
{
    phy_cli_req_t *phy_req = req->module_req;
    
    phy_req->module_all = 1;

    memset(phy_addr_bf, 0, sizeof(phy_addr_bf));
}

/****************************************************************************/
/*  Initialization                                                          */
/****************************************************************************/

void phy_cli_req_init(int a_trace_grp_crit)
{
    /* register the size required for port req. structure */
    cli_req_size_register(sizeof(phy_cli_req_t));

    trace_grp_crit = a_trace_grp_crit;

    critd_init(&crit, "phy_crit", VTSS_MODULE_ID_PHY, VTSS_MODULE_ID_PHY, CRITD_TYPE_MUTEX);
    PHY_CLI_CRIT_EXIT();
}


static void phy_cli_cmd_phy_id(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_phy_type_t         id1g;
    BOOL                    first = 1, phy10g=0;
    u32                     port_count = port_count_max();

#if defined(VTSS_CHIP_10G_PHY)
    vtss_phy_10g_id_t       id10g;
    memset(&id10g,0,sizeof(vtss_phy_10g_id_t));
#endif
    memset(&id1g,0,sizeof(vtss_phy_type_t));
    for (port_no = VTSS_PORT_NO_START; port_no < port_count; port_no++) {
        port = iport2uport(port_no);
        if (req->uport_list[port] == 0) 
            continue;

        if (vtss_phy_id_get(PHY_INST, port_no, &id1g) == VTSS_RC_OK && (id1g.part_number != 0)) {
            phy10g = 0;
        } 
#if defined(VTSS_CHIP_10G_PHY)
        else {
            if (!vtss_phy_10G_is_valid(PHY_INST, port_no)) {
                continue;
            } 
            phy10g = 1;
            if (vtss_phy_10g_id_get(PHY_INST, port_no, &id10g) != VTSS_RC_OK) {
                CPRINTF("Could not perform vtss_phy_10g_mode_get() operation");
                continue;
            }
        }
#endif
        if (first) {
            CPRINTF("%-6s %-10s %-10s %-10s %-10s\n","Port","Channel","API Base","Phy Id","Phy Rev.");
            CPRINTF("%-6s %-10s %-10s %-10s %-10s\n","----","-------","--------","-------","------");
            first = 0;
        }
#if defined(VTSS_CHIP_10G_PHY)        
        CPRINTF("%-6u %-10u %u %-8s %-10d %-10x\n",port,
                phy10g ? id10g.channel_id  : id1g.channel_id,
                phy10g ? id10g.phy_api_base_no : id1g.phy_api_base_no,phy10g?"(10g)":"(1g)",
                phy10g ? id10g.type            : id1g.part_number,
                phy10g ? id10g.revision        : id1g.revision);        
#else
        CPRINTF("%-6u %-10u %u %-8s %-10d %-10x\n",phy10g?0:port,id1g.channel_id,id1g.phy_api_base_no,"(1g)",id1g.part_number,id1g.revision);  
#endif
    }
}

static void phy_cli_cmd_phy_status(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    BOOL                    first = 1;
    u32                     port_count = port_count_max();
    for (port_no = VTSS_PORT_NO_START; port_no < port_count; port_no++) {
        port = iport2uport(port_no);
        if (req->uport_list[port] == 0) 
            continue;

        if (first) {
#if defined(VTSS_CHIP_10G_PHY)
            CPRINTF("%-6s %-35s %-20s\n", "Port", "Issues seen during 1G PHY warmstart", "Issues during 10G PHY WS");
            CPRINTF("%-6s %-35s %-20s\n", "----", "-----------------------------------", "------------------------");
#else
            CPRINTF("%-6s %-35s\n", "Port", "Issues seen during 1G PHY warmstart");
            CPRINTF("%-6s %-35s\n", "----", "-----------------------------------");
#endif
            first = 0;
        }
#if defined(VTSS_CHIP_10G_PHY)
        CPRINTF("%-6u %-35s %-20s\n", port, vtss_phy_warm_start_failed_get(PHY_INST, port_no) == VTSS_RC_ERROR ? "Yes" : "No", 
                vtss_phy_warm_start_10g_failed_get(PHY_INST, port_no) == VTSS_RC_ERROR ? "Yes" : "No");
#else
        CPRINTF("%-6u %-35s\n", port, vtss_phy_warm_start_failed_get(PHY_INST, port_no) == VTSS_RC_ERROR ? "Yes" : "No");
#endif
    }
}

static void phy_cli_cmd_phy_conf(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_phy_type_t         id1g;
    BOOL                    first = 1, phy10g=0;
    vtss_phy_conf_t         conf;
    BOOL                    link=0;
    vtss_port_status_t      stat_1g;
    u32                     port_count = port_count_max();
#if defined(VTSS_CHIP_10G_PHY)
    vtss_phy_10g_mode_t     mode;
    vtss_phy_10g_status_t   stat_10g;
    memset(&mode, 0, sizeof(vtss_phy_10g_mode_t));
#endif
    memset(&conf, 0, sizeof(vtss_phy_conf_t));
    for (port_no = VTSS_PORT_NO_START; port_no < port_count; port_no++) {
        port = iport2uport(port_no);
        if (req->uport_list[port] == 0) 
            continue;


        if (vtss_phy_id_get(PHY_INST, port_no, &id1g) == VTSS_RC_OK && (id1g.part_number != 0)) {
            phy10g = 0;
            if (vtss_phy_conf_get(PHY_INST, port_no, &conf) != VTSS_RC_OK) {
                CPRINTF("Could not perform vtss_phy_conf_get() operation");
                continue;
            }
            if  (vtss_phy_status_get(PHY_INST, port_no, &stat_1g)) {
                CPRINTF("Could not perform vtss_phy_status_get() operation");
                continue;
            }
            link = stat_1g.link;
        } 
#if defined(VTSS_CHIP_10G_PHY)
        else {
            if (!vtss_phy_10G_is_valid(PHY_INST, port_no)) {
                continue;
            }
            phy10g = 1;
            if (vtss_phy_10g_mode_get(PHY_INST, port_no, &mode) != VTSS_RC_OK) {
                CPRINTF("Could not perform vtss_phy_10g_mode_get() operation");
                continue;
            }
            if (vtss_phy_10g_status_get(PHY_INST, port_no, &stat_10g) != VTSS_RC_OK) {
                CPRINTF("Could not perform vtss_phy_10g_mode_get() operation");
                continue;
            }
            link = stat_10g.pma.rx_link && stat_10g.pcs.rx_link && stat_10g.xs.rx_link;
            
        }
#endif
                                                                        
        if (first) {
            CPRINTF("%-6s %-10s %-10s %-10s %-10s %-10s %-10s\n","Port","API Inst","WAN/LAN/1G","Mode","Duplex","Speed","Link");
            CPRINTF("%-6s %-10s %-10s %-10s %-10s %-10s %-10s\n","----","---------","---------","----","------","-----","----");
            first = 0;
        }
#if defined(VTSS_CHIP_10G_PHY)        
        CPRINTF("%-6u %-10s %-10s %-10s %-10s %-10s %-10s\n",port,
                PHY_INST==NULL?"Default":"PHY",
                phy10g ? (mode.oper_mode == VTSS_PHY_WAN_MODE ? "WAN" : mode.oper_mode == VTSS_PHY_LAN_MODE ? "LAN" : "1G"):"1G",
                phy10g ? "-" : (conf.mode == VTSS_PHY_MODE_ANEG ? "ANEG" : conf.mode == VTSS_PHY_MODE_FORCED ? "FORCED" : "PD"),
                phy10g ? "-" : (conf.mode != VTSS_PHY_MODE_ANEG ? (conf.forced.fdx ? "FDX":"HDX") : "-"),
                phy10g ? "-" : (conf.mode != VTSS_PHY_MODE_ANEG ? conf.forced.speed==VTSS_SPEED_10M ? "10M" : conf.forced.speed==VTSS_SPEED_100M ? "100M" : "1G" : "-"),link?"Yes":"No");
        
#else
        CPRINTF("%-6u %-10s %-10s %-10s %-10s %-10s,%-10s\n",phy10g?0:port,
                PHY_INST==NULL?"Default":"PHY",
                "1G",(conf.mode == VTSS_PHY_MODE_FORCED ? "FORCED" : "PD"), 
                (conf.mode != VTSS_PHY_MODE_ANEG ? (conf.forced.fdx ? "FDX":"HDX") : "-"),
                (conf.mode != VTSS_PHY_MODE_ANEG ? conf.forced.speed==VTSS_SPEED_10M ? "10M" : 
                 conf.forced.speed==VTSS_SPEED_100M ? "100M" : "1G" : "-"),link?"Yes":"No");

#endif
    }
}

static const char *cli_restart_txt(vtss_restart_t restart)
{
    return (restart == VTSS_RESTART_COLD ? "Cold" :
            restart == VTSS_RESTART_COOL ? "Cool" :
            restart == VTSS_RESTART_WARM ? "Warm" : "?");
}

static void phy_cli_cmd_state_restart(cli_req_t *req)
{
    phy_cli_req_t          *phy_req = req->module_req;
    vtss_restart_status_t  status;
    vtss_restart_t         restart;
    vtss_init_conf_t       init_conf;
  
    if (req->set) {
        if (PHY_INST == NULL) {
            CPRINTF("You can't restart the default switch instance.  Did you forget to create a PHY instance?\n");
            return;
        }
        if (phy_mgmt_inst_restart(PHY_INST, phy_req->cool?COOL:WARM) != VTSS_RC_OK) {
            CPRINTF("Could not restart the API instance\n");
        } else {
            CPRINTF("%s restart completed successfully\n",phy_req->cool?"COOL":"WARM");
        }
    } else {       
        if (vtss_init_conf_get(PHY_INST, &init_conf) != VTSS_RC_OK) {
            CPRINTF("Failed Could vtss_init_conf_get() operation");
        }
        if (vtss_restart_conf_get(PHY_INST, &restart) == VTSS_RC_OK)
            CPRINTF("Next Restart    : %s\n", cli_restart_txt(restart));
        if (vtss_restart_status_get(PHY_INST, &status) == VTSS_RC_OK) {
            CPRINTF("Previous Restart: %s\n", cli_restart_txt(status.restart));
            CPRINTF("Current API Version : %u\n", status.cur_version);
            CPRINTF("Previous API Version: %u\n", status.prev_version);
        }
        if (vtss_restart_conf_get(PHY_INST, &restart) == VTSS_RC_OK) {
            CPRINTF("Phy Instance Restart Source:%s\n",init_conf.restart_info_src == VTSS_RESTART_INFO_SRC_10G_PHY ? "10G" : "1G");
            CPRINTF("Phy Instance Restart Port:%u\n",init_conf.restart_info_port);
        }
    } 
}


static void phy_cli_cmd_state_create(cli_req_t *req)
{
    phy_cli_req_t          *phy_req = req->module_req;

    if (req->set) {
        if (phy_mgmt_inst_create(phy_req->inst==10 ? PHY_INST_10G_PHY : phy_req->lan==2 ? PHY_INST_1G_PHY : PHY_INST_NONE) != VTSS_RC_OK) {
            CPRINTF("Could not restart inst\n");
        } else {
            CPRINTF("The instance will be created/destroyed after reboot\n");
        }
    } else {

    }
}
#if defined(VTSS_CHIP_10G_PHY)
static void phy_cli_cmd_phy_instance_default(cli_req_t *req)
{
   
    if (phy_mgmt_inst_activate_default() != VTSS_RC_OK)
        CPRINTF("Could not set phy_mgmt_inst_activate_default()\n");
    else
        CPRINTF("Default instance successfully activated\n");
}


static void phy_cli_cmd_channel_failover(cli_req_t *req)
{
    vtss_port_no_t               port, port_no;
    vtss_phy_10g_failover_mode_t phy_failover, api_failover;
    vtss_phy_10g_id_t            phy_id;
    BOOL                         first = 1, active, next, bc;
    phy_cli_req_t                *phy_req = req->module_req;
    u32                          port_count = port_count_max();

    for (port_no = VTSS_PORT_NO_START; port_no < port_count; port_no++) {
        port = iport2uport(port_no);
        if ((req->uport_list[port] == 0) || !vtss_phy_10G_is_valid(PHY_INST, port_no)) 
            continue;

        if (vtss_phy_10g_id_get(PHY_INST, port_no, &phy_id) != VTSS_RC_OK) {
            T_E("Could not get the Phy id for port: %u",port_no);
            continue;
        }

        if (phy_id.part_number != 0x8487) { 
            if (req->set) {
                CPRINTF("This function only applies to VSC8487 (you are addressing 0x%x)\n",phy_id.part_number);
            }
            continue;
        }

        if (vtss_phy_10g_failover_get(PHY_INST, port_no, &api_failover) != VTSS_RC_OK) {
            CPRINTF("Could not get failover\n");
        }

        if (req->set) {
            if (phy_req->enable) {
                if (api_failover == VTSS_PHY_10G_PMA_0_TO_FROM_XAUI_0_TO_XAUI_1) {
                    phy_failover = VTSS_PHY_10G_PMA_0_TO_FROM_XAUI_1_TO_XAUI_0;
                } else {
                    phy_failover = VTSS_PHY_10G_PMA_0_TO_FROM_XAUI_0_TO_XAUI_1;
                }
            }
            if (phy_mgmt_failover_set(port_no, &phy_failover) != VTSS_RC_OK) {
                CPRINTF("Could not get failover\n");
            }


        } else {
            if (phy_mgmt_failover_get(port_no, &phy_failover) != VTSS_RC_OK) {
                CPRINTF("Could not get failover\n");
            }
            if (first) {
                CPRINTF("%-12s %-10s %-10s %-10s %-10s\n","Port","Active","Channel","Broadcast","After reset");
                CPRINTF("%-12s %-10s %-10s %-10s %-10s\n","----","------","-------","---------","------------");;
                first = 0;
            }
            active = 0;
            next = 0;
            bc = 0;
            if ((phy_id.channel_id == 0) && (api_failover == VTSS_PHY_10G_PMA_TO_FROM_XAUI_NORMAL)) {
                active = 1;
            }

            if (((phy_id.channel_id == 0) && (api_failover == VTSS_PHY_10G_PMA_0_TO_FROM_XAUI_0_TO_XAUI_1)) ||
                ((phy_id.channel_id == 1) && (api_failover == VTSS_PHY_10G_PMA_0_TO_FROM_XAUI_1_TO_XAUI_0))) {
                active = 1;
                bc = 1;
            }
            if (((phy_id.channel_id == 0) && (phy_failover == VTSS_PHY_10G_PMA_0_TO_FROM_XAUI_0_TO_XAUI_1)) ||
               (((phy_id.channel_id == 1) && (phy_failover == VTSS_PHY_10G_PMA_0_TO_FROM_XAUI_1_TO_XAUI_0)))) {
                next = 1;
                bc = 1;
            }
            CPRINTF("%-12u %-10s %-10d %-10s %-10s\n",port, active ? "Yes" : "No", phy_id.channel_id, bc ? "Yes" : "No", next ? "Yes" : "No");
        }
    }
}
#endif
/****************************************************************************/
/*  Parameter functions                                                     */
/****************************************************************************/
#if defined(VTSS_CHIP_10G_PHY)
static int32_t cli_phy_phy_channel_parse(char *cmd, char *cmd2, char *stx,
                                          char *cmd_org, cli_req_t *req)
{
    return cli_parse_ulong(cmd, &req->value, 0, 1);
}
#endif /* defined(VTSS_CHIP_10G_PHY) */

static int32_t cli_phy_keyword_parse(char *cmd, char *cmd2, char *stx,
                                      char *cmd_org, cli_req_t *req)
{
    phy_cli_req_t *phy_req = req->module_req;
    char           *found = cli_parse_find(cmd, stx);

    if (found != NULL) {
        if (!strncmp(found, "ail", 3)) {
            phy_req->layer = VTSS_DEBUG_LAYER_AIL;
        } else if (!strncmp(found, "binary", 6)) {
            phy_req->binary = 1;
        } else if (!strncmp(found, "clear", 5)) {
            phy_req->clear = 1;
        } else if (!strncmp(found, "cil", 3)) {
            phy_req->layer = VTSS_DEBUG_LAYER_CIL;
        } else if (!strncmp(found, "detailed", 8)) {
            phy_req->detailed = 1;
        } else if (!strncmp(found, "decimal", 7)) {
            phy_req->decimal = 1;
        } else if (!strncmp(found, "full", 4)) {
            phy_req->full = 1;
        } else if (!strncmp(found, "disable", 7)) {
            phy_req->disable = 1;
        } else if (!strncmp(found, "enable", 6)) {
            phy_req->enable = 1;
        } else if (!strncmp(found, "wan", 3)) {
            phy_req->lan = 0;
        } else if (!strncmp(found, "lan", 3)) {
            phy_req->lan = 1;
        } else if (!strncmp(found, "1g", 2)) {
            phy_req->lan = 2;
        } else if (!strncmp(found, "cool", 4)) {
            phy_req->cool = 1;
        } else if (!strncmp(found, "10g", 3)) {
            phy_req->inst = 10;
        }
    }
    return (found == NULL ? 1 : 0);   
}

static int32_t cli_phy_port_list_parse(char *cmd, char *cmd2, char *stx,
                                        char *cmd_org, cli_req_t *req)
{
    int32_t error;
    
    if ((error = cli_parse_none(cmd)) == 0)
        (void)cli_parm_parse_list(NULL, req->uport_list, 1, VTSS_PORTS, 0);
    else
        error = cli_parm_parse_list(cmd, req->uport_list, 1, VTSS_PORTS, 1);
    return error;
}

/****************************************************************************/
/*  Parameter table                                                         */
/****************************************************************************/

static cli_parm_t phy_cli_parm_table[] = {
    {
        "<port_list>",
        "Port list or or 'none'",
        CLI_PARM_FLAG_NONE,
        cli_phy_port_list_parse,
        NULL
    },
#if defined(VTSS_FEATURE_10G)
    {
        "lan|wan|1g",
        "lan     : LAN mode\n"
        "wan     : WAN mode\n"
        "1g      : 1g mode\n"
        "(default: Show mode)",
        CLI_PARM_FLAG_SET,
        cli_phy_keyword_parse,

    },
#endif
    {
        "enable|disable",
        "enable     : Enable \n"
        "disable    : Disable \n"
        "(default: Show )\n",
        CLI_PARM_FLAG_SET,
        cli_phy_keyword_parse,
    },
#if defined(VTSS_CHIP_10G_PHY)
    {
        "<channel>",
        "Channel 0-1",
        CLI_PARM_FLAG_SET,
        cli_phy_phy_channel_parse,

    },
#endif
    {
        "cool|warm",
        "cool    : Restart API incl. HW\n"
        "warm    : Only restart State\n",
        CLI_PARM_FLAG_SET,
        cli_phy_keyword_parse,

    },
    {
        "1g|10g|none",
        "1g    : 1g PHY instance"
        "10g   : 10G PHY instance\n"
        "none  : Instance will be destroyed\n",
        CLI_PARM_FLAG_SET,
        cli_phy_keyword_parse,

    },
    {
        NULL,
        NULL,
        0,
        0,
        NULL
    },
};

/****************************************************************************/
/*  Command table                                                           */
/****************************************************************************/

enum {
  PRIO_PHY_10G_MODE = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_PHY_10G_FW_LOAD = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_PHY_STATE_RESTART = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_PHY_CHANNEL_ACTIVATE = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_PHY_STATE_CREATE = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_PHY_ID = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_PHY_CONF = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_PHY_DEFAULT_ACTIVATE = CLI_CMD_SORT_KEY_DEFAULT,
#if defined(VTSS_SW_OPTION_BOARD)
  PRIO_DEBUG_INTERRUPT_HOOK = CLI_CMD_SORT_KEY_DEFAULT,
#endif /* VTSS_SW_OPTION_BOARD */
};

cli_cmd_tab_entry (
  "Phy Id [<port_list>]",
  NULL,
  "Phy configuration",
  PRIO_PHY_ID,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_PHY,
  phy_cli_cmd_phy_id,
  phy_cli_req_default_set,
  phy_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "Phy Configuration [<port_list>]",
  NULL,
  "Phy configuration",
  PRIO_PHY_CONF,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_PHY,
  phy_cli_cmd_phy_conf,
  phy_cli_req_default_set,
  phy_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "Phy Status [<port_list>]",
  NULL,
  "Phy warmstart status",
  PRIO_PHY_CONF,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_PHY,
  phy_cli_cmd_phy_status,
  phy_cli_req_default_set,
  phy_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "Phy Instance Restart [cool|warm] ",
  NULL,
  "Restart the API Instance",
  PRIO_PHY_STATE_RESTART,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_PHY,
  phy_cli_cmd_state_restart,
  phy_cli_req_default_set,
  phy_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "Phy Instance Create [1g|10g|none] ",
  NULL,
  "Create/destroy a PHY instance after reboot",
  PRIO_PHY_STATE_CREATE,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_PHY,
  phy_cli_cmd_state_create,
  phy_cli_req_default_set,
  phy_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

#if defined(VTSS_CHIP_10G_PHY)
cli_cmd_tab_entry (
  "Phy Failover [<port_list>] [enable|disable] ",
  NULL,
  "Change to the other channel after instance restart (applies to VSC8487)",
  PRIO_PHY_CHANNEL_ACTIVATE,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_PHY,
  phy_cli_cmd_channel_failover,
  phy_cli_req_default_set,
  phy_cli_parm_table,
  CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry (
  "Phy Instance Default Activate",
  NULL,
  "Phy configuration",
  PRIO_PHY_DEFAULT_ACTIVATE,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_PHY,
  phy_cli_cmd_phy_instance_default,
  phy_cli_req_default_set,
  phy_cli_parm_table,
  CLI_CMD_FLAG_NONE
);
#endif


