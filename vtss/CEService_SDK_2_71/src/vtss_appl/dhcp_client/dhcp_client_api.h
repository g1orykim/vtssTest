/*

 Vitesse software.

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
#ifndef __vtss_dhcp_client_API_H__
#define __vtss_dhcp_client_API_H__

#include "vtss_types.h"

#define VTSS_DHCP_MAX_OFFERS                     3
#define VTSS_DHCP_OPTION_CODE_TIME_SERVER        4
#define VTSS_DHCP_OPTION_CODE_DOMAIN_NAME_SERVER 6

vtss_rc vtss_dhcp_client_init(vtss_init_data_t *data);
const char * dhcp_client_error_txt(vtss_rc rc);

typedef enum {
    DHCP4C_STATE_STOPPED,
    DHCP4C_STATE_INIT,
    DHCP4C_STATE_SELECTING,
    DHCP4C_STATE_REQUESTING,
    DHCP4C_STATE_REBINDING,
    DHCP4C_STATE_BOUND,
    DHCP4C_STATE_RENEWING,
    DHCP4C_STATE_FALLBACK
} vtss_dhcp4c_state_t;

const char * vtss_dhcp4c_state_to_txt(vtss_dhcp4c_state_t s);

typedef struct {
    vtss_ipv4_network_t ip;

    vtss_mac_t  server_mac;
    vtss_ipv4_t server_ip;
    BOOL        has_server_ip;

    vtss_ipv4_t default_gateway;
    BOOL        has_default_gateway;

    vtss_ipv4_t domain_name_server;
    BOOL        has_domain_name_server;
} vtss_dhcp_fields_t;

typedef struct {
    unsigned valid_offers;
    vtss_dhcp_fields_t list[VTSS_DHCP_MAX_OFFERS];
} vtss_dhcp_client_offer_list_t;

typedef struct {
    vtss_dhcp4c_state_t state;
    vtss_ipv4_t server_ip;
    vtss_dhcp_client_offer_list_t offers;
} vtss_dhcp_client_status_t;

int vtss_dhcp4c_status_to_txt(char                            *buf,
                              int                              size,
                              const vtss_dhcp_client_status_t *const st);

/* CONTROL AND STATE ------------------------------------------------- */

/* Start the DHCP client on the given VLAN */
vtss_rc vtss_dhcp_client_start(       vtss_vid_t                  vlan);

/* Stop the DHCP client on the given VLAN */
vtss_rc vtss_dhcp_client_stop(        vtss_vid_t                  vlan);

/* Set the DHCP client in fallback mode */
vtss_rc vtss_dhcp_client_fallback(    vtss_vid_t                  vlan);

/* Kill the DHCP client on the given VLAN */
vtss_rc vtss_dhcp_client_kill(        vtss_vid_t                  vlan);

/* Check if the DHCP client is bound */
BOOL vtss_dhcp_client_bound_get(      vtss_vid_t                  vlan);

/* Inspect the list of received offers */
vtss_rc vtss_dhcp_client_offers_get(  vtss_vid_t                  vlan,
                                      vtss_dhcp_client_offer_list_t *list);

/* Accept one of the received offers */
vtss_rc vtss_dhcp_client_offer_accept(vtss_vid_t                  vlan,
                                      unsigned idx);

/* Get status */
vtss_rc vtss_dhcp_client_status(      vtss_vid_t                  vlan,
                                      vtss_dhcp_client_status_t  *status);

typedef void (*vtss_dhcp_client_callback_t)(vtss_vid_t);
vtss_rc vtss_dhcp_client_callback_add(vtss_vid_t                  vlan,
                                      vtss_dhcp_client_callback_t cb);
vtss_rc vtss_dhcp_client_callback_del(vtss_vid_t                  vlan,
                                      vtss_dhcp_client_callback_t cb);

vtss_rc vtss_dhcp_client_fields_get(  vtss_vid_t                  vlan,
                                      vtss_dhcp_fields_t         *fields);

vtss_rc vtss_dhcp_client_dns_option_ip_any_get(vtss_ipv4_t  prefered,
                                               vtss_ipv4_t *ip);

#if defined(VTSS_SW_OPTION_DHCP_HELPER)
/* Receive/Transmit the DHCP packet via DHCP Helper APIs */
BOOL vtss_dhcp_client_packet_handler(const u8 *const frm,
                                     size_t length,
                                     vtss_vid_t vid,
                                     vtss_isid_t src_isid,
                                     vtss_port_no_t src_port_no,
                                     vtss_glag_no_t src_glag_no);
#else
BOOL vtss_dhcp_client_packet_handler(void *contxt, const u8 *const frm,
                                     const vtss_packet_rx_info_t *const rx_info);
#endif /* VTSS_SW_OPTION_DHCP_HELPER */

vtss_rc vtss_dhcp_client_release(vtss_vid_t vlan);
vtss_rc vtss_dhcp_client_decline(vtss_vid_t vlan);

#endif /* __vtss_dhcp_client_API_H__ */
