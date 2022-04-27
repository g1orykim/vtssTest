/*

 Vitesse Switch Software.

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
#include "lldp_os.h"
#include "lldp_sm.h"
#include "lldp.h"
#include "vtss_lldp.h"
#include "lldp_private.h"
#ifdef VTSS_SW_OPTION_LLDP_MED
#include "lldpmed_rx.h"
#endif

/* ************************************************************************ **
 *
 *
 * Defines
 *
 *
 *
 * ************************************************************************ */

/*
** Definitions for easing state machine implementation
*/

#define SM_STATE(machine, state) \
static void sm_ ## machine ## _ ## state ## _Enter(lldp_sm_t  * sm)

#define SM_ENTRY(machine, _state, _data) SM_ENTRY_DBG(machine, _state, _data)




#define SM_ENTRY_DBG(machine, _state, _data) \
if(sm->_data.state != _state){\
  T_N("Port %u " #machine " -> " #_state, (unsigned)sm->port_no); \
} \
sm->_data.state = _state;


#define SM_ENTER(machine, state) sm_ ## machine ## _ ## state ## _Enter(sm)

#define SM_STEP(machine) \
static void sm_ ## machine ## _Step(lldp_sm_t  *sm)

#define SM_STEP_RUN(machine) sm_ ## machine ## _Step(sm)



/*
** Misc action functions
*/
#define txInitializeLLDP() lldp_tx_initialize_lldp(sm)
#define mibConstrInfoLLDPDU() lldp_construct_info_lldpdu(sm->port_no,sm->sid)
#define mibConstrShutdownLLDPDU() lldp_construct_shutdown_lldpdu(sm->port_no)
#define txFrame() lldp_tx_frame(sm->port_no)
#define rxInitializeLLDP() lldp_rx_initialize_lldp (sm->port_no)
#define rxProcessFrame() lldp_rx_process_frame(sm)

/* These "functions" need not have a bodyas the MIB module fetches info from the lldp_remote module */
#define mibDeleteObjects() T_D("Port %u - mibDeleteObjects", (unsigned)sm->port_no);
#define mibUpdateObjects() T_D("Port %u - mibUpdateObjects", (unsigned)sm->port_no);

/* ************************************************************************ **
 *
 *
 * Typedefs and enums
 *
 *
 *
 * ************************************************************************ */

/* ************************************************************************ **
 *
 *
 * Prototypes for local functions
 *
 *
 *
 * ************************************************************************ */

/* ************************************************************************ **
 *
 *
 * Public data
 *
 *
 *
 * ************************************************************************ */

/* ************************************************************************ **
 *
 *
 * Local data
 *
 *
 *
 * ************************************************************************ */

/*
** in all of these functions, the variable sm is a pointer to the current
** set of state machine set
*/

SM_STATE(TX, TX_LLDP_INITIALIZE)
{
    SM_ENTRY(TX, TX_LLDP_INITIALIZE, tx);

    txInitializeLLDP();
}

SM_STATE(TX, TX_IDLE)
{
    lldp_timer_t msgTxInterval;

    SM_ENTRY(TX, TX_IDLE, tx);

    msgTxInterval = lldp_os_get_msg_tx_interval();

    lldp_u32_t txTTL = msgTxInterval * lldp_os_get_msg_tx_hold();
    if (txTTL >= 65535) {
        txTTL = 65535;
    }
    sm->tx.txTTL = (lldp_u16_t)txTTL;
    sm->timers.txTTR = msgTxInterval;
    sm->timers.txDelayWhile = lldp_os_get_tx_delay();
    sm->tx.re_evaluate_timers = LLDP_FALSE;

}

SM_STATE(TX, TX_SHUTDOWN_FRAME)
{
    SM_ENTRY(TX, TX_SHUTDOWN_FRAME, tx);

    mibConstrShutdownLLDPDU();
    txFrame();
    sm->timers.txShutdownWhile = lldp_os_get_reinit_delay();
}

SM_STATE(TX, TX_INFO_FRAME)
{
    SM_ENTRY(TX, TX_INFO_FRAME, tx);
    mibConstrInfoLLDPDU();
    txFrame();
}

SM_STEP(TX)
{
    if (sm->initialize || sm->portEnabled == LLDP_FALSE) {
        T_NG_PORT(TRACE_GRP_TX, sm->port_no, "sm->initialize:%d, sm->portEnabled:%d",
                  sm->initialize, sm->portEnabled);
        SM_ENTER(TX, TX_LLDP_INITIALIZE);
        return;
    }

    switch (sm->tx.state) {
    case TX_LLDP_INITIALIZE:
        sm->adminStatus = lldp_os_get_admin_status(sm->port_no);

        if ((sm->adminStatus == LLDP_ENABLED_RX_TX) || (sm->adminStatus == LLDP_ENABLED_TX_ONLY)) {
            SM_ENTER(TX, TX_IDLE);
            T_R("Setting next case state to TX_IDLE");
        }

        break;

    case TX_IDLE:
        sm->adminStatus =  lldp_os_get_admin_status(sm->port_no);

#ifdef VTSS_SW_OPTION_LLDP_MED
        // Figure 20:TX ILDE State, TIA1057
        if (lldpmed_medFastStart_timer_get(sm->port_no) != 0) {
            // Make sure that we still can do the count down of txDelayWhile
            // even though we do a fast start.
            if (sm->timers.txDelayWhile > 1) {
                T_NG_PORT(TRACE_GRP_TX, (vtss_port_no_t)sm->port_no, "Forceing DelayWhile to 1");
                sm->timers.txDelayWhile = 1; // Section 11.2.2, TIA1057
            }
        }
#endif

        T_DG_PORT(TRACE_GRP_TX, (vtss_port_no_t)sm->port_no,
                  "Entering TX_IDLE case, timers.txDelayWhile = %d, timers.txTTR = %d, somethingChangedLocal = %d ",
                  sm -> timers.txDelayWhile ,
                  sm -> timers.txTTR,
                  sm->tx.somethingChangedLocal);

        if ((sm->adminStatus == LLDP_DISABLED) || (sm->adminStatus == LLDP_ENABLED_RX_ONLY)) {
            SM_ENTER(TX, TX_SHUTDOWN_FRAME);
        } else if ((sm->timers.txDelayWhile == 0) && ((sm->timers.txTTR == 0) ||
#ifdef VTSS_SW_OPTION_LLDP_MED
                                                      // Figure 20, TIA1057, medFastStart != 0
                                                      (lldpmed_medFastStart_timer_get(sm->port_no) != 0) ||
#endif
                                                      (sm->tx.somethingChangedLocal == LLDP_TRUE))) {
            sm->tx.somethingChangedLocal = LLDP_FALSE;
            SM_ENTER(TX, TX_INFO_FRAME);

            T_RG_PORT(TRACE_GRP_TX, (vtss_port_no_t)sm->port_no, "Next state = TX_INFO_FRAME");

        } else if (sm->tx.re_evaluate_timers) {
            SM_ENTER(TX, TX_IDLE);
        }
        break;

    case TX_SHUTDOWN_FRAME:
        T_IG_PORT(TRACE_GRP_TX, sm->port_no, "Entering TX_SHUTDOWN_FRAME, sm->timers.txShutdownWhile = %d", sm->timers.txShutdownWhile);
        if (sm->timers.txShutdownWhile == 0) {
            SM_ENTER(TX, TX_LLDP_INITIALIZE);
        }
        break;

    case TX_INFO_FRAME:
        T_N("Entering TX_INFO case, port = %d", (unsigned) sm->port_no);
        /* UCT */
        SM_ENTER(TX, TX_IDLE);
        break;

    default:
        T_D("Port %u - Unhandled TX state %u", (unsigned)sm->port_no, (unsigned)sm->tx.state) ;
        break;
    }
}



SM_STATE(RX, LLDP_WAIT_PORT_OPERATIONAL)
{
    SM_ENTRY(RX, LLDP_WAIT_PORT_OPERATIONAL, rx);
}

SM_STATE(RX, DELETE_AGED_INFO)
{
    SM_ENTRY(RX, DELETE_AGED_INFO, rx);

    sm->rx.somethingChangedRemote = LLDP_FALSE;
    mibDeleteObjects();
    sm->rx.rxInfoAge = LLDP_FALSE;
    sm->rx.somethingChangedRemote = LLDP_TRUE;
}

SM_STATE(RX, RX_LLDP_INITIALIZE)
{
    SM_ENTRY(RX, RX_LLDP_INITIALIZE, rx);

    rxInitializeLLDP();
    sm->rx.rcvFrame = LLDP_FALSE;
}

SM_STATE(RX, RX_WAIT_FOR_FRAME)
{
    SM_ENTRY(RX, RX_WAIT_FOR_FRAME, rx);

    sm->rx.badFrame = LLDP_FALSE;
    sm->rx.rxInfoAge = LLDP_FALSE;
    sm->rx.somethingChangedRemote = LLDP_FALSE;
}

SM_STATE(RX, RX_FRAME)
{
    SM_ENTRY(RX, RX_FRAME, rx);

    sm->rx.badFrame = LLDP_FALSE;
    sm->rx.rxChanges = LLDP_FALSE;
    sm->rx.rcvFrame = LLDP_FALSE;
    rxProcessFrame();
}

SM_STATE(RX, DELETE_INFO)
{
    SM_ENTRY(RX, DELETE_INFO, rx);

    mibDeleteObjects();
    sm->rx.somethingChangedRemote = LLDP_TRUE;
}

SM_STATE(RX, UPDATE_INFO)
{
    SM_ENTRY(RX, UPDATE_INFO, rx);

    mibUpdateObjects();
    sm->rx.somethingChangedRemote = LLDP_TRUE;
}

SM_STEP(RX)
{
    if (sm->initialize || ((sm->rx.rxInfoAge == LLDP_FALSE) && (sm->portEnabled == LLDP_FALSE))) {
        SM_ENTER(RX, LLDP_WAIT_PORT_OPERATIONAL);
        return;
    }

    switch (sm->rx.state) {
    case LLDP_WAIT_PORT_OPERATIONAL:
        T_D("LLDP_WAIT_PORT_OPERATIONAL");
        if (sm->rx.rxInfoAge == LLDP_TRUE) {
            SM_ENTER(RX, DELETE_AGED_INFO);
        } else {
            SM_ENTER(RX, RX_LLDP_INITIALIZE);
        }
        break;

    case DELETE_AGED_INFO:
        T_D("DELETE_AGED_INFO");
        /* UCT */
        SM_ENTER(RX, LLDP_WAIT_PORT_OPERATIONAL);
        break;

    case RX_LLDP_INITIALIZE:
        sm->adminStatus = lldp_os_get_admin_status(sm->port_no);
        if ((sm->adminStatus == LLDP_ENABLED_RX_TX) ||
            (sm->adminStatus == LLDP_ENABLED_RX_ONLY)) {
            SM_ENTER(RX, RX_WAIT_FOR_FRAME);
        }
        /* IEEE802.1AB doesn't specifically specify that we shall clear rcvFrame, but in practise
        ** if we don't do it, we might end up trying to read and parse arbitrary input data
        ** because the rcvFrame is stuck with being TRUE until rx is enabled */
        else {
            /* this effectively throws away "old frame" when rx is disabled */
            sm->rx.rcvFrame = LLDP_FALSE;
        }
        break;

    case RX_WAIT_FOR_FRAME:
        sm->adminStatus =  lldp_os_get_admin_status(sm->port_no);
        T_R("RX_WAIT_FOR_FRAME");
        if (sm->rx.rxInfoAge == LLDP_TRUE) {
            SM_ENTER(RX, DELETE_INFO);
        } else if (sm->rx.rcvFrame == LLDP_TRUE) {
            SM_ENTER(RX, RX_FRAME);
        } else if ((sm->adminStatus == LLDP_DISABLED) ||
                   (sm->adminStatus == LLDP_ENABLED_TX_ONLY)) {
            SM_ENTER(RX, RX_LLDP_INITIALIZE);
        }
        break;

    case RX_FRAME:
        T_DG(TRACE_GRP_RX, "RX_FRAME");
        if (sm->rx.rxTTL == 0) {
            SM_ENTER(RX, DELETE_INFO);
        } else if ((sm->rx.rxTTL != 0) && (sm->rx.rxChanges == LLDP_TRUE)) {
            SM_ENTER(RX, UPDATE_INFO);
        } else if ((sm->rx.badFrame == LLDP_TRUE) || ((sm->rx.rxTTL != 0) && (sm->rx.rxChanges == LLDP_FALSE))) {
            SM_ENTER(RX, RX_WAIT_FOR_FRAME);
        }
        break;

    case UPDATE_INFO:
        T_D("UPDATE_INFO");
        /* UCT */
        SM_ENTER(RX, RX_WAIT_FOR_FRAME);
        break;

    case DELETE_INFO:
        T_D("DELETE_INFO");
        /* UCT */
        SM_ENTER(RX, RX_WAIT_FOR_FRAME);
        break;

    default:
        T_D("port %u - Unhandled RX state %u", (unsigned)sm->port_no, (unsigned)sm->rx.state);
        break;
    }
}

/*
** Step state machines
// IN : sm - Pointer to the state machine
//      rx_only - True if only the RX state machine shall be updated.
*/
void lldp_sm_step (lldp_sm_t *sm, BOOL rx_only)
{
    lldp_u8_t prev_tx;
    lldp_u8_t prev_rx;
    do {
        /* register previous states of state machines */
        prev_tx = (lldp_u8_t)sm->tx.state;
        prev_rx = (lldp_u8_t)sm->rx.state;

        /* run state machines */
        if (!rx_only) {
            SM_STEP_RUN(TX);
        }
        SM_STEP_RUN(RX);

        /* repeat until no changes */
    } while ( prev_tx != (lldp_u8_t)sm->tx.state ||
              prev_rx != (lldp_u8_t)sm->rx.state );
}

void lldp_sm_init (lldp_sm_t   *sm, lldp_port_t port)
{
    sm->port_no = port;
    sm->adminStatus = lldp_os_get_admin_status(port);
    T_N("Port %u - initializing, admin_status = %d", (unsigned)port, sm->adminStatus);

    /*
    ** set initialize, and step machine single time before de-asserting initialize
    */

    sm->initialize = LLDP_TRUE;
    lldp_sm_step(sm, FALSE); // Update both state machines
    sm->initialize = LLDP_FALSE;
}

void lldp_sm_timers_tick(lldp_sm_t  *sm)
{
    /* Timer handling */
//    T_R("Entering lldp_port_timers_tick");
    if (sm->timers.txShutdownWhile > 0) {
        sm->timers.txShutdownWhile--;
    }

    if (sm->timers.txDelayWhile > 0) {
        sm->timers.txDelayWhile--;
    }

    if (sm->timers.txTTR > 0) {
        sm->timers.txTTR--;
    }


    /* step state machines */
    lldp_sm_step(sm, FALSE); // Update both rx and tx state machines


#ifdef VTSS_SW_OPTION_LLDP_MED
    lldpmed_medFastStart_timer_action(sm->port_no, 0, DECREMENT); // medFastStart timer in seconds.
#endif
}
