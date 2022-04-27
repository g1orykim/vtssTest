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

#include "main.h"

#include "conf_api.h"
#include "msg_api.h"
#include "standalone_api.h"


vtss_rc topo_isid2mac(const vtss_isid_t isid, mac_addr_t mac_addr)
{
    (void)conf_mgmt_mac_addr_get(mac_addr, 0);
    return VTSS_OK;
}


vtss_rc standalone_init(vtss_init_data_t *data)
{
    if (data->cmd == INIT_CMD_START) {
        msg_topo_event(MSG_TOPO_EVENT_MASTER_UP, VTSS_ISID_START);
        msg_topo_event(MSG_TOPO_EVENT_SWITCH_ADD, VTSS_ISID_START);
    }
    return VTSS_OK;
}






