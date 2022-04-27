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

#include "mirror_cli.h"
#include "main.h"
#include "cli.h"
#include "mirror_api.h"
#include "port_api.h"
#include "mirror.h"
#include "cli.h"
#if VTSS_SWITCH_STACKABLE
#include "topo_api.h"
#endif
#include "msg_api.h"

typedef struct {
    vtss_uport_no_t uport_dis; /* 1 - VTSS_PORTS or 0 (disabled) */

    /* Keywords */
    BOOL            rx;
    BOOL            tx;
} mirror_cli_req_t;

/****************************************************************************/
/*  Initialization                                                          */
/****************************************************************************/

void mirror_cli_init(void)
{
    /* register the size required for mirror req. structure */
    cli_req_size_register(sizeof(mirror_cli_req_t));
}

/****************************************************************************/
/*  Command functions                                                       */
/****************************************************************************/

static char mirror_sid_cmd[128] = "";
static char mirror_sid_cmd_usage[128] = "";

// Function for moving some cli mirror command to debug commands in case that the functionality isn't supported
void mirror_cli_txt_init(void)
{
    // If switch has stacking enabled the allow the SID cli command
    if (vtss_stacking_enabled()) {
        sprintf(mirror_sid_cmd,
                "Mirror SID");

        sprintf(mirror_sid_cmd_usage,
                "Mirror SID [<sid>]");
    } else {
        // Else move it to a debug command.
        sprintf(mirror_sid_cmd,
                "Debug Mirror SID");

        sprintf(mirror_sid_cmd_usage,
                "Debug Mirror SID [<sid>]");
    }
}


/* Mirror commands */
static void cli_cmd_mirror(cli_req_t *req, BOOL port, BOOL sid, BOOL mode, BOOL include_stack_ports)
{
    switch_iter_t        sit;
    mirror_conf_t        conf;
    mirror_switch_conf_t switch_conf;
    char                 buf[80];
    vtss_port_no_t       iport_dis;
    mirror_cli_req_t     *mirror_req = req->module_req;
    vtss_rc              rc;

    if (cli_cmd_conf_slave(req)) {
        return;
    }


    mirror_mgmt_conf_get(&conf);

    /* Get/set mirror port */
    if (port || sid) {
        if (req->set) {
            if (port) {
                iport_dis = uport2iport(mirror_req->uport_dis);
                T_I("Mirror User Port = %u, usid:%d", mirror_req->uport_dis, conf.mirror_switch);
#if VTSS_SWITCH_STACKABLE
                if (mirror_req->uport_dis && port_isid_port_no_is_stack(topo_usid2isid(conf.mirror_switch), iport_dis)) {
                    CPRINTF("Stack port %u for switch %d cannot be mirror port\n", mirror_req->uport_dis, conf.mirror_switch);
                    return;
                }
#endif
                conf.dst_port = iport_dis;
            }
#if VTSS_SWITCH_STACKABLE
            if (sid && vtss_stacking_enabled()) {
                if (msg_switch_exists(topo_usid2isid(req->usid[0]))) {
                    if (port_isid_port_no_is_stack(topo_usid2isid(req->usid[0]), conf.dst_port)) {
                        CPRINTF("Stack port %u for switch %d cannot be mirror port\n", conf.dst_port, req->usid[0]);
                    } else {
                        conf.mirror_switch = topo_usid2isid(req->usid[0]);
                    }
                } else {
                    CPRINTF("Invalid switch id:%d\n", req->usid[0]);
                }
            }
#endif

            if ((rc = mirror_mgmt_conf_set(&conf)) != VTSS_OK) {
                cli_printf("\n%s\n", error_txt(rc));
            }
        } else {
            if (port) {
                if (conf.dst_port != VTSS_PORT_NO_NONE) {
                    sprintf(buf, "%u", iport2uport(conf.dst_port));
                } else {
                    misc_strncpyz(buf, cli_bool_txt(0), 80);
                }
                CPRINTF("Mirror Port: %s\n", buf);
            }
#if VTSS_SWITCH_STACKABLE
            if (sid && vtss_stacking_enabled()) {
                CPRINTF("Mirror SID : %d\n", topo_isid2usid(conf.mirror_switch));
            }
#endif
        }
    }

    if (!mode) {
        return;
    }

    /* Get/set mirror mode */
    (void)cli_switch_iter_init(&sit);
    while (cli_switch_iter_getnext(&sit, req)) {
        port_iter_t pit;

        // Get current configuration
        if ((rc = mirror_mgmt_switch_conf_get(sit.isid, &switch_conf)) != VTSS_OK) {
            cli_printf("\n%s\n", error_txt(rc));
            continue;
        }

        // Loop through all ports
        (void)cli_port_iter_init(&pit, sit.isid, PORT_ITER_FLAGS_FRONT | PORT_ITER_FLAGS_STACK);
        while (cli_port_iter_getnext(&pit, req)) {
            BOOL stack_port = port_isid_port_no_is_stack(sit.isid, pit.iport);

            if (req->set) {
                if (stack_port && !include_stack_ports) {
                    CPRINTF("Stack port %u for switch %d cannot be mirrored (ignored)\n", iport2uport(pit.iport), sit.usid);
                    continue; // Only allow stack ports if include_stack_ports is set
                }

                switch_conf.src_enable[pit.iport] = (req->enable || mirror_req->rx);
                switch_conf.dst_enable[pit.iport] = (req->enable || mirror_req->tx);

                T_D_PORT(pit.iport, "src:%d, dst:%d, req_ena:%d, req_rx:%d, req:_tx:%d",
                         switch_conf.src_enable[pit.iport], switch_conf.dst_enable[pit.iport]
                         , req->enable, mirror_req->rx, mirror_req->tx);

                if (pit.iport == conf.dst_port && conf.mirror_switch == sit.isid && switch_conf.dst_enable[pit.iport] == TRUE) {
                    // Note: This is done in mirror.c when the configuration is updated.
                    CPRINTF("Setting Tx mirroring for mirror port (port %u) has no effect. Tx mirroring ignored \n", iport2uport(conf.dst_port));
                }
                continue;
            }
            if (pit.first) {
                cli_cmd_usid_print(sit.usid, req, 1);
                cli_table_header("Port  Mode      ");
            }

            if (!stack_port || include_stack_ports ||
                switch_conf.src_enable[pit.iport] ||  // Always show src and/or dst enabled ports - even stack ports
                switch_conf.dst_enable[pit.iport]) {
                CPRINTF("%-2u    %s %s\n",
                        pit.uport,
                        switch_conf.src_enable[pit.iport] ? switch_conf.dst_enable[pit.iport] ? "Enabled" : "Rx" :
                        switch_conf.dst_enable[pit.iport] ? "Tx" : "Disabled",
                        stack_port ? "(Stack Port!)" : "");
            }
        }

        if (req->set) {
#ifdef VTSS_FEATURE_MIRROR_CPU
            // Set CPU mirroring
            if (req->cpu_port) {
                switch_conf.cpu_src_enable = (req->enable || mirror_req->rx);
                switch_conf.cpu_dst_enable = (req->enable || mirror_req->tx);
            }
#endif
            mirror_mgmt_switch_conf_set(sit.isid, &switch_conf);
        } else {
#ifdef VTSS_FEATURE_MIRROR_CPU
            // Set CPU mirroring
            if (req->cpu_port) {
                CPRINTF("%s   %s \n",
                        "CPU",
                        switch_conf.cpu_src_enable ? switch_conf.cpu_dst_enable ? "Enabled" : "Rx" :
                        switch_conf.cpu_dst_enable ? "Tx" : "Disabled");
            }
#endif
        }
    }
}

static void cli_cmd_mirror_conf(cli_req_t *req)
{
    if (!req->set) {
        cli_header("Mirror Configuration", 1);
    }

    cli_cmd_mirror(req, 1, 1, 1, 0);
}

static void cli_cmd_mirror_port(cli_req_t *req)
{
    cli_cmd_mirror(req, 1, 0, 0, 0);
}

#if VTSS_SWITCH_STACKABLE
static void cli_cmd_mirror_sid(cli_req_t *req)
{
    cli_cmd_mirror(req, 0, 1, 0, 0);
}

static void cli_cmd_mirror_mode_debug(cli_req_t *req)
{
    cli_cmd_mirror(req, 0, 0, 1, 1);
}

#endif /*VTSS_SWITCH_STACKABLE*/

static void cli_cmd_mirror_mode(cli_req_t *req)
{
    cli_cmd_mirror(req, 0, 0, 1, 0);
}

static int32_t cli_mirror_port_dis (char *cmd, char *cmd2, char *stx, char *cmd_org,
                                    cli_req_t *req)
{
    int32_t error = 0;
    ulong   value = 0;
    ulong   min   = 1, max = VTSS_PORTS;

    mirror_cli_req_t *mirror_req = req->module_req;

    req->parm_parsed = 1;
    error = (cli_parse_ulong(cmd, &value, min, max) && cli_parm_parse_keyword(cmd, cmd2, stx, cmd_org, req));
    mirror_req->uport_dis = value;

    return error;
}

/****************************************************************************/
/*  Parameter functions                                                     */
/****************************************************************************/

static int32_t cli_mirror_parse_keyword (char *cmd, char *cmd2, char *stx, char *cmd_org,
                                         cli_req_t *req)
{
    char *found = cli_parse_find(cmd, stx);
    mirror_cli_req_t *mirror_req = req->module_req;

    req->parm_parsed = 1;
    T_D("Found %s", found);
    if (found != NULL) {
        if (!strncmp(found, "disable", 7)) {
            req->disable = 1;
        } else if (!strncmp(found, "enable", 6)) {
            req->enable = 1;
        } else if (!strncmp(found, "rx", 2)) {
            mirror_req->rx = 1;
        }  else if (!strncmp(found, "tx", 2)) {
            mirror_req->tx = 1;
        }
    }
    return (found == NULL ? 1 : 0);
}

/****************************************************************************/
/*  Parameter table                                                         */
/****************************************************************************/

static cli_parm_t mirror_cli_parm_table[] = {
    {
        "<port>|disable",
        "Mirror port or 'disable', default: Show port",
        CLI_PARM_FLAG_SET,
        cli_mirror_port_dis,
        cli_cmd_mirror_port
    },
    {
        "enable|disable|rx|tx",
        "enable : Enable Rx and Tx mirroring\n"
        "disable: Disable Mirroring\n"
        "rx     : Enable Rx mirroring\n"
        "tx     : Enable Tx mirroring\n"
        "(default: Show mirror mode)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_mirror_parse_keyword,
        NULL
    },
    {NULL, NULL, 0, 0, NULL}
};

/****************************************************************************/
/*  Command table                                                           */
/****************************************************************************/

enum {
    PRIO_MIRROR_CONF,
    PRIO_MIRROR_PORT,
    PRIO_MIRROR_SID,
    PRIO_MIRROR_MODE,
    PRIO_MIRROR_MODE_DEBUG = CLI_CMD_SORT_KEY_DEFAULT,
};

cli_cmd_tab_entry (
    "Mirror Configuration [<port_list>]",
    NULL,
    "Show mirror configuration",
    PRIO_MIRROR_CONF,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_MIRROR,
    cli_cmd_mirror_conf,
    NULL,
    mirror_cli_parm_table,
    CLI_CMD_FLAG_SYS_CONF
);
cli_cmd_tab_entry (
    "Mirror Port",
    "Mirror Port [<port>|disable]",
    "Set or show the mirror port",
    PRIO_MIRROR_PORT,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_MIRROR,
    cli_cmd_mirror_port,
    NULL,
    mirror_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#if VTSS_SWITCH_STACKABLE
cli_cmd_tab_entry (
    mirror_sid_cmd,
    mirror_sid_cmd_usage,
    "Set or show the mirror switch ID",
    PRIO_MIRROR_SID,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_MIRROR,
    cli_cmd_mirror_sid,
    NULL,
    mirror_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif /* VTSS_SWITCH_STACKABLE */

#ifdef VTSS_FEATURE_MIRROR_CPU
cli_cmd_tab_entry (
    "Mirror Mode [<port_cpu_list>]",
    "Mirror Mode [<port_cpu_list>] [enable|disable|rx|tx]",
    "Set or show the mirror mode",
    PRIO_MIRROR_MODE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_MIRROR,
    cli_cmd_mirror_mode,
    NULL,
    mirror_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#else
cli_cmd_tab_entry (
    "Mirror Mode [<port_list>]",
    "Mirror Mode [<port_list>] [enable|disable|rx|tx]",
    "Set or show the mirror mode",
    PRIO_MIRROR_MODE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_MIRROR,
    cli_cmd_mirror_mode,
    NULL,
    mirror_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif
#if VTSS_SWITCH_STACKABLE
cli_cmd_tab_entry (
    "Debug Mirror Mode [<port_list>]",
    "Debug Mirror Mode [<port_list>] [enable|disable|rx|tx]",
    "Set or show the mirror mode (including stack ports).\n"
    "Use the following command on the destination port in order to include VStaX header:\n"
    "Debug Sym Write REW:PHYSPORT[<chip_port>]:VSTAX_CTRL 1\n\n"
    "Settings are saved in flash, so please remember to disable mirroring of stack ports",
    PRIO_MIRROR_MODE_DEBUG,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_MIRROR,
    cli_cmd_mirror_mode_debug,
    NULL,
    mirror_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif /*VTSS_SWITCH_STACKABLE*/
/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
