/*

 Vitesse API software.

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

 $Id$
 $Revision$

*/

#define VTSS_TRACE_GROUP VTSS_TRACE_GROUP_PORT
#include "vtss_serval_cil.h"

#if defined(VTSS_ARCH_SERVAL)

/* - CIL functions ------------------------------------------------- */

/* =================================================================
 *  Serdes configuration
 * =================================================================*/

#define SRVL_SERDES_WAIT 100000

static const char *srvl_serdes_mode_txt(vtss_serdes_mode_t mode)
{
    return (mode == VTSS_SERDES_MODE_DISABLE ? "DISABLE" : 
            mode == VTSS_SERDES_MODE_2G5 ? "2G5" : 
            mode == VTSS_SERDES_MODE_SGMII ? "SGMII" :
            mode == VTSS_SERDES_MODE_100FX ? "100FX" :
            mode == VTSS_SERDES_MODE_1000BaseX ? "1000BX" : "?");
}

/* Serdes1G: Read/write data */
static vtss_rc srvl_sd1g_read_write(vtss_state_t *vtss_state, u32 addr, BOOL write, u32 nsec)
{
    u32 data, mask;
    
    if (write)
        mask = VTSS_F_HSIO_MCB_SERDES1G_CFG_MCB_SERDES1G_ADDR_CFG_SERDES1G_WR_ONE_SHOT;
    else
        mask = VTSS_F_HSIO_MCB_SERDES1G_CFG_MCB_SERDES1G_ADDR_CFG_SERDES1G_RD_ONE_SHOT;

    SRVL_WR(VTSS_HSIO_MCB_SERDES1G_CFG_MCB_SERDES1G_ADDR_CFG,
           VTSS_F_HSIO_MCB_SERDES1G_CFG_MCB_SERDES1G_ADDR_CFG_SERDES1G_ADDR(addr) | mask);
    
    do { /* Wait until operation is completed  */
        SRVL_RD(VTSS_HSIO_MCB_SERDES1G_CFG_MCB_SERDES1G_ADDR_CFG, &data);
    } while (data & mask);

    if (nsec)
        VTSS_NSLEEP(nsec);
    
    return VTSS_RC_OK;
}

/* Serdes1G: Read data */
static vtss_rc srvl_sd1g_read(vtss_state_t *vtss_state, u32 addr) 
{
    return srvl_sd1g_read_write(vtss_state, addr, 0, 0);
}

/* Serdes1G: Write data */
static vtss_rc srvl_sd1g_write(vtss_state_t *vtss_state, u32 addr, u32 nsec) 
{
    return srvl_sd1g_read_write(vtss_state, addr, 1, nsec);
}

/* Wait 100 usec after some SerDes operations */
static vtss_rc srvl_sd1g_cfg(vtss_state_t *vtss_state, vtss_serdes_mode_t mode, u32 addr) 
{
    BOOL ena_lane = 1, if_100fx = 0, ena_dc_coupling = 0;
    u32  ob_amp_ctrl, cpmd_sel=0, mbtr_ctrl=2, des_bw_ana=6;

    VTSS_D("addr: 0x%x, mode: %s", addr, srvl_serdes_mode_txt(mode));
    
    switch (mode) {
    case VTSS_SERDES_MODE_SGMII:
        ob_amp_ctrl = 12;
        break;
    case VTSS_SERDES_MODE_100FX:
        ob_amp_ctrl = 12;
        if_100fx = 1;
        cpmd_sel = 2;
        mbtr_ctrl = 3;
        ena_dc_coupling = 1;
        des_bw_ana = 0;
        break;
    case VTSS_SERDES_MODE_1000BaseX:
        ob_amp_ctrl = 15;
        break;
    case VTSS_SERDES_MODE_DISABLE:
        ena_lane = 0;
        ob_amp_ctrl = 0;
        break;
    default:
        VTSS_E("Serdes1g mode %s not supported", srvl_serdes_mode_txt(mode));
        return VTSS_RC_ERROR;
    }
  
    /* 1. Configure macro, apply reset */
    /* IB_CFG */
    SRVL_WRM(VTSS_HSIO_SERDES1G_ANA_CFG_SERDES1G_IB_CFG,
             (if_100fx ? VTSS_F_HSIO_SERDES1G_ANA_CFG_SERDES1G_IB_CFG_IB_FX100_ENA : 0) |
             VTSS_F_HSIO_SERDES1G_ANA_CFG_SERDES1G_IB_CFG_IB_ENA_DETLEV |
             VTSS_F_HSIO_SERDES1G_ANA_CFG_SERDES1G_IB_CFG_IB_DET_LEV(3) |
             (ena_dc_coupling ? VTSS_F_HSIO_SERDES1G_ANA_CFG_SERDES1G_IB_CFG_IB_ENA_DC_COUPLING : 0) |
             VTSS_F_HSIO_SERDES1G_ANA_CFG_SERDES1G_IB_CFG_IB_EQ_GAIN(2),
             VTSS_F_HSIO_SERDES1G_ANA_CFG_SERDES1G_IB_CFG_IB_FX100_ENA |
             VTSS_F_HSIO_SERDES1G_ANA_CFG_SERDES1G_IB_CFG_IB_ENA_DETLEV |
             VTSS_M_HSIO_SERDES1G_ANA_CFG_SERDES1G_IB_CFG_IB_DET_LEV |
             VTSS_F_HSIO_SERDES1G_ANA_CFG_SERDES1G_IB_CFG_IB_ENA_DC_COUPLING |
             VTSS_M_HSIO_SERDES1G_ANA_CFG_SERDES1G_IB_CFG_IB_EQ_GAIN);

    /* DES_CFG */
    SRVL_WRM(VTSS_HSIO_SERDES1G_ANA_CFG_SERDES1G_DES_CFG,
             VTSS_F_HSIO_SERDES1G_ANA_CFG_SERDES1G_DES_CFG_DES_CPMD_SEL(cpmd_sel) |
             VTSS_F_HSIO_SERDES1G_ANA_CFG_SERDES1G_DES_CFG_DES_MBTR_CTRL(mbtr_ctrl) |
             VTSS_F_HSIO_SERDES1G_ANA_CFG_SERDES1G_DES_CFG_DES_BW_ANA(des_bw_ana),
             VTSS_M_HSIO_SERDES1G_ANA_CFG_SERDES1G_DES_CFG_DES_CPMD_SEL |
             VTSS_M_HSIO_SERDES1G_ANA_CFG_SERDES1G_DES_CFG_DES_MBTR_CTRL |
             VTSS_M_HSIO_SERDES1G_ANA_CFG_SERDES1G_DES_CFG_DES_BW_ANA);
    
    /* OB_CFG */
    SRVL_WRM(VTSS_HSIO_SERDES1G_ANA_CFG_SERDES1G_OB_CFG,
             VTSS_F_HSIO_SERDES1G_ANA_CFG_SERDES1G_OB_CFG_OB_AMP_CTRL(ob_amp_ctrl),
             VTSS_M_HSIO_SERDES1G_ANA_CFG_SERDES1G_OB_CFG_OB_AMP_CTRL);
    
    /* COMMON_CFG */
    SRVL_WRM(VTSS_HSIO_SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG,
             (ena_lane ? VTSS_F_HSIO_SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG_ENA_LANE : 0), 
             VTSS_F_HSIO_SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG_SYS_RST |
             VTSS_F_HSIO_SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG_ENA_LANE);

    /* PLL_CFG */
    SRVL_WRM(VTSS_HSIO_SERDES1G_ANA_CFG_SERDES1G_PLL_CFG,
             VTSS_F_HSIO_SERDES1G_ANA_CFG_SERDES1G_PLL_CFG_PLL_FSM_ENA |
             VTSS_F_HSIO_SERDES1G_ANA_CFG_SERDES1G_PLL_CFG_PLL_FSM_CTRL_DATA(200),
             VTSS_F_HSIO_SERDES1G_ANA_CFG_SERDES1G_PLL_CFG_PLL_FSM_ENA |
             VTSS_M_HSIO_SERDES1G_ANA_CFG_SERDES1G_PLL_CFG_PLL_FSM_CTRL_DATA);   
    
    /* MISC_CFG */
    SRVL_WRM(VTSS_HSIO_SERDES1G_DIG_CFG_SERDES1G_MISC_CFG,
             (if_100fx ? VTSS_F_HSIO_SERDES1G_DIG_CFG_SERDES1G_MISC_CFG_DES_100FX_CPMD_ENA : 0) |
             VTSS_F_HSIO_SERDES1G_DIG_CFG_SERDES1G_MISC_CFG_LANE_RST,
             VTSS_F_HSIO_SERDES1G_DIG_CFG_SERDES1G_MISC_CFG_DES_100FX_CPMD_ENA |
             VTSS_F_HSIO_SERDES1G_DIG_CFG_SERDES1G_MISC_CFG_LANE_RST);
    
    VTSS_RC(srvl_sd1g_write(vtss_state, addr, SRVL_SERDES_WAIT));
    
    /* 2. Release PLL reset */
    SRVL_WRM_SET(VTSS_HSIO_SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG,
                 VTSS_F_HSIO_SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG_SYS_RST);
    VTSS_RC(srvl_sd1g_write(vtss_state, addr, SRVL_SERDES_WAIT));
    
    /* 3. Release digital reset */
    SRVL_WRM(VTSS_HSIO_SERDES1G_DIG_CFG_SERDES1G_MISC_CFG, 0,
             VTSS_F_HSIO_SERDES1G_DIG_CFG_SERDES1G_MISC_CFG_LANE_RST);
    VTSS_RC(srvl_sd1g_write(vtss_state, addr, 0));
    
    return VTSS_RC_OK;
}


/* Serdes6G: Read/write data */
static vtss_rc srvl_sd6g_read_write(vtss_state_t *vtss_state, u32 addr, BOOL write, u32 nsec)
{
    u32 data, mask;

    if (write)
        mask = VTSS_F_HSIO_MCB_SERDES6G_CFG_MCB_SERDES6G_ADDR_CFG_SERDES6G_WR_ONE_SHOT;
    else
        mask = VTSS_F_HSIO_MCB_SERDES6G_CFG_MCB_SERDES6G_ADDR_CFG_SERDES6G_RD_ONE_SHOT;

    SRVL_WR(VTSS_HSIO_MCB_SERDES6G_CFG_MCB_SERDES6G_ADDR_CFG,
            VTSS_F_HSIO_MCB_SERDES6G_CFG_MCB_SERDES6G_ADDR_CFG_SERDES6G_ADDR(addr) | mask);
    
    do { /* Wait until operation is completed  */
        SRVL_RD(VTSS_HSIO_MCB_SERDES6G_CFG_MCB_SERDES6G_ADDR_CFG, &data);
    } while (data & mask);

    if (nsec)
        VTSS_NSLEEP(nsec);
    
    return VTSS_RC_OK;
}

/* Serdes6G: Read data */
static vtss_rc srvl_sd6g_read(vtss_state_t *vtss_state, u32 addr) 
{
    return srvl_sd6g_read_write(vtss_state, addr, 0, 0);
}

/* Serdes6G: Write data */
static vtss_rc srvl_sd6g_write(vtss_state_t *vtss_state, u32 addr, u32 nsec) 
{
    return srvl_sd6g_read_write(vtss_state, addr, 1, nsec);
}

/* Serdes6G setup (Disable/2G5/QSGMII/SGMII) */
#if defined(VTSS_FEATURE_SERDES_MACRO_SETTINGS)
static vtss_rc srvl_sd6g_cfg(vtss_state_t *vtss_state, vtss_serdes_mode_t mode, u32 addr)
{
    u32  ib_rf = 15, ctrl_data = 60, if_mode = 1, ob_ena_cas = 0, ob_lev = 48;
    u32  ib_vbac = 0, ib_vbcom = 0, ib_ic_ac=0, ib_c=15, ser_alisel=0, ob_post0 = 0;
    BOOL ena_lane = 1, rot_frq = 1, ena_rot = 0, qrate = 1, hrate = 0;
    BOOL ob_ena1v, if_100fx = 0, ser_enali = 0, ser_4tap_ena = 0, rot_dir = 0, ob_pol = 1, ib_chf=0;               ;

    VTSS_D("addr: 0x%x, mode: %s", addr, srvl_serdes_mode_txt(mode));
    ob_ena1v = (vtss_state->init_conf.serdes.serdes6g_vdd == VTSS_VDD_1V0) ? 1 : 0;

    switch (mode) {
    case VTSS_SERDES_MODE_2G5:
        ib_rf = ob_ena1v ? 2 : 10;   /* IB_CFG */
        ib_vbac = ob_ena1v ? 4 : 5;  /* IB_CFG */
        ib_ic_ac = ob_ena1v ? 2 : 0; /* IB_CFG */
        ib_vbcom = ob_ena1v ? 5 : 0; /* IB_CFG */
        ib_chf = ob_ena1v ? 1 : 0;   /* IB_CFG */
        ib_c = 10;                   /* IB_CFG */
        ena_rot = 1;                 /* PLL_CFG */
        ctrl_data = 48;              /* PLL_CFG */
        qrate = 0;                   /* COMMON_CFG */ 
        hrate = 1;                   /* COMMON_CFG */
        ob_lev = ob_ena1v ? 48 : 63; /* OB_CFG1 */
        ser_4tap_ena = ob_ena1v ? 1 : 0; /* SER_CFG */
        ser_enali = 1;                /* SER_CFG */
        ser_alisel = 1;               /* SER_CFG */
        break;
    case VTSS_SERDES_MODE_SGMII:
        ob_ena_cas = 2;
        break;
    case VTSS_SERDES_MODE_100FX:
        if_100fx = 1;
        break;
    case VTSS_SERDES_MODE_1000BaseX:
        ob_ena_cas = 2;
        break;
    case VTSS_SERDES_MODE_DISABLE:
        ena_lane = 0;
        break;
    default:
        VTSS_E("Serdes6g mode %s not supported", srvl_serdes_mode_txt(mode));
        return VTSS_RC_ERROR;
    }

    /* 1. Configure macro, apply reset */

    /* OB_CFG */      
    SRVL_WRM(VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_OB_CFG,
             VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_OB_CFG_OB_POST0(ob_post0) |
             (ob_pol ? VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_OB_CFG_OB_POL : 0) |
             (ob_ena1v ? VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_OB_CFG_OB_ENA1V_MODE : 0),
             VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_OB_CFG_OB_POST0 |
             VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_OB_CFG_OB_POL |
             VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_OB_CFG_OB_ENA1V_MODE);

    /* OB_CFG1 */      
    SRVL_WRM(VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_OB_CFG1,
             VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_OB_CFG1_OB_ENA_CAS(ob_ena_cas) |
             VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_OB_CFG1_OB_LEV(ob_lev),
             VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_OB_CFG1_OB_ENA_CAS |
             VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_OB_CFG1_OB_LEV);

    /* IB_CFG */      
    SRVL_WRM(VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG,
             VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_RT(15) | 
             VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_IC_AC(ib_ic_ac) |
             VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_VBAC(ib_vbac) | 
             VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_VBCOM(ib_vbcom) |
             VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_RF(ib_rf),
             VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_RT |
             VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_IC_AC |
             VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_VBAC |
             VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_VBCOM |
             VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_RF);

    /* IB_CFG1 */      
    SRVL_WRM(VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1,
             VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_C(ib_c) |
             (ib_chf ? VTSS_BIT(7) : 0 ) |
             VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_DIS_EQ |
             VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_ENA_OFFSAC |
             VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_ENA_OFFSDC |
             (if_100fx ? VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_FX100_ENA : 0) |
             VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_RST,
             VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_C |
             VTSS_BIT(7) |
             VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_DIS_EQ |
             VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_ENA_OFFSAC |
             VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_ENA_OFFSDC |
             VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_FX100_ENA |
             VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_RST);
    
    /* DES_CFG */      
    SRVL_WRM(VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_DES_CFG,
             (if_100fx ? VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_DES_CFG_DES_CPMD_SEL(2) : 0) |
             VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_DES_CFG_DES_BW_ANA(5),
             VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_DES_CFG_DES_CPMD_SEL |
             VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_DES_CFG_DES_BW_ANA);
    
    /* SER_CFG */      
    SRVL_WRM(VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_SER_CFG,
             (ser_4tap_ena ? VTSS_BIT(8) : 0) |
             VTSS_ENCODE_BITFIELD(ser_alisel,4,2) |
             (ser_enali ? VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_SER_CFG_SER_ENALI : 0),
             VTSS_BIT(8) |
             VTSS_ENCODE_BITMASK(4,2) |
             VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_SER_CFG_SER_ENALI);
    
    /* PLL_CFG */      
    SRVL_WRM(VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG,
             VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG_PLL_FSM_CTRL_DATA(ctrl_data) |
             VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG_PLL_FSM_ENA |
             (rot_dir ? VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG_PLL_ROT_DIR : 0) |
             (rot_frq ? VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG_PLL_ROT_FRQ : 0) |
             (ena_rot ? VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG_PLL_ENA_ROT : 0),
             VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG_PLL_FSM_CTRL_DATA |
             VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG_PLL_ROT_DIR |
             VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG_PLL_FSM_ENA |
             VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG_PLL_ROT_FRQ |
             VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG_PLL_ENA_ROT);

    /* Write masked to avoid changing RECO_SEL_* fields used by SyncE */
    /* COMMON_CFG */      
    SRVL_WRM(VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG,
             (ena_lane ? VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_ENA_LANE : 0) |
             (hrate ? VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_HRATE : 0) |
             (qrate ? VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_QRATE : 0) |
             VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_IF_MODE(if_mode),
             VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_SYS_RST |
             VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_ENA_LANE |
             VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_HRATE |
             VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_QRATE |
             VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_IF_MODE);
    
    /* MISC_CFG */      
    SRVL_WRM(VTSS_HSIO_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG,
             (if_100fx ? VTSS_F_HSIO_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG_DES_100FX_CPMD_ENA : 0) |
             VTSS_F_HSIO_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG_LANE_RST,
             VTSS_F_HSIO_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG_DES_100FX_CPMD_ENA |
             VTSS_F_HSIO_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG_LANE_RST);

    VTSS_RC(srvl_sd6g_write(vtss_state, addr, SRVL_SERDES_WAIT));

    /* 2. Release PLL reset */
    SRVL_WRM_SET(VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG, 
                 VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_SYS_RST);
    VTSS_RC(srvl_sd6g_write(vtss_state, addr, SRVL_SERDES_WAIT));
    
    /* 3. Release digital reset */
    SRVL_WRM_CLR(VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1,
                 VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_RST);

    SRVL_WRM(VTSS_HSIO_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG, 0,
             VTSS_F_HSIO_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG_LANE_RST);
    VTSS_RC(srvl_sd6g_write(vtss_state, addr, 0));

    return VTSS_RC_OK;
}
#endif

/* =================================================================
 *  PLL5G_RCOMP configuration 
 *  (Note that the HSIO:SYNC_ETH_CFG registers seems to be a part of this PLL)
 * =================================================================*/
/* PLL5G_RCOMP: Read/write data */
static vtss_rc srvl_pll5g_read_write(vtss_state_t *vtss_state, BOOL write, u32 nsec)
{
    u32 data, mask;
    
    if (write)
        mask = VTSS_F_HSIO_MCB_PLL5G_RCOMP_CFG_MCB_PLL5G_RCOMP_ADDR_CFG_PLL5G_RCOMP_WR_ONE_SHOT;
    else
        mask = VTSS_F_HSIO_MCB_PLL5G_RCOMP_CFG_MCB_PLL5G_RCOMP_ADDR_CFG_PLL5G_RCOMP_RD_ONE_SHOT;

    SRVL_WRM(VTSS_HSIO_MCB_PLL5G_RCOMP_CFG_MCB_PLL5G_RCOMP_ADDR_CFG, 
             mask, mask);
    do { /* Wait until operation is completed  */
        SRVL_RD(VTSS_HSIO_MCB_PLL5G_RCOMP_CFG_MCB_PLL5G_RCOMP_ADDR_CFG, &data);
    } while (data & mask);
    
    if (nsec)
        VTSS_NSLEEP(nsec);
    
    return VTSS_RC_OK;
}

/* PLL5G_RCOMP: Read data */
static vtss_rc srvl_pll5g_read(vtss_state_t *vtss_state) 
{
    return srvl_pll5g_read_write(vtss_state, 0, 0);
}

/* PLL5G_RCOMP: Write data */
static vtss_rc srvl_pll5g_write(vtss_state_t *vtss_state, u32 nsec) 
{
    return srvl_pll5g_read_write(vtss_state, 1, nsec);
}

/* ================================================================= *
 *  SYNCE (Level 1 syncronization)
 * ================================================================= */

static vtss_rc srvl_serdes_inst_get(u32 port, u32 *inst, BOOL *serdes6g)
{
    if (port >= VTSS_CHIP_PORTS) {
        VTSS_E("illegal port: %u", port);
        return VTSS_RC_ERROR;
    }

    if (port == 8 || port == 9) {
        /* SERDES6G, instance 0-1 */
        *serdes6g = 1;
        *inst = (port - 8);
    } else {
        /* SERDES1G, instance 0-8 */
        *serdes6g = 0;
        *inst = (port == 10 ? 8 : port);
    }

    return VTSS_RC_OK;
}

// Clock A output maps to Serval RCVRD_CLK0
// Use register RECO_CLK_SEL0 to select any port as source
// All individual Clock divider registers are set to 'divider'
// Clock B output maps to Serval RCVRD_CLK1.9 (configured by a separate function)
// Use common clockbus to select any port as source
// Clock divider register is the common bus divider
// clk_port = 0 => CLK_A, clk_port = 1 => CLK_B
static vtss_rc srvl_synce_clock_out_set(vtss_state_t *vtss_state, const u32 clk_port)
{
    vtss_synce_clock_out_t *conf = &vtss_state->synce.out_conf[clk_port];
    u32 div_mask;
    BOOL        serdes6g;
    u32         serdes_instance;
    i32         chip_port;

    switch (conf->divider) {
        case VTSS_SYNCE_DIVIDER_1: div_mask = 0; break;
        case VTSS_SYNCE_DIVIDER_4: div_mask = 2; break;
        case VTSS_SYNCE_DIVIDER_5: div_mask = 1; break;
        default: div_mask = 0; break;
    }

    VTSS_I("clk_port %d divider %u  enable %u", clk_port, conf->divider, conf->enable);
    if (clk_port == 0) {    // clkport A
        // set individual dividers for 1G and 6G serdes's
        for (chip_port = 0; chip_port < VTSS_CHIP_PORTS ; chip_port++) {
            if (VTSS_RC_OK == srvl_serdes_inst_get(chip_port, &serdes_instance, &serdes6g)) {
                VTSS_I("serdes_instance %d", serdes_instance);
                
                if (serdes6g) {
                    VTSS_RC(srvl_sd6g_read(vtss_state, 1<<serdes_instance));
                    SRVL_WRM(VTSS_HSIO_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG,
                             VTSS_F_HSIO_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG_SEL_RECO_CLK(div_mask),
                             VTSS_M_HSIO_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG_SEL_RECO_CLK);
                    VTSS_RC(srvl_sd6g_write(vtss_state, 1<<serdes_instance, 0));
                } else {
                    VTSS_RC(srvl_sd1g_read(vtss_state, 1<<serdes_instance)); /* Readback the 1G common config register */
                    SRVL_WRM(VTSS_HSIO_SERDES1G_DIG_CFG_SERDES1G_MISC_CFG,
                             VTSS_F_HSIO_SERDES1G_DIG_CFG_SERDES1G_MISC_CFG_SEL_RECO_CLK(div_mask),
                             VTSS_M_HSIO_SERDES1G_DIG_CFG_SERDES1G_MISC_CFG_SEL_RECO_CLK);
                    VTSS_RC(srvl_sd1g_write(vtss_state, 1<<serdes_instance, 0));
                }
            } else {
                VTSS_E("chip_port %d has no serdes", chip_port);
            }
        }
    } else {                // clkport B
        // set bus divider
        VTSS_RC(srvl_pll5g_read(vtss_state));
        SRVL_WRM(VTSS_HSIO_SYNC_ETH_CFG_SYNC_ETH_CFG2, VTSS_F_HSIO_SYNC_ETH_CFG_SYNC_ETH_CFG2_SEL_RECO_CLK_B(div_mask),
                 VTSS_M_HSIO_SYNC_ETH_CFG_SYNC_ETH_CFG2_SEL_RECO_CLK_B);
        VTSS_RC(srvl_pll5g_write(vtss_state, 0));
    }
    return VTSS_RC_OK;
}

#define rcvrd_clk_b_output_pin 1        // clock selector uses pin 1

// Clock A output maps to Serval RCVRD_CLK0
// Use register RECO_CLK_ENA to enable the output (individual for all ports)
// Clock B output maps to Serval RCVRD_CLK1..9 (configured by a separate function)
// Use common clockbus to select any port as source (output B is activated for the selected port
// If Clock B outout is disabled, all CLK1..9 outputs are set to individual.
// clk_port = 0 => CLK_A, clk_port = 1 => CLK_B
static vtss_rc srvl_synce_clock_in_set(vtss_state_t *vtss_state, const u32 clk_port)
{
    vtss_synce_clock_in_t *conf = &vtss_state->synce.in_conf[clk_port];
    BOOL                  serdes6g;
    u32                   serdes_instance, common_cfg = 0, common_mask, mask, en_mask, sq_mask;
    i32                   new_chip_port = VTSS_CHIP_PORT(conf->port_no);
    u32                   old_chip_port = vtss_state->synce.old_port_no[clk_port];

    VTSS_I("clk_port %d new_chip_port %u  old_chip_port %d enable %d squelch %d",
           clk_port, new_chip_port, old_chip_port, conf->enable, conf->squelsh);
    
    if ((!conf->enable || (new_chip_port != old_chip_port)) && old_chip_port != 0xFFFFFFFF) {
        /* Disable of this clock port or input port has changed for this clock output port - disable old input */
        if (VTSS_RC_OK == srvl_serdes_inst_get(old_chip_port, &serdes_instance, &serdes6g)) {
            VTSS_I("serdes_instance %d", serdes_instance);
            
            mask = (1<<serdes_instance);
            if (serdes6g) {
                VTSS_RC(srvl_sd6g_read(vtss_state, mask));
                common_mask = (clk_port ? VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_RECO_SEL_B :
                               VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_RECO_SEL_A);
                VTSS_I("disable 6G   instance %u  common_mask %X", serdes_instance, common_mask);
                SRVL_WRM(VTSS_HSIO_SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG, common_cfg, common_mask);
                VTSS_RC(srvl_sd6g_write(vtss_state, mask, 0));
            } else {
                VTSS_RC(srvl_sd1g_read(vtss_state, mask)); /* Readback the 1G common config register */
                common_mask = (clk_port ? VTSS_F_HSIO_SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG_RECO_SEL_B :
                               VTSS_F_HSIO_SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG_RECO_SEL_A);
                VTSS_I("disable 1G   instance %u  common_mask %X", serdes_instance, common_mask);
                SRVL_WRM(VTSS_HSIO_SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG, common_cfg, common_mask);
                VTSS_RC(srvl_sd1g_write(vtss_state, mask, 0));
            }
        } else {
            VTSS_E("could not get serdes instance for port %d", old_chip_port);
        }
    }
    
    if (conf->enable) {
        /* Enable input clock configuration - now configuring the new (or maybe the same) input port */
        if (VTSS_RC_OK == srvl_serdes_inst_get(new_chip_port, &serdes_instance, &serdes6g)) {
            mask = (1<<serdes_instance);
            if (serdes6g) {
                en_mask = (clk_port ? VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_RECO_SEL_B :
                           VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_RECO_SEL_A);
                sq_mask = (clk_port ? VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_SE_AUTO_SQUELCH_B_ENA :
                           VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_SE_AUTO_SQUELCH_A_ENA);
                common_mask = (en_mask | sq_mask);
                common_cfg = (en_mask | (conf->squelsh ? sq_mask : 0));
                VTSS_RC(srvl_sd6g_read(vtss_state, mask)); /* Readback the 6G common config register */
                VTSS_I("enable 6G   instance %u  common_cfg %X", serdes_instance, common_cfg);
                SRVL_WRM(VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG, common_cfg, common_mask);
                VTSS_RC(srvl_sd6g_write(vtss_state, mask, 0)); /* transfer 6G common config register */
            } else {
                en_mask = (clk_port ? VTSS_F_HSIO_SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG_RECO_SEL_B :
                           VTSS_F_HSIO_SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG_RECO_SEL_A);
                sq_mask = (clk_port ? VTSS_F_HSIO_SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG_SE_AUTO_SQUELCH_B_ENA :
                           VTSS_F_HSIO_SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG_SE_AUTO_SQUELCH_A_ENA);
                common_mask = (en_mask | sq_mask);
                common_cfg = (en_mask | (conf->squelsh ? sq_mask : 0));
                VTSS_RC(srvl_sd1g_read(vtss_state, mask)); /* Readback the 1G common config register */
                VTSS_I("enable 1G   instance %u  common_cfg %X", serdes_instance, common_cfg);
                SRVL_WRM(VTSS_HSIO_SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG, common_cfg, common_mask);
                VTSS_RC(srvl_sd1g_write(vtss_state, mask, 0));     /* transfer 1G common config register */
            }
            vtss_state->synce.old_port_no[clk_port] = new_chip_port;
        } else {
            VTSS_E("could not get serdes instance for port %d", new_chip_port);
        }
    } else {  // port disabled
        vtss_state->synce.old_port_no[clk_port] = 0xFFFFFFFF;
    }
    /* Need to write to this register to copy SYNC_ETH_CFG registers to shaddow reg */
    VTSS_RC(srvl_pll5g_read(vtss_state));

    if (clk_port == 0) {    // clkport A
        // set Selector (SEL0) for A output
        SRVL_WRM(VTSS_HSIO_SYNC_ETH_CFG_SYNC_ETH_CFG1,
                 VTSS_F_HSIO_SYNC_ETH_CFG_SYNC_ETH_CFG1_RECO_CLK_SEL0(new_chip_port),
                 VTSS_M_HSIO_SYNC_ETH_CFG_SYNC_ETH_CFG1_RECO_CLK_SEL0);
        // clear Selector (SELx) for selected port A output
        SRVL_WRM(VTSS_HSIO_SYNC_ETH_CFG_SYNC_ETH_CFG1, 0,
                 VTSS_F_HSIO_SYNC_ETH_CFG_SYNC_ETH_CFG1_RECO_CLK_SEL(1<<new_chip_port));
        
    } else {                // clkport B
        // set Selector (SELx) for port B output
        SRVL_WRM(VTSS_HSIO_SYNC_ETH_CFG_SYNC_ETH_CFG1, 1,
                 VTSS_F_HSIO_SYNC_ETH_CFG_SYNC_ETH_CFG1_RECO_CLK_SEL(1<<rcvrd_clk_b_output_pin));
    }
    // enable output
    mask = VTSS_F_HSIO_SYNC_ETH_CFG_SYNC_ETH_CFG0_RECO_CLK_ENA(1<<clk_port);
    SRVL_WRM(VTSS_HSIO_SYNC_ETH_CFG_SYNC_ETH_CFG0, conf->enable ? mask : 0, mask);

    /* Need to write to this register to transfer SYNC_ETH_CFG registers to HW */
    VTSS_RC(srvl_pll5g_write(vtss_state, 0));
    
    return VTSS_RC_OK;
}

/* ================================================================= *
 *  Port control
 * ================================================================= */

vtss_rc vtss_srvl_port_max_tags_set(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
    vtss_port_max_tags_t  max_tags = vtss_state->port.conf[port_no].max_tags;
    vtss_vlan_port_type_t vlan_type = vtss_state->l2.vlan_port_conf[port_no].port_type;
    u32                   port = VTSS_CHIP_PORT(port_no);
    u32                   tgt = VTSS_TO_DEV(port);
    u32                   etype, double_ena, single_ena;

    /* S-ports and VLAN unaware ports both support 0x88a8 (in addition to 0x8100) */
    etype = (vlan_type == VTSS_VLAN_PORT_TYPE_S_CUSTOM ? vtss_state->l2.vlan_conf.s_etype :
             vlan_type == VTSS_VLAN_PORT_TYPE_C ? VTSS_ETYPE_TAG_C : VTSS_ETYPE_TAG_S);
    single_ena = (max_tags == VTSS_PORT_MAX_TAGS_NONE ? 0 : 1);
    double_ena = (max_tags == VTSS_PORT_MAX_TAGS_TWO ? 1 : 0);
    
    SRVL_WR(VTSS_DEV_MAC_CFG_STATUS_MAC_TAGS_CFG(tgt), 
            VTSS_F_DEV_MAC_CFG_STATUS_MAC_TAGS_CFG_TAG_ID(etype) |
            (double_ena ? VTSS_F_DEV_MAC_CFG_STATUS_MAC_TAGS_CFG_VLAN_DBL_AWR_ENA : 0) |
            (single_ena ? VTSS_F_DEV_MAC_CFG_STATUS_MAC_TAGS_CFG_VLAN_AWR_ENA : 0) |
            VTSS_F_DEV_MAC_CFG_STATUS_MAC_TAGS_CFG_VLAN_LEN_AWR_ENA);
    return VTSS_RC_OK;
}

static vtss_rc srvl_miim_read_write(vtss_state_t *vtss_state,
                                    BOOL read, 
                                    u32 miim_controller, 
                                    u8 miim_addr, 
                                    u8 addr, 
                                    u16 *value, 
                                    BOOL report_errors)
{
    u32 data;

    /* Enqueue MIIM operation to be executed */
    SRVL_WR(VTSS_DEVCPU_GCB_MIIM_MII_CMD(miim_controller), 
            VTSS_F_DEVCPU_GCB_MIIM_MII_CMD_MIIM_CMD_VLD |
            VTSS_F_DEVCPU_GCB_MIIM_MII_CMD_MIIM_CMD_PHYAD(miim_addr) |
            VTSS_F_DEVCPU_GCB_MIIM_MII_CMD_MIIM_CMD_REGAD(addr) |
            VTSS_F_DEVCPU_GCB_MIIM_MII_CMD_MIIM_CMD_WRDATA(*value) |
            VTSS_F_DEVCPU_GCB_MIIM_MII_CMD_MIIM_CMD_OPR_FIELD(read ? 2 : 1));
    
    /* Wait for MIIM operation to finish */
    do {
        SRVL_RD(VTSS_DEVCPU_GCB_MIIM_MII_STATUS(miim_controller), &data);
    } while (data & VTSS_F_DEVCPU_GCB_MIIM_MII_STATUS_MIIM_STAT_BUSY);
    
    if (read) {
        SRVL_RD(VTSS_DEVCPU_GCB_MIIM_MII_DATA(miim_controller), &data);
        if (data & VTSS_F_DEVCPU_GCB_MIIM_MII_DATA_MIIM_DATA_SUCCESS(3))
            return VTSS_RC_ERROR;
        *value = VTSS_X_DEVCPU_GCB_MIIM_MII_DATA_MIIM_DATA_RDDATA(data);
    }
    return VTSS_RC_OK;
}

static vtss_rc srvl_miim_read(vtss_state_t *vtss_state,
                              vtss_miim_controller_t miim_controller, 
                              u8 miim_addr, 
                              u8 addr, 
                              u16 *value, 
                              BOOL report_errors)
{
    return srvl_miim_read_write(vtss_state, TRUE, miim_controller, miim_addr, addr, value, report_errors);
}

static vtss_rc srvl_miim_write(vtss_state_t *vtss_state,
                               vtss_miim_controller_t miim_controller, 
                               u8 miim_addr, 
                               u8 addr, 
                               u16 value, 
                               BOOL report_errors)
{
    return srvl_miim_read_write(vtss_state, FALSE, miim_controller, miim_addr, addr, &value, report_errors);
}

static vtss_rc srvl_port_fc_setup(vtss_state_t *vtss_state, u32 port, vtss_port_conf_t *conf)
{
    u8                *mac;
    u32               pause_start, pause_stop, rsrv_raw, rsrv_total = 0, atop_wm, link_speed;
    u32               tgt = VTSS_TO_DEV(port), rsrv_raw_fc_jumbo, rsrv_raw_no_fc_jumbo, rsrv_raw_fc_no_jumbo;
    BOOL              fc = conf->flow_control.generate;
    vtss_port_speed_t speed = conf->speed;
    vtss_port_no_t    port_no;

    pause_start = 0x7ff;
    pause_stop  = 0xfff;
    rsrv_raw_fc_jumbo = 40000;
    rsrv_raw_no_fc_jumbo = 12000;
    rsrv_raw_fc_no_jumbo = 13662; /* 9 x 1518 */

    if (conf->max_frame_length > VTSS_MAX_FRAME_LENGTH_STANDARD) {
        pause_start = 228;     /* 9 x 1518 / 60 */
        if (fc) {
            /* FC and jumbo enabled*/
            pause_stop  = 177;     /* 7 x 1518 / 60 */
            rsrv_raw    = rsrv_raw_fc_jumbo;   
        } else {
            /* FC disabled, jumbo enabled */
            rsrv_raw = rsrv_raw_no_fc_jumbo;
        }
    } else {
        pause_start = 152;    /* 6 x 1518 / 60 */
        if (fc) {
        /* FC enabled, jumbo disabled */
            pause_stop  = 101;    /* 4 x 1518 / 60 */
            rsrv_raw    = rsrv_raw_fc_no_jumbo;  
        } else {
            rsrv_raw    = 0;
        }
    } 

    /* Set Pause WM hysteresis*/
    SRVL_WR(VTSS_SYS_PAUSE_CFG_PAUSE_CFG(port),
            VTSS_F_SYS_PAUSE_CFG_PAUSE_CFG_PAUSE_START(pause_start) |
            VTSS_F_SYS_PAUSE_CFG_PAUSE_CFG_PAUSE_STOP(pause_stop) |
            (fc ? VTSS_F_SYS_PAUSE_CFG_PAUSE_CFG_PAUSE_ENA : 0));
    
    /* Set SMAC of Pause frame */
    mac = &conf->flow_control.smac.addr[0];
    SRVL_WR(VTSS_DEV_MAC_CFG_STATUS_MAC_FC_MAC_HIGH_CFG(tgt), (mac[0]<<16) | (mac[1]<<8) | mac[2]);
    SRVL_WR(VTSS_DEV_MAC_CFG_STATUS_MAC_FC_MAC_LOW_CFG(tgt), (mac[3]<<16) | (mac[4]<<8) | mac[5]);

    /* Enable/disable FC incl. pause value and zero pause */
    link_speed = (speed == VTSS_SPEED_10M ? 3 :
                  speed == VTSS_SPEED_100M ? 2 :
                  speed == VTSS_SPEED_1G ? 1 : 0);
    SRVL_WR(VTSS_SYS_PAUSE_CFG_MAC_FC_CFG(port),
            VTSS_F_SYS_PAUSE_CFG_MAC_FC_CFG_FC_LINK_SPEED(link_speed) |
            VTSS_F_SYS_PAUSE_CFG_MAC_FC_CFG_FC_LATENCY_CFG(7) |
            VTSS_F_SYS_PAUSE_CFG_MAC_FC_CFG_ZERO_PAUSE_ENA |
            (conf->flow_control.obey ? VTSS_F_SYS_PAUSE_CFG_MAC_FC_CFG_RX_FC_ENA : 0) |
            VTSS_F_SYS_PAUSE_CFG_MAC_FC_CFG_PAUSE_VAL_CFG(0xffff));


    SRVL_WRM(VTSS_QSYS_SYSTEM_SWITCH_PORT_MODE(port),
             fc ? 0 : VTSS_F_QSYS_SYSTEM_SWITCH_PORT_MODE_INGRESS_DROP_MODE,
             VTSS_F_QSYS_SYSTEM_SWITCH_PORT_MODE_INGRESS_DROP_MODE);

  
    /* Calculate the total reserved space for all ports */
    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
        conf = &vtss_state->port.conf[port_no];
        fc = conf->flow_control.generate;
        if (conf->max_frame_length > VTSS_MAX_FRAME_LENGTH_STANDARD) {
            if (fc) {
                rsrv_total += rsrv_raw_fc_jumbo; 
            } else {
                rsrv_total += rsrv_raw_no_fc_jumbo;
            }
        } else if (fc) {
            rsrv_total += rsrv_raw_fc_no_jumbo; 
        }
    }
 
    atop_wm = (SRVL_BUFFER_MEMORY - rsrv_total)/SRVL_BUFFER_CELL_SZ;
    if (atop_wm >= 2048) {
        atop_wm = 2048 + atop_wm/16;
    }

    /*  When 'port ATOP' and 'common ATOP_TOT' are exceeded, tail dropping is activated on port */
    SRVL_WR(VTSS_SYS_PAUSE_CFG_ATOP_TOT_CFG, atop_wm);
    SRVL_WR(VTSS_SYS_PAUSE_CFG_ATOP(port), rsrv_raw/SRVL_BUFFER_CELL_SZ);

    return VTSS_RC_OK;
}

static vtss_rc srvl_port_conf_get(vtss_state_t *vtss_state,
                                  const vtss_port_no_t port_no, vtss_port_conf_t *const conf)
{
    u32 port = VTSS_CHIP_PORT(port_no);
    u32 tgt = VTSS_TO_DEV(port);
    u32 value, link_speed;

    /* Speed */
    SRVL_RD(VTSS_DEV_PORT_MODE_CLOCK_CFG(tgt), &value);
    link_speed = VTSS_X_DEV_PORT_MODE_CLOCK_CFG_LINK_SPEED(value);
    conf->speed = (link_speed == 3 ? VTSS_SPEED_10M :
                   link_speed == 2 ? VTSS_SPEED_100M :
                   vtss_state->port.conf[port_no].if_type == VTSS_PORT_INTERFACE_VAUI ?
                   VTSS_SPEED_2500M : VTSS_SPEED_1G);

    /* Duplex */
    SRVL_RD(VTSS_DEV_MAC_CFG_STATUS_MAC_MODE_CFG(tgt), &value);
    conf->fdx = VTSS_BOOL(value & VTSS_F_DEV_MAC_CFG_STATUS_MAC_MODE_CFG_FDX_ENA);

    /* Power down */
    SRVL_RD(VTSS_DEV_MAC_CFG_STATUS_MAC_ENA_CFG(tgt), &value);
    conf->power_down = (value & VTSS_F_DEV_MAC_CFG_STATUS_MAC_ENA_CFG_RX_ENA ? 0 : 1);

    /* Flow control */
    SRVL_RD(VTSS_SYS_PAUSE_CFG_PAUSE_CFG(port), &value);
    conf->flow_control.generate = VTSS_BOOL(value & VTSS_F_SYS_PAUSE_CFG_PAUSE_CFG_PAUSE_ENA);
    SRVL_RD(VTSS_SYS_PAUSE_CFG_MAC_FC_CFG(port), &value);
    conf->flow_control.obey = VTSS_BOOL(value & VTSS_F_SYS_PAUSE_CFG_MAC_FC_CFG_RX_FC_ENA);

    return VTSS_RC_OK;
}

vtss_rc vtss_srvl_port_policer_fc_set(vtss_state_t *vtss_state, const vtss_port_no_t port_no)
{
    vtss_port_conf_t     *port_conf = &vtss_state->port.conf[port_no];
    vtss_qos_port_conf_t *qos_conf = &vtss_state->qos.port_conf[port_no];
    u32                  port = VTSS_CHIP_PORT(port_no);

    SRVL_WR(VTSS_ANA_POL_MISC_POL_FLOWC(port),
            (port_conf->flow_control.generate &&
             (qos_conf->policer_port[0].rate != VTSS_BITRATE_DISABLED) &&
             qos_conf->policer_ext_port[0].flow_control) ? 1 : 0);

    return VTSS_RC_OK;
}

static vtss_rc srvl_port_conf_set(vtss_state_t *vtss_state, const vtss_port_no_t port_no)
{
    vtss_port_conf_t       *conf = &vtss_state->port.conf[port_no];
    u32                    port = VTSS_CHIP_PORT(port_no);
    u32                    link_speed, value, tgt = VTSS_TO_DEV(port), inst, delay = 0, addr;
    vtss_port_frame_gaps_t gaps;
    vtss_port_speed_t      speed = conf->speed;
    BOOL                   fdx = conf->fdx, disable = conf->power_down;
    BOOL                   sgmii = 0, if_100fx = 0, serdes6g;
    vtss_serdes_mode_t     mode = VTSS_SERDES_MODE_SGMII;   
   
    /* Verify speed and interface type */
    switch (speed) {
    case VTSS_SPEED_10M:
        link_speed = 3;
        break;
    case VTSS_SPEED_100M:
        link_speed = 2;
        break;
    case VTSS_SPEED_1G:
    case VTSS_SPEED_2500M:
        link_speed = 1;
        break;
    default:
        VTSS_E("illegal speed:%d, port %u", speed, port);
        return VTSS_RC_ERROR;
    }

    switch (conf->if_type) {
    case VTSS_PORT_INTERFACE_NO_CONNECTION:
        disable = 1;
        break;
    case VTSS_PORT_INTERFACE_INTERNAL:
    case VTSS_PORT_INTERFACE_SGMII:
        sgmii = 1;
        break;
    case VTSS_PORT_INTERFACE_SERDES:
        if (speed != VTSS_SPEED_1G) {
            VTSS_E("illegal speed, port %u", port);
            return VTSS_RC_ERROR;
        }
        mode = VTSS_SERDES_MODE_1000BaseX;
        break;
    case VTSS_PORT_INTERFACE_100FX:
        if (speed != VTSS_SPEED_100M) {
            VTSS_E("illegal speed, port %u", port);
            return VTSS_RC_ERROR;
        }
        mode = VTSS_SERDES_MODE_100FX;
        if_100fx = 1;      
        break;
    case VTSS_PORT_INTERFACE_SGMII_CISCO:
        if (speed != VTSS_SPEED_10M && speed != VTSS_SPEED_100M && speed != VTSS_SPEED_1G) {
            VTSS_E("SFP_CU, illegal speed, port %u", port);
            return VTSS_RC_ERROR;
        }
        mode = VTSS_SERDES_MODE_1000BaseX;
        sgmii = 1;        
        break;
    case VTSS_PORT_INTERFACE_VAUI:
        if (speed != VTSS_SPEED_1G && speed != VTSS_SPEED_2500M) {
            VTSS_E("illegal speed, port %u", port);
            return VTSS_RC_ERROR;
        }
        mode = (speed == VTSS_SPEED_2500M ? VTSS_SERDES_MODE_2G5 : VTSS_SERDES_MODE_1000BaseX);
        break;
    default:
        VTSS_E("illegal interface, port %u", port);
        return VTSS_RC_ERROR;
    }

    /* Configure the Serdes macros to 100FX / 1000BaseX / 2500 */
    if (mode != vtss_state->port.serdes_mode[port_no]) {
        vtss_state->port.serdes_mode[port_no] = mode;
        VTSS_RC(srvl_serdes_inst_get(port, &inst, &serdes6g));
        addr = (1 << inst);
        if (serdes6g) {
            VTSS_RC(srvl_sd6g_read(vtss_state, addr));
            VTSS_MSLEEP(10);
#if defined(VTSS_FEATURE_SERDES_MACRO_SETTINGS)
            VTSS_RC(srvl_sd6g_cfg(vtss_state, mode, addr));
#endif
            VTSS_RC(srvl_sd6g_write(vtss_state, addr, SRVL_SERDES_WAIT));
        } else {
            VTSS_RC(srvl_sd1g_read(vtss_state, addr));
            VTSS_MSLEEP(10);
            VTSS_RC(srvl_sd1g_cfg(vtss_state, mode, addr));
            VTSS_RC(srvl_sd1g_write(vtss_state, addr, SRVL_SERDES_WAIT));
        }
        VTSS_MSLEEP(1);          
    }
    /* Port disable and flush procedure: */
    if (vtss_state->misc.chip_id.revision == 0) {
        /* 0.1: Reset the PCS - only needed for Rev A */
        SRVL_WRM_SET(VTSS_DEV_PORT_MODE_CLOCK_CFG(tgt),
                     VTSS_F_DEV_PORT_MODE_CLOCK_CFG_PCS_RX_RST);
    }

    /* 1: Disable MAC frame reception */
    SRVL_WRM_CLR(VTSS_DEV_MAC_CFG_STATUS_MAC_ENA_CFG(tgt),
                 VTSS_F_DEV_MAC_CFG_STATUS_MAC_ENA_CFG_RX_ENA);

#if defined(VTSS_FEATURE_AFI_SWC)
    /* Pause all AFI-generated frames to this port. */
    VTSS_RC(srvl_afi_pause_resume(vtss_state, port_no, FALSE));
#endif /* VTSS_FEATURE_AFI_SWC */

    /* 2: Disable traffic being sent to or from switch port */
    SRVL_WRM_CLR(VTSS_QSYS_SYSTEM_SWITCH_PORT_MODE(port),
                 VTSS_F_QSYS_SYSTEM_SWITCH_PORT_MODE_PORT_ENA);

    /* 3: Disable dequeuing from the egress queues *\/ */
    SRVL_WRM_SET(VTSS_QSYS_SYSTEM_PORT_MODE(port),
                VTSS_F_QSYS_SYSTEM_PORT_MODE_DEQUEUE_DIS);

    /* 4: Disable Flowcontrol */
    SRVL_WRM_CLR(VTSS_SYS_PAUSE_CFG_PAUSE_CFG(port),  VTSS_F_SYS_PAUSE_CFG_PAUSE_CFG_PAUSE_ENA);

    /* 5: Wait a worst case time 8ms (jumbo/10Mbit) *\/ */
    VTSS_MSLEEP(8);

    /* 6: Disable HDX backpressure (Bugzilla 3203) */
    SRVL_WRM_CLR(VTSS_SYS_SYSTEM_FRONT_PORT_MODE(port), 
                 VTSS_F_SYS_SYSTEM_FRONT_PORT_MODE_HDX_MODE);
   
    /* 7: Flush the queues accociated with the port */
    SRVL_WRM_SET(VTSS_REW_PORT_PORT_CFG(port), 
                 VTSS_F_REW_PORT_PORT_CFG_FLUSH_ENA);

    /* 8: Enable dequeuing from the egress queues */
    SRVL_WRM_CLR(VTSS_QSYS_SYSTEM_PORT_MODE(port),
                VTSS_F_QSYS_SYSTEM_PORT_MODE_DEQUEUE_DIS);

    do { /* 9: Wait until flushing is complete */
        SRVL_RD(VTSS_QSYS_SYSTEM_SW_STATUS(port), &value);
        VTSS_MSLEEP(1);            
        delay++;
        if (delay == 2000) {
            VTSS_E("Flush timeout port %u\n",port);
            break;
        }
    } while (value & VTSS_M_QSYS_SYSTEM_SW_STATUS_EQ_AVAIL);

    /* 10: Reset the clock */
    SRVL_WR(VTSS_DEV_PORT_MODE_CLOCK_CFG(tgt), 
            VTSS_F_DEV_PORT_MODE_CLOCK_CFG_MAC_TX_RST |
            VTSS_F_DEV_PORT_MODE_CLOCK_CFG_MAC_RX_RST |
            VTSS_F_DEV_PORT_MODE_CLOCK_CFG_PORT_RST);

    /* 11: Clear flushing */
    SRVL_WRM_CLR(VTSS_REW_PORT_PORT_CFG(port), VTSS_F_REW_PORT_PORT_CFG_FLUSH_ENA);

    /* The port is disabled and flushed, now set up the port in the new operating mode */

    /* Bugzilla 4388: disabling frame aging when in HDX */
    SRVL_WRM_CTL(VTSS_REW_PORT_PORT_CFG(port), fdx, VTSS_F_REW_PORT_PORT_CFG_AGE_DIS);

    /* GIG/FDX mode */
    if (fdx) {
        value = VTSS_F_DEV_MAC_CFG_STATUS_MAC_MODE_CFG_FDX_ENA;
        if (speed == VTSS_SPEED_1G || speed == VTSS_SPEED_2500M) {
            value |= VTSS_F_DEV_MAC_CFG_STATUS_MAC_MODE_CFG_GIGA_MODE_ENA;
        }
    } else {        
        SRVL_WRM_SET(VTSS_SYS_SYSTEM_FRONT_PORT_MODE(port), 
                     VTSS_F_SYS_SYSTEM_FRONT_PORT_MODE_HDX_MODE);
        value = conf->flow_control.obey ? 0x100 : 0; /* FC_WORD_SYNC_ENA=1 for HDX/FC for Rev B chip */
    }
    SRVL_WR(VTSS_DEV_MAC_CFG_STATUS_MAC_MODE_CFG(tgt), value);
    

    /* Default gaps */
    gaps.fdx_gap = 15;
    gaps.hdx_gap_1 = 0;
    gaps.hdx_gap_2 = 0;
    if (speed == VTSS_SPEED_1G || speed == VTSS_SPEED_2500M) {
        gaps.fdx_gap = 5;
    } else if (!fdx) {
        gaps.hdx_gap_1 = (speed == VTSS_SPEED_100M ? 7 : 11);
        gaps.hdx_gap_2 = 9;
    }

    /* Non default gaps */
    if (conf->frame_gaps.hdx_gap_1 != VTSS_FRAME_GAP_DEFAULT) 
        gaps.hdx_gap_1 = conf->frame_gaps.hdx_gap_1;
    if (conf->frame_gaps.hdx_gap_2 != VTSS_FRAME_GAP_DEFAULT)
        gaps.hdx_gap_2 = conf->frame_gaps.hdx_gap_2;
    if (conf->frame_gaps.fdx_gap != VTSS_FRAME_GAP_DEFAULT)
        gaps.fdx_gap = conf->frame_gaps.fdx_gap;
    
    /* Set MAC IFG Gaps */
    SRVL_WR(VTSS_DEV_MAC_CFG_STATUS_MAC_IFG_CFG(tgt), 
            VTSS_F_DEV_MAC_CFG_STATUS_MAC_IFG_CFG_TX_IFG(gaps.fdx_gap) |
            VTSS_F_DEV_MAC_CFG_STATUS_MAC_IFG_CFG_RX_IFG1(gaps.hdx_gap_1) |
            VTSS_F_DEV_MAC_CFG_STATUS_MAC_IFG_CFG_RX_IFG2(gaps.hdx_gap_2));
    
    /* Load seed and set MAC HDX late collision */
    value = ((conf->exc_col_cont ? 
              VTSS_F_DEV_MAC_CFG_STATUS_MAC_HDX_CFG_RETRY_AFTER_EXC_COL_ENA : 0) | 
             VTSS_F_DEV_MAC_CFG_STATUS_MAC_HDX_CFG_LATE_COL_POS(67));
    SRVL_WR(VTSS_DEV_MAC_CFG_STATUS_MAC_HDX_CFG(tgt), 
            value | VTSS_F_DEV_MAC_CFG_STATUS_MAC_HDX_CFG_SEED_LOAD);
    VTSS_NSLEEP(1000);
    SRVL_WR(VTSS_DEV_MAC_CFG_STATUS_MAC_HDX_CFG(tgt), value);

    /* PCS settings for 100fx/SGMII/SERDES */
    if (if_100fx) {
        /* 100FX PCS */                    
        SRVL_WRM(VTSS_DEV_PCS_FX100_CONFIGURATION_PCS_FX100_CFG(tgt),
                 (disable ? 0 : VTSS_F_DEV_PCS_FX100_CONFIGURATION_PCS_FX100_CFG_PCS_ENA) |
                 (conf->sd_internal ? 0 : VTSS_F_DEV_PCS_FX100_CONFIGURATION_PCS_FX100_CFG_SD_SEL) |
                 (conf->sd_active_high ? VTSS_F_DEV_PCS_FX100_CONFIGURATION_PCS_FX100_CFG_SD_POL : 0) |
                 (conf->sd_enable ? VTSS_F_DEV_PCS_FX100_CONFIGURATION_PCS_FX100_CFG_SD_ENA : 0),
                 VTSS_F_DEV_PCS_FX100_CONFIGURATION_PCS_FX100_CFG_PCS_ENA | 
                 VTSS_F_DEV_PCS_FX100_CONFIGURATION_PCS_FX100_CFG_SD_SEL |
                 VTSS_F_DEV_PCS_FX100_CONFIGURATION_PCS_FX100_CFG_SD_POL |
                 VTSS_F_DEV_PCS_FX100_CONFIGURATION_PCS_FX100_CFG_SD_ENA);
    } else {
        /* Disable 100FX */
        SRVL_WRM_CLR(VTSS_DEV_PCS_FX100_CONFIGURATION_PCS_FX100_CFG(tgt),
                     VTSS_F_DEV_PCS_FX100_CONFIGURATION_PCS_FX100_CFG_PCS_ENA);

        /* Choose SGMII or Serdes PCS mode */
        SRVL_WR(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_MODE_CFG(tgt), 
                (sgmii ? VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_MODE_CFG_SGMII_MODE_ENA : 0));
        
        if (sgmii) {
            /* Set whole register */
            SRVL_WR(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG(tgt),
                    VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG_SW_RESOLVE_ENA);
        } else {
            /* Clear specific bit only */
            SRVL_WRM_CLR(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG(tgt),
                         VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG_SW_RESOLVE_ENA);
        }
        
        SRVL_WR(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_SD_CFG(tgt),
                (conf->sd_active_high ? VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_SD_CFG_SD_POL : 0) |
                (conf->sd_enable ? VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_SD_CFG_SD_ENA : 0) |
                (conf->sd_internal ? 0 : VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_SD_CFG_SD_SEL));
        
        /* Enable/disable PCS */
        SRVL_WR(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_CFG(tgt), 
                disable ? 0 : VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_CFG_PCS_ENA);
        
        if (conf->if_type == VTSS_PORT_INTERFACE_SGMII) {
            SRVL_WR(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG(tgt), 0);
        } else if (conf->if_type == VTSS_PORT_INTERFACE_SGMII_CISCO) {
            /* Complete SGMII aneg */
            SRVL_WR(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG(tgt),
                    VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG_ADV_ABILITY(0x0001) |
                    VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG_SW_RESOLVE_ENA |
                    VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG_ANEG_ENA |
                    VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG_ANEG_RESTART_ONE_SHOT);
            
            /* Clear the sticky bits */
            SRVL_RD(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_STICKY(tgt), &value);
            SRVL_WR(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_STICKY(tgt), value);
        }           
    }

    SRVL_WRM_CTL(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_LB_CFG(tgt), 
                 conf->loop == VTSS_PORT_LOOP_PCS_HOST, 
                 VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_LB_CFG_TBI_HOST_LB_ENA);

    /* Set Max Length and maximum tags allowed */
    SRVL_WR(VTSS_DEV_MAC_CFG_STATUS_MAC_MAXLEN_CFG(tgt), conf->max_frame_length);
    VTSS_RC(vtss_srvl_port_max_tags_set(vtss_state, port_no));

    if (!disable) {      
        /* Update policer flow control configuration */
        VTSS_RC(vtss_srvl_port_policer_fc_set(vtss_state, port_no));

        /* Enable MAC module */
        SRVL_WR(VTSS_DEV_MAC_CFG_STATUS_MAC_ENA_CFG(tgt), 
                VTSS_F_DEV_MAC_CFG_STATUS_MAC_ENA_CFG_RX_ENA |
                VTSS_F_DEV_MAC_CFG_STATUS_MAC_ENA_CFG_TX_ENA);
        
        /* Take MAC, Port, Phy (intern) and PCS (SGMII/Serdes) clock out of reset */
        SRVL_WR(VTSS_DEV_PORT_MODE_CLOCK_CFG(tgt), 
                VTSS_F_DEV_PORT_MODE_CLOCK_CFG_LINK_SPEED(link_speed));

        /* Configure flow control */
        if (srvl_port_fc_setup(vtss_state, port, conf) != VTSS_RC_OK)
            return VTSS_RC_ERROR;
		
        /* Clear the PCS stickys */
        SRVL_WR(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_STICKY(tgt),
                VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_STICKY_LINK_DOWN_STICKY |
                VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_STICKY_OUT_OF_SYNC_STICKY);

        /* Core: Enable port for frame transfer */
        SRVL_WRM_SET(VTSS_QSYS_SYSTEM_SWITCH_PORT_MODE(port),
                     VTSS_F_QSYS_SYSTEM_SWITCH_PORT_MODE_PORT_ENA);

#if defined(VTSS_FEATURE_AFI_SWC)
        /* Resume any AFI-generated frames to this port. This will only happen if using the
         * FDMA Driver to inject frames. An external CPU will have to make sure that
         * the frame is re-injected once link-up event is seen.
         */
        VTSS_RC(srvl_afi_pause_resume(vtss_state, port_no, TRUE));
#endif /*V TSS_FEATURE_AFI */

        /* Enable flowcontrol - must be done after flushing */
        if (conf->flow_control.generate) {   
            SRVL_WRM_SET(VTSS_SYS_PAUSE_CFG_MAC_FC_CFG(port), VTSS_F_SYS_PAUSE_CFG_MAC_FC_CFG_TX_FC_ENA);
        }

    }
    return VTSS_RC_OK;
}

static vtss_rc srvl_port_ifh_set(vtss_state_t *vtss_state, const vtss_port_no_t port_no)
{
    u32 port               = VTSS_CHIP_PORT(port_no);
    vtss_port_ifh_t *ifh = &vtss_state->port.ifh_conf[port_no];

     /* Enable/Disable IFH parsing upon injection - expecting to see No Prefix */
    SRVL_WRM(VTSS_SYS_SYSTEM_PORT_MODE(port), 
             VTSS_F_SYS_SYSTEM_PORT_MODE_INCL_INJ_HDR(ifh->ena_inj_header ? 1 : 0),
             VTSS_M_SYS_SYSTEM_PORT_MODE_INCL_INJ_HDR);
     /* Enable/Disable IFH parsing upon extraction - add long prefix */
    SRVL_WRM(VTSS_SYS_SYSTEM_PORT_MODE(port), 
             VTSS_F_SYS_SYSTEM_PORT_MODE_INCL_XTR_HDR(ifh->ena_xtr_header ? 3 : 0),
             VTSS_M_SYS_SYSTEM_PORT_MODE_INCL_XTR_HDR);
 
    return VTSS_RC_OK;
}

static vtss_rc srvl_port_clause_37_control_get(vtss_state_t *vtss_state,
                                               const vtss_port_no_t port_no,
                                               vtss_port_clause_37_control_t *const control)
{
    u32 value, port = VTSS_CHIP_PORT(port_no);

    SRVL_RD(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG(VTSS_TO_DEV(port)), &value);
    control->enable = VTSS_BOOL(value & VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG_ANEG_ENA);
    value = VTSS_X_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG_ADV_ABILITY(control->enable ? value : 0);
    VTSS_RC(vtss_cmn_port_clause_37_adv_get(value, &control->advertisement));

    return VTSS_RC_OK;
}

static vtss_rc srvl_port_clause_37_control_set(vtss_state_t *vtss_state,
                                               const vtss_port_no_t port_no)
{
    vtss_port_clause_37_control_t *control = &vtss_state->port.clause_37[port_no];
    u32                           value, port = VTSS_CHIP_PORT(port_no);
       
    /* Aneg capabilities for this port */
    VTSS_RC(vtss_cmn_port_clause_37_adv_set(&value, &control->advertisement, control->enable));

    /* Set aneg capabilities */
    SRVL_WR(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG(VTSS_TO_DEV(port)), 
           VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG_ADV_ABILITY(value) |
           VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG_ANEG_RESTART_ONE_SHOT |
           (control->enable ? VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG_ANEG_ENA : 0));
    
    return VTSS_RC_OK;
}

static vtss_rc srvl_port_clause_37_status_get(vtss_state_t *vtss_state,
                                              const vtss_port_no_t         port_no, 
                                              vtss_port_clause_37_status_t *const status)
    
{
    u32                    value, tgt = VTSS_TO_DEV(vtss_state->port.map[port_no].chip_port);
    vtss_port_sgmii_aneg_t *sgmii_adv = &status->autoneg.partner_adv_sgmii;
        
    if (vtss_state->port.conf[port_no].power_down) {
        status->link = 0;
        return VTSS_RC_OK;
    }

    /* Get the link state 'down' sticky bit  */
    SRVL_RD(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_STICKY(tgt), &value);
    status->link = (SRVL_BF(DEV_PCS1G_CFG_STATUS_PCS1G_STICKY_LINK_DOWN_STICKY, value) ? 0 : 1);
    if (status->link == 0) {
        /* The link has been down. Clear the sticky bit and return the 'down' value  */
        SRVL_WR(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_STICKY(tgt), 
                VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_STICKY_LINK_DOWN_STICKY);        
    } else {
        SRVL_RD(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_LINK_STATUS(tgt), &value);
        status->link = SRVL_BF(DEV_PCS1G_CFG_STATUS_PCS1G_LINK_STATUS_LINK_STATUS, value);        
    }

    /* Get PCS ANEG status register */
    SRVL_RD(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_STATUS(tgt), &value);

    /* Get 'Aneg complete'   */
    status->autoneg.complete = SRVL_BF(DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_STATUS_ANEG_COMPLETE, value);

   /* Workaround for a Serdes issue, when aneg completes with FDX capability=0 */
    if (vtss_state->port.conf[port_no].if_type == VTSS_PORT_INTERFACE_SERDES) {
        if (status->autoneg.complete) {
            if (((value >> 21) & 0x1) == 0) { 
                SRVL_WRM_CLR(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_CFG(tgt), VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_CFG_PCS_ENA);
                SRVL_WRM_SET(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_CFG(tgt), VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_CFG_PCS_ENA);
                (void)srvl_port_clause_37_control_set(vtss_state, port_no); /* Restart Aneg */
                VTSS_MSLEEP(50);
                SRVL_RD(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_STATUS(tgt), &value);
                status->autoneg.complete = SRVL_BF(DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_STATUS_ANEG_COMPLETE, value);
            }
        }
    }

    /* Return partner advertisement ability */
    value = VTSS_X_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_STATUS_LP_ADV_ABILITY(value);

    if (vtss_state->port.conf[port_no].if_type == VTSS_PORT_INTERFACE_SGMII_CISCO) {
        sgmii_adv->link = ((value >> 15) == 1) ? 1 : 0;
        sgmii_adv->fdx = (((value >> 12) & 0x1) == 1) ? 1 : 0;
        value = ((value >> 10) & 3);
        sgmii_adv->speed_10M = (value == 0 ? 1 : 0);
        sgmii_adv->speed_100M = (value == 1 ? 1 : 0);
        sgmii_adv->speed_1G = (value == 2 ? 1 : 0);
        sgmii_adv->hdx = (status->autoneg.partner_advertisement.fdx ? 0 : 1);
        if (status->link) {
            /* If the SFP module does not have a link then the port does not have link */
            status->link = sgmii_adv->link;
        }
    } else {
        VTSS_RC(vtss_cmn_port_clause_37_adv_get(value, &status->autoneg.partner_advertisement));
    }

    return VTSS_RC_OK;
}

static vtss_rc srvl_port_status_get(vtss_state_t *vtss_state,
                                    const vtss_port_no_t  port_no, 
                                    vtss_port_status_t    *const status)
{
    vtss_port_conf_t *conf = &vtss_state->port.conf[port_no];
    u32              tgt = VTSS_TO_DEV(vtss_state->port.map[port_no].chip_port);
    u32              value;
    
    if (conf->power_down) {
        memset(status, 0, sizeof(*status));
        return VTSS_RC_OK;
    }

    switch (conf->if_type) {
    case VTSS_PORT_INTERFACE_100FX:
        /* Get the PCS status  */
        SRVL_RD(VTSS_DEV_PCS_FX100_STATUS_PCS_FX100_STATUS(tgt), &value);
        
        /* Link has been down if the are any error stickies */
        status->link_down = SRVL_BF(DEV_PCS_FX100_STATUS_PCS_FX100_STATUS_SYNC_LOST_STICKY, value) ||
                            SRVL_BF(DEV_PCS_FX100_STATUS_PCS_FX100_STATUS_SSD_ERROR_STICKY, value) ||
                            SRVL_BF(DEV_PCS_FX100_STATUS_PCS_FX100_STATUS_FEF_FOUND_STICKY, value) ||
                            SRVL_BF(DEV_PCS_FX100_STATUS_PCS_FX100_STATUS_PCS_ERROR_STICKY, value) ||
                            SRVL_BF(DEV_PCS_FX100_STATUS_PCS_FX100_STATUS_FEF_STATUS, value);
        if (status->link_down) {
            SRVL_WR(VTSS_DEV_PCS_FX100_STATUS_PCS_FX100_STATUS(tgt), 0xFFFF);
            SRVL_RD(VTSS_DEV_PCS_FX100_STATUS_PCS_FX100_STATUS(tgt), &value);
        }
        /* Link=1 if sync status=1 and no error stickies after a clear */
        status->link = SRVL_BF(DEV_PCS_FX100_STATUS_PCS_FX100_STATUS_SYNC_STATUS, value) && 
                     (!SRVL_BF(DEV_PCS_FX100_STATUS_PCS_FX100_STATUS_SYNC_LOST_STICKY, value) &&
                      !SRVL_BF(DEV_PCS_FX100_STATUS_PCS_FX100_STATUS_SSD_ERROR_STICKY, value) &&
                      !SRVL_BF(DEV_PCS_FX100_STATUS_PCS_FX100_STATUS_FEF_FOUND_STICKY, value) &&
                      !SRVL_BF(DEV_PCS_FX100_STATUS_PCS_FX100_STATUS_PCS_ERROR_STICKY, value) &&
                      !SRVL_BF(DEV_PCS_FX100_STATUS_PCS_FX100_STATUS_FEF_STATUS, value));
        status->speed = VTSS_SPEED_100M; 
        break;
    case VTSS_PORT_INTERFACE_VAUI:
        /* Get the PCS status */
        SRVL_RD(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_LINK_STATUS(tgt), &value);
        status->link = SRVL_BF(DEV_PCS1G_CFG_STATUS_PCS1G_LINK_STATUS_LINK_STATUS, value);
        SRVL_RD(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_STICKY(tgt), &value);
        VTSS_I("status->link:%d, status->link_down:%d", status->link, status->link_down);
        status->link_down = SRVL_BF(DEV_PCS1G_CFG_STATUS_PCS1G_STICKY_LINK_DOWN_STICKY, value);
        if (status->link_down) {
            /* The link has been down. Clear the sticky bit */
            SRVL_WRM_SET(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_STICKY(tgt),
                         VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_STICKY_LINK_DOWN_STICKY);
        }
        status->speed = VTSS_SPEED_2500M; 
        break;
    case VTSS_PORT_INTERFACE_NO_CONNECTION:
        status->link = 0;
        status->link_down = 0;
        break;
    default:
        return VTSS_RC_ERROR;
    }        
    status->fdx = 1;    
    return VTSS_RC_OK;
}

static vtss_rc srvl_port_counters_read(vtss_state_t                 *vtss_state,
                                       vtss_port_no_t               port_no,
                                       u32                          port,
                                       vtss_port_luton26_counters_t *c,
                                       vtss_port_counters_t *const  counters, 
                                       BOOL                         clear)
{
    u32                                i, base, *p = &base;
    vtss_port_rmon_counters_t          *rmon;
    vtss_port_if_group_counters_t      *if_group;
    vtss_port_ethernet_like_counters_t *elike;
    vtss_port_proprietary_counters_t   *prop;
    BOOL                               tx_aging_clear = clear;

    /* Setup counter view */
    SRVL_WR(VTSS_SYS_SYSTEM_STAT_CFG, VTSS_F_SYS_SYSTEM_STAT_CFG_STAT_VIEW(port));

    base = 0x00;
    VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->rx_octets, clear));
    VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->rx_unicast, clear));
    VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->rx_multicast, clear));
    VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->rx_broadcast, clear));
    VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->rx_shorts, clear));
    VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->rx_fragments, clear));
    VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->rx_jabbers, clear));
    VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->rx_crc_align_errors, clear));
    VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->rx_symbol_errors, clear));
    VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->rx_64, clear));
    VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->rx_65_127, clear));
    VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->rx_128_255, clear));
    VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->rx_256_511, clear));
    VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->rx_512_1023, clear));
    VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->rx_1024_1526, clear));
    VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->rx_1527_max, clear));
    VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->rx_pause, clear));
    VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->rx_control, clear));
    VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->rx_longs, clear));
    VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->rx_classified_drops, clear));
    for (i = 0; i < VTSS_PRIOS; i++) 
        VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->rx_red_class[i], clear));
    for (i = 0; i < VTSS_PRIOS; i++)
        VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->rx_yellow_class[i], clear));
    for (i = 0; i < VTSS_PRIOS; i++)
        VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->rx_green_class[i], clear));
    
    base = 0x40;
    VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->tx_octets, clear));
    VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->tx_unicast, clear));
    VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->tx_multicast, clear));
    VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->tx_broadcast, clear));
    VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->tx_collision, clear));
    VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->tx_drops, clear));
    VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->tx_pause, clear));
    VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->tx_64, clear));
    VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->tx_65_127, clear));
    VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->tx_128_255, clear));
    VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->tx_256_511, clear));
    VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->tx_512_1023, clear));
    VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->tx_1024_1526, clear));
    VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->tx_1527_max, clear));
    for (i = 0; i < VTSS_PRIOS; i++)
        VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->tx_yellow_class[i], clear));
    for (i = 0; i < VTSS_PRIOS; i++)
        VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->tx_green_class[i], clear));
#if defined(VTSS_FEATURE_EVC)
    if (port_no != VTSS_PORT_NO_NONE && vtss_state->evc.port_info[port_no].uni_count) {
        /* BZ 10411: Clear Tx age counter to ignore Up MEP discarded frames on UNI */
        tx_aging_clear = 1;
    }
#endif /* VTSS_FEATURE_EVC */
    VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->tx_aging, tx_aging_clear));

    /* 32-bit Drop chip counters */
    base = 0x80;
    VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->dr_local, clear));
    VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->dr_tail, clear));
    for (i = 0; i < VTSS_PRIOS; i++)
        VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->dr_yellow_class[i], clear));
    for (i = 0; i < VTSS_PRIOS; i++)
        VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->dr_green_class[i], clear));
   
    if (counters == NULL) {
        return VTSS_RC_OK;
    }

    /* Calculate API counters based on chip counters */
    rmon = &counters->rmon;
    if_group = &counters->if_group;
    elike = &counters->ethernet_like;
    prop = &counters->prop;

    /* Proprietary counters */
    for (i = 0; i < VTSS_PRIOS; i++) {
        prop->rx_prio[i] = (c->rx_red_class[i].value + c->rx_yellow_class[i].value + c->rx_green_class[i].value);
        prop->tx_prio[i] = (c->tx_yellow_class[i].value + c->tx_green_class[i].value);
    }

    /* RMON Rx counters */
    rmon->rx_etherStatsDropEvents = c->dr_tail.value;
    for (i = 0; i < VTSS_PRIOS; i++) {
        rmon->rx_etherStatsDropEvents += (c->dr_yellow_class[i].value + c->dr_green_class[i].value);
    }

    rmon->rx_etherStatsOctets = c->rx_octets.value;
    rmon->rx_etherStatsPkts = 
        (c->rx_shorts.value + c->rx_fragments.value + c->rx_jabbers.value + c->rx_longs.value + 
         c->rx_64.value + c->rx_65_127.value + c->rx_128_255.value + c->rx_256_511.value + 
         c->rx_512_1023.value + c->rx_1024_1526.value + c->rx_1527_max.value);
    rmon->rx_etherStatsBroadcastPkts = c->rx_broadcast.value;
    rmon->rx_etherStatsMulticastPkts = c->rx_multicast.value;
    rmon->rx_etherStatsCRCAlignErrors = c->rx_crc_align_errors.value;
    rmon->rx_etherStatsUndersizePkts = c->rx_shorts.value;
    rmon->rx_etherStatsOversizePkts = c->rx_longs.value;
    rmon->rx_etherStatsFragments = c->rx_fragments.value;
    rmon->rx_etherStatsJabbers = c->rx_jabbers.value;
    rmon->rx_etherStatsPkts64Octets = c->rx_64.value;
    rmon->rx_etherStatsPkts65to127Octets = c->rx_65_127.value;
    rmon->rx_etherStatsPkts128to255Octets = c->rx_128_255.value;
    rmon->rx_etherStatsPkts256to511Octets = c->rx_256_511.value;
    rmon->rx_etherStatsPkts512to1023Octets = c->rx_512_1023.value;
    rmon->rx_etherStatsPkts1024to1518Octets = c->rx_1024_1526.value;
    rmon->rx_etherStatsPkts1519toMaxOctets = c->rx_1527_max.value;

    /* RMON Tx counters */
    rmon->tx_etherStatsDropEvents = (c->tx_drops.value + c->tx_aging.value);
    rmon->tx_etherStatsOctets = c->tx_octets.value;
    rmon->tx_etherStatsPkts = 
        (c->tx_64.value + c->tx_65_127.value + c->tx_128_255.value + c->tx_256_511.value + 
         c->tx_512_1023.value + c->tx_1024_1526.value + c->tx_1527_max.value);
    rmon->tx_etherStatsBroadcastPkts = c->tx_broadcast.value;
    rmon->tx_etherStatsMulticastPkts = c->tx_multicast.value;
    rmon->tx_etherStatsCollisions = c->tx_collision.value;
    rmon->tx_etherStatsPkts64Octets = c->tx_64.value;
    rmon->tx_etherStatsPkts65to127Octets = c->tx_65_127.value;
    rmon->tx_etherStatsPkts128to255Octets = c->tx_128_255.value;
    rmon->tx_etherStatsPkts256to511Octets = c->tx_256_511.value;
    rmon->tx_etherStatsPkts512to1023Octets = c->tx_512_1023.value;
    rmon->tx_etherStatsPkts1024to1518Octets = c->tx_1024_1526.value;
    rmon->tx_etherStatsPkts1519toMaxOctets = c->tx_1527_max.value;
    
    /* Interfaces Group Rx counters */
    if_group->ifInOctets = c->rx_octets.value;
    if_group->ifInUcastPkts = c->rx_unicast.value;
    if_group->ifInMulticastPkts = c->rx_multicast.value;
    if_group->ifInBroadcastPkts = c->rx_broadcast.value;
    if_group->ifInNUcastPkts = c->rx_multicast.value + c->rx_broadcast.value;
    if_group->ifInDiscards = rmon->rx_etherStatsDropEvents;
    if_group->ifInErrors = 
        (c->rx_crc_align_errors.value + c->rx_shorts.value + c->rx_fragments.value +
         c->rx_jabbers.value + c->rx_longs.value);
  
    /* Interfaces Group Tx counters */
    if_group->ifOutOctets = c->tx_octets.value;
    if_group->ifOutUcastPkts = c->tx_unicast.value;
    if_group->ifOutMulticastPkts = c->tx_multicast.value;
    if_group->ifOutBroadcastPkts = c->tx_broadcast.value;
    if_group->ifOutNUcastPkts = (c->tx_multicast.value + c->tx_broadcast.value);
    if_group->ifOutErrors = (c->tx_drops.value + c->tx_aging.value);

    /* Ethernet-like counters */
    elike->dot3InPauseFrames = c->rx_pause.value;
    elike->dot3OutPauseFrames = c->tx_pause.value;     

    /* Bridge counters */
    counters->bridge.dot1dTpPortInDiscards = (c->rx_classified_drops.value + c->dr_local.value);

    return VTSS_RC_OK;
}

static vtss_rc srvl_port_basic_counters_get(vtss_state_t *vtss_state,
                                            const vtss_port_no_t port_no,
                                            vtss_basic_counters_t *const counters)
{
    u32                          base, *p = &base, port = VTSS_CHIP_PORT(port_no);
    vtss_port_luton26_counters_t *c = &vtss_state->port.counters[port_no].counter.luton26;

    /* Setup counter view */
    SRVL_WR(VTSS_SYS_SYSTEM_STAT_CFG, VTSS_F_SYS_SYSTEM_STAT_CFG_STAT_VIEW(port));
    
    /* Rx Counters */
    base = 0x09; /* rx_64 */
    VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->rx_64, 0));
    VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->rx_65_127, 0));
    VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->rx_128_255, 0));
    VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->rx_256_511, 0));
    VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->rx_512_1023, 0));
    VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->rx_1024_1526, 0));
    VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->rx_1527_max, 0));

    /* Tx Counters */
    base = 0x47; /* tx_64 */
    VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->tx_64, 0));
    VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->tx_65_127, 0));
    VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->tx_128_255, 0));
    VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->tx_256_511, 0));
    VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->tx_512_1023, 0));
    VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->tx_1024_1526, 0));
    VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->tx_1527_max, 0));

    /* Rx frames */
    counters->rx_frames = 
        (c->rx_64.value + c->rx_65_127.value + c->rx_128_255.value + c->rx_256_511.value + 
         c->rx_512_1023.value + c->rx_1024_1526.value + c->rx_1527_max.value);
    
    /* Tx frames */
    counters->tx_frames = 
        (c->tx_64.value + c->tx_65_127.value + c->tx_128_255.value + c->tx_256_511.value + 
         c->tx_512_1023.value + c->tx_1024_1526.value + c->tx_1527_max.value);

    return VTSS_RC_OK;
}

static vtss_rc srvl_port_counters_cmd(vtss_state_t                *vtss_state,
                                      const vtss_port_no_t        port_no, 
                                      vtss_port_counters_t *const counters, 
                                      BOOL                        clear)
{
    return srvl_port_counters_read(vtss_state,
                                   port_no,
                                   VTSS_CHIP_PORT(port_no),
                                   &vtss_state->port.counters[port_no].counter.luton26,
                                   counters,
                                   clear);
}

static vtss_rc srvl_port_counters_update(vtss_state_t *vtss_state, const vtss_port_no_t port_no)
{
    return srvl_port_counters_cmd(vtss_state, port_no, NULL, 0);
}

static vtss_rc srvl_port_counters_clear(vtss_state_t *vtss_state, const vtss_port_no_t port_no)
{
    return srvl_port_counters_cmd(vtss_state, port_no, NULL, 1);
}

static vtss_rc srvl_port_counters_get(vtss_state_t *vtss_state,
                                      const vtss_port_no_t port_no,
                                      vtss_port_counters_t *const counters)
{
    memset(counters, 0, sizeof(*counters)); 
    return srvl_port_counters_cmd(vtss_state, port_no, counters, 0);
}

static vtss_rc srvl_port_forward_set(vtss_state_t *vtss_state, const vtss_port_no_t port_no)
{
    return VTSS_RC_OK;
}

/*
 * Watermark encode/decode for QSYS:RES_CTRL:RES_CFG.WM_HIGH
 * Bit 11:   Unit; 0:1, 1:16
 * Bit 10-0: Value to be multiplied with unit
 */
static u16 WmEnc(u16 value)
{
    if (value >= 2048) {
        return 2048 + value / 16;
    }
    return value;
}

u32 vtss_srvl_wm_dec(u32 value)
{
    if (value > 2048) { 
        return (value - 2048) * 16;
    }        
    return value;
}

static vtss_rc srvl_port_buf_conf_set(vtss_state_t *vtss_state)
{
    u32 port_no, port, q, dp;
    u32 buf_q_rsrv_i, buf_q_rsrv_e, ref_q_rsrv_i, ref_q_rsrv_e, buf_prio_shr_i[8], buf_prio_shr_e[8], ref_prio_shr_i[8], ref_prio_shr_e[8];
    u32 buf_p_rsrv_i, buf_p_rsrv_e, ref_p_rsrv_i, ref_p_rsrv_e, buf_col_shr_i, buf_col_shr_e, ref_col_shr_i, ref_col_shr_e;
    u32 buf_prio_rsrv, ref_prio_rsrv, guaranteed, q_rsrv_mask, prio_mem, prio_ref, oversubscription_factor;

    /* SYS::RES_CFG : 1024 watermarks for 1024000 byte shared buffer, unit is 60 byte */
    /* Is divided into 4 resource consumptions, ingress and egress memory (BUF) and ingress and egress frame reference (REF) blocks */   
    /* Queue reserved (q_rsrv) : starts at offset 0 within in each BUF and REF   */
    /* Prio shared (prio_shr)  : starts at offset 216 within in each BUF and REF */
    /* Port reserved (p_rsrv)  : starts at offset 224 within in each BUF and REF */
    /* Colour shared (col_shr) : starts at offset 254 within in each BUF and REF */

    /* Buffer values are in BYTES */ 
    buf_q_rsrv_i = 512;    /* Guarantees at least 1 MTU  */
    buf_p_rsrv_i = 0;      /* No additional ingress guarantees   */
    buf_q_rsrv_e = 3000;   /* Guarantees all QoS classes some space */
    buf_p_rsrv_e = 40960;  /* Guarantees a space to the egress ports */
    buf_col_shr_i = SRVL_BUFFER_MEMORY; /* Coloring - disabled for now */
    buf_col_shr_e = SRVL_BUFFER_MEMORY; /* Coloring - disabled for now */
    buf_prio_rsrv = 12288;  /* In the shared area, each priority is cut off 12kB before the others */

    /* Reference values in NUMBER of FRAMES */
    ref_q_rsrv_e = 10;     /* Number of frames that can be pending at each egress queue   */
    ref_q_rsrv_i = 10;     /* Number of frames that can be pending at each ingress queue  */
    ref_p_rsrv_e = 100;    /* Number of frames that can be pending shared between the QoS classes at egress */
    ref_p_rsrv_i = 20;     /* Number of frames that can be pending shared between the QoS classes at ingress */
    ref_col_shr_i = SRVL_BUFFER_REFERENCE; /* Coloring - disabled  */
    ref_col_shr_e = SRVL_BUFFER_REFERENCE; /* Coloring - disabled  */
    ref_prio_rsrv = 50;   /* Number of frames that can be pending for each class */

    /* The memory is oversubsrcribed by this factor (factor 1 = 100) */
    /* Oversubscription is possible (in some degree) because it's rare that all ports use their reserved space at the same time */
    oversubscription_factor = 100; /* No oversubscription */

    /* Note, the shared reserved space (buf_prio_shr_i, ref_prio_shr_i, buf_prio_shr_e, ref_prio_shr_e) is calulated based on above */

    /* The number of supported queues is given through the state structure                           */
    /* The supported queues (lowest to higest) are givin reserved buffer space as specified above.   */
    /* Frames in remaining queues (if any) are not getting any reserved space - but are allowed in the system.*/
    q_rsrv_mask = 0xff >> (8 - vtss_state->qos.conf.prios); 

    /* **************************************************  */
    /* BELOW, everything is calculated based on the above. */
    /* **************************************************  */

    /* Find the amount of guaranteeed space per port */
    guaranteed = buf_p_rsrv_i+buf_p_rsrv_e;
    for (q=0; q<VTSS_PRIOS; q++) {
        if (q_rsrv_mask & (1<<q)) 
            guaranteed+=(buf_q_rsrv_i+buf_q_rsrv_e);
    }

    prio_mem = (oversubscription_factor * SRVL_BUFFER_MEMORY)/100 - (vtss_state->port_count+1)*guaranteed;

    /* Find the amount of guaranteeed frame references */
    guaranteed = ref_p_rsrv_i+ref_p_rsrv_e;
    for (q=0; q<VTSS_PRIOS; q++) {
        if (q_rsrv_mask & (1<<q)) {
            guaranteed+=(ref_q_rsrv_i+ref_q_rsrv_e);
        }
    }
    prio_ref = SRVL_BUFFER_REFERENCE - (vtss_state->port_count+1)*guaranteed;

    for (q = VTSS_PRIOS-1; ; q--) {
        buf_prio_shr_i[q] = prio_mem;
        ref_prio_shr_i[q] = prio_ref;
        buf_prio_shr_e[q] = prio_mem;
        ref_prio_shr_e[q] = prio_ref;

        if (q_rsrv_mask & (1<<q)) {
            prio_mem -= buf_prio_rsrv;
            prio_ref -= ref_prio_rsrv;
        }
        if (q==0) break;
    }
 
    /* Configure reserved space for all QoS classes per port */
    for (port_no = 0; port_no <= vtss_state->port_count; port_no++) {
        if (port_no == vtss_state->port_count) {
            port = VTSS_CHIP_PORT_CPU;
        } else {
            port = VTSS_CHIP_PORT(port_no);
        }
        for (q = 0; q < VTSS_PRIOS; q++) {
            if (q_rsrv_mask&(1<<q)) {
                SRVL_WR(VTSS_QSYS_RES_CTRL_RES_CFG(port * VTSS_PRIOS + q + 0),   WmEnc(buf_q_rsrv_i/SRVL_BUFFER_CELL_SZ));
                SRVL_WR(VTSS_QSYS_RES_CTRL_RES_CFG(port * VTSS_PRIOS + q + 256), WmEnc(ref_q_rsrv_i));
                SRVL_WR(VTSS_QSYS_RES_CTRL_RES_CFG(port * VTSS_PRIOS + q + 512), WmEnc(buf_q_rsrv_e/SRVL_BUFFER_CELL_SZ));
                SRVL_WR(VTSS_QSYS_RES_CTRL_RES_CFG(port * VTSS_PRIOS + q + 768), WmEnc(ref_q_rsrv_e));
            }
        }
    }

    /* Configure shared space for all QoS classes */
    for (q = 0; q < VTSS_PRIOS; q++) {
        SRVL_WR(VTSS_QSYS_RES_CTRL_RES_CFG((q + 216 + 0)),   WmEnc(buf_prio_shr_i[q]/SRVL_BUFFER_CELL_SZ));
        SRVL_WR(VTSS_QSYS_RES_CTRL_RES_CFG((q + 216 + 256)), WmEnc(ref_prio_shr_i[q]));
        SRVL_WR(VTSS_QSYS_RES_CTRL_RES_CFG((q + 216 + 512)), WmEnc(buf_prio_shr_e[q]/SRVL_BUFFER_CELL_SZ));
        SRVL_WR(VTSS_QSYS_RES_CTRL_RES_CFG((q + 216 + 768)), WmEnc(ref_prio_shr_e[q]));
    }

    /* Configure reserved space for all ports */
    for (port_no = 0; port_no <= vtss_state->port_count; port_no++) {
        if (port_no == vtss_state->port_count) {
            port = VTSS_CHIP_PORT_CPU;
        } else {
            port = VTSS_CHIP_PORT(port_no);
        }
        SRVL_WR(VTSS_QSYS_RES_CTRL_RES_CFG(port + 224 +   0), WmEnc(buf_p_rsrv_i/SRVL_BUFFER_CELL_SZ));
        SRVL_WR(VTSS_QSYS_RES_CTRL_RES_CFG(port + 224 + 256), WmEnc(ref_p_rsrv_i));
        SRVL_WR(VTSS_QSYS_RES_CTRL_RES_CFG(port + 224 + 512), WmEnc(buf_p_rsrv_e/SRVL_BUFFER_CELL_SZ));
        SRVL_WR(VTSS_QSYS_RES_CTRL_RES_CFG(port + 224 + 768), WmEnc(ref_p_rsrv_e));
    }

    /* Configure shared space for  both DP levels (green:0 yellow:1) */
    for (dp = 0; dp < 2; dp++) {
        SRVL_WR(VTSS_QSYS_RES_CTRL_RES_CFG(dp + 254 +   0), WmEnc(buf_col_shr_i/SRVL_BUFFER_CELL_SZ));
        SRVL_WR(VTSS_QSYS_RES_CTRL_RES_CFG(dp + 254 + 256), ref_col_shr_i);
        SRVL_WR(VTSS_QSYS_RES_CTRL_RES_CFG(dp + 254 + 512), WmEnc(buf_col_shr_e/SRVL_BUFFER_CELL_SZ));
        SRVL_WR(VTSS_QSYS_RES_CTRL_RES_CFG(dp + 254 + 768), ref_col_shr_e);
    }
    return VTSS_RC_OK;
}

/* - Debug print --------------------------------------------------- */

static void srvl_debug_fld_nl(const vtss_debug_printf_t pr, const char *name, u32 value)
{
    pr("%-20s: %u\n", name, value);
}

#define SRVL_GET_FLD(tgt, addr, fld, value)  VTSS_X_##tgt##_##addr##_##fld(value)
#define SRVL_GET_BIT(tgt, addr, fld, value)  (VTSS_F_##tgt##_##addr##_##fld & (value) ? 1 : 0)

#define SRVL_DEBUG_HSIO(pr, addr, name) vtss_srvl_debug_reg(vtss_state, pr, VTSS_HSIO_##addr, name)
#define SRVL_DEBUG_MAC(pr, addr, i, name) vtss_srvl_debug_reg_inst(vtss_state, pr, VTSS_DEV_MAC_CFG_STATUS_MAC_##addr, i, "MAC_"name)
#define SRVL_DEBUG_PCS(pr, addr, i, name) vtss_srvl_debug_reg_inst(vtss_state, pr, VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_##addr, i, "PCS_"name)

#define SRVL_DEBUG_HSIO_BIT(pr, addr, fld, value) srvl_debug_fld_nl(pr, #fld, SRVL_GET_BIT(HSIO, addr, fld, x))
#define SRVL_DEBUG_HSIO_FLD(pr, addr, fld, value) srvl_debug_fld_nl(pr, #fld, SRVL_GET_FLD(HSIO, addr, fld, x))
#define SRVL_DEBUG_RAW(pr, offset, length, value, name) srvl_debug_fld_nl(pr, name, VTSS_EXTRACT_BITFIELD(value, offset, length))

static vtss_rc srvl_debug_serdes6g(vtss_state_t *vtss_state,
                                   const vtss_debug_printf_t pr,
                                   u32 inst,
                                   const char *buf)
{
    u32            x;

    vtss_srvl_debug_reg_header(pr, buf);
    VTSS_RC(srvl_sd6g_read(vtss_state, 1 << inst));
    SRVL_DEBUG_HSIO(pr, SERDES6G_ANA_CFG_SERDES6G_DES_CFG, "DES_CFG");
    SRVL_DEBUG_HSIO(pr, SERDES6G_ANA_CFG_SERDES6G_IB_CFG, "IB_CFG");
    SRVL_DEBUG_HSIO(pr, SERDES6G_ANA_CFG_SERDES6G_IB_CFG1, "IB_CFG1");
    SRVL_DEBUG_HSIO(pr, SERDES6G_ANA_CFG_SERDES6G_OB_CFG, "OB_CFG");
    SRVL_DEBUG_HSIO(pr, SERDES6G_ANA_CFG_SERDES6G_OB_CFG1, "OB_CFG1");
    SRVL_DEBUG_HSIO(pr, SERDES6G_ANA_CFG_SERDES6G_SER_CFG, "SER_CFG");
    SRVL_DEBUG_HSIO(pr, SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG, "COMMON_CFG");
    SRVL_DEBUG_HSIO(pr, SERDES6G_ANA_CFG_SERDES6G_PLL_CFG, "PLL_CFG");
    SRVL_DEBUG_HSIO(pr, SERDES6G_ANA_STATUS_SERDES6G_PLL_STATUS, "PLL_STATUS");
    SRVL_DEBUG_HSIO(pr, SERDES6G_DIG_CFG_SERDES6G_DIG_CFG, "DIG_CFG");
    SRVL_DEBUG_HSIO(pr, SERDES6G_DIG_CFG_SERDES6G_DFT_CFG0, "DFT_CFG0");
    SRVL_DEBUG_HSIO(pr, SERDES6G_DIG_CFG_SERDES6G_DFT_CFG1, "DFT_CFG1");
    SRVL_DEBUG_HSIO(pr, SERDES6G_DIG_CFG_SERDES6G_DFT_CFG2, "DFT_CFG2");
    SRVL_DEBUG_HSIO(pr, SERDES6G_DIG_CFG_SERDES6G_TP_CFG0, "TP_CFG0");
    SRVL_DEBUG_HSIO(pr, SERDES6G_DIG_CFG_SERDES6G_TP_CFG1, "TP_CFG1");
    SRVL_DEBUG_HSIO(pr, SERDES6G_DIG_CFG_SERDES6G_MISC_CFG, "MISC_CFG");
    SRVL_DEBUG_HSIO(pr, SERDES6G_DIG_CFG_SERDES6G_OB_ANEG_CFG, "OB_ANEG_CFG");
    SRVL_DEBUG_HSIO(pr, SERDES6G_DIG_STATUS_SERDES6G_DFT_STATUS, "DFT_STATUS");

    pr("\n%s:OB_CFG:\n", buf);
    SRVL_RD(VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_OB_CFG, &x);
    SRVL_DEBUG_HSIO_BIT(pr, SERDES6G_ANA_CFG_SERDES6G_OB_CFG, OB_ENA1V_MODE, x);
    SRVL_DEBUG_HSIO_BIT(pr, SERDES6G_ANA_CFG_SERDES6G_OB_CFG, OB_POL, x);
    SRVL_DEBUG_HSIO_FLD(pr, SERDES6G_ANA_CFG_SERDES6G_OB_CFG, OB_POST0, x);
    SRVL_DEBUG_HSIO_FLD(pr, SERDES6G_ANA_CFG_SERDES6G_OB_CFG, OB_PREC, x);
    SRVL_DEBUG_HSIO_BIT(pr, SERDES6G_ANA_CFG_SERDES6G_OB_CFG, OB_SR_H, x);
    SRVL_DEBUG_HSIO_FLD(pr, SERDES6G_ANA_CFG_SERDES6G_OB_CFG, OB_SR, x);

    pr("\n%s:OB_CFG1:\n", buf);
    SRVL_RD(VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_OB_CFG1, &x);
    SRVL_DEBUG_HSIO_FLD(pr, SERDES6G_ANA_CFG_SERDES6G_OB_CFG1, OB_ENA_CAS, x);
    SRVL_DEBUG_HSIO_FLD(pr, SERDES6G_ANA_CFG_SERDES6G_OB_CFG1, OB_LEV, x);

    pr("\n%s:DES_CFG:\n", buf);
    SRVL_RD(VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_DES_CFG, &x);
    SRVL_DEBUG_HSIO_FLD(pr, SERDES6G_ANA_CFG_SERDES6G_DES_CFG, DES_MBTR_CTRL, x);
    SRVL_DEBUG_HSIO_FLD(pr, SERDES6G_ANA_CFG_SERDES6G_DES_CFG, DES_CPMD_SEL, x);
    SRVL_DEBUG_HSIO_FLD(pr, SERDES6G_ANA_CFG_SERDES6G_DES_CFG, DES_BW_ANA, x);

    pr("\n%s:IB_CFG:\n", buf);
    SRVL_RD(VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG, &x);
    SRVL_DEBUG_HSIO_FLD(pr, SERDES6G_ANA_CFG_SERDES6G_IB_CFG, IB_IC_AC, x);
    SRVL_DEBUG_HSIO_FLD(pr, SERDES6G_ANA_CFG_SERDES6G_IB_CFG, IB_RF, x);
    SRVL_DEBUG_HSIO_FLD(pr, SERDES6G_ANA_CFG_SERDES6G_IB_CFG, IB_RT, x);
    SRVL_DEBUG_HSIO_FLD(pr, SERDES6G_ANA_CFG_SERDES6G_IB_CFG, IB_VBAC, x);
            
    pr("\n%s:SER_CFG:\n", buf);
    SRVL_RD(VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_SER_CFG, &x);
    SRVL_DEBUG_HSIO_BIT(pr, SERDES6G_ANA_CFG_SERDES6G_SER_CFG, SER_ENALI, x);

    pr("\n%s:PLL_CFG:\n", buf);
    SRVL_RD(VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG, &x);
    SRVL_DEBUG_HSIO_BIT(pr, SERDES6G_ANA_CFG_SERDES6G_PLL_CFG, PLL_DIV4, x);
    SRVL_DEBUG_HSIO_BIT(pr, SERDES6G_ANA_CFG_SERDES6G_PLL_CFG, PLL_ENA_ROT, x);
    SRVL_DEBUG_HSIO_FLD(pr, SERDES6G_ANA_CFG_SERDES6G_PLL_CFG, PLL_FSM_CTRL_DATA, x);
    SRVL_DEBUG_HSIO_BIT(pr, SERDES6G_ANA_CFG_SERDES6G_PLL_CFG, PLL_FSM_ENA, x);
    SRVL_DEBUG_HSIO_BIT(pr, SERDES6G_ANA_CFG_SERDES6G_PLL_CFG, PLL_ROT_DIR, x);
    SRVL_DEBUG_HSIO_BIT(pr, SERDES6G_ANA_CFG_SERDES6G_PLL_CFG, PLL_ROT_FRQ, x);

    pr("\n%s:COMMON_CFG:\n", buf);
    SRVL_RD(VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG, &x);
    SRVL_DEBUG_HSIO_BIT(pr, SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG, ENA_LANE, x);
    SRVL_DEBUG_HSIO_BIT(pr, SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG, HRATE, x);
    SRVL_DEBUG_HSIO_BIT(pr, SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG, QRATE, x);
    SRVL_DEBUG_HSIO_FLD(pr, SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG, IF_MODE, x);
    SRVL_DEBUG_HSIO_BIT(pr, SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG, RECO_SEL_A, x);
    SRVL_DEBUG_HSIO_BIT(pr, SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG, RECO_SEL_B, x);
    SRVL_DEBUG_HSIO_BIT(pr, SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG, SE_AUTO_SQUELCH_A_ENA, x);
    SRVL_DEBUG_HSIO_BIT(pr, SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG, SE_AUTO_SQUELCH_B_ENA, x);

    return VTSS_RC_OK;
}

static vtss_rc srvl_debug_port(vtss_state_t *vtss_state,
                               const vtss_debug_printf_t pr,
                               const vtss_debug_info_t   *const info)
{
    u32            port, tgt, inst, x;
    vtss_port_no_t port_no;
    char           buf[32];
    BOOL           serdes6g;
    
    for (port = 0; port < VTSS_CHIP_PORTS; port++) {
        if ((port_no = vtss_cmn_port2port_no(vtss_state, info, port)) == VTSS_PORT_NO_NONE)
            continue;
        sprintf(buf, "Port %u (%u)", port, port_no);
        vtss_srvl_debug_reg_header(pr, buf);
        tgt = VTSS_TO_DEV(port);
        vtss_srvl_debug_reg_inst(vtss_state, pr, VTSS_DEV_PORT_MODE_CLOCK_CFG(tgt), port, "CLOCK_CFG");
        SRVL_DEBUG_MAC(pr, ENA_CFG(tgt), port, "ENA_CFG");
        SRVL_DEBUG_MAC(pr, MODE_CFG(tgt), port, "MODE_CFG");
        SRVL_DEBUG_MAC(pr, MAXLEN_CFG(tgt), port, "MAXLEN_CFG");
        SRVL_DEBUG_MAC(pr, TAGS_CFG(tgt), port, "TAGS_CFG");
        vtss_srvl_debug_reg_inst(vtss_state, pr, VTSS_SYS_PAUSE_CFG_MAC_FC_CFG(port), port, "FC_CFG");
        SRVL_DEBUG_PCS(pr, CFG(tgt), port, "CFG");
        SRVL_DEBUG_PCS(pr, MODE_CFG(tgt), port, "MODE_CFG");
        SRVL_DEBUG_PCS(pr, SD_CFG(tgt), port, "SD_CFG");
        SRVL_DEBUG_PCS(pr, ANEG_CFG(tgt), port, "ANEG_CFG");
        SRVL_DEBUG_PCS(pr, ANEG_STATUS(tgt), port, "ANEG_STATUS");
        SRVL_DEBUG_PCS(pr, LINK_STATUS(tgt), port, "LINK_STATUS");
        vtss_srvl_debug_reg_inst(vtss_state, pr, VTSS_DEV_PCS_FX100_CONFIGURATION_PCS_FX100_CFG(tgt),
                            port, "PCS_FX100_CFG");
        vtss_srvl_debug_reg_inst(vtss_state, pr, VTSS_DEV_PCS_FX100_STATUS_PCS_FX100_STATUS(tgt),
                            port, "FX100_STATUS");
        pr("\n");

        if (!info->full || srvl_serdes_inst_get(port, &inst, &serdes6g) != VTSS_RC_OK)
            continue;
        
        if (serdes6g) {
            sprintf(buf, "SerDes6G_%u", inst);
            (void)srvl_debug_serdes6g(vtss_state, pr, inst, buf);
        } else {
            sprintf(buf, "SerDes1G_%u", inst);
            vtss_srvl_debug_reg_header(pr, buf);
            VTSS_RC(srvl_sd1g_read(vtss_state, 1 << inst));
            SRVL_DEBUG_HSIO(pr, SERDES1G_ANA_CFG_SERDES1G_DES_CFG, "DES_CFG");
            SRVL_DEBUG_HSIO(pr, SERDES1G_ANA_CFG_SERDES1G_IB_CFG, "IB_CFG");
            SRVL_DEBUG_HSIO(pr, SERDES1G_ANA_CFG_SERDES1G_OB_CFG, "OB_CFG");
            SRVL_DEBUG_HSIO(pr, SERDES1G_ANA_CFG_SERDES1G_SER_CFG, "SER_CFG");
            SRVL_DEBUG_HSIO(pr, SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG, "COMMON_CFG");
            SRVL_DEBUG_HSIO(pr, SERDES1G_ANA_CFG_SERDES1G_PLL_CFG, "PLL_CFG");
            SRVL_DEBUG_HSIO(pr, SERDES1G_ANA_STATUS_SERDES1G_PLL_STATUS, "PLL_STATUS");
            SRVL_DEBUG_HSIO(pr, SERDES1G_DIG_CFG_SERDES1G_DFT_CFG0, "DFT_CFG0");
            SRVL_DEBUG_HSIO(pr, SERDES1G_DIG_CFG_SERDES1G_DFT_CFG1, "DFT_CFG1");
            SRVL_DEBUG_HSIO(pr, SERDES1G_DIG_CFG_SERDES1G_DFT_CFG2, "DFT_CFG2");
            SRVL_DEBUG_HSIO(pr, SERDES1G_DIG_CFG_SERDES1G_TP_CFG, "TP_CFG");
            SRVL_DEBUG_HSIO(pr, SERDES1G_DIG_CFG_SERDES1G_MISC_CFG, "MISC_CFG");
            SRVL_DEBUG_HSIO(pr, SERDES1G_DIG_STATUS_SERDES1G_DFT_STATUS, "DFT_STATUS");


            pr("\n%s:IB_CFG:\n", buf);
            SRVL_RD(VTSS_HSIO_SERDES1G_ANA_CFG_SERDES1G_IB_CFG, &x);
            SRVL_DEBUG_HSIO_FLD(pr, SERDES1G_ANA_CFG_SERDES1G_IB_CFG, IB_DET_LEV, x);
            SRVL_DEBUG_HSIO_BIT(pr, SERDES1G_ANA_CFG_SERDES1G_IB_CFG, IB_ENA_DC_COUPLING, x);
            SRVL_DEBUG_HSIO_BIT(pr, SERDES1G_ANA_CFG_SERDES1G_IB_CFG, IB_ENA_DETLEV, x);

            pr("\n%s:DES_CFG:\n", buf);
            SRVL_RD(VTSS_HSIO_SERDES1G_ANA_CFG_SERDES1G_DES_CFG, &x);
            SRVL_DEBUG_HSIO_FLD(pr, SERDES1G_ANA_CFG_SERDES1G_DES_CFG, DES_MBTR_CTRL, x);
            SRVL_DEBUG_HSIO_FLD(pr, SERDES1G_ANA_CFG_SERDES1G_DES_CFG, DES_CPMD_SEL, x);
            SRVL_DEBUG_HSIO_FLD(pr, SERDES1G_ANA_CFG_SERDES1G_DES_CFG, DES_BW_ANA, x);

            pr("\n%s:OB_CFG:\n", buf);
            SRVL_RD(VTSS_HSIO_SERDES1G_ANA_CFG_SERDES1G_OB_CFG, &x);
            SRVL_DEBUG_HSIO_FLD(pr, SERDES1G_ANA_CFG_SERDES1G_OB_CFG, OB_AMP_CTRL, x);

            pr("\n%s:COMMON_CFG:\n", buf);
            SRVL_RD(VTSS_HSIO_SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG, &x);
            SRVL_DEBUG_RAW(pr, 7, 1, x, "HRATE");
            SRVL_DEBUG_HSIO_BIT(pr, SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG, ENA_LANE, x);
            SRVL_DEBUG_HSIO_BIT(pr, SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG, RECO_SEL_A, x);
            SRVL_DEBUG_HSIO_BIT(pr, SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG, RECO_SEL_B, x);
            SRVL_DEBUG_HSIO_BIT(pr, SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG, SE_AUTO_SQUELCH_A_ENA, x);
            SRVL_DEBUG_HSIO_BIT(pr, SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG, SE_AUTO_SQUELCH_B_ENA, x);
             
            pr("\n%s:PLL_CFG:\n", buf);
            SRVL_RD(VTSS_HSIO_SERDES1G_ANA_CFG_SERDES1G_PLL_CFG, &x);
            SRVL_DEBUG_RAW(pr, 21, 1, x, "PLL_FSM_RC_DIV2");
            SRVL_DEBUG_HSIO_FLD(pr, SERDES1G_ANA_CFG_SERDES1G_PLL_CFG, PLL_FSM_CTRL_DATA, x);
                      
        }
        pr("\n");
    }
    pr("\n");

    if (vtss_state->sys_config.using_pcie) {
        inst = 2; /* Serdes2 == PCIe */
        sprintf(buf, "SerDes6G_%u (PCIe)", inst);
        (void)srvl_debug_serdes6g(vtss_state, pr, inst, buf);
    }

    return VTSS_RC_OK;
}

static void srvl_debug_cnt_inst(const vtss_debug_printf_t pr, u32 i,
                                const char *col1, const char *col2, 
                                vtss_chip_counter_t *c1, vtss_chip_counter_t *c2)
{
    char buf[80];
    
    sprintf(buf, "%s_%u", col1, i);
    vtss_srvl_debug_cnt(pr, buf, col2, c1, c2);
}

static vtss_rc srvl_debug_port_cnt(vtss_state_t *vtss_state,
                                   const vtss_debug_printf_t pr,
                                   const vtss_debug_info_t   *const info)
{
    /*lint --e{454, 455} */ // Due to VTSS_EXIT_ENTER
    vtss_port_no_t               port_no = 0;
    u32                          i, port;
    vtss_port_luton26_counters_t *cnt;
    BOOL                         cpu_port;
    
    for (port = 0; port <= VTSS_CHIP_PORTS; port++) {
        cpu_port = (port == VTSS_CHIP_PORT_CPU);
        if (cpu_port) {
            /* CPU port */
            if (!info->full)
                continue;
            cnt = &vtss_state->port.cpu_counters.counter.luton26;
            port_no = VTSS_PORT_NO_NONE;
        } else {
            /* Normal port */
            if ((port_no = vtss_cmn_port2port_no(vtss_state, info, port)) == VTSS_PORT_NO_NONE)
                continue;
            cnt = &vtss_state->port.counters[port_no].counter.luton26;
        }
        VTSS_RC(srvl_port_counters_read(vtss_state, port_no, port, cnt, NULL, 0));
        VTSS_EXIT_ENTER();

        /* Basic counters */
        if (cpu_port) {
            pr("Counters CPU port: %u\n\n", port);
        } else {
            pr("Counters for port: %u (%u):\n\n", port, port_no);
            vtss_srvl_debug_cnt(pr, "oct", "", &cnt->rx_octets, &cnt->tx_octets);
            vtss_srvl_debug_cnt(pr, "uc", "", &cnt->rx_unicast, &cnt->tx_unicast);
            vtss_srvl_debug_cnt(pr, "mc", "", &cnt->rx_multicast, &cnt->tx_multicast);
            vtss_srvl_debug_cnt(pr, "bc", "", &cnt->rx_broadcast, &cnt->tx_broadcast);
        }

        /* Detailed counters */
        if (info->full) {
            if (!cpu_port) {
                vtss_srvl_debug_cnt(pr, "pause", "", &cnt->rx_pause, &cnt->tx_pause);
                vtss_srvl_debug_cnt(pr, "64", "", &cnt->rx_64, &cnt->tx_64);
                vtss_srvl_debug_cnt(pr, "65_127", "", &cnt->rx_65_127, &cnt->tx_65_127);
                vtss_srvl_debug_cnt(pr, "128_255", "", &cnt->rx_128_255, &cnt->tx_128_255);
                vtss_srvl_debug_cnt(pr, "256_511", "", &cnt->rx_256_511, &cnt->tx_256_511);
                vtss_srvl_debug_cnt(pr, "512_1023", "", &cnt->rx_512_1023, &cnt->tx_512_1023);
                vtss_srvl_debug_cnt(pr, "1024_1526", "", &cnt->rx_1024_1526, &cnt->tx_1024_1526);
                vtss_srvl_debug_cnt(pr, "jumbo", "", &cnt->rx_1527_max, &cnt->tx_1527_max);
            }
            vtss_srvl_debug_cnt(pr, "cat_drop", cpu_port ? NULL : "drops", 
                           &cnt->rx_classified_drops, &cnt->tx_drops);
            vtss_srvl_debug_cnt(pr, "dr_local", cpu_port ? NULL : "aged", 
                           &cnt->dr_local, &cnt->tx_aging);
            vtss_srvl_debug_cnt(pr, "dr_tail", NULL, &cnt->dr_tail, NULL);
            if (!cpu_port) {
                vtss_srvl_debug_cnt(pr, "crc", NULL, &cnt->rx_crc_align_errors, NULL);
                vtss_srvl_debug_cnt(pr, "symbol", NULL, &cnt->rx_symbol_errors, NULL);
                vtss_srvl_debug_cnt(pr, "short", NULL, &cnt->rx_shorts, NULL);
                vtss_srvl_debug_cnt(pr, "long", NULL, &cnt->rx_longs, NULL);
                vtss_srvl_debug_cnt(pr, "frag", NULL, &cnt->rx_fragments, NULL);
                vtss_srvl_debug_cnt(pr, "jabber", NULL, &cnt->rx_jabbers, NULL);
                vtss_srvl_debug_cnt(pr, "control", NULL, &cnt->rx_control, NULL);
            }
            for (i = 0; i < VTSS_PRIOS; i++)
                srvl_debug_cnt_inst(pr, i, "green", "", 
                                    &cnt->rx_green_class[i], &cnt->tx_green_class[i]);
            for (i = 0; i < VTSS_PRIOS; i++)
                srvl_debug_cnt_inst(pr, i, "yellow", "", 
                                    &cnt->rx_yellow_class[i], &cnt->tx_yellow_class[i]);
            for (i = 0; i < VTSS_PRIOS; i++) 
                srvl_debug_cnt_inst(pr, i, "red", NULL, &cnt->rx_red_class[i], NULL);
            for (i = 0; i < VTSS_PRIOS; i++)
                srvl_debug_cnt_inst(pr, i, "dr_green", NULL, &cnt->dr_green_class[i], NULL);
            for (i = 0; i < VTSS_PRIOS; i++)
                srvl_debug_cnt_inst(pr, i, "dr_yellow", NULL, &cnt->dr_yellow_class[i], NULL);
        }
        pr("\n");
    }

    return VTSS_RC_OK;
}

static void srvl_debug_wm_dump(const vtss_debug_printf_t pr, const char *reg_name, u32 *value, u32 i, BOOL decode)
{
    u32 q;
    pr("%-25s", reg_name);
    for (q = 0; q < i; q++) {
        pr("%6u ", decode ? vtss_srvl_wm_dec(value[q])*SRVL_BUFFER_CELL_SZ : value[q]);
    }
    pr("\n");
}

static vtss_rc srvl_debug_wm(vtss_state_t *vtss_state,
                             const vtss_debug_printf_t pr,
                             const vtss_debug_info_t   *const info)

{
    u32 port_no, value, q, dp, cpu_port, port;
    u32 id[8], val1[8], val2[8], val3[8], val4[8];
    

    for (port_no = VTSS_PORT_NO_START; port_no <= vtss_state->port_count; port_no++) {
        cpu_port = (port_no == vtss_state->port_count);
        if (cpu_port) {
            /* CPU port */
            if (!info->full)
                continue;
            port = VTSS_CHIP_PORT_CPU;
        } else {
            if (!info->port_list[port_no]) {
                continue;
            }
            port = VTSS_CHIP_PORT(port_no);
        }
        if (cpu_port) 
            pr("CPU_Port          : %u\n\n",port_no);
        else
            pr("Port          : %u\n\n",port_no);
        if (!cpu_port) {
            if ((port_no = vtss_cmn_port2port_no(vtss_state, info, port)) == VTSS_PORT_NO_NONE)
                continue;

            SRVL_RD(VTSS_SYS_PAUSE_CFG_MAC_FC_CFG(VTSS_CHIP_PORT(port_no)), &value);
            pr("FC Tx ena     : %d\n", (value & VTSS_F_SYS_PAUSE_CFG_MAC_FC_CFG_TX_FC_ENA) ? 1 : 0);
            pr("FC Rx ena     : %d\n", (value & VTSS_F_SYS_PAUSE_CFG_MAC_FC_CFG_RX_FC_ENA) ? 1 : 0);
            pr("FC Value      : 0x%x\n", VTSS_F_SYS_PAUSE_CFG_MAC_FC_CFG_PAUSE_VAL_CFG(value));
            pr("FC Zero pause : %d\n", (value & VTSS_F_SYS_PAUSE_CFG_MAC_FC_CFG_ZERO_PAUSE_ENA) ? 1 : 0);          
            SRVL_RD(VTSS_SYS_PAUSE_CFG_PAUSE_CFG(VTSS_CHIP_PORT(port_no)), &value);
            pr("FC Start      : 0x%x\n",  VTSS_X_SYS_PAUSE_CFG_PAUSE_CFG_PAUSE_START(value));
            pr("FC Stop       : 0x%x\n", VTSS_X_SYS_PAUSE_CFG_PAUSE_CFG_PAUSE_STOP(value));
            pr("FC Ena        : %d\n", (value & VTSS_F_SYS_PAUSE_CFG_PAUSE_CFG_PAUSE_ENA) ? 1 : 0);
            SRVL_RD(VTSS_SYS_PAUSE_CFG_ATOP(VTSS_CHIP_PORT(port_no)), &value);
            pr("FC Atop       : 0x%x\n", VTSS_F_SYS_PAUSE_CFG_ATOP_ATOP(value));                  
            SRVL_RD(VTSS_SYS_PAUSE_CFG_PAUSE_TOT_CFG, &value);
            pr("FC TOT_START  : 0x%x\n", VTSS_X_SYS_PAUSE_CFG_PAUSE_TOT_CFG_PAUSE_TOT_START(value));
            pr("FC TOT_STOP   : 0x%x\n", VTSS_X_SYS_PAUSE_CFG_PAUSE_TOT_CFG_PAUSE_TOT_STOP(value));
            SRVL_RD(VTSS_SYS_PAUSE_CFG_ATOP_TOT_CFG, &value);
            pr("FC ATOP_TOT   : 0x%x\n", VTSS_F_SYS_PAUSE_CFG_ATOP_TOT_CFG_ATOP_TOT(value));
            pr("\n");
        }
        
        for (q = 0; q < VTSS_PRIOS; q++) {
            id[q] = q;
            SRVL_RD(VTSS_QSYS_RES_CTRL_RES_CFG((port * VTSS_PRIOS + q + 0)),   &val1[q]);
            SRVL_RD(VTSS_QSYS_RES_CTRL_RES_CFG((port * VTSS_PRIOS + q + 256)), &val2[q]);
            SRVL_RD(VTSS_QSYS_RES_CTRL_RES_CFG((port * VTSS_PRIOS + q + 512)), &val3[q]);
            SRVL_RD(VTSS_QSYS_RES_CTRL_RES_CFG((port * VTSS_PRIOS + q + 768)), &val4[q]);
        }
        
        srvl_debug_wm_dump(pr, "Queue level:", id, 8, 0);
        srvl_debug_wm_dump(pr, "Qu Ingr Buf Rsrv (Bytes) :", val1, 8, 1);
        srvl_debug_wm_dump(pr, "Qu Ingr Ref Rsrv (Frames):", val2, 8, 0);
        srvl_debug_wm_dump(pr, "Qu Egr Buf Rsrv  (Bytes) :", val3, 8, 1);
        srvl_debug_wm_dump(pr, "Qu Egr Ref Rsrv  (Frames):", val4, 8, 0);
        pr("\n");
            
        /* Shared space for all QoS classes */
        for (q = 0; q < VTSS_PRIOS; q++) {
            SRVL_RD(VTSS_QSYS_RES_CTRL_RES_CFG((q + 216 + 0)),   &val1[q]);
            SRVL_RD(VTSS_QSYS_RES_CTRL_RES_CFG((q + 216 + 256)), &val2[q]);
            SRVL_RD(VTSS_QSYS_RES_CTRL_RES_CFG((q + 216 + 512)), &val3[q]);
            SRVL_RD(VTSS_QSYS_RES_CTRL_RES_CFG((q + 216 + 768)), &val4[q]);
        }
        srvl_debug_wm_dump(pr, "QoS level:", id, 8, 0);
        srvl_debug_wm_dump(pr, "QoS Ingr Buf Rsrv (Bytes) :", val1, 8, 1);
        srvl_debug_wm_dump(pr, "QoS Ingr Ref Rsrv (Frames):", val2, 8, 0);
        srvl_debug_wm_dump(pr, "QoS Egr Buf Rsrv  (Bytes) :", val3, 8, 1);
        srvl_debug_wm_dump(pr, "QoS Egr Ref Rsrv  (Frames):", val4, 8, 0);
        pr("\n");
        /* Configure reserved space for port */
        SRVL_RD(VTSS_QSYS_RES_CTRL_RES_CFG((port + 224 +   0)), &val1[0]);
        SRVL_RD(VTSS_QSYS_RES_CTRL_RES_CFG((port + 224 + 256)), &val2[0]);
        SRVL_RD(VTSS_QSYS_RES_CTRL_RES_CFG((port + 224 + 512)), &val3[0]);
        SRVL_RD(VTSS_QSYS_RES_CTRL_RES_CFG((port + 224 + 768)), &val4[0]);
        pr("Port level:\n");
        pr("Port Ingress Buf Rsrv: %u Bytes\n", vtss_srvl_wm_dec(val1[0])*SRVL_BUFFER_CELL_SZ);
        pr("Port Ingress Ref Rsrv: %u Frames\n", val2[0]);
        pr("Port Egress  Buf Rsrv: %u Bytes\n", vtss_srvl_wm_dec(val3[0])*SRVL_BUFFER_CELL_SZ);
        pr("Port Egress  Ref Rsrv: %u Frames\n", val4[0]);
        pr("\n");
        
        pr("Color level:\n");
        /* Configure shared space for  both DP levels (green:0 yellow:1) */
            for (dp = 0; dp < 2; dp++) {
                SRVL_RD(VTSS_QSYS_RES_CTRL_RES_CFG((dp + 254 +   0)), &val1[0]);
                SRVL_RD(VTSS_QSYS_RES_CTRL_RES_CFG((dp + 254 + 256)), &val2[0]);
                SRVL_RD(VTSS_QSYS_RES_CTRL_RES_CFG((dp + 254 + 512)), &val3[0]);
                SRVL_RD(VTSS_QSYS_RES_CTRL_RES_CFG((dp + 254 + 768)), &val4[0]);
                pr("Port DP:%6s Ingress Buf Rsrv: 0x%x\n",dp?"Green":"Yellow",val1[0]);
                pr("Port DP:%6s Ingress Ref Rsrv: 0x%x\n",dp?"Green":"Yellow",val2[0]);
                pr("Port DP:%6s Egress  Buf Rsrv: 0x%x\n",dp?"Green":"Yellow",val3[0]);
                pr("Port DP:%6s Egress  Ref Rsrv: 0x%x\n",dp?"Green":"Yellow",val4[0]);
            }
            pr("\n");
    }
    return VTSS_RC_OK;
}

vtss_rc vtss_srvl_port_debug_print(vtss_state_t *vtss_state,
                                   const vtss_debug_printf_t pr,
                                   const vtss_debug_info_t   *const info)
{
    VTSS_RC(vtss_debug_print_group(VTSS_DEBUG_GROUP_PORT, srvl_debug_port, vtss_state, pr, info));
    VTSS_RC(vtss_debug_print_group(VTSS_DEBUG_GROUP_PORT_CNT, srvl_debug_port_cnt, vtss_state, pr, info));
    VTSS_RC(vtss_debug_print_group(VTSS_DEBUG_GROUP_WM, srvl_debug_wm, vtss_state, pr, info));
    return VTSS_RC_OK;
}

/* - Initialization ------------------------------------------------ */

vtss_rc vtss_srvl_port_init(vtss_state_t *vtss_state, vtss_init_cmd_t cmd)
{
    vtss_port_state_t *state = &vtss_state->port;
    u32               port;

    switch (cmd) {
    case VTSS_INIT_CMD_CREATE:
        state->miim_read = srvl_miim_read;
        state->miim_write = srvl_miim_write;
        state->conf_get = srvl_port_conf_get;
        state->conf_set = srvl_port_conf_set;
        state->clause_37_status_get = srvl_port_clause_37_status_get;
        state->clause_37_control_get = srvl_port_clause_37_control_get;
        state->clause_37_control_set = srvl_port_clause_37_control_set;
        state->status_get = srvl_port_status_get;
        state->counters_update = srvl_port_counters_update;
        state->counters_clear = srvl_port_counters_clear;
        state->counters_get = srvl_port_counters_get;
        state->basic_counters_get = srvl_port_basic_counters_get;
        state->ifh_set = srvl_port_ifh_set;
        state->forward_set = srvl_port_forward_set;
        
        /* SYNCE features */
        vtss_state->synce.clock_out_set = srvl_synce_clock_out_set;
        vtss_state->synce.clock_in_set = srvl_synce_clock_in_set;
        break;
    case VTSS_INIT_CMD_INIT:
        /* Tweak the default PCIe settings */
        if (vtss_state->sys_config.using_pcie) {
            VTSS_I("PCIe serdes setup");
            u32 serdes_instance = 2;
            VTSS_RC(srvl_sd6g_read(vtss_state, 1<<serdes_instance));
            SRVL_WRM(VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_DES_CFG,
                     VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_DES_CFG_DES_PHS_CTRL(3) |
                     VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_DES_CFG_DES_BW_ANA(3)   |
                     VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_DES_CFG_DES_MBTR_CTRL(0),
                     VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_DES_CFG_DES_PHS_CTRL    |
                     VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_DES_CFG_DES_BW_ANA      |
                     VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_DES_CFG_DES_MBTR_CTRL
                );
            VTSS_RC(srvl_sd6g_write(vtss_state, 1<<serdes_instance, 0));
        }
        /* Clear port counters */
        for (port = 0; port <= VTSS_CHIP_PORTS; port++) {
            SRVL_WR(VTSS_SYS_SYSTEM_STAT_CFG,
                    VTSS_F_SYS_SYSTEM_STAT_CFG_STAT_CLEAR_SHOT(0x7) |
                    VTSS_F_SYS_SYSTEM_STAT_CFG_STAT_VIEW(port));
        }
        break;
    case VTSS_INIT_CMD_PORT_MAP:
        if (!vtss_state->warm_start_cur) {
            VTSS_RC(srvl_port_buf_conf_set(vtss_state));
        }
        break;
    default:
        break;
    }
    return VTSS_RC_OK;
}

#endif /* VTSS_ARCH_SERVAL */
