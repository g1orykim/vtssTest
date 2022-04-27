/*

 Vitesse Switch Software.

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

#include "main.h"

#include "vtss_api.h"
#include "critd_api.h"
#include "mgmt_api.h"
#include "misc_api.h"

#include "syslog_api.h"

#include "ipmc_lib.h"
#include "ipmc_lib_porting.h"

/* ************************************************************************ **
 *
 * Defines
 *
 * ************************************************************************ */
#define VTSS_TRACE_MODULE_ID    VTSS_MODULE_ID_IPMC_LIB
#define VTSS_ALLOC_MODULE_ID    VTSS_MODULE_ID_IPMC_LIB

/* ************************************************************************ **
 *
 * Public data
 *
 * ************************************************************************ */


/* ************************************************************************ **
 *
 * Local data
 *
 * ************************************************************************ */
static BOOL                     ipmc_lib_ptg_done_init = FALSE;
static critd_t                  ipmc_ptg_crit;
static u8                       mc6_ctrl_flood_set;
static BOOL                     flooding4_member[VTSS_PORT_ARRAY_SIZE];
static BOOL                     flooding6_member[VTSS_PORT_ARRAY_SIZE];

#if VTSS_TRACE_ENABLED
#define IPMC_PTG_CRIT_ENTER()   critd_enter(&ipmc_ptg_crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define IPMC_PTG_CRIT_EXIT()    critd_exit(&ipmc_ptg_crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#else
#define IPMC_PTG_CRIT_ENTER()   critd_enter(&ipmc_ptg_crit)
#define IPMC_PTG_CRIT_EXIT()    critd_exit(&ipmc_ptg_crit)
#endif /* VTSS_TRACE_ENABLED */

static void                     *vtss_igmp_filter_id = NULL;    // Filter id for subscribing igmp packet.
static void                     *vtss_mld_filter_id = NULL;     // Filter id for subscribing mld packet.

static u8                       *ipmc_lib_mem_area[IPMC_LIB_MEM_MAX_POOL];
static cyg_mempool_var          ipmc_lib_mem_pool_var[IPMC_LIB_MEM_MAX_POOL];
static cyg_mempool_fix          ipmc_lib_mem_pool_fix[IPMC_LIB_MEM_MAX_POOL];
static cyg_handle_t             ipmc_lib_memory[IPMC_LIB_MEM_MAX_POOL];

#define IPMC_LIB_TIME_USE_CLOCK 1


BOOL ipmc_lib_time_curr_get(ipmc_time_t *now_t)
{
#if IPMC_LIB_TIME_USE_CLOCK
#ifdef VTSS_ARCH_LUTON28
    struct timespec curr_time;

    if (!now_t) {
        return FALSE;
    }

    if (clock_gettime(CLOCK_MONOTONIC, &curr_time)) {
        memset(now_t, 0x0, sizeof(ipmc_time_t));
        return FALSE;
    }

    now_t->sec = curr_time.tv_sec;
    now_t->msec = curr_time.tv_nsec / IPMC_LIB_TIME_REF_MSEC_VAL;
    now_t->usec = (curr_time.tv_nsec % IPMC_LIB_TIME_REF_MSEC_VAL) / IPMC_LIB_TIME_REF_USEC_VAL;
//    now_t->nsec = curr_time.tv_nsec % IPMC_LIB_TIME_REF_USEC_VAL;
#else
    cyg_uint64  ssec, msec, usec;

    if (!now_t) {
        return FALSE;
    }

    usec = hal_time_get();
    now_t->sec = ssec = (usec / 1000000ULL);
    now_t->msec = msec = (usec - (1000000ULL * ssec)) / 1000ULL;
    now_t->usec = (usec - (1000000ULL * ssec) - (1000ULL * msec));
//    now_t->nsec = 0;
#endif /* VTSS_ARCH_LUTON28 */
#else
    cyg_tick_count_t    curr_time;

    if (!now_t) {
        return FALSE;
    }

    curr_time = cyg_current_time() * ECOS_MSECS_PER_HWTICK;
    now_t->sec = curr_time / IPMC_LIB_TIME_MSEC_BASE;
    now_t->msec = curr_time % IPMC_LIB_TIME_MSEC_BASE;
    now_t->usec = 0;
//    now_t->usec = now_t->nsec = 0;
#endif /* IPMC_LIB_TIME_USE_CLOCK */

    return TRUE;
}

BOOL ipmc_lib_time_diff_get(BOOL prt, BOOL info_prt, char *str, ipmc_time_t *base_t, ipmc_time_t *diff_t)
{
#if IPMC_LIB_TIME_USE_CLOCK
    ipmc_time_t now_t;
    BOOL        warp_t;
    u32         under_sec_calc;

    if (!base_t || !diff_t) {
        return FALSE;
    }

    if (!ipmc_lib_time_curr_get(&now_t)) {
        return FALSE;
    }

    warp_t = FALSE;
    if (base_t->sec > now_t.sec) {
        warp_t = TRUE;

        memset(&diff_t->sec, 0xFF, sizeof(u32));
        diff_t->sec = diff_t->sec - base_t->sec + 1;
    } else if (base_t->sec == now_t.sec) {
        under_sec_calc = base_t->msec * IPMC_LIB_TIME_REF_MSEC_VAL + base_t->usec * IPMC_LIB_TIME_REF_USEC_VAL;
//        under_sec_calc = base_t->msec * IPMC_LIB_TIME_REF_MSEC_VAL + base_t->usec * IPMC_LIB_TIME_REF_USEC_VAL + base_t->nsec;

        if (under_sec_calc > (now_t.msec * IPMC_LIB_TIME_REF_MSEC_VAL + now_t.usec * IPMC_LIB_TIME_REF_USEC_VAL)) {
//        if (under_sec_calc > (now_t.msec * IPMC_LIB_TIME_REF_MSEC_VAL + now_t.usec * IPMC_LIB_TIME_REF_USEC_VAL + now_t.nsec)) {
            /* OVER 136 years?! */
            memset(diff_t, 0xFF, sizeof(ipmc_time_t));
            T_W("%s consumes OVER 136 years?!", str ? str : "Caller");
            return FALSE;
        } else {
            diff_t->sec = 0;
        }
    } else {
        diff_t->sec = now_t.sec - base_t->sec;
    }

    if (diff_t->sec) {
        under_sec_calc = IPMC_LIB_TIME_REF_UNIT_VAL;
        diff_t->sec--;

        under_sec_calc -= (base_t->msec * IPMC_LIB_TIME_REF_MSEC_VAL + base_t->usec * IPMC_LIB_TIME_REF_USEC_VAL);
        under_sec_calc += (now_t.msec * IPMC_LIB_TIME_REF_MSEC_VAL + now_t.usec * IPMC_LIB_TIME_REF_USEC_VAL);
//        under_sec_calc -= (base_t->msec * IPMC_LIB_TIME_REF_MSEC_VAL + base_t->usec * IPMC_LIB_TIME_REF_USEC_VAL + base_t->nsec);
//        under_sec_calc += (now_t.msec * IPMC_LIB_TIME_REF_MSEC_VAL + now_t.usec * IPMC_LIB_TIME_REF_USEC_VAL + now_t.nsec);

        if (under_sec_calc >= IPMC_LIB_TIME_REF_UNIT_VAL) {
            diff_t->sec += under_sec_calc / IPMC_LIB_TIME_REF_UNIT_VAL;
            under_sec_calc = under_sec_calc % IPMC_LIB_TIME_REF_UNIT_VAL;
        }

        if (warp_t) {
            diff_t->sec += now_t.sec;
        }
    } else {
        under_sec_calc = (now_t.msec * IPMC_LIB_TIME_REF_MSEC_VAL + now_t.usec * IPMC_LIB_TIME_REF_USEC_VAL);
        under_sec_calc -= (base_t->msec * IPMC_LIB_TIME_REF_MSEC_VAL + base_t->usec * IPMC_LIB_TIME_REF_USEC_VAL);
//        under_sec_calc = (now_t.msec * IPMC_LIB_TIME_REF_MSEC_VAL + now_t.usec * IPMC_LIB_TIME_REF_USEC_VAL + now_t.nsec);
//        under_sec_calc -= (base_t->msec * IPMC_LIB_TIME_REF_MSEC_VAL + base_t->usec * IPMC_LIB_TIME_REF_USEC_VAL + base_t->nsec);
    }
    diff_t->msec = under_sec_calc / IPMC_LIB_TIME_REF_MSEC_VAL;
    diff_t->usec = (under_sec_calc % IPMC_LIB_TIME_REF_MSEC_VAL) / IPMC_LIB_TIME_REF_USEC_VAL;
//    diff_t->nsec = under_sec_calc % IPMC_LIB_TIME_REF_USEC_VAL;
#else
    cyg_tick_count_t    curr_time, base_time, diff_time;

    if (!base_t || !diff_t) {
        return FALSE;
    }

    curr_time = cyg_current_time() * ECOS_MSECS_PER_HWTICK;
    base_time = base_t->sec;
    base_time = base_time * IPMC_LIB_TIME_MSEC_BASE + base_t->msec;

    if (base_time > curr_time) {
        memset(&diff_time, 0xFF, sizeof(cyg_tick_count_t));
        diff_time = diff_time - base_time + 1 + curr_time;
    } else {
        diff_time = curr_time - base_time;
    }

    diff_t->sec = diff_time / IPMC_LIB_TIME_MSEC_BASE;
    diff_t->msec = diff_time % IPMC_LIB_TIME_MSEC_BASE;
//    diff_t->usec = diff_t->nsec = 0;
#endif /* IPMC_LIB_TIME_USE_CLOCK */

    if (prt &&
        (diff_t->sec || diff_t->msec || diff_t->usec)) {
        if (info_prt) {
            T_I("%s consumes %u.%um%uu",
                str ? str : "Caller",
                diff_t->sec, diff_t->msec, diff_t->usec);
        } else {
            T_D("%s consumes %u.%um%uu",
                str ? str : "Caller",
                diff_t->sec, diff_t->msec, diff_t->usec);
        }
    }

    return TRUE;
}

BOOL ipmc_lib_memory_initialize(ipmc_mem_alloc_t type,
                                u8 *pool_idx,
                                size_t total_sz,
                                size_t entry_sz)
{
    u8      idx;
    BOOL    idx_fnd;
    size_t  pool_size;

    if (!ipmc_lib_ptg_done_init || !pool_idx || !total_sz) {
        return FALSE;
    }

    *pool_idx = 0;  /* idx 0 reserved to indicate IPMC_MEM_SYS_MALLOC */
    if ((type != IPMC_MEM_PARTITIONED) &&
        (type != IPMC_MEM_DYNA_POOL)) {
        return TRUE;
    }

    idx_fnd = FALSE;
    pool_size = total_sz;
    if (type == IPMC_MEM_PARTITIONED) {
        if ((total_sz / entry_sz) < 2) {
            pool_size = 2 * entry_sz;
        }
    }
    pool_size = pool_size + sizeof(cyg_handle_t);
    pool_size = sizeof(int) * ((pool_size + 3) / sizeof(int));
    for (idx = 1; idx < IPMC_LIB_MEM_MAX_POOL; idx++) {
        if (ipmc_lib_mem_area[idx] == NULL) {
            ipmc_lib_mem_area[idx] = VTSS_MALLOC(pool_size);
            if (ipmc_lib_mem_area[idx]) {
                idx_fnd = TRUE;
            }

            break;
        }
    }

    if (!idx_fnd || (idx >= IPMC_LIB_MEM_MAX_POOL)) {
        return FALSE;
    }

    *pool_idx = idx;
    if (type == IPMC_MEM_PARTITIONED) {
        cyg_mempool_fix_create(ipmc_lib_mem_area[idx],
                               pool_size,
                               entry_sz,
                               &ipmc_lib_memory[idx],
                               &ipmc_lib_mem_pool_fix[idx]);
    } else {
        cyg_mempool_var_create(ipmc_lib_mem_area[idx],
                               pool_size,
                               &ipmc_lib_memory[idx],
                               &ipmc_lib_mem_pool_var[idx]);
    }

    return TRUE;
}

u8 *ipmc_lib_memory_allocate(ipmc_mem_alloc_t type, u8 pool_idx, size_t size)
{
    if (type == IPMC_MEM_PARTITIONED) {
        if (!pool_idx) {
            return NULL;
        }

        return cyg_mempool_fix_try_alloc(ipmc_lib_memory[pool_idx]);
    } else if (type == IPMC_MEM_DYNA_POOL) {
        if (!size || !pool_idx) {
            return NULL;
        }

        return cyg_mempool_var_try_alloc(ipmc_lib_memory[pool_idx], size);
    } else {
        if (!size || pool_idx) {
            return NULL;
        }

        return VTSS_MALLOC(size);
    }
}

BOOL ipmc_lib_memory_free(ipmc_mem_alloc_t type, u8 pool_idx, u8 *ptr)
{
    if (!ptr) {
        return FALSE;
    }

    if (type == IPMC_MEM_PARTITIONED) {
        if (!pool_idx) {
            return FALSE;
        }

        cyg_mempool_fix_free(ipmc_lib_memory[pool_idx], ptr);
    } else if (type == IPMC_MEM_DYNA_POOL) {
        if (!pool_idx) {
            return FALSE;
        }

        cyg_mempool_var_free(ipmc_lib_memory[pool_idx], ptr);
    } else {
        if (pool_idx) {
            return FALSE;
        }

        VTSS_FREE(ptr);
    }

    return TRUE;
}

BOOL ipmc_lib_memory_info_get(ipmc_mem_alloc_t type, u8 pool_idx, ipmc_lib_memory_info_t *info)
{
    if (!info) {
        return FALSE;
    }

    if (type == IPMC_MEM_PARTITIONED) {
        if (!pool_idx) {
            return FALSE;
        }

        cyg_mempool_fix_get_info(ipmc_lib_memory[pool_idx], info);
    } else if (type == IPMC_MEM_DYNA_POOL) {
        if (!pool_idx) {
            return FALSE;
        }

        cyg_mempool_var_get_info(ipmc_lib_memory[pool_idx], info);
    } else {
        if (pool_idx) {
            return FALSE;
        }

        memset(info, 0x0, sizeof(ipmc_lib_memory_info_t));
    }

    return TRUE;
}

#if defined(VTSS_FEATURE_IPV4_MC_SIP) || defined(VTSS_FEATURE_IPV6_MC_SIP)
static vtss_rc  _ipmc_lib_porting_do_chip_sfm(BOOL op, BOOL is_sfm,
                                              ipmc_group_entry_t *grp_op,
                                              vtss_ipv4_t ip4sip, vtss_ipv4_t ip4dip,
                                              vtss_ipv6_t ip6sip, vtss_ipv6_t ip6dip,
                                              BOOL fwd_map[VTSS_PORT_ARRAY_SIZE])
{
    ipmc_ip_version_t   version;
    vtss_vid_t          ifid;
    vtss_rc             rc;

    if (!grp_op) {
        return VTSS_RC_ERROR;
    }

    version = grp_op->ipmc_version;
    ifid = grp_op->vid;
    rc = VTSS_OK;

    if (op) {   /* Add or Update */
        if ((version == IPMC_IP_VERSION_MLD) || (version == IPMC_IP_VERSION_IPV6Z)) {
#ifdef VTSS_FEATURE_IPV6_MC_SIP
            if ((rc = vtss_ipv6_mc_add(NULL, ifid, ip6sip, ip6dip, fwd_map)) != VTSS_OK) {
                T_D("vtss_ipv6_mc_add failed");
            }
#endif /* VTSS_FEATURE_IPV6_MC_SIP */
        } else if ((version == IPMC_IP_VERSION_IGMP) || (version == IPMC_IP_VERSION_IPV4Z)) {
#ifdef VTSS_FEATURE_IPV4_MC_SIP
            if ((rc = vtss_ipv4_mc_add(NULL, ifid, ntohl(ip4sip), ntohl(ip4dip), fwd_map)) != VTSS_OK) {
                T_D("vtss_ipv4_mc_add failed");
            }
#endif /* VTSS_FEATURE_IPV4_MC_SIP */
        } else {
            rc = VTSS_RC_ERROR;
            T_D("Version-%d ADD failed", version);
        }
    } else {    /* Delete */
        ipmc_group_db_t *grp_db = &grp_op->info->db;

        if (is_sfm) {
            BOOL                in_chip;
            ipmc_sfm_srclist_t  *ptr, check;

            memset(&check, 0x0, sizeof(ipmc_sfm_srclist_t));
            if ((version == IPMC_IP_VERSION_MLD) || (version == IPMC_IP_VERSION_IPV6Z)) {
                memcpy(&check.src_ip, &ip6sip, sizeof(vtss_ipv6_t));
            } else if ((version == IPMC_IP_VERSION_IGMP) || (version == IPMC_IP_VERSION_IPV4Z)) {
                memcpy(&check.src_ip.addr[12], (u8 *)&ip4sip, sizeof(vtss_ipv4_t));
            } else {
                rc = VTSS_RC_ERROR;
                T_D("Version-%d DEL failed", version);
                return rc;
            }

            in_chip = FALSE;
            ptr = &check;
            if (IPMC_LIB_DB_GET(grp_db->ipmc_sf_do_forward_srclist, ptr)) {
                in_chip = ptr->sfm_in_hw;
            }
            if (!in_chip) {
                ptr = &check;
                if (IPMC_LIB_DB_GET(grp_db->ipmc_sf_do_not_forward_srclist, ptr)) {
                    in_chip = ptr->sfm_in_hw;
                }
            }

            if (!in_chip) {
                return rc;
            }
        } else {
            if (!grp_db->asm_in_hw) {
                return rc;
            }
        }

        if ((version == IPMC_IP_VERSION_MLD) || (version == IPMC_IP_VERSION_IPV6Z)) {
#ifdef VTSS_FEATURE_IPV6_MC_SIP
            if ((rc = vtss_ipv6_mc_del(NULL, ifid, ip6sip, ip6dip)) != VTSS_OK) {
                T_D("vtss_ipv6_mc_del failed");
            }
#endif /* VTSS_FEATURE_IPV6_MC_SIP */
        } else if ((version == IPMC_IP_VERSION_IGMP) || (version == IPMC_IP_VERSION_IPV4Z)) {
#ifdef VTSS_FEATURE_IPV4_MC_SIP
            if ((rc = vtss_ipv4_mc_del(NULL, ifid, ntohl(ip4sip), ntohl(ip4dip))) != VTSS_OK) {
                T_D("vtss_ipv4_mc_del failed");
            }
#endif /* VTSS_FEATURE_IPV4_MC_SIP */
        } else {
            rc = VTSS_RC_ERROR;
            T_D("Version-%d DEL failed", version);
        }
    }

    return rc;
}
#endif /* defined(VTSS_FEATURE_IPV4_MC_SIP) || defined(VTSS_FEATURE_IPV6_MC_SIP) */

static char                     bufPort[MGMT_PORT_BUF_SIZE];
static char                     bufSip4[40], bufDip4[40];
static char                     bufSip6[40], bufDip6[40];

vtss_rc ipmc_lib_porting_set_chip(BOOL op, ipmc_db_ctrl_hdr_t *p,
                                  ipmc_group_entry_t *grp_op,
                                  ipmc_ip_version_t version, vtss_vid_t ifid,
                                  vtss_ipv4_t ip4sip, vtss_ipv4_t ip4dip,
                                  vtss_ipv6_t ip6sip, vtss_ipv6_t ip6dip,
                                  BOOL fwd_map[VTSS_PORT_ARRAY_SIZE])
{
    vtss_rc                 rc;
    ipmc_group_db_t         *grp_db;
    ipmc_sfm_srclist_t      *element_allow, *element_deny, *check, *check_bak;
    vtss_ipv4_t             rsrvd_mac;
    ipmc_port_bfs_t         rp4_bitmask, rp6_bitmask;
    BOOL                    is_sfm, bypassing;
    BOOL                    api_ssm4, api_ssm6;
    u32                     i, local_port_cnt;
#if !defined(VTSS_FEATURE_IPV4_MC_SIP) || !defined(VTSS_FEATURE_IPV6_MC_SIP)
    ipmc_group_entry_t      *grp, *grp_ptr;

    BOOL                    fwd_found, hash_found;
    vtss_ipv4_t             ip2mac;
    vtss_vid_mac_t          vid_mac_entry;
    vtss_mac_table_entry_t  mac_table_entry;
    u32                     ipHashChk1, ipHashChk2;
#endif /* !defined(VTSS_FEATURE_IPV4_MC_SIP) || !defined(VTSS_FEATURE_IPV6_MC_SIP) */

    if (!p || !grp_op) {
        return VTSS_RC_ERROR;
    }

    rc = VTSS_OK;
    local_port_cnt = ipmc_lib_get_system_local_port_cnt();

    is_sfm = FALSE;
    if (version == IPMC_IP_VERSION_MLD) {
        if (!ipmc_lib_isaddr6_all_zero((vtss_ipv6_t *)&ip6sip)) {
            is_sfm = TRUE;
        }
    }
    if (version == IPMC_IP_VERSION_IGMP) {
        if (!ipmc_lib_isaddr4_all_zero((ipmcv4addr *)&ip4sip)) {
            is_sfm = TRUE;
        }
    }

    T_D("%s-%s Ver/IfId=%s/%u, SIP4=%s, DIP4=%s, SIP6=%s, DIP6=%s, Ports=%s",
        op ? "ADD" : "DEL",
        is_sfm ? "SFM" : "ASM",
        ipmc_lib_version_txt(version, IPMC_TXT_CASE_UPPER), ifid,
        misc_ipv4_txt(htonl(ip4sip), bufSip4),
        misc_ipv4_txt(htonl(ip4dip), bufDip4),
        misc_ipv6_txt(&ip6sip, bufSip6),
        misc_ipv6_txt(&ip6dip, bufDip6),
        fwd_map ? mgmt_iport_list2txt(fwd_map, bufPort) : "NULL");

    bypassing = FALSE;
    switch ( version ) {
    case IPMC_IP_VERSION_MLD:
    case IPMC_IP_VERSION_IPV6Z:
        memcpy((u8 *)&rsrvd_mac, &ip6dip.addr[12], sizeof(ipmcv4addr));
        rsrvd_mac = ntohl(rsrvd_mac);
        rsrvd_mac = rsrvd_mac << IPMC_IP2MAC_V6SHIFT_LEN;
        rsrvd_mac = rsrvd_mac >> IPMC_IP2MAC_V6SHIFT_LEN;
        /* We should bypass ALL_NODE & ALL_RTR */
        if ((((rsrvd_mac >> IPMC_IP2MAC_ARRAY5_SHIFT_LEN) & IPMC_IP2MAC_ARRAY_MASK) == 0x1) ||
            (((rsrvd_mac >> IPMC_IP2MAC_ARRAY5_SHIFT_LEN) & IPMC_IP2MAC_ARRAY_MASK) == 0x2)) {
            if (!((rsrvd_mac >> IPMC_IP2MAC_ARRAY2_SHIFT_LEN) & IPMC_IP2MAC_ARRAY_MASK) &&
                !((rsrvd_mac >> IPMC_IP2MAC_ARRAY3_SHIFT_LEN) & IPMC_IP2MAC_ARRAY_MASK) &&
                !((rsrvd_mac >> IPMC_IP2MAC_ARRAY4_SHIFT_LEN) & IPMC_IP2MAC_ARRAY_MASK)) {
                bypassing = TRUE;
                T_D("Bypass M:...:0000:000%s",
                    (((rsrvd_mac >> IPMC_IP2MAC_ARRAY5_SHIFT_LEN) & IPMC_IP2MAC_ARRAY_MASK) == 0x1) ?
                    "1 (ALL_NODE)" : "2 (ALL_RTR)");
                break;
            }
        }

#if defined(VTSS_FEATURE_IPV6_MC_SIP)
        if (op && fwd_map) {
            VTSS_PORT_BF_CLR(rp6_bitmask.member_ports);
            ipmc_lib_get_discovered_router_port_mask(IPMC_IP_VERSION_MLD, &rp6_bitmask);

            for (i = 0; i < local_port_cnt; i++) {
                fwd_map[i] |= VTSS_PORT_BF_GET(rp6_bitmask.member_ports, i);
            }
        }

        rc = _ipmc_lib_porting_do_chip_sfm(op, is_sfm, grp_op, ip4sip, ip4dip, ip6sip, ip6dip, fwd_map);
#else
        if (is_sfm) {
            bypassing = TRUE;
            break;
        }

        memcpy((u8 *)&ip2mac, &ip6dip.addr[12], sizeof(ipmcv4addr));
        ip2mac = ntohl(ip2mac);
        ip2mac = ip2mac << IPMC_IP2MAC_V6SHIFT_LEN;
        ip2mac = ip2mac >> IPMC_IP2MAC_V6SHIFT_LEN;
        vid_mac_entry.mac.addr[0] = IPMC_IP2MAC_V6MAC_ARRAY0;
        vid_mac_entry.mac.addr[1] = IPMC_IP2MAC_V6MAC_ARRAY1;
        vid_mac_entry.mac.addr[2] = (ip2mac >> IPMC_IP2MAC_ARRAY2_SHIFT_LEN) & IPMC_IP2MAC_ARRAY_MASK;
        vid_mac_entry.mac.addr[3] = (ip2mac >> IPMC_IP2MAC_ARRAY3_SHIFT_LEN) & IPMC_IP2MAC_ARRAY_MASK;
        vid_mac_entry.mac.addr[4] = (ip2mac >> IPMC_IP2MAC_ARRAY4_SHIFT_LEN) & IPMC_IP2MAC_ARRAY_MASK;
        vid_mac_entry.mac.addr[5] = (ip2mac >> IPMC_IP2MAC_ARRAY5_SHIFT_LEN) & IPMC_IP2MAC_ARRAY_MASK;
        vid_mac_entry.vid = ifid;

        fwd_found = FALSE;
        memset(&mac_table_entry, 0x0, sizeof(vtss_mac_table_entry_t));
        if (vtss_mac_table_get(NULL, &vid_mac_entry, &mac_table_entry) != VTSS_OK) {
            memcpy(&mac_table_entry.vid_mac, &vid_mac_entry, sizeof(vtss_vid_mac_t));
        } else {
            fwd_found = TRUE;
        }
        mac_table_entry.copy_to_cpu = FALSE;
        mac_table_entry.locked = TRUE;
        mac_table_entry.aged = FALSE;

        if (op) {   /* Add or Update */
            if (fwd_map) {
                VTSS_PORT_BF_CLR(rp6_bitmask.member_ports);
                ipmc_lib_get_discovered_router_port_mask(IPMC_IP_VERSION_MLD, &rp6_bitmask);

                for (i = 0; i < local_port_cnt; i++) {
                    mac_table_entry.destination[i] = (fwd_map[i] | VTSS_PORT_BF_GET(rp6_bitmask.member_ports, i));
                }
            }

            if (fwd_found) {
                grp_ptr = NULL;
                while ((grp = ipmc_lib_group_ptr_get_next(p, grp_ptr)) != NULL) {
                    grp_ptr = grp;

                    if (grp->vid != ifid ||
                        grp->ipmc_version != version) {
                        continue;
                    } else {
                        if (!memcmp(&grp->group_addr,  &ip6dip, sizeof(vtss_ipv6_t))) {
                            continue;
                        }
                    }

                    /* Same Hash in MAC? */
                    memcpy((u8 *)&ipHashChk1, &grp->group_addr.addr[12], sizeof(ipmcv4addr));
                    memcpy((u8 *)&ipHashChk2, &ip6dip.addr[12], sizeof(ipmcv4addr));
                    if ((ntohl(ipHashChk1) << IPMC_IP2MAC_V6SHIFT_LEN) !=
                        (ntohl(ipHashChk2) << IPMC_IP2MAC_V6SHIFT_LEN)) {
                        continue;
                    }

                    grp_db = &grp->info->db;
                    for (i = 0; i < local_port_cnt; i++) {
                        mac_table_entry.destination[i] |= VTSS_PORT_BF_GET(grp_db->port_mask, i);
                    }
                }
            } /* if (fwd_found) */

            rc = vtss_mac_table_add(NULL, &mac_table_entry);
        } else {    /* Delete */
            if (fwd_found) {    /* Delete */
                hash_found = FALSE;
                for (i = 0; i < local_port_cnt; i++) {
                    mac_table_entry.destination[i] = FALSE;
                }

                grp_ptr = NULL;
                while ((grp = ipmc_lib_group_ptr_get_next(p, grp_ptr)) != NULL) {
                    grp_ptr = grp;

                    if (grp->vid != ifid ||
                        grp->ipmc_version != version) {
                        continue;
                    } else {
                        if (!memcmp(&grp->group_addr,  &ip6dip, sizeof(vtss_ipv6_t))) {
                            continue;
                        }
                    }

                    /* Same Hash in MAC? */
                    memcpy((u8 *)&ipHashChk1, &grp->group_addr.addr[12], sizeof(ipmcv4addr));
                    memcpy((u8 *)&ipHashChk2, &ip6dip.addr[12], sizeof(ipmcv4addr));
                    if ((ntohl(ipHashChk1) << IPMC_IP2MAC_V6SHIFT_LEN) !=
                        (ntohl(ipHashChk2) << IPMC_IP2MAC_V6SHIFT_LEN)) {
                        continue;
                    }

                    hash_found = TRUE;
                    grp_db = &grp->info->db;
                    for (i = 0; i < local_port_cnt; i++) {
                        mac_table_entry.destination[i] |= VTSS_PORT_BF_GET(grp_db->port_mask, i);
                    }
                }

                if (hash_found) {
                    rc = vtss_mac_table_add(NULL, &mac_table_entry);
                } else {
                    rc = vtss_mac_table_del(NULL, &vid_mac_entry);
                }
            } else {            /* Invalid */
                rc = VTSS_RC_ERROR;
            }
        }
#endif /* defined(VTSS_FEATURE_IPV6_MC_SIP) */

        break;
    case IPMC_IP_VERSION_IGMP:
    case IPMC_IP_VERSION_IPV4Z:
#ifdef VTSS_SW_OPTION_UPNP
        /* Filter UPNP-Like group address X.127.255.250, since UPNP may rely on MAC for TX */
        rsrvd_mac = ntohl(ip4dip);
        rsrvd_mac = rsrvd_mac << IPMC_IP2MAC_V4SHIFT_LEN;
        rsrvd_mac = rsrvd_mac >> IPMC_IP2MAC_V4SHIFT_LEN;
        /* We should bypass UPNP-Like group address */
        if ((((rsrvd_mac >> IPMC_IP2MAC_ARRAY5_SHIFT_LEN) & IPMC_IP2MAC_ARRAY_MASK) == 0xFA) &&
            (((rsrvd_mac >> IPMC_IP2MAC_ARRAY4_SHIFT_LEN) & IPMC_IP2MAC_ARRAY_MASK) == 0xFF) &&
            (((rsrvd_mac >> IPMC_IP2MAC_ARRAY3_SHIFT_LEN) & IPMC_IP2MAC_ARRAY_MASK) == 0x7F)) {
            bypassing = TRUE;
            T_D("Bypass UPNP-Like 01-00-5E-7F-FF-FA");
            break;
        }
#endif /* VTSS_SW_OPTION_UPNP */

#if defined(VTSS_FEATURE_IPV4_MC_SIP)
        if (op && fwd_map) {
            VTSS_PORT_BF_CLR(rp4_bitmask.member_ports);
            ipmc_lib_get_discovered_router_port_mask(IPMC_IP_VERSION_IGMP, &rp4_bitmask);

            for (i = 0; i < local_port_cnt; i++) {
                fwd_map[i] |= VTSS_PORT_BF_GET(rp4_bitmask.member_ports, i);
            }
        }

        rc = _ipmc_lib_porting_do_chip_sfm(op, is_sfm, grp_op, ip4sip, ip4dip, ip6sip, ip6dip, fwd_map);
#else
        if (is_sfm) {
            bypassing = TRUE;
            break;
        }

        ip2mac = ntohl(ip4dip);
        ip2mac = ip2mac << IPMC_IP2MAC_V4SHIFT_LEN;
        ip2mac = ip2mac >> IPMC_IP2MAC_V4SHIFT_LEN;
        vid_mac_entry.mac.addr[0] = IPMC_IP2MAC_V4MAC_ARRAY0;
        vid_mac_entry.mac.addr[1] = IPMC_IP2MAC_V4MAC_ARRAY1;
        vid_mac_entry.mac.addr[2] = IPMC_IP2MAC_V4MAC_ARRAY2;
        vid_mac_entry.mac.addr[3] = (ip2mac >> IPMC_IP2MAC_ARRAY3_SHIFT_LEN) & IPMC_IP2MAC_ARRAY_MASK;
        vid_mac_entry.mac.addr[4] = (ip2mac >> IPMC_IP2MAC_ARRAY4_SHIFT_LEN) & IPMC_IP2MAC_ARRAY_MASK;
        vid_mac_entry.mac.addr[5] = (ip2mac >> IPMC_IP2MAC_ARRAY5_SHIFT_LEN) & IPMC_IP2MAC_ARRAY_MASK;
        vid_mac_entry.vid = ifid;

        fwd_found = FALSE;
        memset(&mac_table_entry, 0x0, sizeof(vtss_mac_table_entry_t));
        if (vtss_mac_table_get(NULL, &vid_mac_entry, &mac_table_entry) != VTSS_OK) {
            memcpy(&mac_table_entry.vid_mac, &vid_mac_entry, sizeof(vtss_vid_mac_t));
        } else {
            fwd_found = TRUE;
        }
        mac_table_entry.copy_to_cpu = FALSE;
        mac_table_entry.locked = TRUE;
        mac_table_entry.aged = FALSE;

        if (op) {   /* Add or Update */
            if (fwd_map) {
                VTSS_PORT_BF_CLR(rp4_bitmask.member_ports);
                ipmc_lib_get_discovered_router_port_mask(IPMC_IP_VERSION_IGMP, &rp4_bitmask);

                for (i = 0; i < local_port_cnt; i++) {
                    mac_table_entry.destination[i] = (fwd_map[i] | VTSS_PORT_BF_GET(rp4_bitmask.member_ports, i));
                }
            }

            if (fwd_found) {
                grp_ptr = NULL;
                while ((grp = ipmc_lib_group_ptr_get_next(p, grp_ptr)) != NULL) {
                    grp_ptr = grp;

                    if (grp->vid != ifid ||
                        grp->ipmc_version != version) {
                        continue;
                    } else {
                        if (!memcmp(&grp->group_addr.addr[12], (u8 *)&ip4dip, sizeof(ipmcv4addr))) {
                            continue;
                        }
                    }

                    /* Same Hash in MAC? */
                    memcpy((u8 *)&ipHashChk1, &grp->group_addr.addr[12], sizeof(ipmcv4addr));
                    memcpy((u8 *)&ipHashChk2, (u8 *)&ip4dip, sizeof(ipmcv4addr));
                    if ((ntohl(ipHashChk1) << IPMC_IP2MAC_V4SHIFT_LEN) !=
                        (ntohl(ipHashChk2) << IPMC_IP2MAC_V4SHIFT_LEN)) {
                        continue;
                    }

                    grp_db = &grp->info->db;
                    for (i = 0; i < local_port_cnt; i++) {
                        mac_table_entry.destination[i] |= VTSS_PORT_BF_GET(grp_db->port_mask, i);
                    }
                }
            } /* if (fwd_found) */

            rc = vtss_mac_table_add(NULL, &mac_table_entry);
        } else {    /* Delete */
            if (fwd_found) {    /* Delete */
                hash_found = FALSE;
                for (i = 0; i < local_port_cnt; i++) {
                    mac_table_entry.destination[i] = FALSE;
                }

                grp_ptr = NULL;
                while ((grp = ipmc_lib_group_ptr_get_next(p, grp_ptr)) != NULL) {
                    grp_ptr = grp;

                    if (grp->vid != ifid ||
                        grp->ipmc_version != version) {
                        continue;
                    } else {
                        if (!memcmp(&grp->group_addr.addr[12], (u8 *)&ip4dip, sizeof(ipmcv4addr))) {
                            continue;
                        }
                    }

                    /* Same Hash in MAC? */
                    memcpy((u8 *)&ipHashChk1, &grp->group_addr.addr[12], sizeof(ipmcv4addr));
                    memcpy((u8 *)&ipHashChk2, (u8 *)&ip4dip, sizeof(ipmcv4addr));
                    if ((ntohl(ipHashChk1) << IPMC_IP2MAC_V4SHIFT_LEN) !=
                        (ntohl(ipHashChk2) << IPMC_IP2MAC_V4SHIFT_LEN)) {
                        continue;
                    }

                    hash_found = TRUE;
                    grp_db = &grp->info->db;
                    for (i = 0; i < local_port_cnt; i++) {
                        mac_table_entry.destination[i] |= VTSS_PORT_BF_GET(grp_db->port_mask, i);
                    }
                }

                if (hash_found) {
                    rc = vtss_mac_table_add(NULL, &mac_table_entry);
                } else {
                    rc = vtss_mac_table_del(NULL, &vid_mac_entry);
                }
            } else {            /* Invalid */
                rc = VTSS_RC_ERROR;
            }
        }
#endif /* defined(VTSS_FEATURE_IPV4_MC_SIP) */

        break;
    default:
        T_D("Invalid version: %s(%d)",
            ipmc_lib_version_txt(version, IPMC_TXT_CASE_UPPER), version);
        rc = VTSS_RC_ERROR;

        break;
    }

    grp_db = &grp_op->info->db;
    element_allow = element_deny = NULL;
    if (is_sfm && IPMC_MEM_SYSTEM_MTAKE(check, sizeof(ipmc_sfm_srclist_t))) {
        check_bak = check;

        if (version == IPMC_IP_VERSION_MLD) {
#if defined(VTSS_FEATURE_IPV6_MC_SIP)
            memcpy(&check->src_ip, &ip6sip, sizeof(vtss_ipv6_t));
#endif /* defined(VTSS_FEATURE_IPV6_MC_SIP) */
        } else {
#if defined(VTSS_FEATURE_IPV4_MC_SIP)
            memset(&check->src_ip, 0x0, sizeof(vtss_ipv6_t));
            memcpy(&check->src_ip.addr[12], (u8 *)&ip4sip, sizeof(ipmcv4addr));
#endif /* defined(VTSS_FEATURE_IPV4_MC_SIP) */
        }

        element_allow = ipmc_lib_srclist_adr_get(grp_db->ipmc_sf_do_forward_srclist, check);
        element_deny = ipmc_lib_srclist_adr_get(grp_db->ipmc_sf_do_not_forward_srclist, check);
        IPMC_MEM_SYSTEM_MGIVE(check_bak);
    }

    if (rc == VTSS_OK) {
        if (is_sfm) {
            if (op) {
                if (version == IPMC_IP_VERSION_MLD) {
                    if (element_allow) {
#if defined(VTSS_FEATURE_IPV6_MC_SIP)
                        element_allow->sfm_in_hw = TRUE;
#else
                        element_allow->sfm_in_hw = FALSE;
#endif /* defined(VTSS_FEATURE_IPV6_MC_SIP) */
                    }
                    if (element_deny) {
#if defined(VTSS_FEATURE_IPV6_MC_SIP)
                        element_deny->sfm_in_hw = TRUE;
#else
                        element_deny->sfm_in_hw = FALSE;
#endif /* defined(VTSS_FEATURE_IPV6_MC_SIP) */
                    }
                } else {
                    if (element_allow) {
#if defined(VTSS_FEATURE_IPV4_MC_SIP)
                        element_allow->sfm_in_hw = TRUE;
#else
                        element_allow->sfm_in_hw = FALSE;
#endif /* defined(VTSS_FEATURE_IPV4_MC_SIP) */
                    }
                    if (element_deny) {
#if defined(VTSS_FEATURE_IPV4_MC_SIP)
                        element_deny->sfm_in_hw = TRUE;
#else
                        element_deny->sfm_in_hw = FALSE;
#endif /* defined(VTSS_FEATURE_IPV4_MC_SIP) */
                    }
                }
            } else {
                if (element_allow) {
                    element_allow->sfm_in_hw = FALSE;
                }
                if (element_deny) {
                    element_deny->sfm_in_hw = FALSE;
                }
            }
        } else {
            if (op) {
                grp_db->asm_in_hw = TRUE;
            } else {
                grp_db->asm_in_hw = FALSE;
            }
        }
    }

#if defined(VTSS_FEATURE_IPV4_MC_SIP)
    api_ssm4 = TRUE;
#else
    api_ssm4 = FALSE;
#endif /* defined(VTSS_FEATURE_IPV4_MC_SIP) */
#if defined(VTSS_FEATURE_IPV6_MC_SIP)
    api_ssm6 = TRUE;
#else
    api_ssm6 = FALSE;
#endif /* defined(VTSS_FEATURE_IPV6_MC_SIP) */
    T_D("%s to %s %s-%s-FWD (API-[SSM4->%s|SSM6->%s]/RC-[%d])",
        (rc == VTSS_OK) ? "OK" : "NG",
        bypassing ? "bypass" : "config",
        ipmc_lib_version_txt(version, IPMC_TXT_CASE_UPPER),
        is_sfm ? "SFM" : "ASM",
        api_ssm4 ? "Y" : "N",
        api_ssm6 ? "Y" : "N",
        rc);

    return rc;
}

/* Register/Unregister for frames via VTSS API */
static vtss_rc vtss_ipmc_lib_rcv_reg(BOOL enable, ipmc_ip_version_t rcv_version)
{
    vtss_rc                 rc = VTSS_OK;
    vtss_packet_rx_conf_t   conf;

    memset(&conf, 0x0, sizeof(vtss_packet_rx_conf_t));
    vtss_appl_api_lock();
    if ((rc = vtss_packet_rx_conf_get(NULL, &conf)) == VTSS_OK) {
        switch ( rcv_version ) {
        case IPMC_IP_VERSION_IGMP:
            conf.reg.igmp_cpu_only = enable;

            break;
        case IPMC_IP_VERSION_MLD:
            conf.reg.mld_cpu_only = enable;

            break;
        default:

            break;
        }

        T_D("vtss_packet_rx_conf_set(%s/%s)",
            ipmc_lib_version_txt(rcv_version, IPMC_TXT_CASE_UPPER),
            enable ? "TRUE" : "FALSE");
        rc = vtss_packet_rx_conf_set(NULL, &conf);
    }
    vtss_appl_api_unlock();

    return rc;
}

vtss_rc vtss_ipmc_lib_rx_unregister(ipmc_ip_version_t version)
{
#ifdef VTSS_SW_OPTION_PACKET
    vtss_rc rc = VTSS_OK;

    if ((version == IPMC_IP_VERSION_MLD) ? (vtss_mld_filter_id != NULL) : (vtss_igmp_filter_id != NULL)) {
        if (version == IPMC_IP_VERSION_IGMP) {
            if ((rc = packet_rx_filter_unregister(vtss_igmp_filter_id)) == VTSS_OK) {
                vtss_igmp_filter_id = NULL;
            }
        }

        if (version == IPMC_IP_VERSION_MLD) {
            if ((rc = packet_rx_filter_unregister(vtss_mld_filter_id)) == VTSS_OK) {
                vtss_mld_filter_id = NULL;
            }
        }

        /* Un-register for stopping to capture IPMC(IGMP/MLD) frames via switch API */
        (void) vtss_ipmc_lib_rcv_reg(FALSE, version);
    }

    return rc;
#else
    return VTSS_OK;
#endif /* VTSS_SW_OPTION_PACKET */
}

vtss_rc vtss_ipmc_lib_rx_register(void *cb, ipmc_ip_version_t version)
{
#ifdef VTSS_SW_OPTION_PACKET
    packet_rx_filter_t  filter;

    /* Register for MLD frames via packet API */
    if ((version == IPMC_IP_VERSION_MLD) ? (vtss_mld_filter_id == NULL) : (vtss_igmp_filter_id == NULL)) {
        /* Register for starting to capture IPMC(IGMP/MLD) frames via switch API */
        (void) vtss_ipmc_lib_rcv_reg(TRUE, version);

        memset(&filter, 0x0, sizeof(packet_rx_filter_t));
        switch ( version ) {
        case IPMC_IP_VERSION_IGMP:
            filter.modid = VTSS_MODULE_ID_IPMC_LIB;
            filter.match = PACKET_RX_FILTER_MATCH_IPV4_PROTO;
            filter.prio  = PACKET_RX_FILTER_PRIO_NORMAL;
            filter.ipv4_proto = IP_MULTICAST_IGMP_PROTO_ID;
            filter.cb = cb;

            if (vtss_igmp_filter_id) {
                return packet_rx_filter_change(&filter, &vtss_igmp_filter_id);
            } else {
                return packet_rx_filter_register(&filter, &vtss_igmp_filter_id);
            }
        case IPMC_IP_VERSION_MLD:
            filter.modid = VTSS_MODULE_ID_IPMC_LIB;
            filter.match = PACKET_RX_FILTER_MATCH_DMAC | PACKET_RX_FILTER_MATCH_ETYPE;;
            filter.prio  = PACKET_RX_FILTER_PRIO_NORMAL;
            filter.etype = IP_MULTICAST_V6_ETHER_TYPE; /* IPv6 */
            memset(&filter.dmac_mask[0], 0xFF, sizeof(filter.dmac_mask));
            filter.dmac[0] = 0x33; /* 0x3333 */
            filter.dmac_mask[0] = 0x0;
            filter.dmac[1] = 0x33; /* 0x3333 */
            filter.dmac_mask[1] = 0x0;
            filter.cb = cb;

            if (vtss_mld_filter_id) {
                return packet_rx_filter_change(&filter, &vtss_mld_filter_id);
            } else {
                return packet_rx_filter_register(&filter, &vtss_mld_filter_id);
            }
        default:
            return VTSS_RC_ERROR;
        }
    } else {
        return VTSS_OK;
    }
#else
    return VTSS_OK;
#endif /* VTSS_SW_OPTION_PACKET */
}

static vtss_rc _ipmc_lib_flooding_set(ipmc_ip_version_t version, ipmc_activate_t activate)
{
    vtss_rc retVal = VTSS_OK;
    BOOL    apply, flooding[VTSS_PORT_ARRAY_SIZE];

    switch ( version ) {
    case IPMC_IP_VERSION_IGMP:
        apply = FALSE;
        if (vtss_ipv4_mc_flood_members_get(NULL, flooding) == VTSS_OK) {
            ipmc_lib_lock();
            if (memcmp(flooding4_member, flooding, sizeof(flooding))) {
                memcpy(flooding, flooding4_member, sizeof(flooding));
                apply = TRUE;
            }
            ipmc_lib_unlock();
        }
        if (apply) {
            retVal = vtss_ipv4_mc_flood_members_set(NULL, flooding);
        }

        break;
    case IPMC_IP_VERSION_MLD:
        apply = FALSE;
        if (vtss_ipv6_mc_flood_members_get(NULL, flooding) == VTSS_OK) {
            ipmc_lib_lock();
            if (memcmp(flooding6_member, flooding, sizeof(flooding))) {
                memcpy(flooding, flooding6_member, sizeof(flooding));
                apply = TRUE;
            }
            ipmc_lib_unlock();
        }
        if (apply) {
            retVal = vtss_ipv6_mc_flood_members_set(NULL, flooding);
        }

        break;
    case IPMC_IP_VERSION_ALL:
        apply = FALSE;
        if (vtss_ipv4_mc_flood_members_get(NULL, flooding) == VTSS_OK) {
            ipmc_lib_lock();
            if (memcmp(flooding4_member, flooding, sizeof(flooding))) {
                memcpy(flooding, flooding4_member, sizeof(flooding));
                apply = TRUE;
            }
            ipmc_lib_unlock();
        }

        if (apply) {
            retVal = vtss_ipv4_mc_flood_members_set(NULL, flooding);
        }

        apply = FALSE;
        if (vtss_ipv6_mc_flood_members_get(NULL, flooding) == VTSS_OK) {
            ipmc_lib_lock();
            if (memcmp(flooding6_member, flooding, sizeof(flooding))) {
                memcpy(flooding, flooding6_member, sizeof(flooding));
                apply = TRUE;
            }
            ipmc_lib_unlock();
        }

        if (apply) {
            retVal |= vtss_ipv6_mc_flood_members_set(NULL, flooding);
        }

        break;
    default:
        retVal = VTSS_RC_ERROR;

        break;
    }

    return retVal;
}

BOOL ipmc_lib_unregistered_flood_set(ipmc_owner_t owner,
                                     ipmc_activate_t activate,
                                     BOOL member[VTSS_PORT_ARRAY_SIZE])
{
    ipmc_ip_version_t   version;

    ipmc_lib_lock();
    switch ( owner ) {
    case IPMC_OWNER_IGMP:
    case IPMC_OWNER_SNP4:
    case IPMC_OWNER_MVR4:
        version = IPMC_IP_VERSION_IGMP;
        memcpy(flooding4_member, member, sizeof(flooding4_member));

        break;
    case IPMC_OWNER_MLD:
    case IPMC_OWNER_SNP6:
    case IPMC_OWNER_MVR6:
        version = IPMC_IP_VERSION_MLD;
        memcpy(flooding6_member, member, sizeof(flooding6_member));

        break;
    case IPMC_OWNER_MVR:
    case IPMC_OWNER_SNP:
        version = IPMC_IP_VERSION_ALL;
        memcpy(flooding4_member, member, sizeof(flooding4_member));
        memcpy(flooding6_member, member, sizeof(flooding6_member));

        break;
    case IPMC_OWNER_INIT:
    case IPMC_OWNER_ALL:
    case IPMC_OWNER_MAX:
    default:
        ipmc_lib_unlock();
        return FALSE;
    }
    ipmc_lib_unlock();

    if (_ipmc_lib_flooding_set(version, activate) == VTSS_OK) {
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL ipmc_lib_mc6_ctrl_flood_set(ipmc_owner_t owner, BOOL status)
{
    BOOL    retVal;
    u8      mask, original_regs;

    switch ( owner ) {
    case IPMC_OWNER_MLD:
    case IPMC_OWNER_SNP:
    case IPMC_OWNER_SNP6:
    case IPMC_OWNER_MVR:
    case IPMC_OWNER_MVR6:
        original_regs = mc6_ctrl_flood_set;
        mask = 0;

        if (status) {
            mc6_ctrl_flood_set |= (1 << owner);
        } else {
            mask |= (1 << owner);
            mc6_ctrl_flood_set &= (~mask);
        }

        /* Only for the first one to turn it ON */
        if (status && !original_regs && mc6_ctrl_flood_set) {
            /* Enable local scope MLD */
            if (vtss_ipv6_mc_ctrl_flood_set(NULL, TRUE) != VTSS_OK) {
                T_D("\n\rvtss_ipv6_mc_ctrl_flood_set failed->0xFF02::/16 will be ignored");
            }
        }

        /* Only for the last one to turn it OFF */
        if (!status && original_regs && !mc6_ctrl_flood_set) {
            /* Disable local scope MLD */
            if (vtss_ipv6_mc_ctrl_flood_set(NULL, FALSE) != VTSS_OK) {
                T_D("\n\rvtss_ipv6_mc_ctrl_flood_set failed->0xFF02::/16 will be flooded");
            }
        }

        retVal = TRUE;

        break;
    case IPMC_OWNER_IGMP:
    case IPMC_OWNER_SNP4:
    case IPMC_OWNER_MVR4:
        retVal = TRUE;

        break;
    case IPMC_OWNER_INIT:
    case IPMC_OWNER_ALL:
    case IPMC_OWNER_MAX:
    default:
        retVal = FALSE;

        break;
    }

    return retVal;
}

BOOL ipmc_lib_log(const ipmc_lib_log_t *content, const ipmc_log_severity_t severity)
{
    char        log_buf[IPMC_LIB_LOG_BUF_SIZE];
    char        dip_buf[IPV6_ADDR_IBUF_MAX_LEN], sip_buf[IPV6_ADDR_IBUF_MAX_LEN];
    char        pname[VTSS_IPMC_NAME_MAX_LEN], ename[VTSS_IPMC_NAME_MAX_LEN];
    vtss_ipv6_t dip6, sip6;
    vtss_ipv4_t dip4, sip4;

    /*
        1.  Date and Time: A time string for stamping.
        2.  Severity: A string for specifying the severity.
        3.  IPMC Version: IPv4 or IPv6.
        4.  (Destination) Group: Address that matches the filtering condition.
        5.  (Source) IP: Address of the host that expects to register the group.
        6.  VLAN: VLAN ID that receives report frames.
        7.  Port: Port number that receives report frames.
        8.  Filtering Action: Allow or deny operation that matches the filtering condition.
        9.  Associated Profile: Profile object¡¦s identifier.
        10. Associated Address Entry: Address object¡¦s identifier.
    */
    if (!content) {
        T_D("content is %s", content ? "OK" : "NULL");

        return FALSE;
    }

    memset(log_buf, 0x0, sizeof(log_buf));
    switch ( content->type ) {
    case IPMC_LOG_TYPE_PF:
        if (!content->dst || !content->src) {
            T_D("content->dst is %s; content->src is %s",
                (content && content->dst) ? "OK" : "NULL",
                (content && content->src) ? "OK" : "NULL");

            break;
        }

        dip4 = sip4 = 0;
        IPMC_LIB_ADRS_CPY(&dip6, content->dst);
        IPMC_LIB_ADRS_CPY(&sip6, content->src);
        if (content->version == IPMC_IP_VERSION_IGMP) {
            IPMC_LIB_ADRS_6TO4_SET(dip6, dip4);
            IPMC_LIB_ADRS_6TO4_SET(sip6, sip4);
            dip4 = htonl(dip4);
            sip4 = htonl(sip4);
        }

        memset(pname, 0x0, sizeof(pname));
        memset(ename, 0x0, sizeof(ename));
        if (content->event.profile.name) {
            sprintf(pname, "%s", content->event.profile.name);
        } else {
            sprintf(pname, "Unknown");
        }
        if (content->event.profile.entry) {
            sprintf(ename, "%s", content->event.profile.entry);
        } else {
            sprintf(ename, "Unknown");
        }

        memset(dip_buf, 0x0, sizeof(dip_buf));
        memset(sip_buf, 0x0, sizeof(sip_buf));
        sprintf(log_buf,
                "Date&Time:%s; Severity:%s; Version:%s; DIP:%s; SIP:%s; VID:%u; Port:%u; Action:%s; Profile:%s; Entry:%s.",
                misc_time2str(time(NULL)),
                ipmc_lib_severity_txt(severity, IPMC_TXT_CASE_CAPITAL),
                ipmc_lib_version_txt(content->version, IPMC_TXT_CASE_UPPER),
                (content->version == IPMC_IP_VERSION_IGMP) ? misc_ipv4_txt(dip4, dip_buf) : misc_ipv6_txt(&dip6, dip_buf),
                (content->version == IPMC_IP_VERSION_IGMP) ? misc_ipv4_txt(sip4, sip_buf) : misc_ipv6_txt(&sip6, sip_buf),
                content->vid,
                content->port,
                ipmc_lib_fltr_action_txt(content->event.profile.action, IPMC_TXT_CASE_CAPITAL),
                pname, ename);

        break;
    case IPMC_LOG_TYPE_MSG:
    default:
        sprintf(log_buf,
                "Date&Time:%s; Severity:%s; Version:%s; VID:%u; Port:%u; Message: %s.",
                misc_time2str(time(NULL)),
                ipmc_lib_severity_txt(severity, IPMC_TXT_CASE_CAPITAL),
                ipmc_lib_version_txt(content->version, IPMC_TXT_CASE_UPPER),
                content->vid,
                content->port,
                content->event.message.data);

        break;
    }

    T_D("Severity:%d-> %s", severity, log_buf);

    switch ( severity ) {
    case IPMC_SEVERITY_Normal:
    case IPMC_SEVERITY_InfoDebug:
        S_I(log_buf);

        break;
    case IPMC_SEVERITY_Notice:
    case IPMC_SEVERITY_Warning:
        S_W(log_buf);

        break;
    case IPMC_SEVERITY_Error:
    case IPMC_SEVERITY_Critical:
    case IPMC_SEVERITY_Alert:
    case IPMC_SEVERITY_Emergency:
    default:
        S_E(log_buf);

        break;
    }

    return TRUE;
}

vtss_rc ipmc_lib_porting_init(void)
{
    int i;

    if (ipmc_lib_ptg_done_init) {
        return VTSS_OK;
    }

    critd_init(&ipmc_ptg_crit, "ipmc_ptg_crit", VTSS_MODULE_ID_IPMC_LIB, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
    IPMC_PTG_CRIT_EXIT();

    for (i = 0; i < IPMC_LIB_MEM_MAX_POOL; i++) {
        ipmc_lib_mem_area[i] = NULL;
        memset(&ipmc_lib_memory[i], 0x0, sizeof(cyg_handle_t));
        memset(&ipmc_lib_mem_pool_fix[i], 0x0, sizeof(cyg_mempool_fix));
        memset(&ipmc_lib_mem_pool_var[i], 0x0, sizeof(cyg_mempool_var));
    }

    mc6_ctrl_flood_set = 0x0;
    memset(flooding4_member, 0x0, sizeof(flooding4_member));
    memset(flooding6_member, 0x0, sizeof(flooding6_member));

    ipmc_lib_ptg_done_init = TRUE;
    return VTSS_OK;
}
