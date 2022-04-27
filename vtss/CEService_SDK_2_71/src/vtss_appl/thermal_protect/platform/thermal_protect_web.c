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
#include "l2proto_api.h"


#include "thermal_protect_api.h"
#ifdef VTSS_SW_OPTION_PRIV_LVL
#include "vtss_privilege_api.h"
#include "vtss_privilege_web_api.h"
#endif


/****************************************************************************/
/*    Trace definition                                                      */
/****************************************************************************/
#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>
#include <vtss_trace_api.h>

#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_WEB

#define VTSS_TRACE_GRP_DEFAULT 0
#define VTSS_TRACE_GRP_THERMAL_PROTECT     1
#define TRACE_GRP_CNT          2


#if defined(CYGPKG_ATHTTPD)
/****************************************************************************/
/*  Web Handler  functions                                                  */
/****************************************************************************/
//
// THERMAL_PROTECT config handler
//

cyg_int32 handler_config_thermal_protect_config(CYG_HTTPD_STATE *p)
{
    vtss_isid_t           isid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    int                   ct;
    static char           err_msg[100]; // Need to be static because it is set when posting data, but used getting data.
    thermal_protect_switch_conf_t      switch_conf, newconf;
    vtss_rc               rc;
    char                  form_name[50];
    int                   i;
    port_iter_t           pit;

    //
    // Setting new configuration
    //
    if (web_get_method(p, VTSS_MODULE_ID_THERMAL_PROTECT) == CYG_HTTPD_METHOD_POST) {
        // Get configuration for the selected switch
        thermal_protect_mgmt_switch_conf_get(&switch_conf, isid); // Get current configuration
        newconf = switch_conf;

        // Priority Temperatures
        for (i = 0; i < THERMAL_PROTECT_PRIOS_CNT; i++) {
            sprintf(form_name, "thermal_prio_temp_%d", i); // Set form name
            newconf.glbl_conf.prio_temperatures[i] =  httpd_form_get_value_int(p, form_name, THERMAL_PROTECT_TEMP_MIN, THERMAL_PROTECT_TEMP_MAX);
        }

        // Loop through all front ports
        if (port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT | PORT_ITER_FLAGS_NPI) == VTSS_OK) {
            while (port_iter_getnext(&pit)) {
                // Priority where to start thermal_protect per port.
                sprintf(form_name, "thermal_port_prio_%u", pit.iport); // Set form name
                newconf.local_conf.port_prio[pit.iport] = httpd_form_get_value_int(p, form_name, THERMAL_PROTECT_TEMP_MIN, THERMAL_PROTECT_TEMP_MAX);
                T_D_PORT(pit.iport, "port_prio = %lu", newconf.local_conf.port_prio[pit.iport]);
            }
        }

        // Set current configuration
        if (memcmp(&switch_conf, &newconf, sizeof(newconf)) && (rc = thermal_protect_mgmt_switch_conf_set(&newconf, isid)) != VTSS_OK) {
            misc_strncpyz(err_msg, error_txt(rc), 100); // Set error message
        } else {
            strcpy(err_msg, ""); // Clear error message
        }

        redirect(p, "/thermal_protect_config.htm");

    } else if (web_get_method(p, VTSS_MODULE_ID_THERMAL_PROTECT) == CYG_HTTPD_METHOD_GET) {
        //
        // Getting the configuration.
        //

        cyg_httpd_start_chunked("html");

        // Apply error message (If any )
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s|", err_msg);
        cyg_httpd_write_chunked(p->outbuffer, ct);
        strcpy(err_msg, ""); // Clear error message


        //
        // Pass configuration to Web
        //

        // Get configuration for the global configuration
        thermal_protect_mgmt_switch_conf_get(&switch_conf, VTSS_ISID_LOCAL); // Get current configuration
        for (i = 0; i < THERMAL_PROTECT_PRIOS_CNT; i++) {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d",
                          switch_conf.glbl_conf.prio_temperatures[i]);
            cyg_httpd_write_chunked(p->outbuffer, ct);

            // Split with "," between each priority temperature, and end with a "|".
            if (i == THERMAL_PROTECT_PRIOS_CNT - 1) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "|");
            } else {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), ",");
            }
            cyg_httpd_write_chunked(p->outbuffer, ct);
        }

        // Port priorities
        // Get configuration for the local configuration for the switch
        thermal_protect_mgmt_switch_conf_get(&switch_conf, isid); // Get current configuration
        // Loop through all front ports
        if (port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT | PORT_ITER_FLAGS_NPI) == VTSS_OK) {
            while (port_iter_getnext(&pit)) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d",
                              switch_conf.local_conf.port_prio[pit.iport]);
                cyg_httpd_write_chunked(p->outbuffer, ct);

                // Split with "," between each priority
                if (!pit.last) {
                    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), ",");
                    cyg_httpd_write_chunked(p->outbuffer, ct);
                }
            }
        }
        cyg_httpd_end_chunked();
    }
    return -1; // Do not further search the file system.
}

//
// THERMAL_PROTECT status handler
//
cyg_int32 handler_status_thermal_protect_status(CYG_HTTPD_STATE *p)
{
    vtss_isid_t           isid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    int                   ct;
    thermal_protect_local_status_t    switch_status;
    vtss_rc               rc;
    char                  err_msg[100]; // Buffer for holding error messages
    port_iter_t           pit;

    if (web_get_method(p, VTSS_MODULE_ID_THERMAL_PROTECT) == CYG_HTTPD_METHOD_GET) {
        strcpy(err_msg, ""); // Clear error message

        // The Data is split like this: err_msg|chip_temp|thermal_protect_speed|

        // Get status for the selected switch
        if ((rc = thermal_protect_mgmt_get_switch_status(&switch_status, isid))) {
            misc_strncpyz(err_msg, error_txt(rc), 100); // Set error message
        }

        cyg_httpd_start_chunked("html");

        // Apply error message (If any )
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s|", err_msg);
        cyg_httpd_write_chunked(p->outbuffer, ct);

        //
        // Pass status to Web
        //
        // Loop through all front ports
        if (port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT | PORT_ITER_FLAGS_NPI) == VTSS_OK) {
            while (port_iter_getnext(&pit)) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u&%s|",
                              switch_status.port_temp[pit.iport],
                              thermal_protect_power_down2txt(switch_status.port_powered_down[pit.iport]));

                cyg_httpd_write_chunked(p->outbuffer, ct);
            }
        }
        cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

/****************************************************************************/
/*  HTTPD Table Entries                                                     */
/****************************************************************************/

CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_thermal_protect_config, "/config/thermal_protect_config", handler_config_thermal_protect_config);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_status_thermal_protect_status, "/stat/thermal_protect_status", handler_status_thermal_protect_status);
#endif /* CYGPKG_ATHTTPD */
#endif //VTSS_SW_OPTION_WEB
/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
