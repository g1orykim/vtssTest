/*

 Vitesse Switch Software.

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

/**
 * \file
 * \brief Frame DMA API
 */

#ifndef _VTSS_FDMA_API_H_
#define _VTSS_FDMA_API_H_

#include <vtss_options.h>
#include <vtss_types.h>

/**
 * <B>FREQUENTLY ASKED QUESTIONS</B>
 *
 * <B>Q:</B> What CPUs does the FDMA work with?\n
 * <B>A:</B> The FDMA Driver Kit *must* execute on the embedded processor, since
 *           an external CPU doesn't have access to the FDMA silicon.
 *
 * ------------------------------------oOo------------------------------------
 *
 * <B>Q:</B> Can the FDMA be used in parallel with "manual" frame transmission or
 *    reception?\n
 * <B>A:</B> It is not recommended to use the FDMA together with "manual" frame
 *           transmission or reception (manual refers to the fact that the CPU spends
 *           clock cycles to read or write every single byte to the relevant registers
 *           within the switch core). However, for frame transmission, it is possible
 *           to have the FDMA working on one set of front ports and the manual
 *           transmission working on another set. For frame reception, it is possible
 *           to have the FDMA extract from one set of the extraction queues, and have
 *           the manual reception working on another set. The sets must be disjunct.
 *
 * ------------------------------------oOo------------------------------------
 *
 * <B>Q:</B> Are any of the API functions blocking?\n
 * <B>A:</B> No, none of the functions are blocking. When an API function needs
 *           exclusive access to a global variable, it uses a macro (either
 *           VTSS_OS_INTERRUPT_DISABLE() or VTSS_OS_SCHEDULER_LOCK(), depending
 *           on the value of the VTSS_OPT_FDMA_IRQ_CONTEXT preprocessor symbol)
 *           to ask for OS-specific support to globally ensure mutual exclusiveness.
 *
 * ------------------------------------oOo------------------------------------
 *
 * <B>Q:</B> Who allocates memory?\n
 * <B>A:</B> The API is built in such a way that it is up to the caller to allocate
 *           and free memory. Some may wish to allocate all the buffers needed by the
 *           FDMA statically, while others may want to allocate it dynamically. If
 *           the FDMA code was to implement code for all possible scenaria, it would
 *           not be as flexible as it is as of today.
 *
 * <B>TERMS AND ABBREVIATIONS</B>
 *
 * <B>Extraction:</B>\n
 *   This term is used by the FDMA silicon and driver kit and is equivalent to
 *   what higher levels of software would call Rx, i.e. packet reception.
 *   Extraction is abbreviated XTR.
 *
 * <B>Injection:</B>\n
 *   This term is used by the FDMA silicon and driver kit and is equivalent to
 *   what higher levels of software would call Tx, i.e. packet transmission.
 *   Injection is abbreviated INJ.
 *
 * <B>Packet Module:</B>\n
 *   The software module that implements all the calls to the FDMA API.
 *
 * <B>Call context:</B>\n
 *   The context that a function must be called in. In a multithreaded
 *   environment this could be "thread" or "DSR".
 *
 * <B>Re-entrancy:</B>\n
 *   Some of the functions are allowed to be called simultaneously from
 *   several threads.
 *
 * <B>DSR:</B>\n
 *   Deferred Service Routine. This is the term that eCos uses for the
 *   context that is scheduled by an IRQ handler. The context is assumed
 *   to be interruptible only by another hardware interrupt, i.e.
 *   the scheduler cannot schedule other DSRs or threads while a DSR
 *   is running. In Linux, this is known as the "bottom half". A DSR can
 *   never wait for critical sections or semaphores or the like.
 *
 * <B>DCB:</B>\n
 *   In the FDMA driver context, a DCB is normally a software DCB, i.e.
 *   an instance of the vtss_fdma_list_t structure. Each software DCB
 *   embeds a hardware DCB, which is filled by the FDMA driver code and
 *   passed to the FDMA silicon, i.e. the higher levels of software need
 *   not be concerned about hardware DCBs, but they need to allocate room
 *   for them, which is therefore done in the software DCB. The same software
 *   DCB type is used for injection and extraction, but some of the fields'
 *   meaning differ for the two. Please refer to the vtss_fdma_list_t
 *   structure for the exact interpretation and Packet Module requirements
 *   for the DCBs.
 *
 * <B>SOF DCB:</B>\n
 *   Start-Of-Frame DCB. The first (software or hardware) DCB in a list of
 *   DCBs making up one frame. The list is at least 1 item long.
 *
 * <B>EOF DCB:</B>\n
 *   End-Of-Frame DCB. The last (software or hardware) DCB in a list of DCBs
 *   making up one frame. The list is at least 1 item long, so the EOF DCB
 *   may be identical to the SOF DCB.
 *
 * <B>Non-SOF DCBs:</B>\n
 *   All but the first DCB in a list of (software or hardware) DCBs making up
 *   one frame.
 *
 * <B>Extraction Queue:</B>\n
 *   The chip contains a number of queues for temporarily storing frames destined
 *   for the CPU. These so-called extraction queues are of configurable length
 *   and are located in the CPU Port Module. They share memory with the CPU
 *   Port Module's injection queue. The splitting of memory between the queues
 *   is not handled by the FDMA driver code. Likewise, the assignment of a
 *   particular type of frames to an extraction queue (which typically takes
 *   place in the chip's analyzer (ANA) is not done by the FDMA driver code.
 *   The valid range of extraction queue numbers is [VTSS_PACKET_RX_QUEUE_START;
 *   VTSS_PACKET_RX_QUEUE_END[.
 *
 * <B>DMA Channel:</B>\n
 *   A DMA channel is used per flow. For extraction each extraction queue
 *   is statically mapped to a DMA channel through the call to vtss_fdma_xtr_cfg().
 *   For injection, there is no static mapping between a channel number and a
 *   front port number, but a channel must still be assigned for injection.
 *   Valid channel numbers are in the range [0; VTSS_FDMA_CH_CNT[.
 *   An intrinsic priority exists among the channels, in that higher-numbered
 *   channels have higher priority. Thus, if two channels serve two different
 *   extraction queues, and both report frames present, then the higher numbered
 *   channel (which is not necessarily the higher numbered extraction queue!)
 *   will get serviced first.
 *
 * <B>LAYOUT OF INJECTED AND EXTRACTED FRAMES</B>
 *
 * <B>Injection:</B>\n
 *   When injecting a frame, the SOF DCB's data pointer must point to an
 *   area of at least VTSS_FDMA_INJ_HDR_SIZE_BYTES (VTSS_OPT_FDMA_VER == 1) or
 *   VTSS_FDMA_HDR_SIZE_BYTES (VTSS_OPT_FDMA_VER >= 2), which is reserved by the FDMA.
 *   The byte following these initial bytes must therefore be the first byte of the
 *   frame's DMAC.
 *   If VTSS_OPT_FDMA_VER >= 2, then the same buffers can be used for
 *   both injection and extraction, and the amount of data to reserve in the
 *   beginning of a frame is given by VTSS_FDMA_HDR_SIZE_BYTES.
 *
 * <B>Extraction:</B>\n
 *   When an extracted frame is delivered to the callback function, the first
 *   VTSS_FDMA_XTR_HDR_SIZE_BYTES (VTSS_OPT_FDMA_VER == 1) of the SOF DCB's data
 *   area contain the frame's IFH. For VTSS_OPT_FDMA_VER >= 2, the list->ifh_ptr
 *   points to the IFH, which may or may not (depending on architecture) be the
 *   same as the list->data member. The list->frm points to the actual frame data.
 *   If the frame contains a stack header, then it'll be stripped from the frame
 *   (for architectures that carry this in the payload rather than the IFH) and
 *   placed in the IFH prior to callback. The list->frm_ptr points to the actual
 *   frame data.
*/

#if defined(VTSS_FEATURE_FDMA) && VTSS_OPT_FDMA
/* FDMA available and user-enabled. */

// Avoid including vtss_os.h in newer FDMA versions, because it contains
// info to the API, and not info from the API.
#if VTSS_OPT_FDMA_VER <= 2
#include "vtss_os.h"
#endif

/**
 * \brief Frame DMA channel number.
 * Channels are numbered in range [0; VTSS_FDMA_CH_CNT[.
 */
typedef i32 vtss_fdma_ch_t; /* Must be a signed integer, or some loops will not work anymore. */

/*
 * Fundamental constants
 */
#if defined(VTSS_ARCH_LUTON28)
  #define VTSS_FDMA_CH_CNT              8 /**< Number of DMA Channels.                                                                                 */
  #define VTSS_PHYS_PORT_CNT           28 /**< Number of Port Modules, excluding CPU Port Module.                                                      */
  #define VTSS_FDMA_TOTAL_PORT_CNT     29 /**< Number of Port Modules, including CPU Port Module.                                                      */
  #define VTSS_CPU_PM_NUMBER           28 /**< The physical port number of the CPU Port Module.                                                        */
  #define VTSS_FDMA_DCB_SIZE_BYTES     28 /**< Number of bytes in a DCB.                                                                               */
  #if VTSS_OPT_FDMA_VER >= 2
    // When running Luton28 in forward-compatibility mode, we have one
    // single, overall size. This directly allows for using the same
    // list of frame buffers for both injection and extraction.
    // Furthermore, the VStaX header on Luton28 is placed in the payload
    // (after the SMAC), whereas it's sideband information on Jaguar.
    // To have the application work the same on both Luton28 and Jaguar,
    // putting Luton28 in V2 mode (VTSS_OPT_FDMA_VER >= 2), will cause
    // Luton28's FDMA driver to strip the VStaX header from incoming frames
    // and put it in front of the frame. This will require an additional
    // 12 bytes in the header. Finally, the FDMA driver may re-insert a VLAN tag that
    // was possibly stripped by H/W. We therefore get to:
    // XTR: 8 bytes for normal extraction IFH + 12 bytes for VStaX header + 4 bytes re-inserted VLAN tag = 24 bytes.
    // INJ: 8 bytes for normal IFH + 4 bytes for CMD field + 12 bytes for VStaX header (which is inserted after SMAC by FDMA driver) = 24 bytes.
    // To have symmetry in XTR and INJ header sizes, MAX(XTR, INJ) = 24 bytes must be used.
    // Note that seen from the application, the VStaX header is thus inserted as sideband
    // information (vtss_fdma_inj_props_t) as binary. The FDMA driver will put this header
    // in the correct position of the frame and adjust the position of the IFH accordingly.
    #define VTSS_FDMA_HDR_SIZE_BYTES 24 /**< Number of bytes required by FDMA for extraction and injection header. */
  #else
    // If not running V2 of the FDMA driver, the application may choose to discern
    // between header sizes for buffers used for injection and extraction and use
    // the @gap for extraction to have the FDMA insert a number of dummy 32-bit
    // words between the IFH and the payload to get size symmetry.
    #define VTSS_FDMA_IFH_SIZE_BYTES      8 /**< Number of bytes in the Internal Frame Header (still defined for legacy reasons).                        */
    #define VTSS_FDMA_CMD_SIZE_BYTES      4 /**< Number of bytes in the Command field, used when injecting frames (still defined for legacy reasons).    */
    #define VTSS_FDMA_XTR_HDR_SIZE_BYTES  8 /**< Number of bytes in the extraction header (equal to VTSS_FDMA_IFH_SIZE_BYTES).                           */
    #define VTSS_FDMA_INJ_HDR_SIZE_BYTES 12 /**< Number of bytes in the injection header (equal to VTSS_FDMA_IFH_SIZE_BYTES + VTSS_FDMA_CMD_SIZE_BYTES). */
  #endif /* VTSS_OPT_FDMA_VER */
#endif /* defined(VTSS_ARCH_LUTON28) */

#if defined(VTSS_ARCH_LUTON26)
  #define VTSS_FDMA_CH_CNT          8 /**< Number of DMA Channels.                             */
  #define VTSS_PHYS_PORT_CNT       26 /**< Number of Port Modules, excluding CPU Port Module.  */
  #define VTSS_FDMA_DCB_SIZE_BYTES 24 /**< Number of bytes in a DCB.                           */
  #define VTSS_FDMA_HDR_SIZE_BYTES 16 /**< Max(XTR_HDR_SZ, INJ_HDR_SZ). Worst-case is INJ (8 for IFH, vtss_fdma_inj_props_t::ptp_action != 0 => +4 for timestamp. \@switch_frm == TRUE => +4 for VLAN tag) */
#endif /* defined(VTSS_ARCH_LUTON26) */

#if defined(VTSS_ARCH_SERVAL)
  #define VTSS_FDMA_CH_CNT          4 /**< Number of DMA Channels. Fixed layout                */
  #define VTSS_PHYS_PORT_CNT       11 /**< Number of Port Modules, excluding CPU Port Module.  */
  #define VTSS_FDMA_DCB_SIZE_BYTES 16 /**< Number of bytes in a DCB.                           */
  #define VTSS_FDMA_HDR_SIZE_BYTES 20 /**< Max(XTR_HDR_SZ, INJ_HDR_SZ). Worst-case is INJ (16 for IFH + 4 for VLAN tag (if \@switch_frm == TRUE) */
#endif /* defined(VTSS_ARCH_SERVAL) */

#if defined(VTSS_ARCH_JAGUAR_1)
  #define VTSS_FDMA_CH_CNT          8 /**< Number of DMA Channels.                                                */
  #define VTSS_PHYS_PORT_CNT       32 /**< Number of Port Modules, excluding CPU Port Module.                     */
  #define VTSS_FDMA_DCB_SIZE_BYTES 24 /**< Number of bytes in a DCB.                                              */
  #define VTSS_FDMA_HDR_SIZE_BYTES 52 /**< Max(XTR_HDR_SZ, INJ_HDR_SZ). Worst-case is XTR: 24 bytes for IFH + 4 bytes for queue number + 24 bytes for Rx super-prio artificial IFH).
                                       * In FDMA API v3+, also injection requires 52 bytes: 24 bytes for IFH + 4 bytes for VLAN tag + 24 bytes for a signature IFH used for multicast injections on 48-ported */
#endif /* defined(VTSS_ARCH_JAGUAR_1) */

/*
 * These do not appear in the Doxygen docs...
 *
 * Unfortunately, there are a few bugs in the FDMA silicon. These are cir-
 * cumvented in S/W, unless the relevant VTSS_FDMA_IGNORE_GNATS_xxxx
 * are defined (The xxxx refers to a number in Vitesse's internal bug-tracking
 * system, GNATS).
 * The main-purpose for having these in the code is to be able to ignore the
 * GNATS to provoke the error during testing. Normally these should *not* be
 * defined when compiling vtss_fdma.c.
 *
 * VTSS_FDMA_IGNORE_GNATS_5299:
 *   When receiving a frame and the frame uses one single DCB, and the DCB's
 *   data area is X bytes, and the frame size is in the interval [X-3; X],
 *   then the DCB's SOF flag is not set by the FDMA silicon. By defining
 *   this, the FDMA driver code will check for the SOF flag and assert that
 *   it is set when expecting a SOF.
 *
 * VTSS_FDMA_IGNORE_GNATS_5355:
 *   This GNATS states that the sizeof(non-EOFs' data area) must be a multiple
 *   of 4. This is relevant to injection only, and means that if a frame is
 *   split across several DCBs, then all but the last DCB's data area must have
 *   a size that is a multiple of 4. Define this if you want to allow user-
 *   code to inject invalid-sized DCBs.
 *
 * VTSS_FDMA_IGNORE_GNATS_5376:
 *   This GNATS states that the minimum size of the SOF DCB's data area must
 *   be sizeof(IFH+CMD)+1 byte. This is relevant to injection only, and means
 *   that if a frame is split across several DCBs, then the first DCB's data
 *   area's size must be at least IFH+CMD+1 byte large. Define this if you
 *   want to allow user-code to inject invalid-sized SOF DCB data areas.
 *
 * VTSS_FDMA_IGNORE_GNATS_5418
 *   When transmitting a frame from the CPU port module using the DO_ANALYZE
 *   flag, then the frame is looped back to the CPU, even if the ANA::SRCMASKS[CPU_PM]
 *   doesn't include the CPU Port Module itself. So to avoid frames from being
 *   looped back to the application, this should *not* be defined. In that
 *   case, the FDMA driver code will check if the frame was transmitted from
 *   the CPU itself (it checks the IFH's source port field), and count and
 *   discard it if so. Define this, if you don't want the driver code to do
 *   this check.
 */

/**
 * \def VTSS_FDMA_MIN_DATA_PER_INJ_SOF_DCB_BYTES
 *   Defines the minimum size in bytes of an injection SOF-DCB's associated
 *   data area.
 *
 * \def VTSS_FDMA_MIN_DATA_PER_XTR_SOF_DCB_BYTES
 *   Defines the minimum size in bytes of an extraction SOF-DCB's associated
 *   data area. Since every DCB can become a SOF DCB, this value also defines
 *   the minimum size that the user must allocate per DCB.
 *
 * \def VTSS_FDMA_MIN_DATA_PER_NON_SOF_DCB_BYTES
 *   Defines the minimum size in bytes of an injection/extraction non-SOF-
 *   DCB's associated data area.
 *
 * \def VTSS_FDMA_MAX_DATA_PER_DCB_BYTES
 *   For both injection and extraction, the maximum number of bytes that
 *   one single DCB's associated data area can refer to. If you need to
 *   inject or extract larger frames, use multiple DCBs.
 *
 * \def VTSS_FDMA_MIN_FRAME_SIZE_BYTES
 *   The minimum allowed frame size (excluding IFH and CMD fields, but including FCS) in bytes.
 *   This is defined for legacy reasons. The FDMA will automatically adjust any frame below
 *   the minimum ethernet size to the minimum ethernet size before transmission.
 *
 * \def VTSS_FDMA_MAX_FRAME_SIZE_BYTES
 *   The maximum allowed total frame size (excluding IFH and CMD fields) in
 *   bytes.
 */
#if defined(VTSS_ARCH_LUTON28)
  #if VTSS_OPT_FDMA_VER == 1
    #if defined(VTSS_FDMA_IGNORE_GNATS_5376)
      #define VTSS_FDMA_MIN_DATA_PER_INJ_SOF_DCB_BYTES (VTSS_FDMA_INJ_HDR_SIZE_BYTES)
    #elif defined(VTSS_FDMA_IGNORE_GNATS_5355)
      #define VTSS_FDMA_MIN_DATA_PER_INJ_SOF_DCB_BYTES (VTSS_FDMA_INJ_HDR_SIZE_BYTES + 1)
    #else
      #define VTSS_FDMA_MIN_DATA_PER_INJ_SOF_DCB_BYTES (VTSS_FDMA_INJ_HDR_SIZE_BYTES + 4) /* This is the working case with EStax34 */
    #endif /* defined(VTSS_FDMA_IGNORE_GNATS_5355) */
    #define VTSS_FDMA_MIN_DATA_PER_XTR_SOF_DCB_BYTES  (VTSS_FDMA_XTR_HDR_SIZE_BYTES)
  #endif /* VTSS_OPT_FDMA_VER == 1 */
  #define VTSS_FDMA_MAX_DATA_PER_DCB_BYTES         4092
#endif /* defined(VTSS_ARCH_LUTON28) */

#if defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
  #define VTSS_FDMA_MAX_DATA_PER_DCB_BYTES         16380
#endif /* LUTON26/JAGUAR_1/SERVAL */

#define VTSS_FDMA_MIN_FRAME_SIZE_BYTES             64

#if VTSS_OPT_FDMA_VER >= 2
  #if defined(VTSS_FEATURE_VSTAX)
    #define VTSS_FDMA_MIN_DATA_PER_INJ_SOF_DCB_BYTES (VTSS_FDMA_HDR_SIZE_BYTES + VTSS_VSTAX_HDR_SIZE)
    #define VTSS_FDMA_MIN_DATA_PER_XTR_SOF_DCB_BYTES (VTSS_FDMA_HDR_SIZE_BYTES + VTSS_VSTAX_HDR_SIZE)
  #else
    #define VTSS_FDMA_MIN_DATA_PER_INJ_SOF_DCB_BYTES (VTSS_FDMA_HDR_SIZE_BYTES)
    #define VTSS_FDMA_MIN_DATA_PER_XTR_SOF_DCB_BYTES (VTSS_FDMA_HDR_SIZE_BYTES)
  #endif /* defined(VTSS_FEATURE_VSTAX) */
#endif /* VTSS_OPT_FDMA_VER >= 2 */
#define VTSS_FDMA_MIN_DATA_PER_NON_SOF_DCB_BYTES    1
#define VTSS_FDMA_MAX_FRAME_SIZE_BYTES          10000

#if defined(VTSS_ARCH_LUTON28)
/**
 * @{
 * When injecting frames directly to a front port (i.e. not using the CPU Port
 * Module), the FDMA silicon must insert waits between data writes to the queue.
 * These waits are defined by VTSS_FDMA_INJ_WR_DELAY_1G_PORTS, which is
 * the time to wait for 1Gbps front ports, and VTSS_FDMA_INJ_WR_DELAY_2_5G_PORTS,
 * which is the time to wait for 2.5Gbps front ports. These values are written
 * to the CFG.FDMA_INJ_CFG[ch].INJ_WR_DELAY register prior to each frame
 * injection (if the destination port changes).
 *
 * The default values assigned below allows for full traffic on the 1Gbps front
 * ports without injection failing (unless the front port is in flow control
 * mode and the super priority queue is full). Unfortunately there is a bug
 * in the FDMA silicon that causes frame injection to fail on the 2.5Gbps
 * ports if the port is fully loaded with other traffic while the FDMA is
 * injecting frames to it. The highest possible value to write to the
 * INJ_WR_DELAY register is not high enough to accommodate the value that should
 * have been written. This means that injection to the 2.5Gbps front ports may
 * have to be done manually (see also vtss_fdma_inj_flow_ctrl_cfg_set()).
 */
  #if !defined(VTSS_FDMA_INJ_WR_DELAY_1G_PORTS)
    #define VTSS_FDMA_INJ_WR_DELAY_1G_PORTS 24
  #endif /* !defined(VTSS_FDMA_INJ_WR_DELAY_1G_PORTS) */
  #if !defined(VTSS_FDMA_INJ_WR_DELAY_2_5G_PORTS)
    #define VTSS_FDMA_INJ_WR_DELAY_2_5G_PORTS 31
  #endif /* !defined(VTSS_FDMA_INJ_WR_DELAY_2_5G_PORTS) */
/*@}*/
#endif /* defined(VTSS_ARCH_LUTON28) */

#if defined(VTSS_ARCH_LUTON28)
/* Position of various fields in the IFH. */
/* DEPRECATED. On other architectures, use the vtss_fdma_xtr_hdr_decode() function */
  #define VTSS_FDMA_IFH_F_STACK_HDR_FPOS    63 /**< Position of stack header present bit within IFH.      */
  #define VTSS_FDMA_IFH_F_LEN_FPOS          48 /**< Position of frame length field within IFH.            */
  #define VTSS_FDMA_IFH_F_SRC_PORT_FPOS     42 /**< Position of source port field within IFH.             */
  #define VTSS_FDMA_IFH_F_UPRIO_FPOS        29 /**< Position of uprio field within IFH.                   */
  #define VTSS_FDMA_IFH_F_CFI_FPOS          28 /**< Position of CFI bit within IFH.                       */
  #define VTSS_FDMA_IFH_F_VID_FPOS          16 /**< Position of VID field within IFH.                     */
  #define VTSS_FDMA_IFH_F_TAG_FPOS          16 /**< Position of VID+CFI+UPRIO field within IFH.           */
  #define VTSS_FDMA_IFH_F_STACK_LINK_B_FPOS 14 /**< Position of stack link B field within IFH.            */
  #define VTSS_FDMA_IFH_F_STACK_LINK_A_FPOS 13 /**< Position of stack link A field within IFH.            */
  #define VTSS_FDMA_IFH_F_AGGR_CODE_FPOS    10 /**< Position of aggregation code field within IFH. Can only be used if sending frame via CPU PM and switching it */
  #define VTSS_FDMA_IFH_F_TAG_TYPE_FPOS      9 /**< Position of tag_type bit within IFH.                  */
  #define VTSS_FDMA_IFH_F_FRM_TYPE_FPOS      6 /**< Position of frame type field within IFH.              */
  #define VTSS_FDMA_IFH_F_WAS_TAGGED_FPOS    5 /**< Position of was_tagged bit within IFH.                */
  #define VTSS_FDMA_IFH_F_LEARN_FPOS         4 /**< Position of learn bit within IFH.                     */
  #define VTSS_FDMA_IFH_F_XTR_QU_FPOS        2 /**< Position of extraction queue number field within IFH. */
  #define VTSS_FDMA_IFH_F_QOS_FPOS           0 /**< Position of QoS field within IFH.                     */

  /******************************************************************************
   * Length of various fields in the IFH
   *****************************************************************************/
  #define VTSS_FDMA_IFH_F_STACK_HDR_FLEN     1 /**< Length of stack header present bit within IFH.      */
  #define VTSS_FDMA_IFH_F_LEN_FLEN          14 /**< Length of length field within IFH.                  */
  #define VTSS_FDMA_IFH_F_SRC_PORT_FLEN      5 /**< Length of source port field within IFH.             */
  #define VTSS_FDMA_IFH_F_UPRIO_FLEN         3 /**< Length of uprio field within IFH.                   */
  #define VTSS_FDMA_IFH_F_CFI_FLEN           1 /**< Length of CFI field within IFH.                     */
  #define VTSS_FDMA_IFH_F_VID_FLEN          12 /**< Length of VID field within IFH.                     */
  #define VTSS_FDMA_IFH_F_TAG_FLEN          16 /**< Length of VID+CFI+UPRIO field within IFH.           */
  #define VTSS_FDMA_IFH_F_STACK_LINK_B_FLEN  1 /**< Length of stack link B field within IFH.            */
  #define VTSS_FDMA_IFH_F_STACK_LINK_A_FLEN  1 /**< Length of stack link A field within IFH.            */
  #define VTSS_FDMA_IFH_F_AGGR_CODE_FLEN     4 /**< Length of aggregation code field within IFH. Can only be used if sending frame via CPU PM and switching it */
  #define VTSS_FDMA_IFH_F_TAG_TYPE_FLEN      1 /**< Length of tag_type field within IFH.                */
  #define VTSS_FDMA_IFH_F_FRM_TYPE_FLEN      3 /**< Length of frame type field within IFH.              */
  #define VTSS_FDMA_IFH_F_WAS_TAGGED_FLEN    1 /**< Length of was_tagged field within IFH.              */
  #define VTSS_FDMA_IFH_F_LEARN_FLEN         1 /**< Length of learn field within IFH.                   */
  #define VTSS_FDMA_IFH_F_XTR_QU_FLEN        2 /**< Length of extraction queue number field within IFH. */
  #define VTSS_FDMA_IFH_F_QOS_FLEN           2 /**< Length of QoS field within IFH.                     */

  /******************************************************************************
   * Register definitions exposed to the public, so that they can use them for
   * manual injection, if needed.
   *****************************************************************************/
  /* First a few macros needed to be able to define the real register addresses. */
  #define VTSS_TB_SWC 0xA0000000                                                                                 /**< Offset of SwC Register space                        */
  #define BLK_SUBBLK_ADDR(b,s,a) ((b<<14) | (s<<10) | (a<<2))                                                    /**< Blk/Subblk/Addr macro                               */
  #define DEV_TO_B(dev) ((dev)<16?1:6)                                                                           /**< Computes block from device                          */
  #define DEV_TO_S(dev) ((dev)<16?(dev):((dev)-16))                                                              /**< Computes subblock from device                       */

  /* Register addresses that can be used when performing manual frame injection. */
  #define VTSS_DEV_CPUTXDAT(dev) VTSS_REGISTER(SWC,BLK_SUBBLK_ADDR(DEV_TO_B(dev),DEV_TO_S(dev),0xC0))            /**< Address of DEV::CPUTXDAT register                   */
  #define VTSS_DEV_MISCFIFO(dev) VTSS_REGISTER(SWC,BLK_SUBBLK_ADDR(DEV_TO_B(dev),DEV_TO_S(dev),0xC4))            /**< Address of DEV::MISCFIFO register                   */
  #define VTSS_DEV_MISCSTAT(dev) VTSS_REGISTER(SWC,BLK_SUBBLK_ADDR(DEV_TO_B(dev),DEV_TO_S(dev),0xC8))            /**< Address of DEV::MISCSTAT register                   */

  /* DEV::MISCSSTAT fields */
  #define VTSS_F_MISCSTAT_CPU_TX_QUEUE_FULL    VTSS_BIT(9)                                                       /**< Position of DEV::MISCSTAT::CPU_TX_QUEUE_FULL bit    */
  #define VTSS_F_MISCSTAT_CPU_TX_DATA_PENDING  VTSS_BIT(8)                                                       /**< Position of DEV::MISCSTAT::CPU_TX_DATA_PENDING bit  */
  #define VTSS_F_MISCSTAT_CPU_TX_DATA_OVERFLOW VTSS_BIT(7)                                                       /**< Position of DEV::MISCSTAT::CPU_TX_DATA_OVERFLOW bit */

  /* DEV::MISCFIFO fields */
  #define VTSS_F_MISCFIFO_CLEAR_STICKY_BITS VTSS_BIT(2)                                                          /**< Position of DEV::MISCFIFO::CLEAR_STICKY_BITS bit    */
  #define VTSS_F_MISCFIFO_REWIND_CPU_TX     VTSS_BIT(1)                                                          /**< Position of DEV::MISCFIFO::REWIND_CPU_TX bit        */
  #define VTSS_F_MISCFIFO_CPU_TX            VTSS_BIT(0)                                                          /**< Position of DEV::MISCFIFO::CPU_TX bit               */

  /* Registers for reading out CPU Port Module counters. */
  #define VTSS_DEV_C_CNTDATA(dev)           VTSS_REGISTER(SWC,BLK_SUBBLK_ADDR(DEV_TO_B(dev),DEV_TO_S(dev),0xDC)) /**< Address of DEV::C_CNTDATA register                  */
  #define VTSS_DEV_C_CNTADDR(dev)           VTSS_REGISTER(SWC,BLK_SUBBLK_ADDR(DEV_TO_B(dev),DEV_TO_S(dev),0xDD)) /**< Address of DEV::C:CNTADDR register                  */
#endif /* defined(VTSS_ARCH_LUTON28) */

#if VTSS_OPT_FDMA_VER <= 2
#if VTSS_OS_DCACHE_LINE_SIZE_BYTES <= VTSS_FDMA_DCB_SIZE_BYTES
#error "Internal Error. Rewrite unused member of vtss_fdma_list_t to support H/W DCBs bigger than VTSS_OS_DCACHE_LINE_SIZE_BYTES"
#endif /* VTSS_OS_DCACHE_LINE_SIZE_BYTES <= VTSS_FDMA_DCB_SIZE_BYTES */
#endif /* VTSS_OPT_FDMA_VER <= 2 */

/**
 * \brief Software DCB structure.
 *
 * The following structure defines a software DCB. Software DCBs can be linked
 * together to form a list of software DCBs. This is the fundamental data
 * structure used to transfer information between a user-level application and
 * the FDMA driver code.
 *
 * The structure holds the actual hardware DCB needed by the FDMA silicon,
 * a pointer to the associated data area, and other properties such as frame
 * length and allocation length.
 *
 * The structure is used for both injection and extraction, but some of its
 * members' meaning change slightly in the two cases. The exact interpretation
 * is shown as comments inside the definition below.
 */
typedef struct tag_vtss_fdma_list {
#if VTSS_OPT_FDMA_VER <= 2
    /**
     * <B>XTR/INJ/AFI:</B>\n
     *   Reserved by vtss_fdma.c. The FDMA itself only requires this field to be 4-byte aligned, but
     *   if the dcache is enabled, it's safe to have it cache-line aligned, i.e. 32-byte aligned,
     *   especially if you allocate an array of vtss_fdma_list_t items, because both the CPU and
     *   the FDMA may update two adjacent items simultaneously, and because the cache invalidate
     *   function typically writes back the start cache line if it's not cache-line aligned.
     */
    u8 hw_dcb[VTSS_FDMA_DCB_SIZE_BYTES] VTSS_OS_COMPILER_ATTRIBUTE_ALIGNED(VTSS_OS_DCACHE_LINE_SIZE_BYTES);

    /**
     * Always make the H/W DCB take up a full cache line, or invalidating it might
     * also invalidate any data written to subsequent field in this structure.
     */
    u8 unused[VTSS_OS_DCACHE_LINE_SIZE_BYTES - VTSS_FDMA_DCB_SIZE_BYTES]; // Works as long as this gives a positive number.
#endif /* VTSS_OPT_FDMA_VER <= 2 */

#if VTSS_OPT_FDMA_VER <= 2
    /**
     * <B>XTR:</B>\n
     *   VIRTUAL ADDRESS. Must point to a pre-allocated area of alloc_len bytes. Must be 32-bit aligned. Set by user.\n
     * <B>INJ/AFI:</B>\n
     *   VIRTUAL ADDRESS. Must point to a pre-allocated area of act_len bytes. For SOF DCB,
     *   it must point to the IFH. Need only be byte-aligned. Set by user.
     */
    u8 *data;
#endif /* VTSS_OPT_FDMA_VER <= 2 */

#if VTSS_OPT_FDMA_VER <= 2
    /**
     * <B>XTR/INJ/AFI:</B>\n
     *   VIRTUAL ADDRESS. This points to the first byte of the XTR/INJ IFH. Set by FDMA driver.
     *   This is useful for architectures with variable-length XTR/INJ headers
     *   or in cases where the reserved space in front of the packet is made
     *   such that the same data area can be used for both injection and
     *   extraction (VTSS_OPT_FDMA_VER >= 2).
     */
    u8 *ifh_ptr;
#endif /* VTSS_OPT_FDMA_VER <= 2 */

#if VTSS_OPT_FDMA_VER <= 2
    /**
     * <B>XTR/INJ/AFI:</B>\n
     *   VIRTUAL ADDRESS. This points to the first byte of the frame. Set by FDMA driver.\n
     *   For SOF DCBs, this corresponds to the first byte of the DMAC. For non-SOF DCBs
     *   it points to the first byte of the continued frame.
     *   This is useful for architectures with variable-length XTR/INJ headers
     *   or in cases where the reserved space in front of the packet is made
     *   such that the same data area can be used for both injection and
     *   extraction (VTSS_OPT_FDMA_VER >= 2).
     *   The actual size of the IFH can be found by subtracting ifh_ptr from frm_ptr.
     */
    u8 *frm_ptr;
#else
    /**
     * <B>XTR:</B>\n
     *   This points to the first byte of the frame. Set by FDMA driver.\n
     *   For SOF DCBs, this corresponds to the first byte of the DMAC. For non-SOF DCBs
     *   it points to the first byte of the continued frame.
     *
     * <B>INJ/AFI:</B>\n
     *   This points to the first byte of the frame. Set by application.
     *   For SOF DCBs, VTSS_FDMA_HDR_SIZE_BYTES of head room must be available
     *   just before the #frm_ptr.
     *   The FDMA driver may update this internally, so don't rely on its value
     *   after injection.
     */
    u8 *frm_ptr;
#endif /* VTSS_OPT_FDMA_VER */

#if VTSS_OPT_FDMA_VER <= 2
    /**
     * <B>XTR:</B>\n
     *   Holds the number of bytes extracted into #ifh_ptr. In case of a fragmented frame, the receiver of the frame
     *   must sum up all the act_len in the list of fragments to obtain the full frame size.
     *   For the SOF DCB, act_len includes the size of the IFH and possible start gap, and for EOF DCB,
     *   also the size of the FCS and possible end gap. Set by FDMA driver.\n
     * <B>INJ/AFI:</B>\n
     *   The number of bytes held in associated data area. For the SOF DCB, it includes the size of the IFH
     *   and CMD fields, and for the EOF DCB, also the size of the FCS. Set by user.
     */
    u32 act_len;
#else
    /**
     * <B>XTR:</B>\n
     *   Used internally by the FDMA driver (holds length incl. IFH, frame, and FCS).
     * <B>INJ/AFI:</B>\n
     *   The number of frame bytes to be injected from #frm_ptr for this fragment.
     *   For the SOF DCB, it does not include the size of IFH - only true frame data.
     *   for the EOF DCB, it does not include the size of the FCS.
     *   The FDMA driver may update this internally, so don't rely on its value
     *   after injection.
     */
    u32 act_len;
#endif /* VTSS_OPT_FDMA_VER */

#if VTSS_OPT_FDMA_VER >= 3
    /**
     * <B>XTR:</B>\n
     *   Pointer to allocated frame + meta data. Either set by
     *   application during calls to rx_alloc_cb() or by the FDMA
     *   driver itself if memory management is entirely handled
     *   by the FDMA driver.
     * <B>INJ/AFI:</B>\n
     *   Not used.
     */
    void *alloc_ptr;
#endif /* VTSS_OPT_FDMA_VER >= 3 */

#if VTSS_OPT_FDMA_VER >= 3
    /**
     * <B>XTR:</B>\n
     *   Pointer to decoded extraction header. The allocation of this
     *   is taken care of by the FDMA driver. Only valid in SOF DCB.
     * <B>INJ/AFI:</B>\n
     *   Not used.
     */
    vtss_packet_rx_info_t *rx_info;
#endif /* VTSS_OPT_FDMA_VER >= 3 */

#if VTSS_OPT_FDMA_VER <= 2
    /**
     * <B>XTR:</B>\n
     *   The number of allocated bytes in ->data. Set by user.\n
     * <B>INJ:</B>\n
     *   Not used.
     * <B>AFI:</B>\n
     *   Internally used by the FDMA driver to hold various information.\n
     */
    u32 alloc_len;
#endif /* VTSS_OPT_FDMA_VER <= 2 */

#if defined(VTSS_ARCH_LUTON28) && VTSS_OPT_FDMA_VER <= 2
    /**
     * <B>XTR:</B>\n
     *   Not used.\n
     * <B>INJ:</B>\n
     *   The physical port number that this frame is sent to. Need only be set in SOF DCB.
     *
     * On architectures different from Luton28, the physical port(s) that this frame is
     * sent to are controlled through vtss_fdma_inj_props_t::port_mask and
     * vtss_fdma_inj_props_t::switch_frm members.
     */
    vtss_phys_port_no_t phys_port;
#endif /* defined(VTSS_ARCH_LUTON28) && VTSS_OPT_FDMA_VER <= 2 */

#if VTSS_OPT_FDMA_VER <= 2
    /**
     * <B>XTR:</B>\n
     *   The FDMA driver code time-stamps the packet when the vtss_fdma_irq_handler()
     *   gets invoked based on an extraction interrupt.\n
     * <B>INJ:</B>\n
     *   The FDMA driver code time-stamps the packet when the vtss_fdma_irq_handler()
     *   gets invoked based on an injection interrupt.\n.
     * <B>AFI:</B>\n
     *   Unused.
     * The FDMA driver is agnostic to the time stamp format, and it's up to
     * the platform header (vtss_os.h) to define appropriate types and functions
     * for obtaining the time stamp.
     */
    VTSS_OS_TIMESTAMP_TYPE timestamp;
#else
    /**
     * <B>XTR:</B>\n
     *    Unused. In V3+, it's part of the vtss_packet_rx_info_t structure and is called sw_tstamp.
     * <B>INJ:</B>\n
     *   The FDMA driver code time-stamps the packet when the vtss_fdma_irq_handler()
     *   gets invoked based on an injection interrupt.\n.
     * <B>AFI:</B>\n
     *   Unused.
     * The FDMA driver is agnostic to the time stamp format, and it's up to
     * the platform header (vtss_os.h) to define appropriate types and functions
     * for obtaining the time stamp.
     */
    VTSS_OS_TIMESTAMP_TYPE sw_tstamp;
#endif /* VTSS_OPT_FDMA_VER */

    /**
     * <B>XTR/INJ/AFI:</B>\n
     *   A pointer to any user data. Set by user and used only by the user. The FDMA code doesn't touch nor uses it.
     */
    void *user;

#if VTSS_OPT_FDMA_VER <= 2
    // V3+: Part of vtss_fdma_cfg_t (tx_done_cb())
    /**
     * <B>XTR:</B>\n
     *   Not used (a per-queue (Lu28) or per-channel (JR + Lu26) callback function is configured instead).\n
     * <B>INJ:</B>\n
     *   Callback function called when a frame has been transmitted. Must be set by user in SOF list item.
     *   Not used for non-SOF items, since the callback is only called once per frame.\n
     *     - cntxt:   The value of the parameter passed into the OS-specific call to vtss_fdma_irq_handler().\n
     *     - list:    The list of software-DCBs constituting the frame that has just been injected.\n
     *     - ch:      The FDMA channel that this frame was injected through.\n
     *     - dropped: When FALSE, the frame was successfully transmitted, otherwise an error occurred. Errors can
     *                only occur if injecting to a front port and the INJ_WR_DELAY is not set high
     *                enough, or if the front port is in flow control mode. See also vtss_fdma_inj_flow_ctrl_cfg_set().
     * <B>AFI:</B>\n
     *   Callback function called when a AFI frame has been successfully cancelled transmission. It allows the application
     *   to free the associated memory (DCBs and frame data) once the FDMA is done using it.
     *   The parameters passed in the call to the callback function are the same as the ones passed
     *   for injection (see above). The \@dropped parameter will always be FALSE for AFI, though.
     */
    void (*inj_post_cb)(void *cntxt, struct tag_vtss_fdma_list *list, vtss_fdma_ch_t ch, BOOL dropped);
#endif /* VTSS_OPT_FDMA_VER <= 2 */

#if defined(VTSS_FEATURE_AFI_SWC) && VTSS_OPT_FDMA_VER <= 2
    /**
     * <B>XTR/INJ:</B>\n
     *    Not used.\n
     * <B>AFI:</B>
     *   Indicates whether this is a switch-core-based periodically
     *   injected frame. If #afi_id == VTSS_AFI_ID_NONE, it's not
     *   AFI-based.
     */
    vtss_afi_id_t afi_id;
#endif /* defined(VTSS_FEATURE_AFI_SWC) && VTSS_OPT_FDMA_VER <= 2 */

#if defined(VTSS_FEATURE_AFI_FDMA)
#if VTSS_OPT_FDMA_VER <= 2
    /**
     * <B>XTR/INJ</B>\n
     *   Not used.
     * <B>AFI</B>\n
     *   Internal parameter. Holds the number of frames to increment #ccm_frm_cnt by for
     *   each interrupt, when the AFI channel is counting.
     *
     * Validity:
     *   Luton26: Y
     *   Luton28: N
     *   Jaguar1: Y
     *   Serval : Y (TBD)
     */
    u32 afi_frm_incr;

    /**
     * <B>XTR/INJ</B>\n
     *   Not used.
     * <B>AFI</B>\n
     *   Internal parameter. Holds the number of frames to increment #ccm_frm_cnt by
     *   after an interrupt. The reason for this parameter is that the increment may
     *   change if the AFI channel in question will serve a different list of frames
     *   (because an existing is cancelled or a new frame is added) after the interrupt.
     *   The interrupt handler will then update #afi_frm_incr with the value of
     *   #afi_frm_incr_next.
     *
     * Validity:
     *   Luton26: Y
     *   Luton28: N
     *   Jaguar1: Y
     *   Serval : Y (TBD)
     */
    u32 afi_frm_incr_next;

    /**
     * <B>XTR/INJ</B>\n
     *   Not used.
     * <B>AFI</B>\n
     *   Internal parameter. Temporary that holds the next number of repetitions
     *   of this frame. If all resources required are available, this number will
     *   be copied to #afi_frm_incr_next.
     *
     * Validity:
     *   Luton26: Y
     *   Luton28: N
     *   Jaguar1: Y
     *   Serval : Y (TBD)
     */
    u32 afi_frm_incr_temp;

    /**
     * <B>XTR/INJ</B>\n
     *   Not used.
     * <B>AFI</B>\n
     *   Output parameter. Holds the number of frames that was actually transmitted.
     *   Updated regularly, but is only 100% correct once the AFI injection is cancelled
     *   and the tx_done_cb() is called.
     *
     * Validity:
     *   Luton26: Y
     *   Luton28: N
     *   Jaguar1: Y
     *   Serval : Y (TBD)
     */
    u64 ccm_frm_cnt;

    /**
     * <B>XTR/INJ</B>\n
     *   Not used.
     * <B>AFI</B>\n
     *   Holds the next sequence number to put in a given frame.
     *   Updated repeatedly by S/W when AFI sequence numbering is enabled.
     *
     * Validity:
     *   Luton26: Y
     *   Luton28: N
     *   Jaguar1: Y
     *   Serval : Y (TBD)
     */
    u32 ccm_seq_number;

    /**
     * <B>XTR/INJ:</B>\n
     *   Not used.
     * <B>AFI:</B>\n
     *   Internally used to link AFI frames while adding or deleting AFI frames from a
     *   given AFI-enabled channel. This is in order not to disturb the interrupt handler
     *   in this process, so that interrupts can be kept enabled while updating
     *   the new list of DCBs. Worst case, it takes around 150 ms to create a list with
     *   e.g. 300,000 frames per second that require frame counting and sequence numbering.
     *   It's unacceptable to have interrupts diabled (or the scheduler locked) for such
     *   a long time, since existing frames will not be counted and sequence-number updated then.
     *
     * Validity:
     *   Luton26: Y
     *   Luton28: N
     *   Jaguar1: Y
     *   Serval : Y (TBD)
     */
    struct tag_vtss_fdma_list *afi_next;
#else /* VTSS_OPT_FDMA_VER >= 3 below */
    /**
     * <B>XTR/INJ</B>\n
     *   Not used.
     * <B>AFI</B>\n
     *   Output parameter. Holds the number of frames that was actually transmitted.
     *   Updated regularly, but is only 100% correct once the AFI injection is cancelled
     *   and the tx_done_cb() is called.
     *
     * Validity:
     *   Luton26: Y
     *   Luton28: N
     *   Jaguar1: Y
     *   Serval : Y (TBD)
     */
    u64 afi_frm_cnt;

    /**
     * <B>XTR/INJ</B>\n
     *   Not used.
     * <B>AFI</B>\n
     *   Holds the next sequence number to put in a given frame.
     *   Updated repeatedly by S/W when AFI sequence numbering is enabled.
     *
     * Validity:
     *   Luton26: Y
     *   Luton28: N
     *   Jaguar1: Y
     *   Serval : Y (TBD)
     */
    u32 afi_seq_number;
#endif /* VTSS_OPT_FDMA_VER */
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

    /**
     * <B>XTR:</B>\n
     *   Points to the next entry in the list or NULL if it's the last. Set by user on initialization of list.
     *   Continuously updated by vtss_fdma.c afterwards.\n
     * <B>INJ:</B>\n
     *   Points to the next fragment of the frame and set by user on a per-frame basis. Last fragment of a frame
     *   must set ->next to NULL. Once handed to vtss_fdma.c, the driver code takes over.
     * <B>AFI:</B>\n
     *   Must be NULL (AFI frames must be contained in one fragment (due to injection from multiple GPDMA channels
     *   into the same injection group)).
     *   Internally the FDMA driver uses it to link multiple user AFI frames onto the same AFI channel.
     */
    struct tag_vtss_fdma_list *next;
} vtss_fdma_list_t;

#if (defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC)) && VTSS_OPT_FDMA_VER >= 3
/** \brief Select which AFI to use.
 *
 * On platforms that only support either an FDMA-based or a switch-core-based AFI,
 * select the type it supports or VTSS_FDMA_AFI_TYPE_AUTO, which will resort to
 * the one it supports.
 *
 * On platforms that support both AFI types, selecting VTSS_FDMA_AFI_TYPE_AUTO
 * will cause the FDMA to look at FDMA-only properties, that is, whether
 * frame counting or sequence numbering is enabled, and if so choose the FDMA-based,
 * otherwise it will choose the switch-core based.
 */
typedef enum {
     VTSS_FDMA_AFI_TYPE_AUTO = 0, /**< The FDMA driver chooses an appropriate frame injector. */
     VTSS_FDMA_AFI_TYPE_FDMA,     /**< Tell FDMA driver to use the FDMA-based AFI.            */
     VTSS_FDMA_AFI_TYPE_SWC,      /**< Tell FDMA driver to use the switch-core-based AFI.     */
} vtss_fdma_afi_type_t;
#endif /* (defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC)) && VTSS_OPT_FDMA_VER >= 3 */

#if VTSS_OPT_FDMA_VER >= 3
/**
 * \brief FDMA Injection Properties.
 */
typedef struct {
    /**
     * Any user data. Is passed in a call to \@pre_cb().
     */
    void *pre_cb_ctxt1;

    /**
     * Any user data. Is paased in a call to \@pre_cb().
     */
    void *pre_cb_ctxt2;

    /**
     * Callback function called just before the frame is handed over to the FDMA H/W. If NULL, no callback
     * will occur.
     * The called back function may change frame data, nothing else.
     * The called back function *MUST* execute fast and without waiting for synchronization primitives,
     * i.e. no waits are allowed. When called back, interrupts may be disabled.
     * Not used for non-SOF items, since the callback is only called once per frame.\n
     *
     * Implementation notes:
     *   Due to shuffling of beginning-of-frame-fields, this will not work for the following architectures
     *   under the specified circumstances:
     *   Luton26: This will not work for switched frames.
     *   Serval : This will not work for switched frames.
     *   Luton28: This will not work for frames including stack header if VTSS_OPT_FDMA_VER >= 2.
     *
     * Parameters:
     *   - ctxt1:     The value of \@pre_cb_ctxt1.\n
     *   - ctxt2:     The value of \@pre_cb_ctxt2.\n
     *   - list:      Pointer to the SOF item.
     * Validity:
     *   Luton26: Y
     *   Luton28: Y
     *   Jaguar1: Y
     *   Serval : Y
     */
    void (*pre_cb)(void *ctxt1, void *ctxt2, vtss_fdma_list_t *list);

#if defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC)
    /**
     * Number of times this frame will be injected per second.
     *
     \if defined(VTSS_FEATURE_AFI_FDMA)
     * For FDMA-based AFI, the following applies:
     *
     * Note that there is a limited bandwidth out of the DDR2 SDRAM,
     * through the Shared Bus, and into the FDMA, which also depends on
     * the CPU load. This means that there's an upper limit on the number
     * of frames you can inject per second. Not only does this depend
     * on the frame sizes and their frequencies, but also on H/W and
     * the application's CPU usage (since the CPU shares the Shared Bus
     * with the FDMA). The FDMA driver will not attempt to check that the
     * requested, accumulated bandwidth is greater than this limit, so
     * it's up to the application to determine what makes sense to inject.
     *
     \endif
     * There is a number of restrictions on AFI frames:
     *   - #pre_cb must be NULL,
     *   - vtss_packet_tx_info_t::switch_frm must be FALSE,
     *   - vtss_packet_tx_info_t::dst_port_mask can at most have one bit set, i.e. the frame cannot be multicast,
     *   - vtss_packet_tx_info_t::cos must not be 8 (i.e. super-prio not supported),
     *   - vtss_packet_tx_info_t::tx_vstax_hdr must be VTSS_PACKET_TX_VSTAX_NONE,
     *   - The frame must be held in one single DCB.
     *
     * The memory that contains the actual frame that vtss_fdma_tx() is called
     * with will not be releasable until the AFI frame is cancelled (see vtss_fdma_afi_cancel()).
     *
     *   Luton26: Y
     *   Luton28: N
     *   Jaguar1: Y
     *   Serval : Y (TBD)
     */
    u32 afi_fps;
#endif /* defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC) */

#if defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC)
    /**
     * If the platform supports both FDMA-based and switch-core-based
     * AFI, the application must have a way to select one or the other
     * when requesting injection of an AFI frame.
     *
     * Currently any given platform only supports one or the other, so there's
     * no reason to set it to anything but VTSS_FDMA_AFI_TYPE_AUTO.
     */
    vtss_fdma_afi_type_t afi_type;
#endif /* defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC) */

#if defined(VTSS_FEATURE_AFI_FDMA)
    /**
     * Indicates whether this frame is subject to counting.
     *
     * It is only used when #afi_fps is > 0.
     *
     * The number of frames injected is returned in the call to tx_done_cb()
     * once the frame is cancelled. Interim counters can be obtained through
     * vtss_fdma_list_t::afi_frm_cnt.
     *
     * Enabling counting has some side effects, because the counting must
     * be done in S/W. First of all, to avoid overloading the CPU with
     * interrupts, the FDMA driver makes sure that the frame is repeated a number
     * of times to only get interrupts every, say, 50 ms. This requires an awful lot
     * of DCBs, which will be dynamically allocated by the FDMA driver.
     *
     * Validity:
     *   Luton26: Y
     *   Luton28: N
     *   Jaguar1: Y
     *   Serval : Y (TBD)
     */
    BOOL afi_enable_counting;
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

#if defined(VTSS_FEATURE_AFI_FDMA)
    /**
     * Indicates whether this frame is subject to sequence number updating.
     *
     * It is only used when #afi_fps is > 0.
     *
     * The offset within the frame to update is set with #afi_sequence_number_offset.
     *
     * Enabling sequence numbering has some side effects, because the frame updates must
     * be done in S/W. First of all, to avoid overloading the CPU with
     * interrupts, the frame will be copied a number of times to
     * only get interrupts every, say, 50 ms. This requires dynamic memory allocation
     * within the FDMA driver.
     * Let's take an example: Suppose you wish to send 64-byte frames @ 200 Mbps and
     * have these frames sequence numbered. Such frames will give
     *    200,000,000 / (8 * 64) ~= 400,000 frames per second.
     * In other words, the temporal spacing between these frames is 2.5 microseconds,
     * which - to get to a 50 ms interrupt rate - means that the frames must be copied
     * 50000 / 2.5 = 20000 times. With overhead, one 64-byte frame actually requires
     * 88 bytes, so the total amount of (dynamic) memory required for this operation
     * is 1.76 MBytes (not taking into account the DCBs, which are also dynamically
     * allocated within the driver).
     * In fact, you have to double this number, because the driver is made in such a
     * way that the H/W is not stopped while updating the sequence numbers. Instead,
     * the FDMA driver makes a circular list consisting of two 50 ms half-circles,
     * so that S/W can update one half while the H/W is injecting the other half.
     *
     * SO BE CAREFUL WHEN YOU DESIGN THE APPLICATION!
     *
     * Validity:
     *   Luton26: Y
     *   Luton28: N
     *   Jaguar1: Y
     *   Serval : Y (TBD)
     */
    BOOL afi_enable_sequence_numbering;
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

#if defined(VTSS_FEATURE_AFI_FDMA)
    /**
     * This field indicates the zero-based byte-offset within the frame to increment by one.
     *
     * It is only used when #afi_enable_sequence_numbering is TRUE.
     *
     * The initial value of the sequence number is whatever the frame you inject
     * contains at that offset.
     *
     * The FDMA driver assumes a 4-byte, any-aligned, network-ordered (big endian) value,
     * which wraps to 0 at 0xFFFFFFFF.
     *
     * Needless to say that the full 4-byte value must be contained within the frame's length.
     *
     * Validity:
     *   Luton26: Y
     *   Luton28: N
     *   Jaguar1: Y
     *   Serval : Y (TBD)
     */
    u16 afi_sequence_number_offset;
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

} vtss_fdma_tx_info_t;
#endif /* VTSS_OPT_FDMA_VER >= 3 */


// If this is not defined already by the Switch-core-based AFI, define one for the FDMA-based.
#if !defined(VTSS_AFI_FPS_MAX)
    #define VTSS_AFI_FPS_MAX (1000000)  /**< Maximum number of frames to inject per second using FDMA-based AFI */
#endif /* !defined(VTSS_AFI_FPS_MAX) */

#if VTSS_OPT_FDMA_VER <= 2
/**
 * \brief FDMA Injection Properties.
 *
 * Properties on how to inject a frame using the vtss_fdma_inj() function.
 * All members map or relate to fields in the injected frame's injection header
 * fields.
 *
 * In the following, \@list refers to the list argument passed to the
 * vtss_fdma_inj() function.
 *
 * The application must always memset() this structure
 * to zero before calling vtss_fdma_inj().
 */
typedef struct {
    /**
     * Any user data. Is passed in a call to \@pre_cb().
     */
    void *pre_cb_ctxt1;

    /**
     * Any user data. Is paased in a call to \@pre_cb().
     */
    void *pre_cb_ctxt2;

    /**
     * Callback function called just before the frame is handed over to the FDMA H/W. If NULL, no callback
     * will occur.
     * The called back function may change frame data, nothing else.
     * The called back function *MUST* execute fast and without waiting for synchronization primitives,
     * i.e. no waits are allowed. When called back, interrupts may be disabled.
     * Not used for non-SOF items, since the callback is only called once per frame.\n
     *
     * Implementation notes:
     *   Due to shuffling of beginning-of-frame-fields, this will not work for the following architectures
     *   under the specified circumstances:
     *   Luton26: This will not work for switched frames.
     *   Serval : This will not work for switched frames.
     *   Luton28: This will not work for frames including stack header if VTSS_OPT_FDMA_VER >= 2.
     *
     * Parameters:
     *   - ctxt1:     The value of \@pre_cb_ctxt1.\n
     *   - ctxt2:     The value of \@pre_cb_ctxt2.\n
     *   - list:      Pointer to the SOF item.
     * Validity:
     *   Luton26: Y
     *   Luton28: Y
     *   Jaguar1: Y
     *   Serval : Y
     */
    void (*pre_cb)(void *ctxt1, void *ctxt2, vtss_fdma_list_t *list);

#if defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC)
    /**
     * Number of times this frame will be injected per second.
     *
     * This field must be non-zero if injected on a channel configured for AFI.
     * If injected on a non-AFI channel, this field is ignored.
     *
     * Instead of specifying a specific channel, you may also use a special
     * channel called VTSS_FDMA_CCM_CH_AUTO. Doing so will cause the
     * FDMA driver to attempt to find a suitable channel (pre-configured
     * for AFI) for the injection. See thorough description under
     * 'VTSS_FDMA_CCM_CH_AUTO'.
     *
     * If instead a specific AFI channel is selected, that channel's
     * current frame frequencies must be multiples of this frame's
     * frequency (or vice versa, depending on which number is higher),
     * and there must be room for this new frame, and the frequencies of the
     * currently running frames must not be farther apart than a factor of
     * vtss_fdma_ch_cfg_t::ccm_quotient_max for the call to succeed.
     *
     * Note that there is a limited bandwidth out of the DDR2 SDRAM,
     * through the Shared Bus, and into the FDMA, which also depends on
     * the CPU load. This means that there's an upper limit on the number
     * of frames you can inject per second. Not only does this depend
     * on the frame sizes and their frequencies, but also on H/W and
     * the application's CPU usage (since the CPU shares the Shared Bus
     * with the FDMA). The FDMA driver will not attempt to check that the
     * requested, accumulated bandwidth is greater than this limit, so
     * it's up to the application to determine what makes sense to inject.
     *
     * Valid values when injecting to an AFI channel or using VTSS_FDMA_CCM_CH_AUTO
     * is [1; VTSS_FDMA_CCM_FPS_MAX].
     *
     * There is a number of restrictions on frames injected on a AFI channel:
     *   - #pre_cb must be NULL,
     *   - #switch_frm must be FALSE,
     *   - #port_mask can at most have one bit set, i.e. the frame cannot be multicast,
     *   - #qos_class must not be 8 (i.e. super-prio not supported),
     *   - \@contains_stack_hdr must be FALSE,
     *   - The frame must be held in one single DCB.
     *
     * The vtss_fdma_list_t that vtss_fdma_inj() is called with will not
     * be releasable until the AFI frame is cancelled (see #ccm_cancel) below.
     * Neither will the memory that contains the actual frame. This suggests that AFI frames
     * and associated DCBs should be statically allocated, and for the sake
     * of warm-start, in a place that is not used by the boot loader or
     * overwritten during application initialization.
     *
     *   Luton26: Y
     *   Luton28: N
     *   Jaguar1: Y
     *   Serval : Y (TBD)
     */
    u32 ccm_fps;
#endif /* defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC) */

#if defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC)
    /**
     * This field is used to indicate whether to start a new periodical AFI
     * frame injection or cancel an ongoing injection.
     *
     * Encoding:
     *  FALSE: Start a new periodical injection. The rest of the properties in
     *         this structure must be set according to the description in
     *         #ccm_fps.
     *  TRUE:  Cancel an ongoing periodic injection. All other fields in
     *         this structure are ignored when this is set, but the
     *         channel number and DCB list in the call to vtss_fdma_inj()
     *         must be identical to the original injection (well, you may
     *         set the channel number to VTSS_FDMA_CCM_CH_AUTO even if
     *         you specified an exact channel number in the original call).
     *         Once the frame is completely removed from the list of
     *         frames currently being injected, the list's inj_post_cb()
     *         function will be called - if non-NULL.
     *
     * This field is ignored unless the channel with which vtss_fdma_inj()
     * is called is configured as a AFI channel or is VTSS_FDMA_CCM_CH_AUTO.
     *
     *   Luton26: Y
     *   Luton28: N
     *   Jaguar1: Y
     *   Serval : Y (TBD)
     */
    BOOL ccm_cancel;
#endif /* defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC) */

#if defined(VTSS_FEATURE_AFI_FDMA)
    /**
     * Indicates whether this frame is subject to counting.
     *
     * It is only used when #ccm_fps is > 0.
     *
     * The number of frames injected is returned in the call to inj_post_cb()
     * once the frame is cancelled. Interim counters can be obtained through
     * vtss_fdma_list_t::ccm_frm_cnt.
     *
     * Enabling counting has some side effects, because the counting must
     * be done in S/W. First of all, to avoid overloading the CPU with
     * interrupts, the FDMA driver makes sure that the frame is repeated a number
     * of times to only get interrupts every, say, 50 ms. This requires an awful lot
     * of DCBs, which will be dynamically allocated by the FDMA driver.
     *
     * Validity:
     *   Luton26: Y
     *   Luton28: N
     *   Jaguar1: Y
     *   Serval : Y (TBD)
     */
    BOOL ccm_enable_counting;
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

#if defined(VTSS_FEATURE_AFI_FDMA)
    /**
     * Indicates whether this frame is subject to sequence number updating.
     *
     * It is only used when #ccm_fps is > 0.
     *
     * The offset within the frame to update is set with #ccm_sequence_number_offset.
     *
     * Enabling sequence numbering has some side effects, because the frame updates must
     * be done in S/W. First of all, to avoid overloading the CPU with
     * interrupts, the frame will be copied a number of times to
     * only get interrupts every, say, 50 ms. This requires dynamic memory allocation
     * within the FDMA driver.
     * Let's take an example: Suppose you wish to send 64-byte frames @ 200 Mbps and
     * have these frames sequence numbered. Such frames will give
     *    200,000,000 / (8 * 64) ~= 400,000 frames per second.
     * In other words, the temporal spacing between these frames is 2.5 microseconds,
     * which - to get to a 50 ms interrupt rate - means that the frames must be copied
     * 50000 / 2.5 = 20000 times. With overhead, one 64-byte frame actually requires
     * 88 bytes, so the total amount of (dynamic) memory required for this operation
     * is 1.76 MBytes (not taking into account the DCBs, which are also dynamically
     * allocated within the driver).
     * In fact, you have to double this number, because the driver is made in such a
     * way that the H/W is not stopped while updating the sequence numbers. Instead,
     * the FDMA driver makes a circular list consisting of two 50 ms half-circles,
     * so that S/W can update one half while the H/W is injecting the other half.
     *
     * SO BE CAREFUL WHEN YOU DESIGN THE APPLICATION!
     *
     * Validity:
     *   Luton26: Y
     *   Luton28: N
     *   Jaguar1: Y
     *   Serval : Y (TBD)
     */
    BOOL ccm_enable_sequence_numbering;
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

#if defined(VTSS_FEATURE_AFI_FDMA)
    /**
     * This field indicates the zero-based byte-offset within the frame to increment by one.
     *
     * It is only used when #ccm_enable_sequence_numbering is TRUE.
     *
     * The initial value of the sequence number is whatever the frame you inject
     * contains at that offset.
     *
     * The FDMA driver assumes a 4-byte, any-aligned, network-ordered (big endian) value,
     * which wraps to 0 at 0xFFFFFFFF.
     *
     * Needless to say that the full 4-byte value must be contained within the frame's length.
     *
     * Validity:
     *   Luton26: Y
     *   Luton28: N
     *   Jaguar1: Y
     *   Serval : Y (TBD)
     */
    u16 ccm_sequence_number_offset;
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

    /**
     * As the name implies, if this is TRUE, the FDMA driver
     * code will not modify the IFH before frame injection. The various
     * members of this structure will therefore not be used.
     * This is primarily thought for debugging, or for adding extra
     * features not supported by the API.
     * Normally, you would set this to FALSE.
     */
    BOOL dont_touch_ifh;

    /**
     * If FALSE, the analyzer processing is skipped for this frame,
     * and the destination port set is specified with #port_mask.
     * frame will be passed through the analyzer to find the
     * destination port set. The analyzer looks up the DMAC in the
     * MAC table and forwards based on its findings. The frame may therefore
     * get flooded.
     *
     * If FALSE, the destination port set must be specified with #port_mask.
     * Validity:
     *   Luton26: Y
     *   Luton28: Y
     *   Jaguar1: Y
     *   Serval : Y
     *
     * On Luton28: The value of this member is only used if \@list->phys_port
     * is set to VTSS_CPU_PM_NUMBER.
     *
     * On Luton26 + Serval: If FALSE the rewriter still uses the tag information for rewriting actions.
     */
    BOOL switch_frm;

#if defined(VTSS_ARCH_JAGUAR_1)
    /**
     * On the Jaguar architecture, a given channel can be configured to be capable
     * of injecting into several injection groups.
     * Each group has its own purpose, like switched injection, super-priority
     * injection, etc.
     * Set #inj_grp_auto to TRUE if you want the FDMA driver to automatically
     * choose an injection group to inject into.
     * Set #inj_grp_auto to FALSE if you want to force the FDMA driver to
     * inject into the injection group given by #inj_grp.
     */
    BOOL                 inj_grp_auto;
    /**
     * Used in conjunction with #inj_grp_auto set to FALSE. Determines the injection
     * group to inject the frame to.
     */
    vtss_packet_tx_grp_t inj_grp;
#endif /* defined(VTSS_ARCH_JAGUAR_1) */

    /**
     * This field provides the destination port set onto which to send
     * the frame and may thus be used to multicast the same frame on
     * multiple front ports in one go.
     * The field is only used if #switch_frm is FALSE.
     *
     * The width of the field is VTSS_PHYS_PORT_CNT. The mask thus contains
     * a bit for every front port, where bit 0 in the mask corresponds
     * to physical front port number 0, etc.
     * Some architectures allow for looping a frame back to the CPU on
     * a specific CPU extraction queue, but this is not currently
     * supported.
     * Notice: If #chip_no is 1, this mask can only have one bit set.
     * Validity:
     *   Luton26: Y
     *   Luton28: Y
     *   Jaguar1: Y
     *   Serval : Y
     *
     * On Luton28: The value of this member is only used if \@list->phys_port
     * is set to VTSS_CPU_PM_NUMBER and #switch_frm is FALSE.
     */
    u64  port_mask;

    /**
     * This field provides the destination chip (in a multi-chip solution)
     * that the frame should be injected onto.
     * It is only used in multi-chip solutions like JR-48, and only
     * if #switch_frm is FALSE.
     * If #chip_no == 1, #port_mask must have exactly one bit set.
     *
     * Valid values: [0; 1]
     * Validity:
     *   Luton26: N
     *   Luton28: N
     *   Jaguar1: Y
     *   Serval : N
     */
    vtss_chip_no_t chip_no;

    /**
     * The 12-bit VLAN ID that the frame is injected on.
     *
     * #switch_frm == TRUE:
     *   The frame's broadcast domain is this VID.
     *   It is not possible to specify PCP and DEI.
     *   The frame may or may not be tagged on egress depending on the port's
     *   untagged VID (set). If it gets tagged, the port's setup determines
     *   the VLAN Ethertype.
     *
     * #switch_frm == FALSE:
     *   If #tpid is non-zero, a VLAN tag with TPID = #tpid,
     *   VID = #vlan & 0xFFF, and PCP = #qos_class is inserted in
     *   the frame prior to transmission. CFI will become 0.
     *
     * Validity:
     *   Luton26: Y
     *   Luton28: Y
     *   Jaguar1: Y
     *   Serval : Y
     *
     * On Luton28:
     *   The value of this member is only used if \@list->phys_port
     *   is set to VTSS_CPU_PM_NUMBER.
     *
     * On Luton26, Jaguar, and Serval:
     *   The VLAN is physically inserted into the frame data in
     *   order to be able to inject on this VLAN. This means
     *   that the user's frame data will be modified. This is
     *   not reversible. The additional space needed for this
     *   is reflected in VTSS_FDMA_INJ_HDR_SIZE_BYTES.
     */
    u16 vlan; // 12 LSBits valid.

    /**
     * If not VTSS_ISDX_NONE, it will be used in ES0 lookups instead of the frame's classified VID.
     * Ignored when #switch_frm is TRUE.
     *
     * Validity:
     *   Luton26: N
     *   Luton28: N
     *   Jaguar1: N
     *   Serval : Y
     */
    vtss_isdx_t isdx;

   /**
     * The DP value is inserted into injection header to become drop precedence of the frame.
     * Ignored when #switch_frm is TRUE.
     *
     * Validity:
     *   Luton26: N
     *   Luton28: N
     *   Jaguar1: N
     *   Serval : Y
     */
    vtss_dp_level_t dp;

     /**
     * The PCP value is inserted into injection header to become classified PCP of the frame.
     * Only relevant if 'switch_frm' is FALSE
     *
     * Validity:
     *   Luton26: N
     *   Luton28: N
     *   Jaguar1: N
     *   Serval : Y
     */
    u8 pcp;

   /**
     * The logical port number of the masquerade port.
     * When masquerading, the frame will be handled as if it was received by the
     * ingress port specified in #masquerade_port. Its value will not be used unless
     * #switch_frm is TRUE.
     *
     * When maquerading, the FDMA driver may or may not insert a VLAN tag into the
     * frame prior to transmission. This is determined by #vlan.
     *
     * Set to VTSS_PORT_NO_NONE to disable masquerading.
     *
     * Validity:
     *   Luton26: N
     *   Luton28: N
     *   Jaguar1: N
     *   Serval : Y
     */
    vtss_port_no_t masquerade_port;

   /**
     * VLAN Tag Protocol Identifier.
     *
     * When #tpid != 0 and #switch_frm == FALSE, a VLAN tag
     * will be inserted into the frame prior to transmission.
     * The tag will be composed as TPID = #tpid, VID = #vlan & 0xFFF,
     * and PCP = #qos_class. CFI will become 0.
     *
     * Validity:
     *  Luton26: Y
     *  Luton28: Y (if VTSS_OPT_FDMA_VER >= 2)
     *  Jaguar1: Y
     */
    vtss_etype_t tpid;

    /**
     * The frame's Precision Time Protocol action.
     * See #vtss_packet_ptp_action_t for the enumeration.
     * Ignored when #switch_frm is TRUE.
     *
     * When != VTSS_PACKET_PTP_ACTION_NONE, the #ptp_timestamp and #ptp_id fields must be considered.
     *
     * Validity:
     *   Luton26: Y - except for enumeration VTSS_PACKET_PTP_ACTION_ORIGIN_TIMESTAMP.
     *   Luton28: N
     *   Jaguar1: N
     *   Serval : Y - except for enumeration VTSS_PACKET_PTP_ACTION_ONE_AND_TWO_STEP.
     */
    vtss_packet_ptp_action_t ptp_action;

    /**
     * The PTP identifier used for two-step PTP actions.
     * The CPU can only use from IDs 0 through 3.
     *
     * Used when #ptp_action == VTSS_PACKET_PTP_ACTION_TWO_STEP.
     *
     * Validity:
     *   Luton26: Y
     *   Luton28: N
     *   Jaguar1: N
     *   Serval : Y
     */
    u8 ptp_id;

    /**
     * Holds the PTP timestamp indicating when the injection started.
     * The rewriter can then calculate a residence time based on this and the
     * frame's transmission timestamp.
     *
     * Used when #ptp_action is != VTSS_PACKET_PTP_ACTION_NONE.
     *
     * Validity:
     *   Luton26: Y
     *   Luton28: N
     *   Jaguar1: N
     *   Serval : Y
     */
    u32 ptp_timestamp;

    /**
     * Latch timestamp into a switch core register when the frame
     * is transmitted. This register can then be read-out by S/W
     * to obtain the actual transmission time for that frame.
     * Jaguar note:
     *   It is not possible to request latching of time stamp if
     *   injecting with #qos_class == 8.
     * Encoding:
     *   0: Don't latch timestamp.
     *   1: Latch timestamp into register 0.
     *   2: Latch timestamp into register 1.
     *   3: Latch timestamp into register 2.
     *
     * Validity:
     *   Luton26: N
     *   Luton28: N
     *   Jaguar1: Y
     *   Serval : N
     */
    u32 latch_timestamp;

    /**
     * The aggregation code that this frame will use. The 4-bit number
     * maps directly to the corresponding field in the IFH, but is
     * only set if the frame is being switched (the #switch_frm member
     * is non-zero) and the destination port number is VTSS_CPU_PM_NUMBER.
     * Validity:
     *   Luton26: N
     *   Luton28: Y
     *   Jaguar1: N
     *   Serval : N
     */
    u8 aggr_code;

    /**
     * The QoS class that this frame will be transmitted on.
     * This is a number in range [0; 8]. Note that even if the frame
     * contains a stack header (contains_stack_hdr == TRUE), this field
     * will have to be set to 8 if the frame must be transmitted on
     * super-prio. The SUPER_PRIO field within the stack header determines
     * further transmission (i.e. on the receiving switch, and further in
     * the system).
     *
     * If you set it to '8', the frame will be transmitted with super-priority,
     * i.e. with even higher priority than the highest priority achievable for
     * normal data traffic. This is not a valid setting if #switch_frm == TRUE.
     *
     * Luton26 and Serval notes:
     *   A value of '8' (super priority) will be converted to a value of '7'.
     *
     * Luton28 notes:
     *   Super-priority injection is default on Luton28 when #switch_frm == FALSE
     *   and only directing to one front port. So setting #qos_class to 8 has no
     *   effect other than it will be replaced by the value 7.
     *   Only bits [2:1] will be used, since it only supports four classes.
     *
     * Jaguar notes:
     *   For switched frames (#switch_frm == TRUE), #qos_class goes
     *   into the VLAN tag's PCP in order to have the switch core classify
     *   it to the given QoS class. The reason is that it's not possible
     *   to use IFH.vstax_avail = 1 when transmitting switched in a stacking
     *   configuration. QoS classification based on VLAN.PCP is enabled on
     *   the CPU port.
     *
     * Validity:
     *   Luton26: Y
     *   Luton28: Y
     *   Jaguar1: Y
     *   Serval : Y
     */
    u8 qos_class;

#if defined(VTSS_FEATURE_VSTAX)
    /**
     * If non-zero, the IFH will have the corresponding stack_hdr
     * bit set and filled in. This ensures that stack-enabled front-
     * ports don't insert a stack header. Only frames destined for a
     * stack port can have this bit set.
     *
     * Validity:
     *   Luton26: N
     *   Luton28: Y
     *   Jaguar1: Y
     *   Serval : N
     *
     * On Luton28:
     *   The behavior of this depends on the VTSS_OPT_FDMA_VER
     *   define. In "legacy" mode (VTSS_OPT_FDMA_VER == 1), the stack
     *   header is already filled in by the application and placed
     *   directly in the payload after the SMAC.
     *   In the non-legacy mode (VTSS_OPT_FDMA_VER >= 2), the stack
     *   header comes as sideband information from the stack_hdr_bin
     *   member of this structure and is copied to the relevant
     *   position of the frame. This allows for identical Jaguar
     *   and Luton28 application code-base.
     */
    BOOL contains_stack_hdr;

#if VTSS_OPT_FDMA_VER == 2
    /**
     * This is the VStaX2 header to insert when the frame gets
     * transmitted on a stack port.
     * It is not used unless #contains_stack_hdr is TRUE.
     *
     * Validity:
     *   Luton26: N
     *   Luton28: Y (VTSS_OPT_FDMA_VER == 2)
     *   Jaguar1: Y
     *   Serval : N
     */
    u8 stack_hdr_bin[VTSS_VSTAX_HDR_SIZE];
#endif /* VTSS_OPT_FDMA_VER == 2 */
#endif /* VTSS_FEATURE_VSTAX */

    /**
     * OAM type.
     *
     * Only used if #ptp_action is zero.
     * See vtss_packet_oam_type_t for a description.
     *
     * Validity:
     *   Luton26: N
     *   Luton28: N
     *   Jaguar1: N
     *   Serval : Y
     */
    vtss_packet_oam_type_t oam_type;
} vtss_fdma_inj_props_t;
#endif /* VTSS_OPT_FDMA_VER <= 2 */

/**
 * \brief Channel usage.
 *
 * A given FDMA channel can either be used for extraction or injection.
 */
typedef enum {
    VTSS_FDMA_CH_USAGE_UNUSED, /**< The channel is not currently in use.                               */
    VTSS_FDMA_CH_USAGE_XTR,    /**< The channel is used/supposed to be used for frame extraction.      */
    VTSS_FDMA_CH_USAGE_INJ,    /**< The channel is used/supposed to be used for frame injection.       */
#if defined(VTSS_FEATURE_AFI_FDMA)
    VTSS_FDMA_CH_USAGE_CCM,    /**< The channel is used/supposed to be used for period frame injection */
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */
} vtss_fdma_ch_usage_t;

#if (defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC)) && VTSS_OPT_FDMA_VER <= 2
    /**
     * Use this channel number in calls to vtss_fdma_inj() to
     * ask the FDMA driver to find a suitable channel pre-configured
     * for periodic transmission to inject the frame through.
     *
     * The driver will first search through the channel list for
     * channels that currently use the same frame frequency or a multiple
     * of the frame frequency given by vtss_fdma_inj_props_t::ccm_fps.
     * For this channel to be considered, the frame frequencies of the currently
     * running frames must not be more than a factor of vtss_fdma_ch_cfg_t::ccm_quotient_max
     * apart from the one attempted injected.
     *
     * If such a channel is found, that channel will be used. If it then turns
     * out that there's not room for more AFI frames on this channel, the call will fail.
     *
     * If no suitable channel is found, the driver will search for an AFI channel that
     * is not currently in use and use that one.
     *
     * Otherwise the call to vtss_fdma_inj() will fail.
     */
    #define VTSS_FDMA_CCM_CH_AUTO -1
#endif /* (defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC)) && VTSS_OPT_FDMA_VER <= 2 */

/**
 * \brief Channel configuration structure.
 *
 * The following structure defines the parameters needed to configure
 * a DMA channel.
 * The DMA channel can either be used for frame extraction, frame injection,
 * or period frame injection.
 * This is controlled by the #usage parameter.
 *
 * The interpretation and validity of the remaining fields depend on the
 * #usage parameter.
 *
 * Note: For future compatibility, all applications must memset() this structure to
 * 0 before filling it in.
 */
typedef struct {
    /**
     * <B>XTR/INJ/AFI:</B>\n
     * Indicates whether this channel is used for extraction, injection, or
     * periodic injection.
     * Luton26 and Jaguar note: At most one channel may be configured for AFI use.
     * To disable a channel, set this member to VTSS_FDMA_CH_USAGE_UNUSED.
     */
    vtss_fdma_ch_usage_t usage;

#if defined(VTSS_ARCH_LUTON28)
    /**
     * <B>XTR:</B>\n
     * The extraction queue to associate this channel with. Valid values
     * are [VTSS_PACKET_RX_QUEUE_START; VTSS_PACKET_RX_QUEUE_END[.
     *
     * <B>INJ/AFI:</B>\n
     * Unused.
     */
     vtss_packet_rx_queue_t qu;

#if VTSS_OPT_FDMA_VER == 1
    // Gaps no longer supported unless running legacy code.
    /**
     * <B>XTR:</B>\n
     * Asks the FDMA silicon to insert a gap between the IFH and the first
     * byte of the actual frame (the DMAC). The unit is 32-bit words, and
     * the valid range is [0; 64[. The feature is useful if the module
     * calling the FDMA driver code allows for re-use of software DCBs,
     * since it can then insert a start-gap of four bytes on all extracted
     * frames, and if a frame is going to be re-injected, possibly with
     * small modifications, then there's room for the CMD field, which is
     * required between IFH and actual frame on injection.
     *
     * <B>INJ/AFI:</B>\n
     * Unused.
     */
    int start_gap;

    /**
     * <B>XTR:</B>\n
     * Asks the FDMA silicon to insert a gap after the last byte of the
     * frame (FCS). This is rarely needed, but allows for pre-reserving
     * space for additional tails if the frame is going to be re-injected.
     * The unit is 32-bit words, and the valid range is [0; 64[.
     *
     * <B>INJ/AFI:</B>\n
     * Unused.
     */
    int end_gap;
#endif /* VTSS_OPT_FDMA_VER == 1 */
#endif /* VTSS_ARCH_LUTON28 */

#if defined(VTSS_FEATURE_PACKET_GROUPING)
    /**
     * <B>XTR:</B>\n
     * The extraction group that this channel serves.
     * Need only be set if cfg->chip_no == 1.
     * Valid values are [0; VTSS_PACKET_RX_GRP_CNT[
     *
     * <B>INJ/AFI:</B>\n
     * Unused.
     *
     * Validity:
     *   Luton28: N/A
     *   Jaguar : Y - for 48-ported, only.
     *   Luton26: N
     *   Serval : N
     */
    vtss_packet_rx_grp_t xtr_grp;

    /**
     * <B>XTR:</B>\n
     * Unused.
     *
     * <B>INJ:</B>\n
     * A mask containing the injection groups that this channel serves.
     * A channel may serve more than one injection group. On some architectures
     * a given injection group serves a given type of service (e.g. super-
     * priority injection, switched injection, front-port directed injection,
     * etc.). On such architectures, the actual injection group to use is
     * conveyed in the call to vtss_fdma_inj().
     *
     * At least one bit must be set in the mask. The most significant bit that
     * can be set is VTSS_PACKET_TX_GRP_CNT - 1.
     *
     * <B>AFI:</B>\n
     * A mask containing the injection group that the AFI channel serves.
     * This is a one-hot mask, that is, exactly one bit must be set, and
     * the most significant bit that can be set is VTSS_PACKET_TX_GRP_CNT - 1.
     *
     * Validity:
     *   Luton28: N/A
     *   Jaguar : Y
     *   Luton26: Y, one-hot (exactly one bit set)
     *   Serval : Not used. Implicitly set through the channel number.
     */
    u32 inj_grp_mask;
#endif /* VTSS_FEATURE_PACKET_GROUPING */

    /**
     * <B>XTR:</B>\n
     * A linked, NULL-terminated list of software DCBs, into which the FDMA
     * extract frames. One frame may take one or more DCBs, depending on the
     * size of the associated data area. The following fields of each list
     * item must be pre-initialized by the Packet Module (see definition of
     * vtss_fdma_list_t structure for details):
     * \@data, \@alloc_len, (user), and \@next.
     *
     * <B>INJ:</B>\n
     * Unused.
     *
     * <B>AFI:</B>\n
     * Unused. Must be NULL. The FDMA driver allocates DCBs on its own.
     * This approach is chosen because it's very hard to pre-determine
     * a good number of DCBs that must be allocated, since it depends on
     * whether the application uses frame counting or sequence numbering
     * or not, and whether it may add or cancel frames so fast that both
     * a running, a pending, and a new list of frames must be built up.
     */
    vtss_fdma_list_t *list;

    /**
     * <B>XTR:</B>\n
     * The address of the callback function invoked by the FDMA driver code
     * when a frame arrives. The callback function is invoked once per frame.
     *   - Call context:\n
     *     DSR (the same as what invokes vtss_fdma_irq_handler().
     *   - The parameters to the callback function are as follows:\n
     *      -# cntxt: The value passed in the Packet Module's call to vtss_fdma_irq_handler().
     *      -# list:  NULL-terminated list of software DCBs making up the frame. Depending
     *                on the frame size one or more DCBs are used. The last is always
     *                NULL-terminated. The FDMA has filled in the following fields (see
     *                definition of vtss_fdma_list_t structure for deatils):
     *                (\@dcb), \@*data, \@act_len, (\@next).
     *                Once delivered to the Packet Module, the FDMA will not use the
     *                list again. The FDMA can, however, be handed back a (new) list of
     *                software DCBs by returning the list when the xtr_cb() returns.
     *                Alternatively, use the vtss_fdma_xtr_add_dcbs() function.
     *      -# qu:    The extraction queue that this frame was extracted from. This
     *                may be useful if all extraction callback functions map to the
     *                same function.
     *      - Expected return value from xtr_cb():\n
     *        Non-NULL if the Packet Module wishes to give the FDMA new DCBs to extract to.
     *        In the case where the Packet Module handles the frame directly, it could as well
     *        pass back the list that xtr_cb() was called with right away. In the case where
     *        the frame is passed up to higher levels, like an IP stack, the Packet Module
     *        will probably return NULL in the call to xtr_cb() and provide a replacement
     *        list at a later point in time with a call to vtss_fdma_xtr_add_dcbs().
     *
     * <B>INJ/AFI:</B>\n
     * Unused.
     */
     vtss_fdma_list_t *(*xtr_cb)(void *cntxt, vtss_fdma_list_t *list, vtss_packet_rx_queue_t qu);

    /**
     * <B>XTR/INJ/AFI:</B>\n
     * Controls the channel priority. Valid values are [0; 7].
     * The higher value the higher priority. Everytime the DMA H/W
     * needs to find the next channel to serve, it selects -
     * amongst the pending channels - the channel with the highest
     * priority. Two or more channels may have the same priority,
     * in which case the DMA H/W grants the lowest-numbered channel
     * access.
     * Note: If using the deprecated vtss_fdma_inj_cfg() and
     * vtss_fdma_xtr_cfg() functions, the channel priority will
     * be the same as the channel number.
     */
    int  prio;

    /**
     * <B>XTR</B>\n
     * In a dual-chip solution, this controls the chip to extract from.
     * For single chip solutions, this field must be set to 0.
     * For dual-chip solutions, this field must be set to 0 for extraction
     * from the primary chip, and in this case, the xtr_grp must match
     * the channel number.
     * For dual-chip solutions, this field must be set to 1 for extraction
     * from the secondary chip, and in this case, the xtr_grp need not
     * match the channel number, because - at the time of writing - the
     * channel number isn't really used for anything in that case, because
     * the secondary chip's frames have to be read out manually because they
     * can't be auto-transferred to the primary chip's CPU extraction queues
     * without losing the CPU Rx queue number that it was stored in on the
     * secondary chip.
     *
     * <B>INJ/AFI:</B>\n
     * Unused.
     */
    vtss_chip_no_t chip_no;

#if defined(VTSS_FEATURE_AFI_FDMA)
    /**
     * Maximum quotient allowed on a given AFI channel.
     */
     #define VTSS_FDMA_CCM_QUOTIENT_MAX 3

    /**
     * <B>AFI:</B>\n
     * Controls the channel's maximum frequency quotient.
     * A given AFI channel can handle several frame frequencies provided they
     * are multiples of each other.
     * This configuration option determines the maximum frequency qoutient two frames
     * running on the same channel can have.
     * If for instance, it's set to 2 and the first frame is transmitted with 50 fps,
     * then a subsequent frame can be transmitted with a requested frequency of
     * 25, 50, or 100 fps, only.
     * To allow only one frequency on the channel, set it to 1.
     *
     * Valid values are in the range [1; VTSS_FDMA_CCM_QUOTIENT_MAX].
     *
     * Validity:
     *   Luton26: Y
     *   Luton28: N
     *   Jaguar1: Y
     *   Serval : Y (TBD)
     */
     u32 ccm_quotient_max;

    /**
     * <B>AFI:</B>\n
     * The following defines the maximum number of different frequencies (all multiples of
     * each other) that can run on the same channel.
     *
     * Validity:
     *   Luton26: Y
     *   Luton28: N
     *   Jaguar1: Y
     *   Serval : Y (TBD)
     */
    #define VTSS_FDMA_CCM_FREQ_LIST_LEN 5

    /**
     * <B>AFI:</B>\n
     * The following defines the maximum number of frames per second that can be used
     * in vtss_fdma_inj_props_t's fps member.
     * There is absolutely no guarantee that the actual H/W architecture can reach
     * this number of frames per second.
     *
     * Validity:
     *   Luton26: Y
     *   Luton28: N
     *   Jaguar1: Y
     *   Serval : Y (TBD)
     */
     #define VTSS_FDMA_CCM_FPS_MAX VTSS_AFI_FPS_MAX
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */
} vtss_fdma_ch_cfg_t;

/**
 * \brief FDMA configuration structure.
 *
 * The following structure defines the parameters needed to configure
 * the FDMA in general.
 *
 * Note: For future compatibility, all applications must memset() this structure to
 * 0 before filling it in.
 */
typedef struct {
#if VTSS_OPT_FDMA_VER >= 3
    /**
     * Enable FDMA driver.
     *
     * The FDMA driver can be started and stopped with the use of this member.
     *
     * If the FDMA driver has once been enabled, it can be paused by setting #enable
     * to FALSE.
     */
    BOOL enable;

    /**
     * Rx MTU.
     * The maximum frame length (including FCS) the FDMA will pass on to the application.
     * Typically, an application will set this to 1518 + (4 * max number of VLAN tags).
     *
     * Well, in fact, it indicates the number of frame data bytes associated with
     * one DCB. See also #rx_allow_multiple_dcbs.
     *
     * It can be set to 0 to free all memory allocated by the FDMA driver, but if
     * set to a non-zero value, it must be at or greater than 64.
     */
    u32 rx_mtu;

    /**
     * Rx buffer count.
     * The number of Rx buffers to allocate.
     * Since the FDMA consists of a number of one or more Rx channels,
     * the allocated buffers will be spread evenly across the Rx channels
     * required on a given platform.
     */
    u32 rx_buf_cnt;

    /**
     * The FDMA driver allocates its own software DCBs, to obtain correct
     * alignment of the associated hardware DCBs. It uses VTSS_OS_MALLOC()
     * for this. DCBs are therefore owned by the FDMA driver.
     *
     * However, the associated data area, where frame data gets received into,
     * can either be owned by the FDMA driver or the application.
     *
     * If the application wishes to manage the frame data memory, it must
     * set #rx_alloc_cb() to a non-NULL value.
     * If the application leaves all the frame data memory management to the
     * FDMA driver, it must set #rx_alloc_cb() to NULL.
     *
     * During initialization, the FDMA driver allocates #rx_buf_cnt DCBs
     * and checks whether the application will allocate the corresponding
     * frame data, or it should do that itself. If #rx_alloc_cb() is NULL,
     * the FDMA driver will call VTSS_OS_MALLOC() to allocate the corresponding
     * frame data. Otherwise it will call #rx_alloc_cb() to get it allocated.
     *
     * DCBs are the glue between between the FDMA driver and the application,
     * in the sense that once the FDMA has received a frame, it passes a
     * pointer to the DCB to the application's rx_cb() callback. At this point
     * the application has three options:
     * 1) Pass the DCB further up the food chain to higher parts of the
     *    application, and once it has been handled, feed it back to the FDMA
     *    driver with a call to vtss_fdma_dcb_release().
     *    In this case, the rx_cb() function must return NULL.
     * 2) Detach the frame data from the DCB, set the DCB's alloc_ptr to NULL,
     *    and return the DCB back to the FDMA driver through the return value of
     *    rx_cb().
     * 3) If - for some reason - the application chooses not to pass the frame
     *    further up the food chain in the call to rx_cb(), it may simply return
     *    the DCB as is from rx_cb(), without altering the alloc_ptr member.
     *
     * Whether or not the DCB is returned directly as a return value from rx_cb()
     * or returned through a call to vtss_fdma_dcb_release(), the alloc_ptr
     * member of the DCB indicates whether the frame data has been absorbed by the
     * application of not, as follows:
     * A) If alloc_ptr == NULL when the DCB comes back, the FDMA assumes that the
     *    frame data has been absorbed by the application. In that case, it
     *    will ask the application to re-allocate the alloc_ptr member by calling
     *    #rx_alloc_cb() (which must therefore be non-NULL).
     * B) If alloc_ptr is non-NULL when the DCB comes back to the FDMA driver,
     *    the FDMA driver will just re-initialize the DCB and not call the
     *    #rx_alloc_cb() function.
     *
     * rx_alloc_cb() takes three arguments ([IN] params are seen from the
     * application's point of view.
     * \param sz   [IN]      Number of bytes to allocate.
     * \param list [INOUT]   A NULL-terminated list of DCBs to get allocated
     *                       frame data for. It may consist of more than one
     *                       DCB if the application has chosen to support
     *                       reception of frames fragmented over multiple DCBs
     *                       (#rx_allow_multiple_dcbs) and during initialization.
     *                       The application must fill in the alloc_ptr and possibly
     *                       also the "user" members of the DCB.
     * \param mem_flags [IN] This is a bitwise OR of the vtss_mem_flags_t,
     *                       enumeration, which tells the application how to
     *                       allocate the memory.
     *
     * If - upon return from the rx_alloc_cb() function, the DCB's alloc_ptr member
     * is still NULL, one of two things will happen:
     *  i) If it happens during initialization (vtss_fdma_cfg()), the function
     *     call will fail, and the FDMA will not be started.
     * ii) If it happens at runtime (rx_cb()/vtss_fdma_dcb_release()), the DCB
     *     will be put in a special list that can only be released if the FDMA
     *     driver gets restarted.
     *
     * The rx_alloc_cb() will be invoked for every DCB it should supply frame
     * data for, so don't use the ->next member.
     */
    void (*rx_alloc_cb)(u32 sz, vtss_fdma_list_t *const list, u32 mem_flags);

    /**
     * Don't strip VLAN tag.
     *
     * On all platforms, but Luton28, the switch hardware does not strip VLAN tags
     * from the frames.
     *
     * The FDMA driver can be configured to strip VLAN tags from frames received
     * on ports where the VLAN tag matches the port's VLAN tag setup. This is useful
     * for, e.g. IP stacks that don't expect VLAN tags inside the frames.
     * The recommendation is to set this to FALSE. It is ignored on Luton28.
     */
    BOOL rx_dont_strip_vlan_tag;

    /**
     * Don't re-insert VLAN tag.
     * On Luton28, the switch hardware may happen to strip tags received on ports
     * that are not aware of that particular tag type (TPID). If so, the FDMA driver
     * may re-insert the tag by setting #rx_dont_reinsert_vlan_tag to FALSE.
     * #rx_dont_reinsert_vlan_tag is ignored on all platforms but Luton28.
     * #rx_dont_reinsert_vlan_tag is ignored if #rx_allow_vlan_tag_mismatch is set to FALSE.
     */
    BOOL rx_dont_reinsert_vlan_tag;

    /**
     * If a frame is received with a VLAN tag TPID that does not match the port's setup,
     * it can be dropped (DCB gets recycled) by the FDMA driver, by setting
     * #rx_allow_vlan_tag_mismatch to FALSE.
     */
    BOOL rx_allow_vlan_tag_mismatch;

    /**
     * Rather than indicating the maximum frame size, #rx_mtu indicates the maximum
     * number of frame data bytes that can go into one DCB's data area.
     * If the application can handle a frame spanning more than one DCB, it can set
     * #rx_allow_multiple_dcbs to TRUE, in which case the rx_cb() callback function
     * may be invoked with a list of DCBs (the SOF DCB's next pointer is non-NULL).
     *
     * In this way, the application may support jumbo frames without having to allocate
     * jumbo frame size bytes to each DCB.
     */
    BOOL rx_allow_multiple_dcbs;

    /**
     * The address of the callback function invoked by the FDMA driver code
     * when a frame arrives. The callback function is invoked once per frame.
     *
     * The function is invoked in the same context as the application invoked
     * vtss_fdma_irq_handler().
     *
     * The parameters to the callback function are as follows:\n
     *   -# cntxt: The cntxt passed to vtss_fdma_irq_handler().
     *   -# list:  NULL-terminated list of software DCBs making up the frame. Depending
     *             on the frame size one or more DCBs are used. The last is always
     *             NULL-terminated. The FDMA has filled in the following fields (see
     *             definition of vtss_fdma_list_t structure for deatils):
     *               (\@dcb), \@*data, \@act_len, (\@next).
     *             Once delivered to the Packet Module, the FDMA will not use the
     *             list again. The FDMA can, however, be handed back a (new) list of
     *             software DCBs by returning the list when the rx_cb() returns.
     *             Alternatively, use the vtss_fdma_dcb_release() function.
     *   - Expected return value from rx_cb():\n
     *             See discussion under #rx_alloc_cb.
     */
    vtss_fdma_list_t *(*rx_cb)(void *cntxt, vtss_fdma_list_t *list);

    /**
     * Tx buffer count.
     * The number of Tx DCBs to allocate.
     */
    u32 tx_buf_cnt;

    /**
     * The address of the callback function invoked by the FDMA driver code
     * when a frame has been transmitted. The callback function is invoked once per frame.
     *
     * The function is invoked in the same context as the application invoked
     * vtss_fdma_irq_handler().
     *
     * The parameters to the callback function are as follows:\n
     *   -# cntxt: The cntxt passed to vtss_fdma_irq_handler().
     *   -# list:  NULL-terminated list of software DCBs making up the frame.
     *             The last is always NULL-terminated.
     *   -# rc:    Return code. VTSS_RC_OK on success, otherwise failed to transmit.
     *
     * The application must extract the required information from the DCB list (e.g. through
     * the DCB's user field) prior to returning from this function, because the FDMA driver
     * will recycle the DCBs upon return.
     *
     * In the event that the DCBs were originally coming from extraction, the FDMA driver
     * returns the DCBs back to extxraction.
     */
    void (*tx_done_cb)(void *cntxt, struct tag_vtss_fdma_list *list, vtss_rc rc);

#if defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC)
    /**
     * AFI buffer count.
     * The number of AFI DCBs to allocate.
     */
    u32 afi_buf_cnt;

    /**
     * The address of the callback function invoked by the FDMA driver code
     * when an AFI frame has been cancelled. The callback function is invoked once per frame.
     *
     * The function is invoked in the same context as the application invoked
     * vtss_fdma_irq_handler().
     *
     * The parameters to the callback function are as follows:\n
     *   -# cntxt: The cntxt passed to vtss_fdma_irq_handler().
     *   -# list:  NULL-terminated list of software DCBs making up the frame.
     *             The last is always NULL-terminated.
     *   -# rc:    Return code. VTSS_RC_OK always.
     *
     * The application must extract the required information from the DCB list (e.g. through
     * the DCB's user field) prior to returning from this function, because the FDMA driver
     * will recycle the DCBs upon return.
     *
     */
    void (*afi_done_cb)(void *cntxt, struct tag_vtss_fdma_list *list, vtss_rc rc);
#endif /* defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC) */
#else /* VTSS_OPT_FDMA_VER */
#if defined(VTSS_FEATURE_VSTAX)
    /**
     * <B>XTR/INJ:</B>\n
     *   One-hot mask, where the bit set indicates the physical
     *   stack port A. More than one bit may be set if the stack
     *   port is part of an aggregation.
     *   OBSOLETE. NOT USED ANYMORE.
     *
     * Validity:
     *   Luton26: N
     *   Luton28: Y
     *   Jaguar1: Y
     *   Serval : N
     */
    u64 stack_port_a_mask;

    /**
     * <B>XTR/INJ:</B>\n
     *   One-hot mask, where the bit set indicates the physical
     *   stack port B. More than one bit may be set if the stack
     *   port is part of an aggregation.
     *   OBSOLETE. NOT USED ANYMORE
     *
     * Validity:
     *   Luton26: N
     *   Luton28: Y
     *   Jaguar1: Y
     *   Serval : N
     */
    u64 stack_port_b_mask;

    /**
     * <B>INJ:</B>\n
     *   In dual-chip solutions we need to know the chip on which
     *   stack port A is located, i.e. which chip stack_port_a_mask
     *   pertains to.
     *   OBSOLETE. NOT USED ANYMORE
     *
     * Validity:
     *   Luton26: N
     *   Luton28: Y (Must be 0)
     *   Jaguar1: Y
     *   Serval : N
     */
    vtss_chip_no_t stack_port_a_chip;

    /**
     * <B>INJ:</B>\n
     *   In dual-chip solutions we need to know the chip on which
     *   stack port B is located, i.e. which chip stack_port_b_mask
     *   pertains to.
     *   OBSOLETE. NOT USED ANYMORE
     *
     * Validity:
     *   Luton26: N
     *   Luton28: Y (Must be 0)
     *   Jaguar1: Y
     *   Serval : N
     */
    vtss_chip_no_t stack_port_b_chip;
#else
    /**
     * <B>XTR/INJ:</B>\n
     *   This is just a dummy, that is not used for anything, but which
     *   must be here to make the structure non-empty when stacking is
     *   not defined.
     *   It can safely be removed once other non-stacking parameters are
     *   added to this structure.
     *
     * Validity:
     *   Luton26: N
     *   Luton28: N
     *   Jaguar1: N
     *   Serval : N
     */
    u32 dummy;
#endif /* defined(VTSS_FEATURE_VSTAX) */
#endif /* VTSS_OPT_FDMA_VER */
} vtss_fdma_cfg_t;

/**
 * In order to survive e.g. broadcast storms, the FDMA driver incorporates
 * a poor man's policing/throttling scheme.
 *
 * The idea is to check upon every frame reception whether the CPU Rx queue
 * on which the frame was received has exceeded its limit, and if so,
 * suspend extraction from the queue for a period of time.
 *
 * The FDMA has no notion of time, so this requires a little help from
 * the application.
 * To take advantage of the feature, the application must first call
 * vtss_fdma_throttle_cfg_set() with an appropriate configuration, and
 * then call vtss_fdma_throttle_tick() on a regular, application-defined basis,
 * e.g. 10 times per second.
 *
 * The throttle tick takes care of re-opening the queue after the suspension
 * period elapses.
 *
 \if VTSS_OPT_FDMA_VER <= 2
 * This feature is controlled per extraction queue, and if
 * vtss_fdma_throttle_cfg_t::limit_per_tick[xtr_qu] is 0 this
 * feature is disabled for that queue.
 *
 * If vtss_fdma_throttle_cfg_t::limit_per_tick[xtr_qu] is non-zero,
 * that queue will be suspended for the remainder of the tick period PLUS
 * whatever is specified with vtss_fdma_throttle_cfg_t::suspend_tick_cnt
 * whenever that number of frames is reached.
 *
 \endif
 \if VTSS_OPT_FDMA_VER >= 3
 * This feature is controlled per extraction queue, and if
 * vtss_fdma_throttle_cfg_t::frm_limit_per_tick[xtr_qu] is 0 and
 * vtss_fdma_throttle_Cfg_t::byte_limit_per_tick[xtr_qu] is 0 this
 * feature is disabled for that queue.
 *
 * If either vtss_fdma_throttle_cfg_t::frm_limit_per_tick[xtr_qu] or
 * vtss_fdma_throttle_cfg_t::byte_limit_per_tick[xtr_qu] is non-zero,
 * the feature is enabled, and in case a queue gets suspended, it will be
 * suspended for the remainder of the tick period PLUS whatever is
 * specified with vtss_fdma_throttle_cfg_t::suspend_tick_cnt.
 \endif
 *
 * Notice that once an extraction queue gets disabled, that extraction queue
 * will no longer be a source of interrupts. The feature will only affect
 * extraction queues for which it is enabled.
 *
 * Once disabling an extraction queue, the remaining frames in the queue
 * will be read out, after which the queue will be silent. When re-enabling, it
 * will be fresh frames that come in.
 *
 * Be aware that on Lu26, the trick to disable a queue involves directing the
 * frames to the second CPU port (physical #27). If your application uses two CPU ports,
 * then throttling will have unexpected side-effects.
 *
 * There is no need to call vtss_fdma_throttle_tick() unless throttling is
 * enabled for at least one extraction queue.
 *
 * If throttling is enabled for at least one queue, and vtss_fdma_throttle_tick()
 * is *not* called, you risk that an extraction queue will get disabled and
 * never re-enabled again.
 *
 * Validity:
 *   Luton26: Y
 *   Luton28: N
 *   Jaguar1: Y
 *   Serval1: Y
 */
typedef struct {
#if VTSS_OPT_FDMA_VER <= 2
    /**
     * Controls - per extraction queue - the maximum number of frames
     * extracted between two calls to vtss_fdma_throttle_tick() without
     * suspending extraction from that queue.
     *
     * If 0, throttling is disabled for that extraction queue.
     */
    u32 limit_per_tick[VTSS_PACKET_RX_QUEUE_CNT];
#endif /* VTSS_OPT_FDMA_VER <= 2 */

#if VTSS_OPT_FDMA_VER >= 3
    /**
     * Controls - per extraction queue - the maximum number of frames
     * extracted between two calls to vtss_fdma_throttle_tick() without
     * suspending extraction from that queue.
     *
     * If 0, frame count throttling is disabled for that extraction queue.
     */
    u32 frm_limit_per_tick[VTSS_PACKET_RX_QUEUE_CNT];
#endif /* VTSS_OPT_FDMA_VER >= 3 */

#if VTSS_OPT_FDMA_VER >= 3
    /**
     * Controls - per extraction queue - the maximum number of bytes
     * extracted between two calls to vtss_fdma_throttle_tick() without
     * suspending extraction from that queue.
     *
     * If 0, byte count throttling is disabled for that extraction queue.
     */
    u32 byte_limit_per_tick[VTSS_PACKET_RX_QUEUE_CNT];
#endif /* VTSS_OPT_FDMA_VER >= 3 */

    /**
     * Controls - per extraction queue - the number of invocations of
     * vtss_fdma_throttle_tick() that must happen before an extraction queue
     * that has been disabled, gets re-enabled.
     *
     * For instance,
     *   a value of 0 means: re-enable the extraction queue on the next tick.
     *   a value of 1 means: re-enable the extraction queue two ticks from when it was suspended.
     */
    u32 suspend_tick_cnt[VTSS_PACKET_RX_QUEUE_CNT];
} vtss_fdma_throttle_cfg_t;

#if VTSS_OPT_FDMA_VER <= 2
/**
 * \brief Initialize FDMA.
 *
 * This function must be called before any other FDMA API function.
 * It performs a silicon FDMA version check, disables the FDMA, and
 * initializes the internal FDMA state.
 *
 * THIS FUNCTION IS OBSOLETE AND WILL DO NOTHING USEFUL.
 *
 * - Call context:\n
 *   Thread
 *
 * - Re-entrant:\n
 *   No
 *
 * \param inst [IN]: Target instance reference.
 *
 * \return
 *   VTSS_RC_OK on success.\n
 *   VTSS_RC_ERROR if a supplied parameter was erroneous.
 */
vtss_rc vtss_fdma_init(const vtss_inst_t inst);
#endif /* VTSS_OPT_FDMA_VER <= 2 */

/**
 * \brief Uninitialize FDMA.
 *
 * Rarely used. Use only if you want to make a controlled shut-down of the
 * FDMA. Disables all FDMA channels and interrupts, and clears pending
 * interrupts.
 *
 * - Call context:\n
 *   Thread
 *
 * - Re-entrant:\n
 *   No
 *
 * \param inst [IN]: Target instance reference.
 *
 * \return
 *   VTSS_RC_OK on success.\n
 *   VTSS_RC_ERROR if a supplied parameter was erroneous.
 */
vtss_rc vtss_fdma_uninit(const vtss_inst_t inst);

/**
 * \brief Configure FDMA.
 *
 * Call this function before any other FDMA API function.
 *
 * - Call context:\n
 *   Thread
 *
 * - Re-entrant:\n
 *   No
 *
 * \param inst [IN]: Target instance reference.
 * \param cfg  [IN]: FDMA configuration.
 *
 * \return
 *   VTSS_RC_OK on success.\n
 *   VTSS_RC_ERROR if a supplied parameter was erroneous.
 */
vtss_rc vtss_fdma_cfg(const vtss_inst_t inst, const vtss_fdma_cfg_t *const cfg);

#if defined(VTSS_ARCH_LUTON28) && VTSS_OPT_FDMA_VER <= 2
/**
 * \brief Configure an FDMA channel for extraction.
 *
 * Enables or disables an FDMA channel for extraction.
 * This function is usually called during initialization only, right after
 * vtss_fdma_init(). It connects a CPU Port Module extraction queue to a
 * DMA channel, and hands over an initialized list of software DCBs to the
 * FDMA silicon to fill in with frames received on the extraction queue.
 * Notice: It is not the task of the FDMA driver code to match particular
 * frame types to particular extraction queues.
 * In addition to the list of software DCBs, the address of a a callback
 * function is supplied in the call. The FDMA driver code calls this callback
 * function whenever one frame has been extracted.
 *
 * - Call context:\n
 *   Thread
 *
 * - Re-entrant:\n
 *   No
 *
 * \param inst [IN]:      Target instance reference.
 * \param ena [IN]:       FALSE to disable the channel given by \@ch, TRUE to enable (FALSE is
 *                        rarely used). The remainder of the parameters are described as if
 *                        the channel is being enabled.
 * \param ch [IN]:        The channel to disable/enable. Valid values are [0; VTSS_FDMA_CH_CNT[.
 * \param qu [IN]:        The extraction queue to associate this channel with. Valid values
 *                        are [VTSS_PACKET_RX_QUEUE_START; VTSS_PACKET_RX_QUEUE_END[.
 \if VTSS_OPT_FDMA_VER == 1
 * \param start_gap [IN]: Asks the FDMA silicon to insert a gap between the IFH and the first
 *                        byte of the actual frame (the DMAC). The unit is 32-bit words, and
 *                        the valid range is [0; 64[. The feature is useful if the module
 *                        calling the FDMA driver code allows for re-use of software DCBs,
 *                        since it can then insert a start-gap of four bytes on all extracted
 *                        frames, and if a frame is going to be re-injected, possibly with
 *                        small modifications, then there's room for the CMD field, which is
 *                        required between IFH and actual frame on injection.
 * \param end_gap [IN]:   Asks the FDMA silicon to insert a gap after the last byte of the
 *                        frame (FCS). This is rarely needed, but allows for pre-reserving
 *                        space for additional tails if the frame is going to be re-injected.
 *                        The unit is 32-bit words, and the valid range is [0; 64[.
 \endif
 * \param list [IN]:      A linked, NULL-terminated list of software DCBs, into which the FDMA
 *                        extract frames. One frame may take one or more DCBs, depending on the
 *                        size of the associated data area. The following fields of each list
 *                        item must be pre-initialized by the Packet Module (see definition of
 *                        vtss_fdma_list_t structure for details):
 *                        \@data, \@alloc_len, (user), and \@next.
 * \param xtr_cb [IN]:    The address of the callback function invoked by the FDMA driver code
 *                        when a frame arrives. The callback function is invoked once per frame.
 *             - Call context:\n
 *                DSR (the same as what invokes vtss_fdma_irq_handler().
 *             - The parameters to the callback function are as follows:\n
 *                -# cntxt: The value passed in the Packet Module's call to vtss_fdma_irq_handler().
 *                -# list:  NULL-terminated list of software DCBs making up the frame. Depending
 *                          on the frame size one or more DCBs are used. The last is always
 *                          NULL-terminated. The FDMA has filled in the following fields (see
 *                          definition of vtss_fdma_list_t structure for deatils):
 *                            (\@dcb), \@*data, \@act_len, (\@next).
 *                          Once delivered to the Packet Module, the FDMA will not use the
 *                          list again. The FDMA can, however, be handed back a (new) list of
 *                          software DCBs by returning the list when the xtr_cb() returns.
 *                          Alternatively, use the vtss_fdma_xtr_add_dcbs() function.
 *                -# qu:    The extraction queue that this frame was extracted from. This
 *                          may be useful if all extraction callback functions map to the
 *                          same function.
 *             - Expected return value from xtr_cb():\n
 *                Non-NULL if the Packet Module wishes to give the FDMA new DCBs to extract to.
 *                In the case where the Packet Module handles the frame directly, it could as well
 *                pass back the list that xtr_cb() was called with right away. In the case where
 *                the frame is passed up to higher levels, like an IP stack, the Packet Module
 *                will probably return NULL in the call to xtr_cb() and provide a replacement
 *                list at a later point in time with a call to vtss_fdma_xtr_add_dcbs().
 *
 * \return
 *   VTSS_RC_OK on success.\n
 *   VTSS_RC_ERROR if a supplied parameter was erroneous.\n
 *   VTSS_RC_INV_STATE if\n
 *     - another channel is using the same extraction queue, or
 *     - channel is not currently disabled.
 *
 * \em DEPRECATED. Use vtss_fdma_ch_cfg() instead.
 */
vtss_rc vtss_fdma_xtr_cfg(const vtss_inst_t            inst,
                          const BOOL                   ena,
                          const vtss_fdma_ch_t         ch,
                          const vtss_packet_rx_queue_t qu,
#if VTSS_OPT_FDMA_VER == 1
                          const int                    start_gap,
                          const int                    end_gap,
#endif /* VTSS_OPT_FDMA_VER == 1 */
                                vtss_fdma_list_t       *const list,
                                vtss_fdma_list_t       *(*xtr_cb)(void *cntxt, vtss_fdma_list_t *list, vtss_packet_rx_queue_t qu));
#endif /* defined(VTSS_ARCH_LUTON28) && VTSS_OPT_FDMA_VER <= 2 */

#if defined(VTSS_ARCH_LUTON28) && VTSS_OPT_FDMA_VER <= 2
/**
 * \brief Configure an FDMA channel for injection.
 *
 * Reserves or frees an FDMA channel for injection. Actual injection
 * is performed on a per-frame basis with subsequent calls to
 * vtss_fdma_inj().
 *
 * - Call context:\n
 *   Thread
 *
 * - Re-entrant:\n
 *   No!!
 *
 * \param inst [IN]: Target instance reference.
 * \param ena [IN]: Set to 0 to disable the channel (rarely used), and 1 to enable it.
 * \param ch [IN]:  The channel to disable/enable. Valid values are [0; VTSS_FDMA_CH_CNT[.
 *
 * \return
 *   VTSS_RC_OK on success.\n
 *   VTSS_RC_ERROR if a supplied parameter was erroneous.\n
 *   VTSS_RC_INV_STATE if channel is not currently disabled.
 *
 * \em DEPRECATED. Use vtss_fdma_ch_cfg() instead.
 */
vtss_rc vtss_fdma_inj_cfg(const vtss_inst_t    inst,
                          const BOOL           ena,
                          const vtss_fdma_ch_t ch);
#endif /* defined(VTSS_ARCH_LUTON28) && VTSS_OPT_FDMA_VER <= 2 */

#if VTSS_OPT_FDMA_VER <= 2
/**
 * \brief Configure an FDMA channel for extraction or injection.
 *
 * This function replaces the deprecated vtss_fdma_xtr_cfg()
 * and vtss_fdma_inj_cfg() functions.
 *
 * Reserves or frees an FDMA channel for injection or extraction.
 *
 * This function is usually called during initialization only, right after
 * vtss_fdma_init(). For details, please refer to the vtss_fdma_ch_cfg_t
 * structure.
 *
 * - Call context:\n
 *   Thread
 *
 * - Re-entrant:\n
 *   No
 *
 * \param inst [IN]:      Target instance reference.
 * \param ch   [IN]:      The channel to configure. Valid values are [0; VTSS_FDMA_CH_CNT[.
 * \param cfg  [IN]:      Structure holding the configuration of the channel. Refer
 *                        to vtss_fdma_ch_cfg_t for a description of the individual fields.
 *
 * \return
 *   VTSS_RC_OK on success.\n
 *   VTSS_RC_ERROR if a supplied parameter was erroneous.\n
 *   VTSS_RC_INV_STATE if\n
 *     - another channel is using the same channel, or
 *     - channel is not currently disabled.
 */
vtss_rc vtss_fdma_ch_cfg(const vtss_inst_t        inst,
                         const vtss_fdma_ch_t     ch,
                         const vtss_fdma_ch_cfg_t *const cfg);
#endif /* VTSS_OPT_FDMA_VER <= 2 */

#if VTSS_OPT_FDMA_VER <= 2
/**
 * \brief Add list of DCBs to an extraction FDMA channel.
 *
 * Depending on the application, it may be desirable to send software DCBs
 * back to the FDMA driver code either right away after the call to the
 * xtr_cb() callback function, or later when higher levels of software
 * has processed the frame, or - to avoid a DoS attack - to hold back the
 * list until the attack is over. In the two latter cases, the DCBs may be given
 * (back) to a specific FDMA channel by calling this function.
 *
 * - Call context:\n
 *   Thread
 *
 * - Re-entrant:\n
 *   Yes
 *
 * \param inst   [IN]: Target instance reference.
 * \param xtr_ch [IN]: The channel to store these free DCBs onto.
 * \param list   [IN]: The list of DCBs to store onto the channel's free list of DCBs.
 *
 * \return
 *   VTSS_RC_OK on success.\n
 *   VTSS_RC_ERROR if a supplied parameter was erroneous.\n
 *   VTSS_RC_INV_STATE if\n
 *     - channel is not configured for extraction, or
 *     - channel is not enabled.
 */
vtss_rc vtss_fdma_xtr_add_dcbs(const vtss_inst_t      inst,
                               const vtss_fdma_ch_t   xtr_ch,
                                     vtss_fdma_list_t *const list);
#endif /* VTSS_OPT_FDMA_VER <= 2 */

#if VTSS_OPT_FDMA_VER >= 3
/**
 * \brief Release DCBs.
 *
 * This function returns DCBs (injection, extraction, or AFI) back to the
 * FDMA driver.
 *
 * This is useful in these situations:
 *   Extraction:
 *      If frame data memory is completely managed by the FDMA
 *      driver (rx_alloc_cb() == NULL), then the application may
 *      choose to pass the frame as is to higher levels of software,
 *      i.e. with zero-copy. Once it is handled, the DCBs must be
 *      returned to the FDMA driver with a call to this function.
 *      If frame data memory is managed by the application (
 *      rx_alloc_cb() != NULL), then the application should return
 *      the list of DCBs it was invoked with in the rx_cb() callback
 *      handler (with the alloc_ptr set to NULL if the frame itself
 *      is passed to higher levels of software).
 *
 *   Injection (both normal and AFI):
 *      This is rarely useful, and the only situation where it makes
 *      sense to return injection DCBs to the FDMA driver is when
 *      it has allocated the DCBs (using vtss_fdma_dcb_get()) and
 *      then decides not to use them in a call to vtss_fdma_inj()
 *      afterwards.
 *
 * - Call context:\n
 *   Thread
 *
 * - Re-entrant:\n
 *   Yes
 *
 * \param inst [IN]: Target instance reference.
 * \param list [IN]: The list of DCBs to be returned to the application.
 *
 * \return
 *   VTSS_RC_OK on success.\n
 *   VTSS_RC_ERROR if a supplied parameter was erroneous.
 */
vtss_rc vtss_fdma_dcb_release(const vtss_inst_t      inst,
                                    vtss_fdma_list_t *const list);
#endif /* VTSS_OPT_FDMA_VER >= 3 */

#if VTSS_OPT_FDMA_VER <= 2
/**
 * \brief Inject a frame.
 *
 * This function takes a NULL-terminated list of software
 * DCBs and injects it using the channel number also passed as argument.
 * After the injection has taken place, the inj_post_cb() callback function,
 * set in the SOF DCB's inj_post_cb member, is called back with a success
 * indicator and the actual list of DCBs.
 *
 * - Call context:\n
 *   Thread
 *
 * - Re-entrant:\n
 *   Yes
 *
 * \param inst [IN]:  Target instance reference.
 * \param list [IN]:  The list of software DCBs making up the frame. The destination
 *                    front-port is given by the DCBs' phys_port member.
 *                    This is the physical port number and thus ranges from
 *                    [0; VTSS_FDMA_TOTAL_PORT_CNT[, where VTSS_CPU_PM_NUMBER is used if the
 *                    frame uses the CPU Port Module, and any other number if the frame
 *                    should go to a front port. Any DMA channel can inject to any port.
 * \param ch [IN]:    The channel to use for the injection.
 * \param len [IN]:   The length of the whole frame in bytes, excluding IFH and CMD,
 *                    but including the FCS.
 * \param props [IN]: Pointer to a structure that contains properties on how to inject
 *                    the frame. All the structure's members are used to set the
 *                    IFH and CMD fields appropriately. For future extensions, it is
 *                    recommended that you memset() the structure to all-zeros before
 *                    assigning it. The structure may be allocated on the stack,
 *                    since vtss_fdma_inj() doesn't keep a pointer to it after the
 *                    call returns.
 *
 * \return
 *   VTSS_RC_OK on success.\n
 *   VTSS_RC_ERROR if a supplied parameter was erroneous.\n
 *   VTSS_RC_INV_STATE if\n
 *     - channel is not configured for injection,
 *     - channel is not enabled, or
 *     - attempting to inject stack-headered frame to a non-stack-port.
 */
vtss_rc vtss_fdma_inj(const vtss_inst_t           inst,
                            vtss_fdma_list_t      *list,
                      const vtss_fdma_ch_t        ch,
                      const u32                   len,
                      const vtss_fdma_inj_props_t *const props);
#else
/**
 * \brief Inject a frame.
 *
 * This function takes a NULL-terminated list of software DCBs making up one
 * frame and injects it using the tx info properties given by props.
 *
 * The DCBs must be obtained using the vtss_fdma_dcb_get() call.
 *
 * Once the frame is injected, the configured tx_done_cb() function will be
 * called. Once this function returns, the DCBs that the callback function
 * was invoked with are no longer available to the application.
 *
 * Upon invokation of tx_done_cb(), the application can read additional data
 * from the frame's SOF DCB. The fields filled in by the FDMA driver are
 *   sw_tstamp, afi_frm_cnt, and afi_seq_number.
 *
 * Upon tx_done_cb() the FDMA driver may have modified the actual frame data, and
 * thereby the frm_ptr that was set during the call to vtss_fdma_inj().
 *
 * - Call context:\n
 *   Thread
 *
 * - Re-entrant:\n
 *   Yes
 *
 * \param inst [IN]:      Target instance reference.
 * \param list [IN]:      The list of software DCBs making up the frame. Only the frm_ptr
 *                        and act_len members must be filled in. The user member is useful for
 *                        additional data required by the application to identify the frame that
 *                        is being injected upon the tx_done_cb() callback.
 * \param fdma_info [IN]: Info specifically for use by the FDMA driver. The structure may
 *                        be allocated on the stack, since vtss_fdma_inj() doesn't keep
 *                        a pointer to it after the call returns. Initialize with a call to
 *                        vtss_fdma_tx_info_init() prior to filling it in.
 * \param tx_info   [IN]: Pointer to a structure that contains properties on how to inject
 *                        the frame. Must be initialized with a call to vtss_packet_tx_info_init()
 *                        before assigning it. The structure may be allocated on the stack, since vtss_fdma_inj()
 *                        doesn't keep a pointer to it after the call returns.
 *
 * \return
 *   VTSS_RC_OK on success.\n
 *   VTSS_RC_ERROR on any kind of error. In this case, the DCBs are already returned
 *   to the FDMA driver.
 */
vtss_rc vtss_fdma_tx(const vtss_inst_t           inst,
                           vtss_fdma_list_t      *list,
                           vtss_fdma_tx_info_t   *const fdma_info,
                           vtss_packet_tx_info_t *const tx_info);
#endif /* VTSS_OPT_FDMA_VER */

#if VTSS_OPT_FDMA_VER >= 3
/** \brief Initialize a vtss_fdma_tx_info_t structure
 *
 * \param inst      [IN]: Target instance
 * \param fdma_info [IN]: Structure to initialize
 *
 * \return VTSS_RC_OK unless fdma_info == NULL.
 */
vtss_rc vtss_fdma_tx_info_init(const vtss_inst_t inst,
                                     vtss_fdma_tx_info_t *const fdma_info);
#endif /* VTSS_OPT_FDMA_VER >= 3 */

#if (defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC)) && VTSS_OPT_FDMA_VER >= 3
/** \brief Cancel an ongoing AFI transmission.
 *
 * The frame is identified by a pointer to the frame
 * previously used in list->frm_ptr in the call to
 * vtss_fdma_tx().
 *
 * Once the cancellation has occurred, the
 * afi_tx_done callback will be called.
 *
 * \param inst    [IN]: Target instance
 * \param frm_ptr [IN]: Frame pointer.
 *
 * \return VTSS_RC_ERROR if no such frame was found. VTSS_RC_OK
 * if cancelled successfully.
 */
vtss_rc vtss_fdma_afi_cancel(const vtss_inst_t inst, const u8 *const frm_ptr);
#endif /* (defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC)) && VTSS_OPT_FDMA_VER >= 3 */

#if defined(VTSS_FEATURE_AFI_FDMA) && VTSS_OPT_FDMA_VER >= 3
/** \brief Get an interim frame count for a given periodically injected frame.
 *
 * The frame is identified by a pointer to the frame
 * previously used in list->frm_ptr in the call to
 * vtss_fdma_tx(). To get a count != 0, counting must
 * be enabled with fdma_info.afi_enable_counting.
 *
 * \param inst    [IN]: Target instance
 * \param frm_ptr [IN]: Frame pointer.
 * \param frm_cnt [OUT]: Frame counter.
 *
 * \return VTSS_RC_ERROR if no such frame was found. VTSS_RC_OK
 * if successful.
 */
vtss_rc vtss_fdma_afi_frm_cnt(const vtss_inst_t inst, const u8 *const frm_ptr, u64 *const frm_cnt);
#endif /* defined(VTSS_FEATURE_AFI_FDMA) && VTSS_OPT_FDMA_VER >= 3 */

#if VTSS_OPT_FDMA_VER >= 3
/**
 * \brief DCB type identifying a DCB
 */
typedef enum {
    VTSS_FDMA_DCB_TYPE_XTR, /**< The DCB is an extraction DCB. Not needed by application.            */
    VTSS_FDMA_DCB_TYPE_INJ, /**< The DCB is an injection DCB. Needed in call to vtss_fdma_dcb_get(). */
#if defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC)
    VTSS_FDMA_DCB_TYPE_AFI, /**< The DCB is an AFI DCB. Needed in call to vtss_fdma_dcb_get().       */
#endif /* defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC) */
} vtss_fdma_dcb_type_t;
#endif /* VTSS_OPT_FDMA_VER >= 3 */

#if VTSS_OPT_FDMA_VER >= 3
/**
 * \brief Get one or more DCBs suitable for frame injection.
 *
 * The FDMA API is the owner/maintainer of all DCBs. In order to be able to inject
 * a frame, the application must request DCBs from the FDMA driver and fill in
 * appropriate fields (see description under vtss_fdma_inj()) prior to calling
 * vtss_fdma_inj().
 *
 * One or more DCBs may be requested in one go.
 * If the application regrets that it has requested the DCBs, it may call
 * vtss_fdma_dcbs_release() to hand them back to the FDMA driver.
 *
 * If more than one DCB is requested, the FDMA driver will hook them together
 * with the DCBs' next field. The last DCB's next field will be set to NULL
 * by the FDMA driver.
 *
 * Special DCBs are required for AFI frames, so in addition to the number of
 * DCBs, also a dcb_type parameter must be provided. The dcb_type must only be
 * set to VTSS_FDMA_DCB_TYPE_INJ or VTSS_FDMA_DCB_TYPE_AFI.
 *
 * If the requested number of DCBs are not available of the specified type,
 * the function returns VTSS_RC_INCOMPLETE. It is up to the application to
 * implement the logic that can facilitate waiting for DCBs, if it is required
 * that a given frame must be transmitted. It could, e.g. wait for a semaphore
 * in the thread that calls vtss_fdma_inj() and signal that semaphore whenever
 * the tx_done_cb() gets invoked.
 *
 * - Call context:\n
 *   Thread
 *
 * - Re-entrant:\n
 *   Yes
 *
 * \param inst     [IN]:  Target instance reference.
 * \param dcb_cnt  [IN]:  Number of DCBs.
 * \param dcb_type [IN]:  DCB type requested.
 * \param list     [OUT]: Pointer receiving a pointer to the first DCB.
 *
 * \return
 *   VTSS_RC_OK on success.\n
 *   VTSS_RC_ERROR if a supplied parameter was erroneous.\n
 *   VTSS_RC_INCOMPLETE if dcb_cnt DCBs aren't available.
 */
vtss_rc vtss_fdma_dcb_get(const vtss_inst_t          inst,
                                u32                  dcb_cnt,
                                vtss_fdma_dcb_type_t dcb_type,
                                vtss_fdma_list_t     **list);

#endif /* VTSS_OPT_FDMA_VER >= 3 */

#if VTSS_OPT_FDMA_VER <= 2
/**
 * \brief Get current throttle configuration.
 *
 * Returns the current throttling configuration.
 *
 * - Call context:\n
 *   Any
 *
 * - Re-entrant:\n
 *   Yes, but doesn't really make sense.
 *
 * \param inst [IN]:  Target instance reference.
 * \param ch   [IN]:  FDMA channel number. OBSOLETE. Not used anymore. All possible channels are considered one. Set to 0.
 * \param cfg  [OUT]: Pointer to structure that receives the current throttling configuration.
 *
 * \return
 *   VTSS_RC_OK on success, VTSS_RC_ERROR if channel indicated by ch
 *   is not configured for extraction.
 */
vtss_rc vtss_fdma_throttle_cfg_get(const vtss_inst_t inst, const vtss_fdma_ch_t ch, vtss_fdma_throttle_cfg_t *const cfg);
#else
/**
 * \brief Get current throttle configuration.
 *
 * Returns the current throttling configuration.
 *
 * - Call context:\n
 *   Any
 *
 * - Re-entrant:\n
 *   Yes, but doesn't really make sense.
 *
 * \param inst [IN]:  Target instance reference.
 * \param cfg  [OUT]: Pointer to structure that receives the current throttling configuration.
 *
 * \return
 *   VTSS_RC_OK on success, VTSS_RC_ERROR if channel indicated by ch
 *   is not configured for extraction.
 */
vtss_rc vtss_fdma_throttle_cfg_get(const vtss_inst_t inst, vtss_fdma_throttle_cfg_t *const cfg);
#endif /* VTSS_OPT_FDMA_VER */

#if VTSS_OPT_FDMA_VER <= 2
/**
 * \brief Configure throttling
 *
 * Configure throttling.
 * See vtss_fdma_throttle_cfg_t for a description of the throttle feature
 * and the use of this function.
 *
 * - Call context:\n
 *   Any
 *
 * - Re-entrant:\n
 *   Yes, but doesn't really make sense.
 *
 * \param inst [IN]: Target instance reference.
 * \param ch   [IN]: FDMA channel number. OBSOLETE. Not used anymore. All possible channels are considered one. Set to 0.
 * \param cfg  [IN]: New throttling configuration.
 *
 * \return
 *   VTSS_RC_OK on success, VTSS_RC_ERROR if channel indicated by ch
 *   is not configured for extraction.
 */
vtss_rc vtss_fdma_throttle_cfg_set(const vtss_inst_t inst, const vtss_fdma_ch_t ch, const vtss_fdma_throttle_cfg_t *const cfg);
#else
/**
 * \brief Configure throttling
 *
 * Configure throttling.
 * See vtss_fdma_throttle_cfg_t for a description of the throttle feature
 * and the use of this function.
 *
 * - Call context:\n
 *   Any
 *
 * - Re-entrant:\n
 *   Yes, but doesn't really make sense.
 *
 * \param inst [IN]: Target instance reference.
 * \param cfg  [IN]: New throttling configuration.
 *
 * \return
 *   VTSS_RC_OK on success, VTSS_RC_ERROR if channel indicated by ch
 *   is not configured for extraction.
 */
vtss_rc vtss_fdma_throttle_cfg_set(const vtss_inst_t inst, const vtss_fdma_throttle_cfg_t *const cfg);
#endif /* VTSS_OPT_FDMA_VER */

/**
 * \brief Generate throttle tick.
 *
 * See vtss_fdma_throttle_cfg_t for a description of the throttle feature
 * and the use of this function.
 *
 * - Call context:\n
 *   Any
 *
 * - Re-entrant:\n
 *   Yes, but doesn't really make sense to call this function from more than one thread.
 *
 * \param inst [IN]: Target instance reference.
 *
 * \return
 *   VTSS_RC_OK - always.
 */
vtss_rc vtss_fdma_throttle_tick(const vtss_inst_t inst);

#if defined(VTSS_ARCH_LUTON28) && VTSS_OPT_FDMA_VER <= 2
/**
 * \brief Get the current front port injection flow control configuration.
 *
 * This is the counter-function to vtss_fdma_inj_flow_ctrl_cfg_set(). It
 * returns the current settings of the \@in_tx_flow_ctrl and \@retransmit_cnt
 * for a given \@phys_port. See vtss_fdma_inj_flow_ctrl_cfg_set() for details.
 *
 * - Call context:\n
 *   Thread
 *
 * - Re-entrant:\n
 *   No
 *
 * \param inst [IN]:             Target instance reference.
 * \param phys_port [IN]:        The physical port to get the settings for.
 * \param in_tx_flow_ctrl [OUT]: Pointer to a BOOL that receives the current value of
 *                               the FDMA's perception of whether the port is in CPU-
 *                               assisted injection.
 * \param retransmit_cnt [OUT]:  Pointer to a u8 that receives the current value of
 *                               the FDMA's perception of the ports retransmission count.
 * \return
 *   VTSS_RC_OK on success.\n
 *   VTSS_RC_ERROR if a supplied parameter was erroneous.
 */
vtss_rc vtss_fdma_inj_flow_ctrl_cfg_get(const vtss_inst_t inst, const vtss_phys_port_no_t phys_port, BOOL *const in_tx_flow_ctrl, u8 *const retransmit_cnt);
#endif /* defined(VTSS_ARCH_LUTON28) && VTSS_OPT_FDMA_VER <= 2 */

#if defined(VTSS_ARCH_LUTON28) && VTSS_OPT_FDMA_VER <= 2
/**
 * \brief Set front port injection flow control configuration.
 *
 *   Under certain circumstances, injections to front ports may fail. This
 *   happens in two cases:
 *   -# The FDMA writes to the super priority front port queue too fast for
 *      it to cope with. The INJ_WR_DELAY should've taken care of reducing
 *      the speed, but due to a design flaw, the maximum setting of this
 *      register is not high enough. Only the 2.5Gbps stack links exhibit
 *      this behavior.
 *   -# The super priority front port queue gets overflowed due to flow
 *      control against the port. This can happen for all front ports, if
 *      they are configured to obey flow control.
 *
 *   To remedy these problems, this function allows for the CPU controlling
 *   the final injection. The normal injection case is that the FDMA starts
 *   by injecting all data, followed by a write to a special register that
 *   tells the front port that all of the frame has been injected. If the
 *   FDMA is in "flow control mode", the FDMA still writes all the data, but
 *   upon the interrupt (call to vtss_fdma_irq_handler()), the CPU checks
 *   if all the data was written successfully, and if so, writes the "go"
 *   to the front port. If the data was not written successfully, the
 *   vtss_fdma_irq_handler() sets up the FDMA to retransmit the frame. This
 *   is attempted at most \@retransmit_cnt times, after which the inj_post_cb()
 *   function is called back with the \@dropped argument set to TRUE. The
 *   called back function can then take proper action. Typically, it will
 *   attempt a manual injection if it is for the stack links (case 1, above),
 *   which are never in flow control mode, or it will simply drop the frame
 *   if it's a 1Gbps front port that is in flow control mode (case 2, above).
 *   If attempting a manual injection, the delay between consecutive writes
 *   can be arbitrarily large, thereby assuring that the super priority
 *   queue can take off the data. See  vtss_fdma_inj_suspend_port() and
 *   vtss_fdma_inj_port_active() for a description of how to suspend the FDMA
 *   from injecting to a certain port.
 *   Notice, that there is hardware flow control between the CPU Port Module
 *   and the FDMA, which means that the FDMA cannot overflow the CPU Port
 *   Module's injection queue, whereby this is not a problem for that one.
 *
 * - Call context:\n
 *   Thread
 *
 * - Re-entrant:\n
 *   No
 *
 * \param inst [IN]:            Target instance reference.
 * \param phys_port [IN]:       The physical front port for which to enable or disable the CPU-
 *                              assisted injection control and retransmission count.
 * \param in_tx_flow_ctrl [IN]: FALSE to disable CPU-assisted injection control, TRUE
 *                              to enable it.
 * \param retransmit_cnt [IN]:  Only used if \@in_tx_flow_ctrl is TRUE. Determines
 *                              the number of times the frame will be scheduled for
 *                              retransmission by the FDMA driver code. A value of 0
 *                              thus means that the frame only will be attempted
 *                              injected once.
 *
 * \return
 *   VTSS_RC_OK on success.\n
 *   VTSS_RC_ERROR if a supplied parameter was erroneous.
 */
vtss_rc vtss_fdma_inj_flow_ctrl_cfg_set(const vtss_inst_t         inst,
                                        const vtss_phys_port_no_t phys_port,
                                        const BOOL                in_tx_flow_ctrl,
                                        const u8                  retransmit_cnt);
#endif /* defined(VTSS_ARCH_LUTON28) && VTSS_OPT_FDMA_VER <= 2 */

#if VTSS_OPT_FDMA_VER <= 2
/**
 * \brief Return used extraction channel given a list of one or more software DCBs.
 *
 * This function takes a pointer to a software DCB and investigates the
 * hardware DCB to figure out which channel originally extracted the
 * corresponding frame.
 * This is useful if the Packet Module needs to feed a list of software
 * DCBs back to the FDMA after it has been handled by higher layers. At
 * that point in time, the Packet Module itself has no idea as to which
 * channel the frame was extracted through, and therefore doesn't know
 * to which channel to feed the free list back (provided the Packet Module
 * wishes to feed the same DCBs back to where they were originally
 * assigned).
 *
 * - Call context:\n
 *   Thread
 *
 * - Re-entrant:\n
 *   Yes
 *
 * \param inst [IN]: Target instance reference.
 * \param list [IN]: Pointer to the software DCB list holding the DCB to
 *                   investigate. Only the first DCB in the list is visited.
 * \param ch [OUT]:  Pointer to a variable that receives the channel number.
 *
 * \return
 *   VTSS_RC_OK on success.\n
 *   VTSS_RC_ERROR if a supplied parameter was erroneous.
 */
vtss_rc vtss_fdma_xtr_ch_from_list(const vtss_inst_t inst, const vtss_fdma_list_t *const list, vtss_fdma_ch_t *const ch); /* Can be used to get the channel number that the frame pointed to by list was extracted from. */
#endif /* VTSS_OPT_FDMA_VER <= 2 */

#if defined(VTSS_ARCH_LUTON28)
/**
 * \brief Suspend transmission to a given port.
 *
 * This can be used to resume or gracefully suspend the FDMA from injecting
 * frames to a certain port. A call to vtss_fdma_inj_suspend_port() with
 * \@suspend == TRUE will normally have to be followed by a call to the same
 * function with \@suspend == FALSE.
 * Suspending a port doesn't mean that the port gets suspended immediately.
 * The next time the FDMA has injected a frame, and needs to be fed with
 * the next, the channel stops if the destination port is suspended.
 * This means that all frames that use the corresponding channel will be
 * put on hold even if they're destined for other ports than the suspended.
 * Since the FDMA doesn't stop immediately, but waits until a possibly
 * current frame has been injected, a suspension should be used in
 * conjunction with the vtss_fdma_inj_port_active(), which could be called
 * repeatedly to figure out when the port is ready to perform e.g. manual
 * frame injection into.
 *
 * - Call context:\n
 *   Thread
 *
 * - Re-entrant:\n
 *   Yes
 *
 * \param inst [IN]:      Target instance reference.
 * \param phys_port [IN]: Physical port number to suspend injection to.
 * \param suspend [IN]:   FALSE to stop suspension and resume FDMA usage.
 *                        TRUE to cause FDMA to suspend injection on the
 *                        next invocation of vtss_fdma_irq_handler().
 *
 * \return
 *   VTSS_RC_OK on success.\n
 *   VTSS_RC_ERROR if a supplied parameter was erroneous.\n
 *   VTSS_RC_INV_STATE if\n
 *    - \@suspend is FALSE and it's not already suspended or
 *    - The number of active injections on the port is non-zero.
 */
vtss_rc vtss_fdma_inj_suspend_port(const vtss_inst_t         inst,
                                   const vtss_phys_port_no_t phys_port,
                                   const BOOL                suspend);
#endif /* defined(VTSS_ARCH_LUTON28) */

#if defined(VTSS_ARCH_LUTON28)
/**
 * \brief Is FDMA injection active on a given port?
 *
 * Returns whether the FDMA is currently injecting frames to a given port.
 * This can be used to gracefully wait for a DMA channel having injected a
 * frame to a port, so that the port becomes available for e.g. manual
 * frame injection. You would then have to suspend the port by first calling
 * vtss_fdma_inj_suspend_port(), and then repeatedly poll for the status
 * with a call to vtss_fdma_inj_port_active() until it retuns "not active".
 *
 * - Call context:\n
 *   Thread
 *
 * - Re-entrant:\n
 *   Yes
 *
 * \param inst [IN]:      Target instance reference.
 * \param phys_port [IN]: Physical port number to get status for. Valid range is [0; VTSS_FDMA_TOTAL_PORT_CNT[.
 * \param active [OUT]:   FALSE if the FDMA is not injection frames to \@port.
 *                        TRUE if the FDMA is currently injection frames to \@port.
 *
 * \return
 *   VTSS_RC_OK on success.\n
 *   VTSS_RC_ERROR if a supplied parameter was erroneous.
 */
vtss_rc vtss_fdma_inj_port_active(const vtss_inst_t         inst,
                                  const vtss_phys_port_no_t phys_port,
                                        BOOL                *const active);
#endif /* defined(VTSS_ARCH_LUTON28) */

#if VTSS_OPT_FDMA_VER <= 2
/**
 * \brief Decoded extraction header properties.
 *
 * This structure is populated with a call to vtss_fdma_xtr_hdr_decode().
 *
 * Every decoded parameter has two fields in the structure:
 * One indicating the value of the parameter (e.g. src_port),
 * and another indicating if the parameter is actually decoded or not
 * (e.g. src_port_decoded).
 *
 * The reason for having an XXX_decoded boolean for every parameter is that
 * the information held in the extraction header is very different on the
 * various Vitesse chips.
 *
 */
typedef struct {
    /**
     * The physical port number on which the frame was received.
     * Validity:
     *   Luton26: Y
     *   Luton28: Y
     *   Jaguar1: Y
     *   Serval : Y
     */
    vtss_phys_port_no_t src_port;
    /**
     * TRUE when #src_port contains valid information, FALSE otherwise.
     */
    BOOL                src_port_decoded;

    /**
     * The chip on which the frame was received (dual chip solutions)
     * Validity:
     *   Luton26: Y (always 0)
     *   Luton28: Y (always 0)
     *   Jaguar1: Y
     *   Serval : Y (always 0)
     */
    vtss_chip_no_t chip_no;
    /**
     * TRUE when #chip_no contains valid information, FALSE otherwise.
     */
    BOOL           chip_no_decoded;

    /**
     * If set, frame was received with a tag that was removed.
     * Validity:
     *   Luton26: N
     *   Luton28: Y
     *   Jaguar1: N
     *   Serval : N
     */
    BOOL was_tagged;
    /**
     * TRUE when #was_tagged contains valid information, FALSE otherwise.
     */
    BOOL was_tagged_decoded;

    /**
     * The tag information's associcated Tag Protocol Identifier.
     * The definitions are:
     *   0: C-Tag: EtherType = 0x8100.
     *   1: S-Tag: EtherType = 0x88A8 or custom value.
     * Validity:
     *   Luton26: Y
     *   Luton28: Y
     *   Jaguar1: Y
     *   Serval : Y
     */
    u8   tag_type;
    /**
     * TRUE when #tag_type contains valid information, FALSE otherwise.
     */
    BOOL tag_type_decoded;

    /**
     * The 12-bit VLAN ID from the VLAN classification.
     * Validity:
     *   Luton26: Y
     *   Luton28: Y
     *   Jaguar1: Y
     *   Serval : Y
     */
    u16  vid;
    /**
     * TRUE when #vid contains valid information, FALSE otherwise.
     */
    BOOL vid_decoded;

    /**
     * The 10-bit ISDX from IS1 classification.
     * Validity:
     *   Luton26: N
     *   Luton28: N
     *   Jaguar1: N
     *   Serval : Y
     */
    u16  isdx;
    /**
     * TRUE when #isdx contains valid information, FALSE otherwise.
     */
    BOOL isdx_decoded;

    /**
     * The frame's 1-bit CFI/DEI field from the VLAN classification.
     * Validity:
     *   Luton26: Y
     *   Luton28: Y
     *   Jaguar1: Y
     *   Serval : Y
     */
    BOOL dei;
    /**
     * TRUE when #dei contains valid information, FALSE otherwise.
     */
    BOOL dei_decoded;

    /**
     * The frame's 3-bit uprio/PCP field from the VLAN classification.
     * Validity:
     *   Luton26: Y
     *   Luton28: Y
     *   Jaguar1: Y
     *   Serval : Y
     */
    u8   pcp;
    /**
     * TRUE when #pcp contains valid information, FALSE otherwise.
     */
    BOOL pcp_decoded;

    /**
     * The reason for copying or redirecting the frame to the CPU.
     * The definitions are:
     *   000: CPU_NORMAL
     *   001: CPU_ONLY_RSVD
     *   010: CPU_COPY_IPMC
     *   011: Reserved
     *   100: CPU_ONLY_IGMP
     *   101: CPU_ONLY_MLD
     *   110: Reserved
     *   111: CPU_COPY_ACL
     * Validity:
     *   Luton26: N
     *   Luton28: Y
     *   Jaguar1: N
     *   Serval : N
     */
    u8   frame_type;
    /**
     * TRUE when #frame_type contains valid information, FALSE otherwise.
     */
    BOOL frame_type_decoded;

    /**
     * If set, the frame's SMAC is subject to learning.
     * Validity:
     *   Luton26: N
     *   Luton28: Y
     *   Jaguar1: N
     *   Serval : N
     */
    BOOL learn;
    /**
     * TRUE when #learn contains valid information, FALSE otherwise.
     */
    BOOL learn_decoded;

    /**
     * The CPU extraction queue number when forwarding the frame
     * to the CPU port module.
     * Validity:
     *   Luton26: Y
     *   Luton28: Y
     *   Jaguar1: Y
     *   Serval : Y
     */
    u16  xtr_qu;
    /**
     * TRUE when #xtr_qu contains valid information, FALSE otherwise.
     */
    BOOL xtr_qu_decoded;

    /**
     * CPU extraction queue mask (one bit per CPU extraction queue).
     * Each bit implies te frame was subject to CPU forwarding to the
     * specific queue.
     * Validity:
     *   Luton26: Y
     *   Luton28: N
     *   Jaguar1: Y (but in some cases, it is constructed rather than showing the true story (constructed if the frame hit a non-default VCAP IS2 rule)
     *   Serval : Y
     */
    u32  xtr_qu_mask;
    /**
     * TRUE when #xtr_qu_mask contains valid information, FALSE otherwise.
     */
    BOOL xtr_qu_mask_decoded;

    /**
     * The frame's classified DSCP value.
     * Luton26:
     *   If the frame is hardware timestamped (frame has hit a rule in IS2
     *   with PTP_ENA), the DSCP field contains the timestamp identifier
     *   provided by the analyzer.
     *
     * Validity:
     *   Luton26: Y
     *   Luton28: N
     *   Jaguar1: Y
     *   Serval : N
     */
    u8   dscp;
    /**
     * TRUE when #dscp contains valid information, FALSE otherwise.
     */
    BOOL dscp_decoded;

    /**
     * Two-step PTP Tx timestamp identifier (6 bits).
     *
     * Validity:
     *   Luton26: N
     *   Luton28: N
     *   Jaguar1: N
     *   Serval : Y
     */
    u32 tstamp_id;
    /**
     * TRUE when #tstamp_id contains valid information, FALSE otherwise.
     */
    BOOL tstamp_id_decoded;


    /**
     * The frame's ingress timestamp.
     *
     * Jaguar1:
     *   Frames getting copied to the CPU for SFlow reasons can never have a valid #tstamp (such frames
     *   will be indicated through #tstamp_decoded == FALSE).
     *   Note that the #tstamp_decoded will indicate TRUE for all other frames, but this is only reliable
     *   if the following register are set-up:
     *     ANA_AC:PS_COMMON:MISC_CTRL.OAM_RX_FRM_EXTEND_ENA    == 0
     *     ANA_AC:PS_COMMON:MISC_CTRL.OAM_RX_TSTAMP_IN_FCS_ENA == 1
     *     ASM:CFG:ETH_CFG.ETH_PRE_MODE                        == 1
     *     DEV1G/DEV25G:DEV_CFG_STATUS:DEV_PTP_CFG.PTP_ENA     == 1
     *     DEV10G:DEV_CFG_STATUS:DEV_PTP_CFG.PTP_CFG_ENA       == 1
     *     DEVCPU_GCB:PTP_CFG:PTP_MISC_CFG.PTP_ENA             == 1
     *
     * Serval:
     *   Two-step PTP frames have the 32-bit Rx timestamp saved in this field.
     *
     * Validity:
     *   Luton26: N
     *   Luton28: N
     *   Jaguar1: Y
     *   Serval : Y
     */
    u32  tstamp;
    /**
     * TRUE when #tstamp contains valid information, FALSE otherwise.
     */
    BOOL tstamp_decoded;

    /**
     * Various un-decodable OAM info. Decoding of the OAM info
     * field from the extraction header requires accompanying
     * frame info, and is therefore saved as an opaque type,
     * letting it be up to the application to decode it based on
     * description in the datasheet.
     *
     * Serval:
     *   This corresponds to the contents of the REW_VAL field
     *   in the extraction header. oam_info_decoded = TRUE when
     *   REW_OP[2:0] == 4. Only the 32 lsbits of #oam_info are used.
     *
     * Validity:
     *   Luton26: N
     *   Luton28: N
     *   Jaguar1: N
     *   Serval : Y
     */
    u64  oam_info;
    /**
     * TRUE when #oam_info contains valid information, FALSE otherwise.
     */
    BOOL oam_info_decoded;

    /**
     *   If #acl_hit is set, this value is the entry number of the rule
     *   hit in IS2. If both IS2 lookups hit a rule which copy the frame
     *   to the CPU, then the second lookup's entry number is used.
     *
     *   For Serval, this is the combined ACL_ID action coming out of the
     *   two IS2 look-ups.
     * Validity:
     *   Luton26: Y
     *   Luton28: N
     *   Jaguar1: Y
     *   Serval : Y
     */
    u32  acl_idx;
    /**
     * TRUE when #acl_idx contains valid information, FALSE otherwise.
     */
    BOOL acl_idx_decoded;

    /**
     * SFlow type. Indicates if this is copied to the CPU due to Rx or Tx SFlow.
     *
     * Encoding - only valid if sflow_type_decoded == TRUE:
     *  0: Rx SFlow.
     *  1: Tx SFlow.
     *
     *  If #sflow_type_decoded == TRUE, the Rx or Tx port that gave rise to this is given by #sflow_port.
     *
     * Jaguar:
     *   Note: The #sflow_type is only reliable if ANA_AC:PS_COMMON:PS_COMMON_CFG.SFLOW_SMPL_ID_IN_STAMP_ENA is set to 1.
     *   The #sflow_type_decoded is always reliable.
     *
     * Validity:
     *   Luton26: Y
     *   Luton28: N
     *   Jaguar1: Y
     *   Serval : Y
     */
     u8   sflow_type;
    /**
     * TRUE when #sflow_type contains valid information, FALSE otherwise.
     */
     BOOL sflow_type_decoded;

    /**
     * SFlow port.
     * Indicates the SFlow Rx or Tx port number that caused this frame to be copied to the CPU.
     * #sflow_port_decoded can only be set if #sflow_type_decoded == TRUE.
     * Jaguar:
     *   Note: This is only reliable if ANA_AC:PS_COMMON:PS_COMMON_CFG.SFLOW_SMPL_ID_IN_STAMP_ENA is set to 1.
     *   That bit must be set if Tx SFlow are enabled. If only using Rx SFlows, that bit can be cleared, and
     *   you may use the #src_port member to figure out which port caused this frame.
     * Validity:
     *   Luton26: Y
     *   Luton28: N
     *   Jaguar1: Y
     *   Serval : Y
     */
    u32  sflow_port;
    /**
     * TRUE when #sflow_port contains valid information, FALSE otherwise.
     */
    BOOL sflow_port_decoded;

    /**
     * SFlow frame fwd count.
     * Indicates the total number of frames transmitted. This is normally an 18-bit counter, but if
     * ANA_AC:PS_COMMON_CFG.SFLOW_SMPLD_ID_IN_STAMP_ENA is set to 0, then you may obtain a 24-bit counter
     * by concatenating the 6 LSbits of #sflow_port with the 18 LSbits of #sflow_fwd_cnt.
     * Validity:
     *   Luton26: N
     *   Luton28: N
     *   Jaguar1: Y
     *   Serval : N
     */
    u32  sflow_fwd_cnt;
    /**
     * TRUE when #sflow_fwd_cnt contains valid information, FALSE otherwise.
     */
    BOOL sflow_fwd_cnt_decoded;

    /**
     * SFlow sampled frames count.
     * Indicates the number of SFlow sampled frames.
     * Validity:
     *   Luton26: N
     *   Luton28: N
     *   Jaguar1: Y
     *   Serval : N
     */
    u32  sflow_sample_cnt;
    /**
     * TRUE when #sflow_sample_cnt contains valid information, FALSE otherwise.
     */
    BOOL sflow_sample_cnt_decoded;

    /**
     * Set if frame has hit a rule in IS2, which copies the frame to the
     * CPU (IS2 actions CPU_COPY_ENA or HIT_ME_ONCE). #acl_idx contains
     * the IS2 entry number.
     * For Serval, the #acl_idx contains the combined ACL_ID action of
     * the rules hit in IS2.
     * Validity:
     *   Luton26: Y
     *   Luton28: N
     *   Jaguar1: Y
     *   Serval : Y
     */
    BOOL acl_hit;
    /**
     * TRUE when #acl_hit contains valid information, FALSE otherwise.
     */
    BOOL acl_hit_decoded;

    /**
     * The frame's drop precedence (DP) level after policing.
     * Validity:
     *   Luton26: Y
     *   Luton28: N
     *   Jaguar1: Y
     *   Serval : Y
     */
    u8   dp;
    /**
     * TRUE when #dp contains valid information, FALSE otherwise.
     */
    BOOL dp_decoded;

    /**
     * The source MAC address learning action triggered by the frame.
     * 0: No learning.
     * 1: Learning of a new entry.
     * 2: Updating of an already learned unlocked entry.
     * 3: Updating of an already learned locked entry.
     * Validity:
     *   Luton26: Y
     *   Luton28: N
     *   Jaguar1: N
     *   Serval : Y
     */
    u8   lrn_flags;
    /**
     * TRUE when #lrn_flags contains valid information, FALSE otherwise.
     */
    BOOL lrn_flags_decoded;

    /**
     * The frame's classified QoS class.
     * Validity:
     *   Luton26: Y
     *   Luton28: Y
     *   Jaguar1: Y
     *   Serval : Y
     */
    u8   qos_class;
    /**
     * TRUE when #qos_class contains valid information, FALSE otherwise.
     */
    BOOL qos_class_decoded;

#if defined(VTSS_FEATURE_VSTAX) && VTSS_OPT_FDMA_VER == 2
    /**
     * If the frame was received with a stack header, #contains_stack_hdr
     * is TRUE, otherwise FALSE. Irrespective of the architecture (Luton28 or
     * Jaguar), the VStax header is stripped from the actual frame data and
     * placed in #stack_hdr_bin. See that field for further instructions.
     * Validity:
     *   Luton26: N
     *   Luton28: Y (if VTSS_OPT_FDMA_VER == 2)
     *   Jaguar1: Y
     *   Serval : N
     */
    BOOL contains_stack_hdr;

    /**
     * This field contains the raw, binary stack header and is valid
     * if #contains_stack_hdr is TRUE.
     * If needed, this can be deocded with a call to vtss_packet_vstax_frame2header().
     * Validity:
     *   Luton26: N
     *   Luton28: Y (if VTSS_OPT_FDMA_VER == 2)
     *   Jaguar1: Y
     *   Serval : N
     */
    u8 stack_hdr_bin[VTSS_VSTAX_HDR_SIZE];
#endif /* VTSS_FEATURE_VSTAX && VTSS_OPT_FDMA_VER == 2 */

} vtss_fdma_xtr_props_t;
#endif /* VTSS_OPT_FDMA_VER <= 2 */

#if VTSS_OPT_FDMA_VER <= 2 /* For > 2, use vtss_packet_rx_hdr_decode() */
/**
 * \brief Decode an extraction header.
 *
 * The extraction header is a chip-specific, bit-encoded entity normally
 * stored in the front of extracted frames.
 * This function takes a pointer to the extraction header and decodes it
 * into a structure.
 *
 * - Call context:\n
 *   Thread
 *
 * - Re-entrant:\n
 *   Yes
 *
 * \param inst [IN]:        Target instance reference.
 * \param list [IN]:        Pointer to list of software DCBs making up this frame.
                            Note: Only one frame must be passed to this function, possibly
                            split into several fragments (DCBs). The last fragment's next
                            field must point to NULL
 * \param xtr_props [OUT]:  Pointer to a structure into which this function
 *                          will store the decoded extraction header properties.
 *
 * \return
 *   VTSS_RC_OK on success.\n
 *   VTSS_RC_ERROR if a supplied parameter was erroneous.
 */
vtss_rc vtss_fdma_xtr_hdr_decode(const vtss_inst_t           inst,
                                 const vtss_fdma_list_t      *const list,
                                       vtss_fdma_xtr_props_t *const xtr_props);
#endif /* VTSS_OPT_FDMA_VER <= 2 */

/*
 * Mirroring from CPU of directed frames happen automatically
 * when composing injection header with vtss_packet_tx_hdr_encode()
 * on platforms using VTSS_OPT_FDMA_VER > 2
 */

#if VTSS_OPT_FDMA_VER <= 2
/**
 * \brief Get the current mirror port.
 *
 * This function returns the current mirror port (if any).
 * See details in vtss_fdma_mirror_port_set().
 *
 * Validity:
 *   Luton26: Y
 *   Luton28: N
 *   Jaguar1: N
 *   Serval : Y
 *
 * - Call context:\n
 *   Thread
 *
 * - Re-entrant:\n
 *   No
 *
 * \param inst       [IN]: Target instance reference.
 * \param chip_no   [OUT]: The chip number of phys_port.
 * \param phys_port [OUT]: The PHYSICAL port that directed frames are mirrored to.
 *                         VTSS_PORT_NO_NONE if mirroring is disabled.
 *
 * \return
 *   VTSS_RC_OK on success.\n
 *   VTSS_RC_ERROR if a supplied parameter was erroneous.
 */
vtss_rc vtss_fdma_inj_mirror_port_get(const vtss_inst_t inst, vtss_chip_no_t *const chip_no, vtss_phys_port_no_t *const phys_port);
#endif /* VTSS_OPT_FDMA_VER <= 2 */

#if VTSS_OPT_FDMA_VER <= 2
/**
 * \brief Copy all frames injected to a specific port to another port.
 *
 * This function allows for turning off and turning on mirroring of
 * CPU-generated frames to a given port (the mirror port).
 * It only affects frames that are not sent switched, i.e. it
 * only affects frames directed to one or more front ports, i.e.
 * frames for which vtss_fdma_inj_props_t::switch_frm is FALSE.
 * Set-up of mirroring of frames that go through the analyzer must be
 * done elsewhere.
 *
 * Validity:
 *   Luton26: Y
 *   Luton28: N
 *   Jaguar1: N
 *   Serval : Y
 *
 * - Call context:\n
 *   Thread
 *
 * - Re-entrant:\n
 *   No
 *
 * \param inst      [IN]: Target instance reference.
 * \param chip_no   [IN]: Chip number of phys_port.
 * \param phys_port [IN]: The PHYSICAL port to mirror directed frames to.
 *                        Set it to VTSS_PORT_NO_NONE to disable mirroring.
 *
 * \return
 *   VTSS_RC_OK on success.\n
 *   VTSS_RC_ERROR if a supplied parameter was erroneous.
 */
vtss_rc vtss_fdma_inj_mirror_port_set(const vtss_inst_t inst, vtss_chip_no_t chip_no, vtss_phys_port_no_t phys_port);
#endif /* VTSS_OPT_FDMA_VER <= 2 */

#if defined(VTSS_ARCH_LUTON28) && VTSS_OPT_FDMA_VER <= 2
/**
 * \brief FDMA Statistics.
 *
 * Use vtss_fdma_stats_get() to retrieve current statistics.
 *
 * \em DEPRECATED. On newer platforms, statistics cannot be obtained
 *     directly. Instead, use vtss_debug_info_print(print_func, info) with
 *     \@info.layer set to 'cil' and
 *     \@info.group set to VTSS_DEBUG_GROUP_FDMA.
 */
typedef struct tag_vtss_fdma_stats
{
  u32 rx_packets[VTSS_PACKET_RX_QUEUE_CNT];    /**< Number of extracted frames per extraction queue.    */
  u64 rx_bytes[VTSS_PACKET_RX_QUEUE_CNT];      /**< Number of extracted bytes per extraction queue.     */
  u32 rx_bufs_used[VTSS_PACKET_RX_QUEUE_CNT];  /**< Number of software DCBs used per extraction queue.  */
  u32 rx_bufs_added[VTSS_PACKET_RX_QUEUE_CNT]; /**< Number of software DCBs added per extraction queue. */
  u32 rx_dropped;                              /**< Number of dropped frames per extraction queue. Only frames that were originally sent from CPU PM and (due to silicon flaw) looped back. */

  u32 tx_packets[VTSS_FDMA_CH_CNT];            /**< Number of injected frames per FDMA channel.              */
  u64 tx_bytes[VTSS_FDMA_CH_CNT];              /**< Number of injected bytes per FDMA channel.               */
  u32 tx_bufs_used[VTSS_FDMA_CH_CNT];          /**< Number of injection software DCBs used per FDMA channel. */
  u32 tx_dropped[VTSS_FDMA_TOTAL_PORT_CNT];    /**< Number of frames dropped by FDMA due to flow control.    */

  u32 intr_cnt;                                /**< Number of interrupts (calls to vtss_fdma_irq_handler()). */
} vtss_fdma_stats_t;
#endif /* defined(VTSS_ARCH_LUTON28) && VTSS_OPT_FDMA_VER <= 2 */

#if defined(VTSS_ARCH_LUTON28) && VTSS_OPT_FDMA_VER <= 2
/**
 * \brief Get FDMA statistics.
 *
 * The FDMA driver code continuously
 * updates the current number of injected and extracted packets and bytes,
 * together with the number of DCBs added to the buffer pool and actually
 * used, and the number of times vtss_fdma_irq_handler() is called.
 *
 * - Call context:\n
 *   Thread
 *
 * - Re-entrant:\n
 *   Yes
 *
 * \param inst [IN]:   Target instance reference.
 * \param stats [OUT]: Pointer to structure receiving statistics.
 *
 * \return
 *   VTSS_RC_OK on success.\n
 *   VTSS_RC_ERROR if a supplied parameter was erroneous.
 *
 * \em DEPRECATED. On newer platforms, statistics cannot be obtained
 *     directly. Instead, use vtss_debug_info_print(print_func, info) with
 *     \@info.layer set to 'ail' or 'cil' and
 *     \@info.group set to VTSS_DEBUG_GROUP_FDMA.
 */
vtss_rc vtss_fdma_stats_get(const vtss_inst_t inst, vtss_fdma_stats_t *const stats);
#endif /* defined(VTSS_ARCH_LUTON28) && VTSS_OPT_FDMA_VER <= 2 */

/**
 * \brief Clear FDMA statistics.
 *
 * - Call context:\n
 *   Thread
 *
 * - Re-entrant:\n
 *   Yes
 *
 * \param inst [IN]: Target instance reference.
 *
 * \return
 *   VTSS_RC_OK on success.\n
 *   VTSS_RC_ERROR if supplied parameter was erroneous.
 */
vtss_rc vtss_fdma_stats_clr(const vtss_inst_t inst);

/**
 * \brief FDMA Interrupt Handler.
 *
 * This function is the heart-beat of all FDMA silicon communication.
 * The driver code enables interrupts on the FDMA silicon and expects this
 * function to be called whenver the FDMA generates interrupts.
 * The function is not re-entrant, and it must *not* be interrupted by
 * other non-interrupt code. This function does not call any of the
 * VTSS_OS_INTERRUPT_DISABLE() or VTSS_OS_SCHEDULER_LOCK() macros.
 *
 * The function first checks for extraction channels interrupting and
 * calls back the relevant xtr_cb() functions with one frame at a time.
 * The function calls back for every frame that the FDMA has extxracted
 * since last call.
 * Then it checks for injection completion, takes care of retransmission
 * if that is needed, and calls back the relevant inj_post_cb() functions
 * for every frame that has been injected.
 *
 * This means that both xtr_cb() and inj_post_cb() callback functions are called
 * from the same context as vtss_fdma_irq_handler() is called. BEWARE!!
 *
 * To better understand what is required by the caller of this function,
 * let's branch on the two basic options:
 *
 * - Interrupt Driven Operation:\n
 *   If interrupts are supported, it is important that the FDMA interrupt
 *   is disabled by the "top half" or ISR if it finds out that the
 *   interrupt is for the FDMA. If the OS supports top and bottom halves
 *   like Linux (a.k.a. ISRs and DSRs in eCos), it is recommended to call
 *   vtss_fdma_irq_handler() in the bottom half/DSR rather than in the
 *   top half/ISR. In the bottom half/DSR case, it is ensured that no other
 *   bottom halves/DSRs or threads can preempt the handler. Once the handler
 *   returns, the caller should clear and re-enable top-level FDMA interrupts.
 *   In this scenario VTSS_OPT_FDMA_IRQ_CONTEXT should be defined to 0,
 *   causing the thread code (all other vtss_fdma_XXX() functions) to use the
 *   VTSS_OS_SCHEDULER_LOCK()/UNLOCK() functions.
 *
 *   If the OS doesn't support top/bottom halves, but still supports
 *   interrupt handling, vtss_fdma_irq_handler() may be called directly
 *   from the ISR, which should still start by disabling top-level FDMA interrupts,
 *   then call the handler, and finally clear and re-enable them. In this scendario
 *   VTSS_OPT_FDMA_IRQ_CONTEXT be defined to 1. This causes the thread code
 *   (all other vtss_fdma_XXX() functions) to use the
 *   VTSS_OS_INTERRUPT_DISABLE()/RESTORE() functions whenever it is using or
 *   updating state members also used by the interrupt handler.
 *
 * - Polled Operation:\n
 *   Since the FDMA driver code reads registers directly in the FDMA silicon
 *   to figure out which channels are interrupting, and since it allows for
 *   "no channels want the CPU's attention", it is possible to use
 *   a polled approach rather than an interrupt driven approach. In a single-
 *   threaded application you can call it once in the round-robin trip,
 *   because it cannot be interrupted by any other code. In a multi-threaded
 *   application you will have to disable the scheduler before calling it,
 *   and re-enable it afterwards. This ensures that no other functions
 *   can call e.g. vtss_fdma_inj() while servicing the "interrupt". It is
 *   not good enough to disable interrupts, because the code calls macros
 *   like "output this debug message to the console", which may end up in
 *   the OS kernel, which in turn may schedule the vtss_fdma_irq_handler()
 *   function out in favor of another thread.
 *
 * - Call context:\n
 *   OS-Specific. See description below.
 *
 * - Re-entrant:\n
 *   <B>NO!!!</B>
 *
 * \param inst [IN]:  Target instance reference.
 * \param cntxt [IN]: A user-defined argument, which is passed to the inj_post_cb()
 *                    and xtr_cb() callback functions. Rarely needed.
 *
 * \return
 *   VTSS_RC_OK on success.\n
 *   VTSS_RC_ERROR if \@inst parameter was erroneous.
 */
vtss_rc vtss_fdma_irq_handler(const vtss_inst_t inst, void *const cntxt);

#endif /* defined(VTSS_FEATURE_FDMA) && VTSS_OPT_FDMA */

#endif /* _VTSS_FDMA_API_H_ */
/******************************************************************************
 *
 * End of file
 *
 *****************************************************************************/
