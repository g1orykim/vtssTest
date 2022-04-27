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

#ifndef _VTSS_XXRP_CALLOUT_H_
#define _VTSS_XXRP_CALLOUT_H_

#include "vtss_xxrp_types.h"

/**
 * \file vtss_xxrp_callout.h
 * \brief XXRP host interface header file
 *
 * This file contain the definitions of functions provided by the host
 * system to the XXRP protocol entity. Thus, a given system must provide
 * implementations of these functions according the specified interface.
 *
 */
void *xxrp_sys_malloc(u32 size, char *file, const char *function, u32 line);

u32 xxrp_sys_free(void *ptr, char *file, const char *function, u32 line);

#define XXRP_SYS_MALLOC(size) xxrp_sys_malloc(size, __FILE__, __FUNCTION__, __LINE__ );
#define XXRP_SYS_FREE(ptr)    xxrp_sys_free(ptr, __FILE__, __FUNCTION__, __LINE__);

/**
 * \brief MRP timer kick.
 *
 * This function is called by the base part in order to start the MRP timer.
 * See the description of vtss_mrp_timer_tick() for more information.
 *
 * \return Nothing.
 */
void vtss_mrp_timer_kick(void);

/**
 * \brief MRP crit enter.
 *
 * This function is called when the base part wants to enter the critical section.
 *
 * \return Nothing.
 */
void vtss_mrp_crit_enter(vtss_mrp_appl_t app);

/**
 * \brief MRP crit exit.
 *
 * This function is called when the base part wants to exit the critical section.
 *
 * \return Nothing.
 */
void vtss_mrp_crit_exit(vtss_mrp_appl_t app);

/**
 * \brief MRP crit assert locked.
 *
 * This function is called when the base part wants to verify that the critical section has been entered.
 *
 * \return Nothing.
 */
void vtss_mrp_crit_assert_locked(vtss_mrp_appl_t app);

/**
 *  \brief Get MAD port status.
 *
 * This function is called when the base part wants the port msti status.
 *
 */
vtss_rc mrp_mstp_port_status_get(u8 msti, u32 l2port);

vtss_rc mrp_mstp_index_msti_mapping_get(u32 attr_index, u8 *msti);


typedef void *vtss_mrp_tx_context_t;

/**
 * \brief MRP MRPDU tx allocate function.
 *
 * Allocates a transmit buffer.
 *
 * \param port_no [IN]  L2 port number.
 * \param length  [IN]  Length of MRPDU.
 * \param context [OUT] Pointer to transmit context.
 *
 * \return Return pointer to allocated MRPDU buffer or NULL.
 **/

void *vtss_mrp_mrpdu_tx_alloc(u32 port_no, u32 length, vtss_mrp_tx_context_t *context);

/**
 * \brief MRP MRPDU tx free function.
 *
 * Free a transmit buffer.
 * Use this if a buffer has been allocated but the transmit needs to be cancelled.
 *
 * \param mrpdu   [IN]  Pointer to MRPDU.
 * \param context [IN]  Transmit context provided by vtss_mrp_mrpdu_tx_alloc().
 *
 * \return Nothing.
 **/
void vtss_mrp_mrpdu_tx_free(void *mrpdu, vtss_mrp_tx_context_t context);

/**
 * \brief MRP MRPDU tx function.
 *
 * Inserts an SMAC address based on the port_no and transmits the MRPDU.
 * The allocated buffer is automatically free'ed.
 *
 * \param port_no [IN]  L2 port number.
 * \param mrpdu   [IN]  Pointer to MRPDU.
 * \param length  [IN]  Length of MRPDU.
 * \param context [IN]  Transmit context provided by vtss_mrp_mrpdu_tx_alloc().
 *
 * \return Return TRUE if MRPDU is transmitted successfully, FALSE otherwise.
 **/
BOOL vtss_mrp_mrpdu_tx(u32 port_no, void *mrpdu, u32 length, vtss_mrp_tx_context_t context);









#ifdef VTSS_SW_OPTION_GVRP
u32 XXRP_gvrp_vlan_port_membership_add(u32 port_no, vtss_vid_t vid);
u32 XXRP_gvrp_vlan_port_membership_del(u32 port_no, vtss_vid_t vid);
#endif

BOOL XXRP_is_port_point2point(u32 port_no);

#endif /* _VTSS_XXRP_CALLOUT_H_ */
