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

*/

#include "vtss_jaguar1_cil.h"

#if defined(VTSS_ARCH_JAGUAR_1)

#if defined(VTSS_FEATURE_WIS)
#include "vtss_wis_api.h"
#endif /* VTSS_FEATURE_WIS */

/* ================================================================= *
 *  Function declarations
 * ================================================================= */

/* Is register given by @addr directly accessible? */
static inline BOOL jr_reg_directly_accessible(vtss_chip_no_t chip_no, u32 addr)
{
#if VTSS_OPT_VCORE_III
    /* Running on internal CPU. chip_no == 0 registers are always directly accessible. */
    return (chip_no == 0 || addr < ((VTSS_IO_ORIGIN2_OFFSET - VTSS_IO_ORIGIN1_OFFSET) >> 2));
#else
    /* Running on external CPU. Both chip_no 0 and 1 are subject to indirect access */
    return (addr < ((VTSS_IO_ORIGIN2_OFFSET - VTSS_IO_ORIGIN1_OFFSET) >> 2));
#endif
}

/* Read or write register indirectly */
static vtss_rc jr_reg_indirect_access(const vtss_state_t *vtss_state,
                                      vtss_chip_no_t chip_no, u32 addr, u32 *value, BOOL is_write)
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

    if ((result = vtss_jr1_wr_chip(vtss_state, chip_no, VTSS_DEVCPU_GCB_VCORE_ACCESS_VA_ADDR, addr)) != VTSS_RC_OK) {
        goto do_exit;
    }
    if (is_write) {
        if ((result = vtss_jr1_wr_chip(vtss_state, chip_no, VTSS_DEVCPU_GCB_VCORE_ACCESS_VA_DATA, *value)) != VTSS_RC_OK) {
            goto do_exit;
        }
        // Wait for operation to complete
        do {
            if ((result = vtss_jr1_rd_chip(vtss_state, chip_no, VTSS_DEVCPU_GCB_VCORE_ACCESS_VA_CTRL, &ctrl)) != VTSS_RC_OK) {
                goto do_exit;
            }
        } while (ctrl & VTSS_F_DEVCPU_GCB_VCORE_ACCESS_VA_CTRL_VA_BUSY);
    } else {
        // Dummy read to initiate access
        if ((result = vtss_jr1_rd_chip(vtss_state, chip_no, VTSS_DEVCPU_GCB_VCORE_ACCESS_VA_DATA, value)) != VTSS_RC_OK) {
            goto do_exit;
        }
        // Wait for operation to complete
        do {
            if ((result = vtss_jr1_rd_chip(vtss_state, chip_no, VTSS_DEVCPU_GCB_VCORE_ACCESS_VA_CTRL, &ctrl)) != VTSS_RC_OK) {
                goto do_exit;
            }
        } while (ctrl & VTSS_F_DEVCPU_GCB_VCORE_ACCESS_VA_CTRL_VA_BUSY);
        if ((result = vtss_jr1_rd_chip(vtss_state, chip_no, VTSS_DEVCPU_GCB_VCORE_ACCESS_VA_DATA, value)) != VTSS_RC_OK) {
            goto do_exit;
        }
    }

do_exit:
    VTSS_OS_SCHEDULER_UNLOCK(flags);
    return result;
}

/* Read register */
vtss_rc vtss_jr1_rd(const vtss_state_t *vtss_state, u32 addr, u32 *value)
{
    if (jr_reg_directly_accessible(vtss_state->chip_no, addr)) {
        return vtss_state->init_conf.reg_read(vtss_state->chip_no, addr, value);
    } else {
        return jr_reg_indirect_access(vtss_state, vtss_state->chip_no, addr, value, FALSE);
    }
}

/* Read register */
vtss_rc vtss_jr1_rd_chip(const vtss_state_t *vtss_state,
                         vtss_chip_no_t chip_no, u32 addr, u32 *value)
{
    if (jr_reg_directly_accessible(chip_no, addr)) {
        return vtss_state->init_conf.reg_read(chip_no, addr, value);
    } else {
        return jr_reg_indirect_access(vtss_state, chip_no, addr, value, FALSE);
    }
}

/* Write register */
vtss_rc vtss_jr1_wr(const vtss_state_t *vtss_state, u32 addr, u32 value)
{
    if (jr_reg_directly_accessible(vtss_state->chip_no, addr)) {
        return vtss_state->init_conf.reg_write(vtss_state->chip_no, addr, value);
    } else {
        return jr_reg_indirect_access(vtss_state, vtss_state->chip_no, addr, &value, TRUE);
    }
}

/* Write register */
vtss_rc vtss_jr1_wr_chip(const vtss_state_t *vtss_state,
                         vtss_chip_no_t chip_no, u32 addr, u32 value)
{
    if (jr_reg_directly_accessible(chip_no, addr)) {
        return vtss_state->init_conf.reg_write(chip_no, addr, value);
    } else {
        return jr_reg_indirect_access(vtss_state, chip_no, addr, &value, TRUE);
    }
}

/* Read, modify and write register */
vtss_rc vtss_jr1_wrm(const vtss_state_t *vtss_state, u32 addr, u32 value, u32 mask)
{
    vtss_rc rc;
    u32     val;

    if ((rc = vtss_jr1_rd(vtss_state, addr, &val)) == VTSS_RC_OK)
        rc = vtss_jr1_wr(vtss_state, addr, (val & ~mask) | (value & mask));
    return rc;
}

/* Read, modify and write register */
#ifdef VTSS_FEATURE_EEE
vtss_rc vtss_jr1_wrm_chip(const vtss_state_t *vtss_state,
                          vtss_chip_no_t chip_no, u32 addr, u32 value, u32 mask)
{
  vtss_rc rc;
  u32     val;

  if ((rc = vtss_jr1_rd_chip(vtss_state, chip_no, addr, &val)) == VTSS_RC_OK) {
      rc = vtss_jr1_wr_chip(vtss_state, chip_no, addr, (val & ~mask) | (value & mask));
  }
  return rc;
}
#endif

/* ================================================================= *
 *  Utility functions
 * ================================================================= */

u32 vtss_jr1_port_mask(vtss_state_t *vtss_state, const BOOL member[])
{
    vtss_port_no_t port_no;
    u32            port, mask = 0;

    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
        if (member[port_no] && VTSS_PORT_CHIP_SELECTED(port_no)) {
            port = VTSS_CHIP_PORT(port_no);
            mask |= VTSS_BIT(port);
        }
    }
    return mask;
}

u32 vtss_jr1_port_mask_from_map(vtss_state_t *vtss_state,
                                BOOL include_internal_ports, BOOL include_stack_ports)
{
    vtss_port_no_t port_no;
    u32            mask = 0;

    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
        if (VTSS_PORT_CHIP_SELECTED(port_no) 
#if defined(VTSS_FEATURE_VSTAX_V2)
            && (vtss_state->l2.vstax_port_enabled[port_no] == 0 || include_stack_ports)
#endif /* VSTAX_V2 */
            ) {
            mask |= VTSS_BIT(VTSS_CHIP_PORT(port_no));
        }
    }
    if (include_internal_ports && vtss_state->chip_count == 2) {
        /* Include internal ports in mask */
        mask |= vtss_state->port.mask_int_ports;
    }
    return mask;
}

/* Convert to PGID in chip */
u32 vtss_jr1_chip_pgid(vtss_state_t *vtss_state, u32 pgid)
{
    if (pgid < vtss_state->port_count) {
        return VTSS_CHIP_PORT(pgid);
    } else {
        return (pgid + VTSS_CHIP_PORTS - vtss_state->port_count);
    }
}

/* Convert from PGID in chip */
u32 vtss_jr1_vtss_pgid(const vtss_state_t *const state, u32 pgid)
{
    vtss_port_no_t        port_no;
    const vtss_port_map_t *pmap;

    if (pgid < VTSS_CHIP_PORTS) {
        for (port_no = VTSS_PORT_NO_START; port_no < state->port_count; port_no++) {
            pmap = &state->port.map[port_no];
            if (pmap->chip_port == pgid && pmap->chip_no == state->chip_no)
                break;
        }
        return port_no;
    } else {
        return (state->port_count + pgid - VTSS_CHIP_PORTS);
    }
}

u32 vtss_jr1_port_prio_2_qu(vtss_state_t *vtss_state, u32 port, u32 prio, u32 qs)
{
    if ((port < 32) || (qs == 0)) {
        return 8*port + prio;
    } else {      
        if (port == 32+48) {
            // OQS-VD
            return (port-48)*8+192+prio;
        } else if ((port > 80) && (port < 89)) {
            // OQS CPU
            return (81-48)*8 + 192 + (port-81);
        } else {
#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC)   
            u32 prios;
            // OQS Host ports            
            if ((vtss_jr1_ce_max_hm(vtss_state) == 0) ||  (vtss_jr1_ce_max_hm(vtss_state) == 2)) {
                prios = 8;
            } else {
                prios = 4;
            }
            return 256 + (port-32)*prios + prio;
#else
            return (81-48)*8 + 192 + (port-81);
#endif
        }
    }
}

/* Call configuration function for all devices */
vtss_rc vtss_jr1_conf_chips(vtss_state_t *vtss_state, vtss_rc (*func_conf)(vtss_state_t *))
{
    vtss_chip_no_t chip_no;

    /* Setup all devices */
    for (chip_no = 0; chip_no < vtss_state->chip_count; chip_no++) {
        VTSS_SELECT_CHIP(chip_no);
        VTSS_RC(func_conf(vtss_state));
    }
    return VTSS_RC_OK;
}

/* ================================================================= *
 *  Debug print
 * ================================================================= */

void vtss_jr1_debug_reg_header(const vtss_debug_printf_t pr, const char *name)
{
    char buf[64];
    
    sprintf(buf, "%-18s  Tgt  Addr", name);
    vtss_debug_print_reg_header(pr, buf);
}

void vtss_jr1_debug_print_mask(const vtss_debug_printf_t pr, u32 mask)
{
    u32 port;
    
    for (port = 0; port < VTSS_CHIP_PORTS; port++) {
        pr("%s%s", port == 0 || (port & 7) ? "" : ".", ((1<<port) & mask) ? "1" : "0");
    }
    pr("  0x%08x\n", mask);
}

void vtss_jr1_debug_print_port_header(vtss_state_t *vtss_state,
                                       const vtss_debug_printf_t pr, const char *txt)
{
    vtss_debug_print_port_header(vtss_state, pr, txt, VTSS_CHIP_PORTS, 1);
}

static void jr_debug_reg_clr(vtss_state_t *vtss_state,
                             const vtss_debug_printf_t pr, u32 addr, const char *name, BOOL clr)
{
    u32  value, tgt;
    char buf[64];

    if (vtss_jr1_rd(vtss_state, addr, &value) == VTSS_RC_OK &&
        (clr == 0 || vtss_jr1_wr(vtss_state, addr, value) == VTSS_RC_OK)) {
        tgt = ((addr >> 14) & 0xff);
        if (tgt < 128) {
            /* 14 bit address */
            addr &= 0x3fff;
        } else {
            /* 18 bit address */
            tgt &= 0xf0;
            addr &= 0x3ffff;
        }
        sprintf(buf, "%-18s  0x%02x 0x%05x", name, tgt, addr);
        vtss_debug_print_reg(pr, buf, value);
    }
}

void vtss_jr1_debug_sticky(vtss_state_t *vtss_state,
                           const vtss_debug_printf_t pr, u32 addr, const char *name)
{
    jr_debug_reg_clr(vtss_state, pr, addr, name, 1);
}

void vtss_jr1_debug_sticky_inst(vtss_state_t *vtss_state,
                                const vtss_debug_printf_t pr, u32 addr, u32 i, 
                                const char *name)
{
    char buf[64];

    sprintf(buf, "%s_%u", name, i);
    vtss_jr1_debug_sticky(vtss_state, pr, addr, buf);
}

void vtss_jr1_debug_reg(vtss_state_t *vtss_state,
                        const vtss_debug_printf_t pr, u32 addr, const char *name)
{
    jr_debug_reg_clr(vtss_state, pr, addr, name, 0);
}

void vtss_jr1_debug_reg_inst(vtss_state_t *vtss_state,
                             const vtss_debug_printf_t pr, u32 addr, u32 i, const char *name)
{
    char buf[64];

    sprintf(buf, "%s_%u", name, i);
    vtss_jr1_debug_reg(vtss_state, pr, addr, buf);
}

void vtss_jr1_debug_cnt(vtss_state_t *vtss_state,
                        const vtss_debug_printf_t pr, const char *col1, const char *col2, 
                        vtss_chip_counter_t *c1, vtss_chip_counter_t *c2)
{
    char buf[80];
    
    if (col1 != NULL) {
        sprintf(buf, "rx_%s:", col1);
        pr("%-19s%19llu   ", buf, c1->prev);
    }
    if (col2 != NULL) {
        sprintf(buf, "tx_%s:", strlen(col2) ? col2 : col1);
        pr("%-19s%19llu", buf, c2->prev);
    }
    pr("\n");
}

void vtss_jr1_debug_fld_nl(const vtss_debug_printf_t pr, const char *name, u32 value)
{
    pr("%-20s: %u\n", name, value);
}

BOOL vtss_jr1_debug_port_skip(vtss_state_t *vtss_state,
                              const vtss_port_no_t port_no,
                              const vtss_debug_info_t   *const info)
{
    /* Skip disabled ports and ports not on the current device */
    return (info->port_list[port_no] && VTSS_PORT_CHIP_SELECTED(port_no) ? 0 : 1);
}

static vtss_rc jr_debug_info_print(vtss_state_t *vtss_state,
                                   const vtss_debug_printf_t pr,
                                   const vtss_debug_info_t   *const info)
{
    VTSS_RC(vtss_jr1_misc_debug_print(vtss_state, pr, info));
    VTSS_RC(vtss_jr1_port_debug_print(vtss_state, pr, info));
    VTSS_RC(vtss_jr1_qos_debug_print(vtss_state, pr, info));
    VTSS_RC(vtss_jr1_l2_debug_print(vtss_state, pr, info));
    VTSS_RC(vtss_jr1_vcap_debug_print(vtss_state, pr, info));
#if defined(VTSS_FEATURE_EVC)
    VTSS_RC(vtss_jr1_evc_debug_print(vtss_state, pr, info));
#endif /* VTSS_FEATURE_EVC */
#if defined(VTSS_FEATURE_TIMESTAMP)
    VTSS_RC(vtss_jr1_ts_debug_print(vtss_state, pr, info));
#endif /* VTSS_FEATURE_TIMESTAMP */
    VTSS_RC(vtss_jr1_packet_debug_print(vtss_state, pr, info));
    VTSS_RC(vtss_jr1_l3_debug_print(vtss_state, pr, info));
    return VTSS_RC_OK;
}

/* ================================================================= *
 *  Initialization
 * ================================================================= */

static vtss_rc jr_restart_conf_set(vtss_state_t *vtss_state)
{
    JR_WR(DEVCPU_GCB, CHIP_REGS_GENERAL_PURPOSE, vtss_cmn_restart_value_get(vtss_state));

    return VTSS_RC_OK;
}

vtss_rc vtss_jr1_init_groups(vtss_state_t *vtss_state, vtss_init_cmd_t cmd)
{
    VTSS_RC(vtss_jr1_port_init(vtss_state, cmd)); /* Most block initialization is done by vtss_jr1_port_init() */
    VTSS_RC(vtss_jr1_vcap_init(vtss_state, cmd)); /* VCAP initialized early to allow other groups to setup TCAMs */
    VTSS_RC(vtss_jr1_qos_init(vtss_state, cmd));
    VTSS_RC(vtss_jr1_packet_init(vtss_state, cmd));
    VTSS_RC(vtss_jr1_l2_init(vtss_state, cmd));
    VTSS_RC(vtss_jr1_misc_init(vtss_state, cmd));
#if defined(VTSS_FEATURE_EVC)
    VTSS_RC(vtss_jr1_evc_init(vtss_state, cmd));
#endif /* VTSS_FEATURE_EVC */
#if defined(VTSS_FEATURE_TIMESTAMP)
    VTSS_RC(vtss_jr1_ts_init(vtss_state, cmd));
#endif /* VTSS_FEATURE_TIMESTAMP */
    VTSS_RC(vtss_jr1_l3_init(vtss_state, cmd));

    return VTSS_RC_OK;
}

static vtss_rc jr_port_map_set(vtss_state_t *vtss_state)
{
    /* Initialize function groups */
    return vtss_jr1_init_groups(vtss_state, VTSS_INIT_CMD_PORT_MAP);
}

static vtss_rc jr_init_conf_set_chip(vtss_state_t *vtss_state)
{
    u32 value;

    /* Read chip ID to check CPU interface */
    VTSS_RC(vtss_jr1_chip_id_get_chip(vtss_state, &vtss_state->misc.chip_id));
    VTSS_I("chip_id[%u]: 0x%04x, revision: 0x%04x",
           vtss_state->chip_no, vtss_state->misc.chip_id.part_number, vtss_state->misc.chip_id.revision);

    /* Check chip ID against created target */
    if ((vtss_state->misc.chip_id.part_number & 0xfff0) == 0x7430 && 
        (vtss_state->create.target & 0xfff0) == 0x7460) {
        /* SMB devices (VSC743x) do not support CE targets (VSC746x) */
        VTSS_E("device 0x%04x does not support CE software", vtss_state->misc.chip_id.part_number);
        return VTSS_RC_ERROR;
    }

    /* Read restart type */
    if (vtss_state->chip_no == 0) {
        JR_RD(DEVCPU_GCB, CHIP_REGS_GENERAL_PURPOSE, &value);
        vtss_state->init_conf.warm_start_enable = 0; /* Warm start not supported */
        VTSS_RC(vtss_cmn_restart_update(vtss_state, value));
    }

    /* Initialize function groups */
    VTSS_RC(vtss_jr1_init_groups(vtss_state, VTSS_INIT_CMD_INIT));

    return VTSS_RC_OK;
}

static vtss_rc jr_init_conf_set(vtss_state_t *vtss_state)
{
    /* Store mux modes per device */
    vtss_state->port.mux_mode[0] = vtss_state->init_conf.mux_mode;
#if defined(VTSS_ARCH_JAGUAR_1_DUAL)
    vtss_state->port.mux_mode[1] = vtss_state->init_conf.mux_mode_2;

    /* Select internal ports for dual Jaguar targets */
    switch (vtss_state->init_conf.dual_connect_mode) {
    case VTSS_DUAL_CONNECT_MODE_0: /* XAUI_0 and XAUI_1 */
        vtss_state->port.port_int_0 = 27;
        vtss_state->port.port_int_1 = 28;
        break;
    case VTSS_DUAL_CONNECT_MODE_1: /* XAUI_2 and XAUI_3 */
        vtss_state->port.port_int_0 = 29;
        vtss_state->port.port_int_1 = 30;
        break;
    default:
        VTSS_E("illegal connect mode");
        return VTSS_RC_ERROR;
    }
    vtss_state->port.mask_int_ports = (VTSS_BIT(vtss_state->port.port_int_0) | 
                                       VTSS_BIT(vtss_state->port.port_int_1));
#endif /* VTSS_ARCH_JAGUAR_1_DUAL */

    // For register access checking.
    // Avoid Lint Warning 572: Excessive shift value (precision 1 shifted right by 2),
    // which is due to use of the first target in the range (VTSS_DEVCPU_ORG).
    /*lint --e{572} */
    vtss_state->reg_check.addr = VTSS_DEVCPU_ORG_ORG_ERR_CNTS;

    /* Initialize all devices */
    return vtss_jr1_conf_chips(vtss_state, jr_init_conf_set_chip);
}

vtss_rc vtss_jaguar1_inst_create(vtss_state_t *vtss_state)
{
    vtss_cil_func_t *func = &vtss_state->cil;

    /* Initialization */
    func->init_conf_set = jr_init_conf_set;
    func->restart_conf_set = jr_restart_conf_set;
    func->debug_info_print = jr_debug_info_print;
    vtss_state->port.map_set = jr_port_map_set;

    /* Create function groups */
    VTSS_RC(vtss_jr1_init_groups(vtss_state, VTSS_INIT_CMD_CREATE));

#if defined(VTSS_FEATURE_WIS)
    if (vtss_phy_inst_ewis_create(vtss_state) != VTSS_RC_OK) {
    }
#endif

    return VTSS_RC_OK;
}

#endif /* VTSS_ARCH_JAGUAR_1 */

