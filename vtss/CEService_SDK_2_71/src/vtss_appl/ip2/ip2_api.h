/*

 Vitesse API software.

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

#ifndef _IP2_API_H_
#define _IP2_API_H_

#include "ip2_types.h"

/* Feature config ----------------------------------------------------------- */

static __inline__ BOOL vtss_ip2_hasipv6(void) __attribute__ ((const));
static __inline__ BOOL vtss_ip2_hasipv6(void)
{
#if defined(VTSS_SW_OPTION_IPV6)
    return TRUE;
#else
    return FALSE;
#endif
}

static __inline__ BOOL vtss_ip2_hasrouting(void) __attribute__ ((const));
static __inline__ BOOL vtss_ip2_hasrouting(void)
{
#if defined(VTSS_SW_OPTION_L3RT)
    return TRUE;
#else
    return FALSE;
#endif
}

static __inline__ BOOL vtss_ip2_hasdns(void) __attribute__ ((const));
static __inline__ BOOL vtss_ip2_hasdns(void)
{
#if defined(VTSS_SW_OPTION_DNS)
    return TRUE;
#else
    return FALSE;
#endif
}

/* Global config ----------------------------------------------------------- */
vtss_rc vtss_ip2_global_param_set(  const vtss_ip2_global_param_t *const p);

vtss_rc vtss_ip2_global_param_get(  vtss_ip2_global_param_t     *p);


/* Interface functions ----------------------------------------------------- */
BOOL    vtss_ip2_if_exists(         vtss_if_id_vlan_t            if_id);

// Returns error when no more interfaces exists. To get first id, usr
// vid=VTSS_VID_NULL as current.
vtss_rc vtss_ip2_if_id_next(        vtss_if_id_vlan_t            current,
                                    vtss_if_id_vlan_t           *const next);

// Get interface configuration
vtss_rc vtss_ip2_if_conf_get(       vtss_if_id_vlan_t            if_id,
                                    vtss_if_param_t             *const conf);

// Set interface configuration. If the interface does not allready
// exists, it will be added, otherwise it will be updated
vtss_rc vtss_ip2_if_conf_set(       vtss_if_id_vlan_t            if_id,
                                    const vtss_if_param_t       *const param);

// Delete an existing interface
vtss_rc vtss_ip2_if_conf_del(       vtss_if_id_vlan_t            if_id);

/* Gatherer status related to a given interface */
vtss_rc vtss_ip2_if_status_get(     vtss_if_status_type_t        type,
                                    vtss_if_id_vlan_t            id,
                                    const u32                    max,
                                    u32                         *cnt,
                                    vtss_if_status_t            *status);

/* Gets the first matched status entry for a given interface */
vtss_rc
vtss_ip2_if_status_get_first(       vtss_if_status_type_t        type,
                                    vtss_if_id_vlan_t            id,
                                    vtss_if_status_t            *status);

/* Like vtss_ip2_if_status_get, but for all interfaces */
vtss_rc vtss_ip2_ifs_status_get(    vtss_if_status_type_t        type,
                                    const u32                    max,
                                    u32                         *cnt,
                                    vtss_if_status_t            *status);

/* Inject raw packets into the ip-stack*/
vtss_rc vtss_ip2_if_inject(         vtss_if_id_vlan_t            if_id,
                                    u32                          length,
                                    const u8                    *const data);

/* Provide post-notifications for other modules when an interface has
 * changed */
typedef void (*vtss_ip2_if_callback_t)(vtss_if_id_vlan_t         if_id);
vtss_rc vtss_ip2_if_callback_add(   const vtss_ip2_if_callback_t cb);
vtss_rc vtss_ip2_if_callback_del(   const vtss_ip2_if_callback_t cb);

/* Fill in default settings in configurations structs */
void vtss_if_default_param(         vtss_if_param_t             *param);

/* Callback functions to allow filtering of packages before they enter the IP
 * stack */
vtss_rc vtss_ip2_if_filter_reg(     vtss_ipstack_filter_cb_t     cb);
vtss_rc vtss_ip2_if_filter_unreg(   vtss_ipstack_filter_cb_t     cb);

/* Neighbour functions */
vtss_rc vtss_ip2_nb_clear(          vtss_ip_type_t               type);

vtss_rc vtss_ip2_nb_status_get(     vtss_ip_type_t               type,
                                    u32                          max,
                                    u32                         *cnt,
                                    vtss_neighbour_status_t     *status);

/* IP address functions ---------------------------------------------------- */
vtss_rc vtss_ip2_ipv4_conf_get(     vtss_if_id_vlan_t            if_id,
                                    vtss_ip_conf_t              *conf);

vtss_rc vtss_ip2_ipv4_conf_set(     vtss_if_id_vlan_t            if_id,
                                    const vtss_ip_conf_t        *conf);

vtss_rc vtss_ip2_ipv4_dhcp_restart( vtss_if_id_vlan_t            if_id);

vtss_rc vtss_ip2_ipv6_conf_get(     vtss_if_id_vlan_t            if_id,
                                    vtss_ip_conf_t              *conf);

vtss_rc vtss_ip2_ipv6_conf_set(     vtss_if_id_vlan_t            if_id,
                                    const vtss_ip_conf_t        *conf);

/* Check if an interface exists with the given IP address */
vtss_rc vtss_ip2_ip_to_if(          const vtss_ip_addr_t        *const ip,
                                    vtss_if_status_t            *if_status);

/* Returns an appropriate source address to use for clients on a given VLAN */
vtss_rc vtss_ip2_ip_by_vlan(        const vtss_vid_t             vlan,
                                    vtss_ip_type_t               type,
                                    vtss_ip_addr_t              *const src);

vtss_rc vtss_ip2_ip_by_port(        const vtss_port_no_t         iport,
                                    vtss_ip_type_t               type,
                                    vtss_ip_addr_t              *const src);

/* IP route functions ------------------------------------------------------ */
vtss_rc vtss_ip2_route_add(         const vtss_routing_entry_t  *const rt,
                                    const vtss_routing_params_t *const params);

vtss_rc vtss_ip2_route_del(         const vtss_routing_entry_t  *const rt,
                                    const vtss_routing_params_t *const params);

vtss_rc vtss_ip2_route_get(         vtss_routing_entry_type_t    type,
                                    u32                          max,
                                    vtss_routing_status_t       *rt,
                                    u32                         *const cnt);

/**
 * Return the next (of first) entry of the routing database.
 *
 * \param key (IN) - entry to use as input key. May be NULL, in which
 * case the first entry in the routing database is returned (if any).
 *
 * \param next (OUT) - the returned entry from the DB (see return
 * code).
 *
 * \note The routing entries are compared by: Type (IPv4/IPv6),
 * destination address, prefix length, next hop (gateway).
 *
 * \return VTSS_OK iff entry returned in 'next'.
 *
 */
vtss_rc vtss_ip2_route_getnext(     const vtss_routing_entry_t  *const key,
                                    vtss_routing_entry_t        *const next);

vtss_rc vtss_ip2_route_conf_get(    const int                    max,
                                    vtss_routing_entry_t        *rt,
                                    int                         *const cnt);

/**
 * Return information of a route in the routing database.
 *
 * \param key (IN) - entry to use as input key.
 *
 * \param info (OUT) - information about a route iff route is
 * found. May be NULL, in which case no information is returned.
 *
 * \return VTSS_OK iff entry is found.
 *
 */
vtss_rc vtss_ip2_route_get_info(    const vtss_routing_entry_t  *const key,
                                    vtss_routing_info_t         *const info);

/* Retrieve IP stack global information */
vtss_rc vtss_ip2_ips_status_get(    vtss_ips_status_type_t       type,
                                    const u32                    max,
                                    u32                         *cnt,
                                    vtss_ips_status_t           *status);

/**
 * Clear system's IP statistics.
 *
 * \param version (IN) - Specify the IP version (IPv4/IPv6) to clear counters.
 * version MUST be either IPv4 or IPv6.
 * \note Only stacking master is allowed for this operation.
 *
 * \return VTSS_OK iff operation is done successfully.
 *
 */
vtss_rc vtss_ip2_stat_syst_cntr_clear(vtss_ip_type_t version);

/**
 * Clear per IP interface's statistics.
 *
 * \param version (IN) - Specify the IP version (IPv4/IPv6) to clear counters.
 * \param ifidx (IN) - Specify the interface index to clear counters.
 * version MUST be either IPv4 or IPv6; ifidx MUST NOT be NULL.
 * The indexes used are IP version type (IPv4/IPv6) AND
 * interface type (VTSS_ID_IF_TYPE_VLAN/VTSS_ID_IF_TYPE_OS_ONLY) AND
 * valid interface id (vid from VTSS_ID_IF_TYPE_VLAN/ifno from VTSS_ID_IF_TYPE_OS_ONLY).
 * \note Only stacking master is allowed for this operation.
 *
 * \return VTSS_OK iff operation is done successfully.
 *
 */
vtss_rc vtss_ip2_stat_intf_cntr_clear(vtss_ip_type_t version, vtss_if_id_t *ifidx);

/**
 * Clear system's ICMP statistics.
 *
 * \param version (IN) - Specify the IP version (IPv4/IPv6) to clear counters.
 * version MUST be either IPv4 or IPv6.
 * \note Only stacking master is allowed for this operation.
 *
 * \return VTSS_OK iff operation is done successfully.
 *
 */
vtss_rc vtss_ip2_stat_icmp_cntr_clear(vtss_ip_type_t version);

/**
 * Return the counters of the matched ICMP message type.
 *
 * \param version (IN) - IP version used as input key.
 * \param icmp_msg (IN) - ICMP message type value used as input key.
 *  Zero is a valid index.
 *
 * \param entry (OUT) - the returned statistics from the IP stack.
 *
 * \note Only stacking master is allowed for this operation.
 *
 * \return VTSS_OK iff entry is found.
 *
 */
vtss_rc vtss_ip2_stat_imsg_cntr_get(vtss_ip_type_t              version,
                                    u32                         icmp_msg,
                                    vtss_ips_icmp_stat_t        *entry);

vtss_rc vtss_ip2_stat_imsg_cntr_getfirst(vtss_ip_type_t         version,
                                         u32                    icmp_msg,
                                         vtss_ips_icmp_stat_t   *entry);

vtss_rc vtss_ip2_stat_imsg_cntr_getnext(vtss_ip_type_t          version,
                                        u32                     icmp_msg,
                                        vtss_ips_icmp_stat_t    *entry);
/**
 * Clear per ICMP type's statistics.
 *
 * \param version (IN) - Specify the IP version (IPv4/IPv6) to clear counters.
 * \param type (IN) - Specify the ICMP message type to clear counters.
 * version MUST be either IPv4 or IPv6; type MUST be valid ICMP message value (0 ~ 255).
 * \note Only stacking master is allowed for this operation.
 *
 * \return VTSS_OK iff operation is done successfully.
 *
 */
vtss_rc vtss_ip2_stat_imsg_cntr_clear(vtss_ip_type_t version, u32 type);


/* CLI/ICLI helpers -------------------------------------------------------- */
void vtss_ip2_cli_init(             void);

typedef int vtss_ip2_cli_pr(        const char                  *fmt,
                                    ...);

int vtss_ip2_if_print(              vtss_ip2_cli_pr             *pr,
                                    BOOL                         vlan_only,
                                    vtss_if_status_type_t        type);

int vtss_ip2_if_brief_print(        vtss_ip2_cli_pr *pr);

int vtss_ip2_route_print(           vtss_routing_entry_type_t    type,
                                    vtss_ip2_cli_pr             *pr);

int vtss_ip2_nb_print(              vtss_ip_type_t               type,
                                    vtss_ip2_cli_pr             *pr);

/* Module initialization --------------------------------------------------- */
vtss_rc vtss_ip2_init(              vtss_init_data_t            *data);

#endif /* _IP2_API_H_ */
