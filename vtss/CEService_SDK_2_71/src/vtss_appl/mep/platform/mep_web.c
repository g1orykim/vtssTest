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
#include "mep_api.h"
#ifdef VTSS_SW_OPTION_PRIV_LVL
#include "vtss_privilege_api.h"
#include "vtss_privilege_web_api.h"
#endif

static u8 hexToInt(const char *hex)
{
    return (16 * ((hex[0] >= 'A') ? ((hex[0]-'A')+10) : (hex[0]-'0')) + ((hex[1] >= 'A') ? ((hex[1]-'A')+10) : (hex[1]-'0')));
}

static const char *mep_web_error_txt(vtss_rc error)
{
    if (error == VTSS_RC_OK)
        return("");
    else
        return(error_txt(error));
}


/****************************************************************************/
/*  Web Handler  functions                                                  */
/****************************************************************************/

static cyg_int32 handler_config_mep_create(CYG_HTTPD_STATE *p)
{
    vtss_mep_mgmt_conf_t     config;
    vtss_mep_mgmt_state_t    state;
    vtss_mep_mgmt_def_conf_t def_conf;
    u32                      eps_count;
    u16                      eps_inst[MEP_EPS_MAX];
    int                      ct;
    uint                     mep_id, domain, flow, vid, mode, direct, port, level;
    char                     buf[32];
    u32                      i;
    static vtss_rc           error = VTSS_RC_OK;
    BOOL                     alarm;
    u8                       mac[VTSS_MEP_MAC_LENGTH];
    vtss_rc                  rc;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_MEP))
        return -1;
#endif

    if (p->method == CYG_HTTPD_METHOD_POST)
    {
        /* Delete MEP */
        for (mep_id=0; mep_id<MEP_INSTANCE_MAX; ++mep_id)
        {
            if ((rc = mep_mgmt_conf_get(mep_id, mac, &eps_count, eps_inst, &config)) != VTSS_RC_OK)
                if (error == VTSS_RC_OK)     error = rc;

            if ((rc == VTSS_RC_OK) && config.enable)
            {   /* Created */
                sprintf(buf, "del_%u", mep_id+1);
                if (cyg_httpd_form_varable_find(p, buf))
                {
                    config.enable = FALSE;
                    if ((rc = mep_mgmt_conf_set(mep_id, &config)) != VTSS_RC_OK)
                        if (error == VTSS_RC_OK)     error = rc;
                }
            }
        }

        /* Add MEP */
        if (cyg_httpd_form_varable_int(p, "new_mep", &mep_id))
        {
            if (cyg_httpd_form_varable_int(p, "dom", &domain))
            if (cyg_httpd_form_varable_int(p, "flow", &flow))
            if (cyg_httpd_form_varable_int(p, "direct", &direct))
            if (cyg_httpd_form_varable_int(p, "vid", &vid))
            if (cyg_httpd_form_varable_int(p, "level", &level))
            if (cyg_httpd_form_varable_int(p, "mode", &mode))
            if (cyg_httpd_form_varable_int(p, "port", &port))
            {
                mep_mgmt_def_conf_get(&def_conf);
                config = def_conf.config;   /* Initialize confid to default */

                config.domain = (vtss_mep_mgmt_domain_t)domain;
                config.direction = (vtss_mep_mgmt_direction_t)direct;
                config.level = level;
                config.flow = flow;
                config.vid = vid;
                config.mode = (vtss_mep_mgmt_mode_t)mode;
                config.port = port-1;
                if (domain == VTSS_MEP_MGMT_PORT)
                    config.flow = config.port;
                if (domain == VTSS_MEP_MGMT_EVC)
                    config.flow--;
                config.enable = TRUE;

                if ((rc = mep_mgmt_conf_set(mep_id-1, &config)) != VTSS_RC_OK)
                    if (error == VTSS_RC_OK)     error = rc;
            }
        }
        redirect(p, "/mep.htm");
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

        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u|", MEP_INSTANCE_MAX);
        cyg_httpd_write_chunked(p->outbuffer, ct);

        for (mep_id=0; mep_id<MEP_INSTANCE_MAX; ++mep_id)
        {
            rc = mep_mgmt_conf_get(mep_id, mac, &eps_count, eps_inst, &config);
            if (rc == MEP_RC_VOLATILE) rc = VTSS_RC_OK;

            if (rc != VTSS_RC_OK)
                if (error == VTSS_RC_OK)     error = rc;

            if ((rc == VTSS_RC_OK) && config.enable)
            {   /* Created */
                memset(&state, 0, sizeof(state));
                if ((rc = mep_mgmt_state_get(mep_id, &state)) != VTSS_RC_OK)
                    if (error == VTSS_RC_OK)     error = rc;

                alarm = (state.cLevel || state.cMeg || state.cMep || state.cAis || state.cLck || state.cSsf);
                for (i=0; i<config.peer_count; ++i)
                    alarm = alarm || (state.cLoc[i] || state.cRdi[i] || state.cPeriod[i] || state.cPrio[i]);

                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/%u/%u/%u/%u/%u/%u/%u/%02X-%02X-%02X-%02X-%02X-%02X/%s|",
                              mep_id+1,
                              (uint)config.domain,
                              (uint)config.mode,
                              (uint)config.direction,
                              config.port+1,
                              config.level,
                              (config.domain != VTSS_MEP_MGMT_VLAN) ? config.flow+1 : config.flow,
                              config.vid,
                              mac[0],
                              mac[1],
                              mac[2],
                              mac[3],
                              mac[4],
                              mac[5],
                              alarm ? "Down" : "Up");
                cyg_httpd_write_chunked(p->outbuffer, ct);
            }
        }

        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s", mep_web_error_txt(error));
        cyg_httpd_write_chunked(p->outbuffer, ct);

        error = VTSS_RC_OK;

        cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

static cyg_int32 handler_config_mep(CYG_HTTPD_STATE* p)
{
    vtss_isid_t              sid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    u32                      i, j, cnt;
    char                     buf[100];
    uint                     mep_id, peer, level, mep, cc_prio, cc_rate, aps_prio, aps_cast, aps_type, vid=0, aps_octet, format, evcpag=0, evcqos=0;
    int                      ct;
    const char               *megS, *nameS, *macS;
    size_t                   len, n_len, m_len;
    static vtss_rc           error = VTSS_RC_OK;
    vtss_mep_mgmt_conf_t     config;
    vtss_mep_mgmt_cc_conf_t  cc_conf;
    vtss_mep_mgmt_aps_conf_t aps_conf;
    vtss_mep_mgmt_state_t    state;
    u32                      eps_count;
    u16                      eps_inst[MEP_EPS_MAX];
    u16                      peer_mep[VTSS_MEP_PEER_MAX];
    u8                       peer_mac[VTSS_MEP_PEER_MAX][VTSS_MEP_MAC_LENGTH];
    u8                       mac[VTSS_MEP_MAC_LENGTH];
    u8                       meg[VTSS_MEP_MEG_CODE_LENGTH];
    u8                       name[VTSS_MEP_MEG_CODE_LENGTH];
    char                     encoded_name[3 * VTSS_MEP_MEG_CODE_LENGTH];
    char                     data_string[200];
    vtss_rc                  rc;

    if(redirectUnmanagedOrInvalid(p, sid)) /* Redirect unmanaged/invalid access to handler */
        return -1;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_MEP))
        return -1;
#endif

    //
    // Setting new configuration
    //
    memset(&config, 0, sizeof(config));
    memset(&cc_conf, 0, sizeof(cc_conf));
    memset(&aps_conf, 0, sizeof(aps_conf));
    memset(&state, 0, sizeof(state));
    memset(name, 0, VTSS_MEP_MEG_CODE_LENGTH);
    memset(meg, 0, VTSS_MEP_MEG_CODE_LENGTH);

    if(p->method == CYG_HTTPD_METHOD_POST)
    {
        if (cyg_httpd_form_varable_int(p, "mep_id_hidden", &mep_id))
        {
            if ((rc = mep_mgmt_conf_get(mep_id-1, mac, &eps_count, eps_inst, &config)) != VTSS_RC_OK)
                if (error == VTSS_RC_OK)     error = rc;

            if ((rc == VTSS_RC_OK) && config.enable)
            {   /* Created */
                /* Check for delete peer MEP */
                for (i=0, cnt=0; i<config.peer_count; ++i)
                {
                    sprintf(buf, "del_%u", config.peer_mep[i]);
                    if (!cyg_httpd_form_varable_find(p, buf))
                    {   /* NOT delete */
                        sprintf(buf, "peerMAC_%u", config.peer_mep[i]);
                        macS=cyg_httpd_form_varable_string(p, buf, &len);
                        if (len > 0)
                        {   /* Peer MAC found */
                            peer_mep[cnt] = config.peer_mep[i];
                            for (j=0; j<6; ++j)     peer_mac[cnt][j] = hexToInt(&macS[j*3]);
                            cnt++;
                        }
                    }
                }
                memcpy(config.peer_mep, peer_mep, cnt*sizeof(u16));
                memcpy(config.peer_mac, peer_mac, cnt*VTSS_MEP_MAC_LENGTH);
                config.peer_count = cnt;

                /* Add peer MEP */
                if (cyg_httpd_form_varable_int(p, "new_peer", &peer))
                {
                    macS=cyg_httpd_form_varable_string(p, "new_peerMAC", &len);
                    if (len > 0)
                    {
                        config.peer_mep[config.peer_count] = peer;
                        for (i=0; i<6; ++i)     config.peer_mac[config.peer_count][i] = hexToInt(&macS[i*3]);
                        config.peer_count++;
                    }
                }

                (void)cyg_httpd_form_varable_int(p, "evcpag", &evcpag);
                (void)cyg_httpd_form_varable_int(p, "evcqos", &evcqos);
                (void)cyg_httpd_form_varable_int(p, "vid", &vid);
                if (cyg_httpd_form_varable_int(p, "level", &level))
                if (cyg_httpd_form_varable_int(p, "format", &format)) {
                    if ((nameS=cyg_httpd_form_varable_string(p, "name", &n_len))) {    /* Get 'name' */
                        if (n_len && cgi_unescape(nameS, name, n_len, sizeof(name)))  // name needs to be cgi_unescaped (e.g. %20 -> ' ').
                            n_len = strlen(name);   /* Calculate new real length */
                        else
                            n_len = 0;
                    }

                    if ((n_len == 0) || ((n_len > 0) && (n_len <= (VTSS_MEP_MEG_CODE_LENGTH-1))))   /* If Maintenance name is given length must be correct */
                    {
                        if ((megS=cyg_httpd_form_varable_string(p, "meg", &m_len))) {  /* get 'meg' */
                            if (m_len && cgi_unescape(megS, meg, m_len, sizeof(meg)))  // meg needs to be cgi_unescaped (e.g. %20 -> ' ').
                                m_len = strlen(meg);   /* Calculate new real length */
                            else
                                m_len = 0;
                        }

                        if ((m_len > 0) && (m_len <= (VTSS_MEP_MEG_CODE_LENGTH-1))) {
                            if (cyg_httpd_form_varable_int(p, "mep", &mep)) {
                                config.enable = TRUE;
                                config.level = level;
                                config.vid = vid;
                                config.voe = (cyg_httpd_form_varable_find(p, "voe") != NULL) ? TRUE : FALSE;
                                config.format = (vtss_mep_mgmt_format_t)format;
                                memset(config.name, 0, VTSS_MEP_MEG_CODE_LENGTH);
                                memset(config.meg, 0, VTSS_MEP_MEG_CODE_LENGTH);
                                memcpy(config.name, name, n_len);
                                memcpy(config.meg, meg, m_len);
                                config.mep = mep;
                                config.evc_pag = evcpag;
                                config.evc_qos = evcqos;
                                if ((rc = mep_mgmt_conf_set(mep_id-1, &config)) != VTSS_RC_OK)
                                    if (error == VTSS_RC_OK)     error = rc;
                            }
                        }
                    }
                }

                if (cyg_httpd_form_varable_find(p, "cc")) {
                    if (cyg_httpd_form_varable_int(p, "cc_prio", &cc_prio))
                    if (cyg_httpd_form_varable_int(p, "cc_rate", &cc_rate)) {
                        cc_conf.enable = TRUE;
                        cc_conf.prio = cc_prio;
                        cc_conf.period = (vtss_mep_mgmt_period_t)cc_rate;
                        if ((rc = mep_mgmt_cc_conf_set(mep_id-1,   &cc_conf)) != VTSS_RC_OK)
                            if (error == VTSS_RC_OK)     error = rc;
                    }
                }
                else {
                    cc_conf.enable = FALSE;
                    if ((rc = mep_mgmt_cc_conf_set(mep_id-1,   &cc_conf)) != VTSS_RC_OK)
                        if (error == VTSS_RC_OK)     error = rc;
                }

                if (cyg_httpd_form_varable_find(p, "aps")) {
                    if (cyg_httpd_form_varable_int(p, "aps_cast", &aps_cast))
                    if (cyg_httpd_form_varable_int(p, "aps_type", &aps_type))
                    if (cyg_httpd_form_varable_int(p, "aps_prio", &aps_prio))
                    if (cyg_httpd_form_varable_int(p, "aps_octet", &aps_octet)) {
                        aps_conf.enable = TRUE;
                        aps_conf.prio = aps_prio;
                        aps_conf.cast = aps_cast;
                        aps_conf.type = aps_type;
                        aps_conf.raps_octet = aps_octet;
                        if ((rc = mep_mgmt_aps_conf_set(mep_id-1,   &aps_conf)) != VTSS_RC_OK)
                            if (error == VTSS_RC_OK)     error = rc;
                    }
                }
                else {
                    aps_conf.enable = FALSE;
                    if ((rc = mep_mgmt_aps_conf_set(mep_id-1,   &aps_conf)) != VTSS_RC_OK)
                        if (error == VTSS_RC_OK)     error = rc;
                }
            }
        }
/*T_D("1 %lu", error);*/
        sprintf(buf, "/mep_config.htm?mep=%u", mep_id);
        redirect(p, buf);
    }
    else {
        (void)cyg_httpd_start_chunked("html");

        mep_id = (atoi(var_mep) - 1);

        rc = mep_mgmt_conf_get(mep_id, mac, &eps_count, eps_inst, &config);
        if (rc == MEP_RC_VOLATILE) rc = VTSS_RC_OK;

        if (rc != VTSS_RC_OK)
            if (error == VTSS_RC_OK)     error = rc;
        if ((rc = mep_mgmt_state_get(mep_id, &state)) != VTSS_RC_OK)
            if (error == VTSS_RC_OK)     error = rc;
        if ((rc = mep_mgmt_cc_conf_get(mep_id, &cc_conf)) != VTSS_RC_OK)
            if (error == VTSS_RC_OK)     error = rc;
        if ((rc = mep_mgmt_aps_conf_get(mep_id, &aps_conf)) != VTSS_RC_OK)
            if (error == VTSS_RC_OK)     error = rc;
/*T_D("4 %lu", error);*/

        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u|",  mep_id+1);
        cyg_httpd_write_chunked(p->outbuffer, ct);

#if defined(VTSS_SW_OPTION_UP_MEP)
        if (config.direction == VTSS_MEP_MGMT_UP)            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u|", 1);
        else                                                 ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u|", 0);
#else
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u|", 0);
#endif
        cyg_httpd_write_chunked(p->outbuffer, ct);
#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
#if defined(VTSS_ARCH_JAGUAR_1)
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u|", 1);
#endif
#if defined(VTSS_ARCH_SERVAL)
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u|", 2);
#endif
#else
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u|", 0);
#endif

        cyg_httpd_write_chunked(p->outbuffer, ct);

        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/%u|",
                      (uint)config.evc_pag,
                      (uint)config.evc_qos);
        cyg_httpd_write_chunked(p->outbuffer, ct);

        data_string[0] = '\0';
        for (i=0; i<eps_count; ++i)      /* Compose the EPS instance list */
            sprintf(data_string,"%s%u%c", data_string,
                    eps_inst[i]+1,
                    '-');
        if (eps_count == 0)
        {/* NO EPS instance is related - return value '0' */
            data_string[0] = '0';
            data_string[1] = '\0';
        }
        else
        {/* Overwrite the last ',' in string */
            len = strlen(data_string);
            data_string[len-1] = '\0';
        }

        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/%u/%u/%u/%u/%u/%s/%02X-%02X-%02X-%02X-%02X-%02X|",
                      (uint)config.domain,
                      (uint)config.mode,
                      (uint)config.direction,
                      config.port+1,
                      (config.domain != VTSS_MEP_MGMT_VLAN) ? config.flow+1 : config.flow,
                      config.vid,
                      data_string,
                      mac[0],
                      mac[1],
                      mac[2],
                      mac[3],
                      mac[4],
                      mac[5]);
        cyg_httpd_write_chunked(p->outbuffer, ct);

        data_string[0] = '\0';
        sprintf(data_string,"%s%u/%u/", data_string,
                config.level,
                config.format);

        (void) cgi_escape(config.name, encoded_name);
        sprintf(data_string,"%s%s/", data_string, encoded_name);

        (void) cgi_escape(config.meg, encoded_name);
        sprintf(data_string,"%s%s/", data_string, encoded_name);

        sprintf(data_string,"%s%u/%u/%u", data_string,
                config.mep,
                config.vid,
                config.voe);

        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s|", data_string);
        cyg_httpd_write_chunked(p->outbuffer, ct); //2


        sprintf(data_string,"%u", config.peer_count);
        for (i=0; i<config.peer_count; ++i)
        {
            sprintf(data_string,"%s/%u", data_string, config.peer_mep[i]);
            sprintf(data_string,"%s/%02X-%02X-%02X-%02X-%02X-%02X", data_string,
                      config.peer_mac[i][0],
                      config.peer_mac[i][1],
                      config.peer_mac[i][2],
                      config.peer_mac[i][3],
                      config.peer_mac[i][4],
                      config.peer_mac[i][5]);
        }

        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s|", data_string);
        cyg_httpd_write_chunked(p->outbuffer, ct); //3

        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/%u/%u/%u/%u/%u/%u/%u|",
                      cc_conf.enable,
                      cc_conf.prio,
                      (uint)cc_conf.period,
                      aps_conf.enable,
                      aps_conf.prio,
                      (uint)aps_conf.cast,
                      (uint)aps_conf.type,
                      aps_conf.raps_octet);
        cyg_httpd_write_chunked(p->outbuffer, ct);

        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s/%s/%s/%s/%s/%s/%s/%s|",
                      (state.cLevel) ? "Down" : "Up",
                      (state.cMeg) ? "Down" : "Up",
                      (state.cMep) ? "Down" : "Up",
                      (state.cAis) ? "Down" : "Up",
                      (state.cLck) ? "Down" : "Up",
                      (state.cSsf) ? "Down" : "Up",
                      (state.aBlk) ? "Down" : "Up",
                      (state.aTsf) ? "Down" : "Up");
        cyg_httpd_write_chunked(p->outbuffer, ct); //5

        data_string[0] = '\0';
        for (i=0; i<config.peer_count; ++i)
            sprintf(data_string,"%s%s/%s/%s/%s/", data_string,
                    (state.cLoc[i]) ? "Down" : "Up",
                    (state.cRdi[i]) ? "Down" : "Up",
                    (state.cPeriod[i]) ? "Down" : "Up",
                    (state.cPrio[i]) ? "Down" : "Up");

        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s|", data_string);
        cyg_httpd_write_chunked(p->outbuffer, ct); //6
        
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s",  mep_web_error_txt(error));
        cyg_httpd_write_chunked(p->outbuffer, ct); //7

        error = VTSS_RC_OK;

        cyg_httpd_end_chunked();
    }
    return -1; // Do not further search the file system.
}


static cyg_int32 handler_config_fm_mep(CYG_HTTPD_STATE* p)
{
    vtss_isid_t                  sid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    u32                          i, j;
    char                         buf[32];
    uint                         mep_id, flow;
    uint                         lt_prio, lt_peer, lt_ttl, ais_prio, lck_prio, rate, level, domain;
    uint                         lb_prio, lb_dei, lb_cast, lb_peer, lb_tosend, lb_size, lb_interval;
    uint                         tst_prio, tst_dei, tst_peer, tst_rate, tst_size, tst_pattern, tst_seq;
    int                          ct;
    const char                   *macS;
    size_t                       len;
    static vtss_rc               error = VTSS_RC_OK;
    vtss_mep_mgmt_conf_t         config;
    vtss_mep_mgmt_lt_conf_t      lt_conf;
    vtss_mep_mgmt_lb_conf_t      lb_conf;
    vtss_mep_mgmt_ais_conf_t     ais_conf;
    vtss_mep_mgmt_lck_conf_t     lck_conf;
    vtss_mep_mgmt_tst_conf_t     tst_conf;
    vtss_mep_mgmt_client_conf_t  client_conf;
    vtss_mep_mgmt_lt_state_t     lt_state;
    vtss_mep_mgmt_lb_state_t     lb_state;
    vtss_mep_mgmt_tst_state_t    tst_state;
    u32                          eps_count;
    u16                          eps_inst[MEP_EPS_MAX];    
    u8                           mac[VTSS_MEP_MAC_LENGTH];
    char                         data_string[200];
    vtss_rc                      rc;

    if(redirectUnmanagedOrInvalid(p, sid)) /* Redirect unmanaged/invalid access to handler */
        return -1;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_MEP))
        return -1;
#endif

    //
    // Setting new configuration
    //
    memset(&config, 0, sizeof(config));
    memset(&lt_conf, 0, sizeof(lt_conf));
    memset(&lb_conf, 0, sizeof(lb_conf));
    memset(&ais_conf,0, sizeof(ais_conf));
    memset(&lck_conf,0, sizeof(lck_conf));
    memset(&tst_conf,0, sizeof(tst_conf));
    memset(&lt_state, 0, sizeof(lt_state));
    memset(&lb_state, 0, sizeof(lb_state));

    ais_prio = 0;

    if(p->method == CYG_HTTPD_METHOD_POST)
    {
/*T_D("Post");*/
        if (cyg_httpd_form_varable_int(p, "mep_id_hidden", &mep_id))
        {
            if ((rc = mep_mgmt_conf_get(mep_id-1, mac, &eps_count, eps_inst, &config)) != VTSS_RC_OK)
                if (error == VTSS_RC_OK)     error = rc;

            if ((rc == VTSS_RC_OK) && config.enable)
            {   /* Created */
                if (cyg_httpd_form_varable_find(p, "lt"))
                {
                    if (cyg_httpd_form_varable_int(p, "lt_peer", &lt_peer))
                    if (cyg_httpd_form_varable_int(p, "lt_prio", &lt_prio))
                    if (cyg_httpd_form_varable_int(p, "lt_ttl", &lt_ttl))
                    {
                        macS=cyg_httpd_form_varable_string(p, "lt_mac", &len);
                        if (len > 0)
                        {   /* Unicast MAC found */
                            lt_conf.enable = TRUE;
                            lt_conf.prio = lt_prio;
                            lt_conf.mep = lt_peer;
                            for (j=0; j<6; ++j)     lt_conf.mac[j] = hexToInt(&macS[j*3]);
                            lt_conf.ttl = lt_ttl;
                            if ((rc = mep_mgmt_lt_conf_set(mep_id-1,   &lt_conf)) != VTSS_RC_OK)
                                if (error == VTSS_RC_OK)     error = rc;
                        }
                    }
                }
                else
                {
                    lt_conf.enable = FALSE;
                    if ((rc = mep_mgmt_lt_conf_set(mep_id-1,   &lt_conf)) != VTSS_RC_OK)
                        if (error == VTSS_RC_OK)     error = rc;
                }

                if (cyg_httpd_form_varable_find(p, "lb"))
                {
                    lb_dei = (cyg_httpd_form_varable_find(p, "lb_dei") != NULL);
                    if (cyg_httpd_form_varable_int(p, "lb_prio", &lb_prio))
                    if (cyg_httpd_form_varable_int(p, "lb_cast", &lb_cast))
                    if (cyg_httpd_form_varable_int(p, "lb_peer", &lb_peer))
                    if (cyg_httpd_form_varable_int(p, "lb_tosend", &lb_tosend))
                    if (cyg_httpd_form_varable_int(p, "lb_size", &lb_size))
                    if (cyg_httpd_form_varable_int(p, "lb_interval", &lb_interval))
                    {
                        macS=cyg_httpd_form_varable_string(p, "lb_mac", &len);
                        if (len > 0)
                        {   /* Unicast MAC found */
                            lb_conf.enable = TRUE;
                            lb_conf.prio = lb_prio;
                            lb_conf.dei = lb_dei;
                            lb_conf.cast = lb_cast;
                            lb_conf.mep = lb_peer;
                            for (j=0; j<6; ++j)     lb_conf.mac[j] = hexToInt(&macS[j*3]);
                            lb_conf.to_send = lb_tosend;
                            lb_conf.size = lb_size;
                            lb_conf.interval = lb_interval;
                            if ((rc = mep_mgmt_lb_conf_set(mep_id-1,   &lb_conf)) != VTSS_RC_OK)
                                if (error == VTSS_RC_OK)     error = rc;
                        }
                    }
                }
                else
                {
                    lb_conf.enable = FALSE;
                    if ((rc = mep_mgmt_lb_conf_set(mep_id-1,   &lb_conf)) != VTSS_RC_OK)
                        if (error == VTSS_RC_OK)     error = rc;
                }

                tst_dei = (cyg_httpd_form_varable_find(p, "tst_dei") != NULL);
                tst_seq = (cyg_httpd_form_varable_find(p, "tst_seq") != NULL);
                if (cyg_httpd_form_varable_int(p, "tst_prio", &tst_prio))
                if (cyg_httpd_form_varable_int(p, "tst_peer", &tst_peer))
                if (cyg_httpd_form_varable_int(p, "tst_rate", &tst_rate))
                if (cyg_httpd_form_varable_int(p, "tst_size", &tst_size))
                if (cyg_httpd_form_varable_int(p, "tst_pattern", &tst_pattern))
                {
                    tst_conf.dei = tst_dei;
                    tst_conf.prio = tst_prio;
                    tst_conf.mep = tst_peer;
                    tst_conf.rate = tst_rate * 1000;
                    tst_conf.size = tst_size;
                    tst_conf.pattern = tst_pattern;
                    tst_conf.sequence = tst_seq;

                    if (cyg_httpd_form_varable_find(p, "tst_rx"))
                        tst_conf.enable_rx = TRUE;

                    if (cyg_httpd_form_varable_find(p, "tst_tx"))
                        tst_conf.enable = TRUE;
    
                    if ((rc = mep_mgmt_tst_conf_set(mep_id-1,   &tst_conf)) != VTSS_RC_OK)
                        if (error == VTSS_RC_OK)     error = rc;
                }

                if (cyg_httpd_form_varable_find(p, "tst_clear"))
                    mep_mgmt_tst_state_clear_set(mep_id-1);

                if (cyg_httpd_form_varable_int(p, "c_domain", &domain))
                {
                    client_conf.domain = (vtss_mep_mgmt_domain_t)domain;
//                    client_conf.domain = VTSS_MEP_MGMT_EVC;
                    for (i=0, j=0; i<VTSS_MEP_CLIENT_FLOWS_MAX; i++)
                    {
                        sprintf(buf, "c_flow%u", i);
                        if (cyg_httpd_form_varable_int(p, buf, &flow) && (flow > 0)) {
                            sprintf(buf, "c_level%u", i);
                            if (cyg_httpd_form_varable_int(p, buf, &level)) {
                                sprintf(buf, "c_ais%u", i);
                                if (cyg_httpd_form_varable_int(p, buf, &ais_prio)) {
                                    sprintf(buf, "c_lck%u", i);
                                    if (cyg_httpd_form_varable_int(p, buf, &lck_prio)) {
                                        client_conf.flows[j] = flow;
                                        if (domain != VTSS_MEP_MGMT_VLAN)
                                            client_conf.flows[j]--;
                                        client_conf.level[j] = level;
                                        client_conf.ais_prio[j] = ais_prio;
                                        client_conf.lck_prio[j] = lck_prio;
                                        j++;
                                    }
                                }
                            }
                        }
                    }
                    client_conf.flow_count = j;
                    if ((rc = mep_mgmt_client_conf_set(mep_id-1, &client_conf)) != VTSS_RC_OK)
                        if (error == VTSS_RC_OK)     error = rc;

                    if (cyg_httpd_form_varable_int(p, "ais_rate", &rate))
                    {
                        ais_conf.enable = (cyg_httpd_form_varable_find(p, "ais_en") != NULL);
                        ais_conf.protection = (cyg_httpd_form_varable_find(p, "ais_prot") != NULL);
                        ais_conf.period = rate;
                        if ((rc = mep_mgmt_ais_conf_set(mep_id-1, &ais_conf)) != VTSS_RC_OK)
                            if (error == VTSS_RC_OK)     error = rc;
                    }

                    if (cyg_httpd_form_varable_int(p, "lck_rate", &rate))
                    {
                        lck_conf.enable = (cyg_httpd_form_varable_find(p, "lck_en") != NULL);
                        lck_conf.period = rate;
                        if ((rc = mep_mgmt_lck_conf_set(mep_id-1, &lck_conf)) != VTSS_RC_OK)
                            if (error == VTSS_RC_OK)     error = rc;
                    }
                }
            }
        }
/*T_D("1 %lu", error);*/
        sprintf(buf, "/mep_fm_config.htm?mep=%u", mep_id);
        redirect(p, buf);
    }
    else
    {
        (void)cyg_httpd_start_chunked("html");

        mep_id = (atoi(var_mep) - 1);
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u|",  mep_id+1);
        cyg_httpd_write_chunked(p->outbuffer, ct); // 0

        if ((rc = mep_mgmt_lt_conf_get(mep_id, &lt_conf)) != VTSS_RC_OK)
            if (error == VTSS_RC_OK)     error = rc;
        if ((rc = mep_mgmt_lb_conf_get(mep_id, &lb_conf)) != VTSS_RC_OK)
            if (error == VTSS_RC_OK)     error = rc;
        if ((rc = mep_mgmt_ais_conf_get(mep_id, &ais_conf)) != VTSS_RC_OK)
            if (error == VTSS_RC_OK)     error = rc;
        if ((rc = mep_mgmt_lck_conf_get(mep_id, &lck_conf)) != VTSS_RC_OK)
            if (error == VTSS_RC_OK)     error = rc;
        if ((rc = mep_mgmt_tst_conf_get(mep_id, &tst_conf)) != VTSS_RC_OK)
            if (error == VTSS_RC_OK)     error = rc;
        if ((rc = mep_mgmt_client_conf_get(mep_id, &client_conf)) != VTSS_RC_OK)
            if (error == VTSS_RC_OK)     error = rc;
        if ((rc = mep_mgmt_lt_state_get(mep_id, &lt_state)) != VTSS_RC_OK)
            if (error == VTSS_RC_OK)     error = rc;
        if ((rc = mep_mgmt_lb_state_get(mep_id, &lb_state)) != VTSS_RC_OK)
            if (error == VTSS_RC_OK)     error = rc;
        if ((rc = mep_mgmt_tst_state_get(mep_id, &tst_state)) != VTSS_RC_OK)
            if (error == VTSS_RC_OK)     error = rc;
/*T_D("4 %u", error);*/

        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/%u/%u/%02X-%02X-%02X-%02X-%02X-%02X/%u/%u/%u/%u/%u/%u/%02X-%02X-%02X-%02X-%02X-%02X/%u/%u/%u|",
                      lt_conf.enable,
                      lt_conf.prio,
                      lt_conf.mep,
                      lt_conf.mac[0],
                      lt_conf.mac[1],
                      lt_conf.mac[2],
                      lt_conf.mac[3],
                      lt_conf.mac[4],
                      lt_conf.mac[5],
                      lt_conf.ttl,
                      lb_conf.enable,
                      lb_conf.dei,
                      lb_conf.prio,
                      (uint)lb_conf.cast,
                      lb_conf.mep,
                      lb_conf.mac[0],
                      lb_conf.mac[1],
                      lb_conf.mac[2],
                      lb_conf.mac[3],
                      lb_conf.mac[4],
                      lb_conf.mac[5],
                      lb_conf.to_send,
                      lb_conf.size,
                      lb_conf.interval);
        cyg_httpd_write_chunked(p->outbuffer, ct);

        data_string[0] = '\0';
        sprintf(data_string,"%u/%u/%llu", lb_state.reply_cnt, lb_state.transaction_id, lb_state.lbm_transmitted);

        for (i=0; i<lb_state.reply_cnt; ++i)
        {
            sprintf(data_string,"%s/%02X-%02X-%02X-%02X-%02X-%02X/%llu/%llu", data_string,
                    lb_state.reply[i].mac[0],
                    lb_state.reply[i].mac[1],
                    lb_state.reply[i].mac[2],
                    lb_state.reply[i].mac[3],
                    lb_state.reply[i].mac[4],
                    lb_state.reply[i].mac[5],
                    lb_state.reply[i].lbr_received,
                    lb_state.reply[i].out_of_order);
        }
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s|", data_string);
        cyg_httpd_write_chunked(p->outbuffer, ct);

        data_string[0] = '\0';
        sprintf(data_string,"%u/", lt_state.transaction_cnt);
        for (i=0; i<lt_state.transaction_cnt; ++i)
        {
            sprintf(data_string,"%s%u/%u/", data_string,
                    lt_state.transaction[i].transaction_id,
                    lt_state.transaction[i].reply_cnt);
            for (j=0; j<lt_state.transaction[i].reply_cnt; ++j)
                sprintf(data_string,"%s%u/%u/%u/%s/%u/%02X-%02X-%02X-%02X-%02X-%02X/%02X-%02X-%02X-%02X-%02X-%02X/", data_string,
                        lt_state.transaction[i].reply[j].ttl,
                        (uint)lt_state.transaction[i].reply[j].mode,
                        (uint)lt_state.transaction[i].reply[j].direction,
                        (lt_state.transaction[i].reply[j].forwarded) ? "Yes" : "No",
                        (uint)lt_state.transaction[i].reply[j].relay_action,
                        lt_state.transaction[i].reply[j].last_egress_mac[0],
                        lt_state.transaction[i].reply[j].last_egress_mac[1],
                        lt_state.transaction[i].reply[j].last_egress_mac[2],
                        lt_state.transaction[i].reply[j].last_egress_mac[3],
                        lt_state.transaction[i].reply[j].last_egress_mac[4],
                        lt_state.transaction[i].reply[j].last_egress_mac[5],
                        lt_state.transaction[i].reply[j].next_egress_mac[0],
                        lt_state.transaction[i].reply[j].next_egress_mac[1],
                        lt_state.transaction[i].reply[j].next_egress_mac[2],
                        lt_state.transaction[i].reply[j].next_egress_mac[3],
                        lt_state.transaction[i].reply[j].next_egress_mac[4],
                        lt_state.transaction[i].reply[j].next_egress_mac[5]);
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s", data_string);
            cyg_httpd_write_chunked(p->outbuffer, ct);
            data_string[0] = '\0';
        }
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s|", data_string);
        cyg_httpd_write_chunked(p->outbuffer, ct);

        data_string[0] = '\0';
        sprintf(data_string, "%u/", (uint)client_conf.domain);
        for (i=0; i<client_conf.flow_count; i++)
            sprintf(data_string, "%s%u/%u/%u/%u/", data_string, ((client_conf.domain != VTSS_MEP_MGMT_VLAN) ? client_conf.flows[i]+1 : client_conf.flows[i]), client_conf.level[i], client_conf.ais_prio[i], client_conf.lck_prio[i]);
        for (i=0; i<(VTSS_MEP_CLIENT_FLOWS_MAX - client_conf.flow_count); i++)
            sprintf(data_string, "%s%u/%u/%u/%u/", data_string, 0, 0, 0, 0);

        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s|", data_string); 
        cyg_httpd_write_chunked(p->outbuffer, ct);

        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/%u/%u|",
                      (uint)ais_conf.enable,
                      (uint)ais_conf.period,
                      (uint)ais_conf.protection);
        cyg_httpd_write_chunked(p->outbuffer, ct);

        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/%u|",
                      (uint)lck_conf.enable,
                      (uint)lck_conf.period);
        cyg_httpd_write_chunked(p->outbuffer, ct);

        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/%u/%u/%u/%u/%u/%u/%u/%u|",
                      tst_conf.enable,
                      tst_conf.enable_rx,
                      tst_conf.dei,
                      tst_conf.prio,
                      tst_conf.mep,
                      tst_conf.rate/1000,
                      tst_conf.size,
                      (uint)tst_conf.pattern,
                      tst_conf.sequence);
        cyg_httpd_write_chunked(p->outbuffer, ct);

        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%llu/%llu/%u/%u|",
                      tst_state.tx_counter,
                      tst_state.rx_counter,
                      tst_state.rx_rate,
                      tst_state.time);
        cyg_httpd_write_chunked(p->outbuffer, ct);

#if defined(VTSS_ARCH_SERVAL)
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u|", 1);
#else
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u|", 0);
#endif
        cyg_httpd_write_chunked(p->outbuffer, ct);

        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s",  mep_web_error_txt(error));

        cyg_httpd_write_chunked(p->outbuffer, ct);

        error = VTSS_RC_OK;

        cyg_httpd_end_chunked();
    }
    return -1; // Do not further search the file system.
}


static cyg_int32 handler_config_pm_mep(CYG_HTTPD_STATE* p)
{
    vtss_isid_t              sid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    vtss_rc                  rc;
    char                     buf[32];
    uint                     mep_id;
    uint                     lm_prio, lm_cast, lm_rate, lm_ended, lm_flr;
    uint                     dm_prio, dm_mep, dm_gap, dm_count, dm_tunit, dm_act;
    uint                     dm_cast, dm_way, dm_txway, dm_calcway; 
    int                      ct;
    static vtss_rc           error = VTSS_RC_OK;
    vtss_mep_mgmt_conf_t     config;
    vtss_mep_mgmt_pm_conf_t  pm_conf;
    vtss_mep_mgmt_lm_conf_t  lm_conf;
    vtss_mep_mgmt_lm_state_t lm_state;
    vtss_mep_mgmt_dm_conf_t  dm_conf;
    vtss_mep_mgmt_dm_state_t dmr_state, dm1_state_far_to_near, dm1_state_near_to_far;
    u32                      eps_count;
    u16                      eps_inst[MEP_EPS_MAX];    
    u8                       mac[VTSS_MEP_MAC_LENGTH];

    if(redirectUnmanagedOrInvalid(p, sid)) /* Redirect unmanaged/invalid access to handler */
        return -1;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_MEP))
        return -1;
#endif

    //
    // Setting new configuration
    //
    memset(&config, 0, sizeof(config));
    memset(&pm_conf, 0, sizeof(pm_conf));
    memset(&lm_conf, 0, sizeof(lm_conf));
    memset(&lm_state, 0, sizeof(lm_state));
    memset(&dm_conf, 0, sizeof(dm_conf));
    memset(&dmr_state, 0, sizeof(dmr_state));
    memset(&dm1_state_far_to_near, 0, sizeof(dm1_state_far_to_near));
    memset(&dm1_state_near_to_far, 0, sizeof(dm1_state_near_to_far));

    if(p->method == CYG_HTTPD_METHOD_POST)
    {
/*T_D("Post");*/
        if (cyg_httpd_form_varable_int(p, "mep_id_hidden", &mep_id))
        {
            if ((rc = mep_mgmt_conf_get(mep_id-1, mac, &eps_count, eps_inst, &config)) != VTSS_RC_OK)
                if (error == VTSS_RC_OK)     error = rc;

            if ((rc == VTSS_RC_OK) && config.enable)
            {   /* Created */
                if (cyg_httpd_form_varable_find(p, "pm_data"))
                {
                    pm_conf.enable = TRUE;
                }
                else
                {
                    pm_conf.enable = FALSE;
                }
                if ((rc = mep_mgmt_pm_conf_set(mep_id-1,   &pm_conf)) != VTSS_RC_OK)
                    if (error == VTSS_RC_OK)     error = rc;

                if (cyg_httpd_form_varable_find(p, "lm"))
                {
                    if (cyg_httpd_form_varable_int(p, "lm_rate", &lm_rate))
                    if (cyg_httpd_form_varable_int(p, "lm_flr", &lm_flr))
                    if (cyg_httpd_form_varable_int(p, "lm_ended", &lm_ended))
                    if (cyg_httpd_form_varable_int(p, "lm_cast", &lm_cast))
                    if (cyg_httpd_form_varable_int(p, "lm_prio", &lm_prio))
                    {
                        lm_conf.enable = TRUE;
                        lm_conf.prio = lm_prio;
                        lm_conf.period = lm_rate;
                        lm_conf.ended = lm_ended;
                        lm_conf.cast = lm_cast;
                        lm_conf.flr_interval = lm_flr;
                        if ((rc = mep_mgmt_lm_conf_set(mep_id-1,   &lm_conf)) != VTSS_RC_OK)
                            if (error == VTSS_RC_OK)     error = rc;
                    }
                }
                else
                {
                    lm_conf.enable = FALSE;
                    if ((rc = mep_mgmt_lm_conf_set(mep_id-1,   &lm_conf)) != VTSS_RC_OK)
                        if (error == VTSS_RC_OK)     error = rc;
                }

                if (cyg_httpd_form_varable_find(p, "clear"))
                    mep_mgmt_lm_state_clear_set(mep_id-1);
                
                if (cyg_httpd_form_varable_find(p, "dm"))
                {
                    if (cyg_httpd_form_varable_int(p, "dm_prio", &dm_prio))
                    if (cyg_httpd_form_varable_int(p, "dm_cast", &dm_cast))
                    if (cyg_httpd_form_varable_int(p, "dm_mep", &dm_mep))
                    if (cyg_httpd_form_varable_int(p, "dm_way", &dm_way))
                    if (cyg_httpd_form_varable_int(p, "dm_txway", &dm_txway))
                    if (cyg_httpd_form_varable_int(p, "dm_calcway", &dm_calcway))
                    if (cyg_httpd_form_varable_int(p, "dm_gap", &dm_gap))
                    if (cyg_httpd_form_varable_int(p, "dm_count", &dm_count))
                    if (cyg_httpd_form_varable_int(p, "dm_tunit", &dm_tunit))
                    if (cyg_httpd_form_varable_int(p, "dm_act", &dm_act))   
                    {
                        dm_conf.enable = TRUE;
                        dm_conf.prio = dm_prio;
                        dm_conf.cast = dm_cast;
                        dm_conf.mep = dm_mep;
                        dm_conf.ended = dm_way;
                        dm_conf.proprietary = dm_txway;
                        dm_conf.calcway = dm_calcway;
                        dm_conf.interval = dm_gap;
                        dm_conf.lastn = dm_count;
                        dm_conf.tunit = dm_tunit;
                        dm_conf.syncronized = (cyg_httpd_form_varable_find(p, "dm_d2ford1") != NULL);
                        dm_conf.overflow_act = dm_act;
                        
                              
                        if ((rc = mep_mgmt_dm_conf_set(mep_id-1,   &dm_conf)) != VTSS_RC_OK)
                            if (error == VTSS_RC_OK)     error = rc;
                    }
                }
                else
                {
                    dm_conf.enable = FALSE;
                    if (cyg_httpd_form_varable_int(p, "dm_way", &dm_way))
                        dm_conf.ended = dm_way;
                    if (cyg_httpd_form_varable_int(p, "dm_tunit", &dm_tunit))   /* This is only in order to change the time unit for 1DM reception - 1DM is not enabled */
                        dm_conf.tunit = dm_tunit;
                    if ((rc = mep_mgmt_dm_conf_set(mep_id-1, &dm_conf)) != VTSS_RC_OK)
                        if (error == VTSS_RC_OK)     error = rc;
                }
                if (cyg_httpd_form_varable_find(p, "dm_clear"))
                    mep_mgmt_dm_state_clear_set(mep_id-1);
            }
        }
/*T_D("1 %u", error);*/
        sprintf(buf, "/mep_pm_config.htm?mep=%u", mep_id);
        redirect(p, buf);
    }
    else
    {
        (void)cyg_httpd_start_chunked("html");

        mep_id = (atoi(var_mep) - 1);
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u|",  mep_id+1);
        cyg_httpd_write_chunked(p->outbuffer, ct); // 0

        if ((rc = mep_mgmt_pm_conf_get(mep_id, &pm_conf)) != VTSS_RC_OK)
            if (error == VTSS_RC_OK)     error = rc;
        if ((rc = mep_mgmt_lm_conf_get(mep_id, &lm_conf)) != VTSS_RC_OK)
            if (error == VTSS_RC_OK)     error = rc;
        if ((rc = mep_mgmt_dm_conf_get(mep_id, &dm_conf)) != VTSS_RC_OK)
            if (error == VTSS_RC_OK)     error = rc;    
        if ((rc = mep_mgmt_lm_state_get(mep_id, &lm_state)) != VTSS_RC_OK)
            if (error == VTSS_RC_OK)     error = rc;
        if ((rc = mep_mgmt_dm_state_get(mep_id, &dmr_state, &dm1_state_far_to_near, &dm1_state_near_to_far)) != VTSS_RC_OK)
            if (error == VTSS_RC_OK)     error = rc;            
            
/*T_D("4 %u", error);*/
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u|",
                      lm_conf.enable, //1
                      lm_conf.prio, //2
                      (uint)lm_conf.period, //3
                      (uint)lm_conf.cast, //4
                      (uint)lm_conf.ended,//5
                      lm_conf.flr_interval, //6
                      dm_conf.enable, //7 
                      dm_conf.prio,   //8
                      (uint)dm_conf.cast, //9
                      dm_conf.mep, //10    
                      (uint)dm_conf.ended,  //11   
                      (uint)dm_conf.proprietary, //12  
                      (uint)dm_conf.calcway, //13 
                      dm_conf.interval, //14    
                      dm_conf.lastn, //15
                      (uint)dm_conf.tunit, //16 
                      dm_conf.syncronized, //17
                      (uint)dm_conf.overflow_act);  // 18
        cyg_httpd_write_chunked(p->outbuffer, ct); //1 

        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/%u/%u/%u/%u/%u|",
                      lm_state.tx_counter,
                      lm_state.rx_counter,
                      lm_state.near_los_counter,
                      lm_state.far_los_counter,
                      lm_state.near_los_ratio,
                      lm_state.far_los_ratio);
        cyg_httpd_write_chunked(p->outbuffer, ct); //2

        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u|",
                     dm1_state_far_to_near.tx_cnt,         
                     dm1_state_far_to_near.rx_cnt,         
                     dm1_state_far_to_near.rx_tout_cnt,    
                     dm1_state_far_to_near.rx_err_cnt,     
                     dm1_state_far_to_near.avg_delay,      
                     dm1_state_far_to_near.avg_n_delay,    
                     dm1_state_far_to_near.min_delay,     
                     dm1_state_far_to_near.max_delay,    
                     dm1_state_far_to_near.avg_delay_var,  
                     dm1_state_far_to_near.avg_n_delay_var,
                     dm1_state_far_to_near.min_delay_var,
                     dm1_state_far_to_near.max_delay_var,
                     dm1_state_far_to_near.ovrflw_cnt,
                     dmr_state.tx_cnt,         
                     dmr_state.rx_cnt,         
                     dmr_state.rx_tout_cnt,    
                     dmr_state.rx_err_cnt,     
                     dmr_state.avg_delay,      
                     dmr_state.avg_n_delay,    
                     dmr_state.min_delay,     
                     dmr_state.max_delay,    
                     dmr_state.avg_delay_var,  
                     dmr_state.avg_n_delay_var,
                     dmr_state.min_delay_var,
                     dmr_state.max_delay_var,
                     dmr_state.ovrflw_cnt,
                     dm1_state_near_to_far.tx_cnt,         
                     dm1_state_near_to_far.rx_cnt,         
                     dm1_state_near_to_far.rx_tout_cnt,    
                     dm1_state_near_to_far.rx_err_cnt,     
                     dm1_state_near_to_far.avg_delay,      
                     dm1_state_near_to_far.avg_n_delay,    
                     dm1_state_near_to_far.min_delay,     
                     dm1_state_near_to_far.max_delay,    
                     dm1_state_near_to_far.avg_delay_var,  
                     dm1_state_near_to_far.avg_n_delay_var,
                     dm1_state_near_to_far.min_delay_var,
                     dm1_state_near_to_far.max_delay_var,
                     dm1_state_near_to_far.ovrflw_cnt);
        cyg_httpd_write_chunked(p->outbuffer, ct); //3

        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u|",
                      pm_conf.enable);
        cyg_httpd_write_chunked(p->outbuffer, ct); //2

        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s",  mep_web_error_txt(error));

        cyg_httpd_write_chunked(p->outbuffer, ct); //4

        error = VTSS_RC_OK;

        cyg_httpd_end_chunked();
    }
    return -1; // Do not further search the file system.
}

// Status
static cyg_int32 handler_status_mep(CYG_HTTPD_STATE* p)
{
#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_STAT_TYPE, VTSS_MODULE_ID_MEP))
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

#define MEP_WEB_BUF_LEN 512

static size_t mep_lib_config_js(char **base_ptr, char **cur_ptr, size_t *length)
{
    char buff[MEP_WEB_BUF_LEN];
    (void) snprintf(buff, MEP_WEB_BUF_LEN,
                    "var configMepMin = %d;\n"
                    "var configMepMax = %d;\n",
                    1,
                    MEP_INSTANCE_MAX
        );
    return webCommonBufferHandler(base_ptr, cur_ptr, length, buff);
}

/****************************************************************************/
/*  JS lib config table entry                                               */
/****************************************************************************/

web_lib_config_js_tab_entry(mep_lib_config_js);


/****************************************************************************/
/*  HTTPD Table Entries                                                     */
/****************************************************************************/

CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_mep_create, "/config/mepCreate", handler_config_mep_create);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_mep, "/config/mepConfig", handler_config_mep);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_fm_config_mep, "/config/mepFmConfig", handler_config_fm_mep);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_pm_config_mep, "/config/mepPmConfig", handler_config_pm_mep);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_status_mep, "/stat/mep_status", handler_status_mep);

/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
