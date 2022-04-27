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

#include "vtss_xxrp_mad.h"
#include "vtss_xxrp_madtt.h"
#include "../include/vtss_xxrp_callout.h"
#include "vtss_xxrp_timers.h"
#include "vtss_xxrp_os.h"
#include "vtss_xxrp.h"

/* TODO: connect_port also needs to be implemented */
u32 vtss_mrp_mad_create_port(vtss_mrp_t *application, vtss_mrp_mad_t **mad_ports, u32 port_no)
{
    vtss_mrp_mad_t *mad;
    u32            machine_index;
    BOOL           is_attr_registered;

    T_D("port_no = %u", port_no);
    //ASSERT_LOCKED(application);
    mad = (vtss_mrp_mad_t *)XXRP_SYS_MALLOC(sizeof(vtss_mrp_mad_t));
    memset(mad, 0, sizeof(vtss_mrp_mad_t));
    mad->machines = (vtss_mrp_mad_machine_t *)XXRP_SYS_MALLOC((application->max_mad_index + 1) * sizeof(vtss_mrp_mad_machine_t));

    /* Initialize the machine's applicant to Very Anxious Observer
     * and the registrar to EMPTY */
    for (machine_index = 0; machine_index < (application->max_mad_index + 1); machine_index++) {

        is_attr_registered = vtss_mrp_is_attr_registered(application->appl_type, port_no, machine_index + 1);
        vtss_mrp_madtt_init_state_machine(&(mad->machines[machine_index]), is_attr_registered);
    }

    vtss_mrp_madtt_init_la_and_periodic_state_machines(mad);

    mad->join_timeout = VTSS_MRP_JOIN_TIMER_DEF;
    mad->leave_timeout = VTSS_MRP_LEAVE_TIMER_DEF;
    mad->leaveall_timeout = VTSS_MRP_LEAVEALL_TIMER_DEF;
    mad->periodic_tx_timeout = VTSS_MRP_PERIODIC_TIMER_DEF;
    mad->port_no = port_no;
    mad_ports[port_no] = mad;

    /* Start the LA timer */
    vtss_xxrp_start_leaveall_timer(mad, FALSE);

    return VTSS_XXRP_RC_OK;
}

u32 vtss_mrp_mad_destroy_port(vtss_mrp_t *application, vtss_mrp_mad_t **mad_ports, u32 port_no)
{
    u32 rc;
    T_D("port_no = %u", port_no);
    //ASSERT_LOCKED(application);
    /* TODO: Send leave request on this port for all the registered attributes */
    rc = XXRP_SYS_FREE(mad_ports[port_no]->machines);
    rc = XXRP_SYS_FREE(mad_ports[port_no]);
    mad_ports[port_no] = NULL;
    return rc;
}

u32 vtss_mrp_mad_find_port(vtss_mrp_mad_t **mad_ports, u32 port_no, vtss_mrp_mad_t **mad)
{
    T_D("port_no = %u", port_no);
    //ASSERT_LOCKED(application);
    *mad = mad_ports[port_no];
    return VTSS_XXRP_RC_OK;
}
