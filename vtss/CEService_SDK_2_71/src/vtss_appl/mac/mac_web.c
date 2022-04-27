/*

   Vitesse Switch API software.

 Copyright (c) 2002-2014 Vitesse Semiconductor Corporation "Vitesse". All
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
#include "mac_api.h"
#include "port_api.h"
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
#define MAC_WEB_BUF_LEN 512
/****************************************************************************/
/*  Web Handler  functions                                                  */
/****************************************************************************/
//
// Static MAC TABLE handler
//
static void configure_learning(CYG_HTTPD_STATE *p, vtss_usid_t sid)
{
    vtss_port_no_t    iport;
    vtss_uport_no_t   uport;
    vtss_learn_mode_t learn_mode;
    char              form_name[32];
    size_t            len;
    vtss_rc           rc = VTSS_OK;
    BOOL              call_learn_mode_set;
    const char        *id;
    char              var_id[16];
    char              tmp[200];
    char              *err = NULL;
    u32               port_count = port_isid_port_count(sid);

    learn_mode.cpu       = 0; // cpu not used.

    for (iport = VTSS_PORT_NO_START; iport < port_count; iport++) {
        uport = iport2uport(iport);

        call_learn_mode_set = TRUE;

        // Get the id of the radio button that is set
        sprintf(form_name, "learn_port_%d", uport);// Radio button
        id = cyg_httpd_form_varable_string(p, form_name, &len);
        T_N("form_name = %s, id = %s", form_name , id);

        // Check the ID to figure out which radio button that was set
        sprintf(var_id, "Learn_Auto_%d", uport);
        T_N("var_id = %s " , var_id);
        if (len == strlen(var_id) && memcmp(id, var_id, len) == 0) {
            T_N("Learning for iport %d is set to automatic", iport);
            learn_mode.automatic = 1;
            learn_mode.discard   = 0;
        } else {
            sprintf(var_id, "Learn_Secure_%d", uport); // Radio button
            if (len == strlen(var_id) && memcmp(id, var_id, len) == 0) {
                T_N("Learning for iport %d is set to secure", iport);
                learn_mode.automatic = 0;
                learn_mode.discard   = 1;
            } else {
                sprintf(var_id, "Learn_Disable_%d", uport); // Radio button
                if (len == strlen(var_id) && memcmp(id, var_id, len) == 0) {
                    // Learning disabled
                    T_N("Learning for iport %d is set to disable, var_id = %s", iport, var_id );
                    learn_mode.automatic = 0;
                    learn_mode.discard   = 0;
                } else {
                    // No radio-buttons are selected (which is impossible) or the port is not
                    // allowed to be changed by the user (possible due to 802.1X module's force secure learning).
                    call_learn_mode_set = FALSE;
                }
            }
        }
        if (call_learn_mode_set) {
            rc = mac_mgmt_learn_mode_set(sid, iport, &learn_mode);
        }

        if (rc == MAC_ERROR_LEARN_FORCE_SECURE) {
            sprintf(tmp, "The learn mode cannot be changed on port %d while the learn mode is forced to 'secure' (probably by 802.1X module)", uport);
            err = tmp;
        }
    }

    if (err != NULL) {
        send_custom_error(p, "MAC Error", err, strlen(err));
    }
}

static cyg_int32 handler_config_static_mac(CYG_HTTPD_STATE *p)
{
    vtss_isid_t           sid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    vtss_port_no_t        iport;
    int                   ct;
    mac_age_conf_t        real_age_timer;
    static mac_age_conf_t age_timer;
    mac_mgmt_addr_entry_t return_mac, new_entry;
    size_t                len;
    const char            *value;
    int                   i;
    vtss_vid_mac_t        search_mac, delete_entry;
    static vtss_rc        error_num = VTSS_OK ; // Used to select an error message to be given back to the web page -- 0 = no error
    vtss_learn_mode_t     learn_mode;
    char                  form_name[32];
    uint                  vid;
    ulong                 form_lint_value;
    uint                  entry_num;
    BOOL                  skip_deleting;
    u32                   port_count = port_isid_port_count(sid);

    T_N ("Static MAC web access - SID =  %d", sid );

    if (redirectUnmanagedOrInvalid(p, sid)) { /* Redirect unmanaged/invalid access to handler */
        return -1;
    }

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_MAC)) {
        return -1;
    }
#endif

    mac_mgmt_age_time_get(&real_age_timer);
    memset(&new_entry, 0, sizeof(new_entry));

    if ( real_age_timer.mac_age_time != 0 ) {
        age_timer = real_age_timer;
    }

    if (p->method == CYG_HTTPD_METHOD_POST) {

        T_N ("p->method == CYG_HTTPD_METHOD_POST");

        if (!cyg_httpd_form_varable_find(p, "DisableAgeing") ) { // Continue if age timer is enabled
            // Get age timer from WEB (Form name = "agebox")
            if (cyg_httpd_form_varable_long_int(p, "agebox", &form_lint_value)) {
                age_timer.mac_age_time = (ulong ) form_lint_value;
                real_age_timer = age_timer;
            }

        } else {
            // Disable aging
            real_age_timer.mac_age_time = 0;
        }

        // Update age timer
        mac_mgmt_age_time_set(&real_age_timer);

        // Configure learning
        configure_learning(p, sid);

        // Delete entries
        entry_num = 1; // first entry
        sprintf(form_name, "MAC_%d", entry_num);// First MAC input box

        T_N("Looking for form: %s", form_name);
        while ((value = cyg_httpd_form_varable_string(p, form_name, &len)) && len > 0) {
            T_N("Form : %s -  found", form_name);
            skip_deleting = 0;

            sprintf(form_name, "Delete_%d", entry_num); // select next delete check box
            if (cyg_httpd_form_varable_find(p, form_name) ) {

                sprintf(form_name, "MAC_%d", entry_num);// Hidden MAC input box
                // Set mac address
                if (!cyg_httpd_form_varable_mac(p, form_name, delete_entry.mac.addr)) {
                    T_E("Hidden MAC value did not have the right format -- Shall never happen");
                    skip_deleting = 1; // Skip the deleting
                }

                // Set vid
                sprintf(form_name, "VID_%d", entry_num); // Hidden VID input box
                if (!cyg_httpd_form_varable_int(p, form_name, &vid)) {
                    T_E("Hidden entry (form name = %s) has wrong VID format -- Shall never happen", form_name);
                    skip_deleting = 1; // Skip the deleting
                    delete_entry.vid = 0;
                } else {
                    delete_entry.vid = vid;
                }

                T_D("Deleting entry %d mac address = %02x-%02x-%02x-%02x-%02x-%02x, vid = %d",
                    entry_num,
                    delete_entry.mac.addr[0], delete_entry.mac.addr[1], delete_entry.mac.addr[2],
                    delete_entry.mac.addr[3], delete_entry.mac.addr[4], delete_entry.mac.addr[5],
                    delete_entry.vid);

                if (!skip_deleting) {
                    // Do the deleting
                    mac_mgmt_table_del(sid, &delete_entry, 0);
                }
            }

            entry_num++;  // Select next entry
            sprintf(form_name, "MAC_%d", entry_num);// Hidden MAC input box
            T_N("Updating hidden MAC input box. Entry = %d", entry_num);
        }

        //
        // Adding new entries to the table
        //
        for (entry_num = 1; entry_num <=  MAC_ADDR_NON_VOLATILE_MAX ; entry_num++) {
            // If an entry exists, then add it.
            sprintf(form_name, "MAC_%d", entry_num);// First MAC input box
            // Set mac address
            if (cyg_httpd_form_varable_mac(p, form_name, new_entry.vid_mac.mac.addr)) {
                // Set vid
                sprintf(form_name, "VID_%d", entry_num); // select next MAC input box
                if (!cyg_httpd_form_varable_int(p, form_name, &vid)) {
                    T_E("New entry (form name = %s) has wrong VID format -- Shall never happen", form_name);
                    vid = 0;
                }
                new_entry.vid_mac.vid = vid;

                // Add the port mask
                for (iport = VTSS_PORT_NO_START; iport < port_count; iport++) {
                    sprintf(form_name, "Dest_%d_%d", entry_num, iport2uport(iport)); // select port check box
                    if (cyg_httpd_form_varable_find(p, form_name)) { /* "on" if checked */
                        new_entry.destination[iport] = 1;
                    } else {
                        new_entry.destination[iport] = 0;
                    }
                }

                sprintf(form_name, "Delete_%d", entry_num); // Find corresponding check box

                // Skip the adding if the delete check box is checked
                if (!cyg_httpd_form_varable_find(p, form_name)) {
                    (void)mac_mgmt_table_del(sid, &new_entry.vid_mac, 0); // we don't care if the address exists or not
                    T_N (" Do the table adding");
                    if ((error_num = mac_mgmt_table_add(sid, &new_entry)) != VTSS_OK) {
                        // give error message to web ( MAC table full ).
                        T_D("MAC Table full");
                    }
                }
            }
        } // end for loop

        // Return to the mac page when update completed.
        redirect(p, "/mac.htm");
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */

        char data_string[60];
        char learn_string[60] = "";
        char learn_chg_allowed_str[60] = "";
        BOOL chg_allowed;

        //
        // Learning configuration
        //
        for (iport = VTSS_PORT_NO_START; iport < port_count; iport++) {
            mac_mgmt_learn_mode_get(sid, iport, &learn_mode, &chg_allowed);
            learn_string[iport - VTSS_PORT_NO_START] = learn_mode.automatic ? 'A' : learn_mode.discard ? 'S' : 'D';
            learn_chg_allowed_str[iport - VTSS_PORT_NO_START] = chg_allowed ? '1' : '0';
        }
        learn_string[iport - VTSS_PORT_NO_START] = '\0';
        learn_chg_allowed_str[iport - VTSS_PORT_NO_START] = '\0';

        cyg_httpd_start_chunked("html");

        // General setup
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/%u/%s/%s/%s|",
                      age_timer.mac_age_time,
                      real_age_timer.mac_age_time == 0 ? 1 : 0,
                      learn_string,
                      learn_chg_allowed_str,
                      error_num == VTSS_OK ? "-" : error_txt(error_num));
        cyg_httpd_write_chunked(p->outbuffer, ct);

        // Clear error messages
        error_num = VTSS_OK;

        // Get entries
        memset(&search_mac, 0, sizeof(search_mac)); // Set search starting point.

        for (i = 0 ; i < MAC_ADDR_NON_VOLATILE_MAX; i++ ) {
            T_D("Next lookup %u", i);
            // We want to include the start mac address so first entry must be found using a lookup
            if (i == 0) {
                // Do a lookup for the first entry
                if (mac_mgmt_static_get_next(sid, &search_mac, &return_mac, FALSE, FALSE) != VTSS_OK) {
                    // If lookup wasn't found do a lookup of the next entry
                    T_N("Did not find lookup, sid = %d", sid);
                    if (mac_mgmt_static_get_next(sid, &search_mac, &return_mac, TRUE, FALSE) != VTSS_OK) {
                        T_N("Did not find any entries for sid: %d, search_mac = %s", sid, data_string);
                        break;
                    }
                }
            } else {
                // Find next entry
                if (mac_mgmt_static_get_next(sid, &search_mac, &return_mac, TRUE, FALSE) != VTSS_OK) {
                    T_N("Did not find any more entries for sid: %d, search_mac = %s", sid, data_string);
                    break;
                }
            }

            search_mac = return_mac.vid_mac; // Point to next entry

            sprintf(data_string, "%02X-%02X-%02X-%02X-%02X-%02X/%u",
                    return_mac.vid_mac.mac.addr[0],
                    return_mac.vid_mac.mac.addr[1],
                    return_mac.vid_mac.mac.addr[2],
                    return_mac.vid_mac.mac.addr[3],
                    return_mac.vid_mac.mac.addr[4],
                    return_mac.vid_mac.mac.addr[5],
                    return_mac.vid_mac.vid);

            for (iport = VTSS_PORT_NO_START; iport < port_count; iport++) {
                sprintf(data_string, "%s/%u", data_string, return_mac.destination[iport]);
            }

            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s|", data_string);
            cyg_httpd_write_chunked(p->outbuffer, ct);
        }
    }
    cyg_httpd_end_chunked();

    return -1; // Do not further search the file system.
}

//
// Dynamic MAC TABLE handler
//
static cyg_int32 handler_config_dynamic_mac(CYG_HTTPD_STATE *p)
{
    static BOOL            first_time = 1;
    static vtss_vid_mac_t  start_mac_addr;
    static int             num_of_entries;

    vtss_isid_t            sid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    vtss_port_no_t         iport;
    int                    ct;
    vtss_vid_mac_t         search_mac;
    vtss_mac_table_entry_t return_mac;
    uint                   mac_addr[6];
    int                    i;
    BOOL                   no_entries_found = 1;


    T_N("Dynamic MAC web access - SID =  %d", sid);

    if (redirectUnmanagedOrInvalid(p, sid)) { /* Redirect unmanaged/invalid access to handler */
        return -1;
    }

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_STAT_TYPE, VTSS_MODULE_ID_MAC)) {
        return -1;
    }
#endif

    if (first_time) {
        first_time = 0 ;
        // Clear search mac and vid
        memset(&start_mac_addr, 0, sizeof(start_mac_addr));
        num_of_entries = 20;
        start_mac_addr.vid = 1;
    }

    if (p->method == CYG_HTTPD_METHOD_POST) {
        // Return to the mac page when update completed.
        redirect(p, "/mac.htm");
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        T_D("Updating to dynamic MAC table");

        cyg_httpd_start_chunked("html");

        // Check if we shall flush
        if (var_Flush[0]) {
            T_D ("Flushing MAC table");
            mac_mgmt_table_flush();
        }

        // Check if the get next entries button is pressed
        if (var_dyn_start_mac_addr[0]) {
            // Store the start mac address
            // Convert from string to vtss_mac_t
            if  (sscanf(var_dyn_start_mac_addr, "%2x-%2x-%2x-%2x-%2x-%2x", &mac_addr[0], &mac_addr[1], &mac_addr[2], &mac_addr[3], &mac_addr[4], &mac_addr[5]) == 6 ) {
                for (i = 0 ; i <= 5; i++) {
                    start_mac_addr.mac.addr[i] = (uchar) mac_addr[i];
                }
                T_D ("Start MAC Address set to %s via web", var_dyn_start_mac_addr);
            }

            var_dyn_start_mac_addr[0] = '\0';  // Reset the /* var_dyn_start_mac_addr with a null pointer */

            // Get  number of entries per page
            num_of_entries = atoi(var_dyn_num_of_ent);
            if (num_of_entries <= 0 || num_of_entries > 999 ) {
                T_E("number of entries has wrong format (Shall never happen ),  %u var_dyn_num_of_ent = %s", num_of_entries, var_dyn_num_of_ent);
                num_of_entries = 999;
            }

            // Get start vid
            start_mac_addr.vid = atoi(var_dyn_start_vid);
            if (start_mac_addr.vid < 1 || start_mac_addr.vid > 4095 ) {
                T_E("VID has wrong format (Shall never happen), %u var_dyn_start_vid = %s", num_of_entries, var_dyn_start_vid);
                start_mac_addr.vid = 1;
            }

            T_D("number of entries set to %u " , num_of_entries);
        }



        // General setup
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/%02x-%02x-%02x-%02x-%02x-%02x|",
                      num_of_entries,
                      start_mac_addr.mac.addr[0],
                      start_mac_addr.mac.addr[1],
                      start_mac_addr.mac.addr[2],
                      start_mac_addr.mac.addr[3],
                      start_mac_addr.mac.addr[4],
                      start_mac_addr.mac.addr[5]);
        cyg_httpd_write_chunked(p->outbuffer, ct);

        // Get entries
        search_mac = start_mac_addr; // Set search starting point.

        for (i = 0 ; i < num_of_entries; i++ ) {
            T_D("search_mac_addr = %02x-%02x-%02x-%02x-%02x-%02x, vid = %u, sid = %d",
                search_mac.mac.addr[0],
                search_mac.mac.addr[1],
                search_mac.mac.addr[2],
                search_mac.mac.addr[3],
                search_mac.mac.addr[4],
                search_mac.mac.addr[5],
                search_mac.vid,
                sid);


            T_N("var_dyn_get_next_entry = %d", atoi(var_dyn_get_next_entry));
            if (i ==  0 && atoi(var_dyn_get_next_entry) == 0) {
                // We want to include the start mac address so first entry must be found using a lookup if it is the refresh button that are pressed.

                // Do a lookup for the first entry
                if (mac_mgmt_table_get_next(sid, &search_mac, &return_mac, FALSE) != VTSS_OK) {
                    // If lookup wasn't found do a lookup of the next entry
                    if (mac_mgmt_table_get_next(sid, &search_mac, &return_mac, TRUE) != VTSS_OK) {
                        break ;
                    }
                }
            } else {
                // Find next entry
                if (mac_mgmt_table_get_next(sid, &search_mac, &return_mac, TRUE) != VTSS_OK) {
                    break ;
                }
            }

            no_entries_found = 0;   // If we reached this point at least one entry is found.
            search_mac = return_mac.vid_mac; // Point to next entry

            // Generate entry
#if defined(VTSS_FEATURE_VSTAX_V2) && VTSS_SWITCH_STACKABLE /* For VSTAX 2 we add the Switch id */
            if (vtss_stacking_enabled())
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%02x-%02x-%02x-%02x-%02x-%02x/%u/%u/%u/%u",
                              return_mac.vid_mac.mac.addr[0],
                              return_mac.vid_mac.mac.addr[1],
                              return_mac.vid_mac.mac.addr[2],
                              return_mac.vid_mac.mac.addr[3],
                              return_mac.vid_mac.mac.addr[4],
                              return_mac.vid_mac.mac.addr[5],
                              return_mac.vid_mac.vid,
                              return_mac.locked,
                              mac_mgmt_upsid2usid(return_mac.vstax2.upsid),
                              return_mac.copy_to_cpu);
            else
#endif
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%02x-%02x-%02x-%02x-%02x-%02x/%u/%u/%u",
                              return_mac.vid_mac.mac.addr[0],
                              return_mac.vid_mac.mac.addr[1],
                              return_mac.vid_mac.mac.addr[2],
                              return_mac.vid_mac.mac.addr[3],
                              return_mac.vid_mac.mac.addr[4],
                              return_mac.vid_mac.mac.addr[5],
                              return_mac.vid_mac.vid,
                              return_mac.locked,
                              return_mac.copy_to_cpu);

            cyg_httpd_write_chunked(p->outbuffer, ct);
            T_D ("Return mac address = %s ", p->outbuffer);
            // Add destination
            char *buf = p->outbuffer;
            for (iport = VTSS_PORT_NO_START; iport < VTSS_PORT_NO_END; iport++) {
                buf += snprintf(buf, sizeof(p->outbuffer) - (buf - p->outbuffer), "/%u", return_mac.destination[iport]);

            }
            *buf++ = '|';
            cyg_httpd_write_chunked(p->outbuffer, (buf - p->outbuffer));
        }

        // Signal to WEB that no entries was found.
        if (no_entries_found) {
#if defined(VTSS_FEATURE_VSTAX_V2) && VTSS_SWITCH_STACKABLE /* For VSTAX 2 we add the Switch id */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "NoEntries/-/-/-/-|");
#else
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "NoEntries/-/-/-|");
#endif

            cyg_httpd_write_chunked(p->outbuffer, ct);
        }

        cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

/****************************************************************************/
/*  Module JS lib config routine                                            */
/****************************************************************************/

static size_t mac_lib_config_js(char **base_ptr, char **cur_ptr, size_t *length)
{
    char buff[MAC_WEB_BUF_LEN];
    (void) snprintf(buff, MAC_WEB_BUF_LEN,
                    "var configMacStaticMax = %d;\n",
                    MAC_ADDR_NON_VOLATILE_MAX);
    return webCommonBufferHandler(base_ptr, cur_ptr, length, buff);
}

/****************************************************************************/
/*  JS lib config table entry                                               */
/****************************************************************************/

web_lib_config_js_tab_entry(mac_lib_config_js);


/****************************************************************************/
/*  HTTPD Handler Table Entries                                             */
/****************************************************************************/

CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_static_mac_table, "/config/static_mac_table", handler_config_static_mac);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_dynamic_mac_table, "/config/dynamic_mac_table", handler_config_dynamic_mac);

/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
