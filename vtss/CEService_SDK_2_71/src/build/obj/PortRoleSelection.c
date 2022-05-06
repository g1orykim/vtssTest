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

#define STM_NAME "PortRoleSelection"

enum PortRoleSelection_states {
PORTROLESELECTION_BEGIN,
PORTROLESELECTION_INIT_BRIDGE,
PORTROLESELECTION_ROLE_SELECTION};

static const char *stm_statename(int state)
{
switch(state) {
case PORTROLESELECTION_BEGIN: return "BEGIN";
case PORTROLESELECTION_INIT_BRIDGE: return "INIT_BRIDGE";
case PORTROLESELECTION_ROLE_SELECTION: return "ROLE_SELECTION";
default:
        return "-unknown-";
}
}

static int stm_enter(struct mstp_tree *bridge, uint *transitions, int state)
{
    T_D("%s: MSTI%d, Enter %s", STM_NAME, bridge->msti, stm_statename(state)); 
    switch(state) {
case PORTROLESELECTION_INIT_BRIDGE:
        updtRoleDisabledTree(bridge);
break;
case PORTROLESELECTION_ROLE_SELECTION:
        clearReselectTree(bridge);
        updtRolesTree(bridge);
        setSelectedTree(bridge);
break;
default:;
}
	if(transitions)
(*transitions)++
;return state;}

static int stm_begin(struct mstp_tree *bridge)
{
    
return stm_enter(bridge, NULL, PORTROLESELECTION_INIT_BRIDGE);
}

static int stm_run(struct mstp_tree *bridge, uint *transitions, int state)
{
    uint loop_count = 0;
    switch(state) {
PORTROLESELECTION_INIT_BRIDGE_ENTER : __attribute__ ((unused))
case PORTROLESELECTION_INIT_BRIDGE:
NewState(bridge, transitions, state, PORTROLESELECTION_ROLE_SELECTION);
break;
PORTROLESELECTION_ROLE_SELECTION_ENTER : __attribute__ ((unused))
case PORTROLESELECTION_ROLE_SELECTION:
if(anyReSelect(bridge) ) NewState(bridge, transitions, state, PORTROLESELECTION_INIT_BRIDGE);
break;
      default:
        T_E("%s: Entering illegal state %d (%s)", STM_NAME, state, stm_statename(state));
        VTSS_ABORT();
    }
    return state;
    }

const bridge_stm_t PortRoleSelection_stm = {
    .name = STM_NAME,
    .begin = stm_begin,
    .run = stm_run,
    .statename = stm_statename,
};

