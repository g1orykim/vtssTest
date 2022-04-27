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

#include "web_api.h"
#include "mirror_api.h"
#include "port_api.h"
#include "topo_api.h"
#include "msg_api.h"
#ifdef VTSS_SW_OPTION_PRIV_LVL
#include "vtss_privilege_api.h"
#include "vtss_privilege_web_api.h"
#endif

/* =================
 * Trace definitions
 * -------------- */
#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>
#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_WEB
#include <vtss_trace_api.h>
/* ============== */

/*lint -esym(459, err_msg) */ // OK - that it isn't mutex protected - It is just for error message.

/****************************************************************************/
/*  Web Handler  functions                                                  */
/****************************************************************************/
//
// Mirror handler
//
static cyg_int32 handler_config_mirror(CYG_HTTPD_STATE *p)
{
    vtss_rc               rc = VTSS_OK; // error return code
    vtss_isid_t           sid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    mirror_conf_t         conf;
    mirror_switch_conf_t  local_conf;
    int                   ct;
    char                  form_name[32];
    int                   form_value;
    port_iter_t           pit;
    static char           err_msg[100]; // Need to be static because it is set when posting data, but used getting data.


    T_D ("Mirror web access - SID =  %d", sid );

    if (redirectUnmanagedOrInvalid(p, sid)) { /* Redirect unmanaged/invalid access to handler */
        return -1;
    }

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_MIRROR)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {

        strcpy(err_msg, ""); // No errors so far :)

        mirror_mgmt_conf_get(&conf); // Get the current configuration
        rc = mirror_mgmt_switch_conf_get(sid, &local_conf);
        T_D ("Updating from web");

        // Get mirror port from WEB ( Form name = "portselect" )
        if (cyg_httpd_form_varable_int(p, "portselect", &form_value) &&
            form_value > 0) {
            T_D ("Mirror port set to %d via web", form_value);
            conf.dst_port = uport2iport(form_value);
        } else {
            T_D ("Mirroring disabled from web");
            conf.dst_port = VTSS_PORT_NO_NONE;
        }

        // Get mirror switch from WEB ( Form name = "switchselect" )
#if VTSS_SWITCH_STACKABLE
        if (vtss_stacking_enabled()) {
            if (cyg_httpd_form_varable_int(p, "switchselect", &form_value)) {
                T_D ("Mirror switch set to %d via web", form_value);
                conf.mirror_switch  = topo_usid2isid(form_value);
            }
        } else {
            conf.mirror_switch = VTSS_ISID_START;
        }
#endif /* VTSS_SWITCH_STACKABLE */

        // Get source and destination eanble checkbox values
        rc = mirror_mgmt_switch_conf_get(sid, &local_conf);


        // Loop through all front ports
        if (port_iter_init(&pit, NULL, sid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT) == VTSS_OK) {
            while (port_iter_getnext(&pit)) {

                T_RG_PORT(VTSS_TRACE_GRP_DEFAULT, pit.iport, "Mirror enable configured");
                sprintf(form_name, "mode_%d", pit.uport); // Set to the htm checkbox form name
                if (cyg_httpd_form_varable_int(p, form_name, &form_value)   && (form_value >= 0 && form_value < 4)) {
                    // form_value ok
                } else {
                    form_value = 0;
                }

                if (form_value == 0) {
                    local_conf.src_enable[pit.iport] = 0;
                    local_conf.dst_enable[pit.iport] = 0;
                } else if (form_value == 1) {
                    local_conf.src_enable[pit.iport] = 1;
                    local_conf.dst_enable[pit.iport] = 0;
                } else if (form_value == 2) {
                    local_conf.src_enable[pit.iport] = 0;
                    local_conf.dst_enable[pit.iport] = 1;
                } else {
                    // form_value is 3
                    local_conf.src_enable[pit.iport] = 1;
                    local_conf.dst_enable[pit.iport] = 1;
                }
            }
        }

#ifdef VTSS_FEATURE_MIRROR_CPU
        //
        // Getting CPU configuration
        //
        if (cyg_httpd_form_varable_int(p, "mode_CPU", &form_value)   && (form_value >= 0 && form_value < 4)) {
            // form_value ok
        } else {
            form_value = 0;
        }

        if (form_value == 0) {
            local_conf.cpu_src_enable = 0;
            local_conf.cpu_dst_enable = 0;
        } else if (form_value == 1) {
            local_conf.cpu_src_enable = 1;
            local_conf.cpu_dst_enable = 0;
        } else if (form_value == 2) {
            local_conf.cpu_src_enable = 0;
            local_conf.cpu_dst_enable = 1;
        } else {
            // form_value is 3
            local_conf.cpu_src_enable = 1;
            local_conf.cpu_dst_enable = 1;
        }
#endif

        //
        // Apply new configuration.
        //
        mirror_mgmt_switch_conf_set(sid, &local_conf); // Update switch with new configuration
        // Write new configuration
        rc = mirror_mgmt_conf_set(&conf);

        redirect(p, "/mirror.htm");
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */

        // Transfer format = mirror switch,mirror uport,sid#sid#sid|uport/src enable/dst enable,uport/src enable/dst enable,......

        cyg_httpd_start_chunked("html");

        mirror_mgmt_conf_get(&conf); // Get the current configuration

        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s,%u,%u,",
                      err_msg,
                      topo_isid2usid(conf.mirror_switch),
                      iport2uport(conf.dst_port));
        cyg_httpd_write_chunked(p->outbuffer, ct);
        T_N("cyg_httpd_write_chunked -> %s", p->outbuffer);

#if VTSS_SWITCH_STACKABLE
        vtss_usid_t usid;
        // Make list of SIDs
        for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
            vtss_usid_t isid = topo_usid2isid(usid);
            if (msg_switch_exists(isid)) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u#", usid);
                cyg_httpd_write_chunked(p->outbuffer, ct);
            }
        }
#endif /* VTSS_SWITCH_STACKABLE */


        // Insert Separator (,)
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "|");
        cyg_httpd_write_chunked(p->outbuffer, ct);

        // Make list of ports that each switch in the stack have (Corresponding to the sid list above)
#if VTSS_SWITCH_STACKABLE
        for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
            vtss_usid_t isid = topo_usid2isid(usid);
            if (msg_switch_exists(isid)) {
#else
        vtss_isid_t isid = sid;
#endif
                if (port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT) == VTSS_OK) {
                    while (port_iter_getnext(&pit)) {
                        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u#", pit.uport);
                        cyg_httpd_write_chunked(p->outbuffer, ct);
                    }
                }

                // Insert Separator (?)
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "?");
                cyg_httpd_write_chunked(p->outbuffer, ct);
#if VTSS_SWITCH_STACKABLE
            }
        }
#endif


        // Get  the SID config
        rc = mirror_mgmt_switch_conf_get(sid, &local_conf);


        // Loop through all front ports
        if (port_iter_init(&pit, NULL, sid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT) == VTSS_OK)
        {
            while (port_iter_getnext(&pit)) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s%u/%u/%u,",
                              pit.first ? "|" : "",
                              pit.uport,
                              local_conf.src_enable[pit.iport],
                              local_conf.dst_enable[pit.iport]);
                cyg_httpd_write_chunked(p->outbuffer, ct);
                T_R("cyg_httpd_write_chunked -> %s", p->outbuffer);
            }
        }

#ifdef VTSS_FEATURE_MIRROR_CPU
        // CPU port
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s/%u/%u/%s,",
                      "CPU",
                      local_conf.cpu_src_enable,
                      local_conf.cpu_dst_enable,
                      "-" /* Signal No trunking */);
        cyg_httpd_write_chunked(p->outbuffer, ct);
#endif
        cyg_httpd_end_chunked();

        strcpy(err_msg, ""); // Clear error message
    }


    if (rc != VTSS_OK)
    {
        misc_strncpyz(err_msg, mirror_error_txt(rc), 100);
    }
    return -1; // Do not further search the file system.
}

/****************************************************************************/
/*  HTTPD Table Entries                                                     */
/****************************************************************************/

CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_mirror, "/config/mirroring", handler_config_mirror);

/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
