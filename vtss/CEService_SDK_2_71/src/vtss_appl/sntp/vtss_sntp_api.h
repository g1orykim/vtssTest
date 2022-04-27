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

#ifndef _IP2_SNTP_API_H_
#define _IP2_SNTP_API_H_

/* sntp managent enabled/disabled */
#define SNTP_MGMT_ENABLED       1
#define SNTP_MGMT_DISABLED      0

#define SNTP_MAX_SERVER_COUNT   1
#define SNTP_ADDRSTRLEN         46

/* the type of ip */
typedef enum {
    SNTP_IP_TYPE_IPV4,
    SNTP_IP_TYPE_IPV6
} sntp_ip_type_t;

/* sntp configuration */
typedef struct {
    BOOL                mode;
    uchar               sntp_server[VTSS_SYS_HOSTNAME_LEN];
    sntp_ip_type_t      ip_type;
#ifdef VTSS_SW_OPTION_IPV6
    vtss_ipv6_t         ipv6_addr;
#endif
} sntp_conf_t;


/* Get sntp configuration */
vtss_rc sntp_config_get(sntp_conf_t *conf);

/* Set sntp configuration */
vtss_rc sntp_config_set(sntp_conf_t *conf);

/* Set sntp defaults */
void vtss_sntp_default_set(sntp_conf_t *conf);

/* Module initialization */
vtss_rc vtss_sntp_init(vtss_init_data_t *data);


#endif /* _IP2_API_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/

