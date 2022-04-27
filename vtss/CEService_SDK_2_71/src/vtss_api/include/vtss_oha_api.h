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
 * \file vtss_oha_api.h
 * \brief OHA API
 */

#ifndef _VTSS_OHA_API_H_
#define _VTSS_OHA_API_H_

#include <vtss_options.h>
#include <vtss_types.h>

#if defined(VTSS_FEATURE_OHA)

/* ================================================================= *
 *  Data structures / types
 * ================================================================= */
/** \brief WIS TOSI/ROSI or OTU2 OH A/D mode types */
typedef enum {
    VTSS_OHA_TOSI_ROSI_NONE,     /**< not in use */
    VTSS_OHA_TOSI_ROSI_EWIS2,    /**< select TOSI/ROSI from eWIS2 */
    VTSS_OHA_TOSI_ROSI_OTU2_1,   /**< select OTU2 OH from OTU2-1 */
    VTSS_OHA_TOSI_ROSI_MAX
} vtss_oha_tosi_rosi_mode_t;

/** \brief OTU2 OH A/D mode types */
typedef enum {
    VTSS_OHA_OTU2_NONE,     /**< not in use */
    VTSS_OHA_OTU2_OTU2_1,   /**< select OTU2 OH from OTU2-1 */
    VTSS_OHA_OTU2_OTU2_2,   /**< select OTU2 OH from OTU2-2 */
    VTSS_OHA_OTU2_MAX
} vtss_oha_otu2_mode_t;

/** \brief OTU2 gap_clk_out mode types */
typedef enum {
    VTSS_OHA_GAP_CLK_NONE,     /**< not in use */
    VTSS_OHA_GAP_CLK_OTU2_1,   /**< select OTU2 GAP_CLK from OTU2-1 */
    VTSS_OHA_GAP_CLK_OTU2_2,   /**< select OTU2 GAP_CLK from OTU2-2 */
    VTSS_OHA_GAP_CLK_MAX
} vtss_oha_gap_clk_out_t;

/** \brief oha channels configuration */
typedef struct vtss_oha_cfg_s {
    vtss_oha_tosi_rosi_mode_t tosi_rosi_otu2_1;   /**< Configuration for the WIS TOSI/ROSI or OTU2 OH A/D */
    vtss_oha_otu2_mode_t otu2_1_otu2_2;           /**< Configuration for the OTU2 OH A/D */
    vtss_oha_gap_clk_out_t gap_clk_out;           /**< Configuration for the OTU2 gap_clk_out */
} vtss_oha_cfg_t;

/** \brief oha state - container for vtss_state    */
typedef struct vtss_oha_state_t {
    vtss_oha_cfg_t     oha_cfg;    /**< oha config data */
} vtss_oha_state_t;

/* ================================================================= *
 *  Defects/Events
 * ================================================================= */

/* ================================================================= *
 *  Dynamic Config
 * ================================================================= */


/**
* \brief OHA channels and clock setup.
*  For each channel in Daytona there are 2 OHA serial channels and clock output that can be configured.
* The function sets the multiplexer for the OHA serial channels. Selecting a channel for eWIS will 
* enables the OHA for that block
*
* \param inst [IN]      Target instance reference.
* \param port_no [IN]   Port number.
* \param cfg [IN]       pointer to OHA channels and clock setup.
*
* \return Return code.
**/

vtss_rc vtss_oha_config_set(const vtss_inst_t inst,
                            vtss_port_no_t port_no,
                            const vtss_oha_cfg_t *const cfg
                          );

/**
 * \brief  Get OHA channels configuration.
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 * \param cfg [IN]       pointer to OHA channels and clock setup.
 *
 * \return Return code.
 **/
vtss_rc vtss_oha_config_get(const vtss_inst_t inst,
                            vtss_port_no_t port_no,
                            vtss_oha_cfg_t *const cfg
                           );
             
/* ================================================================= *
 *  State Reporting
 * ================================================================= */


/* ================================================================= *
 *  Performance Primitives
 * ================================================================= */

#endif /* VTSS_FEATURE_OHA */

#endif /* _VTSS_OHA_API_H_ */
