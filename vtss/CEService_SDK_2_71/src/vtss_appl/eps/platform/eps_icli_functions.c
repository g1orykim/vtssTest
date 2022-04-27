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
#include "eps_api.h"
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
static void eps_print_error(u32 session_id,  u32 rc)
{
    switch (rc)
    {
        case EPS_RC_NOT_CREATED:            ICLI_PRINTF("EPS instance not created\n"); break;
        case EPS_RC_CREATED:                ICLI_PRINTF("EPS instance already created\n"); break;
        case EPS_RC_INVALID_PARAMETER:      ICLI_PRINTF("Invalid parameter\n"); break;
        case EPS_RC_NOT_CONFIGURED:         ICLI_PRINTF("EPS NOT created\n"); break;
        case EPS_RC_ARCHITECTURE:           ICLI_PRINTF("Only 1+1 in port domain\n"); break;
        case EPS_RC_W_P_FLOW_EQUAL:         ICLI_PRINTF("Working and protecting flow is equal\n"); break;
        case EPS_RC_W_P_SSF_MEP_EQUAL:      ICLI_PRINTF("Working and protecting SF MEP is equal\n"); break;
        case EPS_RC_INVALID_APS_MEP:        ICLI_PRINTF("Invalid APS MEP\n"); break;
        case EPS_RC_INVALID_W_MEP:          ICLI_PRINTF("Invalid working SF MEP\n"); break;
        case EPS_RC_INVALID_P_MEP:          ICLI_PRINTF("Invalid protecting SF MEP\n"); break;
        case EPS_RC_WORKING_USED:           ICLI_PRINTF("Working flow is used by other instance\n"); break;
        case EPS_RC_PROTECTING_USED:        ICLI_PRINTF("Protecting flow is used by other instance\n"); break;
        default:                            ICLI_PRINTF("Unknown error returned from eps\n"); break;
    }
}


/***************************************************************************/
/*  Functions called by iCLI                                                */
/****************************************************************************/

void eps_show_eps(i32 session_id, icli_range_t *inst, BOOL has_detail)
{
    u32 rc, i, j, list_cnt, min, max;
    vtss_eps_mgmt_state_t         state;
    vtss_eps_mgmt_conf_t          config;
    vtss_eps_mgmt_create_param_t  param;
    eps_mgmt_mep_t                mep;

    ICLI_PRINTF("\n");
    ICLI_PRINTF("EPS state is:\n");
    ICLI_PRINTF("%9s", "Inst");
    ICLI_PRINTF("%13s", "State");
    ICLI_PRINTF("%11s", "Wstate");
    ICLI_PRINTF("%11s", "Pstate");
    ICLI_PRINTF("%14s", "TxAps r b");
    ICLI_PRINTF("%14s", "RxAps r b");
    ICLI_PRINTF("%10s", "FopPm");
    ICLI_PRINTF("%10s", "FopCm");
    ICLI_PRINTF("%10s", "FopNr");
    ICLI_PRINTF("%13s", "FopNoAps");
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
            max = EPS_MGMT_CREATED_MAX;
        }
        else {
            min = inst->u.sr.range[i].min;
            max = inst->u.sr.range[i].max;
        }

        if ((min == 0) || (max == 0)) {
            ICLI_PRINTF("Invalid EPS instance number\n");
            continue;
        }
        for (j=min-1; j<max; ++j) {
            if ((rc = eps_mgmt_state_get(j, &state)) != EPS_RC_OK) {
                if ((rc != EPS_RC_NOT_CREATED) && (rc != EPS_RC_NOT_CONFIGURED)) {
                    eps_print_error(session_id, rc);
                }
                continue;
            }
            ICLI_PRINTF("%9u", j+1);
            switch (state.protection_state)
            {
                case VTSS_EPS_MGMT_PROT_STATE_DISABLED:          ICLI_PRINTF("%13s", "Disable");    break;
                case VTSS_EPS_MGMT_PROT_STATE_NO_REQUEST_W:      ICLI_PRINTF("%13s", "NoReqW");     break;
                case VTSS_EPS_MGMT_PROT_STATE_NO_REQUEST_P:      ICLI_PRINTF("%13s", "NoReqP");     break;
                case VTSS_EPS_MGMT_PROT_STATE_LOCKOUT:           ICLI_PRINTF("%13s", "Lockout");    break;
                case VTSS_EPS_MGMT_PROT_STATE_FORCED_SWITCH:     ICLI_PRINTF("%13s", "Forced");     break;
                case VTSS_EPS_MGMT_PROT_STATE_SIGNAL_FAIL_W:     ICLI_PRINTF("%13s", "SfW");        break;
                case VTSS_EPS_MGMT_PROT_STATE_SIGNAL_FAIL_P:     ICLI_PRINTF("%13s", "SfP");        break;
                case VTSS_EPS_MGMT_PROT_STATE_MANUEL_SWITCH_W:   ICLI_PRINTF("%13s", "ManualW");    break;
                case VTSS_EPS_MGMT_PROT_STATE_MANUEL_SWITCH_P:   ICLI_PRINTF("%13s", "ManualP");    break;
                case VTSS_EPS_MGMT_PROT_STATE_WAIT_TO_RESTORE:   ICLI_PRINTF("%13s", "Wtr");        break;
                case VTSS_EPS_MGMT_PROT_STATE_EXERCISE_W:        ICLI_PRINTF("%13s", "ExerW");      break;
                case VTSS_EPS_MGMT_PROT_STATE_EXERCISE_P:        ICLI_PRINTF("%13s", "ExerP");      break;
                case VTSS_EPS_MGMT_PROT_STATE_REVERSE_REQUEST_W: ICLI_PRINTF("%13s", "RevReqW");    break;
                case VTSS_EPS_MGMT_PROT_STATE_REVERSE_REQUEST_P: ICLI_PRINTF("%13s", "RevReqP");    break;
                case VTSS_EPS_MGMT_PROT_STATE_DO_NOT_REVERT:     ICLI_PRINTF("%13s", "DoNotRev");   break;
            }
            switch (state.w_state)
            {
                case VTSS_EPS_MGMT_DEFECT_STATE_OK:    ICLI_PRINTF("%11s", "Ok");    break;
                case VTSS_EPS_MGMT_DEFECT_STATE_SD:    ICLI_PRINTF("%11s", "Sd");    break;
                case VTSS_EPS_MGMT_DEFECT_STATE_SF:    ICLI_PRINTF("%11s", "Sf");    break;
            }
            switch (state.p_state)
            {
                case VTSS_EPS_MGMT_DEFECT_STATE_OK:    ICLI_PRINTF("%11s", "Ok");    break;
                case VTSS_EPS_MGMT_DEFECT_STATE_SD:    ICLI_PRINTF("%11s", "Sd");    break;
                case VTSS_EPS_MGMT_DEFECT_STATE_SF:    ICLI_PRINTF("%11s", "Sf");    break;
            }
            switch (state.tx_aps.request)
            {
                case VTSS_EPS_MGMT_REQUEST_NR:    ICLI_PRINTF("%10s", "NR");    break;
                case VTSS_EPS_MGMT_REQUEST_DNR:   ICLI_PRINTF("%10s", "DNR");   break;
                case VTSS_EPS_MGMT_REQUEST_RR:    ICLI_PRINTF("%10s", "RR");    break;
                case VTSS_EPS_MGMT_REQUEST_EXER:  ICLI_PRINTF("%10s", "EXER");  break;
                case VTSS_EPS_MGMT_REQUEST_WTR:   ICLI_PRINTF("%10s", "WTR");   break;
                case VTSS_EPS_MGMT_REQUEST_MS:    ICLI_PRINTF("%10s", "MS");    break;
                case VTSS_EPS_MGMT_REQUEST_SD:    ICLI_PRINTF("%10s", "SD");    break;
                case VTSS_EPS_MGMT_REQUEST_SF_W:  ICLI_PRINTF("%10s", "SFw");   break;
                case VTSS_EPS_MGMT_REQUEST_FS:    ICLI_PRINTF("%10s", "FS");    break;
                case VTSS_EPS_MGMT_REQUEST_SF_P:  ICLI_PRINTF("%10s", "SFp");   break;
                case VTSS_EPS_MGMT_REQUEST_LO:    ICLI_PRINTF("%10s", "LO");    break;
            }
            ICLI_PRINTF(" %1u %1u", state.tx_aps.re_signal, state.tx_aps.br_signal);
            switch (state.rx_aps.request)
            {
                case VTSS_EPS_MGMT_REQUEST_NR:    ICLI_PRINTF("%10s", "NR");    break;
                case VTSS_EPS_MGMT_REQUEST_DNR:   ICLI_PRINTF("%10s", "DNR");   break;
                case VTSS_EPS_MGMT_REQUEST_RR:    ICLI_PRINTF("%10s", "RR");    break;
                case VTSS_EPS_MGMT_REQUEST_EXER:  ICLI_PRINTF("%10s", "EXER");  break;
                case VTSS_EPS_MGMT_REQUEST_WTR:   ICLI_PRINTF("%10s", "WTR");   break;
                case VTSS_EPS_MGMT_REQUEST_MS:    ICLI_PRINTF("%10s", "MSp");   break;
                case VTSS_EPS_MGMT_REQUEST_SD:    ICLI_PRINTF("%10s", "SD");    break;
                case VTSS_EPS_MGMT_REQUEST_SF_W:  ICLI_PRINTF("%10s", "SFw");   break;
                case VTSS_EPS_MGMT_REQUEST_FS:    ICLI_PRINTF("%10s", "FS");    break;
                case VTSS_EPS_MGMT_REQUEST_SF_P:  ICLI_PRINTF("%10s", "SFp");   break;
                case VTSS_EPS_MGMT_REQUEST_LO:    ICLI_PRINTF("%10s", "LO");    break;
            }
            ICLI_PRINTF(" %1u %1u", state.rx_aps.re_signal, state.rx_aps.br_signal);
            ICLI_PRINTF("%10s", (state.dFop_pm) ? "True" : "False");
            ICLI_PRINTF("%10s", (state.dFop_cm) ? "True" : "False");
            ICLI_PRINTF("%10s", (state.dFop_nr) ? "True" : "False");
            ICLI_PRINTF("%13s", (state.dFop_NoAps) ? "True" : "False");
            ICLI_PRINTF("\n");
        }
    }
    ICLI_PRINTF("\n");

    if (has_detail) {
        ICLI_PRINTF("\n");
        ICLI_PRINTF("EPS Configuration is:\n");
        ICLI_PRINTF("%9s", "Inst");
        ICLI_PRINTF("%8s", "Dom");
        ICLI_PRINTF("%10s", "Archi");
        ICLI_PRINTF("%10s", "Wflow");
        ICLI_PRINTF("%10s", "Pflow");
        ICLI_PRINTF("%9s", "Wmep");
        ICLI_PRINTF("%9s", "Pmep");
        ICLI_PRINTF("%11s", "APSmep");
        ICLI_PRINTF("%11s", "Direct");
        ICLI_PRINTF("%11s", "Revert");
        ICLI_PRINTF("%8s", "Wtr");
        ICLI_PRINTF("%9s", "Hold");
        ICLI_PRINTF("%8s", "Aps");
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
                max = EPS_MGMT_CREATED_MAX;
            }
            else {
                min = inst->u.sr.range[i].min;
                max = inst->u.sr.range[i].max;
            }

            if ((min == 0) || (max == 0)) {
                ICLI_PRINTF("Invalid EPS instance number\n");
                continue;
            }
            for (j=min-1; j<max; ++j) {
                if (((rc = eps_mgmt_conf_get(j, &param, &config, &mep)) != EPS_RC_OK) && (rc != EPS_RC_NOT_CONFIGURED)) {
                    if (rc != EPS_RC_NOT_CREATED) {
                        eps_print_error(session_id, rc);
                    }
                    continue;
                }
                /* Created */
                ICLI_PRINTF("%9u", j+1);
                switch (param.domain)
                {
                    case VTSS_EPS_MGMT_PORT:   ICLI_PRINTF("%8s", "Port");     break;
                    case VTSS_EPS_MGMT_EVC:    ICLI_PRINTF("%8s", "EVC");      break;
                }
                switch (param.architecture)
                {
                    case VTSS_EPS_MGMT_ARCHITECTURE_1P1:    ICLI_PRINTF("%10s", "1plus1");   break;
                    case VTSS_EPS_MGMT_ARCHITECTURE_1F1:    ICLI_PRINTF("%10s", "1for1");    break;
                }
                ICLI_PRINTF("%10u", param.w_flow+1);
                ICLI_PRINTF("%10u", param.p_flow+1);
                if (mep.w_mep != EPS_MEP_INST_INVALID) {
                    ICLI_PRINTF("%9u", mep.w_mep+1);
                } else {
                    ICLI_PRINTF("%9s", "-");
                }
                if (mep.p_mep != EPS_MEP_INST_INVALID) {
                    ICLI_PRINTF("%9u", mep.p_mep+1);
                } else {
                    ICLI_PRINTF("%9s", "-");
                }
                if (mep.aps_mep != EPS_MEP_INST_INVALID) {
                    ICLI_PRINTF("%11u", mep.aps_mep+1);
                } else {
                    ICLI_PRINTF("%11s", "-");
                }
                if (rc == EPS_RC_OK)
                {
                /* Configured */
                    switch (config.directional)
                    {
                        case VTSS_EPS_MGMT_UNIDIRECTIONAL:    ICLI_PRINTF("%11s", "Unidir");   break;
                        case VTSS_EPS_MGMT_BIDIRECTIONAL:     ICLI_PRINTF("%11s", "Bidir");    break;
                    }
                    if (config.revertive)     ICLI_PRINTF("%11s", "True");
                    else                      ICLI_PRINTF("%11s", "False");
                    if (config.restore_timer == 0)               ICLI_PRINTF("%8s", "w0s");
                    else if (config.restore_timer == 10)         ICLI_PRINTF("%8s", "w10s");
                    else if (config.restore_timer == 30)         ICLI_PRINTF("%8s", "w30s");
                    else if (config.restore_timer == 60*5)       ICLI_PRINTF("%8s", "w5m");
                    else if (config.restore_timer == 60*6)       ICLI_PRINTF("%8s", "w6m");
                    else if (config.restore_timer == 60*7)       ICLI_PRINTF("%8s", "w7m");
                    else if (config.restore_timer == 60*8)       ICLI_PRINTF("%8s", "w8m");
                    else if (config.restore_timer == 60*9)       ICLI_PRINTF("%8s", "w9m");
                    else if (config.restore_timer == 60*10)      ICLI_PRINTF("%8s", "w10m");
                    else if (config.restore_timer == 60*11)      ICLI_PRINTF("%8s", "w11m");
                    else if (config.restore_timer == 60*12)      ICLI_PRINTF("%8s", "w12m");
                    ICLI_PRINTF("%9u", config.hold_off_timer);
                    if (config.aps)     ICLI_PRINTF("%8s", "True");
                    else                ICLI_PRINTF("%8s", "False");
                }
                else
                {
                /* Not configured */
                    ICLI_PRINTF("%11s", "xxx");
                    ICLI_PRINTF("%11s", "xxx");
                    ICLI_PRINTF("%8s", "xxx");
                    ICLI_PRINTF("%9s", "xxx");
                    ICLI_PRINTF("%8s", "xxx");
                }
                ICLI_PRINTF("\n");
            }
        }
        ICLI_PRINTF("\n");
    }
}

void eps_clear_eps(i32 session_id, u32 inst)
{
    u32 rc;
    vtss_eps_mgmt_state_t    state;

    if (inst == 0) {
        ICLI_PRINTF("Invalid EPS instance number\n");
        return;
    }
    if ((rc = eps_mgmt_state_get(inst-1, &state)) != EPS_RC_OK) {
        eps_print_error(session_id, rc);
        return;
    }
    if (state.protection_state != VTSS_EPS_MGMT_PROT_STATE_WAIT_TO_RESTORE) {
        ICLI_PRINTF("Not in WTR state\n");
        return;
    }

    if ((rc = eps_mgmt_command_set(inst-1, VTSS_EPS_MGMT_COMMAND_CLEAR)) != EPS_RC_OK) {
        eps_print_error(session_id, rc);
    }
}

void eps_eps(i32 session_id, u32 inst,
             BOOL has_port, BOOL has_evc, BOOL has_1p1, BOOL has_1f1, u32 flow_w, icli_switch_port_range_t port_w, u32 flow_p, icli_switch_port_range_t port_p)
{
    u32 rc;
    vtss_eps_mgmt_create_param_t  param;
    vtss_eps_mgmt_conf_t          config;
    vtss_eps_mgmt_def_conf_t      def_conf;

    eps_mgmt_def_conf_get(&def_conf);

    if (inst == 0) {
        ICLI_PRINTF("Invalid MEP instance number\n");
        return;
    }

    /* eps <inst:uint> domain {port|evc} architecture {1plus1|1for1} work-flow {<flow_w:uint>|<port_w:port_type_id>} protect-flow {<flow_p:uint>|<port_p:port_type_id>} */
    param = def_conf.param;   /* Initialize param */
    config = def_conf.config; /* Initialize config */
    if (has_port || has_evc) {
        param.domain = has_port ? VTSS_EPS_MGMT_PORT :
                        has_evc ? VTSS_EPS_MGMT_EVC : VTSS_EPS_MGMT_PORT;
    }
    if (has_1p1 || has_1f1) {
        param.architecture = has_1f1 ? VTSS_EPS_MGMT_ARCHITECTURE_1F1 :
                              has_1p1 ? VTSS_EPS_MGMT_ARCHITECTURE_1P1 : VTSS_EPS_MGMT_ARCHITECTURE_1F1;
    }
    if (param.domain == VTSS_EPS_MGMT_PORT) {
        param.w_flow = port_w.begin_iport;
        param.p_flow = port_p.begin_iport;
    }
    if (param.domain == VTSS_EPS_MGMT_EVC) {
        param.w_flow = flow_w;
        param.p_flow = flow_p;
    }

    if ((rc = eps_mgmt_instance_create(inst-1, &param)) != EPS_RC_OK) {
        eps_print_error(session_id, rc);
    }
    if ((rc = eps_mgmt_conf_set(inst-1, &config)) != EPS_RC_OK) {
        eps_print_error(session_id, rc);
    }
}

void eps_no_eps(i32 session_id, u32 inst)
{
    u32  rc;
    vtss_eps_mgmt_conf_t          config;
    vtss_eps_mgmt_create_param_t  param;
    eps_mgmt_mep_t                mep;

    if (inst == 0) {
        ICLI_PRINTF("Invalid EPS instance number\n");
        return;
    }
    if (((rc = eps_mgmt_conf_get(inst-1, &param, &config, &mep)) != EPS_RC_OK) && (rc != EPS_RC_NOT_CONFIGURED)) {
        eps_print_error(session_id, rc);
        return;
    }
    if ((rc = eps_mgmt_instance_delete(inst-1)) != EPS_RC_OK) {
        eps_print_error(session_id, rc);
        return;
    }
}

void eps_eps_mep(i32 session_id, u32 inst,
                 u32 mep_w, u32 mep_p, u32 mep_aps)
{
    u32 rc;
    eps_mgmt_mep_t                mep;
    vtss_eps_mgmt_conf_t          config;
    vtss_eps_mgmt_create_param_t  param;

    if (inst == 0) {
        ICLI_PRINTF("Invalid EPS instance number\n");
        return;
    }
    if ((mep_w == 0) || (mep_p == 0) || (mep_aps == 0)) {
        ICLI_PRINTF("Invalid MEP instance number\n");
        return;
    }
    if (((rc = eps_mgmt_conf_get(inst-1, &param, &config, &mep)) != EPS_RC_OK) && (rc != EPS_RC_NOT_CONFIGURED)) {
        eps_print_error(session_id, rc);
        return;
    }

    /* eps <inst:uint> mep work <mep_w:uint> protect <mep_p:uint> aps <mep_aps:uint> */
    mep.w_mep = mep_w-1;
    mep.p_mep = mep_p-1;
    mep.aps_mep = mep_aps-1;

    if ((rc = eps_mgmt_mep_set(inst-1, &mep)) != EPS_RC_OK) {
        eps_print_error(session_id, rc);
    }
}

void eps_eps_revertive(i32 session_id, u32 inst,
                       BOOL has_10s, BOOL has_30s, BOOL has_5m, BOOL has_6m, BOOL has_7m, BOOL has_8m, BOOL has_9m, BOOL has_10m, BOOL has_11m, BOOL has_12m)
{
    u32 rc;
    eps_mgmt_mep_t                mep;
    vtss_eps_mgmt_conf_t          config;
    vtss_eps_mgmt_create_param_t  param;
    vtss_eps_mgmt_def_conf_t      def_conf;

    eps_mgmt_def_conf_get(&def_conf);

    if (inst == 0) {
        ICLI_PRINTF("Invalid EPS instance number\n");
        return;
    }
    if (((rc = eps_mgmt_conf_get(inst-1, &param, &config, &mep)) != EPS_RC_OK) && (rc != EPS_RC_NOT_CONFIGURED)) {
        eps_print_error(session_id, rc);
        return;
    }

    /* eps <inst:uint> revertive {10s|30s|1m|5m|12m} */
    config.revertive = TRUE;
    config.restore_timer = def_conf.config.restore_timer;
    if (has_10s || has_30s || has_5m || has_6m || has_7m || has_8m || has_9m || has_10m || has_11m || has_12m) {
        config.restore_timer = has_10s ? 10 :
                               has_30s ? 10*3 :
                               has_5m ? 60*5 :
                               has_6m ? 60*6 :
                               has_7m ? 60*7 :
                               has_8m ? 60*8 :
                               has_9m ? 60*9 :
                               has_10m ? 60*10 :
                               has_11m ? 60*11 :
                               has_12m ? 60*12 : 10;
    }

    if ((rc = eps_mgmt_conf_set(inst-1, &config)) != EPS_RC_OK) {
        eps_print_error(session_id, rc);
    }
}

void eps_no_eps_revertive(i32 session_id, u32 inst)
{
    u32 rc;
    eps_mgmt_mep_t                mep;
    vtss_eps_mgmt_conf_t          config;
    vtss_eps_mgmt_create_param_t  param;

    if (inst == 0) {
        ICLI_PRINTF("Invalid EPS instance number\n");
        return;
    }
    if (((rc = eps_mgmt_conf_get(inst-1, &param, &config, &mep)) != EPS_RC_OK) && (rc != EPS_RC_NOT_CONFIGURED)) {
        eps_print_error(session_id, rc);
        return;
    }

    /* no eps <inst:uint> revertive */
    config.revertive = FALSE;

    if ((rc = eps_mgmt_conf_set(inst-1, &config)) != EPS_RC_OK) {
        eps_print_error(session_id, rc);
    }
}

void eps_eps_holdoff(i32 session_id, u32 inst,
                     u32 hold)
{
    u32 rc;
    eps_mgmt_mep_t                mep;
    vtss_eps_mgmt_conf_t          config;
    vtss_eps_mgmt_create_param_t  param;
    vtss_eps_mgmt_def_conf_t      def_conf;

    eps_mgmt_def_conf_get(&def_conf);

    if (inst == 0) {
        ICLI_PRINTF("Invalid EPS instance number\n");
        return;
    }
    if (((rc = eps_mgmt_conf_get(inst-1, &param, &config, &mep)) != EPS_RC_OK) && (rc != EPS_RC_NOT_CONFIGURED)) {
        eps_print_error(session_id, rc);
        return;
    }

    /* eps <inst:uint> holdoff <hold:uint> */
    config.hold_off_timer = hold;

    if ((rc = eps_mgmt_conf_set(inst-1, &config)) != EPS_RC_OK) {
        eps_print_error(session_id, rc);
    }
}

void eps_no_eps_holdoff(i32 session_id, u32 inst)
{
    u32 rc;
    eps_mgmt_mep_t                mep;
    vtss_eps_mgmt_conf_t          config;
    vtss_eps_mgmt_create_param_t  param;

    if (inst == 0) {
        ICLI_PRINTF("Invalid EPS instance number\n");
        return;
    }
    if (((rc = eps_mgmt_conf_get(inst-1, &param, &config, &mep)) != EPS_RC_OK) && (rc != EPS_RC_NOT_CONFIGURED)) {
        eps_print_error(session_id, rc);
        return;
    }

    /* no eps <inst:uint> holdoff */
    config.hold_off_timer = VTSS_EPS_MGMT_HOFF_OFF;

    if ((rc = eps_mgmt_conf_set(inst-1, &config)) != EPS_RC_OK) {
        eps_print_error(session_id, rc);
    }
}

void eps_eps_1p1(i32 session_id, u32 inst,
                 BOOL has_bidirectional, BOOL has_unidirectional, BOOL has_aps)
{
    u32 rc;
    eps_mgmt_mep_t                mep;
    vtss_eps_mgmt_conf_t          config;
    vtss_eps_mgmt_create_param_t  param;
    vtss_eps_mgmt_def_conf_t      def_conf;

    eps_mgmt_def_conf_get(&def_conf);

    if (inst == 0) {
        ICLI_PRINTF("Invalid EPS instance number\n");
        return;
    }
    if (((rc = eps_mgmt_conf_get(inst-1, &param, &config, &mep)) != EPS_RC_OK) && (rc != EPS_RC_NOT_CONFIGURED)) {
        eps_print_error(session_id, rc);
        return;
    }

    /* eps <inst:uint> 1plus1 {bidirectional | {unidirectional [aps]}} */
    config.directional = def_conf.config.directional;
    config.aps = def_conf.config.aps;
    if (has_bidirectional || has_unidirectional) {
        config.directional = has_unidirectional ? VTSS_EPS_MGMT_UNIDIRECTIONAL :
                             has_bidirectional ? VTSS_EPS_MGMT_BIDIRECTIONAL : VTSS_EPS_MGMT_UNIDIRECTIONAL;
    }
    if (config.directional == VTSS_EPS_MGMT_UNIDIRECTIONAL) {
        config.aps = has_aps;
    }

    if ((rc = eps_mgmt_conf_set(inst-1, &config)) != EPS_RC_OK) {
        eps_print_error(session_id, rc);
    }
}

void eps_eps_command(i32 session_id, u32 inst,
                     BOOL has_lockout, BOOL has_forced, BOOL has_manualp, BOOL has_manualw, BOOL has_exercise, BOOL has_freeze, BOOL has_lockoutlocal)
{
    u32 rc;
    eps_mgmt_mep_t                mep;
    vtss_eps_mgmt_conf_t          config;
    vtss_eps_mgmt_create_param_t  param;
    vtss_eps_mgmt_def_conf_t      def_conf;
    vtss_eps_mgmt_command_t       command;

    eps_mgmt_def_conf_get(&def_conf);

    if (inst == 0) {
        ICLI_PRINTF("Invalid EPS instance number\n");
        return;
    }
    if (((rc = eps_mgmt_conf_get(inst-1, &param, &config, &mep)) != EPS_RC_OK) && (rc != EPS_RC_NOT_CONFIGURED)) {
        eps_print_error(session_id, rc);
        return;
    }

    /* eps <inst:uint> command {lockout|forced|manualp|manualw|exercise|freeze|lockoutlocal} */
    command = def_conf.command;
    if (has_lockout || has_forced || has_manualp ||has_manualw || has_exercise || has_freeze || has_lockoutlocal) {
        command = has_lockout ? VTSS_EPS_MGMT_COMMAND_LOCK_OUT :
                  has_forced ? VTSS_EPS_MGMT_COMMAND_FORCED_SWITCH :
                  has_manualp ? VTSS_EPS_MGMT_COMMAND_MANUEL_SWITCH_P :
                  has_manualw ? VTSS_EPS_MGMT_COMMAND_MANUEL_SWITCH_W :
                  has_exercise ? VTSS_EPS_MGMT_COMMAND_EXERCISE :
                  has_freeze ? VTSS_EPS_MGMT_COMMAND_FREEZE :
                  has_lockoutlocal ? VTSS_EPS_MGMT_COMMAND_LOCK_OUT_LOCAL : VTSS_EPS_MGMT_COMMAND_EXERCISE;
    }

    if ((rc = eps_mgmt_command_set(inst-1, command)) != EPS_RC_OK) {
        eps_print_error(session_id, rc);
    }
}

void eps_no_eps_command(i32 session_id, u32 inst)
{
    u32 rc;
    eps_mgmt_mep_t                mep;
    vtss_eps_mgmt_conf_t          config;
    vtss_eps_mgmt_create_param_t  param;
    vtss_eps_mgmt_command_t       command;

    if (inst == 0) {
        ICLI_PRINTF("Invalid EPS instance number\n");
        return;
    }
    if (((rc = eps_mgmt_conf_get(inst-1, &param, &config, &mep)) != EPS_RC_OK) && (rc != EPS_RC_NOT_CONFIGURED)) {
        eps_print_error(session_id, rc);
        return;
    }

    /* eps <inst:uint> command {lockout|forced|manualp|manualw|exercise|freeze|lockoutlocal} */
    command = VTSS_EPS_MGMT_COMMAND_CLEAR;

    if ((rc = eps_mgmt_command_set(inst-1, command)) != EPS_RC_OK) {
        eps_print_error(session_id, rc);
    }
}

/***************************************************************************/
/* ICFG callback functions */
/****************************************************************************/
#ifdef VTSS_SW_OPTION_ICFG
static vtss_rc eps_icfg_conf(const vtss_icfg_query_request_t  *req,
                             vtss_icfg_query_result_t         *result)
{
    u32 i, rc;
    eps_mgmt_mep_t                mep;
    vtss_eps_mgmt_conf_t          config;
    vtss_eps_mgmt_create_param_t  param;
    vtss_eps_mgmt_command_t       command;
    vtss_eps_mgmt_def_conf_t      def_conf;
    vtss_icfg_conf_print_t        conf_print;
    char                          buf[ICLI_PORTING_STR_BUF_SIZE];

    vtss_icfg_conf_print_init(&conf_print);

    // Get default configuration
    eps_mgmt_def_conf_get(&def_conf);
    config = def_conf.config;
    param = def_conf.param;
    command = def_conf.command;
    mep.w_mep = EPS_MEP_INST_INVALID;
    mep.p_mep = EPS_MEP_INST_INVALID;
    mep.aps_mep = EPS_MEP_INST_INVALID;

    for (i=0; i<EPS_MGMT_CREATED_MAX; ++i) {
        if (((rc = eps_mgmt_conf_get(i, &param, &config, &mep)) != EPS_RC_OK) && (rc != EPS_RC_NOT_CONFIGURED)) {
            continue;
        }
        /* eps <inst:uint> domain {port|evc} architecture {1plus1|1for1} work-flow {<flow_w:uint>|<port_w:port_type_id>} protect-flow {<flow_p:uint>|<port_p:port_type_id>} */
        VTSS_RC(vtss_icfg_printf(result, "eps %u", i+1));
        VTSS_RC(vtss_icfg_printf(result, " domain %s", param.domain == VTSS_EPS_MGMT_PORT ? "port" : "evc"));
        VTSS_RC(vtss_icfg_printf(result, " architecture %s", param.architecture == VTSS_EPS_MGMT_ARCHITECTURE_1P1 ? "1plus1" : "1for1"));
        if (param.domain == VTSS_EPS_MGMT_PORT) {
            VTSS_RC(vtss_icfg_printf(result, " work-flow %s", icli_port_info_txt(VTSS_USID_START, iport2uport(param.w_flow), buf)));
            VTSS_RC(vtss_icfg_printf(result, " protect-flow %s", icli_port_info_txt(VTSS_USID_START, iport2uport(param.p_flow), buf)));
        }
        if (param.domain == VTSS_EPS_MGMT_EVC) {
            VTSS_RC(vtss_icfg_printf(result, " work-flow %u", param.w_flow+1));
            VTSS_RC(vtss_icfg_printf(result, " protect-flow %u", param.p_flow+1));
        }
        VTSS_RC(vtss_icfg_printf(result, "\n"));
        /* eps <inst:uint> mep-work <mep_w:uint> mep-protect <mep_p:uint> mep-aps <mep_aps:uint> */
        VTSS_RC(vtss_icfg_printf(result, "eps %u", i+1));
        VTSS_RC(vtss_icfg_printf(result, " mep-work %u", mep.w_mep+1));
        VTSS_RC(vtss_icfg_printf(result, " mep-protect %u", mep.p_mep+1));
        VTSS_RC(vtss_icfg_printf(result, " mep-aps %u", mep.aps_mep+1));
        VTSS_RC(vtss_icfg_printf(result, "\n"));
        /* eps <inst:uint> revertive {10s|30s|1m|5m|12m} */
        if (config.revertive && (config.restore_timer != 0)) {
            VTSS_RC(vtss_icfg_printf(result, "eps %u revertive", i+1));
            VTSS_RC(vtss_icfg_printf(result, " %s", config.restore_timer == 10 ? "10s" :
                                                    config.restore_timer == 10*3 ? "30s" :
                                                    config.restore_timer == 60*5 ? "5m" :
                                                    config.restore_timer == 60*6 ? "6m" :
                                                    config.restore_timer == 60*7 ? "7m" :
                                                    config.restore_timer == 60*8 ? "8m" :
                                                    config.restore_timer == 60*9 ? "9m" :
                                                    config.restore_timer == 60*10 ? "10m" :
                                                    config.restore_timer == 60 *11? "11m" :
                                                    config.restore_timer == 60*12 ? "12m" : "10s"));
            VTSS_RC(vtss_icfg_printf(result, "\n"));
        }
        /* eps <inst:uint> holdoff <hold:uint> */
        if (config.hold_off_timer != VTSS_EPS_MGMT_HOFF_OFF) {
            VTSS_RC(vtss_icfg_printf(result, "eps %u holdoff", i+1));
            VTSS_RC(vtss_icfg_printf(result, " %u", config.hold_off_timer));
            VTSS_RC(vtss_icfg_printf(result, "\n"));
        }
        /* eps <inst:uint> 1plus1 {bidirectional | {unidirectional [aps]}} */
        if (param.architecture == VTSS_EPS_MGMT_ARCHITECTURE_1P1) {
            VTSS_RC(vtss_icfg_printf(result, "eps %u 1plus1", i+1));
            VTSS_RC(vtss_icfg_printf(result, " %s", config.directional == VTSS_EPS_MGMT_UNIDIRECTIONAL ? "unidirectional" : "bidirectional"));
            if (config.directional == VTSS_EPS_MGMT_UNIDIRECTIONAL) {
                if ((config.aps != def_conf.config.aps) || req->all_defaults) {
                    VTSS_RC(vtss_icfg_printf(result, " %s", config.aps ? "aps" : ""));
                }
            }
            VTSS_RC(vtss_icfg_printf(result, "\n"));
        }
        /* eps <inst:uint> command {lockout|forced|manualp|manualw|exercise|freeze|lockoutlocal} */
        if ((eps_mgmt_command_get(i, &command)) == EPS_RC_OK) {
            if (command != VTSS_EPS_MGMT_COMMAND_NONE) {
                VTSS_RC(vtss_icfg_printf(result, "eps %u command", i+1));
                VTSS_RC(vtss_icfg_printf(result, " %s", command == VTSS_EPS_MGMT_COMMAND_LOCK_OUT ? "lockout" :
                                                        command == VTSS_EPS_MGMT_COMMAND_FORCED_SWITCH ? "forced" :
                                                        command == VTSS_EPS_MGMT_COMMAND_MANUEL_SWITCH_P ? "manualp" :
                                                        command == VTSS_EPS_MGMT_COMMAND_MANUEL_SWITCH_W ? "manualw" :
                                                        command == VTSS_EPS_MGMT_COMMAND_EXERCISE ? "exercise" :
                                                        command == VTSS_EPS_MGMT_COMMAND_FREEZE ? "freeze" :
                                                        command == VTSS_EPS_MGMT_COMMAND_LOCK_OUT_LOCAL ? "lockoutlocal" : "exercise"));
                VTSS_RC(vtss_icfg_printf(result, "\n"));
            }
        }
    }

    return VTSS_RC_OK;
}


/* ICFG Initialization function */
vtss_rc eps_icfg_init(void)
{
  VTSS_RC(vtss_icfg_query_register(VTSS_ICFG_EPS_GLOBAL_CONF, "eps", eps_icfg_conf));
  return VTSS_RC_OK;
}
#endif // VTSS_SW_OPTION_ICFG
/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
