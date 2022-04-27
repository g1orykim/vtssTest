/*

 Vitesse Switch API software.

 Copyright (c) 2002-2009 Vitesse Semiconductor Corporation "Vitesse". All
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

#ifndef _DOT1X_ACCT_H_
#define _DOT1X_ACCT_H_

#include "vtss_nas_platform_api.h" /* For nas_port_control_t and nas_eap_info_t */

/****************************************************************************/
// dot1x_acct_authorized_changed()
/****************************************************************************/
void dot1x_acct_authorized_changed(nas_port_control_t admin_state, struct nas_sm *sm, BOOL authorized);

/****************************************************************************/
// dot1x_acct_radius_rx()
/****************************************************************************/
void dot1x_acct_radius_rx(u8 radius_handle, nas_eap_info_t *eap_info);

/****************************************************************************/
// dot1x_acct_append_radius_tlv()
/****************************************************************************/
BOOL dot1x_acct_append_radius_tlv(u8 radius_handle, nas_eap_info_t *eap_info);

/****************************************************************************/
// dot1x_acct_init()
/****************************************************************************/
void dot1x_acct_init(void);

#endif /* _DOT1X_ACCT_H_ */
