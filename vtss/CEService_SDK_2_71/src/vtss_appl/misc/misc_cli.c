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
#include "vtss_api_if_api.h"
#include "cli.h"
#include "cli_api.h"
#include "mgmt_api.h"
#include "misc_cli.h"
#include "ser_gpio.h"
#include "critd_api.h"
#include "port_api.h"
#include "led_api.h"
#include "cli_trace_def.h"
#include "port_api.h"
#ifdef VTSS_ARCH_JAGUAR_1
#include <cyg/io/spi_vcoreiii.h>
#endif



#if defined(VTSS_SW_OPTION_BOARD)
#include "interrupt_api.h"
#endif
#if defined(VTSS_SW_OPTION_PHY)
#include "phy_api.h"
#endif

#if defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_SERVAL)
#define CLI_BLK_MAX  (1<<6)  /* Targets (8 bit) */
#define CLI_ADDR_MAX (1<<14) /* Addresses (14 bits) */
#endif /* VTSS_ARCH_LUTON26/SERVAL */

#if defined(VTSS_ARCH_JAGUAR_1)
#define CLI_BLK_MAX  (1<<8)  /* Targets (8 bit) */
#define CLI_ADDR_MAX (1<<18) /* Addresses (14/18 bits) */
#endif /* VTSS_ARCH_LUTON26 */

#define CLI_PHY_MAX  128  /* PHY register addresses */

#define MISC_INST misc_phy_inst_get()

#define MAX_SPI_DATA  50

static critd_t    crit; 
static int trace_grp_crit;
#define MISC_CLI_CRIT_ENTER() critd_enter(&crit, trace_grp_crit, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define MISC_CLI_CRIT_EXIT()  critd_exit( &crit, trace_grp_crit, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)

/* Register address bitfield, ignore Lint problem with unprotected access. */
/*lint -sem(cli_misc_addr_list_parse, thread_protected) */
/*lint -sem(misc_cli_cmd_debug_reg, thread_protected) */
/*lint -sem(misc_cli_req_default_set, thread_protected) */
/*lint -sem(misc_cli_cmd_phy_10g_mode, thread_protected) */
/*lint -sem(misc_cli_cmd_phy_10g_reset, thread_protected) */
/*lint -sem(misc_cli_cmd_phy_10g_status, thread_protected) */
/*lint -sem(misc_cli_cmd_phy_10g_loopback, thread_protected) */
/*lint -sem(misc_cli_cmd_phy_10g_gpio_role, thread_protected) */
/*lint -sem(misc_cli_cmd_phy_10g_gpio_read, thread_protected) */
/*lint -sem(misc_cli_cmd_phy_10g_gpio_write, thread_protected) */
/*lint -sem(misc_cli_cmd_phy_10g_power, thread_protected) */
/*lint -sem(misc_cli_cmd_phy_10g_failover, thread_protected) */
/*lint -sem(misc_cli_cmd_debug_phy_1588_mmd_read, thread_protected) */
/*lint -sem(misc_cli_cmd_debug_phy_1588_mmd_write, thread_protected) */
/*lint -sem(misc_cli_cmd_sfp_phy_debug_read, thread_protected) */
/*lint -sem(misc_cli_wis_reset, thread_protected) */
/*lint -sem(misc_cli_wis_mode, thread_protected) */
/*lint -sem(misc_cli_wis_atti, thread_protected) */
/*lint -sem(misc_cli_wis_conse_act, thread_protected) */
/*lint -sem(misc_cli_wis_txtti, thread_protected) */
/*lint -sem(misc_cli_wis_test_mode, thread_protected) */
/*lint -sem(misc_cli_wis_status, thread_protected) */
/*lint -sem(misc_cli_wis_test_status, thread_protected) */
/*lint -sem(misc_cli_wis_counters, thread_protected) */
/*lint -sem(misc_cli_wis_defects, thread_protected) */
/*lint -sem(misc_cli_wis_force_conf, thread_protected) */
/*lint -sem(misc_cli_wis_tx_overhead_conf, thread_protected) */
/*lint -sem(misc_cli_wis_prbs31_err_inj_conf, thread_protected) */
/*lint -sem(misc_cli_wis_tx_perf_thr_conf, thread_protected) */
/*lint -sem(misc_cli_wis_perf_counters_get, thread_protected) */
/*lint -sem(misc_cli_wis_event_force_conf, thread_protected) */
/*lint -sem(misc_cli_synce_mode_set, thread_protected) */
/*lint -sem(misc_cli_synce_clkout_set, thread_protected) */
/*lint -sem(misc_cli_xfp_clkout_set, thread_protected) */
/*lint -sem(misc_cli_cmd_phy_10g_fw_status, thread_protected) */
/*lint -sem(misc_cli_spi_transaction, thread_protected) */
/*lint -sem(misc_cli_cmd_sfp_debug_read, thread_protected) */
/*lint -sem(misc_cli_cmd_sfp_debug_write, thread_protected) */

uchar misc_addr_bf[VTSS_BF_SIZE(CLI_ADDR_MAX)];

typedef struct {
#ifdef VTSS_SW_OPTION_I2C
    uchar   i2c_data[CLI_INT_VALUES_MAX];
    int     i2c_data_cnt;
    int     i2c_addr_cnt;
    uchar   i2c_addr;
    uchar   sfp_reg_addr[2];
    uchar   micro_reg_addr1;
    uchar   micro_reg_addr2;
#endif
    u32    serialized_gpio_data[2];
    int    sgpio_bit;
    /* Chip number */
    vtss_chip_no_t chip_no;
    
    /* Register access */
    BOOL    blk_list[CLI_BLK_MAX];
    BOOL    addr_list[CLI_PHY_MAX];
    BOOL    gpio_list[12];
    ulong   page;

    /* Keywords */
    BOOL    detailed;
    BOOL    clear;
    BOOL    addr_sort;
    BOOL    add;
    BOOL    full; 
    BOOL    enable;
    BOOL    disable;
    u32     lan;
    u32     gpio_role;
    BOOL    phy_inst;
    BOOL             module_all;
    vtss_module_id_t module_list[VTSS_MODULE_ID_NONE];
    BOOL    header;
    u32     count;

    vtss_debug_layer_t layer;
    vtss_debug_group_t group;

    u8      i2c_clk_sel;    // I2C mux (register 20G 3:0)
    u8      i2c_mux;        // I2C mux (register 20G 3:0)
    u8      i2c_reg_addr;   // I2C mux address (register 21G 7:0)
    u8      i2c_device_addr;  // I2C device address (register 20G 15:9)
    BOOL    phy_mmd_access; // Indicate if a phy access is a MMD access.
    BOOL    phy_mmd_direct; // Indicate if a phy access is a MMD access.
    u32     devad;          // MMD devad address
    u32     mmd_addr;       // MMD devad address
    u32     mmd_reg_addr;  // MMD reg address
    u32     miim_addr;  // MIIM address
    u32     miim_ctrl;  // MIIM address
    u32     ib_cterm_value;
    u32     ib_eq_mode;
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
    u32     blk_id_1588;         /* 1588 internal block Id's */
    u32     csr_reg_offset_1588; /* 1588 Internal block's CSR register offset*/
#endif /* VTSS_FEATURE_PHY_TIMESTAMP */
#if defined(VTSS_FEATURE_MACSEC)
    u32     macsec_blk;
    u32     macsec_addr; 
#endif /* VTSS_FEATURE_MACSEC */
#if defined(VTSS_SW_OPTION_BOARD)
    u32     int_source;
#endif /* VTSS_SW_OPTION_BOARD */
    char    lb;
    char    failover;

    u8      lb_type;
#ifdef    VTSS_FEATURE_WIS
    u8      wis_overhead;
    BOOL    wis_overhead_set;
    u8      wis_tti_mode;
    BOOL    wis_tti_mode_set;
    u8      wis_gen_mode;
    u8      wis_ana_mode;
    u8      wis_tti[65];
    BOOL    wis_tti_set;
    BOOL    wis_loopback;
    BOOL    wis_loopback_set;
    u16     wis_aisl;
    u16     wis_rdil;
    u16     wis_fault;
    u8      wis_line_rx;
    u8      wis_line_tx;
    u8      wis_path_force;
    u8      wis_oh_id;
    u8      wis_oh_val[27];
    u32     wis_n_ebc_thr_s;
    u32     wis_n_ebc_thr_l;
    u32     wis_f_ebc_thr_l;
    u32     wis_n_ebc_thr_p;
    u32     wis_f_ebc_thr_p;
    u32     wis_force_events;
    u8      wis_err_inj;
#endif //VTSS_FEATURE_WIS
#if defined (VTSS_FEATURE_SYNCE_10G)
    u8      synce_mode;
    u8      synce_clk_out;
    u8      synce_hitless;
    u8      synce_rclk_div;
    u8      synce_sref_div;
    u8      synce_wref_div;
    BOOL    synce_mode_set;
    BOOL    synce_clk_out_set;
    BOOL    synce_hitless_set;
    BOOL    synce_rclk_div_set;
    BOOL    synce_sref_div_set;
    BOOL    synce_wref_div_set;
#endif /* VTSS_FEATURE_SYNCE_10G */

    vtss_phy_clk_source_t clock_src;
    vtss_phy_freq_t       clock_freq;
    vtss_phy_clk_squelch  clock_squelch;
    vtss_port_no_t        clock_phy_port;
    vtss_phy_recov_clk_t  clock_port;
    vtss_port_no_t        autoneg_phy_port;
    BOOL aneg_master_config;
    BOOL  ipv6;
#if defined(VTSS_SW_OPTION_IP2)
    vtss_ipv4_t sip;
    vtss_ipv4_t dip;
#endif /* VTSS_SW_OPTION_IP2 */

    u32 assert_type; // 1 = Application assertion, 2 = eCos assertion, 3 = exception (TLB miss).
    u32 no_of_bytes;
    u32 spi_cs;
    u32 gpio_value;
    u32 gpio_mask;
    u8  value_list[MAX_SPI_DATA];
    BOOL  list_given;
    u32 qs_mode;
    u32 port_value;
    u32 queue_value;
    u32 oversub;
    BOOL prio_list[VTSS_PRIOS];
} misc_cli_req_t;


#ifdef VTSS_FEATURE_PHY_TIMESTAMP
/* These APIs are defined to debug PHY Timestamp features, not intend
   to expose to API customers and hence move to API base folder;
   extern here to access thru Debug CLI */
extern vtss_rc vtss_phy_1588_csr_reg_write(const vtss_inst_t inst,
                                           const vtss_port_no_t port_no,
                                           const u32 blk_id,
                                           const u16 csr_address,
                                           const u32 *const value);
extern vtss_rc vtss_phy_1588_csr_reg_read(const vtss_inst_t inst,
                                          const vtss_port_no_t port_no,
                                          const u32 blk_id,
                                          const u16 csr_address,
                                          u32 *const value);
#endif /*VTSS_FEATURE_PHY_TIMESTAMP*/


/****************************************************************************/
/*  Default misc_req initialization function                                */
/****************************************************************************/
static void misc_cli_req_default_set(cli_req_t *req)
{
    misc_cli_req_t *misc_req = req->module_req;
    
    misc_req->module_all = 1;

    memset(misc_addr_bf, 0, sizeof(misc_addr_bf));
}

/****************************************************************************/
/*  Initialization                                                          */
/****************************************************************************/

void misc_cli_req_init(int a_trace_grp_crit)
{
    /* register the size required for port req. structure */
    cli_req_size_register(sizeof(misc_cli_req_t));

    trace_grp_crit = a_trace_grp_crit;

    critd_init(&crit, "misc_crit", VTSS_MODULE_ID_MISC, VTSS_MODULE_ID_MISC, CRITD_TYPE_MUTEX);
    MISC_CLI_CRIT_EXIT();
}

/****************************************************************************/
/*  Command functions                                                       */
/****************************************************************************/

#ifdef VTSS_SW_OPTION_I2C

#include <cyg/io/i2c_vcoreiii.h>

static void misc_cli_cmd_i2c_rd_scan ( cli_req_t * req )
{
    int32_t i = 0;
    uchar i2c_data[40];

    for (i = 0 ; i <= 127; i ++ ) {
        CPRINTF("Testing I2C address : 0x%X\n",i);
        if (vtss_i2c_rd(NULL, i, &i2c_data[0], 1, 100, NO_I2C_MULTIPLEXER) == VTSS_RC_OK ) {
            CPRINTF("Found I2C Slave at address 0x%X \n",i);
        }
    }
}

#if defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_JAGUAR_1)
static void misc_cli_cmd_sfp_phy_debug_read( cli_req_t * req )
{
    uchar i2c_data[40];
    uchar byte_addr;
    uchar addr;
    int32_t i = 0;
    u32 iport, phy_reg;
    misc_cli_req_t *misc_req = req->module_req;
    
    if((vtss_board_type() != VTSS_BOARD_JAG_SFP24_REF) && (vtss_board_type() != VTSS_BOARD_LUTON26_REF)) {
        CPRINTF("Unsupported board: %d = %s\n", vtss_board_type(), vtss_board_name());
        return;
    }

    for (iport = VTSS_PORT_NO_START; iport < VTSS_PORT_NO_END; iport++) {
        if (req->uport_list[iport2uport(iport)] == 0)
            continue;
    
        if (board_sfp_i2c_lock(1) == VTSS_RC_ERROR) {
            CPRINTF("The SFP i2c is currently locked by the application - try again");
            return;
        }
            
        (void) board_sfp_i2c_enable(iport);

        if (misc_req->i2c_addr != 0) {
            addr = misc_req->i2c_addr;
        } else {
            addr = 0x56;
        }
        /* Workaround for i2c driver issue */
        if (vtss_i2c_rd(NULL, 0, &i2c_data[0], 1, 100, NO_I2C_MULTIPLEXER) != VTSS_RC_OK ) {            
        }

        /* end of workaround */
        phy_reg = 0;
        byte_addr = 0;
        while(phy_reg < 32) {
            byte_addr = (phy_reg < 11) ? 0 : (phy_reg < 22) ? 22 : 44;
            if (vtss_i2c_wr_rd(NULL, addr, &byte_addr, 1, &i2c_data[0], 22, 100, 1) == VTSS_RC_OK) {
                for (i=0; i<22; i=i+2) {
                        
                    CPRINTF("Phy reg:%u = 0x%.2X%.2X %c %c%c\n",phy_reg, i2c_data[i], 
                            i2c_data[i+1],addr==0x50?':':' ',addr==0x50?i2c_data[i]:' ',addr==0x50?i2c_data[i+1]:' ');
                    phy_reg++;
                    if (phy_reg > 31) 
                        break;
                }

            } else {
                printf("Error in read\n");
                break;
            }
        }
        if (board_sfp_i2c_lock(0) == VTSS_RC_ERROR) {
            CPRINTF("Could not unlock the SFP i2c");
            return;
        }
    }
}
#endif /* VTSS_ARCH_LUTON26 || VTSS_ARCH_JAGUAR_1 */

static void misc_cli_cmd_i2c_debug_read ( cli_req_t * req )
{
    uchar i2c_data[40];
    int32_t i = 0;
    misc_cli_req_t *misc_req = req->module_req;
    
    T_D("PRIO_I2C_RD, misc_req->i2c_addr = 0x%X", misc_req->i2c_addr);
    if (vtss_i2c_rd(NULL, misc_req->i2c_addr, &i2c_data[0], misc_req->i2c_data_cnt, 100, NO_I2C_MULTIPLEXER) == VTSS_RC_OK) {
        for (i = 0; i < misc_req->i2c_data_cnt; i++) {
            CPRINTF("Data read from I2C = 0x%X, addr = 0x%X, data_index = %d \n",
                    i2c_data[i], misc_req->i2c_addr, i);
        }
    }
}

static void misc_cli_cmd_i2c_debug_write ( cli_req_t * req )
{
    int32_t i = 0;
    misc_cli_req_t *misc_req = req->module_req;
    
    CPRINTF("Writing I2C addr = 0x%X, Data =  ", misc_req->i2c_addr);
    T_D("misc_req->i2c_data[0] = 0x%X, misc_req->i2c_data_cnt = %d",
        misc_req->i2c_data[0], misc_req->i2c_data_cnt);

    for (i = 0; i < misc_req->i2c_data_cnt; i++) {
        CPRINTF("0x%X ", misc_req->i2c_data[i]);
    }
    CPRINTF("\n");
    if (vtss_i2c_wr(NULL, misc_req->i2c_addr, &misc_req->i2c_data[0], misc_req->i2c_data_cnt, 100, NO_I2C_MULTIPLEXER) == VTSS_RC_ERROR)
        CPRINTF("Write operation failed\n");
}

static void misc_cli_cmd_i2c_debug_mux_write ( cli_req_t * req )
{
    int32_t i = 0;
    misc_cli_req_t *misc_req = req->module_req;
    
    CPRINTF("Writing I2C addr = 0x%X, Data =  ", misc_req->i2c_addr);
    T_D("misc_req->i2c_data[0] = 0x%X, misc_req->i2c_data_cnt = %d",
        misc_req->i2c_data[0], misc_req->i2c_data_cnt);

    for (i = 0; i < misc_req->i2c_data_cnt; i++) {
        CPRINTF("0x%X ", misc_req->i2c_data[i]);
    }
    CPRINTF("\n");
    if (vtss_i2c_wr(NULL, misc_req->i2c_addr, &misc_req->i2c_data[0], misc_req->i2c_data_cnt, 100, misc_req->i2c_clk_sel) == VTSS_RC_ERROR)
        CPRINTF("Write operation failed\n");
}

static void misc_cli_cmd_sfp_debug_read( cli_req_t * req )
{
    uchar i2c_data[40];
    int32_t i = 0;
    misc_cli_req_t *misc_req = req->module_req;
    
    T_D("i2c_data[0] = 0x%X, i2c_addr_cnt = %d, i2c_data_cnt = %d, sfp_reg_addr[0] = 0x%X, sfp_reg_addr[1] = 0x%X, misc_req->i2c_clk_sel:0x%X",
        misc_req->i2c_data[0], misc_req->i2c_addr_cnt, misc_req->i2c_data_cnt, misc_req->sfp_reg_addr[0], misc_req->sfp_reg_addr[1], misc_req->i2c_clk_sel);
    
    if (board_sfp_i2c_lock(1) != VTSS_RC_OK) {
        CPRINTF("i2c bus is busy, please try again\n");
        return ;
    }
    
    if (board_sfp_i2c_read(misc_req->i2c_clk_sel, misc_req->i2c_addr, misc_req->sfp_reg_addr[0], &i2c_data[0], misc_req->i2c_data_cnt) == VTSS_RC_OK) {
        for (i = 0; i < misc_req->i2c_data_cnt; i++) {
            CPRINTF("Data read from I2C = 0x%X,\n",
                    i2c_data[i]);
        }
    }
    (void)board_sfp_i2c_lock(0);
}

static void misc_cli_cmd_sfp_debug_write (cli_req_t * req)
{
    int32_t i = 0;
    misc_cli_req_t *misc_req = req->module_req;
    uchar i2c_data[3];
    
    CPRINTF("Writing I2C addr = 0x%X, Data =  ", misc_req->i2c_addr);
    T_D("misc_req->i2c_data_cnt = %d, misc_req->i2c_addr_cnt = %d",
        misc_req->i2c_data_cnt, misc_req->i2c_addr_cnt);

    CPRINTF("sfp_reg_addr\n");
    for (i = 0; i < 1; i ++ ) {
        CPRINTF("0x%X ", misc_req->sfp_reg_addr[i]);
    }

    CPRINTF("\n");
    CPRINTF("i2c_data\n");
    for (i = 0; i < 2; i ++ ) {
        CPRINTF("0x%X ", misc_req->i2c_data[i]);
    }

    CPRINTF("\n");

    i2c_data[0] = misc_req->sfp_reg_addr[0];
    i2c_data[1] = misc_req->i2c_data[0];
    i2c_data[2] = misc_req->i2c_data[1];
    CPRINTF("serial data\n");
    for (i = 0; i < 3; i++) {
        CPRINTF("0x%X ", i2c_data[i]);
    }
    CPRINTF("\n");

    if (board_sfp_i2c_lock(1) != VTSS_RC_OK) {
        CPRINTF("i2c bus is busy, please try again\n");
        return ;
    }

    if (board_sfp_i2c_write(misc_req->i2c_clk_sel, misc_req->i2c_addr, i2c_data[0], &i2c_data[1]) == VTSS_RC_ERROR) {
        CPRINTF("Write operation failed\n");
    }
    (void)board_sfp_i2c_lock(0);
}


#endif /* VTSS_SW_OPTION_I2C */


static void misc_cli_cmd_debug_ser_rd ( cli_req_t * req )
{
    
#ifdef VTSS_SW_OPTION_SERIALIZED_GPIO
    // This is for Luton28 enzo
    CPRINTF("Data read from Serialized GPIOs = 0x%X \n", serialized_gpio_data_rd());
#endif /* #ifdef VTSS_SW_OPTION_SERIALIZED_GPIO */
#ifdef VTSS_FEATURE_SERIAL_GPIO
    u8 p, bit;
    u32 spgio_hex[4]; // Maximum 4 bits port sgpio
    vtss_sgpio_port_data_t   sgpio_data[VTSS_SGPIO_PORTS];

    vtss_sgpio_conf_t conf;    
    if (vtss_sgpio_conf_get(NULL, 0, 0, &conf) != VTSS_RC_OK) {
        T_E("Could not get sgpio conf");
        return;
    }

    // Loop through all the sgpio and print out their value.
    if (vtss_sgpio_read(NULL, 0, 0, sgpio_data) != VTSS_RC_OK) {
        T_E("Could not get sgpio data");
    } else {
        spgio_hex[0] = 0;
        spgio_hex[1] = 0;
        spgio_hex[2] = 0;
        spgio_hex[3] = 0;

        for (p = 0; p < VTSS_SGPIO_PORTS; p++) {
            for (bit = 0; bit < conf.bit_count; bit++) {
                CPRINTF("p%db%d=%d ", p, bit, sgpio_data[p].value[bit] );
                spgio_hex[bit] |= sgpio_data[p].value[bit] <<p;
            }
            CPRINTF(" \n");
        }
        for (bit = 0; bit < conf.bit_count; bit++) {
            CPRINTF("Bit%d as hex:0x%X\n", bit, spgio_hex[bit]);
        }
    }
#endif // VTSS_FEATURE_SERIAL_GPIO
}

static void misc_cli_cmd_debug_ser_wr ( cli_req_t * req )
{
#if defined(VTSS_SW_OPTION_SERIALIZED_GPIO) || defined(VTSS_FEATURE_SERIAL_GPIO)
    misc_cli_req_t *misc_req = req->module_req;
#endif

#ifdef VTSS_SW_OPTION_SERIALIZED_GPIO    
    // This is for Luton28 enzo
    serialized_gpio_data_wr(misc_req->serialized_gpio_data[0], 0xFFFF);
#endif /* #ifdef VTSS_SW_OPTION_SERIALIZED_GPIO */

#ifdef VTSS_FEATURE_SERIAL_GPIO
    u8 p;
    vtss_sgpio_conf_t conf;
    for (p = 0; p < VTSS_SGPIO_PORTS; p++) {
       conf.port_conf[p].mode[0] = (misc_req->serialized_gpio_data[0] >> p) & 0x1;
       conf.port_conf[p].mode[1] = (misc_req->serialized_gpio_data[1] >> p) & 0x1;
    }
    conf.bit_count = misc_req->sgpio_bit;
   (void)vtss_sgpio_conf_set(NULL, 0, 0, &conf);
#endif // VTSS_FEATURE_SERIAL_GPIO
}

static void misc_cli_cmd_debug_reg(cli_req_t *req)
{
    ulong          first, tgt, addr, reg_addr, value, failed = 0;
    int            i;
    misc_cli_req_t *misc_req = req->module_req;
    vtss_chip_no_t chip_no, chip_no_start = 0, chip_no_count = 1;
    char           buf[32];
    vtss_isid_t    isid = req->stack.isid_debug;

    /* Get current chip number */
    if (misc_chip_no_get(&chip_no) != VTSS_RC_OK)
        return;
    
    if (chip_no == VTSS_CHIP_NO_ALL) {
        chip_no_count = vtss_api_if_chip_count();
    } else {
        chip_no_start = chip_no;
    }

    for (chip_no = chip_no_start; chip_no < (chip_no_start + chip_no_count); chip_no++) {
        if (chip_no_count > 1) {
            sprintf(buf, "Chip %u", chip_no);
            cli_header(buf, 0);
        }
        
        first = 1;
        for (tgt = 0; tgt < CLI_BLK_MAX; tgt++) {
            if (misc_req->blk_list[tgt] == 0)
                continue;
            
            for (addr = 0; addr < CLI_ADDR_MAX; addr++) {
                if (VTSS_BF_GET(misc_addr_bf, addr) == 0)
                    continue;
                
                reg_addr = ((tgt << 14) | addr);
                if (req->set) {
                    /* Write */
                    if (misc_debug_reg_write(isid, chip_no, reg_addr, req->value) != VTSS_OK)
                        failed++;
                } else if (misc_debug_reg_read(isid, chip_no, reg_addr, &value) != VTSS_OK) {
                    /* Read failure */
                    failed++;
                } else {
                    /* Read success */
                    if (first) {
                        CPRINTF("Target  Address  Value       Decimal     31     24 23     16 15      8 7       0\n");
                        first = 0;
                    }
                    CPRINTF("0x%02x    0x%04x   0x%08x  %-12u", tgt, addr, value, value);
                    for (i = 31; i >= 0; i--)
                        CPRINTF("%d%s", value & (1<<i) ? 1 : 0, i == 0 ? "\n" : (i % 4) ? "" : ".");
                }
            }
        }
        if (failed)
            CPRINTF("%u operations failed\n", failed);
    }
}

static const char * const misc_cli_group_table[VTSS_DEBUG_GROUP_COUNT] = {
    [VTSS_DEBUG_GROUP_ALL]       = "all",
    [VTSS_DEBUG_GROUP_INIT]      = "init",
    [VTSS_DEBUG_GROUP_MISC]      = "misc",
    [VTSS_DEBUG_GROUP_PORT]      = "port",
    [VTSS_DEBUG_GROUP_PORT_CNT]  = "counters",
    [VTSS_DEBUG_GROUP_PHY]       = "phy",
    [VTSS_DEBUG_GROUP_VLAN]      = "vlan",
    [VTSS_DEBUG_GROUP_PVLAN]     = "pvlan",
    [VTSS_DEBUG_GROUP_MAC_TABLE] = "mac_table",
    [VTSS_DEBUG_GROUP_ACL]       = "acl",
    [VTSS_DEBUG_GROUP_QOS]       = "qos",
    [VTSS_DEBUG_GROUP_AGGR]      = "aggr",
    [VTSS_DEBUG_GROUP_GLAG]      = "glag",
    [VTSS_DEBUG_GROUP_STP]       = "stp",
    [VTSS_DEBUG_GROUP_MIRROR]    = "mirror",
    [VTSS_DEBUG_GROUP_EVC]       = "evc",
    [VTSS_DEBUG_GROUP_ERPS]      = "erps",
    [VTSS_DEBUG_GROUP_EPS]       = "eps",
    [VTSS_DEBUG_GROUP_PACKET]    = "packet",
    [VTSS_DEBUG_GROUP_FDMA]      = "fdma",
    [VTSS_DEBUG_GROUP_TS]        = "ts",
    [VTSS_DEBUG_GROUP_PHY_TS]    = "pts",
    [VTSS_DEBUG_GROUP_WM]        = "wm",
    [VTSS_DEBUG_GROUP_LRN]       = "lrn",
    [VTSS_DEBUG_GROUP_STACK]     = "stack",
    [VTSS_DEBUG_GROUP_IPMC]      = "ipmc",
    [VTSS_DEBUG_GROUP_CMEF]      = "cmef",
    [VTSS_DEBUG_GROUP_HOST]      = "host",
    [VTSS_DEBUG_GROUP_VXLAT]     = "vxlat",
    [VTSS_DEBUG_GROUP_SER_GPIO]  = "serial_gpio",    
#if defined(VTSS_FEATURE_OAM)
    [VTSS_DEBUG_GROUP_OAM]       = "oam",    
#endif
#if defined(VTSS_SW_OPTION_L3RT) || defined(VTSS_ARCH_JAGUAR_1)
    [VTSS_DEBUG_GROUP_L3]        = "l3",
#endif /* VTSS_SW_OPTION_L3RT || VTSS_ARCH_JAGUAR_1*/
    [VTSS_DEBUG_GROUP_MPLS]      = "mplscore",
    [VTSS_DEBUG_GROUP_MPLS_OAM]  = "mplsoam",
#if defined(VTSS_FEATURE_AFI_SWC)
    [VTSS_DEBUG_GROUP_AFI]       = "afi",
#endif /* VTSS_FEATURE_AFI_SWC */
#if defined(VTSS_FEATURE_MACSEC)
    [VTSS_DEBUG_GROUP_MACSEC]       = "macsec",
#endif
};

static void misc_cli_cmd_debug_chip(cli_req_t *req)
{
    misc_cli_req_t *misc_req = req->module_req;
    vtss_chip_no_t chip_no;
    
    if (req->set) {
        if (misc_chip_no_set(misc_req->chip_no) != VTSS_OK)
            CPRINTF("SET failed");
    } else {
        if (misc_chip_no_get(&chip_no) == VTSS_RC_OK) {
            CPRINTF("Chip Number: ");
            if (chip_no == VTSS_CHIP_NO_ALL)
                CPRINTF("All\n");
            else
                CPRINTF("%u\n", chip_no);
        } else
            CPRINTF("GET failed");
    }
}

static void misc_cli_cmd_debug_api(cli_req_t *req)
{
    vtss_debug_info_t  info;
    vtss_debug_group_t group;
    vtss_port_no_t     iport;
    misc_cli_req_t     *misc_req = req->module_req;
    const char         *name;
    if (misc_req->group == VTSS_DEBUG_GROUP_COUNT) {
        CPRINTF("Legal groups are:\n\n");
        for (group = VTSS_DEBUG_GROUP_ALL; group < VTSS_DEBUG_GROUP_COUNT; group++) {
            name =  misc_cli_group_table[group];
            if (name != NULL)
                CPRINTF("%s\n", name);
        }
    } else if (vtss_debug_info_get(&info) == VTSS_OK) {
        info.layer = misc_req->layer;
        info.group = misc_req->group;
        info.full = misc_req->full;
        info.clear = misc_req->clear;
        for (iport = VTSS_PORT_NO_START; iport < VTSS_PORT_NO_END; iport++)
            info.port_list[iport] = req->uport_list[iport2uport(iport)];
        if (misc_chip_no_get(&info.chip_no) == VTSS_RC_OK &&
            vtss_debug_info_print(MISC_INST, (vtss_debug_printf_t)cli_printf, &info) != VTSS_OK)
            CPRINTF("Debug info failed\n");
    }
}

static void misc_cli_cmd_debug_reg_read ( cli_req_t * req )
{
    req->set = 0;
    misc_cli_cmd_debug_reg(req);
}

static void misc_cli_cmd_debug_reg_write ( cli_req_t * req )
{
    req->set = 1;
    misc_cli_cmd_debug_reg(req);
}

static void cli_cmd_phy_reg(cli_req_t *req, port_iter_t *pit)
{
    misc_cli_req_t  *misc_req = req->module_req;
    vtss_isid_t     isid = req->stack.isid_debug;
    uint            addr = misc_req->mmd_reg_addr;
    BOOL            mmd = misc_req->phy_mmd_access;
    ushort          value;
    int             i;

    if (req->set) {
        /* Write */
        if (misc_debug_phy_write(isid, pit->iport, addr, misc_req->page, req->value, mmd, misc_req->devad) != VTSS_OK)
            misc_req->count++;
    } else if (misc_debug_phy_read(isid, pit->iport, addr, misc_req->page, &value, mmd, misc_req->devad) != VTSS_OK) {
        /* Read failure */
        misc_req->count++;
    } else {
        if (misc_req->header) {
            misc_req->header = 0;
            CPRINTF("Port  Addr     Value   15      8 7       0\n");
        }
        CPRINTF("%-6u", pit->uport);
        if (mmd) {
            CPRINTF("0x%04x   ", addr);
        } else {
            CPRINTF("0x%02x/%-4u", addr, addr);
        }
        CPRINTF("0x%04x  ", value);
        for (i = 15; i >= 0; i--) {
            CPRINTF("%u%s", value & (1<<i) ? 1 : 0, i == 0 ? "\n" : (i % 4) ? "" : ".");
        }
    }
}


static void cli_cmd_debug_phy(cli_req_t *req)
{
    misc_cli_req_t *misc_req = req->module_req;
    uint           addr;
    port_iter_t    pit;

    if (cli_cmd_switch_none(req))
        return;
    
    misc_req->header = 1;
    if (misc_req->addr_sort) {
        /* Iterate in (address, port) order */
        for (addr = 0; addr < CLI_PHY_MAX; addr++) {
            if (misc_req->phy_mmd_access) {
                addr = CLI_PHY_MAX;
            } else {
                misc_req->mmd_reg_addr = addr;
                if (misc_req->addr_list[addr] == 0)
                    continue;
            }
            port_iter_init_local(&pit);
            while (port_iter_getnext(&pit)) {
                if (req->uport_list[pit.uport] == 0)
                    continue;
                cli_cmd_phy_reg(req, &pit);
            }
        }
    } else {
        /* Iterate in (port, address) order */
        port_iter_init_local(&pit);
        while (port_iter_getnext(&pit)) {
            if (req->uport_list[pit.uport] == 0)
                continue;

            for (addr = 0; addr < CLI_PHY_MAX; addr++) {
                if (misc_req->phy_mmd_access) {
                    addr = CLI_PHY_MAX;
                } else {
                    misc_req->mmd_reg_addr = addr;
                    if (misc_req->addr_list[addr] == 0)
                        continue;
                }
                cli_cmd_phy_reg(req, &pit);
            }
        }
    }
    if (misc_req->count)
        CPRINTF("%u operations failed\n", misc_req->count);
}

static void misc_cli_cmd_debug_phy_mmd_read ( cli_req_t * req )
{
    misc_cli_req_t     *misc_req = req->module_req;
    req->set = 0;
    misc_req->phy_mmd_access = TRUE;
    cli_cmd_debug_phy(req);
}

static void misc_cli_cmd_debug_phy_mmd_write ( cli_req_t * req )
{
    misc_cli_req_t     *misc_req = req->module_req;
    req->set = 1;
    misc_req->phy_mmd_access = TRUE;
    cli_cmd_debug_phy(req);
}

static void misc_cli_cmd_debug_phy_clock_conf ( cli_req_t * req )
{
    misc_cli_req_t         *misc_req = req->module_req;
    vtss_phy_clock_conf_t  conf;
    vtss_port_no_t         clock_source;

    if (req->set) {
        conf.src = misc_req->clock_src;
        conf.freq = misc_req->clock_freq;
        conf.squelch = misc_req->clock_squelch;
        if (vtss_phy_clock_conf_set(MISC_INST, misc_req->clock_phy_port, misc_req->clock_port, &conf) != VTSS_RC_OK)    CPRINTF("vtss_phy_clock_conf_set() failed");
    }
    else {
        if (vtss_phy_clock_conf_get(MISC_INST, misc_req->clock_phy_port, misc_req->clock_port, &conf, &clock_source) != VTSS_RC_OK)    CPRINTF("vtss_phy_clock_conf_get() failed");
        CPRINTF("\n");
        CPRINTF("Clock Configuration is:\n");
        CPRINTF("%10s", "Clock");
        CPRINTF("%11s", "Source");
        CPRINTF("%9s", "Type");
        CPRINTF("%14s", "Frequence");
        CPRINTF("%12s", "Squelch");
        CPRINTF("\n");

        switch (misc_req->clock_port)
        {
            case VTSS_PHY_RECOV_CLK1:    CPRINTF("%10s", "1");     break;
            case VTSS_PHY_RECOV_CLK2:    CPRINTF("%10s", "2");     break;
            default:                     CPRINTF("%10s", "-");
        }
        CPRINTF("%11u", clock_source+1);
        switch (conf.src)
        {
            case VTSS_PHY_SERDES_MEDIA:    CPRINTF("%9s", "serdes");     break;
            case VTSS_PHY_COPPER_MEDIA:    CPRINTF("%9s", "copper");     break;
            case VTSS_PHY_TCLK_OUT:        CPRINTF("%9s", "tclk");       break;
            case VTSS_PHY_LOCAL_XTAL:      CPRINTF("%9s", "xtal");       break;
            case VTSS_PHY_CLK_DISABLED:    CPRINTF("%9s", "disable");    break;
            default:                       CPRINTF("%9s", "-------");    break;
        }
        switch (conf.freq)
        {
            case VTSS_PHY_FREQ_25M:     CPRINTF("%14s", "25M");      break;
            case VTSS_PHY_FREQ_125M:    CPRINTF("%14s", "125M");     break;
            case VTSS_PHY_FREQ_3125M:   CPRINTF("%14s", "3125M");    break;
            default:                    CPRINTF("%14s", "-----");    break;
        }
        switch (conf.squelch)
        {
            case VTSS_PHY_CLK_SQUELCH_MAX:    CPRINTF("%12s", "0");    break;
            case VTSS_PHY_CLK_SQUELCH_MED:    CPRINTF("%12s", "1");    break;
            case VTSS_PHY_CLK_SQUELCH_MIN:    CPRINTF("%12s", "2");    break;
            case VTSS_PHY_CLK_SQUELCH_NONE:   CPRINTF("%12s", "3");    break;
            default:                          CPRINTF("%12s", "-");    break;
        }
        CPRINTF("\n");
    }
}

static void misc_cli_cmd_debug_1g_master_slave_conf( cli_req_t * req )
{ 
    misc_cli_req_t         *misc_req = req->module_req;
    vtss_phy_conf_1g_t     phy_setup;

    if (req->set) { 
        phy_setup.master.cfg = misc_req->enable;
        phy_setup.master.val = misc_req->aneg_master_config;
        if (vtss_phy_conf_1g_set(MISC_INST, misc_req->autoneg_phy_port, &phy_setup) != VTSS_RC_OK)    CPRINTF("vtss_phy_clock_conf_set() failed");
    }
    else {
        CPRINTF("\n");
        CPRINTF("Auto Negotiation Manual Master/Slave Configuration is:\n\n");
        CPRINTF("%15s", "Manual Config");
        CPRINTF("%15s", "Master/Slave");
        CPRINTF("\n");
        if (vtss_phy_conf_1g_get(MISC_INST, misc_req->autoneg_phy_port, &phy_setup) != VTSS_RC_OK)    CPRINTF("vtss_phy_clock_conf_get() failed");
        if (phy_setup.master.cfg) {
            CPRINTF("%15s","Enabled");
            if (phy_setup.master.val) {
                CPRINTF("%15s","Master");
            } else {
                CPRINTF("%15s","Slave");
            }
        } else {
            CPRINTF("%15s","Disabled");
            CPRINTF("%15s","-----");
        }
        CPRINTF("\n");
    }
}


#if defined(VTSS_FEATURE_10G)
static void misc_cli_cmd_debug_phy_mmd_read_direct ( cli_req_t * req )
{
    misc_cli_req_t  *misc_req = req->module_req;
    u32 reg, mmd, miim;
    u16 value, ctrl;

    reg = misc_req->mmd_reg_addr;
    mmd = misc_req->devad;
    miim = misc_req->miim_addr;
    ctrl = misc_req->miim_ctrl;

    if (vtss_mmd_read(MISC_INST, 0, ctrl, miim, mmd, reg, &value) != VTSS_RC_OK)
        CPRINTF("mmd read failed\n");
    else             
        CPRINTF("0x%04x\n", value);
    
}


static void misc_cli_cmd_debug_phy_mmd_write_direct ( cli_req_t * req )
{
    misc_cli_req_t  *misc_req = req->module_req;
    u32 reg, mmd, miim;
    u16 ctrl;

    reg = misc_req->mmd_reg_addr;
    mmd = misc_req->devad;
    miim = misc_req->miim_addr;
    ctrl = misc_req->miim_ctrl;

    if (vtss_mmd_write(MISC_INST, 0, ctrl, miim, mmd, reg, req->value) != VTSS_RC_OK)
        CPRINTF("mmd read failed\n");

}
#endif /* VTSS_FEATURE_10G */

static void misc_cli_cmd_debug_phy_read ( cli_req_t * req )
{
    misc_cli_req_t     *misc_req = req->module_req;
    req->set = 0;
    misc_req->phy_mmd_access = FALSE;
    cli_cmd_debug_phy(req);
}


static void misc_cli_cmd_debug_phy_i2c_rd ( cli_req_t * req )
{
    u8 value[200];
    misc_cli_req_t  *misc_req = req->module_req;
    req->set = 0;
    vtss_port_no_t port_no, port;
    u8 byte_cnt;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->uport_list[port] == 0)) 
            continue;
        
        T_I_PORT(port_no, "i2c_mux = %d, i2c_device_addr = 0x%X, i2c_reg_addr = 0x%X", misc_req->i2c_mux, misc_req->i2c_device_addr, misc_req->i2c_reg_addr); 
        
        if (vtss_phy_i2c_read(MISC_INST, port_no, misc_req->i2c_mux, misc_req->i2c_reg_addr, misc_req->i2c_device_addr, &value[0], misc_req->i2c_data_cnt) != VTSS_RC_OK) {
            CPRINTF("Phy i2c read failed\n");
        } else {
            CPRINTF("Addr   Data\n");
            for (byte_cnt = 0; byte_cnt < misc_req->i2c_data_cnt; byte_cnt ++) {
                CPRINTF("0x%04x 0x%04x\n", misc_req->i2c_reg_addr, value[byte_cnt]);
            }
        }
    }
}

static void misc_cli_cmd_debug_phy_i2c_wr ( cli_req_t * req )
{
    vtss_port_no_t port_no, port;
    misc_cli_req_t *misc_req = req->module_req;
    
    CPRINTF("Writing PHY I2C device addr = 0x%X, mux = %d, reg_addr = %d   \n",misc_req->i2c_device_addr, misc_req->i2c_mux, misc_req->i2c_reg_addr);

    T_D("misc_req->i2c_data[0] = 0x%X, misc_req->i2c_data_cnt = %d",
        misc_req->i2c_data[0], misc_req->i2c_data_cnt);

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->uport_list[port] == 0)) 
            continue;

        if (vtss_phy_i2c_write(MISC_INST, port_no, misc_req->i2c_mux, misc_req->i2c_reg_addr, 
                               misc_req->i2c_device_addr, &misc_req->i2c_data[0],  misc_req->i2c_data_cnt) != VTSS_RC_OK) {
            CPRINTF("Phy i2c write failed\n");
        }
    }
}



static void misc_cli_cmd_debug_phy_write ( cli_req_t * req )
{
    misc_cli_req_t     *misc_req = req->module_req;
    req->set = 1;
    misc_req->phy_mmd_access = FALSE;
    cli_cmd_debug_phy(req);
}

static void misc_cli_cmd_debug_ob_post(cli_req_t * req)
{
    vtss_port_no_t iport, uport;
    for (iport = VTSS_PORT_NO_START; iport < VTSS_PORT_NO_END; iport++) {
        uport = iport2uport(iport);
        if (req->uport_list[uport] == 0)
            continue;
        
        if (vtss_phy_cfg_ob_post0(NULL, iport, req->value) == VTSS_RC_OK) {
            CPRINTF("Post0 set \n");
        } else {
            CPRINTF("Post not set \n");
        }
    }
}

// Function for printing a specific bit(s) in the phy patch configuration/status array.
// IN : string - The print to print
//      msb    - MSB bit in array.
//      lsb    - LSB bit in array - If only one bit the LSB shall be set to -1
//      vec    - Pointer the setting/status patch array
static void cfg_print(i8 *string, i16 msb, i16 lsb, u8 *vec) {
    u16 bit_idx;
    u8 value = 0;
    
    if (lsb > 0) {
        for (bit_idx = lsb; bit_idx <= msb; bit_idx++) {
            value += VTSS_BF_GET(vec, bit_idx) << (bit_idx - lsb);
        }
    } else {
        value += VTSS_BF_GET(vec, msb);
    }

    CPRINTF("%s:0x%X\n", string, value);
}

// Function printing the PHY micro patch settings
static void misc_cli_cmd_patch_settings_get(cli_req_t * req)
{
    vtss_port_no_t iport, uport;
    u8 cfg_buf[MAX_CFG_BUF_SIZE];
    u8 stat_buf[MAX_STAT_BUF_SIZE];
    u8 mcb_bus = 0;

    for (iport = VTSS_PORT_NO_START; iport < VTSS_PORT_NO_END; iport++) {
        uport = iport2uport(iport);
        
        if (req->uport_list[uport] == 0)
            continue;

        mcb_bus = 0;
        // Get SerDes 6G      
        CPRINTF("Port :%u \n", uport);
        if (vtss_phy_atom12_patch_setttings_get(NULL, iport, &mcb_bus, &cfg_buf[0], &stat_buf[0]) == VTSS_RC_OK) {
            if (mcb_bus == 1) {
                // Get SerDes 6G      
                CPRINTF("************ serdes6g_dig_cfg ************\n");
                cfg_print("gp",281,279, &cfg_buf[0]);
                cfg_print("tx_bit_doubling_mode_ena",278, -1, &cfg_buf[0]);
                cfg_print("sigdet_testmode",277, -1, &cfg_buf[0]);
                cfg_print("sigdet_ast",276,274, &cfg_buf[0]);
                cfg_print("sigdet_dst",273,271, &cfg_buf[0]);
                CPRINTF("************ serdes6g_dft_cfg0 ************\n");
                cfg_print("lazybit",270, -1,&cfg_buf[0]);
                cfg_print("inv_dis",269, -1, &cfg_buf[0]);
                cfg_print("prbs_sel",268,267, &cfg_buf[0]);
                cfg_print("test_mode",266,264,  &cfg_buf[0]);
                cfg_print("rx_phs_corr_dis",263, -1, &cfg_buf[0]);
                cfg_print("rx_pdsens_ena",262, -1, &cfg_buf[0]);
                cfg_print("rx_dft_ena",261, -1, &cfg_buf[0]);
                cfg_print("tx_dft_ena",260, -1, &cfg_buf[0]);
                CPRINTF("************ serdes6g_dft_cfg1 ************\n");
                cfg_print("tx_jitter_ampl",259,250, &cfg_buf[0]);
                cfg_print("tx_step_freq",249,246, &cfg_buf[0]);
                cfg_print("tx_ji_ena",245, -1, &cfg_buf[0]);
                cfg_print("tx_waveform_sel",244, -1, &cfg_buf[0]);
                cfg_print("tx_freqoff_dir",243, -1, &cfg_buf[0]);
                cfg_print("tx_freqoff_ena",242, -1, &cfg_buf[0]);
                CPRINTF("************ serdes6g_dft_cfg2 ************\n");
                cfg_print("rx_jitter_ampl",241,232, &cfg_buf[0]);
                cfg_print("rx_step_freq",231,228, &cfg_buf[0]);
                cfg_print("rx_ji_ena",227, -1, &cfg_buf[0]);
                cfg_print("rx_waveform_sel",226, -1, &cfg_buf[0]);
                cfg_print("rx_freqoff_dir",225, -1, &cfg_buf[0]);
                cfg_print("rx_freqoff_ena",224, -1, &cfg_buf[0]);
                CPRINTF("************ serdes6g_tp_cfg0 ************\n");
                cfg_print("static_pattern",223,204, &cfg_buf[0]);
                CPRINTF("************ serdes6g_tp_cfg1 ************\n");
                cfg_print("static_pattern2",203,184, &cfg_buf[0]);
                CPRINTF("************ serdes6g_misc_cfg ************\n");
                cfg_print("pll_bist_ena",183, -1, &cfg_buf[0]);
                cfg_print("des_100fx_kick_mode",182,181, &cfg_buf[0]);
                cfg_print("des_100fx_cpmd_swap",180, -1, &cfg_buf[0]);
                cfg_print("des_100fx_cpmd_mode",179, -1, &cfg_buf[0]);
                cfg_print("des_100fx_cpmd_ena",178, -1, &cfg_buf[0]);
                cfg_print("rx_lpi_mode_ena",177, -1, &cfg_buf[0]);
                cfg_print("tx_lpi_mode_ena",176, -1, &cfg_buf[0]);
                cfg_print("rx_data_inv_ena",175, -1, &cfg_buf[0]);
                cfg_print("tx_data_inv_ena",174, -1, &cfg_buf[0]);
                cfg_print("lane_rst",173, -1, &cfg_buf[0]);
                CPRINTF("************ serdes6g_ob_aneg_cfg ************\n");
                cfg_print("an_ob_post0",172,167, &cfg_buf[0]);
                cfg_print("an_ob_post1",166,162, &cfg_buf[0]);
                cfg_print("an_ob_prec",161,157, &cfg_buf[0]);
                cfg_print("an_ob_ena_cas",156,154, &cfg_buf[0]);
                cfg_print("an_ob_lev",153,148, &cfg_buf[0]);
                CPRINTF("************ serdes6g_des_cfg ************\n");
                cfg_print("des_phs_ctrl",147,144,  &cfg_buf[0]);
                cfg_print("des_mbtr_ctrl",143,141,&cfg_buf[0]);
                cfg_print("des_cpmd_sel",140,139, &cfg_buf[0]);
                cfg_print("des_bw_hyst",138,136,  &cfg_buf[0]);
                cfg_print("des_swap_hyst",135, -1, &cfg_buf[0]);
                cfg_print("des_bw_ana",134,132,  &cfg_buf[0]);
                cfg_print("des_swap_ana",131, -1, &cfg_buf[0]);
                CPRINTF("************ serdes6g_ib_cfg ************\n");
                cfg_print("acjtag_hyst",130,128,  &cfg_buf[0]);
                cfg_print("ib_ic_ac",127,125,  &cfg_buf[0]);
                cfg_print("ib_ic_com",124,122,  &cfg_buf[0]);
                cfg_print("ib_ic_dc",121,119,  &cfg_buf[0]);
                cfg_print("ib_r_cor",118, -1, &cfg_buf[0]);
                cfg_print("ib_rf",117,114,  &cfg_buf[0]);
                cfg_print("ib_rt",113,110,  &cfg_buf[0]);
                cfg_print("ib_vbac",109,107,  &cfg_buf[0]);
                cfg_print("ib_vbcom",106,104,  &cfg_buf[0]);
                cfg_print("ib_resistor_ctrl",103,100,  &cfg_buf[0]);
                CPRINTF("************ serdes6g_ib_cfg1 ************\n");
                cfg_print("ib_c_off",99,98,  &cfg_buf[0]);
                cfg_print("ib_c",97,94,  &cfg_buf[0]);
                cfg_print("ib_chf",93, -1, &cfg_buf[0]);
                cfg_print("ib_aneg_mode",92, -1, &cfg_buf[0]);
                cfg_print("ib_cterm_ena",91, -1, &cfg_buf[0]);
                cfg_print("ib_dis_eq",90, -1, &cfg_buf[0]);
                cfg_print("ib_ena_offsac",89, -1, &cfg_buf[0]);
                cfg_print("ib_ena_offsdc",88, -1, &cfg_buf[0]);
                cfg_print("ib_fx100_ena",87, -1, &cfg_buf[0]);
                cfg_print("ib_rst",86, -1, &cfg_buf[0]);
                CPRINTF("************ serdes6g_ob_cfg ************\n");
                cfg_print("ob_idle",85, -1, &cfg_buf[0]);
                cfg_print("ob_ena1v_mode",84, -1, &cfg_buf[0]);
                cfg_print("ob_pol",83, -1, &cfg_buf[0]);
                cfg_print("ob_post0",82,77,  &cfg_buf[0]);
                cfg_print("ob_post1",76,72,  &cfg_buf[0]);
                cfg_print("ob_prec",71,67,  &cfg_buf[0]);
                cfg_print("ob_r_adj_mux",66, -1, &cfg_buf[0]);
                cfg_print("ob_r_adj_pdr",65, -1, &cfg_buf[0]);
                cfg_print("ob_r_cor",64, -1, &cfg_buf[0]);
                cfg_print("ob_sel_rctrl",63, -1, &cfg_buf[0]);
                cfg_print("ob_sr_h",62, -1, &cfg_buf[0]);
                cfg_print("ob_sr",61,58, &cfg_buf[0]);
                cfg_print("ob_resistor_ctrl",57,54,  &cfg_buf[0]);
                CPRINTF("************ serdes6g_ob_cfg1 ************\n");
                cfg_print("ob_ena_cas",53,51,  &cfg_buf[0]);
                cfg_print("ob_lev",50,45,  &cfg_buf[0]);
                CPRINTF("************ serdes6g_ser_cfg ************\n");
                cfg_print("ser_4tap_ena",44, -1, &cfg_buf[0]);
                cfg_print("ser_cpmd_sel",43, -1, &cfg_buf[0]);
                cfg_print("ser_swap_cpmd",42, -1, &cfg_buf[0]);
                cfg_print("ser_alisel",41,40,  &cfg_buf[0]);
                cfg_print("ser_enhys",39, -1, &cfg_buf[0]);
                cfg_print("ser_big_win",38, -1, &cfg_buf[0]);
                cfg_print("ser_en_win",37, -1, &cfg_buf[0]);
                cfg_print("ser_enali",36, -1, &cfg_buf[0]);
                CPRINTF("************ serdes6g_common_cfg ************\n");
                cfg_print("sys_rst",35, -1, &cfg_buf[0]);
                cfg_print("se_auto_squelch_b_ena",34, -1, &cfg_buf[0]);
                cfg_print("se_auto_squelch_a_ena",33, -1, &cfg_buf[0]);
                cfg_print("reco_sel_b",32, -1, &cfg_buf[0]);
                cfg_print("reco_sel_a",31, -1, &cfg_buf[0]);
                cfg_print("ena_lane",30, -1, &cfg_buf[0]);
                cfg_print("pwd_rx",29, -1, &cfg_buf[0]);
                cfg_print("pwd_tx",28, -1, &cfg_buf[0]);
                cfg_print("lane_ctrl",27,25,  &cfg_buf[0]);
                cfg_print("ena_direct",24, -1, &cfg_buf[0]);
                cfg_print("ena_eloop",23, -1, &cfg_buf[0]);
                cfg_print("ena_floop",22, -1, &cfg_buf[0]);
                cfg_print("ena_iloop",21, -1, &cfg_buf[0]);
                cfg_print("ena_ploop",20, -1, &cfg_buf[0]);
                cfg_print("hrate",19, -1, &cfg_buf[0]);
                cfg_print("qrate",18, -1, &cfg_buf[0]);
                cfg_print("if_mode",17,16, &cfg_buf[0]);
                CPRINTF("************ serdes6g_pll_cfg ************\n");
                cfg_print("pll_div4",15, -1, &cfg_buf[0]);
                cfg_print("pll_ena_rot",14, -1, &cfg_buf[0]);
                cfg_print("pll_fsm_ctrl_data",13,6, &cfg_buf[0]);
                cfg_print("pll_fsm_ena",5, -1, &cfg_buf[0]);
                cfg_print("pll_fsm_force_set_ena",4, -1, &cfg_buf[0]);
                cfg_print("pll_fsm_oor_recal_ena",3, -1, &cfg_buf[0]);
                cfg_print("pll_rb_data_sel",2, -1, &cfg_buf[0]);
                cfg_print("pll_rot_dir",1, -1, &cfg_buf[0]);
                cfg_print("pll_rot_frq",0, -1, &cfg_buf[0]);
                CPRINTF("************ serdes6g_dft_status ************\n");
                cfg_print("pll_bist_not_done",17, -1, &stat_buf[0]);
                cfg_print("pll_bist_failed",16, -1, &stat_buf[0]);
                cfg_print("pll_bist_timeout_err",15, -1, &stat_buf[0]);
                cfg_print("bist_active",14, -1, &stat_buf[0]);
                cfg_print("bist_nosync",13, -1, &stat_buf[0]);
                cfg_print("bist_complete_n",12, -1, &stat_buf[0]);
                cfg_print("bist_error",11, -1, &stat_buf[0]);
                CPRINTF("************ serdes6g_pll_status ************\n");
                cfg_print("pll_cal_not_done",10, -1, &stat_buf[0]);
                cfg_print("pll_cal_err",9, -1, &stat_buf[0]);
                cfg_print("pll_out_of_range_err",8, -1, &stat_buf[0]);
                cfg_print("pll_rb_data",7,0, &stat_buf[0]);
            } else {
                CPRINTF("************ Serdes1g_dft_cfg0 ************\n");
                cfg_print("lazybit",177,-1, &cfg_buf[0]);
                cfg_print("inv_dis",176,-1, &cfg_buf[0]);
                cfg_print("prbs_sel",175,174, &cfg_buf[0]);
                cfg_print("test_mode",173,171, &cfg_buf[0]);
                cfg_print("rx_phs_corr_dis",170,-1, &cfg_buf[0]);
                cfg_print("rx_pdsens_ena",169,-1, &cfg_buf[0]);
                cfg_print("rx_dft_ena",168,-1, &cfg_buf[0]);
                cfg_print("tx_dft_ena",167,-1, &cfg_buf[0]);
                CPRINTF("************ serdes1g_dft_cfg1 ************\n");
                cfg_print("tx_jitter_ampl",166,157, &cfg_buf[0]);
                cfg_print("tx_step_freq",156,153, &cfg_buf[0]);
                cfg_print("tx_ji_ena",152,-1, &cfg_buf[0]);
                cfg_print("tx_waveform_sel",151, -1, &cfg_buf[0]);
                cfg_print("tx_freqoff_dir",150,-1, &cfg_buf[0]);
                cfg_print("tx_freqoff_ena",149,-1, &cfg_buf[0]);
                CPRINTF("************ serdes1g_dft_cfg2 ************\n");
                cfg_print("rx_jitter_ampl",148,139, &cfg_buf[0]);
                cfg_print("rx_step_freq",138,135, &cfg_buf[0]);
                cfg_print("rx_ji_ena",134,-1, &cfg_buf[0]);
                cfg_print("rx_waveform_sel",133,-1, &cfg_buf[0]);
                cfg_print("rx_freqoff_dir",132,-1, &cfg_buf[0]);
                cfg_print("rx_freqoff_ena",131,-1, &cfg_buf[0]);
                CPRINTF("************ serdes1g_tp_cfg ************\n");
                cfg_print("static_pattern",130,111, &cfg_buf[0]);
                CPRINTF("************ serdes1g_misc_cfg ************\n");
                cfg_print("pll_bist_ena",110,-1, &cfg_buf[0]);
                cfg_print("des_100fx_kick_mode",109,108, &cfg_buf[0]);
                cfg_print("des_100fx_cpmd_swap",107,-1, &cfg_buf[0]);
                cfg_print("des_100fx_cpmd_mode",106, -1, &cfg_buf[0]);
                cfg_print("des_100fx_cpmd_ena",105,-1, &cfg_buf[0]);
                cfg_print("rx_lpi_mode_ena",104,-1, &cfg_buf[0]);
                cfg_print("tx_lpi_mode_ena",103,-1, &cfg_buf[0]);
                cfg_print("rx_data_inv_ena",102,-1, &cfg_buf[0]);
                cfg_print("tx_data_inv_ena",101,-1, &cfg_buf[0]);
                cfg_print("lane_rst",100,-1, &cfg_buf[0]);
                CPRINTF("************ serdes1g_des_cfg ************\n");
                cfg_print("des_phs_ctrl",99,96, &cfg_buf[0]);
                cfg_print("des_cpmd_sel",95,94, &cfg_buf[0]);
                cfg_print("des_mbtr_ctrl",93,91, &cfg_buf[0]);
                cfg_print("des_bw_ana",90,88, &cfg_buf[0]);
                cfg_print("des_swap_ana",87,-1, &cfg_buf[0]);
                cfg_print("des_bw_hyst",86,84, &cfg_buf[0]);
                cfg_print("des_swap_hyst",83,-1, &cfg_buf[0]);
                CPRINTF("************ serdes1g_ib_cfg ************\n");
                cfg_print("ib_fx100_ena",82,-1, &cfg_buf[0]);
                cfg_print("acjtag_hyst",81,79, &cfg_buf[0]);
                cfg_print("ib_det_lev",78,76, &cfg_buf[0]);
                cfg_print("ib_hyst_lev",75,-1, &cfg_buf[0]);
                cfg_print("ib_ena_cmv_term",74, -1, &cfg_buf[0]);
                cfg_print("ib_ena_dc_coupling",73,-1, &cfg_buf[0]);
                cfg_print("ib_ena_detlev",72,-1, &cfg_buf[0]);
                cfg_print("ib_ena_hyst",71,-1, &cfg_buf[0]);
                cfg_print("ib_ena_offset_comp",70,-1, &cfg_buf[0]);
                cfg_print("ib_eq_gain",69,67, &cfg_buf[0]);
                cfg_print("ib_sel_corner_freq",66,65, &cfg_buf[0]);
                cfg_print("ib_resistor_ctrl",64,61, &cfg_buf[0]);
                CPRINTF("************ serdes1g_ob_cfg ************\n");
                cfg_print("ob_slp",60,59, &cfg_buf[0]);
                cfg_print("ob_amp_ctrl",58,55, &cfg_buf[0]);
                cfg_print("ob_cmm_bias_ctrl",54,52, &cfg_buf[0]);
                cfg_print("ob_dis_vcm_ctrl",51,-1, &cfg_buf[0]);
                cfg_print("ob_en_meas_vreg",50,-1, &cfg_buf[0]);
                cfg_print("ob_vcm_ctrl",49,46, &cfg_buf[0]);
                cfg_print("ob_resistor_ctrl",45,42, &cfg_buf[0]);
                CPRINTF("************ serdes1g_ser_cfg ************\n");
                cfg_print("ser_idle",41,-1, &cfg_buf[0]);
                cfg_print("ser_deemph",40,-1, &cfg_buf[0]);
                cfg_print("ser_cpmd_sel",39,-1, &cfg_buf[0]);
                cfg_print("ser_swap_cpmd",38,-1, &cfg_buf[0]);
                cfg_print("ser_alisel",37,36, &cfg_buf[0]);
                cfg_print("ser_enhys",35,-1, &cfg_buf[0]);
                cfg_print("ser_big_win",34,-1, &cfg_buf[0]);
                cfg_print("ser_en_win",33,-1, &cfg_buf[0]);
                cfg_print("ser_enali",32,-1, &cfg_buf[0]);
                CPRINTF("************ serdes1g_common_cfg ************\n");
                cfg_print("sys_rst",31,-1, &cfg_buf[0]);
                cfg_print("se_auto_squelch_b_ena",30,-1, &cfg_buf[0]);
                cfg_print("se_auto_squelch_a_ena",29,-1, &cfg_buf[0]);
                cfg_print("reco_sel_b",28,-1, &cfg_buf[0]);
                cfg_print("reco_sel_a",27,-1, &cfg_buf[0]);
                cfg_print("ena_lane",26,-1, &cfg_buf[0]);
                cfg_print("pwd_rx",25,-1, &cfg_buf[0]);
                cfg_print("pwd_tx",24,-1, &cfg_buf[0]);
                cfg_print("lane_ctrl",23,21, &cfg_buf[0]);
                cfg_print("ena_direct",20,-1, &cfg_buf[0]);
                cfg_print("ena_eloop",19,-1, &cfg_buf[0]);
                cfg_print("ena_floop",18,-1, &cfg_buf[0]);
                cfg_print("ena_iloop",17,-1, &cfg_buf[0]);
                cfg_print("ena_ploop",16,-1, &cfg_buf[0]);
                cfg_print("hrate",15,-1, &cfg_buf[0]);
                cfg_print("if_mode",14,-1, &cfg_buf[0]);
                CPRINTF("************ serdes1g_pll_cfg ************\n");
                cfg_print("pll_ena_fb_div2",13,-1, &cfg_buf[0]);
                cfg_print("pll_ena_rc_div2",12,-1, &cfg_buf[0]);
                cfg_print("pll_fsm_ctrl_data",11,4, &cfg_buf[0]);
                cfg_print("pll_fsm_ena",3,-1, &cfg_buf[0]);
                cfg_print("pll_fsm_force_set_ena",2,-1, &cfg_buf[0]);
                cfg_print("pll_fsm_oor_recal_ena",1,-1, &cfg_buf[0]);
                cfg_print("pll_rb_data_sel",0,-1, &cfg_buf[0]);
                CPRINTF("************ serdes6g_dft_status ************\n");
                cfg_print("pll_bist_not_done",17,-1, &stat_buf[0]);
                cfg_print("pll_bist_failed",16,-1, &stat_buf[0]);
                cfg_print("pll_bist_timeout_err",15,-1, &stat_buf[0]);
                cfg_print("bist_active",14,-1, &stat_buf[0]);
                cfg_print("bist_nosync",13,-1, &stat_buf[0]);
                cfg_print("bist_complete_n",12,-1, &stat_buf[0]);
                cfg_print("bist_error",11,-1, &stat_buf[0]);
                CPRINTF("************ serdes6g_pll_status ************\n");
                cfg_print("pll_cal_not_done",10,-1, &stat_buf[0]);
                cfg_print("pll_cal_err",9,-1, &stat_buf[0]);
                cfg_print("pll_out_of_range_err",8,-1, &stat_buf[0]);
                cfg_print("pll_rb_data",7,0, &stat_buf[0]);
            }
        } else {
            CPRINTF("Could not get settings \n");
        }
        
        // LCPLL/RComp setting
        mcb_bus = 2;
        if (vtss_phy_atom12_patch_setttings_get(NULL, iport, &mcb_bus, &cfg_buf[0], &stat_buf[0]) == VTSS_RC_OK) {            
            CPRINTF("************ LCPLLcfg ************\n");
            cfg_print("core_clk_div",170,165, &cfg_buf[0]);
            cfg_print("cpu_clk_div",164,159, &cfg_buf[0]);
            cfg_print("ena_bias",158, -1,&cfg_buf[0]);
            cfg_print("ena_vco_buf",157, -1,&cfg_buf[0]);
            cfg_print("ena_cp1",156, -1,&cfg_buf[0]);
            cfg_print("ena_vco_contrh",155, -1,&cfg_buf[0]);
            cfg_print("selcpi",154,153, &cfg_buf[0]);
            cfg_print("loop_bw_res",152,148, &cfg_buf[0]);
            cfg_print("selbgv820",147,144, &cfg_buf[0]);
            cfg_print("ena_lock_fine",143, -1, &cfg_buf[0]);
            cfg_print("div4",142, -1,&cfg_buf[0]);
            cfg_print("ena_clktree",141, -1,&cfg_buf[0]);
            cfg_print("ena_lane",140, -1,&cfg_buf[0]);
            cfg_print("ena_rot",139, -1,&cfg_buf[0]);
            cfg_print("force_set_ena",138, -1,&cfg_buf[0]);
            cfg_print("half_rate",137, -1,&cfg_buf[0]);
            cfg_print("out_of_range_recal_ena",136, -1,&cfg_buf[0]);
            cfg_print("pwd_rx",135, -1,&cfg_buf[0]);
            cfg_print("pwd_tx",134, -1,&cfg_buf[0]);
            cfg_print("quarter_rate",133, -1,&cfg_buf[0]);
            cfg_print("rc_ctrl_data",132,125, &cfg_buf[0]);
            cfg_print("rc_enable",124, -1,&cfg_buf[0]);
            cfg_print("readback_data_sel",123, -1,&cfg_buf[0]);
            cfg_print("rot_dir",122, -1,&cfg_buf[0]);
            cfg_print("rot_speed",121, -1,&cfg_buf[0]);
            cfg_print("ena_direct",120, -1,&cfg_buf[0]);
            cfg_print("ena_gain_test",119, -1,&cfg_buf[0]);
            cfg_print("disable_fsm",118, -1,&cfg_buf[0]);
            cfg_print("en_reset_frq_det",117, -1,&cfg_buf[0]);
            cfg_print("en_reset_lim_det",116, -1,&cfg_buf[0]);
            cfg_print("en_reset_overrun",115, -1,&cfg_buf[0]);
            cfg_print("gain_test",114,110, &cfg_buf[0]);
            cfg_print("disable_fsm_por",109, -1,&cfg_buf[0]);
            cfg_print("frc_fsm_por",108, -1,&cfg_buf[0]);
            cfg_print("ampc_sel",107,100, &cfg_buf[0]);
            cfg_print("ena_amp_ctrl_force",99, -1,&cfg_buf[0]);
            cfg_print("ena_ampctrl",98, -1,&cfg_buf[0]);
            cfg_print("pwd_ampctrl_n",97, -1,&cfg_buf[0]);
            cfg_print("ena_clk_bypass",96, -1,&cfg_buf[0]);
            cfg_print("ena_clk_bypass1",95, -1,&cfg_buf[0]);
            cfg_print("ena_cp2",94, -1,&cfg_buf[0]);
            cfg_print("ena_rcpll",93, -1,&cfg_buf[0]);
            cfg_print("ena_fbtestout",92, -1,&cfg_buf[0]);
            cfg_print("ena_vco_nref_testout",91, -1,&cfg_buf[0]);
            cfg_print("ena_pfd_in_flip",90, -1,&cfg_buf[0]);
            cfg_print("ena_test_mode",89, -1,&cfg_buf[0]);
            cfg_print("fbdivsel",88,81, &cfg_buf[0]);
            cfg_print("fbdivsel_tst_ena",80, -1,&cfg_buf[0]);
            cfg_print("force_cp",79, -1,&cfg_buf[0]);
            cfg_print("force_ena",78, -1,&cfg_buf[0]);
            cfg_print("force_hi",77, -1,&cfg_buf[0]);
            cfg_print("force_lo",76, -1,&cfg_buf[0]);
            cfg_print("force_vco_contrh",75, -1,&cfg_buf[0]);
            cfg_print("rst_fb_n",74, -1,&cfg_buf[0]);
            cfg_print("sel_cml_cmos_pfd",73, -1,&cfg_buf[0]);
            cfg_print("sel_fbdclk",72, -1,&cfg_buf[0]);
            cfg_print("ena_test_out",71, -1,&cfg_buf[0]);
            cfg_print("ena_ana_test_out",70, -1,&cfg_buf[0]);
            cfg_print("testout_sel",69,67, &cfg_buf[0]);
            cfg_print("test_ana_out_sel",66,65, &cfg_buf[0]);
            cfg_print("ib_ctrl",64,49, &cfg_buf[0]);
            cfg_print("ib_bias_ctrl",48,41, &cfg_buf[0]);
            cfg_print("ob_ctrl",40,25, &cfg_buf[0]);
            cfg_print("ob_bias_ctrl",24,17, &cfg_buf[0]);
            CPRINTF("************ RCompcfg ************\n");
            cfg_print("pwd_ena",16, -1,&cfg_buf[0]);
            cfg_print("run_cal",15, -1,&cfg_buf[0]);
            cfg_print("speed_sel",14,13, &cfg_buf[0]);
            cfg_print("mode_sel",12,11, &cfg_buf[0]);
            cfg_print("force_ena",10, -1,&cfg_buf[0]);
            cfg_print("rcomp_val",9,6, &cfg_buf[0]);
            CPRINTF("************ SyncEthcfg ************\n");
            cfg_print("sel_reco_clk_b",5,4, &cfg_buf[0]);
            cfg_print("sel_reco_clk_a",3,2, &cfg_buf[0]);
            cfg_print("reco_clk_b_ena",1, -1,&cfg_buf[0]);
            cfg_print("reco_clk_a_ena",0, -1,&cfg_buf[0]);
            CPRINTF("************ LCPLLstatus ************\n");
            cfg_print("lock_status",45,-1, &stat_buf[0]);
            cfg_print("readback_data",44,37, &stat_buf[0]);
            cfg_print("calibration_done",36, -1, &stat_buf[0]);
            cfg_print("calibration_err",35, -1, &stat_buf[0]);
            cfg_print("out_of_range_err",34, -1, &stat_buf[0]);
            cfg_print("range_lim",33, -1, &stat_buf[0]);
            cfg_print("fsm_lock",32, -1, &stat_buf[0]);
            cfg_print("fsm_stat",31,29, &stat_buf[0]);
            cfg_print("fbcnt_dif",28,19, &stat_buf[0]);
            cfg_print("gain_stat",18,14, &stat_buf[0]);
            cfg_print("sig_del",13,6, &stat_buf[0]);
            CPRINTF("************ RCompstatus ************\n");
            cfg_print("busy",5, -1, &stat_buf[0]);
            cfg_print("delta_alert",4, -1, &stat_buf[0]);
            cfg_print("rcomp",3,0, &stat_buf[0]);

        } else {
            CPRINTF("Could not get patch settings \n");
        }
    }
}

static void misc_cli_cmd_debug_ib_cterm(cli_req_t * req)
{
    vtss_port_no_t iport, uport;
    misc_cli_req_t   *misc_req = req->module_req;
    for (iport = VTSS_PORT_NO_START; iport < VTSS_PORT_NO_END; iport++) {
        uport = iport2uport(iport);
        if (req->uport_list[uport] == 0)
            continue;
        
        if (vtss_phy_cfg_ib_cterm(NULL, iport, misc_req->ib_cterm_value, misc_req->ib_eq_mode) == VTSS_RC_OK) {
            CPRINTF("cterm set \n");
        } else {
            CPRINTF("cterm not set \n");
        }
    }
}

static void misc_cli_resume ( cli_req_t * req )
{
    if (misc_suspend_resume(req->stack.isid_debug, 1) != VTSS_OK)
        CPRINTF("Resume failed\n");
}

static void misc_cli_assert(cli_req_t *req)
{
    misc_cli_req_t *misc_req = req->module_req;
    u32 *addr = NULL;

    switch (misc_req->assert_type) {
    case 1:
        // Application Assertion
        cli_printf("Generating application assertion...\n");
        VTSS_ASSERT(0);
        break;

    case 2:
        // eCos Assertion
        cli_printf("Generating OS assertion...\n");
        // Avoid Lint Warning 506: Constant value Boolean
        /*lint -e{506} */
        CYG_ASSERT(0, "Artificial OS assertion");
        break;

    case 3:
        // Exception. De-reference NULL-pointer to test PC gets into boot data.
        cli_printf("Generating exception...\n");
        // Avoid Lint Warning 413: Likely use of null pointer 'addr' in argument to operator 'unary *'
        /*lint -e{413} */
        if (*addr) {
            cli_printf("."); // Unreachable;
        }
        break;

    default:
        cli_printf("Error: Unknown command\n");
        break;
    }
}

static void misc_cli_suspend ( cli_req_t * req )
{
    if (misc_suspend_resume(req->stack.isid_debug, 0) != VTSS_OK)
        CPRINTF("Suspend failed\n");
}

static void misc_cli_cmd_debug_critd_list ( cli_req_t * req )
{
    vtss_module_id_t mid;
    BOOL             header = 1;
    misc_cli_req_t   *misc_req = req->module_req;

    if (misc_req->module_all) {
        critd_dbg_list(cli_printf, VTSS_MODULE_ID_NONE, misc_req->detailed, 1, NULL);
    } else {
        for (mid = 0; mid < VTSS_MODULE_ID_NONE; mid++) {
            if (misc_req->module_list[mid] == 0)
                continue;
            
            critd_dbg_list(cli_printf, mid, misc_req->detailed, header, NULL);
            if (!misc_req->detailed)
                header = 0;
        }
    }
}

// Setting phy page chk
static void misc_cli_cmd_debug_phy_do_page_chk(cli_req_t *req) {
    BOOL enabled;
    misc_cli_req_t   *misc_req = req->module_req;
    if (req->set) {
        if (vtss_phy_do_page_chk_set(NULL, misc_req->enable) != VTSS_RC_OK) {
            T_E("Could not set do page set");
        }
    } else {
        if (vtss_phy_do_page_chk_get(NULL, &enabled) != VTSS_RC_OK) {
            CPRINTF("Could not get do page check configuration \n");
        } else {
            CPRINTF("Do page check is %s \n", enabled ? "enabled" : "disabled");
        }
    }
}


// For debug - Shows PHY statistics, See vtss_ophy_api.h.
static void misc_cli_cmd_debug_phy_statistic(cli_req_t *req)
{
    vtss_port_no_t port_no;
    BOOL                    first = TRUE;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        if (req->uport_list[iport2uport(port_no)] == 0) {
            continue;
        }
         
        if (!req->set) {// No Set for the function
            if (first) {
                (void) vtss_phy_debug_stat_print(NULL, (vtss_debug_printf_t)cli_printf, port_no, TRUE);
                first = 0;
            }           
            (void) vtss_phy_debug_stat_print(NULL, (vtss_debug_printf_t)cli_printf, port_no, FALSE);
        }
    }
}


// Setting / Getting 1G loopback.
static void misc_cli_cmd_debug_phy_loopback(cli_req_t *req)
{
    misc_cli_req_t   *misc_req = req->module_req;
    vtss_port_no_t port_no, port;
    vtss_phy_loopback_t lb;
    BOOL                    first = 1;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if (req->uport_list[port] == 0) {
            continue;
        }
               
        if (req->set) {
            lb.far_end_enable = misc_req->enable && misc_req->lb_type == VTSS_LB_FAR_END;
            lb.near_end_enable = misc_req->enable && misc_req->lb_type == VTSS_LB_NEAR_END;
            if (vtss_phy_loopback_set(MISC_INST, port_no, lb) != VTSS_RC_OK) {
                CPRINTF("Could not perform vtss_phy_loopback_set() for port %u\n",port_no);
                continue;
            }
        } else {
            if (vtss_phy_loopback_get(MISC_INST, port_no, &lb) != VTSS_RC_OK) {
                CPRINTF("Could not perform vtss_phy_loopback_get() for port %u\n",port_no);
                continue;
            }

            if (first) {
                CPRINTF("%-12s %-12s %-12s\n","Port","Near End Loopback", "Far End Loopback");
                CPRINTF("%-12s %-12s %-12s\n","----","-----------------", "----------------");
                first = 0;
            }           

            CPRINTF("%-12u %-18s %-18s\n",
                    port, 
                    lb.near_end_enable ? "Enabled" : "Disabled",
                    lb.far_end_enable ? "Enabled" : "Disabled");
        }
    }
}


#if defined(VTSS_CHIP_10G_PHY)
static void misc_cli_cmd_phy_10g_fw_status(cli_req_t *req)
{
    vtss_port_no_t port_no,  port;
    vtss_phy_10g_fw_status_t status; 
    vtss_phy_10g_id_t        chip_id;
    BOOL                     first=1;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);

        if ((req->uport_list[port] == 0) || !vtss_phy_10G_is_valid(MISC_INST, port_no)) {
            continue;
        }
        if (vtss_phy_10g_id_get(MISC_INST, port_no, &chip_id) != VTSS_RC_OK) {
            CPRINTF("Could not perform vtss_phy_10g_mode_get() operation");
            continue;
        }
        if ((chip_id.part_number != 0x8488) && (chip_id.part_number != 0x8487) && (chip_id.part_number != 0x8484)) {
            CPRINTF("Not supported on this chip\n");
            continue;
        }
        if (chip_id.channel_id != 0) {
            CPRINTF("This command is only supported through a port which represent Phy channel 0\n");
            continue;
        }
        if (vtss_phy_10g_edc_fw_status_get(MISC_INST, port_no, &status) != VTSS_RC_OK) {
            CPRINTF("Could not perform up load %u\n",port_no);
            continue;
        }            
        if (first) {
            CPRINTF("%-10s %-10s %-10s %-16s %-10s %-10s %-10s\n","CLI Port","API Port","Channel","Loaded via API","FW-Rev","Chksum","CPU activity");
            CPRINTF("%-10s %-10s %-10s %-16s %-10s %-10s %-10s\n","--------","--------","-------","-------------","------","------","-----------");
            first = 0;
        }        
        CPRINTF("%-10u %-10u %-10u %-16s %-10x %-10s %-10s\n",port,port-1,
                chip_id.channel_id, status.edc_fw_api_load?"Yes":"No", 
                status.edc_fw_rev, status.edc_fw_chksum?"Pass":"Fail", status.icpu_activity?"Yes":"No");
    }
}


static void misc_cli_cmd_phy_10g_mode(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_phy_10g_mode_t     mode;
    vtss_phy_10g_id_t       chip_id;
    misc_cli_req_t          *misc_req = req->module_req;
    BOOL                    first = 1;
    
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->uport_list[port] == 0) || !vtss_phy_10G_is_valid(MISC_INST, port_no)) 
            continue;

        if (vtss_phy_10g_id_get(MISC_INST, port_no, &chip_id) != VTSS_RC_OK) {
            CPRINTF("Could not perform vtss_phy_10g_mode_get() operation");
            continue;
        }

        if (req->set) {
            if (vtss_phy_10g_mode_get(MISC_INST, port_no, &mode) != VTSS_RC_OK) {
                CPRINTF("Could not perform vtss_phy_10g_mode_get() operation");
                continue;
            }

            if (misc_req->lan == 1) {
                mode.oper_mode = VTSS_PHY_LAN_MODE;
                mode.xaui_lane_flip = 0;
            } else if (misc_req->lan == 0) {
                mode.oper_mode = VTSS_PHY_WAN_MODE;
                mode.xaui_lane_flip = 0;
            } else {
                mode.oper_mode = VTSS_PHY_1G_MODE;
                /* The Serdes XAUI lane 0 must match phy lane 0 */
                mode.xaui_lane_flip = 1; 
            }
            if (vtss_phy_10g_mode_set(MISC_INST, port_no, &mode) != VTSS_RC_OK) {
                CPRINTF("Could not perform vtss_phy_10g_mode_get() operation");
                continue;
            }
#ifdef VTSS_FEATURE_PHY_TIMESTAMP
            if (chip_id.part_number == 0x8488 || chip_id.part_number == 0x8487) {
                if (vtss_phy_ts_phy_oper_mode_change(MISC_INST, port_no) != VTSS_RC_OK) {
                    CPRINTF("Could not perform vtss_phy_ts_phy_oper_mode_change() operation");
                    continue;
                }
            }
#endif /* VTSS_FEATURE_PHY_TIMESTAMP */
        } else {        
            if (vtss_phy_10g_mode_get(MISC_INST, port_no, &mode) != VTSS_RC_OK) {
                CPRINTF("Could not perform vtss_phy_10g_mode_get() operation");
                continue;
            }
           
            if (first) {
                CPRINTF("%-10s %-10s %-10s %-10s %-10s %-10s %-10s %-10s\n","CLI Port","API Port","Channel","Base","WAN/LAN/1G","Wrefclk","Phy Id","Rev.");
                CPRINTF("%-10s %-10s %-10s %-10s %-10s %-10s %-10s %-10s\n","--------","--------","-------","-----","-------","-------","-------","---");
                first = 0;
            }
                       
            CPRINTF("%-10u %-10u %-10u %-10u %-10s %-10s %-10x %-10x\n",port,port-1,
                    chip_id.channel_id,
                    chip_id.phy_api_base_no,
                    mode.oper_mode == VTSS_PHY_WAN_MODE ? "WAN" : mode.oper_mode == VTSS_PHY_LAN_MODE ? "LAN" : "1G", 
                    mode.oper_mode == VTSS_PHY_WAN_MODE ? (mode.wrefclk == VTSS_WREFCLK_155_52 ? "155.02" : "622.08") : "-", 
                    chip_id.part_number, chip_id.revision);
        }
    }
}

static void misc_cli_cmd_phy_10g_status(cli_req_t * req)
{
    vtss_port_no_t port_no, port;
    vtss_phy_10g_status_t   status;
    vtss_phy_10g_mode_t     mode;
    vtss_phy_10g_cnt_t      cnt;
    BOOL                    first = 1;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->uport_list[port] == 0) || !vtss_phy_10G_is_valid(MISC_INST, port_no)) 
            continue;
       
        if (vtss_phy_10g_status_get(MISC_INST, port_no, &status) != VTSS_RC_OK) {
            CPRINTF("Could not perform vtss_phy_10g_status_get() operation");
            continue;
        }
        if (vtss_phy_10g_mode_get(MISC_INST, port_no, &mode) != VTSS_RC_OK) {
            CPRINTF("Could not perform vtss_phy_10g_mode_get() operation");
            continue;
        }

        CPRINTF("Port: %u\n",port);
        CPRINTF("--------\n");
        if (first) {
            CPRINTF("%-12s %-12s %-16s %-12s %-12s\n","","Link","Link-down-event","Rx-Fault-Sticky","Tx-Fault-Sticky");
            first = 0;
        }           
        CPRINTF("%-12s %-12s %-16s %-12s %-12s\n","PMA:",status.pma.rx_link?"Yes":"No",
               status.pma.link_down?"Yes":"No",status.pma.rx_fault?"Yes":"No",status.pma.tx_fault?"Yes":"No");
        CPRINTF("%-12s %-12s %-16s %-12s %-12s\n","WIS:",mode.oper_mode == VTSS_PHY_WAN_MODE ? status.wis.rx_link?"Yes":"No" : "-",
               mode.oper_mode == VTSS_PHY_WAN_MODE ? status.wis.link_down?"Yes":"No":"-",mode.oper_mode == VTSS_PHY_WAN_MODE ? status.wis.rx_fault?"Yes":"No":"-",
               mode.oper_mode == VTSS_PHY_WAN_MODE ? status.wis.tx_fault?"Yes":"No":"-");
        CPRINTF("%-12s %-12s %-16s %-12s %-12s\n","PCS:",status.pcs.rx_link?"Yes":"No",
               status.pcs.link_down?"Yes":"No",status.pcs.rx_fault?"Yes":"No",status.pcs.tx_fault?"Yes":"No");
        CPRINTF("%-12s %-12s %-16s %-12s %-12s\n","XS:",status.xs.rx_link?"Yes":"No",
               status.xs.link_down?"Yes":"No",status.xs.rx_fault?"Yes":"No",status.xs.tx_fault?"Yes":"No");

        if (vtss_phy_10g_cnt_get(MISC_INST, port_no, &cnt) != VTSS_RC_OK) {
            CPRINTF("Could not perform vtss_phy_10g_cnt_get() operation");
            continue;
        }
        CPRINTF("\n");
        CPRINTF("PCS counters:\n");
        CPRINTF("%-20s %-12s\n", "  Block_lacthed:",cnt.pcs.block_lock_latched?"Yes":"No");
        CPRINTF("%-20s %-12s\n", "  High_ber_latched:",cnt.pcs.high_ber_latched?"Yes":"No");
        CPRINTF("%-20s %-12d\n", "  Ber_cnt:",cnt.pcs.ber_cnt);
        CPRINTF("%-20s %-12d\n", "  Err_blk_cnt:",cnt.pcs.err_blk_cnt);                
    }
}

static void misc_cli_cmd_phy_10g_reset(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_phy_10g_mode_t     oper_mode;
    /* Configure the 10G phy operating mode */    
    memset(&oper_mode, 0, sizeof(oper_mode));

#if defined(VTSS_ARCH_JAGUAR_1)
    /* Assuming Ref boards */
    oper_mode.oper_mode = VTSS_PHY_LAN_MODE;
    oper_mode.high_input_gain = 0;
    oper_mode.interface = VTSS_PHY_XAUI_XFI;
    oper_mode.xfi_pol_invert = 1; /* Invert the XFI data polarity */
#endif /* VTSS_ARCH_JAGUAR_1 */

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->uport_list[port] == 0) || !vtss_phy_10G_is_valid(MISC_INST, port_no)) 
            continue;        
        CPRINTF("Resetting and setting up phy:%u\n",port_no);
        if (vtss_phy_10g_reset(MISC_INST, port_no) != VTSS_RC_OK) {
            CPRINTF("Could not perform a PHY reset port:%u\n",port_no);
        }

        if (vtss_phy_10g_mode_set(0, port_no, &oper_mode) != VTSS_RC_OK) {
            T_E("Could not set the 10g Phy operating mode (port %u)", port_no);
        }                                
    }
}

static void misc_cli_cmd_phy_10g_loopback(cli_req_t *req)
{
    misc_cli_req_t   *misc_req = req->module_req;
    vtss_port_no_t port_no, port;
    vtss_phy_10g_loopback_t lb;
    BOOL                    first = 1;

    lb.lb_type = misc_req->lb=='b'?VTSS_LB_SYSTEM_XS_SHALLOW:
        misc_req->lb=='c'?VTSS_LB_SYSTEM_XS_DEEP:
        misc_req->lb=='e'?VTSS_LB_SYSTEM_PCS_SHALLOW:
        misc_req->lb=='g'?VTSS_LB_SYSTEM_PCS_DEEP:
        misc_req->lb=='j'?VTSS_LB_SYSTEM_PMA:
        misc_req->lb=='d'?VTSS_LB_NETWORK_XS_SHALLOW:
        misc_req->lb=='a'?VTSS_LB_NETWORK_XS_DEEP:
        misc_req->lb=='f'?VTSS_LB_NETWORK_PCS:
        misc_req->lb=='h'?VTSS_LB_NETWORK_WIS:
        misc_req->lb=='k'?VTSS_LB_NETWORK_PMA:'N';           
    
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->uport_list[port] == 0) || !vtss_phy_10G_is_valid(MISC_INST, port_no)) 
            continue;
               
        if (req->set) {
            lb.enable = misc_req->enable;
            if (vtss_phy_10g_loopback_set(MISC_INST, port_no, &lb) != VTSS_RC_OK) {
                CPRINTF("Could not perform vtss_phy_10g_loopback_set() for port %u\n",port_no);
                continue;
            }
        } else {
            if (vtss_phy_10g_loopback_get(MISC_INST, port_no, &lb) != VTSS_RC_OK) {
                CPRINTF("Could not perform vtss_phy_10g_loopback_get() for port %u\n",port_no);
                continue;
            }

            if (first) {
                CPRINTF("%-12s %-12s\n","Port","Loopback");
                CPRINTF("%-12s %-12s\n","----","--------");
                first = 0;
            }           

            CPRINTF("%-12u %-12s\n",port, lb.lb_type==VTSS_LB_SYSTEM_XS_SHALLOW?"b":
                   lb.lb_type==VTSS_LB_SYSTEM_XS_DEEP?"c":
                   lb.lb_type==VTSS_LB_SYSTEM_PCS_SHALLOW?"e":
                   lb.lb_type==VTSS_LB_SYSTEM_PCS_DEEP?"g":
                   lb.lb_type==VTSS_LB_SYSTEM_PMA?"j":
                   lb.lb_type==VTSS_LB_NETWORK_XS_SHALLOW?"d":
                   lb.lb_type==VTSS_LB_NETWORK_XS_DEEP?"a":
                   lb.lb_type==VTSS_LB_NETWORK_PCS?"f":
                   lb.lb_type==VTSS_LB_NETWORK_WIS?"h":
                   lb.lb_type==VTSS_LB_NETWORK_PMA?"k":"None");           
        }
    }
}

static void misc_cli_cmd_phy_10g_power(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    vtss_phy_10g_power_t power;
    misc_cli_req_t   *misc_req = req->module_req;
    BOOL            first = 1;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->uport_list[port] == 0) || !vtss_phy_10G_is_valid(MISC_INST, port_no)) 
            continue;
      
        if (req->set) {
            if (misc_req->enable) {
                power = VTSS_PHY_10G_POWER_ENABLE;
            } else {
                power = VTSS_PHY_10G_POWER_DISABLE;
            }
            if (vtss_phy_10g_power_set(MISC_INST, port_no, &power) != VTSS_RC_OK) {
                CPRINTF("Could not perform vtss_phy_10g_power_set() for port %u\n",port_no);
                continue;
            }
        } else {
            if (vtss_phy_10g_power_get(MISC_INST, port_no, &power) != VTSS_RC_OK) {
                CPRINTF("Could not perform vtss_phy_10g_power_get() for port %u\n",port_no);
                continue;
            }

            if (first) {
                CPRINTF("%-12s %-12s\n","Port","Power");
                CPRINTF("%-12s %-12s\n","----","-----");
                first = 0;
            }                       
            CPRINTF("%-12u %-12s\n",port, power == VTSS_PHY_10G_POWER_ENABLE ? "Enabled":"Disabled");          
        }
    }
}

static void misc_cli_cmd_phy_10g_failover(cli_req_t *req)
{
    misc_cli_req_t   *misc_req = req->module_req;
    vtss_port_no_t port_no, port;
    vtss_phy_10g_failover_mode_t mode;
    BOOL                    first = 1;

    mode = misc_req->failover=='a'?VTSS_PHY_10G_PMA_TO_FROM_XAUI_NORMAL:
        misc_req->failover=='b'?VTSS_PHY_10G_PMA_TO_FROM_XAUI_CROSSED:
        misc_req->failover=='c'?VTSS_PHY_10G_PMA_0_TO_FROM_XAUI_0_TO_XAUI_1:
        misc_req->failover=='d'?VTSS_PHY_10G_PMA_0_TO_FROM_XAUI_1_TO_XAUI_0:
        misc_req->failover=='e'?VTSS_PHY_10G_PMA_1_TO_FROM_XAUI_0_TO_XAUI_1:
        misc_req->failover=='f'?VTSS_PHY_10G_PMA_1_TO_FROM_XAUI_1_TO_XAUI_0:
        VTSS_PHY_10G_PMA_TO_FROM_XAUI_NORMAL;
    
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->uport_list[port] == 0) || !vtss_phy_10G_is_valid(MISC_INST, port_no)) 
            continue;
               
        if (req->set) {
            if (vtss_phy_10g_failover_set(MISC_INST, port_no, &mode) != VTSS_RC_OK) {
                CPRINTF("Could not perform vtss_phy_10g_failover_set() for port %u\n",port_no);
                continue;
            }
        } else {
            if (vtss_phy_10g_failover_get(MISC_INST, port_no, &mode) != VTSS_RC_OK) {
                CPRINTF("Could not perform vtss_phy_10g_failover_get() for port %u\n",port_no);
                continue;
            }

            if (first) {
                CPRINTF("%-12s %-12s\n","Port","Failover");
                CPRINTF("%-12s %-12s\n","----","--------");
                first = 0;
            }

            CPRINTF("%-12u %-12s\n",port, mode==VTSS_PHY_10G_PMA_TO_FROM_XAUI_NORMAL?"a":
                   mode==VTSS_PHY_10G_PMA_TO_FROM_XAUI_CROSSED?"b":
                   mode==VTSS_PHY_10G_PMA_0_TO_FROM_XAUI_0_TO_XAUI_1?"c":
                   mode==VTSS_PHY_10G_PMA_0_TO_FROM_XAUI_1_TO_XAUI_0?"d":
                   mode==VTSS_PHY_10G_PMA_1_TO_FROM_XAUI_0_TO_XAUI_1?"e":
                   mode==VTSS_PHY_10G_PMA_1_TO_FROM_XAUI_1_TO_XAUI_0?"f":"dunno");           
        }
    }
}

static void misc_cli_cmd_debug_csr_write ( cli_req_t * req )
{
    misc_cli_req_t     *misc_req = req->module_req;
    req->set = 1;
    misc_req->phy_mmd_access = TRUE;

    vtss_port_no_t port_no, port;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);

        if (req->uport_list[port] == 0){
            continue;
        } 
        if (vtss_phy_10g_csr_write(MISC_INST, port_no, misc_req->devad,
                                   req->addr, req->value) != VTSS_RC_OK) {
            CPRINTF("Could not perform vtss_macsec_csr_write() operation \n");
            continue;
        }
    }
}

static void misc_cli_cmd_debug_csr_read ( cli_req_t * req )
{
    misc_cli_req_t     *misc_req = req->module_req;
    req->set = 1;
    misc_req->phy_mmd_access = TRUE;
    u32 value;

    vtss_port_no_t port_no, port;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);

        if (req->uport_list[port] == 0){
            continue;
        } 
        if (vtss_phy_10g_csr_read(MISC_INST, port_no, misc_req->devad,
                                  req->addr, &value) != VTSS_RC_OK) {
            CPRINTF("Phy read failed\n");
        } else {
            CPRINTF("0x%08x\n", value);
        }
    }
}

static void misc_cli_cmd_phy_10g_gpio(cli_req_t *req, BOOL role, BOOL read, BOOL write)
{
    vtss_gpio_no_t gpio;
    vtss_port_no_t port_no, port;
    BOOL           first = 1, value;
    misc_cli_req_t   *misc_req = req->module_req;
    vtss_gpio_10g_gpio_mode_t  gpio_mode;
    
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->uport_list[port] == 0) || !vtss_phy_10G_is_valid(MISC_INST, port_no)) 
            continue;
        for (gpio = 0; gpio < 12; gpio++) {
            if (misc_req->gpio_list[gpio] == 0)
                continue;
            if (role) {
                gpio_mode.port = port_no;
                if (misc_req->gpio_role == 1) {
                    gpio_mode.mode = VTSS_10G_PHY_GPIO_OUT;
                    if (vtss_phy_10g_gpio_mode_set(MISC_INST, port_no, gpio, &gpio_mode)!= VTSS_RC_OK) {
                        CPRINTF("Could not perform GPIO operation for port %u\n",port_no);
                    }
                } else if (misc_req->gpio_role == 0) {
                    gpio_mode.mode = VTSS_10G_PHY_GPIO_IN;
                    if (vtss_phy_10g_gpio_mode_set(MISC_INST, port_no, gpio, &gpio_mode)!= VTSS_RC_OK) {
                        CPRINTF("Could not perform GPIO operation for port %u\n",port_no);
                    }
                } else {
                    gpio_mode.mode = VTSS_10G_PHY_GPIO_PCS_RX_FAULT;
                    if (vtss_phy_10g_gpio_mode_set(MISC_INST, port_no, gpio, &gpio_mode)!= VTSS_RC_OK) {
                        CPRINTF("Could not perform GPIO operation for port %u\n",port_no);
                    }
                }
            } else if (read) {
                if (vtss_phy_10g_gpio_read(MISC_INST, port_no, gpio, &value) != VTSS_RC_OK) {
                    CPRINTF("Could not perform GPIO operation for port %u\n",port_no);
                }

                if (first)
                    cli_table_header("GPIO  Value");
                first = 0;
                CPRINTF("%-4u  %u\n", gpio, value);
            } else if (write) {
                if (vtss_phy_10g_gpio_write(MISC_INST, port_no, gpio, req->value) != VTSS_RC_OK) {
                    CPRINTF("Could not perform GPIO operation for port %u\n",port_no);
                }
            } 
        }
    }
}

static void misc_cli_cmd_phy_10g_gpio_role(cli_req_t *req)
{
    misc_cli_cmd_phy_10g_gpio(req, 1, 0, 0);
}
static void misc_cli_cmd_phy_10g_gpio_read(cli_req_t *req)
{
    misc_cli_cmd_phy_10g_gpio(req, 0, 1, 0);
}static void misc_cli_cmd_phy_10g_gpio_write(cli_req_t *req)
{
    misc_cli_cmd_phy_10g_gpio(req, 0, 0, 1);
}
#endif /* VTSS_CHIP_10G_PHY */

#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
static void misc_cli_cmd_debug_phy_1588_mmd_read ( cli_req_t * req )
{
    misc_cli_req_t     *misc_req = req->module_req;
    req->set = 0;
    misc_req->phy_mmd_access = TRUE;
    ulong addr;

    vtss_port_no_t port_no, port;
    BOOL               first = 1;
    u32                value = 0;

    CPRINTF(" 1G: Port 24 : Channel-0 :: Port 25 : Channel-1 etc.\n");
    CPRINTF(" 10G: Port 25 : Channel-1 :: Port 26 : Channel-0\n");
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        
#if 0
        if ((req->uport_list[port] == 0) || !vtss_phy_10G_is_valid(MISC_INST, port_no)) {
            continue;
        }
#endif /* 0 */
        if (req->uport_list[port] == 0) {
            continue;
        }
        for (addr = 0; addr < CLI_ADDR_MAX; addr++) {
            if (VTSS_BF_GET(misc_addr_bf, addr) == 0)
                continue;
            
        
            T_D("port_no : %u Blk_id : %d Offset:%x\n\r",
                              port_no,(int)misc_req->blk_id_1588, misc_req->csr_reg_offset_1588);

            if (vtss_phy_1588_csr_reg_read(MISC_INST, port_no, misc_req->blk_id_1588,
                                           addr, &value) != VTSS_RC_OK) {
                CPRINTF("Could not perform vtss_phy_1588_csr_reg_read() operation \n");
                continue;
            }


            if (first) {
                CPRINTF("%-12s %-12s %-12s %-12s \n","Port","Blk-Id","CSR-Offset","Value");
                CPRINTF("%-12s %-12s %-12s %-12s \n","----","------","----------","-----");
                first = 0;
            }

            CPRINTF("%-12lu %-12u %-12x %-12x\n",(long int)port, (unsigned int)misc_req->blk_id_1588,
                             (unsigned int)addr, (unsigned int)value);
        }
    }
}

static void misc_cli_cmd_debug_phy_1588_mmd_write ( cli_req_t * req )
{
    misc_cli_req_t     *misc_req = req->module_req;
    req->set = 1;
    misc_req->phy_mmd_access = TRUE;

    vtss_port_no_t port_no, port;

    CPRINTF(" Port 25 : Channel-0 :: Port 26 : Channel-1 \n");
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
#if 0
        if ((req->uport_list[port] == 0) || !vtss_phy_10G_is_valid(MISC_INST, port_no))
            continue;
#endif /* 0 */
        if (req->uport_list[port] == 0){
            continue;
        } 
        T_D("port_no : %u Blk_id : %d Offset:%x, Value : %x\n\r",
                          port_no,(int)misc_req->blk_id_1588, misc_req->csr_reg_offset_1588, req->value);
        if (vtss_phy_1588_csr_reg_write(MISC_INST, port_no, misc_req->blk_id_1588,
                        misc_req->csr_reg_offset_1588, &req->value) != VTSS_RC_OK) {
            CPRINTF("Could not perform vtss_phy_1588_csr_reg_write() operation \n");
            continue;
        }
    }
}
#endif /* VTSS_FEATURE_PHY_TIMESTAMP */
#ifdef VTSS_FEATURE_MACSEC
static void misc_cli_cmd_debug_macsec_write ( cli_req_t * req )
{
    misc_cli_req_t     *misc_req = req->module_req;
    req->set = 1;
    misc_req->phy_mmd_access = TRUE;

    vtss_port_no_t port_no, port;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);

        if (req->uport_list[port] == 0){
            continue;
        } 

        if (vtss_macsec_csr_write(MISC_INST, port_no, misc_req->macsec_blk,
                        misc_req->macsec_addr, req->value) != VTSS_RC_OK) {
            CPRINTF("Could not perform vtss_macsec_csr_write() operation \n");
            continue;
        }
    }
}

static void misc_cli_cmd_debug_macsec_read ( cli_req_t * req )
{
    misc_cli_req_t     *misc_req = req->module_req;
    req->set = 0;
    misc_req->phy_mmd_access = TRUE;
    vtss_port_no_t port_no, port;
    u32                value = 0;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        
        if (req->uport_list[port] == 0) {
            continue;
        }

        if (vtss_macsec_csr_read(MISC_INST, port_no, misc_req->macsec_blk, misc_req->macsec_addr, &value) != VTSS_RC_OK) {
            CPRINTF("Could not perform vtss_macsec_csr_read() operation \n");
            continue;
        }
        CPRINTF("0x%x\n",value);
     }
}

static void misc_cli_cmd_debug_macsec_dump ( cli_req_t * req )
{
    misc_cli_req_t     *misc_req = req->module_req;
    req->set = 0;
    misc_req->phy_mmd_access = TRUE;
    vtss_port_no_t port_no, port;
    vtss_macsec_mac_counters_t counters;
    vtss_macsec_init_t         init;
    u32 a, i, value, blk, sa;

     for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
         port = iport2uport(port_no);        
         if (req->uport_list[port] == 0) {
             continue;
         }

         for (i = 0; i < 2; i++) {
             if (i == 0) {
                 if (vtss_macsec_hmac_counters_get(MISC_INST, port_no, &counters, 1) != VTSS_RC_OK) {
                     CPRINTF("Could not perform vtss_macsec_*() operation \n");
                 }
                 CPRINTF("Port: %d Host Mac\n",port_no);
             } else {
                 if (vtss_macsec_lmac_counters_get(MISC_INST, port_no, &counters, 1) != VTSS_RC_OK) {
                     CPRINTF("Could not perform vtss_macsec_*() operation \n");
                 }                
                 CPRINTF("Port: %d Line Mac\n",port_no);
             }            
             CPRINTF("rx_ucast_pkts:     %d\n",counters.if_rx_ucast_pkts);
             CPRINTF("rx_multicast_pkts: %d\n",counters.if_rx_multicast_pkts);
             CPRINTF("rx_broadcast_pkts: %d\n",counters.if_rx_broadcast_pkts);
             CPRINTF("rx_errors:         %d\n",counters.if_rx_errors);
             CPRINTF("tx_ucast_pkts:     %d\n",counters.if_tx_ucast_pkts);
             CPRINTF("tx_multicast_pkts: %d\n",counters.if_tx_multicast_pkts);
             CPRINTF("tx_broadcast_pkts: %d\n",counters.if_tx_broadcast_pkts);  
         }         
         (void)vtss_macsec_init_get(MISC_INST, port_no, &init);
         if (!init.enable) {
             continue;
         }          
         for (a = 0; a < 2; a++) {
             if (a == 0) {
                 blk = 0; /* mac ingress */
                 CPRINTF("\nINGRESS:\n");
             } else { 
                 blk = 0x8000; /* mac egress */
                 CPRINTF("\nEGRESS:\n");
             }

             CPRINTF("\nCounters:\n");
             for (sa = 0; sa < 10; sa++) {
                 for (i = 0; i < 22; i++) {
                     (void)(vtss_macsec_csr_read(MISC_INST, port_no, blk, (0x2000 | (sa * 32))+i, &value));
                     if (value > 0)
                         CPRINTF("SA:%d Cnt (0x%x): %d\n",sa, 0x2000+i,value);  
                 }
                 for (i = 0; i < 27; i++) {
                     (void)(vtss_macsec_csr_read(MISC_INST, port_no, blk, (0x3100 | (sa * 32))+i, &value));
                     if (value > 0)
                         CPRINTF("SA:%d Cnt (0x%x): %d\n",sa, 0x3100+i,value);  
                 }
             }
         }
         {
             vtss_macsec_port_t    mport = {port_no, 0, 0};
             u16                   an=0;
             vtss_macsec_tx_sa_status_t status;
            
             if (vtss_macsec_tx_sa_status_get(MISC_INST, mport, an, &status) != VTSS_RC_ERROR) {
                 CPRINTF("pn:%u created:%x started:%x stopped:%d\n",status.next_pn, 
                         status.created_time, status.started_time, status.stopped_time);
             }
         }

     }
}
#endif /* VTSS_FEATURE_MACSEC */

static void misc_cli_phy_inst_set(cli_req_t *req)
{
    misc_cli_req_t          *misc_req = req->module_req;

    if (req->set) {
        if (misc_phy_inst_set(misc_req->phy_inst ? PHY_INST : NULL) != VTSS_RC_OK) {
            CPRINTF("Failed misc_phy_inst_set() operation");
        }
    } else {
       if (misc_phy_inst_get() == NULL) {
           CPRINTF("Using Default instance\n");
       } else {
           CPRINTF("Using PHY_INST instance\n");
       }
    }
}

#if defined(VTSS_SW_OPTION_BOARD)
static void cb_interrupt_function(vtss_interrupt_source_t        source_id,
                                  u32                            instance_id)
{
    vtss_rc  rc;

    rc = vtss_interrupt_source_hook_set(cb_interrupt_function,
                                        source_id,
                                        INTERRUPT_PRIORITY_NORMAL);
    if (rc != VTSS_RC_OK)       T_D("Error during interrupt hook");

    switch (source_id)
    {
        case INTERRUPT_SOURCE_LOS: printf("INTERRUPT_SOURCE_LOS  instance %u\n", instance_id); break;
        case INTERRUPT_SOURCE_FLNK: printf("INTERRUPT_SOURCE_FLNK  instance %u\n", instance_id); break;
        case INTERRUPT_SOURCE_AMS: printf("INTERRUPT_SOURCE_AMS  instance %u\n", instance_id); break;

        case INTERRUPT_SOURCE_RX_LOL: printf("INTERRUPT_SOURCE_RX_LOL  instance %u\n", instance_id); break;
        case INTERRUPT_SOURCE_LOPC: printf("INTERRUPT_SOURCE_LOPC  instance %u\n", instance_id); break;
        case INTERRUPT_SOURCE_RECEIVE_FAULT: printf("INTERRUPT_SOURCE_RECEIVE_FAULT  instance %u\n", instance_id); break;
        case INTERRUPT_SOURCE_MODULE_STAT: printf("INTERRUPT_SOURCE_MODULE_STAT  instance %u\n", instance_id); break;
        case INTERRUPT_SOURCE_TX_LOL: printf("INTERRUPT_SOURCE_TX_LOL  instance %u\n", instance_id); break;
        case INTERRUPT_SOURCE_HI_BER: printf("INTERRUPT_SOURCE_HI_BER  instance %u\n", instance_id); break;

        case INTERRUPT_SOURCE_LOCS: printf("INTERRUPT_SOURCE_LOCS  instance %u\n", instance_id); break;
        case INTERRUPT_SOURCE_FOS: printf("INTERRUPT_SOURCE_FOS  instance %u\n", instance_id); break;
        case INTERRUPT_SOURCE_LOSX: printf("INTERRUPT_SOURCE_LOSX  instance %u\n", instance_id); break;
        case INTERRUPT_SOURCE_LOL: printf("INTERRUPT_SOURCE_LOL  instance %u\n", instance_id); break;

        case INTERRUPT_SOURCE_I2C: printf("INTERRUPT_SOURCE_I2C  instance %u\n", instance_id); break;
        case INTERRUPT_SOURCE_LOC: printf("INTERRUPT_SOURCE_LOC  instance %u\n", instance_id); break;
                                    
        case INTERRUPT_SOURCE_SYNC: printf("INTERRUPT_SOURCE_SYNC  instance %u\n", instance_id); break;
        case INTERRUPT_SOURCE_EXT_SYNC: printf("INTERRUPT_SOURCE_EXT_SYNC  instance %u\n", instance_id); break;
        case INTERRUPT_SOURCE_CLK_ADJ: printf("INTERRUPT_SOURCE_CLK_ADJ  instance %u\n", instance_id); break;
        case INTERRUPT_SOURCE_CLK_TSTAMP: printf("INTERRUPT_SOURCE_CLK_TSTAMP  instance %u\n", instance_id); break;

        case INTERRUPT_SOURCE_INGR_ENGINE_ERR: printf("INTERRUPT_SOURCE_INGR_ENGINE_ERR  instance %u\n", instance_id); break;
        case INTERRUPT_SOURCE_INGR_RW_PREAM_ERR: printf("INTERRUPT_SOURCE_INGR_RW_PREAM_ERR  instance %u\n", instance_id); break;
        case INTERRUPT_SOURCE_INGR_RW_FCS_ERR: printf("INTERRUPT_SOURCE_INGR_RW_FCS_ERR  instance %u\n", instance_id); break;
        case INTERRUPT_SOURCE_EGR_ENGINE_ERR: printf("INTERRUPT_SOURCE_EGR_ENGINE_ERR  instance %u\n", instance_id); break;
        case INTERRUPT_SOURCE_EGR_RW_FCS_ERR: printf("INTERRUPT_SOURCE_EGR_RW_FCS_ERR  instance %u\n", instance_id); break;
        case INTERRUPT_SOURCE_EGR_TIMESTAMP_CAPTURED: printf("INTERRUPT_SOURCE_EGR_TIMESTAMP_CAPTURED  instance %u\n", instance_id); break;
        case INTERRUPT_SOURCE_EGR_FIFO_OVERFLOW:      printf("INTERRUPT_SOURCE_EGR_FIFO_OVERFLOW  instance %u\n", instance_id); break;

        case INTERRUPT_SOURCE_EWIS_SEF_EV:                  printf("INTERRUPT_SOURCE_EWIS_SEF_EV  instance %u\n", instance_id); break;     
        case INTERRUPT_SOURCE_EWIS_FPLM_EV:                 printf("INTERRUPT_SOURCE_EWIS_FPLM_EV  instance %u\n", instance_id); break;    
        case INTERRUPT_SOURCE_EWIS_FAIS_EV:                 printf("INTERRUPT_SOURCE_EWIS_FAIS_EV  instance %u\n", instance_id); break;   
        case INTERRUPT_SOURCE_EWIS_LOF_EV:                  printf("INTERRUPT_SOURCE_EWIS_LOF_EV  instance %u\n", instance_id); break;  
        case INTERRUPT_SOURCE_EWIS_RDIL_EV:                 printf("INTERRUPT_SOURCE_EWIS_RDIL_EV  instance %u\n", instance_id); break;
        case INTERRUPT_SOURCE_EWIS_AISL_EV:                 printf("INTERRUPT_SOURCE_EWIS_AISL_EV  instance %u\n", instance_id); break;
        case INTERRUPT_SOURCE_EWIS_LCDP_EV:                 printf("INTERRUPT_SOURCE_EWIS_LCDP_EV  instance %u\n", instance_id); break;
        case INTERRUPT_SOURCE_EWIS_PLMP_EV:                 printf("INTERRUPT_SOURCE_EWIS_PLMP_EV  instance %u\n", instance_id); break;
        case INTERRUPT_SOURCE_EWIS_AISP_EV:                 printf("INTERRUPT_SOURCE_EWIS_AISP_EV  instance %u\n", instance_id); break;
        case INTERRUPT_SOURCE_EWIS_LOPP_EV:                 printf("INTERRUPT_SOURCE_EWIS_LOPP_EV  instance %u\n", instance_id); break;
        case INTERRUPT_SOURCE_EWIS_UNEQP_EV:                printf("INTERRUPT_SOURCE_EWIS_UNEQP_EV  instance %u\n", instance_id); break;
        case INTERRUPT_SOURCE_EWIS_FEUNEQP_EV:              printf("INTERRUPT_SOURCE_EWIS_FEUNEQP_EV  instance %u\n", instance_id); break;
        case INTERRUPT_SOURCE_EWIS_FERDIP_EV:               printf("INTERRUPT_SOURCE_EWIS_FERDIP_EV  instance %u\n", instance_id); break;
        case INTERRUPT_SOURCE_EWIS_REIL_EV:                 printf("INTERRUPT_SOURCE_EWIS_REIL_EV  instance %u\n", instance_id); break;
        case INTERRUPT_SOURCE_EWIS_REIP_EV:                 printf("INTERRUPT_SOURCE_EWIS_REIP_EV  instance %u\n", instance_id); break;
        case INTERRUPT_SOURCE_EWIS_B1_NZ_EV:                printf("INTERRUPT_SOURCE_EWIS_B1_NZ_EV  instance %u\n", instance_id); break;
        case INTERRUPT_SOURCE_EWIS_B2_NZ_EV:                printf("INTERRUPT_SOURCE_EWIS_B2_NZ_EV  instance %u\n", instance_id); break;
        case INTERRUPT_SOURCE_EWIS_B3_NZ_EV:                printf("INTERRUPT_SOURCE_EWIS_B3_NZ_EV  instance %u\n", instance_id); break;
        case INTERRUPT_SOURCE_EWIS_REIL_NZ_EV:              printf("INTERRUPT_SOURCE_EWIS_REIL_NZ_EV  instance %u\n", instance_id); break;
        case INTERRUPT_SOURCE_EWIS_REIP_NZ_EV:              printf("INTERRUPT_SOURCE_EWIS_REIP_NZ_EV  instance %u\n", instance_id); break;
        case INTERRUPT_SOURCE_EWIS_B1_THRESH_EV:            printf("INTERRUPT_SOURCE_EWIS_B1_THRESH_EV  instance %u\n", instance_id); break;
        case INTERRUPT_SOURCE_EWIS_B2_THRESH_EV:            printf("INTERRUPT_SOURCE_EWIS_B2_THRESH_EV  instance %u\n", instance_id); break;
        case INTERRUPT_SOURCE_EWIS_B3_THRESH_EV:            printf("INTERRUPT_SOURCE_EWIS_B3_THRESH_EV  instance %u\n", instance_id); break;
        case INTERRUPT_SOURCE_EWIS_REIL_THRESH_EV:          printf("INTERRUPT_SOURCE_EWIS_REIL_THRESH_EV  instance %u\n", instance_id); break;
        case INTERRUPT_SOURCE_EWIS_REIP_THRESH_EV:          printf("INTERRUPT_SOURCE_EWIS_REIP_THRESH_EV  instance %u\n", instance_id); break;
        default:              printf("Invalid interrupt source received\n");
    }
}

static void misc_cli_cmd_debug_interrupt_hook ( cli_req_t * req )
{
    vtss_rc  rc=VTSS_OK;
    misc_cli_req_t     *misc_req = req->module_req;
    req->set = 1;

    if (misc_req->enable)
        rc = vtss_interrupt_source_hook_set(cb_interrupt_function,
                                            misc_req->int_source,
                                            INTERRUPT_PRIORITY_NORMAL);

    if (misc_req->disable)
        rc = vtss_interrupt_source_hook_clear(cb_interrupt_function,
                                              misc_req->int_source);

    if (rc != VTSS_OK)
        T_D("Interrupt Hook call failed");
}
#endif /* VTSS_SW_OPTION_BOARD */

static void misc_cli_cmd_debug_critd_max_lock(cli_req_t *req)
{
    vtss_module_id_t mid;
    BOOL             header = 1;
    misc_cli_req_t   *misc_req = req->module_req;

    if (misc_req->module_all) {
        critd_dbg_max_lock(cli_printf, VTSS_MODULE_ID_NONE, 1, misc_req->clear);
    } else {
        for (mid = 0; mid < VTSS_MODULE_ID_NONE; mid++) {
            if (misc_req->module_list[mid] == 0)
                continue;
            
            critd_dbg_max_lock(cli_printf, mid, header, misc_req->clear);
            header = 0;
        }
    }
}

static void misc_cli_cmd_debug_led_usid(cli_req_t *req)
{
    if (req->set) {
        led_usid_set(req->usid[0]);
    } else {
        CPRINTF("USID LED: %d\n", led_usid_get());
    }
}

#ifdef VTSS_FEATURE_WIS
static void misc_cli_wis_reset(cli_req_t *req)
{
    vtss_port_no_t port_no, port;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->uport_list[port] == 0) || !vtss_phy_10G_is_valid(MISC_INST, port_no)) 
            continue;
      
        if (vtss_ewis_reset(MISC_INST, port_no) != VTSS_RC_OK) {
            CPRINTF("Could not perform vtss_ewis_reset() for port %u\n",port_no);
            continue;
        } 
    }
}
static void misc_cli_wis_mode(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    misc_cli_req_t *misc_req = req->module_req;
    vtss_ewis_mode_t  wis_mode;
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->uport_list[port] == 0) || !vtss_phy_10G_is_valid(MISC_INST, port_no)) 
            continue;
        if (vtss_ewis_mode_get(MISC_INST, port_no, &wis_mode) != VTSS_RC_OK) {
            CPRINTF("Could not perform vtss_ewis_mode_get() for port %u\n",port);
            continue;
        }
        if (req->set) {
#if 0
            if(wis_mode == misc_req->enable) {
                continue;
            }
#endif /* 0 */
            if (misc_req->enable) {
                wis_mode = VTSS_WIS_OPERMODE_WIS_MODE;
            } else {
                wis_mode = VTSS_WIS_OPERMODE_DISABLE;
            }
            if (vtss_ewis_mode_set(MISC_INST, port_no, &wis_mode) != VTSS_RC_OK) {
                CPRINTF("Could not perform vtss_ewis_mode_set() for port %u\n",port);
                continue;
            }
        } else {
            CPRINTF("Port: %u\n",port);
            CPRINTF("--------\n");
            CPRINTF("WIS Mode :%-12s  \n",wis_mode ? "Enabled" : "Disabled");
        }
    }
}

static void misc_cli_wis_atti(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    misc_cli_req_t    *misc_req = req->module_req;
    vtss_ewis_tti_t       wis_section_acti,wis_path_acti;
    BOOL sec=0, path=0;

    memset(&wis_section_acti, 0, sizeof(vtss_ewis_tti_t));
    memset(&wis_path_acti, 0, sizeof(vtss_ewis_tti_t));
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->uport_list[port] == 0) || !vtss_phy_10G_is_valid(MISC_INST, port_no)) 
            continue;
        
        if (misc_req->wis_overhead_set) {
            if (misc_req->wis_overhead==0) {
                sec = 1;
            } else if (misc_req->wis_overhead==1) {
                path = 1;
            }
        } else {
            sec = 1;
            path = 1;
        }
        if (sec){
            if (vtss_ewis_section_acti_get(MISC_INST, port_no, &wis_section_acti) != VTSS_RC_OK) {
                CPRINTF("Could not perform vtss_ewis_section_acti_get() operation for port %u\n",port);
                continue;
            }
            CPRINTF("Port: %u\n",port);
            CPRINTF("--------\n");
            CPRINTF("Section received TTI :\n");
            CPRINTF("%-20s: %-4u\n","Mode",wis_section_acti.mode);
            if (wis_section_acti.valid != FALSE) {
                CPRINTF("%-20s: %-64s\n","TTI",wis_section_acti.tti);
            } else {
                CPRINTF("Invalid TxTI configuration :: Transmitter and Receiver Section TxTI modes are not matching\n");
            }
        }
        if (path){
            if (vtss_ewis_path_acti_get(MISC_INST, port_no, &wis_path_acti) != VTSS_RC_OK) {
                CPRINTF("Could not perform vtss_ewis_section_acti_get() operation for port %u\n",port);
                continue;
            }
            CPRINTF("Port: %u\n",port);
            CPRINTF("--------\n");
            CPRINTF("Path received TTI :\n");
            CPRINTF("%-20s: %-4u\n","Mode",wis_path_acti.mode);
            if (wis_path_acti.valid != FALSE) {
                CPRINTF("%-20s: %-64s\n","TTI",wis_path_acti.tti);
            } else {
                CPRINTF("Invalid TxTI configuration :: Transmitter and Receiver Path TxTI modes are not matching\n");
            }
        }
    }
}
static void misc_cli_wis_conse_act(cli_req_t *req)
{
    vtss_port_no_t       port_no, port;
    misc_cli_req_t       *misc_req = req->module_req;
    vtss_ewis_cons_act_t cons_act;
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->uport_list[port] == 0) || !vtss_phy_10G_is_valid(MISC_INST, port_no)) 
            continue;
        if (vtss_ewis_cons_act_get(MISC_INST, port_no, &cons_act) != VTSS_RC_OK) {
            CPRINTF("Could not perform vtss_ewis_cons_act_get() for port %u\n",port);
            continue;
        }
        if (req->set) {
            cons_act.aisl.ais_on_los         = misc_req->wis_aisl&1;
            cons_act.aisl.ais_on_lof         = (misc_req->wis_aisl>>1)&1;
            
            cons_act.rdil.rdil_on_los        = misc_req->wis_rdil&1;
            cons_act.rdil.rdil_on_lof        = (misc_req->wis_rdil >>1)&1;
            cons_act.rdil.rdil_on_lopc       = (misc_req->wis_rdil >> 2)&1;
            cons_act.rdil.rdil_on_ais_l      = (misc_req->wis_rdil>>3)&1;

            cons_act.fault.fault_on_feplmp   = misc_req->wis_fault&1;
            cons_act.fault.fault_on_feaisp   = (misc_req->wis_fault >>1)&1;
            cons_act.fault.fault_on_rdil     = (misc_req->wis_fault >>2)&1;
            cons_act.fault.fault_on_sef      = (misc_req->wis_fault >>3)&1;
            cons_act.fault.fault_on_lof      = (misc_req->wis_fault >>4)&1;
            cons_act.fault.fault_on_los      = (misc_req->wis_fault >>5)&1;
            cons_act.fault.fault_on_aisl     = (misc_req->wis_fault >>6)&1;
            cons_act.fault.fault_on_lcdp     = (misc_req->wis_fault >>7)&1;
            cons_act.fault.fault_on_plmp     = (misc_req->wis_fault >>8)&1;
            cons_act.fault.fault_on_aisp     = (misc_req->wis_fault >>9)&1;
            cons_act.fault.fault_on_lopp     = (misc_req->wis_fault >>10)&1;
            
            if (vtss_ewis_cons_act_set(MISC_INST, port_no, &cons_act) != VTSS_RC_OK) {
                CPRINTF("Could not perform vtss_ewis_cons_act_set() for port %u\n",port);
                continue;
            }
        } else {
            CPRINTF("Port: %u\n",port);
            CPRINTF("--------\n");
            CPRINTF("AIS-L insertion on LOS           :  %-6d \n",cons_act.aisl.ais_on_los);
            CPRINTF("AIS-L insertion on LOF           :  %-6d \n\n",cons_act.aisl.ais_on_lof);
            CPRINTF("RDI-L back reporting on LOS      :  %-6d \n",cons_act.rdil.rdil_on_los);
            CPRINTF("RDI-L back reporting on LOF      :  %-6d \n",cons_act.rdil.rdil_on_lof);
            CPRINTF("RDI-L back reporting on LOPC     :  %-6d \n",cons_act.rdil.rdil_on_lopc);
            CPRINTF("RDI-L back reporting on AIS_L    :  %-6d \n\n",cons_act.rdil.rdil_on_ais_l);
            CPRINTF("Fault condition on far-end PLM-P :  %-6d \n",cons_act.fault.fault_on_feplmp);
            CPRINTF("Fault condition on far-end AIS-P :  %-6d \n",cons_act.fault.fault_on_feaisp);
            CPRINTF("Fault condition on RDI-L         :  %-6d \n",cons_act.fault.fault_on_rdil);
            CPRINTF("Fault condition on SEF           :  %-6d \n",cons_act.fault.fault_on_sef);
            CPRINTF("Fault condition on LOF           :  %-6d \n",cons_act.fault.fault_on_lof);
            CPRINTF("Fault condition on LOS           :  %-6d \n",cons_act.fault.fault_on_los);
            CPRINTF("Fault condition on AIS-L         :  %-6d \n",cons_act.fault.fault_on_aisl);
            CPRINTF("Fault condition on LCD-P         :  %-6d \n",cons_act.fault.fault_on_lcdp);
            CPRINTF("Fault condition on PLM-P         :  %-6d \n",cons_act.fault.fault_on_plmp);
            CPRINTF("Fault condition on AIS-P         :  %-6d \n",cons_act.fault.fault_on_aisp);
            CPRINTF("Fault condition on LOP-P         :  %-6d \n",cons_act.fault.fault_on_lopp);
        }
    }
}
static void misc_cli_wis_txtti(cli_req_t *req)
{
    vtss_port_no_t    port_no, port;
    misc_cli_req_t    *misc_req = req->module_req;
    vtss_ewis_tti_t   tx_tti_path, tx_tti_section;
    u32               i;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->uport_list[port] == 0) || !vtss_phy_10G_is_valid(MISC_INST, port_no)) 
            continue;
      
        if (req->set) {
            if (misc_req->wis_overhead_set) {
                if (misc_req->wis_overhead == 0) {
                    if (vtss_ewis_section_txti_get(MISC_INST, port_no, &tx_tti_section) != VTSS_RC_OK) {
                        CPRINTF("Could not perform vtss_ewis_section_txti_get() for port %u\n",port);
                        continue;
                    }
                    
                    if (misc_req->wis_tti_mode_set) {
                        tx_tti_section.mode = misc_req->wis_tti_mode;
                    }
                    if (misc_req->wis_tti_set) {
                        if (tx_tti_section.mode == TTI_MODE_16) {
                            memcpy(tx_tti_section.tti,misc_req->wis_tti,15);
                            tx_tti_section.tti[15] = 0x89; /* MSB set to 1 */
                        } else if (tx_tti_section.mode == TTI_MODE_64) {
                            memcpy(&tx_tti_section.tti[2],misc_req->wis_tti,62);
                            tx_tti_section.tti[0] = 13; /* CR */
                            tx_tti_section.tti[1] = 10; /* LF */
                        } else if (tx_tti_section.mode == TTI_MODE_1) {
                            tx_tti_section.tti[0] = misc_req->wis_tti[0];
                        }
                    }
                    if (vtss_ewis_section_txti_set(MISC_INST, port_no, &tx_tti_section) != VTSS_RC_OK) {
                        CPRINTF("Could not perform vtss_ewis_section_txti_set() for port %u\n",port);
                        continue;
                    }
                }
                if (misc_req->wis_overhead == 1) {
                    if (vtss_ewis_path_txti_get(MISC_INST, port_no, &tx_tti_path) != VTSS_RC_OK) {
                        CPRINTF("Could not perform vtss_ewis_path_txti_get() for port %u\n",port);
                        continue;
                    }
                    
                    if (misc_req->wis_tti_mode_set) {
                        tx_tti_path.mode = misc_req->wis_tti_mode;
                    }
                    if (misc_req->wis_tti_set) {
                        if (tx_tti_path.mode == TTI_MODE_16) {
                            memcpy(tx_tti_path.tti,misc_req->wis_tti,15);
                            tx_tti_path.tti[15] = 0x89; /* MSB set to 1 */
                        } else if (tx_tti_path.mode == TTI_MODE_64) {
                            memcpy(&tx_tti_path.tti[2],misc_req->wis_tti,62);
                            tx_tti_path.tti[0] = 13; /* CR */
                            tx_tti_path.tti[1] = 10; /* LF */
                        } else if (tx_tti_path.mode == TTI_MODE_1) {
                            tx_tti_path.tti[0] = misc_req->wis_tti[0];
                        }
                    }
                    if (vtss_ewis_path_txti_set(MISC_INST, port_no, &tx_tti_path) != VTSS_RC_OK) {
                        CPRINTF("Could not perform vtss_ewis_path_txti_set() for port %u\n",port);
                        continue;
                    }
                }
            } else {
                if (vtss_ewis_section_txti_get(MISC_INST, port_no, &tx_tti_section) != VTSS_RC_OK) {
                    CPRINTF("Could not perform vtss_ewis_section_txti_get() for port %u\n",port);
                    continue;
                }
                
                if (misc_req->wis_tti_mode_set) {
                    tx_tti_section.mode = misc_req->wis_tti_mode;
                }
                if (misc_req->wis_tti_set) {
                    memcpy(tx_tti_section.tti,misc_req->wis_tti,64);
                }
                if (vtss_ewis_section_txti_set(MISC_INST, port_no, &tx_tti_section) != VTSS_RC_OK) {
                    CPRINTF("Could not perform vtss_ewis_section_txti_set() for port %u\n",port);
                    continue;
                }
                
                if (vtss_ewis_path_txti_get(MISC_INST, port_no, &tx_tti_path) != VTSS_RC_OK) {
                    CPRINTF("Could not perform vtss_ewis_path_txti_get() for port %u\n",port);
                    continue;
                }
                
                if (misc_req->wis_tti_mode_set) {
                    tx_tti_path.mode = misc_req->wis_tti_mode;
                }
                if (misc_req->wis_tti_mode_set) {
                    memcpy(tx_tti_path.tti,misc_req->wis_tti,64);
                }
                if (vtss_ewis_path_txti_set(MISC_INST, port_no, &tx_tti_path) != VTSS_RC_OK) {
                    CPRINTF("Could not perform vtss_ewis_path_txti_set() for port %u\n",port);
                    continue;
                }
            }
            
        } else {
            if (vtss_ewis_section_txti_get(MISC_INST, port_no, &tx_tti_section) != VTSS_RC_OK) {
                CPRINTF("Could not perform vtss_ewis_section_txti_get() for port %u\n",port);
                continue;
            }

            CPRINTF("Port: %u\n",port);
            CPRINTF("--------\n");
            CPRINTF("Section TTI Mode :%-12d \nSection TTI:",tx_tti_section.mode);
            if (tx_tti_section.mode == 0) {
                CPRINTF("%c",tx_tti_section.tti[0]);
            } else if (tx_tti_section.mode == 1) {
                /* 15th Char is Static charecter filled by API */
                for (i=0; i < 15; i++){
                    CPRINTF("%c",tx_tti_section.tti[i]);
                }                
            }else if (tx_tti_section.mode == 2) {
                /* First two bytes are  filled by API */
                for (i=2; i < 64; i++){
                    CPRINTF("%c",tx_tti_section.tti[i]);
                }            
            }
            CPRINTF("\n");
            if (vtss_ewis_path_txti_get(MISC_INST, port_no, &tx_tti_path) != VTSS_RC_OK) {
                CPRINTF("Could not perform vtss_ewis_test_mode_get() operation");
                continue;
            }
            
            CPRINTF("Port: %u\n",port);
            CPRINTF("--------\n");
            CPRINTF("Path TTI Mode :%-12d \nPath TTI:",tx_tti_path.mode);
            if (tx_tti_path.mode == 0) {
                CPRINTF("%c",tx_tti_path.tti[0]);
            } else if (tx_tti_path.mode == 1) {
                /* 15th Char is Static charecter filled by API */
                for (i=0; i < 15; i++){
                    CPRINTF("%c",tx_tti_path.tti[i]);
                }
            }else if (tx_tti_path.mode == 2) {
                /* First two bytes are  filled by API */
                for (i=2; i < 64; i++){
                    CPRINTF("%c",tx_tti_path.tti[i]);
                }    
            }
            CPRINTF("\n");
        }
    }
}
static void misc_cli_wis_test_mode(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    misc_cli_req_t    *misc_req = req->module_req;
    vtss_ewis_test_conf_t  test_mode;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->uport_list[port] == 0) || !vtss_phy_10G_is_valid(MISC_INST, port_no)) 
            continue;
        if (vtss_ewis_test_mode_get(MISC_INST, port_no, &test_mode) != VTSS_RC_OK) {
            CPRINTF("Could not perform vtss_ewis_test_mode_get() operation");
            continue;
        }
        if (req->set) {
            if (misc_req->wis_loopback_set) {
                test_mode.loopback         = misc_req->wis_loopback;
            }
            test_mode.test_pattern_ana = misc_req->wis_ana_mode;
            test_mode.test_pattern_gen = misc_req->wis_gen_mode;
            if (vtss_ewis_test_mode_set(MISC_INST, port_no, &test_mode) != VTSS_RC_OK) {
                CPRINTF("Could not perform vtss_ewis_test_mode_set() operation for port %u\n",port);
                continue;
            }
        } else {
            CPRINTF("Port: %u\n",port);
            CPRINTF("--------\n");
            CPRINTF("Loopback :%-12s  Test Pattern generator  : %u   Test pattern analyzer  :%u\n",
                    test_mode.loopback?"Yes":"No", 
                    test_mode.test_pattern_gen, test_mode.test_pattern_ana);
        }
    }
}


static void misc_cli_wis_status(cli_req_t * req)
{
    
        vtss_port_no_t       port_no, port;
        vtss_ewis_status_t   wis_status;
        
        for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
            port = iport2uport(port_no);
            if ((req->uport_list[port] == 0) || !vtss_phy_10G_is_valid(MISC_INST, port_no)) 
                continue;
            if (vtss_ewis_status_get(MISC_INST, port_no, &wis_status) != VTSS_RC_OK) {
                CPRINTF("Could not perform vtss_ewis_status_get() operation for Port: %u\n",port);
                continue;
            }
        
            CPRINTF("WIS Status Port: %u\n",port);
            CPRINTF("-------------------\n");
            CPRINTF("%-30s %-12s\n", "  Fault condition       :",wis_status.fault?"Fault":"Normal");
            CPRINTF("%-30s %-12s\n", "  Link status condition :",wis_status.link_stat?"Up":"Down");
        }
}

static void misc_cli_wis_test_status(cli_req_t * req)
{
    vtss_port_no_t port_no, port;
    vtss_ewis_test_status_t test_status;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->uport_list[port] == 0) || !vtss_phy_10G_is_valid(MISC_INST, port_no)) 
            continue;
        if (vtss_ewis_test_counter_get(MISC_INST, port_no, &test_status) != VTSS_RC_OK) {
            CPRINTF("Could not perform vtss_ewis_test_counter_get() operation for Port: %u \n",port);
            continue;
        }

        CPRINTF("WIS Test Status Port: %u\n",port);
        CPRINTF("------------------------\n");
        CPRINTF("%-30s %-12d\n", "  PRBS31 test pattern error counter:",test_status.tstpat_cnt);
        CPRINTF("%-30s %-12s\n", "  PRBS31 test Analyzer status      :",(test_status.ana_sync)? "Sync" : "Not Synced");
    }

}

static void misc_cli_wis_counters(cli_req_t * req)
{
    vtss_port_no_t port_no, port;
    vtss_ewis_counter_t     counter;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->uport_list[port] == 0) || !vtss_phy_10G_is_valid(MISC_INST, port_no)) 
            continue;
        if (vtss_ewis_counter_get(MISC_INST, port_no, &counter) != VTSS_RC_OK) {
            CPRINTF("Could not perform vtss_phy_ewis_counter_get()  operation for Port: %u\n",port);
            continue;
        }

        CPRINTF("WIS Error Counters:\n\n");
        CPRINTF("Port: %u\n",port);
        CPRINTF("--------\n");
        CPRINTF("%-30s %-12u\n", "  Section BIP error count               :",counter.pn_ebc_s);
        CPRINTF("%-30s %-12u\n", "  Near end line block (BIP) error count:",counter.pn_ebc_l);
        CPRINTF("%-30s %-12u\n", "  Far end line block (BIP) error count :",counter.pf_ebc_l);
        CPRINTF("%-30s %-12u\n", "  Path block error count                :",counter.pn_ebc_p);
        CPRINTF("%-30s %-12u\n", "  Far end path block error count        :",counter.pf_ebc_p);
    }
}
static void misc_cli_wis_defects(cli_req_t * req)
{
    vtss_port_no_t          port_no, port;
    vtss_ewis_defects_t     defects;
    
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->uport_list[port] == 0) || !vtss_phy_10G_is_valid(MISC_INST, port_no)) 
            continue;
       
        if (vtss_ewis_defects_get(MISC_INST, port_no, &defects) != VTSS_RC_OK) {
            CPRINTF("Could not perform vtss_ewis_defects_get()  operation for Port: %u\n",port);
            continue;
        }

        CPRINTF("WIS Defects:\n\n");
        CPRINTF("Port: %u\n",port);
        CPRINTF("--------\n");
        CPRINTF("%-30s %-12s\n", "  Loss of signal                      :",defects.dlos_s?"Yes":"No");
        CPRINTF("%-30s %-12s\n", "  Loss of frame                       :",defects.dlof_s?"Yes":"No");
        CPRINTF("%-30s %-12s\n", "  Line alarm indication signal        :",defects.dais_l?"Yes":"No");
        CPRINTF("%-30s %-12s\n", "  Line remote defect indication       :",defects.drdi_l?"Yes":"No");
        CPRINTF("%-30s %-12s\n", "  Path alarm indication signal        :",defects.dais_p?"Yes":"No");
        CPRINTF("%-30s %-12s\n", "  Loss of pointer                     :",defects.dlop_p?"Yes":"No");
        CPRINTF("%-30s %-12s\n", "  Path Unequipped                     :",defects.duneq_p?"Yes":"No");
        CPRINTF("%-30s %-12s\n", "  Path Remote Defect Indication       :",defects.drdi_p?"Yes":"No");
        CPRINTF("%-30s %-12s\n", "  Path loss of code-group delineation :",defects.dlcd_p?"Yes":"No");
        CPRINTF("%-30s %-12s\n", "  Path label Mismatch                 :",defects.dplm_p?"Yes":"No");
        CPRINTF("%-30s %-12s\n", "  Far-end AIS-P or LOP-P              :",defects.dfais_p?"Yes":"No");
        CPRINTF("%-30s %-12s\n", "  Far-end PLM-P or LCD-P defect       :",defects.dfplm_p?"Yes":"No");
//      CPRINTF("%-30s %-12s\n", "  Far End Path Unequipped             :",defects.dfuneq_p?"Yes":"No");
    }
}

static void misc_cli_wis_force_conf(cli_req_t * req)
{
    vtss_port_no_t          port_no, port;
    vtss_ewis_force_mode_t  force_conf;
    misc_cli_req_t          *misc_req = req->module_req;
    
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->uport_list[port] == 0) || !vtss_phy_10G_is_valid(MISC_INST, port_no)) 
            continue;
        if (req->set) {
            
            force_conf.line_rx_force.force_ais =  misc_req->wis_line_rx&1;
            force_conf.line_rx_force.force_rdi = (misc_req->wis_line_rx>>1)&1;
            force_conf.line_tx_force.force_ais =  misc_req->wis_line_tx&1;
            force_conf.line_tx_force.force_rdi = (misc_req->wis_line_tx>>1)&1;
            force_conf.path_force.force_uneq   =  misc_req->wis_path_force&1;;
            force_conf.path_force.force_rdi    = (misc_req->wis_path_force>>1)&1;;
            
            if (vtss_ewis_force_conf_set(MISC_INST, port_no, &force_conf) != VTSS_RC_OK) {
                CPRINTF("Could not perform vtss_ewis_force_conf_set() for port %u\n",port);
                continue;
            }
        } else {
            if (vtss_ewis_force_conf_get(MISC_INST, port_no, &force_conf) != VTSS_RC_OK) {
                CPRINTF("Could not perform vtss_ewis_force_conf_get() for port %u\n",port);
                continue;
            }
            CPRINTF("Port: %u\n",port);
            CPRINTF("--------\n");
            CPRINTF("Line rx force ais :%-6s \n",force_conf.line_rx_force.force_ais?"Yes":"No");
            CPRINTF("Line rx force rdi :%-6s \n",force_conf.line_rx_force.force_rdi?"Yes":"No");
            CPRINTF("Line tx force ais :%-6s \n",force_conf.line_tx_force.force_ais?"Yes":"No");
            CPRINTF("Line tx force rdi :%-6s \n",force_conf.line_tx_force.force_rdi?"Yes":"No");
            CPRINTF("path force uneq   :%-6s \n",force_conf.path_force.force_uneq?"Yes":"No");
            CPRINTF("path force rdi   :%-6s \n",force_conf.path_force.force_rdi?"Yes":"No");
        }
    }
}

static void misc_cli_wis_tx_overhead_conf(cli_req_t * req)
{
    vtss_port_no_t     port_no, port;
    vtss_ewis_tx_oh_t  tx_oh;
    misc_cli_req_t     *misc_req = req->module_req;
    u8                 *oh_value,i=0;
    

    oh_value = misc_req->wis_oh_val;
    memset(&tx_oh, 0,sizeof(vtss_ewis_tx_oh_t));
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->uport_list[port] == 0) || !vtss_phy_10G_is_valid(MISC_INST, port_no)) 
            continue;
        if (vtss_ewis_tx_oh_get(MISC_INST, port_no, &tx_oh) != VTSS_RC_OK) {
            CPRINTF("Could not perform vtss_ewis_tx_oh_get(): %u operation\n",port);
            continue;
        }
        if (req->set) {
            switch(misc_req->wis_oh_id){
                case 1:
                    memcpy((u8 *)(tx_oh.tx_dcc_s),oh_value, 3); i=i+3;
                    tx_oh.tx_e1       = *(oh_value+i++);
                    tx_oh.tx_f1       = *(oh_value+i++);
                    memcpy((u8 *)(tx_oh.tx_dcc_l),(oh_value+i), 9); i=i+9;
                    tx_oh.tx_e2= *(oh_value+i++);
                    memcpy((u8 *)(&tx_oh.tx_k1_k2),(oh_value+i), 2); i=i+2;
                    tx_oh.tx_s1= *(oh_value+i++);
                    memcpy((u8 *)(&tx_oh.tx_z1_z2),(oh_value+i), 2); i=i+2;
                    tx_oh.tx_c2= *(oh_value+i++);
                    tx_oh.tx_f2= *(oh_value+i++);
                    tx_oh.tx_n1= *(oh_value+i++);
                    memcpy((u8 *)(&tx_oh.tx_z3_z4),(oh_value+i), 2);
                    break;
                case 2:
                    memcpy((u8 *)(tx_oh.tx_dcc_s),oh_value, 3); i=i+3;
                    tx_oh.tx_e1       = *(oh_value+i++);
                    tx_oh.tx_f1       = *(oh_value+i++);
                    break;
                case 3:
                    memcpy((u8 *) (tx_oh.tx_dcc_l),oh_value, 9); i=i+9;
                    tx_oh.tx_e2= *(oh_value+i++);
                    memcpy((u8 *) (&tx_oh.tx_k1_k2),(oh_value+i), 2); i=i+2;
                    tx_oh.tx_s1= *(oh_value+i++);
                    memcpy((u8 *)(&tx_oh.tx_z1_z2),(oh_value+i), 2);
                    break;
                case 4:
                    tx_oh.tx_c2= *(oh_value+i++);
                    tx_oh.tx_f2= *(oh_value+i++);
                    tx_oh.tx_n1= *(oh_value+i++);
                    memcpy((u8 *) (&tx_oh.tx_z3_z4),(oh_value+i), 2);
                    break;
                case 5:
                    memcpy((u8 *) (tx_oh.tx_dcc_s),oh_value, 3);
                    break;
                case 6:
                    tx_oh.tx_e1       = oh_value[0];
                    break;
                case 7:
                    tx_oh.tx_f1       = oh_value[0];
                    break;
                case 8:
/*                    tx_oh.tx_z0       = oh_value[0]; */
                    break;
                case 9:
                    memcpy((u8 *) (tx_oh.tx_dcc_l),oh_value, 9);
                    break;
                case 10:
                    tx_oh.tx_e2= oh_value[0];
                    break;
                case 11:
                    memcpy((u8 *) (&tx_oh.tx_k1_k2),oh_value, 2);
                    break;
                case 12:
                    tx_oh.tx_s1= oh_value[0];
                    break;
                case 13:
                    memcpy((u8 *) (&tx_oh.tx_z1_z2),oh_value, 2);
                    break;
                case 14:
                    tx_oh.tx_c2= oh_value[0];
                    break;
                case 15:
                    tx_oh.tx_f2= oh_value[0];
                    break;
                case 16:
                    tx_oh.tx_n1= oh_value[0];
                    break;
                case 17:
                    memcpy((u8 *) (&tx_oh.tx_z3_z4),oh_value, 2);
                    break;
                default:
                    
                    break;
            }
            if (vtss_ewis_tx_oh_set(MISC_INST, port_no, &tx_oh) != VTSS_RC_OK) {
                CPRINTF("Could not perform vtss_ewis_tx_oh_set() for port %u\n",port);
                continue;
            }
        } else {
            if (vtss_ewis_tx_oh_get(MISC_INST, port_no, &tx_oh) != VTSS_RC_OK) {
                CPRINTF("Could not perform vtss_ewis_tx_oh_get(): %u operation\n",port);
                continue;
            }
            CPRINTF("Tx Overhead for Port: %u\n",port);
            CPRINTF("------------------------\n");
            CPRINTF("D1-D3     :  0x%x%x%x \n",tx_oh.tx_dcc_s[0],tx_oh.tx_dcc_s[1],tx_oh.tx_dcc_s[2]);
            CPRINTF("SEC-ORD   :  0x%x\n",tx_oh.tx_e1);
            CPRINTF("SUC       :  0x%x \n",tx_oh.tx_f1);
            CPRINTF("D4-D12    :  0x%x%x%x%x%x%x%x%x%x \n",tx_oh.tx_dcc_l[0],tx_oh.tx_dcc_l[1],tx_oh.tx_dcc_l[2],
                                                           tx_oh.tx_dcc_l[3],tx_oh.tx_dcc_l[4],tx_oh.tx_dcc_l[5],
                                                           tx_oh.tx_dcc_l[6],tx_oh.tx_dcc_l[7],tx_oh.tx_dcc_l[8]);
            CPRINTF("LINE-ORD  :  0x%x \n",tx_oh.tx_e2);
            CPRINTF("APS-RDIL  :  0x%x \n",tx_oh.tx_e2);
            CPRINTF("SYNC      :  0x%x \n",tx_oh.tx_k1_k2);
            CPRINTF("RES-LG    :  0x%x \n",tx_oh.tx_z1_z2);
            CPRINTF("C2PL      :  0x%x \n",tx_oh.tx_c2);
            CPRINTF("PUC       :  0x%x \n",tx_oh.tx_f2);
            CPRINTF("PTCM      :  0x%x \n",tx_oh.tx_n1);
            CPRINTF("RES-PG    :  0x%x \n\n",tx_oh.tx_z3_z4);
        }
    }
}

static void misc_cli_wis_prbs31_err_inj_conf(cli_req_t * req)
{
    vtss_port_no_t port_no, port;
    vtss_ewis_prbs31_err_inj_t  err_inj;
    misc_cli_req_t          *misc_req = req->module_req;
    
    /*  Debug WIS prbs31_err_inj <port_list> [single_err|sat_err]  */
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->uport_list[port] == 0) || !vtss_phy_10G_is_valid(MISC_INST, port_no)) 
            continue;
        err_inj = misc_req->wis_err_inj;
        if (vtss_ewis_prbs31_err_inj_set(MISC_INST, port_no, &err_inj) != VTSS_RC_OK) {
            CPRINTF("Could not perform vtss_ewis_prbs31_err_inj_set(): %u operation\n",port);
            continue;
        }
    }
}


static void misc_cli_wis_tx_perf_thr_conf(cli_req_t * req)
{
    vtss_port_no_t                  port_no, port;
    vtss_ewis_counter_threshold_t   threshold;
    misc_cli_req_t                  *misc_req = req->module_req;
    
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->uport_list[port] == 0) || !vtss_phy_10G_is_valid(MISC_INST, port_no)) 
            continue;
        if (req->set) {
            threshold.n_ebc_thr_s = misc_req->wis_n_ebc_thr_s; 
            threshold.n_ebc_thr_l = misc_req->wis_n_ebc_thr_l; 
            threshold.f_ebc_thr_l = misc_req->wis_f_ebc_thr_l; 
            threshold.n_ebc_thr_p = misc_req->wis_n_ebc_thr_p; 
            threshold.f_ebc_thr_p = misc_req->wis_f_ebc_thr_p; 
            if (vtss_ewis_counter_threshold_set(MISC_INST, port_no, &threshold) != VTSS_RC_OK) {
                CPRINTF("Could not perform vtss_ewis_counter_threshold_set() for port %u\n",port);
                continue;
            }
        } else {
            if (vtss_ewis_counter_threshold_get(MISC_INST, port_no, &threshold) != VTSS_RC_OK) {
                CPRINTF("Could not perform vtss_ewis_counter_threshold_get(): %u operation\n",port);
                continue;
            }
            CPRINTF("Threshold conf for Port: %u \n",port);
            CPRINTF("---------------------------\n");
            CPRINTF("Section error count (B1) threshold        :%-12u \n",threshold.n_ebc_thr_s);
            CPRINTF("Near end line error count (B2) threshold  :%-12u \n",threshold.n_ebc_thr_l );
            CPRINTF("Far end line error count threshold        :%-12u \n",threshold.f_ebc_thr_l );
            CPRINTF("Path block error count (B3) threshold     :%-12u \n",threshold.n_ebc_thr_p );
            CPRINTF("Far end path error count threshold        :%-12u \n\n",threshold.f_ebc_thr_p );
        }
    }
}
    
static void misc_cli_wis_perf_counters_get(cli_req_t * req)
{
    vtss_port_no_t      port_no, port;
    vtss_ewis_perf_t    perf_pre;

    /* Debug WIS perf_counters [<port_list>]     */
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->uport_list[port] == 0) || !vtss_phy_10G_is_valid(MISC_INST, port_no)) 
            continue;
       
        if (vtss_ewis_perf_get(MISC_INST, port_no, &perf_pre) != VTSS_RC_OK) {
            CPRINTF("Could not perform vtss_ewis_perf_get(): %u operation\n",port);
            continue;
        }
        CPRINTF("WIS performance primitives:\n\n");
        CPRINTF("Port: %u\n",port);
        CPRINTF("--------\n");
//        CPRINTF("SEF Received                          : %-12s\n",perf_pre.psef?"Yes":"No");
        CPRINTF("Section BIP error count               : %-12u\n",perf_pre.pn_ebc_s);
//        CPRINTF("SSF/dTIM/dEQ has occurred             : %-12s\n",perf_pre.pn_ds_s?"Yes":"No");
        CPRINTF("Near end line block (BIP) error count : %-12u\n",perf_pre.pn_ebc_l);
//        CPRINTF("aTSF/dEQ has occurred                 : %-12s\n",perf_pre.pn_ds_l?"Yes":"No");
        CPRINTF("Far end line block (BIP) error count  : %-12u\n",perf_pre.pf_ebc_l);
//        CPRINTF("dRDI has occurred                     : %-12s\n",perf_pre.pf_ds_l?"Yes":"No");
        CPRINTF("Path block error count                : %-12u\n",perf_pre.pn_ebc_p);
//        CPRINTF("CI_SSF/dUNEQ/dTIM/dEQ has occurred    : %-12s\n",perf_pre.pn_ds_p?"Yes":"No");
        CPRINTF("Far end path block error count        : %-12u\n",perf_pre.pf_ebc_p);
//        CPRINTF("dRDI has occurred                     : %-12s\n",perf_pre.pf_ds_p?"Yes":"No");
    }
}

static void misc_cli_wis_event_force_conf(cli_req_t * req)
{
    vtss_port_no_t       port_no, port;
    vtss_ewis_event_t    ev_force;
    misc_cli_req_t       *misc_req = req->module_req;
    
    /* Debug WIS event_force [<port_list>] [<event>] */
    
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->uport_list[port] == 0) || !vtss_phy_10G_is_valid(MISC_INST, port_no)) 
            continue;
        if (req->set) {
            ev_force = misc_req->wis_force_events;
            if (misc_req->enable) {
                if (vtss_ewis_event_force(MISC_INST, port_no, TRUE, ev_force) != VTSS_RC_OK) {
                    CPRINTF("Could not perform vtss_ewis_events_force(): %u operation\n",port);
                    continue;
                }
            } else {
                if (vtss_ewis_event_force(MISC_INST, port_no, FALSE, ev_force) != VTSS_RC_OK) {
                    CPRINTF("Could not perform vtss_ewis_events_force(): %u operation\n",port);
                    continue;
                }
            }
        } else {
            CPRINTF(" WIS event force get not applicable \n");
        }
    }
}

#endif //VTSS_FEATURE_WIS
#ifdef VTSS_FEATURE_SYNCE_10G
static void misc_cli_synce_clkout_set(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    misc_cli_req_t *misc_req = req->module_req;
    BOOL synce_clkout;
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->uport_list[port] == 0) || !vtss_phy_10G_is_valid(MISC_INST, port_no)) 
            continue;
        if (vtss_phy_10g_synce_clkout_get(MISC_INST, port_no, &synce_clkout) != VTSS_RC_OK) {
            CPRINTF("Could not perform vtss_phy_10g_synce_clkout_get() for port %u\n",port);
            continue;
        }
        if (req->set) {
            if(synce_clkout == misc_req->enable) {
                continue;
            }
            synce_clkout = misc_req->enable;
            if (vtss_phy_10g_synce_clkout_set(MISC_INST, port_no, synce_clkout) != VTSS_RC_OK) {
                CPRINTF("Could not perform vtss_phy_10g_synce_clkout_set() for port %u\n",port);
                continue;
            }
        } else {
            CPRINTF("Port: %u\n",port);
            CPRINTF("--------\n");
            CPRINTF("SyncE Clkout :%-12s  \n",synce_clkout?"Enabled":"Disabled");
        }
    }
}

static void misc_cli_xfp_clkout_set(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    misc_cli_req_t *misc_req = req->module_req;
    BOOL xfp_clkout;
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->uport_list[port] == 0) || !vtss_phy_10G_is_valid(MISC_INST, port_no)) 
            continue;
        if (vtss_phy_10g_xfp_clkout_get(MISC_INST, port_no, &xfp_clkout) != VTSS_RC_OK) {
            CPRINTF("Could not perform vtss_phy_10g_xfp_clkout_get() for port %u\n",port);
            continue;
        }
        if (req->set) {
            if(xfp_clkout == misc_req->enable) {
                continue;
            }
            xfp_clkout = misc_req->enable;
            if (vtss_phy_10g_xfp_clkout_set(MISC_INST, port_no, xfp_clkout) != VTSS_RC_OK) {
                CPRINTF("Could not perform vtss_phy_10g_xfp_clkout_set() for port %u\n",port);
                continue;
            }
        } else {
            CPRINTF("Port: %u\n",port);
            CPRINTF("--------\n");
            CPRINTF("XFP Clkout :%-12s  \n",xfp_clkout?"Enabled":"Disabled");
        }
    }
}

static void misc_cli_synce_mode_set(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    misc_cli_req_t   *misc_req = req->module_req;
    vtss_phy_10g_id_t       chip_id;
    vtss_phy_10g_mode_t synce_mode;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->uport_list[port] == 0) || !vtss_phy_10G_is_valid(MISC_INST, port_no)) 
            continue;
        if (vtss_phy_10g_mode_get(MISC_INST, port_no, &synce_mode) != VTSS_RC_OK) {
            CPRINTF("Could not perform vtss_phy_10g_mode_get() for port %u\n",port);
            continue;
        }
        if (vtss_phy_10g_id_get(MISC_INST, port_no, &chip_id) != VTSS_RC_OK) {
            CPRINTF("Could not perform vtss_phy_10g_mode_get() operation");
            continue;
        }
        if (req->set) {
            if (misc_req->synce_mode_set)
                synce_mode.oper_mode = misc_req->synce_mode;
            if (misc_req->synce_clk_out_set)
                synce_mode.rcvrd_clk = misc_req->synce_clk_out;
            if (misc_req->synce_hitless_set)
                synce_mode.hl_clk_synth = misc_req->synce_hitless;
            if (misc_req->synce_rclk_div_set)
                synce_mode.rcvrd_clk_div = misc_req->synce_rclk_div;
            if (misc_req->synce_sref_div_set)
                synce_mode.sref_clk_div = misc_req->synce_sref_div;
            if (misc_req->synce_wref_div_set)
                synce_mode.wref_clk_div = misc_req->synce_wref_div;

            if (vtss_phy_10g_reset(MISC_INST, port_no) != VTSS_RC_OK) {
                CPRINTF("Could not perform a PHY reset port:%u\n",port_no);
            }
            if (vtss_phy_10g_mode_set(MISC_INST, port_no, &synce_mode) != VTSS_RC_OK) {
                CPRINTF("Could not perform vtss_phy_10g_mode_set() for port %u\n",port);
                continue;
            }
        } else {
            if (vtss_phy_10g_mode_get(MISC_INST, port_no, &synce_mode) != VTSS_RC_OK) {
                CPRINTF("Could not perform vtss_phy_10g_mode_get() for port %u\n",port);
                continue;
            }
            CPRINTF("Port: %u\n",port);
            CPRINTF("--------\n");
            CPRINTF("SyncE oper mode      :%d  \n",synce_mode.oper_mode);
            CPRINTF("SyncE hl clk synth   :%d  \n",synce_mode.hl_clk_synth);
            CPRINTF("SyncE rcvrd clk      :%d  \n",synce_mode.rcvrd_clk);
            CPRINTF("SyncE rcvrd  clk div :%d  \n",synce_mode.rcvrd_clk_div);
            CPRINTF("SyncE sref clk div   :%d  \n",synce_mode.sref_clk_div);
            CPRINTF("SyncE wref clk div   :%d  \n",synce_mode.wref_clk_div);
        }
    }
}

static void misc_cli_srefclk_set(cli_req_t *req)
{
    vtss_port_no_t port_no, port;
    misc_cli_req_t *misc_req = req->module_req;
    vtss_phy_10g_srefclk_mode_t srefclk;
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        port = iport2uport(port_no);
        if ((req->uport_list[port] == 0) || !vtss_phy_10G_is_valid(MISC_INST, port_no)) 
            continue;
        if (vtss_phy_10g_srefclk_conf_get(MISC_INST, port_no, &srefclk) != VTSS_RC_OK) {
            CPRINTF("Could not perform vtss_phy_10g_srefclk_conf_get() for port %u\n",port);
            continue;
        }
        if (req->set) {
            if(srefclk.enable == misc_req->enable) {
                continue;
            }
            srefclk.enable = misc_req->enable;
            if (vtss_phy_10g_srefclk_conf_set(MISC_INST, port_no, &srefclk) != VTSS_RC_OK) {
                CPRINTF("Could not perform vtss_phy_10g_srefclk_conf_set() for port %u\n",port);
                continue;
            }
        } else {
            CPRINTF("Port: %u\n",port);
            CPRINTF("--------\n");
            CPRINTF("SREFCLK :%-12s  \n",srefclk.enable?"Enabled":"Disabled");
        }
    }
}


#endif //VTSS_FEATURE_SYNCE_10G


static void misc_cli_cmd_debug_qs(cli_req_t *req)
{
#ifdef VTSS_ARCH_JAGUAR_1
    misc_cli_req_t *misc_req = req->module_req;
    vtss_qs_conf_t qs;
    vtss_port_no_t port_no, port;
    u32 q;

    if (vtss_qs_conf_get(NULL, &qs) != VTSS_RC_OK) {
        CPRINTF("Could not get QS\n");
    }
    if (req->set) {
        for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
            port = iport2uport(port_no);
            if (req->uport_list[port] == 0) {
                continue;
            }

            qs.mode = (vtss_qs_mode_t)misc_req->qs_mode;
            qs.oversubscription = misc_req->oversub;
            qs.port[port_no].port_bytes = misc_req->port_value;
            for (q = 0; q < 8; q++) {
                if (misc_req->prio_list[q] == 0) {
                    continue;
                }
                qs.port[port_no].queue_bytes[q] = misc_req->queue_value;
            }
        }

        if (vtss_qs_conf_set(NULL, &qs) != VTSS_RC_OK) {
            CPRINTF("Could not set QS\n");
        }
    } else {
        CPRINTF("QS mode:%s\n",qs.mode==VTSS_QS_MODE_DISABLED?"DISABLED":qs.mode==VTSS_QS_MODE_STRICT?"STRICT":"SHARED");
        CPRINTF("QS oversubscription:%d pct\n",qs.oversubscription);
        for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
            port = iport2uport(port_no);
            if (req->uport_list[port] == 0) {
                continue;
            }

            CPRINTF("Port:%d  bytes:%d\n",port, qs.port[port_no].port_bytes);
            CPRINTF(" Queues 0-7 bytes:");
            for (q = 0; q < 8; q++) {
                CPRINTF(" %u",qs.port[port_no].queue_bytes[q]);
            }
            CPRINTF("\n");
        }
    }
#endif
}

static void misc_cli_spi_transaction(cli_req_t *req)
{
#ifdef VTSS_ARCH_JAGUAR_1
    u32            loop = 0;
    u8             tx_data[MAX_SPI_DATA], rx_data[MAX_SPI_DATA];
    misc_cli_req_t *misc_req = req->module_req;
 
    memset(rx_data, 0, MAX_SPI_DATA);
    memset(tx_data, 0, MAX_SPI_DATA);
    if (misc_req->list_given) {
        for (loop = 0; loop < misc_req->no_of_bytes; loop++) {
            tx_data[loop] = misc_req->value_list[loop];
        }
    }
    CYG_DEVS_SPI_MIPS_VCOREIII_DEVICE_EXT(spi_dev, misc_req->spi_cs, misc_req->gpio_mask, misc_req->gpio_value);
    cyg_spi_transfer((cyg_spi_device*)&spi_dev, FALSE, misc_req->no_of_bytes, tx_data, rx_data);
    loop = 0;
    CPRINTF("Read Bytes::\n");
    while (loop != misc_req->no_of_bytes) {
       if (loop%10 == 0) {
           CPRINTF("\n");
       }
       CPRINTF("%u   ", rx_data[loop]);
       loop++;
    }
    CPRINTF("\nTotal read:: %u\n", loop);   
#endif /* VTSS_ARCH_JAGUAR_1 */
}

#if defined(VTSS_SW_OPTION_IPV6) && defined(VTSS_FEATURE_IPV6_MC_SIP)
static void misc_cli_ipv6_get(vtss_ipv6_t *sip, vtss_ipv6_t *dip, 
                              misc_cli_req_t *misc_req)
{
    u32 i, n;
    
    memset(sip, 0, sizeof(*sip));
    memset(dip, 0, sizeof(*dip));
    
    for (i = 0; i < 4; i++) {
        n = ((3 - i)*8);
        sip->addr[i + 12] = ((misc_req->sip >> n) & 0xff);
        dip->addr[i + 12] = ((misc_req->dip >> n) & 0xff);
    }
}
#endif /* VTSS_FEATURE_IPV6_MC_SIP */

#if defined(VTSS_FEATURE_IPV4_MC_SIP)
static void misc_cli_ipmc_add(cli_req_t *req)
{
    misc_cli_req_t *misc_req = req->module_req;
    BOOL           member[VTSS_PORT_ARRAY_SIZE];
    vtss_port_no_t port_no;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++)
        member[port_no] = req->uport_list[iport2uport(port_no)];
    
    if (misc_req->ipv6) {
#if defined(VTSS_SW_OPTION_IPV6) && defined(VTSS_FEATURE_IPV6_MC_SIP)
        vtss_ipv6_t sip, dip;

        misc_cli_ipv6_get(&sip, &dip, misc_req);
        (void)vtss_ipv6_mc_add(NULL, req->vid, sip, dip, member);
#endif /* VTSS_FEATURE_IPV6_MC_SIP */
    } else {
        (void)vtss_ipv4_mc_add(NULL, req->vid, misc_req->sip, misc_req->dip, member);
    }
}

static void misc_cli_ipmc_del(cli_req_t *req)
{
    misc_cli_req_t *misc_req = req->module_req;

    if (misc_req->ipv6) {
#if defined(VTSS_SW_OPTION_IPV6) && defined(VTSS_FEATURE_IPV6_MC_SIP)
        vtss_ipv6_t sip, dip;

        misc_cli_ipv6_get(&sip, &dip, misc_req);
        (void)vtss_ipv6_mc_del(NULL, req->vid, sip, dip);
#endif /* VTSS_FEATURE_IPV6_MC_SIP */
    } else {
        (void)vtss_ipv4_mc_del(NULL, req->vid, misc_req->sip, misc_req->dip);
    }
}
#endif /* VTSS_FEATURE_IPV4_MC_SIP */

/****************************************************************************/
/*  Parameter functions                                                     */
/****************************************************************************/

#ifdef VTSS_SW_OPTION_I2C
static int32_t cli_misc_i2c_data_parse(char *cmd, char *cmd2, char *stx,
                                       char *cmd_org, cli_req_t *req)
{
    int32_t        error;
    ulong          value = 0;
    misc_cli_req_t *misc_req = req->module_req;

    error = cli_parse_ulong(cmd, &value, 0, 255);
    misc_req->i2c_data[misc_req->i2c_data_cnt] = value;
    misc_req->i2c_data_cnt++;
    return error;
}


static int32_t cli_misc_i2c_addr_parse(char *cmd, char *cmd2, char *stx,
                                       char *cmd_org, cli_req_t *req)
{
    int32_t        error;
    ulong          value = 0;
    misc_cli_req_t *misc_req = req->module_req;

    error = cli_parse_ulong(cmd, &value, 0, 255);
    misc_req->i2c_addr = value;
    return error;
}

static int32_t cli_misc_sfp_reg_addr_parse(char *cmd, char *cmd2, char *stx,
                                           char *cmd_org, cli_req_t *req)
{
    int32_t        error;
    ulong          value = 0;
    misc_cli_req_t *misc_req = req->module_req;

    error = cli_parse_ulong(cmd, &value, 0, 255);
    misc_req->sfp_reg_addr[misc_req->i2c_addr_cnt] = value;
    T_D("cli_misc_sfp_reg_addr_parse value = %u, i2c_addr_cnt =%u", value, misc_req->i2c_addr_cnt);
    misc_req->i2c_addr_cnt++;
    return error;
}

static int32_t cli_misc_i2c_bytes_parse(char *cmd, char *cmd2, char *stx,
                                        char *cmd_org, cli_req_t *req)
{
    int32_t        error;
    ulong          value = 0;
    misc_cli_req_t *misc_req = req->module_req;

    error = cli_parse_ulong(cmd, &value, 0, 40);
    misc_req->i2c_data_cnt = value;
    return error;
}

static int32_t cli_misc_i2c_clk_sel_parse(char *cmd, char *cmd2, char *stx,
                                          char *cmd_org, cli_req_t *req)
{
    int32_t        error;
    ulong          value = 0;
    misc_cli_req_t *misc_req = req->module_req;

    error = cli_parse_ulong(cmd, &value, 0, 40);
    misc_req->i2c_clk_sel = value;
    return error;
}

#endif /* VTSS_SW_OPTION_I2C */

static int32_t cli_misc_serial_data_parse(char *cmd, char *cmd2, char *stx,
                                          char *cmd_org, cli_req_t *req)
{
    int32_t        error;
    ulong          value = 0;
    misc_cli_req_t *misc_req = req->module_req;

    error = cli_parse_ulong(cmd, &value, 0, 0xFFFFFFFF);
    misc_req->serialized_gpio_data[misc_req->sgpio_bit] = value;
    misc_req->sgpio_bit++;
    return error;
}


static int32_t cli_misc_chip_no_parse(char *cmd, char *cmd2, char *stx,
                                      char *cmd_org, cli_req_t *req)
{
    int32_t        error;
    ulong          value = VTSS_CHIP_NO_ALL;
    misc_cli_req_t *misc_req = req->module_req;

    if ((error = cli_parse_all(cmd)) == 0 ||
        (error = cli_parse_ulong(cmd, &value, 0, 1)) == 0) {
        misc_req->chip_no = value;
    }
    return error;
}

static int32_t cli_misc_addr_list_parse(char *cmd, char *cmd2, char *stx,
                                        char *cmd_org, cli_req_t *req)
{
    ulong          max = CLI_ADDR_MAX;
#if defined(VTSS_ARCH_JAGUAR_1)
    misc_cli_req_t *misc_req = req->module_req;
    ulong          tgt;

    /* For targets below 128, only 14-bit addresses are allowed */
    for (tgt = 0; tgt < 128; tgt++) {
        if (misc_req->blk_list[tgt]) {
            max = (1 << 14);
            break;
        }
    }
#endif /* VTSS_ARCH_JAGUAR_1 */    

    return (mgmt_txt2bf(cmd, misc_addr_bf, 0, max - 1, 0) == VTSS_OK ? 0 : 1);
}

static int32_t cli_misc_phy_addr_list_parse(char *cmd, char *cmd2, char *stx,
                                            char *cmd_org, cli_req_t *req)
{
    misc_cli_req_t *misc_req = req->module_req;

    return cli_parm_parse_list(cmd, misc_req->addr_list, 0, CLI_PHY_MAX - 1, 0);

}


static int32_t cli_misc_clock_src_parse(char *cmd, char *cmd2, char *stx,
                                        char *cmd_org, cli_req_t *req)
{
    misc_cli_req_t *misc_req = req->module_req;
    char           *found = cli_parse_find(cmd, stx);

    if(!found)      return 1;
    else if(!strncmp(found, "serdes", 6))      misc_req->clock_src = VTSS_PHY_SERDES_MEDIA;
    else if(!strncmp(found, "copper", 6))      misc_req->clock_src = VTSS_PHY_COPPER_MEDIA;
    else if(!strncmp(found, "tclk", 4))        misc_req->clock_src = VTSS_PHY_TCLK_OUT;
    else if(!strncmp(found, "xtal", 4))        misc_req->clock_src = VTSS_PHY_LOCAL_XTAL;
    else if(!strncmp(found, "disable", 7))     misc_req->clock_src = VTSS_PHY_CLK_DISABLED;
    else return 1;

    return 0;
}

static int32_t cli_misc_clock_freq_parse(char *cmd, char *cmd2, char *stx,
                                         char *cmd_org, cli_req_t *req)
{
    misc_cli_req_t *misc_req = req->module_req;
    char           *found = cli_parse_find(cmd, stx);

    if(!found)      return 1;
    else if(!strncmp(found, "25m", 3))      misc_req->clock_freq = VTSS_PHY_FREQ_25M;
    else if(!strncmp(found, "125m", 4))     misc_req->clock_freq = VTSS_PHY_FREQ_125M;
    else if(!strncmp(found, "3125m", 5))    misc_req->clock_freq = VTSS_PHY_FREQ_3125M;
    else return 1;

    return 0;
}

static int32_t cli_misc_clock_phy_port_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    int32_t        error;
    ulong          value = 0;
    misc_cli_req_t *misc_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 1, VTSS_PORTS);
    misc_req->clock_phy_port = value-1;
    return error;

}

static int32_t cli_misc_clock_autoneg_port_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    int32_t        error;
    ulong          value = 0;
    misc_cli_req_t *misc_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 1, VTSS_PORTS);
    misc_req->autoneg_phy_port = value-1;
    return error;
}



static int32_t cli_misc_clock_port_parse(char *cmd, char *cmd2, char *stx,
                                         char *cmd_org, cli_req_t *req)
{
    int32_t        error;
    ulong          value = 0;
    misc_cli_req_t *misc_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 1, 2);
    misc_req->clock_port = value-1;
    return error;

}

static int32_t cli_misc_clock_squelch_parse(char *cmd, char *cmd2, char *stx,
                                            char *cmd_org, cli_req_t *req)
{
    int32_t        error;
    ulong          value = 0;
    misc_cli_req_t *misc_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 0, 3);
    misc_req->clock_squelch = value;
    return error;

}

static int32_t cli_misc_master_parse(char *cmd, char *cmd2, char *stx,
                                           char *cmd_org, cli_req_t *req) 
{
    int32_t        error;
    ulong          value = 0;
    misc_cli_req_t *misc_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 0, 1);
    misc_req->aneg_master_config = value;
    return error;
}

static int32_t cli_misc_phy_i2c_reg_addr_parse(char *cmd, char *cmd2, char *stx,
                                               char *cmd_org, cli_req_t *req)
{
    int32_t        error;
    ulong          value = 0;
    misc_cli_req_t *misc_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 0, 255);
    misc_req->i2c_reg_addr = value;
    return error;

}

static int32_t cli_misc_phy_i2c_mux_parse(char *cmd, char *cmd2, char *stx,
                                          char *cmd_org, cli_req_t *req)
{
    int32_t        error;
    ulong          value = 0;
    misc_cli_req_t *misc_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 0, 3);
    misc_req->i2c_mux = value;
    return error;

}

static int32_t cli_misc_phy_i2c_device_addr_parse(char *cmd, char *cmd2, char *stx,
                                                  char *cmd_org, cli_req_t *req)
{
    int32_t        error;
    ulong          value = 0;
    misc_cli_req_t *misc_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 0, 127);
    misc_req->i2c_device_addr = value;
    return error;

}

static int32_t cli_misc_phy_mmd_addr_list_parse(char *cmd, char *cmd2, char *stx,
                                            char *cmd_org, cli_req_t *req)
{
    misc_cli_req_t *misc_req = req->module_req;

    return cli_parm_parse_list(cmd, misc_req->addr_list, 0, 0xFF, 0);
}

static int32_t cli_misc_blk_list_parse(char *cmd, char *cmd2, char *stx,
                                       char *cmd_org, cli_req_t *req)
{
    misc_cli_req_t *misc_req = req->module_req;

    return cli_parm_parse_list(cmd, misc_req->blk_list, 0, CLI_BLK_MAX - 1, 0);
}

static int32_t cli_misc_value_parse(char *cmd, char *cmd2, char *stx,
                                    char *cmd_org, cli_req_t *req)
{
    return cli_parse_ulong(cmd, &req->value, 0, 0xffffffff);
}

static int32_t cli_misc_ob_post0_value_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    return cli_parse_ulong(cmd, &req->value, 0, 0xffffffff);
}

static int32_t cli_misc_ib_cterm_value_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    misc_cli_req_t *misc_req = req->module_req;
    return cli_parse_ulong(cmd, &misc_req->ib_cterm_value, 0, 255);
}

static int32_t cli_misc_ib_eq_mode_value_parse(char *cmd, char *cmd2, char *stx,
                                               char *cmd_org, cli_req_t *req)
{
    misc_cli_req_t *misc_req = req->module_req;
    return cli_parse_ulong(cmd, &misc_req->ib_eq_mode, 0, 255);
}

#if defined(VTSS_CHIP_10G_PHY)
static int32_t cli_misc_gpio_value_parse(char *cmd, char *cmd2, char *stx,
                                         char *cmd_org, cli_req_t *req)
{
    return cli_parse_ulong(cmd, &req->value, 0, 1);
}
static int32_t cli_misc_phy_addr_parse(char *cmd, char *cmd2, char *stx,
                                       char *cmd_org, cli_req_t *req)
{
    return cli_parse_ulong(cmd, &req->addr, 0, 0xffffffff);
}
#endif /* defined(VTSS_CHIP_10G_PHY) */

static int32_t cli_misc_phy_value_parse(char *cmd, char *cmd2, char *stx,
                                        char *cmd_org, cli_req_t *req)
{
    return cli_parse_ulong(cmd, &req->value, 0, 0xffffffff);
}



static int32_t cli_misc_phy_devad_parse(char *cmd, char *cmd2, char *stx,
                                        char *cmd_org, cli_req_t *req)
{
    misc_cli_req_t *misc_req = req->module_req;
    return cli_parse_ulong(cmd, &misc_req->devad, 0, 31);
}
#if defined(VTSS_FEATURE_10G)
static int32_t cli_misc_phy_miim_parse(char *cmd, char *cmd2, char *stx,
                                        char *cmd_org, cli_req_t *req)
{
    misc_cli_req_t *misc_req = req->module_req;
    return cli_parse_ulong(cmd, &misc_req->miim_addr, 0, 31);
}

static int32_t cli_misc_phy_miim_ctrl_parse(char *cmd, char *cmd2, char *stx,
                                            char *cmd_org, cli_req_t *req)
{
    misc_cli_req_t *misc_req = req->module_req;
    return cli_parse_ulong(cmd, &misc_req->miim_ctrl, 0, 1);
}
#endif /* VTSS_FEATURE_10G */
static int32_t cli_misc_phy_mmd_reg_addr_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    misc_cli_req_t *misc_req = req->module_req;
    return cli_parse_ulong(cmd, &misc_req->mmd_reg_addr, 0, 0xffff);
}

static int32_t cli_misc_page_parse(char *cmd, char *cmd2, char *stx,
                                   char *cmd_org, cli_req_t *req)
{
    misc_cli_req_t *misc_req = req->module_req;

    return cli_parse_ulong(cmd, &misc_req->page, 0, 0xffff);
}

static int32_t cli_misc_lb_parse(char *cmd, char *cmd2, char *stx,
                                 char *cmd_org, cli_req_t *req)
{
    misc_cli_req_t *misc_req = req->module_req;
    misc_req->lb = *cmd;
    return 0;
}

static int32_t cli_misc_failover_parse(char *cmd, char *cmd2, char *stx,
                                 char *cmd_org, cli_req_t *req)
{
    misc_cli_req_t *misc_req = req->module_req;
    misc_req->failover = *cmd;
    return 0;
}

static int32_t cli_misc_gpio_list_parse(char *cmd, char *cmd2, char *stx,
                                        char *cmd_org, cli_req_t *req)
{
    misc_cli_req_t *misc_req = req->module_req;

    return cli_parm_parse_list(cmd, misc_req->gpio_list, 0, 11, 0);
}

static int32_t cli_misc_keyword_parse(char *cmd, char *cmd2, char *stx,
                                      char *cmd_org, cli_req_t *req)
{
    misc_cli_req_t *misc_req = req->module_req;
    char           *found = cli_parse_find(cmd, stx);

    if (found != NULL) {
        if (!strncmp(found, "addr_sort", 7)) {
            misc_req->addr_sort = 1;
        } else if (!strncmp(found, "ail", 3)) {
            misc_req->layer = VTSS_DEBUG_LAYER_AIL;
        } else if (!strncmp(found, "clear", 5)) {
            misc_req->clear = 1;
        } else if (!strncmp(found, "cil", 3)) {
            misc_req->layer = VTSS_DEBUG_LAYER_CIL;
        } else if (!strncmp(found, "detailed", 8)) {
            misc_req->detailed = 1;
        } else if (!strncmp(found, "full", 4)) {
            misc_req->full = 1;
        } else if (!strncmp(found, "disable", 7)) {
            misc_req->disable = 1;
        } else if (!strncmp(found, "enable", 6)) {
            misc_req->enable = 1;
        } else if (!strncmp(found, "ipv6", 4)) {
            misc_req->ipv6 = 1;
        } else if (!strncmp(found, "wan", 3)) {
            misc_req->lan = 0;
        } else if (!strncmp(found, "lan", 3)) {
            misc_req->lan = 1;
        } else if (!strncmp(found, "1g", 2)) {
            misc_req->lan = 2;
        } else if (!strncmp(found, "input", 5)) {
            misc_req->gpio_role = 0;
        } else if (!strncmp(found, "output", 6)) {
            misc_req->gpio_role = 1;
        } else if (!strncmp(found, "pcs_rx_fault", 12)) {
            misc_req->gpio_role = 3;
        } else if (!strncmp(found, "phy_inst", 8)) {
            misc_req->phy_inst = 1;
#ifdef VTSS_ARCH_JAGUAR_1
        } else if (!strncmp(found, "shared", 6)) {
            misc_req->qs_mode = VTSS_QS_MODE_SHARED;
        } else if (!strncmp(found, "strict", 6)) {
            misc_req->qs_mode = VTSS_QS_MODE_STRICT;
#endif // VTSS_ARCH_JAGUAR_1
#ifdef  VTSS_FEATURE_WIS
        }else if (!strncmp(found, "loopback", 8)) {
            misc_req->wis_loopback = 1;
            misc_req->wis_loopback_set = 1;
        }else if (!strncmp(found, "no_loopback", 11)) {
            misc_req->wis_loopback = 0;
            misc_req->wis_loopback_set = 1;
#endif //VTSS_FEATURE_WIS
        }  
    }

    return (found == NULL ? 1 : 0);
}

// Parsing 1G PHY loopback 
static int32_t cli_misc_phy_loopback_parse(char *cmd, char *cmd2, char *stx,
                                      char *cmd_org, cli_req_t *req)
{
    misc_cli_req_t *misc_req = req->module_req;
    char           *found = cli_parse_find(cmd, stx);
    if (found != NULL) {
        if (!strncmp(found, "far", 3)) {
            misc_req->lb_type = VTSS_LB_FAR_END;
        } else if (!strncmp(found, "near", 4)) {
            misc_req->lb_type = VTSS_LB_NEAR_END;
        } 
    }

    return (found == NULL ? 1 : 0);
    
}

#ifdef  VTSS_FEATURE_WIS
static int32_t cli_misc_wis_gen_mode_parse(char *cmd, char *cmd2, char *stx,
                                      char *cmd_org, cli_req_t *req)
{
    misc_cli_req_t *misc_req = req->module_req;
    char           *found = cli_parse_find(cmd, stx);

    if (found != NULL) {
        if (!strncmp(found, "gen_dis", 7)) {
            misc_req->wis_gen_mode = VTSS_WIS_TEST_MODE_DISABLE;
        } else if (!strncmp(found, "gen_sqr", 7)) {
            misc_req->wis_gen_mode = VTSS_WIS_TEST_MODE_SQUARE_WAVE;
        } else if (!strncmp(found, "gen_prbs31", 10)) {
            misc_req->wis_gen_mode = VTSS_WIS_TEST_MODE_PRBS31;
        } else if (!strncmp(found, "gen_mix", 7)) {
            misc_req->wis_gen_mode = VTSS_WIS_TEST_MODE_MIXED_FREQUENCY;
        } 
    }

    return (found == NULL ? 1 : 0);
    
}


static int32_t cli_misc_wis_ana_mode_parse(char *cmd, char *cmd2, char *stx,
                                      char *cmd_org, cli_req_t *req)
{
    misc_cli_req_t *misc_req = req->module_req;
    char           *found = cli_parse_find(cmd, stx);

    if (found != NULL) {
        if (!strncmp(found, "ana_dis", 7)) {
            misc_req->wis_ana_mode = VTSS_WIS_TEST_MODE_DISABLE;
        } else if (!strncmp(found, "ana_prbs31", 10)) {
            misc_req->wis_ana_mode = VTSS_WIS_TEST_MODE_PRBS31;
        } else if (!strncmp(found, "ana_mix", 7)) {
            misc_req->wis_ana_mode = VTSS_WIS_TEST_MODE_MIXED_FREQUENCY;
        } 
    }
    return (found == NULL ? 1 : 0);
}
static int32_t cli_misc_wis_aisl_parse(char *cmd, char *cmd2, char *stx,
                                      char *cmd_org, cli_req_t *req)
{
    int32_t        error;
    ulong          value = 0;
    misc_cli_req_t *misc_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 0, 0x3);
    misc_req->wis_aisl = value;
    return error;
}
static int32_t cli_misc_wis_rdil_parse(char *cmd, char *cmd2, char *stx,
                                      char *cmd_org, cli_req_t *req)
{
    int32_t        error;
    ulong          value = 0;
    misc_cli_req_t *misc_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 0, 0xF);
    misc_req->wis_rdil = value;
    return error;
}
static int32_t cli_misc_wis_fault_parse(char *cmd, char *cmd2, char *stx,
                                      char *cmd_org, cli_req_t *req)
{
    int32_t        error;
    ulong          value = 0;
    misc_cli_req_t *misc_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 0, 0x7FF);
    misc_req->wis_fault = value;
    return error;
}

static int32_t cli_misc_wis_overhead_parse(char *cmd, char *cmd2, char *stx,
                                      char *cmd_org, cli_req_t *req)
{
    int32_t        error;
    ulong          value = 0;
    misc_cli_req_t *misc_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 0, 255);
    misc_req->wis_overhead = value;
    misc_req->wis_overhead_set=1;
    return error;
}

static int32_t cli_misc_wis_tti_mode_parse(char *cmd, char *cmd2, char *stx,
                                      char *cmd_org, cli_req_t *req)
{
    int32_t        error;
    ulong          value = 0;
    misc_cli_req_t *misc_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 0, 2);
    misc_req->wis_tti_mode = value;
    misc_req->wis_tti_mode_set = 1;
    return error;
}
static int32_t cli_misc_wis_tti_parse(char *cmd, char *cmd2, char *stx,
                                      char *cmd_org, cli_req_t *req)
{
    int32_t         error = 0;
    u32             length = 0, explen=0;
    misc_cli_req_t *misc_req = req->module_req;

    length = strlen(cmd_org);
    switch (misc_req->wis_tti_mode){
        case TTI_MODE_1:
            explen = 1;
            break;
        case TTI_MODE_16:
            explen = 16;
            break;
        case TTI_MODE_64:
            explen = 64;
            break;
        default:
            CPRINTF("TTI Mode mismatch\n");
            error = 1;
            return error;
    }
    if (length > explen) {
        CPRINTF("TTI Length exceeds\n");
        error = 1;
        return error;
    }
    req->parm_parsed = 1;
    misc_req->wis_tti_set=1;
    error = cli_parse_quoted_string(cmd_org, (char *)misc_req->wis_tti, explen);
    return error;
}

static int32_t cli_misc_wis_line_rx_parse(char *cmd, char *cmd2, char *stx,
                                      char *cmd_org, cli_req_t *req)
{
    int32_t        error;
    ulong          value = 0;
    misc_cli_req_t *misc_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 0, 3);
    misc_req->wis_line_rx = value;
    return error;
}
static int32_t cli_misc_wis_line_tx_parse(char *cmd, char *cmd2, char *stx,
                                      char *cmd_org, cli_req_t *req)
{
    int32_t        error;
    ulong          value = 0;
    misc_cli_req_t *misc_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 0, 3);
    misc_req->wis_line_tx = value;
    return error;
}
static int32_t cli_misc_wis_path_force_parse(char *cmd, char *cmd2, char *stx,
                                      char *cmd_org, cli_req_t *req)
{
    int32_t        error;
    ulong          value = 0;
    misc_cli_req_t *misc_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 0, 3);
    misc_req->wis_path_force = value;
    return error;
}
static int32_t cli_misc_wis_overhead_name_parse(char *cmd, char *cmd2, char *stx,
                                      char *cmd_org, cli_req_t *req)
{
    int32_t        error=0;
    misc_cli_req_t *misc_req = req->module_req;
    if (!strncmp(cmd_org, "ALL", 3)) {
        misc_req->wis_oh_id = 1;
    } else if (!strncmp(cmd_org, "SEC-OH", 6)) {
        misc_req->wis_oh_id = 2;
    } else if (!strncmp(cmd_org, "LINE-OH", 7)) {
        misc_req->wis_oh_id = 3;
    } else if (!strncmp(cmd_org, "PATH-OH", 7)) {
        misc_req->wis_oh_id = 4;
    } else if (!strncmp(cmd_org, "D1-D3", 5)) {
        misc_req->wis_oh_id = 5;
    } else if (!strncmp(cmd_org, "SEC-ORD", 7)) {
        misc_req->wis_oh_id = 6;
    } else if (!strncmp(cmd_org, "SUC", 3)) {
        misc_req->wis_oh_id = 7;
    } else if (!strncmp(cmd_org, "RES-SG", 6)) {
        misc_req->wis_oh_id = 8;
    } else if (!strncmp(cmd_org, "D4-D12", 6)) {
        misc_req->wis_oh_id = 9;
    } else if (!strncmp(cmd_org, "LINE-ORD", 8)) {
        misc_req->wis_oh_id = 10;
    } else if (!strncmp(cmd_org, "APS-RDIL", 8)) {
        misc_req->wis_oh_id = 11;
    } else if (!strncmp(cmd_org, "SYNC", 4)) {
        misc_req->wis_oh_id = 12;
    } else if (!strncmp(cmd_org, "RES-LG", 6)) {
        misc_req->wis_oh_id = 13;
    } else if (!strncmp(cmd_org, "C2PL", 4)) {
        misc_req->wis_oh_id = 14;
    } else if (!strncmp(cmd_org, "PUC", 3)) {
        misc_req->wis_oh_id = 15;
    } else if (!strncmp(cmd_org, "PTCM", 4)) {
        misc_req->wis_oh_id = 16;
    } else if (!strncmp(cmd_org, "RES-PG", 6)) {
        misc_req->wis_oh_id = 17;
    }  else {
        
        error = 1;
    }
    return error;
}

static u8 hex2bin(char c)
{
    switch ( c ) {
    case '0':           return 0x0;
    case '1':           return 0x1;
    case '2':           return 0x2;
    case '3':           return 0x3;
    case '4':           return 0x4;
    case '5':           return 0x5;
    case '6':           return 0x6;
    case '7':           return 0x7;
    case '8':           return 0x8;
    case '9':           return 0x9;
    case 'A': case 'a': return 0xa;
    case 'B': case 'b': return 0xb;
    case 'C': case 'c': return 0xc;
    case 'D': case 'd': return 0xd;
    case 'E': case 'e': return 0xe;
    case 'F': case 'f': return 0xf;
    default:            return (0xFF);
    }
}

static int32_t cli_misc_wis_overhead_value_parse(char *cmd, char *cmd2, char *stx,
                                      char *cmd_org, cli_req_t *req)
{
    int32_t        error=1;
    misc_cli_req_t *misc_req = req->module_req;
    u32             length = 0;
    u8              val,val1,i =0,j;
    
    length = strlen(cmd_org);
    if ((length > 54)||(length < 1)) {
        CPRINTF("Overhead value error\n");
        return error;
    }
    
    if (!strncmp(cmd_org,"0x",2)) 
        i = 2;
    
    for (j=0;i<length;i+=2,j++){

        val = hex2bin(cmd_org[i]);
        if (val==0xFF) 
            return error;
        val1 = hex2bin(cmd_org[i+1]);
        if (val1==0xFF)
            return error;
        val = (val<<4)| val1; 
        misc_req->wis_oh_val[j]= val;
    }
    
    return 0;
}


static int32_t cli_misc_wis_n_ebc_thr_s_parse(char *cmd, char *cmd2, char *stx,
                                      char *cmd_org, cli_req_t *req)
{
    int32_t        error;
    ulong          value = 0;
    misc_cli_req_t *misc_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 0, 0xFFFFFFFF);
    misc_req->wis_n_ebc_thr_s = value;
    return error;
}
static int32_t cli_misc_wis_n_ebc_thr_l_parse(char *cmd, char *cmd2, char *stx,
                                      char *cmd_org, cli_req_t *req)
{
    int32_t        error;
    ulong          value = 0;
    misc_cli_req_t *misc_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 0, 0xFFFFFFFF);
    misc_req->wis_n_ebc_thr_l = value;
    return error;
}
static int32_t cli_misc_wis_f_ebc_thr_l_parse(char *cmd, char *cmd2, char *stx,
                                      char *cmd_org, cli_req_t *req)
{
    int32_t        error;
    ulong          value = 0;
    misc_cli_req_t *misc_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 0, 0xFFFFFFFF);
    misc_req->wis_f_ebc_thr_l = value;
    return error;
}
static int32_t cli_misc_wis_n_ebc_thr_p_parse(char *cmd, char *cmd2, char *stx,
                                      char *cmd_org, cli_req_t *req)
{
    int32_t        error;
    ulong          value = 0;
    misc_cli_req_t *misc_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 0, 0xFFFFFFFF);
    misc_req->wis_n_ebc_thr_p = value;
    return error;
}
static int32_t cli_misc_wis_f_ebc_thr_p_parse(char *cmd, char *cmd2, char *stx,
                                      char *cmd_org, cli_req_t *req)
{
    int32_t        error;
    ulong          value = 0;
    misc_cli_req_t *misc_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 0, 0xFFFFFFFF);
    misc_req->wis_f_ebc_thr_p = value;
    return error;
}
static int32_t cli_misc_wis_prbs31_err_inj_parse(char *cmd, char *cmd2, char *stx,
                                      char *cmd_org, cli_req_t *req)
{
    misc_cli_req_t *misc_req = req->module_req;
    char           *found = cli_parse_find(cmd, stx);

    if (found != NULL) {
        if (!strncmp(found, "single_err", 10)) {
            misc_req->wis_err_inj = 0;
        } else if (!strncmp(found, "sat_err", 7)) {
            misc_req->wis_err_inj = 1;
        }
    }
    return (found == NULL ? 1 : 0);
}

static int32_t cli_misc_wis_force_events_parse(char *cmd, char *cmd2, char *stx,
                                      char *cmd_org, cli_req_t *req)
{
    int32_t        error;
    ulong          value = 0;
    misc_cli_req_t *misc_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 0, 0xFFFFFFFF);
    misc_req->wis_force_events = value;
    return error;
}

#endif //VTSS_FEATURE_WIS


#if defined (VTSS_FEATURE_SYNCE_10G)
static int32_t cli_misc_synce_mode_parse(char *cmd, char *cmd2, char *stx,
                                         char *cmd_org, cli_req_t *req)
{
    int32_t        error;
    ulong          value = 0;
    misc_cli_req_t *misc_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 1, 5);
    misc_req->synce_mode = value;
    misc_req->synce_mode_set = 1;
    return error;
}

static int32_t cli_misc_synce_clk_out_parse(char *cmd, char *cmd2, char *stx,
                                            char *cmd_org, cli_req_t *req)
{
    int32_t        error;
    ulong          value = 0;
    misc_cli_req_t *misc_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 0, 255);
    misc_req->synce_clk_out = value;
    misc_req->synce_clk_out_set = 1;
    return error;
}

static int32_t cli_misc_synce_hitless_parse(char *cmd, char *cmd2, char *stx,
                                            char *cmd_org, cli_req_t *req)
{
    int32_t        error;
    ulong          value = 0;
    misc_cli_req_t *misc_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 0, 255);
    misc_req->synce_hitless = value;
    misc_req->synce_hitless_set = 1;
    return error;
}

static int32_t cli_misc_synce_rclk_div_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    int32_t        error;
    ulong          value = 0;
    misc_cli_req_t *misc_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 0, 255);
    misc_req->synce_rclk_div = value;
    misc_req->synce_rclk_div_set = 1;
    return error;
}
static int32_t cli_misc_synce_sref_div_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    int32_t        error;
    ulong          value = 0;
    misc_cli_req_t *misc_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 0, 255);
    misc_req->synce_sref_div = value;
    misc_req->synce_sref_div_set = 1;
    return error;
}

static int32_t cli_misc_synce_wref_div_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    int32_t        error;
    ulong          value = 0;
    misc_cli_req_t *misc_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 0, 255);
    misc_req->synce_wref_div = value;
    misc_req->synce_wref_div_set = 1;
    return error;
}
#endif /* VTSS_FEATURE_SYNCE_10G */

static int32_t cli_misc_spi_cs_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    int32_t        error;
    ulong          value = 0;
    misc_cli_req_t *misc_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 1, 3);
    misc_req->spi_cs = value;
    return error;
}
#ifdef VTSS_FEATURE_MACSEC
static int32_t cli_misc_block_parse(char *cmd, char *cmd2, char *stx,
                                    char *cmd_org, cli_req_t *req)
{
    int32_t        error;
    ulong          value = 0;
    misc_cli_req_t *misc_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 0, 0xFFFFFFFF);
    misc_req->macsec_blk = value;
    return error;
}

static int32_t cli_misc_addr_parse(char *cmd, char *cmd2, char *stx,
                                   char *cmd_org, cli_req_t *req)
{
    int32_t        error;
    ulong          value = 0;
    misc_cli_req_t *misc_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 0, 0xFFFFFFFF);
    misc_req->macsec_addr = value;
    return error;
}
#endif /* VTSS_FEATURE_MACSEC */
static int32_t cli_misc_gpio_val_parse(char *cmd, char *cmd2, char *stx,
                                                    char *cmd_org, cli_req_t *req)
{
    int32_t        error;
    ulong          value = 0;
    misc_cli_req_t *misc_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 0, 0xFFFFFFFF);
    misc_req->gpio_value = value;
    return error;
}


static int32_t cli_misc_gpio_mask_parse (char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    int32_t        error;
    ulong          value = 0;
    misc_cli_req_t *misc_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 0, 0xFFFFFFFF);
    misc_req->gpio_mask = value;
    return error;
}

static int32_t cli_misc_value_list_parse(char *cmd, char *cmd2, char *stx,
                                        char *cmd_org, cli_req_t *req)
{
    int32_t         error;
    misc_cli_req_t  *misc_req = req->module_req;
    char            *ptr;
    char            list[200];
    u32             value, count = 0;

    error = cli_parse_string(cmd_org, list, 0, 200);
    if (error) {
        return error;
    }
    ptr = strtok(list, ","); 
    while (ptr) {
        error = cli_parse_ulong(ptr, &value, 0, 0xFF);
        if (error) {
            CPRINTF("Invalid Value %s in <value_list>", ptr);
            return error;
        }
        if (count >= MAX_SPI_DATA) {
            CPRINTF("Error: Maximum SPI data i/p is 50\n");
            return 1;
        }
        misc_req->value_list[count++] = value;
        ptr = strtok(NULL, ",");
    }
    misc_req->list_given = TRUE;
    return error;
}

static int32_t cli_misc_no_of_bytes_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    int32_t        error;
    ulong          value = 0;
    misc_cli_req_t *misc_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 1, MAX_SPI_DATA);
    misc_req->no_of_bytes = value;
    return error;
}

static int32_t cli_misc_layer_parse(char *cmd, char *cmd2, char *stx,
                                    char *cmd_org, cli_req_t *req)
{
    return cli_misc_keyword_parse(cmd, cmd2, "ail|cil", cmd_org, req);
}

static int32_t cli_misc_mode_parse(char *cmd, char *cmd2, char *stx,
                                   char *cmd_org, cli_req_t *req)
{
    return cli_misc_keyword_parse(cmd, cmd2, "strict|shared", cmd_org, req);
}

static int32_t cli_misc_port_value_parse(char *cmd, char *cmd2, char *stx,
                                         char *cmd_org, cli_req_t *req)
{
    int32_t        error;
    ulong          value = 0;
    misc_cli_req_t *misc_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 0, 4000000);
    misc_req->port_value = value;
    return error;
}

static int32_t cli_misc_queue_value_parse(char *cmd, char *cmd2, char *stx,
                                          char *cmd_org, cli_req_t *req)
{
    int32_t        error;
    ulong          value = 0;
    misc_cli_req_t *misc_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 0, 4000000);
    misc_req->queue_value = value;
    return error;
}
static int32_t cli_misc_queue_oversub_parse(char *cmd, char *cmd2, char *stx,
                                            char *cmd_org, cli_req_t *req)
{
    int32_t        error;
    ulong          value = 0;
    misc_cli_req_t *misc_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 0, 100);
    misc_req->oversub = value;
    return error;
}

static int32_t cli_misc_assert_parse(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
    misc_cli_req_t *misc_req = req->module_req;
    int32_t result = 0;

    if (!strncmp(cmd, "appl", 4)) {
        misc_req->assert_type = 1;
    } else if (!strncmp(cmd, "os", 4)) {
        misc_req->assert_type = 2;
    } else if (!strncmp(cmd, "except", 6)) {
        misc_req->assert_type = 3;
    } else {
        result = 1;
    }

    return result;
}

    
static int32_t cli_misc_group_parse(char *cmd, char *cmd2, char *stx,
                                    char *cmd_org, cli_req_t *req)
{
    int32_t            error = 1;
    const char         *txt = "show";
    vtss_debug_group_t group;
    misc_cli_req_t     *misc_req = req->module_req;
    
    /* Accept 'show' keyword to display groups */
    if (strstr(txt, cmd) == txt) {
        misc_req->group = VTSS_DEBUG_GROUP_COUNT;
        return 0;
    }
    
    for (group = VTSS_DEBUG_GROUP_ALL; group < VTSS_DEBUG_GROUP_COUNT; group++) {
        txt = misc_cli_group_table[group];
        if (txt != NULL && strstr(txt, cmd) == txt) {
            /* Found matching group */
            if (error) {
                error = 0;
                misc_req->group = group;
            } else {
                error = 1;
                break;
            }
        }
    }
    return error;
}

static int32_t cli_misc_port_list_parse(char *cmd, char *cmd2, char *stx,
                                        char *cmd_org, cli_req_t *req)
{
    int32_t error;
    
    if ((error = cli_parse_none(cmd)) == 0)
        (void)cli_parm_parse_list(NULL, req->uport_list, 1, VTSS_PORTS, 0);
    else
        error = cli_parm_parse_list(cmd, req->uport_list, 1, VTSS_PORTS, 1);
    return error;
}



static int32_t cli_misc_prio_list_parse(char *cmd, char *cmd2, char *stx,
                                        char *cmd_org, cli_req_t *req)
{
    misc_cli_req_t   *misc_req = req->module_req;
    int32_t error;

    if ((error = cli_parse_none(cmd)) == 0)
        (void)cli_parm_parse_list(NULL, misc_req->prio_list, 0, VTSS_PRIOS-1, 0);
    else
        error = cli_parm_parse_list(cmd, misc_req->prio_list, 0, VTSS_PRIOS-1, 1);
    return error;
}

static int32_t cli_misc_module_parse(char *cmd, char *cmd2, char *stx,
                                     char *cmd_org, cli_req_t *req)
{
    int32_t          error = 1;
    vtss_module_id_t mid;
    char             name[80];
    int              i, len;
    misc_cli_req_t   *misc_req = req->module_req;

    for (mid = 0; mid < VTSS_MODULE_ID_NONE; mid++) {
        if (vtss_module_names[mid] == NULL)
            continue;
        
        /* Copy name and convert to lower case */
        strncpy(name, vtss_module_names[mid], sizeof(name));
        name[sizeof(name) - 1] = '\0';
        len = strlen(name);
        for (i = 0; i < len; i++)
            name[i] = tolower(name[i]);
        
        if (strncmp(name, cmd, strlen(cmd)) == 0) {
            misc_req->module_list[mid] = 1;
            misc_req->module_all = 0;
            error = 0;
        }
    }
    
    return error;
}

#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
static int32_t cli_misc_1588_blk_id_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    misc_cli_req_t *misc_req = req->module_req;
    return cli_parse_ulong(cmd, &misc_req->blk_id_1588, 0, 7);
}

static int32_t cli_misc_1588_csr_reg_offset_parse(char *cmd, char *cmd2, char *stx,
                                             char *cmd_org, cli_req_t *req)
{
    misc_cli_req_t *misc_req = req->module_req;
    return cli_parse_ulong(cmd, &misc_req->csr_reg_offset_1588, 0, 0x7ff);
}
#endif /* VTSS_FEATURE_PHY_TIMESTAMP */

#if defined(VTSS_SW_OPTION_BOARD)
static int32_t cli_misc_int_source_value_parse(char *cmd, char *cmd2, char *stx,
                                               char *cmd_org, cli_req_t *req)
{
    misc_cli_req_t *misc_req = req->module_req;
    return cli_parse_ulong(cmd, &misc_req->int_source, 0, 0xFFFF);
}
#endif /* VTSS_SW_OPTION_BOARD */

/****************************************************************************/
/*  Parameter table                                                         */
/****************************************************************************/

static cli_parm_t misc_cli_parm_table[] = {
#ifdef VTSS_SW_OPTION_I2C
    {
        "<i2c_data>",
        "I2C data",
        CLI_PARM_FLAG_SET,
        cli_misc_i2c_data_parse,
        NULL
    },
    {
        "<i2c_addr>",
        "I2C addr",
        CLI_PARM_FLAG_SET,
        cli_misc_i2c_addr_parse,
        NULL
    },
    {
        "<sfp_reg_addr>",
        "SFP register address",
        CLI_PARM_FLAG_SET,
        cli_misc_sfp_reg_addr_parse,
        NULL
    },
    {
        "<i2c_bytes>",
        "Number of bytes to read",
        CLI_PARM_FLAG_SET,
        cli_misc_i2c_bytes_parse,
        NULL
    },
    {
        "<i2c_clk_sel>",
        "i2c clk selector (gpio number)",
        CLI_PARM_FLAG_SET,
        cli_misc_i2c_clk_sel_parse,
        NULL
    },

#endif /* VTSS_SW_OPTION_I2C */
    {
        "<serial_data>",
        "Serial GPIO data (32/16 bits depending on hardware)",
        CLI_PARM_FLAG_SET,
        cli_misc_serial_data_parse,
        NULL
    },
    {
        "<appl|os|except>",
        "Assertion/Exception type\n"
        "  appl:   Generate application assertion\n"
        "  os:     Generate eCos assertion\n"
        "  except: Generate exception",
        CLI_PARM_FLAG_NO_TXT,
        cli_misc_assert_parse,
        NULL
    },
    {
        "detailed",
        "Show detailed information",
        CLI_PARM_FLAG_NONE,
        cli_misc_keyword_parse,
        NULL
    },
    {
        "clear",
        "Clear selected max lock info",
        CLI_PARM_FLAG_NONE,
        cli_misc_keyword_parse,
        misc_cli_cmd_debug_critd_max_lock
    },
    {
        "clear",
        "Clear API counters",
        CLI_PARM_FLAG_NONE,
        cli_misc_keyword_parse,
        misc_cli_cmd_debug_api
    },
    {
        "<layer>",
        "API Layer, 'ail' or 'cil' (default: All layers)",
        CLI_PARM_FLAG_NONE,
        cli_misc_layer_parse,
        NULL
    },
    {
        "<group>",
        "API Function Group or 'show' to list groups (default: All groups)",
        CLI_PARM_FLAG_NONE,
        cli_misc_group_parse,
        NULL

    },
    {
        "<port_list>",
        "Port list or 'none'",
        CLI_PARM_FLAG_NONE,
        cli_misc_port_list_parse,
        NULL
    },
    {
        "full",
        "Show full information",
        CLI_PARM_FLAG_NONE,
        cli_misc_keyword_parse,
        NULL
    },
    {
        "<chip_no>",
        "Chip number (0-1) or 'all'",
        CLI_PARM_FLAG_SET,
        cli_misc_chip_no_parse,
        misc_cli_cmd_debug_chip
    },
    {
        "<tgt_list>",
#if defined(VTSS_ARCH_LUTON26)
        "Target list (0-63)",
#else
        "Target list (0-255)",
#endif /* VTSS_ARCH_JAGUAR_1 */
        CLI_PARM_FLAG_NONE,
        cli_misc_blk_list_parse,
        NULL
    },
    {
        "<mmd_reg_addr>",
        "PHY mmd reg addr (0-0xffff)",
        CLI_PARM_FLAG_SET,
        cli_misc_phy_mmd_reg_addr_parse,
        misc_cli_cmd_debug_phy_mmd_read
    },
    {
        "<mmd_reg_addr>",
        "PHY mmd reg addr (0-0xffff)",
        CLI_PARM_FLAG_SET,
        cli_misc_phy_mmd_reg_addr_parse,
        misc_cli_cmd_debug_phy_mmd_write
    },
#if defined(VTSS_FEATURE_10G) 
    {
        "<mmd_reg_addr>",
        "PHY mmd reg addr (0-0xffff)",
        CLI_PARM_FLAG_SET,
        cli_misc_phy_mmd_reg_addr_parse,
        misc_cli_cmd_debug_phy_mmd_read_direct
    },
    {
        "<miim_addr>",
        "PHY miim addr (0-31)",
        CLI_PARM_FLAG_SET,
        cli_misc_phy_miim_parse,
        misc_cli_cmd_debug_phy_mmd_read_direct
    },
    {
        "<miim_ctrl>",
        "MIIM controller (0-1)",
        CLI_PARM_FLAG_SET,
        cli_misc_phy_miim_ctrl_parse,
        misc_cli_cmd_debug_phy_mmd_read_direct
    },
    {
        "<mmd_reg_addr>",
        "PHY mmd reg addr (0-0xffff)",
        CLI_PARM_FLAG_SET,
        cli_misc_phy_mmd_reg_addr_parse,
        misc_cli_cmd_debug_phy_mmd_write_direct
    },
    {
        "<miim_addr>",
        "PHY miim addr (0-31)",
        CLI_PARM_FLAG_SET,
        cli_misc_phy_miim_parse,
        misc_cli_cmd_debug_phy_mmd_write_direct
    },
    {
        "<miim_ctrl>",
        "MIIM controller (0-1)",
        CLI_PARM_FLAG_SET,
        cli_misc_phy_miim_ctrl_parse,
        misc_cli_cmd_debug_phy_mmd_write_direct
    },

    {
        "<value>",
        "PHY register value (0-0xffff)",
        CLI_PARM_FLAG_SET,
        cli_misc_phy_value_parse,
        misc_cli_cmd_debug_phy_mmd_write_direct
    },
#endif

    {
        "<i2c_reg_addr>",
        "I2C register address (0-255)",
        CLI_PARM_FLAG_NONE,
        cli_misc_phy_i2c_reg_addr_parse,
        misc_cli_cmd_debug_phy_i2c_rd
    },
    {
        "<i2c_reg_addr>",
        "I2C register address (0-255)",
        CLI_PARM_FLAG_NONE,
        cli_misc_phy_i2c_reg_addr_parse,
        misc_cli_cmd_debug_phy_i2c_wr
    },

    {
        "<i2c_data>",
        "I2C register address (0-255)",
        CLI_PARM_FLAG_NONE,
        cli_misc_phy_i2c_reg_addr_parse,
        misc_cli_cmd_debug_phy_i2c_wr
    },

    {
        "<i2c_mux>",
        "I2C MUX  (0-3)",
        CLI_PARM_FLAG_NONE,
        cli_misc_phy_i2c_mux_parse,
        misc_cli_cmd_debug_phy_i2c_rd
    },

    {
        "<i2c_mux>",
        "I2C MUX  (0-3)",
        CLI_PARM_FLAG_NONE,
        cli_misc_phy_i2c_mux_parse,
        misc_cli_cmd_debug_phy_i2c_wr
    },
    {
        "<i2c_device_addr>",
        "I2C device address (0-127)",
        CLI_PARM_FLAG_NONE,
        cli_misc_phy_i2c_device_addr_parse,
        misc_cli_cmd_debug_phy_i2c_rd
    },
    {
        "<i2c_device_addr>",
        "I2C device address (0-127)",
        CLI_PARM_FLAG_NONE,
        cli_misc_phy_i2c_device_addr_parse,
        misc_cli_cmd_debug_phy_i2c_wr
    },
    {
        "<addr_list>",
        "Register address list (0-0xffff)",
        CLI_PARM_FLAG_NONE,
        cli_misc_phy_mmd_addr_list_parse,
        misc_cli_cmd_debug_phy_mmd_write
    },
    {
        "<addr_list>",
        "Register address list (0-31)",
        CLI_PARM_FLAG_NONE,
        cli_misc_phy_addr_list_parse,
        misc_cli_cmd_debug_phy_write
    },
    {
        "<addr_list>",
        "Register address list (0-31)",
        CLI_PARM_FLAG_NONE,
        cli_misc_phy_addr_list_parse,
        misc_cli_cmd_debug_phy_read
    },

    {
        "<port>",
        "PHY port number\n"
        "If config set - this port becomes source of recovered clock\n"
        "If config get - this port is the first port on the PHY",
        CLI_PARM_FLAG_NONE,
        cli_misc_clock_phy_port_parse,
        misc_cli_cmd_debug_phy_clock_conf
    },
    {
        "<clock>",
        "Recovered Clock output number - 1 or 2",
        CLI_PARM_FLAG_NONE,
        cli_misc_clock_port_parse,
        misc_cli_cmd_debug_phy_clock_conf
    },
    {
        "serdes|copper|tclk|xtal|disable",
        "Clock source type",
        CLI_PARM_FLAG_SET,
        cli_misc_clock_src_parse,
        misc_cli_cmd_debug_phy_clock_conf
    },
    {
        "25m|125m|3125m",
        "Recovered clock frequency",
        CLI_PARM_FLAG_SET,
        cli_misc_clock_freq_parse,
        misc_cli_cmd_debug_phy_clock_conf
    },
    {
        "<squelch>",
        "Clock squelch level - 0 to 3",
        CLI_PARM_FLAG_SET,
        cli_misc_clock_squelch_parse,
        misc_cli_cmd_debug_phy_clock_conf
    },
    {
        "<master>",
        "Auto-negotiation Master/Slave Configuration 0-Slave 1-Master",
        CLI_PARM_FLAG_SET,
        cli_misc_master_parse,
        misc_cli_cmd_debug_1g_master_slave_conf 
    },
    {
        "<port>",
        "PHY port number",
        CLI_PARM_FLAG_NONE,
        cli_misc_clock_autoneg_port_parse,
        misc_cli_cmd_debug_1g_master_slave_conf 
    },
    {
        "<addr_list>",
#if defined(VTSS_ARCH_LUTON26)
        "Register address list (0-0x3fff)",
#else
        "Register address list (0-0x3ffff)",
#endif /* VTSS_ARCH_LUTON26 */
        CLI_PARM_FLAG_NONE,
        cli_misc_addr_list_parse,
        NULL
    },
    {
        "<value>",
        "Register value (0-0xffffffff)",
        CLI_PARM_FLAG_SET,
        cli_misc_value_parse,
        misc_cli_cmd_debug_reg_write
    },
    {
        "<value>",
        "PHY register value (0-0xffff)",
        CLI_PARM_FLAG_SET,
        cli_misc_phy_value_parse,
        misc_cli_cmd_debug_phy_write
    },
    {
        "far|near",
        "Loopback type \n"
        "far :  Far end loopback (CU ports)\n"
        "near:  Near end loopback (MAC side)\n"
        "(default: Show mode)",
        CLI_PARM_FLAG_SET,
        cli_misc_phy_loopback_parse,
        NULL
    },
    {
        "<value>",
        "PHY register value (0-0xffff)",
        CLI_PARM_FLAG_SET,
        cli_misc_phy_value_parse,
        misc_cli_cmd_debug_phy_mmd_write
    },
    {
        "<value>",
        "Value to call the ob post0 patch (0-0xffff)",
        CLI_PARM_FLAG_SET,
        cli_misc_ob_post0_value_parse,
        misc_cli_cmd_debug_ob_post
    },
    {
        "<ib_cterm_value>",
        "Value to call the ib cterm patch (0-0xff)",
        CLI_PARM_FLAG_SET,
        cli_misc_ib_cterm_value_parse,
        misc_cli_cmd_debug_ib_cterm
    },
    {
        "<ib_eq_mode>",
        "Value to call the ib cterm patch (0-0xff)",
        CLI_PARM_FLAG_SET,
        cli_misc_ib_eq_mode_value_parse,
        misc_cli_cmd_debug_ib_cterm
    },
    {
        "<devad>",
        "PHY devad  (0-31)",
        CLI_PARM_FLAG_SET,
        cli_misc_phy_devad_parse,
        misc_cli_cmd_debug_phy_mmd_read
    },

    {
        "<devad>",
        "PHY devad  (0-31)",
        CLI_PARM_FLAG_SET,
        cli_misc_phy_devad_parse,
        misc_cli_cmd_debug_phy_mmd_write
    },
#if defined(VTSS_FEATURE_10G)
    {
        "<devad>",
        "PHY mmd  (0-31)",
        CLI_PARM_FLAG_SET,
        cli_misc_phy_devad_parse,
        misc_cli_cmd_debug_phy_mmd_read_direct
    },
    {
        "<devad>",
        "PHY mmd  (0-31)",
        CLI_PARM_FLAG_SET,
        cli_misc_phy_devad_parse,
        misc_cli_cmd_debug_phy_mmd_write_direct
    },
    {
        "<devad>",
        "PHY mmd  (1,2,3,4,7,30,31)",
        CLI_PARM_FLAG_SET,
        cli_misc_phy_devad_parse,
        misc_cli_cmd_debug_csr_write
    },
    {
        "<devad>",
        "PHY mmd  (1,2,3,4,7,30,31)",
        CLI_PARM_FLAG_SET,
        cli_misc_phy_devad_parse,
        misc_cli_cmd_debug_csr_read
    },
    {
        "<addr>",
        "CSR address (base+offset) \n",
        CLI_PARM_FLAG_SET,
        cli_misc_phy_addr_parse,
        misc_cli_cmd_debug_csr_read
    },
    {
        "<addr>",
        "CSR address (base+offset) \n",
        CLI_PARM_FLAG_SET,
        cli_misc_phy_addr_parse,
        misc_cli_cmd_debug_csr_write
    },
#endif
    {
        "addr_sort",
        "Sort output according to addresses",
        CLI_PARM_FLAG_NONE,
        cli_misc_keyword_parse,
        NULL
    },
    {
        "<page>",
        "Register page (0-0xffff), default: page 0",
        CLI_PARM_FLAG_NONE,
        cli_misc_page_parse,
        NULL
    },
    {
        "<module>",
        "Module name.",
        CLI_PARM_FLAG_NONE,
        cli_misc_module_parse,
        NULL
    },
    {
        "enable|disable",
        "enable     : Enable \n"
        "disable    : Disable \n"
        "(default: Show )\n",
        CLI_PARM_FLAG_SET,
        cli_misc_keyword_parse,
    },
    {
        "default|phy_inst",
        "default     : default switch API instance \n"
        "phy_inst    : Phy API instance \n"
        "(default: Show )\n",
        CLI_PARM_FLAG_SET,
        cli_misc_keyword_parse,
    },

    {
        "input|output|pcs_rx_fault",
        "input       : input mode        \n"
        "output      : output mode       \n"
        "pcs_rx_fault: pcs_rx_fault mode \n"
        "(default: Show )\n",
        CLI_PARM_FLAG_SET,
        cli_misc_keyword_parse,
    },
    {
        "lan|wan|1g",
        "lan     : LAN mode\n"
        "wan     : WAN mode\n"
        "1g      : 1g mode\n"
        "(default: Show mode)",
        CLI_PARM_FLAG_SET,
        cli_misc_keyword_parse,

    },
    {
        "a|b|c|d|e|f|g|h|j|k",
        "10G Phy Loopbacks system/network\n"
        "                                    Venice equivalent\n"
        "b: XAUI -> XS -> XAUI - shallow     n.a.\n"
        "c: XAUI -> XS -> XAUI - deep        n.a.\n"
        "e: XAUI -> PCS FIFO -> XAUI         n.a.\n"
        "g: XAUI -> PCS -> XAUI              H3\n"
        "j: XAUI -> PMA -> XAUI              H4\n"
        "d: XFI  -> XS  -> XFI - shallow     n.a.\n"
        "a: XFI  -> XS  -> XFI - deep        n.a.\n"
        "f: XFI  -> PCS -> XFI               L2\n"
        "h: XFI  -> WIS -> XFI               n.a.\n"
        "k: XFI  -> PMA -> XFI               n.a.",
        CLI_PARM_FLAG_SET,
        cli_misc_lb_parse,
        NULL
    },
    {
        "a|b|c|d|e|f",
        "10G Phy failover modes\n"
        "a: PMA <--> XAUI normal\n"
        "b: PMA <--> XAUI crossed\n"
        "c: PMA 0 <--> XAUI 0; PMA 0 -->XAUI 1\n"
        "d: PMA 0 <--> XAUI 1; PMA 0 -->XAUI 0\n"
        "e: PMA 1 <--> XAUI 0; PMA 1 -->XAUI 1\n"
        "f: PMA 1 <--> XAUI 1; PMA 1 -->XAUI 0\n",
        CLI_PARM_FLAG_SET,
        cli_misc_failover_parse,
        NULL
    },
    {
        "<gpio_list>",
        "GPIO list, default: All GPIOs",
        CLI_PARM_FLAG_NONE,
        cli_misc_gpio_list_parse,
        NULL
    },
    {
        "<sid>",
        "Switch ID, default: Show SID",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_sid,
        NULL
    },
#if defined(VTSS_CHIP_10G_PHY)
    {
        "<value>",
        "PHY register value (0-0xffff)",
        CLI_PARM_FLAG_SET,
        cli_misc_gpio_value_parse,
        misc_cli_cmd_phy_10g_gpio_write
    },

    {
        "<value>",
        "Register value (0-0xffffffff)",
        CLI_PARM_FLAG_SET,
        cli_misc_value_parse,
        misc_cli_cmd_debug_csr_write
    },
#endif /* defined(VTSS_CHIP_10G_PHY) */
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
    {
        "<value>",
        "PHY 1588 CSR register value (0-0xffffffff)",
        CLI_PARM_FLAG_SET,
        cli_misc_phy_value_parse,
        misc_cli_cmd_debug_phy_1588_mmd_write
    },
    {
        "<blk_id_1588>",
        "1588 internal block id <0 - 7>\n"
        " 0 -  Ingress Analyzer 0 \n"
        " 1 -  Egress  Analyzer 0 \n"
        " 2 -  Ingress Analyzer 1 \n"
        " 3 -  Egress  Analyzer 1 \n"
        " 4 -  Ingress Analyzer 2 \n"
        " 5 -  Egress  Analyzer 2 \n"
        " 6 -  Processor 0 \n"
        " 7 -  Processor 1 \n",
        CLI_PARM_FLAG_NONE,
        cli_misc_1588_blk_id_parse,
        NULL
    },
    {
        "<csr_reg_list>",
        "1588 CSR internal block register offset list.",
        CLI_PARM_FLAG_NONE,
        cli_misc_addr_list_parse,
        misc_cli_cmd_debug_phy_1588_mmd_read
    },
    {
        "<csr_reg_offset>",
        "1588 CSR internal block register offset.",
        CLI_PARM_FLAG_NONE,
        cli_misc_1588_csr_reg_offset_parse,
        NULL
    },
#endif /* VTSS_FEATURE_PHY_TIMESTAMP */

#if defined(VTSS_SW_OPTION_BOARD)
    {
        "<int_source>",
        "Interrupt source selection - vtss_interrupt_source_t value from interrupt_api.h",
        CLI_PARM_FLAG_SET,
        cli_misc_int_source_value_parse,
        misc_cli_cmd_debug_interrupt_hook
    },
#endif /* VTSS_SW_OPTION_BOARD */
#ifdef  VTSS_FEATURE_WIS
    {
        "<overhead>",
        "WIS overhead <0 - 1>\n"
        " 0 -  Section Layer \n"
        " 1 -  Path Layer \n",
        CLI_PARM_FLAG_SET,
        cli_misc_wis_overhead_parse,
        NULL
    },
    {
        "<tti_mode>",
        "WIS tti mode <0 - 2>\n"
        " 0 -  One byte \n"
        " 1 -  16 Bytes \n"
        " 2 -  64 Bytes \n",
        CLI_PARM_FLAG_SET,
        cli_misc_wis_tti_mode_parse,
        NULL
    },
    {
        "<tti>",
        "WIS TTI <64 bytes>\n",
        CLI_PARM_FLAG_SET,
        cli_misc_wis_tti_parse,
        NULL
    },
    {
        "gen_dis|gen_sqr|gen_prbs31|gen_mix",
        "Generator Mode\n",
        CLI_PARM_FLAG_NONE,
        cli_misc_wis_gen_mode_parse,
        NULL
    },
    {
        "ana_dis|ana_prbs31|ana_mix",
        "Analyser Mode\n",
        CLI_PARM_FLAG_SET,
        cli_misc_wis_ana_mode_parse,
        NULL
    },
    {
        "loopback|no_loopback",
        "\nloopback     : Enable loopback\n"
        "no_loopback     : Disable loopback\n"
        "(default: Disable loopback)",
        CLI_PARM_FLAG_SET,
        cli_misc_keyword_parse,
    },
    {
        "<wis_aisl>",
        "WIS AIS-L consequent actions mask <0 - 1>\n"
        " 0 bit -  Enable for AIS-L insertion on LOS  \n"
        " 1 bit -  Enable for AIS-L insertion on LOF \n",
        CLI_PARM_FLAG_SET,
        cli_misc_wis_aisl_parse,
        NULL
    },
    {
        "<wis_rdil>",
        "WIS RDI-L consequent actions Mask <0 to 3>\n"
        " 0 bit -  Enable for RDI-L back reporting on LOS \n"
        " 1 bit -  Enable for RDI-L back reporting on LOF \n"
        " 2 bit -  Enable for RDI-L back reporting on LOPC \n"
        " 3 bit -  Enable for RDI-L back reporting on AIS_L \n",
        CLI_PARM_FLAG_SET,
        cli_misc_wis_rdil_parse,
        NULL
    },
    {
        "<wis_fault>",
        "WIS Fault Condition mask <0 to 10 bits>\n"
        "  0 bit -  Enable fault condition on far-end PLM-P \n"
        "  1 bit -  Enable fault condition on far-end AIS-P \n"
        "  2 bit -  Enable fault condition on RDI-L \n"
        "  3 bit -  Enable fault condition on SEF \n"
        "  4 bit -  Enable fault condition on LOF \n"
        "  5 bit -  Enable fault condition on LOS \n"
        "  6 bit -  Enable fault condition on AIS-L \n"
        "  7 bit -  Enable fault condition on LCD-P \n"
        "  8 bit -  Enable fault condition on PLM-P \n"
        "  9 bit -  Enable fault condition on AIS-P \n"
        " 10 bit -  Enable fault condition on LOP-P \n",
        CLI_PARM_FLAG_SET,
        cli_misc_wis_fault_parse,
        NULL
    },
    {
        "<line_rx>",
        "WIS Line rx force mask <0 to 1 bits>\n"
        "  0 bit -  Force AIS-L configuration \n"
        "  1 bit -  Force RDI-L configuration \n",
        CLI_PARM_FLAG_SET,
        cli_misc_wis_line_rx_parse,
        NULL
    },
    {
        "<line_tx>",
        "WIS Line tx force mask <0 to 1 bits>\n"
        "  0 bit -  Force transmission of AIS-L in the K2 byte \n"
        "  1 bit -  Force transmission of RDI-L in the K2 byte \n",
        CLI_PARM_FLAG_SET,
        cli_misc_wis_line_tx_parse,
        NULL
    },
    {
        "<path_force>",
        "WIS path_force mask <0 to 1 bits>\n"
        "  0 bit -  Force UNEQ-P configuration \n"
        "  1 bit -  Force RDI-P configuration \n",
        CLI_PARM_FLAG_SET,
        cli_misc_wis_path_force_parse,
        NULL
    },
    {
        "<oh_name>",
        "WIS overhead name keywords, Names are case sensitive.\n"
        "\tALL       :  Key word to configure all overheads in one step.\n" 
        "\tSEC-OH    :  Section overhead \n" 
        "\tLINE-OH   :  Line overhead \n" 
        "\tPATH-OH   :  Path overhead \n" 
        "\tD1-D3     :  Section Data Communications Channel\n" 
        "\tSEC-ORD   :  Section Orderwire\n"
        "\tSUC       :  Section User Channel \n" 
        "\tD4-D12    :  Line Data Communications Channel \n" 
        "\tLINE-ORD  :  Line Orderwire overhead \n"
        "\tAPS-RDIL  :  Automatic protection switch (APS) channel and Line Remote Defect Identifier (RDI-L)\n"
        "\tSYNC      :  Synchronization messaging \n"
        "\tRES-LG    :  Reserved for Line growth  \n"
        "\tC2PL      :  Transmitted C2 path label \n"
        "\tPUC       :  Path User Channel \n"
        "\tPTCM      :  Tandem connection maintenance/Path data channel \n"
        "\tRES-PG    :  Reserved for Path growth \n",
        CLI_PARM_FLAG_SET,
        cli_misc_wis_overhead_name_parse,
        NULL
    },
    {
        "<oh_value>",
        "WIS overheadvalue, Value in hexadecimal format, '0x' prefix is optional\n"
        "\t 1. ALL       :  One Hexadecimal value for all overhead(25 bytes), order is from D1-D3 to RES-PG\n" 
        "\t 2. SEC-OH    :  One Hexadecimal value for Section overhead\n" 
        "\t 3. LINE-OH   :  One Hexadecimal value for Line overhead keyword\n" 
        "\t 4. PATH-OH   :  One Hexadecimal value for Path overhead keyword\n" 
        "\t 5. D1-D3     :  Three bytes hexadecimal value, Ex:0xAABBCC\n" 
        "\t 6. SEC-ORD   :  One byte hexadecimal value, Ex:0xAA\n"
        "\t 6. SUC       :  One byte hexadecimal value, Ex:0xAA\n" 
        "\t 8. D4-D12    :  Nine bytes hexadecimal value, Ex:0xAABBCCDDEEFF112233\n" 
        "\t 9. LINE-ORD  :  One byte hexadecimal value, Ex:0xAA\n"
        "\t10. APS-RDIL  :  Two bytes hexadecimal value, Ex:0xAABB \n"
        "\t11. SYNC      :  One byte hexadecimal value, Ex:0xAA\n"
        "\t12. RES-LG    :  Two bytes hexa decimal value, Ex:0xAABB \n"
        "\t13. C2PL      :  One byte hexadecimal value, Ex:0xAA\n"
        "\t14. PUC       :  One byte hexadecimal value, Ex:0xAA\n"
        "\t15. PTCM      :  One byte hexadecimal value, Ex:0xAA\n"
        "\t16. RES-PG    :  Two bytes hexadecimal value, Ex:0xAABB \n",
        CLI_PARM_FLAG_SET,
        cli_misc_wis_overhead_value_parse,
        NULL
    },
    {
        "<n_ebc_thr_s>",
        "WIS Section error count (B1) threshold\n",
        CLI_PARM_FLAG_SET,
        cli_misc_wis_n_ebc_thr_s_parse,
        NULL
    },
    {
        "<n_ebc_thr_l>",
        "WIS Near end line error count (B2) threshold\n",
        CLI_PARM_FLAG_SET,
        cli_misc_wis_n_ebc_thr_l_parse,
        NULL
    },
    {
        "<f_ebc_thr_l>",
        "WIS Far end line error count threshold\n",
        CLI_PARM_FLAG_SET,
        cli_misc_wis_f_ebc_thr_l_parse,
        NULL
    },
    {
        "<n_ebc_thr_p>",
        "WIS Path block error count (B3) threshold\n",
        CLI_PARM_FLAG_SET,
        cli_misc_wis_n_ebc_thr_p_parse,
        NULL
    },
    {
        "<f_ebc_thr_p>",
        "WIS Far end path error count threshold\n",
        CLI_PARM_FLAG_SET,
        cli_misc_wis_f_ebc_thr_p_parse,
        NULL
    },
    {
        "single_err|sat_err",
        "WIS Far end path error count threshold\n",
        CLI_PARM_FLAG_SET,
        cli_misc_wis_prbs31_err_inj_parse,
        NULL
    },
    {
        "<force_events>",
        "WIS MASK for Single event or multiple events\n",
        CLI_PARM_FLAG_SET,
        cli_misc_wis_force_events_parse,
        NULL
    },

#endif //VTSS_FEATURE_WIS 
#if defined (VTSS_FEATURE_SYNCE_10G)
        {
            "<mode>",
            "Synchronous Ethernet modes\n"
            "3 : LAN mode\n"
            "4 : WAN mode\n"
            "5 : Mixed mode, channel being configured in LAN mode and other is in WAN mode\n"
            "6 : Mixed mode, channel being configured in WAN mode and other is in LAN mode\n"
            "(default: Show mode)\n",
            CLI_PARM_FLAG_SET,
            cli_misc_synce_mode_parse,
        },
        {
            "<clk_out>",
            "Recovered Clock used from PHY\n"
            "0 : RXCLKOUT\n"
            "1 : TXCLKOUT\n",
            CLI_PARM_FLAG_SET,
            cli_misc_synce_clk_out_parse,
        },
        {
            "<hitless>",
            "Clock synthesizer is hitless or not\n"
            "0    : External Clock synthesizer is not hitless, SREFCLK and XREFCLK used as input Clock \n"
            "1    : External Clock synthesizer is hitless, XREFCLK is used as input Clock \n",
            CLI_PARM_FLAG_SET,
            cli_misc_synce_hitless_parse,
        },
        {
            "<rclk_div>",
            "Recovered Clock's divisor \n"
            "0  : Recovered clock is divided by 64 \n"
            "1  : Recovered clock is divided by 66 \n",
            CLI_PARM_FLAG_SET,
            cli_misc_synce_rclk_div_parse,
        },
        {
            "<sref_div>",
            "SREFCLK's divisor \n"
            "0  : SREFCLK is divided by 64 \n"
            "1  : SREFCLK is divided by 66 \n"
            "2  : SREFCLK is divided by 16 \n",
            CLI_PARM_FLAG_SET,
            cli_misc_synce_sref_div_parse,
        },
        {
            "<wref_div>",
            "WREFCLK's divisor \n"
            "0  : WREFCLK/16 is not used \n"
            "1  : WREFCLK/16 is used \n",
            CLI_PARM_FLAG_SET,
            cli_misc_synce_wref_div_parse,
        },
#endif /* VTSS_FEATURE_SYNCE_10G */
#ifdef VTSS_FEATURE_MACSEC
    {
        "<block>",
        "MacSec block \n",
        CLI_PARM_FLAG_SET,
        cli_misc_block_parse,
    },
    {
        "<addr>",
        "MacSec addr \n",
        CLI_PARM_FLAG_SET,
        cli_misc_addr_parse,
    },
    {
        "<value>",
        "Register value (0-0xffffffff)",
        CLI_PARM_FLAG_SET,
        cli_misc_value_parse,
        misc_cli_cmd_debug_macsec_write
    },
#endif /* VTSS_FEATURE_MACSEC */
    {
        "<gpio_value>",
        "Value of the GPIO register \n",
        CLI_PARM_FLAG_SET,
        cli_misc_gpio_val_parse,
    },
    {
        "<gpio_mask>",
        "Mask of the GPIO register \n",
        CLI_PARM_FLAG_SET,
        cli_misc_gpio_mask_parse,
    },
    {
        "<value_list>",
        "List of values to write to the SPI Device Format::val1,val2...\n",
        CLI_PARM_FLAG_SET,
        cli_misc_value_list_parse,
    },
    {
        "<spi_cs>",
        "SPI_CS Identifier \n"
        "   Valid CS Range 1/3(SPI_CS1/SPI_CS3)\n",
        CLI_PARM_FLAG_SET,
        cli_misc_spi_cs_parse,
    },
    {
        "<no_of_bytes>",
        "No of bytes to read/write from the SPI (0-50)\n",
        CLI_PARM_FLAG_SET,
        cli_misc_no_of_bytes_parse,
    },
    {
        "<qs_mode>",
        "QS mode: strict|shared \n",
        CLI_PARM_FLAG_SET,
        cli_misc_mode_parse,
    },
    {
        "<port_bytes>",
        "Number of bytes for Port.\n"
        "Strict: max number of bytes.\n"
        "Shared: guaranteed number of bytes\n",
        CLI_PARM_FLAG_SET,
        cli_misc_port_value_parse,
    },
    {
        "<queue_list>",
        "Queues to apply to\n",
        CLI_PARM_FLAG_SET,
        cli_misc_prio_list_parse,
    },

    {
        "<queue_bytes>",
        "Number of bytes for Queue.\n"
        "Strict: max number of bytes.\n"
        "Shared: guaranteed number of bytes\n",
        CLI_PARM_FLAG_SET,
        cli_misc_queue_value_parse,
    },
    {
        "<oversub_in_pct>",
        "Oversubscription of the queue system 0-50%\n",
        CLI_PARM_FLAG_SET,
        cli_misc_queue_oversub_parse,
    },
    {
        NULL,
        NULL,
        0,
        0,
        NULL
    },
};

/****************************************************************************/
/*  Command table                                                           */
/****************************************************************************/

enum {
  PRIO_I2C_RD_SCAN = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_I2C_RD = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_SFP_RD = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_SFP_WR = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_I2C_WR = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_SERIALIZED_RD = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_SERIALIZED_WR = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_API = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_CHIP = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_REG_READ = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_REG_WRITE = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_PHY_READ = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_PHY_I2C_READ = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_PHY_I2C_WRITE = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_PHY_MMD_READ = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_PHY_MMD_WRITE = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_PHY_MMD_READ_DIRECT = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_PHY_MMD_WRITE_DIRECT = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_PHY_WRITE = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_PHY_OB_POST0 = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_PHY_IB_CTERM = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_PHY_PATCH_SETTINGS_GET = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_PHY_LOOPBACK = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_PHY_STATISTIC = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_PHY_CLOCK_CONF = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_PHY_AUTONEGMS_CONF = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_SUSPEND = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_RESUME = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_CRITD_LIST = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_CRITD_MAX_LOCK = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_LED_USID = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_PHY_10G_STATUS = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_PHY_10G_CSR_READ = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_PHY_10G_CSR_WRITE = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_PHY_10G_LOOPBACK = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_PHY_10G_FAILOVER = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_PHY_10G_POWER = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_WM_READ = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_PHY_10G_MODE = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_PHY_10G_RESET = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_PHY_10G_GPIO_OUTPUT = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_PHY_10G_GPIO_READ = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_PHY_10G_GPIO_WRITE = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_PHY_10G_FW_LOAD = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_PHY_INSTANCE = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_PHY_MACSEC = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_QS = CLI_CMD_SORT_KEY_DEFAULT,
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
  PRIO_DEBUG_PHY_1588_MMD_READ  = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_PHY_1588_MMD_WRITE = CLI_CMD_SORT_KEY_DEFAULT,
#endif /* VTSS_FEATURE_PHY_TIMESTAMP */
#if defined(VTSS_SW_OPTION_BOARD)
  PRIO_DEBUG_INTERRUPT_HOOK = CLI_CMD_SORT_KEY_DEFAULT,
#endif /* VTSS_SW_OPTION_BOARD */
#if defined(VTSS_FEATURE_IPV4_MC_SIP)
  PRIO_DEBUG_IPMC_ADD = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_IPMC_DEL = CLI_CMD_SORT_KEY_DEFAULT,
#endif /* VTSS_FEATURE_IPV4_MC_SIP */
  PRIO_DEBUG_PHY_DO_PAGE_CHK = CLI_CMD_SORT_KEY_DEFAULT,
  PRIO_DEBUG_SPI_READ = CLI_CMD_SORT_KEY_DEFAULT,
};

#ifdef VTSS_SW_OPTION_I2C
cli_cmd_tab_entry (
  "Debug I2C_Scan_rd",
  NULL,
  "Do a I2C read from address 0 to address 255, and checks if there was a response from a slave",
  PRIO_I2C_RD_SCAN,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_cmd_i2c_rd_scan,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "Debug I2C_Read <i2c_addr> <i2c_bytes>",
  NULL,
  "Debug I2C_Read",
  PRIO_I2C_RD,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_cmd_i2c_debug_read,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    NULL,
    "Debug I2C_Write <i2c_addr> <i2c_data> [<i2c_data>] [<i2c_data>] [<i2c_data>] [<i2c_data>] [<i2c_data>] [<i2c_data>] ",
    "Debug I2C_Write",
    PRIO_I2C_WR,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    misc_cli_cmd_i2c_debug_write,
    misc_cli_req_default_set,
    misc_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    NULL,
    "Debug I2C_Mux_Write <i2c_addr>  <i2c_clk_sel> <i2c_data> [<i2c_data>] [<i2c_data>] [<i2c_data>] [<i2c_data>] [<i2c_data>] [<i2c_data>] ",
    "Debug I2C_Write using clock selector",
    PRIO_I2C_WR,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    misc_cli_cmd_i2c_debug_mux_write,
    misc_cli_req_default_set,
    misc_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "Debug SFP_rd <i2c_addr> <i2c_bytes> <i2c_clk_sel> <sfp_reg_addr> [<sfp_reg_addr>] ",
  NULL,
  "Debug SFP Read",
  PRIO_SFP_RD,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_cmd_sfp_debug_read,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "Debug SFP_wr <i2c_addr> <i2c_clk_sel> <sfp_reg_addr> <i2c_data> <i2c_data>",
  NULL,
  "Debug SFP Write",
  PRIO_SFP_WR,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_cmd_sfp_debug_write,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

#if defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_JAGUAR_1)
cli_cmd_tab_entry (
  "Debug SFP_rd_phy [<port_list>] [<i2c_addr>]",
  NULL,
  "Debug SFP Read phy",
  PRIO_SFP_RD,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_cmd_sfp_phy_debug_read,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);
#endif /* VTSS_ARCH_LUTON26 || VTSS_ARCH_JAGUAR_1 */
#endif /* VTSS_SW_OPTION_I2C */

cli_cmd_tab_entry (
  "Debug Ser_rd",
  NULL,
  "Do a read of the serialized GPIOs",
  PRIO_SERIALIZED_RD,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_cmd_debug_ser_rd,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  NULL,
  "Debug Ser_wr [<serial_data>] [<serial_data>] [<serial_data>] [<serial_data>]",
  "Do a write to the serialized GPIOs. First serial_data parameter is bit 0 the 2nd bit 1 and so on",
  PRIO_SERIALIZED_WR,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_cmd_debug_ser_wr,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);


cli_cmd_tab_entry (
  "Debug Chip [<chip_no>]",
  NULL,
  "Set or show the current chip context",
  PRIO_DEBUG_CHIP,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_cmd_debug_chip,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "Debug API [<layer>] [<group>] [<port_list>] [full] [clear]",
  NULL,
  "Show API debug information",
  PRIO_DEBUG_API,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_cmd_debug_api,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  NULL,
  "Debug Register Read <tgt_list> <addr_list>",
  "Read switch chip register",
  PRIO_DEBUG_REG_READ,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_cmd_debug_reg_read,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  NULL,
  "Debug Register Write <tgt_list> <addr_list> <value>",
  "Write switch chip register",
  PRIO_DEBUG_REG_WRITE,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_cmd_debug_reg_write,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "Debug PHY i2c_rd <port_list> <i2c_mux> <i2c_device_addr> <i2c_reg_addr> [<i2c_bytes>]",
  NULL,
  "Read PHY i2c register",
  PRIO_DEBUG_PHY_I2C_READ,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_cmd_debug_phy_i2c_rd,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);


cli_cmd_tab_entry (
  "Debug PHY i2c_wr <port_list> <i2c_mux> <i2c_device_addr> <i2c_reg_addr> <i2c_data>",
  NULL,
  "Write PHY i2c register",
  PRIO_DEBUG_PHY_I2C_WRITE,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_cmd_debug_phy_i2c_wr,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "Debug PHY Loopback [<port_list>] [far|near] [enable|disable]",
  NULL,
  "Enable / Disable PHY internal loopbacks",
  PRIO_DEBUG_PHY_LOOPBACK,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_cmd_debug_phy_loopback,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "Debug PHY Statistic [<port_list>]",
  NULL,
  "Shows PHY statistic",
  PRIO_DEBUG_PHY_STATISTIC,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_cmd_debug_phy_statistic,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "Debug PHY do_page_chk [enable|disable]",
  NULL,
  "Enable / Disable PHY page check when phy access is done (used for debugging wild page writes)",
  PRIO_DEBUG_PHY_DO_PAGE_CHK,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_cmd_debug_phy_do_page_chk,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "Debug PHY Read <port_list> <addr_list> [<page>] [addr_sort]",
  NULL,
  "Read PHY register",
  PRIO_DEBUG_PHY_READ,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_cmd_debug_phy_read,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "Debug PHY MMD_Read <port_list> <devad> <mmd_reg_addr>",
  NULL,
  "Read PHY MMD register",
  PRIO_DEBUG_PHY_MMD_READ,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_cmd_debug_phy_mmd_read,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "Debug PHY MMD_Write <port_list> <devad> <mmd_reg_addr> <value>",
  NULL,
  "Write PHY MMD register",
  PRIO_DEBUG_PHY_MMD_WRITE,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_cmd_debug_phy_mmd_write,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "Debug PHY Clock Config <port> <clock> [serdes|copper|tclk|xtal|disable] [25m|125m|3125m] [<squelch>]",
  NULL,
  "Recovered clock Configuration ",
  PRIO_DEBUG_PHY_CLOCK_CONF,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_cmd_debug_phy_clock_conf,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "Debug PHY auto-neg config <port> [enable|disable] [<master>]",
  NULL,
  "1G Auto-neg Master/Slave Configuration ",
  PRIO_DEBUG_PHY_AUTONEGMS_CONF,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_cmd_debug_1g_master_slave_conf,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);


#if defined(VTSS_FEATURE_10G)
cli_cmd_tab_entry (
  "Debug PHY MMD_read_direct <miim_ctrl> <miim_addr> <devad> <mmd_reg_addr>",
  NULL,
  "Read PHY MMD register",
  PRIO_DEBUG_PHY_MMD_READ,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_cmd_debug_phy_mmd_read_direct,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "Debug PHY MMD_write_direct <miim_ctrl> <miim_addr> <devad> <mmd_reg_addr> <value>",
  NULL,
  "Write PHY MMD register",
  PRIO_DEBUG_PHY_MMD_WRITE_DIRECT,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_cmd_debug_phy_mmd_write_direct,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);
#endif /* VTSS_FEATURE_10G */
cli_cmd_tab_entry (
  NULL,
  "Debug PHY Write <port_list> <addr_list> <value> [<page>]",
  "Write PHY register",
  PRIO_DEBUG_PHY_WRITE,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_cmd_debug_phy_write,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  NULL,
  "Debug PHY ob_post0 <port_list> <value>",
  "Checks the ob post0 patch",
  PRIO_DEBUG_PHY_OB_POST0,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_cmd_debug_ob_post,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);


cli_cmd_tab_entry (
  NULL,
  "Debug PHY patch_settings_get <port_list>",
  "Prints the patch settings",
  PRIO_DEBUG_PHY_PATCH_SETTINGS_GET,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_cmd_patch_settings_get,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);


cli_cmd_tab_entry (
  NULL,
  "Debug PHY ib_cterm <port_list> <ib_cterm_value> <ib_eq_mode>",
  "Sets the ib cterm patch",
  PRIO_DEBUG_PHY_IB_CTERM,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_cmd_debug_ib_cterm,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);


cli_cmd_tab_entry (
  NULL,
  "Debug Suspend",
  "Suspend port module",
  PRIO_DEBUG_SUSPEND,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_suspend,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  NULL,
  "Debug Resume",
  "Resume port module",
  PRIO_DEBUG_RESUME,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_resume,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  NULL,
  "Debug Assert <appl|os|except>",
  "Generate assert",
  PRIO_DEBUG_RESUME,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_assert,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "Debug Critd List [<module>] [detailed]",
  NULL,
  "List critd mutexes/semaphores and their status",
  PRIO_DEBUG_CRITD_LIST,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_cmd_debug_critd_list,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "Debug Critd MaxLock [<module>] [clear]",
  NULL,
  "List critd max. lock time locations",
  PRIO_DEBUG_CRITD_MAX_LOCK,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_cmd_debug_critd_max_lock,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "Debug LED Usid [<sid>]",
  NULL,
  "Set or show current value of USID LED",
  PRIO_DEBUG_LED_USID,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_cmd_debug_led_usid,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

#if defined(VTSS_CHIP_10G_PHY)
cli_cmd_tab_entry (
  "Debug csr write <port_list> <devad> <addr> <value>",
  NULL,
  "Write to 16/32 bit csr register.",
  PRIO_DEBUG_PHY_10G_CSR_WRITE,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_cmd_debug_csr_write,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "Debug csr read <port_list> <devad> <addr>",
  NULL,
  "Read from 16/32 bit csr register",
  PRIO_DEBUG_PHY_10G_CSR_READ,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_cmd_debug_csr_read,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "Debug 10g_phy status [<port_list>]",
  NULL,
  "Debug 10G PHY status",
  PRIO_DEBUG_PHY_10G_STATUS,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_cmd_phy_10g_status,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "Debug 10g_phy loopback [<port_list>] [a|b|c|d|e|f|g|h|j|k] [enable|disable]",
  NULL,
  "Debug 10g_phy loopback",
  PRIO_DEBUG_PHY_10G_LOOPBACK,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_cmd_phy_10g_loopback,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "Debug 10g_phy failover [<port_list>] [a|b|c|d|e|f]",
  NULL,
  "Debug 10g_phy failover",
  PRIO_DEBUG_PHY_10G_FAILOVER,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_cmd_phy_10g_failover,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "Debug 10g_phy power [<port_list>] [enable|disable]",
  NULL,
  "Enable/disable power",        
  PRIO_DEBUG_PHY_10G_POWER,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_cmd_phy_10g_power,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "Debug 10g_phy mode [<port_list>] [lan|wan|1g]",
  NULL,
  "Debug 10G PHY mode",
  PRIO_DEBUG_PHY_10G_MODE,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_cmd_phy_10g_mode,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "Debug 10g_phy reset <port_list>",
  NULL,
  "Debug 10G PHY reset",
  PRIO_DEBUG_PHY_10G_RESET,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_cmd_phy_10g_reset,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "Debug 10g_phy gpio_role <port_list> [<gpio_list>] [input|output|pcs_rx_fault]",
  NULL,
  "Setup GPIO direction",
  PRIO_DEBUG_PHY_10G_GPIO_OUTPUT,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_cmd_phy_10g_gpio_role,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "Debug 10g_phy gpio_read <port_list> [<gpio_list>]",
  NULL,
  "Read value from GPIO input pin",
  PRIO_DEBUG_PHY_10G_GPIO_READ,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_cmd_phy_10g_gpio_read,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "Debug 10g_phy gpio_write <port_list> [<gpio_list>] <value>",
  NULL,
  "Write value to GPIO output pin",
  PRIO_DEBUG_PHY_10G_GPIO_WRITE,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_cmd_phy_10g_gpio_write,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry (
  "Debug 10g_phy fw_status <port_list> ",
  NULL,
  "Get the status for the EDC FW running in the internal CPU",
  PRIO_DEBUG_PHY_10G_FW_LOAD,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_cmd_phy_10g_fw_status,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);
#endif /* VTSS_CHIP_10G_PHY */

#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
cli_cmd_tab_entry (
  "Debug PHY 1588_Read <port_list> <blk_id_1588> <csr_reg_list>",
  NULL,
  "Read PHY 1588 register",
  PRIO_DEBUG_PHY_1588_MMD_READ,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_cmd_debug_phy_1588_mmd_read,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "Debug PHY 1588_Write <port_list> <blk_id_1588> <csr_reg_offset> <value>",
  NULL,
  "Write PHY 1588 register",
  PRIO_DEBUG_PHY_1588_MMD_WRITE,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_cmd_debug_phy_1588_mmd_write,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);
#endif /* VTSS_FEATURE_PHY_TIMESTAMP */

#if defined(VTSS_SW_OPTION_BOARD)
cli_cmd_tab_entry (
  "Debug Interrupt_Source_Hook <int_source> [enable|disable]",
  NULL,
  "Enable/disable of interrupt hook",
  PRIO_DEBUG_INTERRUPT_HOOK,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_cmd_debug_interrupt_hook,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);
#endif /* VTSS_SW_OPTION_BOARD */

#ifdef VTSS_FEATURE_WIS
cli_cmd_tab_entry (
  "Debug WIS Reset <port_list>",
  NULL,
  "Reset WIS block",
  PRIO_DEBUG_INTERRUPT_HOOK,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_wis_reset,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry (
  "Debug WIS Mode <port_list> [enable|disable]",
  NULL,
  "Get or Set WIS Mode",
  PRIO_DEBUG_INTERRUPT_HOOK,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_wis_mode,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry (
 "Debug WIS Cons_Act <port_list> [(<wis_aisl> <wis_rdil> <wis_fault>)]",
  NULL,
  "Get or set WIS consequent actions",
  PRIO_DEBUG_INTERRUPT_HOOK,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_wis_conse_act,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry (
 "Debug WIS TxTTI <port_list> [(<overhead> <tti_mode> <tti>)]",
  NULL,
  "Get or set Tx TTI",
  PRIO_DEBUG_INTERRUPT_HOOK,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_wis_txtti,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry (
 "Debug WIS Defects <port_list>",
  NULL,
  "Get WIS Defects",
  PRIO_DEBUG_INTERRUPT_HOOK,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_wis_defects,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry (
 "Debug WIS Status <port_list>",
  NULL,
  "Get WIS Status",
  PRIO_DEBUG_INTERRUPT_HOOK,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_wis_status,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry (
  "Debug WIS ATTI <port_list> [<overhead>]",
  NULL,
  "Get Rx Accepted TTI",
  PRIO_DEBUG_INTERRUPT_HOOK,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_wis_atti,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry (
  "Debug WIS Test_Mode <port_list> [loopback|no_loopback] [gen_dis|gen_sqr|gen_prbs31|gen_mix] [ana_dis|ana_prbs31|ana_mix]",
  NULL,
  "Get or set test mode",
  PRIO_DEBUG_INTERRUPT_HOOK,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_wis_test_mode,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry (
 "Debug WIS Test_Status <port_list>",
  NULL,
  "Get WIS test status",
  PRIO_DEBUG_INTERRUPT_HOOK,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_wis_test_status,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry (
 "Debug WIS Counters <port_list>",
  NULL,
  "Get error counters",
  PRIO_DEBUG_INTERRUPT_HOOK,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_wis_counters,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry (
  "Debug WIS Force_Conf <port_list> [(<line_rx> <line_tx> <path_force>)]",
  NULL,
  "Get or set WIS Force Configuration",
  PRIO_DEBUG_INTERRUPT_HOOK,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_wis_force_conf,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry (
  "Debug WIS Tx_Oh_Conf <port_list> [(<oh_name> <oh_value>)]",
  NULL,
  "Set one tx overhead register or get all tx overhead bytes",
  PRIO_DEBUG_INTERRUPT_HOOK,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_wis_tx_overhead_conf,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "Debug WIS Prbs31_Err_Inj <port_list> [single_err|sat_err]",
  NULL,
  "Inject prbs31 errors",
  PRIO_DEBUG_INTERRUPT_HOOK,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_wis_prbs31_err_inj_conf,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "Debug WIS Perf_Thr <port_list> [(<n_ebc_thr_s> <n_ebc_thr_l> <f_ebc_thr_l> <n_ebc_thr_p> <f_ebc_thr_p>)]",
  NULL,
  "Get or set WIS performance counter threshold configuration",
  PRIO_DEBUG_INTERRUPT_HOOK,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_wis_tx_perf_thr_conf,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "Debug WIS Perf_Counters [<port_list>]",
  NULL,
  "Get performance counters",
  PRIO_DEBUG_INTERRUPT_HOOK,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_wis_perf_counters_get,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry (
  "Debug WIS Event_Force [<port_list>] [<force_events>] [enable|disable]",
  NULL,
  "Force event",
  PRIO_DEBUG_INTERRUPT_HOOK,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_wis_event_force_conf,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

#endif //VTSS_FEATURE_WIS

cli_cmd_tab_entry (
  "Debug PHY instance [default|phy_inst]",
  NULL,
  "Use default or phy_inst with debug PHY CLI commands",
  PRIO_DEBUG_PHY_INSTANCE,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_phy_inst_set,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

#ifdef VTSS_FEATURE_MACSEC
cli_cmd_tab_entry (
  "Debug macsec write <port_list> <block> <addr> <value>",
  NULL,
  "Write to macsec csr register",
  PRIO_DEBUG_PHY_MACSEC,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_cmd_debug_macsec_write,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "Debug macsec read <port_list> <block> <addr>",
  NULL,
  "Read from MacSec",
  PRIO_DEBUG_PHY_MACSEC,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_cmd_debug_macsec_read,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "Debug macsec dump <port_list>",
  NULL,
  "Read from MacSec",
  PRIO_DEBUG_PHY_MACSEC,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_cmd_debug_macsec_dump,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);
#endif

#ifdef VTSS_FEATURE_SYNCE_10G
cli_cmd_tab_entry (
  "Debug 10g_phy synce_clkout <port_list> [enable|disable]",
  NULL,
  "Enable or Disable SyncE Clock out",
  PRIO_DEBUG_INTERRUPT_HOOK,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_synce_clkout_set,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry (
  "Debug 10g_phy xfp_clkout <port_list> [enable|disable]",
  NULL,
  "Enable or Disable Clock out for XFP",
  PRIO_DEBUG_INTERRUPT_HOOK,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_xfp_clkout_set,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry (
 "Debug 10g_phy synce <port_list> [<mode>] [<clk_out>] [<hitless>] [<rclk_div>] [<sref_div>] [<wref_div>]",
  NULL,
  "Set SyncE modes on 10G PHY",
  PRIO_DEBUG_INTERRUPT_HOOK,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_synce_mode_set,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry (
    "Debug 10g_phy srefclk <port_list> [enable|disable]",
    NULL,
    "Enable or Disable locking line tx clock to srefclk input (Venice family only)",
    PRIO_DEBUG_INTERRUPT_HOOK,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    misc_cli_srefclk_set,
    misc_cli_req_default_set,
    misc_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif

#if defined(VTSS_FEATURE_IPV4_MC_SIP)
cli_cmd_tab_entry (
  "Debug IPMC Add <vid> <sip> <dip> <port_list> [ipv6]",
  NULL,
  "Add IP multicast forwarding entry",
  PRIO_DEBUG_IPMC_ADD,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_ipmc_add,
  NULL,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "Debug IPMC Delete <vid> <sip> <dip> [ipv6]",
  NULL,
  "Delete IP multicast forwarding entry",
  PRIO_DEBUG_IPMC_DEL,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_ipmc_del,
  NULL,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);
#endif /* VTSS_FEATURE_IPV4_MC_SIP */
cli_cmd_tab_entry (
  "Debug SPI_Transfer <spi_cs> <gpio_mask> <gpio_value> <no_of_bytes> [<value_list>]",
  NULL,
  "Perform the SPI Transaction\n",
  PRIO_DEBUG_SPI_READ,
  CLI_CMD_TYPE_STATUS,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_spi_transaction,
  NULL,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  "Debug qs [<qs_mode>] [<port_list>] [<port_bytes>] [<queue_list>] [<queue_bytes>] [<oversub_in_pct>]",
  NULL,
  "Setup JR Queue system",
  PRIO_DEBUG_QS,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_DEBUG,
  misc_cli_cmd_debug_qs,
  misc_cli_req_default_set,
  misc_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

