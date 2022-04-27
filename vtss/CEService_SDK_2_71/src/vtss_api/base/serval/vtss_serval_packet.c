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

#define VTSS_TRACE_GROUP VTSS_TRACE_GROUP_PACKET
#include "vtss_serval_cil.h"

// Avoid Lint Warning 572: Excessive shift value (precision 1 shifted right by 2), which occurs
// in this file because (t) - VTSS_IO_ORIGIN1_OFFSET == 0 for t = VTSS_TO_CFG (i.e. ICPU_CFG), and 0 >> 2 gives a lint warning.
/*lint --e{572} */
#if defined(VTSS_ARCH_SERVAL)

/* - CIL functions ------------------------------------------------- */

#if defined(VTSS_FEATURE_AFI_SWC)
/* ================================================================= *
 *  AFI - Automatic Frame Injector
 * ================================================================= */

/*
 * srvl_afi_cancel()
 * Cancels a frame by writing to the AFI.
 */
static vtss_rc srvl_afi_cancel(vtss_state_t *vtss_state, vtss_afi_id_t afi_id)
{
    u32 val;

    // Do cancel frame
    SRVL_WRM(VTSS_QSYS_TIMED_FRAME_CFG_TFRM_MISC,
             VTSS_F_QSYS_TIMED_FRAME_CFG_TFRM_MISC_TIMED_CANCEL_SLOT(afi_id) | VTSS_F_QSYS_TIMED_FRAME_CFG_TFRM_MISC_TIMED_CANCEL_1SHOT,
             VTSS_M_QSYS_TIMED_FRAME_CFG_TFRM_MISC_TIMED_CANCEL_SLOT         | VTSS_F_QSYS_TIMED_FRAME_CFG_TFRM_MISC_TIMED_CANCEL_1SHOT);

    // Wait for the one-shot bit to clear.
    do {
        SRVL_RD(VTSS_QSYS_TIMED_FRAME_CFG_TFRM_MISC, &val);
    } while (val & VTSS_F_QSYS_TIMED_FRAME_CFG_TFRM_MISC_TIMED_CANCEL_1SHOT);

    return VTSS_RC_OK;
}

/*
 * srvl_afi_pause_resume()
 * Called internally on link-up/down events to resume/pause AFI frames.
 */
vtss_rc srvl_afi_pause_resume(vtss_state_t *vtss_state, vtss_port_no_t port_no, BOOL resume)
{
    vtss_afi_id_t afi_id;

#if !VTSS_OPT_FDMA
    if (resume) {
        // Resuming paused AFI frames is only possible when using the FDMA driver.
        // If using an external CPU, that one will have to re-inject the frame using
        // the AFI ID originally obtained with a call to vtss_afi_alloc().
        return VTSS_RC_OK; // So here, we just return VTSS_RC_OK to keep the caller happy.
    }
#endif

    // Search through all AFI slots to see if we send frames to #port_no
    // through this slot.
    for (afi_id = 0; afi_id < VTSS_ARRSZ(vtss_state->afi_slots); afi_id++) {
        vtss_afi_slot_conf_t *slot_conf = &vtss_state->afi_slots[afi_id];

        if (resume && slot_conf->state == VTSS_AFI_SLOT_STATE_PAUSED  && slot_conf->port_no == port_no) {
            // Resuming a paused frame. Only possible with FDMA driver.
#if VTSS_OPT_FDMA
            slot_conf->state = VTSS_AFI_SLOT_STATE_ENABLED;
            VTSS_RC(vtss_state->fdma_state.fdma_func.fdma_afi_pause_resume(vtss_state, afi_id, TRUE));
#endif
        } else if (!resume && slot_conf->state == VTSS_AFI_SLOT_STATE_ENABLED && slot_conf->port_no == port_no) {
            // Pausing an enabled frame. This will remove the frame from the switch core, but keep the software state.
            slot_conf->state = VTSS_AFI_SLOT_STATE_PAUSED;

            VTSS_RC(srvl_afi_cancel(vtss_state, afi_id));

#if VTSS_OPT_FDMA
            // Also notify the FDMA driver about this.
            VTSS_RC(vtss_state->fdma_state.fdma_func.fdma_afi_pause_resume(vtss_state, afi_id, FALSE));
#endif
        }
    }

    return VTSS_RC_OK;
}

/*
 * srvl_afi_alloc()
 * Attempts to find a suitable AFI timer and AFI slot for a given frame to be
 * periodically injected.
 */
static vtss_rc srvl_afi_alloc(vtss_state_t *vtss_state, vtss_afi_frm_dscr_t *const dscr, vtss_afi_id_t *const id)
{
    u32                   timer, slot, min_slot, max_slot;
    u64                   ticks;
    vtss_afi_timer_conf_t *timer_conf;
    vtss_afi_slot_conf_t  *slot_conf;

#define VTSS_QSYS_TIMER_UNIT_PS 198400LLU        /* Timer base period in picoseconds */
#define VTSS_PS_PER_SEC         1000000000000LLU /* Picoseconds per second */

    // Figure out the closest, smaller fps that we can obtain with the AFI at hand.
    ticks = VTSS_DIV64(VTSS_PS_PER_SEC, ((u64)dscr->fps * VTSS_QSYS_TIMER_UNIT_PS));
    if (ticks == 0) {
        VTSS_E("Got ticks == 0 with fps == %u", dscr->fps);
        return VTSS_RC_ERROR;
    }

    dscr->actual_fps = VTSS_DIV64(VTSS_PS_PER_SEC, (ticks * VTSS_QSYS_TIMER_UNIT_PS));

    // Since we are truncating in the ticks computation, ticks will always be <= than had
    // we used floating point. This means that dscr->actual_fps will always be >= dscr->fps.
    if (dscr->actual_fps < dscr->fps) {
        VTSS_E("Error: Assumption didn't hold %u/%u\n", dscr->actual_fps, dscr->fps);
        return VTSS_RC_ERROR;
    }

    if (dscr->actual_fps > dscr->fps) {
        // Try with a higher tick count, which will give a lower rate.
        // We have promised (see public signature for dscr) that the actual
        // fps will be less than or equal to the desired fps.
        ticks++;
        dscr->actual_fps = VTSS_DIV64(VTSS_PS_PER_SEC, (ticks * VTSS_QSYS_TIMER_UNIT_PS));
    }

    if (ticks <= 0 || ticks > 0xFFFFFFFFULL) {
        VTSS_E("Unable to fulfil user request for fps = %u", dscr->fps);
        return VTSS_RC_ERROR;
    }

    // If we get here, the user wants to add a new flow. Find a suitable timer.
    for (timer = 0; timer < VTSS_ARRSZ(vtss_state->afi_timers); timer++) {
        // First see if we can reuse one that is already in use.
        timer_conf = &vtss_state->afi_timers[timer];

        if (timer_conf->ref_cnt > 0 && timer_conf->fps == dscr->actual_fps)  {
            // Found a suitable timer.
            break;
        }
    }

    if (timer == VTSS_ARRSZ(vtss_state->afi_timers)) {
        // Didn't find a suitable already-configured timer. See if there are any free timers.
        for (timer = 0; timer < VTSS_ARRSZ(vtss_state->afi_timers); timer++) {

            if (vtss_state->afi_timers[timer].ref_cnt == 0)  {
                // Found a suitable timer
                break;
            }
        }
    }

    if (timer == VTSS_ARRSZ(vtss_state->afi_timers)) {
        // No free timers.
        VTSS_E("Unable to find a usable AFI timer (fps = %u)", dscr->actual_fps);
        return VTSS_RC_ERROR;
    }

    // Find a free slot. The first 16 slots are reserved for frames faster than 60 us.
    if (dscr->actual_fps > 16666) {
        min_slot = 0;
        max_slot = 15;
    } else {
        min_slot = 16;
        max_slot = VTSS_ARRSZ(vtss_state->afi_slots) - 1;
    }

    for (slot = min_slot; slot <= max_slot; slot++) {
        if (vtss_state->afi_slots[slot].state == VTSS_AFI_SLOT_STATE_FREE) {
            break;
        }
    }

    if (slot == max_slot + 1) {
        // Out of slots.
        VTSS_E("Unable to find a usable AFI slot (fps = %u)", dscr->actual_fps);
        return VTSS_RC_ERROR;
    }

    // Unfortunately, the timers aren't defined as a replication (due to distinct defaults),
    // so we have to define our own address.
    #define VTSS_QSYS_TIMED_FRAME_CFG_TFRM_TIMER_CFG(x) VTSS_IOREG(VTSS_TO_QSYS,0x56c6 + (x))

    timer_conf = &vtss_state->afi_timers[timer];
    slot_conf  = &vtss_state->afi_slots[slot];

    // If we get here, we both have a valid timer and a valid slot.
    if (timer_conf->ref_cnt == 0) {
        SRVL_WR(VTSS_QSYS_TIMED_FRAME_CFG_TFRM_TIMER_CFG(timer), ticks);
    }

    // Update the S/W state
    timer_conf->ref_cnt++;
    timer_conf->fps      = dscr->actual_fps;
    slot_conf->timer_idx = timer;
    slot_conf->state     = VTSS_AFI_SLOT_STATE_RESERVED; // Only reserved. We don't know the destination port yet.

    *id = slot;
    return VTSS_RC_OK;

#undef VTSS_QSYS_TIMER_UNIT_PS
#undef VTSS_PS_PER_SEC
}

/*
 * srvl_afi_free()
 * Cancels a periodically injected frame and frees up the
 * resources allocated for it.
 */
static vtss_rc srvl_afi_free(vtss_state_t *vtss_state, vtss_afi_id_t id)
{
    vtss_afi_slot_conf_t *slot_conf;

    if (id >= VTSS_ARRSZ(vtss_state->afi_slots)) {
        VTSS_E("Invalid AFI ID (%u)", id);
        return VTSS_RC_ERROR;
    }

    slot_conf = &vtss_state->afi_slots[id];

    if (slot_conf->state != VTSS_AFI_SLOT_STATE_FREE) {

        // Ask the switch core to stop it. It doesn't harm to stop
        // a slot that is just reserved and not active.
        VTSS_RC(srvl_afi_cancel(vtss_state, id));

        slot_conf->state = VTSS_AFI_SLOT_STATE_FREE;

        // One less frame referencing this timer.
        vtss_state->afi_timers[slot_conf->timer_idx].ref_cnt--;
    }

    return VTSS_RC_OK;
}
#endif /* defined(VTSS_FEATURE_AFI_SWC) */

/* ================================================================= *
 *  NPI
 * ================================================================= */

static vtss_rc srvl_npi_mask_set(vtss_state_t *vtss_state)
{
    vtss_packet_rx_conf_t *conf = &vtss_state->packet.rx_conf;
    vtss_port_no_t        port_no = vtss_state->packet.npi_conf.port_no;
    u32                   val = 0, qmask, i;

    if (port_no != VTSS_PORT_NO_NONE) {
        for (qmask = i = 0; i < vtss_state->packet.rx_queue_count; i++) {
            if (conf->queue[i].npi.enable) {
                qmask |= VTSS_BIT(i); /* NPI redirect */
            }
        }
        val = VTSS_F_QSYS_SYSTEM_EXT_CPU_CFG_EXT_CPU_PORT(VTSS_CHIP_PORT(port_no)) |
            VTSS_F_QSYS_SYSTEM_EXT_CPU_CFG_EXT_CPUQ_MSK(qmask);
    }
    SRVL_WR(VTSS_QSYS_SYSTEM_EXT_CPU_CFG, val);

    return VTSS_RC_OK;
}

static vtss_rc srvl_npi_update(vtss_state_t *vtss_state)
{
    vtss_npi_conf_t *conf = &vtss_state->packet.npi_conf;

    VTSS_RC(srvl_npi_mask_set(vtss_state));

    if (conf->port_no != VTSS_PORT_NO_NONE) {
        u32 value, port = VTSS_CHIP_PORT(conf->port_no);

        /* Control IFH insertion and  parsing */
        value = (conf->enable ?
                 VTSS_F_SYS_SYSTEM_PORT_MODE_INCL_INJ_HDR(3) | VTSS_F_SYS_SYSTEM_PORT_MODE_INCL_XTR_HDR(3) : 0);
        SRVL_WRM(VTSS_SYS_SYSTEM_PORT_MODE(port), value,
                 VTSS_M_SYS_SYSTEM_PORT_MODE_INCL_INJ_HDR | VTSS_M_SYS_SYSTEM_PORT_MODE_INCL_XTR_HDR);
    }

    return VTSS_RC_OK;
}

static vtss_rc srvl_npi_conf_set(vtss_state_t *vtss_state, const vtss_npi_conf_t *const new)
{
    vtss_npi_conf_t *conf = &vtss_state->packet.npi_conf;

    /* Disable current NPI port */
    conf->enable = FALSE;
    VTSS_RC(srvl_npi_update(vtss_state));

    /* Set new NPI port */
    *conf = *new;
    VTSS_RC(srvl_npi_update(vtss_state));

    return vtss_cmn_vlan_update_all(vtss_state);
}

static vtss_rc srvl_dma_conf_set(vtss_state_t *vtss_state, const vtss_packet_dma_conf_t *const new)
{
    u32 qmask;
    int i, mode;

    for (qmask = 0, i = 0; i < VTSS_QUEUE_END; i++) {
        if (new->dma_enable[i]) {
            qmask |= VTSS_BIT(i);
        }
    }

    VTSS_I("%sabling DMA, mask %08x", qmask ? "En" : "Dis", qmask);

    if (qmask == 0) {
        SRVL_WRM(VTSS_QSYS_SYSTEM_CPU_GROUP_MAP, 
                 VTSS_F_QSYS_SYSTEM_CPU_GROUP_MAP_CPU_GROUP_MAP(0), /* All queues -> GRP0 (NORMAL) */
                 VTSS_M_QSYS_SYSTEM_CPU_GROUP_MAP_CPU_GROUP_MAP);
        SRVL_WRM_CLR(VTSS_ICPU_CFG_MANUAL_XTRINJ_MANUAL_CFG, 
                     VTSS_F_ICPU_CFG_MANUAL_XTRINJ_MANUAL_CFG_INJ_ENA|VTSS_F_ICPU_CFG_MANUAL_XTRINJ_MANUAL_CFG_XTR_ENA);
    }

    mode = (qmask ? 2 : 1);       /* DMA / Register */
    i = VTSS_PACKET_TX_GRP_CNT-1; /* Only setup higest group */
    SRVL_WRM(VTSS_DEVCPU_QS_INJ_INJ_GRP_CFG(i), VTSS_F_DEVCPU_QS_INJ_INJ_GRP_CFG_MODE(mode), VTSS_M_DEVCPU_QS_INJ_INJ_GRP_CFG_MODE);
    i = VTSS_PACKET_RX_GRP_CNT-1; /* Only setup higest group */
    SRVL_WRM(VTSS_DEVCPU_QS_XTR_XTR_GRP_CFG(i), VTSS_F_DEVCPU_QS_XTR_XTR_GRP_CFG_MODE(mode), VTSS_M_DEVCPU_QS_XTR_XTR_GRP_CFG_MODE);

    if (qmask) {
        SRVL_WRM(VTSS_QSYS_SYSTEM_CPU_GROUP_MAP, 
                 VTSS_F_QSYS_SYSTEM_CPU_GROUP_MAP_CPU_GROUP_MAP(qmask), /* queues -> GRP1 (FDMA) */
                 VTSS_M_QSYS_SYSTEM_CPU_GROUP_MAP_CPU_GROUP_MAP);
        SRVL_WRM_SET(VTSS_ICPU_CFG_MANUAL_XTRINJ_MANUAL_CFG, 
                     VTSS_F_ICPU_CFG_MANUAL_XTRINJ_MANUAL_CFG_INJ_ENA|VTSS_F_ICPU_CFG_MANUAL_XTRINJ_MANUAL_CFG_XTR_ENA);
    }

    /* Update config */
    vtss_state->packet.dma_conf = *new;

    return VTSS_RC_OK;
}

static vtss_rc srvl_dma_offset(struct vtss_state_s *vtss_state, BOOL extract, u32 *const offset)
{
    if (extract) {
        *offset = VTSS_ICPU_CFG_MANUAL_XTRINJ_MANUAL_XTR(4096-1); /* Last word is status/control */
    } else {
        *offset = VTSS_ICPU_CFG_MANUAL_XTRINJ_MANUAL_INJ(0);      /* First word is status/control */
    }
    return VTSS_RC_OK;
}

static u32 srvl_cpu_fwd_mask_get(vtss_packet_reg_type_t type, BOOL redir, u32 i)
{
    u32 mask;

    /* Map NORMAL to CPU_ONLY/FORWARD */
    if (type == VTSS_PACKET_REG_NORMAL) {
        type = (redir ? VTSS_PACKET_REG_CPU_ONLY : VTSS_PACKET_REG_FORWARD);
    }

    if (type == VTSS_PACKET_REG_CPU_ONLY) {
         /* Set REDIR bit */
        mask = VTSS_BIT(i);
    } else if (type == VTSS_PACKET_REG_DISCARD) {
        /* Set DROP bit */
        mask = VTSS_BIT(i + 16);
    } else if (type == VTSS_PACKET_REG_CPU_COPY) {
        /* Set REDIR and DROP bits */
        mask = (VTSS_BIT(i) | VTSS_BIT(i + 16));
    } else {
        /* No bits set for VTSS_PACKET_REG_FORWARD */
        mask = 0;
    }
    return mask;
}

static vtss_rc srvl_rx_conf_set(vtss_state_t *vtss_state)
{
    vtss_packet_rx_conf_t      *conf = &vtss_state->packet.rx_conf;
    vtss_packet_rx_reg_t       *reg = &conf->reg;
    vtss_packet_rx_queue_map_t *map = &conf->map;
    u32                        queue, i, value, port, bpdu, garp;
    vtss_port_no_t             port_no;
    vtss_packet_rx_port_conf_t *port_conf;

    /* Each CPU queue gets reserved extraction buffer space. No sharing at port or buffer level */
    for (queue = 0; queue < vtss_state->packet.rx_queue_count; queue++) {
        SRVL_WR(VTSS_QSYS_RES_CTRL_RES_CFG(512 /* egress */ + VTSS_CHIP_PORT_CPU * VTSS_PRIOS + queue), conf->queue[queue].size / SRVL_BUFFER_CELL_SZ);
    }
    SRVL_WR(VTSS_QSYS_RES_CTRL_RES_CFG(512 /* egress */ + 224 /* per-port reservation */ + VTSS_CHIP_PORT_CPU), 0); /* No extra shared space at port level */

    /* Rx IPMC registration */
    value =
        (reg->ipmc_ctrl_cpu_copy ? VTSS_F_ANA_PORT_CPU_FWD_CFG_CPU_IPMC_CTRL_COPY_ENA  : 0) |
        (reg->igmp_cpu_only      ? VTSS_F_ANA_PORT_CPU_FWD_CFG_CPU_IGMP_REDIR_ENA      : 0) |
        (reg->mld_cpu_only       ? VTSS_F_ANA_PORT_CPU_FWD_CFG_CPU_MLD_REDIR_ENA       : 0);
    for (port = 0; port < VTSS_CHIP_PORTS; port++) {
        SRVL_WRM(VTSS_ANA_PORT_CPU_FWD_CFG(port), value,
                 VTSS_F_ANA_PORT_CPU_FWD_CFG_CPU_IPMC_CTRL_COPY_ENA |
                 VTSS_F_ANA_PORT_CPU_FWD_CFG_CPU_IGMP_REDIR_ENA |
                 VTSS_F_ANA_PORT_CPU_FWD_CFG_CPU_MLD_REDIR_ENA);
    }

    /* Rx BPDU and GARP registrations */
    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
        port = VTSS_CHIP_PORT(port_no);
        port_conf = &vtss_state->packet.rx_port_conf[port_no];
        for (i = 0, bpdu = 0, garp = 0; i < 16; i++) {
            bpdu |= srvl_cpu_fwd_mask_get(port_conf->bpdu_reg[i], reg->bpdu_cpu_only, i);
            garp |= srvl_cpu_fwd_mask_get(port_conf->garp_reg[i], reg->garp_cpu_only[i], i);
        }
        SRVL_WR(VTSS_ANA_PORT_CPU_FWD_BPDU_CFG(port), bpdu);
        SRVL_WR(VTSS_ANA_PORT_CPU_FWD_GARP_CFG(port), garp);
    }

    /* Fixme - chipset has more queues of classification the API expose */
    value =
        VTSS_F_ANA_COMMON_CPUQ_CFG_CPUQ_SFLOW(map->sflow_queue != VTSS_PACKET_RX_QUEUE_NONE ? map->sflow_queue - VTSS_PACKET_RX_QUEUE_START : VTSS_PACKET_RX_QUEUE_START) |
        VTSS_F_ANA_COMMON_CPUQ_CFG_CPUQ_MIRROR(map->mac_vid_queue-VTSS_PACKET_RX_QUEUE_START) |
        VTSS_F_ANA_COMMON_CPUQ_CFG_CPUQ_LRN(map->learn_queue-VTSS_PACKET_RX_QUEUE_START) |
        VTSS_F_ANA_COMMON_CPUQ_CFG_CPUQ_MAC_COPY(map->mac_vid_queue-VTSS_PACKET_RX_QUEUE_START) |
        VTSS_F_ANA_COMMON_CPUQ_CFG_CPUQ_SRC_COPY(map->mac_vid_queue-VTSS_PACKET_RX_QUEUE_START) |
        VTSS_F_ANA_COMMON_CPUQ_CFG_CPUQ_LOCKED_PORTMOVE(map->mac_vid_queue-VTSS_PACKET_RX_QUEUE_START) |
        VTSS_F_ANA_COMMON_CPUQ_CFG_CPUQ_ALLBRIDGE(map->bpdu_queue-VTSS_PACKET_RX_QUEUE_START) |
        VTSS_F_ANA_COMMON_CPUQ_CFG_CPUQ_IPMC_CTRL(map->ipmc_ctrl_queue-VTSS_PACKET_RX_QUEUE_START) |
        VTSS_F_ANA_COMMON_CPUQ_CFG_CPUQ_IGMP(map->igmp_queue-VTSS_PACKET_RX_QUEUE_START) |
        VTSS_F_ANA_COMMON_CPUQ_CFG_CPUQ_MLD(map->igmp_queue-VTSS_PACKET_RX_QUEUE_START);
    SRVL_WR(VTSS_ANA_COMMON_CPUQ_CFG, value);

    /* Setup each of the BPDU, GARP and CCM 'address' extraction queues */
    for (i = 0; i < 16; i++) {
        value =
            VTSS_F_ANA_COMMON_CPUQ_8021_CFG_CPUQ_BPDU_VAL(map->bpdu_queue-VTSS_PACKET_RX_QUEUE_START) |
            VTSS_F_ANA_COMMON_CPUQ_8021_CFG_CPUQ_GARP_VAL(map->garp_queue-VTSS_PACKET_RX_QUEUE_START) |
            VTSS_F_ANA_COMMON_CPUQ_8021_CFG_CPUQ_CCM_VAL(VTSS_PACKET_RX_QUEUE_START); /* Fixme */
        SRVL_WR(VTSS_ANA_COMMON_CPUQ_8021_CFG(i), value);
    }

    /* Configure Rx Queue #i to map to an Rx group. */
    for (i = 0; i < vtss_state->packet.rx_queue_count; i++) {
        if (conf->grp_map[i]) {
            VTSS_E("Attempt to redirect queue %d - use vtss_dma_conf_set instead()", i);
        }
    }

#if defined(VTSS_FEATURE_NPI)
    VTSS_RC(srvl_npi_mask_set(vtss_state));
#endif /* VTSS_FEATURE_NPI */

    return VTSS_RC_OK;
}

#ifdef VTSS_OS_BIG_ENDIAN
#define XTR_EOF_0          0x80000000U
#define XTR_EOF_1          0x80000001U
#define XTR_EOF_2          0x80000002U
#define XTR_EOF_3          0x80000003U
#define XTR_PRUNED         0x80000004U
#define XTR_ABORT          0x80000005U
#define XTR_ESCAPE         0x80000006U
#define XTR_NOT_READY      0x80000007U
#define XTR_VALID_BYTES(x) (4 - ((x) & 3))
#else
#define XTR_EOF_0          0x00000080U
#define XTR_EOF_1          0x01000080U
#define XTR_EOF_2          0x02000080U
#define XTR_EOF_3          0x03000080U
#define XTR_PRUNED         0x04000080U
#define XTR_ABORT          0x05000080U
#define XTR_ESCAPE         0x06000080U
#define XTR_NOT_READY      0x07000080U
#define XTR_VALID_BYTES(x) (4 - (((x) >> 24) & 3))
#endif

static vtss_rc srvl_rx_frame_discard_grp(vtss_state_t *vtss_state, const vtss_packet_rx_grp_t xtr_grp)
{
    BOOL done = FALSE;

    while (!done) {
        u32 val;
        SRVL_RD(VTSS_DEVCPU_QS_XTR_XTR_RD(xtr_grp), &val);
        switch(val) {
        case XTR_ABORT:
        case XTR_PRUNED:
        case XTR_EOF_3:
        case XTR_EOF_2:
        case XTR_EOF_1:
        case XTR_EOF_0:
            SRVL_RD(VTSS_DEVCPU_QS_XTR_XTR_RD(xtr_grp), &val); /* Last data */
            done = TRUE;        /* Last 1-4 bytes */
            break;
        case XTR_ESCAPE:
            SRVL_RD(VTSS_DEVCPU_QS_XTR_XTR_RD(xtr_grp), &val); /* Escaped data */
            break;
        case XTR_NOT_READY:
        default:
          break;
        }
    }
    return VTSS_RC_OK;
}

static vtss_rc srvl_rx_frame_discard(vtss_state_t *vtss_state, const vtss_packet_rx_queue_t queue_no)
{
    vtss_packet_rx_grp_t xtr_grp = vtss_state->packet.rx_conf.grp_map[queue_no];
    return srvl_rx_frame_discard_grp(vtss_state, xtr_grp);
}

/**
 * Return values:
 *  0 = Data OK.
 *  1 = EOF reached. Data OK. bytes_valid indicates the number of valid bytes in last word ([1; 4]).
 *  2 = Error. No data from queue system.
 */
static int srvl_rx_frame_word(vtss_state_t *vtss_state, vtss_packet_rx_grp_t grp, BOOL first_word, u32 *rval, u32 *bytes_valid)
{
    u32 val;

    SRVL_RD(VTSS_DEVCPU_QS_XTR_XTR_RD(grp), &val);

    if (val == XTR_NOT_READY) {
        /** XTR_NOT_READY means two different things depending on whether this is the first
         * word read of a frame or after at least one word has been read.
         * When the first word, the group is empty, and we return an error.
         * Otherwise we have to wait for the FIFO to have received some more data. */
        if (first_word) {
            return 2; /* Error */
        }
        do {
            SRVL_RD(VTSS_DEVCPU_QS_XTR_XTR_RD(grp), &val);
        } while (val == XTR_NOT_READY);
    }

    switch(val) {
    case XTR_ABORT:
        /* No accompanying data. */
        VTSS_E("Aborted frame");
        return 2; /* Error */
    case XTR_EOF_0:
    case XTR_EOF_1:
    case XTR_EOF_2:
    case XTR_EOF_3:
    case XTR_PRUNED:
        *bytes_valid = XTR_VALID_BYTES(val);
        SRVL_RD(VTSS_DEVCPU_QS_XTR_XTR_RD(grp), &val);
        if (val == XTR_ESCAPE) {
            SRVL_RD(VTSS_DEVCPU_QS_XTR_XTR_RD(grp), rval);
        } else {
            *rval = val;
        }
        return 1; /* EOF */
    case XTR_ESCAPE:
        SRVL_RD(VTSS_DEVCPU_QS_XTR_XTR_RD(grp), rval);
        *bytes_valid = 4;
        return 0;
    default:
        *rval = val;
        *bytes_valid = 4;
        return 0;
    }
}

static vtss_rc srvl_rx_frame_get_internal(vtss_state_t           *vtss_state,
                                          vtss_packet_rx_grp_t   grp,
                                          u32                    *const ifh,
                                          u8                     *const frame,
                                          const u32              buf_length,
                                          u32                    *frm_length) /* Including FCS */
{
    u32     i, val, bytes_got, bytes_valid, buf_len = buf_length;
    BOOL    done = 0;
    u8      *buf;
    int     result;

    *frm_length = bytes_got = 0;

    /* Read IFH. It consists of 4 words */
    for (i = 0; i < 4; i++) {
        if (srvl_rx_frame_word(vtss_state, grp, TRUE, &val, &bytes_valid) != 0) {
            /* We accept neither EOF nor ERROR when reading the IFH */
            return VTSS_RC_ERROR;
        }
        ifh[i] = val;
    }

    buf = frame;

    /* Read the rest of the frame */
    while (!done && buf_len >= 4) {
        result = srvl_rx_frame_word(vtss_state, grp, FALSE, &val, &bytes_valid);
        if (result == 2) {
            // Error.
            return VTSS_RC_ERROR;
        }
        // Store the data.
        bytes_got += bytes_valid;
#ifdef VTSS_OS_BIG_ENDIAN
        *buf++ = (u8)(val >> 24);
        *buf++ = (u8)(val >> 16);
        *buf++ = (u8)(val >>  8);
        *buf++ = (u8)(val >>  0);
#else
        *buf++ = (u8)(val >>  0);
        *buf++ = (u8)(val >>  8);
        *buf++ = (u8)(val >> 16);
        *buf++ = (u8)(val >> 24);
#endif
        buf_len -= bytes_valid;
        done = result == 1;
    }

    /* Updated received byte count */
    *frm_length = bytes_got;

    if (!done) {
        /* Buffer overrun. Skip remainder of frame */
        (void)srvl_rx_frame_discard_grp(vtss_state, grp);
        return VTSS_RC_ERROR;
    }

    if (bytes_got < 60) {
        VTSS_E("short frame, %u bytes read", bytes_got);
        return VTSS_RC_ERROR;
    }

    return VTSS_RC_OK;
}

static vtss_rc srvl_rx_frame_get(vtss_state_t                 *vtss_state,
                                 const vtss_packet_rx_queue_t queue_no,
                                 vtss_packet_rx_header_t      *const header,
                                 u8                           *const frame,
                                 const u32                    length)
{
    vtss_rc              rc;
    vtss_packet_rx_grp_t grp = vtss_state->packet.rx_conf.grp_map[queue_no];
    u32                  val, port;
    u32                  ifh[4];
    vtss_port_no_t       port_no;
    BOOL                 found = FALSE;
    u16                  ethtype;
    u32                  llen, wlen, ifh_reported_len_incl_fcs;
    int                  i;

    /* Check if data is ready for grp */
    SRVL_RD(VTSS_DEVCPU_QS_XTR_XTR_DATA_PRESENT, &val);
    if (!(val & VTSS_F_DEVCPU_QS_XTR_XTR_DATA_PRESENT_DATA_PRESENT(VTSS_BIT(grp)))) {
        return VTSS_RC_INCOMPLETE;
    }

    if ((rc = srvl_rx_frame_get_internal(vtss_state, grp, ifh, frame, length, &header->length)) != VTSS_RC_OK) {
        return rc;
    }
    header->length -= 4; // According to specification, vtss_packet_rx_header_t::length excludes the FCS, but srvl_rx_frame_get_internal() includes it.
    
    /* Decoding assumes host order IFH */
    for (i = 0; i < 4; i++) {
        ifh[i] = VTSS_OS_NTOHL(ifh[i]);
    }

    /* Check actual extracted length vs. the length reported in the IFH */
    wlen = VTSS_EXTRACT_BITFIELD(ifh[1], 72 - 64, 8);
    llen = VTSS_EXTRACT_BITFIELD(ifh[1], 80 - 64, 6);
    ifh_reported_len_incl_fcs = 60 * wlen + llen - 80;
    if (ifh_reported_len_incl_fcs != header->length + 4) {
        VTSS_E("Extracted %u bytes. IFH reported %u bytes", header->length, ifh_reported_len_incl_fcs);
        return VTSS_RC_ERROR;
    }

    /* Note - VLAN tags are *not* stripped on ingress */
    header->tag.vid     = VTSS_EXTRACT_BITFIELD(ifh[3],  0, 12);
    header->tag.cfi     = VTSS_EXTRACT_BITFIELD(ifh[3], 12,  1);
    header->tag.tagprio = VTSS_EXTRACT_BITFIELD(ifh[3], 13,  3);
    header->queue_mask  = VTSS_EXTRACT_BITFIELD(ifh[3], 20,  8);
    header->learn      = (VTSS_EXTRACT_BITFIELD(ifh[3], 28,  2) ? 1 : 0);

    /* Map from chip port to API port */
    port = VTSS_EXTRACT_BITFIELD(ifh[2], 43 - 32, 4);
    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
        if (VTSS_CHIP_PORT(port_no) == port) {
            header->port_no = port_no;
            found = TRUE;
            break;
        }
    }
    if (!found) {
        VTSS_E("Unknown chip port: %u", port);
        return VTSS_RC_ERROR;
    }

    ethtype = (frame[12] << 8) + frame[13];
    header->arrived_tagged = (ethtype == VTSS_ETYPE_TAG_C || ethtype == VTSS_ETYPE_TAG_S); /* Emulated */

    return VTSS_RC_OK;
}

static vtss_rc srvl_rx_frame_get_raw(struct vtss_state_s *vtss_state,
                                     u8                  *const data,
                                     const u32           buflen,
                                     u32                 *const ifhlen,
                                     u32                 *const frmlen)
{
    vtss_rc rc = VTSS_RC_INCOMPLETE;
    u32     val;

    /* Check if data is ready for grp */
    SRVL_RD(VTSS_DEVCPU_QS_XTR_XTR_DATA_PRESENT, &val);
    if (val) {
        u32 ifh[4];
        u32 length;
        vtss_packet_rx_grp_t grp = VTSS_OS_CTZ(val);

        /* Get frame, separate IFH and frame data */
        if ((rc = srvl_rx_frame_get_internal(vtss_state, grp, ifh, data + sizeof(ifh), buflen - sizeof(ifh), &length)) != VTSS_RC_OK) {
            return rc;
        }

        /* IFH is done separately because of alignment needs */
        memcpy(data, ifh, sizeof(ifh));
        *ifhlen = sizeof(ifh);
        *frmlen = length;
        rc = VTSS_RC_OK;
    }
    return rc;
}

static vtss_rc srvl_tx_frame_ifh_vid(vtss_state_t *vtss_state,
                                     const vtss_packet_tx_ifh_t *const ifh,
                                     const u8 *const frame,
                                     const u32 length,
                                     const vtss_vid_t vid)
{
    u32 val, w, count, last;
    const u8 *buf = frame;
    vtss_packet_tx_grp_t grp = 0;

    VTSS_N("ifh: %u, length: %u", ifh->length, length);

    if (ifh->length != 16) {
        return VTSS_RC_ERROR;
    }

    SRVL_RD(VTSS_DEVCPU_QS_INJ_INJ_STATUS, &val);
    if (!(VTSS_X_DEVCPU_QS_INJ_INJ_STATUS_FIFO_RDY(val) & VTSS_BIT(grp))) {
        VTSS_E("Not ready");
        return VTSS_RC_ERROR;
    }

    if (VTSS_X_DEVCPU_QS_INJ_INJ_STATUS_WMARK_REACHED(val) & VTSS_BIT(grp)) {
        VTSS_E("Watermark reached");
        return VTSS_RC_ERROR;
    }

    /* Indicate SOF */
    SRVL_WR(VTSS_DEVCPU_QS_INJ_INJ_CTRL(grp), VTSS_F_DEVCPU_QS_INJ_INJ_CTRL_GAP_SIZE(1) | VTSS_F_DEVCPU_QS_INJ_INJ_CTRL_SOF);

    // Write the IFH to the chip.
    for (w = 0; w < 4; w++) {
        SRVL_WR(VTSS_DEVCPU_QS_INJ_INJ_WR(grp), ifh->ifh[w]);
    }

    /* Write words, round up */
    count = ((length+3) / 4);
    last = length % 4;
    for (w = 0; w < count; w++, buf += 4) {
        if (w == 3 && vid != VTSS_VID_NULL) {
            /* Insert C-tag */
            SRVL_WR(VTSS_DEVCPU_QS_INJ_INJ_WR(grp), VTSS_OS_NTOHL((0x8100U << 16) | vid));
            w++;
        }
#ifdef VTSS_OS_BIG_ENDIAN
        SRVL_WR(VTSS_DEVCPU_QS_INJ_INJ_WR(grp), (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3]);
#else
        SRVL_WR(VTSS_DEVCPU_QS_INJ_INJ_WR(grp), (buf[3] << 24) | (buf[2] << 16) | (buf[1] << 8) | buf[0]);
#endif
    }

    /* Add padding */
    while (w < (60 / 4)) {
        SRVL_WR(VTSS_DEVCPU_QS_INJ_INJ_WR(grp), 0);
        w++;
    }

    /* Indicate EOF and valid bytes in last word */
    SRVL_WR(VTSS_DEVCPU_QS_INJ_INJ_CTRL(grp),
            VTSS_F_DEVCPU_QS_INJ_INJ_CTRL_GAP_SIZE(1) |
            VTSS_F_DEVCPU_QS_INJ_INJ_CTRL_VLD_BYTES(length < 60 ? 0 : last) |
            VTSS_F_DEVCPU_QS_INJ_INJ_CTRL_EOF);

    /* Add dummy CRC */
    SRVL_WR(VTSS_DEVCPU_QS_INJ_INJ_WR(grp), 0);
    w++;

    VTSS_N("wrote %u words, last: %u", w, last);

    return VTSS_RC_OK;
}

static vtss_rc srvl_tx_frame_ifh(vtss_state_t *vtss_state,
                                 const vtss_packet_tx_ifh_t *const ifh,
                                 const u8 *const frame,
                                 const u32 length)
{
    return srvl_tx_frame_ifh_vid(vtss_state, ifh, frame, length, VTSS_VID_NULL);
}


static vtss_rc srvl_rx_hdr_decode(const vtss_state_t          *const state,
                                  const vtss_packet_rx_meta_t *const meta,
                                  const u8                           xtr_hdr[VTSS_PACKET_HDR_SIZE_BYTES],
                                        vtss_packet_rx_info_t *const info)
{
    u64                 ifh[2];
    u32                 sflow_id;
    vtss_phys_port_no_t chip_port;
    u32                 rew_op, rew_op_lsb, rew_val;
    int                 i;
    vtss_trace_group_t  trc_grp = meta->no_wait ? VTSS_TRACE_GROUP_FDMA_IRQ : VTSS_TRACE_GROUP_PACKET;

    VTSS_DG(trc_grp, "IFH:");
    VTSS_DG_HEX(trc_grp, xtr_hdr, 8);

    for (i = 0; i < 2; i++) {
        int offset = 8 * i;
        ifh[i] = ((u64)xtr_hdr[offset + 0] << 56) | ((u64)xtr_hdr[offset + 1] << 48) | ((u64)xtr_hdr[offset + 2] << 40) | ((u64)xtr_hdr[offset + 3] << 32) |
                 ((u64)xtr_hdr[offset + 4] << 24) | ((u64)xtr_hdr[offset + 5] << 16) | ((u64)xtr_hdr[offset + 6] <<  8) | ((u64)xtr_hdr[offset + 7] <<  0);
    }

    memset(info, 0, sizeof(*info));

    info->sw_tstamp = meta->sw_tstamp;
    info->length    = meta->length;
    info->glag_no   = VTSS_GLAG_NO_NONE;

    // Map from chip port to API port
    chip_port = VTSS_EXTRACT_BITFIELD64(ifh[1], 43, 4);
    info->port_no = vtss_cmn_chip_to_logical_port(state, 0, chip_port);
    if (info->port_no == VTSS_PORT_NO_NONE) {
        VTSS_EG(trc_grp, "Unknown chip port: %u", chip_port);
        return VTSS_RC_ERROR;
    }

    info->tag.pcp     = VTSS_EXTRACT_BITFIELD64(ifh[1], 13,  3);
    info->tag.dei     = VTSS_EXTRACT_BITFIELD64(ifh[1], 12,  1);
    info->tag.vid     = VTSS_EXTRACT_BITFIELD64(ifh[1],  0, 12);
    info->xtr_qu_mask = VTSS_EXTRACT_BITFIELD64(ifh[1], 20,  8);
    info->cos         = VTSS_EXTRACT_BITFIELD64(ifh[1], 17,  3);

    VTSS_RC(vtss_cmn_packet_hints_update(state, trc_grp, meta->etype, info));

    info->acl_hit = VTSS_EXTRACT_BITFIELD64(ifh[1], 31, 1);
    if (info->acl_hit) {
        info->acl_idx = VTSS_EXTRACT_BITFIELD64(ifh[1], 37, 6);
    }

    rew_op  = VTSS_EXTRACT_BITFIELD64(ifh[0], 118 - 64,  9);
    rew_val = VTSS_EXTRACT_BITFIELD64(ifh[0],  86 - 64, 32);
    rew_op_lsb = rew_op & 0x7;

    if (rew_op_lsb == 3) {
        // Two-step PTP Tx timestamp in MSbits of the rew_op field
        info->tstamp_id         = rew_op >> 3;
        info->tstamp_id_decoded = TRUE;
    } else if (rew_op_lsb == 4) {
        // REW_VAL contains OAM info that we cannot decode without frame knowledge.
        // As a way out, we just save it in oam_info and let the application decode it itself :-(
        info->oam_info         = rew_val;
        info->oam_info_decoded = TRUE;
    }

    if (rew_op_lsb != 4) {
        // Rx timestamp in rew_val except when REW_OP[2:0] == 4
        info->hw_tstamp         = rew_val;
        info->hw_tstamp_decoded = TRUE;
    }

    // sflow_id:
    // 0-11 : Frame has been sFlow sampled by a Tx sampler on port given by @sflow_id.
    // 12   : Frame has been sFlow sampled by an Rx sampler on port given by @src_port.
    // 13-14: Reserved.
    // 15   : Frame has not been sFlow sampled.
    sflow_id = VTSS_EXTRACT_BITFIELD64(ifh[1], 32, 4);
    if (sflow_id == 12) {
        info->sflow_type    = VTSS_SFLOW_TYPE_RX;
        info->sflow_port_no = info->port_no;
    } else if (sflow_id < 12) {
        info->sflow_type    = VTSS_SFLOW_TYPE_TX;
        info->sflow_port_no = vtss_cmn_chip_to_logical_port(state, 0, sflow_id);
    }

    info->isdx = VTSS_EXTRACT_BITFIELD64(ifh[1], 47, 10);

    return VTSS_RC_OK;
}

/*****************************************************************************/
// srvl_oam_type_to_ifh()
/*****************************************************************************/
static vtss_rc srvl_oam_type_to_ifh(vtss_packet_oam_type_t oam_type, u32 *result)
{
    vtss_rc rc = VTSS_RC_OK;

    switch (oam_type) {
    case VTSS_PACKET_OAM_TYPE_NONE:
        *result = 0;
        break;
    case VTSS_PACKET_OAM_TYPE_CCM:
        *result = 2;
        break;
    case VTSS_PACKET_OAM_TYPE_CCM_LM:
        *result = 3;
        break;
    case VTSS_PACKET_OAM_TYPE_LBM:
        *result = 4;
        break;
    case VTSS_PACKET_OAM_TYPE_LBR:
        *result = 5;
        break;
    case VTSS_PACKET_OAM_TYPE_LMM:
        *result = 6;
        break;
    case VTSS_PACKET_OAM_TYPE_LMR:
        *result = 7;
        break;
    case VTSS_PACKET_OAM_TYPE_DMM:
        *result = 8;
        break;
    case VTSS_PACKET_OAM_TYPE_DMR:
        *result = 9;
        break;
    case VTSS_PACKET_OAM_TYPE_1DM:
        *result = 10;
        break;
    case VTSS_PACKET_OAM_TYPE_LTM:
        *result = 11;
        break;
    case VTSS_PACKET_OAM_TYPE_LTR:
        *result = 12;
        break;
    case VTSS_PACKET_OAM_TYPE_GENERIC:
        *result = 13;
        break;
    default:
        VTSS_E("Invalid OAM type (%d)", oam_type);
        rc = VTSS_RC_ERROR;
        break;
    }
    *result = (*result << 3) | 4;
    return rc;
}

/*****************************************************************************/
// srvl_ptp_action_to_ifh()
/*****************************************************************************/
static vtss_rc srvl_ptp_action_to_ifh(vtss_packet_ptp_action_t ptp_action, u8 ptp_id, u32 *result)
{
    vtss_rc rc = VTSS_RC_OK;

    switch (ptp_action) {
    case VTSS_PACKET_PTP_ACTION_NONE:
        *result = 0;
        break;
    case VTSS_PACKET_PTP_ACTION_ONE_STEP:
        *result = 2; // Residence compensation must be done in S/W, so bit 5 is not set.
        break;
    case VTSS_PACKET_PTP_ACTION_TWO_STEP:
        *result = (3 | (ptp_id << 3)); // Select a PTP timestamp identifier for two-step PTP.
        break;
    case VTSS_PACKET_PTP_ACTION_ORIGIN_TIMESTAMP:
        *result = 5;
        break;
    default:
        VTSS_E("Invalid PTP action (%d)", ptp_action);
        rc = VTSS_RC_ERROR;
        break;
    }
    return rc;
}

/*****************************************************************************/
// srvl_tx_hdr_encode()
/*****************************************************************************/
static vtss_rc srvl_tx_hdr_encode(      vtss_state_t          *const state,
                                  const vtss_packet_tx_info_t *const info,
                                        u8                    *const bin_hdr,
                                        u32                   *const bin_hdr_len)
{
    u64 inj_hdr[2];

    if (bin_hdr == NULL) {
        // Caller wants us to return the number of bytes required to fill
        // in #bin_hdr. We need 16 bytes for the IFH.
        *bin_hdr_len = 16;
        return VTSS_RC_OK;
    } else if (*bin_hdr_len < 16) {
        return VTSS_RC_ERROR;
    }

    *bin_hdr_len = 16;

    inj_hdr[0] = VTSS_ENCODE_BITFIELD64(!info->switch_frm, 127 - 64, 1); // BYPASS
    inj_hdr[1] = 0;

    if (info->switch_frm) {
        if (info->masquerade_port != VTSS_PORT_NO_NONE) {
            u32 chip_port;
            if (info->masquerade_port >= state->port_count) {
                VTSS_E("Invalid masquerade port (%u)", info->masquerade_port);
                return VTSS_RC_ERROR;
            }

            chip_port = VTSS_CHIP_PORT_FROM_STATE(state, info->masquerade_port);
            inj_hdr[0] |= VTSS_ENCODE_BITFIELD64(1,         126 - 64, 1); // Enable masquerading.
            inj_hdr[0] |= VTSS_ENCODE_BITFIELD64(chip_port, 122 - 64, 4); // Masquerade port.
        }
        if (info->tag.tpid) {
            inj_hdr[1] |= VTSS_ENCODE_BITFIELD64(1, 28 - 0, 2); // POP_CNT = 1, i.e. pop one tag, expected inserted by application/FDMA driver.
        }
    } else {
        u32            port_cnt, rew_op = 0, rew_val = 0, pop_cnt = 3 /* Disable rewriter */;
        u64            chip_port_mask;
        vtss_chip_no_t chip_no;
        vtss_port_no_t stack_port_no, port_no;

        VTSS_RC(vtss_cmn_logical_to_chip_port_mask(state, info->dst_port_mask, &chip_port_mask, &chip_no, &stack_port_no, &port_cnt, &port_no));

#ifdef VTSS_FEATURE_MIRROR_CPU
        // Add mirror port if enabled. Mirroring of directed frames must occur through the port mask.
        if (state->l2.mirror_conf.port_no != VTSS_PORT_NO_NONE && state->l2.mirror_cpu_ingress) {
            chip_port_mask |= VTSS_BIT64(VTSS_CHIP_PORT_FROM_STATE(state, state->l2.mirror_conf.port_no));
        }
#endif

        if (info->ptp_action != VTSS_PACKET_PTP_ACTION_NONE) {
            VTSS_RC(srvl_ptp_action_to_ifh(info->ptp_action, info->ptp_id, &rew_op));
            rew_val = info->ptp_timestamp;
            pop_cnt = 0; // Don't disable rewriter.
        } else if (info->oam_type != VTSS_PACKET_OAM_TYPE_NONE) {
            VTSS_RC(srvl_oam_type_to_ifh(info->oam_type, &rew_op));
            pop_cnt = 0; // Don't disable rewriter.
        } else if (info->tag.tpid == 0 && info->tag.vid != VTSS_VID_NULL) {
            // Get the frame classified to info->tag.vid, and rewritten accordingly.
            pop_cnt = 0;
        }

#if defined(VTSS_FEATURE_AFI_SWC)
        if (info->ptp_action == VTSS_PACKET_PTP_ACTION_NONE && info->afi_id != VTSS_AFI_ID_NONE) {
            vtss_afi_slot_conf_t *slot_conf = &state->afi_slots[info->afi_id];
            vtss_inst_t          inst = state; // For VTSS_ENTER() and VTSS_EXIT()

            // The rew_val is re-used for AFI, so we can't periodically transmit a frame
            // if ptp_action != VTSS_PACKET_PTP_ACTION_NONE
            if (port_cnt != 1) {
                VTSS_E("When using AFI, exactly one port must be specified (dst_port_mask = 0x%" PRIu64 ").", info->dst_port_mask);
                return VTSS_RC_ERROR;
            }

            if (info->afi_id >= VTSS_ARRSZ(state->afi_slots)) {
                VTSS_E("Invalid AFI ID (%u)", info->afi_id);
                return VTSS_RC_ERROR;
            }

            VTSS_ENTER();
            if (slot_conf->state != VTSS_AFI_SLOT_STATE_RESERVED) {
                VTSS_EXIT();
                VTSS_E("AFI ID (%u) not allocated", info->afi_id);
                return VTSS_RC_ERROR;
            }

            slot_conf->state   = VTSS_AFI_SLOT_STATE_ENABLED;
            slot_conf->port_no = port_no;
            VTSS_EXIT();

            rew_val = info->afi_id;
            inj_hdr[1] |= VTSS_ENCODE_BITFIELD64(slot_conf->timer_idx + 1, 37 -  0,  4);
        }
#endif

        inj_hdr[0] |= VTSS_ENCODE_BITFIELD64(rew_op,             118 - 64,  9); // REW_OP
        inj_hdr[0] |= VTSS_ENCODE_BITFIELD64(rew_val,             86 - 64, 32); // REW_VAL
        inj_hdr[0] |= VTSS_ENCODE_BITFIELD64(chip_port_mask >> 7, 64 - 64,  4); // Don't send to the CPU port (hence length == 4 and not 6)
        inj_hdr[1] |= VTSS_ENCODE_BITFIELD64(chip_port_mask,      57 -  0,  7);
        if (info->isdx != VTSS_ISDX_NONE) {
            inj_hdr[1] |= VTSS_ENCODE_BITFIELD64(info->isdx,      47 -  0, 10); // ISDX
            if (!info->isdx_dont_use) {
                inj_hdr[1] |= VTSS_ENCODE_BITFIELD64(1,           32 -  0,  1); // Use ISDX for ES0 lookups
            }
        }
        inj_hdr[1] |= VTSS_ENCODE_BITFIELD64(info->dp,            30 -  0,  1); // Drop Precedence
        inj_hdr[1] |= VTSS_ENCODE_BITFIELD64(pop_cnt,             28 -  0,  2); // POP_CNT
        inj_hdr[1] |= VTSS_ENCODE_BITFIELD64(info->cos >= 8 ? 7 : info->cos, 17 - 0, 3); // QOS Class
        inj_hdr[1] |= VTSS_ENCODE_BITFIELD64(info->tag.tpid == 0 || info->tag.tpid == 0x8100 ? 0 : 1, 16 - 0, 1); // TAG_TYPE
        inj_hdr[1] |= VTSS_ENCODE_BITFIELD64(info->tag.pcp,       13 -  0,  3); // PCP
        inj_hdr[1] |= VTSS_ENCODE_BITFIELD64(info->tag.dei,       12 -  0,  1); // DEI
        inj_hdr[1] |= VTSS_ENCODE_BITFIELD64(info->tag.vid,        0 -  0, 16); // VID
    }

    // Time to store it in DDR SDRAM.
    bin_hdr[ 0] = inj_hdr[0] >> 56;
    bin_hdr[ 1] = inj_hdr[0] >> 48;
    bin_hdr[ 2] = inj_hdr[0] >> 40;
    bin_hdr[ 3] = inj_hdr[0] >> 32;
    bin_hdr[ 4] = inj_hdr[0] >> 24;
    bin_hdr[ 5] = inj_hdr[0] >> 16;
    bin_hdr[ 6] = inj_hdr[0] >>  8;
    bin_hdr[ 7] = inj_hdr[0] >>  0;
    bin_hdr[ 8] = inj_hdr[1] >> 56;
    bin_hdr[ 9] = inj_hdr[1] >> 48;
    bin_hdr[10] = inj_hdr[1] >> 40;
    bin_hdr[11] = inj_hdr[1] >> 32;
    bin_hdr[12] = inj_hdr[1] >> 24;
    bin_hdr[13] = inj_hdr[1] >> 16;
    bin_hdr[14] = inj_hdr[1] >>  8;
    bin_hdr[15] = inj_hdr[1] >>  0;

    VTSS_I("IFH:");
    VTSS_I_HEX(&bin_hdr[0], *bin_hdr_len);

    return VTSS_RC_OK;
}

static vtss_rc srvl_tx_frame_port(vtss_state_t *vtss_state,
                                  const vtss_port_no_t  port_no,
                                  const u8              *const frame,
                                  const u32             length,
                                  const vtss_vid_t      vid)
{
    vtss_packet_tx_ifh_t ifh;
    vtss_packet_tx_info_t tx_info;
    vtss_rc rc;

    (void) vtss_packet_tx_info_init(vtss_state, &tx_info);
    tx_info.dst_port_mask = VTSS_BIT64(port_no);

    ifh.length = sizeof(ifh.ifh);
    if ((rc = srvl_tx_hdr_encode(vtss_state, &tx_info, (u8*) ifh.ifh, &ifh.length) != VTSS_RC_OK)) {
        return rc;
    }
    return srvl_tx_frame_ifh_vid(vtss_state, &ifh, frame, length, vid);
}

/* - Debug print --------------------------------------------------- */

#define SRVL_DEBUG_CPU_FWD(pr, addr, i, name) vtss_srvl_debug_reg_inst(vtss_state, pr, VTSS_ANA_PORT_CPU_FWD_##addr, i, "FWD_"name)
#define SRVL_DEBUG_XTR(pr, addr, name) vtss_srvl_debug_reg(vtss_state, pr, VTSS_DEVCPU_QS_XTR_XTR_##addr, "XTR_"name)
#define SRVL_DEBUG_XTR_INST(pr, addr, i, name) vtss_srvl_debug_reg_inst(vtss_state, pr, VTSS_DEVCPU_QS_XTR_XTR_##addr, i, "XTR_"name)
#define SRVL_DEBUG_INJ(pr, addr, name) vtss_srvl_debug_reg(vtss_state, pr, VTSS_DEVCPU_QS_INJ_INJ_##addr, "INJ_"name)
#define SRVL_DEBUG_INJ_INST(pr, addr, i, name) vtss_srvl_debug_reg_inst(vtss_state, pr, VTSS_DEVCPU_QS_INJ_INJ_##addr, i, "INJ_"name)

static vtss_rc srvl_debug_pkt(vtss_state_t *vtss_state,
                              const vtss_debug_printf_t pr,
                              const vtss_debug_info_t   *const info)
{
    u32  i, port;
    char buf[32];

    /* Analyzer CPU forwarding registers */
    for (port = 0; port <= VTSS_CHIP_PORTS; port++) {
        sprintf(buf, "Port %u", port);
        vtss_srvl_debug_reg_header(pr, buf);
        SRVL_DEBUG_CPU_FWD(pr, CFG(port), port, "CFG");
        SRVL_DEBUG_CPU_FWD(pr, BPDU_CFG(port), port, "BPDU_CFG");
        SRVL_DEBUG_CPU_FWD(pr, GARP_CFG(port), port, "GARP_CFG");
        SRVL_DEBUG_CPU_FWD(pr, CCM_CFG(port), port, "CCM_CFG");
        pr("\n");
    }

    /* Analyzer CPU queue mappings */
    vtss_srvl_debug_reg_header(pr, "CPU Queues");
    vtss_srvl_debug_reg(vtss_state, pr, VTSS_ANA_COMMON_CPUQ_CFG, "CPUQ_CFG");
    for (i = 0; i < 16; i++)
        vtss_srvl_debug_reg_inst(vtss_state, pr, VTSS_ANA_COMMON_CPUQ_8021_CFG(i), i, "CPUQ_8021_CFG");
    pr("\n");

    /* Packet extraction registers */
    vtss_srvl_debug_reg_header(pr, "Extraction");
    for (i = 0; i < VTSS_PACKET_RX_GRP_CNT; i++)
        SRVL_DEBUG_XTR_INST(pr, FRM_PRUNING(i), i, "FRM_PRUNING");
    for (i = 0; i < VTSS_PACKET_RX_GRP_CNT; i++)
        SRVL_DEBUG_XTR_INST(pr, GRP_CFG(i), i, "GRP_CFG");
    vtss_srvl_debug_reg(vtss_state, pr, VTSS_QSYS_SYSTEM_CPU_GROUP_MAP, "CPU_GROUP_MAP");
    SRVL_DEBUG_XTR(pr, DATA_PRESENT, "DATA_PRESENT");
    pr("\n");

    /* Packet injection registers */
    vtss_srvl_debug_reg_header(pr, "Injection");
    for (i = 0; i < VTSS_PACKET_TX_GRP_CNT; i++)
        SRVL_DEBUG_INJ_INST(pr, GRP_CFG(i), i, "GRP_CFG");
    for (i = 0; i < VTSS_PACKET_TX_GRP_CNT; i++)
        SRVL_DEBUG_INJ_INST(pr, CTRL(i), i, "CTRL");
    for (i = 0; i < VTSS_PACKET_TX_GRP_CNT; i++)
        SRVL_DEBUG_INJ_INST(pr, ERR(i), i, "ERR");
    SRVL_DEBUG_INJ(pr, STATUS, "STATUS");
    pr("\n");

    vtss_srvl_debug_reg_header(pr, "DMA xtr/inj");
    vtss_srvl_debug_reg(vtss_state, pr, VTSS_ICPU_CFG_MANUAL_XTRINJ_MANUAL_CFG, "XTRINJ_MANUAL_CFG");
    vtss_srvl_debug_reg(vtss_state, pr, VTSS_ICPU_CFG_MANUAL_XTRINJ_MANUAL_INTR_ENA, "XTRINJ_MANUAL_INTR_ENA");
    vtss_srvl_debug_reg(vtss_state, pr, VTSS_ICPU_CFG_MANUAL_XTRINJ_MANUAL_INTR, "XTRINJ_MANUAL_INTR");
    vtss_srvl_debug_reg_inst(vtss_state, pr, VTSS_SYS_SYSTEM_PORT_MODE(VTSS_CHIP_PORT_CPU_0), VTSS_CHIP_PORT_CPU_0, "SYSTEM_PORT_MODE");
    vtss_srvl_debug_reg_inst(vtss_state, pr, VTSS_SYS_SYSTEM_PORT_MODE(VTSS_CHIP_PORT_CPU_1), VTSS_CHIP_PORT_CPU_1, "SYSTEM_PORT_MODE");
    vtss_srvl_debug_reg(vtss_state, pr, VTSS_ICPU_CFG_FDMA_FDMA_EVT_ERR, "FDMA_EVT_ERR");
    pr("\n");

    if (vtss_state->packet.npi_conf.enable) {
        vtss_srvl_debug_reg_header(pr, "NPI");
        vtss_srvl_debug_reg(vtss_state, pr, VTSS_QSYS_SYSTEM_EXT_CPU_CFG, "EXT_CPU_CFG");
        port = VTSS_CHIP_PORT(vtss_state->packet.npi_conf.port_no);
        vtss_srvl_debug_reg_inst(vtss_state, pr, VTSS_SYS_SYSTEM_PORT_MODE(port), port, "SYSTEM_PORT_MODE");
    }

    return VTSS_RC_OK;
}

vtss_rc vtss_srvl_packet_debug_print(vtss_state_t *vtss_state,
                                     const vtss_debug_printf_t pr,
                                     const vtss_debug_info_t   *const info)
{
    return vtss_debug_print_group(VTSS_DEBUG_GROUP_PACKET, srvl_debug_pkt, vtss_state, pr, info);
}

#if defined(VTSS_FEATURE_AFI_SWC)
static vtss_rc srvl_debug_afi(vtss_state_t *vtss_state,
                              const vtss_debug_printf_t pr,
                              const vtss_debug_info_t   *const info)
{
    u32 timer;

    /* AFI Timers */
    vtss_srvl_debug_reg_header(pr, "Timers");
    for (timer = 0; timer < VTSS_ARRSZ(vtss_state->afi_timers); timer++) {
        vtss_srvl_debug_reg_inst(vtss_state, pr, VTSS_QSYS_TIMED_FRAME_CFG_TFRM_TIMER_CFG(timer), timer, "CFG");
    }
    pr("\n");
    return VTSS_RC_OK;
}

vtss_rc vtss_srvl_afi_debug_print(vtss_state_t *vtss_state,
                                  const vtss_debug_printf_t pr,
                                  const vtss_debug_info_t   *const info)
{
    return vtss_debug_print_group(VTSS_DEBUG_GROUP_AFI, srvl_debug_afi, vtss_state, pr, info);
}
#endif /* VTSS_FEATURE_AFI_SWC */

/* - Initialization ------------------------------------------------ */

static vtss_rc srvl_packet_init(vtss_state_t *vtss_state)
{
    u32 i, pcp, dei;

    /* Setup the CPU port as VLAN aware to support switching frames based on tags */
    SRVL_WR(VTSS_ANA_PORT_VLAN_CFG(VTSS_CHIP_PORT_CPU),
            VTSS_F_ANA_PORT_VLAN_CFG_VLAN_AWARE_ENA  |
            VTSS_F_ANA_PORT_VLAN_CFG_VLAN_POP_CNT(1) |
            VTSS_F_ANA_PORT_VLAN_CFG_VLAN_VID(1));

    /* Disable learning (only RECV_ENA must be set) */
    SRVL_WR(VTSS_ANA_PORT_PORT_CFG(VTSS_CHIP_PORT_CPU), VTSS_F_ANA_PORT_PORT_CFG_RECV_ENA);

    /* Set-up default packet Rx endianness, position of status word, and who will be extracting. */
    for (i = 0; i < VTSS_PACKET_RX_GRP_CNT; i++) {
        /* status word before last data */
        SRVL_WRM_CLR(VTSS_DEVCPU_QS_XTR_XTR_GRP_CFG(i), VTSS_F_DEVCPU_QS_XTR_XTR_GRP_CFG_STATUS_WORD_POS);
#ifdef VTSS_OS_BIG_ENDIAN
        /* Big-endian */
        SRVL_WRM_CLR(VTSS_DEVCPU_QS_XTR_XTR_GRP_CFG(i), VTSS_F_DEVCPU_QS_XTR_XTR_GRP_CFG_BYTE_SWAP);
#else
        /* Little-endian */
        SRVL_WRM_SET(VTSS_DEVCPU_QS_XTR_XTR_GRP_CFG(i), VTSS_F_DEVCPU_QS_XTR_XTR_GRP_CFG_BYTE_SWAP);
#endif

        if (!vtss_state->sys_config.using_vrap) {
            /* If not using VRAP, default to do register-based extraction. An FDMA driver may change this field to "2" later. */
            SRVL_WRM(VTSS_DEVCPU_QS_XTR_XTR_GRP_CFG(i), VTSS_F_DEVCPU_QS_XTR_XTR_GRP_CFG_MODE(1), VTSS_M_DEVCPU_QS_XTR_XTR_GRP_CFG_MODE);
        }

        /* Status word (only used when manually extracting) must come just before last data */
        SRVL_WRM_CLR(VTSS_DEVCPU_QS_XTR_XTR_GRP_CFG(i), VTSS_F_DEVCPU_QS_XTR_XTR_GRP_CFG_STATUS_WORD_POS);
    }

    /* Set-up default packet Tx endianness and who will be injecting. */
    for (i = 0; i < VTSS_PACKET_TX_GRP_CNT; i++) {
#ifdef VTSS_OS_BIG_ENDIAN
        /* Big-endian */
        SRVL_WRM_CLR(VTSS_DEVCPU_QS_INJ_INJ_GRP_CFG(i), VTSS_F_DEVCPU_QS_INJ_INJ_GRP_CFG_BYTE_SWAP);
#else
        /* Little-endian */
        SRVL_WRM_SET(VTSS_DEVCPU_QS_INJ_INJ_GRP_CFG(i), VTSS_F_DEVCPU_QS_INJ_INJ_GRP_CFG_BYTE_SWAP);
#endif

        /* According to datasheet, we must insert a small delay after every end-of-frame when injecting to qs. */
        SRVL_WR(VTSS_DEVCPU_QS_INJ_INJ_CTRL(i), VTSS_F_DEVCPU_QS_INJ_INJ_CTRL_GAP_SIZE(1));

        if (!vtss_state->sys_config.using_vrap) {
            /* If not using VRAP, default to do register-based injection. An FDMA driver may change this field to "2" later. */
            SRVL_WRM(VTSS_DEVCPU_QS_INJ_INJ_GRP_CFG(i), VTSS_F_DEVCPU_QS_INJ_INJ_GRP_CFG_MODE(1), VTSS_M_DEVCPU_QS_INJ_INJ_GRP_CFG_MODE);
        }
    }

    /* Setup CPU port 0 and 1. Only do this if not using VRAP */
    for (i = VTSS_CHIP_PORT_CPU_0; i <= VTSS_CHIP_PORT_CPU_1 && !vtss_state->sys_config.using_vrap; i++) {
        /* Enable IFH insertion upon extraction */
        SRVL_WRM_SET(VTSS_SYS_SYSTEM_PORT_MODE(i), VTSS_F_SYS_SYSTEM_PORT_MODE_INCL_XTR_HDR(1));

        /* Enable IFH parsing upon injection */
        SRVL_WRM_SET(VTSS_SYS_SYSTEM_PORT_MODE(i), VTSS_F_SYS_SYSTEM_PORT_MODE_INCL_INJ_HDR(1));

        /* Strict priority when reading the two CPU ports. */
        SRVL_WRM(VTSS_QSYS_HSCH_SE_CFG(218 + i), VTSS_F_QSYS_HSCH_SE_CFG_SE_DWRR_CNT(0), VTSS_M_QSYS_HSCH_SE_CFG_SE_DWRR_CNT);
    }

    /* Enable CPU port */
    SRVL_WRM_SET(VTSS_QSYS_SYSTEM_SWITCH_PORT_MODE(VTSS_CHIP_PORT_CPU), VTSS_F_QSYS_SYSTEM_SWITCH_PORT_MODE_PORT_ENA);

    /* Setup CPU port 0 and 1 to allow for classification of transmission of
     * switched frames into a user-module-specifiable QoS class.
     * For the two CPU ports, we set a one-to-one mapping between a VLAN tag's
     * PCP and a QoS class. When transmitting switched frames, the PCP value
     * of the VLAN tag (which is always inserted to get it switched on a given
     * VID), then controls the priority. */
    /* Enable looking into PCP bits */
    SRVL_WR(VTSS_ANA_PORT_QOS_CFG(VTSS_CHIP_PORT_CPU), VTSS_F_ANA_PORT_QOS_CFG_QOS_PCP_ENA);

    /* Disable aging of Rx CPU queues to allow the frames to stay there longer than
     * on normal front ports. */
    SRVL_WRM_SET(VTSS_REW_PORT_PORT_CFG(VTSS_CHIP_PORT_CPU_0), VTSS_F_REW_PORT_PORT_CFG_AGE_DIS);
    SRVL_WRM_SET(VTSS_REW_PORT_PORT_CFG(VTSS_CHIP_PORT_CPU_1), VTSS_F_REW_PORT_PORT_CFG_AGE_DIS);

    /* Set-up the one-to-one mapping */
    for (pcp = 0; pcp < VTSS_PCP_END - VTSS_PCP_START; pcp++) {
        for (dei = 0; dei < VTSS_DEI_END - VTSS_DEI_START; dei++) {
            SRVL_WR(VTSS_ANA_PORT_QOS_PCP_DEI_MAP_CFG(VTSS_CHIP_PORT_CPU, (8 * dei + pcp)),
                    VTSS_F_ANA_PORT_QOS_PCP_DEI_MAP_CFG_QOS_PCP_DEI_VAL(pcp));
        }
    }

#if defined(VTSS_FEATURE_AFI_SWC)
    // Cancel any possible ongoing AFI frames from previous session.
    for (i = 0; i < VTSS_AFI_SLOT_CNT; i++) {
        (void)srvl_afi_cancel(vtss_state, i);
    }
#endif /* defined(VTSS_FEATURE_AFI_SWC) */

    return VTSS_RC_OK;
}

vtss_rc vtss_srvl_packet_init(vtss_state_t *vtss_state, vtss_init_cmd_t cmd)
{
    vtss_packet_state_t *state = &vtss_state->packet;

    switch (cmd) {
    case VTSS_INIT_CMD_CREATE:
        state->rx_conf_set      = srvl_rx_conf_set;
        state->rx_frame_get     = srvl_rx_frame_get;
        state->rx_frame_get_raw = srvl_rx_frame_get_raw;
        state->rx_frame_discard = srvl_rx_frame_discard;
        state->tx_frame_port    = srvl_tx_frame_port;
        state->tx_frame_ifh     = srvl_tx_frame_ifh;
        state->rx_hdr_decode    = srvl_rx_hdr_decode;
        state->rx_ifh_size      = VTSS_SVL_RX_IFH_SIZE;
        state->tx_hdr_encode    = srvl_tx_hdr_encode;
        state->npi_conf_set     = srvl_npi_conf_set;
        state->dma_conf_set     = srvl_dma_conf_set;
        state->dma_offset       = srvl_dma_offset;
        state->rx_queue_count   = VTSS_PACKET_RX_QUEUE_CNT;
#if defined(VTSS_FEATURE_AFI_SWC)
        state->afi_alloc = srvl_afi_alloc;
        state->afi_free  = srvl_afi_free;
#endif /* VTSS_FEATURE_AFI_SWC */
#if defined(VTSS_FEATURE_FDMA) && VTSS_OPT_FDMA
        serval_fdma_func_init(vtss_state);
#endif
        break;
    case VTSS_INIT_CMD_INIT:
        VTSS_RC(srvl_packet_init(vtss_state));
        break;
    case VTSS_INIT_CMD_PORT_MAP:
        if (!vtss_state->warm_start_cur) {
            VTSS_RC(srvl_rx_conf_set(vtss_state));
        }
        break;
    default:
        break;
    }
    return VTSS_RC_OK;
}
#endif /* VTSS_ARCH_SERVAL */
