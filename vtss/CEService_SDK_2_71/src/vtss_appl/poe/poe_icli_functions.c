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
#ifdef VTSS_SW_OPTION_POE

#include "poe_api.h"
#include "poe.h" // For trace
#include "poe_custom_api.h"

#include "icli_api.h"
#include "icli_porting_util.h"

#include "icfg_api.h" // For vtss_icfg_query_request_t
#include "misc_api.h" // for uport2iport
#include "msg_api.h" // For msg_switch_exists
#include "mgmt_api.h" //mgmt_str_float2long
#include "topo_api.h"   //topo_usid2isid(), topo_isid2usid()

/***************************************************************************/
/*  Type defines                                                           */
/***************************************************************************/
// Type used for selecting which poe icli configuration to update for the common function.
typedef enum {VTSS_POE_ICLI_MODE,
              VTSS_POE_ICLI_PRIORITY,
              VTSS_POE_ICLI_POWER_LIMIT
             } vtss_poe_icli_conf_t; // Which configuration do to

// Used to passed the PoE configuration value for the corresponding configuration type.
typedef union {
  // Mode
  struct {
    BOOL poe;
    BOOL poe_plus;
  } mode;

  // Priority
  struct {
    BOOL low;
    BOOL high;
    BOOL critical;
  } priority;

  // Power limit
  char *power_limit_value;
} poe_conf_value_t;

/***************************************************************************/
/*  Internal functions                                                     */
/****************************************************************************/
// Function for checking is if PoE is supported for a specific port. If PoE isn't supported for the port, a printout is done.
// In : session_id - session_id of ICLI_PRINTF
//      iport - Internal port
//      uport - User port number
//      isid  - Internal switch id.
// Return: TRUE if PoE chipset is found for the iport, else FALSE
static BOOL is_poe_supported(i32 session_id, vtss_port_no_t iport, vtss_port_no_t uport, vtss_usid_t usid, poe_chipset_t *poe_chip_found)
{
  if (poe_chip_found[iport] == NO_POE_CHIPSET_FOUND) {
    icli_print_port_info_txt(session_id, usid, uport); // Print interface
    ICLI_PRINTF("does not have PoE support\n");
    return FALSE;
  } else {
    T_DG_PORT(VTSS_TRACE_GRP_ICLI, iport, "PoE supported, %d", poe_chip_found[iport]);
    return TRUE;
  }
}

// Help function for printing out status
// In : session_id - Session_Id for ICLI_PRINTF
//      debug  - Set to TRUE in order to get more PoE information printed
//      has_interface - TRUE if user has specified a specific interface
//      list - List of interfaces (ports)
static void poe_status(i32 session_id, const switch_iter_t *sit, BOOL debug, BOOL has_interface, icli_stack_port_range_t *plist)
{
  port_iter_t        pit;
  poe_status_t       status;
  char               txt_string[50];
  char               class_string[10];
  i8                 buf[250];

  // Header
  sprintf(buf, "%-23s %-9s %-41s %-16s %-19s", "Interface", "PD Class", "Port Status", "Power Used [W]", "Current Used [mA]");
  icli_table_header(session_id, buf); // Print header

  poe_mgmt_get_status(sit->isid, &status); // Update the status fields

  poe_local_conf_t poe_local_conf;
  poe_mgmt_get_local_config(&poe_local_conf, sit->isid);

  poe_chipset_t poe_chip_found[VTSS_PORTS];
  poe_mgmt_is_chip_found(sit->isid, &poe_chip_found[0]); // Get a list with which PoE chip there is associated to the port.

  // Loop through all front ports
  if (icli_port_iter_init(&pit, sit->isid, PORT_ITER_FLAGS_FRONT | PORT_ITER_SORT_ORDER_IPORT) == VTSS_RC_OK) {
    while (icli_port_iter_getnext(&pit, plist)) {
      if (is_poe_supported(session_id, pit.iport, pit.uport, sit->usid, &poe_chip_found[0])) {
        (void) icli_port_info_txt(sit->usid, pit.uport, buf);  // Get interface as printable string
        ICLI_PRINTF("%-23s %-9s %-41s %-16s %-19d \n",
                    &buf[0], // interface
                    poe_class2str(&status, pit.iport, &class_string[0]),
                    poe_status2str(status.port_status[pit.iport], pit.iport, &poe_local_conf),
                    one_digi_float2str(status.power_used[pit.iport], &txt_string[0]),
                    status.current_used[pit.iport]);
      }
    }
  }
}

// Help function for setting poe mode
// In : has_poe - TRUE is PoE port power shall be poe mode
//      has_poe_plus - TRUE if PoE port power shall be poe+ mode.
//      iport - Port in question
//      no - TRUE if mode shall be set to default.
//      poe_local_conf - Pointer to the current configuration.
static void poe_icli_mode_conf(const poe_conf_value_t *poe_conf_value, vtss_port_no_t iport, BOOL no, poe_local_conf_t *poe_local_conf)
{
  T_DG_PORT(VTSS_TRACE_GRP_ICLI, iport, "poe:%d, poe_plus:%d, no:%d", poe_conf_value->mode.poe, poe_conf_value->mode.poe_plus, no);
  // Update mode
  if (poe_conf_value->mode.poe) {
    poe_local_conf->poe_mode[iport] = POE_MODE_POE;
  } else if (poe_conf_value->mode.poe_plus) {
    poe_local_conf->poe_mode[iport] = POE_MODE_POE_PLUS;
  } else if (no) {
    poe_local_conf->poe_mode[iport] = POE_MODE_DEFAULT;
  }
}

// Help function for setting power limit
// In : Session_Id - session_id for ICLI_PRINTF
//      iport - Internal port in question
//      uport - User port id
//      value_str - new value (as string)
//      no - TRUE if mode shall be set to default.
//      poe_local_conf - Pointer to the current configuration.

static void poe_icli_power_limit_conf(i32 session_id, const port_iter_t *pit, vtss_usid_t usid, i8 *original_value_str, poe_local_conf_t *poe_local_conf, BOOL no)
{
  u16 port_power;
  char txt_string[50];
  char interface_str[50];
  long value;

  (void) icli_port_info_txt(usid, pit->uport, interface_str);  // Get interface as printable string

  // Take a copy of the original_value_str value string, because the mgmt_str_float2long is modifying the string.
  i8 value_str[100];
  misc_strncpyz(value_str, original_value_str, 100);

  if (no) {
    value = POE_MAX_POWER_DEFAULT;
  } else {
    // Convert from string to long. We don't check for valid range, that is done later.
    if (mgmt_str_float2long(value_str, &value, 0, 2147483647, 1) != VTSS_RC_OK) {
      T_DG_PORT(VTSS_TRACE_GRP_ICLI, pit->iport, "usid:%d, original_value_str:%s, value_str:%s", usid, original_value_str, value_str);
      ICLI_PRINTF("\"%s\" is an invalid power limit value for %s\n", original_value_str, interface_str);
      return;
    }
  }

  T_D("Setting max port power to %ld, poe_max_power_mode_dependent = %d",
      value, poe_max_power_mode_dependent(pit->iport, poe_local_conf->poe_mode[pit->iport]));

  // check for valid range
  if (value > poe_max_power_mode_dependent(pit->iport, poe_local_conf->poe_mode[pit->iport])) {
    port_power = poe_max_power_mode_dependent(pit->iport, poe_local_conf->poe_mode[pit->iport]);
    mgmt_long2str_float(&txt_string[0], port_power, 1);
    ICLI_PRINTF("Maximum allowed power (for the current mode) for %s is limited to %s W\n", interface_str, txt_string);
  } else {
    port_power = value;
  }

  poe_local_conf->max_port_power[pit->iport] = port_power;
}

// Function for configuring PoE priority
// IN - has_low - TRUE is priority
static void poe_icli_priority_conf(const poe_conf_value_t *poe_conf_value, vtss_port_no_t iport, BOOL no, poe_local_conf_t *poe_local_conf)
{
  // Update mode
  if (poe_conf_value->priority.low) {
    poe_local_conf->priority[iport] = LOW;
  } else if (poe_conf_value->priority.high) {
    poe_local_conf->priority[iport] = HIGH;
  } else if (poe_conf_value->priority.critical) {
    poe_local_conf->priority[iport] = CRITICAL;
  } else if (no) {
    poe_local_conf->priority[iport] = POE_PRIORITY_DEFAULT;
  }
}

// Common function used for setting interface configurations.
// In : session_id - Session_Id for ICLI_PRINTF
//      poe_conf_type - Selecting which configuration to update.
//      poe_conf_value - New configuration value.
//      plist - List of interfaces to update
//      no - TRUE if user wants to set configuration to default
static void poe_icli_common(i32 session_id, vtss_poe_icli_conf_t poe_conf_type,
                            const poe_conf_value_t *poe_conf_value,
                            icli_stack_port_range_t *plist, BOOL no)
{
  poe_local_conf_t poe_local_conf;
  u8 isid_cnt;

  // Just making sure that we don't access NULL pointer.
  if (plist == NULL) {
    T_E("plist was unexpected NULL");
    return;
  }

  for (isid_cnt = 0; isid_cnt < plist->cnt; isid_cnt++) {
    // Get current configuration
    poe_mgmt_get_local_config(&poe_local_conf, plist->switch_range[isid_cnt].isid);

    poe_chipset_t poe_chip_found[VTSS_PORTS];
    poe_mgmt_is_chip_found(plist->switch_range[isid_cnt].isid, &poe_chip_found[0]); // Get a list with which PoE chip there is associated to the port.

    port_iter_t        pit;
    if (icli_port_iter_init(&pit, plist->switch_range[isid_cnt].isid, PORT_ITER_FLAGS_FRONT | PORT_ITER_SORT_ORDER_IPORT) == VTSS_RC_OK) {
      T_DG(VTSS_TRACE_GRP_ICLI, "isid:%d", plist->switch_range[isid_cnt].isid);
      while (icli_port_iter_getnext(&pit, plist)) {
        T_DG_PORT(VTSS_TRACE_GRP_ICLI, pit.iport, "poe_conf_type:%d", poe_conf_type);
        // Ignore if PoE isn't supported for this port.
        if (is_poe_supported(session_id, pit.iport, pit.uport, plist->switch_range[isid_cnt].usid, &poe_chip_found[0])) {
          switch  (poe_conf_type) {
          case VTSS_POE_ICLI_MODE:
            poe_icli_mode_conf(poe_conf_value, pit.iport, no, &poe_local_conf);
            break;
          case VTSS_POE_ICLI_PRIORITY:
            poe_icli_priority_conf(poe_conf_value, pit.iport, no, &poe_local_conf);
            break;

          case VTSS_POE_ICLI_POWER_LIMIT:
            poe_icli_power_limit_conf(session_id, &pit, plist->switch_range[isid_cnt].isid, poe_conf_value->power_limit_value, &poe_local_conf, no);
            break;

          default:
            T_E("Unknown poe_conf_type:%d", poe_conf_type);
          }
        }
      }
      // Set new configuration
      poe_mgmt_set_local_config(&poe_local_conf, plist->switch_range[isid_cnt].isid);
    }
  }
}

// Function for configuring PoE power supply
// IN - isid  - isid for the switch to configure
//      value - New power supply value
//      no    - TRUE to restore to default
static void poe_icli_power_supply_set(i32 session_id, vtss_isid_t isid, u32 value, BOOL no)
{

  // Get current configuration
  poe_local_conf_t poe_local_conf;

  poe_mgmt_get_local_config(&poe_local_conf, isid);

  if (no) {
    poe_local_conf.primary_power_supply = POE_POWER_SUPPLY_MAX;
  } else {
    poe_local_conf.primary_power_supply = value;
  }
  // Set new configuration
  poe_mgmt_set_local_config(&poe_local_conf, isid);
}

/***************************************************************************/
/*  Functions called by iCLI                                                */
/****************************************************************************/
// See poe_icli_functions.h
void poe_icli_power_supply(i32 session_id, icli_unsigned_range_t *usid_list, u32 value, BOOL no)
{
  u8 list_cnt;
  vtss_isid_t isid;
  u8 usid_list_cnt;
  // If user has no specified any switches then loop through them all
  if (usid_list == NULL) {
    usid_list_cnt = 1;
  } else {
    usid_list_cnt = usid_list->cnt;
  }

  for (list_cnt = 0; list_cnt < usid_list_cnt; list_cnt++) {
    u8 usid;
    u8 usid_min;
    u8 usid_max;

    // If user has no specified any switches then loop through them all
    if (usid_list == NULL) {
      usid_min = VTSS_USID_START;
      usid_max = VTSS_USID_END - 1;
    } else {
      usid_min = usid_list->range[0].min;
      usid_max = usid_list->range[0].max;
    }

    for (usid = usid_min; usid <= usid_max; usid++) {
      isid = topo_usid2isid(usid);
      if (VTSS_ISID_LEGAL(isid)) {
        if (msg_switch_configurable(isid)) {
          poe_icli_power_supply_set(session_id, isid, value, no);
        } else {
          ICLI_PRINTF("Switch id:%u is not configurable. Ignored\n", usid);
        }
      }
    }
  }
}

// See poe_icli_functions.h
void poe_icli_management_mode(i32 session_id, BOOL has_class_consumption, BOOL has_class_reserved_power, BOOL has_allocation_consumption, BOOL has_alllocation_reserved_power, BOOL has_lldp_consumption, BOOL has_lldp_reserved_power, BOOL no)
{
  poe_master_conf_t poe_master_conf;
  poe_mgmt_get_master_config(&poe_master_conf);

  if (has_class_consumption) {
    poe_master_conf.power_mgmt_mode = CLASS_CONSUMP;
  } else if (has_class_reserved_power) {
    poe_master_conf.power_mgmt_mode = CLASS_RESERVED;
  } else if (has_alllocation_reserved_power) {
    poe_master_conf.power_mgmt_mode = ALLOCATED_RESERVED;
  } else if (has_allocation_consumption) {
    poe_master_conf.power_mgmt_mode = ALLOCATED_CONSUMP;
  } else if (has_lldp_reserved_power) {
    poe_master_conf.power_mgmt_mode = LLDPMED_RESERVED;
  } else if (has_lldp_consumption) {
    poe_master_conf.power_mgmt_mode = LLDPMED_CONSUMP;
  }

  if (no) {
    poe_master_conf.power_mgmt_mode = POE_MGMT_MODE_DEFAULT;
  }

  poe_mgmt_set_master_config(&poe_master_conf);
}

// See poe_icli_functions.h
void poe_icli_priority(i32 session_id, BOOL has_low, BOOL has_high, BOOL has_critical, icli_stack_port_range_t *plist, BOOL no)
{
  poe_conf_value_t poe_conf_value;
  poe_conf_value.priority.low = has_low;
  poe_conf_value.priority.high = has_high;
  poe_conf_value.priority.critical = has_critical;

  poe_icli_common(session_id, VTSS_POE_ICLI_PRIORITY, &poe_conf_value, plist, no);
}

// See poe_icli_functions.h
void poe_icli_mode(i32 session_id, BOOL has_poe, BOOL has_poe_plus, icli_stack_port_range_t *plist, BOOL no)
{
  poe_conf_value_t poe_conf_value;
  poe_conf_value.mode.poe = has_poe;
  poe_conf_value.mode.poe_plus = has_poe_plus;

  T_DG(VTSS_TRACE_GRP_ICLI, "has_poe:%d, has_poe_plus:%d, no:%d", has_poe, has_poe_plus, no);
  poe_icli_common(session_id, VTSS_POE_ICLI_MODE, &poe_conf_value, plist, no);
}

// See poe_icli_functions.h
void poe_icli_power_limit(i32 session_id, char *value, icli_stack_port_range_t *plist, BOOL no)
{
  poe_conf_value_t poe_conf_value;
  poe_conf_value.power_limit_value = value;
  poe_icli_common(session_id, VTSS_POE_ICLI_POWER_LIMIT, &poe_conf_value, plist, no);
}

// See poe_icli_functions.h
void poe_icli_show(i32 session_id, BOOL has_interface, icli_stack_port_range_t *list)
{
  switch_iter_t sit;

  if (icli_switch_iter_init(&sit) != VTSS_RC_OK) {
    return;
  }
  while (icli_switch_iter_getnext(&sit, list)) {
    if (has_interface || msg_switch_exists(sit.isid)) {
      poe_status(session_id, &sit, FALSE, has_interface, list);
    }
  }
}

/***************************************************************************/
/* ICFG (Show running)                                                     */
/***************************************************************************/
#ifdef VTSS_SW_OPTION_ICFG

// Function called by ICFG.
static vtss_rc poe_icfg_global_conf(const vtss_icfg_query_request_t *req,
                                    vtss_icfg_query_result_t *result)
{
  vtss_port_no_t   iport;
  poe_local_conf_t poe_local_conf;

  switch (req->cmd_mode) {
  case ICLI_CMD_MODE_GLOBAL_CONFIG:
    poe_mgmt_get_local_config(&poe_local_conf, VTSS_ISID_LOCAL);

    //
    // Power management mode
    //
    if (req->all_defaults ||
        poe_local_conf.power_mgmt_mode != POE_MGMT_MODE_DEFAULT) {

      if (poe_local_conf.power_mgmt_mode == POE_MGMT_MODE_DEFAULT) {
        VTSS_RC(vtss_icfg_printf(result, "no poe management mode\n"));
      } else {
        VTSS_RC(vtss_icfg_printf(result, "poe management mode %s\n",
                                 poe_local_conf.power_mgmt_mode == CLASS_CONSUMP ? "class-consumption" :
                                 poe_local_conf.power_mgmt_mode == CLASS_RESERVED ? "class-reserved-power" :
                                 poe_local_conf.power_mgmt_mode == ALLOCATED_CONSUMP ? "allocation-consumption" :
                                 poe_local_conf.power_mgmt_mode == ALLOCATED_RESERVED ? "allocation-reserved-power" :
                                 poe_local_conf.power_mgmt_mode == LLDPMED_CONSUMP ? "lldp-consumption" :
                                 poe_local_conf.power_mgmt_mode == LLDPMED_RESERVED ? "lldp-reserved-power" :
                                 "Unknown PoE management mode"));
      }
    }

    //
    // Power supply
    //
#if defined(VTSS_SWITCH_STACKABLE) && VTSS_SWITCH_STACKABLE
    vtss_usid_t usid;
    vtss_isid_t isid;
    for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
      isid = topo_usid2isid(usid);
      if (VTSS_ISID_LEGAL(isid)) {
        if (msg_switch_configurable(isid)) {
          poe_mgmt_get_local_config(&poe_local_conf, isid);
          if (req->all_defaults ||
              poe_local_conf.primary_power_supply != POE_POWER_SUPPLY_MAX) {
            if (poe_local_conf.primary_power_supply == POE_POWER_SUPPLY_MAX) {
              VTSS_RC(vtss_icfg_printf(result, "no poe supply sid %u\n", usid));
            } else {
              VTSS_RC(vtss_icfg_printf(result, "poe supply sid %u %d\n", isid, poe_local_conf.primary_power_supply));
            }
          }
        }
      }
    }
#else
    if (req->all_defaults ||
        poe_local_conf.primary_power_supply != POE_POWER_SUPPLY_MAX) {

      if (poe_local_conf.primary_power_supply == POE_POWER_SUPPLY_MAX) {
        VTSS_RC(vtss_icfg_printf(result, "no poe supply\n"));
      } else {
        VTSS_RC(vtss_icfg_printf(result, "poe supply %d\n", poe_local_conf.primary_power_supply));
      }
    }
#endif
    break;
  //
  // Interface configurations
  //
  case ICLI_CMD_MODE_INTERFACE_PORT_LIST:
    iport = uport2iport(req->instance_id.port.begin_uport);

    if (msg_switch_configurable(req->instance_id.port.isid)) {
      // Get current configuration
      poe_mgmt_get_local_config(&poe_local_conf, req->instance_id.port.isid);

      //
      // Mode
      //
      if (req->all_defaults ||
          poe_local_conf.poe_mode[iport] != POE_MODE_DEFAULT) {
        if (poe_local_conf.poe_mode[iport] == POE_MODE_DEFAULT) {
          // No command
          VTSS_RC(vtss_icfg_printf(result, " no poe mode\n"));
        } else {
          // Normal command
          VTSS_RC(vtss_icfg_printf(result, " poe mode %s\n",
                                   poe_local_conf.poe_mode[iport] == POE_MODE_POE_PLUS ? "plus" :
                                   poe_local_conf.poe_mode[iport] == POE_MODE_POE ? "standard" :
                                   poe_local_conf.poe_mode[iport] == POE_MODE_POE_DISABLED ? "disabled" :
                                   "Unknown PoE mode"));
        }
      }

      //
      // Priority
      //
      if (req->all_defaults ||
          poe_local_conf.priority[iport] != POE_PRIORITY_DEFAULT) {

        if (poe_local_conf.priority[iport] == POE_PRIORITY_DEFAULT) {
          // No command
          VTSS_RC(vtss_icfg_printf(result, " no poe priority\n"));
        } else {
          // Normal command
          VTSS_RC(vtss_icfg_printf(result, " poe priority %s\n",
                                   poe_local_conf.priority[iport] == LOW ? "low" :
                                   poe_local_conf.priority[iport] == HIGH ? "high" :
                                   poe_local_conf.priority[iport] == CRITICAL ? "critical" :
                                   "Unknown PoE priority"));
        }
      }

      //
      // Maximum power
      //
      if (req->all_defaults ||
          poe_local_conf.max_port_power[iport] != POE_MAX_POWER_DEFAULT) {

        if (poe_local_conf.max_port_power[iport] == POE_MAX_POWER_DEFAULT) {
          // No command
          VTSS_RC(vtss_icfg_printf(result, " no poe power limit\n"));
        } else {
          // Normal command
          char txt_string[50];
          mgmt_long2str_float(&txt_string[0], poe_local_conf.max_port_power[iport], 1);
          VTSS_RC(vtss_icfg_printf(result, " poe power limit %s\n", txt_string));
        }
      }
    }
    break;
  default:
    //Not needed for PoE
    break;
  }

  return VTSS_RC_OK;
}


/* ICFG Initialization function */
vtss_rc poe_icfg_init(void)
{
  VTSS_RC(vtss_icfg_query_register(VTSS_ICFG_POE_GLOBAL_CONF, "poe", poe_icfg_global_conf));
  VTSS_RC(vtss_icfg_query_register(VTSS_ICFG_POE_PORT_CONF, "poe", poe_icfg_global_conf));
  return VTSS_RC_OK;
}

#endif // VTSS_SW_OPTION_ICFG
#endif //VTSS_SW_OPTION_POE
