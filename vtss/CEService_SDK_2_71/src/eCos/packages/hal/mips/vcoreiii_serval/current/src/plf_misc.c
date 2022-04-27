//==========================================================================
//
//      plf_misc.c
//
//      HAL platform miscellaneous functions
//
//==========================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Free Software Foundation, Inc.
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
// Author(s):    nickg
// Contributors: nickg, jlarmour, dmoseley, jskov
// Date:         2001-03-20
// Purpose:      HAL miscellaneous functions
// Description:  This file contains miscellaneous functions provided by the
//               HAL.
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <pkgconf/system.h>
#include <pkgconf/hal.h>

#include <cyg/infra/cyg_type.h>         // Base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros
#include <cyg/hal/hal_arch.h>           // architectural definitions
#include <cyg/hal/hal_intr.h>           // Interrupt handling
#include <cyg/hal/hal_cache.h>          // Cache handling
#include <cyg/hal/hal_if.h>
#include <cyg/hal/plf_io.h>
#ifdef CYGPKG_PROFILE_GPROF
#include <cyg/profile/profile.h>
#endif

#ifdef CYGPKG_IO_I2C_MIPS_VCOREIII
#include <cyg/io/i2c_vcoreiii.h>
#endif

#ifdef CYGPKG_DEVS_FLASH_SPI_M25PXX
#include <string.h>
#include <cyg/io/spi.h>                 // Common SPI API
#include <cyg/io/spi_vcoreiii.h>        // VCore-III SPI data structures
#include <cyg/io/m25pxx.h>              // M25P interface
#endif  /* CYGPKG_DEVS_FLASH_SPI_M25PXX */

#include <pkgconf/io_serial_mips_vcoreiii.h>

enum {
    DDR_TRAIN_OK,
    DDR_TRAIN_CONTINUE,
    DDR_TRAIN_ERROR,
};

void vcoreiii_gpio_set_alternate(int gpio, int mode)
{
    cyg_uint32 mask = VTSS_BIT(gpio);
    if (mode == 1) {
        vcoreiii_io_set(mask, &VTSS_DEVCPU_GCB_GPIO_GPIO_ALT(0));
        vcoreiii_io_clr(mask, &VTSS_DEVCPU_GCB_GPIO_GPIO_ALT(1));
    } else if (mode == 2) {
        vcoreiii_io_clr(mask, &VTSS_DEVCPU_GCB_GPIO_GPIO_ALT(0));
        vcoreiii_io_set(mask, &VTSS_DEVCPU_GCB_GPIO_GPIO_ALT(1));
    } else if (mode == 3) {
        vcoreiii_io_set(mask, &VTSS_DEVCPU_GCB_GPIO_GPIO_ALT(0));
        vcoreiii_io_set(mask, &VTSS_DEVCPU_GCB_GPIO_GPIO_ALT(1));
    } else {
        vcoreiii_io_clr(mask, &VTSS_DEVCPU_GCB_GPIO_GPIO_ALT(0));
        vcoreiii_io_clr(mask, &VTSS_DEVCPU_GCB_GPIO_GPIO_ALT(1));
    }
}

void vcoreiii_gpio_set_alternate2(int gpio, int is_alternate)
{
    cyg_uint32 mask = VTSS_BIT(gpio);
    if(is_alternate) {
        vcoreiii_io_clr(mask, &VTSS_DEVCPU_GCB_GPIO_GPIO_ALT(0));
        vcoreiii_io_set(mask, &VTSS_DEVCPU_GCB_GPIO_GPIO_ALT(1));
    } else {
        vcoreiii_io_clr(mask, &VTSS_DEVCPU_GCB_GPIO_GPIO_ALT(0));
        vcoreiii_io_clr(mask, &VTSS_DEVCPU_GCB_GPIO_GPIO_ALT(1));
    }
}

void vcoreiii_gpio_set_input(int gpio, int is_input)
{
    vcoreiii_gpio_set_alternate(gpio, 0);
    if(is_input)
        vcoreiii_io_clr(VTSS_BIT(gpio), &VTSS_DEVCPU_GCB_GPIO_GPIO_OE);
    else
        vcoreiii_io_set(VTSS_BIT(gpio), &VTSS_DEVCPU_GCB_GPIO_GPIO_OE);
}

// Function for setting a output high/low
void vcoreiii_gpio_set_output_level(int gpio, int output_level)
{
    if(output_level)
        vcoreiii_io_set(VTSS_BIT(gpio), &VTSS_DEVCPU_GCB_GPIO_GPIO_OUT);
    else
        vcoreiii_io_clr(VTSS_BIT(gpio), &VTSS_DEVCPU_GCB_GPIO_GPIO_OUT);
}

void (*vtss_system_reset_hook)(void); /* Reset hook */

#ifdef CYGPKG_DEVS_FLASH_SPI_M25PXX

/* Instantiate the CS0 SPI device */
CYG_DEVS_SPI_MIPS_VCOREIII_DEVICE( vcoreiii_spi_cs0, 0, VTSS_SPI_GPIO_NONE);

/* Instantiate the M25xx device on top of SPI CS0 */
CYG_DEVS_FLASH_SPI_M25PXX_DRIVER ( vcoreiii_m25pxx, VTSS_FLASH_TO, &vcoreiii_spi_cs0 );

static struct cyg_flash_dev_funs myfuns, orgfuns; /* To overlay for performance */

void vcoreiii_spi_bus_lock(void)
{
    cyg_spi_bus *bus = vcoreiii_spi_cs0.spi_device.spi_bus;
    while (!cyg_drv_mutex_lock(&(bus->spi_lock)));
#ifdef CYGDBG_USE_ASSERTS    
    bus->spi_current_device = &vcoreiii_spi_cs0.spi_device;
#endif
}

void vcoreiii_spi_bus_unlock(void)
{
    cyg_spi_bus *bus = vcoreiii_spi_cs0.spi_device.spi_bus;
    CYG_ASSERT(bus->spi_current_device == &vcoreiii_spi_cs0.spi_device, "SPI unlock requested without having the bus");
    cyg_drv_mutex_unlock(&(bus->spi_lock));
}

static int vcoreiii_flash_read(struct cyg_flash_dev *dev, 
                               const cyg_flashaddr_t base, 
                               void* data, 
                               size_t len)
{
    /* Guard the SPI bus while doing direct reads */
    vcoreiii_spi_bus_lock();
    memcpy(data, (void*) base, len);
    vcoreiii_spi_bus_unlock();

    return FLASH_ERR_OK;
}

static int vcoreiii_erase_block (struct cyg_flash_dev *dev, 
                                 cyg_flashaddr_t block_base)
{
    int rc = orgfuns.flash_erase_block(dev, block_base);
    HAL_DCACHE_INVALIDATE( block_base, dev->block_info[0].block_size );
    return rc;
}

static int vcoreiii_flash_program (struct cyg_flash_dev *dev, 
                                   cyg_flashaddr_t base, 
                                   const void* data, size_t len)
{
    int rc = orgfuns.flash_program(dev, base, data, len);
    HAL_DCACHE_INVALIDATE( base, len );
    return rc;
}

#endif  /* CYGPKG_DEVS_FLASH_SPI_M25PXX */

/*
 * Embedded I2C bus & devices
 */

#ifdef CYGPKG_IO_I2C_MIPS_VCOREIII

cyg_vcore_i2c_extra hal_vcore_i2c_bus_extra = {
    .i2c_wait = 100,     /* Default to maximum responds time 100 ms */
    .i2c_clk_sel = -1    /* Default use standard clk */
}; 

CYG_I2C_BUS(hal_vcore_i2c_bus,
            cyg_vcoreiii_i2c_init,
            cyg_vcoreiii_i2c_tx,
            cyg_vcoreiii_i2c_rx,
            cyg_vcoreiii_i2c_stop,
            (void *)&hal_vcore_i2c_bus_extra); 

#endif  /* CYGPKG_IO_I2C_MIPS_VCOREIII */

//--------------------------------------------------------------------------

#define TLB_HI_MASK      0xffffe000
#define TLB_LO_MASK      0x3fffffff /* Masks off Fill bits */
#define TLB_LO_SHIFT     6          /* PFN Start bit */

#define PAGEMASK_SHIFT   13

#define MMU_PAGE_CACHED   (3 << 3) /* C(5:3) Cache Coherency Attributes */
#define MMU_PAGE_UNCACHED (2 << 3) /* C(5:3) Cache Coherency Attributes */
#define MMU_PAGE_DIRTY    VTSS_BIT(2) /* = Writeable */
#define MMU_PAGE_VALID    VTSS_BIT(1)
#define MMU_PAGE_GLOBAL   VTSS_BIT(0)
#define MMU_REGIO_RO_C    (MMU_PAGE_CACHED|MMU_PAGE_VALID|MMU_PAGE_GLOBAL)
#define MMU_REGIO_RO      (MMU_PAGE_UNCACHED|MMU_PAGE_VALID|MMU_PAGE_GLOBAL)
#define MMU_REGIO_RW      (MMU_PAGE_DIRTY|MMU_REGIO_RO)
#define MMU_REGIO_INVAL   (MMU_PAGE_GLOBAL)

/*------------------------------------------------------------------------*/
/* Reset support                                                          */

void hal_vcoreiii_reset(void)
{
    CYG_INTERRUPT_STATE old;
    // Disable interrupts
    HAL_DISABLE_INTERRUPTS(old);
    // Reset the whole device, i.e. both CPU, SwC (and possibly PHYs).
    VTSS_DEVCPU_GCB_CHIP_REGS_SOFT_RST = VTSS_F_DEVCPU_GCB_CHIP_REGS_SOFT_RST_SOFT_CHIP_RST;
    while(1); // Wait for the reset to occur.
}

static void _hal_vcoreiii_cpu_reset(void)
{
    /* Remap ROM into normal "place": 0x00000000:P
     *
     * NOTE: We assume to having beeing locked into the istructions
     * cache, or we'll suffer a horrible death hereafter...
     */
    VTSS_ICPU_CFG_CPU_SYSTEM_CTRL_GENERAL_CTRL |=
        VTSS_F_ICPU_CFG_CPU_SYSTEM_CTRL_GENERAL_CTRL_BOOT_MODE_ENA;
    /* Reset CPU only - still executinh _here_, but from cache */
    VTSS_ICPU_CFG_CPU_SYSTEM_CTRL_RESET |= 
        VTSS_F_ICPU_CFG_CPU_SYSTEM_CTRL_RESET_CORE_RST_CPU_ONLY;
    VTSS_ICPU_CFG_CPU_SYSTEM_CTRL_RESET |= 
        VTSS_F_ICPU_CFG_CPU_SYSTEM_CTRL_RESET_CORE_RST_FORCE;
    while(1); // Wait for the reset to occur.
}

// Reset the CPU only
void hal_vcoreiii_cpu_reset(void)
{
    HAL_ICACHE_LOCK(_hal_vcoreiii_cpu_reset, 256); /* Just a few instructions will suffice */
    _hal_vcoreiii_cpu_reset();
}

// The following function is not available in RedBoot (hence the check for CYGPKG_KERNEL)
#ifdef CYGPKG_KERNEL
void hal_cool_restart(cyg_uint32 *addr_to_persist)
{
    extern void vcoreiii_cool_restart_asm_begin(cyg_uint32 *addr_to_persist);
    extern void vcoreiii_cool_restart_asm_end(void);
    CYG_INTERRUPT_STATE old;
    
    // Stop the scheduler
    cyg_scheduler_lock();

    // Disable interrupts
    HAL_DISABLE_INTERRUPTS(old);

    // From this point and on, we need to run in cache only, since a switch core reset
    // unfortunately causes the flip-flops controlling the AHB bus frequency to be reset
    // as well, which in turn causes the DDR2 SDRAM controller to get out of sync, effectively
    // rendering the SDRAM unusable.
    // To be able to run from ICache only, we lock the vcoreiii_cool_restart_asm_begin() function
    // into I-Cache. The length of this function is given by the difference between the
    // start of the function immediately following the vcoreiii_cool_restart_asm_begin() function,
    // which is called vcoreiii_cool_restart_asm_end() and the start of the vcoreiii_cool_restart_asm_begin()
    // function itself.
    // Now, add one cache line to this result, because the end of the vcoreiii_cool_restart_asm_begin()
    // function contains a loop, which - due to CPU pipelining - might cause the following
    // cache line to be loaded without being executed. Such an automatic load will fail
    // after we've reset the switch core.
    // The program is written in assembler because we must ensure that the program does not
    // use variables on the stack or the DDR SDRAM in any other way.
    HAL_ICACHE_LOCK(vcoreiii_cool_restart_asm_begin, (size_t)vcoreiii_cool_restart_asm_end - (size_t)vcoreiii_cool_restart_asm_begin + HAL_ICACHE_LINE_SIZE);
    
    // To be able to run from cache only, we swap to assembler-only
    vcoreiii_cool_restart_asm_begin(addr_to_persist);

    // Unreachable
    while (1) {
    }
}
#endif // CYGPKG_KERNEL

//--------------------------------------------------------------------------
// Memory controler init

/* We actually have very few 'pause' possibilities aparts from
 * these assembly nops (at this very early stage). */
#define PAUSE() asm volatile("nop; nop; nop; nop; nop; nop; nop; nop")

/* NB: Called *early* to init memory controller -
 * assumes inlining as no stack is available! */
inline void hal_vcoreiii_init_memctl(void)
{
    /* Ensure the  memory controller physical iface is forced reset */
    VTSS_ICPU_CFG_MEMCTRL_MEMPHY_CFG |= VTSS_F_ICPU_CFG_MEMCTRL_MEMPHY_CFG_PHY_RST;

    /* Ensure the memory controller is forced reset */
    VTSS_ICPU_CFG_CPU_SYSTEM_CTRL_RESET |= VTSS_F_ICPU_CFG_CPU_SYSTEM_CTRL_RESET_MEM_RST_FORCE;

    /* Wait maybe not needed, but ... */
    PAUSE();

    /* Drop sys ctl memory controller forced reset */
    VTSS_ICPU_CFG_CPU_SYSTEM_CTRL_RESET &= ~VTSS_F_ICPU_CFG_CPU_SYSTEM_CTRL_RESET_MEM_RST_FORCE;
    PAUSE();

    /* Drop Reset, enable SSTL */
    VTSS_ICPU_CFG_MEMCTRL_MEMPHY_CFG = VTSS_F_ICPU_CFG_MEMCTRL_MEMPHY_CFG_PHY_SSTL_ENA;
    PAUSE();
    
    VTSS_ICPU_CFG_MEMCTRL_MEMPHY_ZCAL = 
        VTSS_F_ICPU_CFG_MEMCTRL_MEMPHY_ZCAL_ZCAL_PROG_ODT(7) | /* 60 ohms ODT */
        VTSS_F_ICPU_CFG_MEMCTRL_MEMPHY_ZCAL_ZCAL_PROG(7) | /* 60 ohms drive strength */
        VTSS_F_ICPU_CFG_MEMCTRL_MEMPHY_ZCAL_ZCAL_ENA;

    while(VTSS_ICPU_CFG_MEMCTRL_MEMPHY_ZCAL & VTSS_F_ICPU_CFG_MEMCTRL_MEMPHY_ZCAL_ZCAL_ENA)
        ;                       /* Wait for ZCAL to clear */

    /* Drive CL, CK, ODT */
    VTSS_ICPU_CFG_MEMCTRL_MEMPHY_CFG |=
        VTSS_F_ICPU_CFG_MEMCTRL_MEMPHY_CFG_PHY_ODT_OE |
        VTSS_F_ICPU_CFG_MEMCTRL_MEMPHY_CFG_PHY_CK_OE |
        VTSS_F_ICPU_CFG_MEMCTRL_MEMPHY_CFG_PHY_CL_OE;
        
    /* Initialize memory controller */
    VTSS_ICPU_CFG_MEMCTRL_MEMCTRL_CFG = VTSS_MEMPARM_MEMCFG;
    VTSS_ICPU_CFG_MEMCTRL_MEMCTRL_REF_PERIOD = VTSS_MEMPARM_PERIOD;
    
    VTSS_ICPU_CFG_MEMCTRL_MEMCTRL_TIMING0 = VTSS_MEMPARM_TIMING0;
    VTSS_ICPU_CFG_MEMCTRL_MEMCTRL_TIMING1 = VTSS_MEMPARM_TIMING1;
    VTSS_ICPU_CFG_MEMCTRL_MEMCTRL_TIMING2 = VTSS_MEMPARM_TIMING2;
    VTSS_ICPU_CFG_MEMCTRL_MEMCTRL_TIMING3 = VTSS_MEMPARM_TIMING3;
    VTSS_ICPU_CFG_MEMCTRL_MEMCTRL_MR0_VAL = VTSS_MEMPARM_MR0;        
    VTSS_ICPU_CFG_MEMCTRL_MEMCTRL_MR1_VAL = VTSS_MEMPARM_MR1;
    VTSS_ICPU_CFG_MEMCTRL_MEMCTRL_MR2_VAL = VTSS_MEMPARM_MR2;
    VTSS_ICPU_CFG_MEMCTRL_MEMCTRL_MR3_VAL = VTSS_MEMPARM_MR3;

    /* Termination setup - enable ODT */
    VTSS_ICPU_CFG_MEMCTRL_MEMCTRL_TERMRES_CTRL = 
        VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_TERMRES_CTRL_ODT_WR_ENA(3); /* Assert ODT0 for any write */
}

void hal_vcoreiii_wait_memctl(void)
{
    /* Now, rip it! */
    VTSS_ICPU_CFG_MEMCTRL_MEMCTRL_CTRL = VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_CTRL_INITIALIZE;

    while(!(VTSS_ICPU_CFG_MEMCTRL_MEMCTRL_STAT & VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_STAT_INIT_DONE))
        ;

    /* The training below must now be called for each bytelane - see below */
}

#ifdef CYGNUM_HAL_MIPS_VCOREIII_MEMORY_16BIT
  #define ADDR_DQS_DLY(__byte_lane__) &VTSS_ICPU_CFG_MEMCTRL_MEMCTRL_DQS_DLY(__byte_lane__)
#else
  #define ADDR_DQS_DLY(__byte_lane__) &VTSS_ICPU_CFG_MEMCTRL_MEMCTRL_DQS_DLY
#endif

/* NB: Assumes inlining as no stack is available! */
static inline void set_dly(cyg_uint8 byte_lane, cyg_uint32 dly)
{
    volatile unsigned long *reg = ADDR_DQS_DLY(byte_lane);
    cyg_uint32 r = *reg;
    r &= ~VTSS_M_ICPU_CFG_MEMCTRL_MEMCTRL_DQS_DLY_DQS_DLY;
    *reg = r | VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_DQS_DLY_DQS_DLY(dly);
}

/* NB: Assumes inlining as no stack is available! */
static inline bool adjust_dly(cyg_uint8 byte_lane, int adjust)
{
    volatile unsigned long *reg = ADDR_DQS_DLY(byte_lane);
    cyg_uint32 r   = *reg;
    cyg_uint32 dly = VTSS_X_ICPU_CFG_MEMCTRL_MEMCTRL_DQS_DLY_DQS_DLY(r);
    dly += adjust;
    if(dly < 31) {
        r &= ~VTSS_M_ICPU_CFG_MEMCTRL_MEMCTRL_DQS_DLY_DQS_DLY;
        *reg = r | VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_DQS_DLY_DQS_DLY(dly);
        return true;
    }
    return false;
}

/* NB: Assumes inlining as no stack is available! */
static inline int lookfor_and_incr(cyg_uint8 byte)
{
    volatile cyg_uint8 *ram = (volatile cyg_uint8 *) VTSS_DDR_TO;
#ifdef CYGNUM_HAL_MIPS_VCOREIII_MEMORY_16BIT
    cyg_uint8 b0 = ram[0], b1 = ram[1];
    if(b0 != byte &&
       !adjust_dly(0, 1))
        return DDR_TRAIN_ERROR;
    if(b1 != byte &&
       !adjust_dly(1, 1))
        return DDR_TRAIN_ERROR;
    return ((b0 == byte) && (b1 == byte)) ? DDR_TRAIN_OK : DDR_TRAIN_CONTINUE;
#else
    cyg_uint8 b0 = ram[0];
    if(b0 != byte &&
       !adjust_dly(0, 1))
        return DDR_TRAIN_ERROR;
    return (b0 == byte) ? DDR_TRAIN_OK : DDR_TRAIN_CONTINUE;
#endif
}

/* This algorithm is converted from the TCL training algorithm used
 * during silicon simulation.
 * NB: Assumes inlining as no stack is available!
 */
inline int hal_vcoreiii_train_bytelane(void)
{
    int res;
#ifdef CYGNUM_HAL_MIPS_VCOREIII_MEMORY_16BIT
    ((volatile cyg_uint32 *)VTSS_DDR_TO)[0] = 0x0000FFFF;
    ((volatile cyg_uint32 *)VTSS_DDR_TO)[1] = 0x00000000;
#else
    ((volatile cyg_uint32 *)VTSS_DDR_TO)[0] = 0x000000FF;
#endif

    set_dly(0, 0);
#ifdef CYGNUM_HAL_MIPS_VCOREIII_MEMORY_16BIT
    set_dly(1, 0);
#endif
    while ((res = lookfor_and_incr(0xff)) == DDR_TRAIN_CONTINUE)
        ;
    if(res != DDR_TRAIN_OK)
        return res;
    while ((res = lookfor_and_incr(0x00)) == DDR_TRAIN_CONTINUE)
        ;
    if(res != DDR_TRAIN_OK)
        return res;
    
    adjust_dly(0, -3);
#ifdef CYGNUM_HAL_MIPS_VCOREIII_MEMORY_16BIT
    adjust_dly(1, -3);
#endif

#ifdef CYGNUM_HAL_MIPS_VCOREIII_MEMORY_ECC
    /* Finally, enable ECC */
    VTSS_ICPU_CFG_MEMCTRL_MEMCTRL_CFG |= VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_CFG_DDR_ECC_ENA;
    VTSS_ICPU_CFG_MEMCTRL_MEMCTRL_CFG &= ~VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_CFG_BURST_SIZE;
#endif

    /* Reset Status register - sticky bits */
    VTSS_ICPU_CFG_MEMCTRL_MEMCTRL_STAT = VTSS_ICPU_CFG_MEMCTRL_MEMCTRL_STAT;

    return DDR_TRAIN_OK;
}

static cyg_uint32
hal_getTlbCount(void)
{
    cyg_uint32 config1;
    asm volatile("mfc0 %0, $16, 1" : "=r" (config1));
    return 1 + VTSS_EXTRACT_BITFIELD(config1, 25, 5);
}

static inline void
hal_setTlbEntry(cyg_uint32 index, 
                cyg_uint32 tlbmask, 
                cyg_uint32 tlbhi, 
                cyg_uint32 tlblo0, 
                cyg_uint32 tlblo1)
{
    asm volatile("mtc0 %0, $0" : : "r" (index));
    asm volatile("mtc0 %0, $5" : : "r" (tlbmask));
    asm volatile("mtc0 %0, $10" : : "r" (tlbhi));
    asm volatile("mtc0 %0, $2" : : "r" (tlblo0));
    asm volatile("mtc0 %0, $3" : : "r" (tlblo1));
    asm volatile("nop; nop; tlbwi; nop;");
}

static inline void
create_tlb(int index, cyg_uint32 offset, cyg_uint32 size, 
           cyg_uint32 Tlb_Attrib1, cyg_uint32 Tlb_Attrib2)
{
    cyg_uint32 Tlb_Mask, Tlb_Lo0, Tlb_Lo1;
       
    Tlb_Mask  = ((size >> 12) - 1) << PAGEMASK_SHIFT;
    Tlb_Lo0 = Tlb_Attrib1 | ( offset              >> TLB_LO_SHIFT);
    Tlb_Lo1 = Tlb_Attrib2 | ((offset + size )     >> TLB_LO_SHIFT);

    hal_setTlbEntry(index, 
                    Tlb_Mask,
                    offset & TLB_HI_MASK,
                    Tlb_Lo0 & TLB_LO_MASK, 
                    Tlb_Lo1 & TLB_LO_MASK);
}

/* NB: Called *early* to map register space - 
 * assumes inlining as no stack is available! */
inline void
hal_map_iospace(void)
{
    /* 0x70000000 size 32M (0x02000000) */
    create_tlb(0, VTSS_IO_ORIGIN1_OFFSET, SZ_16M, MMU_REGIO_RW, MMU_REGIO_RW);
    
    /* 0x40000000 - 0x40ffffff - CACHED! BOOT Flash CS0 */
    create_tlb(2, VTSS_FLASH_TO,      SZ_16M, MMU_REGIO_RO_C, MMU_REGIO_RO_C);

    /* 0x20000000 - up */
#if CYGNUM_HAL_MIPS_VCOREIII_DDR_SIZE <= SZ_64M
    create_tlb(3, VTSS_DDR_TO,        SZ_64M,  MMU_REGIO_RW, MMU_REGIO_INVAL);
#elif CYGNUM_HAL_MIPS_VCOREIII_DDR_SIZE <= SZ_128M
    create_tlb(3, VTSS_DDR_TO,        SZ_64M,  MMU_REGIO_RW, MMU_REGIO_RW);
#else  /* 256M */
    create_tlb(3, VTSS_DDR_TO,        SZ_256M, MMU_REGIO_RW, MMU_REGIO_INVAL);
#endif

}

void
hal_init_tlb(void)
{
    int i, max;
    max = hal_getTlbCount();
    for(i = 0; i < max; i++)
        create_tlb(i, i * SZ_1M, SZ_4K, MMU_REGIO_INVAL, MMU_REGIO_INVAL);
}

void hal_platform_init(void)
{
    /* Serial 0 - always needed */
    vcoreiii_gpio_set_alternate(26, true);
    vcoreiii_gpio_set_alternate(27, true);
#if defined(CYGPKG_IO_SERIAL_MIPS_VCOREIII_SERIAL1)
    /* Serial 1 - only needed for RS422 */
    vcoreiii_gpio_set_alternate2(13, true);
    vcoreiii_gpio_set_alternate2(14, true);
#endif  /* CYGPKG_IO_SERIAL_MIPS_VCOREIII_SERIAL1 */

    // Init interrupt controller
    hal_init_irq();

    vtss_system_reset_hook = hal_vcoreiii_reset; /* Default to CPU reset in case of asserts etc. */

#ifdef CYGPKG_REDBOOT

    // Start the timer device running if we are in a RedBoot
    // configuration. 
    
    HAL_CLOCK_INITIALIZE( CYGNUM_HAL_RTC_PERIOD );

#endif

#if defined(CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT)
    // Set up eCos/ROM interfaces
    hal_if_init();
#endif

#ifdef CYGPKG_DEVS_FLASH_SPI_M25PXX
    vcoreiii_spi_cs0.delay = 1;
#if 1
    orgfuns = myfuns = *vcoreiii_m25pxx.funs;
    /* Read directly */
    myfuns.flash_read = vcoreiii_flash_read;
    /* Erase & invalidate */
    myfuns.flash_erase_block = vcoreiii_erase_block;
    /* program & invalidate cache */
    myfuns.flash_program = vcoreiii_flash_program;
    /* Slide in overlay */
    vcoreiii_m25pxx.funs = &myfuns;
#endif
#endif
}

//--------------------------------------------------------------------------
// Clock control

static cyg_uint32 _period;

#if defined(CYG_HAL_TIMER_ADJUSTMENT_SUPPORT)

static int _adj_enable = 0;
static timer_tick_cb _clock_callout = NULL;

// used for clock adjustment
// Initial clock offset = 0 => delta_max = 1, delta_t = 100, delta_h = 0
static  int delta_max = 1;
static  int delta_t = 100; /* number of periods */
static  int delta_h = 0; /* number of periods with delta_max offset */
static  int delta_period = 0;
static  int default_period; /* default value for _period */
static  int next_period;    /* value for _period in the nexe period */
#endif

void
hal_clock_initialize(cyg_uint32 period)
{
    /* Configure timer 0 */
    _period = period - 1;       /* We interrupt when wrapping from 0 => 0xffffffff */

    /* disable rtc ints */
    HAL_INTERRUPT_MASK(CYGNUM_HAL_INTERRUPT_RTC);

    VTSS_ICPU_CFG_TIMERS_TIMER_TICK_DIV = CYGNUM_HAL_VCOREIII_TIMER_DIVIDER-1;
    VTSS_ICPU_CFG_TIMERS_TIMER_RELOAD_VALUE(0) = _period;
    VTSS_ICPU_CFG_TIMERS_TIMER_CTRL(0) =
        VTSS_F_ICPU_CFG_TIMERS_TIMER_CTRL_TIMER_ENA |
        VTSS_F_ICPU_CFG_TIMERS_TIMER_CTRL_FORCE_RELOAD;

#if defined(CYG_HAL_TIMER_ADJUSTMENT_SUPPORT)
    default_period = _period;
    next_period = _period;
#endif
}

#if defined(CYG_HAL_TIMER_ADJUSTMENT_SUPPORT)

// Enable the timer tick period period adjustment and callout.
void hal_clock_enable_set_adjtimer(int enable, timer_tick_cb callout)
{
    CYG_INTERRUPT_STATE old;

    HAL_DISABLE_INTERRUPTS(old);
    _adj_enable = enable;
    if (!enable) {
        _period = default_period;
        next_period = _period;
        VTSS_ICPU_CFG_TIMERS_TIMER_RELOAD_VALUE(0) = _period;
    }
    _clock_callout = callout;
    HAL_RESTORE_INTERRUPTS(old);
}

// Change the timer tick period.
// The change takes place in the timer interrupt callback (hal_clock_reset)

// The function calculates these static variables, used by the interupt callback:
// delta_max = max counter offset (the counter changes between delta_max and delta_max-1
// delta_t   = number of periods for the change in offset
// delta_h   = number of periods with delta_max offset,
// (delta_t - delta_h = number of periods with delta_max-1 offset)
// delta_period = current period counter
void
hal_clock_set_adjtimer(int delta_ppm)
{
    int delta_dutycycle;
    int my_delta_t;
    int my_delta_h;
    CYG_INTERRUPT_STATE old;

    HAL_DISABLE_INTERRUPTS(old);

    if (delta_ppm >= 0) {
        delta_max = (delta_ppm / 100) + 1;
        delta_dutycycle = delta_ppm % 100;
    } else {
        delta_max = ((delta_ppm+1) / 100);
        delta_dutycycle = ((delta_ppm+1) % 100) + 99;
    }

    if (delta_dutycycle == 0) {
        my_delta_t = 100;
        my_delta_h = 0; 
    } else if (delta_dutycycle < 50) {
        my_delta_t = 100 / delta_dutycycle;
        my_delta_h = 1;
    } else {
        my_delta_t = 100 / (100 - delta_dutycycle);
        my_delta_h = my_delta_t - 1;
    }

    if (my_delta_t != delta_t || my_delta_h != delta_h) {
        delta_t = my_delta_t;
        delta_h = my_delta_h;
        delta_period = 0;
    }

    HAL_RESTORE_INTERRUPTS(old);
}
#endif

void
hal_clock_reset(cyg_uint32 vector, cyg_uint32 period)
{
#if defined(CYG_HAL_TIMER_ADJUSTMENT_SUPPORT)
    if (_clock_callout) 
        _clock_callout();
    if (_adj_enable) {
        int offset;
        /* update clock period */
        _period = next_period;
        if (delta_period < delta_h) 
            offset = delta_max;
        else 
            offset = delta_max-1;
        if (next_period != default_period - offset) {
            next_period = default_period - offset;
            VTSS_ICPU_CFG_TIMERS_TIMER_RELOAD_VALUE(0) = next_period;
        }
        if (++delta_period >= delta_t) 
            delta_period = 0;
    }
#endif
}

void
hal_clock_read(cyg_uint32 *pvalue)
{
    cyg_uint32 clockval = VTSS_ICPU_CFG_TIMERS_TIMER_VALUE(0);
    *pvalue = _period - clockval;
}

#if defined(CYG_HAL_TIMER_SUPPORT) || defined(CYGPKG_PROFILE_GPROF)
/*------------------------------------------------------------------------------
 * Compute the timer reload value.
 * The timers are clocked with a 156.25MHz clock, and are prescaled by CYGNUM_HAL_VCOREIII_TIMER_DIVIDER.
 * By default, the prescaler is not exactly one microsecond, so the compile time check below
 * will fail causing the tedious CPU-consuming computations in the else-branch.
 *------------------------------------------------------------------------------*/
static inline cyg_uint32 timer_microseconds_to_timer_value(cyg_uint32 us)
{
    cyg_uint64 result;

    if (((CYGNUM_HAL_VCOREIII_TIMER_CLOCK) / (CYGNUM_HAL_VCOREIII_TIMER_DIVIDER)) == 1000000U) {
        result = us;
    } else {
        result = ((cyg_uint64)us * ((CYGNUM_HAL_VCOREIII_TIMER_CLOCK) / (CYGNUM_HAL_VCOREIII_TIMER_DIVIDER))) / 1000000ULL;
    }
    if (result > 0) {
        result--;
    }
    if (result & 0xFFFFFFFF00000000ULL) {
        result = 0xFFFFFFFFULL;
    }
    return (cyg_uint32)result;
}
#endif

//--------------------------------------------------------------------------
// Multiple timers support
#ifdef CYG_HAL_TIMER_SUPPORT

/*------------------------------------------------------------------------------
 * Compute the number of microseconds given a timer value.
 *------------------------------------------------------------------------------*/
static inline cyg_uint32 timer_value_to_microseconds(cyg_uint32 value)
{
    cyg_uint64 result = (cyg_uint64)value + 1ULL;

    if (((CYGNUM_HAL_VCOREIII_TIMER_CLOCK) / (CYGNUM_HAL_VCOREIII_TIMER_DIVIDER)) != 1000000U) {
        // Gotta do the tedious computations.
        result = (cyg_uint32)((result * 1000000ULL) / (cyg_uint64)(((CYGNUM_HAL_VCOREIII_TIMER_CLOCK) / (CYGNUM_HAL_VCOREIII_TIMER_DIVIDER))));
    }
    return (cyg_uint32)result;
}

// The first is already in use by the system timer.
static cyg_bool hal_timers_reserved[HAL_TIMER_COUNT] ={[0] = 1};

cyg_bool hal_timer_reserve(cyg_uint32 timer_number)
{
    if (timer_number >= HAL_TIMER_COUNT || hal_timers_reserved[timer_number]) {
        return false;
    }
    hal_timers_reserved[timer_number] = true;
    return true;
}

void hal_timer_release(cyg_uint32 timer_number)
{
    if (timer_number > 0 && timer_number < HAL_TIMER_COUNT) {
        // Timer #0 cannot be released.
       hal_timers_reserved[timer_number] = false;
    }
}

cyg_bool hal_timer_enable(cyg_uint32 timer_number, cyg_uint32 period_us, cyg_bool one_shot)
{
    cyg_uint32 value;
    if (period_us == 0 || timer_number < 1 || timer_number >= HAL_TIMER_COUNT || !hal_timers_reserved[timer_number]) {
        return false;
    }

    value = timer_microseconds_to_timer_value(period_us);

    if (one_shot) {
        // In order to detect remaining time of a one-shot timer, we set the reload-value to 0,
        // so that it returns 0 in case the timer has expired.
        VTSS_ICPU_CFG_TIMERS_TIMER_VALUE(timer_number) = (cyg_uint32)value;
        VTSS_ICPU_CFG_TIMERS_TIMER_RELOAD_VALUE(timer_number) = 0;
    } else {
        VTSS_ICPU_CFG_TIMERS_TIMER_RELOAD_VALUE(timer_number) = (cyg_uint32)value;
    }
    VTSS_ICPU_CFG_TIMERS_TIMER_CTRL(timer_number) = VTSS_F_ICPU_CFG_TIMERS_TIMER_CTRL_TIMER_ENA | (one_shot ? VTSS_F_ICPU_CFG_TIMERS_TIMER_CTRL_ONE_SHOT_ENA : VTSS_F_ICPU_CFG_TIMERS_TIMER_CTRL_FORCE_RELOAD);
    return true;
}

cyg_bool hal_timer_disable(cyg_uint32 timer_number)
{
    if (timer_number < 1 || timer_number >= HAL_TIMER_COUNT || !hal_timers_reserved[timer_number]) {
        return false;
    }

    VTSS_ICPU_CFG_TIMERS_TIMER_CTRL(timer_number) &= ~VTSS_F_ICPU_CFG_TIMERS_TIMER_CTRL_TIMER_ENA;
    return true;
}

cyg_bool hal_timer_time_left(cyg_uint32 timer_number, cyg_uint32 *time_left_us)
{
    cyg_uint32 timer_value;
    if (time_left_us == NULL || timer_number >= HAL_TIMER_COUNT || !hal_timers_reserved[timer_number]) {
        return false;
    }

    timer_value = VTSS_ICPU_CFG_TIMERS_TIMER_VALUE(timer_number);
    *time_left_us = timer_value_to_microseconds(timer_value);
    return true;
}

#ifdef CYG_HAL_IRQCOUNT_SUPPORT
cyg_uint64 hal_time_get(void)
{
    cyg_uint64 result;
    cyg_uint32 regval;
    int enabled;
    HAL_DISABLE_INTERRUPTS(enabled);
    result = hal_interrupt_counts[CYGNUM_HAL_INTERRUPT_TIMER0];
    regval = VTSS_ICPU_CFG_TIMERS_TIMER_VALUE(0);
    if (VTSS_ICPU_CFG_INTR_INTR_IDENT & VTSS_BIT(CYGNUM_HAL_INTERRUPT_TIMER0 - PRI_IRQ_BASE)) {
        result++;
        regval = VTSS_ICPU_CFG_TIMERS_TIMER_VALUE(0);
    }
    HAL_RESTORE_INTERRUPTS(enabled);
    regval = _period - regval;

    // Convert to microseconds.
    result *= ((CYGNUM_HAL_RTC_NUMERATOR / CYGNUM_HAL_RTC_DENOMINATOR / 1000000)) * 1000;
    result += timer_value_to_microseconds(regval);
    return result;
}
#endif /* CYG_HAL_IRQCOUNT_SUPPORT */
#endif /* CYG_HAL_TIMER_SUPPORT */

//----------------------------------------------------------------------------
// Profiling support

#ifdef CYGPKG_PROFILE_GPROF

#define MIN_PROFILE_PERIOD	10 /* 10 usec minimum */

// Periodic timer ISR.
static cyg_uint32 
isr_pit(CYG_ADDRWORD vector, CYG_ADDRWORD data, HAL_SavedRegisters *regs)
{

    HAL_INTERRUPT_ACKNOWLEDGE (CYGNUM_HAL_INTERRUPT_TIMER2);
    __profile_hit(regs->pc);

    return 1;                   /* Cyg_InterruptHANDLED */
}

int
hal_enable_profile_timer(int resolution) /* resolution units = 1 usec  */
{
    cyg_uint32 profile_period;

#ifdef CYG_HAL_TIMER_SUPPORT
    (void)hal_timer_reserve(2);
#endif

    if(resolution < MIN_PROFILE_PERIOD)
        resolution = MIN_PROFILE_PERIOD; /* Lower bound */

    /* Calculate ticks from the main timer divider */
    profile_period = timer_microseconds_to_timer_value(resolution);

    // Attach pit isr.
    HAL_INTERRUPT_ATTACH (CYGNUM_HAL_INTERRUPT_TIMER2, &isr_pit, 0, 0);
    HAL_INTERRUPT_UNMASK (CYGNUM_HAL_INTERRUPT_TIMER2);

    /* Disable pit */
    VTSS_ICPU_CFG_TIMERS_TIMER_CTRL(2) = 0;

    /* Clear intr pit */
    HAL_INTERRUPT_ACKNOWLEDGE(CYGNUM_HAL_INTERRUPT_TIMER2);

    VTSS_ICPU_CFG_TIMERS_TIMER_RELOAD_VALUE(2) = profile_period;
    VTSS_ICPU_CFG_TIMERS_TIMER_CTRL(2) =
        VTSS_F_ICPU_CFG_TIMERS_TIMER_CTRL_TIMER_ENA |
        VTSS_F_ICPU_CFG_TIMERS_TIMER_CTRL_FORCE_RELOAD;

    return resolution;
}

void hal_suspend_profile_timer(void)
{
    /* Disable pit */
    VTSS_ICPU_CFG_TIMERS_TIMER_CTRL(2) = 0;
}

void hal_resume_profile_timer(void)
{
    /* Be sure we can resume the timer */
    if(VTSS_ICPU_CFG_TIMERS_TIMER_RELOAD_VALUE(2)) {
        VTSS_ICPU_CFG_TIMERS_TIMER_CTRL(2) =
            VTSS_F_ICPU_CFG_TIMERS_TIMER_CTRL_TIMER_ENA |
            VTSS_F_ICPU_CFG_TIMERS_TIMER_CTRL_FORCE_RELOAD;
    }
}

#endif /* CYGPKG_PROFILE_GPROF */

#ifdef CYGPKG_PROFILE_CALLGRAPH
void
hal_mcount(CYG_ADDRWORD caller_pc, CYG_ADDRWORD callee_pc)
{
    static int  nested = 0;
    int         enabled;

    HAL_DISABLE_INTERRUPTS(enabled);
    if (!nested) {
        nested = 1;
        __profile_mcount(caller_pc, callee_pc);
        nested = 0;
    }
    HAL_RESTORE_INTERRUPTS(enabled);
}
#endif /* CYGPKG_PROFILE_CALLGRAPH */

/*------------------------------------------------------------------------*/
/* End of plf_misc.c                                                      */
