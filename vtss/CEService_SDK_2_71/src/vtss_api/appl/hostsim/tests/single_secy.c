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
#include "vtss_api.h"
#include "vtss_macsec_api.h"
#include "base.h"

// byte order is handled by the API
#define ETHERTYPE_IEEE_802_1_X 0x888E

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

static void sak_update_hash_key(vtss_macsec_sak_t * sak)
{
    // This algorithm must be provided by the target environment
}

int main()
{
    // Create a PHY instance
    vtss_inst_t inst = instance_phy_new();

    // A set of constants used through the program
    const vtss_port_no_t           macsec_physical_port = 0;
    const vtss_macsec_vport_id_t   macsec_virtual_port  = 0;
    const vtss_macsec_service_id_t macsec_service_id    = 0;
    const vtss_macsec_port_t       macsec_port = {
        .port_no    = macsec_physical_port,
        .service_id = macsec_service_id,
        .port_id    = macsec_virtual_port
    };
    const vtss_mac_t port_macaddress = { .addr = {0, 0, 0, 0, 0, 1}};
    const vtss_mac_t peer_macaddress = { .addr = {0, 0, 0, 0, 0, 2}};

    { // Initialize the physical PHY to process MACsec traffic
        // Initialize the MACsec block
        vtss_macsec_init_t init_data = { .enable = TRUE };
        VTSS_RC_TEST(vtss_macsec_init_set(inst, macsec_physical_port,
                                          &init_data));

        // Use the default rules to drop all non-matched traffic
        vtss_macsec_default_action_policy_t default_action_policy;
        memset(&default_action_policy, 0, sizeof default_action_policy);
        VTSS_RC_TEST(vtss_macsec_default_action_set(inst, macsec_physical_port,
                                                    &default_action_policy));
    }

    { // create a new MACsec SecY
        // Start by adding a SecY
        vtss_macsec_secy_conf_t macsec_port_conf = {
            .validate_frames        = VTSS_MACSEC_VALIDATE_FRAMES_STRICT,
            .replay_protect         = TRUE,
            .replay_window          = 0,
            .protect_frames         = TRUE,
            .always_include_sci     = FALSE,
            .use_es                 = TRUE,
            .use_scb                = FALSE,
            .current_cipher_suite   = VTSS_MACSEC_CIPHER_SUITE_GCM_AES_128,
            .confidentiality_offset = VTSS_MACSEC_CONFIDENTIALITY_NOT_USED,
            .mac_addr               = port_macaddress
        };
        VTSS_RC_TEST(vtss_macsec_secy_conf_add(inst, macsec_port,
                                               &macsec_port_conf));

        // No encapsulation is used. Just match everything, but keep the
        // priority lower than the rule matching traffic for the uncontrolled
        // port. In other words, if it is not control traffic, then it should
        // be considered MACsec traffic.
        vtss_macsec_match_pattern_t pattern_ctrl = {
            .priority  = VTSS_MACSEC_MATCH_PRIORITY_LOW
        };

        // Associate the pattern with the controlled MACsec port
        VTSS_RC_TEST(vtss_macsec_pattern_set(inst, macsec_port,
                                             VTSS_MACSEC_DIRECTION_INGRESS,
                                             VTSS_MACSEC_MATCH_ACTION_CONTROLLED_PORT,
                                             &pattern_ctrl));
        VTSS_RC_TEST(vtss_macsec_pattern_set(inst, macsec_port,
                                             VTSS_MACSEC_DIRECTION_EGRESS,
                                             VTSS_MACSEC_MATCH_ACTION_CONTROLLED_PORT,
                                             &pattern_ctrl));

        // All traffic which is not vlan traffic, and have ether-type 0x888E
        // must be associated with the controlled port.
        vtss_macsec_match_pattern_t pattern_unctrl = {
            .match        = VTSS_MACSEC_MATCH_HAS_VLAN |
                            VTSS_MACSEC_MATCH_ETYPE,
            .has_vlan_tag = 0,
            .etype        = ETHERTYPE_IEEE_802_1_X,
            .priority     = VTSS_MACSEC_MATCH_PRIORITY_HIGH
        };

        // Associate the pattern with the uncontrolled MACsec port
        VTSS_RC_TEST(vtss_macsec_pattern_set(inst, macsec_port,
                                             VTSS_MACSEC_DIRECTION_INGRESS,
                                             VTSS_MACSEC_MATCH_ACTION_UNCONTROLLED_PORT,
                                             &pattern_unctrl));
        VTSS_RC_TEST(vtss_macsec_pattern_set(inst, macsec_port,
                                             VTSS_MACSEC_DIRECTION_EGRESS,
                                             VTSS_MACSEC_MATCH_ACTION_UNCONTROLLED_PORT,
                                             &pattern_unctrl));
    }

    { // Add secure TX channels and a secure association
        // Add a new TX secure channel
        VTSS_RC_TEST(vtss_macsec_tx_sc_set(inst, macsec_port));

        // create a zero-key for 128bit cipher suite
        vtss_macsec_sak_t sak_tx_sa_0 = { .len = 16 };

        // Update the hash key in the SAK before the key is installed in HW
        sak_update_hash_key(&sak_tx_sa_0);

        // install the key in HW on the egress side
        VTSS_RC_TEST(vtss_macsec_tx_sa_set(inst, macsec_port,
                                           0,    // associations number
                                           1024, // next_pn,
                                           TRUE, // confidentiality,
                                           &sak_tx_sa_0));
    }

    { // Add a MACsec peer
        // Add a new RX secure channel
        vtss_macsec_sci_t sci_rx = { .mac_addr = peer_macaddress,
                                     .port_id  = macsec_virtual_port};
        VTSS_RC_TEST(vtss_macsec_rx_sc_add(inst, macsec_port, &sci_rx));

        // Add two RX SA's
        vtss_macsec_sak_t sak_rx_sa_0 = { .len = 16 };
        vtss_macsec_sak_t sak_rx_sa_1 = { .len = 16 };

        // Update the hash key in the SAK before the key is installed in HW
        sak_update_hash_key(&sak_rx_sa_0);
        sak_update_hash_key(&sak_rx_sa_1);

        VTSS_RC_TEST(vtss_macsec_rx_sa_set(inst, macsec_port,
                                           &sci_rx, // identify which SC the SA belongs to
                                           0,       // associations number
                                           0,       // lowest_pn,
                                           &sak_rx_sa_0));

        VTSS_RC_TEST(vtss_macsec_rx_sa_set(inst, macsec_port,
                                           &sci_rx, // identify which SC the SA belongs to
                                           1,       // associations number
                                           1023,    // lowest_pn,
                                           &sak_rx_sa_1));
    }

    // TODO, inject traffic here, and verify traffic and counters

    { // clean up
        vtss_macsec_sci_t sci_rx = { .mac_addr = peer_macaddress,
                                     .port_id  = macsec_virtual_port};

        // Delete peer
        VTSS_RC_TEST(vtss_macsec_rx_sa_del(inst, macsec_port, &sci_rx, 1));
        VTSS_RC_TEST(vtss_macsec_rx_sa_del(inst, macsec_port, &sci_rx, 0));
        VTSS_RC_TEST(vtss_macsec_rx_sc_del(inst, macsec_port, &sci_rx));

        // Delete TX SA and SC
        VTSS_RC_TEST(vtss_macsec_tx_sa_del(inst, macsec_port, 0));
        VTSS_RC_TEST(vtss_macsec_tx_sc_del(inst, macsec_port));

        // Delete SecY
        VTSS_RC_TEST(vtss_macsec_secy_conf_del(inst, macsec_port));

        // Disable the MACsec block
        vtss_macsec_init_t deinit_data = { .enable = FALSE };
        VTSS_RC_TEST(vtss_macsec_init_set(inst, macsec_physical_port,
                                          &deinit_data));
    }

    // Delete the PHY instance
    instance_phy_delete(inst);

    return 0;
}

