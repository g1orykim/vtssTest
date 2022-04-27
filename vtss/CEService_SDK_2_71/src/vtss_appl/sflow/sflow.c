/*

 Vitesse sFlow software.

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

#include <network.h>             /* For socket(), sendto(), close()                   */
#include "packet_api.h"          /* For Rx of sFlow marked packets                    */
#include "critd_api.h"           /* For mutex wrapper                                 */
#include "msg_api.h"             /* For msg_XXX() functions                           */
#include "vtss_timer_api.h"      /* For vtss_timer_XXX() functions                    */
#include "port_api.h"            /* For port_isid_port_count() et al.                 */
#include "vtss_bip_buffer_api.h" /* For BIP-buffers                                   */
#include "topo_api.h"            /* For topo_isid2usid() used by GET_UNIQUE_PORT_ID() */
#include "misc_api.h"            /* For iport2uport() used by GET_UNIQUE_PORT_ID()    */
#include "sflow_api.h"           /* Our own definitions                               */
#ifdef VTSS_SW_OPTION_VCLI
#include "sflow_cli.h"           /* For sflow_cli_init()                              */
#endif
#include <network.h>             /* For htonl() and htonll()                          */
#include "ip2_utils.h"           /* FOr vtss_ip_addr_is_zero()                        */

// Since we transfer (from a switch to the master) all counter and flow samples
// only once per such sample, we pass an instance-mask telling the master which
// instances were the reason for that sample. That instance-mask is only 32 bits wide.
#if SFLOW_INSTANCE_CNT > 32
#error "At most able to handle 32 sFlow instances per port"
#endif

#define SFLOW_INLINE inline

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_SFLOW

/******************************************************************************/
// Trace definitions
/******************************************************************************/
#include <vtss_module_id.h>      /* For SFLOW module ID */
#include <vtss_trace_lvl_api.h>
#include <vtss_trace_api.h>

#if VTSS_TRACE_ENABLED
#include "sflow_trace.h"

static vtss_trace_reg_t trace_reg = {
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "sFlow",
    .descr     = "sFlow"
};

static vtss_trace_grp_t trace_grps[TRACE_GRP_CNT] = {
    [VTSS_TRACE_GRP_DEFAULT] = {
        .name        = "default",
        .descr       = "Default",
        .lvl         = VTSS_TRACE_LVL_ERROR,
        .timestamp   = 1,
        .usec        = 1,
    },
    [TRACE_GRP_ICLI] = {
        .name        = "iCLI",
        .descr       = "iCLI",
        .lvl         = VTSS_TRACE_LVL_ERROR,
        .timestamp   = 1,
        .usec        = 1,
    },
    [TRACE_GRP_CRIT] = {
        .name        = "crit",
        .descr       = "Critical regions",
        .lvl         = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    }
};

#define SFLOW_CRIT_ENTER()             critd_enter(        &SFLOW_crit,     TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define SFLOW_CRIT_EXIT()              critd_exit(         &SFLOW_crit,     TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define SFLOW_CRIT_ASSERT_LOCKED()     critd_assert_locked(&SFLOW_crit,     TRACE_GRP_CRIT,                       __FILE__, __LINE__)
#define SFLOW_BIP_CRIT_ENTER()         critd_enter(        &SFLOW_bip_crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define SFLOW_BIP_CRIT_EXIT()          critd_exit(         &SFLOW_bip_crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define SFLOW_BIP_CRIT_ASSERT_LOCKED() critd_assert_locked(&SFLOW_bip_crit, TRACE_GRP_CRIT,                       __FILE__, __LINE__)
#else
#define SFLOW_CRIT_ENTER()             critd_enter(        &SFLOW_crit)
#define SFLOW_CRIT_EXIT()              critd_exit (        &SFLOW_crit)
#define SFLOW_CRIT_ASSERT_LOCKED()     critd_assert_locked(&SFLOW_crit)
#define SFLOW_BIP_CRIT_ENTER()         critd_enter(        &SFLOW_bip_crit)
#define SFLOW_BIP_CRIT_EXIT()          critd_exit(         &SFLOW_bip_crit)
#define SFLOW_BIP_CRIT_ASSERT_LOCKED() critd_assert_locked(&SFLOW_bip_crit)
#endif /* VTSS_TRACE_ENABLED */

#define SFLOW_ASSERT(_expr_, _action_...) do {if (!(_expr_)) {T_E("Assertion failed: " #_expr_); _action_}} while (0)

#define IANA_IF_TYPE_ETHERNET 6 /* According to RFC3635 and http://www.iana.org/assignments/ianaiftype-mib */

// Local state
typedef struct {
    BOOL                   cp_active;
    u32                    cp_seconds_left[SFLOW_INSTANCE_CNT];
    vtss_port_counter_t    cp_prev_rx_octets[SFLOW_INSTANCE_CNT];
    vtss_port_counter_t    cp_prev_tx_octets[SFLOW_INSTANCE_CNT];
    u32                    cp_sequence_number[SFLOW_INSTANCE_CNT];
    vtss_port_counter_t    fs_start_rx_packets[SFLOW_INSTANCE_CNT];
    vtss_port_counter_t    fs_start_tx_packets[SFLOW_INSTANCE_CNT];
    u32                    fs_sequence_number[SFLOW_INSTANCE_CNT];
    u32                    fs_samples_left[SFLOW_INSTANCE_CNT];
    vtss_sflow_port_conf_t fs_api_conf;
    u64                    port_speed_bps; // A vtss_port_speed_t converted to bits per second.
    BOOL                   port_link;
    BOOL                   port_fdx;
} sflow_port_state_t;

typedef struct {
    sflow_port_state_t port[VTSS_PORTS];
} sflow_switch_state_t;

/**
 * sflow_info_exchange_t
 *
 * This structure is meant for exchanging information between
 * SFLOW_sample_rx() and the SFLOW_thread() in a way that
 * doesn't require any of the two to take SFLOW_crit.
 *
 * We don't want to block the Packet Rx thread by waiting for
 * an overloaded SFLOW_crit, so for the flow samples themselves,
 * we store them in a BIP buffer and signal a flag that wakes up
 * the sFlow thread. However, other data has to be exhanged between
 * the two entities, which are:
 * SFLOW_thread-to-SFLOW_sample_rx():
 *   Maximum header size
 *
 * SFLOW_sample_rx()-to-SFLOW_thread():
 *   Number of dropped Rx samples.
 *   Number of dropped Tx samples.
 *
 * The max. header size cannot be taken from the SFLOW_local_state[]
 * array, because these members are protected by SFLOW_crit, which
 * should be avoided taken by Packet Rx thread.
 *
 * The number of dropped samples cannot be passed from the Rx thread
 * to the sFlow thread through use of the SFLOW_crit (same reason as before),
 * and it cannot be passed through the local BIP buffer, since the reason for
 * passing the information is that the BIP buffer is full.
 *
 * Instead, we protect updates of this single structure by the
 * SFLOW_bip_crit, which is taken for very short periods of time.
 */
typedef struct {
    u32 max_header_size; // Written by SFLOW_thread(), read by SFLOW_sample_rx()
    u32 drops;           // Written by SFLOW_sample_rx() and read by SFLOW_thread(). Only reset with a master change or boot.
} sflow_info_exchange_t;

/**
 * sflow_port_cfg_t.
 *
 * Per-port flow sampler and counter poller configuration.
 */
typedef struct {
    sflow_fs_t fs[SFLOW_INSTANCE_CNT];
    sflow_cp_t cp[SFLOW_INSTANCE_CNT];
} sflow_port_cfg_t;

/**
 * sflow_switch_cfg_t.
 *
 * Per-switch flow sampler and counter poller configuration.
 */
typedef struct {
    sflow_port_cfg_t port[VTSS_PORTS];
} sflow_switch_cfg_t;

typedef struct {
    int             sockfd;
    struct addrinfo *addrinfo;
    char            ip_addr_str[INET6_ADDRSTRLEN]; // The latest IP address from DNS lookup
    u32             dgram_len_dwords;
    u32             dgram[(SFLOW_RECEIVER_DATAGRAM_SIZE_MAX + 3) / 4];
    u32             sample_cnt;
    u32             sequence_number;
    u32             timeout_left;
} sflow_rcvr_state_t;

typedef enum {
    SFLOW_META_SAMPLE_TYPE_FLOW,
    SFLOW_META_SAMPLE_TYPE_COUNTER
} sflow_meta_sample_type_t;

// In order to offload the master switch, each slave switch assembles flow
// and counter samples directly into the format required by the final UDP datagram.
// This means that we cannot reliably overlay a  C structure over the data,
// because it's compiler-dependent how alignment of the fields will become.
// Furthermore, we need to collect and pack a sequence of one or more samples
// into the same buffer. Both these arguments call for an opaque data type.
// In order to be able to index selected fields of this opaque data type,
// we create a number of #defines to index into the opaque buffer.
// The offsets are 32-bit offsets, not byte-offsets.
//   The SFLOW_MSG_SAMPLE_HEADER_IDX_xxx are used by the master to index meta
//   data required to figure out the following contents. These fields
//   are host-encoded (not network encoded) since they're not going to be
//   part of the UDP datagram.
//
//   The SFLOW_MSG_SAMPLE_COUNTER_IDX_xxx are used in counter samples.
//   The master, who is the only one capable of translating a slave's iport
//   to an ifIndex, must know where to update.
//
//   The SFLOW_MSG_SAMPLE_FLOW_IDX_xxx are used in flow samples.
//   The master, who is the only one capable of translating a slave's iport
//   to an ifIndex, must know where to update.
#define SFLOW_MSG_SAMPLE_HEADER_IDX_TYPE           0 /* sflow_meta_sample_type_t */
#define SFLOW_MSG_SAMPLE_HEADER_IDX_LEN_DWORDS     1 /* Length in DWORDS of the counter or flow sample, not including this header */
#define SFLOW_MSG_SAMPLE_HEADER_IDX_PORT           2 /* Port on the slave switch to which this sample pertains */
#define SFLOW_MSG_SAMPLE_HEADER_IDX_INSTANCE_MASK  3 /* Bitmask of port instances that should receive this sample. */
#define SFLOW_MSG_SAMPLE_HEADER_IDX_SAMPLE_POOL    4 /* Array of sample pool counters (one per possible sFlow instance) to be replaced in the appropriate field of the UDP flow sample datagram. Not used for counter samples */
#define SFLOW_MSG_SAMPLE_HEADER_IDX_SEQ_NUMBERS    (SFLOW_MSG_SAMPLE_HEADER_IDX_SAMPLE_POOL + SFLOW_INSTANCE_CNT) /* Array of sequence numbers (one per possible sFlow instance) to be replaced in the appropriate field of the UDP datagram. */
#define SFLOW_MSG_SAMPLE_HEADER_LEN_DWORDS         (SFLOW_MSG_SAMPLE_HEADER_IDX_SEQ_NUMBERS + SFLOW_INSTANCE_CNT) /* DWORD length of internal header */

#define SFLOW_MSG_SAMPLE_COUNTER_IDX_FIRST         (SFLOW_MSG_SAMPLE_HEADER_LEN_DWORDS +  0) /* Index of first DWORD of real counter sample */
#define SFLOW_MSG_SAMPLE_COUNTER_IDX_SEQ_NUMBER    (SFLOW_MSG_SAMPLE_HEADER_LEN_DWORDS +  2) /* Index of sequence number to be replaced by master prior to transmission to a particular receiver. The sequence number comes from the internal header */
#define SFLOW_MSG_SAMPLE_COUNTER_IDX_SOURCE_ID     (SFLOW_MSG_SAMPLE_HEADER_LEN_DWORDS +  3) /* Required by master to substitute local switch port number with an USID-based. */
#define SFLOW_MSG_SAMPLE_COUNTER_IDX_IF_IDX        (SFLOW_MSG_SAMPLE_HEADER_LEN_DWORDS +  7) /* Required by master to substitute local switch port number with an USID-based. */
#define SFLOW_MSG_SAMPLE_COUNTER_IDX_LAST          (SFLOW_MSG_SAMPLE_HEADER_LEN_DWORDS + 28) /* Last DWORD of opaque buffer that receives data. */
#define SFLOW_MSG_SAMPLE_COUNTER_LEN_DWORDS        (SFLOW_MSG_SAMPLE_COUNTER_IDX_LAST  +  1) /* Length of total opaque sample including internal header */

#define SFLOW_MSG_SAMPLE_FLOW_IDX_FIRST            (SFLOW_MSG_SAMPLE_HEADER_LEN_DWORDS +  0)
#define SFLOW_MSG_SAMPLE_FLOW_IDX_SEQ_NUMBER       (SFLOW_MSG_SAMPLE_HEADER_LEN_DWORDS +  2) /* Index of sequence number to be replaced by master prior to transmission to a particular receiver. The sequence number comes from the internal header */
#define SFLOW_MSG_SAMPLE_FLOW_IDX_SOURCE_ID        (SFLOW_MSG_SAMPLE_HEADER_LEN_DWORDS +  3) /* Required by master to substitute local switch port number with an USID-based. */
#define SFLOW_MSG_SAMPLE_FLOW_IDX_SAMPLING_RATE    (SFLOW_MSG_SAMPLE_HEADER_LEN_DWORDS +  4) /* This is a per-instance property, so replaced by master */
#define SFLOW_MSG_SAMPLE_FLOW_IDX_SAMPLE_POOL      (SFLOW_MSG_SAMPLE_HEADER_LEN_DWORDS +  5) /* This is a per-instance property, carried in the header and inserted by master */
#define SFLOW_MSG_SAMPLE_FLOW_IDX_INPUT            (SFLOW_MSG_SAMPLE_HEADER_LEN_DWORDS +  7) /* Ditto. Input on slave is in host order, and may be VTSS_PORT_NO_NONE */
#define SFLOW_MSG_SAMPLE_FLOW_IDX_OUTPUT           (SFLOW_MSG_SAMPLE_HEADER_LEN_DWORDS +  8) /* Ditto. Input on slave is in host order, and may be VTSS_PORT_NO_NONE */
#define SFLOW_MSG_SAMPLE_FLOW_IDX_HEADER           (SFLOW_MSG_SAMPLE_HEADER_LEN_DWORDS + 16)
#define SFLOW_MSG_SAMPLE_FLOW_LEN_DWORDS(_dwords_) (SFLOW_MSG_SAMPLE_FLOW_IDX_HEADER + (_dwords_)) /* _dwords_ is the dword length of the raw header. Macro gives length of total opaque sample including internal header and variable-sized frame-header */

// Reasons for waking up SFLOW_thread().
#define SFLOW_THREAD_FLAG_CFG_CHANGE  0x01
#define SFLOW_THREAD_FLAG_TIMEOUT     0x02
#define SFLOW_THREAD_FLAG_SAMPLES     0x04
#define SFLOW_THREAD_FLAG_FLOW_SAMPLE 0x08
#define SFLOW_THREAD_FLAG_MASTER_UP   0x10

typedef enum {
    SFLOW_MSG_ID_CFG_TO_SLAVE,       // Configuration set request (no reply)
    SFLOW_MSG_ID_SAMPLES_TO_MASTER,  // Counter and flow samples from slave to master in almost correct datagram format (to offload master).
} sflow_msg_id_t;

// Message for configuration sent by master to slave.
typedef struct {
    sflow_msg_id_t     msg_id;     // Message ID
    sflow_switch_cfg_t switch_cfg; // Configuration that is local for a switch
} sflow_msg_local_switch_cfg_t;

// The maximum number of 32-bit words to transmit from slave to master in one single message.
#define SFLOW_MSG_SIZE_DWORDS_MAX 2500 /* 10 kbytes */

// The maximum number of bytes in the master BIP-buffer,
// which holds prepared counter and flow samples coming from slaves to be
// processed by master.
#define SFLOW_MASTER_BIP_BUFFER_SIZE_BYTES 10000

// The maximum number of bytes in the flow sample BIP-buffer,
// which holds flow samples on the local switch to be prepared
// by the local switch's SFLOW_thread.
#define SFLOW_LOCAL_BIP_BUFFER_SIZE_BYTES 10000

/******************************************************************************/
// sflow_msg_samples_t
// Layout of message sent from slave to master containing counter and flow
// samples.
// The #samples member is an opaque type holding one or more of either type of
// samples.
/******************************************************************************/
typedef struct {
    sflow_msg_id_t msg_id;

    // The total number of valid dwords in the #sample array.
    u32 dword_len;

    // Opaque type holding samples. Layout is explained
    // in the SFLOW_MSG_xxx_SAMPLE_IDX discussion above.
    u32 samples[SFLOW_MSG_SIZE_DWORDS_MAX];
} sflow_msg_samples_t;

/******************************************************************************/
// sflow_msg_meta_samples_t
// A structure for maintaining an opaque buffer meant for counter and flow
// samples sent from a slave to the master.
// This module will allocate two such buffers to swap amongst, so that
// one can be sent to the master through the message module while the other
// is being filled. If the buffers fill too fast, samples might be dropped.
/******************************************************************************/
typedef struct {
    // TRUE when #msg.samples[] buffer can't be used to fill in samples because
    // the buffer is currently being transmitted
    BOOL in_msg_tx_pipeline;
    // Number of DWORDS (free space) left in #msg.samples[].
    u32  dwords_left;
    // Index to next free index into #msg.samples[].
    u32  samples_idx;
    // The message holding samples.
    sflow_msg_samples_t msg;
} sflow_msg_meta_samples_t;


/**
 * Version to be conveyed through sFlow MIB.
 */
#define SFLOW_MIB_VERSION "1.3;Vitesse;"

/******************************************************************************/
// Global data
/******************************************************************************/
static cyg_handle_t              SFLOW_thread_handle;
static cyg_thread                SFLOW_thread_block;
static char                      SFLOW_thread_stack[THREAD_DEFAULT_STACK_SIZE];
static critd_t                   SFLOW_crit;                                    // sFlow data protection mutex.
static critd_t                   SFLOW_bip_crit;                                // Use one single separate mutex to protect both BIP buffers. Avoid using SFLOW_crit, since the BIP-buffer producers shouldn't be blocked.
static cyg_flag_t                SFLOW_wakeup_thread_flag;                      // Wake-up-thread-flag (SIC!).
static sflow_switch_cfg_t        SFLOW_local_new_cfg;                           // Local configuration to be applied ASAP.
static sflow_switch_cfg_t        SFLOW_stack_cfg[VTSS_ISID_CNT + 1];            // Index 0 is used for local configuration by the local switch. Index [1; VTSS_ISID_CNT] is used by the master.
static sflow_rcvr_t              SFLOW_rcvr_cfg[SFLOW_RECEIVER_CNT + 1];        // Index 0 is unused. Index [1; SFLOW_RECEIVER_CNT] contains receiver info.
static sflow_rcvr_state_t        SFLOW_rcvr_state[SFLOW_RECEIVER_CNT + 1];      // Index 0 is unused. Index [1; SFLOW_RECEIVER_CNT] contains socket handle and statistics.
static sflow_switch_state_t      SFLOW_local_state;
static sflow_msg_meta_samples_t  SFLOW_local_samples[2];                        // Two message buffers: One to fill while the other is being sent.
static u32                       SFLOW_local_fill_idx;                          // Index of current SFLOW_local_samples[] buffer to be filled.
static vtss_bip_buffer_t         SFLOW_master_bip;                              // Counter and flow samples from slaves are sent through a BIP-buffer on the master from the Message Rx thread to the SFLOW thread. This is in order to avoid the sFlow module taking up CPU resources when under heavy load.
static vtss_bip_buffer_t         SFLOW_local_bip;                               // Flow samples on the local switch are received on the packet Rx thread and sent through the BIP buffer for further processing by the local switch's sFlow thread. This is in order to avoid the sFlow module taking of CPU resources when under heavy load.
static sflow_info_exchange_t     SFLOW_local_info_exchange[VTSS_PORTS];         // Exchange of info between SFLOW_sample_rx() and SFLOW_thread(), protected by SFLOW_bip_crit.
static sflow_rcvr_statistics_t   SFLOW_rcvr_statistics[SFLOW_RECEIVER_CNT + 1]; // Index 0 is unused.
static sflow_switch_statistics_t SFLOW_stack_statistics[VTSS_ISID_CNT + 1];     // Index 0 is unused.
static sflow_agent_t             SFLOW_agent_cfg = {                            // Semi-read/only info about ourselves.
    .version            = (i8 *)SFLOW_MIB_VERSION,                                // Unchangeable
    .agent_ip_addr.type = SFLOW_AGENT_IP_TYPE_DEFAULT,                            // Default is loopback, i.e. 127.0.0.1, but can be changed from CLI/Web management.
    .agent_ip_addr.addr = {SFLOW_AGENT_IP_ADDR_DEFAULT},
};

#define SFLOW_ENCODE_32(_ptr32_, _val32_) do {*((u32 *)(_ptr32_)++) = htonl(_val32_);}  while (0)
#define SFLOW_ENCODE_64(_ptr32_, _val64_) do {*((u64 *)(_ptr32_))   = htonll(_val64_); (_ptr32_) += 2;} while (0)

/******************************************************************************/
//
// PRIVATE FUNCTIONS
//
/******************************************************************************/

/****************************************************************************/
// SFLOW_msg_tx_switch_cfg()
// Sends the current configuration to a slave.
// The configuration actually sent is gated with whether a certain receiver
// is having a valid socket file handle, so that unnecessary resources aren't
// spent on the slave if the receiver really doesn't want the datagrams.
// This allows the receiver to set-up all port instances prior to actually
// setting a valid receiver IP address.
/****************************************************************************/
static void SFLOW_msg_tx_switch_cfg(vtss_isid_t isid)
{
    sflow_msg_local_switch_cfg_t *msg;
    vtss_port_no_t               iport;
    int                          inst;
    BOOL                         rcvr_disabled[SFLOW_RECEIVER_CNT + 1];
    u32                          rcvr;

    SFLOW_CRIT_ASSERT_LOCKED();

    // Speed up the tests below.
    rcvr_disabled[0] = TRUE;
    for (rcvr = 1; rcvr <= SFLOW_RECEIVER_CNT; rcvr++) {
        rcvr_disabled[rcvr] = SFLOW_rcvr_state[rcvr].sockfd == -1 || SFLOW_rcvr_state[rcvr].timeout_left == 0;
    }

    msg = VTSS_MALLOC(sizeof(sflow_msg_local_switch_cfg_t));
    if (msg == NULL) {
        T_E("Allocation failed.\n");
        return;
    }

    msg->msg_id = SFLOW_MSG_ID_CFG_TO_SLAVE;
    msg->switch_cfg = SFLOW_stack_cfg[isid];

    for (iport = 0; iport < VTSS_PORTS; iport++) {
        sflow_port_cfg_t *cfg = &msg->switch_cfg.port[iport];
        for (inst = 0; inst < SFLOW_INSTANCE_CNT; inst++) {
            sflow_fs_t *fs = &cfg->fs[inst];
            sflow_cp_t *cp = &cfg->cp[inst];

            if (fs->enabled == FALSE || rcvr_disabled[fs->receiver]) {
                // Either this instance is disabled or a receiver is configured for it, but
                // there's no valid socket to send the datagrams to, so don't spend resources
                // on sampling frames and sending them to the master.
                cfg->fs[inst].sampling_rate = 0; // This is the way to disable sampling on local switch.
            }
            if (cp->enabled == FALSE || rcvr_disabled[cp->receiver]) {
                // Either this instance is disabled or a receiver is configured for it, but
                // there's no valid socket to send the datagrams to, so don't spend resources
                // on polling counters and sending them to the master.
                cfg->cp[inst].interval = 0; // This is the way to disable counter polling on local switch.
            }
        }
    }

    msg_tx(VTSS_MODULE_ID_SFLOW, isid, msg, sizeof(*msg));
}

/****************************************************************************/
// SFLOW_msg_rx()
/****************************************************************************/
static BOOL SFLOW_msg_rx(void *contxt, const void *const rx_msg, const size_t len, const vtss_module_id_t modid, const ulong isid)
{
    sflow_msg_id_t msg_id = *(sflow_msg_id_t *)rx_msg;

    switch (msg_id) {
    case SFLOW_MSG_ID_CFG_TO_SLAVE: {
        sflow_msg_local_switch_cfg_t *msg = (sflow_msg_local_switch_cfg_t *)rx_msg;

        // Just save the new config to a global variable and signal the thread.
        // The thread will in turn investigate changes and handle them appropriately.
        SFLOW_CRIT_ENTER();
        SFLOW_local_new_cfg = msg->switch_cfg;
        SFLOW_CRIT_EXIT();
        cyg_flag_setbits(&SFLOW_wakeup_thread_flag, SFLOW_THREAD_FLAG_CFG_CHANGE);
        break;
    }

    case SFLOW_MSG_ID_SAMPLES_TO_MASTER: {
        sflow_msg_samples_t *msg = (sflow_msg_samples_t *)rx_msg;
        u32 *buf;

        // Received message samples from a slave.
        // In order not to process them on the message Rx thread, put them in a
        // BIP-buffer and send them to the SFLOW_thread() for further processing.
        // The BIP-buffer is protected by a special mutex, so that the message rx thread
        // doesn't have to wait for our normal mutex, which may be taken for excess
        // periods of time due to the low prio of the SFLOW thread.
        SFLOW_BIP_CRIT_ENTER();
        // Get some space from the BIP-buffer. We allocate two more dwords than
        // required so that we can store a total length as the first word and
        // the originating isid as the second.
        if ((buf = (u32 *)vtss_bip_buffer_reserve(&SFLOW_master_bip, (msg->dword_len + 2) * sizeof(u32))) != NULL) {
            // Save the length in DWORDS first.
            buf[0] = msg->dword_len + 2;
            buf[1] = isid;
            // Then the data
            memcpy(&buf[2], msg->samples, msg->dword_len * sizeof(u32));

            // Tell it to the BIP buffer.
            vtss_bip_buffer_commit(&SFLOW_master_bip);
        }
        SFLOW_BIP_CRIT_EXIT();
        cyg_flag_setbits(&SFLOW_wakeup_thread_flag, SFLOW_THREAD_FLAG_SAMPLES);
        break;
    }

    default:
        T_W("Unknown message ID: %d", msg_id);
        break;
    }
    return TRUE;
}

/****************************************************************************/
// SFLOW_msg_init()
// Initializes the message protocol
/****************************************************************************/
static void SFLOW_msg_init(void)
{
    // Register for EEE messages
    msg_rx_filter_t filter;

    memset(&filter, 0, sizeof(filter));
    filter.cb = SFLOW_msg_rx;
    filter.modid = VTSS_MODULE_ID_SFLOW;
    if (msg_rx_filter_register(&filter) != VTSS_RC_OK) {
        T_E("Failed to register for sFlow messages");
    }
}

/******************************************************************************/
// SFLOW_sample_rx()
// Context: Packet Rx thread.
/******************************************************************************/
BOOL SFLOW_sample_rx(void *contxt, const uchar *const frm, const vtss_packet_rx_info_t *const rx_info)
{
    vtss_port_no_t iport = rx_info->sflow_port_no;
    u32            orig = rx_info->length + FCS_SIZE_BYTES;
    u32            copy, stripped, max_size, buf_len_dwords;
    u32            *buf;

    if (rx_info->glag_no != VTSS_GLAG_NO_NONE) {
        T_E("FIXME: SFLOW sample received on a GLAG (%u)", rx_info->glag_no);
        return FALSE;
    }

    SFLOW_ASSERT(iport < VTSS_PORTS, return FALSE;);

    T_I("Ingress port = %u, sflow_port = %u, sflow_type = %s",
        rx_info->port_no,
        rx_info->sflow_port_no,
        rx_info->sflow_type == VTSS_SFLOW_TYPE_NONE ? "None" :
        rx_info->sflow_type == VTSS_SFLOW_TYPE_RX   ? "Rx" :
        rx_info->sflow_type == VTSS_SFLOW_TYPE_TX   ? "Tx" :
        rx_info->sflow_type == VTSS_SFLOW_TYPE_ALL  ? "All" : "?");

    // This is the crit that protects the local BIP buffer and the info exchange array.
    SFLOW_BIP_CRIT_ENTER();

    if ((max_size = SFLOW_local_info_exchange[iport].max_header_size) == 0) {
        // Some frames were on their way to the CPU while the CPU disabled sFlow on this port.
        T_D("Received sflow-marked frame on sflow-disabled port (%u)", uport2iport(iport));
        goto do_exit;
    }

    // Number of bytes from the frame to copy to the BIP buffer. Exclude the FCS.
    copy = rx_info->length > max_size ? max_size : rx_info->length;

    // Add a possible stripped tag to the stripped size.
    stripped = FCS_SIZE_BYTES + rx_info->tag_type != VTSS_TAG_TYPE_UNTAGGED ? 4 : 0;

    // iport is the port on which the frame was sampled.
    // If it's a Tx sampled frame, then rx_info->port_no holds the ingress port.
    // On JR-48, this port number may be VTSS_PORT_NO_NONE if received on an interconnect
    // port, which isn't part of the port map and therefore doesn't have an ifIndex.

    // Get some space from the BIP-buffer. We allocate six more dwords than
    // required so that we can store the following fields in the beginning:
    //  * Number of dwords allocated in the BIP buffer for this flow sample, including this 6-dword header.
    //  * Size of the copied header in bytes.
    //  * Original length of frame in bytes.
    //  * Number of stripped bytes.
    //  * Port number on which the packet was sampled.
    //  * In case of a Tx frame, the ingress port number. In case of Rx, the same as the port on which it was sampled.
    buf_len_dwords = copy + 6 * sizeof(u32); // Now in bytes
    buf_len_dwords = (buf_len_dwords + (sizeof(u32) - 1)) / sizeof(u32); // Now in dwords
    if ((buf = (u32 *)vtss_bip_buffer_reserve(&SFLOW_local_bip, buf_len_dwords * sizeof(u32))) != NULL) {
        buf[0] = buf_len_dwords;
        buf[1] = copy;
        buf[2] = orig;
        buf[3] = stripped;
        buf[4] = iport;
        buf[5] = rx_info->port_no;

        // Zero-out the last dword before copying.
        // This makes it easier for the datagram packaging code.
        buf[6 + (copy + (sizeof(u32) - 1)) / sizeof(u32) - 1] = 0;

        // Then copy the frame data
        memcpy(&buf[6], frm, copy);

        // Tell it to the BIP buffer.
        vtss_bip_buffer_commit(&SFLOW_local_bip);

        // Wake up the sFlow thread.
        cyg_flag_setbits(&SFLOW_wakeup_thread_flag, SFLOW_THREAD_FLAG_FLOW_SAMPLE);
    } else {
        // Tell that a frame was dropped due to lack of BIP buffer space.
        T_D("Unable to get BIP buffer room for sample frame");
        SFLOW_local_info_exchange[iport].drops++;
    }

do_exit:
    SFLOW_BIP_CRIT_EXIT();

    // Let other modules taste the frame. The packet module takes care of not
    // forwarding if only received on sFlow queue.
    return FALSE;
}

/******************************************************************************/
// SFLOW_packet_register()
/******************************************************************************/
static void SFLOW_packet_register(void)
{
    packet_rx_filter_t rx_filter;
    void *SFLOW_filter_id;

    memset(&rx_filter, 0, sizeof(rx_filter));
    rx_filter.modid           = VTSS_MODULE_ID_SFLOW;
    rx_filter.match           = PACKET_RX_FILTER_MATCH_SFLOW;
    rx_filter.cb              = SFLOW_sample_rx;

    rx_filter.contxt          = NULL;
    rx_filter.prio            = PACKET_RX_FILTER_PRIO_NORMAL;

    if (packet_rx_filter_register(&rx_filter, &SFLOW_filter_id) != VTSS_RC_OK) {
        T_E("Unable to register with Packet module");
    }
}

/******************************************************************************/
// SFLOW_check_isid_port()
/******************************************************************************/
static vtss_rc SFLOW_check_isid_port(vtss_isid_t isid, vtss_port_no_t port, BOOL allow_local, BOOL check_port)
{
    if ((isid != VTSS_ISID_LOCAL && !VTSS_ISID_LEGAL(isid)) || (isid == VTSS_ISID_LOCAL && !allow_local)) {
        return SFLOW_ERROR_ISID;
    }
    if (isid != VTSS_ISID_LOCAL && !msg_switch_is_master()) {
        return SFLOW_ERROR_NOT_MASTER;
    }
    if (check_port && (port >= port_isid_port_count(isid) || port_isid_port_no_is_stack(isid, port))) {
        return SFLOW_ERROR_PORT;
    }
    return VTSS_OK;
}

/******************************************************************************/
// SFLOW_sockaddr_to_string()
// Convert a struct sockaddr address to a string
/******************************************************************************/
static SFLOW_INLINE char *SFLOW_sockaddr_to_string(const struct sockaddr *sa, char *s, size_t maxlen)
{
    switch (sa->sa_family) {
    case AF_INET:
        (void)inet_ntop(AF_INET, (char *) & (((struct sockaddr_in *)sa)->sin_addr), s, maxlen);
        break;
#ifdef VTSS_SW_OPTION_IPV6
    case AF_INET6:
        (void)inet_ntop(AF_INET6, (char *) & (((struct sockaddr_in6 *)sa)->sin6_addr), s, maxlen);
        break;
#endif
    default:
        strncpy(s, "Unknown Address Family", maxlen);
        return NULL;
    }
    return s;
}

/******************************************************************************/
// SFLOW_hostname_lookup()
// Lookup the hostname (or IP address) and return the addrinfo in addr
// Returns the same values as getaddrinfo()
// Remember to deallocate the addrinfo with freeaddrinfo();
/******************************************************************************/
static SFLOW_INLINE int SFLOW_hostname_lookup(const u8 *hostname, u16 udp_port, struct addrinfo **addr)
{
    struct addrinfo  hints;
    char             udp_port_str[16];
    int              result;

    SFLOW_ASSERT(hostname);
    SFLOW_ASSERT(addr);

    // Setup hints structure
    memset(&hints, 0, sizeof(hints));
#ifdef VTSS_SW_OPTION_IPV6
    hints.ai_family = AF_UNSPEC; // Accept both IPv4 and IPv6
#else
    hints.ai_family = AF_INET;   // Accept IPv4 only
#endif
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    sprintf(udp_port_str, "%d", udp_port);
    if ((result = getaddrinfo((char *)hostname, udp_port_str, &hints, addr)) == 0) {
        if (addr != NULL && *addr != NULL && (*addr)->ai_family != AF_INET) {
            // Cater for the case where ai_family returns AF_UNSPEC (this is possible
            // according to charlesl, see Bugzilla#7791).
            (*addr)->ai_family = AF_INET6;
        }
    }

    return result;
}

/****************************************************************************/
// SFLOW_dgram_header_update()
// Returns FALSE if IPv4 agent IP, TRUE if IPv6.
/****************************************************************************/
static BOOL SFLOW_dgram_header_update(u32 rcvr_idx)
{
    sflow_rcvr_state_t *rcvr_state = &SFLOW_rcvr_state[rcvr_idx];
    sflow_rcvr_t       *rcvr_cfg   = &SFLOW_rcvr_cfg[rcvr_idx];
    u32                *ptr;
    u64                uptime_ms;
    BOOL               is_ipv6;

    // Figure out where the dgram starts. It is 3 * 32 bits longer when it's IPv6 compared to IPv4.
    is_ipv6 = SFLOW_agent_cfg.agent_ip_addr.type != VTSS_IP_TYPE_IPV4;
    ptr = is_ipv6 ? &rcvr_state->dgram[0] : &rcvr_state->dgram[3];

    // Datagram version
    SFLOW_ENCODE_32(ptr, rcvr_cfg->datagram_version);

    // IP version (1 = IPv4, 2 = IPv6)
    SFLOW_ENCODE_32(ptr, SFLOW_agent_cfg.agent_ip_addr.type);

    // Configured agent IP address.
    if (!is_ipv6) {
        SFLOW_ENCODE_32(ptr, SFLOW_agent_cfg.agent_ip_addr.addr.ipv4);
    } else {
#ifdef VTSS_SW_OPTION_IPV6
        memcpy(ptr, SFLOW_agent_cfg.agent_ip_addr.addr.ipv6.addr, 16);
#else
        // Unreachable, because user cannot configure IPv6 if not included in build.
        memset(ptr, 0, 16);
#endif
        ptr += 4;
    }

    // Sub-agent ID. Always 0.
    SFLOW_ENCODE_32(ptr, 0);

    // Datagram sequence number (apparently 1-based)
    SFLOW_ENCODE_32(ptr, ++rcvr_state->sequence_number);

    // Uptime in milliseconds
    uptime_ms = cyg_current_time() * ECOS_MSECS_PER_HWTICK;
    SFLOW_ENCODE_32(ptr, (u32)uptime_ms); // Truncate.

    // Number of samples in datagram.
    SFLOW_ENCODE_32(ptr, (u32)rcvr_state->sample_cnt);

    return is_ipv6;
}

/****************************************************************************/
// SFLOW_dgram_init()
// Resets receiver state's datagram meta data and reserves room for an
// appropriately sized header.
/****************************************************************************/
static void SFLOW_dgram_init(sflow_rcvr_state_t *rcvr_state)
{
    rcvr_state->sample_cnt = 0;
    rcvr_state->dgram_len_dwords = 10; // Always make room for an IPv6 address.
}

/******************************************************************************/
// SFLOW_rcvr_state_reset()
/******************************************************************************/
static void SFLOW_rcvr_state_reset(u32 rcvr_idx, BOOL close_connection)
{
    u32 rcvr_min, rcvr_max;

    if (rcvr_idx == 0) {
        rcvr_min = 1;
        rcvr_max = SFLOW_RECEIVER_CNT;
    } else {
        rcvr_min = rcvr_max = rcvr_idx;
    }

    for (rcvr_idx = rcvr_min; rcvr_idx <= rcvr_max; rcvr_idx++) {
        sflow_rcvr_state_t *rcvr_state = &SFLOW_rcvr_state[rcvr_idx];
        if (rcvr_state->sockfd != -1 && close_connection) {
            close(rcvr_state->sockfd);
        }
        if (rcvr_state->addrinfo != NULL) {
            freeaddrinfo(rcvr_state->addrinfo);
        }
        rcvr_state->addrinfo         = NULL;
        rcvr_state->sockfd           = -1;
        strcpy(rcvr_state->ip_addr_str, "0.0.0.0");
        rcvr_state->sequence_number  = 0;
        rcvr_state->timeout_left     = 0;
        memset(&SFLOW_rcvr_statistics[rcvr_idx], 0, sizeof(SFLOW_rcvr_statistics[rcvr_idx]));
        SFLOW_dgram_init(rcvr_state);
    }
}

/******************************************************************************/
// SFLOW_ip_addr_from_sockaddr()
// Returns a VTSS IP address type from a struct sockaddr
/******************************************************************************/
static void SFLOW_ip_addr_from_sockaddr(vtss_ip_addr_t *ip, struct sockaddr *sa)
{
    switch (sa->sa_family) {
    case AF_INET:
        ip->type = VTSS_IP_TYPE_IPV4;
        ip->addr.ipv4 = ntohl(((struct sockaddr_in *)sa)->sin_addr.s_addr);
        break;

    case AF_INET6:
        ip->type = VTSS_IP_TYPE_IPV6;
        memcpy(&ip->addr.ipv6, &((struct sockaddr_in6 *)sa)->sin6_addr, sizeof(ip->addr.ipv6));
        break;

    default:
        ip->type = VTSS_IP_TYPE_NONE;
        break;
    }
}

/******************************************************************************/
// SFLOW_rcvr_hostname_update()
/******************************************************************************/
static SFLOW_INLINE vtss_rc SFLOW_rcvr_hostname_update(u32 rcvr_idx, sflow_rcvr_t *cur_cfg, sflow_rcvr_t *new_cfg, BOOL *send_cfg_to_slaves)
{
    int                error, new_sockfd = -1;
    struct addrinfo    *new_addrinfo = NULL;
    vtss_ip_addr_t     ip_addr;
    sflow_rcvr_state_t *rcvr_state = &SFLOW_rcvr_state[rcvr_idx];
    BOOL               is_null = FALSE;

    *send_cfg_to_slaves = FALSE;

    if (strcmp((char *)cur_cfg->hostname, (char *)new_cfg->hostname) != 0 || cur_cfg->udp_port != new_cfg->udp_port) {
        if (new_cfg->hostname[0] == '\0') {
            // No longer (or have never been) a receiver.
            // Free whatever socket resources we have.
            SFLOW_rcvr_state_reset(rcvr_idx, TRUE);

            // We don't reset any flow sampler or counter poller
            // configuration, since it might be on purpose
            // that the receiver waits with assigning a valid
            // hostname/IP address until rest is configured.
            // The configuration we send to the slaves is
            // not the actual configuration, but guarded with
            // the fact that there is no configured receiver.
            *send_cfg_to_slaves = TRUE;
            return VTSS_RC_OK;
        }

        // Check to see if the hostname can be looked up.
        if ((error = SFLOW_hostname_lookup(new_cfg->hostname, new_cfg->udp_port, &new_addrinfo)) != 0) {
            T_W("getaddrinfo(%s), UDP port %d failed (%d, %s)", new_cfg->hostname, new_cfg->udp_port, error, gai_strerror(error));
            return SFLOW_ERROR_RECEIVER_HOSTNAME;
        }

        SFLOW_ASSERT(new_addrinfo, return SFLOW_ERROR_RECEIVER_HOSTNAME;);

        // Check to see if this is a NULL-IP. If so, we don't open a socket
        SFLOW_ip_addr_from_sockaddr(&ip_addr, new_addrinfo->ai_addr);
        if (vtss_ip_addr_is_zero(&ip_addr)) {
            freeaddrinfo(new_addrinfo);
            new_addrinfo = NULL;
            is_null = TRUE;
        }

        if (!is_null) {
            SFLOW_ASSERT(new_addrinfo, return SFLOW_ERROR_RECEIVER_HOSTNAME;);

            // If the destination is an IPv6 link-local address, we need to modify the address
            // so that subsequent sendto() calls don't attempt to send it to the loopback interface.
            // (see Bugzilla#7791).
            if (new_addrinfo->ai_family == AF_INET6 && IN6_IS_ADDR_LINKLOCAL(&((struct sockaddr_in6 *)new_addrinfo->ai_addr)->sin6_addr)) {
                ((struct sockaddr_in6 *)new_addrinfo->ai_addr)->sin6_addr.__u6_addr.__u6_addr16[1] |= htons(0x2);
            }

            // Create a new compatible socket.
            new_sockfd = socket(new_addrinfo->ai_family, new_addrinfo->ai_socktype, new_addrinfo->ai_protocol);
            if (new_sockfd == -1) {
                // Keep the old socket (if any).
                T_W("Socket creation failed (%d, %s).", errno, strerror(errno));
                freeaddrinfo(new_addrinfo);
                return SFLOW_ERROR_RECEIVER_SOCKET_CREATE;
            }
        }

        // Send new configuration to slaves either because the slaves need to
        // start or stop sampling, or because we need the slaves to reset their
        // sequence numbers.
        *send_cfg_to_slaves = TRUE;

        // Close old socket (if any).
        SFLOW_rcvr_state_reset(rcvr_idx, TRUE);
        rcvr_state->sockfd   = new_sockfd;
        rcvr_state->addrinfo = new_addrinfo;

        // Prepare datagram for updates.
        SFLOW_dgram_init(rcvr_state);

        if (new_addrinfo) {
            (void)SFLOW_sockaddr_to_string(new_addrinfo->ai_addr, rcvr_state->ip_addr_str, sizeof(rcvr_state->ip_addr_str));
        }
    }

    return VTSS_RC_OK;
}

/******************************************************************************/
// SFLOW_rcvr_cfg_init()
/******************************************************************************/
static void SFLOW_rcvr_cfg_init(u32 rcvr_idx)
{
    u32 rcvr_min, rcvr_max;

    if (rcvr_idx == 0) {
        rcvr_min = 1;
        rcvr_max = SFLOW_RECEIVER_CNT;
    } else {
        rcvr_min = rcvr_max = rcvr_idx;
    }

    for (rcvr_idx = rcvr_min; rcvr_idx <= rcvr_max; rcvr_idx++) {
        sflow_rcvr_t *rcvr      = &SFLOW_rcvr_cfg[rcvr_idx];
        rcvr->owner[0]          = '\0';
        rcvr->timeout           = 0;
        rcvr->max_datagram_size = SFLOW_RECEIVER_DATAGRAM_SIZE_DEFAULT;
        strcpy((char *)rcvr->hostname, "0.0.0.0");
        rcvr->udp_port          = SFLOW_RECEIVER_UDP_PORT_DEFAULT;
        rcvr->datagram_version  = SFLOW_DATAGRAM_VERSION;
    }
}

/******************************************************************************/
// SFLOW_fs_cfg_init()
/******************************************************************************/
static void SFLOW_fs_cfg_init(vtss_isid_t isid, vtss_port_no_t port, u16 instance)
{
    vtss_isid_t    isid_min, isid_max, isid_iter;
    vtss_port_no_t port_min, port_max, port_iter;
    u16            inst_min, inst_max, inst_iter;

    if (isid == VTSS_ISID_GLOBAL) {
        isid_min = VTSS_ISID_START;
        isid_max = VTSS_ISID_END - VTSS_ISID_START;
    } else {
        isid_min = isid_max = isid;
    }

    if (port == VTSS_PORT_NO_NONE) {
        port_min = 0;
        port_max = VTSS_PORTS - 1;
    } else {
        port_min = port_max = port;
    }

    if (instance == 0) {
        inst_min = 1;
        inst_max = SFLOW_INSTANCE_CNT;
    } else {
        inst_min = inst_max = instance;
    }

    for (isid_iter = isid_min; isid_iter <= isid_max; isid_iter++) {
        for (port_iter = port_min; port_iter <= port_max; port_iter++) {
            for (inst_iter = inst_min; inst_iter <= inst_max; inst_iter++) {
                SFLOW_stack_cfg[isid_iter].port[port_iter].fs[inst_iter - 1].receiver        = 0;
                SFLOW_stack_cfg[isid_iter].port[port_iter].fs[inst_iter - 1].sampling_rate   = SFLOW_FLOW_SAMPLING_RATE_DEFAULT;
                SFLOW_stack_cfg[isid_iter].port[port_iter].fs[inst_iter - 1].max_header_size = SFLOW_FLOW_HEADER_SIZE_DEFAULT;
                SFLOW_stack_cfg[isid_iter].port[port_iter].fs[inst_iter - 1].type            = VTSS_SFLOW_TYPE_TX;
            }
        }
    }
}

/******************************************************************************/
// SFLOW_release_receiver()
/******************************************************************************/
static void SFLOW_release_receiver(u32 rcvr_idx)
{
    vtss_isid_t    isid;
    vtss_port_no_t port;
    int            inst;
    u32            rcvr_idx_min, rcvr_idx_max;

    if (rcvr_idx == 0) {
        rcvr_idx_min = 1;
        rcvr_idx_max = SFLOW_RECEIVER_CNT;
    } else {
        rcvr_idx_min = rcvr_idx_max = rcvr_idx;
    }

    SFLOW_CRIT_ASSERT_LOCKED();

    for (rcvr_idx = rcvr_idx_min; rcvr_idx <= rcvr_idx_max; rcvr_idx++) {
        // Release all resources owned by this receiver.
        for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
            BOOL switch_cfg_changed = FALSE;
            for (port = 0; port < VTSS_PORTS; port++) {
                for (inst = 0; inst < SFLOW_INSTANCE_CNT; inst++) {
                    sflow_fs_t *fs = &SFLOW_stack_cfg[isid].port[port].fs[inst];
                    sflow_cp_t *cp = &SFLOW_stack_cfg[isid].port[port].cp[inst];
                    if (fs->receiver == rcvr_idx && fs->enabled) {
                        // Simply disable this entry. Don't initialize any other configuration
                        // fields than #enable. The SNMP sFlow agent configuration must return
                        // receiver == 0 if #enable == FALSE.
                        // The only problem with this is that the max_header_size and type
                        // fields won't be reset to defaults. I think we can live with that.
                        fs->enabled = FALSE;
                        SFLOW_stack_statistics[isid].port[port].fs_rx[inst] = 0;
                        SFLOW_stack_statistics[isid].port[port].fs_tx[inst] = 0;
                        switch_cfg_changed = TRUE;
                    }
                    if (cp->receiver == rcvr_idx && cp->enabled) {
                        // Simply disable this entry. Don't initialize any other configuration
                        // fields than #enable. The SNMP sFlow agent configuration must return
                        // receiver == 0 if #enable == FALSE.
                        cp->enabled = FALSE;
                        SFLOW_stack_statistics[isid].port[port].cp[inst]   = 0;
                        switch_cfg_changed = TRUE;
                    }
                }
            }
            if (switch_cfg_changed && msg_switch_exists(isid)) {
                // Tell the slave about the new configuration.
                SFLOW_msg_tx_switch_cfg(isid);
            }
        }

        // Close any sockets that may be open
        SFLOW_rcvr_state_reset(rcvr_idx, TRUE);

        // Configuration back to defaults
        SFLOW_rcvr_cfg_init(rcvr_idx);
    }
}

/****************************************************************************/
// SFLOW_cp_find_sync_seconds_left()
// In order to sync-up counter polling, we search for other counter instances
// running the same poll interval.
/****************************************************************************/
static SFLOW_INLINE u32 SFLOW_cp_find_sync_seconds_left(vtss_port_no_t match_port, int match_inst, u32 match_interval)
{
    port_iter_t pit;
    int         inst;

    if (match_interval < 2 ) {
        // No need to run through all instances if disabled, or update is every second.
        return match_interval;
    }

    // Loop through all instances on all ports and see if there are any other
    // counter pollers that use the same interval. If so, return that instance's
    // time-left.
    (void)port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
    while (port_iter_getnext(&pit)) {
        for (inst = 0; inst < SFLOW_INSTANCE_CNT; inst++) {
            if (SFLOW_stack_cfg[VTSS_ISID_LOCAL].port[pit.iport].cp[inst].interval == match_interval) {
                // Don't match against ourselves.
                if (pit.iport != match_port || inst != match_inst) {
                    T_D("Synchronizing %u[%d] to %u[%d]", iport2uport(match_port), match_inst + 1, iport2uport(pit.iport), inst + 1);
                    return SFLOW_local_state.port[pit.iport].cp_seconds_left[inst];
                }
            }
        }
    }

    // No other instance matched.
    return match_interval;
}

/****************************************************************************/
// SFLOW_fs_state_update()
// Updates the local switch state based on differences between current and
// new configuration.
/****************************************************************************/
static SFLOW_INLINE void SFLOW_fs_state_update(void)
{
    port_iter_t pit;
    int         inst;

    SFLOW_CRIT_ASSERT_LOCKED();

    (void)port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
    while (port_iter_getnext(&pit)) {
        sflow_port_state_t     *port_state   = &SFLOW_local_state.port[pit.iport];
        vtss_sflow_port_conf_t *old_api_conf = &port_state->fs_api_conf;
        vtss_sflow_port_conf_t new_api_conf;
        vtss_port_counters_t   port_counters;
        BOOL                   get_port_counters = FALSE;

        memset(&port_counters, 0, sizeof(port_counters));
        memset(&new_api_conf, 0, sizeof(new_api_conf));
        u32 max_header_size = 0; // Compute the largest header size required by any instance.

        // Find new fastest sampling rate, which is required when updating "samples left" in the
        // next loop. This is also the sampling rate that will get applied to the API.
        // It is already taken care of that if SFLOW_INSTANCE_CNT > 1, all sampling rates are
        // powers of two.
        for (inst = 0; inst < SFLOW_INSTANCE_CNT; inst++) {
            sflow_fs_t *cur_fs = &SFLOW_stack_cfg[VTSS_ISID_LOCAL].port[pit.iport].fs[inst];
            sflow_fs_t *new_fs = &SFLOW_local_new_cfg.port[pit.iport].fs[inst];

            if (new_fs->sampling_rate != 0 && (new_api_conf.sampling_rate == 0 || new_api_conf.sampling_rate > new_fs->sampling_rate)) {
                new_api_conf.sampling_rate = new_fs->sampling_rate;
            }
            if (new_fs->sampling_rate != 0 && new_fs->sampling_rate != cur_fs->sampling_rate) {
                get_port_counters = TRUE;
            }
        }

        // We also need port packet counters if sampling has changed for this port.
        if (get_port_counters) {
            if (port_mgmt_counters_get(VTSS_ISID_LOCAL, pit.iport, &port_counters) != VTSS_RC_OK) {
                T_E("Unable to obtain local port counters for uport=%u", pit.uport);
            }
        }

        for (inst = 0; inst < SFLOW_INSTANCE_CNT; inst++) {
            sflow_fs_t *cur_fs = &SFLOW_stack_cfg[VTSS_ISID_LOCAL].port[pit.iport].fs[inst];
            sflow_fs_t *new_fs = &SFLOW_local_new_cfg.port[pit.iport].fs[inst];

            // Here, we don't care about the receiver. The master has set sampling rate to 0
            // if we're not supposed to sample. #sampling_rate is the only enable/disable parameter for
            // the flow sampler. We need, however, to clear counters and sequence numbers
            // if any of the other parameters have changed.
            if (cur_fs->receiver != new_fs->receiver || cur_fs->type != new_fs->type || cur_fs->sampling_rate != new_fs->sampling_rate) {
                // When sampling rate is non-zero, so must the sample type be (i.e. sample Rx, Tx, or both directions).
                SFLOW_ASSERT(new_fs->sampling_rate == 0 || new_fs->type != VTSS_SFLOW_TYPE_NONE);
                // When sampling rate is non-zero, so must the maximum header size be.
                SFLOW_ASSERT(new_fs->sampling_rate == 0 || new_fs->max_header_size != 0);

                // Reset sequence number.
                port_state->fs_sequence_number[inst] = 0;

                // And get the port rx + tx packet counter base.
                port_state->fs_start_rx_packets[inst] = port_counters.rmon.rx_etherStatsPkts;
                port_state->fs_start_tx_packets[inst] = port_counters.rmon.tx_etherStatsPkts;

                // Update the number of samples to skip before sending one for this flow sampler instance.
                port_state->fs_samples_left[inst] = new_api_conf.sampling_rate == 0 ? 0 : new_fs->sampling_rate / new_api_conf.sampling_rate;
            } else if (new_fs->sampling_rate != 0 && new_api_conf.sampling_rate != 0) {
                SFLOW_ASSERT(new_api_conf.sampling_rate != 0, return;);
                if (new_api_conf.sampling_rate != old_api_conf->sampling_rate) {
                    // This instance has not changed configuration, but the base sampling rate is about to change to either something faster
                    // or something slower.
                    // If changing to something faster (because an instance with a higher sampling rate was made active),
                    // we can compute an accurate number of samples to skip, based on the current samples-left counter.
                    // If changing to something slower, we might get rounding errors. Changing to something slower
                    // can occur if you started out with e.g. three sampling rates. e.g. 1/64, 1/8 and 1/2.
                    // If the third goes away, the sampling rate will change from 2 to 8, and we will no longer
                    // be able to provide a 100% accurate sample rate for the remaining instances (only the first next sample
                    // will be inaccurate).
                    port_state->fs_samples_left[inst] *= old_api_conf->sampling_rate;
                    port_state->fs_samples_left[inst]  = MAX(port_state->fs_samples_left[inst] / new_api_conf.sampling_rate, 1);
                }
            } else {
                port_state->fs_samples_left[inst] = 0;
            }

            if (new_fs->sampling_rate != 0) {
                // The maximum header size must be the largest of the requested maximum header sizes.
                if (new_fs->max_header_size > max_header_size) {
                    max_header_size = new_fs->max_header_size;
                }
                // It is not possible to support instances on the same port that require different kinds of sampling types.
                // We select the highest enum value (which happens to be: ALL takes precedence over Tx, which takes precendence over Rx, which takes precedence over none).
                if (new_fs->type > new_api_conf.type) {
                    new_api_conf.type = new_fs->type;
                }
            }
            *cur_fs = *new_fs; // Update cfg, which also includes any changes to header size, which doesn't affect the API.
        }

        if (new_api_conf.sampling_rate == 0) {
            // Default to VTSS_SFLOW_TYPE_TX (to be able to specify both an input and an output interface in PDUs to receiver, so that it can compute the ingress and egress rates correctly).
            new_api_conf.type = VTSS_SFLOW_TYPE_TX;
        }

        // Pass the maximum header size to SFLOW_sample_rx(). The SFLOW_bip_crit mutex protects the
        // SFLOW_local_info_exchange[] array, and we can safely acquire that mutex here (even though
        // we have the SFLOW_crit already), because other threads that acquire SFLOW_bip_crit don't
        // have the SFLOW_crit and will not take it.
        SFLOW_BIP_CRIT_ENTER();
        SFLOW_local_info_exchange[pit.iport].max_header_size = new_api_conf.sampling_rate != 0 ? max_header_size : 0;
        SFLOW_BIP_CRIT_EXIT();

        // Check to see if API update is necessary, and if so, also clear the
        // Rx and Tx drop counters that are normally updated by SFLOW_sample_rx().
        if (memcmp(&new_api_conf, old_api_conf, sizeof(new_api_conf)) != 0) {
            if (vtss_sflow_port_conf_set(NULL, pit.iport, &new_api_conf) != VTSS_RC_OK) {
                T_E("vtss_sflow_port_conf_set(uport=%u) failed", pit.uport);
            } else {
                *old_api_conf = new_api_conf;
            }
        }
    }
}

/****************************************************************************/
// SFLOW_cp_state_update()
// Updates the local switch state based on differences beetween current and
// new configuration.
// Returns TRUE if at least one counter poller is active. FALSE otherwise.
/****************************************************************************/
static SFLOW_INLINE BOOL SFLOW_cp_state_update(void)
{
    port_iter_t pit;
    int         inst;
    BOOL        result = FALSE;

    SFLOW_CRIT_ASSERT_LOCKED();

    (void)port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
    while (port_iter_getnext(&pit)) {
        SFLOW_local_state.port[pit.iport].cp_active = FALSE;
        for (inst = 0; inst < SFLOW_INSTANCE_CNT; inst++) {
            sflow_cp_t *cur_cp = &SFLOW_stack_cfg[VTSS_ISID_LOCAL].port[pit.iport].cp[inst];
            sflow_cp_t *new_cp = &SFLOW_local_new_cfg.port[pit.iport].cp[inst];

            // Here, we don't care about the receiver. The master has set interval to 0
            // if we're not supposed to poll counters. #interval is the only enable/disable parameter for
            // the counter poller. We need, however, to clear counters and sequence numbers
            // if the receiver has changed.
            if (cur_cp->receiver != new_cp->receiver || cur_cp->interval != new_cp->interval) {
                *cur_cp = *new_cp; // Update cfg.

                if (new_cp->interval != 0) {
                    // This port is active.
                    SFLOW_local_state.port[pit.iport].cp_active = TRUE;
                    SFLOW_local_state.port[pit.iport].cp_sequence_number[inst] = 0;
                    result = TRUE;
                }

                // Attempt to synchronize with other instances on any port. Synchronization is possible if
                // they have the same interval. The standard specifies that it must take at most #interval
                // seconds between updates. The first update for this port instance might come a bit earlier then.
                // The idea behind synchronizing is to avoid sending too many small messages from the slave
                // to the master. The drawback is that synchronized counters updates will cause bursty CPU load.
                SFLOW_local_state.port[pit.iport].cp_seconds_left[inst] = SFLOW_cp_find_sync_seconds_left(pit.iport, inst, new_cp->interval);

            } else if (cur_cp->interval) {
                // This port was already active prior to the configuration change.
                SFLOW_local_state.port[pit.iport].cp_active = TRUE;
                result = TRUE;
            }
        }
    }

    return result;
}

/****************************************************************************/
// SFLOW_msg_reset_sample_msg()
// Clears the meta-info in the message containing flow or counter samples to
// be sent to current master.
/****************************************************************************/
static void SFLOW_msg_reset_sample_msg(sflow_msg_meta_samples_t *meta)
{
    meta->in_msg_tx_pipeline = FALSE;
    meta->dwords_left        = SFLOW_MSG_SIZE_DWORDS_MAX;
    meta->samples_idx        = 0;
}

/****************************************************************************/
// SFLOW_msg_tx_done()
// Called whenever one of our two sample buffers is transmitted to the
// current master. We free it.
/****************************************************************************/
static void SFLOW_msg_tx_done(void *context, void *msg, msg_tx_rc_t rc)
{
    sflow_msg_meta_samples_t *meta;
    u32 idx = (u32)context;
    SFLOW_CRIT_ENTER();
    SFLOW_ASSERT(idx == 0 || idx == 1);
    meta = &SFLOW_local_samples[idx];
    SFLOW_ASSERT(meta->in_msg_tx_pipeline);
    SFLOW_msg_reset_sample_msg(meta);
    SFLOW_CRIT_EXIT();
}

/****************************************************************************/
// SFLOW_msg_flush()
/****************************************************************************/
static void SFLOW_msg_flush(void)
{
    sflow_msg_meta_samples_t *meta = &SFLOW_local_samples[SFLOW_local_fill_idx];

    SFLOW_CRIT_ASSERT_LOCKED();

    if (meta->in_msg_tx_pipeline) {
        // The next to be sent to the master is already being sent.
        return;
    }

    if (meta->samples_idx == 0) {
        // Nothing in the buffer yet.
        return;
    }

    meta->msg.msg_id         = SFLOW_MSG_ID_SAMPLES_TO_MASTER;
    meta->msg.dword_len      = meta->samples_idx;
    meta->in_msg_tx_pipeline = TRUE;

    // Transmit the message to the current master.
    // Tell the message module not to free it upon tx done (MSG_TX_OPT_DONT_FREE),
    // and that we don't want the message module to allocate a new buffer if this local switch is
    // currently master. The callback order will be msg_rx, msg_tx_done.
    msg_tx_adv((void *)SFLOW_local_fill_idx, SFLOW_msg_tx_done, MSG_TX_OPT_DONT_FREE | MSG_TX_OPT_NO_ALLOC_ON_LOOPBACK, VTSS_MODULE_ID_SFLOW, 0, &meta->msg, offsetof(sflow_msg_samples_t, samples[0]) + 4 * meta->msg.dword_len);

    // Next buffer that will become free is the opposite.
    SFLOW_local_fill_idx = SFLOW_local_fill_idx == 1 ? 0 : 1;
}

/****************************************************************************/
// SFLOW_msg_sample_buffer_get()
// Returns a buffer guaranteed to hold len_dwords 32-bit words of data.
// It also updates the used length of the buffer if successfully getting one,
// under the assumption that the caller indeed will fill in len_dwords.
// If no such buffer is found, NULL is returned.
/****************************************************************************/
static u32 *SFLOW_msg_sample_buffer_get(u32 len_dwords)
{
    u32 i;

    SFLOW_CRIT_ASSERT_LOCKED();

    for (i = 0; i < ARRSZ(SFLOW_local_samples); i++) {
        sflow_msg_meta_samples_t *meta = &SFLOW_local_samples[SFLOW_local_fill_idx];
        if (meta->in_msg_tx_pipeline) {
            // Both this and the alternate buffer are currently being sent to the master.
            // SFLOW_local_fill_idx points to the buffer that is going to be sent first,
            // and thereby freed first.
            return NULL;
        }

        if (meta->dwords_left < len_dwords) {
            // Not room in this buffer for the sample.
            // Ship it off, and try the alternate.
            // The flush function will swap SFLOW_local_fill_idx.
            SFLOW_msg_flush();
        } else {
            u32 *result = &meta->msg.samples[meta->samples_idx];
            // Update the length fields of the meta object,
            // assuming that the caller indeed will fill in
            // something useful.
            meta->dwords_left -= len_dwords;
            meta->samples_idx += len_dwords;
            return result;
        }
    }

    return NULL;
}

/****************************************************************************/
// SFLOW_msg_fs_collect()
// Collects flow sample in a message buffer supposed to be sent to the master
// when it's full or triggered.
// Returns TRUE if successfully stored in buffer. FALSE if out of space.
/****************************************************************************/
static SFLOW_INLINE BOOL SFLOW_msg_fs_collect(u32 *frm, vtss_port_no_t sample_port, vtss_port_no_t ingr_port, u32 copy_bytes, u32 orig, u32 stripped, u32 drops, u32 instance_mask)
{
    u32                  raw_hdr_len_dwords = (copy_bytes + (sizeof(u32) - 1)) / sizeof(u32);
    u32                  *buf = SFLOW_msg_sample_buffer_get(SFLOW_MSG_SAMPLE_FLOW_LEN_DWORDS(raw_hdr_len_dwords)), *ptr;
    sflow_port_state_t   *port_state;
    vtss_port_counters_t port_counters;
    vtss_port_counter_t  rx_packets, tx_packets;
    int                  inst;

    if (buf == NULL) {
        // Not room for this sample. Drop it.
        // Caller makes sure to count it.
        return FALSE;
    }

    ptr = buf;
    port_state = &SFLOW_local_state.port[sample_port];

    // Time to pupulate the buffer. First the internal header, which is encoded in host order.

    // --------------------------------
    // --- internal_header
    // --------------------------------
    // It's a flow sample
    ptr[SFLOW_MSG_SAMPLE_HEADER_IDX_TYPE] = SFLOW_META_SAMPLE_TYPE_FLOW;

    // Flow sample lengths are variable
    ptr[SFLOW_MSG_SAMPLE_HEADER_IDX_LEN_DWORDS] = SFLOW_MSG_SAMPLE_FLOW_LEN_DWORDS(raw_hdr_len_dwords) - SFLOW_MSG_SAMPLE_FLOW_IDX_FIRST;

    // Port number (iport)
    ptr[SFLOW_MSG_SAMPLE_HEADER_IDX_PORT] = sample_port;

    // Instance bitmask indicating who's gonna receive this sample.
    ptr[SFLOW_MSG_SAMPLE_HEADER_IDX_INSTANCE_MASK] = instance_mask;

    // Get port counters in order to be able to possibly reset sequence counter
    // and send a sample-pool. The sample-pool is a per-instance property, because
    // it depends on when the instance was started.
    // The standard says that sample_pool is:
    //   Total number of packets that could have
    //   been sampled (i.e. packets skipped by
    //   sampling process + total number of
    //   samples).
    //
    // I think "Total number of packets that could have been sampled"
    // can be understood in two ways:
    //   1) Total number of packets that could have been sampled if it hadn't
    //      been for lack of resources, i.e. drop of sFlow marked frames.
    //   2) Total number of packets received or transmitted on the interface,
    //      since every single frame that flows on the interface is subject to
    //      sampling.
    // I take it as #2 and use the port counters.
    if (port_mgmt_counters_get(VTSS_ISID_LOCAL, sample_port, &port_counters) != VTSS_RC_OK) {
        T_E("Unable to obtain local port counters for uport=%u", iport2uport(sample_port));
        memset(&port_counters, 0, sizeof(port_counters));
    }

    rx_packets = port_counters.rmon.rx_etherStatsPkts;
    tx_packets = port_counters.rmon.tx_etherStatsPkts;

    // Check to see if we need to reset sequence numbers.
    for (inst = 0; inst < SFLOW_INSTANCE_CNT; inst++) {
        if (instance_mask & VTSS_BIT(inst)) {
            vtss_sflow_type_t type        = SFLOW_stack_cfg[VTSS_ISID_LOCAL].port[sample_port].fs[inst].type;
            u32               sample_pool = 0;

            if (type == VTSS_SFLOW_TYPE_RX || type == VTSS_SFLOW_TYPE_ALL) {
                if (port_state->fs_start_rx_packets[inst] > rx_packets) {
                    // Someone reset the Rx statistics.
                    // Reset sequence number and start Rx packets.
                    port_state->fs_start_rx_packets[inst] = rx_packets;
                    port_state->fs_sequence_number[inst]  = 0;
                }
                sample_pool += (rx_packets - port_state->fs_start_rx_packets[inst]);
            }

            if (type == VTSS_SFLOW_TYPE_TX || type == VTSS_SFLOW_TYPE_ALL) {
                if (port_state->fs_start_tx_packets[inst] > tx_packets) {
                    // Someone reset the Tx statistics.
                    // Reset sequence number and start Tx packets.
                    port_state->fs_start_tx_packets[inst] = tx_packets;
                    port_state->fs_sequence_number[inst]  = 0;
                }
                sample_pool += (tx_packets - port_state->fs_start_tx_packets[inst]);
            }
            ptr[SFLOW_MSG_SAMPLE_HEADER_IDX_SAMPLE_POOL + inst] = (u32)sample_pool; // Truncate
        } else {
            ptr[SFLOW_MSG_SAMPLE_HEADER_IDX_SAMPLE_POOL + inst] = 0;
        }
    }

    // Flow sample sequence numbers.
    for (inst = 0; inst < SFLOW_INSTANCE_CNT; inst++) {
        if (instance_mask & VTSS_BIT(inst)) {
            ptr[SFLOW_MSG_SAMPLE_HEADER_IDX_SEQ_NUMBERS + inst] = ++port_state->fs_sequence_number[inst];
        } else {
            ptr[SFLOW_MSG_SAMPLE_HEADER_IDX_SEQ_NUMBERS + inst] = 0;
        }
    }

    // Advance the pointer to the UDP flow sample, encoded in network order.
    ptr = &ptr[SFLOW_MSG_SAMPLE_FLOW_IDX_FIRST];

    // --------------------------------
    // --- sample_record
    // --------------------------------

    // u32 sample_format. 0x00000001 (enterprise = 0, format = 1 => flow sample).
    SFLOW_ENCODE_32(ptr, 0x00000001);

    // --------------------------------
    // --- flow sample (opaque)
    // --------------------------------

    // u32 sample_length. Size of remainder of this record. It's the 32-bit aligned size of the raw header + 14 dwords
    SFLOW_ENCODE_32(ptr, (raw_hdr_len_dwords + 14) * sizeof(u32));

    // u32 sequence_number. To be filled in by the master, since this sample may go to multiple receivers.
    ptr++;

    // u32 source_id. Local iport number inserted locally and replaced by master switch who knows the USID of slave switches.
    ptr++;

    // u32 sampling_rate. Also a per-instance property to be filled in by the master. There may be race-conditions here, because
    // the slave uses one sampling rate while the master is re-configured with another, which is sent to slaves only after
    // the master receives this sample, but we live with that.
    ptr++;

    // u32 sample_pool. Also a per-instance property, which we'we inserted in this sample's header.
    ptr++;

    // u32 drops. A global property which is only reset with an agent reset.
    SFLOW_ENCODE_32(ptr, drops);

    // u32 input. In case of Rx flow sample: Same as #source_id. In case of Tx flow sample: VTSS_PORT_NO_NONE if received on an interconnect on JR-48, otherwise local source port. To be replaced by master.
    *(ptr++) = ingr_port; // Host order.

    // u32 output. In case of Rx flow sample: 0 (unknown source). In case of Tx flow sample: Same as #source_id.
    *(ptr++) = sample_port == ingr_port ? VTSS_PORT_NO_NONE : sample_port; // Host order

    // u32 record_cnt. 1 (only one sample record at a time).
    SFLOW_ENCODE_32(ptr, 1);

    // --------------------------------
    // --- flow_record
    // --------------------------------

    // u32 flow_format. 0x00000001 (enterprise = 0, format = 1 => raw packet header).
    SFLOW_ENCODE_32(ptr, 0x00000001);

    // --------------------------------
    // --- flow_data
    // --------------------------------

    // u32 opaque length field. Size of the remainder of this record. It's the 32-bit aligned size of the raw header + 4 dwords
    SFLOW_ENCODE_32(ptr, (raw_hdr_len_dwords + 4) * sizeof(u32));

    // u32 header_protocol. 1 = IEEE802.3 Ethernet.
    SFLOW_ENCODE_32(ptr, 1);

    // u32 frame_length. Original frame length.
    SFLOW_ENCODE_32(ptr, orig);

    // u32 stripped. How many bytes did we strip off?
    SFLOW_ENCODE_32(ptr, stripped);

    // u32 header_size. Number of bytes actually in #header (excluding padding)
    SFLOW_ENCODE_32(ptr, copy_bytes);

    // u32 header[(SFLOW_FLOW_HEADER_SIZE_MAX + 3) / 4]. Must pad to 32-bit boundary (XDR variable-length opaque type).
    // The padding with zeros has already been done.
    memcpy(ptr, frm, raw_hdr_len_dwords * sizeof(u32));

    SFLOW_ASSERT(ptr - buf + raw_hdr_len_dwords == SFLOW_MSG_SAMPLE_FLOW_LEN_DWORDS(raw_hdr_len_dwords));

    return TRUE;
}

/****************************************************************************/
// SFLOW_fs_construct()
/****************************************************************************/
static SFLOW_INLINE BOOL SFLOW_fs_construct(u32 *frm, vtss_port_no_t sample_port, vtss_port_no_t ingr_port, u32 copy_bytes, u32 orig, u32 stripped, u32 drops)
{
    sflow_port_state_t *port_state = &SFLOW_local_state.port[sample_port];
    int                inst;
    u32                instance_mask = 0;
    BOOL               is_tx = sample_port != ingr_port;

    SFLOW_CRIT_ASSERT_LOCKED();

    // Find the active instance(s).
    for (inst = 0; inst < SFLOW_INSTANCE_CNT; inst++) {
        if (port_state->fs_samples_left[inst]) {
            vtss_sflow_type_t type = SFLOW_stack_cfg[VTSS_ISID_LOCAL].port[sample_port].fs[inst].type;
            // Only interesting if this is in the right direction.
            if (type == VTSS_SFLOW_TYPE_ALL || (is_tx && type == VTSS_SFLOW_TYPE_TX) || (is_tx == FALSE && type == VTSS_SFLOW_TYPE_RX)) {
                if (--port_state->fs_samples_left[inst] == 0) {
                    // Mark this instance as receiver of frame sample
                    instance_mask |= VTSS_BIT(inst);

                    // Prepare for next update. Notice that we're subsampling here to support multiple
                    // instances on the same port.
                    SFLOW_ASSERT(port_state->fs_api_conf.sampling_rate != 0, return FALSE;);
                    port_state->fs_samples_left[inst] = SFLOW_stack_cfg[VTSS_ISID_LOCAL].port[sample_port].fs[inst].sampling_rate / port_state->fs_api_conf.sampling_rate;
                }
            }
        }
    }

    if (instance_mask != 0) {
        // Collect the flow sample in a message, which will be sent to the current master whenever we're done here, or if the message overflows.
        return SFLOW_msg_fs_collect(frm, sample_port, ingr_port, copy_bytes, orig, stripped, drops, instance_mask);
    }

    // sFlow-marked frames were in the pipeline for this port, when it timed out or was reconfigured. This is not an error.
    return TRUE;
}

/****************************************************************************/
// SFLOW_msg_cp_collect()
// Collects counters in a message buffer supposed to be sent to the master
// when it's full or triggered.
// Returns TRUE if successfully stored in buffer. FALSE if out of space.
/****************************************************************************/
static SFLOW_INLINE BOOL SFLOW_msg_cp_collect(vtss_port_no_t port, u32 instance_mask, vtss_port_if_group_counters_t *counters)
{
    u32                *buf = SFLOW_msg_sample_buffer_get(SFLOW_MSG_SAMPLE_COUNTER_LEN_DWORDS), *ptr;
    sflow_port_state_t *port_state;
    port_conf_t        port_conf;
    int                inst;
    vtss_rc            rc;

    if (buf == NULL) {
        // Not room for this sample. Drop it.
        return FALSE;
    }

    ptr = buf;
    port_state = &SFLOW_local_state.port[port];

    // Time to pupulate the buffer. First the internal header, which is encoded in host order.

    // --------------------------------
    // --- internal_header
    // --------------------------------
    // It's a counter sample
    ptr[SFLOW_MSG_SAMPLE_HEADER_IDX_TYPE] = SFLOW_META_SAMPLE_TYPE_COUNTER;

    // Counter sample lengths are currently fixed.
    ptr[SFLOW_MSG_SAMPLE_HEADER_IDX_LEN_DWORDS] = SFLOW_MSG_SAMPLE_COUNTER_LEN_DWORDS - SFLOW_MSG_SAMPLE_COUNTER_IDX_FIRST;

    // Port number (iport)
    ptr[SFLOW_MSG_SAMPLE_HEADER_IDX_PORT] = port;

    // Instance bitmask indicating who's gonna receive this sample.
    ptr[SFLOW_MSG_SAMPLE_HEADER_IDX_INSTANCE_MASK] = instance_mask;

    // Counter sequence numbers.
    for (inst = 0; inst < SFLOW_INSTANCE_CNT; inst++) {
        if (instance_mask & VTSS_BIT(inst)) {
            ptr[SFLOW_MSG_SAMPLE_HEADER_IDX_SEQ_NUMBERS + inst] = ++port_state->cp_sequence_number[inst];
        } else {
            ptr[SFLOW_MSG_SAMPLE_HEADER_IDX_SEQ_NUMBERS + inst] = 0;
        }
    }

    // Advance the pointer to the UDP counter sample, encoded in network order.
    ptr = &ptr[SFLOW_MSG_SAMPLE_COUNTER_IDX_FIRST];

    // --------------------------------
    // --- sample_record
    // --------------------------------

    // u32 sample_type. 0x00000002 (enterprise = 0, format = 2 => counters_sample).
    SFLOW_ENCODE_32(ptr, 0x00000002);

    // --------------------------------
    // --- counters_sample (opaque)
    // --------------------------------

    // u32 opaque length field. 108 = fixed size of remainder of this sample.
    SFLOW_ENCODE_32(ptr, 108);

    // u32 sequence_number. This must be updated per instance by the master.
    // The real sequence number for a given instance is included in the internal header
    // of this sample.
    ptr++;

    // u32 source_id. iport inserted by master switch who knows the USID of slave switches.
    ptr++;

    // u32 record_cnt. 1 (only one sample record per I/F per message).
    SFLOW_ENCODE_32(ptr, 1);

    // --------------------------------
    // --- counter_record
    // --------------------------------

    // u32 counter_format. 0x00000001 (enterprise = 0, format = 1 => if_counters).
    SFLOW_ENCODE_32(ptr, 0x00000001);

    // --------------------------------
    // --- counter_data
    // --------------------------------

    // u32 opaque length field. 88 = Size of remainder of this sample.
    SFLOW_ENCODE_32(ptr, 88);

    // u32 if_index. Port number inserted locally and replaced by master switch who knows the USID of slave switches.
    ptr++;

    // u32 if_type. 6 = IANA_IF_TYPE_ETHERNET.
    SFLOW_ENCODE_32(ptr, IANA_IF_TYPE_ETHERNET);

    // u64 if_speed. Port speed in bps.
    SFLOW_ENCODE_64(ptr, port_state->port_speed_bps);

    // u32 if_direction. 1 = Full duplex, 2 = half duplex.
    SFLOW_ENCODE_32(ptr, port_state->port_link ? (port_state->port_fdx ? 1 : 2) : 0);

    // u32 if_status.
    // Ask port module about admin status (ifAdminStatus)
    if ((rc = port_mgmt_conf_get(VTSS_ISID_LOCAL, port, &port_conf)) != VTSS_RC_OK) {
        T_E("port_mgmt_conf_get(%d) return %d = %s", iport2uport(port), rc, error_txt(rc));
        port_conf.enable = TRUE;
    }

    // Bit 0: ifAdminStatus: 0 = down, 1 = up (administratively disabled/enabled)
    // Bit 1: ifOperStatus : 0 = down, 1 = up (link down/up)
    SFLOW_ENCODE_32(ptr, port_conf.enable ? (port_state->port_link ? 3 : 1) : 0); // Better make sure not to report admin status down while link is up.

    // And then the counters
    SFLOW_ENCODE_64(ptr, counters->ifInOctets);
    SFLOW_ENCODE_32(ptr, counters->ifInUcastPkts);
    SFLOW_ENCODE_32(ptr, counters->ifInMulticastPkts);
    SFLOW_ENCODE_32(ptr, counters->ifInBroadcastPkts);
    SFLOW_ENCODE_32(ptr, counters->ifInDiscards);
    SFLOW_ENCODE_32(ptr, counters->ifInErrors);
    SFLOW_ENCODE_32(ptr, 0); // ifInUnknownProtos
    SFLOW_ENCODE_64(ptr, counters->ifOutOctets);
    SFLOW_ENCODE_32(ptr, counters->ifOutUcastPkts);
    SFLOW_ENCODE_32(ptr, counters->ifOutMulticastPkts);
    SFLOW_ENCODE_32(ptr, counters->ifOutBroadcastPkts);
    SFLOW_ENCODE_32(ptr, counters->ifOutDiscards);
    SFLOW_ENCODE_32(ptr, counters->ifOutErrors);
    SFLOW_ENCODE_32(ptr, 0); // ifPromiscuousMode

    SFLOW_ASSERT(ptr - buf == SFLOW_MSG_SAMPLE_COUNTER_LEN_DWORDS);

    return TRUE;
}

/****************************************************************************/
// SFLOW_cp_poll()
/****************************************************************************/
static SFLOW_INLINE void SFLOW_cp_poll(void)
{
    port_iter_t pit;
    int         inst;
    u32         instance_mask;

    SFLOW_CRIT_ASSERT_LOCKED();

    (void)port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);

    // Figure out which counters to poll - if any.
    while (port_iter_getnext(&pit)) {
        sflow_port_state_t *port_state = &SFLOW_local_state.port[pit.iport];
        if (port_state->cp_active) {
            // At least one instance on this port is active.
            instance_mask = 0;

            // Find the active instance(s).
            for (inst = 0; inst < SFLOW_INSTANCE_CNT; inst++) {
                if (port_state->cp_seconds_left[inst]) {
                    if (--port_state->cp_seconds_left[inst] == 0) {
                        // Mark this instance as receiver of counter update
                        instance_mask |= VTSS_BIT(inst);

                        // Prepare for next counter update.
                        port_state->cp_seconds_left[inst] = SFLOW_stack_cfg[VTSS_ISID_LOCAL].port[pit.iport].cp[inst].interval;
                    }
                }
            }

            if (instance_mask != 0) {
                vtss_port_counters_t port_counters;
                vtss_port_counter_t  rx_octets, tx_octets;

                // At least one instance needs these counters at this tick. Read the counters.
                if (port_mgmt_counters_get(VTSS_ISID_LOCAL, pit.iport, &port_counters) != VTSS_RC_OK) {
                    T_E("Unable to read port counters for port #%u", pit.uport);
                    memset(&port_counters, 0, sizeof(port_counters));
                }

                rx_octets = port_counters.if_group.ifInOctets;
                tx_octets = port_counters.if_group.ifOutOctets;

                // Gotta loop once more to check if we need to reset the sequence_number for certain instances.
                // This could happen if statistics are cleared in between previous sampling and current.
                for (inst = 0; inst < SFLOW_INSTANCE_CNT; inst++) {
                    if (instance_mask & VTSS_BIT(inst)) {
                        if (port_state->cp_prev_rx_octets[inst] > rx_octets || port_state->cp_prev_tx_octets[inst] > tx_octets) {
                            port_state->cp_sequence_number[inst] = 0;
                        }
                        port_state->cp_prev_rx_octets[inst] = rx_octets;
                        port_state->cp_prev_tx_octets[inst] = tx_octets;
                    }
                }

                // Collect the counters in a message, which will be sent to the current master whenever we're done here,
                // or if the message overflows.
                (void)SFLOW_msg_cp_collect(pit.iport, instance_mask, &port_counters.if_group);
            }
        }
    }
}

/****************************************************************************/
// SFLOW_timeout()
/****************************************************************************/
static void SFLOW_timeout(vtss_timer_t *timer)
{
    // Another second has elapsed. Tell it to the thread.
    // This could also have been done by the thread itself, but
    // in order to keep timing calculations out of a cyg_flag_timed_wait()
    // call, we can do with a call to cyg_flag_wait().
    cyg_flag_setbits(&SFLOW_wakeup_thread_flag, SFLOW_THREAD_FLAG_TIMEOUT);
}

/****************************************************************************/
// SFLOW_dgram_flush()
/****************************************************************************/
static void SFLOW_dgram_flush(u32 rcvr_idx)
{
    u32 rcvr_idx_min, rcvr_idx_max;

    if (rcvr_idx == 0) {
        rcvr_idx_min = 1;
        rcvr_idx_max = SFLOW_RECEIVER_CNT;
    } else {
        rcvr_idx_min = rcvr_idx_max = rcvr_idx;
    }

    for (rcvr_idx = rcvr_idx_min; rcvr_idx <= rcvr_idx_max; rcvr_idx++) {
        sflow_rcvr_state_t *rcvr_state = &SFLOW_rcvr_state[rcvr_idx];

        if (rcvr_state->sockfd != -1 && rcvr_state->sample_cnt > 0) {
            // Flush dgram into socket after having updated the header of the datagram
            BOOL is_ipv6 = SFLOW_dgram_header_update(rcvr_idx);

            int err = sendto(rcvr_state->sockfd, is_ipv6 ? &rcvr_state->dgram[0] : &rcvr_state->dgram[3], sizeof(u32) * (is_ipv6 ? rcvr_state->dgram_len_dwords : rcvr_state->dgram_len_dwords - 3), 0, rcvr_state->addrinfo->ai_addr, rcvr_state->addrinfo->ai_addrlen);
            if (err <= 0) {
                T_W("sendto(%s) failed. sendto() returned %d. errno = %d = \"%s\"", rcvr_state->addrinfo->ai_family == AF_INET ? "IPv4" : "IPv6", err, errno, strerror(errno));
                SFLOW_rcvr_statistics[rcvr_idx].dgrams_err++;
            } else {
                SFLOW_rcvr_statistics[rcvr_idx].dgrams_ok++;
            }
            // Re-initialize datagram
            SFLOW_dgram_init(rcvr_state);
        }
    }
}

/****************************************************************************/
// SFLOW_dgram_update()
// Transmit samples in #buf to the receiver.
/****************************************************************************/
static SFLOW_INLINE void SFLOW_dgram_update(vtss_isid_t isid, u32 *buf, i32 len_dwords)
{
    // buf points to one of more samples to be shipped off to one or more receivers.
    // The format is according to SFLOW_MSG_SAMPLE_HEADER_IDX_xxx.
    // The number of samples is dictated by every sample's length and #len_dwords in combination.
    // #len_dwords is a signed integer to avoid an endless loop here in case something goes wrong.
    while (len_dwords > 0) {
        sflow_meta_sample_type_t type              = buf[SFLOW_MSG_SAMPLE_HEADER_IDX_TYPE];
        u32                      sample_len_dwords = buf[SFLOW_MSG_SAMPLE_HEADER_IDX_LEN_DWORDS];
        vtss_port_no_t           sample_port       = buf[SFLOW_MSG_SAMPLE_HEADER_IDX_PORT];
        u32                      instance_mask     = buf[SFLOW_MSG_SAMPLE_HEADER_IDX_INSTANCE_MASK];
        u32                      *sample_pool      = &buf[SFLOW_MSG_SAMPLE_HEADER_IDX_SAMPLE_POOL];
        u32                      *seq_numbers      = &buf[SFLOW_MSG_SAMPLE_HEADER_IDX_SEQ_NUMBERS];
        int                      inst;
        u32                      rcvr_idx;
        sflow_rcvr_t             *rcvr_cfg;
        sflow_rcvr_state_t       *rcvr_state;
        u32                      max_dgram_len_dwords;
        i32                      total_sample_len_dwords = (i32)sample_len_dwords + SFLOW_MSG_SAMPLE_HEADER_LEN_DWORDS;
        BOOL                     ports_replaced = FALSE, is_tx = FALSE /* Satisfy lint */;

        SFLOW_ASSERT(type == SFLOW_META_SAMPLE_TYPE_FLOW || type == SFLOW_META_SAMPLE_TYPE_COUNTER);
        SFLOW_ASSERT(len_dwords >= (i32)total_sample_len_dwords);
        SFLOW_ASSERT(instance_mask != 0);
        SFLOW_ASSERT(sample_port < VTSS_PORTS && VTSS_ISID_LEGAL(isid), return;);

        // Loop over the instances causing this sample
        for (inst = 0; inst < SFLOW_INSTANCE_CNT; inst++) {
            if ((instance_mask & VTSS_BIT(inst)) == 0) {
                // Not due to this instance.
                continue;
            }

            if (type == SFLOW_META_SAMPLE_TYPE_FLOW) {
                rcvr_idx = SFLOW_stack_cfg[isid].port[sample_port].fs[inst].receiver;
                if (SFLOW_stack_cfg[isid].port[sample_port].fs[inst].sampling_rate == 0) {
                    // Well, this flow sample came too late (just reconfigured or receiver timed out)
                    continue;
                }
            } else {
                rcvr_idx = SFLOW_stack_cfg[isid].port[sample_port].cp[inst].receiver;
                if (SFLOW_stack_cfg[isid].port[sample_port].cp[inst].interval == 0) {
                    // Well, this counter sample came too late (just reconfigured or receiver timed out).
                    continue;
                }
            }

            SFLOW_ASSERT(rcvr_idx <= SFLOW_RECEIVER_CNT, return;);

            if (rcvr_idx == 0) {
                // Receiver went away in the meantime.
                continue;
            }
            rcvr_state = &SFLOW_rcvr_state[rcvr_idx];
            if (rcvr_state->sockfd == -1) {
                // Socket no longer open.
                continue;
            }
            rcvr_cfg = &SFLOW_rcvr_cfg[rcvr_idx];
            max_dgram_len_dwords = rcvr_cfg->max_datagram_size / sizeof(u32);

            // Check to see if there's room in the actual datagram.
            if (rcvr_state->dgram_len_dwords + sample_len_dwords > max_dgram_len_dwords && rcvr_state->sample_cnt > 0) {
                SFLOW_dgram_flush(rcvr_idx);
            }

            // Check to see if there's room after flushing.
            if (rcvr_state->dgram_len_dwords + sample_len_dwords <= max_dgram_len_dwords) {
                // There is.
                // Start by updating the fields of the sample that couldn't be updated
                // on the slave due to either lack of information or duplicate information (due to
                // multiple receivers of same sample).
                u32 ifindex_network_order = htonl(GET_UNIQUE_PORT_ID(isid, sample_port));

                switch (type) {
                case SFLOW_META_SAMPLE_TYPE_FLOW: {
                    vtss_port_no_t ingr_port;
                    vtss_port_no_t egr_port;

                    if (!ports_replaced) {
                        ingr_port = buf[SFLOW_MSG_SAMPLE_FLOW_IDX_INPUT]; // Host ordered so far.
                        egr_port  = buf[SFLOW_MSG_SAMPLE_FLOW_IDX_OUTPUT]; // Host ordered so far.

                        buf[SFLOW_MSG_SAMPLE_FLOW_IDX_SOURCE_ID]  = ifindex_network_order;

                        // The following line basically says: If we don't know the ingress port (VTSS_PORT_NO_NONE), it's because it's a Tx sample
                        // received on an interconnect port on a JR-48. Otherwise, if it's the same as the sample port, use the precomputed port,
                        // otherwise call the function to get a unique ID again.
                        buf[SFLOW_MSG_SAMPLE_FLOW_IDX_INPUT] = ingr_port == VTSS_PORT_NO_NONE ? 0 : ingr_port == sample_port ? ifindex_network_order : htonl(GET_UNIQUE_PORT_ID(isid, ingr_port));

                        // The output interface is unknown for Rx flow samples, and the same as the sample_port in case of Tx samples.
                        buf[SFLOW_MSG_SAMPLE_FLOW_IDX_OUTPUT] = egr_port == VTSS_PORT_NO_NONE ? 0 : ifindex_network_order;

                        is_tx = sample_port != ingr_port;

                        // Preent this conversion from taking place more than once per sample. Doing it twice would cause unpredictable results.
                        ports_replaced = TRUE;
                    }

                    buf[SFLOW_MSG_SAMPLE_FLOW_IDX_SEQ_NUMBER]    = htonl(seq_numbers[inst]);
                    buf[SFLOW_MSG_SAMPLE_FLOW_IDX_SAMPLING_RATE] = htonl(SFLOW_stack_cfg[isid].port[sample_port].fs[inst].sampling_rate);
                    buf[SFLOW_MSG_SAMPLE_FLOW_IDX_SAMPLE_POOL]   = htonl(sample_pool[inst]);

                    if (is_tx) {
                        SFLOW_stack_statistics[isid].port[sample_port].fs_tx[inst]++;
                    } else {
                        SFLOW_stack_statistics[isid].port[sample_port].fs_rx[inst]++;
                    }
                    SFLOW_rcvr_statistics[rcvr_idx].fs++;
                    break;
                }

                case SFLOW_META_SAMPLE_TYPE_COUNTER:

                    buf[SFLOW_MSG_SAMPLE_COUNTER_IDX_SEQ_NUMBER] = htonl(seq_numbers[inst]);
                    buf[SFLOW_MSG_SAMPLE_COUNTER_IDX_SOURCE_ID]  = ifindex_network_order;
                    buf[SFLOW_MSG_SAMPLE_COUNTER_IDX_IF_IDX]     = ifindex_network_order;
                    SFLOW_stack_statistics[isid].port[sample_port].cp[inst]++;
                    SFLOW_rcvr_statistics[rcvr_idx].cp++;
                    break;
                }

                memcpy(&rcvr_state->dgram[rcvr_state->dgram_len_dwords], &buf[SFLOW_MSG_SAMPLE_HEADER_LEN_DWORDS], sample_len_dwords * sizeof(u32));

                rcvr_state->sample_cnt++;
                rcvr_state->dgram_len_dwords += sample_len_dwords;
            }
        }

        // Done with this sample. Go one with the next.

        len_dwords -= total_sample_len_dwords;
        buf        += total_sample_len_dwords;
    }

    // Must have consumed exactly all.
    SFLOW_ASSERT(len_dwords == 0);
}

/****************************************************************************/
// SFLOW_thread()
/****************************************************************************/
static void SFLOW_thread(cyg_addrword_t data)
{
    // Timer which fires and wakes up ourselves every one second.
    // Upon expiration, it's the intentation that we sample interesting
    // counters and send them to the master. Also, we need to send
    // possible flow samples gathered within the last second to the master.
    // Finally, the master needs to keep track of receiver timeouts.
    // The reason for using a timer to signal our thread is that this
    // allows us to get a steady one-sec timeout without having to do
    // all kinds of funny computations if the thread is awakened by
    // e.g. a flow sample in between two timer ticks.
    vtss_timer_t     timer;
    cyg_flag_value_t flag;
    BOOL             pollers_active = FALSE;

    if (vtss_timer_initialize(&timer) != VTSS_RC_OK) {
        T_E("Unable to initialize timer");
    }

    timer.modid     = VTSS_MODULE_ID_SFLOW;
    timer.repeat    = TRUE;    // Never-ending.
    timer.period_us = 1000000; // 1000000 microseconds = 1 second.
    timer.callback  = SFLOW_timeout;
    memset(&SFLOW_local_state, 0, sizeof(SFLOW_local_state));

    if (vtss_timer_start(&timer) != VTSS_RC_OK) {
        T_E("Unable to start timer");
    }

    // First time we exit the mutexes (this confuses Lint)
    /*lint --e{455} */
    SFLOW_BIP_CRIT_EXIT();
    SFLOW_CRIT_EXIT();

    while (1) {
        flag = cyg_flag_wait(&SFLOW_wakeup_thread_flag, 0xFFFFFFFF, CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR);

        if (flag & SFLOW_THREAD_FLAG_CFG_CHANGE) {
            // We got here due to a configuration change.
            SFLOW_CRIT_ENTER();
            // Gotta investigate differences between SFLOW_stack_cfg[VTSS_ISID_LOCAL] and SFLOW_local_new_cfg.
            pollers_active = SFLOW_cp_state_update();
            SFLOW_fs_state_update();
            SFLOW_CRIT_EXIT();
        }

        if (flag & SFLOW_THREAD_FLAG_MASTER_UP) {
            SFLOW_BIP_CRIT_ENTER();
            vtss_bip_buffer_clear(&SFLOW_master_bip);
            SFLOW_BIP_CRIT_EXIT();
        }

        if (flag & SFLOW_THREAD_FLAG_FLOW_SAMPLE) {
            BOOL done;
            do {
                int            size;
                u32            *bip_buf;
                u32            buf[SFLOW_FLOW_HEADER_SIZE_MAX]; // Provided there's room on the stack.
                u32            dword_len = 0;
                u32            copy_bytes = 0, orig = 0, stripped = 0, aligned_byte_len;
                u32            drops = 0;
                BOOL           success;
                vtss_port_no_t sample_port = VTSS_PORT_NO_START, ingr_port = VTSS_PORT_NO_START;

                // We've received new flow samples in the BIP-buffer. Get'em.
                // The BIP-buffer is protected by another mutex than our normal structures
                // because we don't want the Packet Rx thread to wait for us if we're busy.
                SFLOW_BIP_CRIT_ENTER();
                if ((bip_buf = (u32 *)vtss_bip_buffer_get_contiguous_block(&SFLOW_local_bip, &size)) != NULL) {
                    // Layout of BIP buffer:
                    //  [0] Number of dwords allocated in the BIP buffer for this flow sample, including this 6-dword header.
                    //  [1] Size of the copied header in bytes.
                    //  [2] Original length of frame in bytes.
                    //  [3] Number of stripped bytes.
                    //  [4] Port number on which the packet was sampled.
                    //  [5] In case of a Tx frame, the ingress port number. In case of Rx, the same as the port on which it was sampled.
                    //  [6-end] Frame header.
                    dword_len   = bip_buf[0];
                    copy_bytes  = bip_buf[1];
                    orig        = bip_buf[2];
                    stripped    = bip_buf[3];
                    sample_port = bip_buf[4];
                    ingr_port   = bip_buf[5];
                    aligned_byte_len = sizeof(u32) * ((copy_bytes + (sizeof(u32) - 1)) / sizeof(u32));
                    SFLOW_ASSERT(dword_len > 6 && dword_len * sizeof(u32) <= (u32)size && copy_bytes != 0 && aligned_byte_len <= SFLOW_FLOW_HEADER_SIZE_MAX);
                    memcpy(buf, &bip_buf[6], aligned_byte_len);
                    vtss_bip_buffer_decommit_block(&SFLOW_local_bip, dword_len * sizeof(u32));

                    // While we're at it, also copy the drop counter set by the Rx thread in case
                    // the BIP buffer ran full.
                    drops = SFLOW_local_info_exchange[sample_port].drops;
                }
                SFLOW_BIP_CRIT_EXIT();

                done = bip_buf == NULL;

                if (!done) {
                    // Process the flow sample, i.e. pack it into datagram format and send it to the master.
                    SFLOW_CRIT_ENTER();
                    success = SFLOW_fs_construct(buf, sample_port, ingr_port, copy_bytes, orig, stripped, drops);
                    SFLOW_CRIT_EXIT();

                    if (!success) {
                        T_D("Unable to get message buffer room for sample frame");
                        // Unable to allocate room for the sample in the message buffer.
                        // Put back updated drop counter in the exchange structure.
                        SFLOW_BIP_CRIT_ENTER();
                        SFLOW_local_info_exchange[sample_port].drops++;
                        SFLOW_BIP_CRIT_EXIT();
                    }
                }
            } while (!done);
        }

        if (flag & SFLOW_THREAD_FLAG_TIMEOUT) {
            SFLOW_CRIT_ENTER();

            if (pollers_active) {
                // Counter polling timeout.
                SFLOW_cp_poll();
            }

            // Flush any message buffers we have. The standard specifies
            // that we cannot hold flow samples for more than one second.
            // We violate this a bit, since we first need to send them
            // to the master, who will have to put them into the relevant
            // UDP datagrams and ship them off, but that's not a big deal.
            SFLOW_msg_flush();

            // If we're master, we need to check all receivers for timeout.
            if (msg_switch_is_master()) {
                int rcvr_idx;
                for (rcvr_idx = 1; rcvr_idx <= SFLOW_RECEIVER_CNT; rcvr_idx++) {
                    if (SFLOW_rcvr_state[rcvr_idx].timeout_left && --SFLOW_rcvr_state[rcvr_idx].timeout_left == 0) {
                        SFLOW_release_receiver(rcvr_idx);
                    }
                }
            }
            SFLOW_CRIT_EXIT();
        }

        if (flag & SFLOW_THREAD_FLAG_SAMPLES) {
            // Slave has sent us some samples to be transmitted to receivers.
            int orig_size;
            u32 *bip_buf;

            // We've received new samples in the BIP-buffer from a slave. Get'em.
            // The BIP-buffer is protected by another mutex than our normal structures
            // because we don't want the message Rx thread to wait for us if we're busy.
            SFLOW_BIP_CRIT_ENTER();
            bip_buf = (u32 *)vtss_bip_buffer_get_contiguous_block(&SFLOW_master_bip, &orig_size);
            SFLOW_BIP_CRIT_EXIT();

            // Process the samples from the slave.
            if (bip_buf != NULL) {
                int size = orig_size;

                SFLOW_CRIT_ENTER();

                while (size > 0) {
                    // First dword is the buffer length in dwords including the length and the ISID fields themselves.
                    // Second dword is the isid of the originating switch.
                    u32         dword_len = bip_buf[0];
                    vtss_isid_t isid      = bip_buf[1];

                    SFLOW_ASSERT(dword_len > 2 && dword_len * sizeof(u32) <= (u32)size);

                    // First two words of bip_buf held meta-data.
                    SFLOW_dgram_update(isid, bip_buf + 2, dword_len - 2);

                    // Next sample
                    bip_buf += dword_len;
                    size    -= sizeof(u32) * dword_len;
                }

                SFLOW_CRIT_EXIT();

                // Free whatever we got
                SFLOW_BIP_CRIT_ENTER();
                vtss_bip_buffer_decommit_block(&SFLOW_master_bip, orig_size);
                SFLOW_BIP_CRIT_EXIT();

                // Flush any datagrams that might not be flushed by now.
                SFLOW_dgram_flush(0); // 0 == All receivers.
            }
        }
    }
}

/****************************************************************************/
// SFLOW_port_speed_to_bps()
/****************************************************************************/
static SFLOW_INLINE u64 SFLOW_port_speed_to_bps(vtss_port_speed_t speed)
{
#define SFLOW_MILLION 1000000ULL
    switch (speed) {
    case VTSS_SPEED_10M:
        return 10 * SFLOW_MILLION;

    case VTSS_SPEED_100M:
        return 100 * SFLOW_MILLION;

    case VTSS_SPEED_1G:
        return 1000 * SFLOW_MILLION;

    case VTSS_SPEED_2500M:
        return 2500 * SFLOW_MILLION;

    case VTSS_SPEED_5G:
        return 5000 * SFLOW_MILLION;

    case VTSS_SPEED_10G:
        return 10000 * SFLOW_MILLION;

    case VTSS_SPEED_12G:
        return 12000 * SFLOW_MILLION;

    default:
        return 0;
    }
}

/****************************************************************************/
// SFLOW_port_change_callback()
// Called back by port module whenever a port change is seen.
// We need to send some port state info in counter samplings.
/****************************************************************************/
static void SFLOW_port_change_callback(vtss_port_no_t port, port_info_t *info)
{
    SFLOW_CRIT_ENTER();
    SFLOW_local_state.port[port].port_speed_bps = SFLOW_port_speed_to_bps(info->speed);
    SFLOW_local_state.port[port].port_link      = info->link;
    SFLOW_local_state.port[port].port_fdx       = info->fdx;
    SFLOW_CRIT_EXIT();
}

/****************************************************************************/
// SFLOW_sampling_rate_api_test()
// Run this test on every new architecture.
// Once it passes, put that architecture in the list of !defined() below,
// and also in the call to this function from sflow_init().
/****************************************************************************/
#if !defined(VTSS_ARCH_JAGUAR_1) && !defined(VTSS_ARCH_LUTON26) && !defined(VTSS_ARCH_SERVAL)
static void SFLOW_sampling_rate_api_test(void)
{
    u32 in, out, out_max = 0, error_cnt = 0;
    BOOL error;

    // When it runs, make sure the implementor is notified - hence the T_E() here:
    T_E("Starting sampling rate test");

    // Find highest possible sampling rate:
    in = 1;
    do {
        (void)vtss_sflow_sampling_rate_convert(NULL, FALSE, in, &out);
        if (out == out_max) {
            break;
        }
        out_max = out;
        in <<= 1;
    } while (in != 0); // Stop at latest when the MSBit shifts out.

    if (out_max == 0) {
        T_E("Unable to find highest sampling rate");
        error_cnt++;
        goto do_exit;
    } else {
        T_D("Highest sampling rate = %lu", out_max);
    }


    // Test that if a requested sampling rate of 'in' gives a realized sampling
    // rate of 'out', then a requested sampling rate of 'out' must give a
    // realized sampling rate of 'out'.
    for (in = 0; in <= out_max; in++) {
        (void)vtss_sflow_sampling_rate_convert(NULL, FALSE, in, &out);
        if (in != out) {
            u32 new_out;
            (void)vtss_sflow_sampling_rate_convert(NULL, FALSE, out, &new_out);
            if (out != new_out) {
                T_E("Hysteresis in sampler if 'in' gives 'out', then 'out' must give 'out', but it gives 'in'=%lu, 'out'=%lu, 'new_out'=%lu", in, out, new_out);
                error_cnt++;
            }
        }
    }

    // Test that requested sampling rates that are powers of two produces realized
    // sampling rates that are powers of two.
    in = 1;
    while (in <= out_max) {
        (void)vtss_sflow_sampling_rate_convert(NULL, FALSE, in, &out);
        if (in != out) {
            T_E("Power-of-two test failed: Input sampling rate of '%lu' gives realized sampling rate of '%lu", in, out);
            error_cnt++;
        }
        in <<= 1;
    }

    // Test that one can request powers of two.
    for (in = 1; in <= out_max; in++) {
        (void)vtss_sflow_sampling_rate_convert(NULL, TRUE, in, &out);
        if (out == 0) {
            T_E("Power-of-two return test failed: in = %lu produces out = 0", in);
            error_cnt++;
            continue;
        }
        if ((out & (out - 1)) != 0) {
            // Returned sampling rate is not a power of two.
            T_E("Power-of-two return test failed: Input sampling rate of '%lu = 0x%lx' gives realized sampling rate of '%lu = 0x%lx'", in, in, out, out);
            error_cnt++;
            continue;
        }

        // Check range.
        error = FALSE;
        if (out > in) {
            u32 out_smaller = out >> 1;
            if (out_smaller > in) {
                error = TRUE;
            } else if (out - in > in - out_smaller) {
                error = TRUE;
            }
        } else if (out < in) {
            u32 out_higher = out << 1;
            if (out > out_max) {
                T_E("Power-of-two return test failed: Returned a sampling rate (%lu) higher than it's maximum supported (%lu)", out, out_max);
                error_cnt++;
            } else if (out_higher != 0 && out_higher < out_max) {
                // Didn't wrap.
                if (out_higher < in) {
                    error = TRUE;
                } else if (out_higher - in < in - out) {
                    error = TRUE;
                }
            }
        }

        if (error) {
            T_E("Power-of-two return test failed: Input sampling rate of '%lu = 0x%lx' gives realized sampling rate of '%lu = 0x%lx'", in, in, out, out);
            error_cnt++;
        }
    }

do_exit:
    T_E("Sampling rate test completed with %lu errors", error_cnt);
}
#endif

/******************************************************************************/
//
// PUBLIC FUNCTIONS
//
/******************************************************************************/

/******************************************************************************/
// sflow_error_txt()
// Converts SFLOW error to printable text
/******************************************************************************/
char *sflow_error_txt(vtss_rc rc)
{
    switch (rc) {
    case SFLOW_ERROR_ISID:
        return "Invalid Switch ID.";

    case SFLOW_ERROR_NOT_MASTER:
        return "Switch must be master.";

    case SFLOW_ERROR_PORT:
        return "Invalid port number.";

    case SFLOW_ERROR_INSTANCE:
        // sFlow instances are 1-based.
        return "Invalid port instance index. Valid values: [1; " vtss_xstr(SFLOW_INSTANCE_CNT) "].";

    case SFLOW_ERROR_RECEIVER:
        // When configuring counter and flow samplers, 0 is valid. 0 = No receiver. 1-based for valid receivers.
        return "Invalid receiver index. Valid values: [0; " vtss_xstr(SFLOW_RECEIVER_CNT) "].";

    case SFLOW_ERROR_RECEIVER_IDX:
        // When configuring a given receiver, 0 is invalid.
        return "Invalid receiver index. Valid values: [1; " vtss_xstr(SFLOW_RECEIVER_CNT) "].";

    case SFLOW_ERROR_RECEIVER_VS_SAMPLING_DIRECTECTION:
        return "When receiver is set to non-zero, the sampling type must not be 'none'.";

    case SFLOW_ERROR_RECEIVER_OWNER:
        return "Receiver is already owned by someone else.";

    case SFLOW_ERROR_RECEIVER_TIMEOUT:
        return "Invalid receiver timeout. Valid values are [0; " vtss_xstr(SFLOW_RECEIVER_TIMEOUT_MAX) "].";

    case SFLOW_ERROR_RECEIVER_DATAGRAM_SIZE:
        return "Invalid receiver maximum datagram size. Valid values are [" vtss_xstr(SFLOW_RECEIVER_DATAGRAM_SIZE_MIN) "; " vtss_xstr(SFLOW_RECEIVER_DATAGRAM_SIZE_MAX) "].";

    case SFLOW_ERROR_RECEIVER_HOSTNAME:
        return "Invalid receiver IP address or failed DNS lookup of hostname.";

    case SFLOW_ERROR_RECEIVER_SOCKET_CREATE:
        return "Unable to create socket for this IP address or hostname.";

    case SFLOW_ERROR_RECEIVER_CP_INSTANCE:
        return "Receiver already owns another sFlow counter polling instance on the port.";

    case SFLOW_ERROR_RECEIVER_FS_INSTANCE:
        return "Receiver already owns another sFlow flow sampler instance on the port.";

    case SFLOW_ERROR_DATAGRAM_VERSION:
        return "Unsupported datagram version. This agent only supports v. " vtss_xstr(SFLOW_DATAGRAM_VERSION) ".";

    case SFLOW_ERROR_RECEIVER_ACTIVE:
        return "Can't change configuration while receiver is active.";

    case SFLOW_ERROR_AGENT_IP:
        return "Invalid agent IP address.";

    case SFLOW_ERROR_ARGUMENT:
        // Invalid argument. Most arguments to management functions have their
        // own error code, so this error is for some other types of arguments,
        // e.g. cfg pointers that are NULL, etc.
        return "Invalid argument.";

    default:
        return "";
    }
}

/******************************************************************************/
// sflow_mgmt_agent_cfg_get()
/******************************************************************************/
vtss_rc sflow_mgmt_agent_cfg_get(sflow_agent_t *cfg)
{
    // Configuration can only be fetched on master
    if (!msg_switch_is_master()) {
        return SFLOW_ERROR_NOT_MASTER;
    }

    if (cfg == NULL) {
        return SFLOW_ERROR_ARGUMENT;
    }

    SFLOW_CRIT_ENTER();
    *cfg = SFLOW_agent_cfg;
    SFLOW_CRIT_EXIT();

    return VTSS_RC_OK;
}

/******************************************************************************/
// sflow_mgmt_agent_cfg_set()
/******************************************************************************/
vtss_rc sflow_mgmt_agent_cfg_set(sflow_agent_t *cfg)
{
    // Configuration can only be changed on master
    if (!msg_switch_is_master()) {
        return SFLOW_ERROR_NOT_MASTER;
    }

    if (cfg == NULL) {
        return SFLOW_ERROR_ARGUMENT;
    }

#if !defined(VTSS_SW_OPTION_IPV6)
    if (cfg->agent_ip_addr.type != VTSS_IP_TYPE_IPV4) {
        return SFLOW_ERROR_AGENT_IP;
    }
#else
    if (cfg->agent_ip_addr.type != VTSS_IP_TYPE_IPV4 && cfg->agent_ip_addr.type != VTSS_IP_TYPE_IPV6) {
        return SFLOW_ERROR_AGENT_IP;
    }
#endif

    SFLOW_CRIT_ENTER();

    // Only the agent's IP address can be changed.
    SFLOW_agent_cfg.agent_ip_addr = cfg->agent_ip_addr;

    SFLOW_CRIT_EXIT();

    return VTSS_RC_OK;
}

/******************************************************************************/
// sflow_mgmt_rcvr_cfg_get()
/******************************************************************************/
vtss_rc sflow_mgmt_rcvr_cfg_get(u32 rcvr_idx /* 1..N */, sflow_rcvr_t *cfg, sflow_rcvr_info_t *info)
{
    vtss_rc result;

    // Configuration changes only allowed on master
    if ((result = SFLOW_check_isid_port(VTSS_ISID_START, VTSS_PORT_NO_START, FALSE, FALSE)) != VTSS_RC_OK) {
        return result;
    }

    if (rcvr_idx < 1 || rcvr_idx > SFLOW_RECEIVER_CNT) {
        return SFLOW_ERROR_RECEIVER_IDX;
    }

    if (cfg == NULL) {
        return SFLOW_ERROR_ARGUMENT;
    }

    SFLOW_CRIT_ENTER();
    *cfg = SFLOW_rcvr_cfg[rcvr_idx];
    if (info) {
        if (SFLOW_rcvr_state[rcvr_idx].addrinfo == NULL) {
            info->ip_addr.type = VTSS_IP_TYPE_IPV4;
            info->ip_addr.addr.ipv4 = 0;
        } else {
            struct sockaddr *sa = SFLOW_rcvr_state[rcvr_idx].addrinfo->ai_addr;
            if ((info->ip_addr.type = sa->sa_family == AF_INET6 ? VTSS_IP_TYPE_IPV6 : VTSS_IP_TYPE_IPV4) == VTSS_IP_TYPE_IPV4) {
                info->ip_addr.addr.ipv4 = ntohl(((struct sockaddr_in *)sa)->sin_addr.s_addr);
            } else {
                memcpy(info->ip_addr.addr.ipv6.addr, ((struct sockaddr_in6 *)sa)->sin6_addr.__u6_addr.__u6_addr8, sizeof(info->ip_addr.addr.ipv6.addr));
            }
        }
        strcpy(info->ip_addr_str, SFLOW_rcvr_state[rcvr_idx].ip_addr_str);
        info->timeout_left = SFLOW_rcvr_state[rcvr_idx].timeout_left;
    }
    SFLOW_CRIT_EXIT();

    return VTSS_RC_OK;
}

/******************************************************************************/
// sflow_mgmt_rcvr_cfg_set()
/******************************************************************************/
vtss_rc sflow_mgmt_rcvr_cfg_set(u32 rcvr_idx /* 1..N */, sflow_rcvr_t *new_cfg)
{
    vtss_rc      result;
    sflow_rcvr_t *cur_cfg;

    // Configuration changes only allowed on master
    if ((result = SFLOW_check_isid_port(VTSS_ISID_START, VTSS_PORT_NO_START, FALSE, FALSE)) != VTSS_RC_OK) {
        return result;
    }

    if (rcvr_idx < 1 || rcvr_idx > SFLOW_RECEIVER_CNT) {
        return SFLOW_ERROR_RECEIVER_IDX;
    }

    if (new_cfg == NULL) {
        return SFLOW_ERROR_ARGUMENT;
    }

    SFLOW_CRIT_ENTER();

    cur_cfg = &SFLOW_rcvr_cfg[rcvr_idx];
    result = VTSS_RC_OK;

    if (new_cfg->owner[0] == '\0') {
        // Current receiver is attempted released.

        if (cur_cfg->owner[0] == '\0') {
            // Current receiver not configured, and new receiver is releasing. Nothing to do.
            goto do_exit;
        }

        // Release all resources associated with this receiver. We don't care about the remaining parameters in the receiver configuration.
        SFLOW_release_receiver(rcvr_idx);
    } else {
        BOOL send_cfg_to_slaves = FALSE;

        // A (new) receiver is attempting to modify a(n existing) receiver index.
        if (cur_cfg->owner[0] != '\0') {
            // Someone already owns this index. If it's not the same as who owns it now,
            // return an error. We do case-sensitive match. The standard is vague on this
            // matter.
            if (strncmp(cur_cfg->owner, new_cfg->owner, sizeof(cur_cfg->owner) - 1) != 0) {
                result = SFLOW_ERROR_RECEIVER_OWNER;
                goto do_exit;
            }
        }

        if (new_cfg->datagram_version < SFLOW_DATAGRAM_VERSION) {
            // If requested datagram version is > supported datagram version, we must
            // return the supported one, and not an error.
            result = SFLOW_ERROR_DATAGRAM_VERSION;
            goto do_exit;
        }

        if (new_cfg->timeout > SFLOW_RECEIVER_TIMEOUT_MAX) {
            result = SFLOW_ERROR_RECEIVER_TIMEOUT;
            goto do_exit;
        }

        if (new_cfg->max_datagram_size < SFLOW_RECEIVER_DATAGRAM_SIZE_MIN || new_cfg->max_datagram_size > SFLOW_RECEIVER_DATAGRAM_SIZE_MAX) {
            result = SFLOW_ERROR_RECEIVER_DATAGRAM_SIZE;
            goto do_exit;
        }

        if (new_cfg->udp_port == 0) {
            new_cfg->udp_port = SFLOW_RECEIVER_UDP_PORT_DEFAULT;
        }

        // Time to see if the new IP address is identical or compatible (IPv4 vs. IPv6-wise)
        // with the old. This function looks up the hostname and creates a new socket
        // if required.
        // If result == VTSS_RC_OK, we update the configuration with the new and
        // possibly send configuration to the slaves.
        // If result != VTSS_RC_OK, something went wrong, and the existing socket
        // (if any) is kept.
        result = SFLOW_rcvr_hostname_update(rcvr_idx, cur_cfg, new_cfg, &send_cfg_to_slaves);

        if (result != VTSS_RC_OK) {
            goto do_exit;
        }

        // Setting the timeout to 0 is probably a mishap, but we consider it to be
        // valid enough, and simply mean to still keep ownership while disabling
        // the samplers.
        if ((cur_cfg->timeout == 0 && new_cfg->timeout >  0) ||
            (cur_cfg->timeout >  0 && new_cfg->timeout == 0)) {
            send_cfg_to_slaves = TRUE;
        }

        // Everything seems to be OK.
        // Update configuration state.
        *cur_cfg = *new_cfg;
        SFLOW_rcvr_state[rcvr_idx].timeout_left = new_cfg->timeout;
        cur_cfg->owner[sizeof(cur_cfg->owner) - 1] = '\0'; // Make sure it is zero-terminated.

        T_D("Send to slaves: %d", send_cfg_to_slaves);

        if (send_cfg_to_slaves) {
            switch_iter_t sit;
            (void)switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID);
            while (switch_iter_getnext(&sit)) {
                SFLOW_msg_tx_switch_cfg(sit.isid);
            }
        }
    }

do_exit:
    SFLOW_CRIT_EXIT();
    return result;
}

/******************************************************************************/
// sflow_mgmt_flow_sampler_cfg_get()
/******************************************************************************/
vtss_rc sflow_mgmt_flow_sampler_cfg_get(vtss_isid_t isid, vtss_port_no_t port, u16 instance, sflow_fs_t *cfg)
{
    vtss_rc result;

    // Configuration get only allowed by master
    if ((result = SFLOW_check_isid_port(isid, port, FALSE, TRUE)) != VTSS_RC_OK) {
        return result;
    }

    if (instance < 1 || instance > SFLOW_INSTANCE_CNT) {
        return SFLOW_ERROR_INSTANCE;
    }

    if (cfg == NULL) {
        return SFLOW_ERROR_ARGUMENT;
    }

    SFLOW_CRIT_ENTER();
    *cfg = SFLOW_stack_cfg[isid].port[port].fs[instance - 1];
    SFLOW_CRIT_EXIT();
    return VTSS_RC_OK;
}

/******************************************************************************/
// sflow_mgmt_flow_sampler_cfg_set()
/******************************************************************************/
vtss_rc sflow_mgmt_flow_sampler_cfg_set(vtss_isid_t isid, vtss_port_no_t port, u16 instance, sflow_fs_t *cfg)
{
    vtss_rc    result;
    sflow_fs_t *fs;
    u16        inst_iter;
    u32        realizable_sampling_rate;
    BOOL       power2;

    // Configuration changes only allowed by master
    if ((result = SFLOW_check_isid_port(isid, port, FALSE, TRUE)) != VTSS_RC_OK) {
        return result;
    }

    if (instance < 1 || instance > SFLOW_INSTANCE_CNT) {
        return SFLOW_ERROR_INSTANCE;
    }

    if (cfg == NULL) {
        return SFLOW_ERROR_ARGUMENT;
    }

    if (cfg->receiver > SFLOW_RECEIVER_CNT) {
        return SFLOW_ERROR_RECEIVER;
    }

    // Any sampling rate is allowed. Standard says that we must adjust it
    // to a valid value.

    if (cfg->max_header_size < SFLOW_FLOW_HEADER_SIZE_MIN) {
        // Auto-adjust.
        cfg->max_header_size = SFLOW_FLOW_HEADER_SIZE_MIN;
    }

    if (cfg->max_header_size > SFLOW_FLOW_HEADER_SIZE_MAX) {
        // Auto-adjust.
        cfg->max_header_size = SFLOW_FLOW_HEADER_SIZE_MAX;
    }

    if (cfg->receiver != 0 && cfg->type == VTSS_SFLOW_TYPE_NONE) {
        return SFLOW_ERROR_RECEIVER_VS_SAMPLING_DIRECTECTION;
    }

    SFLOW_CRIT_ENTER();

    fs = &SFLOW_stack_cfg[isid].port[port].fs[0];

    // Check that the same receiver is not enabled on multiple instances
    // on the same port. If they were allowed, that would prevent other
    // receivers from being able to obtain the instance. Also, it would
    // cause us to potentially send more samples than required to the same
    // receiver. There is nothing that prevents the adminstrator from setting
    // up multiple receivers for the same host, so that's a work-around for
    // this limitation, if the administrator really wants it.
    if (cfg->receiver != 0) {
        for (inst_iter = 0; inst_iter < SFLOW_INSTANCE_CNT; inst_iter++) {
            if (inst_iter != instance - 1) {
                if (fs[inst_iter].receiver == cfg->receiver) {
                    result = SFLOW_ERROR_RECEIVER_FS_INSTANCE;
                    goto do_exit;
                }
            }
        }
    }

// We only support powers of two if the number of possible receivers is greater than
// one, because we do software-based sub-sampling in that case.
#if SFLOW_RECEIVER_CNT > 1
    power2 = TRUE;
#else
    power2 = FALSE;
#endif
    (void)vtss_sflow_sampling_rate_convert(NULL, power2, cfg->sampling_rate, &realizable_sampling_rate);
    cfg->sampling_rate = realizable_sampling_rate;

    if (memcmp(&fs[instance - 1], cfg, sizeof(*cfg)) != 0) {
        // Configuration change.
        memcpy(&fs[instance - 1], cfg, sizeof(fs[instance - 1]));
        SFLOW_stack_statistics[isid].port[port].fs_rx[instance - 1] = 0;
        SFLOW_stack_statistics[isid].port[port].fs_tx[instance - 1] = 0;

        if (cfg->receiver == 0) {
            // Reset the remaining fields to defaults when a receiver
            // releases the flow sampler.
            SFLOW_fs_cfg_init(isid, port, instance);
        }
        SFLOW_msg_tx_switch_cfg(isid);
    }

    result = VTSS_RC_OK;

do_exit:
    SFLOW_CRIT_EXIT();
    return result;
}

/******************************************************************************/
// sflow_mgmt_counter_poller_cfg_get()
/******************************************************************************/
vtss_rc sflow_mgmt_counter_poller_cfg_get(vtss_isid_t isid, vtss_port_no_t port, u16 instance, sflow_cp_t *cfg)
{
    vtss_rc result;

    // Configuration get only allowed by master
    if ((result = SFLOW_check_isid_port(isid, port, FALSE, TRUE)) != VTSS_RC_OK) {
        return result;
    }

    if (instance < 1 || instance > SFLOW_INSTANCE_CNT) {
        return SFLOW_ERROR_INSTANCE;
    }

    if (cfg == NULL) {
        return SFLOW_ERROR_ARGUMENT;
    }

    SFLOW_CRIT_ENTER();
    *cfg = SFLOW_stack_cfg[isid].port[port].cp[instance - 1];
    SFLOW_CRIT_EXIT();
    return VTSS_RC_OK;
}

/******************************************************************************/
// sflow_mgmt_counter_poller_cfg_set()
/******************************************************************************/
vtss_rc sflow_mgmt_counter_poller_cfg_set(vtss_isid_t isid, vtss_port_no_t port, u16 instance, sflow_cp_t *cfg)
{
    vtss_rc    result;
    sflow_cp_t *cp;
    u16        inst_iter;

    // Configuration changes only allowed by master
    if ((result = SFLOW_check_isid_port(isid, port, FALSE, TRUE)) != VTSS_RC_OK) {
        return result;
    }

    if (instance < 1 || instance > SFLOW_INSTANCE_CNT) {
        return SFLOW_ERROR_INSTANCE;
    }

    if (cfg == NULL) {
        return SFLOW_ERROR_ARGUMENT;
    }

    if (cfg->receiver > SFLOW_RECEIVER_CNT) {
        return SFLOW_ERROR_RECEIVER;
    }

    if (cfg->interval > SFLOW_POLLING_INTERVAL_MAX) {
        // Auto-adjust (according to standard).
        cfg->interval = SFLOW_POLLING_INTERVAL_MAX;
    }

    SFLOW_CRIT_ENTER();

    cp = &SFLOW_stack_cfg[isid].port[port].cp[0];

    // Check that the same receiver is not enabled on multiple instances
    // on the same port. If they were allowed, that would prevent other
    // receivers from being able to obtain the instance. Also, it would
    // cause us to potentially send more samples than required to the same
    // receiver. There is nothing that prevents the adminstrator from setting
    // up multiple receivers for the same host, so that's a work-around for
    // this limitation, if the administrator really wants it.
    if (cfg->receiver != 0) {
        for (inst_iter = 0; inst_iter < SFLOW_INSTANCE_CNT; inst_iter++) {
            if (inst_iter != instance - 1) {
                if (cp[inst_iter].receiver == cfg->receiver) {
                    result = SFLOW_ERROR_RECEIVER_CP_INSTANCE;
                    goto do_exit;
                }
            }
        }
    }

    if (memcmp(&cp[instance - 1], cfg, sizeof(*cfg)) != 0) {
        // Configuration change.
        memcpy(&cp[instance - 1], cfg, sizeof(cp[instance - 1]));
        SFLOW_stack_statistics[isid].port[port].cp[instance - 1] = 0;
        SFLOW_msg_tx_switch_cfg(isid);
    }

    result = VTSS_RC_OK;

do_exit:
    SFLOW_CRIT_EXIT();
    return result;
}

/******************************************************************************/
// sflow_mgmt_rcvr_statistics_get()
/******************************************************************************/
vtss_rc sflow_mgmt_rcvr_statistics_get(u32 rcvr_idx, sflow_rcvr_statistics_t *statistics, BOOL clear)
{
    vtss_rc result;

    // Statistics reading only allowed by master
    if ((result = SFLOW_check_isid_port(VTSS_ISID_START, VTSS_PORT_NO_START, FALSE, FALSE)) != VTSS_RC_OK) {
        return result;
    }

    if (rcvr_idx < 1 || rcvr_idx > SFLOW_RECEIVER_CNT) {
        return SFLOW_ERROR_RECEIVER_IDX;
    }

    if (clear == FALSE && statistics == NULL) {
        return SFLOW_ERROR_ARGUMENT;
    }

    SFLOW_CRIT_ENTER();
    if (clear) {
        memset(&SFLOW_rcvr_statistics[rcvr_idx], 0, sizeof(SFLOW_rcvr_statistics[rcvr_idx]));
    }
    if (statistics) {
        *statistics = SFLOW_rcvr_statistics[rcvr_idx];
    }
    SFLOW_CRIT_EXIT();

    return VTSS_RC_OK;
}

/******************************************************************************/
// sflow_mgmt_switch_statistics_get()
/******************************************************************************/
vtss_rc sflow_mgmt_switch_statistics_get(vtss_isid_t isid, sflow_switch_statistics_t *statistics)
{
    vtss_rc result;

    // Statistics reading only allowed by master
    if ((result = SFLOW_check_isid_port(isid, VTSS_PORT_NO_START, FALSE, FALSE)) != VTSS_RC_OK) {
        return result;
    }

    if (statistics == NULL) {
        return SFLOW_ERROR_ARGUMENT;
    }

    SFLOW_CRIT_ENTER();
    *statistics = SFLOW_stack_statistics[isid];
    SFLOW_CRIT_EXIT();

    return VTSS_RC_OK;
}

/******************************************************************************/
// sflow_mgmt_instance_statistics_clear()
/******************************************************************************/
vtss_rc sflow_mgmt_instance_statistics_clear(vtss_isid_t isid, vtss_port_no_t port, u16 instance)
{
    vtss_rc result;

    // Statistics clearing only allowed by master
    if ((result = SFLOW_check_isid_port(isid, port, FALSE, TRUE)) != VTSS_RC_OK) {
        return result;
    }

    if (instance < 1 || instance > SFLOW_INSTANCE_CNT) {
        return SFLOW_ERROR_INSTANCE;
    }

    SFLOW_CRIT_ENTER();
    SFLOW_stack_statistics[isid].port[port].fs_rx[instance - 1] = 0;
    SFLOW_stack_statistics[isid].port[port].fs_tx[instance - 1] = 0;
    SFLOW_stack_statistics[isid].port[port].cp   [instance - 1] = 0;
    SFLOW_CRIT_EXIT();

    return VTSS_RC_OK;
}

/******************************************************************************/
// sflow_init()
/******************************************************************************/
vtss_rc sflow_init(vtss_init_data_t *data)
{
    u32 i;

    /*lint --e{454, 456} */
    switch (data->cmd) {
    case INIT_CMD_INIT:
#ifdef VTSS_SW_OPTION_VCLI
        sflow_cli_init();
#endif

        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);

        if (!vtss_bip_buffer_init(&SFLOW_master_bip, SFLOW_MASTER_BIP_BUFFER_SIZE_BYTES)) {
            T_E("Unable to create Master BIP buffer");
        }

        if (!vtss_bip_buffer_init(&SFLOW_local_bip, SFLOW_LOCAL_BIP_BUFFER_SIZE_BYTES)) {
            T_E("Unabled to create Local BIP buffer");
        }

        critd_init(&SFLOW_crit,
                   "sFlow",
                   VTSS_MODULE_ID_SFLOW,
                   VTSS_TRACE_MODULE_ID,
                   CRITD_TYPE_MUTEX);

        critd_init(&SFLOW_bip_crit,
                   "sFlow BIP buffers",
                   VTSS_MODULE_ID_SFLOW,
                   VTSS_TRACE_MODULE_ID,
                   CRITD_TYPE_MUTEX);

        // Create the sFlow thread with lower than normal priority,
        // so that we don't affect other modules by sending frames
        // to a receiver. According to the sFlow spec, it's perfectly
        // alright to miss updates.
        cyg_thread_create(THREAD_BELOW_NORMAL_PRIO,
                          SFLOW_thread,
                          0,
                          "sFlow",
                          SFLOW_thread_stack,
                          sizeof(SFLOW_thread_stack),
                          &SFLOW_thread_handle,
                          &SFLOW_thread_block);

        cyg_thread_resume(SFLOW_thread_handle);
        cyg_flag_init(&SFLOW_wakeup_thread_flag);
        memset(&SFLOW_stack_cfg[VTSS_ISID_LOCAL], 0, sizeof(SFLOW_stack_cfg[VTSS_ISID_LOCAL]));

        // Clear socket handles without closing them (because they aren't open).
        SFLOW_rcvr_state_reset(0, FALSE); // 0 == clear all. FALSE == don't attempt to call close().

        SFLOW_rcvr_cfg_init(0); // 0 == all receivers.

        // Reset the message buffers that gather counter and flow samples
        // before sending them to the master.
        for (i = 0; i < ARRSZ(SFLOW_local_samples); i++) {
            SFLOW_msg_reset_sample_msg(&SFLOW_local_samples[i]);
        }
        break;

    case INIT_CMD_START:
        // Register for stack messages
        SFLOW_msg_init();

        // Register for sFlow packets.
        SFLOW_packet_register();

#if !defined(VTSS_ARCH_JAGUAR_1) && !defined(VTSS_ARCH_LUTON26) && !defined(VTSS_ARCH_SERVAL)
        // Test that the API returns consistent realizable sampling rates on new architectures.
        SFLOW_sampling_rate_api_test();
#endif

        // Register for local port changes. We need to insert various port properties in the UDP datagrams.
        if (port_change_register(VTSS_MODULE_ID_SFLOW, SFLOW_port_change_callback) != VTSS_RC_OK) {
            T_E("Unable to register for port changes");
        }
        break;

    case INIT_CMD_CONF_DEF:
        if (data->isid == VTSS_ISID_GLOBAL) {
            // Reset configuration.
            // The sFlow module does not have persisted configuration, but we should stop it anyway.
            // Since this function is called for every isid, we only react on the global one.
            SFLOW_CRIT_ENTER();
            SFLOW_release_receiver(0); // 0 => release all
            SFLOW_CRIT_EXIT();
        }
        break;

    case INIT_CMD_MASTER_UP:
        SFLOW_CRIT_ENTER();
        // We don't persist any configuration (mainly due to a timeout defined by sFlow receivers),
        // so let's reset our whole stack configuration upon master up. SFLOW_stack_cfg[0] is managed
        // by the local switch-part of this code, so we don't reset that.
        memset(&SFLOW_stack_cfg[VTSS_ISID_LOCAL + 1], 0, sizeof(SFLOW_stack_cfg) - sizeof(SFLOW_stack_cfg[0]));
        SFLOW_fs_cfg_init(VTSS_ISID_GLOBAL, VTSS_PORT_NO_NONE, 0); // Reset the whole stack's flow sampler configuration.
        cyg_flag_setbits(&SFLOW_wakeup_thread_flag, SFLOW_THREAD_FLAG_MASTER_UP);
        SFLOW_CRIT_EXIT();

        SFLOW_BIP_CRIT_ENTER();
        // The drop counter is reset only when the agent resets, which is upon boot and upon master change.
        memset(&SFLOW_local_info_exchange[0], 0, sizeof(SFLOW_local_info_exchange));
        SFLOW_BIP_CRIT_EXIT();
        break;

    case INIT_CMD_MASTER_DOWN:
        // Clear socket handles while closing them.
        SFLOW_CRIT_ENTER();
        SFLOW_release_receiver(0); // 0 => release all.
        SFLOW_agent_cfg.agent_ip_addr.type = SFLOW_AGENT_IP_TYPE_DEFAULT;
        SFLOW_agent_cfg.agent_ip_addr.addr.ipv4 = SFLOW_AGENT_IP_ADDR_DEFAULT;
        SFLOW_CRIT_EXIT();
        break;

    case INIT_CMD_SWITCH_ADD:
        SFLOW_CRIT_ENTER();
        SFLOW_msg_tx_switch_cfg(data->isid);
        SFLOW_CRIT_EXIT();
        break;

    default:
        break;
    }
    return VTSS_RC_OK;
}

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/

