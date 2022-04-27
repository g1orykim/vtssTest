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

#include "vtss_api.h" /* For vtss_xxx() functions                  */
#include "misc_api.h" /* For vtss_chip_family_t                    */
#include "conf_api.h" /* For conf_xxx() functions                  */
#include "msg_api.h"  /* For msg_xxx() functions                   */
#include "eee_api.h"  /* For our own definitions                   */
#if defined(VTSS_SW_OPTION_VCLI) && defined(VTSS_SW_OPTION_PORT_POWER_SAVINGS)
#include "port_power_savings_cli.h"  /* For eee_cli_init() */
#endif
#include "eee.h"      /* For trace definitions and other internals */

#define EEE_CONF_VERSION 2

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_EEE

// The PHYs must conform to IEEE802.3az table 78-4
#define EEE_RX_TW_VALUE_1G    2
#define EEE_RX_TW_VALUE_100M 10
#define EEE_TX_TW_VALUE_1G   17
#define EEE_TX_TW_VALUE_100M 30
#define EEE_TW_VALUE_INIT    30

#define EEE_PHY_RX_WAKE_VALUE(_speed_) ((_speed_) == VTSS_SPEED_1G ? EEE_RX_TW_VALUE_1G : EEE_RX_TW_VALUE_100M)
#define EEE_PHY_TX_WAKE_VALUE(_speed_) ((_speed_) == VTSS_SPEED_1G ? EEE_TX_TW_VALUE_1G : EEE_TX_TW_VALUE_100M)

//****************************************
// TRACE
//****************************************
#if VTSS_TRACE_ENABLED
static vtss_trace_reg_t EEE_trace_reg = {
  .module_id = VTSS_TRACE_MODULE_ID,
  .name      = "eee",
  .descr     = "Energy Eficient Ethernet"
};

static vtss_trace_grp_t EEE_trace_grps[TRACE_GRP_CNT] = {
  [VTSS_TRACE_GRP_DEFAULT] = {
    .name      = "default",
    .descr     = "Default",
    .lvl       = VTSS_TRACE_LVL_WARNING,
    .timestamp = 1,
  },
  [VTSS_TRACE_GRP_CLI] = {
    .name      = "cli",
    .descr     = "CLI",
    .lvl       = VTSS_TRACE_LVL_WARNING,
    .timestamp = 1,
  },
  [TRACE_GRP_CRIT] = {
    .name      = "crit",
    .descr     = "Critical regions ",
    .lvl       = VTSS_TRACE_LVL_WARNING,
    .timestamp = 1,
  },
  [TRACE_GRP_IRQ] = {
    .name      = "irq",
    .descr     = "IRQ",
    .lvl       = VTSS_TRACE_LVL_WARNING,
    .timestamp = 1,
    .irq       = 1,
    .ringbuf   = 1,
    .usec      = 1,
  },
};
#endif

// Configuration for the whole stack - Overall configuration (valid on master only).
typedef struct {
  // One instance per switch in the stack of the switch configuration.
  // Indices are in the range [0; VTSS_ISID_CNT[, so all dereferences
  // must subtract VTSS_ISID_START from @isid to index correctly.
  eee_switch_conf_t switch_conf[VTSS_ISID_CNT];
  eee_switch_global_conf_t     global; // Configuration that is common for all switches in a stack
} eee_stack_conf_t;

// Overall configuration as saved in flash.
typedef struct {
  u32               version;    // Current version of the configuration in flash.
  eee_stack_conf_t  stack_conf; // Configuration for the whole stack
} eee_flash_conf_t;

// Configuration for the whole stack - Overall configuration (valid on master only) for 2.80 release (used for Silent upgrade).
typedef struct {
  // One instance per switch in the stack of the switch configuration.
  // Indices are in the range [0; VTSS_ISID_CNT[, so all dereferences
  // must subtract VTSS_ISID_START from @isid to index correctly.
  eee_switch_conf_t switch_conf[VTSS_ISID_CNT];
} eee_stack_conf_280_t;

// Overall configuration as saved in flash for 2.80 release (Used for silent upgrade)
typedef struct {
  u32               version;    // Current version of the configuration in flash.
  eee_stack_conf_280_t  stack_conf; // Configuration for the whole stack
} eee_flash_conf_280_t;



typedef enum {
  EEE_MSG_ID_CONF_SET,         // Configuration set request (no reply)
  EEE_MSG_ID_SWITCH_STATE_GET, // Get EEE state from slave
  EEE_MSG_ID_SWITCH_STATE_SET, // Returned state from slave to master.
  EEE_MSG_ID_PORT_STATE_SET,   // Sent autonomously during state changes from slave to master.
} eee_msg_id_t;

// Message for configuration sent by master.
typedef struct {
  eee_msg_id_t      msg_id;      // Message ID
  eee_switch_conf_t switch_conf; // Configuration that is local for a switch
} eee_msg_local_switch_conf_t;

// Message for configuration sent by master.
typedef struct {
  eee_msg_id_t      msg_id;      // Message ID
  eee_switch_conf_t switch_conf; // Configuration that is local for a switch
  eee_switch_global_conf_t switch_global_conf; // Configuration that is global for all switches in a stack
} eee_msg_switch_conf_t;

// Message for obtaining current capabilities on master.
typedef struct {
  eee_msg_id_t msg_id; // Message ID
} eee_msg_switch_state_get_t;

// Message for transmitting current switch state to master
typedef struct {
  eee_msg_id_t        msg_id; // Message ID
  eee_switch_state_t  state;
} eee_msg_switch_state_set_t;

// Message for transmitting a changed port state to master
typedef struct {
  eee_msg_id_t      msg_id; // Message ID
  vtss_port_no_t    port;
  eee_port_state_t  state;
} eee_msg_port_state_set_t;

//************************************************
// Global Variables
//************************************************
critd_t                    EEE_crit;        // Cannot be static.
eee_switch_conf_t          EEE_local_conf;  // Cannot be static.
eee_switch_global_conf_t   EEE_global_conf;  // Cannot be static.
eee_switch_state_t         EEE_local_state; // Cannot be static.
static eee_stack_conf_t    EEE_stack_conf;  // Configuration for whole stack (used when we're master, only). Indexed [0; VTSS_ISID_CNT - 1[, so must subtract VTSS_ISID_START from a vtss_isid_t.
static eee_switch_state_t  EEE_stack_state[VTSS_ISID_CNT];
static cyg_handle_t        EEE_thread_handle;
static cyg_thread          EEE_thread_block;
static char                EEE_thread_stack[THREAD_DEFAULT_STACK_SIZE];
static vtss_chip_id_t      EEE_chip_id;

static cyg_flag_t eee_msg_id_get_stat_req_flags; // Flag for synch. of reply of local EEE state.

/****************************************************************************/
// EEE_msg_tx_switch_state_get()
/****************************************************************************/
static void EEE_msg_tx_switch_state_get(vtss_isid_t isid)
{
  eee_msg_switch_state_get_t *msg;
  EEE_CRIT_ASSERT_LOCKED();

  if (!msg_switch_exists(isid)) {
    return;
  }

  msg = VTSS_MALLOC(sizeof(eee_msg_switch_state_get_t));
  if (msg == NULL) {
    T_E("Allocation failed.\n");
    return;
  }

  msg->msg_id = EEE_MSG_ID_SWITCH_STATE_GET;
  msg_tx(VTSS_MODULE_ID_EEE, isid, msg, sizeof(*msg));
  T_D("Sending state get request for isid:%d", isid);
}

/****************************************************************************/
// EEE_local_state_power_save_state_update()
// Updates the local state with the rx and tx power save state
/****************************************************************************/
static void EEE_local_state_power_save_state_update(void)
{
  port_iter_t              pit;
  (void)port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);

  while (port_iter_getnext(&pit)) {
    EEE_local_state.port[pit.iport].rx_in_power_save_state = FALSE;
    EEE_local_state.port[pit.iport].tx_in_power_save_state = FALSE;
    if (EEE_local_state.port[pit.iport].eee_capable) {

      // Get PHYs current power save state.
      if (vtss_phy_eee_power_save_state_get(NULL, pit.iport, &EEE_local_state.port[pit.iport].rx_in_power_save_state, &EEE_local_state.port[pit.iport].tx_in_power_save_state) != VTSS_RC_OK) {
        T_E("Could not get PHY power save state");
      }
    }
  }
}

/****************************************************************************/
// EEE_msg_tx_switch_state_set()
// Transmit properties (capabilities) to current master.
/****************************************************************************/
static void EEE_msg_tx_switch_state_set(void)
{
  eee_msg_switch_state_set_t *msg;
  EEE_CRIT_ASSERT_LOCKED();

  msg = VTSS_MALLOC(sizeof(eee_msg_switch_state_set_t));
  if (msg == NULL) {
    T_E("Allocation failed.\n");
    return;
  }

  msg->msg_id = EEE_MSG_ID_SWITCH_STATE_SET;

  EEE_local_state_power_save_state_update(); // Update the power save state
  T_D("eee_capable:%d", EEE_local_state.port[0].eee_capable);
  msg->state  = EEE_local_state;
  msg_tx(VTSS_MODULE_ID_EEE, 0, msg, sizeof(*msg));
}

/****************************************************************************/
// EEE_msg_tx_port_state_set()
// Transmit state of a port to current master.
/****************************************************************************/
static void EEE_msg_tx_port_state_set(vtss_port_no_t port)
{
  eee_msg_port_state_set_t *msg;

  EEE_CRIT_ASSERT_LOCKED();

  msg = VTSS_MALLOC(sizeof(eee_msg_port_state_set_t));
  if (msg == NULL) {
    T_E("Allocation failed.\n");
    return;
  }

  msg->msg_id = EEE_MSG_ID_PORT_STATE_SET;
  msg->port   = port;
  msg->state  = EEE_local_state.port[port];
  msg_tx(VTSS_MODULE_ID_EEE, 0, msg, sizeof(*msg));
}

/****************************************************************************/
// EEE_msg_tx_switch_conf()
/****************************************************************************/
static void EEE_msg_tx_switch_conf(vtss_isid_t isid)
{
  eee_msg_switch_conf_t *msg;

  EEE_CRIT_ASSERT_LOCKED();

  if (!msg_switch_exists(isid)) {
    return;
  }

  msg = VTSS_MALLOC(sizeof(eee_msg_switch_conf_t));
  if (msg == NULL) {
    T_E("Allocation failed.\n");
    return;
  }

  msg->msg_id = EEE_MSG_ID_CONF_SET;
  msg->switch_conf = EEE_stack_conf.switch_conf[isid - VTSS_ISID_START];
  msg->switch_global_conf = EEE_stack_conf.global;
  msg_tx(VTSS_MODULE_ID_EEE, isid, msg, sizeof(*msg));
}

/******************************************************************************/
// EEE_check_isid_port()
/******************************************************************************/
static vtss_rc EEE_check_isid_port(vtss_isid_t isid, vtss_port_no_t port, BOOL allow_local, BOOL check_port)
{
  if ((isid != VTSS_ISID_LOCAL && !VTSS_ISID_LEGAL(isid)) || (isid == VTSS_ISID_LOCAL && !allow_local)) {
    return EEE_ERROR_ISID;
  }
  if (isid != VTSS_ISID_LOCAL && !msg_switch_is_master()) {
    return EEE_ERROR_NOT_MASTER;
  }
  if (check_port && (port >= port_isid_port_count(isid) || port_isid_port_no_is_stack(isid, port))) {
    return EEE_ERROR_PORT;
  }
  return VTSS_OK;
}

/******************************************************************************/
// EEE_state_change()
/******************************************************************************/
static void EEE_state_change(vtss_port_no_t port,
                             BOOL           local,
                             u16            RemRxSystemValue,
                             u16            RemTxSystemValue,
                             u16            RemRxSystemValueEcho,
                             u16            RemTxSystemValueEcho,
                             BOOL           callback)
{
  eee_port_state_t *p = &EEE_local_state.port[port];
  u16 NEW_RX_VALUE, NEW_TX_VALUE = 0;
  BOOL tx_update = FALSE;
  BOOL changed = local; // State is always changed when this function call is caused by local changes.

  if (p->running) {
    if ((RemTxSystemValueEcho != p->RemTxSystemValueEcho) ||
        (RemRxSystemValue     != p->RemRxSystemValue)     ||
        (RemRxSystemValueEcho != p->RemRxSystemValueEcho) ||
        (RemTxSystemValue     != p->RemTxSystemValue)) {
      changed = TRUE;
    }

    // Tx SM
    // Snippet from 802.3az-2010, 78.4.3.1
    // A transmitting link partner is said to be in sync with the receiving link partner if the presently advertised
    // value of Transmit Tw_sys_tx (aLldpXdot3LocTxTwSys/LocTxSystemValue) and the corresponding echoed value
    // (aLldpXdot3RemTxTwSysEcho/RemTxSystemValueEcho) are equal.
    // During normal operation, the transmitting link partner is in the RUNNING state. If the transmitting link
    // partner wants to initiate a change to the presently resolved value of Tw_sys_tx, the local_system_change is
    // asserted and the transmitting link partner enters the LOCAL CHANGE state where NEW_TX_VALUE is
    // computed. If the new value is smaller than the presently advertised value of Tw_sys_tx or if the transmitting
    // link partner is in sync with the receiving link partner, then it enters TX UPDATE state. Otherwise, it returns
    // to the RUNNING state.
    // If the transmitting link partner sees a change in the Tw_sys_tx requested by the receiving link partner, it
    // recognizes the request only if it is in sync with the transmitting link partner. The transmitting link partner
    // examines the request by entering the REMOTE CHANGE state where a NEW TX VALUE is computed and
    // it then enters the TX UPDATE state.
    // Upon entering the TX UPDATE state, the transmitter updates the advertised value of Transmit Tw_sys_tx with
    // NEW_TX_VALUE. If the NEW_TX_VALUE is equal to or greater than either the resolved Tw_sys_tx value
    // or the value requested by the receiving link partner then it enters the SYSTEM REALLOCATION state
    // where it updates the value of resolved Tw_sys_tx with NEW_TX_VALUE. The transmitting link partner
    // enters the MIRROR UPDATE state either from the SYSTEM REALLOCATION state or directly from the
    // TX UPDATE state. The UPDATE MIRROR state then updates the echo for the Receive Tw_sys_tx and
    // returns to the RUNNING state.
    if (local) {
      // State: LOCAL CHANGE
      p->TempRxVar = p->RemRxSystemValue;
      NEW_TX_VALUE = MAX(p->tx_tw, p->RemRxSystemValue);
      if (p->LocTxSystemValue == p->RemTxSystemValueEcho || NEW_TX_VALUE < p->LocTxSystemValue) {
        tx_update = TRUE;
      }
    } else {
      p->RemTxSystemValueEcho = RemTxSystemValueEcho;
      p->RemRxSystemValue     = RemRxSystemValue;

      if (p->RemRxSystemValue != p->TempRxVar && p->LocTxSystemValue == p->RemTxSystemValueEcho) {
        // State: REMOTE CHANGE
        // We only get here when a change in the remote end's Rx value is detected, and
        // when we're in sync (see above).
        p->TempRxVar = p->RemRxSystemValue;
        NEW_TX_VALUE = MAX(p->tx_tw, p->RemRxSystemValue);
        tx_update = TRUE;
      }
    }

    if (tx_update) {
      // State: TX UPDATE
      p->LocTxSystemValue = NEW_TX_VALUE;

      if (NEW_TX_VALUE >= p->LocResolvedTxSystemValue || NEW_TX_VALUE >= p->TempRxVar) {
        // State: SYSTEM REALLOCATION
        p->LocResolvedTxSystemValue = NEW_TX_VALUE;
      }

      // State: MIRROR UPDATE
      p->LocRxSystemValueEcho = p->TempRxVar;
    }

    // Rx SM
    // Snippet from 802.3az-2010, 78.4.3.2
    // A receiving link partner is said to be in sync with the transmitting link partner if the presently requested
    // value of Receive Tw_sys_tx and the corresponding echoed value are equal.
    // During normal operation, the receiving link partner is in the RUNNING state. If the receiving link partner
    // wants to request a change to the presently resolved value of Tw_sys_tx, the local_system_change is asserted.
    // When local_system_change is asserted or when the receiving link partner sees a change in the Tw_sys_tx
    // advertised by the transmitting link partner, it enters the CHANGE state where NEW_RX_VALUE is
    // computed. If NEW_RX_VALUE is less than either the presently resolved value of Tw_sys_tx or the presently
    // advertised value by the transmitting link partner, it enters the SYSTEM REALLOCATION state where it
    // updates the resolved value of Tw_sys_tx to NEW_RX_VALUE. The receiving link partner ultimately enters
    // the RX UPDATE state, either from the SYSTEM REALLOCATION state or directly from the CHANGE
    // state.
    // In the RX UPDATE state, it updates the presently requested value to NEW_RX_VALUE, then it updates the
    // echo for the Transmit Tw_sys_tx in the UPDATE MIRROR state and finally goes back to the RUNNING
    // state.
    if (!local) {
      p->RemRxSystemValueEcho = RemRxSystemValueEcho;
      p->RemTxSystemValue     = RemTxSystemValue;
    }
    if (local || p->RemTxSystemValue != p->TempTxVar) {
      // State: CHANGE
      p->TempTxVar = p->RemTxSystemValue;
      NEW_RX_VALUE = MAX(p->rx_tw, p->RemTxSystemValue);
      if (NEW_RX_VALUE <= p->LocResolvedRxSystemValue || NEW_RX_VALUE <= p->TempTxVar) {
        // State: SYSTEM REALLOCATION
        p->LocResolvedRxSystemValue = NEW_RX_VALUE;
      }

      // State: RX UPDATE
      p->LocRxSystemValue = NEW_RX_VALUE;
      p->LocFbSystemValue = NEW_RX_VALUE;

      // State: UPDATE MIRROR
      p->LocTxSystemValueEcho = p->TempTxVar;
    }
  }

  // Update switch-specific part and master with any changes.
  if (changed && callback) {
    EEE_CRIT_ASSERT_LOCKED();
    EEE_platform_tx_wakeup_time_changed(port, p->LocResolvedTxSystemValue);

    EEE_msg_tx_port_state_set(port);
  }
}

/******************************************************************************/
// EEE_state_init()
/******************************************************************************/
static void EEE_local_state_init(vtss_port_no_t port, port_info_t *info, BOOL callback)
{
  eee_port_state_t *p = &EEE_local_state.port[port];
  u8               lp_advertisement;

  p->link    = info->link;
  p->speed   = info->speed;
  p->fdx     = info->fdx;

  if (info->link) {
    if (vtss_phy_eee_link_partner_advertisements_get(NULL, port, &lp_advertisement) != VTSS_RC_OK) {
      T_E("Could not get link partner EEE auto negotiation information");
      lp_advertisement = 0;
    }
    p->link_partner_eee_capable = lp_advertisement == 0 ? FALSE : TRUE;
  } else {
    p->link_partner_eee_capable = FALSE;
  }

  if (!p->rx_tw_changed_by_debug_cmd) {
    p->rx_tw = EEE_PHY_RX_WAKE_VALUE(p->speed);
  }

  if (!p->tx_tw_changed_by_debug_cmd) {
    p->tx_tw = EEE_PHY_TX_WAKE_VALUE(p->speed);
  }

  p->running = EEE_local_conf.port[port].eee_ena && p->eee_capable && p->link_partner_eee_capable && info->link && (info->speed == VTSS_SPEED_100M || info->speed == VTSS_SPEED_1G) && info->fdx == TRUE;

  // And then a range of flow-control exceptions.
  if (p->running) {
    port_status_t port_status;
    if (port_mgmt_status_get(VTSS_ISID_LOCAL, port, &port_status) != VTSS_RC_OK) {
      p->running = FALSE;
    } else if (port_status.status.aneg.obey_pause || port_status.status.aneg.generate_pause) {
      // Running flow control. Check chip family.
      vtss_chip_family_t family = misc_chipfamily();
      if (family == VTSS_CHIP_FAMILY_JAGUAR1) {
        // EEE and flow control not supported.
        p->running = FALSE;
      } else if (family == VTSS_CHIP_FAMILY_SPARX_III) {
        // EEE and flow control not supported on Rev. A and B.
        p->running = EEE_chip_id.revision != 0 && EEE_chip_id.revision != 1;
      } else if (family == VTSS_CHIP_FAMILY_SERVAL) {
        p->running = TRUE;
      } else {
        T_E("Unknown chip family:%d. Please revise eee.c", family);
      }
    }
  }

  if (!p->running) {
    // Rx SM
    p->LocRxSystemValue         = EEE_TW_VALUE_INIT;
    p->RemRxSystemValueEcho     = 0;
    p->RemTxSystemValue         = 0;
    p->LocTxSystemValueEcho     = 0;
    p->LocResolvedRxSystemValue = EEE_TW_VALUE_INIT;
    p->LocFbSystemValue         = 0;
    p->TempTxVar                = 0;

    // Tx SM
    p->LocTxSystemValue         = EEE_TW_VALUE_INIT;
    p->RemTxSystemValueEcho     = 0;
    p->RemRxSystemValue         = 0;
    p->LocRxSystemValueEcho     = 0;
    p->LocResolvedTxSystemValue = EEE_TW_VALUE_INIT;
    p->TempRxVar                = 0;
  } else {
    // Rx SM
    p->LocRxSystemValue         = p->rx_tw;
    p->RemRxSystemValueEcho     = p->tx_tw;
    p->RemTxSystemValue         = p->tx_tw;
    p->LocTxSystemValueEcho     = p->tx_tw;
    p->LocResolvedRxSystemValue = p->tx_tw;
    p->LocFbSystemValue         = p->tx_tw;
    p->TempTxVar                = p->tx_tw;

    // Tx SM
    p->LocTxSystemValue         = p->tx_tw;
    p->RemTxSystemValueEcho     = p->tx_tw;
    p->RemRxSystemValue         = p->tx_tw;
    p->LocRxSystemValueEcho     = p->tx_tw;
    p->LocResolvedTxSystemValue = p->tx_tw;
    p->TempRxVar                = p->tx_tw;
  }

  // Run the state machine once.
  EEE_state_change(port, TRUE, 0, 0, 0, 0, callback);
}

/****************************************************************************/
// EEE_msg_rx()
/****************************************************************************/
static BOOL EEE_msg_rx(void *contxt, const void *const rx_msg, const size_t len, const vtss_module_id_t modid, const ulong isid)
{
  eee_msg_id_t msg_id = *(eee_msg_id_t *)rx_msg;

  switch (msg_id) {
  case EEE_MSG_ID_CONF_SET: {
    eee_msg_switch_conf_t *msg = (eee_msg_switch_conf_t *)rx_msg;
    BOOL                        port_changes[VTSS_PORTS];
    port_iter_t                 pit;
    memset(port_changes, 0, sizeof(port_changes));

    (void)port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);

    EEE_CRIT_ENTER();
    // When we get here, EEE_local_state.eee_capable[] is definitely updated because the thread
    // doesn't let go of the EEE crit before it's initialized.

    // Enable or disable EEE-advertisement in PHY. This may be done whether or not the PHY has link.
    while (port_iter_getnext(&pit)) {
      if (EEE_local_state.port[pit.iport].eee_capable) {
        if (vtss_phy_eee_ena(NULL, pit.iport, msg->switch_conf.port[pit.iport].eee_ena) != VTSS_RC_OK) {
          T_E("Unable to set PHY EEE-advertisement for port %u", pit.uport);
        }
        port_changes[pit.iport] = memcmp(&EEE_local_conf.port[pit.iport], &msg->switch_conf.port[pit.iport], sizeof(eee_port_conf_t)) != 0 || EEE_global_conf.optimized_for_power != msg->switch_global_conf.optimized_for_power;
        if (port_changes[pit.iport]) {
          port_info_t info;
          (void)port_info_get(pit.iport, &info);
          EEE_local_state_init(pit.iport, &info, TRUE);
        }
      }
    }

    EEE_local_conf = msg->switch_conf;
    EEE_global_conf  = msg->switch_global_conf;

    EEE_CRIT_EXIT();

    // Call switch-specific function.
    EEE_platform_local_conf_set(port_changes);
    break;
  }

  case EEE_MSG_ID_SWITCH_STATE_GET:
    // Master requests our local EEE state. Send it to him.
    T_D("Master requests our local EEE state. Send it to him");
    EEE_CRIT_ENTER();
    EEE_msg_tx_switch_state_set();
    EEE_CRIT_EXIT();
    break;

  case EEE_MSG_ID_SWITCH_STATE_SET: {
    // Slave has sent his local EEE state for a switch. Save it.
    T_D("Slave has sent his local EEE state for a switch");
    EEE_CRIT_ENTER();
    eee_msg_switch_state_set_t *msg = (eee_msg_switch_state_set_t *)rx_msg;
    EEE_stack_state[isid - VTSS_ISID_START] = msg->state;
    cyg_flag_setbits(&eee_msg_id_get_stat_req_flags, 1 << isid); // Signal that the message has been received
    EEE_CRIT_EXIT();
    break;
  }

  case EEE_MSG_ID_PORT_STATE_SET: {
    // Slave has sent his local EEE state for a port. save it.

    EEE_CRIT_ENTER();
    eee_msg_port_state_set_t *msg = (eee_msg_port_state_set_t *)rx_msg;
    EEE_stack_state[isid - VTSS_ISID_START].port[msg->port] = msg->state;
    EEE_CRIT_EXIT();
    break;
  }

  default:
    T_W("Unknown message ID: %d", msg_id);
    break;
  }
  return TRUE;
}

/****************************************************************************/
// EEE_msg_init()
// Initializes the message protocol
/****************************************************************************/
static void EEE_msg_init(void)
{
  // Register for EEE messages
  msg_rx_filter_t filter;

  memset(&filter, 0, sizeof(filter));
  filter.cb = EEE_msg_rx;
  filter.modid = VTSS_MODULE_ID_EEE;
  if (msg_rx_filter_register(&filter) != VTSS_RC_OK) {
    T_E("Failed to register for EEE messages");
  }
}

/****************************************************************************/
// EEE_read_from_flash()
// On entry, isid is always a legal isid.
/****************************************************************************/
static void  EEE_read_from_flash(vtss_isid_t isid, BOOL create)
{
  vtss_isid_t       isid_start, isid_end, isid_iter;
  eee_flash_conf_t *flash_conf; // Configuration in flash
  ulong             size;
  BOOL              do_create = FALSE;  // Set if we need to create new configuration in flash
  EEE_CRIT_ASSERT_LOCKED();

  if (misc_conf_read_use()) {
    // Get configration from flash (if possible)
    if ((flash_conf = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_EEE_CONF, &size)) == NULL || size != sizeof(eee_flash_conf_t)) {
      // Silent upgrade from 2.80 release
      if (flash_conf != NULL && size == sizeof(eee_flash_conf_280_t)) {
        eee_flash_conf_280_t eee_flash_conf_280;

        // The flash_conf was opened as a 2.80 release configuration. Take a copy, and create a new flash_conf which matches the newest configuration.
        memcpy(&eee_flash_conf_280, flash_conf, sizeof(eee_flash_conf_280));
        if ((flash_conf = conf_sec_create(CONF_SEC_GLOBAL, CONF_BLK_EEE_CONF, sizeof(eee_flash_conf_t))) != NULL) {
          // Copy the configuration from 2.80 configuration to new configuration
          memcpy(&flash_conf->stack_conf, &eee_flash_conf_280.stack_conf, sizeof(eee_flash_conf_280.stack_conf));
          flash_conf->version                               = eee_flash_conf_280.version;
          flash_conf->stack_conf.global.optimized_for_power = OPTIMIZE_FOR_POWER_DEFAULT;
        } else {
          do_create = TRUE;
          isid      = VTSS_ISID_GLOBAL; // Do it for all switches
        }
      } else {
        flash_conf = conf_sec_create(CONF_SEC_GLOBAL, CONF_BLK_EEE_CONF, sizeof(eee_flash_conf_t));
        T_W("conf_sec_open failed or size mismatch, creating defaults");
        do_create = TRUE;
        isid      = VTSS_ISID_GLOBAL; // Do it for all switches
      }
    } else if (flash_conf->version != EEE_CONF_VERSION) {
      T_W("Version mismatch, creating defaults");
      do_create = TRUE;
      isid      = VTSS_ISID_GLOBAL; // Do it for all switches
    } else {
      do_create = create;
    }
  } else {
    flash_conf = NULL;
    do_create  = TRUE;
  }

  if (!do_create) {
    // Check if configuration is valid.
    for (isid_iter = VTSS_ISID_START; isid_iter < VTSS_ISID_END; isid_iter++) {
      if (flash_conf && !EEE_platform_conf_valid(&flash_conf->stack_conf.switch_conf[isid_iter - VTSS_ISID_START])) {
        do_create = TRUE;
        isid      = VTSS_ISID_GLOBAL; // Create defaults for all switches.
        break;
      }
    }
  }

  if (isid == VTSS_ISID_GLOBAL) {
    isid_start = VTSS_ISID_START;
    isid_end   = VTSS_ISID_END;
  } else {
    isid_start = isid;
    isid_end   = isid + 1;
  }

  // Create configuration in flash
  if (do_create) {
    // Set default configuration
    for (isid_iter = isid_start; isid_iter < isid_end; isid_iter++) {
      // Re-initialize switch(es).
      eee_switch_conf_t *conf = &EEE_stack_conf.switch_conf[isid_iter - VTSS_ISID_START];
      memset(conf, 0, sizeof (*conf));
      EEE_platform_conf_reset(conf);
    }

    EEE_stack_conf.global.optimized_for_power = OPTIMIZE_FOR_POWER_DEFAULT;

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    // Write our settings back to flash if we succeeded in allocating a section in flash.
    if (flash_conf) {
      flash_conf->version = EEE_CONF_VERSION;
      flash_conf->stack_conf = EEE_stack_conf;
      conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_EEE_CONF);
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
  } else {
    // Copy the newly read (or defaulted) configuration into the global configuration.
    if (flash_conf != NULL) {
      EEE_stack_conf = flash_conf->stack_conf;
    }
  }

  // Send newly loaded configuration to all switches in case #create is TRUE.
  // If #create is TRUE, this function is called from "load defaults" event.
  // If #create is FALSE, this function is called from "master up" event. Configuration is then sent from "switch add" event.
  for (isid_iter = isid_start; create && isid_iter < isid_end; isid_iter++) {
    EEE_msg_tx_switch_conf(isid_iter);
  }
}

/******************************************************************************/
// EEE_port_link_change_event()
/******************************************************************************/
static void EEE_port_link_change_event(vtss_port_no_t iport, port_info_t *info)
{
  if (info->phy) {
    EEE_CRIT_ENTER();
    // Update state
    EEE_local_state_init(iport, info, TRUE);

    // Call platform-specific code
    EEE_platform_port_link_change_event(iport, info);
    EEE_CRIT_EXIT();
  }
}

/******************************************************************************/
// EEE_thread()
/******************************************************************************/
/* Lint cannot see that EEE_thread() is locked on entry, so tell it that it's
   safe to update EEE_local_state.port[].eee_capable even though other threads
   are reading it. */
/*lint -sem(EEE_thread, thread_protected) */
static void EEE_thread(cyg_addrword_t data)
{
  BOOL        capable;
  port_iter_t pit;

  (void)port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);

  // Register for port link change events on local switch.
  if (port_change_register(VTSS_MODULE_ID_EEE, EEE_port_link_change_event) != VTSS_RC_OK) {
    T_E("Couldn't register for port link change events");
    // Gotta exit crit so that management functions can be called.
    /*lint -e(455) */
    EEE_CRIT_EXIT();

    // The EEE-capability array shows that no PHYs are EEE capable at this point in time-
    // No need for this thread at all.
    return;
  }

  // This will block this thread from running further until the PHYs are initialized.
  port_phy_wait_until_ready();

  // Now the PHYs are ready, we can continue querying for EEE-capableness.
  while (port_iter_getnext(&pit)) {
    if (vtss_phy_port_eee_capable(NULL, pit.iport, &capable) == VTSS_RC_OK) {
      EEE_local_state.port[pit.iport].eee_capable = capable;
      T_D("EEE_local_state.port[%d].eee_capable:%d", pit.iport, EEE_local_state.port[pit.iport].eee_capable);
    }
  }

  // Platform-specific thread handler that never returns.
  // It's up to the platform specific thread handler to
  // call EEE_CRIT_EXIT() when it's ready.
  EEE_platform_thread();
}

/****************************************************************************/
// EEE_save_to_flash()
/****************************************************************************/
static vtss_rc EEE_save_to_flash(void)
{
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
  vtss_rc          rc = EEE_ERROR_FLASH;
  eee_flash_conf_t *flash_conf;
  ulong            size;

  EEE_CRIT_ASSERT_LOCKED();

  if ((flash_conf = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_EEE_CONF, &size)) != NULL) {
    if (size == sizeof(*flash_conf)) {
      flash_conf->stack_conf = EEE_stack_conf;
      rc = VTSS_OK;
    } else {
      T_W("Could not save EEE configuration - size did not match");
    }
    conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_EEE_CONF);
  } else {
    T_W("Could not save EEE configuration");
  }

  return rc;
#else
  return VTSS_RC_OK;
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
}

/******************************************************************************/
//
// PUBLIC FUNCTIONS
//
/******************************************************************************/

/******************************************************************************/
// eee_error_txt()
// Converts EEE error to printable text
/******************************************************************************/
char *eee_error_txt(vtss_rc rc)
{
  switch (rc) {
  case EEE_ERROR_ISID:
    return "Invalid Switch ID";

  case EEE_ERROR_PORT:
    return "Invalid port number";

  case EEE_ERROR_FLASH:
    return "Could not store configuration in flash";

  case EEE_ERROR_SLAVE:
    return "Could not get data from slave switch";

  case EEE_ERROR_NOT_MASTER:
    return "Switch must be master";

  case EEE_ERROR_VALUE:
    return "Invalid value";

  case EEE_ERROR_NOT_CAPABLE:
    return "Interface not EEE capable";

  default:
    return "";
  }
}

/******************************************************************************/
// eee_mgmt_switch_global_conf_get()
// Function that returns the current global configuration for a stack.
// In/out : switch_global_conf - Pointer to configuration struct where the current
// global configuration is copied to.
/******************************************************************************/
vtss_rc eee_mgmt_switch_global_conf_get(eee_switch_global_conf_t *switch_global_conf)
{

  if (switch_global_conf == NULL) {
    return EEE_ERROR_VALUE;
  }

  EEE_CRIT_ENTER();
  *switch_global_conf = EEE_stack_conf.global;
  EEE_CRIT_EXIT();

  return VTSS_RC_OK;
}

/******************************************************************************/
// eee_mgmt_switch_global_conf_set()
// Function for setting the current global configuration for a stack.
//      switch_global_conf - Pointer to configuration struct with the new configuration.
// Return : VTSS error code
/******************************************************************************/
vtss_rc eee_mgmt_switch_global_conf_set(eee_switch_global_conf_t *switch_global_conf)
{
  vtss_isid_t isid;
  vtss_rc result;
  if (switch_global_conf == NULL) {
    return EEE_ERROR_VALUE;
  }

  // Transfer new configuration to the all switches.
  EEE_CRIT_ENTER();

  // Update the configuration for the switch
  EEE_stack_conf.global = *switch_global_conf;

  for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
    EEE_msg_tx_switch_conf(isid);
  }
  // Store the new configuration in flash
  result = EEE_save_to_flash();

  EEE_CRIT_EXIT();

  return result;
}

/******************************************************************************/
// eee_mgmt_switch_conf_get()
// Function that returns the current configuration for a switch.
// In : isid - isid for the switch the shall return its configuration
// In/out : switch_conf - Pointer to configuration struct where the current
// configuration is copied to.
/******************************************************************************/
vtss_rc eee_mgmt_switch_conf_get(vtss_isid_t isid, eee_switch_conf_t *switch_conf)
{
  vtss_rc result;
  if ((result = EEE_check_isid_port(isid, VTSS_PORT_NO_START, TRUE, FALSE)) != VTSS_RC_OK) {
    return result;
  }

  if (switch_conf == NULL) {
    return EEE_ERROR_VALUE;
  }

  EEE_CRIT_ENTER();
  if (isid == VTSS_ISID_LOCAL) {
    *switch_conf = EEE_local_conf;
  } else {
    *switch_conf = EEE_stack_conf.switch_conf[isid - VTSS_ISID_START];
  }
  EEE_CRIT_EXIT();

  return VTSS_RC_OK;
}

/******************************************************************************/
// eee_mgmt_switch_conf_set()
// Function for setting the current configuration for a switch.
// In : isid - isid for the switch the shall return its configuration
//      switch_conf - Pointer to configuration struct with the new configuration.
// Return : VTSS error code
/******************************************************************************/
vtss_rc eee_mgmt_switch_conf_set(vtss_isid_t isid, eee_switch_conf_t *switch_conf)
{
  vtss_rc     result;
  port_iter_t pit;

  // Configuration changes only allowed by master
  if ((result = EEE_check_isid_port(isid, VTSS_PORT_NO_START, TRUE, FALSE)) != VTSS_RC_OK) {
    T_D("result:%d", result);
    return result;
  }

  if (switch_conf == NULL) {
    return EEE_ERROR_VALUE;
  }

  (void)port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);

  result = VTSS_RC_OK;
  EEE_CRIT_ENTER();

  while (port_iter_getnext(&pit)) {
    if (msg_switch_exists(isid)) { //If the switch doesn't exists, we don't know if the port supports EEE, so we acts as it do, and only test for eee capable for switches which do exist.
      if (switch_conf->port[pit.iport].eee_ena && !EEE_stack_state[isid - VTSS_ISID_START].port[pit.iport].eee_capable) {
        T_I("Attempting to enable EEE on non-EEE-capable port (%u)", pit.uport);
        result = EEE_ERROR_NOT_CAPABLE;
        goto do_exit;
      }
    }
  }

  // Update the configuration for the switch
  EEE_stack_conf.switch_conf[isid - VTSS_ISID_START] = *switch_conf;

  // Transfer new configuration to the switch in question.
  EEE_msg_tx_switch_conf(isid);

  // Store the new configuration in flash
  result = EEE_save_to_flash();

do_exit:
  EEE_CRIT_EXIT();
  return result;
}

/******************************************************************************/
// eee_mgmt_switch_state_get()
/******************************************************************************/
#define MSG_TIMEOUT cyg_current_time() + VTSS_OS_MSEC2TICK(5 * 1000) /* Wait for timeout (5 seconds) or synch. flag to be set */
vtss_rc eee_mgmt_switch_state_get(vtss_isid_t isid, eee_switch_state_t *switch_state)
{
  vtss_rc result;

  if ((result = EEE_check_isid_port(isid, VTSS_PORT_NO_START, TRUE, FALSE)) != VTSS_RC_OK) {
    return result;
  }

  if (switch_state == NULL) {
    return EEE_ERROR_VALUE;
  }

  if (isid == VTSS_ISID_LOCAL) {
    EEE_CRIT_ENTER();
    *switch_state = EEE_local_state;
    EEE_CRIT_EXIT();
  } else if (msg_switch_exists(isid)) {
    cyg_flag_value_t flag;
    flag = (1 << isid);
    cyg_flag_maskbits(&eee_msg_id_get_stat_req_flags, ~flag);
    EEE_CRIT_ENTER();
    EEE_msg_tx_switch_state_get(isid); // Ask slave to send its state
    EEE_CRIT_EXIT();
    // Wait for receiving the state from the slave.
    if (cyg_flag_timed_wait(&eee_msg_id_get_stat_req_flags, flag, CYG_FLAG_WAITMODE_OR, MSG_TIMEOUT) & flag ? 0 : 1) {
      T_W("EEE msg state timeout - Status values from switch:%d not valid", isid);
      return EEE_ERROR_VALUE;
    }

    T_D("Got slave state, isid:%d", isid);
    EEE_CRIT_ENTER();
    *switch_state = EEE_stack_state[isid - VTSS_ISID_START];
    EEE_CRIT_EXIT();
  } else {
    // ISID is not local, and doesn't exist at the moment: Pretend all ports are capable.
    vtss_port_no_t i;
    memset(switch_state, 0, sizeof(*switch_state));
    for (i = VTSS_PORT_NO_START; i < VTSS_PORTS; i++) {
      switch_state->port[i].eee_capable = TRUE;
    }
  }

  return VTSS_RC_OK;
}

/******************************************************************************/
// eee_mgmt_port_state_get()
/******************************************************************************/
vtss_rc eee_mgmt_port_state_get(vtss_isid_t isid, vtss_port_no_t port, eee_port_state_t *port_state)
{
  vtss_rc result;
  if ((result = EEE_check_isid_port(isid, port, TRUE, TRUE)) != VTSS_RC_OK) {
    return result;
  }

  if (port_state == NULL) {
    return EEE_ERROR_VALUE;
  }

  EEE_CRIT_ENTER();

  if (isid == VTSS_ISID_LOCAL) {
    EEE_local_state_power_save_state_update(); // Update the power save state
    *port_state = EEE_local_state.port[port];
  } else {
    *port_state = EEE_stack_state[isid - VTSS_ISID_START].port[port];
  }
  EEE_CRIT_EXIT();

  return VTSS_RC_OK;
}

/******************************************************************************/
// eee_mgmt_remote_state_change()
// Called by LLDP on local switch, so no isid.
/******************************************************************************/
void eee_mgmt_remote_state_change(vtss_port_no_t port,
                                  u16            RemRxSystemValue,     /* tx state machine */
                                  u16            RemTxSystemValue,     /* rx state machine */
                                  u16            RemRxSystemValueEcho, /* rx state machine */
                                  u16            RemTxSystemValueEcho) /* tx state machine */

{
  if (port >= port_isid_port_count(VTSS_ISID_LOCAL)) {
    return;
  }

  // Possible remote system change request/update.
  EEE_CRIT_ENTER();
  EEE_state_change(port, FALSE, RemRxSystemValue, RemTxSystemValue, RemRxSystemValueEcho, RemTxSystemValueEcho, TRUE);
  EEE_CRIT_EXIT();
}

/******************************************************************************/
// eee_mgmt_local_state_change()
// Debug function called by CLI to request local changes.
// Only works on local switch.
/******************************************************************************/
void eee_mgmt_local_state_change(vtss_port_no_t port, BOOL clear, BOOL is_tx, u16 LocSystemValue)
{
  if (port >= port_isid_port_count(VTSS_ISID_LOCAL)) {
    return;
  }

  EEE_CRIT_ENTER();
  if (is_tx) {
    EEE_local_state.port[port].tx_tw                      =  clear ? EEE_PHY_TX_WAKE_VALUE(EEE_local_state.port[port].speed) : LocSystemValue;
    EEE_local_state.port[port].tx_tw_changed_by_debug_cmd = !clear;
  } else {
    EEE_local_state.port[port].rx_tw                      =  clear ? EEE_PHY_RX_WAKE_VALUE(EEE_local_state.port[port].speed) : LocSystemValue;
    EEE_local_state.port[port].rx_tw_changed_by_debug_cmd = !clear;
  }

  // Local system change request
  EEE_state_change(port, TRUE, 0, 0, 0, 0, TRUE);
  EEE_CRIT_EXIT();
}

/******************************************************************************/
// eee_init()
/******************************************************************************/
vtss_rc eee_init(vtss_init_data_t *data)
{
  vtss_isid_t    isid = data->isid;
  vtss_port_no_t iport;
  port_info_t    info;

  /*lint --e{454,456} */

  switch (data->cmd) {
  case INIT_CMD_INIT:
    // Initialize and register trace ressources
#if VTSS_TRACE_ENABLED
    VTSS_TRACE_REG_INIT(&EEE_trace_reg, EEE_trace_grps, TRACE_GRP_CNT);
    VTSS_TRACE_REGISTER(&EEE_trace_reg);
#endif
    cyg_flag_init(&eee_msg_id_get_stat_req_flags);

#if defined(VTSS_SW_OPTION_VCLI) && defined(VTSS_SW_OPTION_PORT_POWER_SAVINGS)
    eee_cli_init();
#endif
    memset(&info, 0, sizeof(info));
    for (iport = 0; iport < VTSS_PORTS; iport++) {
      EEE_local_state_init(iport, &info, FALSE);
    }

    // Create our mutex. Don't release it until we're ready (i.e. in the EEE_thread()).
    critd_init(&EEE_crit, "eee_crit", VTSS_MODULE_ID_EEE, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);

    // We need a thread because we need to figure out whether the PHYs are EEE-capable, and
    // this querying must be postponed until the PHYs are ready.
    cyg_thread_create(THREAD_DEFAULT_PRIO,
                      EEE_thread,
                      0,
                      "EEE",
                      EEE_thread_stack,
                      sizeof(EEE_thread_stack),
                      &EEE_thread_handle,
                      &EEE_thread_block);
    cyg_thread_resume(EEE_thread_handle);
    break;

  case INIT_CMD_START:
    (void)vtss_chip_id_get(NULL, &EEE_chip_id);
    EEE_msg_init();
    break;

  case INIT_CMD_CONF_DEF:
    if (isid == VTSS_ISID_LOCAL || isid == VTSS_ISID_GLOBAL) {
      // Reset local configuration or global configuration.
      // No such configuration for this module.
    } else if (VTSS_ISID_LEGAL(isid)) {
      // Reset particular switch configuration
      EEE_CRIT_ENTER();
      EEE_read_from_flash(isid, TRUE);
      EEE_CRIT_EXIT();
    }
    break;


  case INIT_CMD_MASTER_UP:
    // Read our configuration.
    EEE_CRIT_ENTER();
    EEE_read_from_flash(VTSS_ISID_GLOBAL, FALSE);
    EEE_CRIT_EXIT();
    break;

  case INIT_CMD_MASTER_DOWN:
    break;

  case INIT_CMD_SWITCH_ADD:
    EEE_CRIT_ENTER();
    EEE_msg_tx_switch_conf(isid);
    EEE_msg_tx_switch_state_get(isid);
    EEE_CRIT_EXIT();
    break;

  case INIT_CMD_SWITCH_DEL:
    break;

  default:
    break;
  }

  // Call chip-specific initialization function.
  EEE_platform_init(data);

  return VTSS_OK;
}

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
