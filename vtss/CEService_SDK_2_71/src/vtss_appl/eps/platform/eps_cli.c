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

#include "main.h"
#include "cli.h"
#include "cli_api.h"
#include "eps_cli.h"
#include "vtss_module_id.h"
#include "eps_api.h"
#include "cli_trace_def.h"


typedef struct
{
    vtss_eps_mgmt_architecture_t  architecture;
    vtss_eps_mgmt_domain_t        eps_domain;
    BOOL                          directional;
    u32                           w_flow;
    u32                           p_flow;
    u32                           w_mep;
    u32                           p_mep;
    u32                           aps_mep;
    vtss_eps_mgmt_command_t       eps_command;
    BOOL                          aps;
    BOOL                          revertive;
    u32                           restore_timer;
    u32                           hold_off_timer;
    u32                           instance;
    u32                           eps_enable;
} eps_cli_req_t;


void eps_cli_req_init(void)
{
    /* register the size required for eps req. structure */
    cli_req_size_register(sizeof(eps_cli_req_t));
}
static void eps_print_error(vtss_rc rc)
{
    switch (rc)
    {
        case EPS_RC_NOT_CREATED:            CPRINTF("EPS instance not created\n"); break;
        case EPS_RC_CREATED:                CPRINTF("EPS instance already created\n"); break;
        case EPS_RC_INVALID_PARAMETER:      CPRINTF("Invalid parameter\n"); break;
        case EPS_RC_NOT_CONFIGURED:         CPRINTF("EPS NOT created\n"); break;
        case EPS_RC_ARCHITECTURE:           CPRINTF("Only 1+1 in port domain\n"); break;
        case EPS_RC_W_P_FLOW_EQUAL:         CPRINTF("Working and protecting flow is equal\n"); break;
        case EPS_RC_W_P_SSF_MEP_EQUAL:      CPRINTF("Working and protecting SF MEP is equal\n"); break;
        case EPS_RC_INVALID_APS_MEP:        CPRINTF("Invalid APS MEP\n"); break;
        case EPS_RC_INVALID_W_MEP:          CPRINTF("Invalid working SF MEP\n"); break;
        case EPS_RC_INVALID_P_MEP:          CPRINTF("Invalid protecting SF MEP\n"); break;
        case EPS_RC_WORKING_USED:           CPRINTF("Working flow is used by other instance\n"); break;
        case EPS_RC_PROTECTING_USED:        CPRINTF("Protecting flow is used by other instance\n"); break;
        default:                            CPRINTF("Unknown error returned from EPS\n"); break;
    }
}

static void cli_cmd_id_eps_config(cli_req_t *req)
{
    u32                          rc, i;
    vtss_eps_mgmt_conf_t         config;
    vtss_eps_mgmt_create_param_t param;
    eps_mgmt_mep_t               mep;
    eps_cli_req_t                *eps_req = NULL;

    eps_req = req->module_req;

    if (req->set)
    {
        config.directional = eps_req->directional;
        config.aps = eps_req->aps;
        config.revertive = eps_req->revertive;
        config.restore_timer = eps_req->restore_timer;
        config.hold_off_timer = eps_req->hold_off_timer;

        if ((rc = eps_mgmt_conf_set(eps_req->instance, &config)) != EPS_RC_OK)
            eps_print_error(rc);
    }
    else
    {
        CPRINTF("\n");
        CPRINTF("Configuration is:\n");
        CPRINTF("%9s", "Inst");
        CPRINTF("%8s", "Dom");
        CPRINTF("%10s", "Archi");
        CPRINTF("%10s", "Wflow");
        CPRINTF("%10s", "Pflow");
        CPRINTF("%9s", "Wmep");
        CPRINTF("%9s", "Pmep");
        CPRINTF("%11s", "APSmep");
        CPRINTF("%11s", "Direct");
        CPRINTF("%11s", "Revert");
        CPRINTF("%8s", "Wtr");
        CPRINTF("%9s", "Hold");
        CPRINTF("%8s", "Aps");
        CPRINTF("\n");
        for (i=0; i<EPS_MGMT_CREATED_MAX; ++i)
        {
            if ((eps_req->instance != 0xFFFFFFFF) && (eps_req->instance != i))      continue;
            if ((rc = eps_mgmt_conf_get(i, &param, &config, &mep)) == EPS_RC_INVALID_PARAMETER)
                eps_print_error(rc);
            else
            {
                if ((rc == EPS_RC_OK) || (rc == EPS_RC_NOT_CONFIGURED))
                {
                /* Created */
                    CPRINTF("%9u", i+1);
                    switch (param.domain)
                    {
                        case VTSS_EPS_MGMT_PORT:       CPRINTF("%8s", "Port");     break;
//                        case VTSS_EPS_MGMT_PATH:       CPRINTF("%8s", "Path");     break;
                        case VTSS_EPS_MGMT_EVC:        CPRINTF("%8s", "EVC");     break;
//                        case VTSS_EPS_MGMT_MPLS:       CPRINTF("%8s", "Mpls");     break;
                    }
                    switch (param.architecture)
                    {
                        case VTSS_EPS_MGMT_ARCHITECTURE_1P1:    CPRINTF("%10s", "1plus1");   break;
                        case VTSS_EPS_MGMT_ARCHITECTURE_1F1:    CPRINTF("%10s", "1for1");    break;
                    }
                    CPRINTF("%10u", param.w_flow+1);
                    CPRINTF("%10u", param.p_flow+1);
                    CPRINTF("%9u", mep.w_mep+1);
                    CPRINTF("%9u", mep.p_mep+1);
                    CPRINTF("%11u", mep.aps_mep+1);

                    if (rc == EPS_RC_OK)
                    {
                    /* Configured */
                        switch (config.directional)
                        {
                            case VTSS_EPS_MGMT_UNIDIRECTIONAL:    CPRINTF("%11s", "Unidir");   break;
                            case VTSS_EPS_MGMT_BIDIRECTIONAL:     CPRINTF("%11s", "Bidir");    break;
                        }
                        if (config.revertive)     CPRINTF("%11s", "True");
                        else                      CPRINTF("%11s", "False");
                        if (config.restore_timer == 0)               CPRINTF("%8s", "w0s");
                        else if (config.restore_timer == 10)         CPRINTF("%8s", "w10s");
                        else if (config.restore_timer == 30)         CPRINTF("%8s", "w30s");
                        else if (config.restore_timer == 60*1)       CPRINTF("%8s", "w1m");
                        else if (config.restore_timer == 60*5)       CPRINTF("%8s", "w5m");
                        else if (config.restore_timer == 60*12)      CPRINTF("%8s", "w12m");
                        if (config.hold_off_timer == 0)              CPRINTF("%9s", "h0s");
                        else if (config.hold_off_timer == 1)         CPRINTF("%9s", "h100ms");
                        else if (config.hold_off_timer == 5)         CPRINTF("%9s", "h500ms");
                        else if (config.hold_off_timer == 10)        CPRINTF("%9s", "h1s");
                        else if (config.hold_off_timer == 20)        CPRINTF("%9s", "h2s");
                        else if (config.hold_off_timer == 50)        CPRINTF("%9s", "h5s");
                        else if (config.hold_off_timer == 100)       CPRINTF("%9s", "h10s");
                        if (config.aps)     CPRINTF("%8s", "True");
                        else                CPRINTF("%8s", "False");
                    }
                    else
                    {
                    /* Not configured */
                        CPRINTF("%11s", "xxx");
                        CPRINTF("%11s", "xxx");
                        CPRINTF("%8s", "xxx");
                        CPRINTF("%9s", "xxx");
                        CPRINTF("%8s", "xxx");
                    }
                    CPRINTF("\n");
                }
            }
        }
        CPRINTF("\n");
    }
}

static void cli_cmd_id_eps_command(cli_req_t *req)
{
    u32                       rc, i;
    vtss_eps_mgmt_command_t   command;
    eps_cli_req_t * eps_req = NULL;

    eps_req = req->module_req;

    if (req->set)
    {
        if ((rc = eps_mgmt_command_set(eps_req->instance, eps_req->eps_command)) != EPS_RC_OK)
            eps_print_error(rc);
    }
    else
    {
        CPRINTF("\n");
        CPRINTF("Commands:\n");
        CPRINTF("%9s", "Inst");
        CPRINTF("\n");
        for (i=0; i<EPS_MGMT_CREATED_MAX; ++i)
        {
            if ((eps_req->instance != 0xFFFFFFFF) && (eps_req->instance != i))      continue;
            if ((rc = eps_mgmt_command_get(i, &command)) == EPS_RC_INVALID_PARAMETER)
                eps_print_error(rc);
            else
            {
                if (rc == EPS_RC_OK)
                {
                    CPRINTF("%9u", i+1);
                    switch (command)
                    {
                        case VTSS_EPS_MGMT_COMMAND_NONE:             CPRINTF("%17s", "none");           break;
                        case VTSS_EPS_MGMT_COMMAND_CLEAR:            CPRINTF("%17s", "clear");          break;
                        case VTSS_EPS_MGMT_COMMAND_LOCK_OUT:         CPRINTF("%17s", "lockOut");        break;
                        case VTSS_EPS_MGMT_COMMAND_FORCED_SWITCH:    CPRINTF("%17s", "forced");         break;
                        case VTSS_EPS_MGMT_COMMAND_MANUEL_SWITCH_P:  CPRINTF("%17s", "manualp");        break;
                        case VTSS_EPS_MGMT_COMMAND_MANUEL_SWITCH_W:  CPRINTF("%17s", "manualw");        break;
                        case VTSS_EPS_MGMT_COMMAND_EXERCISE:         CPRINTF("%17s", "exercise");       break;
                        case VTSS_EPS_MGMT_COMMAND_FREEZE:           CPRINTF("%17s", "freeze");         break;
                        case VTSS_EPS_MGMT_COMMAND_LOCK_OUT_LOCAL:   CPRINTF("%17s", "lockOutLocal");   break;
                    }
                    CPRINTF("\n");
                }
            }
        }
        CPRINTF("\n");
    }
}

static void cli_cmd_id_eps_create(cli_req_t *req)
{
    u32                           rc;
    vtss_eps_mgmt_create_param_t  param;
    eps_mgmt_mep_t                mep;
    eps_cli_req_t                 *eps_req = NULL;

    eps_req = req->module_req;

    if (req->set)
    {
        if (eps_req->eps_enable)
        {
            param.architecture = eps_req->architecture;
            param.domain = eps_req->eps_domain;
            param.w_flow = eps_req->w_flow;
            param.p_flow = eps_req->p_flow;
            mep.w_mep = eps_req->w_mep;
            mep.p_mep = eps_req->p_mep;
            mep.aps_mep = eps_req->aps_mep;

            if ((rc = eps_mgmt_instance_create(eps_req->instance, &param)) != EPS_RC_OK)
                eps_print_error(rc);
            if ((rc = eps_mgmt_mep_set(eps_req->instance, &mep)) != EPS_RC_OK)
                eps_print_error(rc);
        }
        else
            if ((rc = eps_mgmt_instance_delete(eps_req->instance)) != EPS_RC_OK)
                eps_print_error(rc);
    }
}

static void cli_cmd_id_eps_state(cli_req_t *req )
{
    u32                     rc, i;
    vtss_eps_mgmt_state_t   state;
    eps_cli_req_t           *eps_req = req->module_req;

    CPRINTF("\n");
    CPRINTF("EPS state is:\n");
    CPRINTF("%9s", "Inst");
    CPRINTF("%13s", "State");
    CPRINTF("%11s", "Wstate");
    CPRINTF("%11s", "Pstate");
    CPRINTF("%14s", "TxAps r b");
    CPRINTF("%14s", "RxAps r b");
    CPRINTF("%10s", "FopPm");
    CPRINTF("%10s", "FopCm");
    CPRINTF("%10s", "FopNr");
    CPRINTF("%13s", "FopNoAps");
    CPRINTF("\n");
    for (i=0; i<EPS_MGMT_CREATED_MAX; ++i)
    {
        if ((eps_req->instance != 0xFFFFFFFF) && (eps_req->instance != i))      continue;
        if ((rc = eps_mgmt_state_get(i, &state)) == EPS_RC_INVALID_PARAMETER)
            eps_print_error(rc);
        else
        {
            if (rc == EPS_RC_OK)
            {
                CPRINTF("%9u", i+1);
                switch (state.protection_state)
                {
                    case VTSS_EPS_MGMT_PROT_STATE_DISABLED:          CPRINTF("%13s", "Disable");    break;
                    case VTSS_EPS_MGMT_PROT_STATE_NO_REQUEST_W:      CPRINTF("%13s", "NoReqW");     break;
                    case VTSS_EPS_MGMT_PROT_STATE_NO_REQUEST_P:      CPRINTF("%13s", "NoReqP");     break;
                    case VTSS_EPS_MGMT_PROT_STATE_LOCKOUT:           CPRINTF("%13s", "Lockout");    break;
                    case VTSS_EPS_MGMT_PROT_STATE_FORCED_SWITCH:     CPRINTF("%13s", "Forced");     break;
                    case VTSS_EPS_MGMT_PROT_STATE_SIGNAL_FAIL_W:     CPRINTF("%13s", "SfW");        break;
                    case VTSS_EPS_MGMT_PROT_STATE_SIGNAL_FAIL_P:     CPRINTF("%13s", "SfP");        break;
                    case VTSS_EPS_MGMT_PROT_STATE_MANUEL_SWITCH_W:   CPRINTF("%13s", "ManualW");    break;
                    case VTSS_EPS_MGMT_PROT_STATE_MANUEL_SWITCH_P:   CPRINTF("%13s", "ManualP");    break;
                    case VTSS_EPS_MGMT_PROT_STATE_WAIT_TO_RESTORE:   CPRINTF("%13s", "Wtr");        break;
                    case VTSS_EPS_MGMT_PROT_STATE_EXERCISE_W:        CPRINTF("%13s", "ExerW");      break;
                    case VTSS_EPS_MGMT_PROT_STATE_EXERCISE_P:        CPRINTF("%13s", "ExerP");      break;
                    case VTSS_EPS_MGMT_PROT_STATE_REVERSE_REQUEST_W: CPRINTF("%13s", "RevReqW");    break;
                    case VTSS_EPS_MGMT_PROT_STATE_REVERSE_REQUEST_P: CPRINTF("%13s", "RevReqP");    break;
                    case VTSS_EPS_MGMT_PROT_STATE_DO_NOT_REVERT:     CPRINTF("%13s", "DoNotRev");   break;
                }
                switch (state.w_state)
                {
                    case VTSS_EPS_MGMT_DEFECT_STATE_OK:    CPRINTF("%11s", "Ok");    break;
                    case VTSS_EPS_MGMT_DEFECT_STATE_SD:    CPRINTF("%11s", "Sd");    break;
                    case VTSS_EPS_MGMT_DEFECT_STATE_SF:    CPRINTF("%11s", "Sf");    break;
                }
                switch (state.p_state)
                {
                    case VTSS_EPS_MGMT_DEFECT_STATE_OK:    CPRINTF("%11s", "Ok");    break;
                    case VTSS_EPS_MGMT_DEFECT_STATE_SD:    CPRINTF("%11s", "Sd");    break;
                    case VTSS_EPS_MGMT_DEFECT_STATE_SF:    CPRINTF("%11s", "Sf");    break;
                }
                switch (state.tx_aps.request)
                {
                    case VTSS_EPS_MGMT_REQUEST_NR:    CPRINTF("%10s", "NR");    break;
                    case VTSS_EPS_MGMT_REQUEST_DNR:   CPRINTF("%10s", "DNR");   break;
                    case VTSS_EPS_MGMT_REQUEST_RR:    CPRINTF("%10s", "RR");    break;
                    case VTSS_EPS_MGMT_REQUEST_EXER:  CPRINTF("%10s", "EXER");  break;
                    case VTSS_EPS_MGMT_REQUEST_WTR:   CPRINTF("%10s", "WTR");   break;
                    case VTSS_EPS_MGMT_REQUEST_MS:    CPRINTF("%10s", "MS");    break;
                    case VTSS_EPS_MGMT_REQUEST_SD:    CPRINTF("%10s", "SD");    break;
                    case VTSS_EPS_MGMT_REQUEST_SF_W:  CPRINTF("%10s", "SFw");   break;
                    case VTSS_EPS_MGMT_REQUEST_FS:    CPRINTF("%10s", "FS");    break;
                    case VTSS_EPS_MGMT_REQUEST_SF_P:  CPRINTF("%10s", "SFp");   break;
                    case VTSS_EPS_MGMT_REQUEST_LO:    CPRINTF("%10s", "LO");    break;
                }
                CPRINTF(" %1u %1u", state.tx_aps.re_signal, state.tx_aps.br_signal);
                switch (state.rx_aps.request)
                {
                    case VTSS_EPS_MGMT_REQUEST_NR:    CPRINTF("%10s", "NR");    break;
                    case VTSS_EPS_MGMT_REQUEST_DNR:   CPRINTF("%10s", "DNR");   break;
                    case VTSS_EPS_MGMT_REQUEST_RR:    CPRINTF("%10s", "RR");    break;
                    case VTSS_EPS_MGMT_REQUEST_EXER:  CPRINTF("%10s", "EXER");  break;
                    case VTSS_EPS_MGMT_REQUEST_WTR:   CPRINTF("%10s", "WTR");   break;
                    case VTSS_EPS_MGMT_REQUEST_MS:    CPRINTF("%10s", "MSp");   break;
                    case VTSS_EPS_MGMT_REQUEST_SD:    CPRINTF("%10s", "SD");    break;
                    case VTSS_EPS_MGMT_REQUEST_SF_W:  CPRINTF("%10s", "SFw");   break;
                    case VTSS_EPS_MGMT_REQUEST_FS:    CPRINTF("%10s", "FS");    break;
                    case VTSS_EPS_MGMT_REQUEST_SF_P:  CPRINTF("%10s", "SFp");   break;
                    case VTSS_EPS_MGMT_REQUEST_LO:    CPRINTF("%10s", "LO");    break;
                }
                CPRINTF(" %1u %1u", state.rx_aps.re_signal, state.rx_aps.br_signal);
                CPRINTF("%10s", (state.dFop_pm) ? "True" : "False");
                CPRINTF("%10s", (state.dFop_cm) ? "True" : "False");
                CPRINTF("%10s", (state.dFop_nr) ? "True" : "False");
                CPRINTF("%13s", (state.dFop_NoAps) ? "True" : "False");
                CPRINTF("\n");
            }
        }
    }
    CPRINTF("\n");

}

static int32_t cli_parse_eps_wtr_time(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    eps_cli_req_t  * eps_req = NULL;
    char *found = cli_parse_find(cmd, stx);

    eps_req = req->module_req;
    if(!found)      return 1;
    else if(!strncmp(found, "w0s", 3))       eps_req->restore_timer = 0;
    else if(!strncmp(found, "w10s", 4))      eps_req->restore_timer = 10;
    else if(!strncmp(found, "w30s", 4))      eps_req->restore_timer = 30;
    else if(!strncmp(found, "w1m", 3))       eps_req->restore_timer = 60*1;
    else if(!strncmp(found, "w5m", 3))       eps_req->restore_timer = 60*5;
    else if(!strncmp(found, "w12m", 3))      eps_req->restore_timer = 60*12;
    else return 1;

    return 0;

}

static int cli_parse_architecture(char *cmd, char *cmd2, char *stx, char *cmd_org,
                               cli_req_t *req)
{
    eps_cli_req_t  * eps_req = NULL;
    char *found = cli_parse_find(cmd, stx);

    eps_req = req->module_req;

    if(!found)      return 1;

    else if(!strncmp(found, "1p1", 3))     eps_req->architecture = VTSS_EPS_MGMT_ARCHITECTURE_1P1;
    else if(!strncmp(found, "1f1", 3))     eps_req->architecture = VTSS_EPS_MGMT_ARCHITECTURE_1F1;
    else return 1;

    return 0;
} /* cli_parse_architecture */


static int cli_parse_bidirectional(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char *found = cli_parse_find(cmd, stx);
    eps_cli_req_t * eps_req = NULL;

    eps_req = req->module_req;

    if(!found)      return 1;

    else if(!strncmp(found, "unidir", 6))     eps_req->directional = VTSS_EPS_MGMT_UNIDIRECTIONAL;
    else if(!strncmp(found, "bidir", 5))      eps_req->directional = VTSS_EPS_MGMT_BIDIRECTIONAL;
    else return 1;

    return 0;
} /* cli_parse_bidirectional */

static int32_t cli_parse_working_flow (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    ulong value = 0;
    ulong error = 0;
    eps_cli_req_t  * eps_req = NULL;
    eps_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 1, 0xFFFF);
    eps_req->w_flow = value - 1;
    return (error);
}

static int32_t cli_parse_protection_flow (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    ulong value = 0;
    ulong error = 0;
    eps_cli_req_t  * eps_req = NULL;
    eps_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 1, 0xFFFF);
    eps_req->p_flow = value -1;
    return (error);
}
static int32_t cli_parse_working_mep (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    ulong value = 0;
    ulong error = 0;
    eps_cli_req_t  * eps_req = NULL;
    eps_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 1, 0xFFFF);
    eps_req->w_mep = value - 1;
    return (error);
}

static int32_t cli_parse_protection_mep (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    ulong value = 0;
    ulong error = 0;
    eps_cli_req_t  * eps_req = NULL;
    eps_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 1, 0xFFFF);
    eps_req->p_mep = value - 1;
    return (error);
}

static int32_t cli_parse_aps_mep (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    ulong value = 0;
    ulong error = 0;
    eps_cli_req_t  * eps_req = NULL;
    eps_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 1, 0xFFFF);
    eps_req->aps_mep = value - 1;
    return (error);
}

static int32_t cli_parse_eps_instance (char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    eps_cli_req_t * eps_req = NULL;
    ulong error = 0;
    ulong value;

    req->parm_parsed = 1;
    eps_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 1, 0xFFFF);
    eps_req->instance = value-1;
    return (error);
}

static int cli_parse_aps(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char *found = cli_parse_find(cmd, stx);
    eps_cli_req_t  * eps_req = NULL;
    eps_req = req->module_req;

    if(!found)      return 1;
    else if(!strncmp(found, "aps", 3))       eps_req->aps = true;
    else if(!strncmp(found, "noaps", 5))     eps_req->aps = false;
    else return 1;

    return 0;
} /* cli_parse_aps */

static int cli_parse_eps_hold_time(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char *found = cli_parse_find(cmd, stx);
    eps_cli_req_t  * eps_req = NULL;
    eps_req = req->module_req;

    if(!found)      return 1;
    else if(!strncmp(found, "h0s", 3))       eps_req->hold_off_timer = 0;
    else if(!strncmp(found, "h100ms", 6))    eps_req->hold_off_timer = 1;
    else if(!strncmp(found, "h500ms", 6))    eps_req->hold_off_timer = 5;
    else if(!strncmp(found, "h1s", 3))       eps_req->hold_off_timer = 10;
    else if(!strncmp(found, "h2s", 3))       eps_req->hold_off_timer = 20;
    else if(!strncmp(found, "h5s", 3))       eps_req->hold_off_timer = 50;
    else if(!strncmp(found, "h10s", 4))      eps_req->hold_off_timer = 100;
    else return 1;

    return 0;
} /* cli_parse_wtr_time */

static int cli_parse_eps_command(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char *found = cli_parse_find(cmd, stx);
    eps_cli_req_t  * eps_req = NULL;
    eps_req = req->module_req;

    if(!found)      return 1;
    else if(!strncmp(found, "clear", 5))          eps_req->eps_command = VTSS_EPS_MGMT_COMMAND_CLEAR;
    else if(!strncmp(found, "lockoutlocal", 12))  eps_req->eps_command = VTSS_EPS_MGMT_COMMAND_LOCK_OUT_LOCAL;
    else if(!strncmp(found, "lockout", 7))        eps_req->eps_command = VTSS_EPS_MGMT_COMMAND_LOCK_OUT;
    else if(!strncmp(found, "forced", 6))         eps_req->eps_command = VTSS_EPS_MGMT_COMMAND_FORCED_SWITCH;
    else if(!strncmp(found, "manualp", 7))        eps_req->eps_command = VTSS_EPS_MGMT_COMMAND_MANUEL_SWITCH_P;
    else if(!strncmp(found, "manualw", 7))        eps_req->eps_command = VTSS_EPS_MGMT_COMMAND_MANUEL_SWITCH_W;
    else if(!strncmp(found, "exercise", 8))       eps_req->eps_command = VTSS_EPS_MGMT_COMMAND_EXERCISE;
    else if(!strncmp(found, "freeze", 6))         eps_req->eps_command = VTSS_EPS_MGMT_COMMAND_FREEZE;
    else return 1;

    return 0;
} /* cli_parse_eps_command */

static int cli_parse_revertive(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char *found = cli_parse_find(cmd, stx);
    eps_cli_req_t  * eps_req = NULL;
    eps_req = req->module_req;

    if(!found)      return 1;
    else if(!strncmp(found, "revert", 6))       eps_req->revertive = true;
    else if(!strncmp(found, "norevert", 8))     eps_req->revertive = false;
    else return 1;

    return 0;
} /* cli_parse_revertive */


static int32_t cli_parse_eps_enable(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)

{
    eps_cli_req_t  * eps_req = NULL;
    char *found = cli_parse_find(cmd, stx);

    if(!found)      return 1;
    eps_req = req->module_req;

    if(!strncmp(found, "enable", 6))       eps_req->eps_enable = true;
    else if(!strncmp(found, "disable", 7))      eps_req->eps_enable = false;
    else return 1;

    return 0;
} /* cli_parse_eps_enable */

static int cli_parse_eps_domain(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char *found = cli_parse_find(cmd, stx);
    eps_cli_req_t  * eps_req = NULL;

    eps_req = req->module_req;

    if(!found)      return 1;
    else if(!strncmp(found, "domport", 7))       eps_req->eps_domain = VTSS_EPS_MGMT_PORT;
    else if(!strncmp(found, "domevc", 10))       eps_req->eps_domain = VTSS_EPS_MGMT_EVC;
    else return 1;

    return 0;
} /* cli_parse_eps_domain */


void eps_cli_def_req ( cli_req_t * req )
{
   eps_cli_req_t * eps_req;

   eps_req = req->module_req;

   eps_req->instance = 0xFFFFFFFF;
   eps_req->architecture = VTSS_EPS_MGMT_ARCHITECTURE_1P1;
   eps_req->eps_domain = VTSS_EPS_MGMT_PORT;
   eps_req->directional = VTSS_EPS_MGMT_UNIDIRECTIONAL;
   eps_req->aps = FALSE;
   eps_req->revertive = FALSE;
   eps_req->restore_timer = 0;
   eps_req->hold_off_timer = 0;
   eps_req->eps_command = VTSS_EPS_MGMT_COMMAND_CLEAR;
}

static cli_parm_t eps_cli_parm_table[] = {
    {
        "w0s|w10s|w30s|w1m|w5m|w12m",
        "Wait to restore timer value\n",
        CLI_PARM_FLAG_SET,
        cli_parse_eps_wtr_time,
        cli_cmd_id_eps_config
    },
    {
        "<inst>",
        "Instance number",
        CLI_PARM_FLAG_NONE,
        cli_parse_eps_instance,
        NULL,
    },
    {
        "1p1|1f1",
        "EPS architecture\n",
        CLI_PARM_FLAG_SET,
        cli_parse_architecture,
        cli_cmd_id_eps_create,
    },
    {
        "unidir|bidir",
        "Unidirectional or bidirectional switching\n",
        CLI_PARM_FLAG_SET,
        cli_parse_bidirectional,
        cli_cmd_id_eps_config,
    },
    {
        "enable|disable",
        "enable/disable protection\n",
        CLI_PARM_FLAG_SET,
        cli_parse_eps_enable,
        cli_cmd_id_eps_create,
    },
    {
        "<flow_w>",
        "Working flow instance number",
        CLI_PARM_FLAG_SET,
        cli_parse_working_flow,
        cli_cmd_id_eps_create
    },
    {
        "<flow_p>",
        "Protecting flow instance number",
        CLI_PARM_FLAG_SET,
        cli_parse_protection_flow,
        cli_cmd_id_eps_create,
    },
    {
        "<mep_w>",
        "Working MEP instance number",
        CLI_PARM_FLAG_SET,
        cli_parse_working_mep,
        cli_cmd_id_eps_create,
    },
    {
        "<mep_p>",
        "Protecting MEP instance number",
        CLI_PARM_FLAG_SET,
        cli_parse_protection_mep,
        cli_cmd_id_eps_create,
    },
    {
        "<mep_aps>",
        "APS MEP instance number",
        CLI_PARM_FLAG_SET,
        cli_parse_aps_mep,
        cli_cmd_id_eps_create,
    },
    {
        "domport|domevc",
        "Flow domain\n",
        CLI_PARM_FLAG_SET,
        cli_parse_eps_domain,
        cli_cmd_id_eps_create,
    },
    {
        "aps|noaps",
        "APS enable/disable\n",
        CLI_PARM_FLAG_SET,
        cli_parse_aps,
        cli_cmd_id_eps_config,
    },
    {
        "revert|norevert",
        "Revertive enable/disable\n",
        CLI_PARM_FLAG_SET,
        cli_parse_revertive,
        cli_cmd_id_eps_config,
    },
    {
        "h0s|h100ms|h500ms|h1s|h2s|h5s|h10s",
        "Hold off timer value\n",
        CLI_PARM_FLAG_SET,
        cli_parse_eps_hold_time,
        cli_cmd_id_eps_config,
    },
    {
        "clear|lockout|forced|manualp|manualw|exercise|freeze|lockoutlocal",
        "EPS protection command type - clear is 'no command active'\n",
        CLI_PARM_FLAG_SET,
        cli_parse_eps_command,
        cli_cmd_id_eps_command,
    },
    {
        NULL,
        NULL,
        0,
        0,
        NULL
    },
};


enum {
  PRIO_EPS_CREATE,
  PRIO_EPS_CONFIG,
  PRIO_EPS_COMMAND,
  PRIO_EPS_STATE,
};

/* Command table entries */

cli_cmd_tab_entry (
  "EPS create",
  "EPS create [<inst>] [domport|domevc] [1p1|1f1] [<flow_w>] [<flow_p>] [<mep_w>] [<mep_p>] [<mep_aps>] [enable|disable]",
  "EPS create",
  PRIO_EPS_CREATE,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_EPS,
  cli_cmd_id_eps_create,
  eps_cli_def_req,
  eps_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "EPS config",
  "EPS config [<inst>] [aps|noaps] [revert|norevert] [unidir|bidir] [w0s|w10s|w30s|w1m|w5m|w12m] [h0s|h100ms|h500ms|h1s|h2s|h5s|h10s]",
  "EPS config operation",
  PRIO_EPS_CONFIG,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_EPS,
  cli_cmd_id_eps_config,
  eps_cli_def_req,
  eps_cli_parm_table,
  CLI_CMD_FLAG_SYS_CONF
);

cli_cmd_tab_entry (
 "EPS command",
 "EPS command [<inst>] [clear|lockout|forced|manualp|manualw|exercise|freeze|lockoutlocal]",
 "EPS command set operation",
 PRIO_EPS_COMMAND,
 CLI_CMD_TYPE_CONF,
 VTSS_MODULE_ID_EPS,
 cli_cmd_id_eps_command,
 eps_cli_def_req,
 eps_cli_parm_table,
 CLI_CMD_FLAG_SYS_CONF
);

cli_cmd_tab_entry (
  "EPS state [<inst>]",
  NULL,
  "Get protection state",
  PRIO_EPS_STATE,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_EPS,
  cli_cmd_id_eps_state,
  eps_cli_def_req,
  eps_cli_parm_table,
  CLI_CMD_FLAG_NONE
);
