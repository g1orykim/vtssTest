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

#ifndef _IP2_ITERATORS_H_
#define _IP2_ITERATORS_H_


#define IP2_ITER_MAX_IPS_OBJS       4
#define IP2_ITER_SINGLE_IPS_OBJS    1

#define IP2_ITER_MAX_IFS_OBJS       IP2_MAX_INTERFACES
#define IP2_ITER_SINGLE_IFS_OBJS    1

#define IP2_ITER_MAX_NBR_OBJS       IP2_MAX_STATUS_OBJS
#define IP2_ITER_SINGLE_NBR_OBJS    1

#define IP2_ITER_STATE_ENABLED      0x1
#define IP2_ITER_STATE_DISABLED     0x2
#define IP2_ITER_DO_FORWARDING      IP2_ITER_STATE_ENABLED
#define IP2_ITER_DONOT_FORWARDING   IP2_ITER_STATE_DISABLED


#define IP2_ITER_IFINFST_INIT       0x0
#define IP2_ITER_IFINFST_MGMT_ON    IP2_ITER_STATE_ENABLED
#define IP2_ITER_IFINFST_MGMT_OFF   IP2_ITER_STATE_DISABLED
#define IP2_ITER_IFINFST_LINK_ON    0x10
#define IP2_ITER_IFINFST_LINK_OFF   0x20
#define IP2_ITER_IFINFST_ENA_UP     (IP2_ITER_IFINFST_MGMT_ON | IP2_ITER_IFINFST_LINK_ON)
#define IP2_ITER_IFINFST_ENA_DN     (IP2_ITER_IFINFST_MGMT_ON | IP2_ITER_IFINFST_LINK_OFF)
#define IP2_ITER_IFINFST_DIS_UP     (IP2_ITER_IFINFST_MGMT_OFF | IP2_ITER_IFINFST_LINK_ON)
#define IP2_ITER_IFINFST_DIS_DN     (IP2_ITER_IFINFST_MGMT_OFF | IP2_ITER_IFINFST_LINK_OFF)
#define IP2_ITER_IFINFST_ADM_ACT(x) ((x) & IP2_ITER_IFINFST_MGMT_ON)
#define IP2_ITER_IFINFST_ADM_INA(x) (!IP2_ITER_IFINFST_ADM_ACT((x)))
#define IP2_ITER_IFINFST_LNK_ACT(x) ((x) & IP2_ITER_IFINFST_LINK_ON)
#define IP2_ITER_IFINFST_LNK_INA(x) (!IP2_ITER_IFINFST_LNK_ACT((x)))
#define IP2_ITER_IFINFST_OPR_ACT(x) (IP2_ITER_IFINFST_ADM_ACT((x)) && IP2_ITER_IFINFST_LNK_ACT((x)))
#define IP2_ITER_IFINFST_OPR_INA(x) (!IP2_ITER_IFINFST_OPR_ACT((x)))

typedef enum {
    IP2_IFADR_TYPE_UNICAST      = 0x1,
    IP2_IFADR_TYPE_ANYCAST      = 0x2,
    IP2_IFADR_TYPE_BROADCAST    = 0x3
} ip2_iter_ifadr_type_t;

typedef enum {
    IP2_IFADR_STATUS_PREFER     = 0x1,
    IP2_IFADR_STATUS_DEPRECATE  = 0x2,
    IP2_IFADR_STATUS_INVALID    = 0x3,
    IP2_IFADR_STATUS_INACCESS   = 0x4,
    IP2_IFADR_STATUS_UNKNOWN    = 0x5,
    IP2_IFADR_STATUS_TENTATIVE  = 0x6,
    IP2_IFADR_STATUS_DUPLICATE  = 0x7,
    IP2_IFADR_STATUS_OPTIMISTIC = 0x8
} ip2_iter_ifadr_status_t;

typedef enum {
    IP2_IFADR_STORAGE_OTHER     = 0x1,  /* other than the below kinds */
    IP2_IFADR_STORAGE_VOLATILE  = 0x2,  /* e.g., in RAM */
    IP2_IFADR_STORAGE_NVOLATILE = 0x3,  /* e.g., in NVRAM */
    IP2_IFADR_STORAGE_PERMANENT = 0x4,  /* e.g., partially in ROM */
    IP2_IFADR_STORAGE_READONLY  = 0x5   /* e.g., completely in ROM */
} ip2_iter_ifadr_storage_t;

typedef enum {
    IP2_IFNBR_TYPE_OTHER        = 0x1,  /* none of the following */
    IP2_IFNBR_TYPE_INVALID      = 0x2,  /* an invalidated mapping */
    IP2_IFNBR_TYPE_DYNAMIC      = 0x3,  /* dynamic neighbor from learning */
    IP2_IFNBR_TYPE_STATIC       = 0x4,  /* static neighbor from management */
    IP2_IFNBR_TYPE_LOCAL        = 0x5   /* local interface */
} ip2_iter_ifnbr_type_t;

typedef enum {
    IP2_IFNBR_STATE_REACHABLE   = 0x1,  /* confirmed reachability */
    IP2_IFNBR_STATE_STALE       = 0x2,  /* unconfirmed reachability */
    IP2_IFNBR_STATE_DELAY       = 0x3,  /* waiting for reachability:confirmation before entering the probe state */
    IP2_IFNBR_STATE_PROBE       = 0x4,  /* actively probing */
    IP2_IFNBR_STATE_INVALID     = 0x5,  /* an invalidated mapping */
    IP2_IFNBR_STATE_UNKNOWN     = 0x6,  /* state can not be determined for some reason */
    IP2_IFNBR_STATE_INCOMPLETE  = 0x7   /* address resolution is being performed */
} ip2_iter_ifnbr_state_t;


typedef struct {
    vtss_ip_type_t              version;            /* INDEX */
    vtss_if_id_vlan_t           ifidx;              /* INDEX */

    u32                         os_if_index;        /* actual ifid used in IP stack */
    u32                         reasm_max_size;     /* MTU */
    u32                         reachable_time;     /* life time */
    u32                         retransmit_time;    /* ARP/NS RXMT time */
    u8                          enable_status;      /* management status */
    u8                          forwarding;         /* as a routing intf or not */
} ip2_iter_intf_ifinf_t;

#define IP2_ITER_INTF_IFINFO_VERSION(x)                     \
((x)->version)
#define IP2_ITER_INTF_IFINFO_IFVDX(x)                       \
((x)->ifidx)
#define IP2_ITER_INTF_IFINFO_IFINDEX(x)                     \
((x)->os_if_index)
#define IP2_ITER_INTF_IFINFO_MTU(x)                         \
((x)->reasm_max_size)
#define IP2_ITER_INTF_IFINFO_LIFE_TIME(x)                   \
((x)->reachable_time)
#define IP2_ITER_INTF_IFINFO_RXMT_TIME(x)                   \
((x)->retransmit_time)
#define IP2_ITER_INTF_IFINFO_MGMT_STATE(x)                  \
((x)->enable_status)
#define IP2_ITER_INTF_IFINFO_FORWARDING(x)                  \
((x)->forwarding)

#define IP2_ITER_INTF_IFINFO_VERSION_SET(x, y)              \
(x)->version = (y)
#define IP2_ITER_INTF_IFINFO_IFVDX_SET(x, y)                \
(x)->ifidx = (y)
#define IP2_ITER_INTF_IFINFO_IFINDEX_SET(x, y)              \
(x)->os_if_index = (y)
#define IP2_ITER_INTF_IFINFO_MTU_SET(x, y)                  \
(x)->reasm_max_size = (y)
#define IP2_ITER_INTF_IFINFO_LIFE_TIME_SET(x, y)            \
(x)->reachable_time = (y)
#define IP2_ITER_INTF_IFINFO_RXMT_TIME_SET(x, y)            \
(x)->retransmit_time = (y)
#define IP2_ITER_INTF_IFINFO_MGMT_STATE_CLR(x)              \
(x)->enable_status = IP2_ITER_IFINFST_INIT
#define IP2_ITER_INTF_IFINFO_MGMT_STATE_SET(x, y)           \
(x)->enable_status |= (y)
#define IP2_ITER_INTF_IFINFO_FORWARDING_SET(x, y)           \
(x)->forwarding = (y)

typedef struct {
    vtss_ip_addr_t              ipa;                /* INDEX */

    ip2_iter_intf_ifinf_t       if_info;            /* basic interface information */
    ip2_iter_ifadr_type_t       adr_type;           /* address type */
    ip2_iter_ifadr_status_t     adr_status;         /* address status */
    vtss_timestamp_t            adr_created;        /* time stamp when address is created */
    vtss_timestamp_t            adr_last_changed;   /* time stamp whenever address is changed */
    u32                         adr_row_status;     /* row status */
    ip2_iter_ifadr_storage_t    adr_storage;        /* storage type for this conceptual row */
} ip2_iter_intf_ifadr_t;

#define IP2_ITER_INTF_IFADR_IPA_VERSION(x)                  \
((x)->ipa.type)
#define IP2_ITER_INTF_IFADR_IPA_VERSION_SET(x, y)           \
(x)->ipa.type = (y)
#define IP2_ITER_INTF_IFADR_IPV4_ADDR(x)                    \
(IP2_ITER_INTF_IFADR_IPA_VERSION((x)) == VTSS_IP_TYPE_IPV4) ? ((x)->ipa.addr.ipv4) : 0
#define IP2_ITER_INTF_IFADR_IPV4_ADDR_SET(x, y)             \
do {                                                        \
    (x)->ipa.type = VTSS_IP_TYPE_IPV4;                      \
    (x)->ipa.addr.ipv4 = (y);                               \
} while (0)
#define IP2_ITER_INTF_IFADR_IPV6_ADDR(x)                    \
(IP2_ITER_INTF_IFADR_IPA_VERSION((x)) == VTSS_IP_TYPE_IPV6) ? (&((x)->ipa.addr.ipv6)) : NULL
#define IP2_ITER_INTF_IFADR_IPV6_ADDR_SET(x, y)             \
do {                                                        \
    (x)->ipa.type = VTSS_IP_TYPE_IPV6;                      \
    memcpy(&(x)->ipa.addr.ipv6, (y), sizeof(vtss_ipv6_t));  \
} while (0)
#define IP2_ITER_INTF_IFADR_IFINDEX(x)                      \
IP2_ITER_INTF_IFINFO_IFINDEX(&((x)->if_info))

#define IP2_ITER_INTF_IFADR_TYPE(x)                         \
((x)->adr_type)
#define IP2_ITER_INTF_IFADR_STATUS(x)                       \
((x)->adr_status)
#define IP2_ITER_INTF_IFADR_CREATED(x)                      \
(&((x)->adr_created))
#define IP2_ITER_INTF_IFADR_LAST_CHANGE(x)                  \
(&((x)->adr_last_changed))
#define IP2_ITER_INTF_IFADR_ROW_STATUS(x)                   \
((x)->adr_row_status)
#define IP2_ITER_INTF_IFADR_STORAGE_TYPE(x)                 \
((x)->adr_storage)

#define IP2_ITER_INTF_IFADR_TYPE_SET(x, y)                  \
(x)->adr_type = (y)
#define IP2_ITER_INTF_IFADR_STATUS_SET(x, y)                \
(x)->adr_status = (y)
#define IP2_ITER_INTF_IFADR_CREATED_SET(x, y)               \
memcpy(&((x)->adr_created), (y), sizeof(vtss_timestamp_t))
#define IP2_ITER_INTF_IFADR_LAST_CHANGE_SET(x, y)           \
memcpy(&((x)->adr_last_changed), (y), sizeof(vtss_timestamp_t))
#define IP2_ITER_INTF_IFADR_ROW_STATUS_SET(x, y)            \
(x)->adr_row_status = (y)
#define IP2_ITER_INTF_IFADR_STORAGE_TYPE_SET(x, y)          \
(x)->adr_storage = (y)

#define VTSS_IP2_ITER_INTF_INFO_FIRST(w, x, y, z)           \
(((w) = vtss_ip2_intf_ifidx_iter_first((x), (y), (z))) == VTSS_OK)
#define VTSS_IP2_ITER_INTF_INFO(w, x, y, z)                 \
(((w) = vtss_ip2_intf_ifidx_iter_get((x), (y), (z))) == VTSS_OK)
#define VTSS_IP2_ITER_INTF_INFO_NEXT(w, x, y, z)            \
(((w) = vtss_ip2_intf_ifidx_iter_next((x), (y), (z))) == VTSS_OK)
#define VTSS_IP2_ITER_INTF4_INFO_FIRST(a, b)                \
VTSS_IP2_ITER_INTF_INFO_NEXT((a), VTSS_IP_TYPE_IPV4, VTSS_VID_NULL, (b))
#define VTSS_IP2_ITER_INTF4_INFO(a, b, c)                   \
VTSS_IP2_ITER_INTF_INFO((a), VTSS_IP_TYPE_IPV4, (b), (c))
#define VTSS_IP2_ITER_INTF4_INFO_NEXT(a, b, c)              \
VTSS_IP2_ITER_INTF_INFO_NEXT((a), VTSS_IP_TYPE_IPV4, (b), (c))
#define VTSS_IP2_ITER_INTF6_INFO_FIRST(a, b)                \
VTSS_IP2_ITER_INTF_INFO_NEXT((a), VTSS_IP_TYPE_IPV6, VTSS_VID_NULL, (b))
#define VTSS_IP2_ITER_INTF6_INFO(a, b, c)                   \
VTSS_IP2_ITER_INTF_INFO((a), VTSS_IP_TYPE_IPV6, (b), (c))
#define VTSS_IP2_ITER_INTF6_INFO_NEXT(a, b, c)              \
VTSS_IP2_ITER_INTF_INFO_NEXT((a), VTSS_IP_TYPE_IPV6, (b), (c))

#define VTSS_IP2_ITER_INTF_ADDR_FIRST(x, y, z)              \
(((x) = vtss_ip2_intf_ifadr_iter_first((y), (z))) == VTSS_OK)
#define VTSS_IP2_ITER_INTF_ADDR(x, y, z)                    \
(((x) = vtss_ip2_intf_ifadr_iter_get((y), (z))) == VTSS_OK)
#define VTSS_IP2_ITER_INTF_ADDR_NEXT(x, y, z)               \
(((x) = vtss_ip2_intf_ifadr_iter_next((y), (z))) == VTSS_OK)
#define VTSS_IP2_ITER_INTF4_ADDR_FIRST(a, b)                \
do {                                                        \
    vtss_ip_addr_t  _iteradr;                               \
    memset(&_iteradr, 0x0, sizeof(vtss_ip_addr_t));         \
    _iteradr.type = VTSS_IP_TYPE_IPV4;                      \
    if (VTSS_IP2_ITER_INTF_ADDR_NEXT((a), &_iteradr, (b))) {\
    }                                                       \
} while (0)
#define VTSS_IP2_ITER_INTF4_ADDR(a, b, c)                   \
do {                                                        \
    vtss_ip_addr_t  _iteradr;                               \
    memset(&_iteradr, 0x0, sizeof(vtss_ip_addr_t));         \
    _iteradr.type = VTSS_IP_TYPE_IPV4;                      \
    _iteradr.addr.ipv4 = (b);                               \
    if (VTSS_IP2_ITER_INTF_ADDR((a), &_iteradr, (c))) {     \
    }                                                       \
} while (0)
#define VTSS_IP2_ITER_INTF4_ADDR_NEXT(a, b, c)              \
do {                                                        \
    vtss_ip_addr_t          _iteradr;                       \
    ip2_iter_intf_ifadr_t   _iterifadr;                     \
    memset(&_iteradr, 0x0, sizeof(vtss_ip_addr_t));         \
    _iteradr.type = VTSS_IP_TYPE_IPV4;                      \
    _iteradr.addr.ipv4 = (b);                               \
    memcpy(&_iterifadr, (c), sizeof(_iterifadr));           \
    if (VTSS_IP2_ITER_INTF_ADDR_NEXT((a), &_iteradr, (c))) {\
        switch ( IP2_ITER_INTF_IFADR_IPA_VERSION((c)) ) {   \
        case VTSS_IP_TYPE_IPV4:                             \
            break;                                          \
        default:                                            \
            (a) = IP2_ERROR_NOTFOUND;                       \
            memcpy((c), &_iterifadr, sizeof(_iterifadr));   \
            break;                                          \
        }                                                   \
    }                                                       \
} while (0)
#define VTSS_IP2_ITER_INTF6_ADDR_FIRST(a, b)                \
do {                                                        \
    vtss_ip_addr_t  _iteradr;                               \
    memset(&_iteradr, 0x0, sizeof(vtss_ip_addr_t));         \
    _iteradr.type = VTSS_IP_TYPE_IPV6;                      \
    if (VTSS_IP2_ITER_INTF_ADDR_NEXT((a), &_iteradr, (b))) {\
    }                                                       \
} while (0)
#define VTSS_IP2_ITER_INTF6_ADDR(a, b, c)                   \
do {                                                        \
    vtss_ip_addr_t  _iteradr;                               \
    _iteradr.type = VTSS_IP_TYPE_IPV6;                      \
    memcpy(&_iteradr.addr.ipv6, (b), sizeof(vtss_ipv6_t));  \
    if (VTSS_IP2_ITER_INTF_ADDR((a), &_iteradr, (c))) {     \
    }                                                       \
} while (0)
#define VTSS_IP2_ITER_INTF6_ADDR_NEXT(a, b, c)              \
do {                                                        \
    vtss_ip_addr_t  _iteradr;                               \
    _iteradr.type = VTSS_IP_TYPE_IPV6;                      \
    memcpy(&_iteradr.addr.ipv6, (b), sizeof(vtss_ipv6_t));  \
    if (VTSS_IP2_ITER_INTF_ADDR_NEXT((a), &_iteradr, (c))) {\
    }                                                       \
} while (0)

typedef struct {
    vtss_if_id_vlan_t           ifidx;              /* INDEX */
    vtss_ip_addr_t              nbr;                /* INDEX */

    i8                          if_name[IF_NAMESIZE + 1];
    vtss_mac_t                  nbr_phy_address;    /* physical address (MAC address) of neighbor */
    ip2_iter_ifnbr_type_t       nbr_type;           /* neighbor type */
    ip2_iter_ifnbr_state_t      nbr_state;          /* neighbor state */
    vtss_timestamp_t            nbr_last_updated;   /* time stamp whenever neighbor is updated */
    u32                         nbr_row_status;     /* row status */
} ip2_iter_intf_nbr_t;

#define IP2_ITER_INTF_NBR_IFIDX(x)                          \
((x)->ifidx)
#define IP2_ITER_INTF_NBR_IFIDX_SET(x, y)                   \
(x)->ifidx = (y)
#define IP2_ITER_INTF_NBR_VERSION(x)                        \
((x)->nbr.type)
#define IP2_ITER_INTF_NBR_VERSION_SET(x, y)                 \
(x)->nbr.type = (y)
#define IP2_ITER_INTF_NBR_IPV4_ADDR(x)                      \
(IP2_ITER_INTF_NBR_VERSION((x)) == VTSS_IP_TYPE_IPV4) ? ((x)->nbr.addr.ipv4) : 0
#define IP2_ITER_INTF_NBR_IPV4_ADDR_SET(x, y)               \
do {                                                        \
    (x)->nbr.type = VTSS_IP_TYPE_IPV4;                      \
    (x)->nbr.addr.ipv4 = (y);                               \
} while (0)
#define IP2_ITER_INTF_NBR_IPV6_ADDR(x)                      \
(IP2_ITER_INTF_NBR_VERSION((x)) == VTSS_IP_TYPE_IPV6) ? (&((x)->nbr.addr.ipv6)) : NULL
#define IP2_ITER_INTF_NBR_IPV6_ADDR_SET(x, y)               \
do {                                                        \
    (x)->nbr.type = VTSS_IP_TYPE_IPV6;                      \
    memcpy(&(x)->nbr.addr.ipv6, (y), sizeof(vtss_ipv6_t));  \
} while (0)

#define IP2_ITER_INTF_NBR_PHY_ADDR(x)                       \
(&((x)->nbr_phy_address))
#define IP2_ITER_INTF_NBR_TYPE(x)                           \
((x)->nbr_type)
#define IP2_ITER_INTF_NBR_STATE(x)                          \
((x)->nbr_state)
#define IP2_ITER_INTF_NBR_LAST_UPDATE(x)                    \
(&((x)->nbr_last_updated))
#define IP2_ITER_INTF_NBR_ROW_STATUS(x)                     \
((x)->nbr_row_status)

#define IP2_ITER_INTF_NBR_PHY_ADDR_SET(x, y)                \
memcpy(&((x)->nbr_phy_address), (y), sizeof(vtss_mac_t))
#define IP2_ITER_INTF_NBR_TYPE_SET(x, y)                    \
(x)->nbr_type = (y)
#define IP2_ITER_INTF_NBR_STATE_SET(x, y)                   \
(x)->nbr_state = (y)
#define IP2_ITER_INTF_NBR_LAST_UPDATE_SET(x, y)             \
memcpy(&((x)->nbr_last_updated), (y), sizeof(vtss_timestamp_t))
#define IP2_ITER_INTF_NBR_ROW_STATUS_SET(x, y)              \
(x)->nbr_row_status = (y)


/*
    Return first interface general information found in IP stack.

    \param version (IN) - version to use as input key.
    \param vidx (IN) - vlan index to use as input key.

    \param entry (OUT) - general information of the matched IPv4 or IPv6 interfrace.

    \return VTSS_OK iff entry is found.
 */
vtss_rc vtss_ip2_intf_ifidx_iter_first( const vtss_ip_type_t        version,
                                        const vtss_if_id_vlan_t     vidx,
                                        ip2_iter_intf_ifinf_t       *const entry);

/*
    Return specific interface general information found in IP stack.

    \param version (IN) - version to use as input key.
    \param vidx (IN) - vlan index to use as input key.

    \param entry (OUT) - general information of the matched IPv4 or IPv6 interfrace.

    \return VTSS_OK iff entry is found.
 */
vtss_rc vtss_ip2_intf_ifidx_iter_get(   const vtss_ip_type_t        version,
                                        const vtss_if_id_vlan_t     vidx,
                                        ip2_iter_intf_ifinf_t       *const entry);

/*
    Return next interface general information found in IP stack.

    \param version (IN) - version to use as input key.
    \param vidx (IN) - vlan index to use as input key.

    \param entry (OUT) - general information of the matched IPv4 or IPv6 interfrace.

    \return VTSS_OK iff entry is found.
 */
vtss_rc vtss_ip2_intf_ifidx_iter_next(  const vtss_ip_type_t        version,
                                        const vtss_if_id_vlan_t     vidx,
                                        ip2_iter_intf_ifinf_t       *const entry);

/*
    Return first interface address information found in IP stack.

    \param ifadr (IN) - version and address (defined in vtss_ip_addr_t) to use as input key.

    \param entry (OUT) - address information of the matched IPv4 or IPv6 interfrace.

    \return VTSS_OK iff entry is found.
 */
vtss_rc vtss_ip2_intf_ifadr_iter_first( const vtss_ip_addr_t        *ifadr,
                                        ip2_iter_intf_ifadr_t       *const entry);

/*
    Return specific interface address information found in IP stack.

    \param ifadr (IN) - version and address (defined in vtss_ip_addr_t) to use as input key.

    \param entry (OUT) - address information of the matched IPv4 or IPv6 interfrace.

    \return VTSS_OK iff entry is found.
 */
vtss_rc vtss_ip2_intf_ifadr_iter_get(   const vtss_ip_addr_t        *ifadr,
                                        ip2_iter_intf_ifadr_t       *const entry);

/*
    Return next interface address information found in IP stack.

    \param ifadr (IN) - version and address (defined in vtss_ip_addr_t) to use as input key.

    \param entry (OUT) - address information of the matched IPv4 or IPv6 interfrace.

    \return VTSS_OK iff entry is found.
 */
vtss_rc vtss_ip2_intf_ifadr_iter_next(  const vtss_ip_addr_t        *ifadr,
                                        ip2_iter_intf_ifadr_t       *const entry);


/*
    Return first statistics found in IP stack.

    \param version (IN) - version to use as input key.

    \param entry (OUT) - statistics of either IPv4 or IPv6.

    \return VTSS_OK iff entry is found.
 */
vtss_rc vtss_ip2_cntr_syst_stat_iter_first( const vtss_ip_type_t        *version,
                                            vtss_ips_ip_stat_t          *const entry);

/*
    Return specific statistics found in IP stack.

    \param version (IN) - version to use as input key.

    \param entry (OUT) - statistics of either IPv4 or IPv6.

    \return VTSS_OK iff entry is found.
 */
vtss_rc vtss_ip2_cntr_syst_stat_iter_get(   const vtss_ip_type_t        *version,
                                            vtss_ips_ip_stat_t          *const entry);

/*
    Return next statistics found in IP stack.

    \param version (IN) - version to use as input key.

    \param entry (OUT) - statistics of either IPv4 or IPv6.

    \return VTSS_OK iff entry is found.
 */
vtss_rc vtss_ip2_cntr_syst_stat_iter_next(  const vtss_ip_type_t        *version,
                                            vtss_ips_ip_stat_t          *const entry);

/*
    Return first interface statistics found in IP stack.

    \param version (IN) - version to use as input key.
    \param vidx (IN) - vlan index to use as input key.

    \param entry (OUT) - statistics of the matched IPv4 or IPv6 interfrace.

    \return VTSS_OK iff entry is found.
 */
vtss_rc vtss_ip2_cntr_intf_stat_iter_first( const vtss_ip_type_t        *version,
                                            const vtss_if_id_vlan_t     *vidx,
                                            vtss_if_status_ip_stat_t    *const entry);

/*
    Return specific interface statistics found in IP stack.

    \param version (IN) - version to use as input key.
    \param vidx (IN) - vlan index to use as input key.

    \param entry (OUT) - statistics of the matched IPv4 or IPv6 interfrace.

    \return VTSS_OK iff entry is found.
 */
vtss_rc vtss_ip2_cntr_intf_stat_iter_get(   const vtss_ip_type_t        *version,
                                            const vtss_if_id_vlan_t     *vidx,
                                            vtss_if_status_ip_stat_t    *const entry);

/*
    Return next interface statistics found in IP stack.

    \param version (IN) - version to use as input key.
    \param vidx (IN) - vlan index to use as input key.

    \param entry (OUT) - statistics of the matched IPv4 or IPv6 interfrace.

    \return VTSS_OK iff entry is found.
 */
vtss_rc vtss_ip2_cntr_intf_stat_iter_next(  const vtss_ip_type_t        *version,
                                            const vtss_if_id_vlan_t     *vidx,
                                            vtss_if_status_ip_stat_t    *const entry);

/*
    Return first ICMP statistics found in IP stack.

    \param version (IN) - version to use as input key.

    \param entry (OUT) - statistics of either ICMP4 or ICMP6.

    \return VTSS_OK iff entry is found.
 */
vtss_rc vtss_ip2_cntr_icmp_ver_iter_first(  const vtss_ip_type_t        *version,
                                            vtss_ips_icmp_stat_t        *const entry);

/*
    Return specific ICMP statistics found in IP stack.

    \param version (IN) - version to use as input key.

    \param entry (OUT) - statistics of either ICMP4 or ICMP6.

    \return VTSS_OK iff entry is found.
 */
vtss_rc vtss_ip2_cntr_icmp_ver_iter_get(    const vtss_ip_type_t        *version,
                                            vtss_ips_icmp_stat_t        *const entry);

/*
    Return next ICMP statistics found in IP stack.

    \param version (IN) - version to use as input key.

    \param entry (OUT) - statistics of either ICMP4 or ICMP6.

    \return VTSS_OK iff entry is found.
 */
vtss_rc vtss_ip2_cntr_icmp_ver_iter_next(   const vtss_ip_type_t        *version,
                                            vtss_ips_icmp_stat_t        *const entry);

/*
    Return first ICMP MSG statistics found in IP stack.

    \param version (IN) - version to use as input key.
    \param message (IN) - message type to use as input key.

    \param entry (OUT) - statistics of matched ICMP4 or ICMP6 message.

    \return VTSS_OK iff entry is found.
 */
vtss_rc vtss_ip2_cntr_icmp_msg_iter_first(  const vtss_ip_type_t        *version,
                                            const u32                   *message,
                                            vtss_ips_icmp_stat_t        *const entry);

/*
    Return specific ICMP MSG statistics found in IP stack.

    \param version (IN) - version to use as input key.
    \param message (IN) - message type to use as input key.

    \param entry (OUT) - statistics of matched ICMP4 or ICMP6 message.

    \return VTSS_OK iff entry is found.
 */
vtss_rc vtss_ip2_cntr_icmp_msg_iter_get(    const vtss_ip_type_t        *version,
                                            const u32                   *message,
                                            vtss_ips_icmp_stat_t        *const entry);

/*
    Return next ICMP MSG statistics found in IP stack.

    \param version (IN) - version to use as input key.
    \param message (IN) - message type to use as input key.

    \param entry (OUT) - statistics of matched ICMP4 or ICMP6 message.

    \return VTSS_OK iff entry is found.
 */
vtss_rc vtss_ip2_cntr_icmp_msg_iter_next(   const vtss_ip_type_t        *version,
                                            const u32                   *message,
                                            vtss_ips_icmp_stat_t        *const entry);

/*
    Return first interface neighbor information found in IP stack.

    \param vidx (IN) - vlan index to use as input key.
    \param nbra (IN) - neighbor address to use as input key.

    \param entry (OUT) - general information of the matched IPv4 or IPv6 neighbor.

    \return VTSS_OK iff entry is found.
 */
vtss_rc vtss_ip2_intf_nbr_iter_first(   const vtss_if_id_vlan_t     vidx,
                                        const vtss_ip_addr_t       *nbra,
                                        ip2_iter_intf_nbr_t        *const entry);

/*
    Return specific interface neighbor information found in IP stack.

    \param vidx (IN) - vlan index to use as input key.
    \param nbra (IN) - neighbor address to use as input key.

    \param entry (OUT) - general information of the matched IPv4 or IPv6 neighbor.

    \return VTSS_OK iff entry is found.
 */
vtss_rc vtss_ip2_intf_nbr_iter_get(     const vtss_if_id_vlan_t     vidx,
                                        const vtss_ip_addr_t       *nbra,
                                        ip2_iter_intf_nbr_t        *const entry);

/*
    Return next interface neighbor information found in IP stack.

    \param vidx (IN) - vlan index to use as input key.
    \param nbra (IN) - neighbor address to use as input key.

    \param entry (OUT) - general information of the matched IPv4 or IPv6 neighbor.

    \return VTSS_OK iff entry is found.
 */
vtss_rc vtss_ip2_intf_nbr_iter_next(    const vtss_if_id_vlan_t     vidx,
                                        const vtss_ip_addr_t       *nbra,
                                        ip2_iter_intf_nbr_t        *const entry);


#endif /* _IP2_ITERATORS_H_ */

