/*

 Vitesse Switch API software.

 Copyright (c) 2002-2011 Vitesse Semiconductor Corporation "Vitesse". All
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
#include "dualcpu.h"

#ifdef DUALCPU_MASTER

enum {
    DDR_TRAIN_OK,
    DDR_TRAIN_CONTINUE,
    DDR_TRAIN_ERROR,
};

static volatile u32 *ram_ptr = (void *)VTSS_DDR_TO;
static volatile u32 *boot_ptr = (void *)0x00000000;

#define PAUSE() cyg_thread_delay(1)

static void dual_vcoreiii_init_memctl(void)
{
    /* Drop sys ctl memory controller is forced reset */
    RMT_CLR(VTSS_ICPU_CFG_CPU_SYSTEM_CTRL_RESET, VTSS_F_ICPU_CFG_CPU_SYSTEM_CTRL_RESET_MEM_RST_FORCE);
    PAUSE();
    
    /* Drop Reset, enable SSTL */
    RMT_WR(VTSS_ICPU_CFG_MEMCTRL_MEMPHY_CFG, VTSS_F_ICPU_CFG_MEMCTRL_MEMPHY_CFG_PHY_SSTL_ENA);
    PAUSE();
    
    RMT_WR(VTSS_ICPU_CFG_MEMCTRL_MEMPHY_ZCAL,  
           VTSS_F_ICPU_CFG_MEMCTRL_MEMPHY_ZCAL_ZCAL_PROG_ODT(7) | /* 60 ohms ODT */
           VTSS_F_ICPU_CFG_MEMCTRL_MEMPHY_ZCAL_ZCAL_PROG(7) | /* 60 ohms output impedance */
           VTSS_F_ICPU_CFG_MEMCTRL_MEMPHY_ZCAL_ZCAL_ENA);
           
    while(RMT_RD(VTSS_ICPU_CFG_MEMCTRL_MEMPHY_ZCAL) & 
          VTSS_F_ICPU_CFG_MEMCTRL_MEMPHY_ZCAL_ZCAL_ENA)
        ;                       /* Wait for ZCAL to clear */

    /* Drive CL, CK, ODT */
    RMT_SET(VTSS_ICPU_CFG_MEMCTRL_MEMPHY_CFG, 
            VTSS_F_ICPU_CFG_MEMCTRL_MEMPHY_CFG_PHY_ODT_OE |
            VTSS_F_ICPU_CFG_MEMCTRL_MEMPHY_CFG_PHY_CK_OE |
            VTSS_F_ICPU_CFG_MEMCTRL_MEMPHY_CFG_PHY_CL_OE);
        
    /* Initialize memory controller */
    RMT_WR(VTSS_ICPU_CFG_MEMCTRL_MEMCTRL_CFG, VTSS_MEMPARM_MEMCFG);
    RMT_WR(VTSS_ICPU_CFG_MEMCTRL_MEMCTRL_REF_PERIOD, VTSS_MEMPARM_PERIOD);
    
    RMT_WRM(VTSS_ICPU_CFG_MEMCTRL_MEMCTRL_TIMING0, 
            VTSS_MEMPARM_TIMING0,
            VTSS_BITMASK(20));

    RMT_WR(VTSS_ICPU_CFG_MEMCTRL_MEMCTRL_TIMING1, VTSS_MEMPARM_TIMING1);
    RMT_WR(VTSS_ICPU_CFG_MEMCTRL_MEMCTRL_TIMING2, VTSS_MEMPARM_TIMING2);
    RMT_WR(VTSS_ICPU_CFG_MEMCTRL_MEMCTRL_TIMING3, VTSS_MEMPARM_TIMING3);
    RMT_WR(VTSS_ICPU_CFG_MEMCTRL_MEMCTRL_MR0_VAL, VTSS_MEMPARM_MR0);

    /* DLL-on, Full-OD, AL=0, RTT=off, nDQS-on, RDQS-off, out-en */
    RMT_WR(VTSS_ICPU_CFG_MEMCTRL_MEMCTRL_MR1_VAL, 0x00000382);

    /* DDR2 */
    RMT_WR(VTSS_ICPU_CFG_MEMCTRL_MEMCTRL_MR2_VAL, 0);
    RMT_WR(VTSS_ICPU_CFG_MEMCTRL_MEMCTRL_MR3_VAL, 0);

    /* Termination setup - disable ODT */
    RMT_WR(VTSS_ICPU_CFG_MEMCTRL_MEMCTRL_TERMRES_CTRL, 0);
}

void dual_vcoreiii_wait_memctl(void)
{
    /* Now, rip it! */
    RMT_WR(VTSS_ICPU_CFG_MEMCTRL_MEMCTRL_CTRL, VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_CTRL_INITIALIZE);

    while(!(RMT_RD(VTSS_ICPU_CFG_MEMCTRL_MEMCTRL_STAT) & 
            VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_STAT_INIT_DONE))
        ;

    /* The training below must now be called for each bytelane - see below */
}

#ifdef CYGNUM_HAL_MIPS_VCOREIII_MEMORY_16BIT
  #define ADDR_DQS_DLY(__byte_lane__) VTSS_ICPU_CFG_MEMCTRL_MEMCTRL_DQS_DLY(__byte_lane__)
#else
  #define ADDR_DQS_DLY(__byte_lane__) VTSS_ICPU_CFG_MEMCTRL_MEMCTRL_DQS_DLY
#endif

static void set_dly(u8 byte_lane, u32 dly)
{
    RMT_WRM(ADDR_DQS_DLY(byte_lane), 
            VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_DQS_DLY_DQS_DLY(dly),
            VTSS_M_ICPU_CFG_MEMCTRL_MEMCTRL_DQS_DLY_DQS_DLY);
    IOTRACE("Lane %d, set dly = %ld\n", byte_lane, dly);
}

static bool adjust_dly(u8 byte_lane, int adjust)
{
    u32 r   = RMT_RD(ADDR_DQS_DLY(byte_lane));
    u32 dly = VTSS_X_ICPU_CFG_MEMCTRL_MEMCTRL_DQS_DLY_DQS_DLY(r);
    dly += adjust;
    IOTRACE("Lane %d, adj %d dly = %ld\n", byte_lane, adjust, dly);
    if(dly < 31) {
        set_dly(byte_lane, dly);
        return true;
    }
    return false;
}

/* NB: Assumes inlining as no stack is available! */
static int lookfor_and_incr(u8 byte)
{
    volatile u32 word = RMT_RD(ram_ptr[0]);
#ifdef CYGNUM_HAL_MIPS_VCOREIII_MEMORY_16BIT
    u8 
        b0 = word & 0xff,
        b1 = (word >> 8) & 0xff;
    if(b0 != byte &&
       !adjust_dly(0, 1))
        return DDR_TRAIN_ERROR;
    if(b1 != byte &&
       !adjust_dly(1, 1))
        return DDR_TRAIN_ERROR;
    return ((b0 == byte) && (b1 == byte)) ? DDR_TRAIN_OK : DDR_TRAIN_CONTINUE;
#else
    u8 b0 = RMT_RD(ram[0]);
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
inline int dual_vcoreiii_train_bytelane(void)
{
    int res;
#ifdef CYGNUM_HAL_MIPS_VCOREIII_MEMORY_16BIT
    RMT_WR(ram_ptr[0], 0x0000FFFF);
    RMT_WR(ram_ptr[1], 0x00000000);
#else
    RMT_WR(ram_ptr[0], 0x000000FF);
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
    return DDR_TRAIN_OK;
}

void dualcpu_ddr_init(void)
{
    u32 i, j;
    extern u32 firmware_data[], firmware_data_end[];
    u32 *cur;

    printf("dual_vcoreiii_init_memctl()\n");
    dual_vcoreiii_init_memctl();
    printf("dual_vcoreiii_wait_memctl()\n");
    dual_vcoreiii_wait_memctl();
    printf("dual_vcoreiii_train_bytelane()\n");
    if(dual_vcoreiii_train_bytelane() == DDR_TRAIN_OK)
        printf("Trained OK\n");
    else
        printf("Trained BAD\n");
    for(i = 0; i < 64; i++)
        RMT_WR(ram_ptr[i], ~i);
    for(i = 0; i < 64; i++) {
        j = RMT_RD(ram_ptr[i]);
        if(j != ~i)
            printf("Read error @%ld, read %lx expected %lx\n", i, j, ~i);
    }
    printf("Done: dualcpu_ddr_init()\n");
    printf("Firmware dld start(%p)\n", firmware_data);
    for(i = 0x10000, cur = firmware_data; cur < firmware_data_end; i++, cur++)
        RMT_WR(ram_ptr[i], *cur);
    for(i = 0x10000, cur = firmware_data; cur < firmware_data_end; i++, cur++) {
        j = RMT_RD(ram_ptr[i]);
        if(j != *cur)
            printf("FW Read error @0x%08lx, read %lx expected %lx\n", i, j, *cur);
    }
    RMT_WR(ram_ptr[0], 0x3c028004); /* lui          v0,0x8004 */
    RMT_WR(ram_ptr[1], 0x24420000); /* addiu        v0,v0,0 */
    RMT_WR(ram_ptr[2], 0x00400008); /* jr           v0 */
    RMT_WR(ram_ptr[3], 0x00000000); /* nop */
    printf("Firmware dld done(%p)\n",  firmware_data_end);
    /* Clear boot mode */
    RMT_CLR(VTSS_ICPU_CFG_CPU_SYSTEM_CTRL_GENERAL_CTRL, 
            VTSS_F_ICPU_CFG_CPU_SYSTEM_CTRL_GENERAL_CTRL_BOOT_MODE_ENA);
    u32 val = RMT_RD(VTSS_ICPU_CFG_CPU_SYSTEM_CTRL_GENERAL_CTRL);
    printf("GENERAL_CTRL = 0x%08lx\n", val);
    printf("DDR instruction %p = 0x%08lx\n", &boot_ptr[0], RMT_RD(boot_ptr[0]));
    printf("DDR instruction %p = 0x%08lx\n", &boot_ptr[1], RMT_RD(boot_ptr[1]));
    printf("DDR instruction %p = 0x%08lx\n", &boot_ptr[2], RMT_RD(boot_ptr[2]));
    printf("DDR instruction %p = 0x%08lx\n", &boot_ptr[3], RMT_RD(boot_ptr[3]));
}

#endif  /* DUALCPU_MASTER */
