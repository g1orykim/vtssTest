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


#ifndef _VTSS_SWITCH_PORT_H
#define _VTSS_SWITCH_PORT_H

#include <linux/ioctl.h>
#include <linux/types.h>

#include "vtss_api.h"
#include "port_custom_api.h"

/* Port config */
typedef port_custom_conf_t port_conf_t;

struct _port_conf_ioc {
    vtss_port_no_t port_no;
    port_conf_t conf;
};

struct _port_cap_ioc {
    vtss_port_no_t port_no;
    port_cap_t     cap;
};

#define POIOC_vtss_port_conf_set                 _IOR ('P',  1, struct _port_conf_ioc)
#define POIOC_vtss_port_conf_get                 _IOWR('P',  2, struct _port_conf_ioc)
#define POIOC_vtss_port_cap_get                  _IOWR('P',  3, struct _port_cap_ioc)

#endif				/* ifndef _VTSS_SWITCH_PORT_H */
