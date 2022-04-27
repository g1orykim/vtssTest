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
#include "vtss_macsec_emu_base.h"
#include "vtss_phy_api.h" 
#include "vtss_phy_10g_api.h" 
int main()
{
    // Create a PHY instance
    vtss_inst_t inst = instance_phy_new();

    
    //
    // Capture encrypted frame
    //
    vtss_port_no_t port_no = 0; // Internal port which we are going to test.

    // Setup capturing of egress frame
    vtss_macsec_frame_capture_set(inst, port_no, VTSS_MACSEC_FRAME_CAPTURE_EGRESS); 
    
    // *** Start Your frame transmission - and wait until it is transmitted ****

    // Stop capturing of egress frame
    vtss_macsec_frame_capture_set(inst, port_no, VTSS_MACSEC_FRAME_CAPTURE_DISABLE); 

    // Get the captured frame
    u32 frm_len;                                        // Length of the frame captured
    u8 frm_buffer[VTSS_MACSEC_FRAME_CAPTURE_SIZE_MAX]; // Buffer for storing the frame 
    vtss_macsec_frame_get(inst, port_no, VTSS_MACSEC_FRAME_CAPTURE_SIZE_MAX, &frm_len, &frm_buffer[0]);
    
    // *** Do your checks *** 

    
    //
    // Capture decrypted frame
    //
    
    // Determine PHY type
    vtss_phy_type_t phy_1g_id;
    BOOL is_phy_1g  = (vtss_phy_id_get(inst, port_no, &phy_1g_id) == VTSS_RC_OK)  && (phy_1g_id.part_number != VTSS_PHY_TYPE_NONE);

    vtss_phy_10g_id_t phy_10g_id;
    BOOL is_phy_10g = (vtss_phy_10g_id_get(inst, port_no, &phy_10g_id) == VTSS_RC_OK) && (phy_10g_id.part_number != VTSS_PHY_TYPE_10G_NONE);

    
    // Setup loopback 
    vtss_phy_loopback_t lb_1g;
    vtss_phy_10g_loopback_t lb_10g;
    if (is_phy_1g) {
        lb_1g.far_end_enable  = TRUE;  // Enable loop back at the CU side
        lb_1g.near_end_enable = FALSE; // don't enable loop back at teh MAC side.
        vtss_phy_loopback_set(inst, port_no, lb_1g);
    } else if (is_phy_10g) {
        lb_10g.enable  = TRUE;
        lb_10g.lb_type = VTSS_LB_SYSTEM_XS_SHALLOW;
        vtss_phy_10g_loopback_set(inst, port_no, &lb_10g);
    } else {
        T_E("Unknown PHY type");
        return 1;
    }

    // Setup capturing of ingress frame
    vtss_macsec_frame_capture_set(inst, port_no, VTSS_MACSEC_FRAME_CAPTURE_INGRESS); 
    
    // *** Start Your frame transmission - and wait until it is transmitted ****

    // Stop capturing of frame
    vtss_macsec_frame_capture_set(inst, port_no, VTSS_MACSEC_FRAME_CAPTURE_DISABLE); 

    // Get the captured frame
    vtss_macsec_frame_get(inst, port_no, VTSS_MACSEC_FRAME_CAPTURE_SIZE_MAX, &frm_len, &frm_buffer[0]);
    
    // *** Do your checks *** 
    
    //
    // Remove the loopbacks  and Delete the PHY instance.
    //
    
    if (is_phy_1g) {        
        lb_1g.far_end_enable  = FALSE;  
        vtss_phy_loopback_set(inst, port_no, lb_1g);
    } else if (is_phy_10g) {
        lb_10g.enable  = FALSE;
        vtss_phy_10g_loopback_set(inst, port_no, &lb_10g);
    }
    
    instance_phy_delete(inst);
    
    return 0;
}

