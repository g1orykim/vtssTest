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
 * \file vtss_rab_api.h
 * \brief  RAB  API
 */

#ifndef _VTSS_RAB_API_H_
#define _VTSS_RAB_API_H_

#include <vtss_options.h>
#include <vtss_types.h>

#if defined(VTSS_FEATURE_RAB)

/* ================================================================= *
 *  Data structures / types
 * ================================================================= */

/** \brief RAB config data   */
typedef struct vtss_rab_cfg {
	u16  fifo_threshold_high_rx;      /**< High Threshold for generation of PAUSE flow control on MAC2 TX */
	u16  fifo_threshold_low_rx;       /**< Low  Threshold for generation of PAUSE flow control on MAC2 TX */
	u16  fifo_threshold_adapt_rx;     /**< Threshold for asertion of rate adaption request to MAC1 TX */
	u16  fifo_threshold_high_tx;      /**< High Threshold for generation of PAUSE flow control on MAC1 TX */
	u16  fifo_threshold_low_tx;       /**< Low  Threshold for generation of PAUSE flow control on MAC1 TX */
	u16  fifo_threshold_adapt_tx;     /**< Threshold for asertion of rate adaption request to MAC2 TX */
    BOOL cut_thru_rx;                 /**< RAB does not store any frames for processing in cut_thru mode
                                           (i.e) abort marked frames will not be discarded */
    BOOL cut_thru_tx;                 /**< RAB does not store any frames for processing in cut_thru mode
                                           (i.e) abort marked frames will not be discarded */
} vtss_rab_cfg_t;

/** \brief RAB counters */
typedef struct vtss_rab_counters {
	u32 pkt_drop_cnt_rx;    /**< Rx packet drop counter */
	u32 pkt_drop_cnt_tx;    /**< Tx packet drop counter */
} vtss_rab_counters_t;


/** \brief RAB vtss_state structure definitions */
typedef struct vtss_rab_cfg_state {
        vtss_rab_cfg_t    rab_cfg; /**< RAB configuration */
} vtss_rab_state_t;

/* ================================================================= *
 *  Dynamic Config
 * ================================================================= */
/**
 * \brief  Set RAB configuration parameters.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [IN]      pointer to configuration data.
 *
 * \return Return code.
 **/
vtss_rc vtss_rab_config_set(const vtss_inst_t inst,
                            const vtss_port_no_t port_no,
                            const vtss_rab_cfg_t *const cfg);

/**
 * \brief Get RAB configuration parameters.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param cfg [OUT]     pointer to configuration data.
 *
 * \return Return code.
 **/
vtss_rc vtss_rab_config_get(const vtss_inst_t inst,
                            const vtss_port_no_t port_no,
                            vtss_rab_cfg_t * const cfg);

/* ================================================================= *
 *  State Reporting
 * ================================================================= */


/* ================================================================= *
 *  Performance Primitives
 * ================================================================= */
/**
 * \brief Get actual RAB counters.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param counters [OUT]     pointer to counter structure.
 *
 * \return Return code.
 **/
vtss_rc vtss_rab_counters_get(const vtss_inst_t inst,
                              const vtss_port_no_t port_no,
                              vtss_rab_counters_t *const counters);

#endif /* VTSS_FEATURE_RAB */

#endif /* _VTSS_RAB_API_H_ */
