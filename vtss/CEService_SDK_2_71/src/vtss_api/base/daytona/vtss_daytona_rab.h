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
 $Revisions: $

*/

/**
 * \file vtss_daytona_rab.h
 * \brief  API
 */

#ifndef _VTSS_DAYTONA_RAB_H_
#define _VTSS_DAYTONA_RAB_H_

#if defined(VTSS_ARCH_DAYTONA)

/*
 * \brief Set Rab default configuration
 *
 * \param port_no [IN]   Port number
 * \return Return code. 
 */
vtss_rc vtss_daytona_rab_default_conf_set(vtss_state_t *vtss_state, vtss_port_no_t port_no);

/**
 * \brief Create instance (set up function pointers).
 *
 * \return Return code.
 **/
vtss_rc vtss_daytona_inst_rab_create(vtss_state_t *vtss_state);

/**
 * \brief Restart_conf_set for RAB layer.
 *
 * \param port_no [IN] Port number
 * \return Return code.
 **/
vtss_rc vtss_daytona_rab_restart_conf_set(vtss_state_t *vtss_state, vtss_port_no_t port_no);

#endif /* VTSS_ARCH_DAYTONA */
#endif /* _VTSS_DAYTONA_RAB_H_ */

