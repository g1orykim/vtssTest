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
 * \file vtss_upi_api.h
 * \brief  Define UPI API interface
 */

#ifndef _VTSS_UPI_API_H_
#define _VTSS_UPI_API_H_

#include <vtss_options.h>
#include <vtss_types.h>

#if defined(VTSS_FEATURE_UPI)

/* ================================================================= *
 *  Data structures / types
 * ================================================================= */

/** \brief  UPI interface protocol selection. */
typedef enum vtss_upi_protocol_t {
    tfi5,                   /**< TFI-5 protocol mode */
    xaui,                   /**< XAUI protocol mode */
    sfi4                    /**< SFI-4.2 protocol mode */
} vtss_upi_protocol_t;

/** \brief upi config data  */
typedef struct vtss_upi_cfg_t {
    BOOL tx_to_rx_loopback;         /**< TRUE => select equipment loopback (TX->RX), FALSE => no loopback */
    BOOL rx_to_tx_loopback;         /**< TRUE => select line loopback (RX->TX), FALSE => no loopback */
} vtss_upi_cfg_t;

/** \brief upi status. */
typedef struct vtss_upi_status_t {
    BOOL ssf;                       /**< UPI indicates SSF (Server Signal Fail) */
    BOOL rx_los;                    /**< PIN indicates loss of signal */
    BOOL tx_fifo_oflow;             /**< UPI TX FIFO indicates overflow */
    BOOL tx_fifo_uflow;             /**< UPI TX FIFO indicates underflow */
    struct  {
        BOOL pll_lock;              /**< SerDes indicates PLL lock */
        BOOL hss_rst_out;           /**< PLL reset output indication */
        BOOL hss_eyequality;        /**< SerDes indicates an eye quality problem state */
        BOOL rx_sigdet[4];          /**< RX SerDes indicates lane signal detects (per lane) for 2 lane config ignore 2 entries */
    } hss_status;                   /**< High speed serdes status*/
    BOOL upi_2lane;                 /**< 2 lane or 4 lane UPI interface mode*/
    vtss_upi_protocol_t proto;      /**< UPI interface protocol mode*/
} vtss_upi_status_t;

/** \brief UPI test pattern mode types.
 */
typedef enum  {
    VTSS_UPI_TEST_MODE_DISABLE,         /**< Disable test */
    VTSS_UPI_TEST_MODE_PRBS7_P,         /**< Enable PRBS7 generator */
    VTSS_UPI_TEST_MODE_PRBS7_N,         /**< Enable PRBS7-inverted generator */
    VTSS_UPI_TEST_MODE_PRBS23_P,        /**< Enable PRBS23 generator */
    VTSS_UPI_TEST_MODE_PRBS23_N,        /**< Enable PRBS23-inverted generator */
    VTSS_UPI_TEST_MODE_PRBS31_P,        /**< Enable PRBS31 generator */
    VTSS_UPI_TEST_MODE_PRBS31_N,        /**< Enable PRBS31-inverted generator */
    VTSS_UPI_TEST_MODE_HF,              /**< Enable 10101010.... generator */
    VTSS_UPI_TEST_MODE_LF,              /**< Enable 64*1;64*0;.... generator */
    VTSS_UPI_TEST_MODE_PRBS9_P,         /**< Enable PRBS9 generator */
    VTSS_UPI_TEST_MODE_MAX
} vtss_upi_test_mode_t;

/** \brief UPI test rx wrap mode types.
 */
typedef enum  {
    VTSS_UPI_TEST_WRAP_RX,              /**< Select RX Serial Input */
    VTSS_UPI_TEST_WRAP_INT_LOOP,        /**< Select internal loop (from rx generator, only valid for PRBS7_P mode) */
    VTSS_UPI_TEST_WRAP_INT_WRAP,        /**< Select internal wrap (from tx path) */
    VTSS_UPI_TEST_WRAP_MAX
} vtss_upi_test_wrap_t;

/** \brief upi test configuration */
typedef struct vtss_upi_test_cfg_s {
    vtss_upi_test_mode_t test_cfg_gen;   /**< test pattern generation configuration  */
    vtss_upi_test_mode_t test_cfg_chk;   /**< test pattern checker configuration  */
    vtss_upi_test_wrap_t wrap_mode;      /**< rx checker wrap mode */
} vtss_upi_test_cfg_t;

/** \brief upi test status (pr lane) */
typedef struct vtss_upi_test_status_s {
    BOOL lbsync;            /**< PRBS Sync Status, asserted when PRBS checker locks to incoming PRBS data, deasserted on reset (i.e. when test config is set) */
    BOOL lberror;           /**< PRBS Error Status, asserted when PRBS checker detects an error after sync has neen acheived, deasserted on reset (i.e. when test config is set)  */
    BOOL tx_prbssync;       /**< TX PRBS synced indication  */
    BOOL tx_prbs_err;       /**< indicate TX PRBS error */
    u32  rx_prbs_err_count; /**< Approximate counter for RX PRBS errors (clear on read).*/
    u32  tx_prbs_err_count; /**< Approximate counter for TX PRBS errors (clear on read).*/
                            /* PLLLOCK can be read in the upi_status */
} vtss_upi_test_status_t;

/** \brief upi ffe channel loss configurations */
typedef enum {
    upi_chloss_lt10inch, /**< Less than 10 inch length */
    upi_chloss_gt10inch, /**< Greater than 10 inch length */
    upi_chloss_backplane,/**< Backplane channel */
    upi_chloss_max       /* Maximum number of channel loss types */ 
} vtss_upi_eq_chloss_t;

/** \brief upi ffe configuration modes */
typedef enum {
    UPI_TXEQ_OVERRIDE, /**< Override mode */
    UPI_TXEQ_NORMAL,   /**< Normal mode */
    UPI_TXEQ_MODE_MAX  /**< Number of mode types */
} vtss_upi_txeq_mode_t;

/** \brief upi ffe coefficients */
typedef enum {
    upi_txeq_coef_0,   /**< FFE Coefficient C0 */
    upi_txeq_coef_1,   /**< FFE Coefficient C1 */
    upi_txeq_coef_2,   /**< FFE Coefficient C2 */
    upi_txeq_coef_3,   /**< FFE Coefficient C3 */
    upi_txeq_coef_max  /**< Number of FFE Coefficients */
} vtss_upi_ffe_coef_t;

/** \brief upi ffe configuration parameters */
typedef struct vtss_upi_ffe_cfg_s{
    vtss_upi_txeq_mode_t mode;    /**< Equalization mode */
    vtss_upi_eq_chloss_t channel_loss;/**< Channel loss type */
} vtss_upi_txeq_cfg_t;

/** \brief upi transmit equalization configuration for channel */
typedef struct vtss_upi_txeq_cfg_s{
    vtss_upi_txeq_cfg_t txeq_cfg; /**< Equalization configuration */
    u32 transmit_power;      /**< Transmitter power */
    u32 slew_rate;           /**< Minimum slew rate */
	u32 coefs[upi_txeq_coef_max]; /**< Equalizer coefficient values */
    BOOL polarity[upi_txeq_coef_max]; /**< Polarities of equalizer co-efficients*/
} vtss_upi_tx_cfg_t;

/** \brief upi dfe configuration for channel */
typedef struct vtss_upi_dfe_cfg_s{
    vtss_upi_eq_chloss_t channel_loss;/**< Channel loss type of receiver */
} vtss_upi_rxeq_cfg_t;

/** \brief upi state - container for vtss_state    */
typedef struct vtss_upi_state_t {
    vtss_upi_cfg_t      upi_cfg;        /**< Storage for UPI configuration */
    vtss_upi_test_cfg_t upi_test_cfg;   /**< Storage for UPI Test configuration */
    vtss_upi_tx_cfg_t upi_tx_cfg;   /**< Storage for UPI transmit equalization configuration */
    vtss_upi_rxeq_cfg_t upi_rxeq_cfg;   /**< Storage for UPI receive equalization configuration */
} vtss_upi_state_t;


/* ================================================================= *
 *  Defects/Events
 * ================================================================= */
/**
 * \brief UPI interrupt event mask values
 **/
#define VTSS_UPI_RX_HSS_PRBSSYNC_0_EV   0x0000000000000001LL  /**< RX PRBS synced indication lane 0*/
#define VTSS_UPI_RX_HSS_PRBSSYNC_1_EV   0x0000000000000002LL  /**< RX PRBS synced indication lane 1 */
#define VTSS_UPI_RX_HSS_PRBSSYNC_2_EV   0x0000000000000004LL  /**< RX PRBS synced indication lane 2 */
#define VTSS_UPI_RX_HSS_PRBSSYNC_3_EV   0x0000000000000008LL  /**< RX PRBS synced indication lane 3 */
#define VTSS_UPI_TX_HSS_PRBSSYNC_0_EV   0x0000000000000010LL  /**< TX PRBS synced indication lane 0 */
#define VTSS_UPI_TX_HSS_PRBSSYNC_1_EV   0x0000000000000020LL  /**< TX PRBS synced indication lane 1 */
#define VTSS_UPI_TX_HSS_PRBSSYNC_2_EV   0x0000000000000040LL  /**< TX PRBS synced indication lane 2 */
#define VTSS_UPI_TX_HSS_PRBSSYNC_3_EV   0x0000000000000080LL  /**< TX PRBS synced indication lane 3 */
#define VTSS_UPI_HSS_RESETOUT_EV        0x0000000000000100LL  /**< PLL reset output change indication */
#define VTSS_UPI_RX_LOS_EV              0x0000000000000200LL  /**< PIN indicates a change to loss of signal */
#define VTSS_UPI_RX_SIGDET_0_EV         0x0000000000000400LL  /**< RX SerDes indicates a change to lane signal detects lane 0 */
#define VTSS_UPI_RX_SIGDET_1_EV         0x0000000000000800LL  /**< RX SerDes indicates a change to lane signal detects lane 1 */
#define VTSS_UPI_RX_SIGDET_2_EV         0x0000000000001000LL  /**< RX SerDes indicates a change to lane signal detects lane 2 */
#define VTSS_UPI_RX_SIGDET_3_EV         0x0000000000002000LL  /**< RX SerDes indicates a change to lane signal detects lane 3 */
#define VTSS_UPI_TX_FIFO_OVERFLOW_EV    0x0000000000004000LL  /**< UPI TX FIFO indicates overflow */
#define VTSS_UPI_TX_FIFO_UNDERFLOW_EV   0x0000000000008000LL  /**< UPI TX FIFO indicates underflow */
#define VTSS_UPI_HSS_PRTREADY_EV        0x0000000000010000LL  /**< SerDes indicates a change to the SerDes register access ready state */
#define VTSS_UPI_RX_PRBS_ERR_0_EV       0x0000000000020000LL  /**< RX SerDes indicate PRBS error change lane 0 */
#define VTSS_UPI_RX_PRBS_ERR_1_EV       0x0000000000040000LL  /**< RX SerDes indicate PRBS error change lane 1 */
#define VTSS_UPI_RX_PRBS_ERR_2_EV       0x0000000000080000LL  /**< RX SerDes indicate PRBS error change lane 2 */
#define VTSS_UPI_RX_PRBS_ERR_3_EV       0x0000000000100000LL  /**< RX SerDes indicate PRBS error change lane 3 */
#define VTSS_UPI_TX_PRBS_ERR_0_EV       0x0000000000200000LL  /**< TX SerDes indicate PRBS error change lane 0 */
#define VTSS_UPI_TX_PRBS_ERR_1_EV       0x0000000000400000LL  /**< TX SerDes indicate PRBS error change lane 1 */
#define VTSS_UPI_TX_PRBS_ERR_2_EV       0x0000000000800000LL  /**< TX SerDes indicate PRBS error change lane 2 */
#define VTSS_UPI_TX_PRBS_ERR_3_EV       0x0000000001000000LL  /**< TX SerDes indicate PRBS error change lane 3 */
#define VTSS_UPI_SSF_EV                 0x0000000002000000LL  /**< UPI indicates SSF change */
#define VTSS_UPI_PLLLOCK_EV             0x0000000004000000LL  /**< SerDes indicates PLL lock change */
#define VTSS_UPI_EYE_QUALITY_ERR_EV     0x0000000008000000LL  /**< SerDes indicates an eye quality problem state change */

#define VTSS_UPI_OTN_FAS_POS_EV         0x0000000010000000LL  /**< OTN Framer indicates a change in the frame alignment position */
#define VTSS_UPI_OTN_LOM_EV             0x0000000020000000LL  /**< OTN Framer indicates a change in LOM state */
#define VTSS_UPI_OTN_OOM_EV             0x0000000040000000LL  /**< OTN Framer indicates a change in OOM state */
#define VTSS_UPI_OTN_LOF_EV             0x0000000080000000LL  /**< OTN Framer indicates a change in LOF state */
#define VTSS_UPI_OTN_OOF_EV             0x0000000100000000LL  /**< OTN Framer indicates a change in OOF state */


typedef u64 vtss_upi_event_t; /**< Int events: Single event or 'OR' multiple events above */

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

vtss_rc vtss_upi_event_enable(const vtss_inst_t         inst,
                              const vtss_port_no_t      port_no,
                              const BOOL                enable,
                              const vtss_upi_event_t    ev_mask);

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

vtss_rc vtss_upi_event_poll(const vtss_inst_t     inst,
                            const vtss_port_no_t  port_no,
                            vtss_upi_event_t      *const status);


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

vtss_rc vtss_upi_event_poll_without_mask(const vtss_inst_t     inst,
                                         const vtss_port_no_t  port_no,
                                         vtss_upi_event_t      *const status);



/* ================================================================= *
 *  Dynamic Config
 * ================================================================= */

/**
 * \brief Set UPI configuration data.
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 * \param cfg [IN]       UPI configuration
 *
 * \return Return code.
 **/
vtss_rc vtss_upi_conf_set(const vtss_inst_t inst,
                          const vtss_port_no_t port_no,
                          const vtss_upi_cfg_t *const cfg);
/**
 * \brief  Get UPI configuration data.
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 * \param cfg [OUT]      UPI configuration
 *
 * \return Return code.
 **/
vtss_rc vtss_upi_conf_get(const vtss_inst_t inst,
                          const vtss_port_no_t port_no,
                          vtss_upi_cfg_t *const cfg);

/* ================================================================= *
 *  State Reporting
 * ================================================================= */

/**
 * \brief Get status from the UPI interface.
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 * \param status [OUT]   pointer to defect status structure.
 *
 * \return Return code.
 **/
vtss_rc vtss_upi_status_get(const vtss_inst_t inst,
                            const vtss_port_no_t port_no,
                            vtss_upi_status_t *const status);

/* ================================================================= *
 *  Performance Primitives
 * ================================================================= */

/* ================================================================= *
 *  Test/Debug Config
 * ================================================================= */

/**
 * \brief Set UPI Test configuration data.
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 * \param cfg [IN]       UPI test configuration
 *
 * \return Return code.
 **/
vtss_rc vtss_upi_test_conf_set(const vtss_inst_t inst,
                          const vtss_port_no_t port_no,
                          const vtss_upi_test_cfg_t *const cfg);

/**
 * \brief Get UPI Test configuration data.
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 * \param cfg [OUT]      UPI test configuration
 *
 * \return Return code.
 **/
vtss_rc vtss_upi_test_conf_get(const vtss_inst_t inst,
                          const vtss_port_no_t port_no,
                          vtss_upi_test_cfg_t *const cfg);

/**
 * \brief Get UPI Test status data.
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 * \param status [OUT]   UPI test status
 *
 * \return Return code.
 **/
vtss_rc vtss_upi_test_status_get(const vtss_inst_t inst,
                          const vtss_port_no_t port_no,
                          vtss_upi_test_status_t *const status);

/**
 * \brief Set UPI Transmit equalization mode.
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 * \param cfg [IN]       Transmit Equalization configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_upi_txeq_mode_set(const vtss_inst_t inst,
                               const vtss_port_no_t port_no,
                               vtss_upi_txeq_cfg_t const *cfg);

/**
 * \brief Get UPI Transmit equalization mode.
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 * \param cfg [IN]       Transmit Equalization configuration
 *
 * \return Return code.
 **/
vtss_rc vtss_upi_txeq_mode_get(const vtss_inst_t inst,
                               const vtss_port_no_t port_no,
                               vtss_upi_txeq_cfg_t *const cfg);
/**
 * \brief Fine tune the FFE coefficients.
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 * \param coef [IN]      FFE Coefficient
 * \param step_size [IN] Step size to increment or decrement coefficient
 * \param incr           Increment or Decrement the Co-efficient value
 * \param polarity [IN]  Polarity of the FFE coefficient
 *
 * \return Return code.
 **/
vtss_rc vtss_upi_txeq_coef_adjust(const vtss_inst_t inst,
                                  const vtss_port_no_t port_no,
                                  vtss_upi_ffe_coef_t coef,
                                  u32 step_size,
                                  BOOL incr,
                                  BOOL polarity);

/**
 * \brief Get the FFE coefficient values configured
 *
 * \param inst [IN]		Target instance reference
 * \param port_no [IN]  Port number.
 * \param coefs [OUT]    Array of coefficients
 * \param polarity [OUT] Array of Polarity values of coefficients
 *
 * \return Return code.
 **/
vtss_rc vtss_upi_txeq_coef_get(const vtss_inst_t inst,
                               const vtss_port_no_t port_no,
                               u32 coefs[], BOOL polarity[]);

/**
 * \brief Set the transmit power
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 * \param mVppd [IN]     Amplitude in range of 20 to 127. 115 implies 100% power.
 *
 * \return Return code;
 **/
vtss_rc vtss_upi_txamp_set(const vtss_inst_t inst,
                           const vtss_port_no_t port_no,
                           u32 mVppd);

/**
 * \brief Get the transmit power
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 * \param mVppd [OUT]    Power amplitude 
 *
 * \return Return code;
 **/
vtss_rc vtss_upi_txamp_get(const vtss_inst_t inst,
                           const vtss_port_no_t port_no,
                           u32 *mVppd);

/**
 * \brief Set the transmit slew rate 
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 * \param slew_rate [IN] Output slew rate
 * 0: 30 ps min. (default)
 * 1: 60 ps 
 *
 * \return Return code.
 **/
vtss_rc vtss_upi_txslew_set(const vtss_inst_t inst,
                            const vtss_port_no_t port_no,
                            u32 slew_rate);

/**
 * \brief Get the transmit slew rate 
 *
 * \param inst [IN]        Target instance reference.
 * \param port_no [IN]     Port number.
 * \param slew_rate [OUT]  Output slew rate
 * 0: 30 ps min. (default)
 * 1: 60 ps 
 *
 * \return Return code.
 **/
vtss_rc vtss_upi_txslew_get(const vtss_inst_t inst,
                            const vtss_port_no_t port_no,
                            u32 *slew_rate);

/**
 * \brief Set UPI Receive equalization mode.
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 * \param cfg [IN]       Receive Equalization configuration
 *
 * \return Return code.
 **/
vtss_rc vtss_upi_rxeq_mode_set(const vtss_inst_t inst,
                               const vtss_port_no_t port_no,
                               vtss_upi_rxeq_cfg_t const *cfg);

/**
 * \brief Get UPI Receive equalization mode.
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 * \param cfg [OUT]       Receive Equalization configuration
 *
 * \return Return code.
 **/
    
vtss_rc vtss_upi_rxeq_mode_get(const vtss_inst_t inst,
                               const vtss_port_no_t port_no,
                               vtss_upi_rxeq_cfg_t *const cfg);

#endif /* VTSS_FEATURE_UPI */

#endif /* _VTSS_UPI_API_H_ */
