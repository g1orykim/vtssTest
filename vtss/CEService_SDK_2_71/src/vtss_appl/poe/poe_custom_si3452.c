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
/****************************************************************************/
// PoE chip SI3452 from SilliconLabs. We having got a real data sheet at the time of writing,
// so all refernces are done to the word docment we have got from SilliconLabs.
// The docuemnt can be found at L:\R_D\Components\Silicon Laboratories\si3452 e
/****************************************************************************/

#include "critd_api.h"
#include "poe.h"
#include "poe_custom_api.h"
#include "misc_api.h"
#include "poe_custom_si3452_api.h"

//
// Define the SI3452 register map.
//
const uchar SI3452_INTERRUPT_REG_1  = 0;
const uchar SI3452_PORT_1_ENVETS    = 0x02;
const uchar SI3452_PORT_2_ENVETS    = 0x03;
const uchar SI3452_PORT_3_ENVETS    = 0x04;
const uchar SI3452_PORT_4_ENVETS    = 0x05;
const uchar SI3452_PORT_1_STATUS    = 0x06;
const uchar SI3452_PORT_2_STATUS    = 0x07;
const uchar SI3452_PORT_3_STATUS    = 0x08;
const uchar SI3452_PORT_4_STATUS    = 0x09;
const uchar SI3452_PORT_1_CONFIG    = 0x0A;
const uchar SI3452_PORT_2_CONFIG    = 0x0B;

const uchar SI3452_PORT_3_CONFIG    = 0x0C;
const uchar SI3452_PORT_4_CONFIG    = 0x0D;
const uchar SI3452_PORT_1_ICUT      = 0x0E;
const uchar SI3452_PORT_2_ICUT      = 0x0F;
const uchar SI3452_PORT_3_ICUT      = 0x10;
const uchar SI3452_PORT_4_ICUT      = 0x11;
const uchar SI3452_COMMAND_REGISTER = 0x12;
const uchar SI3452_MEASURE_MSB      = 0x13;
const uchar SI3452_MEASURE_LSB      = 0x14;
const uchar SI3452_PORT_1_MEASURE_CURRENT_MSB      = 0x15;
const uchar SI3452_PORT_1_MEASURE_CURRENT_LSB      = 0x16;
const uchar SI3452_PORT_2_MEASURE_CURRENT_MSB      = 0x17;
const uchar SI3452_PORT_2_MEASURE_CURRENT_LSB      = 0x18;
const uchar SI3452_PORT_3_MEASURE_CURRENT_MSB      = 0x19;
const uchar SI3452_PORT_3_MEASURE_CURRENT_LSB      = 0x1A;
const uchar SI3452_PORT_4_MEASURE_CURRENT_MSB      = 0x1B;
const uchar SI3452_PORT_4_MEASURE_CURRENT_LSB      = 0x1C;
const uchar SI3452_HARDWARE_REVISON = 0x60;


static poe_chip_found_t si3452_chip_found[VTSS_PORTS];
static u8 i2c_wait = 100;

// Do the i2c write.
void si3452_device_wr(vtss_port_no_t port_index, uchar reg_addr, char data)
{
    u8 retry_cnt;
    // Get PoE hardware board configuration
    poe_custom_entry_t poe_hw_conf;
    poe_hw_conf = poe_custom_get_hw_config(port_index, &poe_hw_conf);

    if (poe_hw_conf.available) {
        for (retry_cnt = 1; retry_cnt <= I2C_RETRIES_MAX; retry_cnt++) {
            uchar buf[2];
            buf[0] = reg_addr;
            buf[1] = data;

            // If no bytes were transmitted we need to do redetection of the PoE chipset
            if (vtss_i2c_wr(NULL, poe_hw_conf.i2c_addr[SI3452], &buf[0], 2, i2c_wait, NO_I2C_MULTIPLEXER) != VTSS_RC_OK) {
                if (retry_cnt == I2C_RETRIES_MAX) {
                    T_WG(VTSS_TRACE_GRP_CUSTOM, "I2C TX problem, Re-detecting PoE Chip set");
                    si3452_chip_found[port_index] = DETECTION_NEEDED;
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
void si3452_device_rd(vtss_port_no_t port_index, uchar reg_addr, uchar *data)
{
    u8 retry_cnt;
    // Get PoE hardware board configuration
    poe_custom_entry_t poe_hw_conf;
    poe_hw_conf = poe_custom_get_hw_config(port_index, &poe_hw_conf);

    T_RG_PORT(VTSS_TRACE_GRP_CUSTOM, port_index, "si3452_device_rd, reg_addr = %d, I2C_addr = 0x%X ",
              reg_addr, poe_hw_conf.i2c_addr[SI3452]);

    if (poe_hw_conf.available) {
        for (retry_cnt = 1; retry_cnt <= I2C_RETRIES_MAX; retry_cnt++) {
            // If no bytes were received we need to do re-detection of the PoE chipset
            if (vtss_i2c_wr_rd(NULL, poe_hw_conf.i2c_addr[SI3452], &reg_addr, 1, data, 1, i2c_wait, NO_I2C_MULTIPLEXER) == VTSS_RC_OK) {
                break; // OK - Data was read correctly
            } else {
                if (retry_cnt == I2C_RETRIES_MAX) {
                    si3452_chip_found[port_index] = DETECTION_NEEDED;
                    T_WG(VTSS_TRACE_GRP_CUSTOM, "I2C RX problem, Re-detecting PoE Chip set");
                    break;
                } else {
                    T_IG_PORT(VTSS_TRACE_GRP_CUSTOM, port_index, "RX retry cnt = %d", retry_cnt);
                    continue; // Access failed - retry.
                }
            }
        }
    }
    T_I("Read Exit");
}


// Returns the Vee voltage in mV
static int si3452_get_vee(vtss_port_no_t port_index)
{
    T_RG_PORT(VTSS_TRACE_GRP_CUSTOM, port_index, "Entering si3452_get_vee");
    uchar vee_lsb, vee_msb;
    int vee;

    // Get Vee
    // Select which port to get vee infomation, see Si3452 documentation.
    uchar command    = 0x18 + (port_index %  PORTS_PER_POE_CHIP);

    // Read the vee.
    si3452_device_wr(port_index, SI3452_COMMAND_REGISTER, command); // Initialize the read
    si3452_device_rd(port_index, SI3452_MEASURE_LSB, &vee_lsb);
    si3452_device_rd(port_index, SI3452_MEASURE_MSB, &vee_msb);

    vee = (vee_msb << 8) + vee_lsb; // Vee in mV units - see table 3:"device command summary"

    T_NG_PORT(VTSS_TRACE_GRP_CUSTOM, port_index, "port_index = %d, vee_msb = %d, vee_lsb=%d, vee = %u", port_index, vee_msb, vee_lsb, vee);
    return vee;
}


// read the port status
char si3452_read_port_status (vtss_port_no_t port_index)
{
    T_NG_PORT(VTSS_TRACE_GRP_CUSTOM, port_index, "Entering si3452_read_port_status");
    uchar reg_value = 0;
    char port_id   = port_index % PORTS_PER_POE_CHIP; // 4 ports per chip - See Table 1 - SI3452 register map.
    if (port_id == 0) {
        si3452_device_rd(port_index, SI3452_PORT_1_STATUS, &reg_value);
    } else if (port_id == 1) {
        si3452_device_rd(port_index, SI3452_PORT_2_STATUS, &reg_value);
    } else if (port_id == 2) {
        si3452_device_rd(port_index, SI3452_PORT_3_STATUS, &reg_value);
    } else if (port_id == 3) {
        si3452_device_rd(port_index, SI3452_PORT_4_STATUS, &reg_value);
    } else {
        T_E("Unknown port_id");
    }
    return reg_value;
}

// Returns 1 if PoE is Ok for a port
static char si3452_poe_port_ok(vtss_port_no_t port_index)
{
    char pwr_good_status;
    char pwr_enable_status;

    char port_status =  si3452_read_port_status(port_index);

    // Extract status - See Table 1:Si3452 register map.
    pwr_good_status   = port_status >> 7 & 0x1;
    pwr_enable_status = port_status >> 6 & 0x1;

    T_DG_PORT(VTSS_TRACE_GRP_CUSTOM, port_index, "STATUS = 0x%X", port_status);
    T_NG_PORT(VTSS_TRACE_GRP_CUSTOM, port_index, "pwr_good_status = %d, pwr_enable_status = %d", pwr_good_status, pwr_enable_status);
    return pwr_enable_status;
}


// Get status for all ports
void si3452_port_status_get(poe_port_status_t *port_status, vtss_port_no_t port_index)
{
    char pwr_enable_status;
    char detection;
    char port_status_reg;
    poe_custom_entry_t poe_hw_conf;

    port_status_reg = si3452_read_port_status(port_index);


    // Extract status - See Table 1:Si3452 register map.
    pwr_enable_status = (port_status_reg >> 6) & 0x1;
    detection   = port_status_reg & 0x7;

    // Get PoE hardware board configuration
    poe_hw_conf = poe_custom_get_hw_config(port_index, &poe_hw_conf);


    T_DG_PORT(VTSS_TRACE_GRP_CUSTOM, port_index, "status = 0x%X, detection = 0x%X", port_status_reg, detection);
    if (poe_hw_conf.available == false) {
        // PoE chip not mounted for this port
        port_status[port_index] = POE_NOT_SUPPORTED;
    } else if (detection == 4) { // 4 = Detection Good - See table 2 in documentation
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
void si3452_set_power_limit_channel(vtss_port_no_t port_index, int max_port_power)
{
    int vee = si3452_get_vee(port_index) / 100 ; // convert to DeciVolt
    char icut;
    if (vee == 0) {
        icut = 0; // Avoid divide by zero.
    } else {
        icut = (313 * max_port_power) / vee  ;
    }

    char port_id   = port_index % PORTS_PER_POE_CHIP; // 4 ports per chip - See Table 1 - SI3452 register map.

    if (port_id == 0) {
        si3452_device_wr(port_index, SI3452_PORT_1_ICUT, icut);
    } else if (port_id == 1) {
        si3452_device_wr(port_index, SI3452_PORT_2_ICUT, icut);
    } else if (port_id == 2) {
        si3452_device_wr(port_index, SI3452_PORT_3_ICUT, icut);
    } else if (port_id == 3) {
        si3452_device_wr(port_index, SI3452_PORT_4_ICUT, icut);
    } else {
        T_E("Unknown port_id");
    }
    T_NG_PORT(VTSS_TRACE_GRP_CUSTOM, port_index, "vee = %d, max_port_power = %d,icut set to %d", vee, max_port_power, icut);
}



// Determine the class for each port. Passed back via pointer
void si3452_get_all_port_class(char *classes, vtss_port_no_t port_index)
{
    int class_reg = si3452_read_port_status(port_index);
    // See si3452 documentation Table 1.
    classes[port_index] = (class_reg >> 3) & 0x7;


    T_NG_PORT(VTSS_TRACE_GRP_CUSTOM, port_index, "classes[port_index] = %d, class_reg = 0x%X", classes[port_index], class_reg);


    // We do not use the class encoding ( See Table 2 in documentation). Convert Class 0 ( encoded as 6 )  to 0
    if (classes[port_index] > 4) {
        classes[port_index] = 0;
    }
}



// Updates the poe status with current and power consumption
void si3452_get_port_measurement(poe_status_t *poe_status, vtss_port_no_t port_index)
{

    T_NG(VTSS_TRACE_GRP_CUSTOM, "Entering si3452_get_port_measurement");
    uchar current_used_lsb = 0, current_used_msb = 0;

    int power_used;
    int current_used = 0;
    int vee;


    T_RG_PORT(VTSS_TRACE_GRP_CUSTOM, port_index, "si3452_poe_port_ok = %d", si3452_poe_port_ok(port_index));
    if (si3452_poe_port_ok(port_index)) {
        // Select which port to get current infomation from.
        char command          = 0x1C + (port_index %  PORTS_PER_POE_CHIP); // Read current, Table 24. Si3452/3 Command Codes
        si3452_device_wr(port_index, SI3452_COMMAND_REGISTER, command); // Initialize the read


        char port_id   = port_index % PORTS_PER_POE_CHIP; // 4 ports per chip - See Table 1 - SI3452 register map.
        if (port_id == 0) {
            si3452_device_rd(port_index, SI3452_PORT_1_MEASURE_CURRENT_LSB, &current_used_lsb);
            si3452_device_rd(port_index, SI3452_PORT_1_MEASURE_CURRENT_MSB, &current_used_msb);
        } else if (port_id == 1) {
            si3452_device_rd(port_index, SI3452_PORT_2_MEASURE_CURRENT_LSB, &current_used_lsb);
            si3452_device_rd(port_index, SI3452_PORT_2_MEASURE_CURRENT_MSB, &current_used_msb);
        } else if (port_id == 2) {
            si3452_device_rd(port_index, SI3452_PORT_3_MEASURE_CURRENT_LSB, &current_used_lsb);
            si3452_device_rd(port_index, SI3452_PORT_3_MEASURE_CURRENT_MSB, &current_used_msb);
        } else if (port_id == 3) {
            si3452_device_rd(port_index, SI3452_PORT_4_MEASURE_CURRENT_LSB, &current_used_lsb);
            si3452_device_rd(port_index, SI3452_PORT_4_MEASURE_CURRENT_MSB, &current_used_msb);
        } else {
            T_E("Unknown port_id");
        }

        // Convert to mA. Port current is in 100us units - see table 3:"device command summary"
        current_used = ((current_used_msb << 8) + current_used_lsb) / 10;
        // Get the vee value ( in milivolt )
        vee = si3452_get_vee(port_index);
        T_DG_PORT(VTSS_TRACE_GRP_CUSTOM, port_index, "current_msb = %d, current_lsb=%d, current_used = %u",
                  current_used_msb, current_used_lsb, current_used);
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
static void si3452_set_port_mode (vtss_port_no_t port_index, char mode)
{
    char port_id   = port_index % PORTS_PER_POE_CHIP; // 4 ports per chip - See Table 1 - SI3452 register map.
    T_IG_PORT(VTSS_TRACE_GRP_CUSTOM, port_index, "Mode:%d", mode);
    if (port_id == 0) {
        si3452_device_wr(port_index, SI3452_PORT_1_CONFIG, mode);
    } else if (port_id == 1) {
        si3452_device_wr(port_index, SI3452_PORT_2_CONFIG, mode);
    } else if (port_id == 2) {
        si3452_device_wr(port_index, SI3452_PORT_3_CONFIG, mode);
    } else if (port_id == 3) {
        si3452_device_wr(port_index, SI3452_PORT_4_CONFIG, mode);
    } else {
        T_E("Unknown port_id");
    }

}
// Things that is needed after a reset
void si3452_poe_init(vtss_port_no_t port_index)
{
    si3452_device_wr(port_index, SI3452_COMMAND_REGISTER, 14); // Reset PoE Chip for starting in a known state

    T_NG(VTSS_TRACE_GRP_CUSTOM, "Entering si3452_poe_init");
    si3452_set_port_mode(port_index, 0xF); // Set to auto mode
}


// Enables or disable poe for a port
void si3452_poe_enable(vtss_port_no_t port_index, BOOL enable)
{
    // The enabling / disabling of the PoE ports s´dones't work due to some SillionLabs bugs.
    // This is a work around for now
    if (enable) {
        si3452_set_port_mode(port_index, 0x0F); // Set mode to auto
        T_IG_PORT(VTSS_TRACE_GRP_CUSTOM, port_index, "Enable");
    } else {
        si3452_set_port_mode(port_index, 0x0); // Set mode to shutdown
        VTSS_MSLEEP(50); // We have seen that the shutdown command is missed if we set it to semi mode without this wait.
        si3452_set_port_mode(port_index, 0x0E); // Set mode to semi
        T_IG_PORT(VTSS_TRACE_GRP_CUSTOM, port_index, "Disable");
    }
}

// Function that returns 1 is a PoE chip set is found, else 0.
int si3452_is_chip_available(vtss_port_no_t port_index)
{
    uchar hw_rev  = 0;
    static BOOL init_done = FALSE;
    int i;

    // Initialize chip found array.
    if (!init_done) {
        for (i = 0 ; i < VTSS_PORTS; i++) {
            si3452_chip_found[i] = DETECTION_NEEDED;
        }
        init_done = TRUE;
    }

    // We only want to try to detect the si3452 chipset once.
    if (si3452_chip_found[port_index] == DETECTION_NEEDED) {
        i2c_wait = 5; // Set timeout low during detection in order to fast detection.

        // We detects the chipset by reading the hardware revison register.
        si3452_device_rd(port_index, SI3452_HARDWARE_REVISON, &hw_rev);


        if (hw_rev == 0xB ) {
            si3452_chip_found[port_index] = FOUND;
            T_IG_PORT(VTSS_TRACE_GRP_CUSTOM, port_index, "PoE si3452 chipset found, hw_rev = 0x%X", hw_rev);
        } else {
            si3452_chip_found[port_index] = NOT_FOUND;
            T_IG_PORT(VTSS_TRACE_GRP_CUSTOM, port_index, "PoE si3452 chipset NOT found, hw_rev = 0x%X", hw_rev);
        }

        i2c_wait = 100; // Set timeout "high" for normal accesses
    }


    if (si3452_chip_found[port_index] == FOUND) {
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
