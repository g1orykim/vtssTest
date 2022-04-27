/*

 Vitesse Switch Software.

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

#include "main.h"
#include "critd_api.h"
#include "conf_api.h"
#include "misc_api.h"

#include "ipmc_lib.h"
#include "ipmc_lib_porting.h"


/* ************************************************************************ **
 *
 * Defines
 *
 * ************************************************************************ */
#define VTSS_TRACE_MODULE_ID    VTSS_MODULE_ID_IPMC_LIB


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
static BOOL                         ipmc_lib_cmn_done_init = FALSE;
static int                          ipmc_mem_cnt = 0;
static u8                           ipmc_local_port_cnt = 0;
/*lint -esym(459, ipmc_lib_cmn_crit) */
static critd_t                      ipmc_lib_cmn_crit;

static ipmc_lib_mgmt_info_t         ipmc_ip_mgmt_system;
static ipmc_mem_status_t            ipmc_lib_memory_status[IPMC_MEM_TYPE_MAX];

static BOOL                         router_port_mask[IPMC_IP_VERSION_MAX][VTSS_PORT_BF_SIZE];
static BOOL                         diff_router_port_mask[IPMC_IP_VERSION_MAX][VTSS_PORT_BF_SIZE];
static ipmc_prefix_t                ipmc_prefix_ssm_range[IPMC_IP_VERSION_MAX];

#define IPMC_MGMT_SYSTEM_CHG_WR(x)  do {ipmc_ip_mgmt_system.change = (x);} while (0)
#define IPMC_MGMT_SYSTEM_CHG_RD(x)  do {(x) = (ipmc_ip_mgmt_system.change); IPMC_MGMT_SYSTEM_CHG_WR(FALSE);} while (0)

#define IPMC_MGMT_IFLIST            (&ipmc_ip_mgmt_system.intf_list)
#define IPMC_MGMT_IFLIST_FREE       (IPMC_MGMT_IFLIST->free)
#define IPMC_MGMT_IFLIST_USED       (IPMC_MGMT_IFLIST->used)
#define IPMC_MGMT_IFLIST_CNT_NUM    (IPMC_MGMT_IFLIST->count)
#define IPMC_MGMT_IFLIST_CNT_MAX    (IPMC_MGMT_IFLIST->max)
#define IPMC_MGMT_IFLIST_CNT_INC    (++IPMC_MGMT_IFLIST_CNT_NUM)
#define IPMC_MGMT_IFLIST_CNT_DEC    (IPMC_MGMT_IFLIST_CNT_NUM--)
#define IPMC_MGMT_IFLIST_CNT_OVR    (!(IPMC_MGMT_IFLIST_CNT_NUM < IPMC_MGMT_IFLIST_CNT_MAX))

static vtss_ipv6_t                  IPMC_IP6_ZERO_ADDR;
static vtss_ipv6_t                  IPMC_IP6_ALL1_ADDR;
static vtss_ipv6_t                  IPMC_IP6_ALL_NODE_ADDR;
static vtss_ipv6_t                  IPMC_IP6_ALL_RTR_ADDR;

static ipmcv4addr                   IPMC_IP4_ZERO_ADDR;
static ipmcv4addr                   IPMC_IP4_ALL1_ADDR;
static ipmcv4addr                   IPMC_IP4_ALL_NODE_ADDR;
static ipmcv4addr                   IPMC_IP4_ALL_RTR_ADDR;

#if VTSS_TRACE_ENABLED
#define IPMC_LIB_CRIT_ENTER()       critd_enter(&ipmc_lib_cmn_crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define IPMC_LIB_CRIT_EXIT()        critd_exit(&ipmc_lib_cmn_crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#else
#define IPMC_LIB_CRIT_ENTER()       critd_enter(&ipmc_lib_cmn_crit)
#define IPMC_LIB_CRIT_EXIT()        critd_exit(&ipmc_lib_cmn_crit)
#endif /* VTSS_TRACE_ENABLED */

/* IPMC Memory Object */
typedef struct {
    u32                             h_count;
    u32                             h_max;
    u32                             sl_count;
    u32                             sl_max;
    u32                             grp_count;
    u32                             grp_max;
    u32                             info_count;
    u32                             info_max;
    u32                             rules_count;
    u32                             rules_max;
    u32                             profile_count;
    u32                             profile_max;

    ipmc_db_ctrl_hdr_t              *h_free;
    ipmc_db_ctrl_hdr_t              *h_used;
    ipmc_sfm_srclist_t              *sl_free;
    ipmc_sfm_srclist_t              *sl_used;
    ipmc_group_entry_t              *grp_free;
    ipmc_group_entry_t              *grp_used;
    ipmc_group_info_t               *info_free;
    ipmc_group_info_t               *info_used;
    ipmc_profile_rule_t             *rules_free;
    ipmc_profile_rule_t             *rules_used;
    ipmc_lib_profile_mem_t          *profile_free;
    ipmc_lib_profile_mem_t          *profile_used;
} ipmc_lib_mem_obj_t;

typedef struct {
    ipmc_lib_mem_obj_t              obj;

    ipmc_db_ctrl_hdr_t              h_table[IPMC_LIB_DYN_ALLOC_CTRL_HDR_CNT];
    ipmc_sfm_srclist_t              sl_table[IPMC_LIB_TOTAL_SRC_LIST];
    ipmc_group_entry_t              grp_table[IPMC_LIB_SUPPORTED_GROUPS];
    ipmc_group_info_t               info_table[IPMC_LIB_SUPPORTED_GROUPS];
    ipmc_profile_rule_t             rules_table[IPMC_LIB_SUPPORTED_RULES];
    ipmc_lib_profile_mem_t          profile_table[IPMC_LIB_SUPPORTED_PROFILES];
} ipmc_lib_mem_pool_t;

static ipmc_lib_mem_pool_t          ipmc_mem_pool;


ipmc_time_cmp_t ipmc_lib_time_cmp(ipmc_time_t *time_a, ipmc_time_t *time_b)
{
    if (!time_a || !time_b) {
        return IPMC_LIB_TIME_CMP_INVALID;
    }

    if (time_a->sec > time_b->sec) {
        return IPMC_LIB_TIME_CMP_GREATER;
    } else if (time_a->sec < time_b->sec) {
        return IPMC_LIB_TIME_CMP_LESS;
    } else {
        if (time_a->msec > time_b->msec) {
            return IPMC_LIB_TIME_CMP_GREATER;
        } else if (time_a->msec < time_b->msec) {
            return IPMC_LIB_TIME_CMP_LESS;
        } else {
            if (time_a->usec > time_b->usec) {
                return IPMC_LIB_TIME_CMP_GREATER;
            } else if (time_a->usec < time_b->usec) {
                return IPMC_LIB_TIME_CMP_LESS;
            } else {
#if 1 /* etliang */
                return IPMC_LIB_TIME_CMP_EQUAL;
#else
                if (time_a->nsec > time_b->nsec) {
                    return IPMC_LIB_TIME_CMP_GREATER;
                } else if (time_a->nsec < time_b->nsec) {
                    return IPMC_LIB_TIME_CMP_LESS;
                } else {
                    return IPMC_LIB_TIME_CMP_EQUAL;
                }
#endif /* etliang */
            }
        }
    }
}

void ipmc_lib_time_cpy(ipmc_time_t *time_a, ipmc_time_t *time_b)
{
    if (!time_a || !time_b) {
        return;
    }

    time_a->sec = time_b->sec;
    time_a->msec = time_b->msec;
    time_a->usec = time_b->usec;
//    time_a->nsec = time_b->nsec;
}

BOOL ipmc_lib_time_stamp(ipmc_time_t *time_target, ipmc_time_t *time_offset)
{
    u32     carry_s;
    BOOL    carry_t;

    if (!time_target || !ipmc_lib_time_curr_get(time_target)) {
        return FALSE;
    }

    if (time_offset) {
#if 1 /* etliang */
        carry_s = time_target->usec + time_offset->usec;
#else
        carry_s = time_target->nsec + time_offset->nsec;
        carry_t = FALSE;
        if (carry_s < 1000) {
            time_target->nsec = carry_s;
        } else {
            time_target->nsec = carry_s - 1000;
            carry_t = TRUE;
        }

        if (carry_t) {
            carry_s = time_target->usec + time_offset->usec + 1;
        } else {
            carry_s = time_target->usec + time_offset->usec;
        }
#endif /* etliang */
        carry_t = FALSE;
        if (carry_s < 1000) {
            time_target->usec = carry_s;
        } else {
            time_target->usec = carry_s - 1000;
            carry_t = TRUE;
        }

        if (carry_t) {
            carry_s = time_target->msec + time_offset->msec + 1;
        } else {
            carry_s = time_target->msec + time_offset->msec;
        }
        carry_t = FALSE;
        if (carry_s < 1000) {
            time_target->msec = carry_s;
        } else {
            time_target->msec = carry_s - 1000;
            carry_t = TRUE;
        }

        /* u32 for sec will warp after 136 years */
        if (carry_t) {
            time_target->sec = time_target->sec + time_offset->sec + 1;
        } else {
            time_target->sec = time_target->sec + time_offset->sec;
        }
    }

    return TRUE;
}

u8 ipmc_lib_get_system_local_port_cnt(void)
{
    return ipmc_local_port_cnt;
}

BOOL ipmc_lib_get_next_system_local_port(u8 *port)
{
    if (!port) {
        return FALSE;
    }

    if (*port < ipmc_local_port_cnt) {
        *port = *port + 1;
        return TRUE;
    } else {
        return FALSE;
    }
}

static BOOL ipmc_lib_system_ipif_get(ipmc_mgmt_ipif_t **ipif)
{
    ipmc_mgmt_ipif_t    *ifp;

    if (!ipmc_lib_cmn_done_init || !ipif || !(*ipif)) {
        return FALSE;
    }

    ifp = IPMC_MGMT_IFLIST_USED;
    while (ifp) {
        if (ipmc_mgmt_intf_live(ifp) &&
            (ipmc_mgmt_intf_vidx(ifp) == ipmc_mgmt_intf_vidx(*ipif))) {
            *ipif = ifp;
            return TRUE;
        }

        ifp = ipmc_mgmt_intf_next(ifp);
    }

    return FALSE;
}

static BOOL ipmc_lib_system_ipif_get_next(ipmc_mgmt_ipif_t **ipif)
{
    ipmc_mgmt_ipif_t    *ifp, *nxt;

    if (!ipmc_lib_cmn_done_init || !ipif || !(*ipif)) {
        return FALSE;
    }

    nxt = NULL;
    ifp = IPMC_MGMT_IFLIST_USED;
    while (ifp) {
        if (ipmc_mgmt_intf_live(ifp) &&
            (ipmc_mgmt_intf_vidx(ifp) > ipmc_mgmt_intf_vidx(*ipif))) {
            if (!nxt) {
                nxt = ifp;
            } else {
                if (ipmc_mgmt_intf_vidx(nxt) > ipmc_mgmt_intf_vidx(ifp)) {
                    nxt = ifp;
                }
            }
        }

        ifp = ipmc_mgmt_intf_next(ifp);
    }

    if (!nxt) {
        return FALSE;
    } else {
        *ipif = nxt;
        return TRUE;
    }
}

static BOOL ipmc_lib_system_ipif_set(ipmc_mgmt_ipif_t *ipif)
{
    ipmc_mgmt_ipif_t    *ifp, *prv;
    BOOL                ret;

    if (!ipmc_lib_cmn_done_init || !ipif) {
        return FALSE;
    }

    ret = FALSE;
    prv = NULL;
    ifp = IPMC_MGMT_IFLIST_USED;
    while (ifp) {
        if (ipmc_mgmt_intf_live(ifp) &&
            (ipmc_mgmt_intf_vidx(ifp) == ipmc_mgmt_intf_vidx(ipif))) {
            if (ipmc_mgmt_intf_live(ipif)) {    /* UPDATE */
                if (ifp != ipif) {
                    ipmc_mgmt_intf_vidx(ifp) = ipmc_mgmt_intf_vidx(ipif);
                    ipmc_mgmt_intf_adr4(ifp) = ipmc_mgmt_intf_adr4(ipif);
                    ipmc_mgmt_intf_opst(ifp) = ipmc_mgmt_intf_opst(ipif);
                    ipmc_mgmt_intf_live(ifp) = TRUE;
                }
            } else {                            /* DELETE */
                if (prv) {
                    /* LIST */
                    ipmc_mgmt_intf_next(prv) = ipmc_mgmt_intf_next(ifp);
                } else {
                    /* HEAD */
                    IPMC_MGMT_IFLIST_USED = ipmc_mgmt_intf_next(ifp);
                }

                memset(ifp, 0x0, sizeof(ipmc_mgmt_ipif_t));
                if ((prv = IPMC_MGMT_IFLIST_FREE) != NULL) {
                    while (prv) {
                        if (ipmc_mgmt_intf_next(prv)) {
                            prv = ipmc_mgmt_intf_next(prv);
                        } else {
                            ipmc_mgmt_intf_next(prv) = ifp;
                            break;
                        }
                    }
                } else {
                    IPMC_MGMT_IFLIST_FREE = ifp;
                }

                IPMC_MGMT_IFLIST_CNT_DEC;
            }

            ret = TRUE;
            break;
        } else {
            prv = ifp;
        }

        ifp = ipmc_mgmt_intf_next(ifp);
    }

    if (!ret) {                                 /* ADD */
        if (IPMC_MGMT_IFLIST_CNT_OVR) {
            T_D("IPMC_MGMT_IFLIST is full!");
        } else {
            if ((ifp = IPMC_MGMT_IFLIST_FREE) != NULL) {
                IPMC_MGMT_IFLIST_FREE = ipmc_mgmt_intf_next(ifp);

                memset(ifp, 0x0, sizeof(ipmc_mgmt_ipif_t));
                ipmc_mgmt_intf_vidx(ifp) = ipmc_mgmt_intf_vidx(ipif);
                ipmc_mgmt_intf_adr4(ifp) = ipmc_mgmt_intf_adr4(ipif);
                ipmc_mgmt_intf_opst(ifp) = ipmc_mgmt_intf_opst(ipif);
                ipmc_mgmt_intf_live(ifp) = TRUE;

                if ((prv = IPMC_MGMT_IFLIST_USED) != NULL) {
                    while (prv) {
                        if (ipmc_mgmt_intf_next(prv)) {
                            prv = ipmc_mgmt_intf_next(prv);
                        } else {
                            ipmc_mgmt_intf_next(prv) = ifp;
                            break;
                        }
                    }
                } else {
                    IPMC_MGMT_IFLIST_USED = ifp;
                }

                IPMC_MGMT_IFLIST_CNT_INC;
                ret = TRUE;
            } else {
                T_D("Invalid free list!");
            }
        }
    }

    if (ret) {
        IPMC_MGMT_SYSTEM_CHG_WR(TRUE);
    }

    return ret;
}

static BOOL _ipmc_lib_get_system_mgmt_querier_adr4(ipmc_intf_entry_t *intf, vtss_ipv4_t *ip4addr)
{
    vtss_ipv4_t qradr;

    if (!intf || !ip4addr) {
        return FALSE;
    }

    /* Static Querier Address Per VLAN */
    qradr = intf->param.querier.QuerierAdrs4;
    if (qradr) {
        *ip4addr = qradr;
        return TRUE;
    }

    return FALSE;
}

BOOL ipmc_lib_get_ipintf_igmp_adrs(ipmc_intf_entry_t *intf, vtss_ipv4_t *ip4addr)
{
    vtss_ipv4_t         intf_adr;
    ipmc_mgmt_ipif_t    ipif, *ifp;
    i8                  adrBuf[40];

    if (!ipmc_lib_cmn_done_init || !intf || !ip4addr) {
        return FALSE;
    }

    if (_ipmc_lib_get_system_mgmt_querier_adr4(intf, ip4addr)) {
        return TRUE;
    }

    intf_adr = 0;
    ifp = &ipif;
    ipmc_mgmt_intf_vidx(ifp) = intf->param.vid;
    IPMC_LIB_CRIT_ENTER();
    if (ipmc_lib_system_ipif_get(&ifp)) {
        memset(adrBuf, 0x0, sizeof(adrBuf));
        T_N("INTF-ADR-%s is %s",
            misc_ipv4_txt(ipmc_mgmt_intf_adr4(ifp), adrBuf),
            ipmc_mgmt_intf_opst(ifp) ? "Up" : "Down");

        if (ipmc_mgmt_intf_opst(ifp)) {
            intf_adr = ipmc_mgmt_intf_adr4(ifp);
        }
    } else {
        memset(ifp, 0x0, sizeof(ipmc_mgmt_ipif_t));
        while (ipmc_lib_system_ipif_get_next(&ifp)) {
            memset(adrBuf, 0x0, sizeof(adrBuf));
            T_N("INTF-ADR-%s is %s",
                misc_ipv4_txt(ipmc_mgmt_intf_adr4(ifp), adrBuf),
                ipmc_mgmt_intf_opst(ifp) ? "Up" : "Down");

            if (ipmc_mgmt_intf_opst(ifp)) {
                intf_adr = ipmc_mgmt_intf_adr4(ifp);
                break;
            }
        }
    }
    IPMC_LIB_CRIT_EXIT();

    IPMC_LIB_IP4_ADDR_NON_ZERO(intf_adr);
    *ip4addr = intf_adr;
    return TRUE;
}

BOOL ipmc_lib_get_system_mgmt_macx(u8 *mgmt_mac)
{
    if (!ipmc_lib_cmn_done_init || !mgmt_mac) {
        return FALSE;
    }

    IPMC_LIB_CRIT_ENTER();
    IPMC_MGMT_MAC_ADR_GET(&ipmc_ip_mgmt_system, mgmt_mac);
    IPMC_LIB_CRIT_EXIT();

    return TRUE;
}

BOOL ipmc_lib_get_system_mgmt_intf(ipmc_intf_entry_t *intf, ipmc_mgmt_ipif_t *retifp)
{
    BOOL                intf_fnd;
    ipmc_mgmt_ipif_t    ipif, *ifp;
    i8                  adrBuf[40];

    if (!ipmc_lib_cmn_done_init || !retifp) {
        return FALSE;
    }

    intf_fnd = FALSE;
    ifp = &ipif;
    IPMC_LIB_CRIT_ENTER();
    if (intf) {
        ipmc_mgmt_intf_vidx(ifp) = intf->param.vid;
        if (ipmc_lib_system_ipif_get(&ifp)) {
            memset(adrBuf, 0x0, sizeof(adrBuf));
            T_D("INTF-ADR-%s is %s",
                misc_ipv4_txt(ipmc_mgmt_intf_adr4(ifp), adrBuf),
                ipmc_mgmt_intf_opst(ifp) ? "Up" : "Down");

            if (ipmc_mgmt_intf_opst(ifp)) {
                intf_fnd = TRUE;
                memcpy(retifp, ifp, sizeof(ipmc_mgmt_ipif_t));
            }
        }
    } else {
        memset(ifp, 0x0, sizeof(ipmc_mgmt_ipif_t));
        while (ipmc_lib_system_ipif_get_next(&ifp)) {
            memset(adrBuf, 0x0, sizeof(adrBuf));
            T_D("INTF-ADR-%s is %s",
                misc_ipv4_txt(ipmc_mgmt_intf_adr4(ifp), adrBuf),
                ipmc_mgmt_intf_opst(ifp) ? "Up" : "Down");

            if (ipmc_mgmt_intf_opst(ifp)) {
                intf_fnd = TRUE;
                memcpy(retifp, ifp, sizeof(ipmc_mgmt_ipif_t));
                break;
            }
        }
    }
    IPMC_LIB_CRIT_EXIT();

    return intf_fnd;
}

static BOOL _ipmc_lib_system_mgmt_ipif_syn(ipmc_lib_mgmt_info_t *mgmt_sys, u32 *syn_cnt)
{
    vtss_vid_t          intf_ifid;  /* With IP2, VID is used as the IFID */
    vtss_if_status_t    *ops, ipst[IPMC_IP_INTF_MAX_OPST];
    ipmc_mgmt_ipif_t    *ifp;
    BOOL                intf_up;
    vtss_vid_t          intf_vid;
    vtss_ipv4_t         intf_adr;
    u32                 ops_cnt, ifp_cnt;
    i8                  adrBuf[40];

    if (!mgmt_sys || !syn_cnt) {
        return FALSE;
    }

    ifp_cnt = 0;
    intf_ifid = VTSS_IPMC_VID_NULL;
    IPMC_IP_INTF_IFID_TABLE_WALK(intf_ifid) {
        ops_cnt = 0;
        memset(ipst, 0x0, sizeof(ipst));
        IPMC_IP_INTF_OPST_NGET_CTN(intf_ifid, ipst, ops_cnt);

        intf_up = FALSE;
        intf_vid = VTSS_IPMC_VID_NULL;
        intf_adr = 0;
        if (!(ops_cnt > IPMC_IP_INTF_MAX_OPST)) {
            u32 ops_idx;

            for (ops_idx = 0; ops_idx < ops_cnt; ops_idx++) {
                ops = &ipst[ops_idx];

                if (IPMC_IP_INTF_OPST_UP(ops)) {
                    intf_up = TRUE;
                }
                if (!intf_vid && IPMC_IP_INTF_OPST_VID(ops)) {
                    intf_vid = IPMC_IP_INTF_OPST_VID(ops);
                }
                if (ops->type == VTSS_IF_STATUS_TYPE_IPV4) {
                    intf_adr = IPMC_IP_INTF_OPST_ADR4(ops);
                }
            }
        }

        memset(adrBuf, 0x0, sizeof(adrBuf));
        T_N("INTF-ADR(%s)-%s with VID-%u",
            intf_up ? "Up" : "Down",
            misc_ipv4_txt(intf_adr, adrBuf),
            intf_vid);
        if (intf_vid && intf_adr) {
            ifp = IPMC_MGMT_SYSTEM_IPIF(mgmt_sys, ifp_cnt);
            ipmc_mgmt_intf_live(ifp) = TRUE;
            ipmc_mgmt_intf_vidx(ifp) = intf_vid;
            ipmc_mgmt_intf_adr4(ifp) = intf_adr;
            ipmc_mgmt_intf_opst(ifp) = intf_up;
            ifp_cnt++;
        }
    }

    if (ifp_cnt) {
        *syn_cnt = ifp_cnt;
        return TRUE;
    } else {
        return FALSE;
    }
}

static void _ipmc_lib_system_mgmt_ipif_cfg(ipmc_lib_mgmt_info_t *mgmt_sys, BOOL *chg, u32 syn_cnt)
{
    BOOL                ifc;
    u32                 ifx;
    ipmc_mgmt_ipif_t    *ifp, *cmp;

    if (!mgmt_sys || !chg) {
        return;
    }

    for (ifx = 0; ifx < syn_cnt; ifx++) {
        cmp = ifp = IPMC_MGMT_SYSTEM_IPIF(mgmt_sys, ifx);
        if (!cmp) {
            continue;
        }

        ifc = FALSE;
        if (ipmc_lib_system_ipif_get(&ifp)) {
            if ((ipmc_mgmt_intf_vidx(ifp) != ipmc_mgmt_intf_vidx(cmp)) ||
                (ipmc_mgmt_intf_opst(ifp) != ipmc_mgmt_intf_opst(cmp)) ||
                (ipmc_mgmt_intf_live(ifp) != ipmc_mgmt_intf_live(cmp)) ||
                (ipmc_mgmt_intf_adr4(ifp) != ipmc_mgmt_intf_adr4(cmp))) {
                ifc = TRUE;
            }
        } else {
            ifc = TRUE;
        }
        if (ifc && !ipmc_lib_system_ipif_set(cmp)) {
            T_D("Failed in ADD/UPD ipmc_lib_system_ipif_set for VID:%u", ipmc_mgmt_intf_vidx(cmp));
        }

        ifp = cmp;
        if (ipmc_lib_system_ipif_get(&ifp)) {
            ipmc_mgmt_intf_chks(ifp) = 1;
        } else {
            T_D("Invalid situation: VID-%u is not there!", ipmc_mgmt_intf_vidx(cmp));
        }

        *chg |= ifc;
    }
}

static void _ipmc_lib_system_mgmt_ipif_clr(BOOL *chg)
{
    u32                 ifx;
    ipmc_mgmt_ipif_t    ifd;

    if (!chg) {
        return;
    }

    for (ifx = 0; ifx < VTSS_IPMC_MGMT_IPIF_MAX_CNT; ifx++) {
        if (IPMC_MGMT_IPIF_VALID(&ipmc_ip_mgmt_system, ifx)) {
            if (IPMC_MGMT_IPIF_CHKST(&ipmc_ip_mgmt_system, ifx)) {
                IPMC_MGMT_IPIF_CHKST(&ipmc_ip_mgmt_system, ifx) = 0;
            } else {
                memcpy(&ifd, IPMC_MGMT_SYSTEM_IPIF(&ipmc_ip_mgmt_system, ifx), sizeof(ipmc_mgmt_ipif_t));
                ipmc_mgmt_intf_live(&ifd) = FALSE;
                if (!ipmc_lib_system_ipif_set(&ifd)) {
                    T_D("Failed in DEL ipmc_lib_system_ipif_set for VID:%u", ipmc_mgmt_intf_vidx(&ifd));
                } else {
                    *chg |= TRUE;
                }
            }
        }
    }
}

static void ipmc_lib_system_mgmt_info_polling(void)
{
    u32                     syn_cnt;
    BOOL                    mac_valid, ipa_valid;
    ipmc_lib_mgmt_info_t    *mgmt_sys;

    if (!ipmc_lib_cmn_done_init ||
        !IPMC_MEM_SYSTEM_MTAKE(mgmt_sys, sizeof(ipmc_lib_mgmt_info_t))) {
        return;
    }

    syn_cnt = 0;
    ipa_valid = mac_valid = FALSE;
    memset(mgmt_sys, 0x0, sizeof(ipmc_lib_mgmt_info_t));
    if (conf_mgmt_mac_addr_get(mgmt_sys->mac_addr, 0) == VTSS_OK) {
        mac_valid = TRUE;
    }
    ipa_valid = _ipmc_lib_system_mgmt_ipif_syn(mgmt_sys, &syn_cnt);

    if (mac_valid || ipa_valid) {
        BOOL    chg_flag = FALSE;

        IPMC_LIB_CRIT_ENTER();
        if (mac_valid) {
            if (IPMC_MGMT_MAC_ADR_CMP(&ipmc_ip_mgmt_system, mgmt_sys->mac_addr)) {
                IPMC_MGMT_MAC_ADR_SET(&ipmc_ip_mgmt_system, mgmt_sys->mac_addr);
                chg_flag |= TRUE;
            }
        }

        if (ipa_valid) {
            /* FIRST: Add/Update */
            _ipmc_lib_system_mgmt_ipif_cfg(mgmt_sys, &chg_flag, syn_cnt);

            /* NEXT:  Delete */
            _ipmc_lib_system_mgmt_ipif_clr(&chg_flag);
        }

        if (chg_flag) {
            IPMC_MGMT_SYSTEM_CHG_WR(TRUE);
        }
        IPMC_LIB_CRIT_EXIT();
    }

    IPMC_MEM_SYSTEM_MGIVE(mgmt_sys);
}

BOOL ipmc_lib_system_mgmt_info_set(ipmc_lib_mgmt_info_t *mgmt_sys)
{
    u32                 ifx;
    ipmc_mgmt_ipif_t    ipif, *ifp;

    if (!ipmc_lib_cmn_done_init || !mgmt_sys) {
        return FALSE;
    }

    IPMC_LIB_CRIT_ENTER();
    IPMC_MGMT_MAC_ADR_SET(&ipmc_ip_mgmt_system, mgmt_sys->mac_addr);

    ifp = &ipif;
    memset(ifp, 0x0, sizeof(ipmc_mgmt_ipif_t));
    while (ipmc_lib_system_ipif_get_next(&ifp)) {
        memcpy(&ipif, ifp, sizeof(ipmc_mgmt_ipif_t));
        ipmc_mgmt_intf_live(&ipif) = FALSE;
        if (!ipmc_lib_system_ipif_set(&ipif)) {
            T_D("Failed in DEL ipmc_lib_system_ipif_set for VID:%u", ipmc_mgmt_intf_vidx(&ipif));
        }
        ifp = &ipif;
    }
    for (ifx = 0; ifx < VTSS_IPMC_MGMT_IPIF_MAX_CNT; ifx++) {
        if (!IPMC_MGMT_IPIF_VALID(mgmt_sys, ifx)) {
            continue;
        }

        memcpy(&ipif, IPMC_MGMT_SYSTEM_IPIF(mgmt_sys, ifx), sizeof(ipmc_mgmt_ipif_t));
        if (!ipmc_lib_system_ipif_set(&ipif)) {
            T_D("Failed in ADD ipmc_lib_system_ipif_set for VID:%u", ipmc_mgmt_intf_vidx(&ipif));
        }
    }

    IPMC_MGMT_SYSTEM_CHG_WR(TRUE);
    IPMC_LIB_CRIT_EXIT();

    return TRUE;
}

BOOL ipmc_lib_system_mgmt_info_cpy(ipmc_lib_mgmt_info_t *mgmt_sys)
{
    if (!ipmc_lib_cmn_done_init || !mgmt_sys) {
        return FALSE;
    }

    IPMC_LIB_CRIT_ENTER();
    memcpy(mgmt_sys, &ipmc_ip_mgmt_system, sizeof(ipmc_lib_mgmt_info_t));
    IPMC_LIB_CRIT_EXIT();

    return TRUE;
}

BOOL ipmc_lib_system_mgmt_info_chg(ipmc_lib_mgmt_info_t *mgmt_sys)
{
    BOOL    mgmt_chg;

    if (!ipmc_lib_cmn_done_init || !mgmt_sys) {
        return FALSE;
    }

    mgmt_chg = FALSE;
    IPMC_LIB_CRIT_ENTER();
    IPMC_MGMT_SYSTEM_CHG_RD(mgmt_chg);
    if (mgmt_chg) {
        memcpy(mgmt_sys, &ipmc_ip_mgmt_system, sizeof(ipmc_lib_mgmt_info_t));
        IPMC_LIB_CRIT_EXIT();
        return TRUE;
    }
    IPMC_LIB_CRIT_EXIT();

    ipmc_lib_system_mgmt_info_polling();

    IPMC_LIB_CRIT_ENTER();
    IPMC_MGMT_SYSTEM_CHG_RD(mgmt_chg);
    if (mgmt_chg) {
        memcpy(mgmt_sys, &ipmc_ip_mgmt_system, sizeof(ipmc_lib_mgmt_info_t));
    }
    IPMC_LIB_CRIT_EXIT();

    return mgmt_chg;
}

u8 ipmc_lib_calc_thread_tick(u32 *tick, u32 diff, u32 unit, u32 *overflow)
{
    u8  retVal = 0;

    if (!tick || !overflow) {
        return retVal;
    }

    retVal = (diff / unit) & 0xFF;
    while (diff != 0) {
        if ((*tick + 1) == 0xFFFFFFFF) {
            *tick = 0xFFFFFFFF % unit;
            *overflow = *overflow + 1;
        } else {
            *tick = *tick + 1;
        }

        diff--;
    }

    return retVal;
}

/* Input: Start & End; Output: Diff */
u32 ipmc_lib_diff_u32_wrap_around(u32 start, u32 end)
{
    u32 diff;

    if (start > end) {
        memset(&diff, 0xFF, sizeof(u32));
        diff = diff - start + 1 + end;
    } else {
        diff = end - start;
    }

    return diff;
}
u16 ipmc_lib_diff_u16_wrap_around(u16 start, u16 end)
{
    u16 diff;

    if (start > end) {
        memset(&diff, 0xFF, sizeof(u16));
        diff = diff - start + 1 + end;
    } else {
        diff = end - start;
    }

    return diff;
}
u8 ipmc_lib_diff_u8_wrap_around(u8 start, u8 end)
{
    u8  diff;

    if (start > end) {
        memset(&diff, 0xFF, sizeof(u8));
        diff = diff - start + 1 + end;
    } else {
        diff = end - start;
    }

    return diff;
}

/* Cannot convert VTSS_ISID_LOCAL in Zero-Based ISID */
vtss_isid_t ipmc_lib_isid_convert(BOOL local_valid, vtss_isid_t isid_in)
{
    vtss_isid_t isid_out;

    if (isid_in >= VTSS_ISID_END) {
        if (local_valid) {
            isid_out = VTSS_ISID_LOCAL;
        } else {
            isid_out = VTSS_ISID_UNKNOWN;
        }

        T_D("Invalid ISID for using ipmc_lib_isid_convert %d->%d", isid_in, isid_out);
    } else {
        isid_out = isid_in;

        if (!local_valid && (isid_in == VTSS_ISID_LOCAL)) {
            isid_out = VTSS_ISID_UNKNOWN;
            T_D("Invalid ISID for using ipmc_lib_isid_convert %d->%d", isid_in, isid_out);
        }
    }

    T_D("ipmc_lib_isid_convert %d->%d", isid_in, isid_out);
    return isid_out;
}

char *ipmc_lib_mem_id_txt(ipmc_mem_t mem_id)
{
    char    *txt;

    switch ( mem_id ) {
    case IPMC_MEM_OS_MALLOC:
        txt = "MALLOC(OS)";
        break;
    case IPMC_MEM_SYS:
        txt = "IPMC_MEM_SYS";
        break;
    case IPMC_MEM_JUMBO:
        txt = "IPMC_MEM_JUMBO";
        break;
    case IPMC_MEM_AVLT_NODE:
        txt = "IPMC_MEM_AVLT_NODE";
        break;
    case IPMC_MEM_GROUP:
        txt = "IPMC_MEM_GROUP";
        break;
    case IPMC_MEM_GRP_INFO:
        txt = "IPMC_MEM_GRP_INFO";
        break;
    case IPMC_MEM_CTRL_HDR:
        txt = "IPMC_MEM_CTRL_HDR";
        break;
    case IPMC_MEM_SRC_LIST:
        txt = "IPMC_MEM_SRC_LIST";
        break;
    case IPMC_MEM_RULES:
        txt = "IPMC_MEM_RULES";
        break;
    case IPMC_MEM_PROFILE:
        txt = "IPMC_MEM_PROFILE";
        break;
    default:
        txt = "?";
        break;
    }

    return txt;
}

int ipmc_lib_mem_debug_get_cnt(void)
{
    int retVal;

    IPMC_LIB_CRIT_ENTER();
    retVal = ipmc_mem_cnt;
    IPMC_LIB_CRIT_EXIT();

    return retVal;
}

void ipmc_lib_mem_debug_get_info(ipmc_mem_t idx, ipmc_memory_info_t *info)
{
    ipmc_lib_memory_info_t  entry;
    ipmc_lib_mem_obj_t      *mem_obj;

    if (!info) {
        return;
    }

    memset(&entry, 0x0, sizeof(ipmc_lib_memory_info_t));
    IPMC_LIB_CRIT_ENTER();
    switch ( idx ) {
    case IPMC_MEM_JUMBO:
        (void) ipmc_lib_memory_info_get(IPMC_MEM_PARTITIONED,
                                        ipmc_lib_memory_status[idx].idx,
                                        &entry);
        break;
    case IPMC_MEM_AVLT_NODE:
    case IPMC_MEM_SYS:
        (void) ipmc_lib_memory_info_get(IPMC_MEM_SYS_MALLOC,
                                        ipmc_lib_memory_status[idx].idx,
                                        &entry);
        if (ipmc_lib_memory_status[idx].sz_partition && (entry.blocksize < 0)) {
            entry.blocksize = ipmc_lib_memory_status[idx].sz_partition;
        }
        break;
    case IPMC_MEM_GROUP:
        mem_obj = &ipmc_mem_pool.obj;
        entry.totalmem = mem_obj->grp_max;
        entry.freemem = entry.maxfree = (mem_obj->grp_max - mem_obj->grp_count);
        entry.base = &ipmc_mem_pool.grp_table[0];
        entry.size = entry.blocksize = sizeof(ipmc_group_entry_t);
        entry.size *= entry.totalmem;

        break;
    case IPMC_MEM_GRP_INFO:
        mem_obj = &ipmc_mem_pool.obj;
        entry.totalmem = mem_obj->info_max;
        entry.freemem = entry.maxfree = (mem_obj->info_max - mem_obj->info_count);
        entry.base = &ipmc_mem_pool.info_table[0];
        entry.size = entry.blocksize = sizeof(ipmc_group_info_t);
        entry.size *= entry.totalmem;

        break;
    case IPMC_MEM_CTRL_HDR:
        mem_obj = &ipmc_mem_pool.obj;
        entry.totalmem = mem_obj->h_max;
        entry.freemem = entry.maxfree = (mem_obj->h_max - mem_obj->h_count);
        entry.base = &ipmc_mem_pool.h_table[0];
        entry.size = entry.blocksize = sizeof(ipmc_db_ctrl_hdr_t);
        entry.size *= entry.totalmem;

        break;
    case IPMC_MEM_SRC_LIST:
        mem_obj = &ipmc_mem_pool.obj;
        entry.totalmem = mem_obj->sl_max;
        entry.freemem = entry.maxfree = (mem_obj->sl_max - mem_obj->sl_count);
        entry.base = &ipmc_mem_pool.sl_table[0];
        entry.size = entry.blocksize = sizeof(ipmc_sfm_srclist_t);
        entry.size *= entry.totalmem;

        break;
    case IPMC_MEM_RULES:
        mem_obj = &ipmc_mem_pool.obj;
        entry.totalmem = mem_obj->rules_max;
        entry.freemem = entry.maxfree = (mem_obj->rules_max - mem_obj->rules_count);
        entry.base = &ipmc_mem_pool.rules_table[0];
        entry.size = entry.blocksize = sizeof(ipmc_profile_rule_t);
        entry.size *= entry.totalmem;

        break;
    case IPMC_MEM_PROFILE:
        mem_obj = &ipmc_mem_pool.obj;
        entry.totalmem = mem_obj->profile_max;
        entry.freemem = entry.maxfree = (mem_obj->profile_max - mem_obj->profile_count);
        entry.base = &ipmc_mem_pool.profile_table[0];
        entry.size = entry.blocksize = sizeof(ipmc_lib_profile_mem_t);
        entry.size *= entry.totalmem;

        break;
    default:
        break;
    }
    IPMC_LIB_CRIT_EXIT();

    info->totalmem  = entry.totalmem;
    info->freemem   = entry.freemem;
    info->base      = entry.base;
    info->size      = entry.size;
    info->blocksize = entry.blocksize;
    info->maxfree   = entry.maxfree;
}

BOOL ipmc_lib_mem_free(ipmc_mem_t type, u8 *ptr, u8 freid)
{
    if (!ipmc_lib_cmn_done_init) {
        T_E("Not Initialized Yet!");
        return FALSE;
    }

    IPMC_LIB_CRIT_ENTER();
    if (!ipmc_lib_memory_status[type].valid) {
        IPMC_LIB_CRIT_EXIT();
        T_W("%s(%d) is not a valid IPMC_MEM type", ipmc_lib_mem_id_txt(type), type);
        return FALSE;
    }

    if (ptr) {
        BOOL    free_status = TRUE;
        u8      alcid = 0, chkid = freid;

        ipmc_mem_cnt--;
        if (ipmc_mem_cnt < 0) {
            T_E("IPMCLIB_ASSERT(ipmc_lib_mem_free)->ipmc_mem_cnt(%d) < 0", ipmc_mem_cnt);
            for (;;) {}
        }

        if ((type == IPMC_MEM_GROUP) || (type == IPMC_MEM_GRP_INFO) ||
            (type == IPMC_MEM_SRC_LIST) || (type == IPMC_MEM_CTRL_HDR) ||
            (type == IPMC_MEM_RULES) || (type == IPMC_MEM_PROFILE)) {
            ipmc_lib_mem_obj_t  *mem_obj = &ipmc_mem_pool.obj;

            if (type == IPMC_MEM_GROUP) {
                if (!(((ipmc_group_entry_t *)ptr)->mflag)) {
                    free_status = FALSE;
                    goto ipmc_lib_mem_free_done;
                }

                if (((ipmc_group_entry_t *)ptr)->prev) {
                    ((ipmc_group_entry_t *)ptr)->prev->next = ((ipmc_group_entry_t *)ptr)->next;
                    if (((ipmc_group_entry_t *)ptr)->next) {
                        ((ipmc_group_entry_t *)ptr)->next->prev = ((ipmc_group_entry_t *)ptr)->prev;
                    }
                } else {
                    if (((ipmc_group_entry_t *)ptr)->next) {
                        ((ipmc_group_entry_t *)ptr)->next->prev = NULL;
                        mem_obj->grp_used = ((ipmc_group_entry_t *)ptr)->next;
                    } else {
                        mem_obj->grp_used = NULL;
                    }
                }

                ((ipmc_group_entry_t *)ptr)->next = mem_obj->grp_free;
                ((ipmc_group_entry_t *)ptr)->prev = NULL;
                ((ipmc_group_entry_t *)ptr)->mflag = FALSE;
                mem_obj->grp_free = (ipmc_group_entry_t *)ptr;
                mem_obj->grp_free->prev = NULL;

                mem_obj->grp_count--;
            }
            if (type == IPMC_MEM_GRP_INFO) {
                if (!(((ipmc_group_info_t *)ptr)->mflag)) {
                    free_status = FALSE;
                    goto ipmc_lib_mem_free_done;
                }

                ((ipmc_group_info_t *)ptr)->next = mem_obj->info_free;
                ((ipmc_group_info_t *)ptr)->mflag = FALSE;
                mem_obj->info_free = (ipmc_group_info_t *)ptr;

                mem_obj->info_count--;
            }
            if (type == IPMC_MEM_CTRL_HDR) {
                if (!(((ipmc_db_ctrl_hdr_t *)ptr)->mflag)) {
                    free_status = FALSE;
                    goto ipmc_lib_mem_free_done;
                }

                ((ipmc_db_ctrl_hdr_t *)ptr)->next = mem_obj->h_free;
                ((ipmc_db_ctrl_hdr_t *)ptr)->mflag = FALSE;
                mem_obj->h_free = (ipmc_db_ctrl_hdr_t *)ptr;

                mem_obj->h_count--;
            }
            if (type == IPMC_MEM_SRC_LIST) {
                if (!(((ipmc_sfm_srclist_t *)ptr)->mflag)) {
                    free_status = FALSE;
                    chkid = ((ipmc_sfm_srclist_t *)ptr)->freid;
                    alcid = ((ipmc_sfm_srclist_t *)ptr)->alcid;
                    goto ipmc_lib_mem_free_done;
                }

                ((ipmc_sfm_srclist_t *)ptr)->next = mem_obj->sl_free;
                ((ipmc_sfm_srclist_t *)ptr)->mflag = FALSE;
                ((ipmc_sfm_srclist_t *)ptr)->freid = freid;
                mem_obj->sl_free = (ipmc_sfm_srclist_t *)ptr;

                mem_obj->sl_count--;
            }
            if (type == IPMC_MEM_RULES) {
                if (!(((ipmc_profile_rule_t *)ptr)->mflag)) {
                    free_status = FALSE;
                    goto ipmc_lib_mem_free_done;
                }

                ((ipmc_profile_rule_t *)ptr)->next = mem_obj->rules_free;
                ((ipmc_profile_rule_t *)ptr)->mflag = FALSE;
                mem_obj->rules_free = (ipmc_profile_rule_t *)ptr;

                mem_obj->rules_count--;
            }
            if (type == IPMC_MEM_PROFILE) {
                if (!(((ipmc_lib_profile_mem_t *)ptr)->mflag)) {
                    free_status = FALSE;
                    goto ipmc_lib_mem_free_done;
                }

                ((ipmc_lib_profile_mem_t *)ptr)->next = mem_obj->profile_free;
                ((ipmc_lib_profile_mem_t *)ptr)->mflag = FALSE;
                mem_obj->profile_free = (ipmc_lib_profile_mem_t *)ptr;

                mem_obj->profile_count--;
            }
        } else if (ipmc_lib_memory_status[type].fixed) {
            free_status = ipmc_lib_memory_free(IPMC_MEM_PARTITIONED,
                                               ipmc_lib_memory_status[type].idx,
                                               ptr);
        } else {
            if (type != IPMC_MEM_OS_MALLOC) {
                free_status = ipmc_lib_memory_free(IPMC_MEM_DYNA_POOL,
                                                   ipmc_lib_memory_status[type].idx,
                                                   ptr);
            } else {
                free_status = ipmc_lib_memory_free(IPMC_MEM_SYS_MALLOC,
                                                   ipmc_lib_memory_status[type].idx,
                                                   ptr);
            }
        }

ipmc_lib_mem_free_done:
        if (!free_status) {
            T_D("Failure in freeing %s memory (Last:%u/Curr:%u/Aloc:%u)", ipmc_lib_mem_id_txt(type), chkid, freid, alcid);
        }

        IPMC_LIB_CRIT_EXIT();
        return free_status;
    }
    IPMC_LIB_CRIT_EXIT();
    return FALSE;
}

u8 *ipmc_lib_mem_alloc(ipmc_mem_t type, size_t size, u8 alcid)
{
    u8  *ptr;
    u32 cur_cnt, max_cnt;

    if (!ipmc_lib_cmn_done_init) {
        T_E("Not Initialized Yet!");
        return NULL;
    }

    IPMC_LIB_CRIT_ENTER();
    if (!ipmc_lib_memory_status[type].valid) {
        IPMC_LIB_CRIT_EXIT();
        T_W("%s(%d) is not a valid IPMC_MEM type", ipmc_lib_mem_id_txt(type), type);
        return NULL;
    }

    if (size) {
        ptr = NULL;
        cur_cnt = max_cnt = 0;

        if ((type == IPMC_MEM_GROUP) || (type == IPMC_MEM_GRP_INFO) ||
            (type == IPMC_MEM_SRC_LIST) || (type == IPMC_MEM_CTRL_HDR) ||
            (type == IPMC_MEM_RULES) || (type == IPMC_MEM_PROFILE)) {
            ipmc_lib_mem_obj_t  *mem_obj = &ipmc_mem_pool.obj;

            if ((type == IPMC_MEM_GROUP) &&
                ((cur_cnt = mem_obj->grp_count) < (max_cnt = mem_obj->grp_max))) {
                ipmc_group_entry_t  *grp = mem_obj->grp_free;

                if (grp->mflag) {
                    goto ipmc_lib_mem_alloc_done;
                }

                mem_obj->grp_free = grp->next;

                if (mem_obj->grp_used) {
                    mem_obj->grp_used->prev = grp;
                    grp->next = mem_obj->grp_used;
                    grp->prev = NULL;
                } else {
                    grp->next = grp->prev = NULL;
                }
                grp->mflag = TRUE;
                mem_obj->grp_used = grp;

                ptr = (u8 *)grp;
                mem_obj->grp_count++;
            }
            if ((type == IPMC_MEM_GRP_INFO) &&
                ((cur_cnt = mem_obj->info_count) < (max_cnt = mem_obj->info_max))) {
                ipmc_group_info_t   *inf = mem_obj->info_free;

                if (inf->mflag) {
                    goto ipmc_lib_mem_alloc_done;
                }

                mem_obj->info_free = inf->next;
                inf->next = NULL;
                inf->mflag = TRUE;

                ptr = (u8 *)inf;
                mem_obj->info_count++;
            }
            if ((type == IPMC_MEM_CTRL_HDR) &&
                ((cur_cnt = mem_obj->h_count) < (max_cnt = mem_obj->h_max))) {
                ipmc_db_ctrl_hdr_t  *hdr = mem_obj->h_free;

                if (hdr->mflag) {
                    goto ipmc_lib_mem_alloc_done;
                }

                mem_obj->h_free = hdr->next;
                hdr->next = NULL;
                hdr->mflag = TRUE;

                ptr = (u8 *)hdr;
                mem_obj->h_count++;
            }
            if ((type == IPMC_MEM_SRC_LIST) &&
                ((cur_cnt = mem_obj->sl_count) < (max_cnt = mem_obj->sl_max))) {
                ipmc_sfm_srclist_t  *src = mem_obj->sl_free;

                if (src->mflag) {
                    T_W("ALCID:Curr->%u/Last->%u;FREID->%u", alcid, src->alcid, src->freid);
                    goto ipmc_lib_mem_alloc_done;
                }

                mem_obj->sl_free = src->next;
                src->next = NULL;
                src->mflag = TRUE;
                src->freid = 0;
                src->alcid = alcid;

                ptr = (u8 *)src;
                mem_obj->sl_count++;
            }
            if ((type == IPMC_MEM_RULES) &&
                ((cur_cnt = mem_obj->rules_count) < (max_cnt = mem_obj->rules_max))) {
                ipmc_profile_rule_t *rul = mem_obj->rules_free;

                if (rul->mflag) {
                    goto ipmc_lib_mem_alloc_done;
                }

                mem_obj->rules_free = rul->next;
                rul->next = NULL;
                rul->mflag = TRUE;

                ptr = (u8 *)rul;
                mem_obj->rules_count++;
            }
            if ((type == IPMC_MEM_PROFILE) &&
                ((cur_cnt = mem_obj->profile_count) < (max_cnt = mem_obj->profile_max))) {
                ipmc_lib_profile_mem_t  *prf = mem_obj->profile_free;

                if (prf->mflag) {
                    goto ipmc_lib_mem_alloc_done;
                }

                mem_obj->profile_free = prf->next;
                prf->next = NULL;
                prf->mflag = TRUE;

                ptr = (u8 *)prf;
                mem_obj->profile_count++;
            }
        } else if (ipmc_lib_memory_status[type].fixed) {
            if (ipmc_lib_memory_status[type].sz_partition == size) {
                ptr = ipmc_lib_memory_allocate(IPMC_MEM_PARTITIONED,
                                               ipmc_lib_memory_status[type].idx,
                                               size);
            } else {
                T_W("%s Size Mismatch: %d != %d",
                    ipmc_lib_mem_id_txt(type),
                    ipmc_lib_memory_status[type].sz_partition,
                    size);

                IPMC_LIB_CRIT_EXIT();
                return NULL;
            }
        } else {
            if (type != IPMC_MEM_OS_MALLOC) {
                ptr = ipmc_lib_memory_allocate(IPMC_MEM_DYNA_POOL,
                                               ipmc_lib_memory_status[type].idx,
                                               size);
            } else {
                ptr = ipmc_lib_memory_allocate(IPMC_MEM_SYS_MALLOC,
                                               ipmc_lib_memory_status[type].idx,
                                               size);
            }
        }

ipmc_lib_mem_alloc_done:
        if (ptr != NULL) {
            ipmc_mem_cnt++;
        } else {
            if (max_cnt) {
                if (cur_cnt < max_cnt) {
                    T_W("Failure in allocate %s (CUR/MAX:%u/%u)", ipmc_lib_mem_id_txt(type), cur_cnt, max_cnt);
                } else {
                    T_W("Memory full for %s (CUR/MAX:%u/%u)", ipmc_lib_mem_id_txt(type), cur_cnt, max_cnt);
                }
            } else {
                T_W("Failure in allocate %s (CUR/MAX:%u/%u)", ipmc_lib_mem_id_txt(type), cur_cnt, max_cnt);
            }
        }

        IPMC_LIB_CRIT_EXIT();
        return ptr;
    } else {
        IPMC_LIB_CRIT_EXIT();
        return NULL;
    }
}

/*
    A) Characters listed from ASCII code Dec#33 ~ Dec#126 should be valid inputs.
    B) Characters that against the reserved input characters defined in UI MAY be invalid.
    C) There must exist at least one character derived from (A) & (B) to present a valid name.
    D) Empty (No character) is allowed to denote the parameter is not set while the specific parameter is not used as an index.
*/
static BOOL _ipmc_lib_instance_valid_string(const i8 *const input,
                                            const int str_len,
                                            const BOOL as_idx,
                                            const BOOL inc_sp)
{
    int nmx;

    if (!input) {
        return FALSE;
    }

    if ((nmx = str_len) == 0) {
        if (as_idx) {
            return FALSE;
        } else {
            return TRUE;
        }
    }

    --nmx;
    while (!(nmx < 0)) {
        if ((input[nmx] < IPMC_LIB_CHAR_ASCII_MIN) || (input[nmx] > IPMC_LIB_CHAR_ASCII_MAX)) {
            if (inc_sp) {
                if (input[nmx] != IPMC_LIB_CHAR_ASCII_SPACE) {
                    return FALSE;
                }
            } else {
                return FALSE;
            }
        }

        --nmx;
    }

    return TRUE;
}

BOOL ipmc_lib_instance_name_check(char *inst_name, i32 name_len, BOOL as_idx)
{
    int     str_len;

    if (!inst_name) {
        return FALSE;
    }

    if ((str_len = strlen(inst_name)) >= name_len) {
        return FALSE;
    }

    return _ipmc_lib_instance_valid_string(inst_name, str_len, as_idx, FALSE);
}

BOOL ipmc_lib_instance_desc_check(char *inst_desc, i32 desc_len)
{
    int     str_len;

    if (!inst_desc) {
        return FALSE;
    }

    if ((str_len = strlen(inst_desc)) >= desc_len) {
        return FALSE;
    }

    return _ipmc_lib_instance_valid_string(inst_desc, str_len, FALSE, TRUE);
}

i32 ipmc_lib_addrs_cmp_func(void *elm1, void *elm2)
{
    int         cmp;
    vtss_ipv6_t *element1, *element2;

    if (!elm1 || !elm2) {
        T_W("IPMCLIB_ASSERT(ipmc_lib_srclist_cmp_func)");
        for (;;) {}
    }

    element1 = (vtss_ipv6_t *)elm1;
    element2 = (vtss_ipv6_t *)elm2;

    cmp = IPMC_LIB_ADRS_CMP(element1, element2);
    if (cmp > 0) {
        return 1;
    } else if (cmp < 0) {
        return -1;
    } else {
        return 0;
    }
}

i32 ipmc_lib_srclist_cmp_func(void *elm1, void *elm2)
{
    int                 cmp;
    ipmc_sfm_srclist_t  *element1, *element2;

    if (!elm1 || !elm2) {
        T_W("IPMCLIB_ASSERT(ipmc_lib_srclist_cmp_func)");
        for (;;) {}
    }

    element1 = (ipmc_sfm_srclist_t *)elm1;
    element2 = (ipmc_sfm_srclist_t *)elm2;

    cmp = IPMC_LIB_ADRS_CMP(&element1->src_ip, &element2->src_ip);
    if (cmp > 0) {
        return 1;
    } else if (cmp < 0) {
        return -1;
    } else {
        return 0;
    }
}

BOOL ipmc_lib_get_port_rpstatus(ipmc_ip_version_t version, u32 port)
{
    return VTSS_PORT_BF_GET(router_port_mask[version], port);
}

void ipmc_lib_get_discovered_router_port_mask(ipmc_ip_version_t version, ipmc_port_bfs_t *port_mask)
{
    u32 i, local_port_cnt;

    local_port_cnt = ipmc_lib_get_system_local_port_cnt();
    for (i = 0; i < local_port_cnt; i++) {
        VTSS_PORT_BF_SET(port_mask->member_ports, i, VTSS_PORT_BF_GET(router_port_mask[version], i));
    }
}

void ipmc_lib_set_discovered_router_port_mask(ipmc_ip_version_t version, u32 port, BOOL state)
{
    if (!ipmc_lib_cmn_done_init) {
        return;
    }

    VTSS_PORT_BF_SET(router_port_mask[version], port, state);
}

BOOL ipmc_lib_get_changed_rpstatus(ipmc_ip_version_t version, u32 port)
{
    return VTSS_PORT_BF_GET(diff_router_port_mask[version], port);
}

void ipmc_lib_set_changed_router_port(BOOL init, ipmc_ip_version_t version, u32 port)
{
    if (!ipmc_lib_cmn_done_init) {
        return;
    }

    if (init) {
        VTSS_PORT_BF_CLR(diff_router_port_mask[version]);
    } else {
        VTSS_PORT_BF_SET(diff_router_port_mask[version], port, TRUE);
    }
}

ipmc_bf_status ipmc_lib_bf_status_check(u8 *dst_ptr)
{
    u32         idx, local_port_cnt;
#if VTSS_SWITCH_STACKABLE
    BOOL        done = FALSE;
#endif /* VTSS_SWITCH_STACKABLE */
    local_port_cnt = ipmc_lib_get_system_local_port_cnt();
    for (idx = 0; idx < local_port_cnt; idx++) {
#if VTSS_SWITCH_STACKABLE
        if ((idx == PORT_NO_STACK_0) || (idx == PORT_NO_STACK_1)) {
            /* Bypass stacking port first */
            continue;
        } else {
            if (VTSS_PORT_BF_GET(dst_ptr, idx)) {
                done = TRUE;
                break;
            }
        }
#else
        if (VTSS_PORT_BF_GET(dst_ptr, idx)) {
            return IPMC_BF_HASMEMBER;
        }
#endif /* VTSS_SWITCH_STACKABLE */
    }

#if VTSS_SWITCH_STACKABLE
    if (done) {
        return IPMC_BF_HASMEMBER;
    } else {
        /* Check stacking port now/then */
        idx = PORT_NO_STACK_0;
        if (VTSS_PORT_BF_GET(dst_ptr, idx)) {
            return IPMC_BF_SEMIEMPTY;
        }
        idx = PORT_NO_STACK_1;
        if (VTSS_PORT_BF_GET(dst_ptr, idx)) {
            return IPMC_BF_SEMIEMPTY;
        }
    }
#endif /* VTSS_SWITCH_STACKABLE */

    return IPMC_BF_EMPTY;
}

char *ipmc_lib_op_action_txt(ipmc_operation_action_t op)
{
    char    *txt;

    switch ( op ) {
    case IPMC_OP_INT:
        txt = "OP-INT";
        break;
    case IPMC_OP_SET:
        txt = "OP-SET";
        break;
    case IPMC_OP_ADD:
        txt = "OP-ADD";
        break;
    case IPMC_OP_DEL:
        txt = "OP-DEL";
        break;
    case IPMC_OP_UPD:
        txt = "OP-UPD";
        break;
    case IPMC_OP_ERR:
    default:
        txt = "OP-ERR";
        break;
    }

    return txt;
}

char *ipmc_lib_fltr_action_txt(ipmc_action_t action, ipmc_text_cap_t cap)
{
    char    *txt;

    switch ( action ) {
    case IPMC_ACTION_DENY:
        if (cap == IPMC_TXT_CASE_LOWER) {
            txt = "deny";
        } else if (cap == IPMC_TXT_CASE_UPPER) {
            txt = "DENY";
        } else {
            txt = "Deny";
        }
        break;
    case IPMC_ACTION_PERMIT:
        if (cap == IPMC_TXT_CASE_LOWER) {
            txt = "permit";
        } else if (cap == IPMC_TXT_CASE_UPPER) {
            txt = "PERMIT";
        } else {
            txt = "Permit";
        }
        break;
    default:
        if (cap == IPMC_TXT_CASE_LOWER) {
            txt = "action-?";
        } else if (cap == IPMC_TXT_CASE_UPPER) {
            txt = "ACTION-?";
        } else {
            txt = "Action-?";
        }
        break;
    }

    return txt;
}

char *ipmc_lib_version_txt(ipmc_ip_version_t version, ipmc_text_cap_t cap)
{
    char    *txt;

    switch ( version ) {
    case IPMC_IP_VERSION_ALL:
        if (cap == IPMC_TXT_CASE_LOWER) {
            txt = "hybrid";
        } else if (cap == IPMC_TXT_CASE_UPPER) {
            txt = "HYBRID";
        } else {
            txt = "Hybrid";
        }
        break;
    case IPMC_IP_VERSION_IGMP:
        if (cap == IPMC_TXT_CASE_LOWER) {
            txt = "igmp";
        } else if (cap == IPMC_TXT_CASE_UPPER) {
            txt = "IGMP";
        } else {
            txt = "Igmp";
        }
        break;
    case IPMC_IP_VERSION_MLD:
        if (cap == IPMC_TXT_CASE_LOWER) {
            txt = "mld";
        } else if (cap == IPMC_TXT_CASE_UPPER) {
            txt = "MLD";
        } else {
            txt = "Mld";
        }
        break;
    case IPMC_IP_VERSION_IPV4Z:
        if (cap == IPMC_TXT_CASE_LOWER) {
            txt = "igmpz";
        } else if (cap == IPMC_TXT_CASE_UPPER) {
            txt = "IGMPZ";
        } else {
            txt = "IGMPz";
        }
        break;
    case IPMC_IP_VERSION_IPV6Z:
        if (cap == IPMC_TXT_CASE_LOWER) {
            txt = "mldz";
        } else if (cap == IPMC_TXT_CASE_UPPER) {
            txt = "MLDZ";
        } else {
            txt = "MLDz";
        }
        break;
    case IPMC_IP_VERSION_DNS:
        if (cap == IPMC_TXT_CASE_LOWER) {
            txt = "ver-dns";
        } else if (cap == IPMC_TXT_CASE_UPPER) {
            txt = "VER-DNS";
        } else {
            txt = "Ver-DNS";
        }
        break;
    case IPMC_IP_VERSION_INIT:
        if (cap == IPMC_TXT_CASE_LOWER) {
            txt = "ver-ini";
        } else if (cap == IPMC_TXT_CASE_UPPER) {
            txt = "VER-INI";
        } else {
            txt = "Ver-INI";
        }
        break;
    case IPMC_IP_VERSION_ERR:
    default:
        if (cap == IPMC_TXT_CASE_LOWER) {
            txt = "ver-err";
        } else if (cap == IPMC_TXT_CASE_UPPER) {
            txt = "VER-ERR";
        } else {
            txt = "Ver-ERR";
        }
        break;
    }

    return txt;
}

char *ipmc_lib_ip_txt(ipmc_ip_version_t version, ipmc_text_cap_t cap)
{
    char    *txt;

    switch ( version ) {
    case IPMC_IP_VERSION_ALL:
        if (cap == IPMC_TXT_CASE_LOWER) {
            txt = "ipv4/ipv6";
        } else if (cap == IPMC_TXT_CASE_UPPER) {
            txt = "IPV4/IPV6";
        } else {
            txt = "IPv4/IPv6";
        }
        break;
    case IPMC_IP_VERSION_IGMP:
    case IPMC_IP_VERSION_IPV4Z:
        if (cap == IPMC_TXT_CASE_LOWER) {
            txt = "ip";
        } else if (cap == IPMC_TXT_CASE_UPPER) {
            txt = "IP";
        } else {
            txt = "IPv4";
        }
        break;
    case IPMC_IP_VERSION_MLD:
    case IPMC_IP_VERSION_IPV6Z:
        if (cap == IPMC_TXT_CASE_LOWER) {
            txt = "ipv6";
        } else if (cap == IPMC_TXT_CASE_UPPER) {
            txt = "IPV6";
        } else {
            txt = "IPv6";
        }
        break;
    case IPMC_IP_VERSION_DNS:
        if (cap == IPMC_TXT_CASE_LOWER) {
            txt = "dns";
        } else {
            txt = "DNS";
        }
        break;
    case IPMC_IP_VERSION_INIT:
    case IPMC_IP_VERSION_ERR:
    default:
        if (cap == IPMC_TXT_CASE_LOWER) {
            txt = "unknown";
        } else if (cap == IPMC_TXT_CASE_UPPER) {
            txt = "UNKNOWN";
        } else {
            txt = "Unknown";
        }
        break;
    }

    return txt;
}

char *ipmc_lib_severity_txt(ipmc_log_severity_t severity, ipmc_text_cap_t cap)
{
    char    *txt;

    switch ( severity ) {
    case IPMC_SEVERITY_Normal:
        if (cap == IPMC_TXT_CASE_LOWER) {
            txt = "normal";
        } else if (cap == IPMC_TXT_CASE_UPPER) {
            txt = "NORMAL";
        } else {
            txt = "Normal";
        }
        break;
    case IPMC_SEVERITY_InfoDebug:
        if (cap == IPMC_TXT_CASE_LOWER) {
            txt = "info|debug";
        } else if (cap == IPMC_TXT_CASE_UPPER) {
            txt = "INFO|DEBUG";
        } else {
            txt = "Info|Debug";
        }
        break;
    case IPMC_SEVERITY_Notice:
        if (cap == IPMC_TXT_CASE_LOWER) {
            txt = "notice";
        } else if (cap == IPMC_TXT_CASE_UPPER) {
            txt = "NOTICE";
        } else {
            txt = "Notice";
        }
        break;
    case IPMC_SEVERITY_Warning:
        if (cap == IPMC_TXT_CASE_LOWER) {
            txt = "warning";
        } else if (cap == IPMC_TXT_CASE_UPPER) {
            txt = "WARNING";
        } else {
            txt = "Warning";
        }
        break;
    case IPMC_SEVERITY_Error:
        if (cap == IPMC_TXT_CASE_LOWER) {
            txt = "error";
        } else if (cap == IPMC_TXT_CASE_UPPER) {
            txt = "ERROR";
        } else {
            txt = "Error";
        }
        break;
    case IPMC_SEVERITY_Critical:
        if (cap == IPMC_TXT_CASE_LOWER) {
            txt = "critical";
        } else if (cap == IPMC_TXT_CASE_UPPER) {
            txt = "CRITICAL";
        } else {
            txt = "Critical";
        }
        break;
    case IPMC_SEVERITY_Alert:
        if (cap == IPMC_TXT_CASE_LOWER) {
            txt = "alert";
        } else if (cap == IPMC_TXT_CASE_UPPER) {
            txt = "ALERT";
        } else {
            txt = "Alert";
        }
        break;
    case IPMC_SEVERITY_Emergency:
        if (cap == IPMC_TXT_CASE_LOWER) {
            txt = "emergency";
        } else if (cap == IPMC_TXT_CASE_UPPER) {
            txt = "EMERGENCY";
        } else {
            txt = "Emergency";
        }
        break;
    default:
        if (cap == IPMC_TXT_CASE_LOWER) {
            txt = "unknown";
        } else if (cap == IPMC_TXT_CASE_UPPER) {
            txt = "UNKNOWN";
        } else {
            txt = "Unknown";
        }
        break;
    }

    return txt;
}

char *ipmc_lib_port_role_txt(mvr_port_role_t role, ipmc_text_cap_t cap)
{
    char    *txt;

    switch ( role ) {
    case MVR_PORT_ROLE_SOURC:
        if (cap == IPMC_TXT_CASE_LOWER) {
            txt = "source";
        } else if (cap == IPMC_TXT_CASE_UPPER) {
            txt = "SOURCE";
        } else {
            txt = "Source";
        }
        break;
    case MVR_PORT_ROLE_RECVR:
        if (cap == IPMC_TXT_CASE_LOWER) {
            txt = "receiver";
        } else if (cap == IPMC_TXT_CASE_UPPER) {
            txt = "RECEIVER";
        } else {
            txt = "Receiver";
        }
        break;
    case MVR_PORT_ROLE_STACK:
        if (cap == IPMC_TXT_CASE_LOWER) {
            txt = "stacking";
        } else if (cap == IPMC_TXT_CASE_UPPER) {
            txt = "STACKING";
        } else {
            txt = "Stacking";
        }
        break;
    case MVR_PORT_ROLE_INACT:
    default:
        if (cap == IPMC_TXT_CASE_LOWER) {
            txt = "inactive";
        } else if (cap == IPMC_TXT_CASE_UPPER) {
            txt = "INACTIVE";
        } else {
            txt = "Inactive";
        }
        break;
    }

    return txt;
}

char *ipmc_lib_frm_tagtype_txt(vtss_tag_type_t type, ipmc_text_cap_t cap)
{
    char    *txt;

    switch ( type ) {
    case VTSS_TAG_TYPE_UNTAGGED:
        if (cap == IPMC_TXT_CASE_LOWER) {
            txt = "untagged";
        } else if (cap == IPMC_TXT_CASE_UPPER) {
            txt = "UNTAGGED";
        } else {
            txt = "Untagged";
        }
        break;
    case VTSS_TAG_TYPE_C_TAGGED:
        if (cap == IPMC_TXT_CASE_LOWER) {
            txt = "c-tagged";
        } else if (cap == IPMC_TXT_CASE_UPPER) {
            txt = "C-TAGGED";
        } else {
            txt = "C-Tagged";
        }
        break;
    case VTSS_TAG_TYPE_S_TAGGED:
        if (cap == IPMC_TXT_CASE_LOWER) {
            txt = "s-tagged";
        } else if (cap == IPMC_TXT_CASE_UPPER) {
            txt = "S-TAGGED";
        } else {
            txt = "S-Tagged";
        }
        break;
    case VTSS_TAG_TYPE_S_CUSTOM_TAGGED:
        if (cap == IPMC_TXT_CASE_LOWER) {
            txt = "custom-s-tagged";
        } else if (cap == IPMC_TXT_CASE_UPPER) {
            txt = "CUSTOM-S-TAGGED";
        } else {
            txt = "Custom-S-Tagged";
        }
        break;
    default:
        if (cap == IPMC_TXT_CASE_LOWER) {
            txt = "unknown";
        } else if (cap == IPMC_TXT_CASE_UPPER) {
            txt = "UNKNOWN";
        } else {
            txt = "Unknown";
        }
        break;
    }

    return txt;
}

ipmc_ip_version_t ipmc_lib_grp_adrs_version(vtss_ipv6_t *grps)
{
    u8      idx;
    BOOL    chk4;

    if (!grps) {
        return IPMC_IP_VERSION_ERR;
    }

    chk4 = FALSE;
    for (idx = 0; idx < sizeof(vtss_ipv6_t); idx++) {
        if (!chk4) {
            /* IPv6 Multicast: Leading 0x11111111 */
            if (grps->addr[idx] == 0xFF) {
                return IPMC_IP_VERSION_MLD;
            } else {
                if (grps->addr[idx] != 0x0) {
                    return IPMC_IP_VERSION_ERR;
                } else {
                    chk4 = TRUE;
                }
            }
        } else {
            if (idx < 12) {
                if (grps->addr[idx] != 0x0) {
                    return IPMC_IP_VERSION_ERR;
                }
            } else {
                if (idx == 12) {
                    /* IPv4 Multicast: Leading 0x1110 */
                    if ((grps->addr[idx] & 0xE0) != 0xE0) {
                        return IPMC_IP_VERSION_ERR;
                    } else {
                        return IPMC_IP_VERSION_IGMP;
                    }
                } else {
                    break;
                }
            }
        }
    }

    return IPMC_IP_VERSION_ERR;
}

BOOL ipmc_lib_grp_adrs_boundary(ipmc_ip_version_t version, BOOL floor, vtss_ipv6_t *grps)
{
    BOOL    chk;
    u8      idx, lower, upper;

    if (!grps) {
        return FALSE;
    }

    lower = upper = idx = 0;
    if (version == IPMC_IP_VERSION_IGMP) {
        idx = 12;
        if (floor) {
            lower = 0xE0;
        } else {
            upper = 0xEF;
        }
    } else if (version == IPMC_IP_VERSION_MLD) {
        if (floor) {
            lower = 0xFF;
        } else {
            upper = 0xFF;
        }
    } else {
        return FALSE;
    }

    chk = FALSE;
    for (; idx < sizeof(vtss_ipv6_t); idx++) {
        if (lower) {
            if (!chk) {
                if (grps->addr[idx] != lower) {
                    break;
                }
                chk = TRUE;
            } else {
                if (grps->addr[idx]) {
                    chk = FALSE;
                    break;
                }
            }
        } else if (upper) {
            if (!chk) {
                if (grps->addr[idx] != upper) {
                    break;
                }
                chk = TRUE;
            } else {
                if (grps->addr[idx] != 0xFF) {
                    chk = FALSE;
                    break;
                }
            }
        }
    }

    return chk;
}

BOOL ipmc_lib_prefix_matching(ipmc_ip_version_t version, BOOL mc, ipmc_prefix_t *pfx1, ipmc_prefix_t *pfx2)
{
    vtss_ipv6_t *ipmc_addr1, *ipmc_addr2;
    u32         mask_len, idx;

    if (!pfx1 || !pfx2) {
        return FALSE;
    }

    if (pfx1->len != pfx2->len) {
        return FALSE;
    }

    ipmc_addr1 = &pfx1->addr.array.prefix;
    ipmc_addr2 = &pfx2->addr.array.prefix;

    if (version == IPMC_IP_VERSION_IGMP) {
        if ((pfx1->len < IPMC_ADDR_V4_MIN_BIT_LEN) || (pfx1->len > IPMC_ADDR_V4_MAX_BIT_LEN)) {
            return FALSE;
        }

        /* IPv4 Multicast: Leading 0x1110 */
        if (mc &&
            (((ipmc_addr1->addr[12] & 0xE0) != 0xE0) ||
             ((ipmc_addr2->addr[12] & 0xE0) != 0xE0))) {
            return FALSE;
        }

        mask_len = IPMC_ADDR_4IN6_SHIFT_BIT_LEN + pfx1->len;
    } else if (version == IPMC_IP_VERSION_MLD) {
        if ((pfx1->len < IPMC_ADDR_V6_MIN_BIT_LEN) || (pfx1->len > IPMC_ADDR_V6_MAX_BIT_LEN)) {
            return FALSE;
        }

        /* IPv6 Multicast: Leading 0x11111111 */
        if (mc &&
            ((ipmc_addr1->addr[0] != 0xFF) ||
             (ipmc_addr2->addr[0] != 0xFF))) {
            return FALSE;
        }

        mask_len = pfx1->len;
    } else {
        return FALSE;
    }

    for (idx = 0; idx < IPMC_ADDR_BYTE_LEN; idx++) {
        if (mask_len < IPMC_ADDR_MIN_BIT_LEN) {
            if (mask_len) {
                if (((ipmc_addr1->addr[idx] >> (IPMC_ADDR_MIN_BIT_LEN - mask_len)) << (IPMC_ADDR_MIN_BIT_LEN - mask_len)) !=
                    ((ipmc_addr2->addr[idx] >> (IPMC_ADDR_MIN_BIT_LEN - mask_len)) << (IPMC_ADDR_MIN_BIT_LEN - mask_len))) {
                    return FALSE;
                }
            } else {
                if (ipmc_addr1->addr[idx] != ipmc_addr2->addr[idx]) {
                    return FALSE;
                }
            }

            break;
        } else {
            if (ipmc_addr1->addr[idx] != ipmc_addr2->addr[idx]) {
                return FALSE;
            }

            mask_len -= IPMC_ADDR_MIN_BIT_LEN;
        }
    }

    return TRUE;
}

BOOL ipmc_lib_prefix_maskingNchecking(ipmc_ip_version_t version, BOOL convert, ipmc_prefix_t *in, ipmc_prefix_t *out)
{
    ipmc_prefix_t prefix;
    vtss_ipv6_t   *ipmc_addr;
    u32           mask_len, idx;
    BOOL          mask_out;

    if (!in || !out) {
        return FALSE;
    }

    if (version == IPMC_IP_VERSION_IGMP) {
        if ((in->len < IPMC_ADDR_V4_MIN_BIT_LEN) || (in->len > IPMC_ADDR_V4_MAX_BIT_LEN)) {
            return FALSE;
        }
    } else if (version == IPMC_IP_VERSION_MLD) {
        if ((in->len < IPMC_ADDR_V6_MIN_BIT_LEN) || (in->len > IPMC_ADDR_V6_MAX_BIT_LEN)) {
            return FALSE;
        }
    } else {
        return FALSE;
    }

    memcpy(&prefix, in, sizeof(ipmc_prefix_t));
    if (version == IPMC_IP_VERSION_IGMP) {
        if (convert) {
            prefix.addr.value.prefix = htonl(prefix.addr.value.prefix);
        }

        /* IPv4 Multicast: Leading 0x1110 */
        if ((prefix.addr.array.prefix.addr[12] & 0xE0) != 0xE0) {
            return FALSE;
        }

        mask_len = IPMC_ADDR_4IN6_SHIFT_BIT_LEN + in->len;
    } else {
        /* IPv6 Multicast: Leading 0x11111111 */
        if (prefix.addr.array.prefix.addr[0] != 0xFF) {
            return FALSE;
        }

        mask_len = in->len;
    }

    ipmc_addr = &prefix.addr.array.prefix;
    mask_out = FALSE;
    for (idx = 0; idx < IPMC_ADDR_BYTE_LEN; idx++) {
        if (mask_len >= IPMC_ADDR_MAX_BIT_LEN) {
            break;
        } else {
            if (!mask_out) {
                if (mask_len < IPMC_ADDR_MIN_BIT_LEN) {
                    if (mask_len) {
                        ipmc_addr->addr[idx] = ipmc_addr->addr[idx] >> (IPMC_ADDR_MIN_BIT_LEN - mask_len);
                        ipmc_addr->addr[idx] = ipmc_addr->addr[idx] << (IPMC_ADDR_MIN_BIT_LEN - mask_len);
                        mask_len = 0;
                    } else {
                        ipmc_addr->addr[idx] = 0x0;
                    }

                    mask_out = TRUE;
                } else {
                    mask_len -= IPMC_ADDR_MIN_BIT_LEN;
                }

                continue;
            }
        }

        if (mask_out) {
            ipmc_addr->addr[idx] = 0x0;
        }
    }

    if (convert && (version == IPMC_IP_VERSION_IGMP)) {
        prefix.addr.value.prefix = ntohl(prefix.addr.value.prefix);
    }

    memcpy(out, &prefix, sizeof(ipmc_prefix_t));
    return TRUE;
}

ipmc_group_info_t *ipmc_lib_rxmt_tmrlist_get(ipmc_db_ctrl_hdr_t *p, ipmc_group_info_t *grp_info)
{
    ipmc_group_info_t   *ptr;

    if (!p || !grp_info) {
        return NULL;
    }

    ptr = grp_info;
    if (IPMC_LIB_DB_GET(p, ptr)) {
        return ptr;
    }

    return NULL;
}

ipmc_group_info_t *ipmc_lib_rxmt_tmrlist_get_next(ipmc_db_ctrl_hdr_t *p, ipmc_group_info_t *grp_info)
{
    ipmc_group_info_t   *ptr;

    if (!p) {
        return NULL;
    }

    if (!grp_info) {
        ptr = NULL;
        if (!IPMC_LIB_DB_GET_FIRST(p, ptr)) {
            ptr = NULL;
        }
    } else {
        ptr = grp_info;
        if (!IPMC_LIB_DB_GET_NEXT(p, ptr)) {
            ptr = NULL;
        }
    }

    return ptr;
}

ipmc_group_info_t *ipmc_lib_rxmt_tmrlist_walk(ipmc_db_ctrl_hdr_t *p, ipmc_group_info_t *grp_info, ipmc_time_t *current)
{
    ipmc_group_info_t   *ptr;

    if (!p || !current) {
        return NULL;
    }

    if (!grp_info) {
        ptr = NULL;
        if (!IPMC_LIB_DB_GET_FIRST(p, ptr)) {
            return NULL;
        }
    } else {
        ptr = grp_info;
        if (!IPMC_LIB_DB_GET_NEXT(p, ptr)) {
            return NULL;
        }
    }

    if (!ptr || IPMC_TIMER_GREATER(&ptr->min_tmr, current)) {
        return NULL;
    }

    return ptr;
}

ipmc_group_db_t *ipmc_lib_fltr_tmrlist_get(ipmc_db_ctrl_hdr_t *p, ipmc_group_db_t *grp_db)
{
    ipmc_group_db_t *ptr;

    if (!p || !grp_db) {
        return NULL;
    }

    ptr = grp_db;
    if (IPMC_LIB_DB_GET(p, ptr)) {
        return ptr;
    }

    return NULL;
}

ipmc_group_db_t *ipmc_lib_fltr_tmrlist_get_next(ipmc_db_ctrl_hdr_t *p, ipmc_group_db_t *grp_db)
{
    ipmc_group_db_t *ptr;

    if (!p) {
        return NULL;
    }

    if (!grp_db) {
        ptr = NULL;
        if (!IPMC_LIB_DB_GET_FIRST(p, ptr)) {
            ptr = NULL;
        }
    } else {
        ptr = grp_db;
        if (!IPMC_LIB_DB_GET_NEXT(p, ptr)) {
            ptr = NULL;
        }
    }

    return ptr;
}

ipmc_group_db_t *ipmc_lib_fltr_tmrlist_walk(ipmc_db_ctrl_hdr_t *p, ipmc_group_db_t *grp_db, ipmc_time_t *current)
{
    ipmc_group_db_t *ptr;

    if (!p || !current) {
        return NULL;
    }

    if (!grp_db) {
        ptr = NULL;
        if (!IPMC_LIB_DB_GET_FIRST(p, ptr)) {
            return NULL;
        }
    } else {
        ptr = grp_db;
        if (!IPMC_LIB_DB_GET_NEXT(p, ptr)) {
            return NULL;
        }
    }

    if (!ptr || IPMC_TIMER_GREATER(&ptr->min_tmr, current)) {
        return NULL;
    }

    return ptr;
}

ipmc_sfm_srclist_t *ipmc_lib_srct_tmrlist_get(ipmc_db_ctrl_hdr_t *p, ipmc_sfm_srclist_t *srclist)
{
    ipmc_sfm_srclist_t  *ptr;

    if (!p || !srclist) {
        return NULL;
    }

    ptr = srclist;
    if (IPMC_LIB_DB_GET(p, ptr)) {
        return ptr;
    }

    return NULL;
}

ipmc_sfm_srclist_t *ipmc_lib_srct_tmrlist_get_next(ipmc_db_ctrl_hdr_t *p, ipmc_sfm_srclist_t *srclist)
{
    ipmc_sfm_srclist_t  *ptr;

    if (!p) {
        return NULL;
    }

    if (!srclist) {
        ptr = NULL;
        if (!IPMC_LIB_DB_GET_FIRST(p, ptr)) {
            ptr = NULL;
        }
    } else {
        ptr = srclist;
        if (!IPMC_LIB_DB_GET_NEXT(p, ptr)) {
            ptr = NULL;
        }
    }

    return ptr;
}

ipmc_sfm_srclist_t *ipmc_lib_srct_tmrlist_walk(ipmc_db_ctrl_hdr_t *p, ipmc_sfm_srclist_t *srclist, ipmc_time_t *current)
{
    ipmc_sfm_srclist_t  *ptr;

    if (!p || !current) {
        return NULL;
    }

    if (!srclist) {
        ptr = NULL;
        if (!IPMC_LIB_DB_GET_FIRST(p, ptr)) {
            return NULL;
        }
    } else {
        ptr = srclist;
        if (!IPMC_LIB_DB_GET_NEXT(p, ptr)) {
            return NULL;
        }
    }

    if (!ptr || IPMC_TIMER_GREATER(&ptr->min_tmr, current)) {
        return NULL;
    }

    return ptr;
}

BOOL ipmc_lib_group_get_next(ipmc_db_ctrl_hdr_t *p, ipmc_group_entry_t *grp)
{
    ipmc_group_entry_t  *grp_get_ptr, grp_get_buf;

    if (!p || !grp) {
        return FALSE;
    }

    grp_get_ptr = &grp_get_buf;
    memcpy(grp_get_ptr, grp, sizeof(ipmc_group_entry_t));
    if (IPMC_LIB_DB_GET_NEXT(p, grp_get_ptr)) {
        memcpy(grp, grp_get_ptr, sizeof(ipmc_group_entry_t));
        return TRUE;
    }

    return FALSE;
}

BOOL ipmc_lib_group_get(ipmc_db_ctrl_hdr_t *p, ipmc_group_entry_t *grp)
{
    ipmc_group_entry_t  *grp_get_ptr, grp_get_buf;

    if (!p || !grp) {
        return FALSE;
    }

    grp_get_ptr = &grp_get_buf;
    memcpy(grp_get_ptr, grp, sizeof(ipmc_group_entry_t));
    if (IPMC_LIB_DB_GET(p, grp_get_ptr)) {
        memcpy(grp, grp_get_ptr, sizeof(ipmc_group_entry_t));
        return TRUE;
    }

    return FALSE;
}

ipmc_group_entry_t *ipmc_lib_group_ptr_get_first(ipmc_db_ctrl_hdr_t *p)
{
    ipmc_group_entry_t  *grp_get_ptr;

    if (!p) {
        return NULL;
    }

    grp_get_ptr = NULL;
    if (IPMC_LIB_DB_GET_FIRST(p, grp_get_ptr)) {
        return grp_get_ptr;
    }

    return NULL;
}

ipmc_group_entry_t *ipmc_lib_group_ptr_get(ipmc_db_ctrl_hdr_t *p, ipmc_group_entry_t *grp)
{
    ipmc_group_entry_t  *grp_get_ptr;

    if (!p || !grp) {
        return NULL;
    }

    grp_get_ptr = grp;
    if (IPMC_LIB_DB_GET(p, grp_get_ptr)) {
        return grp_get_ptr;
    }

    return NULL;
}

ipmc_group_entry_t *ipmc_lib_group_ptr_get_next(ipmc_db_ctrl_hdr_t *p, ipmc_group_entry_t *grp)
{
    ipmc_group_entry_t  *grp_get_ptr;

    if (!p) {
        return NULL;
    }

    if (!grp) {
        grp_get_ptr = NULL;
        if (IPMC_LIB_DB_GET_FIRST(p, grp_get_ptr)) {
            return grp_get_ptr;
        } else {
            return NULL;
        }
    }

    grp_get_ptr = grp;
    if (IPMC_LIB_DB_GET_NEXT(p, grp_get_ptr)) {
        return grp_get_ptr;
    }

    return NULL;
}

ipmc_sfm_srclist_t *ipmc_lib_srclist_adr_get(ipmc_db_ctrl_hdr_t *p, ipmc_sfm_srclist_t *srclist)
{
    ipmc_sfm_srclist_t  *sl_get_ptr;

    if (!p || !srclist) {
        return NULL;
    }

    sl_get_ptr = srclist;
    if (IPMC_LIB_DB_GET(p, sl_get_ptr)) {
        return sl_get_ptr;
    }

    return NULL;
}

ipmc_sfm_srclist_t *ipmc_lib_srclist_adr_get_next(ipmc_db_ctrl_hdr_t *p, ipmc_sfm_srclist_t *srclist)
{
    ipmc_sfm_srclist_t  *sl_get_ptr;

    if (!p) {
        return NULL;
    }

    if (!srclist) {
        sl_get_ptr = NULL;
        if (IPMC_LIB_DB_GET_FIRST(p, sl_get_ptr)) {
            return sl_get_ptr;
        } else {
            return NULL;
        }
    }

    sl_get_ptr = srclist;
    if (IPMC_LIB_DB_GET_NEXT(p, sl_get_ptr)) {
        return sl_get_ptr;
    }

    return NULL;
}

BOOL ipmc_lib_srclist_buf_get(ipmc_db_ctrl_hdr_t *p, ipmc_sfm_srclist_t *srclist)
{
    ipmc_sfm_srclist_t  *sl_get_ptr, srclist_get_buf;

    if (!p || !srclist) {
        return FALSE;
    }

    sl_get_ptr = &srclist_get_buf;
    memcpy(sl_get_ptr, srclist, sizeof(ipmc_sfm_srclist_t));
    if (IPMC_LIB_DB_GET(p, sl_get_ptr)) {
        if (sl_get_ptr != srclist) {
            memcpy(srclist, sl_get_ptr, sizeof(ipmc_sfm_srclist_t));
        }
        return TRUE;
    }

    return FALSE;
}

BOOL ipmc_lib_srclist_buf_get_next(ipmc_db_ctrl_hdr_t *p, ipmc_sfm_srclist_t *srclist)
{
    ipmc_sfm_srclist_t  *sl_get_ptr, srclist_get_buf;

    if (!p || !srclist) {
        return FALSE;
    }

    sl_get_ptr = &srclist_get_buf;
    memcpy(sl_get_ptr, srclist, sizeof(ipmc_sfm_srclist_t));
    if (IPMC_LIB_DB_GET_NEXT(p, sl_get_ptr)) {
        if (sl_get_ptr != srclist) {
            memcpy(srclist, sl_get_ptr, sizeof(ipmc_sfm_srclist_t));
        }
        return TRUE;
    }

    return FALSE;
}

BOOL ipmc_lib_srclist_add(ipmc_db_ctrl_hdr_t *p, ipmc_group_entry_t *grp, ipmc_sfm_srclist_t *srclist, u8 alcid)
{
    BOOL                src_mflag;
    ipmc_sfm_srclist_t  *entry;

    if (!p || !srclist) {
        T_W("p is %s;srclist is %s",
            p ? "OK" : "NULL",
            srclist ? "OK" : "NULL");
        return FALSE;
    }

    src_mflag = srclist->mflag;
    if (!IPMC_MEM_SL_MTAKE(entry, alcid)) {
        return FALSE;
    }

    if (entry != srclist) {
        memcpy(&entry->src_ip, &srclist->src_ip, sizeof(entry->src_ip));
        memcpy(entry->port_mask, srclist->port_mask, sizeof(entry->port_mask));
        memcpy(&entry->min_tmr, &srclist->min_tmr, sizeof(entry->min_tmr));
        memcpy(entry->tmr.srct_timer.t, srclist->tmr.srct_timer.t, sizeof(entry->tmr.srct_timer.t));
        entry->sfm_in_hw = srclist->sfm_in_hw;
    } else {
        T_D("entry(%u) == srclist(%u)", entry->mflag, src_mflag);
    }
    memset(entry->sf_calc, 0x0, sizeof(entry->sf_calc));
    if (grp) {
        entry->grp = grp;
    }

    if (!IPMC_LIB_DB_ADD(p, entry)) {
        BOOL    frest;

        T_W("IPMC_LIB_DB_ADD(SRC_LIST) failed!!!");
        IPMC_MEM_SL_MGIVE(entry, &frest, 202);
        return FALSE;
    }

    return TRUE;
}

BOOL ipmc_lib_srclist_del(ipmc_db_ctrl_hdr_t *p, ipmc_sfm_srclist_t *srclist, u8 freid)
{
    if (IPMC_LIB_DB_DEL(p, srclist)) {
        BOOL    frest;

        IPMC_MEM_SL_MGIVE(srclist, &frest, freid);
        return frest;
    } else {
        return FALSE;
    }
}

BOOL ipmc_lib_srclist_clear(ipmc_db_ctrl_hdr_t *p, u8 freid)
{
    ipmc_sfm_srclist_t  *entry, *free_ptr;
    BOOL                retVal, frest;
    u8                  freseq;

    if (!p) {
        return FALSE;
    }

    freseq = freid;
    retVal = TRUE;
    entry = free_ptr = NULL;
    IPMC_SRCLIST_WALK(p, entry) {
        if (free_ptr) {
            frest = TRUE;
            if (IPMC_LIB_DB_DEL(p, free_ptr)) {
                IPMC_MEM_SL_MGIVE(free_ptr, &frest, ++freseq);
            }
            if (!frest) {
                retVal = FALSE;
            }
            free_ptr = NULL;
        }

        free_ptr = entry;
    }
    if (free_ptr && IPMC_LIB_DB_DEL(p, free_ptr)) {
        frest = TRUE;
        IPMC_MEM_SL_MGIVE(free_ptr, &frest, ++freseq);
        if (!frest) {
            retVal = FALSE;
        }
    }

    return retVal;
}

ipmc_group_entry_t *ipmc_lib_group_ptr_walk_start(void)
{
    return ipmc_mem_pool.obj.grp_used;
}

ipmc_group_entry_t *ipmc_lib_group_init(ipmc_intf_entry_t *intf,
                                        ipmc_db_ctrl_hdr_t *p,
                                        ipmc_group_entry_t *grp)
{
    ipmc_group_entry_t  *grp_mem;
    ipmc_group_info_t   *inf_mem;
    ipmc_group_db_t     *grp_db;
    u32                 i, local_port_cnt;
    BOOL                frest;

    if (!p || !grp) {
        return NULL;
    }

    if (!IPMC_MEM_INFO_MTAKE(inf_mem)) {
        return NULL;
    }

    if (!IPMC_MEM_GRP_MTAKE(grp_mem)) {
        IPMC_MEM_INFO_MGIVE(inf_mem, &frest);
        return NULL;
    }

    grp_db = &inf_mem->db;
    memset(grp_db, 0x0, sizeof(ipmc_group_db_t));
    if (!IPMC_MEM_H_MTAKE(grp_db->ipmc_sf_do_forward_srclist)) {
        IPMC_MEM_INFO_MGIVE(inf_mem, &frest);
        IPMC_MEM_GRP_MGIVE(grp_mem, &frest);

        T_D("Allocating ipmc_sf_do_forward_srclist failed");
        return NULL;
    }
    if (!IPMC_LIB_DB_TAKE("GRP_SL_FWD", grp_db->ipmc_sf_do_forward_srclist,
                          IPMC_NO_OF_SUPPORTED_SRCLIST,
                          sizeof(ipmc_sfm_srclist_t),
                          ipmc_lib_srclist_cmp_func)) {
        IPMC_MEM_H_MGIVE(grp_db->ipmc_sf_do_forward_srclist, &frest);
        IPMC_MEM_INFO_MGIVE(inf_mem, &frest);
        IPMC_MEM_GRP_MGIVE(grp_mem, &frest);

        T_D("IPMC_LIB_DB_TAKE(ipmc_sf_do_forward_srclist) failed");
        return NULL;
    }

    if (!IPMC_MEM_H_MTAKE(grp_db->ipmc_sf_do_not_forward_srclist)) {
        if (!IPMC_LIB_DB_GIVE(grp_db->ipmc_sf_do_forward_srclist)) {
            T_D("IPMC_LIB_DB_GIVE() fail !!!");
        }

        IPMC_MEM_H_MGIVE(grp_db->ipmc_sf_do_forward_srclist, &frest);
        IPMC_MEM_INFO_MGIVE(inf_mem, &frest);
        IPMC_MEM_GRP_MGIVE(grp_mem, &frest);

        T_D("Allocating ipmc_sf_do_not_forward_srclist failed");
        return NULL;
    }
    if (!IPMC_LIB_DB_TAKE("GRP_SL_BLK", grp_db->ipmc_sf_do_not_forward_srclist,
                          IPMC_NO_OF_SUPPORTED_SRCLIST,
                          sizeof(ipmc_sfm_srclist_t),
                          ipmc_lib_srclist_cmp_func)) {
        if (!IPMC_LIB_DB_GIVE(grp_db->ipmc_sf_do_forward_srclist)) {
            T_D("IPMC_LIB_DB_GIVE() fail !!!");
        }

        IPMC_MEM_H_MGIVE(grp_db->ipmc_sf_do_not_forward_srclist, &frest);
        IPMC_MEM_H_MGIVE(grp_db->ipmc_sf_do_forward_srclist, &frest);
        IPMC_MEM_INFO_MGIVE(inf_mem, &frest);
        IPMC_MEM_GRP_MGIVE(grp_mem, &frest);

        T_D("IPMC_LIB_DB_TAKE(ipmc_sf_do_not_forward_srclist) failed");
        return NULL;
    }

    grp_mem->vid = grp->vid;
    grp_mem->ipmc_version = grp->ipmc_version;
    memcpy(&grp_mem->group_addr, &grp->group_addr, sizeof(vtss_ipv6_t));
    grp_mem->info = inf_mem;

    inf_mem->valid = TRUE;
    inf_mem->interface = intf;
    IPMC_TIMER_RESET(&inf_mem->min_tmr);

    local_port_cnt = ipmc_lib_get_system_local_port_cnt();
    for (i = 0; i < local_port_cnt; i++) {
        IPMC_TIMER_RESET(&inf_mem->rxmt_timer[i]);
        inf_mem->rxmt_count[i] = 0;
    }

    inf_mem->state = IPMC_OP_NO_LISTENER;
    grp_db->compatibility.mode = VTSS_IPMC_COMPAT_MODE_SFM;
    IPMC_TIMER_RESET(&grp_db->min_tmr);

    if (IPMC_LIB_DB_ADD(p, grp_mem)) {
        i8  addrString[40];

        T_I("ADD %s VLAN-%u Group-%s",
            ipmc_lib_version_txt(grp_mem->ipmc_version, IPMC_TXT_CASE_UPPER),
            grp->vid,
            misc_ipv6_txt(&grp_mem->group_addr, addrString));

        if (grp_mem != grp) {
            memcpy(grp, grp_mem, sizeof(ipmc_group_entry_t));
        }
        return grp_mem;
    } else {
        T_I("The MAX number of supporting group is %d, it is full now!!!",
            !IPMC_INTF_MVR(intf) ? IPMC_LIB_SUPPORTED_SNP_GROUPS : IPMC_LIB_SUPPORTED_MVR_GROUPS);

        if (!IPMC_LIB_DB_GIVE(grp_db->ipmc_sf_do_forward_srclist)) {
            T_D("IPMC_LIB_DB_GIVE() fail !!!");
        }
        if (!IPMC_LIB_DB_GIVE(grp_db->ipmc_sf_do_not_forward_srclist)) {
            T_D("IPMC_LIB_DB_GIVE() fail !!!");
        }

        IPMC_MEM_H_MGIVE(grp_db->ipmc_sf_do_not_forward_srclist, &frest);
        IPMC_MEM_H_MGIVE(grp_db->ipmc_sf_do_forward_srclist, &frest);
        IPMC_MEM_INFO_MGIVE(inf_mem, &frest);
        IPMC_MEM_GRP_MGIVE(grp_mem, &frest);

        return NULL;
    }
}

ipmc_group_entry_t *ipmc_lib_group_sync(ipmc_db_ctrl_hdr_t *p,
                                        ipmc_intf_entry_t *grp_intf,
                                        ipmc_group_entry_t *grp,
                                        BOOL asm_only,
                                        proc_grp_tmp_t type)
{
    ipmc_group_entry_t  *grp_op, *grp_ptr, grp_op_sync;

    if (!p || !grp) {
        return NULL;
    }

    if ((grp_ptr = ipmc_lib_group_ptr_get(p, grp)) == NULL) {
        if ((grp_ptr = ipmc_lib_group_init(grp_intf, p, grp)) == NULL) {
            T_D("ipmc_lib_group_init failed!");
            return NULL;
        } else {
            grp_ptr->info->grp = grp_ptr;
            grp_ptr->info->db.grp = grp_ptr;
        }
    }

    if (asm_only) {
        if (!ipmc_lib_group_update(p, grp_ptr)) {
            T_D("ipmc_lib_group_update failed!");
        }
    } else {
        /* Handle SFM Conditions */
        /* Retrieve the OLD entry first */
        grp_op = &grp_op_sync;
        switch ( type ) {
        case PROC4RCV:
            if (!ipmc_lib_get_grp_sfm_tmp4rcv(IPMC_INTF_IS_MVR_VAL(grp_ptr->info->interface), &grp_op)) {
                T_D("Failed in ipmc_lib_get_grp_sfm_tmp4rcv(grp_op is %s)", grp_op ? "OK" : "NULL");
                return NULL;
            }

            break;
        case PROC4TICK:
            if (!ipmc_lib_get_grp_sfm_tmp4tick(IPMC_INTF_IS_MVR_VAL(grp_ptr->info->interface), &grp_op)) {
                T_D("Failed in ipmc_lib_get_grp_sfm_tmp4tick(grp_op is %s)", grp_op ? "OK" : "NULL");
                return NULL;
            }

            break;
        case PROC4PROXY:
        case PROC4INIT:
        default:
            /* Basically, should not happen here! */
            T_D("Invalid SYNC-PROC Type:%d", type);
            return NULL;
        }

        if (ipmc_lib_forward_process_group_sfm(p, grp_op, grp_ptr) != VTSS_OK) {
            T_D("Type:%d->ipmc_lib_forward_process_group_sfm failed!", type);
        }
    }

    return grp_ptr;
}

static void _ipmc_lib_group_clear_rxmt(ipmc_group_entry_t *grp, ipmc_db_ctrl_hdr_t *rxmt)
{
    ipmc_group_info_t   rxmt_buf, *clear_ptr;

    if (!grp || !rxmt) {
        return;
    }

    memset(&rxmt_buf, 0x0, sizeof(ipmc_group_info_t));
    rxmt_buf.grp = grp;
    ipmc_lib_time_cpy(&rxmt_buf.min_tmr, &grp->info->min_tmr);
    clear_ptr = &rxmt_buf;
    if (IPMC_LIB_DB_GET(rxmt, clear_ptr)) {
        if (!clear_ptr->grp) {
            return;
        }

        if (clear_ptr->grp != grp) {
            return;
        }

        (void) (IPMC_LIB_DB_DEL(rxmt, clear_ptr));
    }
}

static void _ipmc_lib_group_clear_fltr(ipmc_group_entry_t *grp, ipmc_db_ctrl_hdr_t *fltr)
{
    ipmc_group_db_t fltr_buf, *clear_ptr;

    if (!grp || !fltr) {
        return;
    }

    memset(&fltr_buf, 0x0, sizeof(ipmc_group_db_t));
    fltr_buf.grp = grp;
    ipmc_lib_time_cpy(&fltr_buf.min_tmr, &grp->info->db.min_tmr);
    clear_ptr = &fltr_buf;
    if (IPMC_LIB_DB_GET(fltr, clear_ptr)) {
        if (!clear_ptr->grp) {
            return;
        }

        if (clear_ptr->grp != grp) {
            return;
        }

        (void) (IPMC_LIB_DB_DEL(fltr, clear_ptr));
    }
}

static void _ipmc_lib_group_clear_srct(ipmc_group_entry_t *grp, ipmc_db_ctrl_hdr_t *srct)
{
    ipmc_sfm_srclist_t  *clear_ptr;

    if (!grp || !srct) {
        return;
    }

    clear_ptr = NULL;
    while ((clear_ptr = ipmc_lib_srct_tmrlist_get_next(srct, clear_ptr)) != NULL) {
        if (!clear_ptr->grp) {
            continue;
        }

        if (clear_ptr->grp != grp) {
            continue;
        }

        (void) (IPMC_LIB_DB_DEL(srct, clear_ptr));
    }
}

BOOL ipmc_lib_group_delete(ipmc_intf_entry_t *ipmc_intf,
                           ipmc_db_ctrl_hdr_t *p,
                           ipmc_db_ctrl_hdr_t *rxmt,
                           ipmc_db_ctrl_hdr_t *fltr,
                           ipmc_db_ctrl_hdr_t *srct,
                           ipmc_group_entry_t *grp,
                           BOOL proxy, BOOL force)
{
    ipmc_group_db_t     *grp_db;
    i8                  addrString[40];
    u32                 i, local_port_cnt;
    ipmc_sfm_srclist_t  *sfm_src, *free_ptr;
    vtss_ipv4_t         ip4sip, ip4dip;
    vtss_ipv6_t         ip6sip, ip6dip;
    BOOL                retVal, frest;

    if (!p || !grp || !grp->info) {
        return FALSE;
    } else {
        grp_db = &grp->info->db;
    }

    retVal = TRUE;
    local_port_cnt = ipmc_lib_get_system_local_port_cnt();
    _ipmc_lib_group_clear_srct(grp, srct);
    /* SFM-DELETE */
    for (i = 0; i < local_port_cnt; i++) {
        if (!force && !IPMC_LIB_GRP_PORT_DO_SFM(grp_db, i)) {
            continue;
        }

        sfm_src = free_ptr = NULL;
        IPMC_SRCLIST_WALK(grp_db->ipmc_sf_do_forward_srclist, sfm_src) {
            if (free_ptr) {
                if (IPMC_LIB_DB_DEL(grp_db->ipmc_sf_do_forward_srclist, free_ptr)) {
                    IPMC_MEM_SL_MGIVE(free_ptr, &frest, 201);
                }
                free_ptr = NULL;
            }

            if (!VTSS_PORT_BF_GET(sfm_src->port_mask, i)) {
                continue;
            }

            if (IPMC_LIB_GRP_PORT_SFM_IN(grp_db, i)) {
                if (grp->ipmc_version == IPMC_IP_VERSION_MLD) {
                    memset((uchar *)&ip4sip, 0x0, sizeof(ipmcv4addr));
                    memset((uchar *)&ip4dip, 0x0, sizeof(ipmcv4addr));

                    memcpy(&ip6sip, &sfm_src->src_ip, sizeof(vtss_ipv6_t));
                    memcpy(&ip6dip, &grp->group_addr, sizeof(vtss_ipv6_t));
                } else {
                    memcpy((uchar *)&ip4sip, &sfm_src->src_ip.addr[12], sizeof(ipmcv4addr));
                    memcpy((uchar *)&ip4dip, &grp->group_addr.addr[12], sizeof(ipmcv4addr));

                    memset(&ip6sip, 0x0, sizeof(vtss_ipv6_t));
                    memset(&ip6dip, 0x0, sizeof(vtss_ipv6_t));
                }

                if (ipmc_lib_porting_set_chip(FALSE, p, grp, grp->ipmc_version, grp->vid, ip4sip, ip4dip, ip6sip, ip6dip, NULL) != VTSS_OK) {
                    retVal = FALSE;
                    T_D("ipmc_lib_porting_set_chip DEL failed");
                }
                /* SHOULD Notify MC-Routing - */
            }

            VTSS_PORT_BF_SET(sfm_src->port_mask, i, FALSE);
            if (IPMC_LIB_BFS_PROT_EMPTY(sfm_src->port_mask)) {
                free_ptr = sfm_src;
            }
        }
        if (free_ptr && IPMC_LIB_DB_DEL(grp_db->ipmc_sf_do_forward_srclist, free_ptr)) {
            IPMC_MEM_SL_MGIVE(free_ptr, &frest, 200);
        }

        sfm_src = free_ptr = NULL;
        IPMC_SRCLIST_WALK(grp_db->ipmc_sf_do_not_forward_srclist, sfm_src) {
            if (free_ptr) {
                if (IPMC_LIB_DB_DEL(grp_db->ipmc_sf_do_not_forward_srclist, free_ptr)) {
                    IPMC_MEM_SL_MGIVE(free_ptr, &frest, 199);
                }
                free_ptr = NULL;
            }

            if (!VTSS_PORT_BF_GET(sfm_src->port_mask, i)) {
                continue;
            }

            if (IPMC_LIB_GRP_PORT_SFM_EX(grp_db, i)) {
                if (grp->ipmc_version == IPMC_IP_VERSION_MLD) {
                    memset((uchar *)&ip4sip, 0x0, sizeof(ipmcv4addr));
                    memset((uchar *)&ip4dip, 0x0, sizeof(ipmcv4addr));

                    memcpy(&ip6sip, &sfm_src->src_ip, sizeof(vtss_ipv6_t));
                    memcpy(&ip6dip, &grp->group_addr, sizeof(vtss_ipv6_t));
                } else {
                    memcpy((uchar *)&ip4sip, &sfm_src->src_ip.addr[12], sizeof(ipmcv4addr));
                    memcpy((uchar *)&ip4dip, &grp->group_addr.addr[12], sizeof(ipmcv4addr));

                    memset(&ip6sip, 0x0, sizeof(vtss_ipv6_t));
                    memset(&ip6dip, 0x0, sizeof(vtss_ipv6_t));
                }

                if (ipmc_lib_porting_set_chip(FALSE, p, grp, grp->ipmc_version, grp->vid, ip4sip, ip4dip, ip6sip, ip6dip, NULL) != VTSS_OK) {
                    T_D("ipmc_lib_porting_set_chip DEL failed");
                    retVal = FALSE;
                }
                /* SHOULD Notify MC-Routing - */
            }

            VTSS_PORT_BF_SET(sfm_src->port_mask, i, FALSE);
            if (IPMC_LIB_BFS_PROT_EMPTY(sfm_src->port_mask)) {
                free_ptr = sfm_src;
            }
        }
        if (free_ptr && IPMC_LIB_DB_DEL(grp_db->ipmc_sf_do_not_forward_srclist, free_ptr)) {
            IPMC_MEM_SL_MGIVE(free_ptr, &frest, 198);
        }
    }

    /* ASM-DELETE */
    if (grp->ipmc_version == IPMC_IP_VERSION_MLD) {
        memset((uchar *)&ip4sip, 0x0, sizeof(ipmcv4addr));
        memset((uchar *)&ip4dip, 0x0, sizeof(ipmcv4addr));

        ipmc_lib_get_all_zero_ipv6_addr((vtss_ipv6_t *)&ip6sip);
        memcpy(&ip6dip, &grp->group_addr, sizeof(vtss_ipv6_t));
    } else {
        ipmc_lib_get_all_zero_ipv4_addr((ipmcv4addr *)&ip4sip);
        memcpy((uchar *)&ip4dip, &grp->group_addr.addr[12], sizeof(ipmcv4addr));

        memset(&ip6sip, 0x0, sizeof(vtss_ipv6_t));
        memset(&ip6dip, 0x0, sizeof(vtss_ipv6_t));
    }

    if (ipmc_lib_porting_set_chip(FALSE, p, grp, grp->ipmc_version, grp->vid, ip4sip, ip4dip, ip6sip, ip6dip, NULL) != VTSS_OK) {
        T_D("ipmc_lib_porting_set_chip DEL failed");
        retVal = FALSE;
    }
    /* SHOULD Notify MC-Routing - */

    if (proxy) {
        ipmc_port_bfs_t leave_fwd_mask;

        /* Exclude stacking ports for proxy LEAVE */
        VTSS_PORT_BF_CLR(leave_fwd_mask.member_ports);
        for (i = 0; i < local_port_cnt; i++) {
            VTSS_PORT_BF_SET(leave_fwd_mask.member_ports, i, ipmc_lib_get_port_rpstatus(grp->ipmc_version, i));
        }

        (void) ipmc_lib_packet_tx_group_leave(ipmc_intf, &grp->group_addr, &leave_fwd_mask, FALSE, FALSE);
    }

    if (!IPMC_LIB_DB_GIVE(grp_db->ipmc_sf_do_forward_srclist)) {
        T_D("IPMC_LIB_DB_GIVE() fail !!!");
        retVal = FALSE;
    }
    if (!IPMC_LIB_DB_GIVE(grp_db->ipmc_sf_do_not_forward_srclist)) {
        T_D("IPMC_LIB_DB_GIVE() fail !!!");
        retVal = FALSE;
    }

    _ipmc_lib_group_clear_fltr(grp, fltr);
    _ipmc_lib_group_clear_rxmt(grp, rxmt);

    IPMC_MEM_H_MGIVE(grp_db->ipmc_sf_do_forward_srclist, &frest);
    IPMC_MEM_H_MGIVE(grp_db->ipmc_sf_do_not_forward_srclist, &frest);
    IPMC_MEM_INFO_MGIVE(grp->info, &frest);

    if (!IPMC_LIB_DB_DEL(p, grp)) {
        T_D("IPMC_LIB_DB_DEL() fail !!!");
        retVal = FALSE;
    } else {
        T_I("DEL %s VLAN-%u Group-%s",
            ipmc_lib_version_txt(grp->ipmc_version, IPMC_TXT_CASE_UPPER),
            grp->vid,
            misc_ipv6_txt(&grp->group_addr, addrString));

        IPMC_MEM_GRP_MGIVE(grp, &frest);

        if (!IPMC_LIB_DB_GET_COUNT(p) && ipmc_intf) {
            ipmc_intf->param.hst_compatibility.old_present_timer = 0;
            ipmc_intf->param.hst_compatibility.gen_present_timer = 0;
            ipmc_intf->param.hst_compatibility.sfm_present_timer = 0;
            ipmc_intf->param.hst_compatibility.mode = ipmc_intf->param.cfg_compatibility;
        }
    }

    return retVal;
}

BOOL ipmc_lib_group_update(ipmc_db_ctrl_hdr_t *p,
                           ipmc_group_entry_t *grp)
{
    ipmc_group_db_t *grp_db;
    i8              addrString[40];
    u32             i, local_port_cnt;
    vtss_ipv4_t     ip4sip, ip4dip;
    vtss_ipv6_t     ip6sip, ip6dip;
    BOOL            fwd_map[VTSS_PORT_ARRAY_SIZE];

    if (!p || !grp || !grp->info) {
        return FALSE;
    } else {
        grp_db = &grp->info->db;
    }

    memset(fwd_map, 0x0, sizeof(fwd_map));
    local_port_cnt = ipmc_lib_get_system_local_port_cnt();
    for (i = 0; i < local_port_cnt; i++) {
        if (VTSS_PORT_BF_GET(grp_db->port_mask, i)) {
            fwd_map[i] = TRUE;
        }

        /* Handle SFM Conditions */
        if (grp->ipmc_version == IPMC_IP_VERSION_IGMP) {
#ifdef VTSS_FEATURE_IPV4_MC_SIP
            if (!IPMC_LIB_GRP_PORT_DO_SFM(grp_db, i)) {
                continue;
            }
#else
            continue;
#endif /* VTSS_FEATURE_IPV4_MC_SIP */
        } else if (grp->ipmc_version == IPMC_IP_VERSION_MLD) {
#ifdef VTSS_FEATURE_IPV6_MC_SIP
            if (!IPMC_LIB_GRP_PORT_DO_SFM(grp_db, i)) {
                continue;
            }
#else
            continue;
#endif /* VTSS_FEATURE_IPV6_MC_SIP */
        } else {
            continue;
        }

#if defined(VTSS_FEATURE_IPV4_MC_SIP) || defined(VTSS_FEATURE_IPV6_MC_SIP)
        if (IPMC_LIB_GRP_PORT_SFM_IN(grp_db, i)) {
            fwd_map[i] = FALSE;
        } else {
            fwd_map[i] = TRUE;
        }
#endif /* defined(VTSS_FEATURE_IPV4_MC_SIP) || defined(VTSS_FEATURE_IPV6_MC_SIP) */
    }

#if VTSS_SWITCH_STACKABLE
    i = PORT_NO_STACK_0;
    fwd_map[i] = TRUE;
    i = PORT_NO_STACK_1;
    fwd_map[i] = TRUE;
#endif /* VTSS_SWITCH_STACKABLE */

    /* ASM-UPDATE(ADD) */
    if (grp->ipmc_version == IPMC_IP_VERSION_MLD) {
        memset((uchar *)&ip4sip, 0x0, sizeof(ipmcv4addr));
        memset((uchar *)&ip4dip, 0x0, sizeof(ipmcv4addr));

        ipmc_lib_get_all_zero_ipv6_addr((vtss_ipv6_t *)&ip6sip);
        memcpy(&ip6dip, &grp->group_addr, sizeof(vtss_ipv6_t));
    } else {
        ipmc_lib_get_all_zero_ipv4_addr((ipmcv4addr *)&ip4sip);
        memcpy((uchar *)&ip4dip, &grp->group_addr.addr[12], sizeof(ipmcv4addr));

        memset(&ip6sip, 0x0, sizeof(vtss_ipv6_t));
        memset(&ip6dip, 0x0, sizeof(vtss_ipv6_t));
    }

    if (ipmc_lib_porting_set_chip(TRUE, p, grp, grp->ipmc_version, grp->vid, ip4sip, ip4dip, ip6sip, ip6dip, fwd_map) != VTSS_OK) {
        T_D("ipmc_lib_porting_set_chip ADD failed");
        return FALSE;
    }
    /* SHOULD Notify MC-Routing + */

    T_I("UPD %s VLAN-%u Group-%s",
        ipmc_lib_version_txt(grp->ipmc_version, IPMC_TXT_CASE_UPPER),
        grp->vid,
        misc_ipv6_txt(&grp->group_addr, addrString));

    return TRUE;
}

/* Always use EUI-64 as Link-Local Address */
BOOL ipmc_lib_get_eui64_linklocal_addr(vtss_ipv6_t *ipv6_addr)
{
    BOOL        found = FALSE;
    vtss_ipv6_t eui64;
    u8          mac_addr[6];

    if (!ipmc_lib_get_system_mgmt_macx(mac_addr)) {
        T_D("ipmc_lib_get_eui64_linklocal_addr->CANNOT FIND MAC-Address for LinkLocal Address");
        memset(ipv6_addr, 0x0, sizeof(vtss_ipv6_t));
    } else {
        found = TRUE;

        memset(&eui64, 0x0, sizeof(vtss_ipv6_t));
        memcpy(&eui64.addr[13], &mac_addr[3], sizeof(uchar) * 3);
        eui64.addr[12] = 0xFE;
        eui64.addr[11] = 0xFF;
        memcpy(&eui64.addr[8], &mac_addr[0], sizeof(uchar) * 3);

        eui64.addr[8] &= 0xFE;  /* G/I Bit */
        eui64.addr[8] |= 0x02;  /* U/L Bit */

        eui64.addr[1] = 0x80;
        eui64.addr[0] = 0xFE;

        memcpy(ipv6_addr, &eui64, sizeof(vtss_ipv6_t));
    }

    return found;
}

BOOL ipmc_lib_grp_src_list_del4port(ipmc_db_ctrl_hdr_t *srct, u32 intf, ipmc_db_ctrl_hdr_t *head)
{
    ipmc_sfm_srclist_t  *entry, *free_ptr;
    BOOL                frest;

    if (!head) {
        return FALSE;
    }

    entry = free_ptr = NULL;
    IPMC_SRCLIST_WALK(head, entry) {
        if (free_ptr) {
            if (IPMC_LIB_DB_DEL(head, free_ptr)) {
                IPMC_MEM_SL_MGIVE(free_ptr, &frest, 197);
            }
            free_ptr = NULL;
        }

        if (!VTSS_PORT_BF_GET(entry->port_mask, intf)) {
            continue;
        }

        VTSS_PORT_BF_SET(entry->port_mask, intf, FALSE);
        if (IPMC_LIB_BFS_PROT_EMPTY(entry->port_mask)) {
            IPMC_TIMER_UNLINK(srct, entry, &frest);
            free_ptr = entry;
        }
    }
    if (free_ptr && IPMC_LIB_DB_DEL(head, free_ptr)) {
        IPMC_MEM_SL_MGIVE(free_ptr, &frest, 196);
    }

    return TRUE;
}

void ipmc_lib_srclist_prepare(ipmc_intf_entry_t *intf,
                              ipmc_sfm_srclist_t *srclist,
                              void *group_record,
                              u16 idx,
                              u8 port)
{
    if (!intf || !srclist || !group_record) {
        return;
    }

    VTSS_PORT_BF_CLR(srclist->sf_calc);
    VTSS_PORT_BF_CLR(srclist->port_mask);
    srclist->sfm_in_hw = FALSE;
    srclist->grp = NULL;
    IPMC_TIMER_RESET(&srclist->min_tmr);

    if (intf->ipmc_version == IPMC_IP_VERSION_IGMP) {
        IPMC_LIB_ADRS_SET(&srclist->src_ip, 0x0);
        IPMC_LIB_ADRS_4TO6_SET(((igmp_group_record_t *)group_record)->source_addr[idx], srclist->src_ip);
    } else {
        IPMC_LIB_ADRS_CPY(&srclist->src_ip, &((mld_group_record_t *)group_record)->source_addr[idx]);
    }
    VTSS_PORT_BF_SET(srclist->port_mask, port, TRUE);
    memset(srclist->tmr.srct_timer.t, 0x0, sizeof(srclist->tmr.srct_timer.t));
    IPMC_TIMER_MALI_GET(intf, &srclist->tmr.srct_timer.t[port]);
}

BOOL ipmc_lib_srclist_logical_op_pkt(ipmc_group_entry_t *grp, ipmc_intf_entry_t *intf, u8 port, int action, ipmc_db_ctrl_hdr_t *list, void *group_record, u16 src_num, BOOL unlnk, ipmc_db_ctrl_hdr_t *srct)
{
    ipmc_sfm_srclist_t  *element, *free_ptr, *check, *check_bak;
    u16                 idx;
    BOOL                frest;
    u8                  freid, alcid;

    if (!intf || !list || !group_record ||
        !IPMC_MEM_SYSTEM_MTAKE(check, sizeof(ipmc_sfm_srclist_t))) {
        return FALSE;
    }
    check_bak = check;

    switch ( action ) {
    case VTSS_IPMC_SFM_OP_OR:
        alcid = 170;
        for (idx = 0; idx < src_num; idx++) {
            check = check_bak;
            ipmc_lib_srclist_prepare(intf, check, group_record, idx, port);

            if ((element = ipmc_lib_srclist_adr_get(list, check)) != NULL) {
                VTSS_PORT_BF_SET(element->port_mask, port, TRUE);
            } else {
                if (IPMC_LIB_SRCT_ADD_ECK(++alcid, list, grp, check)) {
                    T_W("ipmc_lib_srclist_add failed!!!");
                    IPMC_MEM_SYSTEM_MGIVE(check_bak);
                    return FALSE;
                }
            }
        }

        break;
    case VTSS_IPMC_SFM_OP_AND:
        for (idx = 0; idx < src_num; idx++) {
            check = check_bak;
            ipmc_lib_srclist_prepare(intf, check, group_record, idx, port);

            if ((element = ipmc_lib_srclist_adr_get(list, check)) != NULL) {
                element->sf_calc[port] = IPMC_OP_UPD;
            }
        }

        element = free_ptr = NULL;
        IPMC_SRCLIST_WALK(list, element) {
            if (free_ptr) {
                if (IPMC_LIB_DB_DEL(list, free_ptr)) {
                    IPMC_MEM_SL_MGIVE(free_ptr, &frest, 195);
                }
                free_ptr = NULL;
            }

            if (element->sf_calc[port]) {
                /* Read&Clear */
                element->sf_calc[port] = 0x0;
                continue;
            }

            VTSS_PORT_BF_SET(element->port_mask, port, FALSE);
            if (IPMC_LIB_BFS_PROT_EMPTY(element->port_mask)) {
                if (unlnk) {
                    IPMC_TIMER_UNLINK(srct, element, &frest);
                }

                free_ptr = element;
            }
        }
        if (free_ptr && IPMC_LIB_DB_DEL(list, free_ptr)) {
            IPMC_MEM_SL_MGIVE(free_ptr, &frest, 194);
        }

        break;
    case VTSS_IPMC_SFM_OP_DIFF:
        freid = 210;
        for (idx = 0; idx < src_num; idx++) {
            check = check_bak;
            ipmc_lib_srclist_prepare(intf, check, group_record, idx, port);

            if ((element = ipmc_lib_srclist_adr_get(list, check)) != NULL) {
                VTSS_PORT_BF_SET(element->port_mask, port, FALSE);
                if (IPMC_LIB_BFS_PROT_EMPTY(element->port_mask)) {
                    if (unlnk) {
                        IPMC_TIMER_UNLINK(srct, element, &frest);
                    }

                    if (!ipmc_lib_srclist_del(list, element, ++freid)) {
                        T_D("ipmc_lib_srclist_del failed!!!");
                    }
                }
            }
        }

        break;
    default:

        IPMC_MEM_SYSTEM_MGIVE(check_bak);
        return FALSE;
    }

    IPMC_MEM_SYSTEM_MGIVE(check_bak);
    return TRUE;
}

BOOL ipmc_lib_srclist_logical_op_set(ipmc_group_entry_t *grp, u8 port, int action, BOOL unlnk, ipmc_db_ctrl_hdr_t *srct, ipmc_db_ctrl_hdr_t *list, ipmc_db_ctrl_hdr_t *operand)
{
    ipmc_sfm_srclist_t  *element, *check, *free_ptr;
    BOOL                frest;
    u8                  alcid, freid;

    if (!list || !operand) {
        return FALSE;
    }

    switch ( action ) {
    case VTSS_IPMC_SFM_OP_OR:
        alcid = 160;
        check = NULL;
        IPMC_SRCLIST_WALK(operand, check) {
            if (VTSS_PORT_BF_GET(check->port_mask, port) == FALSE) {
                continue;
            }

            if ((element = ipmc_lib_srclist_adr_get(list, check)) != NULL) {
                VTSS_PORT_BF_SET(element->port_mask, port, TRUE);
            } else {
                IPMC_LIB_SRCT_ADD_ERT(++alcid, FALSE, list, grp, check);
            }
        }

        break;
    case VTSS_IPMC_SFM_OP_AND:
        element = free_ptr = NULL;
        IPMC_SRCLIST_WALK(list, element) {
            if (free_ptr) {
                if (IPMC_LIB_DB_DEL(list, free_ptr)) {
                    IPMC_MEM_SL_MGIVE(free_ptr, &frest, 193);
                }
                free_ptr = NULL;
            }

            if (VTSS_PORT_BF_GET(element->port_mask, port) == FALSE) {
                continue;
            }

            if (ipmc_lib_srclist_adr_get(operand, element) == NULL) {
                VTSS_PORT_BF_SET(element->port_mask, port, FALSE);
                if (IPMC_LIB_BFS_PROT_EMPTY(element->port_mask)) {
                    if (unlnk) {
                        IPMC_TIMER_UNLINK(srct, element, &frest);
                    }

                    free_ptr = element;
                }
            }
        }
        if (free_ptr && IPMC_LIB_DB_DEL(list, free_ptr)) {
            IPMC_MEM_SL_MGIVE(free_ptr, &frest, 192);
        }


        break;
    case VTSS_IPMC_SFM_OP_DIFF:
        freid = 220;
        check = NULL;
        IPMC_SRCLIST_WALK(operand, check) {
            if (VTSS_PORT_BF_GET(check->port_mask, port) == FALSE) {
                continue;
            }

            if ((element = ipmc_lib_srclist_adr_get(list, check)) != NULL) {
                VTSS_PORT_BF_SET(element->port_mask, port, FALSE);
                if (IPMC_LIB_BFS_PROT_EMPTY(element->port_mask)) {
                    if (unlnk) {
                        IPMC_TIMER_UNLINK(srct, element, &frest);
                    }

                    if (!ipmc_lib_srclist_del(list, element, ++freid)) {
                        T_D("ipmc_lib_srclist_del failed!!!");
                    }
                }
            }
        }

        break;
    default:

        return FALSE;
    }

    return TRUE;
}

void ipmc_lib_srclist_logical_op_cmp(BOOL *chg, u8 port, ipmc_db_ctrl_hdr_t *list, ipmc_db_ctrl_hdr_t *operand)
{
    ipmc_sfm_srclist_t  *element;
    ipmc_sfm_srclist_t  *check;

    if (!chg) {
        return;
    }

    if (!list || !operand) {
        *chg = FALSE;
        return;
    }

    if (IPMC_LIB_DB_GET_COUNT(list) != IPMC_LIB_DB_GET_COUNT(operand)) {
        *chg = TRUE;
        return;
    }

    *chg = FALSE;
    check = NULL;
    IPMC_SRCLIST_WALK(operand, check) {
        if ((element = ipmc_lib_srclist_adr_get(list, check)) != NULL) {
            if (VTSS_PORT_BF_GET(element->port_mask, port) != VTSS_PORT_BF_GET(check->port_mask, port)) {
                *chg = TRUE;
                return;
            }
        } else {
            *chg = TRUE;
            return;
        }
    }
}

BOOL ipmc_lib_sfm_logical_op(ipmc_group_entry_t *grp, u32 intf, ipmc_db_ctrl_hdr_t *res_list, ipmc_db_ctrl_hdr_t *a_list, ipmc_db_ctrl_hdr_t *b_list, int action, ipmc_db_ctrl_hdr_t *srct)
{
    ipmc_sfm_srclist_t  *element, *check, *free_ptr;
    u8                  idx, alcid, freid;
    BOOL                frest;

    switch ( action ) {
    case VTSS_IPMC_SFM_OP_OR:
        if (res_list != a_list) {
            if (!ipmc_lib_grp_src_list_del4port(srct, intf, res_list)) {
                return FALSE;
            }

            alcid = 150;
            check = NULL;
            IPMC_SRCLIST_WALK(a_list, check) {
                if (VTSS_PORT_BF_GET(check->port_mask, intf) == FALSE) {
                    continue;
                }

                if ((element = ipmc_lib_srclist_adr_get(res_list, check)) != NULL) {
                    for (idx = 0; idx < VTSS_PORT_BF_SIZE; idx++) {
                        element->port_mask[idx] |= check->port_mask[idx];
                    }
                } else {
                    IPMC_LIB_SRCT_ADD_ERT(++alcid, FALSE, res_list, grp, check);
                }
            }

            alcid = 140;
            check = NULL;
            IPMC_SRCLIST_WALK(b_list, check) {
                if (VTSS_PORT_BF_GET(check->port_mask, intf) == FALSE) {
                    continue;
                }

                if ((element = ipmc_lib_srclist_adr_get(res_list, check)) != NULL) {
                    for (idx = 0; idx < VTSS_PORT_BF_SIZE; idx++) {
                        element->port_mask[idx] |= check->port_mask[idx];
                    }
                } else {
                    IPMC_LIB_SRCT_ADD_ERT(++alcid, FALSE, res_list, grp, check);
                }
            }
        } else {
            alcid = 130;
            check = NULL;
            IPMC_SRCLIST_WALK(b_list, check) {
                if (VTSS_PORT_BF_GET(check->port_mask, intf) == FALSE) {
                    continue;
                }

                if ((element = ipmc_lib_srclist_adr_get(res_list, check)) != NULL) {
                    for (idx = 0; idx < VTSS_PORT_BF_SIZE; idx++) {
                        element->port_mask[idx] |= check->port_mask[idx];
                    }
                } else {
                    IPMC_LIB_SRCT_ADD_ERT(++alcid, FALSE, res_list, grp, check);
                }
            }
        }

        break;
    case VTSS_IPMC_SFM_OP_AND:
        if (res_list != a_list) {
            ipmc_sfm_srclist_t  *check_tmp, chk_buf;

            if (!ipmc_lib_grp_src_list_del4port(srct, intf, res_list)) {
                return FALSE;
            }

            alcid = 120;
            check = NULL;
            IPMC_SRCLIST_WALK(a_list, check) {
                if (VTSS_PORT_BF_GET(check->port_mask, intf) == FALSE) {
                    continue;
                }

                if ((check_tmp = ipmc_lib_srclist_adr_get(b_list, check)) == NULL) {
                    continue;
                }

                if ((element = ipmc_lib_srclist_adr_get(res_list, check)) != NULL) {
                    for (idx = 0; idx < VTSS_PORT_BF_SIZE; idx++) {
                        element->port_mask[idx] |= check->port_mask[idx];
                        element->port_mask[idx] |= check_tmp->port_mask[idx];
                    }
                } else {
                    memcpy(&chk_buf, check_tmp, sizeof(ipmc_sfm_srclist_t));
                    for (idx = 0; idx < VTSS_PORT_BF_SIZE; idx++) {
                        chk_buf.port_mask[idx] |= check->port_mask[idx];
                    }

                    IPMC_LIB_SRCT_ADD_ERT(++alcid, FALSE, res_list, grp, &chk_buf);
                }
            }
        } else {
            element = free_ptr = NULL;
            IPMC_SRCLIST_WALK(res_list, element) {
                if (free_ptr) {
                    if (IPMC_LIB_DB_DEL(res_list, free_ptr)) {
                        IPMC_MEM_SL_MGIVE(free_ptr, &frest, 191);
                    }
                    free_ptr = NULL;
                }

                if (VTSS_PORT_BF_GET(element->port_mask, intf) == FALSE) {
                    continue;
                }

                if (ipmc_lib_srclist_adr_get(b_list, element) == NULL) {
                    VTSS_PORT_BF_SET(element->port_mask, intf, FALSE);
                    if (IPMC_LIB_BFS_PROT_EMPTY(element->port_mask)) {
                        IPMC_TIMER_UNLINK(srct, element, &frest);
                        free_ptr = element;
                    }
                }
            }
            if (free_ptr && IPMC_LIB_DB_DEL(res_list, free_ptr)) {
                IPMC_MEM_SL_MGIVE(free_ptr, &frest, 190);
            }
        }

        break;
    case VTSS_IPMC_SFM_OP_DIFF:
        if (res_list != a_list) {
            if (!ipmc_lib_grp_src_list_del4port(srct, intf, res_list)) {
                return FALSE;
            }

            alcid = 110;
            check = NULL;
            IPMC_SRCLIST_WALK(a_list, check) {
                if (VTSS_PORT_BF_GET(check->port_mask, intf) == FALSE) {
                    continue;
                }

                if ((element = ipmc_lib_srclist_adr_get(res_list, check)) != NULL) {
                    for (idx = 0; idx < VTSS_PORT_BF_SIZE; idx++) {
                        element->port_mask[idx] |= check->port_mask[idx];
                    }
                } else {
                    IPMC_LIB_SRCT_ADD_ERT(++alcid, FALSE, res_list, grp, check);
                }
            }

            freid = 230;
            check = NULL;
            IPMC_SRCLIST_WALK(b_list, check) {
                if (VTSS_PORT_BF_GET(check->port_mask, intf) == FALSE) {
                    continue;
                }

                if ((element = ipmc_lib_srclist_adr_get(res_list, check)) != NULL) {
                    for (idx = 0; idx < VTSS_PORT_BF_SIZE; idx++) {
                        element->port_mask[idx] |= check->port_mask[idx];
                    }

                    VTSS_PORT_BF_SET(element->port_mask, intf, FALSE);
                    if (IPMC_LIB_BFS_PROT_EMPTY(element->port_mask)) {
                        IPMC_TIMER_UNLINK(srct, element, &frest);
                        if (!ipmc_lib_srclist_del(res_list, element, ++freid)) {
                            T_D("ipmc_lib_srclist_del failed!!!");
                        }
                    }
                }
            }
        } else {
            freid = 240;
            check = NULL;
            IPMC_SRCLIST_WALK(b_list, check) {
                if (VTSS_PORT_BF_GET(check->port_mask, intf) == FALSE) {
                    continue;
                }

                if ((element = ipmc_lib_srclist_adr_get(res_list, check)) != NULL) {
                    for (idx = 0; idx < VTSS_PORT_BF_SIZE; idx++) {
                        element->port_mask[idx] |= check->port_mask[idx];
                    }

                    VTSS_PORT_BF_SET(element->port_mask, intf, FALSE);
                    if (IPMC_LIB_BFS_PROT_EMPTY(element->port_mask)) {
                        IPMC_TIMER_UNLINK(srct, element, &frest);
                        if (!ipmc_lib_srclist_del(res_list, element, ++freid)) {
                            T_D("ipmc_lib_srclist_del failed!!!");
                        }
                    }
                }
            }
        }

        break;
    default:

        return FALSE;
    }

    return TRUE;
}

BOOL ipmc_lib_srclist_struct_copy(ipmc_group_entry_t *grp, ipmc_db_ctrl_hdr_t *dest_list, ipmc_db_ctrl_hdr_t *src_list, u32 port)
{
    ipmc_sfm_srclist_t  *element;
    u8                  alcid;

    if (!ipmc_lib_srclist_clear(dest_list, 0)) {
        T_D("ipmc_lib_srclist_clear failed");
        return FALSE;
    }

    if (IPMC_LIB_DB_GET_COUNT(src_list) == 0) {
        return TRUE;
    }

    alcid = 200;
    element = NULL;
    IPMC_SRCLIST_WALK(src_list, element) {
        if ((port != VTSS_IPMC_SFM_OP_PORT_ANY) && !VTSS_PORT_BF_GET(element->port_mask, port)) {
            continue;
        }

        IPMC_LIB_SRCT_ADD_ERT(++alcid, FALSE, dest_list, grp, element);
    }

    return TRUE;
}

void ipmc_lib_get_all_zero_ipv6_addr(vtss_ipv6_t *addr)
{
    if (addr) {
        memcpy(addr, &IPMC_IP6_ZERO_ADDR, sizeof(vtss_ipv6_t));
    }
}

void ipmc_lib_get_all_ones_ipv6_addr(vtss_ipv6_t *addr)
{
    if (addr) {
        memcpy(addr, &IPMC_IP6_ALL1_ADDR, sizeof(vtss_ipv6_t));
    }
}

void ipmc_lib_get_all_node_ipv6_addr(vtss_ipv6_t *addr)
{
    if (addr) {
        memcpy(addr, &IPMC_IP6_ALL_NODE_ADDR, sizeof(vtss_ipv6_t));
    }
}

void ipmc_lib_get_all_router_ipv6_addr(vtss_ipv6_t *addr)
{
    if (addr) {
        memcpy(addr, &IPMC_IP6_ALL_RTR_ADDR, sizeof(vtss_ipv6_t));
    }
}

void ipmc_lib_get_all_zero_ipv4_addr(ipmcv4addr *addr)
{
    if (addr) {
        memcpy(addr, &IPMC_IP4_ZERO_ADDR, sizeof(ipmcv4addr));
    }
}

void ipmc_lib_get_all_ones_ipv4_addr(ipmcv4addr *addr)
{
    if (addr) {
        memcpy(addr, &IPMC_IP4_ALL1_ADDR, sizeof(ipmcv4addr));
    }
}

void ipmc_lib_get_all_node_ipv4_addr(ipmcv4addr *addr)
{
    if (addr) {
        memcpy(addr, &IPMC_IP4_ALL_NODE_ADDR, sizeof(ipmcv4addr));
    }
}

void ipmc_lib_get_all_router_ipv4_addr(ipmcv4addr *addr)
{
    if (addr) {
        memcpy(addr, &IPMC_IP4_ALL_RTR_ADDR, sizeof(ipmcv4addr));
    }
}

BOOL ipmc_lib_isaddr6_all_zero(vtss_ipv6_t *addr)
{
    if (addr && !memcmp(addr, &IPMC_IP6_ZERO_ADDR, sizeof(vtss_ipv6_t))) {
        return TRUE;
    }

    return FALSE;
}

BOOL ipmc_lib_isaddr6_all_ones(vtss_ipv6_t *addr)
{
    if (addr && !memcmp(addr, &IPMC_IP6_ALL1_ADDR, sizeof(vtss_ipv6_t))) {
        return TRUE;
    }

    return FALSE;
}

BOOL ipmc_lib_isaddr6_all_node(vtss_ipv6_t *addr)
{
    if (addr && !memcmp(addr, &IPMC_IP6_ALL_NODE_ADDR, sizeof(vtss_ipv6_t))) {
        return TRUE;
    }

    return FALSE;
}

BOOL ipmc_lib_isaddr6_all_router(vtss_ipv6_t *addr)
{
    if (addr && !memcmp(addr, &IPMC_IP6_ALL_RTR_ADDR, sizeof(vtss_ipv6_t))) {
        return TRUE;
    }

    return FALSE;
}

BOOL ipmc_lib_isaddr4_all_zero(ipmcv4addr *addr)
{
    if (addr && !memcmp(addr, &IPMC_IP4_ZERO_ADDR, sizeof(ipmcv4addr))) {
        return TRUE;
    }

    return FALSE;
}

BOOL ipmc_lib_isaddr4_all_ones(ipmcv4addr *addr)
{
    if (addr && !memcmp(addr, &IPMC_IP4_ALL1_ADDR, sizeof(ipmcv4addr))) {
        return TRUE;
    }

    return FALSE;
}

BOOL ipmc_lib_isaddr4_all_node(ipmcv4addr *addr)
{
    if (addr && !memcmp(addr, &IPMC_IP4_ALL_NODE_ADDR, sizeof(ipmcv4addr))) {
        return TRUE;
    }

    return FALSE;
}

BOOL ipmc_lib_isaddr4_all_router(ipmcv4addr *addr)
{
    if (addr && !memcmp(addr, &IPMC_IP4_ALL_RTR_ADDR, sizeof(ipmcv4addr))) {
        return TRUE;
    }

    return FALSE;
}

void ipmc_lib_set_ssm_range(ipmc_ip_version_t version, ipmc_prefix_t *prefix)
{
    if (!ipmc_lib_cmn_done_init) {
        return;
    }

    if (prefix && (version < IPMC_IP_VERSION_MAX)) {
        memcpy(&ipmc_prefix_ssm_range[version], prefix, sizeof(ipmc_prefix_t));
    }
}

BOOL ipmc_lib_get_ssm_range(ipmc_ip_version_t version, ipmc_prefix_t *prefix)
{
    if (!prefix) {
        return FALSE;
    }

    if (version < IPMC_IP_VERSION_MAX) {
        memcpy(prefix, &ipmc_prefix_ssm_range[version], sizeof(ipmc_prefix_t));
        return TRUE;
    }

    return FALSE;
}

/* Create tree in a given structure */
BOOL ipmc_lib_db_create_tree(ipmc_db_ctrl_hdr_t         *list,
                             char                       *name,
                             u32                        max_entry_cnt,
                             size_t                     size_of_entry,
                             vtss_avl_tree_cmp_func_t   compare_func)
{
    if (!list || !name) {
        return FALSE;
    }

    if (!IPMC_MEM_AVLTND_MTAKE(list->node, max_entry_cnt)) {
        T_D("Failed in ipmc_lib_mem_alloc for %s", name);
        return FALSE;
    }

    list->size = size_of_entry;
    VTSS_AVL_TREE_INIT(&list->ctrl,
                       name,
                       VTSS_MODULE_ID_IPMC_LIB,
                       compare_func,
                       max_entry_cnt,
                       list->node);
    if (!vtss_avl_tree_init(&list->ctrl)) {
        IPMC_MEM_AVLTND_MGIVE(list->node);
        T_D("Failed in vtss_avl_tree_init(%s)", name);
        return FALSE;
    }

    list->cnt = 0;
    return TRUE;
}

/* Destroy tree in a given structure */
BOOL ipmc_lib_db_destroy_tree(ipmc_db_ctrl_hdr_t *list)
{
    void *ptr;

    if (!list) {
        return FALSE;
    }

    ptr = NULL;
    if (vtss_avl_tree_get(&list->ctrl, &ptr, VTSS_AVL_TREE_GET_FIRST)) {
        void *fre = NULL;

        do {
            if (fre) {
                if (!vtss_avl_tree_delete(&list->ctrl, &fre)) {
                    T_D("Failed in vtss_avl_tree_delete(CNT:%u)", list->cnt);
                } else {
                    list->cnt--;
                }
            }

            fre = ptr;
        } while (vtss_avl_tree_get(&list->ctrl, &ptr, VTSS_AVL_TREE_GET_NEXT));

        if (fre) {
            if (!vtss_avl_tree_delete(&list->ctrl, &fre)) {
                T_D("Failed in vtss_avl_tree_delete(CNT:%u)", list->cnt);
            } else {
                list->cnt--;
            }
        }
    }

    if (list->cnt) {
        T_D("CNT:%u is not zero!", list->cnt);
    }

    vtss_avl_tree_destroy(&list->ctrl);
    IPMC_MEM_AVLTND_MGIVE(list->node);
    list->size = 0;
    list->cnt = 0;
    return TRUE;
}

/* Add or update (if existed) an entry in a given structure */
BOOL ipmc_lib_db_set_entry(ipmc_db_ctrl_hdr_t *list, void *entry)
{
    void *ptr;

    if (!list || !entry) {
        return FALSE;
    }

    ptr = entry;
    if (vtss_avl_tree_get(&list->ctrl, &ptr, VTSS_AVL_TREE_GET)) {
        if (ptr != entry) {
            memcpy(ptr, entry, list->size);
        }
    } else {
        if ((u32)list->cnt < list->ctrl.max_node_cnt) {
            if (!vtss_avl_tree_add(&list->ctrl, ptr)) {
                T_D("Failed in vtss_avl_tree_add");
                return FALSE;
            }

            list->cnt++;
        } else {
            /* Table Full */
            T_W("AVLT %s(Max:%u) is full!", list->ctrl.name, list->ctrl.max_node_cnt);
            return FALSE;
        }
    }

    return TRUE;
}

/* Delete a designated entry from a given structure */
BOOL ipmc_lib_db_delete_entry(ipmc_db_ctrl_hdr_t *list, void *entry)
{
    void *ptr;

    if (!list || !entry) {
        return FALSE;
    }

    ptr = entry;
    if (!vtss_avl_tree_delete(&list->ctrl, &ptr)) {
        T_D("Failed in vtss_avl_tree_delete(CNT:%u)", list->cnt);
        return FALSE;
    }

    list->cnt--;
    return TRUE;
}

/* Delete all entries in a given structure */
BOOL ipmc_lib_db_delete_all_entry(ipmc_db_ctrl_hdr_t *list)
{
    void *ptr;

    if (!list) {
        return FALSE;
    }

    ptr = NULL;
    if (vtss_avl_tree_get(&list->ctrl, &ptr, VTSS_AVL_TREE_GET_FIRST)) {
        void *fre = NULL;

        do {
            if (fre) {
                if (!vtss_avl_tree_delete(&list->ctrl, &fre)) {
                    T_D("Failed in vtss_avl_tree_delete(CNT:%u)", list->cnt);
                } else {
                    list->cnt--;
                }
            }

            fre = ptr;
        } while (vtss_avl_tree_get(&list->ctrl, &ptr, VTSS_AVL_TREE_GET_NEXT));

        if (fre) {
            if (!vtss_avl_tree_delete(&list->ctrl, &fre)) {
                T_D("Failed in vtss_avl_tree_delete(CNT:%u)", list->cnt);
            } else {
                list->cnt--;
            }
        }
    }

    if (list->cnt) {
        T_D("CNT:%u is not zero!", list->cnt);
    }

    return TRUE;
}


/* Get the max_cnt in a given structure */
u32 ipmc_lib_db_get_max_cnt(ipmc_db_ctrl_hdr_t *list)
{
    u32 ret = 0;

    if (list) {
        ret = list->ctrl.max_node_cnt;
    }

    return ret;
}

/* Get the current_cnt in a given structure */
u32 ipmc_lib_db_get_current_cnt(ipmc_db_ctrl_hdr_t *list)
{
    u32 ret = 0;

    if (list) {
        ret = list->cnt;
    }

    return ret;
}

/* Get the size_of_entry in a given structure */
u32 ipmc_lib_db_get_size_of_entry(ipmc_db_ctrl_hdr_t *list)
{
    u32 ret = 0;

    if (list) {
        ret = (u32)list->size;
    }

    return ret;
}

/* Get the first entry in a given structure */
BOOL ipmc_lib_db_get_first_entry(ipmc_db_ctrl_hdr_t *header, void *entry)
{
    void *ptr;

    if (!header || !entry) {
        return FALSE;
    }

    ptr = (void *) * ((int *)entry);
    if (!vtss_avl_tree_get(&header->ctrl, &ptr, VTSS_AVL_TREE_GET_FIRST)) {
        return FALSE;
    }

    if (ptr) {
        void    **entry_dp = entry;

        *entry_dp = ptr;
        return TRUE;
    } else {
        return FALSE;
    }
}

/* Get the designated entry in a given structure, the specific keys of current entry should be input */
BOOL ipmc_lib_db_get_entry(ipmc_db_ctrl_hdr_t *header, void *entry)
{
    void *ptr;

    if (!header || !entry) {
        return FALSE;
    }

    ptr = (void *) * ((int *)entry);
    if (!vtss_avl_tree_get(&header->ctrl, &ptr, VTSS_AVL_TREE_GET)) {
        return FALSE;
    }

    if (ptr) {
        void    **entry_dp = entry;

        *entry_dp = ptr;
        return TRUE;
    } else {
        return FALSE;
    }
}

/* Get the next entry in a given structure, the specific keys of current entry should be input */
BOOL ipmc_lib_db_get_next_entry(ipmc_db_ctrl_hdr_t *header, void *entry)
{
    void *ptr;

    if (!header || !entry) {
        return FALSE;
    }

    ptr = NULL;
    if (!*((int *)entry)) {
        if (!vtss_avl_tree_get(&header->ctrl, &ptr, VTSS_AVL_TREE_GET_FIRST)) {
            return FALSE;
        }
    } else {
        ptr = (void *) * ((int *)entry);
        if (!vtss_avl_tree_get(&header->ctrl, &ptr, VTSS_AVL_TREE_GET_NEXT)) {
            return FALSE;
        }
    }

    if (ptr) {
        void    **entry_dp = entry;

        *entry_dp = ptr;
        return TRUE;
    } else {
        return FALSE;
    }
}

/*lint -e{454} */
void ipmc_lib_lock(void)
{
#if VTSS_TRACE_ENABLED
    critd_enter(&ipmc_lib_cmn_crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__);
#else
    critd_enter(&ipmc_lib_cmn_crit);
#endif /* VTSS_TRACE_ENABLED */
}

/*lint -e{455} */
void ipmc_lib_unlock(void)
{
#if VTSS_TRACE_ENABLED
    critd_exit(&ipmc_lib_cmn_crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__);
#else
    critd_exit(&ipmc_lib_cmn_crit);
#endif /* VTSS_TRACE_ENABLED */
}

static BOOL ipmc_lib_static_mem_init = FALSE;
static void _ipmc_lib_cmn_static_mem_init(void)
{
    u32                     i;
    ipmc_lib_mem_obj_t      *mem_obj;
    ipmc_db_ctrl_hdr_t      *h;
    ipmc_sfm_srclist_t      *sl;
    ipmc_group_entry_t      *grp;
    ipmc_group_info_t       *info;
    ipmc_profile_rule_t     *rules;
    ipmc_lib_profile_mem_t  *profile;

    mem_obj = &ipmc_mem_pool.obj;
    memset(mem_obj, 0x0, sizeof(ipmc_lib_mem_obj_t));
    mem_obj->h_max = IPMC_LIB_DYN_ALLOC_CTRL_HDR_CNT;
    mem_obj->sl_max = IPMC_LIB_TOTAL_SRC_LIST;
    mem_obj->grp_max = IPMC_LIB_SUPPORTED_GROUPS;
    mem_obj->info_max = IPMC_LIB_SUPPORTED_GROUPS;
    mem_obj->rules_max = IPMC_LIB_SUPPORTED_RULES;
    mem_obj->profile_max = IPMC_LIB_SUPPORTED_PROFILES;

    for (i = 0; i < IPMC_LIB_DYN_ALLOC_CTRL_HDR_CNT; i++) {
        h = &ipmc_mem_pool.h_table[i];
        h->mflag = FALSE;
        h->next = mem_obj->h_free;
        mem_obj->h_free = h;
    }
    for (i = 0; i < IPMC_LIB_TOTAL_SRC_LIST; i++) {
        sl = &ipmc_mem_pool.sl_table[i];
        sl->mflag = FALSE;
        sl->next = mem_obj->sl_free;
        mem_obj->sl_free = sl;
    }
    for (i = 0; i < IPMC_LIB_SUPPORTED_GROUPS; i++) {
        grp = &ipmc_mem_pool.grp_table[i];
        grp->mflag = FALSE;
        grp->next = mem_obj->grp_free;
        mem_obj->grp_free = grp;
    }
    for (i = 0; i < IPMC_LIB_SUPPORTED_GROUPS; i++) {
        info = &ipmc_mem_pool.info_table[i];
        info->mflag = FALSE;
        info->next = mem_obj->info_free;
        mem_obj->info_free = info;
    }
    for (i = 0; i < IPMC_LIB_SUPPORTED_RULES; i++) {
        rules = &ipmc_mem_pool.rules_table[i];
        rules->mflag = FALSE;
        rules->next = mem_obj->rules_free;
        mem_obj->rules_free = rules;
    }
    for (i = 0; i < IPMC_LIB_SUPPORTED_PROFILES; i++) {
        profile = &ipmc_mem_pool.profile_table[i];
        profile->mflag = FALSE;
        profile->next = mem_obj->profile_free;
        mem_obj->profile_free = profile;
    }
}

vtss_rc ipmc_lib_common_init(void)
{
    u32                 i;
    port_iter_t         pit;
    size_t              size, blocksize;
    ipmc_mem_alloc_t    mem_type;
    ipmc_mgmt_ipif_t    *ifp;

    if (ipmc_lib_cmn_done_init) {
        return VTSS_OK;
    }

    if (ipmc_lib_porting_init() != VTSS_OK) {
        return VTSS_RC_ERROR;
    }

    for (i = 0; i < IPMC_MEM_TYPE_MAX; i++) {
        size = blocksize = 0x0;
        memset(&ipmc_lib_memory_status[i], 0x0, sizeof(ipmc_mem_status_t));
        switch ( i ) {
        case IPMC_MEM_OS_MALLOC:
            ipmc_lib_memory_status[i].valid = TRUE;
            break;
        case IPMC_MEM_JUMBO:
            ipmc_lib_memory_status[i].fixed = TRUE;
            size = IPMC_LIB_MEM_SIZE_JUMBO;
            blocksize = IPMC_LIB_PKT_BUF_SZ;
            break;
        case IPMC_MEM_AVLT_NODE:
            size = IPMC_LIB_MEM_SIZE_AVLTND;
            blocksize = sizeof(vtss_avl_tree_node_t);
            break;
        case IPMC_MEM_SYS:
            size = IPMC_LIB_MEM_SIZE_SYS;
            break;
        case IPMC_MEM_GROUP:
        case IPMC_MEM_GRP_INFO:
        case IPMC_MEM_CTRL_HDR:
        case IPMC_MEM_SRC_LIST:
        case IPMC_MEM_RULES:
        case IPMC_MEM_PROFILE:
            if (!ipmc_lib_static_mem_init) {
                ipmc_lib_static_mem_init = TRUE;
                _ipmc_lib_cmn_static_mem_init();
            }
            ipmc_lib_memory_status[i].valid = TRUE;
            break;
        }

        if (!size) {
            continue;
        }

        ipmc_lib_memory_status[i].valid = TRUE;
        ipmc_lib_memory_status[i].sz_pool = size;
        ipmc_lib_memory_status[i].sz_partition = blocksize;

        if (ipmc_lib_memory_status[i].fixed) {
            mem_type = IPMC_MEM_PARTITIONED;
        } else {
            mem_type = IPMC_MEM_DYNA_POOL;
        }

        T_I("Start to initialize %s %s-(SIZ/BLK:%u/%u)",
            ipmc_lib_mem_id_txt(i),
            ipmc_lib_memory_status[i].fixed ? "FIX" : "DYN",
            ipmc_lib_memory_status[i].sz_pool,
            ipmc_lib_memory_status[i].sz_partition);
        if (!ipmc_lib_memory_initialize(mem_type,
                                        &ipmc_lib_memory_status[i].idx,
                                        ipmc_lib_memory_status[i].sz_pool,
                                        ipmc_lib_memory_status[i].sz_partition)) {
            T_E("Failure in initializing %s memory", ipmc_lib_mem_id_txt(i));
        }
    }

    critd_init(&ipmc_lib_cmn_crit, "ipmc_lib_cmn_crit", VTSS_MODULE_ID_IPMC_LIB, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
    IPMC_LIB_CRIT_EXIT();

    memset(&ipmc_ip_mgmt_system, 0x0, sizeof(ipmc_lib_mgmt_info_t));
    IPMC_MGMT_IFLIST_CNT_MAX = VTSS_IPMC_MGMT_IPIF_MAX_CNT;
    for (i = 0; i < VTSS_IPMC_MGMT_IPIF_MAX_CNT; i++) {
        ifp = IPMC_MGMT_SYSTEM_IPIF(&ipmc_ip_mgmt_system, i);
        ifp->next = IPMC_MGMT_IFLIST_FREE;
        IPMC_MGMT_IFLIST_FREE = ifp;
    }

    memset(&IPMC_IP4_ZERO_ADDR, 0x0, sizeof(ipmcv4addr));
    memset(&IPMC_IP4_ALL1_ADDR, 0xFF, sizeof(ipmcv4addr));
    memset(&IPMC_IP4_ALL_NODE_ADDR, 0x0, sizeof(ipmcv4addr));
    memset(&IPMC_IP4_ALL_RTR_ADDR, 0x0, sizeof(ipmcv4addr));
    IPMC_IP4_ALL_NODE_ADDR.addr[0] = IPMC_IP4_ALL_RTR_ADDR.addr[0] = 0xE0;
    IPMC_IP4_ALL_NODE_ADDR.addr[3] = 0x01;
    IPMC_IP4_ALL_RTR_ADDR.addr[3] = 0x02;

    memset(&IPMC_IP6_ZERO_ADDR, 0x0, sizeof(vtss_ipv6_t));
    memset(&IPMC_IP6_ALL1_ADDR, 0xFF, sizeof(vtss_ipv6_t));
    memset(&IPMC_IP6_ALL_NODE_ADDR, 0x0, sizeof(vtss_ipv6_t));
    memset(&IPMC_IP6_ALL_RTR_ADDR, 0x0, sizeof(vtss_ipv6_t));
    IPMC_IP6_ALL_NODE_ADDR.addr[0] = IPMC_IP6_ALL_RTR_ADDR.addr[0] = 0xFF;
    IPMC_IP6_ALL_NODE_ADDR.addr[1] = IPMC_IP6_ALL_RTR_ADDR.addr[1] = 0x02;
    IPMC_IP6_ALL_NODE_ADDR.addr[15] = 0x01;
    IPMC_IP6_ALL_RTR_ADDR.addr[15] = 0x02;

    /* initialize Router Ports, IPMC SSM Range */
    for (i = 0; i < IPMC_IP_VERSION_MAX; i++) {
        memset(&ipmc_prefix_ssm_range[i], 0x0, sizeof(ipmc_prefix_t));
        VTSS_PORT_BF_CLR(router_port_mask[i]);
        VTSS_PORT_BF_CLR(diff_router_port_mask[i]);
    }

    (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
    while (port_iter_getnext(&pit)) {
        ipmc_local_port_cnt++;
    }

    ipmc_lib_cmn_done_init = TRUE;
    return VTSS_OK;
}
