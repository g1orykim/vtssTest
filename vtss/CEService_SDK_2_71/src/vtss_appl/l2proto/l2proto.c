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

#include "main.h"
#include "vtss_common_os.h"     /* Real "common os" API */
#include "l2proto_api.h"        /* module API */
#include "l2proto.h"            /* Private header file */
#include "port_api.h"
#ifdef VTSS_SW_OPTION_PACKET
#include "packet_api.h"
#endif
#include "msg_api.h"
#include "conf_api.h"
#include "misc_api.h"
#if VTSS_SWITCH_STACKABLE
#include "topo_api.h"
#endif

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_L2PROTO


/*
 * Common Data
 */
 
#if (VTSS_TRACE_ENABLED)
static vtss_trace_reg_t trace_reg =
{ 
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "l2",
    .descr     = "L2 Protocol Helper"
};

static vtss_trace_grp_t trace_grps[TRACE_GRP_CNT] =
{
    [VTSS_TRACE_GRP_DEFAULT] = { 
        .name      = "default",
        .descr     = "Default",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [VTSS_TRACE_GRP_PACKET] = { 
        .name      = "packet",
        .descr     = "Packet",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
};
#endif /* VTSS_TRACE_ENABLED */

/* Stack PDU forwarding */
static uint rx_modules;
static struct {
    vtss_module_id_t modid;
    l2_stack_rx_callback_t cb;
} rx_list[5];

static vtss_common_stpstate_t cached_stpstates[L2_MAX_POAGS];
/* Spanning Tree state that applies to the port when the link is up. */
static vtss_stp_state_t port_stp_state[L2_MAX_POAGS];
static vtss_stp_state_t msti_stp_state[L2_MAX_POAGS][N_L2_MSTI_MAX];

static l2_stp_state_change_callback_t      l2_stp_state_change_table[STP_STATE_CHANGE_REG_MAX];
static l2_stp_msti_state_change_callback_t l2_stp_msti_state_change_table[STP_STATE_CHANGE_REG_MAX];

#define TEMP_LOCK()	cyg_scheduler_lock()
#define TEMP_UNLOCK()	cyg_scheduler_unlock()

/****************************************************************************
 * "Common OS" functions
 ****************************************************************************/

/*
 * Common common code
 */


static BOOL
common_linkstatus(vtss_common_port_t portno, port_status_t *ps)
{
    vtss_port_no_t switchport;
    vtss_isid_t isid;   
  
    if (l2port2port(portno, &isid, &switchport) && port_isid_port_no_is_front_port(isid, switchport)) {
        return  msg_switch_exists(isid) && port_mgmt_status_get(isid, switchport, ps) == VTSS_OK;
    } else {
        return FALSE;
    }
}

static const char *stpstatestring(const vtss_stp_state_t stpst)
{
    switch (stpst) {
    case VTSS_STP_STATE_DISCARDING : /* STP state discarding */
        return "discarding";
    case VTSS_STP_STATE_LEARNING :   /* STP state learning */
        return "learning";
    case VTSS_STP_STATE_FORWARDING : /* STP state forwarding */
        return "forwarding";
    }
    return "<unknown>";
}

const char *vtss_common_str_stpstate(vtss_common_stpstate_t stpstate)
{
    switch (stpstate) {
    case VTSS_COMMON_STPSTATE_DISCARDING :
        return "dis";
    case VTSS_COMMON_STPSTATE_LEARNING :
        return "lrn";
    case VTSS_COMMON_STPSTATE_FORWARDING :
        return "fwd";
    default :
        return "undef";
    }
}

const char *vtss_common_str_linkstate(vtss_common_linkstate_t state)
{
    switch (state) {
    case VTSS_COMMON_LINKSTATE_DOWN :
        return "down";
    case VTSS_COMMON_LINKSTATE_UP :
        return "up";
    default :
        return "Undef";
    }
}

const char *vtss_common_str_linkduplex(vtss_common_duplex_t duplex)
{
    switch (duplex) {
    case VTSS_COMMON_LINKDUPLEX_HALF :
        return "half";
    case VTSS_COMMON_LINKDUPLEX_FULL :
        return "full";
    default :
        return "undef";
    }
}

static l2_msg_t *
l2_alloc_message(size_t size, l2_msg_id_t msg_id)
{
    l2_msg_t *msg = VTSS_MALLOC(size);
    if(msg)
        msg->msg_id = msg_id;
    T_NG(VTSS_TRACE_GRP_PACKET, "msg len %zd, type %d => %p", size, msg_id, msg);
    return msg;
}

static void do_callbacks(vtss_common_port_t portno, vtss_stp_state_t new_state)
{
    uint i;
    /* Callbacks for common port state */
    for (i = 0; i < ARRSZ(l2_stp_state_change_table); i++) {
        l2_stp_state_change_callback_t cb;
        TEMP_LOCK();
        cb = l2_stp_state_change_table[i];
        TEMP_UNLOCK();
        if (cb)
            cb(portno, new_state);
    }
}

static void do_callbacks_msti(vtss_common_port_t l2port, uchar msti, vtss_stp_state_t new_state)
{
    uint i;
    /* Callbacks for MSTI state */
    for (i = 0; i < ARRSZ(l2_stp_msti_state_change_table); i++) {
        l2_stp_msti_state_change_callback_t cb;
        TEMP_LOCK();
        cb = l2_stp_msti_state_change_table[i];
        TEMP_UNLOCK();
        if (cb)
            cb(l2port, msti, new_state);
    }
}

static void l2port_stp_state_set(const l2_port_no_t l2port,
                                 const vtss_stp_state_t stp_state)
{
    vtss_port_no_t switchport;
    vtss_isid_t isid;
    T_N("port %d: new STP state %s", l2port, stpstatestring(stp_state) );

    VTSS_ASSERT(l2port_is_valid(l2port));

    port_stp_state[l2port] = stp_state;
    if(l2port2port(l2port, &isid, &switchport)) {
        if(msg_switch_is_local(isid)) {
            T_I("Local switch port %u: new STP state %s", switchport, stpstatestring(stp_state) );
            (void) vtss_stp_port_state_set(NULL, switchport, stp_state);
        } else {
            if(msg_switch_exists(isid)) {
                l2_msg_t *msg = l2_alloc_message(sizeof(l2_msg_t), L2_MSG_ID_SET_STP_STATE);
                if(msg) {
                    T_I("Remote switch port %d = [%d,%u]: new STP state %s", 
                        l2port, isid, switchport, stpstatestring(stp_state) );
                    msg->data.set_stp_state.switchport = switchport;
                    msg->data.set_stp_state.stp_state = stp_state;
                    msg_tx(VTSS_MODULE_ID_L2PROTO, isid, msg, sizeof(l2_msg_t));
                } else {
                    T_E("Allocation failure, Unable to set STP state %d on port %d", stp_state, l2port);
                }
            }
        }
    } else {
        T_D("Operation on aggr port %d - state %s", l2port, stpstatestring(stp_state));
    }
}

static void
l2_set_msti_stpstate_local(uchar msti,
                           vtss_port_no_t switchport,
                           const vtss_stp_state_t stp_state)
{
    VTSS_ASSERT(msti < N_L2_MSTI_MAX);
    VTSS_ASSERT(l2port_is_poag(switchport));
    T_N("MSTI%d port %d: new STP state %s", msti, switchport, stpstatestring(stp_state) );

    if(port_no_is_stack(switchport) ||
       switchport >= port_isid_port_count(VTSS_ISID_LOCAL)) {
        T_E("Invalid port %d\n", switchport);
        return;
    }

    vtss_rc rc = vtss_mstp_port_msti_state_set(NULL,
                                               switchport, 
                                               (vtss_msti_t)(msti+VTSS_MSTI_START), /* NB: 1-based API */
                                               stp_state);

    VTSS_ASSERT(rc == VTSS_OK);
}

static vtss_rc
l2local_set_msti_map(BOOL all_to_cist, /* Set if *not* MSTP mode */
                     size_t maplen, 
                     const uchar *map)
{
    vtss_vid_t vid;
    vtss_rc rc = VTSS_INCOMPLETE;

    VTSS_ASSERT(maplen <= VTSS_VIDS);
    T_I("Set map - all_to_cist %d, maplen %zd", all_to_cist, maplen);

    for(vid = 1; vid < maplen; vid++) {
        uchar msti = all_to_cist ? 0 : map[vid];
        if(msti) {
            T_I("VID %d in MSTI%d", vid, msti);
        }
        if((rc = vtss_mstp_vlan_msti_set(NULL, vid, (vtss_msti_t)(msti+VTSS_MSTI_START))) != VTSS_OK) {
            T_E("vtss_mstp_vlan_set(%d, %d): Failed: %s", vid, msti, error_txt(rc));
            return rc;
        }
    }
    return rc;
}

/**
 * Set the MSTP MSTI mapping table
 */
vtss_rc
l2_set_msti_map(BOOL all_to_cist, /* Set if *not* MSTP mode */
                size_t maplen, 
                const uchar *map)
{
    vtss_rc rc = VTSS_OK;

    VTSS_ASSERT(maplen <= VTSS_VIDS);
    T_I("Set map - all_to_cist %d, maplen %zd", all_to_cist, maplen);

    if(vtss_switch_stackable()) {
        vtss_isid_t isid;
        for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
            if(msg_switch_exists(isid)) {
                if(!msg_switch_is_local(isid)) {
                    size_t msg_len = sizeof(l2_msg_t) + maplen;
                    l2_msg_t *msg = l2_alloc_message(msg_len, L2_MSG_ID_SET_MSTI_MAP);
                    if(msg) {
                        msg->data.set_msti_map.all_to_cist = all_to_cist;
                        msg->data.set_msti_map.maplen = maplen;
                        memcpy(msg->data.set_msti_map.map, map, maplen);
                        T_I("Slave Set map - ISID %u", isid);
                        msg_tx(VTSS_MODULE_ID_L2PROTO, isid, msg, msg_len);
                    } else {
                        T_E("Allocation failure setting MSTI map (len %zu), isid %d", maplen, isid);
                        rc = VTSS_UNSPECIFIED_ERROR;
                        break;
                    }
                } else {
                    if((rc = l2local_set_msti_map(all_to_cist, maplen, map)) != VTSS_OK)
                        break;
                }
            }
        }
    } else {
        rc = l2local_set_msti_map(all_to_cist, maplen, map);
    }
    return rc;
}

/**
 * Set the Spanning Tree state of all MSTIs for a specific port.
 */
void 
l2_set_msti_stpstate_all(vtss_common_port_t l2port, 
                         vtss_stp_state_t new_state)
{
    if(msg_switch_is_master()) {
        vtss_port_no_t switchport;
        vtss_isid_t isid;
        T_D("MSTIx port %d: new STP state %s", l2port, stpstatestring(new_state) );
        if(l2port_is_valid(l2port)) {
            uchar msti;
            for(msti = 0; msti < N_L2_MSTI_MAX; msti++)
                msti_stp_state[l2port][msti] = (uchar) new_state;
            if(l2port2port(l2port, &isid, &switchport)) {
                if(!port_isid_port_no_is_front_port(isid, switchport)) {
                    T_W("Invalid port %d:%d\n", isid, switchport);
                    return;
                }
                if(msg_switch_is_local(isid)) {
                    for(msti = 0; msti < N_L2_MSTI_MAX; msti++)
                        l2_set_msti_stpstate_local(msti, switchport, new_state);
                } else {
                    l2_msg_t *msg = l2_alloc_message(sizeof(l2_msg_t), L2_MSG_ID_SET_MSTI_STP_STATE_ALL);
                    if(msg) {
                        T_I("Remote switch MSTIx port %d = [%d,%d]: new STP state %s", 
                            l2port, isid, switchport, stpstatestring(new_state) );
                        msg->data.set_msti_stp_state.msti = 0xFF; /* Unused */
                        msg->data.set_msti_stp_state.switchport = switchport;
                        msg->data.set_msti_stp_state.stp_state = new_state;
                        msg_tx(VTSS_MODULE_ID_L2PROTO, isid, msg, sizeof(l2_msg_t));
                    } else {
                        T_E("Allocation failure, Unable to set MSTIx STP state %d on port %d", 
                            new_state, l2port);
                    }
                }
            } 
            /* Looped callback on each MSTI */
            for(msti = 0; msti < N_L2_MSTI_MAX; msti++)
                do_callbacks_msti(l2port, msti, new_state);
        } else {
            T_E("%s: Invalid port %d", __FUNCTION__, l2port);
        }
    }
}

/**
 * Set the Spanning Tree state of a specific MSTI port.
 */
void 
l2_set_msti_stpstate(uchar msti, 
                     vtss_common_port_t l2port, 
                     vtss_stp_state_t new_state)
{
    VTSS_ASSERT(msti < N_L2_MSTI_MAX);
    if(msg_switch_is_master()) {
        vtss_port_no_t switchport;
        vtss_isid_t isid;
        T_D("MSTI%d port %d: new STP state %s", msti, l2port, stpstatestring(new_state) );
        if(l2port_is_valid(l2port)) {
            if(new_state != (vtss_stp_state_t) msti_stp_state[l2port][msti]) {
                msti_stp_state[l2port][msti] = (uchar) new_state;
                if(l2port2port(l2port, &isid, &switchport)) {
                    if(!port_isid_port_no_is_front_port(isid, switchport)) {
                        T_W("Invalid port %d:%d\n", isid, switchport);
                        return;
                    }
                    if(msg_switch_is_local(isid)) {
                        l2_set_msti_stpstate_local(msti, switchport, new_state);
                    } else {
                        l2_msg_t *msg = l2_alloc_message(sizeof(l2_msg_t), L2_MSG_ID_SET_MSTI_STP_STATE);
                        if(msg) {
                            T_I("Remote switch MSTI%d port %d = [%d,%d]: new STP state %s", 
                                msti, l2port, isid, switchport, stpstatestring(new_state) );
                            msg->data.set_msti_stp_state.msti = msti;
                            msg->data.set_msti_stp_state.switchport = switchport;
                            msg->data.set_msti_stp_state.stp_state = new_state;
                            msg_tx(VTSS_MODULE_ID_L2PROTO, isid, msg, sizeof(l2_msg_t));
                        } else {
                            T_E("Allocation failure, Unable to set MSTI%d STP state %d on port %d", 
                                msti, new_state, l2port);
                        }
                    }
                }
                do_callbacks_msti(l2port, msti, new_state);
            }
        } else {
            T_E("%s: Invalid port %d", __FUNCTION__, l2port);
        }
    }
}

/**
 * Get the Spanning Tree state of a specific MSTI port.
 */
vtss_stp_state_t
l2_get_msti_stpstate(uchar msti, 
                     vtss_common_port_t l2port)
{
    VTSS_ASSERT(msti < N_L2_MSTI_MAX);
    VTSS_ASSERT(l2port_is_valid(l2port));
    TEMP_LOCK();
    uchar st = msti_stp_state[l2port][msti];
    TEMP_UNLOCK();
    return (vtss_stp_state_t) st;
}

static void l2_local_set_states(vtss_port_no_t switchport, 
                                vtss_stp_state_t port_state, 
                                const vtss_stp_state_t *msti_state)
{
    uchar msti;
    T_I("Switch port %u: Set STP states", switchport); 
    (void) vtss_stp_port_state_set(NULL, switchport, port_state);
    for(msti = 0; msti < N_L2_MSTI_MAX; msti++)
        l2_set_msti_stpstate_local(msti, switchport, msti_state[msti]);
}

void l2_sync_stpstates(vtss_common_port_t copy, vtss_common_port_t master)
{
    if(msg_switch_is_master()) {
        vtss_port_no_t switchport;
        vtss_isid_t isid;
        if(l2port2port(copy, &isid, &switchport)) {
            if(msg_switch_exists(isid)) {
                T_I("Sync switch port %d = [%d,%d] to %d", copy, isid, switchport, master);
                if(msg_switch_is_local(isid)) {
                    l2_local_set_states(switchport, port_stp_state[master], msti_stp_state[master]);
                } else {
                    l2_msg_t *msg = l2_alloc_message(sizeof(l2_msg_t), L2_MSG_ID_SET_STP_STATES);
                    T_D("Sync MSTI states of %d to %d",  copy, master);
                    if(msg) {
                        uchar msti;
                        msg->data.set_stp_states.switchport = switchport;
                        msg->data.set_stp_states.port_state = port_stp_state[master];
                        for(msti = 0; msti < N_L2_MSTI_MAX; msti++)
                            msg->data.set_stp_states.msti_state[msti] = 
                                msti_stp_state[master][msti];
                        msg_tx(VTSS_MODULE_ID_L2PROTO, isid, msg, sizeof(l2_msg_t));
                    } else {
                        T_W("Sync switch port %d = [%d,%d] to %d unsucessful", 
                            copy, isid, switchport, master);
                    }
                }
            }
        } else {
            T_E("%s: Invalid port %d", __FUNCTION__, copy);
        }
    }
}

/**
 * vtss_os_get_linkspeed - Deliver the current link speed (in Kbps) of a specific port.
 */
vtss_common_linkspeed_t vtss_os_get_linkspeed(vtss_common_port_t portno)
{
    port_status_t port_status;
    if(common_linkstatus(portno, &port_status)) {
        switch (port_status.status.speed) {
        case VTSS_SPEED_UNDEFINED : return (vtss_common_linkspeed_t)0;
        case VTSS_SPEED_10M :       return (vtss_common_linkspeed_t)10;
        case VTSS_SPEED_100M :      return (vtss_common_linkspeed_t)100;
        case VTSS_SPEED_1G :        return (vtss_common_linkspeed_t)1000;
        case VTSS_SPEED_2500M :     return (vtss_common_linkspeed_t)2500;
        case VTSS_SPEED_5G :        return (vtss_common_linkspeed_t)5000;
        case VTSS_SPEED_10G :       return (vtss_common_linkspeed_t)10000;
        case VTSS_SPEED_12G :       return (vtss_common_linkspeed_t)12000;
        default :                   return (vtss_common_linkspeed_t)1;
        }
    }
    return (vtss_common_linkspeed_t)0;
}

/**
 * vtss_os_get_linkstate - Deliver the current link state of a specific port.
 */
vtss_common_linkstate_t vtss_os_get_linkstate(vtss_common_port_t portno)
{
    vtss_common_linkstate_t link = VTSS_COMMON_LINKSTATE_DOWN;
    port_status_t port_status;
    
    if(common_linkstatus(portno, &port_status) && 
       port_status.status.link)
        link = VTSS_COMMON_LINKSTATE_UP;
    T_N("port %d: link %s", portno, link == VTSS_COMMON_LINKSTATE_DOWN ? "down" : "up");
    return link;
}

/**
 * vtss_os_get_linkduplex - Deliver the current link duplex mode of a specific port.
 */
vtss_common_duplex_t vtss_os_get_linkduplex(vtss_common_port_t portno)
{
    if(msg_switch_is_master()) {
        port_status_t port_status;
        if(common_linkstatus(portno, &port_status) && 
           port_status.status.link &&
           port_status.status.fdx)
            return VTSS_COMMON_LINKDUPLEX_FULL;
    }
    return VTSS_COMMON_LINKDUPLEX_HALF;
}

/**
 * vtss_os_get_systemmac - Deliver the MAC address for the switch.
 */
void vtss_os_get_systemmac(vtss_common_macaddr_t *system_macaddr)
{
    if(msg_switch_is_master())
        (void) conf_mgmt_mac_addr_get((uchar*)system_macaddr, 0);
}

/**
 * vtss_os_get_portmac - Deliver the MAC address for the a specific port.
 */
void vtss_os_get_portmac(vtss_common_port_t portno, vtss_common_macaddr_t *port_macaddr)
{
    uchar *mac = (uchar*)port_macaddr;
    vtss_port_no_t switchport;
    vtss_isid_t isid;
    VTSS_ASSERT(l2port_is_port(portno));
    if(l2port2port(portno, &isid, &switchport)) {
        mac_addr_t basemac;
#if VTSS_SWITCH_STACKABLE
        (void) topo_isid2mac(isid, basemac);
#else
        (void) conf_mgmt_mac_addr_get(basemac, 0);
#endif
        misc_instantiate_mac(mac, basemac, 1+switchport-VTSS_PORT_NO_START); /* entry 0 is the CPU port */
        T_D("Port MAC address [%d,%u] = %s", isid, switchport, misc_mac2str(mac));
    }
}

/**
 * vtss_os_set_stpstate - Set the Spanning Tree state of a specific port.
 */
void vtss_os_set_stpstate(vtss_common_port_t portno, vtss_common_stpstate_t new_state)
{
    if(msg_switch_is_master()) {
        VTSS_ASSERT(l2port_is_valid(portno));
        
        cached_stpstates[portno] = new_state;
        l2port_stp_state_set(portno, (vtss_stp_state_t) new_state);
        
        /* Call registered functions outside critical region */
        do_callbacks(portno, new_state);
    }
}

/**
 * vtss_os_get_stpstate - Get the Spanning Tree state of a specific port.
 */
vtss_common_stpstate_t vtss_os_get_stpstate(vtss_common_port_t portno)
{
    VTSS_ASSERT(l2port_is_valid(portno));
    TEMP_LOCK();
    vtss_common_stpstate_t st = cached_stpstates[portno];
    TEMP_UNLOCK();
    return st;
}

/**
 * vtss_os_alloc_xmit - Allocate a buffer to be used for transmitting a frame.
 */
void *vtss_os_alloc_xmit(vtss_common_port_t l2port, 
                         vtss_common_framelen_t len, 
                         vtss_common_bufref_t *pbufref)
{
    vtss_isid_t isid;
    vtss_port_no_t port;
    void *p = NULL;
    VTSS_ASSERT(l2port_is_valid(l2port));
    if(l2port2port(l2port, &isid, &port)) {
        if(msg_switch_is_local(isid)) {
#ifdef VTSS_SW_OPTION_PACKET
            p = packet_tx_alloc(len);
#endif
            *pbufref = NULL;    /* Local operation */
        } else {                /* Remote */
            l2_msg_t *msg = l2_alloc_message(sizeof(*msg) + len, L2_MSG_ID_FRAME_TX_REQ);
            if(msg) {
                msg->data.tx_req.switchport = port;
                msg->data.tx_req.len = len;
                msg->data.tx_req.isid = isid;
                msg->data.tx_req.l2port = l2port;
                *pbufref = (void *) msg; /* Remote op */
                p = ((unsigned char*) msg) + sizeof(*msg);
            } else {
                T_EG(VTSS_TRACE_GRP_PACKET, "Allocation failure, TX port %d length %d", l2port, len);
            }
        }
    }
    T_NG(VTSS_TRACE_GRP_PACKET, "%s(%d) ret %p", __FUNCTION__, len, p);
    return p;
}

int vtss_os_xmit(vtss_common_port_t l2port, 
                 void *frame, 
                 vtss_common_framelen_t len, 
                 vtss_common_bufref_t bufref)
{
    l2_msg_t *msg = (l2_msg_t *)bufref;
    T_DG(VTSS_TRACE_GRP_PACKET, "%s(%d, %p, %d)", __FUNCTION__, l2port, frame, len);
    VTSS_ASSERT(l2port_is_port(l2port));
    if(msg) {
        VTSS_ASSERT(msg->msg_id == L2_MSG_ID_FRAME_TX_REQ &&
                    msg->data.tx_req.l2port == l2port);
        if(msg_switch_is_local(msg->data.tx_req.isid)) {
            T_EG(VTSS_TRACE_GRP_PACKET, "ISID became local (%d)?", msg->data.tx_req.isid);
            VTSS_FREE(msg);
        } else {
            if (msg->data.tx_req.len < len) {
                T_EG(VTSS_TRACE_GRP_PACKET, "Length error %d %d\n", msg->data.tx_req.len, len);
            }
            msg->data.tx_req.len = len;
            msg_tx(VTSS_MODULE_ID_L2PROTO, 
                   msg->data.tx_req.isid, msg, len + sizeof(*msg));

            return VTSS_COMMON_CC_OK;
        }
    } else {
#ifdef VTSS_SW_OPTION_PACKET
        vtss_isid_t isid;
        vtss_port_no_t port;

        if(l2port2port(l2port, &isid, &port)) {
            packet_tx_props_t tx_props;
            packet_tx_props_init(&tx_props);
            tx_props.packet_info.modid     = VTSS_MODULE_ID_L2PROTO;
            tx_props.packet_info.frm[0]    = frame;
            tx_props.packet_info.len[0]    = len;
            tx_props.tx_info.dst_port_mask = VTSS_BIT64(port);
            if (packet_tx(&tx_props) == VTSS_RC_OK) {
                return VTSS_COMMON_CC_OK;
            }
        } else {
            T_EG(VTSS_TRACE_GRP_PACKET, "Transmit on non-port (%d)?", l2port);
            packet_tx_free(frame);
        }
#endif
    }
    T_EG(VTSS_TRACE_GRP_PACKET, "Trame transmit on port %d failed", l2port);
    return VTSS_COMMON_CC_GENERR;
}

/*
 * l2 API
 */

/* l2port mapping functions */

BOOL l2port2port(l2_port_no_t l2port, 
                 vtss_isid_t* pisid, 
                 vtss_port_no_t* port)
{
    if(l2port_is_port(l2port)) {
        if(vtss_switch_stackable()) {
            *pisid = VTSS_ISID_START + (l2port / L2_MAX_SWITCH_PORTS);
            *port  = (l2port % L2_MAX_SWITCH_PORTS);
        } else {
            *pisid = VTSS_ISID_START; /* This is the only valid ISID for the standalone version */
            *port = l2port;
        }
        return TRUE;
    }
    return FALSE;
}

BOOL l2port2poag(l2_port_no_t l2port, 
                 vtss_isid_t* pisid, 
                 vtss_poag_no_t* poag)
{
    if(l2port2port(l2port, pisid, poag)) {
        return TRUE;
    } else if(l2port_is_poag(l2port)) {
        if(vtss_switch_stackable()) {
            l2port -= L2_MAX_PORTS;
#if defined(VTSS_FEATURE_VSTAX_V2)     
            *pisid = VTSS_ISID_START + (l2port / VTSS_GLAGS);
            *poag  = AGGR_MGMT_GROUP_NO_START + (l2port % VTSS_GLAGS);

#else
#if AGGR_LLAG_CNT > 0
	      *pisid = VTSS_ISID_START + (l2port / AGGR_LLAG_CNT);
	      *poag  = AGGR_MGMT_GROUP_NO_START + (l2port % AGGR_LLAG_CNT);
#else
	      *pisid = VTSS_ISID_START + l2port;
	      *poag  = AGGR_MGMT_GROUP_NO_START + l2port;
#endif
#endif

        } else {
            *pisid = VTSS_ISID_START;
            *poag = l2port  - L2_MAX_PORTS + AGGR_MGMT_GROUP_NO_START;
        }
        return TRUE;
    }

    return FALSE;
}

BOOL l2port2glag(l2_port_no_t l2port, 
                 vtss_glag_no_t* glag)
{
#if defined(VTSS_FEATURE_AGGR_GLAG)
    if(l2port_is_glag(l2port)) {
        *glag = (vtss_glag_no_t) (l2port-L2_MAX_PORTS-L2_MAX_LLAGS+AGGR_MGMT_GROUP_NO_START);
        return TRUE;
    }
#endif /* VTSS_FEATURE_AGGR_GLAG */
    return FALSE;
}

BOOL l2port_is_valid(l2_port_no_t l2port)
{
    return (l2port < L2_MAX_POAGS);
}

BOOL l2port_is_port(l2_port_no_t l2port)
{
    return (l2port < L2_MAX_PORTS);
}

BOOL l2port_is_poag(l2_port_no_t l2port)
{
    return (l2port < (L2_MAX_PORTS + L2_MAX_LLAGS));
}

BOOL l2port_is_glag(l2_port_no_t l2port)
{
#if defined(VTSS_FEATURE_AGGR_GLAG)
    return (l2port >= (L2_MAX_PORTS + L2_MAX_LLAGS) && l2port < L2_MAX_POAGS);
#else
    return FALSE;
#endif /* VTSS_FEATURE_AGGR_GLAG */
}

/*lint -sem(l2port2str, thread_protected )
 * No, this is not thread safe. But its a convenience tradeoff.
 */
const char*
l2port2str(l2_port_no_t l2port)
{
    static char _buf[16];       /* NOT THREAD SAFE */
    vtss_isid_t isid;
    vtss_port_no_t port;

    if(l2port2port(l2port, &isid, &port)) {
#if VTSS_SWITCH_STACKABLE
        (void) snprintf(_buf, sizeof(_buf), "%2d:%u", topo_isid2usid(isid), iport2uport(port));
#else
        (void) snprintf(_buf, sizeof(_buf), "%u", iport2uport(port));
#endif
    } else if(l2port2poag(l2port, &isid, &port)) {
#if VTSS_SWITCH_STACKABLE
        (void) snprintf(_buf, sizeof(_buf), "%2d:LLAG%u", topo_isid2usid(isid), port);
#else
        (void) snprintf(_buf, sizeof(_buf), "LLAG%u", port);
#endif        
    } else if(l2port2glag(l2port, &port)) {
        (void) snprintf(_buf, sizeof(_buf), "GLAG%u", port);
    } else {
        (void) snprintf(_buf, sizeof(_buf), "%d (L2)", l2port);
    }
    return _buf;
}

static void
l2_do_rx_callback(vtss_module_id_t modid,
                  const void *packet,
                  size_t len,
                  vtss_vid_t vid,
                  l2_port_no_t l2port)
{
    l2_stack_rx_callback_t cb = NULL;
    uint i;
    TEMP_LOCK();
    for(i = 0; i < rx_modules; i++) {
        if(rx_list[i].modid == modid) {
            cb = rx_list[i].cb;
            break;
        }
    }
    TEMP_UNLOCK();
    if(cb != NULL)
        cb(packet, len, vid, l2port);
}

void l2_receive_indication(vtss_module_id_t modid, 
                           const void *packet,
                           size_t len, 
                           vtss_port_no_t switchport,
                           vtss_vid_t vid,
                           vtss_glag_no_t glag_no)
{
    T_DG(VTSS_TRACE_GRP_PACKET, "len %zd port %u vid %d glag %u", len, switchport, vid, glag_no);
    size_t msg_len = sizeof(l2_msg_t) + len;
    l2_msg_t *msg = l2_alloc_message(msg_len, L2_MSG_ID_FRAME_RX_IND);
    T_DG(VTSS_TRACE_GRP_PACKET, "len %zd port %u vid %d glag %u", len, switchport, vid, glag_no);
    if(msg) {
        msg->data.rx_ind.modid = modid;
        msg->data.rx_ind.switchport = switchport;
        msg->data.rx_ind.len = len;
        msg->data.rx_ind.glag_no = glag_no;
        memcpy(&msg[1], packet, len); /* Copy frame */
        msg_tx_adv(NULL, NULL, MSG_TX_OPT_NO_ALLOC_ON_LOOPBACK | MSG_TX_OPT_SHAPE, VTSS_MODULE_ID_L2PROTO, 0, msg, msg_len);
    } else {
        T_WG(VTSS_TRACE_GRP_PACKET, "Unable to allocate %zd bytes, tossing frame on port %u", msg_len, switchport);
    }
}

void l2_receive_register(vtss_module_id_t modid, l2_stack_rx_callback_t cb)
{
    TEMP_LOCK();
    VTSS_ASSERT(rx_modules < ARRSZ(rx_list));
    rx_list[rx_modules].modid = modid;
    rx_list[rx_modules].cb = cb;
    rx_modules++;
    TEMP_UNLOCK();
}

static void l2_sync_stpstates_switch(vtss_isid_t isid)
{
    if(msg_switch_is_local(isid)) {
        port_iter_t pit;
        (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            l2_port_no_t l2port = L2PORT2PORT(isid, pit.iport);
            T_I("Initial STP sync port %d - %d state %d", pit.iport, l2port, port_stp_state[l2port]);
            (void) vtss_stp_port_state_set(NULL, pit.iport, port_stp_state[l2port]);
        }
    } else {
        size_t msg_len = sizeof(l2_msg_t);
        l2_msg_t *msg = l2_alloc_message(msg_len, L2_MSG_ID_SYNC_STP_STATE);
        if(msg) {
            uint i;
            l2_port_no_t l2port = L2PORT2PORT(isid, VTSS_PORT_NO_START);
            for(i = 0; i < ARRSZ(msg->data.synch_stp_state.stp_states); i++,l2port++) {
                msg->data.synch_stp_state.stp_states[i] = port_stp_state[l2port];
            }
            msg_tx(VTSS_MODULE_ID_L2PROTO, isid, msg, msg_len);
        } else {
            T_W("Allocation failure, Unable to synch STP state to sid %d", isid);
        }
    }
}

static void 
l2local_flush_vport(l2_port_type type, 
                    int vport,
                    vtss_common_vlanid_t vlan_id)
{
    switch(type) {
    case L2_PORT_TYPE_POAG:
        if(vport < VTSS_PORTS) {
            /* Do a local flush vlan port / aggr */
            T_I("FLUSH_FDB: port %d, vid %d", vport, vlan_id);
            if (vlan_id == VTSS_VID_NULL) {
                (void) vtss_mac_table_port_flush(NULL, vport);
            } else {
                (void) vtss_mac_table_vlan_port_flush(NULL, vport, vlan_id);
            }
        } else {
            vtss_aggr_no_t zero_based_aggr_no;
            BOOL members[VTSS_PORT_ARRAY_SIZE];
            T_I("FLUSH_FDB: aggr %d, vid %d", vport, vlan_id);
            zero_based_aggr_no = (vport - VTSS_PORTS - AGGR_MGMT_GROUP_NO_START);
            if(vtss_aggr_port_members_get(NULL, zero_based_aggr_no, members) == VTSS_OK) {
                vtss_port_no_t switchport;
                for (switchport = VTSS_PORT_NO_START; switchport < VTSS_PORT_NO_END; switchport++) {
                    if(members[switchport]) {
                        T_I("FLUSH_FDB: aggr %d, port %u, vid %d", vport, switchport, vlan_id);
                        if (vlan_id == VTSS_VID_NULL) {
                            (void) vtss_mac_table_port_flush(NULL, switchport); /* Loose the vlan? */
                        } else {
                            (void) vtss_mac_table_vlan_port_flush(NULL, switchport, vlan_id); /* Loose the vlan? */
                        }
                    }
                }
            }
        }
        break;
    case L2_PORT_TYPE_GLAG:
        T_I("FLUSH_FDB: glag %d vlan %d", vport, vlan_id);
#if defined(VTSS_FEATURE_AGGR_GLAG)
        if (vlan_id == VTSS_VID_NULL) {
            (void) vtss_mac_table_glag_flush(NULL, vport);
        } else {
            (void) vtss_mac_table_vlan_glag_flush(NULL, vport, vlan_id);
        }
#endif /* VTSS_FEATURE_AGGR_GLAG */
        break;
    default:
        VTSS_ASSERT(0);
    }
}

static void 
l2_flush_glag(vtss_glag_no_t glag)
{
    if(vtss_switch_stackable()) {
        switch_iter_t sit;
        (void)switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID);
        while (switch_iter_getnext(&sit)) {
            if(!msg_switch_is_local(sit.isid)) {
                l2_msg_t *msg = l2_alloc_message(sizeof(l2_msg_t), L2_MSG_ID_FLUSH_FDB);
                if(msg) {
                    msg->data.flush_fdb.type = L2_PORT_TYPE_GLAG;
                    msg->data.flush_fdb.port = glag;
                    msg->data.flush_fdb.vlan_id = VTSS_VID_NULL;
                    msg_tx(VTSS_MODULE_ID_L2PROTO, sit.isid, msg, sizeof(l2_msg_t));
                } else {
                    T_E("Allocation failure, Unable to flush GLAG %u, isid %d", glag, sit.isid);
                }
            } else {
                /* Do a local flush GLAG */
                l2local_flush_vport(L2_PORT_TYPE_GLAG, glag, VTSS_VID_NULL);
            }
        }
    } else {
        /* Do a local flush GLAG */
        l2local_flush_vport(L2_PORT_TYPE_GLAG, glag, VTSS_VID_NULL);
    }
}

void 
l2_flush_port(l2_port_no_t l2port)
{
    VTSS_ASSERT(l2port_is_valid(l2port));
    vtss_glag_no_t glag;
    if(l2port2glag(l2port, &glag)) {
        // The joys of having multiple aggr ranges...!
        l2_flush_glag(glag-AGGR_MGMT_GROUP_NO_START+VTSS_AGGR_NO_START);
    } else {
        vtss_poag_no_t poag;
        vtss_isid_t isid;
        VTSS_ASSERT(l2port2poag(l2port, &isid, &poag));
        if(vtss_switch_stackable()) {
#if defined(VTSS_FEATURE_VSTAX_V2) && VTSS_SWITCH_STACKABLE
            vtss_vstax_upsid_t upsid;
            port_isid_port_info_t info;
            if ((upsid = topo_isid_port2upsid(isid, poag)) != VTSS_VSTAX_UPSID_UNDEF &&
                port_isid_port_info_get(isid, poag, &info) == VTSS_OK) {
                switch_iter_t sit;
                (void)switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID);
                while (switch_iter_getnext(&sit)) {
                    l2_msg_t *msg = l2_alloc_message(sizeof(l2_msg_t), L2_MSG_ID_FLUSH_FDB_STACK);
                    if(msg) {
                        msg->data.flush_fdb_stack.upsid = upsid;
                        msg->data.flush_fdb_stack.upspn = info.chip_port; /* == upspn */
                        msg_tx(VTSS_MODULE_ID_L2PROTO, sit.isid, msg, sizeof(l2_msg_t));
                    } else {
                        T_E("Allocation failure, Unable to flush FDB on port %d", l2port);
                    }
                }
            } else {
                T_I("Unable to map isid %d, port %d to upsid, upspn", isid, poag);
            }
#endif  /* VTSS_FEATURE_VSTAX_V2 && VTSS_SWITCH_STACKABLE */
        } else {
            /* Do a local flush vlan port */
            l2local_flush_vport(L2_PORT_TYPE_POAG, poag, 0);
        }
    }
}

static void 
l2_flush_vlan_glag(vtss_glag_no_t glag,
                   vtss_common_vlanid_t vlan_id)
{
    if(vtss_switch_stackable()) {
        vtss_isid_t isid;
        for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
            if(msg_switch_exists(isid)) {
                if(!msg_switch_is_local(isid)) {
                    l2_msg_t *msg = l2_alloc_message(sizeof(l2_msg_t), L2_MSG_ID_FLUSH_FDB);
                    if(msg) {
                        msg->data.flush_fdb.type     = L2_PORT_TYPE_GLAG;
                        msg->data.flush_fdb.port     = glag;
                        msg->data.flush_fdb.vlan_id  = vlan_id;
                        msg_tx(VTSS_MODULE_ID_L2PROTO, isid, msg, sizeof(l2_msg_t));
                    } else {
                        T_E("Allocation failure, Unable to flush GLAG %u, isid %d vid %d", glag, isid, vlan_id);
                    }
                } else {
                    /* Do a local flush GLAG */
                    l2local_flush_vport(L2_PORT_TYPE_GLAG, glag, vlan_id);
                }
            }
        }
    } else {
        /* Do a local flush GLAG */
        l2local_flush_vport(L2_PORT_TYPE_GLAG, glag, vlan_id);
    }
}

void 
l2_flush_vlan_port(l2_port_no_t l2port,
                   vtss_common_vlanid_t vlan_id)
{
    VTSS_ASSERT(l2port_is_valid(l2port));
    vtss_glag_no_t glag;

    if(l2port2glag(l2port, &glag)) {
        // The joys of having multiple aggr ranges...!
        l2_flush_vlan_glag(glag-AGGR_MGMT_GROUP_NO_START+VTSS_AGGR_NO_START,
                                                                       vlan_id);
    } else {
        vtss_poag_no_t poag;
        vtss_isid_t isid;
        VTSS_ASSERT(l2port2poag((l2port - AGGR_MGMT_GROUP_NO_START), &isid, &poag));
        if(msg_switch_exists(isid)) {
            if(vtss_switch_stackable() && !msg_switch_is_local(isid)) {
                l2_msg_t *msg = l2_alloc_message(sizeof(l2_msg_t), L2_MSG_ID_FLUSH_FDB);
                if(msg) {
                    msg->data.flush_fdb.type    = L2_PORT_TYPE_POAG;
                    msg->data.flush_fdb.port    = poag;
                    msg->data.flush_fdb.vlan_id = vlan_id;
                    msg_tx(VTSS_MODULE_ID_L2PROTO, isid, msg, sizeof(l2_msg_t));
                } else {
                    T_E("Allocation failure, Unable to flush FDB on port %d vid %d", l2port, vlan_id);
                }
            } else {
                /* Do a local flush vlan port */
                l2local_flush_vport(L2_PORT_TYPE_POAG, (VTSS_PORTS + poag), vlan_id);
            }
        }
    }
}

static vtss_rc l2_stp_register(void **array, uint asize, const char *array_name, void *callback)
{
    vtss_rc rc = VTSS_OK;
    uint    i;

    if (callback == NULL) {
        T_E("The callback function for %s is NULL\n", array_name);
        return VTSS_UNSPECIFIED_ERROR;
    }

    /* Lookup if the callback is existing?
       If not, found the index for inserting later */
    for (i = 0; i < asize; i++) {
        if (callback == array[i])
            break;
        if (array[i] == NULL) {
            array[i] = callback;
            break;
        }
    }

    if(i == asize) {
        T_E("The %s table full\n", array_name);
        rc = VTSS_UNSPECIFIED_ERROR;
    }

    return rc;
}

/* L2 STP state change registration */
vtss_rc l2_stp_state_change_register(l2_stp_state_change_callback_t callback)
{
    vtss_rc rc;
    TEMP_LOCK();
    rc = l2_stp_register((void **)l2_stp_state_change_table, 
                         ARRSZ(l2_stp_state_change_table),
                         "l2_stp_state_change_table", callback);
    TEMP_UNLOCK();
    return rc;
}

/* STP MSTI state change callback registration */
vtss_rc l2_stp_msti_state_change_register(l2_stp_msti_state_change_callback_t callback)
{
    vtss_rc rc;
    TEMP_LOCK();
    rc = l2_stp_register((void**)l2_stp_msti_state_change_table, 
                                ARRSZ(l2_stp_msti_state_change_table),
                                "l2_stp_msti_state_change_table", callback);
    TEMP_UNLOCK();
    return rc;
}

vtss_rc l2port_iter_init(l2port_iter_t *l2pit, vtss_isid_t isid, l2port_iter_type_t l2type)
{
    l2pit->s_order = (l2type & L2PORT_ITER_ISID_ALL) ? SWITCH_ITER_SORT_ORDER_ISID_ALL : (l2type & L2PORT_ITER_ISID_CFG) ? ((l2type & L2PORT_ITER_USID_ORDER) ? SWITCH_ITER_SORT_ORDER_USID_CFG : SWITCH_ITER_SORT_ORDER_ISID_CFG) : ((l2type & L2PORT_ITER_USID_ORDER) ? SWITCH_ITER_SORT_ORDER_USID : SWITCH_ITER_SORT_ORDER_ISID);
    l2pit->p_order = (l2type & L2PORT_ITER_PORT_ALL) ? PORT_ITER_SORT_ORDER_IPORT_ALL : PORT_ITER_SORT_ORDER_IPORT;
    l2pit->isid_req = isid;
    l2pit->itertype_req = l2pit->itertype_pend = l2type;
    l2pit->ix = -1;
    (void)switch_iter_init(&l2pit->sit, l2pit->isid_req, l2pit->s_order);
    (void)port_iter_init(&l2pit->pit, &l2pit->sit, VTSS_ISID_GLOBAL,
                         l2pit->p_order, PORT_ITER_FLAGS_NORMAL);
    return VTSS_OK;
}

BOOL l2port_iter_getnext(l2port_iter_t *l2pit)
{
    if(l2pit->itertype_pend & L2PORT_ITER_TYPE_PHYS) {
        if(port_iter_getnext(&l2pit->pit)) {
            l2pit->isid = l2pit->sit.isid;
            l2pit->usid = l2pit->sit.usid;
            l2pit->iport = l2pit->pit.iport;
            l2pit->uport = l2pit->pit.uport;
            l2pit->l2port = L2PORT2PORT(l2pit->isid, l2pit->iport);
            l2pit->type = L2PORT_ITER_TYPE_PHYS;
            return TRUE;
        }
        l2pit->itertype_pend &= ~L2PORT_ITER_TYPE_PHYS; /* Done physports */
        /* Reinit for aggrs */
        (void) switch_iter_init(&l2pit->sit, l2pit->isid_req, l2pit->s_order);
    }
    /*lint --e{506} ... yes, L2_MAX_LLAGS/L2_MAX_GLAGS are constants! */
    if((l2pit->itertype_pend & L2PORT_ITER_TYPE_LLAG) && (L2_MAX_LLAGS > 0)) {
        if(l2pit->ix < 0 || l2pit->ix == (L2_MAX_LLAGS-1)) {
            if(!switch_iter_getnext(&l2pit->sit)) {
                l2pit->itertype_pend &= ~L2PORT_ITER_TYPE_LLAG; /* Done LLAGs */
                l2pit->ix = -1;                                 /* If we're doing GLAGS too */
                goto glags;
            }
            l2pit->ix = 0;      /* Start new series */
        } else {
            l2pit->ix++;
        }
        l2pit->isid = l2pit->sit.isid;
        l2pit->usid = l2pit->sit.usid;
        l2pit->iport = VTSS_PORT_NO_NONE;
        l2pit->uport = VTSS_PORT_NO_NONE;
        l2pit->l2port = L2LLAG2PORT(l2pit->sit.isid, l2pit->ix);
        l2pit->type = L2PORT_ITER_TYPE_LLAG;
        return TRUE;
    }
glags:
    if((l2pit->itertype_pend & L2PORT_ITER_TYPE_GLAG) && (L2_MAX_GLAGS > 0)) {
        if(++l2pit->ix < L2_MAX_GLAGS) {
            l2pit->isid = VTSS_ISID_UNKNOWN;
            l2pit->usid = VTSS_ISID_UNKNOWN;
            l2pit->iport = VTSS_PORT_NO_NONE;
            l2pit->uport = VTSS_PORT_NO_NONE;
            l2pit->l2port = L2GLAG2PORT(l2pit->ix);
            l2pit->type = L2PORT_ITER_TYPE_GLAG;
            return TRUE;
        }
        l2pit->itertype_pend &= ~L2PORT_ITER_TYPE_GLAG; /* Done GLAGs*/
    }
    return FALSE;
}

/*
 ******************* L2 thread *************************
 */

/*
 * Message indication function
 */
static BOOL 
l2_msg_rx(void *contxt, 
               const void *rx_msg, 
               size_t len, 
               vtss_module_id_t modid, 
               ulong isid)
{
    const l2_msg_t *msg = (void*)rx_msg;
    T_N("Sid %u, rx %zd bytes, msg %d", isid, len, msg->msg_id);
    switch (msg->msg_id) {
    case L2_MSG_ID_FRAME_RX_IND:
    {
        l2_port_no_t l2port = (msg->data.rx_ind.glag_no == VTSS_GLAG_NO_NONE) ?
            L2PORT2PORT(isid, msg->data.rx_ind.switchport) : L2GLAG2PORT(msg->data.rx_ind.glag_no);
        l2_do_rx_callback(msg->data.rx_ind.modid, &msg[1],
                          msg->data.rx_ind.len, msg->data.rx_ind.vid, l2port);
        break;
    }
    case L2_MSG_ID_FRAME_TX_REQ: 
        if(!msg_switch_is_master()) { 
#ifdef VTSS_SW_OPTION_PACKET
            void *frame = packet_tx_alloc(msg->data.tx_req.len);
            if(frame) {
                packet_tx_props_t tx_props;
                memcpy(frame, &msg[1], msg->data.tx_req.len);
                packet_tx_props_init(&tx_props);
                tx_props.packet_info.modid     = VTSS_MODULE_ID_L2PROTO;
                tx_props.packet_info.frm[0]    = frame;
                tx_props.packet_info.len[0]    = msg->data.tx_req.len;
                tx_props.tx_info.dst_port_mask = VTSS_BIT64(msg->data.tx_req.switchport);
                (void)packet_tx(&tx_props);
            }
#endif
        }
        break;
    case L2_MSG_ID_SET_STP_STATE:
        if(msg_switch_is_master()) { 
            T_I("SET_STP_STATE: dropped (is master now)");
        } else {
            T_I("Slave: Switch port %u: new STP state %s", 
                msg->data.set_stp_state.switchport, 
                stpstatestring(msg->data.set_stp_state.stp_state));
            (void) vtss_stp_port_state_set(NULL,
                                           msg->data.set_stp_state.switchport, 
                                           msg->data.set_stp_state.stp_state);
        }
        break;
    case L2_MSG_ID_SET_MSTI_STP_STATE:
        if(msg_switch_is_master()) { 
            T_I("SET_MSTI_STP_STATE: dropped (is master now)");
        } else {
            T_I("Slave: Switch MSTI%d port %d: new STP state %s", 
                msg->data.set_msti_stp_state.msti, 
                msg->data.set_msti_stp_state.switchport, 
                stpstatestring(msg->data.set_msti_stp_state.stp_state));
            l2_set_msti_stpstate_local(msg->data.set_msti_stp_state.msti, 
                                       msg->data.set_msti_stp_state.switchport, 
                                       msg->data.set_msti_stp_state.stp_state);
        }
        break;
    case L2_MSG_ID_SET_MSTI_STP_STATE_ALL:
        if(msg_switch_is_master()) { 
            T_I("SET_MSTI_STP_STATE_ALL: dropped (is master now)");
        } else {
            T_I("Slave: Switch MSTIx port %d: new STP state %s", 
                msg->data.set_msti_stp_state.switchport, 
                stpstatestring(msg->data.set_msti_stp_state.stp_state));
            uchar msti;
            for(msti = 0; msti < N_L2_MSTI_MAX; msti++)
                l2_set_msti_stpstate_local(msti, 
                                           msg->data.set_msti_stp_state.switchport, 
                                           msg->data.set_msti_stp_state.stp_state);
        }
        break;
    case L2_MSG_ID_SYNC_STP_STATE:
        if(msg_switch_is_master()) { 
            T_I("SYNC_STP_STATE: dropped (is master now)");
        } else {
            port_iter_t pit;
            (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
            while (port_iter_getnext(&pit)) {
                vtss_stp_state_t state = msg->data.synch_stp_state.stp_states[pit.iport];
                T_I("Slave: Switch port %u: Synch to STP state %s", 
                    pit.iport, stpstatestring(state));
                (void) vtss_stp_port_state_set(NULL, pit.iport, state);
            }
        }
        break;
    case L2_MSG_ID_SET_STP_STATES:
        T_I("SET_STP_STATES");
        l2_local_set_states(msg->data.set_stp_states.switchport, 
                            msg->data.set_stp_states.port_state,
                            msg->data.set_stp_states.msti_state);
        break;
    case L2_MSG_ID_FLUSH_FDB:
        if(msg_switch_is_master()) { 
            T_I("FLUSH_FDB: dropped (is master now)");
        } else {
            l2local_flush_vport(msg->data.flush_fdb.type,
                                msg->data.flush_fdb.port,
                                msg->data.flush_fdb.vlan_id);
        }
        break;
    case L2_MSG_ID_FLUSH_FDB_STACK:
#if defined(VTSS_FEATURE_VSTAX_V2)
        T_D("Flush UPSID %d UPSPN %d", msg->data.flush_fdb_stack.upsid, msg->data.flush_fdb_stack.upspn);
        (void) vtss_mac_table_upsid_upspn_flush(NULL,
                                                msg->data.flush_fdb_stack.upsid,
                                                msg->data.flush_fdb_stack.upspn);
#endif  /* VTSS_FEATURE_VSTAX_V2 */
        break;
    case L2_MSG_ID_SET_MSTI_MAP:
        if(msg_switch_is_master()) { 
            T_I("SET_MSTI_MAP: dropped (is master now)");
        } else {
            (void) l2local_set_msti_map(msg->data.set_msti_map.all_to_cist,
                                        msg->data.set_msti_map.maplen,
                                        msg->data.set_msti_map.map);
        }
        break;
    default:
        T_W("Unhandled msg %d", msg->msg_id);
    }
    return TRUE;
}

/*
 * Stack Register
 */
static void
l2_stack_register(void)
{
    msg_rx_filter_t filter;    
    memset(&filter, 0, sizeof(filter));
    filter.cb = l2_msg_rx;
    filter.modid = VTSS_MODULE_ID_L2PROTO;
    vtss_rc rc =  msg_rx_filter_register(&filter);
    VTSS_ASSERT(rc == VTSS_OK);
}

/****************************************************************************/
/*  Initialization functions                                                */
/****************************************************************************/

vtss_rc
l2_init(vtss_init_data_t *data)
{
    switch (data->cmd) {
    case INIT_CMD_INIT:
        /* Initialize and register trace ressources */
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);
        break;
    case INIT_CMD_START:
        /* Register for stack messages */
        l2_stack_register();
        break;
    case INIT_CMD_MASTER_UP: {
         /* All ports shall be discarding at startup */
        vtss_common_stpstate_t common_stp_state;
        vtss_stp_state_t       stp_state;
        int                    i;

#ifdef VTSS_SW_OPTION_MSTP
        common_stp_state = VTSS_COMMON_STPSTATE_DISCARDING;
        stp_state        = VTSS_STP_STATE_DISCARDING;
#else
        // When STP is not part of the product (but L2Proto is), we need
        // to initialize the STP state to FORWARDING or no-one else will.
        common_stp_state = VTSS_COMMON_STPSTATE_FORWARDING;
        stp_state        = VTSS_STP_STATE_FORWARDING;
#endif
        for (i = 0; i < (int)ARRSZ(cached_stpstates); i++) {
            cached_stpstates[i] = common_stp_state;
        }
        for (i = 0; i < (int)ARRSZ(port_stp_state); i++) {
            port_stp_state[i] = stp_state;
        }
    }
        break;
    case INIT_CMD_SWITCH_ADD:
        l2_sync_stpstates_switch(data->isid); /* Bulk update STP port states */
        break;
    default:
        break;
    }
    
    return VTSS_RC_OK;
}

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
