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

// ***************************************************************
// * Thermal protection is used for protecting the chip from
// * getting overheated. It is done by turning off ports when
// * a configurable temperature is reached. It is possible to give
// * the ports a priority for when to shut down. Each priority has
// * a configurable temperature at which the ports with the given
// * priorities are shut down.
// ***************************************************************
#include "critd_api.h"
#include "main.h"
#include "msg_api.h"

#include "thermal_protect.h"
#include "thermal_protect_api.h"
#include "vtss_misc_api.h"
#include "vtss_phy_api.h"

#include "thermal_protect_custom_api.h"

#ifdef VTSS_SW_OPTION_PACKET
#include "packet_api.h"
#endif
#include "conf_api.h"
#include "misc_api.h"

#ifdef VTSS_SW_OPTION_ICFG
#include "thermal_protect_icli_functions.h" // For thermal_protect_icfg_init
#endif // VTSS_SW_OPTION_ICFG

//****************************************
// TRACE
//****************************************
#if (VTSS_TRACE_ENABLED)

static vtss_trace_reg_t trace_reg = {
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "thermal",
    .descr     = "Thermal_Protect control"
};

static vtss_trace_grp_t trace_grps[TRACE_GRP_CNT] = {
    [VTSS_TRACE_GRP_DEFAULT] = {
        .name      = "default",
        .descr     = "Default",
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 1,
    },
    [TRACE_GRP_CONF] = {
        .name      = "conf",
        .descr     = "THERMAL_PROTECT configuration",
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 1,
    },
    [TRACE_GRP_CLI] = {
        .name      = "cli",
        .descr     = "Command line interface",
        .lvl       = VTSS_TRACE_LVL_INFO,
        .timestamp = 1,
    },
    [TRACE_GRP_CRIT] = {
        .name      = "crit",
        .descr     = "Critical regions ",
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 1,
    }
};
/* Critical region protection protecting the following block of variables */
static critd_t    crit;
#define THERMAL_PROTECT_CRIT_ENTER()         critd_enter(&crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define THERMAL_PROTECT_CRIT_EXIT()          critd_exit( &crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define THERMAL_PROTECT_CRIT_ASSERT_LOCKED() critd_assert_locked(&crit, TRACE_GRP_CRIT, __FILE__, __LINE__)
#else
#define THERMAL_PROTECT_CRIT_ENTER()         critd_enter(&crit)
#define THERMAL_PROTECT_CRIT_EXIT()          critd_exit( &crit)
#define THERMAL_PROTECT_CRIT_ASSERT_LOCKED() critd_assert_locked(&crit)
#endif /* VTSS_TRACE_ENABLED */

//***********************************************
// MISC
//***********************************************
/* Thread variables */
static cyg_handle_t thermal_protect_thread_handle;
static cyg_thread   thermal_protect_thread_block;
static char         thermal_protect_thread_stack[THREAD_DEFAULT_STACK_SIZE];

//************************************************
// Global Variables
//************************************************
static thermal_protect_msg_t msg_conf; // semaphore-protected message transmission buffer(s).
static thermal_protect_stack_conf_t   thermal_protect_stack_conf;  // Configuration for whole stack (used when we're master, only).
static thermal_protect_switch_conf_t  thermal_protect_switch_conf; // Configuration for this switch.
static thermal_protect_mutex_t        crit_region;
static thermal_protect_local_status_t switch_status; // Status from a slave switch.
static cyg_flag_t         status_flag; // Flag for signaling that status data from a slave has been received.
static vtss_mtimer_t timer; // Timer for timeout

//************************************************
// Misc. functions
//************************************************

//
// Converts error to printable text
//
// In : rc - The error type
//
// Return : Error text
//
char *thermal_protect_error_txt(vtss_rc rc)
{
    switch (rc) {
    case THERMAL_PROTECT_ERROR_ISID:
        return "Invalid Switch ID";

    case THERMAL_PROTECT_ERROR_FLASH:
        return "Could not store configuration in flash";

    case THERMAL_PROTECT_ERROR_SLAVE:
        return "Could not get data from slave switch";

    case THERMAL_PROTECT_ERROR_NOT_MASTER:
        return "Switch must to be master";

    case THERMAL_PROTECT_ERROR_VALUE:
        return "Invalid value";

    default:
        T_D("Default");
        return "";
    }
}

// Converts power down status  to printable text
//
// In : power down status
//
// Return : Printable text
//
char *thermal_protect_power_down2txt(BOOL powered_down)
{
    if (powered_down) {
        return "Port link is thermal protected (Link is down)";
    } else {
        return "Port link operating normally";
    }
}

// Function for keeping status off if a port is powered down due to thermal protection.
// IN : iport - The port number starting from 0
//      set   - TRUE if the port power down status shall be updated. FALSE is the port power status shall not be updated (For getting the status).
//      value - The new value for the port power status. Only valid if the parameter "set" is set to TRUE.
//     Return - TRUE if port is powered down due to thermal protection else FALSE
static BOOL is_port_down(vtss_port_no_t iport, BOOL set, BOOL value)
{
    THERMAL_PROTECT_CRIT_ENTER(); // Protect thermal_protect_local_conf
    static BOOL port_down_due_to_thermal_protection_init = FALSE;
    static BOOL port_down_due_to_thermal_protection[VTSS_PORTS];
    port_iter_t  pit;
    BOOL result;

    // Initialize the port down array.
    if (port_down_due_to_thermal_protection_init == FALSE) {
        port_down_due_to_thermal_protection_init = TRUE;
        // Loop through all front ports
        if (port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT | PORT_ITER_FLAGS_NPI) == VTSS_OK) {
            while (port_iter_getnext(&pit)) {
                port_down_due_to_thermal_protection[pit.iport] = FALSE;
            }
        }
    }

    if (set) {
        // Setting new value
        port_down_due_to_thermal_protection[iport] = value;
    }

    result =  port_down_due_to_thermal_protection[iport]; // Getting current port status
    THERMAL_PROTECT_CRIT_EXIT();

    return result; // Return current status
}

// Function that returns this switch's temperature status
//
// Out : status - Pointer to where to put chip status
static void thermal_protect_get_local_status(thermal_protect_local_status_t *status)
{
    i16 temp;
    port_iter_t  pit;
    //vtss_phy_chip_temp_get(NULL, 0, &temp);

    // Loop through all front ports
    if (port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT | PORT_ITER_FLAGS_NPI) == VTSS_OK) {
        while (port_iter_getnext(&pit)) {
            status->port_powered_down[pit.iport] = is_port_down(pit.iport, FALSE, FALSE);
            if (thermal_protect_get_temperture(pit.iport, &temp) != VTSS_OK) {
                T_E("Could not read chip temperature, setting temperature to 255 C");
                status->port_temp[pit.iport] = 255;
            } else {
                status->port_temp[pit.iport] = temp;
            }
        }
    }
}

/*************************************************************************
** Message module functions
*************************************************************************/

/* Allocate request/reply buffer */
static void thermal_protect_msg_alloc(thermal_protect_msg_buf_t *buf, BOOL request)
{
    THERMAL_PROTECT_CRIT_ENTER();
    buf->sem = (request ? &msg_conf.request.sem : &msg_conf.reply.sem);
    buf->msg = (request ? &msg_conf.request.msg[0] : &msg_conf.reply.msg[0]);
    THERMAL_PROTECT_CRIT_EXIT();
    (void) VTSS_OS_SEM_WAIT(buf->sem);
}

/* Release message buffer */
static void thermal_protect_msg_tx_done(void *contxt, void *msg, msg_tx_rc_t rc)
{
    VTSS_OS_SEM_POST(contxt);
}

/* Send message */
static void thermal_protect_msg_tx(thermal_protect_msg_buf_t *buf, vtss_isid_t isid, size_t len)
{
    msg_tx_adv(buf->sem, thermal_protect_msg_tx_done, MSG_TX_OPT_DONT_FREE,
               VTSS_TRACE_MODULE_ID, isid, buf->msg, len);
}

// Transmits status from a slave to the master
//
// In : master_id - The master switch's id
static void thermal_protect_msg_tx_switch_status (vtss_isid_t master_id)
{
    // Send the new configuration to the switch in question
    thermal_protect_msg_buf_t      buf;
    thermal_protect_msg_local_switch_status_t  *msg;

    thermal_protect_local_status_t status;
    thermal_protect_get_local_status(&status); // Get chip temperature and thermal_protect rotation count

    thermal_protect_msg_alloc(&buf, 1);
    msg = (thermal_protect_msg_local_switch_status_t *)buf.msg;
    msg->status = status;

    // Do the transmission
    msg->msg_id = THERMAL_PROTECT_MSG_ID_STATUS_REP; // Set msg ID
    thermal_protect_msg_tx(&buf, master_id, sizeof(*msg)); //  Send the msg
}

// Getting message from the message module.
static BOOL thermal_protect_msg_rx(void *contxt, const void *const rx_msg, const size_t len, const vtss_module_id_t modid, const ulong isid)
{
    vtss_rc        rc;
    vtss_port_no_t port_index;
    thermal_protect_msg_id_t msg_id = *(thermal_protect_msg_id_t *)rx_msg;
    T_R("Entering thermal_protect_msg_rx");
    T_N("msg_id: %d,  len: %zd, isid: %u", msg_id, len, isid);

    switch (msg_id) {

    // Update switch's configuration
    case THERMAL_PROTECT_MSG_ID_CONF_SET_REQ: {
        // Got new configuration
        T_DG(TRACE_GRP_CONF, "rx: msg_id = THERMAL_PROTECT_MSG_ID_CONF_SET_REQ");
        thermal_protect_msg_local_switch_conf_t *msg;
        msg = (thermal_protect_msg_local_switch_conf_t *)rx_msg;
        THERMAL_PROTECT_CRIT_ENTER();
        thermal_protect_switch_conf  =  (msg->switch_conf); // Update configuration for this switch.
        T_DG(TRACE_GRP_CONF, "prio_temperatures[0] = %d",
             thermal_protect_switch_conf.glbl_conf.prio_temperatures[0]);
        THERMAL_PROTECT_CRIT_EXIT();

        // Now that we have received a good configuration, we can safely resume our thread.
        // It is OK to resume a thread multiple times; it doesn't take more than one suspend
        // to suspend it anyway (but the other way around is another story).
        cyg_thread_resume(thermal_protect_thread_handle);
        break;
    }

    // Port shutdown requested
    case THERMAL_PROTECT_MSG_ID_PORT_SHUTDOWN_REQ:
        T_N("msg_id = THERMAL_PROTECT_MSG_ID_PORT_SHUTDOW_REQ");
        port_vol_conf_t conf;
        port_user_t     user = PORT_USER_THERMAL_PROTECT;
        thermal_protect_msg_port_shutdown_t *msg;
        msg = (thermal_protect_msg_port_shutdown_t *)rx_msg;
        port_index = msg->port_index;

        T_D_PORT(port_index, "Thermal protection - Shutting down port, isid:%d", isid);
        if ((rc = port_vol_conf_get(user, isid, port_index, &conf)) != VTSS_OK) {
            T_E(error_txt(rc));
        } else {
            conf.disable = msg->link_down;
            if ((rc = port_vol_conf_set(user, isid, port_index, &conf)) != VTSS_OK) {
                T_E(error_txt(rc));
            }
        }
        break;

    // Master has requested status
    case THERMAL_PROTECT_MSG_ID_STATUS_REQ:
        T_N("msg_id = THERMAL_PROTECT_MSG_ID_STATUS_REQ");
        thermal_protect_msg_tx_switch_status(isid); // Transmit status back to master.
        break;

    // Got status from a slave switch
    case THERMAL_PROTECT_MSG_ID_STATUS_REP:
        T_N("msg_id = THERMAL_PROTECT_MSG_ID_STATUS_REP");
        thermal_protect_msg_local_switch_status_t *msg1;
        msg1 = (thermal_protect_msg_local_switch_status_t *)rx_msg;
        THERMAL_PROTECT_CRIT_ENTER();
        switch_status =  (msg1->status); // Update status for switch.
        cyg_flag_setbits(&status_flag, 1 << isid); // Signal that the message has been received
        THERMAL_PROTECT_CRIT_EXIT();
        break;

    default:
        T_W("unknown message ID: %d", msg_id);
        break;
    }

    return TRUE;
}

// Transmits a new configuration to a slave switch via the message protocol
//
// In : slave_id - The id of the switch to receive the new configuration.
static void thermal_protect_msg_tx_switch_conf (vtss_isid_t slave_id)
{
    // Send the new configuration to the switch in question
    thermal_protect_msg_buf_t      buf;
    thermal_protect_msg_local_switch_conf_t  *msg;
    thermal_protect_msg_alloc(&buf, 1);
    msg = (thermal_protect_msg_local_switch_conf_t *)buf.msg;

    THERMAL_PROTECT_CRIT_ENTER();
    msg->switch_conf.local_conf = thermal_protect_stack_conf.local_conf[slave_id];
    msg->switch_conf.glbl_conf  = thermal_protect_stack_conf.glbl_conf;
    T_R("temp[0] = %d, isid = %d", thermal_protect_stack_conf.glbl_conf.prio_temperatures[0], slave_id);

    THERMAL_PROTECT_CRIT_EXIT();
    T_DG(TRACE_GRP_CONF, "Transmit THERMAL_PROTECT_MSG_ID_CONF_SET_REQ");
    // Do the transmission
    msg->msg_id = THERMAL_PROTECT_MSG_ID_CONF_SET_REQ; // Set msg ID
    thermal_protect_msg_tx(&buf, slave_id, sizeof(*msg)); //  Send the msg
}

// Transmit port shutdown from slave to the master switch. This is needed because we volatile need to shut down the port, and that needs to be done by the master (because the slave doesn't know it's own isid)
//
// In : port_index - Port to shut down
//      link_down  - TRUE to volatile shut down the port. FALSE to volatile power up.
static void thermal_protect_msg_tx_port_shutdown(vtss_port_no_t port_index, BOOL link_down)
{
    // Send the port shutdown to master switch
    thermal_protect_msg_buf_t      buf;
    thermal_protect_msg_port_shutdown_t *msg;
    thermal_protect_msg_alloc(&buf, 1);
    msg = (thermal_protect_msg_port_shutdown_t *)buf.msg;
    msg->port_index = port_index;
    msg->link_down = link_down;
    T_RG(TRACE_GRP_CONF, "Transmit THERMAL_PROTECT_MSG_ID_PORT_SHUTDOW_REQ");
    // Do the transmission
    msg->msg_id = THERMAL_PROTECT_MSG_ID_PORT_SHUTDOWN_REQ; // Set msg ID
    thermal_protect_msg_tx(&buf, 0, sizeof(*msg)); //  Send the msg
}

// Transmits a status request to a slave, and wait for the reply.
//
// In : slave_id - The slave switch id.
//
// Return : True if NO reply from slave switch.
static BOOL thermal_protect_msg_tx_switch_status_req (vtss_isid_t slave_id)
{
    BOOL             timeout;
    cyg_flag_value_t flag;
    cyg_tick_count_t time_tick;

    THERMAL_PROTECT_CRIT_ENTER();
    timeout = VTSS_MTIMER_TIMEOUT(&timer);
    THERMAL_PROTECT_CRIT_EXIT();

    if (timeout) {
        // Setup sync flag.
        flag = (1 << slave_id);
        cyg_flag_maskbits(&status_flag, ~flag);

        // Send the status request to the switch in question
        thermal_protect_msg_buf_t      buf;
        thermal_protect_msg_alloc(&buf, 1);
        thermal_protect_msg_id_req_t *msg = (thermal_protect_msg_id_req_t *)buf.msg;
        // Do the transmission
        msg->msg_id = THERMAL_PROTECT_MSG_ID_STATUS_REQ; // Set msg ID

        if (msg_switch_exists(slave_id)) {
            T_D("Transmitting THERMAL_PROTECT_MSG_ID_STATUS_REQ");
            thermal_protect_msg_tx(&buf, slave_id, sizeof(*msg)); //  Send the Mag
        } else {
            T_D("Skipped thermal_protect_msg_tx due to isid:%d msg switch doesn't exist", slave_id);
            return TRUE; // Signal status get failure.
        }


        // Wait for timeout or synch. flag to be set.
        time_tick = cyg_current_time() + VTSS_OS_MSEC2TICK(1000);
        return (cyg_flag_timed_wait(&status_flag, flag, CYG_FLAG_WAITMODE_OR, time_tick) & flag ? 0 : 1);
    }

    T_DG(TRACE_GRP_CONF, "timeout not set");
    return TRUE; // Signal status get failure
}

// Initializes the message protocol
static void thermal_protect_msg_init(void)
{
    /* Register for stack messages */
    msg_rx_filter_t filter;

    /* Initialize message buffers */
    VTSS_OS_SEM_CREATE(&msg_conf.request.sem, 1);
    VTSS_OS_SEM_CREATE(&msg_conf.request.sem, 1);

    memset(&filter, 0, sizeof(filter));
    filter.cb = thermal_protect_msg_rx;
    filter.modid = VTSS_TRACE_MODULE_ID;
    (void) msg_rx_filter_register(&filter);
}

//************************************************
// Main functions
//************************************************

// Function that checks if ports shall be shut down due to the chip temperature
// is above the configured levels.
static void thermal_protect_chk(void)
{
    u8 port_prio;
    u8 prio_temp;
    port_iter_t  pit;

    // The chip temperature
    thermal_protect_local_status_t status;
    thermal_protect_get_local_status(&status);

    // Loop through all priorities and ports to check if
    // any ports should be shut down
    int  prio_index;
    for (prio_index = 0; prio_index < THERMAL_PROTECT_PRIOS_CNT; prio_index++) {
        // Loop through all front ports
        if (port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT | PORT_ITER_FLAGS_NPI) == VTSS_OK) {
            while (port_iter_getnext(&pit)) {
                THERMAL_PROTECT_CRIT_ENTER(); // Protect thermal_protect_switch_conf
                port_prio = thermal_protect_switch_conf.local_conf.port_prio[pit.iport];
                prio_temp = thermal_protect_switch_conf.glbl_conf.prio_temperatures[prio_index];
                THERMAL_PROTECT_CRIT_EXIT();
                if (port_prio == prio_index) {
                    T_D_PORT(pit.iport, "prio_index = %d, port_prio= %d, Temp = %d, port_temp = %d",
                             prio_index, port_prio, prio_temp, status.port_temp[pit.iport]);

                    if (prio_temp < status.port_temp[pit.iport])  {
                        // Shut down port
                        thermal_protect_msg_tx_port_shutdown(pit.iport, TRUE);
                        (void) is_port_down(pit.iport, TRUE, TRUE); // Store that port is turn on
                    } else if (is_port_down(pit.iport, FALSE, TRUE)) {
                        // Turn on port
                        thermal_protect_msg_tx_port_shutdown(pit.iport, FALSE);
                        (void) is_port_down(pit.iport, TRUE, FALSE);  // Store that port is turn off
                    }
                }
            }
        }
    }
}

//************************************************
// Configuration
//************************************************
// Function transmitting configuration to all switches in the stack
static void thermal_protect_msg_tx_conf_to_all(void)
{
    vtss_isid_t       isid;
    // loop through all isids and send new configuration to slave switch if it exist.
    for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
        if (msg_switch_exists(isid)) {
            thermal_protect_msg_tx_switch_conf(isid);
        }
    }
}

// Function for getting the default configuration for a single switch
// Out - switch_conf - Pointer to where to put the default configuration.
void thermal_protect_switch_conf_default_get(thermal_protect_switch_conf_t *switch_conf)
{
    //Set default configuration
    memset(switch_conf, 0, sizeof(*switch_conf)); // Set everything to 0. Non-zero default values will be set below.
    int prio_index;
    for (prio_index = 0; prio_index < THERMAL_PROTECT_PRIOS_CNT; prio_index++) {
        switch_conf->glbl_conf.prio_temperatures[prio_index] = THERMAL_PROTECT_TEMP_MAX;
    }
}

// Function for reading configuration from flash. Restoring
// to default values if create = TRUE or flash access fails.
//
// In : Create : Restore configuration to default setting if set to TRUE
static void  thermal_protect_conf_read(BOOL create)
{
    thermal_protect_flash_conf_t *thermal_protect_flash_conf; // Configuration in flash
    ulong             size;
    BOOL              do_create;      // Set if we need to create new configuration in flash

    T_RG(TRACE_GRP_CONF, "Entering thermal_protect_conf_read");

    if (misc_conf_read_use()) {
        // Get configration from flash (if possible)
        if ((thermal_protect_flash_conf = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_THERMAL_PROTECT_CONF, &size)) == NULL ||
            size != sizeof(thermal_protect_flash_conf_t)) {
            thermal_protect_flash_conf = conf_sec_create(CONF_SEC_GLOBAL, CONF_BLK_THERMAL_PROTECT_CONF, sizeof(thermal_protect_flash_conf_t));
            T_W("conf_sec_open failed or size mismatch, creating defaults");
            do_create = TRUE;
        } else if (thermal_protect_flash_conf->version != THERMAL_PROTECT_CONF_VERSION) {
            T_W("version mismatch, creating defaults");
            do_create = TRUE;
        } else {
            do_create = create;
        }
    } else {
        thermal_protect_flash_conf = NULL;
        do_create                  = TRUE;
    }

    THERMAL_PROTECT_CRIT_ENTER();

    // Create default configuration in flash
    if (do_create) {
        thermal_protect_switch_conf_t switch_conf;             // Configuration for a single switch
        thermal_protect_switch_conf_default_get(&switch_conf); // Get default configuration

        thermal_protect_stack_conf.glbl_conf = switch_conf.glbl_conf; // Common configuration for all switches

        // Set the configuration that is local for each switch
        vtss_isid_t isid;
        for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
            thermal_protect_stack_conf.local_conf[isid] = switch_conf.local_conf;
        }

        if (thermal_protect_flash_conf != NULL) {
            T_DG(TRACE_GRP_CONF, "Restore to default value - block size =  %u ", sizeof(thermal_protect_flash_conf_t));
            thermal_protect_flash_conf->stack_conf = thermal_protect_stack_conf;
        }
    } else {
        if (thermal_protect_flash_conf != NULL) {  // Quiet lint
            thermal_protect_stack_conf = thermal_protect_flash_conf->stack_conf;
        }
    }

    THERMAL_PROTECT_CRIT_EXIT();

    // loop through all isids and send new configuration to slave switch if it exist.
    thermal_protect_msg_tx_conf_to_all();

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    // Write our settings back to flash
    if (thermal_protect_flash_conf) {
        thermal_protect_flash_conf->version = THERMAL_PROTECT_CONF_VERSION;
        T_NG(TRACE_GRP_CONF, "Closing CONF_BLK_THERMAL_PROTECT_CONF");
        conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_THERMAL_PROTECT_CONF);
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
}

// Store the current configuration in flash
static vtss_rc store_conf (void)
{
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    T_DG(TRACE_GRP_CONF, "Storing THERMAL_PROTECT configuration in flash");
    vtss_rc rc = THERMAL_PROTECT_ERROR_FLASH;
    thermal_protect_flash_conf_t *thermal_protect_flash_conf;
    ulong size;

    if ((thermal_protect_flash_conf = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_THERMAL_PROTECT_CONF, &size)) != NULL) {
        if (size == sizeof(*thermal_protect_flash_conf)) {
            THERMAL_PROTECT_CRIT_ENTER();
            thermal_protect_flash_conf->stack_conf = thermal_protect_stack_conf;
            THERMAL_PROTECT_CRIT_EXIT();
            rc = VTSS_OK;
        } else {
            T_W("Could not store THERMAL_PROTECT configuration - Size did not match");
        }
        conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_THERMAL_PROTECT_CONF);
    } else {
        T_W("Could not store THERMAL_PROTECT configuration");
    }

    return rc;
#else
    return VTSS_RC_OK;
#endif
}

/****************************************************************************
* Module thread
****************************************************************************/
static void thermal_protect_thread(cyg_addrword_t data)
{
    port_iter_t  pit;

    // This will block this thread from running further until the PHYs are initialized.
    port_phy_wait_until_ready();

    THERMAL_PROTECT_CRIT_ENTER();
    VTSS_MTIMER_START(&timer, 1);
    THERMAL_PROTECT_CRIT_EXIT();

    if (thermal_protect_init_temperature_sensor != VTSS_OK) {
        T_E("Could not initialize temperature sensor controller. Shutting down ports and terminating thermal protection thread.");
        // Loop through all front ports
        if (port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT | PORT_ITER_FLAGS_NPI) == VTSS_OK) {
            while (port_iter_getnext(&pit)) {
                thermal_protect_msg_tx_port_shutdown(pit.iport, TRUE);
            }
        }
        return;
    }

    // ***** Go into loop **** //
    T_R("Entering thermal_protect_thread");
    for (;;) {
        VTSS_OS_MSLEEP(5000);
        thermal_protect_chk();
    }
}

/****************************************************************************/
/*  API functions (management  functions)                                   */
/****************************************************************************/
//
// Function that returns the current configuration for a switch.
//
// In : isid - isid for the switch the shall return its configuration
//
// In/out : switch_conf - Pointer to configuration struct where the current configuration is copied to.
//
void thermal_protect_mgmt_switch_conf_get(thermal_protect_switch_conf_t *switch_conf, vtss_isid_t isid)
{
    if (isid != VTSS_ISID_LOCAL) {
        // Get this switch's configuration
        THERMAL_PROTECT_CRIT_ENTER();
        // Update the configuration for the switch
        memcpy(&switch_conf->local_conf, &thermal_protect_stack_conf.local_conf[isid], sizeof(thermal_protect_local_conf_t));
        memcpy(&switch_conf->glbl_conf, &thermal_protect_stack_conf.glbl_conf, sizeof(thermal_protect_glbl_conf_t));
        THERMAL_PROTECT_CRIT_EXIT();
    } else {
        THERMAL_PROTECT_CRIT_ENTER(); // Protect  thermal_protect_switch_conf
        *switch_conf = thermal_protect_switch_conf;
        T_I("prio 0 temp = %d", thermal_protect_switch_conf.glbl_conf.prio_temperatures[0]);
        THERMAL_PROTECT_CRIT_EXIT();
    }
}

// Function for setting the current configuration for a switch.
//
// In : isid - isid for the switch the shall return its configuration
//      switch_conf - Pointer to configuration struct with the new configuration.
//
// Return : VTSS error code
vtss_rc thermal_protect_mgmt_switch_conf_set(thermal_protect_switch_conf_t *new_switch_conf, vtss_isid_t isid)
{
    // Configuration changes only allowed by master
    if (!msg_switch_is_master()) {
        T_W("Configuration change only allowed from master switch");
        return THERMAL_PROTECT_ERROR_NOT_MASTER;
    }

    // Ok now we can do the configuration
    THERMAL_PROTECT_CRIT_ENTER();
    // Update the configuration for the switch. Both common configuration for all switches (global), and the local configuration
    // for the switch in question.
    memcpy(&thermal_protect_stack_conf.glbl_conf, &new_switch_conf->glbl_conf, sizeof(thermal_protect_glbl_conf_t));
    memcpy(&thermal_protect_stack_conf.local_conf[isid], &new_switch_conf->local_conf, sizeof(thermal_protect_local_conf_t));
    THERMAL_PROTECT_CRIT_EXIT();

    T_DG(TRACE_GRP_CONF, "Conf. changed");

    if (isid == VTSS_ISID_LOCAL) {
        thermal_protect_msg_tx_conf_to_all();
    } else {
        // Transfer new configuration to the switch in question.
        thermal_protect_msg_tx_switch_conf(isid);
    }

    // Store the new configuration in flash
    return store_conf();
}

//
// Function that returns status for a switch (e.g. chip temperature).
//
// In : isid - isid for the switch the shall return its chip temperature
//
// In/out : status - Pointer to status struct where the switch's status is copied to.
//
vtss_rc thermal_protect_mgmt_get_switch_status(thermal_protect_local_status_t *status, vtss_isid_t isid)
{
    // All switches have the same configuration
    if (thermal_protect_msg_tx_switch_status_req(isid)) {
        T_D("Communication problem with slave switch");
        memset(status, 0, sizeof(thermal_protect_local_status_t)); // We have no real data, so we reset everything to 0.
        return THERMAL_PROTECT_ERROR_SLAVE;
    } else {
        THERMAL_PROTECT_CRIT_ENTER();
        memcpy(status, &switch_status, sizeof(thermal_protect_local_status_t));
        THERMAL_PROTECT_CRIT_EXIT();
        return VTSS_RC_OK;
    }
}

/****************************************************************************/
/*  Initialization functions                                                */
/****************************************************************************/
vtss_rc thermal_protect_init(vtss_init_data_t *data)
{
    vtss_isid_t isid = data->isid; // Get switch id
    vtss_rc rc;

    if (data->cmd == INIT_CMD_INIT) {
        // Initialize and register trace ressources
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);
    }

    T_D("thermal_protect_init enter, cmd=%d", data->cmd);

    switch (data->cmd) {
    case INIT_CMD_INIT:
        cyg_flag_init(&status_flag);
        critd_init(&crit, "thermal_protect_crit", VTSS_MODULE_ID_THERMAL_PROTECT, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        THERMAL_PROTECT_CRIT_EXIT();

        /* Initialize and register trace resource's */
        memset(&crit_region.if_mutex, 0, sizeof(crit_region.if_mutex));
        crit_region.if_mutex.type = VTSS_ECOS_MUTEX_TYPE_NORMAL;
        vtss_ecos_mutex_init(&crit_region.if_mutex);

        // Create our poller thread. It is resumed once we receive
        // some configuration (in thermal_protect_msg_rx()).
        cyg_thread_create(THREAD_DEFAULT_PRIO,
                          thermal_protect_thread,
                          0,
                          "THERMAL_PROTECT",
                          thermal_protect_thread_stack,
                          sizeof(thermal_protect_thread_stack),
                          &thermal_protect_thread_handle,
                          &thermal_protect_thread_block);

        T_D("enter, cmd=INIT");

        break;
    case INIT_CMD_START:
#ifdef VTSS_SW_OPTION_ICFG
        // Initialize ICLI config
        if ((rc = thermal_protect_icfg_init()) != VTSS_RC_OK) {
            T_E(error_txt(rc));
        }
#endif
        thermal_protect_msg_init();
        break;
    case INIT_CMD_CONF_DEF:
        if (isid == VTSS_ISID_LOCAL) {
            /* Reset local configuration */
            T_D("isid local");
        } else if (VTSS_ISID_LEGAL(isid)) {
            /* Reset configuration (specific switch or all switches) */
            T_D("Restore to default");
            thermal_protect_conf_read(TRUE);
        }
        T_D("enter, cmd=INIT_CMD_CONF_DEF");
        break;

    case INIT_CMD_MASTER_UP:
        thermal_protect_conf_read(FALSE);
        break;

    case INIT_CMD_SWITCH_ADD:
        T_D("SWITCH_ADD, isid: %d", isid);
        thermal_protect_msg_tx_switch_conf(isid); // Update configuration for the switch added
        break;

    case INIT_CMD_SWITCH_DEL:
        T_N("SWITCH_DEL, isid: %d", isid);
        break;

    default:
        break;
    }

    return VTSS_OK;
}
/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
