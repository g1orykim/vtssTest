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
#include "vtss_jaguar1_cil.h"

#if defined(VTSS_ARCH_JAGUAR_1)

/* - CIL functions ------------------------------------------------- */

/* Update MTU */
vtss_rc vtss_jr1_setup_mtu(vtss_state_t *vtss_state, u32 chipport, u32 mtu, BOOL front_port)
{
#if defined(VTSS_FEATURE_VSTAX_V2)
    if (!front_port)
        mtu += VTSS_VSTAX_HDR_SIZE; /* 12 bytes extra MTU */
#endif /* VTSS_FEATURE_VSTAX_V2 */
    if(VTSS_PORT_IS_1G(chipport)) {
        JR_WRX(DEV1G, MAC_CFG_STATUS_MAC_MAXLEN_CFG, VTSS_TO_DEV(chipport), mtu);
    } else {
        JR_WRXM(DEV10G, MAC_CFG_STATUS_MAC_MAXLEN_CFG, VTSS_TO_DEV(chipport), 
                VTSS_F_DEV10G_MAC_CFG_STATUS_MAC_MAXLEN_CFG_MAX_LEN(mtu) |
                VTSS_F_DEV10G_MAC_CFG_STATUS_MAC_MAXLEN_CFG_MAX_LEN_TAG_CHK,
                VTSS_M_DEV10G_MAC_CFG_STATUS_MAC_MAXLEN_CFG_MAX_LEN |
                VTSS_F_DEV10G_MAC_CFG_STATUS_MAC_MAXLEN_CFG_MAX_LEN_TAG_CHK);
    }
    return VTSS_RC_OK;
}

vtss_rc vtss_jr1_port_max_tags_set(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
    vtss_port_max_tags_t  max_tags = vtss_state->port.conf[port_no].max_tags;
    vtss_vlan_port_type_t vlan_type = vtss_state->l2.vlan_port_conf[port_no].port_type;
    u32                   port = VTSS_CHIP_PORT(port_no);
    u32                   tgt = VTSS_TO_DEV(port);
    u32                   etype, double_ena, single_ena;

    /* S-ports and VLAN unaware ports both support 0x88a8 (in addition to 0x8100) */
    etype = (vlan_type == VTSS_VLAN_PORT_TYPE_S_CUSTOM ? vtss_state->l2.vlan_conf.s_etype :
             vlan_type == VTSS_VLAN_PORT_TYPE_C ? 0x8100 : 0x88a8);
    single_ena = (max_tags == VTSS_PORT_MAX_TAGS_NONE ? 0 : 1);
    double_ena = (max_tags == VTSS_PORT_MAX_TAGS_TWO ? 1 : 0);
    
    if (VTSS_PORT_IS_10G(port)) {
        JR_WRX(DEV10G, MAC_CFG_STATUS_MAC_TAGS_CFG, tgt,
               JR_PUT_FLD(DEV10G, MAC_CFG_STATUS_MAC_TAGS_CFG, TAG_ID, etype) |
               JR_PUT_BIT(DEV10G, MAC_CFG_STATUS_MAC_TAGS_CFG, DOUBLE_TAG_ENA, double_ena) | 
               JR_PUT_BIT(DEV10G, MAC_CFG_STATUS_MAC_TAGS_CFG, VLAN_AWR_ENA, single_ena));
    } else {
        JR_WRX(DEV1G, MAC_CFG_STATUS_MAC_TAGS_CFG, tgt,
               JR_PUT_FLD(DEV1G, MAC_CFG_STATUS_MAC_TAGS_CFG, TAG_ID, etype) |
               JR_PUT_BIT(DEV1G, MAC_CFG_STATUS_MAC_TAGS_CFG, PB_ENA, double_ena) | 
               JR_PUT_BIT(DEV1G, MAC_CFG_STATUS_MAC_TAGS_CFG, VLAN_AWR_ENA, single_ena) |
               JR_PUT_BIT(DEV1G, MAC_CFG_STATUS_MAC_TAGS_CFG, VLAN_LEN_AWR_ENA, 1));
    }
    return VTSS_RC_OK;
}

BOOL vtss_jr1_cemax_port_is_host(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC)
    return (VTSS_CHIP_PORT(port_no) == vtss_state->port.ce_mac_hmda ||
            VTSS_CHIP_PORT(port_no) == vtss_state->port.ce_mac_hmdb);
#else
    return FALSE;
#endif
}

/* PHY commands */
#define PHY_CMD_ADDRESS  0 /* 10G: Address */
#define PHY_CMD_WRITE    1 /* 1G/10G: Write */
#define PHY_CMD_READ_INC 2 /* 1G: Read, 10G: Read and increment */
#define PHY_CMD_READ     3 /* 10G: Read */

static vtss_rc jr_miim_cmd(vtss_state_t *vtss_state,
                           u32 cmd, u32 sof, vtss_miim_controller_t miim_controller,
                           u8 miim_addr, u8 addr, u16 *data, BOOL report_errors)
{
    u32 i, n;
    u32 value;

    switch (miim_controller) {
    case VTSS_MIIM_CONTROLLER_0:
        i = 0;
        break;
    case VTSS_MIIM_CONTROLLER_1:
        i = 1;
        break;
    default:
        VTSS_E("illegal miim_controller: %d", miim_controller);
        return VTSS_RC_ERROR;
    }

    /* Set Start of frame field */
    JR_WRX(DEVCPU_GCB, MIIM_MII_CFG, i,
           VTSS_F_DEVCPU_GCB_MIIM_MII_CFG_MIIM_CFG_PRESCALE(0x32) |
           VTSS_F_DEVCPU_GCB_MIIM_MII_CFG_MIIM_ST_CFG_FIELD(sof));

    /* Read command is different for Clause 22 */
    if (sof == 1 && cmd == PHY_CMD_READ) {
        cmd = PHY_CMD_READ_INC;
    }

    /* Start command */
    JR_WRX(DEVCPU_GCB, MIIM_MII_CMD, i,
           VTSS_F_DEVCPU_GCB_MIIM_MII_CMD_MIIM_CMD_VLD |
           VTSS_F_DEVCPU_GCB_MIIM_MII_CMD_MIIM_CMD_PHYAD(miim_addr) |
           VTSS_F_DEVCPU_GCB_MIIM_MII_CMD_MIIM_CMD_REGAD(addr) |
           VTSS_F_DEVCPU_GCB_MIIM_MII_CMD_MIIM_CMD_WRDATA(*data) |
           VTSS_F_DEVCPU_GCB_MIIM_MII_CMD_MIIM_CMD_OPR_FIELD(cmd));

    /* Wait for access to complete */
    for (n = 0; ; n++) {
        JR_RDX(DEVCPU_GCB, MIIM_MII_STATUS, i, &value);
        if (JR_GET_BIT(DEVCPU_GCB, MIIM_MII_STATUS, MIIM_STAT_PENDING_RD, value) == 0 &&
                JR_GET_BIT(DEVCPU_GCB, MIIM_MII_STATUS, MIIM_STAT_PENDING_WR, value) == 0) {
            break;
        }
        if (n == 1000) {
            goto mmd_error;
        }
    }

    /* Read data */
    if (cmd == PHY_CMD_READ || cmd == PHY_CMD_READ_INC) {
        JR_RDX(DEVCPU_GCB, MIIM_MII_DATA, i, &value);
        if (JR_GET_FLD(DEVCPU_GCB, MIIM_MII_DATA, MIIM_DATA_SUCCESS, value)) {
            goto mmd_error;
        }
        *data = JR_GET_FLD(DEVCPU_GCB, MIIM_MII_DATA, MIIM_DATA_RDDATA, value);
    }
    return VTSS_RC_OK;

mmd_error:
    if(report_errors) {
        VTSS_E("miim failed, cmd: %s, miim_addr: %u, addr: %u, miim_controller:%u, sof:%u",
               cmd == PHY_CMD_READ ? "PHY_CMD_READ" :
               cmd == PHY_CMD_WRITE ? "PHY_CMD_WRITE" :
               cmd == PHY_CMD_ADDRESS ? "PHY_CMD_ADDRESS" : "PHY_CMD_READ_INC",
               miim_addr, addr, miim_controller, sof);
    }

    return VTSS_RC_ERROR;
}

static vtss_rc jr_miim_read(vtss_state_t *vtss_state,
                            vtss_miim_controller_t miim_controller,
                            u8 miim_addr,
                            u8 addr,
                            u16 *value, 
                            BOOL report_errors)
{
    return jr_miim_cmd(vtss_state, PHY_CMD_READ, 1, miim_controller, miim_addr, addr,
                       value, report_errors);
}

static vtss_rc jr_miim_write(vtss_state_t *vtss_state,
                             vtss_miim_controller_t miim_controller,
                             u8 miim_addr,
                             u8 addr,
                             u16 value, 
                             BOOL report_errors)
{
    return jr_miim_cmd(vtss_state, PHY_CMD_WRITE, 1, miim_controller, miim_addr, addr,
                       &value, report_errors);
}

/* MMD (MDIO Management Devices (10G)) read */
/* static vtss_rc jr_mmd_read(vtss_state_t *vtss_state, */
/*                            vtss_miim_controller_t miim_controller, u8 miim_addr, u8 mmd, */
/*                            u16 addr, u16 *value, BOOL report_errors) */
/* { */
/*     VTSS_RC(jr_miim_cmd(vtss_state, PHY_CMD_ADDRESS, 0, miim_controller, miim_addr, mmd, */
/*                         &addr, report_errors)); */
/*     return jr_miim_cmd(vtss_state, PHY_CMD_READ, 0, miim_controller, miim_addr, mmd, */
/*                        value, report_errors); */
/* } */

static vtss_rc jr_mmd_read(vtss_state_t *vtss_state,
                           vtss_miim_controller_t miim_controller, u8 miim_addr, u8 mmd,
                           u16 addr, u16 *value, BOOL report_errors)
{

    VTSS_RC(jr_miim_cmd(vtss_state, PHY_CMD_ADDRESS, 0, miim_controller, miim_addr, mmd,
                        &addr, report_errors));
    if (jr_miim_cmd(vtss_state, PHY_CMD_READ, 0, miim_controller, miim_addr, mmd,
                    value, 0) != VTSS_RC_OK) {
        if (report_errors) {
            VTSS_E("jr_mmd_read failed, miim_controller:%d miim_addr:%x, mmd:%x, addr:%x",miim_controller, miim_addr, mmd, addr);
        }
        return VTSS_RC_ERROR;
    }
    return VTSS_RC_OK;
}

/* MMD (MDIO Management Devices (10G)) read-inc */
static vtss_rc jr_mmd_read_inc(vtss_state_t *vtss_state,
                               vtss_miim_controller_t miim_controller, u8 miim_addr, u8 mmd,
                               u16 addr, u16 *buf, u8 count, BOOL report_errors)
{

    VTSS_RC(jr_miim_cmd(vtss_state, PHY_CMD_ADDRESS, 0, miim_controller, miim_addr, mmd,
                        &addr, report_errors));
    while (count > 0) {
        VTSS_RC(jr_miim_cmd(vtss_state, PHY_CMD_READ_INC, 0, miim_controller, miim_addr, mmd,
                            buf, report_errors));
        buf++;
        count--;
    }
    return VTSS_RC_OK;
}


/* MMD (MDIO Management Devices (10G)) write */
static vtss_rc jr_mmd_write(vtss_state_t *vtss_state,
                            vtss_miim_controller_t miim_controller,
                            u8 miim_addr, u8 mmd, u16 addr, u16 data,  BOOL report_errors)
{
    VTSS_RC(jr_miim_cmd(vtss_state, PHY_CMD_ADDRESS, 0, miim_controller, miim_addr, mmd,
                        &addr, report_errors));
    return jr_miim_cmd(vtss_state, PHY_CMD_WRITE, 0, miim_controller, miim_addr, mmd,
                       &data, report_errors);
}

/* ================================================================= *
 *  SQS / WM setup
 * ================================================================= */

u32 vtss_jr1_ce_max_hm(vtss_state_t *vtss_state)
{
#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC)   
    switch (vtss_state->init_conf.host_mode) {
#if defined(VTSS_CHIP_CE_MAX_24)
    case VTSS_HOST_MODE_0:
        return 0;        
    case VTSS_HOST_MODE_1:
        return 1;        
#endif /* VTSS_CHIP_CE_MAX_24 */
    case VTSS_HOST_MODE_2:
        return 2;        
    case VTSS_HOST_MODE_3:
        return 3;        
    default:
        return 0;        
    }
#else
    return 0;
#endif
}

typedef struct {
    u32 atop;
    u32 hwm;
    u32 lwm;
    u32 swm;
    u32 fwd_press;
    BOOL cnt_on_port;
    BOOL cnt_on_prio;
    BOOL cnt_on_buf_port;
    BOOL cnt_on_buf_prio;
    BOOL rc_mode;  // 0: Normal 1: Minimum guaranteed
    BOOL fc_drop_mode;  // 0: FC     1: Drop
    BOOL rx_ena;
    u32 mtu_pre_alloc;
} sqs_qu_t;

typedef struct {
    u32 atop;
    u32 hwm;
    u32 lwm;
    u32 swm;
    u32 gwm;
    // GWM not used in SME
    u32 cmwm; // only OQS
    BOOL cnt_on_buf;
    BOOL rc_mode;  // 0: Normal 1: Minimum guaranteed
    BOOL fc_drop_mode;  // 0: FC     1: Drop
    BOOL rx_ena;
    sqs_qu_t qu[8];
} sqs_port_t;

typedef struct {
    u32 atop;
    u32 hwm;
    u32 lwm;
    BOOL fc_drop_mode;  // 0: FC     1: Drop
    BOOL rx_ena;
    u32 pre_alloc_frm_size[4];
    sqs_port_t port;
} sqs_buf_t;  // BUF-port - BUF-prio not used in this setup.

typedef struct {
    sqs_buf_t iqs;
    sqs_buf_t oqs;
} sqs_t;

static vtss_rc jr_sqs_rd(vtss_state_t *vtss_state, u32 port_no, sqs_t *sqs)
{
    u32 q, qu_id, qs, i, value, prios=8;
    sqs_buf_t *psqs;

    if ((vtss_jr1_ce_max_hm(vtss_state) == 1) || (vtss_jr1_ce_max_hm(vtss_state) == 3)) {
        prios = 4;
    } 

   /*  Queue Level  */
    for (qs = 0; qs < 2; qs++) { /* 0 = IQS, 1 = OQS */
        if ((qs == 0) && (port_no > 34)) {
            continue;
        }
        if (qs == 0) {
            psqs = &sqs->iqs;
        } else {
            psqs = &sqs->oqs;
        }

        if (port_no > 80) {
            prios = 1; /* CPU OQS qu */
        }

        for (q = 0; q < prios; q++) { 
            qu_id = vtss_jr1_port_prio_2_qu(vtss_state, port_no, q, qs);

            RDX_SQS(qs, QU_RAM_CFG_QU_RC_CFG, qu_id, &value);
            psqs->port.qu[q].fwd_press = PST_SQS(VTSS_X, qs, QU_RAM_CFG_QU_RC_CFG_FWD_PRESS_THRES(value));
            psqs->port.qu[q].cnt_on_port = PST_SQS(VTSS_F, qs, QU_RAM_CFG_QU_RC_CFG_QU_CNT_ON_PORT_LVL_ENA) & value ? 1 : 0;
            psqs->port.qu[q].cnt_on_buf_prio = PST_SQS(VTSS_F, qs, QU_RAM_CFG_QU_RC_CFG_QU_CNT_ON_BUF_PRIO_LVL_ENA) & value ? 1 : 0;
            psqs->port.qu[q].cnt_on_buf_port = PST_SQS(VTSS_F, qs, QU_RAM_CFG_QU_RC_CFG_QU_CNT_ON_BUF_PORT_LVL_ENA) & value ? 1 : 0;
            psqs->port.qu[q].rc_mode = PST_SQS(VTSS_F, qs, QU_RAM_CFG_QU_RC_CFG_QU_RC_MODE) & value ? 1 : 0;
            psqs->port.qu[q].fc_drop_mode = PST_SQS(VTSS_F, qs, QU_RAM_CFG_QU_RC_CFG_QU_FC_DROP_MODE) & value ? 1 : 0;
            psqs->port.qu[q].rx_ena = PST_SQS(VTSS_F, qs, QU_RAM_CFG_QU_RC_CFG_RX_QU_ENA) & value ? 1 : 0;
            RDX_SQS(qs, QU_RAM_CFG_QU_RC_HLWM_CFG, qu_id, &value);
            psqs->port.qu[q].hwm = PST_SQS(VTSS_X, qs, QU_RAM_CFG_QU_RC_HLWM_CFG_QU_RC_PROFILE_HWM(value));
            psqs->port.qu[q].lwm = PST_SQS(VTSS_X, qs, QU_RAM_CFG_QU_RC_HLWM_CFG_QU_RC_PROFILE_LWM(value));
            RDX_SQS(qs, QU_RAM_CFG_QU_RC_ATOP_SWM_CFG, qu_id, &value);
            psqs->port.qu[q].atop = PST_SQS(VTSS_X, qs, QU_RAM_CFG_QU_RC_ATOP_SWM_CFG_QU_RC_PROFILE_ATOP(value));
            psqs->port.qu[q].swm  = PST_SQS(VTSS_X, qs, QU_RAM_CFG_QU_RC_ATOP_SWM_CFG_QU_RC_PROFILE_SWM(value));
            RDX_SQS(qs, QU_RAM_CFG_MTU_QU_MAP_CFG, qu_id, &psqs->port.qu[q].mtu_pre_alloc);
        }
        
        /* Port Level */                  
        RDX_SQS(qs, PORT_RAM_CFG_PORT_RC_CFG, port_no, &value); 
        psqs->port.rx_ena       = PST_SQS(VTSS_F, qs, PORT_RAM_CFG_PORT_RC_CFG_RX_PORT_ENA) & value ? 1 : 0;     
        psqs->port.cnt_on_buf   = PST_SQS(VTSS_F, qs, PORT_RAM_CFG_PORT_RC_CFG_PORT_CNT_ON_BUF_LVL_ENA) & value ? 1 : 0;
        psqs->port.fc_drop_mode = PST_SQS(VTSS_F, qs, PORT_RAM_CFG_PORT_RC_CFG_PORT_FC_DROP_MODE) & value ? 1 : 0;
        psqs->port.rc_mode      = PST_SQS(VTSS_F, qs, PORT_RAM_CFG_PORT_RC_CFG_PORT_RC_MODE) & value ? 1 : 0;  
        RDX_SQS(qs, PORT_RAM_CFG_PORT_RC_HLWM_CFG, port_no, &value);
        psqs->port.hwm          = PST_SQS(VTSS_X, qs, PORT_RAM_CFG_PORT_RC_HLWM_CFG_PORT_RC_PROFILE_HWM(value));
        psqs->port.lwm          = PST_SQS(VTSS_X, qs, PORT_RAM_CFG_PORT_RC_HLWM_CFG_PORT_RC_PROFILE_LWM(value));
        RDX_SQS(qs, PORT_RAM_CFG_PORT_RC_ATOP_SWM_CFG, port_no, &value);
        psqs->port.atop          = PST_SQS(VTSS_X, qs, PORT_RAM_CFG_PORT_RC_ATOP_SWM_CFG_PORT_RC_PROFILE_ATOP(value));   
        psqs->port.swm           = PST_SQS(VTSS_X, qs, PORT_RAM_CFG_PORT_RC_ATOP_SWM_CFG_PORT_RC_PROFILE_SWM(value));   
        RDX_SQS(qs, PORT_RAM_CFG_PORT_RC_GWM_CFG, port_no, &value);
        psqs->port.gwm           = PST_SQS(VTSS_X, qs, PORT_RAM_CFG_PORT_RC_ATOP_SWM_CFG_PORT_RC_PROFILE_SWM(value));

        /* Buffer Port Level */
        RD_SQS(qs, RESOURCE_CTRL_CFG_BUF_RC_CFG, &value);
        psqs->rx_ena       = PST_SQS(VTSS_F, qs, RESOURCE_CTRL_CFG_BUF_RC_CFG_RX_BUF_ENA) & value ? 1 : 0; 
        psqs->fc_drop_mode = PST_SQS(VTSS_F, qs, RESOURCE_CTRL_CFG_BUF_RC_CFG_BUF_PORT_FC_DROP_MODE) & value ? 1 : 0;
        psqs->atop         = PST_SQS(VTSS_X, qs, RESOURCE_CTRL_CFG_BUF_RC_CFG_BUF_PORT_RC_PROFILE_ATOP(value));;
        RD_SQS(qs, RESOURCE_CTRL_CFG_BUF_PORT_RC_HLWM_CFG, &value);
        psqs->hwm          = PST_SQS(VTSS_X, qs, RESOURCE_CTRL_CFG_BUF_PORT_RC_HLWM_CFG_BUF_PORT_RC_PROFILE_HWM(value));
        psqs->lwm          = PST_SQS(VTSS_X, qs, RESOURCE_CTRL_CFG_BUF_PORT_RC_HLWM_CFG_BUF_PORT_RC_PROFILE_LWM(value));
        for (i = 0; i < 4; i++) {         
            RDX_SQS(qs, MTU_MTU_FRM_SIZE_CFG, i, &psqs->pre_alloc_frm_size[i]);
        }
    }   
    return VTSS_RC_OK;     
}

static vtss_rc jr_sqs_set(vtss_state_t *vtss_state, u32 port_no, sqs_t *sqs)
{
    u32 q, qu_id, qs, i, prios=8;
    sqs_buf_t *psqs;

    if ((vtss_jr1_ce_max_hm(vtss_state) == 1) || (vtss_jr1_ce_max_hm(vtss_state) == 3)) {
        prios = 4;
    } 
   /*  Queue Level  */
    for (qs = 0; qs < 2; qs++) { /* 0 = IQS, 1 = OQS */
        if ((qs == 0) && (port_no > 34)) {
            continue;
        } 
#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC)   
        if ((prios == 8) && (port_no > 55))
            continue; /* nore more queues */
#else
        if ((qs == 1) && (port_no > 31 && port_no < 81))
            prios = 4; /* Reserved host queues */   
#endif

        if (qs == 0) {
            psqs = &sqs->iqs;
        } else {
            psqs = &sqs->oqs;
        }

        if (port_no > 80) {
            prios = 1; /* CPU OQS qu */
        }

        for (q = 0; q < prios; q++) { 
            qu_id = vtss_jr1_port_prio_2_qu(vtss_state, port_no, q, qs);

            WRX_SQS(qs, QU_RAM_CFG_QU_RC_CFG, qu_id,
                    PST_SQS(VTSS_F, qs, QU_RAM_CFG_QU_RC_CFG_FWD_PRESS_THRES(psqs->port.qu[q].fwd_press)) | VTSS_BIT(7) |
                    (psqs->port.qu[q].cnt_on_prio ? PST_SQS(VTSS_F, qs, QU_RAM_CFG_QU_RC_CFG_QU_CNT_ON_PRIO_LVL_ENA) : 0) |
                    (psqs->port.qu[q].cnt_on_port ? PST_SQS(VTSS_F, qs, QU_RAM_CFG_QU_RC_CFG_QU_CNT_ON_PORT_LVL_ENA) : 0) |
                    (psqs->port.qu[q].cnt_on_buf_prio ? PST_SQS(VTSS_F, qs, QU_RAM_CFG_QU_RC_CFG_QU_CNT_ON_BUF_PRIO_LVL_ENA) : 0) |
                    (psqs->port.qu[q].cnt_on_buf_port ? PST_SQS(VTSS_F, qs, QU_RAM_CFG_QU_RC_CFG_QU_CNT_ON_BUF_PORT_LVL_ENA) : 0) |
                    (psqs->port.qu[q].rc_mode ? PST_SQS(VTSS_F, qs, QU_RAM_CFG_QU_RC_CFG_QU_RC_MODE) : 0) |
                    (psqs->port.qu[q].fc_drop_mode ? PST_SQS(VTSS_F, qs, QU_RAM_CFG_QU_RC_CFG_QU_FC_DROP_MODE) : 0) |
                    (psqs->port.qu[q].rx_ena ? PST_SQS(VTSS_F, qs, QU_RAM_CFG_QU_RC_CFG_RX_QU_ENA) : 0));
                
            WRX_SQS(qs, QU_RAM_CFG_QU_RC_HLWM_CFG, qu_id,
                    PST_SQS(VTSS_F, qs, QU_RAM_CFG_QU_RC_HLWM_CFG_QU_RC_PROFILE_HWM(psqs->port.qu[q].hwm)) |
                    PST_SQS(VTSS_F, qs, QU_RAM_CFG_QU_RC_HLWM_CFG_QU_RC_PROFILE_LWM(psqs->port.qu[q].lwm)));
                
            WRX_SQS(qs, QU_RAM_CFG_QU_RC_ATOP_SWM_CFG, qu_id,
                    PST_SQS(VTSS_F, qs, QU_RAM_CFG_QU_RC_ATOP_SWM_CFG_QU_RC_PROFILE_ATOP(psqs->port.qu[q].atop)) |
                    PST_SQS(VTSS_F, qs, QU_RAM_CFG_QU_RC_ATOP_SWM_CFG_QU_RC_PROFILE_SWM(psqs->port.qu[q].swm)));
            
            WRX_SQS(qs, QU_RAM_CFG_MTU_QU_MAP_CFG, qu_id, psqs->port.qu[q].mtu_pre_alloc);            

        }
 
        /*  Port Level  */
        WRX_SQS(qs, PORT_RAM_CFG_PORT_RC_CFG, port_no,
                (psqs->port.rx_ena ? PST_SQS(VTSS_F, qs, PORT_RAM_CFG_PORT_RC_CFG_RX_PORT_ENA) : 0) |
                (psqs->port.fc_drop_mode ? PST_SQS(VTSS_F, qs, PORT_RAM_CFG_PORT_RC_CFG_PORT_FC_DROP_MODE) : 0) |
                (psqs->port.rc_mode ? PST_SQS(VTSS_F, qs, PORT_RAM_CFG_PORT_RC_CFG_PORT_RC_MODE) : 0) |
                (psqs->port.cnt_on_buf ? PST_SQS(VTSS_F, qs, PORT_RAM_CFG_PORT_RC_CFG_PORT_CNT_ON_BUF_LVL_ENA) : 0));
        
        WRX_SQS(qs, PORT_RAM_CFG_PORT_RC_HLWM_CFG, port_no,
                PST_SQS(VTSS_F, qs, PORT_RAM_CFG_PORT_RC_HLWM_CFG_PORT_RC_PROFILE_HWM(psqs->port.hwm)) |
                PST_SQS(VTSS_F, qs, PORT_RAM_CFG_PORT_RC_HLWM_CFG_PORT_RC_PROFILE_LWM(psqs->port.lwm)));
        
        WRX_SQS(qs, PORT_RAM_CFG_PORT_RC_ATOP_SWM_CFG, port_no,
                PST_SQS(VTSS_F, qs, PORT_RAM_CFG_PORT_RC_ATOP_SWM_CFG_PORT_RC_PROFILE_ATOP(psqs->port.atop)) |
                PST_SQS(VTSS_F, qs, PORT_RAM_CFG_PORT_RC_ATOP_SWM_CFG_PORT_RC_PROFILE_SWM(psqs->port.swm)));
        
        WRX_SQS(qs, PORT_RAM_CFG_PORT_RC_GWM_CFG, port_no,
                PST_SQS(VTSS_F, qs, PORT_RAM_CFG_PORT_RC_GWM_CFG_PORT_RC_PROFILE_GWM(psqs->port.gwm)));

        if (qs == 1) {
            JR_WRX(OQS, PORT_RAM_CFG_CM_PORT_WM, port_no, VTSS_F_OQS_PORT_RAM_CFG_CM_PORT_WM_CM_PORT_WM(psqs->port.cmwm));
        }
       
        /*  BUF port  Level  */
        WR_SQS(qs, RESOURCE_CTRL_CFG_BUF_RC_CFG,
               PST_SQS(VTSS_F, qs, RESOURCE_CTRL_CFG_BUF_RC_CFG_BUF_PORT_RC_PROFILE_ATOP(psqs->atop)) |
               (psqs->fc_drop_mode ? PST_SQS(VTSS_F, qs, RESOURCE_CTRL_CFG_BUF_RC_CFG_BUF_PORT_FC_DROP_MODE) : 0) |
               (psqs->rx_ena ? PST_SQS(VTSS_F, qs, RESOURCE_CTRL_CFG_BUF_RC_CFG_RX_BUF_ENA) : 0));
        
        WR_SQS(qs, RESOURCE_CTRL_CFG_BUF_PORT_RC_HLWM_CFG,
                PST_SQS(VTSS_F, qs, RESOURCE_CTRL_CFG_BUF_PORT_RC_HLWM_CFG_BUF_PORT_RC_PROFILE_HWM(psqs->hwm)) |
                PST_SQS(VTSS_F, qs, RESOURCE_CTRL_CFG_BUF_PORT_RC_HLWM_CFG_BUF_PORT_RC_PROFILE_LWM(psqs->lwm)));

        for (i = 0; i < 4; i++) {         
            WRX_SQS(qs, MTU_MTU_FRM_SIZE_CFG, i, psqs->pre_alloc_frm_size[i]);
        }
    }   
    return VTSS_RC_OK;     
}


#define IQS_SET_QU(q, _atop, _hwm, _lwm, _swm, fwd, cnt_port, cnt_prio, cnt_buf_port, rc, fc, ena, mtu) \
 {           \
  psqs->iqs.port.qu[q].atop  = _atop;  \
  psqs->iqs.port.qu[q].hwm  = _hwm;  \
  psqs->iqs.port.qu[q].lwm  = _lwm; \
  psqs->iqs.port.qu[q].swm  = _swm; \
  psqs->iqs.port.qu[q].fwd_press = fwd; \
  psqs->iqs.port.qu[q].cnt_on_port = cnt_port; \
  psqs->iqs.port.qu[q].cnt_on_prio = cnt_prio; \
  psqs->iqs.port.qu[q].cnt_on_buf_port = cnt_buf_port; \
  psqs->iqs.port.qu[q].cnt_on_buf_prio = 0; \
  psqs->iqs.port.qu[q].rc_mode = rc;  \
  psqs->iqs.port.qu[q].fc_drop_mode = fc; \
  psqs->iqs.port.qu[q].rx_ena = ena; \
  psqs->iqs.port.qu[q].mtu_pre_alloc = mtu; \
}

#define OQS_SET_QU(q,  _atop, _hwm, _lwm, _swm, fwd, cnt_port, cnt_prio, cnt_buf_port, rc, fc, ena, mtu) \
 {           \
  psqs->oqs.port.qu[q].atop  = _atop;      \
  psqs->oqs.port.qu[q].hwm  = _hwm;        \
  psqs->oqs.port.qu[q].lwm  = _lwm;        \
  psqs->oqs.port.qu[q].swm  = _swm;        \
  psqs->oqs.port.qu[q].fwd_press = fwd;    \
  psqs->oqs.port.qu[q].cnt_on_port = cnt_port; \
  psqs->oqs.port.qu[q].cnt_on_prio = cnt_prio; \
  psqs->oqs.port.qu[q].cnt_on_buf_port = cnt_buf_port; \
  psqs->oqs.port.qu[q].cnt_on_buf_prio = 0; \
  psqs->oqs.port.qu[q].rc_mode = rc;        \
  psqs->oqs.port.qu[q].fc_drop_mode = fc;   \
  psqs->oqs.port.qu[q].rx_ena = ena;        \
  psqs->oqs.port.qu[q].mtu_pre_alloc = mtu; \
}

#define OQS_SET_PORT(_atop, _hwm, _lwm, _swm, _cmwm, cnt_buf, ena, rc, fc, _gwm) \
 { \
     psqs->oqs.port.atop        = _atop;  \
     psqs->oqs.port.hwm         = _hwm;   \
     psqs->oqs.port.lwm         = _lwm;   \
     psqs->oqs.port.swm         = _swm;   \
     psqs->oqs.port.gwm         = _gwm;   \
     psqs->oqs.port.cmwm        = _cmwm;  \
     psqs->oqs.port.cnt_on_buf  = cnt_buf;\
     psqs->oqs.port.rx_ena      = ena;    \
     psqs->oqs.port.rc_mode     = rc;     \
     psqs->oqs.port.fc_drop_mode= fc;     \
 }

#define IQS_SET_PORT(_atop, _hwm, _lwm, _swm, cnt_buf, ena, rc, fc, _gwm)  \
 { \
     psqs->iqs.port.atop  = _atop;        \
     psqs->iqs.port.hwm   = _hwm;         \
     psqs->iqs.port.lwm   = _lwm;         \
     psqs->iqs.port.swm   = _swm;         \
     psqs->iqs.port.gwm   = _gwm;   \
     psqs->iqs.port.cnt_on_buf  = cnt_buf;\
     psqs->iqs.port.rx_ena      = ena;    \
     psqs->iqs.port.rc_mode     = rc;     \
     psqs->iqs.port.fc_drop_mode = fc;    \
 }

#define SQS_SET_BUF_WM(fc) \
 { \
     psqs->iqs.hwm  += psqs->iqs.port.hwm * 100 / (fc ? iqs_oversub_fact_fc : iqs_oversub_fact_drop)  - psqs->iqs.port.swm; \
     psqs->iqs.lwm  += psqs->iqs.port.lwm * 100 / (fc ? iqs_oversub_fact_fc : iqs_oversub_fact_drop) - psqs->iqs.port.swm; \
     psqs->iqs.atop += psqs->iqs.port.atop * 100 / (fc ? iqs_oversub_fact_fc : iqs_oversub_fact_drop) - psqs->iqs.port.swm; \
     psqs->oqs.atop -= psqs->iqs.port.atop * 100 / (fc ? iqs_oversub_fact_fc : iqs_oversub_fact_drop); \
     psqs->oqs.hwm  = psqs->oqs.atop - 50 - 10; \
     psqs->oqs.lwm  = psqs->oqs.hwm  - 13;      \
     psqs->iqs.rx_ena = 1;                      \
     psqs->oqs.rx_ena = 1;                      \
}

#define QS_QU(x) x
#define QS_ATOP(x) x
#define QS_HWM(x) x
#define QS_LWM(x) x
#define QS_SWM(x) x
#define QS_GWM(x) x
#define QS_FWDP(x) x
#define QS_CMWM(x) x
#define QS_CPORT(x) x
#define QS_CPRIO(x) x
#define QS_CBPORT(x) x
#define QS_ENA(x) x
#define QS_RC(x) x
#define QS_FC(x) (~x&1)
#define QS_MTU_ID(x) x

/* Calculate OQS WMs  */
static u32 jr_get_oqs_hwm(vtss_target_type_t target, BOOL port)
{
    u32 qu =   512000;
    u32 prt = 768000;

    if (port == 0) {
        return (qu/160);
    } else {        
        return (prt/160);
    }
}

/* Get target from state */
static vtss_target_type_t jr_target_get(vtss_state_t *vtss_state)
{
    vtss_target_type_t target = vtss_state->create.target;   

    /* Dual-device targets are treated like the corresponding single-device targets */
    if (target == VTSS_TARGET_E_STAX_III_24_DUAL)
        target = VTSS_TARGET_E_STAX_III_48;
    if (target == VTSS_TARGET_E_STAX_III_68_DUAL)
        target = VTSS_TARGET_E_STAX_III_68;

    return target;
}

/* 
   For VTSS_TARGET_E_STAX_III_68 and VTSS_TARGET_E_STAX_III_48 (Enterprise):
   - Inititilization (VD and CPU WM are configured)
   - Jumbo mode On/Off
   - Flowcontrol On/Off
   - Port speed: 1G / 10G
   - Port power down / up.   
   For  VTSS_TARGET_JAGUAR_1  VTSS_TARGET_LYNX_1 (Carrier Ethernet):
   - Inititilization (VD and CPU WM are configured)
   - No jumbo and no flowontrol is assumed.
 */

static vtss_rc jr_sqs_calc_wm(vtss_state_t *vtss_state,
                              u32 chipport, vtss_port_speed_t spd, BOOL jumbo, BOOL flowcontrol, BOOL power_down, BOOL init, sqs_t *psqs)
{
    u32 num_of_1G=0, num_of_10G=0, total_cells=0, oqs_ports=0, prios=0;
    BOOL one_in_drop_mode, vd_active;
    u32 max_frm_cells_normal = 10, max_frm_cells_jumbo  = 61; // Up to 9736 byte;
    u32 iqs_oversub_fact_drop, iqs_oversub_fact_fc, oversub_fact;
    u32 port, tmp, rsrv, q, p, i, oqs_total, mtu = 10;
    u32 hwm, lwm, swm, gwm, atop, cmwm, cnt_on_port, cnt_on_buf, rc;
    vtss_target_type_t target = jr_target_get(vtss_state);
    vtss_port_no_t port_no;

    if (vtss_state->init_conf.qs_conf.mode == VTSS_QS_MODE_DISABLED) {        
        if (target == VTSS_TARGET_JAGUAR_1 || target == VTSS_TARGET_LYNX_1) {
            if (!init) {
                if (flowcontrol || jumbo || power_down) { /* Not supported  */
                    return VTSS_RC_ERROR;   
                }
            }
        } else if (target == VTSS_TARGET_CE_MAX_12 || target == VTSS_TARGET_CE_MAX_24) {
            if (!init) {
                if (!flowcontrol || !jumbo) {  /* Not supported  */
                    return VTSS_RC_ERROR;   
                }
            }
        }
    }
    VTSS_D("Target:%x chipport:%d spd:%d jumbo:%d fc:%d power_down:%d, init:%d",target, chipport, spd, jumbo, flowcontrol, power_down, init);
    if (spd != VTSS_SPEED_10G && spd != VTSS_SPEED_12G ) {
        spd = VTSS_SPEED_1G;
    }

    switch (target) {
    case VTSS_TARGET_E_STAX_III_48:
    case VTSS_TARGET_LYNX_1:
    case VTSS_TARGET_E_STAX_III_68:
    case VTSS_TARGET_JAGUAR_1:
        total_cells = 26240; /* cell=160B */
        iqs_oversub_fact_drop = 125;
        iqs_oversub_fact_fc   = 100;
        break;
    case VTSS_TARGET_CE_MAX_12:        
    case VTSS_TARGET_CE_MAX_24:
        iqs_oversub_fact_drop = 125;
        iqs_oversub_fact_fc   = 100;
        if ((vtss_jr1_ce_max_hm(vtss_state) == 0) || (vtss_jr1_ce_max_hm(vtss_state) == 2)) {
            oqs_ports = 56;
            prios = 8;
        } else {
            oqs_ports = 80;
            prios = 4;
        }
        break;
    default:
        VTSS_E("Target not supported");
        return VTSS_RC_ERROR;
    }
    
    for (p = VTSS_PORT_NO_START; p < vtss_state->port_count; p++) {
        if (!VTSS_PORT_CHIP_SELECTED(p)) {
            continue;
        }
        if (VTSS_PORT_IS_10G(VTSS_CHIP_PORT(p))) {
            num_of_10G++;
        } else {
            num_of_1G++;
        }
    }

    if (vtss_state->chip_count == 2) {
        /* Include internal ports */
        num_of_10G += 2;
    }
    vd_active = 0; /* Virtual device currently not used */
    
    if (init) {
        /* ======================== */
        /* ======== Init ========== */
        /* ======================== */
        // Initilize. Set up all the stuff which will not change based on jumbo/flowcontrol/power.             
        if (chipport == 0) {
            for (i = 0; i < 4; i++) {
                psqs->iqs.pre_alloc_frm_size[i] = (i == 1) ? max_frm_cells_jumbo : max_frm_cells_normal; 
                psqs->oqs.pre_alloc_frm_size[i] = (i == 1) ? max_frm_cells_jumbo : max_frm_cells_normal; 
            }
            if (target == VTSS_TARGET_CE_MAX_24 || target == VTSS_TARGET_CE_MAX_12) {
                /* Supporting CE-MAX WM for Jumbo / flowcontrol mode */
                psqs->iqs.rx_ena = 1;                                 
                psqs->oqs.rx_ena = 1;                                 
                psqs->iqs.atop = 7140; /* 8922 cells @ 1.25 oversubsrciption */
                psqs->iqs.hwm  = 0;  
                psqs->iqs.lwm  = 0;
                psqs->oqs.atop = 12080;
                psqs->oqs.hwm  = 11969;
                psqs->oqs.lwm  = 11956;
            } else {
                psqs->iqs.atop = 0;
                psqs->iqs.hwm  = 0;
                psqs->iqs.lwm  = 0;
                psqs->iqs.fc_drop_mode  = 0;                    
                psqs->oqs.fc_drop_mode  = 0;
                psqs->oqs.atop = total_cells;
                psqs->oqs.hwm  = psqs->oqs.atop - 50 - 10; 
                psqs->oqs.lwm  = psqs->oqs.hwm  - 13;
            }
        } 
        psqs->iqs.port.rx_ena = 0;

        /* === Virtual device = 32 ====== */        
        if (chipport == 32 && vd_active) {
            for (q = 0; q < 8; q++) {
                IQS_SET_QU(QS_QU(q), QS_ATOP(100),  QS_HWM(32), QS_LWM(27), QS_SWM(0), QS_FWDP(100), 
                           QS_CPORT(1), QS_CPRIO(0), QS_CBPORT(0), QS_RC(0), QS_FC(1), QS_ENA(1), QS_MTU_ID(0));
            }
            IQS_SET_PORT(QS_ATOP(100), QS_HWM(32), QS_LWM(31), QS_SWM(10), QS_CBPORT(1), QS_ENA(1), QS_RC(1), QS_FC(1), QS_GWM(0));  

            tmp = psqs->iqs.port.hwm * 100 / iqs_oversub_fact_drop; 
            psqs->iqs.hwm  += tmp - psqs->iqs.port.swm;
            tmp = psqs->iqs.port.lwm * 100 / iqs_oversub_fact_drop; 
            psqs->iqs.lwm  += tmp - psqs->iqs.port.swm;
            tmp = psqs->iqs.port.atop * 100 / iqs_oversub_fact_drop;
            psqs->iqs.atop += tmp - psqs->iqs.port.swm;
             
            psqs->oqs.atop -= tmp;
            psqs->oqs.hwm  = psqs->oqs.atop - 50 - 10; 
            psqs->oqs.lwm  = psqs->oqs.hwm  - 13;                        
        }
        
        /* === VD OQS === */
        if (chipport == 80 && vd_active) {
            for (q = 0; q < 8; q++) {      
                OQS_SET_QU(QS_QU(q), QS_ATOP(460), QS_HWM(400), QS_LWM(387), QS_SWM(15), QS_FWDP(0), 
                           QS_CPORT(1), QS_CPRIO(0), QS_CBPORT(1), QS_RC(1), QS_FC(1), QS_ENA(1), QS_MTU_ID(0));
                psqs->oqs.atop -= psqs->oqs.port.qu[q].swm;
            }
            
            OQS_SET_PORT(QS_ATOP(860), QS_HWM(800), QS_LWM(787), QS_SWM(0), QS_CMWM(999), QS_CBPORT(0), QS_ENA(1), QS_RC(0), QS_FC(1), QS_GWM(0));  
            psqs->oqs.hwm  = psqs->oqs.atop - 50 - 10; 
            psqs->oqs.lwm  = psqs->oqs.hwm  - 13;      
        }
        /* === IQS CPU ports 33-34 ====== */            
        if (chipport >= 33 &&  chipport < 35) {
            for (q = 0; q < 8; q++) {       
                IQS_SET_QU(QS_QU(q), QS_ATOP(80),  QS_HWM(18), QS_LWM(15), QS_SWM(0), QS_FWDP(80), 
                           QS_CPORT(1), QS_CPRIO(0), QS_CBPORT(0), QS_RC(0), QS_FC(1), QS_ENA(1), QS_MTU_ID(0));
            }
            IQS_SET_PORT(QS_ATOP(80), QS_HWM(18), QS_LWM(15), QS_SWM(10), QS_CBPORT(1), QS_ENA(1), QS_RC(1), QS_FC(1), QS_GWM(0));  

            tmp = psqs->iqs.port.hwm * 100 / iqs_oversub_fact_drop; 
            psqs->iqs.hwm  += tmp - psqs->iqs.port.swm;
            tmp = psqs->iqs.port.lwm * 100 / iqs_oversub_fact_drop;
            psqs->iqs.lwm  += tmp - psqs->iqs.port.swm;
            tmp = psqs->iqs.port.atop * 100 / iqs_oversub_fact_drop; 
            psqs->iqs.atop += tmp - psqs->iqs.port.swm;
            psqs->oqs.atop -= tmp;
            psqs->oqs.hwm  = psqs->oqs.atop - 50 - 10; 
            psqs->oqs.lwm  = psqs->oqs.hwm  - 13;
        }

        /* === OQS HOST ports 32-79 ====== */            
        if (target == VTSS_TARGET_CE_MAX_12 || target == VTSS_TARGET_CE_MAX_24) {
            if (chipport > 31 && chipport < oqs_ports) { 
                rsrv = 0;
                for (q = 0; q < prios; q++) {                    
                    if (q < (prios - 2)) {
                        /* Service qu WM */
                        OQS_SET_QU(QS_QU(q), QS_ATOP(131),  QS_HWM(20), QS_LWM(20), QS_SWM(20), QS_FWDP(15), 
                                   QS_CPORT(1), QS_CPRIO(0), QS_CBPORT(1), QS_RC(1), QS_FC(1), QS_ENA(1), QS_MTU_ID(0));
                    } else {
                        /* Control qu WM */
                        OQS_SET_QU(QS_QU(q), QS_ATOP(131),  QS_HWM(20), QS_LWM(20), QS_SWM(20), QS_FWDP(15), 
                                   QS_CPORT(0), QS_CPRIO(0), QS_CBPORT(0), QS_RC(1), QS_FC(1), QS_ENA(1), QS_MTU_ID(0));
                    }
                    rsrv += 20;
                }
                OQS_SET_PORT(QS_ATOP(191), QS_HWM(80), QS_LWM(80), QS_SWM(0), QS_CMWM(75), QS_CBPORT(0), QS_ENA(1), QS_RC(0), QS_FC(1), QS_GWM(200));  
                psqs->oqs.atop -= rsrv;
                psqs->oqs.hwm  -= rsrv;
                psqs->oqs.lwm  -= rsrv;
            }            
        }

        /* === OQS CPU ports 81-88 ====== */            
        if (chipport >= 81 &&  chipport < 89) {
            atop = 660;
            hwm  = 600;
            lwm  = 587;

            OQS_SET_QU(QS_QU(0), QS_ATOP(atop),  QS_HWM(hwm), QS_LWM(lwm), QS_SWM(15), QS_FWDP(0), 
                       QS_CPORT(1), QS_CPRIO(0), QS_CBPORT(1), QS_RC(1), QS_FC(1), QS_ENA(1), QS_MTU_ID(0));
            psqs->oqs.atop -= psqs->oqs.port.qu[0].swm;
            OQS_SET_PORT(QS_ATOP(atop), QS_HWM(hwm), QS_LWM(lwm), QS_SWM(0), QS_CMWM(999), QS_CBPORT(0), QS_ENA(1), QS_RC(0), QS_FC(1), QS_GWM(0));  
        }        

        if (chipport == 88) { 
            psqs->oqs.hwm  = psqs->oqs.atop - 50 - 10;
            psqs->oqs.lwm  = psqs->oqs.hwm  - 13;
        }

    } else {
        /* ====================== */
        /* =====  Port down ===== */
        /* ====================== */
        /* Power down a port. It's space goes to the shared pool */
        if (power_down) {
            psqs->iqs.port.rx_ena = 0;       
            oversub_fact = (psqs->iqs.port.fc_drop_mode == 0) ? iqs_oversub_fact_fc : iqs_oversub_fact_drop;             
            psqs->iqs.hwm  -= psqs->iqs.port.hwm * 100 / oversub_fact - psqs->iqs.port.swm; 
            psqs->iqs.lwm  -= psqs->iqs.port.lwm * 100 / oversub_fact  - psqs->iqs.port.swm;
            psqs->iqs.atop -= psqs->iqs.port.atop * 100 / oversub_fact - psqs->iqs.port.swm;
            psqs->oqs.atop += psqs->iqs.port.atop * 100 / oversub_fact;

            // Check if a port is left that runs drop mode. If not, switch buffer to FC mode
            if (psqs->iqs.port.fc_drop_mode == 1) {
                one_in_drop_mode = 0;
                for (port = 0; port < vtss_state->port_count; port++) {                    
                    if ((vtss_state->port.conf[port].power_down == 0) && vtss_state->port.conf[port].flow_control.obey == 0) {
                        one_in_drop_mode = 1;
                        break;
                    }
                }
                if (!one_in_drop_mode) {
                    psqs->iqs.fc_drop_mode = 0;
                }                
            }
            psqs->oqs.hwm  = psqs->oqs.atop - 50 - 10; 
            psqs->oqs.lwm  = psqs->oqs.hwm  - 13;
            for (q = 0; q < 8; q++) {
                psqs->oqs.atop += psqs->oqs.port.qu[q].swm;
            }
            psqs->oqs.hwm  = psqs->oqs.atop - 50 - 10; 
            psqs->oqs.lwm  = psqs->oqs.hwm  - 13;

        } else if ((jumbo) && (flowcontrol)) {
            /* ============================ */
            /* === JUMBO and FLOWONTROL === */
            /* ============================ */           
            /* ============= IQS ============== */     
            
            if (target == VTSS_TARGET_CE_MAX_24 || target == VTSS_TARGET_CE_MAX_12) {
                rsrv = 0;
                for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++)
                    if (vtss_state->port.map[port_no].chip_port == chipport)
                        break;

                if (vtss_jr1_cemax_port_is_host(vtss_state, port_no)) {  /* Host Interface port IQS/OQS */
                    /* IQS */
                    IQS_SET_PORT(QS_ATOP(417), QS_HWM(200), QS_LWM(180), QS_SWM(0), QS_CBPORT(1), QS_ENA(1), QS_RC(0), QS_FC(1), QS_GWM(430));  
                    for (q = 0; q < prios; q++) {
                        IQS_SET_QU(QS_QU(q), QS_ATOP(160),  QS_HWM(150), QS_LWM(150), QS_SWM(0), QS_FWDP(2), 
                                   QS_CPORT(1), QS_CPRIO(0), QS_CBPORT(0), QS_RC(0), QS_FC(1), QS_ENA(1), QS_MTU_ID(0));
                    }
                    hwm = 160; /* 8x10 cells */
                    lwm = 160; /* 8x10 cells */
                    /* No OQS config needed for HI */
                } else if (spd == VTSS_SPEED_1G)  { /* 1G Line port IQS/OQS */
                    hwm = 160; 
                    lwm = 140; 
                    IQS_SET_PORT(QS_ATOP(307), QS_HWM(hwm), QS_LWM(lwm), QS_SWM(0), QS_CBPORT(1), QS_ENA(1), QS_RC(0), QS_FC(1), QS_GWM(320));  
                    OQS_SET_PORT(QS_ATOP(421), QS_HWM(310), QS_LWM(297), QS_SWM(0), QS_CMWM(180), QS_CBPORT(0), QS_ENA(1), QS_RC(0), QS_FC(1), QS_GWM(430));  
                    for (q = 0; q < prios; q++) {
                        if (q < (prios-2)) {         /* IQS Service qu WM for 1G */                    
                            IQS_SET_QU(QS_QU(q), QS_ATOP(307),  QS_HWM(160), QS_LWM(160), QS_SWM(0), QS_FWDP(0), 
                                       QS_CPORT(1), QS_CPRIO(0), QS_CBPORT(0), QS_RC(0), QS_FC(1), QS_ENA(1), QS_MTU_ID(0));

                                                     /* OQS Service qu WM for 1G */ 
                            OQS_SET_QU(QS_QU(q), QS_ATOP(211),  QS_HWM(100), QS_LWM(87), QS_SWM(15), QS_FWDP(20), 
                                       QS_CPORT(1), QS_CPRIO(0), QS_CBPORT(1), QS_RC(1), QS_FC(1), QS_ENA(1), QS_MTU_ID(0));
                            rsrv += 15;
                        } else {                     /* IQS Control qu WM for 1G */ 
                            IQS_SET_QU(QS_QU(q), QS_ATOP(130),  QS_HWM(65), QS_LWM(65), QS_SWM(0), QS_FWDP(0), 
                                       QS_CPORT(0), QS_CPRIO(0), QS_CBPORT(0), QS_RC(0), QS_FC(1), QS_ENA(1), QS_MTU_ID(0));
                                                     /* OQS Control qu WM for 1G */ 
                            OQS_SET_QU(QS_QU(q), QS_ATOP(131),  QS_HWM(20), QS_LWM(7), QS_SWM(20), QS_FWDP(20), 
                                       QS_CPORT(0), QS_CPRIO(0), QS_CBPORT(0), QS_RC(1), QS_FC(1), QS_ENA(1), QS_MTU_ID(0));
                            rsrv += 20;
                        }
                    }
                } else {     
                    /* 10G Line port IQS/OQS */
                    hwm = 480; 
                    lwm = 440; 
                    IQS_SET_PORT(QS_ATOP(697), QS_HWM(hwm), QS_LWM(lwm), QS_SWM(0), QS_CBPORT(1), QS_ENA(1), QS_RC(0), QS_FC(1), QS_GWM(720));  
                    OQS_SET_PORT(QS_ATOP(531), QS_HWM(420), QS_LWM(407), QS_SWM(0), QS_CMWM(290), QS_CBPORT(0), QS_ENA(1), QS_RC(0), QS_FC(1), QS_GWM(540));  
                    for (q = 0; q < prios; q++) {
                        if (q < (prios-2)) {         /* Service qu WM */                    
                            IQS_SET_QU(QS_QU(q), QS_ATOP(697),  QS_HWM(480), QS_LWM(480), QS_SWM(0), QS_FWDP(0), 
                                       QS_CPORT(1), QS_CPRIO(0), QS_CBPORT(0), QS_RC(0), QS_FC(1), QS_ENA(1), QS_MTU_ID(0));

                            OQS_SET_QU(QS_QU(q), QS_ATOP(261),  QS_HWM(150), QS_LWM(137), QS_SWM(30), QS_FWDP(70), 
                                       QS_CPORT(1), QS_CPRIO(0), QS_CBPORT(1), QS_RC(0), QS_FC(1), QS_ENA(1), QS_MTU_ID(0));

                            rsrv += 30;
                        } else {                     /* Control qu WM */
                            IQS_SET_QU(QS_QU(q), QS_ATOP(195),  QS_HWM(130), QS_LWM(130), QS_SWM(0), QS_FWDP(0), 
                                       QS_CPORT(0), QS_CPRIO(0), QS_CBPORT(0), QS_RC(0), QS_FC(1), QS_ENA(1), QS_MTU_ID(0));

                            OQS_SET_QU(QS_QU(q), QS_ATOP(151),  QS_HWM(40), QS_LWM(27), QS_SWM(40), QS_FWDP(70), 
                                       QS_CPORT(0), QS_CPRIO(0), QS_CBPORT(0), QS_RC(0), QS_FC(1), QS_ENA(1), QS_MTU_ID(0));
                            rsrv += 40;
                        }
                    }
                } 
                psqs->iqs.hwm  += (u32)(hwm - (hwm/5)); /* To 80% - no floats */
                psqs->iqs.lwm  += (u32)(lwm - (lwm/5)); /* To 80% - no floats */

                psqs->oqs.atop -= rsrv; 
                psqs->oqs.hwm  -= rsrv; 
                psqs->oqs.lwm  -= rsrv; 
                return VTSS_RC_OK;    /* Nothing more todo for CE-MAX */
                
            } else {
                if (spd == VTSS_SPEED_1G) {
                    atop = 449;
                    hwm  = 305;
                    lwm  = 183;
                    swm  = 122;
                } else {
                    atop = 854;
                    hwm  = 488;
                    lwm  = 305;
                    swm  = 183;
                }
                
                IQS_SET_PORT(QS_ATOP(atop), QS_HWM(hwm), QS_LWM(lwm), QS_SWM(swm), QS_CBPORT(1), QS_ENA(1), QS_RC(1), QS_FC(1), QS_GWM(0));  
                for (q = 0; q < 8; q++) {
                    IQS_SET_QU(QS_QU(q), QS_ATOP(atop),  QS_HWM(hwm), QS_LWM(lwm), QS_SWM(0), QS_FWDP(atop), 
                               QS_CPORT(1), QS_CPRIO(0), QS_CBPORT(0), QS_RC(0), QS_FC(1), QS_ENA(1), QS_MTU_ID(1));
                }
                SQS_SET_BUF_WM(flowcontrol);
            }
            
            /* ============= OQS ============== */     
            for (q = 0; q < 8; q++) {
                if (spd == VTSS_SPEED_1G) {
                    OQS_SET_QU(QS_QU(q), QS_ATOP(416),  QS_HWM(305), QS_LWM(292), QS_SWM(15), QS_FWDP(0), 
                               QS_CPORT(1), QS_CPRIO(0), QS_CBPORT(1), QS_RC(1), QS_FC(1), QS_ENA(1), QS_MTU_ID(1));
                } else {
                    OQS_SET_QU(QS_QU(q), QS_ATOP(904),  QS_HWM(793), QS_LWM(780), QS_SWM(30), QS_FWDP(0), 
                               QS_CPORT(1), QS_CPRIO(0), QS_CBPORT(1), QS_RC(1), QS_FC(1), QS_ENA(1), QS_MTU_ID(1));
                }

                psqs->oqs.atop -= psqs->oqs.port.qu[q].swm;
            }
            
            if (spd == VTSS_SPEED_1G) {
                OQS_SET_PORT(QS_ATOP(599), QS_HWM(488), QS_LWM(474), QS_SWM(0), QS_CMWM(100), QS_CBPORT(0), QS_ENA(1), QS_RC(0), QS_FC(1), QS_GWM(0));  
            } else {
                OQS_SET_PORT(QS_ATOP(1087), QS_HWM(976), QS_LWM(963), QS_SWM(0), QS_CMWM(250), QS_CBPORT(0), QS_ENA(1), QS_RC(0), QS_FC(1), QS_GWM(0));  
            }
            
            psqs->oqs.hwm  = psqs->oqs.atop - 50 - 10; 
            psqs->oqs.lwm  = psqs->oqs.hwm  - 13;
        } else if ((!jumbo) && (flowcontrol)) {
            /* ================================ */
            /* === NOT JUMBO and FLOWONTROL === */
            /* ================================ */           
                
            /* ============= IQS ============== */           
            if (spd == VTSS_SPEED_1G) {
                atop = 125;
                hwm  = 80;
                lwm  = 50;
                swm  = 20;
            } else {
                atop = 275;
                hwm  = 160;
                lwm  = 130;
                swm  = 30;
            }
            IQS_SET_PORT(QS_ATOP(atop), QS_HWM(hwm), QS_LWM(lwm), QS_SWM(swm), QS_CBPORT(1), QS_ENA(1), QS_RC(1), QS_FC(1), QS_GWM(0));  
            for (q = 0; q < 8; q++) {
                IQS_SET_QU(QS_QU(q), QS_ATOP(atop),  QS_HWM(hwm), QS_LWM(lwm), QS_SWM(0), QS_FWDP(atop), 
                           QS_CPORT(1), QS_CPRIO(0), QS_CBPORT(0), QS_RC(0), QS_FC(1), QS_ENA(1), QS_MTU_ID(0));
            }

            SQS_SET_BUF_WM(flowcontrol);

            /* ============= OQS ============== */           
            for (q = 0; q < 8; q++) {
                if (spd == VTSS_SPEED_1G) {
                    OQS_SET_QU(QS_QU(q), QS_ATOP(660),  QS_HWM(600), QS_LWM(587), QS_SWM(15), QS_FWDP(0), 
                               QS_CPORT(1), QS_CPRIO(0), QS_CBPORT(1), QS_RC(1), QS_FC(1), QS_ENA(1), QS_MTU_ID(0));
                } else {
                    OQS_SET_QU(QS_QU(q), QS_ATOP(1260),  QS_HWM(1200), QS_LWM(1187), QS_SWM(30), QS_FWDP(0), 
                               QS_CPORT(1), QS_CPRIO(0), QS_CBPORT(1), QS_RC(1), QS_FC(1), QS_ENA(1), QS_MTU_ID(0));
                }
                
                psqs->oqs.atop -= psqs->oqs.port.qu[q].swm;
            }
            if (spd == VTSS_SPEED_1G) {
                OQS_SET_PORT(QS_ATOP(860), QS_HWM(800), QS_LWM(787), QS_SWM(0), QS_CMWM(300), QS_CBPORT(0), QS_ENA(1), QS_RC(0), QS_FC(1), QS_GWM(0));  
            } else {
                OQS_SET_PORT(QS_ATOP(1660), QS_HWM(1600), QS_LWM(1587), QS_SWM(0), QS_CMWM(600), QS_CBPORT(0), QS_ENA(1), QS_RC(0), QS_FC(1), QS_GWM(0));  
            }
            
            psqs->oqs.hwm  = psqs->oqs.atop - 50 - 10; 
            psqs->oqs.lwm  = psqs->oqs.hwm  - 13;

        } else if ((jumbo) && (!flowcontrol)) {
            /* ================================ */
            /* === JUMBO and NOT FLOWONTROL === */
            /* ================================ */

            /* ============= IQS ============== */           
            if (spd == VTSS_SPEED_1G) {
                IQS_SET_PORT(QS_ATOP(488), QS_HWM(427), QS_LWM(427), QS_SWM(122), QS_CBPORT(1), QS_ENA(1), QS_RC(1), QS_FC(0), QS_GWM(0));  
            } else {
                IQS_SET_PORT(QS_ATOP(793), QS_HWM(732), QS_LWM(732), QS_SWM(244), QS_CBPORT(1), QS_ENA(1), QS_RC(1), QS_FC(0), QS_GWM(0));  
            }
            

            for (q = 0; q < 8; q++) {
                if (spd == VTSS_SPEED_1G) {
                    IQS_SET_QU(QS_QU(q), QS_ATOP(305),  QS_HWM(244), QS_LWM(244), QS_SWM(0), QS_FWDP(40), 
                               QS_CPORT(1), QS_CPRIO(0), QS_CBPORT(0), QS_RC(0), QS_FC(0), QS_ENA(1), QS_MTU_ID(1));
                } else {
                    IQS_SET_QU(QS_QU(q), QS_ATOP(610),  QS_HWM(549), QS_LWM(549), QS_SWM(0), QS_FWDP(70),
                               QS_CPORT(1), QS_CPRIO(0), QS_CBPORT(0), QS_RC(0), QS_FC(0), QS_ENA(1), QS_MTU_ID(1));
                    
                }
            }                                 

       
            SQS_SET_BUF_WM(flowcontrol);

            psqs->iqs.fc_drop_mode = 1; /* dropmode */           
            // As soon as one port runs drop mode, set IQS buffer to drop mode
            /* ============= OQS ============== */           
            for (q = 0; q < 8; q++) {
                /* Same OQS WM for all speeds */
                hwm = jr_get_oqs_hwm(target, 0);
                OQS_SET_QU(QS_QU(q), QS_ATOP((hwm+111)),  QS_HWM(hwm), QS_LWM((hwm-13)), QS_SWM(15), QS_FWDP(0), 
                           QS_CPORT(1), QS_CPRIO(0), QS_CBPORT(1), QS_RC(1), QS_FC(1), QS_ENA(1), QS_MTU_ID(1));
                psqs->oqs.atop -= psqs->oqs.port.qu[q].swm;
            }
            
            /* Same OQS WM for all speeds */
            hwm = jr_get_oqs_hwm(target, 1);
            OQS_SET_PORT(QS_ATOP((hwm+111)), QS_HWM(hwm), QS_LWM((hwm-13)), QS_SWM(0), QS_CMWM((hwm/2)), QS_CBPORT(0), QS_ENA(1), QS_RC(0), QS_FC(1), QS_GWM(0));  
            psqs->oqs.hwm  = psqs->oqs.atop - 50 - 10; 
            psqs->oqs.lwm  = psqs->oqs.hwm  - 13;

        } else  {
            /* ==================================== */
            /* === NOT JUMBO and NOT FLOWONTROL === */
            /* ==================================== */

            /* ============= IQS ============== */           
            if (target == VTSS_TARGET_JAGUAR_1 || target == VTSS_TARGET_LYNX_1) {
                hwm = (spd == VTSS_SPEED_1G) ? 30 : 40;
                for (q = 0; q < 8; q++) { 
                    /* Queues 0-5 (Service traffic) Queues 6-7 (Control traffic)*/
                    IQS_SET_QU(QS_QU(q), QS_ATOP((hwm+mtu)),  QS_HWM(hwm), QS_LWM(hwm), QS_SWM(0), QS_FWDP(2), 
                               QS_CPORT((q < 6 ? 1 : 0)), QS_CPRIO(0), QS_CBPORT(0), QS_RC(0), QS_FC(1), QS_ENA(1), QS_MTU_ID(0));
                }
                
                hwm = (spd == VTSS_SPEED_1G) ? 180 : 240;
                gwm = hwm + mtu;
                atop = gwm + mtu;
                /* ============= IQS PORT ============== */           
                IQS_SET_PORT(QS_ATOP(atop), QS_HWM(hwm), QS_LWM(hwm), QS_SWM(0), QS_CBPORT(1), QS_ENA(1), QS_RC(0), QS_FC(1), QS_GWM(gwm));  

                /* ============= IQS BUF ============== */           
                psqs->iqs.rx_ena = 1;            
                psqs->iqs.fc_drop_mode = 0;            
                psqs->iqs.hwm = num_of_1G * 180 + num_of_10G * 240;
                psqs->iqs.lwm = psqs->iqs.hwm;
                psqs->iqs.atop = (num_of_1G * (180 + 2*mtu)) + (num_of_10G * (240 + 2*mtu));
        
                /* === OQS === */
                oqs_total = total_cells - psqs->iqs.atop - (num_of_1G*2*30 + num_of_10G*2*40);                     
                for (q = 0; q < 8; q++) { 
                    if (q < 6) { /* Queues 0-5 (Service traffic) */
                        rc = 1;     
                        atop = (spd == VTSS_SPEED_1G) ? 290 : 390;
                        hwm = (spd == VTSS_SPEED_1G) ? 230 : 330;
                        lwm = hwm - 13;
                        swm = (spd == VTSS_SPEED_1G) ? 15 : 30;
                        cnt_on_port = 1;           
                        cnt_on_buf = 1; 
                    } else {
                        rc = 0;     
                        atop = (spd == VTSS_SPEED_1G) ? 180 : 220;
                        hwm = (spd == VTSS_SPEED_1G) ? 12*mtu : 16*mtu;
                        lwm = hwm - 13;
                        swm = hwm;
                        cnt_on_port = 0;   
                        cnt_on_buf = 0;
                    }
                        
                    OQS_SET_QU(QS_QU(q), QS_ATOP(atop),  QS_HWM(hwm), QS_LWM(lwm), QS_SWM(swm), QS_FWDP(0), 
                               QS_CPORT(cnt_on_port), QS_CPRIO(0), QS_CBPORT(cnt_on_buf), QS_RC(rc), QS_FC(0), QS_ENA(1), QS_MTU_ID(0));
                }

                /* Buf Port level - OQS */
                /* ==================== */
                psqs->oqs.rx_ena = 1;            
                psqs->oqs.fc_drop_mode = 1; /* Flowcontrol to IQS */
                psqs->oqs.atop = oqs_total - num_of_1G*6*15 - num_of_1G*2*120 -  num_of_10G*6*30 - num_of_10G*2*160;
                psqs->oqs.hwm = psqs->oqs.atop - 50 - mtu;
                psqs->oqs.lwm = psqs->oqs.hwm - 13;
                
                /* Port level - OQS */
                /* ================ */
                hwm = (spd == VTSS_SPEED_1G) ? 600 : 800;
                atop = hwm + 50 + mtu;
                lwm = hwm - 13;
                gwm = (spd == VTSS_SPEED_1G) ? 620 : 820;
                cmwm = (spd == VTSS_SPEED_1G) ? 200 : 270;
                OQS_SET_PORT(QS_ATOP(atop), QS_HWM(hwm), QS_LWM(lwm), QS_SWM(0), QS_CMWM(cmwm), QS_CBPORT(0), QS_ENA(1), QS_RC(0), QS_FC(0), QS_GWM(gwm));  
            } else {
                /* Enterprize */
                if (spd == VTSS_SPEED_1G) {
                    IQS_SET_PORT(QS_ATOP(250), QS_HWM(240), QS_LWM(240), QS_SWM(20), QS_CBPORT(1), QS_ENA(1), QS_RC(1), QS_FC(0), QS_GWM(0));  
                } else {
                    IQS_SET_PORT(QS_ATOP(490), QS_HWM(480), QS_LWM(480), QS_SWM(40), QS_CBPORT(1), QS_ENA(1), QS_RC(1), QS_FC(0), QS_GWM(0));         
                }
                for (q = 0; q < 8; q++) {
                    if (spd == VTSS_SPEED_1G) {
                        IQS_SET_QU(QS_QU(q), QS_ATOP(60),  QS_HWM(50), QS_LWM(50), QS_SWM(0), QS_FWDP(30), 
                                   QS_CPORT(1), QS_CPRIO(0), QS_CBPORT(0), QS_RC(0), QS_FC(0), QS_ENA(1), QS_MTU_ID(0));
                    } else {
                        IQS_SET_QU(QS_QU(q), QS_ATOP(110),  QS_HWM(100), QS_LWM(100), QS_SWM(0), QS_FWDP(60), 
                                   QS_CPORT(1), QS_CPRIO(0), QS_CBPORT(0), QS_RC(0), QS_FC(0), QS_ENA(1), QS_MTU_ID(0));
                    }
                }
                SQS_SET_BUF_WM(flowcontrol); 
                psqs->iqs.fc_drop_mode = 1;
                // As soon as one port runs drop mode, set IQS buffer to drop mode
                /* ============= OQS ============== */           
                for (q = 0; q < 8; q++) {
                    /* Same OQS WM for all speeds */
                    hwm = jr_get_oqs_hwm(target, 0);
                    OQS_SET_QU(QS_QU(q), QS_ATOP((hwm+111)),  QS_HWM(hwm), QS_LWM((hwm-13)), QS_SWM(15), QS_FWDP(0), 
                               QS_CPORT(1), QS_CPRIO(0), QS_CBPORT(1), QS_RC(1), QS_FC(1), QS_ENA(1), QS_MTU_ID(1));
                    psqs->oqs.atop -= psqs->oqs.port.qu[q].swm;                    
                }
                /* Same OQS WM for all speeds */
                hwm = jr_get_oqs_hwm(target, 1);
                OQS_SET_PORT(QS_ATOP((hwm+111)), QS_HWM(hwm), QS_LWM((hwm-13)), QS_SWM(0), QS_CMWM((hwm/2)), QS_CBPORT(0), QS_ENA(1), QS_RC(0), QS_FC(1), QS_GWM(0));  

                psqs->oqs.hwm  = psqs->oqs.atop - 50 - 10; 
                psqs->oqs.lwm  = psqs->oqs.hwm  - 13;        
            } // VTSS_TARGET_E_STAX_III_48 ||  VTSS_TARGET_E_STAX_III_68
        }
    }
    return VTSS_RC_OK;    
}


/* Serdes6G: Read data from MCB slave back to MCB master (CSR target) */
static vtss_rc jr_sd6g_read(vtss_state_t *vtss_state, u32 addr) 
{
    /* Read back data from MCB slave to MCB master (CSR target) */
    JR_WR(MACRO_CTRL, MCB_SERDES6G_CFG_MCB_SERDES6G_ADDR_CFG,
           VTSS_F_MACRO_CTRL_MCB_SERDES6G_CFG_MCB_SERDES6G_ADDR_CFG_SERDES6G_ADDR(addr) | 
           VTSS_F_MACRO_CTRL_MCB_SERDES6G_CFG_MCB_SERDES6G_ADDR_CFG_SERDES6G_RD_ONE_SHOT);
       
    /* Wait until read operation is completed  */
    JR_POLL_BIT(MACRO_CTRL, MCB_SERDES6G_CFG_MCB_SERDES6G_ADDR_CFG, SERDES6G_RD_ONE_SHOT);

    return VTSS_RC_OK;    
}


/* Serdes1G: Read data from  MCB slave back to MCB master (CSR target) */
static vtss_rc jr_sd1g_read(vtss_state_t *vtss_state, u32 addr) 
{
    /* Read back data from MCB slave to MCB master (CSR target) */
    JR_WR(MACRO_CTRL, MCB_SERDES1G_CFG_MCB_SERDES1G_ADDR_CFG,
           VTSS_F_MACRO_CTRL_MCB_SERDES1G_CFG_MCB_SERDES1G_ADDR_CFG_SERDES1G_ADDR(addr) | 
           VTSS_F_MACRO_CTRL_MCB_SERDES1G_CFG_MCB_SERDES1G_ADDR_CFG_SERDES1G_RD_ONE_SHOT);
       
    /* Wait until read operation is completed  */
    JR_POLL_BIT(MACRO_CTRL, MCB_SERDES1G_CFG_MCB_SERDES1G_ADDR_CFG, SERDES1G_RD_ONE_SHOT);

    return VTSS_RC_OK;
}


/* Serdes6G: Transfers data from MCB master (CSR target) to MCB slave */
static vtss_rc jr_sd6g_write(vtss_state_t *vtss_state, u32 addr, u32 nsec) 
{
    JR_WR(MACRO_CTRL, MCB_SERDES6G_CFG_MCB_SERDES6G_ADDR_CFG,
          VTSS_F_MACRO_CTRL_MCB_SERDES6G_CFG_MCB_SERDES6G_ADDR_CFG_SERDES6G_ADDR(addr) |
          VTSS_F_MACRO_CTRL_MCB_SERDES6G_CFG_MCB_SERDES6G_ADDR_CFG_SERDES6G_WR_ONE_SHOT);

    /* Wait until write operation is completed  */
    JR_POLL_BIT(MACRO_CTRL, MCB_SERDES6G_CFG_MCB_SERDES6G_ADDR_CFG, SERDES6G_WR_ONE_SHOT);
    
    if (nsec)
        VTSS_NSLEEP(nsec);

    return VTSS_RC_OK;
}

/* Serdes1G: Transfers data from MCB master (CSR target) to MCB slave */
static vtss_rc jr_sd1g_write(vtss_state_t *vtss_state, u32 addr, u32 nsec) 
{
    JR_WR(MACRO_CTRL, MCB_SERDES1G_CFG_MCB_SERDES1G_ADDR_CFG,
          VTSS_F_MACRO_CTRL_MCB_SERDES1G_CFG_MCB_SERDES1G_ADDR_CFG_SERDES1G_ADDR(addr) |
          VTSS_F_MACRO_CTRL_MCB_SERDES1G_CFG_MCB_SERDES1G_ADDR_CFG_SERDES1G_WR_ONE_SHOT);

    /* Wait until write operation is completed  */
    JR_POLL_BIT(MACRO_CTRL, MCB_SERDES1G_CFG_MCB_SERDES1G_ADDR_CFG, SERDES1G_WR_ONE_SHOT);

    if (nsec)
        VTSS_NSLEEP(nsec);

    return VTSS_RC_OK;
}

/* Serdes6G setup */
#define RCOMP_CFG0 VTSS_IOREG(VTSS_TO_HSIO,0x8)
static vtss_rc jr_sd6g_cfg(vtss_state_t *vtss_state, vtss_serdes_mode_t mode, u32 addr)
{
    /* Initialize parameters to default values */
    u32  ob_post0 = 0, ob_prec = 0, ob_lev = 48, ob_ena_cas = 0, ob_pol = 1;
    u32  ib_ic_ac = 0, ib_rf = 15, ib_vbac = 0, ib_vbcom = 0, ib_c = 0, ib_chf = 0;
    BOOL ob_ena1v = 0, ib_dis_eq = 1, if_100fx = 0, ser_4tab_ena = 0, ser_enali = 0, ser_alisel = 0;
    BOOL div4 = 0, ena_rot = 0, ena_lane = 1, qrate = 1, hrate = 0,  rot_dir = 0;
    u32  ctrl_data = 60, if_mode = 1, rcomp_val, des_mbtr_ctrl = 0, des_cpmd_sel = 0;

    ob_ena1v = (vtss_state->init_conf.serdes.serdes6g_vdd == VTSS_VDD_1V0) ? 1 : 0;

    /* Override default values depending on mode */
    switch (mode) {
    case VTSS_SERDES_MODE_RXAUI:
        if (ob_ena1v) {
            /* 1.0 V */
            ob_prec = 5; /* OB_CFG */
            ob_ena1v = 1;/* OB_CFG */
        } else {
            /* 1.2 V */
            ob_post0 = 11;/* OB_CFG */
            ob_prec = 8; /* OB_CFG */
            ob_lev = 63; /* OB_CFG */
        }
        ib_ic_ac = 2;    /* IB_CFG */
        ib_rf = 2;       /* IB_CFG */
        ib_vbac = 4;     /* IB_CFG */
        ib_vbcom = 5;    /* IB_CFG */
        ib_c = 6;        /* IB_CFG */
        ib_chf = 1;      /* IB_CFG */
        ib_dis_eq = 1;   /* IB_CFG1 */
        ena_rot = 1;     /* PLL_CFG */
        ctrl_data = 96;  /* PLL_CFG */
        if_mode = 3;     /* COMMON_CFG */
        qrate = 0;       /* COMMON_CFG */ 
        ser_4tab_ena = 1;/* SER_CFG */
        ser_enali = 1;   /* SER_CFG */
        ser_alisel = 1;  /* SER_CFG */
        break;
    case VTSS_SERDES_MODE_XAUI:
    case VTSS_SERDES_MODE_2G5:
        ob_post0 = 2;    /* OB_CFG */
        ob_prec = 2;     /* OB_CFG */
        ib_ic_ac = 2;    /* IB_CFG */
        ib_rf = 2;       /* IB_CFG */
        ib_vbac = 4;     /* IB_CFG */
        ib_vbcom = 5;    /* IB_CFG */
        ib_c = 6;        /* IB_CFG */
        ib_chf = 1;      /* IB_CFG */
        ib_dis_eq = 1;   /* IB_CFG1 */
        ena_rot = 1;     /* PLL_CFG */
        ctrl_data = 48;  /* PLL_CFG */
        qrate = 0;       /* COMMON_CFG */
        hrate = 1;       /* COMMON_CFG */
        ser_4tab_ena = 1;/* SER_CFG */
        ser_enali = 1;   /* SER_CFG */
        ser_alisel = 1;  /* SER_CFG */
        break;
    case VTSS_SERDES_MODE_XAUI_12G:
        ib_rf = 8;       /* IB_CFG */
        ib_c = 8;        /* IB_CFG */
        ib_dis_eq = 1;   /* IB_CFG1 */
        ena_rot = 1;     /* PLL_CFG */
        ctrl_data = 80;  /* PLL_CFG */
        div4 = 0;        /* PLL_CFG */
        rot_dir = 1;     /* PLL_CFG */
        qrate = 0;       /* COMMON_CFG */
        ser_enali = 1;   /* SER_CFG */
        ser_alisel = 1;  /* SER_CFG */
        break;
    case VTSS_SERDES_MODE_SGMII:
        ob_ena_cas = 2;  /* OB_CFG */
        ib_c = 15;       /* IB_CFG */
        ib_dis_eq = 1;   /* IB_CFG1 */
        break;
    case VTSS_SERDES_MODE_100FX:
        ib_c = 15;       /* IB_CFG */
        if_100fx = 1;    /* IB_CFG1 */
        ib_dis_eq = 1;   /* IB_CFG1 */
        des_mbtr_ctrl = 3;/* DES_CFG */
        des_cpmd_sel = 2; /* DES_CFG */
        break;
    case VTSS_SERDES_MODE_1000BaseX:
        ob_ena_cas = 2;  /* OB_CFG1 */
        ib_c = 15;       /* IB_CFG */
        ib_dis_eq = 1;   /* IB_CFG1 */
        break;
    case VTSS_SERDES_MODE_DISABLE:
        ena_lane = 0;
        ob_lev = 0;
        ib_rf = 0;
        ib_dis_eq = 0;
        ctrl_data = 0;
        if_mode = 0;
        qrate = 0;
        break;
    default:
        VTSS_E("Serdes6g mode not supported:%u",mode);
        return VTSS_RC_ERROR;
    }
    /* RCOMP_CFG0.MODE_SEL = 2 */
    VTSS_RC(vtss_jr1_wr(vtss_state, RCOMP_CFG0,0x3<<8));
    
    /* RCOMP_CFG0.RUN_CAL = 1 */
    VTSS_RC(vtss_jr1_wr(vtss_state, RCOMP_CFG0, 0x3<<8|1<<12));
    
    /* Wait for calibration to finish */
    JR_POLL_BIT(MACRO_CTRL, RCOMP_STATUS_RCOMP_STATUS, BUSY);  
    JR_RDF(MACRO_CTRL, RCOMP_STATUS_RCOMP_STATUS, RCOMP, &rcomp_val);

    /* 1. Configure macro, apply reset */
    /* OB_CFG */
    JR_WRM(MACRO_CTRL, SERDES6G_ANA_CFG_SERDES6G_OB_CFG,
           (ob_pol ? VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG_OB_POL : 0) |
           VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG_OB_POST0(ob_post0) |
           VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG_OB_PREC(ob_prec) |
           VTSS_ENCODE_BITFIELD(rcomp_val+1,4,4) | /* ob_resistor_ctrl: bit 4-7! (RCOMP readout + 1) */
           (ob_ena1v ? VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG_OB_ENA1V_MODE : 0),

           VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG_OB_POL |
           VTSS_M_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG_OB_POST0 |
           VTSS_M_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG_OB_PREC |
           VTSS_ENCODE_BITMASK(4,4) | /* ob_resistor_ctrl: bit 4-7! */
           VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG_OB_ENA1V_MODE);
    
    /* OB_CFG1 */
    JR_WRM(MACRO_CTRL, SERDES6G_ANA_CFG_SERDES6G_OB_CFG1,
           VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG1_OB_ENA_CAS(ob_ena_cas) |
           VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG1_OB_LEV(ob_lev),
           VTSS_M_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG1_OB_ENA_CAS |
           VTSS_M_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG1_OB_LEV);

     /* IB_CFG */      
    JR_WRM(MACRO_CTRL, SERDES6G_ANA_CFG_SERDES6G_IB_CFG,
          VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_IC_AC(ib_ic_ac) | 
          VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_RT(15) | 
          VTSS_ENCODE_BITFIELD(ib_vbac,7,3) | 
          VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_RESISTOR_CTRL(rcomp_val+2) |
          VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_VBCOM(ib_vbcom) |
          VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_RF(ib_rf),
          VTSS_M_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_IC_AC |
          VTSS_M_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_RT |
          VTSS_ENCODE_BITMASK(7,3) | 
          VTSS_M_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_RESISTOR_CTRL |
          VTSS_M_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_VBCOM |
          VTSS_M_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_RF);

    /* IB_CFG1 */      
    JR_WRM(MACRO_CTRL, SERDES6G_ANA_CFG_SERDES6G_IB_CFG1,
           VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_C(ib_c) |
           (ib_chf ? VTSS_BIT(7) : 0) |
           (ib_dis_eq ? VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_DIS_EQ : 0) |
           VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_ENA_OFFSAC |
           VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_ENA_OFFSDC |
           (if_100fx ? VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_FX100_ENA : 0) |
           VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_RST,
           VTSS_M_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_C |
           VTSS_BIT(7) |
           VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_DIS_EQ |
           VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_ENA_OFFSAC |
           VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_ENA_OFFSDC |
           VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_FX100_ENA |
           VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_RST);
    
    /* DES_CFG */      
    JR_WRM(MACRO_CTRL, SERDES6G_ANA_CFG_SERDES6G_DES_CFG,
           VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_DES_CFG_DES_BW_ANA(5) |
           VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_DES_CFG_DES_CPMD_SEL(des_cpmd_sel) |
           VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_DES_CFG_DES_MBTR_CTRL(des_mbtr_ctrl),
           VTSS_M_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_DES_CFG_DES_BW_ANA |
           VTSS_M_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_DES_CFG_DES_CPMD_SEL |
           VTSS_M_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_DES_CFG_DES_MBTR_CTRL);

    /* SER_CFG */      
    JR_WRM(MACRO, CTRL_SERDES6G_ANA_CFG_SERDES6G_SER_CFG,
          (ser_4tab_ena ? VTSS_BIT(8) : 0) |
          VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_SER_CFG_SER_ALISEL(ser_alisel) |
           (ser_enali ? VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_SER_CFG_SER_ENALI : 0),
           VTSS_BIT(8) |
           VTSS_M_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_SER_CFG_SER_ALISEL |
           VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_SER_CFG_SER_ENALI);

    /* PLL_CFG */      
    JR_WRM(MACRO, CTRL_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG,
          VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG_PLL_FSM_CTRL_DATA(ctrl_data) |
          VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG_PLL_FSM_ENA |
          VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG_PLL_ROT_FRQ |
          (rot_dir ? VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG_PLL_ROT_DIR : 0) |
          (div4 ? VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG_PLL_DIV4 : 0) |
           (ena_rot ? VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG_PLL_ENA_ROT : 0),
           VTSS_M_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG_PLL_FSM_CTRL_DATA |
           VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG_PLL_FSM_ENA |
           VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG_PLL_ROT_FRQ |
           VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG_PLL_ROT_DIR |
           VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG_PLL_DIV4 |
           VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG_PLL_ENA_ROT);

    /* Write masked to avoid changing RECO_SEL_* fields used by SyncE */
    /* COMMON_CFG */      
    JR_WRM(MACRO_CTRL, SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG,
           (ena_lane ? VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_ENA_LANE : 0) |
           (hrate ? VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_HRATE : 0) |
           (qrate ? VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_QRATE : 0) |
           VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_IF_MODE(if_mode),
           VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_SYS_RST |
           VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_ENA_LANE |
           VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_HRATE |
           VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_QRATE |
           VTSS_M_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_IF_MODE);
    
    /* MISC_CFG */      
    JR_WRM(MACRO_CTRL, SERDES6G_DIG_CFG_SERDES6G_MISC_CFG,
           (if_100fx ? VTSS_F_MACRO_CTRL_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG_DES_100FX_CPMD_ENA : 0),
           VTSS_F_MACRO_CTRL_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG_DES_100FX_CPMD_ENA);

    JR_WRM(MACRO_CTRL, SERDES6G_DIG_CFG_SERDES6G_MISC_CFG,
           (if_100fx ? VTSS_F_MACRO_CTRL_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG_DES_100FX_CPMD_ENA : 0) |
           VTSS_F_MACRO_CTRL_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG_LANE_RST,
           VTSS_F_MACRO_CTRL_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG_DES_100FX_CPMD_ENA |
           VTSS_F_MACRO_CTRL_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG_LANE_RST);
    
    VTSS_RC(jr_sd6g_write(vtss_state, addr, 60000));
    
    /* In RXAUI mode, keep odd lanes reset to avoid crosstalk */
    if (mode == VTSS_SERDES_MODE_RXAUI)
        addr &= 0x55555555;

    /* 2. Release PLL Reset */
    JR_WRB(MACRO_CTRL, SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG, SYS_RST, 1);
    VTSS_RC(jr_sd6g_write(vtss_state, addr, 130000));
    
    /* 3. Release digital reset */
    JR_WRB(MACRO_CTRL, SERDES6G_DIG_CFG_SERDES6G_MISC_CFG, LANE_RST, 0);
    JR_WRB(MACRO_CTRL, SERDES6G_ANA_CFG_SERDES6G_IB_CFG1, IB_RST, 0);
    VTSS_RC(jr_sd6g_write(vtss_state, addr, 60000));

    return VTSS_RC_OK;
}

static vtss_rc jr_sd1g_cfg(vtss_state_t *vtss_state, vtss_serdes_mode_t mode, u32 adr)
{
    BOOL ena_lane, if_100fx, ib_ena_dc=0;
    u32  ob_amp_ctrl, rcomp_val, resist_val, value, rev, des_bw_ana=6, cpmd_sel=0, des_mbtr_ctrl=0;
    
    switch (mode) {
    case VTSS_SERDES_MODE_SGMII:
        ena_lane = 1;
        ob_amp_ctrl = 12;
        if_100fx = 0;      
        break;
    case VTSS_SERDES_MODE_100FX:
        ena_lane = 1;
        ob_amp_ctrl = 12;
        ib_ena_dc = 1;
        des_bw_ana = 0;
        cpmd_sel = 2;
        des_mbtr_ctrl = 3;
        if_100fx = 1;
        break;
    case VTSS_SERDES_MODE_1000BaseX:
        ena_lane = 1;
        ob_amp_ctrl = 15;
        if_100fx = 0;
        break;
    case VTSS_SERDES_MODE_DISABLE:
        if_100fx = 0;
        ena_lane = 0;
        ob_amp_ctrl = 0;
        break;
    default:
        VTSS_E("Serdes1g mode not supported");
        return VTSS_RC_ERROR;
    }

    JR_RD(DEVCPU_GCB, CHIP_REGS_CHIP_ID, &value);
    rev = JR_GET_FLD(DEVCPU_GCB, CHIP_REGS_CHIP_ID, REV_ID, value);

    /* RCOMP_CFG0.MODE_SEL = 2 */
    VTSS_RC(vtss_jr1_wr(vtss_state, RCOMP_CFG0,0x3<<8));
    
    /* RCOMP_CFG0.RUN_CAL = 1 */
    VTSS_RC(vtss_jr1_wr(vtss_state, RCOMP_CFG0, 0x3<<8|1<<12));
    
    /* Wait for calibration to finish */
    JR_POLL_BIT(MACRO_CTRL, RCOMP_STATUS_RCOMP_STATUS, BUSY);  
    JR_RDF(MACRO_CTRL, RCOMP_STATUS_RCOMP_STATUS, RCOMP, &rcomp_val);
        
    /* 1. Enable sig_det circuitry, lane, select if mode, apply reset */

    JR_WRM(MACRO_CTRL, SERDES1G_ANA_CFG_SERDES1G_IB_CFG,
           (if_100fx ? VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_IB_CFG_IB_FX100_ENA : 0) |
           VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_IB_CFG_IB_ENA_DETLEV |
           VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_IB_CFG_IB_DET_LEV(3) |
           VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_IB_CFG_IB_ENA_CMV_TERM |
           (ib_ena_dc ? VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_IB_CFG_IB_ENA_DC_COUPLING : 0) |
           VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_IB_CFG_IB_ENA_OFFSET_COMP |
           VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_IB_CFG_IB_RESISTOR_CTRL(rcomp_val-3) |
           VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_IB_CFG_IB_EQ_GAIN(3),
           VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_IB_CFG_IB_FX100_ENA |
           VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_IB_CFG_IB_ENA_DETLEV |
           VTSS_M_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_IB_CFG_IB_DET_LEV |
           VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_IB_CFG_IB_ENA_CMV_TERM |
           VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_IB_CFG_IB_ENA_DC_COUPLING |
           VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_IB_CFG_IB_ENA_OFFSET_COMP |
           VTSS_M_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_IB_CFG_IB_RESISTOR_CTRL |
           VTSS_M_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_IB_CFG_IB_EQ_GAIN);

    JR_WRM(MACRO_CTRL, SERDES1G_ANA_CFG_SERDES1G_DES_CFG,
           VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_DES_CFG_DES_BW_ANA(des_bw_ana) |
           VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_DES_CFG_DES_CPMD_SEL(cpmd_sel) |
           VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_DES_CFG_DES_MBTR_CTRL(des_mbtr_ctrl),
           VTSS_M_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_DES_CFG_DES_BW_ANA |
           VTSS_M_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_DES_CFG_DES_CPMD_SEL |
           VTSS_M_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_DES_CFG_DES_MBTR_CTRL);


    resist_val = (vtss_state->init_conf.serdes.serdes1g_vdd == VTSS_VDD_1V0) ? rcomp_val+1 : rcomp_val-4;
    JR_WRM(MACRO_CTRL, SERDES1G_ANA_CFG_SERDES1G_OB_CFG,
           VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_OB_CFG_OB_RESISTOR_CTRL(resist_val) |
           VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_OB_CFG_OB_AMP_CTRL(ob_amp_ctrl),
           VTSS_M_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_OB_CFG_OB_RESISTOR_CTRL |
           VTSS_M_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_OB_CFG_OB_AMP_CTRL);

    /* Write masked to avoid changing RECO_SEL_* fields used by SyncE */
    JR_WRM(MACRO_CTRL, SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG,
            (ena_lane ? VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG_ENA_LANE : 0) | VTSS_BIT(0),            
            VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG_SYS_RST |
            VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG_ENA_LANE |
           (rev == 0 ? VTSS_BIT(7) : 0) | /* Rev 0 -> HRATE = 0.  Rev 1 -> HRATE = 1 (default) */
            VTSS_BIT(0)); /* IFMODE = 1 */


    JR_WRM(MACRO_CTRL, SERDES1G_ANA_CFG_SERDES1G_PLL_CFG,
           VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_PLL_CFG_PLL_FSM_ENA |
           (rev == 0 ? VTSS_BIT(21) : 0) | /* Rev 0 -> RC_DIV2 = 1. Rev 1 -> RC_DIV2 = 0 (default) */
           VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_PLL_CFG_PLL_FSM_CTRL_DATA(200),
           VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_PLL_CFG_PLL_FSM_ENA |
           VTSS_BIT(21) |
           VTSS_M_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_PLL_CFG_PLL_FSM_CTRL_DATA);
    

    JR_WRM(MACRO_CTRL, SERDES1G_DIG_CFG_SERDES1G_MISC_CFG,
            (if_100fx ? VTSS_F_MACRO_CTRL_SERDES1G_DIG_CFG_SERDES1G_MISC_CFG_DES_100FX_CPMD_ENA : 0) |
            VTSS_F_MACRO_CTRL_SERDES1G_DIG_CFG_SERDES1G_MISC_CFG_LANE_RST,
            VTSS_F_MACRO_CTRL_SERDES1G_DIG_CFG_SERDES1G_MISC_CFG_DES_100FX_CPMD_ENA |
            VTSS_F_MACRO_CTRL_SERDES1G_DIG_CFG_SERDES1G_MISC_CFG_LANE_RST);

    VTSS_RC(jr_sd1g_write(vtss_state, adr, 40000));
    /* 2. Release PLL Reset  */

    JR_WRB(MACRO_CTRL, SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG, SYS_RST, 1);

    VTSS_RC(jr_sd1g_write(vtss_state, adr, 90000));

    /* 3. Release digital reset */

    JR_WRB(MACRO_CTRL, SERDES1G_DIG_CFG_SERDES1G_MISC_CFG, LANE_RST, 0);
    VTSS_RC(jr_sd1g_write(vtss_state, adr, 0));
    return VTSS_RC_OK;
}

#define  P_IQS_SWM 122
#define  Q_OQS_SWM 15
#define  IQS_HWM (VTSS_PORTS * 40 * VTSS_PRIOS * 4/5)
#define  SQS_BUF_TOTAL 26240
#define  SQS_PORTS_ALL 0xFFFF

static vtss_rc jr_qs_conf_update_priv(vtss_state_t *vtss_state, u32 chip_port)
{
    u32 p, iqs_port_swm_cell=0, oqs_port_queue_swm_cell=0, oqs_buf_bytes, q, queue_sum_bytes=0, port_sum_bytes=0;
    u32 qu_cell, port_cell, oqs_buf_cell; 
    sqs_t sqs;
    vtss_port_speed_t speed;
    BOOL rc_mode_queue=0, rc_mode_port=0;
    vtss_qs_conf_t *qs_conf = &vtss_state->init_conf.qs_conf;
    BOOL jumbo = (jr_target_get(vtss_state) == VTSS_TARGET_JAGUAR_1 || jr_target_get(vtss_state) == VTSS_TARGET_LYNX_1) ? 0 : 1;    

    memset(&sqs,0,sizeof(sqs));

    if (qs_conf->mode == VTSS_QS_MODE_DISABLED) {
        VTSS_E("User defined QS configuration is disabled");
        return VTSS_RC_ERROR;
    }

    /* Verify either port OR queue min guaranties */
    for (p = 0; p < VTSS_PORTS; p++) {
        if (qs_conf->mode == VTSS_QS_MODE_SHARED) {
            port_cell = qs_conf->port[p].port_bytes/160 + (qs_conf->port[p].port_bytes % 160 ? 1 : 0);        
            for (q = 0; q < VTSS_PRIOS; q++) {
                qu_cell =  qs_conf->port[p].queue_bytes[q]/160 + (qs_conf->port[p].queue_bytes[q] % 160 ? 1 : 0);
                if (((qu_cell > 0) && (port_cell > 0)) || ((qu_cell == 0) && (port_cell == 0))) {
                    VTSS_E("VTSS_QS_MODE_SHARED requires that you choose between port (%d) and queue (%d) minimum guarantees.", port_cell, qu_cell);
                    return VTSS_RC_ERROR;
                }
            }
        }
    }
    /* Verify oversubscription */
    if (qs_conf->oversubscription > 50) {
        VTSS_E("Oversubscription over 50 pct is not allowed");
        return VTSS_RC_ERROR;
    }

    for (p = 0; p < VTSS_PORTS; p++) {
        port_sum_bytes += qs_conf->port[p].port_bytes;
        iqs_port_swm_cell += VTSS_PORT_IS_1G(VTSS_CHIP_PORT(p)) ? P_IQS_SWM : P_IQS_SWM * 2;
        if (qs_conf->mode == VTSS_QS_MODE_STRICT) {
            oqs_port_queue_swm_cell += Q_OQS_SWM * VTSS_PRIOS;
        } else {
            if (qs_conf->port[p].queue_bytes[0] > 0) {
                for (q = 0; q < VTSS_PRIOS; q++) {
                    oqs_port_queue_swm_cell += qs_conf->port[p].queue_bytes[q]/160 + (qs_conf->port[p].queue_bytes[q] % 160 ? 1 : 0);
                }
            } else {
                oqs_port_queue_swm_cell += qs_conf->port[p].port_bytes/160 + (qs_conf->port[p].port_bytes % 160 ? 1 : 0);
            }
        }
    }

    /* Verify that the requested config is within range */
    if ((IQS_HWM + iqs_port_swm_cell + oqs_port_queue_swm_cell)/(qs_conf->oversubscription + 100) * 100  >= SQS_BUF_TOTAL) {
        VTSS_E("The requested port/queue buffer is to high");
        return VTSS_RC_ERROR;
    }
    oqs_buf_bytes = 160 * (((SQS_BUF_TOTAL - IQS_HWM - iqs_port_swm_cell - oqs_port_queue_swm_cell) * (qs_conf->oversubscription + 100)) / 100); 

    /* Verify VTSS_QS_MODE_STRICT */
    if (qs_conf->mode == VTSS_QS_MODE_STRICT) {
        for (p = 0; p < VTSS_PORTS; p++) {
            queue_sum_bytes = 0;
            for (q = 0; q < VTSS_PRIOS; q++) {
                queue_sum_bytes += qs_conf->port[p].queue_bytes[q];
            }
            for (q = 0; q < VTSS_PRIOS; q++) {
                if (oqs_buf_bytes / VTSS_PORTS >= qs_conf->port[p].port_bytes) {
                    VTSS_E("(Strict)OQS BUF(%u) / Ports(%u) >= port_bytes(%u)!",oqs_buf_bytes, VTSS_PORTS, qs_conf->port[p].port_bytes);
                    return VTSS_RC_ERROR;
                } else if (qs_conf->port[p].port_bytes >= oqs_buf_bytes) {
                    VTSS_E("(Strict)port_bytes(%u) >= OQS BUF(%u)!",qs_conf->port[p].port_bytes, oqs_buf_bytes);
                    return VTSS_RC_ERROR;
                } else if (qs_conf->port[p].queue_bytes[q] >= qs_conf->port[p].port_bytes) {
                    VTSS_E("(Strict)queue_bytes[q](%u) (q:%u) >= port_bytes(%u) (port:%u)!",
                           qs_conf->port[p].queue_bytes[q], q, qs_conf->port[p].port_bytes, p);
                    return VTSS_RC_ERROR;
                } else if (qs_conf->port[p].port_bytes / VTSS_PRIOS >= qs_conf->port[p].queue_bytes[q]) {
                    VTSS_E("(Strict)port_max(%u) p:%u/ VTSS_PRIOS > queue_max(%u) q:%u!",
                           qs_conf->port[p].port_bytes, p, qs_conf->port[p].queue_bytes[q], q);
                    return VTSS_RC_ERROR;
                } else if (queue_sum_bytes <= qs_conf->port[p].port_bytes) {
                    VTSS_E("(Strict)Sum of queues(%u) <= port_bytes(%u)!",queue_sum_bytes, qs_conf->port[p].queue_bytes[q]);
                    return VTSS_RC_ERROR;
                }
            }
        }
    } else {
    /* Verify VTSS_QS_MODE_SHARED */
        if (qs_conf->port[0].queue_bytes[0] > 0) {
            // (queue_val_sum * VTSS_PORTS >= oqs_buf_byte) already covered
            rc_mode_queue = 1;
            rc_mode_port = 0;
        } else  {
            for (p = 0; p < VTSS_PORTS; p++) {
                if (qs_conf->port[p].port_bytes >= (oqs_buf_bytes / VTSS_PORTS)) {
                    VTSS_E("(Shared)port guaranty(%u) >= (oqs_buf_byte(%u) / VTSS_PORTS(%u))!", qs_conf->port[p].port_bytes, oqs_buf_bytes, VTSS_PORTS);
                    return VTSS_RC_ERROR;
                } else if (port_sum_bytes >= oqs_buf_bytes) {
                    VTSS_E("(Shared)sum of port guaranties (%u) >= OQS BUF(%u) !", port_sum_bytes, oqs_buf_bytes);
                    return VTSS_RC_ERROR;
                }
            }
            rc_mode_queue = 0;
            rc_mode_port = 1;
        }
    }
    oqs_buf_cell = oqs_buf_bytes / 160;

    /* All is clear.. apply the config */
    for (p = 0; p < VTSS_PORTS; p++) {
        if (chip_port != SQS_PORTS_ALL && VTSS_CHIP_PORT(p) != chip_port) {
            continue;
        } 
        port_cell = qs_conf->port[p].port_bytes/160 + (qs_conf->port[p].port_bytes % 160 ? 1 : 0);
        speed = VTSS_PORT_IS_1G(VTSS_CHIP_PORT(p)) ? VTSS_SPEED_1G : VTSS_SPEED_10G;

        /* Get the defaults */
        VTSS_RC(jr_sqs_rd(vtss_state, 0, &sqs));

        /* Calclate the current settings */
        VTSS_RC(jr_sqs_calc_wm(vtss_state, VTSS_CHIP_PORT(p), speed, jumbo, 0, 0, 0, &sqs));

        /* Modify the current settings */
        sqs.iqs.atop = IQS_HWM + 63;
        sqs.iqs.hwm  = IQS_HWM;
        sqs.iqs.lwm  = IQS_HWM - 100;        
        sqs.iqs.port.gwm  = sqs.iqs.port.hwm + 10; // Green WM for CEService
        sqs.oqs.atop = oqs_buf_cell + 63;
        sqs.oqs.hwm  = oqs_buf_cell + 20 ;
        sqs.oqs.lwm  = oqs_buf_cell - 40;

        if (qs_conf->mode == VTSS_QS_MODE_STRICT) {
        /* VTSS_QS_MODE_STRICT */
            for (q = 0; q < VTSS_PRIOS; q++) {
                qu_cell = qs_conf->port[p].queue_bytes[q] / 160 + (qs_conf->port[p].queue_bytes[q] % 160 ? 1 : 0);
                sqs.oqs.port.qu[q].atop = qu_cell + 63;
                sqs.oqs.port.qu[q].hwm  = qu_cell;
                sqs.oqs.port.qu[q].lwm  = qu_cell - 15;
            }
            sqs.oqs.port.atop = port_cell + 63;
            sqs.oqs.port.gwm  = port_cell + 20;
            sqs.oqs.port.hwm  = port_cell; 
            sqs.oqs.port.lwm  = port_cell - 100;

        } else {
        /* VTSS_QS_MODE_SHARED */
            for (q = 0; q < VTSS_PRIOS; q++) {
                qu_cell = qs_conf->port[p].queue_bytes[q] / 160 + (qs_conf->port[p].queue_bytes[q] % 160 ? 1 : 0);
                sqs.oqs.port.qu[q].cnt_on_buf_port = rc_mode_queue;
                sqs.oqs.port.qu[q].rc_mode = rc_mode_queue;
                sqs.oqs.port.qu[q].atop    = oqs_buf_cell + 63;
                sqs.oqs.port.qu[q].hwm     = oqs_buf_cell;
                sqs.oqs.port.qu[q].lwm     = oqs_buf_cell - 15;
                sqs.oqs.port.qu[q].swm     = qu_cell * rc_mode_queue;
            }
            sqs.oqs.port.cnt_on_buf     = rc_mode_port;
            sqs.oqs.port.rc_mode        = rc_mode_port;
            sqs.oqs.port.atop           = oqs_buf_cell + 63;
            sqs.oqs.port.gwm            = oqs_buf_cell;
            sqs.oqs.port.hwm            = oqs_buf_cell - 20; 
            sqs.oqs.port.lwm            = oqs_buf_cell - 120;
            sqs.oqs.port.swm            = port_cell * rc_mode_port;
        }
        /* Apply the new settings */
        if (jr_sqs_set(vtss_state, VTSS_CHIP_PORT(p), &sqs) != VTSS_RC_OK) {
            VTSS_E("Could not jr_sqs_set()");
        }
    }

    return VTSS_RC_OK;
}

static vtss_rc jr_qs_conf_set(vtss_state_t *vtss_state)
{   
    VTSS_RC(jr_qs_conf_update_priv(vtss_state, SQS_PORTS_ALL));
    return VTSS_RC_OK;
}

typedef enum
{
    SERDES_1G,
    SERDES_6G,
} serdes_t;

/* Convert port to serdes instance */
static vtss_rc jr_port2serdes_get(vtss_state_t *vtss_state,
                                  u32 port, serdes_t *type, u32 *lane, u32 *count)
{
    vtss_rc              rc = VTSS_RC_OK;
    vtss_target_type_t   target = jr_target_get(vtss_state);
    vtss_port_mux_mode_t mux_mode = vtss_state->port.mux_mode[vtss_state->chip_no];
    u32                  base_1g = 0, cnt_1g = 24;
    u32                  base_2g5 = 23, cnt_2g5 = 0;
    u32                  base_10g = VTSS_PORT_10G_START, cnt_10g = 4;
    
    /* Determine base and count for 1G/2.5G/10G ports based on target and mux mode */
    switch (target) {
    case VTSS_TARGET_JAGUAR_1:
    case VTSS_TARGET_E_STAX_III_48:
    case VTSS_TARGET_E_STAX_III_68:
    case VTSS_TARGET_CE_MAX_24:
        if (mux_mode == VTSS_PORT_MUX_MODE_0) {
            /* 24x1G + 4x10G + NPI */
        } else if (mux_mode == VTSS_PORT_MUX_MODE_1) {
            /* 23x1G + 4x2g5 + 3x10G + NPI */
            cnt_1g = 23;
            cnt_2g5 = 4;
            base_10g = 28;
            cnt_10g = 3;
        } else if (mux_mode == VTSS_PORT_MUX_MODE_7) {
            /* 19x1G + 8x2g5 + 2x10G + NPI */
            cnt_1g = 19;
            base_2g5 = 19;
            cnt_2g5 = 8;
            base_10g = 29;
            cnt_10g = 2;
        } else {
            VTSS_E("Mux mode no supported");
            rc = VTSS_RC_ERROR;
        }
        break;
    case VTSS_TARGET_LYNX_1:
    case VTSS_TARGET_CE_MAX_12:
        base_1g = 11;
        cnt_1g = 12;
        if (mux_mode == VTSS_PORT_MUX_MODE_0) {
            /* 12x1G + 2x10G + NPI */
            cnt_10g = 2;
        } else if (mux_mode == VTSS_PORT_MUX_MODE_1) {
            /* 12x1G + 4x2g5 + 1x10g + NPI */
            cnt_2g5 = 4;
            base_10g = 28;
            cnt_10g = 1;
        } else if (mux_mode == VTSS_PORT_MUX_MODE_7) {
            /* 8x1G + 8x2g5 + NPI */
            if (target == VTSS_TARGET_CE_MAX_12)
                rc = VTSS_RC_ERROR;
            cnt_1g = 8;
            base_2g5 = 19;
            cnt_2g5 = 8;
            base_10g = 29;
            cnt_10g = 0;
        } else {
            VTSS_E("Mux mode no supported");
            rc = VTSS_RC_ERROR;
        }
        if (target == VTSS_TARGET_LYNX_1) {
            /* LynX supports two more XAUI ports */
            cnt_10g += 2;
        }
        break;
    default:
        VTSS_E("Unknown target");
        rc = VTSS_RC_ERROR;
        break;
    } /* switch */

    /* Determine SerDes information based on port information */
    if (port >= base_1g && port < (base_1g + cnt_1g)) {
        /* 1G port */
        *type = SERDES_1G;
        *lane = port;
        *count = 1;
    } else if (port >= base_2g5 && port < (base_2g5 + cnt_2g5)) {
        /* 2.5G port */
        *type = SERDES_6G;
        *lane = (port - base_2g5);
        *count = 1;
    } else if (port >= base_10g && port < (base_10g + cnt_10g)) {
        /* 10G port */
        *type = SERDES_6G;
        *lane = 4*(port - VTSS_PORT_10G_START);
        *count = 4;
    } else if (port == VTSS_PORT_NPI) {
        /* NPI port */
        *type = SERDES_1G;
        *lane = 24;
        *count = 1;
    } else {
        /* Illegal port */
        rc = VTSS_RC_ERROR;
    }

    return rc;
}

/* 10G port rate control register values */
#define JR_RATE_CTRL_GAP_10G   20
#define JR_RATE_CTRL_WM_LO_10G 639
#define JR_RATE_CTRL_WM_HI_10G 641

vtss_rc vtss_jr1_port_setup(vtss_state_t *vtss_state,
                            vtss_port_no_t port_no, u32 chipport, vtss_port_conf_t *conf, BOOL front_port)
{
    const u8 *mac;
    u32      wm_low = 0, wm_high, wm_stop = 4, mac_hi, mac_lo, gap = 12, value;
    BOOL     dev1g = (chipport < 18), valid_sqs_conf = 1;
    sqs_t    sqs;
#if defined(VTSS_FEATURE_QOS_DOT3AR_RATE_LIMITER)
    u32      tmp_gap_val;
#endif /* VTSS_FEATURE_QOS_DOT3AR_RATE_LIMITER */
#if defined(VTSS_FEATURE_VSTAX)
    u32      portmask = VTSS_BIT(chipport), prio;
#endif
    
    vtss_target_type_t target = jr_target_get(vtss_state);

    switch (conf->speed) {
    case VTSS_SPEED_10M:
        wm_high = (dev1g ? 7 : 4);
        break;
    case VTSS_SPEED_100M:
        wm_high = (dev1g ? 30 : 58);
        break;
    case VTSS_SPEED_1G:
        wm_low = (dev1g ? 255 : 63);
        wm_high = (dev1g ? 257 : 65);
        break;
    case VTSS_SPEED_2500M:
        wm_low = 159;
        wm_high = 161;
        break;
    case VTSS_SPEED_10G:
        wm_low = JR_RATE_CTRL_WM_LO_10G;
        wm_high = JR_RATE_CTRL_WM_HI_10G;
        break;
    case VTSS_SPEED_12G:
        wm_low = 767;
        wm_high = 769;
        break;
    default:
        VTSS_E("illegal speed");
        return VTSS_RC_ERROR;
    }

    if (dev1g) {
        /* DEV1G */
        wm_stop = 3;
#if defined(VTSS_FEATURE_QOS_DOT3AR_RATE_LIMITER)
        JR_RDX(DSM, CFG_RATE_CTRL, chipport, &tmp_gap_val);
        tmp_gap_val = VTSS_X_DSM_CFG_RATE_CTRL_FRM_GAP_COMP(tmp_gap_val);
        if (tmp_gap_val > 12) {
            gap = tmp_gap_val;
        }
#endif /* VTSS_FEATURE_QOS_DOT3AR_RATE_LIMITER */
    } else if (VTSS_PORT_IS_10G(chipport)) {
        /* DEV10G */
        JR_RDX(DSM, HOST_IF_CFG_HIH_CFG, (chipport - 27), &value);
        if (JR_GET_BIT(DSM, HOST_IF_CFG_HIH_CFG, HIH_ENA, value)) { 
            gap = 12;  /* HIH is enabled, do not compensate */
        } else {
            gap = JR_RATE_CTRL_GAP_10G;
#if defined(VTSS_FEATURE_QOS_DOT3AR_RATE_LIMITER)
            /* TBD: Make it generic once 802.3ar is supported on host interfaces also */
            JR_RDX(DSM, CFG_RATE_CTRL, chipport,&tmp_gap_val);
            tmp_gap_val = VTSS_X_DSM_CFG_RATE_CTRL_FRM_GAP_COMP(tmp_gap_val);
            if (tmp_gap_val > 12) {
                gap = tmp_gap_val;
            }
#endif /* VTSS_FEATURE_QOS_DOT3AR_RATE_LIMITER */
        }
        wm_stop = 5;
        JR_WRXF(DSM, CFG_RATE_CTRL_WM_DEV10G, (chipport - 27), TAXI_128_RATE_CTRL_WM, 
                conf->speed < VTSS_SPEED_10G ? 24 : 44);
    } else if (chipport != VTSS_PORT_NPI) {
        /* DEV2G5 */
        JR_WRXF(DSM, CFG_RATE_CTRL_WM_DEV2G5, (chipport - 18), TAXI_128_RATE_CTRL_WM_DEV2G5, 
                conf->speed == VTSS_SPEED_2500M ? 36 : 24);
    }

    /* Setup WM in DSM and ASM */  
    JR_WRX(DSM, CFG_SCH_STOP_WM_CFG, chipport, VTSS_F_DSM_CFG_SCH_STOP_WM_CFG_SCH_STOP_WM(wm_stop));
                                                          
    JR_WRX(DSM, CFG_RATE_CTRL, chipport,
           VTSS_F_DSM_CFG_RATE_CTRL_FRM_GAP_COMP(gap) |
           VTSS_F_DSM_CFG_RATE_CTRL_TAXI_RATE_HIGH(wm_high) |
           VTSS_F_DSM_CFG_RATE_CTRL_TAXI_RATE_LOW(wm_low));

    /* Flow Control configuration */
    JR_WRXB(DSM, CFG_RX_PAUSE_CFG, chipport, RX_PAUSE_EN, conf->flow_control.obey); 
    JR_WRXB(DSM, CFG_ETH_FC_GEN, chipport, ETH_PORT_FC_GEN, conf->flow_control.generate); 

#if defined(VTSS_FEATURE_VSTAX)
    /* Enable use of CMEF when the front port can generate flow control, otherwise disable it. */
    for (prio = 0; front_port && prio < (VTSS_PRIOS - 2); prio++) {
        JR_WRXM(ARB, CFG_STATUS_CM_CFG_VEC0, prio, conf->flow_control.generate ? portmask : 0, portmask);
    }
#endif

    if (port_no != VTSS_PORT_NO_NONE) {
        /* Update policer flow control configuration */
        VTSS_RC(vtss_jr1_port_policer_fc_set(vtss_state, port_no, chipport));
    }

    JR_WRX(DSM, CFG_MAC_CFG, chipport, VTSS_F_DSM_CFG_MAC_CFG_TX_PAUSE_VAL(0xFFFF) |
           (conf->fdx ? 0 : VTSS_F_DSM_CFG_MAC_CFG_HDX_BACKPRESSURE) |
           VTSS_F_DSM_CFG_MAC_CFG_TX_PAUSE_XON_XOFF);

    mac = conf->flow_control.smac.addr;
    mac_hi = ((mac[0] << 16) | (mac[1] << 8) | (mac[2] << 0));
    mac_lo = ((mac[3] << 16) | (mac[4] << 8) | (mac[5] << 0));
    JR_WRXF(DSM, CFG_MAC_ADDR_BASE_HIGH_CFG, chipport, MAC_ADDR_HIGH, mac_hi);
    JR_WRXF(DSM, CFG_MAC_ADDR_BASE_LOW_CFG, chipport, MAC_ADDR_LOW, mac_lo);
    JR_WRXF(ASM, CFG_MAC_ADDR_HIGH_CFG, chipport, MAC_ADDR_HIGH, mac_hi);
    JR_WRXF(ASM, CFG_MAC_ADDR_LOW_CFG, chipport, MAC_ADDR_LOW, mac_lo);
    JR_WRX(ASM, CFG_PAUSE_CFG, chipport, 
           VTSS_F_ASM_CFG_PAUSE_CFG_ABORT_PAUSE_ENA |
           VTSS_F_ASM_CFG_PAUSE_CFG_ABORT_CTRL_ENA);

    /* FC Stacking awareness */
    JR_WRXF(ANA_CL_2, PORT_STACKING_CTRL, chipport, IGR_DROP_ENA, (conf->flow_control.obey ? 0 : 0xFF));


    memset(&sqs,0,sizeof(sqs));
    if (target == VTSS_TARGET_E_STAX_III_48 || target == VTSS_TARGET_E_STAX_III_68) {
        if ((vtss_state->init_conf.qs_conf.mode == VTSS_QS_MODE_DISABLED) || conf->flow_control.obey) {
            /* Setup SQS WMs */
            VTSS_RC(jr_sqs_rd(vtss_state, chipport, &sqs));

            if (sqs.iqs.port.rx_ena && conf->power_down == 0) {
                /*  Power down port in the SQS WM calculation */            
                VTSS_RC(jr_sqs_calc_wm(vtss_state, chipport, conf->speed, 0, 0, 1, 0, &sqs));  
            } else if ((sqs.iqs.port.rx_ena == 0) && conf->power_down) {
                valid_sqs_conf = 0;
            }        
        
            /* Everything supported for Enterprise */
            if (valid_sqs_conf) {            
                /* Setup WM based on speed/jumbo/FC/power_down */
                VTSS_RC(jr_sqs_calc_wm(vtss_state, chipport, conf->speed, (conf->max_frame_length>1522?1:0), conf->flow_control.obey, conf->power_down, 0, &sqs)); 
                VTSS_RC(jr_sqs_set(vtss_state, chipport, &sqs));
            }
        } else {
            /* Apply user defined WMs */
            VTSS_RC(jr_qs_conf_update_priv(vtss_state, chipport));
        }
        
        if (port_no != VTSS_PORT_NO_NONE && chipport == VTSS_CHIP_PORT(port_no) &&
            VTSS_PORT_CHIP_SELECTED(port_no)) {
            VTSS_RC(vtss_jr1_qos_wred_conf_set(vtss_state, port_no));  /* Must update RED */
        }
    }
   
    /* Setup MTU */
    VTSS_RC(vtss_jr1_setup_mtu(vtss_state, chipport, conf->max_frame_length, front_port));

#if defined(VTSS_FEATURE_VSTAX_V2)
    VTSS_RC(vtss_jr1_vstax_update_ingr_shaper(vtss_state, chipport, conf->speed, front_port));
#endif /* VTSS_FEATURE_VSTAX_V2 */

    return VTSS_RC_OK;
}

static vtss_rc jr_queue_flush(vtss_state_t *vtss_state, u32 port, BOOL flush)
{
    u32 value, q;

    if (flush) {
        /* Disable Rx and TX queues, but don't flush them (see Gnats#6936 and Bugzilla#8127) */
        for (q = 0; q < 8; q++) {
            JR_WRXB(OQS, QU_RAM_CFG_QU_RC_CFG, (port*8+q), RX_QU_ENA, 0);
            JR_WRXB(IQS, QU_RAM_CFG_QU_RC_CFG, (port*8+q), RX_QU_ENA, 0);
        }

        /* Disable FC in DSM */
        JR_WRXB(DSM, CFG_RX_PAUSE_CFG, port, RX_PAUSE_EN, 0);
        
        /* Disable Rx and Tx in MAC */
        if (VTSS_PORT_IS_1G(port)) {
            JR_WRX(DEV1G, MAC_CFG_STATUS_MAC_ENA_CFG, VTSS_TO_DEV(port), 0);
        } else {
            JR_WRX(DEV10G, MAC_CFG_STATUS_MAC_ENA_CFG, VTSS_TO_DEV(port), 0);
        }

        /*  Remove port from the the  ANA forwarding and learn masks */
        JR_RD(ANA_L3_2, COMMON_PORT_FWD_CTRL, &value);
        JR_WR(ANA_L3_2, COMMON_PORT_FWD_CTRL, (value & ~VTSS_BIT(port)));
        if (vtss_state->create.target != VTSS_TARGET_CE_MAX_12 && 
            vtss_state->create.target != VTSS_TARGET_CE_MAX_24) { 
            JR_RD(ANA_L3_2, COMMON_PORT_LRN_CTRL, &value);
            JR_WR(ANA_L3_2, COMMON_PORT_LRN_CTRL, (value & ~VTSS_BIT(port)));
        }

        /* Wait a worst case time 8ms (jumbo/10Mbit) */
        VTSS_NSLEEP(8000000);
   
        /* Clear port buffer in DSM */
        JR_WRX(DSM, CFG_CLR_BUF, 0, VTSS_BIT(port));
    } else {
        /* Enable the port queues */
        for (q = 0; q < 8; q++) {
            JR_WRXB(IQS, QU_RAM_CFG_QU_RC_CFG, (port*8+q),  RX_QU_ENA, 1);
            JR_WRXB(OQS, QU_RAM_CFG_QU_RC_CFG, (port*8+q),  RX_QU_ENA, 1);
        }
        /* Enable transmission in the scheduler */
        JR_WRXF(SCH, QSIF_QSIF_ETH_PORT_CTRL, port, ETH_TX_DIS, 0);

        /*  Add the port to the  ANA forwarding and learn masks */
        JR_RD(ANA_L3_2, COMMON_PORT_FWD_CTRL, &value);
        JR_WR(ANA_L3_2, COMMON_PORT_FWD_CTRL, (value & ~VTSS_BIT(port)) | VTSS_BIT(port));
        if (vtss_state->create.target != VTSS_TARGET_CE_MAX_12 && 
            vtss_state->create.target != VTSS_TARGET_CE_MAX_24) {  
            JR_RD(ANA_L3_2, COMMON_PORT_LRN_CTRL, &value);
            JR_WR(ANA_L3_2, COMMON_PORT_LRN_CTRL, (value & ~VTSS_BIT(port)) | VTSS_BIT(port));        
        }

    }    
    return VTSS_RC_OK;
}

/* Setup function for SGMII/SERDES/FX100/VAUI ports */
/* Chip ports 0-18:  Dev1g   -> 1G operation        */
/* Chip ports 19-26: Dev2G5g -> 1G/2g5 operation    */
static vtss_rc jr_port_conf_1g_set(vtss_state_t *vtss_state,
                                   vtss_port_no_t port_no, u32 port, 
                                   vtss_port_conf_t *conf, BOOL front_port)
{
    u32                giga = 0, fdx, clk, rx_ifg1, rx_ifg2, tx_ifg;
    u32                sgmii = 0, if_100fx = 0, disabled, value, count;
    u32                tgt = VTSS_TO_DEV(port);
    vtss_port_speed_t  speed;
    vtss_serdes_mode_t mode = VTSS_SERDES_MODE_SGMII;
    serdes_t           serdes_type;
    u32                serdes_instance;

    disabled = (conf->power_down ? 1 : 0);
    fdx = conf->fdx;
    speed = conf->speed;

    /* Verify port speed */
    switch (speed) {
    case VTSS_SPEED_10M:
        clk = 0;
        break;
    case VTSS_SPEED_100M:
        clk = 1;
        break;
    case VTSS_SPEED_1G:
        clk = 2;
        giga = 1;
        break;
    case VTSS_SPEED_2500M:
        if (port < 19 || port > 26) {
            VTSS_E("Illegal speed, port %u", port);
            return VTSS_RC_ERROR;
        }
        clk = 2;
        giga = 1;
        mode = VTSS_SERDES_MODE_2G5;
        break;
    default:
        VTSS_E("Illegal speed: %d", speed);
        return VTSS_RC_ERROR;
    }

    /* Verify Interface type */
    switch (conf->if_type) {
    case VTSS_PORT_INTERFACE_NO_CONNECTION:
        disabled = 1;
        break;
    case VTSS_PORT_INTERFACE_SGMII:
        VTSS_D("chip port: %u interface:SGMII", port);
        if (speed != VTSS_SPEED_10M && speed != VTSS_SPEED_100M && speed != VTSS_SPEED_1G) {
            VTSS_E("SGMII, illegal speed, port %u", port);
            return VTSS_RC_ERROR;
        }
        sgmii = 1;
        mode = VTSS_SERDES_MODE_SGMII;
        break;
    case VTSS_PORT_INTERFACE_100FX:
        VTSS_D("chip port: %u interface:100FX", port);
        if (speed != VTSS_SPEED_100M) {
            VTSS_E("100FX, illegal speed, port %u", port);
            return VTSS_RC_ERROR;
        }
        mode = VTSS_SERDES_MODE_100FX;
        if_100fx = 1;
        break;
    case VTSS_PORT_INTERFACE_SERDES:
        VTSS_D("chip port: %u interface:SERDES", port);
        if (speed != VTSS_SPEED_1G) {
            VTSS_E("SERDES, illegal speed, port %u", port);
            return VTSS_RC_ERROR;
        }            
        mode = VTSS_SERDES_MODE_1000BaseX;
        break;
    case VTSS_PORT_INTERFACE_SGMII_CISCO:
        if (speed != VTSS_SPEED_10M && speed != VTSS_SPEED_100M && speed != VTSS_SPEED_1G) {
            VTSS_E("SGMII_CISCO, illegal speed, port %u", port);
            return VTSS_RC_ERROR;
        }
        mode = VTSS_SERDES_MODE_1000BaseX;
        sgmii = 1;        
        break;
    case VTSS_PORT_INTERFACE_VAUI:
        VTSS_D("chip port: %u interface:VAUI", port);
        if (speed != VTSS_SPEED_1G && speed != VTSS_SPEED_2500M) {
            VTSS_E("VAUI, illegal speed, port %u", port);
            return VTSS_RC_ERROR;
        }
        mode = (speed == VTSS_SPEED_2500M ? VTSS_SERDES_MODE_2G5 : VTSS_SERDES_MODE_1000BaseX);
        sgmii = 0;
        break;
    case VTSS_PORT_INTERFACE_LOOPBACK:
        mode = VTSS_SERDES_MODE_1000BaseX;
        break;
    default:
        VTSS_E("Illegal interface type");
        return VTSS_RC_ERROR;
    }  

    /* (re-)configure the Serdes macros to 100FX / 1000BaseX */
    if (mode != VTSS_SERDES_MODE_DISABLE && mode != vtss_state->port.serdes_mode[port_no] &&
        ((mode == VTSS_SERDES_MODE_SGMII) || (mode == VTSS_SERDES_MODE_1000BaseX) || 
         (mode == VTSS_SERDES_MODE_100FX) || (mode == VTSS_SERDES_MODE_2G5))) {

        if ((jr_port2serdes_get(vtss_state, port, &serdes_type, &serdes_instance, &count) == VTSS_RC_OK)) {
            if (serdes_type == SERDES_1G) {
                VTSS_RC(jr_sd1g_read(vtss_state, 1 << serdes_instance));
                VTSS_MSLEEP(10);
                VTSS_RC(jr_sd1g_cfg(vtss_state, mode, 1 << serdes_instance));
                VTSS_MSLEEP(1);        
            } else {
                VTSS_RC(jr_sd6g_read(vtss_state, 1 << serdes_instance));
                VTSS_MSLEEP(10);
                VTSS_RC(jr_sd6g_cfg(vtss_state, mode, 1 << serdes_instance));
                VTSS_MSLEEP(1);        
            }
            vtss_state->port.serdes_mode[port_no] = mode;                
        } else {
            VTSS_E("Could not read serdes type");
            return VTSS_RC_ERROR;
        }
        vtss_state->port.serdes_mode[port_no] = mode;
    }

    /* Disable and flush port queues */
    VTSS_RC(jr_queue_flush(vtss_state, port, 1));

    /* Reset MAC */
    JR_WRX(DEV1G, DEV_CFG_STATUS_DEV_RST_CTRL, tgt,
           VTSS_F_DEV1G_DEV_CFG_STATUS_DEV_RST_CTRL_MAC_TX_RST |
           VTSS_F_DEV1G_DEV_CFG_STATUS_DEV_RST_CTRL_MAC_RX_RST);

    /* Common port setup */
    VTSS_RC(vtss_jr1_port_setup(vtss_state, port_no, port, conf, front_port));

    /* Decide interframe gaps */
    rx_ifg1 = conf->frame_gaps.hdx_gap_1;
    if (rx_ifg1 == VTSS_FRAME_GAP_DEFAULT) {
        rx_ifg1 = (giga ? 5 : (speed == VTSS_SPEED_100M) ? 7 : 8);
    }

    rx_ifg2 = conf->frame_gaps.hdx_gap_2;
    if (rx_ifg2 == VTSS_FRAME_GAP_DEFAULT) {
        rx_ifg2 = (giga ? 1 : 8);
    }

    tx_ifg = conf->frame_gaps.fdx_gap;
    if (tx_ifg == VTSS_FRAME_GAP_DEFAULT) {
        tx_ifg = (giga ? 4 : (fdx == 0) ? 13 : 11);
    }

    /* Set interframe gaps */
    JR_WRX(DEV1G, MAC_CFG_STATUS_MAC_IFG_CFG, tgt,
           VTSS_F_DEV1G_MAC_CFG_STATUS_MAC_IFG_CFG_TX_IFG(tx_ifg) |
           VTSS_F_DEV1G_MAC_CFG_STATUS_MAC_IFG_CFG_RX_IFG1(rx_ifg1) |
           VTSS_F_DEV1G_MAC_CFG_STATUS_MAC_IFG_CFG_RX_IFG2(rx_ifg2));

    /* MAC mode */
    if (fdx && ((speed == VTSS_SPEED_1G) || speed == VTSS_SPEED_2500M)) {
        value = VTSS_F_DEV1G_MAC_CFG_STATUS_MAC_MODE_CFG_FDX_ENA | VTSS_F_DEV1G_MAC_CFG_STATUS_MAC_MODE_CFG_GIGA_MODE_ENA;
    } else if (fdx) {
        value = VTSS_F_DEV1G_MAC_CFG_STATUS_MAC_MODE_CFG_FDX_ENA;
    } else {
        value = 0;
    }
    /* Giga and FDX mode */
    JR_WRX(DEV1G, MAC_CFG_STATUS_MAC_MODE_CFG, tgt, value);

    /* Half duplex */
    JR_WRX(DEV1G, MAC_CFG_STATUS_MAC_HDX_CFG, tgt,
           VTSS_F_DEV1G_MAC_CFG_STATUS_MAC_HDX_CFG_SEED(conf->flow_control.smac.addr[5]) |
           (fdx ? 0 : VTSS_F_DEV1G_MAC_CFG_STATUS_MAC_HDX_CFG_SEED_LOAD) |
           (conf->exc_col_cont ? VTSS_F_DEV1G_MAC_CFG_STATUS_MAC_HDX_CFG_RETRY_AFTER_EXC_COL_ENA : 0) | 
           VTSS_F_DEV1G_MAC_CFG_STATUS_MAC_HDX_CFG_LATE_COL_POS(0x43));

    /* PCS used in SERDES or SGMII mode */
    JR_WRX(DEV1G, PCS1G_CFG_STATUS_PCS1G_MODE_CFG, tgt,
           sgmii ? VTSS_F_DEV1G_PCS1G_CFG_STATUS_PCS1G_MODE_CFG_SGMII_MODE_ENA : 0);

    /* Signal detect */
    if (if_100fx) {  
        JR_WRXM(DEV1G, PCS_FX100_CONFIGURATION_PCS_FX100_CFG, tgt,
                (conf->sd_enable ?       VTSS_F_DEV1G_PCS_FX100_CONFIGURATION_PCS_FX100_CFG_SD_ENA : 0) |
                (conf->sd_internal ? 0 : VTSS_F_DEV1G_PCS_FX100_CONFIGURATION_PCS_FX100_CFG_SD_SEL) |
                (conf->sd_active_high ? VTSS_BIT(25) : 0),
                VTSS_F_DEV1G_PCS_FX100_CONFIGURATION_PCS_FX100_CFG_SD_ENA |
                VTSS_F_DEV1G_PCS_FX100_CONFIGURATION_PCS_FX100_CFG_SD_SEL |
                VTSS_BIT(25)); 
    } else {
        JR_WRX(DEV1G, PCS1G_CFG_STATUS_PCS1G_SD_CFG, tgt,
               (conf->sd_active_high ? VTSS_F_DEV1G_PCS1G_CFG_STATUS_PCS1G_SD_CFG_SD_POL : 0) |
               (conf->sd_internal ? 0 : VTSS_F_DEV1G_PCS1G_CFG_STATUS_PCS1G_SD_CFG_SD_SEL) |
               (conf->sd_enable ? VTSS_F_DEV1G_PCS1G_CFG_STATUS_PCS1G_SD_CFG_SD_ENA : 0));
    }

    /* Enable/Disable 100FX PCS */
    JR_WRXB(DEV1G, PCS_FX100_CONFIGURATION_PCS_FX100_CFG, tgt, PCS_ENA, (if_100fx ? 1 : 0));
    
    /* Enable/disable Serdes/SGMII PCS */
    JR_WRXB(DEV1G, PCS1G_CFG_STATUS_PCS1G_CFG, tgt, PCS_ENA, (if_100fx ? 0 : 1));

    if (conf->if_type == VTSS_PORT_INTERFACE_SGMII) {
        JR_WRX(DEV1G, PCS1G_CFG_STATUS_PCS1G_ANEG_CFG, tgt, 0);
    } else if (conf->if_type == VTSS_PORT_INTERFACE_SGMII_CISCO) {
        /* Complete SGMII aneg */
        value = 0x0001;       

        JR_WRX(DEV1G, PCS1G_CFG_STATUS_PCS1G_ANEG_CFG, tgt,
               VTSS_F_DEV1G_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG_ADV_ABILITY(value) |
               VTSS_F_DEV1G_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG_ANEG_ENA  | 
               VTSS_F_DEV1G_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG_SW_RESOLVE_ENA |
               VTSS_F_DEV1G_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG_ANEG_RESTART_ONE_SHOT);
        
        /* Clear the sticky bits */
        JR_RDX(DEV1G, PCS1G_CFG_STATUS_PCS1G_STICKY, tgt, &value);
        JR_WRX(DEV1G, PCS1G_CFG_STATUS_PCS1G_STICKY, tgt, value);
    }

    /* Set it in loopback mode if required */
    JR_WRXB(DEV1G, PCS1G_CFG_STATUS_PCS1G_LB_CFG, tgt, TBI_HOST_LB_ENA, conf->if_type == VTSS_PORT_INTERFACE_LOOPBACK ? 1 : 0);

    /* Set fill level before DSM is signalled */
    JR_WRXF(DEV1G, DEV_CFG_STATUS_DEV_DBG_CFG, tgt, TX_BUF_HIGH_WM, 11);

    /* Release MAC from reset */
    JR_WRX(DEV1G, DEV_CFG_STATUS_DEV_RST_CTRL, tgt,
           VTSS_F_DEV1G_DEV_CFG_STATUS_DEV_RST_CTRL_SPEED_SEL(disabled ? 3 : clk) |
           (disabled ? VTSS_F_DEV1G_DEV_CFG_STATUS_DEV_RST_CTRL_MAC_TX_RST : 0) |
           (disabled ? VTSS_F_DEV1G_DEV_CFG_STATUS_DEV_RST_CTRL_MAC_RX_RST : 0));

    /* Enable Rx and Tx in MAC */
    JR_WRX(DEV1G, MAC_CFG_STATUS_MAC_ENA_CFG, tgt,
           (disabled ? 0 : VTSS_F_DEV1G_MAC_CFG_STATUS_MAC_ENA_CFG_RX_ENA) |
           (disabled ? 0 : VTSS_F_DEV1G_MAC_CFG_STATUS_MAC_ENA_CFG_TX_ENA));

    /* Enable port queues */
    if (!disabled) {
        VTSS_RC(jr_queue_flush(vtss_state, port, 0));
    }

    VTSS_D("chip port: %u (dev1g),is configured", port);

    return VTSS_RC_OK;
}

/* Setup function for XAUI ports                   */
/* Chip ports 27-30: Dev10G @ 10g/12g operation    */
static vtss_rc jr_port_conf_10g_set(vtss_state_t *vtss_state,
                                    vtss_port_no_t port_no, u32 port, 
                                    vtss_port_conf_t *conf, BOOL front_port)
{

    u32                disabled = (conf->power_down ? 1 : 0);
    u32                tgt = VTSS_TO_DEV(port);
    vtss_port_speed_t  speed = conf->speed;
    BOOL               rxaui = 0;
    BOOL               rx_flip = conf->xaui_rx_lane_flip;
    BOOL               tx_flip = conf->xaui_tx_lane_flip;
    serdes_t           serdes_type;
    u32                serdes_instance, count;
    vtss_serdes_mode_t mode = VTSS_SERDES_MODE_XAUI;
    BOOL               xaui =  (conf->if_type == VTSS_PORT_INTERFACE_XAUI || conf->if_type == VTSS_PORT_INTERFACE_RXAUI) ? 1 : 0;
    
    /* Verify speed */
    switch (speed) {
    case VTSS_SPEED_1G:
    case VTSS_SPEED_2500M:
    case VTSS_SPEED_10G:
    case VTSS_SPEED_12G:
        break;
    default:
        VTSS_E("Illegal speed: %d", speed);
        return VTSS_RC_ERROR;
    }

    switch (conf->if_type) {
    case VTSS_PORT_INTERFACE_SERDES:
        VTSS_D("chip port: %u interface:SERDES", port);
        if (speed != VTSS_SPEED_1G) {
            VTSS_E("SERDES, illegal speed, port %u", port);
            return VTSS_RC_ERROR;
        }            
        mode = VTSS_SERDES_MODE_1000BaseX;
        break;
    case VTSS_PORT_INTERFACE_XAUI:
        VTSS_D("chip port: %u interface:XAUI", port);
        if (speed == VTSS_SPEED_10G) {
            mode = VTSS_SERDES_MODE_XAUI;
        } else {
            mode = VTSS_SERDES_MODE_XAUI_12G;
        }
        break;
    case VTSS_PORT_INTERFACE_RXAUI:
        mode = VTSS_SERDES_MODE_RXAUI;
        rxaui = 1;
        rx_flip = 0;
        tx_flip = 0;
        break;
    case VTSS_PORT_INTERFACE_VAUI:
        VTSS_D("chip port: %u interface:VAUI", port);
        if (speed != VTSS_SPEED_2500M) {
            VTSS_E("VAUI, illegal speed:%d, port %u", speed, port);
            return VTSS_RC_ERROR;
        }
        mode = VTSS_SERDES_MODE_2G5;
        break;
    case VTSS_PORT_INTERFACE_LOOPBACK:
        mode = VTSS_SERDES_MODE_1000BaseX;
        break;
    default:
        VTSS_E("Illegal interface type");
        return VTSS_RC_ERROR;
    }  

    /* (re-)configure the Serdes macros to 10Gb / 12Gb */
    if (mode != VTSS_SERDES_MODE_DISABLE && (port_no == VTSS_PORT_NO_NONE || mode != vtss_state->port.serdes_mode[port_no])) {
        if ((jr_port2serdes_get(vtss_state, port, &serdes_type, &serdes_instance, &count) == VTSS_RC_OK) && (serdes_type == SERDES_6G)) {
            VTSS_RC(jr_sd6g_read(vtss_state, 1 << serdes_instance));
            VTSS_MSLEEP(10);
            VTSS_RC(jr_sd6g_cfg(vtss_state, mode, 0xf << serdes_instance));
            VTSS_MSLEEP(1);        
        } else {
            VTSS_E("Illegal serdes type chipport:%d",port);
        }
        if (port_no != VTSS_PORT_NO_NONE)
            vtss_state->port.serdes_mode[port_no] = mode;
    }

    /* Disable and flush port queues */
    VTSS_RC(jr_queue_flush(vtss_state, port, 1));

    JR_WRX(DEV10G, MAC_CFG_STATUS_MAC_ENA_CFG, tgt, 0);

    /* Reset MAC */
    JR_WRX(DEV10G, DEV_CFG_STATUS_DEV_RST_CTRL, tgt,
           VTSS_F_DEV10G_DEV_CFG_STATUS_DEV_RST_CTRL_MAC_TX_RST |
           VTSS_F_DEV10G_DEV_CFG_STATUS_DEV_RST_CTRL_MAC_RX_RST |
           VTSS_F_DEV10G_DEV_CFG_STATUS_DEV_RST_CTRL_PCS_TX_RST |
           VTSS_F_DEV10G_DEV_CFG_STATUS_DEV_RST_CTRL_PCS_RX_RST);

    if (xaui) {
        /* Disable 1G PCS */
        JR_WRXB(DEV10G, PCS1G_CFG_STATUS_PCS1G_CFG, tgt, PCS_ENA, 0);
        /* Enable XAUI PCS */
        JR_WRXB(DEV10G, PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG, tgt, PCS_ENA, 1);
    } else {
        /* Disable XAUI PCS */
        JR_WRXB(DEV10G, PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG, tgt, PCS_ENA, 0);
        /* Enable 1G PCS */
        JR_WRXB(DEV10G, PCS1G_CFG_STATUS_PCS1G_CFG, tgt, PCS_ENA, 1);
        JR_WRXB(DEV10G, PCS1G_CFG_STATUS_PCS1G_MODE_CFG, tgt, SGMII_MODE_ENA, 0);
        /* Signal detect */
        JR_WRX(DEV10G, PCS1G_CFG_STATUS_PCS1G_SD_CFG, tgt,
               (conf->sd_active_high ? VTSS_F_DEV10G_PCS1G_CFG_STATUS_PCS1G_SD_CFG_SD_POL : 0) |
               (conf->sd_internal ? 0 : VTSS_F_DEV10G_PCS1G_CFG_STATUS_PCS1G_SD_CFG_SD_SEL) |
               (conf->sd_enable ? VTSS_F_DEV10G_PCS1G_CFG_STATUS_PCS1G_SD_CFG_SD_ENA : 0));
    }

    /* Enable XAUI PCS 2x6G */
    //JR_WRXB(DEV10G, PCS2X6G_CONFIGURATION_PCS2X6G_CFG, tgt, PCS_ENA, 1);

    /* Common port setup */
    VTSS_RC(vtss_jr1_port_setup(vtss_state, port_no, port, conf, front_port));

    /* Link fault signalling used for line ports */
    JR_WRXB(DEV10G, MAC_CFG_STATUS_MAC_LFS_CFG, tgt, LFS_MODE_ENA, (xaui?1:0));

    /* Write Txbuf low and high  */
    VTSS_RC(vtss_jr1_wr(vtss_state, VTSS_IOREG(tgt,0x64), 9 | 2<<5)); 

    /* Enable Rx and Tx in MAC */
    JR_WRX(DEV10G, MAC_CFG_STATUS_MAC_ENA_CFG, tgt,
           (disabled ? 0 : VTSS_F_DEV10G_MAC_CFG_STATUS_MAC_ENA_CFG_RX_ENA) |
           (disabled ? 0 : VTSS_F_DEV10G_MAC_CFG_STATUS_MAC_ENA_CFG_TX_ENA));

    if (xaui) {
        /* Setup four or two lane XAUI */
        JR_WRXB(DEV10G, PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG, 
                tgt, ILV_MODE_ENA, rxaui);
        
        /* XAUI Lane flipping  */
        JR_WRX(DEV10G, PCS_XAUI_CONFIGURATION_PCS_XAUI_EXT_CFG, tgt,
               (rx_flip ? VTSS_F_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_EXT_CFG_RX_FLIP_HMBUS : 0) |
               (tx_flip ? VTSS_F_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_EXT_CFG_TX_FLIP_HMBUS : 0));
    }

    /* Set it in loopback mode if required */
    JR_WRXB(DEV10G, PCS1G_CFG_STATUS_PCS1G_LB_CFG, tgt, TBI_HOST_LB_ENA, conf->if_type == VTSS_PORT_INTERFACE_LOOPBACK ? 1 : 0);
    
    /* Release MAC from reset */
    JR_WRX(DEV10G, DEV_CFG_STATUS_DEV_RST_CTRL, tgt,
           VTSS_F_DEV10G_DEV_CFG_STATUS_DEV_RST_CTRL_SPEED_SEL(disabled ? 6 : xaui ? 4 : (speed == VTSS_SPEED_2500M ? 3 : 2)) |
           (disabled ? VTSS_F_DEV10G_DEV_CFG_STATUS_DEV_RST_CTRL_PCS_TX_RST : 0) |
           (disabled ? VTSS_F_DEV10G_DEV_CFG_STATUS_DEV_RST_CTRL_PCS_RX_RST : 0) |
           (disabled ? VTSS_F_DEV10G_DEV_CFG_STATUS_DEV_RST_CTRL_MAC_TX_RST : 0) |
           (disabled ? VTSS_F_DEV10G_DEV_CFG_STATUS_DEV_RST_CTRL_MAC_RX_RST : 0));
    
    /* Clear the Stickies */
    JR_WRX(DEV10G, PCS1G_CFG_STATUS_PCS1G_STICKY, tgt, 0xFFFF);

    /* Enable port queues */
    VTSS_RC(jr_queue_flush(vtss_state, port, 0));

    VTSS_D("chip port: %u (10G),is configured", port);

    return VTSS_RC_OK;
}


static vtss_rc jr_port_conf_set(vtss_state_t *vtss_state, const vtss_port_no_t port_no)
{
    vtss_port_conf_t *conf = &vtss_state->port.conf[port_no];
    u32              port = VTSS_CHIP_PORT(port_no);
    BOOL             front_port = 0;
    
#if defined(VTSS_FEATURE_VSTAX_V2)
    front_port = vtss_jr1_is_frontport(vtss_state, port_no);
#endif /* VTSS_FEATURE_VSTAX_V2 */

    /* Update maximum tags allowed */
    VTSS_RC(vtss_jr1_port_max_tags_set(vtss_state, port_no));

    if (VTSS_PORT_IS_10G(port)) {
        return jr_port_conf_10g_set(vtss_state, port_no, port, conf, front_port);
    } else {
        return jr_port_conf_1g_set(vtss_state, port_no, port, conf, front_port);
    }
}

#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC)   

#define JR_CNT_HOST_SQS(tgt, xmit, cnt_id, qu, cnt, clr)                               \
{                                                                                      \
    u32 value;                                                                         \
    JR_RDXY(tgt, STAT_CNT_CFG_##xmit##_STAT_LSB_CNT, qu, JR_CNT(xmit, cnt_id), &value); \
    vtss_cmn_counter_32_update(value, cnt, clr);                                       \
}

static BOOL lport_is_valid(vtss_state_t *vtss_state, vtss_lport_no_t lport_no)
{
    BOOL is_valid = FALSE;

    switch(vtss_state->init_conf.host_mode) {
#if defined(VTSS_CHIP_CE_MAX_24)
        case VTSS_HOST_MODE_0:
            is_valid = (lport_no > 23) ? FALSE : TRUE;
            break;
        case VTSS_HOST_MODE_1:
            is_valid = (lport_no > 47) ? FALSE : TRUE;
            break;
#endif
        case VTSS_HOST_MODE_2:
            is_valid = (lport_no > 23) ? FALSE : TRUE;
            break;
        case VTSS_HOST_MODE_3:
            is_valid = (lport_no > 47) ? FALSE : TRUE;
            break;
        default:
            break;
    }
    return is_valid;
}

static vtss_rc jr_init_cemax_oobfc_conf_set(vtss_state_t *vtss_state,
                                            const u8 host_instance,  const BOOL fc_enable)
{
    u32 i, host_mode;
    u32 tmp_port, tmp_prio, tmp_instance, total_queues, total_instaces, total_ports, max_queues;
    u32 total_line_queues, total_host_queues;
    u8  inc_val;
    u32 enable_mask, tmp_mask;


    host_mode = vtss_jr1_ce_max_hm(vtss_state);

    switch(host_mode) {
#if defined(VTSS_CHIP_CE_MAX_24)
        case VTSS_HOST_MODE_0:
            total_instaces = 1;
            total_ports = 12; /* Total ports per instance */
            total_queues = total_line_queues = total_host_queues = 8;
            inc_val = 1;
            break;
        case VTSS_HOST_MODE_1:
            total_instaces = 1;
            total_line_queues = 8;
            total_queues = total_host_queues = 4;
            total_ports = 12;
            inc_val = 2;
            break;
#endif
        case VTSS_HOST_MODE_2:
            total_instaces = 1;
            total_queues = total_line_queues = total_host_queues = 8;
            total_ports = 24;
            inc_val = 1;
#if defined(VTSS_CHIP_CE_MAX_12)
            total_ports = 12;
#endif
            break;
        case VTSS_HOST_MODE_3:
            total_instaces = 1;
            total_line_queues = 8;
            total_queues = total_host_queues = 4;
            total_ports = 24;
            inc_val = 2;
#if defined(VTSS_CHIP_CE_MAX_12)
            total_ports = 12;
#endif
            break;
        default :
            total_instaces = 0;
            total_ports = 12; /* Total ports per instance */
            total_queues = total_line_queues = total_host_queues = 8;
            inc_val = 1;
            break;
    }

    if (total_instaces == 0) {
        return VTSS_RC_ERROR;
    }

    if (host_instance > 0 && ((host_mode == VTSS_HOST_MODE_2 || host_mode == VTSS_HOST_MODE_3))) {
        return VTSS_RC_ERROR;
    }

    if (fc_enable == TRUE) {
        if (host_instance == 0) {
            if (host_mode != VTSS_HOST_MODE_2 && host_mode != VTSS_HOST_MODE_3) {
                enable_mask = 0x00000fff;
            } else {
                enable_mask = 0xffffffff;
            }
        } else {
            enable_mask = 0x00fff000;
        }
        /* Enable Status Capture on port-level for line ports */
        JR_RD(DSM, CM_CFG_CMM_MASK_CFG, &tmp_mask);
        tmp_mask |= enable_mask;
        JR_WR(DSM, CM_CFG_CMM_MASK_CFG, tmp_mask);

        /* Enable Status Capture on Queue-level for line port queues */
        for (i = 0; i < 8; ) {
            JR_RDX(DSM, CM_CFG_CMM_QU_MASK_CFG, i, &tmp_mask);
            tmp_mask |= enable_mask;
            JR_WRX(DSM, CM_CFG_CMM_QU_MASK_CFG, i, tmp_mask);
            i++;
        }
        //For the status channel registers configure which UPSID, they shall capture
        // status for.
        {
            u8 upsid;
            u8 total_upsid;
            u8 tmp;

            if (host_instance == 0) {
                upsid = 8;
                total_upsid = 8;
                if (host_mode != VTSS_HOST_MODE_2 && host_mode != VTSS_HOST_MODE_3) {
                    total_upsid = 4;
                }
            } else {
                upsid = 12;
                total_upsid = 4;
            }
            for (tmp = 0; tmp <= total_upsid; tmp++) {
                JR_WRX(DSM, HOST_IF_IBS_COMMON_CFG_CAPTURE_UPSID_CFG, tmp, upsid);
                upsid++;
            }
            JR_WRX(DSM, HOST_IF_IBS_COMMON_CFG_CAPTURE_UPSID_CFG, 8, 0);
        }

        for (tmp_instance = 0; tmp_instance < total_instaces; tmp_instance++) {
            // Select clock and release reset write
            JR_RDX(DSM, HOST_IF_IBS_CFG_XAUI_IBS_CFG, host_instance, &i);
            i &= (~VTSS_M_DSM_HOST_IF_IBS_CFG_XAUI_IBS_CFG_CONF_IBS_CLK_SEL);
            i |= VTSS_F_DSM_HOST_IF_IBS_CFG_XAUI_IBS_CFG_CONF_IBS_CLK_SEL(0);
            JR_WRX(DSM, HOST_IF_IBS_CFG_XAUI_IBS_CFG, host_instance, i);
            JR_RDX(DSM, HOST_IF_IBS_CFG_XAUI_IBS_CFG, host_instance, &i);
            i &= (~VTSS_F_DSM_HOST_IF_IBS_CFG_XAUI_IBS_CFG_CONF_IBS_RST);
            JR_WRX(DSM, HOST_IF_IBS_CFG_XAUI_IBS_CFG, host_instance, i);

            /* == Enable the Calendar entries for first 12 ports and queues == */
            tmp_port = host_instance * total_ports;
#if defined(VTSS_CHIP_CE_MAX_12)
            tmp_port = 11;
#endif
            for (i = 0; i < total_ports; i++) {
                JR_WRXY(DSM, HOST_IF_CONF_IBS_CAL_XAUI_IBS_CAL, i, host_instance, (32*total_queues)+tmp_port);
                tmp_port++;
            }
            tmp_port = host_instance * total_ports;
#if defined(VTSS_CHIP_CE_MAX_12)
            tmp_port = 11;
#endif

            tmp_prio = 0;
            max_queues = (total_ports + total_ports*total_queues);
            for (i = total_ports; i < max_queues; i++) {
                JR_WRXY(DSM, HOST_IF_CONF_IBS_CAL_XAUI_IBS_CAL, i, host_instance, tmp_port+(32*tmp_prio));
                tmp_prio = (tmp_prio+inc_val)%total_line_queues;
                if (tmp_prio == 0) {
                    tmp_port++;
                }
            }

            JR_RDX(DSM, HOST_IF_IBS_CFG_XAUI_IBS_CFG, host_instance, &i);

            /* Set Calendar length */
            i &= (~VTSS_M_DSM_HOST_IF_IBS_CFG_XAUI_IBS_CFG_CONF_IBS_CAL_LEN);
            i |= VTSS_F_DSM_HOST_IF_IBS_CFG_XAUI_IBS_CFG_CONF_IBS_CAL_LEN(total_ports*(1+total_queues));
            i |= VTSS_F_DSM_HOST_IF_IBS_CFG_XAUI_IBS_CFG_CONF_IBS_CAL_M(1);
            /* Start Status Channel */
            i &= (~VTSS_F_DSM_HOST_IF_IBS_CFG_XAUI_IBS_CFG_CONF_IBS_FORCE_IDLE);
            JR_WRX(DSM, HOST_IF_IBS_CFG_XAUI_IBS_CFG, host_instance, i);

            /* == Configuration settings to receive the filling status of line port OQS == */
            JR_RDX(DSM, HOST_IF_OBS_CFG_XAUI_OBS_CFG, host_instance, &i);
            i &= ~(VTSS_F_DSM_HOST_IF_OBS_CFG_XAUI_OBS_CFG_CONF_OBS_RST);
            JR_WRX(DSM, HOST_IF_OBS_CFG_XAUI_OBS_CFG, host_instance, i);

            tmp_port = host_instance * total_ports;
#if defined(VTSS_CHIP_CE_MAX_12)
            tmp_port = 11;
#endif
            /* Setup OBS flow control Calendar */
            for (i = 0; i < total_ports; i++) {
                JR_WRXY(DSM, HOST_IF_CONF_OBS_CAL_XAUI_OBS_CAL, i, host_instance, 192+tmp_port);
                tmp_port++;
            }
            tmp_prio = 0;
            tmp_port = host_instance * total_ports;
#if defined(VTSS_CHIP_CE_MAX_12)
            tmp_port = 0; /* Host queue starts from 0 again */
#endif
            for (i = total_ports; i < total_ports*(1+total_host_queues); i++) {
                JR_WRXY(DSM, HOST_IF_CONF_OBS_CAL_XAUI_OBS_CAL, i, host_instance, tmp_port*total_host_queues+tmp_prio);
                tmp_prio = (tmp_prio+1)%total_host_queues;
                if (tmp_prio == 0) {
                    tmp_port++;
                }
            }
            JR_RDX(DSM, HOST_IF_OBS_CFG_XAUI_OBS_CFG, host_instance, &i);
            /* Set Calendar length */
            i |= VTSS_F_DSM_HOST_IF_OBS_CFG_XAUI_OBS_CFG_CONF_OBS_CAL_LEN(total_ports*(1+total_host_queues));
            i |= VTSS_F_DSM_HOST_IF_OBS_CFG_XAUI_OBS_CFG_CONF_OBS_CAL_M(1);
            JR_WRX(DSM, HOST_IF_OBS_CFG_XAUI_OBS_CFG, host_instance, i);
        }
    } else {

        if (host_instance == 0) {
            enable_mask = 0x00000000;
            if (host_mode != VTSS_HOST_MODE_2 && host_mode != VTSS_HOST_MODE_3) {
                enable_mask = 0x00fff000;
            }
        } else {
            enable_mask = 0x00000fff;
        }

        /* Enable Status Capture on port-level for all line ports */
        JR_RD(DSM, CM_CFG_CMM_MASK_CFG, &tmp_mask);
        tmp_mask &= enable_mask;
        JR_WR(DSM, CM_CFG_CMM_MASK_CFG, tmp_mask);

        /* Enable Status Capture on Queue-level for line port queues */
        for (i = 0; i < 8; ) {
            JR_RDX(DSM, CM_CFG_CMM_QU_MASK_CFG, i, &tmp_mask);
            tmp_mask &= enable_mask;
            JR_WRX(DSM, CM_CFG_CMM_QU_MASK_CFG, i, tmp_mask);
            i += inc_val;
        }

        //For the status channel registers configure which UPSID, they shall capture
        // status for.
        {
            u8 total_upsid;
            u8 tmp;

            if (host_instance == 0) {
                total_upsid = 8;
                if (host_mode != VTSS_HOST_MODE_2 && host_mode != VTSS_HOST_MODE_3) {
                    total_upsid = 4;
                }
            } else {
                total_upsid = 4;
            }
            for (tmp = 0; tmp <= total_upsid; tmp++) {
                JR_WRX(DSM, HOST_IF_IBS_COMMON_CFG_CAPTURE_UPSID_CFG, tmp, 0);
            }
            JR_WRX(DSM, HOST_IF_IBS_COMMON_CFG_CAPTURE_UPSID_CFG, 8, 0);
        }

        for (tmp_instance = 0; tmp_instance < total_instaces; tmp_instance++) {
            // Select clock and release reset write
            JR_RDX(DSM, HOST_IF_IBS_CFG_XAUI_IBS_CFG, host_instance, &i);
            i &= (~VTSS_M_DSM_HOST_IF_IBS_CFG_XAUI_IBS_CFG_CONF_IBS_CLK_SEL);
            i |= VTSS_F_DSM_HOST_IF_IBS_CFG_XAUI_IBS_CFG_CONF_IBS_CLK_SEL(3);
            JR_WRX(DSM, HOST_IF_IBS_CFG_XAUI_IBS_CFG, host_instance, i);
            JR_RDX(DSM, HOST_IF_IBS_CFG_XAUI_IBS_CFG, host_instance, &i);
            i |= (VTSS_F_DSM_HOST_IF_IBS_CFG_XAUI_IBS_CFG_CONF_IBS_RST);
            JR_WRX(DSM, HOST_IF_IBS_CFG_XAUI_IBS_CFG, host_instance, i);

            /* == Enable the Calendar entries for first 12 ports and queues == */
            tmp_port = host_instance * total_ports;
#if defined(VTSS_CHIP_CE_MAX_12)
            tmp_port = 11;
#endif
            for (i = 0; i < total_ports; i++) {
                JR_WRXY(DSM, HOST_IF_CONF_IBS_CAL_XAUI_IBS_CAL, i, host_instance, 0);
                tmp_port++;
            }
            tmp_port = host_instance * total_ports;
#if defined(VTSS_CHIP_CE_MAX_12)
            tmp_port = 11;
#endif
            tmp_prio = 0;
            max_queues = (total_ports + total_ports*total_queues);
            for (i = total_ports; i < max_queues; i++) {
                JR_WRXY(DSM, HOST_IF_CONF_IBS_CAL_XAUI_IBS_CAL, i, host_instance, 0);
                tmp_prio = (tmp_prio+inc_val)%total_line_queues;
                if (tmp_prio == 0) {
                    tmp_port++;
                }
            }

            JR_RDX(DSM, HOST_IF_IBS_CFG_XAUI_IBS_CFG, host_instance, &i);

            /* Set Calendar length */
            i &= (~VTSS_M_DSM_HOST_IF_IBS_CFG_XAUI_IBS_CFG_CONF_IBS_CAL_LEN);
            i |= VTSS_F_DSM_HOST_IF_IBS_CFG_XAUI_IBS_CFG_CONF_IBS_CAL_LEN(0);
            i |= VTSS_F_DSM_HOST_IF_IBS_CFG_XAUI_IBS_CFG_CONF_IBS_CAL_M(0);
            /* Start Status Channel */
            i |= (VTSS_F_DSM_HOST_IF_IBS_CFG_XAUI_IBS_CFG_CONF_IBS_FORCE_IDLE);
            JR_WRX(DSM, HOST_IF_IBS_CFG_XAUI_IBS_CFG, host_instance, i);

            /* == Configuration settings to receive the filling status of line port OQS == */
            JR_RDX(DSM, HOST_IF_OBS_CFG_XAUI_OBS_CFG, host_instance, &i);
            i |= (VTSS_F_DSM_HOST_IF_OBS_CFG_XAUI_OBS_CFG_CONF_OBS_RST);
            JR_WRX(DSM, HOST_IF_OBS_CFG_XAUI_OBS_CFG, host_instance, i);

            tmp_port = 0;
            /* Setup flow control Calendar */
            for (tmp_port = 0; tmp_port < total_ports ; tmp_port++) {
                JR_WRXY(DSM, HOST_IF_CONF_OBS_CAL_XAUI_OBS_CAL, tmp_port, host_instance, 0);
            }
            tmp_prio = 0;
            tmp_port = 0;
            for (i = total_ports; i < total_ports*(1+total_host_queues); i++) {
                JR_WRXY(DSM, HOST_IF_CONF_OBS_CAL_XAUI_OBS_CAL, i, host_instance, 0);
                tmp_prio = (tmp_prio+1)%total_host_queues;
                if (tmp_prio == 0) {
                    tmp_port++;
                }
            }
            JR_RDX(DSM, HOST_IF_OBS_CFG_XAUI_OBS_CFG, host_instance, &i);
            /* Set Calendar length */
            i &= (~VTSS_M_DSM_HOST_IF_OBS_CFG_XAUI_OBS_CFG_CONF_OBS_CAL_LEN);
            i |= VTSS_F_DSM_HOST_IF_OBS_CFG_XAUI_OBS_CFG_CONF_OBS_CAL_LEN(0);
            i |= VTSS_F_DSM_HOST_IF_OBS_CFG_XAUI_OBS_CFG_CONF_OBS_CAL_M(0);
            JR_WRX(DSM, HOST_IF_OBS_CFG_XAUI_OBS_CFG, host_instance, i);
        }
    }
    return VTSS_RC_OK;
}

static vtss_rc jr_xaui_conf_set(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
    vtss_xaui_conf_t  *xaui;
    u32               hih_ena, format, chip_port = vtss_state->port.map[port_no].chip_port;
    u32               tmp_val, host_mode, total_ports, tmp_port;
    vtss_rc           rc = VTSS_RC_OK;

    xaui = &vtss_state->port.xaui[chip_port-27];
    hih_ena = (xaui->hih.format > 1) ? 1 : 0;  
    host_mode = vtss_jr1_ce_max_hm(vtss_state);

    switch(xaui->hih.format) {
        case VTSS_HIH_PRE_STANDARD:
            format = 0;
            break;
        case VTSS_HIH_PRE_SHORT:
            format = 1;
            break;
        case VTSS_HIH_PRE_SFD:
            format = 2;
            break;
        case VTSS_HIH_POST_SFD:
            format = 3;
            break;
        case VTSS_HIH_IN_CRC:
            format = 4;
            break;
        default:
            return VTSS_RC_ERROR;
    }
    /* HIH position in Preamble */
    JR_WRXF(DEV10G, MAC_CFG_STATUS_MAC_MODE_CFG, VTSS_TO_DEV(chip_port), MAC_PREAMBLE_CFG, format);
    JR_WRXB(DEV10G, MAC_CFG_STATUS_MAC_MODE_CFG, VTSS_TO_DEV(chip_port), HIH_CRC_CHECK, xaui->hih.cksum_enable);   
    /* Move received HIH to IFH */
    JR_WRXB(ASM, CFG_ETH_CFG, chip_port, ETH_PRE_MODE, hih_ena);
    /* Enable / Disable HIH insertion */
    JR_WRXB(REW, COMMON_HIH_CTRL, (chip_port-27), HIH_ENA, hih_ena);
    JR_WRXB(DSM, HOST_IF_CFG_HIH_CFG, (chip_port-27), HIH_ENA, hih_ena);
    /* GAP compenstations is 12 if HIH is enabled, otherwise 20  */
    JR_WRXF(DSM, CFG_RATE_CTRL, chip_port, FRM_GAP_COMP, hih_ena ? 12 : 20);            

    if (chip_port == vtss_state->port.ce_mac_hmda || chip_port == vtss_state->port.ce_mac_hmdb) {
        /* Enable/Disable Status channel */
        rc = jr_init_cemax_oobfc_conf_set(vtss_state, chip_port - vtss_state->port.ce_mac_hmda,
                                          xaui->fc.channel_enable);
        if (rc != VTSS_RC_OK) {
            return rc;
        }
        /* Enable/Disable Extended reach flow control on line ports associated with this host interafce */
        switch (host_mode) {
#if defined(VTSS_CHIP_CE_MAX_24)
            case VTSS_HOST_MODE_0:
                tmp_port = 0;
                if (chip_port != vtss_state->port.ce_mac_hmda) {
                    tmp_port = 12;
                }
                total_ports = 12;
                break;
            case VTSS_HOST_MODE_1:
                tmp_port = 0;
                if (chip_port != vtss_state->port.ce_mac_hmda) {
                    tmp_port = 12;
                }
                total_ports = 12;
                break;
#endif /* VTSS_CHIP_CE_MAX_24 */
            case VTSS_HOST_MODE_2:
                tmp_port = 0;
                total_ports = 24;
                break;
            case VTSS_HOST_MODE_3:
                tmp_port = 0;
                total_ports = 24;
                break;
            default:
                return VTSS_RC_ERROR;
        }
        total_ports += tmp_port;
        /* Enable extended reach on Ethernet ports to send-out pause frames */
        for (; tmp_port < total_ports; tmp_port++) {
            JR_RDX(DSM, CFG_ETH_FC_GEN, tmp_port, &tmp_val);
            tmp_val &= (~VTSS_M_DSM_CFG_ETH_FC_GEN_XAUI_EXT_REACH_FC_GEN);
            tmp_val |= VTSS_F_DSM_CFG_ETH_FC_GEN_XAUI_EXT_REACH_FC_GEN(xaui->fc.extended_reach);
            JR_WRX(DSM, CFG_ETH_FC_GEN, tmp_port, tmp_val);
        }
    }

    return VTSS_RC_OK;
}

static vtss_rc jr_lport_counters_get(vtss_state_t *vtss_state, 
                                     const vtss_lport_no_t lport_no,
                                     vtss_lport_counters_t *const counters)
{
    u8  i, prios;
    u32 qu;
    u32 hm = vtss_jr1_ce_max_hm(vtss_state);
    vtss_lport_chip_counters_t *c = &vtss_state->port.lport_chip_counters[lport_no];

    if (hm == 0 || hm == 2) {
        prios = 8;
    } else {
        prios = 4;
    }
    if (lport_is_valid(vtss_state, lport_no) == FALSE) {
        return VTSS_RC_ERROR;
    }
    if (lport_no >= VTSS_LPORTS) {
        return VTSS_RC_ERROR;
    }
    qu = 256 + (lport_no) * prios;
    for (i = 0; i < prios; i++) {
        /* Update the chip counters */
        JR_CNT_HOST_SQS(OQS, TX, GREEN, qu,  &c->tx_green[i], 0);
        JR_CNT_HOST_SQS(OQS, TX, YELLOW, qu, &c->tx_yellow[i], 0);
        JR_CNT_HOST_SQS(OQS, RX, QDROPS, qu, &c->rx_drops[i], 0);
        JR_CNT_HOST_SQS(OQS, RX, GREEN, qu, &c->rx_green[i], 0);
        JR_CNT_HOST_SQS(OQS, RX, YELLOW, qu, &c->rx_yellow[i], 0);

        /* Tx Counters */
        counters->tx_green[i] = c->tx_green[i].value;
        counters->tx_yellow[i] = c->tx_yellow[i].value;

        /* Rx Counters */
        counters->rx_green[i] = c->rx_green[i].value;
        counters->rx_yellow[i] = c->rx_yellow[i].value;
        counters->rx_drops[i] = c->rx_drops[i].value;
        qu++;
    }

    return VTSS_RC_OK;
}

static vtss_rc jr_lport_counters_clear(vtss_state_t *vtss_state, const vtss_lport_no_t lport_no)
{
    u8  i, prios;
    u32 qu;
    u32 hm = vtss_jr1_ce_max_hm(vtss_state);
    vtss_lport_chip_counters_t *c = &vtss_state->port.lport_chip_counters[lport_no];

    if (hm == 0 || hm == 2) {
        prios = 8;
    } else {
        prios = 4;
    }

    if (lport_is_valid(vtss_state, lport_no) == FALSE) {
        return VTSS_RC_ERROR;
    }
    if (lport_no >= VTSS_LPORTS) {
        return VTSS_RC_ERROR;
    }
    qu = 256 + (lport_no) * prios;
    for (i = 0; i < prios; i++) {
        /* Update the chip counters */
        JR_CNT_HOST_SQS(OQS, TX, GREEN, qu,  &c->tx_green[i], 1);
        JR_CNT_HOST_SQS(OQS, TX, YELLOW, qu, &c->tx_yellow[i], 1);
        JR_CNT_HOST_SQS(OQS, RX, QDROPS, qu, &c->rx_drops[i], 1);
        JR_CNT_HOST_SQS(OQS, RX, GREEN, qu, &c->rx_green[i], 1);
        JR_CNT_HOST_SQS(OQS, RX, YELLOW, qu, &c->rx_yellow[i], 1);

        qu++;
    }

    return VTSS_RC_OK;
}

static BOOL jr_port_is_host(vtss_state_t *vtss_state, const vtss_port_no_t port_no)
{
    return (VTSS_CHIP_PORT(port_no) == vtss_state->port.ce_mac_hmda ||
            VTSS_CHIP_PORT(port_no) == vtss_state->port.ce_mac_hmdb);
}


#endif /* VTSS_ARCH_JAGUAR_1_CE_MAC */

/* Set 1000Base-X Fiber Auto-negotiation (Clause 37) */
static vtss_rc jr_port_clause_37_control_set(vtss_state_t *vtss_state,
                                             const vtss_port_no_t port_no)
{
    vtss_port_clause_37_control_t *control = &vtss_state->port.clause_37[port_no];
    u32 value, port = VTSS_CHIP_PORT(port_no);

    /* Aneg capabilities for this port */
    VTSS_RC(vtss_cmn_port_clause_37_adv_set(&value, &control->advertisement, control->enable));

    if (VTSS_PORT_IS_1G(port)) {
        /* Set aneg capabilities, enable neg and restart */
        JR_WRX(DEV1G, PCS1G_CFG_STATUS_PCS1G_ANEG_CFG, VTSS_TO_DEV(port), 
               JR_PUT_FLD(DEV1G, PCS1G_CFG_STATUS_PCS1G_ANEG_CFG, ADV_ABILITY, value) |
               JR_PUT_BIT(DEV1G, PCS1G_CFG_STATUS_PCS1G_ANEG_CFG, ANEG_ENA, 1) | 
               JR_PUT_BIT(DEV1G, PCS1G_CFG_STATUS_PCS1G_ANEG_CFG, ANEG_RESTART_ONE_SHOT, 1));

        if (!control->enable) {
            /* Disable Aneg */
            JR_WRX(DEV1G, PCS1G_CFG_STATUS_PCS1G_ANEG_CFG, VTSS_TO_DEV(port), 
                   JR_PUT_FLD(DEV1G, PCS1G_CFG_STATUS_PCS1G_ANEG_CFG, ADV_ABILITY, 0) |
                   JR_PUT_BIT(DEV1G, PCS1G_CFG_STATUS_PCS1G_ANEG_CFG, ANEG_ENA, 0) | 
                   JR_PUT_BIT(DEV1G, PCS1G_CFG_STATUS_PCS1G_ANEG_CFG, ANEG_RESTART_ONE_SHOT, 1));
         }
 
    } else {
        JR_WRX(DEV10G, PCS1G_CFG_STATUS_PCS1G_ANEG_CFG, VTSS_TO_DEV(port), 
               JR_PUT_FLD(DEV10G, PCS1G_CFG_STATUS_PCS1G_ANEG_CFG, ADV_ABILITY, value) |
               JR_PUT_BIT(DEV10G, PCS1G_CFG_STATUS_PCS1G_ANEG_CFG, ANEG_ENA, 1) | 
               JR_PUT_BIT(DEV10G, PCS1G_CFG_STATUS_PCS1G_ANEG_CFG, ANEG_RESTART_ONE_SHOT, 1)); 

        if (!control->enable) {
            JR_WRX(DEV10G, PCS1G_CFG_STATUS_PCS1G_ANEG_CFG, VTSS_TO_DEV(port), 
                   JR_PUT_FLD(DEV10G, PCS1G_CFG_STATUS_PCS1G_ANEG_CFG, ADV_ABILITY, 0) |
                   JR_PUT_BIT(DEV10G, PCS1G_CFG_STATUS_PCS1G_ANEG_CFG, ANEG_ENA, 0) | 
                   JR_PUT_BIT(DEV10G, PCS1G_CFG_STATUS_PCS1G_ANEG_CFG, ANEG_RESTART_ONE_SHOT, 1)); 
        }
    }
    return VTSS_RC_OK;
}

/* Get 1000Base-X Fiber Auto-negotiation status (Clause 37) */
static vtss_rc jr_port_clause_37_status_get(vtss_state_t *vtss_state,
                                            const vtss_port_no_t         port_no,
                                            vtss_port_clause_37_status_t *const status)
    
{

    u32 value, port = VTSS_CHIP_PORT(port_no);

    if (vtss_state->port.conf[port_no].power_down) {
        status->link = 0;
        return VTSS_RC_OK;
    }

    if (VTSS_PORT_IS_1G(port)) {
        /* Get the link state 'down' sticky bit  */
        JR_RDX(DEV1G, PCS1G_CFG_STATUS_PCS1G_STICKY, VTSS_TO_DEV(port), &value);
        value = VTSS_F_DEV1G_PCS1G_CFG_STATUS_PCS1G_STICKY_LINK_DOWN_STICKY & value; 

        if (VTSS_BOOL(value)) {
            /* The link has been down. Clear the sticky bit and return the 'down' value  */
            JR_WRXB(DEV1G, PCS1G_CFG_STATUS_PCS1G_STICKY, VTSS_TO_DEV(port), LINK_DOWN_STICKY, 1);
            JR_WRXB(DEV1G, PCS1G_CFG_STATUS_PCS1G_STICKY, VTSS_TO_DEV(port), OUT_OF_SYNC_STICKY, 1);
            status->link = 0;
        } else {
            /*  Return the current status     */
            JR_RDX(DEV1G, PCS1G_CFG_STATUS_PCS1G_LINK_STATUS, VTSS_TO_DEV(port), &value);
            status->link = VTSS_BOOL(VTSS_F_DEV1G_PCS1G_CFG_STATUS_PCS1G_LINK_STATUS_LINK_STATUS & value) && 
                           VTSS_BOOL(VTSS_F_DEV1G_PCS1G_CFG_STATUS_PCS1G_LINK_STATUS_SYNC_STATUS & value);
        }        
        /* Get PCS ANEG status register */
        JR_RDX(DEV1G, PCS1G_CFG_STATUS_PCS1G_ANEG_STATUS, VTSS_TO_DEV(port), &value);           
        /* Get 'Aneg complete'   */
        status->autoneg.complete = VTSS_BOOL(value & VTSS_F_DEV1G_PCS1G_CFG_STATUS_PCS1G_ANEG_STATUS_ANEG_COMPLETE);        

        /* Workaround for a Serdes issue, when aneg completes with FDX capability=0 */
        if (vtss_state->port.conf[port_no].if_type == VTSS_PORT_INTERFACE_SERDES) {
            if (status->autoneg.complete) {
                if (((value >> 21) & 0x1) == 0) { 
                    JR_WRXB(DEV1G, PCS1G_CFG_STATUS_PCS1G_CFG, VTSS_TO_DEV(port), PCS_ENA, 0);
                    JR_WRXB(DEV1G, PCS1G_CFG_STATUS_PCS1G_CFG, VTSS_TO_DEV(port), PCS_ENA, 1);
                    (void)jr_port_clause_37_control_set(vtss_state, port_no); /* Restart Aneg */
                    VTSS_MSLEEP(50);
                    JR_RDX(DEV1G, PCS1G_CFG_STATUS_PCS1G_ANEG_STATUS, VTSS_TO_DEV(port), &value);           
                    status->autoneg.complete = VTSS_BOOL(value & VTSS_F_DEV1G_PCS1G_CFG_STATUS_PCS1G_ANEG_STATUS_ANEG_COMPLETE);        
                }
            }
        }

        /* Return partner advertisement ability */
        value = VTSS_X_DEV1G_PCS1G_CFG_STATUS_PCS1G_ANEG_STATUS_LP_ADV_ABILITY(value);        
    } else {
        /* 10G in 1G mode */
        JR_RDX(DEV10G, PCS1G_CFG_STATUS_PCS1G_STICKY, VTSS_TO_DEV(port), &value);
        value = (VTSS_F_DEV10G_PCS1G_CFG_STATUS_PCS1G_STICKY_LINK_DOWN_STICKY & value); 
        if (VTSS_BOOL(value)) {
            /* The link has been down. Clear the sticky bit and return the 'down' value  */
            JR_WRXB(DEV10G, PCS1G_CFG_STATUS_PCS1G_STICKY, VTSS_TO_DEV(port), LINK_DOWN_STICKY, 1);
            JR_WRXB(DEV10G, PCS1G_CFG_STATUS_PCS1G_STICKY, VTSS_TO_DEV(port), OUT_OF_SYNC_STICKY, 1);
            status->link = 0;
        } else {
            /*  Return the current status     */
            JR_RDX(DEV10G, PCS1G_CFG_STATUS_PCS1G_LINK_STATUS, VTSS_TO_DEV(port), &value);
            status->link = VTSS_BOOL(VTSS_F_DEV10G_PCS1G_CFG_STATUS_PCS1G_LINK_STATUS_LINK_STATUS & value) && 
                           VTSS_BOOL(VTSS_F_DEV10G_PCS1G_CFG_STATUS_PCS1G_LINK_STATUS_SYNC_STATUS & value);        
        }        
        /* Get PCS ANEG status register */
        JR_RDX(DEV10G, PCS1G_CFG_STATUS_PCS1G_ANEG_STATUS, VTSS_TO_DEV(port), &value);           
        /* Get 'Aneg complete'   */
        status->autoneg.complete = VTSS_BOOL(value & VTSS_F_DEV10G_PCS1G_CFG_STATUS_PCS1G_ANEG_STATUS_ANEG_COMPLETE);        

        /* Workaround for a Serdes issue, when aneg completes with FDX capability=0 */
        if (vtss_state->port.conf[port_no].if_type == VTSS_PORT_INTERFACE_SERDES) {
            if (status->autoneg.complete) {
                if (((value >> 21) & 0x1) == 0) { 
                    JR_WRXB(DEV10G, PCS1G_CFG_STATUS_PCS1G_CFG, VTSS_TO_DEV(port), PCS_ENA, 0);
                    JR_WRXB(DEV10G, PCS1G_CFG_STATUS_PCS1G_CFG, VTSS_TO_DEV(port), PCS_ENA, 1);
                    (void)jr_port_clause_37_control_set(vtss_state, port_no); /* Restart Aneg */
                    VTSS_MSLEEP(50);
                    JR_RDX(DEV10G, PCS1G_CFG_STATUS_PCS1G_ANEG_STATUS, VTSS_TO_DEV(port), &value);           
                    status->autoneg.complete = VTSS_BOOL(value & VTSS_F_DEV10G_PCS1G_CFG_STATUS_PCS1G_ANEG_STATUS_ANEG_COMPLETE);        
                }
            }
        }


        /* Return partner advertisement ability */
        value = VTSS_X_DEV10G_PCS1G_CFG_STATUS_PCS1G_ANEG_STATUS_LP_ADV_ABILITY(value);
    }

    if (vtss_state->port.conf[port_no].if_type == VTSS_PORT_INTERFACE_SGMII_CISCO) {
        status->autoneg.partner_adv_sgmii.link = ((value >> 15) == 1) ? 1 : 0;
        status->autoneg.partner_adv_sgmii.speed_10M = (((value >> 10) & 3) == 0) ? 1 : 0;
        status->autoneg.partner_adv_sgmii.speed_100M =(((value >> 10) & 3) == 1) ? 1 : 0;
        status->autoneg.partner_adv_sgmii.speed_1G =  (((value >> 10) & 3) == 2) ? 1 : 0;
        status->autoneg.partner_adv_sgmii.fdx = (((value >> 12) & 0x1) == 1) ? 1 : 0;
        status->autoneg.partner_adv_sgmii.hdx = status->autoneg.partner_advertisement.fdx ? 0 : 1;
        if (status->link) {
            /* If the SFP module does not have a link then the port does not have link */
            status->link = status->autoneg.partner_adv_sgmii.link;
        }
    } else {
        VTSS_RC(vtss_cmn_port_clause_37_adv_get(value, &status->autoneg.partner_advertisement));
    }


    return VTSS_RC_OK;
}

/* Get status of the XAUI, VAUI or 100FX ports. */
/* Status for SERDES and SGMII ports is handled elsewhere (through autonegotiation) */
static vtss_rc jr_port_status_get(vtss_state_t *vtss_state,
                                  const vtss_port_no_t  port_no,
                                  vtss_port_status_t    *const status)
{
    u32              addr, value;
    u32              port = VTSS_CHIP_PORT(port_no);
    u32              tgt = VTSS_TO_DEV(port);
    vtss_port_conf_t *conf = &vtss_state->port.conf[port_no];
    
    if (conf->power_down) {
        /* Disabled port is considered down */
        return VTSS_RC_OK;
    }

    switch (vtss_state->port.conf[port_no].if_type) {
    case VTSS_PORT_INTERFACE_100FX:
        /* Get the PCS status  */
        JR_RDX(DEV1G, PCS_FX100_STATUS_PCS_FX100_STATUS, tgt, &value);
        
        /* Link has been down if the are any error stickies */
        status->link_down = JR_GET_BIT(DEV1G, PCS_FX100_STATUS_PCS_FX100_STATUS, SYNC_LOST_STICKY, value) ||
                            JR_GET_BIT(DEV1G, PCS_FX100_STATUS_PCS_FX100_STATUS, SSD_ERROR_STICKY, value) ||
                            JR_GET_BIT(DEV1G, PCS_FX100_STATUS_PCS_FX100_STATUS, FEF_FOUND_STICKY, value) ||
                            JR_GET_BIT(DEV1G, PCS_FX100_STATUS_PCS_FX100_STATUS, PCS_ERROR_STICKY, value);

        if (status->link_down) {
            JR_WRX(DEV1G, PCS_FX100_STATUS_PCS_FX100_STATUS, tgt, 0xFFFF);
            JR_RDX(DEV1G, PCS_FX100_STATUS_PCS_FX100_STATUS, tgt, &value);
        }
        /* Link=1 if sync status=1 and no error stickies after a clear */
        status->link = JR_GET_BIT(DEV1G, PCS_FX100_STATUS_PCS_FX100_STATUS, SYNC_STATUS, value) && 
                     (!JR_GET_BIT(DEV1G, PCS_FX100_STATUS_PCS_FX100_STATUS, SYNC_LOST_STICKY, value) &&
                      !JR_GET_BIT(DEV1G, PCS_FX100_STATUS_PCS_FX100_STATUS, FEF_FOUND_STICKY, value) &&
                      !JR_GET_BIT(DEV1G, PCS_FX100_STATUS_PCS_FX100_STATUS, PCS_ERROR_STICKY, value) &&
                      !JR_GET_BIT(DEV1G, PCS_FX100_STATUS_PCS_FX100_STATUS, PCS_ERROR_STICKY, value));

        status->speed = VTSS_SPEED_100M; 
        break;
    case VTSS_PORT_INTERFACE_XAUI:
    case VTSS_PORT_INTERFACE_RXAUI:
        /* MAC10G Tx Monitor Sticky bit Register */
        addr = VTSS_IOREG(tgt, 0x1b);
        VTSS_RC(vtss_jr1_rd(vtss_state, addr, &value));
        if (value != 2) {
            /* The link is or has been down. Clear the sticky bit */
            status->link_down = 1;
            VTSS_RC(vtss_jr1_wr(vtss_state, addr, 0x1F));
            VTSS_RC(vtss_jr1_rd(vtss_state, addr, &value));
        }
        /* IDLE_STATE_STICKY = 1 and no others stickys --> link on */
        status->link = (value == 2 ? 1 : 0);    
        status->speed = conf->speed;
        break;
    case VTSS_PORT_INTERFACE_VAUI:
        if (VTSS_PORT_IS_1G(port)) {
            /* Get the PCS status */
            JR_RDX(DEV1G, PCS1G_CFG_STATUS_PCS1G_STICKY, tgt, &value);
            if (JR_GET_BIT(DEV1G, PCS1G_CFG_STATUS_PCS1G_STICKY, LINK_DOWN_STICKY, value)) {
                /* The link has been down. Clear the sticky bit */
                status->link_down = 1;
                JR_WRXB(DEV1G, PCS1G_CFG_STATUS_PCS1G_STICKY, tgt, LINK_DOWN_STICKY, 1);
                JR_WRXB(DEV1G, PCS1G_CFG_STATUS_PCS1G_STICKY, tgt, OUT_OF_SYNC_STICKY, 1);
            }
            JR_RDX(DEV1G, PCS1G_CFG_STATUS_PCS1G_LINK_STATUS, tgt, &value);
            status->link = VTSS_BOOL(VTSS_F_DEV1G_PCS1G_CFG_STATUS_PCS1G_LINK_STATUS_LINK_STATUS & value) && 
                           VTSS_BOOL(VTSS_F_DEV1G_PCS1G_CFG_STATUS_PCS1G_LINK_STATUS_SYNC_STATUS & value);        

            status->speed = VTSS_SPEED_2500M;
        } else {
            /* 10G in 2500M mode */
            JR_RDX(DEV10G, PCS1G_CFG_STATUS_PCS1G_STICKY, tgt, &value);
            value = (VTSS_F_DEV10G_PCS1G_CFG_STATUS_PCS1G_STICKY_LINK_DOWN_STICKY & value);        
            if (VTSS_BOOL(value)) {
                /* The link has been down. Clear the sticky bit and return the 'down' value  */
                JR_WRXB(DEV10G, PCS1G_CFG_STATUS_PCS1G_STICKY, tgt, LINK_DOWN_STICKY, 1);
                JR_WRXB(DEV10G, PCS1G_CFG_STATUS_PCS1G_STICKY, tgt, OUT_OF_SYNC_STICKY, 1);
                status->link_down = 1;
            }        
            /*  Return the current status     */
            JR_RDX(DEV10G, PCS1G_CFG_STATUS_PCS1G_LINK_STATUS, tgt, &value);
            status->link = VTSS_BOOL(VTSS_F_DEV10G_PCS1G_CFG_STATUS_PCS1G_LINK_STATUS_LINK_STATUS & value) && 
                           VTSS_BOOL(VTSS_F_DEV10G_PCS1G_CFG_STATUS_PCS1G_LINK_STATUS_SYNC_STATUS & value);        
            status->speed = VTSS_SPEED_2500M;

        }
        break;
    case VTSS_PORT_INTERFACE_NO_CONNECTION:
        status->link = 0;
        status->link_down = 0;
        break;
    default:
        VTSS_E("Status not supported for port: %u", port_no);
        return VTSS_RC_ERROR;
    }
    status->fdx = conf->fdx;
    return VTSS_RC_OK;
}

/* Handle 1G counter in ASM */
#define JR_CNT_1G(name, i, cnt, clr)                     \
{                                                        \
    u32 value;                                           \
    JR_RDX(ASM, DEV_STATISTICS_##name##_CNT, i, &value); \
    vtss_cmn_counter_32_update(value, cnt, clr);         \
}

/* Handle 10G counter in DEV10G */
#define JR_CNT_10G(name, i, cnt, clr)                       \
{                                                           \
    u32 value;                                              \
    JR_RDX(DEV10G, DEV_STATISTICS_##name##_CNT, i, &value); \
    vtss_cmn_counter_32_update(value, cnt, clr);            \
}

/* Update/clear/get port counters for chip */
static vtss_rc jr_port_counters_chip(vtss_state_t *vtss_state,
                                     u32 port, 
                                     vtss_port_jr1_counters_t *c,
                                     vtss_port_counters_t     *const counters,
                                     BOOL                      clr)
{
    u32                                i;
    vtss_port_counter_t                rx_errors;
    vtss_port_rmon_counters_t          *rmon;
    vtss_port_if_group_counters_t      *if_group;
    vtss_port_ethernet_like_counters_t *elike;
    vtss_port_proprietary_counters_t   *prop;

    if (VTSS_PORT_IS_1G(port)) {
        i = (port == VTSS_PORT_NPI ? 27 : port);
        JR_CNT_1G(RX_IN_BYTES, i, &c->rx_in_bytes, clr);
        JR_CNT_1G(RX_SYMBOL_ERR, i, &c->rx_symbol_err, clr);
        JR_CNT_1G(RX_PAUSE, i, &c->rx_pause, clr);
        JR_CNT_1G(RX_UNSUP_OPCODE, i, &c->rx_unsup_opcode, clr);
        JR_CNT_1G(RX_OK_BYTES, i, &c->rx_ok_bytes, clr);
        JR_CNT_1G(RX_BAD_BYTES, i, &c->rx_bad_bytes, clr);
        JR_CNT_1G(RX_UC, i, &c->rx_unicast, clr);
        JR_CNT_1G(RX_MC, i, &c->rx_multicast, clr);
        JR_CNT_1G(RX_BC, i, &c->rx_broadcast, clr);
        JR_CNT_1G(RX_CRC_ERR, i, &c->rx_crc_err, clr);
        JR_CNT_1G(RX_UNDERSIZE, i, &c->rx_undersize, clr);
        JR_CNT_1G(RX_FRAGMENTS, i, &c->rx_fragments, clr);
        JR_CNT_1G(RX_IN_RANGE_LEN_ERR, i, &c->rx_in_range_len_err, clr);
        JR_CNT_1G(RX_OUT_OF_RANGE_LEN_ERR, i, &c->rx_out_of_range_len_err, clr);
        JR_CNT_1G(RX_OVERSIZE, i, &c->rx_oversize, clr);
        JR_CNT_1G(RX_JABBERS, i, &c->rx_jabbers, clr);
        JR_CNT_1G(RX_SIZE64, i, &c->rx_size64, clr);
        JR_CNT_1G(RX_SIZE65TO127, i, &c->rx_size65_127, clr);
        JR_CNT_1G(RX_SIZE128TO255, i, &c->rx_size128_255, clr);
        JR_CNT_1G(RX_SIZE256TO511, i, &c->rx_size256_511, clr);
        JR_CNT_1G(RX_SIZE512TO1023, i, &c->rx_size512_1023, clr);
        JR_CNT_1G(RX_SIZE1024TO1518, i, &c->rx_size1024_1518, clr);
        JR_CNT_1G(RX_SIZE1519TOMAX, i, &c->rx_size1519_max, clr);

        JR_CNT_1G(TX_OUT_BYTES, i, &c->tx_out_bytes, clr);
        JR_CNT_1G(TX_PAUSE, i, &c->tx_pause, clr);
        JR_CNT_1G(TX_OK_BYTES, i, &c->tx_ok_bytes, clr);
        JR_CNT_1G(TX_UC, i, &c->tx_unicast, clr);
        JR_CNT_1G(TX_MC, i, &c->tx_multicast, clr);
        JR_CNT_1G(TX_BC, i, &c->tx_broadcast, clr);
        JR_CNT_1G(TX_SIZE64, i, &c->tx_size64, clr);
        JR_CNT_1G(TX_SIZE65TO127, i, &c->tx_size65_127, clr);
        JR_CNT_1G(TX_SIZE128TO255, i, &c->tx_size128_255, clr);
        JR_CNT_1G(TX_SIZE256TO511, i, &c->tx_size256_511, clr);
        JR_CNT_1G(TX_SIZE512TO1023, i, &c->tx_size512_1023, clr);
        JR_CNT_1G(TX_SIZE1024TO1518, i, &c->tx_size1024_1518, clr);
        JR_CNT_1G(TX_SIZE1519TOMAX, i, &c->tx_size1519_max, clr);
        JR_CNT_1G(TX_MULTI_COLL, i, &c->tx_multi_coll, clr);
        JR_CNT_1G(TX_LATE_COLL, i, &c->tx_late_coll, clr);
        JR_CNT_1G(TX_XCOLL, i, &c->tx_xcoll, clr);
        JR_CNT_1G(TX_DEFER, i, &c->tx_defer, clr);
        JR_CNT_1G(TX_XDEFER, i, &c->tx_xdefer, clr);
        JR_CNT_1G(TX_BACKOFF1, i, &c->tx_backoff1, clr);
    } else {
        i = VTSS_TO_DEV(port);
        JR_CNT_10G(40BIT_RX_IN_BYTES, i, &c->rx_in_bytes, clr);
        JR_CNT_10G(32BIT_RX_SYMBOL_ERR, i, &c->rx_symbol_err, clr);
        JR_CNT_10G(32BIT_RX_PAUSE, i, &c->rx_pause, clr);
        JR_CNT_10G(32BIT_RX_UNSUP_OPCODE, i, &c->rx_unsup_opcode, clr);
        JR_CNT_10G(40BIT_RX_OK_BYTES, i, &c->rx_ok_bytes, clr);
        JR_CNT_10G(40BIT_RX_BAD_BYTES, i, &c->rx_bad_bytes, clr);
        JR_CNT_10G(32BIT_RX_UC, i, &c->rx_unicast, clr);
        JR_CNT_10G(32BIT_RX_MC, i, &c->rx_multicast, clr);
        JR_CNT_10G(32BIT_RX_BC, i, &c->rx_broadcast, clr);
        JR_CNT_10G(32BIT_RX_CRC_ERR, i, &c->rx_crc_err, clr);
        JR_CNT_10G(32BIT_RX_UNDERSIZE, i, &c->rx_undersize, clr);
        JR_CNT_10G(32BIT_RX_FRAGMENTS, i, &c->rx_fragments, clr);
        JR_CNT_10G(32BIT_RX_IN_RANGE_LEN_ERR, i, &c->rx_in_range_len_err, clr);
        JR_CNT_10G(32BIT_RX_OUT_OF_RANGE_LEN_ERR, i, &c->rx_out_of_range_len_err, clr);
        JR_CNT_10G(32BIT_RX_OVERSIZE, i, &c->rx_oversize, clr);
        JR_CNT_10G(32BIT_RX_JABBERS, i, &c->rx_jabbers, clr);
        JR_CNT_10G(32BIT_RX_SIZE64, i, &c->rx_size64, clr);
        JR_CNT_10G(32BIT_RX_SIZE65TO127, i, &c->rx_size65_127, clr);
        JR_CNT_10G(32BIT_RX_SIZE128TO255, i, &c->rx_size128_255, clr);
        JR_CNT_10G(32BIT_RX_SIZE256TO511, i, &c->rx_size256_511, clr);
        JR_CNT_10G(32BIT_RX_SIZE512TO1023, i, &c->rx_size512_1023, clr);
        JR_CNT_10G(32BIT_RX_SIZE1024TO1518, i, &c->rx_size1024_1518, clr);
        JR_CNT_10G(32BIT_RX_SIZE1519TOMAX, i, &c->rx_size1519_max, clr);

        JR_CNT_10G(40BIT_TX_OUT_BYTES, i, &c->tx_out_bytes, clr);
        JR_CNT_10G(32BIT_TX_PAUSE, i, &c->tx_pause, clr);
        JR_CNT_10G(40BIT_TX_OK_BYTES, i, &c->tx_ok_bytes, clr);
        JR_CNT_10G(32BIT_TX_UC, i, &c->tx_unicast, clr);
        JR_CNT_10G(32BIT_TX_MC, i, &c->tx_multicast, clr);
        JR_CNT_10G(32BIT_TX_BC, i, &c->tx_broadcast, clr);
        JR_CNT_10G(32BIT_TX_SIZE64, i, &c->tx_size64, clr);
        JR_CNT_10G(32BIT_TX_SIZE65TO127, i, &c->tx_size65_127, clr);
        JR_CNT_10G(32BIT_TX_SIZE128TO255, i, &c->tx_size128_255, clr);
        JR_CNT_10G(32BIT_TX_SIZE256TO511, i, &c->tx_size256_511, clr);
        JR_CNT_10G(32BIT_TX_SIZE512TO1023, i, &c->tx_size512_1023, clr);
        JR_CNT_10G(32BIT_TX_SIZE1024TO1518, i, &c->tx_size1024_1518, clr);
        JR_CNT_10G(32BIT_TX_SIZE1519TOMAX, i, &c->tx_size1519_max, clr);
    }

    /* SQS counters */
    for (i = 0; i < VTSS_PRIOS; i++) {
        JR_CNT_SQS(IQS, RX, GREEN, port, i, &c->rx_green_class[i], clr);        
        JR_CNT_SQS(IQS, RX, YELLOW, port, i, &c->rx_yellow_class[i], clr);
        JR_CNT_SQS(IQS, RX, QDROPS, port, i, &c->rx_queue_drops[i], clr);
        JR_CNT_SQS(IQS, RX, PDROPS, port, i, &c->rx_policer_drops[i], clr);
        JR_CNT_SQS(IQS, TX, DROPS, port, i, &c->rx_txqueue_drops[i], clr);

        JR_CNT_SQS(OQS, RX, QDROPS, port, i, &c->tx_queue_drops[i], clr);
        JR_CNT_SQS(OQS, TX, GREEN, port, i, &c->tx_green_class[i], clr);
        JR_CNT_SQS(OQS, TX, YELLOW, port, i, &c->tx_yellow_class[i], clr);
    }

    /* ANA_AC counter */
    {
        u32 value;
        
        JR_RDXY(ANA_AC, STAT_CNT_CFG_PORT_STAT_LSB_CNT, port, JR_CNT_ANA_AC_FILTER, &value);
        vtss_cmn_counter_32_update(value, &c->rx_local_drops, clr);
    }
    
    if (counters == NULL) {
        return VTSS_RC_OK;
    }

    /* Proprietary counters */
    prop = &counters->prop;
    for (i = 0; i < VTSS_PRIOS; i++) {
        prop->rx_prio[i] = c->rx_red_class[i].value + c->rx_yellow_class[i].value + c->rx_green_class[i].value;
        prop->tx_prio[i] = c->tx_yellow_class[i].value + c->tx_green_class[i].value;
    }

    /* RMON Rx counters */
    rmon = &counters->rmon;
    rmon->rx_etherStatsDropEvents = 0;
    for (i = 0; i < VTSS_PRIOS; i++) {
        rmon->rx_etherStatsDropEvents += (c->rx_queue_drops[i].value + c->rx_txqueue_drops[i].value + c->rx_policer_drops[i].value);
    }
    rmon->rx_etherStatsOctets = (c->rx_ok_bytes.value + c->rx_bad_bytes.value);
    rx_errors = (c->rx_crc_err.value +  c->rx_undersize.value + c->rx_oversize.value +
                 c->rx_out_of_range_len_err.value + c->rx_symbol_err.value +
                 c->rx_jabbers.value + c->rx_fragments.value);
    rmon->rx_etherStatsPkts = (c->rx_unicast.value + c->rx_multicast.value +
                               c->rx_broadcast.value + rx_errors);
    rmon->rx_etherStatsBroadcastPkts = c->rx_broadcast.value;
    rmon->rx_etherStatsMulticastPkts = c->rx_multicast.value;
    rmon->rx_etherStatsCRCAlignErrors = c->rx_crc_err.value;
    rmon->rx_etherStatsUndersizePkts = c->rx_undersize.value;
    rmon->rx_etherStatsOversizePkts = c->rx_oversize.value;
    rmon->rx_etherStatsFragments = c->rx_fragments.value;
    rmon->rx_etherStatsJabbers = c->rx_jabbers.value;
    rmon->rx_etherStatsPkts64Octets = c->rx_size64.value;
    rmon->rx_etherStatsPkts65to127Octets = c->rx_size65_127.value;
    rmon->rx_etherStatsPkts128to255Octets = c->rx_size128_255.value;
    rmon->rx_etherStatsPkts256to511Octets = c->rx_size256_511.value;
    rmon->rx_etherStatsPkts512to1023Octets = c->rx_size512_1023.value;
    rmon->rx_etherStatsPkts1024to1518Octets = c->rx_size1024_1518.value;
    rmon->rx_etherStatsPkts1519toMaxOctets = c->rx_size1519_max.value;

    /* RMON Tx counters */
    rmon->tx_etherStatsDropEvents = 0;
    for (i = 0; i < VTSS_PRIOS; i++) {
        rmon->tx_etherStatsDropEvents += c->tx_queue_drops[i].value;
    }
    rmon->tx_etherStatsPkts = (c->tx_unicast.value + c->tx_multicast.value +
                               c->tx_broadcast.value + c->tx_late_coll.value);
    rmon->tx_etherStatsOctets = c->tx_ok_bytes.value;
    rmon->tx_etherStatsBroadcastPkts = c->tx_broadcast.value;
    rmon->tx_etherStatsMulticastPkts = c->tx_multicast.value;
    rmon->tx_etherStatsCollisions = (c->tx_multi_coll.value + c->tx_backoff1.value +
                                     c->tx_late_coll.value + c->tx_xcoll.value);
    rmon->tx_etherStatsPkts64Octets = c->tx_size64.value;
    rmon->tx_etherStatsPkts65to127Octets = c->tx_size65_127.value;
    rmon->tx_etherStatsPkts128to255Octets = c->tx_size128_255.value;
    rmon->tx_etherStatsPkts256to511Octets = c->tx_size256_511.value;
    rmon->tx_etherStatsPkts512to1023Octets = c->tx_size512_1023.value;
    rmon->tx_etherStatsPkts1024to1518Octets = c->tx_size1024_1518.value;
    rmon->tx_etherStatsPkts1519toMaxOctets = c->tx_size1519_max.value;

    /* Interfaces Group Rx counters */
    if_group = &counters->if_group;
    if_group->ifInOctets = rmon->rx_etherStatsOctets;
    if_group->ifInUcastPkts = c->rx_unicast.value;
    if_group->ifInMulticastPkts = c->rx_multicast.value;
    if_group->ifInBroadcastPkts = c->rx_broadcast.value;
    if_group->ifInNUcastPkts = (c->rx_multicast.value + c->rx_broadcast.value);
    if_group->ifInDiscards = rmon->rx_etherStatsDropEvents;
    /* Ignore the in-range lenigth error. The in-range length error counter will be increased 
       when the incoming frame's length/ethertype field is smaller than 0x600, but the value doens't match the actual payload.
       Practically, if users just want to verify the L2 basic forwarding functions, they won't edit the length/ethertype field of the testing frame,
       if the counters is included in ifInErrors, it will confuse users, so it makes sense to ignore it. */
    if_group->ifInErrors = rx_errors;

    /* Interfaces Group Tx counters */
    if_group->ifOutOctets = rmon->tx_etherStatsOctets;
    if_group->ifOutUcastPkts = c->tx_unicast.value;
    if_group->ifOutMulticastPkts = c->tx_multicast.value;
    if_group->ifOutBroadcastPkts = c->tx_broadcast.value;
    if_group->ifOutNUcastPkts = (c->tx_multicast.value + c->tx_broadcast.value);
    if_group->ifOutDiscards = rmon->tx_etherStatsDropEvents;
    if_group->ifOutErrors = (c->tx_late_coll.value + c->tx_csense.value + c->tx_xcoll.value);

    /* Ethernet-like Rx counters */
    elike = &counters->ethernet_like;
    elike->dot3StatsAlignmentErrors = 0; /* Not supported */
    elike->dot3StatsFCSErrors = c->rx_crc_err.value;
    elike->dot3StatsFrameTooLongs = c->rx_oversize.value;
    elike->dot3StatsSymbolErrors = c->rx_symbol_err.value;
    elike->dot3ControlInUnknownOpcodes = c->rx_unsup_opcode.value;
    elike->dot3InPauseFrames = c->rx_pause.value;

    /* Ethernet-like Tx counters */
    elike->dot3StatsSingleCollisionFrames = c->tx_backoff1.value;
    elike->dot3StatsMultipleCollisionFrames = c->tx_multi_coll.value;
    elike->dot3StatsDeferredTransmissions = c->tx_defer.value;
    elike->dot3StatsLateCollisions = c->tx_late_coll.value;
    elike->dot3StatsExcessiveCollisions = c->tx_xcoll.value;
    elike->dot3StatsCarrierSenseErrors = c->tx_csense.value;
    elike->dot3OutPauseFrames = c->tx_pause.value;

    /* Bridge counters */
    /* This will include filtered frames with and without CRC error */
    counters->bridge.dot1dTpPortInDiscards = c->rx_local_drops.value;

    return VTSS_RC_OK;
}

/* Update/clear/get port counters */
static vtss_rc jr_port_counters(vtss_state_t *vtss_state,
                                const vtss_port_no_t port_no,
                                vtss_port_counters_t *const counters,
                                BOOL                 clr)
{
    return jr_port_counters_chip(vtss_state,
                                 VTSS_CHIP_PORT(port_no),
                                 &vtss_state->port.counters[port_no].counter.jr1,
                                 counters,
                                 clr);
}


static vtss_rc jr_port_counters_update(vtss_state_t *vtss_state, const vtss_port_no_t port_no)
{
    return jr_port_counters(vtss_state, port_no, NULL, 0);
}

static vtss_rc jr_port_counters_clear(vtss_state_t *vtss_state, const vtss_port_no_t port_no)
{
    return jr_port_counters(vtss_state, port_no, NULL, 1);
}

static vtss_rc jr_port_counters_get(vtss_state_t *vtss_state, const vtss_port_no_t port_no,
                                    vtss_port_counters_t *const counters)
{
    return jr_port_counters(vtss_state, port_no, counters, 0);
}

static vtss_rc jr_port_basic_counters_get(vtss_state_t *vtss_state,
                                          const vtss_port_no_t port_no,
                                          vtss_basic_counters_t *const counters)
{
    u32                      i, port = VTSS_CHIP_PORT(port_no);
    vtss_port_jr1_counters_t *c = &vtss_state->port.counters[port_no].counter.jr1;

    if (VTSS_PORT_IS_1G(port)) {
        i = (port == VTSS_PORT_NPI ? 27 : port);
        JR_CNT_1G(RX_UC, i, &c->rx_unicast, 0);
        JR_CNT_1G(RX_MC, i, &c->rx_multicast, 0);
        JR_CNT_1G(RX_BC, i, &c->rx_broadcast, 0);
        JR_CNT_1G(TX_UC, i, &c->tx_unicast, 0);
        JR_CNT_1G(TX_MC, i, &c->tx_multicast, 0);
        JR_CNT_1G(TX_BC, i, &c->tx_broadcast, 0);
    } else {
        i = VTSS_TO_DEV(port);
        JR_CNT_10G(32BIT_RX_UC, i, &c->rx_unicast, 0);
        JR_CNT_10G(32BIT_RX_MC, i, &c->rx_multicast, 0);
        JR_CNT_10G(32BIT_RX_BC, i, &c->rx_broadcast, 0);
        JR_CNT_10G(32BIT_TX_UC, i, &c->tx_unicast, 0);
        JR_CNT_10G(32BIT_TX_MC, i, &c->tx_multicast, 0);
        JR_CNT_10G(32BIT_TX_BC, i, &c->tx_broadcast, 0);
    }

    counters->rx_frames = (c->rx_unicast.value + c->rx_multicast.value + c->rx_broadcast.value);
    counters->tx_frames = (c->tx_unicast.value + c->tx_multicast.value + c->tx_broadcast.value);

    return VTSS_RC_OK;
}

static vtss_rc jr_port_forward_set(vtss_state_t *vtss_state, const vtss_port_no_t port_no)
{
    return VTSS_RC_OK;
}

#if defined(VTSS_FEATURE_SYNCE)
/* ================================================================= *
 *  SYNCE (Level 1 syncronization)
 * ================================================================= */

static vtss_rc jr_synce_check_target(vtss_state_t *vtss_state)
{
    vtss_rc rc;

    switch (jr_target_get(vtss_state)) {
    case VTSS_TARGET_JAGUAR_1:
    case VTSS_TARGET_CE_MAX_24:
    case VTSS_TARGET_LYNX_1:
    case VTSS_TARGET_CE_MAX_12:
        rc = VTSS_RC_OK;
        break;
    default:
        rc = VTSS_RC_ERROR;
        break;
    }
    return rc;
}


static vtss_rc jr_synce_clock_out_set(vtss_state_t *vtss_state, const u32 clk_port)
{
    u32 eth_cfg, div_mask, en_mask;

//printf("jr_synce_clock_out_set  clk_port %X  enable %u\n", clk_port, vtss_state->synce_out_conf[clk_port].enable);
    VTSS_RC(jr_synce_check_target(vtss_state));

    div_mask = (clk_port ? VTSS_M_MACRO_CTRL_SYNC_ETH_CFG_SYNC_ETH_CFG_SEL_RECO_CLK_B : VTSS_M_MACRO_CTRL_SYNC_ETH_CFG_SYNC_ETH_CFG_SEL_RECO_CLK_A);
    en_mask = (clk_port ? VTSS_F_MACRO_CTRL_SYNC_ETH_CFG_SYNC_ETH_CFG_RECO_CLK_B_ENA : VTSS_F_MACRO_CTRL_SYNC_ETH_CFG_SYNC_ETH_CFG_RECO_CLK_A_ENA);

    JR_RD(MACRO_CTRL, SYNC_ETH_CFG_SYNC_ETH_CFG, &eth_cfg);
    eth_cfg &= ~(div_mask | en_mask);      /* clear the related bits for this configuration */
    switch (vtss_state->synce.out_conf[clk_port].divider) {
        case VTSS_SYNCE_DIVIDER_1: div_mask = (clk_port ? VTSS_F_MACRO_CTRL_SYNC_ETH_CFG_SYNC_ETH_CFG_SEL_RECO_CLK_B(0) : VTSS_F_MACRO_CTRL_SYNC_ETH_CFG_SYNC_ETH_CFG_SEL_RECO_CLK_A(0)); break;
        case VTSS_SYNCE_DIVIDER_4: div_mask = (clk_port ? VTSS_F_MACRO_CTRL_SYNC_ETH_CFG_SYNC_ETH_CFG_SEL_RECO_CLK_B(2) : VTSS_F_MACRO_CTRL_SYNC_ETH_CFG_SYNC_ETH_CFG_SEL_RECO_CLK_A(2)); break;
        case VTSS_SYNCE_DIVIDER_5: div_mask = (clk_port ? VTSS_F_MACRO_CTRL_SYNC_ETH_CFG_SYNC_ETH_CFG_SEL_RECO_CLK_B(1) : VTSS_F_MACRO_CTRL_SYNC_ETH_CFG_SYNC_ETH_CFG_SEL_RECO_CLK_A(1)); break;
    }
    eth_cfg = eth_cfg | div_mask | (vtss_state->synce.out_conf[clk_port].enable ? en_mask : 0);
//printf("eth_cfg %u\n", eth_cfg);
    JR_WR(MACRO_CTRL, SYNC_ETH_CFG_SYNC_ETH_CFG,  eth_cfg);  /* Set the related bits defending on configuration */

    return VTSS_RC_OK;
}

static vtss_rc jr_synce_clock_in_set(vtss_state_t *vtss_state, const u32 clk_port)
{
    vtss_synce_clock_in_t *conf = &vtss_state->synce.in_conf[clk_port];
    serdes_t    serdes_type;
    u32         serdes_instance, common_cfg, mask, sq_mask, en_mask;
    i32         new_chip_port = vtss_state->port.map[conf->port_no].chip_port;
    i32         old_chip_port = vtss_state->synce.old_port_no[clk_port];
    u32         count;

//printf("jr_synce_clock_in_set  clk_port %X  enable %u\n", clk_port, vtss_state->synce_in_conf[clk_port].enable);
    VTSS_RC(jr_synce_check_target(vtss_state));

    if (!conf->enable || (new_chip_port != old_chip_port)) {
    /* Disable of this clock port or input port has changed for this clock output port - disable old input */
        if (VTSS_RC_OK == jr_port2serdes_get(vtss_state, old_chip_port, &serdes_type, &serdes_instance, &count)) {
            mask = (1<<serdes_instance);
            if (serdes_type == SERDES_1G) {
                VTSS_RC(jr_sd1g_read(vtss_state, mask)); /* Readback the 1G common config register */
                JR_RD(MACRO_CTRL, SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG, &common_cfg);
                common_cfg &= ~(clk_port ? VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG_RECO_SEL_B :
                                VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG_RECO_SEL_A);
//printf("1G  disable   instance %u  common_cfg %X\n", serdes_instance, common_cfg);
                JR_WR(MACRO_CTRL, SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG, common_cfg);   /* Clear the recovered clock enable */
                VTSS_RC(jr_sd1g_write(vtss_state, mask, 0));
            }
            if (serdes_type == SERDES_6G) {
                VTSS_RC(jr_sd6g_read(vtss_state, mask));
                JR_RD(MACRO_CTRL, SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG, &common_cfg);
                common_cfg &= ~(clk_port ? VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_RECO_SEL_B : VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_RECO_SEL_A);
//printf("6G  disable   instance %u  common_cfg %X\n", serdes_instance, common_cfg);
                JR_WR(MACRO_CTRL, SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG, common_cfg);   /* Clear the recovered clock enable */
                VTSS_RC(jr_sd6g_write(vtss_state, mask, 0));
            }
        }
    }

    if (conf->enable) {
    /* Enable input clock configuration - now configuring the new (or maybe the same) input port */
        if (VTSS_RC_OK == jr_port2serdes_get(vtss_state, new_chip_port, &serdes_type, &serdes_instance, &count)) {
            mask = (1<<serdes_instance);
            if (serdes_type == SERDES_1G) {
                sq_mask = (clk_port ? VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG_SE_AUTO_SQUELCH_B_ENA :
                           VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG_SE_AUTO_SQUELCH_A_ENA);
                en_mask = (clk_port ? VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG_RECO_SEL_B :
                           VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG_RECO_SEL_A);
                VTSS_RC(jr_sd1g_read(vtss_state, mask)); /* Readback the 1G common config register */
                JR_RD(MACRO_CTRL, SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG, &common_cfg);
                common_cfg &= ~(sq_mask | en_mask);      /* clear the related bits for this configuration */
                common_cfg |= (conf->squelsh ? sq_mask : 0) | en_mask;
//printf("1G  enable   instance %u  common_cfg %X\n", serdes_instance, common_cfg);
                JR_WR(MACRO_CTRL, SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG, common_cfg);   /* Set the related bits depending on configuration */
                VTSS_RC(jr_sd1g_write(vtss_state, mask, 0)); /* transfer 1G common config register */
            }
            if (serdes_type == SERDES_6G) {
                sq_mask = (clk_port ? VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_SE_AUTO_SQUELCH_B_ENA :
                           VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_SE_AUTO_SQUELCH_A_ENA);
                en_mask = (clk_port ? VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_RECO_SEL_B :
                           VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_RECO_SEL_A);
                VTSS_RC(jr_sd6g_read(vtss_state, mask));                /* Readback the 1G common config register */
                JR_RD(MACRO_CTRL, SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG, &common_cfg);
                common_cfg &= ~(sq_mask | en_mask);      /* clear the related bits for this configuration */
                common_cfg |= (conf->squelsh ? sq_mask : 0) | en_mask;
//printf("6G  enable   instance %u  common_cfg %X\n", serdes_instance, common_cfg);
                JR_WR(MACRO_CTRL, SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG, common_cfg);   /* Set the related bits depending on configuration */
                VTSS_RC(jr_sd6g_write(vtss_state, mask, 0));     /* transfer 1G common config register */
            }
            vtss_state->synce.old_port_no[clk_port] = new_chip_port;
        }
    }

    return VTSS_RC_OK;
}
#endif /* VTSS_FEATURE_SYNCE */

/* - Debug print --------------------------------------------------- */

#define JR_DEBUG_HSIO(pr, addr, name) JR_DEBUG_REG_NAME(pr, MACRO_CTRL, addr, name)
#define JR_DEBUG_MAC(pr, addr, i, name) vtss_jr1_debug_reg_inst(vtss_state, pr, VTSS_DEV1G_MAC_CFG_STATUS_MAC_##addr, i, "MAC_"name)
#define JR_DEBUG_PCS(pr, addr, i, name) vtss_jr1_debug_reg_inst(vtss_state, pr, VTSS_DEV1G_PCS1G_CFG_STATUS_PCS1G_##addr, i, "PCS_"name)
#define JR_DEBUG_10G_MAC(pr, addr, i, name) vtss_jr1_debug_reg_inst(vtss_state, pr, VTSS_DEV10G_MAC_CFG_STATUS_MAC_##addr, i, "MAC_"name)
#define JR_DEBUG_1G_PCS(pr, addr, i, name) vtss_jr1_debug_reg_inst(vtss_state, pr, VTSS_DEV10G_PCS1G_CFG_STATUS_PCS1G_##addr, i, "PCS_"name)
#define JR_DEBUG_10G_PCS(pr, addr, i, name) vtss_jr1_debug_reg_inst(vtss_state, pr, VTSS_DEV10G_PCS_XAUI_##addr, i, "PCS_"name)
#define JR_DEBUG_HSIO_BIT(pr, addr, fld, value) vtss_jr1_debug_fld_nl(pr, #fld, JR_GET_BIT(MACRO_CTRL, addr, fld, x))
#define JR_DEBUG_HSIO_FLD(pr, addr, fld, value) vtss_jr1_debug_fld_nl(pr, #fld, JR_GET_FLD(MACRO_CTRL, addr, fld, x))
#define JR_DEBUG_RAW(pr, offset, length, value, name) vtss_jr1_debug_fld_nl(pr, name, VTSS_EXTRACT_BITFIELD(value, offset, length))


static vtss_rc jr_debug_chip_port(vtss_state_t *vtss_state,
                                  const vtss_debug_printf_t pr,
                                  const vtss_debug_info_t   *const info,
                                  u32 port)
{
    u32      i, tgt, lane = 0, count = 0, x;
    serdes_t type = SERDES_1G;
    char     buf[32];
    
    tgt = VTSS_TO_DEV(port);
    if (VTSS_PORT_IS_10G(port)) {
        JR_DEBUG_10G_MAC(pr, ENA_CFG(tgt), port, "ENA_CFG");
        JR_DEBUG_10G_MAC(pr, MODE_CFG(tgt), port, "MODE_CFG");
        JR_DEBUG_10G_MAC(pr, MAXLEN_CFG(tgt), port, "MAXLEN_CFG");
        JR_DEBUG_10G_MAC(pr, TAGS_CFG(tgt), port, "TAGS_CFG");
        vtss_jr1_debug_reg_inst(vtss_state, pr, VTSS_DEV10G_DEV_CFG_STATUS_DEV_RST_CTRL(tgt), port, "DEV_RST_CTRL");
        JR_DEBUG_10G_PCS(pr, CONFIGURATION_PCS_XAUI_CFG(tgt), port, "XAUI_CFG");
        JR_DEBUG_10G_PCS(pr, STATUS_PCS_XAUI_RX_STATUS(tgt), port,  "XAUI_STATUS");
        JR_DEBUG_10G_PCS(pr, STATUS_PCS_XAUI_RX_ERROR_STATUS(tgt), port, "XAUI_ERROR");
        vtss_jr1_debug_sticky_inst(vtss_state, pr, VTSS_IOREG(tgt, 0x1b), port, "TX_MON_STICKY");
        JR_DEBUG_10G_PCS(pr, CONFIGURATION_PCS_XAUI_EXT_CFG(tgt), port, "XAUI_EXT_CFG");
        JR_DEBUG_10G_PCS(pr, CONFIGURATION_PCS_XAUI_SD_CFG(tgt), port, "XAUI_SD_CFG");
        JR_DEBUG_1G_PCS(pr, ANEG_STATUS(tgt), port, "ANEG_STATUS");

        JR_DEBUG_1G_PCS(pr, ANEG_CFG(tgt), port, "ANEG_CFG");
        JR_DEBUG_1G_PCS(pr, LINK_STATUS(tgt), port, "LINK_STATUS");
    } else {        
        JR_DEBUG_MAC(pr, ENA_CFG(tgt), port, "ENA_CFG");
        JR_DEBUG_MAC(pr, MODE_CFG(tgt), port, "MODE_CFG");
        JR_DEBUG_MAC(pr, MAXLEN_CFG(tgt), port, "MAXLEN_CFG");
        JR_DEBUG_MAC(pr, TAGS_CFG(tgt), port, "TAGS_CFG");
        JR_DEBUG_PCS(pr, CFG(tgt), port, "CFG");
        JR_DEBUG_PCS(pr, MODE_CFG(tgt), port, "MODE_CFG");
        JR_DEBUG_PCS(pr, SD_CFG(tgt), port, "SD_CFG");
        JR_DEBUG_PCS(pr, ANEG_CFG(tgt), port, "ANEG_CFG");
        JR_DEBUG_PCS(pr, ANEG_STATUS(tgt), port, "ANEG_STATUS");
        JR_DEBUG_PCS(pr, LINK_STATUS(tgt), port, "LINK_STATUS");
        vtss_jr1_debug_reg_inst(vtss_state, pr, VTSS_DEV1G_PCS_FX100_CONFIGURATION_PCS_FX100_CFG(tgt), port, 
                          "PCS_FX100_CFG");
        vtss_jr1_debug_reg_inst(vtss_state, pr, VTSS_DEV1G_PCS_FX100_STATUS_PCS_FX100_STATUS(tgt), port, 
                          "FX100_STATUS");
    }
    JR_DEBUG_REGX_NAME(pr, DSM, CFG_RX_PAUSE_CFG, port, "RX_PAUSE_CFG");
    JR_DEBUG_REGX_NAME(pr, DSM, CFG_ETH_FC_GEN, port, "ETH_FC_GEN");
    pr("\n");
    
    if (info->full && jr_port2serdes_get(vtss_state, port, &type, &lane, &count) == VTSS_RC_OK) {
        for (i = lane; i < (lane + count); i++) {
            if (type == SERDES_1G) {
                sprintf(buf, "SerDes1G_%u", i);
                vtss_debug_print_reg_header(pr, buf);
                VTSS_RC(jr_sd1g_read(vtss_state, 1 << i));
                JR_DEBUG_HSIO(pr, SERDES1G_ANA_CFG_SERDES1G_DES_CFG, "DES_CFG");
                JR_DEBUG_HSIO(pr, SERDES1G_ANA_CFG_SERDES1G_IB_CFG, "IB_CFG");
                JR_DEBUG_HSIO(pr, SERDES1G_ANA_CFG_SERDES1G_OB_CFG, "OB_CFG");
                JR_DEBUG_HSIO(pr, SERDES1G_ANA_CFG_SERDES1G_SER_CFG, "SER_CFG");
                JR_DEBUG_HSIO(pr, SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG, "COMMON_CFG");
                JR_DEBUG_HSIO(pr, SERDES1G_ANA_CFG_SERDES1G_PLL_CFG, "PLL_CFG");
                JR_DEBUG_HSIO(pr, SERDES1G_DIG_CFG_SERDES1G_MISC_CFG, "100FX_MISC");
                JR_DEBUG_HSIO(pr, SERDES1G_ANA_STATUS_SERDES1G_PLL_STATUS, "PLL_STATUS");
                JR_DEBUG_HSIO(pr, SERDES1G_DIG_CFG_SERDES1G_DFT_CFG0, "DFT_CFG0");
                JR_DEBUG_HSIO(pr, SERDES1G_DIG_CFG_SERDES1G_DFT_CFG1, "DFT_CFG1");
                JR_DEBUG_HSIO(pr, SERDES1G_DIG_CFG_SERDES1G_DFT_CFG2, "DFT_CFG2");
                JR_DEBUG_HSIO(pr, SERDES1G_DIG_CFG_SERDES1G_TP_CFG, "TP_CFG");
                JR_DEBUG_HSIO(pr, SERDES1G_DIG_CFG_SERDES1G_MISC_CFG, "MISC_CFG");
                JR_DEBUG_HSIO(pr, SERDES1G_DIG_STATUS_SERDES1G_DFT_STATUS, "DFT_STATUS");
            } else {
                sprintf(buf, "SerDes6G_%u", i);
                vtss_debug_print_reg_header(pr, buf);
                VTSS_RC(jr_sd6g_read(vtss_state, 1 << i));
                JR_DEBUG_HSIO(pr, SERDES6G_ANA_CFG_SERDES6G_DES_CFG, "DES_CFG");
                JR_DEBUG_HSIO(pr, SERDES6G_ANA_CFG_SERDES6G_IB_CFG, "IB_CFG");
                JR_DEBUG_HSIO(pr, SERDES6G_ANA_CFG_SERDES6G_IB_CFG1, "IB_CFG1");
                JR_DEBUG_HSIO(pr, SERDES6G_ANA_CFG_SERDES6G_OB_CFG, "OB_CFG");
                JR_DEBUG_HSIO(pr, SERDES6G_ANA_CFG_SERDES6G_OB_CFG1, "OB_CFG1");
                JR_DEBUG_HSIO(pr, SERDES6G_ANA_CFG_SERDES6G_SER_CFG, "SER_CFG");
                JR_DEBUG_HSIO(pr, SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG, "COMMON_CFG");
                JR_DEBUG_HSIO(pr, SERDES6G_ANA_CFG_SERDES6G_PLL_CFG, "PLL_CFG");
                JR_DEBUG_HSIO(pr, SERDES6G_ANA_STATUS_SERDES6G_PLL_STATUS, "PLL_STATUS");
                JR_DEBUG_HSIO(pr, SERDES6G_DIG_CFG_SERDES6G_DIG_CFG, "DIG_CFG");
                JR_DEBUG_HSIO(pr, SERDES6G_DIG_CFG_SERDES6G_DFT_CFG0, "DFT_CFG0");
                JR_DEBUG_HSIO(pr, SERDES6G_DIG_CFG_SERDES6G_DFT_CFG1, "DFT_CFG1");
                JR_DEBUG_HSIO(pr, SERDES6G_DIG_CFG_SERDES6G_DFT_CFG2, "DFT_CFG2");
                JR_DEBUG_HSIO(pr, SERDES6G_DIG_CFG_SERDES6G_TP_CFG0, "TP_CFG0");
                JR_DEBUG_HSIO(pr, SERDES6G_DIG_CFG_SERDES6G_TP_CFG1, "TP_CFG1");
                JR_DEBUG_HSIO(pr, SERDES6G_DIG_CFG_SERDES6G_MISC_CFG, "MISC_CFG");
                JR_DEBUG_HSIO(pr, SERDES6G_DIG_CFG_SERDES6G_OB_ANEG_CFG, "OB_ANEG_CFG");
                JR_DEBUG_HSIO(pr, SERDES6G_DIG_STATUS_SERDES6G_DFT_STATUS, "DFT_STATUS");
                
                pr("\n%s:OB_CFG:\n", buf);
                JR_RD(MACRO_CTRL, SERDES6G_ANA_CFG_SERDES6G_OB_CFG, &x);
                JR_DEBUG_HSIO_BIT(pr, SERDES6G_ANA_CFG_SERDES6G_OB_CFG, OB_IDLE, x);
                JR_DEBUG_HSIO_BIT(pr, SERDES6G_ANA_CFG_SERDES6G_OB_CFG, OB_ENA1V_MODE, x);
                JR_DEBUG_HSIO_BIT(pr, SERDES6G_ANA_CFG_SERDES6G_OB_CFG, OB_POL, x);
                JR_DEBUG_HSIO_FLD(pr, SERDES6G_ANA_CFG_SERDES6G_OB_CFG, OB_POST0, x);
                JR_DEBUG_HSIO_FLD(pr, SERDES6G_ANA_CFG_SERDES6G_OB_CFG, OB_POST1, x);
                JR_DEBUG_HSIO_FLD(pr, SERDES6G_ANA_CFG_SERDES6G_OB_CFG, OB_PREC, x);
                JR_DEBUG_RAW(pr, 12, 1, x, "OB_R_ADJ_MUX");
                JR_DEBUG_RAW(pr, 11, 1, x, "OB_R_ADJ_PDR");
                JR_DEBUG_RAW(pr, 10, 1, x, "OB_R_COR");
                JR_DEBUG_RAW(pr, 9, 1, x, "OB_SEL_RCTRL");
                JR_DEBUG_HSIO_BIT(pr, SERDES6G_ANA_CFG_SERDES6G_OB_CFG, OB_SR_H, x);
                JR_DEBUG_RAW(pr, 4, 4, x, "OB_RESISTOR_CTRL");
                JR_DEBUG_RAW(pr, 0, 4, x, "OB_SR");
                pr("\n%s:OB_CFG1:\n", buf);
                JR_RD(MACRO_CTRL, SERDES6G_ANA_CFG_SERDES6G_OB_CFG1, &x);
                JR_DEBUG_HSIO_FLD(pr, SERDES6G_ANA_CFG_SERDES6G_OB_CFG1, OB_ENA_CAS, x);
                JR_DEBUG_HSIO_FLD(pr, SERDES6G_ANA_CFG_SERDES6G_OB_CFG1, OB_LEV, x);

                pr("\n%s:DES_CFG:\n", buf);
                JR_RD(MACRO_CTRL, SERDES6G_ANA_CFG_SERDES6G_DES_CFG, &x);
                JR_DEBUG_HSIO_FLD(pr, SERDES6G_ANA_CFG_SERDES6G_DES_CFG, DES_PHS_CTRL, x);
                JR_DEBUG_HSIO_FLD(pr, SERDES6G_ANA_CFG_SERDES6G_DES_CFG, DES_MBTR_CTRL, x);
                JR_DEBUG_HSIO_FLD(pr, SERDES6G_ANA_CFG_SERDES6G_DES_CFG, DES_CPMD_SEL, x);
                JR_DEBUG_HSIO_FLD(pr, SERDES6G_ANA_CFG_SERDES6G_DES_CFG, DES_BW_HYST, x);
                JR_DEBUG_RAW(pr, 4, 1, x, "DES_SWAP_HYST");
                JR_DEBUG_HSIO_FLD(pr, SERDES6G_ANA_CFG_SERDES6G_DES_CFG, DES_BW_ANA, x);
                JR_DEBUG_RAW(pr, 0, 1, x, "DES_SWAP_ANA");

                pr("\n%s:IB_CFG:\n", buf);
                JR_RD(MACRO_CTRL, SERDES6G_ANA_CFG_SERDES6G_IB_CFG, &x);
                JR_DEBUG_RAW(pr, 28, 3, x, "ACJTAG_HYST");
                JR_DEBUG_HSIO_FLD(pr, SERDES6G_ANA_CFG_SERDES6G_IB_CFG, IB_IC_AC, x);
                JR_DEBUG_HSIO_FLD(pr, SERDES6G_ANA_CFG_SERDES6G_IB_CFG, IB_IC_COM, x);
                JR_DEBUG_HSIO_FLD(pr, SERDES6G_ANA_CFG_SERDES6G_IB_CFG, IB_IC_DC, x);
                JR_DEBUG_RAW(pr, 18, 1, x, "IB_R_COR");
                JR_DEBUG_HSIO_FLD(pr, SERDES6G_ANA_CFG_SERDES6G_IB_CFG, IB_RF, x);
                JR_DEBUG_HSIO_FLD(pr, SERDES6G_ANA_CFG_SERDES6G_IB_CFG, IB_RT, x);
                JR_DEBUG_RAW(pr, 7, 3, x, "IB_VBAC");
                JR_DEBUG_HSIO_FLD(pr, SERDES6G_ANA_CFG_SERDES6G_IB_CFG, IB_VBCOM, x);
                JR_DEBUG_HSIO_FLD(pr, SERDES6G_ANA_CFG_SERDES6G_IB_CFG, IB_RESISTOR_CTRL, x);

                pr("\n%s:IB_CFG1:\n", buf);
                JR_RD(MACRO_CTRL, SERDES6G_ANA_CFG_SERDES6G_IB_CFG1, &x);
                JR_DEBUG_RAW(pr, 12, 2, x, "IB_C_OFF");
                JR_DEBUG_HSIO_FLD(pr, SERDES6G_ANA_CFG_SERDES6G_IB_CFG1, IB_C, x);
                JR_DEBUG_RAW(pr, 7, 1, x, "IB_CHF");
                JR_DEBUG_RAW(pr, 6, 1, x, "IB_ANEG_MODE");
                JR_DEBUG_HSIO_BIT(pr, SERDES6G_ANA_CFG_SERDES6G_IB_CFG1, IB_CTERM_ENA, x);
                JR_DEBUG_HSIO_BIT(pr, SERDES6G_ANA_CFG_SERDES6G_IB_CFG1, IB_DIS_EQ, x);
                JR_DEBUG_HSIO_BIT(pr, SERDES6G_ANA_CFG_SERDES6G_IB_CFG1, IB_ENA_OFFSAC, x);
                JR_DEBUG_HSIO_BIT(pr, SERDES6G_ANA_CFG_SERDES6G_IB_CFG1, IB_ENA_OFFSDC, x);
                JR_DEBUG_HSIO_BIT(pr, SERDES6G_ANA_CFG_SERDES6G_IB_CFG1, IB_FX100_ENA, x);
                JR_DEBUG_HSIO_BIT(pr, SERDES6G_ANA_CFG_SERDES6G_IB_CFG1, IB_RST, x);
                
                pr("\n%s:SER_CFG:\n", buf);
                JR_RD(MACRO_CTRL, SERDES6G_ANA_CFG_SERDES6G_SER_CFG, &x);
                JR_DEBUG_RAW(pr, 8, 1, x, "SER_4TAP_ENA");
                JR_DEBUG_RAW(pr, 7, 1, x, "SER_CPMD_SEL");
                JR_DEBUG_RAW(pr, 6, 1, x, "SER_SWAP_CPMD");
                JR_DEBUG_HSIO_FLD(pr, SERDES6G_ANA_CFG_SERDES6G_SER_CFG, SER_ALISEL, x);
                JR_DEBUG_HSIO_BIT(pr, SERDES6G_ANA_CFG_SERDES6G_SER_CFG, SER_ENHYS, x);
                JR_DEBUG_RAW(pr, 2, 1, x, "SER_BIG_WIN");
                JR_DEBUG_HSIO_BIT(pr, SERDES6G_ANA_CFG_SERDES6G_SER_CFG, SER_EN_WIN, x);
                JR_DEBUG_HSIO_BIT(pr, SERDES6G_ANA_CFG_SERDES6G_SER_CFG, SER_ENALI, x);
                
                pr("\n%s:PLL_CFG:\n", buf);
                JR_RD(MACRO_CTRL, SERDES6G_ANA_CFG_SERDES6G_PLL_CFG, &x);
                JR_DEBUG_HSIO_BIT(pr, SERDES6G_ANA_CFG_SERDES6G_PLL_CFG, PLL_DIV4, x);
                JR_DEBUG_HSIO_BIT(pr, SERDES6G_ANA_CFG_SERDES6G_PLL_CFG, PLL_ENA_ROT, x);
                JR_DEBUG_HSIO_FLD(pr, SERDES6G_ANA_CFG_SERDES6G_PLL_CFG, PLL_FSM_CTRL_DATA, x);
                JR_DEBUG_HSIO_BIT(pr, SERDES6G_ANA_CFG_SERDES6G_PLL_CFG, PLL_FSM_ENA, x);
                JR_DEBUG_HSIO_BIT(pr, SERDES6G_ANA_CFG_SERDES6G_PLL_CFG, PLL_ROT_DIR, x);
                JR_DEBUG_HSIO_BIT(pr, SERDES6G_ANA_CFG_SERDES6G_PLL_CFG, PLL_ROT_FRQ, x);

                pr("\n%s:COMMOM_CFG:\n", buf);
                JR_RD(MACRO_CTRL, SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG, &x);
                JR_DEBUG_HSIO_BIT(pr, SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG, SYS_RST, x);
                JR_DEBUG_HSIO_BIT(pr, SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG, ENA_LANE, x);
                JR_DEBUG_RAW(pr, 17, 1, x, "PWD_RX");
                JR_DEBUG_RAW(pr, 16, 1, x, "PWD_TX");
                JR_DEBUG_HSIO_BIT(pr, SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG, HRATE, x);
                JR_DEBUG_HSIO_BIT(pr, SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG, QRATE, x);
                JR_DEBUG_HSIO_FLD(pr, SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG, IF_MODE, x);
                
                pr("\nVAUI_CHANNEL_CFG_%u:\n", lane/count);
                JR_RDX(MACRO_CTRL, VAUI_CHANNEL_CFG_VAUI_CHANNEL_CFG, lane/count, &x);
                JR_DEBUG_HSIO_FLD(pr, VAUI_CHANNEL_CFG_VAUI_CHANNEL_CFG, LANE_SYNC_ENA, x);

                pr("\n%s:MISC_CFG:\n", buf);
                JR_RD(MACRO_CTRL, SERDES6G_DIG_CFG_SERDES6G_MISC_CFG, &x);
                JR_DEBUG_HSIO_BIT(pr, SERDES6G_DIG_CFG_SERDES6G_MISC_CFG, LANE_RST, x);
                JR_DEBUG_HSIO_BIT(pr, SERDES6G_DIG_CFG_SERDES6G_MISC_CFG, DES_100FX_CPMD_ENA, x);
            }
            pr("\n");
        }
    }
    return VTSS_RC_OK;
}

static vtss_rc jr_debug_port(vtss_state_t *vtss_state,
                             const vtss_debug_printf_t pr,
                             const vtss_debug_info_t   *const info)
{
    vtss_port_no_t port_no;
    u32            value, port;
    char           buf[32];
    
    pr("Target  : 0x%04x/0x%04x\n", vtss_state->create.target, vtss_state->misc.chip_id.part_number);
    JR_RD(DEVCPU_GCB, VAUI2_MUX_VAUI2_MUX, &value);
    pr("Mux Mode: %u/%u\n\n", vtss_state->port.mux_mode[vtss_state->chip_no], value);

    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
        if (vtss_jr1_debug_port_skip(vtss_state, port_no, info))
            continue;
        port = VTSS_CHIP_PORT(port_no);
        sprintf(buf, "Port %u (%u)", port, port_no);
        vtss_jr1_debug_reg_header(pr, buf);
        VTSS_RC(jr_debug_chip_port(vtss_state, pr, info, port));
    } /* Port loop */

#ifdef VTSS_FEATURE_EEE
    if (vtss_state->eee.ena_cnt) {
        sprintf(buf, "EEE Loopback port %u", vtss_state->eee.loopback_chip_port[vtss_state->chip_no]);
        vtss_jr1_debug_reg_header(pr, buf);
        VTSS_RC(jr_debug_chip_port(vtss_state, pr, info, vtss_state->eee.loopback_chip_port[vtss_state->chip_no]));
    }
#endif

    if (vtss_state->chip_count == 2) {
        /* Internal ports */
        for (port = vtss_state->port.port_int_0; port <= vtss_state->port.port_int_1; port++) {
            sprintf(buf, "Internal port %u", port);
            vtss_jr1_debug_reg_header(pr, buf);
            VTSS_RC(jr_debug_chip_port(vtss_state, pr, info, port));
        }
    }
    pr("\n");

    return VTSS_RC_OK;
}

static vtss_rc jr_debug_port_counters(vtss_state_t *vtss_state,
                                      const vtss_debug_printf_t pr, 
                                      const vtss_debug_info_t   *const info, u32 port)
{
    u32                      i, prio, qu, val1;
    char                     buf[32];
    vtss_port_jr1_counters_t cnt;

    if (jr_port_counters_chip(vtss_state, port, &cnt, NULL, 0) != VTSS_RC_OK)
        return VTSS_RC_ERROR;

    vtss_jr1_debug_cnt(vtss_state, pr, "ok_bytes", "out_bytes", &cnt.rx_ok_bytes, &cnt.tx_out_bytes);
    vtss_jr1_debug_cnt(vtss_state, pr, "uc", "", &cnt.rx_unicast, &cnt.tx_unicast);
    vtss_jr1_debug_cnt(vtss_state, pr, "mc", "", &cnt.rx_multicast, &cnt.tx_multicast);
    vtss_jr1_debug_cnt(vtss_state, pr, "bc", "", &cnt.rx_broadcast, &cnt.tx_broadcast);

    if (!info->full) {
        pr("\n");
        return VTSS_RC_OK;
    }
    
    vtss_jr1_debug_cnt(vtss_state, pr, "pause", "", &cnt.rx_pause, &cnt.tx_pause);
    vtss_jr1_debug_cnt(vtss_state, pr, "64", "", &cnt.rx_size64, &cnt.tx_size64);
    vtss_jr1_debug_cnt(vtss_state, pr, "65_127", "", &cnt.rx_size65_127, &cnt.tx_size65_127);
    vtss_jr1_debug_cnt(vtss_state, pr, "128_255", "", &cnt.rx_size128_255, &cnt.tx_size128_255);
    vtss_jr1_debug_cnt(vtss_state, pr, "256_511", "", &cnt.rx_size256_511, &cnt.tx_size256_511);
    vtss_jr1_debug_cnt(vtss_state, pr, "512_1023", "", &cnt.rx_size512_1023, &cnt.tx_size512_1023);
    vtss_jr1_debug_cnt(vtss_state, pr, "1024_1526", "", &cnt.rx_size1024_1518, &cnt.tx_size1024_1518);
    vtss_jr1_debug_cnt(vtss_state, pr, "jumbo", "", &cnt.rx_size1519_max, &cnt.tx_size1519_max);
    vtss_jr1_debug_cnt(vtss_state, pr, "crc", NULL, &cnt.rx_crc_err, NULL);
    vtss_jr1_debug_cnt(vtss_state, pr, "local_drops", NULL, &cnt.rx_local_drops, NULL);
    
    for (i = 0; i < VTSS_PRIOS; i++) {
        sprintf(buf, "drops_%u", i);
        vtss_jr1_debug_cnt(vtss_state, pr, buf, "", &cnt.rx_queue_drops[i], &cnt.tx_queue_drops[i]);
    }
    for (i = 0; i < VTSS_PRIOS; i++) {
        sprintf(buf, "tx_drops_%u", i);
        vtss_jr1_debug_cnt(vtss_state, pr, buf, NULL, &cnt.rx_txqueue_drops[i], NULL);
    }
    for (i = 0; i < VTSS_PRIOS; i++) {
        sprintf(buf, "pol_drops_%u", i);
        vtss_jr1_debug_cnt(vtss_state, pr, buf, NULL, &cnt.rx_policer_drops[i], NULL);
    }
    for (i = 0; i < VTSS_PRIOS; i++) {
        sprintf(buf, "green_%u", i);
        vtss_jr1_debug_cnt(vtss_state, pr, buf, "", &cnt.rx_green_class[i], &cnt.tx_green_class[i]);
    }
    for (i = 0; i < VTSS_PRIOS; i++) {
        sprintf(buf, "yellow_%u", i);
        vtss_jr1_debug_cnt(vtss_state, pr, buf, "", &cnt.rx_yellow_class[i], &cnt.tx_yellow_class[i]);
    }
    for (i = 0; i < VTSS_PRIOS; i++) {
        sprintf(buf, "red_%u", i);
        vtss_jr1_debug_cnt(vtss_state, pr, buf, NULL, &cnt.rx_red_class[i], NULL);
    }
    pr("\n");
    
    pr("IQS/OQS Stickies (drops are explained):\n");
    for (prio = 0; prio < 8; prio++) {
        qu = port * 8 + prio;  
        JR_RDX(IQS, STAT_CNT_CFG_RX_STAT_EVENTS_STICKY, qu, &val1);
        if (val1 > 0) {
            pr("Port:%u Queue:%u. IQS RX Sticky:0x%x",port,prio,val1);
            if (val1>>0 & 1) 
                pr(" ->Failed frame");
            if (val1>>1 & 1) 
                pr(" ->due to buffer resource ");
            if (val1>>2 & 1) 
                pr(" ->due to prio resource ");
            if (val1>>3 & 1) 
                pr(" ->due to port resource ");
            if (val1>>4 & 1) 
                pr(" ->due to queue resource ");
            if (val1>>5 & 1) 
                pr(" ->due to RED ");
            if (val1>>6 & 1) 
                pr(" ->due to Q disabled ");
            if (val1>>7 & 1) 
                pr(" ->frame is to long ");
            pr("\n");
        }

        JR_RDX(IQS, STAT_CNT_CFG_TX_STAT_EVENTS_STICKY, qu, &val1);
        if (val1 > 0) {
            pr("Port:%u Queue:%u. IQS TX Sticky:0x%x",port,prio,val1);
            if (val1>>0 & 1) 
                pr(" ->Failed frame");
            if (val1>>1 & 1) 
                pr(" ->due to aging ");
            if (val1>>2 & 1) 
                pr(" ->due to Q flush ");
            if (val1>>3 & 1) 
                pr(" ->due to full OQS ");
            pr("\n");
        }

        JR_RDX(OQS, STAT_CNT_CFG_RX_STAT_EVENTS_STICKY, qu, &val1);
        if (val1 > 0) {
            pr("Port:%u Queue:%u. OQS RX Sticky:0x%x",port,prio,val1);
            if (val1>>0 & 1) 
                pr(" ->Failed frame");
            if (val1>>1 & 1) 
                pr(" ->due to buffer resource ");
            if (val1>>2 & 1) 
                pr(" ->due to prio resource ");
            if (val1>>3 & 1) 
                pr(" ->due to port resource ");
            if (val1>>4 & 1) 
                pr(" ->due to queue resource ");
            if (val1>>5 & 1) 
                pr(" ->due to RED ");
            if (val1>>6 & 1) 
                pr(" ->due to Q disabled ");
            if (val1>>7 & 1) 
                pr(" ->frame is to long ");
            pr("\n");
        }

        JR_RDX(OQS, STAT_CNT_CFG_TX_STAT_EVENTS_STICKY, qu, &val1);
        if (val1 > 0) {
            pr("Port:%u Queue:%u. OQS TX Sticky:0x%x",port,prio,val1);
            if (val1>>0 & 1) 
                pr(" ->Failed frame");
            if (val1>>1 & 1) 
                pr(" ->due to aging ");
            if (val1>>2 & 1) 
                pr(" ->due to Q flush ");
            if (val1>>3 & 1) 
                pr(" ->due to full OQS ");
            pr("\n");
        }

        JR_WRX(IQS, STAT_CNT_CFG_RX_STAT_EVENTS_STICKY, qu, 0xFFFFFFFF);
        JR_WRX(IQS, STAT_CNT_CFG_TX_STAT_EVENTS_STICKY, qu, 0xFFFFFFFF);
        JR_WRX(OQS, STAT_CNT_CFG_RX_STAT_EVENTS_STICKY, qu, 0xFFFFFFFF);
        JR_WRX(OQS, STAT_CNT_CFG_TX_STAT_EVENTS_STICKY, qu, 0xFFFFFFFF);


        /* jr_debug_sticky_inst(pr, VTSS_IQS_STAT_CNT_CFG_RX_STAT_EVENTS_STICKY(qu),  */
        /*                      prio, "IQS_RX_STICKY");  */
        /* jr_debug_sticky_inst(pr, VTSS_IQS_STAT_CNT_CFG_TX_STAT_EVENTS_STICKY(qu),  */
        /*                      prio, "IQS_TX_STICKY");  */
        /* jr_debug_sticky_inst(pr, VTSS_OQS_STAT_CNT_CFG_RX_STAT_EVENTS_STICKY(qu),  */
        /*                      prio, "OQS_RX_STICKY");  */
        /* jr_debug_sticky_inst(pr, VTSS_OQS_STAT_CNT_CFG_TX_STAT_EVENTS_STICKY(qu),  */
        /*                      prio, "OQS_TX_STICKY");  */
    }
    return VTSS_RC_OK;
}

static vtss_rc jr_debug_port_cnt(vtss_state_t *vtss_state,
                                 const vtss_debug_printf_t pr,
                                 const vtss_debug_info_t   *const info)
{
    /*lint --e{454, 455} */ // Due to VTSS_EXIT_ENTER
    vtss_port_no_t port_no;
    u32            port;
    
    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
        if (vtss_jr1_debug_port_skip(vtss_state, port_no, info))
            continue;
        VTSS_EXIT_ENTER();
        port = VTSS_CHIP_PORT(port_no);
        pr("Counters for port: %u (chip_port: %u):\n\n", port_no, port);
        (void)jr_debug_port_counters(vtss_state, pr, info, port);
    }

#ifdef VTSS_FEATURE_EEE
    if (vtss_state->eee.ena_cnt) {
        pr("Counters for EEE Loopback port %u:\n\n", vtss_state->eee.loopback_chip_port[vtss_state->chip_no]);
        (void)jr_debug_port_counters(vtss_state, pr, info, vtss_state->eee.loopback_chip_port[vtss_state->chip_no]);
    }
#endif

    if (vtss_state->chip_count == 2) {
        /* Internal port counters */
        for (port = vtss_state->port.port_int_0; port <= vtss_state->port.port_int_1; port++) {
            pr("Counters for internal port %u:\n\n", port);
            (void)jr_debug_port_counters(vtss_state, pr, info, port);
        }
    }
    pr("\n");
    
    return VTSS_RC_OK;
}

static void jr_debug_wm_dump(const vtss_debug_printf_t pr, const char *reg_name, u32 *value, u32 i) {
    u32 q;
    pr("%-25s",reg_name);
    for (q = 0; q <i; q++) {
        pr("%6u",value[q]);
    }
    pr("\n");
}
static vtss_rc jr_debug_wm(vtss_state_t *vtss_state,
                           const vtss_debug_printf_t pr,
                           const vtss_debug_info_t   *const info)
{
    u32 rc_cfg[8], rc_hlwm_cfg[8], rc_atop_cfg[8], rc_gwm_cfg[8];
    u32 fwd_press[8], cnt_on_prio_lvl[8], cnt_on_port_lvl[8],  cnt_on_buf_prio_lvl[8];
    u32 cnt_on_buf_port_lvl[8], cnt_on_buf_lvl[8];
    u32 rc_mode[8], drop_mode[8], enabled[8], id[8], q, qs;
    u32 swm[8], gwm[8], hwm[8], lwm[8], atop[8], mtu_map[8], mtu[4], cmwm[8];
    u32 wm_stop, rate_ctrl, rx_pause, pause_gen, port_no, gap, wm_high, wm_low, qu, port_count, queues, chipport;
    BOOL is_mac;

    /* Run through IQS and OQS, and dump the register fields */
    is_mac = (vtss_state->create.target == VTSS_TARGET_CE_MAX_24 ||
              vtss_state->create.target == VTSS_TARGET_CE_MAX_12);

    port_count = (is_mac ? 56 : (info->full ? 89 : VTSS_CHIP_PORTS));
    for (chipport = 0; chipport < port_count; chipport++) {
        queues = 8;

        if (is_mac) {
            pr("WM for port %u:", chipport);
        } else {
            if (vtss_state->chip_count == 2 && 
                (chipport == vtss_state->port.port_int_0 || chipport == vtss_state->port.port_int_1)) {
                pr("WM for internal port: %u", chipport);
            } else if (chipport < VTSS_CHIP_PORTS) {
                /* Normal port, check if port is enabled */
                for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
                    if (VTSS_CHIP_PORT(port_no) == chipport &&
                        VTSS_PORT_CHIP_SELECTED(port_no) &&
                        info->port_list[port_no]) {
                        break;
                    }
                }
                if (port_no == vtss_state->port_count)
                    continue;
                pr("WM for port %u: (%u)", chipport, port_no);
            } else if (chipport == 32) {
                pr("WM for IQS Virtual device:%u", chipport);                
            } else if (chipport == 33) {
                pr("WM for IQS CPU port :%u", chipport);
            } else if (chipport == 34) {
                pr("WM for IQS CPU port :%u", chipport);
            } else if (chipport == 80) {
                pr("WM for OQS Virtual device:%u", chipport);                
            } else if (chipport > 80) {
                pr("WM for OQS CPU port :%u", chipport);
                queues = 1;
            } else {
                continue;
            } 
        }
        pr("\n==========================================================================\n");  

        for (qs = 0; qs<2; qs++) { /* qs=0 -> IQS  qs=1 -> OQS   */
            if (is_mac) {
                if ((qs == 0) && (chipport > 31)) {
                    continue;
                }
            } else {
                if ((qs == 0) && (chipport > 34)) {
                    continue;
                }
            }
            /* Queue Level */    
            for(q=0;q<4;q++) {
                id[q] = q;
                RDX_SQS(qs, MTU_MTU_FRM_SIZE_CFG, q, &mtu[q]);                
            }
            pr("\nMTU::%s:\n",qs ? "OQS" : "IQS" );  
            pr("-------\n");          
            jr_debug_wm_dump(pr, "MTU id:", id, 4);
            jr_debug_wm_dump(pr, "MTU size:",  mtu, 4);
            
            for (q = 0; q <queues; q++) {
                qu = vtss_jr1_port_prio_2_qu(vtss_state, chipport, q, qs);
                RDX_SQS(qs, QU_RAM_CFG_QU_RC_CFG, qu, &rc_cfg[q]);
                RDX_SQS(qs, QU_RAM_CFG_QU_RC_HLWM_CFG, qu, &rc_hlwm_cfg[q]);
                RDX_SQS(qs, QU_RAM_CFG_QU_RC_ATOP_SWM_CFG, qu, &rc_atop_cfg[q]);
                RDX_SQS(qs, QU_RAM_CFG_MTU_QU_MAP_CFG, qu, &mtu_map[q]);
            }
        
            for (q = 0; q <queues; q++) {
                id[q] = q;
                fwd_press[q]           = PST_SQS(VTSS_X, qs, QU_RAM_CFG_QU_RC_CFG_FWD_PRESS_THRES(rc_cfg[q]));
                cnt_on_prio_lvl[q]     = PST_SQS(VTSS_F, qs, QU_RAM_CFG_QU_RC_CFG_QU_CNT_ON_PRIO_LVL_ENA) & rc_cfg[q] ? 1 : 0;
                cnt_on_port_lvl[q]     = PST_SQS(VTSS_F, qs, QU_RAM_CFG_QU_RC_CFG_QU_CNT_ON_PORT_LVL_ENA) & rc_cfg[q] ? 1 : 0;
                cnt_on_buf_prio_lvl[q] = PST_SQS(VTSS_F, qs, QU_RAM_CFG_QU_RC_CFG_QU_CNT_ON_BUF_PRIO_LVL_ENA) & rc_cfg[q] ? 1 : 0;
                cnt_on_buf_port_lvl[q] = PST_SQS(VTSS_F, qs, QU_RAM_CFG_QU_RC_CFG_QU_CNT_ON_BUF_PORT_LVL_ENA) & rc_cfg[q] ? 1 : 0;
                rc_mode[q]             = PST_SQS(VTSS_F, qs, QU_RAM_CFG_QU_RC_CFG_QU_RC_MODE) & rc_cfg[q] ? 1 : 0;
                drop_mode[q]           = PST_SQS(VTSS_F, qs, QU_RAM_CFG_QU_RC_CFG_QU_FC_DROP_MODE) & rc_cfg[q] ? 1 : 0;
                enabled[q]             = PST_SQS(VTSS_F, qs, QU_RAM_CFG_QU_RC_CFG_RX_QU_ENA) & rc_cfg[q] ? 1 : 0;
                hwm[q]                 = PST_SQS(VTSS_X, qs, QU_RAM_CFG_QU_RC_HLWM_CFG_QU_RC_PROFILE_HWM(rc_hlwm_cfg[q]));
                lwm[q]                 = PST_SQS(VTSS_X, qs, QU_RAM_CFG_QU_RC_HLWM_CFG_QU_RC_PROFILE_LWM(rc_hlwm_cfg[q]));
                atop[q]                = PST_SQS(VTSS_X, qs, QU_RAM_CFG_QU_RC_ATOP_SWM_CFG_QU_RC_PROFILE_ATOP(rc_atop_cfg[q]));
                swm[q]                 = PST_SQS(VTSS_X, qs, QU_RAM_CFG_QU_RC_ATOP_SWM_CFG_QU_RC_PROFILE_SWM(rc_atop_cfg[q]));
            }
            pr("\nQueue level::%s:\n",qs ? "OQS" : "IQS" );  
            pr("--------------\n");  
            jr_debug_wm_dump(pr, "Queue:", id, queues);
            jr_debug_wm_dump(pr, "QU_ENA:",  enabled, queues);
            jr_debug_wm_dump(pr, "FWD_PRESS_THRES:", fwd_press, queues);
            jr_debug_wm_dump(pr, "CNT_ON_PRIO_LVL_ENA:", cnt_on_prio_lvl, queues);
            jr_debug_wm_dump(pr, "CNT_ON_PORT_LVL_ENA:", cnt_on_port_lvl, queues);
            jr_debug_wm_dump(pr, "CNT_ON_BUF_PRIO_LVL_ENA:",  cnt_on_buf_prio_lvl, queues);
            jr_debug_wm_dump(pr, "CNT_ON_BUF_PORT_LVL_ENA:",  cnt_on_buf_port_lvl, queues);
            jr_debug_wm_dump(pr, "RC_MODE:",  rc_mode, queues);
            jr_debug_wm_dump(pr, "DROP_MODE:",  drop_mode, queues);
            jr_debug_wm_dump(pr, "ATOP:",  atop, queues);
            jr_debug_wm_dump(pr, "HWM:",  hwm, queues);
            jr_debug_wm_dump(pr, "LWM:",  lwm, queues);
            jr_debug_wm_dump(pr, "SWM:",  swm, queues);
            jr_debug_wm_dump(pr, "MTU map:",  mtu_map, queues);

            /* Port Level */      
            for (q = 0; q <queues; q++) {
                if (chipport+q > port_count)
                    break;
                RDX_SQS(qs, PORT_RAM_CFG_PORT_RC_CFG, chipport+q, &rc_cfg[q]); 
                RDX_SQS(qs, PORT_RAM_CFG_PORT_RC_HLWM_CFG, chipport+q, &rc_hlwm_cfg[q]);
                RDX_SQS(qs, PORT_RAM_CFG_PORT_RC_ATOP_SWM_CFG, chipport+q, &rc_atop_cfg[q]);
                RDX_SQS(qs, PORT_RAM_CFG_PORT_RC_GWM_CFG, chipport+q, &rc_gwm_cfg[q]);
                if (qs == 1) {
                    JR_RDX(OQS, PORT_RAM_CFG_CM_PORT_WM, chipport, &cmwm[q]);
                } 
            }
        
            for (q = 0; q <queues; q++) {
                if (chipport+q > port_count)
                    break;
                id[q] = chipport+q;
                enabled[q]        = PST_SQS(VTSS_F, qs, PORT_RAM_CFG_PORT_RC_CFG_RX_PORT_ENA) & rc_cfg[q] ? 1 : 0;       
                cnt_on_buf_lvl[q] = PST_SQS(VTSS_F, qs, PORT_RAM_CFG_PORT_RC_CFG_PORT_CNT_ON_BUF_LVL_ENA) & rc_cfg[q] ? 1 : 0;
                drop_mode[q]      = PST_SQS(VTSS_F, qs, PORT_RAM_CFG_PORT_RC_CFG_PORT_FC_DROP_MODE) & rc_cfg[q] ? 1 : 0;
                rc_mode[q]        = PST_SQS(VTSS_F, qs, PORT_RAM_CFG_PORT_RC_CFG_PORT_RC_MODE) & rc_cfg[q] ? 1 : 0;
                hwm[q]            = PST_SQS(VTSS_X, qs, PORT_RAM_CFG_PORT_RC_HLWM_CFG_PORT_RC_PROFILE_HWM(rc_hlwm_cfg[q]));
                lwm[q]            = PST_SQS(VTSS_X, qs, PORT_RAM_CFG_PORT_RC_HLWM_CFG_PORT_RC_PROFILE_LWM(rc_hlwm_cfg[q]));
                atop[q]           = PST_SQS(VTSS_X, qs, PORT_RAM_CFG_PORT_RC_ATOP_SWM_CFG_PORT_RC_PROFILE_ATOP(rc_atop_cfg[q]));   
                swm[q]            = PST_SQS(VTSS_X, qs, PORT_RAM_CFG_PORT_RC_ATOP_SWM_CFG_PORT_RC_PROFILE_SWM(rc_atop_cfg[q]));   
                gwm[q]            = PST_SQS(VTSS_X, qs, PORT_RAM_CFG_PORT_RC_ATOP_SWM_CFG_PORT_RC_PROFILE_SWM(rc_gwm_cfg[q]));   
            }
            
        
            pr("\nPort Level::%s:\n",qs ? "OQS" : "IQS" );  
            pr("--------------\n");  
            jr_debug_wm_dump(pr, "QQS Port:", id, queues);
            jr_debug_wm_dump(pr, "PORT_ENA:", enabled, queues);
            jr_debug_wm_dump(pr, "CNT_ON_BUF_LVL:", cnt_on_buf_lvl, queues);
            jr_debug_wm_dump(pr, "DROP_MODE:", drop_mode, queues);
            jr_debug_wm_dump(pr, "RC_MODE:", rc_mode, queues);
            jr_debug_wm_dump(pr, "GWM:", gwm, queues);
            jr_debug_wm_dump(pr, "ATOP:", atop, queues);
            jr_debug_wm_dump(pr, "HWM:", hwm, queues);
            jr_debug_wm_dump(pr, "LWM:", lwm, queues);
            jr_debug_wm_dump(pr, "SWM:", swm, queues);
            if (qs == 1) {
                jr_debug_wm_dump(pr, "CMWM:", cmwm, queues);
            }
               
            /* Priority Level */
            for (q = 0; q <queues; q++) {
                RDX_SQS(qs, RESOURCE_CTRL_CFG_PRIO_RC_CFG, q, &rc_cfg[q]);
                RDX_SQS(qs, RESOURCE_CTRL_CFG_PRIO_RC_HLWM_CFG, q, &rc_hlwm_cfg[q]);
                RDX_SQS(qs, RESOURCE_CTRL_CFG_PRIO_RC_ATOP_SWM_CFG, q, &rc_atop_cfg[q]);
            }
            
            for (q = 0; q <queues; q++) {
                id[q] = q;
                enabled[q]        = PST_SQS(VTSS_F, qs, RESOURCE_CTRL_CFG_PRIO_RC_CFG_RX_PRIO_ENA) & rc_cfg[q] ? 1 : 0;
                drop_mode[q]      = PST_SQS(VTSS_F, qs, RESOURCE_CTRL_CFG_PRIO_RC_CFG_PRIO_FC_DROP_MODE) & rc_cfg[q] ? 1 : 0;
                rc_mode[q]        = PST_SQS(VTSS_F, qs, RESOURCE_CTRL_CFG_PRIO_RC_CFG_PRIO_RC_MODE) & rc_cfg[q] ? 1 : 0;
                cnt_on_buf_lvl[q] = PST_SQS(VTSS_F, qs, RESOURCE_CTRL_CFG_PRIO_RC_CFG_PRIO_CNT_ON_BUF_LVL_ENA) & rc_cfg[q] ? 1 : 0;
                hwm[q]            = PST_SQS(VTSS_X, qs, RESOURCE_CTRL_CFG_PRIO_RC_HLWM_CFG_PRIO_RC_PROFILE_HWM(rc_hlwm_cfg[q]));
                lwm[q]            = PST_SQS(VTSS_X, qs, RESOURCE_CTRL_CFG_PRIO_RC_HLWM_CFG_PRIO_RC_PROFILE_LWM(rc_hlwm_cfg[q]));
                atop[q]           = PST_SQS(VTSS_X, qs, RESOURCE_CTRL_CFG_PRIO_RC_ATOP_SWM_CFG_PRIO_RC_PROFILE_ATOP(rc_atop_cfg[q]));
                swm[q]            = PST_SQS(VTSS_X, qs, RESOURCE_CTRL_CFG_PRIO_RC_ATOP_SWM_CFG_PRIO_RC_PROFILE_SWM(rc_atop_cfg[q]));
            }
            
            /* pr("\nPrio level::%s:\n",qs ? "OQS" : "IQS" );   */
            /* pr("--------------\n");   */
            /* jr_debug_wm_dump(pr, "Priority:", id, 8); */
            /* jr_debug_wm_dump(pr, "RX_PRIO_ENA:", enabled, 8); */
            /* jr_debug_wm_dump(pr, "CNT_ON_BUF_LVL_ENA:",  cnt_on_buf_lvl, 8); */
            /* jr_debug_wm_dump(pr, "DROP_MODE:",  drop_mode, 8); */
            /* jr_debug_wm_dump(pr, "RC_MODE:",  rc_mode, 8); */
            /* jr_debug_wm_dump(pr, "ATOP:", atop, 8); */
            /* jr_debug_wm_dump(pr, "HWM:", hwm, 8); */
            /* jr_debug_wm_dump(pr, "LWM:", lwm, 8); */
            /* jr_debug_wm_dump(pr, "SWM:", swm, 8); */
            
            /* Buffer Port Level */
            RD_SQS(qs, RESOURCE_CTRL_CFG_BUF_RC_CFG, &rc_cfg[0]);
            RD_SQS(qs, RESOURCE_CTRL_CFG_BUF_PORT_RC_HLWM_CFG, &rc_hlwm_cfg[0]);
            
            enabled[0]   = PST_SQS(VTSS_F, qs, RESOURCE_CTRL_CFG_BUF_RC_CFG_RX_BUF_ENA) & rc_cfg[0] ? 1 : 0;
            drop_mode[0] = PST_SQS(VTSS_F, qs, RESOURCE_CTRL_CFG_BUF_RC_CFG_BUF_PORT_FC_DROP_MODE) & rc_cfg[0] ? 1 : 0;
            atop[0]      = PST_SQS(VTSS_X, qs, RESOURCE_CTRL_CFG_BUF_RC_CFG_BUF_PORT_RC_PROFILE_ATOP(rc_cfg[0]));
            hwm[0]       = PST_SQS(VTSS_X, qs, RESOURCE_CTRL_CFG_BUF_PORT_RC_HLWM_CFG_BUF_PORT_RC_PROFILE_HWM(rc_hlwm_cfg[0]));
            lwm[0]       = PST_SQS(VTSS_X, qs, RESOURCE_CTRL_CFG_BUF_PORT_RC_HLWM_CFG_BUF_PORT_RC_PROFILE_LWM(rc_hlwm_cfg[0]));
            
            pr("\nBuf port level::%s:\n",qs ? "OQS" : "IQS" );  
            pr("-------------------\n");  
            jr_debug_wm_dump(pr, "RX_BUF_ENA:", enabled, 1);
            jr_debug_wm_dump(pr, "DROP_MODE:", drop_mode, 1);
            jr_debug_wm_dump(pr, "ATOP:", atop, 1);
            jr_debug_wm_dump(pr, "HWM:", hwm, 1);
            jr_debug_wm_dump(pr, "LWM:", lwm, 1);
            
            /* Buffer Prio Level */
            RD_SQS(qs, RESOURCE_CTRL_CFG_BUF_PRIO_RC_CFG, &rc_cfg[0]);
            RD_SQS(qs, RESOURCE_CTRL_CFG_BUF_PRIO_RC_HLWM_CFG, &rc_hlwm_cfg[0]);
            
            drop_mode[0] = PST_SQS(VTSS_F, qs, RESOURCE_CTRL_CFG_BUF_PRIO_RC_CFG_BUF_PRIO_FC_DROP_MODE) & rc_cfg[0] ? 1 : 0;
            atop[0]      = PST_SQS(VTSS_X, qs, RESOURCE_CTRL_CFG_BUF_PRIO_RC_CFG_BUF_PRIO_RC_PROFILE_ATOP(rc_cfg[0]));
            hwm[0]       = PST_SQS(VTSS_X, qs, RESOURCE_CTRL_CFG_BUF_PRIO_RC_HLWM_CFG_BUF_PRIO_RC_PROFILE_HWM(rc_hlwm_cfg[0]));
            lwm[0]       = PST_SQS(VTSS_X, qs, RESOURCE_CTRL_CFG_BUF_PRIO_RC_HLWM_CFG_BUF_PRIO_RC_PROFILE_LWM(rc_hlwm_cfg[0]));
        
            /* pr("\nBuf prio level::%s:\n",qs ? "OQS" : "IQS" );   */
            /* pr("-------------------\n");   */
            /* jr_debug_wm_dump(pr, "DROP_MODE:", drop_mode, 1); */
            /* jr_debug_wm_dump(pr, "ATOP:", atop, 1); */
            /* jr_debug_wm_dump(pr, "HWM:", hwm, 1); */
            /* jr_debug_wm_dump(pr, "LWM:", lwm, 1); */
            
            
            RDX_SQS(qs, PORT_RAM_CFG_PORT_RC_CFG, chipport, &rc_cfg[0]); 
            RDX_SQS(qs, PORT_RAM_CFG_PORT_RC_HLWM_CFG, chipport, &rc_hlwm_cfg[0]);
            RDX_SQS(qs, PORT_RAM_CFG_PORT_RC_ATOP_SWM_CFG, chipport, &rc_atop_cfg[0]);
            RDX_SQS(qs, PORT_RAM_CFG_PORT_RC_GWM_CFG, chipport, &rc_gwm_cfg[0]);
            
            id[0] = chipport;
            enabled[0]        = PST_SQS(VTSS_F, qs, PORT_RAM_CFG_PORT_RC_CFG_RX_PORT_ENA) & rc_cfg[0] ? 1 : 0;       
            cnt_on_buf_lvl[0] = PST_SQS(VTSS_F, qs, PORT_RAM_CFG_PORT_RC_CFG_PORT_CNT_ON_BUF_LVL_ENA) & rc_cfg[0] ? 1 : 0;
            drop_mode[0]      = PST_SQS(VTSS_F, qs, PORT_RAM_CFG_PORT_RC_CFG_PORT_FC_DROP_MODE) & rc_cfg[0] ? 1 : 0;
            rc_mode[0]        = PST_SQS(VTSS_F, qs, PORT_RAM_CFG_PORT_RC_CFG_PORT_RC_MODE) & rc_cfg[0] ? 1 : 0;
            hwm[0]            = PST_SQS(VTSS_X, qs, PORT_RAM_CFG_PORT_RC_HLWM_CFG_PORT_RC_PROFILE_HWM(rc_hlwm_cfg[0]));
            lwm[0]            = PST_SQS(VTSS_X, qs, PORT_RAM_CFG_PORT_RC_HLWM_CFG_PORT_RC_PROFILE_LWM(rc_hlwm_cfg[0]));
            atop[0]           = PST_SQS(VTSS_X, qs, PORT_RAM_CFG_PORT_RC_ATOP_SWM_CFG_PORT_RC_PROFILE_ATOP(rc_atop_cfg[0]));   
            swm[0]            = PST_SQS(VTSS_X, qs, PORT_RAM_CFG_PORT_RC_ATOP_SWM_CFG_PORT_RC_PROFILE_SWM(rc_atop_cfg[0]));   
            gwm[0]            = PST_SQS(VTSS_X, qs, PORT_RAM_CFG_PORT_RC_ATOP_SWM_CFG_PORT_RC_PROFILE_SWM(rc_gwm_cfg[0]));   
        } /* for qs=0 (IQS) and qs=1 (OQS) */        

        if (chipport < 32) {
            /* Flowcontrol Settings */
            JR_RDX(DSM, CFG_SCH_STOP_WM_CFG, chipport, &wm_stop);
            JR_RDX(DSM, CFG_RATE_CTRL, chipport, &rate_ctrl);
            JR_RDX(DSM, CFG_RX_PAUSE_CFG, chipport, &rx_pause);
            JR_RDX(DSM, CFG_ETH_FC_GEN, chipport, &pause_gen);
            
            wm_stop = JR_GET_FLD(DSM, CFG_SCH_STOP_WM_CFG, SCH_STOP_WM, wm_stop);
            gap = JR_GET_FLD(DSM, CFG_RATE_CTRL, FRM_GAP_COMP, rate_ctrl);
            wm_high = JR_GET_FLD(DSM, CFG_RATE_CTRL, TAXI_RATE_HIGH, rate_ctrl);
            wm_low = JR_GET_FLD(DSM, CFG_RATE_CTRL, TAXI_RATE_LOW, rate_ctrl);
            rx_pause = VTSS_F_DSM_CFG_RX_PAUSE_CFG_RX_PAUSE_EN & rx_pause ? 1 : 0;
            pause_gen = VTSS_F_DSM_CFG_ETH_FC_GEN_ETH_PORT_FC_GEN & pause_gen ? 1 : 0;
        
            pr("\nFC settings in DSM:\n");      
            pr("-------------------\n");      
            jr_debug_wm_dump(pr, "SCH_STOP_WM:", &wm_stop, 1);
            jr_debug_wm_dump(pr, "GAP:", &gap, 1);
            jr_debug_wm_dump(pr, "WM_HIGH:", &wm_high, 1);
            jr_debug_wm_dump(pr, "WM_LOW:", &wm_low, 1);
            jr_debug_wm_dump(pr, "RX_PAUSE_EN:", &rx_pause, 1);
            jr_debug_wm_dump(pr, "ETH_PAUSE_GEN:", &pause_gen, 1);
            pr("\n");
        }
    } /* For each port in the port list */
    
    return VTSS_RC_OK;
}

static vtss_rc jr_debug_host(vtss_state_t *vtss_state,
                             const vtss_debug_printf_t pr,
                             const vtss_debug_info_t   *const info)
{
#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC)
    u32                      hmda,hmdb,mask,val,pgid,prios,val1,val2;
    u32                      hm = vtss_jr1_ce_max_hm(vtss_state);
    vtss_port_no_t           port_no;
    vtss_lport_no_t          lport_no;
    u32                      i, oqs_ports, id, hport, qu;
    u64                      green, yellow, drops;
    
    pr("Host mode:%u (%s)\n",hm, (hm==0)?"12x8x2":(hm==1)?"24x4x2":(hm==2)?"24x8x1":(hm==3)?"48x4x1":"illegal");

    if (hm == 0 || hm == 2)
        prios = 8;
    else
        prios = 4;

    JR_RDF(ARB, CFG_STATUS_HM_CFG, ETH_DEV_HMDA, &hmda);
    JR_RDF(ARB, CFG_STATUS_HM_CFG, ETH_DEV_HMDB, &hmdb);
    pr("Host Interfaces HMDA:%u  HMDB:%u\n", hmda, hmdb);

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        lport_no = vtss_state->port.map[port_no].lport_no;
        if ((lport_no == VTSS_LPORT_MAP_NONE) || (lport_no == VTSS_LPORT_MAP_DEFAULT)) { /* No mapping - skip */
            continue;
        } 
        pr("Line port %u maps to Host port %u\n", port_no, lport_no);
    }

    vtss_debug_print_header(pr, "Line Port PGID table");
    pr("%-8s %-8s %-8s %-8s %-8s %-8s %-8s %-8s %-8s\n", "PGID","Stack","CPU","Queue","UPSID","UPSPN","HQ","HP","Mask");
    for (pgid = PGID_HQ_START; pgid < (PGID_HQ_START+(VTSS_PORTS*8)); pgid++) {
        if ((prios == 4) && (pgid % 2))
            continue;
            
        JR_RDX(ANA_AC, PGID_PGID_CFG_0, pgid, &mask);
        if (mask == 0)
            continue;

        JR_RDX(ANA_AC, PGID_PGID_CFG_1, pgid, &val);
        pr("%-8u %-8d %-8d %-8u %-8u %-8u %-8u %-8u 0x%-8x", pgid,
           JR_GET_BIT(ANA_AC, PGID_PGID_CFG_1, STACK_TYPE_ENA, val),
           JR_GET_BIT(ANA_AC, PGID_PGID_CFG_1, PGID_CPU_COPY_ENA, val),
           JR_GET_FLD(ANA_AC, PGID_PGID_CFG_1, PGID_CPU_QU, val),
           (mask >> 6 & 0x1F), (mask & 0x1F), ((mask >> 6 & 0x1F)*32 + (mask & 0x1F)), (((mask >> 6 & 0x1F)*32 + (mask & 0x1F))-32)/prios, mask);        
        pr("\n");
    }


    /* for (port_no = 0; port_no < 32; port_no++) { */

    /*     for (i = 0; i < prios; i++) { */
    /*         qu = vtss_jr1_port_prio_2_qu(port_no, i, 0); */

    /*         JR_CNT_HOST_SQS(IQS, RX, GREEN, qu, green);    /\* Green *\/ */
    /*         JR_CNT_HOST_SQS(IQS, RX, YELLOW , qu, yellow);  /\* Yellow *\/ */
    /*         JR_CNT_HOST_SQS(IQS, RX, QDROPS, qu, qdrops);   /\* SQS and RED drops *\/ */
    /*         JR_CNT_HOST_SQS(IQS, RX, PDROPS, qu, pdrops); /\* Policer drops *\/ */
    /*         JR_CNT_HOST_SQS(IQS, TX, TXDROPS, qu, txdrops);   /\* Full OQS drops *\/ */
        
    /*         if ((yellow+green+qdrops+txdrops+pdrops) > 0) { */
    /*             pr("IQS: Port %u Queue:%u RxFrames:%u Qdrops:%u, Pdrops:%u, Txdrops:%u\n",port_no,i,yellow+green,qdrops,pdrops,txdrops);  */
    /*         } */
    /*         if (qdrops+pdrops+txdrops+yellow+green > 0) { */
    /*             JR_RDX(IQS, STAT_CNT_CFG_RX_STAT_EVENTS_STICKY, qu, &val1); */
    /*             JR_RDX(IQS, STAT_CNT_CFG_TX_STAT_EVENTS_STICKY, qu, &val2); */
    /*             pr("IQS:(Qu:%u) Rx Sticky:0x%x   Tx Sticky:0x%x\n",qu, val1, val2);  */
    /*         } */
    /*         id++; */
    /*     } */
    /* }    */

    /* OQS counters */
    if (prios == 8)
        oqs_ports = 56;
    else
        oqs_ports = 80;
        
    id = 0;
    for (hport = 0; hport < oqs_ports-32; hport++) {
        vtss_lport_chip_counters_t *c = &vtss_state->port.lport_chip_counters[hport];
        for (i = 0; i < prios; i++) {
            qu = 256 + id;
            JR_CNT_HOST_SQS(OQS, TX, GREEN, qu,  &c->tx_green[i], 0);
            JR_CNT_HOST_SQS(OQS, TX, YELLOW, qu, &c->tx_yellow[i], 0);
            JR_CNT_HOST_SQS(OQS, RX, QDROPS, qu, &c->rx_drops[i], 0);
            green = c->tx_green[i].value;
            yellow = c->tx_yellow[i].value;
            drops = c->rx_drops[i].value;
            if ((yellow+green) > 0 || drops > 0) {
                pr("OQS: Host port %u (HQ:%u) Queue:%u TxFrames:%" PRIu64 " Drops:%" PRIu64 "\n",hport,qu,i,yellow+green,drops);
            }
            if (drops>0) {
                JR_RDX(OQS, STAT_CNT_CFG_RX_STAT_EVENTS_STICKY, qu, &val1);
                JR_RDX(OQS, STAT_CNT_CFG_TX_STAT_EVENTS_STICKY, qu, &val2);
                pr("OQS:(Qu:%u) Rx Sticky:0x%x   Tx Sticky:0x%x\n",qu, val1, val2);
            }
            id++;
        }
    }   
    pr("\n");

#endif  /* VTSS_ARCH_JAGUAR_1_CE_MAC */
    return VTSS_RC_OK;
}

vtss_rc vtss_jr1_port_debug_print(vtss_state_t *vtss_state,
                                  const vtss_debug_printf_t pr,
                                  const vtss_debug_info_t   *const info)
{
    VTSS_RC(vtss_debug_print_group(VTSS_DEBUG_GROUP_PORT, jr_debug_port, vtss_state, pr, info));
    VTSS_RC(vtss_debug_print_group(VTSS_DEBUG_GROUP_PORT_CNT, jr_debug_port_cnt, vtss_state, pr, info));
    VTSS_RC(vtss_debug_print_group(VTSS_DEBUG_GROUP_WM, jr_debug_wm, vtss_state, pr, info));
    VTSS_RC(vtss_debug_print_group(VTSS_DEBUG_GROUP_HOST, jr_debug_host, vtss_state, pr, info));
    return VTSS_RC_OK;
}

/* - Initialization ------------------------------------------------ */
static vtss_rc jr_init_hsio(vtss_state_t *vtss_state)
{
    vtss_port_mux_mode_t mux_mode = vtss_state->port.mux_mode[vtss_state->chip_no];
    vtss_target_type_t   target = jr_target_get(vtss_state);
    u32                  mask_sgmii = 0, mask_2g5 = 0, mask_xaui = 0;

    if (mux_mode == VTSS_PORT_MUX_MODE_0) {
        /* Multiplexer for interconnection between 10G/2G5 */
        JR_WR(DEVCPU_GCB, VAUI2_MUX_VAUI2_MUX, 0);
        /* Multiplexer for interconnection between 10G/2G5: 4x2G5 */
    } else if (mux_mode == VTSS_PORT_MUX_MODE_1) {
        JR_WR(DEVCPU_GCB, VAUI2_MUX_VAUI2_MUX, 1);
        /* Disable VAUI 0 Lane Sync Mechanism for 4 independent lanes... */
        JR_WRXF(MACRO_CTRL, VAUI_CHANNEL_CFG_VAUI_CHANNEL_CFG, 0, LANE_SYNC_ENA, 0);
    } else if (mux_mode == VTSS_PORT_MUX_MODE_7) {
        /* Multiplexer for interconnection between 10G/2G5: 8x2G5 */
        JR_WR(DEVCPU_GCB, VAUI2_MUX_VAUI2_MUX, 7);
        /* Disable VAUI 0 and 1 Lane Sync Mechanism for 4 independent lanes... */
        JR_WRXF(MACRO_CTRL, VAUI_CHANNEL_CFG_VAUI_CHANNEL_CFG, 0, LANE_SYNC_ENA, 0);
        JR_WRXF(MACRO_CTRL, VAUI_CHANNEL_CFG_VAUI_CHANNEL_CFG, 1, LANE_SYNC_ENA, 0);
    }

    switch (target) {
    case VTSS_TARGET_JAGUAR_1:
    case VTSS_TARGET_CE_MAX_24:
    case VTSS_TARGET_E_STAX_III_48:
    case VTSS_TARGET_E_STAX_III_68:
        if (mux_mode == VTSS_PORT_MUX_MODE_0) {
            /* 24x1G + 4x10G + NPI */            
            mask_sgmii = 0x1ffffff; /* SGMII mode (24 SGMII + 1 RGMII) */
            mask_xaui = 0xffff;     /* XAUI mode, lane 0-15 */
        } else if (mux_mode == VTSS_PORT_MUX_MODE_1) {
            /* 23x1G + 4x2g5 + 3x10G + NPI */
            mask_sgmii = 0x17fffff; /* SGMII mode (23 SGMII + 1 RGMII) */
            mask_2g5 = 0xf;         /* 2.5G mode, lane 0-3 */
            mask_xaui = 0xfff0;     /* XAUI mode, lane 4-15 */
        } else if (mux_mode == VTSS_PORT_MUX_MODE_7) {
            /* 19x1G + 8x2g5 + 2x10G + NPI */
            mask_sgmii = 0x107ffff; /* SGMII mode (19 SGMII + 1 RGMII) */
            mask_2g5 = 0xff;        /* 2.5G mode, lane 0-7 */
            mask_xaui = 0xff00;     /* XAUI mode, lane 8-15 */
        } else {
            VTSS_E("Mux mode no supported");
        }
        break;
    case VTSS_TARGET_LYNX_1:
    case VTSS_TARGET_CE_MAX_12:
        if (mux_mode == VTSS_PORT_MUX_MODE_0) {
            /* 12x1G + 2x10G + NPI */
            mask_sgmii = 0x17ff800; /* SGMII mode (11-22 SGMII + 1 RGMII) */
            mask_xaui = 0xff;       /* XAUI mode, lane 0-7 */
        } else if (mux_mode == VTSS_PORT_MUX_MODE_1) {
            /* 12x1G + 4x2g5 + 1x10g + NPI */
            mask_sgmii = 0x17ff800; /* SGMII mode, (11-22 SGMII + 1 RGMII) */
            mask_2g5 = 0xf;         /* 2.5G mode, lane 0-3 */
            mask_xaui = 0xf0;       /* XAUI mode, lane 4-7 */
        } else if (mux_mode == VTSS_PORT_MUX_MODE_7) {
            if (target == VTSS_TARGET_CE_MAX_12) {
                VTSS_E("Mux mode no supported");
                return VTSS_RC_ERROR;
            }
            /* 8x1G + 8x2g5 + NPI */
            mask_sgmii = 0x107f800; /* SGMII mode, (11-18 SGMII + 1 RGMII) */
            mask_2g5 = 0xf;         /* 2.5G mode, lane 0-3 */
        } else {
            VTSS_E("Mux mode no supported");
            return VTSS_RC_ERROR;
        }
        if (target == VTSS_TARGET_LYNX_1) {
            mask_xaui |= 0xff00;    /* XAUI mode, lane 8-15 */
        }
        break;
    default:
        VTSS_E("Serdes macro - Unknown Target");
        return VTSS_RC_ERROR;
    }
    
    if (mask_sgmii) {
        VTSS_RC(jr_sd1g_cfg(vtss_state, VTSS_SERDES_MODE_SGMII, mask_sgmii));
    }
    if (mask_2g5) {
        VTSS_RC(jr_sd6g_cfg(vtss_state, VTSS_SERDES_MODE_2G5, mask_2g5));
    }
    if (mask_xaui) {
        VTSS_RC(jr_sd6g_cfg(vtss_state, VTSS_SERDES_MODE_XAUI, mask_xaui));
    }
    return VTSS_RC_OK;
}

/* In general the role of the assembler is:                                        */
/* - Store frames from the taxi busses as cells and send them to the Analyzer      */
/* - Detect pause frames and signal to the DSM to pause further transmission       */
/* - Detect super priority frames and send them to the DSM for direct transmission */
/* - Flowcontrol the CPU                                                           */
/* - The calender must be formated with taxi buses based on port speeds */
static vtss_rc jr_init_asm(vtss_state_t *vtss_state)
{
    u32 cbc_len = 48, i;
    vtss_port_mux_mode_t mux_mode = vtss_state->port.mux_mode[vtss_state->chip_no];
    int taxi_c1[] = {5,4,3,5,4,0,5,4,1,5,3,4,5,0,4,5,1,2,5,4,3,5,4,2,
                     5,4,0,5,3,4,5,1,2,5,4,6,5,4,3,5,4,0,5,1,4,5,2,15};
    int taxi_c2[] = {5,3,4,5,0,3,5,4,1,5,3,2,5,4,3,5,0,1,5,4,3,5,2,6,
                     5,3,4,5,0,3,5,6,1,5,2,3,5,4,3,5,0,1,5,3,2,5,4,15};
    int taxi_c3[] = {5,3,0,5,3,1,5,3,2,5,3,6,5,3,0,5,3,1,5,3,2,5,3,6,
                     5,3,0,5,3,1,5,2,3,5,6,3,5,0,3,5,1,3,5,2,3,5,4/* <= EEE port 27 */,15};
    int taxi_c4[] = {4,5,3,4,5,0,4,5,1,4,3,5,4,0,5,4,1,2,4,5,3,4,5,2,
                     4,5,0,4,3,5,4,1,2,4,5,6,4,5,3,4,5,0,4,1,5,4,2,15};
    int taxi_c5[] = {15,3,4,15,15,3,4,15,1,3,15,2,4,15,3,15,15,1,4,15,3,2,15,6,
                     3,15,4,15,15,3,4,15,1,2,15,3,4,15,3,15,15,1,4,3,15,2,6,15};
    int taxi_ac3[] = {4,5,3,4,5,0,4,5,1,4,5,3,4,5,0,1,4,5,2,4,5,3,4,2,
                      5,4,0,5,4,3,5,1,4,2,5,4,0,5,4,3,1,4,5,2,4,5,6,15};
    int taxi_ac5[] = {5,3,4,5,3,0,5,4,1,5,3,2,5,4,3,0,5,1,2,5,3,4,5,6,
                      3,5,0,4,5,3,1,2,5,3,4,5,0,1,5,3,2,5,4,3,5,6,9,15};
    int taxi_ac6[] = {5,3,0,5,3,1,5,3,2,5,3,6,5,3,0,1,5,3,2,5,3,6,5,9,
                      3,5,0,3,5,1,3,2,5,6,3,5,0,3,5,1,2,5,3,6,5,3,4/* <= EEE port 27 */,15};
    int *taxi_p;

    /* Determine the correct cell bus calender configuration (based on target and mux mode) */
    switch (vtss_state->create.target) {
    case VTSS_TARGET_LYNX_1:
    case VTSS_TARGET_JAGUAR_1:
    case VTSS_TARGET_CE_MAX_24:
    case VTSS_TARGET_E_STAX_III_48:
    case VTSS_TARGET_E_STAX_III_68:
        if (mux_mode == VTSS_PORT_MUX_MODE_0) {
            taxi_p = taxi_c1; /* 24x1G + 4x10G (or 12G) */
        } else if (mux_mode == VTSS_PORT_MUX_MODE_1) {
            taxi_p = taxi_c2; /* 20x1G + 4x2G5 + 3x10G (or 12G) */
        } else {
            taxi_p = taxi_c3; /* 16x1G + 8x2G5 + 2x10G (or 12G) */
        }
        break;
    case VTSS_TARGET_E_STAX_III_24_DUAL:
    case VTSS_TARGET_E_STAX_III_68_DUAL:
        if (vtss_state->port.port_int_0 == 27) {
            /* XAUI_0 and XAUI_1 are 12G interconnects */
            taxi_p = taxi_ac3;
        } else if (mux_mode == VTSS_PORT_MUX_MODE_0) {
            taxi_p = taxi_c1;
        } else if (mux_mode == VTSS_PORT_MUX_MODE_1) {
            taxi_p = taxi_ac5;
        } else {
            taxi_p = taxi_ac6;
        }
        break;
    case VTSS_TARGET_CE_MAX_12:
        if (mux_mode == VTSS_PORT_MUX_MODE_0) {
            taxi_p = taxi_c4; /* 12x1G + 2x10G (or 12G)  */
        } else if (mux_mode == VTSS_PORT_MUX_MODE_1) {
            taxi_p = taxi_c5; /* 12x1G + 4x2G5 + 1x10G (or 12G)  */
        } else {
            return VTSS_RC_ERROR;
        }
        break;
    default:
        VTSS_E("Assembler - Unknown Target");
        return VTSS_RC_ERROR;
    }
  
    /* Initilize  counters */
    JR_WR(ASM, CFG_STAT_CFG, VTSS_F_ASM_CFG_STAT_CFG_STAT_CNT_CLR_SHOT);

    /* Disable calender while setting it up */
    JR_WR(ASM, CFG_CBC_LEN_CFG, VTSS_F_ASM_CFG_CBC_LEN_CFG_CBC_LEN(0));
     
    /* Format the  the bus calender with Taxi's */
    for (i=0; i<cbc_len; i++) {
        JR_WRXF(ASM, CFG_CBC_CFG, i, DEV_GRP_NUM, taxi_p[i]);
    }

    /* Set Cell Bus Calendar length */
    JR_WR(ASM, CFG_CBC_LEN_CFG, VTSS_F_ASM_CFG_CBC_LEN_CFG_CBC_LEN(cbc_len));

    /* Wait until csc init is complete  */
    JR_POLL_BIT(ASM, CFG_STAT_CFG, STAT_CNT_CLR_SHOT);

    return VTSS_RC_OK;
}

static vtss_rc jr_init_ana(vtss_state_t *vtss_state)
{
    u32 value, i;    
    /* Initilize multiple RAMs in various blocks of the analyzer */

    /* ANA_CL RAM init */
    JR_WRB(ANA_CL_2, COMMON_COMMON_CTRL, TABLE_INIT_SHOT, 1);
    
    /* Initiatate MAC table clear */
    JR_WRB(ANA_L2, COMMON_TABLE_CLR_CFG, MAC_TBL_INIT_SHOT, 1);
    
    /* Initilizing ANA_L2 RAMs */
    JR_WRB(ANA_L2, COMMON_TABLE_CLR_CFG, ISDX_INIT_SHOT, 1);
    JR_WRB(ANA_L2, COMMON_TABLE_CLR_CFG, PATHGRP_INIT_SHOT, 1);

    /* Initilizing ANA_L3 RAMs */
    JR_WR(ANA_L3_2, COMMON_TABLE_CTRL, 0x1F);

    /* Clearing RAMs */
    JR_WRB(ANA_AC, PS_COMMON_PS_TABLE_CLR_CFG, PGID_TABLE_INIT_SHOT, 1);
    JR_WRB(ANA_AC, PS_COMMON_PS_TABLE_CLR_CFG, SRC_TABLE_INIT_SHOT, 1);

    /* Clear policers */
    JR_WRB(ANA_AC, POL_ALL_CFG_POL_ALL_CFG, PRIO_FORCE_INIT, 1);
    JR_WRB(ANA_AC, POL_ALL_CFG_POL_ALL_CFG, ACL_FORCE_INIT, 1);
    JR_WRB(ANA_AC, POL_ALL_CFG_POL_ALL_CFG, FORCE_INIT, 1);

    /* Enable overall signaling of flow control */
    JR_WRB(ANA_AC, POL_ALL_CFG_POL_ALL_CFG, PORT_FC_ENA, 1);

    /* Clearing Port Stat RAMs */
    JR_WRB(ANA_AC, STAT_GLOBAL_CFG_PORT_STAT_RESET, RESET, 1);

    /* Clearing Queue Stat RAMs ?? */

    /* Clearing ISDX Stat RAMs */
    JR_WRB(ANA_AC, STAT_GLOBAL_CFG_ISDX_STAT_RESET, RESET, 1);

#if defined(VTSS_FEATURE_VLAN_COUNTERS)
    /* Enable VID counters */
    JR_WRB(ANA_AC, PS_COMMON_MISC_CTRL, USE_VID_AS_ISDX_ENA, 1);
#endif /* VTSS_FEATURE_VLAN_COUNTERS */

    /* Clearing Tunnel Stat RAMs */
    JR_WRB(ANA_AC, STAT_GLOBAL_CFG_TUNNEL_STAT_RESET, RESET, 1);

    /* Clearing Rleg Stat RAMs */
    JR_WRB(ANA_AC, STAT_GLOBAL_CFG_IRLEG_STAT_RESET, RESET, 1);
    JR_WRB(ANA_AC, STAT_GLOBAL_CFG_ERLEG_STAT_RESET, RESET, 1);

    /* Clearing service DLB RAMs */
    JR_WRB(ANA_AC, COMMON_SDLB_DLB_CTRL, DLB_INIT_SHOT, 1);
    
    /* Clearing tunnel DLB RAMs */
    JR_WRB(ANA_AC, COMMON_TDLB_DLB_CTRL, DLB_INIT_SHOT, 1);

    /* Clearing the sFlow cnt register */
    JR_WRM_SET(ANA_AC, PS_COMMON_SFLOW_RESET_CTRL, 0xFFFFFFFF);

    /* Initilizing LPM */
    JR_WRF(ANA_L3_2, LPM_ACCESS_CTRL, ACCESS_CMD, 6);
    JR_WRB(ANA_L3_2, LPM_ACCESS_CTRL, ACCESS_SHOT, 1);

    /* Enable usage of DBL_VID_PCP for lookup (default disabled, TN0613) */
    for (i=0; i<35; i++) {
        JR_RDX(ANA_CL_2, PORT_IS0_CFG_1, i,  &value);
        JR_WRX(ANA_CL_2, PORT_IS0_CFG_1, i, (value | VTSS_BIT(1)));
    }
    /* Must be set to 1 to avoid bad pointers from C-domain into B-domain (TN0613) */
    JR_RD(ANA_L2, COMMON_BLRN_CFG, &value);

    JR_WR(ANA_L2, COMMON_BLRN_CFG, (value | VTSS_BIT(13)));

    /* Enable CPU-copying of port-move detects of statically learned MAC addresses */
    JR_WRB(ANA_L2, COMMON_LRN_CFG, LOCKED_PORTMOVE_COPY_ENA, 1);

    /* Now, wait until initilization is complete */

    /* Await MAC table clear complete */
    JR_POLL_BIT(ANA_L2, COMMON_TABLE_CLR_CFG, MAC_TBL_INIT_SHOT);

    /* Await ANA_CL RAM init */
    JR_POLL_BIT(ANA_CL_2, COMMON_COMMON_CTRL, TABLE_INIT_SHOT);
    
    /* Await MAC table clear */
    JR_POLL_BIT(ANA_L2, COMMON_TABLE_CLR_CFG, MAC_TBL_INIT_SHOT);
    
    /* Await ANA_L2 RAMs */
    JR_POLL_BIT(ANA_L2, COMMON_TABLE_CLR_CFG, ISDX_INIT_SHOT);
    JR_POLL_BIT(ANA_L2, COMMON_TABLE_CLR_CFG, PATHGRP_INIT_SHOT);

    /* Await ANA_L3 RAMs */
    JR_POLL_BIT(ANA_L3_2, COMMON_TABLE_CTRL, INIT_IPMC_SHOT);
    JR_POLL_BIT(ANA_L3_2, COMMON_TABLE_CTRL, INIT_ARP_SHOT);
    JR_POLL_BIT(ANA_L3_2, COMMON_TABLE_CTRL, INIT_VLAN_SHOT);

    /* Await clearing RAMs */
    JR_POLL_BIT(ANA_AC, PS_COMMON_PS_TABLE_CLR_CFG, PGID_TABLE_INIT_SHOT);
    JR_POLL_BIT(ANA_AC, PS_COMMON_PS_TABLE_CLR_CFG, SRC_TABLE_INIT_SHOT);

    /* Await policer init */
    JR_POLL_BIT(ANA_AC, POL_ALL_CFG_POL_ALL_CFG, PRIO_FORCE_INIT);
    JR_POLL_BIT(ANA_AC, POL_ALL_CFG_POL_ALL_CFG, ACL_FORCE_INIT);
    JR_POLL_BIT(ANA_AC, POL_ALL_CFG_POL_ALL_CFG, FORCE_INIT);

    /* Await Clearing Port Stat RAMs */
    JR_POLL_BIT(ANA_AC, STAT_GLOBAL_CFG_PORT_STAT_RESET, RESET);

    /* Await Clearing Queue Stat RAMs ?? */

    /* Await Clearing ISDX Stat RAMs */
    JR_POLL_BIT(ANA_AC, STAT_GLOBAL_CFG_ISDX_STAT_RESET, RESET);

#if 0 //defined(VTSS_FEATURE_VLAN_COUNTERS)
    /* Await USE_VID_AS_ISDX_ENA bit */
    JR_POLL_BIT(ANA_AC, PS_COMMON_MISC_CTRL, USE_VID_AS_ISDX_ENA);
#endif

    /* Await Clearing Tunnel Stat RAMs */
    JR_POLL_BIT(ANA_AC, STAT_GLOBAL_CFG_TUNNEL_STAT_RESET, RESET);

    /* Await Clearing Rleg Stat RAMs */
    JR_POLL_BIT(ANA_AC, STAT_GLOBAL_CFG_IRLEG_STAT_RESET, RESET);
    JR_POLL_BIT(ANA_AC, STAT_GLOBAL_CFG_ERLEG_STAT_RESET, RESET);

    /* Await Clearing service DLB RAMs */
    JR_POLL_BIT(ANA_AC, COMMON_SDLB_DLB_CTRL, DLB_INIT_SHOT);
    
    /* Await Clearing tunnel DLB RAMs */
    JR_POLL_BIT(ANA_AC, COMMON_TDLB_DLB_CTRL, DLB_INIT_SHOT);

    /* Await LPM clear */
    JR_POLL_BIT(ANA_L3_2, LPM_ACCESS_CTRL, ACCESS_SHOT);

    /* Resetting the sFlow cnt reg */
    JR_WRM_SET(ANA_AC, PS_COMMON_SFLOW_RESET_CTRL, 0x0); 

    return VTSS_RC_OK; 
}

#define JR_SQS_CNT_SET(tgt, xmit, cnt_id, qu, frm_type, event_mask)                       \
{                                                                                         \
    JR_WRXY(tgt, STAT_CNT_CFG_##xmit##_STAT_CFG, qu, JR_CNT(xmit, cnt_id),                 \
            JR_PUT_FLD(tgt, STAT_CNT_CFG_##xmit##_STAT_CFG, CFG_CNT_FRM_TYPE, frm_type)); \
    JR_WRX(tgt, STAT_GLOBAL_CFG_##xmit##_STAT_GLOBAL_EVENT_MASK, JR_CNT(xmit, cnt_id),     \
           JR_PUT_FLD(tgt, STAT_GLOBAL_CFG_##xmit##_STAT_GLOBAL_EVENT_MASK,               \
                      GLOBAL_EVENT_MASK, event_mask));                  \
}                                                                                         

static vtss_rc jr_init_sqs(vtss_state_t *vtss_state)
{
    u32 qu, p;
    sqs_t   sqs;
    vtss_target_type_t target = jr_target_get(vtss_state);

    /* Start initilization */
    JR_WRB(OQS, MAIN_MAIN_REINIT_1SHOT, MAIN_REINIT, 1);
    JR_WRB(IQS, MAIN_MAIN_REINIT_1SHOT, MAIN_REINIT, 1);

    JR_WRB(IQS, STAT_GLOBAL_CFG_ISDX_STAT_RESET, RESET, 1);
    JR_WRB(OQS, STAT_GLOBAL_CFG_ISDX_STAT_RESET, RESET, 1);

    JR_WRB(IQS, STAT_GLOBAL_CFG_RX_STAT_RESET, RESET, 1);
    JR_WRB(OQS, STAT_GLOBAL_CFG_RX_STAT_RESET, RESET, 1);

    JR_WRB(IQS, STAT_GLOBAL_CFG_TX_STAT_RESET, RESET, 1);
    JR_WRB(OQS, STAT_GLOBAL_CFG_TX_STAT_RESET, RESET, 1);

    /* Wait until completed */
    JR_POLL_BIT(OQS, MAIN_MAIN_REINIT_1SHOT, MAIN_REINIT);
    JR_POLL_BIT(IQS, MAIN_MAIN_REINIT_1SHOT, MAIN_REINIT);

    JR_POLL_BIT(IQS, STAT_GLOBAL_CFG_ISDX_STAT_RESET, RESET);
    JR_POLL_BIT(OQS, STAT_GLOBAL_CFG_ISDX_STAT_RESET, RESET);

    JR_POLL_BIT(IQS, STAT_GLOBAL_CFG_RX_STAT_RESET, RESET);
    JR_POLL_BIT(OQS, STAT_GLOBAL_CFG_RX_STAT_RESET, RESET);

    JR_POLL_BIT(IQS, STAT_GLOBAL_CFG_TX_STAT_RESET, RESET);
    JR_POLL_BIT(OQS, STAT_GLOBAL_CFG_TX_STAT_RESET, RESET);

    /* Setup SQS counters. */
    /* Counters can be extracted from IQS and OQS for RX (4 sets) and TX (2 sets)  */

    /*  ##### IQS RX counters #####  */
    for (qu = 0; qu < 280; qu++) { 
        /* 4 sets of RX counters */
        JR_SQS_CNT_SET(IQS, RX, GREEN, qu, 1, 1<<8);    /* Green */
        JR_SQS_CNT_SET(IQS, RX, YELLOW , qu, 1, 1<<9);  /* Yellow */
        JR_SQS_CNT_SET(IQS, RX, QDROPS, qu, 3, 0x3E);   /* SQS and RED drops */
        JR_SQS_CNT_SET(IQS, RX, PDROPS, qu, 2, 0x7C00); /* Policer drops */

        /* 2 sets of TX counters (using 1) */
        JR_SQS_CNT_SET(IQS, TX, DROPS, qu, 1, 0x8);     /* Full OQS Tx drops */
    }
    
    /*  ##### OQS counters #####  */
    for (qu = 0; qu < 464; qu++) { 
        /* 4 sets of RX counters (using 3) */
        JR_SQS_CNT_SET(OQS, RX, GREEN, qu, 1, 1<<8);    /* Green */
        JR_SQS_CNT_SET(OQS, RX, YELLOW , qu, 1, 1<<9);  /* Yellow */
        JR_SQS_CNT_SET(OQS, RX, QDROPS, qu, 1, 0x3E);   /* SQS drops */

        /* 2 sets of TX counters (using 2) */
        JR_SQS_CNT_SET(OQS, TX, GREEN, qu, 1, 1<<4);  /* Green */
        JR_SQS_CNT_SET(OQS, TX, YELLOW, qu, 1, 1<<5); /* Yellow */
    }

    /* IQS and OQS reserved fields that must be set to 0 during initialization */
    JR_WR(IQS, RESERVED_RESERVED, 0)
    JR_WR(OQS, RESERVED_RESERVED, 0);

    /* Enterprise must react on congestion for all DP levels */
    if (target == VTSS_TARGET_E_STAX_III_48 || 
        target == VTSS_TARGET_E_STAX_III_68) {
        JR_WRF(IQS, CONG_CTRL_CH_CFG, DE_ENA, 0xF); 
        JR_WRF(ARB, CFG_STATUS_CH_CFG, DE_ENA, 0xF);
        JR_WRF(OQS, CONG_CTRL_CH_CFG, DE_ENA, 0xF); 
    } else {
        JR_WRF(IQS, CONG_CTRL_CH_CFG, DE_ENA, 0xE); 
        JR_WRF(ARB, CFG_STATUS_CH_CFG, DE_ENA, 0xE);
        JR_WRF(OQS, CONG_CTRL_CH_CFG, DE_ENA, 0xE); 
    }
    
    /* Initilize SQS WMs */
    for(p = 0; p < 89; p++) {
        /* jr_sqs_calc_wm(<state> <chipport> <spd>, <jumbo> <fc> <power_down> <init> <struct>) */
        VTSS_RC(jr_sqs_calc_wm(vtss_state, p, VTSS_SPEED_1G, 0, 0, 0, 1, &sqs));
        VTSS_RC(jr_sqs_set(vtss_state, p, &sqs));
    }
    
    return VTSS_RC_OK;
}

static vtss_rc jr_init_arb(vtss_state_t *vtss_state)
{
    u8 i;
    
    /* ARB: disable arbitration randomness */
    JR_WR(ARB, CFG_STATUS_RND_GEN_CFG, 1);

    /* ARB: disable force drop mode except for the CPU */
    for(i = 0; i < 8; i++) {
        JR_WRX(ARB, DROP_MODE_CFG_OUTB_ETH_DROP_MODE_CFG, i, 0x00);
        JR_WRX(ARB, DROP_MODE_CFG_OUTB_CPU_DROP_MODE_CFG, i, 0xff); /* Bugzilla#7548 */
        JR_WRX(ARB, DROP_MODE_CFG_OUTB_VD_DROP_MODE_CFG,  i, 0x00);
    }
    
    /* ARB: enable QS spill over watermarks (DBG-reg ARB::SWM_CFG) */
    VTSS_RC(vtss_jr1_wr(vtss_state, VTSS_IOREG(VTSS_TO_ARB, 0x53), 1));

    return VTSS_RC_OK;
}

/* Initilization of the scheduler. */
/* TX cell bus calender is configured based on the Mux mode */
static vtss_rc jr_init_sch(vtss_state_t *vtss_state)
{
    /* Jaguar MUX:0, 10G:27-28, 12G:29-30 */
    u8 cal_s1[] = {27,29,28,0,30,6,29,27,12,28,30,18,29,1,27,30,28,7,29,13,19,27,30,28,
                   29,2,8,30,27,29,28,14,20,30,3,63,27,29,28,9,30,15,29,27,21,28,30,4,
                   29,10,27,30,28,16,29,22,5,27,30,28,29,11,17,30,27,29,28,23,33,30,31,63}; 

    /* CEMAX-24 MUX:0 HMD:30 */
    u8 cal_m1[] = {34,28,29,27,63,33,31,34,28,29,27,23,17,22,34,28,29,27,21,16,20,34,28,29,
                   27,19,15,18,34,28,29,27,14,11,13,10,34,28,29,27,12,9,5,34,28,29,27,8,
                   4,7,34,28,29,27,6,3,63,34,28,29,27,2,63,1,34,28,29,27,0,63,63,63};

    /* CEMAX-24 MUX:0 HMD:29,30 */
    u8 cal_m2[] = {35,28,34,27,63,33,31,35,28,34,27,23,17,22,35,28,34,27,21,16,20,35,28,34,
                   27,19,15,18,35,28,34,27,14,11,13,10,35,28,34,27,12,9,5,35,28,34,27,8,
                   4,7,35,28,34,27,6,3,63,35,28,34,27,2,63,1,35,28,34,27,0,63,63,63};

    /* CEMAX-12 MUX:0 HMD:28 */
    u8 cal_m13[] = {34,63,27,33,31,17,22,34,21,27,20,16,19,15,34,18,27,14,11,13,63,34,12,27,
                    63,63,63,63,34,63,27,63,63,63,63,63,34,63,27,63,63,63,63,34,63,27,63,63,
                    63,63,34,63,27,63,63,63,63,34,63,27,63,63,63,63,34,63,27,63,63,63,63,63};

    u8 cal_ac3[] = {28,29,27,0,30,6,28,29,27,12,18,30,28,29,27,1,7,30,28,29,27,13,19,2,
                    28,29,27,30,8,14,28,29,27,30,20,3,28,29,27,9,30,15,28,29,27,21,4,30,
                    28,29,27,10,16,30,28,29,27,22,5,11,28,29,27,30,17,23,28,29,27,30,33,31,
                    28,29,27,0,30,6,28,29,27,12,18,30,28,29,27,1,7,30,28,29,27,13,19,2,
                    28,29,27,30,8,14,8,29,27,30,20,3,28,29,27,9,30,15,28,29,27,21,4,30,
                    28,29,27,10,16,30,28,29,27,22,5,11,28,29,27,30,17,23,28,29,27,30,63,31};       

    u8 cal_ac5[] = {29,28,23,30,32,24,29,25,28,30,26,0,29,18,6,30,28,19,29,12,1,30,28,7,
                    29,13,2,30,8,28,29,14,23,30,24,3,29,28,25,30,26,32,29,9,28,30,15,4,
                    29,10,16,30,28,5,29,11,31,30,28,22,29,23,17,30,24,28,29,25,33,30,26,63,
                    29,28,20,30,21,32,29,0,6,30,28,19,29,18,12,30,23,28,29,24,1,30,25,7,
                    29,28,26,30,13,2,29,28,8,30,14,3,29,9,15,30,28,32,29,4,10,30,28,23,
                    29,24,16,30,25,28,29,26,5,30,31,11,29,28,17,30,33,22,29,28,21,30,63,20};

    u8 cal_ac6[] = {29,32,23,30,0,24,29,25,6,30,26,12,29,19,32,30,20,1,29,21,7,30,22,13,
                    29,31,32,30,2,8,29,14,23,30,24,3,29,32,25,30,26,9,29,19,15,30,20,4,
                    29,21,32,30,22,10,29,16,5,30,11,18,29,23,32,30,24,17,29,25,63,30,26,33,
                    29,19,32,30,20,0,29,21,6,30,22,12,29,32,1,30,23,7,29,24,13,30,25,2,
                    29,32,26,30,19,8,29,20,14,30,21,3,29,22,32,30,31,9,29,15,4,30,10,23,
                    29,24,32,30,25,16,29,26,5,30,19,11,29,20,32,30,21,17,29,22,63,30,18,33};

    u8 *cal_p;
    u32 cbc_len = 144, i, len, port, port_unused;
    u32 hm = vtss_jr1_ce_max_hm(vtss_state);
    vtss_target_type_t target = vtss_state->create.target;   
    
    /* Determine the correct cell bus calender configuration (based on target and mux mode, i.e. port speeds) */
    switch (vtss_state->port.mux_mode[vtss_state->chip_no]) {
    case VTSS_PORT_MUX_MODE_0:
        if (target == VTSS_TARGET_CE_MAX_24 || target == VTSS_TARGET_CE_MAX_12) {
            cbc_len = 72;
            cal_p = (hm == 0 || hm == 1 ? cal_m2 :
                     target == VTSS_TARGET_CE_MAX_24 ? cal_m1 : cal_m13);
        } else if (target != jr_target_get(vtss_state) && (vtss_state->port.port_int_0 == 27)) {
            /* Dual Jaguar target using XAUI_0 and XAUI_1 interconnects */
            cal_p = cal_ac3; /* Jaguar1 10G:29-30 12G:27,28 */
        } else {
            cbc_len = 72;
            cal_p = cal_s1; /* Jaguar1 10G:27-28, 12G:29-30 */
        }
        break;
    case VTSS_PORT_MUX_MODE_1:
        cal_p = cal_ac5;
        break;
    default:
        cal_p = cal_ac6;
        break;
    }

    /* Init leaky bucket */
    JR_WRB(SCH, ETH_ETH_LB_CTRL, LB_INIT, 1);
    JR_WRB(SCH, HM_HM_SCH_CTRL, LB_INIT, 1);
    /* Init MCTX RAM */
    JR_WR(SCH, QSIF_QSIF_CTRL, (1<<2)); 

    /* Disable calender while setting it up */
    JR_WR(SCH, CBC_CBC_LEN_CFG, 0); 

    /* Setup default host mode devices to unused port */
    port_unused = vtss_jr1_unused_chip_port(vtss_state);
    JR_WRF(SCH, QSIF_QSIF_HM_CTRL, ETH_DEV_HMDA, port_unused);
    JR_WRF(SCH, QSIF_QSIF_HM_CTRL, ETH_DEV_HMDB, port_unused);

    /* The calendar is repeated 4 times */
    len = (cbc_len * 4);
    for (i = 0; i < len; i++) {
        port = cal_p[i % cbc_len];
        if (port == 63 && port_unused != VTSS_PORT_NO_NONE) {
            /* If the calendar is replicated, the first occurrence of the IDLE port
               is replaced by the unused port for EEE */
            port = port_unused;
            port_unused = VTSS_PORT_NO_NONE;
        }
        JR_WRX(SCH, CBC_DEV_ID_CFG_CBC_DEV_ID, i, port);
    }
    
    JR_POLL_BIT(SCH, ETH_ETH_LB_CTRL, LB_INIT);

    /* Enable calender  */
    JR_WR(SCH, CBC_CBC_LEN_CFG, len);
    
    return VTSS_RC_OK;
}

static vtss_rc jr_init_rew(vtss_state_t *vtss_state)
{
    /* Start Ram init */
    JR_WRB(REW, COMMON_COMMON_CTRL, FORCE_PORT_CFG_INIT, 1); 
    JR_WR(REW, STAT_GLOBAL_CFG_ESDX_STAT_RESET, 1); 

    JR_POLL_BIT(REW, COMMON_COMMON_CTRL, FORCE_PORT_CFG_INIT);
    JR_POLL_BIT(REW, STAT_GLOBAL_CFG_ESDX_STAT_RESET, RESET);

    /* Count bytes */
    JR_WRXB(REW, STAT_GLOBAL_CFG_ESDX_STAT_GLOBAL_CFG, 0, GLOBAL_CFG_CNT_BYTE, 1); 

    /* Better default (TN0613) */
    /* REW:COMMON:VSTAX_PORT_GRP_CFG[0-1].VSTAX_LRN_ALL_HP_ENA */
    JR_WRXB(REW, COMMON_VSTAX_PORT_GRP_CFG, 0, VSTAX_LRN_ALL_HP_ENA, 1);
    JR_WRXB(REW, COMMON_VSTAX_PORT_GRP_CFG, 1, VSTAX_LRN_ALL_HP_ENA, 1);
    
    /* REW:COMMON:VSTAX_PORT_GRP_CFG[0-1].VSTAX_MODE */
    JR_WRXB(REW, COMMON_VSTAX_PORT_GRP_CFG, 0, VSTAX_MODE, 1);
    JR_WRXB(REW, COMMON_VSTAX_PORT_GRP_CFG, 1, VSTAX_MODE, 1);

    return VTSS_RC_OK;
}

static vtss_rc jr_init_dsm(vtss_state_t *vtss_state)
{    
    // MAC mode LPORT number mapping init TBD      

    /* Fixed water marks for DEV1G and DEVNPI */
    JR_WR(DSM, CFG_RATE_CTRL_WM,
          JR_PUT_FLD(DSM, CFG_RATE_CTRL_WM, TAXI_128_RATE_CTRL_WM_DEVNPI, 20) |
          JR_PUT_FLD(DSM, CFG_RATE_CTRL_WM, TAXI_32_RATE_CTRL_WM, 36));

    return VTSS_RC_OK;
}

#if defined (VTSS_ARCH_JAGUAR_1_CE_MAC)
/* Configure Jaguar to operate as a MAC */
static vtss_rc jr_init_cemax(vtss_state_t *vtss_state)
{
    u32 hmda, hmdb, hmd_mask=0, flood_mask, pgid, i, prio, host_mode;

    host_mode = vtss_jr1_ce_max_hm(vtss_state);    

    if (host_mode == 0 || host_mode == 1) {
        hmda = VTSS_CE_MAC_HMDA;
        hmdb = VTSS_CE_MAC_HMDB;
    } else { /* host_mode == VTSS_HOST_MODE_2 || host_mode == VTSS_HOST_MODE_3 */
        if (vtss_state->create.target == VTSS_TARGET_CE_MAX_24) {
            hmda = VTSS_CE_MAC_HMDB;
            hmdb = VTSS_CE_MAC_HMDB;
        } else {
            hmda = VTSS_CE_MAC_HMDA;
            hmdb = VTSS_CE_MAC_HMDA;
        }
    }
    vtss_state->port.ce_mac_hmda = hmda;
    vtss_state->port.ce_mac_hmdb = hmdb;

    /* == Analyzer == */
    for (i = 0; i <= (hmdb-hmda); i++) {
        JR_WRXB(ANA_CL_2, COMMON_HM_CFG, i, HMD_PORT_VLD, 1);
        JR_WRXF(ANA_CL_2, COMMON_HM_CFG, i, HMD_PORT, (i==0?hmda:hmdb));            
        JR_WRXB(ANA_AC, PS_COMMON_HM_CFG, i, HMD_PORT_VLD, 1);
        JR_WRXB(ANA_AC, PS_COMMON_HM_CFG, i, HMD_HOST_ENA, 1);
        JR_WRXF(ANA_AC, PS_COMMON_HM_CFG, i, HMD_PORT, (i==0?hmda:hmdb));
        if (i == 1) {
            /* Enable PGID offset for HMDB */
            JR_WRXF(ANA_AC, PS_COMMON_HM_CFG, i, HMD_ADDR_IDX, 1);            
            JR_WRF(ANA_AC, PS_COMMON_PS_COMMON_CFG, HMD_ADDR_OFFSET, 1);
        }
        hmd_mask |= (1<<(i==0?hmda:hmdb));       
    }

    /* Enable MAC based forwarding in ANA_AC */
    JR_WRF(ANA_L2, COMMON_VSTAX_CTRL, OWN_UPSID, 0);
    JR_WRF(ANA_AC, PS_COMMON_COMMON_VSTAX_CFG, OWN_UPSID, 0);
    JR_WRB(ANA_AC, PS_COMMON_PS_COMMON_CFG, HM_FWD_ENA, 1);

    /* Disable learning */
    JR_WR(ANA_L3_2, COMMON_PORT_LRN_CTRL, 0);

    /* No flooding to HMDs */
    flood_mask = 0xffffffff ^ hmd_mask;
    for (pgid = 32; pgid < 38; pgid++) {
        JR_WRX(ANA_AC, PGID_PGID_CFG_0, pgid, flood_mask);        
    }

    /* == Arbiter == */    
    /* Use UPSID 0-7 */
    JR_WRF(ARB, CFG_STATUS_HM_CFG, HM_UPSID_RANGE, 0);    
    JR_WRF(ARB, CFG_STATUS_HM_CFG, ETH_DEV_HMDA, hmda);
    JR_WRF(ARB, CFG_STATUS_HM_CFG, ETH_DEV_HMDB, hmdb);

    /* Enable use of CM for arbitrating traffic from line ports */
    for (prio = 0; prio < 8; prio++) {
        JR_WRX(ARB, CFG_STATUS_CM_CFG_VEC0, prio, 0xffffffff); 
    }
    /* Enable Host mode operation */
    JR_WRB(ARB, CFG_STATUS_HM_CFG, HM_ENA, 1);

    /* == SQS == */
    /* Enable WM for host queue */
    JR_WRB(OQS, CE_HOST_MODE_CFG_HOST_MODE_CM_CFG, HOST_MODE_CM_SEL, 1);
    
    /* 4 or 8 priorities per HP */
    JR_WRB(OQS, CE_HOST_MODE_CFG_HOST_PORT_RESRC_CTL_CFG, HOST_PORT_PRIO_CFG, 
           (host_mode == 0 || host_mode == 2) ? 1 : 0);

    /* == Scheduler == */
    JR_WRF(SCH, QSIF_QSIF_HM_CTRL, HM_MODE, host_mode);
    JR_WRF(SCH, QSIF_QSIF_HM_CTRL, ETH_DEV_HMDA, hmda);
    JR_WRF(SCH, QSIF_QSIF_HM_CTRL, ETH_DEV_HMDB, (hmda == hmdb) ? 0 : hmdb);
    JR_WRF(SCH, HM_HM_SCH_CTRL, HM_MODE, host_mode);

    /* == Rewriter == */    
    JR_WRF(REW, COMMON_HM_CTRL, HM_MODE, host_mode);    
    JR_WRF(REW, COMMON_HM_CTRL, ETH_DEV_HMDA, hmda);    
    JR_WRF(REW, COMMON_HM_CTRL, ETH_DEV_HMDB, (hmda == hmdb) ? 0 : hmdb);

    /* == Disassembler == */    
    /* Select 4 or 8 priorities */
    JR_WRB(DSM, CM_CFG_HOST_PORT_RESRC_CTL_CFG, HOST_PORT_PRIO_CFG, 
           (host_mode == 0 || host_mode == 2) ? 1 : 0);
    JR_WRF(DSM, CM_CFG_CMEF_OWN_UPSID_CFG, CMEF_OWN_UPSID, 0);
    JR_WRF(DSM, CM_CFG_CMEF_OWN_UPSID_CFG, HM_OWN_UPSID_MASK, 3);
    JR_WR(DSM, CM_CFG_CMEF_UPSID_ACTIVE_CFG, 0x7f);

    /* Enable CM generation for HM queues */
    for(i = 0; i < 6; i++) {
        JR_WRX(DSM, CM_CFG_CMM_HOST_QU_MASK_CFG, i, 0xffffffff);
    }
    /* Enable internal FC for HPs */
    if (host_mode == 0 || host_mode == 2) {
        JR_WRX(DSM, CM_CFG_CMM_HOST_LPORT_MASK_CFG, 0, 0xffffffff);  /* 24 HPs */
        JR_WRX(DSM, CM_CFG_CMM_HOST_LPORT_MASK_CFG, 1, 0);
    } else {
        /* 48 HPs */
        JR_WRX(DSM, CM_CFG_CMM_HOST_LPORT_MASK_CFG, 0, 0xffffffff);  /* 48 HPs */
        JR_WRX(DSM, CM_CFG_CMM_HOST_LPORT_MASK_CFG, 1, 0xffffffff);
    }

    /* == OOBFC == */
    JR_WRF(DSM, CM_CFG_CMEF_OWN_UPSID_CFG, HM_OWN_UPSID_MASK, 4);
    JR_WR(DSM, CM_CFG_CMEF_UPSID_ACTIVE_CFG, 0xff7f);

    /* Initialize the LPORT_NUM table */
    JR_WR(DSM, CM_CFG_LPORT_NUM_INIT, 1);
    JR_POLL_BIT(DSM, CM_CFG_LPORT_NUM_INIT, LPORT_NUM_INIT_SHOT);

    // For the status channel registers configure which UPSID, they shall capture
    // status for.
    {
        u8 upsid = 8;
        u8 tmp;
        for (tmp = 0; tmp <= 7; tmp++) {
            JR_WRX(DSM, HOST_IF_IBS_COMMON_CFG_CAPTURE_UPSID_CFG, tmp, upsid);
            upsid++;
        }
        JR_WRX(DSM, HOST_IF_IBS_COMMON_CFG_CAPTURE_UPSID_CFG, 8, 0);
    }
    return VTSS_RC_OK;
}

static vtss_rc jr_cemax_port_map_set(vtss_state_t *vtss_state)
{
    /* Do the Line Port to Host Port mapping */
    u32 host_mode = vtss_jr1_ce_max_hm(vtss_state);
    u32 max_lport, prio, lprio, hmq, hmdx, upsid, upspn, ups_mask, prios, mask, pgid, pgid_offset=64;
    vtss_port_no_t  port_no;
    vtss_lport_no_t lport_no;
    
    if (host_mode == 0 || host_mode == 2) { 
        prios = 8;
        max_lport = 24;
    } else {
        max_lport = 48;
        prios = 4;
    }
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        
        if (vtss_jr1_cemax_port_is_host(vtss_state, port_no)) {
            continue; /* HI do not map to Lports */
        }
        
        lport_no = vtss_state->port.map[port_no].lport_no;
        
        if (lport_no == VTSS_LPORT_MAP_NONE) { /* No mapping - skip */
            continue;
        } else if (lport_no == VTSS_LPORT_MAP_DEFAULT) {  
            lport_no = (port_no - VTSS_PORT_NO_START); /* Default mapping: API port to Lport, starting from 0. */ 
            if (lport_no >= max_lport) {
                break;
            }
        }
        if (lport_no >= max_lport) {
            VTSS_E("illegal lport_no: %d for port_no: %d, host mode: %u", lport_no, port_no, host_mode);
            return VTSS_RC_ERROR;
        }
        /* Update the API map with the lport_no (only meaningful if VTSS_LPORT_MAP_NONE was used) */
        vtss_state->port.map[port_no].lport_no = lport_no;
        
        if (host_mode == 0) { 
            hmdx = (lport_no < 12) ? 29 : 30;
        } else if (host_mode == 1) { 
            hmdx = (lport_no < 24) ? 29 : 30;
        } else { 
            if (vtss_state->create.target == VTSS_TARGET_CE_MAX_24) {
                hmdx = 30;
            } else {
                hmdx = 28;
            }
        }
        
        /* Apply the mappings to the PGID table. The PGID HQ index starts at 1024+32 */
        for (prio = 0, lprio = 0; prio < 8; prio++) {
            if (host_mode == 0 || host_mode == 2) {
                if (prio != 0)
                    lprio++; /* 8 prios maps to 8 host queues */
            } else {
                /* 8 prios map to 4 host queues 0-1 -> 0, ... 6,7 -> 3 */
                if ((prio % 2 == 0) && (prio != 0)) {
                    lprio++;
                } 
            }                
            pgid = PGID_HQ_START + (VTSS_CHIP_PORT(port_no) * 8) + prio;
            hmq = (lport_no * prios) + lprio;
            upsid = (1 + (hmq / 32));
            upspn = hmq % 32;
            ups_mask = hmdx<<16 | upsid<<6 | upspn;
            
            vtss_state->l2.pgid_table[vtss_jr1_vtss_pgid(vtss_state, pgid)].resv = 1; /* Reserve PGID in AIL */
            VTSS_RC(vtss_jr1_pgid_table_write_chip(vtss_state, pgid, ups_mask, 0, 0, 1)); /* Write PGID to chip. CPU copy=0, cpu qu=0, stack=1 */
        }
        
        /* Host to line forwarding. Lport maps back to API port */
        pgid = lport_no;
        mask = 1 << VTSS_CHIP_PORT(port_no - VTSS_PORT_NO_START); 
        vtss_state->l2.pgid_table[vtss_jr1_vtss_pgid(vtss_state, pgid)].resv = 1; /* Reserve PGID in AIL */
        VTSS_RC(vtss_jr1_pgid_table_write_chip(vtss_state, pgid, mask, 0, 0, 0)); /* Write PGID to chip. CPU copy=0, cpu qu=0, stack=0 */
        
        /* Host to line forwarding from HMDB. PGID offset:64 */
        if (host_mode == 0 && lport_no > 11) {
            mask = 1 << VTSS_CHIP_PORT(port_no - VTSS_PORT_NO_START); 
            vtss_state->l2.pgid_table[vtss_jr1_vtss_pgid(vtss_state, pgid_offset)].resv = 1; /* Reserve PGID in AIL */
            VTSS_RC(vtss_jr1_pgid_table_write_chip(vtss_state, pgid_offset, mask, 0, 0, 0)); /* Write PGID to chip. CPU copy=0, cpu qu=0, stack=0 */
            pgid_offset++; 
        }
    }
    return VTSS_RC_OK;
}
#endif /* VTSS_ARCH_JAGUAR_1_CE_MAC */

static vtss_rc jr_update_sqs(vtss_state_t *vtss_state)
{
    vtss_port_no_t port_no;
    vtss_target_type_t target = jr_target_get(vtss_state);
    u32 chipport, startport=0;
    sqs_t sqs;
    vtss_port_speed_t speed;
    vtss_port_mux_mode_t mux_mode = vtss_state->port.mux_mode[vtss_state->chip_no];

    memset(&sqs,0,sizeof(sqs));
    /* The WM is only setup once for these targets */
    if (target == VTSS_TARGET_CE_MAX_12 || target == VTSS_TARGET_CE_MAX_24) {
        /* if (target == VTSS_TARGET_CE_MAX_12) */
        /*     startport = 12; */
        for (chipport = startport; chipport < VTSS_CHIP_PORTS; chipport++) {
            if (mux_mode == VTSS_PORT_MUX_MODE_0) {
                if (chipport > 23 && chipport < 27) 
                    continue;
            } else if (mux_mode == VTSS_PORT_MUX_MODE_1) {
                if ((chipport > 19 && chipport < 23) || (chipport == 27)) 
                    continue;                
            } else if (mux_mode == VTSS_PORT_MUX_MODE_7) {
                if ((chipport > 16 && chipport < 19) || (chipport == 27)) 
                    continue;                
            }               
            if (VTSS_PORT_IS_1G(chipport)) {
                speed = VTSS_SPEED_1G;
            } else {
                speed = VTSS_SPEED_10G;
            }
            VTSS_RC(jr_sqs_rd(vtss_state, chipport, &sqs));
            /* jr_sqs_calc_wm(<state> <chipport> <spd>, <jumbo> <fc> <power_down> <init> <struct>) */
            VTSS_RC(jr_sqs_calc_wm(vtss_state, chipport, speed, 1, 1, 0, 0, &sqs)); /* FC=1, Jumbo=1 */
            VTSS_RC(jr_sqs_set(vtss_state, chipport, &sqs));
        }
    } else if (target == VTSS_TARGET_JAGUAR_1 || target == VTSS_TARGET_LYNX_1) {
        for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
            speed = vtss_state->port.conf[port_no].speed;
            chipport = VTSS_CHIP_PORT(port_no);
            VTSS_RC(jr_sqs_rd(vtss_state, chipport, &sqs));
            /* jr_sqs_calc_wm(<state> <chipport> <spd>, <jumbo> <fc> <power_down> <init> <struct>) */
            VTSS_RC(jr_sqs_calc_wm(vtss_state, chipport, speed, 0, 0, 0, 0, &sqs)); /* FC=0, Jumbo=0 */ 
            VTSS_RC(jr_sqs_set(vtss_state, chipport, &sqs));
        }
    }
    /* Apply user configurable WM (if enabled) */
    if (vtss_state->init_conf.qs_conf.mode != VTSS_QS_MODE_DISABLED) {        
        VTSS_RC(jr_qs_conf_set(vtss_state));
    }
    return VTSS_RC_OK;
}

#if defined(VTSS_FEATURE_VSTAX_V2)
static vtss_rc jr_init_ports_int(vtss_state_t *vtss_state)
{
    vtss_chip_no_t         chip_no;
    u32                    port;
    vtss_port_conf_t       conf;
    vtss_vstax_info_t      *vstax_info;
    vtss_vstax_chip_info_t *chip_info;

    /* Only setup internal ports if dual device target */
    if (vtss_state->chip_count != 2)
        return VTSS_RC_OK;
    
    for (chip_no = 0; chip_no < vtss_state->chip_count; chip_no++) {
        VTSS_SELECT_CHIP(chip_no);
        /* Default VStaX info: UPSID 0/1 and internal ports are STACK_B */
        vstax_info = &vtss_state->l2.vstax_info;
        vstax_info->upsid[chip_no] = chip_no;

        if (chip_no == 0)
            vtss_state->l2.vstax_conf.upsid_0 = chip_no;
        else
            vtss_state->l2.vstax_conf.upsid_1 = chip_no;
        chip_info = &vstax_info->chip_info[chip_no];
        chip_info->mask_b = vtss_state->port.mask_int_ports;
        chip_info->rt_table.topology_type = VTSS_VSTAX_TOPOLOGY_CHAIN;
        chip_info->rt_table.table[chip_no == 0 ? 1 : 0].stack_port_b = 1;
        chip_info->port_conf[1].ttl = 1;

        /* We are using short preamble and shrinked IPG */
        JR_WRF(DSM, CFG_IPG_SHRINK_CFG, IPG_PREAM_SHRINK_ENA, (1<<(vtss_state->port.port_int_0-27)) | (1<<(vtss_state->port.port_int_1-27)));
        JR_WRF(DSM, CFG_IPG_SHRINK_CFG, IPG_SHRINK_ENA, (1<<(vtss_state->port.port_int_0-27)) | (1<<(vtss_state->port.port_int_1-27)));
        
        for (port = vtss_state->port.port_int_0; port <= vtss_state->port.port_int_1; port++) {
            /* Port configuration */
            memset(&conf, 0, sizeof(conf));
            conf.if_type = VTSS_PORT_INTERFACE_XAUI;
            conf.speed = VTSS_SPEED_12G;
            conf.fdx = 1;
            conf.max_frame_length = VTSS_MAX_FRAME_LENGTH_MAX;
            VTSS_RC(jr_port_conf_10g_set(vtss_state, VTSS_PORT_NO_NONE, port, &conf, 0));
            
            /* Shrink preamble to 1 byte */
            JR_WRXF(DEV10G, MAC_CFG_STATUS_MAC_MODE_CFG, VTSS_TO_DEV(port), MAC_PREAMBLE_CFG, 1);

            /* Shrink the IPG to 4-7 bytes */
            JR_WRXB(DEV10G, MAC_CFG_STATUS_MAC_MODE_CFG, VTSS_TO_DEV(port), MAC_IPG_CFG, 1);
           
            /* Source masks: Exclude internal ports */
            VTSS_RC(vtss_jr1_src_table_write_chip(vtss_state, port, 0xffffffff - vtss_state->port.mask_int_ports));
            
            /* Setup logical port numbers */
            JR_WRX(DSM, CM_CFG_LPORT_NUM_CFG, port, port);
            JR_WRX(ANA_CL_2, PORT_PORT_ID_CFG, port,
                   JR_PUT_FLD(ANA_CL_2, PORT_PORT_ID_CFG, LPORT_NUM, port));
            
            /* Destination masks: Include internal ports */
            VTSS_RC(vtss_jr1_pgid_table_write_chip(vtss_state, port, vtss_state->port.mask_int_ports, 0, 0, 0));

            /* VLAN port configuration */
            VTSS_RC(vtss_jr1_vlan_port_conf_apply(vtss_state, port,
                                                  &vtss_state->l2.vlan_port_conf[VTSS_PORT_NO_START], NULL));
            
            /* Enable first IS2 lookup on internal ports for SSM */
            VTSS_RC(vtss_jr1_vcap_is1_is2_set(vtss_state, port, 0, IS2_LOOKUP_FIRST));
        }

        /* Use policers to avoid CPU copy */
        VTSS_RC(vtss_jr1_cpu_pol_setup_chip(vtss_state));

        /* Update VStaX configuration */
        VTSS_RC(vtss_jr1_vstax_conf_set_chip(vtss_state));
    }
    
    return VTSS_RC_OK;
}
#endif /* VTSS_FEATURE_VSTAX_V2 */

#define JR_SDX_CNT_SET(tgt, sdx, idx, frames, mask)                        \
{                                                                          \
    JR_WRX(tgt, STAT_GLOBAL_CFG_##sdx##_STAT_GLOBAL_CFG, idx,              \
           JR_PUT_BIT(tgt, STAT_GLOBAL_CFG_##sdx##_STAT_GLOBAL_CFG,        \
                      GLOBAL_CFG_CNT_BYTE, (frames) ? 0 : 1));          \
    JR_WRX(tgt, STAT_GLOBAL_CFG_##sdx##_STAT_GLOBAL_EVENT_MASK, idx,       \
           JR_PUT_FLD(tgt, STAT_GLOBAL_CFG_##sdx##_STAT_GLOBAL_EVENT_MASK, \
                      GLOBAL_EVENT_MASK, mask));                           \
}                                                                                         

static vtss_rc jr_port_init(vtss_state_t *vtss_state)
{
    u32                  i, port, value, count;
    vtss_port_mux_mode_t mux_mode = vtss_state->port.mux_mode[vtss_state->chip_no];

    VTSS_RC(jr_init_hsio(vtss_state));
    VTSS_RC(jr_init_asm(vtss_state));
    VTSS_RC(jr_init_ana(vtss_state));
    VTSS_RC(jr_init_sqs(vtss_state));
    VTSS_RC(jr_init_arb(vtss_state));
    VTSS_RC(jr_init_sch(vtss_state));
    VTSS_RC(jr_init_rew(vtss_state));
    VTSS_RC(jr_init_dsm(vtss_state));

#if defined (VTSS_ARCH_JAGUAR_1_CE_MAC)
    VTSS_RC(jr_init_cemax(vtss_state));
#endif /* VTSS_ARCH_JAGUAR_1_CE_MAC */

    /* Setup ANA_AC to count local drops per port */
    JR_WRX(ANA_AC, PS_STICKY_MASK_STICKY_MASK, 0, 
           VTSS_F_ANA_AC_PS_STICKY_MASK_STICKY_MASK_ZERO_DST_STICKY_MASK);
    JR_WRX(ANA_AC, STAT_GLOBAL_CFG_PORT_STAT_GLOBAL_EVENT_MASK, 0,
           VTSS_F_ANA_AC_STAT_GLOBAL_CFG_PORT_STAT_GLOBAL_EVENT_MASK_GLOBAL_EVENT_MASK(1<<0));
    for (port = 0; port < VTSS_CHIP_PORTS; port++) {
        JR_WRXY(ANA_AC, STAT_CNT_CFG_PORT_STAT_CFG, port, JR_CNT_ANA_AC_FILTER,
                VTSS_F_ANA_AC_STAT_CNT_CFG_PORT_STAT_CFG_CFG_PRIO_MASK(0xff) |
                VTSS_F_ANA_AC_STAT_CNT_CFG_PORT_STAT_CFG_CFG_CNT_FRM_TYPE(3));
    }

    /* Setup SDX/VLAN statistics:
       - Even counters (0,2,4) are byte counters
       - Odd counters (1,3,5) are frame counters */
    for (i = 0; i < 6; i++) {
        /* ANA_AC */
#if defined(VTSS_FEATURE_VLAN_COUNTERS)
        value = (1<<((i/2)+3));
#else
        value = (1<<(i/2));
#endif /* VTSS_FEATURE_VLAN_COUNTERS */
        JR_SDX_CNT_SET(ANA_AC, ISDX, i, i & 1, value);
    }

    for (i = 0; i < 2; i++) {
        /* IQS/OQS */
        JR_SDX_CNT_SET(IQS, ISDX, i, i & 1, (1<<10) | (1<<11));
        JR_SDX_CNT_SET(OQS, ISDX, i, i & 1, (1<<10) | (1<<11));
    }
    for (i = 0; i < 4; i++) {
        /* REW */
        JR_SDX_CNT_SET(REW, ESDX, i, i & 1, i < 2 ? 0x1 : 0xe);
    }

    /* Setup all 10G ports to avoid delayed SP injection */
    count = (jr_target_get(vtss_state) == VTSS_TARGET_CE_MAX_12 ? 2 : 4);
    for (i = 0; i < count; i++) {
        port = (VTSS_PORT_10G_START + i);
        if (mux_mode == VTSS_PORT_MUX_MODE_1 && port == 27)
            continue; /* Port 27 is not active */
        else if (mux_mode == VTSS_PORT_MUX_MODE_7 && port < 29)
            continue; /* Port 27,28 are not active */
                
        JR_WRX(DSM, CFG_RATE_CTRL, port,
               VTSS_F_DSM_CFG_RATE_CTRL_FRM_GAP_COMP(JR_RATE_CTRL_GAP_10G) |
               VTSS_F_DSM_CFG_RATE_CTRL_TAXI_RATE_HIGH(JR_RATE_CTRL_WM_HI_10G) |
               VTSS_F_DSM_CFG_RATE_CTRL_TAXI_RATE_LOW(JR_RATE_CTRL_WM_LO_10G));
    }

    return VTSS_RC_OK;
}

vtss_rc vtss_jr1_port_init(vtss_state_t *vtss_state, vtss_init_cmd_t cmd)
{
    vtss_port_state_t *state = &vtss_state->port;
    vtss_rc           rc = VTSS_RC_OK;
    
    switch (cmd) {
    case VTSS_INIT_CMD_CREATE:
        state->miim_read = jr_miim_read;
        state->miim_write = jr_miim_write;
        state->mmd_read = jr_mmd_read;
        state->mmd_read_inc = jr_mmd_read_inc;
        state->mmd_write = jr_mmd_write;
        state->conf_set = jr_port_conf_set;
        state->clause_37_status_get = jr_port_clause_37_status_get;
        state->clause_37_control_set = jr_port_clause_37_control_set;
        state->status_get = jr_port_status_get;
        state->counters_update = jr_port_counters_update;
        state->counters_clear = jr_port_counters_clear;
        state->counters_get = jr_port_counters_get;
        state->basic_counters_get = jr_port_basic_counters_get;
        state->forward_set = jr_port_forward_set;
#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC)
        state->host_conf_set = jr_xaui_conf_set;
        state->is_host       = jr_port_is_host;
        state->lport_counters_get = jr_lport_counters_get;
        state->lport_counters_clear = jr_lport_counters_clear;
#endif
#if defined(VTSS_FEATURE_SYNCE)
        vtss_state->synce.clock_out_set = jr_synce_clock_out_set;
        vtss_state->synce.clock_in_set = jr_synce_clock_in_set;
#endif /* VTSS_FEATURE_SYNCE */
        vtss_state->cil.qs_conf_set = jr_qs_conf_set;
        break;
    case VTSS_INIT_CMD_INIT:
        rc = jr_port_init(vtss_state);
        break;
    case VTSS_INIT_CMD_PORT_MAP:
#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC)
        VTSS_RC(jr_cemax_port_map_set(vtss_state));
#endif /* defined(VTSS_ARCH_JAGUAR_1_CE_MAC) */
        
#if defined(VTSS_FEATURE_VSTAX_V2)
        /* Initialize internal ports */
        VTSS_RC(jr_init_ports_int(vtss_state));
#endif /* VTSS_FEATURE_VSTAX_V2 */
        
        /* Setup WM for chip ports (for some targets) */
        VTSS_RC(jr_update_sqs(vtss_state));
        break;
    default:
        break;
    }
    
    return rc;
}

#endif /* VTSS_ARCH_JAGUAR_1 */
