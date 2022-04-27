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
static BOOL                                 ipmc_lib_ptc_done_init = FALSE;

static ipmc_db_ctrl_hdr_t   ipmc_sf_permit_srclist[2][VTSS_PORT_ARRAY_SIZE];
static BOOL ipmc_sf_permit_srclist_created_done = FALSE;
static ipmc_db_ctrl_hdr_t   ipmc_sf_deny_srclist[2][VTSS_PORT_ARRAY_SIZE];
static BOOL ipmc_sf_deny_srclist_created_done = FALSE;
static ipmc_db_ctrl_hdr_t   ipmc_lower_timer_srclist[2][VTSS_PORT_ARRAY_SIZE];
static BOOL ipmc_lower_timer_srclist_created_done = FALSE;

static ipmc_db_ctrl_hdr_t   tmp1_srclist[2];
static BOOL tmp1_srclist_created_done = FALSE;
static ipmc_db_ctrl_hdr_t   tmp2_srclist[2];
static BOOL tmp2_srclist_created_done = FALSE;

static ipmc_db_ctrl_hdr_t   allow_list_tmp4rcv[2];
static BOOL allow_list_tmp4rcv_created = FALSE;
static ipmc_db_ctrl_hdr_t   allow_list_tmp4tick[2];
static BOOL allow_list_tmp4tick_created = FALSE;
static ipmc_db_ctrl_hdr_t   block_list_tmp4rcv[2];
static BOOL block_list_tmp4rcv_created = FALSE;
static ipmc_db_ctrl_hdr_t   block_list_tmp4tick[2];
static BOOL block_list_tmp4tick_created = FALSE;

static ipmc_group_info_t    grp_info_tmp4rcv[2];
static ipmc_group_info_t    grp_info_tmp4tick[2];
static ipmc_group_entry_t   proc_grp_sfm_tmp4rcv[2];
static ipmc_group_entry_t   proc_grp_sfm_tmp4tick[2];


ipmc_db_ctrl_hdr_t *ipmc_lib_get_sf_permit_srclist(u8 is_mvr, u32 port)
{
    if (ipmc_sf_permit_srclist_created_done) {
        return &ipmc_sf_permit_srclist[is_mvr][port];
    }

    return NULL;
}

ipmc_db_ctrl_hdr_t *ipmc_lib_get_sf_deny_srclist(u8 is_mvr, u32 port)
{
    if (ipmc_sf_deny_srclist_created_done) {
        return &ipmc_sf_deny_srclist[is_mvr][port];
    }

    return NULL;
}

BOOL ipmc_lib_listener_set_reporting_timer(u16 *out_timer, u16 in_timer, u16 ref_timeout)
{
    u16 response_timeout;

    if (!out_timer) {
        return FALSE;
    }

    response_timeout = in_timer;
    if (!response_timeout) {
        response_timeout = ref_timeout ? ref_timeout : 0x1;
    }

    response_timeout = rand() % response_timeout;
    if (!response_timeout) {
        response_timeout = 0x1;
    }

    /* restart response timer */
    if ((*out_timer == 0) || (*out_timer > response_timeout)) {
        *out_timer = response_timeout;
    }

    return TRUE;
}

ipmc_send_act_t ipmc_lib_get_sq_ssq_action(BOOL proxy, ipmc_intf_entry_t *entry, u32 port)
{
    ipmc_send_act_t retVal;

    if (!entry) {
        return IPMC_SND_HOLD;
    }

    retVal = IPMC_SND_HOLD;
    /* Querier sends SQ&SSQ */
    if (entry->param.mvr) {
        retVal = IPMC_SND_GO;
    } else {
        if (proxy) {
            if (ipmc_lib_get_port_rpstatus(entry->ipmc_version, port)) {
                retVal = IPMC_SND_GO_HOLD;
            } else {
                retVal = IPMC_SND_GO;
            }
        } else {
            if (entry->param.querier.state != IPMC_QUERIER_IDLE) {
                retVal = IPMC_SND_GO;
            }
        }
    }

    return retVal;
}

BOOL ipmc_lib_get_grp_sfm_tmp4rcv(u8 is_mvr, ipmc_group_entry_t **grp)
{
    if (!grp || !(*grp)) {
        return FALSE;
    }

    *grp = &proc_grp_sfm_tmp4rcv[is_mvr];
    return TRUE;
}

BOOL ipmc_lib_get_grp_sfm_tmp4tick(u8 is_mvr, ipmc_group_entry_t **grp)
{
    if (!grp || !(*grp)) {
        return FALSE;
    }

    *grp = &proc_grp_sfm_tmp4tick[is_mvr];
    return TRUE;
}

void ipmc_lib_proc_grp_sfm_tmp4rcv(u8 is_mvr, BOOL clear, BOOL dosf, ipmc_group_entry_t *grp)
{
    if (!allow_list_tmp4rcv_created) {
        if (!IPMC_LIB_DB_TAKE("SNP_FWD_LST_4RCV", &allow_list_tmp4rcv[0],
                              IPMC_NO_OF_SUPPORTED_SRCLIST,
                              sizeof(ipmc_sfm_srclist_t),
                              ipmc_lib_srclist_cmp_func)) {
            T_W("IPMC_LIB_DB_TAKE(allow_list_tmp4rcv[SNP]) failed");
            return;
        }
        if (!IPMC_LIB_DB_TAKE("MVR_FWD_LST_4RCV", &allow_list_tmp4rcv[1],
                              IPMC_NO_OF_SUPPORTED_SRCLIST,
                              sizeof(ipmc_sfm_srclist_t),
                              ipmc_lib_srclist_cmp_func)) {
            T_W("IPMC_LIB_DB_TAKE(allow_list_tmp4rcv[MVR]) failed");
            if (!IPMC_LIB_DB_GIVE(&allow_list_tmp4rcv[0])) {
                T_W("IPMC_LIB_DB_GIVE(allow_list_tmp4rcv[SNP]) failed");
            }

            return;
        }

        allow_list_tmp4rcv_created = TRUE;
    }
    if (!block_list_tmp4rcv_created) {
        if (!IPMC_LIB_DB_TAKE("SNP_BLK_LST_4RCV", &block_list_tmp4rcv[0],
                              IPMC_NO_OF_SUPPORTED_SRCLIST,
                              sizeof(ipmc_sfm_srclist_t),
                              ipmc_lib_srclist_cmp_func)) {
            T_W("IPMC_LIB_DB_TAKE(block_list_tmp4rcv[SNP]) failed");
            return;
        }
        if (!IPMC_LIB_DB_TAKE("MVR_BLK_LST_4RCV", &block_list_tmp4rcv[1],
                              IPMC_NO_OF_SUPPORTED_SRCLIST,
                              sizeof(ipmc_sfm_srclist_t),
                              ipmc_lib_srclist_cmp_func)) {
            T_W("IPMC_LIB_DB_TAKE(block_list_tmp4rcv[MVR]) failed");
            if (!IPMC_LIB_DB_GIVE(&block_list_tmp4rcv[0])) {
                T_W("IPMC_LIB_DB_GIVE(block_list_tmp4rcv[SNP]) failed");
            }

            return;
        }

        block_list_tmp4rcv_created = TRUE;
    }

    if (clear) {
        if (!ipmc_lib_srclist_clear(&allow_list_tmp4rcv[is_mvr], 180)) {
            T_D("Clear allow_list_tmp4rcv failed!");
        }
        if (!ipmc_lib_srclist_clear(&block_list_tmp4rcv[is_mvr], 170)) {
            T_D("Clear block_list_tmp4rcv failed!");
        }

        return;
    }

    if (!grp || !grp->info) {
        return;
    }

    if (dosf) {
        if (ipmc_lib_srclist_struct_copy(
                grp,
                &allow_list_tmp4rcv[is_mvr],
                grp->info->db.ipmc_sf_do_forward_srclist,
                VTSS_IPMC_SFM_OP_PORT_ANY) != TRUE) {
            T_D("vtss_ipmc_data_struct_copy() failed");
        }
        if (ipmc_lib_srclist_struct_copy(
                grp,
                &block_list_tmp4rcv[is_mvr],
                grp->info->db.ipmc_sf_do_not_forward_srclist,
                VTSS_IPMC_SFM_OP_PORT_ANY) != TRUE) {
            T_D("vtss_ipmc_data_struct_copy() failed");
        }
    } else {
        if (!ipmc_lib_srclist_clear(&allow_list_tmp4rcv[is_mvr], 185)) {
            T_D("Clear allow_list_tmp4rcv failed!");
        }
        if (!ipmc_lib_srclist_clear(&block_list_tmp4rcv[is_mvr], 175)) {
            T_D("Clear block_list_tmp4rcv failed!");
        }
    }
    memcpy(&grp_info_tmp4rcv[is_mvr].db, &grp->info->db, sizeof(ipmc_group_db_t));
//    grp_info_tmp4rcv[is_mvr].db.ipmc_sf_do_forward_srclist = &allow_list_tmp4rcv[is_mvr];
//    grp_info_tmp4rcv[is_mvr].db.ipmc_sf_do_not_forward_srclist = &block_list_tmp4rcv[is_mvr];
//    proc_grp_sfm_tmp4rcv[is_mvr].info = &grp_info_tmp4rcv[is_mvr];
}

void ipmc_lib_proc_grp_sfm_tmp4tick(u8 is_mvr, BOOL clear, BOOL dosf, ipmc_group_entry_t *grp)
{
    if (!allow_list_tmp4tick_created) {
        if (!IPMC_LIB_DB_TAKE("SNP_FWD_LST_4TCK", &allow_list_tmp4tick[0],
                              IPMC_NO_OF_SUPPORTED_SRCLIST,
                              sizeof(ipmc_sfm_srclist_t),
                              ipmc_lib_srclist_cmp_func)) {
            T_W("IPMC_LIB_DB_TAKE(allow_list_tmp4tick[SNP]) failed");
            return;
        }
        if (!IPMC_LIB_DB_TAKE("MVR_FWD_LST_4TCK", &allow_list_tmp4tick[1],
                              IPMC_NO_OF_SUPPORTED_SRCLIST,
                              sizeof(ipmc_sfm_srclist_t),
                              ipmc_lib_srclist_cmp_func)) {
            T_W("IPMC_LIB_DB_TAKE(allow_list_tmp4tick[MVR]) failed");
            if (!IPMC_LIB_DB_GIVE(&allow_list_tmp4tick[0])) {
                T_W("IPMC_LIB_DB_GIVE(allow_list_tmp4tick[SNP]) failed");
            }

            return;
        }

        allow_list_tmp4tick_created = TRUE;
    }
    if (!block_list_tmp4tick_created) {
        if (!IPMC_LIB_DB_TAKE("SNP_BLK_LST_4TCK", &block_list_tmp4tick[0],
                              IPMC_NO_OF_SUPPORTED_SRCLIST,
                              sizeof(ipmc_sfm_srclist_t),
                              ipmc_lib_srclist_cmp_func)) {
            T_W("IPMC_LIB_DB_TAKE(block_list_tmp4tick[SNP]) failed");
            return;
        }
        if (!IPMC_LIB_DB_TAKE("MVR_BLK_LST_4TCK", &block_list_tmp4tick[1],
                              IPMC_NO_OF_SUPPORTED_SRCLIST,
                              sizeof(ipmc_sfm_srclist_t),
                              ipmc_lib_srclist_cmp_func)) {
            T_W("IPMC_LIB_DB_TAKE(block_list_tmp4tick[MVR]) failed");
            if (!IPMC_LIB_DB_GIVE(&block_list_tmp4tick[0])) {
                T_W("IPMC_LIB_DB_GIVE(block_list_tmp4tick[SNP]) failed");
            }

            return;
        }

        block_list_tmp4tick_created = TRUE;
    }

    if (clear) {
        if (!ipmc_lib_srclist_clear(&allow_list_tmp4tick[is_mvr], 160)) {
            T_D("Clear allow_list_tmp4tick failed!");
        }
        if (!ipmc_lib_srclist_clear(&block_list_tmp4tick[is_mvr], 150)) {
            T_D("Clear block_list_tmp4tick failed!");
        }

        return;
    }

    if (!grp || !grp->info) {
        return;
    }

    if (ipmc_lib_srclist_struct_copy(
            grp,
            &allow_list_tmp4tick[is_mvr],
            grp->info->db.ipmc_sf_do_forward_srclist,
            VTSS_IPMC_SFM_OP_PORT_ANY) != TRUE) {
        T_D("vtss_ipmc_data_struct_copy() failed");
    }
    if (ipmc_lib_srclist_struct_copy(
            grp,
            &block_list_tmp4tick[is_mvr],
            grp->info->db.ipmc_sf_do_not_forward_srclist,
            VTSS_IPMC_SFM_OP_PORT_ANY) != TRUE) {
        T_D("vtss_ipmc_data_struct_copy() failed");
    }
    memcpy(&grp_info_tmp4tick[is_mvr].db, &grp->info->db, sizeof(ipmc_group_db_t));
//    grp_info_tmp4tick[is_mvr].db.ipmc_sf_do_forward_srclist = &allow_list_tmp4tick[is_mvr];
//    grp_info_tmp4tick[is_mvr].db.ipmc_sf_do_not_forward_srclist = &block_list_tmp4tick[is_mvr];
//    proc_grp_sfm_tmp4tick[is_mvr].info = &grp_info_tmp4tick[is_mvr];
}

static BOOL ipmc_lib_protocol_update_sublist_timer_pkt(ipmc_intf_entry_t *intf,
                                                       ipmc_db_ctrl_hdr_t *srct,
                                                       ipmc_db_ctrl_hdr_t *srclist,
                                                       void *group_record,
                                                       u16 src_num,
                                                       ipmc_time_t *new_timer,
                                                       u8 port)
{
    u16                 idx;
    u32                 v, local_port_cnt;
    ipmc_sfm_srclist_t  *element, *check, *check_bak;

    if (!srclist || !intf) {
        return FALSE;
    }

    if (IPMC_LIB_DB_GET_COUNT(srclist) == 0) {
        return TRUE;
    }

    if (!IPMC_MEM_SYSTEM_MTAKE(check, sizeof(ipmc_sfm_srclist_t))) {
        return FALSE;
    }
    check_bak = check;

    local_port_cnt = ipmc_lib_get_system_local_port_cnt();
    for (idx = 0; idx < src_num; idx++) {
        check = check_bak;
        ipmc_lib_srclist_prepare(intf, check, group_record, idx, port);

        if (((element = ipmc_lib_srclist_adr_get(srclist, check)) != NULL) &&
            VTSS_PORT_BF_GET(element->port_mask, port)) {
            v = local_port_cnt;
            IPMC_TIMER_SRCT_SET(v, srct, element, port, new_timer);
        }
    }

    IPMC_MEM_SYSTEM_MGIVE(check_bak);
    return TRUE;
}

static BOOL ipmc_lib_protocol_update_sublist_timer_set(ipmc_db_ctrl_hdr_t *srct,
                                                       ipmc_db_ctrl_hdr_t *srclist,
                                                       ipmc_db_ctrl_hdr_t *operand,
                                                       ipmc_time_t *new_timer,
                                                       u8 port)
{
    u32                 v, local_port_cnt;
    ipmc_sfm_srclist_t  *element, *check;

    if (!srclist || !operand) {
        return FALSE;
    }

    if ((IPMC_LIB_DB_GET_COUNT(srclist) == 0) ||
        (IPMC_LIB_DB_GET_COUNT(operand) == 0)) {
        return TRUE;
    }

    local_port_cnt = ipmc_lib_get_system_local_port_cnt();
    check = NULL;
    IPMC_SRCLIST_WALK(operand, check) {
        if (VTSS_PORT_BF_GET(check->port_mask, port) == FALSE) {
            continue;
        }

        if (((element = ipmc_lib_srclist_adr_get(srclist, check)) != NULL) &&
            VTSS_PORT_BF_GET(element->port_mask, port)) {
            v = local_port_cnt;
            IPMC_TIMER_SRCT_SET(v, srct, element, port, new_timer);
        }
    }

    return TRUE;
}

static BOOL ipmc_lib_protocol_keep_union_list_timer(ipmc_db_ctrl_hdr_t *list,
                                                    ipmc_db_ctrl_hdr_t *operand,
                                                    u8 port)
{
    ipmc_sfm_srclist_t  *element;
    ipmc_sfm_srclist_t  *check;

    if (!list || !operand) {
        return FALSE;
    }

    check = NULL;
    IPMC_SRCLIST_WALK(list, check) {
        if (VTSS_PORT_BF_GET(check->port_mask, port) == FALSE) {
            continue;
        }

        if ((element = ipmc_lib_srclist_adr_get(operand, check)) != NULL) {
            ipmc_lib_time_cpy(&check->tmr.srct_timer.t[port], &element->tmr.srct_timer.t[port]);
        }
    }

    return TRUE;
}

static BOOL ipmc_lib_protocol_update_srclist_subset_timer(ipmc_db_ctrl_hdr_t *srct,
                                                          ipmc_db_ctrl_hdr_t *srclist,
                                                          ipmc_db_ctrl_hdr_t *srclist_subset,
                                                          ipmc_time_t *new_timer,
                                                          u8 port)
{
    u32                 v, local_port_cnt;
    ipmc_sfm_srclist_t  *element, *check;

    if (!srclist || !srclist_subset) {
        return FALSE;
    }

    if ((IPMC_LIB_DB_GET_COUNT(srclist) == 0) ||
        (IPMC_LIB_DB_GET_COUNT(srclist_subset) == 0)) {
        return TRUE;
    }

    local_port_cnt = ipmc_lib_get_system_local_port_cnt();
    check = NULL;
    IPMC_SRCLIST_WALK(srclist_subset, check) {
        if (!VTSS_PORT_BF_GET(check->port_mask, port)) {
            continue;
        }

        if (((element = ipmc_lib_srclist_adr_get(srclist, check)) != NULL) &&
            VTSS_PORT_BF_GET(element->port_mask, port)) {
            v = local_port_cnt;
            IPMC_TIMER_SRCT_SET(v, srct, element, port, new_timer);
        }
    }

    return TRUE;
}

static void ipmc_lib_protocol_lower_source_timer(ipmc_db_ctrl_hdr_t *srct,
                                                 ipmc_group_entry_t *grp,
                                                 ipmc_intf_entry_t *entry,
                                                 ipmc_db_ctrl_hdr_t *src_list,
                                                 u8 ndx)
{
    ipmc_group_db_t     *grp_db;
    ipmc_sfm_srclist_t  *src_list_entry;

    if (!grp || !grp->info || !entry || !src_list) {
        return;
    }

    grp_db = &grp->info->db;
    if (grp_db && IPMC_LIB_GRP_PORT_DO_SFM(grp_db, ndx)) {
        src_list_entry = NULL;
        IPMC_SRCLIST_WALK(grp_db->ipmc_sf_do_not_forward_srclist, src_list_entry) {
            if (!VTSS_PORT_BF_GET(src_list_entry->port_mask, ndx)) {
                continue;
            }

            if (ipmc_lib_srclist_adr_get(src_list, src_list_entry) != NULL) {
                IPMC_TIMER_LLQT_GSET(entry, &src_list_entry->tmr.srct_timer.t[ndx], srct, src_list_entry);
            }
        }

        src_list_entry = NULL;
        IPMC_SRCLIST_WALK(grp_db->ipmc_sf_do_forward_srclist, src_list_entry) {
            if (!VTSS_PORT_BF_GET(src_list_entry->port_mask, ndx)) {
                continue;
            }

            if (ipmc_lib_srclist_adr_get(src_list, src_list_entry) != NULL) {
                IPMC_TIMER_LLQT_GSET(entry, &src_list_entry->tmr.srct_timer.t[ndx], srct, src_list_entry);
            }
        }
    }
}

vtss_rc ipmc_lib_protocol_lower_filter_timer(ipmc_db_ctrl_hdr_t *fltr, ipmc_group_entry_t *grp, ipmc_intf_entry_t *entry, u8 ndx)
{
    ipmc_group_db_t *grp_db;

    if (!fltr || !entry) {
        return VTSS_RC_ERROR;
    }

    if (grp && grp->info) {
        grp_db = &grp->info->db;

        if (IPMC_LIB_GRP_PORT_DO_SFM(grp_db, ndx)) {
            IPMC_TIMER_LLQT_GSET(entry, &grp_db->tmr.fltr_timer.t[ndx], fltr, grp_db);
        }

        return VTSS_OK;
    }

    return VTSS_RC_ERROR;
}

static void _ipmc_lib_sfm_mode_is_include(ipmc_db_ctrl_hdr_t *p,
                                          ipmc_db_ctrl_hdr_t *rxmt,
                                          ipmc_db_ctrl_hdr_t *fltr,
                                          ipmc_db_ctrl_hdr_t *srct,
                                          ipmc_group_entry_t *grp_ptr,
                                          ipmc_intf_entry_t *entry,
                                          u8 src_port,
                                          vtss_ipv6_t *group_address,
                                          void *group_record,
                                          u16 num_of_src,
                                          BOOL proxy, ipmc_time_t *sfm_grp_tmr)
{
    BOOL                asm_chg, grp_is_changed;
    ipmc_group_entry_t  *grp_op, sfm_grp_tmp;
    ipmc_group_info_t   *grp_info;
    ipmc_group_db_t     *grp_db;

    if (!p || !entry || !group_address || !group_record) {
        return;
    }

    asm_chg = grp_is_changed = FALSE;
    if (grp_ptr == NULL) {
        sfm_grp_tmp.vid = entry->param.vid;
        memcpy(&sfm_grp_tmp.group_addr, group_address, sizeof(vtss_ipv6_t));
        sfm_grp_tmp.ipmc_version = entry->ipmc_version;

        grp_ptr = ipmc_lib_group_ptr_get(p, &sfm_grp_tmp);
    }

    if (grp_ptr != NULL) {
        ipmc_lib_proc_grp_sfm_tmp4rcv(IPMC_INTF_IS_MVR_VAL(entry), FALSE, TRUE, grp_ptr);

        grp_info = grp_ptr->info;
        grp_db = &grp_info->db;

        if (IPMC_LIB_GRP_PORT_DO_SFM(grp_db, src_port)) {
            VTSS_PORT_BF_SET(grp_db->ipmc_sf_port_status, src_port, VTSS_IPMC_SF_STATUS_ENABLED);

            if (IPMC_LIB_GRP_PORT_SFM_IN(grp_db, src_port)) {
                /*
                    Router State: INCLUDE (A)
                    Report Received: IS_IN (B)
                    New Router State: INCLUDE (A + B)
                    Actions:(B) = MALI
                */
                /* INCLUDE (A + B) */
                if (!ipmc_lib_srclist_logical_op_pkt(
                        grp_ptr,
                        entry,
                        src_port,
                        VTSS_IPMC_SFM_OP_OR,
                        grp_db->ipmc_sf_do_forward_srclist,
                        group_record,
                        num_of_src, TRUE, srct)) {
                    T_D("ipmc_lib_srclist_logical_op_pkt() failed");
                }

                /* (B) = MALI */
                if (!ipmc_lib_protocol_update_sublist_timer_pkt(
                        entry,
                        srct,
                        grp_db->ipmc_sf_do_forward_srclist,
                        group_record,
                        num_of_src,
                        sfm_grp_tmr,
                        src_port)) {
                    T_D("ipmc_lib_protocol_update_sublist_timer_pkt() failed");
                }
                if (!ipmc_lib_protocol_update_sublist_timer_pkt(
                        entry,
                        srct,
                        grp_db->ipmc_sf_do_not_forward_srclist,
                        group_record,
                        num_of_src,
                        sfm_grp_tmr,
                        src_port)) {
                    T_D("ipmc_lib_protocol_update_sublist_timer_pkt() failed");
                }
            } else {
                /*
                    Router State: EXCLUDE (X, Y)
                    Report Received: IS_IN (A)
                    New Router State: EXCLUDE (X + A, Y - A)
                    Actions:(A) = MALI
                */
                /* EXCLUDE (X + A, Y - A) */
                if (!ipmc_lib_srclist_logical_op_pkt(
                        grp_ptr,
                        entry,
                        src_port,
                        VTSS_IPMC_SFM_OP_OR,
                        grp_db->ipmc_sf_do_forward_srclist,
                        group_record,
                        num_of_src, TRUE, srct)) {
                    T_D("ipmc_lib_srclist_logical_op_pkt() failed");
                }
                if (!ipmc_lib_srclist_logical_op_pkt(
                        grp_ptr,
                        entry,
                        src_port,
                        VTSS_IPMC_SFM_OP_DIFF,
                        grp_db->ipmc_sf_do_not_forward_srclist,
                        group_record,
                        num_of_src, TRUE, srct)) {
                    T_D("ipmc_lib_srclist_logical_op_pkt() failed");
                }

                /* (A) = MALI */
                if (!ipmc_lib_protocol_update_sublist_timer_pkt(
                        entry,
                        srct,
                        grp_db->ipmc_sf_do_forward_srclist,
                        group_record,
                        num_of_src,
                        sfm_grp_tmr,
                        src_port)) {
                    T_D("ipmc_lib_protocol_update_sublist_timer_pkt() failed");
                }
                if (!ipmc_lib_protocol_update_sublist_timer_pkt(
                        entry,
                        srct,
                        grp_db->ipmc_sf_do_not_forward_srclist,
                        group_record,
                        num_of_src,
                        sfm_grp_tmr,
                        src_port)) {
                    T_D("ipmc_lib_protocol_update_sublist_timer_pkt() failed");
                }
            } /* VTSS_IPMC_SF_MODE_INCLUDE | VTSS_IPMC_SF_MODE_EXCLUDE */
        } else {
            /*
                Router State: INCLUDE (0)
                Report Received: IS_IN (B)
                New Router State: INCLUDE (B)
                Actions:(B) = MALI
            */
            asm_chg = TRUE;

            /* INCLUDE (B) */
            VTSS_PORT_BF_SET(grp_db->ipmc_sf_port_status, src_port, VTSS_IPMC_SF_STATUS_ENABLED);
            VTSS_PORT_BF_SET(grp_db->ipmc_sf_port_mode, src_port, VTSS_IPMC_SF_MODE_INCLUDE);
            if (!ipmc_lib_grp_src_list_del4port(srct, src_port, grp_db->ipmc_sf_do_forward_srclist)) {
                T_D("ipmc_lib_grp_src_list_del4port failed");
            }
            if (!ipmc_lib_grp_src_list_del4port(srct, src_port, grp_db->ipmc_sf_do_not_forward_srclist)) {
                T_D("ipmc_lib_grp_src_list_del4port failed");
            }

            if (!ipmc_lib_srclist_logical_op_pkt(
                    grp_ptr,
                    entry,
                    src_port,
                    VTSS_IPMC_SFM_OP_OR,
                    grp_db->ipmc_sf_do_forward_srclist,
                    group_record,
                    num_of_src, TRUE, srct)) {
                T_D("ipmc_lib_srclist_logical_op_pkt() failed");
            }

            /* (B) = MALI */
            if (!ipmc_lib_protocol_update_sublist_timer_pkt(
                    entry,
                    srct,
                    grp_db->ipmc_sf_do_forward_srclist,
                    group_record,
                    num_of_src,
                    sfm_grp_tmr,
                    src_port)) {
                T_D("ipmc_lib_protocol_update_sublist_timer_pkt() failed");
            }
            if (!ipmc_lib_protocol_update_sublist_timer_pkt(
                    entry,
                    srct,
                    grp_db->ipmc_sf_do_not_forward_srclist,
                    group_record,
                    num_of_src,
                    sfm_grp_tmr,
                    src_port)) {
                T_D("ipmc_lib_protocol_update_sublist_timer_pkt() failed");
            }
        } /* VTSS_IPMC_SF_STATUS_ENABLED | VTSS_IPMC_SF_STATUS_DISABLED */

        grp_op = grp_ptr;
        ipmc_lib_srclist_logical_op_cmp(&grp_is_changed, src_port, grp_op->info->db.ipmc_sf_do_forward_srclist, &allow_list_tmp4rcv[IPMC_INTF_IS_MVR_VAL(entry)]);
        if (!grp_is_changed) {
            ipmc_lib_srclist_logical_op_cmp(&grp_is_changed, src_port, grp_op->info->db.ipmc_sf_do_not_forward_srclist, &block_list_tmp4rcv[IPMC_INTF_IS_MVR_VAL(entry)]);
        }
    } else {
        if ((grp_op = ipmc_lib_group_init(entry, p, &sfm_grp_tmp)) != NULL) {
            grp_op->info->grp = grp_op;
            grp_op->info->db.grp = grp_op;

            ipmc_lib_proc_grp_sfm_tmp4rcv(IPMC_INTF_IS_MVR_VAL(entry), FALSE, FALSE, grp_op);

            grp_info = grp_op->info;
            grp_db = &grp_info->db;

            /*
                Router State: INCLUDE (0)
                Report Received: IS_IN (B)
                New Router State: INCLUDE (B)
                Actions:(B) = MALI
            */
            asm_chg = TRUE;

            VTSS_PORT_BF_SET(grp_db->ipmc_sf_port_status, src_port, VTSS_IPMC_SF_STATUS_ENABLED);
            VTSS_PORT_BF_SET(grp_db->ipmc_sf_port_mode, src_port, VTSS_IPMC_SF_MODE_INCLUDE);
            /* INCLUDE (B) */
            if (!ipmc_lib_grp_src_list_del4port(srct, src_port, grp_db->ipmc_sf_do_forward_srclist)) {
                T_D("ipmc_lib_grp_src_list_del4port failed");
            }
            if (!ipmc_lib_grp_src_list_del4port(srct, src_port, grp_db->ipmc_sf_do_not_forward_srclist)) {
                T_D("ipmc_lib_grp_src_list_del4port failed");
            }

            if (!ipmc_lib_srclist_logical_op_pkt(
                    grp_ptr,
                    entry,
                    src_port,
                    VTSS_IPMC_SFM_OP_OR,
                    grp_db->ipmc_sf_do_forward_srclist,
                    group_record,
                    num_of_src, TRUE, srct)) {
                T_D("ipmc_lib_srclist_logical_op_pkt() failed");
            }

            /* (B) = MALI */
            if (!ipmc_lib_protocol_update_sublist_timer_pkt(
                    entry,
                    srct,
                    grp_db->ipmc_sf_do_forward_srclist,
                    group_record,
                    num_of_src,
                    sfm_grp_tmr,
                    src_port)) {
                T_D("ipmc_lib_protocol_update_sublist_timer_pkt() failed");
            }
            if (!ipmc_lib_protocol_update_sublist_timer_pkt(
                    entry,
                    srct,
                    grp_db->ipmc_sf_do_not_forward_srclist,
                    group_record,
                    num_of_src,
                    sfm_grp_tmr,
                    src_port)) {
                T_D("ipmc_lib_protocol_update_sublist_timer_pkt() failed");
            }

            grp_is_changed = TRUE;
        }
    } /* if this group already exists */

    if (asm_chg) {
        if (!ipmc_lib_group_update(p, grp_op)) {
            T_D("update group with chip failed!!!");
        }
    }
    if (grp_is_changed) {
        if (!ipmc_lib_group_sync(p, entry, grp_op, FALSE, PROC4RCV)) {
            T_D("sync group with chip failed!!!");
        }
    }
}

static void _ipmc_lib_sfm_mode_is_exclude(ipmc_db_ctrl_hdr_t *p,
                                          ipmc_db_ctrl_hdr_t *rxmt,
                                          ipmc_db_ctrl_hdr_t *fltr,
                                          ipmc_db_ctrl_hdr_t *srct,
                                          ipmc_group_entry_t *grp_ptr,
                                          ipmc_intf_entry_t *entry,
                                          u8 src_port,
                                          vtss_ipv6_t *group_address,
                                          void *group_record,
                                          u16 num_of_src,
                                          BOOL proxy, ipmc_time_t *sfm_grp_tmr)
{
    BOOL                asm_chg, grp_is_changed;
    u32                 v;
    ipmc_group_entry_t  *grp_op, sfm_grp_tmp;
    ipmc_group_info_t   *grp_info;
    ipmc_group_db_t     *grp_db;

    if (!p || !entry || !group_address || !group_record) {
        return;
    }

    asm_chg = grp_is_changed = FALSE;
    if (grp_ptr == NULL) {
        sfm_grp_tmp.vid = entry->param.vid;
        memcpy(&sfm_grp_tmp.group_addr, group_address, sizeof(vtss_ipv6_t));
        sfm_grp_tmp.ipmc_version = entry->ipmc_version;

        grp_ptr = ipmc_lib_group_ptr_get(p, &sfm_grp_tmp);
    }

    if (grp_ptr != NULL) {
        ipmc_lib_proc_grp_sfm_tmp4rcv(IPMC_INTF_IS_MVR_VAL(entry), FALSE, TRUE, grp_ptr);

        grp_info = grp_ptr->info;
        grp_db = &grp_info->db;

        if (IPMC_LIB_GRP_PORT_DO_SFM(grp_db, src_port)) {
            VTSS_PORT_BF_SET(grp_db->ipmc_sf_port_status, src_port, VTSS_IPMC_SF_STATUS_ENABLED);

            if (IPMC_LIB_GRP_PORT_SFM_IN(grp_db, src_port)) {
                /*
                    Router State: INCLUDE (A)
                    Report Received: IS_EX (B)
                    New Router State: EXCLUDE (A * B, B - A)
                    Actions:(B - A) = 0
                            Delete (A - B)
                            Filter(Group) Timer = MALI
                */
                asm_chg = TRUE;

                VTSS_PORT_BF_SET(grp_db->ipmc_sf_port_mode, src_port, VTSS_IPMC_SF_MODE_EXCLUDE);
                /* EXCLUDE (A * B, B - A) */
                if (!ipmc_lib_grp_src_list_del4port(srct, src_port, grp_db->ipmc_sf_do_not_forward_srclist)) {
                    T_D("ipmc_lib_grp_src_list_del4port failed");
                }
                if (!ipmc_lib_srclist_logical_op_pkt(
                        grp_ptr,
                        entry,
                        src_port,
                        VTSS_IPMC_SFM_OP_OR,
                        grp_db->ipmc_sf_do_not_forward_srclist,
                        group_record,
                        num_of_src, TRUE, srct)) {
                    T_D("ipmc_lib_srclist_logical_op_pkt() failed");
                }
                if (!ipmc_lib_srclist_logical_op_set(
                        grp_ptr,
                        src_port,
                        VTSS_IPMC_SFM_OP_DIFF, TRUE, srct,
                        grp_db->ipmc_sf_do_not_forward_srclist,
                        grp_db->ipmc_sf_do_forward_srclist)) {
                    T_D("ipmc_lib_srclist_logical_op_set() failed");
                }
                if (!ipmc_lib_srclist_logical_op_pkt(
                        grp_ptr,
                        entry,
                        src_port,
                        VTSS_IPMC_SFM_OP_AND,
                        grp_db->ipmc_sf_do_forward_srclist,
                        group_record,
                        num_of_src, TRUE, srct)) {
                    T_D("ipmc_lib_srclist_logical_op_pkt() failed");
                }

                /* (B - A) = 0 */
                if (!ipmc_lib_protocol_update_sublist_timer_set(
                        srct,
                        grp_db->ipmc_sf_do_not_forward_srclist,
                        grp_db->ipmc_sf_do_not_forward_srclist,
                        NULL,
                        src_port)) {
                    T_D("ipmc_lib_protocol_update_sublist_timer_set() failed");
                }
                if (!ipmc_lib_protocol_update_sublist_timer_set(
                        srct,
                        grp_db->ipmc_sf_do_forward_srclist,
                        grp_db->ipmc_sf_do_not_forward_srclist,
                        NULL,
                        src_port)) {
                    T_D("ipmc_lib_protocol_update_sublist_timer_set() failed");
                }

                /*
                    Delete (A - B):
                    Done in (A * B) for ipmc_sf_do_forward_srclist
                    In ipmc_sf_do_not_forward_srclist, there is no any A since (B - A)
                */

                v = ipmc_lib_get_system_local_port_cnt();
                /* Filter(Group) Timer = MALI */
                IPMC_TIMER_MALI_SET(v, entry, &grp_db->tmr.fltr_timer.t[src_port], fltr, grp_db);
            } else {
                /*
                    Router State: EXCLUDE (X, Y)
                    Report Received: IS_EX (A)
                    New Router State: EXCLUDE (A - Y, Y * A)
                    Actions:(A - X - Y) = MALI
                            Delete (X - A)
                            Delete (Y - A)
                            Filter(Group) Timer = MALI
                */
                /* Original X */
                if (!ipmc_lib_srclist_struct_copy(
                        grp_ptr,
                        &ipmc_sf_permit_srclist[IPMC_INTF_IS_MVR_VAL(entry)][src_port],
                        grp_db->ipmc_sf_do_forward_srclist,
                        src_port)) {
                    T_D("vtss_ipmc_data_struct_copy() failed");
                }
                /* Original Y */
                if (!ipmc_lib_srclist_struct_copy(
                        grp_ptr,
                        &ipmc_sf_deny_srclist[IPMC_INTF_IS_MVR_VAL(entry)][src_port],
                        grp_db->ipmc_sf_do_not_forward_srclist,
                        src_port)) {
                    T_D("vtss_ipmc_data_struct_copy() failed");
                }
                /* A: tmp1_srclist */
                if (!ipmc_lib_srclist_clear(&tmp1_srclist[IPMC_INTF_IS_MVR_VAL(entry)], 140)) {
                    T_D("ipmc_lib_srclist_clear failed");
                }
                if (!ipmc_lib_srclist_logical_op_pkt(
                        grp_ptr,
                        entry,
                        src_port,
                        VTSS_IPMC_SFM_OP_OR,
                        &tmp1_srclist[IPMC_INTF_IS_MVR_VAL(entry)],
                        group_record,
                        num_of_src, FALSE, srct)) {
                    T_D("ipmc_lib_srclist_logical_op_pkt() failed");
                }

                /* Timer in (A * X) should not be touch, since (A - X - Y) = MALI  */
                (void) ipmc_lib_protocol_keep_union_list_timer(&tmp1_srclist[IPMC_INTF_IS_MVR_VAL(entry)], grp_db->ipmc_sf_do_forward_srclist, src_port);

                /* EXCLUDE (A - Y, Y * A) */
                if (!ipmc_lib_sfm_logical_op(
                        grp_ptr, src_port, grp_db->ipmc_sf_do_forward_srclist,
                        &tmp1_srclist[IPMC_INTF_IS_MVR_VAL(entry)],
                        &ipmc_sf_deny_srclist[IPMC_INTF_IS_MVR_VAL(entry)][src_port],
                        VTSS_IPMC_SFM_OP_DIFF, srct)) {
                    T_D("ipmc_lib_sfm_logical_op() failed");
                }
                if (!ipmc_lib_srclist_logical_op_pkt(
                        grp_ptr,
                        entry,
                        src_port,
                        VTSS_IPMC_SFM_OP_AND,
                        grp_db->ipmc_sf_do_not_forward_srclist,
                        group_record,
                        num_of_src, TRUE, srct)) {
                    T_D("ipmc_lib_srclist_logical_op_pkt() failed");
                }

                /* tmp2_srclist = (A - X - Y) = ((A - Y) - X) */
                if (!ipmc_lib_srclist_struct_copy(
                        grp_ptr,
                        &tmp2_srclist[IPMC_INTF_IS_MVR_VAL(entry)],
                        &tmp1_srclist[IPMC_INTF_IS_MVR_VAL(entry)],
                        src_port)) {
                    T_D("vtss_ipmc_data_struct_copy() failed");
                }
                if (!ipmc_lib_srclist_logical_op_set(
                        grp_ptr,
                        src_port,
                        VTSS_IPMC_SFM_OP_DIFF, FALSE, srct,
                        &tmp2_srclist[IPMC_INTF_IS_MVR_VAL(entry)],
                        &ipmc_sf_permit_srclist[IPMC_INTF_IS_MVR_VAL(entry)][src_port])) {
                    T_D("ipmc_lib_srclist_logical_op_set() failed");
                }
                if (!ipmc_lib_srclist_logical_op_set(
                        grp_ptr,
                        src_port,
                        VTSS_IPMC_SFM_OP_DIFF, FALSE, srct,
                        &tmp2_srclist[IPMC_INTF_IS_MVR_VAL(entry)],
                        &ipmc_sf_deny_srclist[IPMC_INTF_IS_MVR_VAL(entry)][src_port])) {
                    T_D("ipmc_lib_srclist_logical_op_set() failed");
                }
                /* (A - X - Y) = MALI */
                if (!ipmc_lib_protocol_update_srclist_subset_timer(
                        srct,
                        grp_db->ipmc_sf_do_forward_srclist,
                        &tmp2_srclist[IPMC_INTF_IS_MVR_VAL(entry)],
                        sfm_grp_tmr,
                        src_port)) {
                    T_D("ipmc_lib_protocol_update_srclist_subset_timer() failed");
                }
                if (!ipmc_lib_protocol_update_srclist_subset_timer(
                        srct,
                        grp_db->ipmc_sf_do_not_forward_srclist,
                        &tmp2_srclist[IPMC_INTF_IS_MVR_VAL(entry)],
                        sfm_grp_tmr,
                        src_port)) {
                    T_D("ipmc_lib_protocol_update_srclist_subset_timer() failed");
                }

                /* ipmc_sf_permit_srclist[src_port] = (X - A) */
                if (!ipmc_lib_srclist_logical_op_set(
                        grp_ptr,
                        src_port,
                        VTSS_IPMC_SFM_OP_DIFF, FALSE, srct,
                        &ipmc_sf_permit_srclist[IPMC_INTF_IS_MVR_VAL(entry)][src_port],
                        &tmp1_srclist[IPMC_INTF_IS_MVR_VAL(entry)])) {
                    T_D("ipmc_lib_srclist_logical_op_set() failed");
                }
                /* Delete (X - A) */
                if (!ipmc_lib_srclist_logical_op_set(
                        grp_ptr,
                        src_port,
                        VTSS_IPMC_SFM_OP_DIFF, TRUE, srct,
                        grp_db->ipmc_sf_do_forward_srclist,
                        &ipmc_sf_permit_srclist[IPMC_INTF_IS_MVR_VAL(entry)][src_port])) {
                    T_D("ipmc_lib_srclist_logical_op_set() failed");
                }
                if (!ipmc_lib_srclist_logical_op_set(
                        grp_ptr,
                        src_port,
                        VTSS_IPMC_SFM_OP_DIFF, TRUE, srct,
                        grp_db->ipmc_sf_do_not_forward_srclist,
                        &ipmc_sf_permit_srclist[IPMC_INTF_IS_MVR_VAL(entry)][src_port])) {
                    T_D("ipmc_lib_srclist_logical_op_set() failed");
                }

                /* ipmc_sf_deny_srclist[src_port] = (Y - A) */
                if (!ipmc_lib_srclist_logical_op_set(
                        grp_ptr,
                        src_port,
                        VTSS_IPMC_SFM_OP_DIFF, FALSE, srct,
                        &ipmc_sf_deny_srclist[IPMC_INTF_IS_MVR_VAL(entry)][src_port],
                        &tmp1_srclist[IPMC_INTF_IS_MVR_VAL(entry)])) {
                    T_D("ipmc_lib_srclist_logical_op_set() failed");
                }
                /* Delete (Y - A) */
                if (!ipmc_lib_srclist_logical_op_set(
                        grp_ptr,
                        src_port,
                        VTSS_IPMC_SFM_OP_DIFF, TRUE, srct,
                        grp_db->ipmc_sf_do_forward_srclist,
                        &ipmc_sf_deny_srclist[IPMC_INTF_IS_MVR_VAL(entry)][src_port])) {
                    T_D("ipmc_lib_srclist_logical_op_set() failed");
                }
                if (!ipmc_lib_srclist_logical_op_set(
                        grp_ptr,
                        src_port,
                        VTSS_IPMC_SFM_OP_DIFF, TRUE, srct,
                        grp_db->ipmc_sf_do_not_forward_srclist,
                        &ipmc_sf_deny_srclist[IPMC_INTF_IS_MVR_VAL(entry)][src_port])) {
                    T_D("ipmc_lib_srclist_logical_op_set() failed");
                }

                v = ipmc_lib_get_system_local_port_cnt();
                /* Filter(Group) Timer = MALI */
                IPMC_TIMER_MALI_SET(v, entry, &grp_db->tmr.fltr_timer.t[src_port], fltr, grp_db);
            } /* VTSS_IPMC_SF_MODE_INCLUDE | VTSS_IPMC_SF_MODE_EXCLUDE */
        } else {
            /*
                Router State: INCLUDE (0)
                Report Received: IS_EX (B)
                New Router State: EXCLUDE (0, B)
                Actions:(B) = 0
                        Delete (0)
                        Filter(Group) Timer = MALI
            */
            asm_chg = TRUE;

            /* EXCLUDE (0, B) */
            VTSS_PORT_BF_SET(grp_db->ipmc_sf_port_status, src_port, VTSS_IPMC_SF_STATUS_ENABLED);
            VTSS_PORT_BF_SET(grp_db->ipmc_sf_port_mode, src_port, VTSS_IPMC_SF_MODE_EXCLUDE);
            if (!ipmc_lib_grp_src_list_del4port(srct, src_port, grp_db->ipmc_sf_do_forward_srclist)) {
                T_D("ipmc_lib_grp_src_list_del4port failed");
            }
            if (!ipmc_lib_grp_src_list_del4port(srct, src_port, grp_db->ipmc_sf_do_not_forward_srclist)) {
                T_D("ipmc_lib_grp_src_list_del4port failed");
            }
            if (!ipmc_lib_srclist_logical_op_pkt(
                    grp_ptr,
                    entry,
                    src_port,
                    VTSS_IPMC_SFM_OP_OR,
                    grp_db->ipmc_sf_do_not_forward_srclist,
                    group_record,
                    num_of_src, TRUE, srct)) {
                T_D("ipmc_lib_srclist_logical_op_pkt() failed");
            }

            /* (B) = 0 */
            if (!ipmc_lib_protocol_update_sublist_timer_pkt(
                    entry,
                    srct,
                    grp_db->ipmc_sf_do_not_forward_srclist,
                    group_record,
                    num_of_src,
                    NULL,
                    src_port)) {
                T_D("ipmc_lib_protocol_update_sublist_timer_pkt() failed");
            }

            /* Delete (0) */

            v = ipmc_lib_get_system_local_port_cnt();
            /* Filter(Group) Timer = MALI */
            IPMC_TIMER_MALI_SET(v, entry, &grp_db->tmr.fltr_timer.t[src_port], fltr, grp_db);
        } /* VTSS_IPMC_SF_STATUS_ENABLED | VTSS_IPMC_SF_STATUS_DISABLED */

        grp_op = grp_ptr;
        ipmc_lib_srclist_logical_op_cmp(&grp_is_changed, src_port, grp_op->info->db.ipmc_sf_do_forward_srclist, &allow_list_tmp4rcv[IPMC_INTF_IS_MVR_VAL(entry)]);
        if (!grp_is_changed) {
            ipmc_lib_srclist_logical_op_cmp(&grp_is_changed, src_port, grp_op->info->db.ipmc_sf_do_not_forward_srclist, &block_list_tmp4rcv[IPMC_INTF_IS_MVR_VAL(entry)]);
        }
    } else {
        if ((grp_op = ipmc_lib_group_init(entry, p, &sfm_grp_tmp)) != NULL) {
            grp_op->info->grp = grp_op;
            grp_op->info->db.grp = grp_op;

            ipmc_lib_proc_grp_sfm_tmp4rcv(IPMC_INTF_IS_MVR_VAL(entry), FALSE, FALSE, grp_op);

            grp_info = grp_op->info;
            grp_db = &grp_info->db;

            /*
                Router State: INCLUDE (0)
                Report Received: IS_EX (B)
                New Router State: EXCLUDE (0, B)
                Actions:(B) = 0
                        Delete (0)
                        Filter(Group) Timer = MALI
            */
            asm_chg = TRUE;

            /* EXCLUDE (0, B) */
            VTSS_PORT_BF_SET(grp_db->ipmc_sf_port_status, src_port, VTSS_IPMC_SF_STATUS_ENABLED);
            VTSS_PORT_BF_SET(grp_db->ipmc_sf_port_mode, src_port, VTSS_IPMC_SF_MODE_EXCLUDE);
            if (!ipmc_lib_grp_src_list_del4port(srct, src_port, grp_db->ipmc_sf_do_forward_srclist)) {
                T_D("ipmc_lib_grp_src_list_del4port failed");
            }
            if (!ipmc_lib_grp_src_list_del4port(srct, src_port, grp_db->ipmc_sf_do_not_forward_srclist)) {
                T_D("ipmc_lib_grp_src_list_del4port failed");
            }
            if (!ipmc_lib_srclist_logical_op_pkt(
                    grp_ptr,
                    entry,
                    src_port,
                    VTSS_IPMC_SFM_OP_OR,
                    grp_db->ipmc_sf_do_not_forward_srclist,
                    group_record,
                    num_of_src, TRUE, srct)) {
                T_D("ipmc_lib_srclist_logical_op_pkt() failed");
            }

            /* (B) = 0 */
            if (!ipmc_lib_protocol_update_sublist_timer_pkt(
                    entry,
                    srct,
                    grp_db->ipmc_sf_do_not_forward_srclist,
                    group_record,
                    num_of_src,
                    NULL,
                    src_port)) {
                T_D("ipmc_lib_protocol_update_sublist_timer_pkt() failed");
            }

            /* Delete (0) */

            v = ipmc_lib_get_system_local_port_cnt();
            /* Filter(Group) Timer = MALI */
            IPMC_TIMER_MALI_SET(v, entry, &grp_db->tmr.fltr_timer.t[src_port], fltr, grp_db);

            grp_is_changed = TRUE;
        }
    } /* if this group already exists */

    if (asm_chg) {
        if (!ipmc_lib_group_update(p, grp_op)) {
            T_D("update group with chip failed!!!");
        }
    }
    if (grp_is_changed) {
        if (!ipmc_lib_group_sync(p, entry, grp_op, FALSE, PROC4RCV)) {
            T_D("sync group with chip failed!!!");
        }
    }
}

static void _ipmc_lib_sfm_change_to_include(ipmc_db_ctrl_hdr_t *p,
                                            ipmc_db_ctrl_hdr_t *rxmt,
                                            ipmc_db_ctrl_hdr_t *fltr,
                                            ipmc_db_ctrl_hdr_t *srct,
                                            ipmc_group_entry_t *grp_ptr,
                                            ipmc_intf_entry_t *entry,
                                            u8 src_port,
                                            vtss_ipv6_t *group_address,
                                            void *group_record,
                                            u16 num_of_src,
                                            BOOL proxy, ipmc_time_t *sfm_grp_tmr)
{
    BOOL                asm_chg, grp_is_changed, lower_source_timer;
    ipmc_group_entry_t  *grp_op, sfm_grp_tmp;
    ipmc_group_info_t   *grp_info;
    ipmc_group_db_t     *grp_db;
    ipmc_send_act_t     snd_act;

    if (!p || !entry || !group_address || !group_record) {
        return;
    }

    asm_chg = grp_is_changed = lower_source_timer = FALSE;
    if (grp_ptr == NULL) {
        sfm_grp_tmp.vid = entry->param.vid;
        memcpy(&sfm_grp_tmp.group_addr, group_address, sizeof(vtss_ipv6_t));
        sfm_grp_tmp.ipmc_version = entry->ipmc_version;

        grp_ptr = ipmc_lib_group_ptr_get(p, &sfm_grp_tmp);
    }
    if (grp_ptr != NULL) {
        ipmc_lib_proc_grp_sfm_tmp4rcv(IPMC_INTF_IS_MVR_VAL(entry), FALSE, TRUE, grp_ptr);

        grp_info = grp_ptr->info;
        grp_db = &grp_info->db;

        if (IPMC_LIB_GRP_PORT_DO_SFM(grp_db, src_port)) {
            VTSS_PORT_BF_SET(grp_db->ipmc_sf_port_status, src_port, VTSS_IPMC_SF_STATUS_ENABLED);

            if (IPMC_LIB_GRP_PORT_SFM_IN(grp_db, src_port)) {
                /*
                    Router State: INCLUDE (A)
                    Report Received: TO_IN (B)
                    New Router State: INCLUDE (A + B)
                    Actions:(B) = MALI
                            Send Q(MA, A - B)
                */
                /* tmp1_srclist = A - B */
                if (!ipmc_lib_srclist_struct_copy(
                        grp_ptr,
                        &tmp1_srclist[IPMC_INTF_IS_MVR_VAL(entry)],
                        grp_db->ipmc_sf_do_forward_srclist,
                        src_port)) {
                    T_D("vtss_ipmc_data_struct_copy() failed");
                }
                if (!ipmc_lib_srclist_logical_op_pkt(
                        grp_ptr,
                        entry,
                        src_port,
                        VTSS_IPMC_SFM_OP_DIFF,
                        &tmp1_srclist[IPMC_INTF_IS_MVR_VAL(entry)],
                        group_record,
                        num_of_src, FALSE, srct)) {
                    T_D("ipmc_lib_srclist_logical_op_pkt() failed");
                }

                /* INCLUDE (A + B) */
                if (!ipmc_lib_srclist_logical_op_pkt(
                        grp_ptr,
                        entry,
                        src_port,
                        VTSS_IPMC_SFM_OP_OR,
                        grp_db->ipmc_sf_do_forward_srclist,
                        group_record,
                        num_of_src, TRUE, srct)) {
                    T_D("ipmc_lib_srclist_logical_op_pkt() failed");
                }

                /* (B) = MALI */
                if (!ipmc_lib_protocol_update_sublist_timer_pkt(
                        entry,
                        srct,
                        grp_db->ipmc_sf_do_forward_srclist,
                        group_record,
                        num_of_src,
                        sfm_grp_tmr,
                        src_port)) {
                    T_D("ipmc_lib_protocol_update_sublist_timer_pkt() failed");
                }
                if (!ipmc_lib_protocol_update_sublist_timer_pkt(
                        entry,
                        srct,
                        grp_db->ipmc_sf_do_not_forward_srclist,
                        group_record,
                        num_of_src,
                        sfm_grp_tmr,
                        src_port)) {
                    T_D("ipmc_lib_protocol_update_sublist_timer_pkt() failed");
                }

                /* Send Q(MA, A - B) */
                snd_act = ipmc_lib_get_sq_ssq_action(proxy, entry, src_port);
                (void) ipmc_lib_packet_tx_ssq(fltr, snd_act, grp_ptr, entry, src_port, &tmp1_srclist[IPMC_INTF_IS_MVR_VAL(entry)], FALSE);

                if (snd_act != IPMC_SND_HOLD) { /* Querier */
                    grp_info->state = IPMC_OP_CHK_LISTENER;
                    /* start rxmt timer */
                    grp_info->rxmt_count[src_port] = IPMC_TIMER_LLQC(entry);
                    IPMC_TIMER_LLQI_SET(entry, &grp_info->rxmt_timer[src_port], rxmt, grp_info);
                }

                lower_source_timer = TRUE;
            } else {
                /*
                    Router State: EXCLUDE (X, Y)
                    Report Received: TO_IN (A)
                    New Router State: EXCLUDE (X + A, Y - A)
                    Actions:(A) = MALI
                            Send Q(MA, X - A)
                            Send Q(MA)
                */
                /* tmp1_srclist = X - A */
                if (!ipmc_lib_srclist_struct_copy(
                        grp_ptr,
                        &tmp1_srclist[IPMC_INTF_IS_MVR_VAL(entry)],
                        grp_db->ipmc_sf_do_forward_srclist,
                        src_port)) {
                    T_D("vtss_ipmc_data_struct_copy() failed");
                }
                if (!ipmc_lib_srclist_logical_op_pkt(
                        grp_ptr,
                        entry,
                        src_port,
                        VTSS_IPMC_SFM_OP_DIFF,
                        &tmp1_srclist[IPMC_INTF_IS_MVR_VAL(entry)],
                        group_record,
                        num_of_src, FALSE, srct)) {
                    T_D("ipmc_lib_srclist_logical_op_pkt() failed");
                }

                /* EXCLUDE (X + A, Y - A) */
                if (!ipmc_lib_srclist_logical_op_pkt(
                        grp_ptr,
                        entry,
                        src_port,
                        VTSS_IPMC_SFM_OP_OR,
                        grp_db->ipmc_sf_do_forward_srclist,
                        group_record,
                        num_of_src, TRUE, srct)) {
                    T_D("ipmc_lib_srclist_logical_op_pkt() failed");
                }
                if (!ipmc_lib_srclist_logical_op_pkt(
                        grp_ptr,
                        entry,
                        src_port,
                        VTSS_IPMC_SFM_OP_DIFF,
                        grp_db->ipmc_sf_do_not_forward_srclist,
                        group_record,
                        num_of_src, TRUE, srct)) {
                    T_D("ipmc_lib_srclist_logical_op_pkt() failed");
                }

                /* (A) = MALI */
                if (!ipmc_lib_protocol_update_sublist_timer_pkt(
                        entry,
                        srct,
                        grp_db->ipmc_sf_do_forward_srclist,
                        group_record,
                        num_of_src,
                        sfm_grp_tmr,
                        src_port)) {
                    T_D("ipmc_lib_protocol_update_sublist_timer_pkt() failed");
                }
                if (!ipmc_lib_protocol_update_sublist_timer_pkt(
                        entry,
                        srct,
                        grp_db->ipmc_sf_do_not_forward_srclist,
                        group_record,
                        num_of_src,
                        sfm_grp_tmr,
                        src_port)) {
                    T_D("ipmc_lib_protocol_update_sublist_timer_pkt() failed");
                }

                snd_act = ipmc_lib_get_sq_ssq_action(proxy, entry, src_port);
                /* Send Q(MA, X - A) */
                (void) ipmc_lib_packet_tx_ssq(fltr, snd_act, grp_ptr, entry, src_port, &tmp1_srclist[IPMC_INTF_IS_MVR_VAL(entry)], FALSE);

                /* Send Q(MA) */
                (void) ipmc_lib_packet_tx_sq(fltr, snd_act, grp_ptr, entry, src_port, FALSE);

                if (snd_act != IPMC_SND_HOLD) { /* Querier */
                    grp_info->state = IPMC_OP_CHK_LISTENER;
                    /* start rxmt timer */
                    grp_info->rxmt_count[src_port] = IPMC_TIMER_LLQC(entry);
                    IPMC_TIMER_LLQI_SET(entry, &grp_info->rxmt_timer[src_port], rxmt, grp_info);
                }

                lower_source_timer = TRUE;
            } /* VTSS_IPMC_SF_MODE_INCLUDE | VTSS_IPMC_SF_MODE_EXCLUDE */
        } else {
            /*
                Router State: INCLUDE (0)
                Report Received: TO_IN (B)
                New Router State: INCLUDE (B)
                Actions:(B) = MALI
                        Send Q(MA, 0)
            */
            asm_chg = TRUE;

            /* INCLUDE (B) */
            VTSS_PORT_BF_SET(grp_db->ipmc_sf_port_status, src_port, VTSS_IPMC_SF_STATUS_ENABLED);
            VTSS_PORT_BF_SET(grp_db->ipmc_sf_port_mode, src_port, VTSS_IPMC_SF_MODE_INCLUDE);
            if (!ipmc_lib_grp_src_list_del4port(srct, src_port, grp_db->ipmc_sf_do_forward_srclist)) {
                T_D("ipmc_lib_grp_src_list_del4port failed");
            }
            if (!ipmc_lib_grp_src_list_del4port(srct, src_port, grp_db->ipmc_sf_do_not_forward_srclist)) {
                T_D("ipmc_lib_grp_src_list_del4port failed");
            }

            if (!ipmc_lib_srclist_logical_op_pkt(
                    grp_ptr,
                    entry,
                    src_port,
                    VTSS_IPMC_SFM_OP_OR,
                    grp_db->ipmc_sf_do_forward_srclist,
                    group_record,
                    num_of_src, TRUE, srct)) {
                T_D("ipmc_lib_srclist_logical_op_pkt() failed");
            }

            /* (B) = MALI */
            if (!ipmc_lib_protocol_update_sublist_timer_pkt(
                    entry,
                    srct,
                    grp_db->ipmc_sf_do_forward_srclist,
                    group_record,
                    num_of_src,
                    sfm_grp_tmr,
                    src_port)) {
                T_D("ipmc_lib_protocol_update_sublist_timer_pkt() failed");
            }
            if (!ipmc_lib_protocol_update_sublist_timer_pkt(
                    entry,
                    srct,
                    grp_db->ipmc_sf_do_not_forward_srclist,
                    group_record,
                    num_of_src,
                    sfm_grp_tmr,
                    src_port)) {
                T_D("ipmc_lib_protocol_update_sublist_timer_pkt() failed");
            }

            /* Send Q(MA, 0) */
            if (!ipmc_lib_srclist_clear(&tmp1_srclist[IPMC_INTF_IS_MVR_VAL(entry)], 130)) {
                T_D("ipmc_lib_srclist_clear failed");
            }
            snd_act = ipmc_lib_get_sq_ssq_action(proxy, entry, src_port);
            (void) ipmc_lib_packet_tx_ssq(fltr, snd_act, grp_ptr, entry, src_port, &tmp1_srclist[IPMC_INTF_IS_MVR_VAL(entry)], FALSE);

            if (snd_act != IPMC_SND_HOLD) { /* Querier */
                grp_info->state = IPMC_OP_CHK_LISTENER;
                /* start rxmt timer */
                grp_info->rxmt_count[src_port] = IPMC_TIMER_LLQC(entry);
                IPMC_TIMER_LLQI_SET(entry, &grp_info->rxmt_timer[src_port], rxmt, grp_info);
            }
        } /* VTSS_IPMC_SF_STATUS_ENABLED | VTSS_IPMC_SF_STATUS_DISABLED */

        grp_op = grp_ptr;
        ipmc_lib_srclist_logical_op_cmp(&grp_is_changed, src_port, grp_op->info->db.ipmc_sf_do_forward_srclist, &allow_list_tmp4rcv[IPMC_INTF_IS_MVR_VAL(entry)]);
        if (!grp_is_changed) {
            ipmc_lib_srclist_logical_op_cmp(&grp_is_changed, src_port, grp_op->info->db.ipmc_sf_do_not_forward_srclist, &block_list_tmp4rcv[IPMC_INTF_IS_MVR_VAL(entry)]);
        }
    } else {
        if ((grp_op = ipmc_lib_group_init(entry, p, &sfm_grp_tmp)) != NULL) {
            grp_op->info->grp = grp_op;
            grp_op->info->db.grp = grp_op;

            ipmc_lib_proc_grp_sfm_tmp4rcv(IPMC_INTF_IS_MVR_VAL(entry), FALSE, FALSE, grp_op);

            grp_info = grp_op->info;
            grp_db = &grp_info->db;

            /*
                Router State: INCLUDE (0)
                Report Received: TO_IN (B)
                New Router State: INCLUDE (B)
                Actions:(B) = MALI
                        Send Q(MA, 0)
            */
            asm_chg = TRUE;

            /* INCLUDE (B) */
            VTSS_PORT_BF_SET(grp_db->ipmc_sf_port_status, src_port, VTSS_IPMC_SF_STATUS_ENABLED);
            VTSS_PORT_BF_SET(grp_db->ipmc_sf_port_mode, src_port, VTSS_IPMC_SF_MODE_INCLUDE);
            if (!ipmc_lib_grp_src_list_del4port(srct, src_port, grp_db->ipmc_sf_do_forward_srclist)) {
                T_D("ipmc_lib_grp_src_list_del4port failed");
            }
            if (!ipmc_lib_grp_src_list_del4port(srct, src_port, grp_db->ipmc_sf_do_not_forward_srclist)) {
                T_D("ipmc_lib_grp_src_list_del4port failed");
            }

            if (!ipmc_lib_srclist_logical_op_pkt(
                    grp_ptr,
                    entry,
                    src_port,
                    VTSS_IPMC_SFM_OP_OR,
                    grp_db->ipmc_sf_do_forward_srclist,
                    group_record,
                    num_of_src, TRUE, srct)) {
                T_D("ipmc_lib_srclist_logical_op_pkt() failed");
            }

            /* (B) = MALI */
            if (!ipmc_lib_protocol_update_sublist_timer_pkt(
                    entry,
                    srct,
                    grp_db->ipmc_sf_do_forward_srclist,
                    group_record,
                    num_of_src,
                    sfm_grp_tmr,
                    src_port)) {
                T_D("ipmc_lib_protocol_update_sublist_timer_pkt() failed");
            }
            if (!ipmc_lib_protocol_update_sublist_timer_pkt(
                    entry,
                    srct,
                    grp_db->ipmc_sf_do_not_forward_srclist,
                    group_record,
                    num_of_src,
                    sfm_grp_tmr,
                    src_port)) {
                T_D("ipmc_lib_protocol_update_sublist_timer_pkt() failed");
            }

            /* Send Q(MA, 0) */
            if (!ipmc_lib_srclist_clear(&tmp1_srclist[IPMC_INTF_IS_MVR_VAL(entry)], 120)) {
                T_D("ipmc_lib_srclist_clear failed");
            }
            snd_act = ipmc_lib_get_sq_ssq_action(proxy, entry, src_port);
            (void) ipmc_lib_packet_tx_ssq(fltr, snd_act, grp_ptr, entry, src_port, &tmp1_srclist[IPMC_INTF_IS_MVR_VAL(entry)], FALSE);

            if (snd_act != IPMC_SND_HOLD) { /* Querier */
                grp_info->state = IPMC_OP_CHK_LISTENER;
                /* start rxmt timer */
                grp_info->rxmt_count[src_port] = IPMC_TIMER_LLQC(entry);
                IPMC_TIMER_LLQI_SET(entry, &grp_info->rxmt_timer[src_port], rxmt, grp_info);
            }

            grp_is_changed = TRUE;
        }
    } /* if this group already exists */

    if (asm_chg) {
        if (!ipmc_lib_group_update(p, grp_op)) {
            T_D("update group with chip failed!!!");
        }
    }
    if (grp_is_changed) {
        if (!ipmc_lib_group_sync(p, entry, grp_op, FALSE, PROC4RCV)) {
            T_D("sync group with chip failed!!!");
        }
    }

    if (lower_source_timer) {
        ipmc_lib_protocol_lower_source_timer(srct, grp_op,
                                             entry,
                                             &tmp1_srclist[IPMC_INTF_IS_MVR_VAL(entry)],
                                             src_port);
    }
}

static void _ipmc_lib_sfm_change_to_exclude(ipmc_db_ctrl_hdr_t *p,
                                            ipmc_db_ctrl_hdr_t *rxmt,
                                            ipmc_db_ctrl_hdr_t *fltr,
                                            ipmc_db_ctrl_hdr_t *srct,
                                            ipmc_group_entry_t *grp_ptr,
                                            ipmc_intf_entry_t *entry,
                                            u8 src_port,
                                            vtss_ipv6_t *group_address,
                                            void *group_record,
                                            u16 num_of_src,
                                            BOOL proxy, ipmc_time_t *sfm_grp_tmr)
{
    BOOL                asm_chg, grp_is_changed, lower_source_timer;
    u32                 v;
    ipmc_group_entry_t  *grp_op, sfm_grp_tmp;
    ipmc_group_info_t   *grp_info;
    ipmc_group_db_t     *grp_db;
    ipmc_send_act_t     snd_act;

    if (!p || !entry || !group_address || !group_record) {
        return;
    }

    asm_chg = grp_is_changed = lower_source_timer = FALSE;
    if (grp_ptr == NULL) {
        sfm_grp_tmp.vid = entry->param.vid;
        memcpy(&sfm_grp_tmp.group_addr, group_address, sizeof(vtss_ipv6_t));
        sfm_grp_tmp.ipmc_version = entry->ipmc_version;

        grp_ptr = ipmc_lib_group_ptr_get(p, &sfm_grp_tmp);
    }
    if (grp_ptr != NULL) {
        ipmc_lib_proc_grp_sfm_tmp4rcv(IPMC_INTF_IS_MVR_VAL(entry), FALSE, TRUE, grp_ptr);

        grp_info = grp_ptr->info;
        grp_db = &grp_info->db;

        if (IPMC_LIB_GRP_PORT_DO_SFM(grp_db, src_port)) {
            VTSS_PORT_BF_SET(grp_db->ipmc_sf_port_status, src_port, VTSS_IPMC_SF_STATUS_ENABLED);

            if (IPMC_LIB_GRP_PORT_SFM_IN(grp_db, src_port)) {
                /*
                    Router State: INCLUDE (A)
                    Report Received: TO_EX (B)
                    New Router State: EXCLUDE (A * B, B - A)
                    Actions:(B - A) = 0
                            Delete (A - B)
                            Send Q(MA, A * B)
                            Filter(Group) Timer = MALI
                */
                asm_chg = TRUE;

                /* tmp1_srclist = (A - B) */
                if (!ipmc_lib_srclist_struct_copy(
                        grp_ptr,
                        &tmp1_srclist[IPMC_INTF_IS_MVR_VAL(entry)],
                        grp_db->ipmc_sf_do_forward_srclist,
                        src_port)) {
                    T_D("vtss_ipmc_data_struct_copy() failed");
                }
                if (!ipmc_lib_srclist_logical_op_pkt(
                        grp_ptr,
                        entry,
                        src_port,
                        VTSS_IPMC_SFM_OP_DIFF,
                        &tmp1_srclist[IPMC_INTF_IS_MVR_VAL(entry)],
                        group_record,
                        num_of_src, FALSE, srct)) {
                    T_D("ipmc_lib_srclist_logical_op_pkt() failed");
                }

                /* EXCLUDE (A * B, B - A) */
                VTSS_PORT_BF_SET(grp_db->ipmc_sf_port_mode, src_port, VTSS_IPMC_SF_MODE_EXCLUDE);
                if (!ipmc_lib_grp_src_list_del4port(srct, src_port, grp_db->ipmc_sf_do_not_forward_srclist)) {
                    T_D("ipmc_lib_grp_src_list_del4port failed");
                }
                if (!ipmc_lib_srclist_logical_op_pkt(
                        grp_ptr,
                        entry,
                        src_port,
                        VTSS_IPMC_SFM_OP_OR,
                        grp_db->ipmc_sf_do_not_forward_srclist,
                        group_record,
                        num_of_src, TRUE, srct)) {
                    T_D("ipmc_lib_srclist_logical_op_pkt() failed");
                }
                if (!ipmc_lib_srclist_logical_op_set(
                        grp_ptr,
                        src_port,
                        VTSS_IPMC_SFM_OP_DIFF, TRUE, srct,
                        grp_db->ipmc_sf_do_not_forward_srclist,
                        grp_db->ipmc_sf_do_forward_srclist)) {
                    T_D("ipmc_lib_srclist_logical_op_set() failed");
                }
                if (!ipmc_lib_srclist_logical_op_pkt(
                        grp_ptr,
                        entry,
                        src_port,
                        VTSS_IPMC_SFM_OP_AND,
                        grp_db->ipmc_sf_do_forward_srclist,
                        group_record,
                        num_of_src, TRUE, srct)) {
                    T_D("ipmc_lib_srclist_logical_op_pkt() failed");
                }

                /* (B - A) = 0 */
                if (!ipmc_lib_protocol_update_sublist_timer_set(
                        srct,
                        grp_db->ipmc_sf_do_not_forward_srclist,
                        grp_db->ipmc_sf_do_not_forward_srclist,
                        NULL,
                        src_port)) {
                    T_D("ipmc_lib_protocol_update_sublist_timer_set() failed");
                }
                if (!ipmc_lib_protocol_update_sublist_timer_set(
                        srct,
                        grp_db->ipmc_sf_do_forward_srclist,
                        grp_db->ipmc_sf_do_not_forward_srclist,
                        NULL,
                        src_port)) {
                    T_D("ipmc_lib_protocol_update_sublist_timer_set() failed");
                }

                /*
                    Delete (A - B):
                    Done in (A * B) for ipmc_sf_do_forward_srclist
                    In ipmc_sf_do_not_forward_srclist, there is no any A since (B - A)
                */

                /* tmp2_srclist = (A * B) */
                if (!ipmc_lib_srclist_struct_copy(
                        grp_ptr,
                        &tmp2_srclist[IPMC_INTF_IS_MVR_VAL(entry)],
                        grp_db->ipmc_sf_do_forward_srclist,
                        src_port)) {
                    T_D("vtss_ipmc_data_struct_copy() failed");
                } else {
                    lower_source_timer = TRUE;
                }
                /* Send Q(MA, A * B) */
                snd_act = ipmc_lib_get_sq_ssq_action(proxy, entry, src_port);
                (void) ipmc_lib_packet_tx_ssq(fltr, snd_act, grp_ptr, entry, src_port, &tmp2_srclist[IPMC_INTF_IS_MVR_VAL(entry)], FALSE);

                if (snd_act != IPMC_SND_HOLD) { /* Querier */
                    grp_info->state = IPMC_OP_CHK_LISTENER;
                    /* start rxmt timer */
                    grp_info->rxmt_count[src_port] = IPMC_TIMER_LLQC(entry);
                    IPMC_TIMER_LLQI_SET(entry, &grp_info->rxmt_timer[src_port], rxmt, grp_info);
                }

                v = ipmc_lib_get_system_local_port_cnt();
                /* Filter(Group) Timer = MALI */
                IPMC_TIMER_MALI_SET(v, entry, &grp_db->tmr.fltr_timer.t[src_port], fltr, grp_db);
            } else {
                /*
                    Router State: EXCLUDE (X, Y)
                    Report Received: TO_EX (A)
                    New Router State: EXCLUDE (A - Y, Y * A)
                    Actions:(A - X - Y) = Filter(Group) Timer
                            Delete (X - A)
                            Delete (Y - A)
                            Send Q(MA, A - Y)
                            Filter(Group) Timer = MALI
                */
                /* Original X */
                if (!ipmc_lib_srclist_struct_copy(
                        grp_ptr,
                        &ipmc_sf_permit_srclist[IPMC_INTF_IS_MVR_VAL(entry)][src_port],
                        grp_db->ipmc_sf_do_forward_srclist,
                        src_port)) {
                    T_D("vtss_ipmc_data_struct_copy() failed");
                }
                /* Original Y */
                if (!ipmc_lib_srclist_struct_copy(
                        grp_ptr,
                        &ipmc_sf_deny_srclist[IPMC_INTF_IS_MVR_VAL(entry)][src_port],
                        grp_db->ipmc_sf_do_not_forward_srclist,
                        src_port)) {
                    T_D("vtss_ipmc_data_struct_copy() failed");
                }
                /* A: tmp1_srclist */
                if (!ipmc_lib_srclist_clear(&tmp1_srclist[IPMC_INTF_IS_MVR_VAL(entry)], 110)) {
                    T_D("ipmc_lib_srclist_clear failed");
                }
                if (!ipmc_lib_srclist_logical_op_pkt(
                        grp_ptr,
                        entry,
                        src_port,
                        VTSS_IPMC_SFM_OP_OR,
                        &tmp1_srclist[IPMC_INTF_IS_MVR_VAL(entry)],
                        group_record,
                        num_of_src, FALSE, srct)) {
                    T_D("ipmc_lib_srclist_logical_op_pkt() failed");
                }

                /* Timer in (A * X) should not be touch, since (A - X - Y) = Filter(Group) Timer  */
                (void) ipmc_lib_protocol_keep_union_list_timer(&tmp1_srclist[IPMC_INTF_IS_MVR_VAL(entry)], grp_db->ipmc_sf_do_forward_srclist, src_port);

                /* EXCLUDE (A - Y, Y * A) */
                if (!ipmc_lib_sfm_logical_op(
                        grp_ptr, src_port, grp_db->ipmc_sf_do_forward_srclist,
                        &tmp1_srclist[IPMC_INTF_IS_MVR_VAL(entry)],
                        &ipmc_sf_deny_srclist[IPMC_INTF_IS_MVR_VAL(entry)][src_port],
                        VTSS_IPMC_SFM_OP_DIFF, srct)) {
                    T_D("ipmc_lib_sfm_logical_op() failed");
                }
                if (!ipmc_lib_srclist_logical_op_pkt(
                        grp_ptr,
                        entry,
                        src_port,
                        VTSS_IPMC_SFM_OP_AND,
                        grp_db->ipmc_sf_do_not_forward_srclist,
                        group_record,
                        num_of_src, TRUE, srct)) {
                    T_D("ipmc_lib_srclist_logical_op_pkt() failed");
                }

                /* tmp2_srclist = (A - Y) */
                if (!ipmc_lib_srclist_struct_copy(
                        grp_ptr,
                        &tmp2_srclist[IPMC_INTF_IS_MVR_VAL(entry)],
                        &tmp1_srclist[IPMC_INTF_IS_MVR_VAL(entry)],
                        src_port)) {
                    T_W("vtss_ipmc_data_struct_copy() failed");
                } else {
                    if (!ipmc_lib_srclist_logical_op_set(
                            grp_ptr,
                            src_port,
                            VTSS_IPMC_SFM_OP_DIFF, FALSE, srct,
                            &tmp2_srclist[IPMC_INTF_IS_MVR_VAL(entry)],
                            &ipmc_sf_deny_srclist[IPMC_INTF_IS_MVR_VAL(entry)][src_port])) {
                        T_W("ipmc_lib_srclist_logical_op_set() failed");
                    } else {
                        lower_source_timer = TRUE;
                    }
                }

                /* ipmc_sf_permit_srclist[src_port] = (X - A) */
                if (!ipmc_lib_srclist_logical_op_set(
                        grp_ptr,
                        src_port,
                        VTSS_IPMC_SFM_OP_DIFF, FALSE, srct,
                        &ipmc_sf_permit_srclist[IPMC_INTF_IS_MVR_VAL(entry)][src_port],
                        &tmp1_srclist[IPMC_INTF_IS_MVR_VAL(entry)])) {
                    T_D("ipmc_lib_srclist_logical_op_set() failed");
                }
                /* Delete (X - A) */
                if (!ipmc_lib_srclist_logical_op_set(
                        grp_ptr,
                        src_port,
                        VTSS_IPMC_SFM_OP_DIFF, TRUE, srct,
                        grp_db->ipmc_sf_do_forward_srclist,
                        &ipmc_sf_permit_srclist[IPMC_INTF_IS_MVR_VAL(entry)][src_port])) {
                    T_D("ipmc_lib_srclist_logical_op_set() failed");
                }
                if (!ipmc_lib_srclist_logical_op_set(
                        grp_ptr,
                        src_port,
                        VTSS_IPMC_SFM_OP_DIFF, TRUE, srct,
                        grp_db->ipmc_sf_do_not_forward_srclist,
                        &ipmc_sf_permit_srclist[IPMC_INTF_IS_MVR_VAL(entry)][src_port])) {
                    T_D("ipmc_lib_srclist_logical_op_set() failed");
                }

                /* ipmc_sf_deny_srclist[src_port] = (Y - A) */
                if (!ipmc_lib_srclist_logical_op_set(
                        grp_ptr,
                        src_port,
                        VTSS_IPMC_SFM_OP_DIFF, FALSE, srct,
                        &ipmc_sf_deny_srclist[IPMC_INTF_IS_MVR_VAL(entry)][src_port],
                        &tmp1_srclist[IPMC_INTF_IS_MVR_VAL(entry)])) {
                    T_D("ipmc_lib_srclist_logical_op_set() failed");
                }
                /* Delete (Y - A) */
                if (!ipmc_lib_srclist_logical_op_set(
                        grp_ptr,
                        src_port,
                        VTSS_IPMC_SFM_OP_DIFF, TRUE, srct,
                        grp_db->ipmc_sf_do_forward_srclist,
                        &ipmc_sf_deny_srclist[IPMC_INTF_IS_MVR_VAL(entry)][src_port])) {
                    T_D("ipmc_lib_srclist_logical_op_set() failed");
                }
                if (!ipmc_lib_srclist_logical_op_set(
                        grp_ptr,
                        src_port,
                        VTSS_IPMC_SFM_OP_DIFF, TRUE, srct,
                        grp_db->ipmc_sf_do_not_forward_srclist,
                        &ipmc_sf_deny_srclist[IPMC_INTF_IS_MVR_VAL(entry)][src_port])) {
                    T_D("ipmc_lib_srclist_logical_op_set() failed");
                }

                /* tmp1_srclist = (A - X - Y) = ((A - Y) - X) = (tmp2_srclist - X) */
                if (!ipmc_lib_srclist_struct_copy(
                        grp_ptr,
                        &tmp1_srclist[IPMC_INTF_IS_MVR_VAL(entry)],
                        &tmp2_srclist[IPMC_INTF_IS_MVR_VAL(entry)],
                        src_port)) {
                    T_D("vtss_ipmc_data_struct_copy() failed");
                }
                if (!ipmc_lib_srclist_logical_op_set(
                        grp_ptr,
                        src_port,
                        VTSS_IPMC_SFM_OP_DIFF, FALSE, srct,
                        &tmp1_srclist[IPMC_INTF_IS_MVR_VAL(entry)],
                        &ipmc_sf_permit_srclist[IPMC_INTF_IS_MVR_VAL(entry)][src_port])) {
                    T_D("ipmc_lib_srclist_logical_op_set() failed");
                }
                /* (A - X - Y) = Filter(Group) Timer */
                if (!ipmc_lib_protocol_update_srclist_subset_timer(
                        srct,
                        grp_db->ipmc_sf_do_forward_srclist,
                        &tmp1_srclist[IPMC_INTF_IS_MVR_VAL(entry)],
                        &grp_db->tmr.fltr_timer.t[src_port],
                        src_port)) {
                    T_D("ipmc_lib_protocol_update_srclist_subset_timer() failed");
                }
                if (!ipmc_lib_protocol_update_srclist_subset_timer(
                        srct,
                        grp_db->ipmc_sf_do_not_forward_srclist,
                        &tmp1_srclist[IPMC_INTF_IS_MVR_VAL(entry)],
                        &grp_db->tmr.fltr_timer.t[src_port],
                        src_port)) {
                    T_D("ipmc_lib_protocol_update_srclist_subset_timer() failed");
                }

                /* Send Q(MA, A - Y) */
                snd_act = ipmc_lib_get_sq_ssq_action(proxy, entry, src_port);
                (void) ipmc_lib_packet_tx_ssq(fltr, snd_act, grp_ptr, entry, src_port, &tmp2_srclist[IPMC_INTF_IS_MVR_VAL(entry)], FALSE);

                if (snd_act != IPMC_SND_HOLD) { /* Querier */
                    grp_info->state = IPMC_OP_CHK_LISTENER;
                    /* start rxmt timer */
                    grp_info->rxmt_count[src_port] = IPMC_TIMER_LLQC(entry);
                    IPMC_TIMER_LLQI_SET(entry, &grp_info->rxmt_timer[src_port], rxmt, grp_info);
                }

                v = ipmc_lib_get_system_local_port_cnt();
                /* Filter(Group) Timer = MALI */
                IPMC_TIMER_MALI_SET(v, entry, &grp_db->tmr.fltr_timer.t[src_port], fltr, grp_db);
            } /* VTSS_IPMC_SF_MODE_INCLUDE | VTSS_IPMC_SF_MODE_EXCLUDE */
        }  else {
            /*
                Router State: INCLUDE (0)
                Report Received: TO_EX (B)
                New Router State: EXCLUDE (0, B)
                Actions:(B) = 0
                        Delete (0)
                        Send Q(MA, 0)
                        Filter(Group) Timer = MALI
            */
            asm_chg = TRUE;

            /* EXCLUDE (0, B) */
            VTSS_PORT_BF_SET(grp_db->ipmc_sf_port_status, src_port, VTSS_IPMC_SF_STATUS_ENABLED);
            VTSS_PORT_BF_SET(grp_db->ipmc_sf_port_mode, src_port, VTSS_IPMC_SF_MODE_EXCLUDE);
            if (!ipmc_lib_grp_src_list_del4port(srct, src_port, grp_db->ipmc_sf_do_forward_srclist)) {
                T_D("ipmc_lib_grp_src_list_del4port failed");
            }
            if (!ipmc_lib_grp_src_list_del4port(srct, src_port, grp_db->ipmc_sf_do_not_forward_srclist)) {
                T_D("ipmc_lib_grp_src_list_del4port failed");
            }
            if (!ipmc_lib_srclist_logical_op_pkt(
                    grp_ptr,
                    entry,
                    src_port,
                    VTSS_IPMC_SFM_OP_OR,
                    grp_db->ipmc_sf_do_not_forward_srclist,
                    group_record,
                    num_of_src, TRUE, srct)) {
                T_D("ipmc_lib_srclist_logical_op_pkt() failed");
            }

            /* (B) = 0 */
            if (!ipmc_lib_protocol_update_sublist_timer_pkt(
                    entry,
                    srct,
                    grp_db->ipmc_sf_do_not_forward_srclist,
                    group_record,
                    num_of_src,
                    NULL,
                    src_port)) {
                T_D("ipmc_lib_protocol_update_sublist_timer_pkt() failed");
            }

            /* Delete (0) */

            /* Send Q(MA, 0) */
            if (!ipmc_lib_srclist_clear(&tmp1_srclist[IPMC_INTF_IS_MVR_VAL(entry)], 100)) {
                T_D("ipmc_lib_srclist_clear failed");
            }
            snd_act = ipmc_lib_get_sq_ssq_action(proxy, entry, src_port);
            (void) ipmc_lib_packet_tx_ssq(fltr, snd_act, grp_ptr, entry, src_port, &tmp1_srclist[IPMC_INTF_IS_MVR_VAL(entry)], FALSE);

            if (snd_act != IPMC_SND_HOLD) { /* Querier */
                grp_info->state = IPMC_OP_CHK_LISTENER;
                /* start rxmt timer */
                grp_info->rxmt_count[src_port] = IPMC_TIMER_LLQC(entry);
                IPMC_TIMER_LLQI_SET(entry, &grp_info->rxmt_timer[src_port], rxmt, grp_info);
            }

            v = ipmc_lib_get_system_local_port_cnt();
            /* Filter(Group) Timer = MALI */
            IPMC_TIMER_MALI_SET(v, entry, &grp_db->tmr.fltr_timer.t[src_port], fltr, grp_db);
        } /* VTSS_IPMC_SF_STATUS_ENABLED | VTSS_IPMC_SF_STATUS_DISABLED */

        grp_op = grp_ptr;
        ipmc_lib_srclist_logical_op_cmp(&grp_is_changed, src_port, grp_op->info->db.ipmc_sf_do_forward_srclist, &allow_list_tmp4rcv[IPMC_INTF_IS_MVR_VAL(entry)]);
        if (!grp_is_changed) {
            ipmc_lib_srclist_logical_op_cmp(&grp_is_changed, src_port, grp_op->info->db.ipmc_sf_do_not_forward_srclist, &block_list_tmp4rcv[IPMC_INTF_IS_MVR_VAL(entry)]);
        }
    } else {
        if ((grp_op = ipmc_lib_group_init(entry, p, &sfm_grp_tmp)) != NULL) {
            grp_op->info->grp = grp_op;
            grp_op->info->db.grp = grp_op;

            ipmc_lib_proc_grp_sfm_tmp4rcv(IPMC_INTF_IS_MVR_VAL(entry), FALSE, FALSE, grp_op);

            grp_info = grp_op->info;
            grp_db = &grp_info->db;

            /*
                Router State: INCLUDE (0)
                Report Received: TO_EX (B)
                New Router State: EXCLUDE (0, B)
                Actions:(B) = 0
                        Delete (0)
                        Send Q(MA, 0)
                        Filter(Group) Timer = MALI
            */
            asm_chg = TRUE;

            /* EXCLUDE (0, B) */
            VTSS_PORT_BF_SET(grp_db->ipmc_sf_port_status, src_port, VTSS_IPMC_SF_STATUS_ENABLED);
            VTSS_PORT_BF_SET(grp_db->ipmc_sf_port_mode, src_port, VTSS_IPMC_SF_MODE_EXCLUDE);
            if (!ipmc_lib_grp_src_list_del4port(srct, src_port, grp_db->ipmc_sf_do_forward_srclist)) {
                T_D("ipmc_lib_grp_src_list_del4port failed");
            }
            if (!ipmc_lib_grp_src_list_del4port(srct, src_port, grp_db->ipmc_sf_do_not_forward_srclist)) {
                T_D("ipmc_lib_grp_src_list_del4port failed");
            }
            if (!ipmc_lib_srclist_logical_op_pkt(
                    grp_ptr,
                    entry,
                    src_port,
                    VTSS_IPMC_SFM_OP_OR,
                    grp_db->ipmc_sf_do_not_forward_srclist,
                    group_record,
                    num_of_src, TRUE, srct)) {
                T_D("ipmc_lib_srclist_logical_op_pkt() failed");
            }

            /* (B) = 0 */
            if (!ipmc_lib_protocol_update_sublist_timer_pkt(
                    entry,
                    srct,
                    grp_db->ipmc_sf_do_not_forward_srclist,
                    group_record,
                    num_of_src,
                    NULL,
                    src_port)) {
                T_D("ipmc_lib_protocol_update_sublist_timer_pkt() failed");
            }

            /* Delete (0) */

            /* Send Q(MA, 0) */
            if (!ipmc_lib_srclist_clear(&tmp1_srclist[IPMC_INTF_IS_MVR_VAL(entry)], 90)) {
                T_D("ipmc_lib_srclist_clear failed");
            }
            snd_act = ipmc_lib_get_sq_ssq_action(proxy, entry, src_port);
            (void) ipmc_lib_packet_tx_ssq(fltr, snd_act, grp_ptr, entry, src_port, &tmp1_srclist[IPMC_INTF_IS_MVR_VAL(entry)], FALSE);

            if (snd_act != IPMC_SND_HOLD) { /* Querier */
                grp_info->state = IPMC_OP_CHK_LISTENER;
                /* start rxmt timer */
                grp_info->rxmt_count[src_port] = IPMC_TIMER_LLQC(entry);
                IPMC_TIMER_LLQI_SET(entry, &grp_info->rxmt_timer[src_port], rxmt, grp_info);
            }

            v = ipmc_lib_get_system_local_port_cnt();
            /* Filter(Group) Timer = MALI */
            IPMC_TIMER_MALI_SET(v, entry, &grp_db->tmr.fltr_timer.t[src_port], fltr, grp_db);

            grp_is_changed = TRUE;
        }
    } /* if this group already exists */

    if (asm_chg) {
        if (!ipmc_lib_group_update(p, grp_op)) {
            T_D("update group with chip failed!!!");
        }
    }
    if (grp_is_changed) {
        if (!ipmc_lib_group_sync(p, entry, grp_op, FALSE, PROC4RCV)) {
            T_D("sync group with chip failed!!!");
        }
    }

    if (lower_source_timer) {
        ipmc_lib_protocol_lower_source_timer(srct, grp_op,
                                             entry,
                                             &tmp2_srclist[IPMC_INTF_IS_MVR_VAL(entry)],
                                             src_port);
    }
}

static void _ipmc_lib_sfm_allow_new_source(ipmc_db_ctrl_hdr_t *p,
                                           ipmc_db_ctrl_hdr_t *rxmt,
                                           ipmc_db_ctrl_hdr_t *fltr,
                                           ipmc_db_ctrl_hdr_t *srct,
                                           ipmc_group_entry_t *grp_ptr,
                                           ipmc_intf_entry_t *entry,
                                           u8 src_port,
                                           vtss_ipv6_t *group_address,
                                           void *group_record,
                                           u16 num_of_src,
                                           BOOL proxy, ipmc_time_t *sfm_grp_tmr)
{
    BOOL                asm_chg, grp_is_changed;
    ipmc_group_entry_t  *grp_op, sfm_grp_tmp;
    ipmc_group_info_t   *grp_info;
    ipmc_group_db_t     *grp_db;

    if (!p || !entry || !group_address || !group_record) {
        return;
    }

    asm_chg = grp_is_changed = FALSE;
    if (grp_ptr == NULL) {
        sfm_grp_tmp.vid = entry->param.vid;
        memcpy(&sfm_grp_tmp.group_addr, group_address, sizeof(vtss_ipv6_t));
        sfm_grp_tmp.ipmc_version = entry->ipmc_version;

        grp_ptr = ipmc_lib_group_ptr_get(p, &sfm_grp_tmp);
    }
    if (grp_ptr != NULL) {
        ipmc_lib_proc_grp_sfm_tmp4rcv(IPMC_INTF_IS_MVR_VAL(entry), FALSE, TRUE, grp_ptr);

        grp_info = grp_ptr->info;
        grp_db = &grp_info->db;

        if (IPMC_LIB_GRP_PORT_DO_SFM(grp_db, src_port)) {
            VTSS_PORT_BF_SET(grp_db->ipmc_sf_port_status, src_port, VTSS_IPMC_SF_STATUS_ENABLED);

            if (IPMC_LIB_GRP_PORT_SFM_IN(grp_db, src_port)) {
                /*
                    Router State: INCLUDE (A)
                    Report Received: ALLOW (B)
                    New Router State: INCLUDE (A + B)
                    Actions:(B) = MALI
                */
                /* INCLUDE (A + B) */
                if (!ipmc_lib_srclist_logical_op_pkt(
                        grp_ptr,
                        entry,
                        src_port,
                        VTSS_IPMC_SFM_OP_OR,
                        grp_db->ipmc_sf_do_forward_srclist,
                        group_record,
                        num_of_src, TRUE, srct)) {
                    T_D("ipmc_lib_srclist_logical_op_pkt() failed");
                }

                /* (B) = MALI */
                if (!ipmc_lib_protocol_update_sublist_timer_pkt(
                        entry,
                        srct,
                        grp_db->ipmc_sf_do_forward_srclist,
                        group_record,
                        num_of_src,
                        sfm_grp_tmr,
                        src_port)) {
                    T_D("ipmc_lib_protocol_update_sublist_timer_pkt() failed");
                }
                if (!ipmc_lib_protocol_update_sublist_timer_pkt(
                        entry,
                        srct,
                        grp_db->ipmc_sf_do_not_forward_srclist,
                        group_record,
                        num_of_src,
                        sfm_grp_tmr,
                        src_port)) {
                    T_D("ipmc_lib_protocol_update_sublist_timer_pkt() failed");
                }
            } else {
                /*
                    Router State: EXCLUDE (X, Y)
                    Report Received: ALLOW (A)
                    New Router State: EXCLUDE (X + A, Y - A)
                    Actions:(A) = MALI
                */
                /* EXCLUDE (X + A, Y - A) */
                if (!ipmc_lib_srclist_logical_op_pkt(
                        grp_ptr,
                        entry,
                        src_port,
                        VTSS_IPMC_SFM_OP_OR,
                        grp_db->ipmc_sf_do_forward_srclist,
                        group_record,
                        num_of_src, TRUE, srct)) {
                    T_D("ipmc_lib_srclist_logical_op_pkt() failed");
                }
                if (!ipmc_lib_srclist_logical_op_pkt(
                        grp_ptr,
                        entry,
                        src_port,
                        VTSS_IPMC_SFM_OP_DIFF,
                        grp_db->ipmc_sf_do_not_forward_srclist,
                        group_record,
                        num_of_src, TRUE, srct)) {
                    T_D("ipmc_lib_srclist_logical_op_pkt() failed");
                }

                /* (A) = MALI */
                if (!ipmc_lib_protocol_update_sublist_timer_pkt(
                        entry,
                        srct,
                        grp_db->ipmc_sf_do_forward_srclist,
                        group_record,
                        num_of_src,
                        sfm_grp_tmr,
                        src_port)) {
                    T_D("ipmc_lib_protocol_update_sublist_timer_pkt() failed");
                }
                if (!ipmc_lib_protocol_update_sublist_timer_pkt(
                        entry,
                        srct,
                        grp_db->ipmc_sf_do_not_forward_srclist,
                        group_record,
                        num_of_src,
                        sfm_grp_tmr,
                        src_port)) {
                    T_D("ipmc_lib_protocol_update_sublist_timer_pkt() failed");
                }
            } /* VTSS_IPMC_SF_MODE_INCLUDE | VTSS_IPMC_SF_MODE_EXCLUDE */
        } else {
            /*
                Router State: INCLUDE (0)
                Report Received: ALLOW (B)
                New Router State: INCLUDE (B)
                Actions:(B) = MALI
            */
            asm_chg = TRUE;

            /* INCLUDE (B) */
            VTSS_PORT_BF_SET(grp_db->ipmc_sf_port_status, src_port, VTSS_IPMC_SF_STATUS_ENABLED);
            VTSS_PORT_BF_SET(grp_db->ipmc_sf_port_mode, src_port, VTSS_IPMC_SF_MODE_INCLUDE);
            if (!ipmc_lib_grp_src_list_del4port(srct, src_port, grp_db->ipmc_sf_do_forward_srclist)) {
                T_D("ipmc_lib_grp_src_list_del4port failed");
            }
            if (!ipmc_lib_grp_src_list_del4port(srct, src_port, grp_db->ipmc_sf_do_not_forward_srclist)) {
                T_D("ipmc_lib_grp_src_list_del4port failed");
            }

            if (!ipmc_lib_srclist_logical_op_pkt(
                    grp_ptr,
                    entry,
                    src_port,
                    VTSS_IPMC_SFM_OP_OR,
                    grp_db->ipmc_sf_do_forward_srclist,
                    group_record,
                    num_of_src, TRUE, srct)) {
                T_D("ipmc_lib_srclist_logical_op_pkt() failed");
            }

            /* (B) = MALI */
            if (!ipmc_lib_protocol_update_sublist_timer_pkt(
                    entry,
                    srct,
                    grp_db->ipmc_sf_do_forward_srclist,
                    group_record,
                    num_of_src,
                    sfm_grp_tmr,
                    src_port)) {
                T_D("ipmc_lib_protocol_update_sublist_timer_pkt() failed");
            }
            if (!ipmc_lib_protocol_update_sublist_timer_pkt(
                    entry,
                    srct,
                    grp_db->ipmc_sf_do_not_forward_srclist,
                    group_record,
                    num_of_src,
                    sfm_grp_tmr,
                    src_port)) {
                T_D("ipmc_lib_protocol_update_sublist_timer_pkt() failed");
            }
        } /* VTSS_IPMC_SF_STATUS_ENABLED | VTSS_IPMC_SF_STATUS_DISABLED */

        grp_op = grp_ptr;
        ipmc_lib_srclist_logical_op_cmp(&grp_is_changed, src_port, grp_op->info->db.ipmc_sf_do_forward_srclist, &allow_list_tmp4rcv[IPMC_INTF_IS_MVR_VAL(entry)]);
        if (!grp_is_changed) {
            ipmc_lib_srclist_logical_op_cmp(&grp_is_changed, src_port, grp_op->info->db.ipmc_sf_do_not_forward_srclist, &block_list_tmp4rcv[IPMC_INTF_IS_MVR_VAL(entry)]);
        }
    } else {
        if ((grp_op = ipmc_lib_group_init(entry, p, &sfm_grp_tmp)) != NULL) {
            grp_op->info->grp = grp_op;
            grp_op->info->db.grp = grp_op;

            ipmc_lib_proc_grp_sfm_tmp4rcv(IPMC_INTF_IS_MVR_VAL(entry), FALSE, FALSE, grp_op);

            grp_info = grp_op->info;
            grp_db = &grp_info->db;

            /*
                Router State: INCLUDE (0)
                Report Received: ALLOW (B)
                New Router State: INCLUDE (B)
                Actions:(B) = MALI
            */
            asm_chg = TRUE;

            VTSS_PORT_BF_SET(grp_db->ipmc_sf_port_status, src_port, VTSS_IPMC_SF_STATUS_ENABLED);
            VTSS_PORT_BF_SET(grp_db->ipmc_sf_port_mode, src_port, VTSS_IPMC_SF_MODE_INCLUDE);

            /* INCLUDE (B) */
            if (!ipmc_lib_grp_src_list_del4port(srct, src_port, grp_db->ipmc_sf_do_forward_srclist)) {
                T_D("ipmc_lib_grp_src_list_del4port failed");
            }
            if (!ipmc_lib_grp_src_list_del4port(srct, src_port, grp_db->ipmc_sf_do_not_forward_srclist)) {
                T_D("ipmc_lib_grp_src_list_del4port failed");
            }

            if (!ipmc_lib_srclist_logical_op_pkt(
                    grp_ptr,
                    entry,
                    src_port,
                    VTSS_IPMC_SFM_OP_OR,
                    grp_db->ipmc_sf_do_forward_srclist,
                    group_record,
                    num_of_src, TRUE, srct)) {
                T_D("ipmc_lib_srclist_logical_op_pkt() failed");
            }

            /* (B) = MALI */
            if (!ipmc_lib_protocol_update_sublist_timer_pkt(
                    entry,
                    srct,
                    grp_db->ipmc_sf_do_forward_srclist,
                    group_record,
                    num_of_src,
                    sfm_grp_tmr,
                    src_port)) {
                T_D("ipmc_lib_protocol_update_sublist_timer_pkt() failed");
            }
            if (!ipmc_lib_protocol_update_sublist_timer_pkt(
                    entry,
                    srct,
                    grp_db->ipmc_sf_do_not_forward_srclist,
                    group_record,
                    num_of_src,
                    sfm_grp_tmr,
                    src_port)) {
                T_D("ipmc_lib_protocol_update_sublist_timer_pkt() failed");
            }

            grp_is_changed = TRUE;
        }
    } /* if this group already exists */

    if (asm_chg) {
        if (!ipmc_lib_group_update(p, grp_op)) {
            T_D("update group with chip failed!!!");
        }
    }
    if (grp_is_changed) {
        if (!ipmc_lib_group_sync(p, entry, grp_op, FALSE, PROC4RCV)) {
            T_D("sync group with chip failed!!!");
        }
    }
}

static void _ipmc_lib_sfm_block_old_source(ipmc_db_ctrl_hdr_t *p,
                                           ipmc_db_ctrl_hdr_t *rxmt,
                                           ipmc_db_ctrl_hdr_t *fltr,
                                           ipmc_db_ctrl_hdr_t *srct,
                                           ipmc_group_entry_t *grp_ptr,
                                           ipmc_intf_entry_t *entry,
                                           u8 src_port,
                                           vtss_ipv6_t *group_address,
                                           void *group_record,
                                           u16 num_of_src,
                                           BOOL proxy, ipmc_time_t *sfm_grp_tmr)
{
    BOOL                grp_is_changed, lower_source_timer;
    ipmc_group_entry_t  *grp_op, sfm_grp_tmp;
    ipmc_group_info_t   *grp_info;
    ipmc_group_db_t     *grp_db;
    ipmc_send_act_t     snd_act;

    if (!p || !entry || !group_address || !group_record) {
        return;
    }

    grp_is_changed = lower_source_timer = FALSE;
    if (grp_ptr == NULL) {
        sfm_grp_tmp.vid = entry->param.vid;
        memcpy(&sfm_grp_tmp.group_addr, group_address, sizeof(vtss_ipv6_t));
        sfm_grp_tmp.ipmc_version = entry->ipmc_version;

        grp_ptr = ipmc_lib_group_ptr_get(p, &sfm_grp_tmp);
    }
    if (grp_ptr != NULL) {
        ipmc_lib_proc_grp_sfm_tmp4rcv(IPMC_INTF_IS_MVR_VAL(entry), FALSE, TRUE, grp_ptr);

        grp_info = grp_ptr->info;
        grp_db = &grp_info->db;

        if (IPMC_LIB_GRP_PORT_DO_SFM(grp_db, src_port)) {
            VTSS_PORT_BF_SET(grp_db->ipmc_sf_port_status, src_port, VTSS_IPMC_SF_STATUS_ENABLED);

            if (IPMC_LIB_GRP_PORT_SFM_IN(grp_db, src_port)) {
                /*
                    Router State: INCLUDE (A)
                    Report Received: BLOCK (B)
                    New Router State: INCLUDE (A)
                    Actions: Send Q(MA, A * B)
                */
                /* INCLUDE (A) */

                /* tmp1_srclist = A * B */
                if (!ipmc_lib_srclist_struct_copy(
                        grp_ptr,
                        &tmp1_srclist[IPMC_INTF_IS_MVR_VAL(entry)],
                        grp_db->ipmc_sf_do_forward_srclist,
                        src_port)) {
                    T_D("vtss_ipmc_data_struct_copy() failed");
                }
                if (!ipmc_lib_srclist_logical_op_pkt(
                        grp_ptr,
                        entry,
                        src_port,
                        VTSS_IPMC_SFM_OP_AND,
                        &tmp1_srclist[IPMC_INTF_IS_MVR_VAL(entry)],
                        group_record,
                        num_of_src, FALSE, srct)) {
                    T_D("ipmc_lib_srclist_logical_op_pkt() failed");
                }

                /* Send Q(MA, A * B) */
                snd_act = ipmc_lib_get_sq_ssq_action(proxy, entry, src_port);
                (void) ipmc_lib_packet_tx_ssq(fltr, snd_act, grp_ptr, entry, src_port, &tmp1_srclist[IPMC_INTF_IS_MVR_VAL(entry)], FALSE);

                if (snd_act != IPMC_SND_HOLD) { /* Querier */
                    grp_info->state = IPMC_OP_CHK_LISTENER;
                    /* start rxmt timer */
                    grp_info->rxmt_count[src_port] = IPMC_TIMER_LLQC(entry);
                    IPMC_TIMER_LLQI_SET(entry, &grp_info->rxmt_timer[src_port], rxmt, grp_info);
                }

                lower_source_timer = TRUE;
            } else {
                /*
                    Router State: EXCLUDE (X, Y)
                    Report Received: BLOCK (A)
                    New Router State: EXCLUDE (X + (A - Y), Y)
                    Actions:(A - X - Y) = Filter(Group) Timer
                            Send Q(MA, A - Y)
                */
                /* tmp1_srclist = (A - Y) */
                if (!ipmc_lib_srclist_clear(&tmp1_srclist[IPMC_INTF_IS_MVR_VAL(entry)], 80)) {
                    T_D("ipmc_lib_srclist_clear failed");
                }
                if (!ipmc_lib_srclist_logical_op_pkt(
                        grp_ptr,
                        entry,
                        src_port,
                        VTSS_IPMC_SFM_OP_OR,
                        &tmp1_srclist[IPMC_INTF_IS_MVR_VAL(entry)],
                        group_record,
                        num_of_src, FALSE, srct)) {
                    T_D("ipmc_lib_srclist_logical_op_pkt() failed");
                }
                if (!ipmc_lib_srclist_logical_op_set(
                        grp_ptr,
                        src_port,
                        VTSS_IPMC_SFM_OP_DIFF, FALSE, srct,
                        &tmp1_srclist[IPMC_INTF_IS_MVR_VAL(entry)],
                        grp_db->ipmc_sf_do_not_forward_srclist)) {
                    T_D("ipmc_lib_srclist_logical_op_set() failed");
                }

                /* Timer in (A * X) should not be touch, since (A - X - Y) = Filter(Group) Timer  */
                (void) ipmc_lib_protocol_keep_union_list_timer(&tmp1_srclist[IPMC_INTF_IS_MVR_VAL(entry)], grp_db->ipmc_sf_do_forward_srclist, src_port);

                /* tmp2_srclist = (A - X - Y) = ((A - Y) - X) */
                if (!ipmc_lib_srclist_struct_copy(
                        grp_ptr,
                        &tmp2_srclist[IPMC_INTF_IS_MVR_VAL(entry)],
                        &tmp1_srclist[IPMC_INTF_IS_MVR_VAL(entry)],
                        src_port)) {
                    T_D("vtss_ipmc_data_struct_copy() failed");
                }
                if (!ipmc_lib_srclist_logical_op_set(
                        grp_ptr,
                        src_port,
                        VTSS_IPMC_SFM_OP_DIFF, FALSE, srct,
                        &tmp2_srclist[IPMC_INTF_IS_MVR_VAL(entry)],
                        grp_db->ipmc_sf_do_forward_srclist)) {
                    T_D("ipmc_lib_srclist_logical_op_set() failed");
                }

                /* EXCLUDE (X + (A - Y), Y) */
                if (!ipmc_lib_srclist_logical_op_set(
                        grp_ptr,
                        src_port,
                        VTSS_IPMC_SFM_OP_OR, TRUE, srct,
                        grp_db->ipmc_sf_do_forward_srclist,
                        &tmp1_srclist[IPMC_INTF_IS_MVR_VAL(entry)])) {
                    T_D("ipmc_lib_srclist_logical_op_set() failed");
                }

                /* (A - X - Y) = Filter(Group) Timer */
                if (ipmc_lib_protocol_update_srclist_subset_timer(
                        srct,
                        grp_db->ipmc_sf_do_forward_srclist,
                        &tmp2_srclist[IPMC_INTF_IS_MVR_VAL(entry)],
                        &grp_db->tmr.fltr_timer.t[src_port],
                        src_port) != TRUE) {
                    T_D("ipmc_lib_protocol_update_srclist_subset_timer() failed");
                }
                if (ipmc_lib_protocol_update_srclist_subset_timer(
                        srct,
                        grp_db->ipmc_sf_do_not_forward_srclist,
                        &tmp2_srclist[IPMC_INTF_IS_MVR_VAL(entry)],
                        &grp_db->tmr.fltr_timer.t[src_port],
                        src_port) != TRUE) {
                    T_D("ipmc_lib_protocol_update_srclist_subset_timer() failed");
                }

                /* Send Q(MA, A - Y) */
                snd_act = ipmc_lib_get_sq_ssq_action(proxy, entry, src_port);
                (void) ipmc_lib_packet_tx_ssq(fltr, snd_act, grp_ptr, entry, src_port, &tmp1_srclist[IPMC_INTF_IS_MVR_VAL(entry)], FALSE);

                if (snd_act != IPMC_SND_HOLD) { /* Querier */
                    grp_info->state = IPMC_OP_CHK_LISTENER;
                    /* start rxmt timer */
                    grp_info->rxmt_count[src_port] = IPMC_TIMER_LLQC(entry);
                    IPMC_TIMER_LLQI_SET(entry, &grp_info->rxmt_timer[src_port], rxmt, grp_info);
                }

                lower_source_timer = TRUE;
            } /* VTSS_IPMC_SF_MODE_INCLUDE | VTSS_IPMC_SF_MODE_EXCLUDE */
        }  else {
            /*
                Router State: INCLUDE (0)
                Report Received: BLOCK (B)
                New Router State: INCLUDE (0)
                Actions: Send Q(MA, 0)
            */
            /* INCLUDE (0) */
            VTSS_PORT_BF_SET(grp_db->ipmc_sf_port_status, src_port, VTSS_IPMC_SF_STATUS_ENABLED);
            VTSS_PORT_BF_SET(grp_db->ipmc_sf_port_mode, src_port, VTSS_IPMC_SF_MODE_INCLUDE);
            if (!ipmc_lib_grp_src_list_del4port(srct, src_port, grp_db->ipmc_sf_do_forward_srclist)) {
                T_D("ipmc_lib_grp_src_list_del4port failed");
            }
            if (!ipmc_lib_grp_src_list_del4port(srct, src_port, grp_db->ipmc_sf_do_not_forward_srclist)) {
                T_D("ipmc_lib_grp_src_list_del4port failed");
            }

            /* Send Q(MA, 0) */
            if (!ipmc_lib_srclist_clear(&tmp1_srclist[IPMC_INTF_IS_MVR_VAL(entry)], 70)) {
                T_D("ipmc_lib_srclist_clear failed");
            }
            snd_act = ipmc_lib_get_sq_ssq_action(proxy, entry, src_port);
            (void) ipmc_lib_packet_tx_ssq(fltr, snd_act, grp_ptr, entry, src_port, &tmp1_srclist[IPMC_INTF_IS_MVR_VAL(entry)], FALSE);

            if (snd_act != IPMC_SND_HOLD) { /* Querier */
                grp_info->state = IPMC_OP_CHK_LISTENER;
                /* start rxmt timer */
                grp_info->rxmt_count[src_port] = IPMC_TIMER_LLQC(entry);
                IPMC_TIMER_LLQI_SET(entry, &grp_info->rxmt_timer[src_port], rxmt, grp_info);
            } else {
                (void) ipmc_lib_protocol_lower_filter_timer(fltr, grp_ptr, entry, src_port);
            }
        } /* VTSS_IPMC_SF_STATUS_ENABLED | VTSS_IPMC_SF_STATUS_DISABLED */

        grp_op = grp_ptr;
        ipmc_lib_srclist_logical_op_cmp(&grp_is_changed, src_port, grp_op->info->db.ipmc_sf_do_forward_srclist, &allow_list_tmp4rcv[IPMC_INTF_IS_MVR_VAL(entry)]);
        if (!grp_is_changed) {
            ipmc_lib_srclist_logical_op_cmp(&grp_is_changed, src_port, grp_op->info->db.ipmc_sf_do_not_forward_srclist, &block_list_tmp4rcv[IPMC_INTF_IS_MVR_VAL(entry)]);
        }
    } else {
        if ((grp_op = ipmc_lib_group_init(entry, p, &sfm_grp_tmp)) != NULL) {
            grp_op->info->grp = grp_op;
            grp_op->info->db.grp = grp_op;

            ipmc_lib_proc_grp_sfm_tmp4rcv(IPMC_INTF_IS_MVR_VAL(entry), FALSE, FALSE, grp_op);

            grp_info = grp_op->info;
            grp_db = &grp_info->db;

            /*
                Router State: INCLUDE (0)
                Report Received: BLOCK (B)
                New Router State: INCLUDE (0)
                Actions: Send Q(MA, 0)
            */
            VTSS_PORT_BF_SET(grp_db->ipmc_sf_port_status, src_port, VTSS_IPMC_SF_STATUS_ENABLED);
            VTSS_PORT_BF_SET(grp_db->ipmc_sf_port_mode, src_port, VTSS_IPMC_SF_MODE_INCLUDE);
            if (!ipmc_lib_grp_src_list_del4port(srct, src_port, grp_db->ipmc_sf_do_forward_srclist)) {
                T_D("ipmc_lib_grp_src_list_del4port failed");
            }
            if (!ipmc_lib_grp_src_list_del4port(srct, src_port, grp_db->ipmc_sf_do_not_forward_srclist)) {
                T_D("ipmc_lib_grp_src_list_del4port failed");
            }

            /* Send Q(MA, 0) */
            if (!ipmc_lib_srclist_clear(&tmp1_srclist[IPMC_INTF_IS_MVR_VAL(entry)], 60)) {
                T_D("ipmc_lib_srclist_clear failed");
            }
            snd_act = ipmc_lib_get_sq_ssq_action(proxy, entry, src_port);
            (void) ipmc_lib_packet_tx_ssq(fltr, snd_act, grp_ptr, entry, src_port, &tmp1_srclist[IPMC_INTF_IS_MVR_VAL(entry)], FALSE);

            if (snd_act != IPMC_SND_HOLD) { /* Querier */
                grp_info->state = IPMC_OP_CHK_LISTENER;
                /* start rxmt timer */
                grp_info->rxmt_count[src_port] = IPMC_TIMER_LLQC(entry);
                IPMC_TIMER_LLQI_SET(entry, &grp_info->rxmt_timer[src_port], rxmt, grp_info);
            } else {
                (void) ipmc_lib_protocol_lower_filter_timer(fltr, grp_op, entry, src_port);
            }

            grp_is_changed = TRUE;
        }
    } /* if this group already exists */

    if (grp_is_changed) {
        if (!ipmc_lib_group_sync(p, entry, grp_op, FALSE, PROC4RCV)) {
            T_D("sync group with chip failed!!!");
        }
    }

    if (lower_source_timer) {
        ipmc_lib_protocol_lower_source_timer(srct, grp_op,
                                             entry,
                                             &tmp1_srclist[IPMC_INTF_IS_MVR_VAL(entry)],
                                             src_port);
    }
}

static ipmc_group_entry_t *ipmc_lib_protocol_do_asm(ipmc_db_ctrl_hdr_t *p,
                                                    ipmc_db_ctrl_hdr_t *rxmt,
                                                    ipmc_db_ctrl_hdr_t *fltr,
                                                    ipmc_db_ctrl_hdr_t *srct,
                                                    ipmc_intf_entry_t *entry,
                                                    u8 src_port,
                                                    vtss_ipv6_t *group_address,
                                                    u8 msgType,
                                                    BOOL ssm_range,
                                                    specific_grps_fltr_t *grps_fltr,
                                                    int *throttling,
                                                    BOOL proxy,
                                                    ipmc_operation_action_t *op,
                                                    BOOL *do_status)
{
    ipmc_group_entry_t  *grp, grp_buf;
    ipmc_group_info_t   *grp_info;
    ipmc_group_db_t     *grp_db;
    BOOL                apply_fwd;
    u16                 vid;
    ipmc_ip_version_t   version;

    if (!p || !entry || !group_address) {
        *op = IPMC_OP_ERR;
        *do_status = FALSE;
        return NULL;
    }

    vid = entry->param.vid;
    version = entry->ipmc_version;

    if (grps_fltr && grps_fltr->filter_cnt) {
        u32 idx;

        for (idx = grps_fltr->filter_cnt; idx > 0; idx--) {
            if (IPMC_LIB_DENY(grps_fltr->pdx[idx - 1], grps_fltr->port, &grps_fltr->vid, group_address, &grps_fltr->src)) {
                *op = IPMC_OP_INT;
                *do_status = FALSE;
                return NULL;
            }
        }
    }

    /* lookup group for address */
    grp_buf.vid = vid;
    grp_buf.ipmc_version = version;
    IPMC_LIB_ADRS_CPY(&grp_buf.group_addr, group_address);
    if ((grp = ipmc_lib_group_ptr_get(p, &grp_buf)) == NULL) {
        if (throttling) {
            T_D("IN-THROTTLED:%d", *throttling);
            if (*throttling == IPMC_REPORT_THROTTLED) {
                *op = IPMC_OP_INT;
                *do_status = FALSE;
                return grp;
            }

            if (*throttling != IPMC_REPORT_NORMAL) {
                if (--(*throttling) == 0x0) {
                    *throttling = IPMC_REPORT_THROTTLED;
                }
            }
            T_D("OUT-THROTTLED:%d", *throttling);
        }

        /* Notify Routing + */
        grp = ipmc_lib_group_init(entry, p, &grp_buf);
        if (grp == NULL) {
            T_D("init group failed!!!");
            *op = IPMC_OP_ERR;
            *do_status = FALSE;
            return grp;
        } else {
            grp->info->grp = grp;
            grp->info->db.grp = grp;
        }

        *op = IPMC_OP_ADD;
    } else {
        if (!VTSS_PORT_BF_GET(grp->info->db.port_mask, src_port) && throttling) {
            T_D("IN-THROTTLED:%d", *throttling);
            if (*throttling == IPMC_REPORT_THROTTLED) {
                *op = IPMC_OP_INT;
                *do_status = FALSE;
                return grp;
            }

            if (*throttling != IPMC_REPORT_NORMAL) {
                if (--(*throttling) == 0x0) {
                    *throttling = IPMC_REPORT_THROTTLED;
                }
            }
            T_D("OUT-THROTTLED:%d", *throttling);
        }

        *op = IPMC_OP_UPD;
    }

    ipmc_lib_proc_grp_sfm_tmp4rcv(IPMC_INTF_IS_MVR_VAL(entry), FALSE, FALSE, grp);
    grp_info = grp->info;
    grp_db = &grp_info->db;

    if (*op != IPMC_OP_ADD) {
        apply_fwd = (VTSS_PORT_BF_GET(grp_db->port_mask, src_port) == FALSE);
    } else {
        apply_fwd = TRUE;
    }
    VTSS_PORT_BF_SET(grp_db->port_mask, src_port, TRUE);
    IPMC_LIB_CHK_LISTENER_CLR(grp, src_port);

    if (apply_fwd) {
        if (!ipmc_lib_group_sync(p, entry, grp, TRUE, PROC4RCV)) {
            T_D("sync group with chip failed!!!");
        }
    }

    if (IPMC_LIB_BFS_HAS_MEMBER(grp_db->port_mask)) {
        /* Clear Retransmit Timer (Querier Only) */
        if (grp_info->state == IPMC_OP_CHK_LISTENER) {
            if (ipmc_lib_get_sq_ssq_action(proxy, entry, src_port) != IPMC_SND_HOLD) { /* Querier */
                T_D("CLEAR RXMT TMR");

                if (!IPMC_TIMER_ZERO(&grp_info->rxmt_timer[src_port])) {
                    if (IPMC_TIMER_EQUAL(&grp_info->rxmt_timer[src_port], &grp_info->min_tmr)) {
                        ipmc_group_info_t   info_tmp, *info_ptr;

                        info_ptr = &info_tmp;
                        IPMC_TIMER_RESET(&info_ptr->min_tmr);
                        info_ptr->grp = grp;
                        if (((info_ptr = ipmc_lib_rxmt_tmrlist_get_next(rxmt, info_ptr)) != NULL) &&
                            (info_ptr->grp == grp)) {
                            u8  ndx, local_port_cnt = ipmc_lib_get_system_local_port_cnt();

                            (void) IPMC_LIB_DB_DEL(rxmt, info_ptr);
                            IPMC_TIMER_RESET(&grp_info->min_tmr);
                            for (ndx = 0; ndx < local_port_cnt; ndx++) {
                                if ((ndx == src_port) || IPMC_TIMER_ZERO(&grp_info->rxmt_timer[ndx])) {
                                    continue;
                                }

                                if (IPMC_TIMER_ZERO(&grp_info->min_tmr)) {
                                    ipmc_lib_time_cpy(&grp_info->min_tmr, &grp_info->rxmt_timer[ndx]);
                                } else {
                                    if (IPMC_TIMER_GREATER(&grp_info->min_tmr, &grp_info->rxmt_timer[ndx])) {
                                        ipmc_lib_time_cpy(&grp_info->min_tmr, &grp_info->rxmt_timer[ndx]);
                                    }
                                }
                            }

                            if (!IPMC_TIMER_ZERO(&grp_info->min_tmr)) {
                                (void) IPMC_LIB_DB_ADD(rxmt, grp_info);
                            }
                        }
                    }

                    IPMC_TIMER_RESET(&grp_info->rxmt_timer[src_port]);
                }
                grp_info->rxmt_count[src_port] = 0;
            }
        }

        /* Set State &  Set Compatibility */
        grp_info->state = IPMC_OP_HAS_LISTENER;

        switch ( msgType ) {
        case IPMC_IGMP_MSG_TYPE_V1JOIN:
            grp_db->compatibility.mode = VTSS_IPMC_COMPAT_MODE_OLD;
            grp_db->compatibility.old_present_timer = IPMC_TIMER_OVHPT(entry);

            break;
        case IPMC_MLD_MSG_TYPE_V1REPORT:
        case IPMC_MLD_MSG_TYPE_DONE:
        case IPMC_IGMP_MSG_TYPE_V2JOIN:
        case IPMC_IGMP_MSG_TYPE_LEAVE:
            grp_db->compatibility.gen_present_timer = IPMC_TIMER_OVHPT(entry);
            if (grp_db->compatibility.mode == VTSS_IPMC_COMPAT_MODE_AUTO ||
                grp_db->compatibility.mode >= VTSS_IPMC_COMPAT_MODE_GEN) {
                grp_db->compatibility.mode = VTSS_IPMC_COMPAT_MODE_GEN;
            }

            break;
        case IPMC_MLD_MSG_TYPE_V2REPORT:
        case IPMC_IGMP_MSG_TYPE_V3JOIN:
            grp_db->compatibility.sfm_present_timer = IPMC_TIMER_OVHPT(entry);

            if (ssm_range) {
                grp_db->compatibility.mode = VTSS_IPMC_COMPAT_MODE_SFM;
                break;
            }

            if (grp_db->compatibility.mode == VTSS_IPMC_COMPAT_MODE_AUTO ||
                grp_db->compatibility.mode >= VTSS_IPMC_COMPAT_MODE_SFM) {
                grp_db->compatibility.mode = VTSS_IPMC_COMPAT_MODE_SFM;
            }

            break;
        default:
            grp_db->compatibility.old_present_timer = 0;
            grp_db->compatibility.gen_present_timer = 0;
            grp_db->compatibility.sfm_present_timer = 0;
            grp_db->compatibility.mode = VTSS_IPMC_COMPAT_MODE_AUTO;

            break;
        }
        if (entry->param.hst_compatibility.mode > grp_db->compatibility.mode) {
            memcpy(&entry->param.hst_compatibility, &grp_db->compatibility, sizeof(ipmc_compatibility_t));
        } else if (entry->param.hst_compatibility.mode == grp_db->compatibility.mode) {
            if (!entry->param.hst_compatibility.old_present_timer) {
                entry->param.hst_compatibility.old_present_timer = grp_db->compatibility.old_present_timer;
            }
            if (!entry->param.hst_compatibility.gen_present_timer) {
                entry->param.hst_compatibility.gen_present_timer = grp_db->compatibility.gen_present_timer;
            }
            if (!entry->param.hst_compatibility.sfm_present_timer) {
                entry->param.hst_compatibility.sfm_present_timer = grp_db->compatibility.sfm_present_timer;
            }
        } else {
            if (!ssm_range) {
                memcpy(&grp_db->compatibility, &entry->param.hst_compatibility, sizeof(ipmc_compatibility_t));
            } else {
                entry->param.hst_compatibility.old_present_timer = 0;
                entry->param.hst_compatibility.gen_present_timer = 0;
                if (!entry->param.hst_compatibility.sfm_present_timer) {
                    entry->param.hst_compatibility.sfm_present_timer = grp_db->compatibility.sfm_present_timer;
                }
            }
        }
    }

    *do_status = TRUE;
    return grp;
}

vtss_rc ipmc_lib_protocol_do_sfm_report(ipmc_db_ctrl_hdr_t *p,
                                        ipmc_db_ctrl_hdr_t *rxmt,
                                        ipmc_db_ctrl_hdr_t *fltr,
                                        ipmc_db_ctrl_hdr_t *srct,
                                        ipmc_intf_entry_t *entry,
                                        u8 *content,
                                        u8 src_port,
                                        u8 msgType,
                                        u32 ipmc_pkt_len,
                                        specific_grps_fltr_t *grps_fltr,
                                        int *throttling,
                                        BOOL proxy,
                                        ipmc_operation_action_t *op)
{
    ipmc_mld_packet_t           *mld;
    ipmc_igmp_packet_t          *igmp;
    u16                         number_of_record;

    char                        buf[40];
    igmp_group_record_t         *igmp_group_record;
    mld_group_record_t          *mld_group_record;
    ipmc_group_entry_t          *working_grp_ptr;
    u8                          *record_ptr;
    u8                          record_type;
    u8                          aux_len;
    u16                         no_of_sources;
    vtss_ipv6_t                 group_address;
    ipmc_prefix_t               grp_prefix, ssm_ver_prefix;
    u32                         current_pkt_len;

    vtss_rc                     rc;
    ipmc_time_t                 *mali, sfm_grp_tmr;
    BOOL                        grp_in_range, bypass_to_ex_source, do_asm_status;
    ipmc_ip_version_t           version;

    if (!p || !entry || !content) {
        return VTSS_RC_ERROR;
    }

    rc = VTSS_RC_ERROR;
    version = entry->ipmc_version;
    do_asm_status = bypass_to_ex_source = FALSE;
    memset(buf, 0x0, sizeof(buf));
    mld = NULL;
    igmp = NULL;
    record_ptr = NULL;

    /*
        Refer to RFC-3810 8.3.2 (RFC-3376 7.3.2)
    */
    switch ( msgType ) {
    case IPMC_IGMP_MSG_TYPE_V1JOIN:
        entry->param.hst_compatibility.mode = VTSS_IPMC_COMPAT_MODE_OLD;

        break;
    case IPMC_MLD_MSG_TYPE_V1REPORT:
    case IPMC_IGMP_MSG_TYPE_V2JOIN:
    case IPMC_MLD_MSG_TYPE_DONE:
    case IPMC_IGMP_MSG_TYPE_LEAVE:
        if (entry->param.hst_compatibility.mode == VTSS_IPMC_COMPAT_MODE_AUTO ||
            entry->param.hst_compatibility.mode >= VTSS_IPMC_COMPAT_MODE_GEN) {
            entry->param.hst_compatibility.mode = VTSS_IPMC_COMPAT_MODE_GEN;
        }

        break;
    case IPMC_MLD_MSG_TYPE_V2REPORT:
    case IPMC_IGMP_MSG_TYPE_V3JOIN:
        if (entry->param.hst_compatibility.mode == VTSS_IPMC_COMPAT_MODE_AUTO ||
            entry->param.hst_compatibility.mode >= VTSS_IPMC_COMPAT_MODE_SFM) {
            entry->param.hst_compatibility.mode = VTSS_IPMC_COMPAT_MODE_SFM;
        }

        break;
    default:

        break;
    }

    number_of_record = 0;
    current_pkt_len = 0;
    igmp_group_record = NULL;
    mld_group_record = NULL;
    record_type = IPMC_SFM_RECORD_TYPE_NONE;
    switch ( version ) {
    case IPMC_IP_VERSION_IGMP:
        igmp = (ipmc_igmp_packet_t *)content;
        current_pkt_len = sizeof(ipmc_igmp_packet_common_t) + sizeof(ipmcv4addr);
        igmp_group_record = (igmp_group_record_t *)(content + current_pkt_len);

        if (entry->param.hst_compatibility.mode == VTSS_IPMC_COMPAT_MODE_OLD) {
            if (msgType == IPMC_IGMP_MSG_TYPE_LEAVE) {
                return VTSS_OK;  /* IGNORE */
            }

            if (msgType == IPMC_IGMP_MSG_TYPE_V3JOIN) {
                if ((igmp_group_record->record_type == IPMC_SFM_BLOCK_OLD_SOURCES) ||
                    (igmp_group_record->record_type == IPMC_SFM_CHANGE_TO_INCLUDE)) {
                    return VTSS_OK;  /* IGNORE */
                }

                bypass_to_ex_source = TRUE;
            }

            number_of_record = 0x1;
        } else if (entry->param.hst_compatibility.mode == VTSS_IPMC_COMPAT_MODE_GEN) {
            if (msgType == IPMC_IGMP_MSG_TYPE_V3JOIN) {
                if (igmp_group_record->record_type == IPMC_SFM_BLOCK_OLD_SOURCES) {
                    return VTSS_OK;  /* IGNORE */
                }

                bypass_to_ex_source = TRUE;
            }

            number_of_record = 0x1;
        } else {
            number_of_record = ntohs(igmp->sfminfo.sfm_report.number_of_record);
        }

        record_ptr = (u8 *)igmp_group_record;

        break;
    case IPMC_IP_VERSION_MLD:
        mld = (ipmc_mld_packet_t *)content;
        current_pkt_len = sizeof(ipmc_mld_packet_common_t);
        mld_group_record = (mld_group_record_t *)(content + current_pkt_len);

        if (entry->param.hst_compatibility.mode == VTSS_IPMC_COMPAT_MODE_GEN) {
            if (msgType == IPMC_MLD_MSG_TYPE_V2REPORT) {
                if (mld_group_record->record_type == IPMC_SFM_BLOCK_OLD_SOURCES) {
                    return VTSS_OK;  /* IGNORE */
                }

                bypass_to_ex_source = TRUE;
            }

            number_of_record = 0x1;
        } else {
            number_of_record = ntohs(mld->common.number_of_record);
        }

        record_ptr = (u8 *)mld_group_record;

        break;
    default:
        return rc;
    }

    rc = VTSS_OK;
    while ((record_ptr != NULL) && (number_of_record != 0)) {
        /* pkt length checking */
        if (current_pkt_len > ipmc_pkt_len) {
            rc = VTSS_RC_ERROR;
            break;
        }

        aux_len = 0x0;
        no_of_sources = 0x0;
        memset(&group_address, 0x0, sizeof(vtss_ipv6_t));
        if ((version == IPMC_IP_VERSION_IGMP) && igmp_group_record && igmp) {
            if ((entry->param.hst_compatibility.mode == VTSS_IPMC_COMPAT_MODE_GEN) ||
                (entry->param.hst_compatibility.mode == VTSS_IPMC_COMPAT_MODE_OLD)) {
                memcpy(&group_address.addr[12], &igmp->sfminfo.usual.group_address, sizeof(ipmcv4addr));

                if (msgType == IPMC_IGMP_MSG_TYPE_V1JOIN) {
                    record_type = IPMC_SFM_MODE_IS_EXCLUDE;
                } else if (msgType == IPMC_IGMP_MSG_TYPE_V2JOIN) {
                    record_type = IPMC_SFM_MODE_IS_EXCLUDE;
                } else if (msgType == IPMC_IGMP_MSG_TYPE_LEAVE) {
                    record_type = IPMC_SFM_CHANGE_TO_INCLUDE;
                } else {
                    rc = VTSS_RC_ERROR;
                    break;
                }
            } else {
                memcpy(&group_address.addr[12], &igmp_group_record->group_address, sizeof(ipmcv4addr));
                record_type = igmp_group_record->record_type;
                aux_len = igmp_group_record->aux_len;
                no_of_sources = ntohs(igmp_group_record->no_of_sources);

                if ((record_type == IPMC_SFM_CHANGE_TO_EXCLUDE) && bypass_to_ex_source) {
                    no_of_sources = 0x0;
                }
            }

            if ((group_address.addr[12] < 0xE0) || (group_address.addr[12] > 0xEF)) {
                record_type = IPMC_SFM_RECORD_TYPE_ERR;
            }
        } else if ((version == IPMC_IP_VERSION_MLD) && mld_group_record && mld) {
            if (entry->param.hst_compatibility.mode == VTSS_IPMC_COMPAT_MODE_GEN) {
                memcpy(&group_address, &mld->sfminfo.usual.group_address, sizeof(vtss_ipv6_t));

                if (msgType == IPMC_MLD_MSG_TYPE_V1REPORT) {
                    record_type = IPMC_SFM_MODE_IS_EXCLUDE;
                } else if (msgType == IPMC_MLD_MSG_TYPE_DONE) {
                    record_type = IPMC_SFM_CHANGE_TO_INCLUDE;
                } else {
                    rc = VTSS_RC_ERROR;
                    break;
                }
            } else {
                memcpy(&group_address, &mld_group_record->group_address, sizeof(vtss_ipv6_t));
                record_type = mld_group_record->record_type;
                aux_len = mld_group_record->aux_len;
                no_of_sources = ntohs(mld_group_record->no_of_sources);

                if ((record_type == IPMC_SFM_CHANGE_TO_EXCLUDE) && bypass_to_ex_source) {
                    no_of_sources = 0x0;
                }
            }

            if (group_address.addr[0] != 0xFF) {
                record_type = IPMC_SFM_RECORD_TYPE_ERR;
            }
        }

        grp_in_range = FALSE;
        if (!entry->param.mvr && ipmc_lib_get_ssm_range(version, &ssm_ver_prefix)) {
            memcpy(&grp_prefix.addr, &group_address, sizeof(vtss_ipv6_t));
            grp_prefix.len = ssm_ver_prefix.len;
            if (ipmc_lib_prefix_maskingNchecking(version, FALSE, &grp_prefix, &grp_prefix) &&
                !memcmp(&ssm_ver_prefix, &grp_prefix, sizeof(ipmc_prefix_t))) {
                grp_in_range = TRUE;
            }
        }

        mali = &sfm_grp_tmr;
        IPMC_TIMER_MALI_GET(entry, mali);
        T_D("incomming port = %d, group = %s", src_port, misc_ipv6_txt(&group_address, buf));
        switch ( record_type ) {
        case IPMC_SFM_MODE_IS_INCLUDE:
            T_D("Record Type: IPMC_SFM_MODE_IS_INCLUDE");

            /*
                A MODE_IS_INCLUDE Record is never sent with an empty source list.
                MUST check no_of_sources FIRST.
            */
            if (no_of_sources != 0) {
                working_grp_ptr = ipmc_lib_protocol_do_asm(p, rxmt, fltr, srct,
                                                           entry,
                                                           src_port,
                                                           &group_address,
                                                           msgType,
                                                           grp_in_range,
                                                           grps_fltr,
                                                           throttling,
                                                           proxy,
                                                           op,
                                                           &do_asm_status);

                if (do_asm_status) {
                    _ipmc_lib_sfm_mode_is_include(p, rxmt, fltr, srct, working_grp_ptr, entry, src_port, &group_address, record_ptr, no_of_sources, proxy, mali);
                }
            }

            break;
        case IPMC_SFM_CHANGE_TO_INCLUDE:
            T_D("Record Type: IPMC_SFM_CHANGE_TO_INCLUDE");

            working_grp_ptr = ipmc_lib_protocol_do_asm(p, rxmt, fltr, srct,
                                                       entry,
                                                       src_port,
                                                       &group_address,
                                                       msgType,
                                                       grp_in_range,
                                                       grps_fltr,
                                                       throttling,
                                                       proxy,
                                                       op,
                                                       &do_asm_status);

            if (do_asm_status) {
                _ipmc_lib_sfm_change_to_include(p, rxmt, fltr, srct, working_grp_ptr, entry, src_port, &group_address, record_ptr, no_of_sources, proxy, mali);
            }

            break;
        case IPMC_SFM_MODE_IS_EXCLUDE:
            if (grp_in_range) {
                break;  /* IGNORE */
            }

            T_D("Record Type: IPMC_SFM_MODE_IS_EXCLUDE");

            working_grp_ptr = ipmc_lib_protocol_do_asm(p, rxmt, fltr, srct,
                                                       entry,
                                                       src_port,
                                                       &group_address,
                                                       msgType,
                                                       grp_in_range,
                                                       grps_fltr,
                                                       throttling,
                                                       proxy,
                                                       op,
                                                       &do_asm_status);

            if (do_asm_status) {
                _ipmc_lib_sfm_mode_is_exclude(p, rxmt, fltr, srct, working_grp_ptr, entry, src_port, &group_address, record_ptr, no_of_sources, proxy, mali);
            }

            break;
        case IPMC_SFM_CHANGE_TO_EXCLUDE:
            if (grp_in_range) {
                break;  /* IGNORE */
            }

            T_D("Record Type: IPMC_SFM_CHANGE_TO_EXCLUDE");

            working_grp_ptr = ipmc_lib_protocol_do_asm(p, rxmt, fltr, srct,
                                                       entry,
                                                       src_port,
                                                       &group_address,
                                                       msgType,
                                                       grp_in_range,
                                                       grps_fltr,
                                                       throttling,
                                                       proxy,
                                                       op,
                                                       &do_asm_status);

            if (do_asm_status) {
                _ipmc_lib_sfm_change_to_exclude(p, rxmt, fltr, srct, working_grp_ptr, entry, src_port, &group_address, record_ptr, no_of_sources, proxy, mali);
            }

            break;
        case IPMC_SFM_ALLOW_NEW_SOURCES:
            T_D("Record Type: IPMC_SFM_ALLOW_NEW_SOURCES");

            /*
                If the computed source list for either an ALLOW or a BLOCK record is empty,
                that record is omitted from the State Change Report.
                MUST check no_of_sources FIRST.
            */
            if (no_of_sources != 0) {
                working_grp_ptr = ipmc_lib_protocol_do_asm(p, rxmt, fltr, srct,
                                                           entry,
                                                           src_port,
                                                           &group_address,
                                                           msgType,
                                                           grp_in_range,
                                                           grps_fltr,
                                                           throttling,
                                                           proxy,
                                                           op,
                                                           &do_asm_status);

                if (do_asm_status) {
                    _ipmc_lib_sfm_allow_new_source(p, rxmt, fltr, srct, working_grp_ptr, entry, src_port, &group_address, record_ptr, no_of_sources, proxy, mali);
                }
            }

            break;
        case IPMC_SFM_BLOCK_OLD_SOURCES:
            T_D("Record Type: IPMC_SFM_BLOCK_OLD_SOURCES");

            /*
                If the computed source list for either an ALLOW or a BLOCK record is empty,
                that record is omitted from the State Change Report.
                MUST check no_of_sources FIRST.
            */
            if (no_of_sources != 0) {
                working_grp_ptr = ipmc_lib_protocol_do_asm(p, rxmt, fltr, srct,
                                                           entry,
                                                           src_port,
                                                           &group_address,
                                                           msgType,
                                                           grp_in_range,
                                                           grps_fltr,
                                                           throttling,
                                                           proxy,
                                                           op,
                                                           &do_asm_status);

                if (do_asm_status) {
                    _ipmc_lib_sfm_block_old_source(p, rxmt, fltr, srct, working_grp_ptr, entry, src_port, &group_address, record_ptr, no_of_sources, proxy, mali);
                }
            }

            break;
        case IPMC_SFM_RECORD_TYPE_ERR:
        default:
            if (record_type == IPMC_SFM_RECORD_TYPE_ERR) {
                T_W("\n\rIPMC_ERROR_PKT_CONTENT in ipmc_lib_protocol_do_sfm_report->Skip");
            }

            break;
        }

        number_of_record--;
        if (number_of_record == 0) {
            break;
        }

        /* Prepare next group_record */
        if ((version == IPMC_IP_VERSION_IGMP) && igmp_group_record) {
            record_ptr = (u8 *)(&igmp_group_record->group_address);
            current_pkt_len += sizeof(ipmcv4addr);
            record_ptr += sizeof(ipmcv4addr);

            /* skip past all source addresses to get to the next group record */
            current_pkt_len += (no_of_sources * sizeof(ipmcv4addr));
            record_ptr += (no_of_sources * sizeof(ipmcv4addr));
        } else if ((version == IPMC_IP_VERSION_MLD) && mld_group_record) {
            record_ptr = (u8 *)(&mld_group_record->group_address);
            current_pkt_len += sizeof(vtss_ipv6_t);
            record_ptr += sizeof(vtss_ipv6_t);

            /* skip past all source addresses to get to the next group record */
            current_pkt_len += (no_of_sources * sizeof(vtss_ipv6_t));
            record_ptr += (no_of_sources * sizeof(vtss_ipv6_t));
        } else {
            rc = VTSS_RC_ERROR;
            break;
        }
        /* ignore aux data */
        current_pkt_len += (aux_len * sizeof(u32));
        record_ptr += (aux_len * sizeof(u32));

        if (version == IPMC_IP_VERSION_IGMP) {
            if ((current_pkt_len + sizeof(u32) + sizeof(ipmcv4addr)) > ipmc_pkt_len) {
                rc = VTSS_RC_ERROR;
                break;
            } else {
                igmp_group_record = (igmp_group_record_t *)record_ptr;
            }
        } else {
            if ((current_pkt_len + sizeof(u32) + sizeof(vtss_ipv6_t)) > ipmc_pkt_len) {
                rc = VTSS_RC_ERROR;
                break;
            } else {
                mld_group_record = (mld_group_record_t *)record_ptr;
            }
        }
    }

    return rc;
}

static void _ipmc_lib_protocol_get_intf_active_adrs(ipmc_intf_entry_t *intf)
{
    if (!intf) {
        return;
    }

    if (intf->ipmc_version == IPMC_IP_VERSION_IGMP) {
        vtss_ipv4_t ip4addr = 0;

        /* get src address */
        if (ipmc_lib_get_ipintf_igmp_adrs(intf, &ip4addr)) {
            ip4addr = htonl(ip4addr);
            IPMC_LIB_ADRS_4TO6_SET(ip4addr, intf->param.active_querier);
        } else {
            IPMC_LIB_ADRS_SET(&intf->param.active_querier, 0x0);
        }
    } else {
        if (!ipmc_lib_get_eui64_linklocal_addr(&intf->param.active_querier)) {
            IPMC_LIB_ADRS_SET(&intf->param.active_querier, 0x0);
        }
    }
}

vtss_rc ipmc_lib_protocol_intf_tmr(BOOL proxy_active, ipmc_intf_entry_t *ipmc_intf)
{
    ipmc_querier_sm_t       *querier;
    ipmc_compatibility_t    *hst_compatibility;
    ipmc_compatibility_t    *rtr_compatibility;
    ipmc_compat_mode_t      compatibility;
    vtss_ipv6_t             dst_ip6;

    if (!ipmc_intf || !ipmc_intf->param.vid) {
        return VTSS_RC_ERROR;
    }

    if (!ipmc_intf->op_state) {
        return VTSS_OK;
    }

    querier = &ipmc_intf->param.querier;
    hst_compatibility = &ipmc_intf->param.hst_compatibility;
    rtr_compatibility = &ipmc_intf->param.rtr_compatibility;

    /* For Querier */
    if (querier->querier_enabled) {
        switch ( querier->state ) {
        case IPMC_QUERIER_INIT:
            /*
                In INIT state, IPMC should transit to ACTIVE(Querier) state
                immediately.  However, during the period of time that Querier
                should send Queries in Startup-Query-Count times, the timeout
                value MUST follow Startup-Query-Interval instaed of General
                Query-Interval.  So, we still make IPMC internally staying in
                INIT state.  But Querier Status MUST be as ACTIVE(Querier)
                for reporting.
            */
            if (querier->timeout == 0) {
                T_D("IPMCVer-%d Querier Init for VLAN %d", ipmc_intf->ipmc_version, ipmc_intf->param.vid);

                if (!proxy_active) {
                    ipmc_lib_get_all_zero_ipv6_addr(&dst_ip6);
                    if (ipmc_lib_packet_tx_gq(ipmc_intf, &dst_ip6, FALSE, FALSE) != VTSS_OK) {
                        T_D("Failure in ipmc_lib_packet_tx_gq for VLAN %d", ipmc_intf->param.vid);
                    }
                } else {
                    T_D("Proxy Active->Skip");
                }

                querier->timeout = IPMC_TIMER_SQI(ipmc_intf);
                querier->StartUpCnt--;
            } else {
                querier->timeout--;
            }

            if (querier->StartUpCnt == 0) {
                querier->QuerierUpTime = 0;
                querier->state = IPMC_QUERIER_ACTIVE;
                _ipmc_lib_protocol_get_intf_active_adrs(ipmc_intf);
            }

            break;
        case IPMC_QUERIER_ACTIVE:
            querier->QuerierUpTime++;
            _ipmc_lib_protocol_get_intf_active_adrs(ipmc_intf);

            if (querier->timeout) {
                querier->timeout--;

                if (querier->timeout) {
                    break;
                }

                T_D("IPMCVer-%d Querier timeout for VLAN %d", ipmc_intf->ipmc_version, ipmc_intf->param.vid);
                if (!proxy_active) {
                    ipmc_lib_get_all_zero_ipv6_addr(&dst_ip6);
                    if (ipmc_lib_packet_tx_gq(ipmc_intf, &dst_ip6, FALSE, FALSE) != VTSS_OK) {
                        T_D("Failure in ipmc_lib_packet_tx_gq for VLAN %d", ipmc_intf->param.vid);
                    }
                } else {
                    T_D("Proxy Active->Skip");
                }
                querier->timeout = IPMC_TIMER_QI(ipmc_intf);
                querier->state = IPMC_QUERIER_ACTIVE;
            } else {
                T_D("IPMCVer-%d Querier timeout for VLAN %d", ipmc_intf->ipmc_version, ipmc_intf->param.vid);
                if (!proxy_active) {
                    ipmc_lib_get_all_zero_ipv6_addr(&dst_ip6);
                    if (ipmc_lib_packet_tx_gq(ipmc_intf, &dst_ip6, FALSE, FALSE) != VTSS_OK) {
                        T_D("Failure in ipmc_lib_packet_tx_gq for VLAN %d", ipmc_intf->param.vid);
                    }
                } else {
                    T_D("Proxy Active->Skip");
                }
                querier->timeout = IPMC_TIMER_QI(ipmc_intf);
                querier->state = IPMC_QUERIER_ACTIVE;
            }

            break;
        case IPMC_QUERIER_IDLE:
        default:
            querier->QuerierUpTime = 0;
            if (querier->OtherQuerierTimeOut) {
                querier->OtherQuerierTimeOut--;
            } else {
                querier->timeout = IPMC_TIMER_QI(ipmc_intf);
                querier->state = IPMC_QUERIER_ACTIVE;
                _ipmc_lib_protocol_get_intf_active_adrs(ipmc_intf);

                if (!proxy_active) {
                    ipmc_lib_get_all_zero_ipv6_addr(&dst_ip6);
                    if (ipmc_lib_packet_tx_gq(ipmc_intf, &dst_ip6, FALSE, FALSE) != VTSS_OK) {
                        T_D("Failure in ipmc_lib_packet_tx_gq for VLAN %d", ipmc_intf->param.vid);
                    }
                } else {
                    T_D("Proxy Active->Skip");
                }
            }

            break;
        }
    } else {
        querier->state = IPMC_QUERIER_IDLE;

        if (querier->OtherQuerierTimeOut == 0) {
            querier->OtherQuerierTimeOut = IPMC_TIMER_OQPT(ipmc_intf);
        }
    } /* if (querier->querier_enabled) */

    /* Compatibility Checking for Interface */
    compatibility = IPMC_COMPATIBILITY(ipmc_intf);
    if (compatibility != VTSS_IPMC_COMPAT_MODE_AUTO) {
        rtr_compatibility->old_present_timer = 0;
        rtr_compatibility->gen_present_timer = 0;
        rtr_compatibility->sfm_present_timer = 0;
        rtr_compatibility->mode = IPMC_COMPATIBILITY(ipmc_intf);

        hst_compatibility->old_present_timer = 0;
        hst_compatibility->gen_present_timer = 0;
        hst_compatibility->sfm_present_timer = 0;
        hst_compatibility->mode = IPMC_COMPATIBILITY(ipmc_intf);

        return VTSS_OK;
    }

    if (hst_compatibility->old_present_timer != 0) {
        hst_compatibility->old_present_timer--;

        if ((hst_compatibility->old_present_timer == 0) &&
            (ipmc_intf->ipmc_version == IPMC_IP_VERSION_IGMP)) {
            if (hst_compatibility->gen_present_timer) {
                hst_compatibility->mode = VTSS_IPMC_COMPAT_MODE_GEN;
            } else {
                hst_compatibility->mode = VTSS_IPMC_COMPAT_MODE_SFM;
            }
        }
    }
    if (hst_compatibility->gen_present_timer != 0) {
        hst_compatibility->gen_present_timer--;

        if (hst_compatibility->gen_present_timer == 0) {
            hst_compatibility->mode = VTSS_IPMC_COMPAT_MODE_SFM;
        }
    }
    if (hst_compatibility->sfm_present_timer != 0) {
        hst_compatibility->sfm_present_timer--;
    }

    if (rtr_compatibility->old_present_timer != 0) {
        rtr_compatibility->old_present_timer--;

        if ((rtr_compatibility->old_present_timer == 0) &&
            (ipmc_intf->ipmc_version == IPMC_IP_VERSION_IGMP)) {
            if (rtr_compatibility->gen_present_timer) {
                rtr_compatibility->mode = VTSS_IPMC_COMPAT_MODE_GEN;
            } else {
                rtr_compatibility->mode = VTSS_IPMC_COMPAT_MODE_SFM;
            }
        }
    }
    if (rtr_compatibility->gen_present_timer != 0) {
        rtr_compatibility->gen_present_timer--;

        if (rtr_compatibility->gen_present_timer == 0) {
            rtr_compatibility->mode = VTSS_IPMC_COMPAT_MODE_SFM;
        }
    }
    if (rtr_compatibility->sfm_present_timer != 0) {
        rtr_compatibility->sfm_present_timer--;
    }

    return VTSS_OK;
}

vtss_rc ipmc_lib_protocol_intf_rxmt(ipmc_db_ctrl_hdr_t *p,
                                    ipmc_db_ctrl_hdr_t *rxmt,
                                    ipmc_db_ctrl_hdr_t *fltr)
{
    ipmc_group_entry_t  *grp;
    ipmc_group_info_t   *grp_info, *tmr_ptr;
    ipmc_group_db_t     *grp_db;
    ipmc_intf_entry_t   *ipmc_intf;
    ipmc_time_t         current_time;
    BOOL                rxmt_tx_need;
    u32                 j, v, local_port_cnt;

    if (!p || !rxmt) {
        return VTSS_RC_ERROR;
    }

    (void) ipmc_lib_time_curr_get(&current_time);
    local_port_cnt = ipmc_lib_get_system_local_port_cnt();
    /* Process RXMT & ROUTER-PORT-TIMEOUT-EVENT */
    tmr_ptr = NULL;
    while ((tmr_ptr = ipmc_lib_rxmt_tmrlist_walk(rxmt, tmr_ptr, &current_time)) != NULL) {
        grp = tmr_ptr->grp;
        grp_info = grp->info;

        if ((ipmc_intf = grp_info->interface) == NULL) {
            continue;
        }
        if (ipmc_intf->param.mvr) {
            if (!grp_info->interface->param.mvr) {
                continue;
            }
        } else {
            if (grp_info->interface->param.mvr) {
                continue;
            }
        }

        grp_db = &grp_info->db;
        for (j = 0; j < local_port_cnt; j++) {
            if (IPMC_TIMER_ZERO(&grp_info->rxmt_timer[j]) ||
                IPMC_TIMER_GREATER(&grp_info->rxmt_timer[j], &current_time)) {
                continue;
            }

            rxmt_tx_need = FALSE;
            if (ipmc_lib_get_changed_rpstatus(grp->ipmc_version, j) &&
                !IPMC_LIB_GRP_PORT_DO_SFM(grp_db, j) &&
                !ipmc_lib_get_port_rpstatus(grp->ipmc_version, j) &&
                VTSS_PORT_BF_GET(grp_db->port_mask, j)) {
                VTSS_PORT_BF_SET(grp_db->port_mask, j, FALSE);

                rxmt_tx_need = TRUE;
            }

            switch ( grp_info->state ) {
            case IPMC_OP_HAS_LISTENER:

                break;
            case IPMC_OP_CHK_LISTENER:
                /* Any group in IPMC_OP_CHK_LISTENER continues with the same state until expired. */
                if (grp_info->rxmt_count[j]) {
                    (void) ipmc_lib_packet_tx_sq(fltr, IPMC_SND_GO, grp, ipmc_intf, j, FALSE);
                    rxmt_tx_need = FALSE;

                    v = local_port_cnt;
                    IPMC_RXMT_TIMER_RESET(v, j, rxmt, ipmc_intf, grp_info);
                }

                break;
            case IPMC_OP_NO_LISTENER:
            default:

                break;
            }

            if (rxmt_tx_need) {
                (void) ipmc_lib_packet_tx_sq(fltr, IPMC_SND_GO, grp, ipmc_intf, j, FALSE);

                v = local_port_cnt;
                IPMC_RXMT_TIMER_RESET(v, j, rxmt, ipmc_intf, grp_info);
            }
        } /* while (port_iter_getnext(&pit)) */

        v = local_port_cnt;
        IPMC_RTIMER_RELINK(v, rxmt, tmr_ptr, &rxmt_tx_need);
    } /* Process RXMT & ROUTER-PORT-TIMEOUT-EVENT */

    return VTSS_OK;
}

static void _ipmc_lib_protocol_srct_tmr(BOOL from_mvr,
                                        ipmc_db_ctrl_hdr_t *p,
                                        ipmc_db_ctrl_hdr_t *rxmt,
                                        ipmc_db_ctrl_hdr_t *fltr,
                                        ipmc_db_ctrl_hdr_t *srct,
                                        ipmc_port_throttling_t *g_throttling,
                                        BOOL *g_proxy,
                                        BOOL *l_proxy,
                                        u8 local_port_cnt,
                                        ipmc_time_t *current)
{
    ipmc_group_entry_t      *grp;
    ipmc_group_info_t       *grp_info;
    ipmc_group_db_t         *grp_db;
    ipmc_intf_entry_t       *ipmc_intf;
    ipmc_sfm_srclist_t      *tmr_ptr, *src_list_entry, *free_ptr;
    ipmc_sfm_srclist_t      tmp_src_list_entry, *tmp_src_list_ptr;
    BOOL                    rst, found, proxy, mark_for_op_delete, update_sfm_fwd, snd_masq;
    ipmc_port_throttling_t  *throttling;
    ipmc_send_act_t         snd_act;
    u32                     j, v;
    ipmc_db_ctrl_hdr_t      *lower_timer_srclist;
    ipmc_db_ctrl_hdr_t      *sf_do_forward_srclist;
    ipmc_db_ctrl_hdr_t      *sf_do_not_forward_srclist;
    u8                      alcid;

    tmr_ptr = NULL;
    while ((tmr_ptr = ipmc_lib_srct_tmrlist_walk(srct, tmr_ptr, current)) != NULL) {
        grp = tmr_ptr->grp;
        grp_info = grp->info;
        if ((ipmc_intf = grp_info->interface) == NULL) {
            continue;
        }
        if (from_mvr) {
            if (!ipmc_intf->param.mvr) {
                continue;
            }
        } else {
            if (ipmc_intf->param.mvr) {
                continue;
            }
        }

        update_sfm_fwd = FALSE;
        mark_for_op_delete = FALSE;
        if (g_proxy && l_proxy) {
            proxy = g_proxy[ipmc_intf->ipmc_version] | l_proxy[ipmc_intf->ipmc_version];
        } else {
            proxy = FALSE;
        }
        if (g_throttling) {
            throttling = &g_throttling[ipmc_intf->ipmc_version];
        } else {
            throttling = NULL;
        }

        grp_db = &grp_info->db;
        sf_do_forward_srclist = grp_db->ipmc_sf_do_forward_srclist;
        sf_do_not_forward_srclist = grp_db->ipmc_sf_do_not_forward_srclist;
        for (j = 0; j < local_port_cnt; j++) {
            if (!IPMC_LIB_GRP_PORT_DO_SFM(grp_db, j)) {
                continue;
            } else {
                VTSS_PORT_BF_SET(grp_db->ipmc_sf_port_status, j, VTSS_IPMC_SF_STATUS_ENABLED);
                lower_timer_srclist = &ipmc_lower_timer_srclist[IPMC_INTF_IS_MVR_VAL(ipmc_intf)][j];
            }

            /* Prepare for MASSQ */
            snd_masq = FALSE;

            if (IPMC_LIB_GRP_PORT_SFM_IN(grp_db, j)) { /* INCLUDE MODE */
                /* Source Timer */
                /* Include List */
                alcid = 0;
                src_list_entry = free_ptr = NULL;
                IPMC_SRCLIST_WALK(sf_do_forward_srclist, src_list_entry) {
                    if (free_ptr) {
                        if (IPMC_LIB_DB_DEL(sf_do_forward_srclist, free_ptr)) {
                            IPMC_MEM_SL_MGIVE(free_ptr, &rst, 208);
                        }
                        free_ptr = NULL;
                    }

                    if (!VTSS_PORT_BF_GET(src_list_entry->port_mask, j) ||
                        IPMC_TIMER_GREATER(&src_list_entry->tmr.srct_timer.t[j], current)) {
                        continue;
                    }

                    if (!update_sfm_fwd) {
                        ipmc_lib_proc_grp_sfm_tmp4tick(IPMC_INTF_IS_MVR_VAL(ipmc_intf), FALSE, TRUE, grp);

                        /* Prepare for updating SFM FWD */
                        if (ipmc_lib_srclist_struct_copy(
                                grp,
                                &ipmc_sf_permit_srclist[IPMC_INTF_IS_MVR_VAL(ipmc_intf)][j],
                                sf_do_forward_srclist,
                                j) != TRUE) {
                            T_D("vtss_ipmc_data_struct_copy() failed");
                        }
                        if (ipmc_lib_srclist_struct_copy(
                                grp,
                                &ipmc_sf_deny_srclist[IPMC_INTF_IS_MVR_VAL(ipmc_intf)][j],
                                sf_do_not_forward_srclist,
                                j) != TRUE) {
                            T_D("vtss_ipmc_data_struct_copy() failed");
                        }
                    }

                    update_sfm_fwd = TRUE;

                    if (!snd_masq) {
                        if (!ipmc_lib_srclist_clear(&tmp1_srclist[IPMC_INTF_IS_MVR_VAL(ipmc_intf)], 50)) {
                            T_D("ipmc_lib_srclist_clear failed");
                        }

                        snd_masq = TRUE;
                    }
                    /* For sending Q(MA, A) */
                    IPMC_LIB_SRCT_ADD_EPM(++alcid, &tmp1_srclist[IPMC_INTF_IS_MVR_VAL(ipmc_intf)], grp, src_list_entry);

                    /* Remove source record */
                    VTSS_PORT_BF_SET(src_list_entry->port_mask, j, FALSE);
                    memcpy(&tmp_src_list_entry, src_list_entry, sizeof(ipmc_sfm_srclist_t));
                    if (IPMC_LIB_BFS_PROT_EMPTY(src_list_entry->port_mask)) {
                        IPMC_TIMER_UNLINK(srct, src_list_entry, &rst);
                        free_ptr = src_list_entry;
                    }

                    if ((tmp_src_list_ptr = ipmc_lib_srclist_adr_get(sf_do_not_forward_srclist, &tmp_src_list_entry)) != NULL) {
                        if (VTSS_PORT_BF_GET(tmp_src_list_ptr->port_mask, j)) {
                            VTSS_PORT_BF_SET(tmp_src_list_ptr->port_mask, j, FALSE);
                            if (IPMC_LIB_BFS_PROT_EMPTY(tmp_src_list_ptr->port_mask)) {
                                if (tmp_src_list_ptr != src_list_entry) {
                                    IPMC_TIMER_UNLINK(srct, tmp_src_list_ptr, &rst);
                                    if (!ipmc_lib_srclist_del(sf_do_not_forward_srclist, tmp_src_list_ptr, 209)) {
                                        T_D("ipmc_lib_srclist_del failed!!!");
                                    }
                                } else {
                                    (void) IPMC_LIB_DB_DEL(sf_do_not_forward_srclist, tmp_src_list_ptr);
                                }
                            }
                        }
                    }
                } /* Include List */
                if (free_ptr && IPMC_LIB_DB_DEL(sf_do_forward_srclist, free_ptr)) {
                    IPMC_MEM_SL_MGIVE(free_ptr, &rst, 207);
                }
                /* Source Timer */
            } else { /* EXCLUDE MODE */
                found = FALSE;

                /* Source Timer */
                /* Exclude List */
                src_list_entry = NULL;
                IPMC_SRCLIST_WALK(sf_do_not_forward_srclist, src_list_entry) {
                    if (!VTSS_PORT_BF_GET(src_list_entry->port_mask, j) ||
                        IPMC_TIMER_GREATER(&src_list_entry->tmr.srct_timer.t[j], current)) {
                        continue;
                    }

                    IPMC_TIMER_RESET(&src_list_entry->tmr.srct_timer.t[j]);
                } /* Exclude List */

                /* Request List */
                alcid = 10;
                src_list_entry = free_ptr = NULL;
                IPMC_SRCLIST_WALK(sf_do_forward_srclist, src_list_entry) {
                    if (free_ptr) {
                        if (IPMC_LIB_DB_DEL(sf_do_forward_srclist, free_ptr)) {
                            IPMC_MEM_SL_MGIVE(free_ptr, &rst, 206);
                        }
                        free_ptr = NULL;
                    }

                    if (!VTSS_PORT_BF_GET(src_list_entry->port_mask, j) ||
                        IPMC_TIMER_GREATER(&src_list_entry->tmr.srct_timer.t[j], current)) {
                        continue;
                    }

                    IPMC_TIMER_RESET(&src_list_entry->tmr.srct_timer.t[j]);

                    if (!update_sfm_fwd) {
                        ipmc_lib_proc_grp_sfm_tmp4tick(IPMC_INTF_IS_MVR_VAL(ipmc_intf), FALSE, TRUE, grp);

                        /* Prepare for updating SFM FWD */
                        if (ipmc_lib_srclist_struct_copy(
                                grp,
                                &ipmc_sf_permit_srclist[IPMC_INTF_IS_MVR_VAL(ipmc_intf)][j],
                                sf_do_forward_srclist,
                                j) != TRUE) {
                            T_D("vtss_ipmc_data_struct_copy() failed");
                        }
                        if (ipmc_lib_srclist_struct_copy(
                                grp,
                                &ipmc_sf_deny_srclist[IPMC_INTF_IS_MVR_VAL(ipmc_intf)][j],
                                sf_do_not_forward_srclist,
                                j) != TRUE) {
                            T_D("vtss_ipmc_data_struct_copy() failed");
                        }
                    }

                    update_sfm_fwd = TRUE;

                    if (!snd_masq) {
                        if (!ipmc_lib_srclist_clear(&tmp1_srclist[IPMC_INTF_IS_MVR_VAL(ipmc_intf)], 40)) {
                            T_D("ipmc_lib_srclist_clear failed");
                        }

                        snd_masq = TRUE;
                    }
                    /* For sending Q(MA, A) */
                    IPMC_LIB_SRCT_ADD_EPM(++alcid, &tmp1_srclist[IPMC_INTF_IS_MVR_VAL(ipmc_intf)], grp, src_list_entry);

                    /* Add to Exclude-List */
                    memcpy(&tmp_src_list_entry, src_list_entry, sizeof(ipmc_sfm_srclist_t));
                    if ((tmp_src_list_ptr = ipmc_lib_srclist_adr_get(sf_do_not_forward_srclist, &tmp_src_list_entry)) != NULL) {
                        VTSS_PORT_BF_SET(tmp_src_list_ptr->port_mask, j, TRUE);
                    } else {
                        tmp_src_list_ptr = &tmp_src_list_entry;
                        VTSS_PORT_BF_CLR(tmp_src_list_ptr->port_mask);
                        VTSS_PORT_BF_SET(tmp_src_list_ptr->port_mask, j, TRUE);
                        IPMC_TIMER_RESET(&tmp_src_list_ptr->min_tmr);

                        IPMC_LIB_SRCT_ADD_EPM(++alcid, sf_do_not_forward_srclist, grp, tmp_src_list_ptr);
                    }
                    /* Remove from Request-List */
                    VTSS_PORT_BF_SET(src_list_entry->port_mask, j, FALSE);
                    if (IPMC_LIB_BFS_PROT_EMPTY(src_list_entry->port_mask)) {
                        found = TRUE;
                        IPMC_TIMER_UNLINK(srct, src_list_entry, &rst);
                        free_ptr = src_list_entry;
                    }
                } /* Request List */
                if (free_ptr && IPMC_LIB_DB_DEL(sf_do_forward_srclist, free_ptr)) {
                    IPMC_MEM_SL_MGIVE(free_ptr, &rst, 205);
                }

                if (!found) {
                    v = local_port_cnt;
                    IPMC_STIMER_RELINK(v, srct, tmr_ptr, &rst);
                }
                /* Source Timer */
            } /* VTSS_IPMC_SF_MODE_INCLUDE | VTSS_IPMC_SF_MODE_EXCLUDE */

            /* Send Q(MA, A) */
            if (snd_masq && IPMC_LIB_DB_GET_COUNT(&tmp1_srclist[IPMC_INTF_IS_MVR_VAL(ipmc_intf)]) &&
                (grp_info->state != IPMC_OP_CHK_LISTENER)) {
                snd_act = ipmc_lib_get_sq_ssq_action(proxy, ipmc_intf, j);
                (void) ipmc_lib_packet_tx_ssq(fltr, snd_act, grp, ipmc_intf, j, &tmp1_srclist[IPMC_INTF_IS_MVR_VAL(ipmc_intf)], FALSE);

                if (ipmc_lib_srclist_struct_copy(
                        grp,
                        lower_timer_srclist,
                        &tmp1_srclist[IPMC_INTF_IS_MVR_VAL(ipmc_intf)],
                        j) != TRUE) {
                    T_D("vtss_ipmc_data_struct_copy() failed");
                    if (!ipmc_lib_srclist_clear(lower_timer_srclist, 30)) {
                        T_D("ipmc_lib_srclist_clear failed");
                    }
                }

                /* Filter Timer needs to be refresh */
                if (snd_act != IPMC_SND_HOLD) { /* Querier */
                    grp_info->state = IPMC_OP_CHK_LISTENER;
                    /* start rxmt timer */
                    grp_info->rxmt_count[j] = IPMC_TIMER_LLQC(ipmc_intf);
                    IPMC_TIMER_LLQI_SET(ipmc_intf, &grp_info->rxmt_timer[j], rxmt, grp_info);
                }
            }
        } /* port_iter_getnext(&pit) */
        /* Process SFM Timers for FWD */

        if (mark_for_op_delete == FALSE) {
            /* Update Database only when update_sfm_fwd */
            if (update_sfm_fwd && !ipmc_lib_group_sync(p, NULL, grp, FALSE, PROC4TICK)) {
                T_D("Update group failed in _ipmc_lib_protocol_srct_tmr");
            }

            for (j = 0; j < local_port_cnt; j++) {
                lower_timer_srclist = &ipmc_lower_timer_srclist[IPMC_INTF_IS_MVR_VAL(ipmc_intf)][j];
                if (IPMC_LIB_GRP_PORT_DO_SFM(grp_db, j) &&
                    IPMC_LIB_DB_GET_COUNT(lower_timer_srclist)) {
                    /* ipmc_lib_protocol_lower_source_timer */
                    src_list_entry = NULL;
                    IPMC_SRCLIST_WALK(lower_timer_srclist, src_list_entry) {
                        if ((tmp_src_list_ptr = ipmc_lib_srclist_adr_get(sf_do_not_forward_srclist, src_list_entry)) != NULL) {
                            if (VTSS_PORT_BF_GET(tmp_src_list_ptr->port_mask, j)) {
                                IPMC_TIMER_LLQT_GSET(ipmc_intf, &tmp_src_list_ptr->tmr.srct_timer.t[j], srct, tmp_src_list_ptr);
                            }
                        }
                        if ((tmp_src_list_ptr = ipmc_lib_srclist_adr_get(sf_do_forward_srclist, src_list_entry)) != NULL) {
                            if (VTSS_PORT_BF_GET(tmp_src_list_ptr->port_mask, j)) {
                                IPMC_TIMER_LLQT_GSET(ipmc_intf, &tmp_src_list_ptr->tmr.srct_timer.t[j], srct, tmp_src_list_ptr);
                            }
                        }
                    }
                }
            }
        }

        /* Finalize SFM Checking */
        mark_for_op_delete = FALSE;
        update_sfm_fwd = FALSE;
        for (j = 0; j < local_port_cnt; j++) {
            if (!IPMC_LIB_GRP_PORT_DO_SFM(grp_db, j)) {
                continue;
            }

            mark_for_op_delete = TRUE;  /* Prepare to check DELETE */
            /* INCLUDE MODE */
            if (IPMC_LIB_GRP_PORT_SFM_IN(grp_db, j)) {
                BOOL    empty_src_list = TRUE;

                src_list_entry = NULL;
                IPMC_SRCLIST_WALK(grp_db->ipmc_sf_do_forward_srclist, src_list_entry) {
                    if (VTSS_PORT_BF_GET(src_list_entry->port_mask, j)) {
                        empty_src_list = FALSE;
                        break;
                    }
                }

                if (empty_src_list) {
                    snd_act = ipmc_lib_get_sq_ssq_action(proxy, ipmc_intf, j);
                    (void) ipmc_lib_packet_tx_sq(fltr, snd_act, grp, ipmc_intf, j, FALSE);
                    if (snd_act != IPMC_SND_HOLD) { /* Querier */
                        v = local_port_cnt;
                        IPMC_RXMT_TIMER_RESET(v, j, rxmt, ipmc_intf, grp_info);
                    }
                    /* Filter Timer needs to be refresh */

                    VTSS_PORT_BF_SET(grp_db->ipmc_sf_port_status, j, VTSS_IPMC_SF_STATUS_DISABLED);
                    if (throttling && throttling->max_no[j]) {
                        --throttling->max_no[j];
                    }
                    VTSS_PORT_BF_SET(grp_db->port_mask, j, FALSE);
                    update_sfm_fwd = TRUE;
                }
            }
        }

        /* Prepare to check DELETE */
        if (mark_for_op_delete) {
            for (j = 0; j < local_port_cnt; j++) {
                if (!IPMC_LIB_GRP_PORT_DO_SFM(grp_db, j)) {
                    continue;
                }

                mark_for_op_delete = FALSE;
                break;
            }

            /* DELETE only-when ALL-PORTS are VTSS_IPMC_SF_STATUS_DISABLED */
            if (mark_for_op_delete) {
                (void) ipmc_lib_group_delete(ipmc_intf, p, rxmt, fltr, srct, grp, proxy, TRUE);
            } else {
                if (update_sfm_fwd && !ipmc_lib_group_sync(p, NULL, grp, FALSE, PROC4TICK)) {
                    T_D("Update group failed in vtss_ipmc_tick_intf_grp_sfm");
                }
            }
        } /* Prepare to check DELETE */
        /* Finalize SFM Checking */
    }
}

static void _ipmc_lib_protocol_fltr_tmr(BOOL from_mvr,
                                        ipmc_db_ctrl_hdr_t *p,
                                        ipmc_db_ctrl_hdr_t *rxmt,
                                        ipmc_db_ctrl_hdr_t *fltr,
                                        ipmc_db_ctrl_hdr_t *srct,
                                        ipmc_port_throttling_t *g_throttling,
                                        BOOL *g_proxy,
                                        BOOL *l_proxy,
                                        u8 local_port_cnt,
                                        ipmc_time_t *current)
{
    ipmc_group_entry_t      *grp;
    ipmc_group_info_t       *grp_info;
    ipmc_group_db_t         *grp_db, *tmr_ptr;
    ipmc_intf_entry_t       *ipmc_intf;
    ipmc_sfm_srclist_t      *src_list_entry, *tmp_src_list_ptr, *free_ptr;
    BOOL                    rst, proxy, mark_for_op_delete, update_sfm_fwd, snd_masq;
    ipmc_port_throttling_t  *throttling;
    ipmc_send_act_t         snd_act;
    u32                     j, v;
    ipmc_db_ctrl_hdr_t      *lower_timer_srclist;
    ipmc_db_ctrl_hdr_t      *sf_do_forward_srclist;
    ipmc_db_ctrl_hdr_t      *sf_do_not_forward_srclist;
    u8                      alcid;

    /* Process SFM Timers for FWD */
    tmr_ptr = NULL;
    while ((tmr_ptr = ipmc_lib_fltr_tmrlist_walk(fltr, tmr_ptr, current)) != NULL) {
        grp = tmr_ptr->grp;
        grp_info = grp->info;
        if (!grp_info || ((ipmc_intf = grp_info->interface) == NULL)) {
            continue;
        }
        if (from_mvr) {
            if (!ipmc_intf->param.mvr) {
                continue;
            }
        } else {
            if (ipmc_intf->param.mvr) {
                continue;
            }
        }

        update_sfm_fwd = FALSE;
        mark_for_op_delete = FALSE;
        if (g_proxy && l_proxy) {
            proxy = g_proxy[ipmc_intf->ipmc_version] | l_proxy[ipmc_intf->ipmc_version];
        } else {
            proxy = FALSE;
        }
        if (g_throttling) {
            throttling = &g_throttling[ipmc_intf->ipmc_version];
        } else {
            throttling = NULL;
        }

        grp_db = &grp_info->db;
        sf_do_forward_srclist = grp_db->ipmc_sf_do_forward_srclist;
        sf_do_not_forward_srclist = grp_db->ipmc_sf_do_not_forward_srclist;
        for (j = 0; j < local_port_cnt; j++) {
            if (!IPMC_LIB_GRP_PORT_DO_SFM(grp_db, j)) {
                continue;
            } else {
                if (IPMC_TIMER_GREATER(&tmr_ptr->tmr.fltr_timer.t[j], current)) {
                    continue;
                }

                VTSS_PORT_BF_SET(grp_db->ipmc_sf_port_status, j, VTSS_IPMC_SF_STATUS_ENABLED);
            }
            IPMC_TIMER_RESET(&tmr_ptr->tmr.fltr_timer.t[j]);

            /* Prepare for MASSQ */
            snd_masq = FALSE;

            /* Filter Timer -> For EXCLUDE only */
            if (IPMC_LIB_GRP_PORT_SFM_EX(grp_db, j)) {
                /* Filter Timer -> For EXCLUDE only */
                /* Section 7.2.3 & 7.5 in RFC3810 */
                VTSS_PORT_BF_SET(grp_db->ipmc_sf_port_status, j, VTSS_IPMC_SF_STATUS_DISABLED);
                src_list_entry = NULL;
                IPMC_SRCLIST_WALK(sf_do_forward_srclist, src_list_entry) {
                    if (!VTSS_PORT_BF_GET(src_list_entry->port_mask, j) ||
                        IPMC_TIMER_GREATER(&src_list_entry->tmr.srct_timer.t[j], current)) {
                        continue;
                    }

                    /* Logically Transit; Actually Enable */
                    VTSS_PORT_BF_SET(grp_db->ipmc_sf_port_status, j, VTSS_IPMC_SF_STATUS_TRANSIT);

                    IPMC_TIMER_RESET(&src_list_entry->tmr.srct_timer.t[j]);
                    if (IPMC_LIB_DB_GET_COUNT(sf_do_not_forward_srclist) != 0) {
                        ipmc_sfm_srclist_t  *tmp_srclist_op;

                        alcid = 30;
                        tmp_srclist_op = free_ptr = NULL;
                        IPMC_SRCLIST_WALK(sf_do_not_forward_srclist, tmp_srclist_op) {
                            if (free_ptr) {
                                if (IPMC_LIB_DB_DEL(sf_do_not_forward_srclist, free_ptr)) {
                                    IPMC_MEM_SL_MGIVE(free_ptr, &rst, 204);
                                }
                                free_ptr = NULL;
                            }

                            if (!VTSS_PORT_BF_GET(tmp_srclist_op->port_mask, j)) {
                                continue;
                            }

                            if (!snd_masq) {
                                if (!ipmc_lib_srclist_clear(&tmp1_srclist[IPMC_INTF_IS_MVR_VAL(ipmc_intf)], 20)) {
                                    T_D("ipmc_lib_srclist_clear failed");
                                }

                                snd_masq = TRUE;
                            }
                            /* For sending Q(MA, A) */
                            IPMC_LIB_SRCT_ADD_EPM(++alcid, &tmp1_srclist[IPMC_INTF_IS_MVR_VAL(ipmc_intf)], grp, tmp_srclist_op);

                            /* Intened to DELETE ALL */
                            VTSS_PORT_BF_SET(tmp_srclist_op->port_mask, j, FALSE);
                            if (IPMC_LIB_BFS_PROT_EMPTY(tmp_srclist_op->port_mask)) {
                                IPMC_TIMER_UNLINK(srct, tmp_srclist_op, &rst);
                                free_ptr = tmp_srclist_op;
                                if (!IPMC_LIB_DB_GET(sf_do_forward_srclist, free_ptr) ||
                                    (free_ptr != tmp_srclist_op)) {
                                    free_ptr = tmp_srclist_op;
                                } else {
                                    (void) IPMC_LIB_DB_DEL(sf_do_not_forward_srclist, tmp_srclist_op);
                                    free_ptr = NULL;
                                }
                            }
                        }
                        if (free_ptr && IPMC_LIB_DB_DEL(sf_do_not_forward_srclist, free_ptr)) {
                            IPMC_MEM_SL_MGIVE(free_ptr, &rst, 203);
                        }
                    }

                    v = local_port_cnt;
                    IPMC_STIMER_RELINK(v, srct, src_list_entry, &rst);
                }

                if (IPMC_LIB_GRP_PORT_DO_SFM(grp_db, j)) {
                    if (!update_sfm_fwd) {
                        ipmc_lib_proc_grp_sfm_tmp4tick(IPMC_INTF_IS_MVR_VAL(ipmc_intf), FALSE, TRUE, grp);

                        /* Prepare for updating SFM FWD */
                        if (ipmc_lib_srclist_struct_copy(
                                grp,
                                &ipmc_sf_permit_srclist[IPMC_INTF_IS_MVR_VAL(ipmc_intf)][j],
                                sf_do_forward_srclist,
                                j) != TRUE) {
                            T_D("vtss_ipmc_data_struct_copy() failed");
                        }
                        if (ipmc_lib_srclist_struct_copy(
                                grp,
                                &ipmc_sf_deny_srclist[IPMC_INTF_IS_MVR_VAL(ipmc_intf)][j],
                                sf_do_not_forward_srclist,
                                j) != TRUE) {
                            T_D("vtss_ipmc_data_struct_copy() failed");
                        }
                    }

                    VTSS_PORT_BF_SET(grp_db->ipmc_sf_port_mode, j, VTSS_IPMC_SF_MODE_INCLUDE);
                    update_sfm_fwd = TRUE;
                } else {
                    u32 inport;

                    mark_for_op_delete = TRUE;
                    /* Prepare to check DELETE */
                    for (inport = 0; inport < local_port_cnt; inport++) {
                        if (!IPMC_LIB_GRP_PORT_DO_SFM(grp_db, inport)) {
                            continue;
                        }

                        mark_for_op_delete = FALSE;
                        break;
                    }

                    /* Send Q(MA) */
                    snd_act = ipmc_lib_get_sq_ssq_action(proxy, ipmc_intf, j);
                    (void) ipmc_lib_packet_tx_sq(fltr, snd_act, grp, ipmc_intf, j, FALSE);
                    if (snd_act != IPMC_SND_HOLD) { /* Querier */
                        v = local_port_cnt;
                        IPMC_RXMT_TIMER_RESET(v, j, rxmt, ipmc_intf, grp_info);
                    }

                    /* DELETE only-when ALL-PORTS are VTSS_IPMC_SF_STATUS_DISABLED */
                    if (mark_for_op_delete) {
                        if (throttling) {
                            u32 chk;

                            for (chk = 0; chk < local_port_cnt; chk++) {
                                if (!VTSS_PORT_BF_GET(grp_db->port_mask, chk) || !throttling->max_no[chk]) {
                                    continue;
                                }

                                --throttling->max_no[chk];
                            }
                        }

                        IPMC_TIMER_UNLINK(fltr, tmr_ptr, &rst);
                        (void) ipmc_lib_group_delete(ipmc_intf, p, rxmt, fltr, srct, grp, proxy, TRUE);

                        break;  /* For Next Timer */
                    } else {
                        if (!update_sfm_fwd) {
                            ipmc_lib_proc_grp_sfm_tmp4tick(IPMC_INTF_IS_MVR_VAL(ipmc_intf), FALSE, TRUE, grp);
                        }

                        if (throttling && throttling->max_no[j]) {
                            --throttling->max_no[j];
                        }
                        VTSS_PORT_BF_SET(grp_db->port_mask, j, FALSE);
                        update_sfm_fwd = TRUE;
                    }
                }

                /* Send Q(MA, A) */
                if (grp_info && snd_masq && IPMC_LIB_DB_GET_COUNT(&tmp1_srclist[IPMC_INTF_IS_MVR_VAL(ipmc_intf)]) &&
                    (grp_info->state != IPMC_OP_CHK_LISTENER)) {
                    snd_act = ipmc_lib_get_sq_ssq_action(proxy, ipmc_intf, j);
                    (void) ipmc_lib_packet_tx_ssq(fltr, snd_act, grp, ipmc_intf, j, &tmp1_srclist[IPMC_INTF_IS_MVR_VAL(ipmc_intf)], FALSE);

                    ipmc_lib_protocol_lower_source_timer(srct, grp,
                                                         ipmc_intf,
                                                         &tmp1_srclist[IPMC_INTF_IS_MVR_VAL(ipmc_intf)],
                                                         j);

                    if (snd_act != IPMC_SND_HOLD) { /* Querier */
                        grp_info->state = IPMC_OP_CHK_LISTENER;
                        /* start rxmt timer */
                        grp_info->rxmt_count[j] = IPMC_TIMER_LLQC(ipmc_intf);
                        IPMC_TIMER_LLQI_SET(ipmc_intf, &grp_info->rxmt_timer[j], rxmt, grp_info);
                    }
                }
            } /* VTSS_IPMC_SF_MODE_EXCLUDE */
        } /* port_iter_getnext(&pit) */

        if (mark_for_op_delete) {
            continue;
        }

        v = local_port_cnt;
        IPMC_FTIMER_RELINK(v, fltr, tmr_ptr, &rst);

        /* Update Database only when update_sfm_fwd */
        if (update_sfm_fwd && !ipmc_lib_group_sync(p, NULL, grp, FALSE, PROC4TICK)) {
            T_D("Update group failed in _ipmc_lib_protocol_fltr_tmr");
        }
        for (j = 0; j < local_port_cnt; j++) {
            lower_timer_srclist = &ipmc_lower_timer_srclist[IPMC_INTF_IS_MVR_VAL(ipmc_intf)][j];
            if (IPMC_LIB_GRP_PORT_DO_SFM(grp_db, j) &&
                IPMC_LIB_DB_GET_COUNT(lower_timer_srclist)) {
                /* ipmc_lib_protocol_lower_source_timer */
                src_list_entry = NULL;
                IPMC_SRCLIST_WALK(lower_timer_srclist, src_list_entry) {
                    if ((tmp_src_list_ptr = ipmc_lib_srclist_adr_get(sf_do_not_forward_srclist, src_list_entry)) != NULL) {
                        if (VTSS_PORT_BF_GET(tmp_src_list_ptr->port_mask, j)) {
                            IPMC_TIMER_LLQT_GSET(ipmc_intf, &tmp_src_list_ptr->tmr.srct_timer.t[j], srct, tmp_src_list_ptr);
                        }
                    }
                    if ((tmp_src_list_ptr = ipmc_lib_srclist_adr_get(sf_do_forward_srclist, src_list_entry)) != NULL) {
                        if (VTSS_PORT_BF_GET(tmp_src_list_ptr->port_mask, j)) {
                            IPMC_TIMER_LLQT_GSET(ipmc_intf, &tmp_src_list_ptr->tmr.srct_timer.t[j], srct, tmp_src_list_ptr);
                        }
                    }
                }
            }
        }

        /* Finalize SFM Checking */
        update_sfm_fwd = FALSE;
        for (j = 0; j < local_port_cnt; j++) {
            if (!IPMC_LIB_GRP_PORT_DO_SFM(grp_db, j)) {
                continue;
            }

            mark_for_op_delete = TRUE;  /* Prepare to check DELETE */
            /* INCLUDE MODE */
            if (IPMC_LIB_GRP_PORT_SFM_IN(grp_db, j)) {
                BOOL    empty_src_list = TRUE;

                src_list_entry = NULL;
                IPMC_SRCLIST_WALK(grp_db->ipmc_sf_do_forward_srclist, src_list_entry) {
                    if (VTSS_PORT_BF_GET(src_list_entry->port_mask, j)) {
                        empty_src_list = FALSE;
                        break;
                    }
                }

                if (empty_src_list) {
                    snd_act = ipmc_lib_get_sq_ssq_action(proxy, ipmc_intf, j);
                    (void) ipmc_lib_packet_tx_sq(fltr, snd_act, grp, ipmc_intf, j, FALSE);
                    if (snd_act != IPMC_SND_HOLD) { /* Querier */
                        v = local_port_cnt;
                        IPMC_RXMT_TIMER_RESET(v, j, rxmt, ipmc_intf, grp_info);
                    }
                    /* Filter Timer needs to be refresh */

                    VTSS_PORT_BF_SET(grp_db->ipmc_sf_port_status, j, VTSS_IPMC_SF_STATUS_DISABLED);
                    if (throttling && throttling->max_no[j]) {
                        --throttling->max_no[j];
                    }
                    VTSS_PORT_BF_SET(grp_db->port_mask, j, FALSE);
                    update_sfm_fwd = TRUE;
                }
            }
        }

        /* Prepare to check DELETE */
        if (mark_for_op_delete) {
            for (j = 0; j < local_port_cnt; j++) {
                if (!IPMC_LIB_GRP_PORT_DO_SFM(grp_db, j)) {
                    continue;
                }

                mark_for_op_delete = FALSE;
                break;
            }

            /* DELETE only-when ALL-PORTS are VTSS_IPMC_SF_STATUS_DISABLED */
            if (mark_for_op_delete) {
                (void) ipmc_lib_group_delete(ipmc_intf, p, rxmt, fltr, srct, grp, proxy, TRUE);
            } else {
                if (update_sfm_fwd && !ipmc_lib_group_sync(p, NULL, grp, FALSE, PROC4TICK)) {
                    T_D("Update group failed in vtss_ipmc_tick_intf_grp_sfm");
                }
            }
        } /* Prepare to check DELETE */
        /* Finalize SFM Checking */
    } /* Process SFM Timers for FWD */
}

vtss_rc ipmc_lib_protocol_group_tmr(BOOL from_mvr,
                                    ipmc_db_ctrl_hdr_t *p,
                                    ipmc_db_ctrl_hdr_t *rxmt,
                                    ipmc_db_ctrl_hdr_t *fltr,
                                    ipmc_db_ctrl_hdr_t *srct,
                                    ipmc_port_throttling_t *g_throttling,
                                    BOOL *g_proxy,
                                    BOOL *l_proxy)
{
    u8          i, local_port_cnt;
    ipmc_time_t current_time;

    if (!p) {
        return VTSS_RC_ERROR;
    }

    (void) ipmc_lib_time_curr_get(&current_time);
    local_port_cnt = ipmc_lib_get_system_local_port_cnt();
    for (i = 0; i < local_port_cnt; i++) {
        if (!ipmc_lib_srclist_clear(&ipmc_lower_timer_srclist[from_mvr][i], 10)) {
            T_D("ipmc_lib_srclist_clear(ipmc_lower_timer_srclist) failed");
        }
    }

    _ipmc_lib_protocol_srct_tmr(from_mvr, p, rxmt, fltr, srct, g_throttling, g_proxy, l_proxy, local_port_cnt, &current_time);
    _ipmc_lib_protocol_fltr_tmr(from_mvr, p, rxmt, fltr, srct, g_throttling, g_proxy, l_proxy, local_port_cnt, &current_time);

    return VTSS_OK;
}

vtss_rc ipmc_lib_protocol_suppression(u16 *timer, u16 timeout, u16 *fld_cnt)
{
    if (timer && fld_cnt && *timer && (--(*timer) == 0)) {
        *timer = timeout;

        *fld_cnt = 0;
    }

    return VTSS_OK;
}

vtss_rc ipmc_lib_protocol_init(void)
{
    port_iter_t pit;

    ipmc_lib_lock();
    if (ipmc_lib_ptc_done_init) {
        ipmc_lib_unlock();
        return VTSS_OK;
    }
    if (ipmc_lib_forward_init() != VTSS_OK) {
        ipmc_lib_unlock();
        return VTSS_RC_ERROR;
    }
    ipmc_lib_ptc_done_init = TRUE;
    ipmc_lib_unlock();

    /* create data base for storing OLD-Operational IPMC SFM FORWARD List */
    if (!ipmc_sf_permit_srclist_created_done) {
        (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
        while (port_iter_getnext(&pit)) {
            if (!IPMC_LIB_DB_TAKE("SNP_SF_PERMIT_SL", &ipmc_sf_permit_srclist[0][pit.iport],
                                  IPMC_NO_OF_SUPPORTED_SRCLIST,
                                  sizeof(ipmc_sfm_srclist_t),
                                  ipmc_lib_srclist_cmp_func)) {
                T_D("IPMC_LIB_DB_TAKE(ipmc_sf_permit_srclist[SNP]) failed");
            }
            if (!IPMC_LIB_DB_TAKE("MVR_SF_PERMIT_SL", &ipmc_sf_permit_srclist[1][pit.iport],
                                  IPMC_NO_OF_SUPPORTED_SRCLIST,
                                  sizeof(ipmc_sfm_srclist_t),
                                  ipmc_lib_srclist_cmp_func)) {
                T_D("IPMC_LIB_DB_TAKE(ipmc_sf_permit_srclist[MVR]) failed");
            }
        }

        ipmc_sf_permit_srclist_created_done = TRUE;
    }

    /* create data base for storing OLD-Operational IPMC SFM DO_NOT_FORWARD List */
    if (!ipmc_sf_deny_srclist_created_done) {
        (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
        while (port_iter_getnext(&pit)) {
            if (!IPMC_LIB_DB_TAKE("SNP_SF_DENY_SL", &ipmc_sf_deny_srclist[0][pit.iport],
                                  IPMC_NO_OF_SUPPORTED_SRCLIST,
                                  sizeof(ipmc_sfm_srclist_t),
                                  ipmc_lib_srclist_cmp_func)) {
                T_D("IPMC_LIB_DB_TAKE(ipmc_sf_deny_srclist[SNP]) failed");
            }
            if (!IPMC_LIB_DB_TAKE("MVR_SF_DENY_SL", &ipmc_sf_deny_srclist[1][pit.iport],
                                  IPMC_NO_OF_SUPPORTED_SRCLIST,
                                  sizeof(ipmc_sfm_srclist_t),
                                  ipmc_lib_srclist_cmp_func)) {
                T_D("IPMC_LIB_DB_TAKE(ipmc_sf_deny_srclist[MVR]) failed");
            }
        }

        ipmc_sf_deny_srclist_created_done = TRUE;
    }

    /* create data base for storing the list for lowering source timers */
    if (!ipmc_lower_timer_srclist_created_done) {
        (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
        while (port_iter_getnext(&pit)) {
            if (!IPMC_LIB_DB_TAKE("SNP_SL_LWR_TMR", &ipmc_lower_timer_srclist[0][pit.iport],
                                  IPMC_NO_OF_SUPPORTED_SRCLIST,
                                  sizeof(ipmc_sfm_srclist_t),
                                  ipmc_lib_srclist_cmp_func)) {
                T_D("IPMC_LIB_DB_TAKE(ipmc_lower_timer_srclist[SNP]) failed");
            }
            if (!IPMC_LIB_DB_TAKE("MVR_SL_LWR_TMR", &ipmc_lower_timer_srclist[1][pit.iport],
                                  IPMC_NO_OF_SUPPORTED_SRCLIST,
                                  sizeof(ipmc_sfm_srclist_t),
                                  ipmc_lib_srclist_cmp_func)) {
                T_D("IPMC_LIB_DB_TAKE(ipmc_lower_timer_srclist[MVR]) failed");
            }
        }

        ipmc_lower_timer_srclist_created_done = TRUE;
    }

    /* create data base for storing Operational IPMC SFM List-1 */
    if (!tmp1_srclist_created_done) {
        if (!IPMC_LIB_DB_TAKE("SNP_TMP1_SRCLIST", &tmp1_srclist[0],
                              IPMC_NO_OF_SUPPORTED_SRCLIST,
                              sizeof(ipmc_sfm_srclist_t),
                              ipmc_lib_srclist_cmp_func)) {
            T_D("IPMC_LIB_DB_TAKE(tmp1_srclist[SNP]) failed");
        }
        if (!IPMC_LIB_DB_TAKE("MVR_TMP1_SRCLIST", &tmp1_srclist[1],
                              IPMC_NO_OF_SUPPORTED_SRCLIST,
                              sizeof(ipmc_sfm_srclist_t),
                              ipmc_lib_srclist_cmp_func)) {
            T_D("IPMC_LIB_DB_TAKE(tmp1_srclist[MVR]) failed");
        }

        tmp1_srclist_created_done = TRUE;
    }

    /* create data base for storing Operational IPMC SFM List-2 */
    if (!tmp2_srclist_created_done) {
        if (!IPMC_LIB_DB_TAKE("SNP_TMP2_SRCLIST", &tmp2_srclist[0],
                              IPMC_NO_OF_SUPPORTED_SRCLIST,
                              sizeof(ipmc_sfm_srclist_t),
                              ipmc_lib_srclist_cmp_func)) {
            T_D("IPMC_LIB_DB_TAKE(tmp2_srclist[SNP]) failed");
        }
        if (!IPMC_LIB_DB_TAKE("MVR_TMP2_SRCLIST", &tmp2_srclist[1],
                              IPMC_NO_OF_SUPPORTED_SRCLIST,
                              sizeof(ipmc_sfm_srclist_t),
                              ipmc_lib_srclist_cmp_func)) {
            T_D("IPMC_LIB_DB_TAKE(tmp2_srclist[MVR]) failed");
        }

        tmp2_srclist_created_done = TRUE;
    }

    memset(grp_info_tmp4rcv, 0x0, sizeof(grp_info_tmp4rcv));
    memset(proc_grp_sfm_tmp4rcv, 0x0, sizeof(proc_grp_sfm_tmp4rcv));
    grp_info_tmp4rcv[0].db.ipmc_sf_do_forward_srclist = &allow_list_tmp4rcv[0];
    grp_info_tmp4rcv[1].db.ipmc_sf_do_forward_srclist = &allow_list_tmp4rcv[1];
    grp_info_tmp4rcv[0].db.ipmc_sf_do_not_forward_srclist = &block_list_tmp4rcv[0];
    grp_info_tmp4rcv[1].db.ipmc_sf_do_not_forward_srclist = &block_list_tmp4rcv[1];
    proc_grp_sfm_tmp4rcv[0].info = &grp_info_tmp4rcv[0];
    proc_grp_sfm_tmp4rcv[1].info = &grp_info_tmp4rcv[1];

    memset(grp_info_tmp4tick, 0x0, sizeof(grp_info_tmp4tick));
    memset(proc_grp_sfm_tmp4tick, 0x0, sizeof(proc_grp_sfm_tmp4tick));
    grp_info_tmp4tick[0].db.ipmc_sf_do_forward_srclist = &allow_list_tmp4tick[0];
    grp_info_tmp4tick[1].db.ipmc_sf_do_forward_srclist = &allow_list_tmp4tick[1];
    grp_info_tmp4tick[0].db.ipmc_sf_do_not_forward_srclist = &block_list_tmp4tick[0];
    grp_info_tmp4tick[1].db.ipmc_sf_do_not_forward_srclist = &block_list_tmp4tick[1];
    proc_grp_sfm_tmp4tick[0].info = &grp_info_tmp4tick[0];
    proc_grp_sfm_tmp4tick[1].info = &grp_info_tmp4tick[1];

    return VTSS_OK;
}
