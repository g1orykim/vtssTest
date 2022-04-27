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

#define VTSS_TRACE_GROUP VTSS_TRACE_GROUP_VCAP
#include "vtss_jaguar1_cil.h"

#if defined(VTSS_ARCH_JAGUAR_1)

/* - CIL functions ------------------------------------------------- */

typedef struct {
    u32 target;  /* Target offset */
    u16 entries; /* Number of entries */
    u16 actions; /* Number of actions */
} tcam_props_t;

/* Last policer used to discard frames to avoid CPU copies */
#define JR_IS2_POLICER_DISCARD 31

/* VCAP value/mask register */
typedef struct {
    BOOL valid;
    u32  value;
    u32  mask;
} jr_vcap_reg_t;

/* VCAP MAC address registers */
typedef struct {
    jr_vcap_reg_t mach;
    jr_vcap_reg_t macl;
} jr_vcap_mac_t;

/* IS1 entry registers */
typedef struct {
    jr_vcap_reg_t entry;   /* ENTRY */
    jr_vcap_reg_t if_grp;  /* IF_GRP */
    jr_vcap_reg_t vlan;    /* VLAN */
    jr_vcap_reg_t flags;   /* FLAGS */
    jr_vcap_mac_t mac;     /* L2_MAC_ADDR_* */
    jr_vcap_reg_t sip;     /* L3_IP4_SIP */
    jr_vcap_reg_t l3_misc; /* L3_MISC */
    jr_vcap_reg_t l4_misc; /* L4_MISC */
    jr_vcap_reg_t l4_port; /* L4_PORT */
} jr_vcap_is1_regs_t;

/* IS2 entry registers */
typedef struct {
    jr_vcap_reg_t vld;        /* ACE_VLD */
    jr_vcap_reg_t entry_type; /* ACE_ENTRY_TYPE */
    jr_vcap_reg_t type;       /* ACE_TYPE */
    jr_vcap_reg_t port_mask;  /* ACE_IGR_PORT_MASK */
    jr_vcap_reg_t pag;        /* ACE_PAG */
    jr_vcap_mac_t smac;       /* ACE_L2_SMAC_* */
    jr_vcap_mac_t dmac;       /* ACE_L2_DMAC_* */
    jr_vcap_reg_t sip;        /* ACE_L3_IP4_SIP */
    jr_vcap_reg_t dip;        /* ACE_L3_IP4_DIP */
    jr_vcap_reg_t l2_misc;    /* ACE_L2_MISC */
    jr_vcap_reg_t l3_misc;    /* ACE_L3_MISC */
    jr_vcap_reg_t data[8];    /* ACE_CUSTOM_DATA_0 - ACE_CUSTOM_DATA_7 */
} jr_vcap_is2_regs_t;

/* Write VCAP value and mask */
#define JR_VCAP_WR(tgt, pre, suf, reg)        \
{                                             \
    JR_WR(tgt, pre##_ENTRY_##suf, reg.value); \
    JR_WR(tgt, pre##_MASK_##suf, ~reg.mask);  \
}

/* Put bit into VCAP value and mask */
#define JR_VCAP_PUT_BIT(tgt, pre, suf, fld, reg, val)                                     \
{                                                                                         \
    reg.value |= JR_PUT_BIT(tgt, pre##_ENTRY_##suf, fld, (val) == VTSS_VCAP_BIT_1 ? 1 : 0); \
    reg.mask |= JR_PUT_BIT(tgt, pre##_MASK_##suf, fld, (val) == VTSS_VCAP_BIT_ANY ? 0 : 1); \
}

/* Set VCAP bit to 1 */
#define JR_VCAP_SET_BIT(tgt, pre, suf, fld, reg)          \
{                                                         \
    reg.value |= JR_SET_BIT(tgt, pre##_ENTRY_##suf, fld); \
    reg.mask |= JR_SET_BIT(tgt, pre##_MASK_##suf, fld);   \
}

/* Set VCAP bit to 0 */
#define JR_VCAP_CLR_BIT(tgt, pre, suf, fld, reg) \
    reg.mask |= JR_SET_BIT(tgt, pre##_MASK_##suf, fld)

/* Put field into VCAP value and mask */
#define JR_VCAP_PUT_FLD_VM(tgt, pre, suf, fld, reg, val, msk)  \
{                                                              \
    reg.value |= JR_PUT_FLD(tgt, pre##_ENTRY_##suf, fld, val); \
    reg.mask |= JR_PUT_FLD(tgt, pre##_MASK_##suf, fld, msk);   \
}

/* Put field into VCAP value and mask */
#define JR_VCAP_PUT_FLD(tgt, pre, suf, fld, reg, vm) \
    JR_VCAP_PUT_FLD_VM(tgt, pre, suf, fld, reg, vm.value, vm.mask)

/* Put U16 field into VCAP value and mask */
#define JR_VCAP_PUT_U16(tgt, pre, suf, fld, reg, vm) \
    JR_VCAP_PUT_FLD_VM(tgt, pre, suf, fld, reg, (vm.value[0]<<8) | vm.value[1], (vm.mask[0]<<8) | vm.mask[1])

/* Put U32 field into VCAP value and mask based offset by index */
#define JR_VCAP_PUT_U32_NDX(reg, vm, ndx)                        \
{                                                                \
    reg.value = ((vm.value[0+ndx]<<24) | (vm.value[1+ndx]<<16) | \
                 (vm.value[2+ndx]<<8) | vm.value[3+ndx]);        \
    reg.mask = ((vm.mask[0+ndx]<<24) | (vm.mask[1+ndx]<<16) |    \
                (vm.mask[2+ndx]<<8) | vm.mask[3+ndx]);           \
}

/* Put U32 field into VCAP value and mask */
#define JR_VCAP_PUT_U32(reg, vm) JR_VCAP_PUT_U32_NDX(reg, vm, 0)

/* Put U40 field into VCAP value and mask */
#define JR_VCAP_PUT_U40(mac, vm)                                                               \
{                                                                                              \
    mac.mach.value = (vm.value[0]);                                                            \
    mac.mach.mask = (vm.mask[0]);                                                              \
    mac.macl.value = ((vm.value[1]<<24) | (vm.value[2]<<16) | (vm.value[3]<<8) | vm.value[4]); \
    mac.macl.mask = ((vm.mask[1]<<24) | (vm.mask[2]<<16) | (vm.mask[3]<<8) | vm.mask[4]);      \
}

/* Put U48 field into VCAP value and mask */
#define JR_VCAP_PUT_U48(mac, vm)                                                               \
{                                                                                              \
    mac.mach.value = ((vm.value[0]<<8) | vm.value[1]);                                         \
    mac.mach.mask = ((vm.mask[0]<<8) | vm.mask[1]);                                            \
    mac.macl.value = ((vm.value[2]<<24) | (vm.value[3]<<16) | (vm.value[4]<<8) | vm.value[5]); \
    mac.macl.mask = ((vm.mask[2]<<24) | (vm.mask[3]<<16) | (vm.mask[4]<<8) | vm.mask[5]);      \
}

/* Put U48 MAC field into VCAP value and mask (3 bytes in mach and 3 bytes in macl */
#define JR_VCAP_PUT_MAC(mac, vm)                                                               \
{                                                                                              \
    mac.mach.value = ((vm.value[0]<<16) | (vm.value[1]<<8) | vm.value[2]);                     \
    mac.mach.mask  = ((vm.mask[0] <<16) | (vm.mask[1] <<8) | vm.mask[2]);                      \
    mac.macl.value = ((vm.value[3]<<16) | (vm.value[4]<<8) | vm.value[5]);                     \
    mac.macl.mask  = ((vm.mask[3] <<16) | (vm.mask[4] <<8) | vm.mask[5]);                      \
}

/* Write VCAP MAC address value and mask */
#define JR_VCAP_WR_MAC(tgt, pre, suf, vm)            \
{                                                    \
    jr_vcap_mac_t vcap_mac;                          \
    JR_VCAP_PUT_U48(vcap_mac, vm);                   \
    JR_VCAP_WR(tgt, pre, suf##_HIGH, vcap_mac.mach); \
    JR_VCAP_WR(tgt, pre, suf##_LOW, vcap_mac.macl);  \
}

/* Read VCAP value and mask */
#define JR_VCAP_RD(tgt, pre, suf, reg)         \
{                                              \
    reg.valid = 1;                             \
    JR_RD(tgt, pre##_ENTRY_##suf, &reg.value); \
    JR_RD(tgt, pre##_MASK_##suf, &reg.mask);   \
}

/* Read VCAP MAC address */
#define JR_VCAP_RD_MAC(tgt, pre, suf, reg)       \
{                                                \
    JR_VCAP_RD(tgt, pre, suf##_HIGH, reg.mach); \
    JR_VCAP_RD(tgt, pre, suf##_LOW, reg.macl);   \
}

/* Debug VCAP bit */
#define JR_VCAP_DEBUG_BIT(pr, name, tgt, pre, suf, fld, reg) \
    jr_debug_vcap_reg(pr, name, reg, VTSS_F_##tgt##_##pre##_ENTRY_##suf##_##fld)

/* Debug VCAP field */
#define JR_VCAP_DEBUG_FLD(pr, name, tgt, pre, suf, fld, reg)             \
    jr_debug_vcap_reg(pr, name, reg, VTSS_M_##tgt##_##pre##_ENTRY_##suf##_##fld)

/* IS0 VCAP macros */
#define JR_IS0_WR(pre, suf, reg) JR_VCAP_WR(VCAP_IS0, pre, suf, reg)
#define JR_IS0_WR_MAC(pre, suf, mac) JR_VCAP_WR_MAC(VCAP_IS0, pre, suf, mac)
#define JR_IS0_PUT_BIT(pre, suf, fld, reg, val) \
    JR_VCAP_PUT_BIT(VCAP_IS0, pre, suf, fld, reg, val)
#define JR_IS0_SET_BIT(pre, suf, fld, reg) JR_VCAP_SET_BIT(VCAP_IS0, pre, suf, fld, reg)
#define JR_IS0_CLR_BIT(pre, suf, fld, reg) JR_VCAP_CLR_BIT(VCAP_IS0, pre, suf, fld, reg)
#define JR_IS0_PUT_FLD_VM(pre, suf, fld, reg, val, msk) \
    JR_VCAP_PUT_FLD_VM(VCAP_IS0, pre, suf, fld, reg, val, msk)
#define JR_IS0_PUT_FLD(pre, suf, fld, reg, vm) \
    JR_VCAP_PUT_FLD(VCAP_IS0, pre, suf, fld, reg, vm)
#define JR_IS0_PUT_U16(pre, suf, fld, reg, vm) \
    JR_VCAP_PUT_U16(VCAP_IS0, pre, suf, fld, reg, vm)
#define JR_IS0_RD(pre, suf, reg) JR_VCAP_RD(VCAP_IS0, pre, suf, reg)
#define JR_IS0_RD_MAC(pre, suf, reg) JR_VCAP_RD_MAC(VCAP_IS0, pre, suf, reg)
#define JR_IS0_DEBUG_BIT(pr, name, pre, suf, fld, reg) \
    JR_VCAP_DEBUG_BIT(pr, name, VCAP_IS0, pre, suf, fld, reg)
#define JR_IS0_DEBUG_FLD(pr, name, pre, suf, fld, reg) \
    JR_VCAP_DEBUG_FLD(pr, name, VCAP_IS0, pre, suf, fld, reg)

/* IS1 VCAP macros */
#define JR_IS1_WR(pre, suf, reg) JR_VCAP_WR(VCAP_IS1, pre, suf, reg)
#define JR_IS1_WR_MAC(pre, suf, mac) JR_VCAP_WR_MAC(VCAP_IS1, pre, suf, mac)
#define JR_IS1_PUT_BIT(pre, suf, fld, reg, val) \
    JR_VCAP_PUT_BIT(VCAP_IS1, pre, suf, fld, reg, val)
#define JR_IS1_SET_BIT(pre, suf, fld, reg) JR_VCAP_SET_BIT(VCAP_IS1, pre, suf, fld, reg)
#define JR_IS1_CLR_BIT(pre, suf, fld, reg) JR_VCAP_CLR_BIT(VCAP_IS1, pre, suf, fld, reg)
#define JR_IS1_PUT_FLD_VM(pre, suf, fld, reg, val, msk) \
    JR_VCAP_PUT_FLD_VM(VCAP_IS1, pre, suf, fld, reg, val, msk)
#define JR_IS1_PUT_FLD(pre, suf, fld, reg, vm) \
    JR_VCAP_PUT_FLD(VCAP_IS1, pre, suf, fld, reg, vm)
#define JR_IS1_PUT_U16(pre, suf, fld, reg, vm) \
    JR_VCAP_PUT_U16(VCAP_IS1, pre, suf, fld, reg, vm)
#define JR_IS1_RD(pre, suf, reg) JR_VCAP_RD(VCAP_IS1, pre, suf, reg)
#define JR_IS1_RD_MAC(pre, suf, reg) JR_VCAP_RD_MAC(VCAP_IS1, pre, suf, reg)
#define JR_IS1_DEBUG_BIT(pr, name, pre, suf, fld, reg) \
    JR_VCAP_DEBUG_BIT(pr, name, VCAP_IS1, pre, suf, fld, reg)
#define JR_IS1_DEBUG_FLD(pr, name, pre, suf, fld, reg) \
    JR_VCAP_DEBUG_FLD(pr, name, VCAP_IS1, pre, suf, fld, reg)

/* IS2 VCAP macros */
#define JR_IS2_WR(pre, suf, reg) JR_VCAP_WR(VCAP_IS2, pre, suf, reg)
#define JR_IS2_WR_MAC(pre, suf, mac) JR_VCAP_WR_MAC(VCAP_IS2, pre, suf, mac)
#define JR_IS2_PUT_BIT(pre, suf, fld, reg, val) \
    JR_VCAP_PUT_BIT(VCAP_IS2, pre, suf, fld, reg, val)
#define JR_IS2_SET_BIT(pre, suf, fld, reg) JR_VCAP_SET_BIT(VCAP_IS2, pre, suf, fld, reg)
#define JR_IS2_CLR_BIT(pre, suf, fld, reg) JR_VCAP_CLR_BIT(VCAP_IS2, pre, suf, fld, reg)
#define JR_IS2_PUT_FLD_VM(pre, suf, fld, reg, val, msk) \
    JR_VCAP_PUT_FLD_VM(VCAP_IS2, pre, suf, fld, reg, val, msk)
#define JR_IS2_PUT_FLD(pre, suf, fld, reg, vm) \
    JR_VCAP_PUT_FLD(VCAP_IS2, pre, suf, fld, reg, vm)
#define JR_IS2_PUT_U16(pre, suf, fld, reg, vm) \
    JR_VCAP_PUT_U16(VCAP_IS2, pre, suf, fld, reg, vm)
#define JR_IS2_RD(pre, suf, reg) JR_VCAP_RD(VCAP_IS2, pre, suf, reg)
#define JR_IS2_RD_MAC(pre, suf, reg) JR_VCAP_RD_MAC(VCAP_IS2, pre, suf, reg)
#define JR_IS2_DEBUG_BIT(pr, name, pre, suf, fld, reg) \
    JR_VCAP_DEBUG_BIT(pr, name, VCAP_IS2, pre, suf, fld, reg)
#define JR_IS2_DEBUG_FLD(pr, name, pre, suf, fld, reg) \
    JR_VCAP_DEBUG_FLD(pr, name, VCAP_IS2, pre, suf, fld, reg)

/* ES0 VCAP macros */
#define JR_ES0_WR(pre, suf, reg) JR_VCAP_WR(VCAP_ES0, pre, suf, reg)
#define JR_ES0_WR_MAC(pre, suf, mac) JR_VCAP_WR_MAC(VCAP_ES0, pre, suf, mac)
#define JR_ES0_PUT_BIT(pre, suf, fld, reg, val) \
    JR_VCAP_PUT_BIT(VCAP_ES0, pre, suf, fld, reg, val)
#define JR_ES0_SET_BIT(pre, suf, fld, reg) JR_VCAP_SET_BIT(VCAP_ES0, pre, suf, fld, reg)
#define JR_ES0_CLR_BIT(pre, suf, fld, reg) JR_VCAP_CLR_BIT(VCAP_ES0, pre, suf, fld, reg)
#define JR_ES0_PUT_FLD_VM(pre, suf, fld, reg, val, msk) \
    JR_VCAP_PUT_FLD_VM(VCAP_ES0, pre, suf, fld, reg, val, msk)
#define JR_ES0_PUT_FLD(pre, suf, fld, reg, vm) \
    JR_VCAP_PUT_FLD(VCAP_ES0, pre, suf, fld, reg, vm)
#define JR_ES0_PUT_U16(pre, suf, fld, reg, vm) \
    JR_VCAP_PUT_U16(VCAP_ES0, pre, suf, fld, reg, vm)
#define JR_ES0_RD(pre, suf, reg) JR_VCAP_RD(VCAP_ES0, pre, suf, reg)
#define JR_ES0_RD_MAC(pre, suf, reg) JR_VCAP_RD_MAC(VCAP_ES0, pre, suf, reg)
#define JR_ES0_DEBUG_BIT(pr, name, pre, suf, fld, reg) \
    JR_VCAP_DEBUG_BIT(pr, name, VCAP_ES0, pre, suf, fld, reg)
#define JR_ES0_DEBUG_FLD(pr, name, pre, suf, fld, reg) \
    JR_VCAP_DEBUG_FLD(pr, name, VCAP_ES0, pre, suf, fld, reg)

static const tcam_props_t tcam_info[] = {
    [VTSS_TCAM_IS0] = {
        .target = VTSS_TO_VCAP_IS0,
        .entries = VTSS_JR1_IS0_CNT,
        .actions = VTSS_JR1_IS0_CNT + 35
    },
    [VTSS_TCAM_IS1] = {
        .target = VTSS_TO_VCAP_IS1,
        .entries = VTSS_JR1_IS1_CNT,
        .actions = VTSS_JR1_IS1_CNT + 2
    },
    [VTSS_TCAM_IS2] = {
        .target = VTSS_TO_VCAP_IS2,
        .entries = VTSS_JR1_IS2_CNT,
        .actions =  VTSS_JR1_IS2_CNT + 35
    },
    [VTSS_TCAM_ES0] = {
        .target = VTSS_TO_VCAP_ES0,
        .entries = VTSS_JR1_ES0_CNT,
        .actions = VTSS_JR1_ES0_CNT + 1
    }
};

static vtss_rc jr_vcap_command(vtss_state_t *vtss_state, int bank, u16 ix, int cmd)
{
    u32 value;

    if (bank == VTSS_TCAM_IS0) {
        JR_WR(VCAP_IS0, IS0_CONTROL_ACE_UPDATE_CTRL, 
              VTSS_F_VCAP_IS0_IS0_CONTROL_ACE_UPDATE_CTRL_ACE_INDEX(ix) |
              VTSS_F_VCAP_IS0_IS0_CONTROL_ACE_UPDATE_CTRL_ACE_UPDATE_CMD(cmd) |
              VTSS_F_VCAP_IS0_IS0_CONTROL_ACE_UPDATE_CTRL_ACE_UPDATE_SHOT);
        do {
            JR_RDB(VCAP_IS0, IS0_CONTROL_ACE_UPDATE_CTRL, ACE_UPDATE_SHOT, &value);
        } while (value);
    } else if (bank == VTSS_TCAM_IS1) {
        JR_WR(VCAP_IS1, IS1_CONTROL_ACE_UPDATE_CTRL, 
              VTSS_F_VCAP_IS1_IS1_CONTROL_ACE_UPDATE_CTRL_ACE_INDEX(ix) |
              VTSS_F_VCAP_IS1_IS1_CONTROL_ACE_UPDATE_CTRL_ACE_UPDATE_CMD(cmd) |
              VTSS_F_VCAP_IS1_IS1_CONTROL_ACE_UPDATE_CTRL_ACE_UPDATE_SHOT);
        do {
            JR_RDB(VCAP_IS1, IS1_CONTROL_ACE_UPDATE_CTRL, ACE_UPDATE_SHOT, &value);
        } while (value);
    } else if (bank == VTSS_TCAM_IS2) {
        JR_WR(VCAP_IS2, IS2_CONTROL_ACE_UPDATE_CTRL, 
              VTSS_F_VCAP_IS2_IS2_CONTROL_ACE_UPDATE_CTRL_ACE_INDEX(ix) |
              VTSS_F_VCAP_IS2_IS2_CONTROL_ACE_UPDATE_CTRL_ACE_UPDATE_CMD(cmd) |
              VTSS_F_VCAP_IS2_IS2_CONTROL_ACE_UPDATE_CTRL_ACE_UPDATE_SHOT);
        do {
            JR_RDB(VCAP_IS2, IS2_CONTROL_ACE_UPDATE_CTRL, ACE_UPDATE_SHOT, &value);
        } while (value);
    } else if (bank == VTSS_TCAM_ES0) {
        JR_WR(VCAP_ES0, ES0_CONTROL_ACE_UPDATE_CTRL, 
              VTSS_F_VCAP_ES0_ES0_CONTROL_ACE_UPDATE_CTRL_ACE_INDEX(ix) |
              VTSS_F_VCAP_ES0_ES0_CONTROL_ACE_UPDATE_CTRL_ACE_UPDATE_CMD(cmd) |
              VTSS_F_VCAP_ES0_ES0_CONTROL_ACE_UPDATE_CTRL_ACE_UPDATE_SHOT);
        do {
            JR_RDB(VCAP_ES0, ES0_CONTROL_ACE_UPDATE_CTRL, ACE_UPDATE_SHOT, &value);
        } while (value);
    } else {
        VTSS_E("illegal bank: %d", bank);
        return VTSS_RC_ERROR;
    }

    return VTSS_RC_OK;
}

vtss_rc vtss_jr1_vcap_port_command(vtss_state_t *vtss_state, int bank, u32 port, int cmd)
{
    return jr_vcap_command(vtss_state, bank, tcam_info[bank].entries + port, cmd);
}

static vtss_rc jr_vcap_index_command(vtss_state_t *vtss_state, int bank, u32 ix, int cmd)
{
    return jr_vcap_command(vtss_state, bank, tcam_info[bank].entries - ix - 1, cmd);
}

static vtss_rc jr_is2_prepare_action(vtss_state_t *vtss_state,
                                     vtss_acl_action_t *action, u32 action_ext, u32 counter)
{
    BOOL police = action->police;
    u32  policer_no = action->policer_no;
    BOOL rx_timestamp = 0;
    BOOL redir_ena = 0;

    if (action->cpu || action->cpu_once) {
        /* Enable Rx timestamping for all frames to CPU (for PTP) */
        rx_timestamp = 1;
    } else if (action->forward == 0) {
        /* If discarding and not forwarding to CPU, use discard policer to avoid CPU copy */
        police = 1;
        policer_no = JR_IS2_POLICER_DISCARD;
    }

    JR_WR(VCAP_IS2, BASETYPE_ACTION_A,
          JR_PUT_BIT(VCAP_IS2, BASETYPE_ACTION_A, HIT_ME_ONCE, action->cpu_once) |
          JR_PUT_BIT(VCAP_IS2, BASETYPE_ACTION_A, CPU_COPY_ENA, action->cpu) |
          JR_PUT_FLD(VCAP_IS2, BASETYPE_ACTION_A, CPU_QU_NUM, action->cpu_queue) |
          JR_PUT_BIT(VCAP_IS2, BASETYPE_ACTION_A, IRQ_TRIGGER, action->irq_trigger) |
          JR_PUT_BIT(VCAP_IS2, BASETYPE_ACTION_A, FW_ENA, action->forward) |
          JR_PUT_BIT(VCAP_IS2, BASETYPE_ACTION_A, LRN_ENA, action->learn) |
          JR_PUT_BIT(VCAP_IS2, BASETYPE_ACTION_A, POLICE_ENA, police) |
          JR_PUT_FLD(VCAP_IS2, BASETYPE_ACTION_A, POLICE_IDX, policer_no) |
          JR_PUT_BIT(VCAP_IS2, BASETYPE_ACTION_A, OAM_RX_TIMESTAMP_ENA, rx_timestamp) |
          JR_PUT_BIT(VCAP_IS2, BASETYPE_ACTION_A, RT_ENA, action->learn));

    if (action->port_forward && action->port_no < vtss_state->port_count && 
        VTSS_PORT_CHIP_SELECTED(action->port_no)) {
        /* It is only possible to redirect to ports on the same device */
        redir_ena = 1;
    }
    JR_WR(VCAP_IS2, BASETYPE_ACTION_B,
          JR_PUT_BIT(VCAP_IS2, BASETYPE_ACTION_B, REDIR_ENA, redir_ena) |
          JR_PUT_FLD(VCAP_IS2, BASETYPE_ACTION_B, REDIR_PGID, 
                     redir_ena ? vtss_jr1_chip_pgid(vtss_state, action->port_no) : action_ext));
          
    JR_WR(VCAP_IS2, BASETYPE_ACTION_C, 0xffffffff);
    
    JR_WR(VCAP_IS2, BASETYPE_ACTION_D, counter);

    return VTSS_RC_OK;
}

/* ================================================================= *
 *  VCAP functions
 * ================================================================= */

/* Get VCAP entry from all chips */
static vtss_rc jr_vcap_entry_get_chips(vtss_state_t *vtss_state, 
                                       vtss_rc (* func_get)(vtss_state_t *, u32 ix, u32 *cnt, BOOL clear),
                                       u32 ix, u32 *counter, BOOL clear)
{
    vtss_chip_no_t chip_no;
    u32            cnt;

    /* Read/clear from all devices */
    *counter = 0;
    for (chip_no = 0; chip_no < vtss_state->chip_count; chip_no++) {
        VTSS_SELECT_CHIP(chip_no);
        VTSS_RC(func_get(vtss_state, ix, &cnt, clear));
        *counter += cnt;
        vtss_state->vcap.counter[chip_no] = cnt;
    }
    return VTSS_RC_OK;
}

/* Add VCAP entry to all devices */
static vtss_rc jr_vcap_entry_add_chips(vtss_state_t *vtss_state,
                                       vtss_rc (* func_add)(vtss_state_t *, u32 ix, vtss_vcap_data_t *data, u32 cnt),
                                       u32 ix, vtss_vcap_data_t *data, u32 counter)
{
    vtss_chip_no_t chip_no;

    for (chip_no = 0; chip_no < vtss_state->chip_count; chip_no++) {
        VTSS_SELECT_CHIP(chip_no);
        VTSS_RC(func_add(vtss_state, ix, data, counter == 0 ? 0 : vtss_state->vcap.counter[chip_no]));
    }
    return VTSS_RC_OK;
}

/* Delete VCAP entry from all devices */
static vtss_rc jr_vcap_entry_del_chips(vtss_state_t *vtss_state,
                                       vtss_rc (* func_del)(vtss_state_t *, u32 ix), u32 ix)
{
    vtss_chip_no_t chip_no;

    for (chip_no = 0; chip_no < vtss_state->chip_count; chip_no++) {
        VTSS_SELECT_CHIP(chip_no);
        VTSS_RC(func_del(vtss_state, ix));
    }
    return VTSS_RC_OK;
}

/* Move VCAP entry for all devices */
static vtss_rc jr_vcap_entry_move_chips(vtss_state_t *vtss_state,
                                        vtss_rc (* func_move)(vtss_state_t *, u32 ix, u32 count, BOOL up),
                                        u32 ix, u32 count, BOOL up)
{
    vtss_chip_no_t chip_no;

    for (chip_no = 0; chip_no < vtss_state->chip_count; chip_no++) {
        VTSS_SELECT_CHIP(chip_no);
        VTSS_RC(func_move(vtss_state, ix, count, up));
    }
    return VTSS_RC_OK;
}

/* ================================================================= *
 *  IS0 functions
 * ================================================================= */

/* Get IS0 entry for chip */
static vtss_rc jr_is0_entry_get_chip(vtss_state_t *vtss_state, u32 ix, u32 *cnt, BOOL clear) 
{
    VTSS_RC(jr_vcap_index_command(vtss_state, VTSS_TCAM_IS0, ix, VTSS_TCAM_CMD_READ));
    JR_RD(VCAP_IS0, BASETYPE_ACTION_C, cnt);

    if (clear) {
        JR_WR(VCAP_IS0, BASETYPE_ACTION_C, 0);
        VTSS_RC(jr_vcap_index_command(vtss_state, VTSS_TCAM_IS0, ix, VTSS_TCAM_CMD_WRITE));
    }
    return VTSS_RC_OK;
}

static vtss_rc jr_is0_entry_get(vtss_state_t *vtss_state,
                                vtss_vcap_idx_t *idx, u32 *counter, BOOL clear) 
{
    VTSS_I("row: %u", idx->row);

    /* Read/clear from all devices */
    return jr_vcap_entry_get_chips(vtss_state, jr_is0_entry_get_chip, idx->row, counter, clear);
}

static vtss_rc jr_is0_entry_add_chip(vtss_state_t *vtss_state,
                                     u32 ix, vtss_vcap_data_t *data, u32 counter)
{
    vtss_is0_data_t   *is0 = &data->u.is0;
    vtss_is0_key_t    *key = &is0->entry->key;
    vtss_is0_action_t *action = &is0->entry->action;
    vtss_vcap_tag_t   *tag;
    vtss_is0_proto_t  proto;
    jr_vcap_reg_t     reg, reg1;
    u32               port;

    memset(&reg, 0, sizeof(reg));
    JR_IS0_SET_BIT(ISID, A, VLD, reg);
    if (key->port_no != VTSS_PORT_NO_NONE) {
        port = (VTSS_PORT_CHIP_SELECTED(key->port_no) ? VTSS_CHIP_PORT(key->port_no) : 0xff);
        JR_IS0_PUT_FLD_VM(ISID, A, VIGR_PORT, reg, port, 0xff);
    }

    switch (key->type) {
    case VTSS_IS0_TYPE_DBL_VID:
        JR_IS0_WR(DBL_VID, A, reg);

        /* DBL_VID0 and DBL_VID1 */
        memset(&reg, 0, sizeof(reg));
        memset(&reg1, 0, sizeof(reg1));
        
        /* Outer tag */
        tag = &key->data.dbl_vid.outer_tag;
        JR_IS0_PUT_BIT(DBL_VID, DBL_VID0, VLAN_TAGGED, reg, tag->tagged);
        if (tag->tagged == VTSS_VCAP_BIT_1) {
            /* Filtering on outer tag fields and inner tag only done if outer tag present */
            JR_IS0_PUT_BIT(DBL_VID, DBL_VID0, OUTER_DEI, reg, tag->dei);
            JR_IS0_PUT_FLD(DBL_VID, DBL_VID0, OUTER_PCP, reg, tag->pcp);
            JR_IS0_PUT_BIT(DBL_VID, DBL_VID0, OUTER_TPI, reg, tag->s_tag);
            JR_IS0_PUT_FLD(DBL_VID, DBL_VID1, OUTER_VID, reg1, tag->vid);

            /* Inner tag */
            tag = &key->data.dbl_vid.inner_tag;
            JR_IS0_PUT_BIT(DBL_VID, DBL_VID0, VLAN_DBL_TAGGED, reg, tag->tagged);
            if (tag->tagged == VTSS_VCAP_BIT_1) {
                JR_IS0_PUT_BIT(DBL_VID, DBL_VID0, INNER_DEI, reg, tag->dei);
                JR_IS0_PUT_FLD(DBL_VID, DBL_VID0, INNER_PCP, reg, tag->pcp);
                JR_IS0_PUT_BIT(DBL_VID, DBL_VID0, INNER_TPI, reg, tag->s_tag);
                JR_IS0_PUT_FLD(DBL_VID, DBL_VID1, INNER_VID, reg1, tag->vid);
            }
        }
        
        JR_IS0_WR(DBL_VID, DBL_VID0, reg);
        JR_IS0_WR(DBL_VID, DBL_VID1, reg1);
        
        /* DBL_VID2 */
        memset(&reg, 0, sizeof(reg));
        JR_IS0_PUT_FLD(DBL_VID, DBL_VID2, L3_DSCP, reg, key->data.dbl_vid.dscp);
        if ((proto = key->data.dbl_vid.proto) != VTSS_IS0_PROTO_ANY) {
            JR_IS0_PUT_FLD_VM(DBL_VID, DBL_VID2, PROT, reg, proto - 1, 0x3);
        }
        JR_IS0_WR(DBL_VID, DBL_VID2, reg);
        break;
    default:
        VTSS_E("illegal type: %d", key->type);
        return VTSS_RC_ERROR;
    }

    /* Action */
    JR_WR(VCAP_IS0, BASETYPE_ACTION_A,
          JR_PUT_BIT(VCAP_IS0, BASETYPE_ACTION_A, S1_DMAC_ENA, action->s1_dmac_ena) |
          JR_PUT_FLD(VCAP_IS0, BASETYPE_ACTION_A, VLAN_POP_CNT, action->vlan_pop_cnt) |
          JR_PUT_BIT(VCAP_IS0, BASETYPE_ACTION_A, VID_ENA, action->vid_ena ? 1 : 0) |
          JR_PUT_BIT(VCAP_IS0, BASETYPE_ACTION_A, DEI_VAL, action->dei) |
          JR_PUT_FLD(VCAP_IS0, BASETYPE_ACTION_A, PCP_VAL, action->pcp) |
          JR_PUT_BIT(VCAP_IS0, BASETYPE_ACTION_A, PCP_DEI_ENA, action->pcp_dei_ena));
    JR_WR(VCAP_IS0, BASETYPE_ACTION_B,
          JR_PUT_FLD(VCAP_IS0, BASETYPE_ACTION_B, ISDX_VAL, action->isdx) |
          JR_PUT_FLD(VCAP_IS0, BASETYPE_ACTION_B, VID_VAL, action->vid) |
          JR_PUT_FLD(VCAP_IS0, BASETYPE_ACTION_B, PAG_VAL, action->pag));
    JR_WR(VCAP_IS0, BASETYPE_ACTION_C, counter);

    return jr_vcap_index_command(vtss_state, VTSS_TCAM_IS0, ix, VTSS_TCAM_CMD_WRITE);
}

static vtss_rc jr_is0_entry_add(vtss_state_t *vtss_state,
                                vtss_vcap_idx_t *idx, vtss_vcap_data_t *data, u32 counter)
{
    VTSS_I("row: %u", idx->row);

    /* Add to all devices */
    return jr_vcap_entry_add_chips(vtss_state, jr_is0_entry_add_chip, idx->row, data, counter);
}

static vtss_rc jr_is0_entry_del_chip(vtss_state_t *vtss_state, u32 ix)
{
    JR_WRB(VCAP_IS0, ISID_ENTRY_A, VLD, 0);
    JR_WRB(VCAP_IS0, ISID_MASK_A, VLD, 0);
    VTSS_RC(jr_vcap_index_command(vtss_state, VTSS_TCAM_IS0, ix, VTSS_TCAM_CMD_WRITE));
    return VTSS_RC_OK;
}

static vtss_rc jr_is0_entry_del(vtss_state_t *vtss_state, vtss_vcap_idx_t *idx)
{
    VTSS_I("row: %u", idx->row);

    /* Delete from all devices */
    return jr_vcap_entry_del_chips(vtss_state, jr_is0_entry_del_chip, idx->row);
}

static vtss_rc jr_is0_entry_move_chip(vtss_state_t *vtss_state, u32 ix, u32 count, BOOL up)
{
    JR_WR(VCAP_IS0, IS0_CONTROL_ACE_MV_CFG, 
          JR_PUT_FLD(VCAP_IS0, IS0_CONTROL_ACE_MV_CFG, ACE_MV_NUM_POS, 1) |
          JR_PUT_FLD(VCAP_IS0, IS0_CONTROL_ACE_MV_CFG, ACE_MV_SIZE, count));
    VTSS_RC(jr_vcap_index_command(vtss_state, VTSS_TCAM_IS0, ix + count - 1, 
                                  up ? VTSS_TCAM_CMD_MOVE_UP : VTSS_TCAM_CMD_MOVE_DOWN));
    return VTSS_RC_OK;
}

static vtss_rc jr_is0_entry_move(vtss_state_t *vtss_state,
                                 vtss_vcap_idx_t *idx, u32 count, BOOL up)
{
    VTSS_I("row: %u, count: %u, up: %u", idx->row, count, up);

    /* Move for all devices */
    return jr_vcap_entry_move_chips(vtss_state, jr_is0_entry_move_chip, idx->row, count, up);
}

/* ================================================================= *
 *  IS1 functions
 * ================================================================= */
static BOOL jr_vcap_is_udp_tcp(vtss_vcap_u8_t *proto)
{
    return (proto->mask == 0xff && (proto->value == 6 || proto->value == 17));
}

static u32 jr_u8_to_u32(u8 *p)
{
    return ((p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3]);
}

/* Get IS1 entry for chip */
static vtss_rc jr_is1_entry_get_chip(vtss_state_t *vtss_state, u32 ix, u32 *cnt, BOOL clear) 
{
    u32 type;
    
    VTSS_RC(jr_vcap_index_command(vtss_state, VTSS_TCAM_IS1, ix, VTSS_TCAM_CMD_READ));
    JR_RDB(VCAP_IS1, IS1_CONTROL_ACE_STATUS, ACE_ACTION_TYPE, &type);

    if (type) { /* QOS_ACTION */
        JR_RDB(VCAP_IS1, QOS_ACTION_STICKY, HIT_STICKY, cnt);
    } else { /* VLAN_PAG_ACTION */
        JR_RDB(VCAP_IS1, VLAN_PAG_ACTION_STICKY, HIT_STICKY, cnt);
    }
    
    if (clear) {
        if (type) { /* QOS_ACTION */
            JR_WRB(VCAP_IS1, QOS_ACTION_STICKY, HIT_STICKY, 0);
        } else { /* VLAN_PAG_ACTION */
            JR_WRB(VCAP_IS1, VLAN_PAG_ACTION_STICKY, HIT_STICKY, 0);
        }
        VTSS_RC(jr_vcap_index_command(vtss_state, VTSS_TCAM_IS1, ix, VTSS_TCAM_CMD_WRITE));
    }
    return VTSS_RC_OK;
}

static vtss_rc jr_is1_entry_get(vtss_state_t *vtss_state,
                                vtss_vcap_idx_t *idx, u32 *counter, BOOL clear) 
{
    VTSS_I("row: %u", idx->row);

    /* Read/clear from all devices */
    return jr_vcap_entry_get_chips(vtss_state, jr_is1_entry_get_chip, idx->row, counter, clear);
}

static vtss_rc jr_is1_entry_add_chip(vtss_state_t *vtss_state,
                                     u32 ix, vtss_vcap_data_t *data, u32 counter)
{
    vtss_is1_data_t    *is1 = &data->u.is1;
    vtss_is1_key_t     *key = &is1->entry->key;
    vtss_is1_tag_t     *tag = &key->tag;
    vtss_is1_action_t  *action = &is1->entry->action;
    u32                etype_len = 0, ip_snap = 0, ip4 = 0, tcp_udp = 0, tcp = 0;
    vtss_vcap_u8_t     *proto = NULL;
    vtss_vcap_vr_t     *dscp = NULL, *sport = NULL, *dport = NULL;
    vtss_vcap_ip_t     sip;
    vtss_vcap_u16_t    etype;
    vtss_vcap_u8_t     range;
    jr_vcap_is1_regs_t regs;

    memset(&sip, 0, sizeof(sip));
    memset(&etype, 0, sizeof(etype));
    memset(&range, 0, sizeof(range));
    memset(&regs, 0, sizeof(regs));

    JR_IS1_SET_BIT(QOS, ENTRY, VLD, regs.entry);

    /* Enabled ports are don't cared, disabled ports must be 0 */
    regs.if_grp.mask = ~vtss_jr1_port_mask(vtss_state, key->port_list);

    JR_IS1_PUT_BIT(QOS, VLAN, VLAN_TAGGED, regs.vlan, tag->tagged);
    if (tag->vid.type == VTSS_VCAP_VR_TYPE_VALUE_MASK) {
        JR_IS1_PUT_FLD(QOS, VLAN, VID, regs.vlan, tag->vid.vr.v);
    }
    if (is1->vid_range != VTSS_VCAP_RANGE_CHK_NONE) {
        range.mask |= (1<<is1->vid_range);
        if (tag->vid.type == VTSS_VCAP_VR_TYPE_RANGE_INCLUSIVE)
            range.value |= (1<<is1->vid_range);
    }
    JR_IS1_PUT_BIT(QOS, VLAN, VLAN_TAGGED, regs.vlan, tag->tagged);
    JR_IS1_PUT_BIT(QOS, VLAN, OUTER_DEI, regs.vlan, tag->dei);
    JR_IS1_PUT_FLD(QOS, VLAN, PCP, regs.vlan, tag->pcp);

    JR_VCAP_PUT_MAC(regs.mac, key->mac.smac);

    JR_IS1_PUT_BIT(QOS, FLAGS, L2_MC, regs.flags, key->mac.dmac_mc);
    JR_IS1_PUT_BIT(QOS, FLAGS, L2_BC, regs.flags, key->mac.dmac_bc);

    switch (key->type) {
    case VTSS_IS1_TYPE_ANY:
        /* Only common fields are valid */
        break;
    case VTSS_IS1_TYPE_ETYPE:
        etype_len = 1;
        ip_snap = 0;
        ip4 = 0;
        etype = key->frame.etype.etype;
        sip.value = jr_u8_to_u32(key->frame.etype.data.value);
        sip.mask = jr_u8_to_u32(key->frame.etype.data.mask);
        break;
    case VTSS_IS1_TYPE_LLC:
        etype_len = 0;
        ip_snap = 0;
        ip4 = 0;
        etype.value[0] = key->frame.llc.data.value[0];
        etype.mask[0] = key->frame.llc.data.mask[0];
        etype.value[1] = key->frame.llc.data.value[1];
        etype.mask[1] = key->frame.llc.data.mask[1];
        sip.value = jr_u8_to_u32(&key->frame.llc.data.value[2]);
        sip.mask = jr_u8_to_u32(&key->frame.llc.data.mask[2]);
        break;
    case VTSS_IS1_TYPE_SNAP:
        etype_len = 0;
        ip_snap = 1;
        ip4 = 0;
        etype.value[0] = key->frame.snap.data.value[3];     /* Ethertype MSB */
        etype.mask[0] = key->frame.snap.data.mask[3];
        etype.value[1] = key->frame.snap.data.value[4];     /* Ethertype LSB */
        etype.mask[1] = key->frame.snap.data.mask[4];
        sip.value = (key->frame.snap.data.value[0] << 24) | /* OUI[0] */
                    (key->frame.snap.data.value[1] << 16) | /* OUI[1] */
                    (key->frame.snap.data.value[2] << 8)  | /* OUI[2] */
                     key->frame.snap.data.value[5];         /* Payload[0] */
        sip.mask  = (key->frame.snap.data.mask[0] << 24) |
                    (key->frame.snap.data.mask[1] << 16) |
                    (key->frame.snap.data.mask[2] << 8)  |
                     key->frame.snap.data.mask[5];
        break;
    case VTSS_IS1_TYPE_IPV4:
    case VTSS_IS1_TYPE_IPV6:
        etype_len = 1;
        ip_snap = 1;
        if (key->type == VTSS_IS1_TYPE_IPV4) {
            ip4 = 1;
            proto = &key->frame.ipv4.proto;
            dscp = &key->frame.ipv4.dscp;
            sip = key->frame.ipv4.sip;
            sport = &key->frame.ipv4.sport;
            dport = &key->frame.ipv4.dport;
        } else {
            ip4 = 0;
            proto = &key->frame.ipv6.proto;
            dscp = &key->frame.ipv6.dscp;
            sip.value = jr_u8_to_u32(&key->frame.ipv6.sip.value[12]);
            sip.mask = jr_u8_to_u32(&key->frame.ipv6.sip.mask[12]);
            sport = &key->frame.ipv6.sport;
            dport = &key->frame.ipv6.dport;
        }
        if (jr_vcap_is_udp_tcp(proto)) {
            tcp_udp = 1;
            tcp = VTSS_BOOL(proto->value == 6);
            if (dport->type == VTSS_VCAP_VR_TYPE_VALUE_MASK) {
                etype.value[0] = ((dport->vr.v.value >> 8) & 0xff);
                etype.value[1] = (dport->vr.v.value & 0xff);
                etype.mask[0] = ((dport->vr.v.mask >> 8) & 0xff);
                etype.mask[1] = (dport->vr.v.mask & 0xff);
            }
        } else {
            etype.value[1] = proto->value;
            etype.mask[1] = proto->mask;
        }
        break;
    default:
        VTSS_E("illegal type: %d", key->type);
        return VTSS_RC_ERROR;
    }

    if (key->type != VTSS_IS1_TYPE_ANY) {
        JR_IS1_PUT_BIT(QOS, FLAGS, ETYPE_LEN, regs.flags, JR_VCAP_BIT(etype_len));

        JR_IS1_PUT_U16(QOS, FLAGS, ETYPE, regs.flags, etype);
        JR_IS1_PUT_BIT(QOS, FLAGS, IP, regs.flags, JR_VCAP_BIT(ip_snap));
        if ((key->type == VTSS_IS1_TYPE_IPV4) || (key->type == VTSS_IS1_TYPE_IPV6)) {
            JR_IS1_PUT_BIT(QOS, FLAGS, IP4, regs.flags, JR_VCAP_BIT(ip4));
            if (ip4) {
                JR_IS1_PUT_BIT(QOS, L3_MISC, L3_FRAGMENT, regs.l3_misc, key->frame.ipv4.fragment);
            }
        }
        if (dscp != NULL) {
            if (dscp->type == VTSS_VCAP_VR_TYPE_VALUE_MASK)
                JR_IS1_PUT_FLD(QOS, L3_MISC, L3_DSCP, regs.l3_misc, dscp->vr.v);
            if (is1->dscp_range != VTSS_VCAP_RANGE_CHK_NONE) {
                range.mask |= (1<<is1->dscp_range);
                if (dscp->type == VTSS_VCAP_VR_TYPE_RANGE_INCLUSIVE)
                    range.value |= (1<<is1->dscp_range);
            }
        }
        regs.sip.value = sip.value;
        regs.sip.mask = sip.mask;
        if (tcp_udp) {
            JR_IS1_PUT_BIT(QOS, FLAGS, TCP_UDP, regs.flags, JR_VCAP_BIT(tcp_udp));
            JR_IS1_PUT_BIT(QOS, FLAGS, TCP, regs.flags, JR_VCAP_BIT(tcp));
            if (sport != NULL && sport->type == VTSS_VCAP_VR_TYPE_VALUE_MASK)
                JR_IS1_PUT_FLD(QOS, L4_PORT, L4_SPORT, regs.l4_port, sport->vr.v);
            if (is1->sport_range != VTSS_VCAP_RANGE_CHK_NONE) {
                range.mask |= (1<<is1->sport_range);
                if (sport != NULL && sport->type == VTSS_VCAP_VR_TYPE_RANGE_INCLUSIVE)
                    range.value |= (1<<is1->sport_range);
            }
            if (is1->dport_range != VTSS_VCAP_RANGE_CHK_NONE) {
                range.mask |= (1<<is1->dport_range);
                if (dport != NULL && dport->type == VTSS_VCAP_VR_TYPE_RANGE_INCLUSIVE)
                    range.value |= (1<<is1->dport_range);
            }
        }
    }
    JR_IS1_PUT_FLD(QOS, L4_MISC, L4_RNG, regs.l4_misc, range);

    if (is1->lookup == 0) { /* VLAN_PAG */
        /* Update entry */
        JR_IS1_WR(VLAN_PAG, ENTRY, regs.entry);
        JR_IS1_WR(VLAN_PAG, IF_GRP, regs.if_grp);
        JR_IS1_WR(VLAN_PAG, VLAN, regs.vlan);
        JR_IS1_WR(VLAN_PAG, FLAGS, regs.flags);
        JR_IS1_WR(VLAN_PAG, L2_MAC_ADDR_HIGH, regs.mac.mach);
        JR_IS1_WR(VLAN_PAG, L2_MAC_ADDR_LOW, regs.mac.macl);
        JR_IS1_WR(VLAN_PAG, L3_IP4_SIP, regs.sip);
        JR_IS1_WR(VLAN_PAG, L3_MISC, regs.l3_misc);
        JR_IS1_WR(VLAN_PAG, L4_MISC, regs.l4_misc);

        /* Update action */
        JR_WR(VCAP_IS1, VLAN_PAG_ACTION_PAG,
              JR_PUT_FLD(VCAP_IS1, VLAN_PAG_ACTION_PAG, PAG_OVERRIDE_MASK, action->pag_enable ? 0xff : 0) |
              JR_PUT_FLD(VCAP_IS1, VLAN_PAG_ACTION_PAG, PAG_VAL, action->pag & 0xff));
        JR_WR(VCAP_IS1, VLAN_PAG_ACTION_MISC,
              JR_PUT_BIT(VCAP_IS1, VLAN_PAG_ACTION_MISC, CUSTOM_ACE_TYPE_ENA, 0)              | /* TBD */
              JR_PUT_FLD(VCAP_IS1, VLAN_PAG_ACTION_MISC, CUSTOM_ACE_TYPE_VAL, 0)              | /* TBD */
              JR_PUT_BIT(VCAP_IS1, VLAN_PAG_ACTION_MISC, PCP_DEI_ENA, action->pcp_dei_enable) |
              JR_PUT_FLD(VCAP_IS1, VLAN_PAG_ACTION_MISC, PCP_VAL, action->pcp)                |
              JR_PUT_BIT(VCAP_IS1, VLAN_PAG_ACTION_MISC, DEI_VAL, action->dei)                |
              JR_PUT_BIT(VCAP_IS1, VLAN_PAG_ACTION_MISC, VID_REPLACE_ENA, action->vid)        | /* replace if vid != 0 */
              JR_PUT_FLD(VCAP_IS1, VLAN_PAG_ACTION_MISC, VID_ADD_VAL, action->vid));

        JR_WR(VCAP_IS1, VLAN_PAG_ACTION_CUSTOM_POS, 0);                                         /* TBD */
        JR_WR(VCAP_IS1, VLAN_PAG_ACTION_ISDX, 
              JR_PUT_FLD(VCAP_IS1, VLAN_PAG_ACTION_ISDX, ISDX_ADD_VAL, action->isdx) |
              JR_PUT_BIT(VCAP_IS1, VLAN_PAG_ACTION_ISDX, ISDX_REPLACE_ENA, action->isdx_enable));
        JR_WRB(VCAP_IS1, VLAN_PAG_ACTION_STICKY, HIT_STICKY, 0);
    }
    else { /* QOS */
        /* Update entry */
        JR_IS1_WR(QOS, ENTRY, regs.entry);
        JR_IS1_WR(QOS, IF_GRP, regs.if_grp);
        JR_IS1_WR(QOS, VLAN, regs.vlan);
        JR_IS1_WR(QOS, FLAGS, regs.flags);
        JR_IS1_WR(QOS, L2_MAC_ADDR_HIGH, regs.mac.mach);
        JR_IS1_WR(QOS, L3_IP4_SIP, regs.sip);
        JR_IS1_WR(QOS, L3_MISC, regs.l3_misc);
        JR_IS1_WR(QOS, L4_MISC, regs.l4_misc);
        JR_IS1_WR(QOS, L4_PORT, regs.l4_port);

        /* Update action */
        JR_WR(VCAP_IS1, QOS_ACTION_DSCP,
              JR_PUT_BIT(VCAP_IS1, QOS_ACTION_DSCP, DSCP_ENA, action->dscp_enable) |
              JR_PUT_FLD(VCAP_IS1, QOS_ACTION_DSCP, DSCP_VAL, action->dscp));

        JR_WR(VCAP_IS1, QOS_ACTION_QOS,
              JR_PUT_BIT(VCAP_IS1, QOS_ACTION_QOS, QOS_ENA, action->prio_enable) |
              JR_PUT_FLD(VCAP_IS1, QOS_ACTION_QOS, QOS_VAL, action->prio));

        JR_WR(VCAP_IS1, QOS_ACTION_DP,
              JR_PUT_BIT(VCAP_IS1, QOS_ACTION_DP, DP_ENA, action->dp_enable) |
              JR_PUT_FLD(VCAP_IS1, QOS_ACTION_DP, DP_VAL, action->dp));

        JR_WRB(VCAP_IS1, QOS_ACTION_STICKY, HIT_STICKY, 0);
    }
    return jr_vcap_index_command(vtss_state, VTSS_TCAM_IS1, ix, VTSS_TCAM_CMD_WRITE);
}

static vtss_rc jr_is1_entry_add(vtss_state_t *vtss_state,
                                vtss_vcap_idx_t *idx, vtss_vcap_data_t *data, u32 counter)
{
    VTSS_I("row: %u", idx->row);

    /* Add to all devices */
    return jr_vcap_entry_add_chips(vtss_state, jr_is1_entry_add_chip, idx->row, data, counter);
}

static vtss_rc jr_is1_entry_del_chip(vtss_state_t *vtss_state, u32 ix)
{
    JR_WRB(VCAP_IS1, QOS_ENTRY_ENTRY, VLD, 0);
    JR_WRB(VCAP_IS1, QOS_MASK_ENTRY, VLD, 0);
    VTSS_RC(jr_vcap_index_command(vtss_state, VTSS_TCAM_IS1, ix, VTSS_TCAM_CMD_WRITE));
    return VTSS_RC_OK;
}

static vtss_rc jr_is1_entry_del(vtss_state_t *vtss_state, vtss_vcap_idx_t *idx)
{
    VTSS_I("row: %u", idx->row);

    /* Delete from all devices */
    return jr_vcap_entry_del_chips(vtss_state, jr_is1_entry_del_chip, idx->row);
}

static vtss_rc jr_is1_entry_move_chip(vtss_state_t *vtss_state, u32 ix, u32 count, BOOL up)
{
    JR_WR(VCAP_IS1, IS1_CONTROL_ACE_MV_CFG, 
          JR_PUT_FLD(VCAP_IS1, IS1_CONTROL_ACE_MV_CFG, ACE_MV_NUM_POS, 1) |
          JR_PUT_FLD(VCAP_IS1, IS1_CONTROL_ACE_MV_CFG, ACE_MV_SIZE, count));
    VTSS_RC(jr_vcap_index_command(vtss_state, VTSS_TCAM_IS1, ix + count - 1, 
                                  up ? VTSS_TCAM_CMD_MOVE_UP : VTSS_TCAM_CMD_MOVE_DOWN));
    return VTSS_RC_OK;
}

static vtss_rc jr_is1_entry_move(vtss_state_t *vtss_state,
                                 vtss_vcap_idx_t *idx, u32 count, BOOL up)
{
    VTSS_I("row: %u, count: %u, up: %u", idx->row, count, up);

    /* Move for all devices */
    return jr_vcap_entry_move_chips(vtss_state, jr_is1_entry_move_chip, idx->row, count, up);
}

/* ================================================================= *
 *  IS2 functions
 * ================================================================= */

/* Get IS2 entry for chip */
static vtss_rc jr_is2_entry_get_chip(vtss_state_t *vtss_state, u32 ix, u32 *cnt, BOOL clear) 
{
    VTSS_RC(jr_vcap_index_command(vtss_state, VTSS_TCAM_IS2, ix, VTSS_TCAM_CMD_READ));
    JR_RD(VCAP_IS2, BASETYPE_ACTION_D, cnt);
    
    if (clear) {
        JR_WR(VCAP_IS2, BASETYPE_ACTION_D, 0);
        VTSS_RC(jr_vcap_index_command(vtss_state, VTSS_TCAM_IS2, ix, VTSS_TCAM_CMD_WRITE));
    }
    return VTSS_RC_OK;
}

static vtss_rc jr_is2_entry_get(vtss_state_t *vtss_state,
                                vtss_vcap_idx_t *idx, u32 *counter, BOOL clear) 
{
    VTSS_I("row: %u", idx->row);

    /* Read/clear from all devices */
    return jr_vcap_entry_get_chips(vtss_state, jr_is2_entry_get_chip, idx->row, counter, clear);
}

#define JR_IS2_WR_COMMON(pre, regs)                    \
{                                                      \
    JR_IS2_WR(pre, ACE_VLD, regs.vld);                 \
    JR_IS2_WR(pre, ACE_TYPE, regs.type);               \
    JR_IS2_WR(pre, ACE_IGR_PORT_MASK, regs.port_mask); \
    JR_IS2_WR(pre, ACE_PAG, regs.pag);                 \
}

#define JR_IS2_WR_IP(pre, regs)                \
{                                              \
    JR_IS2_WR(pre, ACE_L3_IP4_SIP, regs.sip);  \
    JR_IS2_WR(pre, ACE_L3_IP4_DIP, regs.dip);  \
    JR_IS2_WR(pre, ACE_L2_MISC, regs.l2_misc); \
    JR_IS2_WR(pre, ACE_L3_MISC, regs.l3_misc); \
}

static vtss_rc jr_is2_entry_add_chip(vtss_state_t *vtss_state,
                                     u32 ix, vtss_vcap_data_t *data, u32 counter)
{
    vtss_is2_data_t    *is2 = &data->u.is2;
    vtss_is2_entry_t   *entry = is2->entry;
    vtss_ace_t         *ace = &entry->ace;
    u32                mask, smask, dmask;
    jr_vcap_is2_regs_t regs;
    jr_vcap_reg_t      misc;
    jr_vcap_mac_t      mac;
    vtss_ace_bit_t     sip_eq_dip, sport_eq_dport, seq_zero;
    vtss_ace_bit_t     ttl, tcp_fin, tcp_syn, tcp_rst, tcp_psh, tcp_ack, tcp_urg;
    vtss_ace_u8_t      proto, ds;
    vtss_ace_u48_t     ip_data;
    vtss_ace_udp_tcp_t *sport, *dport, ipv6_port;
    
    mask = entry->type_mask;
    if (mask != 0) {
        /* Type mask must be updated before writing data mask */
        JR_WR(VCAP_IS2, IS2_CONTROL_ACE_UPDATE_CTRL, 
              JR_PUT_FLD(VCAP_IS2, IS2_CONTROL_ACE_UPDATE_CTRL, ACE_ENTRY_TYPE_MASK, mask));
    }
    
    memset(&regs, 0, sizeof(regs));
    JR_IS2_SET_BIT(MAC_ETYPE, ACE_VLD, VLD, regs.vld);

    /* ACE_TYPE register */
    JR_IS2_PUT_BIT(MAC_ETYPE, ACE_TYPE, FIRST, regs.type, 
                   entry->first ? VTSS_VCAP_BIT_1 : VTSS_VCAP_BIT_0);
    JR_IS2_PUT_BIT(MAC_ETYPE, ACE_TYPE, L2_MC, regs.type, ace->dmac_mc);
    JR_IS2_PUT_BIT(MAC_ETYPE, ACE_TYPE, L2_BC, regs.type, ace->dmac_bc);
    JR_IS2_PUT_BIT(MAC_ETYPE, ACE_TYPE, CFI, regs.type, ace->vlan.cfi);
    JR_IS2_PUT_FLD(MAC_ETYPE, ACE_TYPE, UPRIO, regs.type, ace->vlan.usr_prio);
    JR_IS2_PUT_FLD(MAC_ETYPE, ACE_TYPE, VID, regs.type, ace->vlan.vid);
    if (ace->vlan.vid.mask != 0) {
        /* VID matching used, ISDX must be zero */
        JR_IS2_CLR_BIT(MAC_ETYPE, ACE_TYPE, ISDX_NEQ0, regs.type);
    }
#if defined(VTSS_FEATURE_ACL_V2)
    JR_IS2_PUT_BIT(MAC_ETYPE, ACE_TYPE, VLAN_TAGGED, regs.type, ace->vlan.tagged);
#endif

    /* ACE_IGR_PORT_MASK */
#if defined(VTSS_FEATURE_ACL_V1)
    mask = (entry->chip_port_mask[vtss_state->chip_no] != 0 ? entry->chip_port_mask[vtss_state->chip_no] :
            ace->port_no == VTSS_PORT_NO_ANY ? vtss_jr1_port_mask_from_map(vtss_state, entry->include_int_ports, entry->include_stack_ports) :
            ace->port_no < vtss_state->port_count && VTSS_PORT_CHIP_SELECTED(ace->port_no) ? 
            VTSS_BIT(VTSS_CHIP_PORT(ace->port_no)) : 0);
#endif
#if defined(VTSS_FEATURE_ACL_V2)
    mask = vtss_jr1_port_mask(vtss_state, ace->port_list);
#endif
    /* Enabled ports are don't cared, disabled ports must be 0 */
    regs.port_mask.mask = ~mask;

    /* ACE_PAG */
    JR_IS2_PUT_FLD_VM(MAC_ETYPE, ACE_PAG, PAG, regs.pag, ace->policy.value, ace->policy.mask);
    
    misc.value = misc.mask = 0;
    
    switch (entry->type) {
    case IS2_ENTRY_TYPE_ETYPE:
        JR_IS2_WR_COMMON(MAC_ETYPE, regs);
        JR_IS2_WR_MAC(MAC_ETYPE, ACE_L2_SMAC, ace->frame.etype.smac);
        JR_IS2_WR_MAC(MAC_ETYPE, ACE_L2_DMAC, ace->frame.etype.dmac);
        JR_IS2_PUT_U16(MAC_ETYPE, ACE_L2_ETYPE, L2_PAYLOAD, misc, ace->frame.etype.data);
        JR_IS2_PUT_U16(MAC_ETYPE, ACE_L2_ETYPE, ETYPE, misc, ace->frame.etype.etype);
        JR_IS2_WR(MAC_ETYPE, ACE_L2_ETYPE, misc);
        break;
    case IS2_ENTRY_TYPE_LLC:
        JR_IS2_WR_COMMON(MAC_LLC, regs);
        JR_IS2_WR_MAC(MAC_LLC, ACE_L2_SMAC, ace->frame.llc.smac);
        JR_IS2_WR_MAC(MAC_LLC, ACE_L2_DMAC, ace->frame.llc.dmac);
        JR_VCAP_PUT_U32(misc, ace->frame.llc.llc);
        JR_IS2_WR(MAC_LLC, ACE_L2_LLC, misc);
        break;
    case IS2_ENTRY_TYPE_SNAP:
        JR_IS2_WR_COMMON(MAC_SNAP, regs);
        JR_VCAP_PUT_U40(mac, ace->frame.snap.snap);
        JR_IS2_WR(MAC_SNAP, ACE_L2_SNAP_HIGH, mac.mach);
        JR_IS2_WR(MAC_SNAP, ACE_L2_SNAP_LOW, mac.macl);
        break;
    case IS2_ENTRY_TYPE_ARP:
        JR_IS2_WR_COMMON(ARP, regs);
        JR_IS2_WR_MAC(ARP, ACE_L2_SMAC, ace->frame.arp.smac);
        JR_IS2_WR(ARP, ACE_L3_IP4_SIP, ace->frame.arp.sip);
        JR_IS2_WR(ARP, ACE_L3_IP4_DIP, ace->frame.arp.dip);
        JR_IS2_WR(ARP, ACE_L3_MISC, misc);
        JR_IS2_PUT_FLD_VM(ARP, ACE_L3_ARP, ARP_OPCODE, misc, 
                          ((ace->frame.arp.arp == VTSS_ACE_BIT_1 ? 0 : 1) << 1) |
                          ((ace->frame.arp.req == VTSS_ACE_BIT_1 ? 0 : 1) << 0),
                          ((ace->frame.arp.arp == VTSS_ACE_BIT_ANY ? 0 : 1) << 1) |
                          ((ace->frame.arp.req == VTSS_ACE_BIT_ANY ? 0 : 1) << 0));
        JR_IS2_PUT_BIT(ARP, ACE_L3_ARP, ARP_OPCODE_UNKNOWN, misc, ace->frame.arp.unknown);
        JR_IS2_PUT_BIT(ARP, ACE_L3_ARP, ARP_SENDER_MATCH, misc, ace->frame.arp.smac_match);
        JR_IS2_PUT_BIT(ARP, ACE_L3_ARP, ARP_TARGET_MATCH, misc, ace->frame.arp.dmac_match);
        JR_IS2_PUT_BIT(ARP, ACE_L3_ARP, ARP_LEN_OK, misc, ace->frame.arp.length);
        JR_IS2_PUT_BIT(ARP, ACE_L3_ARP, ARP_PROTO_SPACE_OK, misc, ace->frame.arp.ip);
        JR_IS2_PUT_BIT(ARP, ACE_L3_ARP, ARP_ADDR_SPACE_OK, misc, ace->frame.arp.ethernet);
        JR_IS2_WR(ARP, ACE_L3_ARP, misc);
        break;
    case IS2_ENTRY_TYPE_IP_OTHER:
        if (ace->type == VTSS_ACE_TYPE_IPV4) {
            /* IPv4 */
            regs.sip.value = ace->frame.ipv4.sip.value;
            regs.sip.mask = ace->frame.ipv4.sip.mask;
            regs.dip.value = ace->frame.ipv4.dip.value;
            regs.dip.mask = ace->frame.ipv4.dip.mask;
            JR_IS2_SET_BIT(IP_OTHER, ACE_L2_MISC, IP4, regs.l2_misc);
            proto = ace->frame.ipv4.proto;
            ds = ace->frame.ipv4.ds;
            ttl = ace->frame.ipv4.ttl;
            JR_IS2_PUT_BIT(IP_OTHER, ACE_L3_MISC, L3_FRAGMENT, regs.l3_misc, 
                           ace->frame.ipv4.fragment);
            JR_IS2_PUT_BIT(IP_OTHER, ACE_L3_MISC, L3_OPTIONS, regs.l3_misc, 
                           ace->frame.ipv4.options);
            ip_data = ace->frame.ipv4.data;
            sport = &ace->frame.ipv4.sport;
            dport = &ace->frame.ipv4.dport;
            tcp_urg = ace->frame.ipv4.tcp_urg;
            tcp_ack = ace->frame.ipv4.tcp_ack;
            tcp_psh = ace->frame.ipv4.tcp_psh;
            tcp_rst = ace->frame.ipv4.tcp_rst;
            tcp_syn = ace->frame.ipv4.tcp_syn;
            tcp_fin = ace->frame.ipv4.tcp_fin;
            sip_eq_dip = ace->frame.ipv4.sip_eq_dip;
            sport_eq_dport = ace->frame.ipv4.sport_eq_dport;
            seq_zero = ace->frame.ipv4.seq_zero;
        } else if (ace->type == VTSS_ACE_TYPE_IPV6) {
            /* IPv6 */
            JR_VCAP_PUT_U32_NDX(regs.sip, ace->frame.ipv6.sip, 12);        
            JR_VCAP_PUT_U32_NDX(regs.dip, ace->frame.ipv6.sip, 8);        
            JR_IS2_CLR_BIT(IP_OTHER, ACE_L2_MISC, IP4, regs.l2_misc);
            proto = ace->frame.ipv6.proto;
            ds = ace->frame.ipv6.ds;
            ttl = ace->frame.ipv6.ttl;
            ip_data = ace->frame.ipv6.data;
            sport = &ace->frame.ipv6.sport;
            dport = &ace->frame.ipv6.dport;
            tcp_urg = ace->frame.ipv6.tcp_urg;
            tcp_ack = ace->frame.ipv6.tcp_ack;
            tcp_psh = ace->frame.ipv6.tcp_psh;
            tcp_rst = ace->frame.ipv6.tcp_rst;
            tcp_syn = ace->frame.ipv6.tcp_syn;
            tcp_fin = ace->frame.ipv6.tcp_fin;
            sip_eq_dip = ace->frame.ipv6.sip_eq_dip;
            sport_eq_dport = ace->frame.ipv6.sport_eq_dport;
            seq_zero = ace->frame.ipv6.seq_zero;
        } else {
            /* ANY_IP */
            proto.value = proto.mask = 0;
            ds.value = ds.mask = 0;
            ttl = VTSS_ACE_BIT_ANY;
            memset(&ip_data, 0, sizeof(ip_data));
            ipv6_port.in_range = 1;
            ipv6_port.high = 0xffff;
            ipv6_port.low = 0;
            sport = dport = &ipv6_port;
            tcp_urg = VTSS_ACE_BIT_ANY;
            tcp_ack = VTSS_ACE_BIT_ANY;
            tcp_psh = VTSS_ACE_BIT_ANY;
            tcp_rst = VTSS_ACE_BIT_ANY;
            tcp_syn = VTSS_ACE_BIT_ANY;
            tcp_fin = VTSS_ACE_BIT_ANY;
            sip_eq_dip = VTSS_ACE_BIT_ANY;
            sport_eq_dport = VTSS_ACE_BIT_ANY;
            seq_zero = VTSS_ACE_BIT_ANY;
        }
        JR_IS2_PUT_BIT(IP_OTHER, ACE_L3_MISC, DIP_EQ_SIP, regs.l3_misc, sip_eq_dip);
        JR_IS2_PUT_FLD(IP_OTHER, ACE_L3_MISC, L3_IP_PROTO, regs.l3_misc, proto);
        JR_IS2_PUT_FLD(IP_OTHER, ACE_L3_MISC, L3_TOS, regs.l3_misc, ds);
        JR_IS2_PUT_BIT(IP_OTHER, ACE_L3_MISC, L3_TTL_GT0, regs.l3_misc, ttl);

        if (vtss_vcap_udp_tcp_rule(&proto)) {
            /* UDP/TCP */
            JR_IS2_WR_COMMON(IP_TCP_UDP, regs);
            JR_IS2_WR_IP(IP_TCP_UDP, regs);

            /* Port numbers */
            JR_IS2_PUT_FLD_VM(IP_TCP_UDP, ACE_L4_PORT, L4_SPORT, misc, 
                              sport->low, sport->low == sport->high ? 0xffff : 0);
            JR_IS2_PUT_FLD_VM(IP_TCP_UDP, ACE_L4_PORT, L4_DPORT, misc, 
                              dport->low, dport->low == dport->high ? 0xffff : 0);
            JR_IS2_WR(IP_TCP_UDP, ACE_L4_PORT, misc);

            /* Port ranges and TCP flags */
            misc.value = misc.mask = 0;
            smask = (is2->srange == VTSS_VCAP_RANGE_CHK_NONE ? 0 : (1<<is2->srange));
            dmask = (is2->drange == VTSS_VCAP_RANGE_CHK_NONE ? 0 : (1<<is2->drange));
            JR_IS2_PUT_FLD_VM(IP_TCP_UDP, ACE_L4_MISC, L4_RNG, misc, 
                              (sport->in_range ? smask : 0) | (dport->in_range ? dmask : 0), 
                              smask | dmask);
            JR_IS2_PUT_BIT(IP_TCP_UDP, ACE_L4_MISC, SPORT_EQ_DPORT, misc, sport_eq_dport);
            JR_IS2_PUT_BIT(IP_TCP_UDP, ACE_L4_MISC, SEQUENCE_EQ0, misc, seq_zero);
            if (proto.value == 6) {
                JR_IS2_PUT_BIT(IP_TCP_UDP, ACE_L4_MISC, L4_URG, misc, tcp_urg);
                JR_IS2_PUT_BIT(IP_TCP_UDP, ACE_L4_MISC, L4_ACK, misc, tcp_ack);
                JR_IS2_PUT_BIT(IP_TCP_UDP, ACE_L4_MISC, L4_PSH, misc, tcp_psh);
                JR_IS2_PUT_BIT(IP_TCP_UDP, ACE_L4_MISC, L4_RST, misc, tcp_rst);
                JR_IS2_PUT_BIT(IP_TCP_UDP, ACE_L4_MISC, L4_SYN, misc, tcp_syn);
                JR_IS2_PUT_BIT(IP_TCP_UDP, ACE_L4_MISC, L4_FIN, misc, tcp_fin);
            }
            JR_IS2_WR(IP_TCP_UDP, ACE_L4_MISC, misc);
        } else {
            /* Not UDP/TCP */
            JR_IS2_WR_COMMON(IP_OTHER, regs);
            JR_IS2_WR_IP(IP_OTHER, regs);
            JR_VCAP_PUT_U48(mac, ip_data);
            JR_IS2_WR(IP_OTHER, ACE_IP4_OTHER_0, mac.macl);
            JR_IS2_WR(IP_OTHER, ACE_IP4_OTHER_1, mac.mach);
        }
        break;
    case IS2_ENTRY_TYPE_OAM:
        JR_IS2_WR_COMMON(OAM, regs);
        JR_IS2_WR(OAM, ACE_OAM_0, misc);
        JR_IS2_WR(OAM, ACE_OAM_1, misc);
        break;
    default:
        VTSS_E("illegal type: %d", ace->type);
        return VTSS_RC_ERROR;
    }

    VTSS_RC(jr_is2_prepare_action(vtss_state, &ace->action, entry->action_ext, counter));
    return jr_vcap_index_command(vtss_state, VTSS_TCAM_IS2, ix, VTSS_TCAM_CMD_WRITE);
}

static vtss_rc jr_is2_entry_add(vtss_state_t *vtss_state,
                                vtss_vcap_idx_t *idx, vtss_vcap_data_t *data, u32 counter)
{
    VTSS_I("row: %u", idx->row);

    /* Add to all devices */
    return jr_vcap_entry_add_chips(vtss_state, jr_is2_entry_add_chip, idx->row, data, counter);
}

static vtss_rc jr_is2_entry_del_chip(vtss_state_t *vtss_state, u32 ix)
{
    JR_WR(VCAP_IS2, MAC_ETYPE_ENTRY_ACE_VLD, 0);
    JR_WR(VCAP_IS2, MAC_ETYPE_MASK_ACE_VLD, 0);
    VTSS_RC(jr_vcap_index_command(vtss_state, VTSS_TCAM_IS2, ix, VTSS_TCAM_CMD_WRITE));
    return VTSS_RC_OK;
}

static vtss_rc jr_is2_entry_del(vtss_state_t *vtss_state, vtss_vcap_idx_t *idx)
{
    VTSS_I("row: %u", idx->row);

    /* Delete from all devices */
    return jr_vcap_entry_del_chips(vtss_state, jr_is2_entry_del_chip, idx->row);
}

static vtss_rc jr_is2_entry_move_chip(vtss_state_t *vtss_state, u32 ix, u32 count, BOOL up)
{
    JR_WR(VCAP_IS2, IS2_CONTROL_ACE_MV_CFG, 
          JR_PUT_FLD(VCAP_IS2, IS2_CONTROL_ACE_MV_CFG, ACE_MV_NUM_POS, 1) |
          JR_PUT_FLD(VCAP_IS2, IS2_CONTROL_ACE_MV_CFG, ACE_MV_SIZE, count));
    VTSS_RC(jr_vcap_index_command(vtss_state, VTSS_TCAM_IS2, ix + count - 1, 
                                  up ? VTSS_TCAM_CMD_MOVE_UP : VTSS_TCAM_CMD_MOVE_DOWN));
    return VTSS_RC_OK;
}

static vtss_rc jr_is2_entry_move(vtss_state_t *vtss_state,
                                 vtss_vcap_idx_t *idx, u32 count, BOOL up)
{
    VTSS_I("row: %u, count: %u, up: %u", idx->row, count, up);

    /* Move for all devices */
    return jr_vcap_entry_move_chips(vtss_state, jr_is2_entry_move_chip, idx->row, count, up);
}

/* ================================================================= *
 *  ES0 functions
 * ================================================================= */

/* Get ES0 entry for chip */
static vtss_rc jr_es0_entry_get_chip(vtss_state_t *vtss_state, u32 ix, u32 *cnt, BOOL clear) 
{
    u32 type;
    
    VTSS_RC(jr_vcap_index_command(vtss_state, VTSS_TCAM_ES0, ix, VTSS_TCAM_CMD_READ));
    JR_RDB(VCAP_ES0, ES0_CONTROL_ACE_STATUS, ACE_ACTION_TYPE, &type);
    if (type == JR_ES0_ACTION_MACINMAC) {
        JR_RD(VCAP_ES0, MACINMAC_ACTION_C, cnt);
    } else {
        JR_RD(VCAP_ES0, TAG_ACTION_C, cnt);
    }
    
    if (clear) {
        if (type == JR_ES0_ACTION_MACINMAC) {
            JR_WR(VCAP_ES0, MACINMAC_ACTION_C, 0);
        } else {
            JR_WR(VCAP_ES0, TAG_ACTION_C, 0);
        }
        VTSS_RC(jr_vcap_index_command(vtss_state, VTSS_TCAM_ES0, ix, VTSS_TCAM_CMD_WRITE));
    }
    return VTSS_RC_OK;
}

static vtss_rc jr_es0_entry_get(vtss_state_t *vtss_state,
                                vtss_vcap_idx_t *idx, u32 *counter, BOOL clear) 
{
    VTSS_I("row: %u", idx->row);

    /* Read/clear from all devices */
    return jr_vcap_entry_get_chips(vtss_state, jr_es0_entry_get_chip, idx->row, counter, clear);
}

static vtss_rc jr_es0_entry_add_chip(vtss_state_t *vtss_state,
                                     u32 ix, vtss_vcap_data_t *data, u32 counter)
{
    vtss_es0_data_t   *es0 = &data->u.es0;
    vtss_es0_key_t    *key = &es0->entry->key;
    vtss_es0_action_t *action = &es0->entry->action;
    jr_vcap_reg_t     reg;
    u32               port;

    memset(&reg, 0, sizeof(reg));
    JR_ES0_SET_BIT(ISDX, A, VLD, reg);
    JR_ES0_PUT_BIT(ISDX, A, ISDX_NEQ0, reg, key->isdx_neq0);
    if (key->port_no != VTSS_PORT_NO_NONE) {
        port = (VTSS_PORT_CHIP_SELECTED(key->port_no) ? VTSS_CHIP_PORT(key->port_no) : 0xff);
        JR_ES0_PUT_FLD_VM(ISDX, A, VEGR_PORT, reg, port, 0xff);
    }
    
    switch (key->type) {
    case VTSS_ES0_TYPE_ISDX:
        JR_ES0_WR(ISDX, A, reg);
        memset(&reg, 0, sizeof(reg));
        JR_ES0_PUT_FLD_VM(ISDX, ISDX1, ISDX, reg, key->data.isdx.isdx, 0xfff);
        JR_ES0_WR(ISDX, ISDX1, reg);
        break;
    case VTSS_ES0_TYPE_VID:
        JR_ES0_WR(VID, A, reg);
        memset(&reg, 0, sizeof(reg));
        JR_ES0_PUT_FLD_VM(VID, VID1, VID, reg, key->data.vid.vid, 0xfff);
        JR_ES0_WR(VID, VID1, reg);
        break;
    default:
        VTSS_E("illegal type: %d", key->type);
        return VTSS_RC_ERROR;
    }
    
    /* Action */
    vtss_cmn_es0_action_get(vtss_state, es0);
    JR_WR(VCAP_ES0, TAG_ACTION_A, JR_PUT_BIT(VCAP_ES0, TAG_ACTION_A, VLD, 1));
    JR_WR(VCAP_ES0, TAG_ACTION_TAG1,
          JR_PUT_FLD(VCAP_ES0, TAG_ACTION_TAG1, VID_B_VAL, action->vid_b) |
          JR_PUT_FLD(VCAP_ES0, TAG_ACTION_TAG1, TAG_VID_SEL, 2) |
          JR_PUT_FLD(VCAP_ES0, TAG_ACTION_TAG1, TAG_TPI_SEL, action->tpid) |
          JR_PUT_FLD(VCAP_ES0, TAG_ACTION_TAG1, TAG_CTRL, action->tag));
    JR_WR(VCAP_ES0, TAG_ACTION_B,
          JR_PUT_FLD(VCAP_ES0, TAG_ACTION_B, VID_A_VAL, action->vid_a) |
          JR_PUT_FLD(VCAP_ES0, TAG_ACTION_B, ESDX_VAL, action->esdx) |
          JR_PUT_BIT(VCAP_ES0, TAG_ACTION_B, DEI_VAL, action->dei) |
          JR_PUT_FLD(VCAP_ES0, TAG_ACTION_B, PCP_VAL, action->pcp) |
          JR_PUT_FLD(VCAP_ES0, TAG_ACTION_B, QOS_SRC_SEL, action->qos));
    JR_WR(VCAP_ES0, TAG_ACTION_C, counter);

    return jr_vcap_index_command(vtss_state, VTSS_TCAM_ES0, ix, VTSS_TCAM_CMD_WRITE);
}

static vtss_rc jr_es0_entry_add(vtss_state_t *vtss_state,
                                vtss_vcap_idx_t *idx, vtss_vcap_data_t *data, u32 counter)
{
    VTSS_I("row: %u", idx->row);

    /* Add to all devices */
    return jr_vcap_entry_add_chips(vtss_state, jr_es0_entry_add_chip, idx->row, data, counter);
}

static vtss_rc jr_es0_entry_del_chip(vtss_state_t *vtss_state, u32 ix)
{
    JR_WRB(VCAP_ES0, ISDX_ENTRY_A, VLD, 0);
    JR_WRB(VCAP_ES0, ISDX_MASK_A, VLD, 0);
    VTSS_RC(jr_vcap_index_command(vtss_state, VTSS_TCAM_ES0, ix, VTSS_TCAM_CMD_WRITE));
    return VTSS_RC_OK;
}

static vtss_rc jr_es0_entry_del(vtss_state_t *vtss_state, vtss_vcap_idx_t *idx)
{
    VTSS_I("row: %u", idx->row);

    /* Delete from all devices */
    return jr_vcap_entry_del_chips(vtss_state, jr_es0_entry_del_chip, idx->row);
}

static vtss_rc jr_es0_entry_move_chip(vtss_state_t *vtss_state, u32 ix, u32 count, BOOL up)
{
    JR_WR(VCAP_ES0, ES0_CONTROL_ACE_MV_CFG, 
          JR_PUT_FLD(VCAP_ES0, ES0_CONTROL_ACE_MV_CFG, ACE_MV_NUM_POS, 1) |
          JR_PUT_FLD(VCAP_ES0, ES0_CONTROL_ACE_MV_CFG, ACE_MV_SIZE, count));
    VTSS_RC(jr_vcap_index_command(vtss_state, VTSS_TCAM_ES0, ix + count - 1, 
                                  up ? VTSS_TCAM_CMD_MOVE_UP : VTSS_TCAM_CMD_MOVE_DOWN));
    return VTSS_RC_OK;
}

static vtss_rc jr_es0_entry_move(vtss_state_t *vtss_state,
                                 vtss_vcap_idx_t *idx, u32 count, BOOL up)
{
    VTSS_I("row: %u, count: %u, up: %u", idx->row, count, up);

    /* Move for all devices */
    return jr_vcap_entry_move_chips(vtss_state, jr_es0_entry_move_chip, idx->row, count, up);
}

/* Update outer tag TPID for ES0 entry if VLAN port type has changed */
static vtss_rc jr_es0_entry_update(vtss_state_t *vtss_state,
                                   vtss_vcap_idx_t *idx, vtss_es0_data_t *es0) 
{
    vtss_chip_no_t    chip_no;
    vtss_es0_action_t *action = &es0->entry->action;

    for (chip_no = 0; chip_no < vtss_state->chip_count; chip_no++) {
        VTSS_SELECT_CHIP(chip_no);
        VTSS_RC(jr_vcap_index_command(vtss_state, VTSS_TCAM_ES0, idx->row, VTSS_TCAM_CMD_READ));
        if (es0->flags & VTSS_ES0_FLAG_TPID) {
            JR_WRF(VCAP_ES0, TAG_ACTION_TAG1, TAG_TPI_SEL, action->tpid);
        }
        if (es0->flags & VTSS_ES0_FLAG_QOS) {
            JR_WRF(VCAP_ES0, TAG_ACTION_B, QOS_SRC_SEL, action->qos);
            JR_WRF(VCAP_ES0, TAG_ACTION_B, PCP_VAL, action->pcp);
            JR_WRB(VCAP_ES0, TAG_ACTION_B, DEI_VAL, action->dei);
        }
        VTSS_RC(jr_vcap_index_command(vtss_state, VTSS_TCAM_ES0, idx->row, VTSS_TCAM_CMD_WRITE));
    }
    return VTSS_RC_OK;
}

/* ================================================================= *
 *  Utility functions
 * ================================================================= */

/* Advanced REDIR_PGID fields */
#define JR_IS2_REDIR_PGID_ASM ((0<<10) | (1<<3)) /* Force ASM in MAC table */

/* Enable first IS2 lookup and setup default actions for CPU ports */
vtss_rc vtss_jr1_is2_cpu_port_setup(vtss_state_t *vtss_state)
{
    u32               port, cnt;
    vtss_acl_action_t action;

    /* Initialize IS2 action */
    memset(&action, 0, sizeof(action));
    action.learn = 1;
    action.forward = 1;

    for (port = VTSS_CHIP_PORT_CPU_0; port <= VTSS_CHIP_PORT_CPU_1; port++) {
        VTSS_RC(vtss_jr1_vcap_is1_is2_set(vtss_state, port, 0, IS2_LOOKUP_FIRST));
        VTSS_RC(vtss_jr1_vcap_port_command(vtss_state, VTSS_TCAM_IS2, port, VTSS_TCAM_CMD_READ));
        JR_RD(VCAP_IS2, BASETYPE_ACTION_D, &cnt);
        VTSS_RC(jr_is2_prepare_action(vtss_state, &action, JR_IS2_REDIR_PGID_ASM, cnt));
        VTSS_RC(vtss_jr1_vcap_port_command(vtss_state, VTSS_TCAM_IS2, port, VTSS_TCAM_CMD_WRITE));
    }
    return VTSS_RC_OK;
}

/* Offsets to extra entries for any frame type rules */
#define JR_IS2_ID_ANY_LLC   0x100000000LL /* 0001 -> MAC_LLC */
#define JR_IS2_ID_ANY_SNAP  0x200000000LL /* 001X -> MAC_SNAP and ARP */
#define JR_IS2_ID_ANY_OAM   0x300000000LL /* 0111 -> MAC_OAM */
#define JR_IS2_ID_ANY_IP    0x400000000LL /* 010X -> IP_TCP_UDP and IP_OTHER */
                                          /* X000 -> MAC_ETYPE */

#define JR_IS2_ANY_ETYPE_MASK 0x8 /* X000 -> MAC_ETYPE */
#define JR_IS2_ANY_IP_MASK    0x1 /* 010X -> IP_TCP_UDP and IP_OTHER */
#define JR_IS2_ANY_SNAP_MASK  0x1 /* 001X -> MAC_SNAP and ARP */

#define JR_IS2_ID_ANY_CNT   4 /* Number of extra entries */


vtss_rc vtss_jr1_is2_del_any(vtss_state_t *vtss_state,
                             vtss_vcap_user_t user, vtss_vcap_id_t id, BOOL etype)
{
    vtss_vcap_obj_t *obj = &vtss_state->vcap.is2.obj;
    
    if (etype) {
        VTSS_RC(vtss_vcap_del(vtss_state, obj, user, id));
    }
    VTSS_RC(vtss_vcap_del(vtss_state, obj, user, id + JR_IS2_ID_ANY_LLC));
    VTSS_RC(vtss_vcap_del(vtss_state, obj, user, id + JR_IS2_ID_ANY_SNAP));
    VTSS_RC(vtss_vcap_del(vtss_state, obj, user, id + JR_IS2_ID_ANY_OAM));
    VTSS_RC(vtss_vcap_del(vtss_state, obj, user, id + JR_IS2_ID_ANY_IP));
    
    return VTSS_RC_OK;
}

vtss_rc vtss_jr1_is2_add_any(vtss_state_t     *vtss_state,
                             vtss_vcap_user_t user, 
                             vtss_vcap_id_t   id_base,
                             vtss_vcap_id_t   id_next,
                             vtss_vcap_data_t *data,
                             BOOL             etype)
{
    vtss_is2_entry_t *entry = data->u.is2.entry;
    vtss_ace_t       *ace = &entry->ace;
    vtss_vcap_obj_t  *obj = &vtss_state->vcap.is2.obj;
    vtss_vcap_id_t   id = id_base;
    
    /* Add extra any entries */
    memset(&ace->frame, 0, sizeof(ace->frame));

    /* ANY_ETYPE */
    if (etype) {
        entry->type = IS2_ENTRY_TYPE_ETYPE;
        entry->type_mask = JR_IS2_ANY_ETYPE_MASK;
        VTSS_RC(vtss_vcap_add(vtss_state, obj, user, id, id_next, data, 0));
    }
    
    /* ANY_IP */
    entry->type = IS2_ENTRY_TYPE_IP_OTHER;
    entry->type_mask = JR_IS2_ANY_IP_MASK;
    id_next = id;
    id = (id_base + JR_IS2_ID_ANY_IP);
    VTSS_RC(vtss_vcap_add(vtss_state, obj, user, id, id_next, data, 0));

    /* ANY_OAM */
    entry->type = IS2_ENTRY_TYPE_OAM;
    entry->type_mask = 0;
    id_next = id;
    id = (id_base + JR_IS2_ID_ANY_OAM);
    VTSS_RC(vtss_vcap_add(vtss_state, obj, user, id, id_next, data, 0));
    
    /* ANY_SNAP */
    entry->type = IS2_ENTRY_TYPE_SNAP;
    entry->type_mask = JR_IS2_ANY_SNAP_MASK;
    id_next = id;
    id = (id_base + JR_IS2_ID_ANY_SNAP);
    VTSS_RC(vtss_vcap_add(vtss_state, obj, user, id, id_next, data, 0));
    
    /* ANY_LLC */
    entry->type = IS2_ENTRY_TYPE_LLC;
    entry->type_mask = 0;
    id_next = id;
    id = (id_base + JR_IS2_ID_ANY_LLC);
    VTSS_RC(vtss_vcap_add(vtss_state, obj, user, id, id_next, data, 0));

    return VTSS_RC_OK;
}

/* Determine if first IS2 lookup must be enabled */
static BOOL jr_is2_first_enabled(vtss_state_t *vtss_state)
{
    return vtss_state->packet.rx_conf.reg.igmp_cpu_only;
}

/* Enable/disable IS1 and IS2 lookup for port */
vtss_rc vtss_jr1_vcap_is1_is2_set(vtss_state_t *vtss_state, u32 port, BOOL is1, u32 is2)
{
    JR_WRX(ANA_CL_2, PORT_ADV_CL_CFG, port, 
           JR_PUT_BIT(ANA_CL_2, PORT_ADV_CL_CFG, ADV_USER_CL_ENA, is1) |
           JR_PUT_BIT(ANA_CL_2, PORT_ADV_CL_CFG, ADV_QOS_CL_ENA, is1) |
           JR_PUT_FLD(ANA_CL_2, PORT_ADV_CL_CFG, SEC_TYPE_MAC_SNAP_ENA, IS2_LOOKUP_BOTH) |
           JR_PUT_FLD(ANA_CL_2, PORT_ADV_CL_CFG, SEC_TYPE_IP_TCPUDP_ENA, IS2_LOOKUP_BOTH) |
           JR_PUT_FLD(ANA_CL_2, PORT_ADV_CL_CFG, SEC_TYPE_IP6_TCPUDP_OTHER_ENA, IS2_LOOKUP_BOTH) |
           JR_PUT_FLD(ANA_CL_2, PORT_ADV_CL_CFG, SEC_TYPE_MAC_LLC_ENA, IS2_LOOKUP_BOTH) |
           JR_PUT_FLD(ANA_CL_2, PORT_ADV_CL_CFG, SEC_TYPE_ARP_ENA, IS2_LOOKUP_BOTH) |
           JR_PUT_FLD(ANA_CL_2, PORT_ADV_CL_CFG, SEC_TYPE_IP_OTHER_ENA, IS2_LOOKUP_BOTH) |
           JR_PUT_FLD(ANA_CL_2, PORT_ADV_CL_CFG, SEC_ENA, is2));    
    
    return VTSS_RC_OK;
}

/* Enable/disable VCAP lookups */
vtss_rc vtss_jr1_vcap_lookup_set(vtss_state_t *vtss_state, const vtss_port_no_t port_no)
{
    u32  port = VTSS_CHIP_PORT(port_no);
    u32  is2 = IS2_LOOKUP_NONE;
    BOOL is0 = 1, is1 = 1;

    /* Determine if first IS2 lookup is enabled */
    if (jr_is2_first_enabled(vtss_state)) {
        is2 |= IS2_LOOKUP_FIRST;
    }
    
    /* Second IS2 lookup is enabled if ACLs are enabled on port */
    if (vtss_state->vcap.acl_port_conf[port_no].policy_no != VTSS_ACL_POLICY_NO_NONE) {
        is2 |= IS2_LOOKUP_SECOND;
    }
    
#if defined(VTSS_FEATURE_VSTAX_V2)
    if (!vtss_jr1_is_frontport(vtss_state, port_no)) {
        /* Disable VCAP lookups on stack ports */
        is0 = 0;
        is1 = 0;
        is2 &= ~IS2_LOOKUP_SECOND; /* First lookup can be enabled for IGMP/SSM */
    }
#endif /* VTSS_FEATURE_VSTAX_V2 */

    VTSS_RC(vtss_jr1_vcap_is1_is2_set(vtss_state, port, is1, is2));

    /* DBL_VID_ENA */
    JR_WRX(ANA_CL_2, PORT_IS0_CFG_1, port, 
           VTSS_BIT(1) | JR_PUT_BIT(ANA_CL_2, PORT_IS0_CFG_1, IS0_ENA, is0));

    return VTSS_RC_OK;
}

/* BZ 2902: Handle IGMP CPU redirect using IS2.
   If IGMP is enabled, both IS2 lookups are used and an IGMP IS2 rule is added 
   for the first lookup. After this rule, a block of IS2 rules are also added to 
   allow any frame. This is done to ensure that the default action is never hit for the
   first lookup (this would cause problems with counters and default actions).
   If IGMP is disabled, only the second lookup is used and the IS2 rules are deleted. */
vtss_rc vtss_jr1_igmp_cpu_copy_set(vtss_state_t *vtss_state)
{
    vtss_packet_rx_conf_t *conf = &vtss_state->packet.rx_conf;
    vtss_vcap_obj_t       *obj = &vtss_state->vcap.is2.obj;
    vtss_vcap_data_t      data;
    vtss_vcap_id_t        id = 1;
    vtss_is2_entry_t      entry;
    vtss_ace_t            *ace = &entry.ace;
    vtss_port_no_t        port_no;
    
    /* Initialize IS2 data */
    memset(&data, 0, sizeof(data));
    memset(&entry, 0, sizeof(entry));
    data.u.is2.entry = &entry;
    entry.first = 1;
    entry.action_ext = JR_IS2_REDIR_PGID_ASM;
    entry.include_int_ports = 1;
    entry.include_stack_ports = 1;
    ace->port_no = VTSS_PORT_NO_NONE;
    ace->action.learn = 1;
    ace->action.forward = 1;

    if (jr_is2_first_enabled(vtss_state)) {
        /* Add block of default entries */
        VTSS_RC(vtss_jr1_is2_add_any(vtss_state, VTSS_IS2_USER_IGMP_ANY, id, VTSS_VCAP_ID_LAST, &data, 1));
    } else {
        /* Delete block of default entries */
        VTSS_RC(vtss_jr1_is2_del_any(vtss_state, VTSS_IS2_USER_IGMP_ANY, id, 1));
    }

    if (conf->reg.igmp_cpu_only) {
        /* Add IGMP entry */
        entry.include_int_ports = 0;
        entry.include_stack_ports = 0;
        entry.type = IS2_ENTRY_TYPE_IP_OTHER;
        entry.type_mask = 0x1;
        ace->type = VTSS_ACE_TYPE_IPV4;
        ace->frame.ipv4.proto.value = 2;
        ace->frame.ipv4.proto.mask = 0xff;
        ace->action.forward = 0;
        ace->action.cpu = 1;
        ace->action.cpu_queue = conf->map.igmp_queue;
        VTSS_RC(vtss_vcap_add(vtss_state, obj, VTSS_IS2_USER_IGMP, id, VTSS_VCAP_ID_LAST, &data, 0));
    } else {
        /* Delete IGMP entry */
        VTSS_RC(vtss_vcap_del(vtss_state, obj, VTSS_IS2_USER_IGMP, id));
    }

    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
        VTSS_SELECT_CHIP_PORT_NO(port_no);
        VTSS_RC(vtss_jr1_vcap_lookup_set(vtss_state, port_no));
    }

    return VTSS_RC_OK;
}

/* ================================================================= *
 *  ACL functions
 * ================================================================= */

static vtss_rc jr_acl_policer_set_chip(vtss_state_t *vtss_state,
                                       const vtss_acl_policer_no_t policer_no)
{
    u32                     rate, i = (policer_no - VTSS_ACL_POLICER_NO_START);
    vtss_acl_policer_conf_t *conf = &vtss_state->vcap.acl_policer_conf[i];
    
    rate = MIN(0x1ffff, conf->rate);
    JR_WRX(ANA_AC, POL_ALL_CFG_POL_ACL_RATE_CFG, i, 
           JR_PUT_FLD(ANA_AC, POL_ALL_CFG_POL_ACL_RATE_CFG, ACL_RATE, rate));
    
    /* 4 kB threshold */
    JR_WRX(ANA_AC, POL_ALL_CFG_POL_ACL_THRES_CFG, i, 
           JR_PUT_FLD(ANA_AC, POL_ALL_CFG_POL_ACL_THRES_CFG, ACL_THRES, 1));
    
    /* Frame rate */
    JR_WRX(ANA_AC, POL_ALL_CFG_POL_ACL_CTRL, i, 
           JR_PUT_FLD(ANA_AC, POL_ALL_CFG_POL_ACL_CTRL, ACL_TRAFFIC_TYPE_MASK, 3) |
           JR_PUT_BIT(ANA_AC, POL_ALL_CFG_POL_ACL_CTRL, FRAME_RATE_ENA, 1));

    return VTSS_RC_OK;
}

static vtss_rc jr_acl_policer_set(vtss_state_t *vtss_state,
                                  const vtss_acl_policer_no_t policer_no)
{
    vtss_chip_no_t chip_no;

    /* Setup all devices */
    for (chip_no = 0; chip_no < vtss_state->chip_count; chip_no++) {
        VTSS_SELECT_CHIP(chip_no);
        VTSS_RC(jr_acl_policer_set_chip(vtss_state, policer_no));
    }
    return VTSS_RC_OK;
}

static vtss_rc jr_acl_port_conf_set(vtss_state_t *vtss_state,
                                    const vtss_port_no_t port_no)
{
    vtss_acl_port_conf_t *conf = &vtss_state->vcap.acl_port_conf[port_no];
    u32                  pag = 0, cnt, port = VTSS_CHIP_PORT(port_no);
    
    /* Enable/disable IS2 lookups on port */
    VTSS_RC(vtss_jr1_vcap_lookup_set(vtss_state, port_no));

    /* Setup PAG using default actions in IS0 */
    if (conf->policy_no != VTSS_ACL_POLICY_NO_NONE)
        pag = (conf->policy_no - VTSS_ACL_POLICY_NO_START);
    VTSS_RC(vtss_jr1_vcap_port_command(vtss_state, VTSS_TCAM_IS0, port, VTSS_TCAM_CMD_READ));
    JR_WRF(VCAP_IS0, BASETYPE_ACTION_B, PAG_VAL, pag);
    VTSS_RC(vtss_jr1_vcap_port_command(vtss_state, VTSS_TCAM_IS0, port, VTSS_TCAM_CMD_WRITE));

    /* Setup action */
    VTSS_RC(vtss_jr1_vcap_port_command(vtss_state, VTSS_TCAM_IS2, port, VTSS_TCAM_CMD_READ));
    JR_RD(VCAP_IS2, BASETYPE_ACTION_D, &cnt);
    VTSS_RC(jr_is2_prepare_action(vtss_state, &conf->action, 0, cnt));
    return vtss_jr1_vcap_port_command(vtss_state, VTSS_TCAM_IS2, port, VTSS_TCAM_CMD_WRITE);
}

static vtss_rc jr_acl_port_counter_get(vtss_state_t *vtss_state,
                                       const vtss_port_no_t    port_no,
                                       vtss_acl_port_counter_t *const counter)
{
    VTSS_RC(vtss_jr1_vcap_port_command(vtss_state, VTSS_TCAM_IS2, VTSS_CHIP_PORT(port_no), VTSS_TCAM_CMD_READ));
    JR_RD(VCAP_IS2, BASETYPE_ACTION_D, counter);
    return VTSS_RC_OK;
}

static vtss_rc jr_acl_port_counter_clear(vtss_state_t *vtss_state, const vtss_port_no_t port_no)
{
    u32 port = VTSS_CHIP_PORT(port_no);
    
    VTSS_RC(vtss_jr1_vcap_port_command(vtss_state, VTSS_TCAM_IS2, port, VTSS_TCAM_CMD_READ));
    JR_WR(VCAP_IS2, BASETYPE_ACTION_D, 0);
    VTSS_RC(vtss_jr1_vcap_port_command(vtss_state, VTSS_TCAM_IS2, port, VTSS_TCAM_CMD_WRITE));
    return VTSS_RC_OK;
}

static vtss_rc jr_ace_add(vtss_state_t *vtss_state,
                          const vtss_ace_id_t ace_id, const vtss_ace_t *const ace)
{
    vtss_vcap_obj_t             *is2_obj = &vtss_state->vcap.is2.obj;
    vtss_vcap_user_t            is2_user = VTSS_IS2_USER_ACL;
    vtss_vcap_data_t            data;
    vtss_is2_data_t             *is2 = &data.u.is2;
    vtss_is2_entry_t            entry;
    vtss_ace_t                  *ace_copy = &entry.ace;
    const vtss_ace_udp_tcp_t    *sport = NULL, *dport = NULL;
    vtss_vcap_id_t              id, id_next;
    u32                         old = 0, old_any = 0, new_any = 0;
    vtss_vcap_range_chk_table_t range_new = vtss_state->vcap.range; 
    
    /*** Step 1: Check the simple things */
    VTSS_RC(vtss_cmn_ace_add(vtss_state, ace_id, ace));

    /*** Step 2: Check if IS2 entries can be added */

    /* Initialize entry data */
    vtss_vcap_is2_init(&data, &entry);

    /* Check if main entry exists */
    if (vtss_vcap_lookup(vtss_state, is2_obj, is2_user, ace->id, &data, NULL) == VTSS_RC_OK) {
        is2->entry = &entry; /* NOTE: Restore entry pointer which was overwritten by vtss_vcap_lookup() */
        old = 1;
    }

    /* Check if extra any entries exist */
    id = (ace->id + JR_IS2_ID_ANY_LLC);
    if (vtss_vcap_lookup(vtss_state, is2_obj, is2_user, id, NULL, NULL) == VTSS_RC_OK)
        old_any = JR_IS2_ID_ANY_CNT;

    /* Determine IS2 type and check if extra any entries must be added */
    switch (ace->type) {
    case VTSS_ACE_TYPE_ANY:
        entry.type = IS2_ENTRY_TYPE_ETYPE;
        entry.type_mask = JR_IS2_ANY_ETYPE_MASK;
        new_any = JR_IS2_ID_ANY_CNT;
        break;
    case VTSS_ACE_TYPE_ETYPE:
        entry.type = IS2_ENTRY_TYPE_ETYPE;
        break;
    case VTSS_ACE_TYPE_LLC:
        entry.type = IS2_ENTRY_TYPE_LLC;
        break;
    case VTSS_ACE_TYPE_SNAP:
        entry.type = IS2_ENTRY_TYPE_SNAP;
        break;
    case VTSS_ACE_TYPE_ARP:
        entry.type = IS2_ENTRY_TYPE_ARP;
        break;
    case VTSS_ACE_TYPE_IPV4:
        if (!vtss_vcap_udp_tcp_rule(&ace->frame.ipv4.proto))
            entry.type_mask = JR_IS2_ANY_IP_MASK;
        entry.type = IS2_ENTRY_TYPE_IP_OTHER;
        break;
    case VTSS_ACE_TYPE_IPV6:
        if (!vtss_vcap_udp_tcp_rule(&ace->frame.ipv6.proto))
            entry.type_mask = JR_IS2_ANY_IP_MASK;
        entry.type = IS2_ENTRY_TYPE_IP_OTHER;
        break;
    default:
        VTSS_E("illegal type: %d", ace->type);
        return VTSS_RC_ERROR;
    }

    if ((is2_obj->count + new_any) >= (is2_obj->max_count + old + old_any)) {
        VTSS_I("IS2 is full");
        return VTSS_RC_ERROR;
    }

    /* Calculate next ID and check that it exists */
    id = (ace_id + JR_IS2_ID_ANY_LLC);
    id_next = (vtss_vcap_lookup(vtss_state, is2_obj, is2_user, id, NULL, NULL) == VTSS_RC_OK ? 
               id : ace_id);
    VTSS_RC(vtss_vcap_add(vtss_state, is2_obj, is2_user, ace->id, id_next, NULL, 0));

    /*** Step 3: Allocate range checkers */
    /* Free any old range checkers */
    VTSS_RC(vtss_vcap_range_free(&range_new, is2->srange));
    VTSS_RC(vtss_vcap_range_free(&range_new, is2->drange));
    is2->srange = VTSS_VCAP_RANGE_CHK_NONE;
    is2->drange = VTSS_VCAP_RANGE_CHK_NONE;

    if (ace->type == VTSS_ACE_TYPE_IPV4 && vtss_vcap_udp_tcp_rule(&ace->frame.ipv4.proto)) {
        sport = &ace->frame.ipv4.sport;
        dport = &ace->frame.ipv4.dport;
    } else if (ace->type == VTSS_ACE_TYPE_IPV6 && vtss_vcap_udp_tcp_rule(&ace->frame.ipv6.proto)) {
        sport = &ace->frame.ipv6.sport;
        dport = &ace->frame.ipv6.dport;
    }

    if (sport && dport) {
        /* Allocate new range checkers */
        VTSS_RC(vtss_vcap_udp_tcp_range_alloc(&range_new, &is2->srange, sport, 1));
        VTSS_RC(vtss_vcap_udp_tcp_range_alloc(&range_new, &is2->drange, dport, 0));
    }

    /* Commit range checkers */
    VTSS_RC(vtss_vcap_range_commit(vtss_state, &range_new));

    /*** Step 4: Add IS2 entries */

    /* Add main entry */
    *ace_copy = *ace;
    is2->entry = &entry;
    VTSS_RC(vtss_vcap_add(vtss_state, is2_obj, is2_user, ace->id, id_next, &data, 0));
    if (new_any) {
        /* Add block of IS2 rules matching all frame types */
        VTSS_RC(vtss_jr1_is2_add_any(vtss_state, is2_user, ace->id, id_next, &data, 0));
    } else if (old_any) {
        /* Delete extra any entries */
        VTSS_RC(vtss_jr1_is2_del_any(vtss_state, is2_user, ace->id, 0));
    }

    return VTSS_RC_OK;
}

static vtss_rc jr_ace_del(vtss_state_t *vtss_state, const vtss_ace_id_t ace_id)
{
    /* Delete main entry */
    VTSS_RC(vtss_cmn_ace_del(vtss_state, ace_id));
    
    /* Delete extra any entries */
    VTSS_RC(vtss_jr1_is2_del_any(vtss_state, VTSS_IS2_USER_ACL, ace_id, 0));
    
    return VTSS_RC_OK;
}

static vtss_rc jr_ace_get(vtss_state_t *vtss_state,
                          vtss_vcap_id_t id, vtss_ace_counter_t *const counter, BOOL clear)
{
    vtss_rc         rc;
    vtss_vcap_obj_t *obj = &vtss_state->vcap.is2.obj;
    u32             cnt = 0;
    vtss_vcap_idx_t idx;
    
    /* Add/clear counter from extra entry */
    if ((rc = vtss_vcap_lookup(vtss_state, obj, VTSS_IS2_USER_ACL, id, NULL, &idx)) == VTSS_RC_OK) {
        VTSS_RC(obj->entry_get(vtss_state, &idx, &cnt, clear));
        if (counter != NULL)
            *counter += cnt;
    }
    return rc;
}

static vtss_rc jr_ace_get_any(vtss_state_t *vtss_state,
                              const vtss_ace_id_t ace_id, 
                              vtss_ace_counter_t *const counter, 
                              BOOL clear)
{
    if (jr_ace_get(vtss_state, ace_id + JR_IS2_ID_ANY_LLC, counter, clear) == VTSS_RC_OK) {
        VTSS_RC(jr_ace_get(vtss_state, ace_id + JR_IS2_ID_ANY_SNAP, counter, clear));
        VTSS_RC(jr_ace_get(vtss_state, ace_id + JR_IS2_ID_ANY_OAM, counter, clear));
        VTSS_RC(jr_ace_get(vtss_state, ace_id + JR_IS2_ID_ANY_IP, counter, clear));
    }
    return VTSS_RC_OK;
}

static vtss_rc jr_ace_counter_get(vtss_state_t *vtss_state,
                                  const vtss_ace_id_t ace_id, vtss_ace_counter_t *const counter)
{
    VTSS_RC(vtss_cmn_ace_counter_get(vtss_state, ace_id, counter));
    return jr_ace_get_any(vtss_state, ace_id, counter, 0);
}

static vtss_rc jr_ace_counter_clear(vtss_state_t *vtss_state,
                                    const vtss_ace_id_t ace_id)
{
    VTSS_RC(vtss_cmn_ace_counter_clear(vtss_state, ace_id));
    return jr_ace_get_any(vtss_state, ace_id, NULL, 1);
}

static vtss_rc jr_vcap_range_commit(vtss_state_t *vtss_state)
{
    vtss_chip_no_t        chip_no;
    u32                   i, type;
    vtss_vcap_range_chk_t *entry;
    
    for (i = 0; i < VTSS_VCAP_RANGE_CHK_CNT; i++) {
        entry = &vtss_state->vcap.range.entry[i];
        if (entry->count == 0)
            continue;
        switch (entry->type) {
        case VTSS_VCAP_RANGE_TYPE_SPORT:
            type = 2;
            break;
        case VTSS_VCAP_RANGE_TYPE_DPORT:
            type = 1;
            break;
        case VTSS_VCAP_RANGE_TYPE_SDPORT:
            type = 3;
            break;
        case VTSS_VCAP_RANGE_TYPE_VID:
            type = 4;
            break;
        case VTSS_VCAP_RANGE_TYPE_DSCP:
            type = 5;
            break;
        default:
            VTSS_E("illegal type: %d", entry->type);
            return VTSS_RC_ERROR;
        }
        for (chip_no = 0; chip_no < vtss_state->chip_count; chip_no++) {
            VTSS_SELECT_CHIP(chip_no);
            JR_WRX(ANA_CL_2, COMMON_ADV_RNG_CTRL, i, 
                   JR_PUT_FLD(ANA_CL_2, COMMON_ADV_RNG_CTRL, RNG_TYPE_SEL, type));
            JR_WRX(ANA_CL_2, COMMON_ADV_RNG_VALUE_CFG, i, 
                   JR_PUT_FLD(ANA_CL_2, COMMON_ADV_RNG_VALUE_CFG, RNG_MAX_VALUE, entry->max) |
                   JR_PUT_FLD(ANA_CL_2, COMMON_ADV_RNG_VALUE_CFG, RNG_MIN_VALUE, entry->min));
        }
    }
    return VTSS_RC_OK;
}

/* - Debug print --------------------------------------------------- */

static void jr_debug_bit(const vtss_debug_printf_t pr, const char *name, u32 value)
{
    int  i, length = strlen(name);

    for (i = 0; i < length; i++)
        pr("%c", value ? toupper(name[i]) : tolower(name[i]));
    pr(" ");
}

static void jr_debug_fld(const vtss_debug_printf_t pr, const char *name, u32 value)
{
    pr("%s: ", name);
    if (value < 10)
        pr("%u", value);
    else
        pr("0x%x", value);
    pr(" ");
}

static void jr_debug_action(const vtss_debug_printf_t pr, const char *name, u32 ena, u32 val)
{
    int i, length = strlen(name);

    for (i = 0; i < length; i++)
        pr("%c", ena ? toupper(name[i]) : tolower(name[i]));
    pr(":%u ", val);
}

static void jr_debug_vcap_reg(const vtss_debug_printf_t pr, 
                              const char *name, jr_vcap_reg_t *reg, u32 mask)
{
    u32 i, j, m;

    if (reg->valid) {
        if (name)
            pr("%s: ", name);

        if (mask == 0) /* Show all bits by default */
            mask = 0xffffffff;

        for (i = 0, j = 0; i < 32; i++) {
            m = VTSS_BIT(31 - i);
            if (m & mask) {
                if ((name == NULL || j != 0) && (j % 8) == 0)
                    pr(".");
                j++;
                pr("%c", (reg->mask & m) ? 'X' : (reg->value & m) ? '1' : '0') ;
            }
        }
        pr(j > 23 ? "\n" : " ");
    }
}

static void jr_debug_vcap_mac(const vtss_debug_printf_t pr, 
                              const char *name, jr_vcap_mac_t *reg)
{
    jr_debug_vcap_reg(pr, name, &reg->mach, 0xffff);
    jr_debug_vcap_reg(pr, NULL, &reg->macl, 0);
}

static vtss_rc jr_debug_vcap(vtss_state_t *vtss_state,
                             int bank,
                             const char *name,
                             const vtss_debug_printf_t pr,
                             const vtss_debug_info_t   *const info,
                             vtss_rc (* dbg)(vtss_state_t *, const vtss_debug_printf_t pr, BOOL action))
{
    /*lint --e{454, 455, 456} */ // Due to VTSS_EXIT_ENTER
    const tcam_props_t *tcam = &tcam_info[bank];
    int                i, is_entry;
    
    vtss_debug_print_header(pr, name);
    
    for (i = (tcam->actions - 1); i >= 0; i--) {
        /* Leave critical region briefly */
        VTSS_EXIT_ENTER();

        /* Read entry */
        if (jr_vcap_command(vtss_state, bank, i, VTSS_TCAM_CMD_READ) != VTSS_RC_OK)
            continue;

        /* Print entry */
        is_entry = (i < tcam->entries);
        if (is_entry) {
            if (dbg(vtss_state, pr, 0) != VTSS_RC_OK)
                continue;
            pr("\n");
        }
        
        /* Print action */
        pr("%d (", i);
        if (is_entry)
            pr("rule %d", tcam->entries - i - 1);
        else if (bank == VTSS_TCAM_IS1)
            pr("%s_default", i == (tcam->actions - 1) ? "qos" : "vlan_pag");
        else
            pr("port %d", i - tcam->entries);
        pr("): ");
        if (dbg(vtss_state, pr, 1) == VTSS_RC_OK)
            pr("\n-------------------------------------------\n");
    }
    pr("\n");
    return VTSS_RC_OK;
}
    
static vtss_rc jr_debug_is0(vtss_state_t *vtss_state, const vtss_debug_printf_t pr, BOOL action)
{
    u32           value, type;
    jr_vcap_reg_t reg, reg1;

    if (action) {
        JR_RD(VCAP_IS0, BASETYPE_ACTION_A, &value);
        jr_debug_bit(pr, "s1_dmac_ena", value & VTSS_F_VCAP_IS0_BASETYPE_ACTION_A_S1_DMAC_ENA);
        jr_debug_bit(pr, "l3_mc_lookup_dis", 
                     value & VTSS_F_VCAP_IS0_BASETYPE_ACTION_A_L3_MC_LOOKUP_DIS);
        jr_debug_bit(pr, "oam_detect_dis", 
                     value & VTSS_F_VCAP_IS0_BASETYPE_ACTION_A_OAM_DETECT_DIS);
        jr_debug_bit(pr, "second_lookup_dis", 
                     value & VTSS_F_VCAP_IS0_BASETYPE_ACTION_A_SECOND_LOOKUP_DIS);
        pr("\n");
        jr_debug_bit(pr, "mac_pop_cnt", 
                     value & VTSS_F_VCAP_IS0_BASETYPE_ACTION_A_MAC_POP_CNT);
        jr_debug_fld(pr, "vlan_pop_cnt", 
                     VTSS_X_VCAP_IS0_BASETYPE_ACTION_A_VLAN_POP_CNT(value));
        jr_debug_bit(pr, "bvid_ena", 
                     value & VTSS_F_VCAP_IS0_BASETYPE_ACTION_A_BVID_ENA);
        jr_debug_bit(pr, "vid_ena", 
                     value & VTSS_F_VCAP_IS0_BASETYPE_ACTION_A_VID_ENA);
        jr_debug_bit(pr, "dei_val", 
                     value & VTSS_F_VCAP_IS0_BASETYPE_ACTION_A_DEI_VAL);
        jr_debug_fld(pr, "pcp_val", 
                     VTSS_X_VCAP_IS0_BASETYPE_ACTION_A_PCP_VAL(value));
        jr_debug_bit(pr, "pcp_dei_ena", 
                     value & VTSS_F_VCAP_IS0_BASETYPE_ACTION_A_PCP_DEI_ENA);
        pr("\n");
        JR_RD(VCAP_IS0, BASETYPE_ACTION_B, &value);
        jr_debug_fld(pr, "isdx_val", 
                     VTSS_X_VCAP_IS0_BASETYPE_ACTION_B_ISDX_VAL(value));
        jr_debug_fld(pr, "vid_val", 
                     VTSS_X_VCAP_IS0_BASETYPE_ACTION_B_VID_VAL(value));
        jr_debug_fld(pr, "pag_val", 
                     VTSS_X_VCAP_IS0_BASETYPE_ACTION_B_PAG_VAL(value));
        JR_RD(VCAP_IS0, BASETYPE_ACTION_C, &value);
        jr_debug_bit(pr, "hit", value);
        
        return VTSS_RC_OK;
    }

    JR_RDF(VCAP_IS0, IS0_CONTROL_ACE_STATUS, ACE_ENTRY_TYPE, &type);

    switch (type) {
    case JR_IS0_TYPE_ISID:
        JR_RD(VCAP_IS0, ISID_ENTRY_A, &value);
        if ((value & VTSS_F_VCAP_IS0_ISID_ENTRY_A_VLD) == 0)
            return VTSS_RC_ERROR;
        pr("type: isid");
        break;
    case JR_IS0_TYPE_DBL_VID:
        JR_IS0_RD(DBL_VID, A, reg);
        if ((reg.value & VTSS_F_VCAP_IS0_DBL_VID_ENTRY_A_VLD) == 0)
            return VTSS_RC_ERROR;
        pr("type: dbl_vid ");
        JR_IS0_DEBUG_FLD(pr, "vigr_port", DBL_VID, A, VIGR_PORT, &reg);

        /* DBL_VID0 and DBL_VID1 */
        JR_IS0_RD(DBL_VID, DBL_VID0, reg);
        JR_IS0_RD(DBL_VID, DBL_VID1, reg1);
        JR_IS0_DEBUG_BIT(pr, "tagged", DBL_VID, DBL_VID0, VLAN_TAGGED, &reg);
        JR_IS0_DEBUG_BIT(pr, "dbl_tagged", DBL_VID, DBL_VID0, VLAN_DBL_TAGGED, &reg);
        pr("\n");
        JR_IS0_DEBUG_FLD(pr, "outer_vid", DBL_VID, DBL_VID1, OUTER_VID, &reg1);
        JR_IS0_DEBUG_FLD(pr, "outer_pcp", DBL_VID, DBL_VID0, OUTER_PCP, &reg);
        JR_IS0_DEBUG_BIT(pr, "outer_dei", DBL_VID, DBL_VID0, OUTER_DEI, &reg);
        JR_IS0_DEBUG_BIT(pr, "outer_tpi", DBL_VID, DBL_VID0, OUTER_TPI, &reg);
        pr("\n");
        JR_IS0_DEBUG_FLD(pr, "inner_vid", DBL_VID, DBL_VID1, INNER_VID, &reg1);
        JR_IS0_DEBUG_FLD(pr, "inner_pcp", DBL_VID, DBL_VID0, INNER_PCP, &reg);
        JR_IS0_DEBUG_BIT(pr, "inner_dei", DBL_VID, DBL_VID0, INNER_DEI, &reg);
        JR_IS0_DEBUG_BIT(pr, "inner_tpi", DBL_VID, DBL_VID0, INNER_TPI, &reg);
        pr("\n");
        
        /* DBL_VID2 */
        JR_IS0_RD(DBL_VID, DBL_VID2, reg);
        JR_IS0_DEBUG_FLD(pr, "l3_dscp", DBL_VID, DBL_VID2, L3_DSCP, &reg);
        JR_IS0_DEBUG_FLD(pr, "prot", DBL_VID, DBL_VID2, PROT, &reg);
        pr("\n");
        break;
    case JR_IS0_TYPE_MPLS:
        JR_RD(VCAP_IS0, MPLS_ENTRY_A, &value);
        if ((value & VTSS_F_VCAP_IS0_MPLS_ENTRY_A_VLD) == 0)
            return VTSS_RC_ERROR;
        pr("type: mpls");
        break;
    case JR_IS0_TYPE_MAC_ADDR:
        JR_RD(VCAP_IS0, MAC_ADDR_ENTRY_A, &value);
        if ((value & VTSS_F_VCAP_IS0_MAC_ADDR_ENTRY_A_VLD) == 0)
            return VTSS_RC_ERROR;
        pr("type: mac_addr");
        break;
    default:
        break;
    }

    return VTSS_RC_OK;
}

vtss_rc vtss_jr1_debug_vcap_is0(vtss_state_t *vtss_state,
                                const vtss_debug_printf_t pr,
                                const vtss_debug_info_t   *const info)
{
    return jr_debug_vcap(vtss_state, VTSS_TCAM_IS0, "IS0", pr, info, jr_debug_is0);
}

/* Read IS1 common registers */
#define JR_IS1_RD_COMMON(pre, regs)        \
{                                          \
    JR_IS1_RD(pre, ENTRY, regs.entry);     \
    JR_IS1_RD(pre, IF_GRP, regs.if_grp);   \
    JR_IS1_RD(pre, VLAN, regs.vlan);       \
    JR_IS1_RD(pre, FLAGS, regs.flags);     \
    JR_IS1_RD(pre, L3_IP4_SIP, regs.sip);  \
    JR_IS1_RD(pre, L3_MISC, regs.l3_misc); \
    JR_IS1_RD(pre, L4_MISC, regs.l4_misc); \
}

static vtss_rc jr_debug_is1(vtss_state_t *vtss_state, const vtss_debug_printf_t pr, BOOL action)
{
    u32                type, x;
    jr_vcap_is1_regs_t regs;

    JR_RDB(VCAP_IS1, IS1_CONTROL_ACE_STATUS, ACE_ACTION_TYPE, &type);
    if (action) {
        if (type) {
            /* QOS_ACTION */
            JR_RD(VCAP_IS1, QOS_ACTION_DSCP, &x);
            jr_debug_action(pr, "dscp", 
                            JR_GET_BIT(VCAP_IS1, QOS_ACTION_DSCP, DSCP_ENA, x), 
                            JR_GET_FLD(VCAP_IS1, QOS_ACTION_DSCP, DSCP_VAL, x));
            JR_RD(VCAP_IS1, QOS_ACTION_QOS, &x);
            jr_debug_action(pr, "qos",
                            JR_GET_BIT(VCAP_IS1, QOS_ACTION_QOS, QOS_ENA, x),
                            JR_GET_FLD(VCAP_IS1, QOS_ACTION_QOS, QOS_VAL, x));
            JR_RD(VCAP_IS1, QOS_ACTION_DP, &x);
            jr_debug_action(pr, "dp",
                            JR_GET_BIT(VCAP_IS1, QOS_ACTION_DP, DP_ENA, x),
                            JR_GET_FLD(VCAP_IS1, QOS_ACTION_DP, DP_VAL, x));
            JR_RD(VCAP_IS1, QOS_ACTION_STICKY, &x);
        } else {
            /* VLAN_PAG_ACTION */
            JR_RD(VCAP_IS1, VLAN_PAG_ACTION_PAG, &x);
            jr_debug_fld(pr, "pag_val", 
                         JR_GET_FLD(VCAP_IS1, VLAN_PAG_ACTION_PAG, PAG_VAL, x));
            jr_debug_fld(pr, "pag_mask", 
                         JR_GET_FLD(VCAP_IS1, VLAN_PAG_ACTION_PAG, PAG_OVERRIDE_MASK, x));
            JR_RD(VCAP_IS1, VLAN_PAG_ACTION_MISC, &x);
            jr_debug_action(pr, "custom_type", 
                            JR_GET_BIT(VCAP_IS1, VLAN_PAG_ACTION_MISC, CUSTOM_ACE_TYPE_ENA, x),
                            JR_GET_FLD(VCAP_IS1, VLAN_PAG_ACTION_MISC, CUSTOM_ACE_TYPE_VAL, x));
            pr("\n");
            jr_debug_action(pr, "pcp", 
                            JR_GET_BIT(VCAP_IS1, VLAN_PAG_ACTION_MISC, PCP_DEI_ENA, x),
                            JR_GET_FLD(VCAP_IS1, VLAN_PAG_ACTION_MISC, PCP_VAL,x));
            jr_debug_fld(pr, "dei", 
                         JR_GET_BIT(VCAP_IS1, VLAN_PAG_ACTION_MISC, DEI_VAL, x));
            jr_debug_action(pr, "vid", 
                            JR_GET_BIT(VCAP_IS1, VLAN_PAG_ACTION_MISC, VID_REPLACE_ENA, x),
                            JR_GET_FLD(VCAP_IS1, VLAN_PAG_ACTION_MISC, VID_ADD_VAL, x));
            JR_RD(VCAP_IS1, VLAN_PAG_ACTION_CUSTOM_POS, &x);
            jr_debug_fld(pr, "pos_0", 
                         JR_GET_FLD(VCAP_IS1, VLAN_PAG_ACTION_CUSTOM_POS, CUSTOM_POS_VAL_0, x));
            jr_debug_fld(pr, "pos_1", 
                         JR_GET_FLD(VCAP_IS1, VLAN_PAG_ACTION_CUSTOM_POS, CUSTOM_POS_VAL_1, x));
            jr_debug_fld(pr, "pos_2", 
                         JR_GET_FLD(VCAP_IS1, VLAN_PAG_ACTION_CUSTOM_POS, CUSTOM_POS_VAL_2, x));
            jr_debug_fld(pr, "pos_3", 
                         JR_GET_FLD(VCAP_IS1, VLAN_PAG_ACTION_CUSTOM_POS, CUSTOM_POS_VAL_3, x));
            JR_RD(VCAP_IS1, VLAN_PAG_ACTION_ISDX, &x);
            jr_debug_action(pr, "isdx", 
                            JR_GET_BIT(VCAP_IS1, VLAN_PAG_ACTION_ISDX, ISDX_REPLACE_ENA, x),
                            JR_GET_FLD(VCAP_IS1, VLAN_PAG_ACTION_ISDX, ISDX_ADD_VAL, x));
            JR_RD(VCAP_IS1, VLAN_PAG_ACTION_STICKY, &x);
        }
        jr_debug_bit(pr, "hit", x);
        return VTSS_RC_OK;
    }

    memset(&regs, 0, sizeof(regs));
    if (type) {
        /* QOS_ENTRY */
        JR_IS1_RD_COMMON(QOS, regs);
        JR_IS1_RD(QOS, L2_MAC_ADDR_HIGH, regs.mac.mach);
        JR_IS1_RD(QOS, L4_PORT, regs.l4_port);
    } else {
        /* VLAN_PAG_ENTRY */
        JR_IS1_RD_COMMON(VLAN_PAG, regs);
        JR_IS1_RD_MAC(VLAN_PAG, L2_MAC_ADDR, regs.mac);
    }
    
    /* The register layout is the same for different entry types */
    /* Check if entry is valid */
    if (JR_GET_BIT(VCAP_IS1, QOS_ENTRY_ENTRY, VLD, regs.entry.value) == 0)
        return VTSS_RC_ERROR;

    jr_debug_fld(pr, type ? "type_qos" : "type_vlan_pag", type);    
    
    /* ENTRY */
    JR_IS1_DEBUG_FLD(pr, "pag", QOS, ENTRY, PAG, &regs.entry);
    JR_IS1_DEBUG_BIT(pr, "vport_vld", QOS, ENTRY, VPORT_VLD, &regs.entry);
    JR_IS1_DEBUG_BIT(pr, "isdx_neq0", QOS, ENTRY, ISDX_NEQ0, &regs.entry);
    pr("\n");
    
    /* IF_GRP */
    jr_debug_vcap_reg(pr, "igr_port_mask", &regs.if_grp, 0);

    /* VLAN */
    JR_IS1_DEBUG_BIT(pr, "vlan_tagged", QOS, VLAN, VLAN_TAGGED, &regs.vlan);
    JR_IS1_DEBUG_BIT(pr, "dbl_tagged", QOS, VLAN, VLAN_DBL_TAGGED, &regs.vlan);
    JR_IS1_DEBUG_BIT(pr, "in_dei", QOS, VLAN, INNER_DEI, &regs.vlan);
    JR_IS1_DEBUG_BIT(pr, "ou_dei", QOS, VLAN, OUTER_DEI, &regs.vlan);
    JR_IS1_DEBUG_FLD(pr, "pcp", QOS, VLAN, PCP, &regs.vlan);
    JR_IS1_DEBUG_FLD(pr, "vid", QOS, VLAN, VID, &regs.vlan);
    pr("\n");

    /* FLAGS */
    JR_IS1_DEBUG_BIT(pr, "l2_bc", QOS, FLAGS, L2_BC, &regs.flags);
    JR_IS1_DEBUG_BIT(pr, "l2_mc", QOS, FLAGS, L2_MC, &regs.flags);
    JR_IS1_DEBUG_BIT(pr, "etype_len", QOS, FLAGS, ETYPE_LEN, &regs.flags);
    JR_IS1_DEBUG_BIT(pr, "ip", QOS, FLAGS, IP, &regs.flags);
    JR_IS1_DEBUG_BIT(pr, "ip_mc", QOS, FLAGS, IP_MC, &regs.flags);
    JR_IS1_DEBUG_BIT(pr, "ip4", QOS, FLAGS, IP4, &regs.flags);
    JR_IS1_DEBUG_BIT(pr, "tcp_udp", QOS, FLAGS, TCP_UDP, &regs.flags);
    JR_IS1_DEBUG_BIT(pr, "tcp", QOS, FLAGS, TCP, &regs.flags);
    pr("\n");
    JR_IS1_DEBUG_FLD(pr, "etype", QOS, FLAGS, ETYPE, &regs.flags);

    /* L2_MAC_ADDR */
    JR_IS1_DEBUG_FLD(pr, "mac_hi", QOS, L2_MAC_ADDR_HIGH, L2_MAC_ADDR_HIGH, &regs.mac.mach);
    JR_IS1_DEBUG_FLD(pr, "mac_lo", VLAN_PAG, L2_MAC_ADDR_LOW, L2_MAC_ADDR_LOW, &regs.mac.macl);
    
    /* L3_IP_SIP */
    jr_debug_vcap_reg(pr, "sip", &regs.sip, 0);

    /* L3_MISC */
    JR_IS1_DEBUG_BIT(pr, "fragment", QOS, L3_MISC, L3_FRAGMENT, &regs.l3_misc);
    
    /* L4_MISC */
    JR_IS1_DEBUG_FLD(pr, "l4_rng", QOS, L4_MISC, L4_RNG, &regs.l4_misc);

    /* L4_PORT */
    JR_IS1_DEBUG_FLD(pr, "l4_sport", QOS, L4_PORT, L4_SPORT, &regs.l4_port);
    pr("\n");
    
    return VTSS_RC_OK;
}

vtss_rc vtss_jr1_debug_vcap_is1(vtss_state_t *vtss_state,
                                const vtss_debug_printf_t pr,
                                const vtss_debug_info_t   *const info)
{
    return jr_debug_vcap(vtss_state, VTSS_TCAM_IS1, "IS1", pr, info, jr_debug_is1);
}

/* The register field layout is the same for different entry types, 
   so we use a common function to extract and show fields */
static vtss_rc jr_debug_is2_regs(const vtss_debug_printf_t pr,
                                 const char *name,
                                 jr_vcap_is2_regs_t *regs)
{
    int  i;
    char buf[80];

    /* Check if entry is valid */
    if (JR_GET_BIT(VCAP_IS2, MAC_ETYPE_ENTRY_ACE_VLD, VLD, regs->vld.value) == 0)
        return VTSS_RC_ERROR;
    
    /* Type */
    sprintf(buf, "type_%s", name);
    jr_debug_vcap_reg(pr, buf, &regs->entry_type, 0xf);    

    /* ACE_TYPE */
    JR_IS2_DEBUG_BIT(pr, "first", MAC_ETYPE, ACE_TYPE, FIRST, &regs->type);
    JR_IS2_DEBUG_BIT(pr, "l2_mc", MAC_ETYPE, ACE_TYPE, L2_MC, &regs->type);
    JR_IS2_DEBUG_BIT(pr, "l2_bc", MAC_ETYPE, ACE_TYPE, L2_BC, &regs->type);
    pr("\n");
    JR_IS2_DEBUG_FLD(pr, "vid", MAC_ETYPE, ACE_TYPE, VID, &regs->type);
    JR_IS2_DEBUG_FLD(pr, "uprio", MAC_ETYPE, ACE_TYPE, UPRIO, &regs->type);
    JR_IS2_DEBUG_BIT(pr, "cfi", MAC_ETYPE, ACE_TYPE, CFI, &regs->type);
    JR_IS2_DEBUG_BIT(pr, "vlan_tagged", MAC_ETYPE, ACE_TYPE, VLAN_TAGGED, &regs->type);
    pr("\n");
    JR_IS2_DEBUG_BIT(pr, "l2_fwd", MAC_ETYPE, ACE_TYPE, L2_FWD, &regs->type);

    switch (regs->entry_type.value) {
    case IS2_ENTRY_TYPE_LLC:
    case IS2_ENTRY_TYPE_SNAP:
    case IS2_ENTRY_TYPE_ARP:
    case IS2_ENTRY_TYPE_OAM:
        break;
    default:
        /* These fields are not available for all frame types */
        JR_IS2_DEBUG_BIT(pr, "l3_rt", MAC_ETYPE, ACE_TYPE, L3_RT, &regs->type);
        JR_IS2_DEBUG_BIT(pr, "l3_smac_sip_match", MAC_ETYPE, ACE_TYPE, L3_SMAC_SIP_MATCH, 
                         &regs->type);
        JR_IS2_DEBUG_BIT(pr, "l3_dmac_dip_match", MAC_ETYPE, ACE_TYPE, L3_DMAC_DIP_MATCH, 
                         &regs->type);
        break;
    }
    JR_IS2_DEBUG_BIT(pr, "isdx_neq0", MAC_ETYPE, ACE_TYPE, ISDX_NEQ0, &regs->type);
    pr("\n");
    
    /* PAG */
    JR_IS2_DEBUG_FLD(pr, "pag", MAC_ETYPE, ACE_PAG, PAG, &regs->pag);

    /* Port mask */
    jr_debug_vcap_reg(pr, "igr_port_mask", &regs->port_mask, 0);

    /* SMAC/DMAC */
    jr_debug_vcap_mac(pr, "smac", &regs->smac);
    jr_debug_vcap_mac(pr, "dmac", &regs->dmac);

    /* SIP/DIP */
    jr_debug_vcap_reg(pr, "sip", &regs->sip, 0);
    jr_debug_vcap_reg(pr, "dip", &regs->dip, 0);
    
    /* MISC */
    JR_IS2_DEBUG_BIT(pr, "ip4", IP_OTHER, ACE_L2_MISC, IP4, &regs->l2_misc);
    JR_IS2_DEBUG_BIT(pr, "ttl", IP_OTHER, ACE_L3_MISC, L3_TTL_GT0, &regs->l3_misc);
    JR_IS2_DEBUG_BIT(pr, "frag", IP_OTHER, ACE_L3_MISC, L3_FRAGMENT, &regs->l3_misc);
    JR_IS2_DEBUG_BIT(pr, "options", IP_OTHER, ACE_L3_MISC, L3_OPTIONS, &regs->l3_misc);
    JR_IS2_DEBUG_FLD(pr, "tos", IP_OTHER, ACE_L3_MISC, L3_TOS, &regs->l3_misc);
    JR_IS2_DEBUG_FLD(pr, "proto", IP_OTHER, ACE_L3_MISC, L3_IP_PROTO, &regs->l3_misc);
    JR_IS2_DEBUG_BIT(pr, "dip_eq_sip", IP_OTHER, ACE_L3_MISC, DIP_EQ_SIP, &regs->l3_misc);
    if (regs->l3_misc.valid)
        pr("\n");

    /* CUSTOM_DATA */
    if (regs->data[0].valid) {
        for (i = 0; i < 4; i++) {
            sprintf(buf, "data_%u_%u", 2*i, 2*i + 1);
            jr_debug_vcap_reg(pr, buf, &regs->data[2*i], 0xffff);
            jr_debug_vcap_reg(pr, NULL, &regs->data[2*i+1], 0xffff);
            pr("\n");
        }
    }
    
    return VTSS_RC_OK;
}

/* Read IS2 common registers */
#define JR_IS2_RD_COMMON(pre, regs)                    \
{                                                      \
    JR_IS2_RD(pre, ACE_VLD, regs.vld);                 \
    JR_IS2_RD(pre, ACE_TYPE, regs.type);               \
    JR_IS2_RD(pre, ACE_IGR_PORT_MASK, regs.port_mask); \
    JR_IS2_RD(pre, ACE_PAG, regs.pag);                 \
}

/* Read IS2 L2 common registers */
#define JR_IS2_RD_L2(pre, regs)                 \
{                                               \
    JR_IS2_RD_COMMON(pre, regs);                \
    JR_IS2_RD_MAC(pre, ACE_L2_SMAC, regs.smac); \
    JR_IS2_RD_MAC(pre, ACE_L2_DMAC, regs.dmac); \
}

/* Read IS2 IP common registers */
#define JR_IS2_RD_IP(pre, regs)                \
{                                              \
    JR_IS2_RD_COMMON(pre, regs);               \
    JR_IS2_RD(pre, ACE_L3_IP4_SIP, regs.sip);  \
    JR_IS2_RD(pre, ACE_L3_IP4_DIP, regs.dip);  \
    JR_IS2_RD(pre, ACE_L2_MISC, regs.l2_misc); \
    JR_IS2_RD(pre, ACE_L3_MISC, regs.l3_misc); \
}

#define JR_IS2_RD_CUSTOM_IDX(pre, regs, ix) JR_IS2_RD(pre, ACE_CUSTOM_DATA_##ix, regs.data[ix])

/* Read IS2 CUSTOM registers */
#define JR_IS2_RD_CUSTOM(idx, regs)              \
{                                                \
    JR_IS2_RD_COMMON(CUSTOM_##idx, regs);        \
    JR_IS2_RD_CUSTOM_IDX(CUSTOM_##idx, regs, 0); \
    JR_IS2_RD_CUSTOM_IDX(CUSTOM_##idx, regs, 1); \
    JR_IS2_RD_CUSTOM_IDX(CUSTOM_##idx, regs, 2); \
    JR_IS2_RD_CUSTOM_IDX(CUSTOM_##idx, regs, 3); \
    JR_IS2_RD_CUSTOM_IDX(CUSTOM_##idx, regs, 4); \
    JR_IS2_RD_CUSTOM_IDX(CUSTOM_##idx, regs, 5); \
    JR_IS2_RD_CUSTOM_IDX(CUSTOM_##idx, regs, 6); \
    JR_IS2_RD_CUSTOM_IDX(CUSTOM_##idx, regs, 7); \
}

static vtss_rc jr_debug_is2(vtss_state_t *vtss_state, const vtss_debug_printf_t pr, BOOL action)
{
    u32                value;
    jr_vcap_is2_regs_t regs;
    jr_vcap_reg_t      misc;
    jr_vcap_mac_t      mac;
    
    if (action) {
        JR_RD(VCAP_IS2, BASETYPE_ACTION_A, &value);
        jr_debug_bit(pr, "hit_me", value & VTSS_F_VCAP_IS2_BASETYPE_ACTION_A_HIT_ME_ONCE);
        jr_debug_bit(pr, "cpu_copy", value & VTSS_F_VCAP_IS2_BASETYPE_ACTION_A_CPU_COPY_ENA);
        jr_debug_fld(pr, "cpu_qu", VTSS_X_VCAP_IS2_BASETYPE_ACTION_A_CPU_QU_NUM(value));
        jr_debug_bit(pr, "irq", value & VTSS_F_VCAP_IS2_BASETYPE_ACTION_A_IRQ_TRIGGER);
        jr_debug_bit(pr, "fwd", value & VTSS_F_VCAP_IS2_BASETYPE_ACTION_A_FW_ENA);
        jr_debug_bit(pr, "lrn", value & VTSS_F_VCAP_IS2_BASETYPE_ACTION_A_LRN_ENA);
        jr_debug_bit(pr, "rt", value & VTSS_F_VCAP_IS2_BASETYPE_ACTION_A_RT_ENA);
        jr_debug_bit(pr, "pol", value & VTSS_F_VCAP_IS2_BASETYPE_ACTION_A_POLICE_ENA);
        jr_debug_fld(pr, "pol_idx", VTSS_X_VCAP_IS2_BASETYPE_ACTION_A_POLICE_IDX(value));
        pr("\n");
        jr_debug_fld(pr, "dlb_offs", VTSS_X_VCAP_IS2_BASETYPE_ACTION_A_DLB_OFFSET(value));
        jr_debug_fld(pr, "mirror_probe", VTSS_X_VCAP_IS2_BASETYPE_ACTION_A_MIRROR_PROBE(value));
        jr_debug_fld(pr, "ccm_type", VTSS_X_VCAP_IS2_BASETYPE_ACTION_A_OAM_CCM_TYPE(value));
        jr_debug_bit(pr, "loopback", value & VTSS_F_VCAP_IS2_BASETYPE_ACTION_A_LOOPBACK_SEL);
        jr_debug_bit(pr, "ts_rx", value & VTSS_F_VCAP_IS2_BASETYPE_ACTION_A_OAM_RX_TIMESTAMP_ENA);
        jr_debug_fld(pr, "ts_tx", VTSS_X_VCAP_IS2_BASETYPE_ACTION_A_OAM_TX_TIMESTAMP_SEL(value));
        pr("\n");
        JR_RD(VCAP_IS2, BASETYPE_ACTION_B, &value);
        jr_debug_bit(pr, "redir", value & VTSS_F_VCAP_IS2_BASETYPE_ACTION_B_REDIR_ENA);
        jr_debug_fld(pr, "redir_pgid", VTSS_X_VCAP_IS2_BASETYPE_ACTION_B_REDIR_PGID(value));
        JR_RD(VCAP_IS2, BASETYPE_ACTION_C, &value);
        jr_debug_fld(pr, "port_mask", value);
        JR_RD(VCAP_IS2, BASETYPE_ACTION_D, &value);
        jr_debug_fld(pr, "cnt", value);
        
        return VTSS_RC_OK;
    }
    
    memset(&regs, 0, sizeof(regs));

    /* The entry type mask can not be read, so we let it be zero, never indicating don't care */
    regs.entry_type.valid = 1;
    JR_RDF(VCAP_IS2, IS2_CONTROL_ACE_STATUS, ACE_ENTRY_TYPE, &regs.entry_type.value);
    
    switch (regs.entry_type.value) {
    case IS2_ENTRY_TYPE_ETYPE:
        JR_IS2_RD_L2(MAC_ETYPE, regs);
        VTSS_RC(jr_debug_is2_regs(pr, "etype", &regs));
        JR_IS2_RD(MAC_ETYPE, ACE_L2_ETYPE, misc);
        JR_IS2_DEBUG_FLD(pr, "l2_payload", MAC_ETYPE, ACE_L2_ETYPE, L2_PAYLOAD, &misc);
        JR_IS2_DEBUG_FLD(pr, "etype", MAC_ETYPE, ACE_L2_ETYPE, ETYPE, &misc);
        pr("\n");
        break;
    case IS2_ENTRY_TYPE_LLC:
        JR_IS2_RD_L2(MAC_LLC, regs);
        VTSS_RC(jr_debug_is2_regs(pr, "llc", &regs));
        JR_IS2_RD(MAC_LLC, ACE_L2_LLC, misc);
        jr_debug_vcap_reg(pr, "llc", &misc, 0);
        break;
    case IS2_ENTRY_TYPE_SNAP:
        JR_IS2_RD_L2(MAC_SNAP, regs);
        VTSS_RC(jr_debug_is2_regs(pr, "snap", &regs));
        JR_IS2_RD_MAC(MAC_SNAP, ACE_L2_SNAP, mac);
        jr_debug_vcap_reg(pr, "snap", &mac.mach, 0xff);
        jr_debug_vcap_reg(pr, NULL, &mac.macl, 0);
        break;
    case IS2_ENTRY_TYPE_ARP:
        JR_IS2_RD_COMMON(ARP, regs);
        JR_IS2_RD_MAC(ARP, ACE_L2_SMAC, regs.smac);
        JR_IS2_RD(ARP, ACE_L3_IP4_SIP, regs.sip);
        JR_IS2_RD(ARP, ACE_L3_IP4_DIP, regs.dip);
        VTSS_RC(jr_debug_is2_regs(pr, "arp", &regs));
        JR_IS2_RD(ARP, ACE_L3_MISC, misc);
        JR_IS2_DEBUG_BIT(pr, "dip_eq_sip", ARP, ACE_L3_MISC, DIP_EQ_SIP, &misc);
        JR_IS2_RD(ARP, ACE_L3_ARP, misc);
        JR_IS2_DEBUG_FLD(pr, "opcode", ARP, ACE_L3_ARP, ARP_OPCODE, &misc);
        JR_IS2_DEBUG_BIT(pr, "op_unkn", ARP, ACE_L3_ARP, ARP_OPCODE_UNKNOWN, &misc);
        JR_IS2_DEBUG_BIT(pr, "sender_match", ARP, ACE_L3_ARP, ARP_SENDER_MATCH, &misc);
        JR_IS2_DEBUG_BIT(pr, "target_match", ARP, ACE_L3_ARP, ARP_TARGET_MATCH, &misc);
        pr("\n");
        JR_IS2_DEBUG_BIT(pr, "len_ok", ARP, ACE_L3_ARP, ARP_LEN_OK, &misc);
        JR_IS2_DEBUG_BIT(pr, "proto_ok", ARP, ACE_L3_ARP, ARP_PROTO_SPACE_OK, &misc);
        JR_IS2_DEBUG_BIT(pr, "addr_ok", ARP, ACE_L3_ARP, ARP_ADDR_SPACE_OK, &misc);
        pr("\n");
        break;
    case IS2_ENTRY_TYPE_UDP_TCP:
        JR_IS2_RD_IP(IP_TCP_UDP, regs);
        VTSS_RC(jr_debug_is2_regs(pr, "udp_tcp", &regs));
        JR_IS2_RD(IP_TCP_UDP, ACE_L4_PORT, misc);
        JR_IS2_DEBUG_FLD(pr, "sport", IP_TCP_UDP, ACE_L4_PORT, L4_SPORT, &misc);
        JR_IS2_DEBUG_FLD(pr, "dport", IP_TCP_UDP, ACE_L4_PORT, L4_DPORT, &misc);
        JR_IS2_RD(IP_TCP_UDP, ACE_L4_MISC, misc);
        JR_IS2_DEBUG_FLD(pr, "range", IP_TCP_UDP, ACE_L4_MISC, L4_RNG, &misc);
        pr("\n");
        JR_IS2_DEBUG_BIT(pr, "urg", IP_TCP_UDP, ACE_L4_MISC, L4_URG, &misc);
        JR_IS2_DEBUG_BIT(pr, "ack", IP_TCP_UDP, ACE_L4_MISC, L4_ACK, &misc);
        JR_IS2_DEBUG_BIT(pr, "psh", IP_TCP_UDP, ACE_L4_MISC, L4_PSH, &misc);
        JR_IS2_DEBUG_BIT(pr, "rst", IP_TCP_UDP, ACE_L4_MISC, L4_RST, &misc);
        JR_IS2_DEBUG_BIT(pr, "syn", IP_TCP_UDP, ACE_L4_MISC, L4_SYN, &misc);
        JR_IS2_DEBUG_BIT(pr, "fin", IP_TCP_UDP, ACE_L4_MISC, L4_FIN, &misc);
        JR_IS2_DEBUG_BIT(pr, "seq_eq0", IP_TCP_UDP, ACE_L4_MISC, SEQUENCE_EQ0, &misc);
        JR_IS2_DEBUG_BIT(pr, "sp_eq_dp", IP_TCP_UDP, ACE_L4_MISC, SPORT_EQ_DPORT, &misc);
        pr("\n");
        break;
    case IS2_ENTRY_TYPE_IP_OTHER:
        JR_IS2_RD_IP(IP_OTHER, regs);
        VTSS_RC(jr_debug_is2_regs(pr, "ip_other", &regs));
        JR_IS2_RD(IP_OTHER, ACE_IP4_OTHER_0, mac.macl);
        JR_IS2_RD(IP_OTHER, ACE_IP4_OTHER_1, mac.mach);
        jr_debug_vcap_mac(pr, "ip_payload", &mac);
        break;
    case IS2_ENTRY_TYPE_IPV6:
        JR_IS2_RD_COMMON(IP6_STD, regs);
        VTSS_RC(jr_debug_is2_regs(pr, "ipv6", &regs));
        JR_IS2_RD(IP6_STD, ACE_L3_MISC, misc);
        JR_IS2_DEBUG_FLD(pr, "proto", IP6_STD, ACE_L3_MISC, L3_IP_PROTO, &misc);
        pr("\n");
        JR_IS2_RD(IP6_STD, ACE_L3_IP6_SIP_3, misc);
        jr_debug_vcap_reg(pr, "sip_3", &misc, 0);
        JR_IS2_RD(IP6_STD, ACE_L3_IP6_SIP_2, misc);
        jr_debug_vcap_reg(pr, "sip_2", &misc, 0);
        JR_IS2_RD(IP6_STD, ACE_L3_IP6_SIP_1, misc);
        jr_debug_vcap_reg(pr, "sip_1", &misc, 0);
        JR_IS2_RD(IP6_STD, ACE_L3_IP6_SIP_0, misc);
        jr_debug_vcap_reg(pr, "sip_0", &misc, 0);
        break;
    case IS2_ENTRY_TYPE_OAM:
        JR_IS2_RD_L2(OAM, regs);
        VTSS_RC(jr_debug_is2_regs(pr, "oam", &regs));
        JR_IS2_RD(OAM, ACE_OAM_0, misc);
        JR_IS2_DEBUG_BIT(pr, "mac_in_mac", OAM, ACE_OAM_0, ISMACINMACOAM, &misc);
        JR_IS2_DEBUG_FLD(pr, "mel_flags", OAM, ACE_OAM_0, OAM_MEL_FLAGS, &misc);
        JR_IS2_DEBUG_FLD(pr, "ver", OAM, ACE_OAM_0, OAM_VER, &misc);
        JR_IS2_RD(OAM, ACE_OAM_1, misc);
        pr("\n");
        JR_IS2_DEBUG_FLD(pr, "opcode", OAM, ACE_OAM_1, OAM_OPCODE, &misc);
        JR_IS2_DEBUG_FLD(pr, "flags", OAM, ACE_OAM_1, OAM_FLAGS, &misc);
        JR_IS2_DEBUG_FLD(pr, "mepid", OAM, ACE_OAM_1, OAM_MEPID, &misc);
        pr("\n");
        break;
    case IS2_ENTRY_TYPE_CUSTOM_0:
        JR_IS2_RD_CUSTOM(0, regs);
        VTSS_RC(jr_debug_is2_regs(pr, "custom_0", &regs));
        break;
    case IS2_ENTRY_TYPE_CUSTOM_1:
        JR_IS2_RD_CUSTOM(1, regs);
        VTSS_RC(jr_debug_is2_regs(pr, "custom_1", &regs));
        break;
    case IS2_ENTRY_TYPE_CUSTOM_2:
        JR_IS2_RD_CUSTOM(2, regs);
        VTSS_RC(jr_debug_is2_regs(pr, "custom_2", &regs));
        break;
    case IS2_ENTRY_TYPE_CUSTOM_3:
        JR_IS2_RD_CUSTOM(3, regs);
        VTSS_RC(jr_debug_is2_regs(pr, "custom_3", &regs));
        break;
    case IS2_ENTRY_TYPE_CUSTOM_4:
        JR_IS2_RD_CUSTOM(4, regs);
        VTSS_RC(jr_debug_is2_regs(pr, "custom_4", &regs));
        break;
    case IS2_ENTRY_TYPE_CUSTOM_5:
        JR_IS2_RD_CUSTOM(5, regs);
        VTSS_RC(jr_debug_is2_regs(pr, "custom_5", &regs));
        break;
    case IS2_ENTRY_TYPE_CUSTOM_6:
        JR_IS2_RD_CUSTOM(6, regs);
        VTSS_RC(jr_debug_is2_regs(pr, "custom_6", &regs));
        break;
    case IS2_ENTRY_TYPE_CUSTOM_7:
        JR_IS2_RD_CUSTOM(7, regs);
        VTSS_RC(jr_debug_is2_regs(pr, "custom_7", &regs));
        break;
    default:
        return VTSS_RC_ERROR;
    }

    return VTSS_RC_OK;
}

static vtss_rc jr_debug_vcap_is2(vtss_state_t *vtss_state,
                                 const vtss_debug_printf_t pr,
                                 const vtss_debug_info_t   *const info)
{
    return jr_debug_vcap(vtss_state, VTSS_TCAM_IS2, "IS2", pr, info, jr_debug_is2);
}

static vtss_rc jr_debug_es0(vtss_state_t *vtss_state, const vtss_debug_printf_t pr, BOOL action)
{
    u32           x, y, type;
    jr_vcap_reg_t reg;
    
    if (action) {
        JR_RDB(VCAP_ES0, ES0_CONTROL_ACE_STATUS, ACE_ACTION_TYPE, &type);
        if (type == JR_ES0_ACTION_MACINMAC) {
            /* MACINMAC action */
            JR_RD(VCAP_ES0, MACINMAC_ACTION_A, &x);
            if (JR_GET_BIT(VCAP_ES0, MACINMAC_ACTION_A, VLD, x) == 0)
                return VTSS_RC_OK;

            JR_RD(VCAP_ES0, MACINMAC_ACTION_MACINMAC1, &x);
            jr_debug_fld(pr, "isid_val", VTSS_X_VCAP_ES0_MACINMAC_ACTION_MACINMAC1_ISID_VAL(x));
            JR_RD(VCAP_ES0, MACINMAC_ACTION_MACINMAC2, &x);
            jr_debug_bit(pr, "bvid_ena", 
                         x & VTSS_F_VCAP_ES0_MACINMAC_ACTION_MACINMAC2_BVID_ENA);
            jr_debug_fld(pr, "itag_bsa_idx", 
                         VTSS_X_VCAP_ES0_MACINMAC_ACTION_MACINMAC2_ITAG_BSA_IDX(x));
            jr_debug_bit(pr, "itag_keep_ena", 
                         x & VTSS_F_VCAP_ES0_MACINMAC_ACTION_MACINMAC2_ITAG_KEEP_ENA);
            pr("\n");
            jr_debug_fld(pr, "stag_tpi_sel", 
                         VTSS_X_VCAP_ES0_MACINMAC_ACTION_MACINMAC2_STAG_TPI_SEL(x));
            jr_debug_fld(pr, "stag_qos_src_sel", 
                         VTSS_X_VCAP_ES0_MACINMAC_ACTION_MACINMAC2_STAG_QOS_SRC_SEL(x));
            jr_debug_bit(pr, "stag_bundling_ena", 
                         x & VTSS_F_VCAP_ES0_MACINMAC_ACTION_MACINMAC2_STAG_BUNDLING_ENA);
            pr("\n");
            JR_RD(VCAP_ES0, MACINMAC_ACTION_B, &x);
            jr_debug_fld(pr, "vid_a", VTSS_X_VCAP_ES0_MACINMAC_ACTION_B_VID_A_VAL(x));
            jr_debug_fld(pr, "esdx", VTSS_X_VCAP_ES0_MACINMAC_ACTION_B_ESDX_VAL(x));
            jr_debug_bit(pr, "dei", x & VTSS_F_VCAP_ES0_MACINMAC_ACTION_B_DEI_VAL);
            jr_debug_fld(pr, "pcp", VTSS_X_VCAP_ES0_MACINMAC_ACTION_B_PCP_VAL(x));
            jr_debug_fld(pr, "qos_src_sel", VTSS_X_VCAP_ES0_MACINMAC_ACTION_B_QOS_SRC_SEL(x));
            JR_RD(VCAP_ES0, MACINMAC_ACTION_C, &x);
        } else {
            /* TAG action */
            JR_RD(VCAP_ES0, TAG_ACTION_A, &x);
            if (JR_GET_BIT(VCAP_ES0, TAG_ACTION_A, VLD, x) == 0)
                return VTSS_RC_OK;
            JR_RD(VCAP_ES0, TAG_ACTION_TAG1, &x);
            jr_debug_fld(pr, "vid_b", VTSS_X_VCAP_ES0_TAG_ACTION_TAG1_VID_B_VAL(x));
            jr_debug_fld(pr, "tag_vid_sel", VTSS_X_VCAP_ES0_TAG_ACTION_TAG1_TAG_VID_SEL(x));
            y = VTSS_X_VCAP_ES0_TAG_ACTION_TAG1_TAG_TPI_SEL(x);
            jr_debug_fld(pr, "tag_tpi_sel", y);
            pr("(%s)\n", y == VTSS_ES0_TPID_C ? "c" : y == VTSS_ES0_TPID_S ? "s" : 
               y == VTSS_ES0_TPID_PORT ? "port" : "c/port");
            y = VTSS_X_VCAP_ES0_TAG_ACTION_TAG1_TAG_CTRL(x);
            jr_debug_fld(pr, "tag_ctrl", y);
            pr("(%s) ", y == VTSS_ES0_TAG_NONE ? "none" : y == VTSS_ES0_TAG_ES0 ? "es0" :
               y == VTSS_ES0_TAG_PORT ? "port" : "both");
            JR_RD(VCAP_ES0, TAG_ACTION_B, &x);
            jr_debug_fld(pr, "vid_a", VTSS_X_VCAP_ES0_TAG_ACTION_B_VID_A_VAL(x));
            jr_debug_fld(pr, "esdx", VTSS_X_VCAP_ES0_TAG_ACTION_B_ESDX_VAL(x));
            jr_debug_bit(pr, "dei", x & VTSS_F_VCAP_ES0_TAG_ACTION_B_DEI_VAL);
            jr_debug_fld(pr, "pcp", VTSS_X_VCAP_ES0_TAG_ACTION_B_PCP_VAL(x));
            y = VTSS_X_VCAP_ES0_TAG_ACTION_B_QOS_SRC_SEL(x);
            jr_debug_fld(pr, "qos_src_sel", y);
            pr("(%s) ", y == VTSS_ES0_QOS_CLASS ? "class" : y == VTSS_ES0_QOS_ES0 ? "es0" :
               y == VTSS_ES0_QOS_PORT ? "port" : "mapped");
            JR_RD(VCAP_ES0, TAG_ACTION_C, &x);
        }
        jr_debug_bit(pr, "hit", x);
        return VTSS_RC_OK;
    }

    JR_RDB(VCAP_ES0, ES0_CONTROL_ACE_STATUS, ACE_ENTRY_TYPE, &type);
    if (type == JR_ES0_TYPE_ISDX) {
        /* ISDX entry */
        JR_ES0_RD(ISDX, A, reg);
        if ((reg.value & VTSS_F_VCAP_ES0_ISDX_ENTRY_A_VLD) == 0)
            return VTSS_RC_ERROR;
        pr("type: isdx ");
        JR_ES0_DEBUG_BIT(pr, "isdx_neq0", ISDX, A, ISDX_NEQ0, &reg);
        JR_ES0_DEBUG_FLD(pr, "vegr_port", ISDX, A, VEGR_PORT, &reg);
        JR_ES0_RD(ISDX, ISDX1, reg);
        JR_ES0_DEBUG_FLD(pr, "isdx", ISDX, ISDX1, ISDX, &reg);
    } else {
        /* VID entry */
        JR_ES0_RD(VID, A, reg);
        if ((reg.value & VTSS_F_VCAP_ES0_VID_ENTRY_A_VLD) == 0)
            return VTSS_RC_ERROR;
        pr("type: vid ");
        JR_ES0_DEBUG_BIT(pr, "isdx_neq0", VID, A, ISDX_NEQ0, &reg);
        JR_ES0_DEBUG_FLD(pr, "vegr_port", VID, A, VEGR_PORT, &reg);
        JR_ES0_RD(VID, VID1, reg);
        JR_ES0_DEBUG_FLD(pr, "vid", VID, VID1, VID, &reg);
    }
    pr("\n");
    
    return VTSS_RC_OK;
}

vtss_rc vtss_jr1_debug_vcap_es0(vtss_state_t *vtss_state,
                                const vtss_debug_printf_t pr,
                                const vtss_debug_info_t   *const info)
{
    return jr_debug_vcap(vtss_state, VTSS_TCAM_ES0, "ES0", pr, info, jr_debug_es0);
}

vtss_rc vtss_jr1_debug_range_checkers(vtss_state_t *vtss_state,
                                      const vtss_debug_printf_t pr,
                                      const vtss_debug_info_t   *const info)
{
    u32 i;
    
    vtss_jr1_debug_reg_header(pr, "Range Checkers");
    for (i = 0; i < VTSS_VCAP_RANGE_CHK_CNT; i++) {
        JR_DEBUG_REGX_NAME(pr, ANA_CL_2, COMMON_ADV_RNG_CTRL, i, "ADV_RNG_CTRL");
        JR_DEBUG_REGX_NAME(pr, ANA_CL_2, COMMON_ADV_RNG_VALUE_CFG, i, "ADV_RNG_VALUE");
        pr("\n");
    }
    return VTSS_RC_OK;
}

static vtss_rc jr_debug_acl(vtss_state_t *vtss_state,
                            const vtss_debug_printf_t pr,
                            const vtss_debug_info_t   *const info)
{
    u32 i;

    VTSS_RC(jr_debug_vcap_is2(vtss_state, pr, info));
    
    if (info->full) {
        VTSS_RC(vtss_jr1_debug_range_checkers(vtss_state, pr, info));

        vtss_jr1_debug_reg_header(pr, "ACL Policers");
        for (i = 0; i < 32; i++) {
            JR_DEBUG_REGX_NAME(pr, ANA_AC, POL_ALL_CFG_POL_ACL_RATE_CFG, i, "ACL_RATE_CFG");
            JR_DEBUG_REGX_NAME(pr, ANA_AC, POL_ALL_CFG_POL_ACL_THRES_CFG, i, "ACL_THRES_CFG");
            JR_DEBUG_REGX_NAME(pr, ANA_AC, POL_ALL_CFG_POL_ACL_CTRL, i, "ACL_CTRL");
            pr("\n");
        }
    }
    
    return VTSS_RC_OK;
}

vtss_rc vtss_jr1_vcap_debug_print(vtss_state_t *vtss_state,
                                  const vtss_debug_printf_t pr,
                                  const vtss_debug_info_t   *const info)
{
    return vtss_debug_print_group(VTSS_DEBUG_GROUP_ACL, jr_debug_acl, vtss_state, pr, info);
}

/* - Initialization ------------------------------------------------ */

static vtss_rc jr_init_vcap_is0(vtss_state_t *vtss_state)
{
    JR_WR(VCAP_IS0, TCAM_BIST_TCAM_CTRL, VTSS_F_VCAP_IS0_TCAM_BIST_TCAM_CTRL_TCAM_INIT);
    JR_POLL_BIT(VCAP_IS0, TCAM_BIST_TCAM_CTRL, TCAM_INIT);

    VTSS_RC(jr_vcap_command(vtss_state, VTSS_TCAM_IS0, 0, VTSS_TCAM_CMD_INITIALIZE));
    JR_WR(VCAP_IS0, IS0_CONTROL_ACL_CFG, VTSS_F_VCAP_IS0_IS0_CONTROL_ACL_CFG_ACL_ENA);

    return VTSS_RC_OK;
}

static vtss_rc jr_init_vcap_is1(vtss_state_t *vtss_state)
{
    JR_WR(VCAP_IS1, TCAM_BIST_TCAM_CTRL, VTSS_F_VCAP_IS1_TCAM_BIST_TCAM_CTRL_TCAM_INIT);
    JR_POLL_BIT(VCAP_IS1, TCAM_BIST_TCAM_CTRL, TCAM_INIT);

    VTSS_RC(jr_vcap_command(vtss_state, VTSS_TCAM_IS1, 0, VTSS_TCAM_CMD_INITIALIZE));
    JR_WR(VCAP_IS1, IS1_CONTROL_ACL_CFG, VTSS_F_VCAP_IS1_IS1_CONTROL_ACL_CFG_ACL_ENA);

    return VTSS_RC_OK;
}

static vtss_rc jr_init_vcap_is2(vtss_state_t *vtss_state)
{
    JR_WR(VCAP_IS2, TCAM_BIST_TCAM_CTRL, VTSS_F_VCAP_IS2_TCAM_BIST_TCAM_CTRL_TCAM_INIT);
    JR_POLL_BIT(VCAP_IS2, TCAM_BIST_TCAM_CTRL, TCAM_INIT);

    VTSS_RC(jr_vcap_command(vtss_state, VTSS_TCAM_IS2, 0, VTSS_TCAM_CMD_INITIALIZE));
    JR_WR(VCAP_IS2, IS2_CONTROL_ACL_CFG, VTSS_F_VCAP_IS2_IS2_CONTROL_ACL_CFG_ACL_ENA);

    return VTSS_RC_OK;
}

static vtss_rc jr_init_vcap_es0(vtss_state_t *vtss_state)
{
    JR_WR(VCAP_ES0, TCAM_BIST_TCAM_CTRL, VTSS_F_VCAP_ES0_TCAM_BIST_TCAM_CTRL_TCAM_INIT);
    JR_POLL_BIT(VCAP_ES0, TCAM_BIST_TCAM_CTRL, TCAM_INIT);

    VTSS_RC(jr_vcap_command(vtss_state, VTSS_TCAM_ES0, 0, VTSS_TCAM_CMD_INITIALIZE));
    JR_WR(VCAP_ES0, ES0_CONTROL_ACL_CFG, VTSS_F_VCAP_ES0_ES0_CONTROL_ACL_CFG_ACL_ENA);

    return VTSS_RC_OK;
}

vtss_rc vtss_jr1_vcap_init(vtss_state_t *vtss_state, vtss_init_cmd_t cmd)
{
    vtss_vcap_state_t *state = &vtss_state->vcap;
    vtss_vcap_obj_t   *is0 = &state->is0.obj;
    vtss_vcap_obj_t   *is1 = &state->is1.obj;
    vtss_vcap_obj_t   *is2 = &state->is2.obj;
    vtss_vcap_obj_t   *es0 = &state->es0.obj;

    switch (cmd) {
    case VTSS_INIT_CMD_CREATE:
        /* IS0 */
        is0->max_count = VTSS_JR1_IS0_CNT;
        is0->entry_get = jr_is0_entry_get;
        is0->entry_add = jr_is0_entry_add;
        is0->entry_del = jr_is0_entry_del;
        is0->entry_move = jr_is0_entry_move;
        
        /* IS1 */
        is1->max_count = VTSS_JR1_IS1_CNT;
        is1->entry_get = jr_is1_entry_get;
        is1->entry_add = jr_is1_entry_add;
        is1->entry_del = jr_is1_entry_del;
        is1->entry_move = jr_is1_entry_move;
        
        /* IS2 */
        is2->max_count = VTSS_JR1_IS2_CNT;
        is2->entry_get = jr_is2_entry_get;
        is2->entry_add = jr_is2_entry_add;
        is2->entry_del = jr_is2_entry_del;
        is2->entry_move = jr_is2_entry_move;
        
        /* ES0 */
        es0->max_count = VTSS_JR1_ES0_CNT;
        es0->entry_get = jr_es0_entry_get;
        es0->entry_add = jr_es0_entry_add;
        es0->entry_del = jr_es0_entry_del;
        es0->entry_move = jr_es0_entry_move;
        state->es0_entry_update = jr_es0_entry_update;
        
        /* ACL */
        state->acl_policer_set = jr_acl_policer_set;
        state->acl_port_set = jr_acl_port_conf_set;
        state->acl_port_counter_get = jr_acl_port_counter_get;
        state->acl_port_counter_clear = jr_acl_port_counter_clear;
        state->acl_ace_add = jr_ace_add;
        state->acl_ace_del = jr_ace_del;
        state->acl_ace_counter_get = jr_ace_counter_get;
        state->acl_ace_counter_clear = jr_ace_counter_clear;
        state->range_commit = jr_vcap_range_commit;
        break;
    case VTSS_INIT_CMD_INIT:
        VTSS_RC(jr_init_vcap_is0(vtss_state));
        VTSS_RC(jr_init_vcap_is1(vtss_state));
        VTSS_RC(jr_init_vcap_is2(vtss_state));
        VTSS_RC(jr_init_vcap_es0(vtss_state));
        
        /* Enable ES0 lookup (SERVICE_LU_ENA) */
        JR_WR(REW, COMMON_SERVICE_CTRL, 1<<0);
        
        /* Setup IS2 discard policer */
        JR_WRX(ANA_AC, POL_ALL_CFG_POL_ACL_CTRL, JR_IS2_POLICER_DISCARD,
               JR_PUT_FLD(ANA_AC, POL_ALL_CFG_POL_ACL_CTRL, ACL_TRAFFIC_TYPE_MASK, 2));
        break;
    default:
        break;
    }
    return VTSS_RC_OK;
}

#endif /* VTSS_ARCH_JAGUAR_1 */
