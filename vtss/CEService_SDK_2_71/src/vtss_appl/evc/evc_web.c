/*

   Vitesse Switch API software.

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

#include "web_api.h"

#ifdef VTSS_SW_OPTION_PRIV_LVL
#include "vtss_privilege_api.h"
#include "vtss_privilege_web_api.h"
#endif
#include "mgmt_api.h"
#include "port_api.h"
#include "evc_api.h"
#include "vlan_api.h"
#if defined(VTSS_ARCH_SERVAL)
#include "misc_api.h"   // misc_oui_addr_txt()
#endif /* VTSS_ARCH_SERVAL */

#define EVC_WEB_BUF_LEN 512


/* =================
 * Trace definitions
 * -------------- */
#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>
#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_WEB
#include <vtss_trace_api.h>
/* ============== */


/****************************************************************************/
// Convert from internal to user.
/****************************************************************************/
static inline vtss_evc_id_t evc_id_i2u(vtss_evc_id_t evc_id)
{
    return ((evc_id + 1) > EVC_ID_COUNT ? EVC_ID_COUNT :  evc_id + 1);
}

static inline vtss_evc_policer_id_t policer_id_i2u(vtss_evc_policer_id_t policer_id)
{
    return ((policer_id + 1) > EVC_POL_COUNT ? EVC_POL_COUNT :  policer_id + 1);
}

/****************************************************************************/
// Convert from user to internal.
/****************************************************************************/
static inline vtss_evc_id_t evc_id_u2i(vtss_evc_id_t evc_id)
{
    return (evc_id ? evc_id - 1 : evc_id);
}

static inline vtss_evc_policer_id_t policer_id_u2i(vtss_evc_policer_id_t policer_id)
{
    return (policer_id ? policer_id - 1 : policer_id);
}

/****************************************************************************/
/*  Web Handler  functions                                                  */
/****************************************************************************/
static cyg_int32 handler_config_evc(CYG_HTTPD_STATE *p)
{
    int                 ct;
    int                 ece_flag = 0, var_value = 0;
    char                buf[MGMT_PORT_BUF_SIZE];
    vtss_evc_id_t       ievc_id = 0, uevc_id = 0;
    evc_mgmt_conf_t     conf;
#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
    int                 policer_id_filter;
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_EVC)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_GET) {    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        // Format       : [evcConfigFlag]/[selectEvcId]
        // <Edit>       :               1/[evc_id]
        // <Delete>     :               2/[evc_id]
        // <Delete All> :               3/
        // <Add New>    :               4/

        if (cyg_httpd_form_varable_int(p, "evcConfigFlag", &ece_flag)) {
            switch (ece_flag) {
            case 1:
            case 2:
                if (cyg_httpd_form_varable_int(p, "selectEvcId", &var_value)) {
                    uevc_id = (vtss_evc_id_t) var_value;
                    ievc_id = evc_id_u2i(uevc_id);
                    if (ece_flag == 2) {
                        (void) evc_mgmt_del(ievc_id);
                    }
                }
                break;
            case 3:
                ievc_id = EVC_ID_FIRST;
                while (evc_mgmt_get(&ievc_id, &conf, TRUE) == VTSS_OK) {
                    (void) evc_mgmt_del(ievc_id);
                    ievc_id = EVC_ID_FIRST;
                }
                break;
            default:
                break;
            }
        }

        //Format: <evc_id>/<vid>/<ivid>/<learning>
        //        /<policer_id_filter:JR1/SRVL>/<policer_id:JR1/SRVL>/<it_type:Lu26>/<it_vid_mode:Lu26>/<it_vid:Lu26>/<it_preserve:Lu26>/<it_pcp:Lu26>/<it_dei:Lu26>/<ot_vid:Lu26>
        //        /<nni_port_0>/<nni_port_1>/...|...

        (void) cyg_httpd_start_chunked("html");

        // Sendout each entry information
        ievc_id = EVC_ID_FIRST;
        while (evc_mgmt_get(&ievc_id, &conf, TRUE) == VTSS_OK) {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/%u/%u/%u",
                          evc_id_i2u(ievc_id),
                          conf.conf.network.pb.vid,
                          conf.conf.network.pb.ivid,
                          conf.conf.learning);
            (void) cyg_httpd_write_chunked(p->outbuffer, ct);

#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
            // policer_id_filter ("Specific", "Discard", "None")
            if (conf.conf.policer_id == VTSS_EVC_POLICER_ID_DISCARD) {
                policer_id_filter = 1;
            } else if (conf.conf.policer_id == VTSS_EVC_POLICER_ID_NONE) {
                policer_id_filter = 2;
            } else {
                policer_id_filter = 0;
            }

            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u/%u",
                          policer_id_filter,
                          policer_id_filter == 0 ? policer_id_i2u(conf.conf.policer_id) : 1);
            (void) cyg_httpd_write_chunked(p->outbuffer, ct);
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */

#if defined(VTSS_ARCH_CARACAL)
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u/%u/%u/%u/%u/%u/%u",
                          /* it_type     */   conf.conf.network.pb.inner_tag.type,
                          /* it_mode     */   conf.conf.network.pb.inner_tag.vid_mode,
                          /* it_vid      */   conf.conf.network.pb.inner_tag.vid,
                          /* it_preserve */   conf.conf.network.pb.inner_tag.pcp_dei_preserve,
                          /* it_pcp      */   conf.conf.network.pb.inner_tag.pcp,
                          /* it_dei      */   conf.conf.network.pb.inner_tag.dei,
                          /* ot_vid      */   conf.conf.network.pb.uvid);
            (void) cyg_httpd_write_chunked(p->outbuffer, ct);
#endif /* VTSS_ARCH_CARACAL */

            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%s|",
                          /* uni_ports */ mgmt_iport_list2txt(conf.conf.network.pb.nni, buf));
            (void) cyg_httpd_write_chunked(p->outbuffer, ct);
        }

        (void) cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

static cyg_int32 handler_config_evc_edit(CYG_HTTPD_STATE *p)
{
    vtss_evc_id_t       ievc_id = 0, uevc_id = 0;
    evc_mgmt_conf_t     conf, newconf;
    char                var_name[64];
    int                 evc_flag = 0, var_value = 0;
    int                 ct;
    port_iter_t         pit;
    vtss_port_no_t      iport;
    vtss_uport_no_t     uport;
    evc_port_info_t     info[VTSS_PORT_ARRAY_SIZE];
#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
    int                 policer_id_filter;
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_EVC)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {

        if (cyg_httpd_form_varable_int(p, "evc_id", &var_value)) {
            uevc_id = (vtss_evc_id_t) var_value;
            ievc_id = evc_id_u2i(uevc_id);

            memset(&conf, 0, sizeof(conf));
            (void) evc_mgmt_get(&ievc_id, &conf, FALSE);
            newconf = conf;

            // vid
            if (cyg_httpd_form_varable_int(p, "vid", &var_value)) {
                newconf.conf.network.pb.vid = (vtss_vid_t) var_value;
            }

            // ivid
            if (cyg_httpd_form_varable_int(p, "ivid", &var_value)) {
                newconf.conf.network.pb.ivid = (vtss_vid_t) var_value;
            }

            // learning
            if (cyg_httpd_form_varable_int(p, "learning", &var_value)) {
                newconf.conf.learning = (BOOL) var_value;
            }

#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
            // policer_id_filter ("Specific", "Discard", "None")
            // policer_id
            if (cyg_httpd_form_varable_int(p, "policer_id_filter", &var_value)) {
                if (var_value == 1) {
                    newconf.conf.policer_id = VTSS_EVC_POLICER_ID_DISCARD;
                } else if (var_value == 2) {
                    newconf.conf.policer_id = VTSS_EVC_POLICER_ID_NONE;
                } else {
                    if (cyg_httpd_form_varable_int(p, "policer_id", &var_value)) {
                        newconf.conf.policer_id = policer_id_u2i((vtss_evc_policer_id_t) var_value);
                    }
                }
            }
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */

#if defined(VTSS_ARCH_CARACAL)
            // it_type ("None", "C-tag", "S-tag", "S-custom-tag")
            if (cyg_httpd_form_varable_int(p, "it_type", &var_value)) {
                if (var_value == 3) {
                    newconf.conf.network.pb.inner_tag.type = VTSS_EVC_INNER_TAG_S_CUSTOM;
                } else if (var_value == 2) {
                    newconf.conf.network.pb.inner_tag.type = VTSS_EVC_INNER_TAG_S;
                } else if (var_value == 1) {
                    newconf.conf.network.pb.inner_tag.type = VTSS_EVC_INNER_TAG_C;
                } else {
                    newconf.conf.network.pb.inner_tag.type = VTSS_EVC_INNER_TAG_NONE;
                }
            }

            // it_vid_mode
            if (cyg_httpd_form_varable_int(p, "it_vid_mode", &var_value)) {
                if (var_value == 0) {
                    newconf.conf.network.pb.inner_tag.vid_mode = VTSS_EVC_VID_MODE_NORMAL;
                } else {
                    newconf.conf.network.pb.inner_tag.vid_mode = VTSS_EVC_VID_MODE_TUNNEL;
                }
            }

            // it_vid
            if (cyg_httpd_form_varable_int(p, "it_vid", &var_value)) {
                newconf.conf.network.pb.inner_tag.vid = (vtss_vid_t) var_value;
            }

            // it_preserve
            if (cyg_httpd_form_varable_int(p, "it_preserve", &var_value)) {
                newconf.conf.network.pb.inner_tag.pcp_dei_preserve = (BOOL) var_value;
            }

            // it_pcp
            if (cyg_httpd_form_varable_int(p, "it_pcp", &var_value)) {
                newconf.conf.network.pb.inner_tag.pcp = (vtss_tagprio_t) var_value;
            }

            // it_dei
            if (cyg_httpd_form_varable_int(p, "it_dei", &var_value)) {
                newconf.conf.network.pb.inner_tag.dei = (vtss_dei_t) var_value;
            }

            // ot_vid
            if (cyg_httpd_form_varable_int(p, "ot_vid", &var_value)) {
                newconf.conf.network.pb.uvid = (vtss_vid_t) var_value;
            }
#endif /* VTSS_ARCH_CARACAL */

            // NNI ports
            port_iter_init_local(&pit);
            while (port_iter_getnext(&pit)) {
                iport = pit.iport;
                uport = iport2uport(iport);
                sprintf(var_name, "nni_port_%u", uport);
                if (cyg_httpd_form_varable_find(p, var_name) != NULL) {
                    newconf.conf.network.pb.nni[iport] = TRUE;
                } else {
                    newconf.conf.network.pb.nni[iport] = FALSE;
                }
            }

            // Save new configuration
            if (memcmp(&newconf, &conf, sizeof(newconf))) {
                T_D("Calling evc_mgmt_add(%u)", ievc_id);
                if (evc_mgmt_add(ievc_id, &newconf) != VTSS_OK) {
                    T_W("evc_mgmt_add(%u): failed", ievc_id);
                }
            }
        }

        redirect(p, "/evc.htm");
    } else {    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        // Format    : [evcEditFlag]/[selectEvcId]
        // <Edit>    :             3/[evc_id]
        // <Add New> :             3/0

        if (cyg_httpd_form_varable_int(p, "evcEditFlag", &evc_flag)) {
            switch (evc_flag) {
            case 3:
                if (cyg_httpd_form_varable_int(p, "selectEvcId", &var_value)) {
                    uevc_id = (vtss_evc_id_t) var_value;
                    if (uevc_id) {
                        ievc_id = evc_id_u2i(uevc_id);
                    }
                }
                break;
            default:
                break;
            }
        }

        //Format: <evc_id>/<vid>/<ivid>/<learning>
        //        /<policer_id_filter:JR1/SRVL>/<policer_id:JR1/SRVL>/<it_type:Lu26>/<it_vid_mode:Lu26>/<it_vid:Lu26>/<it_preserve:Lu26>/<it_pcp:Lu26>/<it_dei:Lu26>/<ot_vid:Lu26>
        //        /<nni_port_0>/<nni_port_1>/...|...

        // Get UNI ports
        memset(info, 0, sizeof(info));
        (void) evc_mgmt_port_info_get(info);

        (void) cyg_httpd_start_chunked("html");

        // Sendout each entry information
        if (evc_flag == 3 && uevc_id && evc_mgmt_get(&ievc_id, &conf, FALSE) == VTSS_OK) {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/%u/%u/%u",
                          evc_id_i2u(ievc_id),
                          conf.conf.network.pb.vid,
                          conf.conf.network.pb.ivid,
                          conf.conf.learning);
            (void) cyg_httpd_write_chunked(p->outbuffer, ct);

#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
            // policer_id_filter ("Specific", "Discard", "None")
            if (conf.conf.policer_id == VTSS_EVC_POLICER_ID_DISCARD) {
                policer_id_filter = 1;
            } else if (conf.conf.policer_id == VTSS_EVC_POLICER_ID_NONE) {
                policer_id_filter = 2;
            } else {
                policer_id_filter = 0;
            }

            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u/%u",
                          policer_id_filter,
                          policer_id_filter == 0 ? policer_id_i2u(conf.conf.policer_id) : 1);
            (void) cyg_httpd_write_chunked(p->outbuffer, ct);
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */

#if defined(VTSS_ARCH_CARACAL)
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u/%u/%u/%u/%u/%u/%u",
                          /* it_type     */   conf.conf.network.pb.inner_tag.type,
                          /* it_mode     */   conf.conf.network.pb.inner_tag.vid_mode,
                          /* it_vid      */   conf.conf.network.pb.inner_tag.vid,
                          /* it_preserve */   conf.conf.network.pb.inner_tag.pcp_dei_preserve,
                          /* it_pcp      */   conf.conf.network.pb.inner_tag.pcp,
                          /* it_dei      */   conf.conf.network.pb.inner_tag.dei,
                          /* ot_vid      */   conf.conf.network.pb.uvid);
            (void) cyg_httpd_write_chunked(p->outbuffer, ct);
#endif /* VTSS_ARCH_CARACAL */

            port_iter_init_local(&pit);
            while (port_iter_getnext(&pit)) {
                iport = pit.iport;
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u", info[iport].uni_count ? 2 : conf.conf.network.pb.nni[iport]);
                (void) cyg_httpd_write_chunked(p->outbuffer, ct);
            }
        } else {
#if defined(VTSS_ARCH_CARACAL)
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "0/1/1/0/0/0/1/0/0/0/0");
#else
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "0/1/1/0/0/1");
#endif /* VTSS_ARCH_CARACAL */
            (void) cyg_httpd_write_chunked(p->outbuffer, ct);

            port_iter_init_local(&pit);
            while (port_iter_getnext(&pit)) {
                iport = pit.iport;
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u",
                              info[iport].uni_count ? 2 : 0);
                (void) cyg_httpd_write_chunked(p->outbuffer, ct);
            }
        }
        (void) cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

static cyg_int32 handler_config_evc_uni(CYG_HTTPD_STATE *p)
{
    evc_mgmt_port_conf_t    conf, newconf;
    port_iter_t             pit;
    vtss_port_no_t          iport;
    vtss_uport_no_t         uport;
    char                    var_name[32];
    int                     var_value;
    int                     ct;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_EVC)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {
        // Modify existing entry?
        port_iter_init_local(&pit);
        while (port_iter_getnext(&pit)) {
            iport = pit.iport;
            if (evc_mgmt_port_conf_get(iport, &conf) != VTSS_OK) {
                continue;
            }
            newconf = conf;
            uport = iport2uport(iport);

#if defined(VTSS_ARCH_SERVAL)
            //key_type
            sprintf(var_name, "key_type_%u", uport);
            if (cyg_httpd_form_varable_int(p, var_name, &var_value)) {
                newconf.conf.key_type = (vtss_vcap_key_type_t) var_value;
            }

            //advanced_key_type
            sprintf(var_name, "advanced_key_type_%u", uport);
            if (cyg_httpd_form_varable_int(p, var_name, &var_value)) {
                newconf.vcap_conf.key_type_is1_1 = (vtss_vcap_key_type_t) var_value;
            }
#endif /* VTSS_ARCH_SERVAL */
           
#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_CARACAL)
            //dei_mode
            sprintf(var_name, "dei_mode_%u", uport);
            if (cyg_httpd_form_varable_int(p, var_name, &var_value)) {
                newconf.conf.dei_colouring = (BOOL) var_value;
            }
#endif /* VTSS_ARCH_JAGUAR_1/CARACAL */

#if defined(VTSS_ARCH_CARACAL)
            //tag_mode
            sprintf(var_name, "tag_mode_%u", uport);
            if (cyg_httpd_form_varable_int(p, var_name, &var_value)) {
                newconf.conf.inner_tag = (BOOL) var_value;
            }
#endif /* VTSS_ARCH_CARACAL */

#if defined(VTSS_ARCH_CARACAL) || defined(VTSS_ARCH_SERVAL)
            //addr_mode
            sprintf(var_name, "addr_mode_%u", uport);
            if (cyg_httpd_form_varable_int(p, var_name, &var_value)) {
                newconf.conf.dmac_dip = (BOOL)var_value;
            }
#endif /* VTSS_ARCH_CARACAL || VTSS_ARCH_SERVAL */

#if defined(VTSS_ARCH_SERVAL)
            //advanced_addr_mode
            sprintf(var_name, "advanced_addr_mode_%u", uport);
            if (cyg_httpd_form_varable_int(p, var_name, &var_value)) {
                newconf.vcap_conf.dmac_dip_1 = (BOOL)var_value;
            }
#endif /* VTSS_ARCH_SERVAL */

            // Save new configuration
            if (memcmp(&newconf, &conf, sizeof(newconf))) {
                T_D("Calling evc_mgmt_port_conf_set(%u)", iport);
                if (evc_mgmt_port_conf_set(iport, &newconf) != VTSS_OK) {
                    T_W("evc_mgmt_port_conf_set(%u): failed", iport);
                }
            }
        }

        redirect(p, "/evc_uni.htm");
    } else {    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        // Format:
        // Serval - [[port_no]/[key_type]/[advanced_key_type]/[addr_mode]/[advanced_addr_mode]|...
        // JR1    - [port_no]/[dei_mode]|...
        // Lu26   - [port_no]/[dei_mode]/[tag_mode]/[addr_mode]|...

        (void) cyg_httpd_start_chunked("html");
        port_iter_init_local(&pit);
        while (port_iter_getnext(&pit)) {
            iport = pit.iport;
            if (evc_mgmt_port_conf_get(iport, &conf) != VTSS_OK) {
                continue;
            }
            uport = iport2uport(iport);
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u", uport);
            (void) cyg_httpd_write_chunked(p->outbuffer, ct);

#if defined(VTSS_ARCH_SERVAL)
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u/%u/%u/%u",
                          conf.conf.key_type,
                          conf.conf.dmac_dip,
                          conf.vcap_conf.key_type_is1_1,
                          conf.vcap_conf.dmac_dip_1);
            (void) cyg_httpd_write_chunked(p->outbuffer, ct);
#else
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u", conf.conf.dei_colouring);
            (void) cyg_httpd_write_chunked(p->outbuffer, ct);
#if defined(VTSS_ARCH_CARACAL)
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u/%u",
                          conf.conf.inner_tag,
                          conf.conf.dmac_dip);
            (void) cyg_httpd_write_chunked(p->outbuffer, ct);
#endif /* VTSS_ARCH_CARACAL */ 
#endif /* VTSS_ARCH_SERVAL */

            (void) cyg_httpd_write_chunked("|", 1);
        }

        (void) cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

#if defined(VTSS_ARCH_SERVAL)
static cyg_int32 handler_config_evc_l2cp(CYG_HTTPD_STATE *p)
{
    evc_mgmt_port_conf_t    conf, newconf;
    vtss_port_no_t          iport = VTSS_PORT_NO_START, uport;
    char                    var_name[32];
    int                     var_value;
    int                     ct, idx;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_EVC)) {
        return -1;
    }
#endif

    if (cyg_httpd_form_varable_int(p, "port", &var_value)) {
        iport = uport2iport(var_value);
    }
    uport = iport2uport(iport);

    if (p->method == CYG_HTTPD_METHOD_POST) {
        if (evc_mgmt_port_conf_get(iport, &conf) == VTSS_OK) {
            newconf = conf;
        
            for (idx = 0; idx < 32; ++idx) {
                sprintf(var_name, "l2cp_mode_%u", idx);
                if (cyg_httpd_form_varable_int(p, var_name, &var_value) &&
                    (var_value == 1 /* forward */ || var_value == 2 /* discard */ || var_value == 4 /* peer */)) {
                    if (idx < 16) {
                        newconf.reg.bpdu_reg[idx] = (vtss_packet_reg_type_t) var_value;
                    } else {
                        newconf.reg.garp_reg[idx - 16] = (vtss_packet_reg_type_t) var_value;
                    }
                }
            }
            
            // Save new configuration
            if (memcmp(&newconf, &conf, sizeof(newconf))) {
                T_D("Calling evc_mgmt_port_conf_set(%u)", iport);
                if (evc_mgmt_port_conf_set(iport, &newconf) != VTSS_OK) {
                    T_W("evc_mgmt_port_conf_set(%u): failed", iport);
                }
            }
        }

        sprintf(var_name, "/evc_l2cp.htm?port=%u", uport);
        redirect(p, var_name);
    } else {    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        // Format: [select_port],[l2cp_mode]/...

        (void) cyg_httpd_start_chunked("html");

        if (evc_mgmt_port_conf_get(iport, &conf) == VTSS_OK) {
            for (idx = 0; idx < 16; ++idx) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/", conf.reg.bpdu_reg[idx]);
                (void) cyg_httpd_write_chunked(p->outbuffer, ct);
            }
            for (idx = 0; idx < 16; ++idx) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/", conf.reg.garp_reg[idx]);
                (void) cyg_httpd_write_chunked(p->outbuffer, ct);
            }
        }
        (void) cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}
#endif /* VTSS_ARCH_SERVAL*/

static cyg_int32 handler_config_evc_bw(CYG_HTTPD_STATE *p)
{
    vtss_evc_policer_id_t   start_policer_id = 1, ipolicer_id, upolicer_id;
    int                     num_of_entries = 2;
    int                     get_next_entry = 0;
    evc_mgmt_policer_conf_t conf, newconf;
    const char              *var_string;
    char                    var_name[64];
    int                     var_value;
    u32                     var_u32_value = 0;
    int                     ct;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_EVC)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {
        // Modify existing entry?
        for (ipolicer_id = 0; ipolicer_id < EVC_POL_COUNT; ipolicer_id++) {
            // state
            upolicer_id = policer_id_i2u(ipolicer_id);
            sprintf(var_name, "state_%u", upolicer_id);
            if (cyg_httpd_form_varable_int(p, var_name, &var_value) &&
                evc_mgmt_policer_conf_get(ipolicer_id, &conf) == VTSS_OK) {
                newconf = conf;
                newconf.conf.enable = (BOOL) var_value;
            } else {
                continue;
            }

            // type
            upolicer_id = policer_id_i2u(ipolicer_id);
            sprintf(var_name, "type_%u", upolicer_id);
            if (cyg_httpd_form_varable_int(p, var_name, &var_value)) {
                newconf.conf.type = var_value;
            }

            // mode
            sprintf(var_name, "mode_%u", upolicer_id);
            if (cyg_httpd_form_varable_int(p, var_name, &var_value)) {
                if (var_value == 0) {   // "Coupled": 0, "Aware": 1, "Blind": 2
#if defined(VTSS_ARCH_JAGUAR_1)
                    newconf.conf.cm = TRUE;
#endif /* VTSS_ARCH_JAGUAR_1 */
                    newconf.conf.cf = TRUE;
                } else if (var_value == 1) {
#if defined(VTSS_ARCH_JAGUAR_1)
                    newconf.conf.cm = TRUE;
#endif /* VTSS_ARCH_JAGUAR_1 */
                    newconf.conf.cf = FALSE;
                } else {
#if defined(VTSS_ARCH_JAGUAR_1)
                    newconf.conf.cm = FALSE;
#endif /* VTSS_ARCH_JAGUAR_1 */
                    newconf.conf.cf = FALSE;
                }
            }

            // rate_type
            sprintf(var_name, "rate_type_%u", upolicer_id);
            if (cyg_httpd_form_varable_int(p, var_name, &var_value)) {
                newconf.conf.line_rate = (BOOL) var_value;
            }

            // cir
            sprintf(var_name, "cir_%u", upolicer_id);
            if (cyg_httpd_form_varable_long_int(p, var_name, &var_u32_value)) {
                newconf.conf.cir = var_u32_value;
            }

            // cbs
            sprintf(var_name, "cbs_%u", upolicer_id);
            if (cyg_httpd_form_varable_long_int(p, var_name, &var_u32_value)) {
                newconf.conf.cbs = var_u32_value;
            }

            // eir
            sprintf(var_name, "eir_%u", upolicer_id);
            if (cyg_httpd_form_varable_long_int(p, var_name, &var_u32_value)) {
                newconf.conf.eir = var_u32_value;
            }

            // ebs
            sprintf(var_name, "ebs_%u", upolicer_id);
            if (cyg_httpd_form_varable_long_int(p, var_name, &var_u32_value)) {
                newconf.conf.ebs = var_u32_value;
            }

            // Save new configuration
            if (memcmp(&newconf, &conf, sizeof(newconf))) {
                T_D("Calling evc_mgmt_policer_conf_set(%u)", ipolicer_id);
                if (evc_mgmt_policer_conf_set(ipolicer_id, &newconf) != VTSS_OK) {
                    T_W("evc_mgmt_policer_conf_set(%u): failed", ipolicer_id);
                }
            }
        }

        (void) cyg_httpd_form_varable_int(p, "startPolicerId", &var_value);
        (void) cyg_httpd_form_varable_int(p, "numberOfEntries", &num_of_entries);
        sprintf(var_name, "/evc_bw.htm?startPolicerId=%u&numberOfEntries=%u", var_value, num_of_entries);
        redirect(p, var_name);
    } else {    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        // Format: <start_policer_id>,<num_of_entries>,[policer_id]/[state]/[type]/[mode]/[rate_type]/[cir]/[cbs]/[eir]/[ebs]|...

        (void) cyg_httpd_start_chunked("html");

        // Get start policer_id
        if ((var_string = cyg_httpd_form_varable_find(p, "startPolicerId")) != NULL) {
            start_policer_id = atoi(var_string);
        }

        // Get number of entries per page
        if ((var_string = cyg_httpd_form_varable_find(p, "numberOfEntries")) != NULL) {
            num_of_entries = atoi(var_string);
        }
        if (num_of_entries < 0 || num_of_entries > EVC_POL_COUNT) {
            num_of_entries = 20;
        }

        // Get or GetNext
        if ((var_string = cyg_httpd_form_varable_find(p, "GetNextEntry")) != NULL) {
            get_next_entry = atoi(var_string);
            if (get_next_entry) {
                start_policer_id++;
            }
            if (start_policer_id > EVC_POL_COUNT) {
                start_policer_id = EVC_POL_COUNT;
            }
        }

        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u,%u,",
                      start_policer_id,
                      num_of_entries);
        (void) cyg_httpd_write_chunked(p->outbuffer, ct);

        // Sendout each entry information
        for (ipolicer_id = 0; ipolicer_id < num_of_entries; ipolicer_id++) {
            if (policer_id_u2i(start_policer_id) + ipolicer_id >= EVC_POL_COUNT) {
                break;
            }
            if (evc_mgmt_policer_conf_get(policer_id_u2i(start_policer_id) + ipolicer_id, &conf) != VTSS_OK) {
                continue;
            }

            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/%u/%u/%u/%u/%u/%u/%u/%u|",
                          start_policer_id + ipolicer_id,
                          conf.conf.enable,
                          conf.conf.type,
#if defined(VTSS_ARCH_JAGUAR_1)
                          conf.conf.cm == 0 ? 2 :
#endif /* VTSS_ARCH_JAGUAR_1 */
                          conf.conf.cf ? 0 : 1, // "Coupled": 0, "Aware": 1, "Blind": 2
                          conf.conf.line_rate ? 1 : 0,
                          conf.conf.cir,
                          conf.conf.cbs,
                          conf.conf.eir,
                          conf.conf.ebs);
            (void) cyg_httpd_write_chunked(p->outbuffer, ct);
        }

        (void) cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

// range_filter
// ("Any", "Specific", "Range")
// (    0,          1,       2)
static int handler_get_filter_range_mapping(vtss_vcap_vr_t *vr)
{
    int filter;

    if (vr->type == VTSS_VCAP_VR_TYPE_VALUE_MASK) {
        filter = 0;
    } else if (vr->vr.r.low == vr->vr.r.high) {
        filter = 1;
    } else {
        filter = 2;
    }

    return filter;
}

static BOOL is_matched_mask_range(u32 low, u32 high, u32 min, u32 max)
{
    u32 mask;

    if (low < min || high > max || low > high) {
        return FALSE;
    }

    for (mask = 0; mask <= max; mask = (mask * 2 + 1)) {
        if ((low & ~mask) == (high & ~mask) && /* Upper bits match */
            (low & mask) == 0 &&                /* Lower bits of 'low' are zero */
            (high | mask) == high) {            /* Lower bits of 'high are one */
             return TRUE;
        }
    }

    return FALSE;
}

static vtss_rc handler_parse_filter_range_mapping(CYG_HTTPD_STATE *p, char *filter_name, vtss_vcap_vr_t *vr, u32 min_value, u32 max_value, BOOL is_needed_mask_range)
{
    char  var_name[24];
    int   filter_value, low_value, high_vlaue;

    memset(vr, 0, sizeof(*vr));
    vr->type = VTSS_VCAP_VR_TYPE_VALUE_MASK;
    sprintf(var_name, "%s_filter", filter_name);
    if (cyg_httpd_form_varable_int(p, var_name, &filter_value)) {
        if (filter_value) {
            sprintf(var_name, "%s_low", filter_name);
            if (cyg_httpd_form_varable_int(p, var_name, &low_value)) {
                if (filter_value == 1) {
                    vr->type = VTSS_VCAP_VR_TYPE_RANGE_INCLUSIVE;
                    vr->vr.r.low = low_value;
                    vr->vr.r.high = low_value;
                } else {
                    sprintf(var_name, "%s_high", filter_name);
                    if (cyg_httpd_form_varable_int(p, var_name, &high_vlaue)) {
                        if (((u32) low_value < min_value || (u32) high_vlaue > max_value || low_value > high_vlaue) ||
                            (is_needed_mask_range && !is_matched_mask_range((u32) low_value, (u32) high_vlaue, min_value, max_value))) {
                            return VTSS_RC_ERROR;
                        }
                        if (low_value != min_value || high_vlaue != max_value) {
                            vr->type = VTSS_VCAP_VR_TYPE_RANGE_INCLUSIVE;
                            vr->vr.r.low = low_value; 
                            vr->vr.r.high = high_vlaue;
                        }
                    }
                }
            }
        }

        if (vr->type == VTSS_VCAP_VR_TYPE_RANGE_INCLUSIVE) {
            if (vr->vr.r.low == min_value && vr->vr.r.high == max_value) { // match the any condition
                memset(vr, 0, sizeof(*vr));
                vr->type = VTSS_VCAP_VR_TYPE_VALUE_MASK;
            }
        }
    }

    return VTSS_OK;
}

// pcp_range_mapping
// ("Any", "0", "1", "2", "3", "4", "5", "6", "7", "0-1", "2-3", "4-5", "6-7", "0-3", "4-7")
// (   14,   0,   1,   2,   3,   4,   5,   6,   7,     8,     9,    10,    11,    12,    13)
static int handler_get_pcp_range_mapping(vtss_vcap_u8_t *pcp)
{
    int pcp_mapping_vlaue = 0;

    if (pcp->mask == 0) {
        pcp_mapping_vlaue = 14;
    } else if (pcp->mask == 7) {
        pcp_mapping_vlaue = pcp->value;
    } else if (pcp->mask == 6) {
        if (pcp->value == 0) {
            pcp_mapping_vlaue = 8;
        } else if (pcp->value == 2) {
            pcp_mapping_vlaue = 9;
        } else if (pcp->value == 4) {
            pcp_mapping_vlaue = 10;
        } else {
            pcp_mapping_vlaue = 11;
        }
    } else {
        if (pcp->value == 0) {
            pcp_mapping_vlaue = 12;
        } else {
            pcp_mapping_vlaue = 13;
        }
    }

    return pcp_mapping_vlaue;
}

static void handler_parse_pcp_range_mapping(CYG_HTTPD_STATE *p, char *pcp_name, vtss_vcap_u8_t *pcp)
{
    int var_value;

    if (cyg_httpd_form_varable_int(p, pcp_name, &var_value)) {
        switch (var_value) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
            pcp->value = (u8) var_value;
            pcp->mask = 7;
            break;
        case 8:
            pcp->value = 0;
            pcp->mask = 6;
            break;
        case 9:
            pcp->value = 2;
            pcp->mask = 6;
            break;
        case 10:
            pcp->value = 4;
            pcp->mask = 6;
            break;
        case 11:
            pcp->value = 6;
            pcp->mask = 6;
            break;
        case 12:
            pcp->value = 0;
            pcp->mask = 4;
            break;
        case 13:
            pcp->value = 4;
            pcp->mask = 4;
            break;
        default:
            break;
        }
    }
}

static cyg_int32 handler_config_evc_ece(CYG_HTTPD_STATE *p)
{
    vtss_ece_id_t       ece_id;
    evc_mgmt_ece_conf_t conf, newconf;
    vtss_rc             rc;
    int                 ct;
    BOOL                iport_list[VTSS_PORT_ARRAY_SIZE];
    port_iter_t         pit;
    vtss_port_no_t      iport;
    char                buf[MGMT_PORT_BUF_SIZE];
    int                 ece_flag = 0, var_value = 0;
    int                 vid_filter = 0, evc_id_filter = 0;
#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
    int                 policer_id_filter = 0;
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_EVC)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_GET) {
        // Format       : [eceConfigFlag]/[selectEceId]
        // <Delete>     :               1/[ece_id]
        // <Move>       :               2/[ece_id]
        // <Delete All> :               3/

        if (cyg_httpd_form_varable_int(p, "eceConfigFlag", &ece_flag)) {
            switch (ece_flag) {
            case 1:
                if (cyg_httpd_form_varable_int(p, "selectEceId", &var_value)) {
                    ece_id = (vtss_ece_id_t) var_value;
                    (void) evc_mgmt_ece_del(ece_id);
                }
                break;
            case 2:
                if (cyg_httpd_form_varable_int(p, "selectEceId", &var_value)) {
                    ece_id = (vtss_ece_id_t) var_value;
                    if (evc_mgmt_ece_get(ece_id, &conf, TRUE) == VTSS_OK) {
                        if (evc_mgmt_ece_add(ece_id, &conf) != VTSS_OK) {
                            T_W("evc_mgmt_ece_add(%u, %u): failed", ece_id, conf.conf.id);
                        }
                    }
                }
                break;
            case 3:
                ece_id = EVC_ECE_ID_FIRST;
                while (evc_mgmt_ece_get(ece_id, &conf, TRUE) == VTSS_OK) {
                    (void) evc_mgmt_ece_del(conf.conf.id);
                }
                break;
            default:
                break;
            }
        }

        // Format: [ece_id]/[next_ece_id]/[uni_ports]/[tag_type]/[vid_filter]/[vid_low]/[vid_high]/[pcp]/[dei]
        //          /[frame_type]/[direction]/[evc_id_filter]/[evc_id]
        //          /[policer_id_filter:JR1/SRVL]/[policer_id:JR1/SRVL]/[pop]/[policy]/[class:Lu26]
        //          /[ot_mode]/[ot_vid:JR1/SRVL]/[ot_preserve]/[ot_pcp]/[ot_dei_mode:SRVL]/[ot_dei]/[conflict]|...

        (void) cyg_httpd_start_chunked("html");
        conf.conf.id = EVC_ECE_ID_FIRST;
        while (evc_mgmt_ece_get(conf.conf.id, &conf, TRUE) == VTSS_OK) {
            port_iter_init_local_all(&pit);
            while (port_iter_getnext(&pit)) {
                iport = pit.iport;
                iport_list[iport] = (conf.conf.key.port_list[iport] == VTSS_ECE_PORT_NONE ? 0 : 1);
            }

            // next_ece_id
            rc = evc_mgmt_ece_get(conf.conf.id, &newconf, TRUE);

            // vid_filter ("Any", "Specific", "Range")
            vid_filter = handler_get_filter_range_mapping(&conf.conf.key.tag.vid);

            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/%u/%s/%u/%u/%u/%u/%u/%u/%u/%u",
                          /* ece_id        */   conf.conf.id,
                          /* next_ece_id   */   rc == VTSS_OK ? newconf.conf.id : VTSS_ECE_ID_LAST,
                          /* uni_ports     */   mgmt_iport_list2txt(iport_list, buf),
                          /* tag_type      */   conf.conf.key.tag.tagged == VTSS_VCAP_BIT_ANY ? 0 : conf.conf.key.tag.tagged == VTSS_VCAP_BIT_0 ? 1 : conf.conf.key.tag.s_tagged == VTSS_VCAP_BIT_0 ? 2 : conf.conf.key.tag.s_tagged == VTSS_VCAP_BIT_1 ? 3 : 4,
                          /* vid_filter    */   vid_filter,
                          /* vid_low       */   vid_filter == 0 ? 0 : vid_filter == 1 ? conf.conf.key.tag.vid.vr.v.value : conf.conf.key.tag.vid.vr.r.low,
                          /* vid_high      */   vid_filter == 2 ? conf.conf.key.tag.vid.vr.r.high : VLAN_ID_MAX,
                          /* pcp           */   handler_get_pcp_range_mapping(&conf.conf.key.tag.pcp),
                          /* dei           */   conf.conf.key.tag.dei,
#if defined(VTSS_ARCH_SERVAL)
                          /* frame_type    */   conf.data.l2cp.proto != EVC_L2CP_NONE ? 6 : conf.conf.key.type,
#else
                          /* frame_type    */   conf.conf.key.type,
#endif /* VTSS_ARCH_SERVAL */
                          /* direction     */   conf.conf.action.dir);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            // evc_id_filter ("None", "Specific")
            if (conf.conf.action.evc_id == VTSS_EVC_ID_NONE) {
                evc_id_filter = 0;
            } else {
                evc_id_filter = 1;
            }

            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u/%u",
                          /* evc_id_filter */   evc_id_filter,
                          /* evc_id        */   evc_id_filter == 1 ? evc_id_i2u(conf.conf.action.evc_id) : 1);
            (void) cyg_httpd_write_chunked(p->outbuffer, ct);

#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
            // policer_id_filter ("Specific", "Discard", "None", "EVC")
            if (conf.conf.action.policer_id == VTSS_EVC_POLICER_ID_DISCARD) {
                policer_id_filter = 1;
            } else if (conf.conf.action.policer_id == VTSS_EVC_POLICER_ID_NONE) {
                policer_id_filter = 2;
            } else if (conf.conf.action.policer_id == VTSS_EVC_POLICER_ID_EVC) {
                policer_id_filter = 3;
            } else {
                policer_id_filter = 0;
            }
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u/%u",
                          /* policer_id_filter */   policer_id_filter,
                          /* policer_id        */   policer_id_filter == 0 ? policer_id_i2u(conf.conf.action.policer_id) : 1);
            (void) cyg_httpd_write_chunked(p->outbuffer, ct);
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */

            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u/%u",
                          /* pop       */   conf.conf.action.pop_tag,
                          /* policy_no */   conf.conf.action.policy_no);
            (void) cyg_httpd_write_chunked(p->outbuffer, ct);

#if defined(VTSS_ARCH_CARACAL)
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u",
                          /* class */   conf.conf.action.prio_enable ? conf.conf.action.prio : VTSS_PRIOS);
            (void) cyg_httpd_write_chunked(p->outbuffer, ct);
#endif /* VTSS_ARCH_CARACAL/SERVAL */

            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u",
                          /* ot_mode */  conf.conf.action.outer_tag.enable);
            (void) cyg_httpd_write_chunked(p->outbuffer, ct);

#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u",
                          /* ot_vid */  conf.conf.action.outer_tag.vid);
            (void) cyg_httpd_write_chunked(p->outbuffer, ct);
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */

            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u/%u",
#if defined(VTSS_ARCH_SERVAL)
                          /* ot_preserve       */   conf.conf.action.outer_tag.pcp_mode,
#else
                          /* ot_preserve       */   conf.conf.action.outer_tag.pcp_dei_preserve,
#endif /* VTSS_ARCH_SERVAL */
                          /* ot_pcp            */   conf.conf.action.outer_tag.pcp);
            (void) cyg_httpd_write_chunked(p->outbuffer, ct);

#if defined(VTSS_ARCH_SERVAL)
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u",
                          /* ot_dei_mode       */   conf.conf.action.outer_tag.dei_mode);
            (void) cyg_httpd_write_chunked(p->outbuffer, ct);
#endif /* VTSS_ARCH_SERVAL */

            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u/%u|",
                          /* ot_dei            */   conf.conf.action.outer_tag.dei,
                          /* conflict          */   conf.conflict);
            (void) cyg_httpd_write_chunked(p->outbuffer, ct);
        }
        (void) cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

static cyg_int32 handler_config_evc_ece_edit(CYG_HTTPD_STATE *p)
{
    vtss_ece_id_t       ece_id = VTSS_EVC_ID_NONE, next_ece_id = VTSS_ECE_ID_LAST;
    evc_mgmt_ece_conf_t conf, newconf;
    vtss_rc             rc;
    int                 ct;
    port_iter_t         pit;
    vtss_port_no_t      iport;
    vtss_uport_no_t     uport;
    int                 err_rc = 0, ece_flag = 0, var_value = 0;
    int                 common_filter = 0, range_filter = 0;
#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
    int                 policer_id_filter = 0;
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */
    char                var_name[16];
    evc_port_info_t     info[VTSS_PORT_ARRAY_SIZE];
#if defined(VTSS_ARCH_CARACAL) || defined(VTSS_ARCH_SERVAL)
    int                 dmac_type = 0;
    char                str_buff[24], str_buff1[24];
    u32                 var_hex_value;
#endif /* VTSS_ARCH_CARACAL/SERVAL */
    BOOL                is_needed_mask_range = FALSE;

#if defined(VTSS_ARCH_JAGUAR_1)
    is_needed_mask_range = TRUE;
#endif /* VTSS_ARCH_CARACAL/SERVAL */

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_EVC)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {
        memset(&newconf, 0, sizeof(newconf));

        // ece_id, next_ece_id
        if (cyg_httpd_form_varable_int(p, "ece_id", &var_value)) {
            newconf.conf.id = (vtss_ece_id_t) var_value;
        }
        if (cyg_httpd_form_varable_int(p, "next_ece_id", &var_value)) {
            next_ece_id = (vtss_ece_id_t) var_value;
        }

#if defined(VTSS_ARCH_CARACAL)
        // dmac_type
        if (cyg_httpd_form_varable_int(p, "dmac_type", &var_value)) {
            if (var_value == 0) {
                newconf.conf.key.mac.dmac_mc = VTSS_VCAP_BIT_ANY;
                newconf.conf.key.mac.dmac_bc = VTSS_VCAP_BIT_ANY;
            } else if (var_value == 1) {
                newconf.conf.key.mac.dmac_mc = VTSS_VCAP_BIT_0;
                newconf.conf.key.mac.dmac_bc = VTSS_VCAP_BIT_0;
            } else if (var_value == 2) {
                newconf.conf.key.mac.dmac_mc = VTSS_VCAP_BIT_1;
                newconf.conf.key.mac.dmac_bc = VTSS_VCAP_BIT_0;
            } else if (var_value == 3) {
                newconf.conf.key.mac.dmac_mc = VTSS_VCAP_BIT_1;
                newconf.conf.key.mac.dmac_bc = VTSS_VCAP_BIT_1;
            }
        }
#endif /* VTSS_ARCH_CARACAL */

#if defined(VTSS_ARCH_SERVAL)
        // lookup
        if (cyg_httpd_form_varable_int(p, "lookup", &var_value)) {
            newconf.conf.key.lookup = var_value ? 1 : 0;
        }

        // dmac_filter
        if (cyg_httpd_form_varable_int(p, "dmac_filter", &var_value)) {
            if (var_value == 0) {
                newconf.conf.key.mac.dmac_mc = VTSS_VCAP_BIT_ANY;
                newconf.conf.key.mac.dmac_bc = VTSS_VCAP_BIT_ANY;
                memset(&newconf.conf.key.mac.smac, 0, sizeof(newconf.conf.key.mac.smac));
            } else if (var_value == 1) {
                newconf.conf.key.mac.dmac_mc = VTSS_VCAP_BIT_0;
                newconf.conf.key.mac.dmac_bc = VTSS_VCAP_BIT_0;
                memset(&newconf.conf.key.mac.smac, 0, sizeof(newconf.conf.key.mac.smac));
            } else if (var_value == 2) {
                newconf.conf.key.mac.dmac_mc = VTSS_VCAP_BIT_1;
                newconf.conf.key.mac.dmac_bc = VTSS_VCAP_BIT_0;
                memset(&newconf.conf.key.mac.smac, 0, sizeof(newconf.conf.key.mac.smac));
            } else if (var_value == 3) {
                newconf.conf.key.mac.dmac_mc = VTSS_VCAP_BIT_1;
                newconf.conf.key.mac.dmac_bc = VTSS_VCAP_BIT_1;
                memset(&newconf.conf.key.mac.smac, 0, sizeof(newconf.conf.key.mac.smac));
            } else if (var_value == 4) {
                newconf.conf.key.mac.dmac_mc = VTSS_VCAP_BIT_ANY;
                newconf.conf.key.mac.dmac_bc = VTSS_VCAP_BIT_ANY;
                (void) cyg_httpd_form_varable_mac(p, "dmac", newconf.conf.key.mac.dmac.value);
                memset(newconf.conf.key.mac.dmac.mask, 0xFF, sizeof(newconf.conf.key.mac.dmac.mask));
            }
        }
#endif /* VTSS_ARCH_SERVAL */

#if defined(VTSS_ARCH_CARACAL) || defined(VTSS_ARCH_SERVAL)
        // smac_filter, smac
        if (cyg_httpd_form_varable_int(p, "smac_filter", &var_value)) {
            if (var_value) {
                (void) cyg_httpd_form_varable_mac(p, "smac", newconf.conf.key.mac.smac.value);
                memset(newconf.conf.key.mac.smac.mask, 0xFF, sizeof(newconf.conf.key.mac.smac.mask));
            } else {
                memset(&newconf.conf.key.mac.smac, 0, sizeof(newconf.conf.key.mac.smac));
            }
        }
#endif /* VTSS_ARCH_CARACAL/SERVAL */

        // tag_type
        if (cyg_httpd_form_varable_int(p, "tag_type", &var_value)) {
            if (var_value == 1) { //untagged
                newconf.conf.key.tag.tagged = VTSS_VCAP_BIT_0;
                newconf.conf.key.tag.s_tagged = VTSS_VCAP_BIT_ANY;
            } else if (var_value == 2) { //c-tagged
                newconf.conf.key.tag.tagged = VTSS_VCAP_BIT_1;
                newconf.conf.key.tag.s_tagged = VTSS_VCAP_BIT_0;
            } else if (var_value == 3) { //s_tagged
                newconf.conf.key.tag.tagged = VTSS_VCAP_BIT_1;
                newconf.conf.key.tag.s_tagged = VTSS_VCAP_BIT_1;
            } else if (var_value == 4) { //tagged
                newconf.conf.key.tag.tagged = VTSS_VCAP_BIT_1;
                newconf.conf.key.tag.s_tagged = VTSS_VCAP_BIT_ANY;
            } else { //any
                newconf.conf.key.tag.tagged = VTSS_VCAP_BIT_ANY;
                newconf.conf.key.tag.s_tagged = VTSS_VCAP_BIT_ANY;
            }

            if (var_value >= 2 && var_value <= 4) { //tagged, s_tagged, c-tagged
                // vid_filter
                if (handler_parse_filter_range_mapping(p, "vid", &newconf.conf.key.tag.vid, 0, VLAN_ID_MAX, is_needed_mask_range) != VTSS_OK) {
                    err_rc++;
                }

                // pcp
                handler_parse_pcp_range_mapping(p, "pcp", &newconf.conf.key.tag.pcp);

                // dei
                if (cyg_httpd_form_varable_int(p, "dei", &var_value)) {
                    newconf.conf.key.tag.dei = (vtss_vcap_bit_t) var_value;
                }
            } else {
                memset(&newconf.conf.key.tag.vid, 0, sizeof(newconf.conf.key.tag.vid));
                memset(&newconf.conf.key.tag.pcp, 0, sizeof(newconf.conf.key.tag.pcp));
                newconf.conf.key.tag.dei = VTSS_VCAP_BIT_ANY;
            }
        }

#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
        // in_type
        if (cyg_httpd_form_varable_int(p, "in_type", &var_value)) {
            if (var_value == 1) { //untagged
                newconf.conf.key.inner_tag.tagged = VTSS_VCAP_BIT_0;
                newconf.conf.key.inner_tag.s_tagged = VTSS_VCAP_BIT_ANY;
            } else if (var_value == 2) { //c-tagged
                newconf.conf.key.inner_tag.tagged = VTSS_VCAP_BIT_1;
                newconf.conf.key.inner_tag.s_tagged = VTSS_VCAP_BIT_0;
            } else if (var_value == 3) { //s_tagged
                newconf.conf.key.inner_tag.tagged = VTSS_VCAP_BIT_1;
                newconf.conf.key.inner_tag.s_tagged = VTSS_VCAP_BIT_1;
            } else if (var_value == 4) { //tagged
                newconf.conf.key.inner_tag.tagged = VTSS_VCAP_BIT_1;
                newconf.conf.key.inner_tag.s_tagged = VTSS_VCAP_BIT_ANY;
            } else { //any
                newconf.conf.key.inner_tag.tagged = VTSS_VCAP_BIT_ANY;
                newconf.conf.key.inner_tag.s_tagged = VTSS_VCAP_BIT_ANY;
            }

            if (var_value >= 2 && var_value <= 4) { //tagged, s_tagged, c-tagged
                // in_vid_filter
                if (handler_parse_filter_range_mapping(p, "in_vid", &newconf.conf.key.inner_tag.vid, 0, VLAN_ID_MAX, is_needed_mask_range) != VTSS_OK) {
                    err_rc++;
                }

                // in_pcp
                handler_parse_pcp_range_mapping(p, "in_pcp", &newconf.conf.key.inner_tag.pcp);

                // in_dei
                if (cyg_httpd_form_varable_int(p, "in_dei", &var_value)) {
                    newconf.conf.key.inner_tag.dei = (vtss_vcap_bit_t) var_value;
                }
            } else {
                memset(&newconf.conf.key.inner_tag.vid, 0, sizeof(newconf.conf.key.inner_tag.vid));
                memset(&newconf.conf.key.inner_tag.pcp, 0, sizeof(newconf.conf.key.inner_tag.pcp));
                newconf.conf.key.inner_tag.dei = VTSS_VCAP_BIT_ANY;
            }
        }
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */

        // frame_type
        if (cyg_httpd_form_varable_int(p, "frame_type", &var_value)) {
#if defined(VTSS_ARCH_SERVAL)
            newconf.conf.key.type = var_value == 6 ? VTSS_ECE_TYPE_ANY : (vtss_ece_type_t) var_value;
            if (var_value == 6 /* L2CP */) {
                if (cyg_httpd_form_varable_int(p, "l2cp_proto", &var_value)) {
                    newconf.data.l2cp.proto = (evc_l2cp_t) var_value;
                }
            } else {
                newconf.data.l2cp.proto = EVC_L2CP_NONE;
            }
#else
            newconf.conf.key.type = (vtss_ece_type_t) var_value;
#endif /* VTSS_ARCH_SERVAL */

#if defined(VTSS_ARCH_JAGUAR_1)
            if (cyg_httpd_form_varable_int(p, "dscp_filter", &range_filter)) {
                if (range_filter) {
                    if (newconf.conf.key.type == VTSS_ECE_TYPE_IPV4) {
                        if (handler_parse_filter_range_mapping(p, "dscp", &newconf.conf.key.frame.ipv4.dscp, 0, 63, is_needed_mask_range) != VTSS_OK) {
                            err_rc++;
                        }
                    } else if (newconf.conf.key.type == VTSS_ECE_TYPE_IPV6) {
                        if (handler_parse_filter_range_mapping(p, "dscp", &newconf.conf.key.frame.ipv6.dscp, 0, 63, is_needed_mask_range) != VTSS_OK) {
                            err_rc++;
                        }
                    } else {
                        if (handler_parse_filter_range_mapping(p, "dscp", &newconf.conf.key.frame.ipv6.dscp, 0, 63, is_needed_mask_range) != VTSS_OK) {
                            err_rc++;
                        }
                        if (handler_parse_filter_range_mapping(p, "dscp", &newconf.conf.key.frame.ipv6.dscp, 0 , 63, is_needed_mask_range) != VTSS_OK) {
                            err_rc++;
                        }
                    }
                } else {
                    memset(&newconf.conf.key.frame.ipv4.dscp, 0, sizeof(newconf.conf.key.frame.ipv4.dscp));
                    memset(&newconf.conf.key.frame.ipv6.dscp, 0, sizeof(newconf.conf.key.frame.ipv6.dscp));
                }
            }
#endif /* VTSS_ARCH_JAGUAR_1 */

#if defined(VTSS_ARCH_CARACAL) || defined(VTSS_ARCH_SERVAL)
            if (newconf.conf.key.type == VTSS_ECE_TYPE_ANY) {
                memset(&newconf.conf.key.frame, 0, sizeof(newconf.conf.key.frame));

#if defined(VTSS_ARCH_SERVAL)
            } else if (newconf.conf.key.type == VTSS_ECE_TYPE_ETYPE) {
                if (cyg_httpd_form_varable_int(p, "etype_filter", &common_filter)) {
                    if (common_filter == 0) {
                        memset(&newconf.conf.key.frame.etype.etype, 0, sizeof(newconf.conf.key.frame.etype.etype));
                    } else if (common_filter && cyg_httpd_form_varable_hex(p, "etype_value", &var_hex_value)) {
                        newconf.conf.key.frame.etype.etype.value[0] = (u8) ((var_hex_value & 0xFF00) >> 8);
                        newconf.conf.key.frame.etype.etype.value[1] = (u8) (var_hex_value & 0xFF);
                        newconf.conf.key.frame.etype.etype.mask[0] = newconf.conf.key.frame.etype.etype.mask[1] = 0xFF;
                    }
                }
                if (cyg_httpd_form_varable_int(p, "etype_data_filter", &common_filter)) {
                    if (common_filter == 0) {
                        memset(&newconf.conf.key.frame.etype.data, 0, sizeof(newconf.conf.key.frame.etype.data));
                    } else if (common_filter && cyg_httpd_form_varable_hex(p, "etype_data_value", &var_hex_value)) {
                        newconf.conf.key.frame.etype.data.value[0] = (u8) ((var_hex_value & 0xFF00) >> 8);
                        newconf.conf.key.frame.etype.data.value[1] = (u8) (var_hex_value & 0xFF);
                        if (cyg_httpd_form_varable_hex(p, "etype_data_mask", &var_hex_value)) {
                            newconf.conf.key.frame.etype.data.mask[0] = (u8) ((var_hex_value & 0xFF00) >> 8);
                            newconf.conf.key.frame.etype.data.mask[1] = (u8) (var_hex_value & 0xFF);
                        }
                    }
                }
            } else if (newconf.conf.key.type == VTSS_ECE_TYPE_LLC) {
                if (cyg_httpd_form_varable_int(p, "llc_dsap_filter", &common_filter)) {
                    if (common_filter == 0) {
                        newconf.conf.key.frame.llc.data.value[0] = newconf.conf.key.frame.llc.data.mask[0] = 0;
                    } else if (common_filter && cyg_httpd_form_varable_hex(p, "llc_dsap_value", &var_hex_value)) {
                        newconf.conf.key.frame.llc.data.value[0] = (u8) (var_hex_value & 0xFF);
                        newconf.conf.key.frame.llc.data.mask[0] = 0xFF;
                    }
                }
                if (cyg_httpd_form_varable_int(p, "llc_ssap_filter", &common_filter)) {
                    if (common_filter == 0) {
                        newconf.conf.key.frame.llc.data.value[1] = newconf.conf.key.frame.llc.data.mask[1] = 0;
                    } else if (common_filter && cyg_httpd_form_varable_hex(p, "llc_ssap_value", &var_hex_value)) {
                        newconf.conf.key.frame.llc.data.value[1] = (u8) (var_hex_value & 0xFF);
                        newconf.conf.key.frame.llc.data.mask[1] = 0xFF;
                    }
                }
                if (cyg_httpd_form_varable_int(p, "llc_ctrl_filter", &common_filter)) {
                    if (common_filter == 0) {
                        newconf.conf.key.frame.llc.data.value[2] = newconf.conf.key.frame.llc.data.mask[2] = 0;
                    } else if (common_filter && cyg_httpd_form_varable_hex(p, "llc_ctrl_value", &var_hex_value)) {
                        newconf.conf.key.frame.llc.data.value[2] = (u8) (var_hex_value & 0xFF);
                        newconf.conf.key.frame.llc.data.mask[2] = 0xFF;
                    }
                }
                if (cyg_httpd_form_varable_int(p, "llc_data_filter", &common_filter)) {
                    if (common_filter == 0) {
                        newconf.conf.key.frame.llc.data.value[3] = newconf.conf.key.frame.llc.data.mask[4] = 0;
                        newconf.conf.key.frame.llc.data.value[3] = newconf.conf.key.frame.llc.data.mask[4] = 0;
                    } else if (common_filter && cyg_httpd_form_varable_hex(p, "llc_data_value", &var_hex_value)) {
                        newconf.conf.key.frame.llc.data.value[3] = (u8) ((var_hex_value & 0xFF00) >> 8);
                        newconf.conf.key.frame.llc.data.value[4] = (u8) (var_hex_value & 0xFF);
                        if (cyg_httpd_form_varable_hex(p, "llc_data_mask", &var_hex_value)) {
                            newconf.conf.key.frame.llc.data.mask[3] = (u8) ((var_hex_value & 0xFF00) >> 8);
                            newconf.conf.key.frame.llc.data.mask[4] = (u8) (var_hex_value & 0xFF);
                        }
                    }
                }
            } else if (newconf.conf.key.type == VTSS_ECE_TYPE_SNAP) {
                if (cyg_httpd_form_varable_int(p, "snap_oui_filter", &common_filter)) {
                    if (common_filter == 0) {
                        newconf.conf.key.frame.snap.data.mask[0] = \
                        newconf.conf.key.frame.snap.data.mask[1] = \
                        newconf.conf.key.frame.snap.data.mask[2] = 0;
                    } else if (cyg_httpd_form_varable_oui(p, "snap_oui_value", newconf.conf.key.frame.snap.data.value)) {
                        newconf.conf.key.frame.snap.data.mask[0] = \
                        newconf.conf.key.frame.snap.data.mask[1] = \
                        newconf.conf.key.frame.snap.data.mask[2] = 0xFF;
                    }
                }
                if (cyg_httpd_form_varable_int(p, "snap_pid_filter", &common_filter)) {
                    if (common_filter == 0) {
                        newconf.conf.key.frame.snap.data.value[3] = newconf.conf.key.frame.snap.data.mask[3] = 0;
                        newconf.conf.key.frame.snap.data.value[4] = newconf.conf.key.frame.snap.data.mask[4] = 0;
                    } else if (common_filter && cyg_httpd_form_varable_hex(p, "snap_pid_value", &var_hex_value)) {
                        newconf.conf.key.frame.snap.data.value[3] = (u8) ((var_hex_value & 0xFF00) >> 8);
                        newconf.conf.key.frame.snap.data.value[4] = (u8) (var_hex_value & 0xFF);
                        newconf.conf.key.frame.snap.data.mask[3] = newconf.conf.key.frame.snap.data.mask[4] = 0xFF;
                    }
                }
#endif /* VTSS_ARCH_SERVAL */

            } else if (newconf.conf.key.type == VTSS_ECE_TYPE_IPV4) {
                // dscp_filter
                if (handler_parse_filter_range_mapping(p, "dscp", &newconf.conf.key.frame.ipv4.dscp, 0, 63, is_needed_mask_range) != VTSS_OK) {
                    err_rc++;
                }

                // proto_filter ("Any", "UDP", "TCP", "Other");
                if (cyg_httpd_form_varable_int(p, "proto_filter", &common_filter)) {
                    if (common_filter) {
                        newconf.conf.key.frame.ipv4.proto.mask = 0xFF;
                        if (common_filter == 1) {
                            newconf.conf.key.frame.ipv4.proto.value = 17;
                        } else if (common_filter == 2) {
                            newconf.conf.key.frame.ipv4.proto.value = 6;
                        } else if (cyg_httpd_form_varable_int(p, "proto", &var_value)) {
                            newconf.conf.key.frame.ipv4.proto.value = var_value;
                        }
                    } else {
                        newconf.conf.key.frame.ipv4.proto.value = 0;
                        newconf.conf.key.frame.ipv4.proto.mask = 0;
                    }
                }

                // sip_filter ("Any", "Host", "Network")
                if (cyg_httpd_form_varable_int(p, "sip_filter", &var_value)) {
                    if (var_value) {
                        (void) cyg_httpd_form_varable_ipv4(p, "sip", &newconf.conf.key.frame.ipv4.sip.value);
                        if (var_value == 1) {
                            newconf.conf.key.frame.ipv4.sip.mask = 0xFFFFFFFF;
                        } else {
                            (void) cyg_httpd_form_varable_ipv4(p, "sip_mask", &newconf.conf.key.frame.ipv4.sip.mask);
                        }
                    } else {
                        newconf.conf.key.frame.ipv4.sip.value = 0;
                        newconf.conf.key.frame.ipv4.sip.mask = 0;
                    }
                }

#if defined(VTSS_ARCH_SERVAL)
                // dip_filter ("Any", "Host", "Network")
                if (cyg_httpd_form_varable_int(p, "dip_filter", &var_value)) {
                    if (var_value) {
                        (void) cyg_httpd_form_varable_ipv4(p, "dip", &newconf.conf.key.frame.ipv4.dip.value);
                        if (var_value == 1) {
                            newconf.conf.key.frame.ipv4.dip.mask = 0xFFFFFFFF;
                        } else {
                            (void) cyg_httpd_form_varable_ipv4(p, "dip_mask", &newconf.conf.key.frame.ipv4.dip.mask);
                        }
                    } else {
                        newconf.conf.key.frame.ipv4.dip.value = 0;
                        newconf.conf.key.frame.ipv4.dip.mask = 0;
                    }
                }
#endif /* VTSS_ARCH_SERVAL */

                // fragment
                if (cyg_httpd_form_varable_int(p, "fragment", &var_value)) {
                    newconf.conf.key.frame.ipv4.fragment = var_value;
                }

                // sport_filter
                if (handler_parse_filter_range_mapping(p, "sport", &newconf.conf.key.frame.ipv4.sport, 0, 65535, is_needed_mask_range) != VTSS_OK) {
                    err_rc++;
                }

                // dport_filter
                if (handler_parse_filter_range_mapping(p, "dport", &newconf.conf.key.frame.ipv4.dport, 0, 65535, is_needed_mask_range) != VTSS_OK) {
                    err_rc++;
                }
            } else if (newconf.conf.key.type == VTSS_ECE_TYPE_IPV6) {
                // proto_v6_filter
                if (cyg_httpd_form_varable_int(p, "proto_v6_filter", &common_filter)) {
                    if (common_filter) {
                        newconf.conf.key.frame.ipv6.proto.mask = 0xFF;
                        if (common_filter == 1) {
                            newconf.conf.key.frame.ipv6.proto.value = 17;
                        } else if (common_filter == 2) {
                            newconf.conf.key.frame.ipv6.proto.value = 6;
                        } else if (cyg_httpd_form_varable_int(p, "proto_v6", &var_value)) {
                            newconf.conf.key.frame.ipv6.proto.value = var_value;
                        }
                    } else {
                        newconf.conf.key.frame.ipv6.proto.value = 0;
                        newconf.conf.key.frame.ipv6.proto.mask = 0;
                    }
                }

                // sip_v6_filter ("Any", "Specific")
                if (cyg_httpd_form_varable_int(p, "sip_v6_filter", &common_filter)) {
                    if (common_filter) {
                        (void) cyg_httpd_form_varable_ipv4(p, "sip_v6", &var_hex_value);
                        var_hex_value = ntohl(var_hex_value);
                        memcpy(&newconf.conf.key.frame.ipv6.sip.value[12], &var_hex_value, 4);
                        var_hex_value = ntohl(var_hex_value);
                        (void) cyg_httpd_form_varable_ipv4(p, "sip_v6_mask", &var_hex_value);
                        memcpy(&newconf.conf.key.frame.ipv6.sip.mask[12], &var_hex_value, 4);
                    } else {
                        memset(&newconf.conf.key.frame.ipv6.sip, 0, sizeof(newconf.conf.key.frame.ipv6.sip));
                    }
                }

#if defined(VTSS_ARCH_SERVAL)
                // dip_v6_filter ("Any", "Specific")
                if (cyg_httpd_form_varable_int(p, "dip_v6_filter", &common_filter)) {
                    if (common_filter) {
                        (void) cyg_httpd_form_varable_ipv4(p, "dip_v6", &var_hex_value);
                        var_hex_value = ntohl(var_hex_value);
                        memcpy(&newconf.conf.key.frame.ipv6.dip.value[12], &var_hex_value, 4);
                        (void) cyg_httpd_form_varable_ipv4(p, "dip_v6_mask", &var_hex_value);
                        var_hex_value = ntohl(var_hex_value);
                        memcpy(&newconf.conf.key.frame.ipv6.dip.mask[12], &var_hex_value, 4);
                    } else {
                        memset(&newconf.conf.key.frame.ipv6.dip, 0, sizeof(newconf.conf.key.frame.ipv6.dip));
                    }
                }
#endif /* VTSS_ARCH_SERVAL */

                // dscp_v6_filter
                if (handler_parse_filter_range_mapping(p, "dscp_v6", &newconf.conf.key.frame.ipv6.dscp, 0, 63, is_needed_mask_range) != VTSS_OK) {
                    err_rc++;
                }

                // sport_v6_filter
                if (handler_parse_filter_range_mapping(p, "sport_v6", &newconf.conf.key.frame.ipv6.sport, 0, 65535, is_needed_mask_range) != VTSS_OK) {
                    err_rc++;
                }

                // dport_v6_filter
                if (handler_parse_filter_range_mapping(p, "dport_v6", &newconf.conf.key.frame.ipv6.dport, 0, 65535, is_needed_mask_range) != VTSS_OK) {
                    err_rc++;
                }
            }
#endif /* VTSS_ARCH_CARACAL/SERVAL */
        }

        // direction
        if (cyg_httpd_form_varable_int(p, "direction", &var_value)) {
            newconf.conf.action.dir = var_value;
        }

#if defined(VTSS_ARCH_SERVAL)
        // rule_type
        if (cyg_httpd_form_varable_int(p, "rule_type", &var_value)) {
            newconf.conf.action.rule = var_value;
        }

        // tx_lookup
        if (cyg_httpd_form_varable_int(p, "tx_lookup", &var_value)) {
            newconf.conf.action.tx_lookup = var_value;
        }

        // l2cp_mode
        if (cyg_httpd_form_varable_int(p, "l2cp_mode", &var_value)) {
            newconf.data.l2cp.mode = (evc_l2cp_mode_t) var_value;
        }

        // l2cp_dmac
        if (cyg_httpd_form_varable_int(p, "l2cp_dmac", &var_value)) {
            newconf.data.l2cp.dmac = (evc_l2cp_dmac_t) var_value;
        }
#endif /* VTSS_ARCH_SERVAL */

        // evc_id_filter ("None", "Specific")
        if (cyg_httpd_form_varable_int(p, "evc_id_filter", &common_filter)) {
            if (common_filter) {
                // evc_id
                if (cyg_httpd_form_varable_int(p, "evc_id", &var_value)) {
                    newconf.conf.action.evc_id = evc_id_u2i((vtss_evc_id_t) var_value);
                }
            } else {
                newconf.conf.action.evc_id = VTSS_EVC_ID_NONE;
            }
        }

#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
        // policer_id_filter ("Specific", "Discard", "None", "EVC")
        if (cyg_httpd_form_varable_int(p, "policer_id_filter", &policer_id_filter)) {
            if (policer_id_filter == 1) {
                newconf.conf.action.policer_id = VTSS_EVC_POLICER_ID_DISCARD;
            } else if (policer_id_filter == 2) {
                newconf.conf.action.policer_id = VTSS_EVC_POLICER_ID_NONE;
            } else if (policer_id_filter == 3) {
                newconf.conf.action.policer_id = VTSS_EVC_POLICER_ID_EVC;
            } else {
                // policer_id
                if (cyg_httpd_form_varable_int(p, "policer_id", &var_value)) {
                    newconf.conf.action.policer_id = policer_id_u2i((vtss_evc_policer_id_t) var_value);
                }
            }
        }
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */

        // pop
        if (cyg_httpd_form_varable_int(p, "pop", &var_value)) {
            newconf.conf.action.pop_tag = var_value;
        }

        // policy_no
        if (cyg_httpd_form_varable_int(p, "policy_no", &var_value)) {
            newconf.conf.action.policy_no = (vtss_acl_policy_no_t) var_value;
        }

#if defined(VTSS_ARCH_CARACAL) || defined(VTSS_ARCH_SERVAL)
        // class
        if (cyg_httpd_form_varable_int(p, "class", &var_value)) {
            if (var_value == VTSS_PRIOS) {
                newconf.conf.action.prio_enable = FALSE;
                newconf.conf.action.prio = 0;
            } else {
                newconf.conf.action.prio_enable = TRUE;
                newconf.conf.action.prio = var_value;
            }
        }
#endif /* VTSS_ARCH_CARACAL/SERVAL */

#if defined(VTSS_ARCH_SERVAL)
        // dp
        if (cyg_httpd_form_varable_int(p, "dp", &var_value)) {
            if (var_value == 2) { //Disabled
                newconf.conf.action.dp_enable = FALSE;
                newconf.conf.action.dp = 0;
            } else {
                newconf.conf.action.dp_enable = TRUE;
                newconf.conf.action.dp = var_value;
            }
        }
#endif /* VTSS_ARCH_SERVAL */

        // ot_mode
        if (cyg_httpd_form_varable_int(p, "ot_mode", &var_value)) {
            newconf.conf.action.outer_tag.enable = (vtss_vid_t) var_value;
        }

#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
        // ot_vid
        if (cyg_httpd_form_varable_int(p, "ot_vid", &var_value)) {
            newconf.conf.action.outer_tag.vid = (vtss_vid_t) var_value;
        }
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */

        // ot_preserve
        if (cyg_httpd_form_varable_int(p, "ot_preserve", &var_value)) {
#if defined(VTSS_ARCH_SERVAL)
            newconf.conf.action.outer_tag.pcp_mode = var_value;
#else
            newconf.conf.action.outer_tag.pcp_dei_preserve = (BOOL) var_value;
#endif /* VTSS_ARCH_SERVAL */
        }

        // ot_pcp
        if (cyg_httpd_form_varable_int(p, "ot_pcp", &var_value)) {
            newconf.conf.action.outer_tag.pcp = (vtss_tagprio_t) var_value;
        }

#if defined(VTSS_ARCH_SERVAL)
        // ot_dei_mode
        if (cyg_httpd_form_varable_int(p, "ot_dei_mode", &var_value)) {
            newconf.conf.action.outer_tag.dei_mode = (vtss_dei_t) var_value;
        }
#endif /* VTSS_ARCH_SERVAL */

        // ot_dei
        if (cyg_httpd_form_varable_int(p, "ot_dei", &var_value)) {
            newconf.conf.action.outer_tag.dei = (vtss_dei_t) var_value;
        }

#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
        // it_type ("None", "C-tag", "S-tag", "S-custom-tag")
        if (cyg_httpd_form_varable_int(p, "it_type", &var_value)) {
            if (var_value == 3) {
                newconf.conf.action.inner_tag.type = VTSS_ECE_INNER_TAG_S_CUSTOM;
            } else if (var_value == 2) {
                newconf.conf.action.inner_tag.type = VTSS_ECE_INNER_TAG_S;
            } else if (var_value == 1) {
                newconf.conf.action.inner_tag.type = VTSS_ECE_INNER_TAG_C;
            } else {
                newconf.conf.action.inner_tag.type = VTSS_ECE_INNER_TAG_NONE;
            }
        }

        // it_vid
        if (cyg_httpd_form_varable_int(p, "it_vid", &var_value)) {
            newconf.conf.action.inner_tag.vid = (vtss_vid_t) var_value;
        }

        // it_preserve
        if (cyg_httpd_form_varable_int(p, "it_preserve", &var_value)) {
#if defined(VTSS_ARCH_SERVAL)
            newconf.conf.action.inner_tag.pcp_mode = var_value;
#else
            newconf.conf.action.inner_tag.pcp_dei_preserve = (BOOL) var_value;
#endif /* VTSS_ARCH_SERVAL */
        }

        // it_pcp
        if (cyg_httpd_form_varable_int(p, "it_pcp", &var_value)) {
            newconf.conf.action.inner_tag.pcp = (vtss_tagprio_t) var_value;
        }
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */

#if defined(VTSS_ARCH_SERVAL)
        // it_dei_mode
        if (cyg_httpd_form_varable_int(p, "it_dei_mode", &var_value)) {
            newconf.conf.action.inner_tag.dei_mode = (vtss_dei_t) var_value;
        }
#endif /* VTSS_ARCH_SERVAL */

#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
        // it_dei
        if (cyg_httpd_form_varable_int(p, "it_dei", &var_value)) {
            newconf.conf.action.inner_tag.dei = (vtss_dei_t) var_value;
        }
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */

        // uni_ports
        port_iter_init_local(&pit);
        while (port_iter_getnext(&pit)) {
            iport = pit.iport;
            uport = iport2uport(iport);
            sprintf(var_name, "uni_port_%u", uport);
            if (cyg_httpd_form_varable_find(p, var_name) != NULL) {
                newconf.conf.key.port_list[iport] = VTSS_ECE_PORT_ROOT;
            } else {
                newconf.conf.key.port_list[iport] = VTSS_ECE_PORT_NONE;
            }
        }

        // Save new configuration
        if (err_rc || evc_mgmt_ece_add(next_ece_id, &newconf) != VTSS_OK) {
            T_W("evc_mgmt_ece_add(%u, %u): failed", next_ece_id, newconf.conf.id);
        }

        redirect(p, "/evc_ece.htm");
    } else {    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        // Format        : [eceEditFlag]/[selectEceId]
        // <Edit>        :             1/[ece_id]
        // <Insert>      :             2/[next_id]
        // <Add to Last> :             3/0

        if (cyg_httpd_form_varable_int(p, "eceEditFlag", &ece_flag)) {
            switch (ece_flag) {
            case 1:
                if (cyg_httpd_form_varable_int(p, "selectEceId", &var_value)) {
                    ece_id = (vtss_ece_id_t) var_value;
                }
                break;
            case 2:
                if (cyg_httpd_form_varable_int(p, "selectEceId", &var_value)) {
                    next_ece_id = (vtss_ece_id_t) var_value;
                }
                break;
            case 3:
            default:
                break;
            }
        }

        // Format: [ece_id]/[next_ece_id]
        //          /[lookup:SRVL]
        //          /[dmac_type:Lu26]/[dmac_filter:SRVL]/[dmac:SRVL]/[smac_filter:Lu26/SRVL]/[smac:Lu26/SRVL]
        //          /[tag_type]/[vid_filter]/[vid_low]/[vid_high]/[pcp]/[dei]
        //          /[in_type:JR1/SRVL]/[in_vid_filter:JR1/SRVL]/[in_vid_low:JR1/SRVL]/[in_vid_high:JR1/SRVL]/[in_pcp:JR1/SRVL]/[in_dei:JR1/SRVL]
        //          /[frame_type]
        //          [frame_type] = Etype /[etype_value_filter:SRVL]/[etype_value:SRVL]/[etype_data_filter:SRVL]/[etype_data_value:SRVL]/[etype_data_mask:SRVL]
        //          [frame_type] = LLC /[llc_dsap_filter:SRVL]/[llc_dsap_value:SRVL]/[llc_ssap_filter:SRVL]/[llc_ssap_value:SRVL]/[llc_ctrl_filter:SRVL]/[llc_ctrl_value:SRVL]/[llc_data_filter:SRVL]/[llc_data_value:SRVL]/[llc_data_mask:SRVL]
        //          [frame_type] = SNAP /[snap_oui_filter:SRVL]/[snap_oui_value:SRVL]/[snap_pid_filter:SRVL]/[snap_pid_value:SRVL]
        //          [frame_type] = L2CP /[l2cp_type:SRVL]
        //          /[proto_filter:Lu26/SRVL]/[proto:Lu26/SRVL]/[sip_filter:Lu26/SRVL]/[sip:Lu26/SRVL]/[sip_mask:Lu26/SRVL]/[dip_filter:SRVL]/[dip:SRVL]/[dip_mask:SRVL]
        //          /[dscp_filter]/[dscp_low]/[dscp_high]
        //          /[fragment:Lu26/SRVL]/[sport_filter:Lu26/SRVL]/[sport_low:Lu26/SRVL]/[sport_high:Lu26/SRVL]
        //          /[dport_filter:Lu26/SRVL]/[dport_low:Lu26/SRVL]/[dport_high:Lu26/SRVL]
        //          /[proto_v6_filter:Lu26/SRVL]/[proto_v6:Lu26/SRVL]/[sip_v6_filter:Lu26/SRVL]/[sip_v6:Lu26/SRVL]/[sip_v6_mask:Lu26/SRVL]/[dip_v6_filter:SRVL]/[dip_v6:SRVL]/[dip_v6_mask:SRVL]
        //          /[dscp_v6_filter:Lu26/SRVL]/[dscp_v6_low:Lu26/SRVL]/[dscp_v6_high:Lu26/SRVL]
        //          /[sport_v6_filter:Lu26/SRVL]/[sport_v6_low:Lu26/SRVL]/[sport_v6_low:Lu26/SRVL]
        //          /[dport_v6_filter:Lu26/SRVL]/[dport_v6_low:Lu26/SRVL]/[dport_v6_high:Lu26/SRVL]
        //          /[direction]
        //          /[rule_type:SRVL]/[tx_lookup:SRVL]/[l2cp_mode:SRVL]/[l2cp_dmac:SRVL]
        //          /[evc_id_filter]/[evc_id]
        //          /[policer_id_filter:JR1/SRVL]/[policer_id:JR1/SRVL]/[pop]/[policy_no]/[class:Lu26/SRVL]/[dp:SRVL]
        //          /[ot_mode]/[ot_vid:JR1/SRVL]/[ot_preserve]/[ot_pcp]/[ot_dei_mode:SRVL]/[ot_dei]
        //          /[it_type:JR1/SRVL]/[it_vid:JR1/SRVL]/[it_preserve:JR1/SRVL]/[it_pcp:JR1/SRVL]/[it_dei_mode:SRVL]/[it_dei:JR1/SRVL]
        //          /[uni_port_0]/[uni_port_1]/...
        // Note: uni_port = 2 means this port is NNI port

        (void) cyg_httpd_start_chunked("html");

        // Get NNI ports
        memset(info, 0, sizeof(info));
        (void) evc_mgmt_port_info_get(info);

        if (ece_flag == 1 && evc_mgmt_ece_get(ece_id, &conf, FALSE) == VTSS_OK) {
            // next_ece_id
            rc = evc_mgmt_ece_get(conf.conf.id, &newconf, TRUE);

            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/%u",
                          /* ece_id      */   conf.conf.id,
                          /* next_ece_id */   rc == VTSS_OK ? newconf.conf.id : VTSS_ECE_ID_LAST);
            (void) cyg_httpd_write_chunked(p->outbuffer, ct);

#if defined(VTSS_ARCH_SERVAL)
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u",
                          /* lookup */  conf.conf.key.lookup);
            (void) cyg_httpd_write_chunked(p->outbuffer, ct);
#endif /* VTSS_ARCH_SERVAL */

#if defined(VTSS_ARCH_CARACAL) || defined(VTSS_ARCH_SERVAL)
            dmac_type = conf.conf.key.mac.dmac_bc == VTSS_VCAP_BIT_1 ? 3 :
                        conf.conf.key.mac.dmac_mc == VTSS_VCAP_BIT_1 ? 2 :
                        conf.conf.key.mac.dmac_mc == VTSS_VCAP_BIT_0 ? 1 : 0;
#endif /* VTSS_ARCH_CARACAL/SERVAL */

#if defined(VTSS_ARCH_CARACAL)                            
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u",
                          /* dmac_type */ dmac_type);
            (void) cyg_httpd_write_chunked(p->outbuffer, ct);
#endif /* VTSS_ARCH_CARACAL */

#if defined(VTSS_ARCH_SERVAL)
            common_filter = (conf.conf.key.mac.dmac.mask[0] |
                             conf.conf.key.mac.dmac.mask[1] |
                             conf.conf.key.mac.dmac.mask[2] |
                             conf.conf.key.mac.dmac.mask[3] |
                             conf.conf.key.mac.dmac.mask[4] |
                             conf.conf.key.mac.dmac.mask[5]) ? 4 : dmac_type;
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u/%s",
                          /* dmac_filter */   common_filter,
                          /* dmac        */   common_filter == 4 ? misc_mac_txt(conf.conf.key.mac.dmac.value, str_buff) : "00-00-00-00-00-01");
            (void) cyg_httpd_write_chunked(p->outbuffer, ct);
#endif /* VTSS_ARCH_SERVAL */

#if defined(VTSS_ARCH_CARACAL) || defined(VTSS_ARCH_SERVAL)
            common_filter = (conf.conf.key.mac.smac.mask[0] |
                             conf.conf.key.mac.smac.mask[1] |
                             conf.conf.key.mac.smac.mask[2] |
                             conf.conf.key.mac.smac.mask[3] |
                             conf.conf.key.mac.smac.mask[4] |
                             conf.conf.key.mac.smac.mask[5]) ? 1 : 0;
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u/%s",
                          /* smac_filter */ common_filter,
                          /* smac        */ common_filter ? misc_mac_txt(conf.conf.key.mac.smac.value, str_buff) : "00-00-00-00-00-01");
            (void) cyg_httpd_write_chunked(p->outbuffer, ct);
#endif /* VTSS_ARCH_CARACAL/SERVAL */

            range_filter = handler_get_filter_range_mapping(&conf.conf.key.tag.vid);
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u/%u/%u/%u/%u/%u",
                          /* tag_type   */   conf.conf.key.tag.tagged == VTSS_VCAP_BIT_ANY ? 0 : conf.conf.key.tag.tagged == VTSS_VCAP_BIT_0 ? 1 : conf.conf.key.tag.s_tagged == VTSS_VCAP_BIT_0 ? 2 : conf.conf.key.tag.s_tagged == VTSS_VCAP_BIT_1 ? 3 : 4,
                          /* vid_filter */   range_filter,
                          /* vid_low    */   range_filter == 0 ? 0 : range_filter == 1 ? conf.conf.key.tag.vid.vr.v.value : conf.conf.key.tag.vid.vr.r.low,
                          /* vid_high   */   range_filter == 2 ? conf.conf.key.tag.vid.vr.r.high : VLAN_ID_MAX,
                          /* pcp        */   handler_get_pcp_range_mapping(&conf.conf.key.tag.pcp),
                          /* dei        */   conf.conf.key.tag.dei);
            (void) cyg_httpd_write_chunked(p->outbuffer, ct);

#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
            range_filter = handler_get_filter_range_mapping(&conf.conf.key.inner_tag.vid);
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u/%u/%u/%u/%u/%u",
                          /* in_type       */   conf.conf.key.inner_tag.tagged == VTSS_VCAP_BIT_ANY ? 0 : conf.conf.key.inner_tag.tagged == VTSS_VCAP_BIT_0 ? 1 : conf.conf.key.inner_tag.s_tagged == VTSS_VCAP_BIT_0 ? 2 : conf.conf.key.inner_tag.s_tagged == VTSS_VCAP_BIT_1 ? 3 : 4,
                          /* in_vid_filter */   range_filter,
                          /* in_vid_low    */   range_filter == 0 ? 0 : range_filter == 1 ? conf.conf.key.inner_tag.vid.vr.v.value : conf.conf.key.inner_tag.vid.vr.r.low,
                          /* in_vid_high   */   range_filter == 2 ? conf.conf.key.inner_tag.vid.vr.r.high : VLAN_ID_MAX,
                          /* in_pcp        */   handler_get_pcp_range_mapping(&conf.conf.key.inner_tag.pcp),
                          /* in_dei        */   conf.conf.key.inner_tag.dei);
            (void) cyg_httpd_write_chunked(p->outbuffer, ct);
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */

#if defined(VTSS_ARCH_SERVAL)
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u",
                          /* frame_type */  conf.data.l2cp.proto != EVC_L2CP_NONE ? 6 : conf.conf.key.type);
            (void) cyg_httpd_write_chunked(p->outbuffer, ct);

            if (conf.conf.key.type == VTSS_ECE_TYPE_ETYPE) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u/%02X%02X/%u/%02X%02X/%02X%02X",
                              /* etype_filter       */  (conf.conf.key.frame.etype.etype.mask[0] || conf.conf.key.frame.etype.etype.mask[1]) ? 1 : 0,
                              /* etype_value        */  conf.conf.key.frame.etype.etype.value[0], conf.conf.key.frame.etype.etype.value[1],
                              /* etype_data_filter  */  (conf.conf.key.frame.etype.data.mask[0] || conf.conf.key.frame.etype.data.mask[1]) ? 1 : 0,
                              /* etype_data_value   */  conf.conf.key.frame.etype.data.value[0], conf.conf.key.frame.etype.data.value[1],
                              /* etype_data_mask    */  conf.conf.key.frame.etype.data.mask[0], conf.conf.key.frame.etype.data.mask[1]);
            } else {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/0/FFFF/0/FFFF/FFFF");
            }
            (void) cyg_httpd_write_chunked(p->outbuffer, ct);

            if (conf.conf.key.type == VTSS_ECE_TYPE_LLC) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u/%02X/%u/%02X/%u/%02X/%u/%02X%02X/%02X%02X",
                              /* llc_dsap_filter */  conf.conf.key.frame.llc.data.mask[0] ? 1 : 0,
                              /* llc_dsap_value  */  conf.conf.key.frame.llc.data.value[0],
                              /* llc_ssap_filter */  conf.conf.key.frame.llc.data.mask[1] ? 1 : 0,
                              /* llc_ssap_value  */  conf.conf.key.frame.llc.data.value[1],
                              /* llc_ctrl_filter */  conf.conf.key.frame.llc.data.mask[2] ? 1 : 0,
                              /* llc_ctrl_value  */  conf.conf.key.frame.llc.data.value[2],
                              /* llc_data_filter */  (conf.conf.key.frame.llc.data.mask[3] || conf.conf.key.frame.llc.data.mask[4]) ? 1 : 0,
                              /* llc_data_value  */  conf.conf.key.frame.llc.data.value[3], conf.conf.key.frame.llc.data.value[4],
                              /* llc_data_mask   */  conf.conf.key.frame.llc.data.mask[3], conf.conf.key.frame.llc.data.mask[4]);
            } else {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/0/FF/0/FF/0/FF/0/FFFF/FFFF");
            }
            (void) cyg_httpd_write_chunked(p->outbuffer, ct);

            if (conf.conf.key.type == VTSS_ECE_TYPE_SNAP) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u/%s/%u/%02X%02X",
                              /* sanp_oui_filter */  (conf.conf.key.frame.snap.data.mask[0] || conf.conf.key.frame.snap.data.mask[1] || conf.conf.key.frame.snap.data.mask[2]) ? 1 : 0,
                              /* sanp_oui_value  */  misc_oui_addr_txt(conf.conf.key.frame.snap.data.value, str_buff),
                              /* sanp_pid_filter */  (conf.conf.key.frame.snap.data.mask[3] || conf.conf.key.frame.snap.data.mask[4]) ? 1 : 0,
                              /* sanp_pid_value  */  conf.conf.key.frame.snap.data.value[3], conf.conf.key.frame.snap.data.value[4]);
            } else {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/0/00-00-00/0/FFFF");
            }
            (void) cyg_httpd_write_chunked(p->outbuffer, ct);

            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u", conf.data.l2cp.proto);
            (void) cyg_httpd_write_chunked(p->outbuffer, ct);
#else
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u",
                          /* frame_type */  conf.conf.key.type);
            (void) cyg_httpd_write_chunked(p->outbuffer, ct);
#endif /* VTSS_ARCH_SERVAL */

#if defined(VTSS_ARCH_CARACAL) || defined(VTSS_ARCH_SERVAL)
            if (conf.conf.key.type == VTSS_ECE_TYPE_IPV4) {
                common_filter = conf.conf.key.frame.ipv4.proto.mask ? conf.conf.key.frame.ipv4.proto.value == 17 ? 1 : conf.conf.key.frame.ipv4.proto.value == 6 ? 2 : 3 : 0;
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u/%u",
                              /* proto_filter */   common_filter,
                              /* proto        */   common_filter ? conf.conf.key.frame.ipv4.proto.value : 0);
                (void) cyg_httpd_write_chunked(p->outbuffer, ct);

                common_filter = conf.conf.key.frame.ipv4.sip.mask == 0 ? 0 : conf.conf.key.frame.ipv4.sip.mask == 0xFFFFFFFF ? 1 : 2;
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u/%s/%s",
                              /* sip_filter   */   common_filter,
                              /* sip          */   common_filter ? misc_ipv4_txt(conf.conf.key.frame.ipv4.sip.value, str_buff) : "0.0.0.0",
                              /* sip_mask     */   common_filter == 2 ? misc_ipv4_txt(conf.conf.key.frame.ipv4.sip.mask, str_buff1) : "255.255.255.255");
                (void) cyg_httpd_write_chunked(p->outbuffer, ct);

#if defined(VTSS_ARCH_SERVAL)
                common_filter = conf.conf.key.frame.ipv4.dip.mask == 0 ? 0 : conf.conf.key.frame.ipv4.dip.mask == 0xFFFFFFFF ? 1 : 2;
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u/%s/%s",
                              /* dip_filter   */   common_filter,
                              /* dip          */   common_filter ? misc_ipv4_txt(conf.conf.key.frame.ipv4.dip.value, str_buff) : "0.0.0.0",
                              /* dip_mask     */   common_filter == 2 ? misc_ipv4_txt(conf.conf.key.frame.ipv4.dip.mask, str_buff1) : "255.255.255.255");
                (void) cyg_httpd_write_chunked(p->outbuffer, ct);
#endif /* VTSS_ARCH_SERVAL */
            } else {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/0/0/0/0.0.0.0/255.255.255.255");
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
#if defined(VTSS_ARCH_SERVAL)
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/0/0.0.0.0/255.255.255.255");
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
#endif /* VTSS_ARCH_SERVAL */
            }
#endif /* VTSS_ARCH_CARACAL/SERVAL */

#if defined(VTSS_ARCH_JAGUAR_1)
            range_filter = conf.conf.key.type == VTSS_ECE_TYPE_IPV6 ? handler_get_filter_range_mapping(&conf.conf.key.frame.ipv6.dscp) : 0;

            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u/%u/%u",
                          /* dscp_filter   */   range_filter,
                          /* dscp_low      */   range_filter == 0 ? 0 : range_filter == 1 ? conf.conf.key.type == VTSS_ECE_TYPE_IPV6 ? conf.conf.key.frame.ipv6.dscp.vr.v.value : conf.conf.key.frame.ipv4.dscp.vr.v.value : conf.conf.key.type == VTSS_ECE_TYPE_IPV6 ? conf.conf.key.frame.ipv6.dscp.vr.r.low : conf.conf.key.frame.ipv4.dscp.vr.r.low,
                          /* dscp_high     */   range_filter == 2 ? conf.conf.key.type == VTSS_ECE_TYPE_IPV6 ? conf.conf.key.frame.ipv6.dscp.vr.r.high : conf.conf.key.frame.ipv4.dscp.vr.r.high : 63);
            (void) cyg_httpd_write_chunked(p->outbuffer, ct);
#else
            range_filter = conf.conf.key.type == VTSS_ECE_TYPE_IPV4 ? handler_get_filter_range_mapping(&conf.conf.key.frame.ipv4.dscp) : 0;
#endif /* VTSS_ARCH_JAGUAR_1 */

#if defined(VTSS_ARCH_CARACAL) || defined(VTSS_ARCH_SERVAL)
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u/%u/%u",
                          /* dscp_filter   */   range_filter,
                          /* dscp_low      */   range_filter == 0 ? 0 : range_filter == 1 ? conf.conf.key.frame.ipv4.dscp.vr.v.value : conf.conf.key.frame.ipv4.dscp.vr.r.low,
                          /* dscp_high     */   range_filter == 2 ? conf.conf.key.frame.ipv4.dscp.vr.r.high : 63);
            (void) cyg_httpd_write_chunked(p->outbuffer, ct);

            if (conf.conf.key.type == VTSS_ECE_TYPE_ANY) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/0/0/0/65535/0/0/65535/0/0/0/0/0");
                (void) cyg_httpd_write_chunked(p->outbuffer, ct);

#if defined(VTSS_ARCH_SERVAL)
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/0/0/0");
                (void) cyg_httpd_write_chunked(p->outbuffer, ct);
#endif /* VTSS_ARCH_SERVAL */

                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/0/0/63/0/0/65535/0/0/65535");
            } else if (conf.conf.key.type == VTSS_ECE_TYPE_IPV4) {
                range_filter = handler_get_filter_range_mapping(&conf.conf.key.frame.ipv4.sport);
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u/%u/%u/%u",
                              /* fragment     */   conf.conf.key.frame.ipv4.fragment,
                              /* sport_filter */   range_filter,
                              /* sport_low    */   range_filter == 0 ? 0 : range_filter == 1 ? conf.conf.key.frame.ipv4.sport.vr.v.value : conf.conf.key.frame.ipv4.sport.vr.r.low,
                              /* sport_high   */   range_filter == 2 ? conf.conf.key.frame.ipv4.sport.vr.r.high : 65535);
                (void) cyg_httpd_write_chunked(p->outbuffer, ct);

                range_filter = handler_get_filter_range_mapping(&conf.conf.key.frame.ipv4.dport);
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u/%u/%u",
                              /* dport_filter */   range_filter,
                              /* dport_low    */   range_filter == 0 ? 0 : range_filter == 1 ? conf.conf.key.frame.ipv4.dport.vr.v.value : conf.conf.key.frame.ipv4.dport.vr.r.low,
                              /* dport_high   */   range_filter == 2 ? conf.conf.key.frame.ipv4.dport.vr.r.high : 65535);
                (void) cyg_httpd_write_chunked(p->outbuffer, ct);

                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/0/0/0/0/0");
                (void) cyg_httpd_write_chunked(p->outbuffer, ct);
                
#if defined(VTSS_ARCH_SERVAL)
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/0/0/0");
                (void) cyg_httpd_write_chunked(p->outbuffer, ct);
#endif /* VTSS_ARCH_SERVAL */

                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/0/0/63/0/0/65535/0/0/65535");
            } else {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/0/0/0/65535/0/0/65535");
                (void) cyg_httpd_write_chunked(p->outbuffer, ct);

                common_filter = conf.conf.key.frame.ipv6.proto.mask ? conf.conf.key.frame.ipv6.proto.value == 17 ? 1 : conf.conf.key.frame.ipv6.proto.value == 6 ? 2 : 3 : 0;
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u/%u",
                              /* proto_v6_filter */   common_filter,
                              /* proto_v6        */   common_filter ? conf.conf.key.frame.ipv6.proto.value : 0);
                (void) cyg_httpd_write_chunked(p->outbuffer, ct);

                common_filter = (conf.conf.key.frame.ipv6.sip.mask[12] |
                                 conf.conf.key.frame.ipv6.sip.mask[13] |
                                 conf.conf.key.frame.ipv6.sip.mask[14] |
                                 conf.conf.key.frame.ipv6.sip.mask[15]) ? 1 : 0;
                memcpy(&var_hex_value, &conf.conf.key.frame.ipv6.sip.value[12], 4);
                var_hex_value = htonl(var_hex_value);
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u/%s",
                              /* sip_v6_filter   */   common_filter,
                              /* sip_v6          */   common_filter ? misc_ipv4_txt(var_hex_value, str_buff) : "0.0.0.0");
                (void) cyg_httpd_write_chunked(p->outbuffer, ct);
                memcpy(&var_hex_value, &conf.conf.key.frame.ipv6.sip.mask[12], 4);
                var_hex_value = htonl(var_hex_value);
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%s",
                              /* sip_v6_mask     */   common_filter ? misc_ipv4_txt(var_hex_value, str_buff) : "255.255.255.255");
                (void) cyg_httpd_write_chunked(p->outbuffer, ct);

#if defined(VTSS_ARCH_SERVAL)
                common_filter = (conf.conf.key.frame.ipv6.dip.mask[12] |
                                 conf.conf.key.frame.ipv6.dip.mask[13] |
                                 conf.conf.key.frame.ipv6.dip.mask[14] |
                                 conf.conf.key.frame.ipv6.dip.mask[15]) ? 1 : 0;
                memcpy(&var_hex_value, &conf.conf.key.frame.ipv6.dip.value[12], 4);
                var_hex_value = htonl(var_hex_value);
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u/%s",
                              /* dip_v6_filter   */   common_filter,
                              /* dip_v6          */   common_filter ? misc_ipv4_txt(var_hex_value, str_buff) : "0.0.0.0");
                (void) cyg_httpd_write_chunked(p->outbuffer, ct);
                memcpy(&var_hex_value, &conf.conf.key.frame.ipv6.dip.mask[12], 4);
                var_hex_value = htonl(var_hex_value);
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%s",
                              /* dip_v6_mask     */   common_filter ? misc_ipv4_txt(var_hex_value, str_buff) : "255.255.255.255");
                (void) cyg_httpd_write_chunked(p->outbuffer, ct);
#endif /* VTSS_ARCH_SERVAL */

                range_filter = handler_get_filter_range_mapping(&conf.conf.key.frame.ipv6.dscp);
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u/%u/%u",
                              /* dscp_v6_filter */   range_filter,
                              /* dscp_v6_low    */   range_filter == 0 ? 0 : range_filter == 1 ? conf.conf.key.frame.ipv6.dscp.vr.v.value : conf.conf.key.frame.ipv6.dscp.vr.r.low,
                              /* dscp_v6_high   */   range_filter == 2 ? conf.conf.key.frame.ipv6.dscp.vr.r.high : 63);
                (void) cyg_httpd_write_chunked(p->outbuffer, ct);

                range_filter = handler_get_filter_range_mapping(&conf.conf.key.frame.ipv6.sport);
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u/%u/%u",
                              /* sport_v6_filter */   range_filter,
                              /* sport_v6_low    */   range_filter == 0 ? 0 : range_filter == 1 ? conf.conf.key.frame.ipv6.sport.vr.v.value : conf.conf.key.frame.ipv6.sport.vr.r.low,
                              /* sport_v6_high   */   range_filter == 2 ? conf.conf.key.frame.ipv6.sport.vr.r.high : 65535);
                (void) cyg_httpd_write_chunked(p->outbuffer, ct);

                range_filter = handler_get_filter_range_mapping(&conf.conf.key.frame.ipv6.dport);
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u/%u/%u",
                              /* dport_v6_filter */   range_filter,
                              /* dport_v6_low    */   range_filter == 0 ? 0 : range_filter == 1 ? conf.conf.key.frame.ipv6.dport.vr.v.value : conf.conf.key.frame.ipv6.dport.vr.r.low,
                              /* dport_v6_high   */   range_filter == 2 ? conf.conf.key.frame.ipv6.dport.vr.r.high : 65535);
            }
            (void) cyg_httpd_write_chunked(p->outbuffer, ct);
#endif /* VTSS_ARCH_CARACAL/SERVAL */

            // direction
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u",
                          /* direction     */   conf.conf.action.dir);
            (void) cyg_httpd_write_chunked(p->outbuffer, ct);

#if defined(VTSS_ARCH_SERVAL)
            // rule_type, tx_lookup
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u/%u/%u/%u",
                          /* rule type     */   conf.conf.action.rule,
                          /* tx lookup     */   conf.conf.action.tx_lookup,
                          /* L2CP mode     */   conf.data.l2cp.mode,
                          /* L2CP dmac     */   conf.data.l2cp.dmac);
            (void) cyg_httpd_write_chunked(p->outbuffer, ct);
#endif /* VTSS_ARCH_SERVAL */

            // evc_id_filter ("None", "Specific")
            if (conf.conf.action.evc_id == VTSS_EVC_ID_NONE) {
                common_filter = 0;
            } else {
                common_filter = 1;
            }

            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u/%u",
                          /* evc_id_filter */   common_filter,
                          /* evc_id        */   common_filter == 1 ? evc_id_i2u(conf.conf.action.evc_id) : 1);
            (void) cyg_httpd_write_chunked(p->outbuffer, ct);

#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
            // policer_id_filter ("Specific", "Discard", "None", "EVC")
            if (conf.conf.action.policer_id == VTSS_EVC_POLICER_ID_DISCARD) {
                policer_id_filter = 1;
            } else if (conf.conf.action.policer_id == VTSS_EVC_POLICER_ID_NONE) {
                policer_id_filter = 2;
            } else if (conf.conf.action.policer_id == VTSS_EVC_POLICER_ID_EVC) {
                policer_id_filter = 3;
            } else {
                policer_id_filter = 0;
            }

            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u/%u",
                          /* policer_id_filter */   policer_id_filter,
                          /* policer_id        */   policer_id_filter == 0 ? policer_id_i2u(conf.conf.action.policer_id) : 1);
            (void) cyg_httpd_write_chunked(p->outbuffer, ct);
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */

            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u/%u",
                          /* pop       */   conf.conf.action.pop_tag,
                          /* policy_no */   conf.conf.action.policy_no);
            (void) cyg_httpd_write_chunked(p->outbuffer, ct);

#if defined(VTSS_ARCH_CARACAL) || defined(VTSS_ARCH_SERVAL)
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u",
                          /* class */ conf.conf.action.prio_enable ? conf.conf.action.prio : VTSS_PRIOS);
            (void) cyg_httpd_write_chunked(p->outbuffer, ct);
#endif /* VTSS_ARCH_CARACAL/SERVAL */

#if defined(VTSS_ARCH_SERVAL)
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u",
                          /* dp */ conf.conf.action.dp_enable ? conf.conf.action.dp : 2);
            (void) cyg_httpd_write_chunked(p->outbuffer, ct);
#endif /* VTSS_ARCH_SERVAL */

            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u",
                          /* ot_mode */  conf.conf.action.outer_tag.enable);
            (void) cyg_httpd_write_chunked(p->outbuffer, ct);

#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u",
                          /* ot_vid */  conf.conf.action.outer_tag.vid);
            (void) cyg_httpd_write_chunked(p->outbuffer, ct);
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */

            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u/%u",
#if defined(VTSS_ARCH_SERVAL)
                          /* ot_preserve */ conf.conf.action.outer_tag.pcp_mode,
#else
                          /* ot_preserve */ conf.conf.action.outer_tag.pcp_dei_preserve,
#endif /* VTSS_ARCH_SERVAL */
                          /* ot_pcp      */ conf.conf.action.outer_tag.pcp);
            (void) cyg_httpd_write_chunked(p->outbuffer, ct);

#if defined(VTSS_ARCH_SERVAL)
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u",
                          /* ot_dei_mode */ conf.conf.action.outer_tag.dei_mode);
            (void) cyg_httpd_write_chunked(p->outbuffer, ct);
#endif /* VTSS_ARCH_JAGUAR_SERVAL */

            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u",
                          /* ot_dei */ conf.conf.action.outer_tag.dei);
            (void) cyg_httpd_write_chunked(p->outbuffer, ct);

#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u/%u/%u/%u",
                          /* it_type     */ conf.conf.action.inner_tag.type,
                          /* it_vid      */ conf.conf.action.inner_tag.vid,
#if defined(VTSS_ARCH_SERVAL)
                          /* it_preserve */ conf.conf.action.inner_tag.pcp_mode,
#else
                          /* it_preserve */ conf.conf.action.inner_tag.pcp_dei_preserve,
#endif /* VTSS_ARCH_SERVAL */
                          /* it_pcp      */ conf.conf.action.inner_tag.pcp);
            (void) cyg_httpd_write_chunked(p->outbuffer, ct);
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */

#if defined(VTSS_ARCH_SERVAL)
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u",
                          /* it_dei_mode */ conf.conf.action.inner_tag.dei_mode);
            (void) cyg_httpd_write_chunked(p->outbuffer, ct);
#endif /* VTSS_ARCH_JAGUAR_SERVAL */

#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u",
                          /* it_dei */   conf.conf.action.inner_tag.dei);
            (void) cyg_httpd_write_chunked(p->outbuffer, ct);
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */

            // uni_ports
            port_iter_init_local(&pit);
            while (port_iter_getnext(&pit)) {
                iport = pit.iport;
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u",
                              info[iport].nni_count ? 2 : conf.conf.key.port_list[iport] == VTSS_ECE_PORT_NONE ? 0 : 1);
                (void) cyg_httpd_write_chunked(p->outbuffer, ct);
            }
        } else {
#if defined(VTSS_ARCH_CARACAL)
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "0/%u/0/0/00-00-00-00-00-01/0/0/1/4095/14/0/0/0/0/0/0.0.0.0/255.255.255.255/0/0/63/0/0/0/65535/0/0/65535/0/0/0/0/0.0.0.0/255.255.255.255/0/63/0/0/65535/0/0/65535/0/1/1/0/0/8/0/0/0/0", next_ece_id);
#elif defined(VTSS_ARCH_JAGUAR_1)
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "0/%u/0/0/1/4095/14/0/0/0/1/4095/14/0/0/0/0/63/0/1/1/2/0/0/0/0/1/0/0/0/0/1/0/0/0", next_ece_id);
#else
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "0/%u/0/0/00-00-00-00-00-01/0/00-00-00-00-00-01/0/0/1/4095/14/0/0/0/1/4095/14/0/0/0/FFFF/0/FFFF/FFFF/0/FF/0/FF/0/FF/0/FFFF/FFFF/0/00-00-00/0/FFFF/0/0/0/0/0.0.0.0/255.255.255.255/0/0.0.0.0/255.255.255.255/0/0/63/0/0/0/65535/0/0/65535/0/0/0/0.0.0.0/255.255.255.255/0/0.0.0.0/255.255.255.255/0/0/63/0/0/65535/0/0/65535/0/0/0/0/0/1/1/0/1/0/0/8/2/0/1/0/0/0/0/0/1/0/0/0/0", next_ece_id);
#endif /* VTSS_ARCH_CARACAL */
            (void) cyg_httpd_write_chunked(p->outbuffer, ct);

            port_iter_init_local(&pit);
            while (port_iter_getnext(&pit)) {
                iport = pit.iport;
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u", info[iport].nni_count ? 2 : 0);
                (void) cyg_httpd_write_chunked(p->outbuffer, ct);
            }
            (void) cyg_httpd_write_chunked(",", 1);
        }
        (void) cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

static cyg_int32 handler_stat_evc_stat(CYG_HTTPD_STATE *p)
{
    int                     ct;
    vtss_port_no_t          iport = VTSS_PORT_NO_START;
    vtss_uport_no_t         uport;
    const char              *var_string;
    size_t                  var_string_len = 0;
#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
    port_iter_t             pit;
    vtss_evc_id_t           uevc_id = 1, ievc_id = 0, temp_evc_id;
    vtss_evc_counters_t     counters;
    evc_mgmt_conf_t         conf, temp_conf;
    vtss_rc                 rc = VTSS_RC_ERROR;
    char                    var_name[EVC_ID_COUNT * 3];
    int                     var_value;
    BOOL                    evc_list[EVC_ID_COUNT];
    evc_mgmt_ece_conf_t     ece_conf;
#else
    vtss_port_counters_t     counters;
    vtss_port_evc_counters_t *evc = &counters.evc;
    vtss_prio_t              prio;
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_EVC)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_GET) {
        // Format: [selected_evc_id];[valid_evc_list];
        //         [class:Lu26]
        //         /[port_no:JR1/SRVL]
        //         /[green_f_rx]/[green_f_tx]/[green_b_rx]/[green_b_tx]
        //         /[yellow_f_rx]/[yellow_f_tx]/[yellow_b_rx]/[yellow_b_tx]
        //         /[red_f_rx]/[red_b_rx]
        //         /[discard_f_rx]/[discard_f_tx]/[discard_b_rx]/[discard_b_tx]|...

        (void) cyg_httpd_start_chunked("html");

#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
        // Get evc_id
        memset(&conf, 0, sizeof(conf));
        if ((var_string = cyg_httpd_form_varable_string(p, "evc_id", &var_string_len)) != NULL &&
            var_string_len > 0) {
            uevc_id = atoi(var_string);
            ievc_id = evc_id_u2i(uevc_id);
            if ((rc = evc_mgmt_get(&ievc_id, &conf, FALSE)) == VTSS_OK) {
                if ((cyg_httpd_form_varable_find(p, "clearall") != NULL)) { // Clear all ports counters
                    /* Find ECE UNI ports */
                    ece_conf.conf.id = EVC_ECE_ID_FIRST;
                    while (evc_mgmt_ece_get(ece_conf.conf.id, &ece_conf, TRUE) == VTSS_RC_OK) {
                        if (ece_conf.conf.action.evc_id != ievc_id) {
                            continue;
                        }
                        port_iter_init_local(&pit);
                        while (port_iter_getnext(&pit)) {
                            iport = pit.iport;
                            if (ece_conf.conf.key.port_list[iport] != VTSS_ECE_PORT_NONE) {
                                conf.conf.network.pb.nni[iport] = TRUE;
                            }
                        }
                    }

                    port_iter_init_local(&pit);
                    while (port_iter_getnext(&pit)) {
                        iport = pit.iport;
                        if (conf.conf.network.pb.nni[iport]) {
                            (void) vtss_evc_counters_clear(NULL, ievc_id, iport);
                        }
                    }
                } else {    // Clear specific port counters
                    port_iter_init_local(&pit);
                    while (port_iter_getnext(&pit)) {
                        iport = pit.iport;
                        uport = iport2uport(iport);
                        sprintf(var_name, "clear_%u", uport);
                        if (cyg_httpd_form_varable_int(p, var_name, &var_value)) {
                            (void) vtss_evc_counters_clear(NULL, ievc_id, iport);
                        }
                    }
                }
            }
        }

        // Get first valid EVC entry if selected EVC ID not exist
        if (rc != VTSS_OK) {
            ievc_id = EVC_ID_FIRST;
            rc = evc_mgmt_get(&ievc_id, &conf, TRUE);
        }
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u;",
                      rc == VTSS_OK ? evc_id_i2u(ievc_id) : 1);
        (void) cyg_httpd_write_chunked(p->outbuffer, ct);

        // Get valid EVC ID List
        memset(evc_list, 0, sizeof(evc_list));
        temp_evc_id = EVC_ID_FIRST;
        while (evc_mgmt_get(&temp_evc_id, &temp_conf, TRUE) == VTSS_OK) {
            evc_list[temp_evc_id] = TRUE;   // EVC ID start from 0
        }
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s;",
                      mgmt_list2txt(evc_list, 0, EVC_ID_COUNT - 1, var_name));
        (void) cyg_httpd_write_chunked(p->outbuffer, ct);

        // Sendout each entry information
        if (rc == VTSS_OK) {
            // Also show counters for UNI ports of ECEs mapping to the EVC.
            /* Find ECE UNI ports */
            ece_conf.conf.id = EVC_ECE_ID_FIRST;
            while (evc_mgmt_ece_get(ece_conf.conf.id, &ece_conf, TRUE) == VTSS_RC_OK) {
                if (ece_conf.conf.action.evc_id != ievc_id) {
                    continue;
                }
                port_iter_init_local(&pit);
                while (port_iter_getnext(&pit)) {
                    iport = pit.iport;
                    if (ece_conf.conf.key.port_list[iport] != VTSS_ECE_PORT_NONE) {
                        conf.conf.network.pb.nni[iport] = TRUE;
                    }
                }
            }

            port_iter_init_local(&pit);
            while (port_iter_getnext(&pit)) {
                iport = pit.iport;
                if (!conf.conf.network.pb.nni[iport] ||
                    vtss_evc_counters_get(NULL, ievc_id, iport, &counters) != VTSS_OK) {
                    continue;
                }
                uport = iport2uport(iport);
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu|",
                              uport,
                              counters.rx_green.frames,
                              counters.tx_green.frames,
                              counters.rx_green.bytes,
                              counters.tx_green.bytes,
                              counters.rx_yellow.frames,
                              counters.tx_yellow.frames,
                              counters.rx_yellow.bytes,
                              counters.tx_yellow.bytes,
                              counters.rx_red.frames,
                              counters.rx_red.bytes,
                              counters.rx_discard.frames,
                              counters.tx_discard.frames,
                              counters.rx_discard.bytes,
                              counters.tx_discard.bytes);
                (void) cyg_httpd_write_chunked(p->outbuffer, ct);
            }
        }
#else
        // Get port No.
        if ((var_string = cyg_httpd_form_varable_string(p, "port", &var_string_len)) != NULL &&
            var_string_len > 0) {
            uport = atoi(var_string);
            iport = uport2iport(uport);
        }

        if ((cyg_httpd_form_varable_find(p, "clearall") != NULL)) { // Clear all ports counters
            (void) vtss_port_counters_clear(NULL, iport);
        }

        (void) cyg_httpd_write_chunked("1;1;", 4);
        if (vtss_port_counters_get(NULL, iport, &counters) == VTSS_OK) {
            for (prio = VTSS_PRIO_START; prio < VTSS_PRIO_END; prio++) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/%llu/%llu/0/0/%llu/%llu/0/0/%llu/0/%llu/%llu/0/0|",
                              prio,
                              evc->rx_green[prio],
                              evc->tx_green[prio],
                              evc->rx_yellow[prio],
                              evc->tx_yellow[prio],
                              evc->rx_red[prio],
                              evc->rx_green_discard[prio],
                              evc->rx_yellow_discard[prio]);
                (void) cyg_httpd_write_chunked(p->outbuffer, ct);
            }
        }
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */

        (void) cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

static cyg_int32 handler_stat_evc_ece_stat(CYG_HTTPD_STATE *p)
{
#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
    int                 ct;
    vtss_ece_id_t       ece_id = 0;
    port_iter_t         pit;
    vtss_port_no_t      iport;
    vtss_uport_no_t     uport;
    vtss_evc_counters_t counters;
    const char          *var_string;
    size_t              var_string_len = 0;
    evc_mgmt_ece_conf_t conf, temp_conf;
    vtss_rc             rc = VTSS_RC_ERROR;
    char                var_name[EVC_ECE_COUNT * 3];
    int                 var_value;
    BOOL                ece_list[EVC_ECE_COUNT];
    evc_mgmt_conf_t     evc_conf;
    vtss_evc_id_t       evc_id;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_EVC)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_GET) {
        // Format: [selected_ece_id];[valid_ece_list];
        //         [port_no]/[green_f_rx]/[green_f_tx]/[green_b_rx]/[green_b_tx]
        //                  /[yellow_f_rx]/[yellow_f_tx]/[yellow_b_rx]/[yellow_b_tx]
        //                  /[red_f_rx]/[red_b_rx]
        //                  /[discard_f_rx]/[discard_f_tx]/[discard_b_rx]/[discard_b_tx]|...

        (void) cyg_httpd_start_chunked("html");

        // Get ece_id
        memset(&conf, 0, sizeof(conf));
        if ((var_string = cyg_httpd_form_varable_string(p, "ece_id", &var_string_len)) != NULL &&
            var_string_len > 0) {
            ece_id = atoi(var_string);
            if ((rc = evc_mgmt_ece_get(ece_id, &conf, FALSE)) == VTSS_OK) {
                if ((cyg_httpd_form_varable_find(p, "clearall") != NULL)) { // Clear all ports counters
                    evc_id = conf.conf.action.evc_id;
                    if (evc_id == VTSS_EVC_ID_NONE || evc_mgmt_get(&evc_id, &evc_conf, 0) != VTSS_RC_OK) {
                        memset(&evc_conf, 0, sizeof(evc_conf));
                    }

                    port_iter_init_local(&pit);
                    while (port_iter_getnext(&pit)) {
                        iport = pit.iport;
                        if (conf.conf.key.port_list[iport] == VTSS_ECE_PORT_ROOT || evc_conf.conf.network.pb.nni[iport] == TRUE) {
                            (void) vtss_ece_counters_clear(NULL, ece_id, iport);
                        }
                    }
                } else {    // Clear specific port counters
                    port_iter_init_local(&pit);
                    while (port_iter_getnext(&pit)) {
                        iport = pit.iport;
                        uport = iport2uport(iport);
                        sprintf(var_name, "clear_%u", uport);
                        if (cyg_httpd_form_varable_int(p, var_name, &var_value)) {
                            (void) vtss_ece_counters_clear(NULL, ece_id, iport);
                        }
                    }
                }
            }
        }

        // Get first valid ECE entry if selected ECE ID not exist
        if (rc != VTSS_OK) {
            ece_id = EVC_ECE_ID_FIRST;
            rc = evc_mgmt_ece_get(ece_id, &conf, TRUE);
        }

        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u;",
                      rc == VTSS_OK ? conf.conf.id : 1);
        (void) cyg_httpd_write_chunked(p->outbuffer, ct);

        // Get valid ECE ID List
        memset(ece_list, 0, sizeof(ece_list));
        temp_conf.conf.id = EVC_ECE_ID_FIRST;
        while (evc_mgmt_ece_get(temp_conf.conf.id, &temp_conf, TRUE) == VTSS_OK) {
            ece_list[temp_conf.conf.id - 1] = TRUE; // ECE ID start from 1
        }
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s;",
                      mgmt_list2txt(ece_list, 0, EVC_ECE_COUNT - 1, var_name));
        (void) cyg_httpd_write_chunked(p->outbuffer, ct);

        // Sendout each entry information
        if (rc == VTSS_OK) {
            // Also show counters for NNI ports of the EVC to which the ECE is mapped.
            evc_id = conf.conf.action.evc_id;
            if (evc_id == VTSS_EVC_ID_NONE || evc_mgmt_get(&evc_id, &evc_conf, 0) != VTSS_RC_OK) {
                memset(&evc_conf, 0, sizeof(evc_conf));
            }

            port_iter_init_local(&pit);
            while (port_iter_getnext(&pit)) {
                iport = pit.iport;
                if ((conf.conf.key.port_list[iport] == VTSS_ECE_PORT_NONE && evc_conf.conf.network.pb.nni[iport] == FALSE) ||
                    vtss_ece_counters_get(NULL, conf.conf.id, iport, &counters) != VTSS_OK) {
                    continue;
                }
                uport = iport2uport(iport);
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu|",
                              uport,
                              counters.rx_green.frames,
                              counters.tx_green.frames,
                              counters.rx_green.bytes,
                              counters.tx_green.bytes,
                              counters.rx_yellow.frames,
                              counters.tx_yellow.frames,
                              counters.rx_yellow.bytes,
                              counters.tx_yellow.bytes,
                              counters.rx_red.frames,
                              counters.rx_red.bytes,
                              counters.rx_discard.frames,
                              counters.tx_discard.frames,
                              counters.rx_discard.bytes,
                              counters.tx_discard.bytes);
                (void) cyg_httpd_write_chunked(p->outbuffer, ct);
            }
        }

        (void) cyg_httpd_end_chunked();
    }
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */
    return -1; // Do not further search the file system.
}

/****************************************************************************/
/*  Module JS lib config routine                                            */
/****************************************************************************/

static size_t evc_lib_config_js(char **base_ptr, char **cur_ptr, size_t *length)
{
    char buff[EVC_WEB_BUF_LEN];
    (void) snprintf(buff, EVC_WEB_BUF_LEN,
                    "var configEvcIdMax = %u;\n"
                    "var configEceIdMax = %u;\n"
                    "var configEvcPolicerIdMax = %u;\n"
                    "var configEvcCirMin = 0;\n"
                    "var configEvcCirMax = %u;\n"
                    "var configEvcCbsMin = 0;\n"
                    "var configEvcCbsMax = %u;\n"
                    "var configEvcEirMin = 0;\n"
                    "var configEvcEirMax = %u;\n"
                    "var configEvcEbsMin = 0;\n"
                    "var configEvcEbsMax = %u;\n"
                    "var configEvcVidMin = %u;\n"
                    , EVC_ID_COUNT
                    , EVC_ECE_COUNT
                    , EVC_POL_COUNT
                    , EVC_POLICER_RATE_MAX
                    , EVC_POLICER_LEVEL_MAX
                    , EVC_POLICER_RATE_MAX
                    , EVC_POLICER_LEVEL_MAX
                    , EVC_VID_MIN
                   );
    return webCommonBufferHandler(base_ptr, cur_ptr, length, buff);
}

/****************************************************************************/
/*  JS lib config table entry                                               */
/****************************************************************************/

web_lib_config_js_tab_entry(evc_lib_config_js);

/****************************************************************************/
/*  HTTPD Table Entries                                                     */
/****************************************************************************/

CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_evc, "/config/evc", handler_config_evc);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_evc_edit, "/config/evc_edit", handler_config_evc_edit);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_evc_uni, "/config/evc_uni", handler_config_evc_uni);
#if defined(VTSS_ARCH_SERVAL)
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_evc_l2cp, "/config/evc_l2cp", handler_config_evc_l2cp);
#endif /* VTSS_ARCH_SERVAL*/
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_evc_bw, "/config/evc_bw", handler_config_evc_bw);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_evc_ece, "/config/evc_ece", handler_config_evc_ece);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_evc_ece_edit, "/config/evc_ece_edit", handler_config_evc_ece_edit);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_stat_evc_stat, "/stat/evc_statistics", handler_stat_evc_stat);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_stat_evc_ece_stat, "/stat/evc_ece_statistics", handler_stat_evc_ece_stat);

/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
