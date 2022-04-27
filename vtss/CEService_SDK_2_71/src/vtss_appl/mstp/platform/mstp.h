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

#ifndef _MSTP_H_
#define _MSTP_H_

#include "aggr_api.h"

#define CTLFLAG_MSTP_VIDCHANGE			(1 << 0)
#define CTLFLAG_MSTP_AGGRCHANGE			(1 << 1)
#define CTLFLAG_MSTP_AGGRCONFIG			(1 << 2)
#define CTLFLAG_MSTP_DEFCONFIG			(1 << 3)
#define CTLFLAG_MSTP_SWITCH_SYNC		(1 << 4)
#define CTLFLAG_MSTP_CONFIG_CHANGE		(1 << 5)

#define MSTP_CONF_VERSION	2

#define MSTP_PHYS_PORTS		(L2_MAX_PORTS) /* Number of physical ports */
#define MSTP_AGGR_PORTS		(L2_MAX_LLAGS+L2_MAX_GLAGS) /* Number of aggregations */

/* physical ports + aggragations + entry 'zero' - ]0 ; MAX]*/
#define MSTP_BRIDGE_PORTS	(MSTP_PHYS_PORTS+MSTP_AGGR_PORTS)

#define MSTP_CONF_PORT_FIRST	0
#define MSTP_CONF_PORT_LAST     (MSTP_PHYS_PORTS-1)

#define TEMP_LOCK()	cyg_scheduler_lock()
#define TEMP_UNLOCK()	cyg_scheduler_unlock()

#define MSTP_AGGR_SET_CHANGE(l2) VTSS_BF_SET(mstp_global.aggr.change, \
                                             (uint)(l2-MSTP_PHYS_PORTS-VTSS_PORT_NO_START), 1)

#define MSTP_AGGR_GETSET_CHANGE(l2, v)                                  \
    ({                                                                  \
        BOOL rc; uint ix = l2-MSTP_PHYS_PORTS-VTSS_PORT_NO_START;       \
        TEMP_LOCK();                                                    \
        rc = VTSS_BF_GET(mstp_global.aggr.change, ix);                  \
        if(v != rc)                                                     \
            VTSS_BF_SET(mstp_global.aggr.change, ix, v);                \
        TEMP_UNLOCK();                                                  \
        rc;                                                             \
    })

#define MSTP_AGGR_SET_MEMBER(_pno, p, v) VTSS_BF_SET(p->members, _pno, v)
#define MSTP_AGGR_GET_MEMBER(_pno, p) 	 VTSS_BF_GET(p->members, _pno)

#define MSTP_READY()	(mstp_global.ready && mstp_global.mstpi != NULL)

#define LOCK_TRACE_LEVEL VTSS_TRACE_LVL_RACKET

#define MSTP_LOCK()          critd_enter(&mstp_global.mutex, VTSS_TRACE_GRP_DEFAULT, LOCK_TRACE_LEVEL, __FILE__, __LINE__)
#define MSTP_UNLOCK()        critd_exit (&mstp_global.mutex, VTSS_TRACE_GRP_DEFAULT, LOCK_TRACE_LEVEL, __FILE__, __LINE__)
#define MSTP_ASSERT_LOCKED() critd_assert_locked(&mstp_global.mutex, VTSS_TRACE_GRP_DEFAULT, __FILE__, __LINE__)

typedef struct {
    u16 n_members,                               /* Count of ports */
        port_min,                                /* Start */
        port_max;                                /* Stop */
} aggr_participants_t;

/* LLAG state */
typedef struct {
    aggr_participants_t	cmn;    /* Common parts */
    uchar members[VTSS_BF_SIZE(VTSS_PORTS)]; /* Member ports bitmask  - switch local */
} llag_participants_t;

/* GLAG state */
typedef struct {
    aggr_participants_t	cmn;                   /* Common parts */
    uchar members[VTSS_BF_SIZE(L2_MAX_PORTS)]; /* Member ports bitmask - stack global */
} glag_participants_t;

struct mstp_aggr_obj;

typedef struct mstp_aggr_objh {
    uint         (*members)(struct mstp_aggr_obj const *);
    l2_port_no_t (*first_port)(struct mstp_aggr_obj const *);
    l2_port_no_t (*next_port)(struct mstp_aggr_obj const *, l2_port_no_t);
    void         (*update)(struct mstp_aggr_obj*);
    void         (*remove_port)(struct mstp_aggr_obj const *, l2_port_no_t);
} mstp_aggr_objh_t;

typedef struct mstp_aggr_obj {
    void *data_handle;
    mstp_aggr_objh_t const *handler;
    l2_port_no_t l2port;
    union {
        struct {
            vtss_glag_no_t glag;
        } glag;
        struct {
            vtss_isid_t isid;
            vtss_poag_no_t aggr_no;
            u16 port_offset;    /* L2 port offset */
        } llag;
    } u;
} mstp_aggr_obj_t;

#define MSTP_PORT_CONFIG_COUNT (1+MSTP_PHYS_PORTS)        /* [n-1] for aggrs */
#define MSTP_PORT_CONFIG_AGGR  (MSTP_PORT_CONFIG_COUNT-1) /* aggrs index */

/* Configuration */
typedef struct {
    mstp_bridge_param_t    sys;
    u8                     bridgePriority[N_MSTI_MAX];
    mstp_msti_config_t     msti;
    BOOL                   stp_enable[MSTP_PORT_CONFIG_COUNT];
    mstp_port_param_t      portconfig[MSTP_PORT_CONFIG_COUNT];
    mstp_msti_port_param_t msticonfig[MSTP_PORT_CONFIG_COUNT][N_MSTI_MAX];
} mstp_conf_t;

typedef struct {
    unsigned long version;      /* Block version */
    mstp_conf_t   conf;         /* Configuration */
} mstp_conf_blk_t;

#define API2L2PORT(p) (p - 1)
#define L2PORT2API(p) (p + 1)

#endif /* _MSTP_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
