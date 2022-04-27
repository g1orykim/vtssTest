/*

 Vitesse Switch Software.

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
#include "vtss_macsec_test_base.h"

/*
 * Test scenario:
 *
 * This is the simplest possible test scenario where a MACsec port is
 * communicating with a single MACsec peer.
 *
 * This test case covers the configuration of the MACsec port, while we pretend
 * that a MACsec peer is connected to the MACsec port.
 *
 *    +-----------+     +-----------+
 *    |MACsec-Peer|<--->|MACsec-port|
 *    +-----------+     +-----------+
 *
 * - One CA is created comprising the MACsec port, and the MACsec peer
 * - 802.1X frames are associated with the uncontrolled port
 * - All other traffic is treated as MACsec traffic
 * - Traffic which is not recognized as MACsec traffic or 802.1X traffic is
 *   dropped.
 * */

// The real content of this example are to be found in base_single_secy.c

int main()
{
    // Create a PHY instance
    vtss_inst_t inst = instance_phy_new();

    vtss_macsec_port_t macsec_port;
    vtss_mac_t         port_macaddress;
    vtss_macsec_sci_t  peer_sci;

    vtss_single_secy_sample_system_create(inst,
                                          &macsec_port,
                                          &port_macaddress,
                                          &peer_sci);

    // TODO, inject traffic here, and verify traffic and counters

    vtss_single_secy_sample_system_destroy(inst,
                                           &macsec_port,
                                           &port_macaddress,
                                           &peer_sci);

    // Delete the PHY instance
    instance_phy_delete(inst);

    return 0;
}

