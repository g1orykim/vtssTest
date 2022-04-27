/*

 Vitesse Switch API software.

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
#ifndef _VTSS_UPNP_API_H_
#define _VTSS_UPNP_API_H_

#include "vtss_module_id.h"
#include "main.h"
//#include "vtss_upnp.h"

#ifndef VTSS_BOOL
#define VTSS_BOOL
#endif

/* UPNP managent enabled/disabled */
#define UPNP_MGMT_ENABLED       1
#define UPNP_MGMT_DISABLED      0
#define UPNP_MGMT_DEFAULT_TTL   4   /* 1..255*/
#define UPNP_MGMT_MAX_TTL       255
#define UPNP_MGMT_MIN_TTL       1
#define UPNP_MGMT_DEFAULT_INT   100 /* 60..86400 seconds */
#define UPNP_MGMT_MIN_INT       60 /* 60..86400 seconds */
#define UPNP_MGMT_MAX_INT       86400 /* 60..86400 seconds */
#define UPNP_MGMT_UDNSTR_SIZE   (41 + 1) /* "UUID="(5char) + uuid(16bytes/32char) + 4 *'-' + 1 * '\0' */
#define UPNP_MGMT_IPSTR_SIZE    (15 + 1) /* xxx.xxx.xxx.xxx + 1 * '\0' */
#define UPNP_MGMT_DESC_SIZE     80
#define UPNP_MGMT_XML_PORT      80
#define UPNP_MGMT_DESC_DOC_NAME  "xml/devicedesc.xml"
#define UPNP_UDP_PORT           1900

/**
 * \brief API Error Return Codes (vtss_rc)
 */
enum {
    UPNP_ERROR_GEN = MODULE_ERROR_START(VTSS_MODULE_ID_UPNP),   /* Generic error code */
    UPNP_ERROR_PARM,                                            /* Illegal parameter */
    UPNP_ERROR_STACK_STATE,                                     /* Illegal MASTER/SLAVE state */
    UPNP_ERROR_GET_CERT_INFO                                    /* Illegal get certificate information */
};

/* UPNP configuration */
typedef struct {
    unsigned long mode;                                         /* UPNP Mode */
    unsigned char ttl;          /* TTL value in the IP header */
    unsigned long adv_interval; /* SSDP advertisement mesaage interval */
} upnp_conf_t;

/* UPNP error text */
char *upnp_error_txt(vtss_rc rc);

void upnp_default_get(upnp_conf_t *conf);

/* Get UPNP configuration */
vtss_rc upnp_mgmt_conf_get(upnp_conf_t *conf);

/* Set UPNP configuration */
vtss_rc upnp_mgmt_conf_set(upnp_conf_t *conf);

/* Get UPNP xml file */
void vtss_upnp_xml_get(char **xml);

/* Get the ip address string */
int vtss_upnp_get_ip(char *ipaddr);

/* Get the UDN string */
void vtss_upnp_get_udnstr(char *buffer);

/* Get one stack memory from the pool */
char *vtss_upnp_get_memstack(int *size);

/* return all stack memory to the pool */
void vtss_upnp_free_all_memstack(void);

/* Initialize module */
vtss_rc upnp_init(vtss_init_data_t *data);

/* update MAC address table the VLAN ID of IP muticast address 239.255.255.250 */
void upnp_mac_mc_update(vtss_vid_t old_vid, vtss_vid_t new_vid);
#endif /* _VTSS_UPNP_API_H_ */


// ***************************************************************************
//
//  End of file.
//
// ***************************************************************************
