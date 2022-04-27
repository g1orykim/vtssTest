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
#include "vtss_macsec_test_base.h"
#include "vtss_macsec_emu_base.h"

// byte order is handled by the API
#define ETHERTYPE_IEEE_802_1_X 0x888E

/*
 * Test scenario:
 *
 * This is an example on how to configure the MACsec port for handling two
 * MACsec peers connected through a non-MACSec aware switch. The two peers
 * resides on two different VLANS, in two different CA's
 *
 * This test case covers the configuration of the MACsec port, while we pretend
 * that a two MACsec peer is connected as illustrated below:
 *
 *    +------------+  Port VLAN==1000  +-------------+
 *    |MACsec-Peer1|<----------------->|             |
 *    +------------+       (2)         |             | Trunk port +-----------+
 *                                     | VLAN-SWITCH |<---------->|MACsec-port|
 *    +------------+  Port VLAN==2000  |             |     (1)    +-----------+
 *    |MACsec-Peer2|<----------------->|             |
 *    +------------+       (2)         +-------------+
 *
 * In this network, the VLAN-SWITCH is not MACsec aware. It will only accept
 * VLAN tagged frames on the trunk port, on the access frame VLAN tagged traffic
 * is not allowed. When traffic is forwarded from the trunk port to the access
 * port the VLAN tag is popped, when traffic is forwarded from the access port
 * to the trunk port a VLAN frame is pushed.
 *
 * Following is what the frames will look like at point (1) and (2):
 *
 *       +----------------------------+
 *   (1) |DA|SA|SECtag|Payload|ICV|CRC|
 *       +----------------------------+
 *
 *       +---------------------------------+
 *   (2) |DA|SA|VLAN|SECtag|Payload|ICV|CRC|
 *       +---------------------------------+
 *
 * - Two CA's are created, one comprising MACsec-peer1 and the MACsec-port,
 *   another one comprising MACsec-peer2 and the MACsec-port.
 * - 802.1X frames are associated with the uncontrolled port
 * - All other traffic is treated as MACsec traffic
 * - Traffic which is not recognized as MACsec traffic or 802.1X traffic is
 *   dropped.
 * */

// Global variables. WARN: this should not be globals in more advanced
// applications.
vtss_inst_t inst;
const vtss_port_no_t macsec_physical_port = 0;
const vtss_mac_t port_macaddress = { .addr = {0, 0, 0, 0, 0, 3} };

static void secy_create(vtss_vid_t vid)
{
    const vtss_macsec_port_t macsec_port = {
        .port_no    = macsec_physical_port,
        .service_id = vid,
    };

    // Setup the physical port to use tag bypass. This is a global setting which
    // only can be controlled per physical port.
    const vtss_macsec_bypass_mode_t bypass_mode = {
        .mode = VTSS_MACSEC_BYPASS_TAG,
    };
    VTSS_RC_TEST(vtss_macsec_bypass_mode_set(inst, macsec_physical_port,
                                             &bypass_mode));

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
        .confidentiality_offset = 0,
        .mac_addr               = port_macaddress
    };
    VTSS_RC_TEST(vtss_macsec_secy_conf_add(inst, macsec_port,
                                           &macsec_port_conf));

    // Configure VLAN tag skipping
    VTSS_RC_TEST(vtss_macsec_bypass_tag_set(inst, macsec_port,
                                            VTSS_MACSEC_BYPASS_TAG_ONE));

    // Only match traffic on the configured VLAN. On the configured VLAN, match
    // everything, but use a lower priority than the control frame matching
    // pattern.
    vtss_macsec_match_pattern_t pattern_ctrl = {
        .match     = VTSS_MACSEC_MATCH_VLAN_ID,
        .vid       = vid,
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

    // All traffic on the configured VLAN which have ether-type 0x888E
    // must be associated with the controlled port.
    vtss_macsec_match_pattern_t pattern_unctrl = {
        .match              = VTSS_MACSEC_MATCH_ETYPE          |
                              VTSS_MACSEC_MATCH_VLAN_ID        |
                              VTSS_MACSEC_MATCH_HAS_VLAN_INNER,
        .vid                = vid,
        .has_vlan_inner_tag = 0,
        .etype              = ETHERTYPE_IEEE_802_1_X,
        .priority           = VTSS_MACSEC_MATCH_PRIORITY_HIGH
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

static void secy_delete(vtss_vid_t vid)
{
    const vtss_macsec_port_t macsec_port = {
        .port_no    = macsec_physical_port,
        .service_id = vid,
    };

    VTSS_RC_TEST(vtss_macsec_secy_conf_del(inst, macsec_port));
}

static void tx_create(vtss_vid_t vid)
{
    // define the MACsec port
    const vtss_macsec_port_t macsec_port = {
        .port_no    = macsec_physical_port,
        .service_id = vid,
    };

    // Add a new TX secure channel
    VTSS_RC_TEST(vtss_macsec_tx_sc_set(inst, macsec_port));

    // create a zero-key for 128bit cipher suite
    vtss_macsec_sak_t sak_tx_sa_0 = { .len = 16 };

    // Update the hash key in the SAK before the key is installed in HW
    VTSS_RC_TEST(sak_update_hash_key(&sak_tx_sa_0));

    // install the key in HW on the egress side
    VTSS_RC_TEST(vtss_macsec_tx_sa_set(inst, macsec_port,
                                       0,    // associations number
                                       1024, // next_pn,
                                       TRUE, // confidentiality,
                                       &sak_tx_sa_0));
}

static void tx_delete(vtss_vid_t vid)
{
    // define the MACsec port
    const vtss_macsec_port_t macsec_port = {
        .port_no    = macsec_physical_port,
        .service_id = vid,
    };

    VTSS_RC_TEST(vtss_macsec_tx_sa_disable(inst, macsec_port, 0));
    VTSS_RC_TEST(vtss_macsec_tx_sa_del(inst, macsec_port, 0));
    /* VTSS_RC_TEST(vtss_macsec_tx_sa_disable(inst, macsec_port, 1)); */
    /* VTSS_RC_TEST(vtss_macsec_tx_sa_del(inst, macsec_port, 1)); */
    VTSS_RC_TEST(vtss_macsec_tx_sc_del(inst, macsec_port));
}

static void rx_create(vtss_vid_t vid, const vtss_mac_t *const mac)
{
    // Add a new RX secure channel
    const vtss_macsec_port_t macsec_port = {
        .port_no    = macsec_physical_port,
        .service_id = vid,
    };

    vtss_macsec_sci_t sci_rx = { .mac_addr = *mac };
    VTSS_RC_TEST(vtss_macsec_rx_sc_add(inst, macsec_port, &sci_rx));

    // Add two RX SA's
    vtss_macsec_sak_t sak_rx_sa_0 = { .len = 16 };
    vtss_macsec_sak_t sak_rx_sa_1 = { .len = 16 };

    // Update the hash key in the SAK before the key is installed in HW
    VTSS_RC_TEST(sak_update_hash_key(&sak_rx_sa_0));
    VTSS_RC_TEST(sak_update_hash_key(&sak_rx_sa_1));

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

static void rx_delete(vtss_vid_t vid, const vtss_mac_t *const mac)
{
    // Add a new RX secure channel
    const vtss_macsec_port_t macsec_port = {
        .port_no    = macsec_physical_port,
        .service_id = vid,
    };
    vtss_macsec_sci_t sci_rx = { .mac_addr = *mac };
    VTSS_RC_TEST(vtss_macsec_tx_sa_disable(inst, macsec_port, 0));
//    VTSS_RC_TEST(vtss_macsec_tx_sa_disable(inst, macsec_port, 1));
    VTSS_RC_TEST(vtss_macsec_rx_sa_del(inst, macsec_port, &sci_rx, 0));
    VTSS_RC_TEST(vtss_macsec_rx_sa_del(inst, macsec_port, &sci_rx, 1));
    VTSS_RC_TEST(vtss_macsec_rx_sc_del(inst, macsec_port, &sci_rx));
}

int main()
{
    // Create a PHY instance
    vtss_inst_t inst = instance_phy_new();

    // A set of constants used through the program
    const u32 peer1_vlan = 1000;
    const u32 peer2_vlan = 2000;
    const vtss_mac_t peer1_macaddress = { .addr = {0, 0, 0, 0, 0, 1}};
    const vtss_mac_t peer2_macaddress = { .addr = {0, 0, 0, 0, 0, 2}};

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

    // Create SecY's
    secy_create(peer1_vlan);
    secy_create(peer2_vlan);

    // Add secure TX channels and a secure association
    tx_create(peer1_vlan);
    tx_create(peer2_vlan);

    // Create peers
    rx_create(peer1_vlan, &peer1_macaddress);
    rx_create(peer2_vlan, &peer2_macaddress);

    // TODO, inject traffic here, and verify traffic and counters

    // clean up
    rx_delete(peer2_vlan, &peer2_macaddress);
    rx_delete(peer1_vlan, &peer1_macaddress);
    tx_delete(peer2_vlan);
    tx_delete(peer1_vlan);
    secy_delete(peer2_vlan);
    secy_delete(peer1_vlan);

    // Delete the PHY instance
    instance_phy_delete(inst);

    return 0;
}

