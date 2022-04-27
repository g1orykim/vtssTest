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


/*lint -esym(459,err_msg)*/

#ifdef VTSS_SW_OPTION_WEB
#include "web_api.h"
#ifdef VTSS_SW_OPTION_LACP
#include "l2proto_api.h"
#endif
#ifdef VTSS_SW_OPTION_PRIV_LVL
#include "vtss_privilege_api.h"
#include "vtss_privilege_web_api.h"
#endif
#include "led_pow_reduc_api.h"
/****************************************************************************/
/*    Trace definition                                                      */
/****************************************************************************/
#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>
#include <vtss_trace_api.h>

#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_WEB

#define VTSS_TRACE_GRP_DEFAULT 0
#define VTSS_TRACE_GRP_LED_POW_REDUC     1
#define TRACE_GRP_CNT          2


#if defined(CYGPKG_ATHTTPD)
/****************************************************************************/
/*  Web Handler  functions                                                  */
/****************************************************************************/
//
// LED_POW_REDUC config handler
//

cyg_int32 handler_config_led_pow_reduc_config(CYG_HTTPD_STATE *p)
{
    int                   ct;
    static char           err_msg[100]; // Need to be static because it is set when posting data, but used getting data.
    led_pow_reduc_local_conf_t      local_conf;
    vtss_rc               rc;
    int                   timer_index, start_index, end_index;
    char                  form_name[100];
    int                   form_value;

    //
    // Setting new configuration
    //
    if (web_get_method(p, VTSS_MODULE_ID_LED_POW_REDUC) == CYG_HTTPD_METHOD_POST) {

        // Start with setting all timers to default
        if (led_pow_reduc_mgmt_timer_set(LED_POW_REDUC_TIMERS_MIN, LED_POW_REDUC_TIMERS_MAX, LED_POW_REDUC_INTENSITY_DEFAULT) != VTSS_RC_OK) {
            T_E("Could not set LED timers");
        }

        for (timer_index = 0; timer_index < LED_POW_REDUC_TIMERS_CNT; timer_index++) {
            T_D("timer_index = %d", timer_index);
            sprintf(form_name, "start_timer_%u", timer_index); // Set form name

            // Check if there exist data for this timer
            if (cyg_httpd_form_varable_int(p, form_name, &form_value)) {

                // The time at which to change LED intensity
                T_D("Timer found for timer_index = %d", timer_index);
                sprintf(form_name, "start_timer_%u", timer_index); // Set form name

                start_index = httpd_form_get_value_int(p, form_name, LED_POW_REDUC_TIMERS_MIN, LED_POW_REDUC_TIMERS_MAX);
                sprintf(form_name, "next_timer_%u", timer_index); // Set form name
                end_index = httpd_form_get_value_int(p, form_name, LED_POW_REDUC_TIMERS_MIN, LED_POW_REDUC_TIMERS_MAX);

                // Set the corresponding intensity
                sprintf(form_name, "delete_%u", timer_index); // Set form name
                if (!cyg_httpd_form_varable_find(p, form_name) ) {
                    sprintf(form_name, "timer_intensity_%u", timer_index); // Set form name
                    u8 intensity = httpd_form_get_value_int(p, form_name,
                                                            LED_POW_REDUC_INTENSITY_MIN, LED_POW_REDUC_INTENSITY_MAX);

                    if (led_pow_reduc_mgmt_timer_set(start_index, end_index, intensity) != VTSS_RC_OK) {
                        T_E("Could not set LED timers");
                    }
                }
            }
        }

        led_pow_reduc_mgmt_get_switch_conf(&local_conf); // Get current configuration

        // The amount of time the LED shall be turned on at link change
        local_conf.glbl_conf.maintenance_time = httpd_form_get_value_int(p, "maintenance_time", LED_POW_REDUC_MAINTENANCE_TIME_MIN, LED_POW_REDUC_MAINTENANCE_TIME_MAX);

        if (cyg_httpd_form_varable_find(p, "on_at_err") ) {
            local_conf.glbl_conf.on_at_err = TRUE;
        } else {
            local_conf.glbl_conf.on_at_err = FALSE;
        }

        // Set current configuration
        if ((rc = led_pow_reduc_mgmt_set_switch_conf(&local_conf)) != VTSS_OK) {
            T_E(error_txt(rc));
            misc_strncpyz(err_msg, error_txt(rc), 100); // Set error message
        } else {
            strcpy(err_msg, ""); // Clear error message
        }


        redirect(p, "/led_pow_reduc_config.htm");

    } else if (web_get_method(p, VTSS_MODULE_ID_LED_POW_REDUC) == CYG_HTTPD_METHOD_GET) {
        //
        // Getting the configuration.
        //

        // Get configuration for the selected switch
        led_pow_reduc_mgmt_get_switch_conf(&local_conf); // Get current configuration


        cyg_httpd_start_chunked("html");

        // Apply error message (If any )
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s|", err_msg);
        cyg_httpd_write_chunked(p->outbuffer, ct);
        strcpy(err_msg, ""); // Clear error message


        //
        // Pass configuration to Web
        //

        // Pass LED intensity
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d&", LED_POW_REDUC_TIMERS_CNT);
        cyg_httpd_write_chunked(p->outbuffer, ct);

        // Loop through the elements in the list
        BOOL first = TRUE;
        led_pow_reduc_timer_t led_timer;
        led_pow_reduc_mgmt_timer_get_init(&led_timer); // Prepare for looping through all timers

        // Loop through all timers and print them
        while (led_pow_reduc_mgmt_timer_get(&led_timer)) {
            if (!first) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "#");
                cyg_httpd_write_chunked(p->outbuffer, ct);
            }
            first = FALSE;

            T_I("led_timer.start_index:%d, led_timer.end_index:%d", led_timer.start_index, led_timer.end_index);
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d,%d,%d",
                          led_timer.start_index,
                          led_timer.end_index,
                          local_conf.glbl_conf.led_timer_intensity[led_timer.start_index]);

            cyg_httpd_write_chunked(p->outbuffer, ct);
        }

        // Pass maintenance time
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "|%d,%d",
                      local_conf.glbl_conf.maintenance_time,
                      local_conf.glbl_conf.on_at_err );
        cyg_httpd_write_chunked(p->outbuffer, ct);
        cyg_httpd_end_chunked();
    }
    return -1; // Do not further search the file system.
}



/****************************************************************************/
/*  HTTPD Table Entries                                                     */
/****************************************************************************/

CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_led_pow_reduc_config, "/config/led_pow_reduc_config", handler_config_led_pow_reduc_config);
#endif /* CYGPKG_ATHTTPD */
#endif  /* VTSS_SW_OPTION_WEB*/
/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
