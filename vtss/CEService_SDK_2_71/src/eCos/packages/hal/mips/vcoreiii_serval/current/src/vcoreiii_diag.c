//==========================================================================
//
//      vcoreiii_diag.c
//
//      Misc diagnostics
//
//==========================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under    
// the terms of the GNU General Public License as published by the Free     
// Software Foundation; either version 2 or (at your option) any later      
// version.                                                                 
//
// eCos is distributed in the hope that it will be useful, but WITHOUT      
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or    
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License    
// for more details.                                                        
//
// You should have received a copy of the GNU General Public License        
// along with eCos; if not, write to the Free Software Foundation, Inc.,    
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.            
//
// As a special exception, if other files instantiate templates or use      
// macros or inline functions from this file, or you compile this file      
// and link it with other works to produce a work based on this file,       
// this file does not by itself cause the resulting work to be covered by   
// the GNU General Public License. However the source code for this file    
// must still be made available in accordance with section (3) of the GNU   
// General Public License v2.                                               
//
// This exception does not invalidate any other reasons why a work based    
// on this file might be covered by the GNU General Public License.         
// -------------------------------------------                              
// ####ECOSGPLCOPYRIGHTEND####                                              
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    Lars Povlsen
// Contributors:
// Date:         2010-06-11
// Purpose:      Misc diagnostics
// Description:
//
//####DESCRIPTIONEND####

#include <redboot.h>
#include <fis.h>
#include <cyg/hal/plf_io.h>
#include <cyg/infra/cyg_ass.h>         // assertion macros
#include <cyg/io/flash.h>
#include <pkgconf/hal.h>
#include <cyg/hal/vcoreiii_diag.h>

externC void led_mode(int mode) __attribute__ ((weak, alias("_led_mode")));

#if !defined(CYGSEM_REDBOOT_PLF_STARTUP)

/*
 * Dummy definition - no LED support
 */
void _led_mode(int mode)
{
}

#else  /* CYGSEM_REDBOOT_PLF_STARTUP */

/*
 * Generic GPIO control offered
 */

void gpio_out(int gpio)
{
    cyg_uint32 mask = VTSS_BIT(gpio);
    VTSS_DEVCPU_GCB_GPIO_GPIO_ALT(0) &= ~mask; /* GPIO mode 0b00 */
    VTSS_DEVCPU_GCB_GPIO_GPIO_ALT(1) &= ~mask;
    VTSS_DEVCPU_GCB_GPIO_GPIO_OE |= mask; /* OE enable */
}

void gpio_set(int gpio, int on)
{
    cyg_uint32 mask = VTSS_BIT(gpio);
    if(on)
        VTSS_DEVCPU_GCB_GPIO_GPIO_OUT_SET = mask;
    else
        VTSS_DEVCPU_GCB_GPIO_GPIO_OUT_CLR = mask;
}

void gpio_alt0(int gpio)
{
    cyg_uint32 mask = VTSS_BIT(gpio);
    VTSS_DEVCPU_GCB_GPIO_GPIO_ALT(0) |= mask;
    VTSS_DEVCPU_GCB_GPIO_GPIO_ALT(1) &= ~mask;
}

static int green_port, green_bit;
static int red_port, red_bit;

void sgpio_port_bit_set(int port, int bit, int mode)
{
    cyg_uint32 mask = 3 << (3 * bit);
    VTSS_DEVCPU_GCB_SIO_CTRL_SIO_PORT_CONFIG(port) =
        (VTSS_DEVCPU_GCB_SIO_CTRL_SIO_PORT_CONFIG(port) & ~mask) |
        (mode & 3) << (3 * bit);
}

void _led_mode(int mode)
{
    switch(mode) {
    case STATUS_LED_OFF:
        sgpio_port_bit_set(green_port, green_bit, 0);
        sgpio_port_bit_set(red_port,   red_bit,   0);
        break;
    case STATUS_LED_GREEN_ON:
        sgpio_port_bit_set(green_port, green_bit, 1);
        sgpio_port_bit_set(red_port,   red_bit,   0);
        break;
    case STATUS_LED_RED_ON:
        sgpio_port_bit_set(green_port, green_bit, 0);
        sgpio_port_bit_set(red_port,   red_bit,   1);
        break;
    case STATUS_LED_GREEN_BLINK1:
        sgpio_port_bit_set(green_port, green_bit, 2);
        sgpio_port_bit_set(red_port,   red_bit,   0);
        break;
    case STATUS_LED_RED_BLINK1:
        sgpio_port_bit_set(green_port, green_bit, 0);
        sgpio_port_bit_set(red_port,   red_bit,   2);
        break;
    case STATUS_LED_GREEN_BLINK2:
        sgpio_port_bit_set(green_port, green_bit, 3);
        sgpio_port_bit_set(red_port,   red_bit,   0);
        break;
    case STATUS_LED_RED_BLINK2:
        sgpio_port_bit_set(green_port, green_bit, 0);
        sgpio_port_bit_set(red_port,   red_bit,   3);
        break;
    }
}

void cyg_plf_redboot_startup(void) __attribute__ ((weak, alias("_cyg_plf_redboot_startup")));

void _cyg_plf_redboot_startup(void)
{
    int powerup_led = STATUS_LED_GREEN_ON;
    const char *board;
    int bit_count = 2;          /* Lu10 */
    cyg_uint32 port;

    gpio_alt0(0);               /* SG_CLK */
    gpio_alt0(1);               /* SG_LD */
    gpio_alt0(2);               /* SG_DO */
    gpio_alt0(3);               /* SG_DI */

    /* Serval Ref */
    board = "Serval Reference";
    green_port = red_port = 11;
    green_bit = 0;
    red_bit = 1;
    bit_count = 2;
    VTSS_DEVCPU_GCB_SIO_CTRL_SIO_PORT_ENABLE = 0xFFFF0FFF; /* Enable [31:24] and [15:0] */

    VTSS_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG =
        VTSS_F_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG_SIO_BMODE_0(2) |
        VTSS_F_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG_SIO_BMODE_1(1) |
        VTSS_F_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG_SIO_BURST_GAP(0x1F) |
        VTSS_F_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG_SIO_PORT_WIDTH(bit_count - 1) |
        VTSS_F_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG_SIO_AUTO_REPEAT;

    /* Setup the serial IO clock frequency - 12.5MHz (0x14) */
    VTSS_DEVCPU_GCB_SIO_CTRL_SIO_CLOCK = 0x14;
    
    /* Reset all SGPIO ports */
    for (port = 0; port < 32; port++)
        VTSS_DEVCPU_GCB_SIO_CTRL_SIO_PORT_CONFIG(port) = 0;

    diag_printf("%s board detected (VSC%04x Rev. %c).\n",
                board,
                (unsigned int)VTSS_X_DEVCPU_GCB_CHIP_REGS_CHIP_ID_PART_ID(VTSS_DEVCPU_GCB_CHIP_REGS_CHIP_ID), 
                (char) (VTSS_X_DEVCPU_GCB_CHIP_REGS_CHIP_ID_REV_ID(VTSS_DEVCPU_GCB_CHIP_REGS_CHIP_ID)+'A'));

    if(VTSS_DEVCPU_GCB_CHIP_REGS_HW_STAT & VTSS_F_DEVCPU_GCB_CHIP_REGS_HW_STAT_MEM_FAIL) {
        powerup_led = STATUS_LED_RED_ON;
        script = "diag -a\n";    /* Override automatic startup script */
        script_timeout = 1;      /* Speed up wait */
    }

    led_mode(powerup_led);
}

#endif  /* CYGSEM_REDBOOT_PLF_STARTUP */


/******************************************************************************/
// ddrtest_walking_one()
// Helper function for ddrtest(). Writes walking-one values to a given memory
// address. Temporarily disables dcache if on.
/******************************************************************************/
static int 
ddrtest_walking_one(const char *test_name, 
                    cyg_uint32 *ptr)
{
    cyg_uint32 val, expect, i;

    for(i = 0; i < 32; i++) {
        ptr[0] = (1 << i);      /* The cell data */
        ptr[1] = ~0L;           /* Drive other Data pins */
        if((val = *ptr) != (expect = (1 << i))) {
            diag_printf("\n%s: Walking-one RAM failure at %p, expected 0x%08x but got 0x%08x\n", test_name, ptr, expect, val);
            return 0;
        }
    }

    return 1; // Success
}

/******************************************************************************/
// ddrtest()
/******************************************************************************/

#define TCAM_CTRL_INIT        (VTSS_BIT(1)|VTSS_BIT(0)) /* BIST+INIT */
#define TCAM_CTRL_BIST_ACTIVE VTSS_BIT(1)               /* BIST */
#define TCAM_STAT_BIST_ERR    VTSS_BIT(2)               /* BIST_ERR */

static int run_tcam_bist(volatile unsigned long* control,
                         volatile unsigned long* status,
                         const char *name)
{
    int wait = 0;

    diag_printf("%s TCAM self-test: ... ", name);

    *control = TCAM_CTRL_INIT;

    while(*control & TCAM_CTRL_BIST_ACTIVE) {
        if(wait++ > 10) {
            diag_printf("Timeout\n");
            return 0;
        }
        CYGACC_CALL_IF_DELAY_US((cyg_int32)1000);
    }

    if(!(*status & TCAM_STAT_BIST_ERR)) {
        diag_printf("Passed\n");
        return 1;
    }

    diag_printf("Failed\n");
    return 0;
}

static int tcamtest(bool show_progress)
{
    int wait = 0;

    /*
     * NOTE: In order to run ES0 TCAM BIST, we need to initialize the
     * memory controller - AND enable the switch core. Needless to say
     * this and the BIST operations in general will wreck havoc on a
     * warm-start attempt. 
     */

    VTSS_SYS_SYSTEM_RESET_CFG = 
        VTSS_F_SYS_SYSTEM_RESET_CFG_MEM_ENA|VTSS_F_SYS_SYSTEM_RESET_CFG_MEM_INIT;

    while(VTSS_SYS_SYSTEM_RESET_CFG & VTSS_F_SYS_SYSTEM_RESET_CFG_MEM_INIT) {
        if(wait++ > 10) {
            diag_printf("Timeout in memory init.\n");
            return 0;
        }
        CYGACC_CALL_IF_DELAY_US((cyg_int32)1000);
    }

    /* Enable switch core */
    VTSS_SYS_SYSTEM_RESET_CFG |= VTSS_F_SYS_SYSTEM_RESET_CFG_CORE_ENA;

    return 
        run_tcam_bist(&VTSS_VCAP_CORE_TCAM_BIST_TCAM_CTRL(VTSS_TO_IS0), 
                      &VTSS_VCAP_CORE_TCAM_BIST_TCAM_STAT(VTSS_TO_IS0), "IS0") &&
        run_tcam_bist(&VTSS_VCAP_CORE_TCAM_BIST_TCAM_CTRL(VTSS_TO_S1), 
                      &VTSS_VCAP_CORE_TCAM_BIST_TCAM_STAT(VTSS_TO_S1), " S1") &&
        run_tcam_bist(&VTSS_VCAP_CORE_TCAM_BIST_TCAM_CTRL(VTSS_TO_S2), 
                      &VTSS_VCAP_CORE_TCAM_BIST_TCAM_STAT(VTSS_TO_S2), " S2") &&
        run_tcam_bist(&VTSS_VCAP_CORE_TCAM_BIST_TCAM_CTRL(VTSS_TO_ES0), 
                      &VTSS_VCAP_CORE_TCAM_BIST_TCAM_STAT(VTSS_TO_ES0), "ES0");
}

/******************************************************************************/
// ddrtest()
/******************************************************************************/
static int ddrtest(bool show_progress, bool keep_going)
{
#define BOUNDARY_1MBYTE(ptr) ((((cyg_uint32)ptr) & (0x100000-1)) == 0)
    const char *test_name = "DDR SDRAM";
    volatile cyg_uint32 *ptr;
    cyg_uint32 val, expect;

again:

    if(show_progress)
        diag_printf("%s: Testing [%p-%p] - Zero  Sweep", test_name, mem_segments[0].start, mem_segments[0].end);
    vcoreiii_zero_mem((cyg_uint32 *)mem_segments[0].start, (cyg_uint32 *)mem_segments[0].end);

    if(show_progress) {
        diag_printf(" Done\n");
        diag_printf("%s: Testing [%p-%p] - Write Sweep ", test_name, mem_segments[0].start, mem_segments[0].end);
    }

    // Write sweep
    for(ptr = (cyg_uint32 *)mem_segments[0].start; ptr < (cyg_uint32 *)mem_segments[0].end; ptr++) {
        if(BOUNDARY_1MBYTE(ptr)) {
            if(show_progress)
                diag_write_char('.');
            // Do a walking-one test on every 1MByte boundary.
            if(!ddrtest_walking_one(test_name, (cyg_uint32 *)ptr)) {
                return 0;
            }
        }
        *ptr = ~(cyg_uint32)ptr;
    }

    if(show_progress) {
        diag_printf(" Done\n");
        diag_printf("%s: Testing [%p-%p] - Read  Sweep ", test_name, mem_segments[0].start, mem_segments[0].end);
    }

    /* Read sweep */
    for(ptr = (cyg_uint32 *)mem_segments[0].start; ptr < (cyg_uint32 *)mem_segments[0].end; ptr++) {
        if(BOUNDARY_1MBYTE(ptr) && show_progress)
            diag_write_char('.');
        if((val = *ptr) != (expect = ~(cyg_uint32)ptr)) {
            diag_printf("\nError: %s: RAM failure at %p, expected 0x%08x but got 0x%08x\n", test_name, ptr, expect, val);
            return 0;
        }
    }

    if(show_progress)
        diag_printf(" Done\n");

    if(keep_going)
        goto again;

    return 1; // Success
#undef BOUNDARY_1MBYTE
}

static void do_diag(int arg, char *argv[])
{
    bool quiet, run_all, keep_going, run_tcamtest, run_ramtest;
    struct option_info opts[5];
    bool success = 1;
    int tests = 1;              /* Always check DEVCPU:GCB_CHIP_REGS:HW_STAT */

    // Args:   option flag  takes_arg type               arg                     arg_set  name
    init_opts(&opts[0], 'q', false, OPTION_ARG_TYPE_FLG, (void **)&quiet,        NULL, "Quiet operation");
    init_opts(&opts[1], 'a', false, OPTION_ARG_TYPE_FLG, (void **)&run_all,      NULL, "Run all tests");
    init_opts(&opts[2], 't', false, OPTION_ARG_TYPE_FLG, (void **)&run_tcamtest, NULL, "Run TCAM self-test");
    init_opts(&opts[3], 'd', false, OPTION_ARG_TYPE_FLG, (void **)&run_ramtest,  NULL, "Run DDR SDRAM test");
    init_opts(&opts[4], 'k', false, OPTION_ARG_TYPE_FLG, (void **)&keep_going,   NULL, "Continous SDRAM test");

    // Args:     cnt vals idx_of_1st opts num_opts               def_arg def_arg_type def_descr
    if(!scan_opts(argc, argv, 1, opts, sizeof(opts)/sizeof(opts[0]), 0, 0, "")) {
        return;
    }

    led_mode(STATUS_LED_GREEN_BLINK2);

    if(VTSS_DEVCPU_GCB_CHIP_REGS_HW_STAT & VTSS_F_DEVCPU_GCB_CHIP_REGS_HW_STAT_MEM_FAIL) {
        diag_printf("Hardware self-test: ... Failed - 0x%08x\n", (unsigned int)VTSS_DEVCPU_GCB_CHIP_REGS_HW_STAT);
        success = 0;
    } else {
        diag_printf("Hardware self-test: ... Passed\n");
    }

    if(success && (run_all || run_tcamtest))
        tests++, success = tcamtest(!quiet);

    if(success && (run_all || run_ramtest || keep_going))
        tests++, success = ddrtest(!quiet, keep_going);

    if(success) {
        switch(tests) {
        case 1:
            diag_printf("Test completed successfully.\n");
            break;
        default:
            diag_printf("%d tests completed successfully.\n", tests);
        }
        led_mode(STATUS_LED_GREEN_ON);
    } else {
        diag_printf("*** Diagnostics failed! ****\n");
        led_mode(STATUS_LED_RED_ON);
    }
}

RedBoot_cmd("diag",
            "Run Power-On-Self-Test\n"
            "  -q: Quiet operation\n"
            "  -a: Run all tests\n"
            "  -t: Run TCAM self-test\n"
            "  -d: Run DDR SDRAM test\n"
            "  -k: Run DDR SDRAM test continuously (Keep going)",
            "[-q] [-a] [-t] [-d] [-h]",
            do_diag);

static void do_ddr(int arg, char *argv[])
{
    diag_printf("ew $VTSS_MEMPARM_MEMCFG  = 0x%08x\n", VTSS_MEMPARM_MEMCFG);
    diag_printf("ew $VTSS_MEMPARM_PERIOD  = 0x%08x\n", VTSS_MEMPARM_PERIOD);
    diag_printf("ew $VTSS_MEMPARM_TIMING0 = 0x%08x\n", VTSS_MEMPARM_TIMING0);
    diag_printf("ew $VTSS_MEMPARM_TIMING1 = 0x%08x\n", VTSS_MEMPARM_TIMING1);
    diag_printf("ew $VTSS_MEMPARM_TIMING2 = 0x%08x\n", VTSS_MEMPARM_TIMING2);
    diag_printf("ew $VTSS_MEMPARM_TIMING3 = 0x%08x\n", VTSS_MEMPARM_TIMING3);
    diag_printf("ew $VTSS_MEMPARM_MR0     = 0x%08x\n", VTSS_MEMPARM_MR0);
    diag_printf("ew $VTSS_MEMPARM_MR1     = 0x%08x\n", VTSS_MEMPARM_MR1);
    diag_printf("ew $VTSS_MEMPARM_MR2     = 0x%08x\n", VTSS_MEMPARM_MR2);
    diag_printf("ew $VTSS_MEMPARM_MR3     = 0x%08x\n", VTSS_MEMPARM_MR3);
}

RedBoot_cmd("ddrparams",
            "Show calculated ddr parameters",
            "",
            do_ddr);

struct new_fis {
    char          *name;         // Name of FIS index
    CYG_ADDRESS   flash_base;    // Address within FLASH of image
    CYG_ADDRESS   mem_base;      // Address in memory where it executes
    unsigned long size;          // Length of image
    CYG_ADDRESS   entry_point;   // Execution entry point
    CYG_ADDRESS   alternate_base;// Alternate, valid flash address
};

static struct new_fis fis_map[] = {
    { "conf",           0x40040000,  0x00000000,  0x00040000,  0x00000000 },
    { "stackconf",      0x40080000,  0x00000000,  0x00100000,  0x00000000 },
    { "syslog",         0x40180000,  0x00000000,  0x00040000,  0x00000000 },
    { "managed",        0x401C0000,  0x80040000,  0x00600000,  0x800400BC,  0x407C0000 },
    { "managed.bk",     0x407C0000,  0x80040000,  0x00600000,  0x800400BC,  0x401C0000 },
};

static void do_cmd(char *command)
{
    struct cmd *cmd;

    diag_printf("%s\n", command);
    if ((cmd = parse(&command, &argc, &argv[0])) != (struct cmd *)0) {
        (cmd->fun)(argc, argv);
    } else {
        diag_printf("Invalid command: %s\n", command);
    }

}

static void do_layout(int arg, char *argv[])
{
    bool opt_list, opt_yes;
    struct option_info opts[2];
    int i, j;
    char line[CYGPKG_REDBOOT_MAX_CMD_LINE];
    struct fis_migrate { 
        bool        is_ok;
        CYG_ADDRESS flash;
        void        *ram;
        size_t      data_length;
        struct fis_image_desc *fis;
    } migration_data[NUM_ELEMS(fis_map)];
    unsigned char *heap = workspace_start; /* a.k.a. alloca */

    // Args:   option flag  takes_arg type               arg                     arg_set  name
    init_opts(&opts[0], 'l', false, OPTION_ARG_TYPE_FLG, (void **)&opt_list,   NULL, "List layout");
    init_opts(&opts[1], 'u', false, OPTION_ARG_TYPE_FLG, (void **)&opt_yes,    NULL, "Do update");

    if(!scan_opts(argc, argv, 1, opts, NUM_ELEMS(opts), 0, 0, "")) {
        return;
    }

    memset(migration_data, false, sizeof(migration_data));

    if(opt_list) {
        struct new_fis *fp = fis_map;
        diag_printf("%-16s %10s %10s %10s %10s\n",
                    "Name", "Base", "RAM", "Length", "Entry");
        for(i = 0; i < NUM_ELEMS(fis_map); i++) {
            fp = &fis_map[i];
            diag_printf("%-16s 0x%08x 0x%08x 0x%08lx 0x%08x\n",
                        fp->name, fp->flash_base, fp->mem_base, fp->size, fp->entry_point);
        }
        return;
    }

    for(j = i = 0; i < NUM_ELEMS(fis_map); i++) {
        struct new_fis *fp = &fis_map[i];
        struct fis_image_desc *fis;
        if((fis = migration_data[i].fis = fis_lookup(fp->name, NULL))) {
            if(fis->flash_base != fp->flash_base &&
               (fp->alternate_base == 0 ||
                fis->flash_base != fp->alternate_base)) {
                diag_printf("%-16s: Invalid flash offset 0x%08x\n", fp->name, fis->flash_base);
            } else if(fis->mem_base != fp->mem_base) {
                diag_printf("%-16s: Invalid RAM offset 0x%08x\n", fp->name, fis->mem_base);
            } else if(fis->size != fp->size) {
                diag_printf("%-16s: Invalid size 0x%08lx\n", fp->name, fis->size);
            } else if(fp->entry_point != 0 && (fis->entry_point != fp->entry_point)) {
                diag_printf("%-16s: Invalid entrypoint 0x%08x\n", fp->name, fis->entry_point);
            } else {
                migration_data[i].is_ok = true;
                j++;
            }
            if(!migration_data[i].is_ok) {
                /* Save origin to migrate later */
                migration_data[i].flash = fis->flash_base;
                migration_data[i].data_length = fis->data_length;
            }
        } else {
            diag_printf("%-16s: not found!\n", fp->name);
        }
    }

    if(j == NUM_ELEMS(fis_map)) {
        diag_printf("Your FIS layout is satisfactory, no changes necessary.\n");
        return;
    }

    if(!opt_yes) {
        diag_printf("Use the '-u' command to actually perform the update!\n");
        return;
    }

    /* 
     * We need to do changes, and have been told to do so
     */
    
    diag_printf("Healing layout, *DO NOT RESET WHILE UPDATING*!\n");

    /* 1. Read Data to RAM */
    for(i = 0; i < NUM_ELEMS(fis_map); i++) {
        struct new_fis *fp = &fis_map[i];
        if(migration_data[i].is_ok) {
            diag_printf("%-16s: OK, skipping\n", fp->name);
        } else {
            migration_data[i].ram = heap;
            if(migration_data[i].data_length) {
                diag_printf("%-16s: Copying to RAM @ %p\n", fp->name, heap);
                if(cyg_flash_read(migration_data[i].flash,
                                  migration_data[i].ram, 
                                  migration_data[i].data_length, NULL) != CYG_FLASH_ERR_OK) {
                    diag_printf("%-16s: Flash read error (offset 0x%08x)!\n", fp->name, migration_data[i].flash);
                    migration_data[i].data_length = 0;
                }
            } else {
                migration_data[i].data_length = fp->size;
                memset(migration_data[i].ram, 0xFF, fp->size); /* 'Erased' */
            }
            heap += migration_data[i].data_length; /* reserve data */
            CYG_ASSERT(heap < workspace_end, "negative workspace size");
        }
    }

    /* 2. "fis delete(s)" */
    for(j = i = 0; i < NUM_ELEMS(fis_map); i++) {
        struct fis_image_desc *fis;
        if((fis = migration_data[i].fis)) {
            struct new_fis *fp = &fis_map[i];
            if(!migration_data[i].is_ok) {
                fis->u.name[0] = '\xFF';
                diag_printf("%-16s: Delete FIS entry\n", fp->name);
                j++;
            }
        }
    }
    if(j) {
        /* Sync */
        extern int fis_start_update_directory(int autolock);
        extern int fis_update_directory(int autolock, int error);
        fis_start_update_directory(0);
        fis_update_directory(0, 0);
    }

    /* 3. "fis create(s)" */
    for(i = 0; i < NUM_ELEMS(fis_map); i++) {
        struct new_fis *fp = &fis_map[i];
        if(!migration_data[i].is_ok) {
            //diag_printf("%-16s: Create with %6d bytes from RAM %p to Flash 0x%08x\n", 
            //fp->name, migration_data[i].data_length,  
            //migration_data[i].ram, migration_data[i].flash);
            if(fp->size < migration_data[i].data_length) {
                diag_printf("%-16s: *WARNING* Truncating to from %zd to %ld bytes\n", 
                            fp->name, migration_data[i].data_length, fp->size);
                migration_data[i].data_length = fp->size;
            }
            if(fp->mem_base) {
                diag_sprintf(line, "fis create -e 0x%08x -r 0x%08x -b %p -s 0x%08x -f 0x%08x -l 0x%08lx %s",
                             fp->entry_point,
                             fp->mem_base,
                             migration_data[i].ram,
                             migration_data[i].data_length,
                             fp->flash_base,
                             fp->size, 
                             fp->name);
            } else {
                /* Not relocated to RAM */
                diag_sprintf(line, "fis create -b %p -r 0 -e 0 -s 0x%08lx -f 0x%08x -l 0x%08lx %s",
                             migration_data[i].ram,
                             fp->size, /* NB - set full size */
                             fp->flash_base,
                             fp->size, 
                             fp->name);
            }
            do_cmd(line);
        }
    }
    diag_printf("Healing done, please reset system!\n");
}

RedBoot_cmd("layout",
            "Utility to migrate FIS layout\n"
            "  -l: List (desired) layout\n"
            "  -u: Do update",
            "[-l] [-u]",
            do_layout);

