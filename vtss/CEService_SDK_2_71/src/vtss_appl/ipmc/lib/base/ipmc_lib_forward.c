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

#include "ipmc_lib.h"
#include "ipmc_lib_porting.h"


/* ************************************************************************ **
 *
 * Defines
 *
 * ************************************************************************ */
#define VTSS_TRACE_MODULE_ID        VTSS_MODULE_ID_IPMC_LIB


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
static BOOL                                 ipmc_lib_fwd_done_init = FALSE;


vtss_rc ipmc_lib_forward_process_group_sfm(ipmc_db_ctrl_hdr_t *p, ipmc_group_entry_t *old_grp, ipmc_group_entry_t *new_grp)
{
    ipmc_db_ctrl_hdr_t  *working_src_list;
    ipmc_db_ctrl_hdr_t  *compare_src_list;

    ipmc_group_db_t     *old_grp_db;
    ipmc_group_db_t     *new_grp_db;

    ipmc_sfm_fwd_op_t   fwd_proc_op[VTSS_PORT_ARRAY_SIZE];
    ipmc_sfm_srclist_t  *fwd_proc_sfm, *sfm_chk_sip, *sfm_src, *sfm_tmp;
    u32                 i, j, idx, local_port_cnt;

    vtss_ipv4_t         ip4sip, ip4dip;
    vtss_ipv6_t         ip6sip, ip6dip;

    BOOL                fwd_map[VTSS_PORT_ARRAY_SIZE], src_found;
    BOOL                forwarding, bypass_entry, mark_for_delete;

    if (!p || !old_grp || !new_grp || !old_grp->info || !new_grp->info) {
        T_D("p is %s;old_grp is %s;new_grp is %s;old_grp->info is %s;new_grp->info is %s",
            p ? "OK" : "NULL",
            old_grp ? "OK" : "NULL",
            new_grp ? "OK" : "NULL",
            (old_grp && old_grp->info) ? "OK" : "NULL",
            (new_grp && new_grp->info) ? "OK" : "NULL");
        return VTSS_RC_ERROR;
    }
    if (!IPMC_MEM_SYSTEM_MTAKE(fwd_proc_sfm, sizeof(ipmc_sfm_srclist_t))) {
        T_D("fwd_proc_sfm MTAKE failed");
        return VTSS_RC_ERROR;
    }
    if (!IPMC_MEM_SYSTEM_MTAKE(sfm_chk_sip, (sizeof(ipmc_sfm_srclist_t) * IPMC_NO_OF_SUPPORTED_SRCLIST))) {
        T_D("sfm_chk_sip MTAKE failed");
        IPMC_MEM_SYSTEM_MGIVE(fwd_proc_sfm);
        return VTSS_RC_ERROR;
    }

    old_grp_db = &old_grp->info->db;
    new_grp_db = &new_grp->info->db;
    memset(fwd_proc_op, 0x0, sizeof(fwd_proc_op));
    memset(&ip6sip, 0x0, sizeof(vtss_ipv6_t));
    memset(&ip6dip, 0x0, sizeof(vtss_ipv6_t));

    idx = 0;
    ip4sip = ip4dip = 0;
    forwarding = FALSE;
    local_port_cnt = ipmc_lib_get_system_local_port_cnt();
    for (i = 0; i < IPMC_NO_OF_SUPPORTED_SRCLIST; i++) {
        for (j = 0; j < local_port_cnt; j++) {
            sfm_chk_sip[i].sf_calc[j] = VTSS_IPMC_SF_MODE_NONE;
        }
    }
    /* First, Process OLD SFM-FWD */
    working_src_list = NULL;
    for (i = 0; i < local_port_cnt; i++) {
        /* Determine forwarding process operation */
        if (!IPMC_LIB_GRP_PORT_DO_SFM(old_grp_db, i)) {
            if (!IPMC_LIB_GRP_PORT_DO_SFM(new_grp_db, i)) {
                fwd_proc_op[i] = VTSS_IPMC_SF_FWD_OP_NONE_TO_NONE;
            } else {
                if (IPMC_LIB_GRP_PORT_SFM_IN(new_grp_db, i)) {
                    fwd_proc_op[i] = VTSS_IPMC_SF_FWD_OP_NONE_TO_INCL;
                } else {
                    fwd_proc_op[i] = VTSS_IPMC_SF_FWD_OP_NONE_TO_EXCL;
                }
            }

            continue;
        } else {
            if (IPMC_LIB_GRP_PORT_SFM_IN(old_grp_db, i)) {
                if (!IPMC_LIB_GRP_PORT_DO_SFM(new_grp_db, i)) {
                    fwd_proc_op[i] = VTSS_IPMC_SF_FWD_OP_INCL_TO_NONE;
                } else {
                    if (IPMC_LIB_GRP_PORT_SFM_IN(new_grp_db, i)) {
                        fwd_proc_op[i] = VTSS_IPMC_SF_FWD_OP_INCL_TO_INCL;
                    } else {
                        fwd_proc_op[i] = VTSS_IPMC_SF_FWD_OP_INCL_TO_EXCL;
                    }
                }

                working_src_list = ipmc_lib_get_sf_permit_srclist(IPMC_INTF_IS_MVR_VAL(old_grp->info->interface), i);
            } else {
                if (!IPMC_LIB_GRP_PORT_DO_SFM(new_grp_db, i)) {
                    fwd_proc_op[i] = VTSS_IPMC_SF_FWD_OP_EXCL_TO_NONE;
                } else {
                    if (IPMC_LIB_GRP_PORT_SFM_IN(new_grp_db, i)) {
                        fwd_proc_op[i] = VTSS_IPMC_SF_FWD_OP_EXCL_TO_INCL;
                    } else {
                        fwd_proc_op[i] = VTSS_IPMC_SF_FWD_OP_EXCL_TO_EXCL;
                    }
                }

                working_src_list = ipmc_lib_get_sf_deny_srclist(IPMC_INTF_IS_MVR_VAL(old_grp->info->interface), i);
            }
        } /* if (IPMC_LIB_GRP_PORT_DO_SFM(old_grp_db, i)) */

        if (working_src_list && IPMC_LIB_DB_GET_COUNT(working_src_list)) {
            /* Always use the INDEX in new_grp; old_grp doesn't have proper index here */
            memset(fwd_proc_sfm, 0x0, sizeof(ipmc_sfm_srclist_t));
            sfm_src = fwd_proc_sfm;
            IPMC_SRCLIST_WALK(working_src_list, sfm_src) {
                memset(fwd_map, 0x0, sizeof(fwd_map));
                src_found = FALSE;
                for (j = 0; j < IPMC_NO_OF_SUPPORTED_SRCLIST; j++) {
                    if (sfm_chk_sip[j].sf_calc[i] == VTSS_IPMC_SF_MODE_NONE) {
                        continue;
                    }

                    if (sfm_chk_sip[j].sf_calc[i] != VTSS_PORT_BF_GET(old_grp_db->ipmc_sf_port_mode, i)) {
                        continue;
                    }
                    if (!memcmp(&sfm_chk_sip[j].src_ip.addr[12], &sfm_src->src_ip.addr[12], sizeof(ipmcv4addr))) {
                        src_found = TRUE;
                        break;
                    }
                } /* for (j = 0; j < IPMC_NO_OF_SUPPORTED_SRCLIST; j++) */

                if (src_found) {
                    continue;
                } else {
                    if (idx < IPMC_NO_OF_SUPPORTED_SRCLIST) {
                        memcpy(&sfm_chk_sip[idx].src_ip.addr[12], &sfm_src->src_ip.addr[12], sizeof(ipmcv4addr));
                        sfm_chk_sip[idx].sf_calc[i] = VTSS_PORT_BF_GET(old_grp_db->ipmc_sf_port_mode, i);
                        idx++;
                    } else {
                        T_D("sfm_chk_sip full!");
                        break;
                    }
                } /* if (src_found) */

                if (new_grp->ipmc_version == IPMC_IP_VERSION_MLD) {
                    memcpy(&ip6sip, &sfm_src->src_ip, sizeof(vtss_ipv6_t));
                    memcpy(&ip6dip, &new_grp->group_addr, sizeof(vtss_ipv6_t));
                } else {
                    memcpy((uchar *)&ip4sip, &sfm_src->src_ip.addr[12], sizeof(ipmcv4addr));
                    memcpy((uchar *)&ip4dip, &new_grp->group_addr.addr[12], sizeof(ipmcv4addr));
                }
                fwd_map[i] = TRUE;  /* fwd_map now presents SHOULD BE PROCESS */

                for (j = 0; j < local_port_cnt; j++) {
                    if (j == i) {
                        continue;
                    }
                    if (VTSS_PORT_BF_GET(old_grp_db->ipmc_sf_port_status, i) != VTSS_PORT_BF_GET(old_grp_db->ipmc_sf_port_status, j)) {
                        continue;
                    }
                    if (VTSS_PORT_BF_GET(old_grp_db->ipmc_sf_port_mode, i) != VTSS_PORT_BF_GET(old_grp_db->ipmc_sf_port_mode, j)) {
                        continue;
                    }

                    if (IPMC_LIB_GRP_PORT_SFM_IN(old_grp_db, j)) {
                        compare_src_list = ipmc_lib_get_sf_permit_srclist(IPMC_INTF_IS_MVR_VAL(old_grp->info->interface), j);
                    } else {
                        compare_src_list = ipmc_lib_get_sf_deny_srclist(IPMC_INTF_IS_MVR_VAL(old_grp->info->interface), j);
                    }
                    sfm_tmp = fwd_proc_sfm;
                    if (new_grp->ipmc_version == IPMC_IP_VERSION_MLD) {
                        memcpy(&sfm_tmp->src_ip, &ip6sip, sizeof(vtss_ipv6_t));
                    } else {
                        memcpy(&sfm_tmp->src_ip.addr[12], (uchar *)&ip4sip, sizeof(ipmcv4addr));
                    }

                    if (ipmc_lib_srclist_adr_get(compare_src_list, sfm_tmp) != NULL) {
                        fwd_map[j] = TRUE;
                    }
                    memset(&sfm_tmp->src_ip, 0x0, sizeof(vtss_ipv6_t));
                } /* port_iter_getnext(&inpit) */

                switch ( fwd_proc_op[i] ) {
                case VTSS_IPMC_SF_FWD_OP_INCL_TO_NONE:
                case VTSS_IPMC_SF_FWD_OP_EXCL_TO_NONE:
                    mark_for_delete = TRUE;
                    for (j = 0; j < local_port_cnt; j++) {
                        if ((j == i) || !fwd_map[j]) {
                            continue;
                        }

                        if (mark_for_delete) {
                            mark_for_delete = FALSE;

                            if (IPMC_LIB_GRP_PORT_SFM_IN(old_grp_db, i)) {
                                fwd_map[i] = FALSE;
                            } else {
                                fwd_map[i] = TRUE;
                            }
                        }

                        if (IPMC_LIB_GRP_PORT_SFM_IN(old_grp_db, j)) {
                            fwd_map[j] = TRUE;
                        } else {
                            fwd_map[j] = FALSE;
                        }
                    } /* port_iter_getnext(&inpit) */

                    if (mark_for_delete) {
                        if (new_grp->ipmc_version == IPMC_IP_VERSION_MLD) {
                            memset((uchar *)&ip4sip, 0x0, sizeof(ipmcv4addr));
                            memset((uchar *)&ip4dip, 0x0, sizeof(ipmcv4addr));
                        } else {
                            memset(&ip6sip, 0x0, sizeof(vtss_ipv6_t));
                            memset(&ip6dip, 0x0, sizeof(vtss_ipv6_t));
                        }

                        if (ipmc_lib_porting_set_chip(
                                FALSE, p, new_grp,
                                new_grp->ipmc_version, new_grp->vid,
                                ip4sip, ip4dip, ip6sip, ip6dip,
                                NULL) != VTSS_OK) {
                            T_D("ipmc_lib_porting_set_chip DEL failed");
                        }

                        /* SHOULD Notify MC-Routing - */
                    } else {
                        if (new_grp->ipmc_version == IPMC_IP_VERSION_MLD) {
                            memset((uchar *)&ip4sip, 0x0, sizeof(ipmcv4addr));
                            memset((uchar *)&ip4dip, 0x0, sizeof(ipmcv4addr));
                        } else {
                            memset(&ip6sip, 0x0, sizeof(vtss_ipv6_t));
                            memset(&ip6dip, 0x0, sizeof(vtss_ipv6_t));
                        }

                        if (ipmc_lib_porting_set_chip(
                                TRUE, p, new_grp,
                                new_grp->ipmc_version, new_grp->vid,
                                ip4sip, ip4dip, ip6sip, ip6dip,
                                fwd_map) != VTSS_OK) {
                            T_D("ipmc_lib_porting_set_chip ADD failed");
                        }

                        /* SHOULD Notify MC-Routing + */
                    }

                    break;
                case VTSS_IPMC_SF_FWD_OP_INCL_TO_EXCL:
                case VTSS_IPMC_SF_FWD_OP_EXCL_TO_INCL:
                    mark_for_delete = TRUE;
                    if (IPMC_LIB_GRP_PORT_SFM_IN(new_grp_db, i)) {
                        compare_src_list = new_grp_db->ipmc_sf_do_forward_srclist;
                    } else {
                        compare_src_list = new_grp_db->ipmc_sf_do_not_forward_srclist;
                    }

                    if (ipmc_lib_srclist_adr_get(compare_src_list, sfm_src) != NULL) {
                        mark_for_delete = FALSE;
                    }

                    if (mark_for_delete) {
                        for (j = 0; j < local_port_cnt; j++) {
                            if ((j == i) || !fwd_map[j]) {
                                continue;
                            }

                            if (mark_for_delete) {
                                mark_for_delete = FALSE;

                                if (IPMC_LIB_GRP_PORT_SFM_IN(old_grp_db, i)) {
                                    fwd_map[i] = FALSE;
                                } else {
                                    fwd_map[i] = TRUE;
                                }
                            }

                            if (IPMC_LIB_GRP_PORT_SFM_IN(old_grp_db, j)) {
                                fwd_map[j] = TRUE;
                            } else {
                                fwd_map[j] = FALSE;
                            }
                        } /* port_iter_getnext(&inpit) */

                        if (mark_for_delete) {
                            if (new_grp->ipmc_version == IPMC_IP_VERSION_MLD) {
                                memset((uchar *)&ip4sip, 0x0, sizeof(ipmcv4addr));
                                memset((uchar *)&ip4dip, 0x0, sizeof(ipmcv4addr));
                            } else {
                                memset(&ip6sip, 0x0, sizeof(vtss_ipv6_t));
                                memset(&ip6dip, 0x0, sizeof(vtss_ipv6_t));
                            }

                            if (ipmc_lib_porting_set_chip(
                                    FALSE, p, new_grp,
                                    new_grp->ipmc_version, new_grp->vid,
                                    ip4sip, ip4dip, ip6sip, ip6dip,
                                    NULL) != VTSS_OK) {
                                T_D("ipmc_lib_porting_set_chip DEL failed");
                            }

                            /* SHOULD Notify MC-Routing - */
                        } else {
                            if (new_grp->ipmc_version == IPMC_IP_VERSION_MLD) {
                                memset((uchar *)&ip4sip, 0x0, sizeof(ipmcv4addr));
                                memset((uchar *)&ip4dip, 0x0, sizeof(ipmcv4addr));
                            } else {
                                memset(&ip6sip, 0x0, sizeof(vtss_ipv6_t));
                                memset(&ip6dip, 0x0, sizeof(vtss_ipv6_t));
                            }

                            if (ipmc_lib_porting_set_chip(
                                    TRUE, p, new_grp,
                                    new_grp->ipmc_version, new_grp->vid,
                                    ip4sip, ip4dip, ip6sip, ip6dip,
                                    fwd_map) != VTSS_OK) {
                                T_D("ipmc_lib_porting_set_chip ADD failed");
                            }

                            /* SHOULD Notify MC-Routing + */
                        }
                    }

                    break;
                case VTSS_IPMC_SF_FWD_OP_INCL_TO_INCL:
                case VTSS_IPMC_SF_FWD_OP_EXCL_TO_EXCL:
                    mark_for_delete = TRUE;
                    for (j = 0; j < local_port_cnt; j++) {
                        if (IPMC_LIB_GRP_PORT_SFM_IN(new_grp_db, j)) {
                            compare_src_list = new_grp_db->ipmc_sf_do_forward_srclist;
                        } else {
                            compare_src_list = new_grp_db->ipmc_sf_do_not_forward_srclist;
                        }

                        if (ipmc_lib_srclist_adr_get(compare_src_list, sfm_src) != NULL) {
                            mark_for_delete = FALSE;
                            break;
                        }
                    } /* port_iter_getnext(&inpit) */

                    if (mark_for_delete) {
                        for (j = 0; j < local_port_cnt; j++) {
                            if ((j == i) || !fwd_map[j]) {
                                continue;
                            }

                            if (mark_for_delete) {
                                mark_for_delete = FALSE;

                                if (IPMC_LIB_GRP_PORT_SFM_IN(old_grp_db, i)) {
                                    fwd_map[i] = FALSE;
                                } else {
                                    fwd_map[i] = TRUE;
                                }
                            }

                            if (IPMC_LIB_GRP_PORT_SFM_IN(old_grp_db, j)) {
                                fwd_map[j] = TRUE;
                            } else {
                                fwd_map[j] = FALSE;
                            }
                        } /* port_iter_getnext(&inpit) */

                        if (mark_for_delete) {
                            if (new_grp->ipmc_version == IPMC_IP_VERSION_MLD) {
                                memset((uchar *)&ip4sip, 0x0, sizeof(ipmcv4addr));
                                memset((uchar *)&ip4dip, 0x0, sizeof(ipmcv4addr));
                            } else {
                                memset(&ip6sip, 0x0, sizeof(vtss_ipv6_t));
                                memset(&ip6dip, 0x0, sizeof(vtss_ipv6_t));
                            }

                            if (ipmc_lib_porting_set_chip(
                                    FALSE, p, new_grp,
                                    new_grp->ipmc_version, new_grp->vid,
                                    ip4sip, ip4dip, ip6sip, ip6dip,
                                    NULL) != VTSS_OK) {
                                T_D("ipmc_lib_porting_set_chip DEL failed");
                            }

                            /* SHOULD Notify MC-Routing - */
                        } else {
                            if (new_grp->ipmc_version == IPMC_IP_VERSION_MLD) {
                                memset((uchar *)&ip4sip, 0x0, sizeof(ipmcv4addr));
                                memset((uchar *)&ip4dip, 0x0, sizeof(ipmcv4addr));
                            } else {
                                memset(&ip6sip, 0x0, sizeof(vtss_ipv6_t));
                                memset(&ip6dip, 0x0, sizeof(vtss_ipv6_t));
                            }

                            if (ipmc_lib_porting_set_chip(
                                    TRUE, p, new_grp,
                                    new_grp->ipmc_version, new_grp->vid,
                                    ip4sip, ip4dip, ip6sip, ip6dip,
                                    fwd_map) != VTSS_OK) {
                                T_D("ipmc_lib_porting_set_chip ADD failed");
                            }

                            /* SHOULD Notify MC-Routing + */
                        }
                    }

                    break;
                case VTSS_IPMC_SF_FWD_OP_NONE_TO_INCL:
                case VTSS_IPMC_SF_FWD_OP_NONE_TO_EXCL:
                case VTSS_IPMC_SF_FWD_OP_NONE_TO_NONE:
                default:
                    /* leave it for NEW SFM-FWD & should not happen here */

                    break;
                } /* switch ( fwd_proc_op[i] ) */
            } /*IPMC_SRCLIST_WALK(working_src_list, sfm_src) */
        } else {
            /* Handle ASM only! */
            BOOL    delete_flag = TRUE;

            switch ( fwd_proc_op[i] ) {
            case VTSS_IPMC_SF_FWD_OP_INCL_TO_NONE:
            case VTSS_IPMC_SF_FWD_OP_EXCL_TO_NONE:
                /* sip is zero for ASM */
                if (new_grp->ipmc_version == IPMC_IP_VERSION_MLD) {
                    memset(&ip6sip, 0x0, sizeof(vtss_ipv6_t));
                    memcpy(&ip6dip, &new_grp->group_addr, sizeof(vtss_ipv6_t));
                    memset((uchar *)&ip4sip, 0x0, sizeof(ipmcv4addr));
                    memset((uchar *)&ip4dip, 0x0, sizeof(ipmcv4addr));

                } else {
                    memset((uchar *)&ip4sip, 0x0, sizeof(ipmcv4addr));
                    memcpy((uchar *)&ip4dip, &new_grp->group_addr.addr[12], sizeof(ipmcv4addr));
                    memset(&ip6sip, 0x0, sizeof(vtss_ipv6_t));
                    memset(&ip6dip, 0x0, sizeof(vtss_ipv6_t));

                }

                memset(fwd_map, 0x0, sizeof(fwd_map));
                for (j = 0; j < local_port_cnt; j++) {
                    fwd_map[j] = VTSS_PORT_BF_GET(new_grp_db->port_mask, j);
                    if (delete_flag && fwd_map[j]) {
                        delete_flag = FALSE;
                    }
                } /* port_iter_getnext(&inpit) */

                if (delete_flag) {
                    for (j = 0; j < local_port_cnt; j++) {
                        if (i == j) {
                            continue;
                        }

                        if (IPMC_LIB_GRP_PORT_DO_SFM(new_grp_db, j)) {
                            delete_flag = FALSE;
                        }
                    } /* port_iter_getnext(&inpit) */
                }

                if (delete_flag) {
                    if (ipmc_lib_porting_set_chip(
                            FALSE, p, new_grp,
                            new_grp->ipmc_version, new_grp->vid,
                            ip4sip, ip4dip, ip6sip, ip6dip,
                            fwd_map) != VTSS_OK) {
                        T_D("ipmc_lib_porting_set_chip DEL failed");
                    }
                } else {
                    if (ipmc_lib_porting_set_chip(
                            TRUE, p, new_grp,
                            new_grp->ipmc_version, new_grp->vid,
                            ip4sip, ip4dip, ip6sip, ip6dip,
                            fwd_map) != VTSS_OK) {
                        T_D("ipmc_lib_porting_set_chip ADD failed");
                    }
                }

                break;
            case VTSS_IPMC_SF_FWD_OP_INCL_TO_EXCL:
            case VTSS_IPMC_SF_FWD_OP_EXCL_TO_INCL:
            case VTSS_IPMC_SF_FWD_OP_INCL_TO_INCL:
            case VTSS_IPMC_SF_FWD_OP_EXCL_TO_EXCL:
                /* leave it for NEW SFM-FWD */

                break;
            case VTSS_IPMC_SF_FWD_OP_NONE_TO_INCL:
            case VTSS_IPMC_SF_FWD_OP_NONE_TO_EXCL:
            case VTSS_IPMC_SF_FWD_OP_NONE_TO_NONE:
            default:
                /* leave it for NEW SFM-FWD & should not happen here */

                break;
            } /* switch ( fwd_proc_op[i] ) */
        } /* if (IPMC_LIB_DB_GET_COUNT(working_src_list)) */
    } /* port_iter_getnext(&pit) */

    /* Second, Process NEW SFM-FWD */
    idx = 0;
    for (i = 0; i < IPMC_NO_OF_SUPPORTED_SRCLIST; i++) {
        for (j = 0; j < local_port_cnt; j++) {
            sfm_chk_sip[i].sf_calc[j] = VTSS_IPMC_SF_MODE_NONE;
        }
    }

    working_src_list = NULL;
    for (i = 0; i < local_port_cnt; i++) {
        bypass_entry = FALSE;
        switch ( fwd_proc_op[i] ) {
        case VTSS_IPMC_SF_FWD_OP_NONE_TO_INCL:
        case VTSS_IPMC_SF_FWD_OP_INCL_TO_INCL:
        case VTSS_IPMC_SF_FWD_OP_EXCL_TO_INCL:
            working_src_list = new_grp_db->ipmc_sf_do_forward_srclist;
            forwarding = TRUE;

            break;
        case VTSS_IPMC_SF_FWD_OP_NONE_TO_EXCL:
        case VTSS_IPMC_SF_FWD_OP_INCL_TO_EXCL:
        case VTSS_IPMC_SF_FWD_OP_EXCL_TO_EXCL:
            working_src_list = new_grp_db->ipmc_sf_do_not_forward_srclist;
            forwarding = FALSE;

            break;
        case VTSS_IPMC_SF_FWD_OP_INCL_TO_NONE:
        case VTSS_IPMC_SF_FWD_OP_EXCL_TO_NONE:
            bypass_entry = TRUE;
            /* do nothing */

            break;
        case VTSS_IPMC_SF_FWD_OP_NONE_TO_NONE:
        default:
            bypass_entry = TRUE;
            /* do nothing & should not happen here */

            break;
        }

        if (bypass_entry || !working_src_list) {
            continue;
        }

        memset(fwd_proc_sfm, 0x0, sizeof(ipmc_sfm_srclist_t));
        sfm_src = fwd_proc_sfm;
        IPMC_SRCLIST_WALK(working_src_list, sfm_src) {
            memset(fwd_map, 0x0, sizeof(fwd_map));
            src_found = FALSE;
            for (j = 0; j < IPMC_NO_OF_SUPPORTED_SRCLIST; j++) {
                if (sfm_chk_sip[j].sf_calc[i] == VTSS_IPMC_SF_MODE_NONE) {
                    continue;
                }
                if (sfm_chk_sip[j].sf_calc[i] != VTSS_PORT_BF_GET(new_grp_db->ipmc_sf_port_mode, i)) {
                    continue;
                }
                if (!memcmp(&sfm_chk_sip[j].src_ip.addr[12], &sfm_src->src_ip.addr[12], sizeof(ipmcv4addr))) {
                    src_found = TRUE;
                    break;
                }
            } /* for (j = 0; j < IPMC_NO_OF_SUPPORTED_SRCLIST; j++) */

            if (src_found) {
                continue;
            } else {
                if (idx < IPMC_NO_OF_SUPPORTED_SRCLIST) {
                    memcpy(&sfm_chk_sip[idx].src_ip.addr[12], &sfm_src->src_ip.addr[12], sizeof(ipmcv4addr));
                    sfm_chk_sip[idx].sf_calc[i] = VTSS_PORT_BF_GET(new_grp_db->ipmc_sf_port_mode, i);
                    idx++;
                } else {
                    T_D("sfm_chk_sip full!");
                    break;
                }
            } /* if (src_found) */

            if (new_grp->ipmc_version == IPMC_IP_VERSION_MLD) {
                memcpy(&ip6sip, &sfm_src->src_ip, sizeof(vtss_ipv6_t));
                memcpy(&ip6dip, &new_grp->group_addr, sizeof(vtss_ipv6_t));
            } else {
                memcpy((uchar *)&ip4sip, &sfm_src->src_ip.addr[12], sizeof(ipmcv4addr));
                memcpy((uchar *)&ip4dip, &new_grp->group_addr.addr[12], sizeof(ipmcv4addr));
            }
            fwd_map[i] = forwarding;    /* fwd_map now presents SHOULD BE FORWARD */

            for (j = 0; j < local_port_cnt; j++) {
                if (j == i) {
                    continue;
                }
                if (VTSS_PORT_BF_GET(new_grp_db->ipmc_sf_port_status, i) != VTSS_PORT_BF_GET(new_grp_db->ipmc_sf_port_status, j)) {
                    continue;
                }
                if (VTSS_PORT_BF_GET(new_grp_db->ipmc_sf_port_mode, i) != VTSS_PORT_BF_GET(new_grp_db->ipmc_sf_port_mode, j)) {
                    continue;
                }

                if (IPMC_LIB_GRP_PORT_SFM_IN(new_grp_db, j)) {
                    compare_src_list = new_grp_db->ipmc_sf_do_forward_srclist;
                } else {
                    compare_src_list = new_grp_db->ipmc_sf_do_not_forward_srclist;
                }
                sfm_tmp = fwd_proc_sfm;
                if (new_grp->ipmc_version == IPMC_IP_VERSION_MLD) {
                    memcpy(&sfm_tmp->src_ip, &ip6sip, sizeof(vtss_ipv6_t));
                } else {
                    memcpy(&sfm_tmp->src_ip.addr[12], (uchar *)&ip4sip, sizeof(ipmcv4addr));
                }

                if (ipmc_lib_srclist_adr_get(compare_src_list, sfm_tmp) != NULL) {
                    fwd_map[j] = forwarding;
                }
                memset(&sfm_tmp->src_ip, 0x0, sizeof(vtss_ipv6_t));
            } /* port_iter_getnext(&inpit) */

            if (new_grp->ipmc_version == IPMC_IP_VERSION_MLD) {
                memset((uchar *)&ip4sip, 0x0, sizeof(ipmcv4addr));
                memset((uchar *)&ip4dip, 0x0, sizeof(ipmcv4addr));
            } else {
                memset(&ip6sip, 0x0, sizeof(vtss_ipv6_t));
                memset(&ip6dip, 0x0, sizeof(vtss_ipv6_t));
            }

            if (ipmc_lib_porting_set_chip(
                    TRUE, p, new_grp,
                    new_grp->ipmc_version, new_grp->vid,
                    ip4sip, ip4dip, ip6sip, ip6dip,
                    fwd_map) != VTSS_OK) {
                T_D("ipmc_lib_porting_set_chip ADD failed");
            }

            /* SHOULD Notify MC-Routing + */
        } /* IPMC_SRCLIST_WALK(working_src_list, sfm_src) */
    } /* port_iter_getnext(&pit) */

    IPMC_MEM_SYSTEM_MGIVE(sfm_chk_sip);
    IPMC_MEM_SYSTEM_MGIVE(fwd_proc_sfm);
    return VTSS_OK;
}

vtss_rc ipmc_lib_forward_init(void)
{
    if (ipmc_lib_fwd_done_init) {
        return VTSS_OK;
    }

    ipmc_lib_fwd_done_init = TRUE;
    return VTSS_OK;
}
