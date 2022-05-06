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

#define STM_NAME "BridgeDetection"

enum BridgeDetection_states {
BRIDGEDETECTION_BEGIN,
BRIDGEDETECTION_EDGE,
BRIDGEDETECTION_NOT_EDGE};

static const char *stm_statename(int state)
{
switch(state) {
case BRIDGEDETECTION_BEGIN: return "BEGIN";
case BRIDGEDETECTION_EDGE: return "EDGE";
case BRIDGEDETECTION_NOT_EDGE: return "NOT_EDGE";
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
case BRIDGEDETECTION_EDGE:
        cist->operEdge = TRUE;
break;
case BRIDGEDETECTION_NOT_EDGE:
        cist->operEdge = FALSE;
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

if(AdminEdge(cist) ) return stm_enter(port, NULL, BRIDGEDETECTION_EDGE);
if(!AdminEdge(cist) ) return stm_enter(port, NULL, BRIDGEDETECTION_NOT_EDGE);
    T_E("%s: Unable to transition from BEGIN?", STM_NAME);
    VTSS_ABORT();
    return -1;
}

static int stm_run(mstp_port_t *port, uint *transitions, int state)
{
    uint loop_count = 0;
    mstp_cistport_t *cist __attribute__ ((unused)) = getCist(port);
VTSS_ASSERT(cist == port->cistport);
mstp_bridge_t *bridge __attribute__ ((unused)) = cist->bridge;
switch(state) {
BRIDGEDETECTION_EDGE_ENTER : __attribute__ ((unused))
case BRIDGEDETECTION_EDGE:
if((!portEnabled(cist) && !AdminEdge(cist)) || !cist->operEdge ) NewState(port, transitions, state, BRIDGEDETECTION_NOT_EDGE);
break;
BRIDGEDETECTION_NOT_EDGE_ENTER : __attribute__ ((unused))
case BRIDGEDETECTION_NOT_EDGE:
if((!portEnabled(cist) && AdminEdge(cist)) || ((cist->edgeDelayWhile == 0) && AutoEdge(cist) && cist->sendRSTP && port->proposing) ) NewState(port, transitions, state, BRIDGEDETECTION_EDGE);
break;
      default:
        T_E("%s: Entering illegal state %d (%s)", STM_NAME, state, stm_statename(state));
        VTSS_ABORT();
    }
    return state;
    }

const port_stm_t BridgeDetection_stm = {
    .name = STM_NAME,
    .begin = stm_begin,
    .run = stm_run,
    .statename = stm_statename,
};

