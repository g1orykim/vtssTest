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

#ifndef __VTSS_MACSEC_TEST_BASE_H__
#define __VTSS_MACSEC_TEST_BASE_H__

#include "vtss_api.h"
#include "vtss_macsec_api.h"
#include "vtss_macsec_test_base.h"
#include "vtss_macsec_emu_base.h"

void vtss_basic_secy(vtss_inst_t         inst,
                     vtss_port_no_t      macsec_physical_port,
                     vtss_macsec_port_t *_macsec_port,
                     vtss_mac_t         *_port_macaddress
);

void vtss_basic_secy_destroy(
    vtss_inst_t               inst,
    const vtss_macsec_port_t *macsec_port
);

void vtss_single_secy_sample_system_create(
    vtss_inst_t         inst,
    vtss_macsec_port_t *_macsec_port,
    vtss_mac_t         *_port_macaddress,
    vtss_macsec_sci_t  *_peer_sci
);

void vtss_single_secy_sample_system_destroy(
    vtss_inst_t               inst,
    const vtss_macsec_port_t *macsec_port,
    const vtss_mac_t         *port_macaddress,
    const vtss_macsec_sci_t  *peer_sci
);

vtss_rc sak_update_hash_key(vtss_macsec_sak_t * sak);

#endif
