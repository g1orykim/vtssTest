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

#ifndef _VTSS_TOD_PHY_ENGINE_H_
#define _VTSS_TOD_PHY_ENGINE_H_
#include "vtss_types.h"

#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
#include "vtss_phy_ts_api.h"

/**
 * \brief Initialize the PHY engine allocation table.
 * \return nothing.
 **/
void tod_phy_eng_alloc_init(void);

/**
 * \brief Return ist of allocated engines for a port.
 * \param port_no     [IN]  port number that an engine is allocated for.
 * \param engine_list [OUT] array of 4 booleans indicating if an engine i allocated.
 *
 * \return nothing.
 **/
void tod_phy_eng_alloc_get(vtss_port_no_t port_no, BOOL *engine_list);

/**
 * \brief Allocate a PHY engine for a port.
 * \param port_no    [IN]  port number that an engine is allocated for.
 * \param encap_type [IN]  The encapsulation type, that the engine is allocated for.
 *
 * \return allocated engine ID, if no engine can be allocated, VTSS_PHY_TS_ENGINE_ID_INVALID is returned.
 **/
vtss_phy_ts_engine_t tod_phy_eng_alloc(vtss_port_no_t port_no, vtss_phy_ts_encap_t encap_type);

/**
 * \brief Free a PHY engine for a port.
 * \param port_no    [IN]  port number that an engine is allocated for.
 * \param eng_id     [IN]  The engine id that is freed.
 *
 * \return nothing.
 **/
void tod_phy_eng_free(vtss_port_no_t port_no, vtss_phy_ts_engine_t eng_id);

#endif // VTSS_FEATURE_PHY_TIMESTAMP
#endif // _VTSS_TOD_PHY_ENGINE_H_


// ***************************************************************************
//
//  End of file.
//
// ***************************************************************************
