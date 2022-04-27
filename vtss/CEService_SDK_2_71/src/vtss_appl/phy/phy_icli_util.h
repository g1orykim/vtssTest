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
 
 $Id$
 $Revision$

*/

#ifndef _PHY_ICLI_UTIL_H_
#define _PHY_ICLI_UTIL_H_


/* ICLI request structure */
typedef struct {
    u32                     session_id;
    icli_stack_port_range_t *port_list;
    icli_unsigned_range_t   *addr_list;
    u32                     count;
    u32                     page;
    u32                     value;
    BOOL                    addr_sort;
    BOOL                    write;
    BOOL                    header;
    BOOL                    dummy;  /* Unused, for Lint */
} phy_icli_req_t;

void phy_icli_debug_phy(phy_icli_req_t *req);

// Debug function used for testing PHY for simulating cu SFP setup in forced mode.
vtss_rc phy_icli_debug_phy_pass_through_speed(i32 session_id, icli_stack_port_range_t *plist, BOOL has_1g, BOOL has_100M, BOOL has_10M);

// Debug function for enabling/disabling phy page checking. PHY page checking is a mode where the page register is checked for is we are at the right page for all phy registers accesses (This will double the amount of registers access)
vtss_rc phy_icli_debug_do_page_chk(i32 session_id, BOOL has_enable, BOOL has_disable);

/**
 * \brief Debug function for resetting a PHY 
 *
 * \param session_id [IN] The session id use by iCLI print.
 * \param plist      [IN] Port list with ports to configure (Not Stack-aware).
 * \return VTSS_RC_OK if PHY were reset correct else error code.
 **/
vtss_rc phy_icli_debug_phy_reset(i32 session_id, icli_stack_port_range_t *plist);

/**
 * \brief Debug function for accessing/configuring PHY GPIOs
 *
 * \param session_id           [IN] The session id use by iCLI print.
 * \param plist                [IN] Port list with ports to configure (Not Stack-aware).
 * \param has_mode_output      [IN] Set to TRUE in order to configure GPIO as output.
 * \param has_mode_input       [IN] Set to TRUE in order to configure GPIO as input.
 * \param has_mode_alternative [IN] Set to TRUE in order to configure GPIO to alternative mode (See datasheet).
 * \param has_gpio_get         [IN] Set to TRUE in order to print the current state for the GPIO.
 * \param has_gpio_set         [IN] Set to TRUE in order to set the current state of the GPIO.
 * \param value                [IN] Set to TRUE in order to set the current state of the GPIO to high (FALSE = set low). 
 * \param gpio_no              [IN] the gpio number to access/configure.
 * \return VTSS_RC_OK if PHY GPIO were accessed/configured correct else error code.
 **/
vtss_rc phy_icli_debug_phy_gpio(i32 session_id, icli_stack_port_range_t *plist, BOOL has_mode_output, BOOL has_mode_input, BOOL has_mode_alternative, BOOL has_gpio_get, BOOL has_gpio_set, BOOL value, u8 gpio_no);

#endif /* _PHY_ICLI_UTIL_H_ */
