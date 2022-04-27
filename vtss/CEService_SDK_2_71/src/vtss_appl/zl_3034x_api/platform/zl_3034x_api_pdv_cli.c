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

#include "zl_3034x_api_pdv_api.h"
#include "zl_3034x_api_pdv_cli.h"
//#include "zl_3034x_pdv.h"
#include "vtss_module_id.h"
#include "cli_trace_def.h"
#include "zl303xx_LogToMsgQ.h"

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

void zl_3034x_pdv_cli_req_init(void)
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

static int cli_parse_enable (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char *found = cli_parse_find(cmd, stx);
    zl_3034x_cli_req_t *zl_3034x_req = req->module_req;

    if(!found)      return 1;
    else if(!strncmp(found, "enable", 6))       zl_3034x_req->enable = TRUE;
    else if(!strncmp(found, "disable", 7))      zl_3034x_req->enable = FALSE;
    else return 1;

    return 0;
}

static int cli_parm_parse_log_level(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int error = 0;
    zl_3034x_cli_req_t *zl_3034x_req = req->module_req;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &zl_3034x_req->log_level, 0, 3);
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

static int cli_parse_reset(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char *found = cli_parse_find(cmd, stx);
    zl_3034x_cli_req_t *zl_3034x_req = req->module_req;

    if(!found)      return 1;
    else if(!strncmp(found, "reset", 5))       zl_3034x_req->stats_reset = TRUE;
    else return 1;

    return 0;
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

static int cli_parm_parse_apr_mode(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int error = 0;
    zl_3034x_cli_req_t *zl_3034x_req = req->module_req;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &zl_3034x_req->apr_mode, 0, 9);
    return(error);
}

static int cli_parm_parse_apr_adj(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int error = 0;
    zl_3034x_cli_req_t *zl_3034x_req = req->module_req;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &zl_3034x_req->apr_adj, 0, 100);
    return(error);
}

static void cli_cmd_apr_logging(cli_req_t *req)
{
    zl_3034x_cli_req_t   *zl_3034x_req = req->module_req;
    u8 level;
    if (req->set)
    {
        if (zl_3034x_req->enable)
            (void)zl303xx_ReEnableLogToMsgQ();
        else
            (void)zl303xx_DisableLogToMsgQ();
        
        if (zl_3034x_apr_log_level_set(zl_3034x_req->log_level) != VTSS_RC_OK) {
            CPRINTF("APR log level set failed!\n");
        }
    }
    else
    {
        if (zl_3034x_apr_log_level_get(&level) != VTSS_RC_OK) {
            CPRINTF("APR log level get failed!\n");
        }
        CPRINTF("APR log enabled %s, log level %d\n", "?", level);
    }
}

static void cli_cmd_apr_ts_logging(cli_req_t *req)
{
    zl_3034x_cli_req_t   *zl_3034x_req = req->module_req;
    u8 level;
    if (req->set)
    {
        
        if (zl_3034x_apr_ts_log_level_set(zl_3034x_req->log_level) != VTSS_RC_OK) {
            CPRINTF("APR ts log level set failed!\n");
        }
    }
    else
    {
        if (zl_3034x_apr_ts_log_level_get(&level) != VTSS_RC_OK) {
            CPRINTF("APR tslog level get failed!\n");
        }
        CPRINTF("APR tslog level %d\n", level);
    }
}

static void cli_cmd_apr_dev_status(cli_req_t *req)
{
    if (zl_3034x_apr_device_status_get() != VTSS_RC_OK) {
        CPRINTF("Device status get failed!\n");
    }
}

static void cli_cmd_apr_server_config(cli_req_t *req)
{
    if (zl_3034x_apr_server_config_get() != VTSS_RC_OK) {
        CPRINTF("APR server config get failed!\n");
    }
}

static void cli_cmd_apr_server_status(cli_req_t *req)
{
    if (zl_3034x_apr_server_status_get() != VTSS_RC_OK) {
        CPRINTF("APR server status get failed!\n");
    }
}

static void cli_cmd_apr_force_holdover(cli_req_t *req)
{
    zl_3034x_cli_req_t   *zl_3034x_req = req->module_req;
    BOOL value;
    if (req->set) {
        if (zl_3034x_apr_force_holdover_set(zl_3034x_req->enable) != VTSS_RC_OK) {
            CPRINTF("APR force holdover setting failed!\n");
        }
    } else {
        /* TODO: show holdover status: no function found */
        if (zl_3034x_apr_force_holdover_get(&value) != VTSS_RC_OK) {
            CPRINTF("APR force holdover setting failed!\n");
        } else {
            CPRINTF("APR force holdover: %s\n", ((value == TRUE) ? "enable": "disable"));
        }
    }
}

static void cli_cmd_apr_statistics(cli_req_t *req)
{
    zl_3034x_cli_req_t   *zl_3034x_req = req->module_req;

    if (zl_3034x_req->stats_reset) {
        if (zl_3034x_apr_statistics_reset() != VTSS_RC_OK) {
            CPRINTF("APR statistics reset failed!\n");
        }
    } else {
        if (zl_3034x_apr_statistics_get() != VTSS_RC_OK) {
            CPRINTF("APR statistics get failed!\n");
        }
    }
}

static void cli_cmd_apr_server_notify(cli_req_t *req)
{
    zl_3034x_cli_req_t   *zl_3034x_req = req->module_req;
    BOOL value = FALSE;

    if (req->set) {
        if (apr_server_notify_set(zl_3034x_req->enable) != VTSS_RC_OK) {
            CPRINTF("APR server notify setting failed!\n");
        }
    } else {
        if (apr_server_notify_get(&value) != VTSS_RC_OK) {
            CPRINTF("APR server notify get failed!\n");
            return;
        }
        CPRINTF("APR server notify: %s\n", ((value == TRUE) ? "enable": "disable"));
    }
}

static void cli_cmd_apr_server_one_hz(cli_req_t *req)
{
    zl_3034x_cli_req_t   *zl_3034x_req = req->module_req;
    BOOL value = FALSE;

    if (req->set) {
        if (apr_server_one_hz_set(zl_3034x_req->enable) != VTSS_RC_OK) {
            CPRINTF("APR server one hz setting failed!\n");
        }
    } else {
        if (apr_server_one_hz_get(&value) != VTSS_RC_OK) {
            CPRINTF("APR server one Hz get failed!\n");
            return;
        }
        CPRINTF("APR server one Hz capability: %s\n", ((value == TRUE) ? "enable": "disable"));
    }
}

static char *apr_mode_2_txt(u32 value)
{
    switch (value) {
        case 0: return "FREQ_TCXO";
        case 1: return "FREQ_OCXO_S3E";
        case 2: return "BC_PARTIAL_ON_PATH_FREQ";
        case 3: return "BC_PARTIAL_ON_PATH_PHASE"; 
        case 4: return "BC_PARTIAL_ON_PATH_SYNCE";
        case 5: return "BC_FULL_ON_PATH_FREQ";
        case 6: return "BC_FULL_ON_PATH_PHASE";
        case 7: return "BC_FULL_ON_PATH_SYNCE"; 
        case 8: return "FREQ_ACCURACY_FDD";
    }
    return "INVALID";
}

static void cli_cmd_apr_config_parameters(cli_req_t *req)
{
    zl_3034x_cli_req_t   *zl_3034x_req = req->module_req;
    BOOL value = FALSE;
    char zl_config[150];

    if (req->set) {
        if (apr_config_parameters_set(zl_3034x_req->apr_mode) != VTSS_RC_OK) {
            CPRINTF("APR server mode setting failed!\n");
        } else {
            CPRINTF("APR server mode set to %s\n", apr_mode_2_txt(zl_3034x_req->apr_mode));
			CPRINTF("ZL_3034X mode Configuration has been changed, you need to reboot to activate the changed conf.\n");
        }
    } else {
        if (apr_server_one_hz_get(&value) != VTSS_RC_OK || (apr_config_parameters_get(&zl_3034x_req->apr_mode, zl_config) != VTSS_RC_OK)) {
            CPRINTF("APR server mode get failed!\n");
            return;
        }
        CPRINTF("APR server one Hz capability: %s\n", ((value == TRUE) ? "enable": "disable"));
        CPRINTF("APR mode: %s\n", apr_mode_2_txt(zl_3034x_req->apr_mode));
        CPRINTF("ZL config: %s\n", zl_config);
    }
}


static void cli_cmd_apr_adjustment_min(cli_req_t *req)
{
    zl_3034x_cli_req_t   *zl_3034x_req = req->module_req;
    if (req->set) {
        if (apr_adj_min_set(zl_3034x_req->apr_adj) != VTSS_RC_OK) {
            CPRINTF("APR adjustment freq min phase setting failed!\n");
        } else {
            CPRINTF("APR adjustment freq min phase set to %ds\n", zl_3034x_req->apr_adj);
			CPRINTF("ZL_3034X mode Configuration has been changed, you need to reboot to activate the changed conf.\n");
        }
    } else {
        if (apr_adj_min_get(&zl_3034x_req->apr_adj) != VTSS_RC_OK) {
            CPRINTF("APR adjustment freq min phase get failed!\n");
            return;
        }
        CPRINTF("APR adjustment freq min phase: %ds\n", zl_3034x_req->apr_adj);
    }
}

static void cli_cmd_apr_config_dump(cli_req_t *req)
{
    if (zl_3034x_apr_config_dump() != VTSS_RC_OK) {
        CPRINTF("APR config dump failed!\n");
    }
}


static cli_parm_t zl_3034x_cli_parm_table[] = {
    {
        "enable|disable",
        "enable: enter to holdover; disable: leave from holdover",
        CLI_PARM_FLAG_SET,
        cli_parse_enable,
        cli_cmd_apr_force_holdover,
    },
    {
        "<log_level>",
        "APR logging level, 0= least detailed information, 3 = most detailed",
        CLI_PARM_FLAG_NONE | CLI_PARM_FLAG_SET,
        cli_parm_parse_log_level,
        cli_cmd_apr_logging
    },
    {
        "<log_level>",
        "APR ts logging level, 0= log forwarding, 1 = log reverse 2 = log both 3 = stop logging",
        CLI_PARM_FLAG_NONE | CLI_PARM_FLAG_SET,
        cli_parm_parse_log_level,
        cli_cmd_apr_ts_logging
    },
    {
        "reset",
        "Reset statistics",
        CLI_PARM_FLAG_SET,
        cli_parse_reset,
        cli_cmd_apr_statistics,
    },
    {
        "enable|disable",
        "enable/disable",
        CLI_PARM_FLAG_SET,
        cli_parse_enable,
        NULL,
    },
    {
        "<value>",
        "Register value",
        CLI_PARM_FLAG_NONE | CLI_PARM_FLAG_SET,
        cli_parm_parse_value,
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
        "<apr_mode>",
        "\nAPR mode, 0= FREQ_TCXO:\n"
        "             algType = NATIVE_PKT_FREQ, osc.Filt. = TCXO, filter = BW_0\n"
        "          1= FREQ_OCXO_S3E:\n"
        "             algType = NATIVE_PKT_FREQ, osc.Filt. = OCXO, filter = BW_0\n"
        "          2= BC_PARTIAL_ON_PATH_FREQ:\n"
        "             algType = NATIVE_PKT_FREQ_FLEX, osc.Filt. = d.c., filter = BW_0\n"
        "          3= BC_PARTIAL_ON_PATH_PHASE:\n"
        "             algType = NATIVE_PKT_FREQ_FLEX, osc.Filt. = d.c., filter = BW_0\n"
        "          4= BC_PARTIAL_ON_PATH_SYNCE:\n"
        "             p.t.not supported\n"
        "          5= BC_FULL_ON_PATH_FREQ:\n"
        "             algType = BOUNDARY_CLK, osc.Filt. = d.c., filter = BW_1\n"
        "          6= BC_FULL_ON_PATH_PHASE:\n"
        "             algType = BOUNDARY_CLK, osc.Filt. = d.c., filter = BW_1\n"
        "          7= BC_FULL_ON_PATH_SYNCE:\n"
        "             p.t.not supported\n"
        "          8= FREQ_ACCURACY_FDD:\n"
        "             algType = NATIVE_PKT_FREQ_ACCURACY_FDD, osc.Filt. = d.c, filter = BW_2",
        CLI_PARM_FLAG_NONE | CLI_PARM_FLAG_SET,
        cli_parm_parse_apr_mode,
        NULL
    },
    {
        "<apr_adj>",
        "Adjustment freq min phase in ns. [0..100]",
        CLI_PARM_FLAG_NONE | CLI_PARM_FLAG_SET,
        cli_parm_parse_apr_adj,
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
cli_cmd_tab_entry (
    "ZL3034X APR Logging",
    "ZL3034X APR Logging [enable|disable] [<log_level>]",
    "APR Logging enable/disable, and set level",
    PRIO_APR_LOGGING,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_ZL_3034X_PDV,
    cli_cmd_apr_logging,
    zl_3034x_cli_def_req,
    zl_3034x_cli_parm_table,
    CLI_CMD_FLAG_SYS_CONF
);

cli_cmd_tab_entry (
    "ZL3034X APR TS Logging",
    "ZL3034X APR ts Logging [<log_level>]",
    "APR TS Logging get/set level",
    PRIO_APR_LOGGING,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_ZL_3034X_PDV,
    cli_cmd_apr_ts_logging,
    zl_3034x_cli_def_req,
    zl_3034x_cli_parm_table,
    CLI_CMD_FLAG_SYS_CONF
);


cli_cmd_tab_entry (
  "ZL3034X APR Device Status",
  NULL,
  "APR device status information",
  PRIO_APR_DEVICE_STATUS,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_ZL_3034X_PDV,
  cli_cmd_apr_dev_status,
  zl_3034x_cli_def_req,
  zl_3034x_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "ZL3034X APR Server Config",
  NULL,
  "Show the APR Server configuration",
  PRIO_APR_SERVER_CONFIG,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_ZL_3034X_PDV,
  cli_cmd_apr_server_config,
  zl_3034x_cli_def_req,
  zl_3034x_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "ZL3034X APR Server Status",
  NULL,
  "The APR Server status information",
  PRIO_APR_SERVER_STATUS,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_ZL_3034X_PDV,
  cli_cmd_apr_server_status,
  zl_3034x_cli_def_req,
  zl_3034x_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "ZL3034X APR Force Holdover",
  "ZL3034X APR Force Holdover [enable|disable]",
  "Set APR to enter to or leave from holdover state manually",
  PRIO_APR_FORCE_HOLDOVER,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_ZL_3034X_PDV,
  cli_cmd_apr_force_holdover,
  zl_3034x_cli_def_req,
  zl_3034x_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "ZL3034X APR Statistics [reset]",
  NULL,
  "Get/reset all APR statistics including CGU and timing server performace statistics",
  PRIO_APR_STATISTICS,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_ZL_3034X_PDV,
  cli_cmd_apr_statistics,
  zl_3034x_cli_def_req,
  zl_3034x_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "ZL3034X APR Server Notify",
  "ZL3034X APR Server Notify [enable|disable]",
  "APR Server notify enable/disable",
  PRIO_APR_SERVER_NOTIFY,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_ZL_3034X_PDV,
  cli_cmd_apr_server_notify,
  zl_3034x_cli_def_req,
  zl_3034x_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "ZL3034X APR 1Hz Phase",
    "ZL3034X APR 1Hz Phase [enable|disable]",
    "APR Server one Hz phase alignment capability enable/disable",
    PRIO_APR_SERVER_ONE_HZ,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_ZL_3034X_PDV,
    cli_cmd_apr_server_one_hz,
    zl_3034x_cli_def_req,
    zl_3034x_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry (
    "ZL3034X APR Mode",
    "ZL3034X APR mode [<apr_mode>]",
    "APR Filter algorithm mode set",
    PRIO_APR_SERVER_MODE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_ZL_3034X_PDV,
    cli_cmd_apr_config_parameters,
    zl_3034x_cli_def_req,
    zl_3034x_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry (
    "ZL3034X APR Min_adj",
    "ZL3034X APR Min_adj [<apr_adj>]",
    "APR adjustment freq min phase",
    PRIO_APR_SERVER_MODE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_ZL_3034X_PDV,
    cli_cmd_apr_adjustment_min,
    zl_3034x_cli_def_req,
    zl_3034x_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry (
    "ZL3034X APR Config Dump",
    NULL,
    "Dump the APR configuration data",
    PRIO_APR_CONFIG_DUMP,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_ZL_3034X_PDV,
    cli_cmd_apr_config_dump,
    zl_3034x_cli_def_req,
    zl_3034x_cli_parm_table,
    CLI_CMD_FLAG_NONE
);


