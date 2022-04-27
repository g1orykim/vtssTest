/*

Vitesse Switch Software.

 Copyright (c) 2002-2012 Vitesse Semiconductor Corporation "Vitesse". All
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
/************************************************************-*- mode: C -*-*/
/*                                                                          */
/*           Copyright (C) 2007 Vitesse Semiconductor Corporation           */
/*                           All Rights Reserved.                           */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*                            Copyright Notice:                             */
/*                                                                          */
/*  This document contains confidential and proprietary information.        */
/*  Reproduction or usage of this document, in part or whole, by any means, */
/*  electrical, mechanical, optical, chemical or otherwise is prohibited,   */
/*  without written permission from Vitesse Semiconductor Corporation.      */
/*                                                                          */
/*  The information contained herein is protected by Danish and             */
/*  international copyright laws.                                           */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/

#include "critd_api.h"
#include "poe.h"
#include "poe_custom_api.h"
#include "misc_api.h"
#include "poe_custom_slus787_api.h"


#define OFF       0x0
#define MANUAL    0x1
#define SEMI_AUTO 0x2
#define AUTO      0x3


//
// Define the SLUS787 register map.
//
const char SLUS787_INTERRUPT_REG      = 0x0;
const char SLUS787_INTERRUPT_MASK     = 0x01;
const char SLUS787_POWER_ENVENTS      = 0x02;
const char SLUS787_POWER_ENVENTS_COR  = 0x03;
const char SLUS787_DETECT_ENVENTS     = 0x04;
const char SLUS787_DETECT_ENVENTS_COR = 0x05;
const char SLUS787_FAULT_ENVENTS      = 0x06;
const char SLUS787_FAULT_ENVENTS_COR  = 0x07;
const char SLUS787_START_ENVENTS      = 0x08;
const char SLUS787_START_ENVENTS_COR  = 0x09;
const char SLUS787_SUPPLY_ENVENTS     = 0x0A;
const char SLUS787_SUPPLY_ENVENTS_COR = 0x0B;

const char SLUS787_PORT_1_STATUS        = 0x0C;
const char SLUS787_PORT_2_STATUS        = 0x0D;
const char SLUS787_PORT_3_STATUS        = 0x0E;
const char SLUS787_PORT_4_STATUS        = 0x0F;
const char SLUS787_POWER_STATUS         = 0x10;
const char SLUS787_PIn_STATUS           = 0x11;
const char SLUS787_OPERATION_MODE       = 0x12;
const char SLUS787_DISCONNECT_MODE      = 0x13;
const char SLUS787_DETECT_CLASS_MODE    = 0x14;
const char SLUS787_TIMING_CONFIG        = 0x16;
const char SLUS787_GENERAL_MASK         = 0x17;
const char SLUS787_DETECT_CLASS_RESTART = 0x18;
const char SLUS787_POWER_ENABLE         = 0x19;
const char SLUS787_RESET_REG            = 0x1A;

const char SLUS787_ID_REG               = 0x1B;

const char SLUS787_ICUT21_REG           = 0x2A;
const char SLUS787_ICUT43_REG           = 0x2B;

const char SLUS787_PORT_1_CURRENT  = 0x30; /* 2 bytes value */
const char SLUS787_PORT_2_CURRENT  = 0x34;
const char SLUS787_PORT_3_CURRENT  = 0x38;
const char SLUS787_PORT_4_CURRENT  = 0x3C;
const char SLUS787_PORT_1_VOLTAGE  = 0x32;
const char SLUS787_PORT_2_VOLTAGE  = 0x36;
const char SLUS787_PORT_3_VOLTAGE  = 0x3A;
const char SLUS787_PORT_4_VOLTAGE  = 0x3E;


const char SLUS787_HIGH_POWER_AND_SINE_DISABLE = 0x40;
const char SLUS787_FIRMWARE_REVSION            = 0x41;
const char SLUS787_I2C_WATCH_DOG               = 0x42;
const char SLUS787_DEVICE_ID                   = 0x43;

static poe_chip_found_t slus787_chip_found[VTSS_PORTS];
static char i2c_wait = 5;
// Do the i2c write.
void slus787_device_wr(vtss_port_no_t port_index, uchar reg_addr, uchar data)
{
    u8 retry_cnt;
    T_NG_PORT(VTSS_TRACE_GRP_CUSTOM, port_index, "reg_addr = %d, data = 0x%X", reg_addr, data);

    // Get PoE hardware board configuration
    poe_custom_entry_t poe_hw_conf;
    poe_hw_conf = poe_custom_get_hw_config(port_index, &poe_hw_conf);

    if (poe_hw_conf.available) {
        for (retry_cnt = 1; retry_cnt <= I2C_RETRIES_MAX; retry_cnt++) {
            uchar buf[2];
            buf[0] = reg_addr;
            buf[1] = data;

            // If no bytes were transmitted we need to do redetection of the PoE chipset
            if (vtss_i2c_wr(NULL, poe_hw_conf.i2c_addr[SLUS787], &buf[0], 2, i2c_wait, NO_I2C_MULTIPLEXER) != VTSS_RC_OK) {
                if (retry_cnt == I2C_RETRIES_MAX) {
                    T_WG(VTSS_TRACE_GRP_CUSTOM, "I2C TX problem, Re-detecting PoE Chip set");
                    slus787_chip_found[port_index] = DETECTION_NEEDED;
                } else {
                    T_IG(VTSS_TRACE_GRP_CUSTOM, "TX retry cnt = %d", retry_cnt);
                }
            } else {
                break; // OK - Access went well. No need to re-send.
            }
        }
    }
}

// Do the i2c read, the result is passed by pointer.
void slus787_device_rd(vtss_port_no_t port_index, uchar reg_addr, uchar *data, size_t size)
{
    u8 retry_cnt;

    // Get hardware board PoE configuration
    poe_custom_entry_t poe_hw_conf;
    poe_hw_conf = poe_custom_get_hw_config(port_index, &poe_hw_conf);

    T_NG_PORT(VTSS_TRACE_GRP_CUSTOM, port_index, "reg_addr = %d, I2C_addr = 0x%X, size = %d ",
              reg_addr, poe_hw_conf.i2c_addr[SLUS787], size);

    if (poe_hw_conf.available) {
        for (retry_cnt = 1; retry_cnt <= I2C_RETRIES_MAX; retry_cnt++) {
            // If no bytes were transmitted we need to do redetection of the PoE chipset
            if (vtss_i2c_wr_rd(NULL, poe_hw_conf.i2c_addr[SLUS787], &reg_addr, 1, data, size, i2c_wait, NO_I2C_MULTIPLEXER) == VTSS_RC_OK) {
                break; // OK - Data was read correctly
            } else {
                if (retry_cnt == I2C_RETRIES_MAX) {
                    slus787_chip_found[port_index] = DETECTION_NEEDED;
                    T_WG(VTSS_TRACE_GRP_CUSTOM, "I2C RX problem, Re-detecting PoE Chip set");
                    break;
                } else {
                    T_IG_PORT(VTSS_TRACE_GRP_CUSTOM, port_index, "RX retry cnt = %d", retry_cnt);
                    continue; // Access failed - retry.
                }
            }
        }

    }
}

static void slus787_device_wr_masked(vtss_port_no_t port_index, char reg_addr, char data, uchar mask)
{
    char value;
    uchar reg_value;
    value = data & mask;
    slus787_device_rd(port_index, reg_addr, &reg_value, sizeof(reg_value));
    value |= (reg_value & ~mask);
    slus787_device_wr(port_index, reg_addr, value);
}


// Returns the Vee voltage in mV
static int slus787_get_vee(vtss_port_no_t port_index)
{
    T_RG_PORT(VTSS_TRACE_GRP_CUSTOM, port_index, "Entering slus787_get_vee");
    short _vee = 0;
    int vee;
    char port_id   = port_index % PORTS_PER_POE_CHIP; // 4 ports per chip - See Table 1 - SLUS787 register map.

    if (port_id == 0) {
        slus787_device_rd(port_index, SLUS787_PORT_1_VOLTAGE, (uchar *)&_vee, sizeof(_vee));
    } else if (port_id == 1) {
        slus787_device_rd(port_index, SLUS787_PORT_2_VOLTAGE, (uchar *)&_vee, sizeof(_vee));
    } else if (port_id == 2) {
        slus787_device_rd(port_index, SLUS787_PORT_3_VOLTAGE, (uchar *)&_vee, sizeof(_vee));
    } else if (port_id == 3) {
        slus787_device_rd(port_index, SLUS787_PORT_4_VOLTAGE, (uchar *)&_vee, sizeof(_vee));
    } else {
        T_E("Unknown port_id");
    }

    vee = (_vee * 5920) / 1000;

    T_NG_PORT(VTSS_TRACE_GRP_CUSTOM, port_index, "port_index = %d, _vee=%d, vee = %u mV", port_index, _vee, vee);
    return vee;
}


// read the port status
static char slus787_read_port_status (vtss_port_no_t port_index)
{
    T_NG_PORT(VTSS_TRACE_GRP_CUSTOM, port_index, "Entering slus787_read_port_status");
    uchar reg_value = 0;
    char port_id   = port_index % PORTS_PER_POE_CHIP; // 4 ports per chip - See Table 1 - SLUS787 register map.

    // restart class detection.
    slus787_device_wr(port_index, SLUS787_DETECT_CLASS_RESTART, 0xFF);

    if (port_id == 0) {
        slus787_device_rd(port_index, SLUS787_PORT_1_STATUS, &reg_value, sizeof(reg_value));
    } else if (port_id == 1) {
        slus787_device_rd(port_index, SLUS787_PORT_2_STATUS, &reg_value, sizeof(reg_value));
    } else if (port_id == 2) {
        slus787_device_rd(port_index, SLUS787_PORT_3_STATUS, &reg_value, sizeof(reg_value));
    } else if (port_id == 3) {
        slus787_device_rd(port_index, SLUS787_PORT_4_STATUS, &reg_value, sizeof(reg_value));
    } else {
        T_E("Unknown port_id");
    }
    return reg_value;
}

char slus787_read_port_power_status (vtss_port_no_t port_index)
{
    uchar reg_value;
    slus787_device_rd(port_index, SLUS787_POWER_STATUS, &reg_value, sizeof(reg_value));
    T_NG_PORT(VTSS_TRACE_GRP_CUSTOM, port_index, "reg_value:%d", reg_value);
    return reg_value;
}

// Returns 1 if PoE is Ok for a port
static char slus787_poe_port_ok(vtss_port_no_t port_index)
{
    char pwr_good_status = 0;
    char pwr_enable_status = 0;
    char port_id   = port_index % PORTS_PER_POE_CHIP;
    char power_status =  slus787_read_port_power_status(port_index);

    if (port_id == 0) {
        pwr_good_status   = power_status >> 4 & 0x1;
        pwr_enable_status = power_status >> 0 & 0x1;
    } else if (port_id == 1) {
        pwr_good_status   = power_status >> 5 & 0x1;
        pwr_enable_status = power_status >> 1 & 0x1;
    } else if (port_id == 2) {
        pwr_good_status   = power_status >> 6 & 0x1;
        pwr_enable_status = power_status >> 2 & 0x1;
    } else if (port_id == 3) {
        pwr_good_status   = power_status >> 7 & 0x1;
        pwr_enable_status = power_status >> 3 & 0x1;
    } else {
        T_E("Unknown port_id");
    }

    T_DG_PORT(VTSS_TRACE_GRP_CUSTOM, port_index, "STATUS = 0x%X", power_status);
    T_NG_PORT(VTSS_TRACE_GRP_CUSTOM, port_index, "pwr_good_status = %d, pwr_enable_status = %d", pwr_good_status, pwr_enable_status);
    return pwr_enable_status;
}


// Get status for all ports
void slus787_port_status_get(poe_port_status_t *port_status, vtss_port_no_t port_index)
{
    char pwr_enable_status = 0;
    char detection;
    char port_id;
    char port_status_reg;
    poe_custom_entry_t poe_hw_conf;

    // Get hardware board PoE configuration
    poe_hw_conf = poe_custom_get_hw_config(port_index, &poe_hw_conf);

    if (poe_hw_conf.available == false) {
        // PoE chip not mounted for this port
        port_status[port_index] = POE_NOT_SUPPORTED;
        return;
    }

    port_status_reg = slus787_read_port_power_status(port_index);
    detection       = slus787_read_port_status(port_index) & 0x07;
    port_id         = port_index % PORTS_PER_POE_CHIP;

    if (port_id == 0) {
        pwr_enable_status = port_status_reg >> 0 & 0x1;
    } else if (port_id == 1) {
        pwr_enable_status = port_status_reg >> 1 & 0x1;
    } else if (port_id == 2) {
        pwr_enable_status = port_status_reg >> 2 & 0x1;
    } else if (port_id == 3) {
        pwr_enable_status = port_status_reg >> 3 & 0x1;
    } else {
        T_E("Unknown port_id");
    }

    T_DG_PORT(VTSS_TRACE_GRP_CUSTOM, port_index, "status:0x%X, detection:0x%X, pwr_enable_status:%d",
              port_status_reg, detection, pwr_enable_status);

    if (detection == 4) { // 4 = Detection Good
        if (pwr_enable_status) {
            T_NG(VTSS_TRACE_GRP_CUSTOM, "Setting port status to PD_ON Port_index = %u", port_index);
            port_status[port_index] = PD_ON;
        } else {
            T_NG(VTSS_TRACE_GRP_CUSTOM, "Setting port status to PD_OFF Port_index = %u", port_index);
            port_status[port_index] = PD_OFF;
        }
    } else {
        T_DG_PORT(VTSS_TRACE_GRP_CUSTOM, port_index, "Setting port status to NO_PD_DETECTED");
        // See Table 2 register defintions.
        port_status[port_index] = NO_PD_DETECTED;

    }
}


// Function for setting the max power for a port.
void slus787_set_power_limit_channel(vtss_port_no_t port_index, int max_port_power)
{
    ushort icut = max_port_power * 100 / 48;
    char port_id  = port_index % PORTS_PER_POE_CHIP; // 4 ports per chip - See Table 1 - SLUS787 register map.
    char reg_icut = 0;

    if (754 < icut && icut <= 816) {
        reg_icut = 0x07;
    } else if (686 < icut && icut <= 754) {
        reg_icut = 0x04;
    } else if (592 < icut && icut <= 686) {
        reg_icut = 0x06;
    } else if (374 < icut && icut <= 592) {
        reg_icut = 0x05;
    } else if (204 < icut && icut <= 374) {
        reg_icut = 0x03;
    } else if (110 < icut && icut <= 204) {
        reg_icut = 0x02;
    } else if (icut <= 110) {
        reg_icut = 0x01;
    }

    if (reg_icut >= 0x04) {
        slus787_device_wr_masked(port_index, SLUS787_HIGH_POWER_AND_SINE_DISABLE, 1 << (port_id + 4), 1 << (port_id + 4));
    } else {
        slus787_device_wr_masked(port_index, SLUS787_HIGH_POWER_AND_SINE_DISABLE, 0 << (port_id + 4), 1 << (port_id + 4));
    }

    if (port_id == 0) {
        slus787_device_wr_masked(port_index, SLUS787_ICUT21_REG, reg_icut, 0x0f);
    } else if (port_id == 1) {
        slus787_device_wr_masked(port_index, SLUS787_ICUT21_REG, (reg_icut << 4), 0xf0);
    } else if (port_id == 2) {
        slus787_device_wr_masked(port_index, SLUS787_ICUT43_REG, reg_icut, 0x0f);
    } else if (port_id == 3) {
        slus787_device_wr_masked(port_index, SLUS787_ICUT43_REG, (reg_icut << 4), 0xf0);
    } else {
        T_E("Unknown port_id");
    }

    T_NG_PORT(VTSS_TRACE_GRP_CUSTOM, port_index, "max_port_power = %d,icut %d, reg_icut set to 0x%x", max_port_power, icut, reg_icut);

}



// Determine the class for each port. Passed back via pointer
void slus787_get_all_port_class(char *classes, vtss_port_no_t port_index)
{
    int class_reg = slus787_read_port_status(port_index);
    // See slus787 documentation Table 1.
    classes[port_index] = (class_reg >> 4) & 0x7;


    T_NG_PORT(VTSS_TRACE_GRP_CUSTOM, port_index, "classes[port_index] = %d, class_reg = 0x%X", classes[port_index], class_reg);


    // We do not use the class encoding ( See Table 2 in documentation). Convert Class 0 ( encoded as 6 )  to 0
    if (classes[port_index] > 4) {
        classes[port_index] = 0;
    }
}


// Updates the poe status with current and power consumption
void slus787_get_port_measurement(poe_status_t *poe_status, vtss_port_no_t port_index)
{

    T_NG_PORT(VTSS_TRACE_GRP_CUSTOM, port_index, "Entering slus787_get_port_measurement");
    short _current = 0;

    int power_used;
    int current_used = 0;
    int vee;

    T_NG_PORT(VTSS_TRACE_GRP_CUSTOM, port_index, "slus787_poe_port_ok = %d", slus787_poe_port_ok(port_index));
    if (slus787_poe_port_ok(port_index)) {
        // Select which port to get current infomation from.
        char port_id   = port_index % PORTS_PER_POE_CHIP; // 4 ports per chip - See Table 1 - SLUS787 register map.
        if (port_id == 0) {
            slus787_device_rd(port_index, SLUS787_PORT_1_CURRENT, (uchar *)&_current, sizeof(_current));
        } else if (port_id == 1) {
            slus787_device_rd(port_index, SLUS787_PORT_2_CURRENT, (uchar *)&_current, sizeof(_current));
        } else if (port_id == 2) {
            slus787_device_rd(port_index, SLUS787_PORT_3_CURRENT, (uchar *)&_current, sizeof(_current));
        } else if (port_id == 3) {
            slus787_device_rd(port_index, SLUS787_PORT_4_CURRENT, (uchar *)&_current, sizeof(_current));
        } else {
            T_E("Unknown port_id");
        }

        // Convert to mA. Port current is in 100us units - see table 3:"device command summary"
        current_used = (_current * 61035) / 1000 / 1000;
        // Get the vee value ( in milivolt )
        vee = slus787_get_vee(port_index);
        T_DG_PORT(VTSS_TRACE_GRP_CUSTOM, port_index, "_current = %u, current_used = %u",
                  _current, current_used);
        //      current_used = 0; // TBR FJ
        power_used = vee * current_used / 100000; // Calculate power and convert to DeciWatts
    } else {
        vee = 0;
        current_used = 0;
        power_used = 0;
    }


    // Update the status structure
    poe_status -> power_used[port_index]   = power_used;
    poe_status -> current_used[port_index] = current_used;

    if (current_used > 1000 ) {
        T_DG_PORT(VTSS_TRACE_GRP_CUSTOM, port_index, "power_used = %u, current_used = %u, vee = %d",
                  power_used, poe_status -> current_used[port_index], vee);
    }
}

// Set port mode ( Manual / Semiauto or auto )
static void slus787_set_port_mode (vtss_port_no_t port_index, char mode)
{
    int port_id = port_index % PORTS_PER_POE_CHIP; // 4 ports per chip - See Table 1 - SLUS787 register map.
    char port_mode_mask = 0x03 << (port_id * 2);
    char port_mode = (mode << (port_id * 2));

    slus787_device_wr_masked(port_index, SLUS787_OPERATION_MODE, port_mode, port_mode_mask);

    if (mode == SEMI_AUTO) {
        T_DG_PORT(VTSS_TRACE_GRP_CUSTOM, port_index, "Mode = SEMI_AUTO");
        char power_off = 0x10 << port_id;
        slus787_device_wr(port_index, SLUS787_POWER_ENABLE, power_off);
    }

    // Enable class detection for all ports.
    slus787_device_wr(port_index, SLUS787_DETECT_CLASS_MODE, 0xFF);
    slus787_device_wr(port_index, SLUS787_DETECT_CLASS_RESTART, 0xFF);

    VTSS_OS_MSLEEP(10); // Wait a little bit, because we have seen that the PD is signaled overloaded if we do
    // accesses to the PoE chipset after we have enabled the PoE chipset. See bugzilla#3732
}

// Things that is needed after a reset
void slus787_poe_init(vtss_port_no_t port_index)
{
    // Know that the following writes only is need once per PoE chip and not per port, but we will live with the
    // overhead because the init function is only called once.
    slus787_device_wr(port_index, SLUS787_RESET_REG, 0xdf); // Reset PoE Chip for starting in a known state self-cleared
    slus787_device_wr(port_index, SLUS787_DISCONNECT_MODE, 0xf0); // Use AC Disconnect to avoid Cisco
    slus787_device_wr(port_index, SLUS787_DETECT_CLASS_MODE, 0xff); // Enable Classfication and Detection
    slus787_device_wr_masked(port_index, SLUS787_GENERAL_MASK, 0, 0x80); // Disable interrupt. We are not using that.

    T_NG(VTSS_TRACE_GRP_CUSTOM, "Entering slus787_poe_init");
    slus787_set_port_mode(port_index, SEMI_AUTO); // Set to OFF mode
}


// Enables or disable poe for a port
void slus787_poe_enable(vtss_port_no_t port_index, BOOL enable)
{

    if (enable) {
        slus787_set_port_mode(port_index, AUTO); // Set mode to auto
        T_DG_PORT(VTSS_TRACE_GRP_CUSTOM, port_index, "Enable");
    } else {
        slus787_set_port_mode(port_index, SEMI_AUTO); // Set mode to OFF
        T_DG_PORT(VTSS_TRACE_GRP_CUSTOM, port_index, "Disable");
    }
}

void slus787_poe_force_auto_detect(vtss_port_no_t port_index)
{
    int port_id = port_index % PORTS_PER_POE_CHIP; // 4 ports per chip - See Table 1 - SLUS787 register map.
    char port_mode_mask = 0x03 << (port_id * 2);
    char port_mode = (AUTO << (port_id * 2));
    char detect_mode = 0x11 << port_id;

    slus787_device_wr_masked(port_index, SLUS787_OPERATION_MODE, port_mode, port_mode_mask);
    VTSS_OS_MSLEEP(10);
    slus787_device_wr(port_index, SLUS787_DETECT_CLASS_RESTART, detect_mode);
}

void slus787_poe_force_off_detect(vtss_port_no_t port_index)
{
    int port_id = port_index % PORTS_PER_POE_CHIP; // 4 ports per chip - See Table 1 - SLUS787 register map.
    char port_mode_mask = 0x03 << (port_id * 2);
    char port_mode = (OFF << (port_id * 2));
    char detect_mode_mask = 0x11 << port_id;
    char detect_mode = 0x0 << port_id;

    slus787_device_wr_masked(port_index, SLUS787_OPERATION_MODE, port_mode, port_mode_mask);
    slus787_device_wr_masked(port_index, SLUS787_DETECT_CLASS_MODE, detect_mode, detect_mode_mask);
}

// Function that returns 1 is a PoE chip set is found, else 0.
int slus787_is_chip_available(vtss_port_no_t port_index)
{
    int i;
    uchar hw_rev  = 0;
    static BOOL init_done = FALSE;

    // Initialize chip found array.
    if (!init_done) {
        for (i = 0 ; i < VTSS_PORTS; i++) {
            slus787_chip_found[i] = DETECTION_NEEDED;
        }
        init_done = TRUE;
    }

    // We only want to try to detect the slus787 chipset once.
    if (slus787_chip_found[port_index] == DETECTION_NEEDED) {
        T_I("i2c_wait:%d port_index:%u", i2c_wait, port_index);

        i2c_wait = 5; // Set timeout low during detection in order to fast detection.
        // We detects the chipset by reading the hardware revison register.
        slus787_device_rd(port_index, SLUS787_DEVICE_ID, &hw_rev, sizeof(hw_rev));


        if (hw_rev == 0xA0) {
            slus787_chip_found[port_index] = FOUND;
            T_IG(VTSS_TRACE_GRP_CUSTOM, "PoE slus787 chipset found");
        } else {
            slus787_chip_found[port_index] = NOT_FOUND;
            T_IG(VTSS_TRACE_GRP_CUSTOM, "PoE slus787 chipset NOT found - port_index:%u", port_index);
        }
        i2c_wait = 100; // Set timeout "high" for normal accesses
        T_IG(VTSS_TRACE_GRP_CUSTOM, "hw_rev = 0x%X, port_index:%u", hw_rev, port_index);
    }


    if (slus787_chip_found[port_index] == FOUND) {
        return 1;
    } else {
        return 0;
    }
}


/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
