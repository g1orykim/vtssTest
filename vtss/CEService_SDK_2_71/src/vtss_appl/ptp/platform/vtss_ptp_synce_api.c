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
/* vtss_ptp_synce_api.c */

#include "vtss_ptp_synce_api.h"
#include "ptp_api.h"           /* Our module API */
#ifndef VTSS_ARCH_SERVAL
#include "ptp.h"
#endif

#if defined(VTSS_ARCH_LUTON28) || defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_JAGUAR_1)

#if defined(VTSS_ARCH_LUTON28)
#include <cyg/io/i2c_vcoreii.h>
#define CYG_I2C_VCXO_DEVICE CYG_I2C_VCOREII_DEVICE

#endif

#if defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_JAGUAR_1)

#include <cyg/io/i2c_vcoreiii.h>
#define CYG_I2C_VCXO_DEVICE CYG_I2C_VCOREIII_DEVICE
#endif

#define SI570_I2C_ADDR 0x55
// Si 570 register address
#define SI570HS_DIV_N1 7
#define SI570HS_REF_FREQ 8
#define SI570HS_RESET_CONTROL 135

#define MAX_570_I2C_LEN 8



// Initial frequency reference in synce module
static i64 rfreq;
static u8 n1Low; // initial value of N1 bit 1:0, to be or'ed vith rfreq

// Do the i2c read from si570 on the synce module, the result is passed by pointer.
// The device read may fail, therefore read two times and compare the two results
// Returns 0 if device read fail (device may not be present)
// Returns no of byte read otherwise.
static int si570_device_rd(u8 reg_addr, u8 * data, char len)
{
    u8 buf[MAX_570_I2C_LEN];
    int read_fail = 1;
    int rc1, rc2;
    CYG_I2C_VCXO_DEVICE(si570_device,SI570_I2C_ADDR);
    T_DG(_C,"Read, reg_addr = %d, len = %d, device adr: %2x", reg_addr, len, si570_device.i2c_address);
    while (read_fail > 0 && read_fail < 3) {
        (void)cyg_i2c_tx(&si570_device,&reg_addr,1);
        rc1 = cyg_i2c_rx(&si570_device,data,len);
        (void)cyg_i2c_tx(&si570_device,&reg_addr,1);
        rc2 = cyg_i2c_rx(&si570_device,buf,len);
        if (rc1 != len || rc2 != len || memcmp(data, buf, len)) {
            T_IG(_C,"Read error %d, read len = %d", read_fail, len);
            T_IG(_C,"data = %2x, %2x, %2x, %2x, %2x, %2x, %2x, ", data[0], data[1], data[2], data[3], data[4], data[5], data[6]);
            T_IG(_C,"buf = %2x, %2x, %2x, %2x, %2x, %2x, %2x, ", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6]);
            read_fail++;
        } else
            read_fail = 0;
    }
    T_DG(_C,"read_fail = %d", read_fail);
    T_DG(_C,"Read, buf = %2x, %2x, %2x, %2x, %2x, %2x, %2x, ", data[0], data[1], data[2], data[3], data[4], data[5], data[6]);
    if (read_fail == 0) return len;
    else return 0;
}

// Do the i2c write to si570 on the synce module.
// The device write operation may fail, therefore read back and compare result
static void si570_device_wr(u8 reg_addr, u8* data, char len)
{
    CYG_I2C_VCXO_DEVICE(si570_device,SI570_I2C_ADDR);
    T_DG(_C,"Write, reg_addr = %d, len = %d, device adr: %2x", reg_addr, len, si570_device.i2c_address);
    u8 buf[MAX_570_I2C_LEN];
    int write_fail = 1;
    while (write_fail > 0 && write_fail < 3) {
        buf[0] = reg_addr;
        memcpy(&buf[1], data,len >=MAX_570_I2C_LEN ? MAX_570_I2C_LEN-1 : len);
        (void)cyg_i2c_tx(&si570_device,&buf[0],len+1);

        write_fail = 0;
    }
    T_DG(_C,"Write, buf = %2x, %2x, %2x, %2x, %2x, %2x, %2x, ", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6]);
}

// Function that returns 1 is a VCXO chip is found, else 0.
static int si570_is_chip_available(void)
{
    static BOOL si570_chip_found  = FALSE;
    CYG_I2C_VCXO_DEVICE(si570_device,SI570_I2C_ADDR);
    u8 si570reg [1] = {0};
    u8 si570reg_adr = SI570HS_DIV_N1;
    // We only want to try to detect the si570 chip once.
    if (si570_chip_found == FALSE) {
        T_DG(_C,"Checking chip availability");
        if (0 != cyg_i2c_tx(&si570_device,&si570reg_adr,1) && (0 != cyg_i2c_rx(&si570_device,si570reg,sizeof(si570reg)))) {
            si570_chip_found = TRUE;
        }
    }
    if (si570_chip_found) {
        T_DG(_C,"Si570 chip found, reg = %x", si570reg[0]);
        return 1;
    } else {
        T_DG(_C,"Si570 chip NOT found, reg = %x", si570reg[0]);
        return 0;
    }
}
#endif

void vtss_synce_clock_set_adjtimer(Integer32 adj)
{
#if defined(VTSS_ARCH_LUTON28) || defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_JAGUAR_1)
//    u64 rfreqnew = rfreq*(10000000000LL+adj)/10000000000LL;
    i64 rfreqnew =  rfreq + (rfreq*adj)/10000000000LL;
    u8 si570reg [5];
    int i;
    // write new rfreq to chip
    T_IG(_C,"rfreq = %lld, rfreqnew = %lld", rfreq, rfreqnew);

    for (i = 0; i < 5; i++) {
        si570reg[4-i] = rfreqnew & 0xff;
        rfreqnew = (rfreqnew>>8);
    }
    si570reg[0] |= n1Low;
    si570_device_wr(SI570HS_REF_FREQ, si570reg, sizeof(si570reg));
#endif
}

/*
 * The internal initial clock frequency reference is read from HW and saved
 * for later use.
 * If the system is warm booted, this frequency reference may be wrong. This
 * must be corrected when the warm reset feature is implemented.
 * Tried to always reset the XO to default, but this caused some clock
 * disturbance, therefore it is commented out.
 */
int vtss_synce_clock_init_adjtimer(init_synce_t *init_synce, BOOL cold)
{
    int rc = 0;
#if defined(VTSS_ARCH_LUTON28) || defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_JAGUAR_1)
    u8 hs_div;
    const u8 HS_DIV [] = {4,5,6,7,0,9,0,11};
    u8 n1;
    int i;
    u8 si570reg [MAX_570_I2C_LEN];
    u8 si570resetreg = 0x01;
    u8 retrycnt = 0;
    // Read the default setup parameters
    // These values are not used. Until we support Warm reset, we always recalls
    // the default configuration upon sw reset
    //rc = si570_device_rd(SI570HS_DIV_N1, si570reg, sizeof(si570reg));
    //T_IG(_C,"1. Init data = %2x, %2x, %2x, %2x, %2x, %2x", si570reg[0], si570reg[1], si570reg[2], si570reg[3], si570reg[4], si570reg[5]);
    //if (rc) {
    //  // reset device to default (RECALL NVM into RAM)
    //  si570_device_wr(SI570HS_RESET_CONTROL, &si570resetreg, sizeof(si570resetreg));
    //  VTSS_NSLEEP(100000); /* wait until device is ready to continue */
    //  while (si570resetreg && retrycnt++ < 5)
    //      si570_device_rd(SI570HS_RESET_CONTROL, &si570resetreg, sizeof(si570resetreg));
    //  if (0 == si570resetreg) {
    if (si570_is_chip_available()) {
        if (!cold) {
            if (init_synce->magic_word == SYNCE_CONF_MAGIC_WORD) {
                rfreq = init_synce->init_rfreq;
                n1Low = init_synce->n1Low;
                T_IG(_C,"Synce warm boot");
                rc = 1;
            } else { // warm boot but no valid frequence stored in DB, therefore reset the SyncE module
                // reset device to default (RECALL NVM into RAM)
                si570_device_wr(SI570HS_RESET_CONTROL, &si570resetreg, sizeof(si570resetreg));
                VTSS_NSLEEP(100000); /* wait until device is ready to continue */
                while (si570resetreg && retrycnt++ < 5)
                    rc = si570_device_rd(SI570HS_RESET_CONTROL, &si570resetreg, sizeof(si570resetreg));
                if (0 == si570resetreg) cold = TRUE;
                T_WG(_C,"Synce: reset, in warm boot, because not valid rfreq was present");
            }
        }
        if (cold) {
            init_synce->magic_word = 0;
            rc = si570_device_rd(SI570HS_DIV_N1, si570reg, sizeof(si570reg));
            T_IG(_C,"2. Init data = %2x, %2x, %2x, %2x, %2x, %2x", si570reg[0], si570reg[1], si570reg[2], si570reg[3], si570reg[4], si570reg[5]);
            if (rc) {
                hs_div = HS_DIV[(si570reg[0]>>5) & 0x7];
                n1 = ((si570reg[0] & 0x1F)<<2) + ((si570reg[1]>>6) & 0x3) + 1;
                n1Low = si570reg[1] & 0xc0;
                init_synce->n1Low = n1Low;
                rfreq = 0;
                for (i = 0; i < 5; i++)
                    rfreq = (rfreq<<8) + si570reg[i+1];
                rfreq &= 0x3fffffffffULL; // mask N1 bits
                init_synce->init_rfreq = rfreq;
                init_synce->magic_word = SYNCE_CONF_MAGIC_WORD;
                T_IG(_C,"Synce cold boot");
                T_IG(_C,"init values, hs_div = %d, n1= %d, rfreq = %lld", hs_div, n1, rfreq);
                T_IG(_C,"i.e fxtal = %lld", ((114285000LL*hs_div*n1)<<28)/rfreq);
            } else {
                T_IG(_C,"Synce not present");
            }
        }
    }
    T_IG(_C,"return rc = %d", rc);
#endif
    return rc;
}

