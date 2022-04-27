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

#define STM_NAME "PortInformation"

enum PortInformation_states {
PORTINFORMATION_BEGIN,
PORTINFORMATION_DISABLED,
PORTINFORMATION_AGED,
PORTINFORMATION_UPDATE,
PORTINFORMATION_CURRENT,
PORTINFORMATION_RECEIVE,
PORTINFORMATION_OTHER,
PORTINFORMATION_NOT_DESIGNATED,
PORTINFORMATION_INFERIOR_DESIGNATED,
PORTINFORMATION_REPEATED_DESIGNATED,
PORTINFORMATION_SUPERIOR_DESIGNATED};

static const char *stm_statename(int state)
{
switch(state) {
case PORTINFORMATION_BEGIN: return "BEGIN";
case PORTINFORMATION_DISABLED: return "DISABLED";
case PORTINFORMATION_AGED: return "AGED";
case PORTINFORMATION_UPDATE: return "UPDATE";
case PORTINFORMATION_CURRENT: return "CURRENT";
case PORTINFORMATION_RECEIVE: return "RECEIVE";
case PORTINFORMATION_OTHER: return "OTHER";
case PORTINFORMATION_NOT_DESIGNATED: return "NOT_DESIGNATED";
case PORTINFORMATION_INFERIOR_DESIGNATED: return "INFERIOR_DESIGNATED";
case PORTINFORMATION_REPEATED_DESIGNATED: return "REPEATED_DESIGNATED";
case PORTINFORMATION_SUPERIOR_DESIGNATED: return "SUPERIOR_DESIGNATED";
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
case PORTINFORMATION_DISABLED:
        port->rcvdMsg = FALSE;
        port->proposing = port->proposed = port->agree = port->agreed = FALSE;
        port->rcvdInfoWhile = 0;
        port->infoIs = Disabled; port->reselect = TRUE; port->selected = FALSE;
break;
case PORTINFORMATION_AGED:
        port->infoIs = Aged;
        port->reselect = TRUE; port->selected = FALSE;
break;
case PORTINFORMATION_UPDATE:
        port->proposing = port->proposed = FALSE;
        port->agreed = port->agreed && betterorsameInfo(port, Mine);
        port->synced = port->synced && port->agreed; port->portPriority = port->designatedPriority;
        port->portTimes = port->designatedTimes;
        port->updtInfo = FALSE; port->infoIs = Mine; newInfoXst(port,TRUE);
break;
case PORTINFORMATION_RECEIVE:
        port->rcvdInfo = rcvInfo(port);
        recordMastered(port);
break;
case PORTINFORMATION_OTHER:
        port->reselect = TRUE; port->selected = FALSE;
        port->rcvdMsg = FALSE;
break;
case PORTINFORMATION_NOT_DESIGNATED:
        recordAgreement(port); setTcFlags(port);
        port->rcvdMsg = FALSE;
break;
case PORTINFORMATION_INFERIOR_DESIGNATED:
        port->reselect = TRUE; port->selected = FALSE;
        recordDispute(port);
        port->rcvdMsg = FALSE;
break;
case PORTINFORMATION_REPEATED_DESIGNATED:
        cist->infoInternal = cist->rcvdInternal;
        recordProposal(port); setTcFlags(port);
        recordAgreement(port);
        updtRcvdInfoWhile(port);
        port->rcvdMsg = FALSE;
break;
case PORTINFORMATION_SUPERIOR_DESIGNATED:
        cist->infoInternal = cist->rcvdInternal;
        port->agreed = port->proposing = FALSE;
        recordProposal(port); setTcFlags(port);
        port->agree = port->agree && betterorsameInfo(port, Received);
        recordAgreement(port); port->synced = port->synced && port->agreed;
        recordPriority(port); recordTimes(port);
        updtRcvdInfoWhile(port);
        port->infoIs = Received; port->reselect = TRUE; port->selected = FALSE;
        port->rcvdMsg = FALSE;
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

return stm_enter(port, NULL, PORTINFORMATION_DISABLED);
}

static int stm_run(mstp_port_t *port, uint *transitions, int state)
{
    uint loop_count = 0;
    mstp_cistport_t *cist __attribute__ ((unused)) = port->cistport;
mstp_bridge_t *bridge __attribute__ ((unused)) = cist->bridge;
struct mstp_tree *tree __attribute__ ((unused)) = port->tree;
switch(state) {
PORTINFORMATION_DISABLED_ENTER : __attribute__ ((unused))
case PORTINFORMATION_DISABLED:
if(port->rcvdMsg ) NewState(port, transitions, state, PORTINFORMATION_DISABLED);
if(portEnabled(cist) ) NewState(port, transitions, state, PORTINFORMATION_AGED);
break;
PORTINFORMATION_AGED_ENTER : __attribute__ ((unused))
case PORTINFORMATION_AGED:
if((port->selected && port->updtInfo) ) NewState(port, transitions, state, PORTINFORMATION_UPDATE);
break;
PORTINFORMATION_UPDATE_ENTER : __attribute__ ((unused))
case PORTINFORMATION_UPDATE:
NewState(port, transitions, state, PORTINFORMATION_CURRENT);
break;
PORTINFORMATION_CURRENT_ENTER : __attribute__ ((unused))
case PORTINFORMATION_CURRENT:
LOOP_PROTECT(state, 10);
if((port->selected && port->updtInfo) ) NewState(port, transitions, state, PORTINFORMATION_UPDATE);
if((port->infoIs == Received) && (port->rcvdInfoWhile == 0) && !port->updtInfo && !rcvdXstMsg(port) ) NewState(port, transitions, state, PORTINFORMATION_AGED);
if(rcvdXstMsg(port) && !updtXstInfo(port) ) NewState(port, transitions, state, PORTINFORMATION_RECEIVE);
break;
PORTINFORMATION_RECEIVE_ENTER : __attribute__ ((unused))
case PORTINFORMATION_RECEIVE:
if(port->rcvdInfo == OtherInfo ) NewState(port, transitions, state, PORTINFORMATION_OTHER);
if(port->rcvdInfo == InferiorRootAlternateInfo ) NewState(port, transitions, state, PORTINFORMATION_NOT_DESIGNATED);
if(port->rcvdInfo == InferiorDesignatedInfo ) NewState(port, transitions, state, PORTINFORMATION_INFERIOR_DESIGNATED);
if(port->rcvdInfo == RepeatedDesignatedInfo ) NewState(port, transitions, state, PORTINFORMATION_REPEATED_DESIGNATED);
if(port->rcvdInfo == SuperiorDesignatedInfo ) NewState(port, transitions, state, PORTINFORMATION_SUPERIOR_DESIGNATED);
break;
PORTINFORMATION_OTHER_ENTER : __attribute__ ((unused))
case PORTINFORMATION_OTHER:
NewState(port, transitions, state, PORTINFORMATION_CURRENT);
break;
PORTINFORMATION_NOT_DESIGNATED_ENTER : __attribute__ ((unused))
case PORTINFORMATION_NOT_DESIGNATED:
NewState(port, transitions, state, PORTINFORMATION_CURRENT);
break;
PORTINFORMATION_INFERIOR_DESIGNATED_ENTER : __attribute__ ((unused))
case PORTINFORMATION_INFERIOR_DESIGNATED:
NewState(port, transitions, state, PORTINFORMATION_CURRENT);
break;
PORTINFORMATION_REPEATED_DESIGNATED_ENTER : __attribute__ ((unused))
case PORTINFORMATION_REPEATED_DESIGNATED:
NewState(port, transitions, state, PORTINFORMATION_CURRENT);
break;
PORTINFORMATION_SUPERIOR_DESIGNATED_ENTER : __attribute__ ((unused))
case PORTINFORMATION_SUPERIOR_DESIGNATED:
NewState(port, transitions, state, PORTINFORMATION_CURRENT);
break;
      default:
        T_E("%s: Entering illegal state %d (%s)", STM_NAME, state, stm_statename(state));
        VTSS_ABORT();
    }
	/* Global Transitions */
if((!portEnabled(cist) && (port->infoIs != Disabled)) )
 return stm_enter(port, transitions, PORTINFORMATION_DISABLED);
    return state;
    }

const port_stm_t PortInformation_stm = {
    .name = STM_NAME,
    .begin = stm_begin,
    .run = stm_run,
    .statename = stm_statename,
};

