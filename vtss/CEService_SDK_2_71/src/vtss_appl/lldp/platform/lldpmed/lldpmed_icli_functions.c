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
#ifdef VTSS_SW_OPTION_LLDP_MED

#include "icli_api.h"
#include "icli_porting_util.h"
#include "lldp.h"
#include "lldp_api.h"
#include "lldp_os.h"
#include "msg_api.h"
#include "misc_api.h" // for uport2iport
#ifdef VTSS_SW_OPTION_ICFG
#include "icfg_api.h"
#endif
#include "mgmt_api.h"
#include "lldpmed_rx.h"
#include "lldp_icli_shared_functions.h" // For lldp_local_interface_txt_get
/***************************************************************************/
/*  Code start :)                                                           */
/****************************************************************************/

/***************************************************************************/
/*  Internal functions                                                     */
/****************************************************************************/

// Help function for setting optional TLVs
// Input parameters same as for function lldpmed_icli_transmit_tlv_set
// Return - Vitesse return code
static vtss_rc lldpmed_icli_transmit_tlv_set(icli_stack_port_range_t *plist, BOOL has_capabilities, BOOL has_location, BOOL has_network_policy, BOOL no)
{
  switch_iter_t  sit;
  port_iter_t    pit;
  lldp_struc_0_t lldp_conf;

  VTSS_RC(icli_switch_iter_init(&sit));

  while (icli_switch_iter_getnext(&sit, plist)) {
    // get current configuration
    lldp_mgmt_get_config(&lldp_conf, sit.isid);

    // Loop through ports.
    VTSS_RC(icli_port_iter_init(&pit, sit.isid, PORT_ITER_FLAGS_FRONT));
    while (icli_port_iter_getnext(&pit, plist)) {
      T_IG_PORT(TRACE_GRP_CLI, pit.iport, "no:%d, has_capabilities:%d, tlv:0x%X", no, has_capabilities, lldp_conf.lldpmed_optional_tlv[pit.iport]);

      if (has_capabilities) {
        if (no) {
          // Clear the capabilities bit.
          lldp_conf.lldpmed_optional_tlv[pit.iport] &= ~OPTIONAL_TLV_CAPABILITIES_BIT;
        } else {
          // Set the capabilities bit.
          lldp_conf.lldpmed_optional_tlv[pit.iport] |= OPTIONAL_TLV_CAPABILITIES_BIT;
        }
      }

      if (has_network_policy) {
        if (no) {
          // Clear the network-policy bit.
          lldp_conf.lldpmed_optional_tlv[pit.iport] &= ~OPTIONAL_TLV_POLICY_BIT;
        } else {
          // Set the network-policy bit.
          lldp_conf.lldpmed_optional_tlv[pit.iport] |= OPTIONAL_TLV_POLICY_BIT;
        }
      }

      if (has_location) {
        if (no) {
          // Clear the location bit.
          lldp_conf.lldpmed_optional_tlv[pit.iport] &= ~OPTIONAL_TLV_LOCATION_BIT;
        } else {
          // Set the location bit.
          lldp_conf.lldpmed_optional_tlv[pit.iport] |= OPTIONAL_TLV_LOCATION_BIT;
        }
      }
      T_IG_PORT(TRACE_GRP_CLI, pit.iport, "tlv:0x%X", lldp_conf.lldpmed_optional_tlv[pit.iport]);
    }

    VTSS_RC(lldp_mgmt_set_config(&lldp_conf, sit.isid));
  }  // while icli_switch_iter_getnext
  return VTSS_RC_OK;
}

// Help function for printing neighbor inventory list
// In : session_id - Session_Id for ICLI_PRINTF
//      entry      - The information from the remote neighbor
static void icli_cmd_lldpmed_print_inventory(i32 session_id, lldp_remote_entry_t *entry)
{
  lldp_8_t inventory_str[MAX_LLDPMED_INVENTORY_LENGTH];


  if (entry->lldpmed_hw_rev_length            > 0 ||
      entry->lldpmed_firm_rev_length          > 0 ||
      entry->lldpmed_sw_rev_length            > 0 ||
      entry->lldpmed_serial_no_length         > 0 ||
      entry->lldpmed_manufacturer_name_length > 0 ||
      entry->lldpmed_model_name_length        > 0 ||
      entry->lldpmed_assert_id_length         > 0) {
    ICLI_PRINTF("\nInventory \n");

    strncpy(inventory_str, entry->lldpmed_hw_rev, entry->lldpmed_hw_rev_length);
    T_DG(TRACE_GRP_RX, "entry->lldpmed_hw_rev_length = %d", entry->lldpmed_hw_rev_length);
    inventory_str[entry->lldpmed_hw_rev_length] = '\0';// Clear the string since data from the LLDP entry does contain NULL pointer.
    ICLI_PRINTF("%-20s: %s \n", "Hardware Revision", inventory_str);


    strncpy(inventory_str, entry->lldpmed_firm_rev, entry->lldpmed_firm_rev_length);
    T_DG(TRACE_GRP_RX, "entry->lldpmed_hw_rev_length = %d", entry->lldpmed_firm_rev_length);
    inventory_str[entry->lldpmed_firm_rev_length] = '\0'; // Add NULL pointer since data from the LLDP entry does contain NULL pointer.
    ICLI_PRINTF("%-20s: %s \n", "Firmware Revision", inventory_str);

    strncpy(inventory_str, entry->lldpmed_sw_rev, entry->lldpmed_sw_rev_length);
    inventory_str[entry->lldpmed_sw_rev_length] = '\0'; // Add NULL pointer since data from the LLDP entry does contain NULL pointer.
    ICLI_PRINTF("%-20s: %s \n", "Software Revision", inventory_str);

    strncpy(inventory_str, entry->lldpmed_serial_no, entry->lldpmed_serial_no_length);
    inventory_str[entry->lldpmed_serial_no_length] = '\0'; // Add NULL pointer since data from the LLDP entry does contain NULL pointer.
    ICLI_PRINTF("%-20s: %s \n", "Serial Number", inventory_str);

    strncpy(inventory_str, entry->lldpmed_manufacturer_name, entry->lldpmed_manufacturer_name_length);
    inventory_str[entry->lldpmed_manufacturer_name_length] = '\0'; // Add NULL pointer since data from the LLDP entry does contain NULL pointer.
    ICLI_PRINTF("%-20s: %s \n", "Manufacturer Name", inventory_str);

    strncpy(inventory_str, entry->lldpmed_model_name, entry->lldpmed_model_name_length);
    inventory_str[entry->lldpmed_model_name_length] = '\0'; // Add NULL pointer since data from the LLDP entry does contain NULL pointer.
    ICLI_PRINTF("%-20s: %s \n", "Model Name", inventory_str);

    strncpy(inventory_str, entry->lldpmed_assert_id, entry->lldpmed_assert_id_length);
    inventory_str[entry->lldpmed_assert_id_length] = '\0'; // Add NULL pointer since data from the LLDP entry does contain NULL pointer.
    ICLI_PRINTF("%-20s: %s \n", "Assert ID", inventory_str);
  }
}

// Help function for printing the LLDP-MED neighbor information in an entry.
//
// In : session_id    - Session_Id for ICLI_PRINTF
//      sit           - Pointer to switch information
//      has_interface - TRUE if the user has specified a specific interface to show
//      plist         - list containing information about which ports to show the information for.
static void icli_cmd_lldpmed_print_info(i32 session_id, switch_iter_t *sit, BOOL has_interface, icli_stack_port_range_t *plist)
{
  lldp_8_t   buf[1000];
  lldp_u8_t  p_index;
  port_iter_t    pit;
  u32 i;
  lldp_remote_entry_t *table_m, *table_p = NULL, *entry = NULL;
  BOOL lldpmed_no_entry_found = TRUE;

  if (!msg_switch_exists(sit->isid)) {
    return;
  }

  // Loop through all front ports
  if (icli_port_iter_init(&pit, sit->isid, PORT_ITER_SORT_ORDER_IPORT | PORT_ITER_FLAGS_FRONT) == VTSS_OK) {
    // Because the iCLI can be stopped in the middle of the printout (waiting for user input to continue),
    // we need to make a copy of the entry table in order not to block for access to the entry table (and holding the
    // semaphore too long, with the lldp_mgmt_get_lock function.
    if ((table_m = VTSS_MALLOC(LLDP_ENTRIES_TABLE_SIZE)) == NULL) {
      T_E("Error trying to malloc");
      return;
    }

    lldp_mgmt_get_lock();
    table_p = lldp_mgmt_get_entries(sit->isid); // Get the LLDP entries for the switch in question.
    memcpy(table_m, table_p, LLDP_ENTRIES_TABLE_SIZE);
    lldp_mgmt_get_unlock();
    T_IG(TRACE_GRP_CLI, "has_interface:%d", has_interface);

    while (icli_port_iter_getnext(&pit, plist)) {
      for (i = 0, entry = table_m; i < lldp_remote_get_max_entries(); i++, entry++) {
        if (entry->in_use == 0 || !entry->lldpmed_info_vld || (entry->receive_port != pit.iport)) {
          continue;
        }
        lldpmed_no_entry_found = FALSE;

        T_IG_PORT(TRACE_GRP_CLI, entry->receive_port, "size = %zu, lldpmed_info_vld = %d", sizeof(buf), entry->lldpmed_info_vld);

        if (entry->lldpmed_info_vld) {
          ICLI_PRINTF("%-20s: %s\n", "Local port", lldp_local_interface_txt_get(buf, entry, sit, &pit));


          // Device type / capabilities
          if (entry->lldpmed_capabilities_vld) {
            lldpmed_device_type2str(entry, buf);
            ICLI_PRINTF("%-20s: %s \n", "Device Type", buf);

            lldpmed_capabilities2str(entry, buf);
            ICLI_PRINTF("%-20s: %s \n", "Capabilities", buf);
          }

          // Loop through policies
          for (p_index = 0; p_index < MAX_LLDPMED_POLICY_APPLICATIONS_CNT; p_index ++) {
            // make sure that policy exist.
            T_NG(TRACE_GRP_CLI, "Policy valid = %d, p_index = %d", entry->lldpmed_policy_vld[p_index], p_index);
            if (entry->lldpmed_policy_vld[p_index] == LLDP_FALSE) {
              T_NG(TRACE_GRP_CLI, "Continue");
              continue;
            }

            // Policies
            lldpmed_policy_appl_type2str(entry->lldpmed_policy[p_index], buf);
            ICLI_PRINTF("\n%-20s: %s \n", "Application Type", buf);

            lldpmed_policy_flag_type2str(entry->lldpmed_policy[p_index], buf);
            ICLI_PRINTF("%-20s: %s \n", "Policy", buf);

            lldpmed_policy_tag2str(entry->lldpmed_policy[p_index], buf);
            ICLI_PRINTF("%-20s: %s \n", "Tag", buf);

            lldpmed_policy_vlan_id2str(entry->lldpmed_policy[p_index], buf);
            ICLI_PRINTF("%-20s: %s \n", "VLAN ID", buf);

            lldpmed_policy_prio2str(entry->lldpmed_policy[p_index], buf);
            ICLI_PRINTF("%-20s: %s \n", "Priority", buf);

            lldpmed_policy_dscp2str(entry->lldpmed_policy[p_index], buf);
            ICLI_PRINTF("%-20s: %s \n", "DSCP", buf);
          }


          for (p_index = 0; p_index < MAX_LLDPMED_LOCATION_CNT; p_index ++) {
            if (entry->lldpmed_location_vld[p_index] == 1) {
              lldpmed_location2str(entry, buf, p_index);
              ICLI_PRINTF("%-20s: %s \n", "Location", buf);
            }
          }

          icli_cmd_lldpmed_print_inventory(session_id, entry);

          ICLI_PRINTF("\n");
        }
      }
    }

    VTSS_FREE(table_m);

    T_DG(TRACE_GRP_CLI, "lldpmed_no_entry_found:%d", lldpmed_no_entry_found);
    if (lldpmed_no_entry_found) {
      ICLI_PRINTF("No LLDP-MED entries found\n");
    }
  }
}

// See lldpmed_icli_functions.h
void lldpmed_icli_show_remote_device(i32 session_id, BOOL has_interface, icli_stack_port_range_t *plist)
{
  switch_iter_t sit;

  // Loop all the switches in question.
  if (icli_switch_iter_init(&sit) == VTSS_RC_OK) {
    while (icli_switch_iter_getnext(&sit, plist)) {
      icli_cmd_lldpmed_print_info(session_id, &sit, has_interface, plist);
    }
  }
}

// Help function for printing the policies list.
// In : Session_Id - session_id for ICLI_PRINTF
//      conf       - Current configuration
//      policy_index - Index for the policy to print.
static void lldpmed_icli_print_policies(i32 session_id, const lldp_struc_0_t *lldp_conf, u32 policy_index)
{
  lldp_8_t application_type_str[200];

  if (lldp_conf->policies_table[policy_index].in_use) {
    lldpmed_appl_type2str(lldp_conf->policies_table[policy_index].application_type, &application_type_str[0]);
    ICLI_PRINTF("%-10d %-25s %-8s %-8d %-12d %-8d \n",
                policy_index,
                application_type_str,
                lldp_conf->policies_table[policy_index].tagged_flag ? "Tagged" : "Untagged",
                lldp_conf->policies_table[policy_index].vlan_id,
                lldp_conf->policies_table[policy_index].l2_priority,
                lldp_conf->policies_table[policy_index].dscp_value );
  }
}

// Help function for assigning LLDP policies
// Input parameters same as lldpmed_icli_assign_policy function
// return - Vitesse return code
static vtss_rc lldpmed_icli_assign_policy_set(i32 session_id, icli_stack_port_range_t *plist, icli_range_t *policies_list, BOOL no)
{
  switch_iter_t  sit;
  port_iter_t    pit;
  lldp_struc_0_t lldp_conf;

  VTSS_RC(icli_switch_iter_init(&sit));

  while (icli_switch_iter_getnext(&sit, plist)) {
    // Get current configuration
    lldp_mgmt_get_config(&lldp_conf, sit.isid);

    // Loop through ports.
    VTSS_RC(icli_port_iter_init(&pit, sit.isid, PORT_ITER_FLAGS_FRONT));
    while (icli_port_iter_getnext(&pit, plist)) {
      u32 p_index;
      i32 range_index;

      // If user doesn't select some specific policies then remove them all (This only should only happen for the "no" command
      if (policies_list == NULL) {
        if (!no) {
          T_E("policies list is NULL. Should only happen for the no command");
        }
        // Remove all policies from this interface port.
        for (range_index = 0; range_index < LLDPMED_POLICIES_CNT; range_index++) {
          lldp_conf.ports_policies[pit.iport][range_index] = FALSE;
        }
      } else {
        for (p_index = 0; p_index < policies_list->u.sr.cnt; p_index++) {
          for (range_index = policies_list->u.sr.range[p_index].min; range_index <= policies_list->u.sr.range[p_index].max; range_index++) {
            if (no) {
              lldp_conf.ports_policies[pit.iport][range_index] = FALSE; // Remove the policy from this port
            } else {
              // Make sure we don't get out of bounce
              if (range_index > LLDPMED_POLICY_MAX) {
                ICLI_PRINTF("%% Ignoring invalid policy:%d. Valid range is %u-%u\n", range_index, LLDPMED_POLICY_MIN, LLDPMED_POLICY_MAX);
                continue;
              }

              if ((lldp_conf.policies_table[range_index].in_use == 0)) {
                ICLI_PRINTF("Ignoring policy %d for port %u because no such policy is defined \n", range_index, pit.uport);
              } else {
                lldp_conf.ports_policies[pit.iport][range_index] = TRUE;
              }
            }
          }
        }
      }
    }

    VTSS_ICLI_ERR_PRINT(lldp_mgmt_set_config(&lldp_conf, sit.isid));
  }  // while icli_switch_iter_getnext
  return VTSS_RC_OK;
}

/***************************************************************************/
/*  Functions called from iCLI                                             */
/****************************************************************************/
// See lldpmed_icli_functions.h
vtss_rc lldpmed_icli_civic_addr(i32 session_id, BOOL has_country, BOOL has_state, BOOL has_county, BOOL has_city, BOOL has_district, BOOL has_block, BOOL has_street, BOOL has_leading_street_direction, BOOL has_trailing_street_suffix, BOOL has_str_suf, BOOL has_house_no, BOOL has_house_no_suffix, BOOL has_landmark, BOOL has_additional_info, BOOL has_name, BOOL has_zip_code, BOOL has_building, BOOL has_apartment, BOOL has_floor, BOOL has_room_number, BOOL has_place_type, BOOL has_postal_com_name, BOOL has_p_o_box, BOOL has_additional_code, char *v_string250)
{
  lldp_common_conf_t lldp_conf;

  if (v_string250 == NULL) {
    T_E("Sting is NULL - Should never happen");
    return VTSS_RC_ERROR;
  }

  lldp_mgmt_get_common_config(&lldp_conf);

  if (has_country) {
    misc_strncpyz(lldp_conf.location_info.ca_country_code, v_string250, CA_COUNTRY_CODE_LEN);
  }

  if (has_state) {
    lldpmed_update_civic_info(&lldp_conf.location_info.civic, LLDPMED_CATYPE_A1, v_string250);
  }

  if (has_county) {
    lldpmed_update_civic_info(&lldp_conf.location_info.civic, LLDPMED_CATYPE_A2, v_string250);
  }

  if (has_city) {
    lldpmed_update_civic_info(&lldp_conf.location_info.civic, LLDPMED_CATYPE_A3, v_string250);
  }

  if (has_district) {
    lldpmed_update_civic_info(&lldp_conf.location_info.civic, LLDPMED_CATYPE_A4, v_string250);
  }

  if (has_block) {
    lldpmed_update_civic_info(&lldp_conf.location_info.civic, LLDPMED_CATYPE_A5, v_string250);
  }

  if (has_street) {
    lldpmed_update_civic_info(&lldp_conf.location_info.civic, LLDPMED_CATYPE_A6, v_string250);
  }

  if (has_leading_street_direction) {
    lldpmed_update_civic_info(&lldp_conf.location_info.civic, LLDPMED_CATYPE_PRD, v_string250);
  }

  if (has_trailing_street_suffix) {
    lldpmed_update_civic_info(&lldp_conf.location_info.civic, LLDPMED_CATYPE_POD, v_string250);
  }

  if (has_str_suf) {
    lldpmed_update_civic_info(&lldp_conf.location_info.civic, LLDPMED_CATYPE_STS, v_string250);
  }

  if (has_house_no_suffix) {
    lldpmed_update_civic_info(&lldp_conf.location_info.civic, LLDPMED_CATYPE_HNS, v_string250);
  }

  if (has_house_no) {
    lldpmed_update_civic_info(&lldp_conf.location_info.civic, LLDPMED_CATYPE_HNO, v_string250);
  }

  if (has_landmark) {
    lldpmed_update_civic_info(&lldp_conf.location_info.civic, LLDPMED_CATYPE_LMK, v_string250);
  }

  if (has_additional_info) {
    lldpmed_update_civic_info(&lldp_conf.location_info.civic, LLDPMED_CATYPE_LOC, v_string250);
  }

  if (has_name) {
    lldpmed_update_civic_info(&lldp_conf.location_info.civic, LLDPMED_CATYPE_NAM, v_string250);
  }

  T_N("has_zip_code:%d - %s", has_zip_code, v_string250);
  if (has_zip_code) {
    lldpmed_update_civic_info(&lldp_conf.location_info.civic, LLDPMED_CATYPE_ZIP, v_string250);
  }

  if (has_building) {
    lldpmed_update_civic_info(&lldp_conf.location_info.civic, LLDPMED_CATYPE_BUILD, v_string250);
  }

  if (has_apartment) {
    lldpmed_update_civic_info(&lldp_conf.location_info.civic, LLDPMED_CATYPE_UNIT, v_string250);
  }

  if (has_floor) {
    lldpmed_update_civic_info(&lldp_conf.location_info.civic, LLDPMED_CATYPE_FLR, v_string250);
  }

  if (has_room_number) {
    lldpmed_update_civic_info(&lldp_conf.location_info.civic, LLDPMED_CATYPE_ROOM, v_string250);
  }

  if (has_place_type) {
    lldpmed_update_civic_info(&lldp_conf.location_info.civic, LLDPMED_CATYPE_PLACE, v_string250);
  }

  if (has_postal_com_name) {
    lldpmed_update_civic_info(&lldp_conf.location_info.civic, LLDPMED_CATYPE_PCN, v_string250);
  }

  if (has_p_o_box) {
    lldpmed_update_civic_info(&lldp_conf.location_info.civic, LLDPMED_CATYPE_POBOX, v_string250);
  }

  if (has_additional_code) {
    lldpmed_update_civic_info(&lldp_conf.location_info.civic, LLDPMED_CATYPE_ADD_CODE, v_string250);
  }

  VTSS_ICLI_ERR_PRINT(lldp_mgmt_set_common_config(&lldp_conf));
  return VTSS_RC_OK;
}


// See lldpmed_icli_functions.h
vtss_rc lldpmed_icli_elin_addr(i32 session_id, char *elin_string)
{
  lldp_common_conf_t lldp_conf;

  if (elin_string == NULL) {
    T_E("elin_string is NULL. Shall never happen");
    return VTSS_RC_ERROR;
  }

  lldp_mgmt_get_common_config(&lldp_conf); // Get current configuration

  misc_strncpyz(lldp_conf.location_info.ecs, elin_string, ECS_VALUE_LEN_MAX); // Update the elin parameter.

  VTSS_ICLI_ERR_PRINT(lldp_mgmt_set_common_config(&lldp_conf));
  return VTSS_RC_OK;
}

// See lldpmed_icli_functions.h
void lldpmed_icli_show_policies(i32 session_id, icli_unsigned_range_t *policies_list)
{
  lldp_struc_0_t      lldp_conf;
  lldp_16_t policy_index;
  u32 i, idx;
  BOOL at_least_one_policy_defined = FALSE;
  // get current configuration
  lldp_mgmt_get_config(&lldp_conf, VTSS_ISID_LOCAL);

  // Find out if any policy is defined.
  for (policy_index = LLDPMED_POLICY_MIN ; policy_index <= LLDPMED_POLICY_MAX; policy_index++) {
    if (lldp_conf.policies_table[policy_index].in_use) {
      at_least_one_policy_defined = TRUE;
      break;
    }
  }

  // Print out "header"
  if (at_least_one_policy_defined) {
    ICLI_PRINTF("%-10s %-25s %-8s %-8s %-12s %-8s \n", "Policy Id", "Application Type", "Tag", "Vlan ID", "L2 Priority", "DSCP");
  } else {
    ICLI_PRINTF("No policies defined\n");
    return;
  }

  // Print all policies that are currently in use.
  if (policies_list != NULL) {
    // User want some specific policies
    T_IG(TRACE_GRP_CLI, "cnt:%u", policies_list->cnt);
    for ( i = 0; i < policies_list->cnt; i++ ) {
      T_IG(TRACE_GRP_CLI, "(%u, %u) ", policies_list->range[i].min, policies_list->range[i].max);
      for ( idx = policies_list->range[i].min; idx <= policies_list->range[i].max; idx++ ) {
        lldpmed_icli_print_policies(session_id, &lldp_conf, idx);
      }
    }
  } else {
    // User want all policies
    for (policy_index = LLDPMED_POLICY_MIN ; policy_index <= LLDPMED_POLICY_MAX; policy_index++) {
      lldpmed_icli_print_policies(session_id, &lldp_conf, policy_index);
    }
  }
}

// See lldpmed_icli_functions.h
vtss_rc lldpmed_icli_latitude(i32 session_id, BOOL north, BOOL south, char *degree)
{
  lldp_common_conf_t lldp_conf;
  long value = 0;
  // get current configuration
  lldp_mgmt_get_common_config(&lldp_conf);

  // convert floating point "string value" to long
  T_IG(TRACE_GRP_CLI, "Degree:%s", degree);
  if (mgmt_str_float2long(degree, &value, LLDPMED_LATITUDE_VALUE_MIN, LLDPMED_LATITUDE_VALUE_MAX, TUDE_DIGIT) != VTSS_RC_OK) {
    ICLI_PRINTF("Degree \"%s\" is not valid. Must be in the range 0.0000 to 90.0000\n", degree);
    return VTSS_RC_ERROR;
  }
  lldp_conf.location_info.latitude = value;


  if (north) {
    lldp_conf.location_info.latitude_dir = NORTH;
  }

  if (south) {
    lldp_conf.location_info.latitude_dir = SOUTH;
  }

  //Set current configuration
  VTSS_ICLI_ERR_PRINT(lldp_mgmt_set_common_config(&lldp_conf));
  return VTSS_RC_OK;
}

// See lldpmed_icli_functions.h
vtss_rc lldpmed_icli_longitude(i32 session_id, BOOL east, BOOL west, char *degree)
{
  lldp_common_conf_t lldp_conf;
  long value = 0;
  // get current configuration
  lldp_mgmt_get_common_config(&lldp_conf);


  // convert floating point "string value" to long
  T_IG(TRACE_GRP_CLI, "Degree:%s", degree);
  if (mgmt_str_float2long(degree, &value, LLDPMED_LONGITUDE_VALUE_MIN, LLDPMED_LONGITUDE_VALUE_MAX, TUDE_DIGIT) != VTSS_RC_OK) {
    ICLI_PRINTF("Degree \"%s\" is not valid. Must be in the range 0.0000 to 180.0000\n", degree);
    return VTSS_RC_ERROR;
  }
  lldp_conf.location_info.longitude = value;


  if (east) {
    lldp_conf.location_info.longitude_dir = EAST;
  }

  if (west) {
    lldp_conf.location_info.longitude_dir = WEST;
  }

  //Set current configuration
  VTSS_ICLI_ERR_PRINT(lldp_mgmt_set_common_config(&lldp_conf));
  return VTSS_RC_OK;
}

// See lldpmed_icli_functions.h
vtss_rc lldpmed_icli_altitude(i32 session_id, BOOL meters, BOOL floors, char *value_str)
{
  lldp_common_conf_t lldp_conf;
  long value = 0;
  // get current configuration
  lldp_mgmt_get_common_config(&lldp_conf);


  // convert floating point "string value" to long
  if (mgmt_str_float2long(value_str, &value, LLDPMED_ALTITUDE_VALUE_MIN, LLDPMED_ALTITUDE_VALUE_MAX, TUDE_DIGIT) != VTSS_RC_OK) {
    ICLI_PRINTF("Altitude \"%s\" is not valid. Must be in the range -32767.0000 to 32767.0000\n", value_str);
    return VTSS_RC_ERROR;
  }

  lldp_conf.location_info.altitude = value;


  if (meters) {
    lldp_conf.location_info.altitude_type = METERS;
  }

  if (floors) {
    lldp_conf.location_info.altitude_type = FLOOR;
  }

  //Set current configuration
  VTSS_ICLI_ERR_PRINT(lldp_mgmt_set_common_config(&lldp_conf));
  return VTSS_RC_OK;
}

// See lldpmed_icli_functions.h
vtss_rc lldpmed_icli_datum(i32 session_id, BOOL has_wgs84, BOOL has_nad83_navd88, BOOL has_nad83_mllw, BOOL no)
{
  // get current configuration
  lldp_common_conf_t lldp_conf;
  lldp_mgmt_get_common_config(&lldp_conf);

  if (no) {
    lldp_conf.location_info.datum = LLDPMED_DATUM_DEFAULT;
  } else if (has_wgs84) {
    lldp_conf.location_info.datum = WGS84;
  } else if (has_nad83_navd88) {
    lldp_conf.location_info.datum = NAD83_NAVD88;
  } else if (has_nad83_mllw) {
    lldp_conf.location_info.datum = NAD83_MLLW;
  }

  //Set current configuration
  VTSS_ICLI_ERR_PRINT(lldp_mgmt_set_common_config(&lldp_conf));
  return VTSS_RC_OK;
}

// See lldpmed_icli_functions.h
vtss_rc lldpmed_icli_fast_start(i32 session_id, u32 value, BOOL no)
{
  lldp_common_conf_t lldp_conf;
  lldp_mgmt_get_common_config(&lldp_conf);

  if (no) {
    lldp_conf.medFastStartRepeatCount = FAST_START_REPEAT_COUNT_DEFAULT;
  } else {
    lldp_conf.medFastStartRepeatCount = value;
  }

  //Set current configuration
  VTSS_ICLI_ERR_PRINT(lldp_mgmt_set_common_config(&lldp_conf));
  return VTSS_RC_OK;
}

// See lldpmed_icli_functions.h
vtss_rc lldpmed_icli_assign_policy(i32 session_id, icli_stack_port_range_t *plist, icli_range_t *policies_list, BOOL no)
{
  VTSS_ICLI_ERR_PRINT(lldpmed_icli_assign_policy_set(session_id, plist, policies_list, no));
  return VTSS_RC_OK;
}

// See lldpmed_icli_functions.h
vtss_rc lldpmed_icli_media_vlan_policy(i32 session_id, u32 policy_index, BOOL has_voice, BOOL has_voice_signaling, BOOL has_guest_voice_signaling,
                                       BOOL has_guest_voice, BOOL has_softphone_voice, BOOL has_video_conferencing, BOOL has_streaming_video,
                                       BOOL has_video_signaling, BOOL has_tagged, BOOL has_untagged, u32 v_vlan_id, u32 v_0_to_7, u32 v_0_to_63)
{
  lldp_common_conf_t lldp_conf;
  lldp_mgmt_get_common_config(&lldp_conf);
  BOOL update_conf = TRUE;


  lldp_conf.policies_table[policy_index].in_use = TRUE;
  if (has_voice) {
    lldp_conf.policies_table[policy_index].application_type = VOICE;
  } else if (has_voice_signaling) {
    lldp_conf.policies_table[policy_index].application_type = VOICE_SIGNALING;
  } else if (has_guest_voice) {
    lldp_conf.policies_table[policy_index].application_type = GUEST_VOICE;
  } else if (has_guest_voice_signaling) {
    lldp_conf.policies_table[policy_index].application_type = GUEST_VOICE_SIGNALING;
  } else if (has_softphone_voice) {
    lldp_conf.policies_table[policy_index].application_type = SOFTPHONE_VOICE;
  } else if (has_video_conferencing) {
    lldp_conf.policies_table[policy_index].application_type = VIDEO_CONFERENCING;
  } else if (has_streaming_video) {
    lldp_conf.policies_table[policy_index].application_type = STREAMING_VIDEO;
  } else if (has_video_signaling) {
    lldp_conf.policies_table[policy_index].application_type = VIDEO_SIGNALING;
  }

  lldp_conf.policies_table[policy_index].tagged_flag = has_tagged;

  if (has_tagged) {
    if (v_vlan_id == 0) {
      ICLI_PRINTF("VLAN id can't be 0 when tagged.\n");
      update_conf = FALSE;
    }
  }

  T_IG(TRACE_GRP_CLI, "vlan_id:%u has_tagged:%d", v_vlan_id, has_tagged);

  lldp_conf.policies_table[policy_index].vlan_id = v_vlan_id;
  lldp_conf.policies_table[policy_index].l2_priority = v_0_to_7;
  lldp_conf.policies_table[policy_index].dscp_value = v_0_to_63;

  if (update_conf) {
    //Set current configuration
    VTSS_ICLI_ERR_PRINT(lldp_mgmt_set_common_config(&lldp_conf));
  }
  return VTSS_RC_OK;
}

// See lldpmed_icli_functions.h
vtss_rc lldpmed_icli_media_vlan_policy_delete(i32 session_id, icli_unsigned_range_t *policies_list)
{
  u32 p_index;

  lldp_common_conf_t lldp_conf;
  lldp_mgmt_get_common_config(&lldp_conf); // Get current configuration

  if (policies_list == NULL) {
    T_E("policies_list should never be NULL at this point");
    return VTSS_RC_ERROR;
  }

  //Loop through all the policies
  for (p_index = 0; p_index < policies_list->cnt; p_index++) {
    u32 policy_index;
    for (policy_index = policies_list->range[p_index].min; policy_index <= policies_list->range[p_index].max; policy_index++) {
      lldp_conf.policies_table[policy_index].in_use = FALSE;
    }
  }
  VTSS_ICLI_ERR_PRINT(lldp_mgmt_set_common_config(&lldp_conf));
  return VTSS_RC_OK;
}

// See lldpmed_icli_functions.h
vtss_rc lldpmed_icli_transmit_tlv(i32 session_id, icli_stack_port_range_t *plist, BOOL has_capabilities, BOOL has_location, BOOL has_network_policy, BOOL no)
{
  VTSS_ICLI_ERR_PRINT(lldpmed_icli_transmit_tlv_set(plist, has_capabilities, has_location, has_network_policy, no));
  return VTSS_RC_OK;
}

//
// ICFG (Show running)
//
#ifdef VTSS_SW_OPTION_ICFG

// help function for getting Ca type as printable keyword
//
// In : ca_type - CA TYPE as integer
//
// In/Out: key_word - Pointer to the string.
//
static void lldpmed_catype2keyword(lldpmed_catype_t ca_type, char *key_word)
{
  // Table in ANNEX B, TIA1057
  strcpy(key_word, "");
  switch (ca_type) {
  case LLDPMED_CATYPE_A1:
    strcat(key_word, "state");
    break;
  case LLDPMED_CATYPE_A2:
    strcat(key_word, "county");
    break;
  case LLDPMED_CATYPE_A3:
    strcat(key_word, "city");
    break;
  case LLDPMED_CATYPE_A4:
    strcat(key_word, "district");
    break;
  case LLDPMED_CATYPE_A5:
    strcat(key_word, "block");
    break;
  case LLDPMED_CATYPE_A6:
    strcat(key_word, "street");
    break;
  case LLDPMED_CATYPE_PRD:
    strcat(key_word, "leading-street-direction");
    break;
  case LLDPMED_CATYPE_POD:
    strcat(key_word, "trailing-street-suffix");
    break;
  case LLDPMED_CATYPE_STS:
    strcat(key_word, "street-suffix");
    break;
  case LLDPMED_CATYPE_HNO:
    strcat(key_word, "house-no");
    break;
  case LLDPMED_CATYPE_HNS:
    strcat(key_word, "house-no-suffix");
    break;
  case LLDPMED_CATYPE_LMK:
    strcat(key_word, "landmark");
    break;
  case LLDPMED_CATYPE_LOC:
    strcat(key_word, "additional-info");
    break;
  case LLDPMED_CATYPE_NAM:
    strcat(key_word, "name");
    break;
  case LLDPMED_CATYPE_ZIP:
    strcat(key_word, "zip-code");
    break;
  case LLDPMED_CATYPE_BUILD:
    strcat(key_word, "building");
    break;
  case LLDPMED_CATYPE_UNIT:
    strcat(key_word, "apartment");
    break;
  case LLDPMED_CATYPE_FLR:
    strcat(key_word, "floor");
    break;
  case LLDPMED_CATYPE_ROOM:
    strcat(key_word, "room-number");
    break;
  case LLDPMED_CATYPE_PLACE:
    strcat(key_word, "place-type");
    break;
  case LLDPMED_CATYPE_PCN:
    strcat(key_word, "postal-community-name");
    break;
  case LLDPMED_CATYPE_POBOX:
    strcat(key_word, "p-o-box");
    break;
  case LLDPMED_CATYPE_ADD_CODE:
    strcat(key_word, "additional-code");
    break;
  default:
    break;
  }
}


// Help function for printing the civic address
//
// In : all_defaults - TRUE if user want include printing of default values.
//      ca_type      - CA type to print
//      lldp_conf    - Pointer to LLDP configuration.
// In/Out: result - Pointer for ICFG
static vtss_rc lldpmed_icfg_print_civic(BOOL all_defaults, lldpmed_catype_t ca_type, vtss_icfg_query_result_t *result, lldp_struc_0_t *lldp_conf)
{
  lldp_8_t key_word_str[CIVIC_CA_VALUE_LEN_MAX];
  BOOL civic_empty;

  lldpmed_catype2keyword(ca_type, &key_word_str[0]); // Converting ca type to a printable string
  u16 ptr = lldp_conf->location_info.civic.civic_str_ptr_array[lldpmed_catype2index(ca_type)];

  civic_empty = (strlen(&lldp_conf->location_info.civic.ca_value[ptr]) == 0);

  if (all_defaults ||
      (!civic_empty)) {

    VTSS_RC(vtss_icfg_printf(result, "%slldp med location-tlv civic-addr %s %s%s%s\n",
                             civic_empty ? "no " : "",
                             key_word_str,
                             civic_empty ? "" : "\"",
                             civic_empty ? "" : &lldp_conf->location_info.civic.ca_value[ptr],
                             civic_empty ? "" : "\""));
  }
  return VTSS_RC_OK;
}


// Help function for print global configuration.
// help function for printing the mode.
// IN : lldp_conf - Pointer to the configuration.
//      iport - Port in question
//      all_defaults - TRUE if we shall be printing everything (else we are only printing non-default configurations).
//      result - To be used by vtss_icfg_printf
static vtss_rc lldpmed_icfg_print_global(lldp_struc_0_t *lldp_conf, vtss_port_no_t iport, BOOL all_defaults, vtss_icfg_query_result_t *result)
{
  i8 buf[25];

  // Altitude_
  if (all_defaults ||
      (lldp_conf->location_info.altitude_type != LLDPMED_ALTITUDE_TYPE_DEFAULT) ||
      (lldp_conf->location_info.altitude != LLDPMED_ALTITUDE_DEFAULT)) {

    mgmt_long2str_float(&buf[0], lldp_conf->location_info.altitude, TUDE_DIGIT);
    VTSS_RC(vtss_icfg_printf(result, "lldp med location-tlv altitude %s %s\n",
                             lldp_conf->location_info.altitude_type == METERS ? "meters" : "floors",
                             buf));
  }

  // Latitude_
  if (all_defaults ||
      (lldp_conf->location_info.latitude_dir != LLDPMED_LATITUDE_DIR_DEFAULT) ||
      (lldp_conf->location_info.latitude != LLDPMED_LATITUDE_DEFAULT)) {

    mgmt_long2str_float(&buf[0], lldp_conf->location_info.latitude, TUDE_DIGIT);
    VTSS_RC(vtss_icfg_printf(result, "lldp med location-tlv latitude %s %s\n",
                             lldp_conf->location_info.latitude_dir == SOUTH ? "south" : "north",
                             buf));
  }


  // Longitude_
  if (all_defaults ||
      (lldp_conf->location_info.longitude_dir != LLDPMED_LONGITUDE_DIR_DEFAULT) ||
      (lldp_conf->location_info.longitude != LLDPMED_LONGITUDE_DEFAULT)) {

    mgmt_long2str_float(&buf[0], lldp_conf->location_info.longitude, TUDE_DIGIT);
    VTSS_RC(vtss_icfg_printf(result, "lldp med location-tlv longitude %s %s\n",
                             lldp_conf->location_info.longitude_dir == EAST ? "east" : "west",
                             buf));
  }


  // Datum
  if (all_defaults ||
      lldp_conf->location_info.datum != LLDPMED_DATUM_DEFAULT) {

    VTSS_RC(vtss_icfg_printf(result, "%slldp med datum %s\n",
                             lldp_conf->location_info.datum == LLDPMED_DATUM_DEFAULT ? "no " : "",
                             lldp_conf->location_info.datum == WGS84 ? "wgs84" :
                             lldp_conf->location_info.datum == NAD83_NAVD88 ? "nad83-navd88" :
                             "nad83-mllw"));
  }

  // Fast start
  if (all_defaults ||
      lldp_conf->medFastStartRepeatCount != FAST_START_REPEAT_COUNT_DEFAULT) {

    if (lldp_conf->medFastStartRepeatCount == FAST_START_REPEAT_COUNT_DEFAULT) {
      VTSS_RC(vtss_icfg_printf(result, "no lldp med fast\n"));
    } else {
      VTSS_RC(vtss_icfg_printf(result, "lldp med fast %u\n", lldp_conf->medFastStartRepeatCount));
    }
  }

  // Civic address
  BOOL is_default = strlen(lldp_conf->location_info.ca_country_code) == 0;
  if (all_defaults || !is_default) {
    VTSS_RC(vtss_icfg_printf(result, "%slldp med location-tlv civic-addr country %s%s%s\n",
                             is_default ? "no " : "",
                             is_default ? "" : "\"",
                             is_default ? "" : lldp_conf->location_info.ca_country_code,
                             is_default ? "" : "\""));
  }


  VTSS_RC(lldpmed_icfg_print_civic(all_defaults, LLDPMED_CATYPE_A1, result, lldp_conf));
  VTSS_RC(lldpmed_icfg_print_civic(all_defaults, LLDPMED_CATYPE_A2, result, lldp_conf));
  VTSS_RC(lldpmed_icfg_print_civic(all_defaults, LLDPMED_CATYPE_A3, result, lldp_conf));
  VTSS_RC(lldpmed_icfg_print_civic(all_defaults, LLDPMED_CATYPE_A4, result, lldp_conf));
  VTSS_RC(lldpmed_icfg_print_civic(all_defaults, LLDPMED_CATYPE_A5, result, lldp_conf));
  VTSS_RC(lldpmed_icfg_print_civic(all_defaults, LLDPMED_CATYPE_A6, result, lldp_conf));
  VTSS_RC(lldpmed_icfg_print_civic(all_defaults, LLDPMED_CATYPE_PRD, result, lldp_conf));
  VTSS_RC(lldpmed_icfg_print_civic(all_defaults, LLDPMED_CATYPE_POD, result, lldp_conf));
  VTSS_RC(lldpmed_icfg_print_civic(all_defaults, LLDPMED_CATYPE_STS, result, lldp_conf));
  VTSS_RC(lldpmed_icfg_print_civic(all_defaults, LLDPMED_CATYPE_HNO, result, lldp_conf));
  VTSS_RC(lldpmed_icfg_print_civic(all_defaults, LLDPMED_CATYPE_HNS, result, lldp_conf));
  VTSS_RC(lldpmed_icfg_print_civic(all_defaults, LLDPMED_CATYPE_LMK, result, lldp_conf));
  VTSS_RC(lldpmed_icfg_print_civic(all_defaults, LLDPMED_CATYPE_LOC, result, lldp_conf));
  VTSS_RC(lldpmed_icfg_print_civic(all_defaults, LLDPMED_CATYPE_NAM, result, lldp_conf));
  VTSS_RC(lldpmed_icfg_print_civic(all_defaults, LLDPMED_CATYPE_ZIP, result, lldp_conf));
  VTSS_RC(lldpmed_icfg_print_civic(all_defaults, LLDPMED_CATYPE_BUILD, result, lldp_conf));
  VTSS_RC(lldpmed_icfg_print_civic(all_defaults, LLDPMED_CATYPE_UNIT, result, lldp_conf));
  VTSS_RC(lldpmed_icfg_print_civic(all_defaults, LLDPMED_CATYPE_FLR, result, lldp_conf));
  VTSS_RC(lldpmed_icfg_print_civic(all_defaults, LLDPMED_CATYPE_ROOM, result, lldp_conf));
  VTSS_RC(lldpmed_icfg_print_civic(all_defaults, LLDPMED_CATYPE_PLACE, result, lldp_conf));
  VTSS_RC(lldpmed_icfg_print_civic(all_defaults, LLDPMED_CATYPE_PCN, result, lldp_conf));
  VTSS_RC(lldpmed_icfg_print_civic(all_defaults, LLDPMED_CATYPE_POBOX, result, lldp_conf));
  VTSS_RC(lldpmed_icfg_print_civic(all_defaults, LLDPMED_CATYPE_ADD_CODE, result, lldp_conf));


  // Elin
  BOOL is_ecs_non_default = strlen(lldp_conf->location_info.ecs) != 0; // Default value for ecs is empty string
  if (all_defaults ||
      is_ecs_non_default) {
    VTSS_RC(vtss_icfg_printf(result, "%slldp med location-tlv elin-addr %s\n",
                             is_ecs_non_default ? "" : "no ",
                             lldp_conf->location_info.ecs));
  }

  //
  // Policies
  //
  const i32 POLICY_INDEX_NOT_FOUND = -1;
  // First print all no used policies (Default is "no policy", so only print if "print all default" is used)
  u32 policy_index;
  if (all_defaults) {
    BOOL print_command = TRUE;
    i32 start_policy = POLICY_INDEX_NOT_FOUND, end_policy = POLICY_INDEX_NOT_FOUND;
    for (policy_index = LLDPMED_POLICY_MIN ; policy_index <= LLDPMED_POLICY_MAX; policy_index++) {
      if (lldp_conf->policies_table[policy_index].in_use == FALSE) {
        if (start_policy == POLICY_INDEX_NOT_FOUND) {
          start_policy = policy_index;
        }
      } else {
        start_policy = POLICY_INDEX_NOT_FOUND;
        end_policy = POLICY_INDEX_NOT_FOUND;
      }

      if (policy_index != LLDPMED_POLICY_MAX) { // Make sure that we don't get out of bounds
        if (lldp_conf->policies_table[policy_index + 1].in_use) {
          end_policy = policy_index;
        }
      } else {
        end_policy = LLDPMED_POLICY_MAX;
      }


      if ((start_policy != POLICY_INDEX_NOT_FOUND) && (end_policy != POLICY_INDEX_NOT_FOUND)) {
        // Only print command the first time
        if (print_command) {
          VTSS_RC(vtss_icfg_printf(result, "no lldp med media-vlan-policy "));
          print_command = FALSE;
        } else {
          VTSS_RC(vtss_icfg_printf(result, ","));
        }
        if (start_policy == end_policy) {
          VTSS_RC(vtss_icfg_printf(result, "%d", start_policy));
        } else {
          VTSS_RC(vtss_icfg_printf(result, "%d-%d", start_policy, end_policy));
        }
      }
    } // End for

    if (!print_command) {
      VTSS_RC(vtss_icfg_printf(result, "\n"));
    }
  }

  // Print all policies defined
  for (policy_index = LLDPMED_POLICY_MIN ; policy_index <= LLDPMED_POLICY_MAX; policy_index++) {
    if (lldp_conf->policies_table[policy_index].in_use == TRUE) {

      VTSS_RC(vtss_icfg_printf(result, "lldp med media-vlan-policy %d", policy_index));

      switch (lldp_conf->policies_table[policy_index].application_type) {
      case VOICE:
        VTSS_RC(vtss_icfg_printf(result, " voice"));
        break;
      case VOICE_SIGNALING:
        VTSS_RC(vtss_icfg_printf(result, " voice-signaling"));
        break;
      case GUEST_VOICE:
        VTSS_RC(vtss_icfg_printf(result, " guest-voice"));
        break;
      case GUEST_VOICE_SIGNALING:
        VTSS_RC(vtss_icfg_printf(result, " guest-voice-signaling"));
        break;
      case SOFTPHONE_VOICE:
        VTSS_RC(vtss_icfg_printf(result, " softphone-voice"));
        break;
      case VIDEO_CONFERENCING:
        VTSS_RC(vtss_icfg_printf(result, " video-conferencing"));
        break;
      case STREAMING_VIDEO:
        VTSS_RC(vtss_icfg_printf(result, " streaming-video"));
        break;

      case VIDEO_SIGNALING:
        VTSS_RC(vtss_icfg_printf(result, " video-signaling"));
        break;
      }

      if (lldp_conf->policies_table[policy_index].tagged_flag) {
        VTSS_RC(vtss_icfg_printf(result, " tagged"));
        VTSS_RC(vtss_icfg_printf(result, " %d", lldp_conf->policies_table[policy_index].vlan_id));
      } else {
        VTSS_RC(vtss_icfg_printf(result, " untagged"));
      }



      VTSS_RC(vtss_icfg_printf(result, " l2-priority %d", lldp_conf->policies_table[policy_index].l2_priority));

      VTSS_RC(vtss_icfg_printf(result, " dscp %d\n", lldp_conf->policies_table[policy_index].dscp_value));
    }
  }

  return VTSS_RC_OK;
}

// Help function for determining if a port has any policies assigned.
// In : iport - port in question.
//      lldp_conf - LLDP configuration
// Return: TRUE if at least on policy is assigned to the port, else FALSE
static BOOL is_any_policies_assigned(vtss_port_no_t iport, const lldp_struc_0_t *lldp_conf)
{
  u32 policy;

  for (policy = LLDPMED_POLICY_MIN; policy <= LLDPMED_POLICY_MAX; policy++) {
    if (lldp_conf->ports_policies[iport][policy]) {
      T_DG_PORT(TRACE_GRP_CLI, iport, "lldp_conf.ports_policies[iport][%d]:0x%X", lldp_conf->ports_policies[iport][policy], policy);
      return TRUE; // At least one policy set
    }
  }

  return FALSE;
}

// Function called by ICFG.
static vtss_rc lldpmed_icfg_global_conf(const vtss_icfg_query_request_t *req,
                                        vtss_icfg_query_result_t *result)
{
  vtss_isid_t         isid;
  lldp_struc_0_t      lldp_conf;
  vtss_port_no_t      iport;

  switch (req->cmd_mode) {
  case ICLI_CMD_MODE_GLOBAL_CONFIG:
    // Get current configuration for this switch
    lldp_mgmt_get_config(&lldp_conf, VTSS_ISID_LOCAL);

    // Any port can be used
    VTSS_RC(lldpmed_icfg_print_global(&lldp_conf, 0, req->all_defaults, result));
    break;
  case ICLI_CMD_MODE_INTERFACE_PORT_LIST:
    isid = req->instance_id.port.isid;
    iport = req->instance_id.port.begin_iport;
    T_D("Isid:%d, iport:%u, req->instance_id.port.usid:%d", isid, iport, req->instance_id.port.usid);

    if (msg_switch_configurable(isid)) {
      // Get current configuration for this switch
      lldp_mgmt_get_config(&lldp_conf, isid);

      //
      // Optional TLVs
      //
      BOOL tlv_capabilities_disabled = (lldp_conf.lldpmed_optional_tlv[iport] & OPTIONAL_TLV_CAPABILITIES_BIT) == 0;
      BOOL tlv_location_disabled = (lldp_conf.lldpmed_optional_tlv[iport] & OPTIONAL_TLV_LOCATION_BIT) == 0;
      BOOL tlv_policy_disabled = (lldp_conf.lldpmed_optional_tlv[iport] & OPTIONAL_TLV_POLICY_BIT) == 0;

      BOOL at_least_one_tlv_is_disabled = tlv_capabilities_disabled || tlv_location_disabled || tlv_policy_disabled;
      BOOL at_least_one_tlv_is_enabled = !tlv_capabilities_disabled || !tlv_location_disabled || !tlv_policy_disabled;

      //Default is enabled
      if (at_least_one_tlv_is_enabled) {
        if (req->all_defaults) {
          VTSS_RC(vtss_icfg_printf(result, " lldp med transmit-tlv"));
          VTSS_RC(vtss_icfg_printf(result, "%s",  tlv_capabilities_disabled ? "" : " capabilities"));
          VTSS_RC(vtss_icfg_printf(result, "%s",  tlv_location_disabled ? "" : " location"));
          VTSS_RC(vtss_icfg_printf(result, "%s",  tlv_policy_disabled ? "" : " network-policy"));
          VTSS_RC(vtss_icfg_printf(result, "\n"));
        }
      }

      //Default is enabled, so if one of them is disable, we will show
      if (at_least_one_tlv_is_disabled) {
        VTSS_RC(vtss_icfg_printf(result, " no lldp med location-tlv"));
        VTSS_RC(vtss_icfg_printf(result, "%s",  tlv_capabilities_disabled ? " capabilities" : ""));
        VTSS_RC(vtss_icfg_printf(result, "%s",  tlv_location_disabled ? " location" : ""));
        VTSS_RC(vtss_icfg_printf(result, "%s",  tlv_policy_disabled ? " network-policy" : ""));
        VTSS_RC(vtss_icfg_printf(result, "\n"));
      }

      // Policies assigned to the port.
      i8 buf[150];
      if (req->all_defaults ||
          is_any_policies_assigned(iport, &lldp_conf)) {

        if (is_any_policies_assigned(iport, &lldp_conf)) {
          // Convert boolean list to printable string.
          (void) mgmt_non_portlist2txt(lldp_conf.ports_policies[iport], LLDPMED_POLICY_MIN, LLDPMED_POLICY_MAX, buf);
          VTSS_RC(vtss_icfg_printf(result, " lldp med media-vlan policy-list %s\n", buf));
        } else {
          VTSS_RC(vtss_icfg_printf(result, " no lldp med media-vlan policy-list\n"));
        }
      }
      break;

    default:
      //Not needed for LLDP
      break;
    }
  } // End msg_switch_configurable
  return VTSS_RC_OK;
}

/* ICFG Initialization function */
vtss_rc lldpmed_icfg_init(void)
{
  VTSS_RC(vtss_icfg_query_register(VTSS_ICFG_LLDPMED_GLOBAL_CONF, "lldp", lldpmed_icfg_global_conf));
  VTSS_RC(vtss_icfg_query_register(VTSS_ICFG_LLDPMED_PORT_CONF, "lldp", lldpmed_icfg_global_conf));
  return VTSS_RC_OK;
}
#endif // VTSS_SW_OPTION_ICFG

#endif // #ifdef VTSS_SW_OPTION_LLDP_MED
/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
