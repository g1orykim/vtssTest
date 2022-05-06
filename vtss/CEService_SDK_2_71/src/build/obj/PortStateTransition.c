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

#define STM_NAME "PortStateTransition"

enum PortStateTransition_states {
PORTSTATETRANSITION_BEGIN,
PORTSTATETRANSITION_DISCARDING,
PORTSTATETRANSITION_LEARNING,
PORTSTATETRANSITION_FORWARDING};

static const char *stm_statename(int state)
{
switch(state) {
case PORTSTATETRANSITION_BEGIN: return "BEGIN";
case PORTSTATETRANSITION_DISCARDING: return "DISCARDING";
case PORTSTATETRANSITION_LEARNING: return "LEARNING";
case PORTSTATETRANSITION_FORWARDING: return "FORWARDING";
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
case PORTSTATETRANSITION_DISCARDING:
        disableLearning(port); port->learning = FALSE;
        disableForwarding(port); port->forwarding = FALSE;
break;
case PORTSTATETRANSITION_LEARNING:
        enableLearning(port); port->learning = TRUE;
break;
case PORTSTATETRANSITION_FORWARDING:
        enableForwarding(port); port->forwarding = TRUE;
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

return stm_enter(port, NULL, PORTSTATETRANSITION_DISCARDING);
}

static int stm_run(mstp_port_t *port, uint *transitions, int state)
{
    uint loop_count = 0;
    mstp_cistport_t *cist __attribute__ ((unused)) = port->cistport;
mstp_bridge_t *bridge __attribute__ ((unused)) = cist->bridge;
struct mstp_tree *tree __attribute__ ((unused)) = port->tree;
switch(state) {
PORTSTATETRANSITION_DISCARDING_ENTER : __attribute__ ((unused))
case PORTSTATETRANSITION_DISCARDING:
if(port->learn ) NewState(port, transitions, state, PORTSTATETRANSITION_LEARNING);
break;
PORTSTATETRANSITION_LEARNING_ENTER : __attribute__ ((unused))
case PORTSTATETRANSITION_LEARNING:
if(port->forward ) NewState(port, transitions, state, PORTSTATETRANSITION_FORWARDING);
if(!port->learn ) NewState(port, transitions, state, PORTSTATETRANSITION_DISCARDING);
break;
PORTSTATETRANSITION_FORWARDING_ENTER : __attribute__ ((unused))
case PORTSTATETRANSITION_FORWARDING:
if(!port->forward ) NewState(port, transitions, state, PORTSTATETRANSITION_DISCARDING);
break;
      default:
        T_E("%s: Entering illegal state %d (%s)", STM_NAME, state, stm_statename(state));
        VTSS_ABORT();
    }
    return state;
    }

const port_stm_t PortStateTransition_stm = {
    .name = STM_NAME,
    .begin = stm_begin,
    .run = stm_run,
    .statename = stm_statename,
};

