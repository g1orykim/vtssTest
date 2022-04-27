/*

 Vitesse Switch API software.

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
#include "icli_api.h"
#include "icli_porting_util.h"
#include "mep_api.h"
#include "misc_api.h"

#ifdef VTSS_SW_OPTION_ICFG
#include "icfg_api.h"
#endif

/***************************************************************************/
/*  Internal types                                                         */
/****************************************************************************/

/***************************************************************************/
/*  Internal functions                                                     */
/****************************************************************************/
static void mep_print_error(u32 session_id, vtss_rc rc)
{
    ICLI_PRINTF("Error: %s\n", error_txt(rc));
}

/***************************************************************************/
/*  Functions called by iCLI                                                */
/****************************************************************************/

void mep_show_mep(i32 session_id, icli_range_t *inst,
                  BOOL has_peer, BOOL has_cc, BOOL has_lm, BOOL has_dm, BOOL has_lt, BOOL has_lb, BOOL has_tst, BOOL has_aps, BOOL has_client, BOOL has_ais, BOOL has_lck, BOOL has_detail)
{
    u32                          i, j, k, t, list_cnt, min, max, eps_count;
    u16                          eps_inst[MEP_EPS_MAX];
    BOOL                         vola=FALSE;;
    vtss_mep_mgmt_conf_t         config;
    vtss_mep_mgmt_state_t        state;
    vtss_mep_mgmt_cc_conf_t      cc_config;
    vtss_mep_mgmt_pm_conf_t      pm_config;
    vtss_mep_mgmt_lm_conf_t      lm_config;
    vtss_mep_mgmt_lm_state_t     lm_state;
    vtss_mep_mgmt_dm_conf_t      dm_config;
    vtss_mep_mgmt_dm_state_t     dmr_state, dm1_state_far_to_near, dm1_state_near_to_far;
    vtss_mep_mgmt_lt_conf_t      lt_config;
    vtss_mep_mgmt_lt_state_t     lt_state;
    vtss_mep_mgmt_lb_conf_t      lb_config;
    vtss_mep_mgmt_lb_state_t     lb_state;
    vtss_mep_mgmt_tst_conf_t     tst_config;
    vtss_mep_mgmt_tst_state_t    tst_state;
    vtss_mep_mgmt_aps_conf_t     aps_config;
    vtss_mep_mgmt_client_conf_t  client_config;
    vtss_mep_mgmt_ais_conf_t     ais_config;
    vtss_mep_mgmt_lck_conf_t     lck_config;
    u8                           mac[VTSS_MEP_MAC_LENGTH];
    char                         string[MEP_EPS_MAX * 6];
    char                         buf[ICLI_PORTING_STR_BUF_SIZE];
    vtss_rc                      rc;

    if (!has_peer && !has_cc && !has_lm && !has_dm && !has_lt && !has_lb && !has_tst && !has_aps && !has_client && !has_ais && !has_lck) {
        ICLI_PRINTF("\n");
        ICLI_PRINTF("MEP state is:\n");
        ICLI_PRINTF("%6s", "Inst");
        ICLI_PRINTF("%8s", "cLevel");
        ICLI_PRINTF("%6s", "cMeg");
        ICLI_PRINTF("%6s", "cMep");
        ICLI_PRINTF("%6s", "cAis");
        ICLI_PRINTF("%6s", "cLck");
        ICLI_PRINTF("%6s", "cSsf");
        ICLI_PRINTF("%6s", "aBlk");
        ICLI_PRINTF("%6s", "aTsf");
        ICLI_PRINTF("%10s", "Peer MEP");
        ICLI_PRINTF("%6s", "cLoc");
        ICLI_PRINTF("%6s", "cRdi");
        ICLI_PRINTF("%9s", "cPeriod");
        ICLI_PRINTF("%7s", "cPrio");
            ICLI_PRINTF("\n");

        if (inst == NULL) {
            list_cnt = 1;
        }
        else  {
            list_cnt = inst->u.sr.cnt;
        }

        for (i=0; i<list_cnt; ++i) {
            if (inst == NULL) {
                min = 1;
                max = MEP_INSTANCE_MAX;
            }
            else {
                min = inst->u.sr.range[i].min;
                max = inst->u.sr.range[i].max;
            }

            if ((min == 0) || (max == 0)) {
                ICLI_PRINTF("Invalid MEP instance number\n");
                continue;
            }
            for (j=min-1; j<max; ++j) {
                if (((rc = mep_mgmt_conf_get(j, mac, &eps_count, eps_inst, &config)) != VTSS_RC_OK) && (rc != MEP_RC_VOLATILE)) {
                    mep_print_error(session_id, rc);
                    continue;
                }
                if (!config.enable) {
                    continue;
                }
                if (config.mode == VTSS_MEP_MGMT_MIP) {
                    continue;
                }
                if ((rc = mep_mgmt_state_get(j, &state)) != VTSS_RC_OK) {
                    mep_print_error(session_id, rc);
                    continue;
                }
                ICLI_PRINTF("%6u", j+1);
                ICLI_PRINTF("%8s",  (state.cLevel) ? "True" : "False");
                ICLI_PRINTF("%6s",  (state.cMeg) ? "True" : "False");
                ICLI_PRINTF("%6s",  (state.cMep) ? "True" : "False");
                ICLI_PRINTF("%6s",  (state.cAis) ? "True" : "False");
                ICLI_PRINTF("%6s",  (state.cLck) ? "True" : "False");
                ICLI_PRINTF("%6s",  (state.cSsf) ? "True" : "False");
                ICLI_PRINTF("%6s",  (state.aBlk) ? "True" : "False");
                ICLI_PRINTF("%6s",  (state.aTsf) ? "True" : "False");
                for (k=0; k<config.peer_count; ++k) {
                    if (k != 0) {
                        ICLI_PRINTF("%56s", " ");
                    }
                    ICLI_PRINTF("%10u", config.peer_mep[k]);
                    ICLI_PRINTF("%6s",  (state.cLoc[k]) ? "True" : "False");
                    ICLI_PRINTF("%6s",  (state.cRdi[k]) ? "True" : "False");
                    ICLI_PRINTF("%9s",  (state.cPeriod[k]) ? "True" : "False");
                    ICLI_PRINTF("%7s",  (state.cPrio[k]) ? "True" : "False");
                    ICLI_PRINTF("\n");
                }
                if (config.peer_count == 0) {
                    ICLI_PRINTF("\n");
                }
            }
        }
        ICLI_PRINTF("\n");

        if (has_detail) {
            ICLI_PRINTF("\n");
            ICLI_PRINTF("MEP Basic Configuration is:\n");
            ICLI_PRINTF("%6s", "Inst");
            ICLI_PRINTF("%6s", "Mode");
            ICLI_PRINTF("%5s", "Voe");
            ICLI_PRINTF("%4s", "PM");
            ICLI_PRINTF("%6s", "Vola");
            ICLI_PRINTF("%8s", "Direct");
            ICLI_PRINTF("%23s", "Port");
            ICLI_PRINTF("%6s", "Dom");
            ICLI_PRINTF("%7s", "Level");
            ICLI_PRINTF("%13s", "Format");
            ICLI_PRINTF("%18s", "Name");
            ICLI_PRINTF("%18s", "Meg id");
            ICLI_PRINTF("%8s", "Mep id");
            ICLI_PRINTF("%6s", "Vid");
            ICLI_PRINTF("%6s", "Flow");
            ICLI_PRINTF("%11s", "Eps");
            ICLI_PRINTF("%19s", "MAC");
            ICLI_PRINTF("\n");

            if (inst == NULL) {
                list_cnt = 1;
            }
            else {
                list_cnt = inst->u.sr.cnt;
            }

            for (i=0; i<list_cnt; ++i) {
                if (inst == NULL) {
                    min = 1;
                    max = MEP_INSTANCE_MAX;
                }
                else {
                    min = inst->u.sr.range[i].min;
                    max = inst->u.sr.range[i].max;
                }

                if ((min == 0) || (max == 0)) {
                    ICLI_PRINTF("Invalid MEP instance number\n");
                    continue;
                }
                for (j=min-1; j<max; ++j) {
                    if (((rc = mep_mgmt_conf_get(j, mac, &eps_count, eps_inst, &config)) != VTSS_RC_OK) && (rc != MEP_RC_VOLATILE)) {
                        mep_print_error(session_id, rc);
                        continue;
                    }
                    if (!config.enable) {
                        continue;
                    }
                    vola = (rc == MEP_RC_VOLATILE);
                    if ((rc = mep_mgmt_pm_conf_get(j, &pm_config)) != VTSS_RC_OK) {
                        mep_print_error(session_id, rc);
                        continue;
                    }
                    ICLI_PRINTF("%6u", j+1);
                    switch (config.mode) {
                        case VTSS_MEP_MGMT_MEP:       ICLI_PRINTF("%6s", "Mep");     break;
                        case VTSS_MEP_MGMT_MIP:       ICLI_PRINTF("%6s", "Mip");     break;
                    }
                    if (config.voe) {
                        ICLI_PRINTF("%5s", "Voe");
                    }
                    else {
                        ICLI_PRINTF("%5s", " ");
                    }
                    if (pm_config.enable) {
                        ICLI_PRINTF("%4s", "PM");
                    }
                    else {
                        ICLI_PRINTF("%4s", " ");
                    }
                    if (vola) {
                        ICLI_PRINTF("%6s", "Vola");
                    }
                    else {
                        ICLI_PRINTF("%6s", " ");
                    }
                    switch (config.direction) {
                        case VTSS_MEP_MGMT_DOWN:  ICLI_PRINTF("%8s", "Down");     break;
                        case VTSS_MEP_MGMT_UP:    ICLI_PRINTF("%8s", "Up");      break;
                    }
                    ICLI_PRINTF("%23s", icli_port_info_txt(VTSS_USID_START, iport2uport(config.port), buf));

                    switch (config.domain) {
                        case VTSS_MEP_MGMT_PORT:       ICLI_PRINTF("%6s", "Port");     break;
//                      case VTSS_MEP_MGMT_ESP:        ICLI_PRINTF("%6s", "Eps");      break;
                        case VTSS_MEP_MGMT_EVC:        ICLI_PRINTF("%6s", "Evc");      break;
                        case VTSS_MEP_MGMT_VLAN:       ICLI_PRINTF("%6s", "Vlan");     break;
//                      case VTSS_MEP_MGMT_MPLS:       ICLI_PRINTF("%6s", "Mpls");     break;
                    }
                    ICLI_PRINTF("%7u", config.level);
                    if (config.mode == VTSS_MEP_MGMT_MEP) {
                        switch (config.format) {
                            case VTSS_MEP_MGMT_ITU_ICC:     ICLI_PRINTF("%13s", "ITU ICC");     break;
                            case VTSS_MEP_MGMT_IEEE_STR:    ICLI_PRINTF("%13s", "IEEE String");   break;
                            case VTSS_MEP_MGMT_ITU_CC_ICC:  ICLI_PRINTF("%13s", "ITU ICC+CC");   break;
                        }
                        ICLI_PRINTF("%18s", config.name);
                        ICLI_PRINTF("%18s", config.meg);
                        ICLI_PRINTF("%8u", config.mep);
                    }
                    else {
                        ICLI_PRINTF("%13s"," ");
                        ICLI_PRINTF("%18s"," ");
                        ICLI_PRINTF("%18s"," ");
                        ICLI_PRINTF("%8s"," ");
                    }
                    ICLI_PRINTF("%6u", config.vid);
                    ICLI_PRINTF("%6u", (config.domain != VTSS_MEP_MGMT_VLAN) ? config.flow+1 : config.flow);
                    if (config.mode == VTSS_MEP_MGMT_MEP) {
                        if (eps_count) {
                            string[0] = '\0';
                            for (k=0; k<MEP_EPS_MAX && k<3 && k<eps_count; ++k) {
                                sprintf(string, "%s%u-", string, eps_inst[k]+1);
                            }
                            string[strlen(string)-1] = '\0';
                            ICLI_PRINTF("%11s", string);
                        }
                        else {
                            ICLI_PRINTF("%11u", 0);
                        }
                    }
                    else {
                        ICLI_PRINTF("         ");
                    }
                    ICLI_PRINTF("  %02X-%02X-%02X-%02X-%02X-%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
                    ICLI_PRINTF("\n");
                }
            }
            ICLI_PRINTF("\n");
        }
    }

    if (has_peer && has_detail) {
        ICLI_PRINTF("\n");
        ICLI_PRINTF("MEP Peer MEP Configuration is:\n");
        ICLI_PRINTF("%9s", "Inst");
        ICLI_PRINTF("%12s", "Peer id");
        ICLI_PRINTF("%22s", "Peer MAC");
        ICLI_PRINTF("\n");

        if (inst == NULL) {
            list_cnt = 1;
        }
        else {
            list_cnt = inst->u.sr.cnt;
        }
        for (i=0; i<list_cnt; ++i) {
            if (inst == NULL) {
                min = 1;
                max = MEP_INSTANCE_MAX;
            }
            else {
                min = inst->u.sr.range[i].min;
                max = inst->u.sr.range[i].max;
            }

            if ((min == 0) || (max == 0)) {
                ICLI_PRINTF("Invalid MEP instance number\n");
                continue;
            }
            for (j=min-1; j<max; ++j) {
                if (((rc = mep_mgmt_conf_get(j, mac, &eps_count, eps_inst, &config)) != VTSS_RC_OK) && (rc != MEP_RC_VOLATILE)) {
                    mep_print_error(session_id, rc);
                    continue;
                }
                if (!config.enable) {
                    continue;
                }
                if (config.peer_count != 0) {
                    ICLI_PRINTF("%9u", j+1);
                    for (k=0; k<config.peer_count; ++k) {
                        if (k != 0) {
                            ICLI_PRINTF("         ");
                        }
                        ICLI_PRINTF("%12u", config.peer_mep[k]);
                        ICLI_PRINTF("     %02X-%02X-%02X-%02X-%02X-%02X", config.peer_mac[k][0], config.peer_mac[k][1], config.peer_mac[k][2],
                                                                          config.peer_mac[k][3], config.peer_mac[k][4], config.peer_mac[k][5]);
                        ICLI_PRINTF("\n");
                    }
                }
            }
        }
        ICLI_PRINTF("\n");
    }

    if (has_cc && has_detail) {
        ICLI_PRINTF("\n");
        ICLI_PRINTF("MEP CC Configuration is:\n");
        ICLI_PRINTF("%9s", "Inst");
        ICLI_PRINTF("%9s", "Prio");
        ICLI_PRINTF("%11s", "Period");
        ICLI_PRINTF("\n");

        if (inst == NULL) {
            list_cnt = 1;
        }
        else {
            list_cnt = inst->u.sr.cnt;
        }

        for (i=0; i<list_cnt; ++i) {
            if (inst == NULL) {
                min = 1;
                max = MEP_INSTANCE_MAX;
            }
            else {
                min = inst->u.sr.range[i].min;
                max = inst->u.sr.range[i].max;
            }

            if ((min == 0) || (max == 0)) {
                ICLI_PRINTF("Invalid MEP instance number\n");
                continue;
            }
            for (j=min-1; j<max; ++j) {
                if ((rc = mep_mgmt_cc_conf_get(j, &cc_config)) != VTSS_RC_OK) {
                    mep_print_error(session_id, rc);
                    continue;
                }
                if (cc_config.enable) {
                    ICLI_PRINTF("%9u", j+1);
                    ICLI_PRINTF("%9u", cc_config.prio);
                    switch (cc_config.period) {
                        case VTSS_MEP_MGMT_PERIOD_300S:       ICLI_PRINTF("%11s", "300s");   break;
                        case VTSS_MEP_MGMT_PERIOD_100S:       ICLI_PRINTF("%11s", "100s");   break;
                        case VTSS_MEP_MGMT_PERIOD_10S:        ICLI_PRINTF("%11s", "10s");    break;
                        case VTSS_MEP_MGMT_PERIOD_1S:         ICLI_PRINTF("%11s", "1s");     break;
                        case VTSS_MEP_MGMT_PERIOD_6M:         ICLI_PRINTF("%11s", "6m");     break;
                        case VTSS_MEP_MGMT_PERIOD_1M:         ICLI_PRINTF("%11s", "1m");     break;
                        case VTSS_MEP_MGMT_PERIOD_6H:         ICLI_PRINTF("%11s", "6h");     break;
                        default:                              ICLI_PRINTF("%11s", "Unknown");
                    }
                    ICLI_PRINTF("\n");
                }
            }
        }
        ICLI_PRINTF("\n");
    }

    if (has_lm) {
        ICLI_PRINTF("\n");
        ICLI_PRINTF("MEP LM state is:\n");
        ICLI_PRINTF("%9s", "Inst");
        ICLI_PRINTF("%7s", "Tx");
        ICLI_PRINTF("%7s", "Rx");
        ICLI_PRINTF("%15s", "Near Count");
        ICLI_PRINTF("%14s", "Far Count");
        ICLI_PRINTF("%15s", "Near Ratio");
        ICLI_PRINTF("%14s", "Far Ratio");
        ICLI_PRINTF("\n");

        if (inst == NULL) {
            list_cnt = 1;
        }
        else {
            list_cnt = inst->u.sr.cnt;
        }
        for (i=0; i<list_cnt; ++i) {
            if (inst == NULL) {
                min = 1;
                max = MEP_INSTANCE_MAX;
            }
            else {
                min = inst->u.sr.range[i].min;
                max = inst->u.sr.range[i].max;
            }

            if ((min == 0) || (max == 0)) {
                ICLI_PRINTF("Invalid MEP instance number\n");
                continue;
            }
            for (j=min-1; j<max; ++j) {
                if ((rc = mep_mgmt_lm_state_get(j, &lm_state)) != VTSS_RC_OK) {
                    if (rc != MEP_RC_NOT_ENABLED) {
                        mep_print_error(session_id, rc);
                    }
                    continue;
                }
                ICLI_PRINTF("%9u", j+1);
                ICLI_PRINTF("%7u", lm_state.tx_counter);
                ICLI_PRINTF("%7u", lm_state.rx_counter);
                ICLI_PRINTF("%15u", lm_state.near_los_counter);
                ICLI_PRINTF("%14u", lm_state.far_los_counter);
                ICLI_PRINTF("%15u", lm_state.near_los_ratio);
                ICLI_PRINTF("%14u", lm_state.far_los_ratio);
                ICLI_PRINTF("\n");
            }
        }
        ICLI_PRINTF("\n");

        if (has_detail) {
            ICLI_PRINTF("\n");
            ICLI_PRINTF("MEP LM Configuration is:\n");
            ICLI_PRINTF("%9s", "Inst");
            ICLI_PRINTF("%9s", "Prio");
            ICLI_PRINTF("%9s", "Cast");
            ICLI_PRINTF("%11s", "Ended");
            ICLI_PRINTF("%11s", "Period");
            ICLI_PRINTF("%8s", "Flr");
            ICLI_PRINTF("\n");

            if (inst == NULL) {
                list_cnt = 1;
            }
            else {
                list_cnt = inst->u.sr.cnt;
            }

            for (i=0; i<list_cnt; ++i) {
                if (inst == NULL) {
                    min = 1;
                    max = MEP_INSTANCE_MAX;
                }
                else {
                    min = inst->u.sr.range[i].min;
                    max = inst->u.sr.range[i].max;
                }

                if ((min == 0) || (max == 0)) {
                    ICLI_PRINTF("Invalid MEP instance number\n");
                    continue;
                }
                for (j=min-1; j<max; ++j) {
                    if ((rc = mep_mgmt_lm_conf_get(j, &lm_config)) != VTSS_RC_OK) {
                        mep_print_error(session_id, rc);
                        continue;
                    }
                    if (lm_config.enable) {
                        ICLI_PRINTF("%9u", j+1);
                        ICLI_PRINTF("%9u", lm_config.prio);
                        switch (lm_config.cast) {
                            case VTSS_MEP_MGMT_UNICAST:       ICLI_PRINTF("%9s", "Uni");     break;
                            case VTSS_MEP_MGMT_MULTICAST:     ICLI_PRINTF("%9s", "Multi");   break;
                        }
                        switch (lm_config.ended) {
                            case VTSS_MEP_MGMT_SINGEL_ENDED:      ICLI_PRINTF("%10s", "Single");   break;
                            case VTSS_MEP_MGMT_DUAL_ENDED:        ICLI_PRINTF("%10s", "Dual");   break;
                            default:                              ICLI_PRINTF("%10s", "Unknown");
                        }
                        switch (lm_config.period) {
                            case VTSS_MEP_MGMT_PERIOD_300S:       ICLI_PRINTF("%11s", "300s");   break;
                            case VTSS_MEP_MGMT_PERIOD_100S:       ICLI_PRINTF("%11s", "100s");   break;
                            case VTSS_MEP_MGMT_PERIOD_10S:        ICLI_PRINTF("%11s", "10s");    break;
                            case VTSS_MEP_MGMT_PERIOD_1S:         ICLI_PRINTF("%11s", "1s");     break;
                            case VTSS_MEP_MGMT_PERIOD_6M:         ICLI_PRINTF("%11s", "6m");     break;
                            case VTSS_MEP_MGMT_PERIOD_1M:         ICLI_PRINTF("%11s", "1m");     break;
                            case VTSS_MEP_MGMT_PERIOD_6H:         ICLI_PRINTF("%11s", "6h");     break;
                            default:                              ICLI_PRINTF("%11s", "Unknown");
                        }
                        ICLI_PRINTF("%8u", lm_config.flr_interval);
                        ICLI_PRINTF("\n");
                    }
                }
            }
            ICLI_PRINTF("\n");
        }
    }

    if (has_dm) {
        ICLI_PRINTF("\n");
        ICLI_PRINTF("MEP DM state is:\n");
        ICLI_PRINTF("\n");
        ICLI_PRINTF("RxTime : Rx Timeout\n");
        ICLI_PRINTF("RxErr :  Rx Error\n");
        ICLI_PRINTF("AvTot :  Average delay Total\n");
        ICLI_PRINTF("AvN :    Average delay last N\n");
        ICLI_PRINTF("Min :    Min Delay value\n");
        ICLI_PRINTF("Max :    Max Delay value\n");
        ICLI_PRINTF("AvVarT : Average delay Variation Total\n");
        ICLI_PRINTF("AvVarN : Average delay Variation last N\n");
        ICLI_PRINTF("MinVar : Min Delay Variation value\n");
        ICLI_PRINTF("MaxVar : Max Delay Variation value\n");
        ICLI_PRINTF("OF  :    Overflow. The number of statistics overflow.\n");

        ICLI_PRINTF("\n");
        ICLI_PRINTF("%12s", " ");
        ICLI_PRINTF("%4s", "Inst");
        ICLI_PRINTF("%9s", "Tx");
        ICLI_PRINTF("%9s", "Rx");
        ICLI_PRINTF("%9s", "RxTime");
        ICLI_PRINTF("%9s", "RxErr");
        ICLI_PRINTF("%10s", "AvTot");
        ICLI_PRINTF("%10s", "AvN");
        ICLI_PRINTF("%10s", "Min");
        ICLI_PRINTF("%9s", "Max");
        ICLI_PRINTF("%10s", "AvVarTot");
        ICLI_PRINTF("%9s", "AvVarN");
        ICLI_PRINTF("%10s", "MinVar");
        ICLI_PRINTF("%9s", "MaxVar");
        ICLI_PRINTF("%4s", "OF");
        ICLI_PRINTF("%5s", "Unit");
        ICLI_PRINTF("\n");

        if (inst == NULL) {
            list_cnt = 1;
        }
        else {
            list_cnt = inst->u.sr.cnt;
        }

        for (i=0; i<list_cnt; ++i) {
            if (inst == NULL) {
                min = 1;
                max = MEP_INSTANCE_MAX;
            }
            else {
                min = inst->u.sr.range[i].min;
                max = inst->u.sr.range[i].max;
            }

            if ((min == 0) || (max == 0)) {
                ICLI_PRINTF("Invalid MEP instance number\n");
                continue;
            }
            for (j=min-1; j<max; ++j) {
                if ((rc = mep_mgmt_dm_state_get(j, &dmr_state, &dm1_state_far_to_near, &dm1_state_near_to_far)) != VTSS_RC_OK) {
                    if (rc != MEP_RC_NOT_ENABLED) {
                        mep_print_error(session_id, rc);
                    }
                    continue;
                }
                ICLI_PRINTF("%-12s", "1-Way FtoN");
                ICLI_PRINTF("%4u", j+1);
                ICLI_PRINTF("%9u", dm1_state_far_to_near.tx_cnt);
                ICLI_PRINTF("%9u", dm1_state_far_to_near.rx_cnt);
                ICLI_PRINTF("%9u", dm1_state_far_to_near.rx_tout_cnt);
                ICLI_PRINTF("%9u", dm1_state_far_to_near.rx_err_cnt);
                ICLI_PRINTF("%10u", dm1_state_far_to_near.avg_delay);
                ICLI_PRINTF("%10u", dm1_state_far_to_near.avg_n_delay);
                ICLI_PRINTF("%10u", dm1_state_far_to_near.min_delay);
                ICLI_PRINTF("%9u", dm1_state_far_to_near.max_delay);
                ICLI_PRINTF("%10u", dm1_state_far_to_near.avg_delay_var);
                ICLI_PRINTF("%9u", dm1_state_far_to_near.avg_n_delay_var);
                ICLI_PRINTF("%10u", dm1_state_far_to_near.min_delay_var);
                ICLI_PRINTF("%9u", dm1_state_far_to_near.max_delay_var);
                ICLI_PRINTF("%4u", dm1_state_far_to_near.ovrflw_cnt);
                ICLI_PRINTF("%5s", dm1_state_far_to_near.tunit == VTSS_MEP_MGMT_US ? "us" : "ns");
                ICLI_PRINTF("\n");
                ICLI_PRINTF("%-12s", "1-Way NtoF");
                ICLI_PRINTF("%4u", j+1);
                ICLI_PRINTF("%9u", dm1_state_near_to_far.tx_cnt);
                ICLI_PRINTF("%9u", dm1_state_near_to_far.rx_cnt);
                ICLI_PRINTF("%9u", dm1_state_near_to_far.rx_tout_cnt);
                ICLI_PRINTF("%9u", dm1_state_near_to_far.rx_err_cnt);
                ICLI_PRINTF("%10u", dm1_state_near_to_far.avg_delay);
                ICLI_PRINTF("%10u", dm1_state_near_to_far.avg_n_delay);
                ICLI_PRINTF("%10u", dm1_state_near_to_far.min_delay);
                ICLI_PRINTF("%9u", dm1_state_near_to_far.max_delay);
                ICLI_PRINTF("%10u", dm1_state_near_to_far.avg_delay_var);
                ICLI_PRINTF("%9u", dm1_state_near_to_far.avg_n_delay_var);
                ICLI_PRINTF("%10u", dm1_state_near_to_far.min_delay_var);
                ICLI_PRINTF("%9u", dm1_state_near_to_far.max_delay_var);
                ICLI_PRINTF("%4u", dm1_state_near_to_far.ovrflw_cnt);
                ICLI_PRINTF("%5s", dm1_state_near_to_far.tunit == VTSS_MEP_MGMT_US ? "us" : "ns");
                ICLI_PRINTF("\n");
                ICLI_PRINTF("%-12s", "2-Way");
                ICLI_PRINTF("%4u", j+1);
                ICLI_PRINTF("%9u", dmr_state.tx_cnt);
                ICLI_PRINTF("%9u", dmr_state.rx_cnt);
                ICLI_PRINTF("%9u", dmr_state.rx_tout_cnt);
                ICLI_PRINTF("%9u", dmr_state.rx_err_cnt);
                ICLI_PRINTF("%10u", dmr_state.avg_delay);
                ICLI_PRINTF("%10u", dmr_state.avg_n_delay);
                ICLI_PRINTF("%10u", dmr_state.min_delay);
                ICLI_PRINTF("%9u", dmr_state.max_delay);
                ICLI_PRINTF("%10u", dmr_state.avg_delay_var);
                ICLI_PRINTF("%9u", dmr_state.avg_n_delay_var);
                ICLI_PRINTF("%10u", dmr_state.min_delay_var);
                ICLI_PRINTF("%9u", dmr_state.max_delay_var);
                ICLI_PRINTF("%4u", dmr_state.ovrflw_cnt);
                ICLI_PRINTF("%5s", dmr_state.tunit == VTSS_MEP_MGMT_US ? "us" : "ns");
                ICLI_PRINTF("\n");
            }
        }
        ICLI_PRINTF("\n");

        if (has_detail) {
            ICLI_PRINTF("\n");
            ICLI_PRINTF("MEP DM Configuration is:\n");
            ICLI_PRINTF("%9s", "Inst");
            ICLI_PRINTF("%9s", "Prio");
            ICLI_PRINTF("%9s", "Cast");
            ICLI_PRINTF("%8s", "Mep");
            ICLI_PRINTF("%10s", "Way");
            ICLI_PRINTF("%7s", "Proto");
            ICLI_PRINTF("%7s", "Calc");
            ICLI_PRINTF("%10s", "Interval");
            ICLI_PRINTF("%8s", "Last-n");
            ICLI_PRINTF("%7s", "Unit");
            ICLI_PRINTF("%9s", "Sync");
            ICLI_PRINTF("%10s", "Overflow");
            ICLI_PRINTF("\n");

            if (inst == NULL) {
                list_cnt = 1;
            }
            else {
                list_cnt = inst->u.sr.cnt;
            }

            for (i=0; i<list_cnt; ++i) {
                if (inst == NULL) {
                    min = 1;
                    max = MEP_INSTANCE_MAX;
                }
                else {
                    min = inst->u.sr.range[i].min;
                    max = inst->u.sr.range[i].max;
                }

                if ((min == 0) || (max == 0)) {
                    ICLI_PRINTF("Invalid MEP instance number\n");
                    continue;
                }
                for (j=min-1; j<max; ++j) {
                    if ((rc = mep_mgmt_dm_conf_get(j, &dm_config)) != VTSS_RC_OK) {
                        mep_print_error(session_id, rc);
                        continue;
                    }
                    if (dm_config.enable) {
                        ICLI_PRINTF("%9u", j+1);
                        ICLI_PRINTF("%9u", dm_config.prio);
                        switch (dm_config.cast) {
                            case VTSS_MEP_MGMT_UNICAST:     ICLI_PRINTF("%9s", "Uni");     break;
                            case VTSS_MEP_MGMT_MULTICAST:   ICLI_PRINTF("%9s", "Multi");   break;
                            default:                        ICLI_PRINTF("%9s", "ND");
                        }
                        ICLI_PRINTF("%8u", dm_config.mep);
                        switch (dm_config.ended) {
                            case VTSS_MEP_MGMT_DUAL_ENDED:     ICLI_PRINTF("%10s", "Dual");   break;
                            case VTSS_MEP_MGMT_SINGEL_ENDED:   ICLI_PRINTF("%10s", "Single"); break;
                            default:                           ICLI_PRINTF("%10s", "ND");
                        }
                        if (!dm_config.proprietary) {
                            ICLI_PRINTF("%7s", "Std");
                        } else {
                            ICLI_PRINTF("%7s", "Prop");
                        }
                        switch (dm_config.calcway) {
                            case VTSS_MEP_MGMT_RDTRP:       ICLI_PRINTF("%7s", "Rdtrp");   break;
                            case VTSS_MEP_MGMT_FLOW:        ICLI_PRINTF("%7s", "Flow");   break;
                            default:                        ICLI_PRINTF("%7s", "ND");
                        }
                        ICLI_PRINTF("%10u", dm_config.interval);
                        ICLI_PRINTF("%8u", dm_config.lastn);
                        switch (dm_config.tunit) {
                            case VTSS_MEP_MGMT_US:          ICLI_PRINTF("%7s", "us");   break;
                            case VTSS_MEP_MGMT_NS:          ICLI_PRINTF("%7s", "ns");   break;
                            default:                        ICLI_PRINTF("%7s", "ND");
                        }
                        ICLI_PRINTF("%9s", dm_config.syncronized ? "Enable" : "Disable");
                        switch (dm_config.overflow_act) {
                            case VTSS_MEP_MGMT_DISABLE:     ICLI_PRINTF("%10s", "keep");    break;
                            case VTSS_MEP_MGMT_CONTINUE:    ICLI_PRINTF("%10s", "reset");   break;
                            default:                        ICLI_PRINTF("%10s", "ND");
                        }
                        ICLI_PRINTF("\n");
                    }
                }
            }
            ICLI_PRINTF("\n");
        }
    }

    if (has_lt) {
        ICLI_PRINTF("MEP LT state is:\n");
        ICLI_PRINTF("%9s", "Inst");
        ICLI_PRINTF("%19s", "Transaction ID");
        ICLI_PRINTF("%8s", "Ttl");
        ICLI_PRINTF("%9s", "Mode");
        ICLI_PRINTF("%14s", "Direction");
        ICLI_PRINTF("%12s", "Forwarded");
        ICLI_PRINTF("%10s", "relay");
        ICLI_PRINTF("%22s", "Last MAC");
        ICLI_PRINTF("%22s", "Next MAC");
        ICLI_PRINTF("\n");

        if (inst == NULL) {
            list_cnt = 1;
        }
        else {
            list_cnt = inst->u.sr.cnt;
        }

        for (i=0; i<list_cnt; ++i) {
            if (inst == NULL) {
                min = 1;
                max = MEP_INSTANCE_MAX;
            }
            else {
                min = inst->u.sr.range[i].min;
                max = inst->u.sr.range[i].max;
            }

            if ((min == 0) || (max == 0)) {
                ICLI_PRINTF("Invalid MEP instance number\n");
                continue;
            }
            for (j=min-1; j<max; ++j) {
                if ((rc = mep_mgmt_lt_state_get(j, &lt_state)) != VTSS_RC_OK) {
                    if (rc != MEP_RC_NOT_ENABLED) {
                        mep_print_error(session_id, rc);
                    }
                    continue;
                }
                if (lt_state.transaction_cnt != 0) {
                    ICLI_PRINTF("%9u", j+1);
                    for (t=0; t<lt_state.transaction_cnt; ++t) {
                        if (t != 0) {
                            ICLI_PRINTF("%9s", " ");
                        }
                        ICLI_PRINTF("%19u", lt_state.transaction[t].transaction_id);

                        for (k=0; k<lt_state.transaction[t].reply_cnt; ++k) {
                            if (k != 0) {
                                ICLI_PRINTF("%28s"," ");
                            }
                            ICLI_PRINTF("%8u", lt_state.transaction[t].reply[k].ttl);
                            switch (lt_state.transaction[t].reply[k].mode) {
                                case VTSS_MEP_MGMT_MEP:  ICLI_PRINTF("%9s", "Mep");     break;
                                case VTSS_MEP_MGMT_MIP:  ICLI_PRINTF("%9s", "Mip");     break;
                            }
                            switch (lt_state.transaction[t].reply[k].direction) {
                                case VTSS_MEP_MGMT_DOWN:   ICLI_PRINTF("%14s", "Down");     break;
                                case VTSS_MEP_MGMT_UP:     ICLI_PRINTF("%14s", "Up");      break;
                            }
                            ICLI_PRINTF("%12s", lt_state.transaction[t].reply[k].forwarded ? "Yes" : "No");
                            switch (lt_state.transaction[t].reply[k].relay_action) {
                                case VTSS_MEP_MGMT_RELAY_UNKNOWN:   ICLI_PRINTF("%10s", "Unknown");     break;
                                case VTSS_MEP_MGMT_RELAY_HIT:       ICLI_PRINTF("%10s", "MAC");         break;
                                case VTSS_MEP_MGMT_RELAY_FDB:       ICLI_PRINTF("%10s", "FDB");         break;
                                case VTSS_MEP_MGMT_RELAY_MFDB:      ICLI_PRINTF("%10s", "CCM FDB");     break;
                            }
                            ICLI_PRINTF("%5s%02X-%02X-%02X-%02X-%02X-%02X", " ", lt_state.transaction[t].reply[k].last_egress_mac[0], lt_state.transaction[t].reply[k].last_egress_mac[1], lt_state.transaction[t].reply[k].last_egress_mac[2], lt_state.transaction[t].reply[k].last_egress_mac[3], lt_state.transaction[t].reply[k].last_egress_mac[4], lt_state.transaction[t].reply[k].last_egress_mac[5]);
                            ICLI_PRINTF("%5s%02X-%02X-%02X-%02X-%02X-%02X", " ", lt_state.transaction[t].reply[k].next_egress_mac[0], lt_state.transaction[t].reply[k].next_egress_mac[1], lt_state.transaction[t].reply[k].next_egress_mac[2], lt_state.transaction[t].reply[k].next_egress_mac[3], lt_state.transaction[t].reply[k].next_egress_mac[4], lt_state.transaction[t].reply[k].next_egress_mac[5]);
                            ICLI_PRINTF("\n");
                        }
                        if (lt_state.transaction[t].reply_cnt == 0) {
                            ICLI_PRINTF("\n");
                        }
                    }
                }
            }
        }
        ICLI_PRINTF("\n");

        if (has_detail) {
            ICLI_PRINTF("\n");
            ICLI_PRINTF("MEP LT Configuration is:\n");
            ICLI_PRINTF("%9s", "Inst");
            ICLI_PRINTF("%9s", "Prio");
            ICLI_PRINTF("%8s", "Mep");
            ICLI_PRINTF("%22s", "MAC");
            ICLI_PRINTF("%8s", "Ttl");
            ICLI_PRINTF("\n");

            if (inst == NULL) {
                list_cnt = 1;
            }
            else {
                list_cnt = inst->u.sr.cnt;
            }

            for (i=0; i<list_cnt; ++i) {
                if (inst == NULL) {
                    min = 1;
                    max = MEP_INSTANCE_MAX;
                }
                else {
                    min = inst->u.sr.range[i].min;
                    max = inst->u.sr.range[i].max;
                }

                if ((min == 0) || (max == 0)) {
                    ICLI_PRINTF("Invalid MEP instance number\n");
                    continue;
                }
                for (j=min-1; j<max; ++j) {
                    if ((rc = mep_mgmt_lt_conf_get(j, &lt_config)) != VTSS_RC_OK) {
                        mep_print_error(session_id, rc);
                        continue;
                    }
                    if (lt_config.enable) {
                        ICLI_PRINTF("%9u", j+1);
                        ICLI_PRINTF("%9u", lt_config.prio);
                        ICLI_PRINTF("%8u", lt_config.mep);
                        ICLI_PRINTF("     %02X-%02X-%02X-%02X-%02X-%02X", lt_config.mac[0], lt_config.mac[1], lt_config.mac[2], lt_config.mac[3], lt_config.mac[4], lt_config.mac[5]);
                        ICLI_PRINTF("%8u", lt_config.ttl);
                        ICLI_PRINTF("\n");
                    }
                }
            }
            ICLI_PRINTF("\n");
        }
    }

    if (has_lb) {
        ICLI_PRINTF("MEP LB state is:\n");
        ICLI_PRINTF("%9s", "Inst");
        ICLI_PRINTF("%19s", "Transaction ID");
        ICLI_PRINTF("%11s", "TX LBM");
        ICLI_PRINTF("%22s", "MAC");
        ICLI_PRINTF("%13s", "Received");
        ICLI_PRINTF("%17s", "Out Of Order");
        ICLI_PRINTF("\n");

        if (inst == NULL) {
            list_cnt = 1;
        }
        else {
            list_cnt = inst->u.sr.cnt;
        }

        for (i=0; i<list_cnt; ++i) {
            if (inst == NULL) {
                min = 1;
                max = MEP_INSTANCE_MAX;
            }
            else {
                min = inst->u.sr.range[i].min;
                max = inst->u.sr.range[i].max;
            }

            if ((min == 0) || (max == 0)) {
                ICLI_PRINTF("Invalid MEP instance number\n");
                continue;
            }
            for (j=min-1; j<max; ++j) {
                if ((rc = mep_mgmt_lb_state_get(j, &lb_state)) != VTSS_RC_OK) {
                    if (rc != MEP_RC_NOT_ENABLED) {
                        mep_print_error(session_id, rc);
                    }
                    continue;
                }
                if (lb_state.reply_cnt != 0) {
                    ICLI_PRINTF("%9u", j+1);
                    ICLI_PRINTF("%19u", lb_state.transaction_id);
                    ICLI_PRINTF("%11llu", lb_state.lbm_transmitted);
                    for (k=0; k<lb_state.reply_cnt; ++k) {
                        if (k != 0) {
                            ICLI_PRINTF("%39s", " ");
                        }
                        ICLI_PRINTF("%5s%02X-%02X-%02X-%02X-%02X-%02X", " ", lb_state.reply[k].mac[0], lb_state.reply[k].mac[1], lb_state.reply[k].mac[2], lb_state.reply[k].mac[3], lb_state.reply[k].mac[4], lb_state.reply[k].mac[5]);
                        ICLI_PRINTF("%12llu", lb_state.reply[k].lbr_received);
                        ICLI_PRINTF("%14llu\n", lb_state.reply[k].out_of_order);
                    }
                }
            }
        }
        ICLI_PRINTF("\n");

        if (has_detail) {
            ICLI_PRINTF("\n");
            ICLI_PRINTF("MEP LB Configuration is:\n");
            ICLI_PRINTF("%9s", "Inst");
            ICLI_PRINTF("%8s", "Dei");
            ICLI_PRINTF("%9s", "Prio");
            ICLI_PRINTF("%9s", "Cast");
            ICLI_PRINTF("%8s", "Mep");
            ICLI_PRINTF("%22s", "MAC");
            ICLI_PRINTF("%11s", "ToSend");
            ICLI_PRINTF("%9s", "Size");
            ICLI_PRINTF("%13s", "Interval");
            ICLI_PRINTF("\n");

            if (inst == NULL) {
                list_cnt = 1;
            }
            else {
                list_cnt = inst->u.sr.cnt;
            }

            for (i=0; i<list_cnt; ++i) {
                if (inst == NULL) {
                    min = 1;
                    max = MEP_INSTANCE_MAX;
                }
                else {
                    min = inst->u.sr.range[i].min;
                    max = inst->u.sr.range[i].max;
                }

                if ((min == 0) || (max == 0)) {
                    ICLI_PRINTF("Invalid MEP instance number\n");
                    continue;
                }
                for (j=min-1; j<max; ++j) {
                    if ((rc = mep_mgmt_lb_conf_get(j, &lb_config)) != VTSS_RC_OK) {
                        mep_print_error(session_id, rc);
                        continue;
                    }
                    if (lb_config.enable) {
                        ICLI_PRINTF("%9u", j+1);
                        ICLI_PRINTF("%8u", lb_config.dei);
                        ICLI_PRINTF("%9u", lb_config.prio);
                        switch (lb_config.cast) {
                            case VTSS_MEP_MGMT_UNICAST:       ICLI_PRINTF("%9s", "Uni");     break;
                            case VTSS_MEP_MGMT_MULTICAST:     ICLI_PRINTF("%9s", "Multi");   break;
                        }
                        ICLI_PRINTF("%8u", lb_config.mep);
                        ICLI_PRINTF("     %02X-%02X-%02X-%02X-%02X-%02X", lb_config.mac[0], lb_config.mac[1], lb_config.mac[2], lb_config.mac[3], lb_config.mac[4], lb_config.mac[5]);
                        ICLI_PRINTF("%11u", lb_config.to_send);
                        ICLI_PRINTF("%9u", lb_config.size);
                        ICLI_PRINTF("%13u", lb_config.interval);
                        ICLI_PRINTF("\n");
                    }
                }
            }
            ICLI_PRINTF("\n");
        }
    }

    if (has_tst) {
        ICLI_PRINTF("MEP TST state is:\n");
        ICLI_PRINTF("%9s", "Inst");
        ICLI_PRINTF("%19s", "TX frame count");
        ICLI_PRINTF("%19s", "RX frame count");
        ICLI_PRINTF("%12s", "RX rate");
        ICLI_PRINTF("%14s", "Test time");
        ICLI_PRINTF("\n");

        if (inst == NULL) {
            list_cnt = 1;
        }
        else {
            list_cnt = inst->u.sr.cnt;
        }

        for (i=0; i<list_cnt; ++i) {
            if (inst == NULL) {
                min = 1;
                max = MEP_INSTANCE_MAX;
            }
            else {
                min = inst->u.sr.range[i].min;
                max = inst->u.sr.range[i].max;
            }

            if ((min == 0) || (max == 0)) {
                ICLI_PRINTF("Invalid MEP instance number\n");
                continue;
            }
            for (j=min-1; j<max; ++j) {
                if ((rc = mep_mgmt_tst_state_get(j, &tst_state)) != VTSS_RC_OK) {
                    if (rc != MEP_RC_NOT_ENABLED) {
                        mep_print_error(session_id, rc);
                    }
                    continue;
                }
                ICLI_PRINTF("%9u", j+1);
                ICLI_PRINTF("%19llu", tst_state.tx_counter);
                ICLI_PRINTF("%19llu", tst_state.rx_counter);
                ICLI_PRINTF("%12u", tst_state.rx_rate);
                ICLI_PRINTF("%14u\n", tst_state.time);
            }
        }
        ICLI_PRINTF("\n");

        if (has_detail) {
            ICLI_PRINTF("\n");
            ICLI_PRINTF("MEP TST Configuration is:\n");
            ICLI_PRINTF("%9s", "Inst");
            ICLI_PRINTF("%8s", "Dei");
            ICLI_PRINTF("%9s", "Prio");
            ICLI_PRINTF("%8s", "Mep");
            ICLI_PRINTF("%9s", "rate");
            ICLI_PRINTF("%9s", "Size");
            ICLI_PRINTF("%13s", "Pattern");
            ICLI_PRINTF("%11s", "Sequence");
            ICLI_PRINTF("%9s", "tx");
            ICLI_PRINTF("%9s", "rx");
            ICLI_PRINTF("\n");

            if (inst == NULL) {
                list_cnt = 1;
            }
            else {
                list_cnt = inst->u.sr.cnt;
            }

            for (i=0; i<list_cnt; ++i) {
                if (inst == NULL) {
                    min = 1;
                    max = MEP_INSTANCE_MAX;
                }
                else {
                    min = inst->u.sr.range[i].min;
                    max = inst->u.sr.range[i].max;
                }

                if ((min == 0) || (max == 0)) {
                    ICLI_PRINTF("Invalid MEP instance number\n");
                    continue;
                }
                for (j=min-1; j<max; ++j) {
                    if ((rc = mep_mgmt_tst_conf_get(j, &tst_config)) != VTSS_RC_OK) {
                        mep_print_error(session_id, rc);
                        continue;
                    }
                    if ((tst_config.enable) || (tst_config.enable_rx)) {
                        ICLI_PRINTF("%9u", j+1);
                        ICLI_PRINTF("%8u", tst_config.dei);
                        ICLI_PRINTF("%9u", tst_config.prio);
                        ICLI_PRINTF("%8u", tst_config.mep);
                        ICLI_PRINTF("%9u", tst_config.rate/1000);
                        ICLI_PRINTF("%9u", tst_config.size);
                        switch (tst_config.pattern) {
                            case VTSS_MEP_MGMT_PATTERN_ALL_ZERO:    ICLI_PRINTF("%13s", "All zero");     break;
                            case VTSS_MEP_MGMT_PATTERN_ALL_ONE:     ICLI_PRINTF("%13s", "All one");   break;
                            case VTSS_MEP_MGMT_PATTERN_0XAA:        ICLI_PRINTF("%13s", "0xAA");   break;
                        }
                        ICLI_PRINTF("%11s", tst_config.sequence ? "Enable" : "Disable");
                        ICLI_PRINTF("%9s", tst_config.enable ? "Enable" : "Disable");
                        ICLI_PRINTF("%9s", tst_config.enable_rx ? "Enable" : "Disable");
                        ICLI_PRINTF("\n");
                    }
                }
            }
            ICLI_PRINTF("\n");
        }
    }

    if (has_aps && has_detail) {
        ICLI_PRINTF("\n");
        ICLI_PRINTF("MEP APS Configuration is:\n");
        ICLI_PRINTF("%9s", "Inst");
        ICLI_PRINTF("%9s", "Prio");
        ICLI_PRINTF("%9s", "Cast");
        ICLI_PRINTF("%9s", "Type");
        ICLI_PRINTF("%9s", "Octet");
        ICLI_PRINTF("\n");

        if (inst == NULL) {
            list_cnt = 1;
        }
        else {
            list_cnt = inst->u.sr.cnt;
        }

        for (i=0; i<list_cnt; ++i) {
            if (inst == NULL) {
                min = 1;
                max = MEP_INSTANCE_MAX;
            }
            else {
                min = inst->u.sr.range[i].min;
                max = inst->u.sr.range[i].max;
            }

            if ((min == 0) || (max == 0)) {
                ICLI_PRINTF("Invalid MEP instance number\n");
                continue;
            }
            for (j=min-1; j<max; ++j) {
                if ((rc = mep_mgmt_aps_conf_get(j, &aps_config)) != VTSS_RC_OK) {
                    mep_print_error(session_id, rc);
                    continue;
                }
                if (aps_config.enable) {
                    ICLI_PRINTF("%9u", j+1);
                    ICLI_PRINTF("%9u", aps_config.prio);
                    switch (aps_config.cast) {
                        case VTSS_MEP_MGMT_UNICAST:       ICLI_PRINTF("%9s", "Uni");     break;
                        case VTSS_MEP_MGMT_MULTICAST:     ICLI_PRINTF("%9s", "Multi");   break;
                    }
                    switch (aps_config.type) {
                        case VTSS_MEP_MGMT_INV_APS:   ICLI_PRINTF("%9s", "none");   break;
                        case VTSS_MEP_MGMT_L_APS:     ICLI_PRINTF("%9s", "laps");   break;
                        case VTSS_MEP_MGMT_R_APS:     ICLI_PRINTF("%9s", "raps");   break;
                    }
                    ICLI_PRINTF("%9X", aps_config.raps_octet);
                    ICLI_PRINTF("\n");
                }
            }
        }
        ICLI_PRINTF("\n");
    }

    if (has_client && has_detail) {
        ICLI_PRINTF("\n");
        ICLI_PRINTF("MEP CLIENT Configuration is:\n");
        ICLI_PRINTF("%9s", "Inst");
        ICLI_PRINTF("%11s", "Domain");
        ICLI_PRINTF("%10s", "Flows");
        ICLI_PRINTF("%13s", "AIS Prio");
        ICLI_PRINTF("%13s", "LCK Prio");
        ICLI_PRINTF("%10s", "Level");
        ICLI_PRINTF("\n");

        if (inst == NULL) {
            list_cnt = 1;
        }
        else {
            list_cnt = inst->u.sr.cnt;
        }
        for (i=0; i<list_cnt; ++i) {
            if (inst == NULL) {
                min = 1;
                max = MEP_INSTANCE_MAX;
            }
            else {
                min = inst->u.sr.range[i].min;
                max = inst->u.sr.range[i].max;
            }

            if ((min == 0) || (max == 0)) {
                ICLI_PRINTF("Invalid MEP instance number\n");
                continue;
            }
            for (j=min-1; j<max; ++j) {
                if (((rc = mep_mgmt_conf_get(j, mac, &eps_count, eps_inst, &config)) != VTSS_RC_OK) && (rc != MEP_RC_VOLATILE)) {
                    mep_print_error(session_id, rc);
                    continue;
                }
                if (config.mode == VTSS_MEP_MGMT_MIP) {
                    continue;
                }
                if ((rc = mep_mgmt_client_conf_get(j, &client_config)) != VTSS_RC_OK) {
                    mep_print_error(session_id, rc);
                    continue;
                }
                if (config.enable) {
                    ICLI_PRINTF("%9u", j+1);
                    switch (client_config.domain) {
                        case VTSS_MEP_MGMT_PORT:       ICLI_PRINTF("%11s", "Invalid");  break;
//                        case VTSS_MEP_MGMT_ESP:        ICLI_PRINTF("%11s", "ESP");   break;
                        case VTSS_MEP_MGMT_EVC:        ICLI_PRINTF("%11s", "Evc");   break;
                        case VTSS_MEP_MGMT_VLAN:       ICLI_PRINTF("%11s", "Vlan");   break;
//                        case VTSS_MEP_MGMT_MPLS:       ICLI_PRINTF("%11s", "Mpls");  break;
                    }
                    for (k=0; k<client_config.flow_count; ++k) {
                        if (k != 0) {
                            ICLI_PRINTF("%20s"," ");
                        }
                        ICLI_PRINTF("%10u", client_config.flows[k]+1);
                        if (client_config.ais_prio[k] == VTSS_MEP_CLIENT_PRIO_HIGHEST) {
                            ICLI_PRINTF("%13s", "Highest");
                        } else {
                            ICLI_PRINTF("%13u", client_config.ais_prio[k]);
                        }
                        if (client_config.lck_prio[k] == VTSS_MEP_CLIENT_PRIO_HIGHEST) {
                            ICLI_PRINTF("%13s", "Highest");
                        } else {
                            ICLI_PRINTF("%13u", client_config.lck_prio[k]);
                        }
                        ICLI_PRINTF("%10u", client_config.level[k]);
                        ICLI_PRINTF("\n");
                    }
                    if (client_config.flow_count == 0) {
                        ICLI_PRINTF("\n");
                    }
                }
            }
        }
        ICLI_PRINTF("\n");
    }

    if (has_ais && has_detail) {
        ICLI_PRINTF("\n");
        ICLI_PRINTF("MEP AIS Configuration is:\n");
        ICLI_PRINTF("%9s", "Inst");
        ICLI_PRINTF("%11s", "Period");
        ICLI_PRINTF("%15s","Protection");
        ICLI_PRINTF("\n");

        if (inst == NULL) {
            list_cnt = 1;
        }
        else {
            list_cnt = inst->u.sr.cnt;
        }
        for (i=0; i<list_cnt; ++i) {
            if (inst == NULL) {
                min = 1;
                max = MEP_INSTANCE_MAX;
            }
            else {
                min = inst->u.sr.range[i].min;
                max = inst->u.sr.range[i].max;
            }

            if ((min == 0) || (max == 0)) {
                ICLI_PRINTF("Invalid MEP instance number\n");
                continue;
            }
            for (j=min-1; j<max; ++j) {
                if ((rc = mep_mgmt_ais_conf_get(j, &ais_config)) != VTSS_RC_OK) {
                    mep_print_error(session_id, rc);
                    continue;
                }
                if (ais_config.enable) {
                    ICLI_PRINTF("%9u", j+1);
                    switch (ais_config.period) {
                        case VTSS_MEP_MGMT_PERIOD_1S:         ICLI_PRINTF("%11s", "1s");     break;
                        case VTSS_MEP_MGMT_PERIOD_1M:         ICLI_PRINTF("%11s", "1m");     break;
                        default:                              ICLI_PRINTF("%11s", "Unknown");
                    }
                    if (ais_config.protection) {
                        ICLI_PRINTF("%15s", "Enabled");
                    }
                    else {
                        ICLI_PRINTF("%15s", "Disabled");
                    }
                    ICLI_PRINTF("\n");
                }
            }
        }
        ICLI_PRINTF("\n");
    }

    if (has_lck && has_detail) {
        ICLI_PRINTF("\n");
        ICLI_PRINTF("MEP LCK Configuration is:\n");
        ICLI_PRINTF("%9s", "Inst");
        ICLI_PRINTF("%11s", "Period");
        ICLI_PRINTF("\n");

        if (inst == NULL) {
            list_cnt = 1;
        }
        else {
            list_cnt = inst->u.sr.cnt;
        }
        for (i=0; i<list_cnt; ++i) {
            if (inst == NULL) {
                min = 1;
                max = MEP_INSTANCE_MAX;
            }
            else {
                min = inst->u.sr.range[i].min;
                max = inst->u.sr.range[i].max;
            }

            if ((min == 0) || (max == 0)) {
                ICLI_PRINTF("Invalid MEP instance number\n");
                continue;
            }
            for (j=min-1; j<max; ++j) {
                if ((rc = mep_mgmt_lck_conf_get(j, &lck_config)) != VTSS_RC_OK) {
                    mep_print_error(session_id, rc);
                    continue;
                }
                if (lck_config.enable) {
                    ICLI_PRINTF("%9u", j+1);
                    switch (lck_config.period) {
                        case VTSS_MEP_MGMT_PERIOD_1S:         ICLI_PRINTF("%11s", "1s");     break;
                        case VTSS_MEP_MGMT_PERIOD_1M:         ICLI_PRINTF("%11s", "1m");     break;
                        default:                              ICLI_PRINTF("%11s", "Unknown");
                    }
                    ICLI_PRINTF("\n");
                }
            }
        }
        ICLI_PRINTF("\n");
    }
}


void mep_clear_mep(i32 session_id, u32 inst,
                   BOOL has_lm, BOOL has_dm, BOOL has_tst)
{
    vtss_rc rc = VTSS_RC_OK;

    if (inst == 0) {
        ICLI_PRINTF("Invalid MEP instance number\n");
        return;
    }

    if (has_lm) {
        rc = mep_mgmt_lm_state_clear_set(inst - 1);
    }

    if (has_dm) {
        rc = mep_mgmt_dm_state_clear_set(inst - 1);
    }

    if (has_tst) {
        rc = mep_mgmt_tst_state_clear_set(inst - 1);
    }

    if (rc != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
    }
}

void mep_mep(i32 session_id, u32 inst,
             BOOL has_mip, BOOL has_up, BOOL has_down, BOOL has_port, BOOL has_evc, BOOL has_vlan, BOOL has_vid, u32 vid, u32 flow, u32 level, icli_switch_port_range_t port)
{
    u32                       eps_count;
    u16                       eps_inst[MEP_EPS_MAX];
    vtss_mep_mgmt_conf_t      config;
    vtss_mep_mgmt_def_conf_t  def_conf;
    u8                        mac[VTSS_MEP_MAC_LENGTH];
    vtss_rc                   rc;

    mep_mgmt_def_conf_get(&def_conf);

    if (inst == 0) {
        ICLI_PRINTF("Invalid MEP instance number\n");
        return;
    }
    if ((rc = mep_mgmt_conf_get(inst-1, mac, &eps_count, eps_inst, &config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if (config.enable) {
        ICLI_PRINTF("MEP instance is already created - must be deleted first\n");
        return;
    }

    /* mep <inst:uint> [mip] {up|down} domain {port|evc|vlan} [vid <vid:vlan_id>] flow <flow:uint> level <level:0-7> interface <port:port_type_id> */
    config = def_conf.config; /* Initialize config */
    config.enable = TRUE;
    if (has_mip) {
        config.mode = has_mip;
    }
    if (has_up || has_down) {
        config.direction = has_up ? VTSS_MEP_MGMT_UP :
                           has_down ? VTSS_MEP_MGMT_DOWN : VTSS_MEP_MGMT_UP;
    }
    if (has_port || has_evc || has_vlan) {
        config.domain = has_port ? VTSS_MEP_MGMT_PORT :
                        has_evc ? VTSS_MEP_MGMT_EVC :
                        has_vlan ? VTSS_MEP_MGMT_VLAN : VTSS_MEP_MGMT_PORT;
    }

    config.level = level;
    config.port = port.begin_iport;

    if (has_vid) { /* VID can be given to be used for Port Up-MEP or EVC Customer MIP */
        config.vid = vid;
    }
    if (has_port) { /* In Port domain the flow is the port number */
        config.flow = config.port;
    }
    if (has_evc) { /* In EVC domain the flow is the EVC instance number - start in '0' */
        config.flow = flow-1;
    }
    if (has_vlan) { /* In VLAN domain the flow is the VLAN-ID */
        config.flow = flow;
    }
    if ((rc = mep_mgmt_conf_set(inst-1, &config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
    }
}

void mep_no_mep(i32 session_id, u32 inst)
{
    u32                       eps_count;
    u16                       eps_inst[MEP_EPS_MAX];
    vtss_mep_mgmt_conf_t      config;
    u8                        mac[VTSS_MEP_MAC_LENGTH];
    vtss_rc                   rc;

    if (inst == 0) {
        ICLI_PRINTF("Invalid MEP instance number\n");
        return;
    }
    if ((rc = mep_mgmt_conf_get(inst-1, mac, &eps_count, eps_inst, &config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if (config.enable) {
    /* no mep <inst:uint> */
        config.enable = FALSE;

        if ((rc = mep_mgmt_conf_set(inst-1, &config)) != VTSS_RC_OK) {
            mep_print_error(session_id, rc);
        }
    }
}

void mep_mep_meg_id(i32 session_id, u32 inst,
                    char *megid, BOOL has_itu, BOOL has_itu_cc, BOOL has_ieee, BOOL has_name, char *name)
{
    u32                       eps_count, meg_size;
    u16                       eps_inst[MEP_EPS_MAX];
    vtss_mep_mgmt_conf_t      config;
    vtss_mep_mgmt_def_conf_t  def_conf;
    u8                        mac[VTSS_MEP_MAC_LENGTH];
    vtss_rc                   rc;

    mep_mgmt_def_conf_get(&def_conf);

    if (inst == 0) {
        ICLI_PRINTF("Invalid MEP instance number\n");
        return;
    }
    if ((rc = mep_mgmt_conf_get(inst-1, mac, &eps_count, eps_inst, &config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if (config.enable && (megid != NULL)) {
        /* mep <inst:uint> meg-id <megid:word> {itu | itu-cc | {ieee [name <name:word>]}} */
        memcpy(config.name, def_conf.config.name, sizeof(config.name)); /* Initialize Maintenance Domain Name to default */
        memset(config.meg, 0, sizeof(config.meg));  /* Initialize MEGID */
        meg_size = strlen(megid);
        memcpy(config.meg, megid, (meg_size >= sizeof(config.meg)) ? sizeof(config.meg) : meg_size);    /* Copy the MEGID */
        config.meg[VTSS_MEP_MEG_CODE_LENGTH-1] = '\0';  /* Assure 'NULL' termination */
        config.name[VTSS_MEP_MEG_CODE_LENGTH-1] = '\0';
        config.format = has_itu ? VTSS_MEP_MGMT_ITU_ICC :
                        has_itu_cc ? VTSS_MEP_MGMT_ITU_CC_ICC :
                        has_ieee ? VTSS_MEP_MGMT_IEEE_STR : VTSS_MEP_MGMT_ITU_ICC;
        if (has_name && (name != NULL)) {
            memcpy(config.name, name, sizeof(config.name));
            config.name[VTSS_MEP_MEG_CODE_LENGTH-1] = '\0';
        }
        if ((rc = mep_mgmt_conf_set(inst-1, &config)) != VTSS_RC_OK) {
            mep_print_error(session_id, rc);
        }
    }
    else {
        ICLI_PRINTF("This MEP is not enabled\n");
    }
}

void mep_mep_mep_id(i32 session_id, u32 inst,
                    u32 mepid)
{
    u32                       eps_count;
    u16                       eps_inst[MEP_EPS_MAX];
    vtss_mep_mgmt_conf_t      config;
    u8                        mac[VTSS_MEP_MAC_LENGTH];
    vtss_rc                   rc;

    if (inst == 0) {
        ICLI_PRINTF("Invalid MEP instance number\n");
        return;
    }
    if ((rc = mep_mgmt_conf_get(inst-1, mac, &eps_count, eps_inst, &config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if (config.enable) {
        /* mep <inst:uint> mep-id <mepid:uint> */
        config.mep = mepid;

        if ((rc = mep_mgmt_conf_set(inst-1, &config)) != VTSS_RC_OK) {
            mep_print_error(session_id, rc);
        }
    }
    else {
        ICLI_PRINTF("This MEP is not enabled\n");
    }
}

void mep_mep_pm(i32 session_id, u32 inst)
{
    u32                       eps_count;
    u16                       eps_inst[MEP_EPS_MAX];
    vtss_mep_mgmt_conf_t      config;
    u8                        mac[VTSS_MEP_MAC_LENGTH];
    vtss_mep_mgmt_pm_conf_t   pm_config;
    vtss_rc                   rc;

    if (inst == 0) {
        ICLI_PRINTF("Invalid MEP instance number\n");
        return;
    }
    if ((rc = mep_mgmt_conf_get(inst-1, mac, &eps_count, eps_inst, &config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if (config.enable) {
        /* mep <inst:uint> performance-monitoring */
        pm_config.enable = TRUE;

        if ((rc = mep_mgmt_pm_conf_set(inst-1, &pm_config)) != VTSS_RC_OK) {
            mep_print_error(session_id, rc);
        }
    }
    else {
        ICLI_PRINTF("This MEP is not enabled\n");
    }
}

void mep_no_mep_pm(i32 session_id, u32 inst)
{
    u32                       eps_count;
    u16                       eps_inst[MEP_EPS_MAX];
    vtss_mep_mgmt_conf_t      config;
    u8                        mac[VTSS_MEP_MAC_LENGTH];
    vtss_mep_mgmt_pm_conf_t   pm_config;
    vtss_rc                   rc;

    if (inst == 0) {
        ICLI_PRINTF("Invalid MEP instance number\n");
        return;
    }
    if ((rc = mep_mgmt_conf_get(inst-1, mac, &eps_count, eps_inst, &config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if (config.enable) {
        /* no mep <inst:uint> performance-monitoring */
        pm_config.enable = FALSE;

        if ((rc = mep_mgmt_pm_conf_set(inst-1, &pm_config)) != VTSS_RC_OK) {
            mep_print_error(session_id, rc);
        }
    }
    else {
        ICLI_PRINTF("This MEP is not enabled\n");
    }
}

void mep_mep_level(i32 session_id, u32 inst,
                   u32 level)
{
    u32                       eps_count;
    u16                       eps_inst[MEP_EPS_MAX];
    vtss_mep_mgmt_conf_t      config;
    u8                        mac[VTSS_MEP_MAC_LENGTH];
    vtss_rc                   rc;

    if (inst == 0) {
        ICLI_PRINTF("Invalid MEP instance number\n");
        return;
    }
    if ((rc = mep_mgmt_conf_get(inst-1, mac, &eps_count, eps_inst, &config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if (config.enable) {
        /* mep <inst:uint> level <level:0-7> */
        config.level = level;

        if ((rc = mep_mgmt_conf_set(inst-1, &config)) != VTSS_RC_OK) {
            mep_print_error(session_id, rc);
        }
    }
    else {
        ICLI_PRINTF("This MEP is not enabled\n");
    }
}

void mep_mep_vid(i32 session_id, u32 inst,
                 u32 vid)
{
    u32                       eps_count;
    u16                       eps_inst[MEP_EPS_MAX];
    vtss_mep_mgmt_conf_t      config;
    u8                        mac[VTSS_MEP_MAC_LENGTH];
    vtss_rc                   rc;

    if (inst == 0) {
        ICLI_PRINTF("Invalid MEP instance number\n");
        return;
    }
    if ((rc = mep_mgmt_conf_get(inst-1, mac, &eps_count, eps_inst, &config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if (config.enable) {
        /* mep <inst:uint> vid <vid:vlan_id> */
        config.vid = vid;

        if ((rc = mep_mgmt_conf_set(inst-1, &config)) != VTSS_RC_OK) {
            mep_print_error(session_id, rc);
        }
    }
    else {
        ICLI_PRINTF("This MEP is not enabled\n");
    }
}

void mep_no_mep_vid(i32 session_id, u32 inst)
{
    u32                       eps_count;
    u16                       eps_inst[MEP_EPS_MAX];
    vtss_mep_mgmt_conf_t      config;
    u8                        mac[VTSS_MEP_MAC_LENGTH];
    vtss_rc                   rc;

    if (inst == 0) {
        ICLI_PRINTF("Invalid MEP instance number\n");
        return;
    }
    if ((rc = mep_mgmt_conf_get(inst-1, mac, &eps_count, eps_inst, &config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if (config.enable) {
        /* no mep <inst:uint> vid */
        config.vid = 0;

        if ((rc = mep_mgmt_conf_set(inst-1, &config)) != VTSS_RC_OK) {
            mep_print_error(session_id, rc);
        }
    }
    else {
        ICLI_PRINTF("This MEP is not enabled\n");
    }
}

void mep_mep_voe(i32 session_id, u32 inst)
{
    u32                       eps_count;
    u16                       eps_inst[MEP_EPS_MAX];
    vtss_mep_mgmt_conf_t      config;
    u8                        mac[VTSS_MEP_MAC_LENGTH];
    vtss_rc                   rc;

    if (inst == 0) {
        ICLI_PRINTF("Invalid MEP instance number\n");
        return;
    }
    if ((rc = mep_mgmt_conf_get(inst-1, mac, &eps_count, eps_inst, &config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if (config.enable) {
        /* mep <inst:uint> voe */
        config.voe = TRUE;

        if ((rc = mep_mgmt_conf_set(inst-1, &config)) != VTSS_RC_OK) {
            mep_print_error(session_id, rc);
        }
    }
    else {
        ICLI_PRINTF("This MEP is not enabled\n");
    }
}

void mep_no_mep_voe(i32 session_id, u32 inst)
{
    u32                       eps_count;
    u16                       eps_inst[MEP_EPS_MAX];
    vtss_mep_mgmt_conf_t      config;
    u8                        mac[VTSS_MEP_MAC_LENGTH];
    vtss_rc                   rc;

    if (inst == 0) {
        ICLI_PRINTF("Invalid MEP instance number\n");
        return;
    }
    if ((rc = mep_mgmt_conf_get(inst-1, mac, &eps_count, eps_inst, &config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if (config.enable) {
        /* no mep <inst:uint> voe */
        config.voe = FALSE;

        if ((rc = mep_mgmt_conf_set(inst-1, &config)) != VTSS_RC_OK) {
            mep_print_error(session_id, rc);
        }
    }
    else {
        ICLI_PRINTF("This MEP is not enabled\n");
    }
}

void mep_mep_peer_mep_id(i32 session_id, u32 inst,
                         u32 mepid, BOOL has_mac, vtss_mac_t peermac)
{
    u32                       eps_count, i;
    u16                       eps_inst[MEP_EPS_MAX];
    vtss_mep_mgmt_conf_t      config;
    vtss_mep_mgmt_def_conf_t  def_conf;
    u8                        mac[VTSS_MEP_MAC_LENGTH];
    vtss_rc                   rc;

    mep_mgmt_def_conf_get(&def_conf);

    if (inst == 0) {
        ICLI_PRINTF("Invalid MEP instance number\n");
        return;
    }
    if ((rc = mep_mgmt_conf_get(inst-1, mac, &eps_count, eps_inst, &config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if (config.enable) {
        /* mep <inst:uint> peer-mep-id <mepid:uint> [mac <mac:mac_addr>] */
        if (config.peer_count < VTSS_MEP_PEER_MAX) {
            for (i=0; i<config.peer_count; ++i) {    /* Check for this peer MEP id is known */
                if (config.peer_mep[i] == mepid) {
                    break;
                }
            }
            config.peer_mep[i] = mepid;
            memcpy(config.peer_mac[i], def_conf.config.peer_mac[0], sizeof(config.peer_mac[i])); /* Initialize peer MAC to default */
            if (has_mac) {
                memcpy(config.peer_mac[i], peermac.addr, sizeof(config.peer_mac[i]));
            }
            if (i == config.peer_count) { /* New peer MEP is added */
                config.peer_count++;
            }
            if ((rc = mep_mgmt_conf_set(inst-1, &config)) != VTSS_RC_OK) {
                mep_print_error(session_id, rc);
            }
        }
        else {
            ICLI_PRINTF("Max number of peer MEP is reached\n");
        }
    }
    else {
        ICLI_PRINTF("This MEP is not enabled\n");
    }
}

void mep_no_mep_peer_mep_id(i32 session_id, u32 inst,
                            u32 mepid, BOOL has_all)
{
    u32                   eps_count, i, idx;
    u16                   eps_inst[MEP_EPS_MAX];
    vtss_mep_mgmt_conf_t  config;
    u8                    mac[VTSS_MEP_MAC_LENGTH];
    u16                   peer_mep[VTSS_MEP_PEER_MAX];
    u8                    peer_mac[VTSS_MEP_PEER_MAX][VTSS_MEP_MAC_LENGTH];
    vtss_rc               rc;

    if (inst == 0) {
        ICLI_PRINTF("Invalid MEP instance number\n");
        return;
    }
    if ((rc = mep_mgmt_conf_get(inst-1, mac, &eps_count, eps_inst, &config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if (config.enable) {
        /* no mep <inst:uint> peer-mep-id {<mepid:uint> | all} */
        memset(peer_mep, 0, sizeof(peer_mep));
        memset(peer_mac, 0, sizeof(peer_mac));

        if (!has_all) { /* If not all to be deleted then save the peer MEP id that is not going to be deleted */
            for (i=0, idx=0; i<config.peer_count; ++i) {
                if (config.peer_mep[i] != mepid) {
                    peer_mep[idx] = config.peer_mep[i];
                    memcpy(peer_mac[idx], config.peer_mac[i], sizeof(peer_mac[idx]));
                    idx++;
                }
            }
        }
        else {
            idx = 0;    /* All is deleted */
        }
        memcpy(config.peer_mep, peer_mep, sizeof(config.peer_mep));
        memcpy(config.peer_mac, peer_mac, sizeof(config.peer_mac));
        config.peer_count = idx;
        if ((rc = mep_mgmt_conf_set(inst-1, &config)) != VTSS_RC_OK) {
            mep_print_error(session_id, rc);
        }
    }
    else {
        ICLI_PRINTF("This MEP is not enabled\n");
    }
}

void mep_mep_cc(i32 session_id, u32 inst,
                u32 prio, BOOL has_fr300s, BOOL has_fr100s, BOOL has_fr10s, BOOL has_fr1s, BOOL has_fr6m, BOOL has_fr1m, BOOL has_fr6h)
{
    u32                       eps_count;
    u16                       eps_inst[MEP_EPS_MAX];
    vtss_mep_mgmt_conf_t      config;
    vtss_mep_mgmt_cc_conf_t   cc_config;
    vtss_mep_mgmt_def_conf_t  def_conf;
    u8                        mac[VTSS_MEP_MAC_LENGTH];
    vtss_rc                   rc;

    mep_mgmt_def_conf_get(&def_conf);

    if (inst == 0) {
        ICLI_PRINTF("Invalid MEP instance number\n");
        return;
    }
    if ((rc = mep_mgmt_conf_get(inst-1, mac, &eps_count, eps_inst, &config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if (config.enable) {
        /* mep <inst:uint> cc <prio:0-7> [fr300s|fr100s|fr10s|fr1s|fr6m|fr1m|fr6h] */
        cc_config = def_conf.cc_conf;   /* Initialize cc_conf to default */
        cc_config.enable = TRUE;
        cc_config.dei = def_conf.cc_conf.dei;
        cc_config.prio = prio;
        if (has_fr300s || has_fr100s || has_fr10s || has_fr1s || has_fr6m || has_fr1m || has_fr6h) {
            cc_config.period = has_fr300s ? VTSS_MEP_MGMT_PERIOD_300S :
                            has_fr100s ? VTSS_MEP_MGMT_PERIOD_100S :
                            has_fr10s ? VTSS_MEP_MGMT_PERIOD_10S :
                            has_fr1s ? VTSS_MEP_MGMT_PERIOD_1S :
                            has_fr6m ? VTSS_MEP_MGMT_PERIOD_6M :
                            has_fr1m ? VTSS_MEP_MGMT_PERIOD_1M :
                            has_fr6h ? VTSS_MEP_MGMT_PERIOD_6H : VTSS_MEP_MGMT_PERIOD_1S;
        }
        if ((rc = mep_mgmt_cc_conf_set(inst-1, &cc_config)) != VTSS_RC_OK) {
            mep_print_error(session_id, rc);
        }
    }
    else {
        ICLI_PRINTF("This MEP is not enabled\n");
    }
}

void mep_no_mep_cc(i32 session_id, u32 inst)
{
    u32                       eps_count;
    u16                       eps_inst[MEP_EPS_MAX];
    vtss_mep_mgmt_conf_t      config;
    vtss_mep_mgmt_cc_conf_t   cc_config;
    u8                        mac[VTSS_MEP_MAC_LENGTH];
    vtss_rc                   rc;

    if (inst == 0) {
        ICLI_PRINTF("Invalid MEP instance number\n");
        return;
    }
    if ((rc = mep_mgmt_conf_get(inst-1, mac, &eps_count, eps_inst, &config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if (config.enable) {
        /* no mep <inst:uint> cc */
        cc_config.enable = FALSE;
        if ((rc = mep_mgmt_cc_conf_set(inst-1, &cc_config)) != VTSS_RC_OK) {
            mep_print_error(session_id, rc);
        }
    }
    else {
        ICLI_PRINTF("This MEP is not enabled\n");
    }
}

void mep_mep_lm(i32 session_id, u32 inst,
                u32 prio, BOOL has_uni, BOOL has_multi, BOOL has_single, BOOL has_dual, BOOL has_fr10s, BOOL has_fr1s, BOOL has_fr6m, BOOL has_fr1m, BOOL has_fr6h, BOOL has_flr, u32 flr)
{
    u32                       eps_count;
    u16                       eps_inst[MEP_EPS_MAX];
    vtss_mep_mgmt_conf_t      config;
    vtss_mep_mgmt_lm_conf_t   lm_config;
    vtss_mep_mgmt_def_conf_t  def_conf;
    u8                        mac[VTSS_MEP_MAC_LENGTH];
    vtss_rc                   rc;

    mep_mgmt_def_conf_get(&def_conf);

    if (inst == 0) {
        ICLI_PRINTF("Invalid MEP instance number\n");
        return;
    }
    if ((rc = mep_mgmt_conf_get(inst-1, mac, &eps_count, eps_inst, &config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if (config.enable) {
        /* mep <inst:uint> lm <prio:0-7> [multi|uni] [single|dual] [fr10s|fr1s|fr6m|fr1m|fr6h] [flr <flr:uint>] */
        lm_config = def_conf.lm_conf;   /* Initialize lm_conf to default */
        lm_config.enable = TRUE;
        lm_config.dei = def_conf.lm_conf.dei;
        lm_config.prio = prio;
        if (has_fr10s || has_fr1s || has_fr6m || has_fr1m || has_fr6h) {
            lm_config.period = has_fr10s ? VTSS_MEP_MGMT_PERIOD_10S :
                               has_fr1s ? VTSS_MEP_MGMT_PERIOD_1S :
                               has_fr6m ? VTSS_MEP_MGMT_PERIOD_6M :
                               has_fr1m ? VTSS_MEP_MGMT_PERIOD_1M : VTSS_MEP_MGMT_PERIOD_6H;
        }
        if (has_uni || has_multi) {
            lm_config.cast = has_uni ? VTSS_MEP_MGMT_UNICAST :
                             has_multi ? VTSS_MEP_MGMT_MULTICAST : VTSS_MEP_MGMT_UNICAST;
        }
        if (has_single || has_dual) {
            lm_config.ended = has_single ? VTSS_MEP_MGMT_SINGEL_ENDED :
                              has_dual ? VTSS_MEP_MGMT_DUAL_ENDED : VTSS_MEP_MGMT_SINGEL_ENDED;
        }
        if (has_flr) {
            lm_config.flr_interval = flr;
        }
        if ((rc = mep_mgmt_lm_conf_set(inst-1, &lm_config)) != VTSS_RC_OK) {
            mep_print_error(session_id, rc);
        }
    }
    else {
        ICLI_PRINTF("This MEP is not enabled\n");
    }
}

void mep_no_mep_lm(i32 session_id, u32 inst)
{
    u32                       eps_count;
    u16                       eps_inst[MEP_EPS_MAX];
    vtss_mep_mgmt_conf_t      config;
    vtss_mep_mgmt_lm_conf_t   lm_config;
    u8                        mac[VTSS_MEP_MAC_LENGTH];
    vtss_rc                   rc;

    if (inst == 0) {
        ICLI_PRINTF("Invalid MEP instance number\n");
        return;
    }
    if ((rc = mep_mgmt_conf_get(inst-1, mac, &eps_count, eps_inst, &config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if (config.enable) {
        /* no mep <inst:uint> lm */
        lm_config.enable = FALSE;
        if ((rc = mep_mgmt_lm_conf_set(inst-1, &lm_config)) != VTSS_RC_OK) {
            mep_print_error(session_id, rc);
        }
    }
    else {
        ICLI_PRINTF("This MEP is not enabled\n");
    }
}

void mep_mep_dm(i32 session_id, u32 inst,
                u32 prio, BOOL has_uni, BOOL has_multi, u32 mepid, BOOL has_dual, BOOL has_single, BOOL has_rdtrp, BOOL has_flow, u32 interval, u32 lastn)
{
    u32                       eps_count;
    u16                       eps_inst[MEP_EPS_MAX];
    vtss_mep_mgmt_conf_t      config;
    vtss_mep_mgmt_dm_conf_t   dm_config;
    vtss_mep_mgmt_def_conf_t  def_conf;
    u8                        mac[VTSS_MEP_MAC_LENGTH];
    vtss_rc                   rc;

    mep_mgmt_def_conf_get(&def_conf);

    if (inst == 0) {
        ICLI_PRINTF("Invalid MEP instance number\n");
        return;
    }
    if ((rc = mep_mgmt_conf_get(inst-1, mac, &eps_count, eps_inst, &config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if (config.enable) {
        /* mep <inst:uint> dm <prio:0-7> [multi|{uni mep-id <mepid:uint>}] [single|dual] [rdtrp|flow] interval <interval:uint> last-n <lastn:uint> */
        dm_config = def_conf.dm_conf;   /* Initialize dm_conf to default */
        dm_config.enable = TRUE;
        dm_config.dei = def_conf.dm_conf.dei;
        dm_config.prio = prio;
        if (has_uni || has_multi) {
            dm_config.cast = has_uni ? VTSS_MEP_MGMT_UNICAST :
                             has_multi ? VTSS_MEP_MGMT_MULTICAST : VTSS_MEP_MGMT_UNICAST;
        }
        if (has_uni) {
            dm_config.mep = mepid;
        }
        if (has_dual || has_single) {
            dm_config.ended = has_dual ? VTSS_MEP_MGMT_DUAL_ENDED :
                              has_single ? VTSS_MEP_MGMT_SINGEL_ENDED : VTSS_MEP_MGMT_DUAL_ENDED;
        }
        if (has_rdtrp || has_flow) {
            dm_config.calcway = has_rdtrp ? VTSS_MEP_MGMT_RDTRP :
                                has_flow ? VTSS_MEP_MGMT_FLOW : VTSS_MEP_MGMT_RDTRP;
        }
        dm_config.interval = interval;
        dm_config.lastn = lastn;
        if ((rc = mep_mgmt_dm_conf_set(inst-1, &dm_config)) != VTSS_RC_OK) {
            mep_print_error(session_id, rc);
        }
    }
    else {
        ICLI_PRINTF("This MEP is not enabled\n");
    }
}

void mep_no_mep_dm(i32 session_id, u32 inst)
{
    u32                       eps_count;
    u16                       eps_inst[MEP_EPS_MAX];
    vtss_mep_mgmt_conf_t      config;
    vtss_mep_mgmt_dm_conf_t   dm_config;
    u8                        mac[VTSS_MEP_MAC_LENGTH];
    vtss_rc                   rc;

    if (inst == 0) {
        ICLI_PRINTF("Invalid MEP instance number\n");
        return;
    }
    if ((rc = mep_mgmt_conf_get(inst-1, mac, &eps_count, eps_inst, &config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if (config.enable) {
        /* no mep <inst:uint> dm */
        dm_config.enable = FALSE;
        if ((rc = mep_mgmt_dm_conf_set(inst-1, &dm_config)) != VTSS_RC_OK) {
            mep_print_error(session_id, rc);
        }
    }
    else {
        ICLI_PRINTF("This MEP is not enabled\n");
    }
}

void mep_mep_dm_overflow_reset(i32 session_id, u32 inst)
{
    u32                       eps_count;
    u16                       eps_inst[MEP_EPS_MAX];
    vtss_mep_mgmt_conf_t      config;
    vtss_mep_mgmt_dm_conf_t   dm_config;
    u8                        mac[VTSS_MEP_MAC_LENGTH];
    vtss_rc                   rc;

    if (inst == 0) {
        ICLI_PRINTF("Invalid MEP instance number\n");
        return;
    }
    if ((rc = mep_mgmt_conf_get(inst-1, mac, &eps_count, eps_inst, &config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if ((rc = mep_mgmt_dm_conf_get(inst-1, &dm_config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if (config.enable) {
        /* mep <inst:uint> dm overflow reset */
        dm_config.overflow_act = VTSS_MEP_MGMT_CONTINUE;
        if ((rc = mep_mgmt_dm_conf_set(inst-1, &dm_config)) != VTSS_RC_OK) {
            mep_print_error(session_id, rc);
        }
    }
    else {
        ICLI_PRINTF("This MEP is not enabled\n");
    }
}

void mep_no_mep_dm_overflow_reset(i32 session_id, u32 inst)
{
    u32                       eps_count;
    u16                       eps_inst[MEP_EPS_MAX];
    vtss_mep_mgmt_conf_t      config;
    vtss_mep_mgmt_dm_conf_t   dm_config;
    vtss_mep_mgmt_def_conf_t  def_conf;
    u8                        mac[VTSS_MEP_MAC_LENGTH];
    vtss_rc                   rc;

    mep_mgmt_def_conf_get(&def_conf);

    if (inst == 0) {
        ICLI_PRINTF("Invalid MEP instance number\n");
        return;
    }
    if ((rc = mep_mgmt_conf_get(inst-1, mac, &eps_count, eps_inst, &config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if ((rc = mep_mgmt_dm_conf_get(inst-1, &dm_config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if (config.enable) {
        /* no mep <inst:uint> dm overflow reset */
        dm_config.overflow_act = def_conf.dm_conf.overflow_act;   /* Initialize dm_conf.overflow_act to default */
        if ((rc = mep_mgmt_dm_conf_set(inst-1, &dm_config)) != VTSS_RC_OK) {
            mep_print_error(session_id, rc);
        }
    }
    else {
        ICLI_PRINTF("This MEP is not enabled\n");
    }
}

void mep_mep_dm_ns(i32 session_id, u32 inst)
{
    u32                       eps_count;
    u16                       eps_inst[MEP_EPS_MAX];
    vtss_mep_mgmt_conf_t      config;
    vtss_mep_mgmt_dm_conf_t   dm_config;
    u8                        mac[VTSS_MEP_MAC_LENGTH];
    vtss_rc                   rc;

    if (inst == 0) {
        ICLI_PRINTF("Invalid MEP instance number\n");
        return;
    }
    if ((rc = mep_mgmt_conf_get(inst-1, mac, &eps_count, eps_inst, &config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if ((rc = mep_mgmt_dm_conf_get(inst-1, &dm_config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if (config.enable) {
        /* mep <inst:uint> dm ns */
        dm_config.tunit = VTSS_MEP_MGMT_NS;
        if ((rc = mep_mgmt_dm_conf_set(inst-1, &dm_config)) != VTSS_RC_OK) {
            mep_print_error(session_id, rc);
        }
    }
    else {
        ICLI_PRINTF("This MEP is not enabled\n");
    }
}

void mep_no_mep_dm_ns(i32 session_id, u32 inst)
{
    u32                       eps_count;
    u16                       eps_inst[MEP_EPS_MAX];
    vtss_mep_mgmt_conf_t      config;
    vtss_mep_mgmt_dm_conf_t   dm_config;
    vtss_mep_mgmt_def_conf_t  def_conf;
    u8                        mac[VTSS_MEP_MAC_LENGTH];
    vtss_rc                   rc;

    mep_mgmt_def_conf_get(&def_conf);

    if (inst == 0) {
        ICLI_PRINTF("Invalid MEP instance number\n");
        return;
    }
    if ((rc = mep_mgmt_conf_get(inst-1, mac, &eps_count, eps_inst, &config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if ((rc = mep_mgmt_dm_conf_get(inst-1, &dm_config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if (config.enable) {
        /* no mep <inst:uint> dm ns */
        dm_config.tunit = def_conf.dm_conf.tunit;   /* Initialize dm_conf.tunit to default */
        if ((rc = mep_mgmt_dm_conf_set(inst-1, &dm_config)) != VTSS_RC_OK) {
            mep_print_error(session_id, rc);
        }
    }
    else {
        ICLI_PRINTF("This MEP is not enabled\n");
    }
}

void mep_mep_dm_syncronized(i32 session_id, u32 inst)
{
    u32                       eps_count;
    u16                       eps_inst[MEP_EPS_MAX];
    vtss_mep_mgmt_conf_t      config;
    vtss_mep_mgmt_dm_conf_t   dm_config;
    u8                        mac[VTSS_MEP_MAC_LENGTH];
    vtss_rc                   rc;

    if (inst == 0) {
        ICLI_PRINTF("Invalid MEP instance number\n");
        return;
    }
    if ((rc = mep_mgmt_conf_get(inst-1, mac, &eps_count, eps_inst, &config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if ((rc = mep_mgmt_dm_conf_get(inst-1, &dm_config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if (config.enable) {
        /* mep <inst:uint> dm syncronized */
        dm_config.syncronized = TRUE;
        if ((rc = mep_mgmt_dm_conf_set(inst-1, &dm_config)) != VTSS_RC_OK) {
            mep_print_error(session_id, rc);
        }
    }
    else {
        ICLI_PRINTF("This MEP is not enabled\n");
    }
}

void mep_no_mep_dm_syncronized(i32 session_id, u32 inst)
{
    u32                       eps_count;
    u16                       eps_inst[MEP_EPS_MAX];
    vtss_mep_mgmt_conf_t      config;
    vtss_mep_mgmt_dm_conf_t   dm_config;
    vtss_mep_mgmt_def_conf_t  def_conf;
    u8                        mac[VTSS_MEP_MAC_LENGTH];
    vtss_rc                   rc;

    mep_mgmt_def_conf_get(&def_conf);

    if (inst == 0) {
        ICLI_PRINTF("Invalid MEP instance number\n");
        return;
    }
    if ((rc = mep_mgmt_conf_get(inst-1, mac, &eps_count, eps_inst, &config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if ((rc = mep_mgmt_dm_conf_get(inst-1, &dm_config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if (config.enable) {
        /* no mep <inst:uint> dm syncronized */
        dm_config.syncronized = def_conf.dm_conf.syncronized;   /* Initialize dm_conf.syncronized to default */
        if ((rc = mep_mgmt_dm_conf_set(inst-1, &dm_config)) != VTSS_RC_OK) {
            mep_print_error(session_id, rc);
        }
    }
    else {
        ICLI_PRINTF("This MEP is not enabled\n");
    }
}

void mep_mep_dm_proprietary(i32 session_id, u32 inst)
{
    u32                       eps_count;
    u16                       eps_inst[MEP_EPS_MAX];
    vtss_mep_mgmt_conf_t      config;
    vtss_mep_mgmt_dm_conf_t   dm_config;
    u8                        mac[VTSS_MEP_MAC_LENGTH];
    vtss_rc                   rc;

    if (inst == 0) {
        ICLI_PRINTF("Invalid MEP instance number\n");
        return;
    }
    if ((rc = mep_mgmt_conf_get(inst-1, mac, &eps_count, eps_inst, &config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if ((rc = mep_mgmt_dm_conf_get(inst-1, &dm_config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if (config.enable) {
        /* mep <inst:uint> dm proprietary */
        dm_config.proprietary = TRUE;
        if ((rc = mep_mgmt_dm_conf_set(inst-1, &dm_config)) != VTSS_RC_OK) {
            mep_print_error(session_id, rc);
        }
    }
    else {
        ICLI_PRINTF("This MEP is not enabled\n");
    }
}

void mep_no_mep_dm_proprietary(i32 session_id, u32 inst)
{
    u32                       eps_count;
    u16                       eps_inst[MEP_EPS_MAX];
    vtss_mep_mgmt_conf_t      config;
    vtss_mep_mgmt_dm_conf_t   dm_config;
    vtss_mep_mgmt_def_conf_t  def_conf;
    u8                        mac[VTSS_MEP_MAC_LENGTH];
    vtss_rc                   rc;

    mep_mgmt_def_conf_get(&def_conf);

    if (inst == 0) {
        ICLI_PRINTF("Invalid MEP instance number\n");
        return;
    }
    if ((rc = mep_mgmt_conf_get(inst-1, mac, &eps_count, eps_inst, &config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if ((rc = mep_mgmt_dm_conf_get(inst-1, &dm_config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if (config.enable) {
        /* no mep <inst:uint> dm proprietary */
        dm_config.proprietary = def_conf.dm_conf.proprietary;   /* Initialize dm_conf.proprietary to default */
        if ((rc = mep_mgmt_dm_conf_set(inst-1, &dm_config)) != VTSS_RC_OK) {
            mep_print_error(session_id, rc);
        }
    }
    else {
        ICLI_PRINTF("This MEP is not enabled\n");
    }
}

void mep_mep_lt(i32 session_id, u32 inst,
                u32 prio, BOOL has_mep_id, u32 mepid, BOOL has_mac, vtss_mac_t lt_mac, u32 ttl)
{
    u32                       eps_count;
    u16                       eps_inst[MEP_EPS_MAX];
    vtss_mep_mgmt_conf_t      config;
    vtss_mep_mgmt_lt_conf_t   lt_config;
    vtss_mep_mgmt_def_conf_t  def_conf;
    u8                        mac[VTSS_MEP_MAC_LENGTH];
    vtss_rc                   rc;

    mep_mgmt_def_conf_get(&def_conf);

    if (inst == 0) {
        ICLI_PRINTF("Invalid MEP instance number\n");
        return;
    }
    if ((rc = mep_mgmt_conf_get(inst-1, mac, &eps_count, eps_inst, &config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if (config.enable) {
        /* mep <inst:uint> lt <prio:0-7> {{mep-id <mepid:uint>} | {mac <mac:mac_addr>}} ttl <ttl:uint> */
        lt_config = def_conf.lt_conf;   /* Initialize lt_conf to default */
        lt_config.enable = TRUE;
        lt_config.dei = def_conf.lt_conf.dei;
        lt_config.prio = prio;
        if (has_mep_id) {
            lt_config.mep = mepid;
        }
        if (has_mac) {
            memcpy(lt_config.mac, lt_mac.addr, sizeof(lt_config.mac));
        }
        lt_config.ttl = ttl;
        if ((rc = mep_mgmt_lt_conf_set(inst-1, &lt_config)) != VTSS_RC_OK) {
            mep_print_error(session_id, rc);
        }
    }
    else {
        ICLI_PRINTF("This MEP is not enabled\n");
    }
}

void mep_no_mep_lt(i32 session_id, u32 inst)
{
    u32                       eps_count;
    u16                       eps_inst[MEP_EPS_MAX];
    vtss_mep_mgmt_conf_t      config;
    vtss_mep_mgmt_lt_conf_t   lt_config;
    u8                        mac[VTSS_MEP_MAC_LENGTH];
    vtss_rc                   rc;

    if (inst == 0) {
        ICLI_PRINTF("Invalid MEP instance number\n");
        return;
    }
    if ((rc = mep_mgmt_conf_get(inst-1, mac, &eps_count, eps_inst, &config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if (config.enable) {
        /* no mep <inst:uint> lt */
        lt_config.enable = FALSE;
        if ((rc = mep_mgmt_lt_conf_set(inst-1, &lt_config)) != VTSS_RC_OK) {
            mep_print_error(session_id, rc);
        }
    }
    else {
        ICLI_PRINTF("This MEP is not enabled\n");
    }
}

void mep_mep_lb(i32 session_id, u32 inst,
                u32 prio, BOOL has_dei, BOOL has_multi, BOOL has_uni, BOOL has_mep_id, u32 mepid, BOOL has_mac, vtss_mac_t lb_mac, u32 count, u32 size, u32 interval)
{
    u32                       eps_count;
    u16                       eps_inst[MEP_EPS_MAX];
    vtss_mep_mgmt_conf_t      config;
    vtss_mep_mgmt_lb_conf_t   lb_config;
    vtss_mep_mgmt_def_conf_t  def_conf;
    u8                        mac[VTSS_MEP_MAC_LENGTH];
    vtss_rc                   rc;

    mep_mgmt_def_conf_get(&def_conf);

    if (inst == 0) {
        ICLI_PRINTF("Invalid MEP instance number\n");
        return;
    }
    if ((rc = mep_mgmt_conf_get(inst-1, mac, &eps_count, eps_inst, &config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if (config.enable) {
        /* mep <inst:uint> lb <prio:0-7> [dei] [multi|{uni {{mep-id <mepid:uint>} | {mac <mac:mac_addr>}}}] count <count:uint> size <size:uint> interval <interval:uint> */
        lb_config = def_conf.lb_conf;   /* Initialize lb_conf to default */
        lb_config.enable = TRUE;
        if (has_dei) {
            lb_config.dei = TRUE;
        }
        lb_config.prio = prio;
        if (has_uni || has_multi) {
            lb_config.cast = has_uni ? VTSS_MEP_MGMT_UNICAST :
                             has_multi ? VTSS_MEP_MGMT_MULTICAST : VTSS_MEP_MGMT_UNICAST;
        }
        if (has_mep_id) {
            lb_config.mep = mepid;
        }
        if (has_mac) {
            memcpy(lb_config.mac, lb_mac.addr, sizeof(lb_config.mac));
        }
        lb_config.to_send = count;
        lb_config.size = size;
        lb_config.interval = interval;
        if ((rc = mep_mgmt_lb_conf_set(inst-1, &lb_config)) != VTSS_RC_OK) {
            mep_print_error(session_id, rc);
        }
    }
    else {
        ICLI_PRINTF("This MEP is not enabled\n");
    }
}

void mep_no_mep_lb(i32 session_id, u32 inst)
{
    u32                       eps_count;
    u16                       eps_inst[MEP_EPS_MAX];
    vtss_mep_mgmt_conf_t      config;
    vtss_mep_mgmt_lb_conf_t   lb_config;
    u8                        mac[VTSS_MEP_MAC_LENGTH];
    vtss_rc                   rc;

    if (inst == 0) {
        ICLI_PRINTF("Invalid MEP instance number\n");
        return;
    }
    if ((rc = mep_mgmt_conf_get(inst-1, mac, &eps_count, eps_inst, &config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if (config.enable) {
        /* no mep <inst:uint> lb */
        lb_config.enable = FALSE;
        if ((rc = mep_mgmt_lb_conf_set(inst-1, &lb_config)) != VTSS_RC_OK) {
            mep_print_error(session_id, rc);
        }
    }
    else {
        ICLI_PRINTF("This MEP is not enabled\n");
    }
}

void mep_mep_tst(i32 session_id, u32 inst,
                 u32 prio, BOOL has_dei, u32 mepid, BOOL has_sequence, BOOL has_all_zero, BOOL has_all_one, BOOL has_one_zero, u32 rate, u32 size)
{
    u32                       eps_count;
    u16                       eps_inst[MEP_EPS_MAX];
    vtss_mep_mgmt_conf_t      config;
    vtss_mep_mgmt_tst_conf_t  tst_config;
    vtss_mep_mgmt_def_conf_t  def_conf;
    u8                        mac[VTSS_MEP_MAC_LENGTH];
    vtss_rc                   rc;

    mep_mgmt_def_conf_get(&def_conf);

    if (inst == 0) {
        ICLI_PRINTF("Invalid MEP instance number\n");
        return;
    }
    if ((rc = mep_mgmt_conf_get(inst-1, mac, &eps_count, eps_inst, &config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if (config.enable) {
        /* mep <inst:uint> tst <prio:0-7> [dei] mep-id <mepid:uint> [sequence] [all-zero|all-one|one-zero] rate <rate:uint> size <size:uint> */
        tst_config = def_conf.tst_conf;   /* Initialize lb_conf to default */
        if (has_dei) {
            tst_config.dei = TRUE;
        }
        tst_config.prio = prio;
        tst_config.mep = mepid;
        tst_config.rate = rate * 1000;
        tst_config.size = size;
        if (has_all_zero || has_all_one || has_one_zero) {
            tst_config.pattern = has_all_zero ? VTSS_MEP_MGMT_PATTERN_ALL_ZERO :
                                 has_all_one ? VTSS_MEP_MGMT_PATTERN_ALL_ONE :
                                 has_one_zero ? VTSS_MEP_MGMT_PATTERN_0XAA : VTSS_MEP_MGMT_PATTERN_ALL_ZERO;
        }
        if (has_sequence) {
            tst_config.sequence = TRUE;
        }
        if ((rc = mep_mgmt_tst_conf_set(inst-1, &tst_config)) != VTSS_RC_OK) {
            mep_print_error(session_id, rc);
        }
    }
    else {
        ICLI_PRINTF("This MEP is not enabled\n");
    }
}

void mep_mep_tst_tx(i32 session_id, u32 inst)
{
    u32                       eps_count;
    u16                       eps_inst[MEP_EPS_MAX];
    vtss_mep_mgmt_conf_t      config;
    vtss_mep_mgmt_tst_conf_t  tst_config;
    u8                        mac[VTSS_MEP_MAC_LENGTH];
    vtss_rc                   rc;

    if (inst == 0) {
        ICLI_PRINTF("Invalid MEP instance number\n");
        return;
    }
    if ((rc = mep_mgmt_conf_get(inst-1, mac, &eps_count, eps_inst, &config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if ((rc = mep_mgmt_tst_conf_get(inst-1, &tst_config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if (config.enable) {
        /* mep <inst:uint> tst tx */
        tst_config.enable = TRUE;
        if ((rc = mep_mgmt_tst_conf_set(inst-1, &tst_config)) != VTSS_RC_OK) {
            mep_print_error(session_id, rc);
        }
    }
    else {
        ICLI_PRINTF("This MEP is not enabled\n");
    }
}

void mep_no_mep_tst_tx(i32 session_id, u32 inst)
{
    u32                       eps_count;
    u16                       eps_inst[MEP_EPS_MAX];
    vtss_mep_mgmt_conf_t      config;
    vtss_mep_mgmt_tst_conf_t  tst_config;
    vtss_mep_mgmt_def_conf_t  def_conf;
    u8                        mac[VTSS_MEP_MAC_LENGTH];
    vtss_rc                   rc;

    mep_mgmt_def_conf_get(&def_conf);

    if (inst == 0) {
        ICLI_PRINTF("Invalid MEP instance number\n");
        return;
    }
    if ((rc = mep_mgmt_conf_get(inst-1, mac, &eps_count, eps_inst, &config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if ((rc = mep_mgmt_tst_conf_get(inst-1, &tst_config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if (config.enable) {
        /* no mep <inst:uint> tst tx */
        tst_config.enable = def_conf.tst_conf.enable;   /* Initialize tst_conf.enable to default */
        if ((rc = mep_mgmt_tst_conf_set(inst-1, &tst_config)) != VTSS_RC_OK) {
            mep_print_error(session_id, rc);
        }
    }
    else {
        ICLI_PRINTF("This MEP is not enabled\n");
    }
}

void mep_mep_tst_rx(i32 session_id, u32 inst)
{
    u32                       eps_count;
    u16                       eps_inst[MEP_EPS_MAX];
    vtss_mep_mgmt_conf_t      config;
    vtss_mep_mgmt_tst_conf_t  tst_config;
    u8                        mac[VTSS_MEP_MAC_LENGTH];
    vtss_rc                   rc;

    if (inst == 0) {
        ICLI_PRINTF("Invalid MEP instance number\n");
        return;
    }
    if ((rc = mep_mgmt_conf_get(inst-1, mac, &eps_count, eps_inst, &config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if ((rc = mep_mgmt_tst_conf_get(inst-1, &tst_config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if (config.enable) {
        /* mep <inst:uint> tst rx */
        tst_config.enable_rx = TRUE;
        if ((rc = mep_mgmt_tst_conf_set(inst-1, &tst_config)) != VTSS_RC_OK) {
            mep_print_error(session_id, rc);
        }
    }
    else {
        ICLI_PRINTF("This MEP is not enabled\n");
    }
}

void mep_no_mep_tst_rx(i32 session_id, u32 inst)
{
    u32                       eps_count;
    u16                       eps_inst[MEP_EPS_MAX];
    vtss_mep_mgmt_conf_t      config;
    vtss_mep_mgmt_tst_conf_t  tst_config;
    vtss_mep_mgmt_def_conf_t  def_conf;
    u8                        mac[VTSS_MEP_MAC_LENGTH];
    vtss_rc                   rc;

    mep_mgmt_def_conf_get(&def_conf);

    if (inst == 0) {
        ICLI_PRINTF("Invalid MEP instance number\n");
        return;
    }
    if ((rc = mep_mgmt_conf_get(inst-1, mac, &eps_count, eps_inst, &config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if ((rc = mep_mgmt_tst_conf_get(inst-1, &tst_config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if (config.enable) {
        /* no mep <inst:uint> tst rx */
        tst_config.enable_rx = def_conf.tst_conf.enable_rx;   /* Initialize tst_conf.enable_rx to default */
        if ((rc = mep_mgmt_tst_conf_set(inst-1, &tst_config)) != VTSS_RC_OK) {
            mep_print_error(session_id, rc);
        }
    }
    else {
        ICLI_PRINTF("This MEP is not enabled\n");
    }
}

void mep_mep_aps(i32 session_id, u32 inst,
                 u32 prio, BOOL has_multi, BOOL has_uni, BOOL has_laps, BOOL has_raps, BOOL has_octet, u32 octet)
{
    u32                       eps_count;
    u16                       eps_inst[MEP_EPS_MAX];
    vtss_mep_mgmt_conf_t      config;
    vtss_mep_mgmt_aps_conf_t  aps_config;
    vtss_mep_mgmt_def_conf_t  def_conf;
    u8                        mac[VTSS_MEP_MAC_LENGTH];
    vtss_rc                   rc;

    mep_mgmt_def_conf_get(&def_conf);

    if (inst == 0) {
        ICLI_PRINTF("Invalid MEP instance number\n");
        return;
    }
    if ((rc = mep_mgmt_conf_get(inst-1, mac, &eps_count, eps_inst, &config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if (config.enable) {
        /* mep <inst:uint> aps <prio:0-7> [multi|uni] {laps|{raps [octet <octet:uint>]}} */
        aps_config = def_conf.aps_conf;   /* Initialize aps_conf to default */
        aps_config.enable = TRUE;
        aps_config.prio = prio;
        if (has_uni || has_multi) {
            aps_config.cast = has_uni ? VTSS_MEP_MGMT_UNICAST :
                              has_multi ? VTSS_MEP_MGMT_MULTICAST : VTSS_MEP_MGMT_UNICAST;
        }
        if (has_laps || has_raps) {
            aps_config.type = has_laps ? VTSS_MEP_MGMT_L_APS :
                              has_raps ? VTSS_MEP_MGMT_R_APS : VTSS_MEP_MGMT_L_APS;
        }
        if (has_octet) {
            aps_config.raps_octet = octet;
        }
        if ((rc = mep_mgmt_aps_conf_set(inst-1, &aps_config)) != VTSS_RC_OK) {
            mep_print_error(session_id, rc);
        }
    }
    else {
        ICLI_PRINTF("This MEP is not enabled\n");
    }
}

void mep_no_mep_aps(i32 session_id, u32 inst)
{
    u32                       eps_count;
    u16                       eps_inst[MEP_EPS_MAX];
    vtss_mep_mgmt_conf_t      config;
    vtss_mep_mgmt_aps_conf_t  aps_config;
    u8                        mac[VTSS_MEP_MAC_LENGTH];
    vtss_rc                   rc;

    if (inst == 0) {
        ICLI_PRINTF("Invalid MEP instance number\n");
        return;
    }
    if ((rc = mep_mgmt_conf_get(inst-1, mac, &eps_count, eps_inst, &config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if (config.enable) {
        /* no mep <inst:uint> aps */
        aps_config.enable = FALSE;
        if ((rc = mep_mgmt_aps_conf_set(inst-1, &aps_config)) != VTSS_RC_OK) {
            mep_print_error(session_id, rc);
        }
    }
    else {
        ICLI_PRINTF("This MEP is not enabled\n");
    }
}

void mep_mep_client(i32 session_id, u32 inst,
                    BOOL has_evc, BOOL has_vlan)

{
    u32                          eps_count;
    u16                          eps_inst[MEP_EPS_MAX];
    vtss_mep_mgmt_conf_t         config;
    vtss_mep_mgmt_client_conf_t  client_config;
    u8                           mac[VTSS_MEP_MAC_LENGTH];
    vtss_rc                      rc;

    if (inst == 0) {
        ICLI_PRINTF("Invalid MEP instance number\n");
        return;
    }
    if ((rc = mep_mgmt_conf_get(inst-1, mac, &eps_count, eps_inst, &config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if ((rc = mep_mgmt_client_conf_get(inst-1, &client_config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if (config.enable) {
        /* mep <inst:uint> client domain {evc|vlan} */
        client_config.domain = has_evc ? VTSS_MEP_MGMT_EVC :
                               has_vlan ? VTSS_MEP_MGMT_VLAN : VTSS_MEP_MGMT_EVC;
        if ((rc = mep_mgmt_client_conf_set(inst-1, &client_config)) != VTSS_RC_OK) {
            mep_print_error(session_id, rc);
        }
    }
    else {
        ICLI_PRINTF("This MEP is not enabled\n");
    }
}

void mep_mep_client_flow(i32 session_id, u32 inst,
                         u32 cflow, u32 level, BOOL has_ais_prio, u32 aisprio, BOOL has_ais_highest, BOOL has_lck_prio, u32 lckprio, BOOL has_lck_highest)
{
    u32                          eps_count, i;
    u16                          eps_inst[MEP_EPS_MAX];
    vtss_mep_mgmt_conf_t         config;
    vtss_mep_mgmt_client_conf_t  client_config;
    vtss_mep_mgmt_def_conf_t     def_conf;
    u8                           mac[VTSS_MEP_MAC_LENGTH];
    vtss_rc                      rc;

    mep_mgmt_def_conf_get(&def_conf);

    if (inst == 0) {
        ICLI_PRINTF("Invalid MEP instance number\n");
        return;
    }
    if ((rc = mep_mgmt_conf_get(inst-1, mac, &eps_count, eps_inst, &config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if ((rc = mep_mgmt_client_conf_get(inst-1, &client_config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if (config.enable) {
        /* mep <inst:uint> client flow <cflow:uint> level <level:0-7> [ais-prio <aisprio:0-7>] [lck-prio <lckprio:0-7>] */
        if (client_config.domain != VTSS_MEP_MGMT_VLAN) {
            cflow = cflow-1;
        }
        if (client_config.flow_count < VTSS_MEP_CLIENT_FLOWS_MAX) {
            for (i=0; i<client_config.flow_count; ++i) {     /* Check for this client id is known */
                if (client_config.flows[i] == cflow) {
                    break;
                }
            }
            client_config.flows[i] = cflow;
            client_config.level[i] = level;
            client_config.ais_prio[i] = def_conf.client_conf.ais_prio[0];
            client_config.lck_prio[i] = def_conf.client_conf.lck_prio[0];
            if (has_ais_prio) {
                client_config.ais_prio[i] = has_ais_highest ? VTSS_MEP_CLIENT_PRIO_HIGHEST : aisprio;
            }
            if (has_lck_prio) {
                client_config.lck_prio[i] = has_lck_highest ? VTSS_MEP_CLIENT_PRIO_HIGHEST : lckprio;
            }
            if (i == client_config.flow_count) { /* New client flow is added */
                client_config.flow_count++;
            }
            if ((rc = mep_mgmt_client_conf_set(inst-1, &client_config)) != VTSS_RC_OK) {
                mep_print_error(session_id, rc);
            }
        }
        else {
            ICLI_PRINTF("Max number of client flow is reached\n");
        }
    }
    else {
        ICLI_PRINTF("This MEP is not enabled\n");
    }
}

void mep_no_mep_client_flow(i32 session_id, u32 inst,
                         u32 cflow, BOOL has_all)
{
    u32                          eps_count, i, idx;
    u16                          eps_inst[MEP_EPS_MAX];
    vtss_mep_mgmt_conf_t         config;
    vtss_mep_mgmt_client_conf_t  client_config;
    u8                           mac[VTSS_MEP_MAC_LENGTH];
    u32                          flows[VTSS_MEP_CLIENT_FLOWS_MAX];
    u8                           ais_prio[VTSS_MEP_CLIENT_FLOWS_MAX];
    u8                           lck_prio[VTSS_MEP_CLIENT_FLOWS_MAX];
    u8                           level[VTSS_MEP_CLIENT_FLOWS_MAX];
    vtss_rc                      rc;

    if (inst == 0) {
        ICLI_PRINTF("Invalid MEP instance number\n");
        return;
    }
    if ((rc = mep_mgmt_conf_get(inst-1, mac, &eps_count, eps_inst, &config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if ((rc = mep_mgmt_client_conf_get(inst-1, &client_config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if (config.enable) {
        /* no mep <inst:uint> client flow {<cflow:uint> | all} */
        memset(flows, 0, sizeof(flows));
        memset(ais_prio, 0, sizeof(ais_prio));
        memset(lck_prio, 0, sizeof(lck_prio));
        memset(level, 0, sizeof(level));
        cflow = cflow-1;

        if (!has_all) { /* If not all to be deleted then save the client flow that is not going to be deleted */
            for (i=0, idx=0; i<client_config.flow_count; ++i) {
                if (client_config.flows[i] != cflow) {
                    flows[idx] = client_config.flows[i];
                    ais_prio[idx] = client_config.ais_prio[i];
                    lck_prio[idx] = client_config.lck_prio[i];
                    level[idx] = client_config.level[i];
                    idx++;
                }
            }
        }
        else {
            idx = 0;    /* All is deleted */
        }
        memcpy(client_config.flows, flows, sizeof(client_config.flows));
        memcpy(client_config.ais_prio, ais_prio, sizeof(client_config.ais_prio));
        memcpy(client_config.lck_prio, lck_prio, sizeof(client_config.lck_prio));
        memcpy(client_config.level, level, sizeof(client_config.level));
        client_config.flow_count = idx;
        if ((rc = mep_mgmt_client_conf_set(inst-1, &client_config)) != VTSS_RC_OK) {
            mep_print_error(session_id, rc);
        }
    }
    else {
        ICLI_PRINTF("This MEP is not enabled\n");
    }
}

void mep_mep_ais(i32 session_id, u32 inst,
                 BOOL has_fr1s, BOOL has_fr1m, BOOL has_protect)
{
    u32                       eps_count;
    u16                       eps_inst[MEP_EPS_MAX];
    vtss_mep_mgmt_conf_t      config;
    vtss_mep_mgmt_ais_conf_t  ais_config;
    vtss_mep_mgmt_def_conf_t  def_conf;
    u8                        mac[VTSS_MEP_MAC_LENGTH];
    vtss_rc                   rc;

    mep_mgmt_def_conf_get(&def_conf);

    if (inst == 0) {
        ICLI_PRINTF("Invalid MEP instance number\n");
        return;
    }
    if ((rc = mep_mgmt_conf_get(inst-1, mac, &eps_count, eps_inst, &config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if (config.enable) {
        /* mep <inst:uint> ais [fr1s|fr1m] [protect] */
        ais_config = def_conf.ais_conf;   /* Initialize ais_conf to default */
        ais_config.enable = TRUE;
        if (has_fr1s || has_fr1m) {
            ais_config.period = has_fr1s ? VTSS_MEP_MGMT_PERIOD_1S :
                                has_fr1m ? VTSS_MEP_MGMT_PERIOD_1M : VTSS_MEP_MGMT_PERIOD_1S;
        }
        if (has_protect) {
            ais_config.protection = TRUE;
        }
        if ((rc = mep_mgmt_ais_conf_set(inst-1, &ais_config)) != VTSS_RC_OK) {
            mep_print_error(session_id, rc);
        }
    }
    else {
        ICLI_PRINTF("This MEP is not enabled\n");
    }
}

void mep_no_mep_ais(i32 session_id, u32 inst)
{
    u32                       eps_count;
    u16                       eps_inst[MEP_EPS_MAX];
    vtss_mep_mgmt_conf_t      config;
    vtss_mep_mgmt_ais_conf_t  ais_config;
    u8                        mac[VTSS_MEP_MAC_LENGTH];
    vtss_rc                   rc;

    if (inst == 0) {
        ICLI_PRINTF("Invalid MEP instance number\n");
        return;
    }
    if ((rc = mep_mgmt_conf_get(inst-1, mac, &eps_count, eps_inst, &config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    else {
        if (config.enable) {
            /* no mep <inst:uint> ais */
            ais_config.enable = FALSE;
            if ((rc = mep_mgmt_ais_conf_set(inst-1, &ais_config)) != VTSS_RC_OK) {
                mep_print_error(session_id, rc);
            }
        }
        else {
            ICLI_PRINTF("This MEP is not enabled\n");
        }
    }
}

void mep_mep_lck(i32 session_id, u32 inst,
                 BOOL has_fr1s, BOOL has_fr1m)
{
    u32                       eps_count;
    u16                       eps_inst[MEP_EPS_MAX];
    vtss_mep_mgmt_conf_t      config;
    vtss_mep_mgmt_lck_conf_t  lck_config;
    vtss_mep_mgmt_def_conf_t  def_conf;
    u8                        mac[VTSS_MEP_MAC_LENGTH];
    vtss_rc                   rc;

    mep_mgmt_def_conf_get(&def_conf);

    if (inst == 0) {
        ICLI_PRINTF("Invalid MEP instance number\n");
        return;
    }
    if ((rc = mep_mgmt_conf_get(inst-1, mac, &eps_count, eps_inst, &config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if (config.enable) {
        /* mep <inst:uint> lck [fr1s|fr1m] */
        lck_config = def_conf.lck_conf;   /* Initialize lck_conf to default */
        lck_config.enable = TRUE;
        if (has_fr1s || has_fr1m) {
            lck_config.period = has_fr1s ? VTSS_MEP_MGMT_PERIOD_1S :
                                has_fr1m ? VTSS_MEP_MGMT_PERIOD_1M : VTSS_MEP_MGMT_PERIOD_1S;
        }
        if ((rc = mep_mgmt_lck_conf_set(inst-1, &lck_config)) != VTSS_RC_OK) {
            mep_print_error(session_id, rc);
        }
    }
    else {
        ICLI_PRINTF("This MEP is not enabled\n");
    }
}

void mep_no_mep_lck(i32 session_id, u32 inst)
{
    u32                       eps_count;
    u16                       eps_inst[MEP_EPS_MAX];
    vtss_mep_mgmt_conf_t      config;
    vtss_mep_mgmt_lck_conf_t  lck_config;
    u8                        mac[VTSS_MEP_MAC_LENGTH];
    vtss_rc                   rc;

    if (inst == 0) {
        ICLI_PRINTF("Invalid MEP instance number\n");
        return;
    }
    if ((rc = mep_mgmt_conf_get(inst-1, mac, &eps_count, eps_inst, &config)) != VTSS_RC_OK) {
        mep_print_error(session_id, rc);
        return;
    }
    if (config.enable) {
        /* no mep <inst:uint> lck */
        lck_config.enable = FALSE;
        if ((rc = mep_mgmt_lck_conf_set(inst-1, &lck_config)) != VTSS_RC_OK) {
            mep_print_error(session_id, rc);
        }
    }
    else {
        ICLI_PRINTF("This MEP is not enabled\n");
    }
}

void mep_mep_volatile(i32 session_id, u32 inst)
{
    u32                       eps_count;
    u16                       eps_inst[MEP_EPS_MAX];
    vtss_mep_mgmt_conf_t      config;
    u8                        mac[VTSS_MEP_MAC_LENGTH];
    vtss_rc                   rc;

    if (inst == 0) {
        ICLI_PRINTF("Invalid MEP instance number\n");
        return;
    }
    if (((rc = mep_mgmt_conf_get(inst-1, mac, &eps_count, eps_inst, &config)) != VTSS_RC_OK)) {
        mep_print_error(session_id, rc);
        return;
    }
    if (config.enable) {
        /* mep <inst:uint> volatile */
        if ((rc = mep_mgmt_volatile_conf_set(inst-1, &config)) != VTSS_RC_OK) {
            mep_print_error(session_id, rc);
        }
    }
    else {
        ICLI_PRINTF("This MEP is not enabled\n");
    }
}

void mep_no_mep_volatile(i32 session_id, u32 inst)
{
    u32                       eps_count;
    u16                       eps_inst[MEP_EPS_MAX];
    vtss_mep_mgmt_conf_t      config;
    u8                        mac[VTSS_MEP_MAC_LENGTH];
    vtss_rc                   rc;

    if (inst == 0) {
        ICLI_PRINTF("Invalid MEP instance number\n");
        return;
    }
    if (((rc = mep_mgmt_conf_get(inst-1, mac, &eps_count, eps_inst, &config)) != MEP_RC_VOLATILE)) {
        mep_print_error(session_id, rc);
        return;
    }
    if (config.enable) {
        /* no mep <inst:uint> volatile */
        config.enable = FALSE;

        if ((rc = mep_mgmt_volatile_conf_set(inst-1, &config)) != VTSS_RC_OK) {
            mep_print_error(session_id, rc);
        }
    }
    else {
        ICLI_PRINTF("This MEP is not enabled\n");
    }
}


/****************************************************************************/
/* ICFG callback functions */
/****************************************************************************/
#ifdef VTSS_SW_OPTION_ICFG
static vtss_rc mep_icfg_conf(const vtss_icfg_query_request_t  *req,
                             vtss_icfg_query_result_t         *result)
{
    u32 i, j, eps_count;
    vtss_mep_mgmt_conf_t         config;
    vtss_mep_mgmt_pm_conf_t      pm_conf;
    vtss_mep_mgmt_cc_conf_t      cc_conf;
    vtss_mep_mgmt_lm_conf_t      lm_conf;
    vtss_mep_mgmt_dm_conf_t      dm_conf;
    vtss_mep_mgmt_aps_conf_t     aps_conf;
    vtss_mep_mgmt_lt_conf_t      lt_conf;
    vtss_mep_mgmt_lb_conf_t      lb_conf;
    vtss_mep_mgmt_ais_conf_t     ais_conf;
    vtss_mep_mgmt_lck_conf_t     lck_conf;
    vtss_mep_mgmt_tst_conf_t     tst_conf;
    vtss_mep_mgmt_client_conf_t  client_conf;
    vtss_mep_mgmt_def_conf_t     def_conf;
    vtss_icfg_conf_print_t       conf_print;
    u8                           mac[VTSS_MEP_MAC_LENGTH];
    u16                          eps_inst[MEP_EPS_MAX];
    u8                           all_zero_mac[VTSS_MEP_MAC_LENGTH] = {0,0,0,0,0,0};
    char                         buf[ICLI_PORTING_STR_BUF_SIZE];

    vtss_icfg_conf_print_init(&conf_print);

    // Get default configuration
    mep_mgmt_def_conf_get(&def_conf);
    config = def_conf.config;
    cc_conf = def_conf.cc_conf;
    pm_conf = def_conf.pm_conf;
    lm_conf = def_conf.lm_conf;
    dm_conf = def_conf.dm_conf;
    aps_conf = def_conf.aps_conf;
    lt_conf = def_conf.lt_conf;
    lb_conf = def_conf.lb_conf;
    ais_conf = def_conf.ais_conf;
    lck_conf = def_conf.lck_conf;
    tst_conf = def_conf.tst_conf;
    client_conf = def_conf.client_conf;

    for (i=0; i<MEP_INSTANCE_MAX; ++i) {
        if ((mep_mgmt_conf_get(i, mac, &eps_count, eps_inst, &config)) != VTSS_RC_OK) {
            continue;
        }
        if (config.enable) {
            /* mep <inst:uint> [mip] {up|down} domain {evc|vlan|{port [vid <vid:vlan_id>]}} flow <flow:uint> level <level:0-7> interface <port:port_type_id> */
            VTSS_RC(vtss_icfg_printf(result, "mep %u", i+1));
            if ((config.mode != def_conf.config.mode) || req->all_defaults) {
                VTSS_RC(vtss_icfg_printf(result, " %s", config.mode == VTSS_MEP_MGMT_MIP ? "mip" : "mep"));
            }
            VTSS_RC(vtss_icfg_printf(result, " %s", config.direction == VTSS_MEP_MGMT_DOWN ? "down" : "up"));
            VTSS_RC(vtss_icfg_printf(result, " domain %s", config.domain == VTSS_MEP_MGMT_PORT ? "port" :
                                                           config.domain == VTSS_MEP_MGMT_EVC ? "evc" :
                                                           config.domain == VTSS_MEP_MGMT_VLAN ? "vlan" : "port"));
            if (((config.domain == VTSS_MEP_MGMT_PORT) && (config.direction == VTSS_MEP_MGMT_UP) && (config.vid != 0)) ||
                ((config.domain == VTSS_MEP_MGMT_EVC) && (config.mode == VTSS_MEP_MGMT_MIP) && (config.vid != 0))){
                VTSS_RC(vtss_icfg_printf(result, " vid %u", config.vid));
            }
            VTSS_RC(vtss_icfg_printf(result, " flow %u", (config.domain != VTSS_MEP_MGMT_VLAN) ? config.flow+1 : config.flow));
            VTSS_RC(vtss_icfg_printf(result, " level %u", config.level));
            VTSS_RC(vtss_icfg_printf(result, " interface %s", icli_port_info_txt(VTSS_USID_START, iport2uport(config.port), buf)));
            VTSS_RC(vtss_icfg_printf(result, "\n"));
            if ((config.format != def_conf.config.format) || memcmp(config.meg, def_conf.config.meg, VTSS_MEP_MEG_CODE_LENGTH) ||
                memcmp(config.name, def_conf.config.name, VTSS_MEP_MEG_CODE_LENGTH) || req->all_defaults) {
                /* mep <inst:uint> meg-id <megid:word> {itu | itu-cc | {ieee [name <name:word>]}} */
                VTSS_RC(vtss_icfg_printf(result, "mep %u", i+1));
                config.meg[VTSS_MEP_MEG_CODE_LENGTH-1] = '\0'; /* This must be nulterminated string otherwise printf will lead to exception */
                config.name[VTSS_MEP_MEG_CODE_LENGTH-1] = '\0'; /* This must be nulterminated string otherwise printf will lead to exception */
                VTSS_RC(vtss_icfg_printf(result, " meg-id %s", config.meg));
                VTSS_RC(vtss_icfg_printf(result, " %s", config.format == VTSS_MEP_MGMT_ITU_ICC ? "itu" :
                                                        config.format == VTSS_MEP_MGMT_IEEE_STR ? "ieee" :
                                                        config.format == VTSS_MEP_MGMT_ITU_CC_ICC ? "itu-cc" : "itu"));
                if (memcmp(config.name, def_conf.config.name, VTSS_MEP_MEG_CODE_LENGTH) || req->all_defaults) {
                    VTSS_RC(vtss_icfg_printf(result, " name %s", config.format == VTSS_MEP_MGMT_IEEE_STR ? config.name : ""));
                }
                VTSS_RC(vtss_icfg_printf(result, "\n"));
            }
            if ((config.mep != def_conf.config.mep) || req->all_defaults) {
                /* mep <inst:uint> mep-id <mepid:uint> */
                VTSS_RC(vtss_icfg_printf(result, "mep %u", i+1));
                VTSS_RC(vtss_icfg_printf(result, " mep-id %u", config.mep));
                VTSS_RC(vtss_icfg_printf(result, "\n"));
            }
            if ((config.vid != def_conf.config.vid) || req->all_defaults) {
                /* mep <inst:uint> vid <vid:vlan_id> */
                VTSS_RC(vtss_icfg_printf(result, "mep %u", i+1));
                VTSS_RC(vtss_icfg_printf(result, " vid %u", config.vid));
                VTSS_RC(vtss_icfg_printf(result, "\n"));
            }
            if (config.voe) {
                /* mep <inst:uint> voe */
                VTSS_RC(vtss_icfg_printf(result, "mep %u", i+1));
                VTSS_RC(vtss_icfg_printf(result, " voe"));
                VTSS_RC(vtss_icfg_printf(result, "\n"));
            }
            /* mep <inst:uint> peer-mep-id <mepid:uint> [mac <mac:mac_addr>] */
            for (j=0; j<config.peer_count; ++j) {
                VTSS_RC(vtss_icfg_printf(result, "mep %u", i+1));
                VTSS_RC(vtss_icfg_printf(result, " peer-mep-id %u", config.peer_mep[j]));
                if (memcmp(config.peer_mac[j], def_conf.config.peer_mac[j], VTSS_MEP_MAC_LENGTH) || req->all_defaults) {
                    VTSS_RC(vtss_icfg_printf(result, " mac %02X-%02X-%02X-%02X-%02X-%02X", config.peer_mac[j][0], config.peer_mac[j][1], config.peer_mac[j][2],
                                                                                           config.peer_mac[j][3], config.peer_mac[j][4], config.peer_mac[j][5]));
                }
                VTSS_RC(vtss_icfg_printf(result, "\n"));
            }

            if ((mep_mgmt_pm_conf_get(i, &pm_conf)) == VTSS_RC_OK) {
                if (pm_conf.enable) {
                    /* mep <inst:uint> performance-monitoring */
                    VTSS_RC(vtss_icfg_printf(result, "mep %u", i+1));
                    VTSS_RC(vtss_icfg_printf(result, " performance-monitoring"));
                    VTSS_RC(vtss_icfg_printf(result, "\n"));
                }
            }

            if ((mep_mgmt_cc_conf_get(i, &cc_conf)) == VTSS_RC_OK) {
                if (cc_conf.enable) {
                    /* mep <inst:uint> cc <prio:0-7> [fr300s|fr100s|fr10s|fr1s|fr6m|fr1m|fr6h] */
                    VTSS_RC(vtss_icfg_printf(result, "mep %u cc", i+1));
                    VTSS_RC(vtss_icfg_printf(result, " %u", cc_conf.prio));
                    if ((cc_conf.period != def_conf.cc_conf.period) || req->all_defaults) {
                        VTSS_RC(vtss_icfg_printf(result, " %s", cc_conf.period == VTSS_MEP_MGMT_PERIOD_300S ? "fr300s" :
                                                                cc_conf.period == VTSS_MEP_MGMT_PERIOD_100S ? "fr100s" :
                                                                cc_conf.period == VTSS_MEP_MGMT_PERIOD_10S ? "fr10s" :
                                                                cc_conf.period == VTSS_MEP_MGMT_PERIOD_1S ? "fr1s" :
                                                                cc_conf.period == VTSS_MEP_MGMT_PERIOD_6M ? "fr6m" :
                                                                cc_conf.period == VTSS_MEP_MGMT_PERIOD_1M ? "fr1m" : "fr6h"));
                    }
                    VTSS_RC(vtss_icfg_printf(result, "\n"));
                }
            }

            if ((mep_mgmt_lm_conf_get(i, &lm_conf)) == VTSS_RC_OK) {
                if (lm_conf.enable) {
                    /* mep <inst:uint> lm <prio:0-7> [multi|uni] [single|dual] [fr10s|fr1s|fr6m|fr1m|fr6h] [flr <flr:uint>] */
                    VTSS_RC(vtss_icfg_printf(result, "mep %u lm", i+1));
                    VTSS_RC(vtss_icfg_printf(result, " %u", lm_conf.prio));
                    if ((lm_conf.cast != def_conf.lm_conf.cast) || req->all_defaults) {
                        VTSS_RC(vtss_icfg_printf(result, " %s", lm_conf.cast == VTSS_MEP_MGMT_UNICAST ? "uni" : "multi"));
                    }
                    if ((lm_conf.ended != def_conf.lm_conf.ended) || req->all_defaults) {
                        VTSS_RC(vtss_icfg_printf(result, " %s", lm_conf.ended == VTSS_MEP_MGMT_SINGEL_ENDED ? "single" : "dual"));
                    }
                    if ((lm_conf.period != def_conf.lm_conf.period) || req->all_defaults) {
                        VTSS_RC(vtss_icfg_printf(result, " %s", lm_conf.period == VTSS_MEP_MGMT_PERIOD_10S ? "fr10s" :
                                                                lm_conf.period == VTSS_MEP_MGMT_PERIOD_1S ? "fr1s" :
                                                                lm_conf.period == VTSS_MEP_MGMT_PERIOD_6M ? "fr6m" :
                                                                lm_conf.period == VTSS_MEP_MGMT_PERIOD_1M ? "fr1m" : "fr6h"));
                    }
                    if ((lm_conf.flr_interval != def_conf.lm_conf.flr_interval) || req->all_defaults) {
                        VTSS_RC(vtss_icfg_printf(result, " flr %u", lm_conf.flr_interval));
                    }
                    VTSS_RC(vtss_icfg_printf(result, "\n"));
                }
            }

            if ((mep_mgmt_dm_conf_get(i, &dm_conf)) == VTSS_RC_OK) {
                if (dm_conf.enable) {
                    /* mep <inst:uint> dm <prio:0-7> [multi|{uni mep-id <mepid:uint>}] [single|dual] [rdtrp|flow] interval <interval:uint> last-n <lastn:uint> */
                    VTSS_RC(vtss_icfg_printf(result, "mep %u dm", i+1));
                    VTSS_RC(vtss_icfg_printf(result, " %u", dm_conf.prio));
                    if ((dm_conf.cast != def_conf.dm_conf.cast) || req->all_defaults) {
                        if (dm_conf.cast == VTSS_MEP_MGMT_MULTICAST) {
                            VTSS_RC(vtss_icfg_printf(result, " multi"));
                        } else {
                            VTSS_RC(vtss_icfg_printf(result, " uni mep-id %u", dm_conf.mep));
                        }
                    }
                    if ((dm_conf.ended != def_conf.dm_conf.ended) || req->all_defaults) {
                        VTSS_RC(vtss_icfg_printf(result, " %s", dm_conf.ended == VTSS_MEP_MGMT_DUAL_ENDED ? "dual" : "single"));
                    }
                    if ((dm_conf.calcway != def_conf.dm_conf.calcway) || req->all_defaults) {
                        VTSS_RC(vtss_icfg_printf(result, " %s", dm_conf.calcway == VTSS_MEP_MGMT_RDTRP ? "rdtrp" : "flow"));
                    }
                    VTSS_RC(vtss_icfg_printf(result, " interval %u", dm_conf.interval));
                    VTSS_RC(vtss_icfg_printf(result, " last-n %u", dm_conf.lastn));
                    VTSS_RC(vtss_icfg_printf(result, "\n"));
                    /* mep <inst:uint> dm overflow reset*/
                    if (dm_conf.overflow_act == VTSS_MEP_MGMT_CONTINUE) {
                        VTSS_RC(vtss_icfg_printf(result, "mep %u dm", i+1));
                        VTSS_RC(vtss_icfg_printf(result, " overflow reset"));
                        VTSS_RC(vtss_icfg_printf(result, "\n"));
                    }
                    /* mep <inst:uint> dm ns */
                    if (dm_conf.tunit == VTSS_MEP_MGMT_NS) {
                        VTSS_RC(vtss_icfg_printf(result, "mep %u dm", i+1));
                        VTSS_RC(vtss_icfg_printf(result, " ns"));
                        VTSS_RC(vtss_icfg_printf(result, "\n"));
                    }
                    /* mep <inst:uint> dm syncronized */
                    if (dm_conf.syncronized) {
                        VTSS_RC(vtss_icfg_printf(result, "mep %u dm", i+1));
                        VTSS_RC(vtss_icfg_printf(result, " syncronized"));
                        VTSS_RC(vtss_icfg_printf(result, "\n"));
                    }
                    /* mep <inst:uint> dm proprietary */
                    if (dm_conf.proprietary) {
                        VTSS_RC(vtss_icfg_printf(result, "mep %u dm", i+1));
                        VTSS_RC(vtss_icfg_printf(result, " proprietary"));
                        VTSS_RC(vtss_icfg_printf(result, "\n"));
                    }
                }
            }

            if ((mep_mgmt_aps_conf_get(i, &aps_conf)) == VTSS_RC_OK) {
                if (aps_conf.enable) {
                    /* mep <inst:uint> aps <prio:0-7> [multi|uni] {laps|{raps [octet <octet:uint>]}} */
                    VTSS_RC(vtss_icfg_printf(result, "mep %u aps", i+1));
                    VTSS_RC(vtss_icfg_printf(result, " %u", aps_conf.prio));
                    if ((aps_conf.cast != def_conf.aps_conf.cast) || req->all_defaults) {
                        VTSS_RC(vtss_icfg_printf(result, " %s", aps_conf.cast == VTSS_MEP_MGMT_UNICAST ? "uni" : "multi"));
                    }
                    VTSS_RC(vtss_icfg_printf(result, " %s", aps_conf.type == VTSS_MEP_MGMT_L_APS ? "laps" : "raps"));
                    if ((aps_conf.raps_octet != def_conf.aps_conf.raps_octet) || req->all_defaults) {
                        VTSS_RC(vtss_icfg_printf(result, " %u", aps_conf.raps_octet));
                    }
                    VTSS_RC(vtss_icfg_printf(result, "\n"));
                }
            }

            if ((mep_mgmt_lt_conf_get(i, &lt_conf)) == VTSS_RC_OK) {
                if (lt_conf.enable) {
                    /*mep <inst:uint> lt <prio:0-7> {{mep-id <mepid:uint>} | {mac <mac:mac_addr>}} ttl <ttl:uint> */
                    VTSS_RC(vtss_icfg_printf(result, "mep %u lt", i+1));
                    VTSS_RC(vtss_icfg_printf(result, " %u", lt_conf.prio));
                    if (!memcmp(all_zero_mac, lt_conf.mac, VTSS_MEP_MAC_LENGTH)) {
                        VTSS_RC(vtss_icfg_printf(result, " mep-id %u", lt_conf.mep));
                    } else {
                        VTSS_RC(vtss_icfg_printf(result, " mac %02X-%02X-%02X-%02X-%02X-%02X", lt_conf.mac[0], lt_conf.mac[1], lt_conf.mac[2], lt_conf.mac[3], lt_conf.mac[4], lt_conf.mac[5]));
                    }
                    VTSS_RC(vtss_icfg_printf(result, "\n"));
                }
            }

            if ((mep_mgmt_lb_conf_get(i, &lb_conf)) == VTSS_RC_OK) {
                if (lb_conf.enable) {
                    /* mep <inst:uint> lb <prio:0-7> [dei] [multi|{uni {{mep-id <mepid:uint>} | {mac <mac:mac_addr>}}}] count <count:uint> size <size:uint> interval <interval:uint> */
                    VTSS_RC(vtss_icfg_printf(result, "mep %u lb", i+1));
                    VTSS_RC(vtss_icfg_printf(result, " %u", lb_conf.prio));
                    if ((lb_conf.dei != def_conf.lb_conf.dei) || req->all_defaults) {
                        VTSS_RC(vtss_icfg_printf(result, " %s", lb_conf.dei ? "dei" : ""));
                    }
                    if ((lb_conf.cast != def_conf.lb_conf.cast) || req->all_defaults) {
                        VTSS_RC(vtss_icfg_printf(result, " %s", (lb_conf.cast == VTSS_MEP_MGMT_UNICAST) ? "uni" :
                                                                (lb_conf.cast == VTSS_MEP_MGMT_MULTICAST) ? "multi" : "uni"));
                    }
                    if (lb_conf.cast == VTSS_MEP_MGMT_UNICAST) {
                        if (!memcmp(all_zero_mac, lb_conf.mac, VTSS_MEP_MAC_LENGTH)) {
                            VTSS_RC(vtss_icfg_printf(result, " mep-id %u", lb_conf.mep));
                        } else {
                            VTSS_RC(vtss_icfg_printf(result, " mac %02X-%02X-%02X-%02X-%02X-%02X", lb_conf.mac[0], lb_conf.mac[1], lb_conf.mac[2], lb_conf.mac[3], lb_conf.mac[4], lb_conf.mac[5]));
                        }
                    }
                    VTSS_RC(vtss_icfg_printf(result, " count %u", lb_conf.to_send));
                    VTSS_RC(vtss_icfg_printf(result, " size %u", lb_conf.size));
                    VTSS_RC(vtss_icfg_printf(result, " interval %u", lb_conf.interval));
                    VTSS_RC(vtss_icfg_printf(result, "\n"));
                }
            }

            if ((mep_mgmt_tst_conf_get(i, &tst_conf)) == VTSS_RC_OK) {
                if ((tst_conf.prio != def_conf.tst_conf.prio) || (tst_conf.dei != def_conf.tst_conf.dei) || (tst_conf.mep != def_conf.tst_conf.mep) ||
                    (tst_conf.rate != def_conf.tst_conf.rate) || (tst_conf.size != def_conf.tst_conf.size) || (tst_conf.pattern != def_conf.tst_conf.pattern) ||
                    (tst_conf.sequence != def_conf.tst_conf.sequence) || req->all_defaults) {
                    /* mep <inst:uint> tst <prio:0-7> [dei] mep-id <mepid:uint> [sequence] [all-zero|all-one|one-zero] rate <rate:uint> size <size:uint> */
                    VTSS_RC(vtss_icfg_printf(result, "mep %u tst", i+1));
                    VTSS_RC(vtss_icfg_printf(result, " %u", tst_conf.prio));
                    if ((tst_conf.dei != def_conf.tst_conf.dei) || req->all_defaults) {
                        VTSS_RC(vtss_icfg_printf(result, " %s", tst_conf.dei ? "dei" : ""));
                    }
                    VTSS_RC(vtss_icfg_printf(result, " mep-id %u", tst_conf.mep));
                    if ((tst_conf.sequence != def_conf.tst_conf.sequence) || req->all_defaults) {
                        VTSS_RC(vtss_icfg_printf(result, " %s", tst_conf.sequence ? "sequence" : ""));
                    }
                    if ((tst_conf.pattern != def_conf.tst_conf.pattern) || req->all_defaults) {
                        VTSS_RC(vtss_icfg_printf(result, " %s", tst_conf.pattern == VTSS_MEP_MGMT_PATTERN_ALL_ZERO ? "all-zero" :
                                                                tst_conf.pattern == VTSS_MEP_MGMT_PATTERN_ALL_ONE ? "all-one" : "one-zero"));
                    }
                    VTSS_RC(vtss_icfg_printf(result, " rate %u", tst_conf.rate/1000));
                    VTSS_RC(vtss_icfg_printf(result, " size %u", tst_conf.size));
                    VTSS_RC(vtss_icfg_printf(result, "\n"));
                }
                if (tst_conf.enable) {
                    /* mep <inst:uint> tst tx */
                    VTSS_RC(vtss_icfg_printf(result, "mep %u tst tx", i+1));
                    VTSS_RC(vtss_icfg_printf(result, "\n"));
                }
                if (tst_conf.enable_rx) {
                    /* mep <inst:uint> tst rx */
                    VTSS_RC(vtss_icfg_printf(result, "mep %u tst rx", i+1));
                    VTSS_RC(vtss_icfg_printf(result, "\n"));
                }
            }

            if ((mep_mgmt_client_conf_get(i, &client_conf)) == VTSS_RC_OK) {
                /* mep <inst:uint> client domain {evc|vlan} */
                if ((client_conf.domain != def_conf.client_conf.domain) || req->all_defaults) {
                    VTSS_RC(vtss_icfg_printf(result, "mep %u client", i+1));
                    VTSS_RC(vtss_icfg_printf(result, " domain %s", client_conf.domain == VTSS_MEP_MGMT_EVC ? "evc" :
                                                                   client_conf.domain == VTSS_MEP_MGMT_VLAN ? "vlan" : "evc"));
                    VTSS_RC(vtss_icfg_printf(result, "\n"));
                }

                for (j=0; j<client_conf.flow_count && j<VTSS_MEP_CLIENT_FLOWS_MAX; ++j) {
                    /* mep <inst:uint> client flow <cflow:uint> level <level:0-7> [ais-prio [<aisprio:0-7> | ais-highest]] [lck-prio [<lckprio:0-7> | lck-highest]] */
                    VTSS_RC(vtss_icfg_printf(result, "mep %u client", i+1));
                    VTSS_RC(vtss_icfg_printf(result, " flow %u", (client_conf.domain != VTSS_MEP_MGMT_VLAN) ? client_conf.flows[j]+1 : client_conf.flows[j]));
                    VTSS_RC(vtss_icfg_printf(result, " level %u", client_conf.level[j]));
                    if ((client_conf.ais_prio[j] != def_conf.client_conf.ais_prio[j]) || req->all_defaults) {
                        if (client_conf.ais_prio[j] == VTSS_MEP_CLIENT_PRIO_HIGHEST) {
                            VTSS_RC(vtss_icfg_printf(result, " ais-prio ais-highest"));
                        } else {
                            VTSS_RC(vtss_icfg_printf(result, " ais-prio %u", client_conf.ais_prio[j]));
                        }
                    }
                    if ((client_conf.lck_prio[j] != def_conf.client_conf.lck_prio[j]) || req->all_defaults) {
                        if (client_conf.lck_prio[j] == VTSS_MEP_CLIENT_PRIO_HIGHEST) {
                            VTSS_RC(vtss_icfg_printf(result, " lck-prio lck-highest"));
                        } else {
                            VTSS_RC(vtss_icfg_printf(result, " lck-prio %u", client_conf.lck_prio[j]));
                        }
                    }
                    VTSS_RC(vtss_icfg_printf(result, "\n"));
                }
            }

            if ((mep_mgmt_ais_conf_get(i, &ais_conf)) == VTSS_RC_OK) {
                if (ais_conf.enable) {
                    /* mep <inst:uint> ais [fr1s|fr1m] [protect] */
                    VTSS_RC(vtss_icfg_printf(result, "mep %u ais", i+1));
                    if ((ais_conf.period != def_conf.ais_conf.period) || req->all_defaults) {
                        VTSS_RC(vtss_icfg_printf(result, " %s", ais_conf.period == VTSS_MEP_MGMT_PERIOD_1S ? "fr1s" : "fr1m"));
                    }
                    if ((ais_conf.protection != def_conf.ais_conf.protection) || req->all_defaults) {
                        VTSS_RC(vtss_icfg_printf(result, " %s", ais_conf.protection ? "protect" : ""));
                    }
                    VTSS_RC(vtss_icfg_printf(result, "\n"));
                }
            }

            if ((mep_mgmt_lck_conf_get(i, &lck_conf)) == VTSS_RC_OK) {
                if (lck_conf.enable) {
                    /* mep <inst:uint> lck [fr1s|fr1m] */
                    VTSS_RC(vtss_icfg_printf(result, "mep %u lck", i+1));
                    if ((lck_conf.period != def_conf.lck_conf.period) || req->all_defaults) {
                        VTSS_RC(vtss_icfg_printf(result, " %s", lck_conf.period == VTSS_MEP_MGMT_PERIOD_1S ? "fr1s" : "fr1m"));
                    }
                    VTSS_RC(vtss_icfg_printf(result, "\n"));
                }
            }
        }
    }

    return VTSS_RC_OK;
}

/* ICFG Initialization function */
vtss_rc mep_icfg_init(void)
{
    VTSS_RC(vtss_icfg_query_register(VTSS_ICFG_MEP_GLOBAL_CONF, "mep", mep_icfg_conf));
    return VTSS_RC_OK;
}
#endif // VTSS_SW_OPTION_ICFG
/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
