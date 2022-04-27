/*

 Vitesse Switch API software.

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

#include "cli.h"
#include "mgmt_api.h"
#include "packet_cli.h"
#include "packet_api.h"
#include "cli_trace_def.h"
#include "conf_api.h"
#include "vtss_module_id.h"
#include "vtss_api_if_api.h" /* For vtss_api_if_chip_count() */
#include "vlan_api.h" /* For VLAN_ID_MAX */

/*lint -esym(459, packet_cli_tpid)             Yes, it can be accessed simultaneously from different CLI threads, but this is a debug function */
/*lint -esym(459, tx_seq)                      Yes, it can be accessed simultaneously from different CLI threads, but this is a debug function */
/*lint -esym(459, packet_tx_cnt)               Yes, it can be accessed simultaneously from different CLI threads, but this is a debug function */
/*lint -esym(459, afi_frame_ptrs)              No, PACKET_cli_cmd_debug_frame_afi_cancel() protects this by calls to cyg_scheduler_lock/unlock */
/*lint -esym(459, afi_frame_counting_enabled)  Yes, it can be accessed simultaneously from different CLI threads, but this is a debug function */
/*lint -esym(459, afi_frame_seq_number_offset) Yes, it can be accessed simultaneously from different CLI threads, but this is a debug function */

#ifndef ROUNDING_DIVISION
/* Round x divided by y to nearest integer. x and y are integers */
#define ROUNDING_DIVISION(x, y) (((x) + ((y) / 2)) / (y))
#endif

typedef struct {
    cli_spec_t           smac_spec;
    u8                   smac[6];
    cli_spec_t           dmac_spec;
    u8                   dmac[6];
    cli_spec_t           etype_spec;
    u16                  etype;
    u32                  packet_length;
    u32                  tx_cnt;
#if defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC)
    vtss_fdma_afi_type_t afi_type;
    u32                  afi_fps;
    u32                  idx;
#endif /* defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC) */
#if defined(VTSS_FEATURE_AFI_FDMA)
    u16                  seq_number_offset;
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */
    BOOL                 xtr_qu_not_all;
    u32                  xtr_qu;
    u32                  max_frms_per_sec;
    BOOL                 switch_frm;
} packet_cli_req_t;

#if defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC)
#define PACKET_CLI_CMD_DEBUG_FRM_AFI_CANCEL "Debug Frame AFI Cancel"
#endif /* defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC) */

#if defined(VTSS_FEATURE_AFI_FDMA)
static BOOL afi_frame_counting_enabled  = FALSE;
static u16  afi_frame_seq_number_offset = 0xFFFF; // Internally, we use -1 as 'disabled'
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

#if defined(VTSS_FEATURE_AFI_FDMA) && defined(VTSS_FEATURE_AFI_SWC)
static vtss_fdma_afi_type_t afi_type = VTSS_FDMA_AFI_TYPE_AUTO;
#endif /* defined(VTSS_FEATURE_AFI_FDMA) && defined(VTSS_FEATURE_AFI_SWC) */

#if defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC)
typedef struct {
    enum {AFI_FREE = 0, AFI_RESERVED, AFI_IN_USE, AFI_UNDER_CANCELLATION} state;
    vtss_fdma_afi_type_t afi_type;
    u8                   *frm_ptr;
    u32                  fps;
#if defined(VTSS_FEATURE_AFI_FDMA)
    BOOL                 counting_enabled;
    BOOL                 sequence_enabled;
#endif
} afi_frame_ptr_t;
static afi_frame_ptr_t afi_frame_ptrs[20];
#endif /* defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC) */

static vtss_etype_t packet_cli_tpid = 0x8100;

/****************************************************************************/
// packet_cli_init()
/****************************************************************************/
void packet_cli_init(void)
{
    // Register the size required for packet req. structure
    cli_req_size_register(sizeof(packet_cli_req_t));
}

/****************************************************************************/
// PACKET_cli_cmd_debug_packet()
/****************************************************************************/
static void PACKET_cli_cmd_debug_packet(cli_req_t *req)
{
    ulong parms[CLI_INT_VALUES_MAX];
    int i;

    for (i = 0; i < CLI_INT_VALUES_MAX; i++) {
        parms[i] = req->int_values[i];
    }

    packet_dbg(cli_printf, req->int_value_cnt, parms);
}

/****************************************************************************/
// Since we can transmit the same frame more than once, we cannot have the
// packet module deallocate it automatically upon completion, so we have
// to supply a tx_done callback function that does nothing, except for freeing
// the frame when context is != NULL, which happens upon the last Tx.
// The existence of this callback causes the packet module to not deallocate
// the packet automatically.
/****************************************************************************/
static void tx_done(void *context, packet_tx_done_props_t *props)
{
    if (context != NULL) {
        // Release the packet again.
        packet_tx_free(props->frm_ptr[0]);
    }
}

#if defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC)
/****************************************************************************/
// afi_tx_done()
// Cannot use any other print function - not even if passed in @context argument,
// so we have to use diag_printf() throughout this function.
/****************************************************************************/
static void afi_tx_done(void *context, packet_tx_done_props_t *props)
{
    afi_frame_ptr_t p;
    u32             idx   = (u32)context;
    BOOL            match = FALSE;

    // We have to free the frame pointer.
    packet_tx_free(props->frm_ptr[0]);

    if (idx >= ARRSZ(afi_frame_ptrs)) {
        (void)diag_printf("Internal Error: idx = %u. Expected [0; %zu]\n", idx, ARRSZ(afi_frame_ptrs));
        return;
    }

    cyg_scheduler_lock();
    p = afi_frame_ptrs[idx];
    if (afi_frame_ptrs[idx].frm_ptr == props->frm_ptr[0]) {
        match = TRUE;
        afi_frame_ptrs[idx].frm_ptr = NULL;
        afi_frame_ptrs[idx].state   = AFI_FREE;
    }
    cyg_scheduler_unlock();

    if (!match) {
        (void)diag_printf("Error: AFI frame pointer (idx = %u) didn't match. Called with %p, found %p in afi_frame_ptrs[] array\n", idx + 1, props->frm_ptr[0], p.frm_ptr);
        return;
    }

    if (p.state != AFI_UNDER_CANCELLATION) {
        (void)diag_printf("Error: AFI frame (idx = %u, frm_ptr = %p) was not under cancellation (state = %d)\n", idx + 1, props->frm_ptr[0], p.state);
    }

    (void)diag_printf("AFI Frame (idx = %u) cancelled.", idx + 1);
#if defined(VTSS_FEATURE_AFI_FDMA)
    if (p.counting_enabled) {
        (void)diag_printf(" Transmitted %llu times.", props->frm_cnt);
    }

    if (p.sequence_enabled) {
        (void)diag_printf(" Last sequence number = %u = 0x%08x.", props->afi_last_sequence_number, props->afi_last_sequence_number);
    }
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

    (void)diag_printf("\n");
}
#endif /* defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC) */

#if defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC)
/****************************************************************************/
// packet_cli_afi_save_ptr()
/****************************************************************************/
static void packet_cli_afi_save_ptr(u32 idx, u8 *ptr)
{
    cyg_scheduler_lock();
    afi_frame_ptrs[idx].frm_ptr = ptr;
    afi_frame_ptrs[idx].state = ptr == NULL ? AFI_FREE : AFI_IN_USE;
    cyg_scheduler_unlock();
}
#endif /* defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC) */

/****************************************************************************/
// We support transmitting the same frame a number of times, and default to
// once.
/****************************************************************************/
static ulong packet_tx_cnt = 1;
static void PACKET_cli_cmd_debug_frame_tx_cnt(cli_req_t *req)
{
    if (req->set) {
        packet_tx_cnt = ((packet_cli_req_t *)req->module_req)->tx_cnt;
    } else {
        cli_printf("%u\n", packet_tx_cnt);
    }
}

/****************************************************************************/
// PACKET_cli_cmd_debug_frame_tx()
/****************************************************************************/
static void PACKET_cli_cmd_debug_frame_tx(cli_req_t *req)
{
    static unsigned long tx_seq;
    vtss_port_no_t       iport;
    conf_board_t         conf;
    packet_cli_req_t     *packet_req = req->module_req;
    int                  len = packet_req->packet_length, pos;
    packet_tx_props_t    tx_props;
    ulong                i, local_packet_tx_cnt;
    cyg_tick_count_t     start_ticks = 0;
    u64                  port_mask = 0;
    u32                  port_cnt = 0;
    vtss_port_no_t       port_no = 0;
    u8                   *buffer;
    BOOL                 afi = FALSE;
#if defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC)
    u32                  afi_frame_ptr_idx = 0;
    vtss_fdma_afi_type_t local_afi_type = VTSS_FDMA_AFI_TYPE_AUTO;
#endif /* defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC) */

    if (conf_mgmt_board_get(&conf) < 0) {
        return;
    }

    if (len < 60) {
        len = 60;
    }

#if defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC)
    if (packet_req->switch_frm && packet_req->afi_fps > 0) {
        cli_printf("Error: Cannot send AFI frames switched.\n");
        return;
    }
#endif /* defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC) */

    if (packet_req->switch_frm == FALSE) {
        for (iport = VTSS_PORT_NO_START; iport < VTSS_PORT_NO_END; iport++) {
            vtss_uport_no_t uport = iport2uport(iport);
            if (req->uport_list[uport]) {
                port_mask |= (1ULL << iport);
                port_no = iport;
                port_cnt++;
            }
        }

        if (port_cnt == 0) {
            cli_printf("Error: No ports selected\n");
            return;
        }
    }

#if defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC)
    if (packet_req->afi_fps > 0) {
        if (port_cnt > 1) {
            cli_printf("Error: Can only inject to one port when requesting AFI injection\n");
            return;
        }

        cyg_scheduler_lock();
        for (afi_frame_ptr_idx = 0; afi_frame_ptr_idx < ARRSZ(afi_frame_ptrs); afi_frame_ptr_idx++) {
            if (afi_frame_ptrs[afi_frame_ptr_idx].state == AFI_FREE) {
                break;
            }
        }

        if (afi_frame_ptr_idx == ARRSZ(afi_frame_ptrs)) {
            cyg_scheduler_unlock();
            cli_printf("Error: No free AFI slots\n");
            return;
        }

#if defined(VTSS_FEATURE_AFI_FDMA) && defined(VTSS_FEATURE_AFI_SWC)
        // Don't change the AFI type, since it's selected by the user
        // when both FDMA- and SwC-based AFIs are supported.
        local_afi_type = afi_type;
#endif /* defined(VTSS_FEATURE_AFI_FDMA) && defined(VTSS_FEATURE_AFI_SWC) */

        // Reserve it (in case of multithreaded CLI).
        afi_frame_ptrs[afi_frame_ptr_idx].state            = AFI_RESERVED;
        afi_frame_ptrs[afi_frame_ptr_idx].fps              = packet_req->afi_fps;
        afi_frame_ptrs[afi_frame_ptr_idx].afi_type         = local_afi_type;
#if defined(VTSS_FEATURE_AFI_FDMA)
        afi_frame_ptrs[afi_frame_ptr_idx].counting_enabled = afi_frame_counting_enabled;
        afi_frame_ptrs[afi_frame_ptr_idx].sequence_enabled = afi_frame_seq_number_offset != 0xFFFF;
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */
        afi = TRUE;
        cyg_scheduler_unlock();
    }
#endif /* defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC) */

    buffer = packet_tx_alloc(len);

    if (buffer == NULL) {
#if defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC)
        if (afi) {
            packet_cli_afi_save_ptr(afi_frame_ptr_idx, NULL);
        }
#endif /* defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC) */
        cli_printf("Couldn't allocate Tx buffer\n");
        return;
    }

    memset(buffer, 0xff, 14);
    for (i = 14; i < (ulong)len; i++) {
        buffer[i] = rand();
    }
    if (packet_req->dmac_spec == CLI_SPEC_VAL) {
        memcpy(buffer, packet_req->dmac, 6);
    }
    memcpy(buffer + 6, packet_req->smac_spec == CLI_SPEC_VAL ? packet_req->smac : conf.mac_address, 6);
    if (req->vid_spec == CLI_SPEC_VAL && req->vid != 0) {
        buffer[12] = (packet_cli_tpid >> 8) & 0xFF;
        buffer[13] = (packet_cli_tpid >> 0) & 0xFF;
        buffer[14] = (req->vid >> 8) & 0x0F;
        buffer[15] = (req->vid >> 0) & 0xFF;
        pos = 6 + 6 + 4;
    } else {
        pos = 6 + 6;
    }
    if (packet_req->etype_spec) {
        buffer[pos + 0] = (packet_req->etype >> 8) & 0xFF;
        buffer[pos + 1] = (packet_req->etype >> 0) & 0xFF;
    }

    pos += 2;
    buffer[pos] = port_no;
    *((u32 *)&buffer[pos + 2]) = tx_seq++;

    packet_tx_props_init(&tx_props);
    tx_props.packet_info.modid  = VTSS_MODULE_ID_PACKET;
    tx_props.packet_info.frm[0] = buffer;
    tx_props.packet_info.len[0] = len;
    if (packet_req->switch_frm) {
        tx_props.tx_info.switch_frm = TRUE;
        if (req->vid_spec == CLI_SPEC_VAL && req->vid != 0) {
            tx_props.tx_info.tag.vid = req->vid;
        } else {
            tx_props.tx_info.tag.vid = 1; // What else?
        }
    } else if (port_cnt == 1) {
        tx_props.tx_info.dst_port_mask = VTSS_BIT64(port_no);
    } else {
        tx_props.tx_info.dst_port_mask = port_mask;
    }
    tx_props.packet_info.tx_done_cb = tx_done; /* Avoid deallocation of packet */
    local_packet_tx_cnt = packet_tx_cnt;

#if defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC)
    tx_props.fdma_info.afi_fps  = packet_req->afi_fps;

    if (packet_req->afi_fps > 0) {
#if defined(VTSS_FEATURE_AFI_FDMA)
        if (afi_frame_seq_number_offset != 0xFFFF) {
            // Sequence numbering enabled. Check against the packet length.
            if (afi_frame_seq_number_offset > len - 4) {
                packet_tx_free(buffer);
                packet_cli_afi_save_ptr(afi_frame_ptr_idx, NULL);
                cli_printf("Error: Positions that are sequence number updated ([%u; %u]) are not all contained within the packet ([0; %d])\n", afi_frame_seq_number_offset, afi_frame_seq_number_offset + 3, len - 1);
                return;
            }

            tx_props.fdma_info.afi_enable_sequence_numbering = TRUE;
            tx_props.fdma_info.afi_sequence_number_offset    = afi_frame_seq_number_offset;
        }

        tx_props.fdma_info.afi_enable_counting = afi_frame_counting_enabled;
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

        tx_props.fdma_info.afi_type = local_afi_type;

        cli_printf("Value to use in call to \"%s\": %u\n", PACKET_CLI_CMD_DEBUG_FRM_AFI_CANCEL, afi_frame_ptr_idx + 1);
        // When sending AFI packets, don't take into account how many copies of the packet the user wants to transmit.
        // We could have let the packet module take care of releasing the frame, but we want to
        // be able to write the number of times this frame has been transmitted (if enabled), so
        // we must take care of it ourselves.
        tx_props.packet_info.tx_done_cb         = afi_tx_done;
        tx_props.packet_info.tx_done_cb_context = (void *)afi_frame_ptr_idx;
        local_packet_tx_cnt = 1;
    }
#endif /* defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC) */

    if (local_packet_tx_cnt > 1) {
        start_ticks = cyg_current_time();
        cli_printf("Transmitting %u frames...", local_packet_tx_cnt);
        cli_flush();
    }

    for (i = 0; i < local_packet_tx_cnt; i++) {
        // Don't change tx_done_cb_context for AFI frames
        if (afi == FALSE && i == local_packet_tx_cnt - 1) {
            tx_props.packet_info.tx_done_cb_context = buffer; // Free the frame
        }

        // packet_tx() prints an error if it fails, so don't bother catching the return value.
        (void)packet_tx(&tx_props);

#if defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC)
        if (packet_req->afi_fps > 0) {
            packet_cli_afi_save_ptr(afi_frame_ptr_idx, buffer);
        }
#endif /* defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATIRE_AFI_SWC) */
    }

    if (local_packet_tx_cnt > 1) {
        cyg_tick_count_t end_ticks = cyg_current_time();
        u64 ms;
        char buf[40];
        ms = (end_ticks - start_ticks) * ECOS_MSECS_PER_HWTICK;
        mgmt_long2str_float(buf, ms, 3);
        cli_printf("done in %s seconds.\n", buf);
    }
}

/****************************************************************************/
// PACKET_cli_cmd_debug_frame_tpid()
/****************************************************************************/
static void PACKET_cli_cmd_debug_frame_tpid(cli_req_t *req)
{
    packet_cli_req_t *packet_req = req->module_req;
    if (packet_req->etype_spec == CLI_SPEC_VAL) {
        packet_cli_tpid = packet_req->etype;
    } else {
        cli_printf("Current TPID = 0x%04x\n", packet_cli_tpid);
    }
}

#if defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC)
/****************************************************************************/
// PACKET_cli_cmd_debug_frame_afi_cancel()
/****************************************************************************/
static void PACKET_cli_cmd_debug_frame_afi_cancel(cli_req_t *req)
{
    packet_cli_req_t *packet_req = req->module_req;
    u32              idx;

    if (req->all) {
        cyg_scheduler_lock();
        for (idx = 0; idx < ARRSZ(afi_frame_ptrs); idx++) {
            if (afi_frame_ptrs[idx].state == AFI_IN_USE) {
                afi_frame_ptrs[idx].state = AFI_UNDER_CANCELLATION;
                cyg_scheduler_unlock();

                cli_printf("Cancelling %p\n", afi_frame_ptrs[idx].frm_ptr);
                if (packet_tx_afi_cancel(afi_frame_ptrs[idx].frm_ptr) != VTSS_RC_OK) {
                    cli_printf("Error: AFI cancel failed for idx = %u (%p)\n", idx + 1, afi_frame_ptrs[idx].frm_ptr);
                }
                cyg_scheduler_lock();
            }
        }
        cyg_scheduler_unlock();
    } else {
        BOOL            wrong_state = FALSE;
        u8              *frm_ptr    = NULL;
        afi_frame_ptr_t *p          = &afi_frame_ptrs[packet_req->idx - 1];

        // Cancel just one.
        cyg_scheduler_lock();
        if (p->state != AFI_IN_USE) {
            wrong_state = TRUE;
        } else {
            frm_ptr = p->frm_ptr;
            p->state = AFI_UNDER_CANCELLATION;
        }
        cyg_scheduler_unlock();

        if (wrong_state) {
            cli_printf("Error: AFI frame (idx = %u) is not in use\n", packet_req->idx);
            return;
        }

        if (packet_tx_afi_cancel(frm_ptr) != VTSS_RC_OK) {
            cli_printf("Error: AFI Cancel failed for idx = %u (%p)", packet_req->idx, frm_ptr);
        }
    }
}
#endif /* defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC) */

#if defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC)
/****************************************************************************/
// PACKET_cli_cmd_debug_frame_afi_list()
/****************************************************************************/
static void PACKET_cli_cmd_debug_frame_afi_list(cli_req_t *req)
{
    u32 i;

    cli_printf("\n");
#if defined(VTSS_FEATURE_AFI_FDMA) && defined(VTSS_FEATURE_AFI_SWC)
    cli_table_header("#   State     Frames/sec  Frame Ptr   DMAC               AFI Type  Frame Count     ");
#elif defined(VTSS_FEATURE_AFI_FDMA)
    cli_table_header("#   State     Frames/sec  Frame Ptr   DMAC               Frame Count     ");
#else
    cli_table_header("#   State     Frames/sec  Frame Ptr   DMAC               ");
#endif /* defined(VTSS_FEATURE_AFI_FDMA/SWC) */

    for (i = 0; i < ARRSZ(afi_frame_ptrs); i++) {
        afi_frame_ptr_t p;
        cyg_scheduler_lock();
        p = afi_frame_ptrs[i];
        cyg_scheduler_unlock();

        cli_printf("%2u  %-8s  ",
                   i + 1,
                   p.state == AFI_FREE               ? "Free"     :
                   p.state == AFI_RESERVED           ? "Reserved" :
                   p.state == AFI_IN_USE             ? "In use"   :
                   p.state == AFI_UNDER_CANCELLATION ? "Cancel"   : "Unknown");

        if (p.state == AFI_IN_USE || p.state == AFI_UNDER_CANCELLATION) {
            cli_printf("%10u  %p  %s  ", p.fps, p.frm_ptr, misc_mac2str(p.frm_ptr));

#if defined(VTSS_FEATURE_AFI_FDMA) && defined(VTSS_FEATURE_AFI_SWC)
            cli_printf("%-8s  ",
                       p.afi_type == VTSS_FDMA_AFI_TYPE_AUTO ? "Auto" :
                       p.afi_type == VTSS_FDMA_AFI_TYPE_FDMA ? "FDMA" :
                       p.afi_type == VTSS_FDMA_AFI_TYPE_SWC  ? "SwC"  : "Unknown");
#endif

#if defined(VTSS_FEATURE_AFI_FDMA)
            if (p.afi_type == VTSS_FDMA_AFI_TYPE_AUTO || p.afi_type == VTSS_FDMA_AFI_TYPE_FDMA) {
                u64 frm_cnt;
                if (packet_tx_afi_frm_cnt(p.frm_ptr, &frm_cnt) != VTSS_RC_OK) {
                    frm_cnt = 0;
                }
                cli_printf("%14llu", frm_cnt);
            } else {
                cli_printf("N/A");
            }
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */
        }

        cli_printf("\n");
    }

    cli_printf("For AFI frames injected by other modules, use ");

#if defined(VTSS_FEATURE_AFI_FDMA) && defined(VTSS_FEATURE_AFI_SWC)
    cli_printf("'debug api fdma' or 'debug api afi'");
#elif defined(VTSS_FEATURE_AFI_FDMA)
    cli_printf("'debug api fdma'");
#else
    cli_printf("'debug api afi'");
#endif /* defined(VTSS_FEATURE_AFI_FDMA/SWC) */

    cli_printf("\n\n");
}
#endif /* defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC) */

#if defined(VTSS_FEATURE_AFI_FDMA)
/****************************************************************************/
// PACKET_cli_cmd_debug_frame_afi_counting()
/****************************************************************************/
static void PACKET_cli_cmd_debug_frame_afi_counting(cli_req_t *req)
{
    if (req->set) {
        afi_frame_counting_enabled = req->enable;
    } else {
        cli_printf("AFI frame counting is %s for new frames\n", afi_frame_counting_enabled ? "enabled" : "disabled");
    }
}
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

#if defined(VTSS_FEATURE_AFI_FDMA)
/****************************************************************************/
// PACKET_cli_cmd_debug_frame_afi_seq_number_offset()
/****************************************************************************/
static void PACKET_cli_cmd_debug_frame_afi_seq_number_offset(cli_req_t *req)
{
    packet_cli_req_t *packet_req = req->module_req;

    if (req->set) {
        afi_frame_seq_number_offset = packet_req->seq_number_offset;
    } else {
        if (afi_frame_seq_number_offset == 0xFFFF) {
            cli_printf("AFI sequence numbering is disabled for new frames\n");
        } else {
            cli_printf("AFI sequence numbering is enabled for new frames. Update offset within frame = %u\n", afi_frame_seq_number_offset);
        }
    }
}
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

#if defined(VTSS_FEATURE_AFI_FDMA) && defined(VTSS_FEATURE_AFI_SWC)
/****************************************************************************/
// PACKET_cli_parse_afi_type()
/****************************************************************************/
static void PACKET_cli_cmd_debug_frame_afi_type(cli_req_t *req)
{
    packet_cli_req_t *packet_req = req->module_req;

    if (req->set) {
        afi_type = packet_req->afi_type;
    } else {
        cli_printf("AFI type chosen to be %s for new frames\n",
                   afi_type == VTSS_FDMA_AFI_TYPE_AUTO ? "automatic"  :
                   afi_type == VTSS_FDMA_AFI_TYPE_FDMA ? "FDMA-based" :
                   afi_type == VTSS_FDMA_AFI_TYPE_SWC  ? "switch-core-based" : "unknown");
    }
}
#endif /* defined(VTSS_FEATURE_AFI_FDMA) && defined(VTSS_FEATURE_AFI_SWC) */

#if VTSS_OPT_FDMA && !defined(VTSS_ARCH_LUTON28)
/****************************************************************************/
// PACKET_cli_cmd_debug_throttling()
/****************************************************************************/
static void PACKET_cli_cmd_debug_throttling(cli_req_t *req)
{
    packet_cli_req_t         *packet_req = req->module_req;
    vtss_fdma_throttle_cfg_t throttle_cfg;
    vtss_packet_rx_queue_t   xtr_qu, xtr_qu_min, xtr_qu_max;
    char                     usage[100];

    if (!packet_req->xtr_qu_not_all) {
        xtr_qu_min = 0;
        xtr_qu_max = VTSS_PACKET_RX_QUEUE_CNT - 1;
    } else {
        xtr_qu_min = xtr_qu_max = packet_req->xtr_qu;
    }

    if (vtss_fdma_throttle_cfg_get(NULL, &throttle_cfg) != VTSS_RC_OK) {
        cli_printf("Error: Couldn't get current throttle configuration.\n");
        return;
    }

    if (req->set) {
        for (xtr_qu = xtr_qu_min; xtr_qu <= xtr_qu_max; xtr_qu++) {
            throttle_cfg.frm_limit_per_tick[xtr_qu] = packet_req->max_frms_per_sec > 0 ? MAX(ROUNDING_DIVISION(packet_req->max_frms_per_sec,  PACKET_THROTTLE_FREQ_HZ), 1) : 0;
            throttle_cfg.suspend_tick_cnt[xtr_qu]   = 0;
        }

        if (packet_rx_throttle_cfg_set(&throttle_cfg) != VTSS_RC_OK) {
            cli_printf("Error: Couldn't set new throttle configuration.\n");
            return;
        }
    } else {
        cli_printf("Throttle tick frequency: %d ticks/second\n", PACKET_THROTTLE_FREQ_HZ);
        cli_table_header("Xtr Qu  Frames/Sec  Bytes/Sec   Usage            ");

        for (xtr_qu = xtr_qu_min; xtr_qu <= xtr_qu_max; xtr_qu++) {
            cli_printf("%6u  %10u  %10u  %s\n", xtr_qu, throttle_cfg.frm_limit_per_tick[xtr_qu] * PACKET_THROTTLE_FREQ_HZ, throttle_cfg.byte_limit_per_tick[xtr_qu] * PACKET_THROTTLE_FREQ_HZ, packet_rx_queue_usage(xtr_qu, usage, sizeof(usage)));
        }
        cli_printf("\n");
    }
}
#endif

#if defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC)
/****************************************************************************/
// PACKET_cli_parse_afi_fps()
// Note to myself: req->parm_parsed counts the number of parameters eaten
// by this function. This defaults to 1.
// We return 1 on error, 0 on success.
/****************************************************************************/
static int32_t PACKET_cli_parse_afi_fps(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    packet_cli_req_t *packet_req = req->module_req;
    int f;

    if (tolower(cmd[0]) == 'x') {
        if (sscanf(cmd + 1, "%d", &f) != 1) {
            cli_printf("Error: Invalid AFI frame frequency specified\n");
            return 1;
        } else if (f < 1 || f > VTSS_AFI_FPS_MAX) {
            cli_printf("Error: Invalid AFI frame frequency. Valid range is [1; %d]\n", VTSS_AFI_FPS_MAX);
            return 1;
        } else {
            packet_req->afi_fps = f;
            return 0;
        }
    } else {
        // No error. Just not for us.
        req->parm_parsed = 0;
        return 0;
    }
}
#endif /* defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC) */

#if defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC)
/****************************************************************************/
// PACKET_cli_parse_afi_idx()
/****************************************************************************/
static int32_t PACKET_cli_parse_afi_idx(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t          error       = 0;
    packet_cli_req_t *packet_req = req->module_req;
    char *found = cli_parse_find(cmd, "all");

    req->parm_parsed = 1;

    if (found && !strncmp(found, "all", 3)) {
        req->all = 1;
    } else {
        error = cli_parse_ulong(cmd, &packet_req->idx, 1, ARRSZ(afi_frame_ptrs));
    }

    return error;
}
#endif /* defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC) */

#if defined(VTSS_FEATURE_AFI_FDMA)
/****************************************************************************/
// PACKET_cli_parse_afi_offset()
/****************************************************************************/
static int32_t PACKET_cli_parse_afi_offset(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t          error       = 0;
    packet_cli_req_t *packet_req = req->module_req;
    u32              val;
    char             *found = cli_parse_find(cmd, "disable");

    req->parm_parsed = 1;

    if (found && !strncmp(found, "disable", 7)) {
        val = 0xFFFF; /* -1 disables */
    } else {
        error = cli_parse_ulong(cmd, &val, 0, 0xFFFE);
    }

    packet_req->seq_number_offset = val;
    return error;
}
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

#if defined(VTSS_FEATURE_AFI_FDMA)
/****************************************************************************/
// PACKET_cli_parse_afi_counting()
/****************************************************************************/
static int32_t PACKET_cli_parse_afi_counting(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char *found = cli_parse_find(cmd, stx);

    if (found != NULL) {
        if (!strncmp(found, "enable", 6)) {
            req->enable = 1;
        } else if (!strncmp(found, "disable", 7)) {
            req->enable = 0;
        }
    }

    return (found == NULL ? 1 : 0);
}
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

#if defined(VTSS_FEATURE_AFI_FDMA) && defined(VTSS_FEATURE_AFI_SWC)
/****************************************************************************/
// PACKET_cli_parse_afi_type()
/****************************************************************************/
static int32_t PACKET_cli_parse_afi_type(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    char *found = cli_parse_find(cmd, stx);

    if (found != NULL) {
        if (!strncmp(found, "auto", 4)) {
            req->afi_type = VTSS_FDMA_AFI_TYPE_AUTO;
        } else if (!strncmp(found, "fdma", 4)) {
            req->afi_type = VTSS_FDMA_AFI_TYPE_FDMA;
        } else if (!strncmp(found, "swc", 3)) {
            req->afi_type = VTSS_FDMA_AFI_TYPE_SWC;
        }
    }

    return (found == NULL ? 1 : 0);
}
#endif /* defined(VTSS_FEATURE_AFI_FDMA) && defined(VTSS_FEATURE_AFI_SWC) */

// Anything that can be transmitted in a single DCB on all platforms
#define PACKET_CLI_TX_FRAME_LENGTH_MAX 4000

/****************************************************************************/
// PACKET_cli_parse_length()
/****************************************************************************/
static int32_t PACKET_cli_parse_length(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t          error       = 0;
    packet_cli_req_t *packet_req = req->module_req;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &packet_req->packet_length, 60, PACKET_CLI_TX_FRAME_LENGTH_MAX);

    return error;
}

/****************************************************************************/
// PACKET_cli_parse_port_list()
/****************************************************************************/
static int32_t PACKET_cli_parse_port_list(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t          error       = 0;
    packet_cli_req_t *packet_req = req->module_req;

    req->parm_parsed = 1;
    if (cli_parse_none(cmd) == 0) { // 0 means success (!)
        packet_req->switch_frm = TRUE;
    } else {
        error = cli_parm_parse_port_list(cmd, cmd2, stx, cmd_org, req);
    }

    return error;
}

/****************************************************************************/
// PACKET_cli_parse_tx_cnt()
/****************************************************************************/
static int32_t PACKET_cli_parse_tx_cnt(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t          error       = 0;
    packet_cli_req_t *packet_req = req->module_req;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &packet_req->tx_cnt, 1, 1000000);

    return error;
}

/****************************************************************************/
// PACKET_cli_parse_dmac()
/****************************************************************************/
static int32_t PACKET_cli_parse_dmac(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t          error       = 0;
    packet_cli_req_t *packet_req = req->module_req;

    req->parm_parsed = 1;
    error = cli_parse_mac(cmd, packet_req->dmac, &packet_req->dmac_spec, 0);

    return error;
}

/****************************************************************************/
// PACKET_cli_parse_smac()
/****************************************************************************/
static int32_t PACKET_cli_parse_smac(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t          error       = 0;
    packet_cli_req_t *packet_req = req->module_req;

    req->parm_parsed = 1;
    error = cli_parse_mac(cmd, packet_req->smac, &packet_req->smac_spec, 0);

    return error;
}

/****************************************************************************/
// PACKET_cli_parse_vid()
/****************************************************************************/
static int32_t PACKET_cli_parse_vid(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t error = 0;
    ulong value = 0;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &value, VTSS_VID_NULL, VLAN_ID_MAX);
    if (!error) {
        req->vid_spec = CLI_SPEC_VAL;
        req->vid = value;
    }

    return error;
}

/****************************************************************************/
// PACKET_cli_parse_etype()
/****************************************************************************/
static int32_t PACKET_cli_parse_etype(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    int32_t          error       = 0;
    packet_cli_req_t *packet_req = req->module_req;
    ulong            value;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &value, 0, 65535);
    if (!error) {
        packet_req->etype_spec = CLI_SPEC_VAL;
        packet_req->etype = value;
    }

    return error;
}

#if VTSS_OPT_FDMA && !defined(VTSS_ARCH_LUTON28)
/****************************************************************************/
// PACKET_cli_parse_xtr_qu()
/****************************************************************************/
static int32_t PACKET_cli_parse_xtr_qu(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    packet_cli_req_t *packet_req = req->module_req;
    int32_t          error       = 0;

    req->parm_parsed = 1;
    if (cli_parse_all(cmd) != 0) {
        // Didn't match 'all'
        u32 val;
        if ((error = cli_parse_ulong(cmd, &val, 0, VTSS_PACKET_RX_QUEUE_CNT - 1)) == 0) {
            packet_req->xtr_qu_not_all = TRUE;
            packet_req->xtr_qu         = val;
        }
    }

    return error;
}
#endif

#if VTSS_OPT_FDMA && !defined(VTSS_ARCH_LUTON28)
/****************************************************************************/
// PACKET_cli_parse_max_frms_per_sec()
/****************************************************************************/
static int32_t PACKET_cli_parse_max_frms_per_sec(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    packet_cli_req_t *packet_req = req->module_req;
    req->parm_parsed = 1;
    return cli_parse_ulong(cmd, &packet_req->max_frms_per_sec, 0, 0xFFFFFFFF);
}
#endif

/****************************************************************************/
/*  Parameter table                                                         */
/****************************************************************************/
static cli_parm_t PACKET_cli_parm_table[] = {
#if defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC)
    {
        // Notice that the afi_fps is preceded by an 'x' in order not to interfere with
        // normal packet_tx CLI functionality whenever we won't inject to the AFI channels
        // and whenever AFI is not included (and we don't want to put this option
        // last, since all options then have to be specified if we wanted to send a
        // AFI frame).
        "<afi_fps>",
        "Inject frame periodically and with a frequency of <afi_fps> frames per second.\n"
        "                  Precede <afi_fps> by 'x'.\n"
        "                  Valid range of <afi_fps> is [1; " vtss_xstr(VTSS_AFI_FPS_MAX) "].\n"
        "                  Example: x300: inject frame with a period of 1/300 = 3.33 ms",
        CLI_PARM_FLAG_NONE,
        PACKET_cli_parse_afi_fps,
        NULL
    },
#endif /* defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC) */
#if defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC)
    {
        "<idx>",
        "Index of the AFI frame to cancel. Use 'Debug Frame AFI List' to get a list of frames. Use 'all' to cancel all frames",
        CLI_PARM_FLAG_NONE,
        PACKET_cli_parse_afi_idx,
        NULL
    },
#endif /* defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC) */
#if defined(VTSS_FEATURE_AFI_FDMA)
    {
        "<offset>",
        "Byte offset within frame to sequence number update. Use 'disable' to disable sequence number updating of subsequent frames",
        CLI_PARM_FLAG_SET,
        PACKET_cli_parse_afi_offset,
        NULL
    },
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */
#if defined(VTSS_FEATURE_AFI_FDMA)
    {
        "enable|disable",
        "enable     : Enable AFI frame counting\n"
        "disable    : Disable AFI frame counting\n"
        "(default: Show current state)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        PACKET_cli_parse_afi_counting,
        PACKET_cli_cmd_debug_frame_afi_counting,
    },
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */
#if defined(VTSS_FEATURE_AFI_FDMA) && defined(VTSS_FEATURE_AFI_SWC)
    {
        "auto|fdma|swc",
        "auto    : Ask FDMA driver to select the best AFI engine\n"
        "fdma    : Use the FDMA to inject frames periodically\n"
        "swc     : Use the switch-core to inject frames periodically\n"
        "(default: Show current selection)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        PACKET_cli_parse_afi_type,
        PACKET_cli_cmd_debug_frame_afi_type,
    },
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */
    {
        "<port_none_list>",
        "Port list or 'all' or 'none'.\n"
        "                  'all'  : Send non-switched to all ports.\n"
        "                  'none' : Send switched.\n"
        "                  Default: Nothing",
        CLI_PARM_FLAG_SET,
        PACKET_cli_parse_port_list,
        NULL
    },
    {
        "<packet_length>",
        "Packet length (60 - " vtss_xstr(PACKET_CLI_TX_FRAME_LENGTH_MAX) "), excluding FCS",
        CLI_PARM_FLAG_NONE,
        PACKET_cli_parse_length,
        NULL
    },
    {
        "<dmac>",
        "Destination MAC address\n"
        "                  Format: 'xx-xx-xx-xx-xx-xx', 'xx.xx.xx.xx.xx.xx', or 'xxxxxxxxxxxx', x is a hexadecimal digit\n"
        "                  Default: Broadcast MAC address",
        CLI_PARM_FLAG_SET,
        PACKET_cli_parse_dmac,
        NULL
    },
    {
        "<tx_cnt>",
        "Number of times to transmit frame",
        CLI_PARM_FLAG_SET,
        PACKET_cli_parse_tx_cnt,
        NULL
    },
    {
        "<smac>",
        "Source MAC address\n"
        "                  Format: 'xx-xx-xx-xx-xx-xx', 'xx.xx.xx.xx.xx.xx', or 'xxxxxxxxxxxx', x is a hexadecimal digit\n"
        "                  Default: Board MAC address",
        CLI_PARM_FLAG_SET,
        PACKET_cli_parse_smac,
        NULL
    },
    {
        "<vid>",
        "VLAN ID (0-4095). Use empty or 0 for none",
        CLI_PARM_FLAG_SET,
        PACKET_cli_parse_vid, /* In order to be able to parse 0 */
        NULL
    },
    {
        "<etype>",
        "EtherType (0 - 65535)",
        CLI_PARM_FLAG_SET,
        PACKET_cli_parse_etype,
        NULL
    },
#if VTSS_OPT_FDMA && !defined(VTSS_ARCH_LUTON28)
    {
        "<xtr_qu>",
        "The extraction queue to configure or show settings for. Defaults to 'all'",
        CLI_PARM_FLAG_NONE,
        PACKET_cli_parse_xtr_qu,
        PACKET_cli_cmd_debug_throttling
    },
#endif
#if VTSS_OPT_FDMA && !defined(VTSS_ARCH_LUTON28)
    {
        "<max_frms_per_sec>",
        "The maximum number of frames to extract in one second without suspending the Rx queue. 0 disables throttling.",
        CLI_PARM_FLAG_SET,
        PACKET_cli_parse_max_frms_per_sec,
        PACKET_cli_cmd_debug_throttling
    },
#endif
    {NULL, NULL, 0, 0, NULL}
};

/****************************************************************************/
/*  Command table                                                           */
/****************************************************************************/
enum {
    PACKET_CLI_PRIO_DEBUG_PACKET_THROTTLING           = CLI_CMD_SORT_KEY_DEFAULT,
    PACKET_CLI_PRIO_DEBUG_PACKET                      = CLI_CMD_SORT_KEY_DEFAULT,
    PACKET_CLI_PRIO_DEBUG_FRAME_TX                    = CLI_CMD_SORT_KEY_DEFAULT,
    PACKET_CLI_PRIO_DEBUG_FRAME_TPID                  = CLI_CMD_SORT_KEY_DEFAULT,
    PACKET_CLI_PRIO_DEBUG_FRAME_TX_CNT                = CLI_CMD_SORT_KEY_DEFAULT,
#if defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC)
    PACKET_CLI_PRIO_DEBUG_FRAME_AFI_CANCEL            = CLI_CMD_SORT_KEY_DEFAULT,
    PACKET_CLI_PRIO_DEBUG_FRAME_AFI_LIST              = CLI_CMD_SORT_KEY_DEFAULT,
#endif /* defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC) */
#if defined(VTSS_FEATURE_AFI_FDMA)
    PACKET_CLI_PRIO_DEBUG_FRAME_AFI_COUNTING          = CLI_CMD_SORT_KEY_DEFAULT,
    PACKET_CLI_PRIO_DEBUG_FRAME_AFI_SEQ_NUMBER_UPDATE = CLI_CMD_SORT_KEY_DEFAULT,
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */
#if defined(VTSS_FEATURE_AFI_FDMA) && defined(VTSS_FEATURE_AFI_SWC)
    PACKET_CLI_PRIO_DEBUG_FRAME_AFI_TYPE              = CLI_CMD_SORT_KEY_DEFAULT,
#endif /* defined(VTSS_FEATURE_AFI_FDMA) && defined(VTSS_FEATURE_AFI_SWC) */
} packet_cli_prio_t;

#if VTSS_OPT_FDMA && !defined(VTSS_ARCH_LUTON28)
cli_cmd_tab_entry(
    "Debug Packet Throttle [<xtr_qu>]",
    "Debug Packet Throttle [<xtr_qu>] [<max_frms_per_sec>]",
    "Set or show current Rx queue throttling configuration",
    PACKET_CLI_PRIO_DEBUG_PACKET_THROTTLING,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    PACKET_cli_cmd_debug_throttling,
    NULL,
    PACKET_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif

cli_cmd_tab_entry(
    NULL,
    "Debug Packet Statistics [<integer>] [<integer>] [<integer>]\n"
    "                        [<integer>] [<integer>] [<integer>]",
    "Debug Packet Module",
    PACKET_CLI_PRIO_DEBUG_PACKET,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    PACKET_cli_cmd_debug_packet,
    NULL,
    NULL,
    CLI_CMD_FLAG_NONE
);

#if defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC)
#define FRM_TX_SYNTAX "Debug Frame Tx <port_none_list> [<afi_fps>] [<packet_length>] [<dmac>] [<smac>] [<vid>] [<etype>]"
#else
#define FRM_TX_SYNTAX "Debug Frame Tx <port_none_list> [<packet_length>] [<dmac>] [<smac>] [<vid>] [<etype>]"
#endif /* defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC) */

cli_cmd_tab_entry (
    NULL,
    FRM_TX_SYNTAX,
    "Transmit a frame",
    PACKET_CLI_PRIO_DEBUG_FRAME_TX,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    PACKET_cli_cmd_debug_frame_tx,
    NULL,
    PACKET_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    NULL,
    "Debug Frame TPID [<etype>]",
    "When transmitting frames with a VLAN tag, choose TPID (default 0x8100)",
    PACKET_CLI_PRIO_DEBUG_FRAME_TPID,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    PACKET_cli_cmd_debug_frame_tpid,
    NULL,
    PACKET_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

CLI_CMD_TAB_ENTRY_DECL(PACKET_cli_cmd_debug_frame_tx_cnt) = {
    NULL,
    "Debug Frame TxCnt [<tx_cnt>]",
    "Transmit the same frame this many times"
#if defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC)
    " (not in effect for AFI frames)"
#endif /* defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC) */
    ,
    PACKET_CLI_PRIO_DEBUG_FRAME_TX_CNT,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    PACKET_cli_cmd_debug_frame_tx_cnt,
    NULL,
    PACKET_cli_parm_table,
    CLI_CMD_FLAG_NONE
};

#if defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC)
cli_cmd_tab_entry (
    NULL,
    PACKET_CLI_CMD_DEBUG_FRM_AFI_CANCEL " <idx>",
    "Cancel periodic transmission of one or all frames",
    PACKET_CLI_PRIO_DEBUG_FRAME_AFI_CANCEL,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    PACKET_cli_cmd_debug_frame_afi_cancel,
    NULL,
    PACKET_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif /* defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC) */

#if defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC)
cli_cmd_tab_entry (
    NULL,
    "Debug Frame AFI List",
    "List AFI frames currently being injected by debug commands",
    PACKET_CLI_PRIO_DEBUG_FRAME_AFI_LIST,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    PACKET_cli_cmd_debug_frame_afi_list,
    NULL,
    NULL,
    CLI_CMD_FLAG_NONE
);
#endif /* defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC) */

#if defined(VTSS_FEATURE_AFI_FDMA)
cli_cmd_tab_entry (
    NULL,
    "Debug Frame AFI Counting [enable|disable]",
    "Enable or disable AFI frame counting of subsequent frames.\nDefault: Show current state.",
    PACKET_CLI_PRIO_DEBUG_FRAME_AFI_COUNTING,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    PACKET_cli_cmd_debug_frame_afi_counting,
    NULL,
    PACKET_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

#if defined(VTSS_FEATURE_AFI_FDMA)
cli_cmd_tab_entry (
    NULL,
    "Debug Frame AFI SeqOffset [<offset>]",
    "Set a byte offset within the frame to sequence number update. Use 'disable' to disable sequence number updating of subsequent frames.\nDefault: Show current offset/state.",
    PACKET_CLI_PRIO_DEBUG_FRAME_AFI_SEQ_NUMBER_UPDATE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    PACKET_cli_cmd_debug_frame_afi_seq_number_offset,
    NULL,
    PACKET_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

#if defined(VTSS_FEATURE_AFI_FDMA) && defined(VTSS_FEATURE_AFI_SWC)
// Both FDMA- and SwC-based AFI supported. Gotta provide a command
// that allows the user to select from one of them.
cli_cmd_tab_entry (
    NULL,
    "Debug Frame AFI Type [auto|fdma|swc]",
    "Select the AFI to use when injecting frames periodically\nDefault: Show current selection.",
    PACKET_CLI_PRIO_DEBUG_FRAME_AFI_TYPE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    PACKET_cli_cmd_debug_frame_afi_type,
    NULL,
    PACKET_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
