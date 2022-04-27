/*

 Vitesse Switch API software.

 Copyright (c) 2002-2012 Vitesse Semiconductor Corporation "Vitesse". All
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

/**
 * \file
 * \brief Synchronization API
 * \details This header file describes synchronization functions
 */

#ifndef _VTSS_SYNC_API_H_
#define _VTSS_SYNC_API_H_
#include "vtss_types.h"



// ***************************************************************************
//
//  SYNC interface
//
// ***************************************************************************

#if defined(VTSS_FEATURE_SYNCE)

/** \brief Identification of a output clock port */
typedef enum
{
    VTSS_SYNCE_CLK_A,   /**< Clock A output port */
    VTSS_SYNCE_CLK_B,   /**< Clock B output port */
} vtss_synce_clk_port_t;

/** \brief Identification of a Clock dividing value used when selected input clock goes to output. */
typedef enum
{
    VTSS_SYNCE_DIVIDER_1,   /**< Divide input clock with one (no division) */
    VTSS_SYNCE_DIVIDER_4,   /**< Divide input clock with 4 */
    VTSS_SYNCE_DIVIDER_5    /**< Divide input clock with 5 */
} vtss_synce_divider_t;

/** \brief Struct containing configuration for a recovered clock output port */
typedef struct
{
    vtss_synce_divider_t    divider;    /**< Selection the clock division. This should be set to VTSS_SYNCE_DIVIDER_1 if recovered clock is comming from internal PHY  */
    BOOL                    enable;     /**< Enable/disable of this output clock port */
} vtss_synce_clock_out_t;

/**
 * \brief Set the configuration of a selected output clock port - against external clock controller.
 * \param inst [IN]     handle to an API instance.
 * \param clk_port [IN] Selection of the output clock port (VTSS_SYNCE_CLK_A or VTSS_SYNCE_CLK_B)
 * \param conf [IN]     pointer to a vtss_synce_clock_out_t configuration structure.
 *
 */
vtss_rc vtss_synce_clock_out_set(const vtss_inst_t              inst,
                                 const vtss_synce_clk_port_t    clk_port,
                                 const vtss_synce_clock_out_t   *const conf);

/**
 * \brief Get the configuration of a selected output clock port - against external clock controller.
 * \param inst [IN]     handle to an API instance.
 * \param clk_port [IN] Selection of the output clock port (VTSS_SYNCE_CLK_A or VTSS_SYNCE_CLK_B)
 * \param conf [IN]     pointer to a vtss_synce_clock_out_t configuration structure.
 *
 */
vtss_rc vtss_synce_clock_out_get(const vtss_inst_t           inst,
                                 const vtss_synce_clk_port_t clk_port,
                                 vtss_synce_clock_out_t      *const conf);




/** \brief Struct containing configuration selecting the recovered input clock port, to be delivered to a selected output clock port */
typedef struct
{
    vtss_port_no_t          port_no;    /**< Selection of the input port number - must map to a SERDES port*/
    BOOL                    squelsh;    /**< Enable/disable of automatic squelsh */
    BOOL                    enable;     /**< Enable/disable of delivery of recovered clock to this selected output clock port */
} vtss_synce_clock_in_t;

/**
 * \brief Set the configuration of input port for a selected output clock port
 * \param inst [IN]     handle to an API instance.
 * \param clk_port [IN] Selection of the output clock port (VTSS_SYNCE_CLK_A or VTSS_SYNCE_CLK_B)
 * \param conf [IN]     pointer to a vtss_synce_clock_in_t configuration structure.
 *
 */
vtss_rc vtss_synce_clock_in_set(const vtss_inst_t              inst,
                                const vtss_synce_clk_port_t    clk_port,
                                const vtss_synce_clock_in_t    *const conf);

/**
 * \brief Get the configuration of input port for a selected output clock port
 * \param inst [IN]     handle to an API instance.
 * \param clk_port [IN] Selection of the output clock port (VTSS_SYNCE_CLK_A or VTSS_SYNCE_CLK_B)
 * \param conf [IN]     pointer to a vtss_synce_clock_in_t configuration structure.
 *
 */
vtss_rc vtss_synce_clock_in_get(const vtss_inst_t            inst,
                                const vtss_synce_clk_port_t  clk_port,
                                vtss_synce_clock_in_t        *const conf);

                                   
#endif /* VTSS_FEATURE_SYNCE */

#endif // _VTSS_SYNC_API_H_

// ***************************************************************************
//
//  End of file.
//
// ***************************************************************************
