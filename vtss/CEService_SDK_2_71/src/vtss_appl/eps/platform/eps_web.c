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

#include "web_api.h"
#include "eps_api.h"
#ifdef VTSS_SW_OPTION_PRIV_LVL
#include "vtss_privilege_api.h"
#include "vtss_privilege_web_api.h"
#endif

char *eps_error_txt(u32 error)
{
    switch (error)
    {
        case EPS_RC_OK:                     return("EPS_RC_OK");
        case EPS_RC_NOT_CREATED:            return("EPS_RC_NOT_CREATED");
        case EPS_RC_CREATED:                return("EPS_RC_CREATED");
        case EPS_RC_INVALID_PARAMETER:      return("EPS_RC_INVALID_PARAMETER");
        case EPS_RC_NOT_CONFIGURED:         return("EPS_RC_NOT_CONFIGURED");
        case EPS_RC_ARCHITECTURE:           return("EPS_RC_ARCHITECTURE");
        case EPS_RC_W_P_FLOW_EQUAL:         return("EPS_RC_W_P_FLOW_EQUAL");
        case EPS_RC_W_P_SSF_MEP_EQUAL:      return("EPS_RC_W_P_SSF_MEP_EQUAL");
        case EPS_RC_INVALID_APS_MEP:        return("EPS_RC_INVALID_APS_MEP");
        case EPS_RC_INVALID_W_MEP:          return("EPS_RC_INVALID_W_MEP");
        case EPS_RC_INVALID_P_MEP:          return("EPS_RC_INVALID_P_MEP");
        case EPS_RC_WORKING_USED:           return("EPS_RC_WORKING_USED");
        case EPS_RC_PROTECTING_USED:        return("EPS_RC_PROTECTING_USED");
        default:                            return("EPS_RC_OK");
    }
}

/****************************************************************************/
/*  Web Handler  functions                                                  */
/****************************************************************************/

static cyg_int32 handler_config_eps_create(CYG_HTTPD_STATE *p)
{
    eps_mgmt_mep_t                mep;
    vtss_eps_mgmt_conf_t          config;
    vtss_eps_mgmt_create_param_t  param;
    vtss_eps_mgmt_state_t         state;
    int                        ct;
    uint                       eps_id, domain, architecture, w_flow, p_flow, w_mep, p_mep, aps_mep;
    char                       buf[32];
    u32                        rc;
    static u32                 error = EPS_RC_OK;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_EPS))
        return -1;
#endif

    if (p->method == CYG_HTTPD_METHOD_POST)
    {
        /* Delete EPS */
        for (eps_id=0; eps_id<EPS_MGMT_CREATED_MAX; ++eps_id)
        {
            rc = eps_mgmt_conf_get(eps_id, &param, &config, &mep);
            if ((rc == EPS_RC_OK) || (rc == EPS_RC_NOT_CONFIGURED))
            {   /* Created */
                sprintf(buf, "del_%u", eps_id+1);
                if (cyg_httpd_form_varable_find(p, buf))
                    if ((rc = eps_mgmt_instance_delete(eps_id)) != EPS_RC_OK)
                        if (error == EPS_RC_OK)     error = rc;
            }
            else
                if ((rc != EPS_RC_NOT_CREATED) && (error == EPS_RC_OK))     error = rc;
        }

        /* Add EPS */
        if (cyg_httpd_form_varable_int(p, "new_eps", &eps_id))
        {
            if (cyg_httpd_form_varable_int(p, "dom", &domain))
            if (cyg_httpd_form_varable_int(p, "arch", &architecture))
            if (cyg_httpd_form_varable_int(p, "w_flow", &w_flow))
            if (cyg_httpd_form_varable_int(p, "p_flow", &p_flow))
            if (cyg_httpd_form_varable_int(p, "w_mep", &w_mep))
            if (cyg_httpd_form_varable_int(p, "p_mep", &p_mep))
            if (cyg_httpd_form_varable_int(p, "aps_mep", &aps_mep))
            {
                param.domain = (vtss_eps_mgmt_domain_t)domain;
                param.architecture = (vtss_eps_mgmt_architecture_t)architecture;
                param.w_flow = w_flow-1;
                param.p_flow = p_flow-1;
                mep.w_mep = w_mep-1;
                mep.p_mep = p_mep-1;
                mep.aps_mep = aps_mep-1;
                if ((rc = eps_mgmt_instance_create(eps_id-1, &param)) != EPS_RC_OK) { /* Create the EPS instance */
                    if (error == EPS_RC_OK)     error = rc;
                } else {
                    if ((rc = eps_mgmt_mep_set(eps_id-1, &mep)) != EPS_RC_OK) { /* Add the MEP instances */
                        if (error == EPS_RC_OK)     error = rc;
                        (void)eps_mgmt_instance_delete(eps_id-1);   /* Add of MEP instances failed - delete the EPS instance */
                    }
                }
            }
        }

        redirect(p, "/eps.htm");
    }
    else
    {
        /* CYG_HTTPD_METHOD_GET (+HEAD) */
        (void)cyg_httpd_start_chunked("html");

#if defined(VTSS_SW_OPTION_EVC)
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u|", 1);
#else
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u|", 0);
#endif
        cyg_httpd_write_chunked(p->outbuffer, ct);

        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u|", EPS_MGMT_CREATED_MAX);
        cyg_httpd_write_chunked(p->outbuffer, ct);

        for (eps_id=0; eps_id<EPS_MGMT_CREATED_MAX; ++eps_id)
        {
            rc = eps_mgmt_conf_get(eps_id, &param, &config, &mep);
            if ((rc == EPS_RC_OK) || (rc == EPS_RC_NOT_CONFIGURED))
            {   /* Created */
                memset(&state, 0, sizeof(state));
                if ((rc = eps_mgmt_state_get(eps_id, &state)) != EPS_RC_OK)
                    if (error == EPS_RC_OK)     error = rc;

                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/%u/%u/%u/%u/%u/%u/%u/%s/%u|",
                              eps_id+1,
                              (uint)param.domain,
                              (uint)param.architecture,
                              param.w_flow+1,
                              param.p_flow+1,
                              mep.w_mep+1,
                              mep.p_mep+1,
                              mep.aps_mep+1,
                              ((state.w_state != VTSS_EPS_MGMT_DEFECT_STATE_OK) || (state.p_state != VTSS_EPS_MGMT_DEFECT_STATE_OK) ||
                                state.dFop_pm || state.dFop_cm || state.dFop_nr || state.dFop_NoAps) ? "Down" : "Up",
                              EPS_MEP_INST_INVALID+1);
                cyg_httpd_write_chunked(p->outbuffer, ct);

            }
            else
                if ((rc != EPS_RC_NOT_CREATED) && (error == EPS_RC_OK))     error = rc;
        }

        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s", eps_error_txt(error));
        cyg_httpd_write_chunked(p->outbuffer, ct);

        error = EPS_RC_OK;

        cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

static cyg_int32 handler_config_eps(CYG_HTTPD_STATE* p)
{
    vtss_isid_t                  sid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    u32                          rc, conf_rc;
    uint                         eps_id, direct, wtr, hold, comm;
    BOOL                         aps, revert;
    eps_mgmt_mep_t               mep;
    vtss_eps_mgmt_create_param_t param;
    vtss_eps_mgmt_conf_t         conf;
    vtss_eps_mgmt_command_t      command;
    vtss_eps_mgmt_state_t        state;
    int                          ct;
    char                         buf[32];
    static u32                   error = EPS_RC_OK;

    if(redirectUnmanagedOrInvalid(p, sid)) /* Redirect unmanaged/invalid access to handler */
        return -1;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_EPS))
        return -1;
#endif

    //
    // Setting new configuration
    //
    if(p->method == CYG_HTTPD_METHOD_POST)
    {
        if (cyg_httpd_form_varable_int(p, "eps_id_hidden", &eps_id))
        {
            rc = eps_mgmt_conf_get(eps_id-1, &param, &conf, &mep);
            if ((rc == EPS_RC_OK) || (rc == EPS_RC_NOT_CONFIGURED))
            {   /* Created */
                aps = revert = FALSE;
                direct = VTSS_EPS_MGMT_BIDIRECTIONAL;
                (void)cyg_httpd_form_varable_int(p, "direct", &direct);
                if (cyg_httpd_form_varable_find(p, "aps"))   aps = TRUE;
                if (cyg_httpd_form_varable_find(p, "revert"))    revert = TRUE;

                if (cyg_httpd_form_varable_int(p, "wtr", &wtr))
                if (cyg_httpd_form_varable_int(p, "hold", &hold))
                {
                    conf.directional = (param.architecture == VTSS_EPS_MGMT_ARCHITECTURE_1F1) ? VTSS_EPS_MGMT_BIDIRECTIONAL : (vtss_eps_mgmt_directional_t)direct;
                    conf.aps = ((conf.directional == VTSS_EPS_MGMT_BIDIRECTIONAL) || aps);
                    conf.revertive = revert;
                    conf.restore_timer = (u32)wtr;
                    conf.hold_off_timer = (u32)hold;
                    if ((rc = eps_mgmt_conf_set(eps_id-1,   &conf)) != EPS_RC_OK)
                        if (error == EPS_RC_OK)     error = rc;
                }
                if (cyg_httpd_form_varable_int(p, "comm", &comm))
                    if ((rc = eps_mgmt_command_set(eps_id-1,   comm)) != EPS_RC_OK)
                        if (error == EPS_RC_OK)     error = rc;
            }
            else
                if (error == EPS_RC_OK)     error = rc;
        }

        sprintf(buf, "/eps_config.htm?eps=%u", eps_id);
        redirect(p, buf);
    }
    else
    {
        (void)cyg_httpd_start_chunked("html");

        memset(&conf, 0, sizeof(conf));
        memset(&param, 0, sizeof(param));
        memset(&state, 0, sizeof(state));
        command = VTSS_EPS_MGMT_COMMAND_NONE;

        eps_id = (atoi(var_eps) - 1);
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u|",  eps_id+1);
        cyg_httpd_write_chunked(p->outbuffer, ct);

        conf_rc = eps_mgmt_conf_get(eps_id, &param, &conf, &mep);
        if ((conf_rc != EPS_RC_OK) && (conf_rc != EPS_RC_NOT_CONFIGURED))
            if (error == EPS_RC_OK)     error = conf_rc;

        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/%u/%u/%u/%u/%u/%u/%u|",
                      (uint)param.domain,
                      (uint)param.architecture,
                      param.w_flow+1,
                      param.p_flow+1,
                      mep.w_mep+1,
                      mep.p_mep+1,
                      mep.aps_mep+1,
                      EPS_MEP_INST_INVALID+1);
        cyg_httpd_write_chunked(p->outbuffer, ct);

        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s/%u/%u/%u/%u/%u|",
                      (conf_rc == EPS_RC_OK) ? "Up" : "Down",
                      (uint)conf.directional,
                      conf.aps,
                      conf.revertive,
                      conf.restore_timer,
                      conf.hold_off_timer);
        cyg_httpd_write_chunked(p->outbuffer, ct);

        if ((rc = eps_mgmt_command_get(eps_id, &command)) != EPS_RC_OK)
            if (error == EPS_RC_OK)     error = rc;
        if ((rc == EPS_RC_OK) && (rc = eps_mgmt_state_get(eps_id, &state)) != EPS_RC_OK)
            if (error == EPS_RC_OK)     error = rc;

        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u|",
                      (uint)command);
        cyg_httpd_write_chunked(p->outbuffer, ct);

        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/%u/%u/%u/%u/%u/%u/%u/%u/%s/%s/%s/%s|",
                      (uint)state.protection_state,
                      (uint)state.w_state,
                      (uint)state.p_state,
                      (uint)state.tx_aps.request,
                      state.tx_aps.re_signal,
                      state.tx_aps.br_signal,
                      (uint)state.rx_aps.request,
                      state.rx_aps.re_signal,
                      state.rx_aps.br_signal,
                      (state.dFop_pm) ? "Down" : "Up",
                      (state.dFop_cm) ? "Down" : "Up",
                      (state.dFop_nr) ? "Down" : "Up",
                      (state.dFop_NoAps) ? "Down" : "Up");
        cyg_httpd_write_chunked(p->outbuffer, ct);

        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s",  eps_error_txt(error));

        cyg_httpd_write_chunked(p->outbuffer, ct);

        error = EPS_RC_OK;

        cyg_httpd_end_chunked();
    }
    return -1; // Do not further search the file system.
}

// Status
static cyg_int32 handler_status_eps(CYG_HTTPD_STATE* p)
{
#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_STAT_TYPE, VTSS_MODULE_ID_EPS))
        return -1;
#endif

    if(p->method == CYG_HTTPD_METHOD_GET) {
        (void)cyg_httpd_start_chunked("html");
        cyg_httpd_end_chunked();
    }
    return -1; // Do not further search the file system.
}


/****************************************************************************/
/*  Module JS lib config routine                                            */
/****************************************************************************/

#define EPS_WEB_BUF_LEN 512

static size_t eps_lib_config_js(char **base_ptr, char **cur_ptr, size_t *length)
{
    char buff[EPS_WEB_BUF_LEN];
    (void) snprintf(buff, EPS_WEB_BUF_LEN,
                    "var configEpsMin = %d;\n"
                    "var configEpsMax = %d;\n",
                    1,
                    EPS_MGMT_CREATED_MAX
        );
    return webCommonBufferHandler(base_ptr, cur_ptr, length, buff);
}

/****************************************************************************/
/*  JS lib config table entry                                               */
/****************************************************************************/

web_lib_config_js_tab_entry(eps_lib_config_js);


/****************************************************************************/
/*  HTTPD Table Entries                                                     */
/****************************************************************************/

CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_eps_create, "/config/epsCreate", handler_config_eps_create);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_eps, "/config/epsConfig", handler_config_eps);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_status_eps, "/stat/eps_status", handler_status_eps);

/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
