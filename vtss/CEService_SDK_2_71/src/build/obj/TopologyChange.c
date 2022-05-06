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

#define CistCond(x) (isCistPort(port) && x)
#define STM_NAME "TopologyChange"

enum TopologyChange_states {
TOPOLOGYCHANGE_BEGIN,
TOPOLOGYCHANGE_INACTIVE,
TOPOLOGYCHANGE_LEARNING,
TOPOLOGYCHANGE_DETECTED,
TOPOLOGYCHANGE_ACTIVE,
TOPOLOGYCHANGE_ACKNOWLEDGED,
TOPOLOGYCHANGE_PROPAGATING,
TOPOLOGYCHANGE_NOTIFIED_TC,
TOPOLOGYCHANGE_NOTIFIED_TCN};

static const char *stm_statename(int state)
{
switch(state) {
case TOPOLOGYCHANGE_BEGIN: return "BEGIN";
case TOPOLOGYCHANGE_INACTIVE: return "INACTIVE";
case TOPOLOGYCHANGE_LEARNING: return "LEARNING";
case TOPOLOGYCHANGE_DETECTED: return "DETECTED";
case TOPOLOGYCHANGE_ACTIVE: return "ACTIVE";
case TOPOLOGYCHANGE_ACKNOWLEDGED: return "ACKNOWLEDGED";
case TOPOLOGYCHANGE_PROPAGATING: return "PROPAGATING";
case TOPOLOGYCHANGE_NOTIFIED_TC: return "NOTIFIED_TC";
case TOPOLOGYCHANGE_NOTIFIED_TCN: return "NOTIFIED_TCN";
default:
        return "-unknown-";
}
}

static int stm_enter(mstp_port_t *port, uint *transitions, int state)
{
    mstp_cistport_t *cist __attribute__ ((unused)) = port->cistport;
mstp_bridge_t *bridge __attribute__ ((unused)) = cist->bridge;
struct mstp_tree *tree __attribute__ ((unused)) = port->tree;
T_D("%s: Port %d[%d] - Enter %s", STM_NAME, port->port_no, tree->msti, stm_statename(state)); 
    switch(state) {
case TOPOLOGYCHANGE_INACTIVE:
        fdbFlush(port);
        port->tcWhile = 0; 
        if(isCistPort(port)) { cist->tcAck = FALSE; }
break;
case TOPOLOGYCHANGE_LEARNING:
        if(isCistPort(port)) { port->rcvdTc = cist->rcvdTcn = cist->rcvdTcAck = FALSE; }
        port->rcvdTc = port->tcProp = FALSE;
break;
case TOPOLOGYCHANGE_DETECTED:
        newTcWhile(port); setTcPropTree(port);
        newInfoXst(port,TRUE);
break;
case TOPOLOGYCHANGE_ACKNOWLEDGED:
        port->tcWhile = 0; cist->rcvdTcAck = FALSE;
break;
case TOPOLOGYCHANGE_PROPAGATING:
        newTcWhile(port); fdbFlush(port);
        port->tcProp = FALSE;
break;
case TOPOLOGYCHANGE_NOTIFIED_TC:
        if(isCistPort(port)) { cist->rcvdTcn = FALSE; }
        port->rcvdTc = FALSE;
        if (isCistPort(port) && port->role == DesignatedPort) cist->tcAck = TRUE;
        setTcPropTree(port);
break;
case TOPOLOGYCHANGE_NOTIFIED_TCN:
        newTcWhile(port);
break;
default:;
}
	if(transitions)
(*transitions)++
;return state;}

static int stm_begin(mstp_port_t *port)
{
    mstp_cistport_t *cist __attribute__ ((unused)) = port->cistport;
mstp_bridge_t *bridge __attribute__ ((unused)) = cist->bridge;
struct mstp_tree *tree __attribute__ ((unused)) = port->tree;

return stm_enter(port, NULL, TOPOLOGYCHANGE_INACTIVE);
}

static int stm_run(mstp_port_t *port, uint *transitions, int state)
{
    uint loop_count = 0;
    mstp_cistport_t *cist __attribute__ ((unused)) = port->cistport;
mstp_bridge_t *bridge __attribute__ ((unused)) = cist->bridge;
struct mstp_tree *tree __attribute__ ((unused)) = port->tree;
switch(state) {
TOPOLOGYCHANGE_INACTIVE_ENTER : __attribute__ ((unused))
case TOPOLOGYCHANGE_INACTIVE:
if(port->learn ) NewState(port, transitions, state, TOPOLOGYCHANGE_LEARNING);
break;
TOPOLOGYCHANGE_LEARNING_ENTER : __attribute__ ((unused))
case TOPOLOGYCHANGE_LEARNING:
if(((port->role == RootPort) || (port->role == DesignatedPort) || (port->role == MasterPort)) && port->forward && !cist->operEdge ) NewState(port, transitions, state, TOPOLOGYCHANGE_DETECTED);
if((port->role != RootPort) && (port->role != DesignatedPort) && (port->role != MasterPort) && !(port->learn || port->learning) && !(port->rcvdTc || CistCond(cist->rcvdTcn) || CistCond(cist->rcvdTcAck) || port->tcProp) ) NewState(port, transitions, state, TOPOLOGYCHANGE_INACTIVE);
if(port->rcvdTc || CistCond(cist->rcvdTcn) || CistCond(cist->rcvdTcAck) || port->tcProp ) NewState(port, transitions, state, TOPOLOGYCHANGE_LEARNING);
break;
TOPOLOGYCHANGE_DETECTED_ENTER : __attribute__ ((unused))
case TOPOLOGYCHANGE_DETECTED:
NewState(port, transitions, state, TOPOLOGYCHANGE_ACTIVE);
break;
TOPOLOGYCHANGE_ACTIVE_ENTER : __attribute__ ((unused))
case TOPOLOGYCHANGE_ACTIVE:
LOOP_PROTECT(state, 10);
if(((port->role != RootPort) && (port->role != DesignatedPort) && (port->role != MasterPort)) || cist->operEdge ) NewState(port, transitions, state, TOPOLOGYCHANGE_LEARNING);
if(CistCond(cist->rcvdTcAck) ) NewState(port, transitions, state, TOPOLOGYCHANGE_ACKNOWLEDGED);
if(port->tcProp && !cist->operEdge ) NewState(port, transitions, state, TOPOLOGYCHANGE_PROPAGATING);
if(port->rcvdTc ) NewState(port, transitions, state, TOPOLOGYCHANGE_NOTIFIED_TC);
if(CistCond(cist->rcvdTcn) ) NewState(port, transitions, state, TOPOLOGYCHANGE_NOTIFIED_TCN);
break;
TOPOLOGYCHANGE_ACKNOWLEDGED_ENTER : __attribute__ ((unused))
case TOPOLOGYCHANGE_ACKNOWLEDGED:
NewState(port, transitions, state, TOPOLOGYCHANGE_ACTIVE);
break;
TOPOLOGYCHANGE_PROPAGATING_ENTER : __attribute__ ((unused))
case TOPOLOGYCHANGE_PROPAGATING:
NewState(port, transitions, state, TOPOLOGYCHANGE_ACTIVE);
break;
TOPOLOGYCHANGE_NOTIFIED_TC_ENTER : __attribute__ ((unused))
case TOPOLOGYCHANGE_NOTIFIED_TC:
NewState(port, transitions, state, TOPOLOGYCHANGE_ACTIVE);
break;
TOPOLOGYCHANGE_NOTIFIED_TCN_ENTER : __attribute__ ((unused))
case TOPOLOGYCHANGE_NOTIFIED_TCN:
NewState(port, transitions, state, TOPOLOGYCHANGE_NOTIFIED_TC);
break;
      default:
        T_E("%s: Entering illegal state %d (%s)", STM_NAME, state, stm_statename(state));
        VTSS_ABORT();
    }
    return state;
    }

const port_stm_t TopologyChange_stm = {
    .name = STM_NAME,
    .begin = stm_begin,
    .run = stm_run,
    .statename = stm_statename,
};

