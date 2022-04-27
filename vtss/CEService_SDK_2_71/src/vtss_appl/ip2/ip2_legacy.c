/*

 Vitesse Switch Software.

 Copyright (c) 2002-2012 Vitesse Semiconductor Corporation "Vitesse". All
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

#ifdef VTSS_SW_OPTION_WEB
#include "ip2_legacy.h"
#include "mgmt_api.h"
vtss_rc web_parse_ipv4(CYG_HTTPD_STATE *p, char *name, vtss_ipv4_t *ipv4)
{
    const char *buf;
    size_t     len = 16;
    unsigned   int  a, b, c, d, m;

    if ((buf = cyg_httpd_form_varable_string(p, name, &len)) != NULL) {
        m = sscanf(buf, "%u.%u.%u.%u", &a, &b, &c, &d);
        if (m == 4 && a < 256 && b < 256 && c < 256 && d < 256) {
            *ipv4 = ((a << 24) + (b << 16) + (c << 8) + d);
            return VTSS_RC_OK;
        }
    }

    return VTSS_RC_ERROR;
}

vtss_rc web_parse_ipv4_fmt(CYG_HTTPD_STATE *p, vtss_ipv4_t *pip, const char *fmt, ...)
{
    char instance_name[256];
    va_list va = NULL;
    va_start(va, fmt);
    (void) vsnprintf(instance_name, sizeof(instance_name), fmt, va);
    va_end(va);
    return web_parse_ipv4(p, instance_name, pip);
}

vtss_rc web_parse_ipv6(CYG_HTTPD_STATE *p, char *name, vtss_ipv6_t *ipv6)
{
    const char  *buf;
    size_t      len;
    vtss_rc     rc = VTSS_RC_ERROR;
    char        ip_buf[IPV6_ADDR_IBUF_MAX_LEN];

    if ((buf = cyg_httpd_form_varable_string(p, name, &len)) != NULL) {
        memset(&ip_buf[0], 0x0, sizeof(ip_buf));
        if (len > 0 && cgi_unescape(buf, ip_buf, len, sizeof(ip_buf))) {
            rc = mgmt_txt2ipv6(ip_buf, ipv6);
        }

    }
    return rc;
}

vtss_rc web_parse_ipv6_fmt(CYG_HTTPD_STATE *p, vtss_ipv6_t *pip, const char *fmt, ...)
{
    char instance_name[256];
    va_list va = NULL;
    va_start(va, fmt);
    (void) vsnprintf(instance_name, sizeof(instance_name), fmt, va);
    va_end(va);
    return web_parse_ipv6(p, instance_name, pip);
}

vtss_rc web_parse_ip(CYG_HTTPD_STATE *p, char *name, vtss_ip_addr_t *pip)
{
    if (vtss_ip2_hasipv6() && (web_parse_ipv6(p, name, &pip->addr.ipv6) == VTSS_OK)) {
        pip->type = VTSS_IP_TYPE_IPV6;
        return VTSS_OK;
    } else if (web_parse_ipv4(p, name, &pip->addr.ipv4) == VTSS_OK) {
        pip->type = VTSS_IP_TYPE_IPV4;
        return VTSS_OK;
    }
    pip->type = VTSS_IP_TYPE_NONE;
    return VTSS_INCOMPLETE;
}

vtss_rc web_parse_ip_fmt(CYG_HTTPD_STATE *p, vtss_ip_addr_t *pip, const char *fmt, ...)
{
    char instance_name[256];
    va_list va = NULL;
    va_start(va, fmt);
    (void) vsnprintf(instance_name, sizeof(instance_name), fmt, va);
    va_end(va);
    return web_parse_ip(p, instance_name, pip);
}

#endif
