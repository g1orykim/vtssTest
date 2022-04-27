/*

 Vitesse Switch API software.

 Copyright (c) 2002-2014 Vitesse Semiconductor Corporation "Vitesse". All
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

#include "mstp_api.h"           /* Our module API */
#include "mstp.h"               /* Our private definitions */

#include "vtss_mstp_callout.h"  /* mstp_fwdstate_t */

/* Used APIs */
#include "critd_api.h"
#include "packet_api.h"
#include "msg_api.h"
#include "conf_api.h"
#include "vlan_api.h"
#include "misc_api.h"           /* instantiate MAC */
#ifdef VTSS_SW_OPTION_DOT1X
#include "dot1x_api.h"
#endif /* VTSS_SW_OPTION_DOT1X */
#if VTSS_SWITCH_STACKABLE
#include "topo_api.h"           /* topo_isid2mac() */
#endif

#ifdef VTSS_SW_OPTION_VCLI
#include "mstp_cli.h"
#endif
#ifdef VTSS_SW_OPTION_ICLI
#include "mstp_icfg.h"
#endif


static const u8 ieee_bridge[6] = { (u8) 0x01, (u8) 0x80, (u8) 0xC2, 
                                   (u8) 0x00, (u8) 0x00, (u8) 0x00 };

/* ================================================================= *
 *  Trace definitions
 * ================================================================= */

#define VTSS_MODULE_ID_MSTP VTSS_MODULE_ID_RSTP
#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_MSTP
#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_MSTP

//#define VTSS_TRACE_GRP_DEFAULT 0
#define VTSS_TRACE_GRP_CONTROL   1
#define VTSS_TRACE_GRP_INTERFACE 2
#define TRACE_GRP_CNT          3

#define _C VTSS_TRACE_GRP_CONTROL
#define _I VTSS_TRACE_GRP_INTERFACE

#if (VTSS_TRACE_ENABLED)

static vtss_trace_reg_t trace_reg =
{ 
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "mstp",
    .descr     = "Spanning Tree"
};

static vtss_trace_grp_t trace_grps[TRACE_GRP_CNT] =
{
    [VTSS_TRACE_GRP_DEFAULT] = { 
        .name      = "default",
        .descr     = "Default (MSTP core)",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [VTSS_TRACE_GRP_CONTROL] = { 
        .name      = "control",
        .descr     = "MSTP control",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [VTSS_TRACE_GRP_INTERFACE] = { 
        .name      = "interface",
        .descr     = "MSTP Core interfaces",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
};
#endif /* VTSS_TRACE_ENABLED */

/* Thread variables */
static cyg_handle_t mstp_thread_handle;
static cyg_thread   mstp_thread_block;
static char         mstp_thread_stack[THREAD_DEFAULT_STACK_SIZE];

typedef struct {
    /* Pending changes */
    uchar			change[VTSS_BF_SIZE(MSTP_AGGR_PORTS)];
#if L2_MAX_LLAGS > 0
    /* Current STP LLAG state */
    llag_participants_t 	llag[VTSS_ISID_CNT][L2_MAX_LLAGS+VTSS_PORT_NO_START];
#endif
#if defined(VTSS_FEATURE_AGGR_GLAG)
    /* Current STP GLAG state */
    glag_participants_t 	glag[L2_MAX_GLAGS+VTSS_PORT_NO_START];
#else
    glag_participants_t 	glag[1]; /* Dummy entry */
#endif
    /* All ports fwd state */
    mstp_fwdstate_t		stpstate[MSTP_BRIDGE_PORTS+VTSS_PORT_NO_START];
    /* Physical ports -> aggregation */
    u16				parent[MSTP_PHYS_PORTS+VTSS_PORT_NO_START];
} mstp_astate_t;

typedef struct {
    uint n_members;
    uchar members[VTSS_BF_SIZE(MSTP_PHYS_PORTS+VTSS_PORT_NO_START)];
} mstp_mstate_t;

/* MSTP global data */
static struct {
    BOOL ready;                 /* MSTP Initited & we're acting master */
    cyg_flag_t control_flags;   /* MSTP thread control */
    critd_t mutex;              /* Global module/API protection */
    cyg_sem_t defconfig_sema;   /* Signal completion of load defaults from MSTP worker thread => thread running INIT_CMD_CONF_DEF */
    u32 switch_sync;            /* Pending switch sync-ups */
    mstp_astate_t aggr;         /* AGGR state */
    mstp_mstate_t mstate[N_MSTI_MAX]; /* MSTI state */
    mstp_conf_t conf;           /* Current configuration */
    mac_addr_t sysmac[VTSS_ISID_CNT]; /* Switch system MACs */
    u32 traps;                        /* Aggregated trap state */
    mstp_trap_sink_t trap_cb;         /* Trap sink */
    mstp_config_change_cb_t config_cb; /* Config callback */
    mstp_bridge_t *mstpi;             /* MSTP instance handle */
} mstp_global;

/*
 * Forward defs
 */

static void
mstp_enslave(l2_port_no_t l2aggr, l2_port_no_t l2phys);

static void
mstp_liberate(l2_port_no_t l2aggr, l2_port_no_t l2phys);

/*
 * Aggregation abstraction, ( Poor Man's C++ :-) )
 */

#if L2_MAX_LLAGS > 0
static uint
_llag_count(struct mstp_aggr_obj const *aob)
{
    llag_participants_t *llag = aob->data_handle;
    return llag->cmn.n_members;
}

static l2_port_no_t 
_llag_first_port(struct mstp_aggr_obj const *aob)
{
    llag_participants_t *llag = aob->data_handle;
    l2_port_no_t l2port = L2_NULL;
    if(llag->cmn.n_members)
        l2port = llag->cmn.port_min + aob->u.llag.port_offset;
    T_NG(_C, "ret %d", l2port);
    return l2port;
}

static l2_port_no_t
_llag_next_port(struct mstp_aggr_obj const *aob, 
               l2_port_no_t l2port)
{
    llag_participants_t *llag = aob->data_handle;
    u16 ix = (u16) (l2port - aob->u.llag.port_offset);
    l2_port_no_t l2ret = L2_NULL;
    while(++ix <= llag->cmn.port_max)
        if(MSTP_AGGR_GET_MEMBER(ix, llag)) {
            l2ret = ix + aob->u.llag.port_offset;
            break;
        }

    T_NG(_C, "ret %d", l2ret);
    return l2ret;
}

/*lint -sem(_llag_update, thread_protected) ... We are locked already */
static void
_llag_update(struct mstp_aggr_obj *aob)
{
    llag_participants_t *llag = aob->data_handle, oldstate, *pold = &oldstate;
    port_iter_t pit;
    aggr_mgmt_group_member_t am;
    T_DG(_C, "port %d", aob->l2port);
    MSTP_ASSERT_LOCKED();
    *pold = *llag;
    memset(llag, 0, sizeof(*llag));
    if(aggr_mgmt_members_get(aob->u.llag.isid, aob->u.llag.aggr_no, &am, FALSE) == VTSS_OK) {
        (void) port_iter_init(&pit, NULL, aob->u.llag.isid, 
                              PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            if(am.entry.member[pit.iport]) {
                T_DG(_C, "isid %d, LLAG aggr %u, switch port %u", 
                     aob->u.llag.isid, aob->u.llag.aggr_no, pit.iport);
                if(llag->cmn.n_members == 0)
                    llag->cmn.port_min = llag->cmn.port_max = pit.iport;
                else
                llag->cmn.port_max = pit.iport;
                llag->cmn.n_members++;
                MSTP_AGGR_SET_MEMBER(pit.iport, llag, 1);
            }
        }
    }
    /* Check for new/departed physical ports */
    (void) port_iter_init(&pit, NULL, aob->u.llag.isid, 
                          PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
    while (port_iter_getnext(&pit)) {
        if(MSTP_AGGR_GET_MEMBER(pit.iport, llag)) {
            /* Not Member -> Member */
            if(!MSTP_AGGR_GET_MEMBER(pit.iport, pold))
                mstp_enslave(aob->l2port, pit.iport + aob->u.llag.port_offset);
        } else {
            /* Member -> Not member */
            if(MSTP_AGGR_GET_MEMBER(pit.iport, pold))
                mstp_liberate(aob->l2port, pit.iport + aob->u.llag.port_offset);
        }
    }
}

/* NOTE: This is called from mstp_enslave(), which is *always* called
 * in a critical region. Hence no locking needed.
 */
static void
_llag_remove_port(struct mstp_aggr_obj const *aob, 
                  l2_port_no_t l2port)
{
    llag_participants_t *llag = aob->data_handle;
    u16 ix = (u16) (l2port - aob->u.llag.port_offset);
    T_IG(_C, "%s remove port %d - ix %u", l2port2str(aob->l2port), l2port, ix);
    VTSS_ASSERT(MSTP_AGGR_GET_MEMBER(ix, llag) == 1);
    MSTP_AGGR_SET_MEMBER(ix, llag, 0);
}

#endif /* L2_MAX_LLAGS > 0 */

static uint
_glag_count(struct mstp_aggr_obj const *aob)
{
    glag_participants_t *glag = aob->data_handle;
    return glag->cmn.n_members;
}

static l2_port_no_t 
_glag_first_port(struct mstp_aggr_obj const *aob)
{
    glag_participants_t *glag = aob->data_handle;
    return glag->cmn.n_members ? glag->cmn.port_min : L2_NULL;
}

static l2_port_no_t
_glag_next_port(struct mstp_aggr_obj const *aob, 
               l2_port_no_t l2port)
{
    glag_participants_t *glag = aob->data_handle;
    while(++l2port <= glag->cmn.port_max)
        if(MSTP_AGGR_GET_MEMBER(l2port, glag))
            return l2port;
                
    return L2_NULL;
}

/*lint -sem(_glag_update, thread_protected) ... We are locked already */
static void
_glag_update(struct mstp_aggr_obj *aob)
{
    glag_participants_t *glag = aob->data_handle, oldstate, *pold = &oldstate;
    vtss_isid_t isid;
    l2port_iter_t l2pit;
    T_DG(_C, "port %d", aob->l2port);
    MSTP_ASSERT_LOCKED();
    *pold = *glag;
    memset(glag, 0, sizeof(*glag));
    for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
        aggr_mgmt_group_member_t am;
        aggr_mgmt_group_no_t aggr_no = aob->u.glag.glag - AGGR_MGMT_GROUP_NO_START + AGGR_MGMT_GLAG_START;
        if(msg_switch_exists(isid) &&
           aggr_mgmt_members_get(isid, aggr_no, &am, FALSE) == VTSS_OK) {
            (void) l2port_iter_init(&l2pit, isid, L2PORT_ITER_TYPE_PHYS);
            while (l2port_iter_getnext(&l2pit)) {
                if(am.entry.member[l2pit.iport]) {
                    VTSS_ASSERT(l2pit.l2port < L2_MAX_PORTS);
                    T_DG(_C, "isid %d, GLAG aggr %u, port %u => l2port %u", 
                         isid, aggr_no, l2pit.iport, l2pit.l2port);
                    if(glag->cmn.n_members == 0)
                        glag->cmn.port_min = glag->cmn.port_max = l2pit.l2port;
                    else
                        glag->cmn.port_max = l2pit.l2port;
                    glag->cmn.n_members++;
                    MSTP_AGGR_SET_MEMBER(l2pit.l2port, glag, 1);
                }
            }
        }
    }
    (void) l2port_iter_init(&l2pit, VTSS_ISID_GLOBAL, L2PORT_ITER_TYPE_PHYS);
    while(l2port_iter_getnext(&l2pit)) {
        if(MSTP_AGGR_GET_MEMBER(l2pit.l2port, glag)) {
            /* Not Member -> Member */
            if(!MSTP_AGGR_GET_MEMBER(l2pit.l2port, pold))
                mstp_enslave(aob->l2port, l2pit.l2port);
        } else {
            /* Member -> Not member */
            if(MSTP_AGGR_GET_MEMBER(l2pit.l2port, pold))
                mstp_liberate(aob->l2port, l2pit.l2port);
        }
    }
}

/* NOTE: This is called from mstp_enslave(), which is *always* called
 * in a critical region. Hence no locking needed.
 */
static void
_glag_remove_port(struct mstp_aggr_obj const *aob, 
                  l2_port_no_t l2port)
{
    glag_participants_t *glag = aob->data_handle;
    T_IG(_C, "%s remove port %d", l2port2str(aob->l2port), l2port);
    VTSS_ASSERT(MSTP_AGGR_GET_MEMBER(l2port, glag) == 1);
    MSTP_AGGR_SET_MEMBER(l2port, glag, 0);
}

#if L2_MAX_LLAGS > 0
static const mstp_aggr_objh_t 
_llag_handler =  { _llag_count, _llag_first_port, _llag_next_port, _llag_update, _llag_remove_port } ;
#endif /* L2_MAX_LLAGS > 0 */
static const mstp_aggr_objh_t 
_glag_handler = { _glag_count, _glag_first_port, _glag_next_port, _glag_update, _glag_remove_port } ;

mstp_aggr_obj_t *mstp_get_aggr(mstp_aggr_obj_t * paob, l2_port_no_t l2port)
{
    VTSS_ASSERT(!l2port_is_port(l2port));
    if(l2port_is_glag(l2port)) {
        (void) l2port2glag(l2port, &paob->u.glag.glag);
        paob->data_handle = &mstp_global.aggr.glag[paob->u.glag.glag-AGGR_MGMT_GROUP_NO_START];
        paob->handler = &_glag_handler;
    } else {
#if L2_MAX_LLAGS > 0
        if (l2port2poag(l2port, &paob->u.llag.isid, &paob->u.llag.aggr_no)) {
            paob->u.llag.port_offset = L2PORT2PORT(paob->u.llag.isid, VTSS_PORT_NO_START);
            paob->data_handle = 
                &mstp_global.aggr.llag
                [paob->u.llag.isid-VTSS_ISID_START]
                [paob->u.llag.aggr_no-AGGR_MGMT_GROUP_NO_START];
            paob->handler = &_llag_handler;
        } else {
            T_E("L2port(%u) is NOT valid", l2port);
            return NULL;
        }
#else
        VTSS_ASSERT(FALSE);
#endif /* L2_MAX_LLAGS > 0 */

    }
    paob->l2port = l2port;
    return paob;
}

static void mstp_signal_switch_sync(vtss_isid_t isid)
{
    T_IG(_C, "Switch sync - ISID %u", isid);
    MSTP_LOCK();
    mstp_global.switch_sync |= (1 << isid);
    cyg_flag_setbits(&mstp_global.control_flags, CTLFLAG_MSTP_SWITCH_SYNC);
    MSTP_UNLOCK();
}

/* The values shown [in Table 17-3] apply to both full duplex and half
 * duplex operation. The intent of the recommended values and ranges
 * shown is to minimize the number of Bridges in which path costs need to
 * be managed to exert control over the topology of the Bridged Local
 * Area Network.
 */
static uint portspeed(vtss_port_speed_t speed)
{
    switch (speed) {
    case VTSS_SPEED_10M:
        return 10;
    case VTSS_SPEED_100M:
        return 100;
    case VTSS_SPEED_1G:
        return 1000;
    case VTSS_SPEED_2500M:
        return 2500;
    case VTSS_SPEED_5G:
        return 5000;
    case VTSS_SPEED_10G:
        return 10000;
    default:
        return 0;
    }
}

static uint aggrspeed(mstp_aggr_obj_t *pa)
{
    uint aspeed = 0, members = pa->handler->members(pa);
    l2_port_no_t l2port;
    VTSS_ASSERT(members > 0);
    for(l2port = pa->handler->first_port(pa); l2port != L2_NULL; 
        l2port = pa->handler->next_port(pa, l2port)) {
        vtss_port_no_t switchport;
        vtss_isid_t isid;
        port_status_t ps;
        if(l2port2port(l2port, &isid, &switchport) &&
           msg_switch_exists(isid) &&
           port_mgmt_status_get(isid, switchport, &ps) == VTSS_OK &&
           ps.status.link)
            aspeed += portspeed(ps.status.speed);
    }
    T_DG(_C, "Aggregated speed: %u - avg %u", aspeed, aspeed/members);
    return aspeed;
}

const char *
msti_name(u8 msti)
{
    static const char * const mstinames[N_MSTI_MAX] = {
        "CIST", "MSTI1", "MSTI2", "MSTI3", "MSTI4", "MSTI5", "MSTI6", "MSTI7",
    };
    return msti < N_MSTI_MAX ? mstinames[msti] : "?";
}

static inline char const *fwd2str(mstp_fwdstate_t state)
{
    switch(state) {
    case MSTP_FWDSTATE_BLOCKING:
        return "Discarding";    /* This is what STP calls it */
    case MSTP_FWDSTATE_LEARNING:
        return "Learning";
    case MSTP_FWDSTATE_FORWARDING:
        return "Forwarding";
    default:
        return "<unknown>";
    }
}

static void
mstp_set_port_stpstate(l2_port_no_t portnum,
                       mstp_fwdstate_t state)
{
    T_DG(_C, "Set %s state %s -> %s", l2port2str(portnum), 
         fwd2str(mstp_global.aggr.stpstate[portnum]), fwd2str(state));
    if(mstp_global.aggr.stpstate[portnum] != state) {
        T_IG(_C, "Change %s state %s -> %s", l2port2str(portnum),
             fwd2str(mstp_global.aggr.stpstate[portnum]), fwd2str(state));
        switch(state) {
        case MSTP_FWDSTATE_BLOCKING:
            vtss_os_set_stpstate(portnum, VTSS_STP_STATE_DISCARDING);
            break;
        case MSTP_FWDSTATE_LEARNING:
            vtss_os_set_stpstate(portnum, VTSS_STP_STATE_LEARNING);
            break;
        case MSTP_FWDSTATE_FORWARDING:
            vtss_os_set_stpstate(portnum, VTSS_STP_STATE_FORWARDING);
            break;
        default:
            abort();
        }
        mstp_global.aggr.stpstate[portnum] = state;
    }
}
  
static void
mstp_aggr_sync_ports(l2_port_no_t portnum,
                     mstp_aggr_obj_t *pa)
{
    l2_port_no_t l2port;
    VTSS_ASSERT(!l2port_is_port(portnum));
    /* Set STP state for all members */
    mstp_set_port_stpstate(portnum, MSTP_FWDSTATE_FORWARDING);
    for(l2port = pa->handler->first_port(pa); l2port != L2_NULL; 
        l2port = pa->handler->next_port(pa, l2port))
        l2_sync_stpstates(l2port, portnum);
}

static void
mstp_set_all_stpstate(l2_port_no_t portnum,
                      mstp_fwdstate_t state)
{
    mstp_set_port_stpstate(portnum, state);
    l2_set_msti_stpstate_all(portnum, (vtss_stp_state_t)state);
}

static void
mstp_vlan_ingress_filter(l2_port_no_t l2port, BOOL enable)
{
    vtss_port_no_t switchport;
    vtss_isid_t isid;
    vlan_port_conf_t vlan_pconf;
    vtss_rc rc;

    memset(&vlan_pconf, 0, sizeof(vlan_pconf));
    if(enable) {
        vlan_pconf.flags = VLAN_PORT_FLAGS_INGR_FILT;
        vlan_pconf.ingress_filter = enable;
    }

    T_IG(_C, "Set l2port %s VLAN filtering %sabled", l2port2str(l2port), enable ? "en" : "dis");

    if(l2port2port(l2port, &isid, &switchport)) {
        /* The call might fail if we've become slave - benign */
        if ((rc = vlan_mgmt_port_conf_set(isid, switchport, &vlan_pconf, VLAN_USER_MSTP)) != VTSS_RC_OK &&
            rc != VLAN_ERROR_MUST_BE_MASTER) {
            T_E("%u:%u: %s", isid, iport2uport(switchport), error_txt(rc));
        }
    } else {
        T_E("Set l2port %s VLAN filtering %d - not a port", l2port2str(l2port), enable);
    }
}

static void
activate_port(l2_port_no_t l2port, 
              u32 linkspeed, 
              BOOL fdx, 
              const char *reason)
{
    BOOL doadd;
    doadd = !vtss_mstp_port_added(mstp_global.mstpi, L2PORT2API(l2port));
    T_I(reason);
    /*
     * Enable *port* forwarding, but block all MSTI's.
     */
    mstp_set_port_stpstate(l2port, MSTP_FWDSTATE_FORWARDING);
    l2_set_msti_stpstate_all(l2port, MSTP_FWDSTATE_BLOCKING);
    /* Enable Ingress filtering. */
    mstp_vlan_ingress_filter(l2port, TRUE);
    /* Add/kick the port */
    vtss_mstp_stm_lock(mstp_global.mstpi);
    if(doadd) {
        if(!vtss_mstp_add_port(mstp_global.mstpi, L2PORT2API(l2port)))
            T_EG(_C, "Error adding RSTP port %d - %s", l2port, l2port2str(l2port));
    } else {
        if(!vtss_mstp_reinit_port(mstp_global.mstpi, L2PORT2API(l2port)))
            T_EG(_C, "Error reinit RSTP port %d - %s", l2port, l2port2str(l2port));
    }
    if(!vtss_mstp_port_enable(mstp_global.mstpi, L2PORT2API(l2port), TRUE, linkspeed, fdx))
        T_EG(_C, "Error enabling RSTP port %u - %s at speed %u", l2port, l2port2str(l2port), linkspeed);
    vtss_mstp_stm_unlock(mstp_global.mstpi);
}

static void
deactivate_port(l2_port_no_t l2port, 
                mstp_fwdstate_t state,
                BOOL ingressfilter, 
                const char *reason)
{
    if(reason)
        T_IG(_C, reason);
    if(vtss_mstp_port_added(mstp_global.mstpi, L2PORT2API(l2port))) {   /* Must delete */
        (void) vtss_mstp_delete_port(mstp_global.mstpi, L2PORT2API(l2port));
    }
    /* Set Ingress filtering. */
    mstp_vlan_ingress_filter(l2port, ingressfilter);
    /* Set state (for all MSTI's) */
    mstp_set_all_stpstate(l2port, state);
}

/*
 * According to configuration & link state -
 * Instruct core RSTP/MSTP likewise
 */
static void
port_sync(l2_port_no_t l2port, port_info_t *info)
{
    BOOL enable = mstp_global.conf.stp_enable[l2port];
    l2_port_no_t l2aggr = mstp_global.aggr.parent[l2port];
    T_IG(_C, "port %d enb %d added %d", l2port, enable, 
         vtss_mstp_port_added(mstp_global.mstpi, L2PORT2API(l2port)));
    if(l2aggr != L2_NULL) {
        /* 
         * Port is part of aggregation, just sync physical port to the
         * aggregated port.
         */
        T_IG(_C, "syncing aggregated port to %s", l2port2str(l2aggr));
        deactivate_port(l2port, mstp_global.aggr.stpstate[l2aggr], 
                        /* Use ingress filtering on aggregated port - IFF running STP */
                        mstp_global.conf.stp_enable[MSTP_PORT_CONFIG_AGGR],
                        "aggr: sync up");
        return;
    }
    if(enable) {
        if(info) {
            /* Link change */
            if(info->link)
                activate_port(l2port, portspeed(info->speed), info->fdx, "portstate: link up");
            else
                deactivate_port(l2port, MSTP_FWDSTATE_BLOCKING, FALSE, "portstate: link down");
        } else {
            /* Initial sync_up */
            vtss_port_no_t switchport;
            vtss_isid_t isid;
            if(l2port2port(l2port, &isid, &switchport) && 
               msg_switch_exists(isid) &&
               port_isid_port_no_is_front_port(isid, switchport)) {
                port_status_t ps;
                vtss_rc rc;
                if((rc = port_mgmt_status_get(isid, switchport, &ps)) == VTSS_OK &&
                   ps.status.link)
                    activate_port(l2port, portspeed(ps.status.speed), ps.status.fdx, "sync: link up");
                else {
                    T_IG(_C, "[%d,%u] rc: %d link: %d", isid, switchport, rc, ps.status.link);
                    deactivate_port(l2port, MSTP_FWDSTATE_BLOCKING, FALSE, "no link/no port");
                }
            }
        }
    } else {
        deactivate_port(l2port, MSTP_FWDSTATE_FORWARDING, FALSE, "nonstp: fwd"); /* Just plain enable */
    }
}

static void
sync_ports_switch(vtss_isid_t isid)
{
    port_iter_t pit;
    T_IG(_C, "sync switch %d", isid);
    MSTP_ASSERT_LOCKED();
    (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
    while (port_iter_getnext(&pit)) {
        port_sync(L2PORT2PORT(isid, pit.iport), NULL);
    }
}

/*
 * Synchronize front port states (upon startup/restore defaults)
 */
static void
sync_ports_all(void)
{
    switch_iter_t sit;
    vtss_mstp_stm_lock(mstp_global.mstpi);
    (void)switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID);
    while (switch_iter_getnext(&sit)) {
        sync_ports_switch(sit.isid);
    }
    vtss_mstp_stm_unlock(mstp_global.mstpi);
}

/*
 * Enslave a Physical port into an aggregation
 */
static void
mstp_enslave(l2_port_no_t l2aggr, l2_port_no_t l2phys)
{
    T_IG(_C, "%d enslaving %s", l2aggr, l2port2str(l2phys));
    if(mstp_global.aggr.parent[l2phys] != L2_NULL) {
        l2_port_no_t oldparen = mstp_global.aggr.parent[l2phys];
        mstp_aggr_obj_t aob, *paob;
        T_IG(_C, "*** Aggregated port changing parent! %d had %s as parent", l2phys, l2port2str(oldparen));
        paob = mstp_get_aggr(&aob, oldparen);
        if (paob) {
            paob->handler->remove_port(paob, l2phys);
            if(!MSTP_AGGR_GETSET_CHANGE(oldparen, 1)) {
                T_WG(_C, "Revisit %s for update", l2port2str(oldparen));
                cyg_flag_setbits(&mstp_global.control_flags, CTLFLAG_MSTP_AGGRCHANGE);
            }
        }
    }
    mstp_global.aggr.parent[l2phys] = l2aggr;
    /* 
     * Stop the l2 physical STP port, and sync to STP state for the
     * aggregated port
     */
    deactivate_port(l2phys, mstp_global.aggr.stpstate[l2aggr],
                    mstp_global.conf.stp_enable[MSTP_PORT_CONFIG_AGGR], "enslave");
    /* Now sync the physical port MSTIs to the aggregated port MSTIs */
    l2_sync_stpstates(l2phys, l2aggr);
}

/*
 * Enslave a Physical port into an aggregation
 */
static void
mstp_liberate(l2_port_no_t l2aggr, l2_port_no_t l2phys)
{
    T_IG(_C, "%d liberating %s", l2aggr, l2port2str(l2phys));
    mstp_global.aggr.parent[l2phys] = L2_NULL;
    port_sync(l2phys, NULL);
}

/*
 * Determine Port VID membership, map to MSTI
 */
static void
mstp_get_members(u8 msti, vtss_vid_t vid)
{
    uint count = 0;
    vlan_mgmt_entry_t vconf;
    mstp_mstate_t *state = &mstp_global.mstate[msti];
    switch_iter_t sit;

    /* Loop all switches in stack, query for VID membership */
    (void)switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID);
    while (switch_iter_getnext(&sit)) {
        if(vlan_mgmt_vlan_get(sit.isid, vid, &vconf, FALSE, VLAN_USER_ALL) == VTSS_OK) {
            l2port_iter_t l2pit;
            (void) l2port_iter_init(&l2pit, sit.isid, L2PORT_ITER_TYPE_PHYS);
            while (l2port_iter_getnext(&l2pit)) {
                if (vconf.ports[l2pit.iport] == 1) {
                    if(mstp_global.conf.stp_enable[l2pit.l2port] &&
                       !VTSS_BF_GET(state->members, l2pit.l2port)) {
                        VTSS_BF_SET(state->members, l2pit.l2port, 1);
                        state->n_members++;
                    }
                    count++;
                    T_D("%d,%d => %d is member of VID %d in MST%d", sit.isid, l2pit.iport, l2pit.l2port, vid, msti);
                }
            }
        }
    }
    T_I("Vid %d had %d members, MST%d now at %d members", vid, count, msti, state->n_members);
}

static void
save_config(void)
{
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    mstp_conf_blk_t *blk;
    ulong size;
#if defined(VTSS_MSTP_FULL)
    /* No limitations */
#else                                   
    if(mstp_global.conf.sys.forceVersion > MSTP_PROTOCOL_VERSION_RSTP)
        /* Only allowed to use RSTP/STP */
        mstp_global.conf.sys.forceVersion = MSTP_PROTOCOL_VERSION_RSTP; 
#endif  /* VTSS_MSTP_FULL */
    if ((blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_MSTP_CONF, &size)) != NULL) {
        if(size == sizeof(*blk)) {
            T_IG(_C, "Saving configuration");
            blk->conf = mstp_global.conf;
        }
        conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_MSTP_CONF);
    }
#else
    T_N("Silent-upgrade build: Not saving to conf");
#endif
    cyg_flag_setbits(&mstp_global.control_flags, CTLFLAG_MSTP_CONFIG_CHANGE);
}

/*
 * Update internal state MSTI state, and apply MSTI configuration to
 * base MSTP.
 */
static BOOL
mstp_apply_msticonfig(void)
{
    vtss_vid_t i;
    u8 msti;
    BOOL rc;

    MSTP_ASSERT_LOCKED();

    /* Reset state */
    memset(mstp_global.mstate, 0, sizeof(mstp_global.mstate));
    mstp_global.traps = 0;

    /* Determine membership for all VLANs, which have a MSTI mapping */
    for(i = MSTP_MIN_VID; i < MSTP_MAX_VID; i++) {
        msti = mstp_global.conf.msti.map.map[i];
        if(msti)
            mstp_get_members(msti, i);
    }

    vtss_mstp_stm_lock(mstp_global.mstpi); /* Don't run STMs while applying */

    rc = vtss_mstp_set_bridge_parameters(mstp_global.mstpi, &mstp_global.conf.sys);
    vtss_mstp_set_config_id(mstp_global.mstpi, 
                            mstp_global.conf.msti.configname, 
                            mstp_global.conf.msti.revision);
    rc = rc && vtss_mstp_set_mapping(mstp_global.mstpi, &mstp_global.conf.msti.map);
    
    BOOL single_mode = (mstp_global.conf.sys.forceVersion < MSTP_PROTOCOL_VERSION_MSTP);
    rc = rc && (VTSS_OK == l2_set_msti_map(single_mode,
                                           ARRSZ(mstp_global.conf.msti.map.map), 
                                           mstp_global.conf.msti.map.map));
    
    for(msti = 0; msti < N_MSTI_MAX; msti++)
        (void) vtss_mstp_set_bridge_priority(mstp_global.mstpi, 
                                             msti,
                                             mstp_global.conf.bridgePriority[msti]);
    vtss_mstp_stm_unlock(mstp_global.mstpi);

    T_I("Operation %s", rc ? "succedded" : "failed");

    return rc;
}

/*
 * Propagate the MSTP (module) configuration to the MSTP/RSTP core
 * library.
 */
static void
mstp_conf_propagate(BOOL bridge, BOOL ports)
{
    MSTP_ASSERT_LOCKED();

    VTSS_ASSERT(mstp_global.mstpi != NULL);
    /* Make effective in MSTP core */
    if(bridge) {
        (void) mstp_apply_msticonfig();
    }
    if(ports) {
        uint i;
        for(i = MSTP_CONF_PORT_FIRST; i <= MSTP_CONF_PORT_LAST; i++) {

            const mstp_port_param_t *pconf = &mstp_global.conf.portconfig[i];
            (void) vtss_mstp_set_port_parameters(mstp_global.mstpi, L2PORT2API(i), pconf);
            
            int msti;
            for(msti = 0; msti < N_MSTI_MAX; msti++) {
                mstp_msti_port_param_t *mpp = &mstp_global.conf.msticonfig[i][msti];
                (void) vtss_mstp_set_msti_port_parameters(mstp_global.mstpi, msti, L2PORT2API(i), mpp);
            }
        }
    }
}

/*
 * Read the MSTP/RSTP configuration. @create indicates a new default
 * configuration block should be created.
 */
static void mstp_conf_read(BOOL create)
{
    mstp_conf_blk_t *blk;
    ulong           size;
    BOOL            do_create = create;
    mac_addr_t      sysmac;

    if (misc_conf_read_use()) {
        if ((blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_MSTP_CONF, &size)) == NULL ||
            size != sizeof(*blk)) {
            blk = conf_sec_create(CONF_SEC_GLOBAL, CONF_BLK_MSTP_CONF, sizeof(*blk));
            T_WG(_C, "conf_sec_open failed or size mismatch, creating defaults");
            do_create = TRUE;
        } else if(blk->version != MSTP_CONF_VERSION) {
            T_WG(_C, "version mismatch, creating defaults");
            do_create = TRUE;
        }

        if (!blk) {
            T_E("Unable to create MSTP config");
            return;
        }
    }
    else {
        T_N("no silent upgrade; creating defaults");
        do_create = TRUE;
        blk       = NULL;
    }

    if (do_create) {
        /* Use default configuration */
        memset(&mstp_global.conf, 0, sizeof(mstp_global.conf));
        mstp_global.conf.sys.bridgeMaxAge = 20; /* 17.14 - Table 17-1: Default recommended value */
        mstp_global.conf.sys.bridgeHelloTime = 2; /* 17.14 - Table 17-1: Default recommended value */
        mstp_global.conf.sys.bridgeForwardDelay = 15; /* 17.14 - Table 17-1: Default recommended value */
        mstp_global.conf.sys.forceVersion = 3; /* 17.13.4 - The normal, default value */
        mstp_global.conf.sys.txHoldCount = 6; /* 17.14 - Table 17-1: Default recommended value */
        mstp_global.conf.sys.MaxHops = 20; /* 13.37.3 MaxHops */

        /* Get System MAC address */
        (void)conf_mgmt_mac_addr_get(sysmac, 0);
        (void)misc_mac_txt(sysmac, mstp_global.conf.msti.configname);

        mstp_global.conf.msti.revision = 0;
        uint i,j;
        for(i = 0; i < N_MSTI_MAX; i++)
            mstp_global.conf.bridgePriority[i] = 0x80; /* 17.14 - Table 17-2: Default recommended value */
        for(i = 0; i < ARRSZ(mstp_global.conf.portconfig); i++) {
            mstp_port_param_t *pp = &mstp_global.conf.portconfig[i];
            mstp_global.conf.stp_enable[i] = TRUE;
            pp->adminEdgePort = FALSE;
            pp->adminAutoEdgePort = TRUE;
            pp->adminPointToPointMAC = P2P_AUTO;
            for(j = 0; j < N_MSTI_MAX; j++) {
                mstp_msti_port_param_t *mpp = &mstp_global.conf.msticonfig[i][j];
                mpp->adminPathCost = MSTP_PORT_PATHCOST_AUTO; /* 0 = Auto */
                mpp->adminPortPriority = 0x80; /* 17.14 - Table 17-2: Default recommended value */
            }
        }
        /* Use different defaults for aggregated ports */
        mstp_global.conf.stp_enable[MSTP_PORT_CONFIG_AGGR] = TRUE;
        mstp_global.conf.portconfig[MSTP_PORT_CONFIG_AGGR].adminPointToPointMAC = P2P_FORCETRUE;
    } else {
        if (blk) {
            mstp_global.conf = blk->conf;
        }
    }
#if defined(VTSS_MSTP_FULL)
    /* No limitations */
#else                                   
    if(mstp_global.conf.sys.forceVersion > MSTP_PROTOCOL_VERSION_RSTP)
        /* Only allowed to use RSTP/STP */
        mstp_global.conf.sys.forceVersion = MSTP_PROTOCOL_VERSION_RSTP;
#endif  /* VTSS_MSTP_FULL */

    if (blk) {
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
        memset(blk, 0, sizeof(*blk));
        blk->conf = mstp_global.conf;
        blk->version = MSTP_CONF_VERSION;
        conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_MSTP_CONF);
#endif
    }
}

/****************************************************************************
 * Utility
 ****************************************************************************/

// Append string to str safely.
static size_t strfmt_append(
    char         *str,
    size_t        size,   // Size of str
    const char   *fmt,
    ...)
{
    va_list ap = NULL;
    size_t len = strlen(str);
    size_t cnt;

    VTSS_ASSERT(len < size);

    va_start(ap, fmt);
    cnt = vsnprintf(&str[len], size-len, fmt, ap);
    va_end(ap);

    return cnt;
} // strfmt_append

// Convert mstimap array to string of type "1,3,4-16,25-48,49,51"
char *mstp_mstimap2str(const mstp_msti_config_t *conf, u8 msti, char *buf, size_t bufsize)
{
    vtss_vid_t vid;
    vtss_vid_t vid_start       = 0;
    vtss_vid_t vid_end         = 0;
    BOOL       vid_start_found = 0;
    BOOL       first_range = 1;

    buf[0] = '\0';

    for (vid = 0; vid < ARRSZ(conf->map.map); vid++) {
        if (conf->map.map[vid] == msti) {
            // Vid present in mask
            if (!vid_start_found) {
                // New range
                vid_start = vid;
                vid_start_found = 1;
            } else if (vid != vid_end+1) {
                // End of range

                if (!first_range) {
                    (void) strfmt_append(buf, bufsize, ",");
                }
                if (vid_start == vid_end) {
                    // Only one vid in range
                    (void) strfmt_append(buf, bufsize, "%d", vid_start);
                } else {
                    // Two or more vids in range
                    (void) strfmt_append(buf, bufsize, "%d-%d", vid_start, vid_end);
                }
                vid_start = vid;
                first_range = 0;
            }
            vid_end = vid;
        }
    }
    /* Finish off */
    if (vid_start_found) {
        if (!first_range) {
            (void) strfmt_append(buf, bufsize, ",");
        }
        if (vid_start == vid_end) {
            // Only one vid in range
            (void) strfmt_append(buf, bufsize, "%d", vid_start);
        } else {
            // Two or more vids in range
            (void) strfmt_append(buf, bufsize, "%d-%d", vid_start, vid_end);
        }
    }

    return buf;
}

/****************************************************************************
 * Callbacks
 ****************************************************************************/

/*
 * Port state change indication
 */
static void 
mstp_port_state_change_callback(vtss_isid_t isid, 
                                vtss_port_no_t port_no, 
                                port_info_t *info)
{
    MSTP_LOCK();
    if(MSTP_READY() && !info->stack) {
        l2_port_no_t 
            l2port = L2PORT2PORT(isid, port_no),
            l2aggr = mstp_global.aggr.parent[l2port];
        T_IG(_C, "port_no: [%d,%u] = %u - link %s", isid, port_no, l2port, info->link ? "up" : "down");
        if(l2aggr != L2_NULL) {
            /* Update membership/speed in bulk */
            T_IG(_C, "aggr reconfig port %d => l2aggr %d (%s)", l2port, l2aggr, l2port2str(l2aggr));
            MSTP_AGGR_SET_CHANGE(l2aggr);
            cyg_flag_setbits(&mstp_global.control_flags, CTLFLAG_MSTP_AGGRCHANGE);
        } else {
            /* ANIELSEN: Why only notify non-mstp ports on link up? */
            if(mstp_global.conf.stp_enable[l2port] || info->link)
                port_sync(l2port, info); /* The STP ports + nonstp coming up */
        }
    } else {
        T_DG(_C, "LOST portstate callback: [%d,%u] link %s", isid, port_no, info->link ? "up" : "down");
    }
    MSTP_UNLOCK();
}

/*
 * Local port packet receive indication - forward through L2 interface
 */
static BOOL 
RX_mstp(void *contxt, 
        const uchar *const frm, 
        const vtss_packet_rx_info_t *const rx_info)
{
    // For us. Send back through L2 stack-wide interface.
    T_RG(_C, "port_no: %d len %d vid %d tagt %d glag %u", rx_info->port_no, rx_info->length, rx_info->tag.vid, rx_info->tag_type, rx_info->glag_no);
    // NB: Core MSTP doesn't like to receive on aggregations, so null out the GLAG (port is 1st in aggr)
    if (rx_info->tag_type == VTSS_TAG_TYPE_UNTAGGED) {
        l2_receive_indication(VTSS_MODULE_ID_MSTP, frm, rx_info->length, rx_info->port_no,
                              rx_info->tag.vid,
                              VTSS_GLAG_NO_NONE); /* Zap GLAG! */
    }
    // Allow other subscribers to receive the packet
    return FALSE;
}

/*
 * L2 Packet receive indication
 */
static void mstp_stack_receive(const void *packet, 
                               size_t len, 
                               vtss_vid_t vid,
                               l2_port_no_t l2port)
{
    T_NG(_I, "RX port %d len %zd", l2port, len);
    MSTP_LOCK();
    if (MSTP_READY()) {
        /* Physical RX port is aggregated? */
        l2_port_no_t l2paren = mstp_global.aggr.parent[l2port];
        BOOL enable = (l2paren != L2_NULL ? mstp_global.conf.stp_enable[MSTP_PORT_CONFIG_AGGR] : 
                       mstp_global.conf.stp_enable[l2port]);
        if (l2paren != L2_NULL) {
            T_NG(_I, "Map RX port %d -> %d, len %zd", l2port, l2paren, len);
            if (!enable) {
                T_IG(_I, "Receiving BPDU on aggregated port %d - %s, STP on aggrs disabled",
                     l2port, l2port2str(l2port));
            }
            l2port = l2paren;
        } else {
            if (!enable) {
                T_IG(_I, "Receiving BPDU on port %d - %s, but STP is disabled",
                     l2port, l2port2str(l2port));
            }
        }
        /* Consume through MSTP core */
        if (enable) {
            vtss_mstp_rx(mstp_global.mstpi, L2PORT2API(l2port), packet, len);
        }
    }
    MSTP_UNLOCK();
}

/****************************************************************************
 * VLAN
 ****************************************************************************/

/*
 * Signal a VLAN is having configuration changes.
 */
static void mstp_vlan_changed(void)
{
    if(MSTP_READY()) {
        MSTP_LOCK();
        cyg_flag_setbits(&mstp_global.control_flags, CTLFLAG_MSTP_VIDCHANGE);
        MSTP_UNLOCK();
    }
}

/****************************************************************************
 * Aggregation Interfacing
 ****************************************************************************/

/*
 * Signal Aggregation as Changed
 */
void
mstp_aggr_reconfigured(vtss_isid_t isid, uint aggr_no)
{
#if L2_MAX_LLAGS > 0
    l2_port_no_t l2aggr = AGGR_MGMT_GROUP_IS_GLAG(aggr_no) ? 
        L2GLAG2PORT((aggr_no - AGGR_MGMT_GLAG_START)) : 
        L2LLAG2PORT(isid, aggr_no - AGGR_MGMT_GROUP_NO_START);
#else
    l2_port_no_t l2aggr = L2GLAG2PORT((aggr_no - AGGR_MGMT_GLAG_START));
#endif
    if(MSTP_READY()) {
        MSTP_LOCK();
        T_DG(_C, "aggr reconfig isid %d aggr %d => l2aggr %d (%s)", isid, aggr_no, l2aggr, l2port2str(l2aggr));
        MSTP_AGGR_SET_CHANGE(l2aggr);
        cyg_flag_setbits(&mstp_global.control_flags, CTLFLAG_MSTP_AGGRCHANGE);
        MSTP_UNLOCK();
    } else {
        T_WG(_C, "LOST aggr reconfig isid %d aggr %d => l2aggr %d (%s)", isid, aggr_no, l2aggr, l2port2str(l2aggr));
    }
}

/*
 * Activate/Stop aggregation
 */
static void
mstp_aggr_sync(mstp_aggr_obj_t *pa,
               l2_port_no_t l2aggr)
{
    BOOL portadded = vtss_mstp_port_added(mstp_global.mstpi, L2PORT2API(l2aggr));
    if(mstp_global.conf.stp_enable[MSTP_PORT_CONFIG_AGGR]) { /* Run RSTP on aggr's? */
        if(pa->handler->members(pa)) {
            mstp_port_param_t *pconf = &mstp_global.conf.portconfig[MSTP_PORT_CONFIG_AGGR];
            uint linkspeed = aggrspeed(pa);
            int msti;
            T_IG(_C, "Add l2aggr %d, %u members, speed %u", 
                 l2aggr, pa->handler->members(pa), linkspeed);
            if(!portadded) {
                if(!vtss_mstp_add_port(mstp_global.mstpi, L2PORT2API(l2aggr)))
                    T_EG(_C, "Error adding RSTP aggregation: %d - %s", l2aggr, l2port2str(l2aggr));
            }
            if(!vtss_mstp_set_port_parameters(mstp_global.mstpi, L2PORT2API(l2aggr), pconf))
                T_EG(_C, "Error configuring RSTP aggregation: %d - %s", l2aggr, l2port2str(l2aggr));
            /* Apply MSTI config */
            for(msti = 0; msti < N_MSTI_MAX; msti++) {
                mstp_msti_port_param_t *mpp = &mstp_global.conf.msticonfig[MSTP_PORT_CONFIG_AGGR][msti];
                (void) vtss_mstp_set_msti_port_parameters(mstp_global.mstpi, msti, L2PORT2API(l2aggr), mpp);
            }
            if(!vtss_mstp_port_enable(mstp_global.mstpi, L2PORT2API(l2aggr), TRUE, linkspeed, TRUE))
                T_EG(_C, "Error enabling RSTP aggregation %d - %s at speed %d", l2aggr, l2port2str(l2aggr), linkspeed);
            /*
             * Enable *port* forwarding, but block all MSTI's.
             */
            mstp_aggr_sync_ports(l2aggr, pa);
        } else {
            T_IG(_C, "Delete l2aggr %d - %s", l2aggr, l2port2str(l2aggr));
            if(portadded)
                (void) vtss_mstp_delete_port(mstp_global.mstpi, L2PORT2API(l2aggr));
        }
    } else {
        /* Delete the MSTP port (if we had one) */
        if(portadded)
            (void) vtss_mstp_delete_port(mstp_global.mstpi, L2PORT2API(l2aggr));
        /* Enable current members */
        l2_port_no_t l2port;
        for(l2port = pa->handler->first_port(pa); l2port != L2_NULL; 
            l2port = pa->handler->next_port(pa, l2port))
            mstp_set_all_stpstate(l2port, MSTP_FWDSTATE_FORWARDING);
    }
}

/*
 * Reconfigure Aggregations - we are *locked* here!
 */
static void
mstp_aggr_reconfigure(BOOL all)
{
    l2_port_no_t l2aggr;
    T_DG(_C, "Check Aggregated Poags - Start");
    MSTP_ASSERT_LOCKED();
    vtss_mstp_stm_lock(mstp_global.mstpi);
    for(l2aggr = L2_MAX_PORTS; l2aggr < L2_MAX_POAGS; l2aggr++) {
        if(all || MSTP_AGGR_GETSET_CHANGE(l2aggr, 0)) {
            mstp_aggr_obj_t aob, *paob;
            paob = mstp_get_aggr(&aob, l2aggr);
            if (paob) {
                T_DG(_C, "Check Port %d - %s, initially %d members", l2aggr, l2port2str(l2aggr), 
                     paob->handler->members(paob));
                paob->handler->update(paob);
                T_DG(_C, "Check Port %d - %s, now %d members", l2aggr, l2port2str(l2aggr), 
                     paob->handler->members(paob));
                mstp_aggr_sync(paob, l2aggr);
            }
        }
    }
    vtss_mstp_stm_unlock(mstp_global.mstpi);
    T_DG(_C, "Check Aggregated Poags - Done");
}

static void
mstp_call_trap_sink(void)
{
    u32 traps;
    mstp_trap_sink_t cb;
    cb = mstp_global.trap_cb;
    traps = mstp_global.traps;
    mstp_global.traps = 0;      /* Reset traps */
    if(traps && cb)
        cb(traps);
}

/****************************************************************************
 * Module thread
 ****************************************************************************/

/**
 * mstp_master_initialize - initialize MSTP state when starting as
 * stack master.
 * 
 * Function called by main mstp thread - locked - exit locked.
 *
 */
static void
mstp_master_initialize(void)
{
    mstp_macaddr_t sysmac;

    MSTP_ASSERT_LOCKED();

    /* Get System MAC address */
    (void)conf_mgmt_mac_addr_get(sysmac.mac, 0);

    /* Initialize MSTP */
    mstp_global.mstpi = vtss_mstp_create_bridge(&sysmac, MSTP_BRIDGE_PORTS);

    /* Propagate system config */
    mstp_conf_propagate(TRUE, TRUE);

    /* Sync port states */
    memset(&mstp_global.aggr, 0, sizeof(mstp_global.aggr));
    {
        uint i;
        for (i = 0; i < ARRSZ(mstp_global.aggr.parent); i++)
            mstp_global.aggr.parent[i] = L2_NULL;
    }
    mstp_global.ready = TRUE; /* Ready to rock - allow portstate callbacks */
}

/**
 * mstp_master_process - process MSTP main tasks while stack master.
 * 
 * Function called by main mstp thread - unlocked.
 *
 * Terminates when becoming slave - unlocked.
 */
static void
mstp_master_process(void)
{
    while(msg_switch_is_master()) {
        T_RG(_C, "tick()");
        MSTP_LOCK();    /* Lock while ticking */
        vtss_mstp_tick(mstp_global.mstpi);
        mstp_call_trap_sink();
        MSTP_UNLOCK();  /* MSTP API available again */
        cyg_tick_count_t wakeup = cyg_current_time() + (1000/ECOS_MSECS_PER_HWTICK);
        cyg_flag_value_t flags;
        while((flags = cyg_flag_timed_wait(&mstp_global.control_flags, 0xffff, 
                                           CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR, wakeup))) {
            T_IG(_C, "MSTP thread event, flags 0x%x", flags);
            MSTP_LOCK(); /* Process flags while locked */
            if(flags & CTLFLAG_MSTP_VIDCHANGE)
                (void) mstp_apply_msticonfig();                        
            if(flags & CTLFLAG_MSTP_AGGRCHANGE)
                mstp_aggr_reconfigure(FALSE); /* One or more AGGR's changed */
            if(flags & CTLFLAG_MSTP_AGGRCONFIG)
                mstp_aggr_reconfigure(TRUE); /* All AGGR's changed */
            if(flags & CTLFLAG_MSTP_DEFCONFIG) {
                mstp_conf_read(TRUE); /* Reset stack configuration */
                /* Make RSTP configuration effective in RSTP core */
                mstp_conf_propagate(TRUE, TRUE);
                /* Synchronize port states */
                sync_ports_all();
                T_D("Posting load defaults semaphore");
                cyg_semaphore_post(&mstp_global.defconfig_sema);
            }
            if(flags & CTLFLAG_MSTP_SWITCH_SYNC) {
                u32 mask = mstp_global.switch_sync;
                int i;
                mstp_global.switch_sync = 0; /* reset */
                for(i = VTSS_ISID_START; i < VTSS_ISID_END; i++) {
                    if(mask & (1 << i))
                        sync_ports_switch(i);
                }
            }
            MSTP_UNLOCK(); /* Unlock to go back to sleep */
            /* Callbacks while *NOT* locked */
            if(flags & CTLFLAG_MSTP_CONFIG_CHANGE) {
                mstp_config_change_cb_t cb;
                cb = mstp_global.config_cb;
                if (cb) {
                    cb();
                }
            }
        }
    }
}

/*lint -sem(mstp_thread, thread_protected) */
static void mstp_thread(cyg_addrword_t data)
{
    packet_rx_filter_t rx_filter;
    void *filter_id = NULL;

    /* Note - locked here! */
    /*lint --e{455,456} ... Lock/unlock is suddle, but *carefully* designed */
    MSTP_ASSERT_LOCKED();

    /* MSTP frames registration */
    memset(&rx_filter, 0, sizeof(rx_filter));
    rx_filter.modid = VTSS_MODULE_ID_MSTP;
    rx_filter.match = PACKET_RX_FILTER_MATCH_DMAC;
    memcpy(rx_filter.dmac, ieee_bridge, sizeof(rx_filter.dmac));
    rx_filter.cb    = RX_mstp;
    rx_filter.prio  = PACKET_RX_FILTER_PRIO_NORMAL;
    vtss_rc rc = packet_rx_filter_register(&rx_filter, &filter_id);
    VTSS_ASSERT(rc == VTSS_OK);
    l2_receive_register(VTSS_MODULE_ID_MSTP, mstp_stack_receive);

    /* Port change callback */
    (void) port_global_change_register(VTSS_MODULE_ID_MSTP, mstp_port_state_change_callback);

    /* VLAN config change register */
    vlan_membership_bulk_change_register(VTSS_MODULE_ID_MSTP, mstp_vlan_changed);

    /* AGGR config change callback */
    aggr_change_register(mstp_aggr_reconfigured);

    for(;;) {

        MSTP_ASSERT_LOCKED();   /* Locked at entry - and each time looping */

        if (msg_switch_is_master()) {

            mstp_master_initialize();

            MSTP_UNLOCK(); /* We were locked initializing - but open here */

            mstp_master_process(); /* Process while being master */

            MSTP_LOCK(); /* Lock outer airlock when becoming slave */
        }

        /* Note - still locked! */
        MSTP_ASSERT_LOCKED();

        mstp_global.ready = FALSE; /* Done rocking */

        if(mstp_global.mstpi) {
            /* De-Initialize MSTP core */
            vtss_mstp_delete_bridge(mstp_global.mstpi);
            mstp_global.mstpi = NULL;
        }

        MSTP_UNLOCK();

        T_IG(_C, "Suspending MSTP thread (became slave)");
        cyg_thread_suspend(mstp_thread_handle);
        T_IG(_C, "Restarting MSTP thread (became master)");

        MSTP_LOCK(); /* Lock outer airlock when waking up again */
    }
}

/****************************************************************************/
/*  MSTP callout functions                                                  */
/****************************************************************************/

/**
 * BPPDU transmit.
 *
 * \param portnum The physical port on which to send the BPDU.
 *
 * \param buffer The BPDU to transmit.
 *
 * \param size The length of the BPDU buffer.
 */
void 
vtss_mstp_tx(uint portnum,
             void *buffer, 
             size_t size)
{
    vtss_common_bufref_t bufref;
    uchar *osbuf;
    vtss_isid_t isid;
    vtss_port_no_t switchport;

    /* Convert to base-zero */
    portnum = API2L2PORT(portnum);

    T_NG(_I, "Port %d - %s, tx %zd bytes", portnum, l2port2str(portnum), size);

    if(!l2port_is_port(portnum)) { /* Map aggregation to first port number */
        mstp_aggr_obj_t aob, *paob;
        paob = mstp_get_aggr(&aob, portnum);
        if (paob) {
            portnum = paob->handler->first_port(paob);
            if(portnum == L2_NULL)
                return;             /* No ports contained atm? */
        }
    }

    VTSS_ASSERT(l2port2port(portnum, &isid, &switchport));

    osbuf = vtss_os_alloc_xmit(portnum, size, &bufref);
    if(osbuf) {
        uchar *basemac = mstp_global.sysmac[isid-VTSS_ISID_START];
        memcpy(osbuf, buffer, size);
        misc_instantiate_mac(osbuf+6, basemac, switchport+1-VTSS_PORT_NO_START); /* entry 0 is the CPU port */
        (void) vtss_os_xmit(portnum, osbuf, size, bufref);
    }
}

/**
 * Switch interface access - set forwarding state.
 * \param portnum The physical port to control
 * \param state The state to set
 */
void
vtss_mstp_port_setstate(uint portnum,
                        u8 msti,
                        mstp_fwdstate_t state)
{
    /* Convert to base-zero */
    portnum = API2L2PORT(portnum);

    T_IG(_I, "Port %d[%d] - %s, FwdState %s", portnum, msti, l2port2str(portnum), fwd2str(state));
    if(l2port_is_port(portnum))
        l2_set_msti_stpstate(msti, portnum, (vtss_stp_state_t)state);
    else {
        l2_port_no_t l2port;
        mstp_aggr_obj_t aob, *paob;
        l2_set_msti_stpstate(msti, portnum, (vtss_stp_state_t)state); /* Keep aggr state */
        paob = mstp_get_aggr(&aob, portnum);
        if (paob) {
            for(l2port = paob->handler->first_port(paob); l2port != L2_NULL; 
                l2port = paob->handler->next_port(paob, l2port))
                l2_set_msti_stpstate(msti, l2port, (vtss_stp_state_t)state);
        }
    }
}

/**
 * Switch interface access - \e MAC \ table..
 * \param portnum The physical port to flush
 *
 * \note In MSTI operation, the current implementation will flush more
 * than strictly necessary (all VLANS are flushed).
 */
void
vtss_mstp_port_flush(uint portnum,
                     u8 msti)
{
    /* Convert to base-zero */
    portnum = API2L2PORT(portnum);

    T_IG(_I, "Flush Port %d[%d] - %s", portnum, msti, l2port2str(portnum));
    l2_flush_port(portnum);
}

/**
 * VLAN interface access - determine port MSTI membership
 *
 * \param portnum The physical port to query
 *
 * \param msti The MSTI instance to query for membership
 *
 * \return TRUE if the port is a member of the MSTI.
 */
bool
vtss_mstp_port_member(uint portnum,
                      u8 msti)
{
    BOOL member = FALSE;
    mstp_mstate_t *state = &mstp_global.mstate[msti];

    /* Convert to base-zero */
    portnum = API2L2PORT(portnum);

    VTSS_ASSERT(msti > 0 && msti < N_MSTI_MAX);

    if(state->n_members) {
        if(!l2port_is_port(portnum)) { /* Map aggregation to first port number */
            mstp_aggr_obj_t aob, *paob;
            paob = mstp_get_aggr(&aob, portnum);
            if (paob) {
                portnum = paob->handler->first_port(paob);
            }
        }
        
        if(portnum != L2_NULL) /* Ports contained atm? */
            member = VTSS_BF_GET(state->members, portnum);
    }

    T_NG(_I, "Port %d - %s msti %d - %s member", portnum, l2port2str(portnum), 
         msti, member ? "IS" : "not");

    return member;
}

/**
 * Time interface - get current time
 *
 * \return the current time of day in seconds (relative to an
 * arbitrary absolute time)
 */
u32
vtss_mstp_current_time(void)
{
    return time(NULL);
}

/**
 * Callout from MSTP - Trap event occurred. We're delivering this to
 * trap sink (if any) in a batched fashion. (After tick).
 *
 * Note: We're locked already.
 */
void
vtss_mstp_trap(u8 msti,
               mstp_trap_event_t event)
{
    mstp_global.traps |= (1 << (uint) event);
}

/**
 * Callout from MSTP - Allocate memory.
 */
void *
vtss_mstp_malloc(size_t sz)
{
    return VTSS_MALLOC(sz);
}

/**
 * Callout from MSTP - Free memory.
 */
void
vtss_mstp_free(void *ptr)
{
    VTSS_FREE(ptr);
}

/****************************************************************************/
/*  API functions                                                           */
/****************************************************************************/

BOOL
mstp_get_system_config(mstp_bridge_param_t *pconf)
{
    MSTP_LOCK();
    *pconf = mstp_global.conf.sys;
    MSTP_UNLOCK();
    return TRUE;
}

BOOL
mstp_set_system_config(const mstp_bridge_param_t *pconf)
{
    if((pconf->bridgeMaxAge < 6 || pconf->bridgeMaxAge > 40) ||
       (pconf->bridgeForwardDelay < 4 || pconf->bridgeForwardDelay > 30) ||
       (pconf->bridgeMaxAge > ((pconf->bridgeForwardDelay-1)*2))) {
        T_E("Attempt to set illegal system timers: MaxAge %u, FwdDelay %u",
            pconf->bridgeMaxAge, pconf->bridgeForwardDelay);
        return FALSE;
    }

    BOOL rc = TRUE;
    MSTP_LOCK();
    if(memcmp(&mstp_global.conf.sys, pconf, sizeof(*pconf)) != 0) {

        mstp_global.conf.sys = *pconf;
        save_config();

        /* Propagate system config */
        rc = vtss_mstp_set_bridge_parameters(mstp_global.mstpi, pconf);

        BOOL single_mode = (mstp_global.conf.sys.forceVersion < MSTP_PROTOCOL_VERSION_MSTP);
        
        rc = rc && (VTSS_OK == l2_set_msti_map(single_mode,
                                               ARRSZ(mstp_global.conf.msti.map.map), 
                                               mstp_global.conf.msti.map.map));
    }
    MSTP_UNLOCK();

    return rc;
}

u8
mstp_get_msti_priority(u8 msti)
{
    MSTP_LOCK();
    u8 priority = mstp_global.conf.bridgePriority[msti];
    MSTP_UNLOCK();
    return priority;
}

BOOL
mstp_set_msti_priority(u8 msti, 
                       u8 priority)
{
    BOOL rc = TRUE;

    MSTP_LOCK();
    if(mstp_global.conf.bridgePriority[msti] != priority) {

        mstp_global.conf.bridgePriority[msti] = priority;
        save_config();
        
        /* The call will fail if the MSTI is not active */
        (void) vtss_mstp_set_bridge_priority(mstp_global.mstpi, msti, priority);
    }
    MSTP_UNLOCK();
    return rc;
}


BOOL
mstp_get_msti_config(mstp_msti_config_t *conf, u8 cfg_digest[MSTP_DIGEST_LEN])
{
    MSTP_LOCK();
    vtss_mstp_get_config_id(mstp_global.mstpi, NULL, NULL, cfg_digest);
    *conf = mstp_global.conf.msti;
    MSTP_UNLOCK();
    return TRUE;
}

BOOL
mstp_set_msti_config(mstp_msti_config_t *conf)
{
    BOOL rc = TRUE;
    MSTP_LOCK();
    if(memcmp(&mstp_global.conf.msti, conf, sizeof(*conf)) != 0) {
        mstp_global.conf.msti = *conf;
        save_config();

        rc = mstp_apply_msticonfig();

    }
    MSTP_UNLOCK();
    return rc;
}

BOOL
mstp_get_port_config(vtss_isid_t isid,
                     vtss_port_no_t port_no,
                     BOOL *enable,
                     mstp_port_param_t *pconf)
{
    l2_port_no_t l2port = (port_no == VTSS_PORT_NO_NONE ? 
                           MSTP_PORT_CONFIG_AGGR : 
                           L2PORT2PORT(isid, port_no)); /* Aggr or normal */

    if(l2port > MSTP_PORT_CONFIG_AGGR)
        return FALSE;

    MSTP_LOCK();
    *pconf = mstp_global.conf.portconfig[l2port];
    *enable = mstp_global.conf.stp_enable[l2port];
    MSTP_UNLOCK();
    return TRUE;
}

BOOL
mstp_set_port_config(vtss_isid_t isid,
                     vtss_port_no_t port_no,
                     BOOL enable,
                     const mstp_port_param_t *pconf)
{
    l2_port_no_t l2port = (port_no == VTSS_PORT_NO_NONE ? 
                           MSTP_PORT_CONFIG_AGGR : 
                           L2PORT2PORT(isid, port_no)); /* Aggr or normal */

    if(l2port > MSTP_PORT_CONFIG_AGGR)
        return FALSE;

#ifdef VTSS_SW_OPTION_DOT1X
    // Inter-protocol check.
    // MSTP cannot get enabled on ports that are not in 802.1X Authorized state.
    // Note that port_no == 0 is acceptable in this func, hence the extra check.
    if(VTSS_ISID_LEGAL(isid) && port_no != VTSS_PORT_NO_NONE) {
        dot1x_switch_cfg_t dot1x_switch_cfg;
        if(dot1x_mgmt_switch_cfg_get(isid, &dot1x_switch_cfg) != VTSS_OK) {
            return FALSE;  
        }        
        if(enable && dot1x_switch_cfg.port_cfg[port_no - VTSS_PORT_NO_START].admin_state != NAS_PORT_CONTROL_FORCE_AUTHORIZED) {
            return FALSE;
        }
    }
#endif /* VTSS_SW_OPTION_DOT1X */    

    MSTP_LOCK();
    BOOL enb_chg = (mstp_global.conf.stp_enable[l2port] != enable);
    if(memcmp(&mstp_global.conf.portconfig[l2port], pconf, sizeof(*pconf)) != 0 ||
       enb_chg) {

        mstp_global.conf.portconfig[l2port] = *pconf;
        mstp_global.conf.stp_enable[l2port] = enable;
        save_config();

        if(l2port != MSTP_PORT_CONFIG_AGGR) {            /* Plain port */
            if(enb_chg)
                port_sync(l2port, NULL); /* Stop/start port */
            /* Apply in core MSTP as well */
            (void) vtss_mstp_set_port_parameters(mstp_global.mstpi, L2PORT2API(l2port), pconf);
        } else {           /* Potential all aggrs - process in bulk */
            cyg_flag_setbits(&mstp_global.control_flags, CTLFLAG_MSTP_AGGRCONFIG);
        }
    }
    T_I("Port %d -> cport %d, enb %d", port_no, l2port, enable);
    MSTP_UNLOCK();
    return TRUE;
}

BOOL
mstp_get_msti_port_config(vtss_isid_t isid,
                          u8 msti, 
                          vtss_port_no_t port_no,
                          mstp_msti_port_param_t *pconf)
{
    l2_port_no_t l2port = (port_no == VTSS_PORT_NO_NONE ? 
                           MSTP_PORT_CONFIG_AGGR : 
                           L2PORT2PORT(isid, port_no)); /* Aggr or normal */

    if(l2port > MSTP_PORT_CONFIG_AGGR)
        return FALSE;

    MSTP_LOCK();
    *pconf = mstp_global.conf.msticonfig[l2port][msti];
    MSTP_UNLOCK();
    return TRUE;
}

BOOL
mstp_set_msti_port_config(vtss_isid_t isid,
                          u8 msti, 
                          vtss_port_no_t port_no,
                          const mstp_msti_port_param_t *pconf)
{
    l2_port_no_t l2port = (port_no == VTSS_PORT_NO_NONE ? 
                           MSTP_PORT_CONFIG_AGGR : 
                           L2PORT2PORT(isid, port_no)); /* Aggr or normal */

    if(l2port > MSTP_PORT_CONFIG_AGGR)
        return FALSE;

    MSTP_LOCK();
    if(memcmp(&mstp_global.conf.msticonfig[l2port][msti], pconf, sizeof(*pconf)) != 0) {

        mstp_global.conf.msticonfig[l2port][msti] = *pconf;
        save_config();
        
        if(l2port != MSTP_PORT_CONFIG_AGGR) {            /* Plain port */
            /* Apply in core MSTP as well */
            (void) vtss_mstp_set_msti_port_parameters(mstp_global.mstpi, msti, L2PORT2API(l2port), pconf);
        } else {           /* Potential all aggrs - process in bulk */
            cyg_flag_setbits(&mstp_global.control_flags, CTLFLAG_MSTP_AGGRCONFIG);
        }
    }
    T_I("MSTI %d Port %d -> cport %d, prio %d", msti, port_no, l2port, pconf->adminPortPriority);
    MSTP_UNLOCK();
    return TRUE;
}

BOOL
mstp_get_bridge_status(u8 msti, 
                       mstp_bridge_status_t *status)
{
    MSTP_LOCK();
    BOOL ok = FALSE;
    if(MSTP_READY()) {
        ok = vtss_mstp_get_bridge_status(mstp_global.mstpi, msti, status);
        if(ok)
            status->rootPort = status->rootPort ? API2L2PORT(status->rootPort) : L2_NULL;
    }
    MSTP_UNLOCK();
    return ok;
}

BOOL
mstp_get_port_status(u8 msti,
                     l2_port_no_t l2port,
                     mstp_port_mgmt_status_t *status)
{
    MSTP_LOCK();
    BOOL ok = (MSTP_READY() && l2port_is_valid(l2port));
    if(ok) {
        memset(status, 0, sizeof(*status));
        if(l2port_is_port(l2port)) {
            status->enabled = mstp_global.conf.stp_enable[l2port];
            status->parent = mstp_global.aggr.parent[l2port];
        } else {
            status->enabled = mstp_global.conf.stp_enable[MSTP_PORT_CONFIG_AGGR]; /* Shared enabled-ness */
            status->parent = L2_NULL; /* Always top dog */
        }
        status->fwdstate = fwd2str((mstp_fwdstate_t)l2_get_msti_stpstate(msti, l2port));
        uint stpport = (status->parent != L2_NULL ? status->parent : l2port);
        status->active = vtss_mstp_get_port_status(mstp_global.mstpi, msti, L2PORT2API(stpport), &status->core);
    }
    MSTP_UNLOCK();
    return ok;
}

BOOL
mstp_get_port_vectors(u8 msti,
                      l2_port_no_t l2port,
                      mstp_port_vectors_t *vectors)
{
    MSTP_LOCK();
    BOOL ok = (MSTP_READY() && l2port_is_valid(l2port));
    if(ok) {
        memset(vectors, 0, sizeof(*vectors));
        ok = vtss_mstp_get_port_vectors(mstp_global.mstpi, msti, L2PORT2API(l2port), vectors);
    }
    MSTP_UNLOCK();
    return ok;
}

BOOL
mstp_get_port_statistics(l2_port_no_t l2port,
                         mstp_port_statistics_t *stats,
                         BOOL clear)
{
    MSTP_LOCK();
    if(clear)
        (void) vtss_mstp_clear_port_statistics(mstp_global.mstpi, L2PORT2API(l2port));
    BOOL ok = vtss_mstp_get_port_statistics(mstp_global.mstpi, L2PORT2API(l2port), stats);
    MSTP_UNLOCK();
    return ok;
}

BOOL
mstp_set_port_mcheck(l2_port_no_t l2port)
{
    T_IG(_C, "mcheck: l2port %d", l2port);
    MSTP_LOCK();
    BOOL ok = vtss_mstp_port_mcheck(mstp_global.mstpi, L2PORT2API(l2port));
    MSTP_UNLOCK();
    return ok;
}

/* Trap support */

BOOL
mstp_register_trap_sink(mstp_trap_sink_t cb)
{
    BOOL rc = FALSE;
    MSTP_LOCK();
    if(cb == NULL ||
       mstp_global.trap_cb == NULL ||
       mstp_global.trap_cb == cb) {
        mstp_global.trap_cb = cb;
        rc = TRUE;
    }
    MSTP_UNLOCK();
    return rc;
}

BOOL
mstp_register_config_change_cb(mstp_config_change_cb_t cb)
{
    BOOL rc = FALSE;
    MSTP_LOCK();
    if(cb == NULL ||
       mstp_global.config_cb == NULL ||
       mstp_global.config_cb == cb) {
        mstp_global.config_cb = cb;
        rc = TRUE;
    }
    MSTP_UNLOCK();
    return rc;
}

/****************************************************************************/
/*  Initialization functions                                                */
/****************************************************************************/

vtss_rc
mstp_init(vtss_init_data_t *data)
{
    vtss_isid_t isid = data->isid;

    /*lint --e{454,456} ... We leave the Mutex locked */
    switch (data->cmd) {
    case INIT_CMD_INIT:
        /* Initialize and register trace ressources */
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);
        memset(&mstp_global, 0, sizeof(mstp_global));
        mstp_global.ready = FALSE;
        critd_init(&mstp_global.mutex, "mstp", VTSS_MODULE_ID_MSTP, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        cyg_semaphore_init(&mstp_global.defconfig_sema, 0);

#ifdef VTSS_SW_OPTION_VCLI
        mstp_cli_init();
#endif
#ifdef VTSS_SW_OPTION_ICFG
        if (mstp_icfg_init() != VTSS_OK) {
           T_D("Calling mstp_icfg_init() failed");
        }
#endif
        cyg_flag_init( &mstp_global.control_flags );
        cyg_thread_create(THREAD_DEFAULT_PRIO, 
                          mstp_thread, 
                          0, 
                          "MSTP", 
                          mstp_thread_stack, 
                          sizeof(mstp_thread_stack),
                          &mstp_thread_handle,
                          &mstp_thread_block);
        break;
    case INIT_CMD_CONF_DEF:
        if (isid == VTSS_ISID_GLOBAL) {
            /* Load defaults. This has to happen on the thread to avoid race conditions,
             * and we cannot exit INIT_CMD_CONF_DEF until the thread is done either --
             * more race conditions. Thus, a semaphore is used for synchronization.
             */
            T_D("Signal thread to begin loading defaults");
            cyg_flag_setbits(&mstp_global.control_flags, CTLFLAG_MSTP_DEFCONFIG);
            T_D("Waiting for thread to complete loading defaults");
            (void)cyg_semaphore_wait(&mstp_global.defconfig_sema);
            T_D("Load defaults done");
        }
        break;
    case INIT_CMD_MASTER_UP:
        T_IG(_C, "Starting MSTP thread (became master) - size global = %zu bytes", sizeof(mstp_global));
        mstp_conf_read(FALSE); /* Reset stack configuration */
        cyg_thread_resume(mstp_thread_handle);
        break;
    case INIT_CMD_SWITCH_ADD: {
        uchar *basemac = mstp_global.sysmac[isid-VTSS_ISID_START];
        T_IG(_C, "Switch add - ISID %u", isid);
        VTSS_ASSERT((isid-VTSS_ISID_START) < ARRSZ(mstp_global.sysmac));
#if VTSS_SWITCH_STACKABLE
        (void) topo_isid2mac(isid, basemac);
#else
        (void) conf_mgmt_mac_addr_get(basemac, 0);
#endif
        mstp_signal_switch_sync(isid);
        break;
    }
    case INIT_CMD_SWITCH_DEL:
        mstp_signal_switch_sync(isid);
        break;
    case INIT_CMD_MASTER_DOWN:
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
