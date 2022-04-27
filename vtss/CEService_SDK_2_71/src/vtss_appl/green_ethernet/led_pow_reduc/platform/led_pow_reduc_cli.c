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
#include "led_pow_reduc_api.h"
#include "port_api.h"
#include "led_pow_reduc.h"
#include "cli.h"
#include "mgmt_api.h"

typedef struct {
    /* Keywords */
    u8              maintenance_time;
    u8              maintenance_time_vld;
    BOOL            on_at_errors;
    BOOL            on_at_errors_vld;
    u16             intensity; // LEDs intensity
    BOOL            intensity_vld;
    BOOL            start_hour;
    BOOL            end_hour;
    BOOL            timer_vld;
} led_pow_reduc_cli_req_t;

/****************************************************************************/
/*  Initialization                                                          */
/****************************************************************************/

void led_pow_reduc_cli_init(void)
{
    /* register the size required for led_pow_reduc req. structure */
    cli_req_size_register(sizeof(led_pow_reduc_cli_req_t));
}

/****************************************************************************/
/*  Command functions                                                       */
/****************************************************************************/


/* LED_POW_REDUC commands */
static void cli_cmd_led_pow_reduc(cli_req_t *req, BOOL timers, BOOL maintenance, BOOL delete_timer)
{
    led_pow_reduc_local_conf_t     local_conf;
    led_pow_reduc_cli_req_t        *led_pow_reduc_req = req->module_req;;
    i8                   str_buf[200];
    if (cli_cmd_conf_slave(req) || cli_cmd_switch_none(req)) {
        return;
    }

    // Get current configuration
    led_pow_reduc_mgmt_get_switch_conf(&local_conf); // Get current configuration


    //
    // Global configuration shall only be printed once.
    //
    if (delete_timer) {
        T_D("intensity[%d] = %d",
            led_pow_reduc_req->start_hour,
            local_conf.glbl_conf.led_timer_intensity[led_pow_reduc_req->start_hour]);

        if (led_pow_reduc_mgmt_timer_set(led_pow_reduc_req->start_hour, led_pow_reduc_req->end_hour, LED_POW_REDUC_INTENSITY_DEFAULT) != VTSS_RC_OK) {
            T_E("Could not set LED timers");
        }
    }


    if (timers) {
        if (!req->set || !led_pow_reduc_req->intensity_vld) {
            cli_table_header("Start Time  End Time  Intensity");

            // Printout for all timers
            led_pow_reduc_timer_t led_timer;
            led_pow_reduc_mgmt_timer_get_init(&led_timer); // Prepare for looping through all timers

            // Loop through all timers and print them
            while (led_pow_reduc_mgmt_timer_get(&led_timer)) {
                sprintf(str_buf, "%02d:00       %02d:00     %-2d%%",
                        led_timer.start_index, led_timer.end_index, local_conf.glbl_conf.led_timer_intensity[led_timer.start_index]);
                CPRINTF("%-9s \n",  str_buf);
            }
        } else {
            if (led_pow_reduc_req->intensity_vld) {
                if (led_pow_reduc_mgmt_timer_set(led_pow_reduc_req->start_hour, led_pow_reduc_req->end_hour, led_pow_reduc_req->intensity) != VTSS_RC_OK) {
                    T_E("Could not set LED timers");
                }
            }
        }
    }

    if (maintenance) {
        if (!req->set) {
            CPRINTF("\n");
            cli_table_header("Time  Action at errors");
            sprintf(str_buf, "%-5d %s", local_conf.glbl_conf.maintenance_time, local_conf.glbl_conf.on_at_err ? "On" : "Leave" );
            CPRINTF("%s \n",  str_buf);
        } else {
            if (led_pow_reduc_req->maintenance_time_vld) {
                local_conf.glbl_conf.maintenance_time = led_pow_reduc_req->maintenance_time;
            }

            if (led_pow_reduc_req->on_at_errors_vld) {
                local_conf.glbl_conf.on_at_err = led_pow_reduc_req->on_at_errors;
            }
        }

        // Set new configuration
        if (req->set) {
            vtss_rc rc;
            if ((rc = led_pow_reduc_mgmt_set_switch_conf(&local_conf)) != VTSS_OK) {
                T_E(error_txt(rc));
            }
        }
    }
}

static void cli_cmd_led_pow_reduc_conf(cli_req_t *req)
{
    if (!req->set) {
        cli_header("LED Power Reduction Configuration", 1);
    }
    cli_cmd_led_pow_reduc(req, 1, 1, 0);
}


static void cli_cmd_led_pow_reduc_timers(cli_req_t *req)
{
    cli_cmd_led_pow_reduc(req, 1, 0, 0);
}

static void cli_cmd_led_pow_reduc_delete_timer(cli_req_t *req)
{
    cli_cmd_led_pow_reduc(req, 0, 0, 1);
}

static void cli_cmd_led_pow_reduc_maintenance(cli_req_t *req)
{
    cli_cmd_led_pow_reduc(req, 0, 1, 0);
}

/****************************************************************************/
/*  Parameter functions                                                     */
/****************************************************************************/

// Parser
static int32_t cli_led_pow_reduc_start_hour_parse (char *cmd, char *cmd2, char *stx, char *cmd_org,
                                                   cli_req_t *req)
{
    led_pow_reduc_cli_req_t *led_pow_reduc_req = req->module_req;
    ulong error = 0;
    ulong value;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &value, 0, 23);
    led_pow_reduc_req->start_hour = value;
    return (error);
}

static int32_t cli_led_pow_reduc_end_hour_parse (char *cmd, char *cmd2, char *stx, char *cmd_org,
                                                 cli_req_t *req)
{
    led_pow_reduc_cli_req_t *led_pow_reduc_req = req->module_req;
    ulong error = 0;
    ulong value;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &value, 0, 23);
    led_pow_reduc_req->end_hour = value;
    return (error);
}

static int32_t cli_led_pow_reduc_intensity_parse (char *cmd, char *cmd2, char *stx, char *cmd_org,
                                                  cli_req_t *req)
{
    led_pow_reduc_cli_req_t *led_pow_reduc_req = req->module_req;
    ulong error = 0;
    ulong value;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &value, LED_POW_REDUC_INTENSITY_MIN, LED_POW_REDUC_INTENSITY_MAX);
    led_pow_reduc_req->intensity   = value;
    if (error == 0) {
        led_pow_reduc_req->intensity_vld   = TRUE;
    }

    return (error);
}

static int32_t cli_led_pow_reduc_maintenance_parse(char *cmd, char *cmd2, char *stx, char *cmd_org,
                                                   cli_req_t *req)
{
    led_pow_reduc_cli_req_t *led_pow_reduc_req = req->module_req;
    ulong error = 0;
    ulong value;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &value, LED_POW_REDUC_MAINTENANCE_TIME_MIN, LED_POW_REDUC_MAINTENANCE_TIME_MAX);
    if (error == 0) {
        led_pow_reduc_req->maintenance_time_vld = TRUE;
    }
    led_pow_reduc_req->maintenance_time    = value;

    return (error);
}


// Determine user configuration for on_at_errors.
static int32_t cli_led_pow_reduc_on_at_errors_parse(char *cmd, char *cmd2, char *stx, char *cmd_org,
                                                    cli_req_t *req)
{

    led_pow_reduc_cli_req_t *led_pow_reduc_req = NULL;

    char *found = cli_parse_find(cmd, stx);
    req->parm_parsed = 1;
    led_pow_reduc_req = req->module_req;

    if (found != NULL) {
        if (!strncmp(found, "on_at_errors", 12 )) {
            led_pow_reduc_req->on_at_errors = TRUE;
        } else if (!strncmp(found, "leave_at_errors", 15)) {
            led_pow_reduc_req->on_at_errors = FALSE;
        }
    }
    if (found != NULL) {
        led_pow_reduc_req->on_at_errors_vld = TRUE;
    }

    return (found == NULL ? 1 : 0);
}






/****************************************************************************/
/*  Parameter table                                                         */
/****************************************************************************/
static cli_parm_t led_pow_reduc_cli_parm_table[] = {
    {
        "<start_hour>",
        "The hour of the interval (0-23) at which to change LEDs intensity\n",
        CLI_PARM_FLAG_SET,
        cli_led_pow_reduc_start_hour_parse,
        NULL,
    },

    {
        "<end_hour>",
        "The hour of the interval (0-23) at which to change LEDs intensity\n",
        CLI_PARM_FLAG_SET,
        cli_led_pow_reduc_end_hour_parse,
        NULL,
    },

    {
        "<intensity>",
        "The LED intensity in % (0-100)\n",
        CLI_PARM_FLAG_SET,
        cli_led_pow_reduc_intensity_parse,
        NULL,
    },

    {
        "<maintenance_time>",
        "Time in seconds (0-65535) that the LEDs shall be turned on, when any port changes link state \n",
        CLI_PARM_FLAG_SET,
        cli_led_pow_reduc_maintenance_parse,
        NULL,
    },

    {
        "on_at_errors|leave_at_errors",
        "on_at_error if LEDs shall be turned on if any errors has been detected. leave_at_errors if no LED change shall happen when errors have been detected",
        CLI_PARM_FLAG_SET,
        cli_led_pow_reduc_on_at_errors_parse,
        NULL,
    },
    {NULL, NULL, 0, 0, NULL},
};


/****************************************************************************/
/*  Command table                                                           */
/****************************************************************************/
enum {
    LED_POW_REDUC_TIMERS,
    LED_POW_REDUC_DELETE_TIMER,
    LED_POW_REDUC_MAINTANCE,
    LED_POW_REDUC_CONF,
};

cli_cmd_tab_entry (
    "GreenEthernet led timers",
    "GreenEthernet led timers [<start_hour>] [<end_hour>] [<intensity>]",
    "Set or show the time and intensity for the LEDs",
    LED_POW_REDUC_TIMERS,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_LED_POW_REDUC,
    cli_cmd_led_pow_reduc_timers,
    NULL,
    led_pow_reduc_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "GreenEthernet led delete_timer",
    "GreenEthernet led delete_timer <start_hour> <end_hour>",
    "Deletes a timer",
    LED_POW_REDUC_DELETE_TIMER,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_LED_POW_REDUC,
    cli_cmd_led_pow_reduc_delete_timer,
    NULL,
    led_pow_reduc_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "GreenEthernet led maintenance",
    "GreenEthernet led maintenance [<maintenance_time>] [on_at_errors|leave_at_errors]",
    "Set or show the maintenance settings",
    LED_POW_REDUC_MAINTANCE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_LED_POW_REDUC,
    cli_cmd_led_pow_reduc_maintenance,
    NULL,
    led_pow_reduc_cli_parm_table,
    CLI_CMD_FLAG_NONE
);



cli_cmd_tab_entry (
    "GreenEthernet led configuration",
    "GreenEthernet led configuration",
    "Show Led Power Reduction configuration",
    LED_POW_REDUC_CONF,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_LED_POW_REDUC,
    cli_cmd_led_pow_reduc_conf,
    NULL,
    led_pow_reduc_cli_parm_table,
    CLI_CMD_FLAG_NONE
);



/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
