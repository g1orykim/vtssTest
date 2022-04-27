/*

 Vitesse Switch API software.

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
 
*/

#include "main.h"
#include "firmware.h"
#include "firmware_api.h"
#include "control_api.h"
#include "msg_api.h"
#include "cli_api.h"

#if VTSS_SWITCH_STACKABLE
#include "topo_api.h"
#endif /* VTSS_SWITCH_STACKABLE */

#include <cyg/io/flash.h>

#ifdef VTSS_SW_OPTION_VCLI
#include "firmware_cli.h"
#endif

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_FIRMWARE

/*
 * Prvate functions
 */

static void firmware_send_message(vtss_usid_t sid, firmware_msg_id_t msg_id, vtss_restart_t restart);
static void firmware_send_image(vtss_usid_t sid, const unsigned char *buffer, size_t length);
static void slave_firmware_update(cli_iolayer_t *io, const unsigned char *buffer, size_t length, vtss_restart_t restart);

/* Thread variables */
static cyg_handle_t firmware_thread_handle;
static cyg_thread   firmware_thread_block;
static char         firmware_thread_stack[2 * (THREAD_DEFAULT_STACK_SIZE)];
static cyg_handle_t firmware_mbhandle;
static cyg_mbox     firmware_mbox;

/* "simple" lock functions */
#define SIMPLE_LOCK()	cyg_scheduler_lock()
#define SIMPLE_UNLOCK()	cyg_scheduler_unlock()

/* Master update control variables */
cyg_sem_t firmware_update_token; /* Main entry - update in progress */
static struct {
    cli_iolayer_t *io;        /* Master side CLI connection */
    unsigned long switches;   /* Switch set: All switches (online) in the stack */
    unsigned long txactive;   /* Switch set: Image xmit active */
    unsigned long confirmed;  /* Switch set: Image received sucessfully */
    unsigned long failed;     /* Switch set: Image sent unsucessfully */
    const void *image;        /* "Armed" image (slave side) */
    size_t len;               /* "Armed" image length (slave side) */
    vtss_restart_t restart;   /* How to restart after f/w-update */
} stackdata;

static const char *firmware_status = "idle";

#if (VTSS_TRACE_ENABLED)
static vtss_trace_reg_t trace_reg =
{ 
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "firmware",
    .descr     = "Firmware Update"
};

static vtss_trace_grp_t trace_grps[TRACE_GRP_CNT] =
{
    [VTSS_TRACE_GRP_DEFAULT] = { 
        .name      = "default",
        .descr     = "Default",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
};
#endif /* VTSS_TRACE_ENABLED */

/*
 * Message transmit completion function
 */
static void
firmware_msg_tx_done(void *contxt, 
                     void *msg, 
                     msg_tx_rc_t rc)
{
    vtss_isid_t sid = (vtss_isid_t) contxt;
    VTSS_ASSERT(stackdata.io != NULL);
    SIMPLE_LOCK();
    if(rc) {
#ifdef VTSS_SW_OPTION_CLI
        cli_io_printf(stackdata.io, "Unsuccessful image transfer to switch %d (rc %d), skipping unit\n", 
                      sid, rc); 
#endif //  VTSS_SW_OPTION_CLI
        stackdata.switches &= ~(1 << sid);
        stackdata.failed++;
        /* Check to see if we can send GO */
        cyg_mbox_put(firmware_mbhandle, (void*)FIRMWARE_MBMSG_CHECKGO);
    } else {
#if defined(VTSS_SW_OPTION_CLI) && VTSS_SWITCH_STACKABLE
        cli_io_printf(stackdata.io, "Transferred image to switch %d\n", topo_isid2usid(sid));
#endif // defined(VTSS_SW_OPTION_CLI) && VTSS_SWITCH_STACKABLE
    }
    if((--stackdata.txactive) == 0) { /* Still sending? */
        T_D("All switches have been sent image, freeing buffer"); 
        VTSS_FREE(msg);
    }
    SIMPLE_UNLOCK();
}

/* 
 * Send image across stack
 */
static void firmware_send_image(vtss_usid_t sid,  const unsigned char *buffer, size_t length)
{
    T_D("Sending %zd bytes firmware to sid %d", length, sid);
    /* Send using "advanced" interface to get tx completion and avoid
     * free after use (we re-use the buffer) */
    msg_tx_adv((void*) sid, firmware_msg_tx_done, MSG_TX_OPT_DONT_FREE | MSG_TX_OPT_NO_ALLOC_ON_LOOPBACK, VTSS_MODULE_ID_FIRMWARE, sid, buffer, length);
}

/*
 * Send message across stack
 */
static void
firmware_send_message(const vtss_usid_t sid, 
                      const firmware_msg_id_t msg_id,
                      vtss_restart_t restart)
{
    firmware_msg_t *message = VTSS_MALLOC(sizeof(*message));
    if(message) {
        message->msg_id = msg_id;
        message->restart = restart;
        /* Send using "simple" interface - no completion, message
         * freed after use */
        msg_tx(VTSS_MODULE_ID_FIRMWARE, sid, message, sizeof(*message));
    }
}

/*
 * Message indication function
 */
static BOOL 
firmware_msg_rx(void *contxt, 
                const void *rx_msg, 
                size_t len, 
                vtss_module_id_t modid, 
                ulong isid)
{
    if(len == sizeof(firmware_msg_t)) {
        const firmware_msg_t *fwmsg = (void*)rx_msg;
        T_D("Sid %u, rx %zd bytes, msg %d", isid, len, fwmsg->msg_id);
        switch (fwmsg->msg_id) {
        case FIRMWARE_MSG_ID_IMAGE_BEGIN:
#if VTSS_SWITCH_STACKABLE
            topo_led_update_set(FALSE);
#endif
            break;
        case FIRMWARE_MSG_ID_IMAGE_CNF:
            if(msg_switch_is_master()) {
                SIMPLE_LOCK();
                stackdata.confirmed |= (1 << isid);
                SIMPLE_UNLOCK();
                /* Check to see if we can send GO */
                cyg_mbox_put(firmware_mbhandle, (void*)FIRMWARE_MBMSG_CHECKGO);
            }
            break;
        case FIRMWARE_MSG_ID_IMAGE_BURN:
            SIMPLE_LOCK();
            if(stackdata.image) { /* Only update if armed! */
                stackdata.restart = fwmsg->restart;
                slave_firmware_update(cli_get_serial_io_handle(), stackdata.image, stackdata.len, stackdata.restart);
            }

#if VTSS_SWITCH_STACKABLE
            topo_led_update_set(TRUE);
#endif
            stackdata.image = NULL; /* Unarm update */
            SIMPLE_UNLOCK();
            break;
        case FIRMWARE_MSG_ID_IMAGE_ABRT:
#if VTSS_SWITCH_STACKABLE
            topo_led_update_set(TRUE);
#endif
            SIMPLE_LOCK();
            if(stackdata.image) { /* Only abort if armed! */
                VTSS_FREE((void *)stackdata.image);
                stackdata.image = NULL; /* Unarm update */
            }
            SIMPLE_UNLOCK();
            firmware_status_set(NULL);
            break;
        default:;
            T_W("Unhandled msg %d", fwmsg->msg_id);
        }
    } else {
        /* This should be an image buffer */
        if(len > 256) {         /* Sanity check */
            vtss_rc result;
            unsigned long image_version;
            result = firmware_check(rx_msg, len, FIRMWARE_TRAILER_V1_TYPE_MANAGED, 0, &image_version);
            if(result == VTSS_OK) {
                void *image;
                T_D("%s: Valid software image received", __FUNCTION__);
                if ((image = VTSS_MALLOC(len))) { /* Must copy from RX indication message */
                    memcpy(image, rx_msg, len); 
                    SIMPLE_LOCK();
                    if(stackdata.image)
                        VTSS_FREE((void *)stackdata.image);
                    stackdata.image = image;
                    stackdata.len = len;
                    SIMPLE_UNLOCK();
                }
            }
            /* Confirm to master to collectively go ahead */
            firmware_send_message(isid, FIRMWARE_MSG_ID_IMAGE_CNF, -1);
        }
    }
    return TRUE;
}

/*
 * Stack Register
 */
static vtss_rc
firmware_stack_register(void)
{
    msg_rx_filter_t filter;    
    memset(&filter, 0, sizeof(filter));
    filter.cb = firmware_msg_rx;
    filter.modid = VTSS_MODULE_ID_FIRMWARE;
    return msg_rx_filter_register(&filter);
}

/****************************************************************************
 * Module thread
 ****************************************************************************/

static void
firmware_thread(cyg_addrword_t data)
{
    /* Register for stack messages */
    firmware_stack_register();

    for(;;) {
        /*
         * If we enter firmware programming - we *ALWAYS* will reboot.
         * Some scenarios could get past that, but deciding always to
         * take a dive makes the code simpler. (And more robust
         * against stale locks and leaks...)
         */
        firmware_flash_args_t *pReq = cyg_mbox_get(firmware_mbhandle);
        if(FIRMWARE_MBMSG_CHECKGO == (firmware_mbmsg_t) pReq) {
            if((stackdata.confirmed & stackdata.switches) == stackdata.switches) {
                vtss_isid_t sid;
                VTSS_ASSERT(stackdata.io != NULL);
                if(stackdata.failed == 0) {
#if defined(VTSS_SW_OPTION_CLI) && VTSS_SWITCH_STACKABLE
                    cli_io_printf(stackdata.io, "All switches confirmed reception, programming\n");
#endif // defined(VTSS_SW_OPTION_CLI) && VTSS_SWITCH_STACKABLE
                    for(sid = VTSS_ISID_START; sid < VTSS_ISID_END; sid++) {
                        if(stackdata.switches & (1 << sid))
                            firmware_send_message(sid, FIRMWARE_MSG_ID_IMAGE_BURN, stackdata.restart);
                    }
                } else {
#if defined(VTSS_SW_OPTION_CLI) && VTSS_SWITCH_STACKABLE
                    cli_io_printf(stackdata.io, "Error: %ld switches failed to receive image, reboot recommended.\n",
                                  stackdata.failed);
#endif // defined(VTSS_SW_OPTION_CLI) && VTSS_SWITCH_STACKABLE
                    firmware_status_set("Error: Incomplete stack update - update aborted");
                    for(sid = VTSS_ISID_START; sid < VTSS_ISID_END; sid++) {
                        if(stackdata.switches & (1 << sid))
                            firmware_send_message(sid, FIRMWARE_MSG_ID_IMAGE_ABRT, -1);
                    }
                }
            }
        } else {
            if(pReq) {
                // Cache the current value of pReq->restart, because of firmware_program_doit()
                // fails (e.g. because the new image is the same as the current, the caller
                // VTSS_FREE(pReq), which means that the restart member is no longer valid
                // after the next call.
                vtss_restart_t restart = pReq->restart;
                firmware_program_doit(pReq, "managed");
                /* The update function only returns in case of failure - reboot always */

                control_system_reset_sync(restart);
                /* NOTREACHED */
            }
        }
    }
}

/****************************************************************************
 * Utility functions - called from slave functions
 ****************************************************************************/

/*
 * Determine if we can update now - if so take the update semaphore
 */
static vtss_rc firmware_can_update(const unsigned char *buffer, size_t length)
{
    vtss_rc result;
    unsigned long image_version;
    
    /* Valid image? */
    result = firmware_check(buffer, length, FIRMWARE_TRAILER_V1_TYPE_MANAGED, 0, &image_version);
    if(result != VTSS_OK) {
        goto error;
    }

    /* See if we can start the update */
    if(cyg_semaphore_trywait(&firmware_update_token))
        return VTSS_OK;
    result = FIRMWARE_ERROR_BUSY;

 error:
    VTSS_FREE((void *)buffer);
    return result;
}

/****************************************************************************
 * Master part
 ****************************************************************************/
static void master_firmware_update(cli_iolayer_t *io, const unsigned char *buffer, size_t length, vtss_restart_t restart)
{
#if defined(VTSS_SW_OPTION_CLI) && VTSS_SWITCH_STACKABLE
    cli_io_printf(io, "Master initiated software updating starting\n");
#endif // VTSS_SW_OPTION_CLI && VTSS_SWITCH_STACKABLE
    vtss_isid_t sid;
    /* Create switch set to update */
    for(sid = VTSS_ISID_START; sid < VTSS_ISID_END; sid++) {
        if(msg_switch_exists(sid)) {
            SIMPLE_LOCK();
            stackdata.switches |= (1 << sid);
            stackdata.txactive++;
            SIMPLE_UNLOCK();
        }
    }
    /* Initialize active/confirmed mask */
    SIMPLE_LOCK();
    stackdata.failed = stackdata.confirmed = 0; /* None failed/confirmed yet */
    stackdata.io = io;
    stackdata.restart = restart;
    SIMPLE_UNLOCK();
    /* Now send them all a copy (using the same buffer) */
    for(sid = VTSS_ISID_START; sid < VTSS_ISID_END; sid++) {
        if(stackdata.switches & (1 << sid)) {
            firmware_send_message(sid, FIRMWARE_MSG_ID_IMAGE_BEGIN, -1);
            firmware_send_image(sid, buffer, length);
        }
    }
    /*
     * The update requests are now sent, and flashing will start
     * when all switches have confirmed/failed image transfer. Upon
     * flash update completion, each switch will reset/restart.
     * Flash update can only be initiated once (until reset).
     */
}

/****************************************************************************
 * Slave part
 ****************************************************************************/

static void slave_firmware_update(cli_iolayer_t *io, const unsigned char *buffer, size_t length, vtss_restart_t restart)
{
    firmware_flash_args_t *pReq = NULL;
    vtss_rc rc = firmware_update_mkreq(io, buffer, length, &pReq,
                                       FALSE, /* *No* synchronization */
                                       FIRMWARE_TRAILER_V1_TYPE_MANAGED, 0, restart);
    if(rc == VTSS_OK && pReq != NULL) {
        cyg_mbox_put(firmware_mbhandle, pReq);
        /* Don't wait for completion! */
    } else {
        /* Error, free image buffer */
        VTSS_FREE((void *)buffer);
    }
}

/****************************************************************************
 * Local / API
 ****************************************************************************/

/*
 * API: Synchronous firmware update.
 * NB: *Only* returns if called with invalid image!
 */
vtss_rc firmware_update(cli_iolayer_t *io, const unsigned char *buffer, size_t length, vtss_restart_t restart)
{
    vtss_rc result;
    if((result = firmware_can_update(buffer, length)) != VTSS_OK)
        return result;    /* Unable to update */

    if(msg_switch_is_master()) {
        master_firmware_update(io, buffer, length, restart);
    } else {
#ifdef VTSS_SW_OPTION_CLI
        cli_io_printf(io, "Slave, only doing local update\n");
#endif // VTSS_SW_OPTION_CLI
        slave_firmware_update(io, buffer, length, restart);
    }

#ifdef VTSS_SW_OPTION_CLI
    cli_io_printf(io, "Waiting for firmware update to complete\n");
#endif // VTSS_SW_OPTION_CLI
    for(;;) {
        VTSS_OS_MSLEEP(5000);
        T_D("(Still) waiting for firmware update to complete");
    }
    /* NOTREACHED */
}

/*
 * API: Asynchronous firmware update.
 * Returns VTSS_OK if update is started, any other FIRMWARE_ERROR_xxx code otherwise
 */
vtss_rc firmware_update_async(cli_iolayer_t *io, const unsigned char *buffer, size_t length, vtss_restart_t restart)
{
    vtss_rc result;
    if((result = firmware_can_update(buffer, length)) != VTSS_OK)
        return result;          /* Unable to update */

    if(msg_switch_is_master()) {
        master_firmware_update(io, buffer, length, restart);
    } else {
#ifdef VTSS_SW_OPTION_CLI
        cli_io_printf(io, "Slave, only doing local update\n");
#endif // VTSS_SW_OPTION_CLI
        slave_firmware_update(io, buffer, length, restart);
    }
    return result; /* Update started */
}

const char *firmware_status_get(void)
{
    return firmware_status;
}

void firmware_status_set(const char *status)
{
    firmware_status = status ? status : "idle";
    T_I("Firmware update status: %s", firmware_status);
}

/****************************************************************************/
/*  Initialization functions                                                */
/****************************************************************************/

vtss_rc
firmware_init(vtss_init_data_t *data)
{
    switch (data->cmd) {
    case INIT_CMD_INIT:
        /* Initialize and register trace ressources */
#ifdef VTSS_SW_OPTION_VCLI
        firmware_cli_init();
#endif
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);
        cyg_mbox_create(&firmware_mbhandle, &firmware_mbox);
        cyg_semaphore_init(&firmware_update_token, 1); /* Allow only one updater at a time */
        cyg_thread_create(THREAD_DEFAULT_PRIO, 
                          firmware_thread, 
                          0, 
                          "Firmware", 
                          firmware_thread_stack, 
                          sizeof(firmware_thread_stack),
                          &firmware_thread_handle,
                          &firmware_thread_block);
        cyg_thread_resume(firmware_thread_handle);
        break;
    default:
        break;
    }

    return 0;
}

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
