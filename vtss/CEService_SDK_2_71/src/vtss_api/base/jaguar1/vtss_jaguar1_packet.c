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

#define VTSS_TRACE_GROUP VTSS_TRACE_GROUP_PACKET
#include "vtss_jaguar1_cil.h"

#if defined(VTSS_ARCH_JAGUAR_1)

/* - CIL functions ------------------------------------------------- */

#if defined(VTSS_FEATURE_VSTAX) || defined(VTSS_FEATURE_NPI)
vtss_rc vtss_jr1_cpu_queue_redirect_chip(vtss_state_t *vtss_state)
{
    /*
     * This function performs the following CPU queue redirection actions if
     * l3_XXX_queue != VTSS_PACKET_RX_QUEUE_NONE and vstax_info.master_upsid != VTSS_VSTAX_UPSID_UNDEF:
     *   JR-24 standalone       : No redirection.
     *   JR-24 stackable, master: No redirection.
     *   JR-24 stackable, slave : l3_XXX_queue redirected to master.
     *   JR-48 standalone       : Primary unit: No redirection.                    Secondary unit: l3_XXX_queue redirected to primary unit.
     *   JR-48 stackable, master: Primary unit: No redirection.                    Secondary unit: l3_XXX_queue redirected to primary unit (master).
     *   JR-48 stackable, slave : Primary unit: l3_XXX_queue redirected to master. Secondary unit: l3_XXX_queue redirected to master.
     *
     * If user-controlled redirection of any CPU queue to an NPI-port is required, this can
     * only occur in configurations that says "no redirection" in the table above or if
     * l3_XXX_queue is set to VTSS_PACKET_RX_QUEUE_NONE.
     *
     * l3_XXX_queue redirection is required to overcome two bugs (Bugzilla#3792 and
     * Bugzilla#9547) in Jaguar.
     */
    vtss_packet_rx_conf_t *rx_conf = &vtss_state->packet.rx_conf;
    BOOL l3_redir_ena = FALSE;
#ifdef VTSS_FEATURE_VSTAX
    vtss_vstax_upsid_t master_upsid = vtss_state->l2.vstax_info.master_upsid;
    u32                redir_chip_port = 0xFFFFFFFFUL;
#endif

#ifdef VTSS_FEATURE_VSTAX
    {
        vtss_vstax_upsid_t my_upsid = vtss_state->l2.vstax_info.upsid[vtss_state->chip_no];

        // We redirect l3_XXX_queue according to table above.
        if ((rx_conf->map.l3_uc_queue    != VTSS_PACKET_RX_QUEUE_NONE  ||
             rx_conf->map.l3_other_queue != VTSS_PACKET_RX_QUEUE_NONE) &&
             VTSS_VSTAX_UPSID_LEGAL(master_upsid)                      &&
             master_upsid != my_upsid) {

            vtss_vstax_route_entry_t *master_rt = &vtss_state->l2.vstax_info.chip_info[vtss_state->chip_no].rt_table.table[master_upsid];
            vtss_vstax_chip_info_t   *chip_info = &vtss_state->l2.vstax_info.chip_info[vtss_state->chip_no];
            u32                      mask;

            if (master_rt->stack_port_a && chip_info->mask_a && master_rt->stack_port_b && chip_info->mask_b) {
                // Ring topology and the distance to the master is the same on both stack links.
                // We need to pick the link with the highest TTL
                if (chip_info->port_conf[0].ttl > chip_info->port_conf[1].ttl) {
                    mask = chip_info->mask_a;
                } else {
                    mask = chip_info->mask_b;
                }
            } else if (master_rt->stack_port_a && chip_info->mask_a && chip_info->port_conf[0].ttl > 0) {
                mask = chip_info->mask_a;
            } else if (master_rt->stack_port_b && chip_info->mask_b && chip_info->port_conf[1].ttl > 0) {
                mask = chip_info->mask_b;
            } else {
                mask = 0;
            }

            // Get the index of one of the bits set in mask (if any).
            if ((redir_chip_port = VTSS_OS_CTZ(mask)) >= 32) {
                // mask was 0. Master is unreachable.
                l3_redir_ena = FALSE;
            } else {
                l3_redir_ena = TRUE;
            }
        }
    }
#endif

#ifdef VTSS_FEATURE_NPI
    if (l3_redir_ena && vtss_state->packet.npi_conf.enable) {
        VTSS_E("Unable to enable redirection to NPI port while internal L3 redirect is active");
        return VTSS_RC_ERROR;
    }
    if (l3_redir_ena == FALSE && vtss_state->packet.npi_conf.enable) {
        u32 qmask, i;
        for (qmask = i = 0; i < vtss_state->packet.rx_queue_count; i++) {
            if (rx_conf->queue[i].npi.enable) {
                qmask |= VTSS_BIT(i); /* NPI redirect */
            }
        }
        if (VTSS_PORT_CHIP_SELECTED(vtss_state->packet.npi_conf.port_no)) {
            JR_WRF(ARB, CFG_STATUS_STACK_CFG, GCPU_PORT_CFG, VTSS_CHIP_PORT(vtss_state->packet.npi_conf.port_no));
            JR_WRF(ARB, CFG_STATUS_STACK_CFG, CPU_TO_GCPU_MASK, qmask);
        } else {
            // Redirect to one of the internal ports.
            JR_WRF(ARB, CFG_STATUS_STACK_CFG, GCPU_PORT_CFG, vtss_state->port.port_int_0);
            JR_WRF(ARB, CFG_STATUS_STACK_CFG, CPU_TO_GCPU_MASK, qmask);
        }
    } else
#endif
#ifdef VTSS_FEATURE_VSTAX
    if (l3_redir_ena) {
        u32 mask = 0;

        VTSS_IG(VTSS_TRACE_GROUP_PACKET, "Redirecting queue #%u of chip = %u to upsid = %d", rx_conf->map.l3_uc_queue, vtss_state->chip_no, master_upsid);
        // Remote CPU is the UPSID of the master.
        JR_WRF(REW, COMMON_VSTAX_GCPU_CFG, GCPU_UPSID, master_upsid);

        // Remote CPU queue is the highest CPU queue that this revision of the chip supports
        // without putting the frame into the error queue (identified by stack_queue) on
        // reception at the master. This number is 2.
        JR_WRF(REW, COMMON_VSTAX_GCPU_CFG, GCPU_UPSPN, 2);

        // The CPU queue(s) to redirect.
        if (rx_conf->map.l3_uc_queue != VTSS_PACKET_RX_QUEUE_NONE) {
            mask |=  VTSS_BIT(rx_conf->map.l3_uc_queue);
        }
        if (rx_conf->map.l3_other_queue != VTSS_PACKET_RX_QUEUE_NONE) {
            mask |=  VTSS_BIT(rx_conf->map.l3_other_queue);
        }
        JR_WRF(ARB, CFG_STATUS_STACK_CFG, CPU_TO_GCPU_MASK, mask);

        // Send the frame in the right direction.
        JR_WRF(ARB, CFG_STATUS_STACK_CFG, GCPU_PORT_CFG, redir_chip_port);
    } else
#endif
    {
        VTSS_IG(VTSS_TRACE_GROUP_PACKET, "Redirection disabled");
        JR_WRF(ARB, CFG_STATUS_STACK_CFG, GCPU_PORT_CFG, 63); /* == disabled */
        JR_WRF(ARB, CFG_STATUS_STACK_CFG, CPU_TO_GCPU_MASK, 0); /* zero mask to be sure */
    }

    return VTSS_RC_OK;
}
#endif

/* ================================================================= *
 * NPI
 * ================================================================= */

#if defined(VTSS_FEATURE_NPI)
static vtss_rc jr_npi_mask_set_chip(vtss_state_t *vtss_state)
{
    VTSS_RC(vtss_jr1_cpu_queue_redirect_chip(vtss_state));

    /* Control VStaX-2 awareness */
    if (vtss_state->packet.npi_conf.port_no != VTSS_PORT_NO_NONE && VTSS_PORT_CHIP_SELECTED(vtss_state->packet.npi_conf.port_no)) {
        u32 chip_port = VTSS_CHIP_PORT(vtss_state->packet.npi_conf.port_no);
        JR_WRXB(REW, PHYSPORT_VSTAX_CTRL, chip_port, VSTAX_HDR_ENA, vtss_state->packet.npi_conf.enable);
        JR_WRXB(ANA_CL_2, PORT_STACKING_CTRL, chip_port, STACKING_AWARE_ENA, vtss_state->packet.npi_conf.enable);
        JR_WRXB(ANA_CL_2, PORT_STACKING_CTRL, chip_port, STACKING_HEADER_DISCARD_ENA, !vtss_state->packet.npi_conf.enable);
    }

    return VTSS_RC_OK;
}

static vtss_rc jr_npi_conf_set(vtss_state_t *vtss_state, const vtss_npi_conf_t *const conf)
{
    /* Disable current NPI port. */
    vtss_state->packet.npi_conf.enable = FALSE;
    VTSS_RC(vtss_jr1_conf_chips(vtss_state, jr_npi_mask_set_chip));

    /* Set new configuration */
    vtss_state->packet.npi_conf = *conf;
    if (!vtss_state->packet.npi_conf.enable) {
      vtss_state->packet.npi_conf.port_no = VTSS_PORT_NO_NONE;
    }
    VTSS_RC(vtss_jr1_conf_chips(vtss_state, jr_npi_mask_set_chip));

    /* VStaX enabled/disabled, setup all VLANs (an NPI port should not be in normal VLANs) */
    VTSS_RC(vtss_cmn_vlan_update_all(vtss_state));

    // Don't forget DSM taxi rate WM
    return VTSS_RC_OK;
}
#endif /* VTSS_FEATURE_NPI */

/* ================================================================= *
 *  Packet control
 * ================================================================= */

#ifdef VTSS_FEATURE_VSTAX_V2
vtss_rc vtss_jr1_lrn_all_set_chip(vtss_state_t *vtss_state)
{
    /* S/W-assisted H/W learning (see Bugzilla#7730 & 7737) */
    vtss_vstax_chip_info_t *chip_info = &vtss_state->l2.vstax_info.chip_info[vtss_state->chip_no];
    BOOL stacking = chip_info->mask_a != 0 || chip_info->mask_b != 0;

    if (stacking) {
        u32 qu;
        vtss_packet_rx_queue_map_t *map = &vtss_state->packet.rx_conf.map;
        u32 rmask = VTSS_F_ANA_L2_COMMON_VSTAX_CTRL_VSTAX2_LCPU_CP_NEW_FRONT_ENA     |
                    VTSS_F_ANA_L2_COMMON_VSTAX_CTRL_VSTAX2_LCPU_CP_MOVE_FRONT_ENA    |
                    VTSS_F_ANA_L2_COMMON_VSTAX_CTRL_VSTAX2_LCPU_CP_REFRESH_FRONT_ENA |
                    VTSS_F_ANA_L2_COMMON_VSTAX_CTRL_VSTAX2_LCPU_CP_NXT_LRN_ALL_FRONT_ENA;

        /* Turn off CPU-copying of learn-all frames while setting up the queues. */
        JR_WRM(ANA_L2, COMMON_VSTAX_CTRL, 0, rmask);

        /* Set up the learn-all CPU queue */
        JR_WRF(ANA_L2, COMMON_VSTAX_CTRL, CPU_VSTAX_QU, map->lrn_all_queue == VTSS_PACKET_RX_QUEUE_NONE ? 0 : map->lrn_all_queue);

        if (map->lrn_all_queue == VTSS_PACKET_RX_QUEUE_NONE) {
            // Release any truncations.
            for (qu = 0; qu < vtss_state->packet.rx_queue_count; qu++) {
                JR_WRX(DEVCPU_QS, XTR_XTR_FRM_PRUNING, qu, 0);
            }
        } else {
            /* Truncate the lrn-all queue to 64 byte packets + 24 byte IFH under the assumption
             * that this queue is *only* used for learn-all frames. This is also assumed by the FDMA code. */
            JR_WRX(DEVCPU_QS, XTR_XTR_FRM_PRUNING, map->lrn_all_queue, ((64 + 24) / sizeof(u32)) - 1);
        }

        /* Make sure that learn-all frames that go to the CPU for multiple reasons are received twice
         * (both in the learn-all queue and the most-significant other queue).
         * The reason for this is that the learn-all frames are re-injected on the stack ports
         * deep down in the FDMA driver, and never sent to the application code, so if the frame
         * at the same time is marked as CPU-copy/redirect for some other reason, we must get it
         * into that other CPU Rx queue.
         */
        JR_WR(ANA_AC, PS_COMMON_CPU_CFG, map->lrn_all_queue == VTSS_PACKET_RX_QUEUE_NONE ? VTSS_M_ANA_AC_PS_COMMON_CPU_CFG_ONE_CPU_COPY_ONLY_MASK : VTSS_M_ANA_AC_PS_COMMON_CPU_CFG_ONE_CPU_COPY_ONLY_MASK & ~(VTSS_BIT(map->lrn_all_queue)));

        /* Turn on CPU-copying of learn-all frames now that everything is in place */
        if (map->lrn_all_queue != VTSS_PACKET_RX_QUEUE_NONE) {
            JR_WRM(ANA_L2, COMMON_VSTAX_CTRL, rmask, rmask);
        }
    }
    return VTSS_RC_OK;
}
#endif

static vtss_rc jr_rx_conf_set_chip(vtss_state_t *vtss_state)
{
    vtss_packet_rx_conf_t      *conf = &vtss_state->packet.rx_conf;
    vtss_packet_rx_reg_t       *reg = &conf->reg;
    vtss_packet_rx_queue_map_t *map = &conf->map;
    vtss_packet_rx_port_conf_t *port_conf;
    vtss_packet_reg_type_t     type;
    vtss_port_no_t             port_no;
    u32                        port, i, j, cap_cfg, garp_cfg, bpdu_cfg, val;
    BOOL                       cpu_only;
    u32                        hwm, lwm, atop, qu_id, qu;

    /* Setup Rx registrations for all ports */
    cap_cfg = (JR_PUT_BIT(ANA_CL_2, PORT_CAPTURE_CFG, CPU_MLD_REDIR_ENA, reg->mld_cpu_only) |
               JR_PUT_BIT(ANA_CL_2, PORT_CAPTURE_CFG, CPU_IP4_MC_COPY_ENA, reg->ipmc_ctrl_cpu_copy) |
               JR_PUT_BIT(ANA_CL_2, PORT_CAPTURE_CFG, CPU_IGMP_REDIR_ENA, reg->igmp_cpu_only));
    
    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
        if (!VTSS_PORT_CHIP_SELECTED(port_no))
            continue;
        port = VTSS_CHIP_PORT(port_no);
        port_conf = &vtss_state->packet.rx_port_conf[port_no];
        bpdu_cfg = 0;
        garp_cfg = 0;
        for (i = 0; i < 32; i++) {
            if (i < 16) {
                /* BPDU */
                j = i;
                type = port_conf->bpdu_reg[j];
                cpu_only = reg->bpdu_cpu_only;
            } else {
                /* GARP */
                j = (i - 16);
                type = port_conf->garp_reg[j];
                cpu_only = reg->garp_cpu_only[j];
            }
            switch (type) {
            case VTSS_PACKET_REG_NORMAL:
                val = (cpu_only ? 1 : 0);
                break;
            case VTSS_PACKET_REG_FORWARD:
                val = 0;
                break;
            case VTSS_PACKET_REG_DISCARD:
                val = 3;
                break;
            case VTSS_PACKET_REG_CPU_COPY:
                val = 2;
                break;
            case VTSS_PACKET_REG_CPU_ONLY:
                val = 1;
                break;
            default:
                val = 0;
                break;
            }
            val = (val << (j * 2));
            if (i < 16)
                bpdu_cfg |= val;
            else
                garp_cfg |= val;
        }
        JR_WRX(ANA_CL_2, PORT_CAPTURE_CFG, port, cap_cfg);
        JR_WRX(ANA_CL_2, PORT_CAPTURE_GXRP_CFG, port, garp_cfg);
        JR_WRX(ANA_CL_2, PORT_CAPTURE_BPDU_CFG, port, bpdu_cfg);
    }

    /* CPU OQS ports 81-88 (ethernet->CPU). Only one queue per port. */
    for (port = 81; port < 89; port++) {
        hwm  = conf->queue[port - 81].size / 160;
        atop = (hwm == 0) ? 0 : (hwm + 60);
        lwm = hwm > 13 ? (hwm - 13) : 0;
        qu_id = vtss_jr1_port_prio_2_qu(vtss_state, port, 0, 1);

        JR_WRXF(OQS, QU_RAM_CFG_QU_RC_HLWM_CFG, qu_id, QU_RC_PROFILE_HWM, hwm);
        JR_WRXF(OQS, QU_RAM_CFG_QU_RC_HLWM_CFG, qu_id, QU_RC_PROFILE_LWM, lwm);
        JR_WRXF(OQS, QU_RAM_CFG_QU_RC_ATOP_SWM_CFG, qu_id, QU_RC_PROFILE_ATOP, atop);

        JR_WRXF(OQS, PORT_RAM_CFG_PORT_RC_HLWM_CFG, port, PORT_RC_PROFILE_HWM, hwm);
        JR_WRXF(OQS, PORT_RAM_CFG_PORT_RC_HLWM_CFG, port, PORT_RC_PROFILE_LWM, lwm);
        JR_WRXF(OQS, PORT_RAM_CFG_PORT_RC_ATOP_SWM_CFG, port, PORT_RC_PROFILE_ATOP, atop);              
    }
    /* CPU IQS ports 33 and 34 (CPU->ethernet). Setting FWD pressure to 1 to force IQS frames to OQS */
    for (port = 33; port < 35; port++) {
        for (qu = 0; qu < 8; qu++) {
            qu_id = vtss_jr1_port_prio_2_qu(vtss_state, port, qu, 0);
            JR_WRXF(IQS, QU_RAM_CFG_QU_RC_CFG, qu_id, FWD_PRESS_THRES, 1);
        }
    }

    /* Setup queue mappings */
    bpdu_cfg = (JR_PUT_FLD(ANA_CL_2, COMMON_CPU_BPDU_QU_CFG, CPU_GXRP_QU, map->garp_queue) |
                JR_PUT_FLD(ANA_CL_2, COMMON_CPU_BPDU_QU_CFG, CPU_BPDU_QU, map->bpdu_queue));
    for (i = 0; i < 16; i++) {
        JR_WRX(ANA_CL_2, COMMON_CPU_BPDU_QU_CFG, i, bpdu_cfg);
    }
    JR_WR(ANA_CL_2, COMMON_CPU_PROTO_QU_CFG,
          JR_PUT_FLD(ANA_CL_2, COMMON_CPU_PROTO_QU_CFG, CPU_MLD_QU, map->igmp_queue) |
          JR_PUT_FLD(ANA_CL_2, COMMON_CPU_PROTO_QU_CFG, CPU_IP4_MC_CTRL_QU, map->ipmc_ctrl_queue) |
          JR_PUT_FLD(ANA_CL_2, COMMON_CPU_PROTO_QU_CFG, CPU_IGMP_QU, map->igmp_queue));
    JR_WRF(ANA_L2, COMMON_LRN_CFG, CPU_LRN_QU, map->learn_queue);
    JR_WRF(ANA_L2, COMMON_LRN_CFG, CPU_SMAC_QU, map->mac_vid_queue);
    JR_WRF(ANA_L2, COMMON_FWD_CFG, CPU_DMAC_QU, map->mac_vid_queue);
    JR_WRB(ANA_L2, COMMON_FWD_CFG, CPU_DMAC_COPY_ENA, 1);

#ifdef VTSS_FEATURE_VSTAX_V2
    VTSS_RC(vtss_jr1_lrn_all_set_chip(vtss_state));

    VTSS_RC(vtss_jr1_cpu_queue_redirect_chip(vtss_state));

    /* Use policers to avoid CPU copy */
    VTSS_RC(vtss_jr1_cpu_pol_setup_chip(vtss_state));
#endif

    // Configure L3 routing CPU queues
    JR_WR(ANA_L3_2, COMMON_CPU_QU_CFG,
          JR_PUT_FLD(ANA_L3_2, COMMON_CPU_QU_CFG, CPU_RLEG_QU,             map->l3_uc_queue)    |
          JR_PUT_FLD(ANA_L3_2, COMMON_CPU_QU_CFG, CPU_RLEG_IP_OPT_QU,      map->l3_other_queue) |
          JR_PUT_FLD(ANA_L3_2, COMMON_CPU_QU_CFG, CPU_RLEG_IP_HDR_FAIL_QU, map->l3_other_queue) |
          JR_PUT_FLD(ANA_L3_2, COMMON_CPU_QU_CFG, CPU_UC_QU,               map->l3_uc_queue)    |
          JR_PUT_FLD(ANA_L3_2, COMMON_CPU_QU_CFG, CPU_MC_FAIL_QU,          map->l3_other_queue) |
          JR_PUT_FLD(ANA_L3_2, COMMON_CPU_QU_CFG, CPU_UC_FAIL_QU,          map->l3_uc_queue)    |
          JR_PUT_FLD(ANA_L3_2, COMMON_CPU_QU_CFG, CPU_IP_TTL_FAIL_QU,      map->l3_other_queue));

    if (map->sflow_queue != VTSS_PACKET_RX_QUEUE_NONE) {
        JR_WRF(ANA_AC, PS_COMMON_SFLOW_CFG, SFLOW_CPU_QU, map->sflow_queue);
    }

    /* Due to a bug in JR1, CPU-generated stack traffic destined for a CPU queue with
     * a number greater than 2, will get redirected to the error queue. In order to
     * receive such frames in the stack queue number, we must configure the
     * error queue to the stack queue.
     */
    if (map->stack_queue != VTSS_PACKET_RX_QUEUE_NONE) {
        JR_WRF(ANA_AC, PS_COMMON_COMMON_VSTAX_CFG, VSTAX2_FWD_ERR_QU, map->stack_queue);
    }

    /* Enable frame extraction queues and setup mapping to extraction groups */
    for (i = 0; i < JR_PACKET_RX_QUEUE_CNT; i++) {
        JR_WRX(DEVCPU_QS, XTR_XTR_MAP, i,
               JR_PUT_FLD(DEVCPU_QS, XTR_XTR_MAP, GRP, conf->grp_map[i]) |
               JR_PUT_BIT(DEVCPU_QS, XTR_XTR_MAP, MAP_ENA, 1));
    }

#if defined(VTSS_FEATURE_NPI)
    VTSS_RC(jr_npi_mask_set_chip(vtss_state));
#endif

    return VTSS_RC_OK;
}

static vtss_rc jr_rx_conf_set(vtss_state_t *vtss_state) 
{
    /* Setup config for all devices */
    VTSS_RC(vtss_jr1_conf_chips(vtss_state, jr_rx_conf_set_chip));

    /* Register IGMP */
    VTSS_RC(vtss_jr1_igmp_cpu_copy_set(vtss_state));
    
    return VTSS_RC_OK;
}

#define XTR_EOF_0     0x80000000U
#define XTR_EOF_1     0x80000001U
#define XTR_EOF_2     0x80000002U
#define XTR_EOF_3     0x80000003U
#define XTR_PRUNED    0x80000004U
#define XTR_ABORT     0x80000005U
#define XTR_ESCAPE    0x80000006U
#define XTR_NOT_READY 0x80000007U

static vtss_rc jr_rx_frame_discard_internal(const vtss_state_t *vtss_state,
                                            const vtss_chip_no_t       chip_no,
                                            const vtss_packet_rx_grp_t grp,
                                            u32                  *discarded_bytes)
{
    BOOL done = 0;
    u32  val;

    *discarded_bytes = 0;

    while (!done) {
        JR_RDX_CHIP(chip_no, DEVCPU_QS, XTR_XTR_RD, grp, &val);
        switch(val) {
        case XTR_ABORT:
        case XTR_PRUNED:
        case XTR_EOF_3:
        case XTR_EOF_2:
        case XTR_EOF_1:
        case XTR_EOF_0:
            done = 1;
            /* FALLTHROUGH */
        case XTR_ESCAPE:
            JR_RDX_CHIP(chip_no, DEVCPU_QS, XTR_XTR_RD, grp, &val);
            break;
        case XTR_NOT_READY:
            done = 1;
            break;
        default:
            (*discarded_bytes)++;
            break;
        }
    }

    *discarded_bytes *= 4;

    return VTSS_RC_OK;
}

/**
 * Return values:
 *  0 = Data OK.
 *  1 = EOF reached. Data OK. bytes_valid indicates the number of valid bytes in last word ([1; 4]).
 *  2 = Error. No data from queue system.
 */
static int jr_rx_frame_word(const vtss_state_t *vtss_state,
                            vtss_chip_no_t chip_no, vtss_packet_rx_grp_t grp, BOOL first_word, u32 *val, u32 *bytes_valid)
{
    JR_RDX_CHIP(chip_no, DEVCPU_QS, XTR_XTR_RD, grp, val);

    if (*val == XTR_NOT_READY) {
        /** XTR_NOT_READY means two different things depending on whether this is the first
         * word read of a frame or after at least one word has been read.
         * When the first word, the group is empty, and we return an error.
         * Otherwise we have to wait for the FIFO to have received some more data. */
        if (first_word) {
          return 2; /* Error */
        }
        do {
            JR_RDX_CHIP(chip_no, DEVCPU_QS, XTR_XTR_RD, grp, val);
        } while(*val == XTR_NOT_READY);
    }

    switch(*val) {
    case XTR_ABORT:
        /* No accompanying data. */
        VTSS_E("Aborted frame");
        return 2; /* Error */
    case XTR_EOF_0:
    case XTR_EOF_1:
    case XTR_EOF_2:
    case XTR_EOF_3:
    case XTR_PRUNED:
        *bytes_valid = 4 - ((*val) & 3);
        JR_RDX_CHIP(chip_no, DEVCPU_QS, XTR_XTR_RD, grp, val);
        if (*val == XTR_ESCAPE) {
            JR_RDX_CHIP(chip_no, DEVCPU_QS, XTR_XTR_RD, grp, val);
        }
        return 1; /* EOF */
    case XTR_ESCAPE:
        JR_RDX_CHIP(chip_no, DEVCPU_QS, XTR_XTR_RD, grp, val);
        /* FALLTHROUGH */
    default:
        *bytes_valid = 4;
        return 0;
    }
}

static vtss_rc jr_rx_frame_get_internal(const vtss_state_t   *vtss_state,
                                        vtss_chip_no_t       chip_no,
                                        vtss_packet_rx_grp_t grp,
                                        u8                   *const ifh,
                                        u8                   *const frame,
                                        const u32            buf_length,
                                        u32                  *frm_length) /* Including CRC */
{
    u32     i, val, bytes_valid, buf_len = buf_length;
    BOOL    done = 0, first_word = 1;
    u8      *buf;
    int     result;


    /* Check if data is ready for grp */
    JR_RDF_CHIP(chip_no, DEVCPU_QS, XTR_XTR_DATA_PRESENT, DATA_PRESENT_GRP, &val);
    if (!(val & VTSS_BIT(grp))) {
        return VTSS_RC_ERROR;
    }

    *frm_length = 0;
    buf = ifh;

    /* Read IFH. It consists of a 1 word queue number and 6 words of real IFH, hence 7 words */
    for (i = 0; i < 7; i++) {
        if (jr_rx_frame_word(vtss_state, chip_no, grp, first_word, &val, &bytes_valid) != 0) {
            /* We accept neither EOF nor ERROR when reading the IFH */
            return VTSS_RC_ERROR;
        }
        *buf++ = (u8)(val >>  0);
        *buf++ = (u8)(val >>  8);
        *buf++ = (u8)(val >> 16);
        *buf++ = (u8)(val >> 24);
        first_word = 0;
    }

    buf = frame;

    /* Read the rest of the frame */
    while (!done && buf_len >= 4) {
        result = jr_rx_frame_word(vtss_state, chip_no, grp, 0, &val, &bytes_valid);
        if (result == 2) {
            // Error.
            return VTSS_RC_ERROR;
        }
        // Store the data.
        *frm_length += bytes_valid;
        *buf++ = (u8)(val >>  0);
        *buf++ = (u8)(val >>  8);
        *buf++ = (u8)(val >> 16);
        *buf++ = (u8)(val >> 24);
        buf_len -= bytes_valid;
        done = result == 1;
    }

    if (!done) {
        /* Buffer overrun. Skip remainder of frame */
        u32 discarded_bytes;
        (void)jr_rx_frame_discard_internal(vtss_state, chip_no, grp, &discarded_bytes);
        *frm_length += discarded_bytes;
        return VTSS_RC_INCOMPLETE; // Special return value indicating that there wasn't room.
    }

    if (*frm_length < 60) {
        VTSS_E("short frame, %u bytes read", *frm_length);
        return VTSS_RC_ERROR;
    }

    return VTSS_RC_OK;
}


static vtss_rc jr_rx_frame_discard(vtss_state_t *vtss_state,
                                   const vtss_packet_rx_queue_t queue_no)
{
    vtss_packet_rx_grp_t grp = vtss_state->packet.rx_conf.grp_map[queue_no];
    u32 dummy;
    return jr_rx_frame_discard_internal(vtss_state, vtss_state->chip_no, grp, &dummy);
}

static vtss_rc jr_rx_frame_get_chip(vtss_state_t                 *vtss_state,
                                    const vtss_packet_rx_queue_t queue_no,
                                    vtss_packet_rx_header_t      *const header,
                                    u8                           *const frame,
                                    const u32                    length)
{
    vtss_rc              rc;
    vtss_packet_rx_grp_t grp = vtss_state->packet.rx_conf.grp_map[queue_no];
    u32                  val, port;
    u8                   ifh[28];
    u64                  vstax_lo, fwd;

    /* Check if data is ready in queue */
    JR_RDF(DEVCPU_QS, XTR_XTR_DATA_PRESENT, DATA_PRESENT, &val);
    if (!(val & VTSS_BIT(queue_no))) {
        return VTSS_RC_INCOMPLETE;
    }

    if ((rc = jr_rx_frame_get_internal(vtss_state, vtss_state->chip_no, grp, ifh, frame, length, &header->length)) != VTSS_RC_OK) {
        return rc;
    }

    header->length -= 4; /* jr_rx_frame_get_internal() returns frame length including CRC. This function returns frame length excluding CRC. */
    vstax_lo = ((u64)ifh[14] << 56) | ((u64)ifh[15] << 48) | ((u64)ifh[16] << 40) | ((u64)ifh[17] << 32) |
               ((u64)ifh[18] << 24) | ((u64)ifh[19] << 16) | ((u64)ifh[20] <<  8) | ((u64)ifh[21] <<  0);
    fwd      =                                               ((u64)ifh[22] << 40) | ((u64)ifh[23] << 32) |
               ((u64)ifh[24] << 24) | ((u64)ifh[25] << 16) | ((u64)ifh[26] <<  8) | ((u64)ifh[27] <<  0);

    header->arrived_tagged = VTSS_EXTRACT_BITFIELD64(vstax_lo, 15,  1); /* vstax::tag::was_tagged */
    header->tag.vid        = VTSS_EXTRACT_BITFIELD64(vstax_lo, 16, 12); /* vstax::tag::cl_vid     */
    header->tag.cfi        = VTSS_EXTRACT_BITFIELD64(vstax_lo, 28,  1); /* vstax::tag::dei        */
    header->tag.tagprio    = VTSS_EXTRACT_BITFIELD64(vstax_lo, 29,  3); /* vstax::tag::pcp        */
    port                   = VTSS_EXTRACT_BITFIELD64(fwd,      39,  6); /* fwd::src_port          */
    header->learn          = 0;                                         /* TBD */
    header->port_no        = vtss_cmn_chip_to_logical_port(vtss_state, vtss_state->chip_no, port);

    return VTSS_RC_OK;
}

static vtss_rc jr_rx_frame_get(vtss_state_t                 *vtss_state,
                               const vtss_packet_rx_queue_t queue_no,
                               vtss_packet_rx_header_t      *const header,
                               u8                           *const frame,
                               const u32                    length)
{
    vtss_chip_no_t chip_no;
    vtss_rc        rc;

    // Try all units in consecutive order.
    // If jr_rx_frame_get_chip() returns something different from
    // VTSS_RC_INCOMPLETE, then stop, because VTSS_RC_INCOMPLETE
    // indicates that the chip we're reading has no pending frames.
    // All other return codes indicate success or other kind of
    // error.
    for (chip_no = 0; chip_no < vtss_state->chip_count; chip_no++) {
        VTSS_SELECT_CHIP(chip_no);
        rc = jr_rx_frame_get_chip(vtss_state, queue_no, header, frame, length);
        if (rc != VTSS_RC_INCOMPLETE) {
            // Success of failure.
            return rc;
        }
    }

    // Last chip and no frames found on this one either.
    return VTSS_RC_ERROR;
}

static inline u32 BYTE_SWAP(u32 v)
{
    register u32 v1 = v;
    v1 = ((v1 >> 24) & 0x000000FF) | ((v1 >> 8) & 0x0000FF00) | ((v1 << 8) & 0x00FF0000) | ((v1 << 24) & 0xFF000000);
    return v1;
}

static vtss_rc jr_tx_frame_port(vtss_state_t          *vtss_state,
                                const vtss_port_no_t  port_no,
                                const u8              *const frame,
                                const u32             length,
                                const vtss_vid_t      vid)
{
    u32 value, count, w, last, ifh[6], grp = 0; /* First injection group used */
    const u8 *buf = frame;

    VTSS_N("port_no: %u, length: %u, vid: %u", port_no, length, vid);

    VTSS_SELECT_CHIP_PORT_NO(port_no);
    JR_RD(DEVCPU_QS, INJ_INJ_STATUS, &value);
    if (!(JR_GET_FLD(DEVCPU_QS, INJ_INJ_STATUS, FIFO_RDY, value) & VTSS_BIT(grp))) {
        VTSS_E("Not ready");
        return VTSS_RC_ERROR;
    }

    if (JR_GET_FLD(DEVCPU_QS, INJ_INJ_STATUS, WMARK_REACHED, value) & VTSS_BIT(grp)) {
        VTSS_E("WM reached");
        return VTSS_RC_ERROR;
    }
    
    /* Indicate SOF */
    JR_WRX(DEVCPU_QS, INJ_INJ_CTRL, grp, JR_PUT_BIT(DEVCPU_QS, INJ_INJ_CTRL, SOF, 1));

    ifh[0]  = 0;                                                   // MSW(dst)
    ifh[1]  = VTSS_ENCODE_BITFIELD(VTSS_CHIP_PORT(port_no), 0, 8); // LSW(dst).uc_inject_dst
    ifh[2]  = VTSS_ENCODE_BITFIELD(1, 79 - 48, 1);                 // vstax.resv (must be 1)
    ifh[2] |= VTSS_ENCODE_BITFIELD(1, 59 - 48, 1);                 // vstax.sp
    ifh[3]  = 0;
    ifh[4]  = VTSS_ENCODE_BITFIELD(1, 45 - 32, 1);                 // fwd.vstax_avail
    ifh[4] |= VTSS_ENCODE_BITFIELD(1, 44 - 32, 1);                 // fwd.update_fcs
    ifh[5]  = VTSS_ENCODE_BITFIELD(1, 28 -  0, 1);                 // fwd.do_not_rew
    ifh[5] |= VTSS_ENCODE_BITFIELD(2, 16 -  0, 2);                 // fwd.inject_mode = INJECT_UNICAST.

    for (w = 0; w < 6; w++) {
        JR_WRX(DEVCPU_QS, INJ_INJ_WR, grp, BYTE_SWAP(ifh[w]));
    }
            
    /* Write whole words */
    count = (length / 4);
    for (w = 0; w < count; w++, buf += 4) {
        if (w == 3 && vid != VTSS_VID_NULL) {
            /* Insert C-tag */
            JR_WRX(DEVCPU_QS, INJ_INJ_WR, grp, BYTE_SWAP((0x8100U << 16) | vid));
            w++;
        }
        JR_WRX(DEVCPU_QS, INJ_INJ_WR, grp, (buf[3] << 24) | (buf[2] << 16) | (buf[1] << 8) | buf[0]);
    }

    /* Write last, partial word */
    last = (length % 4);
    if (last) {
        JR_WRX(DEVCPU_QS, INJ_INJ_WR, grp, (buf[3] << 24) | (buf[2] << 16) | (buf[1] << 8) | buf[0]);
        w++;
    }
        
    /* Indicate EOF and valid bytes in last word */
    JR_WRX(DEVCPU_QS, INJ_INJ_CTRL, grp, 
           JR_PUT_FLD(DEVCPU_QS, INJ_INJ_CTRL, VLD_BYTES, length < 60 ? 0 : last) |
           JR_PUT_BIT(DEVCPU_QS, INJ_INJ_CTRL, EOF, 1));

    
    /* Add dummy CRC (Jaguar will do padding, if neccessary) */
    JR_WRX(DEVCPU_QS, INJ_INJ_WR, grp, 0);
    w++;

    VTSS_N("Wrote %u words, last: %u", w, last);
    
    return VTSS_RC_OK;
}

#if defined(VTSS_FEATURE_VSTAX_V2)
static vtss_rc jr_vstax_header2frame(const vtss_state_t            *const state,
                                     const vtss_port_no_t          port_no,
                                     const vtss_vstax_tx_header_t  *const vstax,
                                     u8                            *const frame)
{
    u32 chip_port = (vstax->port_no == VTSS_PORT_NO_NONE ? vstax->chip_port : VTSS_CHIP_PORT_FROM_STATE(state, vstax->port_no));
    u32 val;
    u32 vstax_hi = 0;
    u64 vstax_lo = 0;

    /* EtherType */
    vstax_hi |= VTSS_ENCODE_BITFIELD(VTSS_ETYPE_VTSS, 16, 16);

    /* Valid */
    vstax_hi |= VTSS_ENCODE_BITFIELD(1,               15,  1);

    /* Ingress Drop Mode == 1 => No head-of-line blocking. Not used on super-prio injection */
    vstax_lo |= VTSS_ENCODE_BITFIELD64(1, 55, 1);

    /* DP */
    if (vstax->dp) {
        vstax_lo |= VTSS_ENCODE_BITFIELD64(vstax->dp, 60, 2);
    }

    /* QoS */
    if (vstax->prio == VTSS_PRIO_SUPER) {
        vstax_lo |= VTSS_ENCODE_BITFIELD64(TRUE, 59, 1);
    } else {
        vstax_lo |= VTSS_ENCODE_BITFIELD64(vstax->prio, 56, 3);
    }

    /* TTL */
    if (vstax->ttl == VTSS_VSTAX_TTL_PORT) {
        /* In two-chip solutions that send to the secondary's stack port, we actually pick
         * the TTL for the interconnect port, because the FDMA transmits indirectly through
         * the interconnect port. Application must set keep_ttl to 0 in that case.
         */
        val = state->l2.vstax_info.chip_info[0].port_conf[port_no == state->l2.vstax_conf.port_0 ? 0 : 1].ttl;
    } else {
        val = vstax->ttl;
    }
    vstax_lo |= VTSS_ENCODE_BITFIELD64(val, 48, 5);

    /* LRN skip */
    vstax_lo |= VTSS_ENCODE_BITFIELD64(TRUE, 47, 1);

    /* Fwd mode and associated fields*/
    vstax_lo |= VTSS_ENCODE_BITFIELD64(vstax->fwd_mode, 44, 3);
    switch(vstax->fwd_mode) {
    case VTSS_VSTAX_FWD_MODE_LOOKUP:
        break;
    case VTSS_VSTAX_FWD_MODE_UPSID_PORT:
        /* dst_port_type=0 */
        vstax_lo |= VTSS_ENCODE_BITFIELD64((u32)vstax->upsid - VTSS_VSTAX_UPSID_START, 37, 5);
        vstax_lo |= VTSS_ENCODE_BITFIELD64(chip_port, 32, 5);
        break;
    case VTSS_VSTAX_FWD_MODE_CPU_UPSID:
        vstax_lo |= VTSS_ENCODE_BITFIELD64((u32)vstax->upsid - VTSS_VSTAX_UPSID_START, 37, 5);
        vstax_lo |= VTSS_ENCODE_BITFIELD64(vstax->queue_no - VTSS_PACKET_RX_QUEUE_START, 32, 4);
        break;
    case VTSS_VSTAX_FWD_MODE_CPU_ALL:
        vstax_lo |= VTSS_ENCODE_BITFIELD64(vstax->keep_ttl, 41, 1);
        vstax_lo |= VTSS_ENCODE_BITFIELD64(vstax->queue_no - VTSS_PACKET_RX_QUEUE_START, 32, 4);
        break;
    default:
        VTSS_E("Illegal fwd mode: %d", vstax->fwd_mode);
    }

    /* uprio */
    vstax_lo |= VTSS_ENCODE_BITFIELD64(vstax->tci.tagprio, 29, 3);

    /* cfi */
    vstax_lo |= VTSS_ENCODE_BITFIELD64(vstax->tci.cfi, 28, 1);

    /* VID */
    vstax_lo |= VTSS_ENCODE_BITFIELD64(vstax->tci.vid, 16, 12);

    /* Source */
    if (vstax->glag_no != VTSS_GLAG_NO_NONE) {
        /* src_addr_mode (bit #10) == 1, a.k.a. src_glag */
        vstax_lo |= VTSS_ENCODE_BITFIELD64(TRUE, 10, 1);
        vstax_lo |= VTSS_ENCODE_BITFIELD64(vstax->glag_no, 0, 5);
    } else {
        /* src_addr_mode (bit #10) == 0, a.k.a. src_ind_port */
        /* Pick the src_port_type (bit #11) == 1, a.k.a. port_type_intpn. */
        vstax_lo |= VTSS_ENCODE_BITFIELD64(TRUE, 11, 1);
        vstax_lo |= VTSS_ENCODE_BITFIELD64((u8)state->l2.vstax_info.upsid[0], 5, 5); /* Source chip_no == 0, i.e. ourselves. */
        vstax_lo |= VTSS_ENCODE_BITFIELD64(0xf, 0, 5); /* intpn_dlookup */
    }

    frame[ 0] = vstax_hi >> 24;
    frame[ 1] = vstax_hi >> 16;
    frame[ 2] = vstax_hi >>  8;
    frame[ 3] = vstax_hi >>  0;
    frame[ 4] = vstax_lo >> 56;
    frame[ 5] = vstax_lo >> 48;
    frame[ 6] = vstax_lo >> 40;
    frame[ 7] = vstax_lo >> 32;
    frame[ 8] = vstax_lo >> 24;
    frame[ 9] = vstax_lo >> 16;
    frame[10] = vstax_lo >>  8;
    frame[11] = vstax_lo >>  0;

    return VTSS_RC_OK;
}

static vtss_rc jr_vstax_frame2header(const vtss_state_t      *const state,
                                     const u8                *const frame,
                                     vtss_vstax_rx_header_t  *const vstax)
{
    u32 vstax_hi = 0;
    const u8 *ifh = frame;

    /* 4 bytes to vstax_hi */
    vstax_hi <<= 8; vstax_hi += *ifh++;
    vstax_hi <<= 8; vstax_hi += *ifh++;
    vstax_hi <<= 8; vstax_hi += *ifh++;
    vstax_hi <<= 8; vstax_hi += *ifh++;

    memset(vstax, 0, sizeof(*vstax));

    /* Valid IFH */
    vstax->valid =
        (VTSS_EXTRACT_BITFIELD(vstax_hi, 16, 16) == VTSS_ETYPE_VTSS) &&
        VTSS_EXTRACT_BITFIELD(vstax_hi, 15, 1);

    /* Decode rest if OK */
    if(vstax->valid) {
        u64 vstax_lo = 0;

        vstax->isdx = VTSS_EXTRACT_BITFIELD(vstax_hi, 0, 12);

        /* 8 bytes to vstax_lo */
        vstax_lo <<= 8; vstax_lo += *ifh++;
        vstax_lo <<= 8; vstax_lo += *ifh++;
        vstax_lo <<= 8; vstax_lo += *ifh++;
        vstax_lo <<= 8; vstax_lo += *ifh++;
        vstax_lo <<= 8; vstax_lo += *ifh++;
        vstax_lo <<= 8; vstax_lo += *ifh++;
        vstax_lo <<= 8; vstax_lo += *ifh++;
        vstax_lo <<= 8; vstax_lo += *ifh++;

        vstax->sp = VTSS_EXTRACT_BITFIELD64(vstax_lo, 59, 1);

        if(VTSS_EXTRACT_BITFIELD64(vstax_lo, 10, 1)) {
            /* GLAG */
            vstax->port_no = VTSS_PORT_NO_NONE;
            vstax->glag_no = VTSS_EXTRACT_BITFIELD64(vstax_lo, 0, 5);
        } else {
            /* Port */
            vstax->port_no = vtss_jr1_vtss_pgid(state, VTSS_EXTRACT_BITFIELD64(vstax_lo, 0,  5));
            vstax->upsid   = VTSS_EXTRACT_BITFIELD64(vstax_lo, 5,  5);
            vstax->glag_no = VTSS_GLAG_NO_NONE;
        }
    }

    return VTSS_RC_OK;
}
#endif /* VTSS_FEATURE_VSTAX_V2 */

static vtss_rc jr_rx_hdr_decode(const vtss_state_t          *const state,
                                const vtss_packet_rx_meta_t *const meta,
                                const u8                           xtr_hdr[VTSS_PACKET_HDR_SIZE_BYTES],
                                      vtss_packet_rx_info_t *const info)
{
    u16                 vstax_hi;
    u64                 vstax_lo, fwd;
    BOOL                sflow_marked;
    vtss_phys_port_no_t chip_port;
    vtss_trace_group_t  trc_grp = meta->no_wait ? VTSS_TRACE_GROUP_FDMA_IRQ : VTSS_TRACE_GROUP_PACKET;

    VTSS_DG(trc_grp, "IFH (24 bytes) + bit of packet:");
    VTSS_DG_HEX(trc_grp, &xtr_hdr[0], 64);

    vstax_hi =                                                       ((u16)xtr_hdr[ 8] <<  8) | ((u16)xtr_hdr[ 9] <<  0);
    vstax_lo = ((u64)xtr_hdr[10] << 56) | ((u64)xtr_hdr[11] << 48) | ((u64)xtr_hdr[12] << 40) | ((u64)xtr_hdr[13] << 32) |
               ((u64)xtr_hdr[14] << 24) | ((u64)xtr_hdr[15] << 16) | ((u64)xtr_hdr[16] <<  8) | ((u64)xtr_hdr[17] <<  0);
    fwd      =                                                       ((u64)xtr_hdr[18] << 40) | ((u64)xtr_hdr[19] << 32) |
               ((u64)xtr_hdr[20] << 24) | ((u64)xtr_hdr[21] << 16) | ((u64)xtr_hdr[22] <<  8) | ((u64)xtr_hdr[23] <<  0);

    // The VStaX header's MSbit must be 1.
    if (VTSS_EXTRACT_BITFIELD(vstax_hi, 15, 1) != 1) {
        VTSS_EG(trc_grp, "Invalid Rx header signature");
        return VTSS_RC_ERROR;
    }

    memset(info, 0, sizeof(*info));

    info->sw_tstamp = meta->sw_tstamp;
    info->length    = meta->length;

    chip_port = VTSS_EXTRACT_BITFIELD64(fwd, 29, 6);
    info->port_no = vtss_cmn_chip_to_logical_port(state, meta->chip_no, chip_port);

//     VTSS_IG(trc_grp, "Received on xtr_qu = %u, chip_no = %d, chip_port = %u, port_no = %u", meta->xtr_qu, meta->chip_no, chip_port, info->port_no);

#if defined(VTSS_FEATURE_AGGR_GLAG)
    info->glag_no = state->l2.port_glag_no[info->port_no];
#else
    info->glag_no = VTSS_GLAG_NO_NONE;
#endif

    // FIXME: Must support internal ports.

#ifdef VTSS_FEATURE_VSTAX
    if (VTSS_EXTRACT_BITFIELD64(fwd, 45, 1)) {
        // Received with VStaX header. Decode it.
        u8 vstax_hdr_bin[VTSS_VSTAX_HDR_SIZE];

        vstax_hdr_bin[0] = (VTSS_ETYPE_VTSS >> 8) & 0xFF;
        vstax_hdr_bin[1] = (VTSS_ETYPE_VTSS >> 0) & 0xFF;
        memcpy(&vstax_hdr_bin[2], &xtr_hdr[8], VTSS_VSTAX_HDR_SIZE - 2);
        (void)jr_vstax_frame2header(state, vstax_hdr_bin, &info->vstax);
        info->glag_no = info->vstax.glag_no;
    }
#endif

    sflow_marked = VTSS_EXTRACT_BITFIELD64(fwd, 40, 1);
    if (sflow_marked) {
        // This is only reliable if ANA_AC:PS_COMMON:PS_COMMON_CFG.SFLOW_SMPL_ID_IN_STAMP_ENA is set to 1.
        u32 sflow_id        = (meta->fcs >> 26) & 0x3F; // Indicates physical port number.
        info->sflow_type    = chip_port == sflow_id ? VTSS_SFLOW_TYPE_RX : VTSS_SFLOW_TYPE_TX;
        info->sflow_port_no = vtss_cmn_chip_to_logical_port(state, meta->chip_no, sflow_id);
    } else {
        // Get the timestamp from the FCS.
        info->hw_tstamp         = meta->fcs;
        info->hw_tstamp_decoded = TRUE;
    }

    if (VTSS_EXTRACT_BITFIELD64(fwd, 27, 1) && meta->xtr_qu < 8) {
        // If classification_result[9] == 1, classification_result[7:0] == xtr_qu_mask for 8 non-super-priority queues.
        info->xtr_qu_mask = VTSS_EXTRACT_BITFIELD64(fwd, 18, 8);
    } else {
        // If classification_result[9] == 0, classification_result[8:0] == acl_idx for non-default VCAP IS2 rule.
        // If the frame is received on a super-priority queue, the FDMA has created an artificial IFH, and therefore
        // we have to construct the xtr_qu_mask from that.
        info->xtr_qu_mask = 1 << meta->xtr_qu;

        if (meta->xtr_qu < 8) {
            // Hit a non-default VCAP IS2 rule.
            info->acl_hit = TRUE;
            info->acl_idx = VTSS_EXTRACT_BITFIELD64(fwd, 18, 9);

            // We may also set the sflow queue in the queue mask.
            if (sflow_marked && state->packet.rx_conf.map.sflow_queue != VTSS_PACKET_RX_QUEUE_NONE) {
                info->xtr_qu_mask |= (1 << state->packet.rx_conf.map.sflow_queue);
            }
        }
    }

    info->tag.pcp = VTSS_EXTRACT_BITFIELD64(vstax_lo, 29,  3);
    info->tag.dei = VTSS_EXTRACT_BITFIELD64(vstax_lo, 28,  1);
    info->tag.vid = VTSS_EXTRACT_BITFIELD64(vstax_lo, 16, 12);
    info->cos     = VTSS_EXTRACT_BITFIELD64(vstax_lo, 56,  3);

#if defined(VTSS_FEATURE_VSTAX)
    // If it's received on a stack port, we gotta look into the VStaX header to find a tag type on the original port.
    // Unfortunately, a possible tag was stripped on egress of the stack port of the originating switch, so that we
    // can't determine whether it originally arrived on an S- or an S-custom port. We classify it as an S-port in that
    // case. Also, we can't set any of the info->hints flags, because we don't have info about the original
    // ingress port's setup.
    // sFlow frames received through interconnects on secondary device will have info->port_no == VTSS_PORT_NO_NONE,
    // which is not supported by vtss_cmn_packet_hints_update(), so we capture them below.
    if ((state->chip_count == 2 && info->port_no == VTSS_PORT_NO_NONE) || info->port_no == state->l2.vstax_conf.port_0 || info->port_no == state->l2.vstax_conf.port_1) {
        if (VTSS_EXTRACT_BITFIELD64(vstax_lo, 15, 1)) {
            // Frame was originally tagged. Figure out whether it was a C or an S(-custom) tag.
            info->tag_type = VTSS_EXTRACT_BITFIELD64(vstax_lo, 14, 1) ? VTSS_TAG_TYPE_S_TAGGED : VTSS_TAG_TYPE_C_TAGGED;
            info->tag.tpid = info->tag_type == VTSS_TAG_TYPE_S_TAGGED ? VTSS_ETYPE_TAG_S : VTSS_ETYPE_TAG_C; // Application shouldn't rely on this.
        }
    } else
#endif
    {
        VTSS_RC(vtss_cmn_packet_hints_update(state, trc_grp, meta->etype, info));
    }

    return VTSS_RC_OK;
}

static vtss_rc jr_tx_hdr_encode_internal(const vtss_state_t          *const state,
                                         const vtss_packet_tx_info_t *const info,
                                               u8                    *const bin_hdr,
                                               u32                   *const bin_hdr_len,
                                               vtss_packet_tx_grp_t  *const inj_grp)
{
    u64  dst, vstax_lo, fwd;
    u16  vstax_hi;
    BOOL contains_stack_hdr;

    if (bin_hdr == NULL) {
        // Caller wants us to return the number of bytes required to fill
        // in #bin_hdr. We need 24 bytes for the IFH.
        *bin_hdr_len = 24;
        return VTSS_RC_OK;
    } else if (*bin_hdr_len < 24) {
        return VTSS_RC_ERROR;
    }

    *bin_hdr_len = 24;

#if defined(VTSS_FEATURE_VSTAX)
   contains_stack_hdr = info->tx_vstax_hdr != VTSS_PACKET_TX_VSTAX_NONE;
#else
   contains_stack_hdr = FALSE;
#endif

    dst      = 0;
    vstax_hi = 0;
    vstax_lo = VTSS_ENCODE_BITFIELD64(1, 55, 1); // ingr_drop_mode = 1 => don't make head-of-line blocking
    fwd      = VTSS_ENCODE_BITFIELD64(1, 44, 1); // Enforce update of FCS.
    *inj_grp = 0; // Default to injecting non-super prio.

    if (!info->switch_frm) {
        // Most of the stuff that needs to be filled in is when we're not switching the frame.
        // First figure out which ports the caller wants to inject to.
        u64            chip_port_mask;
        vtss_chip_no_t chip_no;
        vtss_port_no_t stack_port_no;
        u32            port_cnt;
        vtss_port_no_t port_no; /* Only valid if port_cnt == 1 */

        VTSS_RC(vtss_cmn_logical_to_chip_port_mask(state, info->dst_port_mask, &chip_port_mask, &chip_no, &stack_port_no, &port_cnt, &port_no));

        if (contains_stack_hdr) {
#if defined(VTSS_FEATURE_VSTAX)
            // Stack header is already filled in by the user. Make sure the selected port is a stack port.
            if (stack_port_no == VTSS_PORT_NO_NONE) {
                VTSS_EG(VTSS_TRACE_GROUP_PACKET, "Injecting with stack header to non-stack port");
                return VTSS_RC_ERROR;
            }

            if (info->tx_vstax_hdr == VTSS_PACKET_TX_VSTAX_SYM) {
                // The following function also inserts the EtherType, hence the "-2" from the correct VStaX header position within the IFH.
                VTSS_RC(jr_vstax_header2frame(state, stack_port_no, &info->vstax.sym, &bin_hdr[8 - 2]));
            } else {
                // Copy the user-defined stack header to the binary header. Skip EtherType.
                memcpy(bin_hdr + 8, info->vstax.bin + 2, VTSS_VSTAX_HDR_SIZE - 2);
            }

            fwd |= VTSS_ENCODE_BITFIELD64(1, 45, 1); // FWD.VSTAX_AVAIL = 1

            if (info->cos >= 8) {
                // Super priority injection. Modify injection group to either 2 (stack left) or 3 (stack right).
                // When doing SP injection stack port injection, most of the non-stack-hdr-part of the IFH is ignored.
                if (state->chip_count == 2) {
                    // In 48-port stackable solutions, transmit on injection group 3, which hits the interconnect
                    // port, if the frame must go out on the external stack port on the secondary device.
                    *inj_grp = state->port.map[stack_port_no].chip_no == 1 ? 3 : 2;
                } else {
                    // In 24-port stackable solutions, transmit on injection group 3
                    // if the stack port number matches stack right.
                    *inj_grp = stack_port_no == state->l2.vstax_conf.port_1 ? 3 : 2;
                }
            } else {
                // Non-super-priority injection with a stack header. Leave the injection group at 0.
                // If the stack port is on the secondary device, and we're running on the on-chip CPU,
                // we reach the stack port by injecting to the interconnect port.
                // An external CPU may or may not want to inject through the interconnect port.
                // What it does depends on whether it has one or two NPI ports, or - if it uses the register interface -
                // has one to each of the two devices, or only one to the primary.
                fwd |= VTSS_ENCODE_BITFIELD64(3, 16, 2); // INJECT_MULTICAST (with only one port).
#if VTSS_OPT_VCORE_III
                if (chip_no == 1) {
                    // Injecting to stack port on secondary device.
                    // Do this through the interconnect port on the primary device.
                    chip_port_mask = VTSS_BIT64(state->port.port_int_0);
                }
#endif
                dst |= VTSS_ENCODE_BITFIELD64(chip_port_mask,         0, 32);
                fwd |= VTSS_ENCODE_BITFIELD64(info->latch_timestamp, 46,  2); // FWD.CAPTURE_TSTAMP
                fwd |= VTSS_ENCODE_BITFIELD64(1,                     28,  1); // FWD.DO_NOT_REW
            }
#endif
        } else {
            // A stack header is not already prefilled by the user.
            if (stack_port_no != VTSS_PORT_NO_NONE) {
                // When injecting to a stack port, the user must already have filled in the VStaX header.
                VTSS_EG(VTSS_TRACE_GROUP_PACKET, "Injecting without a stack header to a stack port");
                return VTSS_RC_ERROR;
            }

            if (info->cos >= 8) {
                // User wants to inject on super-priority to a front-port.
                // As stated in the API, this only works on the primary device (if running on on-chip CPU) and
                // with a single port at a time.
                if (port_cnt != 1) {
                    VTSS_EG(VTSS_TRACE_GROUP_PACKET, "SP FP inj. only supported one port at a time");
                    return VTSS_RC_ERROR;
                }

#if VTSS_OPT_VCORE_III
                // If using the on-chip CPU, we cannot inject super-priority to a front port
                // on the secondary device, because we'd have to go through one of the interconnect ports, after
                // which SP FP injection is impossible.
                if (chip_no != 0) {
                    VTSS_EG(VTSS_TRACE_GROUP_PACKET, "Can't do SP FP inj. on sec. device");
                    return VTSS_RC_ERROR;
                }
#endif

                fwd |= VTSS_ENCODE_BITFIELD64(2, 16, 2); // INJECT_UNICAST
                dst |= VTSS_ENCODE_BITFIELD64((u32)state->port.map[port_no].chip_port, 0, 8);
                *inj_grp = 4; // Inject on pront port, super-prio group
            } else {
                // Injecting non-super-priority to one or more front ports on either the primary or the secondary device.
                // If injecting to front ports on the secondary device, only one port is supported at a time (if running on the on-chip
                // CPU; an external may have direct access to the secondary device). In this case, it's up to e.g. the FDMA driver to inject multiple times.
#if defined(VTSS_FEATURE_VSTAX) && VTSS_OPT_VCORE_III
                if (chip_no == 1) {
                    vtss_vstax_tx_header_t vs_hdr;

                    if (port_cnt != 1) {
                        VTSS_EG(VTSS_TRACE_GROUP_PACKET, "Can't inj. to multiple ports on the secondary device (mask = 0x%" PRIx64 ")", info->dst_port_mask);
                        return VTSS_RC_ERROR;
                    }

                    // Otherwise, we synthesize a VStaX header and send the frame to the interconnect port.
                    memset(&vs_hdr, 0, sizeof(vs_hdr));
                    vs_hdr.fwd_mode  = VTSS_VSTAX_FWD_MODE_UPSID_PORT;
                    vs_hdr.ttl       = 1;
                    vs_hdr.prio      = info->cos >= 8 ? 7 : info->cos;
                    vs_hdr.upsid     = state->l2.vstax_info.upsid[chip_no];
                    vs_hdr.port_no   = port_no;
                    vs_hdr.glag_no   = VTSS_GLAG_NO_NONE; // Disable GLAG source filtering.
                    VTSS_RC(jr_vstax_header2frame(state, VTSS_PORT_NO_NONE, &vs_hdr, &bin_hdr[8 - 2])); // That function also inserts the EtherType, hence the "-2" from the correct VStaX header position within the IFH.
                    contains_stack_hdr = TRUE; // Prevent overwriting further down.
                    chip_port_mask = VTSS_BIT64(state->port.port_int_0); // Transmit on interconnect port.
                    // Leave injection group as 0.
                }
#endif
                fwd |= VTSS_ENCODE_BITFIELD64(3, 16, 2); // INJECT_MULTICAST
                dst |= VTSS_ENCODE_BITFIELD64((u32)chip_port_mask, 0, 32);
            }

            fwd |= VTSS_ENCODE_BITFIELD64(info->latch_timestamp, 46,  2); // FWD.CAPTURE_TSTAMP
            fwd |= VTSS_ENCODE_BITFIELD64(1,                     28,  1); // FWD.DO_NOT_REW
            fwd |= VTSS_ENCODE_BITFIELD64(1,                     45,  1); // FWD.VSTAX_AVAIL = 1

            // We can get here even with a stack header, in the case the user doesn't inject into
            // a super-priority group (i.e. inj_grp != 2 && inj_grp != 3).
            if (!contains_stack_hdr) {
                // We don't have a VLAN tag to get the switch determine the QoS class from,
                // so we must use the VStaX header (props->port_mask must not include stack ports)
                // to convey the information (this is not possible when sending switched).
                vstax_hi  = VTSS_ENCODE_BITFIELD  (1,         15, 1); // MSBit must be 1
                vstax_lo |= VTSS_ENCODE_BITFIELD64(info->cos >= 8 ? 7 : info->cos, 56, 3); // qos_class/iprio (internal priority)

            }
        }
    }

    bin_hdr[0] = dst >> 56;
    bin_hdr[1] = dst >> 48;
    bin_hdr[2] = dst >> 40;
    bin_hdr[3] = dst >> 32;
    bin_hdr[4] = dst >> 24;
    bin_hdr[5] = dst >> 16;
    bin_hdr[6] = dst >>  8;
    bin_hdr[7] = dst >>  0;
    if (!contains_stack_hdr) {
        bin_hdr[ 8] = vstax_hi >>  8;
        bin_hdr[ 9] = vstax_hi >>  0;
        bin_hdr[10] = vstax_lo >> 56;
        bin_hdr[11] = vstax_lo >> 48;
        bin_hdr[12] = vstax_lo >> 40;
        bin_hdr[13] = vstax_lo >> 32;
        bin_hdr[14] = vstax_lo >> 24;
        bin_hdr[15] = vstax_lo >> 16;
        bin_hdr[16] = vstax_lo >>  8;
        bin_hdr[17] = vstax_lo >>  0;
    }
    bin_hdr[18] = fwd >> 40;
    bin_hdr[19] = fwd >> 32;
    bin_hdr[20] = fwd >> 24;
    bin_hdr[21] = fwd >> 16;
    bin_hdr[22] = fwd >>  8;
    bin_hdr[23] = fwd >>  0;

    if (info->cos >= 8) {
        // When injecting stack-frames through the super-priority queues,
        // or frames through the front port super-priority queue,
        // the IFH checksum must be correct, because the check in the
        // chip (DSM) cannot be disabled, as it can for non-SP-transmitted
        // frames (through ASM::CPU_CFG.CPU_CHK_IFH_CHKSUM_ENA).
        // This is computed over the entire IFH as BIP-8 (BIP8).
        u8 chksum = 0;
        u8 *p = &bin_hdr[0];
        int i;
        for (i = 0; i < 24; i++) {
            chksum ^= *(p++);
        }
        bin_hdr[23] = chksum;
    }

    VTSS_IG(VTSS_TRACE_GROUP_PACKET, "IFH:");
    VTSS_IG_HEX(VTSS_TRACE_GROUP_PACKET, &bin_hdr[0], *bin_hdr_len);

    return VTSS_RC_OK;
}

static vtss_rc jr_tx_hdr_encode(      vtss_state_t          *const state,
                                const vtss_packet_tx_info_t *const info,
                                      u8                    *const bin_hdr,
                                      u32                   *const bin_hdr_len)
{
    vtss_packet_tx_grp_t inj_grp;
    return jr_tx_hdr_encode_internal(state, info, bin_hdr, bin_hdr_len, &inj_grp);
}

/* - Debug print --------------------------------------------------- */

static vtss_rc jr_debug_pkt(vtss_state_t *vtss_state,
                            const vtss_debug_printf_t pr,
                            const vtss_debug_info_t   *const info)
{
    u32                 port, i;
    char                buf[32];
    vtss_chip_counter_t cnt;

    /* Rx registration registers */
    for (port = 0; port < VTSS_CHIP_PORTS_ALL; port++) {
        sprintf(buf, "Port %u", port);
        vtss_jr1_debug_reg_header(pr, buf);
        JR_DEBUG_REGX_NAME(pr, ANA_CL_2, PORT_CAPTURE_CFG, port, "CAPTURE_CFG");
        JR_DEBUG_REGX_NAME(pr, ANA_CL_2, PORT_CAPTURE_GXRP_CFG, port, "GXRP_CFG");
        JR_DEBUG_REGX_NAME(pr, ANA_CL_2, PORT_CAPTURE_BPDU_CFG, port, "BPDU_CFG");
        pr("\n");
    }

    /* Rx queue mapping registers */
    for (i = 0; i < 16; i++) {
        sprintf(buf, "GARP/BPDU %u", i);
        vtss_jr1_debug_reg_header(pr, buf);
        JR_DEBUG_REGX_NAME(pr, ANA_CL_2, COMMON_CPU_BPDU_QU_CFG, i, "BPDU_QU_CFG");
        pr("\n");
    }
#if defined(VTSS_FEATURE_NPI)
    vtss_jr1_debug_reg_header(pr, "NPI");
    JR_DEBUG_REG_NAME(pr, ARB, CFG_STATUS_STACK_CFG, "ARB:STACK_CFG");
    pr("\n");
#endif

    vtss_jr1_debug_reg_header(pr, "COMMON");
    JR_DEBUG_REG_NAME(pr, ANA_CL_2, COMMON_CPU_PROTO_QU_CFG, "ANA_CL:PROT_QU_CFG");
    JR_DEBUG_REG_NAME(pr, ANA_L2, COMMON_LRN_CFG, "ANA_L2:LRN_CFG");
    JR_DEBUG_REG_NAME(pr, ANA_L2, COMMON_FWD_CFG, "ANA_L2:FWD_CFG");
    pr("\n");
    
    /* IQS counters */
    for (port = VTSS_CHIP_PORT_CPU_0; port <= VTSS_CHIP_PORT_CPU_1; port++) {
        pr("CPU Tx counters (IQS) for port %u:\n\n", port);
        for (i = 0; i < VTSS_PRIOS; i++) {
            sprintf(buf, "green_%u", i);
            JR_CNT_SQS(IQS, RX, GREEN, port, i, &cnt, 0); 
            vtss_jr1_debug_cnt(vtss_state, pr, NULL, buf, NULL, &cnt);
        }
        for (i = 0; i < VTSS_PRIOS; i++) {
            sprintf(buf, "yellow_%u", i);
            JR_CNT_SQS(IQS, RX, YELLOW, port, i, &cnt, 0); 
            vtss_jr1_debug_cnt(vtss_state, pr, NULL, buf, NULL, &cnt);
        }
        for (i = 0; i < VTSS_PRIOS; i++) {
            sprintf(buf, "drops_%u", i);
            JR_CNT_SQS(IQS, RX, QDROPS, port, i, &cnt, 0); 
            vtss_jr1_debug_cnt(vtss_state, pr, NULL, buf, NULL, &cnt);
        }
        for (i = 0; i < VTSS_PRIOS; i++) {
            sprintf(buf, "pol_drops_%u", i);
            JR_CNT_SQS(IQS, RX, PDROPS, port, i, &cnt, 0);
            vtss_jr1_debug_cnt(vtss_state, pr, NULL, buf, NULL, &cnt);
        }
        pr("\n");
    }

    /* OQS counters */
    port = (456/8); /* Map to queue 456-463 */
    pr("CPU Rx counters (OQS)\n\n");
    for (i = 0; i < VTSS_PRIOS; i++) {
        sprintf(buf, "green_%u", i);
        JR_CNT_SQS(OQS, TX, GREEN, port, i, &cnt, 0); 
        vtss_jr1_debug_cnt(vtss_state, pr, buf, NULL, &cnt, NULL);
    }
    for (i = 0; i < VTSS_PRIOS; i++) {
        sprintf(buf, "yellow_%u", i);
        JR_CNT_SQS(OQS, TX, YELLOW, port, i, &cnt, 0); 
        vtss_jr1_debug_cnt(vtss_state, pr, buf, NULL, &cnt, NULL);
    }
    for (i = 0; i < VTSS_PRIOS; i++) {
        sprintf(buf, "drops_%u", i);
        JR_CNT_SQS(OQS, RX, QDROPS, port, i, &cnt, 0); 
        vtss_jr1_debug_cnt(vtss_state, pr, buf, NULL, &cnt, NULL);
    }
    pr("\n");
    
    /* ASM registers */
    vtss_jr1_debug_reg_header(pr, "ASM");
    for (i = 0; i < 2; i++) {
        JR_DEBUG_REGX(pr, ASM, CFG_CPU_CFG, i);
    }
    for (i = 0; i < JR_STACK_PORT_CNT; i++) {
        JR_DEBUG_REGX_NAME(pr, ASM, SP_CFG_SP_RX_CFG, i, "SP_RX_CFG");
    }
    JR_DEBUG_REG_NAME(pr, ASM, SP_CFG_SP_UPSID_CFG, "SP_UPSID_CFG");
    JR_DEBUG_REG_NAME(pr, ASM, SP_CFG_SP_KEEP_TTL_CFG, "SP_KEEP_TTL_CFG");
    pr("\n");
    
    /* DSM registers */
    vtss_jr1_debug_reg_header(pr, "DSM");
    JR_DEBUG_REG_NAME(pr, DSM, SP_CFG_SP_KEEP_TTL_CFG, "SP_KEEP_TTL_CFG");
    for (i = 0; i < JR_STACK_PORT_CNT; i++) {
        JR_DEBUG_REGX_NAME(pr, DSM, SP_CFG_SP_TX_CFG, i, "SP_TX_CFG");
    }    
    JR_DEBUG_REG_NAME(pr, DSM, SP_CFG_SP_XTRCT_CFG, "SP_XTRCT_CFG");
    JR_DEBUG_REG_NAME(pr, DSM, SP_CFG_SP_FRONT_PORT_INJ_CFG, "SP_PORT_INJ_CFG");
    JR_DEBUG_REG_NAME(pr, DSM, SP_CFG_SP_INJ_CFG, "SP_INJ_CFG");
    JR_DEBUG_REG_NAME(pr, DSM, SP_STATUS_SP_FRONT_PORT_INJ_STAT, "SP_PORT_INJ_STAT");
    for (i = 0; i < JR_STACK_PORT_CNT; i++) {
        JR_DEBUG_REGX_NAME(pr, DSM, SP_STATUS_SP_FRAME_CNT, i, "SP_FRAME_CNT");
    }    
    pr("\n");

    /* Packet extraction registers */
    vtss_jr1_debug_reg_header(pr, "Extraction");
    for (i = 0; i < JR_PACKET_RX_QUEUE_CNT; i++)
        JR_DEBUG_REGX_NAME(pr, DEVCPU_QS, XTR_XTR_FRM_PRUNING, i, "FRM_PRUNING");
    for (i = 0; i < JR_PACKET_RX_GRP_CNT; i++)
        JR_DEBUG_REGX_NAME(pr, DEVCPU_QS, XTR_XTR_GRP_CFG, i, "GRP_CFG");
    for (i = 0; i < JR_PACKET_RX_QUEUE_CNT; i++)
        JR_DEBUG_REGX_NAME(pr, DEVCPU_QS, XTR_XTR_MAP, i, "MAP");
    JR_DEBUG_REG_NAME(pr, DEVCPU_QS, XTR_XTR_DATA_PRESENT, "DATA_PRESENT");
    JR_DEBUG_REG_NAME(pr, DEVCPU_QS, XTR_XTR_QU_DBG, "QU_DBG");
    pr("\n");

    /* Packet injection registers */
    vtss_jr1_debug_reg_header(pr, "Injection");
    for (i = 0; i < JR_PACKET_TX_GRP_CNT; i++)
        JR_DEBUG_REGX_NAME(pr, DEVCPU_QS, INJ_INJ_GRP_CFG, i, "GRP_CFG");
    for (i = 0; i < JR_PACKET_TX_GRP_CNT; i++)
        JR_DEBUG_REGX_NAME(pr, DEVCPU_QS, INJ_INJ_CTRL, i, "CTRL");
    for (i = 0; i < JR_PACKET_TX_GRP_CNT; i++)
        JR_DEBUG_REGX_NAME(pr, DEVCPU_QS, INJ_INJ_ERR, i, "ERR");
    JR_DEBUG_REG_NAME(pr, DEVCPU_QS, INJ_INJ_STATUS, "STATUS");
    pr("\n");

    return VTSS_RC_OK;
}

vtss_rc vtss_jr1_packet_debug_print(vtss_state_t *vtss_state,
                                    const vtss_debug_printf_t pr,
                                    const vtss_debug_info_t   *const info)
{
    VTSS_RC(vtss_debug_print_group(VTSS_DEBUG_GROUP_PACKET, jr_debug_pkt, vtss_state, pr, info));
#if defined(VTSS_FEATURE_FDMA) && VTSS_OPT_FDMA
    if (vtss_debug_group_enabled(pr, info, VTSS_DEBUG_GROUP_FDMA)) {
        VTSS_FUNC_RC(fdma_state.fdma_func.fdma_debug_print, pr, info);
    }
#endif 
    return VTSS_RC_OK;
}

/* - Initialization ------------------------------------------------ */

static vtss_rc jr_packet_init(vtss_state_t *vtss_state)
{
    u32 i;
    int pcp, dei;

    /* Default extraction mode is little endian and status word before last data. Include queue number before IFH */
    for (i = 0; i < JR_PACKET_RX_GRP_CNT; i++) {
        JR_WRX(DEVCPU_QS, XTR_XTR_GRP_CFG, i, 
               JR_PUT_BIT(DEVCPU_QS, XTR_XTR_GRP_CFG, BYTE_SWAP, 1) |
               JR_PUT_BIT(DEVCPU_QS, XTR_XTR_GRP_CFG, STATUS_WORD_POS, 0) |
               JR_PUT_BIT(DEVCPU_QS, XTR_XTR_GRP_CFG, XTR_HDR_ENA, 1));
        /* By default CPU Rx queues are extracted in a round-robin fashion.
         * We need to change that to strict priority, so that higher queue
         * numbers are extracted prior to lower queue numbers.
         */
        JR_WRX(DEVCPU_QS, XTR_XTR_QU_SEL, i, VTSS_F_DEVCPU_QS_XTR_XTR_QU_SEL_QU_SCH(1));
    }

    /* Make the chip store the CPU Rx queue mask (i.e. the reasons for forwarding the frame to the CPU) in the extraction IFH */
    JR_WRM_SET(ANA_AC, PS_COMMON_MISC_CTRL, VTSS_F_ANA_AC_PS_COMMON_MISC_CTRL_CPU_DEST_MASK_IN_CL_RES_ENA);

    /* Enable ability to perform super priority injection on front ports (supported by FDMA driver)
     * Disobey flow control, since that will cause the whole system to stop working if it happens. */
    JR_WR(DSM, SP_CFG_SP_FRONT_PORT_INJ_CFG,
          JR_PUT_BIT(DSM, SP_CFG_SP_FRONT_PORT_INJ_CFG, SP_FRONT_PORT_REGARD_LLFC, 0) |
          JR_PUT_BIT(DSM, SP_CFG_SP_FRONT_PORT_INJ_CFG, SP_FRONT_PORT_TX_ENA, 1));

    /* The super priority queue watermarks must be set to CPU MTU + various header sizes +
     * latency from flow control assertion until FDMA reaction (register watermark
     * granularity is 16 bytes). The maximum is 99.
     */
    JR_WR(DSM, SP_CFG_SP_INJ_CFG, 95);

    /* Disable CPU frame aging in ASM */
    for (i = 0; i < 2; i++) {
        JR_WRXB(ASM, CFG_CPU_CFG, i, CPU_FRM_AGING_DIS, 1);
    }

    for (i = VTSS_CHIP_PORT_CPU_0; i <= VTSS_CHIP_PORT_CPU_1; i++) {
        /* Enable stacking on CPU ports for VLAN classification purposes */
        JR_WRXB(ANA_CL_2, PORT_STACKING_CTRL, i, STACKING_AWARE_ENA, 1);
        JR_WRXB(ANA_CL_2, PORT_STACKING_CTRL, i, STACKING_HEADER_DISCARD_ENA, 0);

        /* VLAN popping is done using default actions in IS0 */
        VTSS_RC(vtss_jr1_vcap_port_command(vtss_state, VTSS_TCAM_IS0, i, VTSS_TCAM_CMD_READ));
        JR_WRF(VCAP_IS0, BASETYPE_ACTION_A, VLAN_POP_CNT, 1);
        VTSS_RC(vtss_jr1_vcap_port_command(vtss_state, VTSS_TCAM_IS0, i, VTSS_TCAM_CMD_WRITE));

        /* Set CPU ports to be VLAN aware, since frames that we send switched
         * must contain a VLAN tag for correct classification. One could use
         * the frame's VStaX header, but that won't work for stacking solutions */
        JR_WRXM(ANA_CL_2, PORT_VLAN_CTRL, i,
                VTSS_F_ANA_CL_2_PORT_VLAN_CTRL_PORT_VID(0) |
                VTSS_F_ANA_CL_2_PORT_VLAN_CTRL_VLAN_AWARE_ENA,
                VTSS_M_ANA_CL_2_PORT_VLAN_CTRL_PORT_VID |
                VTSS_F_ANA_CL_2_PORT_VLAN_CTRL_VLAN_AWARE_ENA);

        /* In order for switched frames to be classified to a user-module-specified
         * QoS class, we use the VLAN tag's PCP and DEI bits to get it classified.
         * For this to work, the CPU port(s) must be enabled for PCP/DEI-to-QoS
         * class classification. Below, we set-up a one-to-one-mapping between
         * PCP value and QoS class independent of value of DEI.
         */
        JR_WRXB(ANA_CL_2, PORT_QOS_CFG, i, UPRIO_QOS_ENA,      1);
        JR_WRXB(ANA_CL_2, PORT_QOS_CFG, i, UPRIO_DP_ENA,       1);
        for (pcp = 0; pcp < VTSS_PCP_END - VTSS_PCP_START; pcp++) {
            for (dei = 0; dei < VTSS_DEI_END - VTSS_DEI_START; dei++) {
                JR_WRXYF(ANA_CL_2, PORT_UPRIO_MAP_CFG, i, (8 * dei + pcp), UPRIO_CFI_TRANSLATE_QOS_VAL, vtss_cmn_pcp2qos(pcp));
                JR_WRXYF(ANA_CL_2, PORT_DP_CONFIG,     i, (8 * dei + pcp), UPRIO_CFI_DP_VAL,            0);
            }
        }
    }

    VTSS_RC(vtss_jr1_is2_cpu_port_setup(vtss_state));

    return VTSS_RC_OK;
}

vtss_rc vtss_jr1_packet_init(vtss_state_t *vtss_state, vtss_init_cmd_t cmd)
{
    vtss_packet_state_t *state = &vtss_state->packet;
    
    switch (cmd) {
    case VTSS_INIT_CMD_CREATE:
        state->rx_conf_set      = jr_rx_conf_set;
        state->rx_frame_get     = jr_rx_frame_get;
        state->rx_frame_discard = jr_rx_frame_discard;
        state->tx_frame_port    = jr_tx_frame_port;
        state->rx_hdr_decode    = jr_rx_hdr_decode;
        state->rx_ifh_size      = VTSS_JR1_RX_IFH_SIZE;
        state->tx_hdr_encode    = jr_tx_hdr_encode;
#if defined(VTSS_FEATURE_VSTAX_V2)
        state->vstax_header2frame = jr_vstax_header2frame;
        state->vstax_frame2header = jr_vstax_frame2header;
#endif /* VTSS_FEATURE_VSTAX_V2 */
#if defined(VTSS_FEATURE_NPI)
        state->npi_conf_set = jr_npi_conf_set;
#endif  /* VTSS_FEATURE_NPI */
        state->rx_queue_count = JR_PACKET_RX_QUEUE_CNT;

#if defined(VTSS_FEATURE_FDMA) && VTSS_OPT_FDMA
#if VTSS_OPT_FDMA_VER > 2
        /* The FDMA needs a special version of the tx_hdr_encode() function,
         * which can also return the injection group. */
        vtss_state->fdma_state.tx_hdr_encode_internal = jr_tx_hdr_encode_internal;
#endif
        vcoreiii_fdma_func_init(vtss_state);
#endif
        break;
    case VTSS_INIT_CMD_INIT:
        VTSS_RC(jr_packet_init(vtss_state));
        break;
    case VTSS_INIT_CMD_PORT_MAP:
#if defined(VTSS_FEATURE_VSTAX_V2)
#if defined(VTSS_FEATURE_FDMA) && VTSS_OPT_FDMA
        /* The FDMA needs a function pointer to a function that can be used to manually read the secondary chip. */
        vtss_state->fdma_state.rx_frame_get_internal     = jr_rx_frame_get_internal;
        /* Also needs a function pointer to a function that can be used to discard frames on the secondary chip. */
        vtss_state->fdma_state.rx_frame_discard_internal = jr_rx_frame_discard_internal;
#endif
#endif /* VTSS_FEATURE_VSTAX_V2 */

        /* Setup Rx registrations and queue mappings */
        VTSS_RC(jr_rx_conf_set(vtss_state));
        break;
    default:
        break;
    }

    return VTSS_RC_OK;
}

#endif /* VTSS_ARCH_JAGUAR_1 */
