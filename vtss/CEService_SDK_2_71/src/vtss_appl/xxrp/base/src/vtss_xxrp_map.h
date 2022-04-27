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
#ifndef _VTSS_XXRP_MAP_H_
#define _VTSS_XXRP_MAP_H_

#include "../include/vtss_xxrp_types.h"
#include "../include/vtss_xxrp_api.h"

u32 vtss_mrp_map_create_port(vtss_map_port_t **map_ports, u32 port_no);
u32 vtss_mrp_map_destroy_port(vtss_map_port_t **map_ports, u32 port_no);
void vtss_mrp_map_print_msti_ports(vtss_map_port_t **map_ports, u8 msti);

/**
 * \brief function to create MAP structure of a port.
 *
 * \parm map_ports    [IN]  Application-specific MAP ports
 * \param msti        [IN]  MSTI instance.
 * \param port_no     [IN]  Port number. 
 *  
 * \return Return error code.                                        
 **/
u32 vtss_mrp_map_connect_port(vtss_map_port_t **map_ports, u8 msti, u32 port_no);

/**
 * \brief function to destroy(free) MAP structure of a port.
 *
 * \parm map_ports    [IN]  Application-specific MAP ports
 * \param msti        [IN]  MSTI instance.
 * \param port_no     [IN]  Port number.                            
 *  
 * \return Return error code.
 **/
u32 vtss_mrp_map_disconnect_port(vtss_map_port_t **map_ports, u8 msti, u32 port_no);

/** 
 * \brief function to find MAP structure of a port.
 *  
 * \parm map_ports    [IN]  Application-specific MAP ports
 * \param msti        [IN]  MSTI instance.
 * \param port_no     [IN]  Port number. 
 * \param map         [OUT] pointer to a pointer of MAP structure of a port.     
 *  
 * \return Return error code.
 **/
u32 vtss_mrp_map_find_port(vtss_map_port_t **map_ports, u8 msti, u32 port_no, vtss_map_port_t **map);

u32 vtss_mrp_map_propagate_join(vtss_mrp_appl_t application, vtss_map_port_t **map_ports, u32 port_no, u32 mad_indx);
u32 vtss_mrp_map_propagate_leave(vtss_mrp_appl_t application, vtss_map_port_t **map_ports, u32 port_no, u32 mad_indx);
#endif /* _VTSS_XXRP_MAP_H_ */
