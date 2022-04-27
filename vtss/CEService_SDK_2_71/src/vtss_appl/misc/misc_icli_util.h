/*

 Vitesse Switch API software.

 Copyright (c) 2002-2014 Vitesse Semiconductor Corporation "Vitesse". All
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

#ifndef _MISC_ICLI_UTIL_H_
#define _MISC_ICLI_UTIL_H_

#define MISC_CHIP_NO_NONE (VTSS_CHIP_NO_ALL - 1)

/* ICLI request structure */
typedef struct {
    u32                     session_id;
    icli_stack_port_range_t *port_list;
    vtss_chip_no_t          chip_no;
    vtss_debug_info_t       debug_info;
    BOOL                    resume;
    BOOL                    dummy; /* Unused, for Lint */
} misc_icli_req_t;

void misc_icli_chip(misc_icli_req_t *req);
void misc_icli_debug_api(misc_icli_req_t *req);
void misc_icli_suspend_resume(misc_icli_req_t *req);

void misc_icli_10g_kr_conf(u32 session_id, BOOL has_cm1, i32 cm_1, BOOL has_c0, i32 c_0, BOOL has_cp1, i32 c_1, 
                           BOOL has_ampl, u32 amp_val, BOOL has_ps25, BOOL has_ps35, BOOL has_ps55, BOOL has_ps70,
                           BOOL has_ps120, BOOL has_en_ob, BOOL has_dis_ob, BOOL has_ser_inv, BOOL has_ser_no_inv,
                           icli_stack_port_range_t *v_port_type_list);

#ifdef VTSS_CHIP_10G_PHY
void misc_icli_10g_phy_loopback(u32 session_id, BOOL has_a, BOOL has_b, BOOL has_c, BOOL has_d, BOOL has_e, BOOL has_f, 
                                BOOL has_g, BOOL has_h, BOOL has_j, BOOL has_k, BOOL has_h2, BOOL has_h3, BOOL has_h4, 
                                BOOL has_h5, BOOL has_h6, BOOL has_l0, BOOL has_l1, BOOL has_l2, BOOL has_l3, 
                                BOOL has_enable, BOOL has_disable, 
                                icli_stack_port_range_t *v_port_type_list);

#if defined(VTSS_FEATURE_SYNCE_10G) 
void misc_icli_10g_phy_rxckout(u32 session_id, BOOL has_disable, BOOL has_rx_clock, BOOL has_tx_clock, 
                               BOOL has_pcs_fault_squelch, BOOL has_no_pcs_fault_squelch, 
                               BOOL has_lopc_squelch, BOOL has_no_lopc_squelch, 
                               icli_stack_port_range_t *v_port_type_list);

void misc_icli_10g_phy_txckout(u32 session_id, BOOL has_disable, BOOL has_rx_clock, BOOL has_tx_clock, 
                               icli_stack_port_range_t *v_port_type_list);
#endif // VTSS_FEATURE_SYNCE_10G

#endif //VTSS_CHIP_10G_PHY


#endif /* _MISC_ICLI_UTIL_H_ */
