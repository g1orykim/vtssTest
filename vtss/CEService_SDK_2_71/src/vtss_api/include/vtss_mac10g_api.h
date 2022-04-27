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
 * \file vtss_mac10g_api.h
 * \brief  MAC10G  API
 */

#ifndef _VTSS_MAC10G_API_H_
#define _VTSS_MAC10G_API_H_

#include <vtss_options.h>
#include <vtss_types.h>

#if defined(VTSS_FEATURE_MAC10G)

/* ================================================================= *
 *  Data structures / types
 * ================================================================= */

/** \brief MAC config data.*/
typedef struct vtss_mac10g_cfg {
	vtss_port_flow_control_conf_t flow_control;      /**< Flow control setup */
	u32                           max_frame_length;  /**< Maximum frame length */
	vtss_port_max_tags_t          max_tags;          /**< VLAN awareness for length check */
	BOOL                          tx_rx_loopback;    /**< Enables loopback from TX to RX */
	BOOL                          rx_tx_loopback;    /**< Enables loopback from RX to TX */
#ifdef VTSS_ARCH_DAYTONA
    BOOL                          preamble_check;    /**< Preamble Check Enable/Disable*/
    BOOL                          local_fault_force; /**< Force Local fault */
    BOOL                          remote_fault_force; /**< Force Remote fault */
#endif /* VTSS_ARCH_DAYTONA */
} vtss_mac10g_cfg_t;

/** \brief mac status   */
typedef struct vtss_mac10g_status {
    BOOL link_state;    /**<  Link State */
    BOOL pause;         /**< Pause state indication, Interface is paused when the pause timer is non zero*/
    BOOL local_fault;   /**< Local Fault state; 0 - No local fault, 1 - Local fault detected */
    BOOL remote_fault;  /**< Remote Fault state; 0 - No Remote fault, 1 - Remote fault detected */
} vtss_mac10g_status_t;

/** \brief mac counters */
typedef struct vtss_mac10g_cnt {
	vtss_port_rmon_counters_t   rmon; /**< RMON counters */

} vtss_mac10g_cnt_t;

/** \brief  mac events */
typedef enum vtss_mac10g_events {
        VTSS_MAC10G_EVENT_NONE, /**< MAC events */
} vtss_mac10g_events_t;

/** \brief mac vtss_state structure definitions */
typedef struct vtss_mac10g_cfg_state {
        vtss_mac10g_cfg_t    mac10g_cfg; /**< MAC 10G configuration */
} vtss_mac10g_state_t;



/* ================================================================= *
 *  Defects/Events
 * ================================================================= */
/**
 * \brief MAC10G interrupt event mask values
 **/
#define VTSS_MAC10G_LINK_DELTA_EV                    0x00000001  /**< Link state has changed */
#define VTSS_MAC10G_REMOTE_FAULT_STATE_DELTA_EV      0x00000002  /**< The LOCAL_FAULT state has either been entered or exited */
#define VTSS_MAC10G_LOCAL_FAULT_STATE_DELTA_EV       0x00000004  /**< The REMOTE_FAULT state has either been entered or exited */
#define VTSS_MAC10G_TX_ABORT_EV                      0x00000008  /**< One or more Tx frames were aborted */
#define VTSS_MAC10G_RX_NON_STD_PREAM_EV              0x00000010  /**< A frame with a non-standard preamble has been received */
#define VTSS_MAC10G_RX_PREAM_ERR_EV                  0x00000020  /**< Preamble error detected */
#define VTSS_MAC10G_RX_PREAM_MISMATCH_EV             0x00000040  /**< A non-standard preamble has been received */
#define VTSS_MAC10G_RX_PREAM_SHRINK_EV               0x00000080  /**< Preamble shrink detected */
#define VTSS_MAC10G_TX_IFG_COLUMN_DROP_EV            0x00000100  /**< An idle column has been dropped */

typedef u32 vtss_mac10g_event_t; /**< Int events: Single event or 'OR' multiple events above */

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

vtss_rc vtss_mac10g_event_enable(const vtss_inst_t           inst,
                                 const vtss_port_no_t        port_no,
                                 const BOOL                  enable,
                                 const vtss_mac10g_event_t   ev_mask);

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

vtss_rc vtss_mac10g_event_poll(const vtss_inst_t     inst,
                               const vtss_port_no_t  port_no,
                               vtss_mac10g_event_t     *const status);

#if defined(VTSS_ARCH_DAYTONA)

/**
 * \brief Polling function called at by interrupt or periodically
 * \note Interrupt status will be cleared on read
 *
 * \param inst    [IN]   Target instance reference.
 * \param port_no [IN]   Port number
 * \param status  [OUT]  Event status, bit set indicates corresponding
 *                       event/interrupt has detected irrespectove of the mask register
 *
 * \return Return code.
 **/

vtss_rc vtss_mac10g_event_poll_without_mask(const vtss_inst_t     inst,
                                            const vtss_port_no_t  port_no,
                                            vtss_mac10g_event_t     *const status);

#endif /* VTSS_ARCH_DAYTONA */

/* ================================================================= *
 *  Dynamic Config
 * ================================================================= */
/**
 * \brief  Set mac configuration parameters.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [IN]      pointer to configuration data.
 *
 * \return Return code.
 **/
vtss_rc vtss_mac10g_config_set(const vtss_inst_t inst,
                               const vtss_port_no_t port_no,
                               const vtss_mac10g_cfg_t *const cfg);

/**
 * \brief Get mac configuration parameters.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [OUT]     pointer to configuration data.
 *
 * \return Return code.
 **/
vtss_rc vtss_mac10g_config_get(const vtss_inst_t inst,
                               const vtss_port_no_t port_no,
                               vtss_mac10g_cfg_t * const cfg);

/* ================================================================= *
 *  State Reporting
 * ================================================================= */

/**
 * \brief Get mac configuration parameters.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param status [OUT]  Status of the MAC block.
 *
 * \return Return code.
 **/
vtss_rc vtss_mac10g_status_get(const vtss_inst_t inst,
                               const vtss_port_no_t port_no,
                               vtss_mac10g_status_t * const status);

/* ================================================================= *
 *  Performance Primitives
 * ================================================================= */
/**
 * \brief Update the actual mac RMON counters.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 *
 * \return Return code.
 **/
vtss_rc vtss_mac10g_counters_update(const vtss_inst_t inst,
                                    const vtss_port_no_t port_no);

/**
 * \brief Clear mac RMON counters.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 *
 * \return Return code.
 **/
vtss_rc vtss_mac10g_counters_clear(const vtss_inst_t inst,
                                   const vtss_port_no_t port_no);

/**
 * \brief Get actual mac RMON counters.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param counters [OUT]     pointer to counter structure.
 *
 * \return Return code.
 **/
vtss_rc vtss_mac10g_counters_get(const vtss_inst_t inst,
                                 const vtss_port_no_t port_no,
                                 vtss_port_counters_t *const counters);

#endif /* VTSS_FEATURE_MAC10G */

#endif /* _VTSS_MAC10G_API_H_ */
