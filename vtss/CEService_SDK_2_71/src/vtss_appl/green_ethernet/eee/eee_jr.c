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

#include <cyg/hal/drv_api.h>    /* For cyg_drv_interrupt_XXX()        */
#include <cyg/kernel/kapi.h>    /* CYGNUM_HAL_INTERRUPT_BLK_ANA       */
#include "vtss_api_if_api.h"    /* For vtss_api_if_chip_count()       */
#include "main.h"               /* For vtss_init_data_t               */
#include "eee.h"                /* For internal EEE declarations      */
#include "eee_api.h"            /* For our own API decls.             */
#include "misc_api.h"           /* For iport2uport()                  */
#include "cli.h"                /* For cli_req_t and cli_printf()     */
#include "vtss_timer_api.h"     /* For timer support                  */

#define EEE_HOLD_OFF_FIRST_TIMEOUT_MS         10 /**< First hold-off time when everything is up and running.  */
#define EEE_HOLD_OFF_SUBSEQUENT_TIMEOUTS_MS   10 /**< Subsequent hold-off times                               */
#define EEE_HOLD_OFF_LINK_UP_TIMEOUT_MS     1000 /**< First hold-off time when linking up or enabling EEE.    */
#define EEE_OQS_THRES                          0 /**< By setting it to 0, it will work as urgent queues. Lu26 sets it to 3000 bytes for non-urgent queues, but this won't work in this current implementation due to missing high-performance timer ticks */

/**
 * A couple of functions are fiddling with the state in DSR context. Only one function can
 * update the state at a time.
 */
/*lint -sem(EEE_ana_dsr,                thread_protected) */
/*lint -sem(EEE_hold_off_timer_expired, thread_protected) */
/*lint -sem(EEE_wake_up_timer_expired,  thread_protected) */

//************************************************
// Global Variables
//************************************************
static cyg_interrupt     EEE_ana_interrupt_object[2];
static cyg_handle_t      EEE_ana_interrupt_handle[2];
static cyg_flag_t        EEE_thread_flag;       // Used to wake up EEE_platform_thread_run().
static u32               EEE_port_cnt;

typedef enum {
  EEE_STATE_DISABLED,
  EEE_STATE_POWERING_DOWN,
  EEE_STATE_POWERED_DOWN,
  EEE_STATE_POWERING_UP,
  EEE_STATE_JUST_POWERED_UP,
  EEE_STATE_POWERED_UP
} EEE_state_enum_t;

// Internal per-port state.
typedef struct {
  EEE_state_enum_t          state;
  EEE_state_enum_t          state_next; // Only valid if #state == EEE_STATE_POWERING_UP
  BOOL                      admin;      // Only valid if #state == EEE_STATE_POWERING_UP
  vtss_timer_t              hold_off_timer;
  vtss_timer_t              wake_up_timer;
  u32                       ctr;
  u64                       power_up_cnt;
  u64                       power_down_cnt;
  u64                       wake_up_time_us;
  u64                       mtu_tx_time_us;
  u64                       link_change_cnt;
} EEE_port_state_t;

static u64              EEE_ana_l2_interrupt_cnt;
static EEE_port_state_t EEE_port_state[VTSS_PORTS];

#define EEE_INLINE inline

/****************************************************************************/
//
// Private functions
//
/****************************************************************************/

/****************************************************************************/
// EEE_assert_low_power_idle()
/****************************************************************************/
static EEE_INLINE void EEE_low_power_idle_set(vtss_port_no_t port, BOOL value)
{
  static vtss_eee_port_state_t lpi_state = {.select = VTSS_EEE_STATE_SELECT_LPI};
  lpi_state.val = value;
  (void)vtss_eee_port_state_set(NULL, port, &lpi_state);
}

/****************************************************************************/
// EEE_scheduler_set()
/****************************************************************************/
static EEE_INLINE void EEE_scheduler_set(vtss_port_no_t port, BOOL enable)
{
  static vtss_eee_port_state_t sch_state = {.select = VTSS_EEE_STATE_SELECT_SCH};
  sch_state.val = enable;
  (void)vtss_eee_port_state_set(NULL, port, &sch_state);
}

/****************************************************************************/
// EEE_read_port_ctr()
/****************************************************************************/
static EEE_INLINE u32 EEE_read_port_ctr(vtss_port_no_t port)
{
  static vtss_eee_port_counter_t eee_tx_counter = {.tx_out_bytes_get = TRUE};
  if (vtss_eee_port_counter_get(NULL, port, &eee_tx_counter) != VTSS_RC_OK) {
    T_EG(TRACE_GRP_IRQ, "Couldn't get port tx out counters (port = %u)", port);
    return 0;
  }
  return eee_tx_counter.tx_out_bytes;
}

/****************************************************************************/
// EEE_exceeds_fill_level()
/****************************************************************************/
static EEE_INLINE BOOL EEE_exceeds_fill_level(vtss_port_no_t port)
{
  static vtss_eee_port_counter_t eee_fill_level_counter = {.fill_level_get = TRUE, .fill_level_thres = EEE_OQS_THRES};
  if (vtss_eee_port_counter_get(NULL, port, &eee_fill_level_counter) != VTSS_RC_OK) {
    T_EG(TRACE_GRP_IRQ, "Couldn't get port fill level (port = %u)", port);
    return FALSE;
  }
  return eee_fill_level_counter.fill_level > EEE_OQS_THRES;
}

/****************************************************************************/
// EEE_frame_present_interrupt_set()
/****************************************************************************/
static EEE_INLINE void EEE_frame_present_interrupt_set(vtss_port_no_t port, BOOL value)
{
  static vtss_eee_port_state_t fp_state = {.select = VTSS_EEE_STATE_SELECT_FP};
  fp_state.val = value;
  (void)vtss_eee_port_state_set(NULL, port, &fp_state);
}

/****************************************************************************/
// EEE_hold_off_timer_start()
// If #administratively == TRUE, we start the hold-off timer as a result of
// either a link up or administratively enabling the port. In that case we
// cannot go to LPI mode until one second has elapsed because that would
// stress the receiving PHY's DSP (EEE cluase 22.6a.1, 35.3a.1 and 78.1.2.1.2).
// If #administratively == FALSE, it's the normal state machine that turns
// it on or off.
/****************************************************************************/
static EEE_INLINE void EEE_hold_off_timer_start(vtss_port_no_t port, BOOL administratively, BOOL first_time)
{
  vtss_timer_t *timer = &EEE_port_state[port].hold_off_timer;
  if (timer->flags & 0x2) {
    T_EG(TRACE_GRP_IRQ, "Hold-off timer for %u is already active", port);
    (void)vtss_timer_cancel(timer);
  }
  // The very first time after linkup or management enabling of EEE, we have to wait
  // one second before allowing going into LPI mode due to some PHY trouble.
  timer->period_us = administratively ? EEE_HOLD_OFF_LINK_UP_TIMEOUT_MS * 1000 : (first_time ? EEE_HOLD_OFF_FIRST_TIMEOUT_MS * 1000 : EEE_HOLD_OFF_SUBSEQUENT_TIMEOUTS_MS * 1000);
  timer->repeat = !first_time;
  if (vtss_timer_start(timer) != VTSS_RC_OK) {
    T_EG(TRACE_GRP_IRQ, "Unable to start hold-off timer for %u", port);
  }
}

/****************************************************************************/
// EEE_wake_up_timer_expired()
/****************************************************************************/
static void EEE_wake_up_timer_expired(vtss_timer_t *timer)
{
  vtss_port_no_t port = (vtss_port_no_t)timer->user_data;
  if (EEE_port_state[port].state == EEE_STATE_POWERING_DOWN) {
    // Now an MTU is definitely sent. Time to put PHY asleep.
    EEE_low_power_idle_set(port, TRUE); // Put PHY asleep
    EEE_port_state[port].state = EEE_STATE_POWERED_DOWN;
    EEE_port_state[port].power_down_cnt++;
  } else if (EEE_port_state[port].state == EEE_STATE_POWERING_UP) {
    if (EEE_port_state[port].state_next == EEE_STATE_JUST_POWERED_UP) {
      // Called as a result of either administratively enabling of EEE or link up (both => admin == TRUE),
      // or as a result of frames in the queue (admin == FALSE).
      EEE_hold_off_timer_start(port, EEE_port_state[port].admin, TRUE);
      EEE_port_state[port].power_up_cnt++;
    } else {
      // Administrative disabling of EEE on this port.
      EEE_port_state[port].state = EEE_STATE_DISABLED;
    }
    EEE_port_state[port].state = EEE_port_state[port].state_next;
    EEE_scheduler_set(port, TRUE); // Start scheduler
  }
}

/****************************************************************************/
// EEE_wake_up_timer_start()
/****************************************************************************/
static EEE_INLINE void EEE_wake_up_timer_start(vtss_port_no_t port, u32 period_us)
{
  vtss_timer_t *timer = &EEE_port_state[port].wake_up_timer;
  if (timer->flags & 0x2) {
    // T_EG(TRACE_GRP_IRQ, "Wake-up timer for %lu is already active", port);
    (void)vtss_timer_cancel(timer);
  }
  timer->period_us = period_us;
  timer->repeat    = FALSE;
  if (vtss_timer_start(timer) != VTSS_RC_OK) {
    T_EG(TRACE_GRP_IRQ, "Unable to start wake-up timer for %u", port);
  }
}

/****************************************************************************/
// EEE_hold_off_timer_cancel()
/****************************************************************************/
static EEE_INLINE void EEE_hold_off_timer_cancel(vtss_port_no_t port)
{
  (void)vtss_timer_cancel(&EEE_port_state[port].hold_off_timer);
}

/****************************************************************************/
// EEE_power_down()
/****************************************************************************/
static EEE_INLINE void EEE_power_down(vtss_port_no_t port)
{
  T_DG(TRACE_GRP_IRQ, " ");
  EEE_scheduler_set(port, FALSE); // Stop scheduler
  EEE_port_state[port].state = EEE_STATE_POWERING_DOWN;
  // Gotta enable frame present interrupt before we sleep for X microseconds,
  // so that we don't lose new frames.
  EEE_frame_present_interrupt_set(port, TRUE);

  // Sleep MTU size frame @ link speed before powering down the PHY.
  EEE_wake_up_timer_start(port, EEE_port_state[port].mtu_tx_time_us);
  // The remaining is done in EEE_wake_up_timer_expired().
}

/****************************************************************************/
// EEE_hold_off_timer_expired()
/****************************************************************************/
static void EEE_hold_off_timer_expired(vtss_timer_t *timer)
{
  vtss_port_no_t port = (vtss_port_no_t)timer->user_data;
  if (EEE_port_state[port].state == EEE_STATE_JUST_POWERED_UP) {
    EEE_port_state[port].ctr = EEE_read_port_ctr(port);
    EEE_port_state[port].state = EEE_STATE_POWERED_UP;
    EEE_hold_off_timer_start(port, FALSE, FALSE);
  } else if (EEE_port_state[port].state == EEE_STATE_POWERED_UP) {
    u32 new_ctr;
    new_ctr = EEE_read_port_ctr(port);
    if (new_ctr == EEE_port_state[port].ctr) {
      EEE_hold_off_timer_cancel(port);
      EEE_power_down(port);
    } else {
      EEE_port_state[port].ctr = new_ctr;
    }
  } else {
    // No idea why we got here.
    EEE_hold_off_timer_cancel(port);
  }
}

/****************************************************************************/
// EEE_wake_up_timer_init()
/****************************************************************************/
static EEE_INLINE void EEE_wake_up_timer_init(void)
{
  vtss_port_no_t port;
  for (port = 0; port < EEE_port_cnt; port++) {
    if (EEE_local_state.port[port].eee_capable) {
      // Create wake-up timers
      vtss_timer_t *timer = &EEE_port_state[port].wake_up_timer;
      (void)vtss_timer_initialize(timer);
      timer->callback    = EEE_wake_up_timer_expired;
      timer->dsr_context = TRUE;
      timer->prio        = VTSS_TIMER_PRIO_HIGH;
      timer->modid       = VTSS_MODULE_ID_EEE;
      timer->user_data   = (void *)port;
    }
  }
}

/****************************************************************************/
// EEE_hold_off_timer_init()
/****************************************************************************/
static EEE_INLINE void EEE_hold_off_timer_init(void)
{
  vtss_port_no_t port;
  for (port = 0; port < EEE_port_cnt; port++) {
    if (EEE_local_state.port[port].eee_capable) {
      // Create hold-off timers
      vtss_timer_t *timer = &EEE_port_state[port].hold_off_timer;
      (void)vtss_timer_initialize(timer);
      timer->callback    = EEE_hold_off_timer_expired;
      timer->dsr_context = TRUE;
      timer->prio        = VTSS_TIMER_PRIO_HIGH;
      timer->modid       = VTSS_MODULE_ID_EEE;
      timer->user_data   = (void *)port;
    }
  }
}

/****************************************************************************/
// EEE_state2str()
/****************************************************************************/
static char *EEE_state2str(EEE_state_enum_t state)
{
  switch (state) {
  case EEE_STATE_DISABLED:
    return "Disabled";
  case EEE_STATE_POWERING_DOWN:
    return "Powering Down";
  case EEE_STATE_POWERED_DOWN:
    return "Powered Down";
  case EEE_STATE_POWERING_UP:
    return "Powering Up";
  case EEE_STATE_JUST_POWERED_UP:
    return "Just Powered Up";
  case EEE_STATE_POWERED_UP:
    return "Powered Up";
  default:
    return "UNKNOWN";
  }
}

/****************************************************************************/
// EEE_power_up()
/****************************************************************************/
static void EEE_power_up(vtss_port_no_t port, BOOL start_hold_off_timer, BOOL administratively)
{
  EEE_low_power_idle_set(port, FALSE); // Wake-up PHY
  EEE_frame_present_interrupt_set(port, FALSE); // Disable frame-present interrupts
  EEE_port_state[port].state = EEE_STATE_POWERING_UP;
  if (start_hold_off_timer) {
    // Called as a result of frames in a queue on this port if administratively == FALSE.
    // Called as a result of link-up on the port if administratively == TRUE.
    // The only difference is whether we simply set LPI mode enable as soon as we detect that
    // there's no traffic on a port (administratively == FALSE), or wait one second before
    // we allow setting LPI mode enable (administratively == TRUE), due to some PHY stuff.
    EEE_port_state[port].state_next = EEE_STATE_JUST_POWERED_UP;
    EEE_port_state[port].admin      = administratively;
  } else {
    EEE_hold_off_timer_cancel(port);
    EEE_port_state[port].state_next = EEE_STATE_DISABLED;
  }

  EEE_wake_up_timer_start(port, EEE_port_state[port].wake_up_time_us);
  // Remaining is done in EEE_wake_up_timer_expired().
}

/****************************************************************************/
// EEE_ana_isr()
// Context:
//   ISR (upper half)
// Description:
//   Called back in ISR context whenever an analyzer interrupt occurs.
/****************************************************************************/
static cyg_uint32 EEE_ana_isr(cyg_vector_t vector, cyg_addrword_t data)
{
  vtss_chip_no_t chip_no;
  vtss_eee_port_state_t ana_state;

  cyg_drv_interrupt_mask(vector); // Block this interrupt until the dsr completes

  chip_no = vector == CYGNUM_HAL_INTERRUPT_BLK_ANA  ? 0 : 1;

  ana_state.select = VTSS_EEE_STATE_SELECT_INTR_ACK;
  ana_state.val    = chip_no;
  (void)vtss_eee_port_state_set(NULL, VTSS_PORT_NO_NONE, &ana_state);

  // And then in interrupt controller
  cyg_drv_interrupt_acknowledge(vector);       // Tell eCos to allow further interrupt processing
  return (CYG_ISR_HANDLED | CYG_ISR_CALL_DSR); // Call the DSR
}

/****************************************************************************/
// EEE_ana_dsr()
// Context:
//   DSR (lower half)
// Description:
//   Called back in DSR context whenever an analyzer interrupt occurs.
/****************************************************************************/
static void EEE_ana_dsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
  vtss_port_no_t port;

  EEE_ana_l2_interrupt_cnt++;

  T_DG(TRACE_GRP_IRQ, "Enter");

  for (port = 0; port < EEE_port_cnt; port++) {
    if (EEE_port_state[port].state == EEE_STATE_POWERING_DOWN || EEE_port_state[port].state == EEE_STATE_POWERED_DOWN) {
      if (EEE_exceeds_fill_level(port)) {
        EEE_power_up(port, TRUE, FALSE);
      }
    }
  }

  T_DG(TRACE_GRP_IRQ, "Exit");

  // Allow interrupts to happen again
  cyg_drv_interrupt_unmask(vector);
}

/****************************************************************************/
// EEE_ana_interrupts_init()
/****************************************************************************/
static EEE_INLINE void EEE_ana_interrupts_init(void)
{
  vtss_chip_no_t        chip_no;
  u32                   chip_cnt = vtss_api_if_chip_count();
  vtss_eee_port_state_t eee_state;

  // Hook Analyzer interrupts
  for (chip_no = 0; chip_no < chip_cnt; chip_no++) {
#if defined(CYG_HAL_MIPS_VCOREIII_DUAL_JAGUAR)
    cyg_vector_t vector = chip_no == 0 ? CYGNUM_HAL_INTERRUPT_BLK_ANA : CYGNUM_HAL_INTERRUPT_SEC_BLK_ANA;
#else
    cyg_vector_t vector = CYGNUM_HAL_INTERRUPT_BLK_ANA; /* Only one chip */
#endif
    cyg_drv_interrupt_create(
      vector,                       // Interrupt vector
      0,                            // Interrupt Priority
      (cyg_addrword_t)NULL,
      EEE_ana_isr,
      EEE_ana_dsr,
      &EEE_ana_interrupt_handle[chip_no],
      &EEE_ana_interrupt_object[chip_no]
    );

    cyg_drv_interrupt_attach(EEE_ana_interrupt_handle[chip_no]);

    eee_state.select = VTSS_EEE_STATE_SELECT_INTR_ENA;
    eee_state.val    = chip_no;
    (void)vtss_eee_port_state_set(NULL, VTSS_PORT_NO_NONE, &eee_state);

    // Enable analyzer interrupts in interrupt controller.
    cyg_drv_interrupt_unmask(vector);
  }
}

/******************************************************************************/
// eee_cli_cmd_dbg_details()
/******************************************************************************/
void eee_cli_cmd_dbg_details(cli_req_t *req)
{
  vtss_port_no_t iport;
  u16            phy_val;
  u32            cnt = 0;
  u64            ana_intr_cnt;

  cyg_scheduler_lock();
  ana_intr_cnt = EEE_ana_l2_interrupt_cnt;
  cyg_scheduler_unlock();

  cli_printf("Analyzer Interrupts: %llu\n", ana_intr_cnt);
  cli_table_header("Port  State            LinkChgCount   PowerUpCount   PowerDownCount  PHY PCS Status");

  EEE_CRIT_ENTER();
  for (iport = 0; iport < VTSS_PORTS; iport++) {
    if (EEE_local_conf.port[iport].eee_ena && EEE_local_state.port[iport].eee_capable) {
      cnt++;
      (void)vtss_phy_mmd_read(NULL, iport, 3, 1, &phy_val);
      cli_printf("%4u  %15s %14llu  %14llu  %14llu  0x%04x\n", iport2uport(iport), EEE_state2str(EEE_port_state[iport].state), EEE_port_state[iport].link_change_cnt, EEE_port_state[iport].power_up_cnt, EEE_port_state[iport].power_down_cnt, phy_val);
    }
  }
  EEE_CRIT_EXIT();

  if (cnt == 0) {
    cli_printf("<No EEE-enabled ports>\n");
  }
  cli_printf("\n");
}

/******************************************************************************/
//
// SEMI-PUBLIC FUNCTIONS, I.E. INTERFACE TOWARDS PLATFORM-INDEPENDENT CODE.
//
/******************************************************************************/

/******************************************************************************/
// EEE_platform_thread()
/******************************************************************************/
void EEE_platform_thread(void)
{
  port_iter_t    pit;
  vtss_port_no_t iport;
  EEE_port_cnt = port_isid_port_count(VTSS_ISID_LOCAL);

  for (iport = 0; iport < EEE_port_cnt; iport++) {
    // Set the default wake-up time, which may be overridden through LLDP.
    EEE_port_state[iport].wake_up_time_us = EEE_local_state.port[iport].LocResolvedTxSystemValue;
  }

  // Create wake-up timers.
  EEE_wake_up_timer_init();

  // Initialize hold-off timers
  EEE_hold_off_timer_init();

  // Hook Analyzer interrupts
  EEE_ana_interrupts_init();

  // First time we exit this.
  /*lint -e(455) */
  EEE_CRIT_EXIT();

  while (1) {
    (void)cyg_flag_wait(&EEE_thread_flag, 0xFFFFFFFF, CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR);
    (void)port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);

    EEE_CRIT_ENTER();
    while (port_iter_getnext(&pit)) {
      if (EEE_local_state.port[pit.iport].eee_capable) {
        // To be thread safe we must only test against state == EEE_STATE_DISABLED.
        if (EEE_local_state.port[pit.iport].running && EEE_port_state[pit.iport].state == EEE_STATE_DISABLED) {
          cyg_scheduler_lock();
          T_DG(TRACE_GRP_IRQ, " ");
          // Power-up with at least a one-second timeout before setting it into low-power-idle.
          EEE_power_up(pit.iport, TRUE, TRUE);
          cyg_scheduler_unlock();
        } else if (EEE_port_state[pit.iport].state != EEE_STATE_DISABLED && EEE_local_state.port[pit.iport].running == FALSE) {
          // Disabling EEE.
          cyg_scheduler_lock();
          T_DG(TRACE_GRP_IRQ, " ");
          // Power-up without starting a hold-off timer, since we don't want to power down again.
          EEE_power_up(pit.iport, FALSE, TRUE);
          cyg_scheduler_unlock();
        }
      }
    }
    EEE_CRIT_EXIT();
  }
}

/******************************************************************************/
// EEE_platform_port_link_change_event()
/******************************************************************************/
void EEE_platform_port_link_change_event(vtss_port_no_t iport, port_info_t *info)
{
  EEE_CRIT_ASSERT_LOCKED();
  cyg_scheduler_lock();
  EEE_port_state[iport].mtu_tx_time_us = info->speed == VTSS_SPEED_100M ? 800 : 80;
  EEE_port_state[iport].link_change_cnt++;
  cyg_scheduler_unlock();
  cyg_flag_setbits(&EEE_thread_flag, 2); // 2 == Link change event, if we need it someday.
}

/******************************************************************************/
// EEE_platform_conf_reset()
/******************************************************************************/
void EEE_platform_conf_reset(eee_switch_conf_t *conf)
{
  // Nothing to be done (#conf already reset by caller)
}

/******************************************************************************/
// EEE_platform_conf_valid()
/******************************************************************************/
BOOL EEE_platform_conf_valid(eee_switch_conf_t *conf)
{
  return TRUE;
}

/******************************************************************************/
// EEE_platform_local_conf_set()
// Configuration received from master. EEE_local_conf already updated, and
// PHY advertisement set-up.
/******************************************************************************/
void EEE_platform_local_conf_set(BOOL *port_changes)
{
  vtss_eee_port_conf_t eee_port_conf;
  vtss_port_no_t       iport;

  memset(&eee_port_conf, 0, sizeof(eee_port_conf));

  EEE_CRIT_ENTER();
  for (iport = 0; iport < VTSS_PORTS; iport++) {
    if (port_changes[iport]) {
      eee_port_conf.eee_ena = EEE_local_conf.port[iport].eee_ena;
      if (vtss_eee_port_conf_set(NULL, iport, &eee_port_conf) != VTSS_RC_OK) {
        T_E("EEE: Failed to set port conf (port = %u)", iport2uport(iport));
      }
    }
  }
  cyg_flag_setbits(&EEE_thread_flag, 1); // 1 = enabledness state change.
  EEE_CRIT_EXIT();
}

/******************************************************************************/
// EEE_platform_tx_wakeup_time_changed()
/******************************************************************************/
void EEE_platform_tx_wakeup_time_changed(vtss_port_no_t port, u16 LocResolvedTxSystemValue)
{
  EEE_CRIT_ASSERT_LOCKED();
  cyg_scheduler_lock();
  EEE_port_state[port].wake_up_time_us = LocResolvedTxSystemValue;
  cyg_scheduler_unlock();
}

/******************************************************************************/
// EEE_platform_init()
/******************************************************************************/
void EEE_platform_init(vtss_init_data_t *data)
{
  if (data->cmd == INIT_CMD_INIT) {
    // Create a flag that can wake up EEE_platform_thread_run().
    cyg_flag_init(&EEE_thread_flag);
  }
}

