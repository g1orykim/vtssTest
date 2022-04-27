/*

 Vitesse API software.

 Copyright (c) 2002-2009 Vitesse Semiconductor Corporation "Vitesse". All
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

 $Id$
 $Revision$
*/

/**
 * \file vtss_pcs_10gbase_r_api.h
 * \brief  PCS_10BASE_R  API
 */

#ifndef _VTSS_PCS_10GBASE_R_API_H_
#define _VTSS_PCS_10GBASE_R_API_H_

#include <vtss_options.h>
#include <vtss_types.h>

#if defined(VTSS_FEATURE_PCS_10GBASE_R)

/* ================================================================= *
 *  Data structures / types
 * ================================================================= */

/** \brief PCS counter type  */
typedef u64 vtss_pcs_10gbase_r_counter_t;

/** \brief PCS test config data   */
typedef struct vtss_pcs_10gbase_r_test_cfg_t {
	/* tbd test pattern mode setup */
	/* u64 tx_seed_a; etc */
    u32 dummy; /**< to avoid lint error 43 */
} vtss_pcs_10gbase_r_test_cfg_t;

/** \brief PCS config data   */
typedef struct vtss_pcs_10gbase_r_cfg {
	BOOL tx_test_mode;              /**< Boolean variable controlling transmit channel operating mode. When false, the
                                         transmit channel operates in normal mode. When true, the transmit channel
                                         operates in test-pattern mode. */
	BOOL rx_test_mode;              /**< Boolean variable controlling receive channel operating mode. When false, the
                                         receive channel operates in normal mode. When true, the receive channel
                                         operates in test-pattern mode. */
	BOOL loopback;                  /**< In loopback mode the PCS accepts data from XGMII and returns it to the XGMII */
	vtss_pcs_10gbase_r_test_cfg_t test_cfg; /**< PCS test configuration */
} vtss_pcs_10gbase_r_cfg_t;

/** \brief pcs status   */
typedef struct vtss_pcs_10gbase_r_status {
	BOOL PCS_status;                /**< = block_lock && !hi_ber  (802.3: 49.2.14.1)*/
	BOOL block_lock;                /**< Indicates the state of the block_lock variable (802.3: 49.2.14.1). */
	BOOL hi_ber;                    /**< Indicates the state of the hi_ber variable (802.3: 49.2.14.1). */
	BOOL testpat_match;             /**< TRUE if test pattern checker detects a match in test mode */
} vtss_pcs_10gbase_r_status_t;

/** \brief pcs counters */
typedef struct vtss_pcs_10gbase_r_cnt {
    vtss_pcs_10gbase_r_counter_t ber_count;                /**< counts each time BER_BAD_SH state is entered */
    vtss_pcs_10gbase_r_counter_t rx_errored_block_count;   /**< counts once for each time RX_E state is entered. */
    vtss_pcs_10gbase_r_counter_t tx_errored_block_count;   /**< counts once for each time TX_E state is entered. */
    vtss_pcs_10gbase_r_counter_t test_pattern_error_count; /**< When the receiver is in test-pattern mode, the test_pattern_error_count counts
                                                                errors as described in 802.3: 49.2.12. */
} vtss_pcs_10gbase_r_cnt_t;



/** \brief  pcs events */
typedef enum vtss_pcs_10gbase_r_events_t {
	/* tbd RX_FIFO_ECC_2BIT =    (1 << 0), */
  RX_FIFO_ECC_NONE /* TBD*/
} vtss_pcs_10gbase_r_events_t;

/** \brief PCS vtss_state structure definitions */
typedef struct vtss_pcs_10gbase_r_cfg_state {
        vtss_pcs_10gbase_r_cfg_t   pcs_10gbase_r_cfg; /**< PCS 10base-R configuration*/
} vtss_pcs_10gbase_r_state_t;


/* ================================================================= *
 *  Defects/Events
 * ================================================================= */
/**
 * \brief PCS_10GBASE_R interrupt event mask values
 **/
#define VTSS_PCS_10GBASE_R_RX_HI_BER_EV             0x00000001  /**< High BER status changed */
#define VTSS_PCS_10GBASE_R_RX_FIFO_OFLOW_EV         0x00000002  /**< RX FIFO overflow status changed */
#define VTSS_PCS_10GBASE_R_RX_FIFO_UFLOW_EV         0x00000004  /**< RX FIFO underflow status changed */
#define VTSS_PCS_10GBASE_R_LOCK_CHANGED_EV          0x00000008  /**< Synchronization (lock) status changed */
#define VTSS_PCS_10GBASE_R_C64B66B_ERR_EV           0x00000010  /**< Invalid block detected */
#define VTSS_PCS_10GBASE_R_RX_OSET_EV               0x00000020  /**< RX ordered set captured in FIFO */
#define VTSS_PCS_10GBASE_R_RX_OSET_FIFO_FULL_EV     0x00000040  /**< RX ordered set FIFO full status changed */
#define VTSS_PCS_10GBASE_R_TX_FIFO_OFLOW_EV         0x00000080  /**< TX FIFO overflow status changed */
#define VTSS_PCS_10GBASE_R_TX_FIFO_UFLOW_EV         0x00000100  /**< TX FIFO underflow status changed */
#define VTSS_PCS_10GBASE_R_XGMII_ERR_EV             0x00000200  /**< Invalid Tx XGMII character is detected */
#define VTSS_PCS_10GBASE_R_RX_FSET_EV               0x00000400  /**< RX ordered set captured in FIFO*/
#define VTSS_PCS_10GBASE_R_RX_FSET_FIFO_FULL_EV     0x00000800  /**< RX ordered set FIFO full status changed */
#define VTSS_PCS_10GBASE_R_TX_FIFO_ECC_1BIT_EV      0x00001000  /**< Single-bit error is detected by the TX FIFO RAM's ECC logic */
#define VTSS_PCS_10GBASE_R_TX_FIFO_ECC_2BIT_EV      0x00002000  /**< Dual-bit error is detected by the TX FIFO RAM's ECC logic */
#define VTSS_PCS_10GBASE_R_RX_FIFO_ECC_1BIT_EV      0x00004000  /**< Single-bit error is detected by the RX FIFO RAM's ECC logic */
#define VTSS_PCS_10GBASE_R_RX_FIFO_ECC_2BIT_EV      0x00008000  /**< Dual-bit error is detected by the RX FIFO RAM's ECC logic */

typedef u32 vtss_pcs_10gbase_r_event_t; /**< Int events: Single event or 'OR' multiple events above */

/**
 * \brief Enable event generation for a specific event type or group of events
 *
 * \param inst    [IN]   Target instance reference.
 * \param port_no [IN]   Port number
 * \param enable  [IN]   Enable or disable events
 * \param ev_mask [IN]   Event type(s) to control (mask)
 *
 * \return Return code.
 **/

vtss_rc vtss_pcs_10gbase_r_event_enable(const vtss_inst_t                  inst,
                                        const vtss_port_no_t               port_no,
                                        const BOOL                         enable,
                                        const vtss_pcs_10gbase_r_event_t   ev_mask);

/**
 * \brief Polling function called at by interrupt or periodically
 * \note Interrupt status will be cleared on read
 *
 * \param inst    [IN]   Target instance reference.
 * \param port_no [IN]   Port number
 * \param status  [OUT]  Event status, bit set indicates corresponding
 *                       event/interrupt has detected
 *
 * \return Return code.
 **/

vtss_rc vtss_pcs_10gbase_r_event_poll(const vtss_inst_t             inst,
                                      const vtss_port_no_t          port_no,
                                      vtss_pcs_10gbase_r_event_t    *const status);


/**
 * \brief Polling function called at by interrupt or periodically
 * \note Interrupt status will be cleared on read
 *
 * \param inst    [IN]   Target instance reference.
 * \param port_no [IN]   Port number
 * \param status  [OUT]  Event status, bit set indicates corresponding
 *                       event/interrupt has detected irrespective of the mask register
 *
 * \return Return code.
 **/

vtss_rc vtss_pcs_10gbase_r_event_poll_without_mask(const vtss_inst_t             inst,
                                                   const vtss_port_no_t          port_no,
                                                   vtss_pcs_10gbase_r_event_t    *const status);


/* ================================================================= *
 *  Dynamic Config
 * ================================================================= */

/**
 * \brief   Set the PCS 10Gbase-R configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [IN]      pointer to configuration data.
 *
 * \return Return code.
 **/
vtss_rc vtss_pcs_10gbase_r_config_set(const vtss_inst_t inst,
                                      const vtss_port_no_t port_no,
                                      const vtss_pcs_10gbase_r_cfg_t *const cfg);

/**
 * \brief   Get the PCS 10Gbase-R configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [OUT]     pointer to configuration data.
 *
 * \return Return code.
 **/
vtss_rc vtss_pcs_10gbase_r_config_get(const vtss_inst_t inst,
                                      const vtss_port_no_t port_no,
                                      vtss_pcs_10gbase_r_cfg_t * const cfg);


/* ================================================================= *
 *  State Reporting
 * ================================================================= */

/**
 * \brief Get the PCS 10Gbase-R status.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param status [OUT]  pointer to defect status structure.
 *
 * \return Return code.
 **/
vtss_rc vtss_pcs_10gbase_r_status_get(const vtss_inst_t inst,
                                      const vtss_port_no_t port_no,
                                      vtss_pcs_10gbase_r_status_t *const status);


/* ================================================================= *
 *  Performance Primitives
 * ================================================================= */
/**
 * \brief Update the PCS 10Gbase-R counters.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 *
 * \return Return code.
 **/

vtss_rc vtss_pcs_10gbase_r_counters_update(const vtss_inst_t inst,
                                           const vtss_port_no_t port_no);

/**
 * \brief Clear PCS 10Gbase-R counters.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 *
 * \return Return code.
 **/

vtss_rc vtss_pcs_10gbase_r_counters_clear(const vtss_inst_t inst,
                                          const vtss_port_no_t port_no);

/**
 * \brief Get actual PCS 10Gbase-R counters.
 *
 * \param inst     [IN]   Target instance reference.
 * \param port_no  [IN]   Port number.
 * \param cnt [OUT]       Pointer to counter structure.
 *
 * \return Return code.
 **/
vtss_rc vtss_pcs_10gbase_r_counters_get(const vtss_inst_t inst,
                                        const vtss_port_no_t port_no,
                                        vtss_pcs_10gbase_r_cnt_t *const cnt);


#endif /* VTSS_FEATURE_PCS */

#endif /* _VTSS_PCS_10GBASE_R_API_H_ */
