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
#include "critd_api.h"
#include "conf_api.h"
#include "mgmt_api.h"

#include "ipmc_lib.h"
#include "ipmc_lib_porting.h"

#include "vtss_bip_buffer_api.h"


/* ************************************************************************ **
 *
 * Defines
 *
 * ************************************************************************ */
#define VTSS_TRACE_MODULE_ID    VTSS_MODULE_ID_IPMC_LIB

#define IPMC_LIB_PKT_BIP_ZC     1

#define IPMC_LIB_PKT_MIN_XMT_SZ 60
#define IPMC_LIB_CHK_PKT_REG4   0x49    /* 01001001 */
#define IPMC_LIB_CHK_PKT_REG6   0x92    /* 10010010 */
#define IPMC_LIB_CHK_PKT_R_SNP  0x18    /* 00011000 */
#define IPMC_LIB_CHK_PKT_R_MVR  0xC0    /* 11000000 */
#define IPMC_LIB_CHK_PKT_MSK4R  0x48    /* 01001000 */
#define IPMC_LIB_CHK_PKT_MSK6R  0x90    /* 10010000 */

#define IPMC_LIB_PKT_SUPPRESS   0

/* ************************************************************************ **
 *
 * Public data
 *
 * ************************************************************************ */
static vtss_ipmc_rx_callback_t  vtss_ipmc_lib_snp_cb = NULL;
static vtss_ipmc_rx_callback_t  vtss_ipmc_lib_mvr_cb = NULL;

/* ************************************************************************ **
 *
 * Local data
 *
 * ************************************************************************ */
static u8                       *snp_uip_buf, *mvr_uip_buf;
static u8                       ipmc_lib_packet_regs;
static BOOL                     ipmc_lib_pkt_done_init = FALSE;

/* IPMC_LIB_RX Thread */
static cyg_handle_t             ipmc_lib_pkt_thread_handle;
static cyg_thread               ipmc_lib_pkt_thread_block;
static char                     ipmc_lib_pkt_thread_stack[IPMC_THREAD_STACK_SIZE];
static critd_t                  ipmc_pkt_rx_crit, ipmc_pkt_set_crit;
static cyg_flag_t               IPMC_LIB_RX_flag;
static vtss_bip_buffer_t        IPMC_LIB_RX_bip;

static mac_addr_t               mld_all_node_mac = {0x33, 0x33, 0x0, 0x0, 0x0, 0x01};
static mac_addr_t               mld_all_rtr_mac = {0x33, 0x33, 0x0, 0x0, 0x0, 0x02};
static mac_addr_t               mld_sfm_rpt_mac = {0x33, 0x33, 0x0, 0x0, 0x0, 0x16};
static mac_addr_t               igmp_all_node_mac = {0x01, 0x0, 0x5E, 0x0, 0x0, 0x01};
static mac_addr_t               igmp_all_rtr_mac = {0x01, 0x0, 0x5E, 0x0, 0x0, 0x02};
static mac_addr_t               igmp_sfm_rpt_mac = {0x01, 0x0, 0x5E, 0x0, 0x0, 0x16};

#if VTSS_TRACE_ENABLED
#define IPMC_PKT_SET_ENTER()    critd_enter(&ipmc_pkt_set_crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define IPMC_PKT_SET_EXIT()     critd_exit(&ipmc_pkt_set_crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define IPMC_PKT_RCV_ENTER()    critd_enter(&ipmc_pkt_rx_crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define IPMC_PKT_RCV_EXIT()     critd_exit(&ipmc_pkt_rx_crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#else
#define IPMC_PKT_SET_ENTER()    critd_enter(&ipmc_pkt_set_crit)
#define IPMC_PKT_SET_EXIT()     critd_exit(&ipmc_pkt_set_crit)
#define IPMC_PKT_RCV_ENTER()    critd_enter(&ipmc_pkt_rx_crit)
#define IPMC_PKT_RCV_EXIT()     critd_exit(&ipmc_pkt_rx_crit)
#endif /* VTSS_TRACE_ENABLED */

#define IPMC_PKT_EVENT_ANY      0xFFFFFFFF  /* Any possible bit... */
#define IPMC_PKT_EVENT_KICK     0x00000001

#if IPMC_LIB_PKT_SUPPRESS
#define IPMC_PKT_DO_QRY_SUPP(x) do {(x)->sfminfo.query.resv_s_qrv |= 0x8;} while (0)
#else
#define IPMC_PKT_DO_QRY_SUPP(x)
#endif /* IPMC_LIB_PKT_SUPPRESS */

static ipmc_time_t              dispatch_max_time;
/*lint -esym(457, dispatch_cnt) */
/*lint -esym(457, transmit_cnt) */
/*lint -esym(457, rx_mvr_cnt) */
/*lint -esym(457, rx_snp_cnt) */
static u32                      dispatch_cnt;
static u32                      transmit_cnt;
static u32                      rx_mvr_cnt;
static u32                      rx_snp_cnt;


void ipmc_lib_packet_max_time_reset(void)
{
    IPMC_PKT_SET_ENTER();
    memset(&dispatch_max_time, 0x0, sizeof(ipmc_time_t));
    IPMC_PKT_SET_EXIT();
}

BOOL ipmc_lib_packet_max_time_get(ipmc_time_t *pkt_time)
{
    if (!pkt_time) {
        return FALSE;
    }

    IPMC_PKT_SET_ENTER();
    ipmc_lib_time_cpy(pkt_time, &dispatch_max_time);
    IPMC_PKT_SET_EXIT();

    return TRUE;
}

static void ipmc_lib_packet_event_set(cyg_flag_value_t flag)
{
    cyg_flag_setbits(&IPMC_LIB_RX_flag, flag);
}

static BOOL ipmc_lib_packet_dispatch(void *contxt, const u8 *const frm, const vtss_packet_rx_info_t *const rx_info)
{
    BOOL        rc = FALSE; /* FALSE means continue processing */
#ifdef VTSS_SW_OPTION_PACKET
    ipmc_time_t exe_time_base;
    ipmc_time_t snp_time_diff, mvr_time_diff;
#endif /* VTSS_SW_OPTION_PACKET */

    if (!ipmc_lib_pkt_done_init) {
        return rc;
    }

    if (!frm || !rx_info) {
        return rc;
    }

    T_D("Start->(DP:%u/TX:%u/MR:%u/SR:%u)",
        dispatch_cnt, transmit_cnt, rx_mvr_cnt, rx_snp_cnt);

    ++dispatch_cnt;

#ifdef VTSS_SW_OPTION_PACKET
    memset(&snp_time_diff, 0x0, sizeof(ipmc_time_t));
    memset(&mvr_time_diff, 0x0, sizeof(ipmc_time_t));

    if (vtss_ipmc_lib_mvr_cb) {
        BOOL    snp_rcving;

        if (vtss_ipmc_lib_snp_cb) {
            snp_rcving = TRUE;
        } else {
            snp_rcving = FALSE;
        }

        ++rx_mvr_cnt;

        (void) ipmc_lib_time_curr_get(&exe_time_base);
        rc = vtss_ipmc_lib_mvr_cb(contxt, frm, rx_info, snp_rcving);
        (void) ipmc_lib_time_diff_get(FALSE, FALSE, NULL, &exe_time_base, &mvr_time_diff);
    }

    if (!rc && vtss_ipmc_lib_snp_cb) {
        ++rx_snp_cnt;

        (void) ipmc_lib_time_curr_get(&exe_time_base);
        rc = vtss_ipmc_lib_snp_cb(contxt, frm, rx_info, FALSE);
        (void) ipmc_lib_time_diff_get(FALSE, FALSE, NULL, &exe_time_base, &snp_time_diff);
    }

    if (ipmc_lib_time_cmp(&mvr_time_diff, &dispatch_max_time) == IPMC_LIB_TIME_CMP_GREATER) {
        ipmc_lib_time_cpy(&dispatch_max_time, &mvr_time_diff);
    }
    if (ipmc_lib_time_cmp(&snp_time_diff, &dispatch_max_time) == IPMC_LIB_TIME_CMP_GREATER) {
        ipmc_lib_time_cpy(&dispatch_max_time, &snp_time_diff);
    }

    T_D("RC=%s consumes SNP:%u.%um%uu/MVR:%u.%um%uu/MAX:%u.%um%uu->(DP:%u/TX:%u/MR:%u/SR:%u)",
        rc ? "TRUE" : "FALSE",
        snp_time_diff.sec, snp_time_diff.msec, snp_time_diff.usec,
        mvr_time_diff.sec, mvr_time_diff.msec, mvr_time_diff.usec,
        dispatch_max_time.sec, dispatch_max_time.msec, dispatch_max_time.usec,
        dispatch_cnt, transmit_cnt, rx_mvr_cnt, rx_snp_cnt);
#endif /* VTSS_SW_OPTION_PACKET */

    return rc;
}

static BOOL ipmc_lib_packet_queue_snd(void *contxt, const u8 *const frm, const vtss_packet_rx_info_t *const rx_info)
{
#ifdef VTSS_SW_OPTION_PACKET
    u8              *rcv_buf;
    u32             aligned_rx_info_len_bytes, aligned_frm_len_bytes;
#endif /* VTSS_SW_OPTION_PACKET */

    /* Return FALSE means continue processing in packet_rx */

    if (!ipmc_lib_pkt_done_init) {
        return FALSE;
    }

#ifdef VTSS_SW_OPTION_PACKET
    if (!frm || !rx_info) {
        return FALSE;
    }

    IPMC_PKT_RCV_ENTER();
    aligned_rx_info_len_bytes = sizeof(vtss_packet_rx_info_t);
    aligned_rx_info_len_bytes = sizeof(int) * ((aligned_rx_info_len_bytes + 3) / sizeof(int));
    aligned_frm_len_bytes = rx_info->length;
    aligned_frm_len_bytes = sizeof(int) * ((aligned_frm_len_bytes + 3) / sizeof(int));

    rcv_buf = vtss_bip_buffer_reserve(&IPMC_LIB_RX_bip, aligned_rx_info_len_bytes + aligned_frm_len_bytes);
    if (!rcv_buf) {
        T_I("Failure in reserving BIP(BUF_SZ:%d/CMT_SZ:%d)",
            vtss_bip_buffer_get_buffer_size(&IPMC_LIB_RX_bip),
            vtss_bip_buffer_get_committed_size(&IPMC_LIB_RX_bip));
        IPMC_PKT_RCV_EXIT();
        return FALSE;
    }
    if ((u32)rcv_buf & 0x3) {
        T_E("BIP buffer not correctly aligned");
    }

    memcpy(&rcv_buf[0], rx_info, sizeof(vtss_packet_rx_info_t));
    memcpy(&rcv_buf[aligned_rx_info_len_bytes], frm, rx_info->length);

    vtss_bip_buffer_commit(&IPMC_LIB_RX_bip);
    IPMC_PKT_RCV_EXIT();

    ipmc_lib_packet_event_set(IPMC_PKT_EVENT_KICK);
#endif /* VTSS_SW_OPTION_PACKET */

    return FALSE;
}

static vtss_packet_rx_info_t ipmc_lib_bip_rtv_rx_info;
#if !IPMC_LIB_PKT_BIP_ZC
static u8                    ipmc_lib_bip_rtv_frm[IPMC_LIB_PKT_BUF_SZ];
#endif /* !IPMC_LIB_PKT_BIP_ZC */
static void ipmc_lib_packet_queue_rtv(void)
{
#ifdef VTSS_SW_OPTION_PACKET
    u8          *rcv_buf;
    int         buf_size;
    size_t      sz_val, sz_offset;
#endif /* VTSS_SW_OPTION_PACKET */

    /* Return FALSE means continue processing in packet_rx */

    if (!ipmc_lib_pkt_done_init) {
        return;
    }

#ifdef VTSS_SW_OPTION_PACKET
    buf_size = 0;
    IPMC_PKT_RCV_ENTER();
    while ((rcv_buf = vtss_bip_buffer_get_contiguous_block(&IPMC_LIB_RX_bip, &buf_size)) != NULL) {
        if ((u32)rcv_buf & 0x3) {
            T_E("BIP buffer not correctly aligned");
        }
        sz_val = sz_offset = 0;

#if IPMC_LIB_PKT_BIP_ZC
        IPMC_PKT_RCV_EXIT();

        sz_val = sizeof(vtss_packet_rx_info_t);
        memcpy(&ipmc_lib_bip_rtv_rx_info, &rcv_buf[sz_offset], sz_val);
        sz_offset = sz_val;
        sz_offset = sizeof(int) * ((sz_offset + 3) / sizeof(int));
        (void) ipmc_lib_packet_dispatch(NULL, &rcv_buf[sz_offset], &ipmc_lib_bip_rtv_rx_info);
        sz_val = sz_offset;
        sz_offset = ipmc_lib_bip_rtv_rx_info.length;
        sz_offset = sizeof(int) * ((sz_offset + 3) / sizeof(int));
        sz_val += sz_offset;

        IPMC_PKT_RCV_ENTER();
        vtss_bip_buffer_decommit_block(&IPMC_LIB_RX_bip, sz_val);
#else
        sz_val = sizeof(vtss_packet_rx_info_t);
        memcpy(&ipmc_lib_bip_rtv_rx_info, &rcv_buf[sz_offset], sz_val);
        sz_offset = sz_val;
        sz_offset = sizeof(int) * ((sz_offset + 3) / sizeof(int));
        memcpy(ipmc_lib_bip_rtv_frm, &rcv_buf[sz_offset], ipmc_lib_bip_rtv_rx_info.length);
        sz_val = sz_offset;
        sz_offset = ipmc_lib_bip_rtv_rx_info.length;
        sz_offset = sizeof(int) * ((sz_offset + 3) / sizeof(int));
        sz_val += sz_offset;
        vtss_bip_buffer_decommit_block(&IPMC_LIB_RX_bip, sz_val);

        IPMC_PKT_RCV_EXIT();
        if (sz_offset < IPMC_LIB_PKT_BUF_SZ) {
            sz_val = IPMC_LIB_PKT_BUF_SZ - sz_offset;
            memset(&ipmc_lib_bip_rtv_frm[sz_offset], 0x0, sz_val);
        }
        (void) ipmc_lib_packet_dispatch(NULL, ipmc_lib_bip_rtv_frm, &ipmc_lib_bip_rtv_rx_info);
        IPMC_PKT_RCV_ENTER();
#endif /* IPMC_LIB_PKT_BIP_ZC */
    }
    IPMC_PKT_RCV_EXIT();
#endif /* VTSS_SW_OPTION_PACKET */
}

static void ipmc_lib_pkt_thread(cyg_addrword_t data)
{
    cyg_flag_value_t    events;

    T_D("enter ipmc_lib_pkt_thread");

    while (!ipmc_lib_pkt_done_init) {
        VTSS_OS_MSLEEP(1000);
    }

    T_I("ipmc_lib_pkt_thread start");

    while (ipmc_lib_pkt_done_init) {
        events = cyg_flag_wait(&IPMC_LIB_RX_flag, IPMC_PKT_EVENT_ANY,
                               CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR);

        if (events & IPMC_PKT_EVENT_KICK) {
            ipmc_lib_packet_queue_rtv();
        }
    }

    T_W("exit ipmc_lib_pkt_thread");
}

vtss_rc ipmc_lib_packet_init(void)
{
    if (ipmc_lib_pkt_done_init) {
        return VTSS_OK;
    }

    if (!IPMC_MEM_JUMBO_MTAKE(snp_uip_buf)) {
        return VTSS_RC_ERROR;
    }
    if (!IPMC_MEM_JUMBO_MTAKE(mvr_uip_buf)) {
        IPMC_MEM_JUMBO_MGIVE(snp_uip_buf);
        return VTSS_RC_ERROR;
    }

    if (!vtss_bip_buffer_init(&IPMC_LIB_RX_bip, IPMC_LIB_BIP_BUF_SZ_B)) {
        IPMC_MEM_JUMBO_MGIVE(mvr_uip_buf);
        IPMC_MEM_JUMBO_MGIVE(snp_uip_buf);
        return VTSS_RC_ERROR;
    }

    critd_init(&ipmc_pkt_set_crit, "ipmc_pkt_set_crit", VTSS_MODULE_ID_IPMC_LIB, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
    IPMC_PKT_SET_EXIT();

    critd_init(&ipmc_pkt_rx_crit, "ipmc_pkt_rx_crit", VTSS_MODULE_ID_IPMC_LIB, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
    IPMC_PKT_RCV_EXIT();

    cyg_flag_init(&IPMC_LIB_RX_flag);
    cyg_thread_create(THREAD_DEFAULT_PRIO,
                      ipmc_lib_pkt_thread,
                      0,
                      "IPMC_PKT",
                      ipmc_lib_pkt_thread_stack,
                      sizeof(ipmc_lib_pkt_thread_stack),
                      &ipmc_lib_pkt_thread_handle,
                      &ipmc_lib_pkt_thread_block);
    cyg_thread_resume(ipmc_lib_pkt_thread_handle);

    ipmc_lib_packet_regs = 0x0;
    vtss_ipmc_lib_snp_cb = NULL;
    vtss_ipmc_lib_mvr_cb = NULL;

    IPMC_PKT_SET_ENTER();
    memset(&dispatch_max_time, 0x0, sizeof(ipmc_time_t));
    dispatch_cnt = transmit_cnt = rx_mvr_cnt = rx_snp_cnt = 0;
    IPMC_PKT_SET_EXIT();

    ipmc_lib_pkt_done_init = TRUE;
    return VTSS_OK;
}

vtss_rc ipmc_lib_packet_register(ipmc_owner_t owner, void *cb)
{
    u8  original_regs;

    if (!ipmc_lib_pkt_done_init) {
        return VTSS_RC_ERROR;
    }

    IPMC_PKT_SET_ENTER();

    original_regs = ipmc_lib_packet_regs;

    switch ( owner ) {
    case IPMC_OWNER_INIT:
        ipmc_lib_packet_regs = 0x0;

        break;
    case IPMC_OWNER_ALL:
        ipmc_lib_packet_regs = 0xFF;

        break;
    case IPMC_OWNER_SNP:
        ipmc_lib_packet_regs |= (1 << IPMC_OWNER_IGMP);
        ipmc_lib_packet_regs |= (1 << IPMC_OWNER_MLD);
        ipmc_lib_packet_regs |= (1 << IPMC_OWNER_SNP);
        ipmc_lib_packet_regs |= (1 << IPMC_OWNER_SNP4);
        ipmc_lib_packet_regs |= (1 << IPMC_OWNER_SNP6);
        if (vtss_ipmc_lib_snp_cb == NULL) {
            vtss_ipmc_lib_snp_cb = cb;
        }

        break;
    case IPMC_OWNER_MVR:
        ipmc_lib_packet_regs |= (1 << IPMC_OWNER_IGMP);
        ipmc_lib_packet_regs |= (1 << IPMC_OWNER_MLD);
        ipmc_lib_packet_regs |= (1 << IPMC_OWNER_MVR);
        ipmc_lib_packet_regs |= (1 << IPMC_OWNER_MVR4);
        ipmc_lib_packet_regs |= (1 << IPMC_OWNER_MVR6);
        if (vtss_ipmc_lib_mvr_cb == NULL) {
            vtss_ipmc_lib_mvr_cb = cb;
        }

        break;
    case IPMC_OWNER_SNP4:
    case IPMC_OWNER_SNP6:
        ipmc_lib_packet_regs |= (1 << IPMC_OWNER_SNP);
        ipmc_lib_packet_regs |= (1 << owner);
        if (vtss_ipmc_lib_snp_cb == NULL) {
            vtss_ipmc_lib_snp_cb = cb;
        }

        break;
    case IPMC_OWNER_MVR4:
    case IPMC_OWNER_MVR6:
        ipmc_lib_packet_regs |= (1 << IPMC_OWNER_MVR);
        ipmc_lib_packet_regs |= (1 << owner);
        if (vtss_ipmc_lib_mvr_cb == NULL) {
            vtss_ipmc_lib_mvr_cb = cb;
        }

        break;
    case IPMC_OWNER_MAX:
        IPMC_PKT_SET_EXIT();
        return VTSS_RC_ERROR;
    default:
        ipmc_lib_packet_regs |= (1 << owner);

        break;
    }

    T_I("(%d)original_regs/ipmc_lib_packet_regs:%u/%u", owner, original_regs, ipmc_lib_packet_regs);
    if (ipmc_lib_packet_regs & IPMC_LIB_CHK_PKT_REG4) {
        if (!(original_regs & IPMC_LIB_CHK_PKT_REG4)) {
            (void) vtss_ipmc_lib_rx_register(ipmc_lib_packet_queue_snd, IPMC_IP_VERSION_IGMP);
        }
    }
    if (ipmc_lib_packet_regs & IPMC_LIB_CHK_PKT_REG6) {
        if (!(original_regs & IPMC_LIB_CHK_PKT_REG6)) {
            (void) vtss_ipmc_lib_rx_register(ipmc_lib_packet_queue_snd, IPMC_IP_VERSION_MLD);
        }
    }
    IPMC_PKT_SET_EXIT();

    return VTSS_OK;
}

vtss_rc ipmc_lib_packet_unregister(ipmc_owner_t owner)
{
    u8      original_regs, mask;
    BOOL    mask_module, empty_packet_regs;

    if (!ipmc_lib_pkt_done_init) {
        return VTSS_RC_ERROR;
    }

    IPMC_PKT_SET_ENTER();

    original_regs = ipmc_lib_packet_regs;
    mask = 0x0;
    mask_module = TRUE;

    switch ( owner ) {
    case IPMC_OWNER_INIT:
        ipmc_lib_packet_regs = 0xFF;
        mask_module = FALSE;

        break;
    case IPMC_OWNER_ALL:
        ipmc_lib_packet_regs = 0x0;
        mask_module = FALSE;

        break;
    case IPMC_OWNER_SNP:
        mask |= (1 << IPMC_OWNER_SNP);
        mask |= (1 << IPMC_OWNER_SNP4);
        mask |= (1 << IPMC_OWNER_SNP6);
        ipmc_lib_packet_regs &= (~mask);
        if (vtss_ipmc_lib_snp_cb) {
            vtss_ipmc_lib_snp_cb = NULL;
        }
        mask_module = FALSE;

        break;
    case IPMC_OWNER_MVR:
        mask |= (1 << IPMC_OWNER_MVR);
        mask |= (1 << IPMC_OWNER_MVR4);
        mask |= (1 << IPMC_OWNER_MVR6);
        ipmc_lib_packet_regs &= (~mask);
        if (vtss_ipmc_lib_mvr_cb) {
            vtss_ipmc_lib_mvr_cb = NULL;
        }
        mask_module = FALSE;

        break;
    case IPMC_OWNER_SNP4:
    case IPMC_OWNER_SNP6:
    case IPMC_OWNER_MVR4:
    case IPMC_OWNER_MVR6:
        mask |= (1 << owner);
        ipmc_lib_packet_regs &= (~mask);

        break;
    case IPMC_OWNER_IGMP:
        mask |= (1 << IPMC_OWNER_IGMP);
        mask |= (1 << IPMC_OWNER_SNP4);
        mask |= (1 << IPMC_OWNER_MVR4);
        ipmc_lib_packet_regs &= (~mask);

        break;
    case IPMC_OWNER_MLD:
        mask |= (1 << IPMC_OWNER_MLD);
        mask |= (1 << IPMC_OWNER_SNP6);
        mask |= (1 << IPMC_OWNER_MVR6);
        ipmc_lib_packet_regs &= (~mask);

        break;
    default:
        IPMC_PKT_SET_EXIT();
        return VTSS_RC_ERROR;
    }

    empty_packet_regs = FALSE;
    if (mask_module) {
        if (!(ipmc_lib_packet_regs & IPMC_LIB_CHK_PKT_R_SNP)) {
            mask = 0x0;
            mask |= (1 << IPMC_OWNER_SNP);
            ipmc_lib_packet_regs &= (~mask);
            if (vtss_ipmc_lib_snp_cb) {
                vtss_ipmc_lib_snp_cb = NULL;
            }
        }
        if (!(ipmc_lib_packet_regs & IPMC_LIB_CHK_PKT_R_MVR)) {
            mask = 0x0;
            mask |= (1 << IPMC_OWNER_MVR);
            ipmc_lib_packet_regs &= (~mask);
            if (vtss_ipmc_lib_mvr_cb) {
                vtss_ipmc_lib_mvr_cb = NULL;
            }
        }
    }
    if (!(ipmc_lib_packet_regs & IPMC_LIB_CHK_PKT_MSK4R)) {
        mask = 0x0;
        mask |= (1 << IPMC_OWNER_IGMP);
        ipmc_lib_packet_regs &= (~mask);
    }
    if (!(ipmc_lib_packet_regs & IPMC_LIB_CHK_PKT_MSK6R)) {
        mask = 0x0;
        mask |= (1 << IPMC_OWNER_MLD);
        ipmc_lib_packet_regs &= (~mask);
    }

    T_I("(%d)original_regs/ipmc_lib_packet_regs:%u/%u", owner, original_regs, ipmc_lib_packet_regs);
    if (original_regs & IPMC_LIB_CHK_PKT_REG4) {
        if (!(ipmc_lib_packet_regs & IPMC_LIB_CHK_PKT_REG4)) {
            (void)vtss_ipmc_lib_rx_unregister(IPMC_IP_VERSION_IGMP);
        }
    }
    if (original_regs & IPMC_LIB_CHK_PKT_REG6) {
        if (!(ipmc_lib_packet_regs & IPMC_LIB_CHK_PKT_REG6)) {
            (void)vtss_ipmc_lib_rx_unregister(IPMC_IP_VERSION_MLD);
        }
    }

    if (!ipmc_lib_packet_regs) {
        empty_packet_regs = TRUE;
    }
    IPMC_PKT_SET_EXIT();

    if (empty_packet_regs) {
        IPMC_PKT_RCV_ENTER();
        vtss_bip_buffer_clear(&IPMC_LIB_RX_bip);
        IPMC_PKT_RCV_EXIT();
    }

    return VTSS_OK;
}

static ushort mld_chksum_tx(mld_ip6_hdr *ip, ulong offset, ulong len)
{
    mld_icmp_pseudo_t   *icmpChkSum;
    ushort              sum;
    ushort              *sdata;

    if ((len < MLD_GEN_MIN_PAYLOAD_LEN) ||
        ((len + IPV6_HDR_FIXED_LEN) > sizeof(mld_icmp_pseudo_t)) ||
        !IPMC_MEM_SYSTEM_MTAKE(icmpChkSum, sizeof(mld_icmp_pseudo_t))) {
        return FALSE;
    }

    memset(icmpChkSum, 0x0, sizeof(mld_icmp_pseudo_t));
    memcpy(&icmpChkSum->pseudoSrc, &ip->ip6_src, sizeof(vtss_ipv6_t));
    memcpy(&icmpChkSum->pseudoDst, &ip->ip6_dst, sizeof(vtss_ipv6_t));
    icmpChkSum->pseudoLen = htonl(len);
    icmpChkSum->pseudoNextHdr = MLD_IPV6_NEXTHDR_ICMP;

    memcpy(&icmpChkSum->ctrl, (char *)ip + offset, len);
    icmpChkSum->ctrl.pkt.common.checksum = 0x0;

    sdata = (ushort *)icmpChkSum;
    len = len + IPV6_HDR_FIXED_LEN;

    sum = 0;
    for (; len > 1; len -= 2) {
        sum += *sdata;
        if (sum < *sdata) {
            /* Overflow, so we add the carry to sum (i.e., increase by one). */
            ++sum;
        }

        ++sdata;
    }

    /* add up any odd byte */
    if (len == 1) {
        sum += (((ushort)(*(uchar *)sdata)) << 8);
        if (sum < (((ushort)(*(uchar *)sdata)) << 8)) {
            ++sum;
        }
    }

    IPMC_MEM_SYSTEM_MGIVE(icmpChkSum);

    /* one's complement */
    return (~sum);
}

static BOOL mld_chksum_rx(mld_ip6_hdr *ip, ulong offset, ulong len, ushort chkSum)
{
    mld_icmp_pseudo_t   *icmpChkSum;
    ushort              sum;
    ushort              *sdata;

    if ((len < MLD_GEN_MIN_PAYLOAD_LEN) ||
        ((len + IPV6_HDR_FIXED_LEN) > sizeof(mld_icmp_pseudo_t)) ||
        !IPMC_MEM_SYSTEM_MTAKE(icmpChkSum, sizeof(mld_icmp_pseudo_t))) {
        return FALSE;
    }

    memset(icmpChkSum, 0x0, sizeof(mld_icmp_pseudo_t));
    memcpy(&icmpChkSum->pseudoSrc, &ip->ip6_src, sizeof(vtss_ipv6_t));
    memcpy(&icmpChkSum->pseudoDst, &ip->ip6_dst, sizeof(vtss_ipv6_t));
    icmpChkSum->pseudoLen = htonl(len);
    icmpChkSum->pseudoNextHdr = MLD_IPV6_NEXTHDR_ICMP;

    memcpy(&icmpChkSum->ctrl, (char *)ip + offset, len);
    icmpChkSum->ctrl.pkt.common.checksum = 0x0;

    sdata = (ushort *)icmpChkSum;
    len = len + IPV6_HDR_FIXED_LEN;

    sum = 0;
    for (; len > 1; len -= 2) {
        sum += *sdata;
        if (sum < *sdata) {
            /* Overflow, so we add the carry to sum (i.e., increase by one). */
            ++sum;
        }

        ++sdata;
    }

    /* add up any odd byte */
    if (len == 1) {
        sum += (((ushort)(*(uchar *)sdata)) << 8);
        if (sum < (((ushort)(*(uchar *)sdata)) << 8)) {
            ++sum;
        }
    }

    /* one's complement */
    sum = ~sum;

    IPMC_MEM_SYSTEM_MGIVE(icmpChkSum);

    if (chkSum != sum) {
        return FALSE;
    } else {
        return TRUE;
    }
}

static ushort igmp_chksum_tx(ushort *sdata, ulong len)
{
    ushort  sum = 0;

    for (; len > 1; len -= 2) {
        sum += *sdata;
        if (sum < *sdata) {
            /* Overflow, so we add the carry to sum (i.e., increase by one). */
            ++sum;
        }

        ++sdata;
    }

    /* add up any odd byte */
    if (len == 1) {
        sum += (((ushort)(*(uchar *)sdata)) << 8);
        if (sum < (((ushort)(*(uchar *)sdata)) << 8)) {
            ++sum;
        }
    }

    /* one's complement */
    return (~sum);
}

static BOOL igmp_chksum_rx(igmp_ip4_hdr *ip, ulong offset, ulong len, ushort chkSum)
{
    ipmc_igmp_packet_t  *igmp;
    ushort              sum;
    ushort              *sdata;

    igmp = (ipmc_igmp_packet_t *) ((char *)ip + offset);
    igmp->common.checksum = 0;
    sdata = (ushort *)igmp;
    sum = 0;
    for (; len > 1; len -= 2) {
        sum += *sdata;
        if (sum < *sdata) {
            /* Overflow, so we add the carry to sum (i.e., increase by one). */
            ++sum;
        }

        ++sdata;
    }

    /* add up any odd byte */
    if (len == 1) {
        sum += (((ushort)(*(uchar *)sdata)) << 8);
        if (sum < (((ushort)(*(uchar *)sdata)) << 8)) {
            ++sum;
        }
    }

    /* one's complement */
    sum = ~sum;

    igmp->common.checksum = chkSum;
    if (chkSum != sum) {
        return FALSE;
    } else {
        ulong   ip_len;
        ushort  ip_chksum;

        ip_len = (ulong) ((ip->vhl & 0xF) * 4);
        ip_chksum = ip->ip_chksum;
        ip->ip_chksum = 0;
        sdata = (ushort *)ip;
        sum = 0;
        for (; ip_len > 1; ip_len -= 2) {
            sum += *sdata;
            if (sum < *sdata) {
                /* Overflow, so we add the carry to sum (i.e., increase by one). */
                ++sum;
            }

            ++sdata;
        }

        /* add up any odd byte */
        if (ip_len == 1) {
            sum += (((ushort)(*(uchar *)sdata)) << 8);
            if (sum < (((ushort)(*(uchar *)sdata)) << 8)) {
                ++sum;
            }
        }

        /* one's complement */
        sum = ~sum;

        ip->ip_chksum = ip_chksum;
        if (ip_chksum != sum) {
            return FALSE;
        } else {
            return TRUE;
        }
    }
}

static void ipmc_lib_packet_dst_mac(u8 *macadr, vtss_ipv6_t *ipadr, ipmc_ip_version_t version)
{
    u32 ip2mac;

    if (!macadr || !ipadr) {
        return;
    }

    memcpy((u8 *)&ip2mac, &ipadr->addr[12], sizeof(ipmcv4addr));
    ip2mac = ntohl(ip2mac);

    switch ( version ) {
    case IPMC_IP_VERSION_IPV4Z:
    case IPMC_IP_VERSION_IGMP:
        ip2mac = ip2mac << IPMC_IP2MAC_V4SHIFT_LEN;
        ip2mac = ip2mac >> IPMC_IP2MAC_V4SHIFT_LEN;
        *(macadr + 0) = IPMC_IP2MAC_V4MAC_ARRAY0;
        *(macadr + 1) = IPMC_IP2MAC_V4MAC_ARRAY1;
        *(macadr + 2) = IPMC_IP2MAC_V4MAC_ARRAY2;
        *(macadr + 3) = (ip2mac >> IPMC_IP2MAC_ARRAY3_SHIFT_LEN) & IPMC_IP2MAC_ARRAY_MASK;
        *(macadr + 4) = (ip2mac >> IPMC_IP2MAC_ARRAY4_SHIFT_LEN) & IPMC_IP2MAC_ARRAY_MASK;
        *(macadr + 5) = (ip2mac >> IPMC_IP2MAC_ARRAY5_SHIFT_LEN) & IPMC_IP2MAC_ARRAY_MASK;

        break;
    case IPMC_IP_VERSION_IPV6Z:
    case IPMC_IP_VERSION_MLD:
        ip2mac = ip2mac << IPMC_IP2MAC_V6SHIFT_LEN;
        ip2mac = ip2mac >> IPMC_IP2MAC_V6SHIFT_LEN;
        *(macadr + 0) = IPMC_IP2MAC_V6MAC_ARRAY0;
        *(macadr + 1) = IPMC_IP2MAC_V6MAC_ARRAY1;
        *(macadr + 2) = (ip2mac >> IPMC_IP2MAC_ARRAY2_SHIFT_LEN) & IPMC_IP2MAC_ARRAY_MASK;
        *(macadr + 3) = (ip2mac >> IPMC_IP2MAC_ARRAY3_SHIFT_LEN) & IPMC_IP2MAC_ARRAY_MASK;
        *(macadr + 4) = (ip2mac >> IPMC_IP2MAC_ARRAY4_SHIFT_LEN) & IPMC_IP2MAC_ARRAY_MASK;
        *(macadr + 5) = (ip2mac >> IPMC_IP2MAC_ARRAY5_SHIFT_LEN) & IPMC_IP2MAC_ARRAY_MASK;

        break;
    default:

        break;
    }
}

void ipmc_lib_packet_strip_vtag(const u8 *const frame, u8 *frm, vtss_tag_type_t tag_type, vtss_packet_rx_info_t *rx_info)
{
    if (tag_type != VTSS_TAG_TYPE_UNTAGGED) {
        if (((frame[12] == 0x08) && (frame[13] == 0x00)) ||
            ((frame[12] == 0x86) && (frame[13] == 0xDD))) {
            memcpy(frm, frame, rx_info->length);
        } else {
            memcpy(frm, frame, 12);
            rx_info->length -= 4;
            if (((frame[16] == 0x08) && (frame[17] == 0x00)) ||
                ((frame[16] == 0x86) && (frame[17] == 0xDD))) {
                memcpy(&frm[12], &frame[16], (rx_info->length - 16));
            } else {
                rx_info->length -= 4;
                memcpy(&frm[12], &frame[20], (rx_info->length - 20));
            }
        }
    } else {
        memcpy(frm, frame, rx_info->length);
    }
}

vtss_rc ipmc_lib_packet_tx(ipmc_port_bfs_t *dst_port_mask,
                           BOOL force_untag,
                           BOOL fast_leave,
                           u32 src_port,
                           ipmc_pkt_src_port_t src_type,
                           vtss_vid_t vid,
                           BOOL cfi,
                           u8 uprio,
                           vtss_glag_no_t glag_id,
                           const u8 *const frame,
                           size_t len)
{
    u8                          tag_offset, vtag[8], *pkt_buf;
    u32                         i, local_port_cnt, tx_len;
    vtss_packet_port_info_t     info;
    vtss_packet_port_filter_t   filter[VTSS_PORT_ARRAY_SIZE];
    packet_tx_props_t           tx_props;
    vtss_rc                     tx_status;
    BOOL                        is_stacking;
#if VTSS_SWITCH_STACKABLE
    BOOL                        priv_flag;
#endif /* VTSS_SWITCH_STACKABLE */
#if IPMC_LIB_TX_PRIO_TAG
    BOOL                        priority_tag;
#endif /* IPMC_LIB_TX_PRIO_TAG */

    ipmc_lib_lock();
    if (!ipmc_lib_pkt_done_init) {
        ipmc_lib_unlock();
        return VTSS_RC_ERROR;
    }
    ipmc_lib_unlock();

    if (!frame || !dst_port_mask) {
        return VTSS_RC_ERROR;
    }

    /*
        Fill out frame information for filtering
        Ingress port number or VTSS_PORT_NO_NONE
    */
    info.port_no = src_port;
#if defined(VTSS_FEATURE_AGGR_GLAG)
    /*
        Ingress GLAG number or zero
    */
    info.glag_no = glag_id;
#endif /* VTSS_FEATURE_AGGR_GLAG */
    info.vid = vid;

    local_port_cnt = ipmc_lib_get_system_local_port_cnt();
    for (i = 0; i < local_port_cnt; i++) {
        if (VTSS_PORT_BF_GET(dst_port_mask->member_ports, i) == FALSE) {
            continue;
        }

        if ((vtss_packet_port_filter_get(NULL, &info, filter) != VTSS_OK) ||
            (filter[i].filter == VTSS_PACKET_FILTER_DISCARD)) {
            continue;
        }

        tag_offset = 0;
        memset(vtag, 0x0, sizeof(vtag));
#if VTSS_SWITCH_STACKABLE
        is_stacking = port_isid_port_no_is_stack(VTSS_ISID_LOCAL, i);
#else
        is_stacking = FALSE;
#endif /* VTSS_SWITCH_STACKABLE */
#if IPMC_LIB_TX_PRIO_TAG
        priority_tag = FALSE;
#endif /* IPMC_LIB_TX_PRIO_TAG */
        if (!is_stacking && (filter[i].filter == VTSS_PACKET_FILTER_TAGGED)) {
            if (force_untag) {
#if IPMC_LIB_TX_PRIO_TAG
                priority_tag = TRUE;
                tag_offset = 4;
#endif /* IPMC_LIB_TX_PRIO_TAG */
            } else {
                if (filter[i].tpid != 0x8100) {
                    tag_offset = 8;
                } else {
                    tag_offset = 4;
                }
            }
        }

        tx_len = (len + tag_offset > IPMC_LIB_PKT_MIN_XMT_SZ) ? (len + tag_offset) : IPMC_LIB_PKT_MIN_XMT_SZ;
#if VTSS_SWITCH_STACKABLE
        priv_flag = FALSE;
        if (is_stacking &&
            (fast_leave || (src_type == IPMC_PKT_SRC_MVR_SOURC) || (src_type == IPMC_PKT_SRC_MVR_RECVR))) {
            tx_len += IPMC_PKT_PRIVATE_PAD_LEN;
            priv_flag = TRUE;
        }
#endif /* VTSS_SWITCH_STACKABLE */

        tx_status = VTSS_RC_ERROR;
        if (force_untag || is_stacking ||
            (filter[i].filter == VTSS_PACKET_FILTER_UNTAGGED)) {
            // Injecting on either a stack port or untagged on a front port.
            if ((pkt_buf = packet_tx_alloc(tx_len))) {
#if IPMC_LIB_TX_PRIO_TAG
                if (priority_tag) {
                    vtag[0] = 0x81;
                    vtag[2] = ((uprio << 5) | (cfi << 4));

                    // Injecting with priority tag
                    memcpy(pkt_buf, frame, 12); // DMAC & SMAC
                    memcpy(pkt_buf + 12, vtag, tag_offset); // VLAN Header
                    memcpy(pkt_buf + 12 + tag_offset, frame + 12, len - 12); // Remainder of frame
                } else {
                    memcpy(pkt_buf, frame, len);
                }
#else
                memcpy(pkt_buf, frame, len);
#endif /* IPMC_LIB_TX_PRIO_TAG */
                if (tx_len > (len + tag_offset)) {
                    memset(pkt_buf + len + tag_offset, 0x0, (tx_len - len - tag_offset));
                }

#if VTSS_SWITCH_STACKABLE
                if (priv_flag) {
                    u32 padding = IPMC_PKT_PRIVATE_PAD_MAGIC;

                    padding += (src_type & 0xF);
                    if (fast_leave) {
                        padding |= 0xF0;
                    }
                    padding = htonl(padding);
                    memcpy(pkt_buf + (tx_len - IPMC_PKT_PRIVATE_PAD_LEN), (u8 *)&padding, IPMC_PKT_PRIVATE_PAD_LEN);
                }
#endif /* VTSS_SWITCH_STACKABLE */

                packet_tx_props_init(&tx_props);
                tx_props.packet_info.modid     = VTSS_MODULE_ID_IPMC_LIB;
                tx_props.packet_info.frm[0]    = pkt_buf;
                tx_props.packet_info.len[0]    = tx_len;
                tx_props.tx_info.dst_port_mask = VTSS_BIT64(i);
#if VTSS_SWITCH_STACKABLE
                if (is_stacking) {
                    tx_props.tx_info.vstax.sym.fwd_mode    = VTSS_VSTAX_FWD_MODE_CPU_ALL; // Forward to all CPUs that can be reached...
                    tx_props.tx_info.vstax.sym.ttl         = VTSS_VSTAX_TTL_PORT;         // ... with the stack port's TTL.
                    tx_props.tx_info.vstax.sym.prio        = 7;
                    tx_props.tx_info.vstax.sym.upsid       = 1;
                    tx_props.tx_info.vstax.sym.tci.vid     = vid;
                    tx_props.tx_info.vstax.sym.tci.cfi     = cfi;
                    tx_props.tx_info.vstax.sym.tci.tagprio = uprio;
                    tx_props.tx_info.vstax.sym.port_no     = 0;
#if defined(VTSS_FEATURE_AGGR_GLAG)
                    tx_props.tx_info.vstax.sym.glag_no     = glag_id;
#endif /* VTSS_FEATURE_AGGR_GLAG */
                    tx_props.tx_info.vstax.sym.queue_no    = PACKET_XTR_QU_IGMP;
                    tx_props.tx_info.tx_vstax_hdr          = VTSS_PACKET_TX_VSTAX_SYM;
                    T_D("IPMCLIB transmit untagged on StackingIPort %u with vid %d", i, vid);
                } else {
                    T_D("IPMCLIB transmit untagged on FrontIPort %u with vid %d", i, vid);
                }
#else
                T_D("IPMCLIB transmit untagged on iPort %u with vid %d", i, vid);
#endif /* VTSS_SWITCH_STACKABLE */

                T_D_HEX(&pkt_buf[0], tx_len);
                tx_status = packet_tx(&tx_props);
            }
        } else {
            if ((pkt_buf = packet_tx_alloc(tx_len))) {
                if (filter[i].tpid != 0x8100) {
                    T_D("IPMCLIB transmit tagged on iPort %u with TPID-%u/SVID-%u/CVID-%u", i, filter[i].tpid, info.vid, vid);

                    vtag[0] = ((filter[i].tpid >> 8) & 0xFF);
                    vtag[1] = (filter[i].tpid & 0xFF);
                    vtag[2] = ((uprio << 5) | (cfi << 4) | (info.vid >> 8));
                    vtag[3] = (info.vid & 0xFF);
                    vtag[4] = 0x81;
                    vtag[6] = ((uprio << 5) | (cfi << 4) | (vid >> 8));
                    vtag[7] = (vid & 0xFF);
                } else {
                    T_D("IPMCLIB transmit tagged on iPort %u with vid %d", i, vid);

                    vtag[0] = ((filter[i].tpid >> 8) & 0xFF);
                    vtag[1] = (filter[i].tpid & 0xFF);
                    vtag[2] = ((uprio << 5) | (cfi << 4) | (vid >> 8));
                    vtag[3] = (vid & 0xFF);
                }

                // Injecting with VLAN tag on front port.
                memcpy(pkt_buf, frame, 12); // DMAC & SMAC
                memcpy(pkt_buf + 12, vtag, tag_offset); // VLAN Header
                memcpy(pkt_buf + 12 + tag_offset, frame + 12, len - 12); // Remainder of frame
                if (tx_len > (len + tag_offset)) {
                    memset(pkt_buf + len + tag_offset, 0x0, (tx_len - len - tag_offset));
                }

                packet_tx_props_init(&tx_props);
                tx_props.packet_info.modid     = VTSS_MODULE_ID_IPMC_LIB;
                tx_props.packet_info.frm[0]    = pkt_buf;
                tx_props.packet_info.len[0]    = tx_len;
                tx_props.tx_info.dst_port_mask = VTSS_BIT64(i);

                T_D_HEX(&pkt_buf[0], tx_len);
                tx_status = packet_tx(&tx_props);
            }
        }

        IPMC_PKT_SET_ENTER();
        ++transmit_cnt;
        T_D("%s(%d) -> packet_tx: Port-%u LEN=%d/%u (DP:%u/TX:%u/MR:%u/SR:%u)",
            (tx_status == VTSS_RC_OK) ? "OK" : "NG",
            tx_status,
            i,
            len + tag_offset,
            tx_len,
            dispatch_cnt, transmit_cnt, rx_mvr_cnt, rx_snp_cnt);
        IPMC_PKT_SET_EXIT();
    }

    return VTSS_OK;
}

BOOL ipmc_lib_packet_tx_mxrc_qqic(ipmc_pkt_exp_t type, ipmc_intf_entry_t *input, void *output)
{
    u32 val;

    if (!input || !output) {
        return FALSE;
    }

    switch ( type ) {
    case IPMC_V4_GEN_MXRC:
    case IPMC_V4_SFM_MXRC:
        val = IPMC_TIMER_QRI(input) * 10;
        if ((val < 128) || (type == IPMC_V4_GEN_MXRC)) {
            *(u8 *)output = val & 0xFF;
            break;
        }

        /*
            (8-Bit)
            0 123 4567
            1 exp mant
            (mant | 0x10) << (exp + 3)
        */

        break;
    case IPMC_V4_SFM_QQIC:
    case IPMC_V6_SFM_QQIC:
        val = IPMC_TIMER_QI(input);

        if (val < 128) {
            *(u8 *)output = val & 0xFF;
        } else {
            /*
                (8-Bit)
                0 123 4567
                1 exp mant
                (mant | 0x10) << (exp + 3)
            */
        }

        break;
    case IPMC_V6_GEN_MXRC:
    case IPMC_V6_SFM_MXRC:
        val = IPMC_TIMER_QRI(input) * 1000;
        if ((val < 32768) || (type == IPMC_V6_GEN_MXRC)) {
            *(u16 *)output = val & 0xFFFF;
            break;
        }

        /*
            (16-Bit)
            0 123 4567...
            1 exp mant
            (mant | 0x1000) << (exp + 3)
        */

        break;
    default:
        return FALSE;
    }

    return TRUE;
}

BOOL ipmc_lib_packet_rx_mxrc_qqic(ipmc_pkt_exp_t type, void *input, u32 *output)
{
    u8  p;
    u16 q;

    if (!input || !output) {
        return FALSE;
    }

    *output = 0;
    switch ( type ) {
    case IPMC_V4_GEN_MXRC:
        /* unit: tenth of second */
        *output = *(u8 *)input / 10;

        break;
    case IPMC_V4_SFM_MXRC:
    case IPMC_V4_SFM_QQIC:
    case IPMC_V6_SFM_QQIC:
        p = *(u8 *)input;

        if (p < 128) {
            *output = p;
        } else {
            /*
                (8-Bit)
                0 123 4567
                1 exp mant
                (mant | 0x10) << (exp + 3)
            */
            *output = (p & 0xF) | 0x10;
            *output = *output << (((p >> 4) & 0x7) + 3);
        }

        if (type == IPMC_V4_SFM_MXRC) {
            /* unit: tenth of second */
            *output = *output / 10;
        }

        break;
    case IPMC_V6_GEN_MXRC:
        /* unit: millisecond */
        *output = *(u16 *)input / 1000;

        break;
    case IPMC_V6_SFM_MXRC:
        q = *(u16 *)input;

        if (q < 32768) {
            *output = q;
        } else {
            /*
                (16-Bit)
                0 123 4567...
                1 exp mant
                (mant | 0x1000) << (exp + 3)
            */
            *output = (q & 0xFFF) | 0x1000;
            *output = *output << (((q >> 12) & 0x7) + 3);
        }

        /* unit: millisecond */
        *output = *output / 1000;

        break;
    default:
        return FALSE;
    }

    return TRUE;
}

vtss_rc ipmc_lib_packet_tx_gq(ipmc_intf_entry_t *entry,
                              vtss_ipv6_t *query_group_addr,
                              BOOL force_untag,
                              BOOL debug)
{
    u8                  *uip_buf;
    size_t              uip_len;
    ipmc_ip_eth_hdr     *eth_hdr;
    ipmc_ip_mld_hdr     *ip_mld_gen_hdr = NULL;
    ipmc_ip_mldv2_hdr   *ip_mld_sfm_hdr = NULL;
    ipmc_ip_igmp_hdr    *ip_igmp_gen_hdr = NULL;
    ipmc_ip_igmpv3_hdr  *ip_igmp_sfm_hdr = NULL;
    u8                  dst_ports[VTSS_PORT_BF_SIZE];
    uchar               mac_addr[6];
    u32                 i, local_port_cnt;
    ipmc_port_bfs_t     dst_port_mask;
    BOOL                tx_flag;

    if (!ipmc_lib_pkt_done_init) {
        return VTSS_RC_ERROR;
    }

    if (!entry || !query_group_addr) {
        return VTSS_RC_ERROR;
    }

    if (entry->param.mvr) {
        uip_buf = mvr_uip_buf;
    } else {
        uip_buf = snp_uip_buf;
    }

    memset(dst_ports, 0x0, sizeof(dst_ports));
    local_port_cnt = ipmc_lib_get_system_local_port_cnt();
    for (i = 0; i < local_port_cnt; i++) {
        if (VTSS_PORT_BF_GET(entry->vlan_ports, i)) {
            VTSS_PORT_BF_SET(dst_ports, i, TRUE);
        }
    }
#if VTSS_SWITCH_STACKABLE
    if (!debug) {
        /* Filter Stacking Ports, since Distributed IPMC */
        i = PORT_NO_STACK_0;
        VTSS_PORT_BF_SET(dst_ports, i, FALSE);
        i = PORT_NO_STACK_1;
        VTSS_PORT_BF_SET(dst_ports, i, FALSE);
    }
#endif /* VTSS_SWITCH_STACKABLE */

    tx_flag = FALSE;
    for (i = 0; i < local_port_cnt; i++) {
        if (VTSS_PORT_BF_GET(dst_ports, i)) {
            tx_flag = TRUE;
            memset((u8 *)&uip_buf[0], 0x0, IPMC_LIB_PKT_BUF_SZ);
            break;
        }
    }

    T_D("ipmc_lib_packet_tx_gq");
    if (tx_flag) {
        T_D("Transmitting IPMCVer-%d GQ to port(s)", entry->ipmc_version);

        eth_hdr = (ipmc_ip_eth_hdr *)&uip_buf[0];
        if (entry->ipmc_version == IPMC_IP_VERSION_MLD) {
            vtss_ipv6_t ipLinkLocalSrc;

            /* ETHERNET */
            memcpy(eth_hdr->dest.addr, mld_all_node_mac, sizeof(uchar) * 6);
            if (!ipmc_lib_get_system_mgmt_macx(mac_addr)) {
                memset(eth_hdr->src.addr, 0x0, sizeof(uchar) * 6);
            } else {
                memcpy(eth_hdr->src.addr, mac_addr, sizeof(uchar) * 6);
            }
            eth_hdr->type = htons(IP_MULTICAST_V6_ETHER_TYPE);

            /* Depend on Compatibility */
            if ((entry->param.hst_compatibility.mode == VTSS_IPMC_COMPAT_MODE_GEN) ||
                (entry->param.rtr_compatibility.mode == VTSS_IPMC_COMPAT_MODE_GEN)) {
                ip_mld_gen_hdr = (ipmc_ip_mld_hdr *)&uip_buf[sizeof(ipmc_ip_eth_hdr)];

                /* IPv6 */
                ip_mld_gen_hdr->VerTcFl = htonl((IP_MULTICAST_V6_IP_VERSION << 28));
                ip_mld_gen_hdr->NxtHdr = MLD_IPV6_NEXTHDR_OPT_HBH;
                ip_mld_gen_hdr->HopLimit = IPMC_IPHDR_HOPLIMIT;
                /* get src address */
                if (ipmc_lib_get_eui64_linklocal_addr(&ipLinkLocalSrc)) {
                    /* link-local source ip */
                    memcpy(&ip_mld_gen_hdr->ip6_src, &ipLinkLocalSrc, sizeof(vtss_ipv6_t));
                } else {
                    return VTSS_RC_ERROR;
                }
                /* dst ip */
                if (ipmc_lib_isaddr6_all_zero(query_group_addr)) {
                    /* general query, dest ip = all-node */
                    ipmc_lib_get_all_node_ipv6_addr(&ip_mld_gen_hdr->ip6_dst);
                } else {
                    /* dest ip = group address */
                    memcpy(&ip_mld_gen_hdr->ip6_dst, query_group_addr, sizeof(vtss_ipv6_t));
                }

                /* Hop-By-Hop */
                ip_mld_gen_hdr->HBHNxtHdr = MLD_IPV6_NEXTHDR_ICMP;
                ip_mld_gen_hdr->HdrExtLen = 0x0;
                ip_mld_gen_hdr->OptNPad[0] = IPMC_IPV6_RTR_ALERT_PREFIX1;
                ip_mld_gen_hdr->OptNPad[1] = IPMC_IPV6_RTR_ALERT_PREFIX2;

                /* MLD */
                ip_mld_gen_hdr->type = IPMC_MLD_MSG_TYPE_QUERY;
                ip_mld_gen_hdr->code = 0x0;
                memcpy(&ip_mld_gen_hdr->group, query_group_addr, sizeof(vtss_ipv6_t));

                uip_len = sizeof(ipmc_ip_eth_hdr) + MLD_MIN_OFFSET + MLD_GEN_MIN_PAYLOAD_LEN;
                ip_mld_gen_hdr->PayloadLen = htons(uip_len - sizeof(ipmc_ip_eth_hdr) - IPV6_HDR_FIXED_LEN);

                ip_mld_gen_hdr->max_resp_time = htons(entry->param.querier.MaxResTime);
                ip_mld_gen_hdr->checksum = 0;
                ip_mld_gen_hdr->checksum = mld_chksum_tx((mld_ip6_hdr *)ip_mld_gen_hdr, MLD_MIN_OFFSET, (ulong)(ntohs(ip_mld_gen_hdr->PayloadLen) - MLD_MIN_HBH_LEN));
            } else {
                ip_mld_sfm_hdr = (ipmc_ip_mldv2_hdr *)&uip_buf[sizeof(ipmc_ip_eth_hdr)];

                /* IPv6 */
                ip_mld_sfm_hdr->VerTcFl = htonl((IP_MULTICAST_V6_IP_VERSION << 28));
                ip_mld_sfm_hdr->NxtHdr = MLD_IPV6_NEXTHDR_OPT_HBH;
                ip_mld_sfm_hdr->HopLimit = IPMC_IPHDR_HOPLIMIT;
                /* get src address */
                if (ipmc_lib_get_eui64_linklocal_addr(&ipLinkLocalSrc)) {
                    /* link-local source ip */
                    memcpy(&ip_mld_sfm_hdr->ip6_src, &ipLinkLocalSrc, sizeof(vtss_ipv6_t));
                }
                /* dst ip */
                if (ipmc_lib_isaddr6_all_zero(query_group_addr)) {
                    /* general query, dest ip = all-node */
                    ipmc_lib_get_all_node_ipv6_addr(&ip_mld_sfm_hdr->ip6_dst);
                } else {
                    /* dest ip = group address */
                    memcpy(&ip_mld_sfm_hdr->ip6_dst, query_group_addr, sizeof(vtss_ipv6_t));
                }

                /* Hop-By-Hop */
                ip_mld_sfm_hdr->HBHNxtHdr = MLD_IPV6_NEXTHDR_ICMP;
                ip_mld_sfm_hdr->HdrExtLen = 0x0;
                ip_mld_sfm_hdr->OptNPad[0] = IPMC_IPV6_RTR_ALERT_PREFIX1;
                ip_mld_sfm_hdr->OptNPad[1] = IPMC_IPV6_RTR_ALERT_PREFIX2;

                /* MLD */
                ip_mld_sfm_hdr->type = IPMC_MLD_MSG_TYPE_QUERY;
                ip_mld_sfm_hdr->code = 0x0;

                uip_len = sizeof(ipmc_ip_eth_hdr) + MLD_MIN_OFFSET + MLD_SFM_MIN_PAYLOAD_LEN;
                ip_mld_sfm_hdr->PayloadLen = htons(uip_len - sizeof(ipmc_ip_eth_hdr) - IPV6_HDR_FIXED_LEN);

                ip_mld_sfm_hdr->sfminfo.query.max_resp_time = htons(entry->param.querier.MaxResTime);
                memcpy(&ip_mld_sfm_hdr->sfminfo.query.group_address, query_group_addr, sizeof(vtss_ipv6_t));
                IPMC_PKT_DO_QRY_SUPP(ip_mld_sfm_hdr);
                if (IPMC_TIMER_RV(entry) < 0x7) {
                    ip_mld_sfm_hdr->sfminfo.query.resv_s_qrv |= IPMC_TIMER_RV(entry);
                }
                ip_mld_sfm_hdr->sfminfo.query.qqic = IPMC_TIMER_QI(entry) & 0xFF;

                ip_mld_sfm_hdr->checksum = 0;
                ip_mld_sfm_hdr->checksum = mld_chksum_tx((mld_ip6_hdr *)ip_mld_sfm_hdr, MLD_MIN_OFFSET, (ulong)(ntohs(ip_mld_sfm_hdr->PayloadLen) - MLD_MIN_HBH_LEN));
            }
        } else if (entry->ipmc_version == IPMC_IP_VERSION_IGMP) {
            vtss_ipv4_t ip4addr = 0;

            /* ETHERNET */
            memcpy(eth_hdr->dest.addr, igmp_all_node_mac, sizeof(uchar) * 6);
            if (!ipmc_lib_get_system_mgmt_macx(mac_addr)) {
                memset(eth_hdr->src.addr, 0x0, sizeof(uchar) * 6);
            } else {
                memcpy(eth_hdr->src.addr, mac_addr, sizeof(uchar) * 6);
            }
            eth_hdr->type = htons(IP_MULTICAST_V4_ETHER_TYPE);

            /* Depend on Compatibility */
            if ((entry->param.hst_compatibility.mode == VTSS_IPMC_COMPAT_MODE_OLD) ||
                (entry->param.rtr_compatibility.mode == VTSS_IPMC_COMPAT_MODE_OLD) ||
                (entry->param.hst_compatibility.mode == VTSS_IPMC_COMPAT_MODE_GEN) ||
                (entry->param.rtr_compatibility.mode == VTSS_IPMC_COMPAT_MODE_GEN)) {
                ip_igmp_gen_hdr = (ipmc_ip_igmp_hdr *)&uip_buf[sizeof(ipmc_ip_eth_hdr)];

                /* IPv4 */
                ip_igmp_gen_hdr->vhl = (IP_MULTICAST_V4_IP_VERSION << 4);
                ip_igmp_gen_hdr->vhl |= sizeof(igmp_ip4_hdr) / 4;
                ip_igmp_gen_hdr->tos = 0;
                ip_igmp_gen_hdr->seq_id = ip_igmp_gen_hdr->offset = 0;
                ip_igmp_gen_hdr->ttl = IPMC_IPHDR_HOPLIMIT;
                ip_igmp_gen_hdr->proto = IP_MULTICAST_IGMP_PROTO_ID;
                ip_igmp_gen_hdr->router_option[0] = IPMC_IPV4_RTR_ALERT_PREFIX1;
                ip_igmp_gen_hdr->router_option[1] = IPMC_IPV4_RTR_ALERT_PREFIX2;

                /* get src address */
                if (ipmc_lib_get_ipintf_igmp_adrs(entry, &ip4addr)) {
                    ip4addr = htonl(ip4addr);
                } else {
                    return VTSS_RC_ERROR;
                }
                memcpy(&ip_igmp_gen_hdr->ip4_src, &ip4addr, sizeof(ipmcv4addr));
                /* dst ip */
                if (ipmc_lib_isaddr4_all_zero((ipmcv4addr *)&query_group_addr->addr[12])) {
                    /* general query, dest ip = 224.0.0.1 */
                    ipmc_lib_get_all_node_ipv4_addr(&ip_igmp_gen_hdr->ip4_dst);
                } else {
                    /* dest ip = group address */
                    memcpy(&ip_igmp_gen_hdr->ip4_dst, &query_group_addr->addr[12], sizeof(ipmcv4addr));
                }

                /* IGMP */
                ip_igmp_gen_hdr->type = IPMC_IGMP_MSG_TYPE_QUERY;
                memcpy(&ip_igmp_gen_hdr->group, &query_group_addr->addr[12], sizeof(ipmcv4addr));
                if ((entry->param.hst_compatibility.mode == VTSS_IPMC_COMPAT_MODE_OLD) ||
                    (entry->param.rtr_compatibility.mode == VTSS_IPMC_COMPAT_MODE_OLD)) {
                    ip_igmp_gen_hdr->max_resp_time = 0x0;
                } else {
                    ip_igmp_gen_hdr->max_resp_time = (entry->param.querier.MaxResTime) & 0xFF;
                }
                uip_len = sizeof(ipmc_ip_eth_hdr) + sizeof(igmp_ip4_hdr) + IGMP_MIN_PAYLOAD_LEN;

                /* IGMP CheckSum First */
                ip_igmp_gen_hdr->checksum = 0;
                ip_igmp_gen_hdr->checksum = igmp_chksum_tx((ushort *)&ip_igmp_gen_hdr->type, IGMP_MIN_PAYLOAD_LEN);
                /* IPv4 CheckSum Later */
                ip_igmp_gen_hdr->PayloadLen = htons(uip_len - sizeof(ipmc_ip_eth_hdr));
                ip_igmp_gen_hdr->ip_chksum = 0;
                ip_igmp_gen_hdr->ip_chksum = igmp_chksum_tx((ushort *)ip_igmp_gen_hdr, sizeof(igmp_ip4_hdr));
            } else {
                ip_igmp_sfm_hdr = (ipmc_ip_igmpv3_hdr *)&uip_buf[sizeof(ipmc_ip_eth_hdr)];

                /* IPv4 */
                ip_igmp_sfm_hdr->vhl = (IP_MULTICAST_V4_IP_VERSION << 4);
                ip_igmp_sfm_hdr->vhl |= sizeof(igmp_ip4_hdr) / 4;
                ip_igmp_sfm_hdr->tos = 0;
                ip_igmp_sfm_hdr->seq_id = ip_igmp_sfm_hdr->offset = 0;
                ip_igmp_sfm_hdr->ttl = IPMC_IPHDR_HOPLIMIT;
                ip_igmp_sfm_hdr->proto = IP_MULTICAST_IGMP_PROTO_ID;
                ip_igmp_sfm_hdr->router_option[0] = IPMC_IPV4_RTR_ALERT_PREFIX1;
                ip_igmp_sfm_hdr->router_option[1] = IPMC_IPV4_RTR_ALERT_PREFIX2;

                /* get src address */
                if (ipmc_lib_get_ipintf_igmp_adrs(entry, &ip4addr)) {
                    ip4addr = htonl(ip4addr);
                } else {
                    return VTSS_RC_ERROR;
                }
                memcpy(&ip_igmp_sfm_hdr->ip4_src, &ip4addr, sizeof(ipmcv4addr));
                /* dst ip */
                if (ipmc_lib_isaddr4_all_zero((ipmcv4addr *)&query_group_addr->addr[12])) {
                    /* general query, dest ip = 224.0.0.1 */
                    ipmc_lib_get_all_node_ipv4_addr(&ip_igmp_sfm_hdr->ip4_dst);
                } else {
                    /* dest ip = group address */
                    memcpy(&ip_igmp_sfm_hdr->ip4_dst, &query_group_addr->addr[12], sizeof(ipmcv4addr));
                }

                /* IGMP */
                ip_igmp_sfm_hdr->type = IPMC_IGMP_MSG_TYPE_QUERY;
                ip_igmp_sfm_hdr->code = (entry->param.querier.MaxResTime) & 0xFF;

                memcpy(&ip_igmp_sfm_hdr->sfminfo.query.group_address, &query_group_addr->addr[12], sizeof(ipmcv4addr));
                IPMC_PKT_DO_QRY_SUPP(ip_igmp_sfm_hdr);
                if (IPMC_TIMER_RV(entry) < 0x7) {
                    ip_igmp_sfm_hdr->sfminfo.query.resv_s_qrv |= IPMC_TIMER_RV(entry);
                }
                ip_igmp_sfm_hdr->sfminfo.query.qqic = IPMC_TIMER_QI(entry) & 0xFF;

                uip_len = sizeof(ipmc_ip_eth_hdr) + sizeof(igmp_ip4_hdr) + IGMP_SFM_MIN_PAYLOAD_LEN;

                /* IGMP CheckSum First */
                ip_igmp_sfm_hdr->checksum = 0;
                ip_igmp_sfm_hdr->checksum = igmp_chksum_tx((ushort *)&ip_igmp_sfm_hdr->type, IGMP_SFM_MIN_PAYLOAD_LEN);
                /* IPv4 CheckSum Later */
                ip_igmp_sfm_hdr->PayloadLen = htons(uip_len - sizeof(ipmc_ip_eth_hdr));
                ip_igmp_sfm_hdr->ip_chksum = 0;
                ip_igmp_sfm_hdr->ip_chksum = igmp_chksum_tx((ushort *)ip_igmp_sfm_hdr, sizeof(igmp_ip4_hdr));
            }
        } else {
            return VTSS_RC_ERROR;
        } /* ipmc_version */

        T_D("\n\rIPMC transmit GQ");
        T_D_HEX(&uip_buf[0], uip_len);

        memset(&dst_port_mask, 0x0, sizeof(ipmc_port_bfs_t));
        for (i = 0; i < local_port_cnt; i++) {
            if (VTSS_PORT_BF_GET(dst_ports, i)) {
                VTSS_PORT_BF_SET(dst_port_mask.member_ports, i, TRUE);
            }
        }

        if (ipmc_lib_packet_tx(&dst_port_mask,
                               force_untag,
                               FALSE,
                               VTSS_PORT_NO_NONE,
                               (entry->param.mvr ? IPMC_PKT_SRC_MVR : IPMC_PKT_SRC_SNP),
                               entry->param.vid,
                               0,
                               (entry->param.priority & IPMC_PARAM_PRIORITY_MASK),
                               VTSS_GLAG_NO_NONE,
                               &uip_buf[0],
                               uip_len) != VTSS_OK) {
            T_D("Failure in ipmc_lib_packet_tx_gq for VLAN %d", entry->param.vid);
        } else {
            entry->param.querier.ipmc_queries_sent++;
        }
    } /* tx_flag */

    return VTSS_OK;
}

vtss_rc ipmc_lib_packet_tx_sq(ipmc_db_ctrl_hdr_t *fltr,
                              ipmc_send_act_t snd_act,
                              ipmc_group_entry_t *grp,
                              ipmc_intf_entry_t *entry,
                              u8 src_port,
                              BOOL force_untag)
{
    u8                  *uip_buf;
    size_t              uip_len;
    ipmc_ip_eth_hdr     *eth_hdr;
    ipmc_ip_mld_hdr     *ip_mld_gen_hdr = NULL;
    ipmc_ip_mldv2_hdr   *ip_mld_sfm_hdr = NULL;
    ipmc_ip_igmp_hdr    *ip_igmp_gen_hdr = NULL;
    ipmc_ip_igmpv3_hdr  *ip_igmp_sfm_hdr = NULL;
    uchar               dst_ports[VTSS_PORT_BF_SIZE] = {0};
    uchar               mac_addr[6] = {0};
    u32                 i, local_port_cnt;
    ipmc_port_bfs_t     dst_port_mask;
    BOOL                tx_flag, tx_untag;

    if (!ipmc_lib_pkt_done_init) {
        return VTSS_RC_ERROR;
    }

    if (!grp || !entry) {
        return VTSS_RC_ERROR;
    }

    IPMC_LIB_CHK_LISTENER_SET(grp, src_port);
    if (snd_act != IPMC_SND_GO) {
        return VTSS_OK;
    }

    tx_untag = force_untag;
    if (entry->param.mvr && (entry->param.vtag == IPMC_INTF_UNTAG)) {
        tx_untag = TRUE;
    }

    if (entry->param.mvr) {
        uip_buf = mvr_uip_buf;
    } else {
        uip_buf = snp_uip_buf;
    }

    VTSS_PORT_BF_SET(dst_ports, src_port, TRUE);
#if VTSS_SWITCH_STACKABLE
    /* Filter Stacking Ports, since Distributed IPMC */
    i = PORT_NO_STACK_0;
    VTSS_PORT_BF_SET(dst_ports, i, FALSE);
    i = PORT_NO_STACK_1;
    VTSS_PORT_BF_SET(dst_ports, i, FALSE);
#endif /* VTSS_SWITCH_STACKABLE */

    tx_flag = FALSE;
    local_port_cnt = ipmc_lib_get_system_local_port_cnt();
    for (i = 0; i < local_port_cnt; i++) {
        if (VTSS_PORT_BF_GET(dst_ports, i)) {
            tx_flag = TRUE;
            memset((u8 *)&uip_buf[0], 0x0, IPMC_LIB_PKT_BUF_SZ);
            break;
        }
    }

    T_D("ipmc_lib_packet_tx_sq");
    if (tx_flag) {
        T_D("  Transmitting MASQ to port(%u)", src_port);

        eth_hdr = (ipmc_ip_eth_hdr *)&uip_buf[0];
        if (entry->ipmc_version == IPMC_IP_VERSION_MLD) {
            vtss_ipv6_t ipLinkLocalSrc;

            /* ETHERNET */
            if (ipmc_lib_isaddr6_all_zero(&grp->group_addr)) {
                memcpy(eth_hdr->dest.addr, mld_all_node_mac, sizeof(ipmc_eth_addr));
            } else {
                ipmc_lib_packet_dst_mac(eth_hdr->dest.addr, &grp->group_addr, entry->ipmc_version);
            }

            if (!ipmc_lib_get_system_mgmt_macx(mac_addr)) {
                memset(eth_hdr->src.addr, 0x0, sizeof(uchar) * 6);
            } else {
                memcpy(eth_hdr->src.addr, mac_addr, sizeof(uchar) * 6);
            }
            eth_hdr->type = htons(IP_MULTICAST_V6_ETHER_TYPE);

            /* Depend on Compatibility */
            if ((entry->param.hst_compatibility.mode == VTSS_IPMC_COMPAT_MODE_GEN) ||
                (entry->param.rtr_compatibility.mode == VTSS_IPMC_COMPAT_MODE_GEN)) {
                ip_mld_gen_hdr = (ipmc_ip_mld_hdr *)&uip_buf[sizeof(ipmc_ip_eth_hdr)];

                /* IPv6 */
                ip_mld_gen_hdr->VerTcFl = htonl((IP_MULTICAST_V6_IP_VERSION << 28));
                ip_mld_gen_hdr->NxtHdr = MLD_IPV6_NEXTHDR_OPT_HBH;
                ip_mld_gen_hdr->HopLimit = IPMC_IPHDR_HOPLIMIT;
                /* get src address */
                if (ipmc_lib_get_eui64_linklocal_addr(&ipLinkLocalSrc)) {
                    /* link-local source ip */
                    memcpy(&ip_mld_gen_hdr->ip6_src, &ipLinkLocalSrc, sizeof(vtss_ipv6_t));
                }
                /* dst ip */
                if (ipmc_lib_isaddr6_all_zero(&grp->group_addr)) {
                    /* general query, dest ip = all-node */
                    ipmc_lib_get_all_node_ipv6_addr(&ip_mld_gen_hdr->ip6_dst);
                } else {
                    /* dest ip = group address */
                    memcpy(&ip_mld_gen_hdr->ip6_dst, &grp->group_addr, sizeof(vtss_ipv6_t));
                }

                /* Hop-By-Hop */
                ip_mld_gen_hdr->HBHNxtHdr = MLD_IPV6_NEXTHDR_ICMP;
                ip_mld_gen_hdr->HdrExtLen = 0x0;
                ip_mld_gen_hdr->OptNPad[0] = IPMC_IPV6_RTR_ALERT_PREFIX1;
                ip_mld_gen_hdr->OptNPad[1] = IPMC_IPV6_RTR_ALERT_PREFIX2;

                /* MLDv1 */
                ip_mld_gen_hdr->type = IPMC_MLD_MSG_TYPE_QUERY;
                ip_mld_gen_hdr->code = 0x0;
                memcpy(&ip_mld_gen_hdr->group, &grp->group_addr, sizeof(vtss_ipv6_t));

                uip_len = sizeof(ipmc_ip_eth_hdr) + MLD_MIN_OFFSET + MLD_GEN_MIN_PAYLOAD_LEN;
                ip_mld_gen_hdr->PayloadLen = htons(uip_len - sizeof(ipmc_ip_eth_hdr) - IPV6_HDR_FIXED_LEN);

                if (ipmc_lib_isaddr6_all_zero(&grp->group_addr)) {
                    ip_mld_gen_hdr->max_resp_time = htons(entry->param.querier.MaxResTime);
                } else {
                    ip_mld_gen_hdr->max_resp_time = htons(entry->param.querier.LastQryItv);
                }
                ip_mld_gen_hdr->checksum = 0;
                ip_mld_gen_hdr->checksum = mld_chksum_tx((mld_ip6_hdr *)ip_mld_gen_hdr, MLD_MIN_OFFSET, (ulong)(ntohs(ip_mld_gen_hdr->PayloadLen) - MLD_MIN_HBH_LEN));
            } else {
                ip_mld_sfm_hdr = (ipmc_ip_mldv2_hdr *)&uip_buf[sizeof(ipmc_ip_eth_hdr)];

                /* IPv6 */
                ip_mld_sfm_hdr->VerTcFl = htonl((IP_MULTICAST_V6_IP_VERSION << 28));
                ip_mld_sfm_hdr->NxtHdr = MLD_IPV6_NEXTHDR_OPT_HBH;
                ip_mld_sfm_hdr->HopLimit = IPMC_IPHDR_HOPLIMIT;
                /* get src address */
                if (ipmc_lib_get_eui64_linklocal_addr(&ipLinkLocalSrc)) {
                    /* link-local source ip */
                    memcpy(&ip_mld_sfm_hdr->ip6_src, &ipLinkLocalSrc, sizeof(vtss_ipv6_t));
                }
                /* dst ip */
                if (ipmc_lib_isaddr6_all_zero(&grp->group_addr)) {
                    /* general query, dest ip = all-node */
                    ipmc_lib_get_all_node_ipv6_addr(&ip_mld_sfm_hdr->ip6_dst);
                } else {
                    /* dest ip = group address */
                    memcpy(&ip_mld_sfm_hdr->ip6_dst, &grp->group_addr, sizeof(vtss_ipv6_t));
                }

                /* Hop-By-Hop */
                ip_mld_sfm_hdr->HBHNxtHdr = MLD_IPV6_NEXTHDR_ICMP;
                ip_mld_sfm_hdr->HdrExtLen = 0x0;
                ip_mld_sfm_hdr->OptNPad[0] = IPMC_IPV6_RTR_ALERT_PREFIX1;
                ip_mld_sfm_hdr->OptNPad[1] = IPMC_IPV6_RTR_ALERT_PREFIX2;

                /* MLD */
                ip_mld_sfm_hdr->type = IPMC_MLD_MSG_TYPE_QUERY;
                ip_mld_sfm_hdr->code = 0x0;

                uip_len = sizeof(ipmc_ip_eth_hdr) + MLD_MIN_OFFSET + MLD_SFM_MIN_PAYLOAD_LEN;
                ip_mld_sfm_hdr->PayloadLen = htons(uip_len - sizeof(ipmc_ip_eth_hdr) - IPV6_HDR_FIXED_LEN);

                if (ipmc_lib_isaddr6_all_zero(&grp->group_addr)) {
                    ip_mld_sfm_hdr->sfminfo.query.max_resp_time = htons(entry->param.querier.MaxResTime);
                } else {
                    ip_mld_sfm_hdr->sfminfo.query.max_resp_time = htons(entry->param.querier.LastQryItv);
                }
                memcpy(&ip_mld_sfm_hdr->sfminfo.query.group_address, &grp->group_addr, sizeof(vtss_ipv6_t));
                IPMC_PKT_DO_QRY_SUPP(ip_mld_sfm_hdr);
                if (IPMC_TIMER_RV(entry) < 0x7) {
                    ip_mld_sfm_hdr->sfminfo.query.resv_s_qrv |= IPMC_TIMER_RV(entry);
                }
                ip_mld_sfm_hdr->sfminfo.query.qqic = IPMC_TIMER_QI(entry) & 0xFF;

                ip_mld_sfm_hdr->checksum = 0;
                ip_mld_sfm_hdr->checksum = mld_chksum_tx((mld_ip6_hdr *)ip_mld_sfm_hdr, MLD_MIN_OFFSET, (ulong)(ntohs(ip_mld_sfm_hdr->PayloadLen) - MLD_MIN_HBH_LEN));
            }
        } else if (entry->ipmc_version == IPMC_IP_VERSION_IGMP) {
            vtss_ipv4_t ip4addr = 0;

            /* ETHERNET */
            if (ipmc_lib_isaddr4_all_zero((ipmcv4addr *)&grp->group_addr.addr[12])) {
                memcpy(eth_hdr->dest.addr, igmp_all_node_mac, sizeof(ipmc_eth_addr));
            } else {
                ipmc_lib_packet_dst_mac(eth_hdr->dest.addr, &grp->group_addr, entry->ipmc_version);
            }

            if (!ipmc_lib_get_system_mgmt_macx(mac_addr)) {
                memset(eth_hdr->src.addr, 0x0, sizeof(uchar) * 6);
            } else {
                memcpy(eth_hdr->src.addr, mac_addr, sizeof(uchar) * 6);
            }
            eth_hdr->type = htons(IP_MULTICAST_V4_ETHER_TYPE);

            /* Depend on Compatibility */
            if ((entry->param.hst_compatibility.mode == VTSS_IPMC_COMPAT_MODE_OLD) ||
                (entry->param.rtr_compatibility.mode == VTSS_IPMC_COMPAT_MODE_OLD) ||
                (entry->param.hst_compatibility.mode == VTSS_IPMC_COMPAT_MODE_GEN) ||
                (entry->param.rtr_compatibility.mode == VTSS_IPMC_COMPAT_MODE_GEN)) {
                ip_igmp_gen_hdr = (ipmc_ip_igmp_hdr *)&uip_buf[sizeof(ipmc_ip_eth_hdr)];

                /* IPv4 */
                ip_igmp_gen_hdr->vhl = (IP_MULTICAST_V4_IP_VERSION << 4);
                ip_igmp_gen_hdr->vhl |= sizeof(igmp_ip4_hdr) / 4;
                ip_igmp_gen_hdr->tos = 0;
                ip_igmp_gen_hdr->seq_id = ip_igmp_gen_hdr->offset = 0;
                ip_igmp_gen_hdr->ttl = IPMC_IPHDR_HOPLIMIT;
                ip_igmp_gen_hdr->proto = IP_MULTICAST_IGMP_PROTO_ID;
                ip_igmp_gen_hdr->router_option[0] = IPMC_IPV4_RTR_ALERT_PREFIX1;
                ip_igmp_gen_hdr->router_option[1] = IPMC_IPV4_RTR_ALERT_PREFIX2;

                /* get src address */
                if (ipmc_lib_get_ipintf_igmp_adrs(entry, &ip4addr)) {
                    ip4addr = htonl(ip4addr);
                }
                memcpy(&ip_igmp_gen_hdr->ip4_src, &ip4addr, sizeof(ipmcv4addr));
                /* dst ip */
                if (ipmc_lib_isaddr4_all_zero((ipmcv4addr *)&grp->group_addr.addr[12])) {
                    /* general query, dest ip = 224.0.0.1 */
                    ipmc_lib_get_all_node_ipv4_addr(&ip_igmp_gen_hdr->ip4_dst);
                } else {
                    /* dest ip = group address */
                    memcpy(&ip_igmp_gen_hdr->ip4_dst, &grp->group_addr.addr[12], sizeof(ipmcv4addr));
                }

                /* IGMP */
                ip_igmp_gen_hdr->type = IPMC_IGMP_MSG_TYPE_QUERY;
                memcpy(&ip_igmp_gen_hdr->group, &grp->group_addr.addr[12], sizeof(ipmcv4addr));
                if ((entry->param.hst_compatibility.mode == VTSS_IPMC_COMPAT_MODE_OLD) ||
                    (entry->param.rtr_compatibility.mode == VTSS_IPMC_COMPAT_MODE_OLD)) {
                    ip_igmp_gen_hdr->max_resp_time = 0x0;
                } else {
                    if (ipmc_lib_isaddr4_all_zero((ipmcv4addr *)&grp->group_addr.addr[12])) {
                        ip_igmp_gen_hdr->max_resp_time = (entry->param.querier.MaxResTime) & 0xFF;
                    } else {
                        ip_igmp_gen_hdr->max_resp_time = (entry->param.querier.LastQryItv) & 0xFF;
                    }
                }
                uip_len = sizeof(ipmc_ip_eth_hdr) + sizeof(igmp_ip4_hdr) + IGMP_MIN_PAYLOAD_LEN;

                /* IGMP CheckSum First */
                ip_igmp_gen_hdr->checksum = 0;
                ip_igmp_gen_hdr->checksum = igmp_chksum_tx((ushort *)&ip_igmp_gen_hdr->type, IGMP_MIN_PAYLOAD_LEN);
                /* IPv4 CheckSum Later */
                ip_igmp_gen_hdr->PayloadLen = htons(uip_len - sizeof(ipmc_ip_eth_hdr));
                ip_igmp_gen_hdr->ip_chksum = 0;
                ip_igmp_gen_hdr->ip_chksum = igmp_chksum_tx((ushort *)ip_igmp_gen_hdr, sizeof(igmp_ip4_hdr));
            } else {
                ip_igmp_sfm_hdr = (ipmc_ip_igmpv3_hdr *)&uip_buf[sizeof(ipmc_ip_eth_hdr)];

                /* IPv4 */
                ip_igmp_sfm_hdr->vhl = (IP_MULTICAST_V4_IP_VERSION << 4);
                ip_igmp_sfm_hdr->vhl |= sizeof(igmp_ip4_hdr) / 4;
                ip_igmp_sfm_hdr->tos = 0;
                ip_igmp_sfm_hdr->seq_id = ip_igmp_sfm_hdr->offset = 0;
                ip_igmp_sfm_hdr->ttl = IPMC_IPHDR_HOPLIMIT;
                ip_igmp_sfm_hdr->proto = IP_MULTICAST_IGMP_PROTO_ID;
                ip_igmp_sfm_hdr->router_option[0] = IPMC_IPV4_RTR_ALERT_PREFIX1;
                ip_igmp_sfm_hdr->router_option[1] = IPMC_IPV4_RTR_ALERT_PREFIX2;

                /* get src address */
                if (ipmc_lib_get_ipintf_igmp_adrs(entry, &ip4addr)) {
                    ip4addr = htonl(ip4addr);
                }
                memcpy(&ip_igmp_sfm_hdr->ip4_src, &ip4addr, sizeof(ipmcv4addr));
                /* dst ip */
                if (ipmc_lib_isaddr4_all_zero((ipmcv4addr *)&grp->group_addr.addr[12])) {
                    /* general query, dest ip = 224.0.0.1 */
                    ipmc_lib_get_all_node_ipv4_addr(&ip_igmp_sfm_hdr->ip4_dst);
                } else {
                    /* dest ip = group address */
                    memcpy(&ip_igmp_sfm_hdr->ip4_dst, &grp->group_addr.addr[12], sizeof(ipmcv4addr));
                }

                /* IGMP */
                ip_igmp_sfm_hdr->type = IPMC_IGMP_MSG_TYPE_QUERY;
                if (ipmc_lib_isaddr4_all_zero((ipmcv4addr *)&grp->group_addr.addr[12])) {
                    ip_igmp_sfm_hdr->code = (entry->param.querier.MaxResTime) & 0xFF;
                } else {
                    ip_igmp_sfm_hdr->code = (entry->param.querier.LastQryItv) & 0xFF;
                }

                memcpy(&ip_igmp_sfm_hdr->sfminfo.query.group_address, &grp->group_addr.addr[12], sizeof(ipmcv4addr));
                IPMC_PKT_DO_QRY_SUPP(ip_igmp_sfm_hdr);
                if (IPMC_TIMER_RV(entry) < 0x7) {
                    ip_igmp_sfm_hdr->sfminfo.query.resv_s_qrv |= IPMC_TIMER_RV(entry);
                }
                ip_igmp_sfm_hdr->sfminfo.query.qqic = IPMC_TIMER_QI(entry) & 0xFF;

                uip_len = sizeof(ipmc_ip_eth_hdr) + sizeof(igmp_ip4_hdr) + IGMP_SFM_MIN_PAYLOAD_LEN;

                /* IGMP CheckSum First */
                ip_igmp_sfm_hdr->checksum = 0;
                ip_igmp_sfm_hdr->checksum = igmp_chksum_tx((ushort *)&ip_igmp_sfm_hdr->type, IGMP_SFM_MIN_PAYLOAD_LEN);
                /* IPv4 CheckSum Later */
                ip_igmp_sfm_hdr->PayloadLen = htons(uip_len - sizeof(ipmc_ip_eth_hdr));
                ip_igmp_sfm_hdr->ip_chksum = 0;
                ip_igmp_sfm_hdr->ip_chksum = igmp_chksum_tx((ushort *)ip_igmp_sfm_hdr, sizeof(igmp_ip4_hdr));
            }
        } else {
            return VTSS_RC_ERROR;
        } /* ipmc_version */

        T_D("\n\rIPMC transmit MASQ");
        T_D_HEX(&uip_buf[0], uip_len);

        VTSS_PORT_BF_CLR(dst_port_mask.member_ports);
        local_port_cnt = ipmc_lib_get_system_local_port_cnt();
        for (i = 0; i < local_port_cnt; i++) {
            VTSS_PORT_BF_SET(dst_port_mask.member_ports, i, VTSS_PORT_BF_GET(dst_ports, i));
        }

        if (ipmc_lib_packet_tx(&dst_port_mask,
                               tx_untag,
                               FALSE,
                               VTSS_PORT_NO_NONE,
                               (entry->param.mvr ? IPMC_PKT_SRC_MVR : IPMC_PKT_SRC_SNP),
                               entry->param.vid,
                               0,
                               (entry->param.priority & IPMC_PARAM_PRIORITY_MASK),
                               VTSS_GLAG_NO_NONE,
                               &uip_buf[0],
                               uip_len) != VTSS_OK) {
            T_D("Failure in ipmc_lib_packet_tx_sq for VLAN %d", entry->param.vid);
        } else {
            entry->param.querier.group_queries_sent++;
            entry->param.querier.ipmc_queries_sent++;
        }
    } /* tx_flag */

    (void) ipmc_lib_protocol_lower_filter_timer(fltr, grp, entry, src_port);

    return VTSS_OK;
}

vtss_rc ipmc_lib_packet_tx_ssq(ipmc_db_ctrl_hdr_t *fltr,
                               ipmc_send_act_t snd_act,
                               ipmc_group_entry_t *grp,
                               ipmc_intf_entry_t *entry,
                               u8 src_port,
                               ipmc_db_ctrl_hdr_t *src_list,
                               BOOL force_untag)
{
    u8                  *uip_buf;
    size_t              uip_len;
    ipmc_ip_eth_hdr     *eth_hdr;
    ipmc_ip_mldv2_hdr   *ip_mld_hdr;
    ipmc_ip_igmpv3_hdr  *ip_igmp_hdr;
    u8                  dst_ports[VTSS_PORT_BF_SIZE];
    u8                  mac_addr[6];
    ipmc_port_bfs_t     dst_port_mask;
    ipmc_sfm_srclist_t  *sfm_src, *sfm_src_bak;
    u32                 i, local_port_cnt, src_entry_cnt, extraLen;
    BOOL                tx_flag, tx_untag;

    if (!ipmc_lib_pkt_done_init) {
        return VTSS_RC_ERROR;
    }

    if (!grp || !entry || !src_list) {
        return VTSS_RC_ERROR;
    }

    tx_untag = force_untag;
    if (entry->param.mvr && (entry->param.vtag == IPMC_INTF_UNTAG)) {
        tx_untag = TRUE;
    }

    if ((entry->param.hst_compatibility.mode == VTSS_IPMC_COMPAT_MODE_OLD) ||
        (entry->param.rtr_compatibility.mode == VTSS_IPMC_COMPAT_MODE_OLD) ||
        (entry->param.hst_compatibility.mode == VTSS_IPMC_COMPAT_MODE_GEN) ||
        (entry->param.rtr_compatibility.mode == VTSS_IPMC_COMPAT_MODE_GEN)) {
        return ipmc_lib_packet_tx_sq(fltr, snd_act, grp, entry, src_port, tx_untag);
    }

    IPMC_LIB_CHK_LISTENER_SET(grp, src_port);
    if (snd_act != IPMC_SND_GO) {
        return VTSS_OK;
    }

    if (IPMC_LIB_DB_GET_COUNT(src_list) == 0) {
        /* RFC-3810 7.6.3 */
        return VTSS_OK;
    }

    if (!IPMC_MEM_SYSTEM_MTAKE(sfm_src, sizeof(ipmc_sfm_srclist_t))) {
        return VTSS_RC_ERROR;
    }
    sfm_src_bak = sfm_src;

    ip_mld_hdr = NULL;
    ip_igmp_hdr = NULL;
    src_entry_cnt = extraLen = 0;

    memset(dst_ports, 0x0, sizeof(dst_ports));
    VTSS_PORT_BF_SET(dst_ports, src_port, TRUE);
#if VTSS_SWITCH_STACKABLE
    /* Filter Stacking Ports, since Distributed IPMC */
    i = PORT_NO_STACK_0;
    VTSS_PORT_BF_SET(dst_ports, i, FALSE);
    i = PORT_NO_STACK_1;
    VTSS_PORT_BF_SET(dst_ports, i, FALSE);
#endif /* VTSS_SWITCH_STACKABLE */

    if (entry->param.mvr) {
        uip_buf = mvr_uip_buf;
    } else {
        uip_buf = snp_uip_buf;
    }

    tx_flag = FALSE;
    local_port_cnt = ipmc_lib_get_system_local_port_cnt();
    for (i = 0; i < local_port_cnt; i++) {
        if (VTSS_PORT_BF_GET(dst_ports, i)) {
            tx_flag = TRUE;
            memset((u8 *)&uip_buf[0], 0x0, IPMC_LIB_PKT_BUF_SZ);
            break;
        }
    }

    T_D("ipmc_lib_packet_tx_ssq");
    if (tx_flag) {
        T_D("  Transmitting MASSQ to port(%u)", src_port);

        eth_hdr = (ipmc_ip_eth_hdr *)&uip_buf[0];
        if (entry->ipmc_version == IPMC_IP_VERSION_MLD) {
            vtss_ipv6_t ipLinkLocalSrc;

            /* ETHERNET */
            if (ipmc_lib_isaddr6_all_zero(&grp->group_addr)) {
                memcpy(eth_hdr->dest.addr, mld_all_node_mac, sizeof(ipmc_eth_addr));
            } else {
                ipmc_lib_packet_dst_mac(eth_hdr->dest.addr, &grp->group_addr, entry->ipmc_version);
            }

            if (!ipmc_lib_get_system_mgmt_macx(mac_addr)) {
                memset(eth_hdr->src.addr, 0x0, sizeof(uchar) * 6);
            } else {
                memcpy(eth_hdr->src.addr, mac_addr, sizeof(uchar) * 6);
            }
            eth_hdr->type = htons(IP_MULTICAST_V6_ETHER_TYPE);

            ip_mld_hdr = (ipmc_ip_mldv2_hdr *)&uip_buf[sizeof(ipmc_ip_eth_hdr)];

            /* IPv6 */
            ip_mld_hdr->VerTcFl = htonl((IP_MULTICAST_V6_IP_VERSION << 28));
            ip_mld_hdr->NxtHdr = MLD_IPV6_NEXTHDR_OPT_HBH;
            ip_mld_hdr->HopLimit = IPMC_IPHDR_HOPLIMIT;
            /* get src address */
            if (ipmc_lib_get_eui64_linklocal_addr(&ipLinkLocalSrc)) {
                /* link-local source ip */
                memcpy(&ip_mld_hdr->ip6_src, &ipLinkLocalSrc, sizeof(vtss_ipv6_t));
            }
            /* dst ip */
            if (ipmc_lib_isaddr6_all_zero(&grp->group_addr)) {
                /* general query, dest ip = all-node */
                ipmc_lib_get_all_node_ipv6_addr(&ip_mld_hdr->ip6_dst);
            } else {
                /* dest ip = group address */
                memcpy(&ip_mld_hdr->ip6_dst, &grp->group_addr, sizeof(vtss_ipv6_t));
            }

            /* Hop-By-Hop */
            ip_mld_hdr->HBHNxtHdr = MLD_IPV6_NEXTHDR_ICMP;
            ip_mld_hdr->HdrExtLen = 0x0;
            ip_mld_hdr->OptNPad[0] = IPMC_IPV6_RTR_ALERT_PREFIX1;
            ip_mld_hdr->OptNPad[1] = IPMC_IPV6_RTR_ALERT_PREFIX2;

            sfm_src = sfm_src_bak;
            if (IPMC_LIB_DB_GET_FIRST(src_list, sfm_src)) {
                do {
                    if ((src_entry_cnt + 1) <= IPMC_NO_OF_PKT_SRCLIST) {
                        memcpy(&ip_mld_hdr->sfminfo.query.source_addr[src_entry_cnt], &sfm_src->src_ip, sizeof(vtss_ipv6_t));
                        src_entry_cnt++;
                    } else {
                        break;
                    }
                } while (IPMC_LIB_DB_GET_NEXT(src_list, sfm_src));

                extraLen = sizeof(vtss_ipv6_t) * src_entry_cnt;
            } else {
                extraLen = 0;
            }

            /* MLD */
            ip_mld_hdr->type = IPMC_MLD_MSG_TYPE_QUERY;
            ip_mld_hdr->code = 0x0;

            uip_len = sizeof(ipmc_ip_eth_hdr) + MLD_MIN_OFFSET + MLD_SFM_MIN_PAYLOAD_LEN + extraLen;
            ip_mld_hdr->PayloadLen = htons(uip_len - sizeof(ipmc_ip_eth_hdr) - IPV6_HDR_FIXED_LEN);
            if (ipmc_lib_isaddr6_all_zero(&grp->group_addr)) {
                ip_mld_hdr->sfminfo.query.max_resp_time = htons(entry->param.querier.MaxResTime);
            } else {
                ip_mld_hdr->sfminfo.query.max_resp_time = htons(entry->param.querier.LastQryItv);
            }

            memcpy(&ip_mld_hdr->sfminfo.query.group_address, &grp->group_addr, sizeof(vtss_ipv6_t));
            IPMC_PKT_DO_QRY_SUPP(ip_mld_hdr);
            if (IPMC_TIMER_RV(entry) < 0x7) {
                ip_mld_hdr->sfminfo.query.resv_s_qrv |= IPMC_TIMER_RV(entry);
            }
            ip_mld_hdr->sfminfo.query.qqic = IPMC_TIMER_QI(entry) & 0xFF;
            ip_mld_hdr->sfminfo.query.no_of_sources = htons(src_entry_cnt);

            ip_mld_hdr->checksum = 0;
            ip_mld_hdr->checksum = mld_chksum_tx((mld_ip6_hdr *)ip_mld_hdr, MLD_MIN_OFFSET, (ulong)(ntohs(ip_mld_hdr->PayloadLen) - MLD_MIN_HBH_LEN));
        } else if (entry->ipmc_version == IPMC_IP_VERSION_IGMP) {
            vtss_ipv4_t ip4addr = 0;

            /* ETHERNET */
            if (ipmc_lib_isaddr4_all_zero((ipmcv4addr *)&grp->group_addr.addr[12])) {
                memcpy(eth_hdr->dest.addr, igmp_all_node_mac, sizeof(ipmc_eth_addr));
            } else {
                ipmc_lib_packet_dst_mac(eth_hdr->dest.addr, &grp->group_addr, entry->ipmc_version);
            }

            if (!ipmc_lib_get_system_mgmt_macx(mac_addr)) {
                memset(eth_hdr->src.addr, 0x0, sizeof(uchar) * 6);
            } else {
                memcpy(eth_hdr->src.addr, mac_addr, sizeof(uchar) * 6);
            }
            eth_hdr->type = htons(IP_MULTICAST_V4_ETHER_TYPE);

            ip_igmp_hdr = (ipmc_ip_igmpv3_hdr *)&uip_buf[sizeof(ipmc_ip_eth_hdr)];

            /* IPv4 */
            ip_igmp_hdr->vhl = (IP_MULTICAST_V4_IP_VERSION << 4);
            ip_igmp_hdr->vhl |= sizeof(igmp_ip4_hdr) / 4;
            ip_igmp_hdr->tos = 0;
            ip_igmp_hdr->seq_id = ip_igmp_hdr->offset = 0;
            ip_igmp_hdr->ttl = IPMC_IPHDR_HOPLIMIT;
            ip_igmp_hdr->proto = IP_MULTICAST_IGMP_PROTO_ID;
            ip_igmp_hdr->router_option[0] = IPMC_IPV4_RTR_ALERT_PREFIX1;
            ip_igmp_hdr->router_option[1] = IPMC_IPV4_RTR_ALERT_PREFIX2;

            /* get src address */
            if (ipmc_lib_get_ipintf_igmp_adrs(entry, &ip4addr)) {
                ip4addr = htonl(ip4addr);
            }
            memcpy(&ip_igmp_hdr->ip4_src, &ip4addr, sizeof(ipmcv4addr));
            /* dst ip */
            if (ipmc_lib_isaddr4_all_zero((ipmcv4addr *)&grp->group_addr.addr[12])) {
                /* general query, dest ip = 224.0.0.1 */
                ipmc_lib_get_all_node_ipv4_addr(&ip_igmp_hdr->ip4_dst);
            } else {
                /* dest ip = group address */
                memcpy(&ip_igmp_hdr->ip4_dst, &grp->group_addr.addr[12], sizeof(ipmcv4addr));
            }

            sfm_src = sfm_src_bak;
            if (IPMC_LIB_DB_GET_FIRST(src_list, sfm_src)) {
                do {
                    if ((src_entry_cnt + 1) <= IPMC_NO_OF_PKT_SRCLIST) {
                        memcpy(&ip_igmp_hdr->sfminfo.query.source_addr[src_entry_cnt], &sfm_src->src_ip.addr[12], sizeof(ipmcv4addr));
                        src_entry_cnt++;
                    } else {
                        break;
                    }
                } while (IPMC_LIB_DB_GET_NEXT(src_list, sfm_src));

                extraLen = sizeof(ipmcv4addr) * src_entry_cnt;
            } else {
                extraLen = 0;
            }

            /* IGMP */
            ip_igmp_hdr->type = IPMC_IGMP_MSG_TYPE_QUERY;
            if (ipmc_lib_isaddr4_all_zero((ipmcv4addr *)&grp->group_addr.addr[12])) {
                ip_igmp_hdr->code = (entry->param.querier.MaxResTime) & 0xFF;
            } else {
                ip_igmp_hdr->code = (entry->param.querier.LastQryItv) & 0xFF;
            }

            memcpy(&ip_igmp_hdr->sfminfo.query.group_address, &grp->group_addr.addr[12], sizeof(ipmcv4addr));
            IPMC_PKT_DO_QRY_SUPP(ip_igmp_hdr);
            if (IPMC_TIMER_RV(entry) < 0x7) {
                ip_igmp_hdr->sfminfo.query.resv_s_qrv |= IPMC_TIMER_RV(entry);
            }
            ip_igmp_hdr->sfminfo.query.qqic = IPMC_TIMER_QI(entry) & 0xFF;
            ip_igmp_hdr->sfminfo.query.no_of_sources = htons(src_entry_cnt);

            uip_len = sizeof(ipmc_ip_eth_hdr) + sizeof(igmp_ip4_hdr) + IGMP_SFM_MIN_PAYLOAD_LEN + extraLen;

            /* IGMP CheckSum First */
            ip_igmp_hdr->checksum = 0;
            ip_igmp_hdr->checksum = igmp_chksum_tx((ushort *)&ip_igmp_hdr->type, (IGMP_SFM_MIN_PAYLOAD_LEN + extraLen));
            /* IPv4 CheckSum Later */
            ip_igmp_hdr->PayloadLen = htons(uip_len - sizeof(ipmc_ip_eth_hdr));
            ip_igmp_hdr->ip_chksum = 0;
            ip_igmp_hdr->ip_chksum = igmp_chksum_tx((ushort *)ip_igmp_hdr, sizeof(igmp_ip4_hdr));
        } else {
            IPMC_MEM_SYSTEM_MGIVE(sfm_src_bak);
            return VTSS_RC_ERROR;
        } /* ipmc_version */

        T_D("\n\rIPMC transmit MASSQ");
        T_D_HEX(&uip_buf[0], uip_len);

        VTSS_PORT_BF_CLR(dst_port_mask.member_ports);
        for (i = 0; i < local_port_cnt; i++) {
            VTSS_PORT_BF_SET(dst_port_mask.member_ports, i, VTSS_PORT_BF_GET(dst_ports, i));
        }

        if (ipmc_lib_packet_tx(&dst_port_mask,
                               tx_untag,
                               FALSE,
                               VTSS_PORT_NO_NONE,
                               (entry->param.mvr ? IPMC_PKT_SRC_MVR : IPMC_PKT_SRC_SNP),
                               entry->param.vid,
                               0,
                               (entry->param.priority & IPMC_PARAM_PRIORITY_MASK),
                               VTSS_GLAG_NO_NONE,
                               &uip_buf[0],
                               uip_len) != VTSS_OK) {
            T_D("Failure in ipmc_lib_packet_tx_ssq for VLAN %d", entry->param.vid);
        } else {
            entry->param.querier.group_queries_sent++;
            entry->param.querier.ipmc_queries_sent++;
        }
    } /* tx_flag */

    IPMC_MEM_SYSTEM_MGIVE(sfm_src_bak);
    return VTSS_OK;
}

vtss_rc ipmc_lib_packet_tx_proxy_query(ipmc_intf_entry_t *entry,
                                       vtss_ipv6_t *query_group_addr,
                                       BOOL force_untag)
{
    u8                  *uip_buf;
    size_t              uip_len;
    ipmc_ip_eth_hdr     *eth_hdr;
    ipmc_ip_mld_hdr     *ip_mld_gen_hdr = NULL;
    ipmc_ip_mldv2_hdr   *ip_mld_sfm_hdr = NULL;
    ipmc_ip_igmp_hdr    *ip_igmp_gen_hdr = NULL;
    ipmc_ip_igmpv3_hdr  *ip_igmp_sfm_hdr = NULL;
    u8                  dst_ports[VTSS_PORT_BF_SIZE];
    uchar               mac_addr[6];
    u32                 i, local_port_cnt;
    ipmc_port_bfs_t     proxy_port_mask;
    BOOL                tx_flag, fwd_map[VTSS_PORT_ARRAY_SIZE];
    char                bufPort[MGMT_PORT_BUF_SIZE];

    if (!ipmc_lib_pkt_done_init) {
        return VTSS_RC_ERROR;
    }

    if (!entry || !query_group_addr) {
        return VTSS_RC_ERROR;
    }

    if (entry->param.mvr) {
        uip_buf = mvr_uip_buf;
    } else {
        uip_buf = snp_uip_buf;
    }

    memset(dst_ports, 0x0, sizeof(dst_ports));
    local_port_cnt = ipmc_lib_get_system_local_port_cnt();
    for (i = 0; i < local_port_cnt; i++) {
        if (VTSS_PORT_BF_GET(entry->vlan_ports, i)) {
            VTSS_PORT_BF_SET(dst_ports, i, TRUE);
        }
    }
#if VTSS_SWITCH_STACKABLE
    /* Filter Stacking Ports, since Distributed IPMC */
    i = PORT_NO_STACK_0;
    VTSS_PORT_BF_SET(dst_ports, i, FALSE);
    i = PORT_NO_STACK_1;
    VTSS_PORT_BF_SET(dst_ports, i, FALSE);
#endif /* VTSS_SWITCH_STACKABLE */

    tx_flag = FALSE;
    memset(fwd_map, 0x0, sizeof(fwd_map));
    local_port_cnt = ipmc_lib_get_system_local_port_cnt();
    for (i = 0; i < local_port_cnt; i++) {
        if (ipmc_lib_get_port_rpstatus(entry->ipmc_version, i)) {
            continue;
        }

        if (VTSS_PORT_BF_GET(dst_ports, i)) {
            if (!tx_flag) {
                tx_flag = TRUE;
                memset((u8 *)&uip_buf[0], 0x0, IPMC_LIB_PKT_BUF_SZ);
            }

            fwd_map[i] = TRUE;
        }
    }

    T_D("ipmc_lib_packet_tx_proxy_query(%s Ports-%s)",
        tx_flag ? "SND" : "",
        mgmt_iport_list2txt(fwd_map, bufPort));
    if (tx_flag) {
        T_D("Transmitting IPMCVer-%d GQ to port(s)", entry->ipmc_version);

        eth_hdr = (ipmc_ip_eth_hdr *)&uip_buf[0];
        if (entry->ipmc_version == IPMC_IP_VERSION_MLD) {
            vtss_ipv6_t ipLinkLocalSrc;

            /* ETHERNET */
            memcpy(eth_hdr->dest.addr, mld_all_node_mac, sizeof(uchar) * 6);
            if (!ipmc_lib_get_system_mgmt_macx(mac_addr)) {
                memset(eth_hdr->src.addr, 0x0, sizeof(uchar) * 6);
            } else {
                memcpy(eth_hdr->src.addr, mac_addr, sizeof(uchar) * 6);
            }
            eth_hdr->type = htons(IP_MULTICAST_V6_ETHER_TYPE);

            /* Depend on Compatibility */
            if ((entry->param.hst_compatibility.mode == VTSS_IPMC_COMPAT_MODE_GEN)) {
                ip_mld_gen_hdr = (ipmc_ip_mld_hdr *)&uip_buf[sizeof(ipmc_ip_eth_hdr)];

                /* IPv6 */
                ip_mld_gen_hdr->VerTcFl = htonl((IP_MULTICAST_V6_IP_VERSION << 28));
                ip_mld_gen_hdr->NxtHdr = MLD_IPV6_NEXTHDR_OPT_HBH;
                ip_mld_gen_hdr->HopLimit = IPMC_IPHDR_HOPLIMIT;
                /* get src address */
                if (ipmc_lib_get_eui64_linklocal_addr(&ipLinkLocalSrc)) {
                    /* link-local source ip */
                    memcpy(&ip_mld_gen_hdr->ip6_src, &ipLinkLocalSrc, sizeof(vtss_ipv6_t));
                } else {
                    return VTSS_RC_ERROR;
                }
                /* dst ip */
                if (ipmc_lib_isaddr6_all_zero(query_group_addr)) {
                    /* general query, dest ip = all-node */
                    ipmc_lib_get_all_node_ipv6_addr(&ip_mld_gen_hdr->ip6_dst);
                } else {
                    /* dest ip = group address */
                    memcpy(&ip_mld_gen_hdr->ip6_dst, query_group_addr, sizeof(vtss_ipv6_t));
                }

                /* Hop-By-Hop */
                ip_mld_gen_hdr->HBHNxtHdr = MLD_IPV6_NEXTHDR_ICMP;
                ip_mld_gen_hdr->HdrExtLen = 0x0;
                ip_mld_gen_hdr->OptNPad[0] = IPMC_IPV6_RTR_ALERT_PREFIX1;
                ip_mld_gen_hdr->OptNPad[1] = IPMC_IPV6_RTR_ALERT_PREFIX2;

                /* MLD */
                ip_mld_gen_hdr->type = IPMC_MLD_MSG_TYPE_QUERY;
                ip_mld_gen_hdr->code = 0x0;
                memcpy(&ip_mld_gen_hdr->group, query_group_addr, sizeof(vtss_ipv6_t));

                uip_len = sizeof(ipmc_ip_eth_hdr) + MLD_MIN_OFFSET + MLD_GEN_MIN_PAYLOAD_LEN;
                ip_mld_gen_hdr->PayloadLen = htons(uip_len - sizeof(ipmc_ip_eth_hdr) - IPV6_HDR_FIXED_LEN);

                ip_mld_gen_hdr->max_resp_time = htons(entry->param.querier.MaxResTime);
                ip_mld_gen_hdr->checksum = 0;
                ip_mld_gen_hdr->checksum = mld_chksum_tx((mld_ip6_hdr *)ip_mld_gen_hdr, MLD_MIN_OFFSET, (ulong)(ntohs(ip_mld_gen_hdr->PayloadLen) - MLD_MIN_HBH_LEN));
            } else {
                ip_mld_sfm_hdr = (ipmc_ip_mldv2_hdr *)&uip_buf[sizeof(ipmc_ip_eth_hdr)];

                /* IPv6 */
                ip_mld_sfm_hdr->VerTcFl = htonl((IP_MULTICAST_V6_IP_VERSION << 28));
                ip_mld_sfm_hdr->NxtHdr = MLD_IPV6_NEXTHDR_OPT_HBH;
                ip_mld_sfm_hdr->HopLimit = IPMC_IPHDR_HOPLIMIT;
                /* get src address */
                if (ipmc_lib_get_eui64_linklocal_addr(&ipLinkLocalSrc)) {
                    /* link-local source ip */
                    memcpy(&ip_mld_sfm_hdr->ip6_src, &ipLinkLocalSrc, sizeof(vtss_ipv6_t));
                }
                /* dst ip */
                if (ipmc_lib_isaddr6_all_zero(query_group_addr)) {
                    /* general query, dest ip = all-node */
                    ipmc_lib_get_all_node_ipv6_addr(&ip_mld_sfm_hdr->ip6_dst);
                } else {
                    /* dest ip = group address */
                    memcpy(&ip_mld_sfm_hdr->ip6_dst, query_group_addr, sizeof(vtss_ipv6_t));
                }

                /* Hop-By-Hop */
                ip_mld_sfm_hdr->HBHNxtHdr = MLD_IPV6_NEXTHDR_ICMP;
                ip_mld_sfm_hdr->HdrExtLen = 0x0;
                ip_mld_sfm_hdr->OptNPad[0] = IPMC_IPV6_RTR_ALERT_PREFIX1;
                ip_mld_sfm_hdr->OptNPad[1] = IPMC_IPV6_RTR_ALERT_PREFIX2;

                /* MLD */
                ip_mld_sfm_hdr->type = IPMC_MLD_MSG_TYPE_QUERY;
                ip_mld_sfm_hdr->code = 0x0;

                uip_len = sizeof(ipmc_ip_eth_hdr) + MLD_MIN_OFFSET + MLD_SFM_MIN_PAYLOAD_LEN;
                ip_mld_sfm_hdr->PayloadLen = htons(uip_len - sizeof(ipmc_ip_eth_hdr) - IPV6_HDR_FIXED_LEN);

                ip_mld_sfm_hdr->sfminfo.query.max_resp_time = htons(entry->param.querier.MaxResTime);
                memcpy(&ip_mld_sfm_hdr->sfminfo.query.group_address, query_group_addr, sizeof(vtss_ipv6_t));
                IPMC_PKT_DO_QRY_SUPP(ip_mld_sfm_hdr);
                if (IPMC_TIMER_RV(entry) < 0x7) {
                    ip_mld_sfm_hdr->sfminfo.query.resv_s_qrv |= IPMC_TIMER_RV(entry);
                }
                ip_mld_sfm_hdr->sfminfo.query.qqic = IPMC_TIMER_QI(entry) & 0xFF;

                ip_mld_sfm_hdr->checksum = 0;
                ip_mld_sfm_hdr->checksum = mld_chksum_tx((mld_ip6_hdr *)ip_mld_sfm_hdr, MLD_MIN_OFFSET, (ulong)(ntohs(ip_mld_sfm_hdr->PayloadLen) - MLD_MIN_HBH_LEN));
            }
        } else if (entry->ipmc_version == IPMC_IP_VERSION_IGMP) {
            vtss_ipv4_t ip4addr = 0;

            /* ETHERNET */
            memcpy(eth_hdr->dest.addr, igmp_all_node_mac, sizeof(uchar) * 6);
            if (!ipmc_lib_get_system_mgmt_macx(mac_addr)) {
                memset(eth_hdr->src.addr, 0x0, sizeof(uchar) * 6);
            } else {
                memcpy(eth_hdr->src.addr, mac_addr, sizeof(uchar) * 6);
            }
            eth_hdr->type = htons(IP_MULTICAST_V4_ETHER_TYPE);

            /* Depend on Compatibility */
            if ((entry->param.hst_compatibility.mode == VTSS_IPMC_COMPAT_MODE_OLD) ||
                (entry->param.hst_compatibility.mode == VTSS_IPMC_COMPAT_MODE_GEN)) {
                ip_igmp_gen_hdr = (ipmc_ip_igmp_hdr *)&uip_buf[sizeof(ipmc_ip_eth_hdr)];

                /* IPv4 */
                ip_igmp_gen_hdr->vhl = (IP_MULTICAST_V4_IP_VERSION << 4);
                ip_igmp_gen_hdr->vhl |= sizeof(igmp_ip4_hdr) / 4;
                ip_igmp_gen_hdr->tos = 0;
                ip_igmp_gen_hdr->seq_id = ip_igmp_gen_hdr->offset = 0;
                ip_igmp_gen_hdr->ttl = IPMC_IPHDR_HOPLIMIT;
                ip_igmp_gen_hdr->proto = IP_MULTICAST_IGMP_PROTO_ID;
                ip_igmp_gen_hdr->router_option[0] = IPMC_IPV4_RTR_ALERT_PREFIX1;
                ip_igmp_gen_hdr->router_option[1] = IPMC_IPV4_RTR_ALERT_PREFIX2;

                /* get src address */
                if (ipmc_lib_get_ipintf_igmp_adrs(entry, &ip4addr)) {
                    ip4addr = htonl(ip4addr);
                } else {
                    return VTSS_RC_ERROR;
                }
                memcpy(&ip_igmp_gen_hdr->ip4_src, &ip4addr, sizeof(ipmcv4addr));
                /* dst ip */
                if (ipmc_lib_isaddr4_all_zero((ipmcv4addr *)&query_group_addr->addr[12])) {
                    /* general query, dest ip = 224.0.0.1 */
                    ipmc_lib_get_all_node_ipv4_addr(&ip_igmp_gen_hdr->ip4_dst);
                } else {
                    /* dest ip = group address */
                    memcpy(&ip_igmp_gen_hdr->ip4_dst, &query_group_addr->addr[12], sizeof(ipmcv4addr));
                }

                /* IGMP */
                ip_igmp_gen_hdr->type = IPMC_IGMP_MSG_TYPE_QUERY;
                memcpy(&ip_igmp_gen_hdr->group, &query_group_addr->addr[12], sizeof(ipmcv4addr));
                if ((entry->param.hst_compatibility.mode == VTSS_IPMC_COMPAT_MODE_OLD) ||
                    (entry->param.rtr_compatibility.mode == VTSS_IPMC_COMPAT_MODE_OLD)) {
                    ip_igmp_gen_hdr->max_resp_time = 0x0;
                } else {
                    ip_igmp_gen_hdr->max_resp_time = (entry->param.querier.MaxResTime) & 0xFF;
                }
                uip_len = sizeof(ipmc_ip_eth_hdr) + sizeof(igmp_ip4_hdr) + IGMP_MIN_PAYLOAD_LEN;

                /* IGMP CheckSum First */
                ip_igmp_gen_hdr->checksum = 0;
                ip_igmp_gen_hdr->checksum = igmp_chksum_tx((ushort *)&ip_igmp_gen_hdr->type, IGMP_MIN_PAYLOAD_LEN);
                /* IPv4 CheckSum Later */
                ip_igmp_gen_hdr->PayloadLen = htons(uip_len - sizeof(ipmc_ip_eth_hdr));
                ip_igmp_gen_hdr->ip_chksum = 0;
                ip_igmp_gen_hdr->ip_chksum = igmp_chksum_tx((ushort *)ip_igmp_gen_hdr, sizeof(igmp_ip4_hdr));
            } else {
                ip_igmp_sfm_hdr = (ipmc_ip_igmpv3_hdr *)&uip_buf[sizeof(ipmc_ip_eth_hdr)];

                /* IPv4 */
                ip_igmp_sfm_hdr->vhl = (IP_MULTICAST_V4_IP_VERSION << 4);
                ip_igmp_sfm_hdr->vhl |= sizeof(igmp_ip4_hdr) / 4;
                ip_igmp_sfm_hdr->tos = 0;
                ip_igmp_sfm_hdr->seq_id = ip_igmp_sfm_hdr->offset = 0;
                ip_igmp_sfm_hdr->ttl = IPMC_IPHDR_HOPLIMIT;
                ip_igmp_sfm_hdr->proto = IP_MULTICAST_IGMP_PROTO_ID;
                ip_igmp_sfm_hdr->router_option[0] = IPMC_IPV4_RTR_ALERT_PREFIX1;
                ip_igmp_sfm_hdr->router_option[1] = IPMC_IPV4_RTR_ALERT_PREFIX2;

                /* get src address */
                if (ipmc_lib_get_ipintf_igmp_adrs(entry, &ip4addr)) {
                    ip4addr = htonl(ip4addr);
                } else {
                    return VTSS_RC_ERROR;
                }
                memcpy(&ip_igmp_sfm_hdr->ip4_src, &ip4addr, sizeof(ipmcv4addr));
                /* dst ip */
                if (ipmc_lib_isaddr4_all_zero((ipmcv4addr *)&query_group_addr->addr[12])) {
                    /* general query, dest ip = 224.0.0.1 */
                    ipmc_lib_get_all_node_ipv4_addr(&ip_igmp_sfm_hdr->ip4_dst);
                } else {
                    /* dest ip = group address */
                    memcpy(&ip_igmp_sfm_hdr->ip4_dst, &query_group_addr->addr[12], sizeof(ipmcv4addr));
                }

                /* IGMP */
                ip_igmp_sfm_hdr->type = IPMC_IGMP_MSG_TYPE_QUERY;
                ip_igmp_sfm_hdr->code = (entry->param.querier.MaxResTime) & 0xFF;

                memcpy(&ip_igmp_sfm_hdr->sfminfo.query.group_address, &query_group_addr->addr[12], sizeof(ipmcv4addr));
                IPMC_PKT_DO_QRY_SUPP(ip_igmp_sfm_hdr);
                if (IPMC_TIMER_RV(entry) < 0x7) {
                    ip_igmp_sfm_hdr->sfminfo.query.resv_s_qrv |= IPMC_TIMER_RV(entry);
                }
                ip_igmp_sfm_hdr->sfminfo.query.qqic = IPMC_TIMER_QI(entry) & 0xFF;

                uip_len = sizeof(ipmc_ip_eth_hdr) + sizeof(igmp_ip4_hdr) + IGMP_SFM_MIN_PAYLOAD_LEN;

                /* IGMP CheckSum First */
                ip_igmp_sfm_hdr->checksum = 0;
                ip_igmp_sfm_hdr->checksum = igmp_chksum_tx((ushort *)&ip_igmp_sfm_hdr->type, IGMP_SFM_MIN_PAYLOAD_LEN);
                /* IPv4 CheckSum Later */
                ip_igmp_sfm_hdr->PayloadLen = htons(uip_len - sizeof(ipmc_ip_eth_hdr));
                ip_igmp_sfm_hdr->ip_chksum = 0;
                ip_igmp_sfm_hdr->ip_chksum = igmp_chksum_tx((ushort *)ip_igmp_sfm_hdr, sizeof(igmp_ip4_hdr));
            }
        } else {
            return VTSS_RC_ERROR;
        } /* ipmc_version */

        T_D("\n\rIPMC transmit GQ");
        T_D_HEX(&uip_buf[0], uip_len);

        memset(&proxy_port_mask, 0x0, sizeof(ipmc_port_bfs_t));
        for (i = 0; i < local_port_cnt; i++) {
            if (VTSS_PORT_BF_GET(dst_ports, i) && !ipmc_lib_get_port_rpstatus(entry->ipmc_version, i)) {
                VTSS_PORT_BF_SET(proxy_port_mask.member_ports, i, TRUE);
            }
        }

        if (ipmc_lib_packet_tx(&proxy_port_mask,
                               force_untag,
                               FALSE,
                               VTSS_PORT_NO_NONE,
                               (entry->param.mvr ? IPMC_PKT_SRC_MVR : IPMC_PKT_SRC_SNP),
                               entry->param.vid,
                               0,
                               (entry->param.priority & IPMC_PARAM_PRIORITY_MASK),
                               VTSS_GLAG_NO_NONE,
                               &uip_buf[0],
                               uip_len) != VTSS_OK) {
            T_D("Failure in ipmc_lib_packet_tx_gq for VLAN %d", entry->param.vid);
        } else {
            entry->param.querier.ipmc_queries_sent++;
        }
    } /* tx_flag */

    return VTSS_OK;
}

vtss_rc ipmc_lib_packet_tx_helping_query(ipmc_intf_entry_t *entry,
                                         vtss_ipv6_t *query_group_addr,
                                         ipmc_port_bfs_t *dst_port_mask,
                                         BOOL force_untag,
                                         BOOL debug)
{
    u8                  *uip_buf;
    size_t              uip_len;
    ipmc_ip_eth_hdr     *eth_hdr;
    ipmc_ip_mld_hdr     *ip_mld_hdr = NULL;
    ipmc_ip_igmp_hdr    *ip_igmp_hdr = NULL;
    uchar               mac_addr[6];
    u32                 pit, local_port_cnt;
    BOOL                tx_flag;

    if (!ipmc_lib_pkt_done_init) {
        return VTSS_RC_ERROR;
    }

    if (!entry || !query_group_addr || !dst_port_mask) {
        return VTSS_RC_ERROR;
    }

    if (entry->param.mvr) {
        uip_buf = mvr_uip_buf;
    } else {
        uip_buf = snp_uip_buf;
    }

#if VTSS_SWITCH_STACKABLE
    if (!debug) {
        u32 idx;

        /* Filter Stacking Ports, since Distributed IPMC */
        idx = PORT_NO_STACK_0;
        VTSS_PORT_BF_SET(dst_port_mask->member_ports, idx, FALSE);
        idx = PORT_NO_STACK_1;
        VTSS_PORT_BF_SET(dst_port_mask->member_ports, idx, FALSE);
    }
#endif /* VTSS_SWITCH_STACKABLE */

    tx_flag = FALSE;
    local_port_cnt = ipmc_lib_get_system_local_port_cnt();
    for (pit = 0; pit < local_port_cnt; pit++) {
        if (VTSS_PORT_BF_GET(dst_port_mask->member_ports, pit)) {
            tx_flag = TRUE;
            memset((u8 *)&uip_buf[0], 0x0, IPMC_LIB_PKT_BUF_SZ);
            break;
        }
    }

    T_D("ipmc_lib_packet_tx_helping_query(%s)", tx_flag ? "SND" : "");
    if (tx_flag) {
        T_D("  Transmitting general query to port(s)");
        eth_hdr = (ipmc_ip_eth_hdr *)&uip_buf[0];
        if (entry->ipmc_version == IPMC_IP_VERSION_MLD) {
            vtss_ipv6_t ipLinkLocalSrc;

            ip_mld_hdr = (ipmc_ip_mld_hdr *)&uip_buf[sizeof(ipmc_ip_eth_hdr)];

            /* ETHERNET */
            if (ipmc_lib_isaddr6_all_zero(query_group_addr)) {
                memcpy(eth_hdr->dest.addr, mld_all_node_mac, sizeof(ipmc_eth_addr));
            } else {
                ipmc_lib_packet_dst_mac(eth_hdr->dest.addr, query_group_addr, entry->ipmc_version);
            }

            if (!ipmc_lib_get_system_mgmt_macx(mac_addr)) {
                memset(eth_hdr->src.addr, 0x0, sizeof(uchar) * 6);
            } else {
                memcpy(eth_hdr->src.addr, mac_addr, sizeof(uchar) * 6);
            }
            eth_hdr->type = htons(IP_MULTICAST_V6_ETHER_TYPE);

            /* IPv6 */
            ip_mld_hdr->VerTcFl = htonl((IP_MULTICAST_V6_IP_VERSION << 28));
            ip_mld_hdr->NxtHdr = MLD_IPV6_NEXTHDR_OPT_HBH;
            ip_mld_hdr->HopLimit = IPMC_IPHDR_HOPLIMIT;
            /* get src address */
            if (ipmc_lib_get_eui64_linklocal_addr(&ipLinkLocalSrc)) {
                /* link-local source ip */
                memcpy(&ip_mld_hdr->ip6_src, &ipLinkLocalSrc, sizeof(vtss_ipv6_t));
            }
            /* dst ip */
            if (ipmc_lib_isaddr6_all_zero(query_group_addr)) {
                /* general query, dest ip = all-node */
                ipmc_lib_get_all_node_ipv6_addr(&ip_mld_hdr->ip6_dst);
            } else {
                /* dest ip = group address */
                memcpy(&ip_mld_hdr->ip6_dst, query_group_addr, sizeof(vtss_ipv6_t));
            }

            /* Hop-By-Hop */
            ip_mld_hdr->HBHNxtHdr = MLD_IPV6_NEXTHDR_ICMP;
            ip_mld_hdr->HdrExtLen = 0x0;
            ip_mld_hdr->OptNPad[0] = IPMC_IPV6_RTR_ALERT_PREFIX1;
            ip_mld_hdr->OptNPad[1] = IPMC_IPV6_RTR_ALERT_PREFIX2;

            /* MLD */
            ip_mld_hdr->type = IPMC_MLD_MSG_TYPE_QUERY;
            ip_mld_hdr->code = 0x0;
            ip_mld_hdr->max_resp_time = htons(IPMC_PARAM_DEF_QRI);
            memcpy(&ip_mld_hdr->group, query_group_addr, sizeof(vtss_ipv6_t));

            uip_len = sizeof(ipmc_ip_eth_hdr) + sizeof(ipmc_ip_mld_hdr);
            ip_mld_hdr->PayloadLen = htons(uip_len - sizeof(ipmc_ip_eth_hdr) - IPV6_HDR_FIXED_LEN);

            ip_mld_hdr->checksum = 0;
            ip_mld_hdr->checksum = mld_chksum_tx((mld_ip6_hdr *)ip_mld_hdr, MLD_MIN_OFFSET, (ulong)(ntohs(ip_mld_hdr->PayloadLen) - MLD_MIN_HBH_LEN));
        } else if (entry->ipmc_version == IPMC_IP_VERSION_IGMP) {
            vtss_ipv4_t ip4addr = 0;

            ip_igmp_hdr = (ipmc_ip_igmp_hdr *)&uip_buf[sizeof(ipmc_ip_eth_hdr)];

            /* ETHERNET */
            if (ipmc_lib_isaddr4_all_zero((ipmcv4addr *)&query_group_addr->addr[12])) {
                memcpy(eth_hdr->dest.addr, igmp_all_node_mac, sizeof(ipmc_eth_addr));
            } else {
                ipmc_lib_packet_dst_mac(eth_hdr->dest.addr, query_group_addr, entry->ipmc_version);
            }

            if (!ipmc_lib_get_system_mgmt_macx(mac_addr)) {
                memset(eth_hdr->src.addr, 0x0, sizeof(uchar) * 6);
            } else {
                memcpy(eth_hdr->src.addr, mac_addr, sizeof(uchar) * 6);
            }
            eth_hdr->type = htons(IP_MULTICAST_V4_ETHER_TYPE);

            /* IPv4 */
            ip_igmp_hdr->vhl = (IP_MULTICAST_V4_IP_VERSION << 4);
            ip_igmp_hdr->vhl |= sizeof(igmp_ip4_hdr) / 4;
            ip_igmp_hdr->tos = 0;
            ip_igmp_hdr->seq_id = ip_igmp_hdr->offset = 0;
            ip_igmp_hdr->ttl = IPMC_IPHDR_HOPLIMIT;
            ip_igmp_hdr->proto = IP_MULTICAST_IGMP_PROTO_ID;
            ip_igmp_hdr->router_option[0] = IPMC_IPV4_RTR_ALERT_PREFIX1;
            ip_igmp_hdr->router_option[1] = IPMC_IPV4_RTR_ALERT_PREFIX2;

            /* get src address */
            if (ipmc_lib_get_ipintf_igmp_adrs(entry, &ip4addr)) {
                ip4addr = htonl(ip4addr);
            }
            memcpy(&ip_igmp_hdr->ip4_src, &ip4addr, sizeof(ipmcv4addr));
            /* dst ip */
            if (ipmc_lib_isaddr4_all_zero((ipmcv4addr *)&query_group_addr->addr[12])) {
                /* general query, dest ip = 224.0.0.1 */
                ipmc_lib_get_all_node_ipv4_addr(&ip_igmp_hdr->ip4_dst);
            } else {
                /* dest ip = group address */
                memcpy(&ip_igmp_hdr->ip4_dst, &query_group_addr->addr[12], sizeof(ipmcv4addr));
            }

            /* IGMP */
            ip_igmp_hdr->type = IPMC_IGMP_MSG_TYPE_QUERY;
            ip_igmp_hdr->max_resp_time = (uchar)(IPMC_PARAM_DEF_QRI);
            memcpy(&ip_igmp_hdr->group, &query_group_addr->addr[12], sizeof(ipmcv4addr));

            uip_len = sizeof(ipmc_ip_eth_hdr) + sizeof(ipmc_ip_igmp_hdr);

            /* IGMP CheckSum First */
            ip_igmp_hdr->checksum = 0;
            ip_igmp_hdr->checksum = igmp_chksum_tx((ushort *)&ip_igmp_hdr->type, sizeof(ipmc_ip_igmp_hdr) - sizeof(igmp_ip4_hdr));
            /* IPv4 CheckSum Later */
            ip_igmp_hdr->PayloadLen = htons(uip_len - sizeof(ipmc_ip_eth_hdr));
            ip_igmp_hdr->ip_chksum = 0; /* init. */
            ip_igmp_hdr->ip_chksum = igmp_chksum_tx((ushort *)ip_igmp_hdr, sizeof(igmp_ip4_hdr));
        } else {
            return VTSS_RC_ERROR;
        }

        /* not entirely a snooped frame, but in this context this is what we want */
        if (ipmc_lib_packet_tx(dst_port_mask,
                               force_untag,
                               FALSE,
                               VTSS_PORT_NO_NONE,
                               (entry->param.mvr ? IPMC_PKT_SRC_MVR : IPMC_PKT_SRC_SNP),
                               entry->param.vid,
                               0,
                               (entry->param.priority & IPMC_PARAM_PRIORITY_MASK),
                               VTSS_GLAG_NO_NONE,
                               &uip_buf[0],
                               uip_len) != VTSS_OK) {
            T_D("Failure in ipmc_lib_packet_tx_helping_query for VLAN %d", entry->param.vid);
        } else {
            entry->param.querier.ipmc_queries_sent++;
        }
    } /* tx_flag */

    return VTSS_OK;
}

vtss_rc ipmc_lib_packet_tx_group_leave(ipmc_intf_entry_t *entry,
                                       vtss_ipv6_t *leave_group_addr,
                                       ipmc_port_bfs_t *dst_port_mask,
                                       BOOL force_untag,
                                       BOOL debug)
{
    u8                  *uip_buf;
    size_t              uip_len;
    ipmc_ip_eth_hdr     *eth_hdr;
    ipmc_ip_mld_hdr     *ip_mld_hdr = NULL;
    ipmc_ip_igmp_hdr    *ip_igmp_hdr = NULL;
    u8                  mac_addr[6];
    u32                 pit, local_port_cnt;
    BOOL                tx_flag, fwd_map[VTSS_PORT_ARRAY_SIZE];
    char                bufPort[MGMT_PORT_BUF_SIZE];
    ipmc_compat_mode_t  compat;

    if (!ipmc_lib_pkt_done_init) {
        return VTSS_RC_ERROR;
    }

    if (!entry || !leave_group_addr || !dst_port_mask) {
        return VTSS_RC_ERROR;
    }

    compat = entry->param.rtr_compatibility.mode;
    if (compat == VTSS_IPMC_COMPAT_MODE_OLD) {
        return VTSS_OK;
    }

    if (entry->param.mvr) {
        uip_buf = mvr_uip_buf;
    } else {
        uip_buf = snp_uip_buf;
    }

#if VTSS_SWITCH_STACKABLE
    if (!debug) {
        u32 idx;

        /* Filter Stacking Ports, since Distributed IPMC */
        idx = PORT_NO_STACK_0;
        VTSS_PORT_BF_SET(dst_port_mask->member_ports, idx, FALSE);
        idx = PORT_NO_STACK_1;
        VTSS_PORT_BF_SET(dst_port_mask->member_ports, idx, FALSE);
    }
#endif /* VTSS_SWITCH_STACKABLE */

    tx_flag = FALSE;
    memset(fwd_map, 0x0, sizeof(fwd_map));
    local_port_cnt = ipmc_lib_get_system_local_port_cnt();
    for (pit = 0; pit < local_port_cnt; pit++) {
        if (VTSS_PORT_BF_GET(dst_port_mask->member_ports, pit)) {
            if (!tx_flag) {
                tx_flag = TRUE;
                memset((u8 *)&uip_buf[0], 0x0, IPMC_LIB_PKT_BUF_SZ);
            }

            fwd_map[pit] = TRUE;
        }
    }

    T_D("ipmc_lib_packet_tx_group_leave(%s Ports-%s)",
        tx_flag ? "SND" : "",
        mgmt_iport_list2txt(fwd_map, bufPort));
    if (tx_flag) {
        eth_hdr = (ipmc_ip_eth_hdr *)&uip_buf[0];
        if (entry->ipmc_version == IPMC_IP_VERSION_MLD) {
            vtss_ipv6_t ipLinkLocalSrc;

            ip_mld_hdr = (ipmc_ip_mld_hdr *)&uip_buf[sizeof(ipmc_ip_eth_hdr)];

            /* ETHERNET */
            memcpy(eth_hdr->dest.addr, mld_all_rtr_mac, sizeof(uchar) * 6);
            if (!ipmc_lib_get_system_mgmt_macx(mac_addr)) {
                memset(eth_hdr->src.addr, 0x0, sizeof(uchar) * 6);
            } else {
                memcpy(eth_hdr->src.addr, mac_addr, sizeof(uchar) * 6);
            }
            eth_hdr->type = htons(IP_MULTICAST_V6_ETHER_TYPE);

            /* IPv6 */
            ip_mld_hdr->VerTcFl = htonl((IP_MULTICAST_V6_IP_VERSION << 28));
            ip_mld_hdr->NxtHdr = MLD_IPV6_NEXTHDR_OPT_HBH;
            ip_mld_hdr->HopLimit = IPMC_IPHDR_HOPLIMIT;
            /* get src address */
            if (ipmc_lib_get_eui64_linklocal_addr(&ipLinkLocalSrc)) {
                /* link-local source ip */
                memcpy(&ip_mld_hdr->ip6_src, &ipLinkLocalSrc, sizeof(vtss_ipv6_t));
            }
            /* Leave MSG, dest ip = all-router */
            ipmc_lib_get_all_router_ipv6_addr(&ip_mld_hdr->ip6_dst);

            /* Hop-By-Hop */
            ip_mld_hdr->HBHNxtHdr = MLD_IPV6_NEXTHDR_ICMP;
            ip_mld_hdr->HdrExtLen = 0x0;
            ip_mld_hdr->OptNPad[0] = IPMC_IPV6_RTR_ALERT_PREFIX1;
            ip_mld_hdr->OptNPad[1] = IPMC_IPV6_RTR_ALERT_PREFIX2;

            /* MLD */
            ip_mld_hdr->type = IPMC_MLD_MSG_TYPE_DONE;
            ip_mld_hdr->code = 0x0;
            ip_mld_hdr->max_resp_time = 0x0;
            memcpy(&ip_mld_hdr->group, leave_group_addr, sizeof(vtss_ipv6_t));

            uip_len = sizeof(ipmc_ip_eth_hdr) + sizeof(ipmc_ip_mld_hdr);
            ip_mld_hdr->PayloadLen = htons(uip_len - sizeof(ipmc_ip_eth_hdr) - IPV6_HDR_FIXED_LEN);

            ip_mld_hdr->checksum = 0;
            ip_mld_hdr->checksum = mld_chksum_tx((mld_ip6_hdr *)ip_mld_hdr, MLD_MIN_OFFSET, (ulong)(ntohs(ip_mld_hdr->PayloadLen) - MLD_MIN_HBH_LEN));
        } else if (entry->ipmc_version == IPMC_IP_VERSION_IGMP) {
            vtss_ipv4_t ip4addr = 0;

            ip_igmp_hdr = (ipmc_ip_igmp_hdr *)&uip_buf[sizeof(ipmc_ip_eth_hdr)];

            /* ETHERNET */
            memcpy(eth_hdr->dest.addr, igmp_all_rtr_mac, sizeof(uchar) * 6);
            if (!ipmc_lib_get_system_mgmt_macx(mac_addr)) {
                memset(eth_hdr->src.addr, 0x0, sizeof(uchar) * 6);
            } else {
                memcpy(eth_hdr->src.addr, mac_addr, sizeof(uchar) * 6);
            }
            eth_hdr->type = htons(IP_MULTICAST_V4_ETHER_TYPE);

            /* IPv4 */
            ip_igmp_hdr->vhl = (IP_MULTICAST_V4_IP_VERSION << 4);
            ip_igmp_hdr->vhl |= sizeof(igmp_ip4_hdr) / 4;
            ip_igmp_hdr->tos = 0;
            ip_igmp_hdr->seq_id = ip_igmp_hdr->offset = 0;
            ip_igmp_hdr->ttl = IPMC_IPHDR_HOPLIMIT;
            ip_igmp_hdr->proto = IP_MULTICAST_IGMP_PROTO_ID;
            ip_igmp_hdr->router_option[0] = IPMC_IPV4_RTR_ALERT_PREFIX1;
            ip_igmp_hdr->router_option[1] = IPMC_IPV4_RTR_ALERT_PREFIX2;

            /* get src address */
            if (ipmc_lib_get_ipintf_igmp_adrs(entry, &ip4addr)) {
                ip4addr = htonl(ip4addr);
            }
            memcpy(&ip_igmp_hdr->ip4_src, &ip4addr, sizeof(ipmcv4addr));
            /* Leave MSG, dest ip = all-router */
            ipmc_lib_get_all_router_ipv4_addr(&ip_igmp_hdr->ip4_dst);

            /* IGMP */
            ip_igmp_hdr->type = IPMC_IGMP_MSG_TYPE_LEAVE;
            ip_igmp_hdr->max_resp_time = 0x0;
            memcpy(&ip_igmp_hdr->group, &leave_group_addr->addr[12], sizeof(ipmcv4addr));

            uip_len = sizeof(ipmc_ip_eth_hdr) + sizeof(ipmc_ip_igmp_hdr);

            /* IGMP CheckSum First */
            ip_igmp_hdr->checksum = 0;
            ip_igmp_hdr->checksum = igmp_chksum_tx((ushort *)&ip_igmp_hdr->type, sizeof(ipmc_ip_igmp_hdr) - sizeof(igmp_ip4_hdr));
            /* IPv4 CheckSum Later */
            ip_igmp_hdr->PayloadLen = htons(uip_len - sizeof(ipmc_ip_eth_hdr));
            ip_igmp_hdr->ip_chksum = 0; /* init. */
            ip_igmp_hdr->ip_chksum = igmp_chksum_tx((ushort *)ip_igmp_hdr, sizeof(igmp_ip4_hdr));
        } else {
            return VTSS_RC_ERROR;
        }

        T_D("\n\rIPMC transmit COMPAT-%d leave/done packet", compat);
        T_D_HEX(&uip_buf[0], uip_len);
        if (ipmc_lib_packet_tx(dst_port_mask,
                               force_untag,
                               FALSE,
                               VTSS_PORT_NO_NONE,
                               (entry->param.mvr ? IPMC_PKT_SRC_MVR : IPMC_PKT_SRC_SNP),
                               entry->param.vid,
                               0,
                               0,
                               VTSS_GLAG_NO_NONE,
                               &uip_buf[0],
                               uip_len) != VTSS_OK) {
            T_D("Failure in ipmc_lib_packet_tx_group_leave for VLAN %d", entry->param.vid);
        }
    } /* tx_flag */

    return VTSS_OK;
}

vtss_rc ipmc_lib_packet_tx_join_report(BOOL is_mvr,
                                       ipmc_compat_mode_t compat,
                                       ipmc_intf_entry_t *entry,
                                       vtss_ipv6_t *join_group_addr,
                                       ipmc_port_bfs_t *dst_port_mask,
                                       ipmc_ip_version_t version,
                                       BOOL force_untag,
                                       BOOL proxy,
                                       BOOL debug)
{
    u8                  *uip_buf;
    size_t              uip_len;
    ipmc_ip_eth_hdr     *eth_hdr;
    ipmc_ip_mld_hdr     *ip_mld_hdr = NULL;
    ipmc_ip_igmp_hdr    *ip_igmp_hdr = NULL;
    vtss_vid_t          vid;
    uchar               mac_addr[6];
    u32                 pit, local_port_cnt;
    BOOL                tx_flag, fwd_map[VTSS_PORT_ARRAY_SIZE];
    char                bufPort[MGMT_PORT_BUF_SIZE];

    if (!ipmc_lib_pkt_done_init) {
        return VTSS_RC_ERROR;
    }

    if (!entry || !join_group_addr || !dst_port_mask) {
        return VTSS_RC_ERROR;
    }

    if (is_mvr) {
        uip_buf = mvr_uip_buf;
    } else {
        uip_buf = snp_uip_buf;
    }

#if VTSS_SWITCH_STACKABLE
    if (!debug) {
        u32 idx;

        /* Filter Stacking Ports, since Distributed IPMC */
        idx = PORT_NO_STACK_0;
        VTSS_PORT_BF_SET(dst_port_mask->member_ports, idx, FALSE);
        idx = PORT_NO_STACK_1;
        VTSS_PORT_BF_SET(dst_port_mask->member_ports, idx, FALSE);
    }
#endif /* VTSS_SWITCH_STACKABLE */

    tx_flag = FALSE;
    memset(fwd_map, 0x0, sizeof(fwd_map));
    local_port_cnt = ipmc_lib_get_system_local_port_cnt();
    for (pit = 0; pit < local_port_cnt; pit++) {
        if (VTSS_PORT_BF_GET(dst_port_mask->member_ports, pit)) {
            if (!tx_flag) {
                tx_flag = TRUE;
                memset((u8 *)&uip_buf[0], 0x0, IPMC_LIB_PKT_BUF_SZ);
            }

            fwd_map[pit] = TRUE;
        }
    }

    vid = entry->param.vid;
    T_D("ipmc_lib_packet_tx_join_report(%s Ports-%s)",
        tx_flag ? "SND" : "",
        mgmt_iport_list2txt(fwd_map, bufPort));
    if (tx_flag) {
        eth_hdr = (ipmc_ip_eth_hdr *)&uip_buf[0];
        if (version == IPMC_IP_VERSION_MLD) {
            vtss_ipv6_t ipLinkLocalSrc;

            ip_mld_hdr = (ipmc_ip_mld_hdr *)&uip_buf[sizeof(ipmc_ip_eth_hdr)];

            /* ETHERNET */
            if ((compat == VTSS_IPMC_COMPAT_MODE_AUTO) || (compat == VTSS_IPMC_COMPAT_MODE_SFM)) {
                if (proxy) {    /* Current PROXY doesn't run in SFM mode */
                    ipmc_lib_packet_dst_mac(eth_hdr->dest.addr, join_group_addr, version);
                } else {
                    memcpy(eth_hdr->dest.addr, mld_sfm_rpt_mac, sizeof(ipmc_eth_addr));
                }
            } else {
                ipmc_lib_packet_dst_mac(eth_hdr->dest.addr, join_group_addr, version);
            }
            if (!ipmc_lib_get_system_mgmt_macx(mac_addr)) {
                memset(eth_hdr->src.addr, 0x0, sizeof(uchar) * 6);
            } else {
                memcpy(eth_hdr->src.addr, mac_addr, sizeof(uchar) * 6);
            }
            eth_hdr->type = htons(IP_MULTICAST_V6_ETHER_TYPE);

            /* IPv6 */
            ip_mld_hdr->VerTcFl = htonl((IP_MULTICAST_V6_IP_VERSION << 28));
            ip_mld_hdr->NxtHdr = MLD_IPV6_NEXTHDR_OPT_HBH;
            ip_mld_hdr->HopLimit = IPMC_IPHDR_HOPLIMIT;
            /* get src address */
            if (ipmc_lib_get_eui64_linklocal_addr(&ipLinkLocalSrc)) {
                /* link-local source ip */
                memcpy(&ip_mld_hdr->ip6_src, &ipLinkLocalSrc, sizeof(vtss_ipv6_t));
            }
            /* destination ip */
            memcpy(&ip_mld_hdr->ip6_dst, join_group_addr, sizeof(vtss_ipv6_t));

            /* Hop-By-Hop */
            ip_mld_hdr->HBHNxtHdr = MLD_IPV6_NEXTHDR_ICMP;
            ip_mld_hdr->HdrExtLen = 0x0;
            ip_mld_hdr->OptNPad[0] = IPMC_IPV6_RTR_ALERT_PREFIX1;
            ip_mld_hdr->OptNPad[1] = IPMC_IPV6_RTR_ALERT_PREFIX2;

            /* MLD */
            /* Listener is now only supported up to MLDv1 */
            ip_mld_hdr->type = IPMC_MLD_MSG_TYPE_V1REPORT;
            ip_mld_hdr->code = 0x0;
            ip_mld_hdr->max_resp_time = 0x0;
            memcpy(&ip_mld_hdr->group, join_group_addr, sizeof(vtss_ipv6_t));

            uip_len = sizeof(ipmc_ip_eth_hdr) + sizeof(ipmc_ip_mld_hdr);
            ip_mld_hdr->PayloadLen = htons(uip_len - sizeof(ipmc_ip_eth_hdr) - IPV6_HDR_FIXED_LEN);

            ip_mld_hdr->checksum = 0;
            ip_mld_hdr->checksum = mld_chksum_tx((mld_ip6_hdr *)ip_mld_hdr, MLD_MIN_OFFSET, (ulong)(ntohs(ip_mld_hdr->PayloadLen) - MLD_MIN_HBH_LEN));
        } else if (version == IPMC_IP_VERSION_IGMP) {
            vtss_ipv4_t ip4addr = 0;

            ip_igmp_hdr = (ipmc_ip_igmp_hdr *)&uip_buf[sizeof(ipmc_ip_eth_hdr)];

            /* ETHERNET */
            if ((compat == VTSS_IPMC_COMPAT_MODE_AUTO) || (compat == VTSS_IPMC_COMPAT_MODE_SFM)) {
                if (proxy) {    /* Current PROXY doesn't run in SFM mode */
                    ipmc_lib_packet_dst_mac(eth_hdr->dest.addr, join_group_addr, version);
                } else {
                    memcpy(eth_hdr->dest.addr, igmp_sfm_rpt_mac, sizeof(ipmc_eth_addr));
                }
            } else {
                ipmc_lib_packet_dst_mac(eth_hdr->dest.addr, join_group_addr, version);
            }
            if (!ipmc_lib_get_system_mgmt_macx(mac_addr)) {
                memset(eth_hdr->src.addr, 0x0, sizeof(uchar) * 6);
            } else {
                memcpy(eth_hdr->src.addr, mac_addr, sizeof(uchar) * 6);
            }
            eth_hdr->type = htons(IP_MULTICAST_V4_ETHER_TYPE);

            /* IPv4 */
            ip_igmp_hdr->vhl = (IP_MULTICAST_V4_IP_VERSION << 4);
            ip_igmp_hdr->vhl |= sizeof(igmp_ip4_hdr) / 4;
            ip_igmp_hdr->tos = 0;
            ip_igmp_hdr->seq_id = ip_igmp_hdr->offset = 0;
            ip_igmp_hdr->ttl = IPMC_IPHDR_HOPLIMIT;
            ip_igmp_hdr->proto = IP_MULTICAST_IGMP_PROTO_ID;
            ip_igmp_hdr->router_option[0] = IPMC_IPV4_RTR_ALERT_PREFIX1;
            ip_igmp_hdr->router_option[1] = IPMC_IPV4_RTR_ALERT_PREFIX2;

            /* get src address */
            if (ipmc_lib_get_ipintf_igmp_adrs(entry, &ip4addr)) {
                ip4addr = htonl(ip4addr);
            }
            memcpy(&ip_igmp_hdr->ip4_src, &ip4addr, sizeof(ipmcv4addr));
            /* destination ip */
            memcpy(&ip_igmp_hdr->ip4_dst, &join_group_addr->addr[12], sizeof(ipmcv4addr));

            /* IGMP */
            if (compat == VTSS_IPMC_COMPAT_MODE_OLD) {
                ip_igmp_hdr->type = IPMC_IGMP_MSG_TYPE_V1JOIN;
            } else {
                /* Listener is now only supported up to IGMPv2 */
                ip_igmp_hdr->type = IPMC_IGMP_MSG_TYPE_V2JOIN;
            }
            ip_igmp_hdr->max_resp_time = 0x0;
            memcpy(&ip_igmp_hdr->group, &join_group_addr->addr[12], sizeof(ipmcv4addr));

            uip_len = sizeof(ipmc_ip_eth_hdr) + sizeof(ipmc_ip_igmp_hdr);

            /* IGMP CheckSum First */
            ip_igmp_hdr->checksum = 0;
            ip_igmp_hdr->checksum = igmp_chksum_tx((ushort *)&ip_igmp_hdr->type, sizeof(ipmc_ip_igmp_hdr) - sizeof(igmp_ip4_hdr));
            /* IPv4 CheckSum Later */
            ip_igmp_hdr->PayloadLen = htons(uip_len - sizeof(ipmc_ip_eth_hdr));
            ip_igmp_hdr->ip_chksum = 0; /* init. */
            ip_igmp_hdr->ip_chksum = igmp_chksum_tx((ushort *)ip_igmp_hdr, sizeof(igmp_ip4_hdr));
        } else {
            return VTSS_RC_ERROR;
        }

        /* not entirely a snooped frame, but in this context this is what we want */
        T_D("\n\rIPMC transmit COMPAT-%d join/report packet", compat);
        T_D_HEX(&uip_buf[0], uip_len);
        if (ipmc_lib_packet_tx(dst_port_mask,
                               force_untag,
                               FALSE,
                               VTSS_PORT_NO_NONE,
                               (is_mvr ? IPMC_PKT_SRC_MVR : IPMC_PKT_SRC_SNP),
                               vid,
                               0,
                               0,
                               VTSS_GLAG_NO_NONE,
                               &uip_buf[0],
                               uip_len) != VTSS_OK) {
            T_D("Failure in ipmc_lib_packet_tx_join_report for VLAN %d", vid);
        }
    } /* tx_flag */

    return VTSS_OK;
}

static BOOL _ipmc_lib_packet_validate_query(u8 *macadr, vtss_ipv6_t *adr6, ipmcv4addr *adr4)
{
    u32 ip2mac;

    if (!macadr) {
        return FALSE;
    }

    ip2mac = 0;
    if (adr6) {
        if (!memcmp(macadr, mld_all_node_mac, sizeof(mac_addr_t))) {
            return TRUE;
        }

        memcpy((u8 *)&ip2mac, &adr6->addr[12], sizeof(ipmcv4addr));
    } else if (adr4) {
        if (!memcmp(macadr, igmp_all_node_mac, sizeof(mac_addr_t))) {
            return TRUE;
        }

        memcpy((u8 *)&ip2mac, adr4, sizeof(ipmcv4addr));
    } else {
        return FALSE;
    }

    if (!ip2mac) {
        return FALSE;
    }

    ip2mac = ntohl(ip2mac);
    if (adr4) {
        ip2mac = ip2mac << IPMC_IP2MAC_V4SHIFT_LEN;
        ip2mac = ip2mac >> IPMC_IP2MAC_V4SHIFT_LEN;

        if ((*(macadr + 0) != IPMC_IP2MAC_V4MAC_ARRAY0) ||
            (*(macadr + 1) != IPMC_IP2MAC_V4MAC_ARRAY1) ||
            (*(macadr + 2) != IPMC_IP2MAC_V4MAC_ARRAY2) ||
            (*(macadr + 3) != ((ip2mac >> IPMC_IP2MAC_ARRAY3_SHIFT_LEN) & IPMC_IP2MAC_ARRAY_MASK)) ||
            (*(macadr + 4) != ((ip2mac >> IPMC_IP2MAC_ARRAY4_SHIFT_LEN) & IPMC_IP2MAC_ARRAY_MASK)) ||
            (*(macadr + 5) != ((ip2mac >> IPMC_IP2MAC_ARRAY5_SHIFT_LEN) & IPMC_IP2MAC_ARRAY_MASK))) {
            return FALSE;
        }
    } else {
        ip2mac = ip2mac << IPMC_IP2MAC_V6SHIFT_LEN;
        ip2mac = ip2mac >> IPMC_IP2MAC_V6SHIFT_LEN;

        if ((*(macadr + 0) != IPMC_IP2MAC_V6MAC_ARRAY0) ||
            (*(macadr + 1) != IPMC_IP2MAC_V6MAC_ARRAY1) ||
            (*(macadr + 2) != ((ip2mac >> IPMC_IP2MAC_ARRAY2_SHIFT_LEN) & IPMC_IP2MAC_ARRAY_MASK)) ||
            (*(macadr + 3) != ((ip2mac >> IPMC_IP2MAC_ARRAY3_SHIFT_LEN) & IPMC_IP2MAC_ARRAY_MASK)) ||
            (*(macadr + 4) != ((ip2mac >> IPMC_IP2MAC_ARRAY4_SHIFT_LEN) & IPMC_IP2MAC_ARRAY_MASK)) ||
            (*(macadr + 5) != ((ip2mac >> IPMC_IP2MAC_ARRAY5_SHIFT_LEN) & IPMC_IP2MAC_ARRAY_MASK))) {
            return FALSE;
        }
    }

    return TRUE;
}

static vtss_rc ipmc_lib_packet_parse(ipmc_intf_entry_t *entry, const uchar *const pkt, vtss_ipv6_t *src_ip_addr, vtss_ipv6_t *dst_ip_addr, ulong *offset)
{
    vtss_rc                     rc = VTSS_RC_ERROR;

    ipmc_ip_eth_hdr             *etherHdr;

    igmp_ip4_hdr                *ip4Hdr;
    mld_ip6_hdr                 *ip6Hdr;
    mld_ip6_hbh_hdr             *ip6HbH;

    ipmc_mld_packet_t           *mld = NULL;
    ipmc_igmp_packet_t          *igmp = NULL;

    uchar                       hdrVer = 0, hdrTc = 0, hdrTmp = 0, msgType = 0;
    ulong                       hdrFl = 0, VerTcFl = 0;
    ulong                       ip_payload_len = 0, ip_ext_hdr_len = 0;

    etherHdr = (ipmc_ip_eth_hdr *) pkt;
    *offset = sizeof(ipmc_ip_eth_hdr);
    if (ntohs(etherHdr->type) == IP_MULTICAST_V6_ETHER_TYPE) {
        ip4Hdr = NULL;
        ip6Hdr = (mld_ip6_hdr *) (pkt + *offset);
        VerTcFl = ntohl(ip6Hdr->VerTcFl);
        hdrVer = (uchar) (VerTcFl >> 28);

        /* Various Checking Conditions */
        if (hdrVer != IP_MULTICAST_V6_IP_VERSION) {
            rc = IPMC_ERROR_PKT_VERSION;
            return rc;
        }
        if (ip6Hdr->NxtHdr != MLD_IPV6_NEXTHDR_OPT_HBH) {
            rc = IPMC_ERROR_PKT_FORMAT;
            return rc;
        }

        ip_payload_len = (ulong) ntohs(ip6Hdr->PayloadLen);

        /* Check Hop-by-Hop Option */
        *offset = *offset + IPV6_HDR_FIXED_LEN;
        ip6HbH = (mld_ip6_hbh_hdr *) (pkt + *offset);
        if (ip6HbH->NextHdr != MLD_IPV6_NEXTHDR_ICMP) {
            rc = IPMC_ERROR_PKT_FORMAT;
            return rc;
        } else {
            uchar ExtOptIdx = 0;
            /* Check Router Alert Option */
            do {
                if ((ip6HbH->OptNPad[ExtOptIdx] == IPMC_IPV6_RTR_ALERT_PREFIX1) &&
                    (ip6HbH->OptNPad[ExtOptIdx + 1] == IPMC_IPV6_RTR_ALERT_PREFIX2)) {
                    /*
                        0       Datagram contains a Multicast Listener Discovery message [RFC-2710].
                        1       Datagram contains RSVP message.
                        2       Datagram contains an Active Networks message.
                        3-65535 Reserved to IANA for future use.
                     */
                    if (ip6HbH->OptNPad[ExtOptIdx + 2] || ip6HbH->OptNPad[ExtOptIdx + 3]) {
                        rc = IPMC_ERROR_PKT_CONTENT;
                        return rc;
                    }

                    /* Found 0x05020000 */
                    break;
                }

                ExtOptIdx++;
            } while (ExtOptIdx < ip6HbH->HdrExtLen);

            if (ip6HbH->HdrExtLen) {
                ip_ext_hdr_len = (ulong)(ip6HbH->HdrExtLen);
            } else {
                ip_ext_hdr_len = MLD_MIN_HBH_LEN;
            }

            *offset = *offset + ip_ext_hdr_len;
            mld = (ipmc_mld_packet_t *) (pkt + *offset);
        }

        if (ip6Hdr->ip6_dst.addr[0] != 0xFF) {
            if (mld->common.type != IPMC_MLD_MSG_TYPE_DONE) {
                T_D("\n\rReceived IPMC6 frames with ANY ADDRESS");
            } else {
                entry->param.stats.mld_error_pkt++;
                rc = IPMC_ERROR_PKT_ADDRESS;
                return rc;
            }
        }

        if (mld->common.type != IPMC_MLD_MSG_TYPE_QUERY) {
            ipmc_prefix_t pfx_reserved, pfx_check;
            vtss_ipv6_t   *addr6 = NULL;

            if (mld->common.type == IPMC_MLD_MSG_TYPE_V2REPORT) {
                addr6 = &mld->sfminfo.sfm_report.group_address;
            } else {
                addr6 = &mld->sfminfo.usual.group_address;
            }

            if ((addr6 == NULL) || (addr6->addr[0] != 0xFF)) {
                entry->param.stats.mld_error_pkt++;
                rc = IPMC_ERROR_PKT_ADDRESS;
                return rc;
            }
            pfx_reserved.len = pfx_check.len = IPMC_ADDR_MAX_BIT_LEN;
            memcpy(&pfx_check.addr.array.prefix, addr6, sizeof(vtss_ipv6_t));

            ipmc_lib_get_all_router_ipv6_addr(&pfx_reserved.addr.array.prefix);
            if (ipmc_lib_prefix_matching(IPMC_IP_VERSION_MLD, TRUE, &pfx_reserved, &pfx_check)) {
                rc = IPMC_ERROR_PKT_RESERVED;
                return rc;
            }

            ipmc_lib_get_all_node_ipv6_addr(&pfx_reserved.addr.array.prefix);
            if (ipmc_lib_prefix_matching(IPMC_IP_VERSION_MLD, TRUE, &pfx_reserved, &pfx_check)) {
                rc = IPMC_ERROR_PKT_RESERVED;
                return rc;
            }
        }

        msgType = mld->common.type;
    } else {
        if (ntohs(etherHdr->type) != IP_MULTICAST_V4_ETHER_TYPE) {
            rc = IPMC_ERROR_PKT_VERSION;
            return rc;
        }

        ip4Hdr = (igmp_ip4_hdr *) (pkt + *offset);
        ip6Hdr = NULL;

        hdrTmp = ip4Hdr->vhl;
        hdrVer = hdrTmp >> 4;

        /* Various Checking Conditions */
        if (hdrVer != IP_MULTICAST_V4_IP_VERSION) {
            rc = IPMC_ERROR_PKT_VERSION;
            return rc;
        }
        if (ip4Hdr->proto != IP_MULTICAST_IGMP_PROTO_ID) {
            rc = IPMC_ERROR_PKT_FORMAT;
            return rc;
        }
        if (ip4Hdr->ttl != IPMC_IPHDR_HOPLIMIT) {
            rc = IPMC_ERROR_PKT_FORMAT;
            return rc;
        }
        if ((ip4Hdr->router_option[0] == IPMC_IPV4_RTR_ALERT_PREFIX1) &&
            (ip4Hdr->router_option[1] == IPMC_IPV4_RTR_ALERT_PREFIX2)) {
            /*
                0       Router shall examine packet. [RFC-2113]
                1-65535 Reserved for future use.
             */
            if (ip4Hdr->router_option[2] || ip4Hdr->router_option[3]) {
                rc = IPMC_ERROR_PKT_CONTENT;
                return rc;
            }
        }

        ip_payload_len = (ulong) ntohs(ip4Hdr->PayloadLen);
        ip_ext_hdr_len = (hdrTmp & 0xF) * 4;

        *offset = *offset + ip_ext_hdr_len;
        igmp = (ipmc_igmp_packet_t *) (pkt + *offset);

        if ((ip4Hdr->ip4_dst.addr[0] < 0xE0) || (ip4Hdr->ip4_dst.addr[0] > 0xEF)) {
            if (igmp->common.type != IPMC_IGMP_MSG_TYPE_LEAVE) {
                T_D("\n\rReceived IPMC4 frames with ANY ADDRESS");
            } else {
                entry->param.stats.igmp_error_pkt++;
                rc = IPMC_ERROR_PKT_ADDRESS;
                return rc;
            }
        }

        if (igmp->common.type != IPMC_IGMP_MSG_TYPE_QUERY) {
            if (igmp->common.type == IPMC_IGMP_MSG_TYPE_V3JOIN) {
                if ((igmp->sfminfo.sfm_report.group_address.addr[0] < 0xE0) ||
                    (igmp->sfminfo.sfm_report.group_address.addr[0] > 0xEF)) {
                    entry->param.stats.igmp_error_pkt++;
                    rc = IPMC_ERROR_PKT_ADDRESS;
                    return rc;
                }

                /* Always filter 224.0.0.X */
                if ((igmp->sfminfo.sfm_report.group_address.addr[0] == 0xE0) &&
                    (igmp->sfminfo.sfm_report.group_address.addr[1] == 0x0) &&
                    (igmp->sfminfo.sfm_report.group_address.addr[2] == 0x0)) {
                    entry->param.stats.igmp_error_pkt++;
                    rc = IPMC_ERROR_PKT_RESERVED;
                    return rc;
                }
            } else {
                if ((igmp->sfminfo.usual.group_address.addr[0] < 0xE0) ||
                    (igmp->sfminfo.usual.group_address.addr[0] > 0xEF)) {
                    entry->param.stats.igmp_error_pkt++;
                    rc = IPMC_ERROR_PKT_ADDRESS;
                    return rc;
                }

                /* Always filter 224.0.0.X */
                if ((igmp->sfminfo.usual.group_address.addr[0] == 0xE0) &&
                    (igmp->sfminfo.usual.group_address.addr[1] == 0x0) &&
                    (igmp->sfminfo.usual.group_address.addr[2] == 0x0)) {
                    entry->param.stats.igmp_error_pkt++;
                    rc = IPMC_ERROR_PKT_RESERVED;
                    return rc;
                }
            }
        }

        msgType = igmp->common.type;
    } /* if (ntohs(etherHdr->type) == IP_MULTICAST_V6_ETHER_TYPE) */

    switch ( msgType ) {
    case IPMC_MLD_MSG_TYPE_QUERY:
        /*
            RFC3810-5.1.14
            If a node (router or host) receives a Query message with
            the IPv6 Source Address set to the unspecified address (::), or any
            other address that is not a valid IPv6 link-local address, it MUST
            silently discard the message and SHOULD log a warning.
        */
        if (ip6Hdr != NULL) { /* avoid LINT warning */
            if (ipmc_lib_isaddr6_all_zero((vtss_ipv6_t *)&ip6Hdr->ip6_src)) {
                entry->param.stats.mld_error_pkt++;
                rc = IPMC_ERROR_PKT_ADDRESS;
                return rc;
            }

            if (ip6Hdr->HopLimit != IPMC_IPHDR_HOPLIMIT) {
                rc = IPMC_ERROR_PKT_CONTENT;
                return rc;
            }
            if (IPMCLIB_IS_LINKLOCAL(ip6Hdr->ip6_src) == FALSE) {
                entry->param.stats.mld_error_pkt++;
                rc = IPMC_ERROR_PKT_ADDRESS;
                return rc;
            }
        }

        if (mld && !_ipmc_lib_packet_validate_query((u8 *)pkt, &mld->sfminfo.usual.group_address, NULL)) {
            rc = IPMC_ERROR_PKT_FORMAT;
            return rc;
        }

        break;
    case IPMC_MLD_MSG_TYPE_V1REPORT:
    case IPMC_MLD_MSG_TYPE_V2REPORT:
    case IPMC_MLD_MSG_TYPE_DONE:
        if (ip6Hdr != NULL) { /* avoid LINT warning */
            if (ip6Hdr->HopLimit != IPMC_IPHDR_HOPLIMIT) {
                rc = IPMC_ERROR_PKT_CONTENT;
                return rc;
            }
            if (IPMCLIB_IS_LINKLOCAL(ip6Hdr->ip6_src) == FALSE) {
                entry->param.stats.mld_error_pkt++;
                rc = IPMC_ERROR_PKT_ADDRESS;
                return rc;
            }
        }

        break;
    case IPMC_IGMP_MSG_TYPE_QUERY:
        if (igmp && !_ipmc_lib_packet_validate_query((u8 *)pkt, NULL, &igmp->sfminfo.usual.group_address)) {
            rc = IPMC_ERROR_PKT_FORMAT;
            return rc;
        }

        break;
    case IPMC_IGMP_MSG_TYPE_V1JOIN:
    case IPMC_IGMP_MSG_TYPE_V2JOIN:
    case IPMC_IGMP_MSG_TYPE_V3JOIN:
    case IPMC_IGMP_MSG_TYPE_LEAVE:

        break;
    default: /* Unknown type should be ignored */
        rc = IPMC_ERROR_PKT_CONTENT;
        return rc;
    }

    /* Checksum Check */
    switch ( ntohs(etherHdr->type) ) {
    case IP_MULTICAST_V6_ETHER_TYPE:
        if ((ip6Hdr != NULL) && (mld != NULL)) { /* avoid LINT warning */
            if (!mld_chksum_rx(ip6Hdr, (*offset - sizeof(ipmc_ip_eth_hdr)), (ip_payload_len - ip_ext_hdr_len), mld->common.checksum)) {
                /* drop packet */
                T_D("\n\rA MLD control packet with error checksum");
                rc = IPMC_ERROR_PKT_CHECKSUM;
                return rc;
            }

            hdrTc = (uchar) ((VerTcFl >> 20) & 0xFF);
            hdrFl = VerTcFl << 12;
            memcpy(src_ip_addr, &ip6Hdr->ip6_src, sizeof(vtss_ipv6_t));
            memcpy(dst_ip_addr, &ip6Hdr->ip6_dst, sizeof(vtss_ipv6_t));
            T_D("\n\rGot MLD-VID%u with TC=%d, FL=%d from VerTcFl=%d",
                entry->param.vid, (int)hdrTc, hdrFl, VerTcFl);
        }

        break;
    case IP_MULTICAST_V4_ETHER_TYPE:
        if ((ip4Hdr != NULL) && (igmp != NULL)) { /* avoid LINT warning */
            if (!igmp_chksum_rx(ip4Hdr, (*offset - sizeof(ipmc_ip_eth_hdr)), (ip_payload_len - ip_ext_hdr_len), igmp->common.checksum)) {
                /* drop packet */
                T_D("\n\rAn IGMP control packet with error checksum");
                rc = IPMC_ERROR_PKT_CHECKSUM;
                return rc;
            }

            memcpy(&src_ip_addr->addr[12], &ip4Hdr->ip4_src, sizeof(ipmcv4addr));
            memcpy(&dst_ip_addr->addr[12], &ip4Hdr->ip4_dst, sizeof(ipmcv4addr));
            T_D("\n\rGot IGMP-VID%u with TC=%d, FL=%d from VerTcFl=%d",
                entry->param.vid, (int)hdrTc, hdrFl, VerTcFl);
        }

        break;
    default:
        rc = IPMC_ERROR_PKT_INGRESS_FILTER;
        return rc;
    }

    return VTSS_OK;
}

vtss_rc ipmc_lib_packet_rx(ipmc_intf_entry_t *entry, const uchar *const frame, const vtss_packet_rx_info_t *const rx_info, ipmc_pkt_attribute_t *atr)
{
    ipmc_mld_packet_t           *mld;
    ipmc_igmp_packet_t          *igmp;

    vtss_ipv6_t                 src_ip_addr, dst_ip_addr;
    ushort                      max_resp_time, no_of_sources;
    u32                         offset, ipmc_pkt_len;
    u8                          msgType, qrv;

    vtss_rc                     rcv_retVal;
    ipmc_ip_version_t           rcv_ipmc_version;

    if (!ipmc_lib_pkt_done_init) {
        return VTSS_RC_ERROR;
    }

    if (!entry || !entry->op_state) {
        return IPMC_ERROR_VLAN_NOT_ACTIVE;
    }

    rcv_ipmc_version = entry->ipmc_version;
    switch ( rcv_ipmc_version ) {
    case IPMC_IP_VERSION_IGMP:
    case IPMC_IP_VERSION_MLD:

        break;
    case IPMC_IP_VERSION_IPV4Z:
    case IPMC_IP_VERSION_IPV6Z:
    case IPMC_IP_VERSION_DNS:
    default:
        return IPMC_ERROR_PKT_FORMAT;
    }

    /* Parsing for IP */
    memset(&src_ip_addr, 0x0, sizeof(vtss_ipv6_t));
    memset(&dst_ip_addr, 0x0, sizeof(vtss_ipv6_t));
    offset = 0;
    rcv_retVal = ipmc_lib_packet_parse(entry, frame, &src_ip_addr, &dst_ip_addr, &offset);

    if (rcv_retVal != VTSS_OK) {
        return rcv_retVal;
    } else {
        ipmc_pkt_len = 0;
        msgType = qrv = 0;
        max_resp_time = no_of_sources = 0;
        memcpy(&atr->src_ip_addr, &src_ip_addr, sizeof(vtss_ipv6_t));
        atr->offset = offset;

        if (rcv_ipmc_version == IPMC_IP_VERSION_MLD) {
            mld = (ipmc_mld_packet_t *) (frame + offset);
            atr->msgType = msgType = mld->common.type;
            atr->ipmc_pkt_len = ipmc_pkt_len = rx_info->length - offset;

            if (msgType == IPMC_MLD_MSG_TYPE_QUERY) {
                if ((ipmc_pkt_len > MLD_GEN_MIN_PAYLOAD_LEN) && (ipmc_pkt_len < MLD_SFM_MIN_PAYLOAD_LEN)) {
                    return IPMC_ERROR_PKT_FORMAT;
                }
            }

            if (msgType == IPMC_MLD_MSG_TYPE_V2REPORT) {
                memcpy(&atr->group_addr, &mld->sfminfo.sfm_report.group_address, sizeof(vtss_ipv6_t));
            } else {
                memcpy(&atr->group_addr, &mld->sfminfo.usual.group_address, sizeof(vtss_ipv6_t));
            }

            max_resp_time = ntohs(mld->common.max_resp_time);
            if ((ipmc_pkt_len >= MLD_SFM_MIN_PAYLOAD_LEN) && (msgType == IPMC_MLD_MSG_TYPE_QUERY)) {
                qrv = mld->sfminfo.sfm_query.resv_s_qrv & 0x7;
                no_of_sources = ntohs(mld->sfminfo.sfm_query.no_of_sources);
            }
        } else {
            igmp_ip4_hdr    *ip4Hdr = (igmp_ip4_hdr *) (frame + sizeof(ipmc_ip_eth_hdr));

            igmp = (ipmc_igmp_packet_t *) (frame + offset);
            atr->msgType = msgType = igmp->common.type;
            atr->ipmc_pkt_len = ipmc_pkt_len = ((ulong) ntohs(ip4Hdr->PayloadLen)) - (offset - sizeof(ipmc_ip_eth_hdr));

            if (msgType == IPMC_IGMP_MSG_TYPE_QUERY) {
                if ((ipmc_pkt_len > IGMP_MIN_PAYLOAD_LEN) && (ipmc_pkt_len < IGMP_SFM_MIN_PAYLOAD_LEN)) {
                    return IPMC_ERROR_PKT_FORMAT;
                }
            }

            if (msgType == IPMC_IGMP_MSG_TYPE_V3JOIN) {
                memcpy(&atr->group_addr.addr[12], &igmp->sfminfo.sfm_report.group_address, sizeof(ipmcv4addr));
            } else {
                memcpy(&atr->group_addr.addr[12], &igmp->sfminfo.usual.group_address, sizeof(ipmcv4addr));
            }

            max_resp_time = igmp->common.max_resp_time;
            if ((ipmc_pkt_len >= IGMP_SFM_MIN_PAYLOAD_LEN) && (msgType == IPMC_IGMP_MSG_TYPE_QUERY)) {
                qrv = igmp->sfminfo.sfm_query.resv_s_qrv & 0x7;
                no_of_sources = ntohs(igmp->sfminfo.sfm_query.no_of_sources);
            }
        }

        atr->max_resp_time = max_resp_time;
        atr->qrv = qrv;
        atr->no_of_sources = no_of_sources;
    }

    return rcv_retVal;
}
