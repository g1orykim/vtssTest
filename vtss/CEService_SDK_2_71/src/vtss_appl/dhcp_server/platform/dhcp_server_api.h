/*

 Vitesse Switch Application software.

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
//----------------------------------------------------------------------------
/**
 *  \file
 *      dhcp_server_api.h
 *
 *  \brief
 *      DHCP Server data types and public APIs
 *
 *  \author
 *      CP Wang
 *
 *  \date
 *      05/09/2013 11:49
 */
//----------------------------------------------------------------------------
#ifndef __DHCP_SERVER_API_H__
#define __DHCP_SERVER_API_H__
//----------------------------------------------------------------------------

/*
==============================================================================

    Include File

==============================================================================
*/
#include "vtss_dhcp_server_type.h"

/*
==============================================================================

    Constant

==============================================================================
*/

/*
==============================================================================

    Macro

==============================================================================
*/

/*
==============================================================================

    Type Definition

==============================================================================
*/

#ifdef __cplusplus
extern "C" {
#endif

/*
==============================================================================

    Public Function

==============================================================================
*/
/**
 * \brief
 *      Start DHCP server.
 *
 * \param
 *      data [IN]: init data.
 *
 * \return
 *      vtss_rc: VTSS_OK on success.\n
 *               others are failed.
 */
vtss_rc dhcp_server_init(
    IN vtss_init_data_t     *data
);

/**
 * \brief
 *      error string for DHCP server.
 *
 * \param
 *      rc [IN]: error code.
 *
 * \return
 *      char *: error string.
 */
char *dhcp_server_error_txt(
    IN vtss_rc  rc
);

/**
 *  \brief
 *      reset DHCP server engine to default configuration.
 *
 *  \param
 *      n/a.
 *
 *  \return
 *      n/a.
 */
void dhcp_server_reset_to_default(
    void
);

/**
 * \brief
 *      get the time elapsed from system start in seconds.
 *      process wrap around.
 *
 * \param
 *      n/a.
 *
 * \return
 *      seconds from system start.
 */
u32 dhcp_server_current_time_get(
    void
);

/**
 *  \brief
 *      Enable/Disable DHCP server.
 *
 *  \param
 *      b_enable [IN]: TRUE - enable, FALSE - disable.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t dhcp_server_enable_set(
    IN  BOOL    b_enable
);

/**
 *  \brief
 *      Get if DHCP server is enabled or not.
 *
 *  \param
 *      b_enable [OUT]: TRUE - enable, FALSE - disable.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t dhcp_server_enable_get(
    OUT BOOL    *b_enable
);

/**
 *  \brief
 *      Add excluded IP address range.
 *      index : low_ip, high_ip
 *
 *  \param
 *      exclued_ip [IN]: IP address range to be excluded.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t dhcp_server_excluded_add(
    IN  dhcp_server_excluded_ip_t     *exclued_ip
);

/**
 *  \brief
 *      Delete excluded IP address range.
 *      index : low_ip, high_ip
 *
 *  \param
 *      exclued_ip [IN]: Excluded IP address range to be deleted.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t dhcp_server_excluded_delete(
    IN  dhcp_server_excluded_ip_t     *exclued_ip
);

/**
 *  \brief
 *      Get excluded IP address range.
 *      index : low_ip, high_ip
 *
 *  \param
 *      exclued_ip [IN] : index.
 *      exclued_ip [OUT]: Excluded IP address range data.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t dhcp_server_excluded_get(
    INOUT  dhcp_server_excluded_ip_t     *exclued_ip
);

/**
 *  \brief
 *      Get next of current excluded IP address range.
 *      index : low_ip, high_ip
 *
 *  \param
 *      exclued_ip [IN] : Currnet excluded IP address range index.
 *      exclued_ip [OUT]: Next excluded IP address range data to get.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t dhcp_server_excluded_get_next(
    INOUT  dhcp_server_excluded_ip_t     *exclued_ip
);

/**
 *  \brief
 *      Set DHCP pool.
 *      index: name
 *
 *  \param
 *      pool [IN]: new or modified DHCP pool.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t dhcp_server_pool_set(
    IN  dhcp_server_pool_t     *pool
);

/**
 *  \brief
 *      Delete DHCP pool.
 *      index: name
 *
 *  \param
 *      pool [IN]: DHCP pool to be deleted.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t dhcp_server_pool_delete(
    IN  dhcp_server_pool_t     *pool
);

/**
 *  \brief
 *      Get DHCP pool.
 *      index: name
 *
 *  \param
 *      pool [IN] : index.
 *      pool [OUT]: DHCP pool data.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t dhcp_server_pool_get(
    INOUT  dhcp_server_pool_t     *pool
);

/**
 *  \brief
 *      Get next of current DHCP pool.
 *      index: name
 *
 *  \param
 *      pool [IN] : Currnet DHCP pool index.
 *      pool [OUT]: Next DHCP pool data to get.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t dhcp_server_pool_get_next(
    INOUT  dhcp_server_pool_t     *pool
);

/**
 *  \brief
 *      Set DHCP pool to be default value.
 *
 *  \param
 *      pool [OUT]: default DHCP pool.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t dhcp_server_pool_default(
    IN  dhcp_server_pool_t     *pool
);

/**
 *  \brief
 *      Clear DHCP server statistics.
 *
 *  \param
 *      n/a.
 *
 *  \return
 *      n/a.
 */
void dhcp_server_statistics_clear(
    void
);

/**
 *  \brief
 *      Get DHCP server statistics.
 *
 *  \param
 *      statistics [OUT]: DHCP server statistics data to get.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t dhcp_server_statistics_get(
    OUT dhcp_server_statistics_t  *statistics
);

/**
 *  \brief
 *      Delete DHCP binding.
 *      index: ip
 *
 *  \param
 *      binding [IN]: DHCP binding to be deleted.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t dhcp_server_binding_delete(
    IN  dhcp_server_binding_t     *binding
);

/**
 *  \brief
 *      Get DHCP binding.
 *      index: ip
 *
 *  \param
 *      binding [IN] : index.
 *      binding [OUT]: DHCP binding data.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t dhcp_server_binding_get(
    INOUT  dhcp_server_binding_t     *binding
);

/**
 *  \brief
 *      Get next of current DHCP binding.
 *      index: ip
 *
 *  \param
 *      binding [IN] : Currnet DHCP binding index.
 *      binding [OUT]: Next DHCP binding data to get.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t dhcp_server_binding_get_next(
    INOUT  dhcp_server_binding_t     *binding
);

/**
 *  \brief
 *      Clear DHCP bindings by binding type.
 *
 *  \param
 *      type - binding type
 *
 *  \return
 *      n/a.
 */
void dhcp_server_binding_clear_by_type(
    IN dhcp_server_binding_type_t     type
);

/**
 *  \brief
 *      Enable/Disable DHCP server per VLAN.
 *
 *  \param
 *      vid      [IN]: VLAN ID
 *      b_enable [IN]: TRUE - enable, FALSE - disable.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t dhcp_server_vlan_enable_set(
    IN  vtss_vid_t      vid,
    IN  BOOL            b_enable
);

/**
 *  \brief
 *      Get Enable/Disable DHCP server per VLAN.
 *
 *  \param
 *      vid      [IN] : VLAN ID
 *      b_enable [OUT]: TRUE - enable, FALSE - disable.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t dhcp_server_vlan_enable_get(
    IN  vtss_vid_t      vid,
    OUT BOOL            *b_enable
);

/**
 *  \brief
 *      add a declined IP. This API should be used for debug only.
 *      index: declined_ip
 *
 *  \param
 *      declined_ip [IN] : index.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t dhcp_server_declined_add(
    IN  vtss_ipv4_t     declined_ip
);

/**
 *  \brief
 *      delete a declined IP. This API should be used for debug only.
 *      index: declined_ip
 *
 *  \param
 *      declined_ip [IN] : index.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t dhcp_server_declined_delete(
    IN  vtss_ipv4_t     declined_ip
);

/**
 *  \brief
 *      Get declined IP.
 *      index: declined_ip
 *
 *  \param
 *      declined_ip [IN] : index.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t dhcp_server_declined_get(
    IN  vtss_ipv4_t     *declined_ip
);

/**
 *  \brief
 *      Get next declined IP.
 *      index: declined_ip
 *
 *  \param
 *      declined_ip [IN] : Currnet declined IP.
 *      declined_ip [OUT]: Next declined IP to get.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t dhcp_server_declined_get_next(
    INOUT   vtss_ipv4_t     *declined_ip
);

//----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
#endif //__DHCP_SERVER_API_H__
