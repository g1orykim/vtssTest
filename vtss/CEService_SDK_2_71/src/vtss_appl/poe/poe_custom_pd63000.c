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

****************************************************************************/
// PoE chip PD63000 from MicroSemi. The user guide that have been used
// for this implementation is :
//  Microsemi - Serial Communication Protocol User Guide - Revision 6.1
/****************************************************************************/

/*lint -esym(459,i2c_rd_data_rdy_interrupt_function)*/


#include "critd_api.h"
#include "poe.h"
#include "poe_custom_pd63000_api.h"
#include "interrupt_api.h"
#include "poe_custom_api.h"
#include "misc_api.h"


// Section 4.1. in PD63000 user guide - key definitions
# define COMMAND_KEY   0x00
# define PROGRAM_KEY   0x01
# define REQUEST_KEY   0x02
# define TELEMETRY_KEY 0x03
# define REPORT_KEY    0x52

#define PD_BUFFER_SIZE 15     // All microSemi's access are 15 bytes

// The sequence number is updated by the transmit function, but we have to insert a dummy seq number when building the data structure.
#define DUMMY_SEQ_NUM 0x00

static poe_chip_found_t pd63000_chip_found  =  DETECTION_NEEDED;

// PD power banking according to Power Banks Definition table in section 4.5.8 in user guide.
# define BACKUP_POWER_BANK 0
# define PRIMARY_POWER_BANK 0
# define ALL_POWER_BANKS 0

#define POE_INTERRUPT_FLAG 0x2
static cyg_flag_t poe_interrupt_wait_flag; // Flag for signaling that data from the PoE chip are ready to be read out.


static void i2c_rd_data_rdy_interrupt_function(vtss_interrupt_source_t    source_id,
                                               u32                        instance_id)
{
    T_WG(VTSS_TRACE_GRP_CUSTOM, "Got I2C data ready interrupt");

    cyg_flag_setbits(&poe_interrupt_wait_flag, POE_INTERRUPT_FLAG); // Signal that there have been an interrupt.
    // Restart interrupt handler.
    (void) vtss_interrupt_source_hook_set(i2c_rd_data_rdy_interrupt_function,
                                          INTERRUPT_SOURCE_I2C,
                                          INTERRUPT_PRIORITY_POE);
}


int pd63000_wait_time = 50; // Wait 400 ms according to the synchonization descriped in section 7, but from e-mail from Shmulik this can be set to 50 ms.
// Function that wait for the PoE chipset to give an interrupt for that data is ready to be read.
static void wait_for_i2c_data(void)
{
    // Set timeout.
    cyg_tick_count_t  loop_timer = cyg_current_time() + VTSS_OS_MSEC2TICK(pd63000_wait_time);

    // Wait for timeout or for an interrupt from the pd63000.
    if (cyg_flag_timed_wait(&poe_interrupt_wait_flag, POE_INTERRUPT_FLAG, CYG_FLAG_WAITMODE_OR , loop_timer) ? 0 : 1) {
        T_RG(VTSS_TRACE_GRP_CUSTOM, "I2C read timeout data can not be trusted\n");
    }
}

// Debug function for printing the I2C buffer
// In : buf - Pointer to the I2C data
//      len - The number of data bytes
static void print_buffer (uchar *buf, uchar len, int line)
{
    int i;
    i8 str_buf[100];
    i8 str_buf_hex[100];

    strcpy(&str_buf[0], "");
    for (i = 0; i < len ; i++) {
        sprintf(str_buf_hex, "0x%02X ", buf[i]);
        strcat(&str_buf[0], &str_buf_hex[0]);
    }

    T_IG(VTSS_TRACE_GRP_CUSTOM, "Line:%d - %s", line, &str_buf[0]);
}

// The sequence number is also named ECHO in the user guide. Two consecutive
// messages must not contain the same sequunce number. See section 4.1 - ECHO
// in the user guide.
// In : reset_seq_num - Set to TRUE in order to reset the sequence number to 0.
static char get_seq_num(BOOL reset_seq_num)
{
    static uchar seq_num = 0;

    if (reset_seq_num) {
        seq_num = 0;
    } else if (seq_num == 255) {
        seq_num = 0;
    } else {
        seq_num++;
    }
    return seq_num;
}


// Updates the check sum for the command. See section 4.1-CHECKSUM in the user guide
// In : buf - pointer to the I2C data
static void pd63000_update_check_sum (uchar *buf)
{
    int buf_index = 0;
    unsigned int sum = 0;

    // Find the check for the data (skip the last 2 bytes - They are the check sum bytes going to be replaced.
    for (buf_index = 0 ; buf_index < PD_BUFFER_SIZE - 2; buf_index++) {
        sum += buf[buf_index]; // Sum up the checksum
    }

    // Convert from integer to 2 bytes.
    buf[13] = sum >> 8;
    buf[14] = sum & 0xFF;

    T_NG(VTSS_TRACE_GRP_CUSTOM, "Check sum = %u, buf[13] = %u, buf[14] = %u", sum, buf[13], buf[14]);
}


// Check if the check sum is correct. See Section 4.1 - CHECKSUM in the user guide
// In : buf - pointer to the I2C data
BOOL pd63000_check_sum_ok (uchar *buf)
{
    int buf_index = 0;
    unsigned int sum = 0;

    // -2 because the Sum bytes shall not be part of the checksum
    for (buf_index = 0 ; buf_index < PD_BUFFER_SIZE - 2; buf_index++) {
        sum += buf[buf_index]; // Sum up the checksum
    }


    // Do the check of the checksum
    if (( buf[13] != (sum >> 8 & 0xFF)) && (buf[14] != (sum & 0xFF)) ) {
        T_WG(VTSS_TRACE_GRP_CUSTOM, "pd6300 I2C check sum error, buf[13] = 0x%X, buf[14] = 0x%X", buf[13], buf[14]);
        print_buffer(&buf[0], PD_BUFFER_SIZE, __LINE__);
        return FALSE;
    } else {
        return TRUE;
    }
}



// Report key - See section 4.6 in the user guide
// expected_seq_num - The expected sequence number for the report
static BOOL report_key_ok (u8 expected_seq_num)
{
    vtss_rc rc;
    BOOL report_key_ok_v = TRUE;

    uchar buf[PD_BUFFER_SIZE];

    int repeat;
    for (repeat = 0 ; repeat < 3; repeat++) {

        rc = pd63000_rd(buf, PD_BUFFER_SIZE);
        if (rc == VTSS_RC_MISC_I2C_REBOOT_IN_PROGRESS) {
            return TRUE;
        } else if (rc != VTSS_RC_OK) {
            return FALSE;
        }
        T_NG(VTSS_TRACE_GRP_CUSTOM, "Entering check_report_key");

        // First make sure that the checksum is correct
        if (pd63000_check_sum_ok(&buf[0])) {
            if (buf[0] != REPORT_KEY) {
                T_WG(VTSS_TRACE_GRP_CUSTOM, "Report Key error - Data isn't a report key buf");
                print_buffer(&buf[0], PD_BUFFER_SIZE, __LINE__);
                report_key_ok_v = FALSE;
            } else if (buf[2] == 0x00 && buf[3] == 0x00) {
                // Command received/correctly executed
                report_key_ok_v = TRUE;
            } else if  (buf[2] == 0xFF && buf[3] == 0xFF && buf[4] == 0xFF && buf[5] == 0xFF) {
                T_WG(VTSS_TRACE_GRP_CUSTOM, "Report key error - Command received / wrong checksum ");
                print_buffer(&buf[0], PD_BUFFER_SIZE, __LINE__);
                report_key_ok_v = FALSE;
            } else if  (buf[2] > 0x0  && buf[2] < 0x80) {
                T_WG(VTSS_TRACE_GRP_CUSTOM, "Report key error - Failed execution / conflict in subject bytes");
                print_buffer(&buf[0], PD_BUFFER_SIZE, __LINE__);
                report_key_ok_v = FALSE;
            } else if  (buf[2] > 0x80  && buf[2] < 0x90) {
                T_WG(VTSS_TRACE_GRP_CUSTOM, "Report key error - Failed execution / Wrong data byte value");
                print_buffer(&buf[0], PD_BUFFER_SIZE, __LINE__);
                report_key_ok_v = FALSE;
            } else if  (buf[2] == 0xFF  && buf[3] == 0xFF) {
                T_WG(VTSS_TRACE_GRP_CUSTOM, "Report key error - Failed execution / Undefined key value");
                print_buffer(&buf[0], PD_BUFFER_SIZE, __LINE__);
                report_key_ok_v = FALSE;
            } else {
                report_key_ok_v = TRUE;
            }
        } else {
            T_WG(VTSS_TRACE_GRP_CUSTOM, "Report key error - Checksum error");
            print_buffer(&buf[0], PD_BUFFER_SIZE, __LINE__);
            report_key_ok_v = FALSE;
        }

        // Stop if the report is the expected report
        if (expected_seq_num == buf[1]) {
            break;
        }
        T_WG(VTSS_TRACE_GRP_CUSTOM, "expect_seq:%d, seq:%d", expected_seq_num, buf[1]);
    }


    return report_key_ok_v;
}


// Function that reads controller response ( reponds upon request ), and check the key and checksum.
// Returns 0 in case of error else 1.
static BOOL get_controller_response (uchar *buf, uchar expected_seq_num)
{
    vtss_rc rc;
    BOOL result ;
    uchar repeat; // If we don't get a wrong reply (wrong sequence number) we will try and read again.

    // Re-read max 5 times
    for (repeat = 0 ; repeat < 5; repeat++) {

        // Try and read the data (Step A)
        rc = pd63000_rd(buf, PD_BUFFER_SIZE);
        if (rc == VTSS_RC_MISC_I2C_REBOOT_IN_PROGRESS) {
            memset(buf, 0x0, PD_BUFFER_SIZE);
            return TRUE;
        } else if (rc != VTSS_RC_OK) {
            memset(buf, 0x0, PD_BUFFER_SIZE);
            return FALSE;
        }

        T_RG(VTSS_TRACE_GRP_CUSTOM, "get_controller_response");

        // Check checksum
        if (!pd63000_check_sum_ok(&buf[0]) || buf[0] != TELEMETRY_KEY) {

            result = FALSE; // Communication error.
            print_buffer(&buf[0], PD_BUFFER_SIZE, __LINE__);


            if (expected_seq_num != buf[1]) {
                T_WG(VTSS_TRACE_GRP_CUSTOM, "Seq number errorr: expect_seq:%d, seq:%d", expected_seq_num, buf[1]);
                continue; // We got a wrong sequence number  - Try and read again
            } else if (!pd63000_check_sum_ok(&buf[0])) {
                T_WG(VTSS_TRACE_GRP_CUSTOM, "Checksum error");
            } else {
                T_WG(VTSS_TRACE_GRP_CUSTOM, "Telemetry error");
            }

        } else {
            result =  TRUE; // Everything is OK
        }

        break; // Ok - We got a result - break out.
    }

    return result;
}


// Do the real I2C transmitting. Returns FALSE is the trasmit went wrong.
static BOOL is_i2c_tx_ok (uchar *buf)
{
    buf[1] = get_seq_num(FALSE);

    // Update the checksum
    pd63000_update_check_sum(buf);

    T_RG(VTSS_TRACE_GRP_CUSTOM, "Clearing poe_interrupt_wait_flag");
    cyg_flag_maskbits(&poe_interrupt_wait_flag, 0); // Clear interrupt flag so we can detect when there have been a response from the PD63000 device.

    int bytes_tranmitted = pd63000_wr(buf, PD_BUFFER_SIZE);

    // Check that all bytes were transmitted
    if (bytes_tranmitted != PD_BUFFER_SIZE) {
        T_WG(VTSS_TRACE_GRP_CUSTOM, "I2C communication error with PoE controller, bytes transmitted = %u", bytes_tranmitted);
        print_buffer(&buf[0], PD_BUFFER_SIZE, __LINE__);
        return FALSE;
    } else {
        return TRUE; // Ok - Everthing went well.
    }
}


// Transmit the command
// Return TRUE in case that everything went well
// If the transmitted data is a request key the data is return in the buf pointer.
static BOOL pd63000_i2c_tx(uchar *buf)
{
    const u8 RETRANSMIT_MAX = 4;  // Maximum number of time we want to try and retransmit before resetting

    uchar buf_copy[PD_BUFFER_SIZE];
    memcpy(&buf_copy[0], buf, PD_BUFFER_SIZE);

    T_RG(VTSS_TRACE_GRP_CUSTOM, "Entering pd63000_i2c_tx");

    char retransmit_cnt;

    BOOL tx_ok = FALSE; // Bool for determine if the transmit went well, or if we shall try and do a reset of the PoE chip.

    for (retransmit_cnt = 1; retransmit_cnt <= RETRANSMIT_MAX; retransmit_cnt ++ ) {
        T_RG(VTSS_TRACE_GRP_CUSTOM, "Retransmit cnt = %d", retransmit_cnt);
        // Do the I2C transmission and check report key.
        if (is_i2c_tx_ok(&buf_copy[0])) {

            // Section 4.6 in PD63000/G user guide - check report in case of command or program
            if ((buf_copy[0] == COMMAND_KEY || buf_copy[0] == PROGRAM_KEY)) {
                T_RG(VTSS_TRACE_GRP_CUSTOM, "Check report key");
                if (report_key_ok(buf_copy[1])) {
                    if (retransmit_cnt > 1) {
                        T_WG(VTSS_TRACE_GRP_CUSTOM, "RPORT_KEY");
                    }
                    tx_ok = TRUE;
                } else {
                    T_WG(VTSS_TRACE_GRP_CUSTOM, "Report key error");
                    print_buffer(&buf_copy[0], PD_BUFFER_SIZE, __LINE__);
                }
            } else if (buf_copy[0] == REQUEST_KEY) {
                if (retransmit_cnt > 1) {
                    T_WG(VTSS_TRACE_GRP_CUSTOM, "REQUEST_KEY");
                }
                tx_ok = get_controller_response(&buf[0], buf_copy[1]);
            } else {
                if (retransmit_cnt > 1) {
                    T_WG(VTSS_TRACE_GRP_CUSTOM, "ELSE");
                }
                tx_ok = TRUE;
            }
        } else {
            T_WG(VTSS_TRACE_GRP_CUSTOM, "tx error");
        }

        if (tx_ok) {
            break; // Ok - Transmission perform and accepted by the PoE chip.
        }
        // Something went wrong, we should do synchronization. We do not do as described in section 7 in the userguide. Instead
        // microSemi has suggested that we simply wait more than 10 sec. After 10 sec. the I2C buffer is flushed, and we do not have
        // to do a reset of the PoE chip, which microSemi didn't like. We only do the wait once.
        if (retransmit_cnt == RETRANSMIT_MAX - 1) {
            T_DG(VTSS_TRACE_GRP_CUSTOM, "Waiting for I2C controller reset");
        }
        pd63000_wait_time += 50; // Add 50 ms to the time out
        T_WG(VTSS_TRACE_GRP_CUSTOM, "Retransmitting %d", retransmit_cnt);
        print_buffer(&buf_copy[0], PD_BUFFER_SIZE, __LINE__);
    }


    pd63000_wait_time = 50;
    //
    if (!tx_ok) {
        T_WG(VTSS_TRACE_GRP_CUSTOM, "I2C communication error - resetting PoE chip-set");
        pd63000_chip_found = NOT_FOUND;
    }

    return tx_ok
           ;
}



//
// The PD63000 commands -- See MicroSemi PD63000/G user guide
//
static void pd63000_interrupt_enable (void)
{

    // From mail
    // There is an option to get an interrupt when the I2C buffer is ready with the info:
    // It is mask 0x1E, you should set it to enable by writing "1" to data 6.
    // (As you can see, that mask is not published in the protocol).
    // Section 4.5.20 in user guide


    T_DG(VTSS_TRACE_GRP_CUSTOM, "Entering pd63000_interrupt_enable");
    uchar buf[PD_BUFFER_SIZE] = {COMMAND_KEY,
                                 DUMMY_SEQ_NUM,
                                 0x07,
                                 0x56,
                                 0x1E,
                                 0x01,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00
                                };
    (void) pd63000_i2c_tx(&buf[0]);
}

void pd63000_10sec_reset_enable (void)
{

    // From mail
    // The 10 sec is a new feature, which we have implemented recently in our firmware for the old generation ICs (PD63000), and in the first firmware for the new generation ICs (690xx, the one you have).
    // It is not updated yet in the document, it is just mentioned in the release note for the firmware (of the old generation ICs).
    // Please see section 3 in the attached release note.
    // You can set it according to section 4.5.20
    // The mask key is 0x1B (this mask is not shared with our customers).
    // 0=disable
    // 1= enable

    T_DG(VTSS_TRACE_GRP_CUSTOM, "Entering pd63000_10sec_reset_enable");
    uchar buf[PD_BUFFER_SIZE] = {COMMAND_KEY,
                                 DUMMY_SEQ_NUM,
                                 0x07,
                                 0x56,
                                 0x1B,
                                 0x01,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00
                                };
    (void) pd63000_i2c_tx(&buf[0]);
}

// Section 4.5.1 in user guide
void pd63000_factory_default (void)
{

    T_DG(VTSS_TRACE_GRP_CUSTOM, "Entering pd63000_factory_default");
    uchar buf[PD_BUFFER_SIZE] = {PROGRAM_KEY,
                                 DUMMY_SEQ_NUM,
                                 0x2D,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00
                                };
    (void) pd63000_i2c_tx(&buf[0]);
}


// Section 4.7.7
static int pd63000_get_active_power_bank(void)
{
    uchar buf[PD_BUFFER_SIZE] = {REQUEST_KEY,
                                 DUMMY_SEQ_NUM,
                                 0x07,
                                 0x0B,
                                 0x17,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00
                                };

    if (pd63000_i2c_tx(&buf[0])) {
        T_IG(VTSS_TRACE_GRP_CUSTOM, "Active power bank = %d, power limit = 0x%X , 0x%X", buf[9], buf[10], buf[11]);
        return buf[9];
    } else {
        T_WG(VTSS_TRACE_GRP_CUSTOM, "Response error - setting active bank to 0");
        print_buffer(&buf[0], PD_BUFFER_SIZE, __LINE__);
        return 0;
    }
}


// Section 4.7.5 - Getting system status
static BOOL pd63000_get_system_status(void)
{
    uchar buf[PD_BUFFER_SIZE] = {REQUEST_KEY,
                                 DUMMY_SEQ_NUM,
                                 0x07,
                                 0x3D,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00
                                };
    if (!is_i2c_tx_ok(&buf[0])) {
        T_WG(VTSS_TRACE_GRP_CUSTOM, "Didn't get system status");
        return FALSE;
    } else {
        return TRUE;
    }
}



poe_power_source_t pd63000_get_power_source (void)
{
    return PRIMARY;
    /*    switch (pd63000_get_active_power_bank()) {
        case BACKUP_POWER_BANK :
            return BACKUP;
        default:
            return PRIMARY;
            }*/
}

// Section 4.5.3
void pd63000_save_system_settings(void)
{
    T_DG(VTSS_TRACE_GRP_CUSTOM, "pd63000_save_system_settings - Section 4.5.3");
    uchar buf[PD_BUFFER_SIZE] = {PROGRAM_KEY,
                                 DUMMY_SEQ_NUM,
                                 0x06,
                                 0x0F,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00
                                };
    (void) pd63000_i2c_tx(&buf[0]);
}


void pd63000_set_power_bank(int power_limit, char bank)
{

    //  pd63000_set_power_banks, Section 4.5.8 in user guide
    int max_shutdown_voltage = 570; // Set max. voltage for the primary supply to 57.0 Volt.
    int min_shutdown_voltage = 440; // Set min. voltage for the primary supply to 44.0 Volt.

#ifdef VTSS_HW_CUSTOM_POE_CHIP_PD63000_SUPPORT
    char guard_band = 0x13; // Recommended value according to PD63000 userguide section 4.5.8
#else
    // Recommended value according to PD69000 & PD69100 userguide section 4.5.8 is 0x1, but recommended work-around for Bugzilla#8436 is to set is t 2.
    char guard_band = 0x2;
#endif

    T_DG(VTSS_TRACE_GRP_CUSTOM, "pd63000_get_active_power_bank:%d  new power_limit:0x%X, guard_band:0x%X", pd63000_get_active_power_bank(),  power_limit, guard_band);


    // Split integers into a high and a low byte
    char power_limit_l = power_limit & 0xFF;
    char power_limit_h = power_limit >> 8 & 0xFF;

    char max_shutdown_voltage_l = max_shutdown_voltage & 0xFF;
    char max_shutdown_voltage_h = max_shutdown_voltage >> 8 & 0xFF;

    char min_shutdown_voltage_l = min_shutdown_voltage & 0xFF;
    char min_shutdown_voltage_h = min_shutdown_voltage >> 8 & 0xFF;


    // Transmit the command
    uchar buf[PD_BUFFER_SIZE] = {COMMAND_KEY,
                                 DUMMY_SEQ_NUM,
                                 0x07,
                                 0x0B,
                                 0x57,
                                 bank,
                                 power_limit_h,
                                 power_limit_l,
                                 max_shutdown_voltage_h,
                                 max_shutdown_voltage_l,
                                 min_shutdown_voltage_h,
                                 min_shutdown_voltage_l,
                                 guard_band,
                                 0x00,
                                 0x00
                                };
    (void) pd63000_i2c_tx(&buf[0]);
    pd63000_save_system_settings(); // I think we have to save system settings at this point - See Section 4.5.8 - "Save System Settings".
}

// Section 4.5.8 in user guide
void pd63000_set_power_banks(int primary_power_limit, int backup_power_limit)
{

    static int last_primary_power_limit = 0;
    static int last_backup_power_limit = 0;

    if (primary_power_limit != last_primary_power_limit || backup_power_limit != last_backup_power_limit) {
        last_primary_power_limit = primary_power_limit;
        last_backup_power_limit  = backup_power_limit;

//      pd63000_set_power_bank(backup_power_limit, BACKUP_POWER_BANK);
//      pd63000_set_power_bank(primary_power_limit, PRIMARY_POWER_BANK);
        pd63000_set_power_bank(primary_power_limit, ALL_POWER_BANKS); // If both primary and backup is available the it is the primary supply that is used.
    }
}


// Section 4.5.13 in user guide
void pd63000_set_enable_disable_channel(char channel, char enable, poe_mode_t mode)
{
    uchar af_mode;
    // UG section 4.5.13 - AF Mask (PD69000 only): 0 - only IEEE802.3af operation; N - stay with the last mode (IEEE802.3af or IEEE802.3at).
    if (mode == POE_MODE_POE) {
        af_mode = 0; // AF mode
    } else {
        af_mode = 1;  // N = 1 according to mail from micsroSemi.
    }

    T_DG(VTSS_TRACE_GRP_CUSTOM, "pd63000_set_enable_disable_channel - Section 4.5.13 in user guide, channel:%d, enable:%d", channel, enable);
    uchar buf[PD_BUFFER_SIZE] = {COMMAND_KEY,
                                 DUMMY_SEQ_NUM,
                                 0x05,
                                 0x0C,
                                 channel,
                                 enable,
                                 af_mode,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00
                                };
    (void) pd63000_i2c_tx(&buf[0]);
}


void pd63000_set_power_limit_channel(char channel, uint ppl )
{
    // We uses deci Watts while pd6300 uses mili Watts, so we have to do a convertion
    ppl = ppl  * 100;

    char ppl_l, ppl_h;

    // Section 4.5.14 in user guide
    uint max_port_power = poe_custom_get_port_power_max(0
                                                       ) * 100; // Getting max port power in milli watt
    if (ppl > max_port_power) {
        ppl = max_port_power;
    }
    // convert from integer to two bytes
    ppl_l = ppl & 0xFF;
    ppl_h = ppl >> 8 & 0xFF;


    uchar buf2[PD_BUFFER_SIZE] = {COMMAND_KEY,
                                  DUMMY_SEQ_NUM,
                                  0x05,
                                  0x0B,
                                  channel,
                                  ppl_h,
                                  ppl_l,
                                  0x00,
                                  0x00,
                                  0x00,
                                  0x00,
                                  0x00,
                                  0x00,
                                  0x00,
                                  0x00
                                 };
    (void) pd63000_i2c_tx(&buf2[0]);

    // Section 4.5.15 in user guide

    uchar buf[PD_BUFFER_SIZE] = {COMMAND_KEY,
                                 DUMMY_SEQ_NUM,
                                 0x05,
                                 0xA2,
                                 channel,
                                 ppl_h,
                                 ppl_l,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00
                                };
    (void) pd63000_i2c_tx(&buf[0]);
}



//  Section 4.5.16 in PD63000 user guide
void pd63000_set_port_priority(char channel, poe_priority_t priority )
{


    static poe_priority_t last_priority[VTSS_PORTS];

    if (last_priority[(int) channel] != priority) {
        last_priority[(int) channel] = priority;
        T_DG(VTSS_TRACE_GRP_CUSTOM, "pd63000_set_port_priority - Section 4.5.16 in PD63000 user guide");
        char pd63000_prio = 3; // Default to low priority

        // Type conversion -- See section 4.5.16 in PD63000 user guide
        switch (priority) {
        case LOW :
            pd63000_prio = 3;
            break;
        case HIGH:
            pd63000_prio = 2;
            break;
        case CRITICAL:
            pd63000_prio = 1;
            break;
        default:
            T_WG(VTSS_TRACE_GRP_CUSTOM, "Unkown priority");
        }


        uchar buf[PD_BUFFER_SIZE] = {COMMAND_KEY,
                                     DUMMY_SEQ_NUM,
                                     0x05,
                                     0x0A,
                                     channel,
                                     pd63000_prio,
                                     0x00,
                                     0x00,
                                     0x00,
                                     0x00,
                                     0x00,
                                     0x00,
                                     0x00,
                                     0x00,
                                     0x00
                                    };
        (void) pd63000_i2c_tx(&buf[0]);
    }
}

// Section 4.5.22 in user guide
void pd63000_set_extended_poe_dev_params(int max_power )
{
    char max_power_l = max_power & 0xFF;
    char max_power_h = max_power >> 8 & 0xFF;

    T_DG(VTSS_TRACE_GRP_CUSTOM, "pd63000_set_extended_poe_dev_params");
    uchar buf[PD_BUFFER_SIZE] = {COMMAND_KEY,
                                 DUMMY_SEQ_NUM,
                                 0x07,
                                 0xA3,
                                 0x08,
                                 max_power_h,
                                 max_power_l,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00
                                };
    (void) pd63000_i2c_tx(&buf[0]);
}


// Section 4.7.5 in user guide
// Reads system status without requesting it. Needed after reset - see section 4.1.
BOOL pd63000_rd_system_status_ok (void)
{
    uchar buf[PD_BUFFER_SIZE];
    // Get response -- Check that response is valid
    if (get_controller_response(&buf[0], 0xFF)) {
        char cpu_status1     = buf[3];
        char cpu_status2     = buf[4];
//        char factory_default = buf[5];
//        char gie             = buf[6];
//        char private_label   = buf[7];
//        char device_fail     = buf[9];
//        char temp_disconnect = buf[10];
//        char temp_alarm      = buf[11];
//        int  interrupt       = buf[12] << 8 & buf[13];

        if (cpu_status1 == 0x1) {
            T_WG(VTSS_TRACE_GRP_CUSTOM, "PoE controller firmware download is required");
        }

        if (cpu_status2 == 0x1) {
            T_WG(VTSS_TRACE_GRP_CUSTOM, "PoE controller memory error");
        }

        return TRUE;

    } else {
        T_WG(VTSS_TRACE_GRP_CUSTOM, "System data read failure, 0x%X 0x%X 0x%X", buf[0], buf[1], buf[2]);
        print_buffer(&buf[0], PD_BUFFER_SIZE, __LINE__);
        return FALSE;
    }
}



BOOL check_startup_response(void)
{
    uchar buf[PD_BUFFER_SIZE];

    // Get response -- Check that response is valid
    if (get_controller_response(&buf[0], 0xFF) &&
        (buf[0] == 0x03)) {
        T_DG(VTSS_TRACE_GRP_CUSTOM, "Found Sw version.");
        return TRUE;
    } else {
        // Invalid response = no chipset found.
        T_WG(VTSS_TRACE_GRP_CUSTOM, "No chipset found.");
        print_buffer(&buf[0], PD_BUFFER_SIZE, __LINE__);
        return FALSE;
    }
}


// Section 4.5.1 in user guide
// Returns TRUE if reset were done correct else FALSE
void pd63000_reset_command (void)
{
    if (pd63000_chip_found != NOT_FOUND) {

        T_EG(VTSS_TRACE_GRP_CUSTOM, "pd63000_reset_command");

        uchar buf[PD_BUFFER_SIZE] = {COMMAND_KEY,
                                     DUMMY_SEQ_NUM,
                                     0x07,
                                     0x55,
                                     0x00,
                                     0x55,
                                     0x00,
                                     0x55,
                                     0x00,
                                     0x00,
                                     0x00,
                                     0x00,
                                     0x00,
                                     0x00,
                                     0x00
                                    };

        (void) is_i2c_tx_ok(&buf[0]);

        VTSS_OS_MSLEEP(4000); // Make sure that the PoE chipset has reset

//        pd63000_chip_found = DETECTION_NEEDED; // Signal the we has to re-detect the chipset-

        (void) get_seq_num(TRUE); // Reset the sequence number
    }
}


// Section 4.7.23 in user guide
// Gets the class for the PD and updates the reserved power in the status field
void pd63000_get_all_port_class(char *classes)

{

    T_RG(VTSS_TRACE_GRP_CUSTOM, "Entering pd63000_get_all_port_class - Section 4.7.23");
    int port_index;
    int channel;
    int grp_num;
    char class_msb, class_lsb;

    // Each group contains information for 16 ports.
    const char num_of_ports_per_grp = 16;
    const char num_of_channels_per_grp = 8;
    for (grp_num = 0 ; grp_num <= VTSS_PORTS / num_of_ports_per_grp; grp_num++) {

        // Send request to get status
        uchar buf[PD_BUFFER_SIZE]  = {REQUEST_KEY,
                                      DUMMY_SEQ_NUM,
                                      0x07,
                                      0x61,
                                      grp_num,
                                      0x00,
                                      0x00,
                                      0x00,
                                      0x00,
                                      0x00,
                                      0x00,
                                      0x00,
                                      0x00,
                                      0x00,
                                      0x00
                                     };

        // Get response -- Check that response is valid
        if (pd63000_i2c_tx(&buf[0])) {
            // Channel represents the port in the response which starts from byte 4 - See Section 4.7.23 in user guide.
            for (channel = 0 ; channel < num_of_channels_per_grp; channel++) {
                port_index = (channel * 2) + (grp_num *  num_of_ports_per_grp);  // Each channel contains information for 2 ports


                // Each channel contians information for 2 ports.
                T_NG_PORT(VTSS_TRACE_GRP_CUSTOM, (vtss_port_no_t)port_index, "buf[channel + 3] = 0x%X, channel = %d", buf[channel + 3], channel);
                if (port_index >= VTSS_PORTS) {
                    break; // We have reached that last port for this switch
                } else {
                    class_lsb = buf[channel + 3] & 0xF;
                    classes[port_index] = class_lsb;
                    T_DG_PORT(VTSS_TRACE_GRP_CUSTOM, (vtss_port_no_t)port_index, "Got class %d,channel = %d, grp_num = %d", class_lsb, channel, grp_num);
                }


                if (port_index + 1 >= VTSS_PORTS) {
                    break; // We have reached that last port for this switch
                } else {
                    class_msb = buf[channel + 3] >> 4;
                    classes[port_index + 1] = class_msb;
                    T_DG_PORT(VTSS_TRACE_GRP_CUSTOM, (vtss_port_no_t)port_index, "Got class %d", class_msb);
                }
            }
        } else {
            T_WG(VTSS_TRACE_GRP_CUSTOM, "Data did not contain valid data.");
        };
    }
}



// section 4.7.17 in user guide
void pd63000_get_port_measurement(vtss_port_no_t port_index, poe_status_t *poe_status)
{
    int current_used;
    int power_used;
    T_RG(VTSS_TRACE_GRP_CUSTOM, "Entering pd63000_get_port_measurement");

    // Send request to get status
    uchar buf[PD_BUFFER_SIZE]  = {REQUEST_KEY,
                                  DUMMY_SEQ_NUM,
                                  0x05,
                                  0x25,
                                  port_index,
                                  0x00,
                                  0x00,
                                  0x00,
                                  0x00,
                                  0x00,
                                  0x00,
                                  0x00,
                                  0x00,
                                  0x00,
                                  0x00
                                 };

    // Get response -- Check that response is valid
    if (pd63000_i2c_tx(&buf[0])) {
        power_used = (buf[6] << 8) + buf[7];
        current_used = (buf[4] << 8) + buf[5];
    } else {
        T_WG(VTSS_TRACE_GRP_CUSTOM, "Data did not contain valid data.");
        power_used = 0;
        current_used = 0;
    };

    // Update the status structure
    poe_status->power_used[port_index] = power_used / 100; // We use deci watts while PD63000 uses mili watts ( That why we divides with 100 )-
    poe_status->current_used[port_index] = current_used;

    T_NG_PORT(VTSS_TRACE_GRP_CUSTOM, (vtss_port_no_t) port_index, "power_used = %u, current_used = %u", power_used, current_used);
}



// Get software version - Section 4.7.1 in user guide
// Out - Pointer to TXT string that shall contain the firmware information
void pd63000_get_sw_version(char *info)
{
    static u16 sw_ver_found = 0;
    static u8  hw_ver_found = 0;

    if (sw_ver_found == 0) {
        // Send request to get sw version
        uchar buf[PD_BUFFER_SIZE]  = {REQUEST_KEY,
                                      DUMMY_SEQ_NUM,
                                      0x07,
                                      0x1E,
                                      0x21,
                                      0x00,
                                      0x00,
                                      0x00,
                                      0x00,
                                      0x00,
                                      0x00,
                                      0x00,
                                      0x00,
                                      0x00,
                                      0x00
                                     };

        T_DG(VTSS_TRACE_GRP_CUSTOM, "pd63000_get_sw_version");

        // Get response -- Check that response is valid
        if (!pd63000_i2c_tx(&buf[0])) {
            T_DG(VTSS_TRACE_GRP_CUSTOM, "Data did not contain valid data.");
        };

        hw_ver_found = buf[2];
        sw_ver_found = (buf[5] << 8) + buf[6];

        T_IG(VTSS_TRACE_GRP_CUSTOM, "HW Ver.:%d, Prod:%d, sw ver:%d, build:%d, internal sw ver:%d, Asic Patch Num:%d", buf[2], buf[4], (buf[5] << 8) + buf[6], buf[8], (buf[9] << 8) + buf[10], (buf[11] << 8) + buf[12]);
        print_buffer(&buf[0], PD_BUFFER_SIZE, __LINE__);
    }
    sprintf(info, "SW:%d, HW:%d", sw_ver_found, hw_ver_found);
}


//  Power management - Section 4.5.9
void pd63000_set_pm_method(poe_stack_conf_t *poe_stack_cfg)
{

    static BOOL first = TRUE;
    static poe_power_mgmt_t last_state;

    if (first || last_state != poe_stack_cfg->master_conf.power_mgmt_mode) {
        first      = FALSE;
        last_state = poe_stack_cfg->master_conf.power_mgmt_mode;

        T_DG(VTSS_TRACE_GRP_CUSTOM, "pd63000_set_pm_method - Section 4.5.9");
        char pm1 = 0x00;
        char pm2 = 0x00;
        char pm3 = 0x00;

        switch (poe_stack_cfg->master_conf.power_mgmt_mode) {
        case CLASS_CONSUMP :
            // See table in section 4.5.9 in PD63000 user guide
            pm1 = 0x00;
            pm2 = 0x00;
            pm3 = 0x00;
            break;

        case CLASS_RESERVED :
            // See table in section 4.5.9 in PD63000 user guide
            pm1 = 0x04;
            pm2 = 0x01;
            pm3 = 0x00;
            break;

        case ALLOCATED_RESERVED:
            // The actually Max Reserved power is done at the user interface level

            // See table in section 4.5.9 in PD63000 user guide
            pm1 = 0x00;
            pm2 = 0x00;
            pm3 = 0x00;

            break;

        case ALLOCATED_CONSUMP:
            // See table in section 4.5.9 in PD63000 user guide
            pm1 = 0x00;
            pm2 = 0x00;
            pm3 = 0x00;

            break;

        case LLDPMED_CONSUMP:
            // See table in section 4.5.9 in PD63000 user guide
            pm1 = 0x00;
            pm2 = 0x00;
            pm3 = 0x00;

            break;

        case LLDPMED_RESERVED:
            // The actually Max Reserved power is done in LLDP module
            pm1 = 0x04;
            pm2 = 0x00;
            pm3 = 0x00;

            break;
        default:
            T_WG(VTSS_TRACE_GRP_CUSTOM, "Unknown managedment mode");
        }


        T_DG(VTSS_TRACE_GRP_CUSTOM, "Power management pm1 = %d, pm2 = %d, pm3 = %d", pm1, pm2, pm3);
        // Send the Power management command -See Section 4.5.9 in PD63000 user guide
        uchar buf[PD_BUFFER_SIZE] = {COMMAND_KEY,
                                     DUMMY_SEQ_NUM,
                                     0x07,
                                     0x0B,
                                     0x5F,
                                     pm1,
                                     pm2,
                                     pm3,
                                     0x00,
                                     0x00,
                                     0x00,
                                     0x00,
                                     0x00,
                                     0x00,
                                     0x00
                                    };
        (void) pd63000_i2c_tx(&buf[0]);
    }
}


//  Set system masks - Section 4.5.10
static void pd63000_set_system_masks (char mask)
{
    mask |= 1 << 2; // maskbit2 must be 1 according to section 4.5.10.
    // Send the system masks command.
    uchar buf[PD_BUFFER_SIZE] = {COMMAND_KEY,
                                 DUMMY_SEQ_NUM,
                                 0x07,
                                 0x2B,
                                 mask,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00
                                };

    T_IG(VTSS_TRACE_GRP_CUSTOM, "mask:%d", mask);
    (void) pd63000_i2c_tx(&buf[0]);
}


// section 4.7.6 in user guide - Getting systems mask
// In/Out - Pointer to where to put the mask result.
static void pd63000_get_system_mask(char *mask)
{
    T_DG(VTSS_TRACE_GRP_CUSTOM, "pd63000_get_system_mask");

    // Send request to get status
    uchar buf[PD_BUFFER_SIZE]  = {REQUEST_KEY,
                                  DUMMY_SEQ_NUM,
                                  0x07,
                                  0x2B,
                                  0x00,
                                  0x00,
                                  0x00,
                                  0x00,
                                  0x00,
                                  0x00,
                                  0x00,
                                  0x00,
                                  0x00,
                                  0x00,
                                  0x00
                                 };

    // Get response -- Check that response is valid
    if (pd63000_i2c_tx(&buf[0])) {
        print_buffer(&buf[0], PD_BUFFER_SIZE, __LINE__);
        *mask = buf[2];
    } else {
        T_WG(VTSS_TRACE_GRP_CUSTOM, "Did not get system mask data.");
        *mask = 0;
    };
    T_IG(VTSS_TRACE_GRP_CUSTOM, "mask:%d", *mask);
}

// Function for setting the legacy capacitor detection mode
//      enable         : True - Enable legacy capacitor detection
void pd63000_capacitor_detection_set(BOOL enable)
{
    char mask;

    pd63000_get_system_mask(&mask);

    T_IG(VTSS_TRACE_GRP_CUSTOM, "mask:%d", mask);
    if (enable) {
        // Enable Cap detection
        pd63000_set_system_masks(mask | 0x2); // Bit 1 is cap detection- High is enable  (Section 4.5.10)
    } else {
        // Disable Cap detection
        pd63000_set_system_masks(mask & ~0x2); // Bit 1 is cap detection- Low is disable  (Section 4.5.10)
    }
}



// Function for getting the legacy capacitor detection mode
BOOL pd63000_capacitor_detection_get(void)
{
    char mask = 0;

    pd63000_get_system_mask(&mask);

    T_IG(VTSS_TRACE_GRP_CUSTOM, "mask:%d", mask);
    // capacitor detection is bit 1 (See UG - section 4.7.6.)
    if (mask & 0x2) {
        return TRUE;
    } else {
        return FALSE;
    }
}



// Function for getting status for all ports within a group (See Table 6 in User guide)
// In: port_status - Pointer to where to put the result
//     grp_num     - The grp number for the which the port status shall be updated
void pd63000_grp_status_get(poe_port_status_t *port_status, int grp_num)
{
    int port;
    vtss_port_no_t port_index;

    // #¤¤#¤&#% The addresses are NOT consecutive - See Table 6 in Data Sheet !¤!¤%"#!
    int table_6_grp_map[10] = {0x31, 0x32, 0x33, 0x47, 0x48, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F};

    // Send request to get status
    uchar buf[PD_BUFFER_SIZE]  = {REQUEST_KEY,
                                  DUMMY_SEQ_NUM,
                                  0x07,
                                  table_6_grp_map[grp_num],
                                  0x00,
                                  0x00,
                                  0x00,
                                  0x00,
                                  0x00,
                                  0x00,
                                  0x00,
                                  0x00,
                                  0x00,
                                  0x00,
                                  0x00
                                 };

    // Get response -- Check that response is valid
    if (pd63000_i2c_tx(&buf[0])) {
        // port represents the port in the response which starts from byte 3 - See Section 4.7.10 in user guide.
        for (port = 0 ; port < 11; port++) {


            // There are normally 11 ports in each group but !¤##¤ NOT for group 3 (logical 2) #¤¤%" - See - See Section 4.7.10 in user guide.
            switch (grp_num) {
            case 0:
            case 1:
                port_index = grp_num * 11 + port;
                break;
            case 2:
                if (port < 2) {
                    port_index = 22 + port;
                } else if (port > 4 && port < 7) {
                    port_index = 22 + port - 3; // -3 for the 3 ports missing in the middle of the group (See data sheet)
                } else {
                    continue;
                }
                break;
            default:
                port_index = grp_num * 11 + port - 7; // The -7 is the ports missing in group 3.
            }


            // Check that this port does support PoE
            poe_custom_entry_t poe_hw_conf = poe_custom_get_hw_config(port_index, &poe_hw_conf);

            if (port_index >= VTSS_PORTS) {
                break; // We have reached that last port for this switch
            }


            T_IG_PORT(VTSS_TRACE_GRP_STATUS, port_index, "Port status = 0x%X", buf[2 + port]);
            if (!poe_hw_conf.available) {
                port_status[port_index] = POE_NOT_SUPPORTED;
                T_NG_PORT(VTSS_TRACE_GRP_CUSTOM, port_index, "port_index:%u", port_index);
                continue;
            }




            // See table 3 in the user guide for understanding the conversion - we do not support all status values.
            switch (buf[2 + port]) {
            case 0x0 :
            case 0x1 :
                port_status[(int)port_index] = PD_ON;
                break;
            case 0x1F :
                port_status[(int)port_index] = PD_OVERLOAD;
                break;

            case 0x1B :
                port_status[(int)port_index] = NO_PD_DETECTED;
                break;


            case 0x20:
            case 0x3C:
                port_status[(int)port_index] = POWER_BUDGET_EXCEEDED;
                break;

            case 0x1A:
                port_status[port_index] = PD_OFF;
                break;

            case 0x6 :
            case 0x7 :
            case 0x8 :
            case 0xC :
            case 0x11 :
            case 0x12 :
            case 0x1C :
            case 0x1D :
            case 0x1E :
            case 0x21 :
            case 0x24 :
            case 0x25 :
            case 0x26  :
            case 0x2B:
            case 0x2C:
            case 0x2D:
            case 0x2E:
            case 0x2F:
            case 0x30:
            case 0x31:
            case 0x32:
            case 0x33:
            case 0x34:
            case 0x35:
            case 0x36:
            case 0x37:
            case 0x38:
            case 0x39:
            case 0x3A:
            case 0x3D:
            case 0x3E:
            case 0x3F:
            case 0x40:
            case 0x41:
            case 0x42  :
                port_status[port_index] = UNKNOWN_STATE;
                break;


            default :
                // This shall never happen all states should be covered.
                T_WG(VTSS_TRACE_GRP_CUSTOM, "Unknown port status: 0x%X", buf[2 + port] );
                port_status[port_index] = PD_OFF;
                break;

            }
        }

    } else {
        T_WG(VTSS_TRACE_GRP_CUSTOM, "Data did not contain valid data.");
    };

}

//  Getting status for a single port
//  In : Port_index - The port for which to get status (starting from 0)
//  Out : poe_status - pointer to where to put the status for the port
void pd63000_port_status_get (vtss_port_no_t port_index, poe_port_status_t *port_status)
{
    // See Section 4.7.10 in user guide

    poe_port_status_t port_grp_status[95]; // Upto 95 ports supported
    int grp_num;
    T_DG(VTSS_TRACE_GRP_CUSTOM, "pd63000_get_all_ports_status");


    if (port_index <= 10) {
        grp_num = 0;
    } else if  (port_index <= 21) {
        grp_num = 1;
    } else if  (port_index <= 25) {
        grp_num = 2;
    } else if  (port_index <= 36) {
        grp_num = 3;
    } else if  (port_index <= 47) {
        grp_num = 4;
    } else if  (port_index <= 58) {
        grp_num = 5;
    } else if  (port_index <= 69) {
        grp_num = 6;
    } else if  (port_index <= 80) {
        grp_num = 7;
    } else if  (port_index <= 91) {
        grp_num = 8;
    } else if  (port_index <= 95) {
        grp_num = 9;
    } else {
        T_E("Not supported port number:%u", port_index);
        return;
    }

    if (port_index < VTSS_PORTS) {
        pd63000_grp_status_get(port_grp_status, grp_num);
        *port_status = port_grp_status[port_index];
    }
}


//  Get all ports status - Section 4.7.10
// Return : poe_status - The status for all PoE ports
void pd63000_get_all_ports_status (poe_port_status_t *port_status)
{
    int grp_num;
    T_DG(VTSS_TRACE_GRP_CUSTOM, "pd63000_get_all_ports_status");

    for (grp_num = 0 ; grp_num < 10; grp_num++) {
        pd63000_grp_status_get(port_status, grp_num);
    }
}

//
// Function that returns 1 is a PoE PD63000 chip set is found, else 0.
// The chip is detected by reading the sw version. If the I2C access fails
// we say that the pd63000 chip isn't available
int pd63000_is_chip_available(void)
{
    T_RG(VTSS_TRACE_GRP_CUSTOM, "entering pd63000_is_chip_available");
    // We only want to try to detect the si3452 chipset once.
    if (pd63000_chip_found == DETECTION_NEEDED) {

//        (void) pd63000_reset_command();

        pd63000_chip_found = NOT_FOUND;

        if (check_startup_response()) {
            T_WG(VTSS_TRACE_GRP_CUSTOM, "Chip found by check_startup_response");
            pd63000_chip_found = FOUND;
        } else if (pd63000_get_system_status()) {
            pd63000_chip_found = FOUND;
            T_WG(VTSS_TRACE_GRP_CUSTOM, "Chip found by Calling pd63000_get_system_status");
        }


        if (pd63000_chip_found == FOUND) {
            // The PD69100 and PD63000 are going to support i2c interrupt in the future according to mail from mcirosemi. For now we don't support it.
            cyg_flag_init(&poe_interrupt_wait_flag);

            // Enable interrupt
            (void) vtss_interrupt_source_hook_set(i2c_rd_data_rdy_interrupt_function,
                                                  INTERRUPT_SOURCE_I2C,
                                                  INTERRUPT_PRIORITY_POE);

            pd63000_interrupt_enable(); // Tell pd63000 chip to give interrupt when data is ready to be read, else we have to wait 400 ms for every read.



            pd63000_10sec_reset_enable(); // Setup the chipset to reset the i2c circuit in the poe chipset if there have 10 sec with no access
        }
    }

    // Return the result.
    if ((pd63000_chip_found == NOT_FOUND)) {
        T_RG(VTSS_TRACE_GRP_CUSTOM, "existing pd63000_is_chip_available - Not Available, pd63000_chip_found = %d", pd63000_chip_found);
        return 0;
    } else {
        T_RG(VTSS_TRACE_GRP_CUSTOM, "existing pd63000_is_chip_available - Available");
        return 1;
    }
}




// Function for writing data from the MicroSemi micro-controller.
// IN  : Data - Pointer to data to write
//     : Size - Number of bytes to write.
int pd63000_wr(uchar *data, char size)
{
    poe_custom_entry_t poe_hw_conf;
    poe_hw_conf = poe_custom_get_hw_config(0, &poe_hw_conf);
    if (vtss_i2c_wr(NULL, poe_hw_conf.i2c_addr[PD63000], data, size, 100, NO_I2C_MULTIPLEXER) != VTSS_RC_OK) {
        return 0;
    } else {
        return size;
    }
}


// Function for reading data from the MicroSemi micro-controller.
// IN/OUT : Data - Pointer to where to put the read data
// IN     : Size - Number of bytes to read.
vtss_rc pd63000_rd(uchar *data, char size)
{
    vtss_rc rc;
    wait_for_i2c_data();
    poe_custom_entry_t poe_hw_conf;
    poe_hw_conf = poe_custom_get_hw_config(0, &poe_hw_conf);
    rc = vtss_i2c_rd(NULL, poe_hw_conf.i2c_addr[PD63000], data, size, 100, NO_I2C_MULTIPLEXER);

    if (rc == VTSS_RC_ERROR) {
        T_W("I2C read problem");
    }
    return rc;
}

// Things that is needed after a boot
void pd63000_poe_init(void)
{
    pd63000_set_system_masks(0x6); // Turn off lowest priority port, when a higher priority has a PD connected,see section 4.5.10

}




//***************************************************************************************************************
//  Code below is used to update the PoE chipset firmware. It is based upon microSemi TN-140_06-0024-081 document
// **************************************************************************************************************
#ifdef FIRMWARE_UPDATE_SUPPORTED


// Dummy write function for that can be used for debugging
static int pd63000_wr_f(uchar *data, char size)
{
    T_RG(VTSS_TRACE_GRP_CUSTOM, "Write %d , size:%d", *data, size);
    (void) pd63000_wr(data, size);
    return size;
}

// Dummy read function for that can be used for debugging
static void pd63000_rd_f(uchar *data, char size)
{
    (void) pd63000_rd(data, size);
    return;
}


// Function for doing the firmware update
void DownloadFirmwareFunc (u8 *microsemi_firmware, u32 firmware_size)
{
    u32 byte_cnt = 0;
    u8 progress = 0;
    u8 prev_progress = 255;
    u8 *byteArr;
    u8 download[15] = { 0x01, 0x01, 0xFF, 0x99, 0x15, 0x16, 0x16, 0x99, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x74 };
    u8 Temp[1] = {0};
    u16 i;
    u8 enter[] = { (u8)'E', (u8)'N', (u8)'T', (u8)'R' };

    pd63000_chip_found  =  NOT_FOUND; // Signal to PoE code to stop access to the PoE chipset.

    // Get system status
    u8 byte_Arr[15] = {REQUEST_KEY, DUMMY_SEQ_NUM, 0x07, 0x3D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    if (!is_i2c_tx_ok(&byte_Arr[0])) {
        T_WG(VTSS_TRACE_GRP_CUSTOM, "Didn't get system status");
        return;
    }

    // Start download
    if (!is_i2c_tx_ok(&download[0])) {
        T_EG(VTSS_TRACE_GRP_CUSTOM, "download problem");
        return;
    }

    printf("Starting firmware update \n");

    VTSS_MSLEEP(100); // wait 100ms

    if (!report_key_ok(byte_Arr[0])) {
        print_buffer(&byte_Arr[0], PD_BUFFER_SIZE, __LINE__);
    };


    // We do always update
    for (i = 0; i < 4; i++) {
        T_IG(VTSS_TRACE_GRP_CUSTOM, "Sending ENTR");
        Temp[0] = enter[i];
        (void) pd63000_wr_f( (uchar *) &Temp[0], 1);
    }

    VTSS_MSLEEP(100); // wait 100ms
    for (i = 0 ; i < 5 ; i ++ ) {
        // I2C_AsyncClient.Read(ref byte_Arr, 1); // read TPE\r\n
        pd63000_rd_f(&byte_Arr[0], 1);
    }

    byte_Arr[0] = ((u8)'E'); // Sending 'E' - erasing memory
    (void) pd63000_wr_f( (uchar *) &byte_Arr[0], 1);
    VTSS_MSLEEP(100);

    for (i = 0; i < 5; i++) {
        pd63000_rd_f(&byte_Arr[0], 1);// read TOE\r\n
    }

    VTSS_MSLEEP(5000); // wait 5s
    for (i = 0; i < 4; i++) {
        pd63000_rd_f(&byte_Arr[0], 1); // read TE\r\n
    }

    VTSS_MSLEEP(100); // wait 100ms
    for (i = 0; i < 5; i++) {
        pd63000_rd_f(&byte_Arr[0], 1); // read TPE\r\n
    }

    byte_Arr[0] = ((u8)'P'); // Sending 'P' - program memory
    (void) pd63000_wr_f(&byte_Arr[0], 1);
    VTSS_MSLEEP(100);    // wait 100ms

    for (i = 0; i < 5; i++) {
        pd63000_rd_f(&byte_Arr[0], 1);
    }


    T_NG(VTSS_TRACE_GRP_CUSTOM, "firmware:%d,  byte_cnt:%u firmware_size:%u", *microsemi_firmware, byte_cnt, firmware_size);
    // If greater than 127 then we have reached end of file
    while ((byte_cnt <= firmware_size) && (*microsemi_firmware <= 127)) {
        byteArr =  microsemi_firmware;

        // Print out progress
        progress = (byte_cnt * 100) / firmware_size;
        if (progress != prev_progress) {
            printf("Firmware update progress:%d %% \n", progress);
            prev_progress = progress;
        }

        if (byteArr[0] == 10) {
            T_IG(VTSS_TRACE_GRP_CUSTOM, "Skipping LF (Line Feed)");
            microsemi_firmware++;
            byteArr =  microsemi_firmware;
        }

        if (byteArr[0] != 'S') {
            T_E("Line should start with S byteArr[0]:0x%X", byteArr[0]);
            break;
        }

        T_IG(VTSS_TRACE_GRP_CUSTOM, "byteArr[0]:0x%X, byteArr[1]:0x%X, byte_cnt:%u, firmware_size:%u", byteArr[0], byteArr[1], byte_cnt, firmware_size);

        if (byteArr[1] == 0x30) { //skip line that starts with "S0"
            // If greater than 127 then we have reached end of file
            while (*microsemi_firmware != '\n' && *microsemi_firmware < 127 && byte_cnt <= firmware_size) {
                byte_cnt++;
                T_NG(VTSS_TRACE_GRP_CUSTOM, "Skipping S0 lines microsemi_firmware:0x%X, byte_cnt:%u", *microsemi_firmware, byte_cnt);
                microsemi_firmware++;
            }
            // Lines Starting with S3 or S7
        } else if (byteArr[1] == 0x33 || byteArr[1] == 0x37) {
            // If greater than 127 then we have reached end of file
            while (*microsemi_firmware != '\n' && *microsemi_firmware < 127 && byte_cnt <= firmware_size) {
                Temp[0] = *microsemi_firmware;
                (void) pd63000_wr_f(&Temp[0], 1);
                microsemi_firmware++;
                byte_cnt++;
            }
            microsemi_firmware++;
            byte_cnt++;

            T_IG(VTSS_TRACE_GRP_CUSTOM, "Writing new line - byte_cnt:%u", byte_cnt);
            byteArr[0] = (u8)'\r';
            (void) pd63000_wr_f((uchar *)&byteArr[0], 1);

            byteArr[1] = (u8)'\n';
            (void) pd63000_wr_f((uchar *)&byteArr[0], 1);
            VTSS_MSLEEP(100); // wait for 100ms

            for (i = 0; i < 4; i++) {
                pd63000_rd_f(&Temp[0], 1); // reading T*\r\n or TP\r\n
                byteArr[i] = Temp[0];
            }

            if (byteArr[0] == 0x54 && byteArr[1] == 0x2a) { // if read T*\r\n
                T_IG(VTSS_TRACE_GRP_CUSTOM, "NOT END of File");
            } else if (byteArr[0] == 0x54 && byteArr[1] == 0x50) { // if read TP\r\n
                T_IG(VTSS_TRACE_GRP_CUSTOM, "END of File");
                break;//sLine = null; //end of file
            }

        } else {
            // Unknown Sx number
            T_E("Unknown start value S:%d", byteArr[1]);
            break;
        }
    }
    VTSS_MSLEEP(10000);
    printf("Firmware update done - Please power cycle the board \n");
}

#endif
/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
