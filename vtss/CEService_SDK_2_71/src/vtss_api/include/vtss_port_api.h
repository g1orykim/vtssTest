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
 * \file vtss_port_api.h
 * \brief Port API
 */

#ifndef _VTSS_PORT_API_H_
#define _VTSS_PORT_API_H_

#include <vtss_options.h>
#include <vtss_types.h>

/** \brief MII management controller */
typedef enum
{
#if defined(VTSS_ARCH_B2) || defined(VTSS_ARCH_LUTON28) || \
    defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
    VTSS_MIIM_CONTROLLER_0    = 0,  /**< MIIM controller 0 */
    VTSS_MIIM_CONTROLLER_1    = 1,  /**< MIIM controller 1 */
#endif /* VTSS_ARCH_B2/LUTON28/LUTON26/JAGUAR_1 */
#if defined(VTSS_ARCH_DAYTONA)
    VTSS_MIIM_CONTROLLER_0    = 0,  /**< MIIM controller 0 */
#endif /* VTSS_ARCH_DAYTONA */
    VTSS_MIIM_CONTROLLERS,          /**< Number of MIIM controllers */
    VTSS_MIIM_CONTROLLER_NONE = -1  /**< Unassigned MIIM controller */
} vtss_miim_controller_t;

#if defined(VTSS_FEATURE_PORT_CONTROL)

#define CHIP_PORT_UNUSED -1 /**< Signifies an unused chip port */

/** \brief Port map structure */
typedef struct
{
    i32                    chip_port;        /**< Set to -1 if not used */
    vtss_chip_no_t         chip_no;          /**< Chip number, multi-chip targets */

#if defined(VTSS_ARCH_B2)
    vtss_lport_no_t        lport_no;         /**< Logical port, host mode 0, 1, 3, 4, 5 and 6.  VTSS_LPORT_DEFAULT means default mapping. */
#endif /* VTSS_ARCH_B2 */

#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC)
    vtss_lport_no_t        lport_no;         /**< Logical (host) port is mapped to API port (and Chip port) for default forwarding. */
                                             /**< VTSS_LPORT_DEFAULT means default mapping: lport \<x\> maps to API port \<x\> - VTSS_PORT_NO_START) */
#endif /* VTSS_ARCH_JAGUAR_1_CE_MAC */
    vtss_miim_controller_t miim_controller;  /**< MII management controller */
    u8                     miim_addr;        /**< PHY address, ignored for VTSS_MIIM_CONTROLLER_NONE */
    vtss_chip_no_t         miim_chip_no;     /**< MII management chip number, multi-chip targets */
} vtss_port_map_t;

/**
 * \brief Set port map.
 *
 * \param inst [IN]      Target instance reference.
 * \param port_map [IN]  Port map array.
 *
 * \return Return code.
 **/
vtss_rc vtss_port_map_set(const vtss_inst_t      inst,
                          const vtss_port_map_t  port_map[VTSS_PORT_ARRAY_SIZE]);


/**
 * \brief Get port map.
 *
 * \param inst [IN]       Target instance reference.
 * \param port_map [OUT]  Port map.
 *
 * \return Return code.
 **/
vtss_rc vtss_port_map_get(const vtss_inst_t  inst,
                          vtss_port_map_t    port_map[VTSS_PORT_ARRAY_SIZE]);

#if defined(VTSS_FEATURE_CLAUSE_37)
/**
 * Advertisement Word (Refer to IEEE 802.3 Clause 37):
 *  MSB                                                                         LSB
 *  D15  D14  D13  D12  D11  D10   D9   D8   D7   D6   D5   D4   D3   D2   D1   D0 
 * +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
 * | NP | Ack| RF2| RF1|rsvd|rsvd|rsvd| PS2| PS1| HD | FD |rsvd|rsvd|rsvd|rsvd|rsvd|
 * +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
 **/

/** \brief Auto-negotiation remote fault type */
typedef enum                      /* RF2      RF1 */
{
    VTSS_PORT_CLAUSE_37_RF_LINK_OK       = ((0<<1) | (0<<0)),   /**< Link OK */
    VTSS_PORT_CLAUSE_37_RF_OFFLINE       = ((1<<1) | (0<<0)),   /**< Off line */
    VTSS_PORT_CLAUSE_37_RF_LINK_FAILURE  = ((0<<1) | (1<<0)),   /**< Link failure */
    VTSS_PORT_CLAUSE_37_RF_AUTONEG_ERROR = ((1<<1) | (1<<0))    /**< Autoneg error */
} vtss_port_clause_37_remote_fault_t;

/** \brief Advertisement control data for Clause 37 aneg */
typedef struct
{
    BOOL                               fdx;               /**< (FD) */
    BOOL                               hdx;               /**< (HD) */
    BOOL                               symmetric_pause;   /**< (PS1) */
    BOOL                               asymmetric_pause;  /**< (PS2) */
    vtss_port_clause_37_remote_fault_t remote_fault;      /**< (RF1) + (RF2) */
    BOOL                               acknowledge;       /**< (Ack) */
    BOOL                               next_page;         /**< (NP) */
} vtss_port_clause_37_adv_t;

/** \brief Advertisement control data for SGMII aneg */
typedef struct
{
    BOOL                               link;              /**< LP link status               */
    BOOL                               fdx;               /**< FD                           */
    BOOL                               hdx;               /**< HD                           */
    BOOL                               speed_10M;         /**< speed 10 advertised          */
    BOOL                               speed_100M;        /**< speed 100 advertised         */
    BOOL                               speed_1G;          /**< speed 1G advertised          */
    BOOL                               aneg_complete;     /**< Aneg process completed       */
} vtss_port_sgmii_aneg_t;


/** \brief Auto-negotiation control parameter struct */
typedef struct
{
    BOOL                      enable;           /**< Enable of Autoneg */
    vtss_port_clause_37_adv_t advertisement;    /**< Clause 37 Advertisement control data */
} vtss_port_clause_37_control_t;

/**
 * \brief Get clause 37 auto-negotiation Control word.
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 * \param control [OUT]  Control structure.
 *
 * \return Return code.
 **/
vtss_rc vtss_port_clause_37_control_get(const vtss_inst_t              inst,
                                        const vtss_port_no_t           port_no,
                                        vtss_port_clause_37_control_t  *const control);



/**
 * \brief Set clause 37 auto-negotiation Control word.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param control [IN]  Control structure.
 *
 * \return Return code.
 **/
vtss_rc vtss_port_clause_37_control_set(const vtss_inst_t                    inst,
                                        const vtss_port_no_t                 port_no,
                                        const vtss_port_clause_37_control_t  *const control);
#endif /* VTSS_FEATURE_CLAUSE_37 */


/** \brief Flow control setup */
typedef struct
{
    BOOL       obey;          /**< TRUE if PAUSE frames should be obeyed */
#if defined(VTSS_ARCH_DAYTONA)  
    BOOL       rx_drop;       /**< TRUE if PAUSE frames needs to be dropped/obey*/
#endif /* VTSS_ARCH_DAYTONA */    
    BOOL       generate;      /**< TRUE if PAUSE frames should generated */
    vtss_mac_t smac;          /**< Port MAC address used as SMAC in PAUSE frames */
#if defined(VTSS_ARCH_B2)  
    BOOL       pfci_enable;   /**< Chip version -01: Generate PAUSE frames based on PFCI */
#endif /* VTSS_ARCH_B2 */    
} vtss_port_flow_control_conf_t;

#define VTSS_FRAME_GAP_DEFAULT 0 /**< Default frame gap used */

/** \brief Inter frame gap structure */
typedef struct
{
    u32 hdx_gap_1;      /**< Half duplex: First part of Rx to Tx gap */
    u32 hdx_gap_2;      /**< Half duplex: Second part of Rx to Tx gap */
    u32 fdx_gap;        /**< Full duplex: Tx to Tx gap */
} vtss_port_frame_gaps_t;

/* A selection of max frame lengths */
#define VTSS_MAX_FRAME_LENGTH_STANDARD 1518  /**< IEEE 802.3 standard */
#define VTSS_MAX_FRAME_LENGTH_TAGGED   (VTSS_MAX_FRAMELENGTH_STANDARD+4) /**< Tagged frames */

#if defined(VTSS_ARCH_LUTON28) || defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_SERVAL)
#define VTSS_MAX_FRAME_LENGTH_MAX      9600  /**< Maximum frame length supported */
#endif /* VTSS_ARCH_LUTON28/LUTON26/SERVAL */

#if defined(VTSS_ARCH_JAGUAR_1)
#undef VTSS_MAX_FRAME_LENGTH_MAX
#define VTSS_MAX_FRAME_LENGTH_MAX      10056 /**< Maximum frame length supported */
#endif /* VTSS_ARCH_JAGUAR_1 */

#if defined(VTSS_ARCH_JAGUAR_2)
// JR2-TBD: Insert correct value
#undef VTSS_MAX_FRAME_LENGTH_MAX
#define VTSS_MAX_FRAME_LENGTH_MAX      10056 /**< Maximum frame length supported */
#endif /* VTSS_ARCH_JAGUAR_2 */

#if defined(VTSS_ARCH_LUTON26)
#undef VTSS_MAX_FRAME_LENGTH_MAX
#define VTSS_MAX_FRAME_LENGTH_MAX      9600 /**< Maximum frame length supported */
#endif /* VTSS_ARCH_LUTON26 */

#if defined(VTSS_ARCH_B2)
#undef VTSS_MAX_FRAME_LENGTH_MAX
#define VTSS_MAX_FRAME_LENGTH_MAX      10240 /**< Maximum frame length supported */
#endif /* VTSS_ARCH_B2 */

#if defined(VTSS_ARCH_DAYTONA)
#undef VTSS_MAX_FRAME_LENGTH_MAX
#define VTSS_MAX_FRAME_LENGTH_MAX      10240 /**< Maximum frame length supported */
#endif /* VTSS_ARCH_DAYTONA */

/** \brief VLAN awareness for frame length check */
typedef enum
{
    VTSS_PORT_MAX_TAGS_NONE,  /**< No extra tags allowed */
    VTSS_PORT_MAX_TAGS_ONE,   /**< Single tag allowed */
    VTSS_PORT_MAX_TAGS_TWO    /**< Single and double tag allowed */
} vtss_port_max_tags_t;

#if defined(VTSS_ARCH_B2)
/** \brief Cut-through threshold */
typedef struct
{
    u32 rx_ct;  /**< Cut-through threshold in bytes. Line port Rx, host port Tx. '0' to disable. */
    u32 tx_ct;  /**< Cut-through threshold in bytes. Line port Tx, host port Rx. '0' to disable. */    
} vtss_port_ct_t;
#endif

/** \brief Port loop back configuration */
typedef enum
{
    VTSS_PORT_LOOP_DISABLE,   /**< No port loop */
    VTSS_PORT_LOOP_PCS_HOST,  /**< PCS host port loop */
} vtss_port_loop_t;

/** \brief Port configuration structure */
typedef struct
{
    vtss_port_interface_t         if_type;           /**< Interface type */
    BOOL                          sd_enable;         /**< Signal detect enable */
    BOOL                          sd_active_high;    /**< External signal detect polarity */
    BOOL                          sd_internal;       /**< Internal signal detect selection */
    vtss_port_frame_gaps_t        frame_gaps;        /**< Inter frame gaps */
    BOOL                          power_down;        /**< Disable and power down the port */
    vtss_port_speed_t             speed;             /**< Port speed */
    BOOL                          fdx;               /**< Full duplex mode */
    vtss_port_flow_control_conf_t flow_control;      /**< Flow control setup */
    u32                           max_frame_length;  /**< Maximum frame length */
    vtss_port_max_tags_t          max_tags;          /**< VLAN awareness for length check */
    BOOL                          exc_col_cont;      /**< Excessive collision continuation */
    BOOL                          xaui_rx_lane_flip; /**< Xaui Rx lane flip */
    BOOL                          xaui_tx_lane_flip; /**< Xaui Tx lane flip */
    vtss_port_loop_t              loop;              /**< Enable/disable of port loop back */
#if defined(VTSS_ARCH_B2)                     
    vtss_port_ct_t                ct;                /**< Cut-through settings */
#endif                                        
} vtss_port_conf_t;

/**
 * \brief Set port configuration.
 *  Note: If if_type in the vtss_port_conf_t/vtss_port_interface_t definition is set to VTSS_PORT_INTERFACE_QSGMII, the ports are mapped together in groups of four. If one of the four ports is used, all four ports in the group must always be configured, but the four ports doesn't need to configured with the same configuration. 
 * This is needed in order to achieve correct comma alignment at the QSGMII interface. Which ports that are mapped together can be found in the chip data-sheet.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param conf [IN]     Port setup structure.
 *
 * \return Return code.
 **/
vtss_rc vtss_port_conf_set(const vtss_inst_t       inst,
                           const vtss_port_no_t    port_no,
                           const vtss_port_conf_t  *const conf);



/**
 * \brief Get port setup.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param conf [OUT]    Port configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_port_conf_get(const vtss_inst_t     inst,
                           const vtss_port_no_t  port_no,
                           vtss_port_conf_t      *const conf);


/**
 * \brief Get port status.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param status [OUT]  Status structure.
 *
 * \return Return code.
 **/
vtss_rc vtss_port_status_get(const vtss_inst_t     inst,
                             const vtss_port_no_t  port_no,
                             vtss_port_status_t    *const status);



/**
 * \brief Update counters for port.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 *
 * \return Return code.
 **/
vtss_rc vtss_port_counters_update(const vtss_inst_t     inst,
                                  const vtss_port_no_t  port_no);



/**
 * \brief Clear counters for port.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port/aggregation number.
 *
 * \return Return code.
 **/
vtss_rc vtss_port_counters_clear(const vtss_inst_t     inst,
                                 const vtss_port_no_t  port_no);



/** \brief Counter type */
typedef u64 vtss_port_counter_t;

/** \brief RMON counter structure (RFC 2819) */
typedef struct {
    vtss_port_counter_t rx_etherStatsDropEvents;           /**< Rx drop events */
    vtss_port_counter_t rx_etherStatsOctets;               /**< Rx octets */
    vtss_port_counter_t rx_etherStatsPkts;                 /**< Rx packets */
    vtss_port_counter_t rx_etherStatsBroadcastPkts;        /**< Rx broadcasts */ 
    vtss_port_counter_t rx_etherStatsMulticastPkts;        /**< Rx multicasts */
    vtss_port_counter_t rx_etherStatsCRCAlignErrors;       /**< Rx CRC/alignment errors */
    vtss_port_counter_t rx_etherStatsUndersizePkts;        /**< Rx undersize packets */
    vtss_port_counter_t rx_etherStatsOversizePkts;         /**< Rx oversize packets */
    vtss_port_counter_t rx_etherStatsFragments;            /**< Rx fragments */
    vtss_port_counter_t rx_etherStatsJabbers;              /**< Rx jabbers */
    vtss_port_counter_t rx_etherStatsPkts64Octets;         /**< Rx 64 byte packets */
    vtss_port_counter_t rx_etherStatsPkts65to127Octets;    /**< Rx 65-127 byte packets */
    vtss_port_counter_t rx_etherStatsPkts128to255Octets;   /**< Rx 128-255 byte packets */
    vtss_port_counter_t rx_etherStatsPkts256to511Octets;   /**< Rx 256-511 byte packets */
    vtss_port_counter_t rx_etherStatsPkts512to1023Octets;  /**< Rx 512-1023 byte packet */
    vtss_port_counter_t rx_etherStatsPkts1024to1518Octets; /**< Rx 1024-1518 byte packets */
    vtss_port_counter_t rx_etherStatsPkts1519toMaxOctets;  /**< Rx 1519- byte packets */

    vtss_port_counter_t tx_etherStatsDropEvents;           /**< Tx drop events */
    vtss_port_counter_t tx_etherStatsOctets;               /**< Tx octets */
    vtss_port_counter_t tx_etherStatsPkts;                 /**< Tx packets */
    vtss_port_counter_t tx_etherStatsBroadcastPkts;        /**< Tx broadcasts */
    vtss_port_counter_t tx_etherStatsMulticastPkts;        /**< Tx multicasts */
    vtss_port_counter_t tx_etherStatsCollisions;           /**< Tx collisions */
    vtss_port_counter_t tx_etherStatsPkts64Octets;         /**< Tx 64 byte packets */
    vtss_port_counter_t tx_etherStatsPkts65to127Octets;    /**< Tx 65-127 byte packets */
    vtss_port_counter_t tx_etherStatsPkts128to255Octets;   /**< Tx 128-255 byte packets */
    vtss_port_counter_t tx_etherStatsPkts256to511Octets;   /**< Tx 256-511 byte packets */
    vtss_port_counter_t tx_etherStatsPkts512to1023Octets;  /**< Tx 512-1023 byte packet */
    vtss_port_counter_t tx_etherStatsPkts1024to1518Octets; /**< Tx 1024-1518 byte packets */
    vtss_port_counter_t tx_etherStatsPkts1519toMaxOctets;  /**< Tx 1519- byte packets */
} vtss_port_rmon_counters_t;

/** \brief Interfaces Group counter structure (RFC 2863) */
typedef struct {
    vtss_port_counter_t ifInOctets;          /**< Rx octets */
    vtss_port_counter_t ifInUcastPkts;       /**< Rx unicasts */
    vtss_port_counter_t ifInMulticastPkts;   /**< Rx multicasts */
    vtss_port_counter_t ifInBroadcastPkts;   /**< Rx broadcasts */
    vtss_port_counter_t ifInNUcastPkts;      /**< Rx non-unicasts */
    vtss_port_counter_t ifInDiscards;        /**< Rx discards */
    vtss_port_counter_t ifInErrors;          /**< Rx errors */
    
    vtss_port_counter_t ifOutOctets;         /**< Tx octets */
    vtss_port_counter_t ifOutUcastPkts;      /**< Tx unicasts */
    vtss_port_counter_t ifOutMulticastPkts;  /**< Tx multicasts */
    vtss_port_counter_t ifOutBroadcastPkts;  /**< Tx broadcasts */
    vtss_port_counter_t ifOutNUcastPkts;     /**< Tx non-unicasts */
    vtss_port_counter_t ifOutDiscards;       /**< Tx discards */
    vtss_port_counter_t ifOutErrors;         /**< Tx errors */
} vtss_port_if_group_counters_t;

/** \brief Ethernet-like Interface counter structure (RFC 3635) */
typedef struct {
#if defined(VTSS_FEATURE_PORT_CNT_ETHER_LIKE)
    vtss_port_counter_t dot3StatsAlignmentErrors;          /**< Rx alignment errors */
    vtss_port_counter_t dot3StatsFCSErrors;                /**< Rx FCS errors */
    vtss_port_counter_t dot3StatsFrameTooLongs;            /**< Rx too long */
    vtss_port_counter_t dot3StatsSymbolErrors;             /**< Rx symbol errors */
    vtss_port_counter_t dot3ControlInUnknownOpcodes;       /**< Rx unknown opcodes */
#endif /* VTSS_FEATURE_PORT_CNT_ETHER_LIKE */
    vtss_port_counter_t dot3InPauseFrames;                 /**< Rx pause */

#if defined(VTSS_FEATURE_PORT_CNT_ETHER_LIKE)
    vtss_port_counter_t dot3StatsSingleCollisionFrames;    /**< Tx single collisions */
    vtss_port_counter_t dot3StatsMultipleCollisionFrames;  /**< Tx multiple collisions */
    vtss_port_counter_t dot3StatsDeferredTransmissions;    /**< Tx deferred */
    vtss_port_counter_t dot3StatsLateCollisions;           /**< Tx late collisions */
    vtss_port_counter_t dot3StatsExcessiveCollisions;      /**< Tx excessive collisions */
    vtss_port_counter_t dot3StatsCarrierSenseErrors;       /**< Tx carrier sense errors */
#endif /* VTSS_FEATURE_PORT_CNT_ETHER_LIKE */
    vtss_port_counter_t dot3OutPauseFrames;                /**< Tx pause */
} vtss_port_ethernet_like_counters_t;

/** \brief Port bridge counter structure (RFC 4188) */
typedef struct
{
    vtss_port_counter_t dot1dTpPortInDiscards; /**< Rx bridge discards */
} vtss_port_bridge_counters_t;

/** \brief Port proprietary counter structure */
typedef struct
{
    vtss_port_counter_t rx_prio[VTSS_PRIOS];        /**< Rx frames */
#if defined(VTSS_ARCH_B2)                        
    vtss_port_counter_t rx_prio_drops[VTSS_PRIOS];  /**< Rx frame queue drops */
    vtss_port_counter_t rx_filter_drops;            /**< Rx frame filter drops */
    vtss_port_counter_t rx_policer_drops;           /**< Rx frame policer drops */
#endif /* VTSS_ARCH_B2 */                        
#if defined(VTSS_ARCH_LUTON28) || defined(VTSS_ARCH_LUTON26) || \
    defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
    vtss_port_counter_t tx_prio[VTSS_PRIOS];        /**< Tx frames */
#endif /* VTSS_ARCH_LUTON28/LUTON26/JAGUAR_1/SERVAL */
} vtss_port_proprietary_counters_t;

#if defined(VTSS_ARCH_CARACAL)
/** \brief EVC counters */
typedef struct
{
    vtss_port_counter_t rx_green[VTSS_PRIOS];          /**< Rx green frames */
    vtss_port_counter_t rx_yellow[VTSS_PRIOS];         /**< Rx yellow frames */
    vtss_port_counter_t rx_red[VTSS_PRIOS];            /**< Rx red frames */
    vtss_port_counter_t rx_green_discard[VTSS_PRIOS];  /**< Rx green discarded frames */
    vtss_port_counter_t rx_yellow_discard[VTSS_PRIOS]; /**< Rx yellow discarded frames */
    vtss_port_counter_t tx_green[VTSS_PRIOS];          /**< Tx green frames */
    vtss_port_counter_t tx_yellow[VTSS_PRIOS];         /**< Tx yellow frames */
} vtss_port_evc_counters_t;
#endif /* VTSS_ARCH_CARACAL */

/** \brief Port counter structure */
typedef struct
{
    vtss_port_rmon_counters_t          rmon;           /**< RMON counters  */
    vtss_port_if_group_counters_t      if_group;       /**< Interfaces Group counters  */
    vtss_port_ethernet_like_counters_t ethernet_like;  /**< Ethernet-like Interface counters */ 

#if defined(VTSS_FEATURE_PORT_CNT_BRIDGE)
    vtss_port_bridge_counters_t        bridge;         /**< Bridge counters */
#endif /* VTSS_FEATURE_PORT_CNT_BRIDGE */
    
    vtss_port_proprietary_counters_t   prop;           /**< Proprietary counters */

#if defined(VTSS_ARCH_CARACAL)
    vtss_port_evc_counters_t           evc;            /**< EVC counters */
#endif /* VTSS_ARCH_CARACAL */
} vtss_port_counters_t;

/**
 * \brief Get counters for port.
 *
 * \param inst [IN]       Target instance reference.
 * \param port_no [IN]    Port/aggregation number.
 * \param counters [OUT]  Counter structure.
 *
 * \return Return code.
 **/
vtss_rc vtss_port_counters_get(const vtss_inst_t     inst,
                               const vtss_port_no_t  port_no,
                               vtss_port_counters_t  *const counters);


/** \brief Basic counters structure */
typedef struct
{
    u32 rx_frames; /**< Rx frames */
    u32 tx_frames; /**< Tx frames */
} vtss_basic_counters_t;

/**
 * \brief Get basic counters for port.
 *
 * \param inst [IN]       Target instance reference.
 * \param port_no [IN]    Port/aggregation number.
 * \param counters [OUT]  Counter structure.
 *
 * \return Return code.
 **/
vtss_rc vtss_port_basic_counters_get(const vtss_inst_t     inst,
                                     const vtss_port_no_t  port_no,
                                     vtss_basic_counters_t *const counters);


/** \brief Port forwarding state */
typedef enum
{
    VTSS_PORT_FORWARD_ENABLED,   /**< Forward in both directions */
    VTSS_PORT_FORWARD_DISABLED,  /**< Forwarding and learning disabled */
    VTSS_PORT_FORWARD_INGRESS,   /**< Forward frames from port only */
    VTSS_PORT_FORWARD_EGRESS     /**< Forward frames to port only (learning disabled) */
} vtss_port_forward_t;

/**
 * \brief Get port forwarding state.
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 * \param forward [OUT]  Forwarding state.
 *
 * \return Return code.
 **/
vtss_rc vtss_port_forward_state_get(const vtss_inst_t     inst,
                                    const vtss_port_no_t  port_no, 
                                    vtss_port_forward_t   *const forward);

/**
 * \brief Set port forwarding state.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param forward [IN]  Forwarding state.
 *
 * \return Return code.
 **/
vtss_rc vtss_port_forward_state_set(const vtss_inst_t          inst,
                                    const vtss_port_no_t       port_no, 
                                    const vtss_port_forward_t  forward);

#if defined(VTSS_ARCH_SERVAL)
/** \brief Port Internal Frame Header structure */
typedef struct
{
    BOOL ena_inj_header; /**< At ingress expect long prefix followed by an internal frame header */
    BOOL ena_xtr_header; /**< At egress prepend long prefix followed by the internal frame header */
} vtss_port_ifh_t;

/**
 * \brief Set port Internal Frame Header settings.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param conf [IN]     Port IFH structure.
 *
 * \return Return code.
 **/
vtss_rc vtss_port_ifh_conf_set(const vtss_inst_t       inst,
                               const vtss_port_no_t    port_no,
                               const vtss_port_ifh_t  *const conf);

/**
 * \brief Get port Internal Frame Header settings.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param conf [OUT]    Port IFH configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_port_ifh_conf_get(const vtss_inst_t     inst,
                               const vtss_port_no_t  port_no,
                               vtss_port_ifh_t      *const conf);
#endif /* VTSS_ARCH_SERVAL) */

/**
 * \brief Direct MIIM read (bypassing port map)
 *
 * \param inst            [IN]  Target instance reference.
 * \param chip_no         [IN]  Chip number (if multi-chip instance).
 * \param miim_controller [IN]  MIIM Controller Instance
 * \param miim_addr       [IN]  MIIM Device Address
 * \param addr            [IN]  MIIM Register Address
 * \param value           [OUT] Register value read
 *
 * \return Return code.
 **/
vtss_rc vtss_miim_read(const vtss_inst_t            inst,
                       const vtss_chip_no_t         chip_no,
                       const vtss_miim_controller_t miim_controller,
                       const u8                     miim_addr,
                       const u8                     addr,
                       u16                          *const value);

/**
 * \brief Direct MIIM write (bypassing port map)
 *
 * \param inst            [IN]  Target instance reference.
 * \param chip_no         [IN]  Chip number (if multi-chip instance).
 * \param miim_controller [IN]  MIIM Controller Instance
 * \param miim_addr       [IN]  MIIM Device Address
 * \param addr            [IN]  MIIM Register Address
 * \param value           [IN]  Register value to write
 *
 * \return Return code.
 **/
vtss_rc vtss_miim_write(const vtss_inst_t            inst,
                        const vtss_chip_no_t         chip_no,
                        const vtss_miim_controller_t miim_controller,
                        const u8                     miim_addr,
                        const u8                     addr,
                        const u16                    value);
#endif /* VTSS_FEATURE_PORT_CONTROL */

#if defined(VTSS_FEATURE_10G) || defined(VTSS_CHIP_10G_PHY)
/**
 * \brief Read value from MMD register.
 *
 * \param inst    [IN]  Target instance reference.
 * \param port_no [IN]  Port number connected to MMD.
 * \param mmd     [IN]  MMD number.
 * \param addr    [IN]  PHY register address.
 * \param value   [OUT] PHY register value.
 *
 * \return Return code.
 **/
vtss_rc vtss_port_mmd_read(const vtss_inst_t     inst,
                           const vtss_port_no_t  port_no,
                           const u8              mmd,
                           const u16             addr,
                           u16                   *const value);
/**
 * \brief Read values (a number of 16 bit values) from MMD register.
 *
 * \param inst    [IN]  Target instance reference.
 * \param port_no [IN]  Port number connected to MMD.
 * \param mmd     [IN]  MMD number.
 * \param addr    [IN]  PHY register address.
 * \param buf     [OUT] PHY register values.
 * \param count   [IN]  number of values to read.
 *
 * \return Return code.
 **/
vtss_rc vtss_port_mmd_read_inc(const vtss_inst_t     inst,
                               const vtss_port_no_t  port_no,
                               const u8              mmd,
                               const u16             addr,
                               u16                   *const buf,
                               u8                    count);
                          
/**
 * \brief Write value to MMD register.
 *
 * \param inst    [IN]  Target instance reference.
 * \param port_no [IN]  Port number connected to MMD.
 * \param mmd     [IN]  MMD number.
 * \param addr    [IN]  PHY register address.
 * \param value   [IN]  PHY register value.
 *
 * \return Return code.
 **/
vtss_rc vtss_port_mmd_write(const vtss_inst_t     inst,
                            const vtss_port_no_t  port_no,
                            const u8              mmd,
                            const u16             addr,
                            const u16             value);


/**
 * \brief Read, modify and write value to MMD register.
 *
 * \param inst     [IN]  Target instance reference.
 * \param port_no  [IN]  Port number connected to MMD.
 * \param mmd      [IN]  MMD number.
 * \param addr     [IN]  PHY register address.
 * \param value    [IN]  PHY register value.
 * \param mask     [IN]  PHY register mask, only enabled bits are changed.
 *
 * \return Return code.
 **/
vtss_rc vtss_port_mmd_masked_write(const vtss_inst_t     inst,
                                   const vtss_port_no_t  port_no,
                                   const u8              mmd,
                                   const u16             addr,
                                   const u16             value,
                                   const u16             mask);

/**
 * \brief Direct MMD read (Clause 45, bypassing port map)
 *
 * \param inst            [IN]  Target instance reference.
 * \param chip_no         [IN]  Chip number (if multi-chip instance).
 * \param miim_controller [IN]  MIIM Controller Instance
 * \param miim_addr       [IN]  MIIM Device Address
 * \param mmd             [IN]  MMD number.
 * \param addr            [IN]  MIIM Register Address
 * \param value           [OUT] Register value read
 *
 * \return Return code.
 **/
vtss_rc vtss_mmd_read(const vtss_inst_t            inst,
                      const vtss_chip_no_t         chip_no,
                      const vtss_miim_controller_t miim_controller,
                      const u8                     miim_addr,
                      const u8                     mmd,
                      const u16                    addr,
                      u16                          *const value);

/**
 * \brief Direct MMD write (Clause 45, bypassing port map)
 *
 * \param inst            [IN]  Target instance reference.
 * \param chip_no         [IN]  Chip number (if multi-chip instance).
 * \param miim_controller [IN]  MIIM Controller Instance
 * \param miim_addr       [IN]  MIIM Device Address
 * \param mmd             [IN]  MMD number.
 * \param addr            [IN]  MIIM Register Address
 * \param value           [IN]  Register value to write
 *
 * \return Return code.
 **/
vtss_rc vtss_mmd_write(const vtss_inst_t            inst,
                       const vtss_chip_no_t         chip_no,
                       const vtss_miim_controller_t miim_controller,
                       const u8                     miim_addr,
                       const u8                     mmd,
                       const u16                    addr,
                       const u16                    value);
#endif /* VTSS_FEATURE_10G */


#if defined(VTSS_ARCH_B2)
/**
 * \brief Set port status interface clock.
 *
 * \param inst [IN]   Target instance reference.
 * \param clock [IN]  Port status interface clock.
 *
 * \return Return code.
 **/
vtss_rc vtss_port_status_interface_set(const vtss_inst_t inst,
                                       const u32         clock);



/**
 * \brief Get port status interface clock.
 *
 * \param inst [IN]   Target instance reference.
 * \param clock [OUT] Port Status Interface clock.
 *
 * \return Return code.
 **/
vtss_rc vtss_port_status_interface_get(const vtss_inst_t      inst,
                                       u32                    *clock);

/** \brief SPI-4.2 configuration. All fields are zero by default, unless otherwise noted. */
typedef struct {
    /* Channel flow control */
    struct {
        BOOL enable;             /**< Enable frame interleave flow control (default 1) */ 
        BOOL three_level_enable; /**< Three levels (default two levels) */
    } fc; /**< Flowcontrol setup */

    /* QoS */
    struct {
        vtss_shaper_t shaper; /**< Maximum rate control (default disabled) */
    } qos; /**< QoS shaper setup */

    /* Inbound setup (corresponds to SPI-4 Tx direction) */
    struct {
        enum {
            VTSS_SPI4_FCS_DISABLED, /**< No FCS check/add */
            VTSS_SPI4_FCS_CHECK,    /**< Check FCS of frames (default) */
            VTSS_SPI4_FCS_ADD       /**< Pad frames and add FCS */
        } fcs; /**< FCS operations */
        enum {
            VTSS_SPI4_CLOCK_250_TO_290, /**< 250-290 MHz */
            VTSS_SPI4_CLOCK_290_TO_360, /**< 290-360 MHz */
            VTSS_SPI4_CLOCK_360_TO_450, /**< 360-450 MHz */
            VTSS_SPI4_CLOCK_450_TO_500  /**< 450-500 MHz (default) */
        } clock; /**< Data clock frequency range 250-500 MHz */
        BOOL data_swap;   /**< Swap data bit N and 16-N */
        BOOL data_invert; /**< Invert data and control bits */
        BOOL clock_shift; /**< Shift status clock by 180 degrees */
    } ib; /**< SPI4 inbound control, SPI4 Tx directions */

    /* Outbound setup (corresponds to SPI-4 Rx direction) */
    struct {
        BOOL frame_interleave;  /**< Frame interleaved operation */
        BOOL hih_enable;        /**< Enable/disable prepended Host Interface Header */

        /* Chip version -01 */
        BOOL fcs_strip;         /**< Strip FCS */
        BOOL hih_length_update; /**< If FCS stripped, update HIH length accordingly */ 

        enum {
            VTSS_SPI4_CLOCK_250_0, /**< 250.0 MHz */
            VTSS_SPI4_CLOCK_312_5, /**< 312.5 MHz */
            VTSS_SPI4_CLOCK_375_0, /**< 375.0 MHz */
            VTSS_SPI4_CLOCK_437_5, /**< 437.5 MHz */
            VTSS_SPI4_CLOCK_500_0  /**< 500.0 MHz (default) */
        } clock; /* Data clock frequency */
        enum {
            VTSS_SPI4_CLOCK_PHASE_0,   /**< 0 degree */
            VTSS_SPI4_CLOCK_PHASE_90,  /**< 90 degree */
            VTSS_SPI4_CLOCK_PHASE_180, /**< 180 degree */
            VTSS_SPI4_CLOCK_PHASE_270  /**< 270 degree */
        } clock_phase; /**< Data clock phase */
        BOOL data_swap;   /**< Swap data bit N and 16-N */
        BOOL data_invert; /**< Invert data and control bits */
        BOOL clock_shift; /**< Shift status clock by 180 degrees */
        enum {
            VTSS_SPI4_BURST_64,  /**< 64 bytes */
            VTSS_SPI4_BURST_96,  /**< 96 bytes */
            VTSS_SPI4_BURST_128, /**< 128 bytes (default) */
            VTSS_SPI4_BURST_160, /**< 160 bytes */
            VTSS_SPI4_BURST_192, /**< 192 bytes */
            VTSS_SPI4_BURST_224, /**< 224 bytes */
            VTSS_SPI4_BURST_256  /**< 256 bytes */
        } burst_size; /**< Burst size in bytes */
        u8 max_burst_1;     /**< SPI-4 MaxBurst1 value in 16 byte blocks (default 64) */
        u8 max_burst_2;     /**< SPI-4 MaxBurst2 value in 16 byte blocks (default 32) */
        u8 link_up_limit;   /**< Number of valid DIP-2 before link up (default 2) */
        u8 link_down_limit; /**< Number of invalid DIP-2 before link down (default 2) */
        enum {
            VTSS_SPI4_TRAINING_DISABLED, /**< Training disabled */
            VTSS_SPI4_TRAINING_AUTO,     /**< Automatic training when '11' received (default) */
            VTSS_SPI4_TRAINING_FORCED,   /**< Send continuous training pattern */
            VTSS_SPI4_TRAINING_NORMAL    /**< Only send periodic training sequences */
        } training_mode; /**< SPI4 training control */
        u8 alpha;      /**< SPI-4 ALPHA value (default 1) */
        u32   data_max_t; /**< SPI-4 DATA_MAX_T value in cycles (default 10240) */
    } ob; /**< SPI4 outbound control, SPI4 Rx directions */
} vtss_spi4_conf_t;

/** \brief XAUI configuration. All fields are zero by default, unless otherwise noted. */
typedef struct {
    /* Flow control */
    struct {
        BOOL channel_enable;     /**< Enable Status Channel flow control */
        BOOL three_level_enable; /**< Three levels (default two levels) */
        BOOL obey_pause;         /**< Obey received pause frames */

        /* Chip version -01 */
        struct {
            BOOL         enable;       /**< Enable in-band flow control */
            vtss_mac_t   dmac;         /**< DMAC of flow control frames */
            vtss_etype_t etype;        /**< Ethernet type of flow control frames */
            u32          pause_value;  /**< The pause value controls the pause frame rate */
        } ib; /**< In-band flow control setup */
    } fc; /**< Flow control */

    /* Chip version -01: Status channel selection for host mode 3, 8 and 9 */
    enum {
        VTSS_XAUI_STATUS_BOTH,   /**< Both status channels used */
        VTSS_XAUI_STATUS_XAUI_0, /**< XAUI_0 status channel used */ 
        VTSS_XAUI_STATUS_XAUI_1  /**< XAUI_1 status channel used */ 
    } status_select; /**< Selection of XAUI status channel */

    /* Chip version -01: */
    BOOL dmac_map_enable;  /**< Use DMAC[11:0] instead of VID for aggregation switching */

    /* QoS */
    struct {
        vtss_shaper_t shaper; /**< Maximum rate control (default disabled) */
    } qos; /**< QoS shaper control */

    /* Host Interface Header */
    struct {
        enum {
            VTSS_HIH_POST_SFD,         /**< HIH after SFD  (at offset 4: SOP-X-X-SFD-H0-H1-H2-H3) */
            VTSS_HIH_PRE_SFD,          /**< HIH before SFD (at offset 3: SOP-X-X-H0-H1-H2-H3-SFD) */
            VTSS_HIH_PRE_STANDARD,     /**< Standard preamble (0xFB-0x55-0x55-0x55-0x55-0x55-0x55-0xD5) */
            VTSS_HIH_PRE_SHORT,        /**< Short preamble    (0xFB) */
            VTSS_HIH_IN_CRC,           /**< HIH in CRC        (0xFB-0x55-0x55-0xD5-H0-H1-H2-H3) */
        } format;                      /**< Host Interface Header format */
        BOOL cksum_enable;  /**< Enable HIH checksum check */
    } hih;  /**< Host Interface Header control */

    /* Inbound setup (corresponds to XAUI host Tx direction) */
    struct {
        BOOL clock_shift; /**< Shift status clock by 180 degrees */
    } ib; /**< Inbound setup (corresponds to XAUI host Tx direction) */

    /* Outbound setup (corresponds to XAUI host Rx direction) */
    struct {
        BOOL clock_shift; /**< Shift status clock by 180 degrees */
    } ob; /**< Outbound setup (corresponds to XAUI host Rx direction) */
} vtss_xaui_conf_t;
#endif /* VTSS_ARCH_B2 */

#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC)
/** \brief XAUI configuration. All fields are zero by default, unless otherwise noted. */
typedef struct {
    /* Flow control */
    struct {
        BOOL channel_enable;     /**< Enable XAUI Status Channel flow control */
        BOOL extended_reach;     /**< Enable/Disable XAUI extended reach control */
    } fc; /**< Flow control*/

    /* Host Interface Header */
    struct {
        enum {
            VTSS_HIH_PRE_STANDARD,     /**< Standard preamble (0xFB-0x55-0x55-0x55-0x55-0x55-0x55-0xD5) */
            VTSS_HIH_PRE_SHORT,        /**< Short preamble    (0xFB) */
            VTSS_HIH_PRE_SFD,          /**< HIH before SFD (at offset 3: SOP-X-X-H0-H1-H2-H3-SFD) */
            VTSS_HIH_POST_SFD,         /**< HIH after SFD  (at offset 4: SOP-X-X-SFD-H0-H1-H2-H3) */
            VTSS_HIH_IN_CRC,           /**< HIH in CRC        (0xFB-0x55-0x55-0xD5-H0-H1-H2-H3) */
        } format;                      /**< Host Interface Header format */
        BOOL cksum_enable;  /**< Enable HIH checksum check */
    } hih;  /**< Host Interface Header control */
} vtss_xaui_conf_t;
#endif /* VTSS_ARCH_JAGUAR_1_CE_MAC */

#if defined(VTSS_ARCH_B2) || defined(VTSS_ARCH_JAGUAR_1_CE_MAC) 
/** \brief Host configuration structure, XAUI or SPI4. */
typedef struct {
#if defined(VTSS_ARCH_B2)
    vtss_spi4_conf_t  spi4;         /**< SPI-4.2 host setup */
#endif /* VTSS_ARCH_B2 */
    vtss_xaui_conf_t  xaui;         /**< XAUI host setup */
} vtss_host_conf_t;

/** 
 * \brief Get the API's default configuration for the host port. 
 *
 * \param inst [IN]    Target instance reference.
 * \param port_no [IN] Host port number (API number).
 * \param conf [OUT]   The default Host configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_host_conf_get(const vtss_inst_t inst,
                           const vtss_port_no_t port_no,
                           vtss_host_conf_t *const conf);

/** 
 * \brief Set host port configuration. 'port_no' points to the host.  
 * VTSS_PORT_NO_NONE is accecpted as 'port_no' and the configuration 
 * for XAUI_0 and SPI4 structure is returned.
 *
 * \param inst [IN]    Target instance reference.
 * \param port_no [IN] Host port number (API number).
 * \param conf [IN]    Host configuration to be set.
 *
 * \return Return code.
 **/
vtss_rc vtss_host_conf_set(const vtss_inst_t inst,
                           const vtss_port_no_t port_no,
                           const vtss_host_conf_t *const conf);

#endif /* defined(VTSS_ARCH_B2) || defined(VTSS_ARCH_JAGUAR_1_CE_MAC) */


#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC)
/** \brief Logical (host port) counter structure */
typedef struct
{   /* Rx counters */
    vtss_port_counter_t rx_yellow[VTSS_PRIOS]; /**< Rx yellow frames */
    vtss_port_counter_t rx_green[VTSS_PRIOS];  /**< Rx green frames  */
    vtss_port_counter_t rx_drops[VTSS_PRIOS];  /**< Rx drops         */
    /* Tx counters */
    vtss_port_counter_t tx_yellow[VTSS_PRIOS]; /**< Tx yellow frames */
    vtss_port_counter_t tx_green[VTSS_PRIOS];  /**< Tx green frames  */
} vtss_lport_counters_t;

/**
 * \brief Get counters for logical port.
 *
 * \param inst [IN]       Target instance reference.
 * \param lport_no [IN]   Lport number.
 * \param counters [OUT]  Counter structure.
 *
 * \return Return code.
 **/
vtss_rc vtss_lport_counters_get(const vtss_inst_t     inst,
                                const vtss_lport_no_t  lport_no,
                                vtss_lport_counters_t  *const counters);

/**
 * \brief Clear counters for logical port (host port).
 *
 * \param inst [IN]     Target instance reference.
 * \param lport_no [IN] Lport number.
 *
 * \return Return code.
 **/
vtss_rc vtss_lport_counters_clear(const vtss_inst_t     inst,
                                  const vtss_lport_no_t lport_no);


/** 
* \brief Get the port role 
* \param inst [IN]   Target instance reference.
* \param port_no [IN] Port number (API number).*
* \return Boolean. True:Is host port. False: Not host port.
*/
BOOL vtss_port_is_host(const vtss_inst_t     inst,
                       const vtss_port_no_t  port_no);
#endif /* VTSS_ARCH_JAGUAR_1_CE_SWITCH */


#endif /* _VTSS_PORT_API_H_ */
