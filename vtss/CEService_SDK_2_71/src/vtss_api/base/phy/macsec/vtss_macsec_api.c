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

#include "vtss_api.h"

#if defined(VTSS_FEATURE_MACSEC)

#define VTSS_TRACE_GROUP VTSS_TRACE_GROUP_MACSEC // Must come before #include "vtss_state.h"

#include "../../ail/vtss_state.h"
#include "../../ail/vtss_common.h"
#if defined(VTSS_IOREG)
#undef VTSS_IOREG
#endif
#include "../phy_10g/chips/venice/vtss_venice_regs.h"
#define VTSS_IOREG(dev, is32, off)  _ioreg(&io_prm, (dev), (is32), (off))
#define MACSEC_INGR              0x0000
#define MACSEC_EGR               0x8000
#define FC_BUFFER                0xF000
#define HOST_MAC                 0xF100
#define LINE_MAC                 0xF200
#define PROC_ID_0                0x9000
#define PROC_ID_2                0x9200
#define MACSEC_10G_INGR_LATENCY  60
#define MACSEC_10G_EGR_LATENCY   56
#define MACSEC_1G_INGR_LATENCY   57
#define MACSEC_1G_EGR_LATENCY    40
#define MACSEC_NOT_IN_USE        0xFFFF
#define MACSEC_SECY_NONE         0xFFFF
#define MACSEC_ENABLE            1
#define MACSEC_DISABLE           0
#define INGRESS                  0
#define EGRESS                   1

#define VTSS_MACSEC_ASSERT(x,_txt) if((x)) { VTSS_E("%s",_txt); return VTSS_RC_ERROR;}
#define PST_DIR(front,egr,back) (egr ? front##_MACSEC_EGR_##back : front##_MACSEC_INGR_##back)
#define MACSEC_BS(x) ((x>>8 & 0x00FF) | (x<<8 & 0xFF00))
#define PHY_BASE_PORT(p) (base_port(vtss_state, p))

#define MACADDRESS_FMT "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx"
#define MACADDRESS_ARG(X) (X).addr[0], (X).addr[1], (X).addr[2], \
                          (X).addr[3], (X).addr[4], (X).addr[5]
#define SCI_FMT MACADDRESS_FMT"#%hu"
#define SCI_ARG(X) MACADDRESS_ARG((X).mac_addr), (X).port_id
#define MACSEC_PORT_FMT "%u/%u/%hu"
#define MACSEC_PORT_ARG(X) (X)->port_no, (X)->service_id, (X)->port_id

#define MPORT_SCI_AN_FMT "Port: "MACSEC_PORT_FMT", SCI: "SCI_FMT", an:%u"
#define MPORT_SCI_AN_ARG(P, S, A) MACSEC_PORT_ARG((&P)), SCI_ARG(S), A

#define MPORT_SCI_FMT "Port: "MACSEC_PORT_FMT", SCI: "SCI_FMT
#define MPORT_SCI_ARG(P, S) MACSEC_PORT_ARG((&P)), SCI_ARG(S)

#define MPORT_AN_FMT "Port: "MACSEC_PORT_FMT", an:%u"
#define MPORT_AN_ARG(P, A) MACSEC_PORT_ARG((&P)), A

#define MPORT_FMT "Port: "MACSEC_PORT_FMT
#define MPORT_ARG(P) MACSEC_PORT_ARG((&P))


#define BOOL_ARG(X) (X) ? "TRUE":"FALSE"

#define MACSEC_RX_SC_CONF_FMT                            \
    "{validate_frames:%u, replay_protect:%s, "           \
    "replay_window:%u, confidentiality_offset:%u}"

#define MACSEC_RX_SC_CONF_ARG(X)                         \
    (X)->validate_frames, BOOL_ARG((X)->replay_protect), \
    (X)->replay_window, (X)->confidentiality_offset

#define MACSEC_PORT_STATUS_FMT \
    "{mac_enabled:%s, mac_operational:%s, oper_point_to_point_mac:%s}"

#define MACSEC_PORT_STATUS_ARG(X)             \
    BOOL_ARG((X).mac_enabled),                \
    BOOL_ARG((X).mac_operational),            \
    BOOL_ARG((X).oper_point_to_point_mac)

#define MACSEC_SECY_PORT_STATUS_FMT           \
    "{controlled:"MACSEC_PORT_STATUS_FMT      \
    ", uncontrolled:"MACSEC_PORT_STATUS_FMT   \
    ", common:"MACSEC_PORT_STATUS_FMT"}"

#define MACSEC_SECY_PORT_STATUS_ARG(X)        \
    MACSEC_PORT_STATUS_ARG((X).controlled),   \
    MACSEC_PORT_STATUS_ARG((X).uncontrolled), \
    MACSEC_PORT_STATUS_ARG((X).common)

#define MACSEC_TX_SC_CONF_FMT \
    "{protect_frames:%s, always_include_sci:%s, use_es:%s, use_scb:%s, " \
    "confidentiality_offset:%u}"

#define MACSEC_TX_SC_CONF_ARG(X) \
    BOOL_ARG((X).protect_frames), \
    BOOL_ARG((X).always_include_sci), \
    BOOL_ARG((X).use_es), \
    BOOL_ARG((X).use_scb), \
    (X).confidentiality_offset

#define MACSEC_TX_SC_STATUS_FMT \
    "{sci:"SCI_FMT", transmitting:%s, encoding_sa:%hu, enciphering_sa:%hu, " \
    "created_time:%u, started_time:%u, stopped_time:%u}"


#define MACSEC_TX_SC_STATUS_ARG(X)    \
    SCI_ARG((X).sci),                 \
    BOOL_ARG((X).transmitting),       \
    (X).encoding_sa,                  \
    (X).enciphering_sa,               \
    (X).created_time,                 \
    (X).started_time,                 \
    (X).stopped_time

#define MACSEC_RX_SA_STATUS_FMT                                                \
    "{in_use:%s, next_pn:%u, lowest_pn:%u, created_time:%u, started_time:%u, " \
    "stopped_time:%u}"

#define MACSEC_RX_SA_STATUS_ARG(X) \
    BOOL_ARG((X).in_use),          \
    (X).next_pn,                   \
    (X).lowest_pn,                 \
    (X).created_time,              \
    (X).started_time,              \
    (X).stopped_time

#define MACSEC_TX_SA_STATUS_FMT \
    "{next_pn:%u, created_time:%u, started_time:%u, stopped_time:%u}"

#define MACSEC_TX_SA_STATUS_ARG(X) \
    (X).next_pn, \
    (X).created_time, \
    (X).started_time, \
    (X).stopped_time


static ioreg_blk io_prm;
static ioreg_blk *_ioreg(ioreg_blk *io, u16 dev, BOOL is32, u32 off)
{
    io->mmd = dev;
    io->is32 = is32;
    io->addr = off;
    return io;
}

/* ================================================================= *
 *  Register access
 * ================================================================= */
#define CSR_RD(p, io_reg, value)     \
    {   \
       vtss_rc __rc = _csr_rd(vtss_state, p, io_reg, value); \
        if (__rc != VTSS_RC_OK)           \
            return __rc;                  \
    }


#define CSR_WR(p, io_reg, value)                 \
    {\
      vtss_rc __rc = _csr_wr(vtss_state, p, io_reg, value); \
        if (__rc != VTSS_RC_OK)           \
            return __rc;                  \
    }

#define CSR_WRM(p, io_reg, value, mask) \
    { \
        vtss_rc __rc = _csr_wrm(vtss_state, p, io_reg, value, mask); \
        if (__rc != VTSS_RC_OK)                  \
            return __rc;                         \
    }

static vtss_rc phy_type_get(vtss_state_t *vtss_state,
                            const vtss_port_no_t port_no, BOOL *const clause45)
{

#ifdef VTSS_CHIP_CU_PHY
    if (vtss_state->phy_state[port_no].type.part_number != VTSS_PHY_TYPE_NONE) {
        *clause45 = 0;
        return VTSS_RC_OK;
    }
#endif

#ifdef VTSS_CHIP_10G_PHY
    if (vtss_state->phy_10g_state[port_no].type != VTSS_PHY_TYPE_10G_NONE) {
        *clause45 = 1;
        return VTSS_RC_OK;
    }
#endif
    return VTSS_RC_ERROR;
}

#define PRINTF(...)                                         \
    if (size - s > 0) {                                     \
        int res = snprintf(buf + s, size - s, __VA_ARGS__); \
        if (res >0 ) {                                      \
            s += res;                                       \
        }                                                   \
    }

int vtss_macsec_match_pattern_to_txt(char                              *buf,
                                     int                                size,
                                     const vtss_macsec_match_pattern_t *const p)
{
    int s = 0;
    PRINTF("{priority:%u", p->priority);

    if (p->match & VTSS_MACSEC_MATCH_IS_CONTROL) {
        PRINTF(" is_control:%s", p->is_control ? "true" : "false");
    }

    if (p->match & VTSS_MACSEC_MATCH_HAS_VLAN) {
        PRINTF(" has_vlan:%s", p->has_vlan_tag ? "true" : "false");
    }

    if (p->match & VTSS_MACSEC_MATCH_HAS_VLAN_INNER) {
        PRINTF(" has_vlan_inner:%s", p->has_vlan_inner_tag ? "true" : "false");
    }

    if (p->match & VTSS_MACSEC_MATCH_ETYPE) {
        PRINTF(" etype:0x%04x", p->etype);
    }

    if (p->match & VTSS_MACSEC_MATCH_VLAN_ID) {
        PRINTF(" vlan:0x%04x", p->vid);
    }

    if (p->match & VTSS_MACSEC_MATCH_VLAN_ID_INNER) {
        PRINTF(" vlan_inner:0x%04x", p->vid_inner);
    }

    if (p->match & VTSS_MACSEC_MATCH_BYPASS_HDR) {
        int i;

        PRINTF(" mpls_hdr:");
        for (i = 0; i < 8; ++i) {
            PRINTF(" %02x", p->hdr[i]);
        }
        PRINTF(" mpls_hdr_mask:");
        for (i = 0; i < 8; ++i) {
            PRINTF(" %02x", p->hdr_mask[i]);
        }
    }

    PRINTF("}");

    return s;
}
int vtss_macsec_frame_match_to_txt(char                                         *buf,
                                   int                                          size,
                                   const vtss_macsec_control_frame_match_conf_t *const p)
{
    int s = 0;

    PRINTF("{");
    if (p->match & VTSS_MACSEC_MATCH_ETYPE) {
        PRINTF(" etype:0x%04x", p->etype);
    }

    if (p->match & VTSS_MACSEC_MATCH_DMAC) {
        PRINTF(" dmac:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx",
               p->dmac.addr[0], p->dmac.addr[1], p->dmac.addr[2], p->dmac.addr[3], p->dmac.addr[4], p->dmac.addr[5]);
    }

    PRINTF("}");

    return s;
}

#undef PRINTF


static BOOL phy_is_1g(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
    BOOL                       clause45 = FALSE;
    vtss_rc __rc__ = phy_type_get(vtss_state, port_no, &clause45);
    if (__rc__ < VTSS_RC_OK) {
        VTSS_D("phy_type_get returned %x", __rc__);
    }
    return !clause45;
}

static vtss_port_no_t base_port(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
#ifdef VTSS_CHIP_10G_PHY
    return vtss_state->phy_10g_state[port_no].phy_api_base_no;
#else
    return port_no;
#endif
}

static vtss_rc get_base_adr(u32 dev, u32 addr, u32 *phy10g_base, u32 *target_id, u32 *offset)
{
    *target_id = 0;
    if (dev == 1) {
        if (addr >= 0xF000) {
            *phy10g_base = 0xF000;
            *offset =  0xF000;
        }
    } else if (dev == 3) {
        if ((addr >= FC_BUFFER) && (addr < 0xF07F)) {
            *phy10g_base = FC_BUFFER;
            *offset = addr - FC_BUFFER;
            *target_id = 4;
        } else if ((addr >= HOST_MAC) && (addr < 0xF17F)) {
            *phy10g_base = HOST_MAC;
            *offset = addr - HOST_MAC;
            *target_id = 5;
        } else if ((addr >= LINE_MAC) && (addr < 0xF27F)) {
            *phy10g_base = LINE_MAC;
            *offset = addr - LINE_MAC;
            *target_id = 6;
        }
    } else if (dev == 30) {
        if ((addr >= PROC_ID_0) && (addr < 0x91FF)) {
            *phy10g_base = PROC_ID_0;
            *offset = addr - PROC_ID_0;
            *target_id = 0xe;
        } else if ((addr >= PROC_ID_2) && (addr < 0x93FF)) {
            *phy10g_base = PROC_ID_2;
            *offset = addr - PROC_ID_2;
            *target_id = 0xf;
        } else if ((addr >= 0xA000) && (addr < 0xA7FF)) {
            *phy10g_base = 0xA000;
            *offset = addr - 0xA000;
        } else if ((addr >= 0xA800) && (addr < 0xAFFF)) {
            *phy10g_base = 0xA800;
            *offset = addr - 0xA800;
        } else if ((addr >= 0xB000) && (addr < 0xB7FF)) {
            *phy10g_base = 0xB000;
            *offset = addr - 0xB000;
        } else if ((addr >= 0xB800) && (addr < 0xBFFF)) {
            *phy10g_base = 0xB800;
            *offset = addr - 0xB800;
        } else if ((addr >= 0xC000) && (addr < 0xC7FF)) {
            *phy10g_base = 0xC000;
            *offset = addr - 0xC000;
        } else if ((addr >= 0xC800) && (addr < 0xCFFF)) {
            *phy10g_base = 0xC800;
            *offset = addr - 0xC800;
        } else if ((addr >= 0xF000) && (addr < 0xF07F)) {
            *phy10g_base = 0xF000;
            *offset = addr - 0xF000;
        }
    } else if (dev == 31) {
        if (addr < MACSEC_EGR) {
            *phy10g_base = MACSEC_INGR;
            *offset = addr - MACSEC_INGR;
            *target_id = 0x38;
        } else {
            *phy10g_base = MACSEC_EGR;
            *offset = addr - MACSEC_EGR;
            *target_id = 0x3c;
        }
    } else {
        return VTSS_RC_ERROR;
    }

    return VTSS_RC_OK;
}

static vtss_rc csr_rd(vtss_state_t *vtss_state, vtss_port_no_t port_no, u16 mmd, BOOL is32, u32 addr, u32 *value)
{
    BOOL                       clause45 = FALSE;
    u32                        reg_addr = 0;
    u16                        reg_value[2];
    u32                        offset, target, base_addr;

    VTSS_RC(phy_type_get(vtss_state, port_no, &clause45));
    VTSS_RC(get_base_adr(mmd, addr, &base_addr, &target, &offset));

    if (clause45) {
        vtss_mmd_read_t     mmd_read_func = vtss_state->init_conf.mmd_read;
        vtss_mmd_read_inc_t mmd_read_inc_func = vtss_state->init_conf.mmd_read_inc;
        vtss_port_no_t p = (mmd == 0x1e) ? PHY_BASE_PORT(port_no) : port_no;

        if (is32) {
            reg_addr = base_addr | (offset << 1);
            VTSS_RC(mmd_read_inc_func(vtss_state, p, mmd, reg_addr, reg_value, 2));
            *value = reg_value[0] + (((u32)reg_value[1]) << 16);
        } else {
            VTSS_RC(mmd_read_func(vtss_state, p, mmd, addr, &reg_value[0]));
            *value = (u32)reg_value[0];
        }
    } else {
#ifdef VTSS_CHIP_CU_PHY
        VTSS_RC(vtss_phy_macsec_csr_rd_private(vtss_state, port_no, target, offset, value));
#endif /* VTSS_CHIP_CU_PHY */
    }

    return VTSS_RC_OK;
}


static vtss_rc csr_wr(vtss_state_t *vtss_state, vtss_port_no_t port_no, u16 mmd, BOOL is32, u32 addr, u32 value)
{

    u16               reg_value_upper, reg_value_lower;
    BOOL              clause45 = FALSE;
    u32               offset, target, base_addr, reg_addr;

    VTSS_RC(phy_type_get(vtss_state, port_no, &clause45));
    VTSS_RC(get_base_adr(mmd, addr, &base_addr, &target, &offset));

    /* Divide the 32 bit value to [15..0] Bits & [31..16] Bits */
    reg_value_lower = (value & 0xffff);
    reg_value_upper = (value >> 16);

    if (clause45) {
        vtss_mmd_write_t  mmd_write_func = vtss_state->init_conf.mmd_write;
        vtss_port_no_t p = (mmd == 0x1e) ? PHY_BASE_PORT(port_no) : port_no;
        if (is32) {
            reg_addr = base_addr | (offset << 1);
            /* Write the Upper 2 Bytes */
            VTSS_RC(mmd_write_func(vtss_state, p, mmd, (reg_addr | 1), reg_value_upper));
            /* Write the Lower 2 Bytes */
            VTSS_RC(mmd_write_func(vtss_state, p, mmd, reg_addr, reg_value_lower));
        } else {
            VTSS_RC(mmd_write_func(vtss_state, p, mmd, addr, reg_value_lower));
        }
    } else {
        // 1G PHY access
#ifdef VTSS_CHIP_CU_PHY
        VTSS_RC(vtss_phy_macsec_csr_wr_private(vtss_state, port_no, target, offset, value));
#endif /* VTSS_CHIP_CU_PHY */
    }
    return VTSS_RC_OK;
}

static vtss_rc csr_wrm(vtss_state_t *vtss_state, vtss_port_no_t port_no, u16 mmd, BOOL is32, u32 addr, u32 value, u32 mask)
{
    vtss_rc rc;
    u32     val;

    if ((rc = csr_rd(vtss_state, port_no, mmd, is32, addr, &val)) == VTSS_RC_OK) {
        val = ((val & ~mask) | (value & mask));
        rc = csr_wr(vtss_state, port_no, mmd, is32, addr, val);
    }
    return rc;
}

static vtss_rc _csr_rd(vtss_state_t *vtss_state, vtss_port_no_t port_no, ioreg_blk *io, u32 *value)
{
    return csr_rd(vtss_state, port_no, io->mmd, io->is32, io->addr, value);
}

static vtss_rc _csr_wr(vtss_state_t *vtss_state, vtss_port_no_t port_no, ioreg_blk *io, u32 value)
{
    return csr_wr(vtss_state, port_no, io->mmd, io->is32, io->addr, value);
}

static vtss_rc _csr_wrm(vtss_state_t *vtss_state, vtss_port_no_t port_no, ioreg_blk *io, u32 value, u32 mask)
{
    return csr_wrm(vtss_state, port_no, io->mmd, io->is32, io->addr, value, mask);
}

/* ================================================================= *
 *  Local functions
 * ================================================================= */

// Return VTSS_RC if the sci is valid, else error code.
static vtss_rc is_sci_valid(const vtss_macsec_sci_t *sci)
{
    vtss_mac_t boardcast = {.addr = MAC_ADDR_BROADCAST};

    // IEEE 802.1AE-2006, section 9.9 - The 64-bit value FF-FF-FF-FF-FF-FF is never used as an SCI and is reserved for use by implementations to indicate the absence of an SC or an SCI in contexts where an SC can be present */
    if (memcmp(&sci->mac_addr, &boardcast, sizeof(boardcast)) == 0) {
        VTSS_I("Broadcast MAC address should not be used");
        return VTSS_RC_ERR_MACSEC_INVALID_SCI_MACADDR;
    }

    return VTSS_RC_OK;
}

static BOOL sci_cmp(const vtss_macsec_sci_t *a, const vtss_macsec_sci_t *b)
{
    u32 i;

    if (b == NULL || a == NULL) {
        return FALSE;
    }

    if (a->port_id != b->port_id) {
        return FALSE;
    }

    for (i = 0; i < 6; ++i) {
        if (a->mac_addr.addr[i] != b->mac_addr.addr[i]) {
            return FALSE;
        }
    }
    return TRUE;
}

static BOOL sci_larger(const vtss_macsec_sci_t *a, const vtss_macsec_sci_t *b)
{
    u32 i;
    if (b == NULL) {
        return TRUE;
    }
    for (i = 0; i < 6; i++) {
        if (a->mac_addr.addr[i] > b->mac_addr.addr[i]) {
            return TRUE;
        } else if (a->mac_addr.addr[i] < b->mac_addr.addr[i]) {
            return FALSE;
        }
    }
    if (a->port_id > b->port_id) {
        return TRUE;
    }
    return FALSE;
}


static void sci_cpy(vtss_macsec_sci_t *const dst, const vtss_macsec_sci_t *const src)
{
    u32 i;
    dst->port_id = src->port_id;
    for (i = 0; i < 6; ++i) {
        dst->mac_addr.addr[i] = src->mac_addr.addr[i];
    }
}

static void prnt_sak(const vtss_macsec_sak_t *const sak)
{
    u32 i;
    char buf[100];
    char h_buf[100];

    memset(buf, 0, sizeof(*buf) * 100);
    memset(h_buf, 0, sizeof(*h_buf) * 100);
    for (i = 0; i < sak->len; i++) {
        sprintf(buf, "%s%s%02hhx", buf, i == 0 ? "" : ":", sak->buf[i]);
        if (i < 16) {
            sprintf(h_buf, "%s%s%02hhx", h_buf, i == 0 ? "" : ":", sak->h_buf[i]);
        }
    }
    VTSS_I("buf:%s", buf);
    VTSS_I("h_buf:%s", h_buf);
}

typedef struct {
    u32 secy_id;
    u32 next_index;
} macsec_secy_in_use_iter_t;

static void macsec_secy_in_use_inter_init(macsec_secy_in_use_iter_t *in_use_inter)
{
    in_use_inter->secy_id = 0;
    in_use_inter->next_index    = 0;
}


static BOOL macsec_secy_in_use_inter_getnext(vtss_state_t        *vtss_state,
                                             const vtss_port_no_t      port_no,
                                             macsec_secy_in_use_iter_t *in_use_inter)
{
    u32 i;
    vtss_macsec_internal_secy_t *secy;
    for (i = in_use_inter->next_index; i < VTSS_MACSEC_MAX_SC_RX; i++) {
        in_use_inter->next_index++;
        secy = &vtss_state->macsec_conf[port_no].secy[i];
        if (!secy->in_use) {
            continue;
        }
        if (secy->rx_sc[i] == NULL) {
            continue;
        }
        if (secy->rx_sc[i]->in_use) {
            in_use_inter->secy_id = i;
            return TRUE;
        }
    }
    return FALSE; // Indicate no more secy's that is in use
}


static vtss_rc sc_from_sci_get(vtss_macsec_internal_secy_t *secy, const vtss_macsec_sci_t *sci, u32 *sc)
{
    u32 i;
    for (i = 0; i < VTSS_MACSEC_MAX_SC_RX; i++) {
        if (secy->rx_sc[i] == NULL) {
            continue;
        }
        if (secy->rx_sc[i]->in_use) {
            if (sci_cmp(sci, &secy->rx_sc[i]->sci)) {
                *sc = i;
                return VTSS_RC_OK;
            }
        }
    }
    VTSS_I("Error-i:%u", i);
    return VTSS_RC_ERR_MACSEC_NO_SCI;
}

static u32 get_u32(const u8 *const buf)
{
    return *(buf) | (*(buf + 1) << 8) | (*(buf + 2) << 16) | (*(buf + 3) << 24);
}

static BOOL check_resources(vtss_state_t *vtss_state,
                            vtss_port_no_t port_no, BOOL is_sc, u32 secy_id)
{
    vtss_macsec_internal_secy_t *secy;
    u32 num_of_sa_rsrv = 0, num_of_rx_sa_inuse = 0, i, sc, sa;
    BOOL found_sc = 0, found_sa = 0, secy_in_use = 0;
    u32 macsec_max_port_sa = phy_is_1g(vtss_state, port_no) ? VTSS_MACSEC_1G_MAX_SA : VTSS_MACSEC_10G_MAX_SA;
    u32 macsec_max_sc_rx = macsec_max_port_sa / 2;
    u32 macsec_max_secy = macsec_max_sc_rx;

    num_of_sa_rsrv += VTSS_MACSEC_SA_PER_SC_MIN;

    for (i = 0; i < macsec_max_secy; i++) {
        secy = &vtss_state->macsec_conf[port_no].secy[i];
        found_sc = 0;
        if (secy->in_use) {
            secy_in_use++;
            for (sc = 0; sc < macsec_max_sc_rx; sc++) {
                if (secy->rx_sc[sc] != NULL) {
                    found_sc = 1;
                    break;
                }
            }
            if (!found_sc) { /* No SC installed yet */
                if (!(is_sc && (secy_id == i))) {
                    num_of_sa_rsrv += VTSS_MACSEC_SA_PER_SC_MIN;
                }
            }
        }
    }
    if (!is_sc && (secy_in_use == macsec_max_secy)) {
        return 0;
    }

    for (sc = 0; sc < macsec_max_sc_rx; sc++) {
        if (vtss_state->macsec_conf[port_no].rx_sc[sc].in_use) {
            for (sa = 0; sa < VTSS_MACSEC_SA_PER_SC; sa++) {
                if (vtss_state->macsec_conf[port_no].rx_sc[sc].sa[sa] != NULL) {
                    found_sa = 1;
                    break;
                }
            }
            if (!found_sa) { /* No SA installed yet */
                num_of_sa_rsrv += VTSS_MACSEC_SA_PER_SC_MIN;
            }
        }
    }

    for (sa = 0; sa < macsec_max_port_sa; sa++) {
        if (vtss_state->macsec_conf[port_no].rx_sa[sa].in_use) {
            num_of_rx_sa_inuse++;
        }
    }

    return (num_of_rx_sa_inuse + num_of_sa_rsrv <= macsec_max_port_sa);
}

static vtss_rc macsec_update_glb_validate(vtss_state_t *vtss_state, vtss_port_no_t p)
{
    macsec_secy_in_use_iter_t in_use_inter;
    macsec_secy_in_use_inter_init(&in_use_inter);
    vtss_validate_frames_t val = VTSS_MACSEC_VALIDATE_FRAMES_STRICT;
    u32 sc;
    vtss_macsec_internal_secy_t *secy;

    while (macsec_secy_in_use_inter_getnext(vtss_state, p, &in_use_inter)) {
        secy = &vtss_state->macsec_conf[p].secy[in_use_inter.secy_id];
        if (secy) {
            for (sc = 0; sc < VTSS_MACSEC_MAX_SC_RX; sc++) {
                if (secy->rx_sc[sc] == NULL || !secy->rx_sc[sc]->in_use) {
                    continue;
                }
                if (secy->rx_sc[sc]->conf.validate_frames < val) {
                    val = secy->rx_sc[sc]->conf.validate_frames;
                }
            }
        }
    }
    CSR_WRM(p, VTSS_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_MISC_CONTROL,
            VTSS_F_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_MISC_CONTROL_VALIDATE_FRAMES(val),
            VTSS_M_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_MISC_CONTROL_VALIDATE_FRAMES);
    return VTSS_RC_OK;
}


/* ================================================================= *
 *  Private functions - Start
 * ================================================================= */
static vtss_rc vtss_macsec_port_check (vtss_inst_t inst, vtss_state_t **vtss_state,
                                       vtss_macsec_port_t port, BOOL create, u32 *secy_id)
{
    BOOL secy_vacancy = 0;
    u16 i;
    vtss_macsec_internal_secy_t *secy;
    VTSS_RC(vtss_inst_port_no_check(inst, vtss_state, port.port_no));

    if ((*vtss_state)->macsec_conf[port.port_no].glb.init.enable == 0) {
        VTSS_E("MacSec API port:%u not enabled", port.port_no);
        return VTSS_RC_ERROR;
    }

    for (i = 0; i < VTSS_MACSEC_MAX_SECY; i++) {
        secy = &(*vtss_state)->macsec_conf[port.port_no].secy[i];

        if ((create && secy->in_use) && (secy->sci.port_id == port.port_id) && (secy->service_id == port.service_id)) {
            VTSS_E("secy with port_no:%u port_id:%u service_id:%u already in use", port.port_no, port.port_id, port.service_id);
            return VTSS_RC_ERROR;
        }
        if ((!create && secy->in_use) && (secy->sci.port_id == port.port_id) && (secy->service_id == port.service_id)) {
            *secy_id = i;
            return VTSS_RC_OK;
        }
        if (create && !secy->in_use) {
            *secy_id = i;
            secy_vacancy = 1;
            break;
        }
    }
    if (!create) {
        VTSS_E("No secy with port_no:%u port_id:%u service_id:%u found", port.port_no, port.port_id, port.service_id);
        return VTSS_RC_ERROR;
    }
    if (create && !secy_vacancy) {
        VTSS_E("No secy vacancy");
        return VTSS_RC_ERROR;
    }
    return VTSS_RC_OK;
}

static vtss_rc vtss_inst_macsec_port_no_check(vtss_inst_t inst,  vtss_state_t **vtss_state, vtss_port_no_t port_no)
{
    VTSS_RC(vtss_inst_port_no_check(inst, vtss_state, port_no));

    if ((*vtss_state)->macsec_conf[port_no].glb.init.enable == 0) {
        VTSS_E("MacSec not enabled\n");
        return VTSS_RC_ERROR;
    }
    return VTSS_RC_OK;
}

static vtss_rc macsec_init_mac(vtss_state_t                      *vtss_state,
                               const vtss_port_no_t              port_no,
                               BOOL enable)
{
    BOOL phy10g;
    u32 ch_id = 0, rx_rd_thres = 5;

    VTSS_RC(phy_type_get(vtss_state, port_no, &phy10g));
#ifdef VTSS_CHIP_10G_PHY
    if (phy10g && vtss_state->phy_10g_state[port_no].mode.oper_mode == VTSS_PHY_WAN_MODE) {
        rx_rd_thres = 127; // For FC buffer
    }
#endif

    if (phy10g) {
#ifdef VTSS_CHIP_10G_PHY
        ch_id =  vtss_state->phy_10g_state[port_no].channel_id;
#endif
    } else {
#if defined(VTSS_CHIP_CU_PHY)
        ch_id =  vtss_state->phy_state[port_no].type.channel_id;
#endif
    }

    if (phy10g) {
        /* Enable MAC in the datapath */
        CSR_WRM(port_no, VTSS_VENICE_DEV1_MAC_ENABLE_MAC_ENA,
                (enable ? VTSS_F_VENICE_DEV1_MAC_ENABLE_MAC_ENA_MAC_ENA : 0),
                VTSS_F_VENICE_DEV1_MAC_ENABLE_MAC_ENA_MAC_ENA);
    }

    // Host MAC
    CSR_WR(port_no, VTSS_HOST_MAC_CONFIG_MAC_PKTINF_CFG,
           VTSS_F_HOST_MAC_CONFIG_MAC_PKTINF_CFG_STRIP_FCS_ENA |
           VTSS_F_HOST_MAC_CONFIG_MAC_PKTINF_CFG_INSERT_FCS_ENA |
           (phy10g ? 0 : VTSS_F_HOST_MAC_CONFIG_MAC_PKTINF_CFG_LPI_RELAY_ENA) |
           (phy10g ? VTSS_F_HOST_MAC_CONFIG_MAC_PKTINF_CFG_LF_RELAY_ENA : 0) |
           (phy10g ? VTSS_F_HOST_MAC_CONFIG_MAC_PKTINF_CFG_RF_RELAY_ENA : 0) |
           VTSS_F_HOST_MAC_CONFIG_MAC_PKTINF_CFG_STRIP_PREAMBLE_ENA |
           VTSS_F_HOST_MAC_CONFIG_MAC_PKTINF_CFG_INSERT_PREAMBLE_ENA |
           (phy10g ? VTSS_F_HOST_MAC_CONFIG_MAC_PKTINF_CFG_ENABLE_TX_PADDING : VTSS_BIT(28)));


    CSR_WRM(port_no, VTSS_HOST_MAC_CONFIG_MAC_MODE_CFG, 0,
            VTSS_F_HOST_MAC_CONFIG_MAC_MODE_CFG_DISABLE_DIC);

    CSR_WRM(port_no, VTSS_HOST_MAC_CONFIG_MAC_MAXLEN_CFG,
            VTSS_F_HOST_MAC_CONFIG_MAC_MAXLEN_CFG_MAX_LEN(10056),
            VTSS_M_HOST_MAC_CONFIG_MAC_MAXLEN_CFG_MAX_LEN);


    CSR_WR(port_no, VTSS_HOST_MAC_CONFIG_MAC_ADV_CHK_CFG,
           (phy10g ? VTSS_F_HOST_MAC_CONFIG_MAC_ADV_CHK_CFG_EXT_EOP_CHK_ENA : 0) |
           VTSS_F_HOST_MAC_CONFIG_MAC_ADV_CHK_CFG_SFD_CHK_ENA |
           VTSS_F_HOST_MAC_CONFIG_MAC_ADV_CHK_CFG_PRM_CHK_ENA |
           VTSS_F_HOST_MAC_CONFIG_MAC_ADV_CHK_CFG_OOR_ERR_ENA |
           VTSS_F_HOST_MAC_CONFIG_MAC_ADV_CHK_CFG_INR_ERR_ENA);

    CSR_WRM(port_no, VTSS_HOST_MAC_CONFIG_MAC_LFS_CFG,
            0,
            VTSS_F_HOST_MAC_CONFIG_MAC_LFS_CFG_LFS_MODE_ENA);

    /* pass through for flow control */
    CSR_WRM(port_no, VTSS_HOST_MAC_PAUSE_CFG_PAUSE_RX_FRAME_CONTROL, 0,
            VTSS_F_HOST_MAC_PAUSE_CFG_PAUSE_RX_FRAME_CONTROL_MAC_RX_PAUSE_MODE |
            VTSS_F_HOST_MAC_PAUSE_CFG_PAUSE_RX_FRAME_CONTROL_MAC_RX_PAUSE_FRAME_DROP_ENA);

    // Line MAC
    CSR_WR(port_no, VTSS_LINE_MAC_CONFIG_MAC_PKTINF_CFG,
           VTSS_F_LINE_MAC_CONFIG_MAC_PKTINF_CFG_STRIP_FCS_ENA |
           VTSS_F_LINE_MAC_CONFIG_MAC_PKTINF_CFG_INSERT_FCS_ENA |
           (phy10g ? 0 : VTSS_F_LINE_MAC_CONFIG_MAC_PKTINF_CFG_LPI_RELAY_ENA) |
           (phy10g ? VTSS_F_LINE_MAC_CONFIG_MAC_PKTINF_CFG_LF_RELAY_ENA : 0) |
           (phy10g ? VTSS_F_LINE_MAC_CONFIG_MAC_PKTINF_CFG_RF_RELAY_ENA : 0) |
           VTSS_F_LINE_MAC_CONFIG_MAC_PKTINF_CFG_STRIP_PREAMBLE_ENA |
           VTSS_F_LINE_MAC_CONFIG_MAC_PKTINF_CFG_INSERT_PREAMBLE_ENA);

    CSR_WRM(port_no, VTSS_LINE_MAC_CONFIG_MAC_MODE_CFG, enable ? 0 : 1,
            VTSS_F_LINE_MAC_CONFIG_MAC_MODE_CFG_DISABLE_DIC);

    CSR_WRM(port_no, VTSS_LINE_MAC_CONFIG_MAC_MAXLEN_CFG,
            VTSS_F_LINE_MAC_CONFIG_MAC_MAXLEN_CFG_MAX_LEN(10056),
            VTSS_M_LINE_MAC_CONFIG_MAC_MAXLEN_CFG_MAX_LEN);

    CSR_WR(port_no, VTSS_LINE_MAC_CONFIG_MAC_ADV_CHK_CFG,
           (phy10g ? VTSS_F_LINE_MAC_CONFIG_MAC_ADV_CHK_CFG_EXT_EOP_CHK_ENA : 0) |
           VTSS_F_LINE_MAC_CONFIG_MAC_ADV_CHK_CFG_SFD_CHK_ENA |
           VTSS_F_LINE_MAC_CONFIG_MAC_ADV_CHK_CFG_PRM_CHK_ENA |
           VTSS_F_LINE_MAC_CONFIG_MAC_ADV_CHK_CFG_OOR_ERR_ENA |
           VTSS_F_LINE_MAC_CONFIG_MAC_ADV_CHK_CFG_INR_ERR_ENA);

    CSR_WRM(port_no, VTSS_LINE_MAC_CONFIG_MAC_LFS_CFG,
            0,
            VTSS_F_LINE_MAC_CONFIG_MAC_LFS_CFG_LFS_MODE_ENA);

    /* pass through for flow control */
    CSR_WRM(port_no, VTSS_LINE_MAC_PAUSE_CFG_PAUSE_RX_FRAME_CONTROL, 0,
            VTSS_F_LINE_MAC_PAUSE_CFG_PAUSE_RX_FRAME_CONTROL_MAC_RX_PAUSE_MODE |
            VTSS_F_LINE_MAC_PAUSE_CFG_PAUSE_RX_FRAME_CONTROL_MAC_RX_PAUSE_FRAME_DROP_ENA);

    // FC buffer
    CSR_WR(port_no, VTSS_FC_BUFFER_CONFIG_FC_READ_THRESH_CFG,
           VTSS_F_FC_BUFFER_CONFIG_FC_READ_THRESH_CFG_TX_READ_THRESH(4) |
           VTSS_F_FC_BUFFER_CONFIG_FC_READ_THRESH_CFG_RX_READ_THRESH(rx_rd_thres));

    CSR_WRM(port_no, VTSS_FC_BUFFER_CONFIG_FC_MODE_CFG,
            VTSS_F_FC_BUFFER_CONFIG_FC_MODE_CFG_RX_PPM_RATE_ADAPT_ENA |
            VTSS_F_FC_BUFFER_CONFIG_FC_MODE_CFG_TX_PPM_RATE_ADAPT_ENA,
            VTSS_F_FC_BUFFER_CONFIG_FC_MODE_CFG_RX_PPM_RATE_ADAPT_ENA |
            VTSS_F_FC_BUFFER_CONFIG_FC_MODE_CFG_TX_PPM_RATE_ADAPT_ENA);

    CSR_WR(port_no, VTSS_FC_BUFFER_CONFIG_PPM_RATE_ADAPT_THRESH_CFG,
           VTSS_F_FC_BUFFER_CONFIG_PPM_RATE_ADAPT_THRESH_CFG_TX_PPM_RATE_ADAPT_THRESH(8) |
           (phy10g ? VTSS_F_FC_BUFFER_CONFIG_PPM_RATE_ADAPT_THRESH_CFG_RX_PPM_RATE_ADAPT_THRESH(rx_rd_thres + 4) :
            VTSS_ENCODE_BITFIELD(rx_rd_thres + 4, 16, 16)));

    CSR_WRM(port_no, VTSS_FC_BUFFER_CONFIG_FC_ENA_CFG,
            VTSS_F_FC_BUFFER_CONFIG_FC_ENA_CFG_TX_ENA |
            VTSS_F_FC_BUFFER_CONFIG_FC_ENA_CFG_RX_ENA,
            VTSS_F_FC_BUFFER_CONFIG_FC_ENA_CFG_TX_ENA |
            VTSS_F_FC_BUFFER_CONFIG_FC_ENA_CFG_RX_ENA);

    /* Enable/disable MAC */
    CSR_WR(port_no, VTSS_LINE_MAC_CONFIG_MAC_ENA_CFG,
           (enable ? VTSS_F_LINE_MAC_CONFIG_MAC_ENA_CFG_RX_CLK_ENA : 0) |
           (enable ? VTSS_F_LINE_MAC_CONFIG_MAC_ENA_CFG_TX_CLK_ENA : 0) |
           (enable ? 0 : VTSS_F_LINE_MAC_CONFIG_MAC_ENA_CFG_RX_SW_RST) |
           (enable ? 0 : VTSS_F_LINE_MAC_CONFIG_MAC_ENA_CFG_TX_SW_RST) |
           (enable ? VTSS_F_LINE_MAC_CONFIG_MAC_ENA_CFG_RX_ENA : 0) |
           (enable ? VTSS_F_LINE_MAC_CONFIG_MAC_ENA_CFG_TX_ENA : 0));

    CSR_WR(port_no, VTSS_HOST_MAC_CONFIG_MAC_ENA_CFG,
           (enable ? VTSS_F_HOST_MAC_CONFIG_MAC_ENA_CFG_RX_CLK_ENA : 0) |
           (enable ? VTSS_F_HOST_MAC_CONFIG_MAC_ENA_CFG_TX_CLK_ENA : 0) |
           (enable ? 0 : VTSS_F_HOST_MAC_CONFIG_MAC_ENA_CFG_RX_SW_RST) |
           (enable ? 0 : VTSS_F_HOST_MAC_CONFIG_MAC_ENA_CFG_TX_SW_RST) |
           (enable ? VTSS_F_HOST_MAC_CONFIG_MAC_ENA_CFG_RX_ENA : 0) |
           (enable ? VTSS_F_HOST_MAC_CONFIG_MAC_ENA_CFG_TX_ENA : 0));

    /* Set the 1588 block into packet mode */
    if ((phy10g && (ch_id == 0)) || (!phy10g && (ch_id < 2))) {
        CSR_WRM(port_no, VTSS_PTP_0_IP_1588_TOP_CFG_STAT_MODE_CTL,
                (enable ? VTSS_F_PTP_0_IP_1588_TOP_CFG_STAT_MODE_CTL_PROTOCOL_MODE(4) : 0),
                VTSS_M_PTP_0_IP_1588_TOP_CFG_STAT_MODE_CTL_PROTOCOL_MODE); // 10G ch-0, 1G ch-0/1
    } else {
        CSR_WRM(port_no, VTSS_PTP_1_IP_1588_TOP_CFG_STAT_MODE_CTL,
                (enable ? VTSS_F_PTP_1_IP_1588_TOP_CFG_STAT_MODE_CTL_PROTOCOL_MODE(4) : 0),
                VTSS_M_PTP_1_IP_1588_TOP_CFG_STAT_MODE_CTL_PROTOCOL_MODE); // 10G ch-0, 1G ch-0/1
    }


    return VTSS_RC_OK;
}

// Function for setting registers depending upon speed configuration for a specific port.
// This is not a function which is public for the application, but need to be public for other API files.
vtss_rc vtss_macsec_speed_conf_priv(vtss_state_t                      *vtss_state,
                                    const vtss_port_no_t              port_no)
{
    u8 value;
    vtss_phy_port_state_t *ps      = &vtss_state->phy_state[port_no];
    vtss_phy_conf_t       *conf    = &ps->setup;
    vtss_port_status_t    *status  = &ps->status;
    vtss_port_speed_t     speed;
    BOOL phy10g;

    if (!vtss_state->macsec_conf[port_no].glb.init.enable) {
        return VTSS_RC_OK; /* Not enabled, return silently */
    }
    VTSS_RC(phy_type_get(vtss_state, port_no, &phy10g));

    // From the register description
    // * 000: 10G
    // * 001: 10G_1G
    // * 101: 1G
    // * 110: 1G_100M
    // * 111: 1G_10M
    // * others: illegal
    value = 0; // Default to 10g
#ifdef VTSS_CHIP_10G_PHY
    if (phy10g) {
        if (vtss_state->phy_10g_state[port_no].mode.oper_mode == VTSS_PHY_1G_MODE) {
            value = 1; // * 001: 10G_1G
        } else {
            // Every other modes are 10G modes.
            value = 0; // * 000: 10G
        }
    } else {
#endif
        // Determine speed.
        if (conf->mode == VTSS_PHY_MODE_ANEG) {
            speed = status->speed;
        } else {
            // Forced speed
            speed = conf->forced.speed;
        }

        // Determine register value according to the description of the register
        switch (speed) {
        case VTSS_SPEED_1G:
            value = 0x5;     // * 101: 1G
            break;

        case VTSS_SPEED_100M:
            value = 0x6;     // * 110: 1G_100M
            break;

        case VTSS_SPEED_10M:
            value = 0x7;     // * 111: 1G_10M
            break;

        default:
            VTSS_E("Unexpected speed:%d, mode:%d", speed, conf->mode);
        }
#ifdef VTSS_CHIP_10G_PHY
    }
#endif
    VTSS_D("port_no:%d, Speed mode:0x%X", port_no, value);
    CSR_WRM(port_no, VTSS_MACSEC_INGR_MACSEC_CTL_REGS_MACSEC_ENA_CFG,
            VTSS_F_MACSEC_INGR_MACSEC_CTL_REGS_MACSEC_ENA_CFG_MACSEC_SPEED_MODE(value),
            VTSS_M_MACSEC_INGR_MACSEC_CTL_REGS_MACSEC_ENA_CFG_MACSEC_SPEED_MODE);

    CSR_WRM(port_no, VTSS_MACSEC_EGR_MACSEC_CTL_REGS_MACSEC_ENA_CFG,
            VTSS_F_MACSEC_EGR_MACSEC_CTL_REGS_MACSEC_ENA_CFG_MACSEC_SPEED_MODE(value),
            VTSS_M_MACSEC_EGR_MACSEC_CTL_REGS_MACSEC_ENA_CFG_MACSEC_SPEED_MODE);

    return VTSS_RC_OK;
}

// Function for setting registers to set 4 byte preamble.
// This is not a function should be called from 1588 API.
vtss_rc vtss_macsec_preamble_shrink_set_priv(vtss_state_t          *vtss_state,
                                             const vtss_port_no_t   port_no,
                                             const BOOL             enable)
{
    BOOL phy10g;

    if (!vtss_state->macsec_conf[port_no].glb.init.enable) {
        return VTSS_RC_OK; /* Not enabled, return silently */
    }

    VTSS_RC(phy_type_get(vtss_state, port_no, &phy10g));

    /* Enable 4 byte preamble */
    CSR_WRM(port_no, VTSS_HOST_MAC_CONFIG_MAC_PKTINF_CFG,
            (enable ? (phy10g ? VTSS_F_HOST_MAC_CONFIG_MAC_PKTINF_CFG_ENABLE_4BYTE_PREAMBLE : VTSS_BIT(31)) : 0),
            (phy10g ? VTSS_F_LINE_MAC_CONFIG_MAC_PKTINF_CFG_ENABLE_4BYTE_PREAMBLE : VTSS_BIT(31)));


    return VTSS_RC_OK;
}

static vtss_rc vtss_macsec_mtu_set_priv(vtss_state_t         *vtss_state,
                                        const vtss_port_no_t port_no)
{
    u8  i;
    u16 mtu_value = vtss_state->macsec_conf[port_no].mtu_conf.mtu;
    BOOL drop     = vtss_state->macsec_conf[port_no].mtu_conf.drop;

    if (mtu_value > 32761) {
        VTSS_E("Maximum MTU allowed is 32761 (+ 4 bytes for VLAN, was:%d", mtu_value);
        mtu_value = 32761;
    }

    VTSS_I("max MTU:%d, drop:%d, val:0x%X", mtu_value, drop, VTSS_F_MACSEC_INGR_POST_PROC_CTL_DEBUG_REGS_NON_VLAN_MTU_CHECK_NV_MTU_COMPARE(mtu_value) | (drop ? VTSS_F_MACSEC_INGR_POST_PROC_CTL_DEBUG_REGS_NON_VLAN_MTU_CHECK_NV_MTU_COMP_DROP : 0));

    CSR_WR(port_no, VTSS_MACSEC_INGR_POST_PROC_CTL_DEBUG_REGS_NON_VLAN_MTU_CHECK,
           VTSS_F_MACSEC_INGR_POST_PROC_CTL_DEBUG_REGS_NON_VLAN_MTU_CHECK_NV_MTU_COMPARE(mtu_value) | (drop ? VTSS_F_MACSEC_INGR_POST_PROC_CTL_DEBUG_REGS_NON_VLAN_MTU_CHECK_NV_MTU_COMP_DROP : 0));


    CSR_WR(port_no, VTSS_MACSEC_EGR_POST_PROC_CTL_DEBUG_REGS_NON_VLAN_MTU_CHECK,
           VTSS_F_MACSEC_EGR_POST_PROC_CTL_DEBUG_REGS_NON_VLAN_MTU_CHECK_NV_MTU_COMPARE(mtu_value) | (drop ? VTSS_F_MACSEC_EGR_POST_PROC_CTL_DEBUG_REGS_NON_VLAN_MTU_CHECK_NV_MTU_COMP_DROP : 0));


    for (i = 0; i < 8; i++) {
        CSR_WR(port_no, VTSS_MACSEC_INGR_POST_PROC_CTL_DEBUG_REGS_VLAN_MTU_CHECK(i),
               VTSS_F_MACSEC_INGR_POST_PROC_CTL_DEBUG_REGS_VLAN_MTU_CHECK_MTU_COMPARE(mtu_value + 4) | (drop ? VTSS_F_MACSEC_INGR_POST_PROC_CTL_DEBUG_REGS_VLAN_MTU_CHECK_MTU_COMP_DROP : 0));

        CSR_WR(port_no, VTSS_MACSEC_EGR_POST_PROC_CTL_DEBUG_REGS_VLAN_MTU_CHECK(i),
               VTSS_F_MACSEC_EGR_POST_PROC_CTL_DEBUG_REGS_VLAN_MTU_CHECK_MTU_COMPARE(mtu_value + 4) | (drop ? VTSS_F_MACSEC_EGR_POST_PROC_CTL_DEBUG_REGS_VLAN_MTU_CHECK_MTU_COMP_DROP : 0));
    }

    return VTSS_RC_OK;
}

static vtss_rc vtss_macsec_init_set_priv(vtss_state_t                      *vtss_state,
                                         const vtss_port_no_t              port_no,
                                         const vtss_macsec_init_t          *const init)
{
    BOOL              phy10g;
    u32               latency, i;

    VTSS_RC(phy_type_get(vtss_state, port_no, &phy10g));
    VTSS_RC(macsec_init_mac(vtss_state, port_no, init->enable));


    if (phy10g) {
        /* Enable/disable 10G MACsec clock */
        CSR_WRM(port_no, VTSS_VENICE_DEV1_MAC_ENABLE_MAC_ENA,
                (init->enable ? VTSS_F_VENICE_DEV1_MAC_ENABLE_MAC_ENA_MACSEC_CLK_ENA : 0),
                VTSS_F_VENICE_DEV1_MAC_ENABLE_MAC_ENA_MACSEC_CLK_ENA);
    }

    if (init->enable) {
        /* Enable Ingress MacSec block */
        CSR_WR(port_no, VTSS_MACSEC_INGR_MACSEC_CTL_REGS_MACSEC_ENA_CFG,
               VTSS_F_MACSEC_INGR_MACSEC_CTL_REGS_MACSEC_ENA_CFG_CLK_ENA |
               VTSS_F_MACSEC_INGR_MACSEC_CTL_REGS_MACSEC_ENA_CFG_MACSEC_ENA |
               VTSS_F_MACSEC_INGR_MACSEC_CTL_REGS_MACSEC_ENA_CFG_MACSEC_SPEED_MODE(phy10g ? 0 : 0x5));


        /* Enable Egress MacSec block */
        CSR_WR(port_no, VTSS_MACSEC_EGR_MACSEC_CTL_REGS_MACSEC_ENA_CFG,
               VTSS_F_MACSEC_EGR_MACSEC_CTL_REGS_MACSEC_ENA_CFG_CLK_ENA |
               VTSS_F_MACSEC_EGR_MACSEC_CTL_REGS_MACSEC_ENA_CFG_MACSEC_ENA |
               VTSS_F_MACSEC_EGR_MACSEC_CTL_REGS_MACSEC_ENA_CFG_MACSEC_SPEED_MODE(phy10g ? 0 : 0x5));

        /* Set the context */
        CSR_WR(port_no, VTSS_MACSEC_INGR_CRYPTO_CTRL_STATUS_REGS_CONTEXT_CTRL, 0xE5880214);

        latency = phy10g ? MACSEC_10G_INGR_LATENCY : MACSEC_1G_INGR_LATENCY;
        CSR_WR(port_no, VTSS_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_MISC_CONTROL,
               VTSS_F_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_MISC_CONTROL_MC_LATENCY_FIX(latency) |
               VTSS_F_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_MISC_CONTROL_NM_MACSEC_EN |
               VTSS_F_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_MISC_CONTROL_XFORM_REC_SIZE(1));

        /* Non-matched frames are dropped, destination:controlled port */
        CSR_WR(port_no, VTSS_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_NCP, 0x89898989);
        CSR_WR(port_no, VTSS_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_CP,  0x89898989);

        /* Counters are clear on read */
        CSR_WRM(port_no, VTSS_MACSEC_INGR_CNTR_CTRL_DEBUG_REGS_COUNT_CONTROL,
                VTSS_F_MACSEC_INGR_CNTR_CTRL_DEBUG_REGS_COUNT_CONTROL_AUTO_CNTR_RESET,
                VTSS_F_MACSEC_INGR_CNTR_CTRL_DEBUG_REGS_COUNT_CONTROL_AUTO_CNTR_RESET);

        /* Set default constency rules to pass unmatched frames */
        CSR_WRM(port_no, VTSS_MACSEC_INGR_IG_CC_PARAMS2_IG_CC_CONTROL,
                VTSS_F_MACSEC_INGR_IG_CC_PARAMS2_IG_CC_CONTROL_NON_MATCH_CTRL_ACT |
                VTSS_F_MACSEC_INGR_IG_CC_PARAMS2_IG_CC_CONTROL_NON_MATCH_ACT,
                VTSS_F_MACSEC_INGR_IG_CC_PARAMS2_IG_CC_CONTROL_NON_MATCH_CTRL_ACT |
                VTSS_F_MACSEC_INGR_IG_CC_PARAMS2_IG_CC_CONTROL_NON_MATCH_ACT);

        /* Egress */
        CSR_WR(port_no, VTSS_MACSEC_EGR_CRYPTO_CTRL_STATUS_REGS_CONTEXT_CTRL, 0xE5880214);

        latency = phy10g ? MACSEC_10G_EGR_LATENCY : MACSEC_1G_EGR_LATENCY;
        CSR_WR(port_no, VTSS_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_MISC_CONTROL,
               VTSS_F_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_MISC_CONTROL_MC_LATENCY_FIX(latency) |
               VTSS_F_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_MISC_CONTROL_XFORM_REC_SIZE(1));

        /* Non-matched frames are dropped, destination:common port */
        CSR_WR(port_no, VTSS_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_NCP,  0x81818181);
        CSR_WR(port_no, VTSS_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_CP,   0x81818181);

        /* Counters are clear on read */
        CSR_WRM(port_no, VTSS_MACSEC_EGR_CNTR_CTRL_DEBUG_REGS_COUNT_CONTROL,
                VTSS_F_MACSEC_EGR_CNTR_CTRL_DEBUG_REGS_COUNT_CONTROL_AUTO_CNTR_RESET,
                VTSS_F_MACSEC_EGR_CNTR_CTRL_DEBUG_REGS_COUNT_CONTROL_AUTO_CNTR_RESET);

        /* Enable VLAN tag parsing */
        CSR_WR(port_no, VTSS_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_CP_TAG,
               VTSS_F_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_CP_TAG_PARSE_STAG |
               VTSS_F_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_CP_TAG_PARSE_QTAG |
               VTSS_F_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_CP_TAG_PARSE_QINQ);

        CSR_WR(port_no, VTSS_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_CP_TAG,
               VTSS_F_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_CP_TAG_PARSE_STAG |
               VTSS_F_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_CP_TAG_PARSE_QTAG |
               VTSS_F_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_CP_TAG_PARSE_QINQ);

        /* BLOCK_CTX_UPDATE::BLOCK_CTX_UPDATES should be programmed to 0x3 */
        CSR_WR(port_no, VTSS_MACSEC_EGR_CRYPTO_CTRL_STATUS_REGS_BLOCK_CTX_UPDATE, 0x3);
        CSR_WR(port_no, VTSS_MACSEC_INGR_CRYPTO_CTRL_STATUS_REGS_BLOCK_CTX_UPDATE, 0x3);

        vtss_state->macsec_conf[port_no].mtu_conf.mtu  = 32761; // We set MTU to maximum, because we don't know which frame size the MAC/Switch supports.
        vtss_state->macsec_conf[port_no].mtu_conf.drop = TRUE;
        VTSS_RC(vtss_macsec_mtu_set_priv(vtss_state, port_no));

        for (i = 0; i < VTSS_MACSEC_CP_RULES; i++) {
            vtss_state->macsec_conf[port_no].glb.control_match[i].match = VTSS_MACSEC_MATCH_DISABLE;
            vtss_state->macsec_conf[port_no].glb.egr_bypass_record[i] = MACSEC_NOT_IN_USE;
        }

    } else {

        /* Disable the core, set static bypass mode */
        /* Ingress */
        CSR_WR(port_no, VTSS_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_MISC_CONTROL,
               VTSS_F_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_MISC_CONTROL_STATIC_BYPASS);

        CSR_WR(port_no, VTSS_MACSEC_INGR_MACSEC_CTL_REGS_MACSEC_ENA_CFG,
               VTSS_F_MACSEC_INGR_MACSEC_CTL_REGS_MACSEC_ENA_CFG_SW_RST |
               VTSS_F_MACSEC_INGR_MACSEC_CTL_REGS_MACSEC_ENA_CFG_MACSEC_BYPASS_ENA);

        /* Egress */
        CSR_WR(port_no, VTSS_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_MISC_CONTROL,
               VTSS_F_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_MISC_CONTROL_STATIC_BYPASS);

        CSR_WR(port_no, VTSS_MACSEC_EGR_MACSEC_CTL_REGS_MACSEC_ENA_CFG,
               VTSS_F_MACSEC_EGR_MACSEC_CTL_REGS_MACSEC_ENA_CFG_SW_RST |
               VTSS_F_MACSEC_EGR_MACSEC_CTL_REGS_MACSEC_ENA_CFG_MACSEC_BYPASS_ENA);

        /* Clear the internals */
        memset(&vtss_state->macsec_conf[port_no], 0, sizeof(vtss_state->macsec_conf[port_no]));

    }
    return VTSS_RC_OK;
}

static vtss_rc macsec_sa_flow_set(vtss_state_t *vtss_state, vtss_port_no_t p, BOOL egr, u32 record,
                                  vtss_macsec_internal_secy_t *secy, u16 an, u32 sc, vtss_macsec_match_action_t action)
{
    u32 tag_bypass = 0, offset = 0, validate = 0, dest_port = 0, flow_type = (egr ? 3 : 2);
    BOOL conf = 0, incl_sci = 0, use_es = 0, use_scb = 0, rp = 0, protect_frm = 0;

    if (action == VTSS_MACSEC_MATCH_ACTION_DROP) {
        flow_type = 1; // Destination: Drop
    } else if (action == VTSS_MACSEC_MATCH_ACTION_CONTROLLED_PORT) {
        dest_port = 2; // Destination: Controlled port
    } else {
        dest_port = 3; // Destination: Uncontrolled port
        flow_type = 0;
    }
    if (secy != NULL) {
        if (egr) {
            conf = secy->tx_sc.sa[an]->confidentiality;
            protect_frm = secy->conf.protect_frames;
            incl_sci = secy->conf.always_include_sci;
            use_es = secy->conf.use_es;
            use_scb = secy->conf.use_scb;
            offset = secy->conf.confidentiality_offset;
        } else {
            rp = secy->rx_sc[sc]->conf.replay_protect;
            offset = secy->rx_sc[sc]->conf.confidentiality_offset;
            if (secy->rx_sc[sc]->conf.validate_frames == VTSS_MACSEC_VALIDATE_FRAMES_DISABLED) {
                validate = 0;
            } else if (secy->rx_sc[sc]->conf.validate_frames == VTSS_MACSEC_VALIDATE_FRAMES_CHECK) {
                validate = 1;
            } else if (secy->rx_sc[sc]->conf.validate_frames == VTSS_MACSEC_VALIDATE_FRAMES_STRICT) {
                validate = 2;
            } else {
                VTSS_E("validate_frames value invalid");
                return VTSS_RC_ERROR;
            }
            /* Update the global 'validate frames' */
            /* The value is the least of all validate_frames entires of all SecYs, i.e. disabled/check/strict */
            VTSS_RC(macsec_update_glb_validate(vtss_state, p));
        }
        if (secy->tag_bypass == VTSS_MACSEC_BYPASS_TAG_ONE) {
            tag_bypass = 1;
        } else if (secy->tag_bypass == VTSS_MACSEC_BYPASS_TAG_TWO) {
            tag_bypass = 2;
        }
    }
    if (egr) {
        VTSS_D("Egress (sa:%d): prot:%d, incl_sci:%d use_es:%d use_scb:%d confidentiality:%d action:%s",
               record, protect_frm, incl_sci, use_es, use_scb, conf,
               flow_type == 0 ? "Bypass" : flow_type == 1 ? "Drop" : flow_type == 2 ? "Ingress" : flow_type == 3 ? "Egress" : "");
        CSR_WR(p, VTSS_MACSEC_EGR_SA_MATCH_FLOW_CONTROL_PARAMS_EGR_SAM_FLOW_CTRL_EGR(record),
               VTSS_F_MACSEC_EGR_SA_MATCH_FLOW_CONTROL_PARAMS_EGR_SAM_FLOW_CTRL_EGR_DROP_ACTION(2) |                   // Drop action=Drop
               VTSS_F_MACSEC_EGR_SA_MATCH_FLOW_CONTROL_PARAMS_EGR_SAM_FLOW_CTRL_EGR_FLOW_TYPE(flow_type) |             // MacSec egress
               VTSS_F_MACSEC_EGR_SA_MATCH_FLOW_CONTROL_PARAMS_EGR_SAM_FLOW_CTRL_EGR_DEST_PORT(dest_port) |             // Controlled port
               (protect_frm ? VTSS_F_MACSEC_EGR_SA_MATCH_FLOW_CONTROL_PARAMS_EGR_SAM_FLOW_CTRL_EGR_PROTECT_FRAME : 0) |// Protect frame
               (incl_sci ? VTSS_F_MACSEC_EGR_SA_MATCH_FLOW_CONTROL_PARAMS_EGR_SAM_FLOW_CTRL_EGR_INCLUDE_SCI : 0) |     // Incl.SCI
               (use_es ? VTSS_F_MACSEC_EGR_SA_MATCH_FLOW_CONTROL_PARAMS_EGR_SAM_FLOW_CTRL_EGR_USE_ES : 0) |            // Use ES
               (use_scb ? VTSS_F_MACSEC_EGR_SA_MATCH_FLOW_CONTROL_PARAMS_EGR_SAM_FLOW_CTRL_EGR_USE_SCB : 0) |          // Use SCB
               VTSS_F_MACSEC_EGR_SA_MATCH_FLOW_CONTROL_PARAMS_EGR_SAM_FLOW_CTRL_EGR_TAG_BYPASS_SIZE(tag_bypass) |      // VLAN Tags to bypass
               VTSS_F_MACSEC_EGR_SA_MATCH_FLOW_CONTROL_PARAMS_EGR_SAM_FLOW_CTRL_EGR_CONFIDENTIALITY_OFFSET(offset) |   // Conf.Offset
//             VTSS_F_MACSEC_EGR_SA_MATCH_FLOW_CONTROL_PARAMS_EGR_SAM_FLOW_CTRL_EGR_SA_IN_USE |                        // Inuse --> in an separate func
               (conf ? VTSS_F_MACSEC_EGR_SA_MATCH_FLOW_CONTROL_PARAMS_EGR_SAM_FLOW_CTRL_EGR_CONF_PROTECT : 0));        // Confidentiality
    } else {
        VTSS_D("Ingress (sa:%d): rp:%d validate:%d, offset:%d action:%s", record, rp, validate, offset,
               flow_type == 0 ? "Bypass" : flow_type == 1 ? "Drop" : flow_type == 2 ? "Ingress" : flow_type == 3 ? "Egress" : "");
        CSR_WR(p, VTSS_MACSEC_INGR_SA_MATCH_FLOW_CONTROL_PARAMS_IGR_SAM_FLOW_CTRL_IGR(record),
               VTSS_F_MACSEC_INGR_SA_MATCH_FLOW_CONTROL_PARAMS_IGR_SAM_FLOW_CTRL_IGR_DROP_ACTION(2) |                  // Drop action=Drop
               VTSS_F_MACSEC_INGR_SA_MATCH_FLOW_CONTROL_PARAMS_IGR_SAM_FLOW_CTRL_IGR_FLOW_TYPE(flow_type) |            // MacSec ingress
               VTSS_F_MACSEC_INGR_SA_MATCH_FLOW_CONTROL_PARAMS_IGR_SAM_FLOW_CTRL_IGR_DEST_PORT(dest_port) |            // Controlled port
               (rp ? VTSS_F_MACSEC_INGR_SA_MATCH_FLOW_CONTROL_PARAMS_IGR_SAM_FLOW_CTRL_IGR_REPLAY_PROTECT : 0) |       // Replay protect
               (VTSS_F_MACSEC_INGR_SA_MATCH_FLOW_CONTROL_PARAMS_IGR_SAM_FLOW_CTRL_IGR_VALIDATE_FRAMES(validate) |      // Validate
//              VTSS_F_MACSEC_INGR_SA_MATCH_FLOW_CONTROL_PARAMS_IGR_SAM_FLOW_CTRL_IGR_SA_IN_USE |                      // Inuse --> in an separate func
                VTSS_F_MACSEC_INGR_SA_MATCH_FLOW_CONTROL_PARAMS_IGR_SAM_FLOW_CTRL_IGR_CONFIDENTIALITY_OFFSET(offset)));// Conf. offset
    }

    return VTSS_RC_OK;
}

static vtss_rc macsec_sa_match_set(vtss_state_t *vtss_state, vtss_port_no_t p, BOOL egr, u32 record, vtss_macsec_match_pattern_t *pattern,
                                   vtss_macsec_internal_secy_t *secy, BOOL sci, u32 sc, u32 an, BOOL untagged)
{
    /*  For each 'pattern->match' a pattern is configured and a the corresponding mask is set.  */
    /*  To match a frame to this record (SA) all configured patterns must be matched */

    VTSS_D("SA:%u  Dir:%s  Match idx:0x%x", record, egr ? "Egress" : "Ingress", pattern->match);

    /* Match all non-control packets - as default */
    CSR_WRM(p,
            PST_DIR(VTSS, egr, SA_MATCH_PARAMS_SAM_MISC_MATCH(record)), 0,
            PST_DIR(VTSS_F, egr, SA_MATCH_PARAMS_SAM_MISC_MATCH_CONTROL_PACKET));
    CSR_WRM(p,
            PST_DIR(VTSS,   egr, SA_MATCH_PARAMS_SAM_MASK(record)),
            PST_DIR(VTSS_F, egr, SA_MATCH_PARAMS_SAM_MASK_CTL_PACKET_MASK),
            PST_DIR(VTSS_F, egr, SA_MATCH_PARAMS_SAM_MASK_CTL_PACKET_MASK));

    if (egr) {
        /* Match macsec tag/untagged frames */
        CSR_WRM(p,
                VTSS_MACSEC_EGR_SA_MATCH_PARAMS_SAM_MISC_MATCH(record),
                VTSS_F_MACSEC_EGR_SA_MATCH_PARAMS_SAM_MISC_MATCH_UNTAGGED,
                VTSS_F_MACSEC_EGR_SA_MATCH_PARAMS_SAM_MISC_MATCH_UNTAGGED |
                VTSS_F_MACSEC_EGR_SA_MATCH_PARAMS_SAM_MISC_MATCH_BAD_TAG |
                VTSS_F_MACSEC_EGR_SA_MATCH_PARAMS_SAM_MISC_MATCH_KAY_TAG |
                VTSS_F_MACSEC_EGR_SA_MATCH_PARAMS_SAM_MISC_MATCH_TAGGED);
    } else {
        CSR_WRM(p,
                VTSS_MACSEC_INGR_SA_MATCH_PARAMS_SAM_MISC_MATCH(record),
                (untagged ? VTSS_F_MACSEC_INGR_SA_MATCH_PARAMS_SAM_MISC_MATCH_UNTAGGED : 0) |
                VTSS_F_MACSEC_INGR_SA_MATCH_PARAMS_SAM_MISC_MATCH_TAGGED,
                VTSS_F_MACSEC_INGR_SA_MATCH_PARAMS_SAM_MISC_MATCH_UNTAGGED |
                VTSS_F_MACSEC_INGR_SA_MATCH_PARAMS_SAM_MISC_MATCH_BAD_TAG |
                VTSS_F_MACSEC_INGR_SA_MATCH_PARAMS_SAM_MISC_MATCH_KAY_TAG |
                VTSS_F_MACSEC_INGR_SA_MATCH_PARAMS_SAM_MISC_MATCH_TAGGED);
    }

    if (!egr && sci) {
        VTSS_MACSEC_ASSERT(secy == NULL, "SecY is invalid");

        if (secy->conf.always_include_sci) {
            /* Match Explicit SCI at ingress, i.e. SCI in the SecTag */
            CSR_WR(p, VTSS_MACSEC_INGR_SA_MATCH_PARAMS_SAM_SCI_MATCH_LO(record), get_u32(&secy->rx_sc[sc]->sci.mac_addr.addr[0]));
            CSR_WR(p, VTSS_MACSEC_INGR_SA_MATCH_PARAMS_SAM_SCI_MATCH_HI(record), secy->rx_sc[sc]->sci.mac_addr.addr[4] |
                   (secy->rx_sc[sc]->sci.mac_addr.addr[5] << 8) | (MACSEC_BS(secy->rx_sc[sc]->sci.port_id) << 16));

            /* Enable SCI mask */
            CSR_WR(p, VTSS_MACSEC_INGR_SA_MATCH_PARAMS_SAM_MASK(record),
                   VTSS_F_MACSEC_INGR_SA_MATCH_PARAMS_SAM_MASK_MACSEC_SCI_MASK);

            /* The SC bit should be set in TCI field */
            CSR_WRM(p, VTSS_MACSEC_INGR_SA_MATCH_PARAMS_SAM_MISC_MATCH(record),
                    VTSS_ENCODE_BITFIELD(1, 29, 1),
                    VTSS_ENCODE_BITMASK(29, 1));

            /* SCI Availability mask */
            CSR_WRM(p, VTSS_MACSEC_INGR_SA_MATCH_PARAMS_SAM_MASK(record),
                    VTSS_ENCODE_BITMASK(29, 1),
                    VTSS_ENCODE_BITMASK(29, 1));
        } else {
            /* We are on P2P line.  Do not match on SCI - at all.  All macsec frames are directed into the SC */
            /* /\* Match Implicit SCI at ingress, i.e. the SMAC of the frame *\/ */
            /* CSR_WR(p, VTSS_MACSEC_INGR_SA_MATCH_PARAMS_SAM_MAC_SA_MATCH_LO(record), get_u32(&secy->rx_sc[sc]->sci.mac_addr.addr[0])); */
            /* CSR_WRM(p, VTSS_MACSEC_INGR_SA_MATCH_PARAMS_SAM_MAC_SA_MATCH_HI(record), */
            /*         secy->rx_sc[sc]->sci.mac_addr.addr[4] | (secy->rx_sc[sc]->sci.mac_addr.addr[5] << 8), */
            /*         VTSS_M_MACSEC_INGR_SA_MATCH_PARAMS_SAM_MAC_SA_MATCH_HI_MAC_SA_MATCH_15_TO_0); */

            /* /\* Enable SA Mask *\/ */
            /* CSR_WRM(p, */
            /*         VTSS_MACSEC_INGR_SA_MATCH_PARAMS_SAM_MASK(record), */
            /*         VTSS_F_MACSEC_INGR_SA_MATCH_PARAMS_SAM_MASK_MAC_SA_MASK(0x3F), */
            /*         VTSS_M_MACSEC_INGR_SA_MATCH_PARAMS_SAM_MASK_MAC_SA_MASK); */

            /* /\* SC bit should cleared in TCI field *\/ */
            /* CSR_WRM(p, VTSS_MACSEC_INGR_SA_MATCH_PARAMS_SAM_MISC_MATCH(record), 0, VTSS_ENCODE_BITMASK(29, 1)); */
            /* SCI Availability mask */
            /* CSR_WRM(p, VTSS_MACSEC_INGR_SA_MATCH_PARAMS_SAM_MASK(record), */
            /*         VTSS_ENCODE_BITMASK(29, 1), */
            /*         VTSS_ENCODE_BITMASK(29, 1)); */
        }

        /* Match AN  */
        CSR_WRM(p, VTSS_MACSEC_INGR_SA_MATCH_PARAMS_SAM_MISC_MATCH(record),
                VTSS_ENCODE_BITFIELD(an, 24, 2),
                VTSS_ENCODE_BITMASK(24, 2));

        /* AN mask */
        CSR_WRM(p, VTSS_MACSEC_INGR_SA_MATCH_PARAMS_SAM_MASK(record),
                VTSS_ENCODE_BITMASK(24, 2),
                VTSS_ENCODE_BITMASK(24, 2));

    }

    if (pattern->match & VTSS_MACSEC_MATCH_ETYPE) {
        /* Etype */
        CSR_WRM(p,
                PST_DIR(VTSS,   egr, SA_MATCH_PARAMS_SAM_MAC_SA_MATCH_HI(record)),
                PST_DIR(VTSS_F, egr, SA_MATCH_PARAMS_SAM_MAC_SA_MATCH_HI_ETHER_TYPE(MACSEC_BS(pattern->etype))),
                PST_DIR(VTSS_M, egr, SA_MATCH_PARAMS_SAM_MAC_SA_MATCH_HI_ETHER_TYPE));
        /* Enable Etype mask */
        CSR_WRM(p,
                PST_DIR(VTSS,   egr, SA_MATCH_PARAMS_SAM_MASK(record)),
                PST_DIR(VTSS_F, egr, SA_MATCH_PARAMS_SAM_MASK_MAC_ETYPE_MASK),
                PST_DIR(VTSS_F, egr, SA_MATCH_PARAMS_SAM_MASK_MAC_ETYPE_MASK));

    }

    if (pattern->match & VTSS_MACSEC_MATCH_VLAN_ID) {
        /* VLAN id */
        CSR_WRM(p,
                PST_DIR(VTSS,   egr, SA_MATCH_PARAMS_SAM_MAC_DA_MATCH_HI(record)),
                PST_DIR(VTSS_F, egr, SA_MATCH_PARAMS_SAM_MAC_DA_MATCH_HI_VLAN_ID(pattern->vid)),
                PST_DIR(VTSS_M, egr, SA_MATCH_PARAMS_SAM_MAC_DA_MATCH_HI_VLAN_ID));

        /* Enable VLAN mask */
        CSR_WRM(p,
                PST_DIR(VTSS,   egr, SA_MATCH_PARAMS_SAM_MASK(record)),
                PST_DIR(VTSS_F, egr, SA_MATCH_PARAMS_SAM_MASK_VLAN_ID_MASK),
                PST_DIR(VTSS_F, egr, SA_MATCH_PARAMS_SAM_MASK_VLAN_ID_MASK));

        /* VLAN valid */
        CSR_WRM(p,
                PST_DIR(VTSS,   egr, SA_MATCH_PARAMS_SAM_MISC_MATCH(record)),
                PST_DIR(VTSS_F, egr, SA_MATCH_PARAMS_SAM_MISC_MATCH_VLAN_VALID),
                PST_DIR(VTSS_F, egr, SA_MATCH_PARAMS_SAM_MISC_MATCH_VLAN_VALID));

        /* Enable VLAN valid mask */
        CSR_WRM(p,
                PST_DIR(VTSS,   egr, SA_MATCH_PARAMS_SAM_MASK(record)),
                PST_DIR(VTSS_F, egr, SA_MATCH_PARAMS_SAM_MASK_VLAN_VLD_MASK),
                PST_DIR(VTSS_F, egr, SA_MATCH_PARAMS_SAM_MASK_VLAN_VLD_MASK));
    }

    if (pattern->match & VTSS_MACSEC_MATCH_VLAN_ID_INNER) {
        /* VLAN id inner */
        CSR_WRM(p,
                PST_DIR(VTSS,   egr, SA_MATCH_PARAMS_SAM_EXT_MATCH(record)),
                PST_DIR(VTSS_F, egr, SA_MATCH_PARAMS_SAM_EXT_MATCH_VLAN_ID_INNER(pattern->vid_inner)),
                PST_DIR(VTSS_M, egr, SA_MATCH_PARAMS_SAM_EXT_MATCH_VLAN_ID_INNER));

        /* Enable VLAN inner mask */
        CSR_WRM(p,
                PST_DIR(VTSS,   egr, SA_MATCH_PARAMS_SAM_MASK(record)),
                PST_DIR(VTSS_F, egr, SA_MATCH_PARAMS_SAM_MASK_VLAN_ID_INNER_MASK),
                PST_DIR(VTSS_F, egr, SA_MATCH_PARAMS_SAM_MASK_VLAN_ID_INNER_MASK));
    }

    if (pattern->match & VTSS_MACSEC_MATCH_BYPASS_HDR) {
        if (egr) {
            CSR_WR(p, VTSS_MACSEC_EGR_SA_MATCH_PARAMS_SAM_HDR_BYPASS_MATCH1(record), get_u32(&pattern->hdr[0]));
            CSR_WR(p, VTSS_MACSEC_EGR_SA_MATCH_PARAMS_SAM_HDR_BYPASS_MATCH2(record), get_u32(&pattern->hdr[4]));
            CSR_WR(p, VTSS_MACSEC_EGR_SA_MATCH_PARAMS_SAM_HDR_BYPASS_MASK1(record),  get_u32(&pattern->hdr_mask[0]));
            CSR_WR(p, VTSS_MACSEC_EGR_SA_MATCH_PARAMS_SAM_HDR_BYPASS_MASK2(record),  get_u32(&pattern->hdr_mask[4]));
        } else {
            CSR_WR(p, VTSS_MACSEC_INGR_SA_MATCH_PARAMS_SAM_HDR_BYPASS_MATCH1(record), get_u32(&pattern->hdr[0]));
            CSR_WR(p, VTSS_MACSEC_INGR_SA_MATCH_PARAMS_SAM_HDR_BYPASS_MATCH2(record), get_u32(&pattern->hdr[4]));
            CSR_WR(p, VTSS_MACSEC_INGR_SA_MATCH_PARAMS_SAM_HDR_BYPASS_MASK1(record),  get_u32(&pattern->hdr_mask[0]));
            CSR_WR(p, VTSS_MACSEC_INGR_SA_MATCH_PARAMS_SAM_HDR_BYPASS_MASK2(record),  get_u32(&pattern->hdr_mask[4]));
        }
    }

    if (pattern->match & VTSS_MACSEC_MATCH_IS_CONTROL) {
        /* Control frame  */
        CSR_WRM(p,
                PST_DIR(VTSS, egr, SA_MATCH_PARAMS_SAM_MISC_MATCH(record)),
                pattern->is_control ? PST_DIR(VTSS_F, egr, SA_MATCH_PARAMS_SAM_MISC_MATCH_CONTROL_PACKET) : 0,
                PST_DIR(VTSS_F, egr, SA_MATCH_PARAMS_SAM_MISC_MATCH_CONTROL_PACKET));

        /* Enable Control frame mask */
        CSR_WRM(p,
                PST_DIR(VTSS,   egr, SA_MATCH_PARAMS_SAM_MASK(record)),
                PST_DIR(VTSS_F, egr, SA_MATCH_PARAMS_SAM_MASK_CTL_PACKET_MASK),
                PST_DIR(VTSS_F, egr, SA_MATCH_PARAMS_SAM_MASK_CTL_PACKET_MASK));
    }

    if (pattern->match & VTSS_MACSEC_MATCH_HAS_VLAN) {
        /* Has VLAN (QTAG or STAG)  */
        CSR_WRM(p,
                PST_DIR(VTSS, egr, SA_MATCH_PARAMS_SAM_MISC_MATCH(record)),
                pattern->has_vlan_tag ? PST_DIR(VTSS_F, egr, SA_MATCH_PARAMS_SAM_MISC_MATCH_VLAN_VALID) : 0,
                PST_DIR(VTSS_F, egr, SA_MATCH_PARAMS_SAM_MISC_MATCH_VLAN_VALID));

        /* Enable VLAN valid mask */
        CSR_WRM(p,
                PST_DIR(VTSS,   egr, SA_MATCH_PARAMS_SAM_MASK(record)),
                PST_DIR(VTSS_F, egr, SA_MATCH_PARAMS_SAM_MASK_VLAN_VLD_MASK),
                PST_DIR(VTSS_F, egr, SA_MATCH_PARAMS_SAM_MASK_VLAN_VLD_MASK));
    }

    if (pattern->match & VTSS_MACSEC_MATCH_HAS_VLAN_INNER) {
        /* Has vlan inner tag  */
        CSR_WRM(p,
                PST_DIR(VTSS, egr, SA_MATCH_PARAMS_SAM_MISC_MATCH(record)),
                pattern->has_vlan_inner_tag ? PST_DIR(VTSS_F, egr, SA_MATCH_PARAMS_SAM_MISC_MATCH_QINQ_FOUND) : 0,
                PST_DIR(VTSS_F, egr, SA_MATCH_PARAMS_SAM_MISC_MATCH_QINQ_FOUND));

        /* Enable VLAN inner tag mask */
        CSR_WRM(p,
                PST_DIR(VTSS,   egr, SA_MATCH_PARAMS_SAM_MASK(record)),
                PST_DIR(VTSS_F, egr, SA_MATCH_PARAMS_SAM_MASK_QINQ_FOUND_MASK),
                PST_DIR(VTSS_F, egr, SA_MATCH_PARAMS_SAM_MASK_QINQ_FOUND_MASK));
    }

    /* Set the priority */
    CSR_WRM(p,
            PST_DIR(VTSS,   egr, SA_MATCH_PARAMS_SAM_MISC_MATCH(record)),
            PST_DIR(VTSS_F, egr, SA_MATCH_PARAMS_SAM_MISC_MATCH_MATCH_PRIORITY(15 - pattern->priority)),
            PST_DIR(VTSS_M, egr, SA_MATCH_PARAMS_SAM_MISC_MATCH_MATCH_PRIORITY));

    return VTSS_RC_OK;
}

static vtss_rc record_empty_get(vtss_state_t *vtss_state,
                                vtss_port_no_t p, u32 *id, BOOL tx)
{
    u32 sa, max_sa;
    if (phy_is_1g(vtss_state, p)) {
        max_sa = VTSS_MACSEC_1G_MAX_SA;
    } else {
        max_sa = VTSS_MACSEC_10G_MAX_SA;
    }
    if (tx) {
        for (sa = 0; sa < max_sa; sa++) {
            if (!vtss_state->macsec_conf[p].tx_sa[sa].in_use) {
                *id = sa;
                return VTSS_RC_OK;
            }
        }
    } else {
        for (sa = 0; sa < max_sa; sa++) {
            if (!vtss_state->macsec_conf[p].rx_sa[sa].in_use) {
                *id = sa;
                return VTSS_RC_OK;
            }
        }
    }

    VTSS_D("All available SA's are in use");
    return VTSS_RC_ERROR;
}

static vtss_rc cp_rule_id_get(vtss_state_t *vtss_state, vtss_port_no_t p, const vtss_macsec_control_frame_match_conf_t *const conf,
                              u32 *indx, u32 *const rule, BOOL store)
{
    u32 i, etype_index;

    if (phy_is_1g(vtss_state, p)) {
        etype_index = 10; // 8 Etypes for Viper
    } else {
        etype_index = 18; // 16 Etypes for Venice
    }

    if (!store) { /* find the index to be deleted */
        if (*rule < 2) {
            *indx = 8 + *rule;
        } else if (*rule < 18) {
            if (*rule < 10) {
                *indx = *rule - 2;
            } else {
                *indx = *rule;
            }
        } else {
            *indx =  *rule - 18;
        }
        return VTSS_RC_OK;
    }

    if ((conf->match & VTSS_MACSEC_MATCH_ETYPE) && (conf->match & VTSS_MACSEC_MATCH_DMAC)) {
        for (i = 0; i < 2; i++) {
            if (vtss_state->macsec_conf[p].glb.control_match[i].match == VTSS_MACSEC_MATCH_DISABLE) {
                vtss_state->macsec_conf[p].glb.control_match[i] = *conf;
                if (rule != NULL) {
                    *rule = i;
                }
                *indx = 8 + i; // DMAC/ETYPE index 8-9
                return VTSS_RC_OK;
            }
        }
    } else if (conf->match & VTSS_MACSEC_MATCH_ETYPE)  {
        for (i = 2; i < etype_index; i++) {
            if (vtss_state->macsec_conf[p].glb.control_match[i].match == VTSS_MACSEC_MATCH_DISABLE) {
                vtss_state->macsec_conf[p].glb.control_match[i] = *conf;
                if (rule != NULL) {
                    *rule = i;
                }
                if (i < 10) {
                    *indx = i - 2;  // ETYPE 0-7
                } else {
                    *indx = i;      // ETYPE 10-17
                }
                return VTSS_RC_OK;
            }
        }
    } else if (conf->match & VTSS_MACSEC_MATCH_DMAC) {
        for (i = 18; i < 26; i++) {
            if (vtss_state->macsec_conf[p].glb.control_match[i].match == VTSS_MACSEC_MATCH_DISABLE) {
                vtss_state->macsec_conf[p].glb.control_match[i] = *conf;
                if (rule != NULL) {
                    *rule = i;
                }
                *indx =  i - 18; // DMAC 18-25
                return VTSS_RC_OK;
            }
        }
    } else if (conf->match & VTSS_MACSEC_MATCH_DISABLE) {
        return VTSS_RC_ERROR;
    } else {
        VTSS_E("Unexpected CP mode %u", conf->match);
    }

    VTSS_E("All CP rules of type '%s' are in use", (conf->match == VTSS_MACSEC_MATCH_ETYPE) ? "Etype" :
           (conf->match == VTSS_MACSEC_MATCH_DMAC) ? "DMAC" : "DMAC/Etype");
    return VTSS_RC_ERROR;
}


static u32 get_xform_value(u32 i, BOOL egr, BOOL aes_128, vtss_macsec_internal_secy_t *secy, u16 an, u32 sc, u16 record)
{
    vtss_macsec_sak_t      *sak;

    if (egr) {
        sak = &secy->tx_sc.sa[an]->sak;
    } else {
        sak = &secy->rx_sc[sc]->sa[an]->sak;
    }
    if (i == 0) {
        if (egr) {
            return (0x9241E066 | ((aes_128 ? 0xB : 0xF) << 16) | (an << 26));
        } else {
            return (0xD241E06F | ((aes_128 ? 0xB : 0xF) << 16));
        }
    } else if (i == 1) {
        return record;
    } else {
        if (aes_128) {
            if (i <= 5) {
                return get_u32(&sak->buf[0] + ((i - 2) * 4)); // AES Key 0-3, Record 2-5
            } else if (i <= 9) {
                return get_u32(&sak->h_buf[0] + ((i - 6) * 4)); // AES Hash 0-3 Record 6-9
            } else if (i == 10) {
                if (egr) {
                    return secy->tx_sc.sa[an]->status.next_pn; // Sequence / Next PN, Record 10
                } else {
                    // Sequence / lowest_pn = next_pn - replay_window, Record 10
                    return secy->rx_sc[sc]->sa[an]->status.lowest_pn + secy->rx_sc[sc]->conf.replay_window;
                }
            } else if (i == 11) {
                if (egr) {
                    return get_u32(&secy->sci.mac_addr.addr[0]); // SCI #0 / egr / Record 11
                } else {
                    return secy->rx_sc[sc]->conf.replay_window; // Replay window / ingr / Record 11
                }
            } else if (i == 12) {
                if (egr) {
                    return (secy->sci.mac_addr.addr[4] |
                            (secy->sci.mac_addr.addr[5] << 8) |
                            (MACSEC_BS(secy->sci.port_id) << 16)); // SCI #1 (egr) / Record 12
                } else {
                    return get_u32(&secy->rx_sc[sc]->sci.mac_addr.addr[0]); // SCI #0 / ingr / Record 12
                }
            } else if (i == 13 && !egr) {
                return (secy->rx_sc[sc]->sci.mac_addr.addr[4] |
                        (secy->rx_sc[sc]->sci.mac_addr.addr[5] << 8) |
                        (MACSEC_BS(secy->sci.port_id) << 16)); // SCI #1 / ingr / Record 13
            } else {
                return 0;
            }
        } else {
            // AES 256
            if (i <= 9) {
                return get_u32(&sak->buf[0] + ((i - 2) * 4)); // AES Key 0-7, Record 2-9
            } else if (i <= 13) {
                return get_u32(&sak->h_buf[0] + ((i - 10) * 4)); // AES Hash 0-3 Record 10-13
            } else if (i == 14) {
                if (egr) {
                    return secy->tx_sc.sa[an]->status.next_pn; // Sequence / Next PN, Record 14
                } else {
                    return secy->rx_sc[sc]->sa[an]->status.lowest_pn + secy->rx_sc[sc]->conf.replay_window;// Sequence / Lowest PN, Record 14
                }
            } else if (i == 15) {
                if (egr) {
                    return get_u32(&secy->sci.mac_addr.addr[0]); // SCI/IV #0
                } else {
                    return secy->rx_sc[sc]->conf.replay_window; // Replay window / ingr / Record 15
                }
            } else if (i == 16) {
                if (egr) {
                    return (secy->sci.mac_addr.addr[4] |
                            (secy->sci.mac_addr.addr[5] << 8) |
                            (MACSEC_BS(secy->sci.port_id) << 16)); // SCI #1 (egr)
                } else {
                    return get_u32(&secy->rx_sc[sc]->sci.mac_addr.addr[0]); // SCI #0
                }
            } else if (i == 17 && !egr) {
                return (secy->rx_sc[sc]->sci.mac_addr.addr[4] |
                        (secy->rx_sc[sc]->sci.mac_addr.addr[5] << 8) |
                        (MACSEC_BS(secy->sci.port_id) << 16)); // SCI #1 (ingr)
            } else {
                return 0;
            }
        }
    }
}

static vtss_rc macsec_sa_xform_set(vtss_state_t *vtss_state, vtss_port_no_t p, BOOL egr, u32 record,
                                   vtss_macsec_internal_secy_t *secy, u16 an, u32 sc)
{
    BOOL aes_128 = (secy->conf.current_cipher_suite == VTSS_MACSEC_CIPHER_SUITE_GCM_AES_128) ? 1 : 0;
    u32 i;
    for (i = 0; i < 20; i++) {
        if (egr) {
            VTSS_RC(csr_wr(vtss_state, p, 0x1f, 1, ((0x8000 + i) | (record * 32)), get_xform_value(i, egr, aes_128, secy, an, sc, record)));
        } else {
            VTSS_RC(csr_wr(vtss_state, p, 0x1f, 1, ((0x0000 + i) | (record * 32)), get_xform_value(i, egr, aes_128, secy, an, sc, record)));
        }
    }
    return VTSS_RC_OK;
}



static vtss_rc macsec_sa_enable(vtss_state_t *vtss_state, vtss_port_no_t p, u32 record, BOOL egr, BOOL enable)
{
    VTSS_D("%s sa:%u dir:%s", enable ? "Enable" : "Disable", record, egr ? "egr" : "ingr" );
    if (enable) {
        if (egr) {
            if (record < 32) {
                CSR_WR(p, VTSS_MACSEC_EGR_SA_MATCH_CTL_PARAMS_SAM_ENTRY_SET1, (1 << record));
            } else {
                CSR_WR(p, VTSS_MACSEC_EGR_SA_MATCH_CTL_PARAMS_SAM_ENTRY_SET2, (1 << (record - 32)));
                CSR_WR(p, VTSS_MACSEC_EGR_SA_MATCH_CTL_PARAMS_SAM_ENTRY_SET1, 0); // SAs above 31 requires 2 writes
            }
        } else {
            if (record < 32) {
                CSR_WR(p, VTSS_MACSEC_INGR_SA_MATCH_CTL_PARAMS_SAM_ENTRY_SET1, (1 << record));
            } else {
                CSR_WR(p, VTSS_MACSEC_INGR_SA_MATCH_CTL_PARAMS_SAM_ENTRY_SET2, (1 << (record - 32)));
                CSR_WR(p, VTSS_MACSEC_INGR_SA_MATCH_CTL_PARAMS_SAM_ENTRY_SET1, 0); // SAs above 31 requires 2 writes
            }
        }
    } else {
        if (egr) {
            if (record < 32) {
                CSR_WR(p, VTSS_MACSEC_EGR_SA_MATCH_CTL_PARAMS_SAM_ENTRY_CLEAR1, (1 << record));
            } else {
                CSR_WR(p, VTSS_MACSEC_EGR_SA_MATCH_CTL_PARAMS_SAM_ENTRY_CLEAR2, (1 << (record - 32)));
                CSR_WR(p, VTSS_MACSEC_EGR_SA_MATCH_CTL_PARAMS_SAM_ENTRY_CLEAR1, 0); // SAs above 31 requires 2 writes
            }
        } else {
            if (record < 32) {
                CSR_WR(p, VTSS_MACSEC_INGR_SA_MATCH_CTL_PARAMS_SAM_ENTRY_CLEAR1, (1 << record));
            } else {
                CSR_WR(p, VTSS_MACSEC_INGR_SA_MATCH_CTL_PARAMS_SAM_ENTRY_CLEAR2, (1 << (record - 32)));
                CSR_WR(p, VTSS_MACSEC_INGR_SA_MATCH_CTL_PARAMS_SAM_ENTRY_CLEAR1, 0); // SAs above 31 requires 2 writes
            }
        }
    }
    return VTSS_RC_OK;
}

static vtss_rc macsec_sa_inuse(vtss_state_t *vtss_state, vtss_port_no_t p, u32 record, BOOL egr, BOOL enable)
{
    VTSS_D("Set sa_inuse bit: sa:%d dir:%s enable:%d", record, egr ? "egr" : "ingr", enable );
    if (egr) {
        CSR_WRM(p, VTSS_MACSEC_EGR_SA_MATCH_FLOW_CONTROL_PARAMS_EGR_SAM_FLOW_CTRL_EGR(record),
                (enable ? VTSS_F_MACSEC_EGR_SA_MATCH_FLOW_CONTROL_PARAMS_EGR_SAM_FLOW_CTRL_EGR_SA_IN_USE : 0),
                VTSS_F_MACSEC_EGR_SA_MATCH_FLOW_CONTROL_PARAMS_EGR_SAM_FLOW_CTRL_EGR_SA_IN_USE);
    } else {
        CSR_WRM(p, VTSS_MACSEC_INGR_SA_MATCH_FLOW_CONTROL_PARAMS_IGR_SAM_FLOW_CTRL_IGR(record),
                (enable ? VTSS_F_MACSEC_INGR_SA_MATCH_FLOW_CONTROL_PARAMS_IGR_SAM_FLOW_CTRL_IGR_SA_IN_USE : 0),
                VTSS_F_MACSEC_INGR_SA_MATCH_FLOW_CONTROL_PARAMS_IGR_SAM_FLOW_CTRL_IGR_SA_IN_USE);
    }
    return VTSS_RC_OK;
}

static vtss_rc macsec_sa_toggle(vtss_state_t *vtss_state, vtss_port_no_t p, u32 new_record, u32 old_record, BOOL egr)
{

    VTSS_D("Toggle old tx sa:%u -> new tx sa:%u dir:%s", old_record, new_record, egr ? "egr" : "ingr" );
    if (!egr) { /* INGRESS */
        if ((old_record < 32 && new_record < 32) ||  (old_record >= 32 && new_record >= 32)) {
            /* Deactivate the old SA and activate new SA in one atomic write */
            if (old_record < 32) {
                CSR_WR(p, VTSS_MACSEC_INGR_SA_MATCH_CTL_PARAMS_SAM_ENTRY_TOGGLE1,
                       ((1 << old_record) | (1 << new_record)));
            } else {
                CSR_WR(p, VTSS_MACSEC_INGR_SA_MATCH_CTL_PARAMS_SAM_ENTRY_TOGGLE2,
                       ((1 << (old_record - 32)) | (1 << (new_record - 32))));
                CSR_WR(p, VTSS_MACSEC_INGR_SA_MATCH_CTL_PARAMS_SAM_ENTRY_TOGGLE1, 0);
            }
        } else {
            /* The SA's are not accessable through a single register - 2 writes are needed */
            if (old_record < 32) {
                CSR_WR(p, VTSS_MACSEC_INGR_SA_MATCH_CTL_PARAMS_SAM_ENTRY_TOGGLE1,
                       1 << (old_record < 32 ? old_record : new_record));
            } else {
                CSR_WR(p, VTSS_MACSEC_INGR_SA_MATCH_CTL_PARAMS_SAM_ENTRY_TOGGLE2,
                       1 << (old_record >= 32 ? old_record : new_record));
                CSR_WR(p, VTSS_MACSEC_INGR_SA_MATCH_CTL_PARAMS_SAM_ENTRY_TOGGLE1, 0);
            }
        }
    } else {   /* EGRESS */
        if ((old_record < 32 && new_record < 32) ||  (old_record >= 32 && new_record >= 32)) {
            /* Deactivate the old SA and activate new SA in one atomic write */
            if (old_record < 32) {
                CSR_WR(p, VTSS_MACSEC_EGR_SA_MATCH_CTL_PARAMS_SAM_ENTRY_TOGGLE1,
                       ((1 << old_record) | (1 << new_record)));
            } else {
                CSR_WR(p, VTSS_MACSEC_EGR_SA_MATCH_CTL_PARAMS_SAM_ENTRY_TOGGLE2,
                       ((1 << (old_record - 32)) | (1 << (new_record - 32))));
                CSR_WR(p, VTSS_MACSEC_EGR_SA_MATCH_CTL_PARAMS_SAM_ENTRY_TOGGLE1, 0);
            }
        } else {
            /* The SA's are not accessable through a single register - 2 writes are needed */
            if (old_record < 32) {
                CSR_WR(p, VTSS_MACSEC_EGR_SA_MATCH_CTL_PARAMS_SAM_ENTRY_TOGGLE1,
                       1 << (old_record < 32 ? old_record : new_record));
            } else {
                CSR_WR(p, VTSS_MACSEC_EGR_SA_MATCH_CTL_PARAMS_SAM_ENTRY_TOGGLE2,
                       1 << (old_record >= 32 ? old_record : new_record));
                CSR_WR(p, VTSS_MACSEC_EGR_SA_MATCH_CTL_PARAMS_SAM_ENTRY_TOGGLE1, 0);
            }
        }
    }

    return VTSS_RC_OK;
}

static vtss_rc vtss_macsec_pattern_set_priv(vtss_state_t                       *vtss_state,
                                            const vtss_port_no_t               port_no,
                                            const u32                          secy_id,
                                            const vtss_macsec_direction_t      direction,
                                            const vtss_macsec_match_action_t   action,
                                            const vtss_macsec_match_pattern_t  *const pattern,
                                            u32                                 rule_id)
{
    vtss_macsec_internal_secy_t *secy = (secy_id == MACSEC_SECY_NONE) ? NULL : &vtss_state->macsec_conf[port_no].secy[secy_id];
    BOOL egr = (direction == VTSS_MACSEC_DIRECTION_EGRESS) ? 1 : 0;
    u32 record, i;
    vtss_macsec_match_pattern_t match = *pattern;

    if (record_empty_get(vtss_state, port_no, &record, egr) != VTSS_RC_OK) {
        VTSS_E("Could not get an empty record");
        return VTSS_RC_ERROR;
    }
    if (pattern->priority > VTSS_MACSEC_MATCH_PRIORITY_LOWEST) {
        VTSS_E("The pattern priority is not valid");
        return VTSS_RC_ERROR;
    }
    VTSS_D("SA:%u Action:%s Dir:%s", record, (action == 0) ? "Drop" : (action == 1) ? "Ctrl" : "Unctrl",  egr ? "egr" : "ingr");

    if (egr) {
        vtss_state->macsec_conf[port_no].tx_sa[record].record = record;
        vtss_state->macsec_conf[port_no].tx_sa[record].in_use = 1;
    } else {
        vtss_state->macsec_conf[port_no].rx_sa[record].record = record;
        vtss_state->macsec_conf[port_no].rx_sa[record].in_use = 1;
    }
    if (secy != NULL) {
        secy->pattern_record[action][direction] = record;
    } else {
        vtss_state->macsec_conf[port_no].glb.egr_bypass_record[rule_id] = record;
    }

    /* In case of VLAN tag as ether type for CP, must replace IS_CONTROL with HAS_VLAN - only ingress */
    if (!egr) {
        for (i = 0; i < VTSS_MACSEC_CP_RULES; i++) {
            if ((vtss_state->macsec_conf[port_no].glb.control_match[i].etype == 0x8100) &&
                (vtss_state->macsec_conf[port_no].glb.control_match[i].match == VTSS_MACSEC_MATCH_ETYPE)) {
                match.match = (match.match | VTSS_MACSEC_MATCH_HAS_VLAN) & (0xFFFFFFFF ^ (VTSS_MACSEC_MATCH_IS_CONTROL));
                match.has_vlan_tag = 1;
            }
        }
    }

    if (macsec_sa_match_set(vtss_state, port_no, egr, record, &match, secy, 0, 0, 0, (action == 1 ? 0 : 1)) != VTSS_RC_OK) {
        VTSS_E("Could program the SA match");
        return VTSS_RC_ERROR;
    }
    if (macsec_sa_flow_set(vtss_state, port_no, egr, record, NULL, 0, 0, action) != VTSS_RC_OK) {
        VTSS_E("Could program the SA flow");
        return VTSS_RC_ERROR;
    }
    if (macsec_sa_enable(vtss_state, port_no, record, egr, MACSEC_ENABLE) != VTSS_RC_OK) {
        VTSS_E("Could not enable the SA:%u", record);
        return VTSS_RC_ERROR;
    }
    if (macsec_sa_inuse(vtss_state, port_no, record, egr, MACSEC_ENABLE) != VTSS_RC_OK) {
        VTSS_E("Could not set SA:%u to 'in_use'", record);
        return VTSS_RC_ERROR;
    }
    return VTSS_RC_OK;
}

static vtss_rc vtss_macsec_bypass_mode_set_priv(vtss_state_t                     *vtss_state,
                                                const vtss_port_no_t             port_no,
                                                const vtss_macsec_bypass_mode_t  *const bypass)
{
    if (bypass->mode == VTSS_MACSEC_BYPASS_HDR) {
        if (bypass->hdr_bypass_len > 16 || bypass->hdr_bypass_len == 0 || (bypass->hdr_bypass_len % 2)) {
            VTSS_E("Invalid header bypass length");
            return VTSS_RC_ERROR;
        }
        CSR_WR(port_no, VTSS_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_HDR_EXT_CTRL,
               VTSS_F_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_HDR_EXT_CTRL_HB_ETYPE_MATCH(MACSEC_BS(bypass->hdr_etype)) |
               VTSS_F_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_HDR_EXT_CTRL_HDR_BYPASS_LEN(bypass->hdr_bypass_len / 2 + 7) | /* len is 8-15 */
               VTSS_F_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_HDR_EXT_CTRL_HB_E_TYPE_MATCH_EN);

        CSR_WR(port_no, VTSS_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_HDR_EXT_CTRL,
               VTSS_F_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_HDR_EXT_CTRL_HB_ETYPE_MATCH(MACSEC_BS(bypass->hdr_etype)) |
               VTSS_F_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_HDR_EXT_CTRL_HDR_BYPASS_LEN(bypass->hdr_bypass_len / 2 + 7) | /* len is 8-15 */
               VTSS_F_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_HDR_EXT_CTRL_HB_E_TYPE_MATCH_EN);



    } else if (bypass->mode == VTSS_MACSEC_BYPASS_TAG) {
        CSR_WR(port_no, VTSS_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_HDR_EXT_CTRL,
               VTSS_F_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_HDR_EXT_CTRL_SECTAG_AFTER_VLAN);

        CSR_WR(port_no, VTSS_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_HDR_EXT_CTRL,
               VTSS_F_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_HDR_EXT_CTRL_SECTAG_AFTER_VLAN);
    }
    return VTSS_RC_OK;
}

static vtss_rc vtss_macsec_rx_sa_disable_priv(vtss_state_t              *vtss_state,
                                              const u32                 secy_id,
                                              const vtss_macsec_port_t  port,
                                              const vtss_macsec_sci_t   *const sci,
                                              const u16                 an)
{
    vtss_macsec_internal_secy_t *secy = &vtss_state->macsec_conf[port.port_no].secy[secy_id];
    u32 sc;
    vtss_timeofday_t tod;
    VTSS_MACSEC_ASSERT(an >= VTSS_MACSEC_SA_PER_SC_MAX, "AN is invalid");

    if (sc_from_sci_get(secy, sci, &sc) != VTSS_RC_OK) {
        VTSS_E("Could not find SC (from sci)");
        return VTSS_RC_ERROR;
    }
    VTSS_MACSEC_ASSERT(secy->rx_sc[sc] == NULL, "SC does not exist");
    VTSS_MACSEC_ASSERT(secy->rx_sc[sc]->sa[an] == NULL, "AN does not exist");

    /* Disable chip SA Flow */
    if (macsec_sa_enable(vtss_state, port.port_no, secy->rx_sc[sc]->sa[an]->record, INGRESS, MACSEC_DISABLE) != VTSS_RC_OK) {
        VTSS_E("Could disable the SA:%hu", an);
    }

    secy->rx_sc[sc]->sa[an]->status.in_use = 0;
    secy->rx_sc[sc]->sa[an]->enabled = 0;

    VTSS_TIME_OF_DAY(tod);
    secy->rx_sc[sc]->sa[an]->status.stopped_time = tod.sec; // TimeOfDay in seconds
    return VTSS_RC_OK;
}

static vtss_rc vtss_macsec_tx_sa_disable_priv(vtss_state_t              *vtss_state,
                                              const u32                 secy_id,
                                              const vtss_macsec_port_t  port,
                                              const u16                 an)
{
    vtss_macsec_internal_secy_t *secy = &vtss_state->macsec_conf[port.port_no].secy[secy_id];
    vtss_timeofday_t tod;

    VTSS_MACSEC_ASSERT(an >= VTSS_MACSEC_SA_PER_SC_MAX, "AN is invalid");
    VTSS_MACSEC_ASSERT(secy->tx_sc.sa[an] == NULL, "AN does not exist");

    /* Disable chip SA Flow */
    if (macsec_sa_enable(vtss_state, port.port_no, secy->tx_sc.sa[an]->record, EGRESS, MACSEC_DISABLE) != VTSS_RC_OK) {
        VTSS_E("Could not disable the AN:%hu", an);
    }

    // Update TX SA/SC/SecY counters

    secy->tx_sc.sa[an]->enabled = 0;
    VTSS_TIME_OF_DAY(tod);
    secy->tx_sc.sa[an]->status.stopped_time = tod.sec; // TimeOfDay in seconds
    return VTSS_RC_OK;
}


static vtss_rc vtss_macsec_default_action_set_priv(vtss_state_t                             *vtss_state,
                                                   const vtss_port_no_t                      port_no,
                                                   const vtss_macsec_default_action_policy_t *const pattern)
{
    BOOL drop;
    /* Ingress. Not control and not macsec tagged */
    drop = (pattern->ingress_non_control_and_non_macsec == VTSS_MACSEC_DEFAULT_ACTION_DROP) ? 1 : 0;
    CSR_WRM(port_no, VTSS_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_NCP,
            (drop ? VTSS_F_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_NCP_NCP_UNTAGGED_FLOW_TYPE : 0) |// bypass / drop
            VTSS_F_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_NCP_NCP_UNTAGGED_DEST_PORT(3) |          // bypass to uncontrolled port
            VTSS_F_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_NCP_NCP_UNTAGGED_DROP_ACTION(2),         // drop silently
            VTSS_F_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_NCP_NCP_UNTAGGED_FLOW_TYPE |
            VTSS_M_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_NCP_NCP_UNTAGGED_DEST_PORT |
            VTSS_M_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_NCP_NCP_UNTAGGED_DROP_ACTION);
    /* Ingress. Not control but macsec tagged */
    drop = (pattern->ingress_non_control_and_macsec == VTSS_MACSEC_DEFAULT_ACTION_DROP) ? 1 : 0;
    CSR_WRM(port_no, VTSS_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_NCP,
            (drop ? VTSS_F_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_NCP_NCP_TAGGED_FLOW_TYPE : 0) |// bypass / drop
            VTSS_F_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_NCP_NCP_TAGGED_DEST_PORT(3) |          // bypass to uncontrolled port
            VTSS_F_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_NCP_NCP_TAGGED_DROP_ACTION(2),         // drop silently
            VTSS_F_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_NCP_NCP_TAGGED_FLOW_TYPE |
            VTSS_M_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_NCP_NCP_TAGGED_DEST_PORT |
            VTSS_M_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_NCP_NCP_TAGGED_DROP_ACTION);
    /* Bad Tag */
    CSR_WRM(port_no, VTSS_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_NCP,
            VTSS_F_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_NCP_NCP_BADTAG_FLOW_TYPE |             // DROP
            VTSS_F_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_NCP_NCP_BADTAG_DEST_PORT(3) |          // bypass to uncontrolled port
            VTSS_F_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_NCP_NCP_BADTAG_DROP_ACTION(2),         // drop silently
            VTSS_F_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_NCP_NCP_BADTAG_FLOW_TYPE |
            VTSS_M_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_NCP_NCP_BADTAG_DEST_PORT |
            VTSS_M_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_NCP_NCP_BADTAG_DROP_ACTION);
    /* Kay Tag */
    CSR_WRM(port_no, VTSS_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_NCP,
            (drop ? VTSS_F_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_NCP_NCP_KAY_FLOW_TYPE : 0) |// bypass / drop
            VTSS_F_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_NCP_NCP_KAY_DEST_PORT(3) |          // bypass to uncontrolled port
            VTSS_F_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_NCP_NCP_KAY_DROP_ACTION(2),         // drop silently
            VTSS_F_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_NCP_NCP_KAY_FLOW_TYPE |
            VTSS_M_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_NCP_NCP_KAY_DEST_PORT |
            VTSS_M_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_NCP_NCP_KAY_DROP_ACTION);

    /* Ingress. Control and macsec tagged */
    drop = (pattern->ingress_control_and_macsec == VTSS_MACSEC_DEFAULT_ACTION_DROP) ? 1 : 0;
    CSR_WRM(port_no, VTSS_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_CP,
            (drop ? VTSS_F_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_CP_CP_TAGGED_FLOW_TYPE : 0) |// bypass / drop
            VTSS_F_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_CP_CP_TAGGED_DEST_PORT(3) |          // bypass to uncontrolled port
            VTSS_F_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_CP_CP_TAGGED_DROP_ACTION(2),         // drop silently
            VTSS_F_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_CP_CP_TAGGED_FLOW_TYPE |
            VTSS_M_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_CP_CP_TAGGED_DEST_PORT |
            VTSS_M_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_CP_CP_TAGGED_DROP_ACTION);
    /* Bad Tag is handled as macsec tagged */
    CSR_WRM(port_no, VTSS_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_CP,
            VTSS_F_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_CP_CP_BADTAG_FLOW_TYPE |             // DROP
            VTSS_F_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_CP_CP_BADTAG_DEST_PORT(3) |          // bypass to uncontrolled port
            VTSS_F_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_CP_CP_BADTAG_DROP_ACTION(2),         // drop silently
            VTSS_F_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_CP_CP_BADTAG_FLOW_TYPE |
            VTSS_M_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_CP_CP_BADTAG_DEST_PORT |
            VTSS_M_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_CP_CP_BADTAG_DROP_ACTION);
    /* Kay is handled as macsec taggged */
    CSR_WRM(port_no, VTSS_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_CP,
            (drop ? VTSS_F_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_CP_CP_KAY_FLOW_TYPE : 0) |// bypass / drop
            VTSS_F_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_CP_CP_KAY_DEST_PORT(3) |          // bypass to uncontrolled port
            VTSS_F_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_CP_CP_KAY_DROP_ACTION(2),         // drop silently
            VTSS_F_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_CP_CP_KAY_FLOW_TYPE |
            VTSS_M_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_CP_CP_KAY_DEST_PORT |
            VTSS_M_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_CP_CP_KAY_DROP_ACTION);

    /* Ingress. Control and not macsec tagged */
    drop = (pattern->ingress_control_and_non_macsec == VTSS_MACSEC_DEFAULT_ACTION_DROP) ? 1 : 0;
    CSR_WRM(port_no, VTSS_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_CP,
            (drop ? VTSS_F_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_CP_CP_UNTAGGED_FLOW_TYPE : 0) |// bypass / drop
            VTSS_F_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_CP_CP_UNTAGGED_DEST_PORT(3) |          // bypass to uncontrolled port
            VTSS_F_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_CP_CP_UNTAGGED_DROP_ACTION(2),         // drop silently
            VTSS_F_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_CP_CP_UNTAGGED_FLOW_TYPE |
            VTSS_M_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_CP_CP_UNTAGGED_DEST_PORT |
            VTSS_M_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_CP_CP_UNTAGGED_DROP_ACTION);

    /* Egress. Non-control frames */
    drop = (pattern->egress_non_control == VTSS_MACSEC_DEFAULT_ACTION_DROP) ? 1 : 0;
    CSR_WRM(port_no, VTSS_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_NCP,
            (drop ? VTSS_F_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_NCP_NCP_UNTAGGED_FLOW_TYPE : 0) |// bypass / drop
            VTSS_F_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_NCP_NCP_UNTAGGED_DEST_PORT(0) |          // bypass to common port
            VTSS_F_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_NCP_NCP_UNTAGGED_DROP_ACTION(2),         // drop silently
            VTSS_F_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_NCP_NCP_UNTAGGED_FLOW_TYPE |
            VTSS_M_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_NCP_NCP_UNTAGGED_DEST_PORT |
            VTSS_M_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_NCP_NCP_UNTAGGED_DROP_ACTION);
    /* Tagged is handled as above */
    CSR_WRM(port_no, VTSS_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_NCP,
            (drop ? VTSS_F_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_NCP_NCP_TAGGED_FLOW_TYPE : 0) |// bypass / drop
            VTSS_F_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_NCP_NCP_TAGGED_DEST_PORT(0) |          // bypass to common port
            VTSS_F_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_NCP_NCP_TAGGED_DROP_ACTION(2),         // drop silently
            VTSS_F_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_NCP_NCP_TAGGED_FLOW_TYPE |
            VTSS_M_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_NCP_NCP_TAGGED_DEST_PORT |
            VTSS_M_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_NCP_NCP_TAGGED_DROP_ACTION);
    /* Bad Tag is handled as above */
    CSR_WRM(port_no, VTSS_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_NCP,
            VTSS_F_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_NCP_NCP_BADTAG_FLOW_TYPE |             // DROP
            VTSS_F_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_NCP_NCP_BADTAG_DEST_PORT(0) |          // bypass to common port
            VTSS_F_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_NCP_NCP_BADTAG_DROP_ACTION(2),         // drop silently
            VTSS_F_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_NCP_NCP_BADTAG_FLOW_TYPE |
            VTSS_M_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_NCP_NCP_BADTAG_DEST_PORT |
            VTSS_M_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_NCP_NCP_BADTAG_DROP_ACTION);
    /* Kay is handled as above */
    CSR_WRM(port_no, VTSS_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_NCP,
            (drop ? VTSS_F_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_NCP_NCP_KAY_FLOW_TYPE : 0) |// bypass / drop
            VTSS_F_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_NCP_NCP_KAY_DEST_PORT(0) |          // bypass to common port
            VTSS_F_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_NCP_NCP_KAY_DROP_ACTION(2),         // drop silently
            VTSS_F_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_NCP_NCP_KAY_FLOW_TYPE |
            VTSS_M_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_NCP_NCP_KAY_DEST_PORT |
            VTSS_M_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_NCP_NCP_KAY_DROP_ACTION);

    /* Egress. Control frames */
    drop = (pattern->egress_control == VTSS_MACSEC_DEFAULT_ACTION_DROP) ? 1 : 0;
    CSR_WRM(port_no, VTSS_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_CP,
            (drop ? VTSS_F_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_CP_CP_UNTAGGED_FLOW_TYPE : 0) |// bypass / drop
            VTSS_F_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_CP_CP_UNTAGGED_DEST_PORT(0) |          // bypass to commom port
            VTSS_F_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_CP_CP_UNTAGGED_DROP_ACTION(2),         // drop silently
            VTSS_F_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_CP_CP_UNTAGGED_FLOW_TYPE |
            VTSS_M_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_CP_CP_UNTAGGED_DEST_PORT |
            VTSS_M_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_CP_CP_UNTAGGED_DROP_ACTION);
    /* Tagged is handled as above */
    CSR_WRM(port_no, VTSS_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_CP,
            (drop ? VTSS_F_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_CP_CP_TAGGED_FLOW_TYPE : 0) |// bypass / drop
            VTSS_F_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_CP_CP_TAGGED_DEST_PORT(0) |          // bypass to commom port
            VTSS_F_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_CP_CP_TAGGED_DROP_ACTION(2),         // drop silently
            VTSS_F_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_CP_CP_TAGGED_FLOW_TYPE |
            VTSS_M_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_CP_CP_TAGGED_DEST_PORT |
            VTSS_M_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_CP_CP_TAGGED_DROP_ACTION);
    /* Bad Tagged is handled as above */
    CSR_WRM(port_no, VTSS_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_CP,
            VTSS_F_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_CP_CP_BADTAG_FLOW_TYPE |             // DROP
            VTSS_F_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_CP_CP_BADTAG_DEST_PORT(0) |          // bypass to commom port
            VTSS_F_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_CP_CP_BADTAG_DROP_ACTION(2),         // drop silently
            VTSS_F_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_CP_CP_BADTAG_FLOW_TYPE |
            VTSS_M_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_CP_CP_BADTAG_DEST_PORT |
            VTSS_M_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_CP_CP_BADTAG_DROP_ACTION);
    /* KAY is handled as above */
    CSR_WRM(port_no, VTSS_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_CP,
            (drop ? VTSS_F_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_CP_CP_KAY_FLOW_TYPE : 0) |// bypass / drop
            VTSS_F_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_CP_CP_KAY_DEST_PORT(0) |          // bypass to commom port
            VTSS_F_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_CP_CP_KAY_DROP_ACTION(2),         // drop silently
            VTSS_F_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_CP_CP_KAY_FLOW_TYPE |
            VTSS_M_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_CP_CP_KAY_DEST_PORT |
            VTSS_M_MACSEC_EGR_FRAME_MATCHING_HANDLING_CTRL_SAM_NM_FLOW_CP_CP_KAY_DROP_ACTION);

    return VTSS_RC_OK;
}


static vtss_rc vtss_macsec_control_frame_match_conf_set_priv(vtss_state_t                                 *vtss_state,
                                                             const vtss_port_no_t                         port_no,
                                                             const vtss_macsec_control_frame_match_conf_t *const conf,
                                                             u32                                          *const rule_id,
                                                             BOOL                                          store)
{
    u32 i, parsed_etype = 1, indx;

    /* Get the next available index for CP rule */
    VTSS_RC(cp_rule_id_get(vtss_state, port_no, conf, &indx, rule_id, store));

    if ((vtss_state->macsec_conf[port_no].glb.bypass_mode.mode == VTSS_MACSEC_BYPASS_HDR) ||
        ((conf->match & VTSS_MACSEC_MATCH_ETYPE) && (conf->etype == 0x8100))) {
        parsed_etype = 0;
    }

    /* Apply to both ingress and egress */
    for (i = 0; i < 2; i++) {
        if (conf->match & VTSS_MACSEC_MATCH_ETYPE) {
            /* Use parsed Etype, after VLAN (if there is one) expect for configurations as above */
            if (indx < 10) {
                CSR_WRM(port_no,
                        PST_DIR(VTSS, i, CTL_PACKET_CLASS_PARAMS2_CP_MATCH_MODE),
                        PST_DIR(VTSS_F, i, CTL_PACKET_CLASS_PARAMS2_CP_MATCH_MODE_ETYPE_SEL_9_TO_0(parsed_etype ? VTSS_BIT(indx) : 0)),
                        PST_DIR(VTSS_F, i, CTL_PACKET_CLASS_PARAMS2_CP_MATCH_MODE_ETYPE_SEL_9_TO_0(parsed_etype ? VTSS_BIT(indx) : 0)));
            } else {
                CSR_WRM(port_no,
                        PST_DIR(VTSS, i, CTL_PACKET_CLASS_PARAMS2_CP_MATCH_MODE),
                        PST_DIR(VTSS_F, i, CTL_PACKET_CLASS_PARAMS2_CP_MATCH_MODE_ETYPE_SEL_17_TO_10(parsed_etype ? VTSS_BIT(indx - 10) : 0)),
                        PST_DIR(VTSS_F, i, CTL_PACKET_CLASS_PARAMS2_CP_MATCH_MODE_ETYPE_SEL_17_TO_10(parsed_etype ? VTSS_BIT(indx - 10) : 0)));
            }
        }

        if ((conf->match & VTSS_MACSEC_MATCH_ETYPE) && (conf->match & VTSS_MACSEC_MATCH_DMAC)) {
            /* DMAC AND Etype => index 8-9 */
            CSR_WR(port_no,
                   PST_DIR(VTSS, i, CTL_PACKET_CLASS_PARAMS_CP_MAC_DA_MATCH(indx)), get_u32(&conf->dmac.addr[0]));

            CSR_WR(port_no,
                   PST_DIR(VTSS,   i, CTL_PACKET_CLASS_PARAMS_CP_MAC_DA_ET_MATCH(indx)),
                   PST_DIR(VTSS_F, i, CTL_PACKET_CLASS_PARAMS_CP_MAC_DA_ET_MATCH_MAC_DA_MATCH_15_TO_0(conf->dmac.addr[5] << 8  | conf->dmac.addr[4])) |
                   PST_DIR(VTSS_F, i, CTL_PACKET_CLASS_PARAMS_CP_MAC_DA_ET_MATCH_ETHER_TYPE_MATCH(MACSEC_BS(conf->etype))));

            /* Enable */
            CSR_WRM(port_no,
                    PST_DIR(VTSS, i, CTL_PACKET_CLASS_PARAMS2_CP_MATCH_ENABLE),
                    (!store ? 0 : PST_DIR(VTSS_F, i, CTL_PACKET_CLASS_PARAMS2_CP_MATCH_ENABLE_COMB_ENABLE_9_TO_8(indx == 8 ? 1 : 2))),
                    PST_DIR(VTSS_F, i, CTL_PACKET_CLASS_PARAMS2_CP_MATCH_ENABLE_COMB_ENABLE_9_TO_8(indx == 8 ? 1 : 2)));

        } else if (conf->match & VTSS_MACSEC_MATCH_ETYPE) {
            /* Etype ONLY => use index 0-7,10-17 */
            if (indx < 8) {
                /* /\* Etype *\/ */
                CSR_WRM(port_no,
                        PST_DIR(VTSS,   i, CTL_PACKET_CLASS_PARAMS_CP_MAC_DA_ET_MATCH(indx)),
                        PST_DIR(VTSS_F, i, CTL_PACKET_CLASS_PARAMS_CP_MAC_DA_ET_MATCH_ETHER_TYPE_MATCH(MACSEC_BS(conf->etype))),
                        PST_DIR(VTSS_M, i, CTL_PACKET_CLASS_PARAMS_CP_MAC_DA_ET_MATCH_ETHER_TYPE_MATCH));

                /* Enable */
                CSR_WRM(port_no,
                        PST_DIR(VTSS, i, CTL_PACKET_CLASS_PARAMS2_CP_MATCH_ENABLE),
                        (!store ? 0 : PST_DIR(VTSS_F, i, CTL_PACKET_CLASS_PARAMS2_CP_MATCH_ENABLE_ETYPE_ENABLE_7_TO_0(VTSS_BIT(indx)))),
                        PST_DIR(VTSS_F, i, CTL_PACKET_CLASS_PARAMS2_CP_MATCH_ENABLE_ETYPE_ENABLE_7_TO_0(VTSS_BIT(indx))));
            } else {
                /* Etype */
                CSR_WR(port_no, VTSS_IOREG(0x1f, 1, (i ? 0x9e14 : 0x1e14) + (indx - 10)), // Workaround for a bug in Register List
                       VTSS_F_MACSEC_INGR_CTL_PACKET_CLASS_PARAMS_CP_MAC_ET_MATCH_ETHER_TYPE(MACSEC_BS(conf->etype)));

                /* Enable */
                CSR_WRM(port_no,
                        PST_DIR(VTSS, i, CTL_PACKET_CLASS_PARAMS2_CP_MATCH_ENABLE),
                        (!store ? 0 : PST_DIR(VTSS_F, i, CTL_PACKET_CLASS_PARAMS2_CP_MATCH_ENABLE_ETYPE_ENABLE_17_TO_10(VTSS_BIT(indx - 10)))),
                        PST_DIR(VTSS_F, i, CTL_PACKET_CLASS_PARAMS2_CP_MATCH_ENABLE_ETYPE_ENABLE_17_TO_10(VTSS_BIT(indx - 10))));
            }
        } else if (conf->match & VTSS_MACSEC_MATCH_DMAC) {
            /* DMAC ONLY => use index 0-7 */
            CSR_WR(port_no,
                   PST_DIR(VTSS, i, CTL_PACKET_CLASS_PARAMS_CP_MAC_DA_MATCH(indx)), get_u32(&conf->dmac.addr[0]));

            CSR_WRM(port_no,
                    PST_DIR(VTSS,   i, CTL_PACKET_CLASS_PARAMS_CP_MAC_DA_ET_MATCH(indx)),
                    PST_DIR(VTSS_F, i, CTL_PACKET_CLASS_PARAMS_CP_MAC_DA_ET_MATCH_MAC_DA_MATCH_15_TO_0(conf->dmac.addr[5] << 8 | conf->dmac.addr[4])),
                    PST_DIR(VTSS_M, i, CTL_PACKET_CLASS_PARAMS_CP_MAC_DA_ET_MATCH_MAC_DA_MATCH_15_TO_0));

            /* Enable */
            CSR_WRM(port_no,
                    PST_DIR(VTSS, i, CTL_PACKET_CLASS_PARAMS2_CP_MATCH_ENABLE),
                    (!store ? 0 : PST_DIR(VTSS_F, i, CTL_PACKET_CLASS_PARAMS2_CP_MATCH_ENABLE_MAC_DA_ENABLE_7_TO_0(VTSS_BIT(indx)))),
                    PST_DIR(VTSS_F, i, CTL_PACKET_CLASS_PARAMS2_CP_MATCH_ENABLE_MAC_DA_ENABLE_7_TO_0(VTSS_BIT(indx))));
        } else if (conf->match & VTSS_MACSEC_MATCH_DISABLE) {
            VTSS_E("Use vtss_macsec_control_frame_match_conf_del() to delete a rule");
            return VTSS_RC_ERROR;
        } else {
            VTSS_E("No control frame match");
            return VTSS_RC_ERROR;
        }
    }

    /* Workaround for Tagged frames with matching ETYPE */
    if (store && conf->match == VTSS_MACSEC_MATCH_ETYPE) {
        vtss_macsec_match_pattern_t pattern;
        memset(&pattern, 0, sizeof(pattern));
        if (conf->etype == 0x8100) {
            pattern.match = VTSS_MACSEC_MATCH_HAS_VLAN;
            pattern.has_vlan_tag = 1;
        } else {
            pattern.match = VTSS_MACSEC_MATCH_ETYPE |  VTSS_MACSEC_MATCH_HAS_VLAN;
            pattern.etype = conf->etype;
            pattern.has_vlan_tag = 1;
        }
        pattern.priority = 0;
        if ((vtss_macsec_pattern_set_priv(vtss_state, port_no, MACSEC_SECY_NONE, VTSS_MACSEC_DIRECTION_EGRESS,
                                          VTSS_MACSEC_MATCH_ACTION_UNCONTROLLED_PORT, &pattern, *rule_id)) != VTSS_RC_OK) {
            VTSS_E("Could no set bypass pattern for CP rule");
            return VTSS_RC_ERROR;
        }
    }
    /* Remove the workaround and delete the API internals  */
    if (!store && vtss_state->macsec_conf[port_no].glb.egr_bypass_record[*rule_id] != MACSEC_NOT_IN_USE) {
        u32 record = vtss_state->macsec_conf[port_no].glb.egr_bypass_record[*rule_id];
        if (macsec_sa_enable(vtss_state, port_no, record, EGRESS, MACSEC_DISABLE) != VTSS_RC_OK) {
            VTSS_E("Could enable the SA");
            return VTSS_RC_ERROR;
        }
        vtss_state->macsec_conf[port_no].tx_sa[record].in_use = 0;
        vtss_state->macsec_conf[port_no].glb.egr_bypass_record[*rule_id] = MACSEC_NOT_IN_USE;
        vtss_state->macsec_conf[port_no].glb.control_match[*rule_id].match = VTSS_MACSEC_MATCH_DISABLE;
    }

    return VTSS_RC_OK;
}


static vtss_rc vtss_macsec_rx_sa_get_priv(vtss_state_t                  *vtss_state,
                                          const u32                     secy_id,
                                          const vtss_macsec_port_t      port,
                                          const vtss_macsec_sci_t       *const sci,
                                          const u16                     an,
                                          u32                           *const lowest_pn,
                                          vtss_macsec_sak_t             *const sak,
                                          BOOL                          *const active)

{
    u32 sc, record, val;
    BOOL aes_128;
    vtss_macsec_internal_secy_t *secy = &vtss_state->macsec_conf[port.port_no].secy[secy_id];

    if (sc_from_sci_get(secy, sci, &sc) != VTSS_RC_OK) {
        VTSS_E("Could not find SC (from sci)");
        return VTSS_RC_ERROR;
    }
    VTSS_MACSEC_ASSERT(an >= VTSS_MACSEC_SA_PER_SC_MAX, "AN is invalid");
    VTSS_MACSEC_ASSERT(secy->rx_sc[sc]->sa[an] == NULL, "AN does not exist");
    aes_128 = (secy->conf.current_cipher_suite == VTSS_MACSEC_CIPHER_SUITE_GCM_AES_128) ? 1 : 0;
    record = secy->rx_sc[sc]->sa[an]->record;
    if (aes_128) {
        CSR_RD(port.port_no, VTSS_MACSEC_INGR_XFORM_RECORD_REGS_XFORM_REC_DATA10(record), &val);
    } else {
        CSR_RD(port.port_no, VTSS_MACSEC_INGR_XFORM_RECORD_REGS_XFORM_REC_DATA14(record), &val);
    }
    *lowest_pn = val - secy->rx_sc[sc]->conf.replay_window;
    *sak = secy->rx_sc[sc]->sa[an]->sak;
    *active = secy->rx_sc[sc]->sa[an]->status.in_use;

    return VTSS_RC_OK;
}

// Macro for reading two 32 bits counter registers into a 64 bit software counter
#define MACSEC_CNT64_RD(port_no, reg_low, reg_up, cnt)  \
    { \
    u32 lower, upper;\
    CSR_RD(port_no, reg_low, &lower);  \
    CSR_RD(port_no, reg_up, &upper); \
    VTSS_N("low:%u, up:%u", lower, upper);    \
    cnt = ((u64)upper << 32) + lower;\
    }


// ** TX_SA counters **
static vtss_rc vtss_macsec_tx_sa_counters_get_priv(vtss_state_t                    *vtss_state,
                                                   const vtss_port_no_t            port_no,
                                                   const u16                       an,
                                                   vtss_macsec_tx_sa_counters_t    *const counters,
                                                   u32                              secy_id)
{
    vtss_macsec_internal_secy_t *secy = &vtss_state->macsec_conf[port_no].secy[secy_id];
    u64 cnt;


    memset(counters, 0, sizeof(vtss_macsec_tx_sa_counters_t));

    if (!secy->tx_sc.in_use) {
        VTSS_I("TX_SC does not exist");
        return VTSS_RC_OK;
    }

    if (!secy->tx_sc.sa[an]) {
        return VTSS_RC_OK;
    }


    u32 record = secy->tx_sc.sa[an]->record;
    // Encrypted and protected shares the same counters. Which once that is using the counters depends upon confidentiality
    MACSEC_CNT64_RD(port_no,
                    VTSS_MACSEC_EGR_SA_STATS_EGR_OUT_PKTS_ENCRYPTED_LOWER(record),
                    VTSS_MACSEC_EGR_SA_STATS_EGR_OUT_PKTS_ENCRYPTED_UPPER(record),
                    cnt);

    VTSS_N("encrypted cnt:%" PRIu64 ", %" PRIu64 ", an:%u, secy_id:%u", cnt, vtss_state->macsec_conf[port_no].tx_sa[an].cnt.out_pkts_encrypted, an, secy_id);
    if (secy->tx_sc.sa[an]->confidentiality) {
        vtss_state->macsec_conf[port_no].tx_sa[an].cnt.out_pkts_encrypted += cnt;
        vtss_state->macsec_conf[port_no].tx_sa[an].cnt.out_pkts_protected = 0;
    } else {
        vtss_state->macsec_conf[port_no].tx_sa[an].cnt.out_pkts_encrypted = 0;
        vtss_state->macsec_conf[port_no].tx_sa[an].cnt.out_pkts_protected += cnt;
    }

    VTSS_MACSEC_ASSERT(an >= VTSS_MACSEC_SA_PER_SC_MAX, "AN is invalid");

    VTSS_D("encrypted cnt:%" PRIu64 ", %" PRIu64 ", an:%u, secy_id:%u", cnt, vtss_state->macsec_conf[port_no].tx_sa[an].cnt.out_pkts_encrypted, an, secy_id);
    // Pass the counters
    memcpy(counters, &vtss_state->macsec_conf[port_no].tx_sa[an].cnt, sizeof(vtss_macsec_tx_sa_counters_t));
    return VTSS_RC_OK;
}

// ** TX_SC counters **
static vtss_rc vtss_macsec_tx_sc_counters_get_priv(vtss_state_t                    *vtss_state,
                                                   const vtss_port_no_t            port_no,
                                                   vtss_macsec_tx_sc_counters_t    *const counters,
                                                   u32                              secy_id)
{
    u32 an;
    vtss_macsec_tx_sa_counters_t tx_sa_counters;
    vtss_state->macsec_conf[port_no].secy[secy_id].tx_sc.cnt.out_pkts_encrypted = 0;
    vtss_state->macsec_conf[port_no].secy[secy_id].tx_sc.cnt.out_pkts_protected = 0;


    for (an = 0; an < VTSS_MACSEC_SA_PER_SC_MAX; an ++ ) {
        VTSS_RC(vtss_macsec_tx_sa_counters_get_priv(vtss_state, port_no, an, &tx_sa_counters, secy_id));
        VTSS_N("encrypted cnt:%" PRIu64 ", %" PRIu64 "", vtss_state->macsec_conf[port_no].secy[secy_id].tx_sc.cnt.out_pkts_encrypted, tx_sa_counters.out_pkts_encrypted);
        vtss_state->macsec_conf[port_no].secy[secy_id].tx_sc.cnt.out_pkts_encrypted += tx_sa_counters.out_pkts_encrypted;
        vtss_state->macsec_conf[port_no].secy[secy_id].tx_sc.cnt.out_pkts_protected += tx_sa_counters.out_pkts_protected;
    }

    // Pass the counters
    memcpy(counters, &vtss_state->macsec_conf[port_no].secy[secy_id].tx_sc.cnt, sizeof(vtss_macsec_tx_sc_counters_t));
    return VTSS_RC_OK;
}

static vtss_rc sa_sam_in_flight(vtss_state_t  *vtss_state, vtss_port_no_t p, BOOL egr)
{
    u32 val, count = 0;
    do {
        if (egr) {
            CSR_RD(p, VTSS_MACSEC_EGR_SA_MATCH_CTL_PARAMS_SAM_IN_FLIGHT, &val);
        } else {
            CSR_RD(p, VTSS_MACSEC_INGR_SA_MATCH_CTL_PARAMS_SAM_IN_FLIGHT, &val);
        }
        VTSS_MSLEEP(1);
        count++;
        if (count == 100) {
            VTSS_E("timeout, bailing out");
            return VTSS_RC_ERROR;
        }
    } while (VTSS_F_MACSEC_INGR_SA_MATCH_CTL_PARAMS_SAM_IN_FLIGHT_UNSAFE(val));
    return VTSS_RC_OK;
}


static vtss_rc vtss_macsec_tx_sa_del_priv(vtss_state_t              *vtss_state,
                                          const u32                 secy_id,
                                          const vtss_macsec_port_t  port,
                                          const u16                 an)
{
    vtss_macsec_internal_secy_t *secy = &vtss_state->macsec_conf[port.port_no].secy[secy_id];
    u32 record, i, sa_in_use = 0;
    vtss_macsec_tx_sc_counters_t dummy_tx_counters;

    VTSS_MACSEC_ASSERT(an >= VTSS_MACSEC_SA_PER_SC_MAX, "AN is invalid");
    VTSS_MACSEC_ASSERT(secy->tx_sc.sa[an] == NULL, "AN is not existing");

    record = secy->tx_sc.sa[an]->record;

    // Wait until the unsafe field has reached zero, i.e. while there are packet in the system
    if (sa_sam_in_flight(vtss_state, port.port_no, EGRESS) != VTSS_RC_OK) {
        VTSS_E("Could not empty the egress pipeline");
        return VTSS_RC_ERROR;
    }

    // Update SC counters before the SA is deleted.
    if (vtss_macsec_tx_sc_counters_get_priv(vtss_state, port.port_no, &dummy_tx_counters, secy_id) != VTSS_RC_OK) {
        VTSS_E("Could not update Tx SC counters");
    }

    for (i = 0; i < VTSS_MACSEC_SA_PER_SC_MAX; i++ ) {
        if (secy->tx_sc.sa[i] == NULL || (i == an)) {
            continue;
        }
        if (secy->tx_sc.sa[i]->in_use) {
            sa_in_use = 1;
            break;
        }
    }
    if (!sa_in_use) {
        /* Last SA in use, must update the SC stopped_time */
        secy->tx_sc.status.stopped_time = secy->tx_sc.sa[an]->status.stopped_time;
    }

    memset(&vtss_state->macsec_conf[port.port_no].tx_sa[record], 0, sizeof(vtss_state->macsec_conf[port.port_no].tx_sa[record]));
    secy->tx_sc.sa[an] = NULL;
    return VTSS_RC_OK;
}

static vtss_rc vtss_macsec_secy_counters_get_priv(vtss_state_t                  *vtss_state,
                                                  const vtss_port_no_t          port_no,
                                                  vtss_macsec_secy_counters_t   *const counters,
                                                  u32                           secy_id)
{
    u16 an;
    vtss_macsec_internal_secy_t *secy = &vtss_state->macsec_conf[port_no].secy[secy_id];
    u32 record;
    u64 cnt;
    BOOL ev_bit = FALSE;
    u16 sc;

    // TX an
    for (an = 0; an < VTSS_MACSEC_SA_PER_SC_MAX; an ++ ) {
        if (secy->tx_sc.sa[an] == NULL || !secy->tx_sc.in_use) {
            continue;
        }
        record = secy->tx_sc.sa[an]->record;

        // Too long
        MACSEC_CNT64_RD(port_no,
                        VTSS_MACSEC_EGR_SA_STATS_EGR_OUT_PKTS_TOO_LONG_LOWER(record),
                        VTSS_MACSEC_EGR_SA_STATS_EGR_OUT_PKTS_TOO_LONG_UPPER(record),
                        cnt);

        vtss_state->macsec_conf[port_no].secy[secy_id].secy_cnt.out_pkts_too_long += cnt;

        // Encrypted and protected shares the same counters.
        // Which once that is using the counters depends upon confidentiality (tp.ebit)
        MACSEC_CNT64_RD(port_no,
                        VTSS_MACSEC_EGR_SA_STATS_EGR_OUT_OCTETS_ENCRYPTED_FIRST_CNTR_LOWER(record),
                        VTSS_MACSEC_EGR_SA_STATS_EGR_OUT_OCTETS_ENCRYPTED_FIRST_CNTR_UPPER(record),
                        cnt);

        if (secy->tx_sc.sa[an]->confidentiality) {
            secy->secy_cnt.out_octets_encrypted += cnt;
        } else {
            secy->secy_cnt.out_octets_protected += cnt;
        }
        VTSS_D("secy->tx_sc.sa[%d]->confidentiality:%u, cnt:%" PRIu64 "", an, secy->tx_sc.sa[an]->confidentiality, cnt);

        // The following comes from Sailesh Rupani (The MACSEC chip designer)
        // Generally the peer device will always send all frames on an SA with confidentiality (rv.ebit == 1) or without confidentiality (rv.ebit == 0).Hence in principal the hardware only needs to implement one counter.
        // For a given SecY, it is known through management if
        //      a)  Confidentiality will be used or not.
        //      b)  If confidentiality is used, what offset is used.
        // Such an information is given through management variable "confidentiality offset".
        //        Per 9.7.1 of 802.1X 2010 :
        // "A participant that believes itself to be the Key Server and its KaY's principal actor encodes the following
        // information with each MACsec SAK that it distributes,

        // The following information is also distributed with each MACsec SAK:
        //   b) Confidentiality Offset, indicating whether confidentiality is to be provided, and whether an offset of
        //  0, 30, or 50 octets is used (see IEEE Std 802.1AE-2006)."

        // Per table 11-6 of 802.1X-2010, the following is the encoding of confidentiality offset.
        // Confidentiality Offset 2-bit
        //    0 if confidentiality not used, 1 if confidentiality with no offset, 2 if offset = 30, 3 if offset = 50.
        //  Hence, SW needs to use  this information (whether confidentiality will be used or not) to distinguish between the counters.
        // Looking at the 802.1X standard I conclude that, you can also use this information from TX direction from any of an active SA, but keep in mind that the "an" values are different in TX and RX direction
        ev_bit = secy->tx_sc.sa[an]->confidentiality; // Note - only one TX AN in use at the time.
    }
    VTSS_N("ev_bit:%d", ev_bit);

    // RX an
    for (sc = 0; sc < VTSS_MACSEC_MAX_SC_RX; sc++) {
        if (secy->rx_sc[sc] == NULL || !secy->rx_sc[sc]->in_use) {
            continue;
        }

        for (an = 0; an < VTSS_MACSEC_SA_PER_SC_MAX; an ++ ) {
            if (secy->rx_sc[sc]->sa[an] == NULL || !secy->rx_sc[sc]->sa[an]->in_use) {
                continue;
            }

            record = secy->rx_sc[sc]->sa[an]->record;

            // Decrypted and validated shares the same counters.
            MACSEC_CNT64_RD(port_no,
                            VTSS_MACSEC_INGR_SA_STATS_IGR_IN_OCTETS_DECRYPTED_FIRST_CNTR_LOWER(record),
                            VTSS_MACSEC_INGR_SA_STATS_IGR_IN_OCTETS_DECRYPTED_FIRST_CNTR_UPPER(record),
                            cnt);

            VTSS_N("secy->conf.validate_frames:%u, secy->conf.protect_frames:%u, ev_bit:%d", secy->conf.validate_frames, secy->conf.protect_frames, ev_bit);

            // (From 802.1AE figure 10.5) if ((validateFrames != Disabled) && !rv.ebit) { rv.Valid = integrity_check(rv);
            // (From 802.1AE figure 10.5)    InOctetsValidated += #Plaintext_octets;};
            if (secy->conf.validate_frames != VTSS_MACSEC_VALIDATE_FRAMES_DISABLED && !(ev_bit)) {
                vtss_state->macsec_conf[port_no].secy[secy_id].secy_cnt.in_octets_validated += cnt;
                vtss_state->macsec_conf[port_no].secy[secy_id].secy_cnt.in_octets_decrypted = 0;
            }

            // (From 802.1AE figure 10.5) if ((validateFrames != Disabled) && rv.ebit) { rv.Valid = integrity_check_and_decrypt(rv);
            // (From 802.1AE figure 10.5)     InOctetsDecrypted += #Plaintext_octets;};
            if (secy->conf.validate_frames != VTSS_MACSEC_VALIDATE_FRAMES_DISABLED && ev_bit) {
                vtss_state->macsec_conf[port_no].secy[secy_id].secy_cnt.in_octets_validated = 0;
                vtss_state->macsec_conf[port_no].secy[secy_id].secy_cnt.in_octets_decrypted += cnt;
            }
        }
    }

    // in_pkts_no_sci
    MACSEC_CNT64_RD(port_no,
                    VTSS_MACSEC_INGR_GLOBAL_STATS_IGR_IN_PKTS_NO_SCI_LOWER,
                    VTSS_MACSEC_INGR_GLOBAL_STATS_IGR_IN_PKTS_NO_SCI_UPPER,
                    cnt);

    vtss_state->macsec_conf[port_no].secy[secy_id].secy_cnt.in_pkts_no_sci += cnt;

    //in_pkts_unknown_sci
    MACSEC_CNT64_RD(port_no,
                    VTSS_MACSEC_INGR_GLOBAL_STATS_IGR_IN_PKTS_UNKNOWN_SCI_LOWER,
                    VTSS_MACSEC_INGR_GLOBAL_STATS_IGR_IN_PKTS_UNKNOWN_SCI_UPPER,
                    cnt);

    vtss_state->macsec_conf[port_no].secy[secy_id].secy_cnt.in_pkts_unknown_sci += cnt;

    // in_pkts_bad_tag
    MACSEC_CNT64_RD(port_no,
                    VTSS_MACSEC_INGR_GLOBAL_STATS_IGR_IN_PKTS_BAD_TAG_LOWER,
                    VTSS_MACSEC_INGR_GLOBAL_STATS_IGR_IN_PKTS_BAD_TAG_UPPER,
                    cnt);

    vtss_state->macsec_conf[port_no].secy[secy_id].secy_cnt.in_pkts_bad_tag += cnt;

    // Untagged
    MACSEC_CNT64_RD(port_no,
                    VTSS_MACSEC_EGR_GLOBAL_STATS_EGR_OUT_PKTS_UNTAGGED_LOWER,
                    VTSS_MACSEC_EGR_GLOBAL_STATS_EGR_OUT_PKTS_UNTAGGED_UPPER,
                    cnt);


    vtss_state->macsec_conf[port_no].secy[secy_id].secy_cnt.out_pkts_untagged += cnt;


    //  in_pkts_no_tag
    MACSEC_CNT64_RD(port_no,
                    VTSS_MACSEC_INGR_GLOBAL_STATS_IGR_IN_PKTS_NO_TAG_LOWER,
                    VTSS_MACSEC_INGR_GLOBAL_STATS_IGR_IN_PKTS_NO_TAG_UPPER,
                    cnt);

    vtss_state->macsec_conf[port_no].secy[secy_id].secy_cnt.in_pkts_no_tag += cnt;

    //in_pkts_untagged
    MACSEC_CNT64_RD(port_no,
                    VTSS_MACSEC_INGR_GLOBAL_STATS_IGR_IN_PKTS_UNTAGGED_LOWER,
                    VTSS_MACSEC_INGR_GLOBAL_STATS_IGR_IN_PKTS_UNTAGGED_UPPER,
                    cnt);

    vtss_state->macsec_conf[port_no].secy[secy_id].secy_cnt.in_pkts_untagged += cnt;

    // Overrun - condition does not occur, report as zero (page 78 in HW3)  .'
    vtss_state->macsec_conf[port_no].secy[secy_id].secy_cnt.in_pkts_overrun = 0;


    // Return counters value
    memcpy(counters, &vtss_state->macsec_conf[port_no].secy[secy_id].secy_cnt, sizeof(*counters));
    return VTSS_RC_OK;
}

static vtss_rc vtss_macsec_secy_cnt_update(vtss_state_t             *vtss_state,
                                           const vtss_port_no_t     port_no,
                                           u32                      secy_id)
{
    vtss_macsec_secy_counters_t  dummy_counters;
    VTSS_RC(vtss_macsec_secy_counters_get_priv(vtss_state,
                                               port_no,
                                               &dummy_counters,
                                               secy_id));
    return VTSS_RC_OK;
}


static vtss_rc vtss_macsec_rx_sa_counters_get_priv(vtss_state_t                 *vtss_state,
                                                   const vtss_port_no_t         port_no,
                                                   const vtss_macsec_sci_t      *const sci,
                                                   const u16                    an,
                                                   vtss_macsec_rx_sa_counters_t *const counters,
                                                   u32                          secy_id)
{
    vtss_macsec_internal_secy_t *secy = &vtss_state->macsec_conf[port_no].secy[secy_id];
    u32 sc;
    u32 record;
    u64 cnt;
    memset(counters, 0, sizeof(vtss_macsec_rx_sa_counters_t));

    // Update the counter for this secy_id
    VTSS_RC(vtss_macsec_secy_cnt_update(vtss_state, port_no, secy_id));
    VTSS_RC(sc_from_sci_get(secy, sci, &sc));

    if (secy->rx_sc[sc] == NULL || !secy->rx_sc[sc]->in_use || secy->rx_sc[sc]->sa[an] == NULL || !secy->rx_sc[sc]->sa[an]->in_use) {
        return VTSS_RC_OK;
    }

    VTSS_MACSEC_ASSERT(secy->rx_sc[sc] == NULL, "SC does not exist");
    VTSS_MACSEC_ASSERT(secy->rx_sc[sc]->sa[an] == NULL, "AN does not exist");
    record = secy->rx_sc[sc]->sa[an]->record;
    MACSEC_CNT64_RD(port_no,
                    VTSS_MACSEC_INGR_SA_STATS_IGR_IN_PKTS_OK_LOWER(record),
                    VTSS_MACSEC_INGR_SA_STATS_IGR_IN_PKTS_OK_UPPER(record),
                    cnt);

    secy->rx_sc[sc]->sa[an]->cnt.in_pkts_ok += cnt;
    VTSS_N("cnt:%" PRIu64 ", record:%u", cnt, record);

    MACSEC_CNT64_RD(port_no,
                    VTSS_MACSEC_INGR_SA_STATS_IGR_IN_PKTS_INVALID_LOWER(record),
                    VTSS_MACSEC_INGR_SA_STATS_IGR_IN_PKTS_INVALID_UPPER(record),
                    cnt);

    secy->rx_sc[sc]->sa[an]->cnt.in_pkts_invalid += cnt;

    MACSEC_CNT64_RD(port_no,
                    VTSS_MACSEC_INGR_SA_STATS_IGR_IN_PKTS_NOT_VALID_LOWER(record),
                    VTSS_MACSEC_INGR_SA_STATS_IGR_IN_PKTS_NOT_VALID_UPPER(record),
                    cnt);

    secy->rx_sc[sc]->sa[an]->cnt.in_pkts_not_valid += cnt;

    MACSEC_CNT64_RD(port_no,
                    VTSS_MACSEC_INGR_SA_STATS_IGR_IN_PKTS_NOT_USING_SA_LOWER(record),
                    VTSS_MACSEC_INGR_SA_STATS_IGR_IN_PKTS_NOT_USING_SA_UPPER(record),
                    cnt);

    secy->rx_sc[sc]->sa[an]->cnt.in_pkts_not_using_sa += cnt;
    VTSS_N("in_pkts_not_using_sa:%" PRIu64, cnt);

    MACSEC_CNT64_RD(port_no,
                    VTSS_MACSEC_INGR_SA_STATS_IGR_IN_PKTS_UNUSED_SA_LOWER(record),
                    VTSS_MACSEC_INGR_SA_STATS_IGR_IN_PKTS_UNUSED_SA_UPPER(record),
                    cnt);

    secy->rx_sc[sc]->sa[an]->cnt.in_pkts_unused_sa += cnt;


    MACSEC_CNT64_RD(port_no,
                    VTSS_MACSEC_INGR_SA_STATS_IGR_IN_PKTS_UNCHECKED_LOWER(record),
                    VTSS_MACSEC_INGR_SA_STATS_IGR_IN_PKTS_UNCHECKED_UPPER(record),
                    cnt);

    secy->rx_sc[sc]->sa[an]->cnt.in_pkts_unchecked += cnt;

    MACSEC_CNT64_RD(port_no,
                    VTSS_MACSEC_INGR_SA_STATS_IGR_IN_PKTS_DELAYED_LOWER(record),
                    VTSS_MACSEC_INGR_SA_STATS_IGR_IN_PKTS_DELAYED_UPPER(record),
                    cnt);
    secy->rx_sc[sc]->sa[an]->cnt.in_pkts_delayed += cnt;

    MACSEC_CNT64_RD(port_no,
                    VTSS_MACSEC_INGR_SA_STATS_IGR_IN_PKTS_LATE_LOWER(record),
                    VTSS_MACSEC_INGR_SA_STATS_IGR_IN_PKTS_LATE_UPPER(record),
                    cnt);
    secy->rx_sc[sc]->sa[an]->cnt.in_pkts_late += cnt;
    VTSS_D("rx_sa_counters.in_pkts_late:%" PRIu64 "", secy->rx_sc[sc]->sa[an]->cnt.in_pkts_late);

    // Pass the counters
    memcpy(counters, &secy->rx_sc[sc]->sa[an]->cnt, sizeof(vtss_macsec_rx_sa_counters_t));
    return VTSS_RC_OK;
}

// ** RX_SC counters **
static vtss_rc vtss_macsec_rx_sc_counters_get_priv(vtss_state_t                    *vtss_state,
                                                   const vtss_port_no_t            port_no,
                                                   const vtss_macsec_sci_t         *const sci,
                                                   vtss_macsec_rx_sc_counters_t    *const counters,
                                                   u32                              secy_id)
{
    u32 sc;
    u32 an;

    vtss_macsec_internal_secy_t *secy = &vtss_state->macsec_conf[port_no].secy[secy_id];
    vtss_macsec_rx_sa_counters_t rx_sa_counters;

    VTSS_RC(sc_from_sci_get(secy, sci, &sc));

    VTSS_MACSEC_ASSERT(secy->rx_sc[sc] == NULL, "SC does not exist");

    memset(&secy->rx_sc[sc]->cnt, 0, sizeof(vtss_macsec_rx_sc_counters_t));

    for (an = 0; an < VTSS_MACSEC_SA_PER_SC_MAX; an ++ ) {
        if (secy->rx_sc[sc]->sa[an] == NULL || !secy->rx_sc[sc]->sa[an]->in_use) {
            continue;
        }

        VTSS_RC(vtss_macsec_rx_sa_counters_get_priv(vtss_state, port_no, sci, an, &rx_sa_counters, secy_id));

        secy->rx_sc[sc]->cnt.in_pkts_ok            += rx_sa_counters.in_pkts_ok;
        secy->rx_sc[sc]->cnt.in_pkts_invalid       += rx_sa_counters.in_pkts_invalid;
        secy->rx_sc[sc]->cnt.in_pkts_not_valid     += rx_sa_counters.in_pkts_not_valid;
        secy->rx_sc[sc]->cnt.in_pkts_not_using_sa  += rx_sa_counters.in_pkts_not_using_sa;
        secy->rx_sc[sc]->cnt.in_pkts_unused_sa     += rx_sa_counters.in_pkts_unused_sa;
        secy->rx_sc[sc]->cnt.in_pkts_unchecked     += rx_sa_counters.in_pkts_unchecked;
        secy->rx_sc[sc]->cnt.in_pkts_delayed       += rx_sa_counters.in_pkts_delayed;
        secy->rx_sc[sc]->cnt.in_pkts_late          += rx_sa_counters.in_pkts_late;
        VTSS_D("rx_sa_counters.in_pkts_late:%" PRIu64 "", rx_sa_counters.in_pkts_late);
    }

    // Pass the counters
    memcpy(counters, &secy->rx_sc[sc]->cnt, sizeof(vtss_macsec_rx_sc_counters_t));
    return VTSS_RC_OK;
}

static vtss_rc vtss_macsec_rx_sa_del_priv(vtss_state_t              *vtss_state,
                                          const u32                 secy_id,
                                          const vtss_macsec_port_t  port,
                                          const vtss_macsec_sci_t   *const sci,
                                          const u16                 an)
{
    vtss_macsec_internal_secy_t *secy = &vtss_state->macsec_conf[port.port_no].secy[secy_id];
    u32 sc, record, sa_in_use = 0, i;
    vtss_macsec_rx_sc_counters_t dummy_counters;

    VTSS_MACSEC_ASSERT(an >= VTSS_MACSEC_SA_PER_SC_MAX, "AN is invalid");

    if (sc_from_sci_get(secy, sci, &sc) != VTSS_RC_OK) {
        VTSS_E("Could not find SC (from sci)");
        return VTSS_RC_ERROR;
    }
    VTSS_MACSEC_ASSERT(secy->rx_sc[sc]->sa[an] == NULL, "AN does not exist");
    if (!secy->rx_sc[sc]->sa[an]) {
        VTSS_E("AN not created. AN=%hu", an);
        return VTSS_RC_ERROR;
    }
    record = secy->rx_sc[sc]->sa[an]->record;

    // Wait until the unsafe field has reached zero, i.e. while there are packet in the system
    if (sa_sam_in_flight(vtss_state, port.port_no, INGRESS) != VTSS_RC_OK) {
        VTSS_E("Could not empty the ingress pipeline");
        return VTSS_RC_ERROR;
    }

    // update the SC counter before the SA is deleted
    if (vtss_macsec_rx_sc_counters_get_priv(vtss_state, port.port_no, sci, &dummy_counters, secy_id) != VTSS_RC_OK) {
        VTSS_E("Could not update Rx SC counter");
    }

    for (i = 0; i < VTSS_MACSEC_SA_PER_SC_MAX; i++ ) {
        if (secy->rx_sc[sc]->sa[i] == NULL || (i == an)) {
            continue;
        }
        if (secy->rx_sc[sc]->sa[i]->in_use) {
            sa_in_use = 1;
            break;
        }
    }

    if (!sa_in_use) {
        secy->rx_sc[sc]->status.stopped_time = secy->rx_sc[sc]->sa[an]->status.stopped_time;
    }
    memset(&vtss_state->macsec_conf[port.port_no].rx_sa[record], 0, sizeof(vtss_state->macsec_conf[port.port_no].rx_sa[record]));
    secy->rx_sc[sc]->sa[an] = NULL;

    return VTSS_RC_OK;
}

static vtss_rc vtss_macsec_rx_sc_del_priv(vtss_state_t              *vtss_state,
                                          const vtss_macsec_port_t  port,
                                          const vtss_macsec_sci_t   *sci,
                                          const u32                 secy_id)
{
    vtss_macsec_internal_secy_t *secy = &vtss_state->macsec_conf[port.port_no].secy[secy_id];
    u32 sc, sa;
    if (sc_from_sci_get(secy, sci, &sc) != VTSS_RC_OK) {
        VTSS_E("No SC found");
        return VTSS_RC_ERROR;
    }

    for (sa = 0; sa < VTSS_MACSEC_SA_PER_SC; sa++) {
        if (secy->rx_sc[sc]->sa[sa] != NULL) {

            if (secy->rx_sc[sc]->sa[sa]->status.in_use) {
                if (vtss_macsec_rx_sa_disable_priv(vtss_state, secy_id, port, sci, sa) != VTSS_RC_OK) {
                    VTSS_E("Could not disable sa:%u", sa);
                    return VTSS_RC_ERROR;
                }
            }
            if (vtss_macsec_rx_sa_del_priv(vtss_state, secy_id, port, sci, sa)) {
                VTSS_E("Could not delete sa:%u", sa);
                return VTSS_RC_ERROR;
            }
        }
    }
    memset(secy->rx_sc[sc], 0, sizeof(*secy->rx_sc[sc]));
    secy->rx_sc[sc] = NULL;
    VTSS_RC(macsec_update_glb_validate(vtss_state, port.port_no));
    return VTSS_RC_OK;
}

static vtss_rc vtss_macsec_tx_sc_del_priv(vtss_state_t              *vtss_state,
                                          const vtss_macsec_port_t  port,
                                          const u32                 secy_id)
{
    vtss_macsec_internal_secy_t *secy = &vtss_state->macsec_conf[port.port_no].secy[secy_id];
    u32 sa;

    if (!secy->tx_sc.in_use) {
        VTSS_E("TX_SC does not exist");
        return VTSS_RC_ERROR;
    }

    for (sa = 0; sa < VTSS_MACSEC_SA_PER_SC; sa++) {
        if (secy->tx_sc.sa[sa] != NULL) {
            if (vtss_macsec_tx_sa_disable_priv(vtss_state, secy_id, port, sa)) {
                VTSS_E("Could not disable sa:%u", sa);
                return VTSS_RC_ERROR;
            }
            if (vtss_macsec_tx_sa_del_priv(vtss_state, secy_id, port, sa)) {
                VTSS_E("Could not delete sa:%u", sa);
                return VTSS_RC_ERROR;
            }
        }
    }
    memset(&secy->tx_sc, 0, sizeof(secy->tx_sc));
    return VTSS_RC_OK;
}

static vtss_rc vtss_macsec_pattern_del_priv(vtss_state_t                       *vtss_state,
                                            const vtss_macsec_port_t           port,
                                            const u32                          secy_id,
                                            const vtss_macsec_direction_t      direction,
                                            const vtss_macsec_match_action_t   action)
{
    vtss_macsec_internal_secy_t *secy = &vtss_state->macsec_conf[port.port_no].secy[secy_id];
    BOOL egr = (direction == VTSS_MACSEC_DIRECTION_EGRESS) ? 1 : 0;
    u32 record;
    record = secy->pattern_record[action][direction];

    if (record == MACSEC_NOT_IN_USE) {
        VTSS_E("Pattern not set");
        return VTSS_RC_ERROR;
    }
    /* Disable the SA */
    if (macsec_sa_enable(vtss_state, port.port_no, record, egr, MACSEC_DISABLE) != VTSS_RC_OK) {
        VTSS_E("Could disable the SA");
    }
    if (egr) {
        vtss_state->macsec_conf[port.port_no].tx_sa[record].in_use = 0;
    } else {
        vtss_state->macsec_conf[port.port_no].rx_sa[record].in_use = 0;
    }
    secy->pattern_record[action][direction] = MACSEC_NOT_IN_USE;
    secy->pattern[action][direction].match = VTSS_MACSEC_MATCH_DISABLE;
    return VTSS_RC_OK;
}


static vtss_rc vtss_macsec_secy_conf_del_priv(vtss_state_t *vtss_state,
                                              const vtss_macsec_port_t port, const u32 secy_id)
{
    vtss_macsec_internal_secy_t *secy = &vtss_state->macsec_conf[port.port_no].secy[secy_id];
    u16 sc;

    if (secy->tx_sc.in_use) {
        VTSS_RC(vtss_macsec_tx_sc_del_priv(vtss_state, port, secy_id));
    }

    for (sc = 0; sc < VTSS_MACSEC_MAX_SC_RX; sc++) {
        if (secy->rx_sc[sc] == NULL) {
            continue;
        }
        VTSS_RC(vtss_macsec_rx_sc_del_priv(vtss_state, port, &secy->rx_sc[sc]->sci, secy_id));
    }

    if (secy->pattern_record[VTSS_MACSEC_MATCH_ACTION_CONTROLLED_PORT][VTSS_MACSEC_DIRECTION_INGRESS] != MACSEC_NOT_IN_USE) {
        VTSS_RC(vtss_macsec_pattern_del_priv(vtss_state, port, secy_id, VTSS_MACSEC_DIRECTION_INGRESS, VTSS_MACSEC_MATCH_ACTION_CONTROLLED_PORT));
    }
    if (secy->pattern_record[VTSS_MACSEC_MATCH_ACTION_UNCONTROLLED_PORT][VTSS_MACSEC_DIRECTION_INGRESS] != MACSEC_NOT_IN_USE) {
        VTSS_RC(vtss_macsec_pattern_del_priv(vtss_state, port, secy_id, VTSS_MACSEC_DIRECTION_INGRESS, VTSS_MACSEC_MATCH_ACTION_UNCONTROLLED_PORT));
    }
    if (secy->pattern_record[VTSS_MACSEC_MATCH_ACTION_DROP][VTSS_MACSEC_DIRECTION_INGRESS] != MACSEC_NOT_IN_USE) {
        VTSS_RC(vtss_macsec_pattern_del_priv(vtss_state, port, secy_id, VTSS_MACSEC_DIRECTION_INGRESS, VTSS_MACSEC_MATCH_ACTION_DROP));
    }

    if (secy->pattern_record[VTSS_MACSEC_MATCH_ACTION_CONTROLLED_PORT][VTSS_MACSEC_DIRECTION_EGRESS] != MACSEC_NOT_IN_USE) {
        VTSS_RC(vtss_macsec_pattern_del_priv(vtss_state, port, secy_id, VTSS_MACSEC_DIRECTION_EGRESS, VTSS_MACSEC_MATCH_ACTION_CONTROLLED_PORT));
    }
    if (secy->pattern_record[VTSS_MACSEC_MATCH_ACTION_UNCONTROLLED_PORT][VTSS_MACSEC_DIRECTION_EGRESS] != MACSEC_NOT_IN_USE) {
        VTSS_RC(vtss_macsec_pattern_del_priv(vtss_state, port, secy_id, VTSS_MACSEC_DIRECTION_EGRESS, VTSS_MACSEC_MATCH_ACTION_UNCONTROLLED_PORT));
    }
    if (secy->pattern_record[VTSS_MACSEC_MATCH_ACTION_DROP][VTSS_MACSEC_DIRECTION_EGRESS] != MACSEC_NOT_IN_USE) {
        VTSS_RC(vtss_macsec_pattern_del_priv(vtss_state, port, secy_id, VTSS_MACSEC_DIRECTION_EGRESS, VTSS_MACSEC_MATCH_ACTION_DROP));
    }

    memset(secy, 0, sizeof(*secy));
    for (sc = 0; sc < VTSS_MACSEC_MAX_SC_RX; sc++) {
        secy->rx_sc[sc] = NULL;
    }
    return VTSS_RC_OK;
}

static vtss_rc vtss_macsec_secy_controlled_set_priv(vtss_state_t              *vtss_state,
                                                    const vtss_macsec_port_t  port,
                                                    const BOOL                enable,
                                                    const u32                 secy_id)
{
    vtss_macsec_internal_secy_t *secy;
    u32 sa, sc;
    secy = &vtss_state->macsec_conf[port.port_no].secy[secy_id];

    if (secy->controlled_port_enabled == enable) {
        return VTSS_RC_OK; // Nothing to do
    }
    for (sc = 0; sc < VTSS_MACSEC_MAX_SC_RX; sc++) {
        if (secy->rx_sc[sc] == NULL) {
            continue;
        }
        for (sa = 0; sa < VTSS_MACSEC_SA_PER_SC; sa++) {
            if (secy->rx_sc[sc]->sa[sa] == NULL) {
                continue;
            }
            if (secy->rx_sc[sc]->sa[sa]->enabled) {
                if (macsec_sa_enable(vtss_state, port.port_no, secy->rx_sc[sc]->sa[sa]->record, INGRESS, enable) != VTSS_RC_OK) {
                    VTSS_E("Could not disable/enable Rx sa:%u", sa);
                    return VTSS_RC_ERROR;
                }
            }
        }
    }

    for (sa = 0; sa < VTSS_MACSEC_SA_PER_SC; sa++) {
        if (secy->tx_sc.sa[sa] != NULL) {
            if (secy->tx_sc.sa[sa]->enabled) {
                if (macsec_sa_enable(vtss_state, port.port_no, secy->tx_sc.sa[sa]->record, EGRESS, enable) != VTSS_RC_OK) {
                    VTSS_E("Could not disable/enable Tx sa:%u", sa);
                    return VTSS_RC_ERROR;
                }
            }
        }
    }

    secy->controlled_port_enabled = enable;
    return VTSS_RC_OK;
}

static vtss_rc vtss_macsec_secy_port_status_get_priv(vtss_state_t                      *vtss_state,
                                                     const vtss_macsec_port_t          port,
                                                     vtss_macsec_secy_port_status_t    *const status,
                                                     const u32                         secy_id,
                                                     BOOL                              fdx)
{
    vtss_macsec_internal_secy_t *secy;
    secy = &vtss_state->macsec_conf[port.port_no].secy[secy_id];
    BOOL transmitting = 0, strict = 0;
    u32 sc_in_use = 0, sc, an;

    if (secy->conf.validate_frames == VTSS_MACSEC_VALIDATE_FRAMES_STRICT) {
        strict = 1;
    }
    for (sc = 0; sc < VTSS_MACSEC_MAX_SC_RX; sc++) {
        if (secy->rx_sc[sc] == NULL) {
            continue;
        }
        for (an = 0; an < VTSS_MACSEC_SA_PER_SC_MAX; an++) {
            if (secy->rx_sc[sc]->sa[an] == NULL) {
                continue;
            }
            if (secy->rx_sc[sc]->sa[an]->in_use) {
                sc_in_use++;
                break;
            }
        }
    }
    for (an = 0; an < VTSS_MACSEC_SA_PER_SC_MAX; an++) {
        if (secy->tx_sc.sa[an] == NULL) {
            continue;
        }
        if (secy->tx_sc.sa[an]->in_use) {
            transmitting = 1;
        }
    }

    // 802.1AE 10.7.2
    status->common.mac_enabled = 1;               // The common MAC is always enabled
    status->common.mac_operational = 1;           // The common MAC is always operational
    status->common.oper_point_to_point_mac = fdx; // Duplex as reported by the Phy

    // 802.1AE 10.7.2
    status->uncontrolled = status->common;

    // 802.1AE 10.7.4
    status->controlled.mac_enabled = secy->controlled_port_enabled && status->common.mac_enabled && transmitting && (sc_in_use > 0);
    status->controlled.mac_operational = status->controlled.mac_enabled && status->common.mac_operational;
    status->controlled.oper_point_to_point_mac = (strict && (sc_in_use < 2)) || (!strict && status->common.oper_point_to_point_mac);
    return VTSS_RC_OK;
}


static vtss_rc vtss_macsec_secy_controlled_get_priv(vtss_state_t              *vtss_state,
                                                    const vtss_macsec_port_t  port,
                                                    BOOL                      *const enable,
                                                    const u32                 secy_id)
{
    vtss_macsec_internal_secy_t *secy;
    secy = &vtss_state->macsec_conf[port.port_no].secy[secy_id];

    *enable = secy->controlled_port_enabled;
    return VTSS_RC_OK;
}

static vtss_rc vtss_macsec_rx_sc_add_priv(vtss_state_t              *vtss_state,
                                          const vtss_macsec_port_t  port,
                                          const vtss_macsec_sci_t   *sci,
                                          const u32                 secy_id)
{
    vtss_macsec_internal_secy_t *secy = &vtss_state->macsec_conf[port.port_no].secy[secy_id];
    u16 sc_secy = 0, sc_conf = 0, sc;
    BOOL found_sc_in_secy = 0, found_sc_in_conf = 0;
    vtss_timeofday_t tod;

    if (!check_resources(vtss_state, port.port_no, 1, secy_id)) {
        VTSS_E("HW resources exhausted");
        return VTSS_RC_ERROR;
    }

    VTSS_RC(is_sci_valid(sci));

    for (sc = 0; sc < VTSS_MACSEC_MAX_SC_RX; sc++) {
        if (sci_cmp(sci, &vtss_state->macsec_conf[port.port_no].rx_sc[sc].sci)) {
            VTSS_E("SCI already exists");
            return VTSS_RC_ERROR;
        }

    }

    for (sc = 0; sc < VTSS_MACSEC_MAX_SC_RX; sc++) {
        if (secy->rx_sc[sc] == NULL && !found_sc_in_secy) {
            sc_secy = sc;
            found_sc_in_secy = 1;
        }
        if (!vtss_state->macsec_conf[port.port_no].rx_sc[sc].in_use && !found_sc_in_conf) {
            sc_conf = sc;
            found_sc_in_conf = 1;
        }
    }

    if (!(found_sc_in_secy && found_sc_in_conf)) {
        VTSS_E("Could not find SC resources");
        return VTSS_RC_ERROR;
    }

    VTSS_I("Adding new RX-SC: Port: "MACSEC_PORT_FMT " SCI: "SCI_FMT
           ", sc-in-secy:%u",
           MACSEC_PORT_ARG(&port), SCI_ARG(*sci), sc_secy);

    secy->rx_sc[sc_secy] = &vtss_state->macsec_conf[port.port_no].rx_sc[sc_conf];
    sci_cpy(&secy->rx_sc[sc_secy]->sci, sci);

    // The RxSC inherits the SecY configuration
    secy->rx_sc[sc_secy]->conf.validate_frames = secy->conf.validate_frames;
    secy->rx_sc[sc_secy]->conf.replay_protect = secy->conf.replay_protect;
    secy->rx_sc[sc_secy]->conf.replay_window = secy->conf.replay_window;
    secy->rx_sc[sc_secy]->conf.confidentiality_offset = secy->conf.confidentiality_offset;
    VTSS_TIME_OF_DAY(tod);
    secy->rx_sc[sc_secy]->status.created_time = tod.sec; // TimeOfDay in seconds
    secy->rx_sc[sc_secy]->in_use = 1;

    return VTSS_RC_OK;
}

static vtss_rc vtss_macsec_tx_sc_set_priv(vtss_state_t              *vtss_state,
                                          const vtss_macsec_port_t  port,
                                          const u32                 secy_id)
{
    vtss_macsec_internal_secy_t *secy = &vtss_state->macsec_conf[port.port_no].secy[secy_id];
    vtss_timeofday_t tod;

    VTSS_RC(is_sci_valid(&secy->sci));

    VTSS_TIME_OF_DAY(tod);
    secy->tx_sc.status.created_time = tod.sec; // 802.1AE 10.7.12
    secy->tx_sc.status.started_time = tod.sec; // 802.1AE 10.7.12
    secy->tx_sc.status.stopped_time = tod.sec; // 802.1AE 10.7.12
    secy->tx_sc.in_use = 1;
    memcpy(&secy->tx_sc.status.sci,  &secy->sci, sizeof(vtss_macsec_sci_t));
    return VTSS_RC_OK;
}


static vtss_rc vtss_macsec_rx_sa_set_priv(vtss_state_t                  *vtss_state,
                                          const u32                     secy_id,
                                          const vtss_macsec_port_t      port,
                                          const vtss_macsec_sci_t       *const sci,
                                          const u16                     an,
                                          const u32                     lowest_pn,
                                          const vtss_macsec_sak_t       *const sak)
{
    vtss_macsec_internal_secy_t *secy = &vtss_state->macsec_conf[port.port_no].secy[secy_id];
    vtss_macsec_match_pattern_t *match = &secy->pattern[VTSS_MACSEC_MATCH_ACTION_CONTROLLED_PORT][VTSS_MACSEC_DIRECTION_INGRESS];
    u32 sc, record;
    vtss_timeofday_t tod;

    if (sc_from_sci_get(secy, sci, &sc) != VTSS_RC_OK) {
        VTSS_E("Could not find SC (from sci)");
        return VTSS_RC_ERROR;
    }

    VTSS_RC(is_sci_valid(sci));

    VTSS_MACSEC_ASSERT(an >= VTSS_MACSEC_SA_PER_SC_MAX, "AN is invalid");
    if (secy->rx_sc[sc]->sa[an] != NULL) {
        VTSS_E("Rx AN:%u is in use", an);
        return VTSS_RC_ERROR;
    }

    if (record_empty_get(vtss_state, port.port_no, &record, INGRESS) != VTSS_RC_OK) {
        VTSS_E("Could not get an empty record");
        return VTSS_RC_ERROR;
    }
    secy->rx_sc[sc]->sa[an] = &vtss_state->macsec_conf[port.port_no].rx_sa[record];
    secy->rx_sc[sc]->sa[an]->record = record;
    secy->rx_sc[sc]->sa[an]->sak = *sak;
    secy->rx_sc[sc]->sa[an]->status.lowest_pn = lowest_pn;
    secy->rx_sc[sc]->sa[an]->in_use = 1;

    if (macsec_sa_match_set(vtss_state, port.port_no, INGRESS, record, match, secy, 1, sc, an, 0) != VTSS_RC_OK) {
        VTSS_E("Could not program the SA match");
        return VTSS_RC_ERROR;
    }
    if (macsec_sa_flow_set(vtss_state, port.port_no, INGRESS, record, secy, an, sc, VTSS_MACSEC_MATCH_ACTION_CONTROLLED_PORT) != VTSS_RC_OK) {
        VTSS_E("Could not program the SA flow");
        return VTSS_RC_ERROR;
    }
    if (macsec_sa_xform_set(vtss_state, port.port_no, INGRESS, record, secy, an, sc) != VTSS_RC_OK) {
        VTSS_E("Could not program the xform record");
        return VTSS_RC_ERROR;
    }
    if (macsec_sa_enable(vtss_state, port.port_no, record, INGRESS, MACSEC_ENABLE) != VTSS_RC_OK) {
        VTSS_E("Could not enable the SA");
    }

    VTSS_TIME_OF_DAY(tod);
    secy->rx_sc[sc]->sa[an]->status.created_time = tod.sec; // TimeOfDay in seconds */
    return VTSS_RC_OK;
}


static vtss_rc vtss_macsec_rx_sa_status_get_priv(vtss_state_t                *vtss_state,
                                                 const u32                   secy_id,
                                                 const vtss_macsec_port_t    port,
                                                 const vtss_macsec_sci_t     *const sci,
                                                 const u16                   an,
                                                 vtss_macsec_rx_sa_status_t  *const status)
{
    u32 next_pn, sc, record;
    BOOL aes_128;
    vtss_macsec_internal_secy_t *secy = &vtss_state->macsec_conf[port.port_no].secy[secy_id];

    if (sc_from_sci_get(secy, sci, &sc) != VTSS_RC_OK) {
        VTSS_E("No SC found");
        return VTSS_RC_OK;
    }
    VTSS_MACSEC_ASSERT(an >= VTSS_MACSEC_SA_PER_SC_MAX, "AN is invalid");
    VTSS_MACSEC_ASSERT(secy->rx_sc[sc]->sa[an] == NULL, "AN does not exist");
    VTSS_MACSEC_ASSERT(!secy->rx_sc[sc]->sa[an]->in_use, "AN is not active");

    record = secy->rx_sc[sc]->sa[an]->record;
    aes_128 = (secy->conf.current_cipher_suite == VTSS_MACSEC_CIPHER_SUITE_GCM_AES_128) ? 1 : 0;
    /* Read the next PN */
    if (aes_128) {
        CSR_RD(port.port_no, VTSS_MACSEC_INGR_XFORM_RECORD_REGS_XFORM_REC_DATA10(record), &next_pn);
    } else {
        CSR_RD(port.port_no, VTSS_MACSEC_INGR_XFORM_RECORD_REGS_XFORM_REC_DATA14(record), &next_pn);
    }

    secy->rx_sc[sc]->sa[an]->status.next_pn = next_pn;
    secy->rx_sc[sc]->sa[an]->status.lowest_pn = (secy->conf.replay_window >= next_pn) ? 0 : (next_pn - secy->conf.replay_window);

    *status = secy->rx_sc[sc]->sa[an]->status;
    return VTSS_RC_OK;
}


static vtss_rc vtss_macsec_rx_sa_lowest_pn_update_priv(vtss_state_t                  *vtss_state,
                                                       const u32                     secy_id,
                                                       const vtss_macsec_port_t      port,
                                                       const vtss_macsec_sci_t       *const sci,
                                                       const u16                     an,
                                                       const u32                     lowest_pn)
{
    u32 next_pn, sc, record;
    BOOL aes_128;
    vtss_macsec_internal_secy_t *secy = &vtss_state->macsec_conf[port.port_no].secy[secy_id];

    if (sc_from_sci_get(secy, sci, &sc) != VTSS_RC_OK) {
        VTSS_E("No SC found");
        return VTSS_RC_OK;
    }

    VTSS_RC(is_sci_valid(sci));

    VTSS_MACSEC_ASSERT(an >= VTSS_MACSEC_SA_PER_SC_MAX, "AN is invalid");
    VTSS_MACSEC_ASSERT(secy->rx_sc[sc]->sa[an] == NULL, "AN does not exist");
    VTSS_MACSEC_ASSERT(!secy->rx_sc[sc]->sa[an]->in_use, "AN is not active");
    record = secy->rx_sc[sc]->sa[an]->record;
    aes_128 = (secy->conf.current_cipher_suite == VTSS_MACSEC_CIPHER_SUITE_GCM_AES_128) ? 1 : 0;
    /* Read the next PN */
    if (aes_128) {
        CSR_RD(port.port_no, VTSS_MACSEC_INGR_XFORM_RECORD_REGS_XFORM_REC_DATA10(record), &next_pn);
    } else {
        CSR_RD(port.port_no, VTSS_MACSEC_INGR_XFORM_RECORD_REGS_XFORM_REC_DATA14(record), &next_pn);
    }

    if (lowest_pn > (next_pn + secy->conf.replay_window)) {
        u32 new_record;
        vtss_macsec_match_pattern_t *match = &secy->pattern[VTSS_MACSEC_MATCH_ACTION_CONTROLLED_PORT][VTSS_MACSEC_DIRECTION_INGRESS];
        VTSS_D("SA must be changed! (lowest_pn > next_pn + replay_window). Lowest_pn:%u next_pn:%u replay_window:%u",
               lowest_pn, next_pn, secy->conf.replay_window);
        /* The exisiting SA must be replaced by an new SA */
        secy->rx_sc[sc]->sa[an]->status.lowest_pn = lowest_pn;

        if (record_empty_get(vtss_state, port.port_no, &new_record, INGRESS) != VTSS_RC_OK) {
            VTSS_E("Could not get an empty record");
            return VTSS_RC_ERROR;
        }

        memcpy(&vtss_state->macsec_conf[port.port_no].rx_sa[new_record],
               &vtss_state->macsec_conf[port.port_no].rx_sa[record], sizeof(vtss_macsec_internal_rx_sa_t));
        memset(&vtss_state->macsec_conf[port.port_no].rx_sa[record], 0, sizeof(vtss_state->macsec_conf[port.port_no].rx_sa[record]));
        secy->rx_sc[sc]->sa[an] = &vtss_state->macsec_conf[port.port_no].rx_sa[new_record];
        secy->rx_sc[sc]->sa[an]->record = new_record;

        if (macsec_sa_match_set(vtss_state, port.port_no, INGRESS, new_record, match, secy, 1, sc, an, 0) != VTSS_RC_OK) {
            VTSS_E("Could not program the SA match");
            return VTSS_RC_ERROR;
        }
        if (macsec_sa_flow_set(vtss_state, port.port_no, INGRESS, new_record, secy, an, sc,
                               VTSS_MACSEC_MATCH_ACTION_CONTROLLED_PORT) != VTSS_RC_OK) {
            VTSS_E("Could not program the SA flow");
            return VTSS_RC_ERROR;
        }
        if (macsec_sa_xform_set(vtss_state, port.port_no, INGRESS, new_record, secy, an, sc) != VTSS_RC_OK) {
            VTSS_E("Could not program the xform record");
            return VTSS_RC_ERROR;
        }
        if (macsec_sa_toggle(vtss_state, port.port_no, record, new_record, INGRESS) != VTSS_RC_OK) {
            VTSS_E("Could not toggle SA:%u -> %u", record, new_record);
            return VTSS_RC_ERROR;
        }
        if (macsec_sa_inuse(vtss_state, port.port_no, record, INGRESS, MACSEC_ENABLE) != VTSS_RC_OK) {
            VTSS_E("Could not set SA:%u to 'in_use'", record);
            return VTSS_RC_ERROR;
        }

    }
    return VTSS_RC_OK;
}

static BOOL one_tx_an_in_use(vtss_macsec_internal_secy_t *secy)
{
    u32 an;
    u32 an_in_use = 0;

    for (an = 0; an < VTSS_MACSEC_SA_PER_SC_MAX; an++ ) {
        if (secy->tx_sc.sa[an] == NULL) {
            continue;
        }
        if (secy->tx_sc.sa[an]->in_use) {
            an_in_use++;
        }
    }
    return (an_in_use > 1) ? 0 : 1;
}

static vtss_rc vtss_macsec_tx_sa_set_priv(vtss_state_t                   *vtss_state,
                                          const u32                      secy_id,
                                          const vtss_macsec_port_t       port,
                                          const u16                      an,
                                          const u32                      next_pn,
                                          const BOOL                     confidentiality,
                                          const vtss_macsec_sak_t        *const sak)
{
    vtss_macsec_internal_secy_t *secy = &vtss_state->macsec_conf[port.port_no].secy[secy_id];
    vtss_macsec_match_pattern_t *match = &secy->pattern[VTSS_MACSEC_MATCH_ACTION_CONTROLLED_PORT][VTSS_MACSEC_DIRECTION_EGRESS];
    u32 record;
    vtss_timeofday_t tod;


    VTSS_MACSEC_ASSERT(an >= VTSS_MACSEC_SA_PER_SC_MAX, "AN is invalid");
    VTSS_MACSEC_ASSERT(!secy->tx_sc.in_use,             "No TxSC installed");

    if (secy->tx_sc.sa[an] != NULL) {
        VTSS_E("Tx AN:%u is in use", an);
        return VTSS_RC_ERROR;
    }

    if (record_empty_get(vtss_state, port.port_no, &record, EGRESS) != VTSS_RC_OK) {
        VTSS_E("Could not get an empty record");
        return VTSS_RC_ERROR;
    }

    secy->tx_sc.sa[an] = &vtss_state->macsec_conf[port.port_no].tx_sa[record];
    secy->tx_sc.sa[an]->record = record;
    secy->tx_sc.sa[an]->sak = *sak;
    secy->tx_sc.sa[an]->confidentiality = confidentiality;
    secy->tx_sc.sa[an]->status.next_pn = next_pn;
    secy->tx_sc.sa[an]->in_use = 1;
    if (macsec_sa_match_set(vtss_state, port.port_no, EGRESS, record, match, secy, 0, 0, 0, 0) != VTSS_RC_OK) {
        VTSS_E("Could not program the SA match");
        return VTSS_RC_ERROR;
    }
    if (macsec_sa_flow_set(vtss_state, port.port_no, EGRESS, record, secy, an, 0, VTSS_MACSEC_MATCH_ACTION_CONTROLLED_PORT) != VTSS_RC_OK) {
        VTSS_E("Could not program the SA flow");
        return VTSS_RC_ERROR;
    }
    if (macsec_sa_xform_set(vtss_state, port.port_no, EGRESS, record, secy, an, 0) != VTSS_RC_OK) {
        VTSS_E("Could not program the xform record");
        return VTSS_RC_ERROR;
    }

    if (one_tx_an_in_use(secy)) { /* When more than one AN are in use we use 'toggle' to change between SA's  */
        if (macsec_sa_enable(vtss_state, port.port_no, record, EGRESS, MACSEC_ENABLE) != VTSS_RC_OK) {
            VTSS_E("Could not enable the SA");
        }
    }

    VTSS_TIME_OF_DAY(tod);
    secy->tx_sc.sa[an]->status.created_time = tod.sec; // TimeOfDay in seconds

    return VTSS_RC_OK;
}


static vtss_rc vtss_macsec_tx_sa_activate_priv(vtss_state_t                  *vtss_state,
                                               const u32                     secy_id,
                                               const vtss_macsec_port_t      port,
                                               const u16                     an)
{
    vtss_macsec_internal_secy_t *secy = &vtss_state->macsec_conf[port.port_no].secy[secy_id];
    vtss_timeofday_t tod;
    u32 old_an;
    BOOL an_in_use = 0;

    VTSS_MACSEC_ASSERT(an >= VTSS_MACSEC_SA_PER_SC_MAX, "AN is invalid");
    VTSS_MACSEC_ASSERT(secy->tx_sc.sa[an] == NULL, "AN does not exist");
    VTSS_MACSEC_ASSERT(!secy->tx_sc.sa[an]->in_use, "AN is not in use");

    for (old_an = 0; old_an < VTSS_MACSEC_SA_PER_SC_MAX; old_an++ ) {
        if (secy->tx_sc.sa[old_an] == NULL || old_an == an) {
            continue;
        }
        if (secy->tx_sc.sa[old_an]->in_use) {
            an_in_use = 1;
            break;
        }
    }

    /* Activate chip SA Flow */
    if (an_in_use && (old_an < VTSS_MACSEC_SA_PER_SC_MAX)) {
        if (macsec_sa_inuse(vtss_state, port.port_no, secy->tx_sc.sa[an]->record, EGRESS, MACSEC_ENABLE) != VTSS_RC_OK) {
            VTSS_E("Could not set SA:%u to 'in_use'", secy->tx_sc.sa[an]->record);
            return VTSS_RC_ERROR;
        }
        if (macsec_sa_toggle(vtss_state, port.port_no, secy->tx_sc.sa[an]->record, secy->tx_sc.sa[old_an]->record, EGRESS) != VTSS_RC_OK) {
            VTSS_E("Could not toggle SA:%u -> %u", an, old_an);
            return VTSS_RC_ERROR;
        }
    } else {
        if (macsec_sa_inuse(vtss_state, port.port_no, secy->tx_sc.sa[an]->record, EGRESS, MACSEC_ENABLE) != VTSS_RC_OK) {
            VTSS_E("Could not set SA:%u to 'in_use'", secy->tx_sc.sa[an]->record);
            return VTSS_RC_ERROR;
        }
    }

    VTSS_TIME_OF_DAY(tod);
    secy->tx_sc.sa[an]->status.started_time = tod.sec; // TimeOfDay in seconds
    secy->tx_sc.sa[an]->enabled = 1;
    if (an_in_use && (old_an < VTSS_MACSEC_SA_PER_SC_MAX)) {
        secy->tx_sc.sa[old_an]->enabled = 0;
    }

    if (!an_in_use) {
        secy->tx_sc.status.started_time = tod.sec;
    }
    secy->tx_sc.status.encoding_sa = an;
    secy->tx_sc.status.enciphering_sa = an;
    return VTSS_RC_OK;
}


static vtss_rc vtss_macsec_rx_sa_activate_priv(vtss_state_t                  *vtss_state,
                                               const u32                     secy_id,
                                               const vtss_macsec_port_t      port,
                                               const vtss_macsec_sci_t       *const sci,
                                               const u16                     an)
{
    vtss_macsec_internal_secy_t *secy = &vtss_state->macsec_conf[port.port_no].secy[secy_id];
    vtss_timeofday_t tod;
    u32 sc, i, sa_in_use = 0;

    if (sc_from_sci_get(secy, sci, &sc) != VTSS_RC_OK) {
        VTSS_E("Could not find SC (from sci)");
        return VTSS_RC_ERROR;
    }

    VTSS_RC(is_sci_valid(sci));

    VTSS_MACSEC_ASSERT(an >= VTSS_MACSEC_SA_PER_SC_MAX, "AN is invalid");
    VTSS_MACSEC_ASSERT(secy->rx_sc[sc]->sa[an] == NULL, "AN does not exist");

    /* Activate chip SA Flow */
    if (macsec_sa_inuse(vtss_state, port.port_no, secy->rx_sc[sc]->sa[an]->record, INGRESS, MACSEC_ENABLE) != VTSS_RC_OK) {
        VTSS_E("Could not set SA:%u to 'in_use'", secy->rx_sc[sc]->sa[an]->record);
        return VTSS_RC_ERROR;
    }

    VTSS_TIME_OF_DAY(tod);
    secy->rx_sc[sc]->sa[an]->status.started_time = tod.sec; // TimeOfDay in seconds
    secy->rx_sc[sc]->sa[an]->status.in_use = 1;
    secy->rx_sc[sc]->sa[an]->enabled = 1;

    for (i = 0; i < VTSS_MACSEC_SA_PER_SC_MAX; i++ ) {
        if (secy->rx_sc[sc]->sa[i] == NULL || (i == an)) {
            continue;
        }
        if (secy->rx_sc[sc]->sa[i]->in_use) {
            sa_in_use = 1;
            break;
        }
    }
    if (!sa_in_use) {
        secy->rx_sc[sc]->status.started_time = tod.sec;
    }
    return VTSS_RC_OK;
}

static vtss_rc vtss_macsec_tx_sa_status_get_priv(vtss_state_t                *vtss_state,
                                                 u32                         secy_id,
                                                 const vtss_macsec_port_t    port,
                                                 const u16                   an)
{
    vtss_macsec_internal_secy_t *secy = &vtss_state->macsec_conf[port.port_no].secy[secy_id];
    u32 val, record;
    BOOL aes_128;

    VTSS_MACSEC_ASSERT(an >= VTSS_MACSEC_SA_PER_SC_MAX, "AN is invalid");
    VTSS_MACSEC_ASSERT(secy->tx_sc.sa[an] == NULL, "AN does not exist");
    record = secy->tx_sc.sa[an]->record;
    aes_128 = (secy->conf.current_cipher_suite == VTSS_MACSEC_CIPHER_SUITE_GCM_AES_128) ? 1 : 0;

    if (aes_128) {
        CSR_RD(port.port_no, VTSS_MACSEC_EGR_XFORM_RECORD_REGS_XFORM_REC_DATA10(record), &val);
    } else {
        CSR_RD(port.port_no, VTSS_MACSEC_EGR_XFORM_RECORD_REGS_XFORM_REC_DATA14(record), &val);
    }
    secy->tx_sc.sa[an]->status.next_pn = val;
    return VTSS_RC_OK;

}


// Function for setting sequencing number threshold
// In/out - Same as vtss_macsec_event_seq_threshold_set (except inst), See vtss_macsec_api.h
static vtss_rc vtss_macsec_event_seq_threshold_set_priv(vtss_state_t         *vtss_state,
                                                        const vtss_port_no_t port_no,
                                                        const u32            threshold)
{
    CSR_WR(port_no, VTSS_MACSEC_EGR_CRYPTO_CTRL_STATUS_REGS_SEQ_NUM_THRESH, threshold);
    return VTSS_RC_OK;
}


// Function for getting sequencing number threshold
// In/out - Same as vtss_macsec_event_seq_threshold_get (except inst), See vtss_macsec_api.h
static vtss_rc vtss_macsec_event_seq_threshold_get_priv(vtss_state_t         *vtss_state,
                                                        const vtss_port_no_t port_no,
                                                        u32                  *threshold)
{
    CSR_RD(port_no, VTSS_MACSEC_EGR_CRYPTO_CTRL_STATUS_REGS_SEQ_NUM_THRESH, threshold);
    return VTSS_RC_OK;
}

#define VTSS_MACSEC_THRESHOLD_EVENT_ENABLE_BIT 20 //See vtss_venice_reg_macsec_egr.h "Sequence Number Threshold"
#define VTSS_MACSEC_ROLLOVER_EVENT_ENABLE_BIT 21 //See vtss_venice_reg_macsec_egr.h "Sequence Number roll-over" 

#define VTSS_MACSEC_THRESHOLD_EVENT_STATUS_BIT 4 //See vtss_venice_reg_macsec_egr.h "Sequence Number Threshold"
#define VTSS_MACSEC_ROLLOVER_EVENT_STATUS_BIT 5 //See vtss_venice_reg_macsec_egr.h "Sequence Number roll-over" 

// Function for setting interrupt events
// In/out - Same as vtss_macsec_event_enable_set (except inst), See vtss_macsec_api.h
static vtss_rc vtss_macsec_event_enable_set_priv(vtss_state_t              *vtss_state,
                                                 const vtss_port_no_t      port_no,
                                                 const vtss_macsec_event_t ev_mask,
                                                 const BOOL                enable)
{
    u32 mask  = 0;
    u32 value = 0;

    if (ev_mask & VTSS_MACSEC_SEQ_THRESHOLD_EVENT) {
        mask |= 1 << VTSS_MACSEC_THRESHOLD_EVENT_ENABLE_BIT;
        if (enable) {
            value |= 1 << VTSS_MACSEC_THRESHOLD_EVENT_ENABLE_BIT;
        }
    }

    if (ev_mask & VTSS_MACSEC_SEQ_ROLLOVER_EVENT) {
        mask |= 1 << VTSS_MACSEC_ROLLOVER_EVENT_ENABLE_BIT;
        if (enable) {
            value |= 1 << VTSS_MACSEC_ROLLOVER_EVENT_ENABLE_BIT;
        }

    }

    CSR_WRM(port_no, VTSS_MACSEC_EGR_CRYPTO_CTRL_STATUS_REGS_INTR_CTRL_STATUS, value, mask);
    VTSS_I("value:0x%X, mask:0x%X", value, mask);
    return VTSS_RC_OK;
}

// Function for getting interrupt events
// In/out - Same as vtss_macsec_event_enable_get (except inst), See vtss_macsec_api.h
static vtss_rc vtss_macsec_event_enable_get_priv(vtss_state_t              *vtss_state,
                                                 const vtss_port_no_t      port_no,
                                                 vtss_macsec_event_t       *ev_mask)
{

    u32 value;
    CSR_RD(port_no, VTSS_MACSEC_EGR_CRYPTO_CTRL_STATUS_REGS_INTR_CTRL_STATUS, &value);

    *ev_mask = 0;

    if (value & (1 << VTSS_MACSEC_THRESHOLD_EVENT_ENABLE_BIT)) {
        *ev_mask |= VTSS_MACSEC_SEQ_THRESHOLD_EVENT;
    }

    if (value & (1 << VTSS_MACSEC_ROLLOVER_EVENT_ENABLE_BIT)) {
        *ev_mask |= VTSS_MACSEC_SEQ_ROLLOVER_EVENT;
    }

    VTSS_I("value:0x%X, ev_mask:0x%X", value, *ev_mask);
    return VTSS_RC_OK;
}

// Function for getting polling events
// In/out - Same as vtss_macsec_event_poll (except inst), See vtss_macsec_api.h
static vtss_rc vtss_macsec_event_poll_priv(vtss_state_t              *vtss_state,
                                           const vtss_port_no_t      port_no,
                                           vtss_macsec_event_t       *ev_mask)
{

    u32 value;
    CSR_RD(port_no, VTSS_MACSEC_EGR_CRYPTO_CTRL_STATUS_REGS_INTR_CTRL_STATUS, &value);
    *ev_mask = 0;

    if (value & (1 << VTSS_MACSEC_THRESHOLD_EVENT_STATUS_BIT)) {
        VTSS_I("value:0x%X, ev_mask:0x%X", value, *ev_mask);
        *ev_mask |= VTSS_MACSEC_SEQ_THRESHOLD_EVENT;
    }

    if (value & (1 << VTSS_MACSEC_ROLLOVER_EVENT_STATUS_BIT)) {
        *ev_mask |= VTSS_MACSEC_SEQ_ROLLOVER_EVENT;
        VTSS_I("value:0x%X, ev_mask:0x%X", value, *ev_mask);
    }

    VTSS_I("value:0x%X, ev_mask:0x%X", value, *ev_mask);
    return VTSS_RC_OK;
}



// Function for configuring capture FIFO
// In/out - Same as vtss_macsec_frame_capture_set (except inst), See vtss_macsec_api.h

// Procedure for capturing frames.
// 1)   Decide the SIDE and MAX_PKT_SIZE and program in CAPT_DEBUG_CTRL.
// 2)   Enable the SA on which you want to capture the first packet. For enabling capture on any SA, program CAPT_DEBUG_TRIGGER_SA1/SA2 = 0xFFFFFFFF. Or if you want to enable capture on SA index [0], then program CAPT_DEBUG_TRIGGER_SA1 = 0x1
// 3)   Enable the capture by programming CAPT_DEBUG_TRIGGER.ENABLE = 1.
// 4)   Send frames.
// 5)   Keep polling CAPT_DEBUG_STATUS to see if any frames have been captured (PKT_COUNT, FULL, WR_PTR).
// 6)   If PKT_COUNT > 0, then frames have been captured, read CAPT_DEBUG_TRIGGER_SA1/SA2 to confirm if the packet for that SA has been captured. Bits will fall back to 0b automatically when a packet was indeed captured, for the SA for which packet was enabled.
// 7)   Stop the capture by programming CAPT_DEBUG_TRIGGER.ENABLE = 0. This is to enable SW to access the fifo.
// 8)   Read CAPT_DEBUG_DATA (0 to 127) to read the packet from capture fifo.

static vtss_rc vtss_macsec_frame_capture_set_priv(vtss_state_t                        *vtss_state,
                                                  const vtss_port_no_t                port_no,
                                                  const vtss_macsec_frame_capture_t   capture)
{
    // We disables all triggers
    CSR_WR(port_no, VTSS_MACSEC_EGR_CAPT_DEBUG_REGS_CAPT_DEBUG_TRIGGER_SA1, 0x0);
    CSR_WR(port_no, VTSS_MACSEC_EGR_CAPT_DEBUG_REGS_CAPT_DEBUG_TRIGGER_SA2, 0x0);
    CSR_WR(port_no, VTSS_MACSEC_EGR_CAPT_DEBUG_REGS_CAPT_DEBUG_TRIGGER, 0x0);

    if (capture != VTSS_MACSEC_FRAME_CAPTURE_DISABLE) {
        VTSS_I("Capture %s, Max packet size = %u Bytes",
               capture == VTSS_MACSEC_FRAME_CAPTURE_INGRESS ? "ingress" : "egress", VTSS_MACSEC_FRAME_CAPTURE_SIZE_MAX);

        // Select ingress/egress side + Max pkt size
        CSR_WR(port_no, VTSS_MACSEC_EGR_CAPT_DEBUG_REGS_CAPT_DEBUG_CTRL,
               (capture == VTSS_MACSEC_FRAME_CAPTURE_INGRESS ? VTSS_F_MACSEC_EGR_CAPT_DEBUG_REGS_CAPT_DEBUG_CTRL_SIDE : 0) |
               VTSS_F_MACSEC_EGR_CAPT_DEBUG_REGS_CAPT_DEBUG_CTRL_MAX_PKT_SIZE(VTSS_MACSEC_FRAME_CAPTURE_SIZE_MAX));


        VTSS_I("Enabling capturing of all SA indexes, %s capturing", capture == VTSS_MACSEC_FRAME_CAPTURE_DISABLE ? "Disabling" : "Enabling");


        // We enables all triggers
        CSR_WR(port_no, VTSS_MACSEC_EGR_CAPT_DEBUG_REGS_CAPT_DEBUG_TRIGGER_SA1, 0xFFFFFFFF);
        CSR_WR(port_no, VTSS_MACSEC_EGR_CAPT_DEBUG_REGS_CAPT_DEBUG_TRIGGER_SA2, 0xFFFFFFFF);
        CSR_WR(port_no, VTSS_MACSEC_EGR_CAPT_DEBUG_REGS_CAPT_DEBUG_TRIGGER, VTSS_F_MACSEC_EGR_CAPT_DEBUG_REGS_CAPT_DEBUG_TRIGGER_ENABLE);
    }
    return VTSS_RC_OK;
}

// Function for getting one frame from the capture FIFO
// In/out - Same as vtss_macsec_frame_get (except inst), See vtss_macsec_api.h
static vtss_rc vtss_macsec_frame_get_priv(vtss_state_t                  *vtss_state,
                                          const vtss_port_no_t          port_no,
                                          const u32                     buf_length,
                                          u32                           *const frm_length,
                                          u8                            *const frame)
{
    const u8 header_size = 2; // Header size (in 32bits)
    u8 i;
    u32 value = 1;
    u8 wr_ptr;
    BOOL truncated;
    u32 adm_hdr0;
    u32 adm_hdr1;
    u8  frm_data_i;
    u8 frm_cnt; // Number of frame in the capture buffer
    *frm_length = 0;
    // Get the amount of data in the FIFO
    CSR_RD(port_no, VTSS_MACSEC_EGR_CAPT_DEBUG_REGS_CAPT_DEBUG_STATUS, &value);

    wr_ptr = VTSS_X_MACSEC_EGR_CAPT_DEBUG_REGS_CAPT_DEBUG_STATUS_WR_PTR(value);
    frm_cnt = VTSS_X_MACSEC_EGR_CAPT_DEBUG_REGS_CAPT_DEBUG_STATUS_PKT_COUNT(value);

    if (frm_cnt == 0) { // No frames captured
        return VTSS_RC_OK;
    }

    if (buf_length < VTSS_MACSEC_FRAME_CAPTURE_SIZE_MAX) {
        VTSS_E("Buffer must be greater than %u bytes", VTSS_MACSEC_FRAME_CAPTURE_SIZE_MAX);
        return VTSS_RC_ERROR;
    }

    // Disable the triggers in order to be able to read the FIFO.
    CSR_WR(port_no, VTSS_MACSEC_EGR_CAPT_DEBUG_REGS_CAPT_DEBUG_TRIGGER, 0);
    CSR_WR(port_no, VTSS_MACSEC_EGR_CAPT_DEBUG_REGS_CAPT_DEBUG_TRIGGER_SA2, 0);
    CSR_WR(port_no, VTSS_MACSEC_EGR_CAPT_DEBUG_REGS_CAPT_DEBUG_TRIGGER_SA1, 0);



    // The first 64bits of the frame captured is an administration header with consist of two 32bits words, namde ADM_HDR0 and ADM_HDR1
    // The Administration header contains the packet size which indicates where the next packet would start.
    // The format of Administration header is
    // ADM_HDR0 = {22bits reserved, 1-bit truncated, 9-bit pkt_size}
    // Truncated (1-bit) -> Indicates the packet is truncated and only a part of the packet is captured.
    // The captured packet could be truncated because the packet could be bigger than the "MAX_PKT_SIZE" programmed by software to capture.
    // Pkt_size (9-bits) -> Indicates the size of the captured packet in bytes. Add 8 to the pkt_size to reach to the next packet.
    // ADM_HDR1 = 32-bit security fail debug code.
    CSR_RD(port_no, VTSS_MACSEC_EGR_CAPT_DEBUG_REGS_CAPT_DEBUG_DATA(0), &adm_hdr0);
    CSR_RD(port_no, VTSS_MACSEC_EGR_CAPT_DEBUG_REGS_CAPT_DEBUG_DATA(1), &adm_hdr1);

    truncated = (adm_hdr0 & 0x200) ? TRUE : FALSE;      // Determine if frame is truncated
    *frm_length = (adm_hdr0 & 0x1FF); //9 bits packet size.

    VTSS_I("Frame size:%u, truncated:%u, adm_hdr1:0x%X, adm_hdr0:0x%X, wr_ptr:%u, value:0x%X", *frm_length, truncated, adm_hdr1, adm_hdr0, wr_ptr, value);

    // Make sure that frame length is not greater than the amount of data in fifo, which should never happen (wr_ptr uses step of 8 bytes)
    if ((*frm_length + header_size) >= wr_ptr * 8)  {
        VTSS_E("Frame length (%u bytes) is supposed to be less than the amount of data in the fifo (%u bytes)",
               *frm_length, wr_ptr * 8);

        *frm_length = wr_ptr;
        return VTSS_RC_ERROR;
    }

    // Get the frame data from the FIFO
    frm_data_i = 0;
    for (i = header_size; i < *frm_length / 4 + header_size; i++) {
        CSR_RD(port_no, VTSS_MACSEC_EGR_CAPT_DEBUG_REGS_CAPT_DEBUG_DATA(i), &value);
        frame[frm_data_i++] = value & 0x00000FF;
        frame[frm_data_i++] = (value & 0x0000FF00) >> 8;
        frame[frm_data_i++] = (value & 0x00FF0000) >> 16;
        frame[frm_data_i++] = (value & 0xFF000000) >> 24;
        VTSS_I("Getting FIFO data:0x%X, i:%d", value, i);
    }

    // Re-enable capture in order to capture next frame.
    CSR_WR(port_no, VTSS_MACSEC_EGR_CAPT_DEBUG_REGS_CAPT_DEBUG_TRIGGER, VTSS_F_MACSEC_EGR_CAPT_DEBUG_REGS_CAPT_DEBUG_TRIGGER_ENABLE);
    CSR_WR(port_no, VTSS_MACSEC_EGR_CAPT_DEBUG_REGS_CAPT_DEBUG_TRIGGER_SA2, 0xFFFFFFFF);
    CSR_WR(port_no, VTSS_MACSEC_EGR_CAPT_DEBUG_REGS_CAPT_DEBUG_TRIGGER_SA1, 0xFFFFFFFF);

    // We use error state to signal that frame is not valid
    if (truncated) {
        VTSS_E("Frame is Truncated");
        return VTSS_RC_ERROR;
    }

    return VTSS_RC_OK;
}

static vtss_rc vtss_macsec_controlled_counters_get_priv(vtss_state_t                      *vtss_state,
                                                        const vtss_macsec_port_t          port,
                                                        vtss_macsec_secy_port_counters_t  *const counters,
                                                        u32                               secy_id)
{
    u16 an;
    u32 sc;
    macsec_secy_in_use_iter_t in_use_inter;
    macsec_secy_in_use_inter_init(&in_use_inter);
    vtss_macsec_secy_counters_t  secy_counters;
    vtss_macsec_internal_secy_t  *secy = &vtss_state->macsec_conf[port.port_no].secy[secy_id];
    vtss_macsec_rx_sa_counters_t rx_sa_counters;
    vtss_macsec_tx_sa_counters_t tx_sa_counters;
    vtss_macsec_rx_sc_counters_t rx_sc_cnt;
    u64                          in_pkts_not_valid = 0;
    u64                          in_pkts_not_using_sa = 0;

    memset(counters, 0, sizeof(vtss_macsec_secy_port_counters_t));
    memset(&rx_sc_cnt, 0, sizeof(vtss_macsec_rx_sc_counters_t));
    memset(&secy_counters, 0, sizeof(vtss_macsec_secy_counters_t));

    // Get Secy counters
    u64 in_pkts_bad_tag  = 0;
    u64 in_pkts_no_sci   = 0;
    u64 in_pkts_no_tag   = 0;
    u64 in_pkts_overrun  = 0;
    u64 out_pkts_too_long = 0;
    //Commented out due to Bugzilla#11986   u64 out_pkts_untagged = 0;
    u64 if_out_octets    = 0;
    u8  octets_add       = 0;
    vtss_state->macsec_conf[port.port_no].secy[secy_id].controlled_cnt.if_out_octets = 0;

    while (macsec_secy_in_use_inter_getnext(vtss_state, port.port_no, &in_use_inter)) {
        // Call the read of hardware counters in order to update the software counters
        VTSS_RC(vtss_macsec_secy_counters_get_priv(vtss_state, port.port_no, &secy_counters, in_use_inter.secy_id));

        in_pkts_bad_tag   += secy_counters.in_pkts_bad_tag;
        in_pkts_no_sci    += secy_counters.in_pkts_no_sci;
        in_pkts_no_tag    += secy_counters.in_pkts_no_tag;
        in_pkts_overrun   += secy_counters.in_pkts_overrun;
        out_pkts_too_long += secy_counters.out_pkts_too_long;
        //Commented out due to Bugzilla#11986   out_pkts_untagged += secy_counters.out_pkts_untagged;
        if_out_octets     += secy_counters.out_octets_protected + secy_counters.out_octets_encrypted;
    }


    vtss_state->macsec_conf[port.port_no].secy[secy_id].controlled_cnt.if_in_pkts  = 0;
    vtss_state->macsec_conf[port.port_no].secy[secy_id].controlled_cnt.if_out_pkts = 0;

    VTSS_N("if_in_octets:%" PRIu64 "", counters->if_in_octets);
    if (secy != NULL && secy->in_use) {
        VTSS_N("secy_id:%u, in_use:%u", secy_id, secy->in_use);
        for (sc = 0; sc < VTSS_MACSEC_MAX_SC_RX; sc++) {
            if (secy->rx_sc[sc] == NULL || !secy->rx_sc[sc]->in_use) {
                continue;
            }
            VTSS_D("rx_sc[sc] i_use");

            for (an = 0; an < VTSS_MACSEC_SA_PER_SC_MAX; an++) {
                if (secy->rx_sc[sc]->sa[an] == NULL || !secy->rx_sc[sc]->sa[an]->in_use) {
                    continue;
                }
                VTSS_I("sc:%u an:%u, in_use:%u", sc, an, secy->rx_sc[sc]->sa[an]->in_use);
                VTSS_RC(vtss_macsec_rx_sa_counters_get_priv(vtss_state, port.port_no, &secy->rx_sc[sc]->sci, an, &rx_sa_counters, secy_id));
                vtss_state->macsec_conf[port.port_no].secy[secy_id].controlled_cnt.if_in_pkts += rx_sa_counters.in_pkts_ok + rx_sa_counters.in_pkts_invalid + rx_sa_counters.in_pkts_not_valid + rx_sa_counters.in_pkts_not_using_sa + rx_sa_counters.in_pkts_unused_sa + rx_sa_counters.in_pkts_unchecked + rx_sa_counters.in_pkts_delayed + rx_sa_counters.in_pkts_late;

                VTSS_RC(vtss_macsec_tx_sa_counters_get_priv(vtss_state, port.port_no, an, &tx_sa_counters, secy_id));

                vtss_state->macsec_conf[port.port_no].secy[secy_id].controlled_cnt.if_out_pkts += tx_sa_counters.out_pkts_encrypted + tx_sa_counters.out_pkts_protected;
                //Commented out due to Bugzilla#11986 vtss_state->macsec_conf[port.port_no].secy[secy_id].controlled_cnt.if_out_pkts += out_pkts_untagged;

                VTSS_D("if_out_pkts:%" PRIu64 "", vtss_state->macsec_conf[port.port_no].secy[secy_id].controlled_cnt.if_out_pkts);
            }
        }


        // From Sailesh Rupani: Controlled.if_in_octets is the sum of SA.InOctetsDecrypted/InOctetsValidated + 12/16/20/x (12 - standard mode, 16 or 20 - vlan tag bypass, x - MPLS header bypass mode)
        // From Sailesh Rupani: Controlled.if_out_octets = sum of SA.OutOctetsEncrypted/OutOctetsProtected + 12/16/20/x (12 - standard mode, 16 or 20 - vlan tag bypass, x - MPLS header bypass mode) for that SecY.
        octets_add = 12; // This 12 bytes are the DMAC and SMAC

        if (vtss_state->macsec_conf[port.port_no].glb.bypass_mode.mode == VTSS_MACSEC_BYPASS_TAG) {
            if (secy->tag_bypass == VTSS_MACSEC_BYPASS_TAG_ONE) {
                octets_add += 4; // This is the 16 (12+4) in Sailesh's calculation above
            } else if (secy->tag_bypass == VTSS_MACSEC_BYPASS_TAG_TWO) {
                octets_add += 8; // This is the 20 (12+8) in Sailesh's calculation above
            }

        } else if (vtss_state->macsec_conf[port.port_no].glb.bypass_mode.mode == VTSS_MACSEC_BYPASS_HDR) {
            // This is the x (12+8) in Sailesh's calculation above
            octets_add += vtss_state->macsec_conf[port.port_no].glb.bypass_mode.hdr_bypass_len;
        }

        vtss_state->macsec_conf[port.port_no].secy[secy_id].controlled_cnt.if_out_octets += if_out_octets + (vtss_state->macsec_conf[port.port_no].secy[secy_id].controlled_cnt.if_out_pkts * octets_add);

        // Finding SA.InOctetsDecrypted or InOctetsValidated - they are mutual exclusive, and the one not used is set to 0, so it is OK to add them (then we don't need to worry about which of them that are currently in use).
        if (secy->conf.validate_frames == VTSS_MACSEC_VALIDATE_FRAMES_DISABLED) {
            // Work-around for Bugzilla#12752/Errata 012. This shall only be done for revision A, but for now we do it for all revisions.
            vtss_state->macsec_conf[port.port_no].secy[secy_id].controlled_cnt.if_in_octets = 0;
        } else {
            vtss_state->macsec_conf[port.port_no].secy[secy_id].controlled_cnt.if_in_octets = secy_counters.in_octets_validated + secy_counters.in_octets_decrypted + octets_add * vtss_state->macsec_conf[port.port_no].secy[secy_id].controlled_cnt.if_in_pkts;
        }

        VTSS_D("if_in_octets:%" PRIu64 ", octets_add:%d, vtss_state->macsec_conf[port.port_no].secy[secy_id].controlled_cnt.if_in_pkts:%" PRIu64 ", validated:%" PRIu64 ", decrypt:%" PRIu64 "",
               vtss_state->macsec_conf[port.port_no].secy[secy_id].controlled_cnt.if_in_octets,
               octets_add, vtss_state->macsec_conf[port.port_no].secy[secy_id].controlled_cnt.if_in_pkts,
               secy_counters.in_octets_validated, secy_counters.in_octets_decrypted);

        // Get rx_sc counter
        for (sc = 0; sc < VTSS_MACSEC_MAX_SC_RX; sc++) {
            if (secy->rx_sc[sc] == NULL) {
                continue;
            }

            VTSS_RC(vtss_macsec_rx_sc_counters_get_priv(vtss_state,
                                                        port.port_no,
                                                        &secy->rx_sc[sc]->sci,
                                                        &rx_sc_cnt,
                                                        secy_id));

            in_pkts_not_valid    += rx_sc_cnt.in_pkts_not_valid;
            in_pkts_not_using_sa += rx_sc_cnt.in_pkts_not_using_sa;
        }

        // From IEEE 802.1AE-2006, page 57- The ifOutErrors count is equal to the OutPktsTooLong count (Figure 10-4).
        vtss_state->macsec_conf[port.port_no].secy[secy_id].controlled_cnt.if_out_errors = out_pkts_too_long;


        VTSS_D("secy_id:%u, in_use:%u", secy_id, secy->in_use);
        // From IEEE 802.1AE-2006, page 57 - The ifInErrors count is the sum of all the
        // InPktsBadTag,
        // InPktsNoSCI,
        // InPktsNotUsingSA, and
        // InPktsNotValid
        vtss_state->macsec_conf[port.port_no].glb.common_cnt.if_in_errors =
            in_pkts_bad_tag +
            in_pkts_no_sci +
            in_pkts_not_valid +
            in_pkts_not_using_sa;

        // From IEEE 802.1AE-2006, page 57 - The ifInDiscards count is the sum of all the
        // InPktsNoTag,
        // InPktsLate, and
        // InPktsOverrun counts.
        VTSS_D("in_pkts_no_tag:%" PRIu64 ", rx_sc_cnt.in_pkts_late:%" PRIu64 ", in_pkts_overrun:%" PRIu64 "", in_pkts_no_tag, rx_sc_cnt.in_pkts_late, in_pkts_overrun);

        vtss_state->macsec_conf[port.port_no].secy[secy_id].controlled_cnt.if_in_discards =
            in_pkts_no_tag +
            rx_sc_cnt.in_pkts_late +
            in_pkts_overrun;

        memcpy(counters, &vtss_state->macsec_conf[port.port_no].secy[secy_id].controlled_cnt, sizeof(vtss_macsec_secy_port_counters_t));
    }
    return VTSS_RC_OK;
}


static vtss_rc vtss_macsec_common_counters_get_priv(vtss_state_t                      *vtss_state,
                                                    const vtss_port_no_t              port_no,
                                                    vtss_macsec_common_counters_t     *const counters,
                                                    BOOL                              clear)
{

    //
    // First update counters
    //
    u32 rx_uc_cnt;
    u32 rx_bc_cnt;
    u32 rx_mc_cnt;
    u64 rx_ok_cnt;
    u64 tx_ok_cnt;
    u32 tx_uc_cnt;
    u32 tx_bc_cnt;
    u32 tx_mc_cnt;
    u32 msb;
    u32 lsb;

    // Secy counters
    u64 in_pkts_bad_tag      = 0;
    u64 in_pkts_no_sci       = 0;
    u64 in_pkts_no_tag       = 0;
    u64 in_pkts_overrun      = 0;
    u64 out_pkts_too_long    = 0;
    u64 in_pkts_not_valid    = 0;
    u64 in_pkts_not_using_sa = 0;
    u64 in_pkts_late         = 0;

    CSR_RD(port_no, VTSS_LINE_MAC_STATISTICS_40BIT_RX_OK_BYTES_CNT, &lsb);
    CSR_RD(port_no, VTSS_LINE_MAC_STATISTICS_40BIT_RX_OK_BYTES_MSB_CNT, &msb);
    rx_ok_cnt = (((u64)msb << 32) + lsb);

    CSR_RD(port_no, VTSS_LINE_MAC_STATISTICS_32BIT_RX_UC_CNT, &rx_uc_cnt);
    CSR_RD(port_no, VTSS_LINE_MAC_STATISTICS_32BIT_RX_MC_CNT, &rx_mc_cnt);
    CSR_RD(port_no, VTSS_LINE_MAC_STATISTICS_32BIT_RX_BC_CNT, &rx_bc_cnt);

    CSR_RD(port_no, VTSS_LINE_MAC_STATISTICS_40BIT_TX_OK_BYTES_CNT, &lsb);
    CSR_RD(port_no, VTSS_LINE_MAC_STATISTICS_40BIT_TX_OK_BYTES_MSB_CNT, &msb);
    tx_ok_cnt = (((u64)msb << 32) + lsb);

    CSR_RD(port_no, VTSS_LINE_MAC_STATISTICS_32BIT_TX_UC_CNT, &tx_uc_cnt);
    CSR_RD(port_no, VTSS_LINE_MAC_STATISTICS_32BIT_TX_MC_CNT, &tx_mc_cnt);
    CSR_RD(port_no, VTSS_LINE_MAC_STATISTICS_32BIT_TX_BC_CNT, &tx_bc_cnt);

    CSR_RD(port_no, VTSS_HOST_MAC_STATISTICS_40BIT_TX_OK_BYTES_CNT, &lsb);
    CSR_RD(port_no, VTSS_HOST_MAC_STATISTICS_40BIT_TX_OK_BYTES_MSB_CNT, &msb);

    // Since HW doesn't support clear on read, we need to remember the counter value whenever
    // the user to clear the counters, in order to be able to compensate for the hw counter are not cleared.
    if (clear) {
        vtss_state->macsec_conf[port_no].glb.common_cnt_clr_val.if_in_octets          = rx_ok_cnt;
        vtss_state->macsec_conf[port_no].glb.common_cnt_clr_val.if_in_ucast_pkts      = rx_uc_cnt;
        vtss_state->macsec_conf[port_no].glb.common_cnt_clr_val.if_in_multicast_pkts  = rx_mc_cnt;
        vtss_state->macsec_conf[port_no].glb.common_cnt_clr_val.if_in_broadcast_pkts  = rx_bc_cnt;
        vtss_state->macsec_conf[port_no].glb.common_cnt_clr_val.if_out_octets         = tx_ok_cnt;
        vtss_state->macsec_conf[port_no].glb.common_cnt_clr_val.if_out_ucast_pkts     = tx_uc_cnt;
        vtss_state->macsec_conf[port_no].glb.common_cnt_clr_val.if_out_multicast_pkts = tx_mc_cnt;
        vtss_state->macsec_conf[port_no].glb.common_cnt_clr_val.if_out_broadcast_pkts = tx_bc_cnt;
    }

    // Compensate for hw counters can't be cleared on read.
    vtss_state->macsec_conf[port_no].glb.common_cnt.if_in_octets          = rx_ok_cnt - vtss_state->macsec_conf[port_no].glb.common_cnt_clr_val.if_in_octets;
    vtss_state->macsec_conf[port_no].glb.common_cnt.if_in_ucast_pkts      = rx_uc_cnt - vtss_state->macsec_conf[port_no].glb.common_cnt_clr_val.if_in_ucast_pkts;
    vtss_state->macsec_conf[port_no].glb.common_cnt.if_in_multicast_pkts  = rx_mc_cnt - vtss_state->macsec_conf[port_no].glb.common_cnt_clr_val.if_in_multicast_pkts;
    vtss_state->macsec_conf[port_no].glb.common_cnt.if_in_broadcast_pkts  = rx_bc_cnt - vtss_state->macsec_conf[port_no].glb.common_cnt_clr_val.if_in_broadcast_pkts;
    vtss_state->macsec_conf[port_no].glb.common_cnt.if_out_octets         = tx_ok_cnt - vtss_state->macsec_conf[port_no].glb.common_cnt_clr_val.if_out_octets;
    vtss_state->macsec_conf[port_no].glb.common_cnt.if_out_ucast_pkts     = tx_uc_cnt - vtss_state->macsec_conf[port_no].glb.common_cnt_clr_val.if_out_ucast_pkts;
    vtss_state->macsec_conf[port_no].glb.common_cnt.if_out_multicast_pkts = tx_mc_cnt - vtss_state->macsec_conf[port_no].glb.common_cnt_clr_val.if_out_multicast_pkts;
    vtss_state->macsec_conf[port_no].glb.common_cnt.if_out_broadcast_pkts = tx_bc_cnt - vtss_state->macsec_conf[port_no].glb.common_cnt_clr_val.if_out_broadcast_pkts;


    // From IEEE 802.1AE-2006, page 57- The ifOutErrors count is equal to the OutPktsTooLong count (Figure 10-4).
    vtss_state->macsec_conf[port_no].glb.common_cnt.if_out_errors = out_pkts_too_long;


    VTSS_N("Ok bytes:%" PRIu64 ", clr:%" PRIu64 ", port:%u, lsb:%u, msb:%u",
           vtss_state->macsec_conf[port_no].glb.common_cnt.if_in_octets,
           vtss_state->macsec_conf[port_no].glb.common_cnt_clr_val.if_in_octets,
           port_no, lsb, msb);


    macsec_secy_in_use_iter_t in_use_inter;

    macsec_secy_in_use_inter_init(&in_use_inter);
    vtss_macsec_secy_counters_t   secy_counters;

    while (macsec_secy_in_use_inter_getnext(vtss_state, port_no, &in_use_inter)) {
        // Call the read of hardware counters in order to update the software counters
        VTSS_RC(vtss_macsec_secy_counters_get_priv(vtss_state, port_no, &secy_counters, in_use_inter.secy_id));
        in_pkts_bad_tag += secy_counters.in_pkts_bad_tag;
        in_pkts_no_sci  += secy_counters.in_pkts_no_sci;
        in_pkts_no_tag  += secy_counters.in_pkts_no_tag;
        in_pkts_overrun += secy_counters.in_pkts_overrun;
        out_pkts_too_long += secy_counters.out_pkts_too_long;
    }

    // From IEEE 802.1AE-2006, page 57 - The ifInErrors count is the sum of all the
    // InPktsBadTag,
    // InPktsNoSCI,
    // InPktsNotUsingSA, and
    // InPktsNotValid
    vtss_state->macsec_conf[port_no].glb.common_cnt.if_in_errors =
        in_pkts_bad_tag +
        in_pkts_no_sci +
        in_pkts_not_valid +
        in_pkts_not_using_sa;

    // From IEEE 802.1AE-2006, page 57 - The ifInDiscards count is the sum of all the
    // InPktsNoTag,
    // InPktsLate, and
    // InPktsOverrun counts.
    vtss_state->macsec_conf[port_no].glb.common_cnt.if_in_discards =
        in_pkts_no_tag +
        in_pkts_late +
        in_pkts_overrun;


    //
    // 2nd, pass the counters back
    //
    memcpy(counters, &vtss_state->macsec_conf[port_no].glb.common_cnt, sizeof(*counters));
    return VTSS_RC_OK;
}

static vtss_rc vtss_macsec_uncontrolled_counters_get_priv(vtss_state_t                        *vtss_state,
                                                          const vtss_port_no_t                port_no,
                                                          vtss_macsec_uncontrolled_counters_t *const counters)
{
    vtss_macsec_common_counters_t    common_counters;
    vtss_macsec_secy_port_counters_t controlled_counters;
    u32                              secy_id;
    vtss_macsec_internal_secy_t      *secy;
    vtss_macsec_port_t                macsec_port;
    u64 ctrl_if_out_octets = 0;
#ifdef BYPASS_IEEE_SECTION_10_7_3
    u64 ctrl_if_in_octets  = 0;
#endif


    // From IEEE 802.1AE-2006 - Section 10.7.3 : The ifInDiscards and ifInErrors counts are zero, as the operation of the Uncontrolled Port provides no error checking or occasion to discard packets, beyond that
    //                                           provided by its users or by the entity supporting the Common Port.
    counters->if_in_discards = 0;
    counters->if_in_errors   = 0;

    // From IEEE 802.1AE-2006 - Section 10.7.3 : The ifOutErrorscount is zero, as no checking is applied to frames transmitted by the Uncontrolled Port.
    counters->if_out_errors  = 0;

    // Uncontrolled out counters are common counter minus the controlled counters.

    // We start be finding controlled counters.
    macsec_port.service_id = 0;
    macsec_port.port_id = 0;
    macsec_port.port_no = port_no;

    for (secy_id = 0; secy_id < VTSS_MACSEC_MAX_SECY; secy_id++) {
        secy = &vtss_state->macsec_conf[port_no].secy[secy_id];
        if (secy != NULL && secy->in_use) {
            VTSS_RC(vtss_macsec_controlled_counters_get_priv(vtss_state, macsec_port, &controlled_counters, secy_id));

            if (secy->conf.always_include_sci) {
                // Note : Sectag = 16, ICV = 16  ICV, 4 bytes CRC - These are not counter by the controlled counter, but must be subtracted when we later calculates the uncontrolled counters
                ctrl_if_out_octets += controlled_counters.if_out_octets + (16 + 16 + 4) * controlled_counters.if_out_pkts ;
            } else {
                // Note : Sectag = 8, ICV = 16  ICV, 4 bytes CRC - These are not counter by the controlled counter, but must be subtracted when we later calculates the uncontrolled counters
                ctrl_if_out_octets += controlled_counters.if_out_octets + (16 + 8 + 4) * controlled_counters.if_out_pkts ;
            }

#ifdef BYPASS_IEEE_SECTION_10_7_3
            ctrl_if_in_octets += controlled_counters.if_in_octets + 36 * ctrl_if_in_pkts;
#endif
        }
    }

    VTSS_RC(vtss_macsec_common_counters_get_priv(vtss_state, port_no, &common_counters, FALSE));

    // From IEEE 802.1AE-2006 - Section 10.7.3 : The ifInOctets, ifInUcastPkts, ifInMulticastPkts, and ifInBroadcastPkts counts are identical to those of Common Port and are not separately recorded,
    counters->if_in_octets         = common_counters.if_in_octets;
    counters->if_in_ucast_pkts     = common_counters.if_in_ucast_pkts;
    counters->if_in_multicast_pkts = common_counters.if_in_multicast_pkts;
    counters->if_in_broadcast_pkts = common_counters.if_in_broadcast_pkts;



    // From IEEE 802.1AE-2006 - Section 10.7.3 :  The ifOutOctets, ifOutUcastPkts, ifOutMulticastPkts, and ifOutBroadcastPkts counts are the same as those for the user of the Uncontrolled Port.
    // This is common_counters minus controlled_counters. Note : ifOutUcastPkts, ifOutMulticastPkts and ifOutBroadcastPkts are not supported
    counters->if_out_octets         = common_counters.if_out_octets -= ctrl_if_out_octets;

    // The above about uncontrolled rx counters doesn't make sense to us, so if we want use the same calculation as for the out counters, BYPASS_IEEE_SECTION_10_7_3 should be defined
#ifdef BYPASS_IEEE_SECTION_10_7_3
    counters->if_in_octets         -= ctrl_if_in_octets;
#endif

    return VTSS_RC_OK;
}

static vtss_rc vtss_macsec_counters_clear_priv(vtss_state_t                  *vtss_state,
                                               const vtss_port_no_t          port_no)
{
    u32 an, sc, secy_id;
    vtss_macsec_internal_secy_t *secy;

    // Clear global counters
    vtss_macsec_common_counters_t common_counters;
    vtss_macsec_tx_sa_counters_t  dummy_tx_sa_counters;

    VTSS_RC(vtss_macsec_common_counters_get_priv(vtss_state, port_no, &common_counters, TRUE));


    for (secy_id = 0; secy_id < VTSS_MACSEC_MAX_SC_RX; secy_id++) {
        secy = &vtss_state->macsec_conf[port_no].secy[secy_id];
        if (secy  == NULL) {
            continue;
        }
        memset(&vtss_state->macsec_conf[port_no].glb.uncontrolled_cnt, 0, sizeof(vtss_macsec_secy_port_counters_t));

        memset(&vtss_state->macsec_conf[port_no].secy[secy_id].controlled_cnt, 0, sizeof(vtss_macsec_secy_port_counters_t));
        // Clear SecY counters
        memset(&secy->secy_cnt, 0, sizeof(vtss_macsec_secy_counters_t));
        // Clear Egress counters
        memset(&secy->tx_sc.cnt, 0, sizeof(vtss_macsec_tx_sc_counters_t));
        for (an = 0; an < VTSS_MACSEC_SA_PER_SC_MAX; an++) {
            if (secy->tx_sc.sa[an] != NULL) {
                memset(&secy->tx_sc.sa[an]->cnt, 0, sizeof(vtss_macsec_tx_sa_counters_t));
                // Clearing tx_sa counters
                // Call update of counters in order to clear the hw counters.
                VTSS_RC(vtss_macsec_tx_sa_counters_get_priv(vtss_state, port_no, an, &dummy_tx_sa_counters, secy_id));
                memset(&vtss_state->macsec_conf[port_no].tx_sa[an].cnt, 0, sizeof(vtss_state->macsec_conf[port_no].tx_sa[an].cnt));
            }
        }

        // Clear Ingress counters
        for (sc = 0; sc < VTSS_MACSEC_MAX_SC_RX; sc++) {
            if (secy->rx_sc[sc] != NULL) {
                memset(&secy->rx_sc[sc]->cnt, 0, sizeof(vtss_macsec_rx_sc_counters_t));
                for (an = 0; an < VTSS_MACSEC_SA_PER_SC_MAX; an++) {
                    if (secy->rx_sc[sc]->sa[an] != NULL) {
                        memset(&secy->rx_sc[sc]->sa[an]->cnt, 0, sizeof(vtss_macsec_rx_sa_counters_t));
                    }
                }
            }
        }
    }
    return VTSS_RC_OK;
}

// Function for updating all MACSEC counters in vtss_state by calling the counter read functions.
static vtss_rc vtss_macsec_counters_update_priv(vtss_state_t                  *vtss_state,
                                                const vtss_port_no_t          port_no)
{
    macsec_secy_in_use_iter_t in_use_inter;
    macsec_secy_in_use_inter_init(&in_use_inter);

    // Dummy, because are only reading the counters in order to update software counters (read hw
    // counters in order to make sure they don't wraps around)
    vtss_macsec_secy_counters_t   dummy_counters;
    vtss_macsec_common_counters_t dummy_common_counters;
    while (macsec_secy_in_use_inter_getnext(vtss_state, port_no, &in_use_inter)) {
        // Call the read of hardware counters in order to update the software counters
        VTSS_RC(vtss_macsec_secy_counters_get_priv(vtss_state, port_no, &dummy_counters, in_use_inter.secy_id));
    }

    VTSS_RC(vtss_macsec_common_counters_get_priv(vtss_state, port_no, &dummy_common_counters, FALSE));

    return VTSS_RC_OK;
}

static vtss_rc vtss_macsec_hmac_counters_get_priv(vtss_state_t                      *vtss_state,
                                                  const vtss_port_no_t              port_no,
                                                  vtss_macsec_mac_counters_t        *const counters,
                                                  const BOOL                        clear)

{
    u32 cnt;
    CSR_RD(port_no, VTSS_HOST_MAC_STATISTICS_40BIT_RX_OK_BYTES_CNT, &cnt);
    counters->if_rx_octets = cnt;
    CSR_RD(port_no, VTSS_HOST_MAC_STATISTICS_32BIT_RX_UC_CNT, &cnt);
    counters->if_rx_ucast_pkts = cnt;
    CSR_RD(port_no, VTSS_HOST_MAC_STATISTICS_32BIT_RX_MC_CNT, &cnt);
    counters->if_rx_multicast_pkts = cnt;
    CSR_RD(port_no, VTSS_HOST_MAC_STATISTICS_32BIT_RX_BC_CNT, &cnt);
    counters->if_rx_broadcast_pkts = cnt;
    CSR_RD(port_no, VTSS_HOST_MAC_STATISTICS_32BIT_RX_CRC_ERR_CNT, &cnt);
    counters->if_rx_errors = cnt;
    CSR_RD(port_no, VTSS_HOST_MAC_STATISTICS_32BIT_RX_UNDERSIZE_CNT, &cnt);
    counters->if_rx_errors += cnt;
    CSR_RD(port_no, VTSS_HOST_MAC_STATISTICS_32BIT_RX_FRAGMENTS_CNT, &cnt);
    counters->if_rx_errors += cnt;
    CSR_RD(port_no, VTSS_HOST_MAC_STATISTICS_32BIT_RX_JABBERS_CNT, &cnt);
    counters->if_rx_errors += cnt;


    CSR_RD(port_no, VTSS_HOST_MAC_STATISTICS_40BIT_TX_OK_BYTES_CNT, &cnt);
    counters->if_tx_octets = cnt;
    CSR_RD(port_no, VTSS_HOST_MAC_STATISTICS_32BIT_TX_UC_CNT, &cnt);
    counters->if_tx_ucast_pkts = cnt;
    CSR_RD(port_no, VTSS_HOST_MAC_STATISTICS_32BIT_TX_MC_CNT, &cnt);
    counters->if_tx_multicast_pkts = cnt;
    CSR_RD(port_no, VTSS_HOST_MAC_STATISTICS_32BIT_TX_BC_CNT, &cnt);
    counters->if_tx_broadcast_pkts = cnt;

    if (clear) {
        CSR_WR(port_no, VTSS_HOST_MAC_STATISTICS_40BIT_RX_OK_BYTES_CNT, 0);
        CSR_WR(port_no, VTSS_HOST_MAC_STATISTICS_32BIT_RX_UC_CNT, 0);
        CSR_WR(port_no, VTSS_HOST_MAC_STATISTICS_32BIT_RX_MC_CNT, 0);
        CSR_WR(port_no, VTSS_HOST_MAC_STATISTICS_32BIT_RX_BC_CNT, 0);
        CSR_WR(port_no, VTSS_HOST_MAC_STATISTICS_32BIT_RX_CRC_ERR_CNT, 0);
        CSR_WR(port_no, VTSS_HOST_MAC_STATISTICS_32BIT_RX_UNDERSIZE_CNT, 0);
        CSR_WR(port_no, VTSS_HOST_MAC_STATISTICS_32BIT_RX_FRAGMENTS_CNT, 0);
        CSR_WR(port_no, VTSS_HOST_MAC_STATISTICS_32BIT_RX_JABBERS_CNT, 0);
        CSR_WR(port_no, VTSS_HOST_MAC_STATISTICS_40BIT_TX_OK_BYTES_CNT, 0);
        CSR_WR(port_no, VTSS_HOST_MAC_STATISTICS_32BIT_TX_UC_CNT, 0);
        CSR_WR(port_no, VTSS_HOST_MAC_STATISTICS_32BIT_TX_MC_CNT, 0);
        CSR_WR(port_no, VTSS_HOST_MAC_STATISTICS_32BIT_TX_BC_CNT, 0);
    }
    return VTSS_RC_OK;
}

static vtss_rc vtss_macsec_lmac_counters_get_priv(vtss_state_t                      *vtss_state,
                                                  const vtss_port_no_t              port_no,
                                                  vtss_macsec_mac_counters_t        *const counters,
                                                  const BOOL                        clear)

{
    u32 cnt;
    CSR_RD(port_no, VTSS_LINE_MAC_STATISTICS_40BIT_RX_OK_BYTES_CNT, &cnt);
    counters->if_rx_octets = cnt;
    CSR_RD(port_no, VTSS_LINE_MAC_STATISTICS_32BIT_RX_UC_CNT, &cnt);
    counters->if_rx_ucast_pkts = cnt;
    CSR_RD(port_no, VTSS_LINE_MAC_STATISTICS_32BIT_RX_MC_CNT, &cnt);
    counters->if_rx_multicast_pkts = cnt;
    CSR_RD(port_no, VTSS_LINE_MAC_STATISTICS_32BIT_RX_BC_CNT, &cnt);
    counters->if_rx_broadcast_pkts = cnt;
    CSR_RD(port_no, VTSS_LINE_MAC_STATISTICS_32BIT_RX_CRC_ERR_CNT, &cnt);
    counters->if_rx_errors = cnt;
    CSR_RD(port_no, VTSS_LINE_MAC_STATISTICS_32BIT_RX_UNDERSIZE_CNT, &cnt);
    counters->if_rx_errors += cnt;
    CSR_RD(port_no, VTSS_LINE_MAC_STATISTICS_32BIT_RX_FRAGMENTS_CNT, &cnt);
    counters->if_rx_errors += cnt;
    CSR_RD(port_no, VTSS_LINE_MAC_STATISTICS_32BIT_RX_JABBERS_CNT, &cnt);
    counters->if_rx_errors += cnt;

    CSR_RD(port_no, VTSS_LINE_MAC_STATISTICS_40BIT_TX_OK_BYTES_CNT, &cnt);
    counters->if_tx_octets = cnt;
    CSR_RD(port_no, VTSS_LINE_MAC_STATISTICS_32BIT_TX_UC_CNT, &cnt);
    counters->if_tx_ucast_pkts = cnt;
    CSR_RD(port_no, VTSS_LINE_MAC_STATISTICS_32BIT_TX_MC_CNT, &cnt);
    counters->if_tx_multicast_pkts = cnt;
    CSR_RD(port_no, VTSS_LINE_MAC_STATISTICS_32BIT_TX_BC_CNT, &cnt);
    counters->if_tx_broadcast_pkts = cnt;

    if (clear) {
        CSR_WR(port_no, VTSS_LINE_MAC_STATISTICS_40BIT_RX_OK_BYTES_CNT, 0);
        CSR_WR(port_no, VTSS_LINE_MAC_STATISTICS_32BIT_RX_UC_CNT, 0);
        CSR_WR(port_no, VTSS_LINE_MAC_STATISTICS_32BIT_RX_MC_CNT, 0);
        CSR_WR(port_no, VTSS_LINE_MAC_STATISTICS_32BIT_RX_BC_CNT, 0);
        CSR_WR(port_no, VTSS_LINE_MAC_STATISTICS_32BIT_RX_CRC_ERR_CNT, 0);
        CSR_WR(port_no, VTSS_LINE_MAC_STATISTICS_32BIT_RX_UNDERSIZE_CNT, 0);
        CSR_WR(port_no, VTSS_LINE_MAC_STATISTICS_32BIT_RX_FRAGMENTS_CNT, 0);
        CSR_WR(port_no, VTSS_LINE_MAC_STATISTICS_32BIT_RX_JABBERS_CNT, 0);
        CSR_WR(port_no, VTSS_LINE_MAC_STATISTICS_40BIT_TX_OK_BYTES_CNT, 0);
        CSR_WR(port_no, VTSS_LINE_MAC_STATISTICS_32BIT_TX_UC_CNT, 0);
        CSR_WR(port_no, VTSS_LINE_MAC_STATISTICS_32BIT_TX_MC_CNT, 0);
        CSR_WR(port_no, VTSS_LINE_MAC_STATISTICS_32BIT_TX_BC_CNT, 0);
    }
    return VTSS_RC_OK;
}

/* ================================================================= *
 *  Private functions - End
 * ================================================================= */

/* ================================================================= *
 *  Public functions for MacSec API
 * ================================================================= */
BOOL is_macsec_capable(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
#ifdef VTSS_CHIP_10G_PHY
    if (vtss_state->phy_10g_state[port_no].type != VTSS_PHY_TYPE_10G_NONE) {
        if (vtss_state->phy_10g_state[port_no].type == VTSS_PHY_TYPE_8490 ||
            vtss_state->phy_10g_state[port_no].type == VTSS_PHY_TYPE_8491) {
            return 1;
        }
    }
#endif
#ifdef VTSS_CHIP_CU_PHY
    if (vtss_phy_can(vtss_state, port_no, VTSS_CAP_MACSEC)) {
        return 1;
    }
#endif
    return 0;
}

vtss_rc vtss_macsec_init_set(const vtss_inst_t                inst,
                             const vtss_port_no_t             port_no,
                             const vtss_macsec_init_t         *const init)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_ERROR;
#ifdef VTSS_CHIP_CU_PHY
    vtss_phy_conf_t power;
    vtss_phy_type_t phy_1g_id;
    BOOL is_phy_1g  = (vtss_phy_id_get(inst, port_no, &phy_1g_id) == VTSS_RC_OK)  && (phy_1g_id.part_number != VTSS_PHY_TYPE_NONE);

    if (is_phy_1g) {
        VTSS_RC(vtss_phy_conf_get(inst,  port_no, &power));
        if (power.mode == VTSS_PHY_MODE_POWER_DOWN) {
            VTSS_E("Phy %u is powered down, i.e. the MacSec block is not accessible", port_no);
            return VTSS_RC_ERROR;
        }
    }
#endif /* VTSS_CHIP_CU_PHY */
    VTSS_D("port_no: %u", port_no);

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        if (!is_macsec_capable(vtss_state, port_no)) {
            VTSS_E("Port/Phy :%u is not MacSec capable", port_no);
            rc = VTSS_RC_ERROR;
        } else {
            if ((vtss_state->macsec_conf[port_no].glb.init.enable != init->enable)) {
                rc = vtss_macsec_init_set_priv(vtss_state, port_no, init);
                vtss_state->macsec_conf[port_no].glb.init = *init;
            }
        }
    }
    if ((rc = vtss_macsec_speed_conf_priv(vtss_state, port_no)) != VTSS_RC_OK) {
        VTSS_E("Could not set speed on port:%u", port_no);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_macsec_init_get(const vtss_inst_t                 inst,
                             const vtss_port_no_t              port_no,
                             vtss_macsec_init_t                *const init)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_ERROR;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        if (!is_macsec_capable(vtss_state, port_no)) {
            VTSS_E("Port/Phy :%u is not MacSec capable", port_no);
            rc = VTSS_RC_ERROR;
        } else {
            *init = vtss_state->macsec_conf[port_no].glb.init;
        }
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_macsec_secy_conf_add(const vtss_inst_t                 inst,
                                  const vtss_macsec_port_t          port,
                                  const vtss_macsec_secy_conf_t     *const conf)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_ERROR;
    u32 secy_id, i;
    vtss_macsec_internal_secy_t *secy;

    VTSS_I("Port: %u/%u/%u. TxMAC:"MACADDRESS_FMT, MACSEC_PORT_ARG(&port), MACADDRESS_ARG(conf->mac_addr));
    VTSS_I("SecY prm: RP:%d RW:%d Prt frms:%d incl_sci:%d use_es:%d use_scb:%d cipher:%s confidentiality offset:%d",
           conf->replay_protect, conf->replay_window, conf->protect_frames, conf->always_include_sci, conf->use_es,
           conf->use_scb, conf->current_cipher_suite == VTSS_MACSEC_CIPHER_SUITE_GCM_AES_128 ? "128" : "256", conf->confidentiality_offset);
    VTSS_MACSEC_ASSERT(conf->confidentiality_offset > 64, "Confidentiality offset value not supported");
    VTSS_ENTER();

    if ((rc = vtss_macsec_port_check(inst, &vtss_state, port, 1, &secy_id)) == VTSS_RC_OK) {
        if (!check_resources(vtss_state, port.port_no, 0, 0)) {
            VTSS_E("HW resources exhausted");
            rc = VTSS_RC_ERROR;
        } else {
            secy = &vtss_state->macsec_conf[port.port_no].secy[secy_id];

            secy->conf = *conf;
            secy->sci.port_id = port.port_id;
            secy->service_id = port.service_id;
            for (i = 0; i < 6; ++i) {
                secy->sci.mac_addr.addr[i] = secy->conf.mac_addr.addr[i];
            }

            for (i = 0; i < 3; ++i) {
                secy->pattern_record[i][0] = MACSEC_NOT_IN_USE;
                secy->pattern_record[i][1] = MACSEC_NOT_IN_USE;
                secy->pattern[i][0].match = VTSS_MACSEC_MATCH_DISABLE;
                secy->pattern[i][1].match = VTSS_MACSEC_MATCH_DISABLE;
            }
            secy->pattern[VTSS_MACSEC_MATCH_ACTION_CONTROLLED_PORT][VTSS_MACSEC_DIRECTION_EGRESS].priority = VTSS_MACSEC_MATCH_PRIORITY_LOWEST;
            secy->pattern[VTSS_MACSEC_MATCH_ACTION_CONTROLLED_PORT][VTSS_MACSEC_DIRECTION_INGRESS].priority = VTSS_MACSEC_MATCH_PRIORITY_LOWEST;
            if ((rc = is_sci_valid(&secy->sci)) == VTSS_RC_OK) {  // Update rc to return error code if sci is not valid
                secy->in_use = 1;
            } else {
                secy->in_use = 0;
            }
        }
    }
    VTSS_EXIT();

    return rc;
}

vtss_rc vtss_macsec_secy_conf_update(const vtss_inst_t                 inst,
                                     const vtss_macsec_port_t          port,
                                     const vtss_macsec_secy_conf_t     *const conf)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_ERROR;
    u32 secy_id, i, sc;
    vtss_macsec_internal_secy_t *secy;

    VTSS_I("Port: %u/%u/%u. TxMAC:"MACADDRESS_FMT, MACSEC_PORT_ARG(&port), MACADDRESS_ARG(conf->mac_addr));
    VTSS_I("SecY prm: RP:%d RW:%d Prt frms:%d incl_sci:%d use_es:%d use_scb:%d cipher:%s confidentiality offset:%d",
           conf->replay_protect, conf->replay_window, conf->protect_frames, conf->always_include_sci, conf->use_es,
           conf->use_scb, conf->current_cipher_suite == VTSS_MACSEC_CIPHER_SUITE_GCM_AES_128 ? "128" : "256", conf->confidentiality_offset);
    VTSS_MACSEC_ASSERT(conf->confidentiality_offset > 64, "Confidentiality offset value not supported");
    VTSS_ENTER();

    if ((rc = vtss_macsec_port_check(inst, &vtss_state, port, 0, &secy_id)) == VTSS_RC_OK) {
        secy = &vtss_state->macsec_conf[port.port_no].secy[secy_id];
        secy->conf = *conf;
        for (i = 0; i < 6; ++i) {
            secy->sci.mac_addr.addr[i] = secy->conf.mac_addr.addr[i];
        }

        for (sc = 0; sc < VTSS_MACSEC_MAX_SC_RX; sc++) {
            if (secy->rx_sc[sc] == NULL || !secy->rx_sc[sc]->in_use || (rc = is_sci_valid(&secy->sci)) != VTSS_RC_OK) {
                continue;
            }
            secy->rx_sc[sc]->conf.validate_frames = conf->validate_frames;
            secy->rx_sc[sc]->conf.replay_protect = conf->replay_protect;
            secy->rx_sc[sc]->conf.replay_window = conf->replay_window;
            secy->rx_sc[sc]->conf.confidentiality_offset = conf->confidentiality_offset;
        }
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_macsec_secy_conf_del(const vtss_inst_t                 inst,
                                  const vtss_macsec_port_t          port)

{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_ERROR;
    u32 secy_id;

    VTSS_I("port_no: %u/%u/%u", port.port_no, port.port_id, port.service_id);
    VTSS_ENTER();
    if ((rc = vtss_macsec_port_check(inst, &vtss_state, port, 0, &secy_id)) == VTSS_RC_OK) {
        rc = vtss_macsec_secy_conf_del_priv(vtss_state, port, secy_id);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_macsec_secy_conf_get(const vtss_inst_t         inst,
                                  const vtss_macsec_port_t  port,
                                  vtss_macsec_secy_conf_t   *const conf)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_ERROR;
    u32 secy_id;

    VTSS_D("Port: %u/%u/%u", MACSEC_PORT_ARG(&port));
    VTSS_ENTER();
    if ((rc = vtss_macsec_port_check(inst, &vtss_state, port, 0, &secy_id)) == VTSS_RC_OK) {
        *conf = vtss_state->macsec_conf[port.port_no].secy[secy_id].conf;
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_macsec_secy_controlled_set(const vtss_inst_t         inst,
                                        const vtss_macsec_port_t  port,
                                        const BOOL                enable)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_ERROR;
    u32 secy_id;
    VTSS_I("Port: %u/%u/%u", MACSEC_PORT_ARG(&port));
    VTSS_ENTER();
    if ((rc = vtss_macsec_port_check(inst, &vtss_state, port, 0, &secy_id)) == VTSS_RC_OK) {
        rc = vtss_macsec_secy_controlled_set_priv(vtss_state, port, enable, secy_id);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_macsec_secy_controlled_get(const vtss_inst_t         inst,
                                        const vtss_macsec_port_t  port,
                                        BOOL                      *const enable)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_ERROR;
    u32 secy_id;

    VTSS_D("Port: %u/%u/%u", MACSEC_PORT_ARG(&port));
    VTSS_ENTER();
    if ((rc = vtss_macsec_port_check(inst, &vtss_state, port, 0, &secy_id)) == VTSS_RC_OK) {
        rc = vtss_macsec_secy_controlled_get_priv(vtss_state, port, enable, secy_id);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_macsec_rx_sc_add(const vtss_inst_t           inst,
                              const vtss_macsec_port_t    port,
                              const vtss_macsec_sci_t     *const sci)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_ERROR;
    u32 secy_id;

    VTSS_I(MPORT_SCI_FMT, MPORT_SCI_ARG(port, *sci));
    VTSS_ENTER();
    if ((rc = vtss_macsec_port_check(inst, &vtss_state, port, 0, &secy_id)) == VTSS_RC_OK) {
        rc = vtss_macsec_rx_sc_add_priv(vtss_state, port, sci, secy_id);
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_macsec_rx_sc_update(const vtss_inst_t              inst,
                                 const vtss_macsec_port_t       port,
                                 const vtss_macsec_sci_t        *const sci,
                                 const vtss_macsec_rx_sc_conf_t *const conf)
{
    vtss_state_t *vtss_state;
    vtss_macsec_internal_secy_t *secy;
    vtss_rc rc = VTSS_RC_ERROR;
    u32 secy_id, sc;

    VTSS_I(MPORT_SCI_FMT", conf:"MACSEC_RX_SC_CONF_FMT,
           MPORT_SCI_ARG(port, *sci), MACSEC_RX_SC_CONF_ARG(conf));
    VTSS_MACSEC_ASSERT(conf->confidentiality_offset > 64, "Confidentiality offset value not supported");
    VTSS_ENTER();
    if ((rc = vtss_macsec_port_check(inst, &vtss_state, port, 0, &secy_id)) == VTSS_RC_OK) {
        secy = &vtss_state->macsec_conf[port.port_no].secy[secy_id];
        if ((rc = sc_from_sci_get(secy, sci, &sc)) != VTSS_RC_OK) {
            VTSS_E("Could not find SC (from sci)");
        } else {
            secy->rx_sc[sc]->conf = *conf;
        }
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_macsec_rx_sc_del(const vtss_inst_t           inst,
                              const vtss_macsec_port_t    port,
                              const vtss_macsec_sci_t     *const sci)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_ERROR;
    u32 secy_id;

    VTSS_I(MPORT_SCI_FMT, MPORT_SCI_ARG(port, *sci));
    VTSS_ENTER();
    if ((rc = vtss_macsec_port_check(inst, &vtss_state, port, 0, &secy_id)) == VTSS_RC_OK) {
        rc = vtss_macsec_rx_sc_del_priv(vtss_state, port, sci, secy_id);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_macsec_secy_port_status_get(const vtss_inst_t                 inst,
                                         const vtss_macsec_port_t          port,
                                         vtss_macsec_secy_port_status_t    *const status)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_ERROR;
    u32 secy_id;
    BOOL fdx = 1;
#ifdef VTSS_CHIP_CU_PHY
    vtss_port_status_t phy_status;
    vtss_phy_type_t phy_1g_id;
    BOOL is_phy_1g  = (vtss_phy_id_get(inst, port.port_no, &phy_1g_id) == VTSS_RC_OK)  && (phy_1g_id.part_number != VTSS_PHY_TYPE_NONE);

    VTSS_D(MACSEC_PORT_FMT, MACSEC_PORT_ARG(&port));
    if (is_phy_1g) {
        VTSS_RC(vtss_phy_status_get(inst, port.port_no, &phy_status));
        fdx = phy_status.fdx;
    }
#endif /* VTSS_CHIP_CU_PHY */
    VTSS_ENTER();
    if ((rc = vtss_macsec_port_check(inst, &vtss_state, port, 0, &secy_id)) == VTSS_RC_OK) {
        rc = vtss_macsec_secy_port_status_get_priv(vtss_state, port, status, secy_id, fdx);
    }
    VTSS_D("Common: Ena:%d P2P:%u Oper:%u", status->common.mac_enabled,
           status->common.oper_point_to_point_mac, status->common.mac_operational);
    VTSS_D("Contrl: Ena:%d P2P:%u Oper:%u", status->controlled.mac_enabled,
           status->controlled.oper_point_to_point_mac, status->controlled.mac_operational);
    VTSS_D("Uncontrl: Ena:%d P2P:%u Oper:%u", status->uncontrolled.mac_enabled,
           status->uncontrolled.oper_point_to_point_mac, status->uncontrolled.mac_operational);
    VTSS_EXIT();

    if (rc == VTSS_RC_OK) {
        VTSS_D(" -> "MACSEC_SECY_PORT_STATUS_FMT,
               MACSEC_SECY_PORT_STATUS_ARG(*status));
    }

    return rc;
}

vtss_rc vtss_macsec_rx_sc_get_next(const vtss_inst_t              inst,
                                   const vtss_macsec_port_t       port,
                                   const vtss_macsec_sci_t        *const search_sci,
                                   vtss_macsec_sci_t              *const found_sci)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_ERROR;
    u32 secy_id, i;
    vtss_macsec_internal_secy_t *secy;
    const vtss_macsec_sci_t *sci_p = search_sci;
    BOOL search_cont = 1, search_larger = 1;

    VTSS_I("Port: %u/%u/%u", MACSEC_PORT_ARG(&port));

    VTSS_ENTER();
    if ((rc = vtss_macsec_port_check(inst, &vtss_state, port, 0, &secy_id)) == VTSS_RC_OK) {
        secy = &vtss_state->macsec_conf[port.port_no].secy[secy_id];

        if (sci_p != NULL) {
            VTSS_I("Search SCI:"SCI_FMT, SCI_ARG(*search_sci));
        } else {
            VTSS_I("Search SCI == NULL");
        }

        while (search_cont) {
            search_cont = 0;
            for (i = 0; i < VTSS_MACSEC_MAX_SC_RX; i++) {
                if (secy->rx_sc[i] == NULL) {
                    continue;
                }
                if (secy->rx_sc[i]->in_use) {
                    if (sci_cmp(&secy->rx_sc[i]->sci, search_sci)) {
                        continue;
                    }
                    if (search_larger) {
                        if (sci_larger(&secy->rx_sc[i]->sci, sci_p)) {
                            sci_p = &secy->rx_sc[i]->sci;
                            search_cont = 1;
                            search_larger = 0;
                            break;
                        }
                    } else {
                        if (sci_cmp(&secy->rx_sc[i]->sci, sci_p)) {
                            continue;
                        }
                        if (!sci_larger(&secy->rx_sc[i]->sci, search_sci)) {
                            continue;
                        }
                        if (!sci_larger(&secy->rx_sc[i]->sci, sci_p))  {
                            sci_p = &secy->rx_sc[i]->sci;
                            search_cont = 1;
                            break;
                        }
                    }
                }
            }
        }
        if (sci_p != NULL) {
            *found_sci = *sci_p;
        }

        if (sci_p != NULL) {
            VTSS_I("Found SCI:"SCI_FMT, SCI_ARG(*sci_p));
        } else {
            VTSS_I("Found SCI == NULL");
        }

        rc = (search_sci == sci_p) ? VTSS_RC_ERROR : VTSS_RC_OK;
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_macsec_rx_sc_status_get(const vtss_inst_t               inst,
                                     const vtss_macsec_port_t        port,
                                     const vtss_macsec_sci_t         *const sci,
                                     vtss_macsec_rx_sc_status_t      *const status)
{
    vtss_macsec_internal_secy_t *secy;
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_ERROR;
    u32 secy_id, an, sc;

    VTSS_D(MPORT_FMT, MPORT_ARG(port));
    VTSS_ENTER();
    if ((rc = vtss_macsec_port_check(inst, &vtss_state, port, 0, &secy_id)) == VTSS_RC_OK) {
        secy = &vtss_state->macsec_conf[port.port_no].secy[secy_id];
        if ((rc = sc_from_sci_get(secy, sci, &sc)) != VTSS_RC_OK) {
            VTSS_E("Could not find SC (from sci)");
        } else {
            secy->rx_sc[sc]->status.receiving = 0;
            for (an = 0; an < VTSS_MACSEC_SA_PER_SC_MAX; an++) {
                if (secy->rx_sc[sc]->sa[an] == NULL) {
                    continue;
                }
                if (secy->rx_sc[sc]->sa[an]->in_use) {
                    secy->rx_sc[sc]->status.receiving = 1;
                }
            }
            *status = secy->rx_sc[sc]->status;
            VTSS_D("Status.receiving:%d created:%d started:%d stopped:%d",
                   status->receiving, status->created_time, status->started_time, status->stopped_time);
        }
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_macsec_tx_sc_set(const vtss_inst_t           inst,
                              const vtss_macsec_port_t    port)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_ERROR;
    u32 secy_id;

    VTSS_I(MPORT_FMT, MPORT_ARG(port));
    VTSS_ENTER();
    if ((rc = vtss_macsec_port_check(inst, &vtss_state, port, 0, &secy_id)) == VTSS_RC_OK) {
        rc = vtss_macsec_tx_sc_set_priv(vtss_state, port, secy_id);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_macsec_tx_sc_update(const vtss_inst_t              inst,
                                 const vtss_macsec_port_t       port,
                                 const vtss_macsec_tx_sc_conf_t *const conf)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_ERROR;
    vtss_macsec_internal_secy_t *secy;
    u32 secy_id;

    VTSS_I(MPORT_FMT" conf:"MACSEC_TX_SC_CONF_FMT, MPORT_ARG(port),
           MACSEC_TX_SC_CONF_ARG(*conf));
    VTSS_MACSEC_ASSERT(conf->confidentiality_offset > 64, "Confidentiality offset value not supported");
    VTSS_ENTER();
    if ((rc = vtss_macsec_port_check(inst, &vtss_state, port, 0, &secy_id)) == VTSS_RC_OK) {
        secy = &vtss_state->macsec_conf[port.port_no].secy[secy_id];
        if (secy->tx_sc.in_use) {
            secy->conf.protect_frames         = conf->protect_frames;
            secy->conf.always_include_sci     = conf->always_include_sci;
            secy->conf.use_es                 = conf->use_es;
            secy->conf.use_scb                = conf->use_scb;
            secy->conf.confidentiality_offset = conf->confidentiality_offset;
        } else {
            VTSS_E("TX_SC does not exist");
            rc = VTSS_RC_ERROR;
        }
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_macsec_tx_sc_del(const vtss_inst_t           inst,
                              const vtss_macsec_port_t    port)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_ERROR;
    u32 secy_id;

    VTSS_I(MPORT_FMT, MPORT_ARG(port));
    VTSS_ENTER();
    if ((rc = vtss_macsec_port_check(inst, &vtss_state, port, 0, &secy_id)) == VTSS_RC_OK) {
        rc = vtss_macsec_tx_sc_del_priv(vtss_state, port, secy_id);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_macsec_tx_sc_status_get(const vtss_inst_t           inst,
                                     const vtss_macsec_port_t    port,
                                     vtss_macsec_tx_sc_status_t  *const status)
{
    vtss_macsec_internal_secy_t *secy;
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_ERROR;
    u32 secy_id, an;

    VTSS_D(MPORT_FMT, MPORT_ARG(port));
    VTSS_ENTER();
    if ((rc = vtss_macsec_port_check(inst, &vtss_state, port, 0, &secy_id)) == VTSS_RC_OK) {
        secy = &vtss_state->macsec_conf[port.port_no].secy[secy_id];
        if (secy->tx_sc.in_use) {
            secy->tx_sc.status.transmitting = 0;
            for (an = 0; an < VTSS_MACSEC_SA_PER_SC_MAX; an++) {
                if (secy->tx_sc.sa[an] == NULL) {
                    continue;
                }
                if (secy->tx_sc.sa[an]->in_use) {
                    secy->tx_sc.status.transmitting = 1;
                }
            }
            *status = secy->tx_sc.status;
            VTSS_D("Status enciphering:%d encoding:%d transmitting:%d created:%d started:%d stopped:%d", status->enciphering_sa, status->encoding_sa,
                   status->transmitting, status->created_time, status->started_time, status->stopped_time);
        } else {
            VTSS_E("TX_SC does not exist");
            rc = VTSS_RC_ERROR;
        }
    }
    VTSS_EXIT();
    if (rc == VTSS_RC_OK) {
        VTSS_D(" -> status:"MACSEC_TX_SC_STATUS_FMT,
               MACSEC_TX_SC_STATUS_ARG(*status));
    }
    return rc;
}

vtss_rc vtss_macsec_tx_sc_get_conf(const vtss_inst_t              inst,
                                   const vtss_macsec_port_t       port,
                                   vtss_macsec_tx_sc_conf_t       *const conf)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_ERROR;
    vtss_macsec_internal_secy_t *secy;
    u32 secy_id;

    VTSS_I("Port: %u/%u/%u", MACSEC_PORT_ARG(&port));
    VTSS_ENTER();
    if ((rc = vtss_macsec_port_check(inst, &vtss_state, port, 0, &secy_id)) == VTSS_RC_OK) {
        secy = &vtss_state->macsec_conf[port.port_no].secy[secy_id];
        if (secy->tx_sc.in_use) {
            conf->protect_frames         = secy->conf.protect_frames;
            conf->always_include_sci     = secy->conf.always_include_sci;
            conf->use_es                 = secy->conf.use_es;
            conf->use_scb                = secy->conf.use_scb;
            conf->confidentiality_offset = secy->conf.confidentiality_offset;
        } else {
            VTSS_E("TX_SC does not exist");
            rc = VTSS_RC_ERROR;
        }
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_macsec_rx_sc_get_conf(const vtss_inst_t              inst,
                                   const vtss_macsec_port_t       port,
                                   const vtss_macsec_sci_t        *const sci,
                                   vtss_macsec_rx_sc_conf_t       *const conf)
{
    vtss_state_t *vtss_state;
    vtss_macsec_internal_secy_t *secy;
    vtss_rc rc = VTSS_RC_ERROR;
    u32 secy_id, sc;

    VTSS_I("Port: %u/%u/%u", MACSEC_PORT_ARG(&port));
    VTSS_ENTER();
    if ((rc = vtss_macsec_port_check(inst, &vtss_state, port, 0, &secy_id)) == VTSS_RC_OK) {
        secy = &vtss_state->macsec_conf[port.port_no].secy[secy_id];
        if ((rc = sc_from_sci_get(secy, sci, &sc)) != VTSS_RC_OK) {
            VTSS_E("Could not find SC (from sci)");
        } else {
            *conf = secy->rx_sc[sc]->conf;
        }
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_macsec_rx_sa_set(const vtss_inst_t             inst,
                              const vtss_macsec_port_t      port,
                              const vtss_macsec_sci_t       *const sci,
                              const u16                     an,
                              const u32                     lowest_pn,
                              const vtss_macsec_sak_t       *const sak)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_ERROR;
    u32 secy_id;

    VTSS_I(MPORT_SCI_AN_FMT", pn:%u cipher:%u", MPORT_SCI_AN_ARG(port, *sci, an), lowest_pn, sak->len == 16 ? 128 : 256);
    prnt_sak(sak);
    VTSS_ENTER();
    if ((rc = vtss_macsec_port_check(inst, &vtss_state, port, 0, &secy_id)) == VTSS_RC_OK) {
        rc = vtss_macsec_rx_sa_set_priv(vtss_state, secy_id, port, sci, an, lowest_pn, sak);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_macsec_rx_sa_get(const vtss_inst_t             inst,
                              const vtss_macsec_port_t      port,
                              const vtss_macsec_sci_t       *const sci,
                              const u16                     an,
                              u32                           *const lowest_pn,
                              vtss_macsec_sak_t             *const sak,
                              BOOL                          *const active)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_ERROR;
    u32 secy_id;

    VTSS_D(MPORT_SCI_AN_FMT, MPORT_SCI_AN_ARG(port, *sci, an));
    VTSS_ENTER();
    if ((rc = vtss_macsec_port_check(inst, &vtss_state, port, 0, &secy_id)) == VTSS_RC_OK) {
        rc = vtss_macsec_rx_sa_get_priv(vtss_state, secy_id, port, sci, an, lowest_pn, sak, active);
    }
    VTSS_EXIT();

    if (rc == VTSS_RC_OK) {
        VTSS_D("-> pn:%u, active:%s", *lowest_pn, BOOL_ARG(*active));
    }
    return rc;
}

vtss_rc vtss_macsec_rx_sa_activate(const vtss_inst_t             inst,
                                   const vtss_macsec_port_t      port,
                                   const vtss_macsec_sci_t       *const sci,
                                   const u16                     an)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_ERROR;
    u32 secy_id;

    VTSS_I(MPORT_SCI_AN_FMT, MPORT_SCI_AN_ARG(port, *sci, an));
    VTSS_ENTER();
    if ((rc = vtss_macsec_port_check(inst, &vtss_state, port, 0, &secy_id)) == VTSS_RC_OK) {
        rc = vtss_macsec_rx_sa_activate_priv(vtss_state, secy_id, port, sci, an);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_macsec_rx_sa_disable(const vtss_inst_t         inst,
                                  const vtss_macsec_port_t  port,
                                  const vtss_macsec_sci_t   *const sci,
                                  const u16                 an)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_ERROR;
    u32 secy_id;

    VTSS_I(MPORT_SCI_AN_FMT, MPORT_SCI_AN_ARG(port, *sci, an));
    VTSS_ENTER();
    if ((rc = vtss_macsec_port_check(inst, &vtss_state, port, 0, &secy_id)) == VTSS_RC_OK) {
        rc = vtss_macsec_rx_sa_disable_priv(vtss_state, secy_id, port, sci, an);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_macsec_rx_sa_del(const vtss_inst_t         inst,
                              const vtss_macsec_port_t  port,
                              const vtss_macsec_sci_t   *const sci,
                              const u16                 an)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_ERROR;
    u32 secy_id;

    VTSS_I(MPORT_SCI_AN_FMT, MPORT_SCI_AN_ARG(port, *sci, an));
    VTSS_ENTER();
    if ((rc = vtss_macsec_port_check(inst, &vtss_state, port, 0, &secy_id)) == VTSS_RC_OK) {
        rc = vtss_macsec_rx_sa_del_priv(vtss_state, secy_id, port, sci, an);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_macsec_rx_sa_lowest_pn_update(const vtss_inst_t            inst,
                                           const vtss_macsec_port_t     port,
                                           const vtss_macsec_sci_t      *const sci,
                                           const u16                    an,
                                           const u32                    lowest_pn)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_ERROR;
    u32 secy_id;

    VTSS_I(MPORT_SCI_AN_FMT", pn: %u", MPORT_SCI_AN_ARG(port, *sci, an),
           lowest_pn);
    VTSS_ENTER();
    if ((rc = vtss_macsec_port_check(inst, &vtss_state, port, 0, &secy_id)) == VTSS_RC_OK) {
        rc = vtss_macsec_rx_sa_lowest_pn_update_priv(vtss_state, secy_id, port, sci, an, lowest_pn);
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_macsec_rx_sa_status_get(const vtss_inst_t           inst,
                                     const vtss_macsec_port_t    port,
                                     const vtss_macsec_sci_t     *const sci,
                                     const u16                   an,
                                     vtss_macsec_rx_sa_status_t  *const status)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_ERROR;
    u32 secy_id;

    VTSS_D(MPORT_SCI_AN_FMT, MPORT_SCI_AN_ARG(port, *sci, an));
    VTSS_ENTER();
    if ((rc = vtss_macsec_port_check(inst, &vtss_state, port, 0, &secy_id)) == VTSS_RC_OK) {
        rc = vtss_macsec_rx_sa_status_get_priv(vtss_state, secy_id, port, sci, an, status);
    }
    VTSS_EXIT();

    if (rc == VTSS_RC_OK) {
        VTSS_D(" -> status:"MACSEC_RX_SA_STATUS_FMT, MACSEC_RX_SA_STATUS_ARG(*status));
    }
    return rc;
}

vtss_rc vtss_macsec_tx_sa_set(const vtss_inst_t              inst,
                              const vtss_macsec_port_t       port,
                              const u16                      an,
                              const u32                      next_pn,
                              const BOOL                     confidentiality,
                              const vtss_macsec_sak_t        *const sak)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_ERROR;
    u32 secy_id;

    VTSS_I(MPORT_AN_FMT" pn:%u, confidentiality:%d cipher:%u", MPORT_AN_ARG(port, an),
           next_pn, confidentiality, sak->len == 16 ? 128 : 256);
    prnt_sak(sak);
    VTSS_ENTER();
    if ((rc = vtss_macsec_port_check(inst, &vtss_state, port, 0, &secy_id)) == VTSS_RC_OK) {
        rc = vtss_macsec_tx_sa_set_priv(vtss_state, secy_id, port, an, next_pn, confidentiality, sak);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_macsec_tx_sa_get(const vtss_inst_t              inst,
                              const vtss_macsec_port_t       port,
                              const u16                      an,
                              u32                            *const next_pn,
                              BOOL                           *const confidentiality,
                              vtss_macsec_sak_t              *const sak,
                              BOOL                           *const active)
{
    vtss_macsec_internal_secy_t *secy;
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_ERROR;
    u32 secy_id;

    VTSS_D(MPORT_AN_FMT, MPORT_AN_ARG(port, an));
    VTSS_MACSEC_ASSERT(an >= VTSS_MACSEC_SA_PER_SC_MAX, "AN is invalid");

    VTSS_ENTER();
    if ((rc = vtss_macsec_port_check(inst, &vtss_state, port, 0, &secy_id)) == VTSS_RC_OK) {
        secy = &vtss_state->macsec_conf[port.port_no].secy[secy_id];
        if (secy->tx_sc.sa[an] == NULL) {
            VTSS_E("AN does not exist");
            rc = VTSS_RC_ERROR;
        } else {
            *sak             = secy->tx_sc.sa[an]->sak;
            *active          = secy->tx_sc.sa[an]->enabled;
            *confidentiality = secy->tx_sc.sa[an]->confidentiality;
            *next_pn         = secy->tx_sc.sa[an]->status.next_pn;
        }
    }
    VTSS_EXIT();

    if (rc == VTSS_RC_OK) {
        VTSS_D(" -> pn:%u, confidentiality:%s, active:%s", *next_pn,
               BOOL_ARG(*confidentiality), BOOL_ARG(*active));
    }

    return rc;
}

vtss_rc vtss_macsec_tx_sa_activate(const vtss_inst_t         inst,
                                   const vtss_macsec_port_t  port,
                                   const u16                 an)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_ERROR;
    u32 secy_id;

    VTSS_I(MPORT_AN_FMT, MPORT_AN_ARG(port, an));
    VTSS_ENTER();
    if ((rc = vtss_macsec_port_check(inst, &vtss_state, port, 0, &secy_id)) == VTSS_RC_OK) {
        rc = vtss_macsec_tx_sa_activate_priv(vtss_state, secy_id, port, an);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_macsec_tx_sa_disable(const vtss_inst_t         inst,
                                  const vtss_macsec_port_t  port,
                                  const u16                 an)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_ERROR;
    u32 secy_id;

    VTSS_I(MPORT_AN_FMT, MPORT_AN_ARG(port, an));
    VTSS_ENTER();
    if ((rc = vtss_macsec_port_check(inst, &vtss_state, port, 0, &secy_id)) == VTSS_RC_OK) {
        rc = vtss_macsec_tx_sa_disable_priv(vtss_state, secy_id, port, an);
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_macsec_tx_sa_del(const vtss_inst_t         inst,
                              const vtss_macsec_port_t  port,
                              const u16                 an)
{

    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_ERROR;
    u32 secy_id;

    VTSS_I(MPORT_AN_FMT, MPORT_AN_ARG(port, an));
    VTSS_ENTER();
    if ((rc = vtss_macsec_port_check(inst, &vtss_state, port, 0, &secy_id)) == VTSS_RC_OK) {
        rc = vtss_macsec_tx_sa_del_priv(vtss_state, secy_id, port, an);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_macsec_tx_sa_status_get(const vtss_inst_t           inst,
                                     const vtss_macsec_port_t    port,
                                     const u16                   an,
                                     vtss_macsec_tx_sa_status_t  *const status)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_ERROR;
    vtss_macsec_internal_secy_t *secy;
    u32 secy_id;

    VTSS_D(MPORT_AN_FMT, MPORT_AN_ARG(port, an));
    VTSS_ENTER();
    if ((rc = vtss_macsec_port_check(inst, &vtss_state, port, 0, &secy_id)) == VTSS_RC_OK) {
        if ((rc = vtss_macsec_tx_sa_status_get_priv(vtss_state, secy_id, port, an)) == VTSS_RC_OK) {
            secy = &vtss_state->macsec_conf[port.port_no].secy[secy_id];
            *status = secy->tx_sc.sa[an]->status;
        }
    }
    VTSS_EXIT();

    if (rc == VTSS_RC_OK) {
        VTSS_D(" -> status:"MACSEC_TX_SA_STATUS_FMT,
               MACSEC_TX_SA_STATUS_ARG(*status));
    }

    return rc;
}

vtss_rc vtss_macsec_controlled_counters_get(const vtss_inst_t                  inst,
                                            const vtss_macsec_port_t           port,
                                            vtss_macsec_secy_port_counters_t  *const counters)
{
    vtss_state_t *vtss_state;
    vtss_rc rc;
    u32 secy_id;
    VTSS_D("Port: %u/%u/%u", MACSEC_PORT_ARG(&port));

    VTSS_ENTER();
    if ((rc = vtss_macsec_port_check(inst, &vtss_state, port, 0, &secy_id)) == VTSS_RC_OK) {
        rc = vtss_macsec_controlled_counters_get_priv(vtss_state, port, counters, secy_id);
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_macsec_uncontrolled_counters_get(const vtss_inst_t                   inst,
                                              const vtss_port_no_t                port_no,
                                              vtss_macsec_uncontrolled_counters_t *const counters)
{
    vtss_state_t *vtss_state;
    vtss_rc rc;

    VTSS_D("Port: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_macsec_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = vtss_macsec_uncontrolled_counters_get_priv(vtss_state, port_no, counters);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_macsec_common_counters_get(const vtss_inst_t               inst,
                                        const vtss_port_no_t            port_no,
                                        vtss_macsec_common_counters_t   *const counters)
{
    vtss_state_t *vtss_state;
    vtss_rc rc;

    VTSS_D("Port: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_macsec_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = vtss_macsec_common_counters_get_priv(vtss_state, port_no, counters, FALSE);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_macsec_secy_cap_get(const vtss_inst_t             inst,
                                 const vtss_port_no_t          port_no,
                                 vtss_macsec_secy_cap_t        *const cap)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_ERROR;

    VTSS_D("Port: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_macsec_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        if (phy_is_1g(vtss_state, port_no)) {
            cap->max_peer_scs = VTSS_MACSEC_1G_MAX_SA / 2;
            cap->max_receive_keys = VTSS_MACSEC_1G_MAX_SA;
            cap->max_transmit_keys = VTSS_MACSEC_1G_MAX_SA;
        } else {
            cap->max_peer_scs = VTSS_MACSEC_10G_MAX_SA / 2;
            cap->max_receive_keys = VTSS_MACSEC_10G_MAX_SA;
            cap->max_transmit_keys = VTSS_MACSEC_10G_MAX_SA;
        }
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_macsec_secy_counters_get(const vtss_inst_t             inst,
                                      const vtss_macsec_port_t      port,
                                      vtss_macsec_secy_counters_t   *const counters)
{
    vtss_state_t *vtss_state;
    vtss_rc rc;
    u32 secy_id;

    VTSS_D("Port: %u/%u/%u", MACSEC_PORT_ARG(&port));
    VTSS_ENTER();
    if ((rc = vtss_macsec_port_check(inst, &vtss_state, port, 0, &secy_id)) == VTSS_RC_OK) {
        rc = vtss_macsec_secy_counters_get_priv(vtss_state, port.port_no, counters, secy_id);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_macsec_counters_update(const vtss_inst_t     inst,
                                    const vtss_port_no_t  port_no)

{
    vtss_state_t *vtss_state;
    vtss_rc rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_macsec_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = vtss_macsec_counters_update_priv(vtss_state, port_no);
    }
    VTSS_EXIT();
    return rc;
}
vtss_rc vtss_macsec_counters_clear(const vtss_inst_t     inst,
                                   const vtss_port_no_t  port_no)
{
    vtss_state_t *vtss_state;
    vtss_rc rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_macsec_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = vtss_macsec_counters_clear_priv(vtss_state, port_no);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_macsec_rx_sc_counters_get(const vtss_inst_t               inst,
                                       const vtss_macsec_port_t        port,
                                       const vtss_macsec_sci_t         *const sci,
                                       vtss_macsec_rx_sc_counters_t    *const counters)
{
    vtss_state_t *vtss_state;
    vtss_rc rc;
    u32 secy_id;

    VTSS_D("Port: %u/%u/%u", MACSEC_PORT_ARG(&port));
    VTSS_ENTER();
    if ((rc = vtss_macsec_port_check(inst, &vtss_state, port, 0, &secy_id)) == VTSS_RC_OK) {
        rc = vtss_macsec_rx_sc_counters_get_priv(vtss_state, port.port_no, sci, counters, secy_id);
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_macsec_tx_sc_counters_get(const vtss_inst_t               inst,
                                       const vtss_macsec_port_t        port,
                                       vtss_macsec_tx_sc_counters_t    *const counters)
{
    vtss_state_t *vtss_state;
    vtss_rc rc;
    u32 secy_id;

    VTSS_D("Port: %u/%u/%u", MACSEC_PORT_ARG(&port));
    VTSS_ENTER();
    if ((rc = vtss_macsec_port_check(inst, &vtss_state, port, 0, &secy_id)) == VTSS_RC_OK) {
        rc = vtss_macsec_tx_sc_counters_get_priv(vtss_state, port.port_no, counters, secy_id);
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_macsec_tx_sa_counters_get(const vtss_inst_t               inst,
                                       const vtss_macsec_port_t        port,
                                       const u16                       an,
                                       vtss_macsec_tx_sa_counters_t    *const counters)
{
    vtss_state_t *vtss_state;
    vtss_rc rc;
    u32 secy_id;

    VTSS_D("Port: %u/%u/%u an:%u", MACSEC_PORT_ARG(&port), an);
    VTSS_ENTER();
    if ((rc = vtss_macsec_port_check(inst, &vtss_state, port, 0, &secy_id)) == VTSS_RC_OK) {
        rc = vtss_macsec_tx_sa_counters_get_priv(vtss_state, port.port_no, an, counters, secy_id);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_macsec_rx_sa_counters_get(const vtss_inst_t            inst,
                                       const vtss_macsec_port_t     port,
                                       const vtss_macsec_sci_t      *const sci,
                                       const u16                    an,
                                       vtss_macsec_rx_sa_counters_t *const counters)
{
    vtss_state_t *vtss_state;
    vtss_rc rc;
    u32 secy_id;

    VTSS_D(MPORT_SCI_AN_FMT, MPORT_SCI_AN_ARG(port, *sci, an));
    VTSS_ENTER();
    if ((rc = vtss_macsec_port_check(inst, &vtss_state, port, 0, &secy_id)) == VTSS_RC_OK) {
        rc = vtss_macsec_rx_sa_counters_get_priv(vtss_state, port.port_no, sci, an, counters, secy_id);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_macsec_control_frame_match_conf_set(const vtss_inst_t                             inst,
                                                 const vtss_port_no_t                          port_no,
                                                 const vtss_macsec_control_frame_match_conf_t *const conf,
                                                 u32                                          *const rule_id)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_ERROR;
    vtss_macsec_control_frame_match_conf_t match = *conf;
    u32 local_rule, *rule_id_p = NULL;

    {
        char buf[256];
        (void) vtss_macsec_frame_match_to_txt(buf, 256, conf);
        VTSS_I("port_no:%u. Control frame match: %s", port_no, buf);
    }

    VTSS_ENTER();
    if ((rc = vtss_inst_macsec_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rule_id_p = (rule_id == NULL) ? &local_rule : rule_id;
        if ((rc = vtss_macsec_control_frame_match_conf_set_priv(vtss_state, port_no, &match, rule_id_p, 1)) == VTSS_RC_OK) {
            vtss_state->macsec_conf[port_no].glb.control_match[*rule_id_p] = *conf;
            VTSS_I("Rule_id used:%u", *rule_id_p);
        }
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_macsec_control_frame_match_conf_del(const vtss_inst_t                             inst,
                                                 const vtss_port_no_t                          port_no,
                                                 const u32                                     rule_id)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_ERROR;
    vtss_macsec_control_frame_match_conf_t match;
    u32 rule = rule_id;

    VTSS_I("port_no: %u rule_id:%u", port_no, rule_id);
    VTSS_ENTER();
    if ((rc = vtss_inst_macsec_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        if (rule_id >= VTSS_MACSEC_CP_RULES) {
            VTSS_E("Rule id is out of range");
        } else if (vtss_state->macsec_conf[port_no].glb.control_match[rule_id].match == VTSS_MACSEC_MATCH_DISABLE) {
            VTSS_E("Rule does not exist (id:%u)", rule_id);
        } else {
            match = vtss_state->macsec_conf[port_no].glb.control_match[rule_id];
            rc = vtss_macsec_control_frame_match_conf_set_priv(vtss_state, port_no, &match, &rule, 0);
            memset(&vtss_state->macsec_conf[port_no].glb.control_match[rule_id], 0, sizeof(vtss_macsec_control_frame_match_conf_t));
            vtss_state->macsec_conf[port_no].glb.control_match[rule_id].match = VTSS_MACSEC_MATCH_DISABLE;
        }
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_macsec_control_frame_match_conf_get(const vtss_inst_t                             inst,
                                                 const vtss_port_no_t                          port_no,
                                                 vtss_macsec_control_frame_match_conf_t       *const conf,
                                                 u32                                           rule_id)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_ERROR;

    VTSS_I("port_no: %u rule_id:%u", port_no, rule_id);
    VTSS_ENTER();
    if ((rc = vtss_inst_macsec_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        if (rule_id >= VTSS_MACSEC_CP_RULES) {
            VTSS_E("Rule id is out of range");
        } else {
            *conf = vtss_state->macsec_conf[port_no].glb.control_match[rule_id];
        }
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_macsec_default_action_set(const vtss_inst_t                          inst,
                                       const vtss_port_no_t                       port_no,
                                       const vtss_macsec_default_action_policy_t *const policy)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_ERROR;

    VTSS_I("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_macsec_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = vtss_macsec_default_action_set_priv(vtss_state, port_no, policy);
        vtss_state->macsec_conf[port_no].glb.default_action = *policy;
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_macsec_default_action_get(const vtss_inst_t                          inst,
                                       const vtss_port_no_t                       port_no,
                                       vtss_macsec_default_action_policy_t       *const policy)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_ERROR;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_macsec_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        *policy = vtss_state->macsec_conf[port_no].glb.default_action;
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_macsec_pattern_set(const vtss_inst_t                  inst,
                                const vtss_macsec_port_t           port,
                                const vtss_macsec_direction_t      direction,
                                const vtss_macsec_match_action_t   action,
                                const vtss_macsec_match_pattern_t  *const pattern)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_ERROR;
    vtss_macsec_internal_secy_t *secy;
    u32 secy_id;

    {
        char buf[256];
        (void) vtss_macsec_match_pattern_to_txt(buf, 256, pattern);
        VTSS_I("port_no:%u action:%s dir:%s pattern match:%s",
               port.port_no, action == VTSS_MACSEC_MATCH_ACTION_DROP ? "drop"
               : action == VTSS_MACSEC_MATCH_ACTION_CONTROLLED_PORT ? "ctrl" : "unctrl",
               direction == VTSS_MACSEC_DIRECTION_INGRESS ? "ingr" : "egr", buf);
    }

    VTSS_ENTER();
    do {
        if ((rc = vtss_macsec_port_check(inst, &vtss_state, port, 0, &secy_id)) != VTSS_RC_OK) {
            break;
        }
        secy = &vtss_state->macsec_conf[port.port_no].secy[secy_id];
        if (action == VTSS_MACSEC_MATCH_ACTION_DROP || action == VTSS_MACSEC_MATCH_ACTION_UNCONTROLLED_PORT) {
            if ((rc = vtss_macsec_pattern_set_priv(vtss_state, port.port_no, secy_id, direction, action, pattern, 0)) != VTSS_RC_OK) {
                break;
            }
        }
        secy->pattern[action][direction] = *pattern;
    } while (0);
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_macsec_pattern_del(const vtss_inst_t                  inst,
                                const vtss_macsec_port_t           port,
                                const vtss_macsec_direction_t      direction,
                                const vtss_macsec_match_action_t   action)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_ERROR;
    vtss_macsec_internal_secy_t *secy;
    u32 secy_id;

    VTSS_I("Port: %u/%u/%u", MACSEC_PORT_ARG(&port));
    VTSS_ENTER();
    do {
        if ((rc = vtss_macsec_port_check(inst, &vtss_state, port, 0, &secy_id)) != VTSS_RC_OK) {
            break;
        }
        secy = &vtss_state->macsec_conf[port.port_no].secy[secy_id];
        if (action == VTSS_MACSEC_MATCH_ACTION_DROP || action == VTSS_MACSEC_MATCH_ACTION_UNCONTROLLED_PORT) {
            if ((rc = vtss_macsec_pattern_del_priv(vtss_state, port, secy_id, direction, action)) != VTSS_RC_OK) {
                break;
            }
        }
        memset(&secy->pattern[action][direction], 0, sizeof(secy->pattern[action][direction]));
        secy->pattern[action][direction].match = VTSS_MACSEC_MATCH_DISABLE;
    } while (0);
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_macsec_pattern_get(const vtss_inst_t                  inst,
                                const vtss_macsec_port_t           port,
                                const vtss_macsec_direction_t      direction,
                                const vtss_macsec_match_action_t   action,
                                vtss_macsec_match_pattern_t        *const pattern)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_ERROR;
    vtss_macsec_internal_secy_t *secy;
    u32 secy_id;

    VTSS_D("Port: %u/%u/%u", MACSEC_PORT_ARG(&port));
    VTSS_ENTER();
    if ((rc = vtss_macsec_port_check(inst, &vtss_state, port, 0, &secy_id)) == VTSS_RC_OK) {
        secy = &vtss_state->macsec_conf[port.port_no].secy[secy_id];
        if (secy->pattern[action][direction].match == VTSS_MACSEC_MATCH_DISABLE) {
            VTSS_E("No pattern is configured");
            rc = VTSS_RC_ERROR;
        } else {
            *pattern = secy->pattern[action][direction];
        }
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_macsec_bypass_mode_set(const vtss_inst_t                inst,
                                    const vtss_port_no_t             port_no,
                                    const vtss_macsec_bypass_mode_t  *const bypass)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_ERROR;

    VTSS_I("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_macsec_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = vtss_macsec_bypass_mode_set_priv(vtss_state, port_no, bypass);
        vtss_state->macsec_conf[port_no].glb.bypass_mode = *bypass;
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_macsec_bypass_mode_get(const vtss_inst_t          inst,
                                    const vtss_port_no_t       port_no,
                                    vtss_macsec_bypass_mode_t  *const bypass)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_ERROR;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_macsec_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        *bypass = vtss_state->macsec_conf[port_no].glb.bypass_mode;
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_macsec_bypass_tag_set(const vtss_inst_t              inst,
                                   const vtss_macsec_port_t       port,
                                   const vtss_macsec_tag_bypass_t tag)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_ERROR;
    vtss_macsec_internal_secy_t *secy;
    u32 secy_id;

    VTSS_I("Port: %u/%u/%u", MACSEC_PORT_ARG(&port));
    VTSS_ENTER();
    if ((rc = vtss_macsec_port_check(inst, &vtss_state, port, 0, &secy_id)) == VTSS_RC_OK) {
        secy = &vtss_state->macsec_conf[port.port_no].secy[secy_id];
        secy->tag_bypass = tag;
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_macsec_bypass_tag_get(const vtss_inst_t             inst,
                                   const vtss_macsec_port_t      port,
                                   vtss_macsec_tag_bypass_t      *const tag)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_ERROR;
    vtss_macsec_internal_secy_t *secy;
    u32 secy_id;

    VTSS_D("Port: %u/%u/%u", MACSEC_PORT_ARG(&port));
    VTSS_ENTER();
    if ((rc = vtss_macsec_port_check(inst, &vtss_state, port, 0, &secy_id)) == VTSS_RC_OK) {
        secy = &vtss_state->macsec_conf[port.port_no].secy[secy_id];
        *tag = secy->tag_bypass;
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_macsec_frame_capture_set(const vtss_inst_t                   inst,
                                      const vtss_port_no_t                port_no,
                                      const vtss_macsec_frame_capture_t   capture)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_ERROR;

    VTSS_I("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_macsec_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = vtss_macsec_frame_capture_set_priv(vtss_state, port_no, capture);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_macsec_frame_get(const vtss_inst_t             inst,
                              const vtss_port_no_t          port_no,
                              const u32                     buf_length,
                              u32                           *const frm_length,
                              u8                            *const frame)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_ERROR;

    VTSS_I("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_macsec_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = vtss_macsec_frame_get_priv(vtss_state, port_no, buf_length, frm_length, frame);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_macsec_event_enable_set(const vtss_inst_t           inst,
                                     const vtss_port_no_t        port_no,
                                     const vtss_macsec_event_t   ev_mask,
                                     const BOOL                  enable)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_ERROR;

    VTSS_I("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_macsec_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = vtss_macsec_event_enable_set_priv(vtss_state, port_no, ev_mask, enable);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_macsec_event_enable_get(const vtss_inst_t      inst,
                                     const vtss_port_no_t   port_no,
                                     vtss_macsec_event_t    *const ev_mask)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_ERROR;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_macsec_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = vtss_macsec_event_enable_get_priv(vtss_state, port_no, ev_mask);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_macsec_event_poll(const vtss_inst_t     inst,
                               const vtss_port_no_t  port_no,
                               vtss_macsec_event_t  *const ev_mask)
{

    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_ERROR;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_macsec_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = vtss_macsec_event_poll_priv(vtss_state, port_no, ev_mask);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_macsec_event_seq_threshold_set(const vtss_inst_t     inst,
                                            const vtss_port_no_t  port_no,
                                            const u32             threshold)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_ERROR;

    VTSS_I("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_macsec_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = vtss_macsec_event_seq_threshold_set_priv(vtss_state, port_no, threshold);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_macsec_event_seq_threshold_get(const vtss_inst_t     inst,
                                            const vtss_port_no_t  port_no,
                                            u32                   *const threshold)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_ERROR;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_macsec_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = vtss_macsec_event_seq_threshold_get_priv(vtss_state, port_no, threshold);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_macsec_mtu_get(const vtss_inst_t       inst,
                            const vtss_port_no_t    port_no,
                            vtss_macsec_mtu_t       *mtu_conf)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_ERROR;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_macsec_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        *mtu_conf = vtss_state->macsec_conf[port_no].mtu_conf;
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_macsec_mtu_set(const vtss_inst_t       inst,
                            const vtss_port_no_t    port_no,
                            const vtss_macsec_mtu_t *const mtu_conf)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_ERROR;

    VTSS_I("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_macsec_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        vtss_state->macsec_conf[port_no].mtu_conf.mtu  = mtu_conf->mtu;
        vtss_state->macsec_conf[port_no].mtu_conf.drop = mtu_conf->drop;
        rc = vtss_macsec_mtu_set_priv(vtss_state, port_no);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_macsec_csr_read(const vtss_inst_t           inst,
                             const vtss_port_no_t        port_no,
                             const u32                   block,
                             const u32                   addr,
                             u32                         *const value)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_ERROR;
    u32 mmd;
    u32 val;
    VTSS_ENTER();

    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        if (block == MACSEC_INGR || block == MACSEC_EGR) {
            mmd = 0x1f;
        } else {
            mmd = 3;
        }

        if (csr_rd(vtss_state, port_no, mmd, 1, block + addr, &val) != VTSS_RC_OK) {
            VTSS_E("Could not do CSR read");
        }
        *value = val;
    }
    VTSS_EXIT();

    return rc;
}

vtss_rc vtss_macsec_csr_write(const vtss_inst_t           inst,
                              const vtss_port_no_t        port_no,
                              const u32                   block,
                              const u32                   addr,
                              const u32                   value)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_ERROR;
    u32 mmd;

    VTSS_ENTER();

    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        if (block == MACSEC_INGR || block == MACSEC_EGR) {
            mmd = 0x1f;
        } else {
            mmd = 3;
        }

        if (csr_wr(vtss_state, port_no, mmd, 1, block + addr, value) != VTSS_RC_OK) {
            VTSS_E("Could not do CSR write");
        }

    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_macsec_hmac_counters_get(const vtss_inst_t           inst,
                                      const vtss_port_no_t        port_no,
                                      vtss_macsec_mac_counters_t  *const counters,
                                      const BOOL                  clear)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_ERROR;
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = vtss_macsec_hmac_counters_get_priv(vtss_state, port_no,  counters, clear);
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_macsec_lmac_counters_get(const vtss_inst_t           inst,
                                      const vtss_port_no_t        port_no,
                                      vtss_macsec_mac_counters_t  *const counters,
                                      const BOOL                  clear)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_ERROR;
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = vtss_macsec_lmac_counters_get_priv(vtss_state, port_no,  counters, clear);
    }
    VTSS_EXIT();
    return rc;
}


/* ================================================================= *
 *  Debug
 * ================================================================= */

// Function for printing the counter values in 2 rows (rx to the left, tx to the right).
// In: pr    - Pointer to print function.
//     name  - string with counter name
//     rx_cnt - RX counter value - Set to NULL if there is no RX counter
//     tx_cnt - TX counter value - Set to NULL if there is no TX counter
static void  vtss_macsec_debug_print_counter(const vtss_debug_printf_t pr,
                                             const char *name,
                                             const u64 *rx_cnt,
                                             const u64 *tx_cnt)
{
    if (rx_cnt != NULL) {
        pr("Rx %-25s %-25" PRIu64, name, *rx_cnt);
    } else {

        pr("   %-25s %-25s", "", ""); // Make the space to TX counters
    }

    if (tx_cnt != NULL) {
        pr("Tx %-25s %-25" PRIu64, name, *tx_cnt);
    }

    pr("\n");
}



// Function for printing the counter header.
// In: pr    - Pointer to print function.
//     name  - string with counter header name
static void  vtss_macsec_debug_print_counter_hdr(const vtss_debug_printf_t pr,
                                                 const char *name)
{


    pr("--------------------- %s ---------------------\n", name);
}


void vtss_macsec_debug_print_sticky_dbg(const vtss_debug_printf_t pr,
                                        const char *name,
                                        const u32 value,
                                        const u32 val2,
                                        const u32 bit)
{
    if (value & VTSS_BIT(bit) || val2 & VTSS_BIT(bit)) {
        pr("%-25s  %-15d  %u\n", name, (value & VTSS_BIT(bit)) ? 1 : 0, (val2 & VTSS_BIT(bit)) ? 1 : 0);
    }
}


void  vtss_macsec_debug_print_field_dbg(const vtss_debug_printf_t pr,
                                        const char *name,
                                        const u32 value,
                                        const u32 val2,
                                        const u32 o,
                                        const u32 w,
                                        BOOL hex)
{
    if (hex) {
        pr("%-26s 0x%-14x 0x%x\n", name, VTSS_EXTRACT_BITFIELD(value, o, w), VTSS_EXTRACT_BITFIELD(val2, o, w));
    } else {
        pr("%-26s %-14d   %u\n", name, VTSS_EXTRACT_BITFIELD(value, o, w), VTSS_EXTRACT_BITFIELD(val2, o, w));
    }
}

// Debug function for printing MACSEC configuration in vtss_state
static void vtss_debug_print_macsec_vtss_state(vtss_state_t              *vtss_state,
                                               const vtss_debug_printf_t pr,
                                               const vtss_port_no_t      port_no)
{

    // MTU
    pr("\n*********** MACSEC vtss_state configuration for port:%u ************\n", port_no);
    pr("**** MACSEC vtss_state MTU configuration ****\n");
    pr("mtu:%d  drop:%d\n", vtss_state->macsec_conf[port_no].mtu_conf.mtu, vtss_state->macsec_conf[port_no].mtu_conf.drop);
}


// Function for printing debug infomation
vtss_rc vtss_debug_print_macsec(vtss_state_t *vtss_state,
                                const vtss_debug_printf_t pr,
                                const vtss_debug_info_t *const info)
{
    vtss_port_no_t port_no;
    vtss_macsec_uncontrolled_counters_t uncontrolled_counters;
    vtss_macsec_secy_port_counters_t    controlled_counters;
    vtss_macsec_common_counters_t       common_counters;
    vtss_macsec_rx_sa_counters_t        rx_sa_counters;
    vtss_macsec_tx_sa_counters_t        tx_sa_counters;
    vtss_macsec_tx_sc_counters_t        tx_sc_counters;
    vtss_macsec_rx_sc_counters_t        rx_sc_counters;
    vtss_macsec_secy_counters_t         secy_counters;
    u32 i, value, val2, an;
    u32 sc;
    char str_buf[50];

    if (!vtss_debug_group_enabled(pr, info, VTSS_DEBUG_GROUP_MACSEC)) {
        return VTSS_RC_OK;
    }

    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
        if (!info->port_list[port_no]) {
            continue;
        }
        if (!is_macsec_capable(vtss_state, port_no)) {
            pr("Port%u : No MACSEC support\n", port_no);
        }

        VTSS_N("info->clear:%u", info->clear);
        if (info->clear) {
            pr("Clearing MACSEC counters for port:%u\n", port_no);
            VTSS_RC(vtss_macsec_counters_clear_priv(vtss_state, port_no));
            continue;
        }

        pr("*********** Port:%u ************\n", port_no);

        //
        // Common counters
        //
        vtss_macsec_debug_print_counter_hdr(pr, "Common Counters");
        VTSS_RC(vtss_macsec_common_counters_get_priv(vtss_state, port_no, &common_counters, FALSE));

        vtss_macsec_debug_print_counter(pr, "Octets", &common_counters.if_in_octets, &common_counters.if_out_octets);
        vtss_macsec_debug_print_counter(pr, "Unicast", &common_counters.if_in_ucast_pkts, &common_counters.if_out_ucast_pkts);
        vtss_macsec_debug_print_counter(pr, "Multicast", &common_counters.if_in_multicast_pkts, &common_counters.if_out_multicast_pkts);
        vtss_macsec_debug_print_counter(pr, "Broadcast", &common_counters.if_in_broadcast_pkts, &common_counters.if_out_broadcast_pkts);

        vtss_macsec_debug_print_counter(pr, "Discards", &common_counters.if_in_discards, NULL);
        vtss_macsec_debug_print_counter(pr, "Errors", &common_counters.if_in_errors, &common_counters.if_out_errors);


        //
        // Uncontrolled counters
        //
        vtss_macsec_debug_print_counter_hdr(pr, "Uncontrolled Counters");
        VTSS_RC(vtss_macsec_uncontrolled_counters_get_priv(vtss_state, port_no, &uncontrolled_counters));
        vtss_macsec_debug_print_counter(pr, "Octets",    &uncontrolled_counters.if_in_octets, &uncontrolled_counters.if_out_octets);
        vtss_macsec_debug_print_counter(pr, "Unicast",   &uncontrolled_counters.if_in_ucast_pkts, NULL);
        vtss_macsec_debug_print_counter(pr, "Multicast", &uncontrolled_counters.if_in_multicast_pkts, NULL);
        vtss_macsec_debug_print_counter(pr, "Broadcast", &uncontrolled_counters.if_in_broadcast_pkts, NULL);

        vtss_macsec_debug_print_counter(pr, "Discards",  &uncontrolled_counters.if_in_discards, NULL);
        vtss_macsec_debug_print_counter(pr, "Errors",    &uncontrolled_counters.if_in_errors, &uncontrolled_counters.if_out_errors);

        //
        // Controlled counters
        //
        vtss_macsec_port_t macsec_port;
        macsec_port.service_id = 0;
        macsec_port.port_id = 0;
        macsec_port.port_no = port_no;
        vtss_macsec_internal_secy_t *secy;
        u32 secy_id;
        for (secy_id = 0; secy_id < VTSS_MACSEC_MAX_SECY; secy_id++) {
            secy = &vtss_state->macsec_conf[port_no].secy[secy_id];
            if (secy != NULL && secy->in_use) {
                sprintf(&str_buf[0], "Controlled Counters - Secy:%u", secy_id);
                vtss_macsec_debug_print_counter_hdr(pr, &str_buf[0]);
                VTSS_RC(vtss_macsec_controlled_counters_get_priv(vtss_state, macsec_port, &controlled_counters, secy_id));
                vtss_macsec_debug_print_counter(pr, "Octets", &controlled_counters.if_in_octets, &controlled_counters.if_out_octets);

                vtss_macsec_debug_print_counter(pr, "Packets", &controlled_counters.if_in_pkts, &controlled_counters.if_out_pkts);
                vtss_macsec_debug_print_counter(pr, "Discards", &controlled_counters.if_in_discards, NULL);
                vtss_macsec_debug_print_counter(pr, "Errors", &controlled_counters.if_in_errors, &controlled_counters.if_out_errors);


                sprintf(&str_buf[0], "Secy Counters - Secy:%u", secy_id);
                vtss_macsec_debug_print_counter_hdr(pr, &str_buf[0]);

                VTSS_RC(vtss_macsec_secy_counters_get_priv(vtss_state, port_no, &secy_counters, secy_id));
                vtss_macsec_debug_print_counter(pr, "Packets untagged", &secy_counters.in_pkts_untagged, &secy_counters.out_pkts_untagged);
                vtss_macsec_debug_print_counter(pr, "Packets no tag", &secy_counters.in_pkts_no_tag, NULL);
                vtss_macsec_debug_print_counter(pr, "Packets bad tag", &secy_counters.in_pkts_bad_tag, NULL);
                vtss_macsec_debug_print_counter(pr, "Packets unknown sci", &secy_counters.in_pkts_unknown_sci, NULL);
                vtss_macsec_debug_print_counter(pr, "Packets no sci", &secy_counters.in_pkts_no_sci, NULL);
                vtss_macsec_debug_print_counter(pr, "Packets overrun", &secy_counters.in_pkts_overrun, NULL);

                vtss_macsec_debug_print_counter(pr, "Octets validated", &secy_counters.in_octets_validated, NULL);

                vtss_macsec_debug_print_counter(pr, "Octets decrypted", &secy_counters.in_octets_decrypted, NULL);

                vtss_macsec_debug_print_counter(pr, "Packets too long", NULL, &secy_counters.out_pkts_too_long);
                vtss_macsec_debug_print_counter(pr, "Octets protected", NULL, &secy_counters.out_octets_protected);
                vtss_macsec_debug_print_counter(pr, "Octets encrypted", NULL, &secy_counters.out_octets_encrypted);

                sprintf(&str_buf[0], "SC Counters - Secy:%u", secy_id);
                vtss_macsec_debug_print_counter_hdr(pr, &str_buf[0]);

                VTSS_RC(vtss_macsec_tx_sc_counters_get_priv(vtss_state, port_no, &tx_sc_counters, secy_id));
                vtss_macsec_debug_print_counter(pr, "Packets protected", NULL, &tx_sc_counters.out_pkts_protected);
                vtss_macsec_debug_print_counter(pr, "Packets encrypted", NULL, &tx_sc_counters.out_pkts_encrypted);

                //
                // Tx SA
                //
                for (an = 0; an < VTSS_MACSEC_SA_PER_SC_MAX; an++) {
                    if (secy->tx_sc.sa[an] == NULL || !secy->tx_sc.sa[an]->in_use) {
                        VTSS_I("an:%u", an);
                        continue;
                    }
                    sprintf(&str_buf[0], "TX SA Counters - Secy:%u, an:%u", secy_id, an);
                    vtss_macsec_debug_print_counter_hdr(pr, &str_buf[0]);

                    VTSS_RC(vtss_macsec_tx_sa_counters_get_priv(vtss_state, port_no, an, &tx_sa_counters, secy_id));
                    vtss_macsec_debug_print_counter(pr, "Packets protected", NULL, &tx_sa_counters.out_pkts_protected);
                    vtss_macsec_debug_print_counter(pr, "Packets encrypted", NULL, &tx_sa_counters.out_pkts_encrypted);
                }

                //
                // Rx Secy
                //
                for (sc = 0; sc < VTSS_MACSEC_MAX_SC_RX; sc++) {
                    if (secy->rx_sc[sc] == NULL || !secy->rx_sc[sc]->in_use) {
                        continue;
                    }

                    sprintf(&str_buf[0], "SC Counters - SC:%u", sc);
                    vtss_macsec_debug_print_counter_hdr(pr, &str_buf[0]);

                    // Get rx_sc counter
                    VTSS_RC(vtss_macsec_rx_sc_counters_get_priv(vtss_state,
                                                                port_no,
                                                                &secy->rx_sc[sc]->sci,
                                                                &rx_sc_counters,
                                                                secy_id));

                    vtss_macsec_debug_print_counter(pr, "Packets unchecked", &rx_sc_counters.in_pkts_unchecked, NULL);
                    vtss_macsec_debug_print_counter(pr, "Packets delayed", &rx_sc_counters.in_pkts_delayed, NULL);
                    vtss_macsec_debug_print_counter(pr, "Packets late", &rx_sc_counters.in_pkts_late, NULL);
                    vtss_macsec_debug_print_counter(pr, "Packets ok", &rx_sc_counters.in_pkts_ok, NULL);
                    vtss_macsec_debug_print_counter(pr, "Packets invalid", &rx_sc_counters.in_pkts_invalid, NULL);
                    vtss_macsec_debug_print_counter(pr, "Packets not valid", &rx_sc_counters.in_pkts_not_valid, NULL);
                    vtss_macsec_debug_print_counter(pr, "Packets Not using SA", &rx_sc_counters.in_pkts_not_using_sa, NULL);
                    vtss_macsec_debug_print_counter(pr, "Packets Unused SA", &rx_sc_counters.in_pkts_unused_sa, NULL);

                    // Rx SA
                    for (an = 0; an < VTSS_MACSEC_SA_PER_SC_MAX; an++) {
                        if (secy->rx_sc[sc]->sa[an] == NULL || !secy->rx_sc[sc]->sa[an]->in_use) {
                            VTSS_I("an:%u, NULL:%u", an, secy->rx_sc[sc]->sa[an] == NULL);
                            continue;
                        }
                        sprintf(&str_buf[0], "RX SA Counters - Secy:%u, an:%u", secy_id, an);
                        vtss_macsec_debug_print_counter_hdr(pr, &str_buf[0]);

                        VTSS_RC(vtss_macsec_rx_sa_counters_get_priv(vtss_state, port_no, &secy->rx_sc[sc]->sci, an, &rx_sa_counters, secy_id));
                        vtss_macsec_debug_print_counter(pr, "Packets Ok", &rx_sa_counters.in_pkts_ok, NULL);
                        vtss_macsec_debug_print_counter(pr, "Packets Invalid", &rx_sa_counters.in_pkts_invalid, NULL);
                        vtss_macsec_debug_print_counter(pr, "Packets Not valid", &rx_sa_counters.in_pkts_not_valid, NULL);
                        vtss_macsec_debug_print_counter(pr, "Packets Not using SA", &rx_sa_counters.in_pkts_not_using_sa, NULL);
                        vtss_macsec_debug_print_counter(pr, "Packets Unused SA", &rx_sa_counters.in_pkts_unused_sa, NULL);
                    }
                }
            }
        }


        if (info->full) {
            vtss_debug_print_macsec_vtss_state(vtss_state, pr, port_no);

            if (!vtss_state->macsec_conf[port_no].glb.init.enable) {
                pr("MacSec not enabled\n");
                return VTSS_RC_OK;
            }
            VTSS_RC(csr_rd(vtss_state, port_no, 0x1f, 1, MACSEC_INGR + 0x802, &value));
            VTSS_RC(csr_wr(vtss_state, port_no, 0x1f, 1, MACSEC_INGR + 0x802, 0xFFFFFFFF));
            VTSS_RC(csr_rd(vtss_state, port_no, 0x1f, 1, MACSEC_EGR + 0x802, &val2));
            VTSS_RC(csr_wr(vtss_state, port_no, 0x1f, 1, MACSEC_EGR + 0x802, 0xFFFFFFFF));
            pr("                           INGRESS          EGRESS\n");
            vtss_macsec_debug_print_sticky_dbg(pr, "DROP_CLASS_STICKY", value, val2, 0);
            vtss_macsec_debug_print_sticky_dbg(pr, "DROP_PP_STICKY", value, val2, 1);
            vtss_macsec_debug_print_sticky_dbg(pr, "DROP_MTU_STICKY", value, val2, 2);
            vtss_macsec_debug_print_sticky_dbg(pr, "ENG_IRQ_STICKY", value, val2, 3);
            vtss_macsec_debug_print_sticky_dbg(pr, "DROP_IRQ_STICKY", value, val2, 4);
            vtss_macsec_debug_print_sticky_dbg(pr, "FIFO_OVERFLOW_STICKY", value, val2, 5);
            vtss_macsec_debug_print_sticky_dbg(pr, "SA_STORE_RAM_ERROR_STICKY", value, val2, 6);
            vtss_macsec_debug_print_sticky_dbg(pr, "STAT_RAM_ERROR_STICKY", value, val2, 7);

            VTSS_RC(csr_rd(vtss_state, port_no, 0x1f, 1, 0x1f08, &value));
            VTSS_RC(csr_rd(vtss_state, port_no, 0x1f, 1, MACSEC_EGR + 0x1f08, &val2));
            vtss_macsec_debug_print_field_dbg(pr, "SA_MATCH_SA_HIT", value, val2, 23, 1, 0);
            vtss_macsec_debug_print_field_dbg(pr, "SA_MATCH_SA_NR", value, val2, 16, 7, 0);
            vtss_macsec_debug_print_field_dbg(pr, "SA_MATCH_SOURCE_PORT", value, val2, 24, 2, 0);
            vtss_macsec_debug_print_field_dbg(pr, "SA_MATCH_MACSEC_CLASS", value, val2, 26, 2, 0);
            vtss_macsec_debug_print_field_dbg(pr, "SA_MATCH_CONTROL_PKT", value, val2, 28, 1, 0);

            VTSS_RC(csr_rd(vtss_state, port_no, 0x1f, 1, MACSEC_INGR + 0x1f10, &value));
            VTSS_RC(csr_rd(vtss_state, port_no, 0x1f, 1, MACSEC_EGR + 0x1f10, &val2));
            vtss_macsec_debug_print_field_dbg(pr, "SA Match raw 31_to_0", value, val2, 0, 31, 1);


            VTSS_RC(csr_rd(vtss_state, port_no, 0x1f, 1, 0x1f11, &value));
            VTSS_RC(csr_rd(vtss_state, port_no, 0x1f, 1, MACSEC_EGR + 0x1f11, &val2));
            vtss_macsec_debug_print_field_dbg(pr, "SA Match raw 63_to_32", value, val2, 0, 31, 1);

            VTSS_RC(csr_rd(vtss_state, port_no, 0x1f, 1, 0x1f20, &value));
            VTSS_RC(csr_rd(vtss_state, port_no, 0x1f, 1, MACSEC_EGR + 0x1f20, &val2));
            vtss_macsec_debug_print_field_dbg(pr, "VLAN_PARSED_ETHERTYPE", value, val2, 0, 16, 1);
            vtss_macsec_debug_print_field_dbg(pr, "VLAN_PARSED_VLAN", value, val2, 16, 12, 0);
            vtss_macsec_debug_print_field_dbg(pr, "VLAN_PARSED_UP", value, val2, 28, 3, 0);

            VTSS_RC(csr_rd(vtss_state, port_no, 0x1f, 1, 0x1f21, &value));
            VTSS_RC(csr_rd(vtss_state, port_no, 0x1f, 1, MACSEC_EGR + 0x1f21, &val2));
            vtss_macsec_debug_print_field_dbg(pr, "VLAN_VALID", value, val2, 0, 1, 0);
            vtss_macsec_debug_print_field_dbg(pr, "VLAN_QINIQ_FOUND", value, val2, 1, 1, 0);
            vtss_macsec_debug_print_field_dbg(pr, "VLAN_STAG_VALID", value, val2, 2, 1, 0);
            vtss_macsec_debug_print_field_dbg(pr, "VLAN_QTAG_VALID", value, val2, 3, 1, 0);
            vtss_macsec_debug_print_field_dbg(pr, "VLAN_LPIDLE_PKT", value, val2, 4, 1, 0);
            vtss_macsec_debug_print_field_dbg(pr, "VLAN_ONE_WORD_PKT", value, val2, 5, 1, 0);


            VTSS_RC(csr_rd(vtss_state, port_no, 0x1f, 1, MACSEC_INGR + 0x1f22, &value));
            VTSS_RC(csr_rd(vtss_state, port_no, 0x1f, 1, MACSEC_EGR + 0x1f22, &val2));
            vtss_macsec_debug_print_field_dbg(pr, "VLAN_PARSED_VLAN_INNER", value, val2, 0, 12, 0);
            vtss_macsec_debug_print_field_dbg(pr, "VLAN_PARSED_UP_INNER", value, val2, 12, 3, 0);

            VTSS_RC(csr_rd(vtss_state, port_no, 0x1f, 1, MACSEC_INGR + 0x1f00, &value));
            VTSS_RC(csr_rd(vtss_state, port_no, 0x1f, 1, MACSEC_EGR + 0x1f00, &val2));
            vtss_macsec_debug_print_field_dbg(pr, "CP_MATCH_STATE", value, val2, 0, 31, 1);

            VTSS_RC(csr_rd(vtss_state, port_no, 0x1f, 1, MACSEC_INGR + 0x1f40, &value));
            VTSS_RC(csr_rd(vtss_state, port_no, 0x1f, 1, MACSEC_EGR + 0x1f40, &val2));
            vtss_macsec_debug_print_field_dbg(pr, "PARSED_MAC_DA_LO", value, val2, 0, 31, 1);
            VTSS_RC(csr_rd(vtss_state, port_no, 0x1f, 1, MACSEC_INGR + 0x1f41, &value));
            VTSS_RC(csr_rd(vtss_state, port_no, 0x1f, 1, MACSEC_EGR + 0x1f41, &val2));
            vtss_macsec_debug_print_field_dbg(pr, "PARSED_MAC_DA_HI", value, val2, 0, 16, 1);
            vtss_macsec_debug_print_field_dbg(pr, "PARSED_ETYPE", value, val2, 16, 16, 1);

            VTSS_RC(csr_rd(vtss_state, port_no, 0x1f, 1, MACSEC_INGR + 0x1f42, &value));
            VTSS_RC(csr_rd(vtss_state, port_no, 0x1f, 1, MACSEC_EGR + 0x1f42, &val2));
            vtss_macsec_debug_print_field_dbg(pr, "PARSED_MAC_SA_LO", value, val2, 0, 31, 1);

            VTSS_RC(csr_rd(vtss_state, port_no, 0x1f, 1, MACSEC_INGR + 0x1f43, &value));
            VTSS_RC(csr_rd(vtss_state, port_no, 0x1f, 1, MACSEC_EGR + 0x1f43, &value));
            vtss_macsec_debug_print_field_dbg(pr, "PARSED_MAC_SA_HI", value, val2, 0, 31, 1);

            VTSS_RC(csr_rd(vtss_state, port_no, 0x1f, 1, MACSEC_INGR + 0x1f44, &value));
            VTSS_RC(csr_rd(vtss_state, port_no, 0x1f, 1, MACSEC_EGR + 0x1f44, &val2));
            vtss_macsec_debug_print_field_dbg(pr, "PARSED_SECTAG_LO", value, val2, 0, 31, 1);
            VTSS_RC(csr_rd(vtss_state, port_no, 0x1f, 1, MACSEC_INGR + 0x1f45, &value));
            VTSS_RC(csr_rd(vtss_state, port_no, 0x1f, 1, MACSEC_EGR + 0x1f45, &val2));
            vtss_macsec_debug_print_field_dbg(pr, "PARSED_SECTAG_HI", value, val2, 0, 31, 1);

            VTSS_RC(csr_rd(vtss_state, port_no, 0x1f, 1, MACSEC_INGR + 0x1f46, &value));
            VTSS_RC(csr_rd(vtss_state, port_no, 0x1f, 1, MACSEC_EGR + 0x1f46, &val2));
            vtss_macsec_debug_print_field_dbg(pr, "PARSED_SCI_LO", value, val2, 0, 31, 1);
            VTSS_RC(csr_rd(vtss_state, port_no, 0x1f, 1, MACSEC_INGR + 0x1f47, &value));
            VTSS_RC(csr_rd(vtss_state, port_no, 0x1f, 1, MACSEC_EGR + 0x1f47, &val2));
            vtss_macsec_debug_print_field_dbg(pr, "PARSED_SCI_HI", value, val2, 0, 31, 1);

            VTSS_RC(csr_rd(vtss_state, port_no, 0x1f, 1, MACSEC_INGR + 0x1f48, &value));
            VTSS_RC(csr_rd(vtss_state, port_no, 0x1f, 1, MACSEC_EGR + 0x1f48, &val2));
            vtss_macsec_debug_print_field_dbg(pr, "FLOW_TYPE (BP,DR,I,E)", value, val2, 0, 2, 0);
            vtss_macsec_debug_print_field_dbg(pr, "FLOW_DEST_PORT (CM,R,C,U)", value, val2, 2, 2, 0);
            vtss_macsec_debug_print_field_dbg(pr, "FLOW_DROP_ACTION", value, val2, 6, 2, 0);
            vtss_macsec_debug_print_field_dbg(pr, "FLOW_PROTECT_FRAME", value, val2, 16, 1, 0);
            vtss_macsec_debug_print_field_dbg(pr, "FLOW_SA_IN_USE", value, val2, 17, 1, 0);
            vtss_macsec_debug_print_field_dbg(pr, "FLOW_INCLUDE_SCI", value, val2, 18, 1, 0);
            vtss_macsec_debug_print_field_dbg(pr, "FLOW_USE_ES", value, val2, 19, 1, 0);
            vtss_macsec_debug_print_field_dbg(pr, "FLOW_USE_SCB", value, val2, 20, 1, 0);
            vtss_macsec_debug_print_field_dbg(pr, "FLOW_TAG_BYPASS_SIZE", value, val2, 21, 2, 0);
            vtss_macsec_debug_print_field_dbg(pr, "FLOW_CONF_OFFSET", value, val2, 24, 7, 0);
            vtss_macsec_debug_print_field_dbg(pr, "FLOW_CONF_PROTECT", value, val2, 31, 1, 0);
            vtss_macsec_debug_print_field_dbg(pr, "FLOW_VALIDATE_FRAMES", value, val2, 19, 2, 0);

            CSR_RD(port_no, VTSS_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_MISC_CONTROL, &val2);
            val2 = VTSS_X_MACSEC_INGR_FRAME_MATCHING_HANDLING_CTRL_MISC_CONTROL_VALIDATE_FRAMES(val2);
            pr("GLOBAL_VALIDATE_FRAMES:%s\n", val2 == 0 ? "DISABLED" : val2 == 1 ? "CHECK" : "STRICT");

            for (i = 0; i < 4; i += 2) {
                VTSS_RC(csr_rd(vtss_state, port_no, 0x1f, 1, MACSEC_INGR + 0x1f80 + i, &value));
                VTSS_RC(csr_rd(vtss_state, port_no, 0x1f, 1, MACSEC_EGR + 0x1f80 + i, &val2));
                pr("SA %u\n", i / 2);
                vtss_macsec_debug_print_field_dbg(pr, " MAC_SMAC_MATCH_5_to_0", value, val2, 0, 6, 1);
                vtss_macsec_debug_print_field_dbg(pr, " MAC_DMAC_MATCH_5_to_0", value, val2, 6, 6, 1);
                vtss_macsec_debug_print_field_dbg(pr, " MAC_ETYPE_MATCH", value, val2, 12, 1, 0);
                vtss_macsec_debug_print_field_dbg(pr, " VLAN_VLD_MATCH", value, val2, 13, 1, 0);
                vtss_macsec_debug_print_field_dbg(pr, " QINQ_FOUND_MATCH", value, val2, 14, 1, 0);
                vtss_macsec_debug_print_field_dbg(pr, " STAG_VLD_MATCH", value, val2, 15, 1, 0);
                vtss_macsec_debug_print_field_dbg(pr, " QTAG_VLD_MATCH", value, val2, 16, 1, 0);
                vtss_macsec_debug_print_field_dbg(pr, " QTAG_UP_MATCH", value, val2, 17, 1, 0);
                vtss_macsec_debug_print_field_dbg(pr, " VLAN_ID_MATCH", value, val2, 18, 1, 0);
                vtss_macsec_debug_print_field_dbg(pr, " SOURCE_PORT_MATCH", value, val2, 19, 1, 0);
                vtss_macsec_debug_print_field_dbg(pr, " CTRL_PACKET_MATCH", value, val2, 20, 1, 0);
                vtss_macsec_debug_print_field_dbg(pr, " MACSEC_TAG_MATCH", value, val2, 21, 1, 0);
                vtss_macsec_debug_print_field_dbg(pr, " MACSEC_SCI_MATCH", value, val2, 23, 1, 0);
                vtss_macsec_debug_print_field_dbg(pr, " TCI_AN_MATCH", value, val2, 24, 8, 1);
            }
            for (i = 0; i < 4; i += 2) {
                VTSS_RC(csr_rd(vtss_state, port_no, 0x1f, 1, MACSEC_INGR + 0x1f81 + i, &value));
                VTSS_RC(csr_rd(vtss_state, port_no, 0x1f, 1, MACSEC_EGR + 0x1f81 + i, &val2));
                pr("SA %u\n", i / 2);
                vtss_macsec_debug_print_field_dbg(pr, " VLAN_UP_INNER_MATCH", value, val2, 0, 1, 0);
                vtss_macsec_debug_print_field_dbg(pr, " VLAN_ID_INNER_MATCH", value, val2, 1, 1, 0);
                vtss_macsec_debug_print_field_dbg(pr, " HDR_2B_16B_MATCH", value, val2, 2, 1, 0);
            }

            for (i = 0; i < 2; i++) {
                pr("SA %u:\n", i);
                CSR_RD(port_no, VTSS_MACSEC_EGR_SA_MATCH_PARAMS_SAM_MISC_MATCH(i), &val2);
                pr(" VTSS_MACSEC_EGR_SA_MATCH_PARAMS_SAM_MISC_MATCH 0x%x\n", val2);

                CSR_RD(port_no, VTSS_MACSEC_EGR_SA_MATCH_PARAMS_SAM_MASK(i), &val2);
                pr(" VTSS_MACSEC_EGR_SA_MATCH_PARAMS_SAM_MASK 0x%x\n", val2);

                CSR_RD(port_no, VTSS_MACSEC_EGR_SA_MATCH_PARAMS_SAM_MAC_SA_MATCH_HI(i), &val2);
                pr(" VTSS_MACSEC_EGR_SA_MATCH_PARAMS_SAM_MAC_SA_MATCH_HI 0x%x\n", val2);

            }
        }
    }
    return VTSS_RC_OK;
}

#endif /*VTSS_FEATURE_MACSEC */


