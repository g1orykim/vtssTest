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
#include "thermal_protect_api.h"
#include "port_api.h"
#include "thermal_protect.h"
#include "cli.h"
#include "mgmt_api.h"

typedef struct {
    /* Keywords */
    u8              temp;
    BOOL            prio;
    BOOL            prio_list[THERMAL_PROTECT_PRIOS_CNT];
} thermal_protect_cli_req_t;

/****************************************************************************/
/*  Initialization                                                          */
/****************************************************************************/

void thermal_protect_cli_init(void)
{
    /* register the size required for thermal_protect req. structure */
    cli_req_size_register(sizeof(thermal_protect_cli_req_t));
}

/****************************************************************************/
/*  Command functions                                                       */
/****************************************************************************/


/* THERMAL_PROTECT commands */
static void cli_cmd_thermal_protect(cli_req_t *req, BOOL port_prio, BOOL prio_temp, BOOL status)
{
    vtss_usid_t          usid;
    vtss_isid_t          isid;
    thermal_protect_switch_conf_t     thermal_protect_switch_conf;
    thermal_protect_cli_req_t        *thermal_protect_req = req->module_req;;
    thermal_protect_local_status_t   switch_status;
    i8                   str_buf[200];
    vtss_rc              rc;
    int                  prio_index;
    BOOL                 print_all_prio_conf = TRUE;
    port_iter_t          pit;

    if (cli_cmd_conf_slave(req) || cli_cmd_switch_none(req)) {
        return;
    }

    //
    // Global configuration shall only be printed once.
    //
    if (prio_temp) {
        if (!req->set) {
            thermal_protect_mgmt_switch_conf_get(&thermal_protect_switch_conf, VTSS_ISID_LOCAL);
            cli_table_header("\nPriority  Temp.  ");

            for (prio_index = 0; prio_index < THERMAL_PROTECT_PRIOS_CNT; prio_index++) {
                if (thermal_protect_req->prio_list[prio_index] == 0) {
                    continue;
                }
                print_all_prio_conf = FALSE;
                break;
            }


            for (prio_index = 0; prio_index < THERMAL_PROTECT_PRIOS_CNT; prio_index++) {
                if (thermal_protect_req->prio_list[prio_index] == 0 && print_all_prio_conf == FALSE) {
                    continue;
                }

                sprintf(str_buf, "%-9d %d %s", prio_index, thermal_protect_switch_conf.glbl_conf.prio_temperatures[prio_index], "C");
                CPRINTF("%-9s \n",  str_buf);
            }
        }
    }

    /* Get/set thermal_protect port configuration */
    for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
        if ((isid = req->stack.isid[usid]) == VTSS_ISID_END) {
            continue;
        }

        // Get configuration for the current switch
        thermal_protect_mgmt_switch_conf_get(&thermal_protect_switch_conf, isid);

        // Setting new configuration
        if (req->set) {
            // Priority temperature setting
            if (prio_temp) {
                for (prio_index = 0; prio_index < THERMAL_PROTECT_PRIOS_CNT; prio_index++) {
                    if (thermal_protect_req->prio_list[prio_index] == 0) {
                        continue;
                    }
                    thermal_protect_switch_conf.glbl_conf.prio_temperatures[prio_index] = thermal_protect_req->temp;
                }
            }

            // Port Priority setting
            if (port_prio) {
                // Port priorities
                // Loop through all front ports
                if (port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT | PORT_ITER_FLAGS_NPI) == VTSS_OK) {
                    while (port_iter_getnext(&pit)) {
                        if (req->uport_list[pit.uport] == 0) {
                            continue;
                        }
                        thermal_protect_switch_conf.local_conf.port_prio[pit.iport]  = thermal_protect_req->prio;
                    }
                }
            }

            // Set the new configuration
            if ((rc = thermal_protect_mgmt_switch_conf_set(&thermal_protect_switch_conf, isid)) != VTSS_OK) {
                CPRINTF("%s \n", error_txt(rc));
            }

            continue;
        }

        // Do the header printout
        cli_cmd_usid_print(usid, req, 1);

        // Print parameters
        if (port_prio) {
            cli_table_header("Port  Prio.  ");

            // Port priorities
            // Loop through all front ports
            if (port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT | PORT_ITER_FLAGS_NPI) == VTSS_OK) {
                while (port_iter_getnext(&pit)) {
                    if (req->uport_list[pit.uport] == 0) {
                        continue;
                    }

                    sprintf(str_buf, "%-5u %d", pit.uport, thermal_protect_switch_conf.local_conf.port_prio[pit.iport]);
                    CPRINTF("%-11s \n", str_buf);
                }
            }
        }

        // Print out of status data
        if (status) {
            cli_table_header("Port  Chip Temp.  Port Status");
            if ((rc = thermal_protect_mgmt_get_switch_status(&switch_status, isid))) {
                CPRINTF("%s", error_txt(rc));
            } else {
                // Loop through all front ports
                if (port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT | PORT_ITER_FLAGS_NPI) == VTSS_OK) {
                    while (port_iter_getnext(&pit)) {
                        if (req->uport_list[pit.uport] == 0) {
                            continue;
                        }

                        sprintf(str_buf, "%-5u %d %-8s %s", pit.uport, switch_status.port_temp[pit.iport], "C",
                                thermal_protect_power_down2txt(switch_status.port_powered_down[pit.iport]));
                        CPRINTF("%-12s \n", str_buf);
                    }
                }
            }
        }
    }
}

static void cli_cmd_thermal_protect_conf(cli_req_t *req)
{
    if (!req->set) {
        cli_header("Thermal Protection Configuration", 1);
    }
    cli_cmd_thermal_protect(req, 1, 1, 0);
}


static void cli_cmd_thermal_protect_port_prio(cli_req_t *req)
{
    cli_cmd_thermal_protect(req, 1, 0, 0);
}

static void cli_cmd_thermal_protect_prio_temp(cli_req_t *req)
{
    cli_cmd_thermal_protect(req, 0, 1, 0);
}

static void cli_cmd_thermal_protect_status(cli_req_t *req)
{
    cli_cmd_thermal_protect(req, 0, 0, 1);
}




/****************************************************************************/
/*  Parameter functions                                                     */
/****************************************************************************/

// Parser wakeup time
static int32_t cli_thermal_protect_temp_parse (char *cmd, char *cmd2, char *stx, char *cmd_org,
                                               cli_req_t *req)
{
    thermal_protect_cli_req_t *thermal_protect_req = req->module_req;
    ulong error = 0;
    ulong value;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &value, THERMAL_PROTECT_TEMP_MIN, THERMAL_PROTECT_TEMP_MAX);
    thermal_protect_req->temp    = value;
    return (error);
}

static int32_t cli_thermal_protect_prio_parse(char *cmd, char *cmd2, char *stx, char *cmd_org,
                                              cli_req_t *req)
{
    thermal_protect_cli_req_t *thermal_protect_req = req->module_req;
    ulong error = 0;
    ulong value;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &value, THERMAL_PROTECT_PRIOS_MIN, THERMAL_PROTECT_PRIOS_MAX);
    thermal_protect_req->prio    = value;
    return (error);
}

static int cli_thermal_protect_prio_list_parse ( char *cmd, char *cmd2, char *stx, char *cmd_org,
                                                 cli_req_t *req)
{
    i16  error = 0; /* As a start there is no error */
    i16  prio_index;
    thermal_protect_cli_req_t *thermal_protect_req = req->module_req;

    req->parm_parsed = 1;
    char *found = cli_parse_find(cmd, stx);


    error = (cli_parse_all(cmd) && cli_parm_parse_list(cmd, thermal_protect_req->prio_list, THERMAL_PROTECT_PRIOS_MIN, THERMAL_PROTECT_PRIOS_MAX, 1));
    if (error && (error = cli_parse_none(cmd)) == 0) {
        for (prio_index = THERMAL_PROTECT_PRIOS_MIN; prio_index <= THERMAL_PROTECT_PRIOS_MAX; prio_index++) {
            thermal_protect_req->prio_list[prio_index] = 0;
            T_D("thermal_protect_req- %s, %d,l error = %d", found, cli_parse_none(cmd), error);
        }
    }
    T_D("ALL %d,l error = %d", cli_parse_none(cmd), error);
    return (error);
}




/****************************************************************************/
/*  Parameter table                                                         */
/****************************************************************************/
static cli_parm_t thermal_protect_cli_parm_table[] = {
    {
        "<shut_down_temp>",
        "Temperature at which ports shall be shut down (0-255 degree C) \n",
        CLI_PARM_FLAG_SET,
        cli_thermal_protect_temp_parse,
        NULL,
    },
    {
        "<prio>",
        "Priority  (0-3)\n",
        CLI_PARM_FLAG_SET,
        cli_thermal_protect_prio_parse,
        NULL,
    },

    {
        "<prio_list>",
        "List of priorities (0-3)",
        CLI_PARM_FLAG_NONE,
        cli_thermal_protect_prio_list_parse,
        NULL,
    },
    {NULL, NULL, 0, 0, NULL},
};


/****************************************************************************/
/*  Command table                                                           */
/****************************************************************************/
enum {
    THERMAL_PROTECT_PRIO_TEMP,
    THERMAL_PROTECT_PORT_PRIO,
    THERMAL_PROTECT_STATUS,
    THERMAL_PROTECT_CONF,
};

cli_cmd_tab_entry (
    "Thermal prio_temp",
    "Thermal prio_temp [<prio_list>] [<shut_down_temp>]",
    "Set or show the temperature at which the ports shall be shut down",
    THERMAL_PROTECT_PRIO_TEMP,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_THERMAL_PROTECT,
    cli_cmd_thermal_protect_prio_temp,
    NULL,
    thermal_protect_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "Thermal port_prio",
    "Thermal port_prio  [<port_list>] [<prio>]",
    "Set or show the ports priority",
    THERMAL_PROTECT_PORT_PRIO,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_THERMAL_PROTECT,
    cli_cmd_thermal_protect_port_prio,
    NULL,
    thermal_protect_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "Thermal status",
    "Thermal status",
    "Shows the chip temperature",
    THERMAL_PROTECT_STATUS,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_THERMAL_PROTECT,
    cli_cmd_thermal_protect_status,
    NULL,
    thermal_protect_cli_parm_table,
    CLI_CMD_FLAG_NONE
);


cli_cmd_tab_entry (
    "Thermal configuration",
    "Thermal configuration",
    "Show thermal_protect configuration",
    THERMAL_PROTECT_CONF,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_THERMAL_PROTECT,
    cli_cmd_thermal_protect_conf,
    NULL,
    thermal_protect_cli_parm_table,
    CLI_CMD_FLAG_NONE
);



/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
