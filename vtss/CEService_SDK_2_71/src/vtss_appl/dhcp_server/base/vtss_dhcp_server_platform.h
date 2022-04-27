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
 *      vtss_dhcp_server_platform.h
 *
 *  \brief
 *      API used in base/ but implemented in platform/
 *
 *  \author
 *      CP Wang
 *
 *  \date
 *      05/08/2013 13:38
 */
//----------------------------------------------------------------------------
#ifndef __VTSS_DHCP_SERVER_PLATFORM_H__
#define __VTSS_DHCP_SERVER_PLATFORM_H__
//----------------------------------------------------------------------------

/*
==============================================================================

    Include File

==============================================================================
*/
#include "vtss_dhcp_server_type.h"
#include "vtss_dhcp_server_message.h"
#include "dhcp_server.h"

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
 *      get IP interface of VLAN
 *
 *  \param
 *      vid     [IN] : VLAN ID
 *      ip      [OUT]: IP address of the VLAN
 *      netmask [OUT]: Netmask of the VLAN
 *
 *  \return
 *      TRUE  : successful
 *      FALSE : failed
 */
BOOL dhcp_server_vid_info_get(
    IN  vtss_vid_t          vid,
    OUT vtss_ipv4_t         *ip,
    OUT vtss_ipv4_t         *netmask
);

/**
 * \brief
 *      register packet rx callback.
 *
 * \param
 *      n/a.
 *
 * \return
 *      n/a.
 */
void dhcp_server_packet_rx_register(
    void
);

/**
 * \brief
 *      deregister packet rx callback.
 *
 * \param
 *      n/a.
 *
 * \return
 *      n/a.
 */
void dhcp_server_packet_rx_deregister(
    void
);

/**
 * \brief
 *      send DHCP message.
 *
 * \param
 *      dhcp_message [IN]: DHCP message.
 *      option_len   [IN]: option field length.
 *      vid          [IN]: VLAN ID to send.
 *      sip          [IN]: source IP.
 *      dmac         [IN]: destination MAC.
 *      dip          [IN]: destination IP.
 *
 * \return
 *      TRUE  : successfully.
 *      FALSE : fail to send
 */
BOOL dhcp_server_packet_tx(
    IN  dhcp_server_message_t   *dhcp_message,
    IN  u32                     option_len,
    IN  vtss_vid_t              vid,
    IN  vtss_ipv4_t             sip,
    IN  u8                      *dmac,
    IN  vtss_ipv4_t             dip
);

/**
 *  \brief
 *      syslog message.
 *
 *  \param
 *      format [IN] : message format.
 *      ...    [IN] : message parameters
 *
 *  \return
 *      n/a.
 */
void dhcp_server_syslog(
    IN  const char  *format,
    IN  ...
);

//----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
#endif //__VTSS_DHCP_SERVER_PLATFORM_H__
