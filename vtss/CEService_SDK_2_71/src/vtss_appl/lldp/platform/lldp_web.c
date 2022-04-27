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
#include "lldp_api.h"
#include "lldp_remote.h"
#include "lldp_tlv.h"
#ifdef VTSS_SW_OPTION_LLDP_MED
#include "lldpmed_rx.h"
#endif
#ifdef VTSS_SW_OPTION_LLDP_ORG
#include "lldporg_spec_tlvs_rx.h"
#endif
#include "l2proto_api.h"
#ifdef VTSS_SW_OPTION_AGGR
#include "aggr_api.h"
#endif
#ifdef VTSS_SW_OPTION_PRIV_LVL
#include "vtss_privilege_api.h"
#include "vtss_privilege_web_api.h"
#endif

#ifdef VTSS_SW_OPTION_EEE
#include "eee_api.h"
#endif
#define LLDP_WEB_BUF_LEN 512


/* =================
* Trace definitions
* -------------- */
#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>
#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_WEB
#include <vtss_trace_api.h>
/* ============== */

//#define DO_WEB_TEST(p, module)  if (do_web_test(p, module) == -1) { return -1; }

/****************************************************************************/
/*  Web Handler  functions                                                  */
/****************************************************************************/

// Function that returns the value from a web form containing a long. It checks
// if the value is within an allowed range given by min_value and max_value (both
// values included) . If the value isn't within the allowed ranged an error message
// is thrown, and the minimum value is returned.
#ifdef VTSS_SW_OPTION_LLDP_MED
static ulong httpd_form_get_value_ulong_int(CYG_HTTPD_STATE *p, const char form_name[255], ulong min_value, ulong max_value)
{
    ulong form_value;
    if (cyg_httpd_form_varable_long_int(p, form_name, &form_value)) {
        if (form_value < min_value || form_value > max_value) {
            T_E("Invalid value. Form name = %s, form value = %u", form_name, form_value );
            form_value =  min_value;
        }
    } else {
        T_E("Unknown form. Form name = %s", form_name);
        form_value =  min_value;
    }

    return form_value;
}
#endif
//
// LLDP neighbors handler
//
static cyg_int32 handler_config_lldp_neighbor(CYG_HTTPD_STATE *p)
{
    vtss_isid_t         sid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    int                 ct;
    int                 entry_index;
    char                no_entries_found = 1;
    lldp_remote_entry_t *entry;
    lldp_remote_entry_t *table = NULL;
    port_iter_t         pit;
    u8                  mgmt_addr_index;

    if (web_get_method(p, VTSS_MODULE_ID_LLDP) == CYG_HTTPD_METHOD_GET) {
        cyg_httpd_start_chunked("html");

        lldp_mgmt_get_lock();

        table = lldp_mgmt_get_entries(sid); // Get the entries table.


        // Loop through all front ports
        if (port_iter_init(&pit, NULL, sid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT) == VTSS_OK) {
            while (port_iter_getnext(&pit)) {

                // Sort the entries by local port.
                entry = table;       // Restart the entries
                for (entry_index = 0 ; entry_index < lldp_remote_get_max_entries(); entry_index++) {
                    if (entry->in_use == 0 || entry->receive_port != pit.iport) {
                        // This is the sorting of the entries
                        entry++;
                        continue;
                    }
                    if ((entry->in_use)) {
                        char chassis_string[255]     = "";
                        char capa_string[255]        = "";
                        char port_id_string[255]     = "";
                        char mgmt_addr_string[255]   = "";
                        char system_name_string[255] = "";
                        char port_descr_string[255]  = "";
                        char receive_port[255]       = "";
                        no_entries_found = 0;
                        lldp_remote_chassis_id_to_string(entry, &chassis_string[0]);
                        lldp_remote_system_capa_to_string(entry, &capa_string[0]);
                        lldp_remote_port_id_to_string(entry, &port_id_string[0]);
                        (void) lldp_remote_receive_port_to_string(entry->receive_port, &receive_port[0], sid);
                        lldp_remote_system_name_to_string(entry, &system_name_string[0]);
                        lldp_remote_port_descr_to_string(entry, &port_descr_string[0]);
                        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s?%s?%s?%s?%s?%s?",
                                      receive_port,
                                      chassis_string,
                                      port_id_string,
                                      system_name_string,
                                      port_descr_string,
                                      capa_string
                                     );
                        cyg_httpd_write_chunked(p->outbuffer, ct);

                        BOOL add_split = FALSE;
                        for (mgmt_addr_index = 0; mgmt_addr_index < LLDP_MGMT_ADDR_CNT; mgmt_addr_index++) {
                            lldp_remote_mgmt_addr_to_string(entry, &mgmt_addr_string[0], FALSE, mgmt_addr_index);
                            if (entry->mgmt_addr[mgmt_addr_index].length > 0) {
                                if (add_split) {
                                    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "¤ %s",
                                                  mgmt_addr_string
                                                 );
                                } else {
                                    add_split = TRUE;
                                    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s",
                                                  mgmt_addr_string
                                                 );
                                }
                                cyg_httpd_write_chunked(p->outbuffer, ct);
                            }
                        }

                        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s|", "");
                        cyg_httpd_write_chunked(p->outbuffer, ct);

                        T_R("cyg_httpd_write_chunked -> %s", p->outbuffer);
                    }
                    entry++;
                }
            }
        }
        lldp_mgmt_get_unlock();

        if (no_entries_found) {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s|", "");
            cyg_httpd_write_chunked(p->outbuffer, ct);
        }
        cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

//
// LLDP-MED neighbors handler
//
#ifdef VTSS_SW_OPTION_LLDP_MED
static cyg_int32 handler_lldp_neighbor_med(CYG_HTTPD_STATE *p)
{
    vtss_isid_t         sid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    int                 ct;
    int                 entry_index;
    char                no_entries_found = 1;
    lldp_remote_entry_t *entry;
    lldp_remote_entry_t *table = NULL;
    uint                p_index = 0;
    port_iter_t          pit;

    if (web_get_method(p, VTSS_MODULE_ID_LLDP) == CYG_HTTPD_METHOD_GET) {

        cyg_httpd_start_chunked("html");

        lldp_mgmt_get_lock();

        table = lldp_mgmt_get_entries(sid); // Get the entries table.

        // Loop through all front ports
        if (port_iter_init(&pit, NULL, sid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT) == VTSS_OK) {
            while (port_iter_getnext(&pit)) {

                // Sort the entries by local port.
                entry = table;       // Restart the entries
                for (entry_index = 0 ; entry_index < lldp_remote_get_max_entries(); entry_index++) {
                    if (entry->in_use == 0 || entry->receive_port != pit.iport) {
                        // This is the sorting of the entries
                        entry++;
                        continue;
                    }

                    if (entry->lldpmed_info_vld) {
                        no_entries_found = 0;
                        // Port + Capability
                        char device_type_str[255] = "";
                        lldpmed_device_type2str(entry, &device_type_str[0]);

                        char capa_str[255] = "";
                        lldpmed_capabilities2str(entry, &capa_str[0]);

                        char receive_port_str[255]       = "";
                        (void) lldp_remote_receive_port_to_string(entry->receive_port, &receive_port_str[0], sid);

                        // Local Port
                        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s?",
                                      receive_port_str);
                        cyg_httpd_write_chunked(p->outbuffer, ct);

                        T_D("device_type_str = %s", device_type_str);
                        // Device type + capabilities
                        if (entry->lldpmed_capabilities_vld) {
                            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s&%s",
                                          device_type_str,
                                          capa_str
                                         );
                            cyg_httpd_write_chunked(p->outbuffer, ct);
                        }


                        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "?%d~",
                                      lldpmed_get_policies_cnt(entry));
                        cyg_httpd_write_chunked(p->outbuffer, ct);
                        if (lldpmed_get_policies_cnt(entry) > 0) {
                            for (p_index = 0; p_index < MAX_LLDPMED_POLICY_APPLICATIONS_CNT; p_index ++) {
                                // Policies

                                char appl_str[255]     = "";
                                lldpmed_policy_appl_type2str(entry->lldpmed_policy[p_index], &appl_str[0]);

                                char policy_str[255]   = "";
                                lldpmed_policy_flag_type2str(entry->lldpmed_policy[p_index], &policy_str[0]);

                                char tag_str[255] = "";
                                lldpmed_policy_tag2str(entry->lldpmed_policy[p_index], &tag_str[0]);

                                char vlan_str[255]  = "";
                                lldpmed_policy_vlan_id2str(entry->lldpmed_policy[p_index], &vlan_str[0]);

                                char prio_str[255]       = "";
                                lldpmed_policy_prio2str(entry->lldpmed_policy[p_index], &prio_str[0]);

                                char dscp_str[255]       = "";
                                lldpmed_policy_dscp2str(entry->lldpmed_policy[p_index], &dscp_str[0]);

                                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s¤%s¤%s¤%s¤%s¤%s~",
                                              appl_str,
                                              policy_str,
                                              tag_str,
                                              vlan_str,
                                              prio_str,
                                              dscp_str
                                             );
                                cyg_httpd_write_chunked(p->outbuffer, ct);

                                // Check if the next policy contains valid infomation.
                                if (p_index < MAX_LLDPMED_POLICY_APPLICATIONS_CNT - 1) {
                                    if (entry->lldpmed_policy_vld[p_index + 1] == 0) {
                                        break;
                                    }
                                }
                            }
                        }

                        // location  information is split as this <Number of locations tlvs>~<location TLV 1>~<location TLV 2>~<location TLV 2>
                        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "?%d",
                                      lldpmed_get_locations_cnt(entry));
                        cyg_httpd_write_chunked(p->outbuffer, ct);

                        char location_str[1000]   = "";
                        for (p_index = 0; p_index < MAX_LLDPMED_LOCATION_CNT; p_index ++) {
                            if (entry->lldpmed_location_vld[p_index] == 1) {
                                lldpmed_location2str(entry, &location_str[0], p_index);
                                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "~%s",
                                              location_str
                                             );
                                cyg_httpd_write_chunked(p->outbuffer, ct);
                            }
                        }


                        //
                        // MAC_PHY
                        //


                        cyg_httpd_write_chunked("?", 1); // Insert "splitter" for MAC_PHY data

                        if (entry->lldporg_autoneg_vld) {
                            // mac_phy conf is split as this <Autoneg Support>~<Autoneg status>~<Autoneg capa>~<Mau type>

                            char lldporg_str[255]     = "";
                            lldporg_autoneg_support2str(entry, &lldporg_str[0]);
                            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s", lldporg_str);
                            cyg_httpd_write_chunked(p->outbuffer, ct);
                            cyg_httpd_write_chunked("~", 1); // Insert "splitter" between <Autoneg Support> and <Autoneg status>

                            lldporg_autoneg_status2str(entry, &lldporg_str[0]);
                            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s", lldporg_str);
                            cyg_httpd_write_chunked(p->outbuffer, ct);
                            cyg_httpd_write_chunked("~", 1); // Insert "splitter" between <Autoneg status> and <Autoneg capa>

                            lldporg_autoneg_capa2str(entry, &lldporg_str[0]);
                            if (strlen(lldporg_str) > 0) {
                                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s", lldporg_str);
                                cyg_httpd_write_chunked(p->outbuffer, ct);
                            }
                            cyg_httpd_write_chunked("~", 1); // Insert "splitter" between <Autoneg capa> and <mau type>

                            lldporg_operational_mau_type2str(entry, &lldporg_str[0]);
                            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s", lldporg_str);
                            cyg_httpd_write_chunked(p->outbuffer, ct);
                        }


                        // Next Entry
                        cyg_httpd_write_chunked("|", 1);
                    }
                    entry++;
                }
            }
        }
        lldp_mgmt_get_unlock();

        if (no_entries_found) {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s|", "");
            cyg_httpd_write_chunked(p->outbuffer, ct);
        }
        cyg_httpd_end_chunked();

    }
    return -1; // Do not further search the file system.
}

cyg_int32 handler_lldpmed_config(CYG_HTTPD_STATE *p)
{
    vtss_isid_t           sid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    uchar                 p_index = 0;
    int                   ct;
    char                  form_name[24];
    int                   form_value;
    size_t                len;
    BOOL                  insert_comma;
    static char           err_msg[100]; // Need to be static because it is set when posting data, but used getting data.
    port_iter_t           pit;
    T_D ("lldpmed_config  web access - SID =  %d", sid );

    const char *str;
    int str_ptr = 0;
    char tmp_str[255];
    int ca_index = 0;
    char ca_cnt  = 0; // Counter the number of CA in the Civic Address LCI, Figure 10, TIA1057


    lldp_struc_0_t conf;
    lldp_mgmt_get_config(&conf, sid); // Get current configuration

    if (web_get_method(p, VTSS_MODULE_ID_LLDP) == CYG_HTTPD_METHOD_POST) {

        strcpy(err_msg, ""); // No errors so far :)
        //
        // Setting new configuration
        //

        // Get information from WEB
        conf.medFastStartRepeatCount     = httpd_form_get_value_int(p, "fast_start_repeat_count_value", FAST_START_REPEAT_COUNT_MIN, FAST_START_REPEAT_COUNT_MAX); // Limits defined in medFastStartRepeatCount MIB in TIA1057
        conf.location_info.latitude      = httpd_form_get_value_ulong_int(p, "latitude_integer", 0, 9000000);
        conf.location_info.latitude_dir  = httpd_form_get_value_int(p, "latitude_dir", 0, 1);
        conf.location_info.longitude     = httpd_form_get_value_ulong_int(p, "longitude_integer", 0, 180000000);
        conf.location_info.longitude_dir = httpd_form_get_value_int(p, "longitude_dir", 0, 1);
        conf.location_info.altitude      = httpd_form_get_value_ulong_int(p, "altitude_integer", 0, 0xFFFFFFFF);
        conf.location_info.altitude_type = httpd_form_get_value_int(p, "altitude_type", 1, 2);
        conf.location_info.datum         = httpd_form_get_value_int(p, "map_datum_type", 1, 3);
//      T_D("latitude = %lu, longitude = %lu, altitude = %lu",conf.location_info.latitude ,conf.location_info.longitude, conf.location_info.altitude);
        T_D("longitude = %u, altitude = %u", conf.location_info.longitude, conf.location_info.altitude);


        // Definition of form names, see lldp_med_config.htm
        char *civic_form_names[] = {"state",
                                    "county",
                                    "city",
                                    "city_district",
                                    "block",
                                    "street",
                                    "leading_street_direction",
                                    "trailing_street_suffix",
                                    "str_suf",
                                    "house_no",
                                    "house_no_suffix",
                                    "landmark",
                                    "additional_info",
                                    "name",
                                    "zip_code",
                                    "building",
                                    "apartment",
                                    "floor",
                                    "room_number",
                                    "place_type",
                                    "postal_com_name",
                                    "p_o_box",
                                    "additional_code"
                                   };

        // Country code
        str = cyg_httpd_form_varable_string(p, "country_code", &len);
        strcpy(tmp_str, "");

        if (len > 0) {
            (void) cgi_unescape(str, tmp_str, len, sizeof(tmp_str));
        }

        misc_strncpyz(conf.location_info.ca_country_code, tmp_str, CA_COUNTRY_CODE_LEN);


        // Ca value
        for (ca_index = 0; ca_index < LLDPMED_CATYPE_CNT; ca_index++ ) {
            str = cyg_httpd_form_varable_string(p, civic_form_names[ca_index], &len);
            // make sure that we don't writes outside the array
            if ((len + str_ptr + ca_cnt * 2) > CIVIC_CA_VALUE_LEN_MAX) { // Each CA takes 2 bytes (one for CA type and one for CA length), Figure 10, TIA1057
                strcpy(err_msg, "Total infomation for Civic Address Location exceeds maximum allowed charaters");
                goto stop;
            }

            conf.location_info.civic.civic_str_ptr_array[ca_index] = str_ptr;
            strcpy(tmp_str, "");

            (void) cgi_unescape(str, tmp_str, len, sizeof(tmp_str));
            if (len > 0) {
                strcpy(&conf.location_info.civic.ca_value[str_ptr], tmp_str);
                str_ptr += strlen(tmp_str) + 1;
                ca_cnt++;
            } else {
                conf.location_info.civic.ca_value[str_ptr] = '\0';
                str_ptr ++;
            }
            conf.location_info.civic.civic_ca_type_array[ca_index] = lldpmed_index2catype(ca_index); // Store the CAType
        }
        str = cyg_httpd_form_varable_string(p, "ecs", &len);
        memcpy(conf.location_info.ecs, str, len);
        conf.location_info.ecs[len] = '\0';
        T_I("ecs =%s, %s", conf.location_info.ecs, str);

        //
        // Policies
        //
        for (p_index = LLDPMED_POLICY_MIN ; p_index <= LLDPMED_POLICY_MAX; p_index++ ) {
            // We only need to check that one of the forms in the row exists
            sprintf(form_name, "application_type_%u", p_index); // Set form name
            if (cyg_httpd_form_varable_int(p, form_name, &form_value)) {
                conf.policies_table[p_index].application_type = httpd_form_get_value_int(p, form_name, 1, 8);
                sprintf(form_name, "tag_%u", p_index); // Set form name
                conf.policies_table[p_index].tagged_flag = httpd_form_get_value_int(p, form_name, 0, 1);

                // VLAN ID Shall be 0 when untagged, TIA1057 table 13.
                if (conf.policies_table[p_index].tagged_flag) {
                    sprintf(form_name, "vlan_id_%u", p_index); // Set form name
                    conf.policies_table[p_index].vlan_id = httpd_form_get_value_int(p, form_name, 1, 4095);
                } else {
                    conf.policies_table[p_index].vlan_id = 0;
                }

                sprintf(form_name, "l2_priority_%u", p_index); // Set form name
                conf.policies_table[p_index].l2_priority = httpd_form_get_value_int(p, form_name, LLDPMED_L2_PRIORITY_MIN, LLDPMED_L2_PRIORITY_MAX);
                sprintf(form_name, "dscp_value_%u", p_index); // Set form name
                conf.policies_table[p_index].dscp_value = httpd_form_get_value_int(p, form_name, LLDPMED_DSCP_MIN, LLDPMED_DSCP_MAX);

                //"Delete" the policy if the delete checkbox is checked
                sprintf(form_name, "Delete_%u", p_index); // Set form name
                if (cyg_httpd_form_varable_find(p, form_name) ) {
                    conf.policies_table[p_index].in_use = 0;
                } else {
                    conf.policies_table[p_index].in_use = 1; // Ok - Now this policy is in use.
                }
            }
        }

        //
        // Ports
        //
        // Loop through all front ports
        if (port_iter_init(&pit, NULL, sid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT) == VTSS_OK) {
            while (port_iter_getnext(&pit)) {
                for (p_index = LLDPMED_POLICY_MIN ; p_index <= LLDPMED_POLICY_MAX; p_index++ ) {

                    conf.ports_policies[pit.iport][p_index] = 0;
                    // We only need to check that one of the forms in the row exists
                    sprintf(form_name, "port_policies_%u_%u", pit.iport, p_index); // Set form name

                    if (cyg_httpd_form_varable_find(p, form_name) ) {
                        conf.ports_policies[pit.iport][p_index] = 1;
                    }
                }
            }
        }


        T_I("ecs =%s, %s", conf.location_info.ecs, str);
        // Update the configuration
        (void) lldp_mgmt_set_config(&conf, sid);

stop:
        redirect(p, "/lldp_med_config.htm");

    } else if (web_get_method(p, VTSS_MODULE_ID_LLDP) == CYG_HTTPD_METHOD_GET) {
        //
        // Getting the configuration.
        //
        T_D("Getting the configuration");
        cyg_httpd_start_chunked("html");

        // The "infomation" is split like this:<fast start repeat count>|<Location Data>|<Policies Data>|<Ports Data>|<error message>
        // Fast Start Repeat Count
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u|", conf.medFastStartRepeatCount);
        cyg_httpd_write_chunked(p->outbuffer, ct);


        // Coordinate Location information
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u,%u,%d,%u,%d,%d,%d,%d#",
                      TUDE_MULTIPLIER,
                      conf.location_info.latitude,
                      conf.location_info.latitude_dir,
                      conf.location_info.longitude,
                      conf.location_info.longitude_dir,
                      conf.location_info.altitude,
                      conf.location_info.altitude_type,
                      conf.location_info.datum);

        cyg_httpd_write_chunked(p->outbuffer, ct);


        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s¤",
                      &conf.location_info.ca_country_code[0]);


        cyg_httpd_write_chunked(p->outbuffer, ct);

        int ptr;
        for (ca_index = 0 ; ca_index < LLDPMED_CATYPE_CNT; ca_index ++) {

            ptr = conf.location_info.civic.civic_str_ptr_array[ca_index];

            if (strlen(&conf.location_info.civic.ca_value[ptr])) {
                (void) cgi_escape(&conf.location_info.civic.ca_value[ptr], &tmp_str[0]);
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s¤",
                              &tmp_str[0]);
            } else {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "¤");
            }

            cyg_httpd_write_chunked(p->outbuffer, ct);

            T_R("location_info.civic.ca_value[%d] = %s, strlen(&conf.location_info.civic.ca_value[ptr] = %d",
                ptr,
                &conf.location_info.civic.ca_value[ptr], strlen(&conf.location_info.civic.ca_value[ptr]));

        }

        // ecs
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "#%s",
                      conf.location_info.ecs);

        cyg_httpd_write_chunked(p->outbuffer, ct);

        //
        // Policies
        //
        cyg_httpd_write_chunked("|", 1);

        // The policies is split like this: <policy 1>#<policy 2>#....
        // Each policy is split like this: <policy number>,<Application Type>,<Tag>,<VLAN ID>,<L2 Priority>,<DSCP Value>
        for (p_index = LLDPMED_POLICY_MIN ; p_index <= LLDPMED_POLICY_MAX; p_index++ ) {
            T_D("conf.policies_table[p_index].in_use = %d, p_index = %d", conf.policies_table[p_index].in_use, p_index);
            if (conf.policies_table[p_index].in_use) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u,%u,%u,%u,%u,%u#",
                              p_index,
                              conf.policies_table[p_index].application_type,
                              conf.policies_table[p_index].tagged_flag,
                              conf.policies_table[p_index].vlan_id,
                              conf.policies_table[p_index].l2_priority,
                              conf.policies_table[p_index].dscp_value);
                cyg_httpd_write_chunked(p->outbuffer, ct);
            }
        }

        //
        // Ports
        //
        cyg_httpd_write_chunked("|", 1);

        // The ports is split like this: <list of policies in use>&<port 1>#<port 2>#....
        insert_comma = 0;
        for (p_index = LLDPMED_POLICY_MIN ; p_index <= LLDPMED_POLICY_MAX; p_index++ ) {
            if (conf.policies_table[p_index].in_use) {
                if (insert_comma) {
                    cyg_httpd_write_chunked(",", 1);
                }
                insert_comma = 1;

                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u",
                              p_index);
                cyg_httpd_write_chunked(p->outbuffer, ct);
            }
        }
        cyg_httpd_write_chunked("&", 1);




        // Each port is split like this: <port number>,<Policy#1>,<Policy#2>.....
        if (port_iter_init(&pit, NULL, sid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT) == VTSS_OK) {
            while (port_iter_getnext(&pit)) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u", pit.uport);
                cyg_httpd_write_chunked(p->outbuffer, ct);

                for (p_index = LLDPMED_POLICY_MIN ; p_index <= LLDPMED_POLICY_MAX; p_index++ ) {
                    if (conf.policies_table[p_index].in_use) {
                        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), ",%u",
                                      conf.ports_policies[pit.iport][p_index]);

                        cyg_httpd_write_chunked(p->outbuffer, ct);
                    }
                }


                cyg_httpd_write_chunked("#", 1);
            }
        }

        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "|%s|",
                      err_msg);
        cyg_httpd_write_chunked(p->outbuffer, ct);

        strcpy(err_msg, ""); // Clear error message

        T_R("cyg_httpd_write_chunked->%s", p->outbuffer);

        cyg_httpd_end_chunked();
    }
    return -1; // Do not further search the file system.
}
#endif // VTSS_SW_OPTION_LLDP_MED

//
// LLDP statistics handler
//
static cyg_int32 handler_config_lldp_statistic(CYG_HTTPD_STATE *p)
{
    vtss_isid_t         isid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    int                 ct;
    char                receive_port[255] = "";
    lldp_counters_rec_t stat_cnts[LLDP_PORTS];
    port_iter_t         pit;
    lldp_mib_stats_t    global_cnt;
    time_t              last_change_ago;
    char                last_changed_str[255] = "";

    if (web_get_method(p, VTSS_MODULE_ID_LLDP) == CYG_HTTPD_METHOD_GET) {

        if (var_clear[0]) {     /* Clear? */
            lldp_mgmt_stat_clr(isid);
        }

        cyg_httpd_start_chunked("html");

        (void)lldp_mgmt_stat_get(isid, &stat_cnts[0], &global_cnt, &last_change_ago);
        lldp_mgmt_last_change_ago_to_str(last_change_ago, &last_changed_str[0]);

        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s/%d/%d/%d/%d/|",
                      last_changed_str,
                      global_cnt.table_inserts,
                      global_cnt.table_deletes,
                      global_cnt.table_drops,
                      global_cnt.table_ageouts
                     );
        cyg_httpd_write_chunked(p->outbuffer, ct);

#ifdef VTSS_SW_OPTION_AGGR
        // Get statistic for LAGs
        aggr_mgmt_group_member_t glag_members;
        vtss_glag_no_t glag_no;
        for (glag_no = AGGR_MGMT_GROUP_NO_START; glag_no < AGGR_MGMT_GROUP_NO_END; glag_no++) {
            vtss_port_no_t iport;

            // Get the port members
            (void)aggr_mgmt_members_get(isid, glag_no, &glag_members, FALSE);

            // Determine if there is a port and use the lowest port number for statistic (See packet_api.h as well).
            for (iport = VTSS_PORT_NO_START; iport < VTSS_PORT_NO_END; iport++) {
                int iport_idx;
                iport_idx = iport - VTSS_PORT_NO_START;

                // Make sure that we don't get out of bounds
                if (iport_idx >= LLDP_PORTS) {
                    continue;
                }

                if (glag_members.entry.member[iport]) {
                    if (glag_no >= AGGR_MGMT_GLAG_START) {
                        misc_strncpyz(receive_port, l2port2str(L2GLAG2PORT(glag_no - AGGR_MGMT_GLAG_START + VTSS_GLAG_NO_START)), 255);  /* henrikb - not sure */
                    } else {
                        strcpy(receive_port, l2port2str(L2LLAG2PORT(isid, glag_no - AGGR_MGMT_GROUP_NO_START)));
                    }

                    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s/%d/%d/%d/%d/%d/%d/%d/%d/|",
                                  receive_port,                                                                         /* henrikb - not sure */
                                  stat_cnts[iport_idx].tx_total,
                                  stat_cnts[iport_idx].rx_total,
                                  stat_cnts[iport_idx].rx_error,
                                  stat_cnts[iport_idx].rx_discarded,
                                  stat_cnts[iport_idx].TLVs_discarded,
                                  stat_cnts[iport_idx].TLVs_unrecognized,
                                  stat_cnts[iport_idx].TLVs_org_discarded,
                                  stat_cnts[iport_idx].ageouts);
                    cyg_httpd_write_chunked(p->outbuffer, ct);
                    break;
                }
            }
        }
#endif

        // Loop through all front ports
        if (port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT) == VTSS_OK) {
            while (port_iter_getnext(&pit)) {

                // Check if port is part of a GLAG
                if (lldp_remote_receive_port_to_string(pit.iport, &receive_port[0], isid) == 1) {
                    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/%s/%s///////|",
                                  pit.uport,
                                  "Part of aggr",
                                  receive_port);
                } else {
                    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/%d/%d/%d/%d/%d/%d/%d/%d/|",
                                  pit.uport,
                                  stat_cnts[pit.iport].tx_total,
                                  stat_cnts[pit.iport].rx_total,
                                  stat_cnts[pit.iport].rx_error,
                                  stat_cnts[pit.iport].rx_discarded,
                                  stat_cnts[pit.iport].TLVs_discarded,
                                  stat_cnts[pit.iport].TLVs_unrecognized,
                                  stat_cnts[pit.iport].TLVs_org_discarded,
                                  stat_cnts[pit.iport].ageouts);
                }

                cyg_httpd_write_chunked(p->outbuffer, ct);

            } // End pit.iport for loop

            T_R("cyg_httpd_write_chunked -> %s", p->outbuffer);
        }

        cyg_httpd_end_chunked();
    }
    return -1; // Do not further search the file system.
}



#ifdef VTSS_SW_OPTION_EEE
//
// LLDP EEE neighbors handler
//
static cyg_int32 handler_lldp_eee_neighbors(CYG_HTTPD_STATE *p)
{
    vtss_isid_t         isid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    int                 ct;
    BOOL                no_entries_found = TRUE;
    int                 entry_index;
    lldp_remote_entry_t *entry;
    lldp_remote_entry_t *table = NULL;
    BOOL                insert_seperator = FALSE;
    eee_switch_state_t  eee_state;
    port_iter_t         pit;

    if (web_get_method(p, VTSS_MODULE_ID_LLDP) == CYG_HTTPD_METHOD_GET) {

        table = lldp_mgmt_get_entries(isid); // Get the entries table.
        (void)eee_mgmt_switch_state_get(isid, &eee_state);

        cyg_httpd_start_chunked("html");

        if (port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT) == VTSS_OK) {
            while (port_iter_getnext(&pit)) {

                BOOL entry_found = FALSE;
                entry = table;       // Restart the entries
                for (entry_index = 0 ; entry_index < lldp_remote_get_max_entries(); entry_index++) {
                    T_R_PORT(pit.iport, "in_use =%d, eee_info_vld = %d", entry->in_use, entry->eee.valid);
                    if (entry->in_use == 0 || entry->receive_port != pit.iport || entry->eee.valid == FALSE) {
                        entry++;
                        continue;
                    } else {
                        entry_found = TRUE;
                        break;
                    }
                }

                T_D_PORT(pit.iport, "entry_found = %d", entry_found);

                if (entry_found) {
                    no_entries_found = FALSE;
                    if (insert_seperator) {
                        cyg_httpd_write_chunked("|", 1);
                    }
                    insert_seperator = TRUE;

                    u8 in_sync = (entry->eee.RemTxTwSysEcho == eee_state.port[pit.iport].LocTxSystemValue) && (entry->eee.RemRxTwSysEcho == eee_state.port[pit.iport].LocRxSystemValue);
                    T_D("in_sync = %d", in_sync);
                    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/%d/%d/%d/%d/%d/%d/%d/%d",
                                  pit.uport,
                                  entry->eee.RemTxTwSys,
                                  entry->eee.RemRxTwSys,
                                  entry->eee.RemFbTwSys,
                                  entry->eee.RemTxTwSysEcho,
                                  entry->eee.RemRxTwSysEcho,
                                  in_sync,
                                  eee_state.port[pit.iport].LocResolvedTxSystemValue,
                                  eee_state.port[pit.iport].LocResolvedRxSystemValue);
                    cyg_httpd_write_chunked(p->outbuffer, ct);
                }
            } // End iport for loop
        }

        if (no_entries_found) {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s|",
                          "No EEE info");
            cyg_httpd_write_chunked(p->outbuffer, ct);
        }

        T_R("cyg_httpd_write_chunked->%s", p->outbuffer);

        cyg_httpd_end_chunked();
    }
    return -1; // Do not further search the file system.
}
#endif

#ifdef VTSS_SW_OPTION_POE
static cyg_int32 handler_lldp_poe_neighbors(CYG_HTTPD_STATE *p)
{
    vtss_isid_t           sid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    int                   ct;
    BOOL                  no_entries_found = true;
    int                   entry_index;
    lldp_remote_entry_t   *table = NULL, *entry;
    port_iter_t           pit;

    if (web_get_method(p, VTSS_MODULE_ID_LLDP) == CYG_HTTPD_METHOD_GET) {

        cyg_httpd_start_chunked("html");

        // Get LLDP information
        table = lldp_mgmt_get_entries(sid); // Get the entries table.

        if (port_iter_init(&pit, NULL, sid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT) == VTSS_OK) {
            while (port_iter_getnext(&pit)) {

                entry = table;       // Restart the entries
                for (entry_index = 0 ; entry_index < lldp_remote_get_max_entries(); ++entry_index) {

                    // This do the sorting of the entries ( Sorted by recieved port )
                    if (entry->receive_port != pit.iport || entry->in_use == 0) {
                        entry++; // Get next entry
                        continue;
                    }

                    int power_type     = 0;
                    int power_source   = 0;
                    int power_priority = 0;
                    int power_value    = 0;

                    if (lldp_remote_get_poe_power_info(entry, &power_type, &power_source, &power_priority, &power_value)) {
                        no_entries_found = false; // Signal that at least one entry contained data

                        T_D_PORT(pit.iport, "Entry found");
                        // Push data to web
                        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/%d/%d/%d/%d/|",
                                      pit.uport,
                                      power_type,
                                      power_source,
                                      power_priority,
                                      power_value);

                        cyg_httpd_write_chunked(p->outbuffer, ct);
                    }

                    T_D_PORT(pit.iport, "%d/%d/%d/%d/|",
                             power_type,
                             power_source,
                             power_priority,
                             power_value);



                    entry++;// Get next entry

                } // End iport for loop
            }

            T_R("cyg_httpd_write_chunked -> %s", p->outbuffer);

            if (no_entries_found) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s|", "");
                cyg_httpd_write_chunked(p->outbuffer, ct);
            }
        }
        cyg_httpd_end_chunked();
    }
    return -1; // Do not further search the file system.
}
#endif //// VTSS_SW_OPTION_POE
//
// LLDP config handler
//

cyg_int32 handler_config_lldp_config(CYG_HTTPD_STATE *p)
{
    vtss_isid_t           sid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    int                   ct;
    BOOL                  rx_enable = 0, tx_enable = 0;
    int                   form_value;
    char                  form_name[32];
    static char           err_msg[100]; // Need to be static because it is set when posting data, but used getting data.
    vtss_rc               rc = VTSS_OK;
    port_iter_t           pit;

    if (web_get_method(p, VTSS_MODULE_ID_LLDP) == CYG_HTTPD_METHOD_POST) {

        lldp_struc_0_t conf;

        //
        // Parameters
        //
        lldp_mgmt_get_config(&conf, sid); // Get current configuration

        // Get tx interval from WEB ( Form name = "txInterval" )
        if (cyg_httpd_form_varable_int(p, "txInterval", &form_value)) {
            T_D ("txinterval set to %d via web", form_value);
            conf.msgTxInterval = form_value;
        }

        if (cyg_httpd_form_varable_int(p, "txHold", &form_value)) {
            T_D ("txhold set to %d via web", form_value);
            conf.msgTxHold  = form_value;
        }

        if (cyg_httpd_form_varable_int(p, "txDelay", &form_value)) {
            T_D ("txdelay set to %d via web", form_value);
            conf.txDelay = form_value;
        }

        if (cyg_httpd_form_varable_int(p, "reInitDelay", &form_value)) {
            conf.reInitDelay = form_value;
        }

        // Update the configuration
        (void) lldp_mgmt_set_config(&conf, sid);

        //
        // Admin states
        //
        if ((rc |= port_iter_init(&pit, NULL, sid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT)) == VTSS_OK) {
            while (port_iter_getnext(&pit)) {

                // Note:  htm-checkbox returns 1 if set else 0
                sprintf(form_name, "mode_%u", pit.uport); // Set form name
                if (cyg_httpd_form_varable_int(p, form_name, &form_value)   && (form_value >= 0 && form_value < 4)) {
                    // form_value ok
                } else {
                    T_E("Unknown value for form : %s, value = %d disabling LLDP ", form_name, form_value);
                    form_value = 0;
                }

                T_N("uport = %u, form_value = %d", pit.uport, form_value);
                if (form_value == 0) {
                    T_N("uport = %u disabled", pit.uport);
                    conf.admin_state[pit.iport] = LLDP_DISABLED;
                } else if (form_value == 2) {
                    conf.admin_state[pit.iport] = LLDP_ENABLED_RX_ONLY;
                } else if (form_value == 1) {
                    conf.admin_state[pit.iport] = LLDP_ENABLED_TX_ONLY;
                } else {
                    // form_value is 3
                    conf.admin_state[pit.iport] = LLDP_ENABLED_RX_TX;
                }


#ifdef VTSS_SW_OPTION_CDP
                //
                // CDP AWARE
                //
                sprintf(form_name, "cdp_aware_%u", pit.uport); // Set form name
                if (cyg_httpd_form_varable_find(p, form_name) ) {
                    conf.cdp_aware[pit.iport] = 1;
                } else {
                    conf.cdp_aware[pit.iport] = 0;
                }
#endif

                //
                // Configure  optional TLVs
                //
                // Port description
                sprintf(form_name, "port_descr_ena_%u", pit.uport); // Set to the htm checkbox form name
                T_D("1 pit.uport:%u, pit.iport:%u,  form_name:%s value:%s", pit.uport, pit.iport, form_name, cyg_httpd_form_varable_find(p, form_name));
                if (cyg_httpd_form_varable_find(p, form_name)) {
                    lldp_os_set_optional_tlv(LLDP_TLV_BASIC_MGMT_PORT_DESCR, 1, &conf, pit.iport);
                    T_I("conf.optional_tlv[0]:%d, sid:%d", conf.optional_tlv[0], sid);
                } else {
                    lldp_os_set_optional_tlv(LLDP_TLV_BASIC_MGMT_PORT_DESCR, 0, &conf, pit.iport);
                }

                // sys_name
                sprintf(form_name, "sys_name_ena_%u", pit.uport); // Set to the htm checkbox form name
                if (cyg_httpd_form_varable_find(p, form_name) ) {
                    lldp_os_set_optional_tlv(LLDP_TLV_BASIC_MGMT_SYSTEM_NAME, 1, &conf, pit.iport);
                } else {
                    lldp_os_set_optional_tlv(LLDP_TLV_BASIC_MGMT_SYSTEM_NAME, 0, &conf, pit.iport);
                }

                // sys_descr
                sprintf(form_name, "sys_descr_ena_%u", pit.uport); // Set to the htm checkbox form name
                if (cyg_httpd_form_varable_find(p, form_name) ) {
                    lldp_os_set_optional_tlv(LLDP_TLV_BASIC_MGMT_SYSTEM_DESCR, 1, &conf, pit.iport);
                } else {
                    lldp_os_set_optional_tlv(LLDP_TLV_BASIC_MGMT_SYSTEM_DESCR, 0, &conf, pit.iport);
                }

                // sys_capa
                sprintf(form_name, "sys_capa_ena_%u", pit.uport); // Set to the htm checkbox form name
                if (cyg_httpd_form_varable_find(p, form_name) ) {
                    lldp_os_set_optional_tlv(LLDP_TLV_BASIC_MGMT_SYSTEM_CAPA, 1, &conf, pit.iport);
                } else {
                    lldp_os_set_optional_tlv(LLDP_TLV_BASIC_MGMT_SYSTEM_CAPA, 0, &conf, pit.iport);
                }

                // management addr
                sprintf(form_name, "mgmt_addr_ena_%u", pit.uport); // Set to the htm checkbox form name
                if (cyg_httpd_form_varable_find(p, form_name) ) {
                    lldp_os_set_optional_tlv(LLDP_TLV_BASIC_MGMT_MGMT_ADDR, 1, &conf, pit.iport);
                } else {
                    lldp_os_set_optional_tlv(LLDP_TLV_BASIC_MGMT_MGMT_ADDR, 0, &conf, pit.iport);
                }
                T_I("conf.optional_tlv[0]:%d, sid:%d", conf.optional_tlv[0], sid);
            }

            // Update new configuration
            rc |= lldp_mgmt_set_admin_state(conf.admin_state, sid);



#ifdef VTSS_SW_OPTION_CDP
            rc |= lldp_mgmt_set_cdp_aware(conf.cdp_aware, sid);
#endif
            T_I("conf.optional_tlv[0]:%d, sid:%d", conf.optional_tlv[0], sid);
            rc |= lldp_mgmt_set_optional_tlvs(conf.optional_tlv, sid);
        }

        misc_strncpyz(err_msg, lldp_error_txt(rc), 100); // Set err_msg ( if any )
        redirect(p, "/lldp_config.htm");

    } else if (web_get_method(p, VTSS_MODULE_ID_LLDP) == CYG_HTTPD_METHOD_GET) {
        //
        // Getting the configuration.
        //

        cyg_httpd_start_chunked("html");

        // Apply error message (If any )
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s|", err_msg);
        cyg_httpd_write_chunked(p->outbuffer, ct);
        strcpy(err_msg, ""); // Clear error message


        lldp_struc_0_t conf;
        lldp_mgmt_get_config(&conf, sid) ;


        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/%d/%d/%d|",
                      conf.msgTxInterval,
                      conf.msgTxHold,
                      conf.txDelay,
                      conf.reInitDelay);

        cyg_httpd_write_chunked(p->outbuffer, ct);


        if (port_iter_init(&pit, NULL, sid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT) == VTSS_OK) {
            while (port_iter_getnext(&pit)) {

                if (conf.admin_state[pit.iport] == LLDP_ENABLED_RX_TX ) {
                    tx_enable = 1;
                    rx_enable = 1;
                } else if (conf.admin_state[pit.iport] == LLDP_ENABLED_TX_ONLY ) {
                    tx_enable = 1;
                    rx_enable = 0;
                } else if (conf.admin_state[pit.iport] == LLDP_ENABLED_RX_ONLY ) {
                    tx_enable = 0;
                    rx_enable = 1;
                } else {
                    // LLDP disabled
                    tx_enable = 0;
                    rx_enable = 0;
                }

                //
                // Getting if the optional TLVs are enabled
                //
                BOOL port_descr_ena = 0;
                BOOL sys_name_ena   = 0;
                BOOL sys_descr_ena  = 0;
                BOOL sys_capa_ena   = 0;
                BOOL mgmt_addr_ena  = 0;

                if (lldp_mgmt_get_opt_tlv_enabled(LLDP_TLV_BASIC_MGMT_PORT_DESCR, pit.iport, sid)) {
                    port_descr_ena = 1;
                }

                if (lldp_mgmt_get_opt_tlv_enabled(LLDP_TLV_BASIC_MGMT_SYSTEM_NAME, pit.iport, sid)) {
                    sys_name_ena = 1;
                }

                if (lldp_mgmt_get_opt_tlv_enabled(LLDP_TLV_BASIC_MGMT_SYSTEM_DESCR, pit.iport, sid)) {
                    sys_descr_ena = 1;
                }

                if (lldp_mgmt_get_opt_tlv_enabled(LLDP_TLV_BASIC_MGMT_SYSTEM_CAPA, pit.iport, sid)) {
                    sys_capa_ena = 1;
                }

                if (lldp_mgmt_get_opt_tlv_enabled(LLDP_TLV_BASIC_MGMT_MGMT_ADDR, pit.iport, sid)) {
                    mgmt_addr_ena = 1;
                }

                //
                // Pass configuration to Web
                //
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/%d/%d/%d/%d/%d/%d/%d/%d,",
                              pit.uport,
                              tx_enable,
                              rx_enable,
#ifdef VTSS_SW_OPTION_CDP
                              conf.cdp_aware[pit.iport],
#else
                              0,
#endif
                              port_descr_ena,
                              sys_name_ena,
                              sys_descr_ena,
                              sys_capa_ena,
                              mgmt_addr_ena
                             );

                cyg_httpd_write_chunked(p->outbuffer, ct);
            }
        }

        T_R("cyg_httpd_write_chunked -> %s", p->outbuffer);

        cyg_httpd_end_chunked();
    }
    return -1; // Do not further search the file system.
}

/****************************************************************************/
/*  Module JS lib config routine                                            */
/****************************************************************************/

static size_t lldp_lib_config_js(char **base_ptr, char **cur_ptr, size_t *length)
{
    char buff[LLDP_WEB_BUF_LEN];
    (void) snprintf(buff, LLDP_WEB_BUF_LEN,
                    "var configLLDPRemoteEntriesMax = \"%d\";\n"
                    "var configHasCDP = \"%d\";\n",
                    LLDP_REMOTE_ENTRIES,
#ifdef VTSS_SW_OPTION_CDP
                    1
#else
                    0
#endif
                   );
    return webCommonBufferHandler(base_ptr, cur_ptr, length, buff);
}

/****************************************************************************/
/*  JS lib config table entry                                               */
/****************************************************************************/
web_lib_config_js_tab_entry(lldp_lib_config_js);
/****************************************************************************/
/*  HTTPD Table Entries                                                     */
/****************************************************************************/

CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_lldp_neighbor, "/config/lldp_neighbors", handler_config_lldp_neighbor);
#ifdef VTSS_SW_OPTION_LLDP_MED
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_lldp_neighbor_med, "/stat/lldpmed_neighbors", handler_lldp_neighbor_med);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_lldpmed_config, "/config/lldpmed_config", handler_lldpmed_config);
#endif /* VTSS_SW_OPTION_LLDP_MED */
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_lldp_statistic, "/config/lldp_statistics", handler_config_lldp_statistic);
#ifdef VTSS_SW_OPTION_POE
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_stat_lldp_poe_neighbors, "/stat/lldp_poe_neighbors", handler_lldp_poe_neighbors);
#endif /* VTSS_SW_OPTION_POE */
#ifdef VTSS_SW_OPTION_EEE
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_stat_lldp_eee_neighbors, "/stat/lldp_eee_neighbors", handler_lldp_eee_neighbors);
#endif /* VTSS_SW_OPTION_POE */

CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_lldp_config, "/config/lldp_config", handler_config_lldp_config);

/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
