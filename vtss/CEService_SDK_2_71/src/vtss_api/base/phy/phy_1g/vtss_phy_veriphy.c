/*

 Vitesse PHY API software.

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

// Avoid "vtss_options.h not used in module vtss_phy_veriphy.c"
/*lint --e{766} */

#include "vtss_options.h"

#ifdef VTSS_CHIP_CU_PHY
#if defined(VTSS_PHY_OPT_VERIPHY)

#define VTSS_TRACE_GROUP VTSS_TRACE_GROUP_PHY

#include "vtss_api.h"
#include "../../ail/vtss_state.h"
#include "vtss_phy.h"

#define SmiWrite(vtss_state, phy, reg, val) vtss_phy_wr(vtss_state, phy, reg, val)

/*- ExtMiiWrite is not applicable to Mustang VSC8201 PHY chips */
#define ExtMiiWrite(vtss_state, phy, reg, val) (void) SmiWrite(vtss_state, phy, 31, VTSS_PHY_PAGE_EXTENDED); (void) SmiWrite(vtss_state, phy, reg, val)

#define VTSS_PHY_1_GEN_DSP(port_no) ((port_family(vtss_state, port_no) == VTSS_PHY_FAMILY_MUSTANG) || \
                                     (port_family(vtss_state, port_no) == VTSS_PHY_FAMILY_COBRA))


static int SmiRead(vtss_state_t *vtss_state, int port_no, int reg)
{
    u16 rv;
    (void) vtss_phy_rd(vtss_state, port_no, reg, &rv);
    return rv;
}

/* Local functions */
static int ExtMiiReadBits(vtss_state_t *vtss_state,
                          const vtss_port_no_t port, char reg, char msb, char lsb)
{
    int x;

    VTSS_RC(vtss_phy_page_ext(vtss_state, port));
    x = SmiRead(vtss_state, port, reg);
    if (msb < 15) {
        x &= (1 << (msb + 1)) - 1;
    }
    x = (int)((unsigned int) x >> lsb);

    return x;
}

static int MiiReadBits(vtss_state_t *vtss_state,
                       const vtss_port_no_t port, char reg, char msb, char lsb)
{
    int x;

    (void) vtss_phy_page_std(vtss_state, port);
    x = SmiRead(vtss_state, port, reg);
    if (msb < 15) {
        x &= (1 << (msb + 1)) - 1;
    }
    x = (int)((unsigned int) x >> lsb);

    return x;
}

static void MiiWriteBits(vtss_state_t *vtss_state,
                         const vtss_port_no_t port, char reg, char msb, char lsb, i8 value)
{
    int x;
    unsigned int t = 1, mask;

    (void)vtss_phy_page_std(vtss_state, port);
    x = SmiRead(vtss_state, port, reg);
    mask = (((t << (msb - lsb + 1)) - t) << lsb);
    x = ((value << lsb) & mask) | (x & (~mask));
    (void) SmiWrite(vtss_state, port, reg, x);
}

static int TpReadBits(vtss_state_t *vtss_state,
                      const vtss_port_no_t port, char reg, char msb, char lsb)
{
    int x;

    (void) vtss_phy_page_test(vtss_state, port);
    x = SmiRead(vtss_state, port, reg);
    if (msb < 15) {
        x &= (1 << (msb + 1)) - 1;
    }
    x = (int)((unsigned int) x >> lsb);

    return x;
}

static void TpWriteBit(vtss_state_t *vtss_state,
                       const vtss_port_no_t port, i8 TpReg, i8 bitNum, i8 value)
{
    short val;

    (void) vtss_phy_page_test(vtss_state, port);
    val = SmiRead(vtss_state, port, TpReg);
    if (value) {
        val = val | (1 << bitNum);
    } else {
        val = val & ~(1 << bitNum);
    }
    (void) vtss_phy_wr(vtss_state, port, TpReg, val);
}

/* PHY family */
static vtss_phy_family_t port_family(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
    return vtss_state->phy_state[port_no].family;
}

static vtss_rc tr_raw_read(vtss_state_t *vtss_state,
                           const vtss_port_no_t port, const u16 TrSubchanNodeAddr, u32 *tr_raw_data)
{
    u16 x;
    vtss_mtimer_t timer;
    u32 base;

    /* Determine base address */
    /* base = (port_family(port) == VTSS_PHY_FAMILY_MUSTANG ? 0 : 16); */
    base = VTSS_PHY_1_GEN_DSP(port) ? 0 : 16;
    VTSS_RC(vtss_phy_wr(vtss_state, port, base, (5 << 13) | TrSubchanNodeAddr));
    VTSS_MTIMER_START(&timer, 500);

    while (1) {
        VTSS_RC(vtss_phy_rd(vtss_state, port, base, &x));
        if (x & 0x8000) {
            break;
        }
        if (VTSS_MTIMER_TIMEOUT(&timer)) {
            VTSS_MTIMER_CANCEL(&timer);
            return VTSS_RC_ERROR; /*- should not happen */
        }
    }
    VTSS_MTIMER_CANCEL(&timer);

    VTSS_RC(vtss_phy_rd(vtss_state, port, base + 2, &x)); /*- high part */
    *tr_raw_data = (u32) x << 16;
    VTSS_RC(vtss_phy_rd(vtss_state, port, base + 1, &x)); /*- low part */
    *tr_raw_data |= x;

    return VTSS_RC_OK;
}

static vtss_rc tr_raw_write(vtss_state_t *vtss_state,
                            u8 port_no, u16 ctrl_word, u32 val)
{
    u32 base;

    /* Determine base address */
    /* base = (port_family(port_no) == VTSS_PHY_FAMILY_MUSTANG ? 0 : 16); */
    base = VTSS_PHY_1_GEN_DSP(port_no) ? 0 : 16;

    VTSS_RC(vtss_phy_wr(vtss_state, port_no, base + 2, val >> 16));   /*- high part */
    VTSS_RC(vtss_phy_wr(vtss_state, port_no, base + 1, (u16) val));  /*- low part */
    VTSS_RC(vtss_phy_wr(vtss_state, port_no, base, (4 << 13) | ctrl_word));

    return VTSS_RC_OK;
}

/*- Function issues a "long read" command to read Token Ring Registers */
/*- in the VSC8201 Mustang PHY. Results appear in Token Ring Registers 1-20 -*/
static vtss_rc tr_raw_long_read(vtss_state_t *vtss_state,
                                u8 subchan_phy, const u16 TrSubchanNode)
{
    u16 x, phy_num;
    vtss_mtimer_t timer;

    phy_num = subchan_phy & 0x3f;
    VTSS_RC(vtss_phy_page_tr(vtss_state, phy_num));
    VTSS_RC(vtss_phy_wr(vtss_state, phy_num, 0, TrSubchanNode));

    VTSS_MTIMER_START(&timer, 500);
    while (1) {
        VTSS_RC(vtss_phy_rd(vtss_state, phy_num, 0, &x));
        if (x & 0x8000) {
            break;
        }
        if (VTSS_MTIMER_TIMEOUT(&timer)) {
            VTSS_MTIMER_CANCEL(&timer);
            return VTSS_RC_ERROR; /*- should not happen */
        }
    }
    VTSS_MTIMER_CANCEL(&timer);
    return VTSS_RC_OK;
}

/*- Processes the Result from reading an EC debug node */
static vtss_rc process_ec_result(vtss_state_t *vtss_state,
                                 u8 subchan_phy, u16 tap, u32 *tr_raw_data)
{
    u16 reg_beg, reg_end, reg_beg_msb, reg_end_lsb;
    u16 x, y, phy_num;
    u32 ret;
    u16 firstbit, bitwidth;
    phy_num = subchan_phy & 0x3f;

    firstbit = (subchan_phy >> 6) * 68;
    bitwidth = 15;

    if (tap < 16) {
        bitwidth = 20;
    } else {
        firstbit += 20;
        if (tap < 32) {
            bitwidth = 18;
        } else {
            firstbit += 18;
            if (tap >= 48) {
                firstbit += 15;
            }
        }
    }
    reg_beg = 20 - (firstbit >> 4);
    reg_beg_msb = 15 - (firstbit & 0xf);
    reg_end = 20 - ((firstbit + bitwidth - 1) >> 4);
    reg_end_lsb = 15 - ((firstbit + bitwidth - 1) & 0xf);

    if (reg_beg == reg_end) {
        VTSS_RC(vtss_phy_rd(vtss_state, phy_num, reg_beg, &x));
        x &= (1 << (reg_beg_msb + 1)) - 1;
        ret = x >> reg_end_lsb;
    } else if (reg_beg == (reg_end + 1)) {
        VTSS_RC(vtss_phy_rd(vtss_state, phy_num, reg_beg, &x));
        x &= (1 << (reg_beg_msb + 1)) - 1;
        ret = (u32)x << (16 - reg_end_lsb);
        VTSS_RC(vtss_phy_rd(vtss_state, phy_num, reg_end, &y));
        y >>= reg_end_lsb;
        ret += y;
    } else {
        /*- This case cannot happen -*/
        return -1;
    }
    if (ret >= (1L << (bitwidth - 1))) {
        ret -= (1L << bitwidth);
    }

    *tr_raw_data = ret;

    return VTSS_RC_OK;
}

/*- Processes the result from reading an ECVar/NC debug node */
static vtss_rc process_var_result(vtss_state_t *vtss_state,
                                  u8 subchan_phy, u16 flt, u32 *tr_raw_data)
{
    u16 reg_beg, reg_end, reg_beg_msb, reg_end_lsb;
    u16 x, y, phy_num, sub_chan, scl = 0;
    u16 firstbit, bitwidth;

    reg_beg = reg_end = reg_beg_msb = reg_end_lsb = x = 0;
    phy_num = subchan_phy & 0x3f;
    sub_chan =     subchan_phy >> 6;
    bitwidth = 15;

    switch (flt) {
    case 0:        /*- NC1 -*/
    case 1:        /*- NC2 -*/
    case 2:        /*- NC3 -*/
        scl = (sub_chan << 2) - sub_chan + flt;
        break;
    case 3:        /*- ECVar -*/
        scl = sub_chan + (((flt + 1) << 2) - (flt + 1));
        break;
    default:    /*- will never get here -*/
        break;
    }
    firstbit = (scl << 4) - scl;

    reg_beg = 20 - (firstbit >> 4);
    reg_beg_msb = 15 - (firstbit & 0xf);
    reg_end = 20 - ((firstbit + bitwidth - 1) >> 4);
    reg_end_lsb = 15 - ((firstbit + bitwidth - 1) & 0xf);

    if (reg_beg == reg_end) {
        VTSS_RC(vtss_phy_rd(vtss_state, phy_num, reg_beg, &x));
        x &= (u16)((1L << (reg_beg_msb + 1)) - 1);
        x >>= reg_end_lsb;
    } else if (reg_beg == (reg_end + 1)) {
        VTSS_RC(vtss_phy_rd(vtss_state, phy_num, reg_beg, &x));
        x &= (u16)((1L << (reg_beg_msb + 1)) - 1);
        x <<= 16 - reg_end_lsb;
        VTSS_RC(vtss_phy_rd(vtss_state, phy_num, reg_end, &y));
        y >>= reg_end_lsb;
        x += y;
    } else {
        /*- This case cannot happen -*/
        return -1;
    }
    if (x >= (1 << (bitwidth - 1))) {
        x -= (1 << bitwidth);
    }

    *tr_raw_data = (u32)((long)((short)x));

    return VTSS_RC_OK;
}
static vtss_rc vtss_phy_1_gen_pre_veriphy(vtss_state_t *vtss_state,
                                          vtss_phy_family_t family, vtss_veriphy_task_t c51_idata *tsk)
{
    u32 x;
    BOOL conf_none;

    tsk->saveReg = 0;
    tsk->tokenReg = 0;

    VTSS_D("family = %d\n", family);
    if (family == VTSS_PHY_FAMILY_MUSTANG) {
        /*- MII Registers 0, 9, 28 saved and modified */
        /*- TP[5.4:3], MII [28.2], MII[9.12:11], MII[0] - 21 bits */
        tsk->saveReg |= SmiRead(vtss_state, tsk->port, 0);
        tsk->saveReg |= ((MiiReadBits(vtss_state, tsk->port, 9, 12, 11)) << 16);
        tsk->saveReg |= ((MiiReadBits(vtss_state, tsk->port, 28, 2, 2)) << 18);
        MiiWriteBits(vtss_state, tsk->port, 28, 2, 2, 1);    /*- SMI Register priority select */
        tsk->saveReg |= ((TpReadBits(vtss_state, tsk->port, 5, 4, 3)) << 19);
    } else { /* family == VTSS_PHY_FAMILY_COBRA */
        /*- MII Register 0, 4, 9 saved */
        /*- MII [23.15:12], MII [23.3:2], MII [9.12:8], MII [4.11:5], MII [0.14:6]  - 27 bits */
        tsk->saveReg |= (MiiReadBits(vtss_state, tsk->port, 0, 14, 6));
        tsk->saveReg |= ((MiiReadBits(vtss_state, tsk->port, 4, 11, 5)) << 9);
        tsk->saveReg |= ((MiiReadBits(vtss_state, tsk->port, 9, 12, 8)) << 16);
        tsk->saveReg |= ((MiiReadBits(vtss_state, tsk->port, 23, 2, 1)) << 21);
        tsk->saveReg |= ((MiiReadBits(vtss_state, tsk->port, 23, 15, 12)) << 23);

        VTSS_RC(vtss_phy_page_std(vtss_state, tsk->port));
        /*
        tsk->saveReg = SmiRead(vtss_state, tsk->port, 23) & 0xf006;
        tsk->saveReg = (tsk->saveReg & 0xf000) | ((tsk->saveReg & 6) << 9);
        tsk->saveReg = tsk->saveReg << 10;
        */

        VTSS_RC(vtss_phy_wr_masked(vtss_state, tsk->port, 23, 0x1004, 0xf006));
        conf_none = vtss_state->phy_state[tsk->port].conf_none;
        vtss_state->phy_state[tsk->port].conf_none = 0;
        VTSS_RC(vtss_phy_reset_private(vtss_state, tsk->port));
        vtss_state->phy_state[tsk->port].conf_none = conf_none;
        VTSS_RC(vtss_phy_wr_masked(vtss_state, tsk->port, 28, 0x0000, 0x0040));
    }
    MiiWriteBits(vtss_state, tsk->port, 9, 12, 11, 3);    /*- Forced Master */
    VTSS_RC(SmiWrite(vtss_state, tsk->port, 0, 0x0140));           /*- Force 1000 Base-T Duplex */

    /*- Test Page Registers 5, 8 saved and modified */
    VTSS_RC(vtss_phy_page_test(vtss_state, tsk->port));
    TpWriteBit(vtss_state, tsk->port, 5, 4, 1);     /*- Force MDIX */
    TpWriteBit(vtss_state, tsk->port, 5, 3, 1);

    /*- Some Token Ring Values modified are saved for Mustang only */
    /*- Token ring values are saved as follows: */
    /*- Bit [5:0] XcDecRateForce/Val, [8:6] NcUpdGainForce/Val, */
    /*- Bit [11:9] EcUpdGainForce/Val - 0x1680*/
    /*- Bit [13:12] EcLoadNew32_63Force/Val, [18:14] EcLoadNew0_15Force/Val, */
    /*- Bit [20:19] EcShiftBy16Force/Val, [22:21] EnableECvarDelayForce/Val - 0x0f86 */
    /*- Bit [24:23] EcTailClearForceVal - 0x0f82 */
    /*- Bit [28:25] VgaStartupGain - 0xfa0*/
    VTSS_RC(vtss_phy_page_tr(vtss_state, tsk->port));

    /*- Force ECdisabLast48Force/Val, DigPhaseForce/Val */
    VTSS_RC(tr_raw_read(vtss_state, tsk->port, 0xf8a /*1, 0xf, 5*/, &x));
    x = (x & ~0x1fc) | 0x108;
    VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0xf8a, x));

    /*- EcUpdGainForce/Val, NcUpdGainForce/Val, XcDecRateForce/Val */
    VTSS_RC(tr_raw_read(vtss_state, tsk->port, 0x1680 /*2, 0xd, 0*/, &x));
    if (family == VTSS_PHY_FAMILY_MUSTANG) {
        tsk->tokenReg = ((x >> 11) & 0xfff);
    }
    x = (x & ~0x3ffc00L) | 0x3ffc00L;
    VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0x1680, x));

    /*- EcLoadNew32_63Force/Val, EcLoadNew0_15Force/Val, */
    /*- EcShiftBy16Force/Val, EnableECvarDelayForce/Val */
    VTSS_RC(tr_raw_read(vtss_state, tsk->port, 0xf86 /* 1, 0xf, 3 */, &x));
    if (family == VTSS_PHY_FAMILY_MUSTANG) {
        tsk->tokenReg |= ((x >> 11) & 0x7ff) << 12;
    }
    x = (x & ~0x3ff800L) | 0x3a1000L;
    VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0xf86, x));

    /*- SlicerSelForce/Val, SignalDetectEnableForce/Val */
    VTSS_RC(tr_raw_read(vtss_state, tsk->port, 0xf88 /*01, 0xf, 4*/, &x));
    x = (x & ~0x1f00) | 0x1200;
    VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0xf88, x));

    /*- EcTailClearForce/Val */
    VTSS_RC(tr_raw_read(vtss_state, tsk->port, 0xf82 /* 1, 0xf, 1 */, &x));
    if (family == VTSS_PHY_FAMILY_MUSTANG) {
        tsk->tokenReg |= ((x >> 3) & 3) << 23;
    }
    x = (x & ~0x180) | 0x100;
    VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0xf82, x));

    /*- VgaStartupGain and set VgaAFreezeForceVal */
    VTSS_RC(tr_raw_read(vtss_state, tsk->port, 0xfa0 /*1, 0xf, 0x10*/, &x));
    if (family == VTSS_PHY_FAMILY_MUSTANG) {
        tsk->tokenReg |= ((x >> 8) & 0x1f) << 25;
    }
    x = (x & ~0x7f00) | 0x6800;
    VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0xfa0, x));

    /*- VgaBFreezeForceVal, VgaCFreezeForceVal,VgaDFreezeForceVal */
    VTSS_RC(tr_raw_read(vtss_state, tsk->port, 0xfa2 /*1, 0xf, 0x11 */, &x));
    x = (x & ~0x30303) | 0x30303;
    VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0xfa2, x));

    /*- Ffe[A-D]ForceUpdateDisab, Ffe[A-D]ForceGainswapDisab */
    VTSS_RC(tr_raw_read(vtss_state, tsk->port, 0x240 /*0, 4, 0x20 */, &x));
    x = (x & ~0xff) | 0xff;
    VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0x240, x));

    /*- DSPReadyForceVal */
    if (family == VTSS_PHY_FAMILY_MUSTANG) {
        VTSS_RC(tr_raw_read(vtss_state, tsk->port, 0x770 /*0, 0xe, 0x38*/, &x));
        x = (x & ~0x0c) | 8;
        VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0x770, x));
    } else { /* family == VTSS_PHY_FAMILY_COBRA) */
        VTSS_RC(tr_raw_read(vtss_state, tsk->port, 0x708, &x));
        x = (x & ~0xff0) | 0xaa0;
        VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0x708, x));
    }

    /* TpWriteBit(vtss_state, tsk->port, 8, 9, 1); */
    VTSS_RC(vtss_phy_page_std(vtss_state, tsk->port));
    MiiWriteBits(vtss_state, tsk->port, 0, 11, 11, 1);        /*- Power-down */
    MiiWriteBits(vtss_state, tsk->port, 0, 11, 11, 0);        /*- Power-up */

    VTSS_RC(vtss_phy_page_tr(vtss_state, tsk->port));
    /*- DSPclear[A-D}*/
    VTSS_RC(tr_raw_read(vtss_state, tsk->port, 0x1684, /* 2, 0xd, 2 */&x));
    x = (x & ~0x3fc000L) | 0x3c0000L;
    VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0x1684, x));

    /*- Save SignalDetectForceVal */
    if (family == VTSS_PHY_FAMILY_MUSTANG) {
        VTSS_RC(tr_raw_read(vtss_state, tsk->port, 0x0770 /* 0, 0x0e, 0x38 */, &x));
        x = (x & ~3) | 3;
        VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0x0770, x));
    } else { /*- family == VTSS_PHY_FAMILY_COBRA */
        VTSS_RC(tr_raw_read(vtss_state, tsk->port, 0x708, &x));
        x = (x & ~0x0c) | 0x0c;
        VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0x708, x));
    }
    return vtss_phy_page_std(vtss_state, tsk->port);
}

static vtss_rc vtss_phy_1_gen_post_veriphy(vtss_state_t *vtss_state,
                                           vtss_phy_family_t family, vtss_veriphy_task_t c51_idata *tsk)
{
    u32 x;
    BOOL conf_none;

    if (family == VTSS_PHY_FAMILY_MUSTANG) {
        /*- Token ring values are saved as follows: */
        /*- Bit [5:0] XcDecRateForce/Val, [8:6] NcUpdGainForce/Val, */
        /*- Bit [11:9] EcUpdGainForce/Val - 0x1680*/
        /*- Bit [13:12] EcLoadNew32_63Force/Val, [18:14] EcLoadNew0_15Force/Val, */
        /*- Bit [20:19] EcShiftBy16Force/Val, [22:21] EnableECvarDelayForce/Val - 0x0f86 */
        /*- Bit [24:23] EcTailClearForce/Val - 0x0f82 */
        /*- Bit [29:25] VgaStartupGain - 0xfa0*/

        VTSS_RC(vtss_phy_page_tr(vtss_state, tsk->port));
        /*- Unforce SignalDetectForce/Val, DSPReadyForce/Val */
        \
        VTSS_RC(tr_raw_read(vtss_state, tsk->port, 0x770 /*0, 0xe, 0x38 */, &x));
        x = (x & ~0xf);
        VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0x770, x));

        /*- Unforce ECdisabLast48Force/Val, DigPhaseForce/Val    */
        VTSS_RC(tr_raw_read(vtss_state, tsk->port, 0xf8a, &x));
        x = (x & ~0x1fc);
        VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0xf8a, x));

        /*- Restore EcUpdGainForce/Val, NCUpdGainForce/Val, XcDecRateForce/Val */
        VTSS_RC(tr_raw_read(vtss_state, tsk->port, 0x1680, &x));
        x = (x & ~0x3ffc00) | ((tsk->tokenReg  & 0xfff) << 10);
        VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0x1680, x));

        /*- Restore EcLoadNew32_63Force/Val, EcLoadNew0_15Force/Val, */
        /*- EcShiftBy16Force/Val, EnbleECvarDelayForce/Val */
        VTSS_RC(tr_raw_read(vtss_state, tsk->port, 0xf86, &x));
        x = (x & ~0x3ff800) | (((tsk->tokenReg >> 12) & 0x7ff) << 11);
        VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0xf86, x));

        /*- Unforce SignalDetectEnableForceVal, SlicerSelForceVal */
        VTSS_RC(tr_raw_read(vtss_state, tsk->port, 0xf88, &x));
        x = (x & ~0x1f00);
        VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0xf88, x));

        /*- Restore EcTailClearForceVal */
        VTSS_RC(tr_raw_read(vtss_state, tsk->port, 0xf82, &x));
        x = (x & ~0x18) | (((tsk->tokenReg >> 23) & 0x3) << 3);
        VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0xf82, x));
        /*- Restore original VgaStartupGain & unforce VgaAFreezeForce/Val */
        VTSS_RC(tr_raw_read(vtss_state, tsk->port, 0xfa0, &x));
        x = (x & ~0x7f00) | (((tsk->tokenReg >> 25) & 0x1f) << 8);
        VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0xfa0, x));

        /*- Unforce VgaBFreezeForceVal, VgaCFreezeForceVal, VgaDFreezeForceVal */
        VTSS_RC(tr_raw_read(vtss_state, tsk->port, 0xfa2, &x));
        x = (x & ~0x30303);
        VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0xfa2, x));

        /*- Unforce Ffe[A-D]ForceUpdateDisab, Ffe[A-D]ForceGainswapDisab */
        VTSS_RC(tr_raw_read(vtss_state, tsk->port, 0x240, &x));
        x = (x & ~0xff);
        VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0x240, x));

        /*- Unforce DSPclear[A-D] */
        VTSS_RC(tr_raw_read(vtss_state, tsk->port, 0x1684, &x));
        x = (x & ~0x3fc000L);
        VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0x1684, x));

        /*- Restore Test Page [5] and MII [28, 9 ,0] Register settings */
        TpWriteBit(vtss_state, tsk->port, 5, 4, ((tsk->saveReg >> 20) & 1));
        TpWriteBit(vtss_state, tsk->port, 5, 3, ((tsk->saveReg >> 19) & 1));
        MiiWriteBits(vtss_state, tsk->port, 9, 12, 11, ((tsk->saveReg >> 16) & 3));
        VTSS_RC(SmiWrite(vtss_state, tsk->port, 0, (tsk->saveReg & 0xffff)));
        MiiWriteBits(vtss_state, tsk->port, 28, 2, 2, ((tsk->saveReg >> 18) & 1));
        MiiWriteBits(vtss_state, tsk->port, 0, 11, 11, 1);        /*- Power-down */
        MiiWriteBits(vtss_state, tsk->port, 0, 11, 11, 0);        /*- Power-up */
    } else { /* (family == VTSS_PHY_FAMILY_COBRA) */
        u16 reg23 = 0;

        VTSS_RC(vtss_phy_page_test(vtss_state, tsk->port));
        VTSS_RC(vtss_phy_wr_masked(vtss_state, tsk->port, 8, 0, 0x0002));
        VTSS_RC(vtss_phy_page_std(vtss_state, tsk->port));
        VTSS_RC(vtss_phy_wr_masked(vtss_state, tsk->port, 22, 0, 0x0200));
        /* tsk->saveReg = ((tsk->saveReg & 0x3c00000L) >> 10) | ((tsk->saveReg & 0x0300000L) >> 19); */

        /*- MII [23.15:12], MII [23.3:2], MII [9.12:8], MII [4.11:5], MII [0.14:6]  - 27 bits */
        reg23 = ((tsk->saveReg & 0x7800000L) >> 11) | ((tsk->saveReg & 0x600000L) >> 20);
        VTSS_RC(vtss_phy_wr_masked(vtss_state, tsk->port, 23, reg23, 0xf006));

        conf_none = vtss_state->phy_state[tsk->port].conf_none;
        vtss_state->phy_state[tsk->port].conf_none = 0;
        VTSS_RC(vtss_phy_reset_private(vtss_state, tsk->port));
        vtss_state->phy_state[tsk->port].conf_none = conf_none;

        VTSS_RC(vtss_phy_wr_masked(vtss_state, tsk->port, 9, ((tsk->saveReg & 0x1f0000L) >> 8), 0x1f00));
        VTSS_RC(vtss_phy_wr_masked(vtss_state, tsk->port, 4, (((tsk->saveReg & 0xfe00L) >> 9) << 5), 0xfe0));
        VTSS_RC(vtss_phy_wr_masked(vtss_state, tsk->port, 0, ((tsk->saveReg & 0x1ffL) << 6), 0x7fc0));
    }
    return vtss_phy_page_std(vtss_state, tsk->port);
}

/*- get_anom_thresh: determines threshold as a function of tap and EC/NC */
static void get_anom_thresh(vtss_veriphy_task_t c51_idata *tsk, u8 tap)
{
    short log2ThreshX256;
    register i8 sh;

    log2ThreshX256 = 542 - (8 * (short)tap) + tsk->log2VGAx256;
    if (tsk->nc != 0) {
        log2ThreshX256 -= 500;
    }
    tsk->thresh[0] = 256 + (log2ThreshX256 & 255);
    sh = (i8)(log2ThreshX256 >> 8);
    if (sh >= 0) {
        tsk->thresh[0] <<= sh;
    } else {
        tsk->thresh[0] >>= -sh;
        if (tsk->thresh[0] < 23) {
            tsk->thresh[0] = 23;
        }
    }

    if (tsk->flags & 1) {
        tsk->thresh[1]  = tsk->thresh[0] >> 1;    /*- anomaly = 1/2 open/short when link up   */
    } else {
        u8 idx;

        tsk->thresh[1] = 0;
        for (idx = 0; idx < 5; ++idx) { /*- anomaly = 1/3 open/short when link down */
            tsk->thresh[1] = (tsk->thresh[1] + tsk->thresh[0] + 2) >> 2;
        }
    }
    if (tsk->thresh[1] < 15) {
        tsk->thresh[1] = 15;
    }

    /*- Limit anomaly threshold to 12 lsbs less than tap range for each EC & NC tap */
    if ((tsk->thresh[1] > 1012) && ((tsk->nc > 0) || (tap >= 32))) {
        tsk->thresh[1] = 1012;
    } else if ((tsk->thresh[1] > 4084) && (tap >= 16)) {
        tsk->thresh[1] = 4084;
    } else if (tsk->thresh[1] > 8180) {
        tsk->thresh[1] = 8180;
    }
    VTSS_D("get_anom_thresh, thresh[0]=%d, thresh[1]=%d\n", tsk->thresh[0], tsk->thresh[1]);
}

/*- readAvgECNCECVar: reads averaged echo/NEXT-canceller coefficients */
/*- numCoeffs must be less than or equal to 12 */
/*- rpt can be 1, 2, 3, 4, or 8! */
static short c51_idata *readAvgECNCECVar(vtss_state_t *vtss_state,
                                         u8 subchan_phy, u8 first, u8 rpt_numCoeffs)
{
    VTSS_D("readAvgECNCECVar(%d%c, %d #%d, rpt %d, discard=%d)\n", subchan_phy & 0x3f, ('A' + (subchan_phy >> 6)), first, \
           rpt_numCoeffs & 15, ((rpt_numCoeffs >> 4) & 7) + 1, (rpt_numCoeffs & DISCARD) ? 1 : 0);

    /* if (port_family(subchan_phy & 0x3f) == VTSS_PHY_FAMILY_MUSTANG) { */
    if (VTSS_PHY_1_GEN_DSP(subchan_phy & 0x3f)) {
        long c;
        u8 i, j, discrd_flg = 0, flt, numRpt = 0;
        i8 preScale;
        u16 TrSubchanNode = 0, SubchanNode = 0;
        u32 tr_raw_data = 0;

        VTSS_N("readAvgECNCECVar(%d%c, %d #%d, %d) returns ", subchan_phy & 0x3f, ('A' + (subchan_phy >> 6)), first, rpt_numCoeffs & 7, rpt_numCoeffs >> 3);
        vtss_state->phy_inst_state.maxAbsCoeff = c = 0;
        numRpt = ((rpt_numCoeffs >> 4) & 7) + 1;

        if (rpt_numCoeffs & DISCARD) {
            discrd_flg = 1;
            rpt_numCoeffs &= ~DISCARD;
        }

        if (!discrd_flg) {
            /*- Clear coefficients to be averaged */
            for (j = 0; j < 12; ++j) {
                vtss_state->phy_inst_state.coeff[j] = 0;
            }
        }

        /*- Figure out whether we are dealing with EC, ECVar or NCn -*/
        if (first & 0x80) {
            flt = first >> 6;
            if (flt == 2) {    /*- EC -*/
                SubchanNode = 7 << 13;
                VTSS_D("ECHO CANCELER, Sub-channel=%d, first tap=%d\n", (subchan_phy >> 6), (first & 0x3f));
            } else if (flt == 3) { /* ECVar */
                SubchanNode = 0x1c1 << 7;
                VTSS_D("ECVar, Sub-channel=%d, first tap=%d\n", (subchan_phy >> 6), (first & 0x3f));
            }
        } else { /*- NC -*/
            flt = first >> 5;
            SubchanNode = 0x1c1 << 7;
            VTSS_D("NC%d, Sub-channel=%d, first tap=%d\n", (flt + 1), (subchan_phy >> 6), (first & 0x1f));
        }

        /*- Compute variable prescale factor -*/
        preScale = 0;
        if ((first & 0x80) && (flt == 2)) {        /*- EC -*/
            if ((first & 0x3f) < 16) {
                if (numRpt > 4) {
                    preScale = 3;
                } else if (numRpt > 2) {
                    preScale = 2;
                } else if (numRpt > 1) {
                    preScale = 1;
                }
            } else if ((first & 0x3f) < 32) {
                if (numRpt > 4 ) {
                    preScale = 1;
                }
            }
        }

        /*- Accumulate coefficients -*/
        for (i = 0; i < numRpt; ++i) {
            for (j = 0; j < (rpt_numCoeffs & 0x0f); ++j) {
                if ((first & 0x80) && (flt == 2)) {    /*- EC -*/
                    TrSubchanNode = SubchanNode | ((32 + ((first + j) & 0xf)) << 1);
                    (void) tr_raw_long_read(vtss_state, subchan_phy, TrSubchanNode);
                    (void) process_ec_result(vtss_state, subchan_phy, (first & 0x3f) + j, &tr_raw_data);
                } else {    /*- NC and ECVar -*/
                    TrSubchanNode = SubchanNode | ((32 + ((first & 0x1f) + j)) << 1);
                    (void) tr_raw_long_read(vtss_state, subchan_phy, TrSubchanNode);
                    (void) process_var_result(vtss_state, subchan_phy, flt, &tr_raw_data);
                }

                /*- Scale 8201 coefficients so they all fit w/in 16-bit word -*/
                c = (long)tr_raw_data;

                if (c > 0) {
                    if ((c >> 4) > vtss_state->phy_inst_state.maxAbsCoeff) {
                        vtss_state->phy_inst_state.maxAbsCoeff = c >> 4;
                    }
                } else if (-(c >> 4) > vtss_state->phy_inst_state.maxAbsCoeff) {
                    vtss_state->phy_inst_state.maxAbsCoeff = -(c >> 4);
                }

                if (!discrd_flg) {
                    /*- Must add pre-scale and post-scale as appropriate for c -*/
                    /*- Length of c is one of 16, 14, or 11 bits (after >> 4) -*/
                    vtss_state->phy_inst_state.coeff[j] += (c + (1 << (3 + preScale))) >> (4 + preScale);
                }
            }
        }

        /*- Compute Average Coefficient Values -*/
        if (!discrd_flg) {
            if (numRpt == 3) {
                for (j = 0; j <= (rpt_numCoeffs & 0xf); ++j) {
                    c = ((long)vtss_state->phy_inst_state.coeff[j]) << preScale;
                    for (i = 0; i < 4; ++i) {
                        c = (c >> 2) + ((long)vtss_state->phy_inst_state.coeff[j] << preScale);
                    }
                    vtss_state->phy_inst_state.coeff[j] = (c + 2) >> 2;
                }
            } else {      /*- Repeat == 1/2/4/8 -*/
                for (i = -preScale, j = numRpt; j > 1; ++i, j = j >> 1) {
                    ;
                }

                if (i > 0) {
                    for (j = 0; j <= (rpt_numCoeffs & 0xf ); ++j) {
                        vtss_state->phy_inst_state.coeff[j] = (vtss_state->phy_inst_state.coeff[j] + (1 << (i - 1))) >> i;
                    }
                }
            }
        }

        VTSS_D("{");
        for (j = 0; j < (rpt_numCoeffs & 0xf); ++j) {
            VTSS_D(" %d", vtss_state->phy_inst_state.coeff[j]);
        }
        VTSS_D(" }, ");
        VTSS_D("MaxAbsCoeff = %d\n", vtss_state->phy_inst_state.maxAbsCoeff);
    } else {
        /* Not Mustang family */
        i8 i, preScale = 0;
        short c;
        u8 j, numRpt;
        u32 tr_raw_data;
        BOOL discrd_flg = 0;

        VTSS_N("readAvgECNCECVar(%d%c, %d #%d, rpt %d, discard=%d)\n", subchan_phy & 0x3f, ('A' + (subchan_phy >> 6)), first, rpt_numCoeffs & 15, ((rpt_numCoeffs >> 4) & 7) + 1, (rpt_numCoeffs & DISCARD) ? 1 : 0);

        vtss_state->phy_inst_state.maxAbsCoeff = 0;
        numRpt = ((rpt_numCoeffs >> 4) & 7) + 1;

        if (rpt_numCoeffs & DISCARD) {
            preScale = -1;
            discrd_flg = 1;
            rpt_numCoeffs &= ~DISCARD;
        }

        if (!discrd_flg) {
            /*- Normal read & store coefficients -*/
            preScale = (first < 16 && numRpt >= 8) ? 1 : 0;

            /*- Clear coefficients to be averaged -*/
            for (j = 0; j < 8; ++j) {
                vtss_state->phy_inst_state.coeff[j] = 0;
            }
        }

        /*- Accumulate coefficients (pre-scaling when necessay for dynamic range) -*/
        for (i = 0; i < numRpt; ++i) {
            for (j = 0; j < (rpt_numCoeffs & 15); ++j) {
                (void) tr_raw_read(vtss_state, subchan_phy & 0x3f, ((u16)(subchan_phy & 0xc0) << 5)
                                   | ((u16)(j + first) << 1), &tr_raw_data);
                if (tr_raw_data & 0x2000 ) {
                    c = 0xc000 | (short)tr_raw_data;
                } else {
                    c = (short)tr_raw_data;
                }

                if (c > 0) {
                    if (c > vtss_state->phy_inst_state.maxAbsCoeff) {
                        vtss_state->phy_inst_state.maxAbsCoeff = c;
                    }
                } else if (-c > vtss_state->phy_inst_state.maxAbsCoeff) {
                    vtss_state->phy_inst_state.maxAbsCoeff = -c;
                }

                if (preScale >= 0) {
                    vtss_state->phy_inst_state.coeff[j] += c >> preScale;
                }
            }
        }

        if (preScale >= 0) {
            if (numRpt == 3) {
                /*- Complete averaging by scaling coefficients -*/
                for (j = 0; j < (rpt_numCoeffs & 15); ++j) {
                    c = vtss_state->phy_inst_state.coeff[j];
                    for (i = 0; i < 4; ++i) {
                        c = (c >> 2) + vtss_state->phy_inst_state.coeff[j];
                    }
                    vtss_state->phy_inst_state.coeff[j] = (c + 2) >> 2;
                }
            } else {
                /*- Determine averaging scale-factor accounting for pre-scale -*/
                for (i = 0, j = numRpt >> preScale; j > 1; ++i, j = j >> 1) {
                    ;
                }

                /*- Complete averaging by scaling coefficients -*/
                for (j = 0; j < (rpt_numCoeffs & 15); ++j) {
                    vtss_state->phy_inst_state.coeff[j] = vtss_state->phy_inst_state.coeff[j] >> i;
                }
            }
            VTSS_D("{");
            for (j = 0; j < (rpt_numCoeffs & 15); ++j) {
                VTSS_D(" %d", vtss_state->phy_inst_state.coeff[j]);
            }
            VTSS_D(" }, ");
        }
        VTSS_D("MaxAbsCoeff = %d\n", vtss_state->phy_inst_state.maxAbsCoeff);
    }
    /*- Return pointer to first coefficient that was read */
    return &vtss_state->phy_inst_state.coeff[0];
}

/*- checkValidity: checks the validity of results in case of far-end turn-on or near-end bug -*/
static BOOL checkValidity(vtss_state_t *vtss_state,
                          vtss_veriphy_task_t c51_idata *tsk, short noiseLimit)
{
    vtss_mtimer_t timer;
    i8 timeout = 0;
    vtss_phy_family_t family;

    if (tsk->flags & 1) {
        return 1;
    }

    family = port_family(vtss_state, tsk->port);

    VTSS_RC(vtss_phy_page_tr(vtss_state, tsk->port));
    if (VTSS_PHY_1_GEN_DSP(tsk->port)) {
        /* if (family == VTSS_PHY_FAMILY_MUSTANG) { */
        /*- EcVarForceDelayVal = 255 - 64, EcVarForceDelay = 1 , EcVarForceIdle = 1 then 0 */
        VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0x0b6/*0, 1, 1b*/, (((255 - 64) << 2) | 3)));
        VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0x0b6/*0, 1, 1b*/, (((255 - 64) << 2) | 2))); /*- EcVarForceIdle = 0 */
    }

    if (family == VTSS_PHY_FAMILY_QUATTRO ||
        family == VTSS_PHY_FAMILY_SPYDER ||
        family == VTSS_PHY_FAMILY_ENZO) {
        /*- EcVarForceDelayVal = 232 - 72, EcVarForceDelay = 1 */
        VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0x0188/*0, 3, 4*/, (tsk->tr_raw0188 & 0xfffe00) | (160 << 1) | 1));
        VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0x0184/*0, 3, 2*/, 0x02 << tsk->subchan)); /*- EcVar<subchan>ForceIdle = 1 */
        VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0x0184/*0, 3, 2*/, 0)); /*- EcVar<subchan>ForceIdle = 0 */
    }

    VTSS_MSLEEP(1);
    /* if (family != VTSS_PHY_FAMILY_MUSTANG) { */
    if (!VTSS_PHY_1_GEN_DSP(tsk->port)) {
        VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0x0188/*0, 3, 4*/, tsk->tr_raw0188)); /*- restore */
    }
    VTSS_MTIMER_START(&timer, 200);
    while (1) {
        if ((readAvgECNCECVar(vtss_state,
                              (tsk->subchan << 6) | tsk->port,
                              /* family == VTSS_PHY_FAMILY_MUSTANG ? 192 : 72, */
                              VTSS_PHY_1_GEN_DSP(tsk->port) ? 192 : 72,
                              0xb8), vtss_state->phy_inst_state.maxAbsCoeff) >= ((tsk->stat[(int)tsk->subchan] == 0) ? 4 : 1)) {
            break;
        }
        if (VTSS_MTIMER_TIMEOUT(&timer)) {
            timeout = 1;
            break;
        }
    }
    VTSS_MTIMER_CANCEL(&timer);
    /* if (family == VTSS_PHY_FAMILY_MUSTANG) { */
    if (VTSS_PHY_1_GEN_DSP(tsk->port)) {
        VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0x0b6/*0, 1, 1b*/, tsk->tr_raw0188 | 1)); /*- EcVarForceIdle = 1 */
        VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0x0b6/*0, 1, 1b*/, tsk->tr_raw0188 | 0)); /*- EcVarForceIdle = 0 */
    } else {
        VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0x0184/*0, 3, 2*/, 0x02 << tsk->subchan)); /*- EcVar<subchan>ForceIdle = 1 */
        VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0x0184/*0, 3, 2*/, 0)); /*- EcVar<subchan>ForceIdle = 0 */
    }

    if (timeout) {
        if (tsk->stat[(int)tsk->subchan] != 0) {  /*- Anomaly found, but invalid! */
            tsk->stat[(int)tsk->subchan]      = 0;
            tsk->loc[(int)tsk->subchan]       = 0;
            tsk->strength[(int)tsk->subchan]  = 0;
            VTSS_D("Invalid Anomaly Found");
        }
        return 0;
    } else if (vtss_state->phy_inst_state.maxAbsCoeff > noiseLimit) {
        if (tsk->stat[(int)tsk->subchan] == 0) {  /*- Length measurement invalid? */
            tsk->loc[(int)tsk->subchan] = 255;    /*- Block this subchannel's contribution */
            tsk->strength[(int)tsk->subchan] = vtss_state->phy_inst_state.maxAbsCoeff;
            VTSS_D("Length Invalid??? Block off this channel contribution...");
        } else {                      /*- Anomaly found, but invalid! */
            tsk->stat[(int)tsk->subchan]      = 0;
            tsk->loc[(int)tsk->subchan]       = 0;
            tsk->strength[(int)tsk->subchan]  = 0;
            VTSS_D("Invalid Anomaly");
        }
        return 0;
    } else if (tsk->stat[(int)tsk->subchan] == 0 && (vtss_state->phy_inst_state.maxAbsCoeff + 2) >= ABS(tsk->strength[(int)tsk->subchan]))  {
        tsk->thresh[(int)tsk->subchan]   = (vtss_state->phy_inst_state.maxAbsCoeff + 2);
        tsk->loc[(int)tsk->subchan]      = 0;
        tsk->strength[(int)tsk->subchan] = 0;
        VTSS_D(" thresh[%d] = %d, loc[%d] =0", tsk->subchan, vtss_state->phy_inst_state.maxAbsCoeff + 2, tsk->subchan);
        return 1;
    } else {
        return 10;
    }
    return 1;
}


/*- `: search for anomalous echo/cross-coupled pair within 8-tap range of EC/NC */
static void xc_search(vtss_state_t *vtss_state,
                      vtss_veriphy_task_t c51_idata *tsk, u8 locFirst, u8 prefix)
{
    u8 idx;
    short s;
    short c51_idata *c;

    VTSS_D("xc_search: locFirst = %d, prefix = %d", locFirst, prefix);
    (void) vtss_phy_page_tr(vtss_state, tsk->port);
    c = readAvgECNCECVar(vtss_state, (tsk->subchan << 6) | tsk->port, tsk->firstCoeff,
                         (3 << 4) | tsk->numCoeffs);

    for (idx = (tsk->numCoeffs - 1), c += (tsk->numCoeffs - 1); (i8)idx >= 0; --idx, --c) {

        s = (short)(32L * (long) * c / (long)tsk->thresh[1]);
        VTSS_D("xc_search: Strength = %d", s);

        if (ABS(*c) > tsk->thresh[1]) {
            if ((ABS(s) <= ABS(tsk->strength[(int)tsk->subchan]))) {
                if ((tsk->signFlip < 0) && (tsk->stat[(int)tsk->subchan] < 4)
                    && (tsk->loc[(int)tsk->subchan] <= (locFirst + idx + 3))) {
                    if ((*c > 0) && (tsk->strength[(int)tsk->subchan] < 0)) {
                        tsk->stat[(int)tsk->subchan] = 2;
                        tsk->signFlip = 2;
                        if ((locFirst + idx) < prefix) {
                            tsk->loc[(int)tsk->subchan] = locFirst + idx;
                        }
                        VTSS_D("\txc_search: Open-->short flip @ tap %d, strength = %d\n", locFirst + idx, s);
                    } else if ((*c < 0) && (tsk->strength[(int)tsk->subchan] > 0)) {
                        tsk->stat[(int)tsk->subchan] = 1;
                        tsk->signFlip = 2;
                        if ((locFirst + idx) < prefix) {
                            tsk->loc[(int)tsk->subchan] = locFirst + idx;
                        }
                        VTSS_D("\txc_search: Short-->open flip @ tap %d, strength = %d\n", locFirst + idx, s);
                    }
                }
            } else if (((locFirst + idx) >= prefix) || /* Or, implies ((locFirst + idx) < prefix) && .. */
                       ((tsk->loc[(int)tsk->subchan] <= (locFirst + idx + 3)) &&
                        (tsk->stat[(int)tsk->subchan] > 0) && (tsk->stat[(int)tsk->subchan] <= 4))) {
                /*- magnitude is largest seen so far */
                if (*c < -tsk->thresh[0]) {
                    if ((tsk->loc[(int)tsk->subchan] <= (locFirst + idx + 3)) &&
                        (tsk->strength[(int)tsk->subchan] > 0) &&
                        (tsk->strength[(int)tsk->subchan] > (-s >> 1))) {
                        tsk->signFlip = 2;
                    }
                    tsk->stat[(int)tsk->subchan] = 1;
                    VTSS_D("\txc_search: Open  @ tap %d, strength = %d < -thresh = -%d\n", locFirst + idx, s, tsk->thresh[0]);
                } else if (*c > tsk->thresh[0]) {
                    if ((tsk->loc[(int)tsk->subchan] <= (locFirst + idx + 3)) &&
                        (tsk->strength[(int)tsk->subchan] < 0) &&
                        (-tsk->strength[(int)tsk->subchan] > (s >> 1))) {
                        tsk->signFlip = 2;
                    }
                    tsk->stat[(int)tsk->subchan] = 2;
                    VTSS_D("\txc_search: Short @ tap %d, strength = %d > +thresh = %d\n", locFirst + idx, s, tsk->thresh[0]);
                } else {
                    tsk->stat[(int)tsk->subchan] = 4;
                    VTSS_D("\txc_search: Anom. @ tap %d, strength = %d > thresh = %d\n", locFirst + idx, s, tsk->thresh[1]);
                }
                tsk->loc[(int)tsk->subchan] = locFirst + idx;
                tsk->strength[(int)tsk->subchan] = s;
            }
        }                       /*- end if (ABS(*c) > tsk->thresh[1]) */
        tsk->signFlip = tsk->signFlip - 1;
    }                           /*- end for ( idx = numCoeffsM1: (i8)idx >= 0; --idx, --c ) */
}

static const int c51_code FFEinit4_7search[2][4] = {
    {  1,   3,  -3,  -1 }, /*- coeff-set for anomSearch */
    { 10,  30, -30, -10 }  /*- coeff-set for lengthSearch */
};

/*- FFEinit4_7: Initializes FFE taps 4-7 from selected table and taps 0-3 & 8-15 are cleared */
static void FFEinit4_7(vtss_state_t *vtss_state,
                       vtss_veriphy_task_t c51_idata *tsk, i8 taps4_7select)
{
    i8 idx, max;
    /* vtss_phy_family_t family;

    family = port_family(tsk->port);

    if (family == VTSS_PHY_FAMILY_MUSTANG) { */
    if (VTSS_PHY_1_GEN_DSP(tsk->port)) {
        VTSS_D("In FFEinit4_7 PHY_1_GEN_DSP");
        max = 28;
        (void) tr_raw_write(vtss_state, tsk->port, 0x0240/*0, 3, 2*/, 0); /*- EcVar<subchan>ForceIdle = 0 */
    } else {
        max = 16;
        (void) tr_raw_write(vtss_state, tsk->port, 0x0240/*0, 4, 0x20*/, 0x100); /*- Set FfeWriteAllSubchans */
    }

    for (idx = 0; idx < max; ++idx) {      /*- Clear FFE except for taps 4-7 */
        (void) tr_raw_write(vtss_state, tsk->port, 0x0200 | (idx << 1)/*0, 4, idx*/, 0);
        if (idx == 3) {
            idx = 7;
        }
    }
    for (idx = 0; idx < 4; ++idx) {
        /* if (family == VTSS_PHY_FAMILY_MUSTANG) { */
        if (VTSS_PHY_1_GEN_DSP(tsk->port)) {
            (void) tr_raw_write(vtss_state, tsk->port, 0x0200 | ((4 + idx) << 1)/*0, 4, 4+idx*/, ((long)FFEinit4_7search[(int)taps4_7select][(int)idx]) << 16);
        } else {
            (void) tr_raw_write(vtss_state, tsk->port, 0x0200 | ((4 + idx) << 1)/*0, 4, 4+idx*/, FFEinit4_7search[(int)taps4_7select][(int)idx] << 9);
        }
    }
    /* if (family == VTSS_PHY_FAMILY_MUSTANG) { */
    if (VTSS_PHY_1_GEN_DSP(tsk->port)) {
        (void) tr_raw_write(vtss_state, tsk->port, 0x0240/*0, 4, 0x20*/, 0x0ff);        /*- Clear FfeWriteAllSubchans */
    } else {
        (void) tr_raw_write(vtss_state, tsk->port, 0x0240/*0, 4, 0x20*/, 0);      /*- Clear FfeWriteAllSubchans */
    }
}

vtss_rc vtss_phy_veriphy_task_start(vtss_state_t *vtss_state, vtss_port_no_t port_no, u8 mode)
{
    vtss_phy_port_state_t         *ps = &vtss_state->phy_state[port_no];
    vtss_veriphy_task_t c51_idata *tsk;
    vtss_phy_family_t             family;
    u16                           reg23;

    tsk = &ps->veriphy;
    family = port_family(vtss_state, port_no);

    if (family == VTSS_PHY_FAMILY_MUSTANG || \
        family == VTSS_PHY_FAMILY_COBRA   || \
        family == VTSS_PHY_FAMILY_QUATTRO || \
        family == VTSS_PHY_FAMILY_LUTON   || \
        family == VTSS_PHY_FAMILY_LUTON_E || \
        family == VTSS_PHY_FAMILY_LUTON24 || \
        family == VTSS_PHY_FAMILY_COOPER  || \
        family == VTSS_PHY_FAMILY_SPYDER  || \
        family == VTSS_PHY_FAMILY_ENZO) {
        tsk->task_state = (u8) ((vtss_veriphy_task_state_t) VERIPHY_STATE_INIT_0);
        tsk->port = port_no;
        tsk->flags = mode << 6;
        VTSS_D("VeriPHY algorithm initialized, Port=%d, State=%d\n", tsk->port, tsk->task_state);
        VTSS_MTIMER_START(&tsk->timeout, 1); /*- start now */

        /* Save Reg23 - to be restored after VeriPHY completes */
        /* select copper */
        VTSS_RC(vtss_phy_rd(vtss_state, tsk->port, 23, &reg23));
        ps->SaveMediaSelect = (reg23 >> 6) & 3;
        reg23 = (reg23 & 0xff3f) | 0x0080;
        VTSS_RC(vtss_phy_wr(vtss_state, tsk->port, 23, reg23));
    } else if (family == VTSS_PHY_FAMILY_LUTON26 ||
               family == VTSS_PHY_FAMILY_ATOM ||
               family == VTSS_PHY_FAMILY_TESLA ||
               family == VTSS_PHY_FAMILY_VIPER) {

        //Bugzilla#6769 - Ports get stuck if running Verify at port that is disabled.
        if (ps->setup.mode != VTSS_PHY_MODE_POWER_DOWN) {
            tsk->task_state = (u8) ((vtss_veriphy_task_state_t) VERIPHY_STATE_INIT_0);
            tsk->port = port_no;

            //signal to API the veriphy is running for this port
            VTSS_RC(vtss_phy_veriphy_running(vtss_state, tsk->port, TRUE, TRUE));

            // Micro patch must be suspended while veriphy is running.
            VTSS_RC(atom_patch_suspend(vtss_state, port_no, TRUE));

            // Starting Veriphy
            VTSS_I("Starting VeriPhY, port_no = %d", port_no);
            VTSS_RC(vtss_phy_page_ext(vtss_state, port_no));
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_VERIPHY_CTRL_REG1, VTSS_F_PHY_VERIPHY_CTRL_REG1_TRIGGER));
            VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
        }
        VTSS_MTIMER_START(&tsk->timeout, 60000); /*- Run for max 60 sec. */
    }
    return VTSS_RC_OK;
}

vtss_rc vtss_phy_veriphy(vtss_state_t *vtss_state, vtss_veriphy_task_t c51_idata *tsk)
{
    u8 idx = 0;
    u32 tr_raw_data;
    vtss_phy_family_t family;
    u16 reg_val;
    vtss_phy_port_state_t  *ps = &vtss_state->phy_state[tsk->port];

    family = port_family(vtss_state, tsk->port);

    if (family == VTSS_PHY_FAMILY_ATOM || \
        family == VTSS_PHY_FAMILY_LUTON26 ||
        family == VTSS_PHY_FAMILY_TESLA ||
        family == VTSS_PHY_FAMILY_VIPER) {
        VTSS_RC(vtss_phy_page_ext(vtss_state, tsk->port));
        VTSS_RC(PHY_RD_PAGE(vtss_state, tsk->port, VTSS_PHY_VERIPHY_CTRL_REG1, &reg_val));
        VTSS_N("VeriPhY Complete reg24E1:0x%X, port = %d", reg_val, tsk->port);
        if (ps->setup.mode == VTSS_PHY_MODE_POWER_DOWN) {
            VTSS_I("Bugzilla#6769 - Ports get stuck if running Verify at port that is disabled.");
            tsk->loc[0] = 0;
            tsk->loc[1] = 0;
            tsk->loc[2] = 0;
            tsk->loc[3] = 0;
            tsk->stat[3] = VTSS_VERIPHY_STATUS_OPEN;
            tsk->stat[2] = VTSS_VERIPHY_STATUS_OPEN;
            tsk->stat[1] = VTSS_VERIPHY_STATUS_OPEN;
            tsk->stat[0] = VTSS_VERIPHY_STATUS_OPEN;
            tsk->flags = 1 << 1; // Signal Veriphy result valid
            VTSS_RC(vtss_phy_page_std(vtss_state, tsk->port));
            return VTSS_RC_OK; // Signal Veriphy Done
        } else if ((reg_val & VTSS_F_PHY_VERIPHY_CTRL_REG1_TRIGGER) == 0x0) {
            // Multiply by 3 because resolution is 3 meters // See datasheet
            VTSS_I("VeriPhY Complete reg24E1:0x%X, port = %d", reg_val, tsk->port);
            tsk->loc[0] = ((reg_val & VTSS_M_PHY_VERIPHY_CTRL_REG1_PAIR_A_DISTANCE) >> 8) * 3;
            tsk->loc[1] = (reg_val & VTSS_M_PHY_VERIPHY_CTRL_REG1_PAIR_B_DISTANCE) * 3;

            VTSS_RC(PHY_RD_PAGE(vtss_state, tsk->port, VTSS_PHY_VERIPHY_CTRL_REG2, &reg_val));
            VTSS_I("VeriPhY Complete reg25E1:0x%X, port = %d", reg_val, tsk->port);
            tsk->loc[2] = ((reg_val & VTSS_M_PHY_VERIPHY_CTRL_REG2_PAIR_C_DISTANCE) >> 8) * 3;
            tsk->loc[3] = (reg_val & VTSS_M_PHY_VERIPHY_CTRL_REG2_PAIR_D_DISTANCE) * 3;

            VTSS_RC(PHY_RD_PAGE(vtss_state, tsk->port, VTSS_PHY_VERIPHY_CTRL_REG3, &reg_val));
            VTSS_I("VeriPhY Complete reg26E1:0x%X, port = %d", reg_val, tsk->port);
            tsk->stat[3] =  reg_val & VTSS_M_PHY_VERIPHY_CTRL_REG3_PAIR_D_TERMINATION_STATUS;
            tsk->stat[2] = (reg_val & VTSS_M_PHY_VERIPHY_CTRL_REG3_PAIR_C_TERMINATION_STATUS) >> 4;
            tsk->stat[1] = (reg_val & VTSS_M_PHY_VERIPHY_CTRL_REG3_PAIR_B_TERMINATION_STATUS) >> 8;
            tsk->stat[0] = (reg_val & VTSS_M_PHY_VERIPHY_CTRL_REG3_PAIR_A_TERMINATION_STATUS) >> 12;

            tsk->flags = 1 << 1; // Signal Veriphy result valid

            VTSS_RC(vtss_phy_page_std(vtss_state, tsk->port));

            //signal to API the veriphy isn't running for this port
            VTSS_RC(vtss_phy_veriphy_running(vtss_state, tsk->port, TRUE, FALSE));

            // Ok veriphy done we can resume the micro patchning.
            VTSS_RC(atom_patch_suspend(vtss_state, tsk->port, FALSE));

            return VTSS_RC_OK; // Signal Veriphy Done
        } else if (VTSS_MTIMER_TIMEOUT(&tsk->timeout)) {
            VTSS_RC(vtss_phy_page_std(vtss_state, tsk->port));
            return VTSS_RC_ERROR;
        } else {
            VTSS_N("VeriPhY Running, port:%d, reg:0x%X", tsk->port, reg_val);
            VTSS_RC(vtss_phy_page_std(vtss_state, tsk->port));
            return VTSS_RC_INCOMPLETE; // Signaling that veriphy is still running
        }
    }


    if (!VTSS_MTIMER_TIMEOUT(&tsk->timeout)) {
        return VTSS_RC_INCOMPLETE;
    }

    VTSS_D("---> ENTER VeriPHY state of port %d is: 0x%02x\n", tsk->port, tsk->task_state);
    VTSS_D("---> statA[%d], statB[%d], statC[%d], statD[%d] ", tsk->stat[0], tsk->stat[1], tsk->stat[2], tsk->stat[3]);

    /*- Handle cleanup for VeriPHY task abort */
    if (tsk->task_state & VTSS_VERIPHY_STATE_DONEBIT) {
        VTSS_D("Handle cleanup for VeriPHY task abort");
        switch (tsk->task_state & ~VTSS_VERIPHY_STATE_DONEBIT) {

        case VTSS_VERIPHY_STATE_IDLE : /*- Nothing to abort from Idle state */
            break;

        default :
            /*- Restore PHY to normal (non-VeriPHY) operation */

            /*- Restore blip-searches to default:  EcVarTrainingTime = 244, */
            /*-            EcVarTrainingGain = 1, EcVarShowtimeGain = 3    */

            VTSS_RC(vtss_phy_page_tr(vtss_state, tsk->port));
            /* if (family == VTSS_PHY_FAMILY_MUSTANG) { */
            if (VTSS_PHY_1_GEN_DSP(tsk->port)) {
                VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0x0b4/*0, 1, 1a*/, 0x1e8ed));
            } else {
                VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0x0190/*0, 3, 8*/, 0xf47));
            }

            if (family == VTSS_PHY_FAMILY_QUATTRO) {
                /*- Force H/W VeriPHY state machine to final state: VeriphyDisable = 1, */
                /*-   VeriPhyStateForce = 1, VeriPhyStateForceVal = 0x10, others default */
                VTSS_RC(vtss_phy_page_tr(vtss_state, tsk->port));
                VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0x0400/*0,  8, 0x00*/, 0x1dec00));
            }

            /*- If VeriPHY operated in link-down mode, ... */
            if ((tsk->flags & 1) == 0) {
                /* if (family == VTSS_PHY_FAMILY_MUSTANG) { */
                if (VTSS_PHY_1_GEN_DSP(tsk->port)) {
                    VTSS_RC(vtss_phy_1_gen_post_veriphy(vtss_state, family, tsk));
                } else {
                    /*- Clear EnableECvarDelayForce/Val */
                    VTSS_RC(vtss_phy_page_tr(vtss_state, tsk->port));
                    VTSS_RC(tr_raw_read(vtss_state, tsk->port, 0x0f86/*1, 15, 0x03*/, &tr_raw_data));
                    tr_raw_data &= ~0x300000;
                    VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0x0f86/*1, 15, 0x03*/, tr_raw_data));
                }
            }

            if (family == VTSS_PHY_FAMILY_LUTON24) {
                /*- Write-back VeriPHY results to Spyder registers and signal VeriPHY completion */
                ExtMiiWrite(vtss_state, tsk->port, 26, ((u16)tsk->stat[0] << 12) |
                            ((u16)(tsk->stat[1] & 0xf) << 8) |
                            ((u16)(tsk->stat[2] & 0xf) << 4) |
                            (u16)(tsk->stat[3] & 0xf) );
                ExtMiiWrite(vtss_state, tsk->port, 25, ((u16)(tsk->loc[2] & 0x3f) << 8) |
                            (u16)(tsk->loc[3] & 0x3f) |
                            ((u16)(tsk->flags & 1) << 15) );
                ExtMiiWrite(vtss_state, tsk->port, 24, ((u16)(tsk->loc[0] & 0x3f) << 8) |
                            (u16)(tsk->loc[1] & 0x3f) |
                            (u16)(tsk->flags & 0xc0) | 0x8000 );
            }

            if (family == VTSS_PHY_FAMILY_SPYDER || \
                family == VTSS_PHY_FAMILY_ENZO || \
                family == VTSS_PHY_FAMILY_LUTON || \
                family == VTSS_PHY_FAMILY_LUTON_E || \
                family == VTSS_PHY_FAMILY_LUTON24 || \
                family == VTSS_PHY_FAMILY_COOPER) {
                /*- Signal completion of VeriPHY operation for Luton/Spyder */
                TpWriteBit(vtss_state, tsk->port, 12, 0, 1);
            }

            /*- Link-up mode VeriPHY now completed, read valid bit! */
            if (tsk->flags & 1) {
                VTSS_MSLEEP(10);              /*- Wait for valid flag to complete */

                if ((family == VTSS_PHY_FAMILY_MUSTANG) ||
                    (family == VTSS_PHY_FAMILY_COBRA) ||
                    (family == VTSS_PHY_FAMILY_COOPER)) {
                    /*- Conditionally set valid bit */
                    tsk->flags |= (MiiReadBits(vtss_state, tsk->port, 17, 13, 13) ^ 1) << 1;
                } else if (family == VTSS_PHY_FAMILY_LUTON || \
                           family == VTSS_PHY_FAMILY_LUTON_E || \
                           family == VTSS_PHY_FAMILY_SPYDER || \
                           family == VTSS_PHY_FAMILY_ENZO) {
                    tsk->flags |= ExtMiiReadBits(vtss_state, tsk->port, 24, 14, 14) << 1; /*- Set valid bit */
                } else {
                    tsk->flags |= ExtMiiReadBits(vtss_state, tsk->port, 25, 14, 14) << 1; /*- Set valid bit */
                }
            } else { /*- Results always valid in link-down mode (avoids AMS problem) */
                tsk->flags |= 2;    /*- Set valid bit */
            }

            if (family == VTSS_PHY_FAMILY_LUTON24) {
                /*- Restore original ams_force_* (VeriPHY must force cu) */
                MiiWriteBits(vtss_state, tsk->port, 23, 7, 6, tsk->flags2 & 3);
            }

            if (family == VTSS_PHY_FAMILY_LUTON ||
                family == VTSS_PHY_FAMILY_LUTON_E) {
                /*- Restore original ActiPHY enable state */
                MiiWriteBits(vtss_state, tsk->port, 28, 6, 6, tsk->flags2 & 1);
            }

            if (family == VTSS_PHY_FAMILY_COOPER) {
                /*- Restore original ActiPHY enable state */
                MiiWriteBits(vtss_state, tsk->port, 23, 5, 5, tsk->flags2 & 1);
            }

            /*- Re-enable RClk125 gating */
            TpWriteBit(vtss_state, tsk->port, 8, 9, 0);

            /*- Switch page back to main page */
            VTSS_RC(vtss_phy_page_std(vtss_state, tsk->port));
        }   /* switch (tsk->task_state & ~VTSS_VERIPHY_STATE_DONEBIT) */

        {
            u16 reg23;
            /* restore mediaselect */
            VTSS_RC(vtss_phy_rd(vtss_state, tsk->port, 23, &reg23));
            reg23 = (reg23 & 0xff3f) | (ps->SaveMediaSelect << 6);
            VTSS_RC(vtss_phy_wr(vtss_state, tsk->port, 23, reg23));
        }

        /*- Return to idle state */
        tsk->task_state = VTSS_VERIPHY_STATE_IDLE;
        return VTSS_RC_OK;
    }  /*- if ABORT */

    switch (tsk->task_state) {
    /*- VeriPHY task is idle. Check for pending VeriPHY request */
    case VTSS_VERIPHY_STATE_IDLE :
        VTSS_D("VTSS_VERIPHY_STATE_IDLE");
        if (family == VTSS_PHY_FAMILY_LUTON24) {
            for (idx = 0; idx < 8; ++idx) {
                /*- Begin search with PHY after last PHY which ran VeriPHY (fairness) */
                tsk->port = (tsk->port + 1) & 7;
                VTSS_RC(vtss_phy_page_ext(vtss_state, tsk->port));
                tsk->thresh[0] = SmiRead(vtss_state, tsk->port, 24);
                if (tsk->thresh[0] & 0x8000) {
                    VTSS_RC(vtss_phy_page_std(vtss_state, tsk->port));
                    tsk->flags = (unsigned char)(tsk->thresh[0] & 0xc0); /*- VeriPHY operating mode */
                    tsk->thresh[0] = SmiRead(vtss_state, tsk->port, 23);
                    tsk->flags2 = (tsk->thresh[0] >> 6) & 3; /*- Save state of ams_force_* */
                    tsk->thresh[0] = (tsk->thresh[0] & 0xff3f) | 0x0080; /*- ams_force_cu=1, ams_force_fi=0 */
                    VTSS_RC(vtss_phy_page_std(vtss_state, tsk->port));
                    VTSS_RC(SmiWrite(vtss_state, tsk->port, 23, tsk->thresh[0]));

                    tsk->task_state = VERIPHY_STATE_INIT_0;
                    break;
                }
            }
        }
        break;

    /*- Configure for VeriPHY operation, and if successful, run VeriPHY algo. */
    case VERIPHY_STATE_INIT_0 :
        /*- Initialize globals for VeriPHY search */
        /* if (family == VTSS_PHY_FAMILY_MUSTANG) { */
        if (VTSS_PHY_1_GEN_DSP(tsk->port)) {
            tsk->numCoeffs = 12;
        } else {
            tsk->numCoeffs = 8;
        }
        for (idx = 0; idx < 4; ++idx) {
            tsk->stat[idx]     = 0;
            tsk->loc[idx]      = 0;
            tsk->strength[idx] = 0;
        }

        VTSS_D("VeriPHY init - TpWriteBit(%d, 8, 9, 1)", tsk->port);
        TpWriteBit(vtss_state, tsk->port, 8, 9, 1);   /*- Disable RClk125 gating */

        if (family == VTSS_PHY_FAMILY_QUATTRO) {
            /*- Hold H/W VeriPHY state machine in the IDLE state: VeriphyDisable = 1, */
            /*-      VeriPhyStateForce = 1, VeriPhyStateForceVal = 0, others default */
            VTSS_D("VeriPHY init - TrRawWrite(%d, 0, 8, 0x00, 0x1de800)", tsk->port);
            VTSS_RC(vtss_phy_page_tr(vtss_state, tsk->port));
            VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0x0400/*0,  8, 0x00*/, 0x1de800));
        }

        /* if (family == VTSS_PHY_FAMILY_MUSTANG) { */
        if (VTSS_PHY_1_GEN_DSP(tsk->port)) {
            /*- Prepare to use VeriPHY on Mustang which has no internal VeriPHY trigger logic */
            /*- Starting VeriPHY requires determining whether 1000-Base-T link is up or not */
            tsk->flags |= MiiReadBits(vtss_state, tsk->port, 17, 12, 12);
            if (tsk->flags & 1) { /*- 1000-Base-T link is up */
                /*- maybe more to do later */;
            } else { /*- 1000-Base-T link is down */
                VTSS_RC(vtss_phy_1_gen_pre_veriphy(vtss_state, family, tsk));
            }
        } else {
            VTSS_D("VeriPHY init - ExtMiiWrite(%d, 24, 0x8000)", tsk->port);
            ExtMiiWrite(vtss_state, tsk->port, 24, 0x8000);       /*- Trigger VeriPHY */
        }

        if (family == VTSS_PHY_FAMILY_LUTON24) {
            /*- Save state of ams_force_* and set ams_force_cu=1, ams_force_fi=0 */
            VTSS_RC(vtss_phy_page_std(vtss_state, tsk->port));
            tsk->thresh[0] = SmiRead(vtss_state, tsk->port, 23);
            tsk->flags2 = (tsk->thresh[0] >> 6) & 3;
            tsk->thresh[0] = (tsk->thresh[0] & 0xff3f) | 0x0080;
            VTSS_RC(vtss_phy_page_std(vtss_state, tsk->port));
            VTSS_RC(SmiWrite(vtss_state, tsk->port, 23, tsk->thresh[0]));
        }

        if (family == VTSS_PHY_FAMILY_LUTON ||
            family == VTSS_PHY_FAMILY_LUTON_E) {
            /*- Save state of ActiPHY enable state and disable ActiPHY */
            VTSS_RC(vtss_phy_page_std(vtss_state, tsk->port));
            tsk->thresh[0] = SmiRead(vtss_state, tsk->port, 28);
            tsk->flags2 = (tsk->thresh[0] >> 6) & 1;
            tsk->thresh[0] = tsk->thresh[0] & 0xffbf;
            VTSS_RC(vtss_phy_page_std(vtss_state, tsk->port));
            VTSS_RC(SmiWrite(vtss_state, tsk->port, 28, tsk->thresh[0]));
        }

        /* Check how it changes the value of Reg23 before and after */
        if (family == VTSS_PHY_FAMILY_COOPER) {
            /*- Save state of ActiPHY enable state and disable ActiPHY */
            VTSS_RC(vtss_phy_page_std(vtss_state, tsk->port));
            tsk->thresh[0] = SmiRead(vtss_state, tsk->port, 23);
            tsk->flags2 = (tsk->thresh[0] >> 5) & 1;
            tsk->thresh[0] = tsk->thresh[0] & 0xffdf;
            VTSS_RC(vtss_phy_page_std(vtss_state, tsk->port));
            VTSS_RC(SmiWrite(vtss_state, tsk->port, 23, tsk->thresh[0]));
        }

        /* if (family != VTSS_PHY_FAMILY_MUSTANG) { */
        if (!VTSS_PHY_1_GEN_DSP(tsk->port)) {
            /*- T(TR_MOD_PORT,TR_LVL_CRIT,"VeriPHY delay 750ms"); */
            /*- Wait for 750 ms for locRcvrStatus fall to propagate to link-down */
            VTSS_MTIMER_START(&tsk->timeout, 750);
        }
        tsk->task_state = VERIPHY_STATE_INIT_1;
        break;

    /*- Continue configuration for VeriPHY operation (after 750 ms delay) */
    case VERIPHY_STATE_INIT_1 :
        /* if (family != VTSS_PHY_FAMILY_MUSTANG) { */
        if (!VTSS_PHY_1_GEN_DSP(tsk->port)) {
            tsk->flags |= MiiReadBits(vtss_state, tsk->port, 17, 12, 12);
            VTSS_D("VeriPHY init - vphy_linkup = %d", tsk->flags & 1);
            if (ExtMiiReadBits(vtss_state, tsk->port, 24, 15, 15) == 0) {
                /*- Link was up, but has gone down after trigger */
                VTSS_D("VeriPHY Init failed: ExtMiiReadBits(tsk->port, 24, 15, 15) == 0");
                tsk->task_state |= 0x80; /*- Abort task! */
                break;
            }
        }
        if (tsk->flags & 1) {
            i8 vgaGain;

            /*- Speed up blip-searches:  EcVarTrainingTime = 56 (32 for VSC8201), */
            /*-   EcVarTrainingGain = 1, EcVarShowtimeGain = 3 */
            VTSS_RC(vtss_phy_page_tr(vtss_state, tsk->port));
            /* if (family == VTSS_PHY_FAMILY_MUSTANG) { */
            if (VTSS_PHY_1_GEN_DSP(tsk->port)) {
                VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0x0b4/*0, 1, 1a*/, 0x40ed));
            } else {
                VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0x0190/*0, 3, 8*/, 0x000387));
            }

            /*- Read VGA state for all four subchannels, extract VGA gain for A */
            VTSS_RC(tr_raw_read(vtss_state, tsk->port, 0x0ff0/*1, 15, 0x38*/, &tr_raw_data));
            vgaGain = (i8)(tr_raw_data >> 4) & 0x0f;

            if (tr_raw_data & 0x100) {
                vgaGain -= 16;
            }
            tsk->log2VGAx256 = 900 + (26 * (int)vgaGain);

            VTSS_D("VeriPHY link-up anomalySearch");
            tsk->task_state = VERIPHY_STATE_INIT_ANOMSEARCH_0;
        } else {
            tsk->thresh[0]   = 400; /*- N: Setup timeout after N*5 ms of LinkControl1000 asserted */
            VTSS_MTIMER_START(&tsk->timeout, 10); /*- Sleep for 10ms before polling MrSpeedStatus the first time */
            tsk->task_state  = VERIPHY_STATE_INIT_LINKDOWN;
        }
        break;

    /*- Continue configuration for link-down VeriPHY */
    case VERIPHY_STATE_INIT_LINKDOWN :
        /*- Wait for MrSpeedStatus[1:0] to become 2 (LinkControl1000 asserted) */
        if (MiiReadBits(vtss_state, tsk->port, 28, 4, 3) == 2) {
            /*- Initialize FFE for link-down operation */
            VTSS_RC(vtss_phy_page_tr(vtss_state, tsk->port));
            FFEinit4_7(vtss_state, tsk, FFEinit4_7anomSearch);

            /*- Speed up blip-searches:  EcVarTrainingTime = 56 (32 for VSC8201), */
            /*-   EcVarTrainingGain = 0, EcVarShowtimeGain = 0 */
            /* if (family == VTSS_PHY_FAMILY_MUSTANG)  */
            if (VTSS_PHY_1_GEN_DSP(tsk->port)) {
                VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0x0b4/*0, 1, 1a*/, 0x400d));
            } else {
                VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0x0190/*0, 3, 8*/, 0x381));
            }
            VTSS_RC(tr_raw_read(vtss_state, tsk->port, 0x0f86/*1, 15, 0x03*/, &tr_raw_data));    /*- Set EnableECvarDelayForce/Val */
            tr_raw_data |= 0x300000;
            VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0x0f86/*1, 15, 0x03*/, tr_raw_data));
            tsk->log2VGAx256 = 0;

            VTSS_D("VeriPHY link-down anomalySearch");
            tsk->task_state = VERIPHY_STATE_INIT_ANOMSEARCH_0;
        } else if (--(tsk->thresh[0]) > 0) {
            VTSS_MTIMER_START(&tsk->timeout, 5); /*- Sleep for 5ms before polling MrSpeedStatus again */
        } else { /*- timed out waiting for MrSpeedStatus to indicate LinkControl1000 asserted! */
            tsk->task_state |= 0x80; /*- Abort VeriPHY task */
        }
        break;

    /*- Setup for start of anomaly search */
    case VERIPHY_STATE_INIT_ANOMSEARCH_0 :
        tsk->nc = (tsk->flags & 0x80) ? 0 : 3;
        tsk->thresh[2] = 0; /*- Clear EC invalid count accumulator */
    case VERIPHY_STATE_INIT_ANOMSEARCH_1 :
        tsk->thresh[1] = 0; /*- Clear EC invalid count (previous value) */
        tsk->thresh[0] = 0; /*- Clear EC invalid count */
    /*- fall throuh into VERIPHY_STATE_ANOMSEARCH_0 state */

    /*- Search for anomalous pair-coupling, pair-shorts, anomalous termination */
    /*- impedance, open-circuits, or short-circuits on all four twisted-pairs. */
    case VERIPHY_STATE_ANOMSEARCH_0 :
        /*- EC invalid count prev = current */
        tsk->thresh[1] = tsk->thresh[0];

        /* if (family == VTSS_PHY_FAMILY_MUSTANG) { */
        if (VTSS_PHY_1_GEN_DSP(tsk->port)) {
            /*- EcVar does not have an input-select on Mustang, but still need */
            /*- to record that there is no forced-delay (bit 1)                */
            tsk->tr_raw0188 = 0;
            /*- allow EC blip time to train to anomaly location */
            if (tsk->nc == 0) {
                VTSS_MTIMER_START(&tsk->timeout, 500);
            }
        } else {
            VTSS_RC(vtss_phy_page_tr(vtss_state, tsk->port));
            VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0x0184/*0, 3, 2*/, 1)); /*- EcVarForceIdle = 1 */

            /*- EcVar[A-D]InputSel = nc */
            tsk->tr_raw0188 = (long)tsk->nc * 0x0aa000;

            VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0x0188/*0, 3, 4*/, tsk->tr_raw0188));
            VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0x0184/*0, 3, 2*/, 0)); /*- EcVarForceIdle = 0 */
            VTSS_D("VeriPHY delay 500ms");

            /*- allow blip time to train to anomaly location */
            VTSS_MTIMER_START(&tsk->timeout, 500);
        }
        tsk->task_state = VERIPHY_STATE_ANOMSEARCH_1;
        break;

    /*- Validate at start of anomaly search that all subchannels are active */
    case VERIPHY_STATE_ANOMSEARCH_1 :
        VTSS_RC(vtss_phy_page_tr(vtss_state, tsk->port));
        /* if (family == VTSS_PHY_FAMILY_MUSTANG) { */
        if (VTSS_PHY_1_GEN_DSP(tsk->port)) {
            VTSS_RC(tr_raw_read(vtss_state, tsk->port, 0x0ba/*0, 1, 0x1d*/, &tr_raw_data));
            if (((tr_raw_data & 0xff0080) == 0xf00080)) {
                /*- EC invalid count current = prev count + 1 */
                tsk->thresh[0] = tsk->thresh[1] + 1;

                VTSS_D("VeriPHY %d%c: %s, blip is ZERO at all delays!", tsk->port, idx + 'A', (tsk->nc >= 2) ? ((tsk->nc > 2) ? "NC3" : "NC2") : ((tsk->nc > 0) ? "NC1" : "EC"));
                /*- Clear any anomaly found under this errant condition */
                for (idx = 0; idx < 4; ++idx) {
                    tsk->stat[idx]      = 0;
                    tsk->loc[idx]       = 0;
                    tsk->strength[idx]  = 0;
                }
            }
        } else {
            for (idx = 0; idx < 4; ++idx) {
                VTSS_RC(tr_raw_read(vtss_state, tsk->port, (idx << 11) | 0x01c0/*idx, 3, 0x20*/, &tr_raw_data));
                if (((tr_raw_data & 0xff0080) == 0xf00080)) {
                    /*- EC invalid count current = prev count + 1 */
                    tsk->thresh[0] = tsk->thresh[1] + 1;

                    VTSS_D("VeriPHY %d%c: %s, blip is ZERO at all delays!", tsk->port, idx + 'A', (tsk->nc >= 2) ? ((tsk->nc > 2) ? "NC3" : "NC2") : ((tsk->nc > 0) ? "NC1" : "EC"));
                    /*- Clear any anomaly found under this errant condition */
                    tsk->stat[idx]      = 0;
                    tsk->loc[idx]       = 0;
                    tsk->strength[idx]  = 0;
                }
            }
        }

        /*- If any invalid EC blips have been found, ... */
        if (tsk->thresh[0] != 0) {
            /*- If the most recent test had no invalid EC blips, ... */
            if (tsk->thresh[0] == tsk->thresh[1]) {
                /*- If the anomaly search is just starting (NC == 3), ... */
                if (tsk->nc == ((tsk->flags & 0x80) ? 0 : 3)) {
                    /*- Accumulate invalid EC blip count for timeouts */
                    tsk->thresh[2] += tsk->thresh[0] - 1;

                    /*- Continue on w/the anomaly search */
                    VTSS_MTIMER_START(&tsk->timeout, 500); /*- Allow blip time to train to anomaly location */
                    tsk->subchan     = 0;     /*- Start search with subchannel A */
                    tsk->task_state  = VERIPHY_STATE_ANOMSEARCH_2;
                } else { /*- in the middle of anomaly search, ... */
                    /*- Restart with NEXT canceller anomaly search */
                    tsk->nc = (tsk->flags & 0x80) ? 0 : 3;

                    /*- EC invalid count current = prev count + 1 */
                    tsk->thresh[0] = tsk->thresh[1] + 1;
                    VTSS_D("VeriPHY %d: After blip is ZERO at all delays, reset to NC3!", tsk->port);

                    /*- delay before restarting anomaly search on NC 3 */
                    VTSS_MTIMER_START(&tsk->timeout, 500); /*- x 5 ms/tick = 500 ms delay */
                    tsk->task_state  = VERIPHY_STATE_ANOMSEARCH_0;
                }
            }
            /*- else the most recent test had an invalid EC blip, */
            /*-      if the accumulated EC blip counts < timeout limit, ... */
            else if ((tsk->thresh[2] + tsk->thresh[0]) < 10) {
                /*- Loop back to wait for all valid EC blips or future timeout */
                tsk->task_state = VERIPHY_STATE_ANOMSEARCH_0;
            } else { /*- more than 10 accumulated invalid EC blips, ... */
                tsk->task_state |= 0x80; /*- Abort VeriPHY task */
            }
            if ((tsk->flags & 1) == 0) { /*- If running in link-down mode, reinitialize FFE */
                FFEinit4_7(vtss_state, tsk, FFEinit4_7anomSearch);
            }
        } else { /*- no invalid EC blips have been found */
            /*- Continue on w/the anomaly search */
            /* if (family == VTSS_PHY_FAMILY_MUSTANG) { */
            if (VTSS_PHY_1_GEN_DSP(tsk->port)) {
                VTSS_MTIMER_START(&tsk->timeout, 200);/*- Allow blip time to train to anomaly location */
            } else {
                if (tsk->nc == 0) {
                    VTSS_MTIMER_START(&tsk->timeout, 200);    /*- Allow blip time to train to anomaly location */
                }
            }
            tsk->subchan     = 0;     /*- Start search with subchannel A */
            tsk->task_state  = VERIPHY_STATE_ANOMSEARCH_2;
        }
        break;

    /*- All subchannels are now known to be active, commence anomaly search */
    case VERIPHY_STATE_ANOMSEARCH_2 :
        /*- Save current status as previous status */
        tsk->stat[(int)tsk->subchan] = (tsk->stat[(int)tsk->subchan] << 4) | (tsk->stat[(int)tsk->subchan] & 0x0f);
        if (tsk->nc != 0 || (tsk->stat[(int)tsk->subchan] & 0xc0) != 0x80) {  /*- Keep cross-pair shorts */
            tsk->signFlip = 0;
            VTSS_RC(vtss_phy_page_tr(vtss_state, tsk->port));

            /* if (family != VTSS_PHY_FAMILY_MUSTANG || tsk->nc == 0) { */
            if ((!VTSS_PHY_1_GEN_DSP(tsk->port)) || (tsk->nc == 0)) {
                /* if (family == VTSS_PHY_FAMILY_MUSTANG) { */
                if (VTSS_PHY_1_GEN_DSP(tsk->port)) {
                    VTSS_D("VERIPHY_STATE_ANOMSEARCH_2 PHY_1_GEN_DSP");
                    VTSS_RC(tr_raw_read(vtss_state, tsk->port, 0x00ba/*0, 1, 0x1d*/, &tr_raw_data));
                    VTSS_D("tr_raw_data (&0x00ba) = 0x%x\n", tr_raw_data);
                } else {
                    VTSS_RC(tr_raw_read(vtss_state, tsk->port, (tsk->subchan << 11) | 0x1c0/*- tsk->subchan, 3, 0x20*/, &tr_raw_data));
                }

                if (tr_raw_data & 128) {     /*- found anything? */
                    /*- ecVarBestDelay signals something really found (maybe < thresh) */
                    /*- ^^^^^^^^^^^^^^ ---------> vvvvvvvvvvvv */
                    /* if (family == VTSS_PHY_FAMILY_MUSTANG) { */
                    if (VTSS_PHY_1_GEN_DSP(tsk->port)) {
                        idx = 64 + (tr_raw_data >> 16); /*- Mustang only can append ECvar onto the end of EC, not NC */
                    } else {
                        idx = ((tsk->nc > 0) ? 16 : 72) + (tr_raw_data >> 16);
                    }
                    VTSS_D("VeriPHY %d%c: %s, blip @ tap %d", tsk->port, tsk->subchan + 'A', (tsk->nc >= 2) ? ((tsk->nc > 2) ? "NC3" : "NC2") : ((tsk->nc > 0) ? "NC1" : "EC"), idx);
                    VTSS_D("possible anomaly near tap %d\n", idx);
                    get_anom_thresh(tsk, idx + (tsk->numCoeffs >> 1));
                    /* tsk->firstCoeff = (family == VTSS_PHY_FAMILY_MUSTANG ? 192 : 72); */
                    tsk->firstCoeff = VTSS_PHY_1_GEN_DSP(tsk->port) ? 192 : 72;

                    xc_search(vtss_state, tsk, idx, 0);
                    if (tsk->stat[(int)tsk->subchan] > 0 && tsk->stat[(int)tsk->subchan] <= 0x0f) {
                        (void) checkValidity(vtss_state, tsk, MAX_ABS_COEFF_ANOM_INVALID_NOISE);

                        /*- Update previous status to match current status after validity check */
                        tsk->stat[(int)tsk->subchan] = (tsk->stat[(int)tsk->subchan] << 4) | (tsk->stat[(int)tsk->subchan] & 0x0f);
                    }
                } else {
                    VTSS_D("VeriPHY %d%c: %s (no blip)", tsk->port, tsk->subchan + 'A', (tsk->nc >= 2) ? ((tsk->nc > 2) ? "NC3" : "NC2") : ((tsk->nc > 0) ? "NC1" : "EC"));
                }
            }
            /* if (family == VTSS_PHY_FAMILY_MUSTANG) { */
            if (VTSS_PHY_1_GEN_DSP(tsk->port)) {
                tsk->firstCoeff = (tsk->nc > 0) ? ((tsk->nc << 5) - 20) : 0xb4;
            } else {
                tsk->firstCoeff = (tsk->nc > 0) ? ((tsk->nc << 4) + 72) : 64;
            }
            tsk->task_state = VERIPHY_STATE_ANOMSEARCH_3;
        } else if (tsk->subchan < 3) { /*- Move on to next subchannel, if not done */
            (tsk->subchan)++; /*- Re-enter in the same state for next subchannel */
        } else if (tsk->nc > 0) { /*- Completed sweep of all 4 subchannels, move to next NC or EC */
            (tsk->nc)--;
            tsk->task_state = VERIPHY_STATE_INIT_ANOMSEARCH_1;
        } else { /*- Completed sweep of EC, exit anomaly search */
            tsk->task_state = VERIPHY_STATE_INIT_CABLELEN;
            tsk->subchan = 0;
        }
        break;

    /*- Continue anomaly search by sweeping through fixed EC/NC @ tsk->firstCoeff */
    case VERIPHY_STATE_ANOMSEARCH_3 :
        /* if (family == VTSS_PHY_FAMILY_MUSTANG) { */
        if (VTSS_PHY_1_GEN_DSP(tsk->port)) {
            if (tsk->firstCoeff >= 128) { /*- if searching EC */
                idx = tsk->firstCoeff - 128;
            } else { /*- searching NC1..3 */
                idx = tsk->firstCoeff & 0x1f;
            }
        } else {
            if (tsk->firstCoeff < 80) { /*- if searching EC */
                idx = tsk->firstCoeff;
            } else { /*- searching NC1..3 */
                idx = tsk->firstCoeff & 0x0f;
            }
        }
        get_anom_thresh(tsk, idx + (tsk->numCoeffs >> 1));
        xc_search(vtss_state, tsk, idx, (idx == 4) ? 6 : 0);
        if (tsk->stat[(int)tsk->subchan] > 0 && tsk->stat[(int)tsk->subchan] <= 0x0f) {
            (void) checkValidity(vtss_state, tsk, MAX_ABS_COEFF_ANOM_INVALID_NOISE);

            /*- Update previous status to match current status after validity check */
            tsk->stat[(int)tsk->subchan] = (tsk->stat[(int)tsk->subchan] << 4) | (tsk->stat[(int)tsk->subchan] & 0x0f);
        }
        if (tsk->nc > 0) {
            if (idx > 0) {
                tsk->firstCoeff -= tsk->numCoeffs;
            } else {
                /*- Recode NC-anomalies as cross-pair anomalies at end-of-search */
                if ((tsk->stat[(int)tsk->subchan] & 0x0f) > 0 && (tsk->stat[(int)tsk->subchan] & 0x08) == 0) {
                    tsk->stat[(int)tsk->subchan] = (tsk->stat[(int)tsk->subchan] & 0xf4) | 8;
                    if (tsk->nc != tsk->subchan) {
                        tsk->stat[(int)tsk->subchan] += tsk->nc;
                    }
                }
            }
        } else { /*- for EC, conditionally extend search down to 6th tap */
            /* if (family == VTSS_PHY_FAMILY_MUSTANG) { */
            if (VTSS_PHY_1_GEN_DSP(tsk->port)) {
                if (idx > 16) {
                    tsk->firstCoeff -= tsk->numCoeffs;
                } else if (idx == 16) {
                    tsk->firstCoeff  = 0x80 | 5;
                    tsk->numCoeffs = 11;
                } else {
                    tsk->numCoeffs = 12; /*- Restore to default of 12 coeffs */
                    idx = 0; /*- Walk to next state */
                }
            } else {
                if (idx > 8) {
                    tsk->firstCoeff -= tsk->numCoeffs;
                } else if (idx == 8) {
                    tsk->firstCoeff  = 4;
                    tsk->numCoeffs = 4;
                } else {
                    tsk->numCoeffs = 8; /*- Restore to default of 8 coeffs */
                    idx = 0; /*- Walk to next state */
                }
            }
        }
        if (idx == 0) {
            if (tsk->subchan < 3) { /*- Move on to next subchannel, if not done */
                (tsk->subchan)++;
                tsk->task_state = VERIPHY_STATE_ANOMSEARCH_2;
            } else if (tsk->nc > 0) { /*- Completed sweep of all 4 subchannels, move to next NC or EC */
                (tsk->nc)--;
                tsk->task_state = VERIPHY_STATE_INIT_ANOMSEARCH_1;
            } else { /*- Completed sweep of EC, exit anomaly search */
                tsk->task_state = VERIPHY_STATE_INIT_CABLELEN;
                tsk->subchan = 0;
            }
        }
        break;

    /*- Initialize getCableLength search */
    case VERIPHY_STATE_INIT_CABLELEN :
        for (idx = 0; (tsk->flags & 0xc0) == 0 && idx < 4; ++idx) {
            tsk->stat[idx] &= 0x0f;
            if (tsk->stat[idx] == 0) {
                if ((tsk->flags & 0xc0) != 0) {
                    tsk->loc[idx] = 255;    /*- Set to unknown location, if cable length not measured */
                } else {
                    tsk->flags |= 4;
                }
            }
        }
        if ((tsk->flags & 0xc0) != 0 || (tsk->flags & 4) == 0) { /*- Anoamlies found on all four pairs */
            tsk->task_state = VERIPHY_STATE_PAIRSWAP;
        } else { /*- Initialize for measuring cable length */
            VTSS_RC(vtss_phy_page_tr(vtss_state, tsk->port));
            VTSS_D("VeriPHY getCableLength");
            if ((tsk->flags & 1) == 0) {/*- If link-down, gain up FFE to make blip easier to spot! */
                FFEinit4_7(vtss_state, tsk, FFEinit4_7lengthSearch);
                /*- Smooth EcVar training: EcVarTrainingTime = 75, EcVarShowtimeGain = 1 */
                /* if (family == VTSS_PHY_FAMILY_MUSTANG) { */
                if (VTSS_PHY_1_GEN_DSP(tsk->port)) {
                    VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0x0b4/*0, 1, 1a*/, 0x96ad));
                } else {
                    VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0x190/*0, 3, 8*/, 0x4b5));
                }
            }
            /* if (family == VTSS_PHY_FAMILY_MUSTANG) { */
            if (VTSS_PHY_1_GEN_DSP(tsk->port)) {
                /*- EcVarForceDelayVal = 232 - 64, EcVarForceDelay = 1, EcVarForceIdle=1 */
                VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0x0b6/*0, 1, 1b*/, (((232 - 64) << 2) | 3)));
                VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0x0b6/*0, 1, 1b*/, (((232 - 64) << 2) | 2))); /*- EcVarForceIdle = 0 */
            } else {
                /*- EcVarForceDelayVal = 232 - 72, EcVarForceDelay = 1 */
                VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0x0188/*0, 3, 4*/, ((232 - 72) << 1) | 1));
                VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0x0184/*0, 3, 2*/, 1));   /*- EcVarForceIdle = 1 */
                VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0x0184/*0, 3, 2*/, 0));   /*- EcVarForceIdle = 0 */
            }
            idx = 0xff;
            for (tsk->subchan = 0; tsk->subchan < 4; ++(tsk->subchan)) {
                if (tsk->stat[(int)tsk->subchan] != 0) {
                    tsk->thresh[(int)tsk->subchan] = 0;
                } else {
                    for (tsk->signFlip = 0, vtss_state->phy_inst_state.maxAbsCoeff = 0; vtss_state->phy_inst_state.maxAbsCoeff < ((tsk->flags & 1) ? 1 : 4) && tsk->signFlip < 100; ++(tsk->signFlip)) {
                        /*- ECVar Tap=0, Discard =1, Rpt = 8, numCoeffs=8 */

                        (void) readAvgECNCECVar(vtss_state, (tsk->subchan << 6) | tsk->port,
                                                family == VTSS_PHY_1_GEN_DSP(tsk->port) ? 192 : 72,
                                                0xf8);

                        if (vtss_state->phy_inst_state.maxAbsCoeff < ((tsk->flags & 1) ? 1 : 4)) {
                            VTSS_MSLEEP(2);
                        } else {
                            tsk->thresh[(int)tsk->subchan] = vtss_state->phy_inst_state.maxAbsCoeff;
                        }
                    }
                    if (tsk->signFlip > 1) {
                        VTSS_D("VeriPHY %d%c: maxAbsC(tsk->thresh) = %d took %d attempts!", tsk->port, tsk->subchan + 'A', tsk->thresh[(int)tsk->subchan], tsk->signFlip);
                    }
                    if (tsk->thresh[(int)tsk->subchan] < 14) {
                        tsk->thresh[(int)tsk->subchan] = 20;
                    } else {
                        tsk->thresh[(int)tsk->subchan] += 6;
                    }
                    if (tsk->thresh[(int)tsk->subchan] > MAX_ABS_COEFF_LEN_INVALID_NOISE) {
                        VTSS_D("VeriPHY %d%c: maxAbsC(tsk->thresh) = %d > noise limit of %d!", tsk->port, tsk->subchan + 'A', tsk->thresh[(int)tsk->subchan], MAX_ABS_COEFF_LEN_INVALID_NOISE);
                        tsk->loc[(int)tsk->subchan] = 255;
                        tsk->strength[(int)tsk->subchan] = tsk->thresh[(int)tsk->subchan];
                    } else if (idx == 0xff) {
                        idx = (tsk->subchan << 2) | tsk->subchan;
                    } else if (tsk->thresh[(int)tsk->subchan] > tsk->thresh[idx >> 2]) {
                        idx = tsk->subchan << 2 | (idx & 3);
                    }
                    if (idx != 0xff && tsk->thresh[(int)tsk->subchan] < tsk->thresh[idx & 3]) {
                        idx = (idx & 0x0c) | tsk->subchan;
                    }
                }
            }
            /*- if max(tsk->thresh) >= 1.5*min(tsk->thresh), the max subchan may be unreliable */
            if (idx != 0xff && (tsk->thresh[idx >> 2] >= (tsk->thresh[idx & 3] + (tsk->thresh[idx & 3] >> 1)))) {
                tsk->flags = ((idx << 2) & 0x30) | 0x08 | (tsk->flags & 0xc7);
            } else {
                tsk->flags = (tsk->flags & 0xc7);
            }

            VTSS_D("VeriPHY %d: tsk->thresh = { %d, %d, %d, %d }, unreliablePtr = %d", tsk->port, tsk->thresh[0], tsk->thresh[1], tsk->thresh[2], tsk->thresh[3], (((tsk->flags >> 4) & 3) + (tsk->flags & 8)));

            tsk->flags &= ~4; /*- Clear done flag */
            tsk->signFlip = 0;
            /* if (family == VTSS_PHY_FAMILY_MUSTANG) { */
            if (VTSS_PHY_1_GEN_DSP(tsk->port)) {
                tsk->tr_raw0188 = (168 << 2) | 2;
                tsk->firstCoeff = 192;
            } else {
                tsk->tr_raw0188 = (160 << 1) | 1;
                tsk->firstCoeff = 72;
            }
            tsk->task_state = VERIPHY_STATE_GETCABLELEN_0;
        }
        break;

    /*- start getCableLength search */
    case VERIPHY_STATE_GETCABLELEN_0 :
        if (((tsk->flags & 4) == 0) && ((tsk->firstCoeff & 0x7f) >= tsk->numCoeffs)) {
            /* if (family == VTSS_PHY_FAMILY_MUSTANG) { */
            if (VTSS_PHY_1_GEN_DSP(tsk->port)) {
                if (tsk->firstCoeff == 192) {
                    idx = tsk->tr_raw0188 >> 2;
                    if (idx > 0) {
                        /*- EcVarForceDelayVal = current delay, EcVarForceDelay = 1 */
                        idx -= tsk->numCoeffs;
                        tsk->tr_raw0188 = (idx << 2) | 2;
                        idx += 64; /*- Make idx indicate coefficient tap # */
                    } else { /*- End blip scan; restore blip canceller to normal operation */
                        /*- EcVarForceDelayVal = 0, EcVarForceDelay = 0 */
                        tsk->tr_raw0188 = 0;
                        tsk->firstCoeff = 0x80 | 52;

                        /*- Scale up threshold to avoid false triggers */
                        for (idx = 0; idx < 4; ++idx) {
                            tsk->thresh[idx] = tsk->thresh[idx] + (tsk->thresh[idx] >> 2);
                        }
                        idx = tsk->firstCoeff & 0x3f; /*- Make idx indicate coefficient tap # */
                    }

                    VTSS_RC(vtss_phy_page_tr(vtss_state, tsk->port));
                    VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0x0b6/*0, 1, 1b*/, tsk->tr_raw0188 | 1)); /*- EcVarForceIdle = 1 */
                    VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0x0b6/*0, 1, 1b*/, tsk->tr_raw0188));     /*- EcVarForceIdle = 0 */
                } else {
                    tsk->firstCoeff -= tsk->numCoeffs;
                    idx = tsk->firstCoeff & 0x3f; /*- Make idx indicate coefficient tap # */
                }
            } else {
                if (tsk->firstCoeff == 72) {
                    idx = tsk->tr_raw0188 >> 1;
                    if (idx > 0) {
                        /*- EcVarForceDelayVal = current delay, EcVarForceDelay = 1 */
                        idx -= tsk->numCoeffs;
                        tsk->tr_raw0188 = (idx << 1) | 1;
                        idx += 72; /*- Make idx indicate coefficient tap # */
                    } else { /*- End blip scan; restore blip canceller to normal operation */
                        /*- EcVarForceDelayVal = 0, EcVarForceDelay = 0 */
                        tsk->tr_raw0188 = 0;
                        tsk->firstCoeff = 64;

                        /*- Scale up threshold to avoid false triggers */
                        for (idx = 0; idx < 4; ++idx) {
                            tsk->thresh[idx] = tsk->thresh[idx] + (tsk->thresh[idx] >> 2);
                        }
                        idx = tsk->firstCoeff; /*- Make idx indicate coefficient tap # */
                    }

                    VTSS_RC(vtss_phy_page_tr(vtss_state, tsk->port));
                    VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0x0188/*0, 3, 4*/, tsk->tr_raw0188));
                    VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0x0184/*0, 3, 2*/, 1));     /*- EcVarForceIdle = 1 */
                    VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0x0184/*0, 3, 2*/, 0));     /*- EcVarForceIdle = 0 */
                } else {
                    tsk->firstCoeff -= tsk->numCoeffs;
                    idx = tsk->firstCoeff; /*- Make idx indicate coefficient tap # */
                }
            }

            VTSS_D("probing for cable length at tap %d\n", idx);
            /*- delay done by numCoeffs taps to scan numCoeffs taps past 1st detection to refine length estimate */
            if ((tsk->loc[0] > 0) && (tsk->loc[1] > 0) && (tsk->loc[2] > 0) && (tsk->loc[3] > 0)) {
                tsk->flags |= 4;    /*- Set done flag */
            }

            for (tsk->subchan = 0; tsk->subchan < 4; ++(tsk->subchan)) {
                /*- only measure length on non-anomalous subchans */
                VTSS_RC(vtss_phy_page_tr(vtss_state, tsk->port));
                if (tsk->stat[(int)tsk->subchan] == 0 && ((idx + 12) > tsk->loc[(int)tsk->subchan])) {
                    short c51_idata *c = readAvgECNCECVar(vtss_state, (tsk->subchan << 6) | tsk->port,
                                                          tsk->firstCoeff, 0x20 | tsk->numCoeffs);
                    u8 cnt;

                    c   += tsk->numCoeffs;
                    idx += tsk->numCoeffs;
                    cnt  = tsk->numCoeffs;
                    do {
                        --idx;
                        --c;
                        if (((tsk->signFlip >> tsk->subchan) & 1) != 0) {
                            if (tsk->loc[(int)tsk->subchan] == (idx + 1)) {
                                if ( (tsk->strength[(int)tsk->subchan] < 0 && *c < tsk->strength[(int)tsk->subchan]) ||
                                     (tsk->strength[(int)tsk->subchan] > 0 && *c > tsk->strength[(int)tsk->subchan]) ) {
                                    tsk->loc[(int)tsk->subchan] = idx;
                                    tsk->strength[(int)tsk->subchan] = *c;
                                    VTSS_D("VeriPHY %d%c: ", (int)tsk->port, (int)(tsk->subchan + 'A'));
                                    VTSS_D("sign-move 2 tap %d, strength = %d\n", (int)tsk->loc[(int)tsk->subchan], tsk->strength[(int)tsk->subchan]);
                                }
                            }
                        } else if (ABS(*c) > tsk->thresh[(int)tsk->subchan]) {
                            if ((tsk->strength[(int)tsk->subchan] == 0) ||
                                ((tsk->loc[(int)tsk->subchan] <= (idx + 3)) &&
                                 (((tsk->strength[(int)tsk->subchan] > 0) &&
                                   (tsk->strength[(int)tsk->subchan] <= *c))
                                  || ((tsk->strength[(int)tsk->subchan] < 0) &&
                                      (tsk->strength[(int)tsk->subchan] >= *c))
                                 ))) {
                                if (tsk->strength[(int)tsk->subchan] == 0) { /*- Test validity 1st detection only */
                                    tsk->strength[(int)tsk->subchan] = *c;
                                    tsk->loc[(int)tsk->subchan] = idx;
                                    (void) checkValidity(vtss_state, tsk, MAX_ABS_COEFF_LEN_INVALID_NOISE);
                                    if (tsk->strength[(int)tsk->subchan] != 0) {
                                        VTSS_D("VeriPHY %d%c: ", (int)tsk->port, (int)(tsk->subchan + 'A'));
                                        VTSS_D("blip-det. @ tap %d, strength = %d\n", idx, *c);
                                    }
                                } else {
                                    tsk->strength[(int)tsk->subchan] = *c;
                                    tsk->loc[(int)tsk->subchan] = idx;
                                    VTSS_D("VeriPHY %d%c: ", (int)tsk->port, (int)(tsk->subchan + 'A'));
                                    VTSS_D("blip-move 2 tap %d, strength = %d\n", idx, *c);
                                }
                            } else if ((tsk->loc[(int)tsk->subchan] <= (idx + 5)) &&
                                       (((tsk->strength[(int)tsk->subchan] > 0) &&
                                         ((-tsk->strength[(int)tsk->subchan] + (tsk->strength[(int)tsk->subchan] >> 3)) >= *c))
                                        || ((tsk->strength[(int)tsk->subchan] < 0) &&
                                            ((-tsk->strength[(int)tsk->subchan] + (tsk->strength[(int)tsk->subchan] >> 3)) <= *c))
                                       )) {
                                tsk->loc[(int)tsk->subchan] = idx;
                                tsk->strength[(int)tsk->subchan] = *c;
                                tsk->signFlip |= 1 << tsk->subchan;
                                VTSS_D("VeriPHY %d%c: ", (int)tsk->port, (int)(tsk->subchan + 'A'));
                                VTSS_D("sign-flip @ tap %d, strength = %d\n", tsk->loc[(int)tsk->subchan], tsk->strength[(int)tsk->subchan]);
                            }
                        }
                    } while (--cnt != 0);
                }
            }                   /*- end for ( tsk->subchan = 0; tsk->subchan < 4; ++(tsk->subchan) ) */
            if (tsk->flags & 4) {
                tsk->task_state = VERIPHY_STATE_GETCABLELEN_1;
                VTSS_RC(vtss_phy_page_tr(vtss_state, tsk->port));
                /* if (family == VTSS_PHY_FAMILY_MUSTANG) { */
                if (VTSS_PHY_1_GEN_DSP(tsk->port)) {
                    VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0x0b6/*0, 1, 1b*/, 1)); /*- EcVarForceDelay = 0, EcVarForceIdle = 1 */
                    VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0x0b6/*0, 1, 1b*/, 0)); /*- EcVarForceIdle = 0 */
                } else {
                    VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0x0188/*0, 3, 4*/, 0)); /*- EcVarForceDelay = 0 */
                    VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0x0184/*0, 3, 2*/, 1)); /*- EcVarForceIdle = 1 */
                    VTSS_RC(tr_raw_write(vtss_state, tsk->port, 0x0184/*0, 3, 2*/, 0)); /*- EcVarForceIdle = 0 */
                }
            }
        } else {
            tsk->task_state = VERIPHY_STATE_GETCABLELEN_1;
        }
        break;

    /*- getCableLength search state 2 */
    case VERIPHY_STATE_GETCABLELEN_1 :
        /*- Length estimate to be the average of medians of computed loc's */
    {
        u8 maxptr, minptr, max2ptr, min2ptr, endloc = 0;

        maxptr = minptr = 8;
        for (idx = 0; idx < 4; ++idx) {
            if (tsk->stat[idx] == 0 && tsk->loc[idx] < 255) {
                short s;

                get_anom_thresh(tsk, tsk->loc[idx]);
                if (tsk->loc[idx] <= 8) {
                    s = 0;    /*- Don't change blips on <2m loops anomalous */
                } else if (tsk->flags & 1) { /*- if in link-up mode, ... */
                    s = (3 * tsk->strength[idx] + 2) >> 2;    /*- 3/4  of strength link-up */
                } else { /*- else in link-down mode */
                    s = (3 * tsk->strength[idx] + 16) >> 5;    /*- 3/32 of strength link-down */
                }

                if (s > tsk->thresh[0]) {
                    tsk->stat[idx] = 2;
                    VTSS_D("VeriPHY %d%c: changing length (%d) to short (%d > %d)!\n", tsk->port, idx + 'A', endloc, s, tsk->thresh[0]);
                } else if (s < -tsk->thresh[0]) {
                    tsk->stat[idx] = 1;
                    VTSS_D("VeriPHY %d%c: changing length (%d) to open (%d < -%d)!\n", tsk->port, idx + 'A', endloc, s, tsk->thresh[0]);
                } else if (ABS(s) > tsk->thresh[1]) {
                    tsk->stat[idx] = 4;
                    VTSS_D("VeriPHY %d%c: changing length (%d) to anom (ABS(%d) > %d)!\n", tsk->port, idx + 'A', endloc, s, tsk->thresh[1]);
                } else if (((tsk->signFlip >> idx) & 1) == 0) { /*- if no sign flip detected, */
                    /* if (family == VTSS_PHY_FAMILY_MUSTANG) { */
                    if (VTSS_PHY_1_GEN_DSP(tsk->port)) {
                        tsk->loc[idx] = tsk->loc[idx] - 3;      /*- reduce estimated tap by 3 */
                    } else {
                        tsk->loc[idx] = tsk->loc[idx] - 1;      /*- reduce estimated tap by 1 */
                    }
                }
                VTSS_D("subchan %c, loc = %d\n", idx + 'A', tsk->loc[idx]);

                if (((tsk->flags & 8) == 0 || idx != ((tsk->flags >> 4) & 3)) && ((minptr >= 4) || (tsk->loc[idx] < tsk->loc[minptr]))) {
                    minptr = idx;
                }
                if (((tsk->flags & 8) == 0 || idx != ((tsk->flags >> 4) & 3)) && ((maxptr >= 4) || (tsk->loc[idx] > tsk->loc[maxptr]))) {
                    maxptr = idx;
                }
            }
        }
        if (minptr == 8) {
            /*- Only use unreliablePtr length as a last resort */
            if (tsk->flags & 8) {
                endloc = tsk->loc[(tsk->flags >> 4) & 3];
                VTSS_D("VeriPHY %d: unreliable endloc = %d is only available!\n", tsk->port, endloc);
            } else {
                endloc = 255;
                VTSS_D("VeriPHY %d: No length (endloc = 255) is available!\n", tsk->port);
            }
        } else {
            /*- Rehabilitate the unreliablePtr if result is reasonable before median search */
            if ((tsk->flags & 8) && tsk->loc[(tsk->flags >> 4) & 3] >= (tsk->loc[minptr] - 1)) {
                if (tsk->loc[(tsk->flags >> 4) & 3] == (tsk->loc[minptr] - 1)) {
                    minptr = (tsk->flags >> 4) & 3;
                    tsk->flags &= ~8;
                } else if (tsk->loc[(tsk->flags >> 4) & 3] <= (tsk->loc[maxptr] + 1)) {
                    if (tsk->loc[(tsk->flags >> 4) & 3] == (tsk->loc[maxptr] + 1)) {
                        maxptr = (tsk->flags >> 4) & 3;
                    }
                    tsk->flags &= ~8;
                }
            }

            /*- Find the middle 2 results (may point to the same item, min, or max) */
            min2ptr = maxptr;
            max2ptr = minptr;
            for (idx = 0; idx < 4; ++idx) {
                if (tsk->stat[idx] == 0 && tsk->loc[idx] < 255 && ((tsk->flags & 8) == 0 || idx != ((tsk->flags >> 4) & 3))) {
                    if ((idx != minptr) && (tsk->loc[idx] < tsk->loc[min2ptr])) {
                        min2ptr = idx;
                    }
                    if ((idx != maxptr) && (tsk->loc[idx] > tsk->loc[max2ptr])) {
                        max2ptr = idx;
                    }
                }
            }
            endloc = (max2ptr == minptr) ? tsk->loc[maxptr] : (u8)(((u32)tsk->loc[min2ptr] + tsk->loc[max2ptr]) >> 1);
            VTSS_D("VeriPHY %d: max/minptr = %d/%d, max2/min2ptr = %d/%d, endloc = %d!\n", tsk->port, maxptr, minptr, max2ptr, min2ptr, endloc);
        }
        for (idx = 0; idx < 4; ++idx) {
            if (tsk->stat[idx] == 0) {
                VTSS_D("VeriPHY loc[%d] = %d --> %d\n", idx, tsk->loc[idx], endloc);
                tsk->loc[idx] = endloc;
            }
        }
        tsk->task_state = VERIPHY_STATE_PAIRSWAP;
    }
    break;

    /*- PairSwap & VeriPHY finish-up state */
    case VERIPHY_STATE_PAIRSWAP :
        /*- Swap pairs as appropriate to represent pairs at connector */
        /*- (up to now, it was subchan) */
        VTSS_D("VeriPHY swapPairs");
        {
            i8 MDIX_CDpairSwap;
            u8 ubtemp;
            short stemp;

            /*- Read MDI/MDIX, bit[1], and CD-pair-swap, bit [0] */
            MDIX_CDpairSwap = MiiReadBits(vtss_state, tsk->port, 28, 13, 12 );

            if (MDIX_CDpairSwap < 2) {
                /*- Swap pairs A & B */
                stemp            = tsk->strength[1];
                tsk->strength[1] = tsk->strength[0];
                tsk->strength[0] = stemp;
                ubtemp           = tsk->stat[1];
                tsk->stat[1]     = tsk->stat[0];
                tsk->stat[0]     = ubtemp;
                ubtemp           = tsk->loc[1];
                tsk->loc[1]      = tsk->loc[0];
                tsk->loc[0]      = ubtemp;

                /*- Recode cross-pair status for pairs A & B */
                for (idx = 0; idx < 4; ++idx) {
                    if ( (tsk->stat[idx] & 10) == 8 ) {
                        tsk->stat[idx] = tsk->stat[idx] ^ 1;
                    }
                }
            }

            if ((MDIX_CDpairSwap == 0) || (MDIX_CDpairSwap == 3)) {
                /*- Swap pairs C & D */
                stemp            = tsk->strength[3];
                tsk->strength[3] = tsk->strength[2];
                tsk->strength[2] = stemp;
                ubtemp           = tsk->stat[3];
                tsk->stat[3]     = tsk->stat[2];
                tsk->stat[2]     = ubtemp;
                ubtemp           = tsk->loc[3];
                tsk->loc[3]      = tsk->loc[2];
                tsk->loc[2]      = ubtemp;

                /*- Recode cross-pair status for pairs C & D */
                for (idx = 0; idx < 4; ++idx) {
                    if ( (tsk->stat[idx] & 10) == 10 ) {
                        tsk->stat[idx] = tsk->stat[idx] ^ 1;
                    }
                }
            }
        }

        /*- Convert tap numbers into length in meters for return to user */
        for (idx = 0; idx < 4; ++idx) {
            u8 loc = tsk->loc[idx];
            VTSS_D("Tap Location - loc[%d] = %d\n", idx, tsk->loc[idx]);

            if (loc <= 7) {
                tsk->loc[idx] = 0;
            } else if (loc < 255) {
                /*- Initial taps that are close to zero, differing FFE spreading effects, */
                /*- scale factor is slightly different in Mustang */
                /* if (family == VTSS_PHY_FAMILY_MUSTANG) { */
                if (VTSS_PHY_1_GEN_DSP(tsk->port)) {
                    if (tsk->flags & 1) {
                        if (family == VTSS_PHY_FAMILY_MUSTANG) {
                            loc -= 13;
                        } else {
                            loc -= 7;
                        }
                    } else {
                        loc -= 7;
                    }
                } else {
                    if (tsk->flags & 1) {
                        loc -= 7;
                    } else {
                        loc -= 6;
                    }
                }
                loc = loc - ((((loc + (loc >> 4)) >> 2) + 1) >> 1);
                tsk->loc[idx] = loc;
                if ((tsk->stat[idx] == 2) && (loc < 5)) {
                    tsk->stat[idx] = 4;
                }
            }
        }
        tsk->task_state = VERIPHY_STATE_FINISH;
        break;
    }

    if (tsk->task_state == VTSS_VERIPHY_STATE_IDLE) { /*- also covers FINISH state */
        VTSS_MTIMER_CANCEL(&tsk->timeout);
    }

    VTSS_D("<--- EXIT VeriPHY state of port %d is: 0x%02x\n", tsk->port, tsk->task_state);

    return (tsk->task_state == VTSS_VERIPHY_STATE_IDLE) ? VTSS_RC_OK : VTSS_RC_INCOMPLETE;
}

#endif /* VTSS_PHY_OPT_VERIPHY */
#endif // VTSS_CHIP_CU_PHY
