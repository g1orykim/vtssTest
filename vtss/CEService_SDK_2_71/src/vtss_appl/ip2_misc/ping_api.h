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

 $Id$
 $Revision$

*/

#ifndef _VTSS_PING_API_H_
#define _VTSS_PING_API_H_

#include "ip2_api.h"
#include "cli_api.h"

#define PING_MAX_CLIENT             8

/* Parameters valid range */
#define PING_MIN_PACKET_LEN         2   /* MUST-NOT be ZERO */
#define PING_MAX_PACKET_LEN         1452
#define PING_MIN_PACKET_CNT         1
#define PING_MAX_PACKET_CNT         60
#define PING_MIN_PACKET_INTERVAL    0
#define PING_MAX_PACKET_INTERVAL    30  /* When PING_MIN_PACKET_LEN < 2, Max.Allow is 2 */
#define PING_TIME_STAMP_BASE        ((1 << (8 * PING_MIN_PACKET_LEN)) - 1)
#define PING_TIME_STAMP_VALUE       (cyg_current_time() % PING_TIME_STAMP_BASE)

/* Default configuration */
#define PING_DEF_PACKET_LEN         56
#define PING_DEF_PACKET_CNT         5
#define PING_DEF_PACKET_INTERVAL    0
#define PING_DEF_EGRESS_INTF_VID    VTSS_VID_NULL

#define PING_SESSION_CTRL_C_WAIT    500 /* 500 msecs */
#define PING_SESSION_ID_IGNORE      IP_IO_SESSION_ID_IGNORE

/* Initialize module */
vtss_rc ping_init(vtss_init_data_t *data);

/* Main ping entry point - CLI version */
BOOL ping_test(vtss_ip2_cli_pr *io, const char *ip_address, size_t len, size_t count, size_t interval);

#ifdef VTSS_SW_OPTION_IPV6
/* Main ping6 entry point - CLI version */
BOOL ping6_test(vtss_ip2_cli_pr *pr, const vtss_ipv6_t *ipv6_address, size_t len, size_t count, size_t interval, vtss_vid_t vid);
#endif /* VTSS_SW_OPTION_IPV6 */

#ifdef VTSS_SW_OPTION_WEB
/* Main ping test - Web version */
/* Returns NULL on failure */
cli_iolayer_t *ping_test_async(const char *ip_address, size_t len, size_t count, size_t interval);

#ifdef VTSS_SW_OPTION_IPV6
/* Main ping6 test - Web version
   return TRUE: success, FALSE: fail */
cli_iolayer_t *ping6_test_async(const vtss_ipv6_t *ipv6_address, size_t len, size_t count, size_t interval, vtss_vid_t vid);
#endif /* VTSS_SW_OPTION_IPV6 */
#endif /* VTSS_SW_OPTION_WEB */

/* Main ping test - SNMP version */
BOOL ping_test_trap_server_exist(const char *ip_address);

#endif /* _VTSS_PING_API_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
