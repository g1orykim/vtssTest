/*

 Vitesse Switch API software.

 Copyright (c) 2002-2012 Vitesse Semiconductor Corporation "Vitesse". All
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

#ifndef _VTSS_HTTPS_ICFG_H_
#define _VTSS_HTTPS_ICFG_H_

/*
******************************************************************************

    Include files

******************************************************************************
*/

/*
******************************************************************************

    Constant and Macro definition

******************************************************************************
*/
#define VTSS_HTTPS_NO_FORM_TEXT                 "no "
#define VTSS_HTTPS_GLOBAL_MODE_ENABLE_TEXT      "ip http secure-server"
#define VTSS_HTTPS_REDIRECT_MODE_ENABLE_TEXT    "ip http secure-redirect"

/*
******************************************************************************

    Data structure type definition

******************************************************************************
*/

/**
 * \file vtss_https_icfg_init.h
 * \brief This file defines the interface to the HTTPS module's ICFG commands.
 */

/**
  * \brief Initialization function.
  *
  * Call once, preferably from the INIT_CMD_INIT section of
  * the module's _init() function.
  */
vtss_rc vtss_https_icfg_init(void);

#endif /* _VTSS_HTTPS_ICFG_H_ */