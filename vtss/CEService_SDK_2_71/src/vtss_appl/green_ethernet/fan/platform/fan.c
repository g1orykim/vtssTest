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

/*lint -esym(459,status_flag) */




#include "critd_api.h"
#include "main.h"
#include "msg_api.h"

#include "fan.h"
#include "fan_api.h"
#include "fan_custom_api.h"
#include "vtss_misc_api.h"
#include "misc_api.h"

#ifdef VTSS_SW_OPTION_PACKET
#include "packet_api.h"
#endif
#include "conf_api.h"


#ifdef VTSS_SW_OPTION_ICFG
#include "fan_icli_functions.h"
#endif
static fan_msg_t msg_conf; // semaphore-protected message transmission buffer(s).
//****************************************
// TRACE
//****************************************
#if (VTSS_TRACE_ENABLED)

static vtss_trace_reg_t trace_reg = {
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "fan",
    .descr     = "Fan control"
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
        .descr     = "FAN configuration",
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
#define FAN_CRIT_ENTER()         critd_enter(&crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define FAN_CRIT_EXIT()          critd_exit( &crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define FAN_CRIT_ASSERT_LOCKED() critd_assert_locked(&crit, TRACE_GRP_CRIT, __FILE__, __LINE__)
#else
#define FAN_CRIT_ENTER()         critd_enter(&crit)
#define FAN_CRIT_EXIT()          critd_exit( &crit)
#define FAN_CRIT_ASSERT_LOCKED() critd_assert_locked(&crit)
#endif /* VTSS_TRACE_ENABLED */


//***********************************************
// MISC
//***********************************************


/* Thread variables */
static cyg_handle_t fan_thread_handle;
static cyg_thread   fan_thread_block;
static char         fan_thread_stack[THREAD_DEFAULT_STACK_SIZE];

//************************************************
// Function declartions
//************************************************

//************************************************
// Global Variables
//************************************************
static fan_stack_conf_t   fan_stack_conf;        // Configuration for whole stack (used when we're master, only).
static fan_local_conf_t   fan_local_conf; // Current confiugration for this switch.
static fan_mutex_t        crit_region;
static fan_local_status_t switch_status; // Status from a slave switch.
static cyg_flag_t         status_flag; // Flag for signaling that status data from a slave has been received.
static vtss_mtimer_t timer; // Timer for timeout
static BOOL fan_init_done = FALSE;
// 0 = fan off, 255 = Fan at full speed
static u8 fan_speed_lvl = 255; // The fan speed level ( PWM duty cycle). Start at full speed.

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
char *fan_error_txt(vtss_rc rc)
{
    switch (rc) {
    case FAN_ERROR_ISID:
        return "Invalid Switch ID";

    case FAN_ERROR_FLASH:
        return "Could not store configuration in flash";

    case FAN_ERROR_SLAVE:
        return "Could not get data from slave switch";

    case FAN_ERROR_NOT_MASTER:
        return "Switch must to be master";

    case FAN_ERROR_VALUE:
        return "Invalid value";

    case FAN_ERROR_T_CONF:
        return "Max. Temperature must be higher than 'On' temperature";

    case FAN_ERROR_FAN_NOT_RUNNING:
        return "Fan is supposed to be running, but fan speed is 0 rpm. Please make sure that the fan isn't blocked";

    default:
        T_D("Default");
        return "";
    }

}


// Checks if the switch is master and that the isid is a valid isid.
//
// In : isid - The isid to be checked
//
// Return : If isid is valid and switch is master then return VTSS_OK, else return error code
vtss_rc fan_is_master_and_isid_legal (vtss_isid_t isid)
{
    if (!msg_switch_is_master()) {
        T_W("Configuration change only allowed from master switch");
        return FAN_ERROR_NOT_MASTER;
    }

    if (!VTSS_ISID_LEGAL(isid)) {
        return FAN_ERROR_ISID;
    }

    return VTSS_OK;
}



void fan_get_fan_spec(vtss_fan_conf_t *fan_spec)
{
    // Setup the kind of FAN that is used.

    fan_spec->fan_low_pol  = FAN_CUSTOM_POL;    // FAN PWM polarity.
    fan_spec->fan_open_col = FAN_CUSTOM_OC;    // Open collector
    fan_spec->type         = FAN_CUSTOM_TYPE(VTSS_ISID_LOCAL);
    fan_spec->ppr          = FAN_CUSTOM_PPR;

    // To make lint happy - In this case it is OK to have a constant
    // if (true) in the case where FAN_CUSTOM_TYPE = VTSS_FAN_4_WIRE_TYPE
    /*lint --e{550} --e{506} */
    if (fan_spec->type == VTSS_FAN_4_WIRE_TYPE) {
        fan_spec->fan_pwm_freq = VTSS_FAN_PWM_FREQ_25KHZ; // 4 wire fans supports high pwm frequency
    } else {
        fan_spec->fan_pwm_freq = VTSS_FAN_PWM_FREQ_20HZ;
    }


    T_N("fan_low_pol:%d, fan_open_col:%d, type:%d, ppr:%u, fan_pwm_freq:%d",
        fan_spec->fan_low_pol, fan_spec->fan_open_col, fan_spec->type, fan_spec->ppr, fan_spec->fan_pwm_freq);
}


//************************************************
// Fan control
//************************************************

// Function that returns this switch's status
//
// Out : status - Pointer to where to put chip status
static void fan_get_local_status(fan_local_status_t *status)
{
    u32  rotation_count = 0;
    vtss_fan_conf_t fan_spec;
    vtss_rc rc;

    u32 rpm;

    u32 fan_count_diff;


    FAN_CRIT_ENTER();
    static u32 last_rpm = 0;
    static u32  last_rotation_count = 0;
    static cyg_tick_count_t last_time;
    u16 fan_speed = 0;
    i16 chip_temp[FAN_TEMPERATURE_SENSOR_CNT_MAX];
    cyg_tick_count_t current_time;
    u32 time_diff;


    memset(chip_temp, 0, sizeof(chip_temp)); // Initialize the array (Making Lint happy)

    // Get temperature
    if (fan_get_temperture(VTSS_ISID_LOCAL, chip_temp) != VTSS_OK) {
        T_E("Could not get chip temperature");
        memset(status, 0, sizeof(*status));
        FAN_CRIT_EXIT();
        return;
    } else {
        memcpy(status->chip_temp, chip_temp, sizeof(chip_temp));
    }



    fan_get_fan_spec(&fan_spec);

    // Get fan speed
    status->fan_speed_setting_pct = fan_speed_lvl * 100 / 255;

    current_time = cyg_current_time(); // Time where the rotation count was read

    T_N("current_time = %lld, last_time = %lld", current_time, last_time);

    // There shall always be at least 0.5  s sec. between reading the rotation count in order
    // to get correct value (Specially at low speed), so if there is less than 0.5 sec since last
    // eread we simply use the last rpm found.
    if (last_time > current_time - 50) {
        rpm  = last_rpm;
        T_D("Reusing last rpm:%u", last_rpm);
    } else {
        // Get rotation count
        if ((rc = vtss_fan_rotation_get(NULL, &fan_spec, &rotation_count)) != VTSS_OK) {
            T_R("%s", fan_error_txt(rc));
        }

        time_diff = current_time - last_time; // Calculate the time since last read of rotation counter


        if (rotation_count >= last_rotation_count) {
            fan_count_diff = rotation_count - last_rotation_count; // Calculate the number of rotation counts since last read
        } else {
            // Counter wrap around
            fan_count_diff = 0xFFFF - last_rotation_count + rotation_count ;
        }


        rpm =  (fan_count_diff * CYGNUM_HAL_RTC_DENOMINATOR * 60) / time_diff; // Calculate the Round per minute.

        T_N("new rpm:%u, fan_count_diff:%u, last_rotation_count:%u, rotation_count:%u, time_diff:%u",
            rpm, fan_count_diff, last_rotation_count, rotation_count, time_diff);

        // Store current rotation count and time
        last_rotation_count = rotation_count;
        last_time           = current_time;
    }

    last_rpm = rpm; // Remember the last rpm found

    FAN_CRIT_EXIT();
    // Check compile customization is valid
    if (fan_spec.ppr == 0) {
        T_E("FAN PPR must not to set to zero");

    } else {
        // Adjust for PPR (pulses per rotation)
        if (fan_spec.type == VTSS_FAN_3_WIRE_TYPE) {
            // If the fan is a 3-wire type, the pulses are only valid when PWM pulse is high.
            // We need to take that into account. See AN0xxx for how the calculation is done.

            // Get PWM duty cycle
            u8 duty_cycle = 0;
            if (fan_speed_level_get(VTSS_ISID_LOCAL, &duty_cycle) != VTSS_OK) {
                duty_cycle = 0;
            }

            if (duty_cycle == 0 || fan_spec.ppr == 0) {
                // Avoid divide by zero.
                T_D("Setting Fan Speed to 0");
                fan_speed  = 0;
            } else {
                fan_speed = rpm * duty_cycle / 0xFF / fan_spec.ppr;
                T_N("Calculating Fan Speed. fan_speed:%d, rpm:%u, duty_cycle:%d, ppr:%u", fan_speed, rpm, duty_cycle, fan_spec.ppr);
            }
        } else {
            fan_speed = rpm / fan_spec.ppr;
        }
    }


    // Round up/down to the nearest 100.
    fan_speed = fan_speed / 100;
    fan_speed = fan_speed * 100;

    // Return the fan speed
    status->fan_speed = fan_speed;
}


// Function for finding the highest system temperature in case there are multiple temperature sensors.
//
// In : temperature_array : Array with all the sensors readings
//
// Return : The value of the sensor with the highest temperature

i16 fan_find_highest_temp(i16 *temperature_array)
{
    u8 sensor;
    i16 max_temp = temperature_array[0];

    // Loop through all temperature sensors values and find the highest temperature.
    int sensor_cnt = FAN_TEMPERATURE_SENSOR_CNT(VTSS_ISID_LOCAL);
    for (sensor = 0; sensor < sensor_cnt; sensor++) {
        if (temperature_array[sensor] > max_temp) {
            max_temp = temperature_array[sensor];
        }
    }

    return max_temp;
}

// See Section 4 in AN0xxxx
// In : reset_last_temp - The Fan speed is only updated when the temperature has changed.
//                        If fan_control is call with "reset_last_temp" set to TRUE, the temperature
//                        "memory" is cleared, and the fan speed will be re-configured the next time fan_control is called.
static void fan_control(BOOL reset_last_temp)
{
    u8 new_fan_speed_lvl = 0;

    T_R("Entering fan_control");

    // Make sure that we done try and adjust fan before the controller is initialised.
    if (!fan_init_done) {
        return;
    }

    FAN_CRIT_ENTER();
    static i8 last_temp = 0; // The Temperature the last time the fan speed was adjusted

    if (reset_last_temp) {
        last_temp = 0;
        FAN_CRIT_EXIT();
        return;
    }
    FAN_CRIT_EXIT();

    //Because some result from the calculations below will be lower the zero, and we don't
    //to use floating point operations, we multiply with the resolution constant.
    const u16 resolution = 1000;
    //
    i32 fan_speed_lvl_pct; // The fan speed in percent. ( Negative number = fan stopped )
    const u8 fan_level_steps_pct = 10;  // How much the fan shall change for every adjustment.

    // Get the chip temperature
    fan_local_status_t status;
    fan_get_local_status(&status); // Get chip temperature

    FAN_CRIT_ENTER();
    // Add some hysteresis to avoid that the fan is adjusted all the time.
    if ((last_temp < FAN_TEMP_MAX && fan_find_highest_temp(&status.chip_temp[0]) > last_temp + 1) ||
        (last_temp > FAN_TEMP_MIN && fan_find_highest_temp(&status.chip_temp[0]) < last_temp - 1)) {

        // Figure 4 in AN0xxx shows a state machine, but instead of a state machine I have
        // use the following which does the same. In this way we don't have to have fixed
        // number of states (Each fan speed level corresponds to a state).
        i32 delta_t = (fan_local_conf.glbl_conf.t_max - fan_local_conf.glbl_conf.t_on) * resolution *  fan_level_steps_pct / 100; // delta_t is described in AN0xxx section 4.

        // Calculate the fan speed in percent
        if (delta_t == 0) {
            // avoid divide by zero
            fan_speed_lvl_pct = 0;
        } else {
            fan_speed_lvl_pct = (fan_find_highest_temp(&status.chip_temp[0]) - fan_local_conf.glbl_conf.t_on) * resolution * fan_level_steps_pct / delta_t ;
        }

        // Make sure that fan that the fan speed doesn't get below the PWM need to keep the fan running
        if ((fan_speed_lvl_pct < FAN_CUSTOM_MIN_PWM_PCT(VTSS_ISID_LOCAL)) && (fan_speed_lvl_pct > 0)) {
            fan_speed_lvl_pct = FAN_CUSTOM_MIN_PWM_PCT(VTSS_ISID_LOCAL);
        }

        if (fan_find_highest_temp(&status.chip_temp[0]) > fan_local_conf.glbl_conf.t_on) {
            if (fan_speed_lvl_pct > 100) {
                new_fan_speed_lvl = 255;
            } else {
                new_fan_speed_lvl = 255 * fan_speed_lvl_pct / 100;
            }

        } else {
            new_fan_speed_lvl = 0;
        }
        T_I("new_fan_speed_lvl = %d, chip_temp =%d, delta_t = %u, temp_fan_speed_lvl_pct = %u, fan_speed_lvl = %d",
            new_fan_speed_lvl, fan_find_highest_temp(&status.chip_temp[0]), delta_t, fan_speed_lvl_pct, fan_speed_lvl);


        // Set new fan speed level
        if (new_fan_speed_lvl != fan_speed_lvl) {

            // Some fans can not start at a low PWM pulse width and needs to be kick started at full speed.
            if ((fan_speed_lvl_pct < FAN_CUSTOM_KICK_START_LVL_PCT(VTSS_ISID_LOCAL)) && (fan_speed_lvl_pct != 0)) {
                // Start fan at full speed
                if (fan_speed_level_set(VTSS_ISID_LOCAL, 255) != VTSS_OK) {
                    T_E("Could not set fan cooling level");
                };
                VTSS_OS_MSLEEP(FAN_CUSTOM_KICK_START_ON_TIME);
            }

            T_N("Setting fan_speed_lvl %d", new_fan_speed_lvl);
            if (fan_speed_level_set(VTSS_ISID_LOCAL, new_fan_speed_lvl) != VTSS_OK) {
                T_E("Could not set fan cooling level");
            }
        }

        fan_speed_lvl = new_fan_speed_lvl;
        last_temp = fan_find_highest_temp(&status.chip_temp[0]);
    }
    FAN_CRIT_EXIT();
}


/*************************************************************************
** Message module functions
*************************************************************************/

/* Allocate request/reply buffer */
static void fan_msg_alloc(fan_msg_buf_t *buf, BOOL request)
{
    FAN_CRIT_ENTER();
    buf->sem = (request ? &msg_conf.request.sem : &msg_conf.reply.sem);
    buf->msg = (request ? &msg_conf.request.msg[0] : &msg_conf.reply.msg[0]);
    FAN_CRIT_EXIT();
    (void) VTSS_OS_SEM_WAIT(buf->sem);
}

/* Release message buffer */
static void fan_msg_tx_done(void *contxt, void *msg, msg_tx_rc_t rc)
{
    VTSS_OS_SEM_POST(contxt);
}

/* Send message */
static void fan_msg_tx(fan_msg_buf_t *buf, vtss_isid_t isid, size_t len)
{
    msg_tx_adv(buf->sem, fan_msg_tx_done, MSG_TX_OPT_DONT_FREE,
               VTSS_TRACE_MODULE_ID, isid, buf->msg, len);
}


// Transmits status from a slave to the master
//
// In : master_id - The master switch's id
static void fan_msg_tx_switch_status (vtss_isid_t master_id)
{
    // Send the new configuration to the switch in question
    fan_msg_buf_t      buf;
    fan_msg_local_switch_status_t  *msg;

    fan_local_status_t status;


    fan_get_local_status(&status); // Get chip temperature and fan rotation count

    fan_msg_alloc(&buf, 1);
    msg = (fan_msg_local_switch_status_t *)buf.msg;
    msg->status = status;

    // Do the transmission
    msg->msg_id = FAN_MSG_ID_STATUS_REP; // Set msg ID
    fan_msg_tx(&buf, master_id, sizeof(*msg)); //  Send the msg
}




// Getting message from the message module.
static BOOL fan_msg_rx(void *contxt, const void *const rx_msg, const size_t len, const vtss_module_id_t modid, const ulong isid)
{
    fan_msg_id_t msg_id = *(fan_msg_id_t *)rx_msg;
    T_R("msg_id: %d,  len: %zd, isid: %u", msg_id, len, isid);

    switch (msg_id) {
    case FAN_MSG_ID_CONF_SET_REQ: {
        // Update switch's configuration
        // Got new configuration
        T_DG(TRACE_GRP_CONF, "msg_id = FAN_MSG_ID_CONF_SET_REQ");
        fan_msg_local_switch_conf_t *msg;
        msg = (fan_msg_local_switch_conf_t *)rx_msg;
        FAN_CRIT_ENTER();
        fan_local_conf  =  (msg->local_conf); // Update configuration for this switch.
        FAN_CRIT_EXIT();

        // Reset last temperature reading in order to take new configuration into account the next time the fan speed is updated.
        fan_control(TRUE);


        break;
    }


    // Master has requested status
    case FAN_MSG_ID_STATUS_REQ:
        T_N("msg_id = FAN_MSG_ID_STATUS_REQ");
        fan_msg_tx_switch_status(isid); // Transmit status back to master.
        break;

    case FAN_MSG_ID_STATUS_REP:
        // Got status from a slave switch
        T_N("msg_id = FAN_MSG_ID_STATUS_REP");
        fan_msg_local_switch_status_t *msg;
        msg = (fan_msg_local_switch_status_t *)rx_msg;
        FAN_CRIT_ENTER();
        switch_status =  (msg->status); // Update status for switch.
        cyg_flag_setbits(&status_flag, 1 << isid); // Signal that the message has been received
        FAN_CRIT_EXIT();
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
static void fan_msg_tx_switch_conf (vtss_isid_t slave_id)
{
    // Send the new configuration to the switch in question
    fan_msg_buf_t      buf;
    fan_msg_local_switch_conf_t  *msg;
    fan_msg_alloc(&buf, 1);
    msg = (fan_msg_local_switch_conf_t *)buf.msg;

    FAN_CRIT_ENTER();
    msg->local_conf.glbl_conf = fan_stack_conf.glbl_conf;
    FAN_CRIT_EXIT();
    T_DG(TRACE_GRP_CONF, "Transmit FAN_MSG_ID_CONF_SET_REQ");
    // Do the transmission
    msg->msg_id = FAN_MSG_ID_CONF_SET_REQ; // Set msg ID
    fan_msg_tx(&buf, slave_id, sizeof(*msg)); //  Send the msg
}

// Transmits a status request to a slave, and wait for the reply.
//
// In : slave_id - The slave switch id.
//
// Return : True if NO reply from slave switch.
static BOOL fan_msg_tx_switch_status_req (vtss_isid_t slave_id)
{
    BOOL             timeout;
    cyg_flag_value_t flag;
    cyg_tick_count_t time_tick;

    FAN_CRIT_ENTER();
    timeout = VTSS_MTIMER_TIMEOUT(&timer);
    FAN_CRIT_EXIT();

    if (timeout) {
        // Setup sync flag.
        flag = (1 << slave_id);
        cyg_flag_maskbits(&status_flag, ~flag);



        // Send the status request to the switch in question
        fan_msg_buf_t      buf;
        fan_msg_alloc(&buf, 1);
        fan_msg_id_req_t *msg = (fan_msg_id_req_t *)buf.msg;
        // Do the transmission
        msg->msg_id = FAN_MSG_ID_STATUS_REQ; // Set msg ID

        T_D("slave_id = %d", slave_id);
        if (msg_switch_exists(slave_id)) {
            T_D("Transmitting FAN_MSG_ID_STATUS_REQ");
            fan_msg_tx(&buf, slave_id, sizeof(*msg)); //  Send the Mag
        } else {
            T_W("Skipped fan_msg_tx due to isid:%d msg switch doesn't exist", slave_id);
            return TRUE; // Signal status get failure.
        }


        // Wait for timeout or synch. flag to be set. Timeout set to 5 sec
        time_tick = cyg_current_time() + VTSS_OS_MSEC2TICK(5000);
        return (cyg_flag_timed_wait(&status_flag, flag, CYG_FLAG_WAITMODE_OR, time_tick) & flag ? 0 : 1);
    }

    T_DG(TRACE_GRP_CONF, "timeout not set");
    return TRUE; // Signal status get failure
}




// Initializes the message protocol
static void fan_msg_init(void)
{
    /* Register for stack messages */
    msg_rx_filter_t filter;

    /* Initialize message buffers */
    VTSS_OS_SEM_CREATE(&msg_conf.request.sem, 1);
    VTSS_OS_SEM_CREATE(&msg_conf.request.sem, 1);

    memset(&filter, 0, sizeof(filter));
    filter.cb = fan_msg_rx;
    filter.modid = VTSS_TRACE_MODULE_ID;
    (void) msg_rx_filter_register(&filter);
}



//************************************************
// Configuration
//************************************************
// Function for updating all switches in a stack with the configuration
static void update_all_switches(void)
{
    // loop through all isids and send new configuration to slave switch if it exist.
    vtss_isid_t isid;
    for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
        if (msg_switch_exists(isid)) {
            fan_msg_tx_switch_conf(isid);
        }
    }

}

// Function reading configuration from flash. Restoring
// to dafault values is create = TRUE or flash access fails.
//
// In : Create : Restore configuration to default setting if set to TRUE
static void  fan_conf_read(BOOL create)
{
    fan_flash_conf_t *fan_flash_conf; // Configuration in flash
    ulong             size;
    BOOL              do_create;      // Set if we need to create new configuration in flash

    T_RG(TRACE_GRP_CONF, "Entering fan_conf_read");

    if (misc_conf_read_use()) {
        // Get configration from flash (if possible)
        if ((fan_flash_conf = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_FAN_CONF, &size)) == NULL ||
            size != sizeof(fan_flash_conf_t)) {
            fan_flash_conf = conf_sec_create(CONF_SEC_GLOBAL, CONF_BLK_FAN_CONF, sizeof(fan_flash_conf_t));
            T_W("conf_sec_open failed or size mismatch, creating defaults");
            do_create = TRUE;
        } else if (fan_flash_conf->version != FAN_CONF_VERSION) {
            T_W("Version mismatch, creating defaults. Got version:%u, expected version:%d",
                fan_flash_conf->version, FAN_CONF_VERSION);
            do_create = TRUE;
        } else {
            do_create = create;
        }
    } else {
        fan_flash_conf = NULL;
        do_create      = TRUE;
    }

    FAN_CRIT_ENTER();

    // Create configuration in flash
    if (do_create) {
        //Set default configuration
        T_DG(TRACE_GRP_CONF, "Restore to default value - block size =  %u ", sizeof(fan_flash_conf_t));
        fan_stack_conf.glbl_conf.t_max = FAN_CONF_T_MAX_DEFAULT;
        fan_stack_conf.glbl_conf.t_on  = FAN_CONF_T_ON_DEFAULT;
        if (fan_flash_conf != NULL) {
            memset(fan_flash_conf, 0, sizeof(*fan_flash_conf)); // Set everything to 0. Non-zero default values will be set below.
            fan_flash_conf->stack_conf = fan_stack_conf;
        }
    } else {
        if (fan_flash_conf != NULL) {
            fan_stack_conf = fan_flash_conf->stack_conf;
        }
    }

    FAN_CRIT_EXIT();

    update_all_switches(); // Send configuration to all switches in the stack

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    // Write our settings back to flash
    if (fan_flash_conf) {
        fan_flash_conf->version = FAN_CONF_VERSION;
        T_NG(TRACE_GRP_CONF, "Closing CONF_BLK_FAN_CONF");
        conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_FAN_CONF);
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
}


// Store the current configuration in flash
static vtss_rc store_conf (void)
{
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    T_DG(TRACE_GRP_CONF, "Storing FAN configuration in flash");
    vtss_rc rc = FAN_ERROR_FLASH;
    fan_flash_conf_t *fan_flash_conf;
    ulong size;

    if ((fan_flash_conf = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_FAN_CONF, &size)) != NULL) {
        if (size == sizeof(*fan_flash_conf)) {
            FAN_CRIT_ENTER();
            fan_flash_conf->stack_conf = fan_stack_conf;
            FAN_CRIT_EXIT();
            rc = VTSS_OK;
        } else {
            T_W("Could not store FAN configuration - Size did not match");
        }
        conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_FAN_CONF);
    } else {
        T_W("Could not store FAN configuration");
    }

    return rc;
#else
    return VTSS_RC_OK;
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
}

/****************************************************************************
* Module thread
****************************************************************************/
static void fan_thread(cyg_addrword_t data)
{
    vtss_fan_conf_t fan_spec;
    T_R("Entering fan_thread");

    // This will block this thread from running further until the PHYs are initialized.
    port_phy_wait_until_ready();
    FAN_CRIT_ENTER();
    VTSS_MTIMER_START(&timer, 1);
    FAN_CRIT_EXIT();

    fan_get_fan_spec(&fan_spec);
    // Initialize temperature sensor.
    if (fan_init_temperature_sensor(VTSS_ISID_LOCAL) != VTSS_OK) {  // initialize the temperature sensor
        T_E("Could not initialize temperature sensor, fan thread terminated");
        return;
    }


    // Initialize icfg
#ifdef VTSS_SW_OPTION_ICFG
    FAN_CRIT_ENTER();
    if (fan_icfg_init() != VTSS_RC_OK) {
        T_E("ICFG not initialized correctly");
    }
    FAN_CRIT_EXIT();
#endif



    // To make lint happy - In this case it is OK to have a constant
    // if (true) in the case where no fan initialization is needed
    /*lint --e{506} */
    if (fan_initialize(VTSS_ISID_LOCAL, fan_spec) != VTSS_OK) {
        T_E("Could not initialize fan controller, fan thread terminated");
        return;
    }

    fan_init_done = TRUE;

    // ***** Go into loop **** //
    T_R("Entering fan_thread Loop");
    for (;;) {
        VTSS_OS_MSLEEP(1000);
        fan_control(FALSE); // Control fan speed
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
void fan_mgmt_get_switch_conf(fan_local_conf_t *switch_conf)
{

    // All switches have the same configuration
    FAN_CRIT_ENTER();
    memcpy(switch_conf, &fan_local_conf, sizeof(fan_local_conf_t));
    FAN_CRIT_EXIT();
}

// Function for setting the current configuration for a switch.
//
// In : isid - isid for the switch the shall return its configuration
//      switch_conf - Pointer to configuration struct with the new configuration.
//
// Return : VTSS error code
vtss_rc fan_mgmt_set_switch_conf(fan_local_conf_t  *new_switch_conf)
{
    // Configuration changes only allowed by master
    if (!msg_switch_is_master()) {
        T_W("Configuration change only allowed from master switch");
        return FAN_ERROR_NOT_MASTER;
    }

    // It doesn't make any sense to have a t_max that is lower than T_on.
    if (new_switch_conf->glbl_conf.t_max <= new_switch_conf->glbl_conf.t_on) {
        return FAN_ERROR_T_CONF;
    }


    // Ok now we can do the configuration
    FAN_CRIT_ENTER();
    memcpy(&fan_stack_conf.glbl_conf, &new_switch_conf->glbl_conf, sizeof(fan_glbl_conf_t)); // Update the configuration for the switch
    FAN_CRIT_EXIT();

    T_DG(TRACE_GRP_CONF, "Conf. changed");

    // Transfer new configuration to the switch in question.
    update_all_switches();

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
vtss_rc fan_mgmt_get_switch_status(fan_local_status_t *status, vtss_isid_t isid)
{
    vtss_fan_conf_t fan_spec;

    // Configuration changes only allowed by master
    vtss_rc rc;
    if ((rc = fan_is_master_and_isid_legal(isid)) != VTSS_OK) {
        return rc;
    }

    if (fan_msg_tx_switch_status_req(isid)) {
        T_D("Communication problem with slave switch");
        memset(status, 0, sizeof(fan_local_status_t)); // We have no real data, so we resets everything to 0.
        return FAN_ERROR_SLAVE;
    } else {
        FAN_CRIT_ENTER();
        memcpy(status, &switch_status, sizeof(fan_local_status_t));
        FAN_CRIT_EXIT();

        // Give a warning if the FAN is not running (when it is supposed to run)
        fan_get_fan_spec(&fan_spec);
        // 2 wire fan doesn't give any rotation information.
        if (fan_spec.type != VTSS_FAN_2_WIRE_TYPE) {
            if (switch_status.fan_speed == 0 && switch_status.fan_speed_setting_pct > 0) {
                return FAN_ERROR_FAN_NOT_RUNNING;
            }
        }


        return VTSS_RC_OK;
    }
}



/****************************************************************************/
/*  Initialization functions                                                */
/****************************************************************************/

vtss_rc fan_init(vtss_init_data_t *data)
{

    vtss_isid_t isid = data->isid; // Get switch id


    if (data->cmd == INIT_CMD_INIT) {
        // Initialize and register trace ressources
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);
    }

    T_D("fan_init enter, cmd=%d", data->cmd);

    switch (data->cmd) {
    case INIT_CMD_INIT:
        cyg_flag_init(&status_flag);
        critd_init(&crit, "fan_crit", VTSS_MODULE_ID_FAN, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        FAN_CRIT_EXIT();

        /* Initialize and register trace resource's */
        memset(&crit_region.if_mutex, 0, sizeof(crit_region.if_mutex));
        crit_region.if_mutex.type = VTSS_ECOS_MUTEX_TYPE_NORMAL;
        vtss_ecos_mutex_init(&crit_region.if_mutex);
        cyg_thread_create(THREAD_DEFAULT_PRIO,
                          fan_thread,
                          0,
                          "FAN",
                          fan_thread_stack,
                          sizeof(fan_thread_stack),
                          &fan_thread_handle,
                          &fan_thread_block);
        T_D("enter, cmd=INIT");

        break;
    case INIT_CMD_START:
        fan_msg_init();
        break;
    case INIT_CMD_CONF_DEF:
        if (isid == VTSS_ISID_LOCAL) {
            /* Reset local configuration */
            T_D("isid local");
        } else if (VTSS_ISID_LEGAL(isid)) {
            /* Reset configuration (specific switch or all switches) */
            T_D("Restore to default");
            fan_conf_read(TRUE);
        }
        T_D("enter, cmd=INIT_CMD_CONF_DEF");
        break;


    case INIT_CMD_MASTER_UP:
        fan_conf_read(FALSE);
        break;

    case INIT_CMD_SWITCH_ADD:
        T_D("SWITCH_ADD, isid: %d", isid);
        fan_msg_tx_switch_conf(isid); // Update configuration for the switch added
        cyg_thread_resume(fan_thread_handle); // Start thread
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
