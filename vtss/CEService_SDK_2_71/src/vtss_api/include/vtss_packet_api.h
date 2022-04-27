/*

 Vitesse API software.

 Copyright (c) 2002-2014 Vitesse Semiconductor Corporation "Vitesse". All
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
 * \file vtss_packet_api.h
 * \brief Packet API
 * \details This header file describes CPU Rx/Tx packet functions.
 */

#ifndef _VTSS_PACKET_API_H_
#define _VTSS_PACKET_API_H_

#include <vtss_options.h>
#include <vtss_types.h>
#include <vtss_l2_api.h> /* For vtss_sflow_type_t */

#if defined(VTSS_FEATURE_PACKET)

/* - Rx frame registration ----------------------------------------- */

#if defined(VTSS_FEATURE_NPI)

/** \brief NPI configuration */
typedef struct {
    BOOL           enable;      /**< Enable NPI port */
    vtss_port_no_t port_no;     /**< Port to use as NPI - if configurable */
} vtss_npi_conf_t;

/**
 * \brief Get NPI configuration.
 *
 * \param inst [IN]   Target instance reference.
 * \param conf [OUT]  NPI port configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_npi_conf_get(const vtss_inst_t inst,
                          vtss_npi_conf_t   *const conf);

/**
 * \brief Set NPI configuration.
 *
 * \param inst [IN]  Target instance reference.
 * \param conf [IN]  NPI port configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_npi_conf_set(const vtss_inst_t     inst,
                          const vtss_npi_conf_t *const conf);

/** \brief CPU Rx queue NPI configuration */
typedef struct {
    BOOL         enable; /**< Enable redirect of frames to NPI port */
} vtss_packet_rx_queue_npi_conf_t;

#endif /* VTSS_FEATURE_NPI */

/** \brief CPU Rx queue buffer size in bytes */
typedef u32 vtss_packet_rx_queue_size_t;

/** \brief CPU Rx queue configuration */
typedef struct {
    vtss_packet_rx_queue_size_t     size; /**< Queue size */
#if defined(VTSS_FEATURE_NPI)
    vtss_packet_rx_queue_npi_conf_t npi;  /**< NPI configuration */
#endif /* VTSS_FEATURE_NPI */
} vtss_packet_rx_queue_conf_t;

/** \brief CPU Rx packet registration */
typedef struct
{
    BOOL bpdu_cpu_only;        /**< Redirect BPDUs (DMAC 01-80-C2-00-00-0X) */
    BOOL garp_cpu_only[16];    /**< Redirect GARPs (DMAC 01-80-C2-00-00-2X) */
    BOOL ipmc_ctrl_cpu_copy;   /**< Copy IP MC control (DIP 224.0.0.x) to CPU */
    BOOL igmp_cpu_only;        /**< Redirect IGMP frames to the CPU */
    BOOL mld_cpu_only;         /**< Redirect MLD frames to the CPU */
} vtss_packet_rx_reg_t;

/** \brief CPU Rx queue map */
typedef struct
{
    vtss_packet_rx_queue_t bpdu_queue;        /**< BPDUs */
    vtss_packet_rx_queue_t garp_queue;        /**< GARP frames */
    vtss_packet_rx_queue_t learn_queue;       /**< Learn frames */
    vtss_packet_rx_queue_t igmp_queue;        /**< IGMP/MLD frames */
    vtss_packet_rx_queue_t ipmc_ctrl_queue;   /**< IP multicast control frames */
    vtss_packet_rx_queue_t mac_vid_queue;     /**< MAC address table */
    vtss_packet_rx_queue_t stack_queue;       /**< CPU-generated VStaX traffic */
    vtss_packet_rx_queue_t sflow_queue;       /**< SFlow-marked frames */
    vtss_packet_rx_queue_t lrn_all_queue;     /**< Learn-all queue - JR-48 and JR-Stacking only */
#if defined(VTSS_SW_OPTION_L3RT) || defined(VTSS_ARCH_JAGUAR_1)
    vtss_packet_rx_queue_t l3_uc_queue;       /**< L3 routing unicast queue */
    vtss_packet_rx_queue_t l3_other_queue;    /**< L3 routing other frames queue */
#endif /* VTSS_SW_OPTION_L3RT || VTSS_ARCH_JAGUAR_1*/
} vtss_packet_rx_queue_map_t;

/** \brief CPU Rx configuration */
typedef struct {
    vtss_packet_rx_queue_conf_t queue[VTSS_PACKET_RX_QUEUE_CNT];   /**< Queue configuration */
    vtss_packet_rx_reg_t        reg;                               /**< Packet registration */
    vtss_packet_rx_queue_map_t  map;                               /**< Queue mapping */
#if defined(VTSS_FEATURE_PACKET_GROUPING)
    vtss_packet_rx_grp_t        grp_map[VTSS_PACKET_RX_QUEUE_CNT]; /**< Queue to extraction group map */
#endif
} vtss_packet_rx_conf_t;

/**
 * \brief Get Packet Rx configuration.
 *
 * \param inst [IN]  Target instance reference.
 * \param conf [IN]  Packet Rx configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_packet_rx_conf_get(const vtss_inst_t      inst,
                                vtss_packet_rx_conf_t  *const conf);

/**
 * \brief Set CPU Rx queue configuration.
 *
 * \param inst [IN]  Target instance reference.
 * \param conf [IN]  CPU Rx queue configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_packet_rx_conf_set(const vtss_inst_t            inst,
                                const vtss_packet_rx_conf_t  *const conf);

#if defined(VTSS_FEATURE_PACKET_PORT_REG)
/** \brief Packet registration type */
typedef enum {
    VTSS_PACKET_REG_NORMAL,   /**< Global registration configuration is used */
    VTSS_PACKET_REG_FORWARD,  /**< Forward normally */
#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
    VTSS_PACKET_REG_DISCARD,  /**< Discard */
    VTSS_PACKET_REG_CPU_COPY, /**< Copy to CPU */
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */
    VTSS_PACKET_REG_CPU_ONLY  /**< Redirect to CPU */
} vtss_packet_reg_type_t;

/** \brief Packet registration per port */
typedef struct {
    vtss_packet_reg_type_t bpdu_reg[16]; /**< BPDU range: 01-80-C2-00-00-0X */
    vtss_packet_reg_type_t garp_reg[16]; /**< GARP range: 01-80-C2-00-00-2X */
} vtss_packet_rx_port_conf_t;

/**
 * \brief Get packet configuration for port.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param conf [OUT]    Packet port configuration structure.
 *
 * \return Return code.
 **/
vtss_rc vtss_packet_rx_port_conf_get(const vtss_inst_t          inst,
                                     const vtss_port_no_t       port_no,
                                     vtss_packet_rx_port_conf_t *const conf);

/**
 * \brief Set packet configuration for port.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param conf [IN]     Packet port configuration structure.
 *
 * \return Return code.
 **/
vtss_rc vtss_packet_rx_port_conf_set(const vtss_inst_t                inst,
                                     const vtss_port_no_t             port_no,
                                     const vtss_packet_rx_port_conf_t *const conf);
#endif /* VTSS_FEATURE_PACKET_PORT_REG */

/* - Rx frames ----------------------------------------------------- */

#if defined(VTSS_FEATURE_VSTAX)

/**
 * \brief VStaX frame header used for reception
 **/
typedef struct
{
    BOOL               valid;       /**< TRUE if frame was VStaX tagged */
    BOOL               sp;          /**< TRUE if received on super-prio */
    vtss_vstax_upsid_t upsid;       /**< Ingress Unit Port Set ID */
    vtss_port_no_t     port_no;     /**< Ingress Unit port number (or zero) */
    vtss_glag_no_t     glag_no;     /**< Ingress GLAG (or zero) */
#if defined(VTSS_FEATURE_VSTAX_V2)
    vtss_isdx_t        isdx;        /**< 12 bit ingress service index */
#endif /* VTSS_FEATURE_VSTAX_V2 */
} vtss_vstax_rx_header_t;
#endif /* VTSS_FEATURE_VSTAX */

/**
 * \brief Tag Control Information (according to IEEE 802.1Q)
 **/
typedef struct
{
    vtss_vid_t     vid;       /**< VLAN ID */
    BOOL           cfi;       /**< Canonical Format Indicator */
    vtss_tagprio_t tagprio;   /**< Tag priority */
} vtss_tci_t;

/**
 * \brief System frame header describing received frame
 **/
typedef struct
{
    u32                    length;           /**< Frame length excluding CRC */
    vtss_port_no_t         port_no;          /**< Ingress port number */
    u32                    queue_mask;       /**< Bitmask of queues where received on */
    BOOL                   learn;            /**< TRUE if learn frame */
    BOOL                   arrived_tagged;   /**< TRUE is frame was tagged */
    vtss_tci_t             tag;              /**< VLAN tag from frame or port setup */
#if defined(VTSS_FEATURE_VSTAX)
    vtss_vstax_rx_header_t vstax;            /**< VStaX header */
#endif /* VTSS_FEATURE_VSTAX */
} vtss_packet_rx_header_t;

/**
 * \brief Copy a received frame from a CPU queue.
 *
 * \param inst [IN]      Target instance reference.
 * \param queue_no [IN]  CPU queue number.
 * \param header [OUT]   Frame header.
 * \param frame [OUT]    Frame buffer.
 * \param length [IN]    Length of frame buffer.
 *
 * \note Depending on chipset, \em queue_no may be \em don't \em
 * care. Actual queue(s) received on are returned in \em
 * header->queue_mask as a bitmask. (I.e. frames may be received on
 * more than one queue at a time - a '1' indicates the frame was
 * copied for the queue at the corresponding bit position).
 *
 * \return Return code.
 **/
vtss_rc vtss_packet_rx_frame_get(const vtss_inst_t             inst,
                                 const vtss_packet_rx_queue_t  queue_no,
                                 vtss_packet_rx_header_t       *const header,
                                 u8                            *const frame,
                                 const u32                     length);

/**
 * \brief Copy a received frame from a CPU queue - with IFH.  
 *
 * The extracted frame will be preceeded with an IFH and will have the
 * frame FCS at the end. The length of the IFH and frame are returned
 * separately, but are sequentially placed in the same output buffer.
 *
 * Use vtss_packet_rx_hdr_decode() to decode the IFH if necessary.
 *
 * \param inst   [IN]    Target instance reference.
 * \param data   [IN]    Data buffer.
 * \param buflen [IN]    Length of data buffer.
 * \param ifhlen [OUT]   Length of IFH at the start of the buffer.
 * \param frmlen [OUT]   Length of received frame data - incl FCS.
 *
 * \note If the system has more than one CPU queue, a frame from the
 * CPU queue with the lowest numerical number will be returned
 * first. The actual queue can be decoded from the IFH.
 *
 * \return VTSS_RC_OK if a frame was extracted, VTSS_RC_INCOMPLETE otherwise.
 **/
vtss_rc vtss_packet_rx_frame_get_raw(const vtss_inst_t inst,
                                     u8                *const data,
                                     const u32         buflen,
                                     u32               *const ifhlen,
                                     u32               *const frmlen);

/**
 * \brief Discard a received frame from a CPU queue.
 *
 * \param inst [IN]      Target instance reference.
 * \param queue_no [IN]  CPU queue number.
 *
 * \return Return code.
 **/
vtss_rc vtss_packet_rx_frame_discard(const vtss_inst_t             inst,
                                     const vtss_packet_rx_queue_t  queue_no);

/* - Tx frames ----------------------------------------------------- */

/**
 * \brief Send frame unmodified on port.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param frame [IN]    Frame buffer excluding room for CRC.
 * \param length [IN]   Frame length excluding CRC.
 *
 * \return Return code.
 **/
vtss_rc vtss_packet_tx_frame_port(const vtss_inst_t     inst,
                                  const vtss_port_no_t  port_no,
                                  const u8              *const frame,
                                  const u32             length);

/**
 * \brief Send frame on port using egress rules.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param vid [IN]      VLAN ID inserted if the frame is tagged on egress.
 * \param frame [IN]    Frame buffer excluding room for CRC.
 * \param length [IN]   Frame length excluding CRC.
 *
 * \return Return code.
 **/
vtss_rc vtss_packet_tx_frame_port_vlan(const vtss_inst_t     inst,
                                       const vtss_port_no_t  port_no,
                                       const vtss_vid_t      vid,
                                       const u8              *const frame,
                                       const u32             length);

/**
 * \brief Send frame on VLAN using egress rules.
 *
 * \param inst [IN]    Target instance reference.
 * \param vid [IN]     VLAN ID inserted if the frame is tagged on egress.
 * \param frame [IN]   Frame buffer excluding room for CRC.
 * \param length [IN]  Frame length excluding CRC.
 *
 * \return Return code.
 **/
vtss_rc vtss_packet_tx_frame_vlan(const vtss_inst_t  inst,
                                  const vtss_vid_t   vid,
                                  const u8           *const frame,
                                  const u32          length);

/**
 * \brief Information about frame
 **/
typedef struct
{
    vtss_port_no_t port_no;         /**< Ingress port (or VTSS_PORT_NO_NONE) */
#if defined(VTSS_FEATURE_AGGR_GLAG)
    vtss_glag_no_t glag_no;         /**< Ingress GLAG (or VTSS_GLAG_NO_NONE) */
#endif /* VTSS_FEATURE_AGGR_GLAG */
    vtss_vid_t     vid;             /**< Egress VID (or VTSS_VID_NULL) */
    vtss_port_no_t port_tx;         /**< Egress port (or VTSS_PORT_NO_NONE) */
    BOOL           aggr_rx_disable; /**< Disable aggregation Rx filtering */
    BOOL           aggr_tx_disable; /**< Disable aggregation Tx filtering */
} vtss_packet_frame_info_t;

/**
 * \brief Initialize filter information to default values.
 *
 * \param info [OUT]  Frame information.
 **/
void vtss_packet_frame_info_init(vtss_packet_frame_info_t *const info);

/**
 * \brief CPU filter
 **/
typedef enum
{
    VTSS_PACKET_FILTER_DISCARD,   /**< Discard */
    VTSS_PACKET_FILTER_TAGGED,    /**< Tagged transmission */
    VTSS_PACKET_FILTER_UNTAGGED   /**< Untagged transmission */
} vtss_packet_filter_t;

/**
 * \brief Determine ingress/egress filter for frame.
 * The function may be used for ingress filtering, egress filtering or both.
 *
 * \param inst [IN]     Target instance reference.
 * \param info [IN]     Frame information.
 * \param filter [OUT]  Frame filter.
 *
 * \return Return code.
 **/
vtss_rc vtss_packet_frame_filter(const vtss_inst_t               inst,
                                 const vtss_packet_frame_info_t  *const info,
                                 vtss_packet_filter_t            *const filter);

/** \brief Port info structure. */
typedef struct {
    vtss_port_no_t port_no;         /**< Ingress port (or VTSS_PORT_NO_NONE) */
#if defined(VTSS_FEATURE_AGGR_GLAG)
    vtss_glag_no_t glag_no;         /**< Ingress GLAG (or VTSS_GLAG_NO_NONE) */
#endif /* VTSS_FEATURE_AGGR_GLAG */
    vtss_vid_t     vid;             /**< Egress VID (or VTSS_VID_NULL) */
    BOOL           aggr_rx_disable; /**< Disable aggregation Rx filtering */
    BOOL           aggr_tx_disable; /**< Disable aggregation Tx filtering */
} vtss_packet_port_info_t;

/**
 * \brief Initialize filter information to default values.
 *
 * \param info [OUT]  Frame information.
 *
 * \return Return code.
 **/
vtss_rc vtss_packet_port_info_init(vtss_packet_port_info_t *const info);

/** \brief Packet information for each port */
typedef struct {
    vtss_packet_filter_t filter; /**< Packet filtering */
    vtss_etype_t         tpid;   /**< Tag Ethernet Type */
} vtss_packet_port_filter_t;

/**
 * \brief Determine ingress/egress filter for frame per port.
 * The function may be used for ingress filtering, egress filtering or both.
 *
 * \param inst [IN]     Target instance reference.
 * \param info [IN]     Frame information.
 * \param filter [OUT]  Frame filter.
 *
 * \return Return code.
 **/
vtss_rc vtss_packet_port_filter_get(const vtss_inst_t             inst,
                                    const vtss_packet_port_info_t *const info,
                                    vtss_packet_port_filter_t     filter[VTSS_PORT_ARRAY_SIZE]);

#if defined(VTSS_FEATURE_VSTAX)
/**
 * \brief VStaX frame forward mode
 **/
typedef enum
{
    VTSS_VSTAX_FWD_MODE_LOOKUP     = 0, /**< Local lookup in all units */
    VTSS_VSTAX_FWD_MODE_UPSID_PORT = 2, /**< Physical port on UPSID */
    VTSS_VSTAX_FWD_MODE_CPU_UPSID  = 5, /**< CPU queue on UPSID */
    VTSS_VSTAX_FWD_MODE_CPU_ALL    = 6  /**< CPU queue on all UPSIDs */
} vtss_vstax_fwd_mode_t;

/**
 * \brief VStaX TTL value, 0-31
 **/
typedef u32 vtss_vstax_ttl_t;

#define VTSS_VSTAX_TTL_PORT 0 /**< TTL value configured for port is used */

#define VTSS_PRIO_SUPER VTSS_PRIO_END /**< VStaX super priority */

/**
 * \brief VStaX frame header used for transmission
 **/
typedef struct
{
    vtss_vstax_fwd_mode_t  fwd_mode;   /**< Frame forward mode */
    vtss_vstax_ttl_t       ttl;        /**< TTL value or VTSS_VSTAX_TTL_PORT */
    vtss_prio_t            prio;       /**< Priority, VTSS_PRIO_SUPER allowed */
    vtss_vstax_upsid_t     upsid;      /**< Dest. unit port set: FWD_MODE_UPSID_PORT/CPU_UPSID */
    vtss_tci_t             tci;        /**< VLAN tag information */
    vtss_port_no_t         port_no;    /**< Dest. port: FWD_MODE_UPSID_PORT - Src. port : FWD_MODE_CPU_UPSID/ALL */
    u8                     chip_port;  /**< If port_no is VTSS_PORT_NO_NONE, then chip_port is used as is */
#if defined(VTSS_FEATURE_AGGR_GLAG)
    vtss_glag_no_t         glag_no;    /**< GLAG number: FWD_MODE_CPU_UPSID/ALL */
#endif /* VTSS_FEATURE_AGGR_GLAG */
    vtss_packet_rx_queue_t queue_no;   /**< CPU queue : FWD_MODE_CPU_UPSID/ALL */
#if defined(VTSS_FEATURE_VSTAX_V2)
    BOOL                   keep_ttl;   /**< Special TTL handling : FWD_MODE_CPU_ALL */
    u8                     dp;         /**< 2 bits drop precedence level */
#endif
} vtss_vstax_tx_header_t;

/**
 * \brief Send frame on VStaX port.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param header [IN]   VStaX header structure.
 * \param frame [IN]    Frame buffer excluding room for CRC.
 * \param length [IN]   Frame length excluding CRC.
 *
 * \return Return code.
 **/
vtss_rc vtss_packet_tx_frame_vstax(const vtss_inst_t             inst,
                                   const vtss_port_no_t          port_no,
                                   const vtss_vstax_tx_header_t  *const header,
                                   const u8                      *const frame,
                                   const u32                     length);


#define VTSS_VSTAX_HDR_SIZE 12 /**< VStaX header size */
/**
 * \brief Build 12 bytes VStaX frame header.
 *
 * \param inst [IN]     Target instance reference.
 * \param port_no [IN]  Port number.
 * \param header [IN]   VStaX header structure.
 * \param frame [OUT]   Pointer where to write 12 bytes header in frame buffer.
 *
 * \return Return code.
 **/
vtss_rc vtss_packet_vstax_header2frame(const vtss_inst_t             inst,
                                       const vtss_port_no_t          port_no,
                                       const vtss_vstax_tx_header_t  *const header,
                                       u8                            *const frame);


/**
 * \brief Extract 12 bytes VStaX header.
 *
 * \param inst [IN]     Target instance reference.
 * \param frame [IN]    Pointer where to extract 12 bytes header from frame buffer.
 * \param header [OUT]  VStaX header structure.
 *
 * \return Return code.
 **/
vtss_rc vtss_packet_vstax_frame2header(const vtss_inst_t       inst,
                                       const u8                *const frame,
                                       vtss_vstax_rx_header_t  *const header);
#endif /* VTSS_FEATURE_VSTAX */

/**
 * OAM types to be used when encoding an injection header.
 */
typedef enum {
    VTSS_PACKET_OAM_TYPE_NONE = 0, /**< No-op                                                      */
    VTSS_PACKET_OAM_TYPE_CCM,      /**< Continuity Check Message                                   */
    VTSS_PACKET_OAM_TYPE_CCM_LM,   /**< Continuity Check Message with Loss Measurement information */
    VTSS_PACKET_OAM_TYPE_LBM,      /**< Loopback Message                                           */
    VTSS_PACKET_OAM_TYPE_LBR,      /**< Loopback Reply                                             */
    VTSS_PACKET_OAM_TYPE_LMM,      /**< Loss Measurement Message                                   */
    VTSS_PACKET_OAM_TYPE_LMR,      /**< Loss Measurement Reply                                     */
    VTSS_PACKET_OAM_TYPE_DMM,      /**< Delay Measurement Message                                  */
    VTSS_PACKET_OAM_TYPE_DMR,      /**< Delay Measurement Reply                                    */
    VTSS_PACKET_OAM_TYPE_1DM,      /**< A.k.a. SDM, One-Way Delay Measurement                      */
    VTSS_PACKET_OAM_TYPE_LTM,      /**< Link Trace message                                         */
    VTSS_PACKET_OAM_TYPE_LTR,      /**< Link Trace Reply                                           */
    VTSS_PACKET_OAM_TYPE_GENERIC,  /**< Generic OAM type                                           */
} vtss_packet_oam_type_t;

/**
 * PTP actions used when encoding an injection header.
 */
typedef enum {
    VTSS_PACKET_PTP_ACTION_NONE = 0,         /**< No-op                                                   */
    VTSS_PACKET_PTP_ACTION_ONE_STEP,         /**< One-step PTP                                            */
    VTSS_PACKET_PTP_ACTION_TWO_STEP,         /**< Two-step PTP                                            */
    VTSS_PACKET_PTP_ACTION_ONE_AND_TWO_STEP, /**< Both one- and two-step PTP                              */
    VTSS_PACKET_PTP_ACTION_ORIGIN_TIMESTAMP, /**< Update time-of-day in PTP frame's originTimestamp field */
} vtss_packet_ptp_action_t;

/**
 * VLAN tag with "arbitrary" TPID.
 */
typedef struct {
    vtss_etype_t   tpid; /**< Tag Protocol Identifier */
    vtss_tagprio_t pcp;  /**< Priority Code Point     */
    BOOL           dei;  /**< Drop Eligible Indicator */
    vtss_vid_t     vid;  /**< VLAN Identifier         */
} vtss_vlan_tag_t;

/**
 * Tag type the frame was received with.
 */
typedef enum {
    VTSS_TAG_TYPE_UNTAGGED = 0,    /**< Frame was received untagged or on an unaware port or (on non-Lu28-archs) on with a tag that didn't match the port type. */
    VTSS_TAG_TYPE_C_TAGGED,        /**< Frame was received with a  C-tag                  */
    VTSS_TAG_TYPE_S_TAGGED,        /**< Frame was received with an S-tag                  */
    VTSS_TAG_TYPE_S_CUSTOM_TAGGED, /**< Frame was received with a custom S-tag            */
} vtss_tag_type_t;

#if defined(VTSS_FEATURE_AFI_SWC)
/** \page AFI Automatic Frame Injector.
 *
 * The automatic frame injector allows for periodic
 * transmission of frames from within the switch core.
 *
 * The flow for setting up a frame to be periodically
 * injected is as follows for external CPUs:\n
 *
 * 1) Allocate resources for the frame using vtss_afi_alloc().
 *    This function takes a structure in which the application
 *    must specify properties, like transmissions per seconds.\n
 *
 * 2) The vtss_afi_alloc() function returns a unique ID, which
 *    must be transferred in vtss_packet_tx_info_t::afi_id of the
 *    structure that is passed to vtss_packet_tx_hdr_encode().\n
 *
 * 3) Transmit the frame with the injection header that came
 *    out of vtss_packet_tx_hdr_encode().\n
 *
 * 4) In order to cancel the frame transmission, call vtss_afi_free()
 *    with the ID returned originally by vtss_afi_alloc().\n
 *
 * 5) If the port onto which the frame is transmitted gets a link-down event,
 *    the Vitesse API will autonomously pause frame transmission.
 *    When the link comes back up, the external CPU will have to re-transmit
 *    the frame with the ID retrieved in the original call to vtss_afi_alloc(),
 *    because internally in the API, the frame is seen as paused, not cancelled.
 *
 * If using the FDMA driver to inject AFI-based frames, the FDMA driver will take
 * care of allocating, pausing, resuming, and freeing frames. So in this event, the
 * application should not call any of the vtss_afi_XXX() functions, but use the
 * FDMA driver's interface (vtss_fdma_tx_info_t).
 */

/**
 * \brief AFI Frame description structure.
 *
 * Contains properties that describe how a frame should be
 * injected periodically.
 * Upon exit from vtss_afi_alloc() it contains additional
 * information about the injection.
 * For future compatibility, memset() this structure
 * to 0 before filling it in.
 */
typedef struct {
    /**
     * [IN]
     * Frame rate (in frames per second) at which a frame
     * should be transmitted.
     */
    u32 fps;

    /**
     * [OUT]
     * On some platforms, it may not be possible to achieve
     * the requested frame rate in one single go. A work-around
     * for this is to inject the same frame with different
     * frame rates. Typically, a particular desired frame rate
     * can be obtained by transmitting the same frame at most
     * four times.
     * vtss_afi_alloc() fills in #actual_fps with the number
     * of frames that the AFI will actually send.
     * It is guaranteed that #actual_fps <= #fps, so that
     * vtss_afi_alloc() can be called repeatedly with the
     * missing frame count.
     */
    u32 actual_fps;

} vtss_afi_frm_dscr_t;

/**
 * \brief AFI identifier. Identifies the resources allocated for a periodically injected frame.
 */
typedef u32 vtss_afi_id_t;

#define VTSS_AFI_ID_NONE (0xFFFFFFFF) /**< Use this to encode Tx headers for non-AFI frame */
#define VTSS_AFI_FPS_MAX (3800000)    /**< Maximum number of frames per second we support (corresponds to wirespeed 2.5Gbps 64 byte frames) */

/**
 * \brief Allocate resources for an AFI frame.
 *
 * This function allocates resources required for a frame subject to
 * automatic frame injection.
 *
 * The function returns an ID that can be used in subsequent calls to
 * vtss_packet_tx_hdr_encode() and vtss_afi_free(). The ID is only valid
 * for one single frame.
 *
 * \param inst [IN]    Target instance reference.
 * \param dscr [INOUT] Pointer to structure describing how the frame should be injected.
 * \param id   [OUT]   Pointer receiving a unique ID that identifies the allocated resources for this frame.
 *
 * \return VTSS_RC_OK on success. VTSS_RC_ERROR on error.
 */
vtss_rc vtss_afi_alloc(const vtss_inst_t inst, vtss_afi_frm_dscr_t *const dscr, vtss_afi_id_t *const id);

/**
 * \brief Free resources associated with an AFI frame.
 *
 * This function cancels any ongoing transmissions of the frame
 * associated with the ID passed to this function, and frees the
 * allocated resources.
 *
 * \param inst [IN] Target instance reference.
 * \param id   [IN] ID of AFI frame to cancel and free resources for.
 *
 * \return VTSS_RC_OK on success. VTSS_RC_ERROR on error.
 */
vtss_rc vtss_afi_free(const vtss_inst_t inst, vtss_afi_id_t id);
#endif /* defined(VTSS_FEATURE_AFI_SWC) */

#define VTSS_JR1_PACKET_HDR_SIZE_BYTES 52 /**< Max header size. Worst case: XTR = INJ (XTR: 24 bytes for IFH + 4 bytes for queue number + 24 bytes for Rx super-prio artificial IFH, INJ: 24 bytes for IFH + 4 bytes VLAN tag + 24 bytes for signature IFH for multicast injections) */
#define VTSS_L28_PACKET_HDR_SIZE_BYTES 24 /**< Max header size. Worst case: INJ (8 bytes for IFH + 4 bytes CMD field + 12 bytes for VStaX header) */
#define VTSS_SVL_PACKET_HDR_SIZE_BYTES 20 /**< Max header size. Worst case: INJ (16 bytes for IFH + 4 bytes for VLAN tag)                         */
#define VTSS_L26_PACKET_HDR_SIZE_BYTES 16 /**< Max header size. Worst case: INJ (8 bytes for IFH + 4 for timestamp + 4 for VLAN tag)              */
#define VTSS_JR2_PACKET_HDR_SIZE_BYTES 90 /**< JR2-TBD: Use correct value; this is WRONG!!! */

// Find the largest required header size.
#if defined(VTSS_ARCH_JAGUAR_2)
// JR2-TBD: This is likely to be wrong; update when real JR2 size is known
#define VTSS_PACKET_HDR_SIZE_BYTES VTSS_JR2_PACKET_HDR_SIZE_BYTES /**< Maximum header size. This define is only useful if you only compile for one target. */
#elif defined(VTSS_ARCH_JAGUAR_1)
#define VTSS_PACKET_HDR_SIZE_BYTES VTSS_JR1_PACKET_HDR_SIZE_BYTES /**< Maximum header size. This define is only useful if you only compile for one target. */
#elif defined(VTSS_ARCH_LUTON28)
#define VTSS_PACKET_HDR_SIZE_BYTES VTSS_L28_PACKET_HDR_SIZE_BYTES /**< Maximum header size. This define is only useful if you only compile for one target. */
#elif defined(VTSS_ARCH_SERVAL)
#define VTSS_PACKET_HDR_SIZE_BYTES VTSS_SVL_PACKET_HDR_SIZE_BYTES /**< Maximum header size. This define is only useful if you only compile for one target. */
#elif defined(VTSS_ARCH_LUTON26)
#define VTSS_PACKET_HDR_SIZE_BYTES VTSS_L26_PACKET_HDR_SIZE_BYTES /**< Maximum header size. This define is only useful if you only compile for one target. */
#endif

#define VTSS_SVL_RX_IFH_SIZE 16 /**< Serval1 RX IFH size is 16 bytes*/
#define VTSS_JR1_RX_IFH_SIZE 24 /**< Jaguar1 RX IFH size is 24 bytes*/
#define VTSS_L26_RX_IFH_SIZE  8 /**< Luton26 RX IFH size is  8 bytes*/
#define VTSS_JR2_RX_IFH_SIZE 24 /**< JR2-TBD: Jaguar2 RX IFH size is ??? bytes*/

/**
 * \brief Input structure to vtss_packet_rx_hdr_decode().
 *
 * In order to be able to decode the side-band information coming with
 * a frame when extracting through an external CPU with extraction headers
 * enabled, a number of inputs are required.
 *
 * This structure provides this meta data.
 *
 * For future compatibility, memset() this structure to 0 prior
 * to filling it in.
 */
typedef struct {
    /**
     * This function may provide trace output as it decodes
     * the header.
     * Since it may be called from various contexts, hereunder
     * interrupt (DSR) if e.g. used by the FDMA driver, it's
     * crucial that it calls the correct trace print function,
     * that is, one that can print without making waiting
     * points.
     * If #no_wait is TRUE, the trace output will be
     * directed to VTSS_TRACE_GROUP_FDMA_IRQ, which is
     * assumed to be able to print directly to the console
     * without waiting.
     * If #no_wait is FALSE, the trace output will
     * be directed to VTSS_TRACE_GROUP_PACKET, which is
     * assumed to print from thread context.
     *
     * <pre>
     * Required to be set?
     * -------------------
     *   Luton26: Y
     *   Luton28: Y
     *   Jaguar1: Y
     *   Serval:  Y
     * </pre>
     */
    BOOL no_wait;

    /**
     * Chip number on which this frame was extracted.
     * It is not possible to deduct from the binary extraction header
     * which device the frame was extracted. In order to be able to
     * provide a logical source port in the decoded extraction properties,
     * a chip number is required for targets made up of multiple
     * physical chips.
     *
     * <pre>
     * Required to be set?
     * -------------------
     *   Luton26: N (assumed to be 0)
     *   Luton28: N (assumed to be 0)
     *   Jaguar1: Y
     *   Serval:  N (assumed to be 0)
     * </pre>
     */
    vtss_chip_no_t chip_no;

    /**
     * Rx queue number from which this frame was really extracted.
     * This is only needed on particular architectures, where
     * this info is not part of the extraction header.
     *
     * Jaguar1:
     *   Frames received in super priority queues (queue 8 and 9),
     *   do not contain extraction headers. Only the FDMA driver
     *   and external CPUs using register-based readings are able to
     *   extract super priority frames. Such applications will have
     *   to construct a synthetic IFH in order to get it decoded
     *   with this function.
     *   Also, in cases where a frame hits a non-default IS2 rule,
     *   the extraction queue mask in the IFH is overloaded with
     *   an ACL index. Therefore, in order to be able to construct
     *   an extraction queue mask in the decoded extraction header,
     *   #xtr_qu should be filled in.
     *
     * <pre>
     * Required to be set?
     * -------------------
     *   Luton26: N
     *   Luton28: N
     *   Jaguar1: Y
     *   Serval:  N
     * </pre>
     */
    vtss_packet_rx_queue_t xtr_qu;

    /**
     * The Ethernet type of the received frame.
     *      
     * This is needed in order to be able to decode
     * vtss_packet_rx_info_t::tag_type correctly.
     *
     * <pre>
     * Required to be set?
     * -------------------
     *   Luton26: Y
     *   Luton28: Y
     *   Jaguar1: Y
     *   Serval:  Y
     * </pre>
     */
    vtss_etype_t etype;

    /**
     * Frame checksum.
     *
     * On some architectures, the frame's FCS/CRC may be used to hold
     * additional side-band information about the frame itself.
     *
     * To be able to decode this information, the caller must therefore
     * find the end of the frame and extract the 32-bit FCS and place it
     * here.
     *
     * <pre>
     * Required to be set?
     * -------------------
     *   Luton26: N
     *   Luton28: N
     *   Jaguar1: Y
     *   Serval:  N
     * </pre>
     */
    u32 fcs;

    /**
     * Software timestamp of packet.
     *
     * This may be used by an external CPU to S/W-wise timestamp the packet.
     * If the FDMA driver is being used to extract frames, it will take
     * care of filling this field in.
     *
     * The field will be copied directly to vtss_packet_rx_info_t::sw_tstamp.
     *
     * <pre>
     * Required to be set?
     * -------------------
     *   Luton26: N
     *   Luton28: N
     *   Jaguar1: N
     *   Serval:  N
     * </pre>
     */
    VTSS_OS_TIMESTAMP_TYPE sw_tstamp;

    /**
     * The length of the frame from DMAC to end-of-frame excluding FCS.
     *
     * The extraction header normally doesn't include the frame length, so
     * in order to provide a structure to the application that contains all
     * meta data about any given frame, the application may fill in this member,
     * which will be copied exactly as is into vtss_packet_rx_info_t::length.
     *
     * If the FDMA driver is used to extract frames, it will take care of filling
     * this field in.
     *
     * <pre>
     * Required to be set?
     * -------------------
     *   Luton26: N
     *   Luton28: N
     *   Jaguar1: N
     *   Serval:  N
     * </pre>
     */
    u32 length;

} vtss_packet_rx_meta_t;

/**
 * \brief Provides additional info on decoded extraction header.
 *
 * In some cases, a frame received by the CPU should be
 * dropped. This cannot always be detected by just looking
 * at the decoded extraction header.
 *
 * This enum provides the caller of vtss_packet_rx_hdr_decode()
 * with a means to determine whether to drop or forward the frame
 * further to the application. Whether to actually drop it is fully up
 * to the application.
 *
 * The individual enums may be combined into a mask.
 */
typedef enum {

    /**
     * No hints.
     */
    VTSS_PACKET_RX_HINTS_NONE = 0x00,

    /**
     * If a frame is received on a C-port with a "foreign" tag
     * (i.e. an S-tag or S-custom-tag), the frame should possibly
     * be dropped. This is indicated with this enum being member
     * of the hints flags.
     *
     * The same goes for frames received on S-ports or S-custom-ports
     * with "foreign" tags.
     *
     * <pre>
     * Can be set by:
     * -------------------
     *   Luton26: Y
     *   Luton28: Y
     *   Jaguar1: Y
     *   Serval:  Y
     * </pre>
     */
    VTSS_PACKET_RX_HINTS_VLAN_TAG_MISMATCH = 0x01,

    /**
     * If a tagged frame is received on a port that only should
     * accept untagged frames or if an untagged frame is received
     * on a port that should only accept tagged frames, then this
     * will be set. In general, the application should not
     * drop such frames, because e.g. BPDUs are normally untagged.
     *
     * <pre>
     * Can be set by:
     * -------------------
     *   Luton26: Y
     *   Luton28: Y
     *   Jaguar1: Y
     *   Serval:  Y
     * </pre>
     */
    VTSS_PACKET_RX_HINTS_VLAN_FRAME_MISMATCH = 0x02,

    /**
     * If a frame gets classified to a VLAN ID on a port that is not member of
     * that VID, the hints flags include this enum.
     *
     * <pre>
     * Can be set by:
     * -------------------
     *   Luton26: Y
     *   Luton28: Y
     *   Jaguar1: Y
     *   Serval:  Y
     * </pre>
     */
    VTSS_PACKET_RX_HINTS_VID_MISMATCH = 0x04,
} vtss_packet_rx_hints_t;

/**
 * \brief Decoded extraction header properties.
 *
 * This structure gets populated with a call to vtss_packet_rx_hdr_decode().
 *
 * Many decoded parameters have two fields in the structure:
 * One indicating the value of the parameter (e.g. #tstamp_id),
 * and another indicating if the parameter is actually decoded or not
 * (e.g. #tstamp_id_decoded).
 *
 * The reason for having an XXX_decoded boolean for every parameter is that
 * the information held in the extraction header is very different on the
 * various Vitesse chips, and even on the same chip type, the information
 * may be overloaded, depending on the incoming frame type.
 *
 * Most parameters don't have a decoded field, and in that case, they are
 * always decoded.
 */
typedef struct {
    /**
     * The #hints member is useful for applications that wish to perform
     * some kind of ingress filtering on received frames.
     * Please refer to vtss_packet_rx_hints_t for a full description.
     * Each of the enumerations can be bitwise ORed into the #hints member.
     *
     * <pre>
     * Validity:
     *   Luton26: Y
     *   Luton28: Y
     *   Jaguar1: Y
     *   Serval : Y
     * </pre>
     */
    u32 hints;

    /**
     * The length of the frame from DMAC to end-of-frame excluding FCS.
     *
     * This is a copy of vtss_packet_rx_meta_t::length.
     *
     * <pre>
     * Validity:
     *   Luton26: Y
     *   Luton28: Y
     *   Jaguar1: Y
     *   Serval : Y
     * </pre>
     */
    u32 length;

    /**
     * The logical source port on which the frame was received.
     * In a few cases, this may be VTSS_PORT_NO_NONE, if the physical
     * source port was not part of the port map (e.g. in JR-48, where
     * sFlow frames were Tx sampled and received on an interconnect port).
     *
     * <pre>
     * Validity:
     *   Luton26: Y
     *   Luton28: Y
     *   Jaguar1: Y
     *   Serval : Y
     * </pre>
     */
    vtss_port_no_t port_no;

    /**
     * The Global Link Aggregation Group this frame was received on.
     * VTSS_GLAG_NO_NONE if not received on a GLAG (also on non-supporting
     * architectures).
     * If received on a GLAG, #port_no will contain the first
     * member port in the GLAG.
     *
     * If received on a stack port, #glag_no will always be VTSS_GLAG_NO_NONE,
     * but if the sender supports it in S/W, the stack header can be filled
     * with a glag_no before it is transmitted. To obtain this glag_no on
     * the receiving side, you can find it in vstax member's glag_no member.
     *
     * <pre>
     * Validity:
     *   Luton26: N
     *   Luton28: Y
     *   Jaguar1: Y
     *   Serval : N
     * </pre>
     */
    vtss_glag_no_t glag_no;

    /**
     * The tag type (802.1Q TPID) the frame was received with, if any.
     *
     * Not all architectures support all four enumerations
     * of the tag type.
     *
     * On all architectures, but Luton28, the tag stays in the frame.
     * On Luton28, H/W strips the tag.
     *
     * In general, it works like this:
     *   If a frame is received on a VLAN unaware port,
     *   #tag_type will always be set to VTSS_TAG_TYPE_UNTAGGED,
     *   whether it contains a tag or not. The classified VLAN (#tag's
     *   vid member) will always be the port VID.
     *
     *   If a frame is received on a C-port, then only frames
     *   recveived with a C-tag are marked as VTSS_TAG_TYPE_C_TAGGED.
     *   S- and S-custom-tagged frames will be marked as VTSS_TAG_TYPE_UNTAGGED.
     *   but notice that the frame *may* be classified according to
     *   that tag, anyway (architecture dependent).
     *   The #hints vlan_tag_mismatch member will be set to TRUE to indicate
     *   such a condition.
     *
     *   The same goes for frames received on an S-port or S-custom-port with
     *   a "foreign" VLAN tag. The frame may on some architectures be classified
     *   to the VID in the tag, but on others be classified according to the PVID.
     *
     * If the FDMA driver is being used for extracting frames, it can be configured to
     * drop frames received with a wrong tag. It can also be configured to strip tags
     * if received with a tag according to its port setup (for all-but-Luton28-architectures),
     * and even insert a tag if received with a tag not according to its port setup (for
     * Luton28).
     *
     * <pre>
     * Validity:
     *   Luton26: Y
     *   Luton28: Y
     *   Jaguar1: Y
     *   Serval : Y
     * </pre>
     */
    vtss_tag_type_t tag_type;

    /**
     * VLAN classification information.
     *
     * Contains the classified VLAN information, as opposed to #stripped_tag,
     * which contains the actual VLAN tag as was in the frame.
     *
     * Only the .pcp, .dei, and .vid members are used.
     * Notice that this is not necessarily the classification
     * coming from a VLAN tag; it may come from normal port
     * classification.
     *
     * On Luton28, the .tpid indicates the VLAN tag TPID if
     * the frame was received tagged. This is in order to let
     * the FDMA driver re-insert a VLAN tag on mismatch between
     * VLAN tag in frame and port-setup.
     *
     * <pre>
     * Validity:
     *   Luton26: Y
     *   Luton28: Y
     *   Jaguar1: Y
     *   Serval : Y
     * </pre>
     */
    vtss_vlan_tag_t tag;

    /**
     * VLAN stripped tag information.
     *
     * Opposed to #tag, #stripped_tag contains the VLAN information
     * coming directly from the frame. The classified and frame VLAN
     * information may differ due to ACL/VCAP rules that causes the
     * frame to get classified based on other properties than a possible
     * VLAN tag embedded in the frame.
     *
     * Whenever the .tpid member is non-zero, the tag was stripped from
     * the frame.
     *
     * NOTICE: Only platforms that use the FDMA driver will have the
     * ability to strip tags, and this will only happen when the FDMA
     * driver is configured for it (this is the default, though).
     *
     * <pre>
     * Validity (FDMA only):
     *   Luton26: Y
     *   Luton28: N
     *   Jaguar1: Y
     *   Serval : Y
     * </pre>
     */
    vtss_vlan_tag_t stripped_tag;

    /**
     * CPU extraction queue mask (one bit per CPU extraction queue).
     * Each bit implies the frame was subject to CPU forwarding to the
     * specific queue.
     * The actual queue the frame was received on is given by the
     * most significant bit set in the mask.
     *
     * <pre>
     * Validity:
     *   Luton26: Y
     *   Luton28: Y
     *   Jaguar1: Y (but in some cases, it is constructed rather than showing the true story (constructed if the frame hit a non-default VCAP IS2 rule))
     *   Serval : Y
     * </pre>
     */
    u32 xtr_qu_mask;

    /**
     * The frame's classified QoS class.
     *
     * <pre>
     * Validity:
     *   Luton26: Y
     *   Luton28: Y
     *   Jaguar1: Y
     *   Serval : Y
     * </pre>
     */
    vtss_prio_t cos;

    /**
     * Set if frame has hit a rule in IS2, which copies the frame to the
     * CPU (IS2 actions CPU_COPY_ENA or HIT_ME_ONCE). #acl_idx may contain
     * the IS2 entry number.
     * For Serval, the #acl_idx contains the combined ACL_ID action of
     * the rules hit in IS2.
     *
     * <pre>
     * Validity:
     *   Luton26: Y
     *   Luton28: Y
     *   Jaguar1: Y
     *   Serval : Y
     * </pre>
     */
    BOOL acl_hit;

    /**
     * If #acl_hit is set, this value is the entry number of the rule
     * hit in IS2. If both IS2 lookups hit a rule which copy the frame
     * to the CPU, then the second lookup's entry number is used.
     *
     * For Serval, this is the combined ACL_ID action coming out of the
     * two IS2 look-ups.
     *
     * This is never valid for Luton28.
     *
     * <pre>
     * Validity:
     *   Luton26: Y
     *   Luton28: N
     *   Jaguar1: Y
     *   Serval : Y
     * </pre>
     */
    u32 acl_idx;

    /**
     * Software timestamp of packet.
     *
     * This is a copy of the vtss_packet_rx_meta_t::sw_tstamp field.
     *
     * <pre>
     * Validity:
     *   Luton26: Y
     *   Luton28: Y
     *   Jaguar1: Y
     *   Serval : Y
     * </pre>
     */
    VTSS_OS_TIMESTAMP_TYPE sw_tstamp;

    /**
     * Two-step PTP timestamp identifier (6 bits).
     *
     * On Luton26, this field identifies an Rx timestamp and potential
     * Tx timestamps (if the PTP frame was forwarded to other ports).
     * Notice that #tstamp_id_decoded will be TRUE for all frames that
     * have hit in IS2 rule. This means that the application must make
     * additional checks that this indeed is a PTP frame before relying
     * on #tstamp_id.
     *
     * On Serval, this field identifies Tx timestamps. Rx timestamps
     * are embedded in the extraction header.
     *
     * <pre>
     * Validity:
     *   Luton26: Y
     *   Luton28: N
     *   Jaguar1: N
     *   Serval : Y
     * </pre>
     */
    u32 tstamp_id;
    /**
     * TRUE when #tstamp_id contains valid information, FALSE otherwise.
     */
    BOOL tstamp_id_decoded;

    /**
     * The frame's ingress timestamp.
     *
     * Jaguar1:
     *   Frames getting copied to the CPU for SFlow reasons can never have a valid #hw_tstamp (such frames
     *   will be indicated through #hw_tstamp_decoded == FALSE).
     *   Note that the #hw_tstamp_decoded will indicate TRUE for all other frames, but this is only reliable
     *   if the following register are set-up:
     *     ANA_AC:PS_COMMON:MISC_CTRL.OAM_RX_FRM_EXTEND_ENA    == 0
     *     ANA_AC:PS_COMMON:MISC_CTRL.OAM_RX_TSTAMP_IN_FCS_ENA == 1
     *     ASM:CFG:ETH_CFG.ETH_PRE_MODE                        == 1
     *     DEV1G/DEV25G:DEV_CFG_STATUS:DEV_PTP_CFG.PTP_ENA     == 1
     *     DEV10G:DEV_CFG_STATUS:DEV_PTP_CFG.PTP_CFG_ENA       == 1
     *     DEVCPU_GCB:PTP_CFG:PTP_MISC_CFG.PTP_ENA             == 1
     *
     * Serval:
     *   Two-step PTP frames have the 32-bit Rx timestamp saved in this field.
     *
     * <pre>
     * Validity:
     *   Luton26: N
     *   Luton28: N
     *   Jaguar1: Y
     *   Serval : Y
     * </pre>
     */
    u32 hw_tstamp;
    /**
     * TRUE when #hw_tstamp contains valid information, FALSE otherwise.
     */
    BOOL hw_tstamp_decoded;

#if defined(VTSS_FEATURE_SFLOW)
    /**
     * sFlow type. Indicates if this is copied to the CPU due to Rx or Tx SFlow, or if
     * it's not due to sFlow in the first place (VTSS_SFLOW_TYPE_NONE).
     *
     * Only VTSS_SFLOW_TYPE_NONE, VTSS_SFLOW_TYPE_RX, and VTSS_SFLOW_TYPE_TX are possible.
     *
     * Jaguar:
     *   Note: #sflow_type's RX and TX enumerations are only reliable if
     *   ANA_AC:PS_COMMON:PS_COMMON_CFG.SFLOW_SMPL_ID_IN_STAMP_ENA is set to 1.
     *   However, #sflow_type == VTSS_SFLOW_TYPE_NONE is always reliable.
     *
     * <pre>
     * Validity:
     *   Luton26: Y
     *   Luton28: N
     *   Jaguar1: Y
     *   Serval : Y
     * </pre>
     */
    vtss_sflow_type_t sflow_type;

    /**
     * sFlow port.
     * Indicates the logical sFlow Rx or Tx port number that caused this frame to be copied to the CPU.
     * Only valid if #sflow_type != VTSS_SFLOW_TYPE_NONE.
     *
     * Jaguar:
     *   Note: This is only reliable if ANA_AC:PS_COMMON:PS_COMMON_CFG.SFLOW_SMPL_ID_IN_STAMP_ENA is set to 1.
     *   That bit must be set if Tx sFlow are enabled. If only using Rx sFlows, that bit can be cleared, and
     *   you may use the #port_no member to figure out which port caused this frame.
     *
     * <pre>
     * Validity:
     *   Luton26: Y
     *   Luton28: N
     *   Jaguar1: Y
     *   Serval : Y
     * </pre>
     */
    vtss_port_no_t sflow_port_no;
#endif

    /**
     * Various un-decodable OAM info. Decoding of the OAM info
     * field from the extraction header requires accompanying
     * frame info, and is therefore saved as an opaque type,
     * letting it be up to the application to decode it based on
     * description in the datasheet.
     *
     * Serval:
     *   This corresponds to the contents of the REW_VAL field
     *   in the extraction header. oam_info_decoded = TRUE when
     *   REW_OP[2:0] == 4. Only the 32 lsbits of #oam_info are used.
     *
     * <pre>
     * Validity:
     *   Luton26: N
     *   Luton28: N
     *   Jaguar1: N
     *   Serval : Y
     * </pre>
     */
    u64 oam_info;
    /**
     * TRUE when #oam_info contains valid information, FALSE otherwise.
     */
    BOOL oam_info_decoded;

    /**
     * The N-bit ISDX from IS1 classification, or VTSS_ISDX_NONE.
     *
     * <pre>
     * Validity:
     *   Luton26: N
     *   Luton28: N
     *   Jaguar1: N
     *   Serval : Y
     * </pre>
     */
    vtss_isdx_t isdx;

#if defined(VTSS_FEATURE_VSTAX)
    /**
     * The frame's decoded VStaX header.
     * vtss_vstax_rx_header_t::valid indicates whether the frame
     * was received with a VStaX header.
     *
     * <pre>
     * Validity:
     *   Luton26: N
     *   Luton28: Y
     *   Jaguar1: Y
     *   Serval : N
     * </pre>
     */
    vtss_vstax_rx_header_t vstax;
#endif /* VTSS_FEATURE_VSTAX */

} vtss_packet_rx_info_t;

/**
 * \brief Transmit frames with VStaX header,
 * and if so, how is it specified?
 */
typedef enum {
    VTSS_PACKET_TX_VSTAX_NONE = 0, /**< Don't send frame with VStaX header.                                                            */
    VTSS_PACKET_TX_VSTAX_BIN,      /**< Send frame with VStaX header. The header is already encoded into binary format.                */
    VTSS_PACKET_TX_VSTAX_SYM       /**< Send frame with VStaX header. The header is in symbolic format and needs to be encoded by API. */
} vtss_packet_tx_vstax_t;

/**
 * \brief Injection Properties.
 *
 * Structural properties used to compose a binary injection header useful
 * for injection into an injection-header-enabled port on the switch.
 *
 * This structure must be initialized with vtss_packet_tx_info_init()
 * prior to calling vtss_packet_tx_hdr_encode().
 *
 * For each property, it is indicated which platforms the property is
 * valid for. There are two columns, one named 'A' and another named 'F'.
 *   'A' stands for 'API' and indicates whether the vtss_packet_tx_hdr_encode()
 *       function itself utilizes the value.
 *   'F' stands for 'FDMA' and indicates whether the FDMA driver (which also
 *       uses this structure, and indirectly also the vtss_packet_tx_hdr_encode()
 *       function) requires this value to be set in its "inject packet API" call.
 */
typedef struct {
    /**
     * If FALSE, the analyzer processing is skipped for this frame,
     * and the destination port set is specified with #dst_port_mask.
     * If TRUE, the frame will be passed through the analyzer to find the
     * destination port set. The analyzer looks up the DMAC in the
     * MAC table and forwards based on its findings. The frame may therefore
     * get flooded.
     *
     * Luton26, Jaguar1, and Serval:
     *   If #switch_frm is TRUE, the frame must have inserted a VLAN tag into the
     *   frame prior to transmission, to get it classified to the correct VLAN.
     *   There is one exception to this, namely when masquerading (see #masquerade_port),
     *   where a VLAN tag may or may not be inserted. If not inserted, the masquerade
     *   port's PVID is used for classification. In no VLAN tag is inserted by the
     *   application/FDMA driver, this structure's #tag member's tpid must be set to 0.
     *
     * If FALSE, the destination port set must be specified with #dst_port_mask.
     * <pre>
     * Validity:  A F
     * --------------
     *   Luton26: Y Y
     *   Luton28: Y Y
     *   Jaguar1: Y Y
     *   Serval : Y Y
     * </pre>
     *
     * On Luton26 + Serval: If FALSE the rewriter still uses the tag information for rewriting actions.
     */
    BOOL switch_frm;

    /**
     * This field provides the logical destination port set onto which to send
     * the frame and may thus be used to multicast the same frame on
     * multiple front ports in one go.
     * The field is only used if #switch_frm is FALSE.
     *
     * If the frame is going to be transmitted with a VStaX header (tx_vstax_hdr is != VTSS_PACKET_TX_VSTAX_NONE)
     * the dst_port_mask must have exactly one bit set, representing the stack port (A or B) to
     * transmit the frame to.
     * Also, if the frame is subject to periodic transmission through the AFI, exactly one bit
     * must be set.
     *
     * Luton28: #dst_port_mask is only used to figure out which stack port to send a possible stack frame to.
     *          It's up to the application, to take care of injecting the frame correctly using either the
     *          CPU Port module or a direct port. If using the FDMA driver, there are no worries, since it
     *          takes care of this.
     *
     * Jaguar1: In 48-port solutions, only one bit can be set if hitting the secondary device,
     *          unless using the FDMA driver v. 3, which will take care of injecting the frame
     *          multiple times.
     *
     * <pre>
     * Validity:  A F
     * --------------
     *   Luton26: Y Y
     *   Luton28: Y Y
     *   Jaguar1: Y Y
     *   Serval : Y Y
     * </pre>
     */
    u64 dst_port_mask;

    /**
     * On some architectures, the frame length must be specified in the injection
     * header. The length - in bytes - is the size of the frame starting from the
     * DMAC up to, but not including, the FCS/CRC.
     *
     * Luton28 notes:
     *   Remember to include the size of a possible VStaX header and a possible
     *   additional VLAN tag added by S/W (see also description of stack_hdr_bin below).
     *
     * <pre>
     * Validity:  A F
     * --------------
     *   Luton26: N N
     *   Luton28: Y N (the FDMA driver inserts this member itself)
     *   Jaguar1: N N
     *   Serval : N N
     * </pre>
     */
    u32 frm_len;

    /**
     * VLAN tag information.
     *
     * Use of this field is architecture specific, and depends on whether
     * #switch_frm is TRUE or FALSE.
     *
     * #switch_frm == TRUE:
     *   The frame will get classified according to tag.vid, tag.dei, and tag.pcp.
     *
     *   On non-Luton28 architectures, an application that directly calls
     *   vtss_packet_tx_hdr_encode() must insert a VLAN tag into the frame
     *   with these properties, and the vtss_packet_tx_hdr_encode() function
     *   will not use #tag for anything.
     *   If the application uses the FDMA, the FDMA driver code will insert
     *   the tag into the frame.
     *
     *   On Luton28, no tags should be inserted by the application.
     *
     *   When masquerading, the FDMA driver can be controlled not to insert a VLAN
     *   tag into the frame by setting tag.vid to 0. See also #masquerade_port.
     *
     * #switch_frm == FALSE:
     *   On Serval, egress ES0 lookups occur even when the rewriter is disabled.
     *   The result of the lookup may be used in e.g. the VOE to count frames.
     *   In order to get the VOE to count in the correct buckets, the #tag's
     *   pcp member must be set correctly.
     *
     *   If #tag's tpid member is non-zero, the FDMA driver inserts a tag into
     *   the frame (this must be done by application software if running without
     *   the FDMA). In this case, the chip's rewriter will be disabled, so the
     *   frame will be sent as is. The inserted tag will use all members of #tag
     *   (i.e. tag.gpid, tag.vid, tag.pcp, and tag.dei).
     *
     *   If #tag's tpid member is zero, the FDMA driver will not insert a tag
     *   into the frame.
     *
     *   The following applies to all platforms but Luton28 and Jaguar1:
     *
     *   If tpid is zero, rewriting of the frame can now be controlled with #tag's
     *   vid member: If vid is zero, the chip's rewriter will be disabled, so the
     *   frame will be sent as is. If vid is non-zero, the vid will be inserted
     *   into the injection header's classified VID field, and the chip's rewriter
     *   will be enabled, causing the frame to potentially be rewritten by the chip.
     *
     * <pre>
     * Validity:  A F
     * --------------
     *   Luton26: N Y
     *   Luton28: Y Y (A: when #switch_frm == TRUE, F: Always).
     *   Jaguar1: N Y
     *   Serval : Y Y (A: tag.pcp when #switch_frm == FALSE, F: Always).
     * </pre>
     */
    vtss_vlan_tag_t tag;

    /**
     * The aggregation code that this frame will use. The 4-bit number
     * maps directly to the corresponding field in the IFH, but is
     * only set if the frame is being switched (the #switch_frm member
     * is TRUE) and the destination port number is VTSS_CPU_PM_NUMBER.
     *
     * If using the FDMA driver, the application code does not need
     * set it.
     *
     * <pre>
     * Validity:  A F
     * --------------
     *   Luton26: N N
     *   Luton28: Y N (the FDMA driver uses the current aggrecation code configuration and the frame data itself to compute and insert the #aggr_code).
     *   Jaguar1: N N
     *   Serval : N N
     * </pre>
     */
    u8 aggr_code;

    /**
     * The QoS class that this frame will be transmitted on.
     * This is a number in range [0; 8].
     *
     * If you set it to '8', the frame will be transmitted with super-priority,
     * i.e. with even higher priority than the highest priority achievable for
     * normal data traffic (if supported on the platform). This is not a valid
     * setting if #switch_frm == TRUE.
     *
     * Luton26 and Serval notes:
     *   A value of '8' (super priority) will be converted to a value of '7'.
     *
     * Luton28 notes:
     *   Super-priority injection is default on Luton28 when #switch_frm == FALSE
     *   and only directing to one single front port. So setting #cos to 8 has no
     *   effect other than it will be replaced by the value 7.
     *   Only bits [2:1] will be used, since it only supports four classes.
     *
     * Jaguar notes:
     *   For switched frames (#switch_frm == TRUE), #cos goes
     *   into the VLAN tag's PCP in order to have the switch core classify
     *   it to the given QoS class. The reason is that it's not possible
     *   to use IFH.vstax_avail = 1 when transmitting switched in a stacking
     *   configuration. QoS classification based on VLAN.PCP is enabled on
     *   the CPU port.
     *
     * <pre>
     * Validity:  A F
     * --------------
     *   Luton26: Y Y
     *   Luton28: Y Y
     *   Jaguar1: Y Y
     *   Serval : Y Y
     * </pre>
     */
    vtss_prio_t cos;

#ifdef VTSS_FEATURE_VSTAX
    /**
     * If a frame is going to be transmitted with a VStaX header, set this
     * member to a value different from VTSS_PACKET_TX_VSTAX_NONE.
     *
     * When transmitting with stack header, the application may choose to
     * either provide a binary or a non-binary stack header to this function.
     * The binary, selected with #tx_vstax_hdr == VTSS_PACKET_TX_VSTAX_BIN,
     * is useful if the application uses the same stack header in all frames
     * it may send. Instead of encoding it every time, it may encode it once
     * (with vtss_packet_vstax_header2frame() and copy it to the bin member of
     * #vstax everytime it needs to send a frame with that stack header.
     *
     * On the other hand, if the stack header changes from time to time,
     * the application will probably wish to use VTSS_PACKET_TX_VSTAX_SYM,
     * which causes the API to encode the stack header for it.
     *
     * <pre>
     * Validity:  A F
     * --------------
     *   Luton26: N N
     *   Luton28: Y Y
     *   Jaguar1: Y Y
     *   Serval : N N
     * </pre>
     */
    vtss_packet_tx_vstax_t tx_vstax_hdr;

    /**
     * Contains the VStaX header in either binary or symbolic form.
     */
    union {
        /**
         * This is the binary version of the VStaX header to insert when the
         * frame gets transmitted on a stack port.
         * It is only used if #tx_vstax_hdr is VTSS_PACKET_TX_VSTAX_BIN.
         * It may be composed with vtss_packet_vstax_header2frame()
         *
         * Luton28 notes:
         *   On Luton28, the stack header is not part of the injection header,
         *   so it's up to the application to insert an encoded VStaX header
         *   into the correct position of the frame payload (use
         *   vtss_packet_vstax_header2frame() to encode the VStaX header).
         *   The FDMA driver will take care of this if that is being used.
         *
         * <pre>
         * Validity:  A F
         * --------------
         *   Luton26: N N
         *   Luton28: N Y
         *   Jaguar1: Y Y
         *   Serval : N N
         * </pre>
         */
        u8 bin[VTSS_VSTAX_HDR_SIZE];

        /**
         * This is the symbolic version of the VStaX header to insert when the
         * frame gets transmitted on a stack port.
         * The binary version will be encoded vtss_packet_tx_hdr_encode().
         * It is only used if #tx_vstax_hdr is VTSS_PACKET_TX_VSTAX_SYM.
         *
         * Luton28 notes:
         *   On Luton28, the stack header is not part of the injection header,
         *   so it's up to the application to insert an encoded VStaX header
         *   into the correct position of the frame payload (use
         *   vtss_packet_vstax_header2frame() to encode the VStaX header).
         *   The FDMA driver will take care of this if that is being used.
         *
         * <pre>
         * Validity:  A F
         * --------------
         *   Luton26: N N
         *   Luton28: N Y
         *   Jaguar1: Y Y
         *   Serval : N N
         * </pre>
         */
        vtss_vstax_tx_header_t sym;
    } vstax; /**< Contains the VStaX header in either binary or symbolic form. */

#endif /* VTSS_FEATURE_VSTAX */

    /**
     * The frame's Precision Time Protocol action.
     * See #vtss_packet_ptp_action_t for the enumeration.
     * Ignored when #switch_frm is TRUE.
     *
     * When != VTSS_PACKET_PTP_ACTION_NONE, the #ptp_timestamp and #ptp_id fields must be filled in.
     *
     * <pre>
     * Validity:  A F
     * --------------
     *   Luton26: Y Y - except for enumeration VTSS_PACKET_PTP_ACTION_ORIGIN_TIMESTAMP.
     *   Luton28: N N
     *   Jaguar1: N N
     *   Serval : Y Y - except for enumeration VTSS_PACKET_PTP_ACTION_ONE_AND_TWO_STEP.
     * </pre>
     */
    vtss_packet_ptp_action_t ptp_action;

    /**
     * The PTP identifier used for two-step PTP actions.
     * The CPU can only use from IDs 0 through 3.
     * Ignored when #switch_frm is TRUE.
     *
     * Used when #ptp_action == VTSS_PACKET_PTP_ACTION_TWO_STEP.
     *
     * <pre>
     * Validity:  A F
     * --------------
     *   Luton26: Y Y
     *   Luton28: N N
     *   Jaguar1: N N
     *   Serval : Y Y
     * </pre>
     */
    u8 ptp_id;

    /**
     * Holds the PTP timestamp indicating when the injection started.
     * The rewriter can then calculate a residence time based on this and the
     * frame's transmission timestamp.
     * Ignored when #switch_frm is TRUE.
     *
     * Used when #ptp_action is != VTSS_PACKET_PTP_ACTION_NONE.
     *
     * <pre>
     * Validity:  A F
     * --------------
     *   Luton26: Y Y
     *   Luton28: N N
     *   Jaguar1: N N
     *   Serval : Y Y
     * </pre>
     */
    u32 ptp_timestamp;

    /**
     * Latch timestamp into a switch core register when the frame
     * is transmitted. This register can then be read-out by S/W
     * to obtain the actual transmission time for that frame.
     * <pre>
     * Encoding:
     *   0: Don't latch timestamp.
     *   1: Latch timestamp into register 0.
     *   2: Latch timestamp into register 1.
     *   3: Latch timestamp into register 2.
     * </pre>
     *
     * <pre>
     * Validity:  A F
     * --------------
     *   Luton26: N N
     *   Luton28: N N
     *   Jaguar1: Y Y
     *   Serval : N N
     * </pre>
     */
    u32 latch_timestamp;

    /**
     * OAM type.
     *
     * Only used if #ptp_action is VTSS_PACKET_PTP_ACTION_NONE.
     *
     * See vtss_packet_oam_type_t for a description.
     * Ignored when #switch_frm is TRUE.
     *
     * <pre>
     * Validity:  A F
     * --------------
     *   Luton26: N N
     *   Luton28: N N
     *   Jaguar1: N N
     *   Serval : Y Y
     * </pre>
     */
    vtss_packet_oam_type_t oam_type;

    /**
     * Ingress Service Index.
     *
     * If not VTSS_ISDX_NONE, it will be used in ES0
     * lookups instead of the frame's classified VID.
     * See also #isdx_dont_use.
     * Ignored when #switch_frm is TRUE.
     *
     * <pre>
     * Validity:  A F
     * --------------
     *   Luton26: N N
     *   Luton28: N N
     *   Jaguar1: N N
     *   Serval : Y Y
     * </pre>
     */
    vtss_isdx_t isdx;

    /**
     * When set to TRUE, #isdx is not used for ES0 lookups,
     * only for frame counting.
     *
     * Ignored when #switch_frm is TRUE or #isdx is VTSS_ISDX_NONE.
     *
     * <pre>
     * Validity:
     *   Luton26: N
     *   Luton28: N
     *   Jaguar1: N
     *   Serval : Y
     * </pre>
     */
    BOOL isdx_dont_use;

    /**
     * Drop Precedence.
     *
     * The frame's drop precedence level after policing.
     * Ignored when #switch_frm is TRUE.
     *
     * <pre>
     * Validity:  A F
     * --------------
     *   Luton26: N N
     *   Luton28: N N
     *   Jaguar1: N N
     *   Serval : Y Y
     * </pre>
     */
    vtss_dp_level_t dp;

    /**
     * Masquerade port.
     * When masquerading, the frame will be handled as if it was received by
     * the ingress port specified in #masquerade_port.
     *
     * Its value will not be used unless #switch_frm is TRUE.
     * Also, when masquerading, the FDMA driver may or may not insert a VLAN tag
     * into the frame prior to transmission. Please consult the FDMA driver API
     * to locate the property that allows for insertion of a VLAN tag.
     *
     * Set it to VTSS_PORT_NO_NONE to disable masquerading.
     *
     * <pre>
     * Validity:  A F
     * --------------
     *   Luton26: N N
     *   Luton28: N N
     *   Jaguar1: N N
     *   Serval : Y Y
     * </pre>
     */
    vtss_port_no_t masquerade_port;

#if defined(VTSS_FEATURE_AFI_SWC)
    /**
     * Automatic Frame Injector ID.
     *
     * The ID of a previously allocated AFI resource.
     *
     * Set to VTSS_AFI_ID_NONE if you don't want
     * a frame injected periodically.
     *
     * If using the FDMA to inject frames, the value of this
     * field is not used for anything. The FDMA driver has
     * its own Tx properties that define whether to
     * periodically inject a frame.
     *
     * <pre>
     * Validity:  A F
     * --------------
     *   Luton26: N N
     *   Luton28: N N
     *   Jaguar1: N N
     *   Serval : Y N
     * </pre>
     */
    vtss_afi_id_t afi_id;
#endif /* defined(VTSS_FEATURE_AFI_SWC) */

} vtss_packet_tx_info_t;

/**
 * \brief Decode binary extraction/Rx header.
 *
 * This function is mainly useful for external CPUs that wish
 * to decode the side-band information they get on NPI ports
 * with extraction headers enabled.
 *
 * \param inst [IN]  Target instance reference.
 * \param meta [IN]  Meta info on received frame.
 * \param hdr  [IN]  Packet header (IFH)
 * \param info [OUT] Decoded extraction header.
 *
 * \return Return code. On some architectures, it is possible to detect whether the function
 *         is invoked with a valid binary extraction header, in which case the function can
 *         return an error if it's not valid. On others it can't, in which case the return
 *         code can only be different from VTSS_RC_OK if called with invalid arguments like
 *         NULL-pointers.
 **/
vtss_rc vtss_packet_rx_hdr_decode(const vtss_inst_t                  inst,
                                  const vtss_packet_rx_meta_t *const meta,
                                  const u8                           hdr[VTSS_PACKET_HDR_SIZE_BYTES],
                                        vtss_packet_rx_info_t *const info);

/**
 * \brief Compose binary injection/Tx header.
 *
 * This function is mainly useful for external CPUs that inject
 * frames into the switch core with an injection frame header.
 *
 * In a few cases, it may also be useful for an application
 * running on the internal CPU: If the frame is to be looped
 * internally on a loop port, which is set-up to accept an
 * injection header, the injection header may be encoded with
 * this function.
 *
 * The info structure is the input to the encoding, which
 * results in a binary injection header and a length.
 *
 * On many architectures, the resulting binary length is constant,
 * but on some, it may vary with the contents of info properties.
 * To overcome this, call this function twice. The first time,
 * use a NULL pointer for bin_hdr. On return, the length
 * parameter will contain the number of bytes required in
 * bin_hdr. The second time, provide a non-NULL pointer to
 * bin_hdr.
 * On successful exit, bin_hdr_len will always be updated to contain
 * the actual number of bytes required to hold the IFH.
 * If you don't want to call this function twice, you may allocate
 * an array of VTSS_PACKET_HDR_SIZE_BYTES (or VTSS_arch_PACKET_HDR_SIZE_BYTES)
 * bytes, which will be long enough to accommodate all combinations
 * of content of info. But notice that there is no guarantee that
 * the actual length of the returned data matches VTSS_arch_PACKET_HDR_SIZE_BYTES.
 *
 * For constellations that use the internal CPU in conjunction
 * with the Frame DMA, this function will be called by the FDMA
 * driver. Please refer to the FDMA API for details on how to
 * specify injection properties for use with the FDMA.
 *
 * \param inst        [IN]    Target instance reference.
 * \param info        [IN]    Tx header properties.
 * \param bin_hdr     [OUT]   NULL to get bin_hdr_len filled with required length in bytes. Non-NULL to get it filled in with the binary injection header.
 * \param bin_hdr_len [INOUT] If bin_hdr is NULL, the [IN]-part is not used. Instead, it will be filled with resulting
 *                            length of binary injection header in bytes. If bin_hdr is non-NULL, bin_hdr_len specifies the
 *                            number of bytes that the function may write from the beginning of bin_hdr. On exit, it will contain
 *                            the actual number of bytes.
 *
 * \return Return code.
 **/
vtss_rc vtss_packet_tx_hdr_encode(const vtss_inst_t                  inst,
                                  const vtss_packet_tx_info_t *const info,
                                        u8                    *const bin_hdr,
                                        u32                   *const bin_hdr_len);

/* Maximum Tx IFH length */
#if defined(VTSS_ARCH_JAGUAR_2)
// JR2-TBD: Update when correct value is known
#define VTSS_PACKET_TX_IFH_MAX     24  /**< Tx IFH byte length (Constant) */
#elif defined(VTSS_ARCH_JAGUAR_1)
#define VTSS_PACKET_TX_IFH_MAX     24  /**< Tx IFH byte length (Constant) */
#elif defined(VTSS_ARCH_SERVAL)
#define VTSS_PACKET_TX_IFH_MAX     16  /**< Tx IFH byte length (Constant) */
#elif defined(VTSS_ARCH_LUTON26)
#define VTSS_PACKET_TX_IFH_MAX     12  /**< Tx IFH byte length (Varies: 8/12 depending on PTP) */
#elif defined(VTSS_ARCH_LUTON28)
#define VTSS_PACKET_TX_IFH_MAX      8  /**< Tx IFH byte length (Constant) */
#endif

/**
 * \brief Compiled Tx Frame Header
 *
 * This is a pre-compiled representation of injection properties
 * similar to those given by vtss_packet_tx_hdr_encode(), but wrapped
 * in the following structure. The structure is useful for
 * preallocation, and can be used in combination with
 * vtss_packet_tx_frame().
 */
typedef struct {
    u32 length;                        /**< Length of compiled IFH (in bytes) */
    u32 ifh[VTSS_PACKET_TX_IFH_MAX/4]; /**< Compiled, binary IFH   */
} vtss_packet_tx_ifh_t;

/**
 * \brief Compile Tx Frame Header
 *
 * Compile a Tx frame header suitable for use with vtss_packet_tx_frame().
 *
 * \param inst [IN]  Target instance reference.
 * \param info [IN]  Tx header properties.
 * \param ifh  [OUT] Compiled Tx header.
 */
vtss_rc vtss_packet_tx_hdr_compile(const vtss_inst_t                  inst,
                                   const vtss_packet_tx_info_t *const info,
                                         vtss_packet_tx_ifh_t  *const ifh);

/**
 * \brief Send frame unmodified on port with pre-compiled IFH.
 *
 * \param inst [IN]     Target instance reference.
 * \param ifh [IN]      Compiled IFH
 * \param frame [IN]    Frame buffer excluding room for CRC.
 * \param length [IN]   Frame length excluding CRC.
 *
 * \return Return code.
 **/
vtss_rc vtss_packet_tx_frame(const vtss_inst_t                 inst,
                             const vtss_packet_tx_ifh_t *const ifh,
                             const u8                   *const frame,
                             const u32                         length);

/**
 * \brief Initialize a Tx info structure.
 *
 * Initialize the contents of a vtss_packet_tx_info_t
 * structure.
 *
 * \param inst [IN]  Target instance reference.
 * \param info [OUT] Pointer to structure that gets initialized to defaults.
 *
 * \return VTSS_RC_OK. VTSS_RC_ERROR only if info == NULL.
 */
vtss_rc vtss_packet_tx_info_init(const vtss_inst_t                  inst,
                                       vtss_packet_tx_info_t *const info);

/**
 * The packet DMA configuration.
 */
typedef struct {
    BOOL dma_enable[VTSS_QUEUE_ARRAY_SIZE]; /**< Enable the given queues for DMA */
} vtss_packet_dma_conf_t;

/**
 * \brief Retreive packet DMA configuration.
 *
 * \param inst [IN]  Target instance reference.
 * \param conf [OUT] Packet DMA configuration structure.
 *
 * \return VTSS_RC_OK.
 */
vtss_rc vtss_packet_dma_conf_get(const vtss_inst_t             inst,
                                 vtss_packet_dma_conf_t        *const conf);

/**
 * \brief Set packet DMA configuration.
 *
 * Zero or more packet queues may be enabled for DMA extraction.
 *
 * If a queue is enabled for FDMA, it can be extracted/injected on the
 * DMA interface of the system. Otherwise, the queues are enabled for
 * register-based extraction/injection.
 *
 * \param inst [IN]  Target instance reference.
 * \param conf [OUT] Packet DMA configuration structure.
 *
 * \return VTSS_RC_OK.
 */
vtss_rc vtss_packet_dma_conf_set(const vtss_inst_t            inst,
                                 const vtss_packet_dma_conf_t *const conf);

/**
 * \brief Retreive the register offset for extration/injection DMA.
 *
 * One or more queues should be enabled for DMA before using the register offset.
 *
 * The returned offset is for the \em status register, which is the
 * last register location in the DMA window. (Whole) Data word can be
 * written by selecting the approprate register offsets before this
 * offset, such that the status offset if the last word written/read.
 *
 * \param inst       [IN]  Target instance reference.
 * \param extraction [IN]
 * \param offset     [OUT] Offset (32-bit word offset) for the DMA
 * status register.
 */
vtss_rc vtss_packet_dma_offset(const vtss_inst_t inst,
                               BOOL extraction,
                               u32 *offset);

#endif /* VTSS_FEATURE_PACKET */
#endif /* _VTSS_PACKET_API_H_ */
