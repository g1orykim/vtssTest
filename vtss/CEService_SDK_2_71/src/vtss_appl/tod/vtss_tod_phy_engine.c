/*

 Vitesse Switch Software.

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


#include "main.h"
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)

#include "vtss_tod_phy_engine.h"
#include "tod.h"
#include "vtss_tod_mod_man.h"
#include "port_api.h"
#include "critd_api.h"

/**
 * \brief PHY timestamp configuration for each PTP instance
 */
static BOOL tod_phy_eng_alloc_table [VTSS_PORTS] [VTSS_PHY_TS_ENGINE_ID_INVALID];

static const vtss_phy_ts_engine_t eng_priority_table [2] [VTSS_PHY_TS_ENCAP_NONE] [VTSS_PHY_TS_ENGINE_ID_INVALID] =
    /* gen 1 */
    {
    {[VTSS_PHY_TS_ENCAP_ETH_PTP]            = {VTSS_PHY_TS_PTP_ENGINE_ID_0,  VTSS_PHY_TS_PTP_ENGINE_ID_1,  VTSS_PHY_TS_ENGINE_ID_INVALID, VTSS_PHY_TS_ENGINE_ID_INVALID},
    [VTSS_PHY_TS_ENCAP_ETH_IP_PTP]          = {VTSS_PHY_TS_PTP_ENGINE_ID_0,  VTSS_PHY_TS_PTP_ENGINE_ID_1,  VTSS_PHY_TS_ENGINE_ID_INVALID, VTSS_PHY_TS_ENGINE_ID_INVALID},
    [VTSS_PHY_TS_ENCAP_ETH_IP_IP_PTP]       = {VTSS_PHY_TS_PTP_ENGINE_ID_0,  VTSS_PHY_TS_PTP_ENGINE_ID_1,  VTSS_PHY_TS_ENGINE_ID_INVALID, VTSS_PHY_TS_ENGINE_ID_INVALID},
    [VTSS_PHY_TS_ENCAP_ETH_ETH_PTP]         = {VTSS_PHY_TS_PTP_ENGINE_ID_0,  VTSS_PHY_TS_PTP_ENGINE_ID_1,  VTSS_PHY_TS_ENGINE_ID_INVALID, VTSS_PHY_TS_ENGINE_ID_INVALID},
    [VTSS_PHY_TS_ENCAP_ETH_ETH_IP_PTP]      = {VTSS_PHY_TS_PTP_ENGINE_ID_0,  VTSS_PHY_TS_PTP_ENGINE_ID_1,  VTSS_PHY_TS_ENGINE_ID_INVALID, VTSS_PHY_TS_ENGINE_ID_INVALID},
    [VTSS_PHY_TS_ENCAP_ETH_MPLS_IP_PTP]     = {VTSS_PHY_TS_PTP_ENGINE_ID_0,  VTSS_PHY_TS_PTP_ENGINE_ID_1,  VTSS_PHY_TS_ENGINE_ID_INVALID, VTSS_PHY_TS_ENGINE_ID_INVALID},
    [VTSS_PHY_TS_ENCAP_ETH_MPLS_ETH_PTP]    = {VTSS_PHY_TS_PTP_ENGINE_ID_0,  VTSS_PHY_TS_PTP_ENGINE_ID_1,  VTSS_PHY_TS_ENGINE_ID_INVALID, VTSS_PHY_TS_ENGINE_ID_INVALID},
    [VTSS_PHY_TS_ENCAP_ETH_MPLS_ETH_IP_PTP] = {VTSS_PHY_TS_PTP_ENGINE_ID_0,  VTSS_PHY_TS_PTP_ENGINE_ID_1,  VTSS_PHY_TS_ENGINE_ID_INVALID, VTSS_PHY_TS_ENGINE_ID_INVALID},
    [VTSS_PHY_TS_ENCAP_ETH_MPLS_ACH_PTP]    = {VTSS_PHY_TS_PTP_ENGINE_ID_0,  VTSS_PHY_TS_PTP_ENGINE_ID_1,  VTSS_PHY_TS_ENGINE_ID_INVALID, VTSS_PHY_TS_ENGINE_ID_INVALID},
    /* OAM encap */
    [VTSS_PHY_TS_ENCAP_ETH_OAM]             = {VTSS_PHY_TS_OAM_ENGINE_ID_2A, VTSS_PHY_TS_OAM_ENGINE_ID_2B, VTSS_PHY_TS_ENGINE_ID_INVALID, VTSS_PHY_TS_ENGINE_ID_INVALID},
    [VTSS_PHY_TS_ENCAP_ETH_ETH_OAM]         = {VTSS_PHY_TS_OAM_ENGINE_ID_2A, VTSS_PHY_TS_ENGINE_ID_INVALID,VTSS_PHY_TS_ENGINE_ID_INVALID, VTSS_PHY_TS_ENGINE_ID_INVALID},
    [VTSS_PHY_TS_ENCAP_ETH_MPLS_ETH_OAM]    = {VTSS_PHY_TS_OAM_ENGINE_ID_2A, VTSS_PHY_TS_ENGINE_ID_INVALID,VTSS_PHY_TS_ENGINE_ID_INVALID, VTSS_PHY_TS_ENGINE_ID_INVALID},
    [VTSS_PHY_TS_ENCAP_ETH_MPLS_ACH_OAM]    = {VTSS_PHY_TS_OAM_ENGINE_ID_2A, VTSS_PHY_TS_OAM_ENGINE_ID_2B, VTSS_PHY_TS_ENGINE_ID_INVALID, VTSS_PHY_TS_ENGINE_ID_INVALID}}
    ,
    /* gen 2 */
    {[VTSS_PHY_TS_ENCAP_ETH_PTP]            = {VTSS_PHY_TS_PTP_ENGINE_ID_0,  VTSS_PHY_TS_PTP_ENGINE_ID_1,  VTSS_PHY_TS_OAM_ENGINE_ID_2A,  VTSS_PHY_TS_OAM_ENGINE_ID_2B},
    [VTSS_PHY_TS_ENCAP_ETH_IP_PTP]          = {VTSS_PHY_TS_PTP_ENGINE_ID_0,  VTSS_PHY_TS_PTP_ENGINE_ID_1,  VTSS_PHY_TS_ENGINE_ID_INVALID, VTSS_PHY_TS_ENGINE_ID_INVALID},
    [VTSS_PHY_TS_ENCAP_ETH_IP_IP_PTP]       = {VTSS_PHY_TS_PTP_ENGINE_ID_0,  VTSS_PHY_TS_PTP_ENGINE_ID_1,  VTSS_PHY_TS_ENGINE_ID_INVALID, VTSS_PHY_TS_ENGINE_ID_INVALID},
    [VTSS_PHY_TS_ENCAP_ETH_ETH_PTP]         = {VTSS_PHY_TS_PTP_ENGINE_ID_0,  VTSS_PHY_TS_PTP_ENGINE_ID_1,  VTSS_PHY_TS_OAM_ENGINE_ID_2A,  VTSS_PHY_TS_ENGINE_ID_INVALID},
    [VTSS_PHY_TS_ENCAP_ETH_ETH_IP_PTP]      = {VTSS_PHY_TS_PTP_ENGINE_ID_0,  VTSS_PHY_TS_PTP_ENGINE_ID_1,  VTSS_PHY_TS_ENGINE_ID_INVALID, VTSS_PHY_TS_ENGINE_ID_INVALID},
    [VTSS_PHY_TS_ENCAP_ETH_MPLS_IP_PTP]     = {VTSS_PHY_TS_PTP_ENGINE_ID_0,  VTSS_PHY_TS_PTP_ENGINE_ID_1,  VTSS_PHY_TS_ENGINE_ID_INVALID, VTSS_PHY_TS_ENGINE_ID_INVALID},
    [VTSS_PHY_TS_ENCAP_ETH_MPLS_ETH_PTP]    = {VTSS_PHY_TS_PTP_ENGINE_ID_0,  VTSS_PHY_TS_PTP_ENGINE_ID_1,  VTSS_PHY_TS_OAM_ENGINE_ID_2A,  VTSS_PHY_TS_ENGINE_ID_INVALID},
    [VTSS_PHY_TS_ENCAP_ETH_MPLS_ETH_IP_PTP] = {VTSS_PHY_TS_PTP_ENGINE_ID_0,  VTSS_PHY_TS_PTP_ENGINE_ID_1,  VTSS_PHY_TS_ENGINE_ID_INVALID, VTSS_PHY_TS_ENGINE_ID_INVALID},
    [VTSS_PHY_TS_ENCAP_ETH_MPLS_ACH_PTP]    = {VTSS_PHY_TS_PTP_ENGINE_ID_0,  VTSS_PHY_TS_PTP_ENGINE_ID_1,  VTSS_PHY_TS_OAM_ENGINE_ID_2A,  VTSS_PHY_TS_ENGINE_ID_INVALID},
    /* OAM encap */
    [VTSS_PHY_TS_ENCAP_ETH_OAM]             = {VTSS_PHY_TS_OAM_ENGINE_ID_2A, VTSS_PHY_TS_OAM_ENGINE_ID_2B, VTSS_PHY_TS_PTP_ENGINE_ID_0,   VTSS_PHY_TS_PTP_ENGINE_ID_1},
    [VTSS_PHY_TS_ENCAP_ETH_ETH_OAM]         = {VTSS_PHY_TS_OAM_ENGINE_ID_2A, VTSS_PHY_TS_PTP_ENGINE_ID_0,  VTSS_PHY_TS_PTP_ENGINE_ID_1,   VTSS_PHY_TS_ENGINE_ID_INVALID},
    [VTSS_PHY_TS_ENCAP_ETH_MPLS_ETH_OAM]    = {VTSS_PHY_TS_OAM_ENGINE_ID_2A, VTSS_PHY_TS_PTP_ENGINE_ID_0,  VTSS_PHY_TS_PTP_ENGINE_ID_1,   VTSS_PHY_TS_ENGINE_ID_INVALID},
    [VTSS_PHY_TS_ENCAP_ETH_MPLS_ACH_OAM]    = {VTSS_PHY_TS_OAM_ENGINE_ID_2A, VTSS_PHY_TS_PTP_ENGINE_ID_0,  VTSS_PHY_TS_PTP_ENGINE_ID_1,   VTSS_PHY_TS_ENGINE_ID_INVALID}}
};

#define TOD_PHY_ENG_LOCK()        critd_enter(&datamutex, VTSS_TRACE_GRP_DEFAULT, LOCK_TRACE_LEVEL, __FILE__, __LINE__)
#define TOD_PHY_ENG_UNLOCK()      critd_exit (&datamutex, VTSS_TRACE_GRP_DEFAULT, LOCK_TRACE_LEVEL, __FILE__, __LINE__)
static critd_t datamutex;          /* Global data protection */

/**
 * \brief Initialize the PHY engine allocation table.
 * \return nothing.
 **/
void tod_phy_eng_alloc_init(void)
{
    vtss_port_no_t j;
    int i;
    port_iter_t       pit;
    port_iter_init_local(&pit);
    while (port_iter_getnext(&pit)) {
        j = pit.iport;
        for (i = 0; i < VTSS_PHY_TS_ENGINE_ID_INVALID; i++) {
            tod_phy_eng_alloc_table[j][i] = FALSE;
        }
    }
    critd_init(&datamutex, "phy_eng", VTSS_MODULE_ID_TOD, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
    TOD_PHY_ENG_UNLOCK();
}

/**
 * \brief Return ist of allocated engines for a port.
 * \param port_no     [IN]  port number that an engine is allocated for.
 * \param engine_list [OUT] array of 4 booleans indicating if an engine i allocated.
 *
 * \return nothing.
 **/
void tod_phy_eng_alloc_get(vtss_port_no_t port_no, BOOL *engine_list)
{
    int i;
    TOD_PHY_ENG_LOCK();
    for (i = 0; i < VTSS_PHY_TS_ENGINE_ID_INVALID; i++) {
        engine_list[i] = tod_phy_eng_alloc_table[port_no][i];
    }
    TOD_PHY_ENG_UNLOCK();
}
/**
 * \brief Allocate a PHY engine for a port.
 * \param port_no    [IN]  port number that an engine is allocated for.
 * \param encap_type [IN]  The encapsulation type, that the engine is allocated for.
 *
 * \return allocated engine ID, if no engine can be allocated, VTSS_PHY_TS_ENGINE_ID_INVALID is returned.
 **/
vtss_phy_ts_engine_t tod_phy_eng_alloc(vtss_port_no_t port_no, vtss_phy_ts_encap_t encap_type)
{
    vtss_phy_ts_engine_t i, alloc_eng;
    vtss_tod_ts_phy_topo_t phy_topo;
    int phy_gen;

    if (port_no >= VTSS_PORTS) {
        T_WG(VTSS_TRACE_GRP_PHY_ENG, "invalid port no %d", port_no);
        return VTSS_PHY_TS_ENGINE_ID_INVALID;
    }
    tod_ts_phy_topo_get(port_no, &phy_topo);
    
    if (phy_topo.ts_gen == VTSS_PTP_TS_GEN_1) {
        phy_gen = 0;
    } else if (phy_topo.ts_gen == VTSS_PTP_TS_GEN_2) {
        phy_gen = 1;
    } else {
        return VTSS_PHY_TS_ENGINE_ID_INVALID;
    }
    if (encap_type >= VTSS_PHY_TS_ENCAP_NONE) {
        T_WG(VTSS_TRACE_GRP_PHY_ENG, "invalid encapsulation type %d", encap_type);
        return VTSS_PHY_TS_ENGINE_ID_INVALID;
    }
    
    TOD_PHY_ENG_LOCK();
    for (i = VTSS_PHY_TS_PTP_ENGINE_ID_0; i < VTSS_PHY_TS_ENGINE_ID_INVALID; i++) {
        alloc_eng = eng_priority_table[phy_gen] [encap_type] [i];
        T_IG(VTSS_TRACE_GRP_PHY_ENG, "phy_gen %d, encap_type %d, index %d, alloc_eng %d", phy_gen, encap_type, i, alloc_eng);
        if (alloc_eng != VTSS_PHY_TS_ENGINE_ID_INVALID) {
            if (!tod_phy_eng_alloc_table[port_no][alloc_eng]) {
                tod_phy_eng_alloc_table[port_no][alloc_eng] = TRUE;
                if (phy_topo.port_shared) {
                    tod_phy_eng_alloc_table[phy_topo.shared_port_no][alloc_eng] = TRUE;
                }
                T_IG(VTSS_TRACE_GRP_PHY_ENG, "allocated engine Id %d, port %d, shared %d, shared_port %d", i, port_no, phy_topo.port_shared, phy_topo.shared_port_no);
                break;
            }
        } else {
            break;
        }
    }
    TOD_PHY_ENG_UNLOCK();
    if (i == VTSS_PHY_TS_ENGINE_ID_INVALID) {
        T_WG(VTSS_TRACE_GRP_PHY_ENG, "could not allocate engine for encapsulation %d, port %d", encap_type, port_no);
        alloc_eng = i;
    }
    return alloc_eng;
}

/**
 * \brief Free a PHY engine for a port.
 * \param port_no    [IN]  port number that an engine is allocated for.
 * \param eng_id     [IN]  The engine id that is freed.
 *
 * \return nothing.
 **/
void tod_phy_eng_free(vtss_port_no_t port_no, vtss_phy_ts_engine_t eng_id)
{
    vtss_tod_ts_phy_topo_t phy_topo;
    
    if (port_no >= VTSS_PORTS) {
        T_WG(VTSS_TRACE_GRP_PHY_ENG, "invalid port no %d", port_no);
        return;
    }
    tod_ts_phy_topo_get(port_no, &phy_topo);
    if (phy_topo.ts_feature != VTSS_PTP_TS_PTS) {
        T_WG(VTSS_TRACE_GRP_PHY_ENG, "port no %d does not support PHY timestamping", port_no);
        return;
    }
    if (eng_id >= VTSS_PHY_TS_ENGINE_ID_INVALID) {
        T_WG(VTSS_TRACE_GRP_PHY_ENG, "invalid engine Id %d", eng_id);
        return;
    }
    TOD_PHY_ENG_LOCK();
    if (tod_phy_eng_alloc_table[port_no][eng_id]) {
        tod_phy_eng_alloc_table[port_no][eng_id] = FALSE;
        if (phy_topo.port_shared) {
            tod_phy_eng_alloc_table[phy_topo.shared_port_no][eng_id] = FALSE;
        }
        T_IG(VTSS_TRACE_GRP_PHY_ENG, "free engine Id %d, port %d, shared %d, shared_port %d", eng_id, port_no, phy_topo.port_shared, phy_topo.shared_port_no);
    }
    TOD_PHY_ENG_UNLOCK();
}

#endif
