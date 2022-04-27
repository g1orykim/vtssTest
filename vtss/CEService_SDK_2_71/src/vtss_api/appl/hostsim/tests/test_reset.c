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
#include "vtss_api.h"
#include "vtss_macsec_emu_base.h"

#include "base/phy/phy_10g/chips/venice/vtss_venice_regs.h"
#define VTSS_IOREG(d, is32, o)      (d), (is32), (o)

#define VTSS_RC_TEST(X)           \
{                                 \
    if ((X) != VTSS_RC_OK) {      \
        printf("Failed: "#X"\n"); \
        abort();                  \
    }                             \
}

#define VTSS_RC_NTEST(X)          \
{                                 \
    if ((X) == VTSS_RC_OK) {      \
        printf("Failed: "#X"\n"); \
        abort();                  \
    }                             \
}

#define VTSS_INT_HEX_TEST(A, B)   \
{                                 \
    if (A != B) {                 \
        printf("Failed: "#A" != "#B" (0x%08x != 0x%08x)\n", (u32)A, (u32)B); \
        abort();                  \
    }                             \
}

static vtss_rc venice_mmd_read(const vtss_inst_t    inst,
                               const vtss_port_no_t port_no,
                               const u8             device,
                               const BOOL           is32bit,
                               const u16            addr,
                               u32                  *const value)
{
    vtss_rc rc;
    if(is32bit) {
        u16 val16_1, val16_2;
        if((rc  = _mmd_read(inst, port_no, device,  addr << 1,      &val16_1)) == VTSS_RC_OK &&
           (rc  = _mmd_read(inst, port_no, device, (addr << 1) + 1, &val16_2)) == VTSS_RC_OK) {
            *value = ((val16_2 << 16) | val16_1);
        }
    } else {
        u16 val16;
        if((rc  = _mmd_read(inst, port_no, device, addr, &val16)) == VTSS_RC_OK) {
            *value = (u32) val16;
        }
    }
    return rc;
}

int main()
{
    u32 val;
    vtss_inst_t inst = instance_phy_new();

    VTSS_RC_NTEST(raw_hw_read("no-such-register", &val));
    VTSS_RC_TEST(raw_hw_write("venice::INGR::SA_STATS_IGR::IN_PKTS_LATE_LOWER[7]", 0xdeadbeaf));
    VTSS_RC_TEST(raw_hw_write("venice::INGR::SA_STATS_IGR::IN_PKTS_LATE_UPPER[7]", 0xaaaa));

    venice_mmd_read(inst, 0, VTSS_VENICE_GLOBAL_Device_Info_Device_Revision, &val);
    VTSS_INT_HEX_TEST(val, 0x0);

    venice_mmd_read(inst, 0, VTSS_MACSEC_INGR_SA_STATS_IGR_IN_PKTS_LATE_LOWER(7), &val);
    VTSS_INT_HEX_TEST(val, 0xdeadbeaf);

    venice_mmd_read(inst, 0, VTSS_MACSEC_INGR_SA_STATS_IGR_IN_PKTS_LATE_UPPER(7), &val);
    VTSS_INT_HEX_TEST(val, 0xaaaa);

    instance_phy_delete(inst);
    return 0;
}

