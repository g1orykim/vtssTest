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
 
 $Id$
 $Revision$

*/

/**
 * \file
 * \brief QoS API
 * \details This header file describes Quality of Service functions
 */

#ifndef _VTSS_QOS_API_H_
#define _VTSS_QOS_API_H_

#include <vtss_options.h>
#include <vtss_types.h>
#if defined(VTSS_ARCH_SERVAL) || (defined VTSS_FEATURE_QCL_POLICY_ACTION)
#include <vtss_packet_api.h> /* Must be included before vtss_l2_api.h */
#include <vtss_l2_api.h>     /* For vtss_vcap_key_type_t */
#endif /* defined(VTSS_ARCH_SERVAL) || (defined VTSS_FEATURE_QCL_POLICY_ACTION) */

#if defined(VTSS_FEATURE_QOS)

#if defined(VTSS_ARCH_B2)
/**
 * \brief RED profile, 1-3
 **/
typedef u32 vtss_red_no_t;

#define VTSS_RED_NO_NONE  0 /**< No RED */
#define VTSS_RED_NO_START 1 /**< RED start number */
#define VTSS_RED_NO_COUNT 3 /**< RED count */
#define VTSS_RED_NO_END   (VTSS_RED_NO_START + VTSS_RED_NO_COUNT) /**< RED end number */

/**
 * \brief QoS result
 **/
typedef struct
{
    vtss_prio_t   prio; /**< Priority    (QoS class) */
    vtss_red_no_t red;  /**< RED profile (Drop Precedence level) */
} vtss_qos_result_t;

/**
 * \brief DSCP table number, 0-1
 **/
typedef u32 vtss_dscp_table_no_t;

#define VTSS_DSCP_TABLE_COUNT    2 /**< DSCP table count */
#define VTSS_DSCP_TABLE_NO_START 0 /**< DSCP table start number */
#define VTSS_DSCP_TABLE_NO_END   (VTSS_DSCP_TABLE_NO_START + VTSS_DSCP_TABLE_COUNT) /**< DSCP table end number */
#define VTSS_DSCP_TABLE_SIZE     64 /**< DSCP table size */
/**
 * \brief Entry in DSCP table
 **/
typedef struct
{
    BOOL              enable; /**< Enable/trust DSCP value */
    vtss_qos_result_t qos;    /**< QoS result */
} vtss_dscp_entry_t;

/**
 * \brief Get DSCP table.
 *
 * \param inst [IN]         Target instance reference.
 * \param table_no [IN]     DSCP table number.
 * \param dscp_table [OUT]  DSCP table.
 *
 * \return Return code.
 **/
vtss_rc vtss_dscp_table_get(const vtss_inst_t           inst,
                            const vtss_dscp_table_no_t  table_no,
                            vtss_dscp_entry_t           dscp_table[64]);


/**
 * \brief Set DSCP table.
 *
 * \param inst [IN]        Target instance reference.
 * \param table_no [IN]    DSCP table number.
 * \param dscp_table [IN]  DSCP table.
 *
 * \return Return code.
 **/
vtss_rc vtss_dscp_table_set(const vtss_inst_t           inst,
                            const vtss_dscp_table_no_t  table_no,
                            const vtss_dscp_entry_t     dscp_table[64]);
#endif /* VTSS_ARCH_B2 */

#if defined(VTSS_ARCH_B2)
/**
 * \brief Custom filter
 **/
typedef struct
{
    BOOL              forward;    /**< Permit/deny forwarding */
    vtss_qos_result_t qos;        /**< QoS result */
    struct
    {
        enum
        {
            VTSS_HEADER_L2 = 1,   /**< Layer 2 header */
            VTSS_HEADER_L3 = 2,   /**< Layer 3 header */
            VTSS_HEADER_L4 = 3    /**< Layer 4 header */
        } header;                 /**< Header offset */
        u32 offset;               /**< Offset in bytes, must be even */
        u16 val;                  /**< Value */
        u16 mask;                 /**< Mask */
    } filter[7];                  /**< Filter */
} vtss_custom_filter_t;

#define VTSS_CUSTOM_FILTER_COUNT 8 /**< Global custom filter count */

/**
 * \brief UDP/TCP pair
 **/
typedef struct
{
    BOOL range;                  /**< Set if the port pair is a range */
    struct
    {
        vtss_udp_tcp_t    port;  /**< Port number */
        vtss_qos_result_t qos;   /**< QoS result */
    } pair[2]; /**< UDP/TDP port pair */
} vtss_udp_tcp_pair_t;

#define VTSS_UDP_TCP_PAIR_COUNT 4 /**< Global UDP/TCP port pair count */
#endif /* VTSS_ARCH_B2 */

/** \brief DSCP **/
typedef u8 vtss_dscp_t;

#if defined(VTSS_FEATURE_QOS_WRED)
/**
 * \brief Random Early Detection configuration struct version 1 (per port, per queue)
 **/
typedef struct
{
    BOOL       enable;          /**< Enable/disable RED */
#if defined(VTSS_ARCH_JAGUAR_1)
    vtss_pct_t max_th;          /**< Maximum threshold */
    vtss_pct_t min_th;          /**< Minimum threshold */
#endif
#if defined(VTSS_ARCH_B2)
    u32        max;             /**< Maximum threshold */
    u32        min;             /**< Minimum threshold */
    u8         weight;          /**< Weight */
#endif
    vtss_pct_t max_prob_1;      /**< Drop probability at max_th for drop precedence level 1 */
    vtss_pct_t max_prob_2;      /**< Drop probability at max_th for drop precedence level 2 */
    vtss_pct_t max_prob_3;      /**< Drop probability at max_th for drop precedence level 3 */
} vtss_red_t;
#endif /* defined(VTSS_FEATURE_QOS_WRED) */

#if defined(VTSS_FEATURE_QOS_WRED_V2)
/**
 * \brief Random Early Detection configuration struct version 2 (per queue, per dpl - switch global)
 **/
typedef struct
{
    BOOL       enable;       /**< Enable/disable RED */
    vtss_pct_t min_fl;       /**< Minimum fill level */
    vtss_pct_t max;          /**< Maximum drop probability or fill level - selected by max_unit */
    enum {
        VTSS_WRED_V2_MAX_DP, /**< Unit for max is drop probability */
        VTSS_WRED_V2_MAX_FL  /**< Unit for max is fill level */
    } max_unit;              /**< Selects the unit for max */
} vtss_red_v2_t;
#endif /* defined(VTSS_FEATURE_QOS_WRED_V2) */

/**
 * \brief All parameters below are defined per chip
 **/
typedef struct
{
    vtss_prio_t        prios;           /**< Number of priorities (1/2/4/8) */

#if defined(VTSS_FEATURE_QOS_CLASSIFICATION_V2)
    BOOL               dscp_trust[64];         /**< Ingress: Only trusted DSCP values are used for QOS class and DP level classification  */
    vtss_prio_t        dscp_qos_class_map[64]; /**< Ingress: Mapping from DSCP value to QOS class  */
    vtss_dp_level_t    dscp_dp_level_map[64];  /**< Ingress: Mapping from DSCP value to DP level */
#endif /* VTSS_FEATURE_QOS_CLASSIFICATION_V2 */

#if defined(VTSS_FEATURE_QOS_DSCP_REMARK)
    BOOL               dscp_remark[64];        /**< Ingress: DSCP remarking enable. Used when port.dscp_mode = VTSS_DSCP_MODE_SEL */
#if defined(VTSS_FEATURE_QOS_DSCP_REMARK_V2)
    vtss_dscp_t        dscp_translate_map[64];                 /**< Ingress: Translated DSCP value. Used when port.dscp_translate = TRUE) */
    vtss_dscp_t        dscp_qos_map[VTSS_PRIO_ARRAY_SIZE];     /**< Ingress: Mapping from QoS class to DSCP (DP unaware or DP level = 0) */
    vtss_dscp_t        dscp_remap[64];                         /**< Egress: Remap one DSCP to another (DP unaware or DP level = 0) */
#if defined(VTSS_FEATURE_QOS_DSCP_REMARK_DP_AWARE)
    vtss_dscp_t        dscp_qos_map_dp1[VTSS_PRIO_ARRAY_SIZE]; /**< Ingress: Mapping from QoS class to DSCP (DP aware and DP level = 1) */
    vtss_dscp_t        dscp_remap_dp1[64];                     /**< Egress: Remap one DSCP to another (DP aware and DP level = 1) */
#endif /* VTSS_FEATURE_QOS_DSCP_REMARK_DP_AWARE */
#endif /* VTSS_FEATURE_QOS_DSCP_REMARK_V2 */
#endif /* VTSS_FEATURE_QOS_DSCP_REMARK */

#if defined(VTSS_FEATURE_QOS_POLICER_CPU_SWITCH)
    vtss_packet_rate_t policer_mac;     /**< MAC table CPU policer */
    vtss_packet_rate_t policer_cat;     /**< BPDU, GARP, IGMP, IP MC and MLD CPU policer */
    vtss_packet_rate_t policer_learn;   /**< Learn frame policer */
#endif /* defined(VTSS_FEATURE_QOS_POLICER_CPU_SWITCH) */
#if defined(VTSS_FEATURE_QOS_POLICER_UC_SWITCH)
    vtss_packet_rate_t policer_uc;      /**< Unicast packet policer */
#endif /* defined(VTSS_FEATURE_QOS_POLICER_UC_SWITCH) */
#if defined(VTSS_FEATURE_QOS_POLICER_MC_SWITCH)
    vtss_packet_rate_t policer_mc;      /**< Multicast packet policer */
#endif /* defined(VTSS_FEATURE_QOS_POLICER_MC_SWITCH) */
#if defined(VTSS_FEATURE_QOS_POLICER_BC_SWITCH)
    vtss_packet_rate_t policer_bc;      /**< Broadcast packet policer */
#endif /* defined(VTSS_FEATURE_QOS_POLICER_BC_SWITCH) */

#if defined(VTSS_ARCH_B2)
    vtss_udp_tcp_pair_t  udp_tcp[VTSS_UDP_TCP_PAIR_COUNT];        /**< UDP/TCP port pairs */
    vtss_custom_filter_t custom_filter[VTSS_CUSTOM_FILTER_COUNT]; /**< Custom filters */   
#endif /* VTSS_ARCH_B2 */
#if defined(VTSS_FEATURE_QOS_DOT3AR_RATE_LIMITER)
    u8                 header_size;  /**< Frame's header size to be subtracted from the payload by rate limiter */
#endif /* VTSS_FEATUTE_QOS_DOT3AR_RATE_LIMITER */

#if defined(VTSS_FEATURE_QOS_WRED_V2)
    vtss_red_v2_t      red_v2[VTSS_QUEUE_ARRAY_SIZE][2]; /**< Random Early Detection - per queue, per dpl */
#endif /* VTSS_FEATURE_QOS_WRED */
} vtss_qos_conf_t;

/**
 * \brief Get QoS setup for switch.
 *
 * \param inst [IN]   Target instance reference.
 * \param conf [OUT]  QoS setup structure.
 *
 * \return Return code.
 **/
vtss_rc vtss_qos_conf_get(const vtss_inst_t  inst,
                          vtss_qos_conf_t    *const conf);


/**
 * \brief Set QoS setup for switch.
 *
 * \param inst [IN]  Target instance reference.
 * \param conf [IN]  QoS setup structure.
 *
 * \return Return code.
 **/
vtss_rc vtss_qos_conf_set(const vtss_inst_t      inst,
                          const vtss_qos_conf_t  *const conf);


#if defined(VTSS_FEATURE_QCL)
/**
 * \brief QCL ID type
 **/
typedef u32 vtss_qcl_id_t; 

#define VTSS_QCL_ID_NONE    0xffffffff                       /**< Means QCLs disabled on port */
#define VTSS_QCL_IDS        1                                /**< Number of QCLs */
#define VTSS_QCL_ID_START   0                                /**< QCL ID start number */
#define VTSS_QCL_ID_END     (VTSS_QCL_ID_START+VTSS_QCL_IDS) /**< QCL ID end number */
#define VTSS_QCL_ARRAY_SIZE VTSS_QCL_ID_END                  /**< QCL ID array size */
#endif /* VTSS_FEATURE_QCL */

#if defined(VTSS_FEATURE_QOS_DSCP_REMARK)
/**
 * \brief DSCP mode for ingress port
 **/
typedef enum
{
    VTSS_DSCP_MODE_NONE,   /**< DSCP not remarked */
    VTSS_DSCP_MODE_ZERO,   /**< DSCP value zero remarked */
    VTSS_DSCP_MODE_SEL,    /**< DSCP values selected above (dscp_remark) are remarked */
    VTSS_DSCP_MODE_ALL     /**< DSCP remarked for all values */
} vtss_dscp_mode_t;

/**
 * \brief DSCP mode for egress port
 **/
typedef enum
{
    VTSS_DSCP_EMODE_DISABLE,   /**< DSCP not remarked */
    VTSS_DSCP_EMODE_REMARK,    /**< DSCP remarked with DSCP value from analyzer */
    VTSS_DSCP_EMODE_REMAP,     /**< DSCP remarked with DSCP value from analyzer remapped through global remap table  */
#if defined(VTSS_FEATURE_QOS_DSCP_REMARK_DP_AWARE)
    VTSS_DSCP_EMODE_REMAP_DPA  /**< DSCP remarked with DSCP value from analyzer remapped through global remap dp aware tables */
#endif /* VTSS_FEATURE_QOS_DSCP_REMARK_DP_AWARE */
} vtss_dscp_emode_t;
#endif /* VTSS_FEATURE_QOS_DSCP_REMARK */

/* Port policers */
#if defined(VTSS_ARCH_JAGUAR_1)
#define VTSS_PORT_POLICERS 4 /**< Number of port policers */
#elif defined(VTSS_ARCH_B2)
#define VTSS_PORT_POLICERS 2 /**< Number of port policers */
#else
#define VTSS_PORT_POLICERS 1 /**< Number of port policers */
#endif /* VTSS_ARCH_B2 */

/**
 * \brief Policer
 **/
typedef struct
{
#if defined(VTSS_ARCH_B2)
    BOOL               unicast;     /**< Unicast frames are policed */
    BOOL               multicast;   /**< Multicast frames are policed */
    BOOL               broadcast;   /**< Broadcast frames are policed */
#endif /* VTSS_ARCH_B2 */
    vtss_burst_level_t level;       /**< Burst level */
    vtss_bitrate_t     rate;        /**< Maximum rate */
} vtss_policer_t;

#if defined(VTSS_FEATURE_QOS_PORT_POLICER_EXT)

#define VTSS_PORT_POLICER_CPU_QUEUES 8 /**< Number of cpu queues pr port policer */

/**
 * \brief Policer Extensions
 **/
typedef struct
{
#if defined(VTSS_FEATURE_QOS_PORT_POLICER_EXT_FPS)
    BOOL               frame_rate;           /**< Measure rates in frames per seconds instead of bits per second */
#endif /* VTSS_FEATURE_QOS_PORT_POLICER_EXT_FPS */
#if defined(VTSS_FEATURE_QOS_PORT_POLICER_EXT_DPBL)
    vtss_dp_level_t    dp_bypass_level;      /**< Drop Predence bypass level */
#endif /* VTSS_FEATURE_QOS_PORT_POLICER_EXT_DPBL */
#if defined(VTSS_FEATURE_QOS_PORT_POLICER_EXT_TTM)
    BOOL               unicast;     /**< Unicast frames are policed */
    BOOL               multicast;   /**< Multicast frames are policed */
    BOOL               broadcast;   /**< Broadcast frames are policed */
    BOOL               uc_no_flood; /**< Exclude flooding unicast frames (if unicast is set) */
    BOOL               mc_no_flood; /**< Exclude flooding multicast frames (if multicast is set) */
    BOOL               flooded;     /**< Flooded frames are policed */
    BOOL               learning;    /**< Learning frames are policed */
    BOOL               to_cpu;      /**< Frames to the CPU are policed */
    BOOL               cpu_queue[VTSS_PORT_POLICER_CPU_QUEUES]; /**< Enable each individual CPU queue (if to_cpu is set) */
    BOOL               limit_noncpu_traffic; /**< Remove the front ports from the destination set for a policed frame */
    BOOL               limit_cpu_traffic;    /**< Remove the CPU ports from the destination set for a policed frame */
//    vtss_burst_level_t lower_level;          /**< Hysteris size for port policer */
#endif /* VTSS_FEATURE_QOS_PORT_POLICER_EXT_TTM */
#if defined(VTSS_FEATURE_QOS_PORT_POLICER_EXT_FC)
    BOOL               flow_control; /**< Flow control is enabled */
#endif /* VTSS_FEATURE_QOS_PORT_POLICER_EXT_FC */
} vtss_policer_ext_t;
#endif /* VTSS_FEATURE_QOS_PORT_POLICER_EXT */

#if defined(VTSS_FEATURE_QOS_POLICER_DLB)
/** \brief Dual leaky buckets policer configuration */
typedef enum {
    VTSS_POLICER_TYPE_MEF,    /**< MEF bandwidth profile */
    VTSS_POLICER_TYPE_SINGLE  /**< Single bucket policer (CIR/CBS) */
} vtss_policer_type_t;

/** \brief Dual leaky buckets policer configuration */
typedef struct {
    vtss_policer_type_t type;      /**< Policer type */
    BOOL                enable;    /**< Enable/disable policer */
#if defined(VTSS_ARCH_JAGUAR_1)
    BOOL                cm;        /**< Colour Mode (TRUE means colour aware) */
#endif /* VTSS_ARCH_JAGUAR_1 */
    BOOL                cf;        /**< Coupling Flag */
    BOOL                line_rate; /**< Line rate policing (default is data rate policing) */
    vtss_bitrate_t      cir;       /**< Committed Information Rate */
    vtss_burst_level_t  cbs;       /**< Committed Burst Size */
    vtss_bitrate_t      eir;       /**< Excess Information Rate */
    vtss_burst_level_t  ebs;       /**< Excess Burst Size */
} vtss_dlb_policer_conf_t;
#endif /* VTSS_FEATURE_QOS_POLICER_DLB */

#if defined(VTSS_ARCH_CARACAL)
/**
 * \brief Get MEP policer configuration.
 *
 * \param inst [IN]        Target instance reference.
 * \param port_no [IN]     Ingress port number.
 * \param prio [IN]        Selected priority (QoS class).
 * \param conf [OUT]       Policer configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_mep_policer_conf_get(const vtss_inst_t       inst,
                                  const vtss_port_no_t    port_no,
                                  const vtss_prio_t       prio,
                                  vtss_dlb_policer_conf_t *const conf);

/**
 * \brief Set MEP policer configuration.
 *
 * \param inst [IN]        Target instance reference.
 * \param port_no [IN]     Ingress port number.
 * \param prio [IN]        Selected priority (QoS class).
 * \param conf [IN]        Policer configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_mep_policer_conf_set(const vtss_inst_t             inst,
                                  const vtss_port_no_t          port_no,
                                  const vtss_prio_t             prio,
                                  const vtss_dlb_policer_conf_t *const conf);
#endif /* defined(VTSS_ARCH_CARACAL) */

#if defined(VTSS_ARCH_B2)
/**
 * \brief Classification order
 **/
typedef enum
{
    VTSS_QOS_ORDER_DEFAULT,
    VTSS_QOS_ORDER_L2,
    VTSS_QOS_ORDER_L3,
    VTSS_QOS_ORDER_CFI,
    VTSS_QOS_ORDER_DSCP_TRUSTED,
    VTSS_QOS_ORDER_UDP_TCP,
    VTSS_QOS_ORDER_IP_PROTO,
    VTSS_QOS_ORDER_DSCP_ALL,
    VTSS_QOS_ORDER_IPV4_ARP,
    VTSS_QOS_ORDER_IPV6,
    VTSS_QOS_ORDER_ETYPE,
    VTSS_QOS_ORDER_MPLS_EXP,
    VTSS_QOS_ORDER_VID,
    VTSS_QOS_ORDER_VLAN_TAG,
    VTSS_QOS_ORDER_LLC,
    VTSS_QOS_ORDER_GLOBAL_CUSTOM,
    VTSS_QOS_ORDER_LOCAL_CUSTOM,
    VTSS_QOS_ORDER_COUNT
} vtss_qos_order_t;



/**
 * \brief DMAC - VID - ETYPE struct
 **/
typedef struct
{
    struct
    {
        BOOL enable;     /**< Enable filter */
        struct
        {
            u8 value[6]; /**< DMAC value */
            u8 mask[6];  /**< DMAC mask */
        } dmac;
        struct
        {
            u16 value;   /**< VLAN ID value */
            u16 mask;    /**< VLAN ID mask */
        } vid;
        struct
        {
            u16 value;   /**< Ethernet type value */
            u16 mask;    /**< Ethernet type mask */
        } etype;
    } filter[2]; /**< Filters */
    BOOL             order_enable;                /**< Enable alternate ordering */
    vtss_qos_order_t order[VTSS_QOS_ORDER_COUNT]; /**< Alternate ordering */
} dmac_qos_vid_etype_t;
#endif /* VTSS_ARCH_B2 */

/**
 * \brief Weight for port WFQ: 1, 2, 4 or 8
 **/
typedef enum
{
    VTSS_WEIGHT_1,
    VTSS_WEIGHT_2,
    VTSS_WEIGHT_4,
    VTSS_WEIGHT_8
} vtss_weight_t;

#if defined(VTSS_FEATURE_QOS_DOT3AR_RATE_LIMITER)
/** \brief Rate limiting configuration */
typedef struct {
    BOOL frame_overhead_enable;   /**< Enable/Disable Frame overhead mode. IPG increased by a fixed value specified in frame_overhead */
    u32  frame_overhead;          /**< Extra frame overhead to be added to the IPG for every outgoing frame */
    BOOL payload_rate_enable;     /**< Enable/Disable payload rate limiting */
    u32  payload_rate;            /**< Target link bandwidth utilization rate */
    BOOL frame_rate_enable;       /**< Enable/Disable frame rate limiting */
    u32  frame_rate;              /**< Frame rate. IPG is adjusted based frame_length + frame_rate for every outgoing frame */
    BOOL preamble_in_payload;     /**< Include or exclude preamble size from payload size  calculation */

    BOOL header_in_payload;       /**< Include or exclude header size from payload size calculation */
    BOOL accumulate_mode_enable;  /**< IPG accumulate mode to support multiple rate limiters simultanelously */
} vtss_qos_port_dot3ar_rate_limiter_t;
#endif /* VTSS_FEATURE_QOS_DOT3AR_RATE_LIMITER */


/** \brief QoS setup per port **/
typedef struct
{
#if defined(VTSS_ARCH_B2)
    BOOL vlan_aware;      /**< Enable VLAN awareness */
    BOOL ctag_cfi_stop;   /**< Stop frame analysis if C-tag CFI is set */
    BOOL ctag_stop;       /**< Stop frame analysis if C-tag is found */
    BOOL tag_inner;       /**< Use inner tag if present */

    vtss_qos_order_t     order[VTSS_QOS_ORDER_COUNT];  /**< Primary ordering */
    dmac_qos_vid_etype_t dmac_vid_etype;               /**< DMAC/VID/EType filter */
    vtss_qos_result_t    default_qos;                  /**< Default QoS */
    vtss_qos_result_t    l2_control_qos;               /**< L2 control QoS */
    vtss_qos_result_t    l3_control_qos;               /**< L3 control QoS */
    vtss_qos_result_t    cfi_qos;                      /**< CFI QoS */
    vtss_qos_result_t    ipv4_arp_qos;                 /**< IPv4/ARP QoS */
    vtss_qos_result_t    ipv6_qos;                     /**< IPv6 QoS */
    vtss_qos_result_t    mpls_exp_qos[8];              /**< MPLS EXP QoS */
    vtss_qos_result_t    vlan_tag_qos[8];              /**< VLAN user priority QoS */
    vtss_qos_result_t    llc_qos;                      /**< LLC QoS */

    vtss_dscp_table_no_t dscp_table_no;            /**< DSCP table number */

    struct
    {
        vtss_udp_tcp_pair_t local;                                     /**< Local pair */
        BOOL                global_enable[VTSS_UDP_TCP_PAIR_COUNT][2]; /**< Global UDP/TCP filter pairs */
    } udp_tcp; /**< UDP/TCP port filter */

    struct
    {
        u8                proto;   /**< IP protocol */
        vtss_qos_result_t qos;     /**< QoS */
    } ip_proto; /**< IP protocol filter */                
                                
    struct                      
    {                           
        vtss_etype_t      etype;   /**< Ethernet type */
        vtss_qos_result_t qos;     /**< QoS */
    } etype; /**< Ethernet type filter */
    
    struct
    {
        vtss_vid_t        vid;     /**< VLAN ID */
        vtss_qos_result_t qos;     /**< QoS */
    } vlan[2]; /**< VLAN ID filter */

    struct
    {
        BOOL                 global_enable[VTSS_CUSTOM_FILTER_COUNT];   /**< Global custom filters */
        vtss_custom_filter_t local;                                     /**< Local custom filter */
    } custom_filter; /**< Custom filter */

    vtss_shaper_t  shaper_host;                               /**< Chip version -01: Host interface shaper for host mode 0, 1, 3, 4, 5 and 6 */

    struct
    {
        vtss_bitrate_t rate;                                  /**< Minimum guaranteed port rate */
        vtss_pct_t     queue_pct[VTSS_QUEUE_ARRAY_SIZE - 2];  /**< Queue percentages */
    } scheduler;                                              /**< Host interface scheduler used for host mode 0, 1, 3, 4, 5 and 6 */
#endif /* VTSS_ARCH_B2 */

#if defined(VTSS_FEATURE_QOS_WRED)
    vtss_red_t     red[VTSS_QUEUE_ARRAY_SIZE];                   /**< Random Early Detection */
#endif /* VTSS_FEATURE_QOS_WRED */

    vtss_policer_t policer_port[VTSS_PORT_POLICERS];             /**< Ingress port policers */

#if defined(VTSS_FEATURE_QOS_PORT_POLICER_EXT)
    vtss_policer_ext_t policer_ext_port[VTSS_PORT_POLICERS];     /**< Ingress port policers extensions */
#endif /* VTSS_FEATURE_QOS_PORT_POLICER_EXT */

#if defined(VTSS_FEATURE_QOS_QUEUE_POLICER)
    vtss_policer_t     policer_queue[VTSS_QUEUE_ARRAY_SIZE];     /**< Ingress queue policers */
#endif /* VTSS_FEATURE_QOS_QUEUE_POLICER */

    vtss_shaper_t  shaper_port;                                  /**< Egress port shaper */

#if defined(VTSS_FEATURE_QOS_EGRESS_QUEUE_SHAPERS)
    vtss_shaper_t shaper_queue[VTSS_QUEUE_ARRAY_SIZE];           /**< Egress queue shapers */
    BOOL          excess_enable[VTSS_QUEUE_ARRAY_SIZE];          /**< Allow this queue to use excess bandwidth */
#endif  /* VTSS_FEATURE_QOS_EGRESS_QUEUE_SHAPERS */

#if defined(VTSS_FEATURE_LAYER2)         
    vtss_prio_t    default_prio;                                 /**< Default port priority (QoS class) */
    vtss_tagprio_t usr_prio;                                     /**< Default Ingress VLAN tag priority (PCP) */
#endif /* VTSS_FEATURE_LAYER2 */

#if defined(VTSS_FEATURE_QOS_CLASSIFICATION_V2)
    vtss_dp_level_t   default_dpl;                                             /**< Default Ingress Drop Precedence level */
    vtss_dei_t        default_dei;                                             /**< Default Ingress DEI value  */
    BOOL              tag_class_enable;                                        /**< Ingress classification of QoS class and DP level based PCP and DEI */
    vtss_prio_t       qos_class_map[VTSS_PCP_ARRAY_SIZE][VTSS_DEI_ARRAY_SIZE]; /**< Ingress mapping for tagged frames from PCP and DEI to QOS class  */
    vtss_dp_level_t   dp_level_map[VTSS_PCP_ARRAY_SIZE][VTSS_DEI_ARRAY_SIZE];  /**< Ingress mapping for tagged frames from PCP and DEI to DP level */
    BOOL              dscp_class_enable;                                       /**< Ingress classification of QoS class and DP level based on DSCP */
#endif /* VTSS_FEATURE_QOS_CLASSIFICATION_V2 */
                                        
#if defined(VTSS_FEATURE_QOS_DSCP_REMARK)
    vtss_dscp_mode_t dscp_mode;                       /**< Ingress DSCP mode */
#if defined(VTSS_FEATURE_QOS_DSCP_REMARK_V2)
    vtss_dscp_emode_t dscp_emode;                     /**< Egress DSCP mode */
    BOOL              dscp_translate;                 /**< Ingress: Translate DSCP value via dscp_translate_map[DSCP] before use */
#endif /* VTSS_FEATURE_QOS_DSCP_REMARK_V2 */
#endif /* VTSS_FEATURE_QOS_DSCP_REMARK */

#if defined(VTSS_FEATURE_QOS_TAG_REMARK)
    BOOL             tag_remark;                                 /**< Egress tag priority mode */
    vtss_tagprio_t   tag_map[VTSS_PRIO_ARRAY_SIZE];              /**< Egress mapping from priority to tag priority */
#endif /* VTSS_FEATURE_QOS_TAG_REMARK */

#if defined(VTSS_FEATURE_QOS_TAG_REMARK_V2)
    vtss_tag_remark_mode_t tag_remark_mode;                      /**< Egress tag remark mode */
    vtss_tagprio_t         tag_default_pcp;                      /**< Default PCP value for Egress port */
    vtss_dei_t             tag_default_dei;                      /**< Default DEI value for Egress port */
    vtss_dei_t             tag_dp_map[4];                        /**< Egress mapping from 2 bit DP level to 1 bit DP level */
    vtss_tagprio_t         tag_pcp_map[VTSS_PRIO_ARRAY_SIZE][2]; /**< Egress mapping from QOS class and (1 bit) DP level to PCP */
    vtss_dei_t             tag_dei_map[VTSS_PRIO_ARRAY_SIZE][2]; /**< Egress mapping from QOS class and (1 bit) DP level to DEI */
#endif /* VTSS_FEATURE_QOS_TAG_REMARK_V2 */

#if defined(VTSS_FEATURE_QOS_WFQ_PORT)
    BOOL          wfq_enable;                                    /**< Weighted fairness queueing */
    vtss_weight_t weight[VTSS_QUEUE_ARRAY_SIZE];                 /**< Queue weights */
#endif /* VTSS_FEATURE_QOS_WFQ_PORT */

#if defined(VTSS_FEATURE_QOS_SCHEDULER_V2)
    BOOL       dwrr_enable;                                      /**< Enable Weighted fairness queueing */
    vtss_pct_t queue_pct[VTSS_QUEUE_ARRAY_SIZE - 2];             /**< Queue percentages */
#endif /* VTSS_FEATURE_QOS_SCHEDULER_V2 */

#if defined(VTSS_FEATURE_QOS_DOT3AR_RATE_LIMITER)
    vtss_qos_port_dot3ar_rate_limiter_t tx_rate_limiter;         /**< 802.3ar rate limiter */
#endif /* VTSS_FEATURE_QOS_DOT3AR_RATE_LIMITER */

#ifdef VTSS_FEATURE_QCL_DMAC_DIP
    BOOL       dmac_dip;                                         /**< Enable DMAC/DIP matching in QCLs (default SMAC/SIP) */
#endif /* VTSS_FEATURE_QCL_DMAC_DIP */

#if defined(VTSS_ARCH_SERVAL)
    vtss_vcap_key_type_t key_type;                               /**< Key type for received frames */
#endif /* defined(VTSS_ARCH_SERVAL) */

} vtss_qos_port_conf_t;

/**
 * \brief Get QoS setup for port.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param conf [OUT]    QoS setup structure.
 *
 * \return Return code.
 **/
vtss_rc vtss_qos_port_conf_get(const vtss_inst_t     inst,
                               const vtss_port_no_t  port_no,
                               vtss_qos_port_conf_t  *const conf);


/**
 * \brief Set QoS setup for port.
 *
 * \param inst [IN]    Target instance reference.
 * \param port_no [IN] Port number.
 * \param conf [IN]    QoS setup structure.
 *
 * \return Return code.
 **/
vtss_rc vtss_qos_port_conf_set(const vtss_inst_t           inst,
                               const vtss_port_no_t        port_no,
                               const vtss_qos_port_conf_t  *const conf);



#if defined(VTSS_ARCH_B2)
/**
 * \brief Logical port QoS setup for host mode 8, 9, 10 and 11
 **/
typedef struct
{
    vtss_shaper_t shaper;       /**< Chip version -01: Rx shaper seen from SPI-4 interface */
    struct
    {
        vtss_bitrate_t rx_rate; /**< Rx rate seen from SPI-4 interface */
        vtss_bitrate_t tx_rate; /**< Tx rate seen from SPI-4 interface */
    } scheduler; /**< Scheduler configuration */
} vtss_qos_lport_conf_t;
#endif /* VTSS_ARCH_B2 */


#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC) 
/**
 * \brief Logical port QoS setup for host mode
 **/
typedef struct
{
    vtss_shaper_t shaper;                      /**< Host port shaper */
    vtss_pct_t    lport_pct;                   /**< Host port DWRR percentage */
    vtss_red_t    red[VTSS_PRIOS];             /**< Random Early Detection configurations*/
    struct
    {
        vtss_bitrate_t rate;                   /**< Min guaranted port rate */
        vtss_pct_t     queue_pct[6];           /**< DWRR queue percentage */
        vtss_shaper_t  queue[2];               /**< Strict queue shaper */
    } scheduler; /**< Scheduler configuration */
} vtss_qos_lport_conf_t;
#endif /* VTSS_ARCH_JAGUAR_1_CE_MAC */

#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC) || defined(VTSS_ARCH_B2)
/**
 * \brief Get QoS setup for logical port.
 *
 * \param inst [IN]      Target instance reference.
 * \param lport_no [IN]  Logical port number.
 * \param qos [OUT]      QoS setup structure.
 *
 * \return Return code.
 **/
vtss_rc vtss_qos_lport_conf_get(const vtss_inst_t       inst,
                                 const vtss_lport_no_t   lport_no,
                                 vtss_qos_lport_conf_t  *const qos);

/**
 * \brief Set QoS setup for logical port.
 *
 * \param inst [IN]      Target instance reference.
 * \param lport_no [IN]  Logical port number.
 * \param qos [IN]       QoS setup structure.
 *
 * \return Return code.
 **/
vtss_rc vtss_qos_lport_conf_set(const vtss_inst_t             inst,
                                 const vtss_lport_no_t         lport_no,
                                 const vtss_qos_lport_conf_t  *const qos);
#endif /* (VTSS_ARCH_JAGUAR_1_CE_MAC) || defined(VTSS_ARCH_B2) */



#if defined(VTSS_FEATURE_QCL)
/**
 * \brief QoS Control Entry ID
 **/
typedef u32 vtss_qce_id_t;

#define VTSS_QCE_ID_LAST 0 /**< Special value used to add last in list */

/** \brief QoS Control Entry type */
typedef enum
{
    VTSS_QCE_TYPE_ETYPE,     /**< Ethernet Type */
#if defined(VTSS_FEATURE_QCL_V2)
    VTSS_QCE_TYPE_ANY,     /**< Any frame type */
    VTSS_QCE_TYPE_LLC,     /**< LLC */
    VTSS_QCE_TYPE_SNAP,    /**< SNAP */
    VTSS_QCE_TYPE_IPV4,    /**< IPv4 */
    VTSS_QCE_TYPE_IPV6     /**< IPv6 */
#endif /* VTSS_FEATURE_QCL_V2 */
} vtss_qce_type_t;

#if defined(VTSS_FEATURE_QCL_V2)
/** \brief QCE MAC information */
typedef struct 
{
    vtss_vcap_bit_t dmac_mc; /**< Multicast DMAC */
    vtss_vcap_bit_t dmac_bc; /**< Broadcast DMAC */
#if defined(VTSS_ARCH_SERVAL)
    vtss_vcap_u48_t dmac;    /**< DMAC - Serval: key_type = mac_ip_addr */
#endif /* VTSS_ARCH_SERVAL */
    vtss_vcap_u48_t smac;    /**< SMAC - Only the 24 most significant bits (OUI) are supported on Jaguar1, rest are wildcards */
} vtss_qce_mac_t;

/** \brief QCE tag information */
typedef struct 
{
    vtss_vcap_vr_t  vid;    /**< VLAN ID (12 bit) */
    vtss_vcap_u8_t  pcp;    /**< PCP (3 bit) */
    vtss_vcap_bit_t dei;    /**< DEI */
    vtss_vcap_bit_t tagged; /**< Tagged/untagged frame */
#if defined(VTSS_ARCH_SERVAL)
    vtss_vcap_bit_t s_tag;  /**< S-tagged/C-tagged frame */
#endif /* VTSS_ARCH_SERVAL */
} vtss_qce_tag_t;

/** \brief Frame data for VTSS_QCE_TYPE_ETYPE */
typedef struct 
{
    vtss_vcap_u16_t etype; /**< Ethernet Type value */
    vtss_vcap_u32_t data;  /**< MAC data */ 
} vtss_qce_frame_etype_t;

/** \brief Frame data for VTSS_QCE_TYPE_LLC */
typedef struct 
{
    vtss_vcap_u48_t data; /**< Data */
} vtss_qce_frame_llc_t;

/** \brief Frame data for VTSS_QCE_TYPE_SNAP */
typedef struct 
{
    vtss_vcap_u48_t data; /**< Data */
} vtss_qce_frame_snap_t;

/** \brief Frame data for VTSS_QCE_TYPE_IPV4 */
typedef struct 
{
    vtss_vcap_bit_t fragment; /**< Fragment */
    vtss_vcap_vr_t  dscp;     /**< DSCP field (6 bit) */
    vtss_vcap_u8_t  proto;    /**< Protocol */
    vtss_vcap_ip_t  sip;      /**< Source IP address - Serval: key_type = normal, ip_addr and mac_ip_addr */
#if defined(VTSS_ARCH_SERVAL)
    vtss_vcap_ip_t  dip;      /**< Destination IP address - Serval: key_type = ip_addr and mac_ip_addr */
#endif /* VTSS_ARCH_SERVAL */
    vtss_vcap_vr_t  sport;    /**< UDP/TCP: Source port - Serval: key_type = normal, ip_addr and mac_ip_addr */
    vtss_vcap_vr_t  dport;    /**< UDP/TCP: Destination port - Serval: key_type = double_tag, ip_addr and mac_ip_addr */
} vtss_qce_frame_ipv4_t;

/** \brief Frame data for VTSS_QCE_TYPE_IPV6 */
typedef struct 
{
    vtss_vcap_vr_t   dscp;    /**< DSCP field (6 bit) */
    vtss_vcap_u8_t   proto;   /**< Protocol */
    vtss_vcap_u128_t sip;     /**< Source IP address (32 LSB on L26 and J1, 64 LSB on Serval when key_type = mac_ip_addr) */
#if defined(VTSS_ARCH_SERVAL)
    vtss_vcap_u128_t dip;     /**< Destination IP address - 64 LSB on Serval when key_type = mac_ip_addr */
#endif /* VTSS_ARCH_SERVAL */
    vtss_vcap_vr_t   sport;   /**< UDP/TCP: Source port - Serval: key_type = normal, ip_addr and mac_ip_addr */
    vtss_vcap_vr_t   dport;   /**< UDP/TCP: Destination port - Serval: key_type = double_tag, ip_addr and mac_ip_addr */
} vtss_qce_frame_ipv6_t;

/**
 * \brief QCE key 
 **/
typedef struct
{
    BOOL            port_list[VTSS_PORT_ARRAY_SIZE]; /**< Port list */ 
    vtss_qce_mac_t  mac;                             /**< MAC */
    vtss_qce_tag_t  tag;                             /**< Tag */
#if defined(VTSS_ARCH_SERVAL)
    vtss_qce_tag_t  inner_tag;                       /**< Inner tag */
#endif /* VTSS_ARCH_SERVAL */
    vtss_qce_type_t type;                            /**< Frame type */

    union
    {
        /* VTSS_QCE_TYPE_ANY: No specific fields */
        vtss_qce_frame_etype_t etype; /**< VTSS_QCE_TYPE_ETYPE */
        vtss_qce_frame_llc_t   llc;   /**< VTSS_QCE_TYPE_LLC */
        vtss_qce_frame_snap_t  snap;  /**< VTSS_QCE_TYPE_SNAP */
        vtss_qce_frame_ipv4_t  ipv4;  /**< VTSS_QCE_TYPE_IPV4 */
        vtss_qce_frame_ipv6_t  ipv6;  /**< VTSS_QCE_TYPE_IPV6 */
    } frame; /**< Frame type specific data */
} vtss_qce_key_t;

/**
 * \brief QCE action 
 **/
typedef struct
{
    BOOL                 prio_enable;      /**< Enable priority classification */
    vtss_prio_t          prio;             /**< Priority value */
    BOOL                 dp_enable;        /**< Enable DP classification */
    vtss_dp_level_t      dp;               /**< DP value */
    BOOL                 dscp_enable;      /**< Enable DSCP classification */
    vtss_dscp_t          dscp;             /**< DSCP value */
#if (defined VTSS_FEATURE_QCL_PCP_DEI_ACTION)
    BOOL                 pcp_dei_enable;   /**< Enable PCP and DEI classification */
    vtss_tagprio_t       pcp;              /**< PCP value */
    vtss_dei_t           dei;              /**< DEI value */
#endif /* (defined VTSS_FEATURE_QCL_PCP_DEI_ACTION) */
#if (defined VTSS_FEATURE_QCL_POLICY_ACTION)
    BOOL                 policy_no_enable; /**< Enable ACL policy classification */
    vtss_acl_policy_no_t policy_no;        /**< ACL policy number */
#endif /* VTSS_FEATURE_QCL_POLICY_ACTION */
} vtss_qce_action_t;
#endif /* VTSS_FEATURE_QCL_V2 */

/**
 * \brief QoS Control Entry
 **/
typedef struct
{
    vtss_qce_id_t     id;         /**< Entry ID */

#if defined(VTSS_FEATURE_QCL_V2)
    vtss_qce_key_t    key;        /**< QCE key */      
    vtss_qce_action_t action;     /**< QCE action */      
#endif /* VTSS_FEATURE_QCL_V2 */

} vtss_qce_t;

/**
 * \brief Initialize QCE to default values.
 *
 * \param inst [IN]  Target instance reference.
 * \param type [IN]  QCE type.
 * \param qce [OUT]  QCE structure.
 *
 * \return Return code.
 **/
vtss_rc vtss_qce_init(const vtss_inst_t      inst,
                      const vtss_qce_type_t  type,
                      vtss_qce_t             *const qce);

/**
 * \brief Add QCE to QCL.
 *
 * \param inst [IN]    Target instance reference.
 * \param qcl_id [IN]  QCL ID. 
 * \param qce_id [IN]  QCE ID. The QCE will be added before the entry with this ID. 
 *                     VTSS_QCE_ID_LAST is reserved for inserting last.
 * \param qce [IN]     QCE structure.
 *
 * \return Return code.
 **/
vtss_rc vtss_qce_add(const vtss_inst_t    inst,
                     const vtss_qcl_id_t  qcl_id,
                     const vtss_qce_id_t  qce_id,
                     const vtss_qce_t     *const qce);

/**
 * \brief Delete QCE from QCL.
 *
 * \param inst [IN]    Target instance reference.
 * \param qcl_id [IN]  QCL ID.
 * \param qce_id [IN]  QCE ID.
 *
 * \return Return code.
 **/
vtss_rc vtss_qce_del(const vtss_inst_t    inst,
                     const vtss_qcl_id_t  qcl_id,
                     const vtss_qce_id_t  qce_id);

#endif /* VTSS_FEATURE_QCL */

#endif /* VTSS_FEATURE_QOS */

#endif /* _VTSS_QOS_API_H_ */
