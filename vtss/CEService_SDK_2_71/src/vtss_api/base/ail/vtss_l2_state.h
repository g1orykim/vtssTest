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

#ifndef _VTSS_L2_STATE_H_
#define _VTSS_L2_STATE_H_

#if defined(VTSS_FEATURE_LAYER2)

/* Port forwarding state */
#define VTSS_PORT_RX_FORWARDING(fwd_state) (fwd_state == VTSS_PORT_FORWARD_ENABLED || fwd_state == VTSS_PORT_FORWARD_INGRESS ? 1 : 0)
#define VTSS_PORT_TX_FORWARDING(fwd_state) (fwd_state == VTSS_PORT_FORWARD_ENABLED || fwd_state == VTSS_PORT_FORWARD_EGRESS ? 1 : 0)

#if defined(VTSS_FEATURE_PVLAN)
/* PVLAN entry */
typedef struct {
    BOOL member[VTSS_PORT_ARRAY_SIZE]; /* Member ports */
} vtss_pvlan_entry_t;
#endif /* VTSS_FEATURE_PVLAN */

/* PGID entry */
typedef struct {
    BOOL member[VTSS_PORT_ARRAY_SIZE]; /* Egress ports */
    BOOL resv;                         /* Fixed reservation */
    u32  references;                   /* Number references to entry */
    BOOL cpu_copy;                     /* CPU copy */
    vtss_packet_rx_queue_t cpu_queue;  /* CPU queue */
} vtss_pgid_entry_t;

/* Number of destination masks */

#if defined(VTSS_ARCH_LUTON28)
#define VTSS_PGID_LUTON28 64
#define VTSS_PGIDS        VTSS_PGID_LUTON28
#endif /* VTSS_ARCH_LUTON28 */

#if defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_SERVAL)
#define VTSS_PGID_LUTON26 64
#undef VTSS_PGIDS
#define VTSS_PGIDS        VTSS_PGID_LUTON26
#endif /* VTSS_ARCH_LUTON26/SERVAL */

#if defined(VTSS_ARCH_JAGUAR_1)
#define VTSS_PGID_JAGUAR_1 1312
#undef VTSS_PGIDS
#define VTSS_PGIDS VTSS_PGID_JAGUAR_1
#endif /* VTSS_ARCH_JAGUAR_1 */

#if defined(VTSS_ARCH_JAGUAR_2)
// JR2-TBD: Insert correct value
#define VTSS_PGID_JAGUAR_2 1312
#undef VTSS_PGIDS
#define VTSS_PGIDS VTSS_PGID_JAGUAR_2
#endif /* VTSS_ARCH_JAGUAR_2 */

/* Pseudo PGID for IPv4/IPv6 MC */
#define VTSS_PGID_NONE VTSS_PGIDS

#define VTSS_GLAG_NO_NONE 0xffffffff

/* Size of lookup page and pointer array */
#define VTSS_MAC_PAGE_SIZE 128
#define VTSS_MAC_PTR_SIZE  (VTSS_MAC_ADDRS/VTSS_MAC_PAGE_SIZE)

/* MAC address table users */
#define VTSS_MAC_USER_NONE 0 /* Normal entries added by the application */
#define VTSS_MAC_USER_SSM  1 /* Internal entries added for SSM purposes */
#define VTSS_MAC_USER_ASM  2 /* Internal entries added for ASM purposes */
typedef u8 vtss_mac_user_t;

/* MAC address entry flags */
#define VTSS_MAC_FLAG_ADDED 0x01 /* Entry added flag used for warm start synchronization */

/* MAC address table for get next operations */
typedef struct vtss_mac_entry_t {
    struct vtss_mac_entry_t *next;  /* Next in list */
    u32                     mach;  /* VID and 16 MSB of MAC */
    u32                     macl;  /* 32 LSB of MAC */
    u8                      member[VTSS_PORT_BF_SIZE];
    BOOL                    cpu_copy;
    u8                      flags;
    vtss_packet_rx_queue_t  cpu_queue;
    vtss_mac_user_t         user;
} vtss_mac_entry_t;

/* IPv4 and IPv6 multicast address */
#define VTSS_MAC_IPV4_MC(mac) (mac[0] == 0x01 && mac[1] == 0x00 && mac[2] == 0x5e && (mac[3] & 0x80) == 0x00)
#define VTSS_MAC_IPV6_MC(mac) (mac[0] == 0x33 && mac[1] == 0x33)
#define VTSS_MAC_BC(mac)      (mac[0] == 0xff && mac[1] == 0xff && mac[2] == 0xff && mac[3] == 0xff && mac[4] == 0xff && mac[5] == 0xff)

void vtss_mach_macl_get(const vtss_vid_mac_t *vid_mac, u32 *mach, u32 *macl);
void vtss_mach_macl_set(vtss_vid_mac_t *vid_mac, u32 mach, u32 macl);

/* Counter for number of enabled rings with port in discarding state */
#if (VTSS_ERPIS > 255)
typedef u16 vtss_erps_counter_t;
#else
typedef u8 vtss_erps_counter_t;
#endif

/* For IPMC SSM, VLAN ID may be allocated for IPv4/IPv6 FID */
#define IPMC_USED_NONE 0x00
#define IPMC_USED_IPV4 0x01
#define IPMC_USED_IPV6 0x02

/* VLAN entry */
typedef struct {
    BOOL        enabled;                   /* At least one port enabled */
    BOOL        update;                    /* Update flag */
    u8          member[VTSS_PORT_BF_SIZE]; /* Port members */
#if defined(VTSS_FEATURE_VLAN_TX_TAG)
    u8          tx_tag[VTSS_PORT_ARRAY_SIZE]; /* Tx tagging */
#endif /* VTSS_FEATURE_VLAN_TX_TAG */
    vtss_erps_counter_t erps_discard_cnt[VTSS_PORT_ARRAY_SIZE]; /* ERPS discard counter */
    vtss_msti_t msti;                      /* MSTP instance */
    BOOL        isolated;                  /* Port isolation */
    BOOL        evc_learning;              /* Learning for EVC */
    BOOL        ipmc_used;                 /* In use for SSM */
    u32         mask;                      /* Previous mask written */
    vtss_vlan_vid_conf_t conf;             /* VID configuration */
} vtss_vlan_entry_t;

/* MSTP entry */
typedef struct {
    vtss_stp_state_t state[VTSS_PORT_ARRAY_SIZE]; /* MSTP state */
} vtss_mstp_entry_t;

/* ERPS entry */
typedef struct {
    u8   vlan_member[VTSS_BF_SIZE(VTSS_VIDS)]; /* VLAN members */
    u8   port_member[VTSS_PORT_BF_SIZE];       /* Forwarding port members */
} vtss_erps_entry_t;

/* Port protection */
typedef struct {
    vtss_eps_port_conf_t conf;     /* Configuration */
    vtss_eps_selector_t  selector; /* Selector */
} vtss_port_eps_t;

#if defined(VTSS_FEATURE_VLAN_TRANSLATION)
/* VLAN Translation Group entry (Group to VLAN Translation mappings) */
typedef struct vtss_vlan_trans_grp2vlan_entry_t {
    struct vtss_vlan_trans_grp2vlan_entry_t     *next;                             /* Next in list                      */
    vtss_vlan_trans_grp2vlan_conf_t             conf;                              /* Group to VLAN configuration       */
} vtss_vlan_trans_grp2vlan_entry_t;

/* VLAN Translation Group lists - used and free */
typedef struct {
    vtss_vlan_trans_grp2vlan_entry_t  *used;                                       /* used list                         */
    vtss_vlan_trans_grp2vlan_entry_t  *free;                                       /* free list                         */
    vtss_vlan_trans_grp2vlan_entry_t  trans_list[VTSS_VLAN_TRANS_MAX_CNT];         /* Actual storage for list members   */
} vtss_vlan_trans_grp2vlan_t;

/* VLAN Translation Port entry (Ports to Group mappings) */
typedef struct vtss_vlan_trans_port2grp_entry_t {
    struct vtss_vlan_trans_port2grp_entry_t     *next;                             /* Next in list                      */
    vtss_vlan_trans_port2grp_conf_t             conf;                              /* Port to Group configuration       */
} vtss_vlan_trans_port2grp_entry_t;

/* VLAN Translation Port lists - used and free */
typedef struct {
    vtss_vlan_trans_port2grp_entry_t   *used;                                      /* used list                         */
    vtss_vlan_trans_port2grp_entry_t   *free;                                      /* free list                         */
    vtss_vlan_trans_port2grp_entry_t   port_list[VTSS_VLAN_TRANS_GROUP_MAX_CNT];   /* Actual storage for list members   */
} vtss_vlan_trans_port2grp_t;
#endif /* VTSS_FEATURE_VLAN_TRANSLATION */

#if defined(VTSS_FEATURE_VLAN_COUNTERS)
typedef struct {
    vtss_chip_counter_t frames;                         /**< Frame counters */
    vtss_chip_counter_t bytes;                          /**< Byte counters */
} vtss_chip_counter_pair_t;

typedef struct {
    vtss_chip_counter_pair_t rx_vlan_unicast;           /**< The total number of unicast packets and bytes counted on this VLAN */
    vtss_chip_counter_pair_t rx_vlan_multicast;         /**< The total number of multicast packets and bytes counted on this VLAN */
    vtss_chip_counter_pair_t rx_vlan_flood;             /**< The total number of flood packets and bytes counted on this VLAN */
} vtss_vlan_chip_counters_t;

/* VLAN Counter information */
typedef struct {
    u32                         poll_idx;               /* Counter polling index */
    vtss_vlan_chip_counters_t   counters[VTSS_VIDS];    /* Counters for all the VLANs */
} vtss_vlan_counter_info_t;
#endif /* VTSS_FEATURE_VLAN_COUNTERS */

#if defined(VTSS_FEATURE_VSTAX)
/* VStaX information per chip */
typedef struct {
    u32                      mask_a;       /* Stack port A mask */
    u32                      mask_b;       /* Stack port B mask */
    vtss_vstax_port_conf_t   port_conf[2]; /* Stack A/B information */
#if defined(VTSS_FEATURE_VSTAX_V2)
    vtss_vstax_route_table_t rt_table;     /* Route table */
    u32                      upsid_active; /* Active UPSID mask */
#endif /* VTSS_FEATURE_VSTAX_V2 */
} vtss_vstax_chip_info_t;

/* VStaX information */
typedef struct {
    vtss_vstax_upsid_t     upsid[2];                  /* Info per UPS */
    vtss_vstax_chip_info_t chip_info[VTSS_CHIP_CNT];  /* Info per chip */
    vtss_vstax_upsid_t     master_upsid;              /* UPSID of current master */
} vtss_vstax_info_t;
#endif /* VTSS_FEATURE_VSTAX */

#if defined(VTSS_FEATURE_VSTAX_V2)
/* GLAG configuration */
typedef struct {
    vtss_vstax_glag_entry_t entry;   /* UPSID/UPSPN entry */
    vtss_port_no_t          port_no; /* Local port or VTSS_PORT_NO_NONE */
} vtss_glag_conf_t;
#endif /* VTSS_FEATURE_VSTAX_V2 */

#if defined(VTSS_FEATURE_IPV4_MC_SIP) || defined(VTSS_FEATURE_IPV6_MC_SIP)
/* Numner of sources and destinations */
#define VTSS_IPMC_SRC_MAX 256
#define VTSS_IPMC_DST_MAX 2048

typedef union {
    vtss_ipv4_t ipv4; /* IPv4 address */
    vtss_ipv6_t ipv6; /* IPv6 address */
} vtss_ip_addr_internal_t;

/* IPMC source data */
typedef struct {
    vtss_ip_addr_internal_t sip; /* Source IP address */
    vtss_vid_t              vid; /* VLAN ID */
    vtss_vid_t              fid; /* FID allocated for this source (if SIP non-zero) */
    BOOL                    ssm; /* SSM indication (zero SIP) */
} vtss_ipmc_src_data_t;

/* IPMC destination data */
typedef struct {
    vtss_ip_addr_internal_t dip;                       /* Destination IP address */
    u8                      member[VTSS_PORT_BF_SIZE]; /* Port members */
} vtss_ipmc_dst_data_t;

/* IPMC source and destination data */
typedef struct {
    BOOL                 ipv6;    /* IPv4/IPv6 indication */
    BOOL                 src_add; /* Source add flag */
    BOOL                 src_del; /* Source delete flag */
    BOOL                 dst_add; /* destination add flag */
    BOOL                 dst_del; /* destionation delete flag */
    vtss_ipmc_src_data_t src;
    vtss_ipmc_dst_data_t dst;
} vtss_ipmc_data_t;

/* IPMC destination entry */
typedef struct vtss_ipmc_dst_t {
    struct vtss_ipmc_dst_t *next; /* Next destination entry */
    vtss_ipmc_dst_data_t   data;  /* Entry data */
    BOOL                   add;   /* Internal add flag */
} vtss_ipmc_dst_t;

/* IPMC source entry */
typedef struct vtss_ipmc_src_t {
    struct vtss_ipmc_src_t *next;  /* Next source entry */
    vtss_ipmc_dst_t        *dest;  /* Destination list */
    vtss_ipmc_src_data_t   data;   /* Entry data */
} vtss_ipmc_src_t;

/* IPMC object */
typedef struct {
    u32             src_count;    /* Actual number of sources */
    u32             src_max;      /* Maximum number of sources */
    u32             dst_count;    /* Actual number of destinations */
    u32             dst_max;      /* Maximum number of destinations */
    vtss_ipmc_src_t *src_used[2]; /* Used IPv4(0)/IPv6(1) source entries */
    vtss_ipmc_src_t *src_free;    /* Free source entries */
    vtss_ipmc_dst_t *dst_free;    /* Free destination entries */
    const char      *name;        /* IPMC name for debugging */
} vtss_ipmc_obj_t;

typedef struct {
    vtss_ipmc_obj_t obj;                          /* Object */
    vtss_ipmc_src_t src_table[VTSS_IPMC_SRC_MAX]; /* Source table */
    vtss_ipmc_dst_t dst_table[VTSS_IPMC_DST_MAX]; /* Destination table */
} vtss_ipmc_info_t;

typedef enum {
    VTSS_IPMC_CMD_CHECK, /* Check resources */
    VTSS_IPMC_CMD_ADD,   /* Add resources */
    VTSS_IPMC_CMD_DEL,   /* Delete resources */
} vtss_ipmc_cmd_t;
#endif /* VTSS_FEATURE_IPV4_MC_SIP || VTSS_FEATURE_IPV6_MC_SIP */

typedef struct {
    /* CIL function pointers */
    vtss_rc (* mac_table_add)(struct vtss_state_s *vtss_state,
                              const vtss_mac_table_entry_t *const entry, u32 pgid);
    vtss_rc (* mac_table_del)(struct vtss_state_s *vtss_state,
                              const vtss_vid_mac_t *const vid_mac);
    vtss_rc (* mac_table_get)(struct vtss_state_s *vtss_state,
                              vtss_mac_table_entry_t *const entry, u32 *pgid);
    vtss_rc (* mac_table_get_next)(struct vtss_state_s *vtss_state,
                                   vtss_mac_table_entry_t *const entry, u32 *pgid);
    vtss_rc (* mac_table_age_time_set)(struct vtss_state_s *vtss_state);
    vtss_rc (* mac_table_age)(struct vtss_state_s *vtss_state,
                              BOOL             pgid_age,
                              u32              pgid,
                              BOOL             vid_age,
                              const vtss_vid_t vid);
    vtss_rc (* mac_table_status_get)(struct vtss_state_s *vtss_state,
                                     vtss_mac_table_status_t *const status);
#if defined(VTSS_FEATURE_VSTAX_V2)
    vtss_rc (* mac_table_upsid_upspn_flush)(struct vtss_state_s *vtss_state,
                                            const vtss_vstax_upsid_t upsid,
                                            const vtss_vstax_upspn_t upspn);
#endif /* VTSS_FEATURE_VSTAX_V2 */
    vtss_rc (* learn_port_mode_set)(struct vtss_state_s *vtss_state,
                                    const vtss_port_no_t port_no);
    vtss_rc (* learn_state_set)(struct vtss_state_s *vtss_state,
                                const BOOL member[VTSS_PORT_ARRAY_SIZE]);
    vtss_rc (* mstp_vlan_msti_set)(struct vtss_state_s *vtss_state,
                                   const vtss_vid_t vid);
    vtss_rc (* mstp_state_set)(struct vtss_state_s *vtss_state,
                               const vtss_port_no_t port_no,
                               const vtss_msti_t    msti);
    vtss_rc (* erps_vlan_member_set)(struct vtss_state_s *vtss_state,
                                     const vtss_erpi_t erpi,
                                     const vtss_vid_t  vid);
    vtss_rc (* erps_port_state_set)(struct vtss_state_s *vtss_state,
                                    const vtss_erpi_t    erpi,
                                    const vtss_port_no_t port_no);
#if defined(VTSS_FEATURE_VLAN_PORT_V2)
    vtss_rc (* vlan_conf_set)(struct vtss_state_s *vtss_state);
#endif /* VTSS_FEATURE_VLAN_PORT_V2 */
    vtss_rc (* vlan_port_conf_set)(struct vtss_state_s *vtss_state,
                                   const vtss_port_no_t port_no);
    vtss_rc (* vlan_port_conf_update)(struct vtss_state_s *vtss_state,
                                      vtss_port_no_t port_no, vtss_vlan_port_conf_t *conf);
    vtss_rc (* vlan_port_members_set)(struct vtss_state_s *vtss_state,
                                      const vtss_vid_t vid);
#if defined(VTSS_FEATURE_VLAN_TX_TAG)
    vtss_rc (* vlan_tx_tag_set)(struct vtss_state_s *vtss_state,
                                const vtss_vid_t vid,
                                const vtss_vlan_tx_tag_t tx_tag[VTSS_PORT_ARRAY_SIZE]);
#endif /* VTSS_FEATURE_VLAN_TX_TAG */
    vtss_rc (* vlan_mask_update)(struct vtss_state_s *vtss_state,
                                 vtss_vid_t vid, BOOL member[VTSS_PORT_ARRAY_SIZE]);
    vtss_rc (* isolated_vlan_set)(struct vtss_state_s *vtss_state,
                                  const vtss_vid_t vid);
    vtss_rc (* isolated_port_members_set)(struct vtss_state_s *vtss_state);
    vtss_rc (* aggr_mode_set)(struct vtss_state_s *vtss_state);
    vtss_rc (* mirror_port_set)(struct vtss_state_s *vtss_state);
    vtss_rc (* mirror_ingress_set)(struct vtss_state_s *vtss_state);
    vtss_rc (* mirror_egress_set)(struct vtss_state_s *vtss_state);
    vtss_rc (* mirror_cpu_ingress_set)(struct vtss_state_s *vtss_state);
    vtss_rc (* mirror_cpu_egress_set)(struct vtss_state_s *vtss_state);
    vtss_rc (* eps_port_set)(struct vtss_state_s *vtss_state,
                             const vtss_port_no_t port_no);
    vtss_rc (* flood_conf_set)(struct vtss_state_s *vtss_state);
#if defined(VTSS_FEATURE_IPV4_MC_SIP)
    vtss_rc (* ipv4_mc_add)(struct vtss_state_s *vtss_state,
                            vtss_vid_t vid, vtss_ip_t sip, vtss_ip_t dip,
                            const BOOL member[VTSS_PORT_ARRAY_SIZE]);
    vtss_rc (* ipv4_mc_del)(struct vtss_state_s *vtss_state,
                            vtss_vid_t vid, vtss_ip_t sip, vtss_ip_t dip);
#endif /* VTSS_FEATURE_IPV4_MC_SIP */
#if defined(VTSS_FEATURE_IPV6_MC_SIP)
    vtss_rc (* ipv6_mc_add)(struct vtss_state_s *vtss_state,
                            vtss_vid_t vid, vtss_ipv6_t sip, vtss_ipv6_t dip,
                            const BOOL member[VTSS_PORT_ARRAY_SIZE]);
    vtss_rc (* ipv6_mc_del)(struct vtss_state_s *vtss_state,
                            vtss_vid_t vid, vtss_ipv6_t sip, vtss_ipv6_t dip);
#endif /* VTSS_FEATURE_IPV6_MC_SIP */
#if defined(VTSS_FEATURE_IPV4_MC_SIP) || defined(VTSS_FEATURE_IPV6_MC_SIP)
    vtss_rc (* ip_mc_update)(struct vtss_state_s *vtss_state,
                             vtss_ipmc_data_t *ipmc, vtss_ipmc_cmd_t cmd);
#endif /* VTSS_FEATURE_IPV4_MC_SIP || VTSS_FEATURE_IPV6_MC_SIP */
#if defined(VTSS_FEATURE_AGGR_GLAG) && defined(VTSS_FEATURE_VSTAX_V2)
    vtss_rc (* glag_src_table_write)(struct vtss_state_s *vtss_state,
                                     u32 glag_no, u32 port_count,
                                     BOOL member[VTSS_PORT_ARRAY_SIZE]);
    vtss_rc (* glag_port_write)(struct vtss_state_s *vtss_state,
                                u32 glag_no, u32 idx, vtss_vstax_glag_entry_t *entry);
#endif /* VTSS_FEATURE_AGGR_GLAG && VTSS_FEATURE_VSTAX_V2 */
    vtss_rc (* src_table_write)(struct vtss_state_s *vtss_state,
                                vtss_port_no_t port_no, BOOL member[VTSS_PORT_ARRAY_SIZE]);
    vtss_rc (* pgid_table_write)(struct vtss_state_s *vtss_state,
                                 u32 pgid, BOOL member[VTSS_PORT_ARRAY_SIZE]);
    vtss_rc (* aggr_table_write)(struct vtss_state_s *vtss_state,
                                 u32 ac, BOOL member[VTSS_PORT_ARRAY_SIZE]);
    vtss_rc (* pmap_table_write)(struct vtss_state_s *vtss_state,
                                 vtss_port_no_t port_no, vtss_port_no_t l_port_no);
#ifdef VTSS_FEATURE_SFLOW
    vtss_rc (*sflow_port_conf_set)(struct vtss_state_s *vtss_state,
                                   vtss_port_no_t port_no, const vtss_sflow_port_conf_t *const conf);
    vtss_rc (*sflow_sampling_rate_convert)(struct vtss_state_s *const state, const BOOL power2, const u32 rate_in, u32 *const rate_out);
#endif /* VTSS_FEATURE_SFLOW */

#if defined(VTSS_FEATURE_VCL)
    vtss_rc (* vcl_port_conf_set)(struct vtss_state_s *vtss_state,
                                  const vtss_port_no_t port_no);
    vtss_rc (* vce_add)(struct vtss_state_s *vtss_state,
                        const vtss_vce_id_t  vce_id,
                        const vtss_vce_t     *const vce);
    vtss_rc (* vce_del)(struct vtss_state_s *vtss_state,
                        const vtss_vce_id_t  vce_id);
#endif /* VTSS_FEATURE_VCL */
#if defined(VTSS_FEATURE_VLAN_TRANSLATION)
    vtss_rc (* vlan_trans_group_add) (struct vtss_state_s *vtss_state,
                                      const u16           group_id,
                                      const vtss_vid_t    vid,
                                      const vtss_vid_t    trans_vid);
    vtss_rc (* vlan_trans_group_del) (struct vtss_state_s *vtss_state,
                                      const u16           group_id,
                                      const vtss_vid_t    vid);
    vtss_rc (* vlan_trans_group_get) (struct vtss_state_s *vtss_state,
                                      vtss_vlan_trans_grp2vlan_conf_t *conf, BOOL next);
    vtss_rc (* vlan_trans_port_conf_set) (struct vtss_state_s *vtss_state,
                                          const vtss_vlan_trans_port2grp_conf_t *conf);
    vtss_rc (* vlan_trans_port_conf_get) (struct vtss_state_s *vtss_state,
                                          vtss_vlan_trans_port2grp_conf_t *conf, BOOL next);
#endif /* VTSS_FEATURE_VLAN_TRANSLATION */
#if defined(VTSS_ARCH_SERVAL)
    vtss_rc (* vcap_port_conf_set)(struct vtss_state_s *vtss_state,
                                   const vtss_port_no_t port_no);
#endif /* VTSS_ARCH_SERVAL */
#if defined(VTSS_FEATURE_VLAN_COUNTERS)
    vtss_rc (* vlan_counters_get)(struct vtss_state_s *vtss_state,
                                  const vtss_vid_t          vid,
                                  vtss_vlan_counters_t      *const counters);
    vtss_rc (* vlan_counters_clear)(struct vtss_state_s *vtss_state,
                                    const vtss_vid_t        vid);
#endif /* VTSS_FEATURE_VLAN_COUNTERS */
#if defined(VTSS_FEATURE_VSTAX)
    vtss_rc (* vstax_conf_set)(struct vtss_state_s *vtss_state);
    vtss_rc (* vstax_port_conf_set)(struct vtss_state_s *vtss_state,
                                    const BOOL stack_port_a);
    vtss_rc (* vstax_master_upsid_set)(struct vtss_state_s *vtss_state);
#if defined(VTSS_FEATURE_VSTAX_V2)
    vtss_rc (* vstax_topology_set)(struct vtss_state_s *vtss_state);
#endif
#endif /* VTSS_FEATURE_VSTAX */

    /* Configuration/state */
    /* Aggregated forwarding information */
    BOOL                          rx_forward[VTSS_PORT_ARRAY_SIZE];
    BOOL                          tx_forward[VTSS_PORT_ARRAY_SIZE];
    BOOL                          tx_forward_aggr[VTSS_PORT_ARRAY_SIZE];

    BOOL                          port_state[VTSS_PORT_ARRAY_SIZE];
    vtss_aggr_no_t                port_aggr_no[VTSS_PORT_ARRAY_SIZE];
    vtss_auth_state_t             auth_state[VTSS_PORT_ARRAY_SIZE];
    vtss_stp_state_t              stp_state[VTSS_PORT_ARRAY_SIZE];
#if defined(VTSS_FEATURE_PVLAN)
    vtss_pvlan_entry_t            pvlan_table[VTSS_PVLAN_ARRAY_SIZE];
    BOOL                          apvlan_table[VTSS_PORT_ARRAY_SIZE][VTSS_PORT_ARRAY_SIZE];
#endif /* VTSS_FEATURE_PVLAN */
    vtss_dgroup_port_conf_t       dgroup_port_conf[VTSS_PORT_ARRAY_SIZE];
    vtss_mirror_conf_t            mirror_conf;

    BOOL                          mirror_ingress[VTSS_PORT_ARRAY_SIZE];
    BOOL                          mirror_egress[VTSS_PORT_ARRAY_SIZE];
    BOOL                          mirror_cpu_ingress;
    BOOL                          mirror_cpu_egress;
#if defined(VTSS_FEATURE_VLAN_PORT_V2)
    vtss_vlan_conf_t              vlan_conf;
    vtss_vlan_port_type_t         vlan_port_type[VTSS_PORT_ARRAY_SIZE];
#endif /* VTSS_FEATURE_VLAN_PORT_V2 */
    vtss_vlan_port_conf_t         vlan_port_conf[VTSS_PORT_ARRAY_SIZE];
#if defined(VTSS_FEATURE_VCL)
    vtss_vcl_port_conf_t          vcl_port_conf[VTSS_PORT_ARRAY_SIZE];
    vtss_vcl_port_conf_t          vcl_port_conf_old;
#endif /* VTSS_FEATURE_VCL */
    vtss_port_eps_t               port_protect[VTSS_PORT_ARRAY_SIZE];
    vtss_vlan_entry_t             vlan_table[VTSS_VIDS];
    vtss_mstp_entry_t             mstp_table[VTSS_MSTI_ARRAY_SIZE];
    vtss_erps_entry_t             erps_table[VTSS_ERPI_ARRAY_SIZE];
    vtss_learn_mode_t             learn_mode[VTSS_PORT_ARRAY_SIZE];
    BOOL                          isolated_port[VTSS_PORT_ARRAY_SIZE];
    BOOL                          uc_flood[VTSS_PORT_ARRAY_SIZE];
    BOOL                          mc_flood[VTSS_PORT_ARRAY_SIZE];
    BOOL                          ipv4_mc_flood[VTSS_PORT_ARRAY_SIZE];
    BOOL                          ipv6_mc_flood[VTSS_PORT_ARRAY_SIZE];
    BOOL                          ipv6_mc_scope;
    vtss_mac_table_age_time_t     mac_age_time;
    vtss_mac_table_status_t       mac_status;
    u32                           mac_index_next;  /* Index for MAC table get next */
    u32                           mac_table_max;   /* Maximum number of entries in mac_table */
    u32                           mac_table_count; /* Actual number of entries in mac_table */
    vtss_mac_entry_t              *mac_list_used;  /* Sorted list of entries */
    vtss_mac_entry_t              *mac_list_free;  /* Free list */
    vtss_mac_entry_t              mac_table[VTSS_MAC_ADDRS]; /* Sorted MAC address table */
    u32                           mac_ptr_count;   /* Number of valid pointers */
    vtss_mac_entry_t              *mac_list_ptr[VTSS_MAC_PTR_SIZE]; /* Pointer array */
    u32                           ac_count;
    vtss_aggr_mode_t              aggr_mode;
    u32                           aggr_chip_port_next[2];
    u32                           pgid_count;
    u32                           pgid_drop;
    u32                           pgid_flood;
    u32                           pgid_glag_dest;
    u32                           pgid_glag_src;
    u32                           pgid_glag_aggr_a;
    u32                           pgid_glag_aggr_b;
    vtss_pgid_entry_t             pgid_table[VTSS_PGIDS+1];

#ifdef VTSS_FEATURE_SFLOW
    vtss_sflow_port_conf_t        sflow_conf[VTSS_PORT_ARRAY_SIZE];
    u32                           sflow_max_power_of_two;
#ifdef VTSS_ARCH_JAGUAR_1
    u32                           sflow_ena_cnt[2]; /* Counts - per chip - the number of ports on which sFlow is enabled */
#endif
#endif

#if defined(VTSS_FEATURE_VLAN_TRANSLATION)
    vtss_vlan_trans_grp2vlan_t    vt_trans_conf;
    vtss_vlan_trans_port2grp_t    vt_port_conf;
#endif /* VTSS_FEATURE_VLAN_TRANSLATION */
#if defined(VTSS_FEATURE_VLAN_COUNTERS)
    vtss_vlan_counter_info_t      vlan_counters_info;
#endif /* VTSS_FEATURE_VLAN_COUNTERS */
#if defined(VTSS_FEATURE_AGGR_GLAG)
#if defined(VTSS_FEATURE_VSTAX_V1)
    vtss_port_no_t                glag_members[VTSS_GLAGS][VTSS_GLAG_PORT_ARRAY_SIZE];
#endif /* VSTAX_V1 */
#if defined(VTSS_FEATURE_VSTAX_V2)
    vtss_glag_conf_t              glag_conf[VTSS_GLAGS][VTSS_GLAG_PORT_ARRAY_SIZE];
#endif /* VSTAX_V2 */
    vtss_glag_no_t                port_glag_no[VTSS_PORT_ARRAY_SIZE];
#endif /* VTSS_FEATURE_AGGR_GLAG */
#if defined(VTSS_FEATURE_VSTAX)
    vtss_vstax_conf_t             vstax_conf;
    BOOL                          vstax_port_enabled[VTSS_PORT_ARRAY_SIZE];
    const vtss_vstax_tx_header_t  *vstax_tx_header;
    vtss_vstax_info_t             vstax_info;
#endif /* VTSS_FEATURE_VSTAX */
#if defined(VTSS_FEATURE_IPV4_MC_SIP) || defined(VTSS_FEATURE_IPV6_MC_SIP)
    vtss_ipmc_info_t              ipmc;
#endif /* VTSS_FEATURE_IPV4_MC_SIP || VTSS_FEATURE_IPV6_MC_SIP */
} vtss_l2_state_t;

#if defined(VTSS_FEATURE_IPV4_MC_SIP) || defined(VTSS_FEATURE_IPV6_MC_SIP)
BOOL l26_ssm_validate_fid(u16 *fid, BOOL fid_only);
vtss_rc l26_ssm_conflict_adjustment(vtss_ipmc_src_t *ssm);
#endif /* VTSS_FEATURE_IPV4_MC_SIP || VTSS_FEATURE_IPV6_MC_SIP */

vtss_rc vtss_l2_inst_create(struct vtss_state_s *vtss_state);
vtss_rc vtss_l2_restart_sync(struct vtss_state_s *vtss_state);

vtss_rc vtss_vid_check(const vtss_vid_t vid);
vtss_rc vtss_mac_add(struct vtss_state_s *vtss_state,
                     vtss_mac_user_t user, const vtss_mac_table_entry_t *const entry);
vtss_rc vtss_mac_del(struct vtss_state_s *vtss_state,
                     vtss_mac_user_t user, const vtss_vid_mac_t *const vid_mac);
vtss_rc vtss_mac_get(struct vtss_state_s *vtss_state,
                     const vtss_vid_mac_t   *const vid_mac,
                     vtss_mac_table_entry_t *const entry);
vtss_rc vtss_update_masks(struct vtss_state_s *vtss_state,
                          BOOL src_update, BOOL dest_update, BOOL aggr_update);
vtss_rc vtss_cmn_vlan_members_get(struct vtss_state_s *state, 
                                  const vtss_vid_t vid,
                                  BOOL member[VTSS_PORT_ARRAY_SIZE]);
vtss_rc vtss_cmn_vlan_members_set(struct vtss_state_s *vtss_state, const vtss_vid_t vid);
vtss_rc vtss_cmn_vlan_port_conf_set(struct vtss_state_s *vtss_state, const vtss_port_no_t port_no);
#if defined(VTSS_FEATURE_VLAN_TX_TAG)
vtss_rc vtss_cmn_vlan_tx_tag_set(struct vtss_state_s *vtss_state,
                                 const vtss_vid_t         vid,
                                 const vtss_vlan_tx_tag_t tx_tag[VTSS_PORT_ARRAY_SIZE]);
#endif /* VTSS_FEATURE_VLAN_TX_TAG */
vtss_rc vtss_cmn_vlan_update_all(struct vtss_state_s *vtss_state);
vtss_rc vtss_cmn_mstp_state_set(struct vtss_state_s *vtss_state,
                                const vtss_port_no_t   port_no,
                                const vtss_msti_t      msti);
vtss_rc vtss_cmn_erps_vlan_member_set(struct vtss_state_s *vtss_state,
                                      const vtss_erpi_t erpi,
                                      const vtss_vid_t  vid);
vtss_rc vtss_cmn_erps_port_state_set(struct vtss_state_s *vtss_state,
                                     const vtss_erpi_t    erpi,
                                     const vtss_port_no_t port_no);
vtss_rc vtss_cmn_eps_port_set(struct vtss_state_s *vtss_state, const vtss_port_no_t port_w);
void vtss_debug_print_mac_entry(const vtss_debug_printf_t pr,
                                const char *name,
                                BOOL *header,
                                vtss_mac_table_entry_t *entry,
                                u32 pgid);
#if defined(VTSS_FEATURE_IPV4_MC_SIP) || defined(VTSS_FEATURE_IPV6_MC_SIP)
u32 vtss_cmn_ip2u32(vtss_ip_addr_internal_t *ip, BOOL ipv6);
vtss_rc vtss_cmn_ipv4_mc_add(struct vtss_state_s     *vtss_state,
                             const vtss_vid_t vid,
                             const vtss_ip_t  sip,
                             const vtss_ip_t  dip,
                             const BOOL       member[VTSS_PORT_ARRAY_SIZE]);
vtss_rc vtss_cmn_ipv4_mc_del(struct vtss_state_s     *vtss_state,
                             const vtss_vid_t vid,
                             const vtss_ip_t  sip,
                             const vtss_ip_t  dip);
vtss_rc vtss_cmn_ipv6_mc_add(struct vtss_state_s      *vtss_state,
                             const vtss_vid_t  vid,
                             const vtss_ipv6_t sip,
                             const vtss_ipv6_t dip,
                             const BOOL        member[VTSS_PORT_ARRAY_SIZE]);
vtss_rc vtss_cmn_ipv6_mc_del(struct vtss_state_s      *vtss_state,
                             const vtss_vid_t  vid,
                             const vtss_ipv6_t sip,
                             const vtss_ipv6_t dip);
#endif /* VTSS_FEATURE_IPV4_MC_SIP || VTSS_FEATURE_IPV6_MC_SIP */

#if defined(VTSS_FEATURE_VCL)
vtss_rc vtss_cmn_vce_add(struct vtss_state_s *vtss_state,
                         const vtss_vce_id_t vce_id, const vtss_vce_t *const vce);
vtss_rc vtss_cmn_vce_del(struct vtss_state_s *vtss_state, const vtss_vce_id_t vce_id);
#endif /* VTSS_FEATURE_VCL */
#if defined(VTSS_FEATURE_VLAN_TRANSLATION)
vtss_rc vtss_cmn_vlan_trans_group_add(struct vtss_state_s *vtss_state,
                                      const u16 group_id, const vtss_vid_t vid, const vtss_vid_t trans_vid);
vtss_rc vtss_cmn_vlan_trans_group_del(struct vtss_state_s *vtss_state,
                                      const u16 group_id, const vtss_vid_t vid);
vtss_rc vtss_cmn_vlan_trans_group_get(struct vtss_state_s *vtss_state,
                                      vtss_vlan_trans_grp2vlan_conf_t *conf, BOOL next);
vtss_rc vtss_cmn_vlan_trans_port_conf_set(struct vtss_state_s *vtss_state,
                                          const vtss_vlan_trans_port2grp_conf_t *conf);
vtss_rc vtss_cmn_vlan_trans_port_conf_get(struct vtss_state_s *vtss_state,
                                          vtss_vlan_trans_port2grp_conf_t *conf, BOOL next);
#endif /* VTSS_FEATURE_VLAN_TRANSLATION */

void vtss_l2_debug_print(struct vtss_state_s *vtss_state,
                         const vtss_debug_printf_t pr,
                         const vtss_debug_info_t   *const info);

#endif /* VTSS_FEATURE_LAYER2 */

#endif /* _VTSS_L2_STATE_H_ */
