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

static int directly_accessible(volatile u32 *addr)
{
    return  
        (addr > (volatile u32 *) VTSS_IO_ORIGIN1_OFFSET) &&
        (addr < (volatile u32 *) (VTSS_IO_ORIGIN1_OFFSET + VTSS_IO_ORIGIN1_SIZE));
}

static inline volatile u32 *pi_map(volatile u32 *addr)
{
    return (volatile u32 *) ((((u32)addr) - VTSS_IO_ORIGIN1_OFFSET) + VTSS_MEMCTL2_TO);
}

u32 indirect_read(volatile u32 *addr)
{
    u32 val, ctl;
    *pi_map(&VTSS_DEVCPU_GCB_VCORE_ACCESS_VA_ADDR) = (u32) addr;
    val = *pi_map(&VTSS_DEVCPU_GCB_VCORE_ACCESS_VA_DATA);
    do {
        ctl = *pi_map(&VTSS_DEVCPU_GCB_VCORE_ACCESS_VA_CTRL);
    } while(ctl & VTSS_F_DEVCPU_GCB_VCORE_ACCESS_VA_CTRL_VA_BUSY);
    if(ctl & VTSS_F_DEVCPU_GCB_VCORE_ACCESS_VA_CTRL_VA_ERR) {
        IOERROR("Read error on address %p, ctl = 0x%08lx\n", addr, ctl);
    }
    val = *pi_map(&VTSS_DEVCPU_GCB_VCORE_ACCESS_VA_DATA_INERT);
    IOTRACE("Rd(%p) = 0x%08lx\n", addr, val);
    return val;
}

void indirect_write(volatile u32 *addr, u32 val)
{
    u32 ctl;
    IOTRACE("Wr(%p) = 0x%08lx\n", addr, val);
    *pi_map(&VTSS_DEVCPU_GCB_VCORE_ACCESS_VA_ADDR) = (u32) addr;
    *pi_map(&VTSS_DEVCPU_GCB_VCORE_ACCESS_VA_DATA) = val;
    do {
        ctl = *pi_map(&VTSS_DEVCPU_GCB_VCORE_ACCESS_VA_CTRL);
    } while(ctl & VTSS_F_DEVCPU_GCB_VCORE_ACCESS_VA_CTRL_VA_BUSY);
    if(ctl & VTSS_F_DEVCPU_GCB_VCORE_ACCESS_VA_CTRL_VA_ERR) {
        IOERROR("Write error on address %p, ctl = 0x%08lx\n", addr, ctl);
    }
}

u32 remote_read(volatile u32 *addr)
{
    if(directly_accessible(addr)) {
        return *pi_map(addr);
    } else {
        return indirect_read(addr);
    }
}

void remote_write(volatile u32 *addr, u32 val)
{
    if(directly_accessible(addr)) {
        (*pi_map(addr)) = val;
    } else {
        indirect_write(addr, val);
    }
}

void remote_write_masked(volatile u32 *addr, u32 value, u32 mask)
{
    u32 val = remote_read(addr);
    remote_write(addr, (val & ~mask) | (value & mask));
}

