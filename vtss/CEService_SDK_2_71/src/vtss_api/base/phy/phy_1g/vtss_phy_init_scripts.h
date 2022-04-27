/*

 Vitesse API software.

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

#ifndef _VTSS_PHY_INIT_SCRIPTS_H_
#define _VTSS_PHY_INIT_SCRIPTS_H_


#include <vtss_types.h>

vtss_rc vtss_phy_post_reset_private(struct vtss_state_s *vtss_state, const vtss_port_no_t port_no);



vtss_rc vtss_phy_init_seq_atom(struct vtss_state_s *vtss_state,
                               vtss_phy_port_state_t *ps,
                               vtss_port_no_t        port_no,
                               BOOL                  luton26_mode);

vtss_rc vtss_phy_pre_reset_private(struct vtss_state_s *vtss_state, const vtss_port_no_t port_no);
vtss_rc vtss_phy_wait_for_micro_complete(struct vtss_state_s *vtss_state, vtss_port_no_t port_no);

vtss_rc vtss_phy_is_8051_crc_ok_private (struct vtss_state_s *vtss_state,
                                         vtss_port_no_t port_no,
                                         u16 start_addr,
                                         u16 code_length,
                                         u16 expected_crc);

const char *vtss_phy_family2txt(vtss_phy_family_t family);

vtss_rc vtss_phy_init_seq_blazer(struct vtss_state_s *vtss_state,
                                 vtss_phy_port_state_t *ps,
                                 vtss_port_no_t        port_no);

// The address at where the first byte of the internal 8051 firmware is placed.
#define FIRMWARE_START_ADDR 0x4000

#endif /* _VTSS_PHY_INIT_SCRIPTS_H_ */

