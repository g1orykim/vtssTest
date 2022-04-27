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

#ifndef _VTSS_STANDALONE_API_H_
#define _VTSS_STANDALONE_API_H_

vtss_rc topo_isid2mac(const vtss_isid_t isid, mac_addr_t mac_addr);


static __inline__ vtss_port_no_t topo_mac2port(const mac_addr_t mac_addr) __attribute__ ((const));
static __inline__ vtss_port_no_t topo_mac2port(const mac_addr_t mac_addr)
{
    return 0;
}

static __inline__ vtss_isid_t topo_usid2isid(const vtss_usid_t usid) __attribute__ ((const));
static __inline__ vtss_isid_t topo_usid2isid(const vtss_usid_t usid)
{
    return (vtss_isid_t) usid;
}

static __inline__ vtss_usid_t topo_isid2usid(const vtss_isid_t isid) __attribute__ ((const));
static __inline__ vtss_usid_t topo_isid2usid(const vtss_isid_t isid)
{
    return (vtss_usid_t) isid;
}


vtss_rc standalone_init(vtss_init_data_t *data);

#endif 






