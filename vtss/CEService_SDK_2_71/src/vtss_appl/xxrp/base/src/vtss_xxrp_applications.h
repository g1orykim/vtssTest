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
#ifndef _VTSS_XXRP_APPPLICATIONS_H_
#define _VTSS_XXRP_APPLICATIONS_H_

#include "../include/vtss_xxrp_types.h"
#include "../include/vtss_xxrp_api.h"

/**
 * \brief function to create MRP application.
 * This function will be called on global enable of a MRP application.
 *
 * \param application   [IN]  pointer to MRP application (vtss_mrp_t).
 *
 * \return Return error code.
 **/
u32 mrp_appl_add_to_database(vtss_mrp_t *apps[], vtss_mrp_t *application);

/**
 * \brief function to delete MRP application.
 * This function will be called on global disable of a MRP application.
 *
 * \param appl_type    [IN]  MRP application type.
 *
 * \return Return error code.
 **/
u32  mrp_appl_del_from_database(vtss_mrp_t *apps[], vtss_mrp_appl_t appl_type);

#endif /* _VTSS_XXRP_APPLICATIONS_H_ */
