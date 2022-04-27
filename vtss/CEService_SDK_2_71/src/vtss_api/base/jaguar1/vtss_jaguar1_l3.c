/*

 Vitesse API software.

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

 $Id$
 $Revision$

*/

#define VTSS_TRACE_GROUP VTSS_TRACE_GROUP_L3
#include "vtss_jaguar1_cil.h"

#if defined(VTSS_ARCH_JAGUAR_1)

/* - CIL functions ------------------------------------------------- */

/* ================================================================= *
 *  LPM functions
 * ================================================================= */

enum
{
    VTSS_LPM_ACCESS_CTRL_CMD_IDLE = 0,
    VTSS_LPM_ACCESS_CTRL_CMD_READ = 1,
    VTSS_LPM_ACCESS_CTRL_CMD_WRITE = 2,
    VTSS_LPM_ACCESS_CTRL_CMD_MOVE_UP = 3,
    VTSS_LPM_ACCESS_CTRL_CMD_MOVE_DOWN = 4,
    VTSS_LPM_ACCESS_CTRL_CMD_INIT = 5,
};

enum
{
    VTSS_LPM_ACCESS_CTRL_WID_QUAD = 0,
    VTSS_LPM_ACCESS_CTRL_WID_HALF = 1,
    VTSS_LPM_ACCESS_CTRL_WID_FULL = 2,
};

enum
{
    VTSS_LPM_TYPE_INVALID  = 0,
    VTSS_LPM_TYPE_IPV4_UC = 1,
    VTSS_LPM_TYPE_IPV4_MC = 2,
    VTSS_LPM_TYPE_IPV6_UC = 3,
};

static vtss_rc jr_lpm_table_idle(vtss_state_t *vtss_state)
{
    // TODO, timeout?

    u32 cmd;
    while (1) {
        JR_RD(ANA_L3_2_LPM, ACCESS_CTRL, &cmd);
        if ((VTSS_F_ANA_L3_2_LPM_ACCESS_CTRL_ACCESS_SHOT & cmd) == 0)
            break;
    }
    return VTSS_RC_OK;
}

static vtss_rc jr_l3_ipv4_uc_clear_chip(vtss_state_t *vtss_state, const u32 row)
{
    JR_WRX(ANA_L3_2_LPM, LPM_USAGE_CFG, 0,
            VTSS_F_ANA_L3_2_LPM_LPM_USAGE_CFG_LPM_TYPE(VTSS_LPM_TYPE_INVALID));

    JR_WR(ANA_L3_2_LPM, ACCESS_CTRL,
            VTSS_F_ANA_L3_2_LPM_ACCESS_CTRL_ACCESS_IDX(row) |
            VTSS_F_ANA_L3_2_LPM_ACCESS_CTRL_ACCESS_WID(
                VTSS_LPM_ACCESS_CTRL_WID_QUAD) |
            VTSS_F_ANA_L3_2_LPM_ACCESS_CTRL_ACCESS_CMD(
                VTSS_LPM_ACCESS_CTRL_CMD_WRITE) |
            VTSS_F_ANA_L3_2_LPM_ACCESS_CTRL_ACCESS_SHOT
         );

    VTSS_RC(jr_lpm_table_idle(vtss_state));

    return VTSS_RC_OK;
}

static vtss_rc jr_l3_ipv4_uc_clear(vtss_state_t *vtss_state, const u32 row)
{
    vtss_chip_no_t chip_no;

    for (chip_no = 0; chip_no < vtss_state->chip_count; chip_no++) {
        VTSS_SELECT_CHIP(chip_no);

        VTSS_RC(jr_l3_ipv4_uc_clear_chip(vtss_state, row));
    }

    return VTSS_RC_OK;
}

static vtss_rc jr_l3_ipv4_mc_clear_chip(vtss_state_t *vtss_state, const u32 row)
{
    JR_WRX(ANA_L3_2_LPM, LPM_USAGE_CFG, 0,
            VTSS_F_ANA_L3_2_LPM_LPM_USAGE_CFG_LPM_TYPE(VTSS_LPM_TYPE_INVALID));

    JR_WR(ANA_L3_2_LPM, ACCESS_CTRL,
            VTSS_F_ANA_L3_2_LPM_ACCESS_CTRL_ACCESS_IDX(row) |
            VTSS_F_ANA_L3_2_LPM_ACCESS_CTRL_ACCESS_WID(
                VTSS_LPM_ACCESS_CTRL_WID_HALF) |
            VTSS_F_ANA_L3_2_LPM_ACCESS_CTRL_ACCESS_CMD(
                VTSS_LPM_ACCESS_CTRL_CMD_WRITE) |
            VTSS_F_ANA_L3_2_LPM_ACCESS_CTRL_ACCESS_SHOT
         );

    VTSS_RC(jr_lpm_table_idle(vtss_state));

    return VTSS_RC_OK;
}

static vtss_rc jr_l3_ipv4_mc_clear(vtss_state_t *vtss_state, const u32 row)
{
    vtss_chip_no_t chip_no;

    for (chip_no = 0; chip_no < vtss_state->chip_count; chip_no++) {
        VTSS_SELECT_CHIP(chip_no);

        VTSS_RC(jr_l3_ipv4_mc_clear_chip(vtss_state, row));
    }

    return VTSS_RC_OK;
}

static vtss_rc jr_l3_ipv6_uc_clear_chip(vtss_state_t *vtss_state, const u32 row)
{
    JR_WRX(ANA_L3_2_LPM, LPM_USAGE_CFG, 0,
            VTSS_F_ANA_L3_2_LPM_LPM_USAGE_CFG_LPM_TYPE(VTSS_LPM_TYPE_INVALID));

    JR_WR(ANA_L3_2_LPM, ACCESS_CTRL,
            VTSS_F_ANA_L3_2_LPM_ACCESS_CTRL_ACCESS_IDX(row) |
            VTSS_F_ANA_L3_2_LPM_ACCESS_CTRL_ACCESS_WID(
                VTSS_LPM_ACCESS_CTRL_WID_FULL) |
            VTSS_F_ANA_L3_2_LPM_ACCESS_CTRL_ACCESS_CMD(
                VTSS_LPM_ACCESS_CTRL_CMD_WRITE) |
            VTSS_F_ANA_L3_2_LPM_ACCESS_CTRL_ACCESS_SHOT
         );

    VTSS_RC(jr_lpm_table_idle(vtss_state));

    return VTSS_RC_OK;
}

static vtss_rc jr_l3_ipv6_uc_clear(vtss_state_t *vtss_state, const u32 row)
{
    vtss_chip_no_t chip_no;

    for (chip_no = 0; chip_no < vtss_state->chip_count; chip_no++) {
        VTSS_SELECT_CHIP(chip_no);

        VTSS_RC(jr_l3_ipv6_uc_clear_chip(vtss_state, row));
    }

    return VTSS_RC_OK;
}

static vtss_rc jr_l3_ipv4_uc_set_chip(vtss_state_t *vtss_state,
                                      const u32 row, const u32 remap_ptr,
                                      const BOOL ecmp, const u32 address, const u32 netmask)
{
    JR_WRX(ANA_L3_2_LPM, LPM_DATA_CFG, 0, address);
    JR_WRX(ANA_L3_2_LPM, LPM_MASK_CFG, 0, ~netmask); // netmask must be inverted in the TCAM
    JR_WR(ANA_L3_2_REMAP, REMAP_CFG,
            (ecmp ? VTSS_F_ANA_L3_2_REMAP_REMAP_CFG_ECMP_CNT : 0 ) |
            VTSS_F_ANA_L3_2_REMAP_REMAP_CFG_BASE_PTR(remap_ptr)
         );
    JR_WRX(ANA_L3_2_LPM, LPM_USAGE_CFG, 0,
            VTSS_F_ANA_L3_2_LPM_LPM_USAGE_CFG_LPM_DST_FLAG_DATA |
            VTSS_F_ANA_L3_2_LPM_LPM_USAGE_CFG_LPM_TYPE(
                VTSS_LPM_TYPE_IPV4_UC)
          );
    JR_WR(ANA_L3_2_LPM, ACCESS_CTRL,
            VTSS_F_ANA_L3_2_LPM_ACCESS_CTRL_ACCESS_IDX(row) |
            VTSS_F_ANA_L3_2_LPM_ACCESS_CTRL_ACCESS_WID(
                VTSS_LPM_ACCESS_CTRL_WID_QUAD) |
            VTSS_F_ANA_L3_2_LPM_ACCESS_CTRL_ACCESS_CMD(
                VTSS_LPM_ACCESS_CTRL_CMD_WRITE) |
            VTSS_F_ANA_L3_2_LPM_ACCESS_CTRL_ACCESS_SHOT
         );

    VTSS_RC(jr_lpm_table_idle(vtss_state));

    return VTSS_RC_OK;
}

static vtss_rc jr_l3_ipv4_uc_set(vtss_state_t *vtss_state,
                                 const u32 row, const u32 remap_ptr,
                                 const BOOL ecmp, const u32 address, const u32 netmask)
{
    vtss_chip_no_t chip_no;

    for (chip_no = 0; chip_no < vtss_state->chip_count; chip_no++) {
        VTSS_SELECT_CHIP(chip_no);

        VTSS_RC(jr_l3_ipv4_uc_set_chip(vtss_state, row, remap_ptr, ecmp, address, netmask));
    }

    return VTSS_RC_OK;
}

static vtss_rc jr_l3_ipv6_uc_set_chip(vtss_state_t *vtss_state,
        const u32 row, const u32 remap_ptr,
        const BOOL ecmp, const u32 a0, const u32 a1, const u32 a2,
        const u32 a3, const u32 n0, const u32 n1, const u32 n2, const u32 n3 )
{
    int i;

    JR_WRX(ANA_L3_2_LPM, LPM_DATA_CFG, 0, a3);
    JR_WRX(ANA_L3_2_LPM, LPM_DATA_CFG, 1, a2);
    JR_WRX(ANA_L3_2_LPM, LPM_DATA_CFG, 2, a1);
    JR_WRX(ANA_L3_2_LPM, LPM_DATA_CFG, 3, a0);
    JR_WRX(ANA_L3_2_LPM, LPM_MASK_CFG, 0, ~n3); // netmask must be inverted in the TCAM
    JR_WRX(ANA_L3_2_LPM, LPM_MASK_CFG, 1, ~n2);
    JR_WRX(ANA_L3_2_LPM, LPM_MASK_CFG, 2, ~n1);
    JR_WRX(ANA_L3_2_LPM, LPM_MASK_CFG, 3, ~n0);

    JR_WR(ANA_L3_2_REMAP, REMAP_CFG,
            (ecmp ? VTSS_F_ANA_L3_2_REMAP_REMAP_CFG_ECMP_CNT : 0 ) |
            VTSS_F_ANA_L3_2_REMAP_REMAP_CFG_BASE_PTR(remap_ptr)
         );

    for (i = 0; i < 4; i++) {
        JR_WRX(ANA_L3_2_LPM, LPM_USAGE_CFG, i,
                VTSS_F_ANA_L3_2_LPM_LPM_USAGE_CFG_LPM_DST_FLAG_DATA |
                VTSS_F_ANA_L3_2_LPM_LPM_USAGE_CFG_LPM_TYPE(
                    VTSS_LPM_TYPE_IPV6_UC)
              );
    }

    JR_WR(ANA_L3_2_LPM, ACCESS_CTRL,
            VTSS_F_ANA_L3_2_LPM_ACCESS_CTRL_ACCESS_IDX(row) |
            VTSS_F_ANA_L3_2_LPM_ACCESS_CTRL_ACCESS_WID(
                VTSS_LPM_ACCESS_CTRL_WID_FULL) |
            VTSS_F_ANA_L3_2_LPM_ACCESS_CTRL_ACCESS_CMD(
                VTSS_LPM_ACCESS_CTRL_CMD_WRITE) |
            VTSS_F_ANA_L3_2_LPM_ACCESS_CTRL_ACCESS_SHOT
         );

    VTSS_RC(jr_lpm_table_idle(vtss_state));

    return VTSS_RC_OK;
}

static vtss_rc jr_l3_ipv6_uc_set(vtss_state_t *vtss_state, 
        const u32 row, const u32 remap_ptr,
        const BOOL ecmp, const u32 a0, const u32 a1, const u32 a2,
        const u32 a3, const u32 n0, const u32 n1, const u32 n2, const u32 n3 )
{
    vtss_chip_no_t chip_no;

    for (chip_no = 0; chip_no < vtss_state->chip_count; chip_no++) {
        VTSS_SELECT_CHIP(chip_no);

        VTSS_RC(jr_l3_ipv6_uc_set_chip(vtss_state,
                    row, remap_ptr, ecmp, a0, a1, a2, a3, n0, n1, n2, n3));
    }

    return VTSS_RC_OK;
}

static vtss_rc jr_l3_lpm_move_chip(vtss_state_t *vtss_state,
        const u32 idx, const u32 size,
        const u32 distance, const BOOL down)
{
    u32 tmp = 0;

    VTSS_D("idx=%u, n=%u, distance=%u, down=%hhu", idx, size, distance, down);

    JR_WR(ANA_L3_2_LPM, ACCESS_MV_CFG,
            VTSS_F_ANA_L3_2_LPM_ACCESS_MV_CFG_MV_NUM_POS(distance) |
            VTSS_F_ANA_L3_2_LPM_ACCESS_MV_CFG_MV_SIZE(size));

    tmp = VTSS_F_ANA_L3_2_LPM_ACCESS_CTRL_ACCESS_IDX(idx) |
        VTSS_F_ANA_L3_2_LPM_ACCESS_CTRL_ACCESS_SHOT;

    /* Invert direction */
    if( down )
        tmp |= VTSS_F_ANA_L3_2_LPM_ACCESS_CTRL_ACCESS_CMD(
                VTSS_LPM_ACCESS_CTRL_CMD_MOVE_UP);
    else
        tmp |= VTSS_F_ANA_L3_2_LPM_ACCESS_CTRL_ACCESS_CMD(
                VTSS_LPM_ACCESS_CTRL_CMD_MOVE_DOWN);

    JR_WR(ANA_L3_2_LPM, ACCESS_CTRL, tmp);
    VTSS_RC(jr_lpm_table_idle(vtss_state));

    return VTSS_RC_OK;
}

static vtss_rc jr_l3_lpm_move(vtss_state_t *vtss_state,
        const u32 idx, const u32 size,
        const u32 distance, const BOOL down)
{
    vtss_chip_no_t chip_no;

    for (chip_no = 0; chip_no < vtss_state->chip_count; chip_no++) {
        VTSS_SELECT_CHIP(chip_no);

        VTSS_RC(jr_l3_lpm_move_chip(vtss_state, idx, size, distance, down));
    }

    return VTSS_RC_OK;
}

static vtss_rc jr_l3_debug_sticky_clear_chip(vtss_state_t *vtss_state)
{
    JR_WR(ANA_L3_2, LPM_REMAP_STICKY_L3_LPM_REMAP_STICKY, 0xffffffff);
    JR_WR(ANA_L3_2, VLAN_ARP_L3MC_STICKY_VLAN_STICKY, 0xffffffff);
    JR_WR(ANA_L3_2, VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY, 0xffffffff);
    return VTSS_RC_OK;
}

static vtss_rc jr_l3_debug_sticky_clear(vtss_state_t *vtss_state)
{
    vtss_chip_no_t chip_no;

    for (chip_no = 0; chip_no < vtss_state->chip_count; chip_no++) {
        VTSS_SELECT_CHIP(chip_no);
        VTSS_RC(jr_l3_debug_sticky_clear_chip(vtss_state));
    }

    return VTSS_RC_OK;
}

static vtss_rc jr_l3_arp_set_chip(vtss_state_t *vtss_state,
                                  const u32 row,
                                  const vtss_mac_t * const mac, const vtss_vid_t evid)
{
    // We can not do atomic update, we msut therefor perform the
    // following steps:
    //  - invalidate the entry by update reg 0
    //  - write valid information to reg 1
    //  - write valid information to reg 0, and enable the entry
    JR_WRX(ANA_L3_2_ARP, ARP_CFG_0, row, 0);
    JR_WRX(ANA_L3_2_ARP, ARP_CFG_1, row,
            ( (mac->addr[2] << 24) | (mac->addr[3] << 16) |
              (mac->addr[4] <<  8) |  mac->addr[5]) );

    JR_WRX(ANA_L3_2_ARP, ARP_CFG_0, row,
            VTSS_F_ANA_L3_2_ARP_ARP_CFG_0_MAC_MSB(
                (mac->addr[0] << 8) | mac->addr[1] ) |
            VTSS_F_ANA_L3_2_ARP_ARP_CFG_0_ARP_VMID(evid) |
            VTSS_F_ANA_L3_2_ARP_ARP_CFG_0_ARP_ENA);

    return VTSS_RC_OK;
}

static vtss_rc jr_l3_arp_set(vtss_state_t *vtss_state,
                             const u32 row, const vtss_mac_t * const mac,
                             const vtss_vid_t evid)
{
    vtss_chip_no_t chip_no;

    for (chip_no = 0; chip_no < vtss_state->chip_count; chip_no++) {
        VTSS_SELECT_CHIP(chip_no);

        VTSS_RC(jr_l3_arp_set_chip(vtss_state, row, mac, evid));
    }

    return VTSS_RC_OK;
}

static vtss_rc jr_l3_arp_clear_chip(vtss_state_t *vtss_state, const u32 row)
{
    JR_WRX(ANA_L3_2_ARP, ARP_CFG_0, row, 0);
    JR_WRX(ANA_L3_2_ARP, ARP_CFG_1, row, 0);

    return VTSS_RC_OK;
}

static vtss_rc jr_l3_arp_clear(vtss_state_t *vtss_state, const u32 row)
{
    vtss_chip_no_t chip_no;

    for (chip_no = 0; chip_no < vtss_state->chip_count; chip_no++) {
        VTSS_SELECT_CHIP(chip_no);

        VTSS_RC(jr_l3_arp_clear_chip(vtss_state, row));
    }

    return VTSS_RC_OK;
}

static vtss_rc jr_l3_common_set_chip(vtss_state_t *vtss_state,
                                     const vtss_l3_common_conf_t * const conf)
{
    u32 lsb = 0;
    u32 msb = 0;

    if( conf->rleg_mode != VTSS_ROUTING_RLEG_MAC_MODE_SINGLE ) {
        return VTSS_RC_ERROR;
    }

    // NOTE: we do not use the routing_enable flag here, as the LPM table is
    // used even when routing are disabled.

    msb <<= 8; msb |= (u32)conf->base_address.addr[0];
    msb <<= 8; msb |= (u32)conf->base_address.addr[1];
    msb <<= 8; msb |= (u32)conf->base_address.addr[2];
    lsb <<= 8; lsb |= (u32)conf->base_address.addr[3];
    lsb <<= 8; lsb |= (u32)conf->base_address.addr[4];
    lsb <<= 8; lsb |= (u32)conf->base_address.addr[5];

    JR_WR(ANA_L3_2_COMMON, RLEG_CFG_0,
          VTSS_F_ANA_L3_2_COMMON_RLEG_CFG_0_RLEG_MAC_LSB( lsb ));
    JR_WR(REW_COMMON, RLEG_CFG_0,
          VTSS_F_REW_COMMON_RLEG_CFG_0_RLEG_MAC_LSB( lsb ));

#define RLEG_MAC_TYPE_SEL 2
    JR_WR(ANA_L3_2_COMMON, RLEG_CFG_1,
          VTSS_F_ANA_L3_2_COMMON_RLEG_CFG_1_RLEG_MAC_TYPE_SEL(RLEG_MAC_TYPE_SEL) |
          VTSS_F_ANA_L3_2_COMMON_RLEG_CFG_1_RLEG_MAC_MSB(msb));
    JR_WR(REW_COMMON, RLEG_CFG_1,
          VTSS_F_REW_COMMON_RLEG_CFG_1_RLEG_MAC_TYPE_SEL(RLEG_MAC_TYPE_SEL) |
          VTSS_F_REW_COMMON_RLEG_CFG_1_RLEG_MAC_MSB(msb));
#undef RLEG_MAC_TYPE_SEL

    return VTSS_RC_OK;
}

static vtss_rc jr_l3_common_set(vtss_state_t *vtss_state,
                                const vtss_l3_common_conf_t * const conf)
{
    vtss_chip_no_t chip_no;

    for (chip_no = 0; chip_no < vtss_state->chip_count; chip_no++) {
        VTSS_SELECT_CHIP(chip_no);

        VTSS_RC(jr_l3_common_set_chip(vtss_state, conf));
    }

    return VTSS_RC_OK;
}

static vtss_rc jr_l3_vlan_set_chip(vtss_state_t *vtss_state,
                                   const vtss_l3_rleg_id_t rleg_id,
                                   const vtss_vid_t        vlan,
                                   const BOOL              enable)
{
    if (enable) {
        VTSS_DG(VTSS_TRACE_GROUP_L3, "Enable rleg_id %u, vlan %u", rleg_id, vlan);
        JR_WRX(ANA_L3_2_VLAN, VMID_CFG, vlan,
                VTSS_F_ANA_L3_2_VLAN_VMID_CFG_VMID(rleg_id));
        JR_WRXM_SET(ANA_L3_2_VLAN, VLAN_CFG, vlan,
                VTSS_F_ANA_L3_2_VLAN_VLAN_CFG_VLAN_RLEG_ENA);
    } else {
        VTSS_DG(VTSS_TRACE_GROUP_L3, "disable rleg_id %u, vlan %u", rleg_id, vlan);
        JR_WRXM_CLR(ANA_L3_2_VLAN, VLAN_CFG, vlan,
                VTSS_F_ANA_L3_2_VLAN_VLAN_CFG_VLAN_RLEG_ENA);
    }

    return VTSS_RC_OK;
}

static vtss_rc jr_l3_vlan_set(vtss_state_t *vtss_state,
                              const vtss_l3_rleg_id_t rleg_id,
                              const vtss_vid_t        vlan,
                              const BOOL              enable)
{
    vtss_chip_no_t chip_no;

    for (chip_no = 0; chip_no < vtss_state->chip_count; chip_no++) {
        VTSS_SELECT_CHIP(chip_no);
        VTSS_RC(jr_l3_vlan_set_chip(vtss_state, rleg_id, vlan, enable));
    }

    return VTSS_RC_OK;
}

static vtss_rc jr_l3_rleg_set_chip(vtss_state_t *vtss_state,
                                   const vtss_l3_rleg_id_t   rleg,
                                   const vtss_l3_rleg_conf_t *const conf )
{
    u32 tmp = 0;

#define COND_APPEND(x, y, z) \
    x |= conf-> y ? VTSS_F_ANA_L3_2_VMID_RLEG_CTRL_RLEG_## z : 0;
    COND_APPEND(tmp, ipv4_unicast_enable, IP4_UC_ENA);
    COND_APPEND(tmp, ipv6_unicast_enable, IP6_UC_ENA);
    COND_APPEND(tmp, ipv4_icmp_redirect_enable, IP4_ICMP_REDIR_ENA);
    COND_APPEND(tmp, ipv6_icmp_redirect_enable, IP6_ICMP_REDIR_ENA);
#undef COND_APPEND
    tmp |= VTSS_F_ANA_L3_2_VMID_RLEG_CTRL_RLEG_EVID(conf->vlan);

    JR_WRX(ANA_L3_2_VMID, RLEG_CTRL, rleg, tmp);

    JR_WRX(REW, VMID_RLEG_CTRL, rleg,
           VTSS_F_REW_VMID_RLEG_CTRL_RLEG_EVID(conf->vlan));

    return VTSS_RC_OK;
}

static vtss_rc jr_l3_rleg_set(vtss_state_t *vtss_state,
                              const vtss_l3_rleg_id_t   rleg,
                              const vtss_l3_rleg_conf_t * const conf)
{
    vtss_chip_no_t chip_no;

    for (chip_no = 0; chip_no < vtss_state->chip_count; chip_no++) {
        VTSS_SELECT_CHIP(chip_no);

        VTSS_RC(jr_l3_rleg_set_chip(vtss_state, rleg, conf));
    }

    return VTSS_RC_OK;
}

enum {
    VTSS_RLEG_EVENT_MASK_ACL_DIS = VTSS_BIT(0),
    VTSS_RLEG_EVENT_MASK_TOTAL = VTSS_BIT(1),
    VTSS_RLEG_EVENT_MASK_VLAN_DIS = VTSS_BIT(2),
    VTSS_RLEG_EVENT_MASK_IPV4_UC = VTSS_BIT(3),
    VTSS_RLEG_EVENT_MASK_IPV6_UC = VTSS_BIT(4),
    VTSS_RLEG_EVENT_MASK_IP_MC = VTSS_BIT(5),
    VTSS_RLEG_EVENT_MASK_IP_MC_DIS = VTSS_BIT(6),
    VTSS_RLEG_EVENT_MASK_TTS_DIS = VTSS_BIT(7),
};

enum {
    VTSS_RLEG_FRAME_CNT = 0,
    VTSS_RLEG_BYTE_CNT = 1,
};

/* L3 IRLEG COUNTER CONFIGURATION */
//                         IDX TRIGGER-MASK                    COUNTER TYPE         DESTINATION
#define VTSS_L3_IRLEG_CNT0 (0, VTSS_RLEG_EVENT_MASK_IPV4_UC,   VTSS_RLEG_FRAME_CNT, ipv4uc_received_frames)
#define VTSS_L3_IRLEG_CNT1 (1, VTSS_RLEG_EVENT_MASK_IPV4_UC,   VTSS_RLEG_BYTE_CNT,  ipv4uc_received_octets)
#define VTSS_L3_IRLEG_CNT2 (2, VTSS_RLEG_EVENT_MASK_IPV6_UC,   VTSS_RLEG_FRAME_CNT, ipv6uc_received_frames)
#define VTSS_L3_IRLEG_CNT3 (3, VTSS_RLEG_EVENT_MASK_IPV6_UC,   VTSS_RLEG_BYTE_CNT,  ipv6uc_received_octets)
//#define VTSS_L3_IRLEG_CNT4
//#define VTSS_L3_IRLEG_CNT5

/* L3 ERLEG COUNTER CONFIGURATION */
//                         IDX TRIGGER-MASK                    COUNTER TYPE         DESTINATION
#define VTSS_L3_ERLEG_CNT0 (0, VTSS_RLEG_EVENT_MASK_IPV4_UC,   VTSS_RLEG_FRAME_CNT, ipv4uc_transmitted_frames)
#define VTSS_L3_ERLEG_CNT1 (1, VTSS_RLEG_EVENT_MASK_IPV4_UC,   VTSS_RLEG_BYTE_CNT,  ipv4uc_transmitted_octets)
#define VTSS_L3_ERLEG_CNT2 (2, VTSS_RLEG_EVENT_MASK_IPV6_UC,   VTSS_RLEG_FRAME_CNT, ipv6uc_transmitted_frames)
#define VTSS_L3_ERLEG_CNT3 (3, VTSS_RLEG_EVENT_MASK_IPV6_UC,   VTSS_RLEG_BYTE_CNT,  ipv6uc_transmitted_octets)
//#define VTSS_L3_ERLEG_CNT4
//#define VTSS_L3_ERLEG_CNT5

static vtss_rc jr_l3_rleg_hw_stat_poll_chip(vtss_state_t *vtss_state,
                                            vtss_l3_rleg_id_t rleg,
                                            vtss_l3_counters_t *const stat)
{
#define CNT_POLL(idx, direction, dest)                                  \
    {                                                                   \
        u64 val = 0;                                                    \
        u32 msb = 0, msb2 = 0, lsb = 0;                                 \
        JR_RDXY(ANA_AC, STAT_CNT_CFG_##direction##_STAT_MSB_CNT,        \
                rleg, idx, &msb);                                       \
        JR_RDXY(ANA_AC, STAT_CNT_CFG_##direction##_STAT_LSB_CNT,        \
                rleg, idx, &lsb);                                       \
        JR_RDXY(ANA_AC, STAT_CNT_CFG_##direction##_STAT_MSB_CNT,        \
                rleg, idx, &msb2);                                      \
        if( msb != msb2 ) /* detect lsb wrap around */                  \
            lsb = 0;                                                    \
        val = msb2;                                                     \
        val <<= 32;                                                     \
        val |= (u64)lsb;                                                \
        stat -> dest += val;                                            \
    }

#define CNT_POLL_IRLEG(T) CNT_POLL_IRLEG_ T
#define CNT_POLL_IRLEG_(idx, mask, cnt_type, dest) \
    CNT_POLL(idx, IRLEG, dest)
    CNT_POLL_IRLEG(VTSS_L3_IRLEG_CNT0);
    CNT_POLL_IRLEG(VTSS_L3_IRLEG_CNT1);
    CNT_POLL_IRLEG(VTSS_L3_IRLEG_CNT2);
    CNT_POLL_IRLEG(VTSS_L3_IRLEG_CNT3);
    //CNT_POLL_IRLEG(VTSS_L3_IRLEG_CNT4);
    //CNT_POLL_IRLEG(VTSS_L3_IRLEG_CNT5);
#undef CNT_POLL_IRLEG
#undef CNT_POLL_IRLEG_

#define CNT_POLL_ERLEG(T) CNT_POLL_ERLEG_ T
#define CNT_POLL_ERLEG_(idx, mask, cnt_type, dest) \
    CNT_POLL(idx, ERLEG, dest)
    CNT_POLL_ERLEG(VTSS_L3_ERLEG_CNT0);
    CNT_POLL_ERLEG(VTSS_L3_ERLEG_CNT1);
    CNT_POLL_ERLEG(VTSS_L3_ERLEG_CNT2);
    CNT_POLL_ERLEG(VTSS_L3_ERLEG_CNT3);
    //CNT_POLL_ERLEG(VTSS_L3_ERLEG_CNT4);
    //CNT_POLL_ERLEG(VTSS_L3_ERLEG_CNT5);
#undef CNT_POLL_ERLEG
#undef CNT_POLL_ERLEG_

#undef CNT_POLL

    return VTSS_RC_OK;
}

static vtss_rc jr_l3_rleg_stat_reset_chip(vtss_state_t *vtss_state)
{
    JR_WR(ANA_AC, STAT_GLOBAL_CFG_ERLEG_STAT_RESET, 1);
    JR_WR(ANA_AC, STAT_GLOBAL_CFG_IRLEG_STAT_RESET, 1);

    JR_POLL_BIT(ANA_AC, STAT_GLOBAL_CFG_ERLEG_STAT_RESET, RESET);
    JR_POLL_BIT(ANA_AC, STAT_GLOBAL_CFG_IRLEG_STAT_RESET, RESET);

    return VTSS_RC_OK;
}

static vtss_rc jr_l3_rleg_stat_reset(vtss_state_t *vtss_state)
{
    vtss_chip_no_t chip_no;

    for (chip_no = 0; chip_no < vtss_state->chip_count; chip_no++) {
        VTSS_SELECT_CHIP(chip_no);

        VTSS_RC(jr_l3_rleg_stat_reset_chip(vtss_state));
    }

    return VTSS_RC_OK;
}

static vtss_rc jr_l3_rleg_hw_stat_poll(vtss_state_t *vtss_state,
                                       const vtss_l3_rleg_id_t rleg,
                                       vtss_l3_counters_t      *const stat )
{
    vtss_chip_no_t chip_no;
    memset(stat, 0, sizeof(vtss_l3_counters_t));

    for (chip_no = 0; chip_no < vtss_state->chip_count; chip_no++) {
        VTSS_SELECT_CHIP(chip_no);

        VTSS_RC(jr_l3_rleg_hw_stat_poll_chip(vtss_state, rleg, stat));
    }

    return VTSS_RC_OK;
}

#if defined(VTSS_SW_OPTION_L3RT)
static vtss_rc jr_l3_rleg_stat_poll(vtss_state_t *vtss_state)
{
    u32 i;
    u32 bad_cnt = 0;
    u32 good_cnt = 0;
    u32 incomplete_cnt = 0;

#define CNT_MAX 0x000000FFFFFFFFFFLLU
#define CNT_VAL(NEW_VAL, OLD_VAL)                            \
        (                                                    \
           (NEW_VAL) >= (OLD_VAL) ?                          \
              ((NEW_VAL) - (OLD_VAL))                        \
           :                                                 \
              (((CNT_MAX) - (NEW_VAL)) + (OLD_VAL) + (1LLU)) \
        )

#define CNT_UPDATE(DST, SAMPLE, SHADOW)                      \
    (DST) = CNT_VAL((SAMPLE), (SHADOW));                     \
    (SHADOW) = (SAMPLE);

#define CNT_ACC(DST_A, DST_B, SAMPLE, SHADOW, FIELD)         \
    {                                                        \
        unsigned long long d = 0;                            \
        CNT_UPDATE(d, (SAMPLE).FIELD, (SHADOW).FIELD);       \
        DST_A.FIELD += d;                                    \
        DST_B.FIELD += d;                                    \
    }

    for (i = 0; i < VTSS_RLEG_CNT; ++i) {
        vtss_l3_counters_t stat_sample;
        vtss_rc rc = jr_l3_rleg_hw_stat_poll(vtss_state, i, &stat_sample);

        switch( rc ) {
        case VTSS_RC_OK:
            good_cnt++;
            break;

        case VTSS_RC_ERROR:
            bad_cnt++;
            break;

        default:
            incomplete_cnt++;
            break;
        }

        if( rc != VTSS_RC_OK )
            continue;

#define ACC(I, F)                                                    \
    CNT_ACC(vtss_state->l3.statistics.interface_counter[(I)],        \
            vtss_state->l3.statistics.system_counter,                \
            stat_sample,                                             \
            vtss_state->l3.statistics.interface_shadow_counter[(I)], \
            F)
        ACC(i, ipv4uc_received_octets);
        ACC(i, ipv4uc_received_frames);
        ACC(i, ipv6uc_received_octets);
        ACC(i, ipv6uc_received_frames);
        ACC(i, ipv4uc_transmitted_octets);
        ACC(i, ipv4uc_transmitted_frames);
        ACC(i, ipv6uc_transmitted_octets);
        ACC(i, ipv6uc_transmitted_frames);
#undef ACC
    }

#undef CNT_ACC
#undef CNT_UPDATE
#undef CNT_VAL
#undef CNT_MAX

    if (bad_cnt == 0 && incomplete_cnt == 0)
        return VTSS_RC_OK;

    if (incomplete_cnt == 0 && good_cnt == 0)
        return VTSS_RC_ERROR;

    return VTSS_RC_INCOMPLETE;
}
#endif /* VTSS_SW_OPTION_L3RT) */

static vtss_rc jr_lpm_l3_chip(vtss_state_t *vtss_state)
{
    /* Start initializing of the LPM TMAM */
    JR_WR(ANA_L3_2_TCAM_BIST, TCAM_CTRL,
            VTSS_F_ANA_L3_2_TCAM_BIST_TCAM_CTRL_TCAM_BIST |
            VTSS_F_ANA_L3_2_TCAM_BIST_TCAM_CTRL_TCAM_INIT );

    /* Start initializing of arp, ipmc and vmid */
#define INIT_MASK \
    ( VTSS_F_ANA_L3_2_COMMON_TABLE_CTRL_INIT_ARP_SHOT  | \
      VTSS_F_ANA_L3_2_COMMON_TABLE_CTRL_INIT_IPMC_SHOT | \
      VTSS_F_ANA_L3_2_COMMON_TABLE_CTRL_INIT_VMID_SHOT )
    JR_WR(ANA_L3_2_COMMON, TABLE_CTRL, INIT_MASK);

    /* Configure the 6 erleg counters and the 6 irleg counters.
     *
     * To change the counter configuration, change the tuples
     * VTSS_L3_ERLEG_CNT0-5 and VTSS_L3_IRLEG_CNT0-5 instead */
#define CNT_CONF_IRLEG(T) CNT_CONF_IRLEG_ T
#define CNT_CONF_IRLEG_(idx, mask, cnt_type, dest) \
    JR_WRX(ANA_AC, STAT_GLOBAL_CFG_IRLEG_STAT_GLOBAL_CFG, idx, cnt_type); \
    JR_WRX(ANA_AC, STAT_GLOBAL_CFG_IRLEG_STAT_GLOBAL_EVENT_MASK, idx, mask);

#define CNT_CONF_ERLEG(T) CNT_CONF_ERLEG_ T
#define CNT_CONF_ERLEG_(idx, mask, cnt_type, dest) \
    JR_WRX(ANA_AC, STAT_GLOBAL_CFG_ERLEG_STAT_GLOBAL_CFG, idx, cnt_type); \
    JR_WRX(ANA_AC, STAT_GLOBAL_CFG_ERLEG_STAT_GLOBAL_EVENT_MASK, idx, mask);

    CNT_CONF_IRLEG(VTSS_L3_IRLEG_CNT0);
    CNT_CONF_IRLEG(VTSS_L3_IRLEG_CNT1);
    CNT_CONF_IRLEG(VTSS_L3_IRLEG_CNT2);
    CNT_CONF_IRLEG(VTSS_L3_IRLEG_CNT3);
    //CNT_CONF_IRLEG(VTSS_L3_IRLEG_CNT4);
    //CNT_CONF_IRLEG(VTSS_L3_IRLEG_CNT5);
    CNT_CONF_ERLEG(VTSS_L3_ERLEG_CNT0);
    CNT_CONF_ERLEG(VTSS_L3_ERLEG_CNT1);
    CNT_CONF_ERLEG(VTSS_L3_ERLEG_CNT2);
    CNT_CONF_ERLEG(VTSS_L3_ERLEG_CNT3);
    //CNT_CONF_ERLEG(VTSS_L3_ERLEG_CNT4);
    //CNT_CONF_ERLEG(VTSS_L3_ERLEG_CNT5);
    /* Counter configuration is completed */

    // Reset counters after setup
    VTSS_RC(jr_l3_rleg_stat_reset_chip(vtss_state));

    // Forward all router errors to CPU
    JR_WR(ANA_L3_2_COMMON, ROUTING_CFG,
          VTSS_F_ANA_L3_2_COMMON_ROUTING_CFG_ALWAYS_SWITCHED_COPY_ENA |
          VTSS_F_ANA_L3_2_COMMON_ROUTING_CFG_DIP_ADDR_VIOLATION_REDIR_ENA(15) |
          VTSS_F_ANA_L3_2_COMMON_ROUTING_CFG_SIP_ADDR_VIOLATION_REDIR_ENA(7) |
          VTSS_F_ANA_L3_2_COMMON_ROUTING_CFG_CPU_RLEG_IP_HDR_FAIL_REDIR_ENA |
          VTSS_F_ANA_L3_2_COMMON_ROUTING_CFG_CPU_IP6_HOPBYHOP_REDIR_ENA |
          VTSS_F_ANA_L3_2_COMMON_ROUTING_CFG_CPU_IP4_OPTIONS_REDIR_ENA |
          VTSS_F_ANA_L3_2_COMMON_ROUTING_CFG_IP6_HC_REDIR_ENA |
          VTSS_F_ANA_L3_2_COMMON_ROUTING_CFG_IP4_TTL_REDIR_ENA
    );

    // Routing related stacking settings. No sideeffects when not used in stack
    JR_WRM_SET(ANA_AC_PS_COMMON, COMMON_VSTAX_CFG,
               VTSS_F_ANA_AC_PS_COMMON_COMMON_VSTAX_CFG_VSTAX2_RT_MC_SRC_LRN_SKIP_ENA |
               VTSS_F_ANA_AC_PS_COMMON_COMMON_VSTAX_CFG_VSTAX2_RT_UC_SRC_LRN_SKIP_ENA |
               VTSS_F_ANA_AC_PS_COMMON_COMMON_VSTAX_CFG_VSTAX2_MC_ROUTE_TO_STACK_LINK_ENA(0)
    );

    { /* wait for TCAM init to complete */
        u32 clear_bit = 0;
        while(1) {
            JR_RD(ANA_L3_2_TCAM_BIST, TCAM_CTRL, &clear_bit);
            if( clear_bit == 0 )
                break;
        }
    }

    { /* wait for arp, ipmc and vmid */
        u32 clear_bit = 0;

        while(1){
            JR_RD(ANA_L3_2_COMMON, TABLE_CTRL, &clear_bit);
            if( (clear_bit & INIT_MASK) == 0 )
                break;
        }
    }

#undef INIT_MASK
#undef CNT_CONF_IRLEG
#undef CNT_CONF_ERLEG
#undef CNT_CONF_IRLEG_
#undef CNT_CONF_ERLEG_

    return VTSS_RC_OK;
}

static vtss_rc jr_routing_uc_global_enable_chip(vtss_state_t *vtss_state)
{
    // Enable routing on all ports, including stack ports.
    // The reason for enabling routing on stack ports as well
    // is that a bug in Jaguar causes frames that are H/W-forwarded
    // from CPU Rx queues on one unit towards CPU Rx
    // queues on another unit to otherwise hit the same CPU Rx queue,
    // making it impossible to prioritize traffic on the receiving
    // unit. By enabling routing on stack ports, frames not already
    // routed will be redirected to CPU Rx queues according to
    // the ANA_L3:COMMON:CPU_QU_CFG register. In principle,
    // we could live with only enabling routing on the master unit's
    // stack ports, but doing it on all units simplifies the implementation.
    // A draw-back of enabling routing on even only the master's stack ports
    // is if a front port is *not* enabled for routing,
    // then frames subject to routing (i.e. with DMAC = L3 base address),
    // might get forwarded to stack links as a result of normal bridging,
    // and get routed whenever it hits a stack port. But in our current
    // implementation, routing is always enabled on all front ports as well.
    // To overcome the problem, an IS2 rule could be implemented on ports
    // not enabled for routing.
    u32 portmask = 0xffffffff;
    JR_WR(ANA_L3_2_COMMON, L3_UC_ENA, portmask);
    return VTSS_RC_OK;
}

static vtss_rc jr_init_l3_chip(vtss_state_t *vtss_state)
{
    VTSS_RC(jr_lpm_l3_chip(vtss_state));
    VTSS_RC(jr_routing_uc_global_enable_chip(vtss_state));

    return VTSS_RC_OK;
}

/* - Debug print --------------------------------------------------- */

static vtss_rc jr_debug_routing(vtss_state_t *vtss_state,
                                const vtss_debug_printf_t pr,
                                const vtss_debug_info_t   *const info)
{
    u32 tmp, tmp0, tmp1, mac_lsb, mac_msb;
    u32 row;
    u32 data, mask, conf, remap, arp0, arp1;
    vtss_mac_t mac;

#define FLAG(X, Y) ((Y) & VTSS_F_ANA_L3_2_ ## X )
#define ENA_DIS(X, Y) (FLAG(X, Y) ? "ENA":"DIS")
#define RAW_REG(var, fam, reg)                              \
    {                                                       \
        var = 0;                                            \
        JR_RD(fam, reg, &var);                              \
        pr("%-35s: 0x%08x\n", "VTSS_" #fam "_" #reg, var); \
    }
#define REG_BIT_DESC(var, desc, mask)      \
    {                                      \
        if( var & mask)                    \
            pr("  %-35s: ON\n", desc);     \
        else                               \
            pr("  %-35s: OFF\n", desc);    \
    }
#define REG_BIT(var, fam, reg, mask) \
    REG_BIT_DESC(var, #mask, VTSS_F_##fam##_##reg##_##mask)
#define MAC_FORMAT "%02x:%02x:%02x:%02x:%02x:%02x"
#define MAC_ARGS(X) \
    X.addr[0], X.addr[1], X.addr[2], X.addr[3], X.addr[4], X.addr[5]

    // TODO, extract and print individual bit fields
    pr("Raw dump of global routing related registers:\n");
    pr("=============================================\n");
    RAW_REG(tmp, ANA_L3_2_COMMON, L3_UC_ENA);
    RAW_REG(tmp, ANA_L3_2_COMMON, L3_MC_ENA);

    RAW_REG(tmp, ANA_L3_2_COMMON, ROUTING_CFG);
    REG_BIT(tmp, ANA_L3_2_COMMON, ROUTING_CFG, ALWAYS_SWITCHED_COPY_ENA);
    pr("  DIP_ADDR_VIOLATION_REDIR_ENA:\n");
    REG_BIT_DESC(tmp, "    0.0.0.0-0.255.255.255 ", VTSS_BIT(8));
    REG_BIT_DESC(tmp, "    127.0.0.0-127.255.255.255 ", VTSS_BIT(9));
    REG_BIT_DESC(tmp, "    240.0.0.0-255.255.255.254 ", VTSS_BIT(10));
    REG_BIT_DESC(tmp, "    224.0.0.0-239.255.255.255 ", VTSS_BIT(11));
    pr("  SIP_ADDR_VIOLATION_REDIR_ENA :\n");
    REG_BIT_DESC(tmp, "    0.0.0.0-0.255.255.255 ", VTSS_BIT(8));
    REG_BIT_DESC(tmp, "    127.0.0.0-127.255.255.255 ", VTSS_BIT(9));
    REG_BIT_DESC(tmp, "    224.0.0.0-255.255.255.255 ", VTSS_BIT(11));
    REG_BIT(tmp, ANA_L3_2_COMMON, ROUTING_CFG, CPU_RLEG_IP_HDR_FAIL_REDIR_ENA);
    REG_BIT(tmp, ANA_L3_2_COMMON, ROUTING_CFG, CPU_IP6_HOPBYHOP_REDIR_ENA);
    REG_BIT(tmp, ANA_L3_2_COMMON, ROUTING_CFG, CPU_IP4_OPTIONS_REDIR_ENA);
    REG_BIT(tmp, ANA_L3_2_COMMON, ROUTING_CFG, IP6_HC_REDIR_ENA);
    REG_BIT(tmp, ANA_L3_2_COMMON, ROUTING_CFG, IP4_TTL_REDIR_ENA);

    RAW_REG(tmp0, ANA_L3_2_COMMON, RLEG_CFG_0);
    RAW_REG(tmp1, ANA_L3_2_COMMON, RLEG_CFG_1);

    mac_lsb = VTSS_X_ANA_L3_2_COMMON_RLEG_CFG_0_RLEG_MAC_LSB(tmp0);
    mac.addr[5] = (mac_lsb) & 0xff;
    mac.addr[4] = (mac_lsb >> 8) & 0xff;
    mac.addr[3] = (mac_lsb >> 16) & 0xff;

    mac_msb = VTSS_X_ANA_L3_2_COMMON_RLEG_CFG_1_RLEG_MAC_MSB(tmp1);
    mac.addr[2] = (mac_msb) & 0xff;
    mac.addr[1] = (mac_msb >> 8) & 0xff;
    mac.addr[0] = (mac_msb >> 16) & 0xff;

    pr("  RLEG_MAC_TYPE_SEL (%u):\n",
            VTSS_X_ANA_L3_2_COMMON_RLEG_CFG_1_RLEG_MAC_TYPE_SEL(tmp1) );
    switch( VTSS_X_ANA_L3_2_COMMON_RLEG_CFG_1_RLEG_MAC_TYPE_SEL(tmp1) ) {
        case 0:
            pr("    RLEG_MAC_ADDR + VIMD\n");
            break;
        case 1:
            pr("    RLEG_MAC_ADDR + VID\n");
            break;
        default:
            pr("    COMMON MAC ADDRESS FOR ALL LEGS\n");
            break;
    }
    pr("  RLEG_MAC_ADDR: " MAC_FORMAT "\n", MAC_ARGS(mac));
    RAW_REG(tmp, ANA_L3_2_COMMON, VRRP_CFG_0);
    RAW_REG(tmp, ANA_L3_2_COMMON, VRRP_CFG_1);


    pr(" \n");
    pr("VLAN to rleg association\n");
    pr("========================\n");
    pr("VLAN ID   ->     RLEG ID\n");
    pr("------------------------\n");
    //("0000      ->     0000\n");
    for( row = 0; row < VTSS_VIDS; ++row ) {
        JR_RDX(ANA_L3_2_VLAN, VLAN_CFG, row, &tmp);

        if( tmp & VTSS_F_ANA_L3_2_VLAN_VLAN_CFG_VLAN_RLEG_ENA ) {
            u32 vid;

            JR_RDX(ANA_L3_2_VLAN, VMID_CFG, row, &vid);
            pr("%4u      ->     %4u\n",
                    row, VTSS_X_ANA_L3_2_VLAN_VMID_CFG_VMID(vid));
        }
    }
    pr(" \n");

    pr("Routing leg configurations\n");
    pr("==========================\n");
    pr("ID   EVID IP4_MC_TTL VRID IP6_ICMP_REDIR IP6_UC IP4_MC IP4_ICMP_REDIR IP4_UC VRID-0 VRID-1\n");
    pr("------------------------------------------------------------------------------------------\n");
    //("0000 0000 000        DIS  DIS            DIS    DIS    DIS            DIS       000    000\n");
    // TODO, MAGIC!
    for( row = 0; row < 128; ++row ) {

        JR_RDX(ANA_L3_2_VMID, RLEG_CTRL, row, &tmp);

        if( ! ((tmp & VTSS_F_ANA_L3_2_VMID_RLEG_CTRL_RLEG_VRID_ENA) ||
                (tmp & VTSS_F_ANA_L3_2_VMID_RLEG_CTRL_RLEG_IP6_ICMP_REDIR_ENA) ||
                (tmp & VTSS_F_ANA_L3_2_VMID_RLEG_CTRL_RLEG_IP6_UC_ENA) ||
                (tmp & VTSS_F_ANA_L3_2_VMID_RLEG_CTRL_RLEG_IP4_MC_ENA) ||
                (tmp & VTSS_F_ANA_L3_2_VMID_RLEG_CTRL_RLEG_IP4_ICMP_REDIR_ENA) ||
                (tmp & VTSS_F_ANA_L3_2_VMID_RLEG_CTRL_RLEG_IP4_UC_ENA)) )
            continue;

        pr("%4u ", row); // ID
        pr("%4u ", VTSS_X_ANA_L3_2_VMID_RLEG_CTRL_RLEG_EVID(tmp)); // EVID
        pr("%3u        ", // IP4_MC_TTL
                VTSS_X_ANA_L3_2_VMID_RLEG_CTRL_RLEG_IP4_MC_TTL(tmp));

        pr("%s  ", ENA_DIS(VMID_RLEG_CTRL_RLEG_VRID_ENA, tmp)); // VRID
        pr("%s            ", // IP6_ICMP_REDIR
                ENA_DIS(VMID_RLEG_CTRL_RLEG_IP6_ICMP_REDIR_ENA, tmp));
        pr("%s    ", ENA_DIS(VMID_RLEG_CTRL_RLEG_IP6_UC_ENA, tmp)); // IP6_UC
        pr("%s    ", ENA_DIS(VMID_RLEG_CTRL_RLEG_IP4_MC_ENA, tmp)); // IP4_MC
        pr("%s            ", // IP4_ICMP_REDIR
                ENA_DIS(VMID_RLEG_CTRL_RLEG_IP4_ICMP_REDIR_ENA, tmp));
        pr("%s       ", ENA_DIS(VMID_RLEG_CTRL_RLEG_IP4_UC_ENA, tmp)); // IP4_UC

        JR_RDXY(ANA_L3_2_VMID, VRRP_CFG, row, 0, &tmp);
        pr("%3u    ", VTSS_X_ANA_L3_2_VMID_VRRP_CFG_RLEG_VRID(tmp)); // VRID-0

        JR_RDXY(ANA_L3_2_VMID, VRRP_CFG, row, 1, &tmp);
        pr("%3u", VTSS_X_ANA_L3_2_VMID_VRRP_CFG_RLEG_VRID(tmp)); // VRID-1

        pr("\n");
    }
    pr(" \n");


    pr("Routing table\n");
    pr("=============\n");
    pr("                LPM TABLE                             |             ARP TABLE                   \n");
    pr("row  match    type    data       mask        eq  Ptr  | EVMID  secure secure arp    mac         \n");
    pr("                                                      |        vmid   mac    enable address     \n");
    pr("------------------------------------------------------|-----------------------------------------\n");
    // "0000 SECURITY INVALID 0x00000000 NO-READBACK Yes 0000 | 0000   No     No     Yes    000000000000

    for( row = 0; row < VTSS_JR1_LPM_CNT; ++row ) {
        // just assume that this is IPv4
        JR_WR(ANA_L3_2_LPM,
              ACCESS_CTRL,
              VTSS_F_ANA_L3_2_LPM_ACCESS_CTRL_ACCESS_IDX(row) |
              VTSS_F_ANA_L3_2_LPM_ACCESS_CTRL_ACCESS_WID(
                  VTSS_LPM_ACCESS_CTRL_WID_QUAD) |
              VTSS_F_ANA_L3_2_LPM_ACCESS_CTRL_ACCESS_CMD(
                  VTSS_LPM_ACCESS_CTRL_CMD_READ) |
              VTSS_F_ANA_L3_2_LPM_ACCESS_CTRL_ACCESS_SHOT
        );

        VTSS_RC(jr_lpm_table_idle(vtss_state));

        JR_RDX(ANA_L3_2_LPM, LPM_USAGE_CFG, 0, &conf);

        // don't print invalid entries
        if( VTSS_F_ANA_L3_2_LPM_LPM_USAGE_CFG_LPM_TYPE(conf) ==
                VTSS_LPM_TYPE_INVALID)
            continue;

        JR_RDX(ANA_L3_2_LPM, LPM_DATA_CFG, 0, &data);
        JR_RDX(ANA_L3_2_LPM, LPM_DATA_CFG, 0, &mask);
        JR_RD(ANA_L3_2_REMAP, REMAP_CFG, &remap);

        // print row
        pr("%4u ", row);

        // print match
        if( VTSS_F_ANA_L3_2_LPM_LPM_USAGE_CFG_LPM_DST_FLAG_MASK & conf ) {
            pr("BOTH     ");
        } else {
            if( VTSS_F_ANA_L3_2_LPM_LPM_USAGE_CFG_LPM_DST_FLAG_DATA & conf )
                pr("LPM      ");
            else
                pr("SECURITY ");
        }

        // print type
        switch (VTSS_F_ANA_L3_2_LPM_LPM_USAGE_CFG_LPM_TYPE(conf)) {
            case VTSS_LPM_TYPE_INVALID:
                pr("INVALID ");
                break;
            case VTSS_LPM_TYPE_IPV4_UC:
                pr("IPv4 UC ");
                break;
            case VTSS_LPM_TYPE_IPV4_MC:
                // Re-read as we before assumed this was IPv4uc
                JR_WR(ANA_L3_2_LPM,
                      ACCESS_CTRL,
                      VTSS_F_ANA_L3_2_LPM_ACCESS_CTRL_ACCESS_IDX(row) |
                      VTSS_F_ANA_L3_2_LPM_ACCESS_CTRL_ACCESS_WID(
                          VTSS_LPM_ACCESS_CTRL_WID_HALF) |
                      VTSS_F_ANA_L3_2_LPM_ACCESS_CTRL_ACCESS_CMD(
                          VTSS_LPM_ACCESS_CTRL_CMD_READ) |
                      VTSS_F_ANA_L3_2_LPM_ACCESS_CTRL_ACCESS_SHOT
                );
                row += 1;
                VTSS_RC(jr_lpm_table_idle(vtss_state));
                pr("IPv4 MC ");
                break;
            case VTSS_LPM_TYPE_IPV6_UC:
                // Re-read as we before assumed this was IPv4uc
                JR_WR(ANA_L3_2_LPM,
                      ACCESS_CTRL,
                      VTSS_F_ANA_L3_2_LPM_ACCESS_CTRL_ACCESS_IDX(row) |
                      VTSS_F_ANA_L3_2_LPM_ACCESS_CTRL_ACCESS_WID(
                          VTSS_LPM_ACCESS_CTRL_WID_FULL) |
                      VTSS_F_ANA_L3_2_LPM_ACCESS_CTRL_ACCESS_CMD(
                          VTSS_LPM_ACCESS_CTRL_CMD_READ) |
                      VTSS_F_ANA_L3_2_LPM_ACCESS_CTRL_ACCESS_SHOT
                );
                row += 3;
                VTSS_RC(jr_lpm_table_idle(vtss_state));
                pr("IPv6 UC ");
                break;
            default:
                pr("UNKNOWN ");
        };

        // print data and mask
        pr("0x%08x NO-READBACK ", data);
        pr("%s ", ENA_DIS(REMAP_REMAP_CFG_ECMP_CNT, remap));

        // print pointer
        pr("%4u | ", VTSS_X_ANA_L3_2_REMAP_REMAP_CFG_BASE_PTR(remap));

        // Fetch ARP entry
        JR_RDX(ANA_L3_2_ARP, ARP_CFG_0,
                VTSS_X_ANA_L3_2_REMAP_REMAP_CFG_BASE_PTR(remap), &arp0);
        JR_RDX(ANA_L3_2_ARP, ARP_CFG_1,
                VTSS_X_ANA_L3_2_REMAP_REMAP_CFG_BASE_PTR(remap), &arp1);

        pr("%4u   ", VTSS_X_ANA_L3_2_ARP_ARP_CFG_0_ARP_VMID(arp0));
        pr("%s    ", ENA_DIS(ARP_ARP_CFG_0_SECUR_MATCH_VMID_ENA, arp0));
        pr("%s    ", ENA_DIS(ARP_ARP_CFG_0_SECUR_MATCH_MAC_ENA, arp0));
        pr("%s    ", ENA_DIS(ARP_ARP_CFG_0_ARP_ENA, arp0));
        pr("%04x%08x",VTSS_X_ANA_L3_2_ARP_ARP_CFG_0_MAC_MSB(arp0), arp1);

        // fetch and display multi row entries
        switch (VTSS_F_ANA_L3_2_LPM_LPM_USAGE_CFG_LPM_TYPE(conf)) {
            case VTSS_LPM_TYPE_IPV6_UC:
                JR_RDX(ANA_L3_2_LPM, LPM_DATA_CFG, 1, &data);
                pr("\n                      0x%08x NO-READBACK", data);
                JR_RDX(ANA_L3_2_LPM, LPM_DATA_CFG, 2, &data);
                pr("\n                      0x%08x NO-READBACK", data);
                JR_RDX(ANA_L3_2_LPM, LPM_DATA_CFG, 3, &data);
                pr("\n                      0x%08x NO-READBACK", data);
                break;

            case VTSS_LPM_TYPE_IPV4_MC:
                JR_RDX(ANA_L3_2_LPM, LPM_DATA_CFG, 1, &data);
                pr("\n                      0x%08x NO-READBACK", data);
                break;

            default:
                ;
        };

        pr("\n");
    }
    pr("\n");


    pr("ARP table\n");
    pr("=========\n");
    pr("row   EVMID  secure secure arp    mac         \n");
    pr("             vmid   mac    enable address     \n");
    pr("----------------------------------------------\n");
    // "0000  0000   No     No     Yes    000000000000
    for (row = 0; row < VTSS_JR1_ARP_CNT; ++row) {
        // Fetch ARP entry
        JR_RDX(ANA_L3_2_ARP, ARP_CFG_0,
                VTSS_X_ANA_L3_2_REMAP_REMAP_CFG_BASE_PTR(row), &arp0);
        JR_RDX(ANA_L3_2_ARP, ARP_CFG_1,
                VTSS_X_ANA_L3_2_REMAP_REMAP_CFG_BASE_PTR(row), &arp1);

        if ((!FLAG(ARP_ARP_CFG_0_SECUR_MATCH_VMID_ENA, arp0)) &&
            (!FLAG(ARP_ARP_CFG_0_SECUR_MATCH_MAC_ENA, arp0)) &&
            (!FLAG(ARP_ARP_CFG_0_ARP_ENA, arp0))) {
            continue;
        }

        pr("%4u ", row);
        pr("%4u   ", VTSS_X_ANA_L3_2_ARP_ARP_CFG_0_ARP_VMID(arp0));
        pr("%s    ", ENA_DIS(ARP_ARP_CFG_0_SECUR_MATCH_VMID_ENA, arp0));
        pr("%s    ", ENA_DIS(ARP_ARP_CFG_0_SECUR_MATCH_MAC_ENA, arp0));
        pr("%s    ", ENA_DIS(ARP_ARP_CFG_0_ARP_ENA, arp0));
        pr("%04x%08x",VTSS_X_ANA_L3_2_ARP_ARP_CFG_0_MAC_MSB(arp0), arp1);
        pr("\n");
    }
    pr("\n");
    pr("\n");


    // dump sticky bits
    pr("Debug sticky bits\n");
    pr("=================\n");
    RAW_REG(tmp, ANA_L3_2, LPM_REMAP_STICKY_L3_LPM_REMAP_STICKY);
    REG_BIT(tmp, ANA_L3_2, LPM_REMAP_STICKY_L3_LPM_REMAP_STICKY, SECUR_IP6_LPM_FOUND_STICKY);
    REG_BIT(tmp, ANA_L3_2, LPM_REMAP_STICKY_L3_LPM_REMAP_STICKY, SECUR_IP4_LPM_FOUND_STICKY);
    REG_BIT(tmp, ANA_L3_2, LPM_REMAP_STICKY_L3_LPM_REMAP_STICKY, LPM_IP6UC_FOUND_STICKY);
    REG_BIT(tmp, ANA_L3_2, LPM_REMAP_STICKY_L3_LPM_REMAP_STICKY, LPM_IP4UC_FOUND_STICKY);
    REG_BIT(tmp, ANA_L3_2, LPM_REMAP_STICKY_L3_LPM_REMAP_STICKY, LPM_IP4MC_FOUND_STICKY);
    pr("\n");
    RAW_REG(tmp, ANA_L3_2, VLAN_ARP_L3MC_STICKY_VLAN_STICKY);
    REG_BIT(tmp, ANA_L3_2, VLAN_ARP_L3MC_STICKY_VLAN_STICKY, BMSTP_FWD_ALLOWED_STICKY );
    REG_BIT(tmp, ANA_L3_2, VLAN_ARP_L3MC_STICKY_VLAN_STICKY, BMSTP_DISCARD_STICKY);
    REG_BIT(tmp, ANA_L3_2, VLAN_ARP_L3MC_STICKY_VLAN_STICKY, BVLAN_LRN_DENY_STICKY);
    REG_BIT(tmp, ANA_L3_2, VLAN_ARP_L3MC_STICKY_VLAN_STICKY, BMSTP_LRN_DENY_STICKY);
    REG_BIT(tmp, ANA_L3_2, VLAN_ARP_L3MC_STICKY_VLAN_STICKY, BMSTP_LRN_ALLOWED_STICKY);
    REG_BIT(tmp, ANA_L3_2, VLAN_ARP_L3MC_STICKY_VLAN_STICKY, BVLAN_LOOKUP_INVLD_STICKY);
    REG_BIT(tmp, ANA_L3_2, VLAN_ARP_L3MC_STICKY_VLAN_STICKY, BVLAN_IGR_FILTER_STICKY);
    REG_BIT(tmp, ANA_L3_2, VLAN_ARP_L3MC_STICKY_VLAN_STICKY, MSTP_FWD_ALLOWED_STICKY);
    REG_BIT(tmp, ANA_L3_2, VLAN_ARP_L3MC_STICKY_VLAN_STICKY, MSTP_DISCARD_STICKY);
    REG_BIT(tmp, ANA_L3_2, VLAN_ARP_L3MC_STICKY_VLAN_STICKY, VLAN_LRN_DENY_STICKY);
    REG_BIT(tmp, ANA_L3_2, VLAN_ARP_L3MC_STICKY_VLAN_STICKY, MSTP_LRN_DENY_STICKY);
    REG_BIT(tmp, ANA_L3_2, VLAN_ARP_L3MC_STICKY_VLAN_STICKY, MSTP_LRN_ALLOWED_STICKY);
    REG_BIT(tmp, ANA_L3_2, VLAN_ARP_L3MC_STICKY_VLAN_STICKY, VLAN_LOOKUP_INVLD_STICKY);
    REG_BIT(tmp, ANA_L3_2, VLAN_ARP_L3MC_STICKY_VLAN_STICKY, VLAN_IGR_FILTER_STICKY);

    RAW_REG(tmp, ANA_L3_2, VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY);
    REG_BIT(tmp, ANA_L3_2, VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY, NO_MC_VMID_AVAIL_STICKY);
    REG_BIT(tmp, ANA_L3_2, VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY, MC_ENTRY_NOT_FOUND_STICKY);
    REG_BIT(tmp, ANA_L3_2, VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY, ENTRY_NOT_FOUND_STICKY);
    REG_BIT(tmp, ANA_L3_2, VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY, SECUR_DIP_FAIL_STICKY);
    REG_BIT(tmp, ANA_L3_2, VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY, MC_LOOPED_CP_STICKY);
    REG_BIT(tmp, ANA_L3_2, VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY, NO_MC_FWD_STICKY);
    REG_BIT(tmp, ANA_L3_2, VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY, L3_MC_FWD_STICKY);
    REG_BIT(tmp, ANA_L3_2, VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY, MC_RPF_FILTER_STICKY);
    REG_BIT(tmp, ANA_L3_2, VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY, L2_MC_FWD_STICKY);
    REG_BIT(tmp, ANA_L3_2, VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY, UC_ICMP_REDIR_STICKY);
    REG_BIT(tmp, ANA_L3_2, VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY, UC_ZERO_DMAC_FOUND_STICKY);
    REG_BIT(tmp, ANA_L3_2, VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY, UC_TTL_FILTERING_STICKY);
    REG_BIT(tmp, ANA_L3_2, VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY, UC_ENTRY_FOUND_STICKY);
    REG_BIT(tmp, ANA_L3_2, VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY, RLEG_MC_HIT_STICKY);
    REG_BIT(tmp, ANA_L3_2, VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY, RLEG_MC_TTL_STICKY);
    REG_BIT(tmp, ANA_L3_2, VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY, RLEG_MC_IP_OPT_REDIR_STICKY);
    REG_BIT(tmp, ANA_L3_2, VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY, SECUR_IP4_SIP_MATCH_STICKY);
    REG_BIT(tmp, ANA_L3_2, VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY, SECUR_IP6_DIP_MATCH_STICKY);
    REG_BIT(tmp, ANA_L3_2, VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY, SECUR_IP4_DIP_MATCH_STICKY);
    REG_BIT(tmp, ANA_L3_2, VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY, RLEG_MC_HDR_ERR_REDIR_STICKY);
    REG_BIT(tmp, ANA_L3_2, VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY, SECUR_SIP_FAIL_STICKY);
    REG_BIT(tmp, ANA_L3_2, VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY, SECUR_IP6_SIP_MATCH_STICKY);
    REG_BIT(tmp, ANA_L3_2, VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY, RLEG_UC_HIT_STICKY);
    REG_BIT(tmp, ANA_L3_2, VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY, RLEG_UC_IP_OPT_REDIR_STICKY);
    REG_BIT(tmp, ANA_L3_2, VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY, RLEG_UC_HDR_ERR_REDIR_STICKY);
    REG_BIT(tmp, ANA_L3_2, VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY, RLEG_UC_NONIP_REDIR_STICKY);

#undef ENA_DIS
#undef RAW_REG_DUMP
#undef REG_BIT_DESC
#undef REG_BIT
#undef MAC_FORMAT
#undef MAC_ARGS
#undef FLAG

    return VTSS_RC_OK;
}

vtss_rc vtss_jr1_l3_debug_print(vtss_state_t *vtss_state,
                                const vtss_debug_printf_t pr,
                                const vtss_debug_info_t   *const info)
{
    return vtss_debug_print_group(VTSS_DEBUG_GROUP_L3, jr_debug_routing, vtss_state, pr, info);
}

/* - Initialization ------------------------------------------------ */

vtss_rc vtss_jr1_l3_init(vtss_state_t *vtss_state, vtss_init_cmd_t cmd)
{
    vtss_l3_state_t *state = &vtss_state->l3;
    vtss_rc         rc = VTSS_RC_OK;
    
    switch (cmd) {
    case VTSS_INIT_CMD_CREATE:
        state->common_set = jr_l3_common_set;
        state->rleg_counters_get = jr_l3_rleg_hw_stat_poll;
        state->rleg_counters_reset = jr_l3_rleg_stat_reset;
        state->rleg_set = jr_l3_rleg_set;
        state->vlan_set = jr_l3_vlan_set;
        state->arp_set = jr_l3_arp_set;
        state->arp_clear = jr_l3_arp_clear;
        state->ipv4_uc_set = jr_l3_ipv4_uc_set;
        state->ipv6_uc_set = jr_l3_ipv6_uc_set;
        state->ipv4_uc_clear = jr_l3_ipv4_uc_clear;
        state->ipv4_mc_clear = jr_l3_ipv4_mc_clear;
        state->ipv6_uc_clear = jr_l3_ipv6_uc_clear;
        state->lpm_move = jr_l3_lpm_move;
        state->debug_sticky_clear = jr_l3_debug_sticky_clear;
        break;
    case VTSS_INIT_CMD_INIT:
        rc = jr_init_l3_chip(vtss_state);
        break;
    case VTSS_INIT_CMD_POLL:
#if defined(VTSS_SW_OPTION_L3RT)
        rc = jr_l3_rleg_stat_poll(vtss_state);
#endif /* VTSS_SW_OPTION_L3RT */
        break;
    default:
        break;
    }
    return rc;
}

#endif /* VTSS_ARCH_JAGUAR_1 */
