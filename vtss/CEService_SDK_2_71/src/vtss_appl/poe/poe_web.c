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
#include "poe_api.h"
#include "poe_custom_api.h"
/* =================
 * Trace definitions
 * -------------- */
#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>

#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_WEB

#define VTSS_TRACE_GRP_DEFAULT 0
#define VTSS_TRACE_GRP_POE     1
#define TRACE_GRP_CNT          2

#ifdef VTSS_SW_OPTION_PRIV_LVL
#include "vtss_privilege_web_api.h"
#endif

#include <vtss_trace_api.h>
/* ============== */

/****************************************************************************/
/*  Web Handler  functions                                                  */
/****************************************************************************/
//
// Power Over Ethernet POE handler
//
static cyg_int32 handler_config_poe(CYG_HTTPD_STATE *p)
{
    vtss_isid_t     isid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    char            form_name[24];
    int             hidden_uport;
    port_iter_t     pit;
    vtss_port_no_t  iport, form_index;


    if (redirectUnmanagedOrInvalid(p, isid)) { /* Redirect unmanaged/invalid access to handler */
        return -1;
    }

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_POE)) {
        return -1;
    }
#endif


    poe_chipset_t poe_chip_found[VTSS_PORTS];
    poe_mgmt_is_chip_found(isid, &poe_chip_found[0]);

    T_N("Web Accessing PoE configuration- isid = %d", isid);

    poe_master_conf_t poe_master_conf;
    poe_mgmt_get_master_config(&poe_master_conf); // Get current configuration for the master

    poe_local_conf_t poe_local_conf;
    poe_mgmt_get_local_config(&poe_local_conf, isid);

    //
    // Setting new configuration
    //
    if (p->method == CYG_HTTPD_METHOD_POST) {
        T_N("Web Setting PoE configuration");
        const char *id;
        size_t len;
        BOOL mgmt_mode_reserved;
        id = cyg_httpd_form_varable_string(p, "mode_group", &len);
        if (id != NULL && len == strlen("ActualConsumption") && memcmp(id, "ActualConsumption", len) == 0) {
            mgmt_mode_reserved = FALSE;
        } else {
            mgmt_mode_reserved = TRUE;
        }

        id = cyg_httpd_form_varable_string(p, "base_group", &len);
        //
        // Parameters that are only valid for the master
        //
        // Get management configuration
        if (id != NULL && len == strlen("Class") && memcmp(id, "Class", len) == 0) {
            T_D("Class radio button was set");
            if (mgmt_mode_reserved) {
                poe_master_conf.power_mgmt_mode = CLASS_RESERVED;
            } else {
                poe_master_conf.power_mgmt_mode = CLASS_CONSUMP;
            }


        } else if (id != NULL && len == strlen("Allocation") && memcmp(id, "Allocation", len) == 0) {
            T_D("Allocated radio button was set");
            if (mgmt_mode_reserved) {
                poe_master_conf.power_mgmt_mode = ALLOCATED_RESERVED;
            } else {
                poe_master_conf.power_mgmt_mode = ALLOCATED_CONSUMP;
            }
        } else if (id != NULL && len == strlen("LLDP-Med") && memcmp(id, "LLDP-Med", len) == 0) {
            T_D("LLDP-Med radio button was set");
            if (mgmt_mode_reserved) {
                poe_master_conf.power_mgmt_mode = LLDPMED_RESERVED;
            } else {
                poe_master_conf.power_mgmt_mode = LLDPMED_CONSUMP;
            }
        } else {
            T_E ("Unknown radio button - defaulting management mode to CLASS");
            poe_master_conf.power_mgmt_mode = CLASS_CONSUMP;
        }


        // Write back the configuration
        poe_mgmt_set_master_config(&poe_master_conf);

        //
        // Parameters that are valid for all switches
        //

        // Power supplies
        poe_local_conf.primary_power_supply    = httpd_form_get_value_int(p, "PrimaryPowerSupply", 0, POE_POWER_SUPPLY_MAX);
        if (poe_mgmt_is_backup_power_supported()) {
            poe_local_conf.backup_power_supply = httpd_form_get_value_int(p, "BackupPowerSupply",  0, POE_POWER_SUPPLY_MAX);
        }


        // Priority, PoE enable and  Max. Port Power
        for (form_index = 0; form_index < VTSS_PORTS; form_index++) {
            // Because some ports might not support PoE, a hidden web form shows the real port number for the corresponding
            // web form. We do only continue if the hidden port number is found.
            sprintf(form_name, "hidden_portno_%u", form_index);
            if (cyg_httpd_form_varable_int(p, form_name, &hidden_uport)) {
                iport = uport2iport(hidden_uport);

                // PoE mode
                sprintf(form_name, "hidden_poe_mode_%u", form_index); // Set form name
                poe_local_conf.poe_mode[iport] = httpd_form_get_value_int(p, &form_name[0], 0, 2);

                // Max power.
                sprintf(form_name, "hidden_max_power_%u", form_index);
                poe_local_conf.max_port_power[iport] = httpd_form_get_value_int(p, &form_name[0], 0, poe_custom_get_port_power_max(iport));

                // Priority
                sprintf(form_name, "hidden_priority_%u", form_index); // Set form name
                poe_local_conf.priority[iport] = httpd_form_get_value_int(p, &form_name[0], 0, 3);
                T_D_PORT(iport, "poe_local_conf.poe_mode=%d, poe_local_conf.max_port_power=%d, poe_local_conf.priority=%d",
                         poe_local_conf.poe_mode[iport], poe_local_conf.max_port_power[iport], poe_local_conf.priority[iport]);
            }
        }


        // Write back the configuration
        poe_mgmt_set_local_config(&poe_local_conf, isid);

        redirect(p, "/poe_config.htm");

    } else {

        //
        // Getting the configuration.
        //
        T_N("Web Getting PoE");
        int ct;

        cyg_httpd_start_chunked("html");
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d|%d/%d/%d/%d/%d/%d|",
                      poe_master_conf.power_mgmt_mode,
                      poe_local_conf.primary_power_supply,
                      poe_local_conf.backup_power_supply,
                      poe_mgmt_is_backup_power_supported(),// Hardware supports backup power supply. Signal that to the web interface
                      poe_custom_get_power_supply_min(),
                      POE_POWER_SUPPLY_MAX,
#ifdef VTSS_SW_OPTION_LLDP
// signal that LLDP is supported
                      1
#else
// signal that LLDP is NOT supported
                      0
#endif

                     );

        cyg_httpd_write_chunked(p->outbuffer, ct);

        if (port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT) == VTSS_OK) {
            while (port_iter_getnext(&pit)) {
                // Pass configuration to Web
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/%d/%d/%d/%d,",
                              pit.uport,
                              poe_local_conf.priority[pit.iport],
                              poe_local_conf.max_port_power[pit.iport],
                              poe_local_conf.poe_mode[pit.iport],
                              poe_chip_found[pit.iport] != NO_POE_CHIPSET_FOUND);
                cyg_httpd_write_chunked(p->outbuffer, ct);
            } // End for loop
        }
        T_N("cyg_httpd_write_chunked -> %s", p->outbuffer);
        cyg_httpd_end_chunked();

    }
    return -1; // Do not further search the file system.
}

// Status
static cyg_int32 handler_status_poe(CYG_HTTPD_STATE *p)
{
    vtss_isid_t    isid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    port_iter_t    pit;
    char           err_msg[100];
    char           class_str[10];
    strcpy(err_msg, ""); // No errors so far :)

    if (redirectUnmanagedOrInvalid(p, isid)) { /* Redirect unmanaged/invalid access to handler */
        return -1;
    }

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_STAT_TYPE, VTSS_MODULE_ID_POE)) {
        return -1;
    }
#endif

    poe_chipset_t   poe_chip_found[VTSS_PORTS];
    poe_mgmt_is_chip_found(isid, &poe_chip_found[0]);

    T_D("Web Accessing PoE Status");
    poe_status_t          poe_status;

    // Get the status fields.
    poe_mgmt_get_status(isid, &poe_status);

    poe_local_conf_t poe_local_conf;
    poe_mgmt_get_local_config(&poe_local_conf, isid);

    //
    // Getting the status
    //
    T_D("Web Getting PoE Status");
    int ct;

    cyg_httpd_start_chunked("html");

    // Update with error message.
    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s¤",
                  err_msg);

    cyg_httpd_write_chunked(p->outbuffer, ct);



    if (port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT) == VTSS_OK) {
        while (port_iter_getnext(&pit)) {
            T_D("PoE - %u/%d/%d/%d/%d",
                pit.uport,
                poe_status.power_allocated[pit.iport],
                poe_status.power_used[pit.iport],
                poe_local_conf.priority[pit.iport],
                poe_status.current_used[pit.iport]
               );

            // Pass configuration to Web
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/%d/%d/%d/%d/%s/%s/%d|",
                          pit.uport,
                          poe_status.power_requested[pit.iport],
                          poe_status.power_used[pit.iport],
                          poe_local_conf.priority[pit.iport],
                          poe_status.current_used[pit.iport],
                          poe_status2str(poe_status.port_status[pit.iport], pit.iport, &poe_local_conf),
                          poe_class2str(&poe_status, pit.iport, &class_str[0]),
                          poe_status.power_allocated[pit.iport]);
            cyg_httpd_write_chunked(p->outbuffer, ct);
        }
    }

    T_D("cyg_httpd_write_chunked -> %s", p->outbuffer);
    cyg_httpd_end_chunked();
    return -1; // Do not further search the file system.
}

/****************************************************************************/
/*  HTTPD Table Entries                                                     */
/****************************************************************************/

CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_poe, "/config/poe_config", handler_config_poe);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_status_poe, "/stat/poe_status", handler_status_poe);

/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
