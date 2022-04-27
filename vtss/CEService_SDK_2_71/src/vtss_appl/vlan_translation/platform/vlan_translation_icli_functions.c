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
#include "icli_api.h"
#include "icli_porting_util.h"

#include "vlan_translation_api.h"
#ifdef VTSS_SW_OPTION_ICFG
#include "icfg_api.h"
#endif


/***************************************************************************/
/*  Internal types                                                         */
/****************************************************************************/


/***************************************************************************/
/*  Internal functions                                                     */
/****************************************************************************/

// Function mapping a list of VLAN to a VLAN translation for a given group
static vtss_rc vlan_translation_icli_map_vlan(u32 group, icli_unsigned_range_t *vlan_list, u32 translation_vlan, BOOL no)
{
  u16 list_index;
  // Loop through all the VLANs to translate
  for (list_index = 0; list_index < vlan_list->cnt; list_index++) {
    vtss_vid_t vid;
    for (vid = vlan_list->range[list_index].min; vid <= vlan_list->range[list_index].max; vid++) {
      if (no) { // Delete
        vtss_rc rc = vlan_trans_mgmt_grp2vlan_entry_delete(group, vid);

        // Don't give error message if user try to delete an entry which doesn't exist.
        if (rc == VT_ERROR_ENTRY_NOT_FOUND) {
          rc = VTSS_RC_OK;
        }

        VTSS_RC(rc);
      } else { // Add
        if (translation_vlan == vid) { // make no sense to map a vlan to itself
          return VT_ERROR_VLAN_SAME_AS_TRANSLATE_VLAN;
        }

        VTSS_RC(vlan_trans_mgmt_grp2vlan_entry_add(group, vid, translation_vlan));
      }
    }
  }
  return VTSS_RC_OK;
}

// Function for mapping port and groups
// In - group_id - Id gor the group to map
//    - plist - port information
//    - no - Set mapping to default.
static vtss_rc vlan_translation_icli_interface_map_vlan(u8 group_id, icli_stack_port_range_t *plist, BOOL no)
{

  BOOL  ports_to_configure[VTSS_PORT_ARRAY_SIZE];
  u8 pid; //port index

  // Set false as default
  for (pid = 0; pid < VTSS_PORTS; pid++) {
    ports_to_configure[pid] = FALSE;
  }


  switch_iter_t   sit; // In fact VLAN translation doesn't support stacking at the moment, this is for future use.
  VTSS_RC(switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID));
  while (icli_switch_iter_getnext(&sit, plist)) {
    // Loop through all the ports in the plist, and set the corresponding bit in the ports BOOL list is the port is part of the user's port list
    port_iter_t pit;
    VTSS_RC(icli_port_iter_init(&pit, sit.isid, PORT_ITER_FLAGS_FRONT));
    while (icli_port_iter_getnext(&pit, plist)) {
      ports_to_configure[pit.iport] = TRUE;
    }
  }

  if (no) {
    BOOL  ports2[VTSS_PORTS];
    for (pid = 0; pid < VTSS_PORTS; pid++) {

      // Skip  port that is no part of the list given by the user
      if (!ports_to_configure[pid]) {
        continue;
      }

      // No command is a little bit special because the default value depends upon the port number,
      // so we have to find the default value for each port-
      vlan_trans_port2grp_conf_t entry;
      entry_default_set(&entry, pid);

      // Then we must map the group and port for only the port in question.
      u8 pid2;
      for (pid2 = 0; pid2 < VTSS_PORTS; pid2++) {
        if (pid2 == pid) { // Finding the port in question
          ports2[pid2] = TRUE;
        } else {
          ports2[pid2] = FALSE;
        }
      }
      VTSS_RC(vlan_trans_mgmt_port2grp_entry_add(entry.group_id, &ports2[0]));
    }
  } else {
    // Straight forward map the port to the group
    VTSS_RC(vlan_trans_mgmt_port2grp_entry_add(group_id, &ports_to_configure[0]));
  }
  return VTSS_RC_OK;
}

/***************************************************************************/
/*  Functions called by iCLI                                                */
/****************************************************************************/

//  see vlan_translation_icli_functions.h
vtss_rc vlan_translation_icli_map(i32 session_id, u32 group, icli_unsigned_range_t *vlan_list, u32 translation_vlan, BOOL no)
{
  VTSS_ICLI_ERR_PRINT(vlan_translation_icli_map_vlan(group, vlan_list, translation_vlan, no));
  return VTSS_RC_OK;
}

//  see vlan_translation_icli_functions.h
vtss_rc vlan_translation_icli_interface_map(i32 session_id, u8 group_id, icli_stack_port_range_t *plist, BOOL no)
{
  VTSS_ICLI_ERR_PRINT(vlan_translation_icli_interface_map_vlan(group_id, plist, no));
  return VTSS_RC_OK;
}

//  see vlan_translation_icli_functions.h
BOOL vlan_translation_icli_runtime_groups(u32                session_id,
                                          icli_runtime_ask_t ask,
                                          icli_runtime_t     *runtime)
{
  switch (ask) {
  case ICLI_ASK_BYWORD:
    icli_sprintf(runtime->byword, "<group id : %u-%u>", 1, port_isid_port_count(VTSS_ISID_LOCAL));
    return TRUE;
  case ICLI_ASK_RANGE:
    runtime->range.type = ICLI_RANGE_TYPE_UNSIGNED;
    runtime->range.u.sr.cnt = 1;
    runtime->range.u.sr.range[0].min = 1;
    runtime->range.u.sr.range[0].max = port_isid_port_count(VTSS_ISID_LOCAL);
    return TRUE;
  default:
    break;
  }
  return FALSE;
}
/***************************************************************************/
/* ICFG callback functions */
/****************************************************************************/
#ifdef VTSS_SW_OPTION_ICFG
static vtss_rc vlan_translation_icfg_conf(const vtss_icfg_query_request_t *req,
                                          vtss_icfg_query_result_t *result)
{

  //  vtss_isid_t isid;
  vtss_port_no_t iport;
  //BOOL is_default;
  vtss_icfg_conf_print_t conf_print;
  vtss_icfg_conf_print_init(&conf_print);

  switch (req->cmd_mode) {
  case ICLI_CMD_MODE_GLOBAL_CONFIG: {
    // Get current configuration for this switch
    vlan_trans_mgmt_grp2vlan_conf_t     entry;


    //
    // VLAN mapping
    //
    u8 group_id;
    for (group_id = 1; group_id <= port_isid_port_count(VTSS_ISID_LOCAL); group_id++) {
      entry.group_id = VT_NULL_GROUP_ID;
      BOOL next = FALSE;

      BOOL default_list[VTSS_VIDS];

      u16 i;
      for (i = 0; i < VTSS_VIDS; i++) {
        default_list[i] = TRUE;
      }

      while (vlan_trans_mgmt_grp2vlan_entry_get(&entry, next) == VTSS_RC_OK) {
        next = TRUE;

        if (entry.vid == 0) { //
          //T_E("vid should never be 0, just making sure");
          continue;
        }

        if (entry.group_id != group_id) {
          continue;
        }

        default_list[entry.vid - 1] = FALSE;

        conf_print.is_default = FALSE;
        conf_print.bool_list = NULL;
        VTSS_RC(vtss_icfg_conf_print(req, result, conf_print, "switchport vlan mapping", "%u %u %u", entry.group_id, entry.vid, entry.trans_vid));
      }

      // Print out NO command
      conf_print.print_no_arguments = TRUE;
      conf_print.bool_list_max = VTSS_VIDS - 2;
      conf_print.bool_list_min = 0;
      conf_print.is_default = TRUE;
      conf_print.bool_list_in_front_of_parameter = FALSE;
      conf_print.bool_list = &default_list[0];
      VTSS_RC(vtss_icfg_conf_print(req, result, conf_print, "switchport vlan mapping", "%u", group_id));
    }
    break;
  }
  case ICLI_CMD_MODE_INTERFACE_PORT_LIST:
    //    isid =  req->instance_id.port.isid;
    iport = req->instance_id.port.begin_iport;
    vlan_trans_mgmt_port2grp_conf_t     conf;
    vlan_trans_port2grp_conf_t entry;

    BOOL next = FALSE;
    conf.group_id = VT_NULL_GROUP_ID;
    while (vlan_trans_mgmt_port2grp_entry_get(&conf, next) == VTSS_RC_OK) {
      next = TRUE;
      if (conf.group_id > port_isid_port_count(VTSS_ISID_LOCAL)) {
        continue;
      }
      if (conf.ports[iport]) {
        entry_default_set(&entry, iport);
        conf_print.is_default = entry.group_id == conf.group_id;
        VTSS_RC(vtss_icfg_conf_print(req, result, conf_print, "switchport vlan mapping", "%d", conf.group_id));
      }
    }
    break;
  default:
    break;
  }
  return VTSS_RC_OK;
}


/* ICFG Initialization function */
vtss_rc vlan_translation_icfg_init(void)
{
  VTSS_RC(vtss_icfg_query_register(VTSS_ICFG_VLAN_TRANSLATION_GLOBAL_CONF, "vlan", vlan_translation_icfg_conf));
  VTSS_RC(vtss_icfg_query_register(VTSS_ICFG_VLAN_TRANSLATION_INTERFACE_CONF, "vlan", vlan_translation_icfg_conf));
  return VTSS_RC_OK;
}

#endif // VTSS_SW_OPTION_ICFG
/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
