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
#include "voice_vlan_api.h"
#include "vlan_api.h"

#ifdef VTSS_SW_OPTION_MVR
#include "mvr_api.h"
#endif /* VTSS_SW_OPTION_MVR */

#ifdef VTSS_SW_OPTION_LLDP
#include "lldp_api.h"
#include "lldp_remote.h"
#endif /* VTSS_SW_OPTION_LLDP */

#define VOICE_VLAN_WEB_BUF_LEN 512
#ifdef VTSS_SW_OPTION_PRIV_LVL
#include "vtss_privilege_api.h"
#include "vtss_privilege_web_api.h"
#endif
#include "port_api.h"

#define VOICE_VLAN_WEB_BUF_LEN 512

/* =================
 * Trace definitions
 * -------------- */
#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>
#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_WEB
#include <vtss_trace_api.h>
/* ============== */

/****************************************************************************/
/*  Web Handler  functions                                                  */
/****************************************************************************/

static cyg_int32 handler_conf_voice_vlan(CYG_HTTPD_STATE *p)
{
    vtss_isid_t             sid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    port_iter_t             pit;
    int                     ct;
    vlan_port_conf_t        vlan_port_conf;
    voice_vlan_conf_t       voice_vlan_conf, voice_vlan_newconf;
    voice_vlan_port_conf_t  voice_vlan_port_conf, voice_vlan_port_newconf;
#ifdef VTSS_SW_OPTION_LLDP
    lldp_struc_0_t          lldp_conf;
#endif
    int                     var_value;
    char                    var_name[32];
    vtss_rc                 rc;
#ifdef VTSS_SW_OPTION_MVR
    ushort                  mvr_vid = 100;
#endif

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_VOICE_VLAN)) {
        return -1;
    }
#endif

    if (voice_vlan_mgmt_conf_get(&voice_vlan_conf) != VTSS_OK) {
        return -1;
    }
    if (voice_vlan_mgmt_port_conf_get(sid, &voice_vlan_port_conf) != VTSS_OK) {
        return -1;
    }
#ifdef VTSS_SW_OPTION_LLDP
    lldp_mgmt_get_config(&lldp_conf, sid);
#endif

    (void) port_iter_init(&pit, NULL, sid, PORT_ITER_SORT_ORDER_UPORT, PORT_ITER_FLAGS_NORMAL);

    if (p->method == CYG_HTTPD_METHOD_POST) {
        /* store form data */
        voice_vlan_newconf = voice_vlan_conf;

        //voice_vlan_mode
        if (cyg_httpd_form_varable_int(p, "voice_vlan_mode", &var_value)) {
            voice_vlan_newconf.mode = var_value;
        }

        //vid
        if (cyg_httpd_form_varable_int(p, "vid", &var_value)) {
            voice_vlan_newconf.vid = var_value;
        }

#if VOICE_VLAN_CHECK_CONFLICT_CONF
        if ((rc = VOICE_VLAN_is_valid_voice_vid(voice_vlan_newconf.vid)) != VTSS_OK) {
            if (rc == VOICE_VLAN_ERROR_VID_IS_CONFLICT_WITH_MGMT_VID) {
                send_custom_error(p, "The Voice VLAN ID should not equal switch management VLAN ID", " ", 1);
                return -1;
            } else if (rc == VOICE_VLAN_ERROR_VID_IS_CONFLICT_WITH_MVR_VID) {
                send_custom_error(p, "The Voice VLAN ID should not equal MVR VLAN ID", " ", 1);
                return -1;
            } else if (rc == VOICE_VLAN_ERROR_VID_IS_CONFLICT_WITH_STATIC_VID) {
                send_custom_error(p, "The Voice VLAN ID should not existing VLAN ID", " ", 1);
                return -1;
            } else if (rc == VOICE_VLAN_ERROR_VID_IS_CONFLICT_WITH_PVID) {
                send_custom_error(p, "The Voice VLAN ID should not equal Port PVID", " ", 1);
                return -1;
            }
        }
#endif

        //age_time
        if (cyg_httpd_form_varable_int(p, "age_time", &var_value)) {
            voice_vlan_newconf.age_time = var_value;
        }

        //traffic_class
        if (cyg_httpd_form_varable_int(p, "traffic_class", &var_value)) {
            voice_vlan_newconf.traffic_class = uprio2iprio(var_value);
        }

        if (memcmp(&voice_vlan_newconf, &voice_vlan_conf, sizeof(voice_vlan_newconf)) != 0) {
            T_D("Calling voice_vlan_mgmt_conf_set()");
            if ((rc = voice_vlan_mgmt_conf_set(&voice_vlan_newconf)) != VTSS_OK) {
                if (rc == VOICE_VLAN_ERROR_IS_CONFLICT_WITH_LLDP) {
                    char err[64];
                    sprintf(err, "The LLDP feature should be enabled first");
                    send_custom_error(p, err, err, strlen(err));
                } else {
                    T_E("voice_vlan_mgmt_conf_set(): failed");
                    redirect(p, "/voice_vlan_config.htm");
                }
                return -1;
            }
        }

        /* store form port data */
        voice_vlan_port_newconf = voice_vlan_port_conf;

        while (port_iter_getnext(&pit)) {
            //port_mode
            sprintf(var_name, "port_mode_%u", pit.uport);
            if (cyg_httpd_form_varable_int(p, var_name, &var_value)) {
                voice_vlan_port_newconf.port_mode[pit.iport] = var_value;

#if VOICE_VLAN_CHECK_CONFLICT_CONF
                // Check LLDP port mode
                if (voice_vlan_newconf.mode == VOICE_VLAN_MGMT_ENABLED &&
                    voice_vlan_port_newconf.port_mode[pit.iport] == VOICE_VLAN_PORT_MODE_AUTO &&
                    voice_vlan_port_conf.discovery_protocol[pit.iport] != VOICE_VLAN_DISCOVERY_PROTOCOL_OUI
#ifdef VTSS_SW_OPTION_LLDP
                    && (lldp_conf.admin_state[pit.iport] == LLDP_DISABLED || lldp_conf.admin_state[pit.iport] == LLDP_ENABLED_TX_ONLY)
#endif
                   ) {
                    char err[64];
                    sprintf(err, "The LLDP mode is disabled on Port %u", pit.uport);
                    send_custom_error(p, err, err, strlen(err));
                    return -1;
                }
#endif
            }

            //security
            sprintf(var_name, "security_%u", pit.uport);
            if (cyg_httpd_form_varable_int(p, var_name, &var_value)) {
                voice_vlan_port_newconf.security[pit.iport] = var_value;
            }

            //discovery_protocol
            sprintf(var_name, "discovery_protocol_%u", pit.uport);
            if (cyg_httpd_form_varable_int(p, var_name, &var_value)) {
                voice_vlan_port_newconf.discovery_protocol[pit.iport] = var_value;
            }
        }

        if (memcmp(&voice_vlan_port_newconf, &voice_vlan_port_conf, sizeof(voice_vlan_port_newconf)) != 0) {
            T_D("Calling voice_vlan_mgmt_port_conf_set()");
            if ((rc = voice_vlan_mgmt_port_conf_set(sid, &voice_vlan_port_newconf)) != VTSS_OK) {
                if (rc == VOICE_VLAN_ERROR_IS_CONFLICT_WITH_LLDP) {
                    char err[64];
                    sprintf(err, "The LLDP feature should be enabled first");
                    send_custom_error(p, err, err, strlen(err));
                    return -1;
                } else {
                    T_E("voice_vlan_mgmt_port_conf_set(): failed");
                }
            }
        }

        redirect(p, "/voice_vlan_config.htm");
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        (void)cyg_httpd_start_chunked("html");

        /* get form data
           Format: [voice_vlan_mode],[vid],[age_time],[traffic_class],[mgmt_vid],[mvr_vid],[port_no]/[port_mode]/[security]/[discovery_protocol]/[pvid]/[lldp_mode]/[vlan_aware]|...
        */

        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d,%d,%d,%d,",
                      voice_vlan_conf.mode, voice_vlan_conf.vid, voice_vlan_conf.age_time, iprio2uprio(voice_vlan_conf.traffic_class));
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);

        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "0,");
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);

#ifdef VTSS_SW_OPTION_MVR
        (void) mvr_mgmt_get_mvid(&mvr_vid);
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d,", mvr_vid);
#else
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "0,");
#endif
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        while (port_iter_getnext(&pit)) {
            if (vlan_mgmt_port_conf_get(sid, pit.iport, &vlan_port_conf, VLAN_USER_VOICE_VLAN) != VTSS_OK) {
                continue;
            }
#ifdef VTSS_SW_OPTION_LLDP
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/%d/%d/%d/%u/%d/%d|",
#else
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/%d/%d/%d/%u/%d|",
#endif
                          pit.uport,
                          voice_vlan_port_conf.port_mode[pit.iport],
                          voice_vlan_port_conf.security[pit.iport],
                          voice_vlan_port_conf.discovery_protocol[pit.iport],
                          vlan_port_conf.pvid,
#ifdef VTSS_SW_OPTION_LLDP
                          (lldp_conf.admin_state[pit.iport] == LLDP_ENABLED_RX_TX || lldp_conf.admin_state[pit.iport] == LLDP_ENABLED_RX_ONLY) ? 1 : 0,
#endif
                          vlan_port_conf.port_type);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        }
        (void)cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

static cyg_int32 handler_conf_voice_vlan_oui(CYG_HTTPD_STATE *p)
{
    int                     ct, idx;
    voice_vlan_oui_entry_t  prev_entry, entry;
    char                    search_str[64], buf[16];
    char                    encoded_string[3 * VOICE_VLAN_MAX_DESCRIPTION_LEN];
    const char              *var_string;
    size_t                  len;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_VOICE_VLAN)) {
        return -1;
    }
#endif

    memset(&prev_entry, 0x0, sizeof(prev_entry));
    memset(&entry, 0x0, sizeof(entry));

    if (p->method == CYG_HTTPD_METHOD_POST) {
        //delete entry
        while (voice_vlan_oui_entry_get(&entry, TRUE) == VTSS_OK) {
            (void) misc_oui_addr_txt(entry.oui_addr, buf);
            sprintf(search_str, "delete_%s", buf);
            if (cyg_httpd_form_varable_find(p, search_str)) {   /* "delete" if checked */
                (void) voice_vlan_oui_entry_del(&entry);
                entry = prev_entry;
            } else {
                prev_entry = entry;
            }
        };

        //add new entry
        for (idx = 1; idx <= VOICE_VLAN_OUI_ENTRIES_CNT; idx++) {
            //oui_addr
            sprintf(search_str, "new_oui_addr_%d", idx);
            if (cyg_httpd_form_varable_oui(p, search_str, entry.oui_addr)) {

                //description
                sprintf(search_str, "new_description_%d", idx);
                var_string = cyg_httpd_form_varable_string(p, search_str, &len);
                if (len > 0) {
                    if (cgi_unescape(var_string, entry.description, len, sizeof(entry.description)) == FALSE) {
                        continue;
                    }
                } else {
                    entry.description[0] = '\0';
                }

                entry.valid = 1;
                if (voice_vlan_oui_entry_add(&entry) != VTSS_OK) {
                    T_W("voice_vlan_oui_entry_add() failed");
                }
            }
        }

        redirect(p, "/voice_vlan_oui.htm");
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        (void)cyg_httpd_start_chunked("html");

        /* get form data
           Format: <max_entries_num>,<oui_addr>/<description>|...
        */
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d,", VOICE_VLAN_OUI_ENTRIES_CNT);
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);

        while (voice_vlan_oui_entry_get(&entry, TRUE) == VTSS_OK) {
            (void) cgi_escape(entry.description, encoded_string);
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s/%s|",
                          misc_oui_addr_txt(entry.oui_addr, buf), encoded_string);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        };
        (void)cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

/****************************************************************************/
/*  Module JS lib config routine                                            */
/****************************************************************************/

static size_t voice_vlan_lib_config_js(char **base_ptr, char **cur_ptr, size_t *length)
{
    char buff[VOICE_VLAN_WEB_BUF_LEN];
    (void) snprintf(buff, VOICE_VLAN_WEB_BUF_LEN,
                    "var configVoiceVlanOuiEntryCnt = %d;\n"
#if !defined(VOICE_VLAN_CLASS_SUPPORTED)
                    "function configHasVoiceVlanClass() {\n"
                    "  return 0;\n"
                    "}\n"
#endif /* VOICE_VLAN_CLASS_SUPPORTED */
                    , VOICE_VLAN_OUI_ENTRIES_CNT);
    return webCommonBufferHandler(base_ptr, cur_ptr, length, buff);
}

/****************************************************************************/
/*  JS lib config table entry                                               */
/****************************************************************************/

web_lib_config_js_tab_entry(voice_vlan_lib_config_js);

#if !defined(VTSS_SW_OPTION_LLDP) || !defined(VOICE_VLAN_CLASS_SUPPORTED)
/****************************************************************************/
/*  Module Filter CSS routine                                               */
/****************************************************************************/
static size_t voice_vlan_lib_filter_css(char **base_ptr, char **cur_ptr, size_t *length)
{
    char buff[VOICE_VLAN_WEB_BUF_LEN];
    (void) snprintf(buff, VOICE_VLAN_WEB_BUF_LEN,
#if !defined(VTSS_SW_OPTION_LLDP)
                    ".hasLLDPVoiceVlan { display: none; }\r\n"
#endif /* VTSS_SW_OPTION_LLDP */
#if !defined(VOICE_VLAN_CLASS_SUPPORTED)
                    ".hasClassVoiceVlan { display: none; }\r\n"
#endif /* VOICE_VLAN_CLASS_SUPPORTED */
                   );
    return webCommonBufferHandler(base_ptr, cur_ptr, length, buff);
}

/****************************************************************************/
/*  Filter CSS table entry                                                  */
/****************************************************************************/
web_lib_filter_css_tab_entry(voice_vlan_lib_filter_css);
#endif /* !VTSS_SW_OPTION_LLDP && !VOICE_VLAN_CLASS_SUPPORTED */

/****************************************************************************/
/*  HTTPD Table Entries                                                     */
/****************************************************************************/

CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_conf_voice_vlan, "/config/voice_vlan_config", handler_conf_voice_vlan);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_conf_voice_vlan_oui, "/config/voice_vlan_oui", handler_conf_voice_vlan_oui);

/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
