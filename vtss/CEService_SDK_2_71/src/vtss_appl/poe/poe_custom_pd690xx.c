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
/****************************************************************************/
// PoE chip PD69xxx from MicroSimi.
/****************************************************************************/

#include "critd_api.h"
#include "poe.h"
#include "misc_api.h"
#include "poe_custom_api.h"
#include "poe_custom_pd690xx_api.h"



// Subset of the PD690xx register map. The datasheet used is rev. 1.2
#define VMAIN_REG_ADDR 0x105C
#define CFGC_ICVER_ADRR 0x031A
#define PORT_STATUS_BASE_REG_ADDR 0x11AA
#define PORT_PPL_BASE_REG_ADDR 0x1334
#define PORT_CLASS_BASE_REG_ADDR 0x11C2
#define PORT_POWER_CONSUM_BASE_REG_ADDR 0x12B4
#define PORT_CONF_BASE_REG_ADDR 0x131A
#define SYSTEM_CONF_AND_CTRL  0x1160
#define DIS_PORT_CMD  0x1332
#define SW_CONFIG 0x139E
#define I2C_EXT_SYNC_TYPE 0x1318
#define EXT_EV_IRQ 0x1144


static BOOL OperateEEprom(void);
static u8 i2c_wait = 100;


static poe_chip_found_t chip_found[VTSS_PORTS]; // Variable used to determine if chip auto detection is needed.

// Does the i2c write.
//
// In : iport    : Port starting from 0.
//      reg_addr : 16 bits register address
//      data     : Data to be written
//
// Return : None.
void pd690xx_device_wr(vtss_port_no_t iport, u16 reg_addr, u16 data)
{
    u8 retry_cnt; // Number of retries in case that the access fails
    u8 buf[4];

    // Get PoE hardware board configuration
    poe_custom_entry_t poe_hw_conf;
    poe_hw_conf = poe_custom_get_hw_config(iport, &poe_hw_conf);

    if (poe_hw_conf.available) {
        for (retry_cnt = 1; retry_cnt <= I2C_RETRIES_MAX; retry_cnt++) {
            T_RG_PORT(VTSS_TRACE_GRP_CUSTOM, iport, "addr = 0x%X, data = 0x%X", reg_addr, data);
            buf[0] = (reg_addr >> 8) & 0xFF; // Address MSB
            buf[1] = reg_addr & 0xFF;        // Address LSB
            buf[2] = (data >> 8) & 0xFF;     // Data MSB
            buf[3] = data & 0xFF;            // Data LSB

            // If 4 bytes weren't transmitted we need to do re-detection of the PoE chipset
            if (vtss_i2c_wr(NULL, poe_hw_conf.i2c_addr[PD690xx], &buf[0], 4, i2c_wait, NO_I2C_MULTIPLEXER) != VTSS_RC_OK) {
                if (retry_cnt == I2C_RETRIES_MAX) {
                    T_WG(VTSS_TRACE_GRP_CUSTOM, "I2C TX problem, Re-detecting PoE Chip set");
                    chip_found[iport] = DETECTION_NEEDED;
                } else {
                    T_IG_PORT(VTSS_TRACE_GRP_CUSTOM, iport, "TX retry cnt = %d", retry_cnt);
                }
            } else {
                break; // OK - Access went well. No need to re-send.
            }
        }
    }
}





// Does the i2c read
//
// In : iport    : Port starting from 0.
//      reg_addr : 16 bits register address
//
// In/Out : data : Pointer Data being read.
// Return : None.
void pd690xx_device_rd(vtss_port_no_t iport, u16 reg_addr, u16 *data)
{
    u8 retry_cnt; // Number of retries in case that the access fails
    u8 buf[2];
    u8 rx_buf[2];


    // Clear buffer;
    memset(&rx_buf[0], 0, sizeof(rx_buf));

    // Get PoE hardware board configuration
    poe_custom_entry_t poe_hw_conf;
    poe_hw_conf = poe_custom_get_hw_config(iport, &poe_hw_conf);

    if (poe_hw_conf.available) {
        for (retry_cnt = 1; retry_cnt <= I2C_RETRIES_MAX; retry_cnt++) {

            buf[0] = (reg_addr >> 8) & 0xFF; // Address MSB
            buf[1] = reg_addr & 0xFF;        // Address LSB

            T_RG_PORT(VTSS_TRACE_GRP_CUSTOM, iport,
                      "Addr = 0x%X, reg_addr = 0x%X, buf[0] = 0x%X, buf[1]=0x%X",
                      poe_hw_conf.i2c_addr[PD690xx], reg_addr, buf[0], buf[1]);



            if (vtss_i2c_wr_rd(NULL, poe_hw_conf.i2c_addr[PD690xx], &buf[0], 2, &rx_buf[0], 2, i2c_wait, NO_I2C_MULTIPLEXER) == VTSS_RC_OK) {
                break; // OK - Data was read correctly
            } else {
                if (retry_cnt == I2C_RETRIES_MAX) {
                    chip_found[iport] = DETECTION_NEEDED;
                    T_WG_PORT(VTSS_TRACE_GRP_CUSTOM, iport, "I2C RX register address problem, Re-detecting PoE Chip set");
                    break;
                } else {
                    T_IG_PORT(VTSS_TRACE_GRP_CUSTOM, iport, "RX reg_addr: 0x%X retry cnt = %d", reg_addr, retry_cnt);
                    continue; // Access failed - retry.
                }

            }
        }

        // Convert the array to a 16 bits value (which is returned)
        *data = (rx_buf[0] << 8) + rx_buf[1];

        T_RG_PORT(VTSS_TRACE_GRP_CUSTOM, iport,
                  "data = 0x%X", *data);
    }
}


// Does the i2c write of the bit set in the mask vector.
//
// In : iport    : Port starting from 0.
//      reg_addr : 16 bits register address
//      data     : Data to be written
//      mask     : Mask vector defining the bits to be changed.
//
// Return : None.
static void pd690xx_device_wr_mask(vtss_port_no_t iport, u16 reg_addr, u16 data, u16 mask)
{
    u16 reg_val;

    pd690xx_device_rd(iport, reg_addr, &reg_val); // Get current register value.
    reg_val &= ~mask; // Clear the bits that shall be changed.
    reg_val |= (data & mask); // Set the bits that shall be changed.

    pd690xx_device_wr(iport, reg_addr, reg_val); // Write back to register.

    T_RG_PORT(VTSS_TRACE_GRP_CUSTOM, iport, "reg_val  = 0x%X", reg_val);
}

// Returns the Main voltage in mV
//
// In : iport    : Port starting from 0.
//
// Return : Main power supply's voltage in mV
static u16 pd690xx_get_vee(vtss_port_no_t iport)
{
    T_RG_PORT(VTSS_TRACE_GRP_CUSTOM, iport, "Entering get_vee");
    u16 vmain;

    // Read the vee.
    pd690xx_device_rd(iport, VMAIN_REG_ADDR, &vmain);

    return vmain * 61; // 1 LSB bit = 61 mV. See datasheet
}

// Get status from the status register
//
// In/out : port_status ->Pointer to port_status to be returned.
// In     : iport : port number that shall be checked.
void pd690xx_new_status_get(poe_port_status_t *port_status, vtss_port_no_t iport)
{
    u16 port_status_reg;

    // Register width is 16bits -> Read every 2nd address
    pd690xx_device_rd(iport, PORT_STATUS_BASE_REG_ADDR + (iport % PD690xx_PORTS_PER_POE_CHIP) * 2, &port_status_reg); // Get the port status


    T_DG_PORT(VTSS_TRACE_GRP_CUSTOM, iport, "port_status_reg = 0x%X", port_status_reg);

    // See datasheet for the case values
    switch (port_status_reg & 0xFF) {
    case 0:
        port_status[iport] = PD_ON;
        break;

    case 2:
    case 4:
    case 5: // From MicroSemi -  When you connect with non-poe device, the device may with impedance
    // not in IEEE802.3 range. PD690xx will keep detect the device, So Port Status register will
    // keep show "invalid signature". This is normal. -- Bugzilla#5118.

    case 8:
        port_status[iport] = NO_PD_DETECTED;
        break;

    case 9:
        port_status[iport] = POE_DISABLED;
        break;

    case 18:
        port_status[iport] = PD_OFF;
        break;

    default:
        port_status[iport] = UNKNOWN_STATE;
        break;
    }
}



// Get status for all ports (If PD is turned on etc.)
//
// In/out : Pointer to port_status to be returned.
//
void pd690xx_port_status_get(poe_port_status_t *port_status, vtss_port_no_t iport)
{
    poe_custom_entry_t poe_hw_conf;

    // Get PoE hardware board configuration
    poe_hw_conf = poe_custom_get_hw_config(iport, &poe_hw_conf);

    if (poe_hw_conf.available == false) {
        // PoE chip not mounted for this port
        port_status[iport] = POE_NOT_SUPPORTED;
    } else {
        pd690xx_new_status_get(port_status, iport);
    }
}


// Function for setting the max power for a port.
//
// In : iport          : Port starting from 0.
//      max_port_power : The maximum power that the port must deliver (Icut) - In DeciWatts.
void pd690xx_set_power_limit_channel(vtss_port_no_t iport, u16 max_port_power)
{
    // Register width is 16bits -> Access every 2nd address
    T_DG_PORT(VTSS_TRACE_GRP_CUSTOM, iport, "max_port_power = %d", max_port_power);
    pd690xx_device_wr(iport, PORT_PPL_BASE_REG_ADDR + (iport % PD690xx_PORTS_PER_POE_CHIP) * 2, max_port_power);
}



// Determine the class for each port. Passed back via pointer
//
// In/Out : Classes - Pointer to classes for all ports
void pd690xx_get_all_port_class(i8 *classes, vtss_port_no_t iport)
{
    u16 class_reg;

    // Register width is 16bits -> Access every 2nd address
    pd690xx_device_rd(iport, PORT_CLASS_BASE_REG_ADDR + (iport % PD690xx_PORTS_PER_POE_CHIP) * 2, &class_reg);

    // [3:0] = First Finger Class Result, See datasheet
    classes[iport] = class_reg & 0xF;


    T_NG_PORT(VTSS_TRACE_GRP_CUSTOM, iport, "classes[iport] = %d, class_reg = 0x%X", classes[iport], class_reg);

    // We do not use the class encoding ( See Table 2 in documentation). Convert Class 0 ( encoded as 6 )  to 0
    if (classes[iport] > 4) {
        classes[iport] = 0;
    }
}


// Updates the poe status with current (in mA)  and power consumption (in deciWatts)
//
// In/Out : poe_status - Pointer to poe_status for all ports
void pd690xx_get_port_measurement(poe_status_t *poe_status, vtss_port_no_t iport)
{

    T_NG(VTSS_TRACE_GRP_CUSTOM, "Entering get_port_measurement");
    u16 power_used;
    u16 current_used = 0;
    u16 vee;

    // Get the main power voltage
    vee = pd690xx_get_vee(iport) / 1000; // Divide by 1000 to convert from mV to V


    // Register width is 16bits -> Access every 2nd address - Get power used
    pd690xx_device_rd(iport, PORT_POWER_CONSUM_BASE_REG_ADDR + (iport % PD690xx_PORTS_PER_POE_CHIP) * 2, &power_used);


    if (power_used == 0) {
        // Avoid divide by zero
        current_used = 0;
    } else {
        current_used = power_used * 100 / vee; // OHM law : I=P/U - Multiply with 100 to convert between deciW/mA
    }


    // Update the status structure
    poe_status->power_used[iport]   = power_used;
    poe_status->current_used[iport] = current_used;
}


// Things that is needed after a reset
//
void pd690xx_poe_init(void)
{
    int port_num;
    if (OperateEEprom() == FALSE) {
        T_E("PoE micro code download failed");
    } else {
        T_D("PoE micro code download success");
    };

    for (port_num = 0 ; port_num < VTSS_PORTS; port_num++) {
        pd690xx_capacitor_detection_set(port_num, TRUE); // Lots of old PDs uses capacitor detection so we enables it.
    }

}


// Enables or disables poe for a port
//
// In : iport          : Port starting from 0.
//      enable         : Set to TRUE if PoE shall be enabled for the port in question else FALSE.
//      mode           : PoE mode (For setting AT/AF)
void pd690xx_poe_enable(vtss_port_no_t iport, BOOL enable, poe_mode_t mode)
{
    if (enable) {
        // Register width is 16bits -> Access every 2nd address
        pd690xx_device_wr_mask(iport, PORT_CONF_BASE_REG_ADDR + (iport % PD690xx_PORTS_PER_POE_CHIP) * 2, 1, 0x3);
    } else {
        pd690xx_device_wr_mask(iport, PORT_CONF_BASE_REG_ADDR + (iport % PD690xx_PORTS_PER_POE_CHIP) * 2, 0, 0x3);
    }


    // Set AT / AF mode
    if (mode == POE_MODE_POE) {
        pd690xx_device_wr_mask(iport, PORT_CONF_BASE_REG_ADDR + (iport % PD690xx_PORTS_PER_POE_CHIP) * 2, 0x0, 0x30);
    } else {
        pd690xx_device_wr_mask(iport, PORT_CONF_BASE_REG_ADDR + (iport % PD690xx_PORTS_PER_POE_CHIP) * 2, 0x10, 0x30);
    }
}




// Function that returns 1 is a PoE chip set is found, else 0.
//
// In : None
//
// Return : 1 if PoE chip found, else 0
u16 pd690xx_is_chip_available(vtss_port_no_t iport)
{
    u16 hw_rev  = 0;

    static BOOL init_done = FALSE;
    int i;

    // Initialize chip found array.
    if (!init_done) {
        for (i = 0 ; i < VTSS_FRONT_PORT_COUNT; i++) {
            chip_found[i] = DETECTION_NEEDED;
        }
        init_done = TRUE;
    }

    // We only want to try to detect the PD690xx chipset once.
    if (chip_found[iport] == DETECTION_NEEDED) {
        i2c_wait = 5; // Set timeout low during detection in order to fast detection.
        // Read the Device Version Control Register
        // At the moment We simply only check port 0
        pd690xx_device_rd(iport, CFGC_ICVER_ADRR, &hw_rev);

        // For now only check the SW Rom and digital version. See Device Version Control register description.
        if ((hw_rev & 0x00FF) == 0x22 ) {
            chip_found[iport] = FOUND;
            T_IG_PORT(VTSS_TRACE_GRP_CUSTOM, iport, "PoE Pd690xx chipset found, hw_rev = 0x%X", hw_rev);
        } else {
            chip_found[iport] = NOT_FOUND;
            T_IG_PORT(VTSS_TRACE_GRP_CUSTOM, iport, "PoE Pd690xx chipset NOT found, hw_rev = 0x%X", hw_rev);
        }
        i2c_wait = 100; // Set timeout "high" for normal accesses
    }

    if (chip_found[iport] == FOUND) {

        return 1;
    } else {
        return 0;
    }
}



// Function for setting the chip set on configuration mode ( See section 1.4 in PoE datasheet)
// In : iport          : Port starting from 0.
//      enable         : True to enable configuration mode
static void pd690xx_condig_mode(vtss_port_no_t iport, BOOL enable)
{
    if (enable) {
        pd690xx_device_wr(iport, DIS_PORT_CMD, 0x03FF);
        pd690xx_device_wr(iport, SW_CONFIG, 0xDC03);
        pd690xx_device_wr(iport, I2C_EXT_SYNC_TYPE, 0x0020);
        pd690xx_device_wr(iport, EXT_EV_IRQ, 0x0020);
    } else {
        pd690xx_device_wr(iport, SW_CONFIG, 0xDC00);
        pd690xx_device_wr(iport, I2C_EXT_SYNC_TYPE, 0x0020);
        pd690xx_device_wr(iport, EXT_EV_IRQ, 0x0020);
        pd690xx_device_wr(iport, DIS_PORT_CMD, 0x0000);
    }
}


// Function for setting the legacy capacitor detection mode
// In : iport          : Port starting from 0.
//      enable         : True - Enable legacy capacitor detection
void pd690xx_capacitor_detection_set(vtss_port_no_t iport, BOOL enable)
{
    u16 reg_val;

    pd690xx_condig_mode(iport, TRUE); // Need to go into configuration mode in order to change the SYSTEM_CONF_AND_CTRL register

    pd690xx_device_rd(iport, SYSTEM_CONF_AND_CTRL, &reg_val); // Get current register value.
    if (enable) {
        // Enable Cap detection
        pd690xx_device_wr(0, SYSTEM_CONF_AND_CTRL, reg_val & 0xFFFB);
    } else {
        // Disable Cap detection
        pd690xx_device_wr(iport, SYSTEM_CONF_AND_CTRL, reg_val | 0x4);
    }

    pd690xx_condig_mode(iport, FALSE); // Leave configuration mode
}

// Function for getting the legacy capacitor detection mode
// In : iport          : Port starting from 0.
BOOL pd690xx_capacitor_detection_get(vtss_port_no_t iport)
{
    u16 reg_val;

    pd690xx_device_rd(iport, SYSTEM_CONF_AND_CTRL, &reg_val); // Get current register value.

    // capacitor detection is bit 2 (See data sheet)
    if (reg_val & 0x4) {
        return FALSE;
    } else {
        return TRUE;
    }
}


// The Code below comes from MicroSemi.

/****************************************************************************************************
 *   31/5/2010 15:18:43
 *
 *   THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MICROSEMI COP.- ANALOG MIXED
 *   SIGNAL GROUP LTD. ("AMSG") AND IS SUBJECT TO A NON-DISCLOSURE AGREEMENT
 *
 *   NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT
 *   OF AMSG OR ANY THIRD PARTY. AMSG RESERVES THE RIGHT AT ITS SOLE DISCRETION
 *   TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO AMSG.
 *
 *   THIS CODE IS PROVIDED "AS IS". AMSG MAKES NO WARRANTIES, EXPRESSED, IMPLIEDOR
 *   OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.
 *
 *   THIS SOURCE CODE, MAY NOT BE DISCLOSED TO ANY THIRD PARTY OR USED IN ANY OTHER
 *   MANNER, AND KNOWLEDGE DERIVED THEREFROM OR CANNOT BE USED TO WRITE ANY PROGRAM
 *   OR CODE. USE IS PERMITTED ONLY PURSUANT TO WRITTEN AGREEMENT SIGNED BY AMSG.
 *
 *   KNOWLEDGE OF THIS FILE MAY UNDER NO CIRCUMSTANCES BE USED TO WRITE ANY PROGRAM.
 *
 ****************************************************************************************************/
#define word u16
#define byte u8

//constant definitions
#define EEPROM_ID 0x1910
#define BROADCAST_ADD 0x000E

//structure definitions
typedef struct {
    word ID;
    word StructVer;
    word DataVer;
    word NumRecords;
    word EnabledPatches;
    word DataStartAdd;
    byte Spare[32];
    word HeaderCS;
} t_EEpromHeader;

typedef struct {
    word ChipNo;
    word RamAdd;
    word Data;
} t_EEpromData;



const t_EEpromHeader EEpromHeader = {
    0x1910,               /* word ID             */
    0x0001,               /* word StructVer      */
    0x0006,               /* word DataVer        */
    0x0083,               /* word NumRecords     */
    0x001F,               /* word EnabledPatches */
    0x00,                 /* word DataStartAdd   */ // Was 0x0064
    {                     /* byte Spare[32]      */
        0x00FF,
        0x00FF,
        0x00FF,
        0x00FF,
        0x00FF,
        0x00FF,
        0x00FF,
        0x00FF,
        0x00FF,
        0x00FF,
        0x00FF,
        0x00FF,
        0x00FF,
        0x00FF,
        0x00FF,
        0x00FF,
    },
    0x10C2                /* word HeaderCS      */ // Was 0x1126
};

const t_EEpromData EEpromData[] = {
    {
        0x0000,    /* word ChipNo;        disable all ports */
        0x1332,    /* word RamAdd;        */
        0x0FFF     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        open protkey */
        0x031E,    /* word RamAdd;        */
        0x00AB     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        patch 0 - Rise time exit */
        0x0080,    /* word RamAdd;        */
        0xDB49     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        patch 0 - Rise time exit dest */
        0x0082,    /* word RamAdd;        */
        0x1CA0     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        patch 1 - class spike exit */
        0x0084,    /* word RamAdd;        */
        0xE89F     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        patch 1 - class spike exit dest */
        0x0086,    /* word RamAdd;        */
        0x1C00     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        patch 2 - cass threshold exit */
        0x0088,    /* word RamAdd;        */
        0xE8F5     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        patch 2 - class threshold exit dest */
        0x008A,    /* word RamAdd;        */
        0x1C30     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        patch 3 - PM threshold exit */
        0x008C,    /* word RamAdd;        */
        0xC897     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        patch 3 - PM threshold exit dest */
        0x008E,    /* word RamAdd;        */
        0x1C50     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        patch 4 - Static PM Startup exit */
        0x0090,    /* word RamAdd;        */
        0xDE03     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        patch 4 - Static PM Startup exit dest */
        0x0092,    /* word RamAdd;        */
        0x1C70     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        change UDL Thtreshold */
        0x1062,    /* word RamAdd;        */
        0x1D07     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        change Power Resolution Factor */
        0x129E,    /* word RamAdd;        */
        0x153C     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        set Pre1 */
        0x1172,    /* word RamAdd;        */
        0x03E8     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        set Pre2 */
        0x1174,    /* word RamAdd;        */
        0x00AF     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        set LD1 */
        0x1176,    /* word RamAdd;        */
        0x00AF     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        set LD2 */
        0x1178,    /* word RamAdd;        */
        0x0075     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        clear patch function reg */
        0x13AA,    /* word RamAdd;        */
        0x0000     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        close protkey */
        0x031E,    /* word RamAdd;        */
        0x0000     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        class spike code LDL R4,#128 */
        0x1C00,    /* word RamAdd;        */
        0xF480     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        ORH R4,#1 */
        0x1C02,    /* word RamAdd;        */
        0xAC01     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        LDW R3,(R4,#4) */
        0x1C04,    /* word RamAdd;        */
        0x4B84     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        OR R3,R3,R1 */
        0x1C06,    /* word RamAdd;        */
        0x1366     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        STW R3,(R4,#4) */
        0x1C08,    /* word RamAdd;        */
        0x5B84     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        LDL R3,#170 */
        0x1C0A,    /* word RamAdd;        */
        0xF3AA     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        ORH R3,#19 */
        0x1C0C,    /* word RamAdd;        */
        0xAB13     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        LDW R4,(R3,#0) */
        0x1C0E,    /* word RamAdd;        */
        0x4C60     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        ORL R4,#1 */
        0x1C10,    /* word RamAdd;        */
        0xA401     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        STW R4,(R3,#0) */
        0x1C12,    /* word RamAdd;        */
        0x5C60     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        LDL R4,#64 */
        0x1C14,    /* word RamAdd;        */
        0xF440     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        LDL R3,#160 */
        0x1C16,    /* word RamAdd;        */
        0xF3A0     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        ORH R3,#232 */
        0x1C18,    /* word RamAdd;        */
        0xABE8     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        JAL R3 */
        0x1C1A,    /* word RamAdd;        */
        0x03F6     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        class 0 upper threshold */
        0x1C1C,    /* word RamAdd;        */
        0x0017     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        class 1 upper threshold */
        0x1C1E,    /* word RamAdd;        */
        0x0031     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        class 2 upper threshold */
        0x1C20,    /* word RamAdd;        */
        0x0051     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        class 3 upper threshold */
        0x1C22,    /* word RamAdd;        */
        0x006D     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        class 4 upper threshold */
        0x1C24,    /* word RamAdd;        */
        0x00A0     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        class thresholds patch code - LDL R3,#170 */
        0x1C30,    /* word RamAdd;        */
        0xF3AA     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        ORH R3,#13 */
        0x1C32,    /* word RamAdd;        */
        0xAB13     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        LDW R2,(R3,#0) */
        0x1C34,    /* word RamAdd;        */
        0x4A60     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        ORL R2,#2 */
        0x1C36,    /* word RamAdd;        */
        0xA202     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        STW R2,(R3,#0) */
        0x1C38,    /* word RamAdd;        */
        0x5A60     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        LDL R3,#28 */
        0x1C3A,    /* word RamAdd;        */
        0xF31C     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        ORH R3, #28 */
        0x1C3C,    /* word RamAdd;        */
        0xAB1C     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        LDL R2, F8 */
        0x1C3E,    /* word RamAdd;        */
        0xF2F8     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        ORH R2, E8 */
        0x1C40,    /* word RamAdd;        */
        0xAAE8     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        JAL R2 */
        0x1C42,    /* word RamAdd;        */
        0x02F6     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        PM th change from 25% to 12.5% - LDL R3,#170 */
        0x1C50,    /* word RamAdd;        */
        0xF3AA     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        ORH R3,#19 */
        0x1C52,    /* word RamAdd;        */
        0xAB13     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        LDW R2,(R3,#0) */
        0x1C54,    /* word RamAdd;        */
        0x4A60     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        ORL R2,#4 */
        0x1C56,    /* word RamAdd;        */
        0xA204     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        STW R2,(R3,#0) */
        0x1C58,    /* word RamAdd;        */
        0x5A60     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        MOV R2,R4 */
        0x1C5A,    /* word RamAdd;        */
        0x1212     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        ASR R2,#3 */
        0x1C5C,    /* word RamAdd;        */
        0x0A39     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        LDL R3,9a */
        0x1C5E,    /* word RamAdd;        */
        0xF39A     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        ORH R3,c8 */
        0x1C60,    /* word RamAdd;        */
        0xABC8     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        JAL R3 */
        0x1C62,    /* word RamAdd;        */
        0x03F6     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        Static PM Patch code - LDL R3,#170 */
        0x1C70,    /* word RamAdd;        */
        0xF3AA     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        ORH R3,#19 */
        0x1C72,    /* word RamAdd;        */
        0xAB13     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        LDW R2,(R3,#0) */
        0x1C74,    /* word RamAdd;        */
        0x4A60     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        ORL R2,#8 */
        0x1C76,    /* word RamAdd;        */
        0xA208     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        STW R2,(R3,#0) */
        0x1C78,    /* word RamAdd;        */
        0x5A60     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        LDL R2,#172 */
        0x1C7A,    /* word RamAdd;        */
        0xF2AC     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        ORH R2,#18 */
        0x1C7C,    /* word RamAdd;        */
        0xAA12     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        LDW R3,(R2,#0) */
        0x1C7E,    /* word RamAdd;        */
        0x4B40     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        LDL R2, AA */
        0x1C80,    /* word RamAdd;        */
        0xF2AA     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        ORH R2, 12 */
        0x1C82,    /* word RamAdd;        */
        0xAA12     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        LDW R2, (R2,#0) */
        0x1C84,    /* word RamAdd;        */
        0x4A40     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        CMP R3,R2 */
        0x1C86,    /* word RamAdd;        */
        0x1868     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        BLS 1 */
        0x1C88,    /* word RamAdd;        */
        0x3201     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        MOV R3,R2 */
        0x1C8A,    /* word RamAdd;        */
        0x130A     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        LDL R2, #196 */
        0x1C8C,    /* word RamAdd;        */
        0xF2C4     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        LDL R1,#10 */
        0x1C8E,    /* word RamAdd;        */
        0xF10A     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        ORH R1,#222 */
        0x1C90,    /* word RamAdd;        */
        0xA9DE     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        JAL R1 */
        0x1C92,    /* word RamAdd;        */
        0x01F6     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        Rise Time patch code  - LDL R6,#170 */
        0x1CA0,    /* word RamAdd;        */
        0xF6AA     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        ORH R6,#19 */
        0x1CA2,    /* word RamAdd;        */
        0xAE13     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        LDW R2,(R6,#0) */
        0x1CA4,    /* word RamAdd;        */
        0x4AC0     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        ORL R2,#16 */
        0x1CA6,    /* word RamAdd;        */
        0xA210     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        STW R2,(R6,#0) */
        0x1CA8,    /* word RamAdd;        */
        0x5AC0     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        MOV R2,R1 */
        0x1CAA,    /* word RamAdd;        */
        0x1206     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        LDL R6,#170(SetIntOut Add) */
        0x1CAC,    /* word RamAdd;        */
        0xF6AA     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        ORH R6, #202 */
        0x1CAE,    /* word RamAdd;        */
        0xAECA     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        JAL R6 */
        0x1CB0,    /* word RamAdd;        */
        0x06F6     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        LDB R2,(R5,#5) */
        0x1CB2,    /* word RamAdd;        */
        0x42A5     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        MOV R3,R1 */
        0x1CB4,    /* word RamAdd;        */
        0x1306     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        LSL R3,R2 */
        0x1CB6,    /* word RamAdd;        */
        0x0B54     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        LDL R4,#128 */
        0x1CB8,    /* word RamAdd;        */
        0xF480     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        ORH R4,#1 */
        0x1CBA,    /* word RamAdd;        */
        0xAC01     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        STW R3,(R4,#4) */
        0x1CBC,    /* word RamAdd;        */
        0x5B84     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        COM R3 */
        0x1CBE,    /* word RamAdd;        */
        0x130F     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        LDL R4,#64 */
        0x1CC0,    /* word RamAdd;        */
        0xF440     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        ORH R4,#2 */
        0x1CC2,    /* word RamAdd;        */
        0xAC02     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        LDW R2,(R4,#6) */
        0x1CC4,    /* word RamAdd;        */
        0x4A86     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        AND R2,R2,R3 */
        0x1CC6,    /* word RamAdd;        */
        0x124C     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        STW R2,(R4,#6) */
        0x1CC8,    /* word RamAdd;        */
        0x5A86     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        LDL R2,(R5,#5) */
        0x1CCA,    /* word RamAdd;        */
        0x42A5     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        LSL R2,#1 */
        0x1CCC,    /* word RamAdd;        */
        0x0A1C     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        LDL R4,#74 */
        0x1CCE,    /* word RamAdd;        */
        0xF44A     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        ORH R4,#2 */
        0x1CD0,    /* word RamAdd;        */
        0xAC02     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        LDL R1,#8 */
        0x1CD2,    /* word RamAdd;        */
        0xF110     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        STW R1,(R4,R2) */
        0x1CD4,    /* word RamAdd;        */
        0x7988     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        LDL R2,#100 */
        0x1CD6,    /* word RamAdd;        */
        0xF264     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        NOP */
        0x1CD8,    /* word RamAdd;        */
        0x0100     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        SUBL R2,#1 */
        0x1CDA,    /* word RamAdd;        */
        0xC201     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        BNE -3 */
        0x1CDC,    /* word RamAdd;        */
        0x25FD     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        LDB R2,(R5,#5) */
        0x1CDE,    /* word RamAdd;        */
        0x42A5     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        LDL R1,#1 */
        0x1CE0,    /* word RamAdd;        */
        0xF101     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        LSL R1,R2 */
        0x1CE2,    /* word RamAdd;        */
        0x0954     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        LDL R4,#128 */
        0x1CE4,    /* word RamAdd;        */
        0xF480     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        ORH R4,#1 */
        0x1CE6,    /* word RamAdd;        */
        0xAC01     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        STW R1,(R4,#6) */
        0x1CE8,    /* word RamAdd;        */
        0x5986     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        LDL R2, #100 */
        0x1CEA,    /* word RamAdd;        */
        0xF24B     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        NOP */
        0x1CEC,    /* word RamAdd;        */
        0x0100     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        SUBL R2,#1 */
        0x1CEE,    /* word RamAdd;        */
        0xC201     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        BNE -3 */
        0x1CF0,    /* word RamAdd;        */
        0x25FD     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        LDB R2,(R5,#5) */
        0x1CF2,    /* word RamAdd;        */
        0x42A5     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        LSL R2,#1 */
        0x1CF4,    /* word RamAdd;        */
        0x0A1C     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        LDL R1,#1 */
        0x1CF6,    /* word RamAdd;        */
        0xF101     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        LDL R4,#74 */
        0x1CF8,    /* word RamAdd;        */
        0xF44A     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        ORH R4,#2 */
        0x1CFA,    /* word RamAdd;        */
        0xAC02     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        STW R1,(R4,R2) */
        0x1CFC,    /* word RamAdd;        */
        0x7988     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        LDB R2,(R5,#5) */
        0x1CFE,    /* word RamAdd;        */
        0x42A5     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        LDL R1,#1 */
        0x1D00,    /* word RamAdd;        */
        0xF101     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        LSL R1,R2 */
        0x1D02,    /* word RamAdd;        */
        0x0954     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        LDL R2,#106 */
        0x1D04,    /* word RamAdd;        */
        0xF26A     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        ORH R2,#219 */
        0x1D06,    /* word RamAdd;        */
        0xAADB     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        JAL R2 */
        0x1D08,    /* word RamAdd;        */
        0x02F6     /* word Data;          */
    },
    {
        0x0000,    /* word ChipNo;        enable all ports */
        0x1332,    /* word RamAdd;        */
        0x0000     /* word Data;          */
    }
};


//function prototypes
void EepromRead(word *Dest, word EepromAdd, word NumWords);
void RotemWrite(word ChipAdd, word ChipRamAdd, word *Data, word NumWords);
//========================================================================
//Function : OperateEEprom
//Description : this function reads from EEprom, verifies the data and
// sends it to slaves
//Parameters : none
//Return Value : bool - TRUE if successfull
// FALSE if error
//Basic assumptions :
// 1.The EEprom can be substituted by a memory space.
// in such case the EepromRead function should be replaced by
// a read from the memory space.
// 2.The rotem chips are numbered in the EEprom data from 0
// to 7. master and 7 slaves in A mode or 8 slaves in E mode.
// when working via I2C add the base I2C address to the
// chip address written in the EEprom.
//========================================================================

static BOOL OperateEEprom(void)
{
    t_EEpromHeader Header;
    t_EEpromData Data;
    word CheckSum = 0;
    word CurrAdd;
    byte DataCsIndex;
    word Index;
    byte *bPtr;
//read the header from the EEprom
    Header = EEpromHeader;
//check the ID
    if (Header.ID != EEPROM_ID) {
        T_E("Header.ID error - Header.ID = %d, EEPROM_ID = %d", Header.ID, EEPROM_ID);
//invalid ID
        return FALSE;
    }
//get the address of the struct object
    bPtr = (byte *)&Header;
//check the check sum of the header
    for (Index = 0; Index < (sizeof(t_EEpromHeader) - 2); Index++) {
//calc checksum
        CheckSum += *bPtr++;
    }
//check the checksum
    if (CheckSum != Header.HeaderCS) {
//Checksum Error
        T_E("Checksum Error  - checksum = 0x%X, Header.HeaderCS = 0x%X", CheckSum, Header.HeaderCS);
        return FALSE;
    }
//get the data start address
    CurrAdd = Header.DataStartAdd;
//zero the checksum
    CheckSum = 0;
//read the data records and write them to the slaves
    for (Index = 0; Index < Header.NumRecords; Index++) {
//read a record
        Data = EEpromData[CurrAdd];
        T_D("chip = %d, addr = 0x%X, data = 0x%X, CurrAdd = %d", Data.ChipNo, Data.RamAdd, Data.Data, CurrAdd);
//calculate the checksum
        bPtr = (byte *)&Data;
        for (DataCsIndex = 0; DataCsIndex < sizeof(t_EEpromData); DataCsIndex++) {
            CheckSum += *bPtr++;
        }
//check if the chip address is broadcast
        if (Data.ChipNo == BROADCAST_ADD) {
//when working in I2C then loop over all chip addresses
//and write the data to all
        } else {
//write the data to the slaves
            //RotemWrite(Data.ChipNo, Data.RamAdd, (word)&Data.Data, 1);
//            poe_device_wr(Data.ChipNo, Data.RamAdd, (word)&Data.Data);
            pd690xx_device_wr(Data.ChipNo, Data.RamAdd, Data.Data);
            T_D("chip = %d, addr = 0x%X, data = 0x%X", Data.ChipNo, Data.RamAdd, Data.Data);
        }
//advance the current EEprom address
        CurrAdd += 1; // was - CurrAdd += sizeof(t_EEpromData);
    }
//read a the data CS to the CurrAdd var to save space
    CurrAdd = CheckSum;
//check if the checsum matches
    if (CheckSum != CurrAdd) {
//Data Checksum Error
        return FALSE;
    }
//return success
    return TRUE;
}

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
