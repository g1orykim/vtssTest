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

#include <signal.h>
#include <unistd.h>

#include "mstp_priv.h"
#include "mstp_misc.h"
#include "mstp_stm.h"
#include "mstp_util.h"

static const port_stm_t * const cist_stms[N_CIST_STM] = {
    &PortReceive_stm,
    &PortProtocolMigration_stm,
    &BridgeDetection_stm,
    &PortTransmit_stm,
};

static const port_stm_t * const msti_stms[N_MSTI_STM] = {
    &PortInformation_stm,
    &PortRoleTransition_stm,
    &PortStateTransition_stm,
    &TopologyChange_stm,
};

static const bridge_stm_t * const tree_stms[N_TREE_STM] = {
    &PortRoleSelection_stm,
};

/*
 * Private functions
 */
static void*
zmalloc(size_t size)
{
    void *p = vtss_mstp_malloc(size);
    if(p)
        memset(p, 0, size);
    return p;
}

static void
strncpyz(char *dst, const char *src, size_t maxlen)
{
    (void) strncpy(dst, src, maxlen);
    dst[maxlen] = '\0'; /* Explicit null terminated/truncated */
}

static void
dispose_ports(mstp_tree_t *tree)
{
    mstp_port_t *port;
    for(port = tree->portlist; port != NULL; ) {
        mstp_port_t *next = port->next; /* Getnext */
        /* Zap from index */
        tree->portmap[port->port_no-1] = NULL;
        /* Block port */
        vtss_mstp_port_setstate(port->port_no, tree->msti, 
                                MSTP_FWDSTATE_BLOCKING);
        /* Dismiss current */
        vtss_mstp_free(port);
        port = next;            /* Advance */
    }
    tree->portlist = NULL;
}

static void
initialize_port(mstp_port_t *port,
                mstp_tree_t *tree,
                mstp_cistport_t *cist,
                uint portnum)
{
    mstp_msti_port_param_t *mparam = vtss_mstp_get_msti_port_block(tree->bridge, tree->msti, portnum);
    VTSS_ASSERT(mparam != NULL);
    port->port_no = portnum;
    port->msti = tree->msti;
    port->tree = tree;
    port->cistport = cist;
    port->adminPathCost = mparam->adminPathCost;
    port->adminPortPriority = mparam->adminPortPriority;
    port->portId.bytes[0] = (u8) (port->adminPortPriority | (port->port_no >> 8));
    port->portId.bytes[1] = (u8) (port->port_no & 0xff);
}

static mstp_port_t*
add_tport(mstp_tree_t *tree,
          mstp_cistport_t *cist,
          uint portnum)
{
    mstp_port_t     *tport;
    
    if((tport = zmalloc(sizeof(*tport))) == NULL)
        return NULL;

    /* Initialize tree port */
    initialize_port(tport, tree, cist, portnum);

    /* Inherit from CIST state */
    if(cist->linkEnabled) {
        tport->portPathCost = calc_pathcost(tport->adminPathCost, cist->linkspeed);
        mstp_update_point2point(tport, cist, cist->fdx); /* Update derived state */
    }

    /* Establish MSTI in MSTI portmap */
    tree->portmap[portnum-1] = tport;
    
    /* Establish in portlist */
    tport->next = tree->portlist; /* Add current list */
    tree->portlist = tport;       /* This in front */

    return tport;
}

static int
vtss_mstp_mac2str(void *buffer, 
                  size_t size,
                  const u8 *mac)
{
    return snprintf(buffer, size,
                    "%02X:%02X:%02X:%02X:%02X:%02X", 
                    mac[0], mac[1], 
                    mac[2], mac[3],
                    mac[4], mac[5]);
}

static void
mstp_run_stm_tree(mstp_tree_t *tree, uint *transitions)
{
    size_t i;
    mstp_port_t *port;
    tree->topologyChange = FALSE;
    /* Run bridge STM(s) */
    for(i = 0; i < ARR_SZ(tree_stms); i++) {
        const bridge_stm_t *stm = tree_stms[i];
        int new_state;
        if((new_state = stm->run(tree, transitions, tree->state[i])) != tree->state[i]) {
            T_D("Run - bridge %d stm %s, state %s => %s", tree->msti,
                stm->name, stm->statename(tree->state[i]), stm->statename(new_state));
            tree->state[i] = new_state;
        }
    }
    ForAllPorts(tree, port) {
        /* Run port STM(s) */
        for(i = 0; i < ARR_SZ(msti_stms); i++) {
            const port_stm_t *stm = msti_stms[i];
            int new_state;
            if((new_state = stm->run(port, transitions, port->state[i])) != port->state[i]) {
                T_D("Run - MSTI %d[%d] stm %s, state %s => %s", port->port_no, tree->msti,
                    stm->name, stm->statename(port->state[i]), stm->statename(new_state));
                port->state[i] = new_state;
            }
        }
        if(port->tcWhile > 0)
            tree->topologyChange = TRUE;
    }
}

static void
mstp_run_stm(mstp_bridge_t *mstp)
{
    if(mstp->lock) {
        T_D("%s - locked level %d", __FUNCTION__, mstp->lock);
    } else {
        uint transitions, loops = 0;
#define MAX_LOOPS 10
        do {
            uint i;
            transitions = 0;
            /* Run CIST port STM(s) */
            for(i = 0; i < ARR_SZ(cist_stms); i++) {
                const port_stm_t *stm = cist_stms[i];
                mstp_port_t *port;
                ForAllCistPorts(mstp, port) {
                    int new_state;
                    mstp_cistport_t *cist = getCist(port);
                    if((new_state = stm->run(port, &transitions, cist->state[i])) != cist->state[i]) {
                        T_D("Run - cistport %d stm %s, state %s => %s", port->port_no,
                            stm->name, stm->statename(cist->state[i]), stm->statename(new_state));
                        cist->state[i] = new_state;
                    }
                }
            }
            ForAllTrees(mstp, itree, {
                    mstp_run_stm_tree(itree, &transitions);
                });
        } while(transitions && (loops++ < MAX_LOOPS));
        if(transitions) {
            T_I("%s: Throttling, %u transitions, %d loops", __FUNCTION__, transitions, loops);
        }
        /* Update lastTopologyChange, topologyChangeCount */
        ForAllTrees(mstp, itree, {
                if(itree->topologyChange) {  /* Any ports in with active tcWhile? */
                    itree->lastTopologyChange = vtss_mstp_current_time();
                    itree->topologyChangeCount++;
                }
            });
    }
}

static void
cist_stm_begin(mstp_port_t *port)
{
    size_t i;
    for(i = 0; i < ARR_SZ(cist_stms); i++) {
        const port_stm_t *stm = cist_stms[i];
        mstp_cistport_t *cist = getCist(port);
        cist->state[i] = stm->begin(port);
        T_D("Begin - cistport %d stm %s, state %s", 
            port->port_no, stm->name, stm->statename(cist->state[i]));
    }
}

static void
port_stm_begin(mstp_port_t *port)
{
    size_t i;
    port->resetTime = vtss_mstp_current_time();
    for(i = 0; i < ARR_SZ(msti_stms); i++) {
        const port_stm_t *stm = msti_stms[i];
        port->state[i] = stm->begin(port);
        T_D("Begin - tport %d[%d] stm %s, state %s", 
            port->port_no, port->msti, 
            stm->name, stm->statename(port->state[i]));
    }
}

static void
tree_stm_begin(mstp_tree_t *tree)
{
    size_t i;
    for(i = 0; i < ARR_SZ(tree_stms); i++) {
        const bridge_stm_t *stm = tree_stms[i];
        tree->state[i] = stm->begin(tree);
        T_D("Begin - MST%d stm %s, state %s", tree->msti, 
            stm->name, stm->statename(tree->state[i]));
    }
    /* Begin MSTI ports */
    mstp_port_t *port;
    ForAllPorts(tree, port) {
        port_stm_begin(port);
    }
}

static void
bridge_stm_begin(mstp_bridge_t *bridge)
{
    mstp_port_t *port;
    /* Begin CIST ports */
    ForAllCistPorts(bridge, port) {
        cist_stm_begin(port);
    }
    /* Begin MSTI's */
    ForAllTrees(bridge, tree, {
            tree_stm_begin(tree);
        });
}

static void
mstp_init_tree(mstp_bridge_t *bridge, 
               u8 msti, 
               u16 MSTID,
               mstp_tree_t *tree, 
               const mstp_macaddr_t *bridge_id)
{
    tree->msti = msti;
    tree->bridge = bridge;
    /* Initalize BridgeIdentifier */
    tree->BridgeIdentifier.bridgeIdPrio[0] = 
        bridge->conf.bridgePriority[msti] | /* Configured priority */
        ((MSTID >> 8) & 0xF);   /* Top 4 MSTID bits */
    tree->BridgeIdentifier.bridgeIdPrio[1] = (u8) MSTID; /* Low 8 MSTID bits */
    memcpy(tree->BridgeIdentifier.bridgeAddress, bridge_id, sizeof(*bridge_id));
    /* Initalize BridgePriority */
    if(tree->msti == MSTID_CIST)
        tree->BridgePriority.rootBridgeId = tree->BridgeIdentifier;
    tree->BridgePriority.regRootBridgeId = tree->BridgeIdentifier;
    tree->BridgePriority.DesignatedBridgeId = tree->BridgeIdentifier;
    /* Instance mapping */
    tree->msti = msti;
    tree->MSTID = MSTID;
    bridge->trees[msti] = tree;
}

/**
 * 12.12.1.2 Create MSTI
 *
 * Purpose: To create a new MSTI and its associated state machines and
 * parameters, and to add its MSTID to the MSTI List.
 *
 * \return TRUE if the operation succeeded, FALSE otherwise. Failure
 * will occur if the \e MSTID is invalid, out of range, exceeds the
 * MSTI capacity (\e N_MSTI_MAX), if the MSTID is already added or in
 * case of memory allocation failure.
 *
 * \param mstp The MSTP instance data.
 *
 * \param msti the tree instance index. Must be between 1 and 8.
 *
 * \param MSTID The MSTID of the MSTI to create.
 *
 * \note This function used to be an API functions, but is now only
 * used internally.
 */
static bool
vtss_mstp_create_msti(mstp_bridge_t *mstp, 
                      uint msti,
                      u16 MSTID)
{
    mstp_tree_t *tree;
    if((MSTID == MSTID_CIST || MSTID > MSTID_MAX) ||
       (tree = mstp_get_tree_by_id(mstp, MSTID)) != NULL || 
       msti >= N_MSTI_MAX ||
       mstp_get_instance(mstp, msti) ||
       (tree = zmalloc(sizeof(*tree))) == NULL)
        return FALSE;           /* Entry taken/no room/no memory */

    /* Add portmap */
    tree->portmap = zmalloc(mstp->n_ports*sizeof(mstp_port_t *));
    if(tree->portmap) {
        mstp_init_tree(mstp, msti, MSTID, tree, &mstp->bridge_id);
        tree_stm_begin(tree);
    } else {
        vtss_mstp_free(tree);
        return FALSE;
    }

    return TRUE;
}

/**
 * 12.12.1.3 Delete MSTI
 *
 * Purpose: To delete an existing MSTI and its associated state
 * machines and parameters, and to remove its MSTID from the MSTI
 * List.
 *
 * \return TRUE if the operation succeeded, FALSE otherwise. Failure
 * will occur if the \e MSTID is invalid, out of range, or if the
 * MSTID has not been added.
 *
 * \param mstp The MSTP instance data.
 *
 * \param msti the tree instance index. Must be between 1 and 8.
 *
 * \note This function used to be an API functions, but is now only
 * used internally.
 */
static bool
vtss_mstp_delete_msti(mstp_bridge_t *mstp, 
                      uint msti)
{
    mstp_tree_t *tree;
    if((msti == MSTID_CIST || msti >= N_MSTI_MAX) ||
       (tree = mstp_get_instance(mstp, msti)) == NULL)
        return FALSE;

    /* Zap all MSTI ports in tree */
    dispose_ports(tree);

    /* Zap tree from bridge list */
    mstp->trees[msti] = NULL;

    /* Dealloc tree memory */
    vtss_mstp_free(tree->portmap);
    vtss_mstp_free(tree);

    return TRUE;
}

static void PortFree(mstp_tree_t *tree, mstp_port_t *port)
{
    /* Now remove from tree portlist */
    if(port == tree->portlist) {
        tree->portlist = port->next; /* Head of list - just bypass */
    } else {
        mstp_port_t *p = tree->portlist;
        while(p && p->next != port) /* Forward until we have it */
            p = p->next;
        VTSS_ASSERT(p != NULL);
        VTSS_ASSERT(p->next == port); /* We've found it */
        p->next = port->next;         /* Hook out port  */
    }

    /* Make sure the port is blocked */
    if (port->cistport->linkEnabled) {
        vtss_mstp_port_setstate(port->port_no, tree->msti, MSTP_FWDSTATE_BLOCKING);
    }

    /* Finally, deallocate */
    tree->portmap[port->port_no-1] = NULL;
    vtss_mstp_free(port);
}

static bool
apply_mapping(mstp_bridge_t *mstp, 
              mstp_map_t *map)
{
    /* First and last index must be zero */
    if(map->map[0] != 0 || map->map[ARR_SZ(map->map)-1] != 0)
        return FALSE;

    uint i;
    bool in_use[N_MSTI_MAX];
    memset(in_use, 0, sizeof(in_use));

    /* Validate rest */
    for(i = 1; i < ARR_SZ(map->map)-1; i++) {
        u8 msti = map->map[i];
        /* Valid entry ? */
        if(msti && msti >= N_MSTI_MAX)
            return FALSE;       /* Invalid entry */
        in_use[msti] = TRUE;
    }

    /* Synchronize MSTI instances */
    for(i = MSTI_MSTI1; i < N_MSTI_MAX; i++) {
        if(in_use[i] && mstpVersion(mstp)) {
            if(!mstp->trees[i] && 
               !vtss_mstp_create_msti(mstp, i, i)) {
                T_E("Create of MST%d fails", i);
                return FALSE;
            }

            /* 
             * Sync up Existing port add/delete as appropriate
             */
            mstp_tree_t *tree = mstp->trees[i];
            VTSS_ASSERT(tree != NULL);
            mstp_port_t *port;
            ForAllCistPorts(mstp, port) {
                mstp_port_t *tport = get_tport(tree, port->port_no);
                bool member = vtss_mstp_port_member(port->port_no, i);
                if(!tport) {
                    if(member &&
                       (tport = add_tport(tree, port->cistport, port->port_no)) != NULL) {
                        T_I("Added MSTI port %d[%d]", port->port_no, i);
                        port_stm_begin(tport);
                    }
                } else {
                    if(!member) {
                        T_I("Must delete MSTI port %d[%d]", tport->port_no, i);
                        PortFree(tree, tport);
                    }
                }
            }
            /* 
             * In case the active topology changed.
             */
            updtRolesTree(tree);
        } else {
            if(mstp->trees[i] && !vtss_mstp_delete_msti(mstp, i)) {
                T_E("Delete of MST%d fails", i);
                return FALSE;
            }
        }
    }

    /* Churn STM's */
    mstp_run_stm(mstp);

    return TRUE;
}

/*
 * API functions
 */

mstp_bridge_t *
vtss_mstp_create_bridge(const mstp_macaddr_t *bridge_id, 
                        uint n_ports)
{
    mstp_bridge_t *bridge;
    if((bridge = zmalloc(sizeof(*bridge))) != NULL &&
       (bridge->ist.portmap = zmalloc(n_ports*sizeof(mstp_port_t *))) != NULL &&
       (bridge->conf.cist = zmalloc(n_ports*sizeof(mstp_port_param_t))) != NULL && 
       (bridge->conf.msti = zmalloc(N_MSTI_MAX*n_ports*sizeof(mstp_msti_port_param_t))) != NULL) {

        /* Initialize bridge */
        bridge->n_ports = n_ports;
        bridge->bridge_id = *bridge_id;

        uint msti, port;
        bridge->conf.bridge.bridgeMaxAge = 20;
        bridge->conf.bridge.bridgeHelloTime = 2;
        bridge->conf.bridge.bridgeForwardDelay = 15;
        bridge->conf.bridge.forceVersion = MSTP_PROTOCOL_VERSION_MSTP;
        bridge->conf.bridge.txHoldCount = 6;
        bridge->conf.bridge.MaxHops = 20;

        /* Initialize Bridge Times */
        bridge->BridgeTimes.fwdDelay = bridge->conf.bridge.bridgeForwardDelay;
        bridge->BridgeTimes.maxAge = bridge->conf.bridge.bridgeMaxAge;
        bridge->BridgeTimes.remainingHops = bridge->conf.bridge.MaxHops; /* 13.37.3 */

        /* Default MSTI tree priority */
        for(msti = 0; msti < N_MSTI_MAX; msti++)
            bridge->conf.bridgePriority[msti] = 0x80;

        for(port = 1; port <= n_ports; port++) {
            mstp_port_param_t *cparam = vtss_mstp_get_port_block(bridge, port);
            VTSS_ASSERT(cparam != NULL);
            cparam->adminEdgePort = FALSE;
            cparam->adminAutoEdgePort = TRUE;
            cparam->adminPointToPointMAC = P2P_AUTO;
            cparam->restrictedRole = 
                cparam->restrictedTcn = FALSE;
            cparam->bpduGuard = FALSE;

            for(msti = 0; msti < N_MSTI_MAX; msti++) {
                mstp_msti_port_param_t *mparam = vtss_mstp_get_msti_port_block(bridge, msti, port);
                VTSS_ASSERT(mparam != NULL);
                mparam->adminPathCost = MSTP_PORT_PATHCOST_AUTO; /* 0 = Auto */
                mparam->adminPortPriority = 0x80; /* 17.14 - Table 17-2: Default recommended value */
            }
        }

        /* Initialize cfgid */
        bridge->cfgid.cfgid_fmt = 0; /* To be explicit - already set */
        (void) vtss_mstp_mac2str(bridge->cfgid.cfgid_name, 
                                 sizeof(bridge->cfgid.cfgid_name), 
                                 bridge_id->mac);
        mstp_calc_digest(&bridge->mst_config, bridge->cfgid.cfgid_digest);

        mstp_init_tree(bridge, MSTID_CIST, MSTID_CIST, &bridge->ist, &bridge->bridge_id);
        bridge_stm_begin(bridge);
    } else {
        if(bridge) {
            if(bridge->ist.portmap != NULL)
                vtss_mstp_free(bridge->ist.portmap);
            if(bridge->conf.cist != NULL)
                vtss_mstp_free(bridge->conf.cist);
            if(bridge->conf.msti != NULL)
                vtss_mstp_free(bridge->conf.msti);
        }
        vtss_mstp_free(bridge);
        bridge = NULL;
    }
    return bridge;
}

void
vtss_mstp_delete_bridge(mstp_bridge_t *mstp)
{
    T_I("Delete MSTP instance");

    dispose_ports(&mstp->ist);
    vtss_mstp_free(mstp->ist.portmap);

    /* Now remove all trees */
    ForAllMstiTrees(mstp, tree, {
            (void) vtss_mstp_delete_msti(mstp, tree->msti);
        });

    vtss_mstp_free(mstp->conf.cist);
    vtss_mstp_free(mstp->conf.msti);
    vtss_mstp_free(mstp);
}

bool
vtss_mstp_set_mapping(mstp_bridge_t *mstp, 
                      mstp_map_t *map)
{
    bool rc = apply_mapping(mstp, map);
    if(rc) {
        /* We're now using this map */
        mstp->mst_config = *map;

        /* Calculate digest */
        mstp_calc_digest(&mstp->mst_config, mstp->cfgid.cfgid_digest);
    }
    return rc;
}

void
vtss_mstp_set_config_id(mstp_bridge_t *mstp, 
                        const char *name,
                        u16 revision)
{
    memset(mstp->cfgid.cfgid_name, 0, sizeof(mstp->cfgid.cfgid_name));
    strncpy((char *) mstp->cfgid.cfgid_name, name, MSTP_CONFIG_NAME_MAXLEN);
    mstp->cfgid.cfgid_revision[0] = revision >> 8; /* Store msb */
    mstp->cfgid.cfgid_revision[1] = (u8) revision; /* Then lsb */
}

void
vtss_mstp_get_config_id(mstp_bridge_t *mstp, 
                        char name[MSTP_CONFIG_NAME_MAXLEN],
                        u16 *revision,
                        u8 digest[MSTP_DIGEST_LEN])
{
    if(name)
        memcpy(name, mstp->cfgid.cfgid_name, MSTP_CONFIG_NAME_MAXLEN);
    if(revision) {
        u16 r;
        r = mstp->cfgid.cfgid_revision[0] << 8; /* Network order - msb */
        r += mstp->cfgid.cfgid_revision[1];     /* Add lsb */
        *revision = r;
    }
    if(digest)
        memcpy(digest, mstp->cfgid.cfgid_digest, MSTP_DIGEST_LEN);
}

bool
vtss_mstp_get_bridge_status(mstp_bridge_t *mstp,
                            uint msti,
                            mstp_bridge_status_t *status)
{
    mstp_tree_t *tree = mstp_get_instance(mstp, msti);
    if(!tree)
        return FALSE;

    memset(status, 0, sizeof(*status));

    /* Number of ports in bridge (max) */
    status->n_ports = mstp->n_ports;

    /* a) Bridge Identifier - as defined in 17.18.3. */
    memcpy(status->bridgeId, &tree->BridgeIdentifier, sizeof(status->bridgeId));

    /* b) Time Since Topology Change - the count in seconds of the
     * time since the tcWhile timer (17.17.8) for any Port was
     * non-zero.
     */
    status->timeSinceTopologyChange = (tree->lastTopologyChange ? 
                                       vtss_mstp_current_time() - tree->lastTopologyChange :
                                       MSTP_TIMESINCE_NEVER);
    
    /* c) Topology Change Count - the count of times that there has
     * been at least one non-zero tcWhile timer (17.17.8).
     */
    status->topologyChangeCount = tree->topologyChangeCount;

    /* d) Topology Change. Asserted if the tcWhile timer (17.17.8)
     * for any Port is non-zero.
     */
    status->topologyChange = tree->topologyChange;

    if(msti == MSTID_CIST) {
        /* e) Designated Root (17.18.6). */
        memcpy(status->designatedRoot, &tree->rootPriority.rootBridgeId, sizeof(status->designatedRoot));

        /* f) Root Path Cost (17.18.6). */
        status->rootPathCost = unal_ntohl(tree->rootPriority.extRootPathCost.bytes);
    } else {
        /* e) Designated Root (17.18.6). */
        memcpy(status->designatedRoot, &tree->rootPriority.regRootBridgeId, sizeof(status->designatedRoot));

        /* f) Root Path Cost (17.18.6). */
        status->rootPathCost = unal_ntohl(tree->rootPriority.intRootPathCost.bytes);
    }
    
    /* g) Root Port (17.18.6). */
    status->rootPort = (u32) (0xfff & unal_ntohs(tree->rootPortId.bytes));

    /* h) Max Age (17.18.7). */
    status->maxAge = tree->rootTimes.maxAge;

    /* i) Forward Delay (17.13.5). */
    status->forwardDelay = tree->rootTimes.fwdDelay;

    /* j) Bridge Max Age (17.18.4). */
    status->bridgeMaxAge = mstp->BridgeTimes.maxAge;
    
    /* k) Bridge Hello Time (17.18.4). */
    status->bridgeHelloTime = mstp->conf.bridge.bridgeHelloTime;
    
    /* l) Bridge Forward Delay (17.18.4). */
    status->bridgeForwardDelay = mstp->BridgeTimes.fwdDelay;
    
    /* m) TxHoldCount (17.13.12). */
    status->txHoldCount = TxHoldCount(mstp);

    /* n) forceVersion (17.13.4). */
    status->forceVersion = (u8) ForceProtocolVersion(mstp);

    /* CIST? */
    if(msti == MSTID_CIST) {
        /* o) CIST Regional Root Identifier */
        memcpy(status->cistRegionalRoot, &tree->rootPriority.regRootBridgeId, sizeof(status->cistRegionalRoot));
    
        /* p) CIST Path Cost.*/
        status->cistInternalPathCost = unal_ntohl(tree->rootPriority.intRootPathCost.bytes);

        /* q) MaxHops (13.22.1). */
        status->maxHops = mstp->BridgeTimes.remainingHops;
    }

    return TRUE;
}

bool
vtss_mstp_set_bridge_parameters(mstp_bridge_t *mstp,
                                const mstp_bridge_param_t *param)
{
    bool 
        mustInit = FALSE,
        mustUpdateTimes = FALSE,
        resetTxCount = FALSE;  /* 17.13 paragraph 4 */
    Times_t BridgeTimes;

    BridgeTimes.messageAge = 0; /* b) A Message Age value of zero. */

    /* a) Bridge Max Age - the new value (17.18.4). */
    BridgeTimes.maxAge = param->bridgeMaxAge;
            
    /* c) Bridge Forward Delay - the new value (17.18.4). */
    BridgeTimes.fwdDelay = param->bridgeForwardDelay;

    /* g) MaxHops - the new value of MaxHops */
    BridgeTimes.remainingHops = param->MaxHops;

    /* Init if needed */
    if(struct_cmp(mstp->BridgeTimes, !=, BridgeTimes))
        mustUpdateTimes = TRUE;

    /* b) Bridge Hello Time - the new value (17.18.4). */
    mstp->conf.bridge.bridgeHelloTime = param->bridgeHelloTime; /* No-op, really */

    /* d) - see vtss_mstp_set_bridge_priority() */

    /* e) forceVersion - the new value of the Force Protocol Version
     * parameter (17.13.4). 
     */
    if(mstp->conf.bridge.forceVersion != param->forceVersion)
        mustInit = TRUE;        /* 17.13 a) */

    /* f) TxHoldCount - the new value of TxHoldCount (17.13.12). */
    if(TxHoldCount(mstp) != param->txHoldCount)
        resetTxCount = TRUE; /* 17.13 para 4 */

    if(mstp->conf.bridge.bpduGuard != param->bpduGuard) {
        mstp->conf.bridge.bpduGuard = param->bpduGuard;
        /* If bpduGuard is disabled then reset the stpInconsistent.
         * (if non-port-bpduGuard enabled). 
         */
        if(!mstp->conf.bridge.bpduGuard) {
            mstp_port_t *port;
            ForAllCistPorts(mstp, port) {
                mstp_cistport_t *cist = getCist(port);
                if(!cist->bpduGuard && cist->stpInconsistent) {
                    cist->stpInconsistent = FALSE;
                }
            }
        }
    }

    if(mstp->conf.bridge.errorRecoveryDelay != param->errorRecoveryDelay) {
        /* If errorRecoveryDelay timer changed, reset timeout for
         * inconsitent ports - but don't change the state itself. */
        mstp_port_t *port;
        mstp->conf.bridge.errorRecoveryDelay = param->errorRecoveryDelay;
        ForAllCistPorts(mstp, port) {
            mstp_cistport_t *cist = getCist(port);
            if(cist->stpInconsistent)
                cist->errorRecoveryWhile = mstp->conf.bridge.errorRecoveryDelay; 
        }
    }

    /* Store param 'copy' - updates all config params */
    mstp->conf.bridge = *param;
    
    /* Management change handling */
    if(mustInit) {
        (void) apply_mapping(mstp, &mstp->mst_config);
        bridge_stm_begin(mstp);
    }
    if(mustUpdateTimes) {
        mstp->BridgeTimes = BridgeTimes;
        ForAllTrees(mstp, itree, {
                updtRolesTree(itree); /* Force update of portTimes */
            });
    }
    if(resetTxCount) { /* 17.13 para 4 */
        mstp_port_t *port;
        ForAllCistPorts(mstp, port) {
            getCist(port)->txCount = 0; 
        }
    }
    mstp_run_stm(mstp);
    return TRUE;
}

bool
vtss_mstp_set_bridge_priority(mstp_bridge_t *mstp,
                              uint msti,
                              u8 bridgePriority)
{
    if(msti >= N_MSTI_MAX)
        return FALSE;
    
    /* Store param 'copy' */
    mstp->conf.bridgePriority[msti] = bridgePriority;

    mstp_tree_t *tree = mstp_get_instance(mstp, msti);
    if(!tree)
        return FALSE;

    bridgePriority &= 0xF0;
    if(tree->BridgeIdentifier.bridgeIdPrio[0] != bridgePriority) {
        tree->BridgeIdentifier.bridgeIdPrio[0] = bridgePriority;
        
        /* Update BridgePriority */
        if(tree->msti == MSTID_CIST)
            tree->BridgePriority.rootBridgeId = tree->BridgeIdentifier;
        tree->BridgePriority.regRootBridgeId = tree->BridgeIdentifier;
        tree->BridgePriority.DesignatedBridgeId = tree->BridgeIdentifier;

        mstp_port_t *port;
        ForAllPorts(tree, port) {
            port->reselect = TRUE;
            port->selected = FALSE;
        }
        
        mstp_run_stm(mstp);
    }

    return TRUE;
}

bool 
vtss_mstp_add_port(mstp_bridge_t *mstp, 
                   uint portnum)
{
    uint i;
    mstp_port_t *port;

    T_I("Add Port %d, max ports %u", portnum, mstp->n_ports);

    if(!valid_port(mstp->n_ports, portnum) ||
       get_port(mstp, portnum) != NULL ||
       (port = zmalloc(sizeof(mstp_port_t)+sizeof(mstp_cistport_t))) == NULL)
        return FALSE;

    mstp_cistport_t *cist = getCist(port);

    /* Initialize Common parts */
    initialize_port(port, &mstp->ist, cist, portnum);

    /* Initialize CIST port */
    cist->bridge = mstp;

    /* Configure */
    mstp_port_param_t *cparam = vtss_mstp_get_port_block(mstp, portnum);
    VTSS_ASSERT(cparam != NULL);
    cist->adminEdgePort = cparam->adminEdgePort;
    cist->adminAutoEdgePort = cparam->adminAutoEdgePort;
    cist->adminPointToPointMAC = cparam->adminPointToPointMAC;
    cist->restrictedRole = cparam->restrictedRole;
    cist->restrictedTcn = cparam->restrictedTcn;
    cist->bpduGuard = cparam->bpduGuard;

    /* Establish CIST in IST portmap */
    mstp->ist.portmap[portnum-1] = port;
    
    /* Establish in portlist */
    port->next = mstp->ist.portlist; /* Add current list */
    mstp->ist.portlist = port;       /* This in front */

    /* Begin state for CIST port */
    for(i = 0; i < ARR_SZ(cist_stms); i++) {
        const port_stm_t *stm = cist_stms[i];
        cist->state[i] = stm->begin(port);
        T_D("Begin - port %d stm %s, state %s", 
            port->port_no, stm->name, stm->statename(cist->state[i]));
    }

    /* Add instantiated port(s) */
    ForAllMstiTrees(mstp, tree, {
            if(vtss_mstp_port_member(portnum, tree->msti))
                (void) add_tport(tree, cist, portnum);
        });

    /* Begin state for each MSTI port */
    ForAllPortNo(mstp, port->port_no, { 
            port_stm_begin(_tp_);
        });

    /* Churn STM's - as the port is not enabled this may be
     * superfluous, but harmless */
    mstp_run_stm(mstp);

    return TRUE;
}

bool 
vtss_mstp_delete_port(mstp_bridge_t *mstp, 
                      uint portnum)
{
    mstp_port_t *port;

    T_I("Delete Port %d", portnum);

    if(!valid_port(mstp->n_ports, portnum) ||
       (port = get_port(mstp, portnum)) == NULL)
        return FALSE;

    /* Get base CIST port */
    mstp_cistport_t *cist = getCist(port);
    VTSS_ASSERT(cist == port->cistport);

    /* Let port leave bridge by disabling it first (if needed) */
    if(cist->linkEnabled) {
        cist->linkEnabled = FALSE; 
        mstp_run_stm(mstp);
    }

    /* Iterate all trees - unlist, free */
    ForAllPortNo(mstp, portnum, {
            PortFree(_tree_, _tp_);
        });
    
    return TRUE;
}

bool
vtss_mstp_port_added(const mstp_bridge_t *mstp,
                     uint portnum)
{
    return (valid_port(mstp->n_ports, portnum) &&
            get_port(mstp, portnum) != NULL);
}

bool
vtss_mstp_reinit_port(mstp_bridge_t *mstp, 
                      uint portnum)
{
    mstp_port_t *port;

    if(!valid_port(mstp->n_ports, portnum) ||
       (port = get_port(mstp, portnum)) == NULL)
        return FALSE;

    /* Note that the port was reset */
    port->resetTime = vtss_mstp_current_time();

    /* Init CIST */
    cist_stm_begin(port);

    /* Initialize port STM's */
    port_stm_begin(port);

    /* Churn STM's */
    mstp_run_stm(mstp);

    return TRUE;
}

bool
vtss_mstp_port_enable(mstp_bridge_t *mstp,
                      uint portnum,
                      bool enable,
                      u32 linkspeed,
                      bool fdx)
{
    mstp_port_t *port;
    T_I("Port %d enable %d speed %u fdx %d", portnum, enable, linkspeed, fdx);

    if(!valid_port(mstp->n_ports, portnum) ||
       (port = get_port(mstp, portnum)) == NULL)
        return FALSE;

    mstp_cistport_t *cist = port->cistport;
    if(cist->linkEnabled != enable) {
        T_I("Port %d link state change to %sABLE", portnum, enable ? "EN" : "DIS");
        port->resetTime = vtss_mstp_current_time();
        memset(&cist->stat, 0, sizeof(cist->stat));
        cist->linkEnabled = enable;
        /*
         * The Port Information STM needs only portEnabled to trigger
         * updates, so reselect/selected needs no explicit setting.
        */
    }
    if(cist->linkEnabled) {
        mstp_update_linkspeed(mstp, port->port_no, cist, linkspeed);
        mstp_update_point2point(port, cist, fdx);
    } else {
        /*
         * A downed interface always clear stpInconsistent.
         */
        cist->stpInconsistent = FALSE;
    }
    mstp_run_stm(mstp);
    return TRUE;
}

bool
vtss_mstp_get_port_status(mstp_bridge_t *mstp,
                          uint msti,
                          uint portnum,
                          mstp_port_status_t *status)
{
    mstp_tree_t *tree = mstp_get_instance(mstp, msti);
    mstp_port_t *port;

    if(!tree ||
       !valid_port(mstp->n_ports, portnum) ||
       (port = get_tport(tree, portnum)) == NULL)
        return FALSE;

    /* Get base CIST port */
    mstp_cistport_t *cist = port->cistport;
    VTSS_ASSERT(cist != NULL);

    /* a) Uptime  */
    status->uptime = cist->linkEnabled ? vtss_mstp_current_time() - port->resetTime : 0;

    /* b) State */
    status->state = (int) (portEnabled(cist) ? 
                           (port->forwarding ? PORTSTATE_FORWARDING :
                            (port->learning ? PORTSTATE_LEARNING : PORTSTATE_DISCARDING)) : 
                           PORTSTATE_DISABLED);
    strncpyz(status->statestr, rstp_portstate2str(port), sizeof(status->statestr));

    /* c) Port Identifier */
    memcpy(status->portId, port->portId.bytes, sizeof(status->portId));

    /* d) Path Cost */
    status->pathCost = port->portPathCost;

    if(isCistPort(port)) {
        /* e) Designated Root */
        memcpy(status->designatedRoot, &port->designatedPriority.rootBridgeId, sizeof(status->designatedRoot));

        /* f) Designated Cost */
        status->designatedCost = unal_ntohl(port->designatedPriority.extRootPathCost.bytes);
    } else {
        /* e) Designated Root */
        memcpy(status->designatedRoot, &port->designatedPriority.regRootBridgeId, sizeof(status->designatedRoot));

        /* f) Designated Cost */
        status->designatedCost = unal_ntohl(port->designatedPriority.intRootPathCost.bytes);
    }

    /* g) Designated Bridge */
    memcpy(status->designatedBridge, &port->portPriority.DesignatedBridgeId, sizeof(status->designatedBridge));

    /* h) Designated Port */
    memcpy(status->designatedPort, &port->portPriority.DesignatedPortId, sizeof(status->designatedPort));

    /* i) Topology Change Acknowledge. CIST only. */
    status->tcAck = cist->tcAck;

    /* j) Hello Time. */
    status->helloTime = HelloTime(port);

    /* k) adminEdgePort. CIST only. */
    status->adminEdgePort = cist->adminEdgePort;

    /* l) operEdgePort. CIST only. */
    status->operEdgePort = cist->operEdge;

    /* 802.1D-l) autoEdgePort. CIST only. */
    status->autoEdgePort = cist->adminAutoEdgePort;

    /* n) MAC Operational. CIST only. */
    status->macOperational = cist->linkEnabled;

    /* o) adminPointToPointMAC. CIST only. */
    status->adminPointToPointMAC = cist->adminPointToPointMAC;

    /* p) operPointToPointMAC. CIST only. */
    status->operPointToPointMAC = cist->operPointToPointMAC;

    /* q) restrictedRole. CIST only. */
    status->restrictedRole = cist->restrictedRole;

    /* r) restrictedTcn. CIST only. */
    status->restrictedTcn = cist->restrictedTcn;

    /* s) Port Role */
    strncpyz(status->rolestr, rstp_portrole2str(port->role), sizeof(status->rolestr));

    /* t) disputed */
    status->disputed = port->disputed;

    return TRUE;
}

static void inline copy_vector(mstp_bridge_vector_t *dst, const PriorityVector_t *src)
{
    memcpy(dst->rootBridgeId, src->rootBridgeId.bridgeIdPrio, 2);
    memcpy(dst->rootBridgeId+2, src->rootBridgeId.bridgeAddress, 6);
    dst->extRootPathCost = unal_ntohl(src->extRootPathCost.bytes);
    memcpy(dst->regRootBridgeId, src->regRootBridgeId.bridgeIdPrio, 2);
    memcpy(dst->regRootBridgeId+2, src->regRootBridgeId.bridgeAddress, 6);
    dst->intRootPathCost = unal_ntohl(src->intRootPathCost.bytes);
    memcpy(dst->DesignatedBridgeId, src->DesignatedBridgeId.bridgeIdPrio, 2);
    memcpy(dst->DesignatedBridgeId+2, src->DesignatedBridgeId.bridgeAddress, 6);
    memcpy(dst->DesignatedPortId, src->DesignatedPortId.bytes, 2);
}

bool
vtss_mstp_get_port_vectors(const mstp_bridge_t *mstp,
                           uint msti,
                           uint portnum,
                           mstp_port_vectors_t *vectors)
{
    mstp_port_t *port;

    if(!valid_port(mstp->n_ports, portnum) ||
       (port = get_port(mstp, portnum)) == NULL)
        return FALSE;

    copy_vector(&vectors->designated, &port->designatedPriority);
    copy_vector(&vectors->port,       &port->portPriority);
    copy_vector(&vectors->message,    &port->msgPriority);

    strncpyz(vectors->infoIs, rstp_info2str(port->infoIs), sizeof(vectors->infoIs));

    return TRUE;
}

bool
vtss_mstp_get_port_statistics(const mstp_bridge_t *mstp,
                              uint portnum,
                              mstp_port_statistics_t *statistics)
{
    mstp_port_t *port;

    if(!valid_port(mstp->n_ports, portnum) ||
       (port = get_port(mstp, portnum)) == NULL)
        return FALSE;

    *statistics = port->cistport->stat;

    return TRUE;
}

bool
vtss_mstp_clear_port_statistics(const mstp_bridge_t *mstp,
                                uint portnum)
{
    mstp_port_t *port;

    if(!valid_port(mstp->n_ports, portnum) ||
       (port = get_port(mstp, portnum)) == NULL)
        return FALSE;

    memset(&port->cistport->stat, 0, sizeof(port->cistport->stat));

    return TRUE;
}

bool
vtss_mstp_set_port_parameters(mstp_bridge_t *mstp,
                              uint portnum,
                              const mstp_port_param_t *param)
{
    T_I("Port %d Set Params", portnum);
    
    mstp_port_param_t *cparam = vtss_mstp_get_port_block(mstp, portnum);
    if(!cparam)
        return FALSE;

    /* Store param 'copy' */
    *cparam = *param;

    /* Apply to active port */
    mstp_port_t *port;
    if((port = get_port(mstp, portnum)) == NULL)
        return TRUE;            /* Port not active, just return */

    /* Get base CIST port */
    mstp_cistport_t *cist = port->cistport;
    VTSS_ASSERT(cist != NULL);

    /* Copy settings - CIST */
    cist->adminEdgePort = param->adminEdgePort;
    cist->adminAutoEdgePort = param->adminAutoEdgePort;
    cist->adminPointToPointMAC = param->adminPointToPointMAC;
    cist->restrictedRole = param->restrictedRole;
    cist->restrictedTcn = param->restrictedTcn;
    cist->bpduGuard = param->bpduGuard;
    if(!cist->bpduGuard && cist->stpInconsistent) {
        cist->stpInconsistent = FALSE; /* Clear inconsistent state if active */
    }

    /* a) Port Number - the number of the Bridge Port. */
    // Nothing to do - used to locate the port

    /* d) adminEdgePort - the new value of the adminEdgePort parameter
     * (17.13.1). Present in implementations that support the
     * identification of edge ports.
     */
    // Nothing to do - STM's use cist->adminEdgePort

    /* e) autoEdgePort - the new value of the autoEdgePort parameter
     * (17.13.3). Optional and provided only by RSTP Bridges that
     * support the automatic identification of edge ports.
     */
    // Nothing to do - STM's use cist->adminAutoEdgePort

    /* NB: The Bridge Detection Machine *only* use AdminEdge when the
     * port is disabled and in the BEGIN state. To allow changing
     * *explicitly* operEdge state by changing AdminEdge, we add this
     * test. (Otherwise you'ld have to disable/reenable port to change
     * edge state (and not uing auto).
     */
    if(!cist->adminAutoEdgePort) {
        /* Explictly configured edge state */
        cist->operEdge = cist->adminEdgePort;
    }

    /* g) adminPointToPointMAC - the new value of the
     * adminPointToPointMAC parameter (6.4.3). May be present if the
     * implementation supports the adminPointToPointMAC parameter.
     */
    // Code uses cist->param.adminPointToPointMAC
    mstp_update_point2point(port, cist, cist->fdx); /* Update derived state */

    /* Be sure to trigger new port role selection */
    ForAllPortNo(mstp, port->port_no, {
            _tp_->reselect = TRUE;
            _tp_->selected = FALSE;
        });
    
    /* Churn STM's */
    mstp_run_stm(mstp);

    return TRUE;
}

bool
vtss_mstp_set_msti_port_parameters(mstp_bridge_t *mstp,
                                   uint msti,
                                   uint portnum,
                                   const mstp_msti_port_param_t *param)
{
    T_I("Port %d[%d] Set Params", portnum, msti);
    
    mstp_msti_port_param_t *mparam = vtss_mstp_get_msti_port_block(mstp, msti, portnum);
    if(!mparam)
        return FALSE;

    /* Store param 'copy' */
    *mparam = *param;

    /* Apply to active port */
    mstp_tree_t *tree;
    mstp_port_t *port;
    if((tree = mstp_get_instance(mstp, msti)) == NULL ||
       (port = get_tport(tree, portnum)) == NULL)
        return TRUE;            /* Port not active, just return */

    /* Get base CIST port */
    mstp_cistport_t *cist = port->cistport;
    VTSS_ASSERT(cist != NULL);

    /* Copy settings */
    port->adminPathCost = param->adminPathCost;
    port->adminPortPriority = param->adminPortPriority & 0xf0; /* Mask low bits */

    /* c) Path Cost - the new value (17.13.11). */
    mstp_update_linkspeed(mstp, port->port_no, cist, cist->linkspeed); /* Update derived state */

    /* d) Port Priority - the new value of the priority field for the
     * Port Identifier (17.19.21).
     */
    if((port->portId.bytes[0] & 0xf0) != port->adminPortPriority) {
        T_I("Port %d priority change %d -> %d", portnum, 
            port->portId.bytes[0] & 0xf0, port->adminPortPriority);
        port->portId.bytes[0] = (u8) (port->adminPortPriority | (portnum >> 8));
        port->portId.bytes[1] = (u8) (portnum & 0xff);
        port->reselect = TRUE;
        port->selected = FALSE;
    }

    /* Churn STM's */
    mstp_run_stm(mstp);

    return TRUE;
}

bool
vtss_mstp_port_mcheck(const mstp_bridge_t *mstp,
                      uint portnum)
{
    mstp_port_t *port;

    T_I("Port %d mcheck", portnum);
    
    if(valid_port(mstp->n_ports, portnum) &&
       (port = get_port(mstp, portnum)) != NULL &&
       rstpVersion(mstp)) {
        port->cistport->mcheck = TRUE;
        return TRUE;
    }
    return FALSE;
}

void
vtss_mstp_tick(mstp_bridge_t *mstp)
{
#define dec(x)  do { if(x) { x--; } } while(0)
    VTSS_ASSERT(mstp != NULL);
    if(mstp->ist.portlist) {
        mstp_port_t *port;
        T_N("tick");
        ForAllCistPorts(mstp, port) {
            mstp_cistport_t *cist = getCist(port);
            dec(cist->helloWhen);
            dec(cist->mdelayWhile);
            dec(cist->edgeDelayWhile);
            dec(cist->txCount);
            /*
             * Error recovery is non-standard and as such kept out of
             * STM's (which are strict 802.1Q).
             */
            if(cist->stpInconsistent &&
               cist->errorRecoveryWhile) {
                if(--cist->errorRecoveryWhile == 0) { 
                    T_W("STP inconsistent port#%u recovered by timeout", port->port_no);
                    cist->stpInconsistent = FALSE;
                }
            }
            ForAllPortNo(mstp, port->port_no, 
                         {
                             dec(_tp_->rbWhile);
                             dec(_tp_->fdWhile);
                             dec(_tp_->rrWhile);
                             dec(_tp_->tcWhile);
                             dec(_tp_->rcvdInfoWhile);
                         });
        }
        mstp_run_stm(mstp);
    }
#undef dec
}

static void 
getTimes(mstp_cistport_t *cist, 
         const mstp_bpdu_t *bpdu)
{
    cist->rcvBpdu.times.fwdDelay = bpdu_time(bpdu->forwardDelay);
    cist->rcvBpdu.times.maxAge = bpdu_time(bpdu->maxAge);
    cist->rcvBpdu.times.messageAge = bpdu_time(bpdu->messageAge);
    cist->rcvBpdu.times.remainingHops = cist->bridge->conf.bridge.MaxHops; /* Default for STP/RSTP */
}

static void
getStpPrio(mstp_cistport_t *cist, 
           const mstp_bpdu_t *bpdu)
{
    cist->rcvBpdu.priority.rootBridgeId = bpdu->rootBridgeId;
    cist->rcvBpdu.priority.extRootPathCost = bpdu->rootPathCost;
    /*
     * From 802.1D-2005 sect 13.10:
     *
     * NOTE — If a Configuration Message is received in an RST or ST
     * BPDU, both the Regional Root Identifier and the Designated
     * Bridge Identifier are decoded from the single BPDU field used
     * for the Designated Bridge Parameter (the MST BPDU field in this
     * position encodes the CIST Regional Root Identifier).
     */
    cist->rcvBpdu.priority.regRootBridgeId = bpdu->DesignatedBridgeId;
    /* 
     * An STP or RSTP Bridge is always treated by MSTP as being in an
     * MST Region of its own, so the Internal Root Path Cost is
     * decoded as zero, and the tests below become the familiar checks
     * used by STP and RSTP.
     */
    memset(&cist->rcvBpdu.priority.intRootPathCost, 0, sizeof(PathCost_t));
    cist->rcvBpdu.priority.DesignatedBridgeId = bpdu->DesignatedBridgeId;
    cist->rcvBpdu.priority.DesignatedPortId = bpdu->DesignatedPortId;
}
        
static void
getMstpPrio(mstp_cistport_t *cist, 
            const mstp_bpdu_t *bpdu)
{
    cist->rcvBpdu.priority.rootBridgeId = bpdu->rootBridgeId;
    cist->rcvBpdu.priority.extRootPathCost = bpdu->rootPathCost;
    cist->rcvBpdu.priority.regRootBridgeId = bpdu->DesignatedBridgeId;
    /* 
     * If B is not in the same MST Region as D, the Internal Root Path
     * Cost is decoded as 0, as it has no meaning to B. 
     */
    if(cist->rcvBpdu.sameRegion)
        cist->rcvBpdu.priority.intRootPathCost = bpdu->mstp.cistinfo.intRootPathCost;
    // else ... already zero
    cist->rcvBpdu.priority.DesignatedBridgeId = bpdu->mstp.cistinfo.cistBridgeId;
    cist->rcvBpdu.priority.DesignatedPortId = bpdu->DesignatedPortId;
}

void
vtss_mstp_rx(const mstp_bridge_t *mstp,
             uint portnum,
             const void *buffer, 
             size_t size)
{
    mstp_port_t *port;
    mstp_bpdu_t *bpdu = (mstp_bpdu_t *) buffer;
    size_t len8023 = (size_t) unal_ntohs(bpdu->len8023);
    mstp_bpdutype_t decode_as = BPDU_ILL;
    uint mstp_records = 0;
    
    T_D("%s: port %d length %zu - 802.11 len %zu", __FUNCTION__, portnum, size, len8023);

    if(!valid_port(mstp->n_ports, portnum) ||
       (port = get_port(mstp, portnum)) == NULL)
        return;

    /* Get base CIST port */
    mstp_cistport_t *cist = port->cistport;
    VTSS_ASSERT(cist != NULL);

    if(size > (14 + 3) &&        /* MAC + LLC present*/
       len8023 > 3 && len8023 <= 1500 && /* 802.3 valid len */
       size >= (len8023 + 14) && /* 802.3 len and (padded) full len consistent */
       bpdu->dsap == BPDU_L_SAP && /* LLC BPDU HDR */
       bpdu->ssap == BPDU_L_SAP && 
       bpdu->llc == LLC_UI) {
        size_t bpdulen = len8023 - 3; /* Drop LLC encap for 802.1D checks */

        if(!portEnabled(cist)) {
            T_D("Port not enabled");
            return;
        }

        if(cist->rcvdBpdu) {
            T_D("Port port BPDU not consumed yet");
            return;
        }

        /* 802.1Q-2005 14.4 - Validation of received BPDUs
         * 
         * If the Protocol Identifier is 0000 0000 0000 0000, ... 
         */
        if(!(bpdu->protocolIdentifier[0] == (u8) 0 &&
             bpdu->protocolIdentifier[1] == (u8) 0)) {
            T_D("BPDU fails PID checks");
            cist->stat.unknown_frame_recvs++;
            return;
        }

        /* a) ..., the BPDU Type is 0000 0000, and the BPDU contains
         * 35 or more octets, it shall be decoded as an STP
         * Configuration BPDU.
         */
        if (bpdu->bpduType == MSTP_BPDU_TYPE_CONFIG && bpdulen >= 35)
            decode_as = BPDU_CONFIG;
        else if
            /* b) ..., the BPDU Type is 1000 0000 (where bit 8 is
             * shown at the left of the sequence), and the BPDU
             * contains 4 or more octets, it shall be decoded as an
             * STP TCN BPDU (9.3.2 of IEEE Std 802.1D).
             */
            (bpdu->bpduType == MSTP_BPDU_TYPE_TCN && bpdulen >= 4) 
            decode_as = BPDU_TCN;
        else if
            /* c) ..., the Protocol Version Identifier is 2, and the BPDU
             * Type is 0000 0010 (where bit 8 is shown at the left of the
             * sequence), and the BPDU contains 36 or more octets, it
             * shall be decoded as an RST BPDU.
             */
            (bpdu->protocolVersionIdentifier == 2 &&
             bpdu->bpduType == MSTP_BPDU_TYPE_RSTP && 
             bpdulen >= 36)
            decode_as = BPDU_RSTP;
        else if
            /* d) ..., the Protocol Version Identifier is 3 or
             * greater, and the BPDU Type is 0000 0010, */ 
            (bpdu->protocolVersionIdentifier >= 3 &&
             bpdu->bpduType == MSTP_BPDU_TYPE_RSTP) {
            uint mstp_residual = 0;
            uint version3Length = (bpdu->version3Length[0] << 8)+ bpdu->version3Length[1];
            if(bpdulen >= 102 && version3Length >= sizeof(mstp_bpdu_mstpstatic_t)) {
                mstp_residual = version3Length - sizeof(mstp_bpdu_mstpstatic_t);
                mstp_records = mstp_residual/sizeof(mstp_bpdu_mstirec_t);
                mstp_residual %= sizeof(mstp_bpdu_mstirec_t);
            }
            /* and the BPDU:
             *
             *  1) Contains 35 or more but less than *102* octets; or
             *  2) Contains a Version 1 Length that is not 0; or
             *  3) Contains a Version 3 length that does not represent
             *     an integral number, from 0 to 64 inclusive, of MSTI
             *     Configuration Messages;
             *
             * it shall be decoded as an RST BPDU.
             *
             * NB: The 802.1Q has a inconsistency in 14.4 d)-e) - the
             * length the provide MSTP decode is *102* as in e) - not
             * *103* as d) suggests.
             */
            if(bpdulen < 35) 
                decode_as = BPDU_ILL;
            else if(bpdulen < 102 ||
                    bpdu->version1Length != 0 ||
                    version3Length < sizeof(mstp_bpdu_mstpstatic_t) ||
                    mstp_residual != 0 ||
                    mstp_records > 64)
                decode_as = BPDU_RSTP;
            else
                decode_as = BPDU_MSTP;
        }

        if(decode_as != BPDU_ILL) {
            memset(&cist->rcvBpdu, 0, sizeof(cist->rcvBpdu));
            cist->rcvBpdu.version = (int) bpdu->protocolVersionIdentifier;
            if(cist->rcvBpdu.version > 3)
                cist->rcvBpdu.version = 3; /* Clamp to known version */
            cist->rcvBpdu.type = bpdu->bpduType;
        }

        switch(decode_as) {
        case BPDU_ILL:
            cist->stat.illegal_frame_recvs++;
            T_D("Config BPDU check fails checks - no decode");
            break;

        case BPDU_TCN:
            cist->stat.tcn_frame_recvs++;
            cist->rcvBpdu.role = DesignatedPort;
            cist->rcvBpdu.flags = (u8) 0; /* No flags */
            T_D("Decoded as TCN");
            break;

        case BPDU_CONFIG:
            cist->stat.stp_frame_recvs++;
            cist->rcvBpdu.role = UnknownPort;
            cist->rcvBpdu.flags = bpdu->flags & (MSTP_BPDU_FLAG_TC|MSTP_BPDU_FLAG_TC_ACK);
            getTimes(cist, bpdu);
            getStpPrio(cist, bpdu);
            T_D("Decoded as CONFIG");
            break;

        case BPDU_RSTP:
        case BPDU_MSTP:
            cist->rcvBpdu.role = (PortRole_t) ((bpdu->flags >> 2) & 0x3);
            cist->rcvBpdu.flags = bpdu->flags & MSTP_BPDU_FLAG_MASK;
            getTimes(cist, bpdu);
            if(decode_as == BPDU_RSTP) {
                cist->stat.rstp_frame_recvs++;
                getStpPrio(cist, bpdu);
                T_D("Decoded as RSTP");
            } else {
                cist->stat.mstp_frame_recvs++;
                cist->rcvBpdu.sameRegion = struct_cmp(bpdu->mstp.cfgid, ==, mstp->cfgid);
                cist->rcvBpdu.times.remainingHops = bpdu->mstp.cistinfo.cist_remaining_hops;
                getMstpPrio(cist, bpdu);
                if(cist->rcvBpdu.sameRegion && mstp_records < N_MSTI_MAX) {
                    memcpy(cist->rcvBpdu.msti_rec, 
                           bpdu->mstpcfg, 
                           sizeof(bpdu->mstpcfg[0])*mstp_records);
                    cist->rcvBpdu.mstp_records = mstp_records;
                }
                T_D("Decoded as MSTP, %d MSTI records, from %s region", mstp_records, 
                    cist->rcvBpdu.sameRegion ? "same" : "different");
            }
            break;
        }

        /* Validated & parsed */
        if(decode_as != BPDU_ILL) {
            if(bpduGuard(cist)) {
                T_W("STP inconsistent port#%u disabled (BPDU Guard)", portnum);
                cist->stpInconsistent = TRUE;
                cist->errorRecoveryWhile = mstp->conf.bridge.errorRecoveryDelay;
            } else {
                if(bpduFiltering(cist)) {
                    T_N("port#%u: BPDU Filtering enabled", portnum);
                } else {
                    cist->rcvdBpdu = TRUE;
                }
            }
            mstp_run_stm(cist->bridge);
        }

    } else {
        cist->stat.illegal_frame_recvs++;
        T_D("Basic BPDU length or type check fails checks");
    }
}

void
vtss_mstp_stm_lock(mstp_bridge_t *mstp)
{
    mstp->lock++;
}

void
vtss_mstp_stm_unlock(mstp_bridge_t *mstp)
{
    VTSS_ASSERT(mstp->lock > 0);
    mstp->lock--;
    if(mstp->lock == 0)
        mstp_run_stm(mstp);
}

/******************************************************************************
 * Configuration access
 */

bool
vtss_mstp_get_bridge_parameters(mstp_bridge_t *mstp,
                                mstp_bridge_param_t *param)
{
    *param = mstp->conf.bridge;
    return TRUE;
}

bool
vtss_mstp_get_bridge_priority(mstp_bridge_t *mstp,
                              uint msti,
                              u8 *bridgePriority)
{
    if(msti >= N_MSTI_MAX)
        return FALSE;

    *bridgePriority = mstp->conf.bridgePriority[msti];
    return TRUE;
}

bool
vtss_mstp_get_port_parameters(mstp_bridge_t *mstp,
                              uint portnum,
                              mstp_port_param_t *param)
{
    const mstp_port_param_t *conf = vtss_mstp_get_port_block(mstp, portnum);
    if(!conf)
        return FALSE;
    *param = *conf;
    return TRUE;
}

bool
vtss_mstp_get_msti_port_parameters(mstp_bridge_t *mstp,
                                   uint msti,
                                   uint portnum,
                                   mstp_msti_port_param_t *param)
{
    const mstp_msti_port_param_t *conf = vtss_mstp_get_msti_port_block(mstp, msti, portnum);
    if(!conf)
        return FALSE;
    *param = *conf;
    return TRUE;
}

/******************************************************************************
 * 'Simple' functions
 */

int
vtss_mstp_bridge2str(void *buffer, 
                     size_t size,
                     const u8 *bridgeid)
{
    return snprintf(buffer, size,
                    "%d.%02X-%02X-%02X-%02X-%02X-%02X", 
                    (bridgeid[0] << 8) +  bridgeid[1],
                    bridgeid[2], bridgeid[3],
                    bridgeid[4], bridgeid[5],
                    bridgeid[6], bridgeid[7]);
}
