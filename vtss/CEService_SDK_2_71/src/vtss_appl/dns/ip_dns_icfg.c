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

#include "icfg_api.h"
#include "misc_api.h"
#include "ip_dns_api.h"
#include "ip_dns_icfg.h"


/*
******************************************************************************

    Constant and Macro definition

******************************************************************************
*/
#define VTSS_TRACE_MODULE_ID        VTSS_MODULE_ID_IP_DNS
#define IP_DNS_ICFG_REG(x, y, z, w) (((x) = vtss_icfg_query_register((y), (z), (w))) == VTSS_OK)

/*
******************************************************************************

    Data structure type definition

******************************************************************************
*/

/*
******************************************************************************

    Static Function

******************************************************************************
*/
/* ICFG callback functions */
static vtss_rc _ip_dns_icfg_state_ctrl(const vtss_icfg_query_request_t *req,
                                       vtss_icfg_query_result_t *result)
{
    vtss_rc             rc = VTSS_OK;
    BOOL                dns_proxy;
    vtss_dns_srv_conf_t dns_srv;

    if (req && result) {
        /*
            COMMAND = ip name-server { <ipv4_ucast> | dhcp [ interface vlan <vlan_id> ] }
            COMMAND = ip dns proxy
        */
        if (vtss_dns_mgmt_get_server(DNS_DEF_SRV_IDX, &dns_srv) == VTSS_OK) {
            BOOL    pr_cfg;

            pr_cfg = FALSE;
            if (VTSS_DNS_TYPE_DEFAULT(&dns_srv)) {
                if (req->all_defaults) {
                    pr_cfg = TRUE;
                }
            } else {
                pr_cfg = TRUE;
            }

            switch ( VTSS_DNS_TYPE_GET(&dns_srv) ) {
            case VTSS_DNS_SRV_TYPE_DHCP_ANY:
                if (pr_cfg) {
                    rc = vtss_icfg_printf(result, "ip name-server dhcp\n");
                }

                break;
            case VTSS_DNS_SRV_TYPE_STATIC:
                if (pr_cfg) {
                    char    buf[40];

                    rc = vtss_icfg_printf(result, "ip name-server %s\n",
                                          misc_ipv4_txt(VTSS_DNS_ADDR_IPA4_GET(&dns_srv), buf));
                }

                break;
            case VTSS_DNS_SRV_TYPE_DHCP_VLAN:
                if (pr_cfg) {
                    rc = vtss_icfg_printf(result, "ip name-server dhcp interface vlan %u\n",
                                          VTSS_DNS_VLAN_GET(&dns_srv));
                }

                break;
            case VTSS_DNS_SRV_TYPE_NONE:
            default:
                if (pr_cfg) {
                    rc = vtss_icfg_printf(result, "no ip name-server\n");
                }

                break;
            }
        } else {
            if (req->all_defaults) {
                rc = vtss_icfg_printf(result, "no ip name-server\n");
            }
        }

        if (ip_dns_mgmt_get_proxy_status(&dns_proxy) == VTSS_OK) {
            if (req->all_defaults ||
                (dns_proxy != VTSS_DNS_PROXY_DEF_STATE)) {
                rc = vtss_icfg_printf(result, "%sip dns proxy\n",
                                      dns_proxy ? "" : "no ");
            }
        } else {
            if (req->all_defaults) {
                rc = vtss_icfg_printf(result, "no ip dns proxy\n");
            }
        }
    }

    return rc;
}

/*
******************************************************************************

    Public functions

******************************************************************************
*/
/* Initialization function */
vtss_rc ip_dns_icfg_init(void)
{
    vtss_rc rc;

    /* Register callback functions to ICFG module. */
    if (IP_DNS_ICFG_REG(rc, VTSS_ICFG_IP_DNS_CONF, "dns", _ip_dns_icfg_state_ctrl)) {
        T_I("ip_dns ICFG done");
    }

    return rc;
}
