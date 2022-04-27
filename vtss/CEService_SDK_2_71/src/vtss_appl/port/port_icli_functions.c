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
#include "misc_api.h" // For e.g. vtss_uport_no_t
#include "msg_api.h"
#include "mgmt_api.h" // For mgmt_prio2txt
#ifdef VTSS_SW_OPTION_ICFG
#include "icfg_api.h"
#endif
#include "port.h" // For Trace
#include "port_api.h" // For vtss_port_conf_default

/***************************************************************************/
/*  Internal types                                                         */
/****************************************************************************/
// Enum for selecting which port configuration to perform.
typedef enum {
  MTU,               // Configuration of max frame length
  SHUT_DOWN,         // shut down of port
  FORCED_SPEED,      // Configuration of forced speed
  AUTONEG,           // Configuration of auto negotiation
  DUPLEX,            // Configuration of duplex mode
  MEDIA,             // Configuration of media type (cu/sfp)
  FLOW_CONTROL,      // Configuration of flow control
  EXCESSIVE_RESTART  // Configuration of excessive collisions.
} port_conf_type_t;


// Enum for selecting which port status to display.
typedef enum {
  STATISTICS,        // Displaying statistics
  CAPABILITIES,      // Displaying capabilities
  STATUS,            // Displaying status.
} port_status_type_t;

// Used to passed the port configuration to be changed.
typedef union {
  // Statistics options
  struct {
    BOOL has_clear; // Clear statistics
    BOOL has_packets;
    BOOL has_bytes;
    BOOL has_errors;
    BOOL has_discards;
    BOOL has_filtered;
    BOOL has_priority;
    BOOL has_up;
    BOOL has_down;
    icli_unsigned_range_t *priority_list;
  } statistics;

  // Used to passed the port capabilities to be shown.
  struct {
    BOOL sfp_detect; // Show SFP information read from detected SFP (via i2c)
  } port_capa_show;
} port_status_value_t;

// Used to passed the port status to be displayed.
typedef union {
  u16 max_length;
  vtss_port_speed_t speed;

  // Media type
  struct {
    BOOL rj45;
    BOOL sfp;
  } media_type;

  // Auto negotiation
  struct {
    BOOL autoneg;
    BOOL has_neg_10;
    BOOL has_neg_100;
    BOOL has_neg_1000;
  } autoneg_conf;

  // Duplex
  struct {
    BOOL has_full;
    BOOL has_half;
    BOOL has_auto;
    BOOL has_advertise_fdx;
    BOOL has_advertise_hdx;
  } duplex;

  struct {
    BOOL rx_enable;
    BOOL tx_enable;
  } flow_control;
} port_conf_value_t;

/***************************************************************************/
/*  Internal functions                                                     */
/****************************************************************************/
#ifdef VTSS_SW_OPTION_QOS
/* Print counters for priority queue */
static void port_icli_stat_priority(i32 session_id, vtss_usid_t usid, vtss_uport_no_t uport, BOOL first, const char *name,
                                    u64 c1, u64 c2)
{
  char buf[200];

  if (first) {
    sprintf(&buf[0], "%-43s Rx %-17s Tx %-17s", icli_port_info_txt(usid, uport, buf), name, name);
    icli_table_header(session_id, buf);
  } else {
    ICLI_PRINTF("%-43s %-20llu %-20llu\n", name, c1, c2);
  }
}

// Loop though priority queue and print statistics
static void port_icli_stat_priority_print(i32 session_id, port_status_value_t *status_value, const switch_iter_t *sit, const port_iter_t *pit, const vtss_port_counters_t *counters, BOOL print_header)
{
  /* Handle '<class>' specified */
  u8 i, idx;
  i8 prio_str_buf[40];

  if (print_header) {
    port_icli_stat_priority(session_id, sit->usid, pit->uport, TRUE, "Priority queue", 0, 0);
  }

  if (status_value->statistics.priority_list != NULL) {
    // User want some specific priorities
    for (i = 0; i < status_value->statistics.priority_list->cnt; i++) {
      for (idx = status_value->statistics.priority_list->range[i].min; idx <= status_value->statistics.priority_list->range[i].max; idx++) {
        strcpy(prio_str_buf, "Priority ");
        strncat(prio_str_buf, mgmt_prio2txt(idx, FALSE), 20);
        port_icli_stat_priority(session_id, sit->usid, pit->uport, FALSE, prio_str_buf,
                                counters->prop.rx_prio[idx - VTSS_PRIO_START],
                                counters->prop.tx_prio[idx - VTSS_PRIO_START]);
      }
    }
  } else {
    // User want all priorities
    for (idx = VTSS_PRIO_START; idx < VTSS_PRIO_END; idx++) {
      strcpy(prio_str_buf, "Priority ");
      strcat(prio_str_buf, mgmt_prio2txt(idx, FALSE));
      port_icli_stat_priority(session_id, sit->usid, pit->uport, FALSE, prio_str_buf,
                              counters->prop.rx_prio[idx - VTSS_PRIO_START],
                              counters->prop.tx_prio[idx - VTSS_PRIO_START]);
    }
  }
}
#endif

/* Print counters in two columns with header */
// IN : session_id - For ICLI printing
//      usid       - User switch id
//      uport      - User port id
//      first      - Set to TRUE to print header
//      name       - String with the counter name
//      tx         - Set to TRUE is it is a Tx counter, set to FALSE for both rx & tx counters
//      link       - Set to TRUE if the port has link up
//      has_down   - Set to TRUE to only print statistics for ports with link down
//      has_up     - Set to TRUE to only print statistics for ports with link up
//      c1         - RX counter value
//      c2         - TX counter value
static void cli_cmd_stat_port(i32 session_id, vtss_usid_t usid, vtss_uport_no_t uport, BOOL first, const char *name, BOOL tx, BOOL link, BOOL has_down, BOOL has_up,
                              u64 c1, u64 c2)
{
  char buf[ICLI_PORTING_STR_BUF_SIZE], *p;

  if (first) {
    p = &buf[0];
    p += sprintf(p, "%-23s Rx %-17s", "Interface", name);
    if (tx) {
      sprintf(p, "Tx %-17s", name);
    }

    icli_table_header(session_id, buf);
  }

  if ((has_down && link) ||
      (has_up && !link)) {
    T_IG(TRACE_GRP_ICLI, "has_up:%d, has_down:%d, link:%d", has_up, has_down, link);
    return;
  }

  ICLI_PRINTF("%-23s %-20llu", icli_port_info_txt(usid, uport, buf), c1); // 20 is 17 + the size of "Rx "
  if (tx) {
    ICLI_PRINTF("%-20llu", c2);
  }
  ICLI_PRINTF("\n");
}

// Help function for printing statistis
// IN - session_id - For iCLI printing
//      sit        - Switch information
//      pit        - Port information.
//      status_value - Information about what to print.
static vtss_rc icli_print_statistics(i32 session_id, const switch_iter_t *sit, const port_iter_t *pit, port_status_value_t *status_value)
{
  port_status_t        port_status;
  vtss_port_status_t   *status = &port_status.status;
  vtss_port_counters_t counters;

  VTSS_RC(port_mgmt_status_get_all(sit->isid, pit->iport, &port_status)); // Get status

  T_IG_PORT(TRACE_GRP_ICLI, pit->iport, "has_clear:%d", status_value->statistics.has_clear);
  /* Handle 'clear' command */
  if (status_value->statistics.has_clear) {
    VTSS_RC(port_mgmt_counters_clear(sit->isid, pit->iport));
    return VTSS_RC_OK;
  }

  /* Get counters for remaining commands */
  if (port_mgmt_counters_get(sit->isid, pit->iport, &counters) != VTSS_OK) {
    return VTSS_RC_OK;
  }

  /* Handle 'packet' command */
  if (status_value->statistics.has_packets) {
    cli_cmd_stat_port(session_id, sit->usid, pit->uport, pit->first, "Packets", 1, status->link, status_value->statistics.has_down, status_value->statistics.has_up, counters.rmon.rx_etherStatsPkts, counters.rmon.tx_etherStatsPkts);
    return VTSS_RC_OK;
  }

  /* Handle 'bytes' command */
  if (status_value->statistics.has_bytes) {
    cli_cmd_stat_port(session_id, sit->usid, pit->uport, pit->first, "Octets", 1, status->link, status_value->statistics.has_down, status_value->statistics.has_up, counters.rmon.rx_etherStatsOctets, counters.rmon.tx_etherStatsOctets);
    return VTSS_RC_OK;
  }

  /* Handle 'errors' command */
  if (status_value->statistics.has_errors) {
    cli_cmd_stat_port(session_id, sit->usid, pit->uport, pit->first, "Errors", 1, status->link, status_value->statistics.has_down, status_value->statistics.has_up, counters.if_group.ifInErrors, counters.if_group.ifOutErrors);
    return VTSS_RC_OK;
  }

  /* Handle 'discards' command */
  if (status_value->statistics.has_discards) {
    cli_cmd_stat_port(session_id, sit->usid, pit->uport, pit->first, "Discards", 1, status->link, status_value->statistics.has_down, status_value->statistics.has_up, counters.if_group.ifInDiscards, counters.if_group.ifOutDiscards);
    return VTSS_RC_OK;
  }

  /* Handle 'filtered' command */
  if (status_value->statistics.has_filtered) {
    cli_cmd_stat_port(session_id, sit->usid, pit->uport, pit->first, "Filtered", 0, status->link, status_value->statistics.has_down, status_value->statistics.has_up, counters.bridge.dot1dTpPortInDiscards, 0);
    return VTSS_RC_OK;
  }

#ifdef VTSS_SW_OPTION_QOS
  if (status_value->statistics.has_priority) {
    port_icli_stat_priority_print(session_id, status_value, sit, pit, &counters, TRUE);
    ICLI_PRINTF("\n");
    return VTSS_RC_OK;
  }
#endif /* VTSS_SW_OPTION_QOS */

  // Print out interface
  ICLI_PRINTF("\n");
  icli_print_port_info_txt(session_id, sit->usid, pit->uport);
  ICLI_PRINTF("Statistics:\n");

  /* Handle default command */
  icli_cmd_stati(session_id, "Packets", "",
                 counters.rmon.rx_etherStatsPkts,
                 counters.rmon.tx_etherStatsPkts);
  icli_cmd_stati(session_id, "Octets", "",
                 counters.rmon.rx_etherStatsOctets,
                 counters.rmon.tx_etherStatsOctets);
  icli_cmd_stati(session_id, "Unicast", "",
                 counters.if_group.ifInUcastPkts,
                 counters.if_group.ifOutUcastPkts);
  icli_cmd_stati(session_id, "Multicast", "",
                 counters.rmon.rx_etherStatsMulticastPkts,
                 counters.rmon.tx_etherStatsMulticastPkts);
  icli_cmd_stati(session_id, "Broadcast", "",
                 counters.rmon.rx_etherStatsBroadcastPkts,
                 counters.rmon.tx_etherStatsBroadcastPkts);
  icli_cmd_stati(session_id, "Pause", "",
                 counters.ethernet_like.dot3InPauseFrames,
                 counters.ethernet_like.dot3OutPauseFrames);
  ICLI_PRINTF("\n");
  icli_cmd_stati(session_id, "64", "",
                 counters.rmon.rx_etherStatsPkts64Octets,
                 counters.rmon.tx_etherStatsPkts64Octets);
  icli_cmd_stati(session_id, "65-127", "",
                 counters.rmon.rx_etherStatsPkts65to127Octets,
                 counters.rmon.tx_etherStatsPkts65to127Octets);
  icli_cmd_stati(session_id, "128-255", "",
                 counters.rmon.rx_etherStatsPkts128to255Octets,
                 counters.rmon.tx_etherStatsPkts128to255Octets);
  icli_cmd_stati(session_id, "256-511", "",
                 counters.rmon.rx_etherStatsPkts256to511Octets,
                 counters.rmon.tx_etherStatsPkts256to511Octets);
  icli_cmd_stati(session_id, "512-1023", "",
                 counters.rmon.rx_etherStatsPkts512to1023Octets,
                 counters.rmon.tx_etherStatsPkts512to1023Octets);
  icli_cmd_stati(session_id, "1024-1526", "",
                 counters.rmon.rx_etherStatsPkts1024to1518Octets,
                 counters.rmon.tx_etherStatsPkts1024to1518Octets);
  icli_cmd_stati(session_id, "1527-    ", "",
                 counters.rmon.rx_etherStatsPkts1519toMaxOctets,
                 counters.rmon.tx_etherStatsPkts1519toMaxOctets);
  ICLI_PRINTF("\n");

#ifdef VTSS_SW_OPTION_QOS
  i8 prio_str_buf[50];
  i8 idx;
  for (idx = VTSS_PRIO_START; idx < VTSS_PRIO_END; idx++) {
    strcpy(prio_str_buf, "Priority ");
    strncat(prio_str_buf, mgmt_prio2txt(idx, FALSE), 30);
    icli_cmd_stati(session_id, prio_str_buf, "",
                   counters.prop.rx_prio[idx - VTSS_PRIO_START],
                   counters.prop.tx_prio[idx - VTSS_PRIO_START]);
  }
  ICLI_PRINTF("\n");
#endif /* VTSS_SW_OPTION_QOS */


  icli_cmd_stati(session_id, "Drops", "",
                 counters.rmon.rx_etherStatsDropEvents,
                 counters.rmon.tx_etherStatsDropEvents);
  icli_cmd_stati(session_id, "CRC/Alignment", "Late/Exc. Coll.",
                 counters.rmon.rx_etherStatsCRCAlignErrors,
                 counters.if_group.ifOutErrors);
  icli_cmd_stati(session_id, "Undersize", NULL, counters.rmon.rx_etherStatsUndersizePkts, 0);
  icli_cmd_stati(session_id, "Oversize", NULL, counters.rmon.rx_etherStatsOversizePkts, 0);
  icli_cmd_stati(session_id, "Fragments", NULL, counters.rmon.rx_etherStatsFragments, 0);
  icli_cmd_stati(session_id, "Jabbers", NULL, counters.rmon.rx_etherStatsJabbers, 0);
  /* Bridge counters */
  icli_cmd_stati(session_id, "Filtered", NULL, counters.bridge.dot1dTpPortInDiscards, 0);
  return VTSS_RC_OK;
}

// Help function for printing port status.
// IN - session_id - For iCLI printing
//      sit        - Switch information
//      pit        - Port information-
static vtss_rc print_port_status(i32 session_id, const switch_iter_t *sit, const port_iter_t *pit)
{
  i8 buf[150];
  i8 *txt = buf;
  port_status_t   port_status;
  port_conf_t     port_conf;
  VTSS_RC(port_mgmt_conf_get(sit->isid, pit->iport, &port_conf));
  VTSS_RC(port_mgmt_status_get_all(sit->isid, pit->iport, &port_status)); // Get status
  port_isid_info_t       info;

  VTSS_RC(port_isid_info_get(sit->isid, &info));

  T_NG(TRACE_GRP_ICLI, "info:0x%X", info.cap);

  // Header
  if (pit->first) {
    txt +=  sprintf(txt, "%-23s %-8s %-15s ", "Interface", "Mode", "Speed & Duplex");
    if (info.cap & PORT_CAP_FLOW_CTRL) {
      txt += sprintf(txt, "%-13s", "Flow Control");
    }
    txt += sprintf(txt, " %-10s %-10s %-10s", "Max Frame", "Excessive", "Link");
    icli_table_header(session_id, buf);
  }

  // Print Interface and State
  ICLI_PRINTF("%-23s %-8s ", icli_port_info_txt(sit->usid, pit->uport, buf), icli_bool_txt(port_conf.enable));

  // Print mode
  BOOL no_fiber = (port_conf.dual_media_fiber_speed == VTSS_SPEED_FIBER_NOT_SUPPORTED_OR_DISABLED);
  if (no_fiber) {
    ICLI_PRINTF("%-15s ",
                port_conf.autoneg ? "Auto" : vtss_port_mgmt_mode_txt(pit->iport, port_conf.speed, port_conf.fdx, FALSE));
  } else {
    ICLI_PRINTF("%-15s ",
                port_fiber_mgmt_mode_txt(port_conf.dual_media_fiber_speed, port_conf.autoneg));
  }

  // Print flow control
  if (info.cap & PORT_CAP_FLOW_CTRL) {
    ICLI_PRINTF("%-13s", icli_bool_txt(port_conf.flow_control));
  }

  // Print Max frame size
  ICLI_PRINTF(" %-10u ", port_conf.max_length);

  // Print Excessive
  ICLI_PRINTF("%-10s ", port_conf.exc_col_cont ? "Restart" : "Discard");


  // Print Link
  port_vol_status_t  vol_status;
  if (port_status.status.link) {
    sprintf(buf, "%s",
            vtss_port_mgmt_mode_txt(pit->iport, port_status.status.speed, port_status.status.fdx, port_status.fiber));
  } else if (sit->isid != VTSS_ISID_LOCAL &&
             port_vol_status_get(PORT_USER_CNT, sit->isid, pit->iport,
                                 &vol_status) == VTSS_OK &&
             vol_status.conf.disable && vol_status.user != PORT_USER_STATIC) {
    sprintf(buf, "Down (%s)", vol_status.name);
  } else {
    strcpy(buf, "Down");
  }
  ICLI_PRINTF("%-10s ", buf);

  ICLI_PRINTF("\n");
  return VTSS_RC_OK;
}

/* Port status */
static vtss_rc icli_cmd_port_status(i32 session_id, icli_stack_port_range_t *plist, port_status_type_t status_type, port_status_value_t *status_value)
{
  switch_iter_t   sit;
  port_status_t   port_status;
  // Loop through all switches in stack
  VTSS_RC(switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID));
  while (icli_switch_iter_getnext(&sit, plist)) {
    // Loop through all ports
    port_iter_t pit;
    VTSS_RC(icli_port_iter_init(&pit, sit.isid, PORT_ITER_FLAGS_ALL));
    T_IG(TRACE_GRP_ICLI, "status_type:%d", status_type);
    while (icli_port_iter_getnext(&pit, plist)) {
      switch (status_type) {
      case STATISTICS:
        VTSS_RC(icli_print_statistics(session_id, &sit, &pit, status_value));
        break;
      case CAPABILITIES:
        VTSS_RC(port_mgmt_status_get_all(sit.isid, pit.iport, &port_status)); // Get status

        ICLI_PRINTF("\n");
        icli_print_port_info_txt(session_id, sit.usid, pit.uport);
        ICLI_PRINTF("Capabilities:\n");

        // Print out SFP information
        if (status_value->port_capa_show.sfp_detect) {
          ICLI_PRINTF("SFP Type: %s\n", sfp_if2txt(port_status.sfp.type));
          ICLI_PRINTF("SFP Vendor name: %s\n", port_status.sfp.vendor_name);
          ICLI_PRINTF("SFP Vendor PN: %s\n", port_status.sfp.vendor_pn);
          ICLI_PRINTF("SFP Vendor revision: %s\n", port_status.sfp.vendor_rev);
        }
        break;
      case STATUS:
        VTSS_RC(print_port_status(session_id, &sit, &pit));
        break;
      }
    }
  } /* USID loop */
  return VTSS_RC_OK;
}

//  see port_icli_functions.h
vtss_rc port_icli_statistics(i32 session_id, icli_stack_port_range_t *plist, BOOL has_packets, BOOL has_bytes, BOOL has_errors,
                             BOOL has_discards, BOOL has_filtered, BOOL has_priority, icli_unsigned_range_t *priority_list, BOOL has_up,
                             BOOL has_down)
{
  port_status_value_t port_status_value;

#if 1 /* CP, 08/27/2013 10:50, Bugzilla#12550 - Unable to view interface statistics in serval */
  memset(&port_status_value, 0, sizeof(port_status_value));
#endif

  port_status_value.statistics.has_packets   = has_packets;
  port_status_value.statistics.has_bytes     = has_bytes;
  port_status_value.statistics.has_errors    = has_errors;
  port_status_value.statistics.has_discards  = has_discards;
  port_status_value.statistics.has_filtered  = has_filtered;
  port_status_value.statistics.has_priority  = has_priority;
  port_status_value.statistics.has_up        = has_up;
  port_status_value.statistics.has_down      = has_down;
  port_status_value.statistics.priority_list = priority_list;
  T_IG(TRACE_GRP_ICLI, "has_priority:%d, has_packets:%d", has_priority, has_packets);
  VTSS_ICLI_ERR_PRINT(icli_cmd_port_status(session_id, plist,  STATISTICS, &port_status_value));
  return VTSS_RC_OK;
}

// help function for checking that the new configuration (and adjusting) is valid.
// In - session_id - For message printouts
//      port_conf - Pointer to the new port configuration
//      update_duplex - check and in case of mis-configuration overwrite duplex mode
//      update_fiber_speed - check and in case of mis-configuration overwrite fiber speed configuration
// return : TRUE if new port speed is valid, else FALSE
static BOOL port_icli_is_port_speed_valid(u32 session_id, port_conf_t *port_conf, BOOL update_duplex, BOOL update_fiber_speed, switch_iter_t sit, port_iter_t pit)
{
  // Check for half duplex - Speed / media type takes precedence
  T_IG(TRACE_GRP_ICLI, "fdx:%d, update_fiber_speed:%d, update_duplex:%d, speed:%d, fiber_speed:%d",
       port_conf->fdx, update_fiber_speed, update_duplex, port_conf->speed, port_conf->dual_media_fiber_speed);

  if (update_duplex) {
    if (!port_conf->fdx)
      if ((port_conf->speed != VTSS_SPEED_10M && port_conf->speed != VTSS_SPEED_100M) ||
          port_conf->dual_media_fiber_speed != VTSS_SPEED_FIBER_NOT_SUPPORTED_OR_DISABLED) {
        icli_print_port_info_txt(session_id, sit.usid, pit.uport); // Print interface
        ICLI_PRINTF("with current speed does not support half duplex, duplex changed to full duplex\n");
        port_conf->fdx = TRUE;
      }
  }

  // Checking for dual media and forced speed.
  if (port_conf->dual_media_fiber_speed == VTSS_SPEED_FIBER_AUTO && !port_conf->autoneg) {
    icli_print_port_info_txt(session_id, sit.usid, pit.uport); // Print interface
    ICLI_PRINTF("does not support dual media in forced speed (defaulting to SFP media)\n");
  }

  // Checking SFP configuration
  if (update_fiber_speed) {
    if (port_conf->dual_media_fiber_speed == VTSS_SPEED_FIBER_NOT_SUPPORTED_OR_DISABLED) {
      // Fiber currently disabled - Don't change
    } else {
      // Half duplex not supported for fiber interface
      if (!port_conf->fdx) {
        icli_print_port_info_txt(session_id, sit.usid, pit.uport); // Print interface
        ICLI_PRINTF(" does not support half duplex for SPF interface, duplex changed to full duplex");
        port_conf->fdx = TRUE;
      }

      if (port_conf->autoneg) {
        port_conf->dual_media_fiber_speed = VTSS_SPEED_FIBER_AUTO;
      } else if (port_conf->speed == VTSS_SPEED_100M) {
        port_conf->dual_media_fiber_speed = VTSS_SPEED_FIBER_100FX;
      } else  if (port_conf->speed == VTSS_SPEED_1G) {
        port_conf->dual_media_fiber_speed = VTSS_SPEED_FIBER_1000X;
      } else {
        icli_print_port_info_txt(session_id, sit.usid, pit.uport); // Print interface
        ICLI_PRINTF("does not supported the requested speed\n");
        return FALSE;
      }
    }
  }
  return TRUE;
}

// Help function for setting port configurations
// In : max_length - Max frame length (MTU)
//      plist - List of ports to configure
//      no - TRUE when the configuration parameter shall be set to default.
static vtss_rc port_icli_port_conf_set(i32 session_id, const port_conf_type_t port_conf_type, const port_conf_value_t *port_conf_new, icli_stack_port_range_t *plist, BOOL no)
{
  switch_iter_t sit;
  port_iter_t pit;
  vtss_rc rc               = VTSS_RC_OK;
  port_cap_t cap;
  vtss_rc port_setup_valid = TRUE;

  // Loop through all switches in a stack
  VTSS_RC(icli_switch_iter_init(&sit));
  while (icli_switch_iter_getnext(&sit, plist)) {
    T_IG(TRACE_GRP_ICLI, "isid:%d", sit.isid);
    // Loop though the ports
    VTSS_RC(icli_port_iter_init(&pit, sit.isid, PORT_ITER_FLAGS_FRONT));
    while (icli_port_iter_getnext(&pit, plist)) {
      port_conf_t port_conf;
      port_conf_t port_conf_def; // Used to get the default configuration settings
      port_isid_port_info_t info;

      VTSS_RC(port_isid_port_info_get(sit.isid, pit.iport, &info));

      // Get current configuration
      VTSS_RC(port_mgmt_conf_get(sit.isid, pit.iport, &port_conf));

      // Get default values for this interface
      (void)vtss_port_conf_default(sit.isid, pit.iport, &port_conf_def);

      switch (port_conf_type) {
      case MTU:
        // Setup max length (MTU)
        if (no) {
          port_conf.max_length = port_conf_def.max_length; // update MTU to default value
        } else {
          port_conf.max_length = port_conf_new->max_length; // update new MTU
        }
        break;

      case SHUT_DOWN:
        // Setup port enable/disable
        port_conf.enable = no; // No means "no shut down" = enable
        break;

      case MEDIA:
        cap = port_isid_port_cap(sit.isid, pit.iport); // get port capabilities
        // It only make sense to change media type for dual media port
        if (!(cap & PORT_CAP_SPEED_DUAL_ANY_FIBER)) {
          if (!no) {
            // Check if fiber is supported
            if (port_conf_new->media_type.sfp) {
              if (!(cap & PORT_CAP_FIBER)) {
                icli_print_port_info_txt(session_id, sit.usid, pit.uport); // Print interface
                ICLI_PRINTF("only supports rj45 media type. New media type configuration ignored.\n");
              }
            } else if (port_conf_new->media_type.rj45) {
              if (!(cap & PORT_CAP_COPPER)) {
                icli_print_port_info_txt(session_id, sit.usid, pit.uport); // Print interface
                ICLI_PRINTF("only supports SFP media type. New media type configuration ignored.\n");
              }
            }
          }
          break;
        }

        // Setup port media type
        if (no) {
          // Default is dual media
          port_conf.dual_media_fiber_speed = port_conf_def.dual_media_fiber_speed;
          port_conf.enable = port_conf_def.enable; // This is for CU ports
          port_conf.autoneg = TRUE;
        } else {
          // User setup to dual media. That is only supported in auto negotiation mode (for the cu interface).
          if (port_conf_new->media_type.sfp && port_conf_new->media_type.rj45) {
            port_conf.autoneg = TRUE;
          }

          // Setup dual media SFP speed
          if (port_conf_new->media_type.sfp) {
            port_conf.dual_media_fiber_speed = VTSS_SPEED_FIBER_AUTO; // Default to auto detect

            if (!port_conf.autoneg) { // Setup speed if user has selected forced speed
              switch (port_conf.speed) {
              case VTSS_SPEED_100M:
                port_conf.dual_media_fiber_speed = VTSS_SPEED_FIBER_100FX;
                break;
              case VTSS_SPEED_1G:
                port_conf.dual_media_fiber_speed = VTSS_SPEED_FIBER_1000X;
                break;
              default:
                port_conf.dual_media_fiber_speed = VTSS_SPEED_FIBER_AUTO; // Should never happen
                break;
              }
            }
          } else {
            // User has not selected SFP so disable
            port_conf.dual_media_fiber_speed = VTSS_SPEED_FIBER_NOT_SUPPORTED_OR_DISABLED;
          }
          T_IG_PORT(TRACE_GRP_ICLI, pit.iport, "media:%d, cap:0x%X, sfp_only:0x%X, dual_media_fiber_speed:%d, port_conf.speed:%d, port_conf_new->media_type.sfp:%d",
                    port_conf_new->media_type.rj45, cap, cap & PORT_CAP_SFP_ONLY, port_conf.dual_media_fiber_speed, port_conf.speed, port_conf_new->media_type.sfp);
        }

        port_setup_valid = port_icli_is_port_speed_valid(session_id, &port_conf, TRUE, FALSE, sit, pit);

        T_NG_PORT(TRACE_GRP_ICLI, pit.iport, "port_conf_type:%d, no:%d, port_conf_new->media_type.sfp:%d, port_conf_new->media_type.rj45:%d",
                  port_conf_type, no, port_conf_new->media_type.sfp, port_conf_new->media_type.rj45);
        break;
      case FORCED_SPEED:
        if (no) {
          port_conf.speed = port_conf_def.speed;
        } else {
          port_conf.speed = port_conf_new->speed;
        }

        // Forced_speed
        port_conf.autoneg = FALSE;

        port_setup_valid = port_icli_is_port_speed_valid(session_id, &port_conf, TRUE, TRUE, sit, pit);

        T_DG_PORT(TRACE_GRP_ICLI, pit.iport, "Auto:%d, speed:%d, no:%d", port_conf.autoneg, port_conf.speed, no);
        break;
      case AUTONEG:
        // Setup auto negotiation
        if (no) {
          port_conf.adv_dis &= ~PORT_ADV_DIS_1G_FDX; // Clear the bit
          port_conf.adv_dis &= ~PORT_ADV_DIS_100M; // Clear the bit
          port_conf.adv_dis &= ~PORT_ADV_DIS_10M; // Clear the bit

          port_conf.autoneg = port_conf_def.autoneg;
        } else {
          port_conf.autoneg = port_conf_new->autoneg_conf.autoneg;

          // Change the advertisement if user has set any of the advertisement settings
          if (port_conf_new->autoneg_conf.has_neg_1000 | port_conf_new->autoneg_conf.has_neg_100 | port_conf_new->autoneg_conf.has_neg_10) {
            port_conf.adv_dis |= PORT_ADV_DIS_1G_FDX;
            port_conf.adv_dis |= PORT_ADV_DIS_100M; //
            port_conf.adv_dis |= PORT_ADV_DIS_10M; //

            if (port_conf_new->autoneg_conf.has_neg_1000) {
              port_conf.adv_dis &= ~PORT_ADV_DIS_1G_FDX; // Clear the bit
            }

            if (port_conf_new->autoneg_conf.has_neg_100) {
              port_conf.adv_dis &= ~PORT_ADV_DIS_100M; // Clear the bit
            }

            if (port_conf_new->autoneg_conf.has_neg_10) {
              port_conf.adv_dis &= ~PORT_ADV_DIS_10M; // Clear the bit
            }
          } else {
            port_conf.adv_dis &= ~PORT_ADV_DIS_1G_FDX; // Clear the bit
            port_conf.adv_dis &= ~PORT_ADV_DIS_100M; // Clear the bit
            port_conf.adv_dis &= ~PORT_ADV_DIS_10M; // Clear the bit
          }
        }

        port_setup_valid = port_icli_is_port_speed_valid(session_id, &port_conf, TRUE, TRUE, sit, pit);
        break;

      case DUPLEX:
        if (no) {
          //Setup auto negotiation advertisement
          if (port_conf_new->duplex.has_advertise_fdx) {
            port_conf.adv_dis |= PORT_ADV_DIS_100M_FDX; //
            port_conf.adv_dis |= PORT_ADV_DIS_10M_FDX; //
          }

          if (port_conf_new->duplex.has_advertise_hdx) {
            port_conf.adv_dis |= PORT_ADV_DIS_100M_HDX; // Set the bit
            port_conf.adv_dis |= PORT_ADV_DIS_10M_HDX;  // Set the bit
          }
        } else {
          if (port_conf_new->duplex.has_full) {
            port_conf.fdx = TRUE; // Forced full duplex
          } else if (port_conf_new->duplex.has_half) {
            port_conf.fdx = FALSE; // Forced half duplex
          } else if (port_conf_new->duplex.has_auto) {
            if (port_conf.speed != VTSS_SPEED_10M && port_conf.speed != VTSS_SPEED_100M) {
              icli_print_port_info_txt(session_id, sit.usid, pit.uport); // Print interface
              ICLI_PRINTF("with current speed is restricted to full duplex\n");
              port_conf.fdx = TRUE; // Forced full duplex
            } else {
              //Setup auto negotiation advertisement
              if (port_conf_new->duplex.has_advertise_fdx) {
                port_conf.adv_dis &= ~PORT_ADV_DIS_100M_FDX; // Clear the bit
                port_conf.adv_dis &= ~PORT_ADV_DIS_10M_FDX; // Clear the bit
              }

              if (port_conf_new->duplex.has_advertise_hdx) {
                port_conf.adv_dis &= ~PORT_ADV_DIS_100M_HDX; // Clear the bit
                port_conf.adv_dis &= ~PORT_ADV_DIS_10M_HDX; // Clear the bit
              }
            }
          } else {
            T_E("Should never happen");
          }
        }

        port_setup_valid = port_icli_is_port_speed_valid(session_id, &port_conf, TRUE, TRUE, sit, pit);

        break;
      case FLOW_CONTROL:
        if (no) {
          port_conf.flow_control = port_conf_def.flow_control;
        } else {
          port_conf.flow_control = port_conf_new->flow_control.rx_enable | port_conf_new->flow_control.rx_enable;
        }

        break;
      case EXCESSIVE_RESTART:
        if (no) {
          port_conf.exc_col_cont = port_conf_def.exc_col_cont;
        } else {
          port_conf.exc_col_cont = TRUE;
        }
        break;
      }

      T_IG_PORT(TRACE_GRP_ICLI, pit.iport, "Auto:%d, speed:%d, no:%d, port_conf.adv_dis:0x%X", port_conf.autoneg, port_conf.speed, no, port_conf.adv_dis);
      //
      // Set the new configuration
      //

      // Give an error message if the parameters is not supported for the given interface
      if (vtss_port_mgmt_conf_set(sit.isid, pit.iport, &port_conf) == PORT_ERROR_PARM) {
        T_IG_PORT(TRACE_GRP_ICLI, pit.iport, "conf_set%d, port_setup_valid:0x%X", vtss_port_mgmt_conf_set(sit.isid, pit.iport, &port_conf), port_setup_valid);
        icli_print_port_info_txt(session_id, sit.usid, pit.uport);
        ICLI_PRINTF("does not support this mode\n");
        rc = VTSS_RC_OK; // No need to return error code, since we have already given an error message.
      }
    }
  }
  return rc;
}

/***************************************************************************/
/*  Functions called by iCLI                                                */
/****************************************************************************/
//  see port_icli_functions.h
BOOL port_icli_runtime_qos_queues(u32                session_id,
                                  icli_runtime_ask_t ask,
                                  icli_runtime_t     *runtime)
{
  switch (ask) {
  case ICLI_ASK_PRESENT:
#if defined(VTSS_SW_OPTION_QOS)
    runtime->present = TRUE;
#else
    runtime->present = FALSE;
#endif
    return TRUE;
  case ICLI_ASK_BYWORD:
    icli_sprintf(runtime->byword, "<queue : %u-%u>", VTSS_PRIO_START, VTSS_PRIO_END - 1);
    return TRUE;
  case ICLI_ASK_RANGE:
    runtime->range.type = ICLI_RANGE_TYPE_UNSIGNED;
    runtime->range.u.sr.cnt = 1;
    runtime->range.u.sr.range[0].min = VTSS_PRIO_START;
    runtime->range.u.sr.range[0].max = VTSS_PRIO_END - 1;
    return TRUE;
  case ICLI_ASK_HELP:
    icli_sprintf(runtime->help, "Queue number");
    return TRUE;
  default:
    break;
  }
  return FALSE;
}

//  see port_icli_functions.h
BOOL port_icli_runtime_10g(u32                session_id,
                           icli_runtime_ask_t ask,
                           icli_runtime_t     *runtime)
{
  switch (ask) {
  case ICLI_ASK_PRESENT:
    runtime->present =  port_stack_support_cap(PORT_CAP_10G_FDX);
    return TRUE;
  default:
    return FALSE;
  }
}

//  see port_icli_functions.h
BOOL port_icli_runtime_2g5(u32                session_id,
                           icli_runtime_ask_t ask,
                           icli_runtime_t     *runtime)
{
  switch (ask) {
  case ICLI_ASK_PRESENT:
    runtime->present =  port_stack_support_cap(PORT_CAP_2_5G_FDX);
    return TRUE;
  default:
    return FALSE;
  }
}

//  see port_icli_functions.h
vtss_rc port_icli_statistics_clear(i32 session_id, icli_stack_port_range_t *plist)
{
  port_status_value_t port_status_value;
  port_status_value.statistics.has_clear = TRUE;
  T_IG(TRACE_GRP_ICLI, "Enter");
  VTSS_ICLI_ERR_PRINT(icli_cmd_port_status(session_id, plist,  STATISTICS, &port_status_value));
  return VTSS_RC_OK;
}

//  see port_icli_functions.h
vtss_rc port_icli_capabilities(i32 session_id, icli_stack_port_range_t *plist)
{
  port_status_value_t port_status_value;
  port_status_value.port_capa_show.sfp_detect = TRUE;

  VTSS_ICLI_ERR_PRINT(icli_cmd_port_status(session_id, plist, CAPABILITIES, &port_status_value));
  return VTSS_RC_OK;
}

//  see port_icli_functions.h
vtss_rc icli_cmd_port_veriphy(i32 session_id, icli_stack_port_range_t *plist)
{
#ifdef VTSS_UI_OPT_VERIPHY
  port_isid_port_info_t info;
  port_veriphy_mode_t   mode[VTSS_PORT_ARRAY_SIZE];
  switch_iter_t         sit;
  BOOL                  print_hdr = TRUE;
  if (!msg_switch_is_master()) {
    return PORT_ERROR_MUST_BE_MASTER;
  }

  ICLI_PRINTF("Starting VeriPHY - Please wait\n");
  VTSS_RC(switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID));
  while (icli_switch_iter_getnext(&sit, plist)) {
    // Loop through all ports
    port_iter_t pit;
    VTSS_RC(icli_port_iter_init(&pit, sit.isid, PORT_ITER_FLAGS_FRONT | PORT_ITER_FLAGS_NPI));

    // Start by setting all port to NOT run veriphy
    while (icli_port_iter_getnext(&pit, NULL)) {
      mode[pit.iport] = PORT_VERIPHY_MODE_NONE;
    }

    VTSS_RC(icli_port_iter_init(&pit, sit.isid, PORT_ITER_FLAGS_FRONT | PORT_ITER_FLAGS_NPI));
    // Start veriphy at the selected ports.
    while (icli_port_iter_getnext(&pit, plist)) {
      mode[pit.iport] = (port_isid_port_info_get(sit.isid, pit.iport, &info) != VTSS_OK ||
                         (info.cap & PORT_CAP_1G_PHY) == 0 ? PORT_VERIPHY_MODE_NONE :
                         PORT_VERIPHY_MODE_FULL);
    }
    VTSS_RC(port_mgmt_veriphy_start(sit.isid, mode));
  }

  // Get veriPHY result
  VTSS_RC(switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID));
  while (icli_switch_iter_getnext(&sit, plist)) {
    port_iter_t pit;
    char                      buf[250], *p;
    int                       i;
    vtss_phy_veriphy_result_t veriphy;
    vtss_rc                   port_info_rc = VTSS_RC_OK, veriphy_get_rc = VTSS_RC_OK;
    VTSS_RC(icli_port_iter_init(&pit, sit.isid, PORT_ITER_FLAGS_FRONT | PORT_ITER_FLAGS_NPI));
    while (icli_port_iter_getnext(&pit, plist)) {
      if ((port_info_rc = port_isid_port_info_get(sit.isid, pit.iport, &info) != VTSS_OK) ||
          (info.cap & PORT_CAP_1G_PHY) == 0 ||
          (veriphy_get_rc = vtss_port_mgmt_veriphy_get(sit.isid, pit.iport, &veriphy, 60) != VTSS_OK)) {
        T_I("port_info_rc:%d, cap:%d, veriphy_get_rc:%d", port_info_rc, info.cap & PORT_CAP_1G_PHY, veriphy_get_rc);
        continue;
      }

      if (print_hdr) {
        sprintf(buf, "%-23s %-7s %-7s %-7s %-7s %-7s %-7s %-7s %-7s", "Interface", "Pair A", "Length", "Pair B,", "Length", "Pair C", "Length", "Pair D", "Length");
        icli_table_header(session_id, buf);
        print_hdr = FALSE;
      }

      // Print interface
      ICLI_PRINTF("%-23s ", icli_port_info_txt(sit.usid, pit.uport, &buf[0]));

      p = &buf[0];
      for (i = 0; i < 4; i++) {
        p += sprintf(p, "%-7s %-7d ",
                     port_mgmt_veriphy_txt(veriphy.status[i]), veriphy.length[i]);
      }
      ICLI_PRINTF("%s\n", buf);
    }
  }
#endif
  return VTSS_RC_OK;
}

//  see port_icli_functions.h
vtss_rc port_icli_veriphy(i32 session_id, icli_stack_port_range_t *plist)
{
  VTSS_ICLI_ERR_PRINT(icli_cmd_port_veriphy(session_id, plist));
  return VTSS_RC_OK;
}

//  see port_icli_functions.h
vtss_rc port_icli_status(i32 session_id, icli_stack_port_range_t *plist)
{
  VTSS_ICLI_ERR_PRINT(icli_cmd_port_status(session_id, plist, STATUS, NULL));
  return VTSS_RC_OK;
}

//  see port_icli_functions.h
vtss_rc port_icli_media_type(i32 session_id, icli_stack_port_range_t *plist, BOOL has_rj45, BOOL has_sfp, BOOL no)
{
  port_conf_value_t port_conf_value;

  port_conf_value.media_type.rj45 = has_rj45;
  port_conf_value.media_type.sfp = has_sfp;

  VTSS_ICLI_ERR_PRINT(port_icli_port_conf_set(session_id, MEDIA, &port_conf_value, plist, no));
  return VTSS_RC_OK;
}

//  see port_icli_functions.h
vtss_rc port_icli_mtu(i32 session_id, u16 max_length, icli_stack_port_range_t *plist, BOOL no)
{
  port_conf_value_t port_conf_value;

  port_conf_value.max_length = max_length;
  VTSS_ICLI_ERR_PRINT(port_icli_port_conf_set(session_id, MTU, &port_conf_value, plist, no));
  return VTSS_RC_OK;
}

// See port_icli_functions.h
vtss_rc port_icli_shutdown(i32 session_id, icli_stack_port_range_t *plist, BOOL no)
{
  VTSS_ICLI_ERR_PRINT(port_icli_port_conf_set(session_id, SHUT_DOWN, NULL, plist, no));
  return VTSS_RC_OK;
}

// See port_icli_functions.h
vtss_rc port_icli_flow_control(i32 session_id, icli_stack_port_range_t *plist, BOOL has_receive, BOOL has_send, BOOL has_on, BOOL has_off, BOOL no)
{
  port_conf_value_t port_conf_value;
  port_conf_value.flow_control.rx_enable = has_off;
  port_conf_value.flow_control.tx_enable = has_off;

  if (has_receive) {
    port_conf_value.flow_control.rx_enable = has_on; // If has_on is false then has_off is TRUE.
  }

  if (has_send) {
    port_conf_value.flow_control.tx_enable = has_on; // If has_on is false then has_off is TRUE.
  }
  VTSS_ICLI_ERR_PRINT(port_icli_port_conf_set(session_id, FLOW_CONTROL, &port_conf_value, plist, no));
  return VTSS_RC_OK;
}

// See port_icli_functions.h
vtss_rc port_icli_speed(i32 session_id, icli_stack_port_range_t *plist,  BOOL has_speed_10g, BOOL has_speed_2g5, BOOL has_speed_1g, BOOL has_speed_100m,
                        BOOL has_speed_10m, BOOL has_speed_auto, BOOL has_neg_10, BOOL has_neg_100, BOOL has_neg_1000, BOOL no)
{
  port_conf_value_t port_conf_value;


  if (has_speed_10g) {
    port_conf_value.speed = VTSS_SPEED_10G;
  } else   if (has_speed_2g5) {
    port_conf_value.speed = VTSS_SPEED_2500M;
  } else if (has_speed_1g) {
    port_conf_value.speed = VTSS_SPEED_1G;
  } else if (has_speed_100m) {
    port_conf_value.speed = VTSS_SPEED_100M;
  } else if (has_speed_10m) {
    port_conf_value.speed = VTSS_SPEED_10M;
  }

  if (has_speed_auto) {
    port_conf_value.autoneg_conf.autoneg = TRUE;
  }
  port_conf_value.autoneg_conf.has_neg_10 = has_neg_10;
  port_conf_value.autoneg_conf.has_neg_100 = has_neg_100;
  port_conf_value.autoneg_conf.has_neg_1000 = has_neg_1000;

  VTSS_ICLI_ERR_PRINT(port_icli_port_conf_set(session_id,  has_speed_auto ? AUTONEG : FORCED_SPEED, &port_conf_value, plist, no));
  return VTSS_RC_OK;
}

// See port_icli_functions.h
vtss_rc port_icli_duplex(i32 session_id, icli_stack_port_range_t *plist, BOOL has_half, BOOL has_full, BOOL has_auto, BOOL has_advertise_hdx, BOOL has_advertise_fdx, BOOL no)
{
  port_conf_value_t port_conf_value;

  port_conf_value.duplex.has_advertise_hdx = has_advertise_hdx;
  port_conf_value.duplex.has_advertise_fdx = has_advertise_fdx;
  port_conf_value.duplex.has_full = has_full;
  port_conf_value.duplex.has_half = has_half;
  port_conf_value.duplex.has_auto = has_auto;

  VTSS_ICLI_ERR_PRINT(port_icli_port_conf_set(session_id, DUPLEX, &port_conf_value, plist, no));
  return VTSS_RC_OK;
}

// See port_icli_functions.h
vtss_rc port_icli_excessive_restart(i32 session_id, icli_stack_port_range_t *plist, BOOL no)
{
  port_conf_value_t port_conf_value; // Dummy. In fact no data is needed for this command
  port_conf_value.duplex.has_auto = TRUE;
  VTSS_ICLI_ERR_PRINT(port_icli_port_conf_set(session_id, EXCESSIVE_RESTART, &port_conf_value, plist, no));
  return VTSS_RC_OK;
}

/***************************************************************************/
/* ICFG callback functions */
/****************************************************************************/
#ifdef VTSS_SW_OPTION_ICFG
static vtss_rc port_icfg_conf(const vtss_icfg_query_request_t *req, vtss_icfg_query_result_t *result)
{
  vtss_isid_t isid;
  vtss_port_no_t iport;
  BOOL is_default;
  vtss_icfg_conf_print_t conf_print;
  vtss_icfg_conf_print_init(&conf_print);

  T_NG(TRACE_GRP_ICLI, "mode:%d", req->cmd_mode);
  switch (req->cmd_mode) {
  case ICLI_CMD_MODE_GLOBAL_CONFIG:
    // Get current configuration for this switch
    break;
  case ICLI_CMD_MODE_INTERFACE_PORT_LIST:
    isid =  req->instance_id.port.isid;
    iport = req->instance_id.port.begin_iport;
    T_DG_PORT(TRACE_GRP_ICLI, iport, "Isid:%d, req->instance_id.port.usid:%d configurable:%d", isid, req->instance_id.port.usid, msg_switch_configurable(isid));

    if (msg_switch_configurable(isid)) {
      // Get current configuration for this switch
      port_conf_t port_conf;
      VTSS_RC(port_mgmt_conf_get(isid, iport, &port_conf));

      // Get default configuration
      port_conf_t port_conf_def;
      VTSS_RC(vtss_port_conf_default(isid, iport, &port_conf_def));

      //
      // Media
      //
      port_cap_t cap = port_isid_port_cap(isid, iport);
      BOOL media_type_is_default = FALSE;

      if (cap & PORT_CAP_SPEED_DUAL_ANY_FIBER) {
        if (port_conf.dual_media_fiber_speed == VTSS_SPEED_FIBER_NOT_SUPPORTED_OR_DISABLED) {
          // cu only
          VTSS_RC(vtss_icfg_printf(result, " media-type rj45\n"));
        } else if (!port_conf.autoneg) {
          // sfp only
          VTSS_RC(vtss_icfg_printf(result, " media-type sfp\n"));
        } else {
          // Both SFP and cu (default)
          media_type_is_default = TRUE;
        }
      } else {
        // SFP or CU only ports - meaning always return default as it make no sense to not having a media interface
        media_type_is_default = TRUE;
      }

      // Print default if required
      if (req->all_defaults && media_type_is_default) {
        VTSS_RC(vtss_icfg_printf(result, " no media-type\n"));
      }

      T_DG_PORT(TRACE_GRP_ICLI, iport, "all:%d, speed:%d, default_speed:%d", req->all_defaults, port_conf.speed, port_conf_def.speed);

      //
      // Speed
      //
      is_default = (port_conf.speed == port_conf_def.speed) && (port_conf.autoneg == port_conf_def.autoneg) && (port_conf.adv_dis == port_conf_def.adv_dis);

      // Auto negotiation
      if (req->all_defaults || !is_default) {
        if (is_default) {
          VTSS_RC(vtss_icfg_printf(result, " no speed"));
        } else {
          VTSS_RC(vtss_icfg_printf(result, " speed "));

          T_IG_PORT(TRACE_GRP_ICLI, iport, "port_conf.adv_dis:0x%X", port_conf.adv_dis);
          // Auto negotiation
          if (port_conf.autoneg) {
            VTSS_RC(vtss_icfg_printf(result, "auto"));
            if ((port_conf.adv_dis & PORT_ADV_DIS_10M) == 0) {
              VTSS_RC(vtss_icfg_printf(result, " 10"));
            }

            if ((port_conf.adv_dis & PORT_ADV_DIS_100M) == 0) {
              VTSS_RC(vtss_icfg_printf(result, " 100"));
            }

            if ((port_conf.adv_dis & PORT_ADV_DIS_1G_FDX) == 0) {
              VTSS_RC(vtss_icfg_printf(result, " 1000"));
            }
          } else {
            // Forced speed
            switch (port_conf.speed) {
            case VTSS_SPEED_UNDEFINED:
              T_E_PORT(iport, "Speed shall never be undefined");
              break;
            case VTSS_SPEED_10M:
              VTSS_RC(vtss_icfg_printf(result, "10"));
              break;
            case VTSS_SPEED_100M:
              VTSS_RC(vtss_icfg_printf(result, "100"));
              break;
            case VTSS_SPEED_1G:
              VTSS_RC(vtss_icfg_printf(result, "1000"));
              break;
            case VTSS_SPEED_2500M:
              VTSS_RC(vtss_icfg_printf(result, "2500"));
              break;
            case VTSS_SPEED_5G:
              VTSS_RC(vtss_icfg_printf(result, "5g"));
              break;
            case VTSS_SPEED_10G:
              VTSS_RC(vtss_icfg_printf(result, "10g"));
              break;
            case VTSS_SPEED_12G:
              VTSS_RC(vtss_icfg_printf(result, "12g"));
              break;
            }
          }
        }
        VTSS_RC(vtss_icfg_printf(result, "\n"));
      }

      //
      // Flow control
      //
      conf_print.is_default = port_conf.flow_control == port_conf_def.flow_control;
      VTSS_RC(vtss_icfg_conf_print(req, result, conf_print, "flowcontrol", "%s", port_conf.flow_control ? "on" : "off"));

      //
      // MTU
      //
      conf_print.is_default = port_conf.max_length == port_conf_def.max_length;
      VTSS_RC(vtss_icfg_conf_print(req, result, conf_print, "mtu", "%d", port_conf.max_length));

      //
      // DUPLEX
      //
      conf_print.is_default = port_conf.fdx == port_conf_def.fdx && port_conf.autoneg == port_conf_def.autoneg;
      T_NG_PORT(TRACE_GRP_ICLI, iport, "conf_print.is_default:%d, port_conf.fdx:%d, port_conf_def.fdx:%d, port_conf.autoneg:%d,port_conf_def.autoneg:%d;", conf_print.is_default, port_conf.fdx, port_conf_def.fdx, port_conf.autoneg, port_conf_def.autoneg);
      VTSS_RC(vtss_icfg_conf_print(req, result, conf_print, "duplex", "%s", port_conf.autoneg ? "auto" : port_conf.fdx ? "full" : "half"));

      //
      // Excessive Restart
      //
      conf_print.is_default = port_conf.exc_col_cont == port_conf_def.exc_col_cont;
      VTSS_RC(vtss_icfg_conf_print(req, result, conf_print, "excessive-restart", ""));

      //
      // Shut down
      //
      conf_print.is_default = port_conf.enable == port_conf_def.enable;
      VTSS_RC(vtss_icfg_conf_print(req, result, conf_print, "shutdown", ""));

    } // End msg_switch_configurable()
    break;

  default:
    //Not needed
    break;
  }

  return VTSS_RC_OK;
}

/* ICFG Initialization function */
vtss_rc port_icfg_init(void)
{
  VTSS_RC(vtss_icfg_query_register(VTSS_ICFG_PORT_GLOBAL_CONF, "port", port_icfg_conf));
  VTSS_RC(vtss_icfg_query_register(VTSS_ICFG_PORT_INTERFACE_CONF, "port", port_icfg_conf));
  return VTSS_RC_OK;
}
#endif // VTSS_SW_OPTION_ICFG
/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
