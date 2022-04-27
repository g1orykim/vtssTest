/*

 Vitesse Switch API software.

# Copyright (c) 2002-2011 Vitesse Semiconductor Corporation "Vitesse". All
# Rights Reserved.
#
# Unpublished rights reserved under the copyright laws of the United States of
# America, other countries and international treaties. Permission to use, copy,
# store and modify, the software and its source code is granted. Permission to
# integrate into other products, disclose, transmit and distribute the software
# in an absolute machine readable format (e.g. HEX file) is also granted.  The
# source code of the software may not be disclosed, transmitted or distributed
# without the written permission of Vitesse. The software and its source code
# may only be used in products utilizing the Vitesse switch products.
#
# This copyright notice must appear in any copy, modification, disclosure,
# transmission or distribution of the software. Vitesse retains all ownership,
# copyright, trade secret and proprietary rights in the software.
#
# THIS SOFTWARE HAS BEEN PROVIDED "AS IS," WITHOUT EXPRESS OR IMPLIED WARRANTY
# INCLUDING, WITHOUT LIMITATION, IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A PARTICULAR USE AND NON-INFRINGEMENT.
 
*/

#include "mstp_priv.h"
#include "mstp_stm.h"
#include "mstp_util.h"

/*lint -esym(438,bridge,tree) -esym(529,bridge,tree) ... may be unused in port STM(s) */
/*lint --e{616, 527, 563} ... generated switch() code */
/*lint --e{801, 825}      ... goto, fallthrough */
/*lint --e{767}           ... STM_NAME multiple defined */
/*lint --e{818}           ... may be defined as const */

#define STM_NAME "PortProtocolMigration"

enum PortProtocolMigration_states {
PORTPROTOCOLMIGRATION_BEGIN,
PORTPROTOCOLMIGRATION_CHECKING_RSTP,
PORTPROTOCOLMIGRATION_SENSING,
PORTPROTOCOLMIGRATION_SELECTING_STP};

static const char *stm_statename(int state)
{
switch(state) {
case PORTPROTOCOLMIGRATION_BEGIN: return "BEGIN";
case PORTPROTOCOLMIGRATION_CHECKING_RSTP: return "CHECKING_RSTP";
case PORTPROTOCOLMIGRATION_SENSING: return "SENSING";
case PORTPROTOCOLMIGRATION_SELECTING_STP: return "SELECTING_STP";
default:
        return "-unknown-";
}
}

static int stm_enter(mstp_port_t *port, uint *transitions, int state)
{
    mstp_cistport_t *cist __attribute__ ((unused)) = getCist(port);
VTSS_ASSERT(cist == port->cistport);
mstp_bridge_t *bridge __attribute__ ((unused)) = cist->bridge;
T_D("%s: Port %d - Enter %s", STM_NAME, port->port_no, stm_statename(state)); 
    switch(state) {
case PORTPROTOCOLMIGRATION_CHECKING_RSTP:
        cist->mcheck = FALSE;
        cist->sendRSTP = rstpVersion(bridge);
        cist->mdelayWhile = MigrateTime;
break;
case PORTPROTOCOLMIGRATION_SENSING:
        cist->rcvdRSTP = cist->rcvdSTP = FALSE;
break;
case PORTPROTOCOLMIGRATION_SELECTING_STP:
        cist->sendRSTP = FALSE;
        cist->mdelayWhile = MigrateTime;
break;
default:;
}
	if(transitions)
(*transitions)++
;return state;}

static int stm_begin(mstp_port_t *port)
{
    mstp_cistport_t *cist __attribute__ ((unused)) = getCist(port);
VTSS_ASSERT(cist == port->cistport);
mstp_bridge_t *bridge __attribute__ ((unused)) = cist->bridge;

return stm_enter(port, NULL, PORTPROTOCOLMIGRATION_CHECKING_RSTP);
}

static int stm_run(mstp_port_t *port, uint *transitions, int state)
{
    uint loop_count = 0;
    mstp_cistport_t *cist __attribute__ ((unused)) = getCist(port);
VTSS_ASSERT(cist == port->cistport);
mstp_bridge_t *bridge __attribute__ ((unused)) = cist->bridge;
switch(state) {
PORTPROTOCOLMIGRATION_CHECKING_RSTP_ENTER : __attribute__ ((unused))
case PORTPROTOCOLMIGRATION_CHECKING_RSTP:
if((cist->mdelayWhile == 0) ) NewState(port, transitions, state, PORTPROTOCOLMIGRATION_SENSING);
if((cist->mdelayWhile != MigrateTime) && !portEnabled(cist) ) NewState(port, transitions, state, PORTPROTOCOLMIGRATION_CHECKING_RSTP);
break;
PORTPROTOCOLMIGRATION_SENSING_ENTER : __attribute__ ((unused))
case PORTPROTOCOLMIGRATION_SENSING:
if(!portEnabled(cist) || cist->mcheck || (rstpVersion(bridge) && !cist->sendRSTP && cist->rcvdRSTP) ) NewState(port, transitions, state, PORTPROTOCOLMIGRATION_CHECKING_RSTP);
if(cist->sendRSTP && cist->rcvdSTP ) NewState(port, transitions, state, PORTPROTOCOLMIGRATION_SELECTING_STP);
break;
PORTPROTOCOLMIGRATION_SELECTING_STP_ENTER : __attribute__ ((unused))
case PORTPROTOCOLMIGRATION_SELECTING_STP:
if((cist->mdelayWhile == 0) || !portEnabled(cist) || cist->mcheck ) NewState(port, transitions, state, PORTPROTOCOLMIGRATION_SENSING);
break;
      default:
        T_E("%s: Entering illegal state %d (%s)", STM_NAME, state, stm_statename(state));
        VTSS_ABORT();
    }
    return state;
    }

const port_stm_t PortProtocolMigration_stm = {
    .name = STM_NAME,
    .begin = stm_begin,
    .run = stm_run,
    .statename = stm_statename,
};

