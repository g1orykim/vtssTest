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
/**
 * \file
 * \brief This file contains MAD structure definitions and MAD functions to manipulate MAD database.
 */
#ifndef _VTSS_XXRP_MAD_H_
#define _VTSS_XXRP_MAD_H_

#include "../include/vtss_xxrp_types.h"
#include "l2proto_api.h" /* For L2_MAX_PORTS */
#include "../include/vtss_xxrp_api.h"

/**
 * \brief function to create MAD structure of a port.
 *
 * \parm map_ports    [IN]  Application-specific MAD ports
 * \param port_no     [IN]  Port number. 
 *
 * \return Return error code.
 **/
u32 vtss_mrp_mad_create_port(vtss_mrp_t *application, vtss_mrp_mad_t **mad_ports, u32 port_no);

/**
 * \brief function to destroy(free) MAD structure of a port.
 *
 * \parm map_ports    [IN]  Application-specific MAD ports
 * \param port_no     [IN]  Port number. 
 *
 * \return Return error code.
 **/
u32 vtss_mrp_mad_destroy_port(vtss_mrp_t *application, vtss_mrp_mad_t **mad_ports, u32 port_no);

/**
 * \brief function to find MAD structure of a port.
 *
 * \parm map_ports    [IN]  Application-specific MAD ports
 * \param port_no     [IN]  Port number. 
 * \param mad         [OUT] pointer to a pointer of MAD structure of a port. 
 *
 * \return Return error code.
 **/
u32 vtss_mrp_mad_find_port(vtss_mrp_mad_t **mad_ports, u32 port_no, vtss_mrp_mad_t **mad);

#endif /* _VTSS_XXRP_MAD_H_ */
