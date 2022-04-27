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
 * \file vtss_xfi_api.h
 * \brief  XFI  API
 */

#ifndef _VTSS_XFI_API_H_
#define _VTSS_XFI_API_H_

#include <vtss_options.h>
#include <vtss_types.h>

#if defined(VTSS_FEATURE_XFI)

/* ================================================================= *
 *  Data structures / types
 * ================================================================= */

/** \brief xfi state - Recovered clock output attributes  */
typedef struct vtss_xfi_rec_clock_output_t {
    BOOL lane_a;                   /**< Status of the XFI rec clock output mode for lane A */
    BOOL lane_b;                   /**< Status of the XFI rec clock output mode for lane B */
} vtss_xfi_rec_clock_output_t;

/** \brief xfi config data   */
typedef struct vtss_xfi_cfg_t {
    BOOL sel_lane_b;                                      /**< TRUE => select lane B, FALSE => select lane A */
    BOOL equipment_loopback;                              /**< TRUE => select equipment loopback (TX->RX), FALSE => no loopback */
    BOOL line_loopback;                                   /**< TRUE => select line loopback (RX->TX), FALSE => no loopback */
    BOOL bypass_aneg;                                     /**< Bypass the ANEG (not supported in release 1.0) */
    BOOL bypass_ae;                                       /**< Bypass the AE (not supported in release 1.0) */
    vtss_xfi_rec_clock_output_t xfi_rec_clock_output;     /**< XFI Recovered clock output status  */
} vtss_xfi_cfg_t;

/** \brief xfi status   */
typedef struct vtss_xfi_status_t {

    BOOL ssf;                       /**< Status of the XFI_SSF output port */
    struct  {
        BOOL pll_lock;              /**< IFPLL locked indicator */
        BOOL hss_rst_out;           /**< Reset Output */
        BOOL hss_eyequality;        /**< Eye Quality Status */
        BOOL prt_rdy;               /**< Parallel Register Port Ready */
        BOOL tx_fifo_los;           /**< X_FIFO 'Loss of Sync' */
        BOOL ifpll_lock;            /**< IFPLL locked indicator */
        BOOL rx_a_sigdet;           /**< Lane-A Signal Detect indicator */
        BOOL rx_a_los;              /**< Current value of Lane-A XFI LOS pin */
        BOOL rx_b_sigdet;           /**< Lane-B Signal Detect indicator */
        BOOL rx_b_los;              /**< Current value of Lane-B XFI LOS pin */
    } hss_status;                   /**< High speed serdes status */
    struct {
        BOOL train_local;            /**< Training FSM is in the TRAIN_LOCAL state */
        BOOL train_ok;               /**< Training FSM is in the SEND_DATA state and the TRAIN_FAIL output is not asserted */
    } ae_status;                     /**< Adaptive equalization status */
    u16 internal_status_a;           /**< HSS11G RX Receiver Internal Status lane a */
    u16 internal_status_b;           /**< HSS11G RX Receiver Internal Status lane b */
} vtss_xfi_status_t;

/** \brief xfi lane    */
typedef enum  {
    xfi_lane_a,
    xfi_lane_b
} vtss_xfi_lane_t;

/** \brief XFI test pattern mode types.
 */
typedef enum  {
    VTSS_XFI_TEST_MODE_DISABLE,         /**< Disable test */
    VTSS_XFI_TEST_MODE_PRBS7_P,         /**< Enable PRBS7 generator/checker */
    VTSS_XFI_TEST_MODE_PRBS7_N,         /**< Enable PRBS7-inverted generator/checker */
    VTSS_XFI_TEST_MODE_PRBS23_P,        /**< Enable PRBS23 generator/checker */
    VTSS_XFI_TEST_MODE_PRBS23_N,        /**< Enable PRBS23-inverted generator/checker */
    VTSS_XFI_TEST_MODE_PRBS31_P,        /**< Enable PRBS31 generator/checker */
    VTSS_XFI_TEST_MODE_PRBS31_N,        /**< Enable PRBS31-inverted generator/checker */
    VTSS_XFI_TEST_MODE_HF,              /**< Enable 10101010.... generator */
    VTSS_XFI_TEST_MODE_LF,              /**< Enable 64*1;64*0;.... generator */
    VTSS_XFI_TEST_MODE_PRBS9_P,         /**< Enable PRBS9 generator */
    VTSS_XFI_TEST_MODE_MAX
} vtss_xfi_test_mode_t;

/** \brief xfi test configuration */
typedef struct vtss_xfi_test_cfg_s {
    vtss_xfi_test_mode_t test_cfg_gen;   /**< test pattern generation configuration  */
    vtss_xfi_test_mode_t test_cfg_chk;   /**< test pattern checker configuration  */
} vtss_xfi_test_cfg_t;

/** \brief xfi test status (pr lane) */
typedef struct vtss_xfi_test_status_s {
    BOOL lbsync[2];            /**< PRBS Sync Status  */
    BOOL lberror[2];           /**< PRBS Error Status  */
    u32  rx_prbs_err_count; /**< Counts assertions (leading edges) of the RXPRBSERR input (clear on read).*/
    BOOL rx_prbssync;       /**< RX PRBS synced indication  */
    BOOL rx_prbs_err;       /**< indicate RX PRBS error */
} vtss_xfi_test_status_t;

/** \brief ffe channel loss configurations */
typedef enum {
    xfi_chloss_lt10inch, /**< Less than 10 inch length  */
    xfi_chloss_gt10inch, /**< Greater than 10 inch length  */
    xfi_chloss_sfp_plus, /**< SFP Plus Channel  */
    xfi_chloss_copper_cable, /**< Copper cable */
    xfi_chloss_max,      /* Number of channel loss types */
} vtss_xfi_eq_chloss_t;

/** \brief ffe configuration mode */
typedef enum {
    XFI_TXEQ_TRADITIONAL_OVERRIDE, /**< Traditional mode with override*/
    XFI_TXEQ_TRADITIONAL,          /**< Traditional mode */
    XFI_TXEQ_8023AP_OVERRIDE,      /**< 802.3ap with manual override*/
    XFI_TXEQ_8023AP,               /**< 802.3ap */
    XFI_TXEQ_MODE_MAX,             /**< Maximum modes */
} vtss_xfi_txeq_mode_t;

/** \brief ffe coefficients */
typedef enum {
    txeq_ffe_coef_0,         /**< FFE Coefficient C0 */
    txeq_ffe_coef_1,         /**< FFE Coefficient C1 */
    txeq_ffe_coef_2,         /**< FFE Coefficient C2 */
    txeq_ffe_max_coef,       /**< Number of FFE Coefficients */
} vtss_xfi_ffe_coef_t;

/** \brief 802.3 ap ffe coefficients update request*/
typedef enum {
    txeq_8023ap_updreq_hold, /**< Hold */
    txeq_8023ap_updreq_incr, /**< Increment */
    txeq_8023ap_updreq_decr, /**< Decrement */
    txeq_8023ap_updreq_resv, /**< Reserved */
} vtss_txeq_8023ap_updreq_t;

/** \brief 802.3 ap ffe coefficients status*/
typedef enum {
    txeq_8023ap_stat_noupdt, /**< Coefficient not updated */
    txeq_8023ap_stat_updt,   /**< Coefficient updated */
    txeq_8023ap_stat_min,    /**< At minimum value */
    txeq_8023ap_stat_max,    /**< At maximum value */
} vtss_txeq_8023ap_status_t;

/** \brief sturucture to hold coefficient status*/
typedef struct vtss_txeq_8023ap_coef_stat_s{
   vtss_txeq_8023ap_status_t state_c0; /**< FFE Coefficient C0 status */
   vtss_txeq_8023ap_status_t state_c1; /**< FFE Coefficient C1 status */
   vtss_txeq_8023ap_status_t state_c2; /**< FFE Coefficient C2 status */
} vtss_txeq_8023ap_coef_stat_t;

/** \brief sturucture to hold transmit equalization configuration*/
typedef struct vtss_xfi_ffe_cfg_s {
    vtss_xfi_txeq_mode_t mode;       /**< Equalization mode */
    vtss_xfi_eq_chloss_t channel_loss; /**< Type of channel loss */
} vtss_xfi_txeq_cfg_t;

/** \brief transmit equalization configuration for channel */
typedef struct vtss_xfi_txeq_cfg_s {
    vtss_xfi_txeq_cfg_t txeq_cfg; /**< Equalization configuration */
    u32 transmit_power;           /**< Transmitter power */
    u32 slew_rate;                /**< Slew Rate of the output */
    u32 coefs[txeq_ffe_max_coef]; /**< Array to hold the coefficient values */
    BOOL polarity[txeq_ffe_max_coef]; /**< Array to hold the coefficient polarities */
} vtss_xfi_tx_cfg_t;

/** \brief dfe configuration for channel */
typedef struct vtss_xfi_dfe_cfg_s{
    vtss_xfi_eq_chloss_t channel_loss; /**< Channel loss type for receiver  */
} vtss_xfi_rxeq_cfg_t;


/** \brief xfi state - container for vtss_state    */
typedef struct vtss_xfi_state_t {
    vtss_xfi_cfg_t     xfi_cfg;         /**< Storage for XFI configuration */
    vtss_xfi_status_t  xfi_status;      /**< Storage for XFI status */
    vtss_xfi_test_cfg_t xfi_test_cfg;   /**< Storage for XFI Test configuration */
    vtss_xfi_tx_cfg_t  xfi_tx_cfg;      /**< Storage for XFI trasmit equalization configuration */
    vtss_xfi_rxeq_cfg_t xfi_rxeq_cfg;   /**< Storage for XFI receive equalization configuration */
} vtss_xfi_state_t;


/* ================================================================= *
 *  Defects/Events
 * ================================================================= */

/**
 * \brief XFI interrupt event mask values
 **/
#define VTSS_XFI_HSS_HSSPLLLOCK_EV            0x0000000000000001LL  /**< HSS HSS11G Macro HSSPLLLOCK ouptut port change was detected */
#define VTSS_XFI_HSS_HSSRESETOUT_EV           0x0000000000000002LL  /**< HSS HSS11G Macro HSSRESETOUT output port change was detected */
#define VTSS_XFI_HSS_HSSEYEQUALITY_EV         0x0000000000000004LL  /**< HSS HSS11G Macro HSSEYEQUALITY output port change was detected */
#define VTSS_XFI_HSS_HSSPRTREADY_EV           0x0000000000000008LL  /**< HSS HSS11G Macro HSSPRTREADY output port change was detected */
#define VTSS_XFI_HSS_RXPRBSSYNC_EV            0x0000000000000010LL  /**< HSS HSS11G Macro RXPRBSSYNC output port change was detected */
#define VTSS_XFI_HSS_RXPRBSERR_EV             0x0000000000000020LL  /**< HSS HSS11G Macro RXPRBSERR output port change was detected */
#define VTSS_XFI_HSS_RXASIGDET_EV             0x0000000000000040LL  /**< HSS HSS11G Macro RXASIGDET output port change was detected */
#define VTSS_XFI_HSS_RXA_LOS_EV               0x0000000000000080LL  /**< HSS XFI RXA_LOS input pin change was detected */
#define VTSS_XFI_HSS_IFPLL_LOCK_EV            0x0000000000000100LL  /**< HSS IFPLL_LOCK input pin change was detected */
#define VTSS_XFI_HSS_TX_FIFO_LOS_EV           0x0000000000000200LL  /**< HSS The TX_FIFO was detected asserted */
#define VTSS_XFI_HSS_RXBSIGDET_EV             0x0000000000000400LL  /**< HSS HSS11G Macro RXBSIGDET output port change was detected */
#define VTSS_XFI_HSS_RXB_LOS_EV               0x0000000000000800LL  /**< HSS XFI RXB_LOS input pin change was detected */

#define VTSS_XFI_AE_AET_RUN_B_EV              0x0000000000001000LL  /**< AE Lane-B AE Macros run status change change w */
#define VTSS_XFI_AE_AET_FRM_LCK_B_EV          0x0000000000002000LL  /**< AE Lane-B FSM_FRM_LCK status change bit w */
#define VTSS_XFI_AE_PCS_RX_READY_B_EV         0x0000000000004000LL  /**< AE Lane-B PMA_RX_READY && change FSM_TR_COMP w */
#define VTSS_XFI_AE_PCS_TX_READY_B_EV         0x0000000000008000LL  /**< AE Lane-B PMA_TX_READY && change FSM_TR_COMP w */
#define VTSS_XFI_AE_FEC_NEGOTIATED_B_EV       0x0000000000010000LL  /**< AE Lane-B Training FSM is in SEND_DATA state and (TXSTAT_FEC && RXSTAT_FEC && (TXREQ_FEC || RXREQ_FECchange )) w */
#define VTSS_XFI_AE_TRAIN_OK_B_EV             0x0000000000020000LL  /**< AE Lane-B Training FSM is in SEND_DATA state and TRAIN_FAIL output change is not */
#define VTSS_XFI_AE_TRAIN_FAIL_B_EV           0x0000000000040000LL  /**< AE Lane-B Training FSM is in change the TRAIN_FA */
#define VTSS_XFI_AE_TRAIN_LOCAL_B_EV          0x0000000000080000LL  /**< AE Lane-B Training FSM is in change the TRAIN_ */
#define VTSS_XFI_AE_AET_RUN_A_EV              0x0000000000100000LL  /**< AE Lane-A AE Macros run status change change w */
#define VTSS_XFI_AE_AET_FRM_LCK_A_EV          0x0000000000200000LL  /**< AE Lane-A FSM_FRM_LCK status change bit w */
#define VTSS_XFI_AE_PCS_RX_READY_A_EV         0x0000000000400000LL  /**< AE Lane-A PMA_RX_READY && change FSM_TR_COMP w */
#define VTSS_XFI_AE_PCS_TX_READY_A_EV         0x0000000000800000LL  /**< AE Lane-A PMA_TX_READY && change FSM_TR_COMP w */
#define VTSS_XFI_AE_FEC_NEGOTIATED_A_EV       0x0000000001000000LL  /**< AE Lane-A Training FSM is in SEND_DATA state and (TXSTAT_FEC && RXSTAT_FEC && (TXREQ_FEC || RXREQ_FECchange )) w */
#define VTSS_XFI_AE_TRAIN_OK_A_EV             0x0000000002000000LL  /**< AE Lane-A Training FSM is in SEND_DATA state and TRAIN_FAIL output change is not */
#define VTSS_XFI_AE_TRAIN_FAIL_A_EV           0x0000000004000000LL  /**< AE Lane-A Training FSM is in change the TRAIN_FA */
#define VTSS_XFI_AE_TRAIN_LOCAL_A_EV          0x0000000008000000LL  /**< AE Lane-A Training FSM is in change the TRAIN_ */

#define VTSS_XFI_OTN_FAS_POS_EV               0x0000000010000000LL  /**< OTN Framer indicates a change in the frame alignment position */
#define VTSS_XFI_OTN_LOM_EV                   0x0000000020000000LL  /**< OTN Framer indicates a change in LOM state */
#define VTSS_XFI_OTN_OOM_EV                   0x0000000040000000LL  /**< OTN Framer indicates a change in OOM state */
#define VTSS_XFI_OTN_LOF_EV                   0x0000000080000000LL  /**< OTN Framer indicates a change in LOF state */
#define VTSS_XFI_OTN_OOF_EV                   0x0000000100000000LL  /**< OTN Framer indicates a change in OOF state */

#define VTSS_XFI_ANEG_INCOMPATIBLE_LINK_EV    0x0000000200000000LL  /**< ANEG no compatible link was found */
#define VTSS_XFI_ANEG_PAR_DETECT_FAULT_EV     0x0000000400000000LL  /**< ANEG errors during parallel detection */
#define VTSS_XFI_ANEG_AN_GOOD_CHECK_EV        0x0000000800000000LL  /**< ANEG Arbitration FSM enters the AN_GOOD_CHECK state */
#define VTSS_XFI_ANEG_PAGE_RX_EV              0x0000001000000000LL  /**< ANEG new page has been received */
#define VTSS_XFI_ANEG_ANEG_COMPLETE_EV        0x0000002000000000LL  /**< ANEG the ANEG_COMPLETE status is set */

typedef u64 vtss_xfi_event_t; /**< Int events: Single event or 'OR' multiple events above */

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

vtss_rc vtss_xfi_event_enable(const vtss_inst_t         inst,
                              const vtss_port_no_t      port_no,
                              const BOOL                enable,
                              const vtss_xfi_event_t    ev_mask);

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

vtss_rc vtss_xfi_event_poll(const vtss_inst_t     inst,
                            const vtss_port_no_t  port_no,
                            vtss_xfi_event_t      *const status);


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

vtss_rc vtss_xfi_event_poll_without_mask(const vtss_inst_t     inst,
                                         const vtss_port_no_t  port_no,
                                         vtss_xfi_event_t      *const status);


/* ================================================================= *
 *  Dynamic Config
 * ================================================================= */

/**
 * \brief Set XFI configuration parameters (loopback etc.).
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 * \param cfg [IN]       XFI configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_xfi_conf_set(const vtss_inst_t inst,
                          const vtss_port_no_t port_no,
                          const vtss_xfi_cfg_t *const cfg);

/**
 * \brief Get XFI configuration parameters (loopback etc.).
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 * \param cfg [OUT]      XFI configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_xfi_conf_get(const vtss_inst_t inst,
                          const vtss_port_no_t port_no,
                          vtss_xfi_cfg_t * const cfg);

/**
 * \brief   Force AE lane reset  (not implemented in 0.9b).
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 * \param lane [IN]      Lane a or lane B.
 *
 * \return Return code.
 **/
vtss_rc vtss_xfi_ae_reset(const vtss_inst_t inst,
                          const vtss_port_no_t port_no,
                          const vtss_xfi_lane_t  lane);


/* ================================================================= *
 *  State Reporting
 * ================================================================= */

/**
 * \brief Get XFI interface status.
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 * \param status [OUT]   pointer to defect status structure.
 *
 * \return Return code.
 **/
vtss_rc vtss_xfi_status_get(const vtss_inst_t inst,
                            const vtss_port_no_t port_no,
                            vtss_xfi_status_t *const status);

/* ================================================================= *
 *  Performance Primitives
 * ================================================================= */
/* ================================================================= *
 *  Test/Debug Config
 * ================================================================= */

/**
 * \brief Set XFI Test configuration data.
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 * \param cfg [IN]       XFI test configuration
 *
 * \return Return code.
 **/
vtss_rc vtss_xfi_test_conf_set(const vtss_inst_t inst,
                          const vtss_port_no_t port_no,
                          const vtss_xfi_test_cfg_t *const cfg);

/**
 * \brief Get XFI Test configuration data.
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 * \param cfg [OUT]      XFI test configuration
 *
 * \return Return code.
 **/
vtss_rc vtss_xfi_test_conf_get(const vtss_inst_t inst,
                          const vtss_port_no_t port_no,
                          vtss_xfi_test_cfg_t *const cfg);

/**
 * \brief Get XFI Test status data.
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 * \param status [OUT]   XFI test status
 *
 * \return Return code.
 **/
vtss_rc vtss_xfi_test_status_get(const vtss_inst_t inst,
                          const vtss_port_no_t port_no,
                          vtss_xfi_test_status_t *const status);


/**
 * \brief Set XFI Recovered Clock Output status.
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 * \param status [IN]    XFI Recovered clock output status
 *
 * \return Return code.
 **/
vtss_rc vtss_xfi_rec_clock_output_set(const vtss_inst_t inst,
                                      const vtss_port_no_t port_no,
                                      vtss_xfi_rec_clock_output_t const *status);

/**
 * \brief Get XFI Recovered Clock Output status.
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 * \param status [OUT]   XFI Recovered clock output status
 *
 * \return Return code.
 **/
vtss_rc vtss_xfi_rec_clock_output_get(const vtss_inst_t inst,
                                      const vtss_port_no_t port_no,
                                      vtss_xfi_rec_clock_output_t *const status);

/**
 * \brief Set XFI Transmit equalization mode.
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 * \param cfg [IN]       Transmit Equalization configuration
 *
 * \return Return code.
 **/
vtss_rc vtss_xfi_txeq_mode_set(const vtss_inst_t inst,
                               const vtss_port_no_t port_no,
                               vtss_xfi_txeq_cfg_t const *cfg);

/**
 * \brief Get XFI Transmit equalization mode.
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 * \param cfg [IN]       Transmit Equalization configuration
 *
 * \return Return code.
 **/
vtss_rc vtss_xfi_txeq_mode_get(const vtss_inst_t inst,
                               const vtss_port_no_t port_no,
                               vtss_xfi_txeq_cfg_t *const cfg);

/**
 * \brief Fine tune the FFE coefficients in Traditional modes
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 * \param coef [IN]      FFE Coefficient
 * \param step_size [IN] Step size to increment or decrement coefficient
 * \param incr [IN]      Increment or Decrement coefficient
 * \param polarity [IN]  Polarity of the FFE coefficient
 *
 * \return Return code.
 **/

vtss_rc vtss_xfi_txeq_coef_adjust(const vtss_inst_t inst,
                                  const vtss_port_no_t port_no,
                                  vtss_xfi_ffe_coef_t coef,
                                  u32 step_size,
                                  BOOL incr,
                                  BOOL polarity);
/**
 * \brief Get the FFE coeffient values configured
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 * \param coefs [IN]     Array of coefficients
 * \param polarity [IN]     Array of polarity
 *
 * \return Return code.
 **/

vtss_rc vtss_xfi_txeq_coef_get(const vtss_inst_t inst,
                               const vtss_port_no_t port_no,
                               u32 coefs[], BOOL polarity[]);
/**
 * \brief Set the transmit power
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 * \param power_mVppd [IN]     Amplitude in range of 200 to 1200 milli volt peak to peak 
 *                       differential in 96 steps 0x20 to 0x7F
 *
 * \return Return code.
 **/
vtss_rc vtss_xfi_txamp_set(const vtss_inst_t inst,
                           const vtss_port_no_t port_no,
                           u32 power_mVppd);

/**
 * \brief Get the transmit power
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 * \param power_mVppd [IN]     Amplitude in range of 200 to 1200 milli volt peak to peak
 *                       differential in 96 steps 0x20 to 0x7F
 *
 * \return Return code.
 **/
vtss_rc vtss_xfi_txamp_get(const vtss_inst_t inst,
                           const vtss_port_no_t port_no,
                           u32 *power_mVppd);

/**
 * \brief Set the transmit slew rate 
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 * \param slew_rate [IN]     Output slew rate (0,3,5,7)
 * 000: 31 ps min. (default)
 * 101: 37 ps min
 * 011: 38 ps min
 * 111: 145 ps min
 * \return Return code.
 **/
vtss_rc vtss_xfi_txslew_set(const vtss_inst_t inst,
                           const vtss_port_no_t port_no,
                           u32 slew_rate);

/**
 * \brief Get the transmit slew rate
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 * \param slew_rate [IN]     Output slew rate (0,3,5,7)
 * 000: 31 ps min. (default)
 * 101: 37 ps min
 * 011: 38 ps min
 * 111: 145 ps min
 * \return Return code.
 **/
vtss_rc vtss_xfi_txslew_get(const vtss_inst_t inst,
                           const vtss_port_no_t port_no,
                           u32 *slew_rate);

/**
 * \brief Set XFI Receive equalization mode.
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 * \param cfg [IN]       Receive Equalization configuration
 *
 * \return Return code.
 **/
vtss_rc vtss_xfi_rxeq_mode_set(const vtss_inst_t inst,
                               const vtss_port_no_t port_no,
                               vtss_xfi_rxeq_cfg_t const *cfg);

/**
 * \brief Get XFI Receive equalization mode.
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 * \param cfg [OUT]      Receive Equalization configuration
 *
 * \return Return code.
 **/
vtss_rc vtss_xfi_rxeq_mode_get(const vtss_inst_t inst,
                               const vtss_port_no_t port_no,
                               vtss_xfi_rxeq_cfg_t *const cfg);


/**
 * \brief Set XFI equalization coefficient request.
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 * \param lane [IN]      Lane A/B.
 * \param coef [IN]      coef (c0,c1,c2) 
 * \param updreq [IN]    Update Request 
 *
 * \return Return code.
 **/
vtss_rc vtss_xfi_txeq_8023ap_coef_update(const vtss_inst_t inst, 
                                         const vtss_port_no_t port_no, 
                                         vtss_xfi_lane_t lane,
                                         vtss_xfi_ffe_coef_t coef,  
                                         vtss_txeq_8023ap_updreq_t updreq);
/**
 * \brief Get XFI equalization coefficient status.
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 * \param lane [IN]      Lane A/B.
 * \param ae_status [IN]       Adaptive Equalization Status status
 *
 * \return Return code.
 **/
vtss_rc vtss_xfi_txeq_8023ap_coef_stat_get(const vtss_inst_t inst, 
                                           const vtss_port_no_t port_no, 
                                           vtss_xfi_lane_t lane, vtss_txeq_8023ap_coef_stat_t *ae_status);


/**
 * \brief Get XFI equalization FSM control.
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 * \param lane [IN]      Lane A/B.
 * \param val [IN]       802.3ap FSM control
 *
 * \return Return code.
 **/
vtss_rc vtss_xfi_txeq_8023ap_fsm_ctl_set(const vtss_inst_t inst,
                                            const vtss_port_no_t port_no, 
                                            vtss_xfi_lane_t lane, u32 val);

/**
 * \brief Get XFI equalization FSM status.
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 * \param lane [IN]      Lane A/B.
 * \param val [IN]       802.3ap FSM status
 *
 * \return Return code.
 **/
vtss_rc vtss_xfi_txeq_8023ap_fsm_stat_get(const vtss_inst_t inst,
                                          const vtss_port_no_t port_no, 
                                          vtss_xfi_lane_t lane, u32 *val);


#endif /* VTSS_FEATURE_XFI */

#endif /* _VTSS_XFI_API_H_ */
