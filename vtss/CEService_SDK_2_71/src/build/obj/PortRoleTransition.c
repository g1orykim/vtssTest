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

#define STM_NAME "PortRoleTransition"

enum PortRoleTransition_states {
PORTROLETRANSITION_BEGIN,
PORTROLETRANSITION_INIT_PORT,
PORTROLETRANSITION_DISABLE_PORT,
PORTROLETRANSITION_DISABLED_PORT,
PORTROLETRANSITION_MASTER_PORT,
PORTROLETRANSITION_MASTER_PROPOSED,
PORTROLETRANSITION_MASTER_AGREED,
PORTROLETRANSITION_MASTER_SYNCED,
PORTROLETRANSITION_MASTER_RETIRED,
PORTROLETRANSITION_MASTER_DISCARD,
PORTROLETRANSITION_MASTER_LEARN,
PORTROLETRANSITION_MASTER_FORWARD,
PORTROLETRANSITION_ROOT_PORT,
PORTROLETRANSITION_ROOT_PROPOSED,
PORTROLETRANSITION_ROOT_AGREED,
PORTROLETRANSITION_ROOT_SYNCED,
PORTROLETRANSITION_REROOT,
PORTROLETRANSITION_REROOTED,
PORTROLETRANSITION_ROOT_LEARN,
PORTROLETRANSITION_ROOT_FORWARD,
PORTROLETRANSITION_DESIGNATED_PORT,
PORTROLETRANSITION_DESIGNATED_PROPOSE,
PORTROLETRANSITION_DESIGNATED_AGREED,
PORTROLETRANSITION_DESIGNATED_SYNCED,
PORTROLETRANSITION_DESIGNATED_RETIRED,
PORTROLETRANSITION_DESIGNATED_DISCARD,
PORTROLETRANSITION_DESIGNATED_LEARN,
PORTROLETRANSITION_DESIGNATED_FORWARD,
PORTROLETRANSITION_ALTERNATE_PORT,
PORTROLETRANSITION_ALTERNATE_PROPOSED,
PORTROLETRANSITION_ALTERNATE_AGREED,
PORTROLETRANSITION_BACKUP_PORT,
PORTROLETRANSITION_BLOCK_PORT};

static const char *stm_statename(int state)
{
switch(state) {
case PORTROLETRANSITION_BEGIN: return "BEGIN";
case PORTROLETRANSITION_INIT_PORT: return "INIT_PORT";
case PORTROLETRANSITION_DISABLE_PORT: return "DISABLE_PORT";
case PORTROLETRANSITION_DISABLED_PORT: return "DISABLED_PORT";
case PORTROLETRANSITION_MASTER_PORT: return "MASTER_PORT";
case PORTROLETRANSITION_MASTER_PROPOSED: return "MASTER_PROPOSED";
case PORTROLETRANSITION_MASTER_AGREED: return "MASTER_AGREED";
case PORTROLETRANSITION_MASTER_SYNCED: return "MASTER_SYNCED";
case PORTROLETRANSITION_MASTER_RETIRED: return "MASTER_RETIRED";
case PORTROLETRANSITION_MASTER_DISCARD: return "MASTER_DISCARD";
case PORTROLETRANSITION_MASTER_LEARN: return "MASTER_LEARN";
case PORTROLETRANSITION_MASTER_FORWARD: return "MASTER_FORWARD";
case PORTROLETRANSITION_ROOT_PORT: return "ROOT_PORT";
case PORTROLETRANSITION_ROOT_PROPOSED: return "ROOT_PROPOSED";
case PORTROLETRANSITION_ROOT_AGREED: return "ROOT_AGREED";
case PORTROLETRANSITION_ROOT_SYNCED: return "ROOT_SYNCED";
case PORTROLETRANSITION_REROOT: return "REROOT";
case PORTROLETRANSITION_REROOTED: return "REROOTED";
case PORTROLETRANSITION_ROOT_LEARN: return "ROOT_LEARN";
case PORTROLETRANSITION_ROOT_FORWARD: return "ROOT_FORWARD";
case PORTROLETRANSITION_DESIGNATED_PORT: return "DESIGNATED_PORT";
case PORTROLETRANSITION_DESIGNATED_PROPOSE: return "DESIGNATED_PROPOSE";
case PORTROLETRANSITION_DESIGNATED_AGREED: return "DESIGNATED_AGREED";
case PORTROLETRANSITION_DESIGNATED_SYNCED: return "DESIGNATED_SYNCED";
case PORTROLETRANSITION_DESIGNATED_RETIRED: return "DESIGNATED_RETIRED";
case PORTROLETRANSITION_DESIGNATED_DISCARD: return "DESIGNATED_DISCARD";
case PORTROLETRANSITION_DESIGNATED_LEARN: return "DESIGNATED_LEARN";
case PORTROLETRANSITION_DESIGNATED_FORWARD: return "DESIGNATED_FORWARD";
case PORTROLETRANSITION_ALTERNATE_PORT: return "ALTERNATE_PORT";
case PORTROLETRANSITION_ALTERNATE_PROPOSED: return "ALTERNATE_PROPOSED";
case PORTROLETRANSITION_ALTERNATE_AGREED: return "ALTERNATE_AGREED";
case PORTROLETRANSITION_BACKUP_PORT: return "BACKUP_PORT";
case PORTROLETRANSITION_BLOCK_PORT: return "BLOCK_PORT";
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
case PORTROLETRANSITION_INIT_PORT:
        port->role = DisabledPort;     
        port->learn = port->forward = FALSE;   
        port->synced = FALSE;   
        port->sync = port->reRoot = TRUE;    
        port->rrWhile = FwdDelay(port);       
        port->fdWhile = MaxAge(port);
        port->rbWhile = 0;                
break;
case PORTROLETRANSITION_DISABLE_PORT:
        port->role = port->selectedRole;
        port->learn = port->forward = FALSE;
break;
case PORTROLETRANSITION_DISABLED_PORT:
        port->fdWhile = MaxAge(port);
        port->synced = TRUE; port->rrWhile = 0;
        port->sync = port->reRoot = FALSE;
break;
case PORTROLETRANSITION_MASTER_PORT:
        port->role = MasterPort;
break;
case PORTROLETRANSITION_MASTER_PROPOSED:
        setSyncTree(tree);
        port->proposed = FALSE;
break;
case PORTROLETRANSITION_MASTER_AGREED:
        port->proposed = port->sync = FALSE;
        port->agree = TRUE;
break;
case PORTROLETRANSITION_MASTER_SYNCED:
        port->rrWhile = 0;
        port->synced = TRUE;
        port->sync = FALSE;
break;
case PORTROLETRANSITION_MASTER_RETIRED:
        port->reRoot = FALSE;
break;
case PORTROLETRANSITION_MASTER_DISCARD:
        port->learn = port->forward = port->disputed = FALSE;
        port->fdWhile = forwardDelay(port);
break;
case PORTROLETRANSITION_MASTER_LEARN:
        port->learn = TRUE;
        port->fdWhile = forwardDelay(port);
break;
case PORTROLETRANSITION_MASTER_FORWARD:
        port->forward = TRUE; port->fdWhile = 0;
        port->agreed = cist->sendRSTP;
break;
case PORTROLETRANSITION_ROOT_PORT:
        port->role = RootPort;        
        port->rrWhile = FwdDelay(port);
break;
case PORTROLETRANSITION_ROOT_PROPOSED:
        setSyncTree(tree);
        port->proposed = FALSE;
break;
case PORTROLETRANSITION_ROOT_AGREED:
        port->proposed = port->sync = FALSE;
        port->agree = TRUE;
        newInfoXst(port,TRUE);
break;
case PORTROLETRANSITION_ROOT_SYNCED:
        port->synced = TRUE;
        port->sync = FALSE;
break;
case PORTROLETRANSITION_REROOT:
        setReRootTree(tree);
break;
case PORTROLETRANSITION_REROOTED:
        port->reRoot = FALSE;
break;
case PORTROLETRANSITION_ROOT_LEARN:
        port->fdWhile = forwardDelay(port);
        port->learn = TRUE;
break;
case PORTROLETRANSITION_ROOT_FORWARD:
        port->fdWhile = 0;
        port->forward = TRUE;
break;
case PORTROLETRANSITION_DESIGNATED_PORT:
        port->role = DesignatedPort;
break;
case PORTROLETRANSITION_DESIGNATED_PROPOSE:
        port->proposing = TRUE;
        if(isCistPort(port)) { cist->edgeDelayWhile = EdgeDelay(port); }
        newInfoXst(port,TRUE);
break;
case PORTROLETRANSITION_DESIGNATED_AGREED:
        port->proposed = port->sync = FALSE;
        port->agree = TRUE;
        newInfoXst(port,TRUE);
break;
case PORTROLETRANSITION_DESIGNATED_SYNCED:
        port->rrWhile = 0; port->synced = TRUE;
        port->sync = FALSE;
break;
case PORTROLETRANSITION_DESIGNATED_RETIRED:
        port->reRoot = FALSE;
break;
case PORTROLETRANSITION_DESIGNATED_DISCARD:
        port->learn = port->forward = port->disputed = FALSE;
        port->fdWhile = forwardDelay(port);
break;
case PORTROLETRANSITION_DESIGNATED_LEARN:
        port->learn = TRUE;
        port->fdWhile = forwardDelay(port);
break;
case PORTROLETRANSITION_DESIGNATED_FORWARD:
        port->forward = TRUE; port->fdWhile = 0;
        port->agreed = cist->sendRSTP;
break;
case PORTROLETRANSITION_ALTERNATE_PORT:
        port->fdWhile = forwardDelay(port); port->synced = TRUE; port->rrWhile = 0; port->sync = port->reRoot = FALSE;
break;
case PORTROLETRANSITION_ALTERNATE_PROPOSED:
        setSyncTree(tree);
        port->proposed = FALSE;
break;
case PORTROLETRANSITION_ALTERNATE_AGREED:
        port->proposed = FALSE;
        port->agree = TRUE;
        newInfoXst(port,TRUE);
break;
case PORTROLETRANSITION_BACKUP_PORT:
        port->rbWhile = (timer)(2*HelloTime(port));
break;
case PORTROLETRANSITION_BLOCK_PORT:
        port->role = port->selectedRole;
        port->learn = port->forward = FALSE;
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

return stm_enter(port, NULL, PORTROLETRANSITION_INIT_PORT);
}

static int stm_run(mstp_port_t *port, uint *transitions, int state)
{
    uint loop_count = 0;
    mstp_cistport_t *cist __attribute__ ((unused)) = port->cistport;
mstp_bridge_t *bridge __attribute__ ((unused)) = cist->bridge;
struct mstp_tree *tree __attribute__ ((unused)) = port->tree;
switch(state) {
PORTROLETRANSITION_INIT_PORT_ENTER : __attribute__ ((unused))
case PORTROLETRANSITION_INIT_PORT:
NewState(port, transitions, state, PORTROLETRANSITION_DISABLE_PORT);
break;
PORTROLETRANSITION_DISABLE_PORT_ENTER : __attribute__ ((unused))
case PORTROLETRANSITION_DISABLE_PORT:
if( (port->selected && !port->updtInfo)) { /* DEFAULT GUARD */
if(!port->learning && !port->forwarding ) NewState(port, transitions, state, PORTROLETRANSITION_DISABLED_PORT);
}
break;
PORTROLETRANSITION_DISABLED_PORT_ENTER : __attribute__ ((unused))
case PORTROLETRANSITION_DISABLED_PORT:
if( (port->selected && !port->updtInfo)) { /* DEFAULT GUARD */
if((port->fdWhile != MaxAge(port)) || port->sync || port->reRoot || !port->synced ) NewState(port, transitions, state, PORTROLETRANSITION_DISABLED_PORT);
}
break;
PORTROLETRANSITION_MASTER_PORT_ENTER : __attribute__ ((unused))
case PORTROLETRANSITION_MASTER_PORT:
LOOP_PROTECT(state, 10);
if( (port->selected && !port->updtInfo)) { /* DEFAULT GUARD */
if(port->proposed && !port->agree ) NewState(port, transitions, state, PORTROLETRANSITION_MASTER_PROPOSED);
if((!port->agree && allSynced(tree, port)) || (port->agree && port->proposed) ) NewState(port, transitions, state, PORTROLETRANSITION_MASTER_AGREED);
if((!port->synced && (port->agreed || cist->operEdge || (!port->learning && !port->forwarding))) || (port->synced && port->sync) ) NewState(port, transitions, state, PORTROLETRANSITION_MASTER_SYNCED);
if(port->reRoot && (port->rrWhile == 0) ) NewState(port, transitions, state, PORTROLETRANSITION_MASTER_RETIRED);
if(((port->sync && !port->synced) || (port->reRoot && (port->rrWhile != 0)) || port->disputed) && !cist->operEdge && (port->learn || port->forward) ) NewState(port, transitions, state, PORTROLETRANSITION_MASTER_DISCARD);
if(((port->fdWhile == 0) || allSynced(tree, port)) && !port->learn ) NewState(port, transitions, state, PORTROLETRANSITION_MASTER_LEARN);
if(((port->fdWhile == 0) || allSynced(tree, port)) && (port->learn && !port->forward) ) NewState(port, transitions, state, PORTROLETRANSITION_MASTER_FORWARD);
}
break;
PORTROLETRANSITION_MASTER_PROPOSED_ENTER : __attribute__ ((unused))
case PORTROLETRANSITION_MASTER_PROPOSED:
NewState(port, transitions, state, PORTROLETRANSITION_MASTER_PORT);
break;
PORTROLETRANSITION_MASTER_AGREED_ENTER : __attribute__ ((unused))
case PORTROLETRANSITION_MASTER_AGREED:
NewState(port, transitions, state, PORTROLETRANSITION_MASTER_PORT);
break;
PORTROLETRANSITION_MASTER_SYNCED_ENTER : __attribute__ ((unused))
case PORTROLETRANSITION_MASTER_SYNCED:
NewState(port, transitions, state, PORTROLETRANSITION_MASTER_PORT);
break;
PORTROLETRANSITION_MASTER_RETIRED_ENTER : __attribute__ ((unused))
case PORTROLETRANSITION_MASTER_RETIRED:
NewState(port, transitions, state, PORTROLETRANSITION_MASTER_PORT);
break;
PORTROLETRANSITION_MASTER_DISCARD_ENTER : __attribute__ ((unused))
case PORTROLETRANSITION_MASTER_DISCARD:
NewState(port, transitions, state, PORTROLETRANSITION_MASTER_PORT);
break;
PORTROLETRANSITION_MASTER_LEARN_ENTER : __attribute__ ((unused))
case PORTROLETRANSITION_MASTER_LEARN:
NewState(port, transitions, state, PORTROLETRANSITION_MASTER_PORT);
break;
PORTROLETRANSITION_MASTER_FORWARD_ENTER : __attribute__ ((unused))
case PORTROLETRANSITION_MASTER_FORWARD:
NewState(port, transitions, state, PORTROLETRANSITION_MASTER_PORT);
break;
PORTROLETRANSITION_ROOT_PORT_ENTER : __attribute__ ((unused))
case PORTROLETRANSITION_ROOT_PORT:
LOOP_PROTECT(state, 10);
if( (port->selected && !port->updtInfo)) { /* DEFAULT GUARD */
if(port->proposed && !port->agree ) NewState(port, transitions, state, PORTROLETRANSITION_ROOT_PROPOSED);
if((!port->agree && allSynced(tree, port)) || (port->agree && port->proposed) ) NewState(port, transitions, state, PORTROLETRANSITION_ROOT_AGREED);
if((port->agreed && !port->synced) || (port->sync && port->synced) ) NewState(port, transitions, state, PORTROLETRANSITION_ROOT_SYNCED);
if(!port->forward && !port->reRoot ) NewState(port, transitions, state, PORTROLETRANSITION_REROOT);
if(port->rrWhile != FwdDelay(port) ) NewState(port, transitions, state, PORTROLETRANSITION_ROOT_PORT);
if(port->reRoot && port->forward ) NewState(port, transitions, state, PORTROLETRANSITION_REROOTED);
if(!port->learn && ((port->fdWhile == 0) || (rstpVersion(bridge) && (port->rbWhile == 0) && (reRooted(port)))) ) NewState(port, transitions, state, PORTROLETRANSITION_ROOT_LEARN);
if(port->learn && !port->forward && ((port->fdWhile == 0) || (rstpVersion(bridge) && (port->rbWhile == 0) && (reRooted(port)))) ) NewState(port, transitions, state, PORTROLETRANSITION_ROOT_FORWARD);
}
break;
PORTROLETRANSITION_ROOT_PROPOSED_ENTER : __attribute__ ((unused))
case PORTROLETRANSITION_ROOT_PROPOSED:
NewState(port, transitions, state, PORTROLETRANSITION_ROOT_PORT);
break;
PORTROLETRANSITION_ROOT_AGREED_ENTER : __attribute__ ((unused))
case PORTROLETRANSITION_ROOT_AGREED:
NewState(port, transitions, state, PORTROLETRANSITION_ROOT_PORT);
break;
PORTROLETRANSITION_ROOT_SYNCED_ENTER : __attribute__ ((unused))
case PORTROLETRANSITION_ROOT_SYNCED:
NewState(port, transitions, state, PORTROLETRANSITION_ROOT_PORT);
break;
PORTROLETRANSITION_REROOT_ENTER : __attribute__ ((unused))
case PORTROLETRANSITION_REROOT:
NewState(port, transitions, state, PORTROLETRANSITION_ROOT_PORT);
break;
PORTROLETRANSITION_REROOTED_ENTER : __attribute__ ((unused))
case PORTROLETRANSITION_REROOTED:
NewState(port, transitions, state, PORTROLETRANSITION_ROOT_PORT);
break;
PORTROLETRANSITION_ROOT_LEARN_ENTER : __attribute__ ((unused))
case PORTROLETRANSITION_ROOT_LEARN:
NewState(port, transitions, state, PORTROLETRANSITION_ROOT_PORT);
break;
PORTROLETRANSITION_ROOT_FORWARD_ENTER : __attribute__ ((unused))
case PORTROLETRANSITION_ROOT_FORWARD:
NewState(port, transitions, state, PORTROLETRANSITION_ROOT_PORT);
break;
PORTROLETRANSITION_DESIGNATED_PORT_ENTER : __attribute__ ((unused))
case PORTROLETRANSITION_DESIGNATED_PORT:
LOOP_PROTECT(state, 10);
if( (port->selected && !port->updtInfo)) { /* DEFAULT GUARD */
if(!port->forward && !port->agreed && !port->proposing && !cist->operEdge ) NewState(port, transitions, state, PORTROLETRANSITION_DESIGNATED_PROPOSE);
if((port->proposed || !port->agree) && allSynced(tree, port) ) NewState(port, transitions, state, PORTROLETRANSITION_DESIGNATED_AGREED);
if((!port->learning && !port->forwarding && !port->synced) || (port->agreed && !port->synced) || (cist->operEdge && !port->synced) || (port->sync && port->synced) ) NewState(port, transitions, state, PORTROLETRANSITION_DESIGNATED_SYNCED);
if(port->reRoot && (port->rrWhile == 0) ) NewState(port, transitions, state, PORTROLETRANSITION_DESIGNATED_RETIRED);
if(((port->sync && !port->synced) || (port->reRoot && (port->rrWhile != 0)) || port->disputed) && !cist->operEdge && (port->learn || port->forward) ) NewState(port, transitions, state, PORTROLETRANSITION_DESIGNATED_DISCARD);
if(((port->fdWhile == 0) || port->agreed || cist->operEdge) && ((port->rrWhile == 0) || !port->reRoot) && !port->sync && !port->learn ) NewState(port, transitions, state, PORTROLETRANSITION_DESIGNATED_LEARN);
if(((port->fdWhile == 0) || port->agreed || cist->operEdge) && ((port->rrWhile == 0) || !port->reRoot) && !port->sync && (port->learn && !port->forward) ) NewState(port, transitions, state, PORTROLETRANSITION_DESIGNATED_FORWARD);
}
break;
PORTROLETRANSITION_DESIGNATED_PROPOSE_ENTER : __attribute__ ((unused))
case PORTROLETRANSITION_DESIGNATED_PROPOSE:
NewState(port, transitions, state, PORTROLETRANSITION_DESIGNATED_PORT);
break;
PORTROLETRANSITION_DESIGNATED_AGREED_ENTER : __attribute__ ((unused))
case PORTROLETRANSITION_DESIGNATED_AGREED:
NewState(port, transitions, state, PORTROLETRANSITION_DESIGNATED_PORT);
break;
PORTROLETRANSITION_DESIGNATED_SYNCED_ENTER : __attribute__ ((unused))
case PORTROLETRANSITION_DESIGNATED_SYNCED:
NewState(port, transitions, state, PORTROLETRANSITION_DESIGNATED_PORT);
break;
PORTROLETRANSITION_DESIGNATED_RETIRED_ENTER : __attribute__ ((unused))
case PORTROLETRANSITION_DESIGNATED_RETIRED:
NewState(port, transitions, state, PORTROLETRANSITION_DESIGNATED_PORT);
break;
PORTROLETRANSITION_DESIGNATED_DISCARD_ENTER : __attribute__ ((unused))
case PORTROLETRANSITION_DESIGNATED_DISCARD:
NewState(port, transitions, state, PORTROLETRANSITION_DESIGNATED_PORT);
break;
PORTROLETRANSITION_DESIGNATED_LEARN_ENTER : __attribute__ ((unused))
case PORTROLETRANSITION_DESIGNATED_LEARN:
NewState(port, transitions, state, PORTROLETRANSITION_DESIGNATED_PORT);
break;
PORTROLETRANSITION_DESIGNATED_FORWARD_ENTER : __attribute__ ((unused))
case PORTROLETRANSITION_DESIGNATED_FORWARD:
NewState(port, transitions, state, PORTROLETRANSITION_DESIGNATED_PORT);
break;
PORTROLETRANSITION_ALTERNATE_PORT_ENTER : __attribute__ ((unused))
case PORTROLETRANSITION_ALTERNATE_PORT:
LOOP_PROTECT(state, 10);
if( (port->selected && !port->updtInfo)) { /* DEFAULT GUARD */
if(port->proposed && !port->agree ) NewState(port, transitions, state, PORTROLETRANSITION_ALTERNATE_PROPOSED);
if((!port->agree && allSynced(tree, port)) || (port->agree && port->proposed) ) NewState(port, transitions, state, PORTROLETRANSITION_ALTERNATE_AGREED);
if((port->fdWhile != forwardDelay(port)) || port->sync || port->reRoot || !port->synced ) NewState(port, transitions, state, PORTROLETRANSITION_ALTERNATE_PORT);
if((port->rbWhile != (timer)(2*HelloTime(port))) && (port->role == BackupPort) ) NewState(port, transitions, state, PORTROLETRANSITION_BACKUP_PORT);
}
break;
PORTROLETRANSITION_ALTERNATE_PROPOSED_ENTER : __attribute__ ((unused))
case PORTROLETRANSITION_ALTERNATE_PROPOSED:
NewState(port, transitions, state, PORTROLETRANSITION_ALTERNATE_PORT);
break;
PORTROLETRANSITION_ALTERNATE_AGREED_ENTER : __attribute__ ((unused))
case PORTROLETRANSITION_ALTERNATE_AGREED:
NewState(port, transitions, state, PORTROLETRANSITION_ALTERNATE_PORT);
break;
PORTROLETRANSITION_BACKUP_PORT_ENTER : __attribute__ ((unused))
case PORTROLETRANSITION_BACKUP_PORT:
NewState(port, transitions, state, PORTROLETRANSITION_ALTERNATE_PORT);
break;
PORTROLETRANSITION_BLOCK_PORT_ENTER : __attribute__ ((unused))
case PORTROLETRANSITION_BLOCK_PORT:
if( (port->selected && !port->updtInfo)) { /* DEFAULT GUARD */
if(!port->learning && !port->forwarding ) NewState(port, transitions, state, PORTROLETRANSITION_ALTERNATE_PORT);
}
break;
      default:
        T_E("%s: Entering illegal state %d (%s)", STM_NAME, state, stm_statename(state));
        VTSS_ABORT();
    }
	/* Global Transitions */
if( (port->selected && !port->updtInfo)) { /* DEFAULT GUARD */
if((port->selectedRole == DisabledPort) && (port->role != port->selectedRole) )
 return stm_enter(port, transitions, PORTROLETRANSITION_DISABLE_PORT);
if((port->selectedRole == MasterPort) && (port->role != port->selectedRole) )
 return stm_enter(port, transitions, PORTROLETRANSITION_MASTER_PORT);
if((port->selectedRole == RootPort) && (port->role != port->selectedRole) )
 return stm_enter(port, transitions, PORTROLETRANSITION_ROOT_PORT);
if((port->selectedRole == DesignatedPort) && (port->role != port->selectedRole) )
 return stm_enter(port, transitions, PORTROLETRANSITION_DESIGNATED_PORT);
if(((port->selectedRole == AlternatePort) || (port->selectedRole == BackupPort)) && (port->role != port->selectedRole) )
 return stm_enter(port, transitions, PORTROLETRANSITION_BLOCK_PORT);
}
    return state;
    }

const port_stm_t PortRoleTransition_stm = {
    .name = STM_NAME,
    .begin = stm_begin,
    .run = stm_run,
    .statename = stm_statename,
};

