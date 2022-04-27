/*

 Vitesse Switch API software.

 Copyright (c) 2002-2011 Vitesse Semiconductor Corporation "Vitesse". All
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

//#include "fan_cli.h"
#include "cli.h"
#include "fan_api.h"
#include "port_api.h"
#include "fan.h"
#include "cli.h"
#include "mgmt_api.h"

typedef struct {
    vtss_uport_no_t uport_dis; /* 1 - VTSS_PORTS or 0 (disabled) */

    /* Keywords */
    u8              temp;
    BOOL            t_max;
    BOOL            t_on;
} fan_cli_req_t;

/****************************************************************************/
/*  Initialization                                                          */
/****************************************************************************/

void fan_cli_init(void)
{
    /* register the size required for fan req. structure */
    cli_req_size_register(sizeof(fan_cli_req_t));
}

/****************************************************************************/
/*  Command functions                                                       */
/****************************************************************************/

/* FAN commands */
static void cli_cmd_fan(cli_req_t *req, BOOL temp_max, BOOL temp_on, BOOL status)
{
    vtss_usid_t          usid;
    vtss_isid_t          isid;
    fan_local_conf_t     fan_local_conf;
    i8                   header_txt[255];
    i8                   conf_txt[255];
    fan_cli_req_t        *fan_req = req->module_req;;
    fan_local_status_t   switch_status;
    i8                   str_buf[200];
    vtss_rc              rc;
    u8                   sensor_id;
    u8                   sensor_cnt;
    if (cli_cmd_conf_slave(req) || cli_cmd_switch_none(req)) {
        return;
    }

    strcpy(conf_txt, ""); //Clear string



    /* Get fan status */
    if (status) {
        for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
            if ((isid = req->stack.isid[usid]) == VTSS_ISID_END) {
                continue;
            }

            strcpy(header_txt, ""); //Clear string

            sensor_cnt = FAN_TEMPERATURE_SENSOR_CNT(isid);
            for (sensor_id = 0; sensor_id < sensor_cnt; sensor_id++) {
                strcat(header_txt, "Chip Temp.  ");
            }
            strcat(header_txt, "Fan Speed  ");

            cli_cmd_usid_print(usid, req, 1);
            cli_table_header(header_txt);

            if ((rc = fan_mgmt_get_switch_status(&switch_status, isid))) {
                CPRINTF("%s", error_txt(rc));
            } else {
                for (sensor_id = 0; sensor_id < sensor_cnt; sensor_id++) {
                    sprintf(str_buf, "%d %s", switch_status.chip_temp[sensor_id], "C");
                    CPRINTF("%-12s", str_buf);
                }
                sprintf(str_buf, "%d %s", switch_status.fan_speed, "RPM");
                CPRINTF("%-10s", str_buf);
            }
            CPRINTF("\n");
        }
        return;
    }




    // Get configuration for the current switch
    fan_mgmt_get_switch_conf(&fan_local_conf);

    // Setting new configuration
    if (req->set) {
        if (temp_max) {
            fan_local_conf.glbl_conf.t_max  = fan_req->temp;
        }

        if (temp_on) {
            fan_local_conf.glbl_conf.t_on  = fan_req->temp;
        }


        // Do the new configuration
        T_DG(TRACE_GRP_CONF, "Setting conf., fan_local_conf.glbl_conf.t_on = %d", fan_local_conf.glbl_conf.t_on);
        if ((rc = fan_mgmt_set_switch_conf(&fan_local_conf)) != VTSS_OK) {
            CPRINTF("%s \n", error_txt(rc));
        }
        return;
    }


    // Print out table header
    strcpy(header_txt, ""); //Clear string

    // Add the header for the configuration entries that needs to be printed.
    if (temp_max) {
        strcat(header_txt, "Temp Max.  ");
    }

    if (temp_on) {
        strcat(header_txt, "Temp On  ");
    }

    // Do the header printout
    cli_table_header(header_txt);


    // Print parameters
    if (temp_max) {
        sprintf(str_buf, "%d %s", fan_local_conf.glbl_conf.t_max, "C");
        CPRINTF("%-11s", str_buf);
    }

    if (temp_on) {
        sprintf(str_buf, "%d %s", fan_local_conf.glbl_conf.t_on, "C");
        CPRINTF("%-9s", str_buf);
    }

    CPRINTF("\n");
}



static void cli_cmd_fan_conf(cli_req_t *req)
{
    if (!req->set) {
        cli_header("FAN Configuration", 1);
    }
    cli_cmd_fan(req, 1, 1, 0);
}


static void cli_cmd_fan_temp_max(cli_req_t *req)
{
    cli_cmd_fan(req, 1, 0, 0);
}

static void cli_cmd_fan_temp_on(cli_req_t *req)
{
    cli_cmd_fan(req, 0, 1, 0);
}

static void cli_cmd_fan_status(cli_req_t *req)
{
    cli_cmd_fan(req, 0, 0, 1);
}




/****************************************************************************/
/*  Parameter functions                                                     */
/****************************************************************************/

static int32_t cli_fan_parse_keyword (char *cmd, char *cmd2, char *stx, char *cmd_org,
                                      cli_req_t *req)
{
    char *found = cli_parse_find(cmd, stx);

    fan_cli_req_t *fan_req = req->module_req;

    req->parm_parsed = 1;
    T_D("Found %s", found);
    if (found != NULL) {
        if (!strncmp(found, "t_on", 4)) {
            fan_req->t_on = 1;
        } else if (!strncmp(found, "t_max", 5)) {
            fan_req->t_max = 1;
        }
    }
    return (found == NULL ? 1 : 0);
}

// Parser wakeup time
static int32_t cli_fan_temp_parse (char *cmd, char *cmd2, char *stx, char *cmd_org,
                                   cli_req_t *req)
{
    fan_cli_req_t *fan_req = req->module_req;
    ulong error = 0;
    long value;

    req->parm_parsed = 1;
    error = cli_parse_long(cmd, &value, FAN_TEMP_MIN, FAN_TEMP_MAX);
    fan_req->temp    = value;
    return (error);
}


/****************************************************************************/
/*  Parameter table                                                         */
/****************************************************************************/
static cli_parm_t fan_cli_parm_table[] = {
    {
        "<temp>",
        "Chip temperature (-127-127)\n",
        CLI_PARM_FLAG_SET,
        cli_fan_temp_parse,
        NULL,
    },
    {
        "<t_on>",
        "Temperature at which the fan shall start cooling\n",
        CLI_PARM_FLAG_SET,
        cli_fan_parse_keyword,
        NULL,
    },

    {NULL, NULL, 0, 0, NULL},
};


/****************************************************************************/
/*  Command table                                                           */
/****************************************************************************/
enum {
    FAN_T_ON,
    FAN_T_MAX,
    FAN_STATUS,
    FAN_CONF,
};

cli_cmd_tab_entry (
    "GreenEthernet Fan t_on",
    "GreenEthernet Fan t_on  [<temp>]",
    "Set or show the temperature at which the fan shall start cooling",
    FAN_T_ON,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_FAN,
    cli_cmd_fan_temp_on,
    NULL,
    fan_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "GreenEthernet Fan t_max",
    "GreenEthernet Fan t_max  [<temp>]",
    "Set or show the temperature at where the FAN must to running at 100%",
    FAN_T_MAX,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_FAN,
    cli_cmd_fan_temp_max,
    NULL,
    fan_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "GreenEthernet Fan status",
    "GreenEthernet Fan status",
    "Shows the chip temperature and fan speed",
    FAN_STATUS,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_FAN,
    cli_cmd_fan_status,
    NULL,
    fan_cli_parm_table,
    CLI_CMD_FLAG_NONE
);


cli_cmd_tab_entry (
    "GreenEthernet Fan configuration",
    "GreenEthernet Fan configuration",
    "Show fan configuration",
    FAN_CONF,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_FAN,
    cli_cmd_fan_conf,
    NULL,
    fan_cli_parm_table,
    CLI_CMD_FLAG_NONE
);



/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
