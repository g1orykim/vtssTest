/*

 Vitesse Switch API software.

 Copyright (c) 2002-2012 Vitesse Semiconductor Corporation "Vitesse". All
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
#if VTSS_SWITCH_STACKABLE
#include "topo_api.h"
#endif

#include "zl_3034x_api_api.h"
#include "zl_3034x_api_cli.h"
#include "vtss_module_id.h"
#include "cli_trace_def.h"

typedef struct
{
    BOOL  enable;
    u32   mem_page;
    u32   mem_addr;
    u32   mem_size;
    u32   value;
    u32   log_level;
    BOOL  stats_reset;
    u32   apr_mode;
    u32   apr_adj;
} zl_3034x_cli_req_t;

void zl_3034x_api_cli_req_init(void)
{
    /* register the size required for mep req. structure */
    cli_req_size_register(sizeof(zl_3034x_cli_req_t));
}

static void zl_3034x_cli_def_req (cli_req_t * req)
{
    zl_3034x_cli_req_t *zl_3034x_req = NULL;
  
    zl_3034x_req = req->module_req;

    zl_3034x_req->enable = FALSE;
    zl_3034x_req->mem_size = 1;
    zl_3034x_req->stats_reset = FALSE;
}

static int cli_parm_parse_refid(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int error = 0;
    zl_3034x_cli_req_t *zl_3034x_req = req->module_req;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &zl_3034x_req->value, 0, 8);
    return(error);
}

static int cli_parm_parse_pllid(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int error = 0;
    zl_3034x_cli_req_t *zl_3034x_req = req->module_req;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &zl_3034x_req->value, 1, 2);
    return(error);
}

static int cli_parm_parse_my_mem_addr(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int error = 0;
    zl_3034x_cli_req_t *zl_3034x_req = req->module_req;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &zl_3034x_req->mem_addr, 0, 0xffffffff);
    return(error);
}

static int cli_parm_parse_mem_page(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int error = 0;
    zl_3034x_cli_req_t *zl_3034x_req = req->module_req;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &zl_3034x_req->mem_page, 0, 0xffffffff);
    return(error);
}

static int cli_parm_parse_mem_size(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int error = 0;
    zl_3034x_cli_req_t *zl_3034x_req = req->module_req;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &zl_3034x_req->mem_size, 0, 0x7F);
    return(error);
}

static int cli_parm_parse_value(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int error = 0;
    zl_3034x_cli_req_t *zl_3034x_req = req->module_req;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &zl_3034x_req->value, 0, 0xffffffff);
    return(error);
}

static void cli_cmd_reg_read(cli_req_t *req)
{
    u32 i, n;
    u32 value;
    zl_3034x_cli_req_t   *zl_3034x_req = req->module_req;

    for (i=0; i<zl_3034x_req->mem_size; i++)
    {
        (void)zl_3034x_register_get(zl_3034x_req->mem_page,  zl_3034x_req->mem_addr+i,  &value);

        n = (i & 0x7);
        if (n == 0)   CPRINTF("%03X: ", i + zl_3034x_req->mem_addr);
        CPRINTF("%02x ", value);
        if (n == 0x7 || i == (zl_3034x_req->mem_size - 1))  CPRINTF("\n");
    }
}

static void cli_cmd_reg_write(cli_req_t *req)
{
    zl_3034x_cli_req_t   *zl_3034x_req = req->module_req;

   (void)zl_3034x_register_set(zl_3034x_req->mem_page,  zl_3034x_req->mem_addr,  zl_3034x_req->value);
}


static void cli_cmd_pll_status(cli_req_t *req)
{
    if (zl_3034x_debug_pll_status() != VTSS_RC_OK) {
        CPRINTF("ZL pll cfg get failed!\n");
    }
}

static void cli_cmd_hw_ref_status(cli_req_t *req)
{
    zl_3034x_cli_req_t *zl_3034x_req = req->module_req;
    if (zl_3034x_debug_hw_ref_status(zl_3034x_req->value) != VTSS_RC_OK) {
        CPRINTF("ZL hw ref status get failed!\n");
    }
}

static void cli_cmd_hw_ref_cfg(cli_req_t *req)
{
    zl_3034x_cli_req_t *zl_3034x_req = req->module_req;
    if (zl_3034x_debug_hw_ref_cfg(zl_3034x_req->value) != VTSS_RC_OK) {
        CPRINTF("ZL hw ref cfg get failed!\n");
    }
}

static void cli_cmd_dpll_status(cli_req_t *req)
{
    zl_3034x_cli_req_t *zl_3034x_req = req->module_req;
    if (zl_3034x_debug_dpll_status(zl_3034x_req->value) != VTSS_RC_OK) {
        CPRINTF("ZL dpll status get failed!\n");
    }
}

static void cli_cmd_dpll_cfg(cli_req_t *req)
{
    zl_3034x_cli_req_t *zl_3034x_req = req->module_req;
    if (zl_3034x_debug_dpll_cfg(zl_3034x_req->value) != VTSS_RC_OK) {
        CPRINTF("ZL dpll cfg get failed!\n");
    }
}


static cli_parm_t zl_3034x_cli_parm_table[] = {
    {
        "<refid>",
        "sync reference input (0..8)",
        CLI_PARM_FLAG_NONE | CLI_PARM_FLAG_SET,
        cli_parm_parse_refid,
        NULL
    },
    {
        "<pllid>",
        "pll number 1: DPLL1 used for Synce Clock, 2: DPLL2 used for Station clock output or PTP clock",
        CLI_PARM_FLAG_NONE | CLI_PARM_FLAG_SET,
        cli_parm_parse_pllid,
        NULL
    },
    {
        "<mem_page>",
        "Register memory page",
        CLI_PARM_FLAG_NONE | CLI_PARM_FLAG_SET,
        cli_parm_parse_mem_page,
        NULL
    },
    {
        "<mem_addr>",
        "Register memory address",
        CLI_PARM_FLAG_NONE | CLI_PARM_FLAG_SET,
        cli_parm_parse_my_mem_addr,
        NULL
    },
    {
        "<mem_size>",
        "Register memory size",
        CLI_PARM_FLAG_NONE | CLI_PARM_FLAG_SET,
        cli_parm_parse_mem_size,
        NULL
    },
    {
        "<value>",
        "Register value",
        CLI_PARM_FLAG_NONE | CLI_PARM_FLAG_SET,
        cli_parm_parse_value,
        NULL
    },
};

enum
{
  PRIO_APR_LOGGING,
  PRIO_APR_READ_REG,
  PRIO_APR_WRITE_REG,
  PRIO_APR_TEST1,
  PRIO_APR_DEVICE_STATUS,
  PRIO_APR_SERVER_CONFIG,
  PRIO_APR_SERVER_STATUS,
  PRIO_APR_FORCE_HOLDOVER,
  PRIO_APR_STATISTICS,
  PRIO_APR_SERVER_NOTIFY,
  PRIO_APR_SERVER_ONE_HZ,
  PRIO_APR_SERVER_MODE,
  PRIO_APR_CONFIG_DUMP
};

/* Command table entries */
cli_cmd_tab_entry(
    "ZL3034X Read ",
    "ZL3034X Read <mem_page> <mem_addr> [<mem_size>]",
    "Read Zarlink register",
    PRIO_APR_READ_REG,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_ZL_3034X_API,
    cli_cmd_reg_read,
    zl_3034x_cli_def_req,
    zl_3034x_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    "ZL3034X Write ",
    "ZL3034X Write <mem_page> <mem_addr> <value>",
    "Read Zarlink register",
    PRIO_APR_WRITE_REG,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_ZL_3034X_API,
    cli_cmd_reg_write,
    zl_3034x_cli_def_req,
    zl_3034x_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "ZL3034X PLL Status",
  NULL,
  "Show the PLL status",
  PRIO_APR_SERVER_CONFIG,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_ZL_3034X_API,
  cli_cmd_pll_status,
  zl_3034x_cli_def_req,
  zl_3034x_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "ZL3034X HW ref status",
    "ZL3034X HW ref status <refid>",
    "The HW ref status information",
    PRIO_APR_SERVER_STATUS,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_ZL_3034X_API,
    cli_cmd_hw_ref_status,
    zl_3034x_cli_def_req,
    zl_3034x_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry (
    "ZL3034X HW ref cfg",
    "ZL3034X HW ref cfg <refid>",
    "The HW ref config information",
    PRIO_APR_SERVER_STATUS,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_ZL_3034X_API,
    cli_cmd_hw_ref_cfg,
    zl_3034x_cli_def_req,
    zl_3034x_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "ZL3034X DPLL status",
    "ZL3034X DPLL status <pllid>",
    "The DPLL status information",
    PRIO_APR_SERVER_STATUS,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_ZL_3034X_API,
    cli_cmd_dpll_status,
    zl_3034x_cli_def_req,
    zl_3034x_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry (
    "ZL3034X DPLL cfg",
    "ZL3034X DPLL cfg <pllid>",
    "The DPLL config information",
    PRIO_APR_SERVER_STATUS,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_ZL_3034X_API,
    cli_cmd_dpll_cfg,
    zl_3034x_cli_def_req,
    zl_3034x_cli_parm_table,
    CLI_CMD_FLAG_NONE
);



