/*

 Vitesse Switch Software.

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

#ifndef IFINDEX_H
#define IFINDEX_H

/* interfaces ----------------------------------------------------------*/
#include "topo_api.h"
#include "msg_api.h"
#include "vlan_api.h"
#include "port_api.h"
#include "aggr_api.h"
#ifdef VTSS_SW_OPTION_IP2
#include "ip2_api.h"
#endif /* VTSS_SW_OPTION_IP2 */

/* mib2/ifTable - ifIndex type */
typedef enum {
    IFTABLE_IFINDEX_TYPE_PORT,
    IFTABLE_IFINDEX_TYPE_LLAG,
    IFTABLE_IFINDEX_TYPE_GLAG,
    IFTABLE_IFINDEX_TYPE_VLAN,
    IFTABLE_IFINDEX_TYPE_IP,
    IFTABLE_IFINDEX_TYPE_UNDEF
} ifIndex_type_t;

#define IFTABLE_IFINDEX_SWITCH_INTERVAL 1000
#define IFTABLE_IFINDEX_END             70000
typedef u_long  ifIndex_id_t;

typedef struct {
    ifIndex_id_t    ifIndex;
    ifIndex_type_t  type;
    vtss_isid_t     isid;
    u32             if_id;
} iftable_info_t;

/**
  * \brief Get the existent IfIndex.
  *
  * \param info [INOUT]:    The information parameter has following members\n
  *                       [IN] ifIndex: The ifIndex
  *                       [OUT] type: The interface type
  *                       [OUT] isid: The ISID, if the output type is neither IFINDEX_TYPE_PORT nor IFINDEX_TYPE_LLAG interface, the isid shouldn・t be modified.
  *                       [OUT] if_id: The interface ID
  * \return
  *    FALSE if the IfIndex is not available.\n
  *     Otherwise, return TRUE.
  */
BOOL ifIndex_get( iftable_info_t *info );

/**
  * \brief Get the valid IfIndex.
  *
  * \param info [INOUT]:    The information parameter has following members\n
  *                       [IN] ifIndex: The ifIndex
  *                       [OUT] type: The interface type
  *                       [OUT] isid: The ISID, if the output type is neither IFINDEX_TYPE_PORT nor IFINDEX_TYPE_LLAG interface, the isid shouldn・t be modified.
  *                       [OUT] if_id: The interface ID
  * \return
  *    FALSE if the IfIndex is not available.\n
  *     Otherwise, return TRUE.
  */
BOOL ifIndex_get_valid( iftable_info_t *info );

/**
  * \brief Get the next existent IfIndex.
  *
  * \param info [INOUT]:    The information parameter has following members\n
  *                       [INOUT] ifIndex: The next ifIndex
  *                       [OUT] type: The next interface type
  *                       [OUT] isid: The next ISID, if the output type is neither IFINDEX_TYPE_PORT nor IFINDEX_TYPE_LLAG interface, the isid shouldn・t be modified.
  *                       [OUT] if_id: the next interface ID
  * \return
  *    FALSE if the IfIndex is not available.\n
  *     Otherwise, return TRUE.
  */
BOOL ifIndex_get_next( iftable_info_t *info );

/**
  * \brief Get the next valid IfIndex.
  *
  * \param info [INOUT]:    The information parameter has following members\n
  *                       [INOUT] ifIndex: The next ifIndex
  *                       [OUT] type: The next interface type
  *                       [OUT] isid: The next ISID, if the output type is neither IFINDEX_TYPE_PORT nor IFINDEX_TYPE_LLAG interface, the isid shouldn・t be modified.
  *                       [OUT] if_id: the next interface ID
  * \return
  *    FALSE if the IfIndex is not available.\n
  *     Otherwise, return TRUE.
  */
BOOL ifIndex_get_next_valid( iftable_info_t *info );

/**
  * \brief Get the first existent IfIndex in the specific type.
  *
  * \param info [INOUT]:    The info parameter has following members\n
  *                       [IN] type: The next interface type
  *                       [OUT] ifIndex: The next ifIndex
  *                       [OUT] isid: The next ISID, if the output type is neither IFINDEX_TYPE_PORT nor IFINDEX_TYPE_LLAG interface, the isid shouldn・t be modified.
  *                       [OUT] if_id: The next interface ID
  * \return
  *    FALSE if there is no available ifIndex.\n
  *     Otherwise, return TRUE.
  */
BOOL ifIndex_get_first_by_type( iftable_info_t *info );

/**
  * \brief Get the existent IfIndex in the specific type.
  *
  * \param info [INOUT]:    The info parameter has following members\n
  *                       [IN] type: The interface type
  *                       [IN] if_id: The interface ID
  *                       [IN] isid: The ISID, if the output type is neither IFINDEX_TYPE_PORT nor IFINDEX_TYPE_LLAG interface, the isid shouldn・t be modified.
  *                       [OUT] ifIndex: The ifIndex
  * \return
  *    FALSE if there is no available ifIndex.\n
  *     Otherwise, return TRUE.
  */
BOOL ifIndex_get_by_interface( iftable_info_t *info );

#ifdef VTSS_SW_OPTION_IP2
BOOL get_next_ip(vtss_ip_addr_t *ip_addr, vtss_if_id_vlan_t   *if_id);
BOOL get_next_ip_status(u32 *if_id, vtss_if_status_t *status);
#endif /* VTSS_SW_OPTION_IP2 */

#endif                          /* IFINDEX_H */

