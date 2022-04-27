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
#ifdef VTSS_SW_OPTION_PRIV_LVL
#include "vtss_privilege_api.h"
#include "vtss_privilege_web_api.h"
#endif

#include "fan_api.h"
/****************************************************************************/
/*    Trace definition                                                      */
/****************************************************************************/
#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>
#include <vtss_trace_api.h>

#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_WEB

#define VTSS_TRACE_GRP_DEFAULT 0
#define VTSS_TRACE_GRP_FAN     1
#define TRACE_GRP_CNT          2


#if defined(CYGPKG_ATHTTPD)
/****************************************************************************/
/*  Web Handler  functions                                                  */
/****************************************************************************/
//
// FAN config handler
//

cyg_int32 handler_config_fan_config(CYG_HTTPD_STATE *p)
{
    vtss_isid_t           isid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    int                   ct;
    static char           err_msg[100]; // Need to be static because it is set when posting data, but used getting data.
    fan_local_conf_t      local_conf;
    vtss_rc               rc;

    // The Data is split like this: err_msg|t_max|t_on|

    T_D ("Fan_Config  web access - ISID =  %d", isid );

    if (redirectUnmanagedOrInvalid(p, isid)) { /* Redirect unmanaged/invalid access to handler */
        return -1;
    }

    T_D ("Fan_Config  web access - ISID =  %d", isid );
#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_FAN)) {
        return -1;
    }
#endif

    T_D ("Fan_Config  web access - ISID =  %d", isid );

    // Get configuration for the selected switch
    fan_mgmt_get_switch_conf(&local_conf); // Get current configuration

    //
    // Setting new configuration
    //
    if (p->method == CYG_HTTPD_METHOD_POST) {

        // Temperature maximum
        local_conf.glbl_conf.t_max =  httpd_form_get_value_int(p, "t_max", FAN_TEMP_MIN, FAN_TEMP_MAX);


        // Temperature where to start fan.
        local_conf.glbl_conf.t_on = httpd_form_get_value_int(p, "t_on", FAN_TEMP_MIN, FAN_TEMP_MAX);

        // Set current configuration
        if ((rc = fan_mgmt_set_switch_conf(&local_conf)) != VTSS_OK) {
            T_E("%s", fan_error_txt(rc));
            misc_strncpyz(err_msg, fan_error_txt(rc), 100); // Set error message
        } else {
            strcpy(err_msg, ""); // Clear error message
        }
        redirect(p, "/fan_config.htm");
    } else {
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
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d|%d",
                      local_conf.glbl_conf.t_max,
                      local_conf.glbl_conf.t_on);

        cyg_httpd_write_chunked(p->outbuffer, ct);
        cyg_httpd_end_chunked();
    }
    return -1; // Do not further search the file system.
}


//
// FAN status handler
//

cyg_int32 handler_status_fan_status(CYG_HTTPD_STATE *p)
{
    vtss_isid_t           isid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    int                   ct;
    fan_local_status_t    switch_status;
    vtss_rc               rc;
    char                  err_msg[100]; // Buffer for holding error messages
    T_D ("Fan_Status  web access - ISID =  %d", isid );

    if (redirectUnmanagedOrInvalid(p, isid)) { /* Redirect unmanaged/invalid access to handler */
        return -1;
    }


    strcpy(err_msg, ""); // Clear error message

    // The Data is split like this: err_msg|chip_temp|fan_speed|
    // Get status for the selected switch
    if ((rc = fan_mgmt_get_switch_status(&switch_status, isid))) {
        misc_strncpyz(err_msg, fan_error_txt(rc), 100); // Set error message
    }

    cyg_httpd_start_chunked("html");

    // Apply error message (If any )
    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s|", err_msg);
    cyg_httpd_write_chunked(p->outbuffer, ct);



    //
    // Pass status to Web
    //
    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u|", switch_status.fan_speed);
    cyg_httpd_write_chunked(p->outbuffer, ct);


    u8 sensor_id;
    u8 sensor_cnt = FAN_TEMPERATURE_SENSOR_CNT(isid);
    for (sensor_id = 0; sensor_id < sensor_cnt; sensor_id++) {
        if (sensor_id == 0) {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u",
                          switch_status.chip_temp[sensor_id]);

        } else {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "¤%u",
                          switch_status.chip_temp[sensor_id]);
        }
        cyg_httpd_write_chunked(p->outbuffer, ct);
    }

    cyg_httpd_end_chunked();

    return -1; // Do not further search the file system.
}

/****************************************************************************/
/*  HTTPD Table Entries                                                     */
/****************************************************************************/

CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_fan_config, "/config/fan_config", handler_config_fan_config);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_status_fan_status, "/stat/fan_status", handler_status_fan_status);
#endif /* CYGPKG_ATHTTPD */
#endif // VTSS_SW_OPTION_WEB
/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
