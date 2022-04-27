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

#include "vtss_api.h"
#include "vtss_state.h"
#include "vtss_common.h"
#include "vtss_util.h" // For VTSS_ENCODE_BITFIELD

#if defined(VTSS_FEATURE_FDMA)
#include "vtss_fdma_common.h"
#endif

#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
#include "../phy/ts/vtss_phy_ts.h"
#endif
/*lint -esym(459, vtss_trace_conf) */
/*lint -esym(459, vtss_state)      */

/* Default mapping of PCP to QoS Class */
/* Can also be used for default mapping of QoS class to PCP */
/* This is the IEEE802.1Q-2011 recommended priority to traffic class mappings */
u32 vtss_cmn_pcp2qos(u32 pcp)
{
    switch (pcp) {
    case 0:
        return 1;
    case 1:
        return 0;
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
        return pcp;
    default:
        VTSS_E("Invalid PCP (%u)", pcp);
        return 0;
    }
}

vtss_rc vtss_port_no_check(vtss_state_t *vtss_state, const vtss_port_no_t port_no)
{
    if (port_no >= vtss_state->port_count) {
        VTSS_E("illegal port_no: %u", port_no);
        return VTSS_RC_ERROR;
    }

#if defined(VTSS_FEATURE_PORT_CONTROL)
    /* Set currently selected device using port map */
    VTSS_SELECT_CHIP_PORT_NO(port_no);
#endif /* VTSS_FEATURE_PORT_CONTROL */

    return VTSS_RC_OK;
}

vtss_rc vtss_cmn_restart_update(vtss_state_t *vtss_state, u32 value)
{
    vtss_init_conf_t *conf = &vtss_state->init_conf;

    /* Return if restart has already been updated */
    if (vtss_state->restart_updated)
        return VTSS_RC_OK;
    vtss_state->restart_updated = 1;

    vtss_state->restart_prev = VTSS_EXTRACT_BITFIELD(value,
                                                     VTSS_RESTART_TYPE_OFFSET,
                                                     VTSS_RESTART_TYPE_WIDTH);
    vtss_state->version_prev = VTSS_EXTRACT_BITFIELD(value,
                                                     VTSS_RESTART_VERSION_OFFSET,
                                                     VTSS_RESTART_VERSION_WIDTH);
    vtss_state->restart_cur = vtss_state->restart_prev;
    vtss_state->version_cur = VTSS_API_VERSION;
    switch (vtss_state->restart_cur) {
    case VTSS_RESTART_COLD:
        VTSS_I("cold start detected");
        break;
    case VTSS_RESTART_COOL:
        VTSS_I("cool start detected");
        break;
    case VTSS_RESTART_WARM:
        VTSS_I("warm start detected");
        if (conf->warm_start_enable) {
            if (vtss_state->version_prev > vtss_state->version_cur) {
                VTSS_I("downgrade from version %u to %u",
                       vtss_state->version_prev, vtss_state->version_cur);
            } else {
                vtss_state->warm_start_cur = 1;
                VTSS_I("warm starting");
                return VTSS_RC_OK;
            }
        } else {
            VTSS_I("warm start disabled");
        }
        /* Fall back to cool start */
        vtss_state->restart_cur = VTSS_RESTART_COOL;
        vtss_state->restart_prev = VTSS_RESTART_COOL;
        break;
    default:
        VTSS_I("unknown restart type");
        break;
    }
    return VTSS_RC_OK;
}

u32 vtss_cmn_restart_value_get(vtss_state_t *vtss_state)
{
    return (VTSS_ENCODE_BITFIELD(vtss_state->restart_cur,
                                 VTSS_RESTART_TYPE_OFFSET,
                                 VTSS_RESTART_TYPE_WIDTH) |
            VTSS_ENCODE_BITFIELD(VTSS_API_VERSION,
                                 VTSS_RESTART_VERSION_OFFSET,
                                 VTSS_RESTART_VERSION_WIDTH));
}

#if defined(VTSS_FEATURE_PORT_CONTROL)  ||  defined(VTSS_ARCH_SERVAL)
/* Rebase 64-bit counter, i.e. discard changes since last update, based on 32-bit chip counter */
void vtss_cmn_counter_32_rebase(u32 new_base_value, vtss_chip_counter_t *counter)
{
    counter->prev = new_base_value;
}

/* Clear/increment 64-bit counter based on 32 bit chip counter */
void vtss_cmn_counter_32_update(u32 value, vtss_chip_counter_t *counter, BOOL clear)
{
    u64 add = 0, new = value;
    
    if (clear) {
        /* Clear counter */
        counter->value = 0;
    } else {
        /* Accumulate counter */
        if (new < counter->prev)
            add = (1ULL<<32); /* Wrapped */
        counter->value += (new + add - counter->prev);
    }
    counter->prev = new;
}

/* Rebase 64-bit counter, i.e. discard changes since last update, based on 40-bit chip counter */
void vtss_cmn_counter_40_rebase(u32 new_lsb, u32 new_msb, vtss_chip_counter_t *counter)
{
    counter->prev = new_msb;
    counter->prev = ((counter->prev << 32) + new_lsb);
}

/* Clear/increment 64-bit counter based on 40 bit chip counter */
void vtss_cmn_counter_40_update(u32 lsb, u32 msb, vtss_chip_counter_t *counter, BOOL clear)
{
    u64 add = 0, new = msb;

    new = ((new << 32) + lsb);
    if (clear) {
        /* Clear counter */
        counter->value = 0;
    } else {
        /* Accumulate counter */
        if (new < counter->prev)
            add = (1ULL<<40); /* Wrapped */
        counter->value += (new + add - counter->prev);
    }
    counter->prev = new;
}
#endif

/* ================================================================= *
 *  Debug print
 * ================================================================= */

const char *vtss_bool_txt(BOOL enabled)
{
    return (enabled ? "Enabled" : "Disabled");
}

void vtss_debug_print_header_underlined(const vtss_debug_printf_t pr,
                                        const char                *header,
                                        BOOL                      layer)
{
    int i, len = strlen(header);

    pr("%s\n", header);
    for (i = 0; i < len; i++)
        pr(layer ? "=" : "-");
    pr("\n\n");
}

void vtss_debug_print_header(const vtss_debug_printf_t pr,
                             const char                *header)
{
    pr("%s:\n\n", header);
}

static const char *const vtss_debug_group_name[VTSS_DEBUG_GROUP_COUNT] = {
    [VTSS_DEBUG_GROUP_ALL]       = "All",       /**< All groups */
    [VTSS_DEBUG_GROUP_INIT]      = "Initialization",
    [VTSS_DEBUG_GROUP_MISC]      = "Miscellaneous",
    [VTSS_DEBUG_GROUP_PORT]      = "Port",
    [VTSS_DEBUG_GROUP_PORT_CNT]  = "Port Counters",
    [VTSS_DEBUG_GROUP_PHY]       = "PHY",
    [VTSS_DEBUG_GROUP_VLAN]      = "VLAN",
    [VTSS_DEBUG_GROUP_PVLAN]     = "PVLAN",
    [VTSS_DEBUG_GROUP_MAC_TABLE] = "MAC Table",
    [VTSS_DEBUG_GROUP_ACL]       = "ACL",
    [VTSS_DEBUG_GROUP_QOS]       = "QoS",
    [VTSS_DEBUG_GROUP_AGGR]      = "Aggregation",
    [VTSS_DEBUG_GROUP_GLAG]      = "Global aggregation",
    [VTSS_DEBUG_GROUP_STP]       = "Spanning Tree",
    [VTSS_DEBUG_GROUP_MIRROR]    = "Mirroring",
    [VTSS_DEBUG_GROUP_EVC]       = "EVC",
    [VTSS_DEBUG_GROUP_ERPS]      = "ERPS",
    [VTSS_DEBUG_GROUP_EPS]       = "EPS",
    [VTSS_DEBUG_GROUP_PACKET]    = "Packet",
    [VTSS_DEBUG_GROUP_FDMA]      = "FDMA",
    [VTSS_DEBUG_GROUP_TS]        = "Timestamping",
    [VTSS_DEBUG_GROUP_WM]        = "Watermarks",
    [VTSS_DEBUG_GROUP_LRN]       = "LRN:COMMON",
    [VTSS_DEBUG_GROUP_IPMC]      = "IP Multicast",
    [VTSS_DEBUG_GROUP_STACK]     = "Stacking",
    [VTSS_DEBUG_GROUP_CMEF]      = "Congestion Management",
    [VTSS_DEBUG_GROUP_HOST]      = "Host Configuration",
#if defined(VTSS_FEATURE_MPLS)
    [VTSS_DEBUG_GROUP_MPLS]      = "MPLS",
    [VTSS_DEBUG_GROUP_MPLS_OAM]  = "MPLS OAM",
#endif
    [VTSS_DEBUG_GROUP_VXLAT]     = "VLAN Translation",
    [VTSS_DEBUG_GROUP_SER_GPIO]  = "Serial GPIO",
#ifdef VTSS_ARCH_DAYTONA
    [VTSS_DEBUG_GROUP_XFI]       = "HSS Data Interface (XFI)",
    [VTSS_DEBUG_GROUP_UPI]       = "Universal PHY Interface (UPI)",
    [VTSS_DEBUG_GROUP_PCS_10GBASE_R] = "PCS 10GBaseR",
    [VTSS_DEBUG_GROUP_MAC10G]    = "MAC10G",
    [VTSS_DEBUG_GROUP_WIS]       = "WIS",
    [VTSS_DEBUG_GROUP_RAB]       = "Rate Adaptation Block (RAB)",
    [VTSS_DEBUG_GROUP_XAUI]      = "PCS XAUI Interface",
    [VTSS_DEBUG_GROUP_OTN]       = "OTN",
    [VTSS_DEBUG_GROUP_GFP]       = "GFP",
#endif /* VTSS_ARCH_DAYTONA */
#if defined(VTSS_FEATURE_OAM)
    [VTSS_DEBUG_GROUP_OAM]       = "OAM",
#endif
#if defined(VTSS_SW_OPTION_L3RT) || defined(VTSS_ARCH_JAGUAR_1)
    [VTSS_DEBUG_GROUP_L3]        = "L3",
#endif /* VTSS_SW_OPTION_L3RT || VTSS_ARCH_JAGUAR_1*/
#if defined(VTSS_FEATURE_AFI_SWC)
    [VTSS_DEBUG_GROUP_AFI]       = "AFI",
#endif /* VTSS_FEATURE_AFI_SWC */
#if defined(VTSS_FEATURE_MACSEC)
    [VTSS_DEBUG_GROUP_MACSEC]    = "MacSec",
#endif /* VTSS_FEATURE_MACSEC */
};

BOOL vtss_debug_group_enabled(const vtss_debug_printf_t pr,
                              const vtss_debug_info_t *const info,
                              const vtss_debug_group_t group)
{
    if (info->group == VTSS_DEBUG_GROUP_ALL || info->group == group) {
        vtss_debug_print_header_underlined(pr, 
                                           group < VTSS_DEBUG_GROUP_COUNT ? 
                                           vtss_debug_group_name[group] : "?", 0);
        return 1;
    }
    return 0;
}

vtss_rc vtss_debug_print_group(const vtss_debug_group_t group,
                               vtss_rc (* dbg)(vtss_state_t *vtss_state,
                                               const vtss_debug_printf_t pr, 
                                               const vtss_debug_info_t   *const info),
                               vtss_state_t *vtss_state,
                               const vtss_debug_printf_t pr,
                               const vtss_debug_info_t *const info)
{
    if (!vtss_debug_group_enabled(pr, info, group))
        return VTSS_RC_OK;

    return dbg(vtss_state, pr, info);
}

void vtss_debug_print_sticky(const vtss_debug_printf_t pr, 
                             const char *name, u32 value, u32 mask)
{
    pr("%-32s: %u\n", name, VTSS_BOOL(value & mask));
}

void vtss_debug_print_value(const vtss_debug_printf_t pr, const char *name, u32 value)
{
    pr("%-32s: %u\n", name, value);
}

void vtss_debug_print_reg_header(const vtss_debug_printf_t pr, const char *name)
{
    pr("%-32s  31    24.23    16.15     8.7      0 Hex\n", name);
}

void vtss_debug_print_reg(const vtss_debug_printf_t pr, const char *name, u32 value)
{
    u32 i;
    
    pr("%-32s: ", name);
    for (i = 0; i < 32; i++) {
        pr("%s%u", i == 0 || (i % 8) ? "" : ".", value & (1 << (31 - i)) ? 1 : 0);
    }
    pr(" 0x%08x\n", value);
}

static void vtss_debug_print_init(vtss_state_t *vtss_state,
                                  const vtss_debug_printf_t pr,
                                  const vtss_debug_info_t   *const info)
{
    if (!vtss_debug_group_enabled(pr, info, VTSS_DEBUG_GROUP_INIT))
        return;

    pr("Target    : 0x%04X\n", vtss_state->create.target);
#if defined(VTSS_FEATURE_PORT_MUX)
    pr("Mux mode  : 0x%04x\n", vtss_state->init_conf.mux_mode);
#endif /* VTSS_FEATURE_PORT_MUX */
    pr("State Size: %zu\n\n", sizeof(*vtss_state));
}

/* Print port header, e.g, "0      7.8     15.16    23.24  28" */
void vtss_debug_print_port_header(vtss_state_t *vtss_state,
                                  const vtss_debug_printf_t pr,
                                  const char *txt, u32 count, BOOL nl)
{
    u32 i, port;

    if (count == 0)
        count = vtss_state->port_count;
    if (txt != NULL)
        pr("%s", txt);
    for (port = 0; port < count; port++) {
        i = (port & 7);
        if (i == 0 || i == 7 || (port == (count - 1) && i > 2))
            pr ("%s%u", port != 0 && i == 0 ? "." : "", port);
        else if (port < 10 || i > 2)
            pr(" ");
    }
    if (nl)
        pr("\n");
}

void vtss_debug_print_ports(vtss_state_t *vtss_state,
                            const vtss_debug_printf_t pr, u8 *member, BOOL nl)
{
    vtss_port_no_t port_no;

    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
        pr("%s%s",
           port_no == 0 || (port_no & 7) ? "" : ".",
           VTSS_PORT_BF_GET(member, port_no) ? "1" : "0");
    }
    if (nl)
        pr("\n");
}

void vtss_debug_print_port_members(vtss_state_t *vtss_state,
                                   const vtss_debug_printf_t pr,
                                   BOOL port_member[VTSS_PORT_ARRAY_SIZE],
                                   BOOL nl)
{
    vtss_port_no_t port_no;
    u8             member[VTSS_PORT_BF_SIZE];
    
    VTSS_PORT_BF_CLR(member);
    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++)
        VTSS_PORT_BF_SET(member, port_no, port_member[port_no]);
    vtss_debug_print_ports(vtss_state, pr, member, nl);
}

#if defined(VTSS_SW_OPTION_L3RT) || defined(VTSS_ARCH_JAGUAR_1)
/* implemented in vtss_l3.c */
void vtss_debug_print_l3(vtss_state_t *vtss_state,
                         const vtss_debug_printf_t pr,
                         const vtss_debug_info_t   *const info);
#endif /* VTSS_SW_OPTION_L3RT || VTSS_ARCH_JAGUAR_1*/

#if defined(VTSS_FEATURE_AFI_SWC)
static void vtss_debug_print_afi(vtss_state_t *vtss_state,
                                 const vtss_debug_printf_t pr,
                                 const vtss_debug_info_t   *const info)
{
    u32 slot, cnt = 0;

    if (!vtss_debug_group_enabled(pr, info, VTSS_DEBUG_GROUP_AFI))
        return;

    // Print the DCB layout of the frames that are currently being injected.
    pr("Automatic Frame Injector\n");
    pr("ID   State    Frames/sec Port Timer\n");
    pr("---- -------- ---------- ---- -----\n");

    for (slot = 0; slot < VTSS_ARRSZ(vtss_state->afi_slots); slot++) {
        vtss_afi_slot_conf_t  *slot_conf = &vtss_state->afi_slots[slot];
        vtss_afi_timer_conf_t *timer_conf;

        if (slot_conf->state == VTSS_AFI_SLOT_STATE_FREE) {
            continue;
        }

        cnt++;
        timer_conf = &vtss_state->afi_timers[slot_conf->timer_idx];

        pr("%4u %-8s %10u %4u %5u\n",
           slot,
           slot_conf->state == VTSS_AFI_SLOT_STATE_RESERVED ? "Reserved" :
           slot_conf->state == VTSS_AFI_SLOT_STATE_ENABLED  ? "Enabled"  :
           slot_conf->state == VTSS_AFI_SLOT_STATE_PAUSED   ? "Paused"   : "Unknown",
           timer_conf->fps,
           slot_conf->port_no,
           slot_conf->timer_idx);
    }

    if (cnt == 0) {
        pr("<none>\n");
    }
    pr("\n");
}
#endif

/* Print Application Interface Layer state */
static vtss_rc vtss_debug_ail_print(vtss_state_t *vtss_state,
                                    const vtss_debug_printf_t pr,
                                    const vtss_debug_info_t   *const info)
{
    if (info->layer != VTSS_DEBUG_LAYER_ALL && info->layer != VTSS_DEBUG_LAYER_AIL)
        return VTSS_RC_OK;

    vtss_debug_print_header_underlined(pr, "Application Interface Layer", 1);

    vtss_debug_print_init(vtss_state, pr, info);

#if defined(VTSS_FEATURE_MISC)
    vtss_misc_debug_print(vtss_state, pr, info);
#endif /* VTSS_FEATURE_MISC */

#if defined(VTSS_CHIP_CU_PHY)
    VTSS_RC(vtss_phy_debug_info_print(vtss_state, pr, info, TRUE, FALSE, TRUE));
#endif

#if defined(VTSS_CHIP_10G_PHY)
    VTSS_RC(vtss_phy_10g_debug_info_print(vtss_state, pr, info, 1));
#endif /* VTSS_CHIP_10G_PHY */

#if defined(VTSS_FEATURE_PORT_CONTROL)
    vtss_port_debug_print(vtss_state, pr, info);
#endif /* VTSS_FEATURE_PORT_CONTROL */

#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
    if(info->group == VTSS_DEBUG_GROUP_ALL || info->group == VTSS_DEBUG_GROUP_PHY_TS)
	vtss_phy_ts_api_debug_print(vtss_state, pr, info);
#endif /* VTSS_FEATURE_PHY_TIMESTAMP */

#if defined(VTSS_FEATURE_LAYER2)
    vtss_l2_debug_print(vtss_state, pr, info);
#endif /* VTSS_FEATURE_LAYER2 */

#if defined(VTSS_FEATURE_ACL)
    vtss_vcap_debug_print_acl(vtss_state, pr, info);
#endif /* VTSS_FEATURE_ACL */

#if defined(VTSS_FEATURE_QOS)
    vtss_qos_debug_print(vtss_state, pr, info);
#endif /* VTSS_FEATURE_QOS */

#if defined(VTSS_FEATURE_EVC) || defined(VTSS_FEATURE_QOS_POLICER_DLB)
    if (vtss_debug_group_enabled(pr, info, VTSS_DEBUG_GROUP_EVC))
#if defined(VTSS_FEATURE_EVC)
        vtss_evc_debug_print(vtss_state, pr, info);
#else
        vtss_qos_debug_print_dlb(vtss_state, pr, info);
#endif
#endif /* VTSS_FEATURE_EVC || VTSS_FEATURE_QOS_POLICER_DLB*/

#if defined(VTSS_FEATURE_PACKET)
    vtss_packet_debug_print(vtss_state, pr, info);
#endif /* VTSS_FEATURE_PACKET */

#if defined(VTSS_FEATURE_TIMESTAMP)
    vtss_ts_debug_print(vtss_state, pr, info);
#endif /* VTSS_FEATURE_TIMESTAMP */

#if defined(VTSS_FEATURE_OAM)
    vtss_oam_debug_print(vtss_state, pr, info);
#endif
    
#if defined(VTSS_SW_OPTION_L3RT) && defined(VTSS_ARCH_JAGUAR_1)
    if (vtss_debug_group_enabled(pr, info, VTSS_DEBUG_GROUP_L3))
        vtss_debug_print_l3(vtss_state, pr, info);
#endif /* VTSS_SW_OPTION_L3RT || VTSS_ARCH_JAGUAR_1*/

#if defined(VTSS_FEATURE_FDMA) && VTSS_OPT_FDMA
    if (vtss_debug_group_enabled(pr, info, VTSS_DEBUG_GROUP_FDMA)) {
        VTSS_RC(vtss_fdma_cmn_debug_print_fdma(vtss_state, pr, info));
    }
#endif

#if defined(VTSS_FEATURE_MPLS)
    vtss_mpls_debug_print(vtss_state, pr, info);
#endif /* VTSS_FEATURE_MPLS */

#if defined(VTSS_FEATURE_AFI_SWC)
    vtss_debug_print_afi(vtss_state, pr, info);
#endif

    return VTSS_RC_OK;
}

/* Print Chip Interface Layer state */
static vtss_rc vtss_debug_cil_print(vtss_state_t *vtss_state,
                                    const vtss_debug_printf_t pr,
                                    const vtss_debug_info_t   *const info)
{
    vtss_rc        rc = VTSS_RC_OK;
    char           buf[80];
    vtss_chip_no_t chip_no;

    if (info->layer != VTSS_DEBUG_LAYER_ALL && info->layer != VTSS_DEBUG_LAYER_CIL)
        return VTSS_RC_OK;

    /* Print CIL information for all devices */
    for (chip_no = 0; chip_no < vtss_state->chip_count; chip_no++) {
        if (info->chip_no != VTSS_CHIP_NO_ALL && chip_no != info->chip_no)
            continue;
        VTSS_SELECT_CHIP(chip_no);
        sprintf(buf, "Chip Interface Layer[%u]", chip_no);
        vtss_debug_print_header_underlined(pr, buf, 1);
#if defined(VTSS_CHIP_CU_PHY)
        VTSS_RC(vtss_phy_debug_info_print(vtss_state, pr, info, FALSE, FALSE, TRUE));
#endif
#if defined(VTSS_CHIP_10G_PHY)
        VTSS_RC(vtss_phy_10g_debug_info_print(vtss_state, pr, info, 0));
#endif
#if defined(VTSS_FEATURE_MACSEC)
        VTSS_RC(vtss_debug_print_macsec(vtss_state, pr, info));
#endif
        rc = VTSS_FUNC(cil.debug_info_print, pr, info);
    }
    return rc;
}

vtss_rc vtss_cmn_debug_info_print(vtss_state_t *vtss_state,
                                  const vtss_debug_printf_t pr,
                                  const vtss_debug_info_t   *const info)
{
    VTSS_RC(vtss_debug_ail_print(vtss_state, pr, info));
    return vtss_debug_cil_print(vtss_state, pr, info);
}

/* This function snoops the last X calls to reg_read()/reg_write()
 * in a circular buffer. It also checks whether an access to the switch
 * core fails, and if so, prints an error message.
 */
static vtss_rc vtss_cmn_reg_check_access(const vtss_chip_no_t chip_no,
                                         const u32            addr,
                                         u32                  *const value,
                                         BOOL                 is_read)
{
    // Only default instance works.
    vtss_state_t            *state = (vtss_state_t *)vtss_default_inst;
    u32                     new_err_cnt;
    // We need to lock the scheduler here so that the accesses to the user address and
    // to the error counter address are indivisible.
    /*lint --e{529} */ // Avoid "Symbol 'flags' not subsequently referenced" Lint warning
    VTSS_OS_SCHEDULER_FLAGS flags = 0;
    vtss_rc rc;

    VTSS_OS_SCHEDULER_LOCK(flags);

    if (is_read) {
        rc = state->reg_check.orig_read(chip_no, addr, value);
    } else {
        rc = state->reg_check.orig_write(chip_no, addr, *value);
    }

    if (chip_no != 0 || rc != VTSS_RC_OK) {
        // We only support the register access checking on chip #0,
        // and only if the user-access went OK.
        goto do_exit;
    }

    // Save the address and access type of the register that is attempted accessed in a circular buffer.
    state->reg_check.last_accesses[state->reg_check.last_access_idx  ].addr    = addr;
    state->reg_check.last_accesses[state->reg_check.last_access_idx++].is_read = is_read;
    if (state->reg_check.last_access_idx >= VTSS_ARRSZ(state->reg_check.last_accesses)) {
        state->reg_check.last_access_idx = 0;
    }

    if (state->reg_check.orig_read(chip_no, state->reg_check.addr, &new_err_cnt) != VTSS_RC_OK) {
        VTSS_EG(VTSS_TRACE_GROUP_REG_CHECK, "Unable to read error counter address");
        goto do_exit; // Don't modify rc.
    }

    if (new_err_cnt != state->reg_check.last_error_cnt) {
        u32 i, cnt = 0;

        // Use VTSS_EG(), which may be called in both ISR, DSR; and thread context.
        VTSS_EG(VTSS_TRACE_GROUP_REG_CHECK, "Error when %sing 0x%06x (err_cnt = 0x%x):\n  ", is_read ? "read" : "writ", 4U * addr, new_err_cnt);
        for (i = state->reg_check.last_access_idx; cnt != VTSS_ARRSZ(state->reg_check.last_accesses); i++, cnt++) {
            if (i >= VTSS_ARRSZ(state->reg_check.last_accesses)) {
                i = 0;
            }
            if (state->reg_check.last_accesses[i].addr != 0) {
                VTSS_EG(VTSS_TRACE_GROUP_REG_CHECK, "[%2u] = %c(0x%06x) ", cnt, state->reg_check.last_accesses[i].is_read ? 'R' : 'W', 4U * state->reg_check.last_accesses[i].addr);
                state->reg_check.last_accesses[i].addr = 0;
            }
        }
        state->reg_check.last_error_cnt = new_err_cnt;
    }

do_exit:
    VTSS_OS_SCHEDULER_UNLOCK(flags);
    return rc;
}

static vtss_rc vtss_cmn_debug_reg_check_read(const vtss_chip_no_t chip_no,
                                             const u32            addr,
                                                   u32            *const value)
{
    return vtss_cmn_reg_check_access(chip_no, addr, value, TRUE);
}

static vtss_rc vtss_cmn_debug_reg_check_write(const vtss_chip_no_t chip_no,
                                              const u32            addr,
                                              const u32            value)
{
    u32 non_const_value = (u32)value;
    return vtss_cmn_reg_check_access(chip_no, addr, &non_const_value, FALSE);
}

vtss_rc vtss_cmn_debug_reg_check_set(vtss_state_t *state, BOOL enable)
{
    // We need to lock scheduler here when installing new reg_read/write() handlers.
    /*lint --e{529, 438} */ // Avoid "Symbol 'flags' not subsequently referenced" Lint warning
    VTSS_OS_SCHEDULER_FLAGS flags = 0;

    if (state->reg_check.addr == 0) {
        // The CIL layer says: Not supported on this platform.
        VTSS_IG(VTSS_TRACE_GROUP_REG_CHECK, "Register checking is not support on this platform");
        return VTSS_RC_ERROR;
    }

    VTSS_OS_SCHEDULER_LOCK(flags);

    if (enable) {
        if (state->reg_check.disable_cnt > 0) {
            if (--state->reg_check.disable_cnt == 0) {
                state->reg_check.orig_read  = state->init_conf.reg_read;
                state->reg_check.orig_write = state->init_conf.reg_write;
                state->init_conf.reg_read   = vtss_cmn_debug_reg_check_read;
                state->init_conf.reg_write  = vtss_cmn_debug_reg_check_write;
                // Update the error counter.
                (void)state->reg_check.orig_read(0, state->reg_check.addr, &state->reg_check.last_error_cnt);
            }
        }
    } else if (state->reg_check.disable_cnt++ == 0) {
        // Supported on this platform. Restore original handlers.
        state->init_conf.reg_read   = state->reg_check.orig_read;
        state->init_conf.reg_write  = state->reg_check.orig_write;
        state->reg_check.orig_read  = NULL;
        state->reg_check.orig_write = NULL;
    }

    VTSS_OS_SCHEDULER_UNLOCK(flags);
    return VTSS_RC_OK;
}

vtss_rc vtss_cmn_bit_from_one_hot_mask64(u64 mask, u32 *bit_pos)
{
    u32 msw = (u32)(mask >> 32), lsw = (u32)mask;

    // Exactly one bit must be set in #mask.
    if ((msw == 0 && lsw == 0) || (msw != 0 && lsw != 0)) {
        // Either both are 0 or both are non-zero, hence can't be a one-hot.
        goto err;
    }

    if (msw == 0) {
        *bit_pos = VTSS_OS_CTZ(lsw);
        lsw &= ~VTSS_BIT(*bit_pos);
        if (lsw) {
            // Two or more bits set in lsw
            goto err;
        }
    } else {
        *bit_pos = VTSS_OS_CTZ(msw);
        msw &= ~VTSS_BIT(*bit_pos);
        if (msw) {
            // Two or more bits set in msw
            goto err;
        }
        *bit_pos += 32;
    }
    return VTSS_RC_OK;

err:
    VTSS_E("0x%" PRIx64 " is not a one-hot mask", mask);
    return VTSS_RC_ERROR;
}

