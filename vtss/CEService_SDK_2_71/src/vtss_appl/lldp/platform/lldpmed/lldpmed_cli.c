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
#include "main.h"
#include "cli.h"
#include "cli_api.h"
#include "lldpmed_cli.h"
#include "vtss_module_id.h"
#include "lldp_remote.h"
#include "lldp.h"
#include "lldpmed_rx.h"
#include "mgmt_api.h"           // For mgmt_txt2ulong

typedef struct {
    ushort lldpmed;
    BOOL latitude;
    BOOL wgs84;
    BOOL nad83_navd88;
    BOOL nad83_mllw;
    BOOL latitude_dir;
    BOOL longitude;
    BOOL longitude_dir;
    BOOL altitude;
    BOOL altitude_type;
    BOOL country;
    BOOL state;
    BOOL county;
    BOOL city;
    BOOL district;
    BOOL block;
    BOOL street;
    BOOL leading_street_direction;
    BOOL trailing_street_suffix;
    BOOL str_suf;
    BOOL house_no;
    BOOL house_no_suffix;
    BOOL landmark;
    BOOL additional_info;
    BOOL name;
    BOOL zip_code;
    BOOL building;
    BOOL apartment;
    BOOL floor;
    BOOL room_number;
    BOOL place_type;
    BOOL postal_com_name;
    BOOL p_o_box;
    BOOL additional_code;
    lldpmed_application_type_t application_type;
    BOOL tagged;

    int               coordinates_type;
    BOOL              policies_list[LLDPMED_POLICIES_CNT];
    u8                l2_prio;
    u8                dscp;

} lldpmed_cli_req_t;

static int cli_lldpmed_policy_list_parse ( char *cmd, char *cmd2, char *stx, char *cmd_org,
                                           cli_req_t *req)
{
    lldp_16_t error = 0; /* As a start there is no error */
    lldp_8_t min = LLDPMED_POLICY_MIN, max = LLDPMED_POLICY_MAX;
    lldp_16_t policy_index;
    lldpmed_cli_req_t           *lldpmed_req = req->module_req;

    req->parm_parsed = 1;
    char *found = cli_parse_find(cmd, stx);

    T_D("ALL %s, %d", found, cli_parse_none(cmd));
    error = (cli_parse_all(cmd) && cli_parm_parse_list(cmd, lldpmed_req->policies_list, min, max, 1));

    if (error && (error = cli_parse_none(cmd)) == 0) {
        for (policy_index = min; policy_index <= max; policy_index++) {
            lldpmed_req->policies_list[policy_index] = 0;
        }
    }
    return (error);
}



static void cli_cmd_lldpmed_conf(cli_req_t *req, BOOL location, BOOL civic, BOOL fast_start_repeat_count,
                                 BOOL ecs, BOOL policy_delete, BOOL policy_add,   BOOL info,  BOOL show_policies,
                                 BOOL port_policy, BOOL datum);

void lldpmed_cli_req_init(void)
{
    /* register the size required for lldpmed req. structure */
    cli_req_size_register(sizeof(lldpmed_cli_req_t));
}

static void cli_cmd_lldpmed_config ( cli_req_t *req )
{
    if (!req->set) {
        cli_header("LLDP-MED Configuration", 1);
    }
    cli_cmd_lldpmed_conf(req, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1);
}


static void cli_cmd_lldpmed_lldpmed_coordinates( cli_req_t *req )
{
    cli_cmd_lldpmed_conf(req, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

static void cli_cmd_lldpmed_lldpmed_datum( cli_req_t *req )
{
    cli_cmd_lldpmed_conf(req, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1);
}

static void cli_cmd_lldpmed_lldpmed_civic( cli_req_t *req )
{
    cli_cmd_lldpmed_conf(req, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0);
}

static void cli_cmd_lldpmed_lldpmed_fast_start_repeat_count ( cli_req_t *req )
{
    cli_cmd_lldpmed_conf(req, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0);
}

static void cli_cmd_lldpmed_lldpmed_ecs( cli_req_t *req )
{
    cli_cmd_lldpmed_conf(req, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0);
}

static void cli_cmd_lldpmed_lldpmed_policy_delete( cli_req_t *req )
{
    cli_cmd_lldpmed_conf(req, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0);
}

static void cli_cmd_lldpmed_lldpmed_policy_add( cli_req_t *req )
{
    cli_cmd_lldpmed_conf(req, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0);
}

static void cli_cmd_lldpmed_lldpmed_info ( cli_req_t *req )
{
    cli_cmd_lldpmed_conf(req, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0);
}

static void cli_cmd_lldpmed_lldpmed_port_policy( cli_req_t *req )
{
    cli_cmd_lldpmed_conf(req, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0);
}




//
// Prints the LLDP-MED policies configuration
//
// In : conf - Pointer to the current configuration
//
//
static void cli_cmd_lldpmed_print_policies(const lldp_struc_0_t *conf)
{

    lldp_bool_t print_header = LLDP_TRUE;
    lldp_16_t policy_index;

    lldp_8_t application_type_str[200];
    // Print all policies that are currently in use.
    for (policy_index = LLDPMED_POLICY_MIN ; policy_index <= LLDPMED_POLICY_MAX; policy_index++) {
        if (conf->policies_table[policy_index].in_use) {

            // Make sure that header is only printed once.
            if (print_header) {
                CPRINTF("\nPolicies defined\n");
                CPRINTF("%-10s %-25s %-8s %-8s %-12s %-8s \n", "Policy Id", "Application Type", "Tag", "Vlan ID", "L2 Priority", "DSCP");
                print_header = LLDP_FALSE;
            }


            lldpmed_appl_type2str(conf->policies_table[policy_index].application_type, &application_type_str[0]);
            CPRINTF("%-10d %-25s %-8s %-8d %-12d %-8d \n",
                    policy_index,
                    application_type_str,
                    conf->policies_table[policy_index].tagged_flag ? "Tagged" : "Untagged",
                    conf->policies_table[policy_index].vlan_id,
                    conf->policies_table[policy_index].l2_priority,
                    conf->policies_table[policy_index].dscp_value );
        }
    }
}




//
// Prints the LLDP-MED Civic Addreess Location configuration
//
// In : conf - Pointer to the current configuration
//
//
static void cli_cmd_lldpmed_print_civic_location(const  lldp_struc_0_t *conf)
{
    lldp_u8_t ca_index;
    lldp_8_t   buf[255];
    lldp_16_t ptr;
    BOOL first = TRUE; // Signaling the first civic printout
    // Print CIVIC location

    CPRINTF("%-25s: ", "Civic Address Location");

    if (strlen(conf->location_info.ca_country_code)) {
        CPRINTF("%-25s - %s  \n", "Country code", &conf->location_info.ca_country_code[0]);
    }

    for (ca_index = 0 ; ca_index < LLDPMED_CATYPE_CNT; ca_index ++) {
        ptr = conf->location_info.civic.civic_str_ptr_array[ca_index];
        if (strlen(&conf->location_info.civic.ca_value[ptr])) {
            lldpmed_catype2str(lldpmed_index2catype(ca_index), &buf[0]);
            if (first) {
                // Don't use -25 because "Civic Address Location" has just been printed.
                CPRINTF("%s  %-25s - %s  \n", "", &buf[0], &conf->location_info.civic.ca_value[ptr]);
                first = FALSE;
            } else {
                CPRINTF("%-25s  %-25s - %s  \n", "", &buf[0], &conf->location_info.civic.ca_value[ptr]);
            }
        }
    }
    CPRINTF("\n");
}




// Prints the LLDP-MED coordinates location configuration
//
// In : conf - Pointer to the current configuration
//
//
static void cli_cmd_lldpmed_print_coordinates(const  lldp_struc_0_t *conf)
{
    lldp_8_t   buf[255];
    lldp_8_t   altitude_type[255];

    CPRINTF("%-25s: ", "Location Coordinates");

    // Convert Latitude configuration to string
    mgmt_long2str_float(buf, abs(conf->location_info.latitude), TUDE_DIGIT);
    if (conf->location_info.latitude_dir == SOUTH) {
        CPRINTF("%-15s - %s %s \n", "Latitude", buf, "South");
    }  else {
        CPRINTF("%-15s - %s %s \n", "Latitude", buf, "North");
    }

    // Convert Longitude configuration to string
    mgmt_long2str_float(buf, conf->location_info.longitude, TUDE_DIGIT);
    if (conf->location_info.longitude_dir == WEST) {
        CPRINTF("%-25s  %-15s - %s %s \n", "", "Longitude", buf, "West");
    }  else {
        CPRINTF("%-25s  %-15s - %s %s \n", "", "Longitude", buf, "East");
    }

    // Convert Altitude configuration to string
    mgmt_long2str_float(buf, conf->location_info.altitude, TUDE_DIGIT);
    lldpmed_at2str(conf->location_info.altitude_type, &altitude_type[0]);
    CPRINTF("%-25s  %-15s - %s%s \n", "", "Altitude", &buf[0], &altitude_type[0]);
}

// Prints the LLDP-MED datum configuration
//
// In : conf - Pointer to the current configuration
//
//
static void cli_cmd_lldpmed_print_datum(const lldp_struc_0_t *conf)
{
    // Convert datum configuration to string
    lldp_8_t   datum_str[25];
    lldpmed_datum2str(conf->location_info.datum, &datum_str[0]);
    CPRINTF("%-15s:  %s\n", "Map datum", &datum_str[0]);
}




static char *lldpmed_list2txt(BOOL *list, int min, int max, char *buf)
{
    int  i, first = 1, count = 0;
    BOOL member;
    char *p;

    p = buf;
    *p = '\0';
    for (i = min; i <= max; i++) {
        member = list[i];
        if ((member && (count == 0 || i == max)) || (!member && count > 1)) {
            p += sprintf(p, "%s%d",
                         first ? "" : count > (member ? 1 : 2) ? "-" : ",",
                         member ? (i) : i - 1);
            first = 0;
        }
        if (member) {
            count++;
        } else {
            count = 0;
        }
    }
    return buf;
}


//
// Prints the LLDP-MED policies defined for each port
//
// In : conf - Pointer to the current configuration
//      req - Contains information about which information the user is requesting
//
//
static void cli_cmd_lldpmed_print_port_policies(cli_req_t *req)
{
    lldp_8_t       buf[64];
    lldp_u16_t     p_index;
    lldp_bool_t    policies_list[LLDPMED_POLICIES_CNT];
    vtss_usid_t    usid;
    vtss_isid_t    isid;
    port_iter_t    pit;

    for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
        isid = req->stack.isid[usid];
        if (isid == VTSS_ISID_END) {
            continue;
        }

        lldp_struc_0_t conf;
        lldp_mgmt_get_config(&conf, isid);

        cli_cmd_usid_print(usid, req, 1);

        CPRINTF("\n%-10s %s  \n", "Port", "Policies");
        // Loop through all front ports
        if (port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT) == VTSS_OK) {
            while (port_iter_getnext(&pit)) {
                if (req->uport_list[pit.uport] == 0) {
                    continue;
                }

                for (p_index = LLDPMED_POLICY_MIN ; p_index <= LLDPMED_POLICY_MAX; p_index++ ) {
                    policies_list[p_index] = conf.ports_policies[pit.iport][p_index] && conf.policies_table[p_index].in_use;
                }

                (void) lldpmed_list2txt(&policies_list[0], LLDPMED_POLICY_MIN, LLDPMED_POLICY_MAX, buf);
                if (strlen(buf) == 0) {
                    CPRINTF("%-10u %s\n", pit.uport, "none" );
                } else {
                    CPRINTF("%-10u %s\n", pit.uport, buf );
                }
            }
        }
    }
}



void cli_cmd_lldpmed_print_inventory(lldp_remote_entry_t *entry)
{


    lldp_8_t inventory_str[MAX_LLDPMED_INVENTORY_LENGTH];


    if (entry->lldpmed_hw_rev_length            > 0 ||
        entry->lldpmed_firm_rev_length          > 0 ||
        entry->lldpmed_sw_rev_length            > 0 ||
        entry->lldpmed_serial_no_length         > 0 ||
        entry->lldpmed_manufacturer_name_length > 0 ||
        entry->lldpmed_model_name_length        > 0 ||
        entry->lldpmed_assert_id_length         > 0) {
        CPRINTF("\nInventory \n");

        strncpy(inventory_str, entry->lldpmed_hw_rev, entry->lldpmed_hw_rev_length);
        T_DG(TRACE_GRP_RX, "entry->lldpmed_hw_rev_length = %d", entry->lldpmed_hw_rev_length);
        inventory_str[entry->lldpmed_hw_rev_length] = '\0';// Clear the string since data from the LLDP entry does contain NULL pointer.
        CPRINTF("%-20s: %s \n", "Hardware Revision", inventory_str);


        strncpy(inventory_str, entry->lldpmed_firm_rev, entry->lldpmed_firm_rev_length);
        T_DG(TRACE_GRP_RX, "entry->lldpmed_hw_rev_length = %d", entry->lldpmed_firm_rev_length);
        inventory_str[entry->lldpmed_firm_rev_length] = '\0'; // Add NULL pointer since data from the LLDP entry does contain NULL pointer.
        CPRINTF("%-20s: %s \n", "Firmware Revision", inventory_str);

        strncpy(inventory_str, entry->lldpmed_sw_rev, entry->lldpmed_sw_rev_length);
        inventory_str[entry->lldpmed_sw_rev_length] = '\0'; // Add NULL pointer since data from the LLDP entry does contain NULL pointer.
        CPRINTF("%-20s: %s \n", "Software Revision", inventory_str);

        strncpy(inventory_str, entry->lldpmed_serial_no, entry->lldpmed_serial_no_length);
        inventory_str[entry->lldpmed_serial_no_length] = '\0'; // Add NULL pointer since data from the LLDP entry does contain NULL pointer.
        CPRINTF("%-20s: %s \n", "Serial Number", inventory_str);

        strncpy(inventory_str, entry->lldpmed_manufacturer_name, entry->lldpmed_manufacturer_name_length);
        inventory_str[entry->lldpmed_manufacturer_name_length] = '\0'; // Add NULL pointer since data from the LLDP entry does contain NULL pointer.
        CPRINTF("%-20s: %s \n", "Manufacturer Name", inventory_str);

        strncpy(inventory_str, entry->lldpmed_model_name, entry->lldpmed_model_name_length);
        inventory_str[entry->lldpmed_model_name_length] = '\0'; // Add NULL pointer since data from the LLDP entry does contain NULL pointer.
        CPRINTF("%-20s: %s \n", "Model Name", inventory_str);

        strncpy(inventory_str, entry->lldpmed_assert_id, entry->lldpmed_assert_id_length);
        inventory_str[entry->lldpmed_assert_id_length] = '\0'; // Add NULL pointer since data from the LLDP entry does contain NULL pointer.
        CPRINTF("%-20s: %s \n", "Assert ID", inventory_str);
    }
}


//
// Prints the LLDP-MED neighbor infomation in an entry.
//
// In : entry - Pointer to the entry that contains the information that shall be outputed
//      isid  - Internal switch id.
//
//
static void cli_cmd_lldpmed_print_info(lldp_remote_entry_t *entry, vtss_isid_t isid)
{

    lldp_8_t   buf[1000];
    lldp_u8_t  p_index;

    T_DG(TRACE_GRP_RX, "size = %zu, lldpmed_info_vld = %d", sizeof(buf), entry->lldpmed_info_vld);

    // This function takes an iport and prints a uport.
    if (lldp_remote_receive_port_to_string(entry->receive_port, buf, isid)) {
        T_DG_PORT(TRACE_GRP_CLI, entry->receive_port, "Part of LAG");
    }

    if (entry->lldpmed_info_vld) {
        CPRINTF("%-20s: %s\n", "Local port", buf);


        // Device type / capabilities
        if (entry->lldpmed_capabilities_vld) {
            lldpmed_device_type2str(entry, buf);
            CPRINTF("%-20s: %s \n", "Device Type", buf);

            lldpmed_capabilities2str(entry, buf);
            CPRINTF("%-20s: %s \n", "Capabilities", buf);
        }


        for (p_index = 0; p_index < MAX_LLDPMED_POLICY_APPLICATIONS_CNT; p_index ++) {
            // make sure that policy exist.
            T_NG(TRACE_GRP_CLI, "Policy valid = %d, p_index = %d", entry->lldpmed_policy_vld[p_index], p_index);
            if (entry->lldpmed_policy_vld[p_index] == LLDP_FALSE) {
                T_NG(TRACE_GRP_CLI, "Continue");
                continue;
            }

            // Policies
            lldpmed_policy_appl_type2str(entry->lldpmed_policy[p_index], buf);
            CPRINTF("\n%-20s: %s \n", "Application Type", buf);

            lldpmed_policy_flag_type2str(entry->lldpmed_policy[p_index], buf);
            CPRINTF("%-20s: %s \n", "Policy", buf);

            lldpmed_policy_tag2str(entry->lldpmed_policy[p_index], buf);
            CPRINTF("%-20s: %s \n", "Tag", buf);

            lldpmed_policy_vlan_id2str(entry->lldpmed_policy[p_index], buf);
            CPRINTF("%-20s: %s \n", "VLAN ID", buf);

            lldpmed_policy_prio2str(entry->lldpmed_policy[p_index], buf);
            CPRINTF("%-20s: %s \n", "Priority", buf);

            lldpmed_policy_dscp2str(entry->lldpmed_policy[p_index], buf);
            CPRINTF("%-20s: %s \n", "DSCP", buf);
        }


        for (p_index = 0; p_index < MAX_LLDPMED_LOCATION_CNT; p_index ++) {
            if (entry->lldpmed_location_vld[p_index] == 1) {
                lldpmed_location2str(entry, buf, p_index);
                CPRINTF("%-20s: %s \n", "Location", buf);
            }
        }


        cli_cmd_lldpmed_print_inventory(entry);

        CPRINTF("\n");
    }
}

//
// Loops through all entries and prints entries containing neighbor information
//
// In : req - Contains information about which information the user is requesting
//
//
static void cli_cmd_lldpmed_print_entries(cli_req_t *req)
{
    lldp_u8_t count = lldp_remote_get_max_entries();
    BOOL lldpmed_no_entry_found = LLDP_TRUE;
    lldp_remote_entry_t *table = NULL, *entry = NULL;
    lldp_u8_t i;
    vtss_usid_t         usid;
    vtss_isid_t         isid;
    port_iter_t         pit;

    for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
        if ((isid = req->stack.isid[usid]) == VTSS_ISID_END) {
            continue;
        }

        cli_cmd_usid_print(usid, req, 1);

        T_DG(TRACE_GRP_CLI, "Got a correct isid : %d", isid);
        lldp_mgmt_get_lock();
        table = lldp_mgmt_get_entries(isid);


        // Loop through all front ports
        if (port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT) == VTSS_OK) {
            while (port_iter_getnext(&pit)) {
                if (req->uport_list[pit.uport] == 0) {
                    continue;
                }

                for (i = 0, entry = table; i < count; i++, entry++) {
                    if (entry->in_use == FALSE || !entry->lldpmed_info_vld || (entry->receive_port != pit.iport) ) {
                        continue;
                    }

                    lldpmed_no_entry_found = FALSE;
                    cli_cmd_lldpmed_print_info(entry, isid);
                }

            }
        } /* Port loop */

        lldp_mgmt_get_unlock();

        if (lldpmed_no_entry_found) {
            CPRINTF("No LLDP-MED entries found\n");
        }
        lldpmed_no_entry_found = LLDP_TRUE; // Prepare for next switch
    } /* USID loop */


}


static void cli_cmd_lldpmed_conf(cli_req_t *req, BOOL coordinates, BOOL civic, BOOL fast_start_repeat_count,
                                 BOOL ecs, BOOL policy_delete, BOOL policy_add,   BOOL info,  BOOL show_policies,
                                 BOOL port_policy, BOOL datum)
{
    BOOL                update_conf = TRUE;
    vtss_usid_t         usid;
    vtss_isid_t         isid;
    port_iter_t         pit;
    lldp_struc_0_t      conf;
    lldp_common_conf_t  lldp_common_conf;
    lldpmed_cli_req_t           *lldpmed_req = req->module_req;

    T_IG(TRACE_GRP_CLI, "Enter cli_cmd_lldpmed_conf");
    if (cli_cmd_switch_none(req)) {
        return;
    }

    lldp_16_t policy_index;


    if (req->set) {

        lldp_mgmt_get_common_config(&lldp_common_conf);

        // Delete the policies selected by the policy_list
        if (policy_delete) {
            for (policy_index = LLDPMED_POLICY_MIN ; policy_index <= LLDPMED_POLICY_MAX; policy_index++) {
                if (lldpmed_req->policies_list[policy_index]) {
                    lldp_common_conf.policies_table[policy_index].in_use = FALSE;
                }
            }
        }


        // Adding new policies
        if (policy_add) {
            BOOL policy_added = false;

            // Loop through all poicies to find a free policy entry to add the new policy.
            for (policy_index = LLDPMED_POLICY_MIN ; policy_index <= LLDPMED_POLICY_MAX; policy_index++) {
                T_IG(TRACE_GRP_CLI, "lldp_common_conf.policies_table[%d].in_use:%d", policy_index, lldp_common_conf.policies_table[policy_index].in_use);
                if (lldp_common_conf.policies_table[policy_index].in_use == FALSE) {
                    policy_added = LLDP_TRUE;
                    lldp_common_conf.policies_table[policy_index].in_use = TRUE;
                    lldp_common_conf.policies_table[policy_index].application_type = lldpmed_req->application_type;
                    lldp_common_conf.policies_table[policy_index].tagged_flag = lldpmed_req->tagged;

                    if (lldpmed_req->tagged) {
                        if (req->vid == 0) {
                            CPRINTF("VLAN id can't be 0 when tagged.\n");
                            update_conf = FALSE;
                        } else {
                            lldp_common_conf.policies_table[policy_index].vlan_id = req->vid;
                        }
                    } else {
                        if (req->vid != 0) {
                            CPRINTF("VLAN id MUST be 0 when untagged.\n");
                            update_conf = FALSE;
                        }
                    }

                    lldp_common_conf.policies_table[policy_index].l2_priority = lldpmed_req->l2_prio;
                    lldp_common_conf.policies_table[policy_index].dscp_value = lldpmed_req->dscp;
                    break;
                }
            }

            if (!policy_added) {
                CPRINTF("Maximum number of policies supported (%d) reached. New policy was not added\n", LLDPMED_POLICIES_CNT);
            } else {
                if (update_conf) {
                    CPRINTF("New policy added with policy id: %d \n", policy_index);
                }
            }
        }

        if (update_conf) {
            // Set the new configuration
            vtss_rc rc;
            if ((rc = lldp_mgmt_set_common_config(&lldp_common_conf)) != VTSS_OK) {
                CPRINTF("%s\n", error_txt(rc));
                return;
            }
        }


        T_IG(TRACE_GRP_CLI, "Setting ecs:%d, show_policies:%d, policy_add:%d", ecs, show_policies, policy_add);
        for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
            if ((isid = req->stack.isid[usid]) == VTSS_ISID_END) {
                continue;
            }

            lldp_mgmt_get_config(&conf, isid);

            // Fast count
            if (fast_start_repeat_count) {
                conf.medFastStartRepeatCount = req->value;
            }



            // Location information
            if (coordinates) {
                if (lldpmed_req->latitude) {
                    conf.location_info.latitude_dir = (lldpmed_latitude_dir_t) lldpmed_req->coordinates_type;
                    conf.location_info.latitude = req->signed_value;
                }

                if (lldpmed_req->longitude) {
                    conf.location_info.longitude_dir = (lldpmed_longitude_dir_t) lldpmed_req->coordinates_type;
                    conf.location_info.longitude = req->signed_value;
                }

                if (lldpmed_req->altitude) {
                    conf.location_info.altitude_type = (lldpmed_at_type_t) lldpmed_req->coordinates_type;
                    conf.location_info.altitude = req->signed_value;
                }
            }

            if (datum) {
                if (lldpmed_req->nad83_mllw) {
                    conf.location_info.datum = NAD83_MLLW;
                }
                if (lldpmed_req->wgs84) {
                    conf.location_info.datum = WGS84;
                }

                if (lldpmed_req->nad83_navd88) {
                    conf.location_info.datum = NAD83_NAVD88;
                }
            }

            if (ecs) {
                strcpy(conf.location_info.ecs, req->parm);
            }

            if (civic) {
                if (lldpmed_req->country) {
                    misc_strncpyz(conf.location_info.ca_country_code, req->parm, CA_COUNTRY_CODE_LEN);
                }

                if (lldpmed_req->state) {
                    lldpmed_update_civic_info(&conf.location_info.civic, LLDPMED_CATYPE_A1, req->parm);
                }

                if (lldpmed_req->county) {
                    lldpmed_update_civic_info(&conf.location_info.civic, LLDPMED_CATYPE_A2, req->parm);
                }

                if (lldpmed_req->city) {
                    lldpmed_update_civic_info(&conf.location_info.civic, LLDPMED_CATYPE_A3, req->parm);
                }

                if (lldpmed_req->district) {
                    lldpmed_update_civic_info(&conf.location_info.civic, LLDPMED_CATYPE_A4, req->parm);
                }

                if (lldpmed_req->block) {
                    lldpmed_update_civic_info(&conf.location_info.civic, LLDPMED_CATYPE_A5, req->parm);
                }

                if (lldpmed_req->street) {
                    lldpmed_update_civic_info(&conf.location_info.civic, LLDPMED_CATYPE_A6, req->parm);
                }

                if (lldpmed_req->leading_street_direction) {
                    lldpmed_update_civic_info(&conf.location_info.civic, LLDPMED_CATYPE_PRD, req->parm);
                }

                if (lldpmed_req->trailing_street_suffix) {
                    lldpmed_update_civic_info(&conf.location_info.civic, LLDPMED_CATYPE_POD, req->parm);
                }

                if (lldpmed_req->str_suf) {
                    lldpmed_update_civic_info(&conf.location_info.civic, LLDPMED_CATYPE_STS, req->parm);
                }

                if (lldpmed_req->house_no_suffix) {
                    lldpmed_update_civic_info(&conf.location_info.civic, LLDPMED_CATYPE_HNS, req->parm);
                }

                if (lldpmed_req->house_no) {
                    lldpmed_update_civic_info(&conf.location_info.civic, LLDPMED_CATYPE_HNO, req->parm);
                }

                if (lldpmed_req->landmark) {
                    lldpmed_update_civic_info(&conf.location_info.civic, LLDPMED_CATYPE_LMK, req->parm);
                }

                if (lldpmed_req->additional_info) {
                    lldpmed_update_civic_info(&conf.location_info.civic, LLDPMED_CATYPE_LOC, req->parm);
                }

                if (lldpmed_req->name) {
                    lldpmed_update_civic_info(&conf.location_info.civic, LLDPMED_CATYPE_NAM, req->parm);
                }

                if (lldpmed_req->zip_code) {
                    lldpmed_update_civic_info(&conf.location_info.civic, LLDPMED_CATYPE_ZIP, req->parm);
                }

                if (lldpmed_req->building) {
                    lldpmed_update_civic_info(&conf.location_info.civic, LLDPMED_CATYPE_BUILD, req->parm);
                }

                if (lldpmed_req->apartment) {
                    lldpmed_update_civic_info(&conf.location_info.civic, LLDPMED_CATYPE_UNIT, req->parm);
                }

                if (lldpmed_req->floor) {
                    lldpmed_update_civic_info(&conf.location_info.civic, LLDPMED_CATYPE_FLR, req->parm);
                }

                if (lldpmed_req->room_number) {
                    lldpmed_update_civic_info(&conf.location_info.civic, LLDPMED_CATYPE_ROOM, req->parm);
                }

                if (lldpmed_req->place_type) {
                    lldpmed_update_civic_info(&conf.location_info.civic, LLDPMED_CATYPE_PLACE, req->parm);
                }

                if (lldpmed_req->postal_com_name) {
                    lldpmed_update_civic_info(&conf.location_info.civic, LLDPMED_CATYPE_PCN, req->parm);
                }

                if (lldpmed_req->p_o_box) {
                    lldpmed_update_civic_info(&conf.location_info.civic, LLDPMED_CATYPE_POBOX, req->parm);
                }

                if (lldpmed_req->additional_code) {
                    lldpmed_update_civic_info(&conf.location_info.civic, LLDPMED_CATYPE_ADD_CODE, req->parm);
                }

            }


            if (port_policy) {
                // Loop through all front ports
                if (port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT) == VTSS_OK) {
                    while (port_iter_getnext(&pit)) {
                        if (req->uport_list[pit.uport] == 0) {
                            continue;
                        }


                        lldp_16_t p_index;
                        for (p_index = LLDPMED_POLICY_MIN ; p_index <= LLDPMED_POLICY_MAX; p_index++ ) {
                            T_IG(TRACE_GRP_CLI, "lldpmed_req->policies_list[%d]:%d, in_use:%d", p_index, lldpmed_req->policies_list[p_index], conf.policies_table[p_index].in_use);
                            if ((conf.policies_table[p_index].in_use == FALSE) && lldpmed_req->policies_list[p_index]) {
                                CPRINTF("Ignoring policy %d for port %u because no such policy is defined \n", p_index, pit.uport);
                            } else {
                                conf.ports_policies[pit.iport][p_index] = lldpmed_req->policies_list[p_index];
                            }
                        }
                    }
                }
            }

            vtss_rc rc;
            if ((rc = lldp_mgmt_set_config(&conf, isid)) != VTSS_OK) {
                CPRINTF("%s\n", error_txt(rc));
            }
        }
    } else {
        T_IG(TRACE_GRP_CLI, "Getting ecs:%d, show_policies:%d", ecs, show_policies);
        lldp_mgmt_get_config(&conf, VTSS_ISID_LOCAL);
        // Show current configuration
        if (fast_start_repeat_count) {
            CPRINTF("%-25s: %d \n", "Fast Start Repeat Count", conf.medFastStartRepeatCount);
        }

        if (coordinates) {
            cli_cmd_lldpmed_print_coordinates(&conf);
        }

        if (datum) {
            cli_cmd_lldpmed_print_datum(&conf);
        }

        if (civic) {
            cli_cmd_lldpmed_print_civic_location(&conf);
        }

        if (ecs) {
            // Print ECS location info
            if (strlen(conf.location_info.ecs)) {
                CPRINTF("%-25s: %s  \n", "Emergency Call Service", &conf.location_info.ecs[0]);
            } else {
                CPRINTF("%s\n", "No Emergency Call Service defined");
            }
        }

        if (show_policies) {
            cli_cmd_lldpmed_print_policies(&conf);
        }

        if (info) {
            cli_cmd_lldpmed_print_entries(req);
        }

        if (port_policy) {
            cli_cmd_lldpmed_print_port_policies(req);
        }
    }
}

#ifdef VTSS_SW_OPTION_LLDP_MED_DEBUG
static void lldpmed_cli_cmd_debug_lldpmed( cli_req_t *req )
{
    port_iter_t         pit;
    if (req->set) {
        // Loop through all front ports
        if (port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT) == VTSS_OK) {
            while (port_iter_getnext(&pit)) {
                if (req->uport_list[pit.uport] == 0) {
                    continue;
                }
                lldp_mgmt_set_med_transmit_var(pit.iport, req->enable);
            }
        }
    } else {
        CPRINTF("%-5s %s \n", "Port", "medTansmitEnable");

        if (port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT) == VTSS_OK) {
            while (port_iter_getnext(&pit)) {
                if (req->uport_list[pit.uport] == 0) {
                    continue;
                }
                CPRINTF("%-5u %s \n", pit.uport, cli_bool_txt(lldp_mgmt_get_med_transmit_var(pit.iport, req->enable)));
            }
        }
    }
}
#endif

/****************************************************************************/
/*  Parameter functions                                                     */
/****************************************************************************/


static int32_t cli_lldpmed_fast_start_repeat_count_parse (char *cmd, char *cmd2, char *stx, char *cmd_org,
                                                          cli_req_t *req)
{
    ulong error = 0;
    ulong value;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &value, 1, 10);  // Limits define in TIA1057, MIB lldpXMedFastStartRepeatCount object.
    req->value = value;
    return (error);
}



static int32_t cli_lldpmed_vlan_id_parse (char *cmd, char *cmd2, char *stx, char *cmd_org,
                                          cli_req_t *req)
{
    ulong error = 0;
    ulong value;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &value, 0, LLDPMED_VID_MAX); // Table 13, TIA1057, 0 is allowed when untagged. Checked else where
    req->vid = value;
    return (error);
}


static int32_t cli_lldpmed_l2_priority_parse (char *cmd, char *cmd2, char *stx, char *cmd_org,
                                              cli_req_t *req)
{
    ulong error = 0;
    ulong value;
    lldpmed_cli_req_t           *lldpmed_req = req->module_req;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &value, LLDPMED_L2_PRIORITY_MIN, LLDPMED_L2_PRIORITY_MAX);
    lldpmed_req->l2_prio = value;
    return (error);
}

static int32_t cli_lldpmed_dscp_parse (char *cmd, char *cmd2, char *stx, char *cmd_org,
                                       cli_req_t *req)
{
    ulong error = 0;
    ulong value;
    lldpmed_cli_req_t           *lldpmed_req = req->module_req;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &value, LLDPMED_DSCP_MIN, LLDPMED_DSCP_MAX);
    lldpmed_req->dscp = value;
    return (error);
}


static BOOL cli_lldpmed_keyword_cmp(char *found, char *keyword)
{
    int result = strncmp(found, keyword, strlen(keyword));
    return result;
}
static int32_t cli_lldpmed_parse_keyword(char *cmd, char *cmd2, char *stx, char *cmd_org,
                                         cli_req_t *req)
{
    lldpmed_cli_req_t *lldpmed_req = NULL;

    char *found = cli_parse_find(cmd, stx);

    T_DG(TRACE_GRP_CLI, "found:%s, cmd:%s, stx:%s", found, cmd, stx);
    req->parm_parsed = 1;

    lldpmed_req = req->module_req;

    if (found != NULL) {
        if (!cli_lldpmed_keyword_cmp(found, "country"))         {
            lldpmed_req->country = 1;
        } else if (!cli_lldpmed_keyword_cmp(found, "state"))           {
            lldpmed_req->state = 1;
        } else if (!cli_lldpmed_keyword_cmp(found, "county"))          {
            lldpmed_req->county = 1;
        } else if (!cli_lldpmed_keyword_cmp(found, "district"))        {
            lldpmed_req->district = 1;
        } else if (!cli_lldpmed_keyword_cmp(found, "city"))            {
            lldpmed_req->city = 1;
        } else if (!cli_lldpmed_keyword_cmp(found, "block"))           {
            lldpmed_req->block = 1;
        } else if (!cli_lldpmed_keyword_cmp(found, "street"))          {
            lldpmed_req->street = 1;
        } else if (!cli_lldpmed_keyword_cmp(found, "leading_street_direction")) {
            lldpmed_req->leading_street_direction = 1;
        } else if (!cli_lldpmed_keyword_cmp(found, "trailing_street_suffix")) {
            lldpmed_req->trailing_street_suffix = 1;
        } else if (!cli_lldpmed_keyword_cmp(found, "str_suf"))         {
            lldpmed_req->str_suf = 1;
        } else if (!cli_lldpmed_keyword_cmp(found, "house_no_suffix")) {
            lldpmed_req->house_no_suffix = 1;
        } else if (!cli_lldpmed_keyword_cmp(found, "house_no"))        {
            lldpmed_req->house_no = 1;
        } else if (!cli_lldpmed_keyword_cmp(found, "landmark"))        {
            lldpmed_req->landmark = 1;
        } else if (!cli_lldpmed_keyword_cmp(found, "additional_info")) {
            lldpmed_req->additional_info = 1;
        } else if (!cli_lldpmed_keyword_cmp(found, "name"))            {
            lldpmed_req->name = 1;
        } else if (!cli_lldpmed_keyword_cmp(found, "zip_code"))        {
            lldpmed_req->zip_code = 1;
        } else if (!cli_lldpmed_keyword_cmp(found, "building"))        {
            lldpmed_req->building = 1;
        } else if (!cli_lldpmed_keyword_cmp(found, "apartment"))       {
            lldpmed_req->apartment = 1;
        } else if (!cli_lldpmed_keyword_cmp(found, "floor"))           {
            lldpmed_req->floor = 1;
        } else if (!cli_lldpmed_keyword_cmp(found, "room_number"))     {
            lldpmed_req->room_number = 1;
        } else if (!cli_lldpmed_keyword_cmp(found, "place_type"))      {
            lldpmed_req->place_type = 1;
        } else if (!cli_lldpmed_keyword_cmp(found, "postal_com_name")) {
            lldpmed_req->postal_com_name = 1;
        } else if (!cli_lldpmed_keyword_cmp(found, "p_o_box"))         {
            lldpmed_req->p_o_box = 1;
        } else if (!cli_lldpmed_keyword_cmp(found, "additional_code")) {
            lldpmed_req->additional_code = 1;
        } else if (!cli_lldpmed_keyword_cmp(found, "tagged"))          {
            lldpmed_req->tagged = 1;
        } else if (!cli_lldpmed_keyword_cmp(found, "untagged"))        {
            lldpmed_req->tagged = 0;
        }
    }
    T_DG(TRACE_GRP_CLI, "Failed = %d", (found == NULL ? 1 : 0));
    return (found == NULL ? 1 : 0);
}

static int32_t cli_lldpmed_appl_type_keyword(char *cmd, char *cmd2, char *stx, char *cmd_org,
                                             cli_req_t *req)
{
    lldpmed_cli_req_t *lldpmed_req = NULL;

    int32_t        error = 0;
    error = cli_parse_text(cmd_org, req->parm, 50);

    if (error != 0 || strlen(req->parm) > 50) {
        return VTSS_INVALID_PARAMETER;
    }

    req->parm_parsed = 1;
    lldpmed_req = req->module_req;

    // Conversion to application type as integer, See Table 12, TIA1057
    if (!cli_lldpmed_keyword_cmp("voice", req->parm)) {
        lldpmed_req->application_type = VOICE;
    } else if (!cli_lldpmed_keyword_cmp("voice_signaling", req->parm)) {
        lldpmed_req->application_type = VOICE_SIGNALING;
    } else if (!cli_lldpmed_keyword_cmp("guest_voice_signaling", req->parm)) {
        lldpmed_req->application_type = GUEST_VOICE_SIGNALING;
    } else if (!cli_lldpmed_keyword_cmp("guest_voice", req->parm))     {
        lldpmed_req->application_type = GUEST_VOICE;
    } else if (!cli_lldpmed_keyword_cmp("softphone_voice", req->parm)) {
        lldpmed_req->application_type = SOFTPHONE_VOICE;
    } else if (!cli_lldpmed_keyword_cmp("video_conferencing", req->parm)) {
        lldpmed_req->application_type = VIDEO_CONFERENCING;
    } else if (!cli_lldpmed_keyword_cmp("streaming_video", req->parm)) {
        lldpmed_req->application_type = STREAMING_VIDEO;
    } else if (!cli_lldpmed_keyword_cmp("video_signaling", req->parm)) {
        lldpmed_req->application_type = VIDEO_SIGNALING;
    } else {
        return VTSS_INVALID_PARAMETER;
    }

    return 0;
}

static int32_t cli_lldpmed_tude_type_keyword(char *cmd, char *cmd2, char *stx, char *cmd_org,
                                             cli_req_t *req)
{
    lldpmed_cli_req_t *lldpmed_req = NULL;

    int32_t        error = 0;
    error = cli_parse_text(cmd_org, req->parm, 50);

    if (error != 0 || strlen(req->parm) > 50) {
        return VTSS_INVALID_PARAMETER;
    }

    req->parm_parsed = 1;
    lldpmed_req = req->module_req;

    // Conversion to application type as integer, See Table 12, TIA1057
    if  (!cli_lldpmed_keyword_cmp("latitude", req->parm))               {
        lldpmed_req->latitude = 1;
    } else if (!cli_lldpmed_keyword_cmp("longitude", req->parm))       {
        lldpmed_req->longitude = 1;
    } else if (!cli_lldpmed_keyword_cmp("wgs84", req->parm))           {
        lldpmed_req->wgs84 = 1;
    } else if (!cli_lldpmed_keyword_cmp("nad83_navd88", req->parm))    {
        lldpmed_req->nad83_navd88 = 1;
    } else if (!cli_lldpmed_keyword_cmp("nad83_mllw", req->parm))      {
        lldpmed_req->nad83_mllw = 1;
    } else if (!cli_lldpmed_keyword_cmp("altitude", req->parm))        {
        lldpmed_req->altitude = 1;
    } else if (!cli_lldpmed_keyword_cmp("altitude_type", req->parm))   {
        lldpmed_req->altitude_type = 1;

    } else {
        return VTSS_INVALID_PARAMETER;
    }

    return 0;
}

static int32_t cli_lldpmed_coordinates_value_parse(char *cmd, char *cmd2, char *stx,
                                                   char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;
    req->parm_parsed = 1;

    long value = 0;

    lldpmed_cli_req_t *lldpmed_req = NULL;
    lldpmed_req = req->module_req;

    // Parse tude value, and check limits
    if (lldpmed_req->latitude) {
        error = mgmt_str_float2long(cmd_org, &value, LLDPMED_LATITUDE_VALUE_MIN, LLDPMED_LATITUDE_VALUE_MAX, TUDE_DIGIT);
    } else if (lldpmed_req->longitude) {
        error = mgmt_str_float2long(cmd_org, &value, LLDPMED_LONGITUDE_VALUE_MIN, LLDPMED_LONGITUDE_VALUE_MAX, TUDE_DIGIT);
    } else {
        error = mgmt_str_float2long(cmd_org, &value, LLDPMED_ALTITUDE_VALUE_MIN, LLDPMED_ALTITUDE_VALUE_MAX, TUDE_DIGIT);
    }


    req->signed_value = value;
    return error;
}

static int32_t cli_lldpmed_coordinates_type_parse(char *cmd, char *cmd2, char *stx,
                                                  char *cmd_org, cli_req_t *req)
{
    int32_t parse_failure = 1;
    lldpmed_cli_req_t *lldpmed_req = NULL;

    int32_t        error = 0;
    error = cli_parse_text(cmd_org, req->parm, 50);

    if (error != 0 || strlen(req->parm) > 50) {
        return VTSS_INVALID_PARAMETER;
    }

    req->parm_parsed = 1;
    lldpmed_req = req->module_req;

    if (!cli_lldpmed_keyword_cmp("north", req->parm) && lldpmed_req->latitude) {
        lldpmed_req->coordinates_type  = (int) NORTH;
        parse_failure = 0;
    }
    if (!cli_lldpmed_keyword_cmp("south", req->parm) && lldpmed_req->latitude) {
        lldpmed_req->coordinates_type  = (int) SOUTH;
        parse_failure = 0;
    }
    if (!cli_lldpmed_keyword_cmp("west", req->parm) && lldpmed_req->longitude) {
        lldpmed_req->coordinates_type  = (int) WEST;
        parse_failure = 0;
    }
    if (!cli_lldpmed_keyword_cmp("east", req->parm) && lldpmed_req->longitude) {
        lldpmed_req->coordinates_type  = (int) EAST;
        parse_failure = 0;
    }
    if (!cli_lldpmed_keyword_cmp("meters", req->parm) && lldpmed_req->altitude) {
        lldpmed_req->coordinates_type  = (int) METERS;
        parse_failure = 0;
    }

    if (!cli_lldpmed_keyword_cmp("floor", req->parm) && lldpmed_req->altitude) {
        lldpmed_req->coordinates_type  = (int) FLOOR;
        parse_failure = 0;
    }

    T_DG(TRACE_GRP_CLI, "cli_lldpmed_coordinates_type_parse parse_failure = %d", parse_failure);
    return parse_failure;
}

static int32_t cli_lldpmed_civic_parse(char *cmd, char *cmd2, char *stx,
                                       char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;

    req->parm_parsed = 1;

    lldpmed_cli_req_t *lldpmed_req;
    lldpmed_req = req->module_req;

    if (lldpmed_req->country) {
        // According to the standard section 3.1 the country code must max. be 2 letters long. (Len set
        // to 3 because the parse function takes  the terminating NULL character into account).
        error = cli_parse_text(cmd_org, req->parm, 3);
    } else {
        error = cli_parse_text(cmd_org, req->parm, CIVIC_CA_VALUE_LEN_MAX);
    }
    return error;
}

static int32_t cli_lldpmed_ecs_parse(char *cmd, char *cmd2, char *stx,
                                     char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;

    req->parm_parsed = 1;
    error = cli_parse_text_numbers_only(cmd_org, req->parm, ECS_VALUE_LEN_MAX + 1);
    return error;
}

cli_parm_t lldpmed_parm_table[] = {

    {
        "<tude_type>",
        "The tude_type parameter takes the following values:\n"
        "latitude : Latitude, 0 to 90 degrees with max. 4 digits (Positive numbers are north of the equator and negative numbers are south of the equator).\n"
        "longitude: Longitude, 0 to 180 degrees with max. 4 digits (Positive values are East of the prime meridian and negative numbers are West of the prime meridian).\n"
        "altitude : Altitude, -32767 to 32767 Meters or floors with max. 4 digits.\n",
        CLI_PARM_FLAG_SET,
        cli_lldpmed_tude_type_keyword,
        cli_cmd_lldpmed_lldpmed_coordinates,
    },
    {
        "<datum_type>",
        "The datum_type parameter takes the following values:\n"
        "wgs84        : WGS84\n"
        "nad83_navd88 : NAD83_NAVD88\n"
        "nad83_mllw   : NAD83_MLLW\n",
        CLI_PARM_FLAG_SET,
        cli_lldpmed_tude_type_keyword,
        cli_cmd_lldpmed_lldpmed_datum,
    },

    {
        "<policy_type>",
        "The policy_type parameter takes the following values:\n"
        "voice                 : Voice  for use by dedicated IP Telephony handsets and other similar appliances supporting interactive voice services. These devices are typically deployed on a separate VLAN for ease of deployment and enhanced security by isolation from data applications \n"
        "voice_signaling       : Voice Signaling (conditional)  for use in network topologies that require a different policy for the voice signaling than for the voice media.\n"
        "guest_voice           : Guest Voice  to support a separate limited feature-set voice service for guest users and visitors with their own IP Telephony handsets and other similar appliances supporting interactive voice services.\n"
        "guest_voice_signaling : Guest Voice Signaling (conditional)  for use in network topologies that require a different policy for the guest voice signaling than for the guest voice media.\n"
        "softphone_voice       : Softphone Voice  for use by softphone applications on typical data centric devices, such as PCs or laptops. This class of endpoints frequently does not support multiple VLANs, if at all,and are typically configured to use an untagged VLAN or a single tagged data specific VLAN.\n"
        "video_conferencing    : Video Conferencing  for use by dedicated Video Conferencing equipment and other similar appliances supporting real-time interactive video/audio services.\n"
        "streaming_video       : Streaming Video  for use by broadcast or multicast based video content distribution and other similar applications supporting streaming video services that require specific network policy treatment. Video applications relying on TCP with buffering would not be an intended use of this application type.\n"
        "video_signaling       : Video Signaling (conditional)  for use in network topologies that require a separate policy for the video signaling than for the video media.\n",
        CLI_PARM_FLAG_SET,
        cli_lldpmed_appl_type_keyword,
        NULL,
    },

    {
        "tagged|untagged",
        "tagged          : The device is using tagged frames"
        "unragged        : The device is using untagged frames",
        CLI_PARM_FLAG_NO_TXT,
        cli_lldpmed_parse_keyword,
        NULL,
    },
    {
        "country|state|county|city|district|block|street|leading_street_direction|trailing_street_suffix|str_suf|house_no|house_no_suffix|landmark|additional_info|name|zip_code|building|apartment|floor|room_number|place_type|postal_com_name|p_o_box|additional_code",
        "country                  : Country \n"
        "state                    : National subdivisions (state, caton, region, province, prefecture)\n"
        "county                   : County, parish,gun (JP), district(IN)\n"
        "city                     : City, townchip, shi (JP)\n"
        "district                 : City division,borough, city, district, ward,chou (JP)\n"
        "block                    : Neighborhood, block\n"
        "street                   : Street\n"
        "leading_street_direction : Leading street direction\n"
        "trailing_street_suffix   : Trailing street suffix\n"
        "str_suf                  : Street Suffix\n"
        "house_no                 : House Number\n"
        "house_no_suffix          : House number suffix\n"
        "landmark                 : Landmark or vanity address \n"
        "additional_info          : Additional location information"
        "name                     : Name(residence and office occupant)\n"
        "zip_code                 : Postal/zip code\n"
        "building                 : Building (structure)\n"
        "apartment                : Unit (apartment, suite)\n"
        "floor                    : Floor\n"
        "room_number              : Room number\n"
        "place_type               : Placetype\n"
        "postal_com_name          : Postal community name\n"
        "p_o_box                  : Post office box (P.O. Box)\n"
        "additional_code          : Additional code\n"
        "(default: Show Civic Address Location configuration)",
        CLI_PARM_FLAG_NO_TXT,
        cli_lldpmed_parse_keyword,
        NULL,
    },
    {
        "<civic_value>",
        "lldpmed The value for the Civic Address Location entry.",
        CLI_PARM_FLAG_SET,
        cli_lldpmed_civic_parse,
        NULL,
    },
    {
        "<ecs_value>",
        "lldpmed The value for the Emergency Call Service",
        CLI_PARM_FLAG_SET,
        cli_lldpmed_ecs_parse,
        NULL,
    },

    {
        "<policy_list>",
        "List of policies to delete",
        CLI_PARM_FLAG_SET,
        cli_lldpmed_policy_list_parse,
        NULL,
    },

    {
        "<count>",
        "The number of times the fast start LLDPDU are being sent during the activation of the fast start mechanism defined by LLDP-MED (1-10).",
        CLI_PARM_FLAG_SET,
        cli_lldpmed_fast_start_repeat_count_parse,
        NULL,
    },

    {
        "<vlan_id>",
        "VLAN id",
        CLI_PARM_FLAG_SET,
        cli_lldpmed_vlan_id_parse,
        NULL,
    },

    {
        "<l2_priority>",
        "This field may specify one of eight priority levels (0 through 7), as defined by IEEE 802.1D-2004 [3].",
        CLI_PARM_FLAG_SET,
        cli_lldpmed_l2_priority_parse,
        NULL,
    },

    {
        "<dscp>",
        "This field shall contain the DSCP value to be used to provide Diffserv node behavior for the specified application type as defined in IETF RFC 2474 [5]. This 6 bit field may contain one of 64 code point values (0 through 63). A value of 0 represents use of the default DSCP value as defined in RFC 2475.",
        CLI_PARM_FLAG_SET,
        cli_lldpmed_dscp_parse,
        NULL,
    },


    {
        "coordinate_value",
        "Coordinate value",
        CLI_PARM_FLAG_SET,
        cli_lldpmed_coordinates_value_parse,
        NULL,
    },
    {
        "<direction>",
        "The direction parameter takes the following values:\n"
        "North       : North (Valid for latitude)\n"
        "South       : South (Valid for latitude)\n"
        "West        : West (Valid for longitude)\n"
        "East        : East (Valid for longitude)\n"
        "Meters      : Meters (Valid for altitude)\n"
        "Floor       : Floor (Valid for altitude)\n",
        CLI_PARM_FLAG_SET,
        cli_lldpmed_coordinates_type_parse,
        NULL,
    },
#ifdef VTSS_SW_OPTION_LLDP_MED_DEBUG
    {
        "enable|disable",
        "enable : Enable  - Set medTansmitEnable variable to true\n"
        "disable: Disable - Set medTansmitEnable variable to false\n"
        "(default: Show medTansmitEnable variable value)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        lldpmed_cli_cmd_debug_lldpmed,
    },
#endif
    {
        NULL,
        NULL,
        0,
        0,
        NULL
    },
};

enum {
    PRIO_LLDPMED_CONF,
    PRIO_LLDPMED_CIVIC,
    PRIO_LLDPMED_ECS,
    PRIO_LLDPMED_POLICY_DELETE,
    PRIO_LLDPMED_POLICY_ADD,
    PRIO_LLDPMED_PORT_POLICY,
    PRIO_LLDPMED_COORDINATES,
    PRIO_LLDPMED_COORDINATES_DATUM,
    PRIO_LLDPMED_FAST_START_REPEAT_COUNT,
    PRIO_LLDPMED_LOCATION,
    PRIO_LLDPMED_INFO,
    PRIO_LLDPMED_DEBUG_MED_TRANSMIT =  CLI_CMD_SORT_KEY_DEFAULT,
};

/* Command table entries */
cli_cmd_tab_entry (
    "LLDPMED Configuration [<port_list>]",
    NULL,
    "Show LLDP-MED configuration",
    PRIO_LLDPMED_CONF,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_LLDPMED,
    cli_cmd_lldpmed_config,
    NULL,
    lldpmed_parm_table,
    CLI_CMD_FLAG_SYS_CONF
);


cli_cmd_tab_entry (
    "LLDPMED Info [<port_list>]",
    NULL,
    "Show LLDP-MED neighbor device information",
    PRIO_LLDPMED_INFO,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_LLDPMED,
    cli_cmd_lldpmed_lldpmed_info,
    NULL,
    lldpmed_parm_table,
    CLI_CMD_FLAG_NONE
);


cli_cmd_tab_entry (
    "LLDPMED Fast",
    "LLDPMED Fast [<count>]",
    "Set or show LLDP-MED Fast Start Repeat Count",
    PRIO_LLDPMED_FAST_START_REPEAT_COUNT,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_LLDPMED,
    cli_cmd_lldpmed_lldpmed_fast_start_repeat_count,
    NULL,
    lldpmed_parm_table,
    CLI_CMD_FLAG_NONE
);


cli_cmd_tab_entry (
    "LLDPMED Civic",
    "LLDPMED Civic [country|state|county|city|district|block|street|leading_street_direction|trailing_street_suffix|str_suf|house_no|house_no_suffix|landmark|additional_info|name|zip_code|building|apartment|floor|room_number|place_type|postal_com_name|p_o_box|additional_code] [<civic_value>]",
    "Set or show LLDP-MED Civic Address Location",
    PRIO_LLDPMED_CIVIC,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_LLDPMED,
    cli_cmd_lldpmed_lldpmed_civic,
    NULL,
    lldpmed_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "LLDPMED Coordinates",
    "LLDPMED Coordinates [<tude_type>]  [<direction>] [coordinate_value] ",
    "Set or show LLDP-MED Location",
    PRIO_LLDPMED_COORDINATES,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_LLDPMED,
    cli_cmd_lldpmed_lldpmed_coordinates,
    NULL,
    lldpmed_parm_table,
    CLI_CMD_FLAG_NONE
);


cli_cmd_tab_entry (
    "LLDPMED Datum",
    "LLDPMED Datum [<datum_type>]",
    "Set or show LLDP-MED Coordinates map datum",
    PRIO_LLDPMED_COORDINATES_DATUM,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_LLDPMED,
    cli_cmd_lldpmed_lldpmed_datum,
    NULL,
    lldpmed_parm_table,
    CLI_CMD_FLAG_NONE
);


cli_cmd_tab_entry (
    "LLDPMED ecs",
    "LLDPMED ecs [<ecs_value>]",
    "Set or show LLDP-MED Emergency Call Service",
    PRIO_LLDPMED_ECS,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_LLDPMED,
    cli_cmd_lldpmed_lldpmed_ecs,
    NULL,
    lldpmed_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "LLDPMED policy delete",
    "LLDPMED policy delete <policy_list>",
    "Delete the selected policy",
    PRIO_LLDPMED_POLICY_DELETE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_LLDPMED,
    cli_cmd_lldpmed_lldpmed_policy_delete,
    NULL,
    lldpmed_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "LLDPMED policy add",
    "LLDPMED policy add <policy_type> [tagged|untagged] [<vlan_id>] [<l2_priority>] [<dscp>]",
    "Adds a policy to the list of policies",
    PRIO_LLDPMED_POLICY_ADD,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_LLDPMED,
    cli_cmd_lldpmed_lldpmed_policy_add,
    NULL,
    lldpmed_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "LLDPMED port policies",
    "LLDPMED port policies [<port_list>] [<policy_list>]",
    "Set or show LLDP-MED port policies",
    PRIO_LLDPMED_PORT_POLICY,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_LLDPMED,
    cli_cmd_lldpmed_lldpmed_port_policy,
    NULL,
    lldpmed_parm_table,
    CLI_CMD_FLAG_NONE
);

/*
cli_cmd_tab_entry (
    "LLDPMED Optional_TLV [<port_list>]",
    "LLDPMED Optional_TLV [<port_list>] [capabilities|network_policy|location] [enable|disable]",
    "Set or show LLDPMED Optional TLVs",
    PRIO_LLDP_OPT_TLV,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_LLDP,
    cli_cmd_lldp_opt_tlv,
    NULL,
    lldp_parm_table,
    CLI_CMD_FLAG_NONE
);
*/

//
// Debug commands
//
#ifdef VTSS_SW_OPTION_LLDP_MED_DEBUG
cli_cmd_tab_entry (
    "Debug lldpmed_tx_ena [<port_list>]",
    "Debug lldpmed_tx_ena [<port_list>] [enable|disable]",
    "Set or show if the current value of the global medTansmitEnable variable (Section  Section 11.2.1, TIA 1057)",
    PRIO_LLDPMED_DEBUG_MED_TRANSMIT,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    lldpmed_cli_cmd_debug_lldpmed,
    NULL,
    lldpmed_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif
