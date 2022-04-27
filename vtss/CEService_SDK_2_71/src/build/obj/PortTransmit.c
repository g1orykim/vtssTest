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

#define can_send ((cist->txCount < TxHoldCount(bridge)) && (cist->helloWhen != 0))
#define STM_NAME "PortTransmit"

enum PortTransmit_states {
PORTTRANSMIT_BEGIN,
PORTTRANSMIT_TRANSMIT_INIT,
PORTTRANSMIT_IDLE,
PORTTRANSMIT_TRANSMIT_PERIODIC,
PORTTRANSMIT_TRANSMIT_CONFIG,
PORTTRANSMIT_TRANSMIT_TCN,
PORTTRANSMIT_TRANSMIT_RSTP};

static const char *stm_statename(int state)
{
switch(state) {
case PORTTRANSMIT_BEGIN: return "BEGIN";
case PORTTRANSMIT_TRANSMIT_INIT: return "TRANSMIT_INIT";
case PORTTRANSMIT_IDLE: return "IDLE";
case PORTTRANSMIT_TRANSMIT_PERIODIC: return "TRANSMIT_PERIODIC";
case PORTTRANSMIT_TRANSMIT_CONFIG: return "TRANSMIT_CONFIG";
case PORTTRANSMIT_TRANSMIT_TCN: return "TRANSMIT_TCN";
case PORTTRANSMIT_TRANSMIT_RSTP: return "TRANSMIT_RSTP";
default:
        return "-unknown-";
}
}

static int stm_enter(mstp_port_t *port, uint *transitions, int state)
{
    mstp_cistport_t *cist __attribute__ ((unused)) = getCist(port);
VTSS_ASSERT(cist == port->cistport);
mstp_bridge_t *bridge __attribute__ ((unused)) = cist->bridge;
T_N("%s: Port %d - Enter %s", STM_NAME, port->port_no, stm_statename(state)); 
    switch(state) {
case PORTTRANSMIT_TRANSMIT_INIT:
        cist->newInfo = cist->newInfoMsti = TRUE;
        cist->txCount = 0;
break;
case PORTTRANSMIT_IDLE:
        cist->helloWhen = HelloTime(port);
break;
case PORTTRANSMIT_TRANSMIT_PERIODIC:
        cist->newInfo = cist->newInfo || (port->role == DesignatedPort || (port->role == RootPort && (port->tcWhile != 0)));        
        cist->newInfoMsti = cist->newInfoMsti || mstiDesignatedOrTCpropagatingRootPort(port);
break;
case PORTTRANSMIT_TRANSMIT_CONFIG:
        cist->newInfo = FALSE;
        txConfig(port);
        cist->txCount += 1;
        cist->tcAck = FALSE;
break;
case PORTTRANSMIT_TRANSMIT_TCN:
        cist->newInfo = FALSE;
        txTcn(port); 
        cist->txCount += 1;
break;
case PORTTRANSMIT_TRANSMIT_RSTP:
        cist->newInfo = cist->newInfoMsti = FALSE; 
        txMstp(port); 
        cist->txCount += 1;
        cist->tcAck = FALSE;
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

return stm_enter(port, NULL, PORTTRANSMIT_TRANSMIT_INIT);
}

static int stm_run(mstp_port_t *port, uint *transitions, int state)
{
    uint loop_count = 0;
    mstp_cistport_t *cist __attribute__ ((unused)) = getCist(port);
VTSS_ASSERT(cist == port->cistport);
mstp_bridge_t *bridge __attribute__ ((unused)) = cist->bridge;
switch(state) {
PORTTRANSMIT_TRANSMIT_INIT_ENTER : __attribute__ ((unused))
case PORTTRANSMIT_TRANSMIT_INIT:
NewState(port, transitions, state, PORTTRANSMIT_IDLE);
break;
PORTTRANSMIT_IDLE_ENTER : __attribute__ ((unused))
case PORTTRANSMIT_IDLE:
if( allTransmitReady(port)) { /* DEFAULT GUARD */
if(cist->helloWhen == 0 ) NewState(port, transitions, state, PORTTRANSMIT_TRANSMIT_PERIODIC);
if(!cist->sendRSTP && cist->newInfo && port->role == DesignatedPort && can_send ) NewState(port, transitions, state, PORTTRANSMIT_TRANSMIT_CONFIG);
if(!cist->sendRSTP && cist->newInfo && port->role == RootPort && can_send ) NewState(port, transitions, state, PORTTRANSMIT_TRANSMIT_TCN);
if(cist->sendRSTP && (cist->newInfo || (cist->newInfoMsti && !mstiMasterPort(port))) && can_send ) NewState(port, transitions, state, PORTTRANSMIT_TRANSMIT_RSTP);
}
break;
PORTTRANSMIT_TRANSMIT_PERIODIC_ENTER : __attribute__ ((unused))
case PORTTRANSMIT_TRANSMIT_PERIODIC:
NewState(port, transitions, state, PORTTRANSMIT_IDLE);
break;
PORTTRANSMIT_TRANSMIT_CONFIG_ENTER : __attribute__ ((unused))
case PORTTRANSMIT_TRANSMIT_CONFIG:
NewState(port, transitions, state, PORTTRANSMIT_IDLE);
break;
PORTTRANSMIT_TRANSMIT_TCN_ENTER : __attribute__ ((unused))
case PORTTRANSMIT_TRANSMIT_TCN:
NewState(port, transitions, state, PORTTRANSMIT_IDLE);
break;
PORTTRANSMIT_TRANSMIT_RSTP_ENTER : __attribute__ ((unused))
case PORTTRANSMIT_TRANSMIT_RSTP:
NewState(port, transitions, state, PORTTRANSMIT_IDLE);
break;
      default:
        T_E("%s: Entering illegal state %d (%s)", STM_NAME, state, stm_statename(state));
        VTSS_ABORT();
    }
    return state;
    }

const port_stm_t PortTransmit_stm = {
    .name = STM_NAME,
    .begin = stm_begin,
    .run = stm_run,
    .statename = stm_statename,
};

