/*

 Vitesse API software.

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


#ifndef _IP2_LEGACY_H_
#define _IP2_LEGACY_H_
#ifdef VTSS_SW_OPTION_WEB
#include "web_api.h"
#include "vtss_types.h"
vtss_rc web_parse_ipv4    (CYG_HTTPD_STATE *p, char *name, vtss_ipv4_t *ipv4);
vtss_rc web_parse_ipv6    (CYG_HTTPD_STATE *p, char *name, vtss_ipv6_t *ipv6);
vtss_rc web_parse_ipv4_fmt(CYG_HTTPD_STATE *p, vtss_ipv4_t *pip, const char *fmt, ...);
vtss_rc web_parse_ipv6_fmt(CYG_HTTPD_STATE *p, vtss_ipv6_t *pip, const char *fmt, ...);
vtss_rc web_parse_ip_fmt  (CYG_HTTPD_STATE *p, vtss_ip_addr_t *pip, const char *fmt, ...);
vtss_rc web_parse_ip_fmt  (CYG_HTTPD_STATE *p, vtss_ip_addr_t *pip, const char *fmt, ...);
#endif
#endif /* _IP2_LEGACY_H_ */

