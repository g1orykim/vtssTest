/*

 Vitesse Switch API software.

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

*/

#ifndef _PACKET_API_H_
#define _PACKET_API_H_

/****************************************************************************/
/*  Public                                                                  */
/****************************************************************************/
#include "vtss_api.h"
#include "vtss_fdma_api.h"
#include "main.h"

#define SSP_PROT_EPID  2
#define ETYPE_IPV4     0x0800
#define ETYPE_IPV6     0x86DD
#define ETYPE_ARP      0x0806
#define ETYPE_RARP     0x8035

#define IP_PROTO_ICMP      1
#define IP_PROTO_IGMP      2
#define IP_PROTO_TCP       6
#define IP_PROTO_UDP      17

#define TCP_PROTO_SSH     22
#define TCP_PROTO_TELNET  23
#define TCP_PROTO_DNS     53
#define TCP_PROTO_HTTP    80
#define TCP_PROTO_HTTPS  443

#define UDP_PROTO_DNS     53
#define UDP_PROTO_SNMP   161

#define FCS_SIZE_BYTES 4 /* Frame Check Sequence - FCS */

/* The highest queue number has the highest priority */

#if VTSS_PACKET_RX_QUEUE_CNT < 8
// Not enough Rx queues on this architecture (e.g. Luton28)
// Adjust it to using only 4, even though we need 7.
#define PACKET_QU_ARCH_ADJUSTMENT_1 1
#define PACKET_QU_ARCH_ADJUSTMENT_2 2
#define PACKET_QU_ARCH_ADJUSTMENT_3 3
#define PACKET_QU_ARCH_ADJUSTMENT_4 4
#else
#define PACKET_QU_ARCH_ADJUSTMENT_1 0
#define PACKET_QU_ARCH_ADJUSTMENT_2 0
#define PACKET_QU_ARCH_ADJUSTMENT_3 0
#define PACKET_QU_ARCH_ADJUSTMENT_4 0
#endif

#define PACKET_XTR_QU_LOWEST  (VTSS_PACKET_RX_QUEUE_START + 0)
#define PACKET_XTR_QU_LOWER   (VTSS_PACKET_RX_QUEUE_START + 1 - PACKET_QU_ARCH_ADJUSTMENT_1)
#define PACKET_XTR_QU_LOW     (VTSS_PACKET_RX_QUEUE_START + 2 - PACKET_QU_ARCH_ADJUSTMENT_2)
#define PACKET_XTR_QU_NORMAL  (VTSS_PACKET_RX_QUEUE_START + 3 - PACKET_QU_ARCH_ADJUSTMENT_2)
#define PACKET_XTR_QU_MEDIUM  (VTSS_PACKET_RX_QUEUE_START + 4 - PACKET_QU_ARCH_ADJUSTMENT_2)
#define PACKET_XTR_QU_HIGH    (VTSS_PACKET_RX_QUEUE_START + 5 - PACKET_QU_ARCH_ADJUSTMENT_2)
#define PACKET_XTR_QU_HIGHER  (VTSS_PACKET_RX_QUEUE_START + 6 - PACKET_QU_ARCH_ADJUSTMENT_3)
#define PACKET_XTR_QU_HIGHEST (VTSS_PACKET_RX_QUEUE_START + 7 - PACKET_QU_ARCH_ADJUSTMENT_4)

// Extraction Queue Allocation
#define PACKET_XTR_QU_LRN_ALL   PACKET_XTR_QU_LOWEST  /* Only Learn-All frames end up in this queue (JR Stacking + JR-48 standalone). */
#define PACKET_XTR_QU_SFLOW     PACKET_XTR_QU_LOWER   /* Only sFlow-marked frames must be forwarded on this queue. If not, other modules will not be able to receive the frame. */
#define PACKET_XTR_QU_ACL_COPY  PACKET_XTR_QU_LOW     /* For ACEs with CPU copy                          */
#define PACKET_XTR_QU_LEARN     PACKET_XTR_QU_LOW     /* For the sake of MAC-based Authentication        */
#define PACKET_XTR_QU_BC        PACKET_XTR_QU_LOW     /* For Broadcast MAC address frames                */
#define PACKET_XTR_QU_MAC       PACKET_XTR_QU_LOW     /* For other MAC addresses that require CPU copies */
#define PACKET_XTR_QU_L2CP      PACKET_XTR_QU_NORMAL  /* For L2CP frames                                 */
#define PACKET_XTR_QU_OAM       PACKET_XTR_QU_NORMAL  /* For OAM frames                                  */
#define PACKET_XTR_QU_L3_OTHER  PACKET_XTR_QU_NORMAL  /* For L3 frames with errors or TTL expiration     */
#define PACKET_XTR_QU_MGMT_MAC  PACKET_XTR_QU_MEDIUM  /* For the switch's own MAC address                */
#define PACKET_XTR_QU_IGMP      PACKET_XTR_QU_HIGHER  /* For IP multicast frames                         */
#define PACKET_XTR_QU_BPDU      PACKET_XTR_QU_HIGHER  /* For BPDUs                                       */
#define PACKET_XTR_QU_STACK     PACKET_XTR_QU_HIGHEST /* For Message module frames                       */
#define PACKET_XTR_QU_SPROUT    PACKET_XTR_QU_HIGHEST /* For SPROUT frames on archs not using super-prio */

#if defined(VTSS_ARCH_JAGUAR_1_DUAL) || (defined(VTSS_ARCH_JAGUAR_1) && VTSS_SWITCH_STACKABLE)
// Due to Bugzilla#10644, we need to use a higher prioritized CPU queue for frames that hit an ACL rule
// that causes the frame to be redirected to the CPU (not just copied).
#define PACKET_XTR_QU_ACL_REDIR PACKET_XTR_QU_HIGH      /* For ACEs with CPU redirection */
#else
// On other architectures, use the same queue as frames that hit an ACL rule that caused
// the frame to be copied (not redirected) to the CPU
#define PACKET_XTR_QU_ACL_REDIR PACKET_XTR_QU_ACL_COPY  /* For ACEs with CPU redirection */
#endif

// Extraction rate for each of 4 policers in pps.
// The total rate to the CPU must be less than approximaly 200 Mbps.
// With 4000 pps and maximum size frames on all 4 policers,
// we will get 4*1518*8*4000 bps = 194 Mbps
#define PACKET_XTR_POLICER_RATE 4000

/* Packet Module error codes (vtss_rc) */
typedef enum {
    PACKET_ERROR_GEN = MODULE_ERROR_START(VTSS_MODULE_ID_PACKET),  /* Generic error code */
    PACKET_ERROR_PARM,                                             /* Illegal parameter  */
    PACKET_ASSERT_FAILURE,                                         /* Assertion failed   */
} packet_error_t;

/****************************************************************************/
// PACKET_RX_FILTER_PRIO_xxx definitions
/****************************************************************************/
#define PACKET_RX_FILTER_PRIO_SUPER            10
#define PACKET_RX_FILTER_PRIO_HIGH            100
#define PACKET_RX_FILTER_PRIO_NORMAL         1000
#define PACKET_RX_FILTER_PRIO_BELOW_NORMAL  10000
#define PACKET_RX_FILTER_PRIO_LOW          100000

/****************************************************************************/
// Fields to match on are:
//   * IFH.source_port
//   * IFH.frame_type (for ACLs)
//   * IFH.vid
//   * IFH.learn
//   * DMAC
//   * EtherType
//   * IP.Proto (for EtherType == 0x0800)
//   * SSPID (for EtherType == 0x8880 and EPID == 0x0002)
//   * UDP/TCP Port number
// This results in the following match flags for the API, that may be ORed
// together to form a complex match. Some of the flags are mutually exclusive
// and some are implied by the use of others.
// Special note about PACKET_RX_FILTER_MATCH_ANY:
//   If this is used, no other flags may be used (obviously). A subscriber
//   using this flag will get all packets (can be used by e.g. Ethereal).
// Special note about PACKET_RX_FILTER_MATCH_DEFAULT:
//   A subscriber using this flag will get all packets not matching any other
//   subscriptions. This flag may only be used alone.
/****************************************************************************/
#define PACKET_RX_FILTER_MATCH_ANY          0x00000000 /* packet_rx_filter_t member(s): None                               */
#define PACKET_RX_FILTER_MATCH_SRC_PORT     0x00000001 /* packet_rx_filter_t member(s): src_port_mask                      */
#define PACKET_RX_FILTER_MATCH_ACL          0x00000002 /* packet_rx_filter_t member(s): None                               */
#define PACKET_RX_FILTER_MATCH_VID          0x00000004 /* packet_rx_filter_t member(s): vid, vid_mask                      */
#define PACKET_RX_FILTER_MATCH_FREE_TO_USE  0x00000008 /* Unused                                                           */
#define PACKET_RX_FILTER_MATCH_DMAC         0x00000010 /* packet_rx_filter_t member(s): dmac, dmac_mask                    */
#define PACKET_RX_FILTER_MATCH_SMAC         0x00000020 /* packet_rx_filter_t member(s): smac, smac_mask                    */
#define PACKET_RX_FILTER_MATCH_ETYPE        0x00000040 /* packet_rx_filter_t member(s): etype, etype_mask                  */
#define PACKET_RX_FILTER_MATCH_IPV4_PROTO   0x00000080 /* packet_rx_filter_t member(s): ipv4_proto, ipv4_proto_mask        */
#define PACKET_RX_FILTER_MATCH_SSPID        0x00000100 /* packet_rx_filter_t member(s): sspid, sspid_mask                  */
#define PACKET_RX_FILTER_MATCH_SFLOW        0x00000200 /* packet_rx_filter_t member(s): None                               */
#define PACKET_RX_FILTER_MATCH_IP_ANY       0x00000400 /* packet_rx_filter_t member(s): None. Matches IPv4, IPv6, ARP      */
#define PACKET_RX_FILTER_MATCH_UDP_SRC_PORT 0x00000800 /* packet_rx_filter_t member(s): udp_src_port_min, udp_src_port_max */
#define PACKET_RX_FILTER_MATCH_UDP_DST_PORT 0x00001000 /* packet_rx_filter_t member(s): udp_dst_port_min, udp_dst_port_max */
#define PACKET_RX_FILTER_MATCH_TCP_SRC_PORT 0x00002000 /* packet_rx_filter_t member(s): tcp_src_port_min, tcp_src_port_max */
#define PACKET_RX_FILTER_MATCH_TCP_DST_PORT 0x00004000 /* packet_rx_filter_t member(s): tcp_dst_port_min, tcp_dst_port_max */
#define PACKET_RX_FILTER_MATCH_DEFAULT      0x80000000 /* packet_rx_filter_t member(s): None                               */

// The following structure contains masks and what to match against.
typedef struct {
    //
    // GENERAL CONFIGURATION
    //

    // Module ID. Only used to find the name of the subscription module when
    // printing statistics.
    // Validity:
    //   Luton26: Y
    //   Luton28: Y
    //   Jaguar1: Y
    //   Serval : Y
    vtss_module_id_t modid;

    // Match flags. Bitwise OR the PACKET_RX_FILTER_MATCH_xxx to form the final
    // filter.
    // Validity:
    //   Luton26: Y
    //   Luton28: Y
    //   Jaguar1: Y
    //   Serval : Y
    u32 match;

    // Priority. The lower this number, the higher priority. Use
    // the PACKET_RX_FILTER_PRIO_xxx definitions.
    // Do not set this to 0. This will cause the packet module to issue an
    // error. You really need to think about a real priority.
    // Validity:
    //   Luton26: Y
    //   Luton28: Y
    //   Jaguar1: Y
    //   Serval : Y
    u32 prio;

    // User-defined info. Supplied when calling back.
    //   Luton26: Y
    //   Luton28: Y
    //   Jaguar1: Y
    //   Serval : Y
    void *contxt;

    // Callback function called when a match occurs.
    // Arguments:
    //   #contxt  : User-defined (the value of this structure's .contxt member).
    //   #frm     : Points to the first byte of the DMAC of the frame.
    //   #rx_info : Various frame properties, a.o. length, source port, VID.
    // The function MUST return
    //   TRUE : If the frame is consumed for good and shouldn't be dispatched
    //          to other subscribers.
    //   FALSE: If the frame is allowed to be dispatched to other subscribers.
    // NOTE 1): The callback function is NOT allowed in anyway to modify the packet.
    // NOTE 2): The callback function *is* allowed to call packet_rx_filter_XXX().
    //   Luton26: Y
    //   Luton28: Y
    //   Jaguar1: Y
    //   Serval : Y
    BOOL (*cb)(void *contxt, const u8 *const frm, const vtss_packet_rx_info_t *const rx_info);

    // The advanced version of the callback function called back when a match occurs.
    // When this is non-NULL, it is the responsibility of the called-back function to
    // release the #fdma_buffers to the FDMA driver code when handling of the frame is
    // over. This feature is particularly useful when relaying frames, since the incoming
    // frame buffers may be re-used for transmission and released back to the FDMA in
    // the tx_done callback. The function for releasing the buffers is called
    // packet_rx_release_fdma_buffers().
    // The return value is not used by the Packet Module. The frame is considered
    // consumed when a subscriber using the the #cb_adv callback function is
    // used.
    //   Luton26: Y
    //   Luton28: Y
    //   Jaguar1: Y
    //   Serval : Y
    BOOL (*cb_adv)(void *contxt, const u8 *const frm, const vtss_packet_rx_info_t *const rx_info, const void *const fdma_buffers);

    //
    // FILTER CONFIGURATION
    //
    // Note that most members have a corresponding mask. If this mask is
    // all-zeros, all bits must match (inverse polarity). The rule is as
    // follows:
    // There's a match if (packet->fld & ~.fld_mask) == .fld,
    // where fld is either of the following that have a corresponding mask.
    // The reason for the inverse polarity is that most modules will ask
    // for a particular match, not a wild card match, so most modules will
    // have the compiler insert zeros for uninitialized fields.
    // Furthermore, since we have the .match member, future extensions to
    // this structure will not affect present code.

    // If PACKET_RX_FILTER_MATCH_SRC_PORT is set, a match against .src_port
    // is made. .src_port is actually a bitmask with a bit set for each
    // logical port it wishes to match. You may use the VTSS_PORT_BF_xxx macros
    // defined in main.h for this.
    //   Luton26: Y
    //   Luton28: Y
    //   Jaguar1: Y
    //   Serval : Y
    u8 src_port_mask[VTSS_PORT_BF_SIZE];

    // If PACKET_RX_FILTER_MATCH_VID is set, a match against .vid is made.
    // At most the 12 LSBits are used in the match.
    //   Luton26: Y
    //   Luton28: Y
    //   Jaguar1: Y
    //   Serval : Y
    u16 vid;
    u16 vid_mask;

    // If PACKET_RX_FILTER_MATCH_DMAC is set, a match like the following is made:
    // if ((packet->dmac & ~.dmac_mask) == .dmac) hit();
    //   Luton26: Y
    //   Luton28: Y
    //   Jaguar1: Y
    //   Serval : Y
    u8 dmac[6];
    u8 dmac_mask[6];

    // If PACKET_RX_FILTER_MATCH_SMAC is set, a match like the following is made:
    // if ((packet->smac & ~.smac_mask) == .smac) hit();
    //   Luton26: Y
    //   Luton28: Y
    //   Jaguar1: Y
    //   Serval : Y
    u8 smac[6];
    u8 smac_mask[6];

    // If PACKET_RX_FILTER_MATCH_ETYPE is set, a match against .etype is made.
    //   Luton26: Y
    //   Luton28: Y
    //   Jaguar1: Y
    //   Serval : Y
    u16 etype;
    u16 etype_mask;

    // If PACKET_RX_FILTER_MATCH_IPV4_PROTO is set (this implicitly implies
    // PACKET_RX_FILTER_MATCH_ETYPE with .etype==0x0800) then a match against this prototype is made:
    //   Luton26: Y
    //   Luton28: Y
    //   Jaguar1: Y
    //   Serval : Y
    u8 ipv4_proto;
    u8 ipv4_proto_mask;

    // If PACKET_RX_FILTER_MATCH_UDP_SRC_PORT is set then a match against the following range
    // of UDP source port numbers is made (both ends included). Set both min and max to the
    // same value to match on only one port number.
    // Note: Setting PACKET_RX_FILTER_MATCH_UDP_SRC_PORT causes the packet module to automatically insert
    //   PACKET_RX_FILTER_MATCH_ETYPE      with .etype      == 0x0800 (ETYPE_IPV4) and
    //   PACKET_RX_FILTER_MATCH_IPV4_PROTO with .ipv4_proto == 17 (IP_PROTO_UDP)
    // Supported platforms:
    //   Luton26: Y
    //   Luton28: Y
    //   Jaguar1: Y
    //   Serval : Y
    u16 udp_src_port_min;
    u16 udp_src_port_max;

    // If PACKET_RX_FILTER_MATCH_UDP_DST_PORT is set then a match against the following range
    // of UDP destination port numbers is made (both ends included). Set both min and max to the
    // same value to match on only one port number.
    // Note: Setting PACKET_RX_FILTER_MATCH_UDP_DST_PORT causes the packet module to automatically insert
    //   PACKET_RX_FILTER_MATCH_ETYPE      with .etype      == 0x0800 (ETYPE_IPV4) and
    //   PACKET_RX_FILTER_MATCH_IPV4_PROTO with .ipv4_proto == 17 (IP_PROTO_UDP)
    // Supported platforms:
    //   Luton26: Y
    //   Luton28: Y
    //   Jaguar1: Y
    //   Serval : Y
    u16 udp_dst_port_min;
    u16 udp_dst_port_max;

    // If PACKET_RX_FILTER_MATCH_TCP_SRC_PORT is set then a match against the following range
    // of TCP source port numbers is made (both ends included). Set both min and max to the
    // same value to match on only one port number.
    // Note: Setting PACKET_RX_FILTER_MATCH_TCP_SRC_PORT causes the packet module to automatically insert
    //   PACKET_RX_FILTER_MATCH_ETYPE      with .etype      == 0x0800 (ETYPE_IPV4) and
    //   PACKET_RX_FILTER_MATCH_IPV4_PROTO with .ipv4_proto == 6 (IP_PROTO_TCP)
    // Supported platforms:
    //   Luton26: Y
    //   Luton28: Y
    //   Jaguar1: Y
    //   Serval : Y
    u16 tcp_src_port_min;
    u16 tcp_src_port_max;

    // If PACKET_RX_FILTER_MATCH_TCP_DST_PORT is set then a match against the following range
    // of TCP destination port numbers is made (both ends included). Set both min and max to the
    // same value to match on only one port number.
    // Note: Setting PACKET_RX_FILTER_MATCH_TCP_DST_PORT causes the packet module to automatically insert
    //   PACKET_RX_FILTER_MATCH_ETYPE      with .etype      == 0x0800 (ETYPE_IPV4) and
    //   PACKET_RX_FILTER_MATCH_IPV4_PROTO with .ipv4_proto == 6 (IP_PROTO_TCP)
    // Supported platforms:
    //   Luton26: Y
    //   Luton28: Y
    //   Jaguar1: Y
    //   Serval : Y
    u16 tcp_dst_port_min;
    u16 tcp_dst_port_max;

    // If PACKET_RX_FILTER_MATCH_SSPID is set (this implicitly implies
    // PACKET_RX_FILTER_MATCH_ETYPE with .etype=0x8880 and EPID == 0x0002)
    //   Luton26: N
    //   Luton28: Y
    //   Jaguar1: Y
    //   Serval : Y
    u16 sspid;
    u16 sspid_mask;

} packet_rx_filter_t;

// Register a filter. The filter may be allocated on the
// stack, because packet_rx_filter_register() makes a copy
// of it, so that the caller cannot change the filter
// behind this file's back. The function returns an ID
// that may be used later to unregister or change
// the subscription.
vtss_rc packet_rx_filter_register(const packet_rx_filter_t *filter, void **filter_id);

// Unregister an existing filter.
vtss_rc packet_rx_filter_unregister(void *filter_id);

// Change an existing filter
vtss_rc packet_rx_filter_change(const packet_rx_filter_t *filter, void **filter_id);

/******************************************************************************/
// packet_rx_release_fdma_buffers()
// Gives back buffers to the FDMA. Only use this function if you use the
// advanced callback function (#cb_adv) in the RX filter. The argument is
// the fdma_buffers passed to #cb_adv. See #cb_adv for further description.
/******************************************************************************/
void packet_rx_release_fdma_buffers(void *fdma_buffers);

/*
 * Status of Tx.
 */
typedef struct {
    /**
     * TRUE if frame was successfully transmitted. FALSE otherwise.
     * In the case of AFI frames, FALSE typically indcates that the
     * FDMA Driver ran out of resources. Use '/Debug Trace RingBuffer Print'
     * to see more info (error info saved in ring buffer, because
     * scheduler is disabled when the trace message is printed). The ringbuffer
     * supports this, whereas console writings don't.
     */
    BOOL tx;

    /**
     * Time of transmission. Only valid if .tx == TRUE and not an AFI frame.
     */
    VTSS_OS_TIMESTAMP_TYPE sw_tstamp;

    /**
     * Pointer to the frame.
     * Can be used directly in calls to packet_tx_free().
     */
    u8 *frm_ptr[2];

    /**
     * Number of times this frame was injected (mainly useful for FDMA-based
     * AFI frames).
     * Will be 0 if frame counting was not enabled for the FDMA-based AFI injection.
     * Will be 1 for normal injection.
     * Will be 0 if #tx == FALSE.
     *
     * For FDMA-based AFI frames, you can obtain interim counters with a call to
     * packet_tx_afi_frm_cnt().
     */
    u64 frm_cnt;

    /**
     * Last sequence number put in an FDMA-based AFI frame.
     * This is mainly for debugging purposes.
     */
    u32 afi_last_sequence_number;

} packet_tx_done_props_t;

/*
 * Inject completion callback
 */
typedef void (*packet_tx_done_cb_t)(void                   *context, /* Callback defined */
                                    packet_tx_done_props_t *props);  /* Tx Done status   */

/*
 * Signature of function to callback just before the frame is handed over to the FDMA H/W.
 * May be useful for modifying a packet with a timestamp just before the packet
 * is handed over to the FDMA.
 *
 * The called back function is allowed to modify the frame contents.
 *
 * NOTE: The function called back cannot wait for synchronization primitives of
 *       any kind, and it should be very swift, since interrupts may be disabled
 *       when calling back.
 */
typedef void (*packet_tx_pre_cb_t)(void   *tx_pre_contxt, /* Defined by user module                                      */
                                   u8     *frm,           /* Pointer to the frame to be transmitted (points to the DMAC) */
                                   size_t len);           /* Frame length (excluding IFH, CMD, and FCS).                 */

/**
 * \brief Packet Tx Filter.
 *
 * Describes ingress and egress properties
 * to be used when the packet_tx() function
 * must assess whether or not to insert a VLAN tag
 * in the frame prior to transmission on a given
 * front port. In some cases, the packet_tx() must
 * even discard the frame (if for instance the ingress
 * aggregation is equal to the egress aggregation).
 */
typedef struct {
    /**
     * Set this member to TRUE to enable filtering
     * and destination-port tag info lookup.
     * The remaining members of this structure are not
     * are not used if this is set to FALSE.
     *
     * The lookup is a pretty expensive operation, so use
     * this capability with care.
     */
    BOOL enable;

    /**
     * The port that this frame originally arrived on.
     * This is used to discard the frame in case the ingress
     * port is not in forwarding mode, or if the port is not
     * a member of the VLAN ID specified with
     * packet_tx_props_t::vid (!= VTSS_VID_NULL),
     * or if the ingress port equals the egress port specified
     * with packet_tx_props_t::dst_port, or if the ingress
     * and egress ports are members of the same LLAG.
     *
     * Ingress filtering may be turned off by setting #src_port
     * to VTSS_PORT_NO_NONE.
     */
    vtss_port_no_t src_port;

#if defined(VTSS_FEATURE_AGGR_GLAG)
    /**
     * The GLAG that this frame originally arrived on.
     * This is used to determine whether to discard the
     * frame in case the egress port (packet_tx_props_t::dst_port)
     * is member of the same GLAG.
     *
     * Set to VTSS_GLAG_NO_NONE to disable this check.
     */
    vtss_glag_no_t glag_no;
#endif /* VTSS_FEATURE_AGGR_GLAG */

} packet_tx_filter_t;

/**
 * \brief Packet Tx Info
 */
typedef struct {
    /**
     * Set your module's ID here, or leave it as is if your
     * module doesn't have a module ID.
     * The module ID is used for providing per-module
     * statistics, and is mainly for debugging.
     */
    vtss_module_id_t modid;

    /**
     * This member contains pointer(s) to actual frame data.
     * The packet module supports a frame to be split into two
     * fragments, indexed by frm[0] and frm[1], respectively.
     * The first fragment must be allocated with packet_tx_alloc(),
     * whereas a possible second may be statically allocated,
     * dynamically allocated with VTSS_MALLOC() or allocated with
     * packet_tx_alloc().
     * If frm[1] is non-NULL, the tx_done_cb() must be non-NULL,
     * and the user is responsible for deallocating the
     * fragments herself.
     *
     * frm[0][0] is expected to point to the MSByte of the DMAC.
     *
     * Validity:
     *   Luton26: Y
     *   Luton28: Y
     *   Jaguar1: Y
     *   Serval : Y
     */
    u8 *frm[2];

    /**
     * This member contains lengths of the corresponding fragments
     * specified with the #frm member array.
     *
     * If #frm[1] == NULL:
     *   Only #len[0] is used, and must be >= 14 bytes.
     *   It specifies the length of the frame excluding IFH and FCS.
     * If #frm[1] != NULL:
     *   #len[0] must be a multiple of 4 and >= 16 bytes.
     *   It excludes the length of the IFH.
     *   #len[1] must be > 0 bytes and excludes the FCS.
     *
     * Validity:
     *   Luton26: Y
     *   Luton28: Y
     *   Jaguar1: Y
     *   Serval : Y
     */
    size_t len[2];

    /**
     * Set this to the address of a function that will be called back once
     * the frame has been (attempted) transmitted. The called back function
     * will indicate success or failure  of the transmission together with the
     * value of the #tx_done_cb_context member.
     *
     * If you set this member to NULL, the frame (previously allocated by a
     * call to packet_tx_alloc()) will be auto-deallocated by the packet
     * module, using packet_tx_free().
     * If you used the packet_tx_alloc_extra() function to allocate the frame,
     * then you must specify a Tx Done callback function and call the
     * packet_tx_free_extra() yourself.
     *
     * Validity:
     *   Luton26: Y
     *   Luton28: Y
     *   Jaguar1: Y
     *   Serval : Y
     */
    packet_tx_done_cb_t tx_done_cb;

    /**
     * If the user has elected to be called back once the frame is transmitted
     * (through the #tx_done_cb member), the called back function will contain
     * the value of this parameter as the #context argument.
     *
     * Validity:
     *   Luton26: Y
     *   Luton28: Y
     *   Jaguar1: Y
     *   Serval : Y
     */
    void *tx_done_cb_context;

    /**
     * Set this member to the address of a function that will be called back
     * just before the frame is handed over to the FDMA.
     * The called back function may modify the frame data, but no other fields
     * may be changed (i.e. the hidden IFH must not be changed (since it
     * won't be cache invalidated)).
     * The called back function will be called with a pointer to frm[0] and
     * the value of the user-specified #tx_pre_cb_context parameter.
     *
     * #filter.enable must be FALSE.
     * On Luton26 and Jaguar, #switch_frm must be FALSE.
     * On Luton28, #contains_stack_hdr must be FALSE.
     * The reason is that the FDMA module modifies the packet layout before
     * transmission, so frm[0] no longer points to the beginning of the frame.
     *
     * This functionality may be used in conjunction with one-step PTP
     * timestamping in cases where the H/W doesn't support it, or the higher
     * layers of S/W doesn't support it.
     *
     * Validity:
     *   Luton26: Y if #switch_frm is FALSE
     *   Luton28: Y if #contains_stack_hdr is FALSE (for VTSS_OPT_FDMA_VER >= 2)
     *   Jaguar1: Y if #switch_frm is FALSE
     *   Serval : Y if #switch_frm is FALSE
     */
    packet_tx_pre_cb_t tx_pre_cb;

    /**
     * If the user has elected to be called back just before the frame is
     * handed over to the FDMA (through the #tx_pre_cb member), the called
     * back function will contain the value of this parameter as the
     * #context argument.
     *
     * Validity:
     *   Luton26: Y
     *   Luton28: Y
     *   Jaguar1: Y
     *   Serval : Y
     */
    void *tx_pre_cb_context;

    /**
     * Under certain circumstances, a user module may wish to send a frame
     * directed to a front port, but with a VLAN tag that matches the front
     * port's VLAN properties (tag type, membership, untagged VID, etc.).
     * This #filter property allows for that.
     *
     * #filter.enable must be set to TRUE to enable this feature.
     * If enabled, #switch_frm must be FALSE and #dst_port_mask must be 0, that is,
     * only one single frame can be transmitted at a time.
     * Furthermore, #dst_port must not be a stack port, and #contains_stack_hdr must be FALSE.
     *
     * The lookup of egress properties is based on the port given by this structure's
     * #dst_port and using the egress VID given by this structure's #vid member.
     *
     * For further use of the filter, please refer to its definition above.
     *
     * NOTICE: If the filter gives rise to discarding of the frame, packet_tx() will
     *         return VTSS_RC_INV_STATE. The caller must remember to deallocate
     *         resources if this happens, because the tx_done function won't be called,
     *         and therefore auto-deallocation also won't be done.
     *
     * NOTICE: If packet_tx() returns VTSS_RC_OK, there is a chance that the frame
     *         has been modified prior to transmission (insertion of a VLAN tag).
     *         This means that you cannot use the same copy of the frame in a tight
     *         loop that transmits to multiple ports. You *must* allocate a new copy
     *         for each port you transmit to.
     *
     * Validity:
     *   Luton26: Y
     *   Luton28: Y
     *   Jaguar1: Y
     *   Serval : Y
     */
    packet_tx_filter_t filter;

    /**
     * Internal flags. Don't use.
     */
    u32 internal_flags;

} packet_tx_info_t;

/**
 * \brief Packet Tx Properties.
 *
 * Properties on how to transmit a frame using packet_tx().
 * This must be initialized by a call to packet_tx_props_init().
 * The structure can be allocated on the stack.
 */
typedef struct {

    /**
     * Tx Properties as defined by the underlying VTSS API
     */
    vtss_packet_tx_info_t tx_info;

    /*
     * Tx properties needed by the FDMA
     */
    vtss_fdma_tx_info_t fdma_info;

    /**
     * Tx properties needed by the packet module
     */
    packet_tx_info_t packet_info;

} packet_tx_props_t;

/******************************************************************************/
// packet_tx_props_init()
// Initialize a packet_tx_props_t structure.
/******************************************************************************/
void packet_tx_props_init(packet_tx_props_t *tx_props);

/**
 * \brief Transmit frame.
 *
 * Transmit a frame using the propeties set in #tx_props.
 * Refer to definition of #packet_tx_props_t for a description of
 * transmission options.
 * #tx_props may be allocated on the stack, but it must be initialized by a
 * call to packet_tx_props_init().
 *
 * \return
 *    VTSS_RC_OK on success\n
 *    VTSS_RC_ERROR on invalid combination of #tx_props or if FIFO is full.
 *    VTSS_RC_INV_STATE if #packet_tx_props_t::filter gave rise to a discard of this frame.
 */
vtss_rc packet_tx(packet_tx_props_t *tx_props);

/******************************************************************************/
// Tx buffer alloc & free.
// Args:
//   #size : Size excluding IFH, possible stack header, and FCS
// Returns:
//   Pointer to location that the DMAC should be stored or NULL on out-of-memory.
/******************************************************************************/
u8 *packet_tx_alloc(size_t size);
void packet_tx_free(u8 *buffer);

/******************************************************************************/
// packet_tx_alloc_extra()
// The difference between this function and the packet_tx_alloc() function is
// that the user is able to reserve a number of 32-bit words at the beginning
// of the packet, which is useful when some state must be saved between the call
// to the packet_tx() function and the user-defined callback function.
// Args:
//   #size              : Size exluding IFH, CMD, and FCS
//   #extra_size_dwords : Number of 32-bit words to reserve room for.
//   #extra_ptr         : Pointer that after the call will contain the pointer to the additional space.
// Returns:
//   Pointer to location that the DMAC should be stored or NULL on out-of-space.
// Use packet_tx_free_extra() when freeing the packet rather than packet_tx_free().
/******************************************************************************/
u8 *packet_tx_alloc_extra(size_t size, size_t extra_size_dwords, u8 **extra_ptr);

/******************************************************************************/
// packet_tx_free_extra()
// This function is the counter-part to the packet_tx_alloc_extra() function.
// It must be called with the value returned in packet_tx_alloc_extra()'s
// extra_ptr argument.
/******************************************************************************/
void packet_tx_free_extra(u8 *extra_ptr);

#if defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC)
/**
 * \brief Cancel a periodic frame transmission.
 *
 * #frm is a pointer to the AFI frame originally used
 * in a call to packet_tx().
 * The function cancels the AFI frame asynchronously, that is,
 * it returns immediately, and only after a while will the frame
 * be cancelled. If a tx_done callback function was specified
 * in the call to packet_tx(), it will be called once the frame has been
 * cancelled.
 */
vtss_rc packet_tx_afi_cancel(u8 *frm);
#endif /* defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC) */

#if defined(VTSS_FEATURE_AFI_FDMA)
/**
 * \brief Get interim frame count for a frame currently being injected
 * using the FDMA-based AFI.
 *
 * @frm is a pointer to the frame originally allocated with a call
 * to packet_tx_alloc().
 *
 * Upon exit, @frm_cnt will contain the current number of frames injected.
 * This will be 0 if the frame is not AFI-injected with frame counting
 * enabled.
 *
 * If this function is called too soon after the call to packet_tx(),
 * it might be that the internal structures aren't yet updated, which
 * in turn means that the function may return VTSS_RC_ERROR.
 * Try again later in that case.
 *
 * The exact number of frames transmitted during an AFI session is
 * returned in the "Tx Done" callback function's packet_tx_done_props_t
 * structure's frm_cnt member once the frame is cancelled.
 */
vtss_rc packet_tx_afi_frm_cnt(u8 *frm, u64 *frm_cnt);
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

/******************************************************************************/
// packet_uninit()
// Gracefully shut-down the packet module.
// This call only has effect if VTSS_SW_OPTION_WARM_START is defined.
/******************************************************************************/
void packet_uninit(void);

typedef int (*packet_dbg_printf_t)(const char *fmt, ...) __attribute__ ((format (__printf__, 1, 2)));

/****************************************************************************/
// packet_dbg()
// Entry point to Packet Module Debug features. Should be called from
// cli only.
/****************************************************************************/
void packet_dbg(packet_dbg_printf_t dbg_printf, ulong parms_cnt, ulong *parms);

/****************************************************************************/
// Module init
/****************************************************************************/
vtss_rc packet_init(vtss_init_data_t *data);

#endif /* _PACKET_API_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
