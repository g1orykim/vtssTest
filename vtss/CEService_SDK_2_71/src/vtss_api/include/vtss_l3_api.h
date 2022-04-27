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

/**
 * \file
 * \brief L3 routing API
 * \details This header file describes L3 IPv4/IPv6 hardware assisted routing functions
 */

#ifndef _VTSS_L3_API_H_
#define _VTSS_L3_API_H_
#include <vtss_options.h>

#if defined(VTSS_ARCH_JAGUAR_1)
#include <vtss_types.h>

/** \page layer3 Layer 3
 *
 * The Layer 3 functions are used to control are used to control the
 * IPv4 and IPv6 routing routing.
 */

// TODO, move this??
#define VTSS_JR1_LPM_CNT  (1024u) /**< JR1 length of LPM table */
#define VTSS_JR1_ARP_CNT  (1024u) /**< JR1 length of ARP table */
#define VTSS_JR1_RLEG_CNT  (128u) /**< JR1 length of RLEG table */

#if defined(VTSS_ARCH_JAGUAR_1)
#define VTSS_LPM_CNT  VTSS_JR1_LPM_CNT  /**< Length of LPM table */
#define VTSS_ARP_CNT  VTSS_JR1_ARP_CNT  /**< Length of LPM table */
#define VTSS_RLEG_CNT VTSS_JR1_RLEG_CNT /**< Length of LPM table */
#define VTSS_ARP_IPV4_RELATIONS VTSS_ARP_CNT /**< Length of IPv4 ARP table */
#define VTSS_ARP_IPV6_RELATIONS VTSS_ARP_CNT /**< Length of IPv4 ARP table */
#endif


/** \brief Router leg ID */
typedef u32 vtss_l3_rleg_id_t;

/** \brief MAC addressing mode for routing legs */
typedef enum
{
    VTSS_ROUTING_RLEG_MAC_MODE_INVALID = 0, /**< The addressing mode
                                                  has still not been
                                                  configured */

    VTSS_ROUTING_RLEG_MAC_MODE_SINGLE = 1, /**< One common MAC address
                                                is used for all legs */
} vtss_l3_rleg_common_mode_t;

/** \brief Common configurations for all routing legs */
typedef struct
{
    vtss_l3_rleg_common_mode_t rleg_mode;      /**< Common rleg-mode for all
                                                    routing legs. */
    vtss_mac_t                 base_address;   /**< Base mac address used to
                                                    derive addresses for all
                                                    routing legs. */
    BOOL                       routing_enable; /**< Globally enable/disable
                                                    routing. */
} vtss_l3_common_conf_t;

/** \brief Router leg control structure */
typedef struct
{
    BOOL ipv4_unicast_enable;       /**< Enable IPv4 unicast routing */
    BOOL ipv6_unicast_enable;       /**< Enable IPv6 unicast routing */
    BOOL ipv4_icmp_redirect_enable; /**< Enable IPv4 icmp redirect */
    BOOL ipv6_icmp_redirect_enable; /**< Enable IPv6 icmp redirect */
    vtss_vid_t          vlan;       /**< Vlan for which the router leg is
                                         instantiated */
    vtss_l3_rleg_id_t   rl_idx;     /**< Router leg index/number */
} vtss_l3_rleg_conf_t;

/** \brief Neighbour type */
typedef enum
{
    VTSS_L3_NEIGHBOUR_TYPE_INVALID = 0,
    VTSS_L3_NEIGHBOUR_TYPE_ARP = 1,
    VTSS_L3_NEIGHBOUR_TYPE_NDP = 2,
} vtss_l3_neighbour_type_t;

/** \brief Neighbour entry */
typedef struct
{
    vtss_mac_t         dmac; /**< MAC address of destination */
    vtss_vid_t         vlan; /**< VLAN of destination */
    vtss_ip_addr_t     dip;  /**< IP address of destination */
} vtss_l3_neighbour_t;

/**
 * \brief Flush all L3 configurations
 *
 * \param inst [IN]     Target instance reference.
 *
 * \return Return code.
 **/
vtss_rc vtss_l3_flush(const vtss_inst_t inst);


/**
 * \brief Get common router configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param conf [OUT]    Common routing configurations.
 *
 * \return Return code.
 **/
vtss_rc vtss_l3_common_get(const vtss_inst_t      inst,
                           vtss_l3_common_conf_t *const conf);

/**
 * \brief Set common router configuration.
 *
 * \param inst [IN]     Target instance reference.
 * \param conf [OUT]    Common routing configurations.
 *
 * \return Return code.
 **/
vtss_rc vtss_l3_common_set(const vtss_inst_t            inst,
                           const vtss_l3_common_conf_t *const conf);

/**
 * \brief Get all configured routes
 *
 * \param inst [IN]     Target instance reference.
 * \param cnt [OUT]     Amount of entries copied to output buffer
 * \param buf [OUT]     Output buffer
 *
 * \return Return code.
 **/
vtss_rc vtss_l3_rleg_get(const vtss_inst_t     inst,
                         u32                  *cnt,
                         vtss_l3_rleg_conf_t   buf[VTSS_RLEG_CNT]);

/**
 * \brief Add a router leg on the given VLAN
 *
 * \param inst [IN] Target instance reference.
 * \param conf [IN] Routing leg configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_l3_rleg_add(const vtss_inst_t          inst,
                         const vtss_l3_rleg_conf_t *const conf);

/**
 * \brief Delete a router leg associated with VLAN
 *
 * \param inst [IN]     Target instance reference.
 * \param vlan [IN]     VLAN to delete router leg from
 *
 * \return Return code.
 **/
vtss_rc vtss_l3_rleg_del(const vtss_inst_t         inst,
                         const vtss_vid_t          vlan);

#  if defined(VTSS_SW_OPTION_L3RT)
/**
 * \brief Get all configured routes
 *
 * \param inst [IN]     Target instance reference.
 * \param cnt [OUT]     Amount of entries copied to output buffer
 * \param buf [OUT]     Output buffer
 *
 * \return Return code.
 **/
vtss_rc vtss_l3_route_get(const vtss_inst_t     inst,
                          u32                  *cnt,
                          vtss_routing_entry_t  buf[VTSS_LPM_CNT]);

/**
 * \brief Add an entry to the routing table
 *
 * \param inst [IN]     Target instance reference.
 * \param entry [IN]    Route to add
 *
 * \return Return code.
 **/
vtss_rc vtss_l3_route_add(const vtss_inst_t             inst,
                          const vtss_routing_entry_t    * const entry);

/**
 * \brief Delete an entry from the routing table
 *
 * \param inst [IN]     Target instance reference.
 * \param entry [IN]    Entry to delete.
 *
 * \return Return code.
 **/
vtss_rc vtss_l3_route_del(const vtss_inst_t             inst,
                          const vtss_routing_entry_t    *const entry);

/**
 * \brief Get all configured neighbours
 *
 * \param inst [IN]     Target instance reference.
 * \param cnt [OUT]     Amount of entries copied to output buffer
 * \param buf [OUT]     Output buffer
 *
 * \return Return code.
 **/
vtss_rc vtss_l3_neighbour_get(const vtss_inst_t    inst,
                              u32                 *cnt,
                              vtss_l3_neighbour_t  buf[VTSS_ARP_CNT]);

/**
 * \brief Add a new entry to the neighbour cache.
 *
 * \param inst [IN]     Target instance reference.
 * \param entry [IN]    Entry to add.
 *
 * \return Return code.
 **/
vtss_rc vtss_l3_neighbour_add(const vtss_inst_t         inst,
                              const vtss_l3_neighbour_t * const entry);

/**
 * \brief Delete an entry from the neighbour  cache.
 *
 * \param inst [IN]     Target instance reference.
 * \param entry [IN]    Entry to delete.
 *
 * \return Return code.
 **/
vtss_rc vtss_l3_neighbour_del(const vtss_inst_t         inst,
                              const vtss_l3_neighbour_t * const entry);

/**
 * \brief Reset all routing leg statistics counters
 *
 * \param inst [IN]     Target instance reference.
 *
 * \return Return code.
 **/
vtss_rc vtss_l3_counters_reset(const vtss_inst_t inst);

/**
 * \brief Get routing system counters
 *
 * \param inst [IN]      Target instance reference.
 * \param counters [OUT] Counters
 *
 * \return Return code.
 **/
vtss_rc vtss_l3_counters_system_get(const vtss_inst_t  inst,
                                    vtss_l3_counters_t * const counters);

/**
 * \brief Get routing legs counters
 *
 * \param inst [IN]      Target instance reference.
 * \param vlan [IN]      Routing leg
 * \param counters [OUT] Counters
 *
 * \return Return code.
 **/
vtss_rc vtss_l3_counters_rleg_get(const vtss_inst_t       inst,
                                  const vtss_vid_t        vlan,
                                  vtss_l3_counters_t      * const counters);

/**
 * \brief Clear routing legs counters
 *
 * \param inst [IN]      Target instance reference.
 * \param vlan [IN]      Routing leg
 *
 * \return Return code.
 **/
vtss_rc vtss_l3_counters_rleg_clear(const vtss_inst_t       inst,
                                    const vtss_vid_t        vlan);


#endif /* VTSS_SW_OPTION_L3RT */
#endif /* VTSS_ARCH_JAGUAR_1*/
#endif /* _VTSS_L3_API_H_ */
