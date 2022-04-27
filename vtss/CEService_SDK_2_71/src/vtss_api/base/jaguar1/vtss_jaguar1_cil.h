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

#ifndef _VTSS_JAGUAR1_CIL_H_
#define _VTSS_JAGUAR1_CIL_H_

/* Use relative addresses for registers - must be first */
#define VTSS_IOADDR(t,o) ((((t) - VTSS_TO_DEVCPU_ORG) >> 2) + (o))
#define VTSS_IOREG(t,o)  (VTSS_IOADDR(t,o))

// Avoid "vtss_jaguar1_cil.h not used in module vtss_jaguar1.c"
/*lint --e{766} */

#include "vtss_api.h"

#if defined(VTSS_ARCH_JAGUAR_1)
#define VTSS_TRACE_LAYER VTSS_TRACE_LAYER_CIL
#include "../ail/vtss_state.h"
#include "../ail/vtss_common.h"
#include "../ail/vtss_util.h"
#if defined(VTSS_FEATURE_FDMA) && VTSS_OPT_FDMA
#include "../ail/vtss_vcoreiii_fdma.h"
#endif
#include "vtss_jaguar1.h"
#include "vtss_jaguar_reg.h"

/* Chip ports */
#define VTSS_CHIP_PORTS      32
#define VTSS_CHIP_PORT_CPU_0 33
#define VTSS_CHIP_PORT_CPU_1 34
#define VTSS_CHIP_PORTS_ALL  35 /* Includes CPU ports */

#define VTSS_PORT_10G_START  27
#define VTSS_PORT_NPI        31
#define JR_STACK_PORT_START  24 /* Port 24-30 are potential stack ports */
#define JR_STACK_PORT_CNT     7 /* Port 24-30 are potential stack ports */
#define JR_STACK_PORT_END    (JR_STACK_PORT_START + JR_STACK_PORT_CNT)

#define VTSS_PORT_IS_1G(port)             ((port) < VTSS_PORT_10G_START || (port) == VTSS_PORT_NPI)
#define VTSS_PORT_IS_10G(port)            ((port) >= VTSS_PORT_10G_START && (port) < VTSS_PORT_NPI)
#define VTSS_PORT_IS_STACKABLE(chip_port) ((chip_port) >= JR_STACK_PORT_START && (chip_port) < VTSS_PORT_NPI)

#define VTSS_TO_DEV(port) (port == VTSS_PORT_NPI ? VTSS_TO_DEVRGMII_0 : VTSS_PORT_IS_10G(port) ? \
                           (VTSS_TO_DEV10G_0 + (port-27) * (VTSS_TO_DEV10G_1 - VTSS_TO_DEV10G_0)) :\
                           (VTSS_TO_DEV1G_0 + (port) * (VTSS_TO_DEV1G_1 - VTSS_TO_DEV1G_0)))


/* MC PGIDs */
#define PGID_UC_FLOOD     32
#define PGID_MC_FLOOD     33
#define PGID_IPV4_MC_DATA 34
#define PGID_IPV4_MC_CTRL 35
#define PGID_IPV6_MC_DATA 36
#define PGID_IPV6_MC_CTRL 37
#define PGID_DROP         38
#define PGID_FLOOD        39

/* GLAG PGID */
#define PGID_GLAG_START   800

/* Host Queue */
#define PGID_HQ_START   1024+32

/* Total number of SQS Cells */
#define JR_TOTAL_NUM_CELLS 26240

#define JR_PRIOS 8   /* Number of priorities */

#define JR_GPIOS        24
#define JR_SGPIO_GROUPS 2
#define JR_ACS          16  /* Number of aggregation masks */

#define JR_MIRROR_PROBE_RX  0 /* Ingress mirror probe */
#define JR_MIRROR_PROBE_TX  1 /* Egress mirror probe */
#define JR_MIRROR_PROBE_EEE 2 /* EEE mirror probe */

/* Counter allocations, 4 RX and 2 TX */
/* RX */
#define JR_CNT_RX_GREEN     0
#define JR_CNT_RX_YELLOW    1
#define JR_CNT_RX_QDROPS    2  /* RX Queue drop  */
#define JR_CNT_RX_PDROPS    3  /* Policer drop   */
/* TX */
#define JR_CNT_TX_DROPS     0  /* TX Queue drop  */
#define JR_CNT_TX_GREEN     0
#define JR_CNT_TX_YELLOW    1

#define JR_CNT_ANA_AC_FILTER 0

/* Packet injection and extraction */
#define JR_PACKET_RX_QUEUE_CNT 10
#define JR_PACKET_RX_GRP_CNT   4
#define JR_PACKET_TX_GRP_CNT   5

/* Reserved EVC policers */
#define JR_EVC_POLICER_NONE     0 /* Policer 0 must always be open, so we use this as "None" */
#define JR_EVC_POLICER_DISCARD  1 /* Policer 1 used to discard frames */
#define JR_EVC_POLICER_RESV_CNT 2 /* Number of reserved policers */

/* CE-MaX host interfaces */
#if defined(VTSS_CHIP_CE_MAX_24)
#define VTSS_CE_MAC_HMDA 29
#define VTSS_CE_MAC_HMDB 30
#elif defined(VTSS_CHIP_CE_MAX_12)
#define VTSS_CE_MAC_HMDA 28
#define VTSS_CE_MAC_HMDB 28
#endif /* VTSS_CHIP_CE_MAX_24 */

/* sFlow H/W-related min/max */
#define JR_SFLOW_MIN_SAMPLE_RATE       1 /**< Minimum allowable sampling rate for sFlow */
#define JR_SFLOW_MAX_SAMPLE_RATE   32767 /**< Maximum allowable sampling rate for sFlow */

/* ================================================================= *
 *  Register access
 * ================================================================= */

vtss_rc vtss_jr1_rd(const vtss_state_t *vtss_state, u32 addr, u32 *value);
vtss_rc vtss_jr1_wr(const vtss_state_t *vtss_state, u32 addr, u32 value);
vtss_rc vtss_jr1_wrm(const vtss_state_t *vtss_state, u32 addr, u32 value, u32 mask);

vtss_rc vtss_jr1_wr_chip(const vtss_state_t *vtss_state,
                         vtss_chip_no_t chip_no, u32 addr, u32 value);
vtss_rc vtss_jr1_rd_chip(const vtss_state_t *vtss_state,
                         vtss_chip_no_t chip_no, u32 addr, u32 *value);
vtss_rc vtss_jr1_wrm_chip(const vtss_state_t *vtss_state,
                          vtss_chip_no_t chip_no, u32 addr, u32 value, u32 mask);

/* Concatenations */
#define JR_ADDR(tgt, addr)                 VTSS_##tgt##_##addr
#define JR_ADDR_X(tgt, addr, x)            VTSS_##tgt##_##addr(x)
#define JR_ADDR_XY(tgt, addr, x, y)        VTSS_##tgt##_##addr(x,y)
#define JR_GET_FLD(tgt, addr, fld, value)  VTSS_X_##tgt##_##addr##_##fld(value)
#define JR_PUT_FLD(tgt, addr, fld, value)  VTSS_F_##tgt##_##addr##_##fld(value)
#define JR_GET_BIT(tgt, addr, fld, value)  (VTSS_F_##tgt##_##addr##_##fld & (value) ? 1 : 0)
#define JR_PUT_BIT(tgt, addr, fld, value)  ((value) ? VTSS_F_##tgt##_##addr##_##fld : 0)
#define JR_SET_BIT(tgt, addr, fld)         VTSS_F_##tgt##_##addr##_##fld
#define JR_MSK(tgt, addr, fld)             VTSS_M_##tgt##_##addr##_##fld
#define JR_CNT(xmit, id)                   JR_CNT_##xmit##_##id

/* - Helpers (not used directly) ----------------------------------- */

/* Read register */
#define JR_RD_REG(reg, value)               VTSS_RC(vtss_jr1_rd(vtss_state, reg, value));
#define JR_RD_REG_CHIP(chip_no, reg, value) VTSS_RC(vtss_jr1_rd_chip(vtss_state, chip_no, reg, value));

/* Write register */
#define JR_WR_REG(reg, value)               VTSS_RC(vtss_jr1_wr(vtss_state, reg, value));
#define JR_WR_REG_CHIP(chip_no, reg, value) VTSS_RC(vtss_jr1_wr_chip(vtss_state, chip_no, reg, value));

/* Write register masked */
#define JR_WR_MSK(reg, value, mask)                VTSS_RC(vtss_jr1_wrm(vtss_state, reg, value, mask));
#define JR_WR_MSK_CHIP(chip_no, reg, value, mask)  VTSS_RC(vtss_jr1_wrm_chip(vtss_state, chip_no, reg, value, mask));

/* Read and extract register field */
#define JR_RD_FLD(reg, tgt, addr, fld, value)     \
{                                                 \
    u32 __x__;                                    \
    JR_RD_REG(reg, &__x__);                       \
    *(value) = JR_GET_FLD(tgt, addr, fld, __x__); \
}

/* Read and extract register field */
#define JR_RD_FLD_CHIP(chip_no, reg, tgt, addr, fld, value) \
{                                                           \
    u32 __x__;                                              \
    JR_RD_REG_CHIP(chip_no, reg, &__x__);                   \
    *(value) = JR_GET_FLD(tgt, addr, fld, __x__);           \
}

/* Write register field */
#define JR_WR_FLD(reg, tgt, addr, fld, value) \
    JR_WR_MSK(reg, JR_PUT_FLD(tgt, addr, fld, value), JR_MSK(tgt, addr, fld))
#define JR_WR_FLD_CHIP(chip_no, reg, tgt, addr, fld, value) \
    JR_WR_MSK_CHIP(chip_no, reg, JR_PUT_FLD(tgt, addr, fld, value), JR_MSK(tgt, addr, fld))

/* Read and extract register bit field */
#define JR_RD_BIT(reg, tgt, addr, fld, value)     \
{                                                 \
    u32 __x__;                                    \
    JR_RD_REG(reg, &__x__);                       \
    *(value) = JR_GET_BIT(tgt, addr, fld, __x__); \
}

/* Write register bit field */
#define JR_WR_BIT(reg, tgt, addr, fld, value) \
    JR_WR_MSK(reg, JR_PUT_BIT(tgt, addr, fld, value), JR_PUT_BIT(tgt, addr, fld, 1))
#define JR_WR_BIT_CHIP(chip_no, reg, tgt, addr, fld, value) \
    JR_WR_MSK_CHIP(chip_no, reg, JR_PUT_BIT(tgt, addr, fld, value), JR_PUT_BIT(tgt, addr, fld, 1))


/* - No replication ------------------------------------------------ */

/* Read register */
#define JR_RD(tgt, addr, value) JR_RD_REG(JR_ADDR(tgt, addr), value)

/* Write register */
#define JR_WR(tgt, addr, value)               JR_WR_REG(JR_ADDR(tgt, addr), value);
#define JR_WR_CHIP(chip_no, tgt, addr, value) JR_WR_REG_CHIP(chip_no, JR_ADDR(tgt, addr), value);

/* Read register field */
#define JR_RDF(tgt, addr, fld, value)               JR_RD_FLD(JR_ADDR(tgt, addr), tgt, addr, fld, value)
#define JR_RDF_CHIP(chip_no, tgt, addr, fld, value) JR_RD_FLD_CHIP(chip_no, JR_ADDR(tgt, addr), tgt, addr, fld, value)

/* Write register field */
#define JR_WRF(tgt, addr, fld, value) JR_WR_FLD(JR_ADDR(tgt, addr), tgt, addr, fld, value)

/* Read register bit field */
#define JR_RDB(tgt, addr, fld, value) JR_RD_BIT(JR_ADDR(tgt, addr), tgt, addr, fld, value)

/* Write register bit field */
#define JR_WRB(tgt, addr, fld, value)               JR_WR_BIT(JR_ADDR(tgt, addr), tgt, addr, fld, value)
#define JR_WRB_CHIP(chip_no, tgt, addr, fld, value) JR_WR_BIT_CHIP(chip_no, JR_ADDR(tgt, addr), tgt, addr, fld, value)


/* Write register masked and set/clear */
#define JR_WRM(tgt, addr, value, mask) JR_WR_MSK(JR_ADDR(tgt, addr), value, mask)
#define JR_WRM_SET(tgt, addr, mask)    JR_WRM(tgt, addr, mask, mask)
#define JR_WRM_CLR(tgt, addr, mask)    JR_WRM(tgt, addr, 0,    mask)

/* - Single replication (X) ---------------------------------------- */

/* Read register */
#define JR_RDX(tgt, addr, x, value) \
    JR_RD_REG(JR_ADDR_X(tgt, addr, x), value)

#define JR_RDX_CHIP(chip_no, tgt, addr, x, value) \
    JR_RD_REG_CHIP(chip_no, JR_ADDR_X(tgt, addr, x), value)

/* Write register */
#define JR_WRX(tgt, addr, x, value) \
    JR_WR_REG(JR_ADDR_X(tgt, addr, x), value)
#define JR_WRX_CHIP(chip_no, tgt, addr, x, value) \
    JR_WR_REG_CHIP(chip_no, JR_ADDR_X(tgt, addr, x), value)

/* Read register field */
#define JR_RDXF(tgt, addr, x, fld, value) \
    JR_RD_FLD(JR_ADDR_X(tgt, addr, x), tgt, addr, fld, value)
#define JR_RDXF_CHIP(chip_no, tgt, addr, x, fld, value) \
    JR_RD_FLD_CHIP(chip_no, JR_ADDR_X(tgt, addr, x), tgt, addr, fld, value)

/* Write register field */
#define JR_WRXF(tgt, addr, x, fld, value) \
    JR_WR_FLD(JR_ADDR_X(tgt, addr, x), tgt, addr, fld, value)
#define JR_WRXF_CHIP(chip_no, tgt, addr, x, fld, value) \
    JR_WR_FLD_CHIP(chip_no, JR_ADDR_X(tgt, addr, x), tgt, addr, fld, value)

/* Read register bit field */
#define JR_RDXB(tgt, addr, x, fld, value) \
    JR_RD_BIT(JR_ADDR_X(tgt, addr, x), tgt, addr, fld, value)

/* Write register bit field */
#define JR_WRXB(tgt, addr, x, fld, value) \
    JR_WR_BIT(JR_ADDR_X(tgt, addr, x), tgt, addr, fld, value)

#define JR_WRXB_CHIP(chip_no, tgt, addr, x, fld, value) \
    JR_WR_BIT_CHIP(chip_no, JR_ADDR_X(tgt, addr, x), tgt, addr, fld, value)

/* Write register masked and set/clear */
#define JR_WRXM(tgt, addr, x, value, mask) JR_WR_MSK(JR_ADDR_X(tgt, addr, x), value, mask)
#define JR_WRXM_SET(tgt, addr, x, mask)    JR_WRXM(tgt, addr, x, mask, mask)
#define JR_WRXM_CLR(tgt, addr, x, mask)    JR_WRXM(tgt, addr, x, 0,    mask)

/* - Double replication (X, Y) ------------------------------------- */

/* Read register */
#define JR_RDXY(tgt, addr, x, y, value) \
    JR_RD_REG(JR_ADDR_XY(tgt, addr, x, y), value)

/* Write register */
#define JR_WRXY(tgt, addr, x, y, value) \
    JR_WR_REG(JR_ADDR_XY(tgt, addr, x, y), value)

/* Read register field */
#define JR_RDXYF(tgt, addr, x, y, fld, value) \
    JR_RD_FLD(JR_ADDR_XY(tgt, addr, x, y), tgt, addr, fld, value)

/* Write register field */
#define JR_WRXYF(tgt, addr, x, y, fld, value) \
    JR_WR_FLD(JR_ADDR_XY(tgt, addr, x, y), tgt, addr, fld, value)

/* Read register bit field */
#define JR_RDXYB(tgt, addr, x, y, fld, value) \
    JR_RD_BIT(JR_ADDR_XY(tgt, addr, x, y), tgt, addr, fld, value)

/* Write register bit field */
#define JR_WRXYB(tgt, addr, x, y, fld, value) \
    JR_WR_BIT(JR_ADDR_XY(tgt, addr, x, y), tgt, addr, fld, value)

/* Write register masked and set/clear */
#define JR_WRXYM(tgt, addr, x, y, value, mask) \
    JR_WR_MSK(JR_ADDR_XY(tgt, addr, x, y), value, mask)
#define JR_WRXYM_SET(tgt, addr, x, y, mask) JR_WRXM(tgt, addr, x, y, mask, mask)
#define JR_WRXYM_CLR(tgt, addr, x, y, mask) JR_WRXM(tgt, addr, x, y, 0,    mask)

/* Poll bit until zero */
#define JR_POLL_BIT(tgt, addr, fld)                                \
{                                                                  \
    u32 _x_, count = 0;                                            \
    do {                                                           \
        JR_RDB(tgt, addr, fld, &_x_);                              \
        VTSS_MSLEEP(1);                                            \
        count++;                                                   \
        if (count == 100) {                                        \
            VTSS_E("timeout, target address: %s::%s", #tgt, #addr); \
            return VTSS_RC_ERROR;                                  \
        }                                                          \
    } while (_x_);                                                 \
}

/* ----------------------------------------------------------------- */

#define WRX_SQS(qs, addr, x, value)                     \
{                                                      \
    vtss_rc rc = vtss_jr1_wr(vtss_state, (qs ? VTSS_OQS_##addr(x) : VTSS_IQS_##addr(x)), value); \
    if (rc != VTSS_RC_OK)                              \
        return rc;                                     \
}

#define WR_SQS(qs, addr, value)                     \
{                                                      \
    vtss_rc rc = vtss_jr1_wr(vtss_state, (qs ? VTSS_OQS_##addr : VTSS_IQS_##addr), value); \
    if (rc != VTSS_RC_OK)                              \
        return rc;                                     \
}

#define RDX_SQS(qs, addr, x, value)                \
{                                                      \
    vtss_rc rc = vtss_jr1_rd(vtss_state, (qs ? VTSS_OQS_##addr(x) : VTSS_IQS_##addr(x)), value); \
    if (rc != VTSS_RC_OK)                              \
        return rc;                                     \
}
#define RD_SQS(qs, addr, value)                \
{                                                      \
    vtss_rc rc = vtss_jr1_rd(vtss_state, (qs ? VTSS_OQS_##addr : VTSS_IQS_##addr), value); \
    if (rc != VTSS_RC_OK)                              \
        return rc;                                     \
}

#define JR_CNT_SQS(tgt, xmit, cnt_id, port, prio, cnt, clr)                            \
{                                                                                      \
    u32 qu, value;                                                                     \
    qu = (port * 8 + prio);                                                            \
    JR_RDXY(tgt, STAT_CNT_CFG_##xmit##_STAT_LSB_CNT, qu, JR_CNT(xmit, cnt_id), &value); \
    vtss_cmn_counter_32_update(value, cnt, clr);                                       \
}                                                                                      \

#define PST_SQS(front,qs,back) (qs ? front##_OQS_##back : front##_IQS_##back)

#define JR_DEBUG_REG(pr, tgt, addr) \
    vtss_jr1_debug_reg(vtss_state, pr, JR_ADDR(tgt, addr), #addr)
#define JR_DEBUG_REG_NAME(pr, tgt, addr, name) \
    vtss_jr1_debug_reg(vtss_state, pr, JR_ADDR(tgt, addr), name)
#define JR_DEBUG_REGX(pr, tgt, addr, x) \
    vtss_jr1_debug_reg_inst(vtss_state, pr, JR_ADDR_X(tgt, addr, x), x, #addr)
#define JR_DEBUG_REGX_NAME(pr, tgt, addr, x, name) \
    vtss_jr1_debug_reg_inst(vtss_state, pr, JR_ADDR_X(tgt, addr, x), x, name)
#define JR_DEBUG_REGXY_NAME(pr, tgt, addr, x, y, name) \
    vtss_jr1_debug_reg_inst(vtss_state, pr, JR_ADDR_XY(tgt, addr, x, y), x, name)
    
/* ----------------------------------------------------------------- */

enum vtss_tcam_cmd {
    VTSS_TCAM_CMD_WRITE       = 0, /* Copy from Cache to TCAM */
    VTSS_TCAM_CMD_READ        = 1, /* Copy from TCAM to Cache */
    VTSS_TCAM_CMD_MOVE_UP     = 2, /* Move <count> up */
    VTSS_TCAM_CMD_MOVE_DOWN   = 3, /* Move <count> down */
    VTSS_TCAM_CMD_INITIALIZE  = 4, /* Init entries and normal actions. Reset default actions */
};

enum vtss_tcam_bank {
    VTSS_TCAM_IS0, /* IS0 */
    VTSS_TCAM_IS1, /* IS1 */
    VTSS_TCAM_IS2, /* IS2 */
    VTSS_TCAM_ES0  /* ES0 */
};

/* VCAP bit */
#define JR_VCAP_BIT(val) ((val) ? VTSS_VCAP_BIT_1 : VTSS_VCAP_BIT_0)

/* ----------------------------------------------------------------- */

/* Common functions */
u32 vtss_jr1_port_prio_2_qu(vtss_state_t *vtss_state, u32 port, u32 prio, u32 qs);
vtss_rc vtss_jr1_gpio_mode(vtss_state_t           *vtss_state,
                           const vtss_chip_no_t   chip_no,
                           const vtss_gpio_no_t   gpio_no,
                           const vtss_gpio_mode_t mode);
vtss_rc vtss_jr1_conf_chips(vtss_state_t *vtss_state, vtss_rc (*func_conf)(vtss_state_t *));
u32 vtss_jr1_port_mask_from_map(vtss_state_t *vtss_state,
                                BOOL include_internal_ports, BOOL include_stack_ports);
u32 vtss_jr1_vtss_pgid(const vtss_state_t *const state, u32 pgid);
u32 vtss_jr1_chip_pgid(vtss_state_t *vtss_state, u32 pgid);
u32 vtss_jr1_port_mask(vtss_state_t *vtss_state, const BOOL member[]);
vtss_rc vtss_jr1_init_groups(vtss_state_t *vtss_state, vtss_init_cmd_t cmd);

BOOL vtss_jr1_debug_port_skip(vtss_state_t *vtss_state,
                              const vtss_port_no_t port_no,
                              const vtss_debug_info_t   *const info);
void vtss_jr1_debug_reg_header(const vtss_debug_printf_t pr, const char *name);
void vtss_jr1_debug_print_port_header(vtss_state_t *vtss_state,
                                      const vtss_debug_printf_t pr, const char *txt);
void vtss_jr1_debug_print_mask(const vtss_debug_printf_t pr, u32 mask);
void vtss_jr1_debug_reg(vtss_state_t *vtss_state,
                        const vtss_debug_printf_t pr, u32 addr, const char *name);
void vtss_jr1_debug_reg_inst(vtss_state_t *vtss_state,
                             const vtss_debug_printf_t pr, u32 addr, u32 i, const char *name);
void vtss_jr1_debug_cnt(vtss_state_t *vtss_state,
                        const vtss_debug_printf_t pr, const char *col1, const char *col2, 
                        vtss_chip_counter_t *c1, vtss_chip_counter_t *c2);
void vtss_jr1_debug_fld_nl(const vtss_debug_printf_t pr, const char *name, u32 value);
void vtss_jr1_debug_sticky(vtss_state_t *vtss_state,
                           const vtss_debug_printf_t pr, u32 addr, const char *name);
void vtss_jr1_debug_sticky_inst(vtss_state_t *vtss_state,
                                const vtss_debug_printf_t pr, u32 addr, u32 i, 
                                const char *name);

/* Port functions */
vtss_rc vtss_jr1_port_init(vtss_state_t *vtss_state, vtss_init_cmd_t cmd);
vtss_rc vtss_jr1_port_map_set(vtss_state_t *vtss_state);
vtss_rc vtss_jr1_port_setup(vtss_state_t *vtss_state,
                            vtss_port_no_t port_no, u32 chipport, vtss_port_conf_t *conf, BOOL front_port);
BOOL vtss_jr1_cemax_port_is_host(vtss_state_t *vtss_state, vtss_port_no_t port_no);
vtss_rc vtss_jr1_setup_mtu(vtss_state_t *vtss_state, u32 chipport, u32 mtu, BOOL front_port);
vtss_rc vtss_jr1_port_max_tags_set(vtss_state_t *vtss_state, vtss_port_no_t port_no);
u32 vtss_jr1_ce_max_hm(vtss_state_t *vtss_state);
vtss_rc vtss_jr1_port_debug_print(vtss_state_t *vtss_state,
                                  const vtss_debug_printf_t pr,
                                  const vtss_debug_info_t   *const info);
/* QoS functions */
vtss_rc vtss_jr1_qos_init(vtss_state_t *vtss_state, vtss_init_cmd_t cmd);
vtss_rc vtss_jr1_qos_port_conf_set(vtss_state_t *vtss_state, const vtss_port_no_t port_no);
vtss_rc vtss_jr1_port_policer_fc_set(vtss_state_t *vtss_state, const vtss_port_no_t port_no, u32 chipport);
vtss_rc vtss_jr1_qos_wred_conf_set(vtss_state_t *vtss_state, const vtss_port_no_t port_no);
vtss_rc vtss_jr1_cpu_pol_setup_chip(vtss_state_t *vtss_state);
vtss_rc vtss_jr1_qos_debug_print(vtss_state_t *vtss_state,
                                 const vtss_debug_printf_t pr,
                                 const vtss_debug_info_t   *const info);

/* L2 functions */
vtss_rc vtss_jr1_l2_init(vtss_state_t *vtss_state, vtss_init_cmd_t cmd);
vtss_rc vtss_jr1_vstax_conf_set_chip(vtss_state_t *vtss_state);
vtss_rc vtss_jr1_vstax_update_ingr_shaper(vtss_state_t *vtss_state,
                                          u32 chipport, vtss_port_speed_t speed, BOOL front_port);
u32 vtss_jr1_unused_chip_port(vtss_state_t *vtss_state);
vtss_rc vtss_jr1_vlan_port_conf_apply(vtss_state_t *vtss_state,
                                      u32 port, vtss_vlan_port_conf_t *conf, vtss_qos_port_conf_t *qp_conf);
BOOL vtss_jr1_is_frontport(vtss_state_t *vtss_state, vtss_port_no_t port_no);
vtss_rc vtss_jr1_src_table_write_chip(vtss_state_t *vtss_state, u32 port, u32 mask);
vtss_rc vtss_jr1_pgid_table_write_chip(vtss_state_t *vtss_state,
                                       u32 pgid, u32 mask, BOOL cpu_copy, 
                                       vtss_packet_rx_queue_t cpu_queue, BOOL stack_type_ena);
vtss_rc vtss_jr1_l2_debug_print(vtss_state_t *vtss_state,
                                const vtss_debug_printf_t pr,
                                const vtss_debug_info_t   *const info);

/* Packet functions */
vtss_rc vtss_jr1_packet_init(vtss_state_t *vtss_state, vtss_init_cmd_t cmd);
vtss_rc vtss_jr1_cpu_queue_redirect_chip(vtss_state_t *vtss_state);
vtss_rc vtss_jr1_lrn_all_set_chip(vtss_state_t *vtss_state);
vtss_rc vtss_jr1_packet_debug_print(vtss_state_t *vtss_state,
                                    const vtss_debug_printf_t pr,
                                    const vtss_debug_info_t   *const info);

/* VCAP functions */
vtss_rc vtss_jr1_vcap_init(vtss_state_t *vtss_state, vtss_init_cmd_t cmd);
vtss_rc vtss_jr1_vcap_port_command(vtss_state_t *vtss_state, int bank, u32 port, int cmd);
vtss_rc vtss_jr1_igmp_cpu_copy_set(vtss_state_t *vtss_state);
vtss_rc vtss_jr1_vcap_lookup_set(vtss_state_t *vtss_state, const vtss_port_no_t port_no);
vtss_rc vtss_jr1_vcap_is1_is2_set(vtss_state_t *vtss_state, u32 port, BOOL is1, u32 is2);
vtss_rc vtss_jr1_is2_add_any(vtss_state_t     *vtss_state,
                             vtss_vcap_user_t user, 
                             vtss_vcap_id_t   id_base,
                             vtss_vcap_id_t   id_next,
                             vtss_vcap_data_t *data,
                             BOOL             etype);
vtss_rc vtss_jr1_is2_del_any(vtss_state_t *vtss_state,
                             vtss_vcap_user_t user, vtss_vcap_id_t id, BOOL etype);
vtss_rc vtss_jr1_is2_cpu_port_setup(vtss_state_t *vtss_state);

vtss_rc vtss_jr1_debug_range_checkers(vtss_state_t *vtss_state,
                                      const vtss_debug_printf_t pr,
                                      const vtss_debug_info_t   *const info);
vtss_rc vtss_jr1_debug_vcap_is0(vtss_state_t *vtss_state,
                                const vtss_debug_printf_t pr,
                                const vtss_debug_info_t   *const info);
vtss_rc vtss_jr1_debug_vcap_is1(vtss_state_t *vtss_state,
                                const vtss_debug_printf_t pr,
                                const vtss_debug_info_t   *const info);
vtss_rc vtss_jr1_debug_vcap_es0(vtss_state_t *vtss_state,
                                const vtss_debug_printf_t pr,
                                const vtss_debug_info_t   *const info);
vtss_rc vtss_jr1_vcap_debug_print(vtss_state_t *vtss_state,
                                  const vtss_debug_printf_t pr,
                                  const vtss_debug_info_t   *const info);

/* Miscellaneous functions */
vtss_rc vtss_jr1_misc_init(vtss_state_t *vtss_state, vtss_init_cmd_t cmd);
vtss_rc vtss_jr1_chip_id_get_chip(vtss_state_t *vtss_state, vtss_chip_id_t *const chip_id);
vtss_rc vtss_jr1_misc_debug_print(vtss_state_t *vtss_state,
                                  const vtss_debug_printf_t pr,
                                  const vtss_debug_info_t   *const info);

/* EVC functions */
vtss_rc vtss_jr1_evc_init(vtss_state_t *vtss_state, vtss_init_cmd_t cmd);
vtss_rc vtss_jr1_evc_debug_print(vtss_state_t *vtss_state,
                                 const vtss_debug_printf_t pr,
                                 const vtss_debug_info_t   *const info);

/* Timestamp functions */
vtss_rc vtss_jr1_ts_init(vtss_state_t *vtss_state, vtss_init_cmd_t cmd);
vtss_rc vtss_jr1_ts_debug_print(vtss_state_t *vtss_state,
                                const vtss_debug_printf_t pr,
                                const vtss_debug_info_t   *const info);

/* L3 functions */
vtss_rc vtss_jr1_l3_init(vtss_state_t *vtss_state, vtss_init_cmd_t cmd);
vtss_rc vtss_jr1_l3_debug_print(vtss_state_t *vtss_state,
                                const vtss_debug_printf_t pr,
                                const vtss_debug_info_t   *const info);

#endif /* VTSS_ARCH_JAGUAR_1 */
#endif /* _VTSS_JAGUAR1_CIL_H_ */
