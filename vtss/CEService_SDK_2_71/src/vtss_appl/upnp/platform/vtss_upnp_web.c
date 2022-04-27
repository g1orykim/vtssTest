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

#include <stdio.h>
#include "main.h"
#include "web_api.h"
#include "vtss_upnp_api.h"
#ifdef VTSS_SW_OPTION_PRIV_LVL
#include "vtss_privilege_api.h"
#include "vtss_privilege_web_api.h"
#endif

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_UPNP

/* =================
 * Trace definitions
 * -------------- */
#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>
#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_WEB
#include <vtss_trace_api.h>
/* ============== */

static void get_httpd_host(CYG_HTTPD_STATE *p, char *ip_str)
{
    sprintf(ip_str, "%d.%d.%d.%d", p->host[0], p->host[1], p->host[2], p->host[3]);
}

/****************************************************************************/
/*  Web Handler  functions                                                  */
/****************************************************************************/
/*lint -esym(459,udnstr)*/
cyg_int32 handler_config_description_xml(CYG_HTTPD_STATE *p)
{
    char udnstr[UPNP_MGMT_UDNSTR_SIZE];
    char ip_address[UPNP_MGMT_IPSTR_SIZE];

    char *buffer;
    char *buffer1;

    vtss_upnp_xml_get(&buffer1);
    int buffer_size = strlen(buffer1) + sizeof(udnstr) + sizeof(ip_address) + 1;

    buffer = VTSS_MALLOC(buffer_size);

#if 1 /* CP: avoid use NULL */
    if ( buffer == NULL ) {
        return -1;
    }
#endif

    vtss_upnp_get_udnstr(&udnstr[0]);
    get_httpd_host (p, ip_address);

    int cnt = snprintf(buffer, buffer_size, buffer1, udnstr, ip_address);

    /* Make this look like a cache-able ires asset */
    cyg_httpd_ires_table_entry entry;
    entry.f_pname = "/xml/devicedesc.xml";
    entry.f_ptr = (unsigned char *)buffer;
    entry.f_size = cnt;
    cyg_httpd_send_ires(&entry);

    VTSS_FREE(buffer);
    return -1; // Do not further search the file system.
}

cyg_int32 handler_config_upnp(CYG_HTTPD_STATE *p)
{
    int         ct;
    upnp_conf_t conf, newconf;
    int         var_value;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_UPNP)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {
        /* store form data */
        if (upnp_mgmt_conf_get(&conf) == VTSS_OK) {
            newconf = conf;

            //mode
            if (cyg_httpd_form_varable_int(p, "mode", &var_value)) {
                newconf.mode = var_value;
            }

            //ttl
            if (cyg_httpd_form_varable_int(p, "ttl", &var_value)) {
                newconf.ttl = var_value;
            }

            //interval
            if (cyg_httpd_form_varable_int(p, "interval", &var_value)) {
                newconf.adv_interval = var_value;
            }

            if (memcmp(&newconf, &conf, sizeof(newconf)) != 0) {
                T_D("Calling upnp_mgmt_conf_set()");
                if (upnp_mgmt_conf_set(&newconf) < 0) {
                    T_E("upnp_mgmt_conf_set(): failed");
                }
            }
        }
        redirect(p, "/upnp.htm");

    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        (void)cyg_httpd_start_chunked("html");

        /* get form data
           Format: [mode]/[ttl]/[interval]
        */

        if (upnp_mgmt_conf_get(&conf) == VTSS_OK) {
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%ld/%d/%ld",
                          conf.mode,
                          conf.ttl,
                          conf.adv_interval);

            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        }
        (void)cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

/****************************************************************************/
/*  HTTPD Table Entries                                                     */
/****************************************************************************/

CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_description_xml, "/xml/devicedesc.xml", handler_config_description_xml);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_upnp, "/config/upnp", handler_config_upnp);

/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
