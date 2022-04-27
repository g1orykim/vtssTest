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

#ifndef _VTSS_XXRP_TYPES_H_
#define _VTSS_XXRP_TYPES_H_

#if defined(__LINUX__)

#error "You must supply type definitions for your Linux OS"

#elif defined(VTSS_OPSYS_ECOS)

/* Types part of platform code */
#include "main.h"
#include "../src/vtss_xxrp_util.h"
#define VTSS_XXRP_ETH_TYPE_LEN                 2
#define VTSS_XXRP_MAC_ADDR_LEN                 6
#define VTSS_MRP_PEER_MAC_ADDRESS_SIZE         6
#define    XXRP_MAX_ATTRS                      4096                   /**< Maximum attrbutes supported by MRP. This should be equal
to the max attributes required by any MRP application    */
#define    XXRP_MAX_ATTRS_BF                   (XXRP_MAX_ATTRS / 8)   /**< Bitfield for maximum attrbutes supported by MRP          */
#define    XXRP_MAX_ATTR_EVENTS_ARR            (XXRP_MAX_ATTRS / 2)   /**< Maximum attrbute events array */
#define    XXRP_MAX_ATTR_TYPES                 1                      /**< For MVRP, this is 1, this will change when a new 
MRP application is added                                 */
#define MRP_MSTI_MAX 8 /* TODO :: later you  assign the VTSS_MSTP_API.h */

/***************************************************************************************************
* MRP application types
*
* Additional types such as MMRP, GVRP etc. can be defined here
**************************************************************************************************/
/**
 * \brief enumeration to list MRP applications.
 **/
typedef enum {



#ifdef VTSS_SW_OPTION_GVRP
    VTSS_GARP_APPL_GVRP,
#endif
    VTSS_MRP_APPL_MAX  /* Always last entry! */
} vtss_mrp_appl_t;

/**
 * \brief structure to hold the timer values
 **/
typedef struct {
    u32 join_timer;         /**< Join timer     */
    u32 leave_timer;        /**< Leave timer    */
    u32 leave_all_timer;    /**< LeaveAll timer */
} vtss_mrp_timer_conf_t;










/**
 * \brief Depending on the application, the following union will be parsed.
 **/
typedef union {
    u32                        dummy; /* At least one member in the union */



} vtss_mrp_attribute_type_t;

/**
 * \brief MSTP port state change enumeration.
 **/
typedef enum {
    VTSS_MRP_MSTP_PORT_ADD,    /**< Port moving to FORWARDING state       */
    VTSS_MRP_MSTP_PORT_DELETE  /**< Port moving out of FORWARDING state   */
} vtss_mrp_mstp_port_state_change_type_t;

/**
 * \brief MSTP port role change enumeration.
 **/
typedef enum {
    VTSS_MRP_MSTP_PORT_ROOT_OR_ALTERNATE_TO_DESIGNATED, /**< Port role to designated      */
    VTSS_MRP_MSTP_PORT_DEESIGNATED_TO_ROOT_OR_ALTERNATE /**< Port role to non-designated  */
} vtss_mrp_mstp_port_role_change_type_t;

/**
 * \brief MRP statistics enumeration.
 **/
typedef enum {
    VTSS_MRP_TOTAL_RX_PKTS,
    VTSS_MRP_DROPPED_PKTS,
    VTSS_MRP_RX_NEW,
    VTSS_MRP_RX_JOININ,
    VTSS_MRP_RX_IN,
    VTSS_MRP_RX_JOINMT,
    VTSS_MRP_RX_MT,
    VTSS_MRP_RX_LV,
    VTSS_MRP_RX_LA,
    VTSS_MRP_TX_PKTS,
    VTSS_MRP_TX_NEW,
    VTSS_MRP_TX_JOININ,
    VTSS_MRP_TX_IN,
    VTSS_MRP_TX_JOINMT,
    VTSS_MRP_TX_MT,
    VTSS_MRP_TX_LV,
    VTSS_MRP_TX_LA
} vtss_mrp_stat_type_t;

/**
 * \brief structure to denote MRP statistics.
 **/
typedef struct {
    /* Rx statistics */
    u32  total_pkts_recvd;      /**< Count of total received packets        */
    u32  pkts_dropped;          /**< Count of packets dropped               */
    u32  new_recvd;             /**< Count of new events received           */
    u32  joinin_recvd;          /**< Count of joinin events received        */
    u32  in_recvd;              /**< Count of in received                   */
    u32  joinmt_recvd;          /**< Count of joinmt events received        */
    u32  mt_recvd;              /**< Count of mt events received            */
    u32  leave_recvd;           /**< Count of leave events received         */
    u32  leaveall_recvd;        /**< Count of leaveall events received      */
    /* Tx statistics */
    u32  pkts_transmitted;      /**< Count of packets transmitted           */
    u32  new_transmitted;       /**< Count of new events transmitted        */
    u32  joinin_transmitted;    /**< Count of joinin events transmitted     */
    u32  in_transmitted;        /**< Count of in events transmitted         */
    u32  joinmt_transmitted;    /**< Count of joinmt events transmitted     */
    u32  mt_transmitted;        /**< Count of mt events transmitted         */
    u32  leave_transmitted;     /**< Count of leave events transmitted      */
    u32  leaveall_transmitted;  /**< Count of leaveall events transmitted   */
} vtss_mrp_statistics_t;

/**
 *  \brief Structure to represent Applicant and Registrar states of an attribute.
 */
typedef struct vtss_mrp_mad_machine {
    u8 applicant;                                               /**<  Current Applicant state for an attribute                 */
    u8 registrar;                                               /**<  Current Registrar state for an attribute                 */
} vtss_mrp_mad_machine_t;

/**
 *  \brief Structure to represent MRP MAD for a port.
 */
typedef struct vtss_mrp_mad { /* mad */
    u32                    port_no;                                 /**< L2 port number                                             */
    i32                    join_timeout;                            /**< Configured join timeout                                    */
    i32                    leave_timeout;                           /**< Configure leave timeout                                    */
    i32                    leaveall_timeout;                        /**< Configured leaveall timeout                                */
    i32                    periodic_tx_timeout;                     /**< Configured periodic tx timeout                             */
    vtss_mrp_mad_machine_t *machines;                               /**< Applicant and registrar states for all the attributes      */
    u8                     peer_mac_address[VTSS_MRP_PEER_MAC_ADDRESS_SIZE];/**< Peer MAC address                                   */
    BOOL                   peer_mac_updated;                        /**< Flag to indicate whether peer_mac_address is updated       */
    BOOL                   join_timer_running;                      /**< Flag to indicate join timer is running                     */
    BOOL                   leaveall_timer_running;                  /**< Flag to indicate leaveall timer is running                 */
    BOOL                   periodic_timer_running;                  /**< Flag to indicate periodic timer is running                 */
    u8                     leave_timer_running[XXRP_MAX_ATTRS_BF];  /**< Flag for each attribute to indicate leave timer is running */
    BOOL                   periodic_timer_control_status;           /**< Periodic timer control status                              */
    BOOL                   leaveall_stm_current_state;              /**< Current FSM state of leaveall timer                        */
    BOOL                   periodic_stm_current_state;              /**< Current FSM state of periodic timer                        */
    i16                    join_timer_count;                        /**< Current join timer expiry in cs. This is valid only when
                                                                      join_timer_running = TRUE                                     */
    i16                    leave_timer_count[XXRP_MAX_ATTRS];       /**< Current leave timer expiry in cs for all the attributes,
                                                                      valid when leave_timer_running = TRUE for an attribute        */
    i16                    leaveall_timer_count;                    /**< Current leaveall timer expiry in cs, valid only when
                                                                      leaveall_timer_running = TRUE                                 */
    i16                    periodic_timer_count;                    /**< Current periodic tx timer expiry in cs. This is valid only
                                                                      when periodic_timer_running = TRUE                            */
    BOOL                   join_timer_kick;
    u8                     leave_timer_kick[XXRP_MAX_ATTRS_BF];
    BOOL                   leaveall_timer_kick;
    BOOL                   periodic_timer_kick;
    BOOL                   attr_type_admin_status[XXRP_MAX_ATTR_TYPES]; /**< Attribute type control status for each attribute type  */
    vtss_mrp_statistics_t  stats;                                   /**< MRP statistics                                             */
    //u32                  failed_to_register;                      /**< TODO: Not the correct place for this count                 */
    //uint                 is_point_to_point : 1;                   /**< This field may not be required as we can call the reqd func*/
} vtss_mrp_mad_t;

/**
 *  \brief Structure to represent MRP MAP for a port.
 */
typedef struct vtss_map_port_s {
    u32                    port;                                    /**< Port number                                            */
    BOOL                   tc_detected[MRP_MSTI_MAX];               /**< tc_detected flag for each MSTI instance                */
unsigned int           is_connected :
    MRP_MSTI_MAX;             /**< each bit represents an MSTI instance. If it is set,
                                                                      this port is connected in that MSTI                       */
    void                   *next_in_port_ring;                      /**< pointer to connect all the ports for which MRP is
                                                                      enabled and port is in FORWARDING state                   */
    void                   *next_in_connected_ring[MRP_MSTI_MAX];   /**< ointer to connect all the ports of a MSTI instance     */
} vtss_map_port_t;

/**
 *  \brief Structure to represent MRP application.
 *  Each MRP, i.e., each instance of an application that uses the MRP
 *  protocol, is represented as a struct or control block with common
 *  initial fields. These comprise pointers to application-specific
 *  functions that are by the MAD and MAP components to signal protocol
 *  events to the application, and other controls common to all
 *  applications. The pointers include a pointer to the instances of MAD
 *  (one per port) for the application,and to MAP (one per application).
 *  The signaling functions include the addition and removal of ports,
 *  which the application should use to initialize port attributes with
 *  any management state required.
 */
typedef struct {
    vtss_mrp_appl_t appl_type;                                                          /**< Application type MVRP or MMRP  */
    vtss_mrp_mad_t  **mad;                                                              /**< pointer to MAD pointer array   */
    vtss_map_port_t **map;                                                              /**< pointer to MAP pointer array   */
    u32             max_mad_index;                                                      /**< Maximum attributes             */
    u32             last_mad_used;                                                      /**< TODO                           */
    void (*join_indication_fn)(u32 port_no, vtss_mrp_attribute_type_t *attr_type,
                               u16 joining_mad_index, BOOL new);                        /**< Join indication function       */
    void (*leave_indication_fn)(u32 port_no, vtss_mrp_attribute_type_t *attr_type,
                                u32 leaving_mad_index);                                 /**< Leave indication function      */
    void (*join_propagated_fn)(void *, void *mad, u32 joining_mad_index, BOOL new);     /**< Join propagation function      */
    void (*leave_propagated_fn)(void *, void *mad, u32 leaving_mad_index);              /**< Leave propagation function     */
    u32  (*transmit_fn)(u32 port_no, u8 *all_attr_events, u32 no_events, BOOL la_flag); /**< PDU transmit function          */
    BOOL (*receive_fn)(u32 port_no, const u8 *pdu, u32 length);                         /**< PDU receive function           */
    void (*added_port_fn)(void *, u32 port_no);                                         /**< Port add function              */
    void (*removed_port_fn)(void *, u32 port_no);                                       /**< Port remove function           */
    void *appl_glob_data;      /* Used to Pass the Application Global Data */           /**< TODO                           */
} vtss_mrp_t;

#else

#error "You must supply type definitions for your platform OS"

#endif

#endif /* _VTSS_XXRP_TYPES_H_ */
