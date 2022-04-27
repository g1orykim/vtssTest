/*

 Vitesse Switch API software.

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
//----------------------------------------------------------------------------
/**
 *  \file
 *      dhcp_server_icfg.c
 *
 *  \brief
 *      ICFG implementation
 *
 *  \author
 *      CP Wang
 *
 *  \date
 *      05/09/2013 11:50
 */
//----------------------------------------------------------------------------
#ifndef __DHCP_SERVER_ICFG_H__
#define __DHCP_SERVER_ICFG_H__
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

/*
******************************************************************************

    Public Function

******************************************************************************
*/

/**
 * \file dhcp_server_icfg.h
 * \brief This file defines the interface to the DHCP snooping module's ICFG commands.
 */

/**
 * \brief Initialization function.
 *      Call once, preferably from the INIT_CMD_INIT section of
 *      the module's _init() function.
 */
vtss_rc dhcp_server_icfg_init(void);

#endif /* __DHCP_SERVER_ICFG_H__ */
