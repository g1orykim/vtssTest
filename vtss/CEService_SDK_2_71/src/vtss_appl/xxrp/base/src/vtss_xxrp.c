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
#include "vtss_xxrp_api.h"
#include "vtss_xxrp_os.h"
#include "vtss_xxrp_callout.h" /* MSTP function call */



#include "vtss_xxrp_types.h"
#include "vtss_xxrp.h"
#include "vtss_xxrp_os.h"
#include "vtss_xxrp_madtt.h"
#include "vtss_xxrp_timers.h"
#include "vtss_gvrp.h"

/*lint -sem( vtss_mrp_crit_enter, thread_lock ) */
/*lint -sem( vtss_mrp_crit_exit, thread_unlock ) */

static const u8 mrp_mvrp_multicast_macaddr[] = {0x01, 0x80, 0xC2, 0x00, 0x00, 0x21};





#define MRP_MVRP_MULTICAST_MACADDR           mrp_mvrp_multicast_macaddr
#define VTSS_MVRP_ETH_TYPE                   mrp_mvrp_eth_type

/**
 * \brief structure to maintain MRP global data.
 **/
typedef struct {
    vtss_mrp_t          *applications[VTSS_MRP_APPL_MAX];                   /**< MRP application pointers               */
} vtss_xxrp_glb_t;

static vtss_xxrp_glb_t glb_mrp;        /**< Global MRP data */


#define MRP_APPL_GET(app_type)          (glb_mrp.applications[app_type])
#define MRP_MAD_GET(app_type, port_no)  (glb_mrp.applications[app_type]->mad[port_no])
#define MRP_MAP_GET(app_type, port_no)  (glb_mrp.applications[app_type]->map[port_no])
#define MRP_APPL_BASE_GET               (glb_mrp.applications)
#define MRP_MAD_BASE_GET(app_type)      (glb_mrp.applications[app_type]->mad)
#define MRP_MAP_BASE_GET(app_type)      (glb_mrp.applications[app_type]->map)
#define LOCK(app_type)                  vtss_mrp_crit_enter(app_type);
#define UNLOCK(app_type)                vtss_mrp_crit_exit(app_type);
#define ASSERT_LOCKED(app_type)         vtss_mrp_crit_assert_locked(app_type);

/* TODO: On disable remove static mac entry, release all map and mad structures for that port, unregister with packet module
 *       On enable add static mac entry and register with packet module.
 */
u32 vtss_mrp_global_control_conf_set(vtss_mrp_appl_t application, BOOL enable)
{
    vtss_mrp_t  *mrp_app = NULL;
    u32         rc = VTSS_XXRP_RC_UNKNOWN;

    T_D("application = %u, enable = %d\n", application, enable);
    if ((application >= VTSS_MRP_APPL_MAX) || (application < 0)) {
        T_E("Invalid application");
        return VTSS_XXRP_RC_INVALID_PARAMETER;
    }
    LOCK(application);
    do {
        if (enable) { /* MRP application enable */
            if (MRP_APPL_GET(application)) { /* Application present in the db */
                rc = VTSS_XXRP_RC_INVALID_PARAMETER;
                T_E("MRP application: %u is already enabled\n", application);
                break;
            } else { /* Application is not present in the db; it means it is not globally enabled */
                switch (application) {








#ifdef VTSS_SW_OPTION_GVRP
                case VTSS_GARP_APPL_GVRP:
                    if ((rc = vtss_gvrp_global_control_conf_create(&mrp_app)) != VTSS_XXRP_RC_OK) {
                        T_E("unable to create application structure");
                    }
                    break;
#endif

                default:
                    rc = VTSS_XXRP_RC_INVALID_PARAMETER;
                }
                if ((rc == VTSS_XXRP_RC_OK) && (mrp_app != NULL)) {
                    rc = mrp_appl_add_to_database(MRP_APPL_BASE_GET, mrp_app);
                }
            }
        } else { /* MRP application disable */
            if (MRP_APPL_GET(application)) { /* Application present in the db */
                rc = mrp_appl_del_from_database(MRP_APPL_BASE_GET, application);
            } else { /* Application is not present in the db; it means it is not globally enabled */
                rc = VTSS_XXRP_RC_INVALID_PARAMETER;
                T_E("MRP application: %u is not enabled\n", application);
            }
        }
    } while (0); /* end of do-while(0) */
    UNLOCK(application);

    return rc;
}

u32 vtss_mrp_global_control_conf_get(vtss_mrp_appl_t application, BOOL  *const status)
{
    T_D("application = %u\n", application);
    if ((application >= VTSS_MRP_APPL_MAX) || (application < 0)) {
        T_E("Invalid application");
        return VTSS_XXRP_RC_INVALID_PARAMETER;
    }
    if (!status) {
        T_E("status pointer is NULL\n");
        return VTSS_XXRP_RC_INVALID_PARAMETER;
    }
    //*status = (glb_mrp.applications[application]) ? TRUE : FALSE;
    LOCK(application);
    *status = (MRP_APPL_GET(application)) ? TRUE : FALSE;
    UNLOCK(application);

    return VTSS_XXRP_RC_OK;
}






































































u32 vtss_xxrp_port_control_conf_set(vtss_mrp_appl_t application, u32 port_no, BOOL enable)
{
    switch (application) {







#ifdef VTSS_SW_OPTION_GVRP
    case VTSS_GARP_APPL_GVRP:
        return vtss_gvrp_port_control_conf_set(port_no, enable);

#endif

    default:
        break;
    }

    return VTSS_XXRP_RC_INVALID_PARAMETER;
}




u32 vtss_mrp_port_control_conf_get(vtss_mrp_appl_t application, u32 port_no, BOOL *const status)
{
    u32 rc = VTSS_XXRP_RC_OK;

    T_D("application = %u, port_no = %u\n", application, port_no);
    if ((application >= VTSS_MRP_APPL_MAX) || (application < 0)) {
        T_E("Invalid application");
        return VTSS_XXRP_RC_INVALID_PARAMETER;
    }
    if (port_no >=  L2_MAX_PORTS) {
        T_E("port_no >=  L2_MAX_PORTS");
        return VTSS_XXRP_RC_INVALID_PARAMETER;
    }
    if (!status) {
        T_E("status pointer is NULL\n");
        return VTSS_XXRP_RC_INVALID_PARAMETER;
    }
    LOCK(application);
    do {
        if (!MRP_APPL_GET(application)) {
            T_E("MRP application is not enalbed globally");
            rc = VTSS_XXRP_RC_INVALID_PARAMETER;
            break;
        }
        //*status = (glb_mrp.applications[application]->mad[port_no]) ? TRUE : FALSE;
        *status = (MRP_MAD_GET(application, port_no)) ? TRUE : FALSE;
    } while (0); /* end of do-while(0) */
    UNLOCK(application);

    return rc;
}


u32 vtss_xxrp_port_control_conf_get(vtss_mrp_appl_t application, u32 port_no, BOOL *const status)
{
    switch (application) {







#ifdef VTSS_SW_OPTION_GVRP
    case VTSS_GARP_APPL_GVRP:
        return vtss_gvrp_port_control_conf_get(port_no, status);

#endif

    default:
        break;
    }

    return VTSS_XXRP_RC_INVALID_PARAMETER;
}

u32 vtss_mrp_port_ring_print(vtss_mrp_appl_t application, u8 msti)
{
    u32 rc = VTSS_XXRP_RC_OK;

    LOCK(application);
    do {
        if (!MRP_APPL_GET(application)) {
            T_E("MRP application is not enalbed globally");
            rc = VTSS_XXRP_RC_INVALID_PARAMETER;
            break;
        }
        vtss_mrp_map_print_msti_ports(MRP_MAP_BASE_GET(application), msti);
    } while (0); /* end of do-while */
    UNLOCK(application);

    return rc;
}












































































u32 vtss_xxrp_periodic_transmission_control_conf_set(vtss_mrp_appl_t application, u32 port_no, BOOL enable)
{
    switch (application) {







#ifdef VTSS_SW_OPTION_GVRP
    case VTSS_GARP_APPL_GVRP:
        return 0;

#endif

    default:
        break;
    }

    return VTSS_XXRP_RC_INVALID_PARAMETER;
}



u32 vtss_mrp_periodic_transmission_control_conf_get(vtss_mrp_appl_t application, u32 port_no, BOOL *const status)
{
    u32 rc = VTSS_XXRP_RC_OK;

    T_D("application = %u, port_no = %u\n", application, port_no);
    if ((application >= VTSS_MRP_APPL_MAX) || (application < 0)) {
        T_E("Invalid application");
        return VTSS_XXRP_RC_INVALID_PARAMETER;
    }
    if (port_no >=  L2_MAX_PORTS) {
        T_E("port_no >=  L2_MAX_PORTS");
        return VTSS_XXRP_RC_INVALID_PARAMETER;
    }
    if (!status) {
        T_E("status pointer is NULL\n");
        return VTSS_XXRP_RC_INVALID_PARAMETER;
    }
    LOCK(application);
    do {
        if (!MRP_APPL_GET(application)) {
            T_E("MRP application is not enalbed globally");
            rc = VTSS_XXRP_RC_INVALID_PARAMETER;
            break;
        }
        if (!MRP_MAD_GET(application, port_no)) {
            T_E("MRP application is not enalbed on this port = %u", port_no);
            rc = VTSS_XXRP_RC_INVALID_PARAMETER;
            break;
        }
        *status = MRP_MAD_GET(application, port_no)->periodic_timer_control_status;
    } while (0); /* end of do-while */
    UNLOCK(application);

    return rc;
}










































u32 vtss_xxrp_timers_conf_set(vtss_mrp_appl_t application, u32 port_no, vtss_mrp_timer_conf_t *const timers)
{

    switch (application) {






        // Timers are not set on a port basis for GVRP, but on an application basis
#ifdef VTSS_SW_OPTION_GVRP
    case VTSS_GARP_APPL_GVRP:
        return 0;
#endif

    default:
        break;
    }

    return VTSS_XXRP_RC_INVALID_PARAMETER;
}










































static u32 vtss_gvrp_timers_conf_get(vtss_mrp_timer_conf_t *const timers)
{
    timers->join_timer = 0;
    timers->leave_timer = 0;
    timers->leave_all_timer = 0;

    return VTSS_XXRP_RC_OK;
}

u32 vtss_xxrp_timers_conf_get(vtss_mrp_appl_t application, u32 port_no, vtss_mrp_timer_conf_t *const timers)
{

    switch (application) {






        // Timers are not set on a port basis for GVRP, but on an application basis
#ifdef VTSS_SW_OPTION_GVRP
    case VTSS_GARP_APPL_GVRP:
        return vtss_gvrp_timers_conf_get(timers);
#endif

    default:
        break;
    }

    return VTSS_XXRP_RC_INVALID_PARAMETER;
}


u32 vtss_mrp_applicant_admin_control_conf_set(vtss_mrp_appl_t             application,
                                              u32                         port_no,
                                              vtss_mrp_attribute_type_t   attr_type,
                                              BOOL                        participant)
{

    u32 rc = VTSS_XXRP_RC_OK;

    T_D("application = %u, port_no = %u, participant = %u\n", application, port_no, participant);
    if ((application >= VTSS_MRP_APPL_MAX) || (application < 0)) {
        T_E("Invalid application");
        return VTSS_XXRP_RC_INVALID_PARAMETER;
    }
    if (port_no >=  L2_MAX_PORTS) {
        T_E("port_no >=  L2_MAX_PORTS");
        return VTSS_XXRP_RC_INVALID_PARAMETER;
    }
    LOCK(application);
    do {
        if (!MRP_APPL_GET(application)) {
            T_E("MRP application is not enalbed globally");
            rc = VTSS_XXRP_RC_INVALID_PARAMETER;
            break;
        }
        if (!MRP_MAD_GET(application, port_no)) {
            T_E("MRP application is not enalbed on this port = %u", port_no);
            rc = VTSS_XXRP_RC_INVALID_PARAMETER;
            break;
        }










    } while (0); /* end of do-while */
    UNLOCK(application);

    return rc;
}

u32 vtss_gvrp_applicant_admin_control_conf_set(u32                         port_no,
                                               vtss_mrp_attribute_type_t   attr_type,
                                               BOOL                        participant)
{
    return 0;
}

u32 vtss_xxrp_applicant_admin_control_conf_set(vtss_mrp_appl_t             application,
                                               u32                         port_no,
                                               vtss_mrp_attribute_type_t   attr_type,
                                               BOOL                        participant)
{

    switch (application) {







        // Timers are not set on a port basis for GVRP, but on an application basis
#ifdef VTSS_SW_OPTION_GVRP
    case VTSS_GARP_APPL_GVRP:
        return vtss_gvrp_applicant_admin_control_conf_set(port_no, attr_type, participant);
#endif

    default:
        break;
    }

    return VTSS_XXRP_RC_INVALID_PARAMETER;
}


u32 vtss_mrp_applicant_admin_control_conf_get(vtss_mrp_appl_t             application,
                                              u32                         port_no,
                                              vtss_mrp_attribute_type_t   attr_type,
                                              BOOL  *const                status)
{
    u32 rc = VTSS_XXRP_RC_OK;

    T_D("application = %u, port_no = %u\n", application, port_no);
    if ((application >= VTSS_MRP_APPL_MAX) || (application < 0)) {
        T_E("Invalid application");
        return VTSS_XXRP_RC_INVALID_PARAMETER;
    }
    if (port_no >=  L2_MAX_PORTS) {
        T_E("port_no >=  L2_MAX_PORTS");
        return VTSS_XXRP_RC_INVALID_PARAMETER;
    }
    if (!status) {
        T_E("status pointer is NULL\n");
        return VTSS_XXRP_RC_INVALID_PARAMETER;
    }
    LOCK(application);
    do {
        if (!MRP_APPL_GET(application)) {
            T_E("MRP application is not enalbed globally");
            rc = VTSS_XXRP_RC_INVALID_PARAMETER;
            break;
        }
        if (!MRP_MAD_GET(application, port_no)) {
            T_E("MRP application is not enalbed on this port = %u", port_no);
            rc = VTSS_XXRP_RC_INVALID_PARAMETER;
            break;
        }










    } while (0); /* end of do-while */
    UNLOCK(application);

    return rc;
}

u32 vtss_mrp_statistics_get(vtss_mrp_appl_t application, u32 port_no, vtss_mrp_statistics_t *const stats)
{
    u32 rc = VTSS_XXRP_RC_OK;

    T_D("application = %u, port_no = %u\n", application, port_no);
    if ((application >= VTSS_MRP_APPL_MAX) || (application < 0)) {
        T_E("Invalid application");
        return VTSS_XXRP_RC_INVALID_PARAMETER;
    }
    if (port_no >=  L2_MAX_PORTS) {
        T_E("port_no >=  L2_MAX_PORTS");
        return VTSS_XXRP_RC_INVALID_PARAMETER;
    }
    if (!stats) {
        T_E("stats pointer is NULL\n");
        return VTSS_XXRP_RC_INVALID_PARAMETER;
    }
    LOCK(application);
    do {
        if (!MRP_APPL_GET(application)) {
            T_E("MRP application is not enalbed globally");
            rc = VTSS_XXRP_RC_INVALID_PARAMETER;
            break;
        }
        if (!MRP_MAD_GET(application, port_no)) {
            T_E("MRP application is not enalbed on this port = %u", port_no);
            rc = VTSS_XXRP_RC_INVALID_PARAMETER;
            break;
        }
        *stats = MRP_MAD_GET(application, port_no)->stats;
    } while (0); /* end of do-while */
    UNLOCK(application);

    return rc;
}

u32 vtss_mrp_rx_statistics_set(vtss_mrp_appl_t application, u32 port_no, vtss_mrp_stat_type_t stat_type)
{
    vtss_mrp_mad_t  *mad;
    vtss_rc         rc = VTSS_RC_OK;

    T_D("application = %u, port_no = %u\n", application, port_no);
    if ((application >= VTSS_MRP_APPL_MAX) || (application < 0)) {
        T_E("Invalid application");
        return VTSS_XXRP_RC_INVALID_PARAMETER;
    }
    if (port_no >=  L2_MAX_PORTS) {
        T_E("port_no >=  L2_MAX_PORTS");
        return VTSS_XXRP_RC_INVALID_PARAMETER;
    }
    LOCK(application);
    do {
        if (!MRP_APPL_GET(application)) {
            T_E("MRP application is not enalbed globally");
            rc = VTSS_XXRP_RC_INVALID_PARAMETER;
            break;
        }
        mad = MRP_MAD_GET(application, port_no);
        if (!mad) {
            T_E("MRP application is not enalbed on this port = %u", port_no);
            rc = VTSS_XXRP_RC_INVALID_PARAMETER;
            break;
        }
        switch (stat_type) {
        case VTSS_MRP_TOTAL_RX_PKTS:
            mad->stats.total_pkts_recvd++;
            break;
        case VTSS_MRP_DROPPED_PKTS:
            mad->stats.pkts_dropped++;
            break;
        case VTSS_MRP_RX_NEW:
            mad->stats.new_recvd++;
            break;
        case VTSS_MRP_RX_JOININ:
            mad->stats.joinin_recvd++;
            break;
        case VTSS_MRP_RX_IN:
            mad->stats.in_recvd++;
            break;
        case VTSS_MRP_RX_JOINMT:
            mad->stats.joinmt_recvd++;
            break;
        case VTSS_MRP_RX_MT:
            mad->stats.mt_recvd++;
            break;
        case VTSS_MRP_RX_LV:
            mad->stats.leave_recvd++;
            break;
        case VTSS_MRP_RX_LA:
            mad->stats.leaveall_recvd++;
            break;
        default:
            break;
        } /* switch(stat_type)  */
    } while (0);
    UNLOCK(application);

    return rc;
}


u32 vtss_mrp_tx_statistics_set(vtss_mrp_appl_t application, u32 port_no, vtss_mrp_stat_type_t stat_type)
{
    vtss_mrp_mad_t  *mad;
    vtss_rc         rc = VTSS_RC_OK;

    T_N("application = %u, port_no = %u\n", application, port_no);
    if ((application >= VTSS_MRP_APPL_MAX) || (application < 0)) {
        T_E("Invalid application");
        return VTSS_XXRP_RC_INVALID_PARAMETER;
    }
    if (port_no >=  L2_MAX_PORTS) {
        T_E("port_no >=  L2_MAX_PORTS");
        return VTSS_XXRP_RC_INVALID_PARAMETER;
    }
    ASSERT_LOCKED(application);
    do {
        if (!MRP_APPL_GET(application)) {
            T_E("MRP application is not enalbed globally");
            rc = VTSS_XXRP_RC_INVALID_PARAMETER;
            break;
        }
        mad = MRP_MAD_GET(application, port_no);
        if (!mad) {
            T_E("MRP application is not enalbed on this port = %u", port_no);
            rc = VTSS_XXRP_RC_INVALID_PARAMETER;
            break;
        }
        switch (stat_type) {
        case VTSS_MRP_TX_PKTS:
            mad->stats.pkts_transmitted++;
            break;
        case VTSS_MRP_TX_NEW:
            mad->stats.new_transmitted++;
            break;
        case VTSS_MRP_TX_JOININ:
            mad->stats.joinin_transmitted++;
            break;
        case VTSS_MRP_TX_IN:
            mad->stats.in_transmitted++;
            break;
        case VTSS_MRP_TX_JOINMT:
            mad->stats.joinmt_transmitted++;
            break;
        case VTSS_MRP_TX_MT:
            mad->stats.mt_transmitted++;
            break;
        case VTSS_MRP_TX_LV:
            mad->stats.leave_transmitted++;
            break;
        case VTSS_MRP_TX_LA:
            mad->stats.leaveall_transmitted++;
            break;
        default:
            break;
        } /* switch(stat_type)  */
    } while (0);

    return rc;
}

u32 vtss_mrp_statistics_clear(vtss_mrp_appl_t application, u32 port_no)
{
    u32 rc = VTSS_XXRP_RC_OK;

    T_D("application = %u, port_no = %u\n", application, port_no);
    if ((application >= VTSS_MRP_APPL_MAX) || (application < 0)) {
        T_E("Invalid application");
        return VTSS_XXRP_RC_INVALID_PARAMETER;
    }
    if (port_no >=  L2_MAX_PORTS) {
        T_E("port_no >=  L2_MAX_PORTS");
        return VTSS_XXRP_RC_INVALID_PARAMETER;
    }
    LOCK(application);
    do {
        if (!MRP_APPL_GET(application)) {
            T_E("MRP application is not enalbed globally");
            rc = VTSS_XXRP_RC_INVALID_PARAMETER;
            break;
        }
        if (!MRP_MAD_GET(application, port_no)) {
            T_E("MRP application is not enalbed on this port = %u", port_no);
            rc = VTSS_XXRP_RC_INVALID_PARAMETER;
            break;
        }
        memset(&(MRP_MAD_GET(application, port_no)->stats), 0, sizeof(vtss_mrp_statistics_t));
    } while (0); /* end of do-while */
    UNLOCK(application);

    return rc;
}

BOOL vtss_mrp_is_attr_registered(vtss_mrp_appl_t application, u32 port_no, u32 attr)
{





    return FALSE;
}
// ?tf?













































u32 vtss_xxrp_mstp_port_state_change_handler(u32 port_no, u8 msti, vtss_mrp_mstp_port_state_change_type_t  port_state_type)
{
    u32 rc1 = 0, rc2 = 0;





#ifdef VTSS_SW_OPTION_GVRP
    rc2 = vtss_gvrp_mstp_port_state_change_handler(port_no, msti, port_state_type);
#endif

    return rc1 ? rc1 : rc2;
}






















































































u32 vtss_mrp_port_update_peer_mac_addr(vtss_mrp_appl_t application, u32 port_no, u8 *mac_addr)
{
    vtss_mrp_mad_t  *mad;
    u32             rc = VTSS_XXRP_RC_OK;

    LOCK(application);
    do {
        if (!MRP_APPL_GET(application)) {
            T_E("MRP application is not enalbed globally");
            rc = VTSS_XXRP_RC_INVALID_PARAMETER;
            break;
        }
        mad = MRP_MAD_GET(application, port_no);
        if (mad) {
            memcpy(mad->peer_mac_address, mac_addr, sizeof(mad->peer_mac_address));
            mad->peer_mac_updated = TRUE;
        }
    } while (0); /* end of do-while */
    UNLOCK(application);

    return rc;
}

/* JGSD: Can we decalre this as static? */
u32 vtss_mrp_port_mad_get(vtss_mrp_appl_t application, u32 port_no, vtss_mrp_mad_t **mad)
{
    u32 rc = VTSS_XXRP_RC_OK;

    do {
        if (!MRP_APPL_GET(application)) {
            T_E("MRP application is not enalbed globally");
            rc = VTSS_XXRP_RC_INVALID_PARAMETER;
            break;
        }
        *mad = MRP_MAD_GET(application, port_no);
    } while (0); /* end of do-while */

    return rc;
}
/* JGSD: This function should be called with lock */
u32 vtss_mrp_mad_process_events(vtss_mrp_appl_t application, vtss_mrp_attribute_type_t *attr_type, u32 port_no,
                                u16 mad_fsm_indx, vtss_mad_fsm_events *fsm_events)
{
    vtss_mrp_mad_t *mad_port;
    u32            rc;

    ASSERT_LOCKED(application);
    if ((rc = vtss_mrp_port_mad_get(application, port_no, &mad_port)) == VTSS_XXRP_RC_OK) {

        rc = vtss_mrp_madtt_event_handler(application, attr_type, mad_port, mad_fsm_indx, fsm_events);
    }

    return rc;
}
#if 0
void vtss_mrp_mad_indx_to_attr_val(vtss_mrp_appl_t application, u16 mad_indx, u16 *attr_val)
{





}
#endif
/* JGSD: This function should be called with lock */
u32 vtss_mrp_propagate_join(vtss_mrp_appl_t application, u32 port_no, u32 mad_indx)
{
    u32             rc;

    rc = vtss_mrp_map_propagate_join(application, MRP_MAP_BASE_GET(application), port_no, mad_indx);

    return rc;
}

/* JGSD: Can we make this as a static func? */
/* JGSD: This function should be called with lock */
u32 vtss_mrp_propagate_leave(vtss_mrp_appl_t application, u32 port_no, u32 mad_indx)
{
    u32             rc;

    rc = vtss_mrp_map_propagate_leave(application, MRP_MAP_BASE_GET(application), port_no, mad_indx);

    return rc;
}

/* JGSD: Need to check the LOCK path: */
u32 vtss_mrp_join_indication(vtss_mrp_appl_t application, u32 port_no, vtss_mrp_attribute_type_t *attr_type, u32 mad_indx, BOOL new)
{
    BOOL    tc_detected = FALSE;
    u32     rc;
    /* TODO: Check topology change and set the new accordingly */
    if (!new && tc_detected) {
        new = TRUE;
    }

    ASSERT_LOCKED(application);
    //vtss_mrp_mad_indx_to_attr_val(application, mad_indx, &attr_val);
    MRP_APPL_GET(application)->join_indication_fn(port_no, attr_type, mad_indx, new);
    rc = vtss_mrp_propagate_join(application, port_no, mad_indx);

    return rc;
}


















BOOL vtss_mrp_is_registrar_check_is_change_allowed(vtss_mrp_appl_t application, u32 port_no, u32 mad_indx)
{
    return FALSE;
}


/* JGSD: Need to check the LOCK path */
u32 vtss_mrp_leave_indication(vtss_mrp_appl_t application, u32 port_no, vtss_mrp_attribute_type_t *attr_type, u32 mad_indx)
{
    u32 rc;

    ASSERT_LOCKED(application);
    MRP_APPL_GET(application)->leave_indication_fn(port_no, attr_type, mad_indx);
    rc = vtss_mrp_propagate_leave(application, port_no, mad_indx);

    return rc;
}

/* JGSD: Need to check the Lock path */
u32 vtss_mrp_map_port_change_handler(vtss_mrp_appl_t application, u32 port_no, BOOL is_add)
{
    vtss_map_port_t             *map;
    u32                         machine_index, rc = VTSS_XXRP_RC_OK, tmp_port;
    u32                         machine_index_cnt;
    u8                          msti;
    vtss_mad_fsm_events         fsm_events;
    vtss_mrp_attribute_type_t   attr_type;
    BOOL                        another_port_has_registered = FALSE;
    u8                          tmp_state;

    ASSERT_LOCKED(application);

    //vtss_mrp_crit_assert_locked();
    if (!MRP_APPL_GET(application)) {
        T_D("MRP application is not enalbed globally");
        rc = VTSS_XXRP_RC_INVALID_PARAMETER;
    }
    if (!MRP_MAD_GET(application, port_no)) {
        T_D("MRP application is not enalbed on this port");
        rc = VTSS_XXRP_RC_INVALID_PARAMETER;
    }
    if (rc != VTSS_XXRP_RC_OK) {
        return rc;
    }
    /* Only applicant state machines will be impacted */
    fsm_events.la_event = last_la_event;
    fsm_events.periodic_event = last_periodic_event;
    fsm_events.reg_event = last_reg_event;





    machine_index_cnt = (MRP_APPL_GET(application)->max_mad_index - 1);
    if (is_add) { /* A port is added to the port set to participate in MRP protocol operation (MSTP or Port Add)*/
        fsm_events.appl_event = join_app;
        for (machine_index = 0; machine_index < (machine_index_cnt); machine_index++) {
            T_N("indx = %u", machine_index);
            /* For each registered atribure, join should be propagated on other ports in this msti */
            if (vtss_mrp_madtt_get_mad_registrar_state(MRP_MAD_GET(application, port_no), machine_index) == IN) {
                T_N("Propagating join to other ports of the msti. indx = %u, port_no = %u", machine_index, port_no);
                rc += vtss_mrp_propagate_join(application, port_no, machine_index);
            } /* if (vtss_mrp_madtt_get_mad_registrar_state(MRP_MAD_GET(application, port_no), machine_index) == IN) */
            for (tmp_port = 0; tmp_port < L2_MAX_PORTS; tmp_port++) {
                if (MRP_MAD_GET(application, tmp_port)) {
                    if (vtss_mrp_madtt_get_mad_registrar_state(MRP_MAD_GET(application, tmp_port), machine_index) == IN) {
                        if (tmp_port != port_no) {
                            /* Check if port_no is part of this msti. If yes, propagate join on this port */
                            vtss_mrp_get_msti_from_mad_index(machine_index, &msti);
                            /* Send join request on this port */
                            if ((vtss_mrp_map_find_port(MRP_MAP_BASE_GET(application), msti, port_no, &map)) == VTSS_XXRP_RC_OK) {
                                T_N("Propagating other ports registered attributes on this port. indx = %u, port_no = %u",
                                    machine_index, port_no);
                                rc += vtss_mrp_mad_process_events(application, &attr_type, port_no, machine_index, &fsm_events);
                            } /* if ((vtss_mrp_map_find_port(MRP_MAP_BASE_GET(application), msti, port_no, &map)) == VTSS_XXRP_RC_OK) */
                        } /* if (tmp_port != port_no) */
                    } /* if (vtss_mrp_madtt_get_mad_registrar_state(MRP_MAD_GET(application, tmp_port), machine_index) == IN) */
                } /* if (MRP_MAD_GET(application, tmp_port)) */
            } /* for (tmp_port = 0; tmp_port < L2_MAX_PORTS; tmp_port++) */
        } /* for (machine_index = 0; machine_index < (MRP_APPL_GET(application)->max_mad_index); machine_index++) */
    } else {
        fsm_events.appl_event = lv_app;
        for (machine_index = 0; machine_index < (machine_index_cnt); machine_index++) {
            T_N("indx = %u", machine_index);
            /* For each registered atribure, leave should be propagated on other ports in this msti */
            tmp_state = vtss_mrp_madtt_get_mad_registrar_state(MRP_MAD_GET(application, port_no), machine_index);
            if (tmp_state == IN) {
                another_port_has_registered = FALSE;
                for (tmp_port = 0; tmp_port < L2_MAX_PORTS; tmp_port++) {
                    if (MRP_MAD_GET(application, tmp_port)) {
                        if (tmp_port != port_no) {
                            if (vtss_mrp_madtt_get_mad_registrar_state(MRP_MAD_GET(application, tmp_port), machine_index) == IN) {
                                another_port_has_registered = TRUE;
                                T_N("Propagate leave request to the other registered ports. indx = %u, port_no = %u",
                                    machine_index, tmp_port);
                                /* Send leave request to tmp_port */
                                rc += vtss_mrp_mad_process_events(application, &attr_type, tmp_port, machine_index, &fsm_events);
                            } /* if (vtss_mrp_madtt_get_mad_registrar_state(MRP_MAD_GET(application, tmp_port), machine_index) == IN) */
                        } /* if (tmp_port != port_no) */
                    } /* if (MRP_MAD_GET(application, tmp_port)) */
                } /* for (tmp_port = 0; tmp_port < L2_MAX_PORTS; tmp_port++) */
                if (another_port_has_registered == FALSE) { /* Send leave request to all other ports in the msti */
                    T_N("Propagating leave to other ports of the msti. indx = %u, port_no = %u", machine_index, port_no);
                    rc += vtss_mrp_propagate_leave(application, port_no, machine_index);
                } /* if (another_port_has_registered == FALSE) */
            } /* if (vtss_mrp_madtt_get_mad_registrar_state(MRP_MAD_GET(application, port_no), machine_index) == IN) */
        } /* for (machine_index = 0; machine_index < (MRP_APPL_GET(application)->max_mad_index); machine_index++) */
    } /* if (is_add) */

    return rc;
}


/* JGSD: This function should be called with LOCK */
static u32 vtss_mrp_handle_leaveall(vtss_mrp_appl_t application, vtss_mrp_mad_t *mad)
{
    vtss_mad_fsm_events         fsm_events;
    u32                         rc = VTSS_XXRP_RC_OK, machine_index;
    vtss_mrp_attribute_type_t   attr_type;

    T_D("port = %u", mad->port_no);
    ASSERT_LOCKED(application);
    fsm_events.la_event = last_la_event;
    fsm_events.periodic_event = last_periodic_event;
    fsm_events.reg_event = rLA_reg;
    fsm_events.appl_event = rLA_app;
    attr_type.dummy = 0; /* This is not really required. This is just to make lint happy */
    for (machine_index = 0; machine_index < (MRP_APPL_GET(application)->max_mad_index); machine_index++) {
        T_N("in=%u ", machine_index);
        if (vtss_mrp_is_registrar_check_is_change_allowed(application, mad->port_no, machine_index)) {
            rc = vtss_mrp_mad_process_events(application, &attr_type, mad->port_no, machine_index, &fsm_events);
        }
    }

    return rc;
}

/* JGSD: Triggeres by state machine call */
u32 vtss_mrp_handle_periodic_timer(vtss_mrp_appl_t application, u32 port_no)
{
    vtss_mad_fsm_events         fsm_events;
    u32                         rc = VTSS_XXRP_RC_OK, machine_index;
    vtss_mrp_attribute_type_t   attr_type;
    vtss_mrp_mad_t              *mad;

    T_D("port = %u", port_no);

    ASSERT_LOCKED(application);
    do {
        if (!MRP_APPL_GET(application)) {
            T_E("MRP application is not enalbed globally");
            rc = VTSS_XXRP_RC_INVALID_PARAMETER;
        }
        mad = MRP_MAD_GET(application, port_no);
        if (mad) {
            fsm_events.la_event = last_la_event;
            fsm_events.periodic_event = last_periodic_event;
            fsm_events.reg_event = last_reg_event;
            fsm_events.appl_event = periodic_app;
            attr_type.dummy = 0; /* This is not really required. This is just to make lint happy */
            for (machine_index = 0; machine_index < (MRP_APPL_GET(application)->max_mad_index); machine_index++) {
                rc = vtss_mrp_mad_process_events(application, &attr_type, port_no, machine_index, &fsm_events);
            }
        }
    } while (0); /* end of do-while */

    return rc;
}
u32 vtss_mrp_process_leaveall(vtss_mrp_appl_t application, vtss_mrp_attribute_type_t *attr_type, u32 port_no)
{
    vtss_mad_fsm_events fsm_events;
    u32                 rc;

    fsm_events.la_event = rLA_la;
    fsm_events.periodic_event = last_periodic_event;
    fsm_events.reg_event = last_reg_event;
    fsm_events.appl_event = last_appl_event;
    LOCK(application);
    rc = vtss_mrp_mad_process_events(application, attr_type, port_no, MADTT_INVALID_INDEX, &fsm_events);
    rc += vtss_mrp_handle_leaveall(application, MRP_MAD_GET(application, port_no));
    UNLOCK(application);

    return rc;
}
u32 vtss_mrp_process_new_event(vtss_mrp_appl_t application, vtss_mrp_attribute_type_t *attr_type, u32 port_no, u16 mad_fsm_index)
{
    vtss_mad_fsm_events fsm_events;
    u32                 rc;

    fsm_events.la_event = last_la_event;
    fsm_events.periodic_event = last_periodic_event;
    fsm_events.reg_event = rNew_reg;
    fsm_events.appl_event = rNew_app;
    LOCK(application);
    rc = vtss_mrp_mad_process_events(application, attr_type, port_no, mad_fsm_index, &fsm_events);
    UNLOCK(application);

    return rc;
}
u32 vtss_mrp_process_joinin_event(vtss_mrp_appl_t application, vtss_mrp_attribute_type_t *attr_type, u32 port_no, u16 mad_fsm_index)
{
    vtss_mad_fsm_events fsm_events;
    u32                 rc;

    fsm_events.la_event = last_la_event;
    fsm_events.periodic_event = last_periodic_event;
    fsm_events.reg_event = rJoinIn_reg;
    fsm_events.appl_event = rJoinIn_app;
    LOCK(application);
    rc = vtss_mrp_mad_process_events(application, attr_type, port_no, mad_fsm_index, &fsm_events);
    UNLOCK(application);

    return rc;
}
u32 vtss_mrp_process_in_event(vtss_mrp_appl_t application, vtss_mrp_attribute_type_t *attr_type, u32 port_no, u16 mad_fsm_index)
{
    vtss_mad_fsm_events fsm_events;
    u32                 rc;

    fsm_events.la_event = last_la_event;
    fsm_events.periodic_event = last_periodic_event;
    fsm_events.reg_event = last_reg_event;
    fsm_events.appl_event = rIn_app;
    LOCK(application);
    rc = vtss_mrp_mad_process_events(application, attr_type, port_no, mad_fsm_index, &fsm_events);
    UNLOCK(application);

    return rc;
}
u32 vtss_mrp_process_joinmt_event(vtss_mrp_appl_t application, vtss_mrp_attribute_type_t *attr_type, u32 port_no, u16 mad_fsm_index)
{
    vtss_mad_fsm_events fsm_events;
    u32                 rc;

    fsm_events.la_event = last_la_event;
    fsm_events.periodic_event = last_periodic_event;
    fsm_events.reg_event = rJoinMt_reg;
    fsm_events.appl_event = rJoinMt_app;
    LOCK(application);
    rc = vtss_mrp_mad_process_events(application, attr_type, port_no, mad_fsm_index, &fsm_events);
    UNLOCK(application);

    return rc;
}
u32 vtss_mrp_process_mt_event(vtss_mrp_appl_t application, vtss_mrp_attribute_type_t *attr_type, u32 port_no, u16 mad_fsm_index)
{
    vtss_mad_fsm_events fsm_events;
    u32                 rc;

    fsm_events.la_event = last_la_event;
    fsm_events.periodic_event = last_periodic_event;
    fsm_events.reg_event = last_reg_event;
    fsm_events.appl_event = rMt_app;
    LOCK(application);
    rc = vtss_mrp_mad_process_events(application, attr_type, port_no, mad_fsm_index, &fsm_events);
    UNLOCK(application);

    return rc;
}
u32 vtss_mrp_process_lv_event(vtss_mrp_appl_t application, vtss_mrp_attribute_type_t *attr_type, u32 port_no, u16 mad_fsm_index)
{
    vtss_mad_fsm_events fsm_events;
    u32                 rc;

    fsm_events.la_event = last_la_event;
    fsm_events.periodic_event = last_periodic_event;
    fsm_events.reg_event = rLv_reg;
    fsm_events.appl_event = rLv_app;
    LOCK(application);
    rc = vtss_mrp_mad_process_events(application, attr_type, port_no, mad_fsm_index, &fsm_events);
    UNLOCK(application);

    return rc;
}

/* JGSD: Removing the locks from here */
BOOL vtss_mrp_mrpdu_rx(u32 port_no, const u8 *mrpdu, u32 length)
{
    cyg_tick_count_t    start_time, end_time;
    xxrp_eth_hdr_t      *l2_hdr;
    T_N("MRPDU rx, port %u, len %u", port_no, length);




    T_D("Start");
    start_time = cyg_current_time();
    l2_hdr = (xxrp_eth_hdr_t *) mrpdu;

#if defined(VTSS_SW_OPTION_MVRP) || defined(VTSS_SW_OPTION_GVRP)

    /* Check if the MRPDU is MVRP PDU */
    if (!memcmp(l2_hdr->dst_mac, MRP_MVRP_MULTICAST_MACADDR, VTSS_XXRP_MAC_ADDR_LEN)) {











#ifdef VTSS_SW_OPTION_GVRP
        // This must be a LLC packet, i.e. len/type<=1500 with DSAP=0x42, LSAP=0x42, Control=0x03
        if (xxrp_ntohs(l2_hdr->eth_type) <= 1500 && l2_hdr->dsap == 0x42 && l2_hdr->lsap == 0x42 && l2_hdr->control == 0x03) {

            T_D("Received GVRP PDU");

            // This is a LLC frame. ?tf?
            if ( vtss_gvrp_rx_pdu(port_no, mrpdu, length) ) {
                T_W("GVRP PDU partially or not parsed");
            }

        }




#endif /* VTSS_SW_OPTION_GVRP */
    }

#endif /* VTSS_SW_OPTION_MVRP or VTSS_SW_OPTION_GVRP */


    end_time = cyg_current_time();
    T_I("Time to execute vtss_mrp_mrpdu_rx = %u ms\n", (u32)VTSS_OS_TICK2MSEC(end_time - start_time));
    T_D("End");
    return TRUE;
}

void vtss_xxrp_update_tx_stats(vtss_mrp_appl_t application, u32 port_no, u8 event)
{
    switch (event) {
    case VTSS_XXRP_APPL_EVENT_NEW:
        (void)vtss_mrp_tx_statistics_set(application, port_no, VTSS_MRP_TX_NEW);
        break;
    case VTSS_XXRP_APPL_EVENT_JOININ:
        (void)vtss_mrp_tx_statistics_set(application, port_no, VTSS_MRP_TX_JOININ);
        break;
    case VTSS_XXRP_APPL_EVENT_IN:
        (void)vtss_mrp_tx_statistics_set(application, port_no, VTSS_MRP_TX_IN);
        break;
    case VTSS_XXRP_APPL_EVENT_JOINMT:
        (void)vtss_mrp_tx_statistics_set(application, port_no, VTSS_MRP_TX_JOINMT);
        break;
    case VTSS_XXRP_APPL_EVENT_MT:
        (void)vtss_mrp_tx_statistics_set(application, port_no, VTSS_MRP_TX_MT);
        break;
    case VTSS_XXRP_APPL_EVENT_LV:
        (void)vtss_mrp_tx_statistics_set(application, port_no, VTSS_MRP_TX_LV);
        break;
    case VTSS_XXRP_APPL_EVENT_LA:
        (void)vtss_mrp_tx_statistics_set(application, port_no, VTSS_MRP_TX_LA);
        break;
    case VTSS_XXRP_APPL_EVENT_TX_PKTS:
        (void)vtss_mrp_tx_statistics_set(application, port_no, VTSS_MRP_TX_PKTS);
        break;
    default:
        break;
    }
}
























u32 vtss_xxrp_vlan_change_handler(vtss_mrp_appl_t application, u32 fsm_index, u32 port_no, BOOL is_add)
{
    u32                         rc = VTSS_XXRP_RC_OK;

    T_D("indx = %u, port = %u, is_add = %u", fsm_index, port_no, is_add);

    LOCK(application);
    do {
        if (!MRP_APPL_GET(application)) {
            T_D("MRP application is not enalbed globally");
            rc = VTSS_XXRP_RC_INVALID_PARAMETER;
            break;
        }
        if (!MRP_MAD_GET(application, port_no)) {
            T_D("MRP application is not enalbed on this port = %u", port_no);
            rc = VTSS_XXRP_RC_INVALID_PARAMETER;
            break;
        }
        if (is_add) {
            vtss_mrp_madtt_set_mad_registrar_state(MRP_MAD_GET(application, port_no), fsm_index, IN);
            /* TODO: send the join on the port_no also */
            rc = vtss_mrp_propagate_join(application, port_no, fsm_index);
        } else {
            vtss_mrp_madtt_set_mad_registrar_state(MRP_MAD_GET(application, port_no), fsm_index, MT);
            rc = vtss_mrp_propagate_leave(application, port_no, fsm_index);
        }
    } while (0); /* end of do-while */
    UNLOCK(application);

    return rc;
}













































































































































































































































































































































































uint vtss_xxrp_timer_tick(uint delay)
{





#ifdef VTSS_SW_OPTION_GVRP
    return vtss_gvrp_timer_tick(delay);
#endif


}


/* MRP initialization function */
void vtss_mrp_init(void)
{



    memset(&glb_mrp, 0, sizeof(glb_mrp));



}
