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

#include "vtss_serval_cil.h"

// Avoid Lint Warning 572: Excessive shift value (precision 1 shifted right by 2), which occurs
// in this file because (t) - VTSS_IO_ORIGIN1_OFFSET == 0 for t = VTSS_TO_CFG (i.e. ICPU_CFG), and 0 >> 2 gives a lint warning.
/*lint --e{572} */
#if defined(VTSS_ARCH_SERVAL)

static vtss_rc srvl_wr_indirect(vtss_state_t *vtss_state, u32 addr, u32 value);
static vtss_rc srvl_rd_indirect(vtss_state_t *vtss_state, u32 addr, u32 *value);

vtss_rc (*vtss_srvl_wr)(vtss_state_t *vtss_state, u32 addr, u32 value) = srvl_wr_indirect;
vtss_rc (*vtss_srvl_rd)(vtss_state_t *vtss_state, u32 addr, u32 *value) = srvl_rd_indirect;

/* Read target register using current CPU interface */
static inline vtss_rc srvl_rd_direct(vtss_state_t *vtss_state, u32 reg, u32 *value)
{
    return vtss_state->init_conf.reg_read(0, reg, value);
}

/* Write target register using current CPU interface */
static inline vtss_rc srvl_wr_direct(vtss_state_t *vtss_state, u32 reg, u32 value)
{
    return vtss_state->init_conf.reg_write(0, reg, value);
}

static inline BOOL srvl_reg_directly_accessible(u32 addr)
{
    /* Running on external CPU. VCoreIII registers require indirect access. */
    /* On internal CPU, all registers are always directly accessible. */
    return (addr >= ((VTSS_IO_ORIGIN2_OFFSET - VTSS_IO_ORIGIN1_OFFSET) >> 2));
}

/* Read or write register indirectly */
static vtss_rc srvl_reg_indirect_access(vtss_state_t *vtss_state,
                                        u32 addr, u32 *value, BOOL is_write)
{
    /* The following access must be executed atomically, and since this function may be called
     * without the API lock taken, we have to disable the scheduler
     */
    /*lint --e{529} */ // Avoid "Symbol 'flags' not subsequently referenced" Lint warning
    VTSS_OS_SCHEDULER_FLAGS flags = 0;
    u32 ctrl;
    vtss_rc result;

    /* The @addr is an address suitable for the read or write callout function installed by
     * the application, i.e. it's a 32-bit address suitable for presentation on a PI
     * address bus, i.e. it's not suitable for presentation on the VCore-III shared bus.
     * In order to make it suitable for presentation on the VCore-III shared bus, it must
     * be made an 8-bit address, so we multiply by 4, and it must be offset by the base
     * address of the switch core registers, so we add VTSS_IO_ORIGIN1_OFFSET.
     */
    addr <<= 2;
    addr += VTSS_IO_ORIGIN1_OFFSET;

    VTSS_OS_SCHEDULER_LOCK(flags);

    if ((result = vtss_srvl_wr(vtss_state, VTSS_DEVCPU_GCB_VCORE_ACCESS_VA_ADDR, addr)) != VTSS_RC_OK) {
        goto do_exit;
    }
    if (is_write) {
        if ((result = vtss_srvl_wr(vtss_state, VTSS_DEVCPU_GCB_VCORE_ACCESS_VA_DATA, *value)) != VTSS_RC_OK) {
            goto do_exit;
        }
        // Wait for operation to complete
        do {
            if ((result = vtss_srvl_rd(vtss_state, VTSS_DEVCPU_GCB_VCORE_ACCESS_VA_CTRL, &ctrl)) != VTSS_RC_OK) {
                goto do_exit;
            }
        } while (ctrl & VTSS_F_DEVCPU_GCB_VCORE_ACCESS_VA_CTRL_VA_BUSY);
    } else {
        // Dummy read to initiate access
        if ((result = vtss_srvl_rd(vtss_state, VTSS_DEVCPU_GCB_VCORE_ACCESS_VA_DATA, value)) != VTSS_RC_OK) {
            goto do_exit;
        }
        // Wait for operation to complete
        do {
            if ((result = vtss_srvl_rd(vtss_state, VTSS_DEVCPU_GCB_VCORE_ACCESS_VA_CTRL, &ctrl)) != VTSS_RC_OK) {
                goto do_exit;
            }
        } while (ctrl & VTSS_F_DEVCPU_GCB_VCORE_ACCESS_VA_CTRL_VA_BUSY);
        if ((result = vtss_srvl_rd(vtss_state, VTSS_DEVCPU_GCB_VCORE_ACCESS_VA_DATA, value)) != VTSS_RC_OK) {
            goto do_exit;
        }
    }

do_exit:
    VTSS_OS_SCHEDULER_UNLOCK(flags);
    return result;
}

/* Read target register using current CPU interface */
static vtss_rc srvl_rd_indirect(vtss_state_t *vtss_state, u32 reg, u32 *value)
{
    if (srvl_reg_directly_accessible(reg)) {
        return vtss_state->init_conf.reg_read(0, reg, value);
    } else {
        return srvl_reg_indirect_access(vtss_state, reg, value, FALSE);
    }
}

/* Write target register using current CPU interface */
static vtss_rc srvl_wr_indirect(vtss_state_t *vtss_state, u32 reg, u32 value)
{
    if (srvl_reg_directly_accessible(reg)) {
        return vtss_state->init_conf.reg_write(0, reg, value);
    } else {
        return srvl_reg_indirect_access(vtss_state, reg, &value, TRUE);
    }
}

/* Read-modify-write target register using current CPU interface */
vtss_rc vtss_srvl_wrm(vtss_state_t *vtss_state, u32 reg, u32 value, u32 mask)
{
    vtss_rc rc;
    u32     val;

    if ((rc = vtss_srvl_rd(vtss_state, reg, &val)) == VTSS_RC_OK) {
        val = ((val & ~mask) | (value & mask));
        rc = vtss_srvl_wr(vtss_state, reg, val);
    }
    return rc;
}

/* ================================================================= *
 *  Utility functions
 * ================================================================= */

u32 vtss_srvl_port_mask(vtss_state_t *vtss_state, const BOOL member[])
{
    vtss_port_no_t port_no;
    u32            port, mask = 0;
    
    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
        if (member[port_no]) {
            port = VTSS_CHIP_PORT(port_no);
            mask |= VTSS_BIT(port);
        }
    }
    return mask;
}

vtss_rc vtss_srvl_counter_update(vtss_state_t *vtss_state,
                                 u32 *addr, vtss_chip_counter_t *counter, BOOL clear)
{
    u32 value;

    SRVL_RD(VTSS_SYS_STAT_CNT(*addr), &value);
    *addr = (*addr + 1); /* Next counter address */
    vtss_cmn_counter_32_update(value, counter, clear);

    return VTSS_RC_OK;
}

/* ================================================================= *
 *  Debug print utility functions
 * ================================================================= */

void vtss_srvl_debug_print_port_header(vtss_state_t *vtss_state,
                                       const vtss_debug_printf_t pr, const char *txt)
{
    vtss_debug_print_port_header(vtss_state, pr, txt, VTSS_CHIP_PORTS + 1, 1);
}

void vtss_srvl_debug_print_mask(const vtss_debug_printf_t pr, u32 mask)
{
    u32 port;
    
    for (port = 0; port <= VTSS_CHIP_PORTS; port++) {
        pr("%s%s", port == 0 || (port & 7) ? "" : ".", ((1<<port) & mask) ? "1" : "0");
    }
    pr("  0x%08x\n", mask);
}

void vtss_srvl_debug_reg_header(const vtss_debug_printf_t pr, const char *name) 
{
    char buf[64];
    
    sprintf(buf, "%-32s  Tgt   Addr", name);
    vtss_debug_print_reg_header(pr, buf);
}

void vtss_srvl_debug_reg(vtss_state_t *vtss_state,
                         const vtss_debug_printf_t pr, u32 addr, const char *name)
{
    u32 value;
    char buf[64];

    if (vtss_srvl_rd(vtss_state, addr, &value) == VTSS_RC_OK) {
        sprintf(buf, "%-32s  0x%02x  0x%04x", name, (addr >> 14) & 0x3f, addr & 0x3fff);
        vtss_debug_print_reg(pr, buf, value);
    }
}

void vtss_srvl_debug_reg_inst(vtss_state_t *vtss_state,
                              const vtss_debug_printf_t pr, u32 addr, u32 i, const char *name)
{
    char buf[64];

    sprintf(buf, "%s_%u", name, i);
    vtss_srvl_debug_reg(vtss_state, pr, addr, buf);
}

void vtss_srvl_debug_cnt(const vtss_debug_printf_t pr, const char *col1, const char *col2, 
                         vtss_chip_counter_t *c1, vtss_chip_counter_t *c2)
{
    char buf[80];
    
    sprintf(buf, "rx_%s:", col1);
    pr("%-28s%10" PRIu64 "   ", buf, c1->prev);
    if (col2 != NULL) {
        sprintf(buf, "tx_%s:", strlen(col2) ? col2 : col1);
        pr("%-28s%10" PRIu64, buf, c2->prev);
    }
    pr("\n");
}

static vtss_rc srvl_debug_info_print(vtss_state_t *vtss_state,
                                     const vtss_debug_printf_t pr,
                                     const vtss_debug_info_t   *const info)
{
    VTSS_RC(vtss_srvl_misc_debug_print(vtss_state, pr, info));
    VTSS_RC(vtss_srvl_port_debug_print(vtss_state, pr, info));
    VTSS_RC(vtss_srvl_l2_debug_print(vtss_state, pr, info));
    VTSS_RC(vtss_srvl_vcap_debug_print(vtss_state, pr, info));
    VTSS_RC(vtss_srvl_qos_debug_print(vtss_state, pr, info));
#ifdef VTSS_FEATURE_EVC
    VTSS_RC(vtss_srvl_evc_debug_print(vtss_state, pr, info));
#endif
    VTSS_RC(vtss_srvl_packet_debug_print(vtss_state, pr, info));
#if defined(VTSS_FEATURE_AFI_SWC)
    VTSS_RC(vtss_srvl_afi_debug_print(vtss_state, pr, info));
#endif /* VTSS_FEATURE_AFI_SWC */
#if defined(VTSS_FEATURE_TIMESTAMP)
    VTSS_RC(vtss_srvl_ts_debug_print(vtss_state, pr, info));
#endif /* VTSS_FEATURE_TIMESTAMP */
#if defined(VTSS_FEATURE_OAM)
    VTSS_RC(vtss_srvl_oam_debug_print(vtss_state, pr, info));
#endif /* VTSS_FEATURE_OAM */
#if defined (VTSS_FEATURE_MPLS)
    VTSS_RC(vtss_srvl_mpls_debug_print(vtss_state, pr, info));
#endif /* VTSS_FEATURE_MPLS */
#if defined(VTSS_FEATURE_FDMA) && VTSS_OPT_FDMA
    if (vtss_debug_group_enabled(pr, info, VTSS_DEBUG_GROUP_FDMA)) {
        if (vtss_state->fdma_state.fdma_func.fdma_debug_print != NULL) {
            return vtss_state->fdma_state.fdma_func.fdma_debug_print(vtss_state, pr, info);
        } else {
            return VTSS_RC_ERROR;
        }
    }
#endif 
    return VTSS_RC_OK;
}

vtss_rc vtss_srvl_init_groups(vtss_state_t *vtss_state, vtss_init_cmd_t cmd)
{
    /* Initialize ports */
    VTSS_RC(vtss_srvl_port_init(vtss_state, cmd));

    /* Initialize miscellaneous */
    VTSS_RC(vtss_srvl_misc_init(vtss_state, cmd));

    /* Initialize packet before L2 to ensure that VLAN table clear does not break VRAP access */
    VTSS_RC(vtss_srvl_packet_init(vtss_state, cmd));

    /* Initialize L2 */
    VTSS_RC(vtss_srvl_l2_init(vtss_state, cmd));

#ifdef VTSS_FEATURE_EVC
    VTSS_RC(vtss_srvl_evc_init(vtss_state, cmd));
#endif

    /* Initialize VCAP */
    VTSS_RC(vtss_srvl_vcap_init(vtss_state, cmd));

    /* Initialize QoS */
    VTSS_RC(vtss_srvl_qos_init(vtss_state, cmd));

#if defined(VTSS_FEATURE_TIMESTAMP)
    VTSS_RC(vtss_srvl_ts_init(vtss_state, cmd));
#endif /* VTSS_FEATURE_TIMESTAMP */

#if defined(VTSS_FEATURE_OAM)
    VTSS_RC(vtss_srvl_oam_init(vtss_state, cmd));
#endif

#if defined(VTSS_FEATURE_MPLS)
    VTSS_RC(vtss_srvl_mpls_init(vtss_state, cmd));
#endif

    return VTSS_RC_OK;
}

static vtss_rc srvl_port_map_set(vtss_state_t *vtss_state)
{
    /* Initialize function groups */
    return vtss_srvl_init_groups(vtss_state, VTSS_INIT_CMD_PORT_MAP);
}   

#define SRVL_API_VERSION 1

static vtss_rc srvl_restart_conf_set(vtss_state_t *vtss_state)
{
    SRVL_WR(VTSS_DEVCPU_GCB_CHIP_REGS_GPR, vtss_cmn_restart_value_get(vtss_state));

    return VTSS_RC_OK;
}

static vtss_rc srvl_stop_prepare(vtss_state_t *vtss_state)

{
    int i;
    /* This will discard all CPU queue data */
    SRVL_WRM(VTSS_QSYS_SYSTEM_EXT_CPU_CFG, 
             VTSS_F_QSYS_SYSTEM_EXT_CPU_CFG_EXT_CPU_PORT(VTSS_CHIP_PORT_CPU_1),
             VTSS_M_QSYS_SYSTEM_EXT_CPU_CFG_EXT_CPU_PORT);
    /* All queues redirect to "discard" port */
    SRVL_WRM_SET(VTSS_QSYS_SYSTEM_EXT_CPU_CFG, 
                 VTSS_M_QSYS_SYSTEM_EXT_CPU_CFG_EXT_CPUQ_MSK);  /* All queues */
    /* Rx disable all ports */
    for (i = 0; i < VTSS_CHIP_PORTS; i++) {
        u32 tgt = VTSS_TO_DEV(i);
        SRVL_WRM_CLR(VTSS_DEV_MAC_CFG_STATUS_MAC_ENA_CFG(tgt),
                     VTSS_F_DEV_MAC_CFG_STATUS_MAC_ENA_CFG_RX_ENA);
    }
    /* Allow to settle/drain */
    VTSS_MSLEEP(500);
    return VTSS_RC_OK;
}

static vtss_rc srvl_fdma_flush(vtss_state_t *vtss_state)
{
    int i, max_qframes = 16*1024;  /* Max arbitrarily chosen */
    u32 val, dummy;

    /* Be sure to flush to avoid PCIe bus lock-up. And 2 times due to buffer architecture */
    for (i = 0; i < max_qframes; i++) {
        SRVL_RD(VTSS_ICPU_CFG_MANUAL_XTRINJ_MANUAL_INTR, &val);
        if (val & VTSS_F_ICPU_CFG_MANUAL_XTRINJ_MANUAL_INTR_INTR_XTR_ANY_RDY) {
            SRVL_RD(VTSS_ICPU_CFG_MANUAL_XTRINJ_MANUAL_XTR(0), &dummy); /* Dummy read */
            SRVL_WR(VTSS_ICPU_CFG_MANUAL_XTRINJ_MANUAL_XTR(4096-1), VTSS_BIT(18)); /* Force abort current frame */
            VTSS_NSLEEP(1000);  /* 1usec */
            SRVL_RD(VTSS_ICPU_CFG_MANUAL_XTRINJ_MANUAL_XTR(4096-1), &dummy); /* bring FDMA manual logic into known state */
        } else {
            break;
        }
    }
    return i < max_qframes ? VTSS_RC_OK : VTSS_RC_ERROR;
}

static vtss_rc srvl_init_conf_set(vtss_state_t *vtss_state)
{
    u32 value, i;
    /*lint -esym(459, vtss_srvl_rd, vtss_srvl_wr) */

    VTSS_D("enter");

#if VTSS_OPT_VCORE_III
    // For register access checking.
    vtss_state->reg_check.addr = VTSS_DEVCPU_ORG_ORG_ERR_CNTS;
#endif

    /* Read chip ID to check CPU interface */
    VTSS_RC(vtss_srvl_chip_id_get(vtss_state, &vtss_state->misc.chip_id));
    VTSS_I("chip_id: 0x%04x, revision: 0x%04x", 
           vtss_state->misc.chip_id.part_number, vtss_state->misc.chip_id.revision);

    /* Use SEMA0_OWNER to determine if using VCOREIII or PCIe */
    SRVL_WR(VTSS_DEVCPU_ORG_ORG_SEMA0, 0xFFFFFFFF); /* Release sema */
    SRVL_RD(VTSS_DEVCPU_ORG_ORG_SEMA0, &value);     /* Get sema */
    SRVL_RD(VTSS_DEVCPU_ORG_ORG_SEMA0_OWNER, &value);
    SRVL_WR(VTSS_DEVCPU_ORG_ORG_SEMA0, 0xFFFFFFFF); /* Release again */
    if (value == 0x01) {        /* VCoreIII/PCIe */
        VTSS_I("Has VCoreIII/PCIe, Acessing registers directly");
        vtss_srvl_rd = srvl_rd_direct;
        vtss_srvl_wr = srvl_wr_direct;
    }

    /* Read VCore_cfg */
    /*lint -e(572) ... bogus warning */
    SRVL_RD(VTSS_ICPU_CFG_CPU_SYSTEM_CTRL_GENERAL_STAT, &value);
    vtss_state->sys_config.vcore_cfg = VTSS_X_ICPU_CFG_CPU_SYSTEM_CTRL_GENERAL_STAT_VCORE_CFG(value);
    /* See datasheet, Table 152 */
    vtss_state->sys_config.using_vcoreiii = 
        (vtss_state->sys_config.vcore_cfg == 12 || vtss_state->sys_config.vcore_cfg == 13);
    vtss_state->sys_config.using_vrap = (vtss_state->sys_config.vcore_cfg < 9); 
    vtss_state->sys_config.using_pcie = (vtss_state->sys_config.vcore_cfg == 9);
    VTSS_I("Vcore_cfg: 0x%04x, VCOREIII: %d, VRAP: %d, PCIe: %d", 
           vtss_state->sys_config.vcore_cfg, 
           vtss_state->sys_config.using_vcoreiii,
           vtss_state->sys_config.using_vrap,
           vtss_state->sys_config.using_pcie);

    /* Read restart type */
    SRVL_RD(VTSS_DEVCPU_GCB_CHIP_REGS_GPR, &value);
    VTSS_RC(vtss_cmn_restart_update(vtss_state, value));
    if (vtss_state->warm_start_cur)
        return VTSS_RC_OK;

    if ((!vtss_state->sys_config.using_vcoreiii) && (!vtss_state->sys_config.using_vrap)) {
        if (vtss_state->sys_config.using_pcie) {
            SRVL_RD(VTSS_ICPU_CFG_MANUAL_XTRINJ_MANUAL_CFG, &value);
            /* Only flush if FDMA is active */
            if (value & VTSS_F_ICPU_CFG_MANUAL_XTRINJ_MANUAL_CFG_XTR_ENA) {
                VTSS_RC(srvl_stop_prepare(vtss_state));
                VTSS_RC(srvl_fdma_flush(vtss_state));
                /* Disable until further notice */
                SRVL_WRM_CLR(VTSS_ICPU_CFG_MANUAL_XTRINJ_MANUAL_CFG, VTSS_F_ICPU_CFG_MANUAL_XTRINJ_MANUAL_CFG_XTR_ENA);
            }
        } else {
            /* In lack of better, reset extraction queue */
            SRVL_WRM_SET(VTSS_DEVCPU_QS_XTR_XTR_FLUSH, VTSS_M_DEVCPU_QS_XTR_XTR_FLUSH_FLUSH); /* All Queues */
            VTSS_MSLEEP(1);         /* Allow to drain */
            SRVL_WRM_CLR(VTSS_DEVCPU_QS_XTR_XTR_FLUSH, VTSS_M_DEVCPU_QS_XTR_XTR_FLUSH_FLUSH);
        }
#if 0
        u32 endian;
        /* Reset, preserve endianess */
        SRVL_RD(VTSS_DEVCPU_ORG_ORG_IF_CTRL, &endian);
        VTSS_I("External CPU and not VRAP - Resetting (endian 0x%0x)", endian);
        SRVL_WR(VTSS_DEVCPU_GCB_CHIP_REGS_SOFT_RST,
                VTSS_F_DEVCPU_GCB_CHIP_REGS_SOFT_RST_SOFT_CHIP_RST);
        SRVL_RD(VTSS_DEVCPU_GCB_CHIP_REGS_SOFT_RST, &value);
        VTSS_MSLEEP(10);        /* For good measure...? */
        SRVL_WR(VTSS_DEVCPU_ORG_ORG_IF_CTRL, endian);
        SRVL_RD(VTSS_DEVCPU_ORG_ORG_IF_CTRL, &value);
        if (endian != value) {
            VTSS_E("Endianness changed after reset: 0x%0x >= 0x%0x", endian, value);
            return VTSS_RC_ERROR;
        }
#endif
    }

    /* Initialize memories */
    SRVL_RD(VTSS_SYS_SYSTEM_RESET_CFG, &value);
    if (!(value & VTSS_F_SYS_SYSTEM_RESET_CFG_MEM_ENA)) {
        /* Avoid initialization if already done by VRAP strapping */
        value = 0;
#if (VTSS_MPLS_OUT_ENCAP_CNT > 0)
        // We must reserve MPLS egress encapsulation memory. The chip field
        // semantics are:
        //  N == 0 => don't reserve;
        //  N > 0  => reserve 32 + N * 32 entries

#if (VTSS_MPLS_OUT_ENCAP_CNT < 64)
        value = VTSS_F_SYS_SYSTEM_RESET_CFG_ENCAP_CNT(1);
#else
        value = VTSS_F_SYS_SYSTEM_RESET_CFG_ENCAP_CNT((VTSS_MPLS_OUT_ENCAP_CNT + 31) / 32 - 1);
#endif
#endif
        SRVL_WR(VTSS_SYS_SYSTEM_RESET_CFG,
                VTSS_F_SYS_SYSTEM_RESET_CFG_MEM_ENA  |
                VTSS_F_SYS_SYSTEM_RESET_CFG_MEM_INIT |
                value);
        for (i = 0; ; i++) {
            VTSS_MSLEEP(1); /* MEM_INIT should clear after appx. 22us */
            SRVL_RD(VTSS_SYS_SYSTEM_RESET_CFG, &value);
            if (value & VTSS_F_SYS_SYSTEM_RESET_CFG_MEM_INIT) {
                if (i == 10) {
                    VTSS_E("Memory initialization error, SYS::RESET_CFG: 0x%08x", value);
                    return VTSS_RC_ERROR;
                }
            } else {
                break;
            }
        }
        
        /* Enable switch core */
        SRVL_WRM_SET(VTSS_SYS_SYSTEM_RESET_CFG, VTSS_F_SYS_SYSTEM_RESET_CFG_CORE_ENA);
    }

    /* Initialize function groups */
    VTSS_RC(vtss_srvl_init_groups(vtss_state, VTSS_INIT_CMD_INIT));

    VTSS_D("exit");

    return VTSS_RC_OK;
}

vtss_rc vtss_serval_inst_create(vtss_state_t *vtss_state)
{
    /* Initialization */
    vtss_state->cil.init_conf_set = srvl_init_conf_set;
    vtss_state->cil.restart_conf_set = srvl_restart_conf_set;
    vtss_state->cil.debug_info_print = srvl_debug_info_print;
    vtss_state->port.map_set = srvl_port_map_set;

    /* Create function groups */
    return vtss_srvl_init_groups(vtss_state, VTSS_INIT_CMD_CREATE);
}

#endif /* VTSS_ARCH_SERVAL */
