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

#ifndef _COMMON_OS_API_H_
#define _COMMON_OS_API_H_

#include "vtss_common_os.h"
#include "port_api.h"           /* For port state structure */
#ifdef VTSS_SW_OPTION_AGGR
#include "aggr_api.h"           /* For AGGR_LLAG_CNT */
#else
#define AGGR_LLAG_CNT 0
#define AGGR_MGMT_GROUP_NO_START 1
#endif

/* L2_MAX_POAGS */
typedef unsigned int l2_port_no_t;

/* From old API */
typedef vtss_port_no_t vtss_poag_no_t;

#define L2_MAX_SWITCH_PORTS      (VTSS_PORTS)
#define L2_MAX_PORTS             (L2_MAX_SWITCH_PORTS * VTSS_ISID_CNT)
#define L2_MAX_LLAGS             (AGGR_LLAG_CNT * VTSS_ISID_CNT)
#if defined(VTSS_FEATURE_AGGR_GLAG)
#define L2_MAX_GLAGS	         (VTSS_GLAGS   /* Global */ )
#else
#define L2_MAX_GLAGS	         0
#endif
#define L2_MAX_POAGS             (L2_MAX_PORTS+L2_MAX_LLAGS+L2_MAX_GLAGS)

/* l2port mapping functions */
BOOL l2port2port(l2_port_no_t, vtss_isid_t*, vtss_port_no_t*);
BOOL l2port2poag(l2_port_no_t, vtss_isid_t*, vtss_poag_no_t*);
BOOL l2port2glag(l2_port_no_t, vtss_glag_no_t*);
const char *l2port2str(l2_port_no_t l2port);

BOOL l2port_is_valid(l2_port_no_t);
BOOL l2port_is_port(l2_port_no_t);
BOOL l2port_is_poag(l2_port_no_t);
BOOL l2port_is_glag(l2_port_no_t);

#define L2PORT2PORT(isid, port) ( ((isid-1)*L2_MAX_SWITCH_PORTS)       + (port) )
#define L2LLAG2PORT(isid, llag) ( L2_MAX_PORTS + ((isid-1)*AGGR_LLAG_CNT) + ((llag) - VTSS_AGGR_NO_START) )
#define L2GLAG2PORT(glag)       ( L2_MAX_PORTS + L2_MAX_LLAGS          + ((glag) - VTSS_AGGR_NO_START) )

void l2_receive_indication(vtss_module_id_t modid, 
                           const void *packet,
                           size_t len, 
                           vtss_port_no_t switchport,
                           vtss_vid_t vid,
                           vtss_glag_no_t glag_no);

typedef void (*l2_stack_rx_callback_t)(const void *packet, 
                                       size_t len, 
                                       vtss_vid_t vid,
                                       l2_port_no_t l2port);

void l2_receive_register(vtss_module_id_t modid, l2_stack_rx_callback_t cb);

#define N_L2_MSTI_MAX	8

#if defined(VTSS_SW_OPTION_RSTP)
void l2_flush(l2_port_no_t, vtss_common_vlanid_t vlan_id, BOOL exclude, uchar reason);
#endif
#if defined(VTSS_SW_OPTION_MSTP)
void l2_flush_port(l2_port_no_t l2port);

void l2_flush_vlan_port(l2_port_no_t l2port,vtss_common_vlanid_t vlan_id);

/**
 * Set the MSTP MSTI mapping table
 */
vtss_rc
l2_set_msti_map(BOOL all_to_cist, size_t maplen, const uchar *map);

/**
 * Set the Spanning Tree state of all MSTIs for a specific port.
 */
void l2_set_msti_stpstate_all(vtss_common_port_t portno, vtss_stp_state_t new_state);

/**
 * Set the Spanning Tree state of a specific MSTI port.
 */
void l2_set_msti_stpstate(uchar msti, vtss_common_port_t portno, vtss_stp_state_t new_state);

/**
 * Get the Spanning Tree state of a specific MSTI port.
 */
vtss_stp_state_t l2_get_msti_stpstate(uchar msti, vtss_common_port_t portno);

/**
 * Set the Spanning Tree states (port, MSTIs) of a port to that of another.
 */
void l2_sync_stpstates(vtss_common_port_t copy, vtss_common_port_t master);

#endif

vtss_rc l2_init(vtss_init_data_t *data);

/* STP state change callback */
typedef void (*l2_stp_state_change_callback_t)(vtss_common_port_t l2port, vtss_common_stpstate_t new_state);

/* STP state change callback registration */
vtss_rc l2_stp_state_change_register(l2_stp_state_change_callback_t callback);

/* STP MSTI state change callback */
typedef void (*l2_stp_msti_state_change_callback_t)(vtss_common_port_t l2port, 
                                                    uchar msti,
                                                    vtss_common_stpstate_t new_state);

/* STP MSTI state change callback registration */
vtss_rc l2_stp_msti_state_change_register(l2_stp_msti_state_change_callback_t callback);

typedef enum {
    L2PORT_ITER_TYPE_PHYS     = 0x01,                  /**< This is a physical port.            */
    L2PORT_ITER_TYPE_LLAG     = 0x02,                  /**< This is a local aggregation.        */
    L2PORT_ITER_TYPE_GLAG     = 0x04,                  /**< This is a global aggregation.       */
    L2PORT_ITER_TYPE_AGGR     = L2PORT_ITER_TYPE_LLAG|L2PORT_ITER_TYPE_GLAG,
                                                       /**< This is any aggregation type.       */
    L2PORT_ITER_TYPE_ALL      = 0x07,                  /**< All l2port types.                   */
    /* Flags */
    L2PORT_ITER_ISID_ALL      = 0x40,                  /**< Iterate all isids - present or not. */
    L2PORT_ITER_PORT_ALL      = 0x80,                  /**< Iterate all ports - present or not. */
    L2PORT_ITER_ISID_CFG      = 0x100,                 /**< Iterate configurable ISIDs.         */
    L2PORT_ITER_USID_ORDER    = 0x200,                 /**< Get ISIDs in USID order.            */
    L2PORT_ITER_ALL           = L2PORT_ITER_ISID_ALL|L2PORT_ITER_PORT_ALL,
                                                       /**< Iterate all - present or not.       */
} l2port_iter_type_t;

/**
 * l2port iterator structure - combines switch, port iterator into one
 * logical unit.
 **/
typedef struct {

    /* Public members */
    l2_port_no_t             l2port;        /**< The current l2port */
    l2port_iter_type_t       type;          /**< The current l2port type (mask) */
    vtss_isid_t              isid;          /**< The current isid. */
    vtss_usid_t              usid;          /**< The current usid. */
    vtss_port_no_t           iport;         /**< The current iport */
    vtss_port_no_t           uport;         /**< The current uport */

    /* Private members - don't use! */
    l2port_iter_type_t itertype_req, itertype_pend;
    vtss_isid_t isid_req;
    int ix;
    switch_iter_sort_order_t s_order;
    port_iter_sort_order_t p_order;
    switch_iter_t sit;
    port_iter_t pit;

} l2port_iter_t;

#define L2PIT_TYPE(p, t) (((p)->type) & (t))

/**
 * \brief Initialize a l2port iterator.
 *
 * \param l2pit      [IN] L2Port iterator.
 *
 * \param isid       [IN] ISID to iterate (or VTSS_ISID_GLOBAL)
 *
 * \param l2type     [IN] Types of l2ports to iterate
 *
 * \return Return code (always VTSS_OK).
 **/
vtss_rc l2port_iter_init(l2port_iter_t *l2pit, vtss_isid_t isid, l2port_iter_type_t l2type);

/**
 * \brief Get the next l2port.
 *
 * \param l2pit      [IN] L2Port iterator.
 *
 * \return TRUE if a l2port is found, otherwise FALSE.
 **/
BOOL l2port_iter_getnext(l2port_iter_t *l2pit);

#endif // _RSTP_API_H_


// ***************************************************************************
// 
//  End of file.
// 
// ***************************************************************************
