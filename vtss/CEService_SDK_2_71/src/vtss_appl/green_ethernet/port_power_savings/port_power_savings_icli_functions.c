
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

#include "icli_api.h"
#include "icli_porting_util.h"

#include "cli.h"
#ifdef VTSS_SW_OPTION_EEE
#include "mgmt_api.h" // For mgmt_ulong2bool_list and mgmt_list2txt
#include "eee.h"
#include "eee_api.h"
#endif // #ifdef VTSS_SW_OPTION_EEE

#undef VTSS_TRACE_MODULE_ID // We use the port module trace in this file
#undef TRACE_GRP_CRIT       // We use the port module trace in this file
#undef TRACE_GRP_CNT        // We use the port module trace in this file
#include "port.h" // For trace

#include "msg_api.h"
#ifdef VTSS_SW_OPTION_ICFG
#include "icfg_api.h"
#endif

#include "topo_api.h"   //topo_usid2isid(), topo_isid2usid()

#ifdef VTSS_SW_OPTION_EEE
/***************************************************************************/
/* Internal Functions                                                      */
/***************************************************************************/
// Function for return "Yes" is a boolean is TRUE, else "No" is returned.
static char *yn_txt(BOOL val)
{
  return val ? "Yes" : "No";
}

// Function for printing EEE mode and urgent queues configuration
// In : session_id - Need for ICLI_PRINTF
//      sit   - Switch information
//      uport - User port number
//      iport - Internal port number
//      eee_switch_conf - EEE configuration.
//      eee_switch_state - EEE status state
static vtss_rc port_power_savings_eee_show(i32 session_id, const switch_iter_t *sit, vtss_port_no_t uport, vtss_port_no_t iport,
                                           eee_switch_conf_t *eee_switch_conf, eee_switch_state_t *eee_switch_state, BOOL print_header,
                                           BOOL eee, BOOL actiphy, BOOL perfectreach)
{
  port_status_t      port_status;
  i8 str[200];

  // Silently skip printing status for switch that is currently taken out the stack
  if (!msg_switch_exists(sit->isid)) {
    return VTSS_RC_OK;
  }

  VTSS_RC(port_mgmt_status_get(sit->isid, iport, &port_status));

  // Print out table header
  if (print_header) {
    (void) snprintf(&str[0], sizeof(str), "%-23s %-4s", "Interface",  "Lnk");
    if (actiphy) {
      (void) snprintf(&str[0], sizeof(str), "%s %-14s", str, "Energy-detect");
    }

    if (perfectreach) {
      (void) snprintf(&str[0], sizeof(str), "%s %-12s", str, "Short-Reach");
    }

    if (eee) {
      (void) snprintf(&str[0], sizeof(str), "%s %-12s %-12s %-16s %-12s", str, "EEE Capable", "EEE Enabled", "LP EEE Capable", "In Power Save");
    }

    icli_table_header(session_id, str);
  }

  ICLI_PRINTF("%-23s %-4s",
              icli_port_info_txt(sit->usid, uport, &str[0]),
              yn_txt(eee_switch_state->port[iport].link));

  if (actiphy) {
    ICLI_PRINTF(" %-14s", port_status.power.actiphy_capable ? yn_txt(port_status.power.actiphy_power_savings) : "N/A");
  }

  if (perfectreach) {
    ICLI_PRINTF(" %-12s", port_status.power.perfectreach_capable ? yn_txt(port_status.power.perfectreach_power_savings) : "N/A");
  }

  if (eee) {
    ICLI_PRINTF(" %-12s %-12s %-16s %-12s",
                yn_txt(eee_switch_state->port[iport].eee_capable),
                eee_switch_state->port[iport].eee_capable ? yn_txt(eee_switch_conf->port[iport].eee_ena) : "N/A",
                eee_switch_state->port[iport].eee_capable ? yn_txt(eee_switch_state->port[iport].link_partner_eee_capable) : "N/A",
                eee_switch_state->port[iport].eee_capable ? yn_txt(eee_switch_state->port[iport].rx_in_power_save_state || eee_switch_state->port[iport].tx_in_power_save_state) : "N/A");
  }
  ICLI_PRINTF("\n");
  return VTSS_RC_OK;
}

// Function for setting EEE urgent queues configuration
// In : iport - Internal port number
//      eee_switch_conf - EEE configuration.
//      eee_switch_state - EEE status state
//      no  - TRUE to disable EEE
//      interface - TRUE if user has specified a specific interface port
//      urgent_uq_list - List with queues that are urgent

#if EEE_FAST_QUEUES_CNT > 0
// See port_power_savings_icli_functions.h
static void port_power_savings_eee_urgent_queues(i32 session_id, vtss_port_no_t iport, eee_switch_conf_t *eee_switch_conf,
                                                 eee_switch_state_t *eee_switch_state, BOOL no, BOOL interface, icli_range_t *urgent_qu_list, const vtss_isid_t isid)
{
  u8 qu_index;
  u8 range_index;

  // Urgent_Qu_List can be NULL if the uses simply want to disable all urgent queues
  if (urgent_qu_list != NULL) {
    T_D("interface:%d,u.sr.cnt:%u", interface, urgent_qu_list->u.sr.cnt);

    // Loop through the urgent queues list
    for (qu_index = 0; qu_index < urgent_qu_list->u.sr.cnt; qu_index++) {
      T_D_PORT(iport,  "min:%d, max:%d, no%d", urgent_qu_list->u.sr.range[qu_index].min, urgent_qu_list->u.sr.range[qu_index].max, no);
      for (range_index = urgent_qu_list->u.sr.range[qu_index].min; range_index <= urgent_qu_list->u.sr.range[qu_index].max; range_index++) {

        // Make sure we don't get out of bounce
        if ((range_index < EEE_FAST_QUEUES_MIN) || (range_index > EEE_FAST_QUEUES_MAX)) {
          ICLI_PRINTF("%% Ignoring invalid queue:%d. Valid range is %d-%d\n", range_index, EEE_FAST_QUEUES_MIN, EEE_FAST_QUEUES_MAX);
          continue;
        }

        if (no) {// No command
          eee_switch_conf->port[iport].eee_fast_queues &= ~(u8)(1 << (range_index - EEE_FAST_QUEUES_MIN));
        } else {

          T_D_PORT(iport, "qu:%d, cap:%d, if:%d, range:%d",
                   eee_switch_conf->port[iport].eee_fast_queues, eee_switch_state->port[iport].eee_capable, interface, range_index);

          // Enable eee if the port is eee capable or if the user has specified a specific interface
          if (eee_switch_state->port[iport].eee_capable || interface || !msg_switch_exists(isid)) {
            eee_switch_conf->port[iport].eee_fast_queues |= (1 << (range_index - EEE_FAST_QUEUES_MIN));
          }
          T_D_PORT(iport, "qu:%d", eee_switch_conf->port[iport].eee_fast_queues);
        }
      }
    }
  } else {
    if (no) {
      eee_switch_conf->port[iport].eee_fast_queues = 0;
    }
  }
}
#endif // EEE_FAST_QUEUES_CNT
#endif // #ifdef VTSS_SW_OPTION_EEE

#ifdef VTSS_SW_OPTION_PHY_POWER_CONTROL
static vtss_rc port_power_savings_power(i32 session_id, const port_iter_t *pit, const switch_iter_t *sit, BOOL actiphy, BOOL no)
{
  i8             interface_str_buf[100];
  port_conf_t    port_conf;
  port_status_t  port_status;

  // Get port configuration
  VTSS_RC(port_mgmt_conf_get(sit->isid, pit->iport, &port_conf));

  // Getting greenEthernet status
  if (!msg_switch_exists(sit->isid)) {
    // If the switch is currently not in the stack, we will assume that the switch is green ethernet capable in order to let the stack be pre-configured (It will be checked when the switch is added to the stack)
    memset(&port_status, 0, sizeof(port_status));
    port_status.power.perfectreach_capable = TRUE;
    port_status.power.actiphy_capable      = TRUE;
  } else {
    VTSS_RC(port_mgmt_status_get(sit->isid, pit->iport, &port_status));
  }

  if (actiphy) {
    if (!port_status.power.actiphy_capable) {
      ICLI_PRINTF("%s is not energy detect capable. Skipping\n", icli_port_info_txt(sit->usid, pit->uport, &interface_str_buf[0]));
      return VTSS_RC_OK; // Return OK, because we have already printed notification to the user.
    }

    if (no) {
      port_conf.power_mode &= ~VTSS_PHY_POWER_ACTIPHY; // Clear the actiphy bit.
    } else {
      port_conf.power_mode |= VTSS_PHY_POWER_ACTIPHY; //  Set the actiphy bit.
    }
  } else {
    // Not Actiphy mean PerfectReach
    if (!port_status.power.perfectreach_capable) {
      ICLI_PRINTF("%s is not short reach capable. Skipping\n", icli_port_info_txt(sit->usid, pit->uport, &interface_str_buf[0]));
      return VTSS_RC_OK; // Return OK, because we have already printed notification to the user.
    }

    if (no) {
      port_conf.power_mode &= ~VTSS_PHY_POWER_DYNAMIC; // Clear the PerfectReach bit.
    } else {
      port_conf.power_mode |= VTSS_PHY_POWER_DYNAMIC; //  Set the PerfectReach bit.
    }
  }

  //
  // Do the configuration update
  //
  VTSS_RC(port_mgmt_conf_set(sit->isid, pit->iport, &port_conf));

  return VTSS_RC_OK;
}
#endif /* VTSS_SW_OPTION_PHY_POWER_CONTROL */

// Function return error code to the port_power_savings_common function. All parameter are the same as for the port_power_savings_common function.
static vtss_rc port_power_savings(i32 session_id, BOOL mode, BOOL urgent_queues, BOOL has_interface, BOOL actiphy, BOOL perfectreach,
                                  icli_stack_port_range_t *plist, icli_range_t *urgent_queues_list, BOOL no, BOOL show_eee,
                                  BOOL show_actiphy, BOOL show_perfect_reach)
{
#ifdef VTSS_SW_OPTION_EEE
  eee_switch_conf_t eee_switch_conf;
  eee_switch_state_t eee_switch_state;
  BOOL print_header = TRUE;
#endif
  switch_iter_t   sit;
  // Loop through all switches in stack
  VTSS_RC(icli_switch_iter_init(&sit));
  while (icli_switch_iter_getnext(&sit, plist)) {
    port_iter_t pit;

    T_DG(TRACE_GRP_ICLI, "isid:%d", sit.isid);
#ifdef VTSS_SW_OPTION_EEE
    // Get configuration for the current switch
    VTSS_RC(eee_mgmt_switch_conf_get(sit.isid, &eee_switch_conf));

    // Get state for the current switch
    VTSS_RC(eee_mgmt_switch_state_get(sit.isid, &eee_switch_state));

#endif // VTSS_SW_OPTION_EEE
    // Loop through all ports
    VTSS_RC(icli_port_iter_init(&pit, sit.isid, PORT_ITER_FLAGS_FRONT | PORT_ITER_FLAGS_NPI));
    while (icli_port_iter_getnext(&pit, plist)) {

      // Call the function for the ICLI caller
#ifdef VTSS_SW_OPTION_EEE
      T_DG_PORT(TRACE_GRP_ICLI, pit.iport, "session_id:%u, usid:%d, no:%d, interface:%d, mode:%d, urgent_queues:%d",
                session_id, sit.usid, no, has_interface, mode, urgent_queues);

      if (show_eee || show_actiphy || show_perfect_reach) {
        VTSS_RC(port_power_savings_eee_show(session_id, &sit, pit.uport, pit.iport, &eee_switch_conf, &eee_switch_state, print_header, show_eee, show_actiphy, show_perfect_reach));
        print_header = FALSE; // Only print header once
      }

      if (mode) {
        T_DG(TRACE_GRP_ICLI, "eee_switch_state.port[%d].eee_capable:%d,  msg_switch_exists(%d):%d, no:%d", pit.iport, eee_switch_state.port[pit.iport].eee_capable, sit.isid, msg_switch_exists(sit.isid), no);
        if (!eee_switch_state.port[pit.iport].eee_capable && msg_switch_exists(sit.isid)) { // If the switch doesn't exist yet, we don't know if the port is EEE capable, so we apply the configuration anyway. If the port isn't EEE capable when the switch is added there will be thrown an error at that point in time.
          i8 interface_str_buf[100];
          ICLI_PRINTF("%s is not EEE capable. Skipping\n", icli_port_info_txt(sit.usid, pit.uport, &interface_str_buf[0]));
          T_IG(TRACE_GRP_ICLI, "EEE not capable, iport:%d", pit.uport);
        } else {
          if (no) {// No command
            eee_switch_conf.port[pit.iport].eee_ena = FALSE;
          } else {
            eee_switch_conf.port[pit.iport].eee_ena = TRUE;
          }
        }
      }

#if EEE_FAST_QUEUES_CNT > 0
      if (urgent_queues) {
        port_power_savings_eee_urgent_queues(session_id, pit.iport, &eee_switch_conf, &eee_switch_state, no, has_interface, urgent_queues_list, sit.isid);
      }
#endif

#endif // VTSS_SW_OPTION_EEE
#ifdef VTSS_SW_OPTION_PHY_POWER_CONTROL
      if (actiphy) {
        VTSS_RC(port_power_savings_power(session_id, &pit, &sit, TRUE, no));
      }

      if (perfectreach) {
        VTSS_RC(port_power_savings_power(session_id, &pit, &sit, FALSE, no));
      }
#endif //#ifdef VTSS_SW_OPTION_PHY_POWER_CONTROL

    }
#ifdef VTSS_SW_OPTION_EEE
    // If it is a set command the update the configuration
    if (mode || urgent_queues) {
      VTSS_RC(eee_mgmt_switch_conf_set(sit.isid, &eee_switch_conf));
    }
#endif // VTSS_SW_OPTION_EEE
  }
  return VTSS_RC_OK;
}
/***************************************************************************/
/* Functions called by iCLI                                                */
/***************************************************************************/
// Common function used for looping though all switches and ports in a stack
// In : session_id - Need for ICLI_PRINTF
//      show - TRUE if it is the ICLI show command that is calling the function// In : session_id - Need for ICLI_PRINTF
//      mode - TRUE if it is the ICLI enable/disable command that is calling the function// In : session_id - Need for ICLI_PRINTF
//      urgent_queues - TRUE if it is the ICLI urgent queues command that is calling the function// In : session_id - Need for ICLI_PRINTF
//      interface - TRUE if is a command is called with a specific interface
//      actiphy - TRUE is it actiphy that shall be set.
//      eee_switch_conf - EEE configuration.
//      eee_switch_state - EEE status state
//      no  - TRUE to disable EEE
vtss_rc port_power_savings_common(i32 session_id, BOOL mode, BOOL urgent_queues, BOOL has_interface, BOOL actiphy, BOOL perfectreach,
                                  icli_stack_port_range_t *plist, icli_range_t *urgent_queues_list, BOOL no, BOOL show_eee,
                                  BOOL show_actiphy, BOOL show_perfect_reach)
{
  VTSS_ICLI_ERR_PRINT(port_power_savings(session_id, mode, urgent_queues, has_interface, actiphy, perfectreach, plist, urgent_queues_list, no, show_eee, show_actiphy, show_perfect_reach));
  return VTSS_RC_OK;
}


#if defined(VTSS_SW_OPTION_EEE) && EEE_OPTIMIZE_SUPPORT == 1
// Function for setting the optimize for power or latency.
// In : optimized_for_power - TRUE to optimized for power else optimize for latency
vtss_rc port_power_savings_eee_optimize_for_power(BOOL optimize_for_power)
{
  eee_switch_global_conf_t switch_global_conf;
  // Get current configuration
  VTSS_RC(eee_mgmt_switch_global_conf_get(&switch_global_conf));

  // Do the change
  switch_global_conf.optimized_for_power = optimize_for_power;

  // Write the new configuration
  VTSS_RC(eee_mgmt_switch_global_conf_set(&switch_global_conf));
  return VTSS_RC_OK;
}
#endif  // VTSS_SW_OPTION_EEE

/***************************************************************************/
/* ICFG (Show running) functions                                           */
/***************************************************************************/
#ifdef VTSS_SW_OPTION_ICFG
/* ICFG callback functions */
static vtss_rc port_power_savings_global_conf(const vtss_icfg_query_request_t *req,
                                              vtss_icfg_query_result_t *result)
{
  vtss_port_no_t iport;
  vtss_isid_t isid;

  T_NG(TRACE_GRP_ICLI, "req->cmd_mode:%d", req->cmd_mode);
  switch (req->cmd_mode) {
  case ICLI_CMD_MODE_GLOBAL_CONFIG: {
#if defined(VTSS_SW_OPTION_EEE) && EEE_OPTIMIZE_SUPPORT == 1
    eee_switch_global_conf_t eee_switch_global_conf;
    VTSS_RC(eee_mgmt_switch_global_conf_get(&eee_switch_global_conf));

    // Print out if not default value (or if requested to print all)
    // Optimize for power
    if (req->all_defaults ||
        (eee_switch_global_conf.optimized_for_power != OPTIMIZE_FOR_POWER_DEFAULT)) {
      VTSS_RC(vtss_icfg_printf(result, "%sgreen-ethernet eee optimize-for-power\n", eee_switch_global_conf.optimized_for_power != OPTIMIZE_FOR_POWER_DEFAULT ? "" : "no "));
    }
#endif // VTSS_SW_OPTION_EEE
  }
  break;
  case ICLI_CMD_MODE_INTERFACE_PORT_LIST:
    isid = topo_usid2isid(req->instance_id.port.usid);
    iport = uport2iport(req->instance_id.port.begin_uport);

    T_DG_PORT(TRACE_GRP_ICLI, iport, "isid:%d", isid);
#ifdef VTSS_SW_OPTION_EEE
    eee_switch_conf_t eee_switch_conf;
    eee_switch_state_t eee_switch_state;

    // Get configuration for the current switch
    VTSS_RC(eee_mgmt_switch_conf_get(isid, &eee_switch_conf));
    VTSS_RC(eee_mgmt_switch_state_get(isid, &eee_switch_state));
    if (eee_switch_state.port[iport].eee_capable) {
      if (req->all_defaults ||
          (eee_switch_conf.port[iport].eee_ena != FALSE)) {

        VTSS_RC(vtss_icfg_printf(result, " %sgreen-ethernet eee\n", eee_switch_conf.port[iport].eee_ena ? "" : "no "));
      } else {
        T_DG(TRACE_GRP_ICLI, "Port:%d not EEE capable", iport);
      }

#if EEE_FAST_QUEUES_CNT > 0
      BOOL bool_list[32];
      char queue_buf[200];

      (void)mgmt_ulong2bool_list((ulong) eee_switch_conf.port[iport].eee_fast_queues, &bool_list[0]);
      (void)mgmt_list2txt(&bool_list[0], 0, EEE_FAST_QUEUES_CNT, &queue_buf[0]);

      if (req->all_defaults ||
          (eee_switch_conf.port[iport].eee_fast_queues != 0)) {
        VTSS_RC(vtss_icfg_printf(result, " %sgreen-ethernet eee urgent-queue %s\n",
                                 eee_switch_conf.port[iport].eee_fast_queues == 0 ? "no " : "",
                                 eee_switch_conf.port[iport].eee_fast_queues == 0 ? "" : queue_buf));
      }
#endif // EEE_FAST_QUEUES_CNT
    }
#endif // VTSS_SW_OPTION_EEE

    // PrefectReach and ActiPhy
#ifdef VTSS_SW_OPTION_PHY_POWER_CONTROL
    port_conf_t        port_conf;
    BOOL actiphy_conf_ena;
    BOOL perfect_reach_conf_ena;

    // Get port configuration
    VTSS_RC(port_mgmt_conf_get(isid, iport, &port_conf));

    actiphy_conf_ena = (port_conf.power_mode & VTSS_PHY_POWER_ACTIPHY) >> VTSS_PHY_POWER_ACTIPHY_BIT;
    perfect_reach_conf_ena  = (port_conf.power_mode & VTSS_PHY_POWER_DYNAMIC) >> VTSS_PHY_POWER_DYNAMIC_BIT;
    T_DG_PORT(TRACE_GRP_ICLI, iport, "actiphy_conf_ena:%d, perfect_reach_conf_ena:%d, port_conf.power_mode:%d, VTSS_PHY_POWER_ACTIPHY:%d",
              actiphy_conf_ena, perfect_reach_conf_ena, port_conf.power_mode, VTSS_PHY_POWER_ACTIPHY);

    if (req->all_defaults ||
        (actiphy_conf_ena)) {
      VTSS_RC(vtss_icfg_printf(result, " %sgreen-ethernet energy-detect\n", actiphy_conf_ena ? "" : "no "));
    }

    if (req->all_defaults ||
        (perfect_reach_conf_ena)) {
      VTSS_RC(vtss_icfg_printf(result, " %sgreen-ethernet short-reach\n", perfect_reach_conf_ena ? "" : "no "));
    }
#endif // VTSS_SW_OPTION_PHY_POWER_CONTROL


  default:
    break;
  }

  return VTSS_RC_OK;
}

/* ICFG Initialization function */
vtss_rc port_power_savings_icfg_init(void)
{
  VTSS_RC(vtss_icfg_query_register(VTSS_ICFG_PHY_POWER_CONTROL_PORT_CONF, "green-ethernet", port_power_savings_global_conf));
  VTSS_RC(vtss_icfg_query_register(VTSS_ICFG_PORT_INTERFACE_CONF,         "green-ethernet", port_power_savings_global_conf));
  return VTSS_RC_OK;
}
#endif // VTSS_SW_OPTION_ICFG
/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
