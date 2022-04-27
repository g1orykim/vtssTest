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

#include "mstp_priv.h"
#include "mstp_misc.h"

void
mstp_update_linkspeed(mstp_bridge_t *mstp,
                      uint portnum,
                      mstp_cistport_t *cist, 
                      u32 linkspeed)
{
    cist->linkspeed = linkspeed;
    ForAllPortNo(mstp, portnum, 
                 {
                     u32 pc = calc_pathcost(_tp_->adminPathCost, linkspeed);
                     if(_tp_->portPathCost != pc) {
                         T_I("Port %d[%d] Speed change %u -> %u", 
                             _tp_->port_no, _tp_->msti, _tp_->portPathCost, pc);
                         _tp_->portPathCost = pc;
                         _tp_->reselect = TRUE;
                         _tp_->selected = FALSE;
                     }
                 });
}

void
mstp_update_point2point(mstp_port_t *port, mstp_cistport_t *cist, bool fdx)
{
    bool isp2p = FALSE;
    cist->fdx = fdx;
    if(cist->adminPointToPointMAC == P2P_AUTO)
        isp2p = cist->fdx; 
    else
        isp2p = (cist->adminPointToPointMAC == P2P_FORCETRUE);
    if(cist->operPointToPointMAC != isp2p) {
        T_I("Port %d p2p change %d -> %d", port->port_no,
            cist->operPointToPointMAC, isp2p);
        cist->operPointToPointMAC = isp2p;
    }
}

/** 13.7 mstp_config_digest()
 *
 * The Configuration Digest, a 16-octet signature of type HMAC-MD5
 * (see IETF RFC 2104) created from the MST Configuration Table (3.17,
 * 8.9). For the purposes of calculating the Configuration Digest, the
 * MST Configuration Table is considered to contain 4096 consecutive
 * two octet elements, where each element of the table (with the
 * exception of the first and last) contains an MSTID value encoded as
 * a binary number, with the first octet being most significant.  The
 * first element of the table contains the value 0, the second element
 * the MSTID value corresponding to VID 1, the third element the MSTID
 * value corresponding to VID 2, and so on, with the next to last
 * element of the table containing the MSTID value corresponding to
 * VID 4094, and the last element containing the value 0. The key used
 * to generate the signature consists of the 16-octet string specified
 * in Table 13-1 .
 *
 * \param map the MSTP config table to generate the MD5 digest
 * for. This has a 1-byte entry per VID, allowing for *only* MSTI =
 * MSTID kind of mapping.
 *
 * \param digest the output buffer for the generated MD5 digest
 * (length MD5_MAC_LEN)
 */
void
mstp_calc_digest(mstp_map_t *map, 
                 u8 *digest)
{
    static const u8 md5_key[] = { 
        0x13, 0xAC, 0x06, 0xA6, 0x2E, 0x47, 0xFD, 0x51, 
        0xF9, 0x5D, 0x2B, 0xA2, 0x43, 0xCD, 0x03, 0x46
    };

    mstp_mstidmap_t *dmap = vtss_mstp_malloc(sizeof(*dmap));
    VTSS_ASSERT(dmap != NULL);

    /* Store, network order */
    uint i;
    for(i = 0; i < ARR_SZ(dmap->map); i++) {
        u16 mstid = (u16) map->map[i];
        /* Entries are stored in network order (for the digest) */
        mstid = (mstid >> 8) + ((mstid & 0xF) << 8);
        dmap->map[i] = mstid;
    }

    vtss_hmac_md5(md5_key, sizeof(md5_key), (u8*) dmap->map, sizeof(*dmap), digest);
    vtss_mstp_free(dmap);
}
