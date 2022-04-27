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
 * \brief MPLS API
 * \details This header file describes the MPLS functions
 */

#ifndef _VTSS_MPLS_API_NEW_H_
#define	_VTSS_MPLS_API_NEW_H_

#if defined(VTSS_FEATURE_MPLS)

#include <vtss_options.h>
#include <vtss_types.h>

/* MPLS API OVERVIEW
 * =================
 * The MPLS API is made up of the following main elements:
 *
 *   - Segments
 *   - Cross-connects
 *   - Layer-2 configuration
 *
 *
 * Segment & Layer-2
 * -----------------
 * The Segment is the fundamental MPLS entity, designating either an incoming
 * unidirectional LSP (in-segment) or an outgoing unidirectional LSP (out-
 * segment).
 *
 * A segment thus holds an MPLS label, TC information as well as other details
 * necessary to configure the LSP.
 *
 * Segments may be nested: One segment may be Server to other segments, Clients.
 * Each Client segment may itself be Server to other Clients, and so on; only
 * limited by the HW capabilities.
 *
 * In order to move MPLS packets in and out of a port, a Layer-2 configuration
 * must be created; it holds the relevant L2 info (port, peer MAC, tag type and
 * VID, own MAC) enabling RX and TX.
 *
 * A segment must therefore be attached to a L2 entry in order to be able to
 * process MPLS. Obviously, only the ultimate server segment ("outermost
 * segment") can have L2 attached.
 *
 * Finally, each segment must be attached to exactly one Cross-connect; see
 * below.
 *
 *
 * Cross-connect (XC)
 * ------------------
 * Every segment must be attached to a Cross Connect (XC). The XC serves these
 * purposes:
 *
 *   - Connect an in- and out-segment, enabling a Label Swap operation (LSR)
 *   - Configure egress MPLS multicast (MC) operation
 *   - For initiating/terminating LSPs/PWs, identify the type.
 *
 * MC is enabled by adding out-segments to a per-XC MC chain. Each MC chain
 * entry is a self-contained unidirectional LSP setup, complete with servers,
 * clients and L2.
 *
 */


/* MPLS API USAGE
 * ==============
 *
 * HW allocation
 * -------------
 * A segment will attempt to acquire HW resources when it is sufficiently
 * configured. What constitutes "sufficient" depends on many different factors,
 * detailed later. This HW allocation may fail if the underlying resources are
 * unavailable/consumed; in that case the segment will stay in this state and
 * there will be no allocation retrying except when explicitly initiated by a
 * call to @vtss_mpls_segment_set() or one of the attachment functions (attach
 * to L2, segment, XC).
 *
 * Segment states
 * --------------
 * A segment can either be UNCONFigured, CONFigured or UP.
 *
 * UNCONF means that the segment lacks configuration in order to operate.
 * 
 * CONF means that the segment has sufficient configuration to operate, but
 * that:
 *   - either the underlying server segment isn't UP,
 *   - or the segment hasn't been able to allocate hardware resources
 * 
 * UP indicates that:
 *   - the configuration is sufficient
 *   - the underlying server (if any) is UP
 *   - all necessary HW resources (if any) have been allocated
 *
 * Please note that UP does not indicate whether the ultimately underlying L2
 * port is up or not. The MPLS API considers it a resource, not a server layer.
 *
 */


// Don't-care values for label and TC values

#define VTSS_MPLS_LABEL_VALUE_DONTCARE  0xffffffff
#define VTSS_MPLS_TC_VALUE_DONTCARE     0xff



// Various indices and ranges.

#define VTSS_MPLS_IDX_UNDEFINED         (-1)
#define VTSS_MPLS_IDX_IS_UNDEF(idx)     ((idx) == VTSS_MPLS_IDX_UNDEFINED)
#define VTSS_MPLS_IDX_IS_DEF(idx)       ((idx) != VTSS_MPLS_IDX_UNDEFINED)
#define VTSS_MPLS_IDX_UNDEF(idx)        do { (idx) = VTSS_MPLS_IDX_UNDEFINED; } while (0)

typedef u32 vtss_mpls_label_value_t;    /**< [0..2^20[ or VTSS_MPLS_LABEL_VALUE_DONTCARE for don't care/undefined */
typedef u8  vtss_mpls_tc_t;             /**< [0..7] or VTSS_MPLS_TC_VALUE_DONTCARE for don't care/undefined */
typedef u8  vtss_mpls_cos_t;            /**< [0..7] */
typedef u8  vtss_mpls_qos_t;            /**< [0..7] */
typedef i16 vtss_mpls_segment_idx_t;    /**< Index into table of vtss_mpls_segment_t */
typedef i16 vtss_mpls_xc_idx_t;         /**< Index into table of vtss_mpls_xc_t */
typedef i16 vtss_mpls_policer_idx_t;    /**< Policer index */
typedef i16 vtss_mpls_l2_idx_t;         /**< Index into table of vtss_mpls_l2_t */



/** \brief Frame tagging. Used both for ingress and egress. */
typedef enum {
    VTSS_MPLS_TAGTYPE_UNTAGGED = 0,     /**< Frame is untagged */
    VTSS_MPLS_TAGTYPE_CTAGGED  = 1,     /**< Frame is C-tagged */
    VTSS_MPLS_TAGTYPE_STAGGED  = 2      /**< Frame is S-tagged */
} vtss_mll_tagtype_t;




/** \brief Segment state. This expresses whether a segment is sufficiently
 * configured to be able to operate, and in that case, whether it has been
 * able to allocate the necessary hardware resources.
 *
 * A segment may fail to acquire HW resources for two main reasons:
 *
 *   1. The underlying resource pool is shared with other components
 *   2. The underlying resource is dynamically partitioned, meaning that
 *      the allocation limit for one allocation type is dependent on the
 *      number of allocations of other types
 */
typedef enum {
  VTSS_MPLS_SEGMENT_STATE_UNCONF,       /**< Segment is not fully configured */
  VTSS_MPLS_SEGMENT_STATE_CONF,         /**< Segment is sufficiently configured, but has not acquired HW resources */
  VTSS_MPLS_SEGMENT_STATE_UP            /**< Segment has acquired HW resources */
} vtss_mpls_segment_state_t;



/** \brief Cross-connect (XC) types.
 *
 * Note: Once an XC is created, it cannot change type.
 */
typedef enum {
    VTSS_MPLS_XC_TYPE_LSR,              /**< Label swap. In- and out-segment */
    VTSS_MPLS_XC_TYPE_LER               /**< LSP init/term. In- or out-segment */
} vtss_mpls_xc_type_t;



/** \brief DiffServ tunnel modes for TC/TTL */
typedef enum {
    VTSS_MPLS_TUNNEL_MODE_PIPE,         /**< Pipe mode */
    VTSS_MPLS_TUNNEL_MODE_SHORT_PIPE,   /**< Short Pipe mode */
    VTSS_MPLS_TUNNEL_MODE_UNIFORM       /**< Uniform mode */
} vtss_mpls_tunnel_mode_t;



/** \brief MPLS label. The TTL value is only relevant for LER out-segments. */
typedef struct {
    vtss_mpls_label_value_t value;      /**< Label value */
    vtss_mpls_tc_t          tc;         /**< Used unless TC mapping is installed */
    u8                      ttl;        /**< Used when pushing label (not for swap) */
} vtss_mpls_label_t;



/** \brief QoS-to-TC and TC-to-(QoS,DP) mapping. */

#define VTSS_MPLS_QOS_TO_TC_MAP_CNT     8       /**< 8 tables */
#define VTSS_MPLS_QOS_TO_TC_ENTRY_CNT   8       /**< 8 entries per table */

#define VTSS_MPLS_TC_TO_QOS_MAP_CNT     8       /**< 8 tables */
#define VTSS_MPLS_TC_TO_QOS_ENTRY_CNT   8       /**< 8 entries per table */

typedef struct {
    vtss_mpls_tc_t  dp0_tc;                     /**< TC value for DP == 0. Valid range is [0..7] */
    vtss_mpls_tc_t  dp1_tc;                     /**< TC value for DP == 1. Valid range is [0..7] */
} vtss_mpls_qos_to_tc_map_entry_t;

typedef struct {
    vtss_mpls_qos_t qos;                        /**< QoS value, valid range [0..7] */
    u8              dp;                         /**< DP value, valid range [0..1] */
} vtss_mpls_tc_to_qos_map_entry_t;

typedef struct {
    vtss_mpls_qos_to_tc_map_entry_t  qos_to_tc_map[VTSS_MPLS_QOS_TO_TC_MAP_CNT][VTSS_MPLS_QOS_TO_TC_ENTRY_CNT];     /**< (QoS, DP) => TC tables. Access with [map][qos].dp0_tc (or .dp1_tc) */
    vtss_mpls_tc_to_qos_map_entry_t  tc_to_qos_map[VTSS_MPLS_TC_TO_QOS_MAP_CNT][VTSS_MPLS_TC_TO_QOS_ENTRY_CNT];     /**< TC => (QoS, DP) tables. */
} vtss_mpls_tc_conf_t;



/** \brief Layer-2 configuration. */
typedef struct {
    vtss_port_no_t              port;           /**< Port number */
    vtss_port_no_t              ring_port;      /**< TBD: Right choice to put it here? == port => no ring */
    vtss_mac_t                  peer_mac;       /**< MAC address of peer MPLS entity */
    vtss_mac_t                  self_mac;       /**< MAC address of this MPLS entity (TBD) */
    vtss_mll_tagtype_t          tag_type;       /**< Tag type */
    vtss_vid_t                  vid;            /**< VLAN ID, if @tag_type isn't untagged */
    vtss_tagprio_t              pcp;            /**< VLAN PCP; only relevant for tagged egress */
    vtss_dei_t                  dei;            /**< VLAN DEI; only relevant for tagged egress */
} vtss_mpls_l2_t;



/** \brief Pseudo-wire configuration. */
typedef struct {
    BOOL                        is_pw;          /**< TRUE == this segment is used for a PW */
    BOOL                        process_cw;     /**< TRUE for in-segment: Check and pop CW. TRUE for out-segment: Add cw in @cw */
    u32                         cw;             /**< Control Word to use for out-segments */
} vtss_mpls_pw_conf_t;



/** \brief Segment configuration.
 *
 * All indices are VTSS_MPLS_IDX_UNDEFINED if unused/uninitialized.
 */
typedef struct {
    BOOL                        is_in;               /**< [RO] TRUE == in-segment; FALSE == out-segment */
    BOOL                        e_lsp;               /**< [RW] TRUE == E-LSP; FALSE == L-LSP */
    vtss_mpls_cos_t             l_lsp_cos;           /**< [RW] Fixed COS: Ingress TC is mapped to this value for L-LSPs */
    i8                          tc_qos_map_idx;      /**< [RW] In-segment:  Ingress TC-to-(QoS,DP) map: L-LSP: Only DP is mapped; E-LSP: Both QoS and DP are mapped. VTSS_MPLS_IDX_UNDEF = no mapping.
                                                               Out-segment: Egress (QoS,DP)-to-TC map. VTSS_MPLS_IDX_UNDEF = no mapping */
    vtss_mpls_l2_idx_t          l2_idx;              /**< [RO] L2 index */
    vtss_mpls_label_t           label;               /**< [RW] Label value, TC, TTL */
    BOOL                        upstream;            /**< [RW] TRUE == Label is upstream-assigned */
    vtss_mpls_policer_idx_t     policer_idx;         /**< TBD */
    vtss_mpls_pw_conf_t         pw_conf;             /**< [RW] Pseudo-wire configuration */
    vtss_mpls_xc_idx_t          xc_idx;              /**< [RO] Index of XC using this segment */
    vtss_mpls_segment_idx_t     server_idx;          /**< [RO] Server (outer) segment */
} vtss_mpls_segment_t;



/** \brief Cross Connect function for unidirectional LSP/PW.
 * \details
 * A transit (label-swapped) LSP has both an in- and out-segment
 * A terminating LSP/PW only has an in-segment.
 * An initiating LSP/PW only has an out-segment.
 *
 * A segment which isn't attached to an XC cannot transport traffic.
 *
 * Most fields should not be set directly; treat them as R/O and use the
 * appropriate vtss_mpls_...() functions for manipulation.
 *
 * All indices are VTSS_MPLS_IDX_UNDEFINED if unused/uninitialized.
 */
typedef struct {
    vtss_mpls_xc_type_t         type;                       /**< [RO] XC type*/
    vtss_mpls_tunnel_mode_t     tc_tunnel_mode;             /**< [RW] DiffServ tunneling mode for TC. Default: VTSS_MPLS_TUNNEL_MODE_PIPE */
    vtss_mpls_tunnel_mode_t     ttl_tunnel_mode;            /**< [RW] DiffServ tunneling mode for TTL. Default: VTSS_MPLS_TUNNEL_MODE_PIPE */
    vtss_mpls_segment_idx_t     in_seg_idx;                 /**< [RO] Index of in-segment */
    vtss_mpls_segment_idx_t     out_seg_idx;                /**< [RO] Index of out-segment */
#if 0
    // TBD. These are preliminary; do not use!
    vtss_mpls_xc_idx_t          prot_linear_w_idx;          /**< Index of working XC or VTSS_MPLS_IDX_UNDEFINED if it's this entry */
    vtss_mpls_xc_idx_t          prot_linear_p_idx;          /**< Index of protecting XC or VTSS_MPLS_IDX_UNDEFINED if it's this entry (and then prot_w_idx is defined), or none (and then prot_w_idx is not defined) */
    BOOL                        prot_linear_is_failover;    /**< TRUE == W/P is in manual failover state */
    vtss_mpls_xc_idx_t          prot_fb_idx;                /**< Index of FB LSP XC, or VTSS_MPLS_IDX_UNDEFINED if no FB is installed */
    BOOL                        prot_fb_is_failover;        /**< TRUE == FB protection is in manual failover state */
#endif
} vtss_mpls_xc_t;



/*----------------------------------------------------------------------------
 *  Layer-2 functions
 *--------------------------------------------------------------------------*/

/** \brief Allocate L2 entry.
 *
 * The entry contains default values after allocation: Indices are undefined
 * and all other values are zero.
 *
 * \param inst      [IN]     Instance.
 * \param idx       [OUT]    Index of newly allocated entry.
 *
 * \return VTSS_RC_OK when successful; VTSS_RC_ERROR if no entries available.
 */
vtss_rc vtss_mpls_l2_alloc(vtss_inst_t                inst,
                           vtss_mpls_l2_idx_t * const idx);



/** \brief Free L2 entry.
 *
 * \param inst      [IN]     Instance.
 * \param idx       [IN]     Index of entry to free.
 *
 * \return VTSS_RC_OK when successful; VTSS_RC_ERROR if entry is still in use.
 */
vtss_rc vtss_mpls_l2_free(vtss_inst_t              inst,
                          const vtss_mpls_l2_idx_t idx);



/** \brief Get L2 entry.
 *
 * Note that it is possible _but not recommended_ to retrieve entries that
 * haven't been allocated. In that case the contents are undefined.
 *
 * \param inst      [IN]     Instance.
 * \param idx       [IN]     Index of entry to retrieve.
 * \param entry     [OUT]    Entry.
 *
 * \return VTSS_RC_OK when successful; VTSS_RC_ERROR if index is out-of-range.
 */
vtss_rc vtss_mpls_l2_get(vtss_inst_t              inst,
                         const vtss_mpls_l2_idx_t idx,
                         vtss_mpls_l2_t * const   l2);



/** \brief Set L2 entry.
 *
 * Note that it is possible _but not recommended_ to set entries that
 * haven't been allocated. The results are undefined and may lead to
 * system malfunction.
 *
 * If segments are attached, they will be asked to refresh themselves. This
 * will not cause attempted allocation of new HW resources for segments in
 * state VTSS_MPLS_SEGMENT_STATE_CONF (i.e. segments which have not earlier on
 * been able acquire HW resources).
 *
 * \param inst      [IN]     Instance.
 * \param idx       [IN]     Index of entry to retrieve.
 * \param entry     [OUT]    Entry.
 *
 * \return VTSS_RC_OK when successful; VTSS_RC_ERROR if index is out-of-range
 * or some of the new values are invalid.
 */
vtss_rc vtss_mpls_l2_set(vtss_inst_t                  inst,
                         const vtss_mpls_l2_idx_t     idx,
                         const vtss_mpls_l2_t * const l2);



/** \brief Attach segment to L2 entry.
 *
 * If the segment is in state VTSS_MPLS_SEGMENT_STATE_CONF after attachment, it
 * will try to go to state VTSS_MPLS_SEGMENT_STATE_UP, i.e. allocate HW
 * resources.
 *
 * If the segment reaches UP and is a server it will try to (recursively) bring
 * each client UP as well.
 *
 * HW allocation may succeed or fail; use @vtss_mpls_segment_state_get() to
 * determine the outcome.
 *
 * \param inst      [IN]     Instance.
 * \param l2_idx    [IN]     L2 index.
 * \param seg_idx   [IN]     Segment index to attach.
 *
 * \return VTSS_RC_OK when successful; VTSS_RC_ERROR if parameters are invalid.
 */
vtss_rc vtss_mpls_l2_segment_attach(vtss_inst_t                   inst,
                                    const vtss_mpls_l2_idx_t      idx,
                                    const vtss_mpls_segment_idx_t seg_idx);



/** \brief Detach segment from L2 entry.
 *
 * If the segment is a server it will (recursively) bring all clients out of
 * UP state (and usually back to CONF, but don't depend on that behavior.).
 *
 * \param inst      [IN]     Instance.
 * \param seg_idx   [IN]     Segment index to detach.
 *
 * \return VTSS_RC_OK when successful; VTSS_RC_ERROR if parameters are invalid.
 */
vtss_rc vtss_mpls_l2_segment_detach(vtss_inst_t                   inst,
                                    const vtss_mpls_segment_idx_t seg_idx);



/*----------------------------------------------------------------------------
 *  Segment functions
 *--------------------------------------------------------------------------*/

/** \brief Allocate Segment entry.
 *
 * The entry contains default values after allocation: Indices are undefined
 * and all other values except @is_in are zero/FALSE.
 *
 * \param inst      [IN]     Instance.
 * \param is_in     [IN]     TRUE: Allocate in-segment; FALSE: out-segment.
 * \param idx       [OUT]    Index of newly allocated entry.
 *
 * \return VTSS_RC_OK when successful; VTSS_RC_ERROR if no entries available.
 */
vtss_rc vtss_mpls_segment_alloc(vtss_inst_t                     inst,
                                const BOOL                      is_in,
                                vtss_mpls_segment_idx_t * const idx);



/** \brief Free segment entry.
 *
 * \param inst      [IN]     Instance.
 * \param idx       [IN]     Index of entry to free.
 *
 * \return VTSS_RC_OK when successful; VTSS_RC_ERROR if entry is still in use
 * (attached to XC, attached to server, or has clients).
 */
vtss_rc vtss_mpls_segment_free(vtss_inst_t                   inst,
                               const vtss_mpls_segment_idx_t idx);



/** \brief Get segment entry.
 *
 * Note that it is possible _but not recommended_ to retrieve entries that
 * haven't been allocated. In that case the contents are undefined.
 *
 * \param inst      [IN]     Instance.
 * \param idx       [IN]     Index of entry to retrieve.
 * \param entry     [OUT]    Entry.
 *
 * \return VTSS_RC_OK when successful; VTSS_RC_ERROR if index is out-of-range.
 */
vtss_rc vtss_mpls_segment_get(vtss_inst_t                   inst,
                              const vtss_mpls_segment_idx_t idx,
                              vtss_mpls_segment_t * const   seg);



/** \brief Set segment entry.
 *
 * Note that it is possible _but not recommended_ to set entries that
 * haven't been allocated. The results are undefined and may lead to
 * system malfunction.
 *
 * Several values in a @vtss_mpls_segment_t are write-once: Once set they
 * cannot be changed. RO/RW for others depend on the current state of the
 * segment, in particular whether it is attached to something or is a server.
 *
 * TBD: Once the segment is configured and attached, only the label can be
 * changed.
 *
 * If the segment is sufficiently configured, but has been unable to acquire HW
 * resources, the allocation can be re-tried by calling set() again.
 *
 * \param inst      [IN]     Instance.
 * \param idx       [IN]     Index of entry to retrieve.
 * \param entry     [OUT]    Entry.
 *
 * \return VTSS_RC_OK when successful; VTSS_RC_ERROR if index is out-of-range
 * or some of the new values are invalid. NOTE that success does not imply that
 * the segment is up. Use @vtss_mpls_segment_state_get() to determine resulting
 * state.
 */
vtss_rc vtss_mpls_segment_set(vtss_inst_t                       inst,
                              const vtss_mpls_segment_idx_t     idx,
                              const vtss_mpls_segment_t * const seg);



/** \brief Get current segment state.
 *
 * \param inst      [IN]     Instance.
 * \param idx       [IN]     Index of segment to retrieve state for.
 * \param state     [OUT]    Current segment state.
 *
 * \return VTSS_RC_OK when successful; VTSS_RC_ERROR if index is out-of-range.
 */
vtss_rc vtss_mpls_segment_state_get(vtss_inst_t                       inst,
                                    const vtss_mpls_segment_idx_t     idx,
                                    vtss_mpls_segment_state_t * const state);



/** \brief Attach segment to server segment.
 *
 * If the segment is in state VTSS_MPLS_SEGMENT_STATE_CONF after attachment, it
 * will try to go to state VTSS_MPLS_SEGMENT_STATE_UP, i.e. allocate HW
 * resources. (This requires that the server is UP, too.)
 *
 * If the segment reaches UP and is a server it will try to (recursively) bring
 * each client UP as well.
 *
 * HW allocation may succeed or fail; use @vtss_mpls_segment_state_get() to
 * determine the outcome.
 *
 * \param inst      [IN]     Instance.
 * \param idx       [IN]     Segment index to attach.
 * \param srv_idx   [IN]     Server segment index.
 *
 * \return VTSS_RC_OK when successful; VTSS_RC_ERROR if parameters are invalid.
 */
vtss_rc vtss_mpls_segment_server_attach(vtss_inst_t                   inst,
                                        const vtss_mpls_segment_idx_t idx,
                                        const vtss_mpls_segment_idx_t srv_idx);



/** \brief Detach segment from server segment.
 *
 * This will cause the segment to go out of UP state.
 *
 * If the segment is itself a server it will (recursively) bring all clients
 * out of UP state (and usually back to CONF, but don't depend on that
 * behavior.).
 *
 * \param inst      [IN]     Instance.
 * \param seg_idx   [IN]     Segment index to detach from server.
 *
 * \return VTSS_RC_OK when successful; VTSS_RC_ERROR if parameters are invalid.
 */
vtss_rc vtss_mpls_segment_server_detach(vtss_inst_t                   inst,
                                        const vtss_mpls_segment_idx_t idx);



/*----------------------------------------------------------------------------
 *  XC functions
 *--------------------------------------------------------------------------*/

/** \brief Allocate XC entry.
 *
 * The entry contains default values after allocation: Indices are undefined
 * and all other values except @type are zero/FALSE.
 *
 * \param inst      [IN]     Instance.
 * \param type      [IN]     XC type.
 * \param idx       [OUT]    Index of newly allocated entry.
 *
 * \return VTSS_RC_OK when successful; VTSS_RC_ERROR if no entries available.
 */
vtss_rc vtss_mpls_xc_alloc(vtss_inst_t                inst,
                           const vtss_mpls_xc_type_t  type,
                           vtss_mpls_xc_idx_t * const idx);



/** \brief Free XC entry.
 *
 * \param inst      [IN]     Instance.
 * \param idx       [IN]     Index of entry to free.
 *
 * \return VTSS_RC_OK when successful; VTSS_RC_ERROR if entry is still in use
 * or still has segments/multicast chain attached.
 */
vtss_rc vtss_mpls_xc_free(vtss_inst_t              inst,
                          const vtss_mpls_xc_idx_t idx);



/** \brief Get XC entry.
 *
 * Note that it is possible _but not recommended_ to retrieve entries that
 * haven't been allocated. In that case the contents are undefined.
 *
 * \param inst      [IN]     Instance.
 * \param idx       [IN]     Index of entry to retrieve.
 * \param entry     [OUT]    Entry.
 *
 * \return VTSS_RC_OK when successful; VTSS_RC_ERROR if index is out-of-range.
 */
vtss_rc vtss_mpls_xc_get(vtss_inst_t              inst,
                         const vtss_mpls_xc_idx_t idx,
                         vtss_mpls_xc_t * const   xc);



/** \brief Set XC entry.
 *
 * Note that it is possible _but not recommended_ to set entries that
 * haven't been allocated. The results are undefined and may lead to
 * system malfunction.
 *
 * \param inst      [IN]     Instance.
 * \param idx       [IN]     Index of entry to retrieve.
 * \param entry     [OUT]    Entry.
 *
 * \return VTSS_RC_OK when successful; VTSS_RC_ERROR if index is out-of-range
 * or some of the new values are invalid.
 */
vtss_rc vtss_mpls_xc_set(vtss_inst_t                  inst,
                         const vtss_mpls_xc_idx_t     idx,
                         const vtss_mpls_xc_t * const xc);



/** \brief Attach segment to XC.
 *
 * If the segment is in state VTSS_MPLS_SEGMENT_STATE_CONF after attachment, it
 * will try to go to state VTSS_MPLS_SEGMENT_STATE_UP, i.e. allocate HW
 * resources.
 *
 * If the segment reaches UP and is a server it will try to (recursively) bring
 * each client UP as well.
 *
 * HW allocation may succeed or fail; use @vtss_mpls_segment_state_get() to
 * determine the outcome.
 *
 * \param inst      [IN]     Instance.
 * \param xc_idx    [IN]     XC index.
 * \param seg_idx   [IN]     Segment index to attach.
 *
 * \return VTSS_RC_OK when successful; VTSS_RC_ERROR if parameters are invalid.
 */
vtss_rc vtss_mpls_xc_segment_attach(vtss_inst_t                   inst,
                                    const vtss_mpls_xc_idx_t      xc_idx,
                                    const vtss_mpls_segment_idx_t seg_idx);



/** \brief Detach segment from XC.
 *
 * This will cause the segment to go out of UP state.
 *
 * If the segment is itself a server it will (recursively) bring all clients
 * out of UP state (and usually back to CONF, but don't depend on that
 * behavior.).
 *
 * \param inst      [IN]     Instance.
 * \param seg_idx   [IN]     Segment index to detach from server.
 *
 * \return VTSS_RC_OK when successful; VTSS_RC_ERROR if parameters are invalid.
 */
vtss_rc vtss_mpls_xc_segment_detach(vtss_inst_t                   inst,
                                    const vtss_mpls_segment_idx_t seg_idx);



/** \brief Attach multicast segment to XC.
 *
 * \param inst      [IN]     Instance.
 * \param xc_idx    [IN]     XC index.
 * \param seg_idx   [IN]     Multicast segment index to attach.
 *
 * \return VTSS_RC_OK when successful; VTSS_RC_ERROR if parameters are invalid.
 */
vtss_rc vtss_mpls_xc_mc_segment_attach(vtss_inst_t                   inst,
                                       const vtss_mpls_xc_idx_t      xc_idx,
                                       const vtss_mpls_segment_idx_t seg_idx);



/** \brief Detach multicast segment from XC.
 *
 * \param inst      [IN]     Instance.
 * \param xc_idx    [IN]     XC index.
 * \param seg_idx   [IN]     Multicast segment index to detach.
 *
 * \return VTSS_RC_OK when successful; VTSS_RC_ERROR if parameters are invalid.
 */
vtss_rc vtss_mpls_xc_mc_segment_detach(vtss_inst_t                   inst,
                                       const vtss_mpls_xc_idx_t      xc_idx,
                                       const vtss_mpls_segment_idx_t seg_idx);



/*----------------------------------------------------------------------------
 *  TC config functions
 *--------------------------------------------------------------------------*/

/** \brief Retrieve current global TC config.
 *
 * \param inst      [IN]     Instance.
 * \param conf      [OUT]    Configuration.
 *
 * \return Return code.
 */
vtss_rc vtss_mpls_tc_conf_get(vtss_inst_t                 inst,
                              vtss_mpls_tc_conf_t * const conf);



/** \brief Set global TC config.
 *
 * NOTE: conf.map[0] is pre-configured and cannot be altered. It provides
 * an identity mapping.
 *
 * \param inst      [IN]     Instance.
 * \param conf      [IN]     Configuration.
 *
 * \return VTSS_RC_OK when successful; VTSS_RC_ERROR if parameters are invalid.
 */
vtss_rc vtss_mpls_tc_conf_set(vtss_inst_t                       inst,
                              const vtss_mpls_tc_conf_t * const conf);



#endif	/* VTSS_FEATURE_MPLS */

#endif	/* _VTSS_MPLS_API_NEW_H_ */
