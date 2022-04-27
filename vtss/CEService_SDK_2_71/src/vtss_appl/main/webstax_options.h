/*

 Vitesse Switch API software.

 Copyright (c) 2002-2008 Vitesse Semiconductor Corporation "Vitesse". All
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

#ifndef _VTSS_WEBSTAX_OPTIONS_H_
#define _VTSS_WEBSTAX_OPTIONS_H_

/* ================================================================= *
 *  Unmanaged module options
 * ================================================================= */

/* VeriPHY during initialization */
#ifndef VTSS_UNMGD_OPT_VERIPHY
#define VTSS_UNMGD_OPT_VERIPHY    0
#endif /* VTSS_UNMGD_OPT_VERIPHY */

/* Flow control setup via GPIO14 and LED control via GPIO8 */
#ifndef VTSS_UNMGD_OPT_FC_GPIO
#define VTSS_UNMGD_OPT_FC_GPIO    0
#endif /* VTSS_UNMGD_OPT_FC_GPIO */

/* Aggregation setup via GPIO12/GPIO13 and LED control via GPIO4/GPIO5/GPIO6 */
#ifndef VTSS_UNMGD_OPT_AGGR_GPIO
#define VTSS_UNMGD_OPT_AGGR_GPIO  0
#endif /* VTSS_UNMGD_OPT_AGGR_GPIO */

/* HDMI CABLE detect via GPIO2 and GPIO3 */
#ifndef VTSS_UNMGD_OPT_HDMI_GPIO
#define VTSS_UNMGD_OPT_HDMI_GPIO    0
#endif /* VTSS_UNMGD_OPT_FC_GPIO */


/* Serial LED for VeriPHY and port status */
#ifndef VTSS_UNMGD_OPT_SERIAL_LED
#define VTSS_UNMGD_OPT_SERIAL_LED 0
#endif /* VTSS_UNMGD_OPT_SERIAL_LED */

/* Firmware upgrade */
#ifndef VTSS_UNMGD_OPT_SWUP
#define VTSS_UNMGD_OPT_SWUP 0
#endif /* VTSS_UNMGD_OPT_SERIAL_LED */

/* ================================================================= *
 *  User interface module options
 * ================================================================= */

/* VeriPHY supported in user interface */
#ifndef VTSS_UI_OPT_VERIPHY
#define VTSS_UI_OPT_VERIPHY 1
#endif /* VTSS_UI_OPT_VERIPHY */

#endif /* _VTSS_WEBSTAX_OPTIONS_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
