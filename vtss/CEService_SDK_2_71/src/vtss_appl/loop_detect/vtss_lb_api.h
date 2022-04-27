/*

 Vitesse Switch API software.

 Copyright (c) 2002-2011 Vitesse Semiconductor Corporation "Vitesse". All
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



// Callback type
typedef void (*vtss_lb_callback_t)(void);

// Callback function for registering for "magic packet"
// Do not attempt to call any other functions from this module
// when inside the callback function.
void vtss_lb_callback_register(vtss_lb_callback_t cb, int vtss_type) ;

// Function for unregistering the callback function.
void vtss_lb_callback_unregister(int vtss_type);

/* Return whether port is (actively) being monitored by this module */
BOOL vtss_lb_port(vtss_port_no_t port_no);

// Function for transmitting a OSSP frame with user defined vtsss_type.
void vtss_lb_tx_ossp_pck(int vtss_type);

/* Initialize module */
vtss_rc vtss_lb_init(vtss_init_data_t *data);



/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
