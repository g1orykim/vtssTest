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

 $Id$
 $Revision$

*/

#include "main.h"
#include "conf_api.h"
#include "conf_cli.h"
#include "port_custom_api.h" /* For vtss_board_name/type() */
#include "cli_trace_def.h"
#include "cli.h"

typedef struct {
    ulong          ip_server;
    BOOL           board_id_valid;
    ulong          board_id;
    BOOL           board_type_valid;
    ulong          board_type;
} conf_cli_req_t;

/****************************************************************************/
/*  Initialization                                                          */
/****************************************************************************/

void conf_cli_init(void)
{
    /* register the size required for port req. structure */
    cli_req_size_register(sizeof(conf_cli_req_t));
}

/****************************************************************************/
/*  Command functions                                                       */
/****************************************************************************/

/* Debug board */
static void cli_cmd_debug_board(cli_req_t *req)
{
    conf_board_t conf;
    uint         i;
    char         buf[32];
    conf_cli_req_t *conf_req = req->module_req;

    if (conf_mgmt_board_get(&conf) < 0)
        return;
    if (req->set) {
        if (req->mac_addr_spec == CLI_SPEC_VAL) {
            for (i = 0; i < 6; i++)
                conf.mac_address[i] = req->mac_addr[i];
        }
        if (conf_req->board_id_valid)
            conf.board_id = conf_req->board_id;
        if (conf_req->board_type_valid)
            conf.board_type = conf_req->board_type;
        if (conf_mgmt_board_set(&conf) != VTSS_OK)
            CPRINTF("Board set operation failed\n");
    } else {
        CPRINTF("Board MAC Address: %s\n", misc_mac_txt(conf.mac_address, buf));
        CPRINTF("Board ID         : %d\n", conf.board_id);
        CPRINTF("Board Type Conf  : %d\n", conf.board_type);
        CPRINTF("Board Type Active: %s (%d)\n", vtss_board_name(), vtss_board_type());
    }
}

/* Debug configuration blocks */
static void cli_cmd_debug_conf_blocks(cli_req_t *req)
{
    conf_sec_t      sec;
    conf_sec_info_t info;
    conf_mgmt_blk_t blk;
    conf_blk_id_t   id;
    BOOL            first = 0;
    ulong           size = 0;

    for (sec = CONF_SEC_LOCAL; sec < CONF_SEC_CNT; sec++) {
        if (req->clear) {
            conf_sec_renew(sec);
            continue;
        }
        
        cli_header(sec == CONF_SEC_LOCAL ? "Local Section" : "Global Section", 1);

        first = 1;
        size = 0;
        id = CONF_BLK_CONF_HDR;
        while (conf_mgmt_sec_blk_get(sec, id, &blk, 1) == VTSS_OK) {
            if (first)
                cli_table_header("ID   Data        Size    CRC32       Changes  Name  ");
            first = 0;
            CPRINTF("%-3d  0x%08x  %-6d  0x%08x  %-7d  %s\n",
                    blk.id, (ulong)blk.data, blk.size, blk.crc, blk.change_count, blk.name);
            id = blk.id;
            size += blk.size;
        }
        CPRINTF("%sTotal size: %d\n", first ? "" : "\n", size);
        conf_sec_get(sec, &info);
        CPRINTF("Save count: %d\n", info.save_count);
    }
}

// #what == 0 => stack_copy
// #what == 1 => flash_save
// #what == 2 => change detect
static void cli_cmd_conf(cli_req_t *req, int what)
{
    conf_mgmt_conf_t conf;

    if (conf_mgmt_conf_get(&conf) != VTSS_OK)
        return;

    if (req->set) {
        switch (what) {
        case 0:
            conf.stack_copy = req->enable;
            break;

        case 1:
            conf.flash_save = req->enable;
            break;

        case 2:
            conf.change_detect = req->enable;
            break;

        default:
            CPRINTF("Error: Unknown action %d", what);
            break;
        }
        if (conf_mgmt_conf_set(&conf) != VTSS_OK) {
            CPRINTF("conf_set failed\n");
        }
    } else {
        CPRINTF("Stack Copy   : %s\n", cli_bool_txt(conf.stack_copy));
        CPRINTF("Flash Save   : %s\n", cli_bool_txt(conf.flash_save));
        CPRINTF("Change Detect: %s\n", cli_bool_txt(conf.change_detect));
    }
}

/* Debug Stack copy mode */
static void cli_cmd_debug_conf_stack_copy(cli_req_t *req)
{
    cli_cmd_conf(req, 0);
}

/* Debug Flash save mode */
static void cli_cmd_debug_conf_flash_save(cli_req_t *req)
{
    cli_cmd_conf(req, 1);
}

/* Debug Stack copy mode */
static void cli_cmd_debug_conf_change_detect(cli_req_t *req)
{
    cli_cmd_conf(req, 2);
}


/****************************************************************************/
/*  Parameter functions                                                     */
/****************************************************************************/

static int32_t cli_conf_mac_addr_parse (char *cmd, char *cmd2, char *stx, char *cmd_org,
        cli_req_t *req)
{
    req->parm_parsed = 1;
    return cli_parse_mac(cmd, req->mac_addr, &req->mac_addr_spec, 0);
}


static int32_t cli_conf_board_id_parse (char *cmd, char *cmd2, char *stx, char *cmd_org,
        cli_req_t *req)
{
    int32_t error = 0;
    conf_cli_req_t *conf_req = req->module_req;

    req->parm_parsed = 1;
    if ((error = cli_parse_ulong(cmd, &conf_req->board_id, 0, 0xffffffff)) == 0)
        conf_req->board_id_valid = 1;

    return error;
}

static int32_t cli_conf_board_type_parse (char *cmd, char *cmd2, char *stx, char *cmd_org,
                                          cli_req_t *req)
{
    int32_t error;
    conf_cli_req_t *conf_req = req->module_req;

    req->parm_parsed = 1;
    if ((error = cli_parse_ulong(cmd, &conf_req->board_type, 0, 1024)) == 0)
        conf_req->board_type_valid = TRUE;

    return error;
}

/****************************************************************************/
/*  Parameter table                                                         */
/****************************************************************************/

static cli_parm_t conf_cli_parm_table[] = {
    {
        "<mac_addr>",
        "Board MAC address ('xx-xx-xx-xx-xx-xx' or 'xx.xx.xx.xx.xx.xx' or 'xxxxxxxxxxxx', x is a hexadecimal digit), default: Show MAC address",
        CLI_PARM_FLAG_SET,
        cli_conf_mac_addr_parse,
        cli_cmd_debug_board
    },
    {
        "clear",
        "clear: Clear flash save count",
        CLI_PARM_FLAG_NO_TXT,
        cli_parm_parse_keyword,
        cli_cmd_debug_conf_blocks
    },
    {
        "<board_id>",
        "Board ID, default: Show board ID",
        CLI_PARM_FLAG_SET,
        cli_conf_board_id_parse,
        NULL
    },
    {
        "<board_type>",
        "Board ID, default: Show board Type",
        CLI_PARM_FLAG_SET,
        cli_conf_board_type_parse,
        NULL
    },
    {
        "enable|disable",
        "enable  : Enable Flash saving\n"
        "disable : Disable Flash saving\n"
        "(default: Show Flash save mode)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        cli_cmd_debug_conf_flash_save
    },
    {
        "enable|disable",
        "enable  : Enable stack copying\n"
        "disable : Disable stack copying\n"
        "(default: Show stack copy mode)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        cli_cmd_debug_conf_stack_copy
    },
    {
        "enable|disable",
        "enable  : Enable change detection\n"
        "disable : Disable change detection\n"
        "(default: Show change detection mode)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        cli_cmd_debug_conf_change_detect
    },

    {NULL, NULL, 0, 0, NULL}
};

/****************************************************************************/
/*  Command table                                                           */
/****************************************************************************/

enum {
    PRIO_DEBUG_BOARD = CLI_CMD_SORT_KEY_DEFAULT,
    PRIO_DEBUG_CONF_BLOCKS = CLI_CMD_SORT_KEY_DEFAULT,
    PRIO_DEBUG_CONF_FLASH_SAVE = CLI_CMD_SORT_KEY_DEFAULT,
    PRIO_DEBUG_CONF_STACK_COPY = CLI_CMD_SORT_KEY_DEFAULT,
};

cli_cmd_tab_entry (
    "Debug Board",
    "Debug Board [<mac_addr>] [<board_id>] [<board_type>]",
    "Set or show board MAC address and ID",
    PRIO_DEBUG_BOARD,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_board,
    NULL,
    conf_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "Debug Configuration Blocks",
    "Debug Configuration Blocks [clear]",
    "Show configuration blocks or clear section save counter",
    PRIO_DEBUG_CONF_BLOCKS,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_conf_blocks,
    NULL,
    conf_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "Debug Configuration Flash",
    "Debug Configuration Flash [enable|disable]",
    "Set or show the Flash save mode",
    PRIO_DEBUG_CONF_FLASH_SAVE,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_conf_flash_save,
    NULL,
    conf_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "Debug Configuration Stack",
    "Debug Configuration Stack [enable|disable]",
    "Set or show the stack copy mode",
    PRIO_DEBUG_CONF_STACK_COPY,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_conf_stack_copy,
    NULL,
    conf_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "Debug Configuration ChangeDetect",
    "Debug Configuration ChangeDetect [enable|disable]",
    "Set or show the change detect mode",
    CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_conf_change_detect,
    NULL,
    conf_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

