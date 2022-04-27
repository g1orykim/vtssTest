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
#include <unistd.h>
#include "vtss_macsec_test_base.h"


/*
   To allow SA rollover without interrupting traffic, multiple RX-SA's must be
   installed in the PHY currently (within a single SC).

   When the transmitter decides to move-on to the next AN, all the peers have
   already the key for the up coming SA installed.

   On the TX side change of SA is an atomic action, which will be applied when
   calling the vtss_macsec_tx_sa_activate function.

   On the RX side, the SA matching the AN will be used, if it is available. If
   it is not available, the packet is dropped.

   Below is illustrated two MACsec peers which performs SA rollover.

   In the example below is an actually implementation of PEER-A.


ID  PEER-A                   Dependency-graph    PEER-B                    ID
-----------------------------------------------------------------------------
A1  rx_sa_set(an == 0,                           tx_sa_set(an == 0,        B1
              lowest_pn == 0,                              next_pn == 0,
              KEY1);                                       KEY1);

A2  rx_sa_activate(an == 0); <-------+           rx_sa_set(an == 0,        B2
                                     |                     lowest_pn == 0,
A3  tx_sa_set(an == 0,               |                     KEY2);
              next_pn == 0,          |
              KEY2);            +--- | --------> rx_sa_activate(an == 0);  B3
                                |    |
A4  tx_sa_activate(an == 0); ---+    +---------- tx_sa_activate(an == 0);  B4

    //  time elapeses... in our keysec reference system ~1 hour elapes
    //  at this point

A5  rx_sa_set(an == 1,                           tx_sa_set(an == 1,        B5
              lowest_pn == 0,                              next_pn == 0,
              KEY3);                                       KEY3);

A6  rx_sa_activate(an == 1); <-------+           rx_sa_set(an == 1,        B6
                                     |                     lowest_pn == 0,
A7  tx_sa_set(an == 1,               |                     KEY4);
              next_pn == 0,          |
              KEY4);            +--- | --------> rx_sa_activate(an == 1);  B7
                                |    |
A8  tx_sa_activate(an == 1); ---+    +---------- tx_sa_activate(an == 1);  B8

A9  tx_sa_del(an == 0);                          tx_sa_del(an == 0);       B9

    // poll until true                           // poll until true
A10 rx_sa_not_in_use(an == 1);                   rx_sa_not_in_use(an == 1);B10

A11 rx_sa_del(an == 0);                          rx_sa_del(an == 0);       B11


// Complete make-style dependency graph
A2: A1
A4: A2, B3
A6: A5
A8: A7, B7
A9: A8
A11: A10
B3: B2
B4: B1, A2
B7: B6
B8: B5, A6
B9: B8
B11: B10

// Function mapping
rx_sa_set        -> vtss_macsec_rx_sa_set
rx_sa_activate   -> vtss_macsec_rx_sa_activate
tx_sa_set        -> vtss_macsec_tx_sa_set
tx_sa_activate   -> vtss_macsec_tx_sa_activate
rx_sa_not_in_use -> vtss_macsec_rx_sa_status_get
rx_sa_del        -> vtss_macsec_rx_sa_del
tx_sa_del        -> vtss_macsec_tx_sa_del

*/

int main()
{
    const vtss_port_no_t macsec_physical_port = 0;
    vtss_macsec_port_t macsec_port;
    vtss_mac_t         port_macaddress;
    const vtss_mac_t peer_macaddress = { .addr = {0, 0, 0, 0, 0, 2}};

    // Prepare some test keys to use in this example
    vtss_macsec_sak_t KEY1 = { .len = 16 };
    VTSS_RC_TEST(sak_update_hash_key(&KEY1));
    vtss_macsec_sak_t KEY2 = { .len = 16 };
    VTSS_RC_TEST(sak_update_hash_key(&KEY2));
    vtss_macsec_sak_t KEY3 = { .len = 16 };
    VTSS_RC_TEST(sak_update_hash_key(&KEY3));
    vtss_macsec_sak_t KEY4 = { .len = 16 };
    VTSS_RC_TEST(sak_update_hash_key(&KEY4));

    // Create a PHY instance
    vtss_inst_t inst = instance_phy_new();

    // Create a simple SecY
    vtss_basic_secy(inst, macsec_physical_port, &macsec_port, &port_macaddress);

    // Add a new MACsec peer
    vtss_macsec_sci_t peer_sci = { .mac_addr = peer_macaddress,
                                   .port_id  = macsec_port.port_id};
    VTSS_RC_TEST(vtss_macsec_rx_sc_add(inst, macsec_port, &peer_sci));

    // Create a new RX-SA, AN==0
    VTSS_RC_TEST(vtss_macsec_rx_sa_set(inst, macsec_port, &peer_sci, 0, 0, &KEY1));
    VTSS_RC_TEST(vtss_macsec_rx_sa_activate(inst, macsec_port, &peer_sci, 0));

    // Create a new TX-SC, AN==0
    VTSS_RC_TEST(vtss_macsec_tx_sc_set(inst, macsec_port));

    // Create a new TX-SA, AN==0
    VTSS_RC_TEST(vtss_macsec_tx_sa_set(inst, macsec_port, 0, 0, TRUE, &KEY2));

    // TODO, make sure that all peers within this CA have installed KEY2 as a RX
    // SA on AN==0

    // Activate the TX SA, AN==0
    VTSS_RC_TEST(vtss_macsec_tx_sa_activate(inst, macsec_port, 0));

    // TODO, Use the secure connectivity which is now available

    // Some time has elapsed, and we should now prepare for SA rollover

    // Create a new RX-SA, AN==1
    VTSS_RC_TEST(vtss_macsec_rx_sa_set(inst, macsec_port, &peer_sci, 1, 0, &KEY3));
    VTSS_RC_TEST(vtss_macsec_rx_sa_activate(inst, macsec_port, &peer_sci, 1));

    // Create a new TX-SA, AN==1
    VTSS_RC_TEST(vtss_macsec_tx_sa_set(inst, macsec_port, 1, 0, TRUE, &KEY4));

    // Activate the TX SA, AN==1
    VTSS_RC_TEST(vtss_macsec_tx_sa_activate(inst, macsec_port, 1));

    // Disable the sa AN==0
    VTSS_RC_TEST(vtss_macsec_tx_sa_disable(inst, macsec_port, 0));

    // Delete the old TX-SA, AN==0
    VTSS_RC_TEST(vtss_macsec_tx_sa_del(inst, macsec_port, 0));

    // TODO, Wait for the old RX-SA to become non-used before deleting it
    VTSS_RC_TEST(vtss_macsec_rx_sa_del(inst, macsec_port, &peer_sci, 0));

    // TODO, continue to use the SC running on a new SA

    // Clean up
    VTSS_RC_TEST(vtss_macsec_rx_sa_del(inst, macsec_port, &peer_sci, 1));
    VTSS_RC_TEST(vtss_macsec_rx_sc_del(inst, macsec_port, &peer_sci));
    VTSS_RC_TEST(vtss_macsec_tx_sa_disable(inst, macsec_port, 1));
    VTSS_RC_TEST(vtss_macsec_tx_sa_del(inst, macsec_port, 1));
    VTSS_RC_TEST(vtss_macsec_tx_sc_del(inst, macsec_port));

    // Destroy the SecY
    vtss_basic_secy_destroy(inst, &macsec_port);

    // Delete the PHY instance
    instance_phy_delete(inst);

    return 0;
}

