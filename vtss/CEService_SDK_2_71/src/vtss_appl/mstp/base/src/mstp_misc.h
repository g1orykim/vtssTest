/*

 Vitesse Switch API software.

 Copyright (c) 2002-2010 Vitesse Semiconductor Corporation "Vitesse". All
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

#ifndef _VTSS_MSTP_MISC_H_
#define _VTSS_MSTP_MISC_H_

#include "vtss_md5_api.h"

/**
 * \file mstp_misc.h
 * \brief Management plane helper functions
 *
 * This file contain various helper function used by management and
 * control planes.
 *
 * \author Lars Povlsen <lpovlsen@vitesse.com>
 *
 * \date 13-01-2009
 */

static inline bool
valid_port(uint max, 
           uint portnum)
{
    return (portnum > 0 && portnum <= max);
}

static inline mstp_port_t *
get_port(const mstp_bridge_t *mstp, 
         uint portnum)
{
    return mstp->ist.portmap[portnum-1];
}

static inline mstp_port_t *
get_tport(const mstp_tree_t *tree, 
          uint portnum)
{
    return tree->portmap[portnum-1];
}

static inline mstp_tree_t *
mstp_get_instance(mstp_bridge_t *mstp,
                  u8 msti)
{
    return mstp->trees[msti];
}

static inline mstp_tree_t *
mstp_get_tree_by_id(mstp_bridge_t *mstp,
                    u16 MSTID)
{
    uint i;
    for(i = 1; i < N_MSTI_MAX; i++) {
        mstp_tree_t *tree = mstp->trees[i];
        if(tree && tree->MSTID == MSTID)
            return tree;
    }
    return NULL;
}

static inline u32
calc_pathcost(u32 adminPathCost, u32 linkspeed)
{
    u32 pc;
    if(adminPathCost != 0) {
        pc = adminPathCost;
    } else {
        if(linkspeed) {
            if((pc = ((u32) 20000000)/linkspeed) == 0)
                pc = 2;
        } else
            pc = (u32) 200000000;
    }
    return pc;
}

/** 
 * Update derived path cost based upon current port settings. In
 * accordance with 17.13 and 14.8.2.3.4 the reselect parameter for the
 * Port (17.19.34) is set TRUE, and the selected parameter for the
 * Port (17.19.36) is set FALSE.
 *
 * \param mstp The MSTP instance data.
 *
 * \param portnum The Port number.
 *
 * \param cist to update path cost for
 *
 * \param linkspeed The link speed of the interface, in MB/s. The
 * speed will be converted into a path cost according to Table 17-3.
 */
void
mstp_update_linkspeed(mstp_bridge_t *mstp,
                      uint portnum,
                      mstp_cistport_t *cist, 
                      u32 linkspeed);

/** 
 * Update p2p state based upon current port settings. In accordance
 * with 17.13 and 14.8.2.3.4 the reselect parameter for the Port
 * (17.19.34) is set TRUE, and the selected parameter for the Port
 * (17.19.36) is set FALSE.
 *
 * \param port to update p2p state for
 *
 * \param fdx Full duplex operation of physical link
 * (operPointToPointMAC)
 */
void
mstp_update_point2point(mstp_port_t *port, 
                        mstp_cistport_t *cist, 
                        bool fdx);

void
mstp_calc_digest(mstp_map_t *map, 
                 u8 *digest);

/*
 * config data access 
 */

static inline 
mstp_port_param_t *
vtss_mstp_get_port_block(mstp_bridge_t *mstp,
                         uint portnum)
{
    if(!valid_port(mstp->n_ports, portnum))
        return NULL;

    return &mstp->conf.cist[portnum-1];
}

static inline 
mstp_msti_port_param_t *
vtss_mstp_get_msti_port_block(mstp_bridge_t *mstp,
                              uint msti,
                              uint portnum)
{
    if(msti >= N_MSTI_MAX ||
       !valid_port(mstp->n_ports, portnum))
        return NULL;

    uint ix = (mstp->n_ports*msti) + (portnum-1);
    return &mstp->conf.msti[ix];
}

#endif /* _VTSS_MSTP_MISC_H_ */
