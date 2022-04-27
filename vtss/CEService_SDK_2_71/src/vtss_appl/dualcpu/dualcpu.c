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

#include "main.h"
#include "dualcpu.h"
#include "rpc_api.h"

cyg_flag_t cpu_flags;

static cyg_interrupt    cpu_intobject;
static cyg_handle_t     cpu_inthandle;

static cyg_handle_t     cpu_thread_handle;
static cyg_thread       cpu_thread_block;
static char             cpu_thread_stack[THREAD_DEFAULT_STACK_SIZE];

#define CRITICAL_ON()	cyg_scheduler_lock()
#define CRITICAL_OFF()	cyg_scheduler_unlock()

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_DUALCPU

rpc_msg_t *rpc_queue, *rpc_pending;

#define SW_INT VTSS_BIT(0)

#define CPU_BUFSIZE 1024

static u8 *toICPU, *toECPU;

#ifdef DUALCPU_MASTER
#define CPU_INT    CYGNUM_HAL_INTERRUPT_EXT1
#define TX_BUFFER 0             /* iCPU */
#define RX_BUFFER 1             /* eCPU */
#define toOtherCPU toICPU
#define toThisCPU  toECPU
#else
#define CPU_INT    CYGNUM_HAL_INTERRUPT_SW0
#define TX_BUFFER 1             /* eCPU */
#define RX_BUFFER 0             /* iCPU */
#define toOtherCPU toECPU
#define toThisCPU  toICPU
#endif

static u32 mailbox(void)
{
#ifdef DUALCPU_MASTER
    return RMT_RD(VTSS_DEVCPU_GCB_SW_REGS_MAILBOX);
#else
    return VTSS_DEVCPU_GCB_SW_REGS_MAILBOX;
#endif
}

static BOOL have_output_buffer(void)
{
#ifdef DUALCPU_MASTER
    return (RMT_RD(VTSS_DEVCPU_GCB_SW_REGS_MAILBOX) & VTSS_BIT(TX_BUFFER)) != 0; /* 1 => eCPU owner */
#else
    return (VTSS_DEVCPU_GCB_SW_REGS_MAILBOX & VTSS_BIT(TX_BUFFER)) == 0; /* 0 => iCPU owner */
#endif
}

static void release_output_buffer(void)
{
#ifdef DUALCPU_MASTER
    RMT_WR(VTSS_DEVCPU_GCB_SW_REGS_MAILBOX_CLR, VTSS_BIT(TX_BUFFER)); /* 0 => iCPU owner */
#else
    VTSS_DEVCPU_GCB_SW_REGS_MAILBOX_SET = VTSS_BIT(TX_BUFFER); /* 1 => eCPU owner */
#endif
}

static BOOL have_input_buffer(void)
{
#ifdef DUALCPU_MASTER
    return (RMT_RD(VTSS_DEVCPU_GCB_SW_REGS_MAILBOX) & VTSS_BIT(RX_BUFFER)) != 0; /* 1 => eCPU owner */
#else
    return (VTSS_DEVCPU_GCB_SW_REGS_MAILBOX & VTSS_BIT(RX_BUFFER)) == 0; /* 0 => iCPU owner */
#endif
}

static void release_input_buffer(void)
{
#ifdef DUALCPU_MASTER
    RMT_WR(VTSS_DEVCPU_GCB_SW_REGS_MAILBOX_CLR, VTSS_BIT(RX_BUFFER)); /* 0 => iCPU owner */
#else
    VTSS_DEVCPU_GCB_SW_REGS_MAILBOX_SET = VTSS_BIT(RX_BUFFER); /* 1 => eCPU owner */
#endif
}

static void ring_doorbell(void)
{
#ifdef DUALCPU_MASTER
    RMT_SET(VTSS_DEVCPU_GCB_SW_REGS_SW_INTR, VTSS_BIT(0));
#else
    VTSS_DEVCPU_GCB_SW_REGS_SW_INTR = VTSS_BIT(1);
#endif
}

static cyg_uint32 cpu_isr(cyg_vector_t vector, cyg_addrword_t data)
{
#ifdef DUALCPU_MASTER
    RMT_WR(VTSS_ICPU_CFG_INTR_INTR, VTSS_BIT(CYGNUM_HAL_INTERRUPT_SW1));
#endif
    cyg_drv_interrupt_acknowledge(vector);     // Tell eCos to allow further interrupt processing
    return (CYG_ISR_HANDLED|CYG_ISR_CALL_DSR); // Call the DSR
}

u32 dualcpu_buffer_read(u32 *buffer)
{
    u32 val;
#ifdef DUALCPU_MASTER
    val = remote_read((void*)buffer);
#else
    val = *buffer;
#endif
    IOTRACE("read %p = 0x%08lx\n", buffer, val);
    return val;
}

void dualcpu_buffer_read_n(u32 *dst, u32 *src, size_t len)
{
#ifdef DUALCPU_MASTER
    int i;
    len = (len + 3) / 4;        /* To dwords */
    for(i = 0; i < len; i++)
        *dst++ = remote_read((void*)src++);
#else
    memcpy(dst, src, len);
#endif
}

void dualcpu_buffer_write_n(u32 *dst, u32 *src, size_t len)
{
#ifdef DUALCPU_MASTER
    int i;
    len = (len + 3) / 4;        /* To dwords */
    for(i = 0; i < len; i++)
        remote_write((void*)dst++, *src++);
#else
    memcpy(dst, src, len);
#endif
}

void dualcpu_buffer_write(u32 *buffer, u32 val)
{
    IOTRACE("write %p = 0x%08lx\n", buffer, val);
#ifdef DUALCPU_MASTER
    remote_write((void*)buffer, val);
#else
    *buffer = val;
#endif
}

rpc_msg_t *dualcpu_receive(void *buffer)
{
    rpc_msg_t *msg = NULL;
    u32 *in = buffer;
    u32 length;
#ifndef DUALCPU_MASTER
    HAL_DCACHE_INVALIDATE(buffer, 5*sizeof(u32));
#endif
    length = RPC_NTOHL(dualcpu_buffer_read(&in[0]));
    if(length) {
        length &= VTSS_BITMASK(16);
        IOTRACE("Got %ld bytes input\n", length);
        msg = rpc_message_alloc();
        if(msg) {
            msg->length = length;
            msg->id = RPC_NTOHL(dualcpu_buffer_read(&in[1]));
            msg->rc = RPC_NTOHL(dualcpu_buffer_read(&in[2]));
            msg->type = RPC_NTOHL(dualcpu_buffer_read(&in[3]));
            msg->seq = RPC_NTOHL(dualcpu_buffer_read(&in[4]));
            IOTRACE("Msg %d rc %ld type %0x seq %ld\n", msg->id, msg->rc, msg->type, msg->seq);
#ifndef DUALCPU_MASTER
            HAL_DCACHE_INVALIDATE(&in[5], length);
#endif
            dualcpu_buffer_read_n((u32*)msg->data, &in[5], length);
        }
        dualcpu_buffer_write(&in[0], 0); /* Drained */
#ifndef DUALCPU_MASTER
        HAL_DCACHE_STORE(buffer, HAL_DCACHE_LINE_SIZE);
#endif
    }
    return msg;
}

/*
 * Enqueue to the end
 */
void dualcpu_queue(rpc_msg_t **queue, rpc_msg_t *msg)
{
    rpc_msg_t *tail;
    msg->next = NULL;
    CRITICAL_ON();
    tail = *queue;
    if(tail) {
        while(tail->next)
            tail = tail->next;
        tail->next = msg;
    } else {
        *queue = msg;
    }
    CRITICAL_OFF();
}

rpc_msg_t *dualcpu_find_req(u16 id, u32 seq)
{
    rpc_msg_t *req = NULL, *prev, *tail;
    CRITICAL_ON();
    for(prev = NULL, tail = rpc_pending; tail != NULL; tail = tail->next) {
        if(tail->id == id && tail->seq == seq) {
            req = tail;
            if(prev) {
                // Middle of a list
                prev->next = req->next;
            } else {
                // Start of list
                rpc_pending = req->next;
            }
            break;
        }
        prev = tail;
    }
    CRITICAL_OFF();
    return req;
}

void dualcpu_age_queue(rpc_msg_t **queue)
{
    u32 now = cyg_current_time();
    CRITICAL_ON();
    if(*queue) {
        rpc_msg_t *prev = NULL, *req = *queue;
        while(req) {
            IOTRACE("Ageing req, tmo %lu time %lu\n", req->timeout, now);
            if(now > req->timeout) {
                rpc_msg_t *next = req->next; /* Current chain */
                IOERROR("Timeout cmd %d type %d seq %ld timeout %ld now %ld\n", 
                        req->id, req->type, req->seq, req->timeout, now);
                if(prev)
                    prev->next = next;
                else
                    *queue = next;
                req->next = NULL;
                req->rc = VTSS_RC_INCOMPLETE;
                if(req->syncro)
                    cyg_semaphore_post((cyg_sem_t*)req->syncro);
                req = next;
            } else {
                prev = req;
                req = req->next;
            }
        }
    }
    CRITICAL_OFF();
}

void dualcpu_transmit(rpc_msg_t *msg)
{
    static u32 seq;
    u32 *out = (void*) toOtherCPU;
    dualcpu_buffer_write(&out[0], RPC_HTONL((VTSS_BIT(31) | msg->length)));
    dualcpu_buffer_write(&out[1], RPC_HTONL(msg->id));
    dualcpu_buffer_write(&out[2], RPC_HTONL(msg->rc));
    dualcpu_buffer_write(&out[3], RPC_HTONL(msg->type));
    if(msg->type != MSG_TYPE_RSP)
        msg->seq = seq++;       /* New sequence number unless response message */
    dualcpu_buffer_write(&out[4], RPC_HTONL(msg->seq));
    dualcpu_buffer_write_n(&out[5], (u32*)msg->data, msg->length);
    /* Flush cache */
#ifndef DUALCPU_MASTER
    HAL_DCACHE_STORE(toOtherCPU, msg->length + 5*sizeof(u32));
#endif
    release_output_buffer();
    ring_doorbell();
    if(msg->type == MSG_TYPE_REQ) {
        dualcpu_queue(&rpc_pending, msg);
    } else {
        msg->rc = VTSS_RC_OK;       /* Tx OK */
        if(msg->syncro)
            cyg_semaphore_post((cyg_sem_t*)msg->syncro);
    }
}

void dualcpu_send(rpc_msg_t *msg)
{
    CRITICAL_ON();
    msg->timeout = cyg_current_time() + RPC_TIMEOUT * VTSS_OS_MSEC2TICK(1000);
    IOTRACE("Start req @%lld, timeout @%ld\n", cyg_current_time(), msg->timeout);
    if(have_output_buffer()) {
        IOTRACE("Have buffer, xmit\n");
        dualcpu_transmit(msg);
    } else {
        dualcpu_queue(&rpc_queue, msg);
        IOTRACE("No buffer, queue\n");
    }
    CRITICAL_OFF();
}

static void cpu_dsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    cyg_flag_setbits((cyg_flag_t*) data, SW_INT);
}

static void setup_irq(void)
{
    /* GPIO7 => EXT1 irq mode */
    vtss_gpio_mode_set(NULL, 0, 7, VTSS_GPIO_ALT_0);
    /* Edge mode */
    //hal_interrupt_configure(CYGNUM_HAL_INTERRUPT_EXT1, TRUE, FALSE);
#ifdef DUALCPU_MASTER
    /* EXT1 IRQ input, ext1 */
    VTSS_ICPU_CFG_INTR_EXT_IRQ1_INTR_CFG =
        VTSS_F_ICPU_CFG_INTR_EXT_IRQ1_INTR_CFG_EXT_IRQ1_INTR_SEL(0); /* 0 = MIPS INT 0 */
#else
    /* EXT1 IRQ output */
    VTSS_ICPU_CFG_INTR_EXT_IRQ1_INTR_CFG =
        VTSS_F_ICPU_CFG_INTR_EXT_IRQ1_INTR_CFG_EXT_IRQ1_INTR_DIR;
    /* Ena EXT1 */
    VTSS_ICPU_CFG_INTR_EXT_IRQ1_ENA = VTSS_BIT(0);
    /* SW1 to EXT1 */
    VTSS_ICPU_CFG_INTR_SW1_INTR_CFG =
        VTSS_F_ICPU_CFG_INTR_SW1_INTR_CFG_SW1_INTR_SEL(3);
    cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_SW1); /* Need to unmask SW1 (=> EXT1) */
#endif
}

#ifdef DUALCPU_MASTER
/* 
 * Consume 'evt_port_state' event (input side)
 */
void evt_port_state_receive(const vtss_port_no_t  port_no , const port_info_t * info )
{
    printf("Port(%ld): link %d, speed %d, fdx %d\n",
           port_no+1, info->link, info->speed, info->fdx);
}
#else
void port_change_callback(vtss_port_no_t port_no, port_info_t *info)
{
    (void) rpc_evt_port_state(port_no, info);
}
#endif

static void cpu_thread(cyg_addrword_t data)
{

    setup_irq();

#ifdef DUALCPU_MASTER
    u32 val;

    IOTRACE("Remote unit: 0x%08lx\n", RMT_RD(VTSS_DEVCPU_GCB_CHIP_REGS_CHIP_ID));

    RMT_WR(VTSS_DEVCPU_GCB_SW_REGS_MAILBOX, 0);

    /* Assert reset - NB: Documentation *wrong* must write 0b'0' */
    IOTRACE("CTRL_RESET = 0x%08lx\n", RMT_RD(VTSS_ICPU_CFG_CPU_SYSTEM_CTRL_RESET));
    RMT_CLR(VTSS_ICPU_CFG_CPU_SYSTEM_CTRL_RESET, 
            VTSS_F_ICPU_CFG_CPU_SYSTEM_CTRL_RESET_CPU_RELEASE);

    /* Set PLL CPU Clock divider for 416 Mhz CPU speed */
    RMT_WRM(VTSS_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG0,
            VTSS_F_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG0_CPU_CLK_DIV(6),
            VTSS_M_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG0_CPU_CLK_DIV);
    
    dualcpu_ddr_init();

    /* Clear boot mode */
    RMT_CLR(VTSS_ICPU_CFG_CPU_SYSTEM_CTRL_GENERAL_CTRL, 
            VTSS_F_ICPU_CFG_CPU_SYSTEM_CTRL_GENERAL_CTRL_BOOT_MODE_ENA);
    val = RMT_RD(VTSS_ICPU_CFG_CPU_SYSTEM_CTRL_GENERAL_CTRL);
    IOTRACE("GENERAL_CTRL = 0x%08lx\n", val);

    /* Leave boot interface alone */
    RMT_SET(VTSS_ICPU_CFG_CPU_SYSTEM_CTRL_GENERAL_CTRL, VTSS_BIT(3));
    val = RMT_RD(VTSS_ICPU_CFG_CPU_SYSTEM_CTRL_GENERAL_CTRL);
    IOTRACE("GENERAL_CTRL = 0x%08lx\n", val);

    /* Release reset - NB: Documentation *wrong* must write 0b'1' */
    IOTRACE("CTRL_RESET = 0x%08lx\n", RMT_RD(VTSS_ICPU_CFG_CPU_SYSTEM_CTRL_RESET));
    RMT_SET(VTSS_ICPU_CFG_CPU_SYSTEM_CTRL_RESET, 
            VTSS_F_ICPU_CFG_CPU_SYSTEM_CTRL_RESET_CPU_RELEASE);
    IOTRACE("CTRL_RESET = 0x%08lx\n", RMT_RD(VTSS_ICPU_CFG_CPU_SYSTEM_CTRL_RESET));
#endif

    cyg_drv_interrupt_create(CPU_INT, /* Interrupt Vector */
                             0,       /* Interrupt Priority */
                             (cyg_addrword_t)&cpu_flags,
                             cpu_isr,
                             cpu_dsr,
                             &cpu_inthandle,
                             &cpu_intobject);
    cyg_drv_interrupt_attach(cpu_inthandle);
    cyg_drv_interrupt_unmask(CPU_INT);

#ifdef DUALCPU_MASTER
    while((val = mailbox()) == 0) {
        IOTRACE("%s: Wait for mailbox\n", __FUNCTION__);
        cyg_thread_delay(100);
    }

    toECPU = (void*) val;
    toICPU = toECPU + CPU_BUFSIZE;
    IOTRACE("%s: Mailbox = %p\n", __FUNCTION__, toECPU);

    /* Clear mailbox to ack */
    RMT_WR(VTSS_DEVCPU_GCB_SW_REGS_MAILBOX, 0);

    /* Wait for output buffer to be released */
    while(!have_output_buffer()) {
        IOTRACE("%s: Wait for output buffer\n", __FUNCTION__);
        cyg_thread_delay(100);
    }

    IOTRACE("%s: Got output buffer, operational\n", __FUNCTION__);

#else
    toECPU = VTSS_MALLOC(2*CPU_BUFSIZE);
    VTSS_ASSERT(toECPU != NULL);
    toICPU = toECPU + CPU_BUFSIZE;

    /* Setup mailbox for ref */
    VTSS_DEVCPU_GCB_SW_REGS_MAILBOX = VTSS_OS_VIRT_TO_PHYS(toECPU);

    while(mailbox() != 0) {
        IOTRACE("%s: Wait for mailbox\n", __FUNCTION__);
        cyg_thread_delay(10);
    }

    /* Initially, the iCPU have both buffers, release the input buffer for eCPU output */
    release_input_buffer();
    IOTRACE("%s: Released input buffer for eCPU output\n", __FUNCTION__);

    /* Port change callback */
    (void) port_change_register(VTSS_MODULE_ID_DUALCPU, port_change_callback);

#endif // DUALCPU_MASTER
    ring_doorbell();

    while(1) {
        cyg_tick_count_t wakeup = cyg_current_time() + VTSS_OS_MSEC2TICK(1000);
        cyg_flag_value_t flags;
        while((flags = cyg_flag_timed_wait(&cpu_flags, 0xffff, 
                                           CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR, wakeup))) {
            IOTRACE("Got flags %d\n", flags);
#ifdef DUALCPU_MASTER
            if(have_input_buffer()) {
                rpc_msg_t *msg = dualcpu_receive(toThisCPU);
                release_input_buffer();
                ring_doorbell();
                IOTRACE("Received cmd %d type %d seq %ld\n", msg->id, msg->type, msg->seq);
                switch(msg->type) {
                case MSG_TYPE_RSP: {
                    rpc_msg_t *req;
                    if((req = dualcpu_find_req(msg->id, msg->seq))) {
                        req->next = NULL;
                        req->rsp = msg;
                        req->rc = VTSS_RC_OK;
                        if(req->syncro)
                            cyg_semaphore_post((cyg_sem_t*)req->syncro);
                    } else {
                        IOTRACE("Unable to match response to request, disposing\n");
                        rpc_message_dispose(msg);
                    }
                    break;
                }

                case MSG_TYPE_EVT:
                    rpc_event_receive(msg);
                    break;

                default:
                    IOERROR("Invalid message received: %d\n", msg->type);
                    rpc_message_dispose(msg);
                }
            }
#else
            if(have_input_buffer()) {
                rpc_msg_t *msg = dualcpu_receive(toThisCPU);
                release_input_buffer();
                ring_doorbell();
                if(msg && msg->type == MSG_TYPE_REQ) {
                    rpc_message_dispatch(msg);
                    if(msg->type == MSG_TYPE_RSP)
                        dualcpu_send(msg);
                    else
                        rpc_message_dispose(msg);
                } else {
                    IOTRACE("Unsupported message type %d, disposing\n", msg->type);
                    rpc_message_dispose(msg);
                }
            }
#endif
            if(have_output_buffer() && rpc_queue) {
                rpc_msg_t *req;
                CRITICAL_ON();
                req = rpc_queue;
                rpc_queue = req->next;
                CRITICAL_OFF();
                IOTRACE("Restarted XMIT: cmd %d type %d seq %ld\n", req->id, req->type, req->seq);
                dualcpu_transmit(req);
            }
        }
        /* Age pending requests */
        dualcpu_age_queue(&rpc_queue);
        dualcpu_age_queue(&rpc_pending);
    }
}

static void dualcpu_setup_hw(void)
{
#ifdef DUALCPU_MASTER
    /* Enable PI master for primary device */
    VTSS_ICPU_CFG_PI_MST_PI_MST_CFG = 0x3f;
    VTSS_ICPU_CFG_PI_MST_PI_MST_CTRL(0) = 0x000200A0;
    VTSS_ICPU_CFG_CPU_SYSTEM_CTRL_GENERAL_CTRL |= 
        VTSS_F_ICPU_CFG_CPU_SYSTEM_CTRL_GENERAL_CTRL_IF_MASTER_PI_ENA;

    /* Initialize second device for '16-bit ndone' mode */
    RMT_WR(VTSS_DEVCPU_PI_PI_PI_MODE, 0x18181818);
    RMT_WR(VTSS_DEVCPU_PI_PI_PI_MODE, 0x18181818);

    /* Setup PI for primary device accordingly */
    VTSS_ICPU_CFG_PI_MST_PI_MST_CFG = 0x22;
    VTSS_ICPU_CFG_PI_MST_PI_MST_CTRL(0) = 0x00C200B3;
#endif
}

/* Initialize module */
vtss_rc dualcpu_init(vtss_init_data_t *data)
{
    switch (data->cmd) {
    case INIT_CMD_INIT:
        cyg_flag_init(&cpu_flags);
        cyg_thread_create(THREAD_HIGHEST_PRIO, 
                          cpu_thread, 
                          0, 
                          "DualCpu Thread", 
                          cpu_thread_stack, 
                          sizeof(cpu_thread_stack),
                          &cpu_thread_handle,
                          &cpu_thread_block);
        dualcpu_setup_hw();
        break;
    case INIT_CMD_START:
        break;
    case INIT_CMD_CONF_DEF:
        break;
    case INIT_CMD_MASTER_UP:
        cyg_thread_resume(cpu_thread_handle);
        break;
    case INIT_CMD_MASTER_DOWN:
        break;
    case INIT_CMD_SWITCH_ADD:
        break;
    case INIT_CMD_SWITCH_DEL:
        break;
    default:
        break;
    }

    return VTSS_OK;
}

/*
 * RPC interfaces
 */

#define RPC_BUFSIZE 256

rpc_msg_t *rpc_message_alloc(void)
{
    rpc_msg_t *msg;
    if((msg = VTSS_MALLOC(sizeof(*msg) + RPC_BUFSIZE))) {
        memset(msg, 0, sizeof(*msg));
        rpc_msg_init(msg, (void*) &msg[1], RPC_BUFSIZE);
    }
    return msg;
}

void rpc_message_dispose(rpc_msg_t *msg)
{
    VTSS_FREE(msg);
}

vtss_rc rpc_message_exec(rpc_msg_t *msg)
{
    cyg_sem_t sem;
    cyg_semaphore_init(&sem, 0);
    msg->rc = VTSS_RC_INCOMPLETE;
    msg->rsp = NULL;
    msg->syncro = &sem;
    dualcpu_send(msg);
    cyg_semaphore_wait(&sem);
    /* Return cc from response, if given, or from request */
    return msg->rsp ? msg->rsp->rc : msg->rc;
}

vtss_rc rpc_message_send(rpc_msg_t *msg)
{
    dualcpu_send(msg);
    return VTSS_RC_OK;
}
