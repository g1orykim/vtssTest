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
#ifdef VTSS_SW_OPTION_PHY
#include "misc_api.h"
#include "phy_api.h" // For PHY_INST
#endif

#include "critd_api.h"
#include "main.h"
#include "msg_api.h"

#include "led_pow_reduc.h"
#include "led_pow_reduc_api.h"
#include "led_pow_reduc_custom_api.h" // For custom API functions/configuration


#ifdef VTSS_SW_OPTION_PACKET
#include "packet_api.h"
#endif
#include "conf_api.h"
#include "misc_api.h"

#ifdef VTSS_SW_OPTION_DAYLIGHT_SAVING
#include "daylight_saving_api.h"
#endif

#include <sysutil_api.h> // For system_get_tz_off
#include "vtss_misc_api.h"

#ifdef VTSS_SW_OPTION_ICFG
#include "led_pow_reduc_icli_functions.h"
#endif

static led_pow_reduc_msg_t msg_conf; // semaphore-protected message transmission buffer(s).

//****************************************
// TRACE
//****************************************
#if (VTSS_TRACE_ENABLED)

static vtss_trace_reg_t trace_reg = {
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "led_pwr",
    .descr     = "Led_Pow_Reduc control"
};

static vtss_trace_grp_t trace_grps[TRACE_GRP_CNT] = {
    [VTSS_TRACE_GRP_DEFAULT] = {
        .name      = "default",
        .descr     = "Default",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [TRACE_GRP_CONF] = {
        .name      = "conf",
        .descr     = "LED_POW_REDUC configuration",
        .lvl       = VTSS_TRACE_LVL_WARNING,
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
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    }
};
/* Critical region protection protecting the following block of variables */
static critd_t    crit;
#define LED_POW_REDUC_CRIT_ENTER()         critd_enter(&crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define LED_POW_REDUC_CRIT_EXIT()          critd_exit( &crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define LED_POW_REDUC_CRIT_ASSERT_LOCKED() critd_assert_locked(&crit, TRACE_GRP_CRIT, __FILE__, __LINE__)
#else
#define LED_POW_REDUC_CRIT_ENTER()         critd_enter(&crit)
#define LED_POW_REDUC_CRIT_EXIT()          critd_exit( &crit)
#define LED_POW_REDUC_CRIT_ASSERT_LOCKED() critd_assert_locked(&crit)
#endif /* VTSS_TRACE_ENABLED */


//***********************************************
// MISC
//***********************************************


/* Thread variables */
static cyg_handle_t led_pow_reduc_thread_handle;
static cyg_thread   led_pow_reduc_thread_block;
static char         led_pow_reduc_thread_stack[THREAD_DEFAULT_STACK_SIZE];

//************************************************
// Function declartions
//************************************************

//************************************************
// Global Variables
//************************************************
static led_pow_reduc_stack_conf_t   led_pow_reduc_stack_conf;        // Configuration for whole stack (used when we're master, only).
static led_pow_reduc_local_conf_t   led_pow_reduc_local_conf; // Current confiugration for this switch.
static led_pow_reduc_mutex_t        crit_region;
static cyg_flag_t         status_flag; // Flag for signaling that status data from a slave has been received.
static u8 led_intensity_current_value = 100;
static BOOL conf_read_done            = FALSE; // We can not start the thread stuff before the configuration is read,
// so we use this variable to signal that configuration is read.

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
char *led_pow_reduc_error_txt(vtss_rc rc)
{
    switch (rc) {
    case LED_POW_REDUC_ERROR_ISID:
        return "Invalid Switch ID";

    case LED_POW_REDUC_ERROR_FLASH:
        return "Could not store configuration in flash";

    case LED_POW_REDUC_ERROR_SLAVE:
        return "Could not get data from slave switch";

    case LED_POW_REDUC_ERROR_NOT_MASTER:
        return "Switch must to be master";

    case LED_POW_REDUC_ERROR_VALUE:
        return "Invalid value";

    case LED_POW_REDUC_ERROR_T_CONF:
        return "T_max must be higher than T_on";
    default:
        T_D("Default");
        return "";
    }

}

/*************************************************************************
** Message module functions
*************************************************************************/

/* Allocate request/reply buffer */
static void led_pow_reduc_msg_alloc(led_pow_reduc_msg_buf_t *buf, BOOL request)
{
    LED_POW_REDUC_CRIT_ENTER();
    buf->sem = (request ? &msg_conf.request.sem : &msg_conf.reply.sem);
    buf->msg = (request ? &msg_conf.request.msg[0] : &msg_conf.reply.msg[0]);
    LED_POW_REDUC_CRIT_EXIT();
    (void) VTSS_OS_SEM_WAIT(buf->sem);
}

/* Release message buffer */
static void led_pow_reduc_msg_tx_done(void *contxt, void *msg, msg_tx_rc_t rc)
{
    VTSS_OS_SEM_POST(contxt);
}

/* Send message */
static void led_pow_reduc_msg_tx(led_pow_reduc_msg_buf_t *buf, vtss_isid_t isid, size_t len)
{
    msg_tx_adv(buf->sem, led_pow_reduc_msg_tx_done, MSG_TX_OPT_DONT_FREE,
               VTSS_TRACE_MODULE_ID, isid, buf->msg, len);
}




// Getting message from the message module.
static BOOL led_pow_reduc_msg_rx(void *contxt, const void *const rx_msg, const size_t len, const vtss_module_id_t modid, const ulong isid)
{
    led_pow_reduc_msg_id_t msg_id = *(led_pow_reduc_msg_id_t *)rx_msg;
    T_R("Entering led_pow_reduc_msg_rx");
    T_N("msg_id: %d,  len: %zd, isid: %u", msg_id, len, isid);

    switch (msg_id) {

    // Update switch's configuration
    case LED_POW_REDUC_MSG_ID_CONF_SET_REQ: {
        // Got new configuration
        T_DG(TRACE_GRP_CONF, "msg_id = LED_POW_REDUC_MSG_ID_CONF_SET_REQ");
        led_pow_reduc_msg_local_switch_conf_t *msg;
        msg = (led_pow_reduc_msg_local_switch_conf_t *)rx_msg;
        LED_POW_REDUC_CRIT_ENTER();
        led_pow_reduc_local_conf  =  (msg->local_conf); // Update configuration for this switch.
        LED_POW_REDUC_CRIT_EXIT();
        break;
    }


    default:
        T_W("unknown message ID: %d", msg_id);
        break;
    }

    return TRUE;
}



// Transmits a new configuration to a slave switch via the message protocol
//
// In : slave_id - The id of the switch to receive the new configuration.
static void led_pow_reduc_msg_tx_switch_conf (vtss_isid_t slave_id)
{
    // Send the new configuration to the switch in question
    led_pow_reduc_msg_buf_t      buf;
    led_pow_reduc_msg_local_switch_conf_t  *msg;
    led_pow_reduc_msg_alloc(&buf, 1);
    msg = (led_pow_reduc_msg_local_switch_conf_t *)buf.msg;
    LED_POW_REDUC_CRIT_ENTER(); // Protect led_pow_reduc_stack_conf
    msg->local_conf.glbl_conf = led_pow_reduc_stack_conf.glbl_conf;
    LED_POW_REDUC_CRIT_EXIT();
    T_DG(TRACE_GRP_CONF, "Transmit LED_POW_REDUC_MSG_ID_CONF_SET_REQ");
    // Do the transmission
    msg->msg_id = LED_POW_REDUC_MSG_ID_CONF_SET_REQ; // Set msg ID
    led_pow_reduc_msg_tx(&buf, slave_id, sizeof(*msg)); //  Send the msg
}



// Initializes the message protocol
static void led_pow_reduc_msg_init(void)
{
    /* Register for stack messages */
    msg_rx_filter_t filter;

    /* Initialize message buffers */
    VTSS_OS_SEM_CREATE(&msg_conf.request.sem, 1);
    VTSS_OS_SEM_CREATE(&msg_conf.request.sem, 1);

    memset(&filter, 0, sizeof(filter));
    filter.cb = led_pow_reduc_msg_rx;
    filter.modid = VTSS_TRACE_MODULE_ID;
    (void) msg_rx_filter_register(&filter);
}


//************************************************
// Configuration
//************************************************

// Function for sending configuration to all switches in the stack
void led_pow_reduc_conf_to_all (void)
{
    // loop through all isids and send new configuration to slave switch if it exist.
    vtss_isid_t isid;
    for (isid = 1; isid < VTSS_ISID_END; isid++) {
        if (msg_switch_exists(isid)) {
            led_pow_reduc_msg_tx_switch_conf(isid);
        }
    }
}

#ifdef VTSS_SW_OPTION_SILENT_UPGRADE
// Function for applying a configuration done with 2.80 release to newer release.
static void silent_upgrade(led_pow_reduc_flash_conf_t *blk)
{
    led_pow_reduc_flash_conf_280_t led_pow_reduc_flash_conf_280;

    T_IG(TRACE_GRP_CONF, "LLDP silent upgrade");
    memcpy(&led_pow_reduc_flash_conf_280, blk, sizeof(led_pow_reduc_flash_conf_280)); // blk do at this point in time contain the 2.80 release flash layout
    memset(blk, 0, sizeof(*blk)); //Default all configuration to a known value.

    blk->version                               = led_pow_reduc_flash_conf_280.version;
    blk->stack_conf.glbl_conf.on_at_err        = led_pow_reduc_flash_conf_280.stack_conf.glbl_conf.on_at_err;
    blk->stack_conf.glbl_conf.maintenance_time = led_pow_reduc_flash_conf_280.stack_conf.glbl_conf.maintenance_time;

    int i;
    u8 intensity = led_pow_reduc_flash_conf_280.stack_conf.glbl_conf.led_timer_intensity[0];

    for (i = 0; i < LED_POW_REDUC_TIMERS_CNT; i++) {
        if (led_pow_reduc_flash_conf_280.stack_conf.glbl_conf.led_timer[i] != LED_POW_REDUC_TIMER_UNUSED) {
            intensity = led_pow_reduc_flash_conf_280.stack_conf.glbl_conf.led_timer_intensity[i];
        }
        blk->stack_conf.glbl_conf.led_timer_intensity[i] = intensity;
    }
}
#endif

// Function reading configuration from flash. Restoring
// to dafault values is create = TRUE or flash access fails.
//
// In : Create : Restore configuration to default setting if set to TRUE
static void  led_pow_reduc_conf_read(BOOL create)
{
    led_pow_reduc_flash_conf_t *led_pow_reduc_flash_conf; // Configuration in flash
    ulong             size;
    BOOL              do_create = FALSE;      // Set if we need to create new configuration in flash

    T_RG(TRACE_GRP_CONF, "Entering led_pow_reduc_conf_read");

    if (misc_conf_read_use()) {
        // Get configration from flash (if possible)
        if ((led_pow_reduc_flash_conf = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_LED_POW_REDUC_CONF, &size)) == NULL ||
            size != sizeof(led_pow_reduc_flash_conf_t)) {
#ifdef VTSS_SW_OPTION_SILENT_UPGRADE
            if (led_pow_reduc_flash_conf != NULL && size == sizeof(led_pow_reduc_flash_conf_280_t)) {
                silent_upgrade(led_pow_reduc_flash_conf);
            } else {
#endif
                led_pow_reduc_flash_conf = conf_sec_create(CONF_SEC_GLOBAL, CONF_BLK_LED_POW_REDUC_CONF, sizeof(led_pow_reduc_flash_conf_t));
                T_W("conf_sec_open failed or size mismatch, creating defaults");
                do_create = TRUE;
#ifdef VTSS_SW_OPTION_SILENT_UPGRADE
            }
#endif
        } else if (led_pow_reduc_flash_conf->version != LED_POW_REDUC_CONF_VERSION) {
            T_W("version mismatch, creating defaults");
            do_create = TRUE;
        } else {
            do_create = create;
        }
    } else {
        led_pow_reduc_flash_conf = NULL;
        do_create                = TRUE;
    }

    LED_POW_REDUC_CRIT_ENTER();

    // Create default configuration in flash
    if (do_create) {
        //Set default configuration
        int timer_index;

        T_DG(TRACE_GRP_CONF, "Restore to default value - block size =  %u ", sizeof(led_pow_reduc_flash_conf_t));
        led_pow_reduc_stack_conf.glbl_conf.maintenance_time = LED_POW_REDUC_MAINTENANCE_TIME_DEFAULT;
        led_pow_reduc_local_conf.glbl_conf.on_at_err = LED_POW_REDUC_ON_AT_ERR_DEFAULT;
        for (timer_index = LED_POW_REDUC_TIMERS_MIN; timer_index <= LED_POW_REDUC_TIMERS_MAX; timer_index++) {
            led_pow_reduc_stack_conf.glbl_conf.led_timer_intensity[timer_index] = LED_POW_REDUC_INTENSITY_DEFAULT;
        }
        if (led_pow_reduc_flash_conf != NULL) {
            memset(led_pow_reduc_flash_conf, 0, sizeof(*led_pow_reduc_flash_conf));
            led_pow_reduc_flash_conf->stack_conf = led_pow_reduc_stack_conf;
        }
    } else {
        if (led_pow_reduc_flash_conf != NULL) {
            led_pow_reduc_stack_conf = led_pow_reduc_flash_conf->stack_conf;
        }
    }

    LED_POW_REDUC_CRIT_EXIT();

    // loop through all isids and send new configuration to slave switch if it exist.
    led_pow_reduc_conf_to_all();

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    // Write our settings back to flash
    if (led_pow_reduc_flash_conf) {
        led_pow_reduc_flash_conf->version = LED_POW_REDUC_CONF_VERSION;
        T_NG(TRACE_GRP_CONF, "Closing CONF_BLK_LED_POW_REDUC_CONF");
        conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_LED_POW_REDUC_CONF);
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
}

// Store the current configuration in flash
static vtss_rc store_conf (void)
{
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    T_DG(TRACE_GRP_CONF, "Storing LED_POW_REDUC configuration in flash");
    vtss_rc rc = LED_POW_REDUC_ERROR_FLASH;
    led_pow_reduc_flash_conf_t *led_pow_reduc_flash_conf;
    ulong size;

#ifdef VTSS_SW_OPTION_SILENT_UPGRADE
    led_pow_reduc_glbl_conf_280_t *led_pow_reduc_flash_conf_280;
#endif

    if ((led_pow_reduc_flash_conf = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_LED_POW_REDUC_CONF, &size)) != NULL) {
        if (size == sizeof(*led_pow_reduc_flash_conf)) {
            LED_POW_REDUC_CRIT_ENTER();
            led_pow_reduc_flash_conf->stack_conf = led_pow_reduc_stack_conf;
            LED_POW_REDUC_CRIT_EXIT();
            rc = VTSS_OK;
        } else {
            T_W("Could not store LED_POW_REDUC configuration - Size did not match");
        }
        conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_LED_POW_REDUC_CONF);
    } else {
        T_W("Could not store LED_POW_REDUC configuration");
    }

    return rc;
#else
    return VTSS_RC_OK;
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
}


// Check the maintenance timer and set the LED intensity (MUST be crit_region protected for protecting led_pow_reduc_local_conf )
//
// IN: new_maintenance_time - Value that the maintenance_timer should be set to if new_maintenance_time_vld = TRUE
//     new_maintenance_time_vld - Indicates if new_maintenance_time is valid
static void led_pow_reduc_maintenance_timer(u32 new_maintenance_time, BOOL new_maintenance_time_vld)
{
    static u16 maintenance_timer = 0;

    if (new_maintenance_time_vld) {
        T_D("Setting maintenance_timer to %d", maintenance_timer);
        maintenance_timer = new_maintenance_time;
    } else {
        if (maintenance_timer > 0) {
            maintenance_timer--;
            T_D("Decreasing maintenance_timer to %d", maintenance_timer);
        }
    }


    LED_POW_REDUC_CRIT_ASSERT_LOCKED(); // Make sure that we are crit_region protected.

    // Turn on LEDs at full power ( led_pow_reduc_custom_on_at_error macro is defined in led_pow_reduc_custom_api.h)
    if (maintenance_timer > 0 || (led_pow_reduc_custom_on_at_error && led_pow_reduc_local_conf.glbl_conf.on_at_err)) {
        T_D("Setting intensity to 100 pct");
        led_pow_reduc_custom_set_led_intensity(100);
    } else {
        T_D("Setting intensity to %d", led_intensity_current_value);
        led_pow_reduc_custom_set_led_intensity(led_intensity_current_value);
    }

}

static void led_pow_reduc_port_change(vtss_port_no_t port_no, port_info_t *info)
{
    LED_POW_REDUC_CRIT_ENTER();
    T_D_PORT(port_no, "maintenance_time:%d", led_pow_reduc_local_conf.glbl_conf.maintenance_time);
    led_pow_reduc_maintenance_timer(led_pow_reduc_local_conf.glbl_conf.maintenance_time, TRUE);
    LED_POW_REDUC_CRIT_EXIT();
}

// Function that checks the LED intensity timers and updates the LED intensity accordingly
static void led_pow_reduc_chk_timers (void)
{
    time_t t = time(NULL);
    struct tm *timeinfo_p;


#if VTSS_SWITCH && defined(VTSS_SW_OPTION_SYSUTIL)
    /* Correct for timezone */
    t += (system_get_tz_off() * 60);
#endif
#ifdef VTSS_SW_OPTION_DAYLIGHT_SAVING
    /* Correct for DST */
    t += (time_dst_get_offset() * 60);
#endif
    timeinfo_p = localtime(&t);

    T_N("Time %02d:%02d:%02d",
        timeinfo_p->tm_hour,
        timeinfo_p->tm_min,
        timeinfo_p->tm_sec);

    int timer_index;
    LED_POW_REDUC_CRIT_ENTER();
    for (timer_index = LED_POW_REDUC_TIMERS_MIN; timer_index <= LED_POW_REDUC_TIMERS_MAX; timer_index++) {
        T_R("Time %02d:%02d:%02d, intensity:%d, index:%d ",
            timeinfo_p->tm_hour,
            timeinfo_p->tm_min,
            timeinfo_p->tm_sec,
            led_pow_reduc_local_conf.glbl_conf.led_timer_intensity[timer_index], timer_index);

        T_R("intensity[%d]:%d", timer_index, led_pow_reduc_local_conf.glbl_conf.led_timer_intensity[timer_index]);

        // Set intensity corresponding to the current hour
        if (timer_index == timeinfo_p->tm_hour) {
            T_D("Setting LED led_intensity_current_value to %d ", led_pow_reduc_local_conf.glbl_conf.led_timer_intensity[timer_index]);
            led_intensity_current_value = led_pow_reduc_local_conf.glbl_conf.led_timer_intensity[timer_index];
        }
    }
    LED_POW_REDUC_CRIT_EXIT();

}

/****************************************************************************
* Module thread
****************************************************************************/
static void led_pow_reduc_thread(cyg_addrword_t data)
{
    // ***** Go into loop **** //
    T_R("Entering led_pow_reduc_thread");

    // This will block this thread from running further until the PHYs are initialized.
    port_phy_wait_until_ready();

    // Initializing the LED power reduction API
    led_pow_reduc_custom_api_init; // Macro defined in led_pow_reduc_custom_api.h

    // Wait for configuration to be read from flash
    while (!conf_read_done) {
        VTSS_OS_MSLEEP(1000);
    }

    for (;;) {
        VTSS_OS_MSLEEP(1000);
        led_pow_reduc_chk_timers();
        LED_POW_REDUC_CRIT_ENTER();
        led_pow_reduc_maintenance_timer(0, FALSE);
        LED_POW_REDUC_CRIT_EXIT();
    }
}



/****************************************************************************/
/*  API functions (management  functions)                                   */
/****************************************************************************/

// Function that returns the next index where the intensity changes
//
// In/out : switch_conf - Pointer to configuration struct where the current configuration is copied to.
//
u8 led_pow_reduc_mgmt_next_change_get(u8 start_index)
{
    u8 timer_index;
    u8 intensity;
    u8 start_intensity;

    // Find if there is an intensity change from starting time until midnight
    for (timer_index = start_index; timer_index <= LED_POW_REDUC_TIMERS_MAX; timer_index++) {

        LED_POW_REDUC_CRIT_ENTER(); // Protect led_pow_reduc_local_conf
        intensity = led_pow_reduc_local_conf.glbl_conf.led_timer_intensity[timer_index];
        start_intensity = led_pow_reduc_local_conf.glbl_conf.led_timer_intensity[start_index];
        LED_POW_REDUC_CRIT_EXIT();

        T_I("start_intensity:%d, intensity:%d, timer_index:%d", start_intensity, intensity, timer_index);
        if (intensity != start_intensity) {
            return timer_index;
        }

    }
    // Find if there is an intensity change from midnight until start time
    for (timer_index = LED_POW_REDUC_TIMERS_MIN; timer_index <= start_index; timer_index++) {

        LED_POW_REDUC_CRIT_ENTER(); // Protect led_pow_reduc_local_conf
        intensity = led_pow_reduc_local_conf.glbl_conf.led_timer_intensity[timer_index];
        start_intensity = led_pow_reduc_local_conf.glbl_conf.led_timer_intensity[start_index];
        LED_POW_REDUC_CRIT_EXIT();

        T_I("start_intensity:%d, intensity:%d, timer_index:%d", start_intensity, intensity, timer_index);
        if (intensity != start_intensity) {
            return timer_index;
        }
    }

    // There is no change in intensity, so we signal that by returning the start_index
    return start_index;
}

// Function for setting the led timer interval
vtss_rc led_pow_reduc_mgmt_timer_set(u8 start_index, u8 end_index, u8 intensity)
{
    u8 led_index;

    // We allow both 00:00 and 24:00 as midnight, but in code midnight is given as index 0
    if (end_index == 24) {
        end_index = 0;
    }
    if (start_index == 24) {
        start_index = 0;
    }


    T_I("end_index:%d, start_index:%d, intensity:%d", end_index, start_index, intensity);

    // If start index = end_index then it mean set all timers to same value.
    if (start_index == end_index) {
        start_index = LED_POW_REDUC_TIMERS_MIN;
        end_index = LED_POW_REDUC_TIMERS_CNT;
    }

    // Crossing midnight
    if (start_index > end_index) {
        for (led_index = start_index; led_index <= 23; led_index++) {
            LED_POW_REDUC_CRIT_ENTER();
            led_pow_reduc_stack_conf.glbl_conf.led_timer_intensity[led_index] = intensity;
            LED_POW_REDUC_CRIT_EXIT();
        }
        start_index = 0; // Continue from midnight
    }

    for (led_index = start_index; led_index < end_index; led_index++) {
        T_I("led_index:%d, intensity:%d", led_index, intensity);
        LED_POW_REDUC_CRIT_ENTER();
        led_pow_reduc_stack_conf.glbl_conf.led_timer_intensity[led_index] = intensity;
        LED_POW_REDUC_CRIT_EXIT();
    }

    // Transfer new configuration to the switch in question.
    led_pow_reduc_conf_to_all();

    // Store the new configuration in flash
    return store_conf();
}

// Function for initializing the timer struct used for looping through all timers for find the timer intervals
//
// In/out : current_timer - Pointer to a timer record containing the timer information.
void led_pow_reduc_mgmt_timer_get_init(led_pow_reduc_timer_t *current_timer)
{
    current_timer->start_index = 0;
    current_timer->end_index = 0;
    current_timer->first_index = 0;
    current_timer->start_new_search = TRUE;
}


// Function that can be used for looping through all timers for find the timer intervals
//
// In/out : current_timer - Pointer to a timer record containing the timer information.
// Return : FALSE when all timer has been "passed" else TRUE
BOOL led_pow_reduc_mgmt_timer_get(led_pow_reduc_timer_t *current_timer)
{
    T_I("end_index:%d, first_index:%d, start_index:%d, search:%d",
        current_timer->end_index, current_timer->first_index, current_timer->start_index, current_timer->start_new_search);
    if (!current_timer->start_new_search && (current_timer->end_index == current_timer->first_index)) {
        return FALSE; // OK, now we have been through all timers
    }

    // Starting a new loop through all timer intervals
    if (current_timer->start_new_search) {
        current_timer->first_index = led_pow_reduc_mgmt_next_change_get(0);
        current_timer->start_index = current_timer->first_index;
        current_timer->start_new_search = FALSE;
    } else {
        current_timer->start_index = current_timer->end_index; // Continue from where we left off, then last time the function was called.
    }

    if (current_timer->first_index == 0) {
        // Intensity is always the same.
        current_timer->start_index = 0;
        current_timer->end_index = 0;
    } else {
        current_timer->end_index = led_pow_reduc_mgmt_next_change_get(current_timer->start_index);
        T_I("end_index:%d, first_index:%d, start_index:%d", current_timer->end_index, current_timer->first_index, current_timer->start_index);
    }

    return TRUE; // More timer intervals exists.
}

//
// Function that returns the current configuration for a switch.
//
// In/out : switch_conf - Pointer to configuration struct where the current configuration is copied to.
//
void led_pow_reduc_mgmt_get_switch_conf(led_pow_reduc_local_conf_t *switch_conf)
{
    LED_POW_REDUC_CRIT_ENTER();
    // All switches have the same configuration
    memcpy(&switch_conf->glbl_conf, &led_pow_reduc_stack_conf.glbl_conf, sizeof(led_pow_reduc_stack_conf.glbl_conf));
    LED_POW_REDUC_CRIT_EXIT();
}


// Function for setting the current configuration for a switch.
//
// In : switch_conf - Pointer to configuration struct with the new configuration.
//
// Return : VTSS error code
vtss_rc led_pow_reduc_mgmt_set_switch_conf(led_pow_reduc_local_conf_t  *new_switch_conf)
{
    BOOL change = (led_pow_reduc_stack_conf.glbl_conf.maintenance_time != new_switch_conf->glbl_conf.maintenance_time) ||
                  (led_pow_reduc_stack_conf.glbl_conf.on_at_err != new_switch_conf->glbl_conf.on_at_err); // Store if maintenance time is changed

    T_DG(TRACE_GRP_CONF, "Conf. changed");
    // Configuration changes only allowed by master
    if (!msg_switch_is_master()) {
        return LED_POW_REDUC_ERROR_NOT_MASTER;
    }

    // Ok now we can do the configuration
    LED_POW_REDUC_CRIT_ENTER();
    memcpy(&led_pow_reduc_stack_conf.glbl_conf, &new_switch_conf->glbl_conf, sizeof(led_pow_reduc_glbl_conf_t)); // Update the configuration for the switch
    LED_POW_REDUC_CRIT_EXIT();

    // Transfer new configuration to the switch in question.
    led_pow_reduc_conf_to_all();

    LED_POW_REDUC_CRIT_ENTER();
    // Activate maintenance_timer when LED power reduction configuration is changed.
    if (change) {
        led_pow_reduc_maintenance_timer(led_pow_reduc_stack_conf.glbl_conf.maintenance_time, TRUE);
    }
    LED_POW_REDUC_CRIT_EXIT();

    // Store the new configuration in flash
    return store_conf();
}
/****************************************************************************/
/*  Initialization functions                                                */
/****************************************************************************/

vtss_rc led_pow_reduc_init(vtss_init_data_t *data)
{

    vtss_isid_t isid = data->isid; // Get switch id

    if (data->cmd == INIT_CMD_INIT) {
        // Initialize and register trace ressources
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);
    }
    T_D("led_pow_reduc_init enter, cmd=%d", data->cmd);

    switch (data->cmd) {
    case INIT_CMD_INIT:
        cyg_flag_init(&status_flag);
        critd_init(&crit, "led_pow_reduc_crit", VTSS_MODULE_ID_LED_POW_REDUC, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        LED_POW_REDUC_CRIT_EXIT();

        /* Initialize and register trace resource's */
        memset(&crit_region.if_mutex, 0, sizeof(crit_region.if_mutex));
        crit_region.if_mutex.type = VTSS_ECOS_MUTEX_TYPE_NORMAL;
        vtss_ecos_mutex_init(&crit_region.if_mutex);
        cyg_thread_create(THREAD_DEFAULT_PRIO,
                          led_pow_reduc_thread,
                          0,
                          "LED_POW_REDUC",
                          led_pow_reduc_thread_stack,
                          sizeof(led_pow_reduc_thread_stack),
                          &led_pow_reduc_thread_handle,
                          &led_pow_reduc_thread_block);
        cyg_thread_resume(led_pow_reduc_thread_handle);

        T_D("enter, cmd=INIT");

        // Initialize icfg
#ifdef VTSS_SW_OPTION_ICFG
        if (led_pow_reduc_icfg_init() != VTSS_RC_OK) {
            T_E("ICFG not initialized correctly");
        }
#endif

        break;
    case INIT_CMD_START:
        led_pow_reduc_msg_init();
        (void) port_change_register(VTSS_MODULE_ID_LED_POW_REDUC, led_pow_reduc_port_change);        // Prepare callback function for link up/down for ports
        break;
    case INIT_CMD_CONF_DEF:
        if (isid == VTSS_ISID_LOCAL) {
            /* Reset local configuration */
            T_D("isid local");
        } else if (VTSS_ISID_LEGAL(isid)) {
            /* Reset configuration (specific switch or all switches) */
            T_D("Restore to default");
            led_pow_reduc_conf_read(TRUE);
        }
        T_D("enter, cmd=INIT_CMD_CONF_DEF");
        break;

    case INIT_CMD_MASTER_UP:
        led_pow_reduc_conf_read(FALSE);
        conf_read_done = TRUE; // Signal that reading of configuration is done.
        break;

    case INIT_CMD_SWITCH_ADD:
        T_D("SWITCH_ADD, isid: %d", isid);
        led_pow_reduc_msg_tx_switch_conf(isid); // Update configuration for the switch added
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
