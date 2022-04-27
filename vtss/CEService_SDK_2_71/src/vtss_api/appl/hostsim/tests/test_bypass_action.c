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
 * This test case explorer the usage of bypassing in default rules, patterns
 * matching the un-controlled port.
 *
 * The test is configured to match all 802.1X frames as control frames, and
 * by-pass them from the MACsec processing. Beside this, frames with ether-type
 * 0x8809 is match to the un-controlled port of the configured SecY.
 *
 * The result is that frames of both types will be passed through without being
 * processed by the MACsec block.
 * */

// The real content of this example are to be found in base_single_secy.c

int main()
{
    // Create a PHY instance
    vtss_inst_t inst = instance_phy_new();

    const vtss_port_no_t           macsec_physical_port = 0;
    const vtss_macsec_vport_id_t   macsec_virtual_port  = 0;
    const vtss_macsec_service_id_t macsec_service_id    = 0;
    const vtss_mac_t port_macaddress = { .addr = {0, 0, 0, 0, 0, 1}};

    // Build the macsec_port struct
    const vtss_macsec_port_t       macsec_port = {
        .port_no    = macsec_physical_port,
        .service_id = macsec_service_id,
        .port_id    = macsec_virtual_port
    };


    { // Initialize the physical PHY to process MACsec traffic
        // Initialize the MACsec block
        vtss_macsec_init_t init_data = { .enable = TRUE };
        VTSS_RC_TEST(vtss_macsec_init_set(inst, macsec_physical_port,
                                          &init_data));

        // Configure which frames should be considered as control frames
        vtss_macsec_control_frame_match_conf_t ctrl_match = {
            .match = VTSS_MACSEC_MATCH_ETYPE,
            .etype = ETHERTYPE_IEEE_802_1_X
        };
        VTSS_RC_TEST(vtss_macsec_control_frame_match_conf_set(inst,
                                                              macsec_physical_port,
                                                              &ctrl_match,
                                                              NULL));

        // Setup default rules to by-pass traffic classified as control frames
        vtss_macsec_default_action_policy_t default_action_policy = {
            .ingress_non_control_and_non_macsec = VTSS_MACSEC_DEFAULT_ACTION_DROP,
            .ingress_control_and_non_macsec     = VTSS_MACSEC_DEFAULT_ACTION_BYPASS,
            .ingress_non_control_and_macsec     = VTSS_MACSEC_DEFAULT_ACTION_DROP,
            .ingress_control_and_macsec         = VTSS_MACSEC_DEFAULT_ACTION_DROP,
            .egress_control                     = VTSS_MACSEC_DEFAULT_ACTION_BYPASS,
            .egress_non_control                 = VTSS_MACSEC_DEFAULT_ACTION_DROP
        };
        VTSS_RC_TEST(vtss_macsec_default_action_set(inst, macsec_physical_port,
                                                    &default_action_policy));
    }

    { // Setup the physical port to use tag bypass.
        // This is a global setting which only can be controlled per physical
        // port.
        const vtss_macsec_bypass_mode_t bypass_mode = {
            .mode = VTSS_MACSEC_BYPASS_TAG,
        };
        VTSS_RC_TEST(vtss_macsec_bypass_mode_set(inst, macsec_physical_port,
                                                 &bypass_mode));
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
            .confidentiality_offset = 0,
            .mac_addr               = port_macaddress
        };
        VTSS_RC_TEST(vtss_macsec_secy_conf_add(inst, macsec_port,
                                               &macsec_port_conf));

        // The control pattern matches everything which is not matched by others
        vtss_macsec_match_pattern_t pattern_ctrl = {
            .priority  = VTSS_MACSEC_MATCH_PRIORITY_LOW
        };
        VTSS_RC_TEST(vtss_macsec_pattern_set(inst, macsec_port,
                                             VTSS_MACSEC_DIRECTION_INGRESS,
                                             VTSS_MACSEC_MATCH_ACTION_CONTROLLED_PORT,
                                             &pattern_ctrl));
        VTSS_RC_TEST(vtss_macsec_pattern_set(inst, macsec_port,
                                             VTSS_MACSEC_DIRECTION_EGRESS,
                                             VTSS_MACSEC_MATCH_ACTION_CONTROLLED_PORT,
                                             &pattern_ctrl));

        // Frames with ethertype 0x8809 must go on the un-controlled port
        vtss_macsec_match_pattern_t pattern_unctrl = {
            .priority = VTSS_MACSEC_MATCH_PRIORITY_HIGHEST,
            .match    = VTSS_MACSEC_MATCH_ETYPE,
            .etype    = 0x8809
        };
        VTSS_RC_TEST(vtss_macsec_pattern_set(inst, macsec_port,
                                             VTSS_MACSEC_DIRECTION_INGRESS,
                                             VTSS_MACSEC_MATCH_ACTION_UNCONTROLLED_PORT,
                                             &pattern_unctrl));
        VTSS_RC_TEST(vtss_macsec_pattern_set(inst, macsec_port,
                                             VTSS_MACSEC_DIRECTION_EGRESS,
                                             VTSS_MACSEC_MATCH_ACTION_UNCONTROLLED_PORT,
                                             &pattern_unctrl));

        // Enable the SecY
        VTSS_RC_TEST(vtss_macsec_secy_controlled_set(inst, macsec_port, TRUE));
    }


    // TODO, test that traffic with ET==8809 is passed through, while other
    // traffic will be matched by the default rules, and dropped.

    // Delete SecY
    VTSS_RC_TEST(vtss_macsec_secy_conf_del(inst, macsec_port));

    // Disable the MACsec block
    vtss_macsec_init_t deinit_data = { .enable = FALSE };
    VTSS_RC_TEST(vtss_macsec_init_set(inst, macsec_physical_port,
                                      &deinit_data));

    // Delete the PHY instance
    instance_phy_delete(inst);

    return 0;
}

